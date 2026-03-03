#include <netinet/ether.h>
#include <arpa/inet.h>
#include "platform.h"
#include "datamodel.h"
#include "feature/feature_helper.h"
#include "feature/ubus/ubus_helper.h"
#include "feature/steering/record.h"
#include "driver/cls/nl80211_cls.h"
#include "wifi.h"
#include "al_driver.h"

#define OBJ_NAME "clmesh.api"

struct dev_desc {
    dlist_item l;
    uint8_t al_mac[MACLEN];
    uint8_t up_mac[MACLEN];
};

static struct ubus_context *ubus_ctx = NULL;
static struct blob_buf b_local;

static void _addDevDesc(dlist_head *devs, uint8_t *mac, struct al_device *up)
{
    struct dev_desc *d_desc = calloc(1, sizeof(struct dev_desc));

    if (d_desc) {
        MACCPY(d_desc->al_mac, mac);
        if (up)
            MACCPY(d_desc->up_mac, up->al_mac);
        dlist_add_tail(devs, &d_desc->l);
    }
}

static struct dev_desc *_findDevDesc(dlist_head *devs, uint8_t *mac)
{
    struct dev_desc *ret = NULL;

    dlist_for_each(ret, *devs, l) {
        if (!MACCMP(mac, ret->al_mac))
            return ret;
    }

    return ret;
}


static void _fillDevice(dlist_head *devs, struct al_device *d, struct al_device *up_d);
static void _fillRadio(struct radio *r)
{
    void *t;

    t = blobmsg_open_table(&b, NULL);
    blobmsgAddMacStr(&b, "rid", r->uid);
    blobmsg_add_u32(&b, "bw", r->bw);
    blobmsg_add_u32(&b, "channel", r->channel);
    blobmsg_close_table(&b, t);
}

static void clsqos_get_sta_status_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *v;
    int rem;

    blob_for_each_attr(v, msg, rem)
        blobmsg_add_blob(&b, v);
}

static void _fillLeaf(uint8_t *mac)
{
    uint32_t id;

    if (ubus_lookup_id(ubus_ctx, "clsqos", &id))
        return;

    blob_buf_init(&b_local, 0);
    blobmsgAddMacStr(&b_local, "mac", mac);

    ubus_invoke(ubus_ctx, id, "get_sta_status", b_local.head, clsqos_get_sta_status_cb, NULL, 3000);

    blob_buf_free(&b_local);
}

static void _fillClient(struct client *client)
{
    void *t = blobmsg_open_table(&b, NULL);

    blobmsgAddMacStr(&b, "mac", client->mac);
    _fillLeaf(client->mac);

    blobmsg_add_u32(&b, "ul_rate", client->link_metrics.mac_rate_ul);
    blobmsg_add_u32(&b, "dl_rate", client->link_metrics.mac_rate_dl);
    blobmsg_add_u32(&b, "rssi", rcpi2Rssi(client->link_metrics.rcpi_ul));
    blobmsg_add_u32(&b, "age", (PLATFORM_GET_TIMESTAMP(0)-client->last_assoc_ts)/1000);

    blobmsg_close_table(&b, t);
}


static uint8_t *_getInterfaceMac(struct al_device *d, uint8_t *nei_mac)
{
    struct interface *intf;

    dlist_for_each(intf, d->interfaces, l) {
        struct neighbor *nei;
        dlist_for_each(nei, intf->neighbors, l) {
            if (!MACCMP(nei->al_mac, nei_mac))
                return intf->mac;
        }
    }
    return NULL;
}

static void _fillInterface(dlist_head *devs, struct al_device *d,
                            struct al_device *up_d, struct interface *intf)
{
    void *t;

    t = blobmsg_open_table(&b, NULL);
    blobmsgAddMacStr(&b, "mac", intf->mac);

    if (intf->type == interface_type_ethernet) {
        blobmsg_add_string(&b, "type", "ethernet");
    } else if (intf->type == interface_type_wifi) {
        struct wifi_interface *wif = (struct wifi_interface *)intf;
        blobmsg_add_string(&b, "type", "wifi");
        if (wif->radio)
            blobmsgAddMacStr(&b, "rid", wif->radio->uid);
        blobmsg_add_string(&b, "ssid", (const char *)wif->bssInfo.ssid.ssid);

        if (dlist_count(&wif->clients)) {
            void *a = blobmsg_open_array(&b, "clients");
            struct client *client;

            dlist_for_each(client, wif->clients, l) {
                _fillClient(client);
            }
            blobmsg_close_array(&b, a);
        }

    }
    if (!dlist_empty(&intf->neighbors)) {
        struct neighbor *nei;
        void *a = blobmsg_open_array(&b, "peers");

        dlist_for_each(nei, intf->neighbors, l) {
            struct al_device *peer;
            if ((up_d) && ((_getInterfaceMac(up_d, nei->al_mac)) || (!MACCMP(nei->al_mac, up_d->al_mac))))
                continue;
            if (_findDevDesc(devs, nei->al_mac)) {
                DEBUG_WARNING("potential loop from "MACFMT" to "MACFMT"\n", MACARG(d->al_mac), MACARG(nei->al_mac));
                break;
            }

            if ((peer = alDeviceFind(nei->al_mac))) {
                void *t2;
                t2 = blobmsg_open_table(&b, NULL);
                _fillDevice(devs, peer, d);
                blobmsg_close_table(&b, t2);
            } else {
                void *t = blobmsg_open_table(&b, NULL);
                blobmsgAddMacStr(&b, "al_mac", nei->al_mac);
                _fillLeaf(nei->al_mac);
                blobmsg_close_table(&b, t);
            }
        }

        blobmsg_close_array(&b, a);
    }
    blobmsg_close_table(&b, t);
}

static void _fillBackhaul(struct al_device *d, struct al_device *up_d)
{
    struct interface *intf;
    struct neighbor *neigh = NULL, *remote_neigh = NULL;
    void *t;

    dlist_for_each(intf, up_d->interfaces, l) {
        if ((remote_neigh = neighborFind(intf, d->al_mac, NULL)))
            break;
    }
    dlist_for_each(intf, d->interfaces, l) {
        if ((neigh = neighborFind(intf, up_d->al_mac, NULL))) {
            break;
        }
    }

    //no record, give up
    if (!neigh) return;

    t = blobmsg_open_table(&b, "backhaul");
    blobmsgAddMacStr(&b, "uplink", up_d->al_mac);
    if (intf->type == interface_type_wifi) {
        struct wifi_interface *wintf = (struct wifi_interface *)intf;
        blobmsg_add_string(&b, "type", "wifi");
        if (wintf->radio) {
            if (wintf->radio->current_band_idx == band_2g_idx) {
                blobmsg_add_string(&b, "band", "2G");
            } else if (wintf->radio->current_band_idx == band_5g_idx) {
                blobmsg_add_string(&b, "band", "5G");
            } else if (wintf->radio->current_band_idx == band_6g_idx) {
                blobmsg_add_string(&b, "band", "6G");
            }
        }

        blobmsg_add_string(&b, "ssid", (char *)wintf->bssInfo.ssid.ssid);
        blobmsg_add_string(&b, "password", (char *)wintf->bssInfo.key.key);
        blobmsg_add_u32(&b, "auth", wintf->bssInfo.auth);
        blobmsg_add_u32(&b, "encryption", wintf->bssInfo.encrypt);
    } else if (intf->type == interface_type_ethernet) {
        blobmsg_add_string(&b, "type", "ethernet");
    }

    blobmsg_add_u32(&b, "tx_rate", neigh->link_metric.tx_link_metrics.tx_phy_rate);
    blobmsg_add_u32(&b, "rx_rate",
        (remote_neigh != NULL) ? remote_neigh->link_metric.tx_link_metrics.tx_phy_rate : 0);

    blobmsg_close_table(&b, t);
}

static struct ipv4 *_getIpv4(struct al_device *d)
{
    struct ipv4 *ip = NULL;
    uint8_t proto = 0;

    struct interface *intf;

    dlist_for_each(intf, d->interfaces, l) {
        struct ipv4_item *ip_item;
        dlist_for_each(ip_item, intf->ipv4s, l) {
            if ((!ip) || (proto<ip_item->proto)) {
                ip = &ip_item->ip;
                proto = ip_item->proto;
            }
        }
    }
    return ip;
}

static struct ipv6 *_getIpv6(struct al_device *d)
{
    struct ipv6 *ip = NULL;
    uint8_t proto = 0;

    struct interface *intf;

    dlist_for_each(intf, d->interfaces, l) {
        struct ipv6_item *ip_item;
        dlist_for_each(ip_item, intf->ipv6s, l) {
            if ((!ip) || (proto<ip_item->proto)) {
                ip = &ip_item->ip;
                proto = ip_item->proto;
            }
        }
    }
    return ip;
}

static void _fillDevice(dlist_head *devs, struct al_device *d, struct al_device *up_d)
{
    void *a;
    struct ipv6 *ipv6;
    struct ipv4 *ipv4;
    char str[INET6_ADDRSTRLEN+2];

    _addDevDesc(devs, d->al_mac, up_d);

    DEBUG_INFO("fill device "MACFMT"\n", MACARG(d->al_mac));

    if (d == local_device) {
        dmUpdateIPInfo();
    }

    blobmsgAddMacStr(&b, "al_mac", d->al_mac);

    blobmsg_add_u32(&b, "profile", d->profile);

    blobmsg_add_u32(&b, "uptime", (PLATFORM_GET_TIMESTAMP(0)/1000+d->device_info.start_time));

    a = blobmsg_open_array(&b, "service");
    if (d->is_controller)
        blobmsg_add_string(&b, NULL, "controller");
    if (d->is_agent)
        blobmsg_add_string(&b, NULL, "agent");
    blobmsg_close_array(&b, a);

    if (local_device->is_controller) {
        if ((d==local_device) || (d->status == STATUS_CONFIGURED))
            blobmsg_add_string(&b, "status", "configured");
        else
            blobmsg_add_string(&b, "status", "unconfigured");
    }

    if ((ipv4 = _getIpv4(d))) {
        blobmsg_add_string(&b, "ipv4", inet_ntop(AF_INET, ipv4, str, sizeof(str)));
    }
    if ((ipv6 = _getIpv6(d))) {
        blobmsg_add_string(&b, "ipv6", inet_ntop(AF_INET6, ipv6, str, sizeof(str)));
    }

    if (d->device_info.manufacturer)
        blobmsg_add_string(&b, "manufacturer", d->device_info.manufacturer);
    if (d->device_info.model_name)
        blobmsg_add_string(&b, "model", d->device_info.model_name);

    if (up_d) {
        _fillBackhaul(d, up_d);
    }

    if (!dlist_empty(&d->radios)) {
        struct radio *r;
        a = blobmsg_open_array(&b, "radios");
        dlist_for_each(r, d->radios, l) {
            _fillRadio(r);
        }
        blobmsg_close_array(&b, a);
    }
    if (!dlist_empty(&d->interfaces)) {
        struct interface *intf;
        a = blobmsg_open_array(&b, "interfaces");
        dlist_for_each(intf, d->interfaces, l) {
            _fillInterface(devs, d, up_d, intf);
        }
        blobmsg_close_array(&b, a);
    }
}

DECLARE_FUNC_UBUS_METHOD(_topology)
{
    void *t;
    struct al_device *d;
    DEFINE_DLIST_HEAD(devs);

    ubus_ctx = ctx;

    if (!(d = registrar))
        d = local_device;

    blob_buf_init(&b, 0);

    t = blobmsg_open_table(&b, "network");
    _fillDevice(&devs, d, NULL);
    blobmsg_close_table(&b, t);

    dlist_free_items(&devs, struct dev_desc, l);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
}

enum {
    WPS_POLICY_ATTR_IFNAME = 0,

    NUM_WPS_POLICY_ATTRS,
};

static const struct blobmsg_policy _wps_policy[] = {
    POLICY_ATTR(WPS_POLICY_ATTR_IFNAME, "ifname", BLOBMSG_TYPE_STRING),
};

DECLARE_FUNC_UBUS_METHOD(_startWPS)
{
    struct blob_attr *tb[NUM_WPS_POLICY_ATTRS];
    struct interface *intf;
    char *intf_name = NULL;
    int role;
    struct wifi_interface *bsta_intf =NULL, *fronthaul_2g=NULL, *fronthaul_5g=NULL, *wpsintf=NULL;;

    blobmsg_parse(_wps_policy, NUM_WPS_POLICY_ATTRS, tb,
                    blobmsg_data(msg), blobmsg_len(msg));

    if (tb[WPS_POLICY_ATTR_IFNAME]) {
        intf_name = blobmsg_get_string(tb[WPS_POLICY_ATTR_IFNAME]);
        if ((role = nl80211GetInterfaceMode(intf_name))>=0) {
            bssStartWPS(if_nametoindex(intf_name), role, intf_name);
        }
    } else {
        dlist_for_each(intf, local_device->interfaces, l) {
            if (intf->type == interface_type_wifi) {
                struct wifi_interface *wintf = (struct wifi_interface *)intf;
                if (wintf->role == role_sta) {
                    if (wintf->bssInfo.backhaul_sta)
                        bsta_intf = wintf;
                } else if (wintf->role==role_ap) {
                    if ((wintf->radio) && (wintf->radio->bands & band_2g) && (wintf->bssInfo.fronthaul)) {
                        fronthaul_2g = wintf;
                    }
                    if ((wintf->radio) && (wintf->radio->bands & band_5g) && (wintf->bssInfo.fronthaul)) {
                        fronthaul_5g = wintf;
                    }
                }
            }
        }

        if ((bsta_intf) && IS_ZERO_MAC(bsta_intf->bssInfo.bssid)) {
            wpsintf = bsta_intf;
        } else if (fronthaul_5g) {
            wpsintf = fronthaul_5g;
        } else if (fronthaul_2g) {
            wpsintf = fronthaul_2g;;
        }
        bssStartWPS(wpsintf->i.index, wpsintf->role, wpsintf->i.name);
    }
    blob_buf_init(&b, 0);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
}

enum {
    METRICS_RPT_POLICY_ATTR_BAND_IDX = 0,
    METRICS_RPT_POLICY_ATTR_RCPI_THRESH,
    METRICS_RPT_POLICY_ATTR_RCPI_MARGIN,
    METRICS_RPT_POLICY_ATTR_CHUTIL_THRESH,
    METRICS_RPT_POLICY_ATTR_INCLUSION_POLICY,

    NUM_METRICS_RPT_POLICY_ATTRS,
};

static const struct blobmsg_policy rpt_policy[] = {
    POLICY_ATTR(METRICS_RPT_POLICY_ATTR_BAND_IDX, "band_idx", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(METRICS_RPT_POLICY_ATTR_RCPI_THRESH, "rcpi_thresh", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(METRICS_RPT_POLICY_ATTR_RCPI_MARGIN, "rcpi_margin", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(METRICS_RPT_POLICY_ATTR_CHUTIL_THRESH, "ch_util_thresh", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(METRICS_RPT_POLICY_ATTR_INCLUSION_POLICY, "inclusion_policy", BLOBMSG_TYPE_INT32),
};

enum {
    STEER_POLICY_ATTR_RADIO_UID = 0,
    STEER_POLICY_ATTR_MODE,
    STEER_POLICY_ATTR_CHUTIL_THRESH,
    STEER_POLICY_ATTR_RCPI_THRESH,

    NUM_STEER_POLICY_ATTRS,
};

static const struct blobmsg_policy steer_policy_policy[] = {
    POLICY_ATTR(STEER_POLICY_ATTR_RADIO_UID, "rid", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(STEER_POLICY_ATTR_MODE, "mode", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(STEER_POLICY_ATTR_CHUTIL_THRESH, "ch_util_thresh", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(STEER_POLICY_ATTR_RCPI_THRESH, "rcpi_thresh", BLOBMSG_TYPE_INT32),
};

enum {
    BAND_STEER_ATTR_ENABLE = 0,
    BAND_STEER_ATTR_RSSI_LOW,
    BAND_STEER_ATTR_RSSI_HIGH,
    BAND_STEER_ATTR_COOLDOWN,

    NUM_BAND_STEER_ATTRS,
};

static const struct blobmsg_policy band_steer_policy[] = {
    POLICY_ATTR(BAND_STEER_ATTR_ENABLE, "enable", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(BAND_STEER_ATTR_RSSI_LOW, "low", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(BAND_STEER_ATTR_RSSI_HIGH, "high", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(BAND_STEER_ATTR_COOLDOWN, "cooldown", BLOBMSG_TYPE_INT32),
};

enum {
    POLICY_CONF_ATTR_LOCAL_DISALLOW = 0,
    POLICY_CONF_ATTR_BTM_DISALLOW,
    POLICY_CONF_ATTR_STEER_POLICY,
    POLICY_CONF_ATTR_METRICS_RPT_INTVL,
    POLICY_CONF_ATTR_METRICS_RPT_POLICY,
    POLICY_CONF_ATTR_BAND_STEER,

    NUM_POLICY_CONFIG_ATTRS,
};

static const struct blobmsg_policy policy_conf_policy[] = {
    POLICY_ATTR(POLICY_CONF_ATTR_LOCAL_DISALLOW, "local_disallow", BLOBMSG_TYPE_ARRAY ),
    POLICY_ATTR(POLICY_CONF_ATTR_BTM_DISALLOW, "btm_disallow", BLOBMSG_TYPE_ARRAY ),
    POLICY_ATTR(POLICY_CONF_ATTR_STEER_POLICY, "steer_policy", BLOBMSG_TYPE_ARRAY ),
    POLICY_ATTR(POLICY_CONF_ATTR_METRICS_RPT_INTVL, "metrics_rpt_intvl", BLOBMSG_TYPE_INT32 ),
    POLICY_ATTR(POLICY_CONF_ATTR_METRICS_RPT_POLICY, "metrics_rpt_policy", BLOBMSG_TYPE_ARRAY ),
    POLICY_ATTR(POLICY_CONF_ATTR_BAND_STEER, "band_steer", BLOBMSG_TYPE_TABLE ),
};


enum {
    PARAM_ATTR_MAC = 0,
    NUM_PARAM_MAC_ATTRS,
};

static const struct blobmsg_policy _mac_list_policy[] = {
    POLICY_ATTR(PARAM_ATTR_MAC, "mac", BLOBMSG_TYPE_STRING ),
};

void _mac_array_to_dlist(void *ctx, void *param, void *data, int len)
{
    dlist_head *head = (dlist_head *)ctx;
    struct mac_item *item;
    struct blob_attr *tb[NUM_PARAM_MAC_ATTRS];

    blobmsg_parse(_mac_list_policy, NUM_PARAM_MAC_ATTRS, tb, data, len);

    if (!tb[PARAM_ATTR_MAC])
        return;

    item = calloc(1, sizeof(struct mac_item));
    blobmsg_get_mac(tb[PARAM_ATTR_MAC], item->mac);
    dlist_add_tail(head, &item->l);
}

static int _set_map_policy(struct map_policy *policy, struct blob_attr *conf_policy)
{
    struct blob_attr *tb[NUM_POLICY_CONFIG_ATTRS];

    blobmsg_parse(policy_conf_policy, NUM_POLICY_CONFIG_ATTRS, tb,
                    blobmsg_data(conf_policy), blobmsg_len(conf_policy));

    if (tb[POLICY_CONF_ATTR_LOCAL_DISALLOW])
        visit_attrs(tb[POLICY_CONF_ATTR_LOCAL_DISALLOW], _mac_array_to_dlist,
                    &policy->stalist_local_steer_disallow, NULL);

    if (tb[POLICY_CONF_ATTR_BTM_DISALLOW])
        visit_attrs(tb[POLICY_CONF_ATTR_BTM_DISALLOW], _mac_array_to_dlist,
                    &policy->stalist_btm_steer_disallow, NULL);

    if (tb[POLICY_CONF_ATTR_BAND_STEER]) {
        struct blob_attr *sub_tb[NUM_BAND_STEER_ATTRS];

        blobmsg_parse(band_steer_policy, NUM_BAND_STEER_ATTRS, sub_tb,
           blobmsg_data(tb[POLICY_CONF_ATTR_BAND_STEER]), blobmsg_len(tb[POLICY_CONF_ATTR_BAND_STEER]));
        if (sub_tb[BAND_STEER_ATTR_ENABLE])
            policy->roaming_policy.band_steer = blobmsg_get_u32(sub_tb[BAND_STEER_ATTR_ENABLE]);
        if (sub_tb[BAND_STEER_ATTR_RSSI_LOW])
            policy->roaming_policy.rssi_low = blobmsg_get_u32(sub_tb[BAND_STEER_ATTR_RSSI_LOW]);
        if (sub_tb[BAND_STEER_ATTR_RSSI_HIGH])
            policy->roaming_policy.rssi_high = blobmsg_get_u32(sub_tb[BAND_STEER_ATTR_RSSI_HIGH]);
        if (sub_tb[BAND_STEER_ATTR_COOLDOWN])
            policy->roaming_policy.cooldown = blobmsg_get_u32(sub_tb[BAND_STEER_ATTR_COOLDOWN]);
    }

    if (tb[POLICY_CONF_ATTR_STEER_POLICY]) {
        struct blob_attr *steer_attr, *steer_tb[NUM_STEER_POLICY_ATTRS];
        int rem;
        struct steer_policy_item *item;

        blobmsg_for_each_attr(steer_attr, tb[POLICY_CONF_ATTR_STEER_POLICY], rem) {
            blobmsg_parse(steer_policy_policy, NUM_STEER_POLICY_ATTRS, steer_tb,
                blobmsg_data(steer_attr), blobmsg_len(steer_attr));

            if (!steer_tb[STEER_POLICY_ATTR_RADIO_UID] || !steer_tb[STEER_POLICY_ATTR_MODE]
                || !steer_tb[STEER_POLICY_ATTR_CHUTIL_THRESH] || !steer_tb[STEER_POLICY_ATTR_RCPI_THRESH])
                continue;

            item = (struct steer_policy_item *)calloc(1, sizeof(*item));
            blobmsg_get_mac(steer_tb[STEER_POLICY_ATTR_RADIO_UID], item->rid);
            item->params.agt_steer_mode = blobmsg_get_u32(steer_tb[STEER_POLICY_ATTR_MODE]);
            item->params.chan_util = blobmsg_get_u32(steer_tb[STEER_POLICY_ATTR_CHUTIL_THRESH]);
            item->params.rcpi_thresh = blobmsg_get_u32(steer_tb[STEER_POLICY_ATTR_RCPI_THRESH]);
            dlist_add_tail(&policy->steer, &item->l);

            /* send policy changed event to logic */
            struct radio *r = radioFind(NULL, item->rid);
            if (r) {
                DEBUG_INFO("radio"MACFMT" steer policy changed, send event to logic\n", MACARG(item->rid));
                mapSteeringPolicyChangedEvent(r->uid, item->params.rcpi_thresh, item->params.chan_util,
                                item->params.agt_steer_mode);
            }
        }
    }

    if (tb[POLICY_CONF_ATTR_METRICS_RPT_INTVL])
        policy->metrics_rpt_intvl = blobmsg_get_u32(tb[POLICY_CONF_ATTR_METRICS_RPT_INTVL]);

    if (tb[POLICY_CONF_ATTR_METRICS_RPT_POLICY]) {
        struct blob_attr *metrics_attr, *metrics_tb[NUM_METRICS_RPT_POLICY_ATTRS];
        int rem;
        struct policy_param_metrics_rpt *rpt;

        blobmsg_for_each_attr(metrics_attr, tb[POLICY_CONF_ATTR_METRICS_RPT_POLICY], rem) {
            blobmsg_parse(rpt_policy, NUM_METRICS_RPT_POLICY_ATTRS, metrics_tb,
                blobmsg_data(metrics_attr), blobmsg_len(metrics_attr));

            if (!metrics_tb[METRICS_RPT_POLICY_ATTR_BAND_IDX] || !metrics_tb[METRICS_RPT_POLICY_ATTR_RCPI_THRESH]
                || !metrics_tb[METRICS_RPT_POLICY_ATTR_RCPI_MARGIN] || !metrics_tb[METRICS_RPT_POLICY_ATTR_CHUTIL_THRESH]
                || !metrics_tb[METRICS_RPT_POLICY_ATTR_INCLUSION_POLICY])
                continue;

            rpt = findReportPolicy(blobmsg_get_u32(metrics_tb[METRICS_RPT_POLICY_ATTR_BAND_IDX]));
            if (rpt) {
                rpt->sta_rcpi_thresh = blobmsg_get_u32(metrics_tb[METRICS_RPT_POLICY_ATTR_RCPI_THRESH]);
                rpt->sta_rcpi_margin = blobmsg_get_u32(metrics_tb[METRICS_RPT_POLICY_ATTR_RCPI_MARGIN]);
                rpt->ap_chutil_thresh = blobmsg_get_u32(metrics_tb[METRICS_RPT_POLICY_ATTR_CHUTIL_THRESH]);
                rpt->assoc_sta_inclusion_mode = blobmsg_get_u32(metrics_tb[METRICS_RPT_POLICY_ATTR_INCLUSION_POLICY]);
            }
        }
    }
    return 0;
}


enum {
    API_CH_PREF_OPC_ATTR_OPCLASS = 0,
    API_CH_PREF_OPC_ATTR_PREF,
    API_CH_PREF_OPC_ATTR_CHANNELS,

    NUM_API_CH_PREF_OPC_ATTRS,
};

static const struct blobmsg_policy ch_pref_opc_policy[] = {
    POLICY_ATTR(API_CH_PREF_OPC_ATTR_OPCLASS, "opclass", BLOBMSG_TYPE_INT32 ),
    POLICY_ATTR(API_CH_PREF_OPC_ATTR_PREF, "pref", BLOBMSG_TYPE_INT32 ),
    POLICY_ATTR(API_CH_PREF_OPC_ATTR_CHANNELS, "chans", BLOBMSG_TYPE_STRING ),
};

enum {
    API_CH_PREF_ATTR_RADIO = 0,
    API_CH_PREF_ATTR_OPCLASSES,

    NUM_API_CH_PREF_ATTRS,
};

static const struct blobmsg_policy ch_pref_policy[] = {
    POLICY_ATTR(API_CH_PREF_ATTR_RADIO, "radio", BLOBMSG_TYPE_STRING ),
    POLICY_ATTR(API_CH_PREF_ATTR_OPCLASSES, "opclasses", BLOBMSG_TYPE_ARRAY ),
};

static int _set_ch_pref(struct blob_attr *pref, uint8_t *al_mac)
{
    struct blob_attr *attr_pref, *pref_tb[NUM_API_CH_PREF_ATTRS];
    struct al_device *dev;
    struct radio *r;
    uint8_t radio_uid[6] = {0};
    int rem_pref;

    dev = alDeviceFind(al_mac);
    if (!dev)
    {
        DEBUG_WARNING("can not find al dev["MACFMT"]\n", MACARG(al_mac));
        return 0;
    }
    blobmsg_for_each_attr(attr_pref, pref, rem_pref)
    {
        blobmsg_parse(ch_pref_policy, NUM_API_CH_PREF_ATTRS, pref_tb,
            blobmsg_data(attr_pref), blobmsg_len(attr_pref));

        if (!pref_tb[API_CH_PREF_ATTR_RADIO] || !pref_tb[API_CH_PREF_ATTR_OPCLASSES])
            continue;

        blobmsg_get_mac(pref_tb[API_CH_PREF_ATTR_RADIO], radio_uid);
        r = radioAdd(dev, radio_uid);

        struct blob_attr *attr_pref_opc, *opc_tb[NUM_API_CH_PREF_OPC_ATTRS];
        int rem_pref_opc;
        struct operating_class *opc;

        blobmsg_for_each_attr(attr_pref_opc, pref_tb[API_CH_PREF_ATTR_OPCLASSES], rem_pref_opc)
        {
            blobmsg_parse(ch_pref_opc_policy, NUM_API_CH_PREF_OPC_ATTRS, opc_tb,
                blobmsg_data(attr_pref_opc), blobmsg_len(attr_pref_opc));

            opc = opclassAdd(r, blobmsg_get_u32(opc_tb[API_CH_PREF_OPC_ATTR_OPCLASS]));
            DEBUG_ERROR("radio["MACFMT"] has %d opcs \n", MACARG(r->uid), r->num_opc_support);
            char *ch_list, *hit;

            ch_list = blobmsg_get_string(opc_tb[API_CH_PREF_OPC_ATTR_CHANNELS]);
            hit = strtok(ch_list, ",");
            int i = 0;
            while(hit)
            {
                opc->channels[i].id = atoi(hit);
                opc->channels[i].pref = blobmsg_get_u32(opc_tb[API_CH_PREF_OPC_ATTR_PREF]);
                i++;
                hit = strtok(NULL, ",");
            }
            opc->num_support_chan = i;
        }
    }
    return 1;
}

enum {
    API_TXPWR_ATTR_RADIO = 0,
    API_TXPWR_ATTR_LIMITS,

    NUM_API_TXPWR_ATTRS,
};

static const struct blobmsg_policy pwr_limit_policy[] = {
    POLICY_ATTR(API_TXPWR_ATTR_RADIO, "radio", BLOBMSG_TYPE_STRING ),
    POLICY_ATTR(API_TXPWR_ATTR_LIMITS, "pwr_limit", BLOBMSG_TYPE_INT32 ),
};

static int _set_txpwr_limit(struct blob_attr *pwr_limit, uint8_t *al_mac)
{
    struct blob_attr *attr_pwr, *pwr_tb[NUM_API_TXPWR_ATTRS];
    struct al_device *dev;
    struct radio *r;
    uint8_t radio_uid[6] = {0};
    int rem;

//    dev = local_device;
    dev = alDeviceFind(al_mac);
    if (!dev)
    {
        DEBUG_WARNING("can not find al dev["MACFMT"]\n", MACARG(al_mac));
        return 0;
    }
    blobmsg_for_each_attr(attr_pwr, pwr_limit, rem)
    {
        blobmsg_parse(pwr_limit_policy, NUM_API_TXPWR_ATTRS, pwr_tb,
            blobmsg_data(attr_pwr), blobmsg_len(attr_pwr));

        if (!pwr_tb[API_TXPWR_ATTR_RADIO] || !pwr_tb[API_TXPWR_ATTR_LIMITS])
            continue;

        blobmsg_get_mac(pwr_tb[API_TXPWR_ATTR_RADIO], radio_uid);
        r = radioAdd(dev, radio_uid);
        if (r)
            r->tx_power = blobmsg_get_u32(pwr_tb[API_TXPWR_ATTR_LIMITS]);
    }
    return 1;
}

enum {
    GET_ATTR_PATH = 0,
    GET_ATTR_MAX,
};

static const struct blobmsg_policy _get_attr_policy[] = {
    POLICY_ATTR(GET_ATTR_PATH, "path", BLOBMSG_TYPE_STRING ),
};

enum {
    API_ATTR_AL_MAC = 0,
    API_ATTR_MAP_POLICY,
    API_ATTR_MOST_PREF,
    API_ATTR_TXPWR_LIMIT,
    API_ATTR_CONTROLLER_WEIGHT,

    NUM_API_ATTRS,
};

static const struct blobmsg_policy _api_attr_policy[] = {
    POLICY_ATTR(API_ATTR_AL_MAC, "al_mac", BLOBMSG_TYPE_STRING ),
    POLICY_ATTR(API_ATTR_MAP_POLICY, "policy", BLOBMSG_TYPE_TABLE ),
    POLICY_ATTR(API_ATTR_MOST_PREF, "most_pref", BLOBMSG_TYPE_ARRAY ),
    POLICY_ATTR(API_ATTR_TXPWR_LIMIT, "txpwr_limit", BLOBMSG_TYPE_ARRAY ),
    POLICY_ATTR(API_ATTR_CONTROLLER_WEIGHT, "controller_weight", BLOBMSG_TYPE_INT32 ),
};

DECLARE_FUNC_UBUS_METHOD(_set_config)
{
    struct blob_attr *tb[NUM_API_ATTRS];
    uint8_t al_mac[MACLEN] = BCMAC;

    blobmsg_parse(_api_attr_policy, NUM_API_ATTRS, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (tb[API_ATTR_AL_MAC]) {
        blobmsg_get_mac(tb[API_ATTR_AL_MAC], al_mac);
    }

    if (tb[API_ATTR_MAP_POLICY]) {
        policyReset(&local_policy);
        _set_map_policy(&local_policy, tb[API_ATTR_MAP_POLICY]);
    }

    if (tb[API_ATTR_MOST_PREF])
        _set_ch_pref(tb[API_ATTR_MOST_PREF], al_mac);

    if (tb[API_ATTR_TXPWR_LIMIT])
        _set_txpwr_limit(tb[API_ATTR_TXPWR_LIMIT], al_mac);

    if (tb[API_ATTR_CONTROLLER_WEIGHT])
        controllerWeightReset(blobmsg_get_u32(tb[API_ATTR_CONTROLLER_WEIGHT]));

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
}

void _fill_policy()
{
    void *pt, *rpt, *st;
    struct roam_policy *rsp = &local_policy.roaming_policy;
    struct steer_policy_item *item = NULL;

    pt = blobmsg_open_table(&b, "policy");
    rpt = blobmsg_open_table(&b, "roam_policy");

    blobmsg_add_u32(&b, "rcpi_steer", rsp->rcpi_steer);
    blobmsg_add_u32(&b, "band_steer", rsp->band_steer);
    blobmsg_add_u32(&b, "band_steer_high", rsp->rssi_high);
    blobmsg_add_u32(&b, "band_steer_low", rsp->rssi_low);
    blobmsg_add_u32(&b, "rcpi_steer_thresh", rsp->rcpi_thresh);
    blobmsg_add_u32(&b, "rssi_gain_of_5g", rsp->rssi_gain_of_5g);
    blobmsg_add_u32(&b, "rssi_gain_thresh", rsp->rssi_gain_thresh);
    blobmsg_add_u32(&b, "cooldown", rsp->cooldown);

    blobmsg_close_table(&b, rpt);

    st = blobmsg_open_array(&b, "steer_policy");

    dlist_for_each(item, local_policy.steer, l) {
        void *item_table = blobmsg_open_table(&b, NULL);
        blobmsgAddMacStr(&b, "rid", item->rid);
        blobmsg_add_u32(&b, "mode", item->params.agt_steer_mode);
        blobmsg_add_u32(&b, "ch_util_thresh", item->params.chan_util);
        blobmsg_add_u32(&b, "rcpi_thresh", item->params.rcpi_thresh);
        blobmsg_close_table(&b, item_table);
    }

    blobmsg_close_array(&b, st);
    blobmsg_close_table(&b, pt);

}

DECLARE_FUNC_UBUS_METHOD(_get_config)
{
    struct blob_attr *tb[GET_ATTR_MAX];

    blob_buf_init(&b, 0);
    blobmsg_parse(_get_attr_policy, GET_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[GET_ATTR_PATH]) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto bail;
    }

    if (!strcmp(blobmsg_get_string(tb[GET_ATTR_PATH]), "policy")) {
        _fill_policy();
    } else {
        addResult(RESULT_INVALID_ARGUMENT);
        goto bail;
    }

    addResult(RESULT_OK);
bail:
    return ubus_send_reply(ctx, req, b.head);
}

enum {
    RECORD_ATTR_MAC = 0,
    RECORD_ATTR_MAX,
};

static const struct blobmsg_policy _record_attr_policy[] = {
    POLICY_ATTR(RECORD_ATTR_MAC, "mac", BLOBMSG_TYPE_STRING ),
};

DECLARE_FUNC_UBUS_METHOD(_get_steer_record)
{
    struct blob_attr *tb[RECORD_ATTR_MAX];
    uint8_t sta_mac[MACLEN] = {0};
    struct steer_client *steer_c;
    struct steer_record *item;

    blob_buf_init(&b, 0);
    blobmsg_parse(_record_attr_policy, RECORD_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

    if (tb[RECORD_ATTR_MAC]) {
        blobmsg_get_mac(tb[RECORD_ATTR_MAC], sta_mac);
    }
    blob_buf_init(&b, 0);
    void *clients = blobmsg_open_array(&b, "clients");
    dlist_for_each(steer_c, g_steer.steer_clients, l) {
        if (tb[RECORD_ATTR_MAC] && MACCMP(sta_mac, steer_c->mac)) {
            continue;
        }

        void *sta_table = blobmsg_open_table(&b, NULL);
        blobmsgAddMacStr(&b, "sta_mac", steer_c->mac);
        blobmsg_add_string(&b, "create_ts", steer_c->create_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(steer_c->create_ts));
        blobmsg_add_string(&b, "deny_acl_ts", steer_c->deny_acl_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(steer_c->deny_acl_ts));
        blobmsg_add_u32(&b, "record_num", steer_c->record_num);
        void *records = blobmsg_open_array(&b, "steer_records");
        dlist_for_each(item, steer_c->records, l) {
            void *item_table = blobmsg_open_table(&b, NULL);
            blobmsg_add_string(&b, "create_ts", item->create_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(item->create_ts));
            blobmsg_add_string(&b, "steer_type", STEER_TYPE_TO_STRING(item->type));
            if (item->type == STEER_TYPE_RCPI) {
                blobmsg_add_u32(&b, "src_rcpi", item->src_rcpi);
                blobmsg_add_u32(&b, "target_rcpi", item->target_rcpi);
            }
            blobmsgAddMacStr(&b, "src_bss", item->src_bss);
            blobmsgAddMacStr(&b, "target_bss", item->target_bss);
            blobmsgAddMacStr(&b, "final_bss", item->final_bss);
            blobmsg_add_string(&b, "steer_method", STEER_METHOD_TO_STRING(item->method));
            blobmsg_add_u32(&b, "btm_status", item->btm_status_code);
            blobmsg_add_string(&b, "status", item->status==1?"success":"fail");
            blobmsg_add_string(&b, "btm_rsp_ts", item->btm_rsp_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(item->btm_rsp_ts));
            blobmsg_add_string(&b, "finish_ts", item->finish_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(item->finish_ts));
            blobmsg_add_string(&b, "deauth_ts", item->deauth_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(item->deauth_ts));
            blobmsg_close_table(&b, item_table);
        }
        blobmsg_close_array(&b, records);
        blobmsg_close_table(&b, sta_table);
    }
    blobmsg_close_array(&b, clients);

    addResult(RESULT_OK);

    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_get_role)
{
    void *t;

    ubus_ctx = ctx;

    blob_buf_init(&b, 0);

    t = blobmsg_open_table(&b, NULL);
    blobmsg_add_string(&b, "role", local_device->is_controller ? "controller" : "agent");
    blobmsg_add_u32(&b, "weight", local_device->controller_weight);
    blobmsg_close_table(&b, t);

    addResult(RESULT_OK);

    return ubus_send_reply(ctx, req, b.head);
}


static const struct ubus_method _api_obj_methods[] = {
    UBUS_METHOD_NOARG("topology", _topology),
    UBUS_METHOD("set", _set_config, _api_attr_policy),
    UBUS_METHOD("get", _get_config, _get_attr_policy),
    UBUS_METHOD("start_wps", _startWPS, _wps_policy),
    UBUS_METHOD("steer_record", _get_steer_record, _record_attr_policy),
    UBUS_METHOD_NOARG("role", _get_role),
};

static struct ubus_object_type _api_obj_type =
    UBUS_OBJECT_TYPE(OBJ_NAME, _api_obj_methods);

DECLARE_UBUS_OBJ(_api_obj, OBJ_NAME, &_api_obj_type, _api_obj_methods);

struct ubus_object *getApiObj(void)
{
    return &_api_obj;
}
