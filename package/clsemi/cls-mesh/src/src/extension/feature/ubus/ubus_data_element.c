#include <stdio.h>
#include <netinet/ether.h>
#include "platform.h"
#include "datamodel.h"
#include "feature/ubus/ubus_helper.h"

#define OBJ_NAME "clmesh.de"

#define ARRAY_OPEN "["
#define ARRAY_CLOSE "]"

#define DE_DESC_NEXT(_desc) ((_desc)+1)

#define DE_DESC_STRING(_key, _func) { .key = _key, .type = DE_TYPE_STRING, .de_data.de_string = _func }
#define DE_DESC_INT(_key, _func) { .key = _key, .type = DE_TYPE_INT, .de_data.de_int = _func }
#define DE_DESC_BOOL(_key, _func) { .key = _key, .type = DE_TYPE_BOOL, .de_data.de_bool = _func }
#define DE_DESC_OBJ(_key, _child, _de_obj) { .key = _key, .type = DE_TYPE_OBJECT, .child = _child, .de_obj = _de_obj }
#define DE_DESC_ARRAY(_key, _type, _child, _get_list, _get_ctx) \
    {.key = _key, .type = _type, .child = _child, .de_list = _get_list, .de_ctx = _get_ctx }
#define DE_DESC_ARRAY_SINGLE_OBJ(_key, _de_obj) \
    {.key = _key, .type = DE_TYPE_ARRAY, .de_obj = _de_obj }

#define DE_DESC_TERMINAL { .key = NULL }

#define DE_STRING_FUNC_DECLARE(_name) static char *_name(char *path, void *ctx)
#define DE_INT_FUNC_DECLARE(_name) static int _name(char *path, void *ctx)
#define DE_BOOL_FUNC_DECLARE(_name) static int _name(char *path, void *ctx)
#define DE_GET_LIST_FUNC_DECLARE(_name) struct de_list *_name(char *path, void *ctx)
#define DE_CTX_FUNC_DECLARE(_name) void *_name(char *path, void *ctx)
#define DE_OBJ_FUNC_DECLARE(_name) void *_name(char *path, void *ctx)

#define DE_BASE64_FUNC(_name, _structure, _member, _len) \
DE_STRING_FUNC_DECLARE(_name) \
{ \
    struct _structure *p = ctx; \
    if (p) { \
        return _base64_encode((char *)(_member), (int)(_len)); \
    } \
    return NULL; \
}

#define DE_STRING_FUNC(_name, _structure, _member) \
DE_STRING_FUNC_DECLARE(_name) \
{ \
    struct _structure *p = ctx; \
    if (p) { \
        return (char *)(_member);\
    } \
    return 0; \
}

#define DE_INT_FUNC(_name, _structure, _member) \
    DE_INT_FUNC_DECLARE(_name) \
    { \
        struct _structure *p = ctx; \
        if (p) { \
            return (int)(_member);\
        } \
        return 0; \
    }


enum {
    DE_TYPE_OBJECT = 0,
    DE_TYPE_ARRAY,
    DE_TYPE_INT,
    DE_TYPE_INT64,
    DE_TYPE_BOOL,
    DE_TYPE_STRING,
};


struct de_list {
    int size;
    int len;
    char *item[0];
};

struct de_desc {
    char *key;
    uint8_t type;
    struct de_list *(*de_list) (char *path, void *ctx);
    void *(*de_ctx)(char *path, void *ctx);
    void *(*de_obj)(char *path, void *ctx);
    union {
        int (*de_int) (char *path, void *ctx);
        uint64_t(*de_int64) (char *path, void *ctx);
        char *(*de_string) (char *path, void *ctx);
        int (*de_bool) (char *path, void *ctx);
    } de_data;
    struct de_desc *child;
};

#define DE_STR_BUF_LEN 128
static char _de_str_tmp[DE_STR_BUF_LEN];

static const unsigned char _base64_table[65] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *_base64_encode(char *src, int len)
{
    unsigned char *out, *pos;
    unsigned char *end, *in;
    int olen;

    olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
    olen += olen / 72; /* line feeds */
    olen++; /* nul termination */
    if (olen > DE_STR_BUF_LEN) {
        DEBUG_ERROR("_base64_encode failed, need buf len = %d\n", olen);
        return NULL;
    }
    out = (unsigned char *)_de_str_tmp;
    in = (unsigned char *)src;
    end = in + len;
    pos = out;
    while (end - in >= 3) {
        *pos++ = _base64_table[in[0] >> 2];
        *pos++ = _base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = _base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = _base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = _base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = _base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = _base64_table[((in[0] & 0x03) << 4) |
                (in[1] >> 4)];
            *pos++ = _base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    *pos = '\0';
    return (char *)out;
}

static struct de_list *_deListAdd(struct de_list *list, char *item)
{
    int new_size = 4, len=0;
    struct de_list *ret = list;

    if (list) {
        new_size = list->size<<1;
        len = list->len;
    }
    if (!(list) || (list->len>=list->size-1)) {
        ret = realloc(list, sizeof(struct de_list)+new_size*sizeof(char *));
        ret->size = new_size;
        ret->len = len;
    }
    ret->item[ret->len++] = item;
    return ret;
}

static void _deListFree(struct de_list *list)
{
    int i = 0;
    if (list) {
        for (i=0;i<list->len;i++) {
            free(list->item[i]);
        }
        free(list);
    }
}

struct ether_addr *deMACIndex(char *path, char *keyword)
{
    struct ether_addr *ret = NULL;
    char tmp;
    char *str = strstr(path, keyword);

    if (str) {
        str+=strlen(keyword)+strlen(ARRAY_OPEN);
        /* need a terminator for ether_aton */
        tmp = str[17];
        str[17] = 0;
        ret = ether_aton(str);
        str[17] = tmp;
    }
    return ret;
}

DE_STRING_FUNC_DECLARE(deNow)
{
    return PLATFORM_GET_TIMESTAMP_STR(NULL);
}


DE_BOOL_FUNC_DECLARE(deBoolTrue)
{
    return 1;
}

DE_STRING_FUNC_DECLARE(_deNetworkID)
{
    return local_config.device_info.model_name;
}

DE_STRING_FUNC_DECLARE(_deNetworkControllerID)
{
    if (local_device == registrar)
        return ether_ntoa((const struct ether_addr *)local_device->al_mac);
    return NULL;
}

struct al_device *_dealDevice(char *path)
{
    struct al_device *dev = NULL;
    struct ether_addr *mac = deMACIndex(path, "DeviceList");

    if (mac) {
        dev = alDeviceFind((uint8_t *)mac);
    }
    return dev;
}

DE_GET_LIST_FUNC_DECLARE(_deDeviceList)
{
    struct de_list *list = NULL;
    struct al_device *d;
    dlist_for_each(d, local_network, l)
    {
        char *item_s = NULL;
        asprintf(&item_s, MACFMT, MACARG(d->al_mac));
        list = _deListAdd(list, item_s);
    }
    return list;
}

DE_CTX_FUNC_DECLARE(_deDeviceContext)
{
    return (void *)_dealDevice(path);
}

DE_STRING_FUNC_DECLARE(_deDeviceID)
{
    struct al_device *dev = ctx;

    if (dev)
        return ether_ntoa((struct ether_addr *)dev->al_mac);

    return NULL;
}

DE_INT_FUNC_DECLARE(_deDeviceCollectionInterval)
{
    return local_config.collection_interval;
}

DE_BASE64_FUNC(_deDeviceApcapability, al_device, &p->ap_capability, 1)
DE_INT_FUNC(_deDeviceRadio_num, al_device, dlist_count(&p->radios))

struct radio *_deRadio(struct al_device *dev, char *path)
{
    struct radio *radio = NULL;
    struct ether_addr *mac = deMACIndex(path, "RadioList");

    if ((dev) && (mac)) {
        radio = radioFind(dev, (uint8_t *)mac);
    }
    return radio;
}

DE_GET_LIST_FUNC_DECLARE(_deDeviceRadioList)
{
    struct de_list *list = NULL;
    struct al_device *dev = ctx;
    struct radio *radio;

    if (dev) {
       dlist_for_each(radio, dev->radios, l) {
           char *item_s = NULL;
           asprintf(&item_s, MACFMT, MACARG(radio->uid));
           list = _deListAdd(list, item_s);
       }
    }
    return list;
}

DE_CTX_FUNC_DECLARE(_deRadioContext)
{
    struct al_device *dev = ctx;
    return (void *)_deRadio(dev, path);
}

DE_OBJ_FUNC_DECLARE(_deRadioCapabilities)
{
    struct radio *r = ctx;
    void *t;
    int i, j;

    if (r) {
        if (r->bands_capa[r->current_band_idx].ht_capa_valid)
            blobmsg_add_string(&b, "HTCapabilities",
                _base64_encode((char *)&r->bands_capa[r->current_band_idx].ht_capa.capa, 1));

        if (r->bands_capa[r->current_band_idx].vht_capa_valid)
            blobmsg_add_string(&b, "VHTCapabilities",
                    _base64_encode((char *)&r->bands_capa[r->current_band_idx].vht_capa, 6));

        if (r->bands_capa[r->current_band_idx].he_capa_valid) {
            struct he_capability *capa = &r->bands_capa[r->current_band_idx].he_capa;
            char buf[MAX_HE_MCS_SIZE+4];
            uint8_t mcs_len = capa->mcs[0];
            memcpy(buf, &capa->mcs[1], mcs_len);
            buf[mcs_len] = (capa->capa & 0x00ff);
            buf[mcs_len+1] = ((capa->capa & 0xff00) >> 8);
            blobmsg_add_string(&b, "HECapabilities",
                    _base64_encode(buf, mcs_len+2));
        }

        t = blobmsg_open_array(&b, "OperatingClasses");

        for (i=0;i<r->num_opc_support;i++) {
            void *t1, *a1;
            int num = 0;
            struct operating_class *opclass = &r->opclasses[i];

            t1 = blobmsg_open_table(&b, NULL);
            blobmsg_add_u32(&b, "Class", opclass->op_class);
            blobmsg_add_u32(&b, "MaxTxPower", opclass->max_tx_power);

            for (j=0;j<opclass->num_support_chan;j++) {
                if (opclass->channels[j].disable)
                    num++;
            }

            blobmsg_add_u32(&b, "NumberOfNonOperChan", num);
            a1 = blobmsg_open_array(&b, "NonOperable");
            for (j=0;j<opclass->num_support_chan;j++) {
                if (opclass->channels[j].disable)
                    blobmsg_add_u32(&b, NULL, opclass->channels[j].id);
            }

            blobmsg_close_array(&b, a1);


            blobmsg_close_table(&b, t1);
        }

        blobmsg_close_array(&b, t);
    }

    return r;
}

DE_OBJ_FUNC_DECLARE(_deRadioScanCapabilities)
{
    struct radio *r = ctx;

    if (r) {
        blobmsg_add_field(&b, BLOBMSG_TYPE_BOOL, "OnBootOnly", &r->scan_capa.scan_bootonly, 1);
        blobmsg_add_u32(&b, "Impact", r->scan_capa.impact_mode);
        blobmsg_add_u32(&b, "MinimumInterval", r->scan_capa.min_scan_interval);
        // TODO: OpClassList
    }

    return r;
}

DE_OBJ_FUNC_DECLARE(_deRadioCurrentOpclass)
{
    struct radio *r = ctx;
    if (r) {
        void *t = blobmsg_open_table(&b, NULL);
        blobmsg_add_string(&b, "TimeStamp", PLATFORM_GET_TIMESTAMP_STR(NULL));
        blobmsg_add_u32(&b, "TxPower", r->tx_power);
        blobmsg_add_u32(&b, "Channel", r->channel);
        blobmsg_close_table(&b, t);
    }
    return r;
}

DE_BASE64_FUNC(_deRadioID, radio, p->uid, MACLEN)
DE_INT_FUNC(_deRadioBSSNum, radio, p->configured_bsses.len)
DE_INT_FUNC(_deRadioNoise, radio, p->metrics.noise)
DE_INT_FUNC(_deRadioTransmit, radio, p->metrics.transmit)
DE_INT_FUNC(_deRadioReceiveSelf, radio, p->metrics.receive_self)
DE_INT_FUNC(_deRadioReceiveOther, radio, p->metrics.receive_other);
DE_INT_FUNC(_deRadioUtilization, radio, p->ch_util);


DE_GET_LIST_FUNC_DECLARE(_deRadioCurrentOpclassList)
{
    struct de_list *list = NULL;
    struct radio *r = ctx;

    if (r) {
        char *item_s = NULL;
        asprintf(&item_s, "%u", r->opclass);
        list = _deListAdd(list, item_s);
    }
    return list;
}

DE_CTX_FUNC_DECLARE(_deCurrentOpclassContext)
{
    struct radio *r = ctx;
    return (void *)r;
}

struct wifi_interface *_deBSS(struct radio *r, char *path)
{
    struct wifi_interface *bss = NULL;
    struct ether_addr *mac = deMACIndex(path, "BSSList");

    if ((r) && (mac)) {
        bss = radioFindInterface(r, (uint8_t *)mac);
    }
    return bss;
}

DE_GET_LIST_FUNC_DECLARE(_deRadioBSSList)
{
    struct de_list *list = NULL;
    struct radio *r = ctx;
    int i;

    if (r) {
        for (i = 0; i < r->configured_bsses.len; i++) {
            struct wifi_interface *ifw = r->configured_bsses.p[i];
            char *item_s = NULL;
            asprintf(&item_s, MACFMT, MACARG(ifw->i.mac));
            list = _deListAdd(list, item_s);
        }
    }
    return list;
}

DE_CTX_FUNC_DECLARE(_deBSSContext)
{
    struct radio *r = ctx;
    return (void *)_deBSS(r, path);
}

DE_STRING_FUNC_DECLARE(_deBSSId)
{
    struct wifi_interface *ifw = ctx;

    if (ifw)
        return ether_ntoa((struct ether_addr *)ifw->i.mac);

    return NULL;
}

DE_STRING_FUNC(_deBSSSsid, wifi_interface, p->bssInfo.ssid.ssid)
DE_INT_FUNC(_deBSSUnicastByteSend, wifi_interface, p->ext_metrics.uc_tx)
DE_INT_FUNC(_deBSSUnicastByteReceive, wifi_interface, p->ext_metrics.uc_rx)
DE_INT_FUNC(_deBSSMulticastByteSend, wifi_interface, p->ext_metrics.mc_tx)
DE_INT_FUNC(_deBSSMulticastByteReceive, wifi_interface, p->ext_metrics.mc_rx)
DE_INT_FUNC(_deBSSBroadcastByteSend, wifi_interface, p->ext_metrics.bc_tx)
DE_INT_FUNC(_deBSSBroadcastByteReceive, wifi_interface, p->ext_metrics.bc_rx)

struct client *_deSta(struct wifi_interface *ifw, char *path)
{
    struct client *c;
    struct ether_addr *mac = deMACIndex(path, "STAList");

    if ((ifw) && (mac)) {
        dlist_for_each(c, ifw->clients, l) {
            if (!MACCMP(c->mac, mac))
                return c;
        }
    }
    return NULL;
}

DE_GET_LIST_FUNC_DECLARE(_deBSSStalist)
{
    struct de_list *list = NULL;
    struct wifi_interface *ifw = ctx;
    struct client *sta;

    if (ifw) {
        dlist_for_each(sta, ifw->clients, l)
        {
            char *item_s = NULL;
            asprintf(&item_s, MACFMT, MACARG(sta->mac));
            list = _deListAdd(list, item_s);
        }
    }
    return list;
}

DE_CTX_FUNC_DECLARE(_deClientContext)
{
    struct wifi_interface *ifw = ctx;
    return (void *)_deSta(ifw, path);
}

DE_STRING_FUNC_DECLARE(_deClientMac)
{
    struct client *sta = ctx;

    if (sta)
        return ether_ntoa((struct ether_addr *)sta->mac);

    return NULL;
}

DE_STRING_FUNC_DECLARE(_deClientHTCapabilities)
{
    //FIXME: TO DO
    return NULL;
}

DE_STRING_FUNC_DECLARE(_deClientVHTCapabilities)
{
    //FIXME: TO DO
    return NULL;
}

DE_INT_FUNC(_deClientEstMACDataRateDL, client, p->link_metrics.mac_rate_dl)
DE_INT_FUNC(_deClientEstMACDataRateUL, client, p->link_metrics.mac_rate_ul)
DE_INT_FUNC(_deClientRCPIUL, client, p->link_metrics.rcpi_ul)
DE_INT_FUNC(_deClientLastConnectTime, client, PLATFORM_GET_AGE(p->last_assoc_ts))

DE_INT_FUNC(_deClientLastDataRateDL, client, p->ext_link_metrics.last_data_rate_dl)
DE_INT_FUNC(_deClientLastDataRateUL, client, p->ext_link_metrics.last_data_rate_ul)
DE_INT_FUNC(_deClientUtilizationReceive, client, p->ext_link_metrics.util_rx)
DE_INT_FUNC(_deClientUtilizationTransmit, client, p->ext_link_metrics.util_tx)

DE_INT_FUNC(_deClientBytesSent, client, p->traffic_stats.bytes_tx)
DE_INT_FUNC(_deClientBytesReceived, client, p->traffic_stats.bytes_rx)
DE_INT_FUNC(_deClientPacketsSent, client, p->traffic_stats.packets_tx)
DE_INT_FUNC(_deClientPacketsReceived, client, p->traffic_stats.packets_rx)
DE_INT_FUNC(_deClientErrsSent, client, p->traffic_stats.packets_err_tx)
DE_INT_FUNC(_deClientErrsReceived, client, p->traffic_stats.packets_err_rx)
DE_INT_FUNC(_deClientRetransmission, client, p->traffic_stats.retransmission)

static struct de_desc _de_desc_sta[] = {
    DE_DESC_STRING("MACAddress", _deClientMac),
    DE_DESC_STRING("HTCapabilities", _deClientHTCapabilities),
    DE_DESC_STRING("VHTCapabilities", _deClientVHTCapabilities),
    DE_DESC_INT("LastDataDownlinkRate", _deClientLastDataRateDL),
    DE_DESC_INT("LastDataUplinkRate", _deClientLastDataRateUL),
    DE_DESC_INT("UtilizationReceive", _deClientUtilizationReceive),
    DE_DESC_INT("UtilizationTransimit", _deClientUtilizationTransmit),
    DE_DESC_INT("EstMACDataRateDownlink", _deClientEstMACDataRateDL),
    DE_DESC_INT("EstMACDataRateUplink", _deClientEstMACDataRateUL),
    DE_DESC_INT("SignalStrength", _deClientRCPIUL),
    DE_DESC_INT("LastConnectTime", _deClientLastConnectTime),
    DE_DESC_INT("BytesSent", _deClientBytesSent),
    DE_DESC_INT("BytesReceived", _deClientBytesReceived),
    DE_DESC_INT("PacketsSent", _deClientPacketsSent),
    DE_DESC_INT("PacketsReceived", _deClientPacketsReceived),
    DE_DESC_INT("ErrorsSent", _deClientErrsSent),
    DE_DESC_INT("ErrorsReceived", _deClientErrsReceived),
    DE_DESC_INT("RetransCount", _deClientRetransmission ),
    DE_DESC_TERMINAL
};

static struct de_desc _de_desc_bss[] = {
    DE_DESC_STRING("BSSID", _deBSSId),
    DE_DESC_STRING("SSID", _deBSSSsid),
    DE_DESC_BOOL("Enabled", deBoolTrue),
    DE_DESC_INT("UnicastBytesSent", _deBSSUnicastByteSend),
    DE_DESC_INT("UnicastBytesReceived", _deBSSUnicastByteReceive),
    DE_DESC_INT("MulticastBytesSent", _deBSSMulticastByteSend),
    DE_DESC_INT("MulticastBytesReceived", _deBSSMulticastByteReceive),
    DE_DESC_INT("BroadcastBytesSent", _deBSSBroadcastByteSend),
    DE_DESC_INT("BroadcastBytesReceived", _deBSSBroadcastByteReceive),
    DE_DESC_ARRAY("STAList", DE_TYPE_OBJECT, _de_desc_sta, _deBSSStalist, _deClientContext),
    DE_DESC_TERMINAL
};

DE_STRING_FUNC(_deRadioCurrentOpclassTimeStamp, radio, PLATFORM_GET_TIMESTAMP_STR(NULL))
DE_INT_FUNC(_deRadioCurrentOpclassValue, radio, p->opclass)
DE_INT_FUNC(_deRadioCurrentOpclassTxpower, radio, p->tx_power)
DE_INT_FUNC(_dRadioCurrentOpclassChannel, radio, p->channel)

static struct de_desc _de_desc_current_opclass[] = {
    DE_DESC_STRING("TimeStamp", _deRadioCurrentOpclassTimeStamp),
    DE_DESC_INT("Class", _deRadioCurrentOpclassValue),
    DE_DESC_INT("TxPower", _deRadioCurrentOpclassTxpower),
    DE_DESC_INT("Channel", _dRadioCurrentOpclassChannel),
    DE_DESC_TERMINAL
};

static struct de_desc _de_desc_radio[] = {
    DE_DESC_STRING("ID", _deRadioID),
    DE_DESC_BOOL("Enabled", deBoolTrue),
    DE_DESC_ARRAY("CurrentOperatingClasses", DE_TYPE_OBJECT, _de_desc_current_opclass, _deRadioCurrentOpclassList, _deCurrentOpclassContext),
    DE_DESC_INT("NumberOfBSS", _deRadioBSSNum),
    DE_DESC_ARRAY("BSSList", DE_TYPE_OBJECT, _de_desc_bss, _deRadioBSSList, _deBSSContext),
    DE_DESC_INT("Noise", _deRadioNoise),
    DE_DESC_INT("Utilization", _deRadioUtilization),
    DE_DESC_INT("Transmit", _deRadioTransmit),
    DE_DESC_INT("ReceiveSelf", _deRadioReceiveSelf),
    DE_DESC_INT("ReceiveOther", _deRadioReceiveOther),
    DE_DESC_OBJ("Capabilities", NULL, _deRadioCapabilities),
    DE_DESC_OBJ("ScanCapability", NULL, _deRadioScanCapabilities),
    DE_DESC_TERMINAL
};

static struct de_desc _de_desc_device[] = {
    DE_DESC_STRING("ID", _deDeviceID),
    DE_DESC_STRING("MultiAPCapabilities", _deDeviceApcapability),
    DE_DESC_INT("NumberOfRadios", _deDeviceRadio_num),
    DE_DESC_ARRAY("RadioList", DE_TYPE_OBJECT, _de_desc_radio, _deDeviceRadioList, _deRadioContext),
    DE_DESC_INT("CollectionInterval", _deDeviceCollectionInterval),
    DE_DESC_TERMINAL
};

static struct de_desc _de_desc_network[] = {
    DE_DESC_STRING("ID", _deNetworkID),
    DE_DESC_STRING("TimeStamp", deNow),
    DE_DESC_STRING("ControllerID", _deNetworkControllerID),
    DE_DESC_ARRAY("DeviceList", DE_TYPE_OBJECT, _de_desc_device, _deDeviceList, _deDeviceContext),
    DE_DESC_TERMINAL
};

static struct de_desc _de_desc_network_root[] = {
    DE_DESC_OBJ("wfa-dataelements:Network", _de_desc_network, NULL),
    DE_DESC_TERMINAL
};

static void _deGetData(struct de_desc *desc, char *path, void *ctx)
{
    uint8_t value;

    if (desc->de_obj ) {
        if (desc->type == DE_TYPE_OBJECT) {
            void *t = blobmsg_open_table(&b, desc->key);
            desc->de_obj(path, ctx);
            blobmsg_close_table(&b, t);
        } else if (desc->type == DE_TYPE_ARRAY) {
            void *t = blobmsg_open_array(&b, desc->key);
            desc->de_obj(path, ctx);
            blobmsg_close_array(&b, t);
        }
    } else if ((desc->type == DE_TYPE_INT) && (desc->de_data.de_int)) {
        blobmsg_add_u32(&b, desc->key, desc->de_data.de_int(path, ctx));
    } else if ((desc->type == DE_TYPE_INT64) && (desc->de_data.de_int64)) {
        blobmsg_add_u64(&b, desc->key, desc->de_data.de_int64(path, ctx));
    } else if ((desc->type == DE_TYPE_BOOL) && (desc->de_data.de_bool)) {
        if (desc->de_data.de_bool(path, ctx))
            value = 1;
        else
            value = 0;
        blobmsg_add_field(&b, BLOBMSG_TYPE_BOOL, desc->key, &value, 1);
    } else if ((desc->type == DE_TYPE_STRING) && (desc->de_data.de_string)) {
        char *str;
        if ((str = desc->de_data.de_string(path, ctx)))
            blobmsg_add_string(&b, desc->key, str);
    } else {
        DEBUG_ERROR("unknown type=%d\n", desc->type);
    }
}

static void getDataElement(struct de_desc *desc, char *path, char *index, void *ctx)
{
    /* all array */
    if ((index == NULL) && (desc->de_list)) {
        struct de_list *list = desc->de_list(path, ctx);
        void *a = blobmsg_open_array(&b, desc->key);
        if (list) {
            int i = 0;
            for (i=0; i<list->len;i++) {
                char *tpath;
                char *sindex = list->item[i];
                if (asprintf(&tpath, "%s%s%s%s", path, ARRAY_OPEN, sindex, ARRAY_CLOSE) > 0) {
                    getDataElement(desc, tpath, sindex, ctx);
                    free(tpath);
                }
            }
            _deListFree(list);
        }
        blobmsg_close_array(&b, a);
    } else if (desc->child) {
        void *t;
        struct de_desc *child = desc->child;
        t = blobmsg_open_table(&b, desc->key);
        void *tctx = NULL;
        if (desc->de_ctx)
            tctx = desc->de_ctx(path, ctx);
        while (child->key) {
            char *tpath;
            if (asprintf(&tpath, "%s.%s", path, child->key) > 0) {
                getDataElement(child, tpath, NULL, tctx);
                free(tpath);
            }
            child = DE_DESC_NEXT(child);
        }
        blobmsg_close_table(&b, t);
    } else
        _deGetData(desc, path, ctx);
}

static int _deNetwork(struct ubus_context *ctx,
        struct ubus_object *obj, struct ubus_request_data *req,
        const char *method, struct blob_attr *msg)
{
    int ret;
    blob_buf_init(&b, 0);
    getDataElement(_de_desc_network_root, "", NULL, &ret);
    return ubus_send_reply(ctx, req, b.head);
}

static const struct ubus_method _de_obj_methods[] = {
    UBUS_METHOD_NOARG("network", _deNetwork),
};

static struct ubus_object_type _de_obj_type =
    UBUS_OBJECT_TYPE(OBJ_NAME, _de_obj_methods);

DECLARE_UBUS_OBJ(_de_obj, OBJ_NAME, &_de_obj_type, _de_obj_methods);

struct ubus_object *getDeObj(void)
{
    return &_de_obj;
}
