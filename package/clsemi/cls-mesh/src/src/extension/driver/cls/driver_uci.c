#ifdef USE_UCI
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <uci.h>
#include <uci_blob.h>
#include <libubus.h>
#include <libubox/list.h>
#include <libubox/utils.h>
#include "extension.h"
#include "platform.h"
#include "platform_interfaces.h"
#include "datamodel.h"
#include "1905_tlvs.h"
#include "driver_uci.h"

struct uci_driver_ctx {
    struct uci_context *uci_ctx;
};

struct uci_wdev {
    dlist_item l;
    uint8_t band;
    char name[0];
};

enum {
    WIFI_DEVICE_ATTR_BAND,
    WIFI_DEVICE_ATTR_MAX
};

static const struct blobmsg_policy wifi_device_attrs[WIFI_DEVICE_ATTR_MAX] = {
    [WIFI_DEVICE_ATTR_BAND] = { .name = "band", .type = BLOBMSG_TYPE_STRING },
};

const struct uci_blob_param_list wifi_device_attr_list = {
    .n_params = WIFI_DEVICE_ATTR_MAX,
    .params = wifi_device_attrs,
};

enum {
    WIFI_IFACE_ATTR_DEVICE,
    WIFI_IFACE_ATTR_MODE,
    WIFI_IFACE_ATTR_DISABLED,
    WIFI_IFACE_ATTR_MULTI_AP,
    WIFI_IFACE_ATTR_HIDDEN,
    WIFI_IFACE_ATTR_SSID,
    WIFI_IFACE_ATTR_ENCRYPTION,
    WIFI_IFACE_ATTR_KEY,
    WIFI_IFACE_ATTR_VLAN,
    WIFI_IFACE_ATTR_MAX
};

static const struct blobmsg_policy wifi_iface_attrs[WIFI_IFACE_ATTR_MAX] = {
    [WIFI_IFACE_ATTR_DEVICE] = { .name = "device", .type = BLOBMSG_TYPE_STRING },
    [WIFI_IFACE_ATTR_MODE] = { .name = "mode", .type = BLOBMSG_TYPE_STRING },
    [WIFI_IFACE_ATTR_DISABLED] = { .name = "disabled", .type = BLOBMSG_TYPE_BOOL },
    [WIFI_IFACE_ATTR_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [WIFI_IFACE_ATTR_ENCRYPTION] = { .name = "encryption", .type = BLOBMSG_TYPE_STRING },
    [WIFI_IFACE_ATTR_KEY] = { .name = "key", .type = BLOBMSG_TYPE_STRING },
    [WIFI_IFACE_ATTR_MULTI_AP] = { .name = "multi_ap", .type = BLOBMSG_TYPE_INT32 },
    [WIFI_IFACE_ATTR_HIDDEN] = { .name = "hidden", .type = BLOBMSG_TYPE_BOOL },
    [WIFI_IFACE_ATTR_VLAN] = { .name = "brvlan", .type = BLOBMSG_TYPE_ARRAY },
};

const struct uci_blob_param_list wifi_iface_attr_list = {
    .n_params = WIFI_IFACE_ATTR_MAX,
    .params = wifi_iface_attrs,
};

struct encryption_map {
    const char	*uci;
    uint8_t	auth;
    uint8_t	encrypt;
};

#define UCI_ENCRYPTION_MAP(_uci, _auth, _encrypt) {.uci = _uci, .auth = _auth, .encrypt = _encrypt}
static struct encryption_map encryption_maps[] = {
    UCI_ENCRYPTION_MAP("none",              auth_open,                  encrypt_none),
    UCI_ENCRYPTION_MAP("sae",               auth_sae,                   encrypt_aes),
    UCI_ENCRYPTION_MAP("sae-mixed",         (auth_sae|auth_wpa2psk),    encrypt_aes),
    UCI_ENCRYPTION_MAP("psk2",              auth_wpa2psk,               encrypt_aes),
    UCI_ENCRYPTION_MAP("psk2+aes",          auth_wpa2psk,               encrypt_aes),
    UCI_ENCRYPTION_MAP("psk2+ccmp",         auth_wpa2psk,               encrypt_aes),
    UCI_ENCRYPTION_MAP("psk2+tkip",         auth_wpa2psk,               encrypt_tkip),
    UCI_ENCRYPTION_MAP("psk2+tkip+aes",     auth_wpa2psk,               (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("psk2+tkip+ccmp",    auth_wpa2psk,               (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("psk",               auth_wpapsk,                encrypt_aes),
    UCI_ENCRYPTION_MAP("psk+aes",           auth_wpapsk,                encrypt_aes),
    UCI_ENCRYPTION_MAP("psk+ccmp",          auth_wpapsk,                encrypt_aes),
    UCI_ENCRYPTION_MAP("psk+tkip",          auth_wpapsk,                encrypt_tkip),
    UCI_ENCRYPTION_MAP("psk+tkip+aes",      auth_wpapsk,                (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("psk+tkip+ccmp",     auth_wpapsk,                (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("psk-mixed",         (auth_wpapsk|auth_wpa2psk), encrypt_aes),
    UCI_ENCRYPTION_MAP("psk-mixed+aes",     (auth_wpapsk|auth_wpa2psk), encrypt_aes),
    UCI_ENCRYPTION_MAP("psk-mixed+ccmp",    (auth_wpapsk|auth_wpa2psk), encrypt_aes),
    UCI_ENCRYPTION_MAP("psk-mixed+tkip",    (auth_wpapsk|auth_wpa2psk), encrypt_tkip),
    UCI_ENCRYPTION_MAP("psk-mixed+tkip+aes", (auth_wpapsk|auth_wpa2psk), (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("psk-mixed+tkip+ccmp",(auth_wpapsk|auth_wpa2psk), (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wpa2",              auth_wpa2,                  encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa2+aes",          auth_wpa2,                  encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa2+ccmp",         auth_wpa2,                  encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa2+tkip",         auth_wpa2,                  encrypt_tkip),
    UCI_ENCRYPTION_MAP("wpa2+tkip+aes",     auth_wpa2,                  (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wpa2+tkip+ccmp",    auth_wpa2,                  (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wpa",               auth_wpa,                   encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa+aes",           auth_wpa,                   encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa+ccmp",          auth_wpa,                   encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa+tkip",          auth_wpa,                   encrypt_tkip),
    UCI_ENCRYPTION_MAP("wpa+tkip+aes",      auth_wpa,                   (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wpa+tkip+ccmp",     auth_wpa,                   (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wpa-mixed",         (auth_wpa|auth_wpa2),       encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa-mixed+aes",     (auth_wpa|auth_wpa2),       encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa-mixed+ccmp",    (auth_wpa|auth_wpa2),       encrypt_aes),
    UCI_ENCRYPTION_MAP("wpa-mixed+tkip",    (auth_wpa|auth_wpa2),       encrypt_tkip),
    UCI_ENCRYPTION_MAP("wpa-mixed+tkip+aes", (auth_wpa|auth_wpa2),      (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wpa-mixed+tkip+ccmp", (auth_wpa|auth_wpa2),     (encrypt_tkip|encrypt_aes)),
    UCI_ENCRYPTION_MAP("wep",               auth_open,                  encrypt_wep),
    UCI_ENCRYPTION_MAP("wep+open",          auth_open,                  encrypt_wep),
    UCI_ENCRYPTION_MAP("wep+shared",        auth_shared,                encrypt_wep),
};

enum {
    MESH_BASIC_ATTR_ENABLE,
    MESH_BASIC_ATTR_RCPI_THRESHOLD_LOW,
    MESH_BASIC_ATTR_RCPI_GAIN_LOW,
    MESH_BASIC_ATTR_RCPI_REPORT_INTERVAL,
    MESH_BASIC_ATTR_NEIGHBOR_RCPI_TIMEOUT,
    MESH_BASIC_ATTR_STEERING_TIMEOUT,
    MESH_BASIC_ATTR_MAX
};


static DEFINE_DLIST_HEAD(uci_wdevs);
static struct blob_buf b;


static const struct blobmsg_policy mesh_basic_attrs[MESH_BASIC_ATTR_MAX] = {
    [MESH_BASIC_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
    [MESH_BASIC_ATTR_RCPI_THRESHOLD_LOW] = { .name = "rcpi_threshold_low", .type = BLOBMSG_TYPE_INT32 },
    [MESH_BASIC_ATTR_RCPI_GAIN_LOW] = { .name = "rcpi_gain_low", .type = BLOBMSG_TYPE_INT32 },
    [MESH_BASIC_ATTR_RCPI_REPORT_INTERVAL] = { .name = "rcpi_report_interval", .type = BLOBMSG_TYPE_INT32 },
    [MESH_BASIC_ATTR_NEIGHBOR_RCPI_TIMEOUT] = { .name = "neighbor_rcpi_timeout", .type = BLOBMSG_TYPE_INT32 },
    [MESH_BASIC_ATTR_STEERING_TIMEOUT] = { .name = "steering_timeout", .type = BLOBMSG_TYPE_INT32 },
};

const struct uci_blob_param_list mesh_basic_attr_list = {
    .n_params = MESH_BASIC_ATTR_MAX,
    .params = mesh_basic_attrs,
};


static int parse_uci_encryption(const char *uci, uint8_t *auth, uint8_t *encrypt)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(encryption_maps); ++i) {
        if (!strcmp(encryption_maps[i].uci, uci)) {
            *auth = encryption_maps[i].auth;
            *encrypt = encryption_maps[i].encrypt;
            return 0;
        }
    }

    return -1;
}

const char *get_uci_encryption(uint8_t auth, uint8_t encrypt)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(encryption_maps); ++i) {
        if ((encryption_maps[i].auth == auth) && (encryption_maps[i].encrypt == encrypt))
            return encryption_maps[i].uci;
    }

    return NULL;
}

static struct uci_wdev *uci_find_wdev(const char *name)
{
    struct uci_wdev *wdev;

    dlist_for_each(wdev, uci_wdevs, l) {
        if (!strcmp(wdev->name, name))
            return wdev;
    }
    return NULL;
}

static int uciParseWdev(struct uci_section *s)
{
    struct blob_attr *tb[WIFI_DEVICE_ATTR_MAX] = {NULL}, *cur;
    struct uci_wdev *wdev;
    const char *name = s->e.name;
    const char *str;

    blob_buf_init(&b, 0);
    uci_to_blob(&b, s, &wifi_device_attr_list);
    blobmsg_parse(wifi_device_attrs, WIFI_DEVICE_ATTR_MAX, tb, blob_data(b.head), blob_len(b.head));

    wdev = calloc(1, sizeof(struct uci_wdev)+strlen(name)+1);
    if (!wdev)
        return -1;
    strcpy(wdev->name, name);

    cur = tb[WIFI_DEVICE_ATTR_BAND];
    if (cur)
        str = blobmsg_get_string(cur);
    else {
        DEBUG_WARNING("WARN: wifi-device '%s' don't have option 'band', treat band as 2.4G\n", name);
        str = "2g";
    }
    if (!strcmp(str, "2g"))
        wdev->band = band_2g;
    else if (!strcmp(str, "5g"))
        wdev->band = band_5g;
    else if (!strcmp(str, "6g"))
        wdev->band = band_6g;

    dlist_add_tail(&uci_wdevs, &wdev->l);
    return 0;
}

static int uciParseWintf(dlist_head *list, struct uci_section *s)
{
    struct blob_attr *tb[WIFI_IFACE_ATTR_MAX] = {NULL}, *cur;
    struct uci_wdev *wdev;
    struct wifi_config *wcfg;
    const char *str;

    blob_buf_init(&b, 0);
    uci_to_blob(&b, s, &wifi_iface_attr_list);
    blobmsg_parse(wifi_iface_attrs, WIFI_IFACE_ATTR_MAX, tb, blob_data(b.head), blob_len(b.head));

    cur = tb[WIFI_IFACE_ATTR_MODE];
    if ((!cur) ||
        ((strcmp(blobmsg_get_string(cur), "ap")) && (strcmp(blobmsg_get_string(cur), "sta"))))
        return 0;

    cur = tb[WIFI_IFACE_ATTR_DISABLED];
    if (cur && blobmsg_get_bool(cur))
        return 0;

    cur = tb[WIFI_IFACE_ATTR_DEVICE];
    if (!cur)
        return -1;
    wdev = uci_find_wdev(blobmsg_get_string(cur));
    if (!wdev)
        return -1;

    wcfg = calloc(1, sizeof(struct wifi_config));
    if (!wcfg)
        return -1;

    cur = tb[WIFI_IFACE_ATTR_MODE];
    if (!(strcmp(blobmsg_get_string(cur), "ap")))
        wcfg->bss.role = role_ap;
    else
        wcfg->bss.role = role_sta;

    memset(wcfg->bss.bssid, 0xff, MACLEN);
    wcfg->bands = wdev->band;
    cur = tb[WIFI_IFACE_ATTR_MULTI_AP];
    if (cur) {
        int multi_ap = blobmsg_get_u32(cur);
        if (wcfg->bss.role==role_ap) {
            wcfg->bss.backhaul  = !!(multi_ap & 0x01);
            wcfg->bss.fronthaul = !!(multi_ap & 0x02);
        } else {
            wcfg->bss.backhaul_sta = !!(multi_ap & 0x01);
        }
    }

    cur = tb[WIFI_IFACE_ATTR_HIDDEN];
    if (cur)
        wcfg->bss.hidden = blobmsg_get_bool(cur);

    cur = tb[WIFI_IFACE_ATTR_SSID];
    if (cur) {
        wcfg->bss.ssid.len = blobmsg_data_len(cur) - 1;
        if (wcfg->bss.ssid.len > MAX_SSID_LEN)
            wcfg->bss.ssid.len = MAX_SSID_LEN;
        memcpy(wcfg->bss.ssid.ssid, blobmsg_data(cur), wcfg->bss.ssid.len);
    }

    cur = tb[WIFI_IFACE_ATTR_KEY];
    if (cur) {
        wcfg->bss.key.len = blobmsg_data_len(cur) - 1;
        memcpy(wcfg->bss.key.key, blobmsg_data(cur), wcfg->bss.key.len);
    }
    cur = tb[WIFI_IFACE_ATTR_ENCRYPTION];
    if (cur)
        str = blobmsg_get_string(cur);
    else
        str = "none";

    if ((cur = tb[WIFI_IFACE_ATTR_VLAN])) {
        struct blob_attr *each;
        unsigned rem;
        int i = 0;
        int vlan;
        char *vlan_str;
        if (wcfg->bss.backhaul) {
            blobmsg_for_each_attr(each, cur, rem) {
                if (((vlan_str = blobmsg_get_string(each)))
                    && (sscanf(vlan_str, "%d:t", &vlan)==1))
                    wcfg->bss.vlan_map[i++] = vlan;
            }
        } else {
            blobmsg_for_each_attr(each, cur, rem) {
                if (((vlan_str = blobmsg_get_string(each)))
                    && (sscanf(vlan_str, "%d:u", &vlan)==1)) {
                    wcfg->bss.vlan_map[0] = vlan;
                    break;
                }
            }
        }

    }
    parse_uci_encryption(str, &wcfg->bss.auth, &wcfg->bss.encrypt);

    dlist_add_tail(list, &wcfg->l);

    return 0;
}

int uciGetWifiConfig(void *data, dlist_head *bsses)
{
    struct uci_driver_ctx *ctx = (struct uci_driver_ctx *)data;
    struct uci_package *wireless = NULL;
    struct uci_section *s;
    struct uci_element *e;

    if (uci_load(ctx->uci_ctx, "wireless", &wireless) != UCI_OK)
        return -1;

    uci_foreach_element(&wireless->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "wifi-device")) {
            if (uciParseWdev(s) < 0)
                continue;
        }
    }

    uci_foreach_element(&wireless->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "wifi-iface"))
            uciParseWintf(bsses, s);
    }

    uci_unload(ctx->uci_ctx, wireless);
    return 0;
}

static int uciParseRoamingPolicy(struct uci_context *ctx, struct uci_section *s, struct map_policy *policy)
{
    const char *t_s;

    if ((t_s = uci_lookup_option_string(ctx, s, "rcpi_steer"))) {
        policy->roaming_policy.rcpi_steer = (atoi(t_s)==1 ? 1 : 0);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "band_steer"))) {
        policy->roaming_policy.band_steer = (atoi(t_s)==1 ? 1 : 0);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "band_steer_high"))) {
        policy->roaming_policy.rssi_high = atoi(t_s);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "band_steer_low"))) {
        policy->roaming_policy.rssi_low = atoi(t_s);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "rcpi_steer_thresh"))) {
        policy->roaming_policy.rcpi_thresh = atoi(t_s);
        /* fix me: report threshhold set as same */
        for (int i = 0; i < band_max_idx; i++)
            policy->metrics_rpt[i].rpt_policy.sta_rcpi_thresh = atoi(t_s);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "rssi_gain_of_5g"))) {
        policy->roaming_policy.rssi_gain_of_5g = atoi(t_s);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "rssi_gain_thresh"))) {
        policy->roaming_policy.rssi_gain_thresh = atoi(t_s);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "steer_cooldown"))) {
        policy->roaming_policy.cooldown = atoi(t_s);
    }
    return 0;
}

static int uciParseQos(struct uci_context *ctx, struct uci_section *s, struct map_policy *policy)
{
    const char *t_s;

    if ((t_s = uci_lookup_option_string(ctx, s, "dscp2up"))) {
        if (string2Hex((char *)t_s, local_policy.dscp2up_table, DSCP2UP_SIZE)==DSCP2UP_SIZE)
            local_policy.dscp2up_set = 1;
    }
    return 0;
}

static int uciParseVIP(struct uci_context *ctx, struct uci_section *s, struct map_policy *policy)
{
    struct uci_option *o;
    if ((o = uci_lookup_option(ctx, s, "vip_sta"))) {
        if (o->type==UCI_TYPE_LIST) {
            struct uci_element *e;
            uci_foreach_element(&o->v.list, e) {
                struct mac_item *vip = calloc(1, sizeof(struct mac_item));
                if (vip) {
                   ether_aton_r(e->name, (struct ether_addr *)vip->mac);
                   dlist_add_tail(&policy->vips, &vip->l);
                }
            }
        }
    }
    return 0;
}

static int uciParseVbss(struct uci_context *ctx, struct uci_section *s, struct map_policy *policy)
{
    const char *t_s;

    if ((t_s = uci_lookup_option_string(ctx, s, "enable"))) {
        if (atoi(t_s)==1)
            policy->vbss_conf.enable = 1;
        else
            policy->vbss_conf.enable = 0;
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "ssid"))) {
        policy->vbss_conf.vbss_ssid.len = strlen(t_s);
        memcpy(policy->vbss_conf.vbss_ssid.ssid, t_s, policy->vbss_conf.vbss_ssid.len + 1);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "encryption")))
        parse_uci_encryption(t_s, &policy->vbss_conf.auth, &policy->vbss_conf.encrypt);
    else
        parse_uci_encryption("none", &policy->vbss_conf.auth, &policy->vbss_conf.encrypt);
    if ((t_s = uci_lookup_option_string(ctx, s, "key"))) {
        int key_len = strlen(t_s);
        if (key_len > 0 && key_len < MAX_KEY_LEN) {
            policy->vbss_conf.k.len = strlen(t_s);
            memcpy(policy->vbss_conf.k.key, t_s, key_len + 1);
        }
    }

    return 0;
}

static int uciParseDebugConfig(struct uci_context *ctx, struct uci_section *s, struct map_config *config)
{
    const char *t_s;

    if ((t_s = uci_lookup_option_string(ctx, s, "level"))) {
        int level = atoi(t_s);
        DEBUG_SET(debug_param_level, level);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "syslog"))) {
        if (atoi(t_s))
            DEBUG_SET(debug_param_syslog, 1);
    } else if ((t_s = uci_lookup_option_string(ctx, s, "file"))) {
        DEBUG_SET_LOGFILE((char *)t_s);
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "flush"))) {
        DEBUG_SET(debug_param_flush, atoi(t_s));
    }
    return 0;
}

static int uciParseFeatConfig(struct uci_context *ctx, struct uci_section *s, struct map_config *config)
{
    const char *t_s;

    if ((t_s = uci_lookup_option_string(ctx, s, "sync_sta"))) {
        if (atoi(t_s))
            local_config.sync_sta = 1;
    }
    if ((t_s = uci_lookup_option_string(ctx, s, "wfa"))) {
        if (atoi(t_s))
            local_config.wfa_mode = 1;
    }
    return 0;
}

int uciGetConfig(void *data, struct map_config *config)
{
    struct uci_driver_ctx *ctx = (struct uci_driver_ctx *)data;
    struct uci_package *p = NULL;
    struct uci_section *s;
    struct uci_element *e;
    if (uci_load(ctx->uci_ctx, "cls-mesh", &p) != UCI_OK)
        return -1;

    uci_foreach_element(&p->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "debug")) {
            uciParseDebugConfig(ctx->uci_ctx, s, config);

        } else if (!strcmp(s->type, "feature")) {
            uciParseFeatConfig(ctx->uci_ctx, s, config);
        }
    }
    uci_unload(ctx->uci_ctx, p);

    return 0;
}

int uciGetPolicy(void *data, struct map_policy *policy)
{
    struct uci_driver_ctx *ctx = (struct uci_driver_ctx *)data;
    struct uci_package *p = NULL;
    struct uci_section *s;
    struct uci_element *e;
    bool cls_vip_exist = false;

    if (uci_load(ctx->uci_ctx, "cls-mesh", &p) != UCI_OK)
        return -1;

    uci_foreach_element(&p->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "roaming_policy")) {
            uciParseRoamingPolicy(ctx->uci_ctx, s, policy);
        } else if (!strcmp(s->type, "qos")) {
            uciParseQos(ctx->uci_ctx, s, policy);
        } else if (!strcmp(s->type, "vbss")) {
            uciParseVbss(ctx->uci_ctx, s, policy);
        }
    }

    uci_unload(ctx->uci_ctx, p);

    if (uci_load(ctx->uci_ctx, "cls-qos", &p) != UCI_OK)
        return -1;

    uci_foreach_element(&p->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "vip_qos")) { /* this section is existed when vip sta is existed */
            if (!dlist_empty(&policy->vips))
                dlist_free_items(&policy->vips, struct mac_item, l);
            uciParseVIP(ctx->uci_ctx, s, policy);
            cls_vip_exist = true;
        }
    }
    /* clear vip list when no vip sta in cls-qos */
    if (false == cls_vip_exist && !dlist_empty(&policy->vips))
        dlist_free_items(&policy->vips, struct mac_item, l);

    uci_unload(ctx->uci_ctx, p);

    return 0;
}
static int uciParseBrVlan(struct uci_context *ctx, struct uci_section *s, struct map_policy *policy)
{
    const char *t_s;
    int vlan;

    if ((t_s = uci_lookup_option_string(ctx, s, "device"))) {
        if (strcmp(t_s, "br-lan"))
            return 0;
    } else
        return 0;

    if ((t_s = uci_lookup_option_string(ctx, s, "vlan"))) {
        if ((vlan=atoi(t_s))<=0)
            return 0;
    } else
        return 0;

    if ((t_s = uci_lookup_option_string(ctx, s, "primary")) && (atoi(t_s)==1)) {
        policy->def_vlan = vlan;
    } else {
        struct vlan_config_item *vlan_item = calloc(1, sizeof(struct vlan_config_item));

        vlan_item->vlan = vlan;
        dlist_add_tail(&policy->vlans, &vlan_item->l2.l);
    }
    return 0;
}


int uciGetVlan(void *data, struct map_policy *policy)
{
    struct uci_driver_ctx *ctx = (struct uci_driver_ctx *)data;
    struct uci_package *p = NULL;
    struct uci_section *s;
    struct uci_element *e;

    if (uci_load(ctx->uci_ctx, "network", &p) != UCI_OK)
        return -1;

    uci_foreach_element(&p->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "bridge-vlan")) {
            uciParseBrVlan(ctx->uci_ctx, s, policy);
        }
    }

    uci_unload(ctx->uci_ctx, p);

    return 0;
}

static void getBoardCb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct device_info *info = (struct device_info *)req->priv;
    struct blob_attr *cur;
    int rem;

    blobmsg_for_each_attr(cur, msg, rem) {
        if (!strcmp(blobmsg_name(cur), "model"))
            REPLACE_STR(info->model_name, strdup(blobmsg_get_string(cur)));
    }
}

static void getInfoCb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct device_info *info = (struct device_info *)req->priv;
    struct blob_attr *cur;
    int rem;

    blobmsg_for_each_attr(cur, msg, rem) {
        if (!strcmp(blobmsg_name(cur), "uptime")) {
            info->start_time = blobmsg_get_u32(cur) - PLATFORM_GET_TIMESTAMP(0)/1000;
        }
    }
}

static int ubusGetDeviceInfo(void *data, struct device_info *info)
{
    struct ubus_context *ubus;
    uint32_t id;

    if ((!info) || (!(ubus = PLATFORM_GET_UBUS()))
        || (ubus_lookup_id(ubus, "system", &id)))
        return -1;

    ubus_invoke(ubus, id, "board", NULL, getBoardCb, info, UBUS_INVOKE_DEFAULT_TIMEOUT);
    ubus_invoke(ubus, id, "info", NULL, getInfoCb, info, UBUS_INVOKE_DEFAULT_TIMEOUT);

    return 0;
}

static void getIpCb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    dlist_head *ips = (dlist_head *)req->priv;
    int rem, rem1, rem2;
    char *str;
    struct interface_ip_item *interface_ip_item;
    uint8_t protocol = 0;
    struct blob_attr *cur, *cur1, *cur2, *proto=NULL, *device=NULL, *ipv4=NULL, *ipv6=NULL;
    struct blob_attr *ipv6_assign=NULL;
    uint8_t interface_mac[MACLEN];

    blobmsg_for_each_attr(cur, msg, rem) {
        if (!strcmp(blobmsg_name(cur), "proto"))
            proto = cur;
        else if (!strcmp(blobmsg_name(cur), "device"))
            device = cur;
        else if (!strcmp(blobmsg_name(cur), "ipv4-address"))
            ipv4 = cur;
        else if (!strcmp(blobmsg_name(cur), "ipv6-address"))
            ipv6 = cur;
        else if (!strcmp(blobmsg_name(cur), "ipv6-prefix-assignment"))
            ipv6_assign = cur;
    }

    if ((!proto) || (!device))
        return;

    if (getInterfaceMac(blobmsg_get_string(device), interface_mac))
        return;

    if (!strcmp(str = blobmsg_get_string(proto), "static"))
        protocol = IP_PROTO_STATIC;
    else if (!strcmp(str, "dhcp"))
        protocol = IP_PROTO_DHCP;

    interface_ip_item = malloc(sizeof(struct interface_ip_item));
    MACCPY(interface_ip_item->intf_mac, interface_mac);
    dlist_head_init(&interface_ip_item->ipv4s);
    dlist_head_init(&interface_ip_item->ipv6s);

    if (ipv4) {
        struct ipv4_item *ip_item = malloc(sizeof(struct ipv4_item));
        blobmsg_for_each_attr(cur, ipv4, rem) {
            blobmsg_for_each_attr(cur1, cur, rem1) {
                if (!strcmp(blobmsg_name(cur1), "address")) {
                    if (inet_pton(AF_INET, blobmsg_get_string(cur1), &ip_item->ip)==1) {
                        ip_item->proto = protocol;
                        dlist_add_tail(&interface_ip_item->ipv4s, &ip_item->l);

                        ip_item = malloc(sizeof(struct ipv4_item));
                    }
                }
            }
        }
        free(ip_item);
    }
    if (ipv6) {
        struct ipv6_item *ip_item = malloc(sizeof(struct ipv6_item));
        blobmsg_for_each_attr(cur, ipv6, rem) {
            blobmsg_for_each_attr(cur1, cur, rem1) {
                if (!strcmp(blobmsg_name(cur1), "address")) {
                    if (inet_pton(AF_INET6, blobmsg_get_string(cur1), &ip_item->ip)==1) {
                        ip_item->proto = protocol;
                        dlist_add_tail(&interface_ip_item->ipv6s, &ip_item->l);

                        ip_item = malloc(sizeof(struct ipv6_item));
                    }
                }
            }
        }
        free(ip_item);
    }
    if (ipv6_assign) {
        blobmsg_for_each_attr(cur, ipv6_assign, rem) {
            blobmsg_for_each_attr(cur1, cur, rem1) {
                if (!strcmp(blobmsg_name(cur1), "local-address")) {
                    blobmsg_for_each_attr(cur2, cur1, rem2) {
                        if (!strcmp(blobmsg_name(cur2), "address")) {
                            inet_pton(AF_INET6, blobmsg_get_string(cur2), &interface_ip_item->local_ipv6);
                        }
                    }
                }
            }
        }
    }
    dlist_add_tail(ips, &interface_ip_item->l);
}

static int ubusGetIpInfo(void *data, dlist_head *ips)
{
    struct ubus_context *ubus;
    uint32_t id;

    if ((!ips) || (!(ubus = PLATFORM_GET_UBUS()))
        || (ubus_lookup_id(ubus, "network.interface.lan", &id)))
        return -1;

    ubus_invoke(ubus, id, "status", NULL, getIpCb, ips, UBUS_INVOKE_DEFAULT_TIMEOUT);

    return 0;
}

static struct configurator_ops uci_configurator_ops = {
    .getWifiConfig = uciGetWifiConfig,
    .getPolicy = uciGetPolicy,
    .getConfig = uciGetConfig,
    .getDeviceInfo = ubusGetDeviceInfo,
    .getIpInfo = ubusGetIpInfo,
    .getVlan = uciGetVlan,
};

static void *uciDriverInit()
{
    struct uci_driver_ctx *ctx;

    ctx = calloc(1, sizeof(struct uci_driver_ctx));
    if (ctx) {
        ctx->uci_ctx = uci_alloc_context();
        if (!ctx->uci_ctx) {
            DEBUG_ERROR("failed to uci_alloc_context()\n");
            goto fail;
        }

        configuratorAdd(&uci_configurator_ops, ctx);
    }
    return ctx;
fail:
    if (ctx) {
        free(ctx);
    }
    return NULL;
}

static struct extension_ops uci_driver_ops = {
    .init = uciDriverInit,
};


void uciDriverLoad()
{
    registerExtension(&uci_driver_ops);
}

#endif
