#include <arpa/inet.h>
#include "platform.h"
#include "datamodel.h"
#include "feature/ubus/ubus_helper.h"
#include "1905_tlvs.h"
#include "al_send.h"
#include "os_utils.h"

#define OBJ_NAME "clmesh.sigma"

enum {
    SIGMA_SEND_ATTR_ALID,
    SIGMA_SEND_ATTR_MSGTYPE,
    SIGMA_SEND_ATTR_MAX,
};

static const struct blobmsg_policy _sigma_send_1905_attr_policy[] = {
    POLICY_ATTR(SIGMA_SEND_ATTR_ALID, "DestALid", BLOBMSG_TYPE_STRING ),
    POLICY_ATTR(SIGMA_SEND_ATTR_MSGTYPE, "MessageTypeValue", BLOBMSG_TYPE_STRING ),
};

static const struct blobmsg_policy _sigma_set_config_attr_policy[] = {
    POLICY_ATTR(SIGMA_SEND_ATTR_ALID, "program", BLOBMSG_TYPE_STRING ),
};

struct generalTLV {
    struct TLV tlv;
    struct vvData data;
};


#define FIELD_DESC(structtype, field, type, ...) \
{ \
    .name = #field, \
    .size = sizeof(((structtype*)0)->field), \
    .offset = offsetof(structtype, field), \
    .fmt = type, \
    __VA_ARGS__ \
}

#define TERMINATOR {0}

#define EXTRA_FIELDS \
    .fields1 = { \
        TERMINATOR, \
    },

#define TLV_DESC_1FIELD(struct_name, _count_type, field1, fmt1, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }


static struct TLVDesc _general_tlv_desc =
    TLV_DESC_1FIELD(generalTLV, 0, data, fmt_l0vv, NULL);



DECLARE_FUNC_UBUS_METHOD(_devSend1905)
{
    uint8_t almac[MACLEN] = {0};
    struct al_device *d;
    uint16_t type;
    struct blob_attr *o;
    unsigned rem;
    uint16_t mid = 0;
    uint8_t tlv_type = 0;
    uint16_t tlv_len = 0;
    struct CMDU2 *c = NULL;
    struct generalTLV *tlv;
    char *value = NULL, *name = NULL;
    uint8_t result = RESULT_INVALID_ARGUMENT;

    blob_buf_init(&b, 0);

    blobmsg_for_each_attr(o, msg, rem) {
        if (blobmsg_type(o) != BLOBMSG_TYPE_STRING)
            continue;
        name = (char *)blobmsg_name(o);
        value = blobmsg_get_string(o);
        if (!strcmp(name, "DestALid")) {
            if (string2Hex(value, almac, MACLEN)!=MACLEN)
                goto fail;
        } else if (!strcmp(name, "MessageTypeValue")) {
            if (string2Hex(value, (uint8_t *)&type, 2)!=2)
                goto fail;
            //not allow 2 messagetypevalue fields
            if (c)
                goto fail;
            type = ntohs(type);
            mid = getNextMid();
            c = cmdu2New(type, mid);

            if (!c)
                goto fail;
        } else if (!strncmp(name, "tlv_type", 8)) {
            if (string2Hex(value, &tlv_type, 1)!=1)
                goto fail;
        } else if (!strncmp(name, "tlv_length", 10)) {
            if (string2Hex(value, (uint8_t *)&tlv_len, 2)!=2)
                goto fail;
            tlv_len = ntohs(tlv_len);
        } else if (!strncmp(name, "tlv_value", 9)) {
            if (c) {
                if (!(tlv = (struct generalTLV *)TLVStructNew(&_general_tlv_desc, NULL,
                                (sizeof(struct generalTLV)+tlv_len))))
                    goto fail;
                tlv->tlv.tlv_type = tlv_type;
                tlv->data.datap = (uint8_t *)(tlv+1);
                tlv->data.len = tlv_len;
                cmdu2AddTlv(c, (struct TLV *)tlv);
                if (string2Hex(value, tlv->data.datap, tlv_len)!=tlv_len)
                    goto fail;
            }
        }
    }

    if (!(d = alDeviceFind(almac))) {
        result = RESULT_NOT_FOUND;
        DEBUG_ERROR("can not find device["MACFMT"]\n", MACARG(almac));
        goto fail;
    }

    if (!c) {
        goto fail;
    }

    if (0 == sendRawPacket2(c, idx2InterfaceName(d->recv_intf_idx),  (uint8_t *)&almac, NULL)) {
        DEBUG_WARNING("sendRawPacket2 to "MACFMT" type=%s failed.", MACARG(almac),
                        convert_1905_CMDU_type_to_string(c->type));
    }

    blobmsg_add_u32(&b, "mid", mid);
    cmdu2Free(c);
    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
fail:
    DEBUG_ERROR("parse %s=%s failed\n", name, value);
    cmdu2Free(c);
    addResult(result);
    return ubus_send_reply(ctx, req, b.head);
}

static uint8_t _parseBand(char *band_str)
{
    if (!strcmp(band_str, "8x"))
        return band_2g;
    if ((!strcmp(band_str, "11x")) || (!strcmp(band_str, "12x")))
        return band_5g;
    return 0;
}

static struct wifi_config *_parseWifiConfig(char *info_str)
{
#define DELIMITER " "
    struct wifi_config *wconfig = calloc(1, sizeof(struct wifi_config));
    char *token;
    char *tmp;
    uint16_t value16;

    if (!wconfig)
        goto fail;

    if ((!(token = strtok_r(info_str, " ", &tmp)))
        || (string2Hex(token, wconfig->bss.bssid, MACLEN)!=MACLEN))
        goto fail;
    if ((!(token = strtok_r(NULL, " ", &tmp)))
        || (!(wconfig->bands = _parseBand(token))))
        goto fail;
    if ((!(token = strtok_r(NULL, " ", &tmp)))
        || (!strlen(token)))
        goto fail;
    wconfig->bss.ssid.len = strlen(token);
    memcpy(wconfig->bss.ssid.ssid, token, wconfig->bss.ssid.len);

    if ((!(token = strtok_r(NULL, " ", &tmp)))
        || (string2Hex(token, (uint8_t *)&value16, 2)!=2))
        goto fail;
    wconfig->bss.auth = ntohs(value16);

    if ((!(token = strtok_r(NULL, " ", &tmp)))
        || (string2Hex(token, (uint8_t *)&value16, 2)!=2))
        goto fail;
    wconfig->bss.encrypt = ntohs(value16);

    if ((!(token = strtok_r(NULL, " ", &tmp)))
        || (!strlen(token)))
        goto fail;
    wconfig->bss.key.len = strlen(token);
    memcpy(wconfig->bss.key.key, token, wconfig->bss.key.len);

    if (!(token = strtok_r(NULL, " ", &tmp)))
        goto fail;
    wconfig->bss.backhaul = atoi(token);
    if (!(token = strtok_r(NULL, " ", &tmp)))
        goto fail;
    wconfig->bss.fronthaul = atoi(token);

    return wconfig;

fail:
    if (wconfig)
        free(wconfig);
    return NULL;
}

DECLARE_FUNC_UBUS_METHOD(_devSetConfig)
{
    struct blob_attr *o;
    unsigned rem;
    char *value, *name;
    struct wifi_config *config;
    uint8_t result = RESULT_INVALID_ARGUMENT;

    dlist_free_items(&local_config.wifi_config.bsses, struct wifi_config, l);

    blob_buf_init(&b, 0);

    blobmsg_for_each_attr(o, msg, rem) {
        if (blobmsg_type(o) != BLOBMSG_TYPE_STRING)
            continue;
        name = (char *)blobmsg_name(o);
        value = blobmsg_get_string(o);
        if (!strcmp(name, "program")) {
            if (strcmp(value, "map"))
                goto fail;
        } else if (!strncmp(name, "bss_info", 8)) {
            if (!(config = _parseWifiConfig(value)))
                goto fail;
            DEBUG_INFO("Add config for band %d\n", config->bands);
            printBSSConfig(&config->bss);
            dlist_add_tail(&local_config.wifi_config.bsses, &config->l);
        }
    }

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
fail:
    addResult(result);
    return ubus_send_reply(ctx, req, b.head);
}

static const struct ubus_method _sigma_obj_methods[] = {
    UBUS_METHOD("dev_send_1905", _devSend1905, _sigma_send_1905_attr_policy),
    UBUS_METHOD("dev_set_config", _devSetConfig, _sigma_set_config_attr_policy),
};

static struct ubus_object_type _sigma_obj_type =
    UBUS_OBJECT_TYPE(OBJ_NAME, _sigma_obj_methods);

DECLARE_UBUS_OBJ(_sigma_obj, OBJ_NAME, &_sigma_obj_type, _sigma_obj_methods);

struct ubus_object *getSigmaObj(void)
{
    return &_sigma_obj;
}

