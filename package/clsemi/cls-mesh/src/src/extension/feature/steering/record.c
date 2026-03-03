#include "platform.h"
#include "datamodel.h"
#include "platform_os.h"
#include "al_driver.h"
#include "record.h"
#include "wifi.h"


void _steerClientOldAgeTimer(void *data)
{
    struct steer_client *steer_c, *tmp;
    struct steer_record *record, *record_tmp;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    dlist_for_each_safe(steer_c, tmp, g_steer.steer_clients, l) {
        record = steerRecordFindLatest(steer_c);
        // clear more than one day clients
        if(record && ((current_ts - record->create_ts)/1000 > g_steer.old_age_time)) {
            DEBUG_INFO("steer client("MACFMT") and records deleted for old age\n",
                    MACARG(steer_c->mac));
            steerClientDel(steer_c);
        } else {
            dlist_for_each_safe(record, record_tmp, steer_c->records, l) {
                if((current_ts - record->create_ts)/1000 > g_steer.old_age_time) {
                    DEBUG_INFO("steer client("MACFMT") delete record for old age\n",
                            MACARG(steer_c->mac));
                    steerRecordDel(record);
                }
            }
        }
    }
}

void steerInit(void)
{
    g_steer.max_stick_times = 3;
    g_steer.stick_check_interval = (g_steer.max_stick_times+1)*10;
    g_steer.max_record_num = 128;
    g_steer.old_age_time = 86400;
    g_steer.deny_acl_period = 120;

    dlist_head_init(&g_steer.steer_clients);
    dlist_head_init(&g_steer.nac_channels);

    platformAddTimer(600*1000, TIMER_FLAG_PERIODIC, _steerClientOldAgeTimer, NULL);
}

int isStickSta(struct steer_client *steer_c, uint8_t *bssid)
{
    struct steer_record *record;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);
    uint8_t count = 0;

    dlist_for_each(record, steer_c->records, l) {
        if (IS_ZERO_MAC(record->target_bss))
            continue;
        if ((current_ts - record->create_ts)/1000 > g_steer.stick_check_interval)
            break;
        // check src bssid NOT match indicate not continous
        if (MACCMP(record->src_bss, bssid))
            break;
        // has steer success record in stick_check_interval not stick sta
        if (record->status)
            break;
        count++;
        if (count >= g_steer.max_stick_times)
            return 1;
    }

    return 0;
}

struct steer_record *steerRecordFindTypeLatest(struct steer_client *steer_c, enum steer_type stype)
{
    struct steer_record *record = NULL;
    if (!steer_c)
        return NULL;

    dlist_for_each(record, steer_c->records, l) {
        if (record->type == stype) {
            return record;
        }
    }

    return NULL;
}

struct steer_record *steerRecordFindLatest(struct steer_client *steer_c)
{
    if (!steer_c)
        return NULL;

    return container_of(dlist_get_first(&steer_c->records), struct steer_record, l);
}

struct steer_record *steerRecordAdd(enum steer_type type, uint8_t method, uint8_t *mac,
        uint8_t *current_bss, uint8_t *target_bss, uint8_t src_rcpi, uint8_t target_rcpi)
{
    struct steer_record *record, *tmp;
    struct steer_client *steer_c;
    uint32_t current_ts = 0;
    uint8_t del_num = 0;

    if (!mac || !current_bss || !target_bss)
        return NULL;

    steer_c = steerClientFind(mac);
    if (!steer_c)
        steer_c = steerClientAdd(mac);
    if (!steer_c)
        return NULL;

    if (steer_c->record_num >= g_steer.max_record_num) {
        DEBUG_INFO("client("MACFMT") record_num(%u) reached max record num(%u), delete first 10 item!\n",
                MACARG(steer_c->mac), steer_c->record_num, g_steer.max_record_num);
        dlist_for_each_safe(record, tmp, steer_c->records, l) {
            if (del_num >= 10)
                break;
            steerRecordDel(record);
            del_num++;
        }
    }

    record = calloc(1, sizeof(struct steer_record));
    current_ts = PLATFORM_GET_TIMESTAMP(0);

    if(!record) {
        DEBUG_ERROR("calloc record failed!\n");
        return NULL;
    }

    record->create_ts = current_ts;
    record->type = type;
    record->c = steer_c;
    if (record->type == STEER_TYPE_RCPI) {
        record->src_rcpi = src_rcpi;
        record->target_rcpi = target_rcpi;
    }
    MACCPY(record->src_bss, current_bss);
    MACCPY(record->target_bss, target_bss);
    record->method = method;
    if (record->method == STEER_METHOD_DEAUTH) {
        record->deauth_ts = current_ts;
    }
    record->btm_status_code = 0xff;

    dlist_add_head(&steer_c->records, &record->l);
    steer_c->record_num++;

    STEER_RECORD_PRINT(record);

    return record;
}

int steerRecordDel(struct steer_record *record)
{
    if(!record)
        return 0;

    if (record->c && record->c->record_num > 0)
        record->c->record_num--;

    dlist_remove(&record->l);
    free(record);
    record = NULL;

    return 0;
}

struct steer_client *steerClientFind(uint8_t *mac)
{
    struct steer_client *steer_c = NULL;

    dlist_for_each(steer_c, g_steer.steer_clients, l) {
        if (!MACCMP(steer_c->mac, mac))
            return steer_c;
    }

    return NULL;
}

struct steer_client * steerClientAdd(uint8_t *mac)
{
    struct steer_client *steer_c = NULL;
    if (!mac)
        return NULL;

    steer_c = calloc(1, sizeof(struct steer_client));
    if (!steer_c)
        return NULL;

    steer_c->create_ts = PLATFORM_GET_TIMESTAMP(0);
    MACCPY(steer_c->mac, mac);
    dlist_head_init(&steer_c->records);
    dlist_add_head(&g_steer.steer_clients, &steer_c->l);

    return steer_c;
}

int steerClientDel(struct steer_client *steer_c)
{
    if (!steer_c)
        return 0;

    dlist_remove(&steer_c->l);
    dlist_free_items(&steer_c->records, struct steer_record, l);
    free(steer_c);
    steer_c = NULL;

    return 0;
}

struct nac_channel_item *findNacChannelItem(uint8_t channel)
{
    struct nac_channel_item *item = NULL;

    dlist_for_each(item, g_steer.nac_channels, l) {
        if (item->channel == channel)
            return item;
    }

    return NULL;
}

void nacTimerHandler(void *data)
{
    struct nac_channel_item *item = (struct nac_channel_item *)data;

    if (!item)
        return;

    setNacMonitorEnable(item->channel, 0);
    flushNacSta(item->channel);

    dlist_remove(&item->l);
    free(item);

    return;
}

int setNacEnable(uint8_t channel)
{
    struct nac_channel_item *item = NULL;

    item = findNacChannelItem(channel);
    if (item) {
        PLATFORM_CANCEL_TIMER(item->nac_timer);
        item->nac_timer = platformAddTimer(2000, 0, nacTimerHandler, item);
    }
    else {
        item = (struct nac_channel_item *)calloc(1, sizeof(struct nac_channel_item));
        if (!item)
            return -1;
        item->channel = channel;
        dlist_add_head(&g_steer.nac_channels, &item->l);
        item->nac_timer = platformAddTimer(2000, 0, nacTimerHandler, item);
    }

    setNacMonitorEnable(channel, 1);

    return 0;
}


