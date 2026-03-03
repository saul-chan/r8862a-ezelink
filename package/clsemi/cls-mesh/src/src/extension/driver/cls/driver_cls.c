#include <stdlib.h>
#include <asm/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netinet/ether.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <glob.h>
#include <fnmatch.h>
#ifdef USE_UCI
#include <uci.h>
#endif
#include <linux/nl80211.h>
#include <libubus.h>
#include "platform.h"
#include "datamodel.h"
#include "platform_os.h"
#include "extension.h"
#include "wifi.h"
#include "al_msg.h"
#include "nl80211_helper.h"
#include "nl80211_cls.h"
#ifdef USE_UCI
#include "driver_uci.h"
#endif
#include "feature/ubus/ubus_helper.h"
#include "al_action.h"

#include "cls/cls_nl80211_vendor.h"


#define CLS_WIFI_NAME_PREFIX "wlan"
#define MAX_ASSOC_FRAME_NUM 128
#define CHANNEL_SCAN_MAX_WAIT_TIME 60

#define NL_SOCK_MUTEX_LOCK(sc) do { \
    pthread_mutex_lock(&sc->sock_mutex); \
} while(0)

#define NL_SOCK_MUTEX_UNLOCK(sc) do { \
    pthread_mutex_unlock(&sc->sock_mutex); \
} while(0)

#define WIFI_INTERFACE_INFO_CORRECT(wif_intf) (0 != strlen(wif_intf->ssid) && 0 != wif_intf->channel)

/* must default NLA_F_NESTED */
#ifndef NL_CAPABILITY_VERSION_3_5_0
#define nla_nest_start(msg, attrtype) \
    nla_nest_start(msg, NLA_F_NESTED | (attrtype))
#endif

/* Allowed bandwidth mask */
enum clmesh_chan_width_attr {
    ALLOW_CHAN_WIDTH_10   = BIT(0),
    ALLOW_CHAN_WIDTH_20   = BIT(1),
    ALLOW_CHAN_WIDTH_40P  = BIT(2),
    ALLOW_CHAN_WIDTH_40M  = BIT(3),
    ALLOW_CHAN_WIDTH_80   = BIT(4),
    ALLOW_CHAN_WIDTH_160  = BIT(5),
};

struct sock_ctrl {
    struct nl_sock *sock;
    pthread_mutex_t sock_mutex;
    int id;
    int (*handler)(struct nl_msg *, void *);
    void *handler_data;
};

struct frame_registered_item {
    dlist_item l;
    int32_t if_idx;
    uint16_t f_type;
    uint32_t match_len;
    char *match;
};

struct notification_ctx {
    dlist_item l;
    struct ubus_subscriber suscriber[1];
    uint32_t id;
};

struct cls_driver_ctx {
    char *name;
    struct sock_ctrl *cmd_ctrl;
    struct sock_ctrl *event_ctrl;
    struct sock_ctrl *rtm_ctrl;
    void *nl80211evt_task;
    void *rtmevt_task;
    uint32_t split_wiphy_dump:1;
    dlist_head radios;
    dlist_head bsses;
    uint32_t assoc_frame_num;
    dlist_head assoc_frames;
    dlist_head registered_frames;
    void *timer;  // get radio timer if get radio info failed

    struct notification_ctx service_notifcation;
    dlist_head hostapd_notifications;
};

struct rtm_new_link_event_node {
    struct cls_driver_ctx *ctx;
    int if_idx;
    void *timer;
};

struct wiphy_band_capa {
    uint8_t band;

    uint8_t ht_support:1;
    uint8_t vht_support:1;
    uint8_t he_support:1;

    uint16_t ht_capa;
    uint8_t ht_mcsset[MAX_HT_MCS_SIZE];

    uint8_t vht_capa[4];
    uint16_t vht_rx_mcs;
    uint16_t vht_tx_mcs;

    uint8_t he_phy_cap[11];
    uint8_t he_mac_cap[6];
    uint8_t he_mcs_len;
    uint8_t he_mcs[MAX_HE_MCS_SIZE]; /* 0-3: RX TX HE-MCS Map <=80MHz; 4-7: RX TX HE-MCS Map =160MHz; 8-11: RX TX HE-MCS Map =80+80MHz; */
    uint8_t he_ppet[25];
    uint16_t he_6ghz_capa;
};

struct wiphy_private
{
    dlist_item l;
    const char *name;       //phy name
    int idx;                //phy index
    const char *uci_name;   //uci wifi-device section name
    uint8_t ap_num;
    uint8_t mac[MACLEN];

    struct wiphy_band_capa band_capa[band_max_idx];
    uint8_t band;
    enum e_wifi_band_idx current_band_idx;

    uint8_t opclass_num;
    struct operating_class opclasses[MAX_OPCLASS];

    struct chan_scan_capability scan_capa;
    struct vbss_capability vbss_capa;

    char *primary_name;

#define CHANNEL_SCAN_STARTED 1
#define CHANNEL_SCAN_FINISHED 2
    uint8_t channel_scan_status;
    uint32_t last_scan_trigger_ts;
    uint8_t last_scan_opclass;
    uint8_t capa[1];
    dlist_head last_scan_list_head;
};

struct wifi_interface_private
{
    dlist_item l;
    void *ctx;
    int index;
    uint8_t mac[MACLEN];
    char *name;
    char ssid[MAX_SSID_LEN+2];
    uint8_t power_level;
    uint8_t opclass;
    uint8_t channel;
    uint8_t bandwidth;
    uint8_t role;
    uint8_t wds;
    struct wiphy_private *radio;
    void *timer;
    uint8_t delete;
};

struct assoc_req_data {
    dlist_item l;

    uint32_t ts;     //timestamp for new association request
    uint8_t bssid[MACLEN];
    uint8_t mac[MACLEN];
    uint8_t *frame;
    uint32_t frame_len;
};

#if 0
struct scanlist_ht_chan_entry {
    uint8_t primary_chan;
    uint8_t secondary_chan_off;
    uint8_t chan_width;
};

struct scanlist_vht_chan_entry {
    uint8_t chan_width;
    uint8_t center_chan_1;
    uint8_t center_chan_2;
};
#endif

struct scanlist_entry {
    dlist_item l;
    uint8_t bssid[6];
    uint16_t caps;
    uint8_t channel;
    uint8_t signal;
    uint32_t last_seen_ms;
    uint32_t ie_len;
    uint32_t beacon_ie_len;
    /* Followed by ie_len + beacon_ie_len octets of IE data */
};

struct handler_args {
    const char *group;
    int id;
};

struct station_stats {
    dlist_item l;
    uint8_t mac[MACLEN];
#define FLAG_CONNECTED_TIME BIT(0)
#define FLAG_INACTIVE_TIME BIT(1)
#define FLAG_RX_BYTES BIT(2)
#define FLAG_TX_BYTES BIT(3)
#define FLAG_SIGNAL BIT(4)
#define FLAG_SIGNAL_AVG BIT(5)
#define FLAG_RCPI_UL BIT(6)
#define FLAG_RATE_UL BIT(7)
#define FLAG_RATE_DL BIT(8)
#define FLAG_AGE_LAST_TX BIT(9)
#define FLAG_AGE_LAST_RX BIT(10)
#define FLAG_RX_PACKETS BIT(11)
#define FLAG_TX_PACKETS BIT(12)
#define FLAG_TX_RETRIES BIT(13)
#define FLAG_TX_FAILED BIT(14)
#define FLAG_RX_DROPPED_MISC BIT(15)
#define FLAG_AIRTIME_WEIGHT BIT(16)
#define FLAG_RX_MPDU_COUNT BIT(17)
#define FLAG_FCS_ERR_COUNT BIT(18)
#define FLAG_AIRTIME_LINK_METRIC BIT(19)
#define FLAG_LAST_DATARATE_DL BIT(20)
#define FLAG_LAST_DATARATE_UL BIT(21)
#define FLAG_UTILIZATION_RX BIT(22)
#define FLAG_UTILIZATION_TX BIT(23)

    uint32_t flags;
    uint32_t connected_time;
    uint32_t inactive_time;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    int8_t signal;
    int8_t signal_avg;
    uint8_t rcpi_ul;  /* Uplink RCPI for STA. */

    uint32_t rate_ul; /* tx rate */
    uint32_t rate_dl; /* rx rate */
    uint32_t age_last_tx; /* TODO */
    uint32_t age_last_rx; /* TODO */

    uint32_t rx_packets;
    uint32_t tx_packets;
    uint32_t tx_retries; /* tx tries */
    uint32_t tx_failed;  /* tx errors */
    uint32_t rx_dropped_misc; /* rx errors */

    uint16_t airtime_weight;

    uint32_t rx_mpdu_count;
    uint32_t fcs_err_count;

    uint32_t airtime_link_metric;

    uint32_t last_datarate_dl; /* TODO */
    uint32_t last_datarate_ul; /* TODO */
    uint32_t utilization_rx; /* TODO */
    uint32_t utilization_tx; /* TODO */
};

static int _issueCMD(struct sock_ctrl *ctrl, struct nl_msg *msg, int wiphy, int intf, int wdev,
                    int ext_flag, int8_t cmd, int flag,
                    int (*handler)(struct nl_msg *, void *), void *data);
static void _disconnectDriver(struct sock_ctrl *ctrl);

#ifdef USE_UCI
static struct uci_context *_local_uci_ctx = NULL;
#endif

enum {
    ATTR_DISALLOW_BSSID = 0,
    ATTR_DISALLOW_STAT,

    ATTR_DISALLOW_MAX,
};

static const struct blobmsg_policy disallow_attr_policy[] = {
    POLICY_ATTR(ATTR_DISALLOW_BSSID, "bssid", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_DISALLOW_STAT, "disallow", BLOBMSG_TYPE_INT8),
};

static void _reportAssocDisallow(struct blob_attr *msg)
{
    struct blob_attr *attr[ATTR_DISALLOW_MAX];
    uint8_t bssid[MACLEN] = {0};
    uint8_t assoc_allow_status = 0;
    uint8_t *buf, *p;

    blobmsg_parse(disallow_attr_policy, ATTR_DISALLOW_MAX, attr, blob_data(msg), blob_len(msg));
    if (!attr[ATTR_DISALLOW_BSSID] || !attr[ATTR_DISALLOW_STAT]) {
        DEBUG_WARNING("Missing key info about disallow\n");
        return;
    }
    blobmsg_get_mac(attr[ATTR_DISALLOW_BSSID], bssid);
    assoc_allow_status = blobmsg_get_u8(attr[ATTR_DISALLOW_STAT]);

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_disallow_notification);
    p = msgaPutBin(p, attr_bssid, bssid, MACLEN);

    if (assoc_allow_status) /* disallow value from hostapd, >0 means assoc disallow, =0 means assoc allow */
        p = msgaPutU8(p, attr_disallow, 0); /* the attr_disallow value is defined as MAP SPEC */
    else
        p = msgaPutU8(p, attr_disallow, 1);

    msgSend(buf, p-buf);
    msgPutBuf(buf);
}

static const struct blobmsg_policy hostapd_watch_policy = { "address", BLOBMSG_TYPE_STRING };

static int hostapdEventHandler(struct ubus_context *ctx, struct ubus_object *obj,
                           struct ubus_request_data *req, const char *method,
                           struct blob_attr *msg)
{
    uint8_t *buf, *p;
    struct blob_attr *attr;
    uint8_t sta_mac[MACLEN];
    char if_name[32] = {0};
    uint16_t reason_code = WLAN_REASON_UNSPECIFIED;

    /* only concern "inactive-deauth"、"deauth" event now */
    if (!strcmp(method, "inactive-deauth")) {
        reason_code = WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY;
    }
    else if (!strcmp(method, "deauth")) {
        reason_code = WLAN_REASON_DEAUTH_LEAVING;
    }
    else if (!strcmp(method, "disallow_status")) {
        _reportAssocDisallow(msg);
        return 0;
    }
    else
        return 0;

    if (!obj->name || strlen(obj->name) < 9)
        return 0;

    blobmsg_parse(&hostapd_watch_policy, 1, &attr, blob_data(msg), blob_len(msg));
    if (!attr)
        return 0;
    blobmsg_get_mac(attr, sta_mac);

    memcpy(if_name, obj->name + 8, strlen(obj->name) - 8 + 1);

    DEBUG_INFO("Receive event from %s: if_name[%s], method[%s], sta_mac["MACFMT"]!\n",
        obj->name, if_name, method, MACARG(sta_mac));

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_station_delete);

    p = msgaPutBin(p, attr_if_name, (uint8_t *)if_name, strlen(if_name));
    p = msgaPutBin(p, attr_mac, sta_mac, MACLEN);
    p = msgaPutU16(p, attr_reason_code, reason_code);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

#ifdef USE_UCI
static struct uci_context *_getUCICtx()
{
    if (!_local_uci_ctx)
        _local_uci_ctx = uci_alloc_context();
    return _local_uci_ctx;
}
#endif

static struct wiphy_private *_findWiphyIdx(struct cls_driver_ctx *ctx, int idx)
{
    struct wiphy_private *wiphy;

    dlist_for_each(wiphy, ctx->radios, l) {
        if (wiphy->idx == idx)
            return wiphy;
    }
    return NULL;
}

static struct wiphy_private *_findWiphy(struct cls_driver_ctx *ctx, uint8_t *mac)
{
    struct wiphy_private *wiphy;

    dlist_for_each(wiphy, ctx->radios, l) {
        if (!MACCMP(mac, wiphy->mac))
            return wiphy;
    }
    return NULL;
}

static void _wiphyDelete(struct wiphy_private *radio)
{
    if (!radio)
        return;

    if (radio->name)
        free((void *)radio->name);

    if (radio->uci_name)
        free((void *)radio->uci_name);

    dlist_remove(&radio->l);
    dlist_free_items(&radio->last_scan_list_head, struct scanlist_entry, l);
    free(radio);
}

struct notification_ctx *_findNotification(struct cls_driver_ctx *ctx, char *name)
{
    struct notification_ctx *n_ctx;

    dlist_for_each(n_ctx, ctx->hostapd_notifications, l) {
        if (!strcmp(name, n_ctx->suscriber[0].obj.name)) {
            break;
        }
    }
    return n_ctx;
}
static int _addHostapdListener(struct wifi_interface_private *wif)
{
    int ret = -1;
    char name[128] = {0};
    struct ubus_context *ubus = PLATFORM_GET_UBUS();
    struct cls_driver_ctx *ctx;
    struct notification_ctx *n_ctx;

    if (!wif)
        return -1;

    snprintf(name, sizeof(name) -1, "hostapd.%s", wif->name);

    ctx = (struct cls_driver_ctx *)wif->ctx;
    if ((n_ctx = _findNotification(wif->ctx, name))) {
        dlist_remove(&n_ctx->l);

        if ((ret = ubus_unsubscribe(ubus, &n_ctx->suscriber[0], n_ctx->id))) {
            DEBUG_ERROR("unsubscribe on %s failed(ret=%d)\n", name, ret);
        }
        if ((ret = ubus_unregister_subscriber(ubus, &n_ctx->suscriber[0]))) {
            DEBUG_ERROR("unregister on %s failed(ret=%d)\n", name, ret);
        }

        free((void *)n_ctx->suscriber[0].obj.name);
        n_ctx->id = 0;
        memset(n_ctx->suscriber, 0, sizeof(struct ubus_subscriber));
    } else {
        n_ctx = (struct notification_ctx *)calloc(1, sizeof(struct notification_ctx));
    }

    n_ctx->suscriber[0].cb = hostapdEventHandler;
    n_ctx->suscriber[0].obj.name = strdup(name);

    if ((ret = ubus_lookup_id(ubus, name, &n_ctx->id))) {
        DEBUG_ERROR("can not lookup on ubus: %s!\n", name);
        goto bail;
    }

    if ((ret = ubus_register_subscriber(ubus, &n_ctx->suscriber[0]))) {
        DEBUG_ERROR("can not register subscriber on %s\n", name);
        goto bail;
    }

    if ((ret = ubus_subscribe(ubus, &n_ctx->suscriber[0], n_ctx->id))) {
        DEBUG_ERROR("can not subscribe on %s\n", name);
        goto bail;
    }

    dlist_add_tail(&ctx->hostapd_notifications, &n_ctx->l);

    DEBUG_ERROR("added hostapd listener for %s\n", wif->name);
    return 0;
bail:
    free(n_ctx);
    return ret;
}

static struct wifi_interface_private *_findWifiInterface(struct cls_driver_ctx *ctx, uint8_t *mac)
{
    struct wifi_interface_private *wif;

    dlist_for_each(wif, ctx->bsses, l) {
        if (!memcmp(wif->mac, mac, MACLEN))
            return wif;
    }

    return NULL;
}

static struct wifi_interface_private *_findWifiInterfaceIdx(struct cls_driver_ctx *ctx, int idx)
{
    struct wifi_interface_private *wif;

    dlist_for_each(wif, ctx->bsses, l) {
        if (wif->index == idx)
            return wif;
    }

    return NULL;
}

static struct wifi_interface_private *_wifiInterfaceNew(struct cls_driver_ctx *ctx, int ifidx, char *name)
{
    struct wifi_interface_private *wif_priv;

    if (!(wif_priv = _findWifiInterfaceIdx(ctx, ifidx))) {
        wif_priv = calloc(1, sizeof(struct wifi_interface_private));
        wif_priv->ctx = ctx;
        wif_priv->index = ifidx;
        dlist_add_tail(&ctx->bsses, &wif_priv->l);
    }

    if (name) {
        REPLACE_STR(wif_priv->name, strdup(name));
    }

    return wif_priv;
}

static void _wifiInterfaceDelete(struct wifi_interface_private *wif)
{
    if (!wif)
        return;

    if (wif->timer) {
        wif->delete = 1;
        return;
    }
    wif->timer = NULL;
    if (wif->name)
        free(wif->name);

    dlist_remove(&wif->l);
    free(wif);
}

static int _findWiphyIdxByIfidx(struct cls_driver_ctx *ctx, int ifidx)
{
    struct wifi_interface_private *wif = NULL;

    wif = _findWifiInterfaceIdx(ctx, ifidx);

    if (!wif || !wif->radio)
        return -1;

    return wif->radio->idx;
}

static struct assoc_req_data *_assocReqFrameFind(struct cls_driver_ctx *ctx, uint8_t *mac)
{
    struct assoc_req_data *assoc = NULL;

    dlist_for_each(assoc, ctx->assoc_frames, l) {
        if (!memcmp(assoc->mac, mac, MACLEN)) {
            return assoc;
        }
    }

    return NULL;
}

static int _assocReqFrameDel(struct cls_driver_ctx *ctx, uint8_t *mac)
{
    struct assoc_req_data *assoc = _assocReqFrameFind(ctx, mac);

    if (assoc) {
        dlist_remove(&assoc->l);
        if (assoc->frame)
            free(assoc->frame);
        free(assoc);
        ctx->assoc_frame_num--;
    }

    return 0;
}

static int _assocReqFrameAdd(struct cls_driver_ctx *ctx, uint8_t *bssid, uint8_t *mac, uint8_t *frame, uint32_t frame_len)
{
    struct assoc_req_data *assoc = _assocReqFrameFind(ctx, mac);
    struct wifi_interface_private *wif = _findWifiInterface(ctx, bssid);

    if (!wif) {
        DEBUG_ERROR("bssid: "MACFMT"not exist. add assoc frame failed.\n", MACARG(bssid));
        return -1;
    }

    if (assoc) {
        if (assoc->frame)
            free(assoc->frame);
        goto store;
    }

    if (ctx->assoc_frame_num >= MAX_ASSOC_FRAME_NUM) {
        struct assoc_req_data *first;
        first = (struct assoc_req_data *)container_of(dlist_get_first(&ctx->assoc_frames), struct assoc_req_data, l);
        _assocReqFrameDel(ctx, first->mac);
    }

    assoc = calloc(1, sizeof(struct assoc_req_data));
    if (!assoc) {
        DEBUG_ERROR("calloc assoc_req_data failed.\n");
        return -1;
    }
    dlist_add_tail(&ctx->assoc_frames, &assoc->l);
    ctx->assoc_frame_num++;

store:
    MACCPY(assoc->bssid, bssid);
    MACCPY(assoc->mac, mac);
    assoc->ts = PLATFORM_GET_TIMESTAMP(0);
    assoc->frame = calloc(1, frame_len);
    if (!assoc->frame) {
        DEBUG_ERROR("calloc assoc_req_data.frame failed.\n");
        _assocReqFrameDel(ctx, mac);
        return -1;
    }
    memcpy(assoc->frame, frame, frame_len);
    assoc->frame_len = frame_len;

    return 0;
}

static struct sock_ctrl *_connectDriver()
{
    struct sock_ctrl *ret = calloc(1, sizeof(struct sock_ctrl));
    int err = 0;

    if (ret) {
        ret->sock = nl_socket_alloc();
        pthread_mutex_init(&ret->sock_mutex, NULL);
        if (!ret->sock) {
            DEBUG_ERROR("failed to nl_socket_alloc()\n");
            goto fail;
        }
        if (genl_connect(ret->sock)) {
            DEBUG_ERROR("failed to connect genl_connect\n");
            goto fail;
        }
        /*
         * libnl uses a pretty small buffer (32 kB that gets converted to 64 kB)
         * by default. It is possible to hit that limit in some cases where
         * operations are blocked, e.g., with a burst of Deauthentication frames
         * to hostapd and STA entry deletion. Try to increase the buffer to make
         * this less likely to occur.
         */
        nl_socket_set_buffer_size(ret->sock, 262144, 8192);

        if (setsockopt(nl_socket_get_fd(ret->sock), SOL_NETLINK, NETLINK_EXT_ACK, &err, sizeof(err))) {
            DEBUG_WARNING("setsockopt NETLINK_EXT_ACK failed\n");
        }

        ret->id = genl_ctrl_resolve(ret->sock, "nl80211");
        if (ret->id<0) {
            DEBUG_ERROR("genl_ctrl_resolve failed\n");
            goto fail;
        }
    }
    return ret;
fail:
    free(ret);
    return NULL;
}

static struct sock_ctrl *_connectRTM()
{
    struct sock_ctrl *ret = calloc(1, sizeof(struct sock_ctrl));
    int rxbuf = 20*1024;

    if (ret) {
        ret->sock = nl_socket_alloc();
        pthread_mutex_init(&ret->sock_mutex, NULL);
        if (!ret->sock) {
            DEBUG_ERROR("failed to nl_socket_alloc()\n");
            goto fail;
        }
        nl_join_groups(ret->sock, RTMGRP_LINK);

        if (nl_connect(ret->sock, NETLINK_ROUTE)) {
            DEBUG_ERROR("failed to nl_connect()\n");
            goto fail;
        }

        if (setsockopt(nl_socket_get_fd(ret->sock), SOL_SOCKET, SO_RCVBUFFORCE, &rxbuf, sizeof(rxbuf))) {
            DEBUG_WARNING("setsockopt SO_RCVBUFFORCE failed\n");
        }
    }
    return ret;
fail:
    free(ret);
    return NULL;
}

static void _disconnectDriver(struct sock_ctrl *ctrl)
{
    if (ctrl) {
        if (ctrl->sock)
            nl_socket_free(ctrl->sock);
        free(ctrl);
    }
}

static int _cmdError(struct sockaddr_nl *nla, struct nlmsgerr *err,
        void *arg)
{
    int *ret = arg;

    *ret = err->error;
    DEBUG_ERROR("_cmdError return: %d(%s)\n", *ret, nl_geterror(*ret));

    return NL_STOP;
}

static int _cmdSeqCheck(struct nl_msg *msg, void *arg)
{
    struct nl_msg *req = (struct nl_msg *)arg;
    struct nlmsghdr *nlhdr_req = nlmsg_hdr(req);
    struct nlmsghdr *nlhdr = nlmsg_hdr(msg);

    if (nlhdr_req->nlmsg_seq != nlhdr->nlmsg_seq) {
        DEBUG_ERROR("nl seq not matched! req_seq: %u, rsp_seq: %u\n", nlhdr_req->nlmsg_seq, nlhdr->nlmsg_seq);
        return NL_SKIP;
    }
    return NL_OK;
}

static int _cmdFinish(struct nl_msg *msg, void *arg)
{
    int *ret = arg;
    *ret = 0;
    return NL_SKIP;
}

static int _cmdAck(struct nl_msg *msg, void *arg)
{
    int *ret = arg;
    *ret = 0;
    return NL_STOP;
}

static int _eventNoSeqCheck(struct nl_msg *msg, void *arg)
{
    return NL_OK;
}

static int _cmdFamily(struct nl_msg *msg, void *arg)
{
    struct handler_args *grp = arg;
    struct nlattr *tb[CTRL_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *mcgrp;
    int rem_mcgrp;

    nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
          genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[CTRL_ATTR_MCAST_GROUPS])
        return NL_SKIP;

    nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {
        struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

        nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX,
              nla_data(mcgrp), nla_len(mcgrp), NULL);

        if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] ||
            !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID])
            continue;
        if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]),
                grp->group, nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME])))
            continue;
        grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
        break;
    }

    return NL_SKIP;
}

#ifdef USE_UCI
static int _readIntFile(const char *path)
{
    int fd;
    int ret = -1;
    char tmp[16];

    if ((fd = open(path, O_RDONLY)) > -1) {
        if (read(fd, tmp, sizeof(tmp)) > 0)
            ret = atoi(tmp);
        close(fd);
    }
    return ret;
}
#endif

static char * _readStrFile(const char *path)
{
    char *ret = NULL;
    int fd, len;
#define TMP_STR_LEN (20)
    static char tmp_str[TMP_STR_LEN+1];

    if ((fd = open(path, O_RDONLY)) > -1) {
        if ((len = read(fd, tmp_str, TMP_STR_LEN)) > 0) {
            if (tmp_str[len-1] == '\n')
                tmp_str[--len] = 0;
            ret = tmp_str;
        }
        close(fd);
    }
    return ret;
}

static int _getNlMulticastId(struct nl_sock *sock, const char *family, const char *group)
{
    struct nl_msg *msg;
    struct nl_cb *cb;
    int ret, ctrlid;
    struct handler_args grp = {
        .group = group,
        .id = -ENOENT,
    };

    msg = nlmsg_alloc();
    if (!msg)
        return -ENOMEM;

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        ret = -ENOMEM;
        goto out_fail_cb;
    }

    ctrlid = genl_ctrl_resolve(sock, "nlctrl");

    genlmsg_put(msg, 0, 0, ctrlid, 0,
            0, CTRL_CMD_GETFAMILY, 0);

    ret = -ENOBUFS;
    NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

    ret = nl_send_auto_complete(sock, msg);
    if (ret < 0)
        goto out;

    ret = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, _cmdError, &ret);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, _cmdAck, &ret);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _cmdFamily, &grp);

    while (ret > 0)
        ret = nl_recvmsgs(sock, cb);

    if (ret == 0)
        ret = grp.id;
 nla_put_failure:
 out:
    nl_cb_put(cb);
 out_fail_cb:
    nlmsg_free(msg);
    return ret;
}

static int _initEventSocket(struct sock_ctrl *ctrl)
{
    int ret = -1, mid;
    if (ctrl) {
        if ((mid = _getNlMulticastId(ctrl->sock, "nl80211", "config")<0))
            return mid;
        ret = nl_socket_add_membership(ctrl->sock, mid);
        if (ret)
            return ret;

        if ((mid = _getNlMulticastId(ctrl->sock, "nl80211", "scan"))>=0) {
            ret = nl_socket_add_membership(ctrl->sock, mid);
            if (ret)
                return ret;
        }
        if ((mid = _getNlMulticastId(ctrl->sock, "nl80211", "regulatory"))>=0) {
            ret = nl_socket_add_membership(ctrl->sock, mid);
            if (ret)
                return ret;
        }

        if ((mid = _getNlMulticastId(ctrl->sock, "nl80211", "mlme"))>=0) {
            ret = nl_socket_add_membership(ctrl->sock, mid);
            if (ret)
                return ret;
        }

        if ((mid = _getNlMulticastId(ctrl->sock, "nl80211", "vendor"))>=0) {
            ret = nl_socket_add_membership(ctrl->sock, mid);
            if (ret)
                return ret;
        }

        if ((mid = _getNlMulticastId(ctrl->sock, "nl80211", "nan"))>=0) {
            ret = nl_socket_add_membership(ctrl->sock, mid);
            if (ret)
                return ret;
        }
        ret = 0;
    }
    return ret;
}

static void *_clsDriverInit()
{
    struct cls_driver_ctx *ctx;

    ctx = calloc(1, sizeof(struct cls_driver_ctx));
    if (ctx) {
        if (!(ctx->cmd_ctrl = _connectDriver())) {
            DEBUG_ERROR("failed to connect driver for cmd\n");
            goto fail;
        }
        if (!(ctx->event_ctrl = _connectDriver())) {
            DEBUG_ERROR("failed to connect driver for event\n");
            goto fail;
        }

        if (!(ctx->rtm_ctrl = _connectRTM())) {
            DEBUG_ERROR("failed to connect RTM\n");
            goto fail;
        }

        if (_initEventSocket(ctx->event_ctrl))
            goto fail;

        dlist_head_init(&ctx->radios);
        dlist_head_init(&ctx->bsses);
        dlist_head_init(&ctx->assoc_frames);
        dlist_head_init(&ctx->registered_frames);
        dlist_head_init(&ctx->hostapd_notifications);
    }
    return ctx;
fail:
    if (ctx) {
        if (ctx->cmd_ctrl)
            _disconnectDriver(ctx->cmd_ctrl);
        if (ctx->event_ctrl)
            _disconnectDriver(ctx->event_ctrl);
        if (ctx->rtm_ctrl)
            _disconnectDriver(ctx->rtm_ctrl);
        free(ctx);
    }
    return NULL;
}



void _registerHandler(struct sock_ctrl *ctrl, int (*handler)(struct nl_msg *, void *), void *data)
{
    if (ctrl) {
        ctrl->handler = handler;
        ctrl->handler_data = data;
    }
}

int _cmdValid(struct nl_msg *msg, void *arg)
{
    struct sock_ctrl *ctrl = (struct sock_ctrl *)arg;
    if ((ctrl) && (ctrl->handler))
        return ctrl->handler(msg, ctrl->handler_data);
    return NL_OK;
}

#ifdef USE_UCI
#ifndef PATH_MAX
#define PATH_MAX (256)
#endif
static const char *_phyName2Path(const char *phyname)
{
    static char path[PATH_MAX];
    const char *prefix = "/sys/devices/";
    int prefix_len = strlen(prefix);
    int buf_len, offset;
    struct dirent *e;
    char buf[128], *link;
    int phy_id;
    int seq = 0;
    DIR *d;

    if (strncmp(phyname, "phy", 3) != 0)
        return NULL;

    phy_id = atoi(phyname + 3);
    buf_len = snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/device", phyname);
    link = realpath(buf, path);
    if (!link)
        return NULL;

    if (strncmp(link, prefix, prefix_len) != 0)
        return NULL;

    link += prefix_len;

    prefix = "platform/";
    prefix_len = strlen(prefix);
    if (!strncmp(link, prefix, prefix_len) && strstr(link, "/pci"))
        link += prefix_len;

    snprintf(buf + buf_len, sizeof(buf) - buf_len, "/ieee80211");
    d = opendir(buf);
    if (!d)
        return link;

    while ((e = readdir(d)) != NULL) {
        int cur_id;

        if (strncmp(e->d_name, "phy", 3) != 0)
            continue;

        cur_id = atoi(e->d_name + 3);
        if (cur_id >= phy_id)
            continue;

        seq++;
    }

    closedir(d);

    if (!seq)
        return link;

    offset = link - path + strlen(link);
    snprintf(path + offset, sizeof(path) - offset, "+%d", seq);

    return link;
}


static int _path2PhyIdx(const char *path)
{
    char buf[300];
    struct dirent *e;
    const char *cur_path;
    int cur_path_len;
    int path_len;
    int idx = -1;
    DIR *d;

    if (!path)
        return -1;

    path_len = strlen(path);
    if (!path_len)
        return -1;

    d = opendir("/sys/class/ieee80211");
    if (!d)
        return -1;

    while ((e = readdir(d)) != NULL) {
        cur_path = _phyName2Path(e->d_name);
        if (!cur_path)
            continue;

        cur_path_len = strlen(cur_path);
        if (cur_path_len < path_len)
            continue;

        if (strcmp(cur_path + cur_path_len - path_len, path) != 0)
            continue;

        snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", e->d_name);
        idx = _readIntFile(buf);

        if (idx >= 0)
            break;
    }

    closedir(d);

    return idx;
}

static int _mac2PhyIdx(const char *opt)
{
    char buf[128];
    int i, idx = -1;
    glob_t gl;
    char *str;

    if (!opt)
        return -1;

    snprintf(buf, sizeof(buf), "/sys/class/ieee80211/*");   /**/
    if (glob(buf, 0, NULL, &gl))
        return -1;

    for (i = 0; i < gl.gl_pathc; i++)
    {
        snprintf(buf, sizeof(buf), "%s/macaddress", gl.gl_pathv[i]);
        if (!(str = _readStrFile(buf)))
            continue;

        if (fnmatch(opt, str, FNM_CASEFOLD))
            continue;

        snprintf(buf, sizeof(buf), "%s/index", gl.gl_pathv[i]);
        if ((idx = _readIntFile(buf)) > -1)
            break;
    }

    globfree(&gl);

    return idx;
}

static int _phy2PhyIdx(const char *opt)
{
    char buf[128];

    if (!opt)
        return -1;

    snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", opt);
    return _readIntFile(buf);
}

static char *_getWiphyUCIName(int idx, uint8_t *pband)
{
    struct uci_context *ctx;
    struct uci_package *wireless;
    struct uci_section *s;
    struct uci_element *e;
    char *ret = NULL;
    const char *t_s;
    uint8_t band = band_none;

    ctx = _getUCICtx();;
    uci_load(ctx, "wireless", &wireless);

    uci_foreach_element(&wireless->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "wifi-device")) {
           if ((t_s = uci_lookup_option_string(ctx, s, "type"))) {
                if (!strcmp(t_s, "mac80211")) {
                    if ((t_s = uci_lookup_option_string(ctx, s, "path"))) {
                        if (_path2PhyIdx(t_s) == idx) {
                            ret = strdup(s->e.name);
                            goto bail;
                        }
                    }
                    if ((t_s = uci_lookup_option_string(ctx, s, "macaddr"))) {
                        if (_mac2PhyIdx(t_s) == idx) {
                            ret = strdup(s->e.name);
                            goto bail;
                        }
                    }
                    if ((t_s = uci_lookup_option_string(ctx, s, "phy"))) {
                        if (_phy2PhyIdx(t_s) == idx) {
                            ret = strdup(s->e.name);
                            goto bail;
                        }
                    }
                }
            }
        }
    }

bail:
    if ((ret) && (pband)) {
        if ((t_s = uci_lookup_option_string(ctx, s, "band"))) {
            if (!strcmp(t_s, "2g"))
                band = band_2g;
            else if (!strcmp(t_s, "5g"))
                band = band_5g;
            else if (!strcmp(t_s, "6g"))
                band = band_6g;
        }
        *pband = band;
    }
    uci_unload(ctx, wireless);
    return ret;
}
#endif


static int _getWiphyVBSSCapability(struct wiphy_private *priv)
{
    bool vbss_onoff = true;
    mac_address fixed_bits_mask = {0};
    mac_address fixed_bits_value = {0};

    if (!priv)
        return 1;

    //clsapi_get_wifi_vbss_enabled(priv->name, &vbss_onoff);
    if (vbss_onoff) {
        priv->vbss_capa.max_vbss = 2;  /* TODO need api supports*/
        priv->vbss_capa.vbss_subtract = 1;  /* TODO need api supports*/
        priv->vbss_capa.vbssid_restrictions = 1;  /* TODO need api supports*/
        priv->vbss_capa.match_and_mask_restrictions = 1; /* TODO need api supports*/
        priv->vbss_capa.fixed_bits_restrictions = 1; /* TODO need api supports*/
        MACCPY(priv->vbss_capa.fixed_bits_mask, fixed_bits_mask); /* TODO need api supports*/
        MACCPY(priv->vbss_capa.fixed_bits_value, fixed_bits_value); /* TODO need api supports*/
    }

    return 0;
}

static struct operating_class *_findOpClass(struct wiphy_private *priv,
                uint8_t opclass, uint32_t max_txpower)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(priv->opclasses); i++) {
        if (priv->opclasses[i].op_class == opclass)
            return &priv->opclasses[i];

        if (priv->opclasses[i].op_class == 0) {
            priv->opclasses[i].op_class = opclass;
            priv->opclasses[i].max_tx_power = max_txpower;
            priv->opclasses[i].num_support_chan = 0;
            priv->opclass_num++;
            return &priv->opclasses[i];
        }
    }

    return NULL;
}

static void _opClassAddChannel(struct wiphy_private *priv, uint8_t opclass,
                uint8_t channel, uint32_t max_txpower)
{
    int i;
    struct operating_class *opclass_info = NULL;

    opclass_info = _findOpClass(priv, opclass, max_txpower);
    if (!opclass_info)
        return;

    for (i = 0; i < ARRAY_SIZE(opclass_info->channels); i++) {
        if (opclass_info->channels[i].id != 0) {
            if (opclass_info->channels[i].id == channel)
                break;
            else
                continue;
        }

        if (opclass_info->num_support_chan > ARRAY_SIZE(opclass_info->channels) - 1)
            break;

        opclass_info->channels[i].id = channel; /* TODO: other para need fill */
        opclass_info->channels[i].pref = MOST_PREF_SCORE; /* max perfered as default */
        opclass_info->num_support_chan++;
        break;
    }

    return;
}

#define OPCLASSES_ADD_CHANNELS(_priv, _num, _opclasses, _channel,  _maxtxpower)\
do {\
    int i;\
    for (i=0;i<(_num);i++) {\
        _opClassAddChannel((_priv), (_opclasses)[i], (_channel), (_maxtxpower));\
    }\
}while(0)

static void _channelAddToOpclass(struct wiphy_private *priv, uint32_t primary_freq,
                uint32_t max_txpower, uint32_t allowed_bw)
{
    uint8_t channel = 0;
    uint8_t opclass = 0;

    if (allowed_bw & ALLOW_CHAN_WIDTH_10) {
        if (!ieee80211Freq2ChannelExt(primary_freq, 0, NL80211_CHAN_WIDTH_10, &opclass, &channel))
            _opClassAddChannel(priv, opclass, channel, max_txpower);
    }

    if (allowed_bw & ALLOW_CHAN_WIDTH_20) {
        if (!ieee80211Freq2ChannelExt(primary_freq, 0, NL80211_CHAN_WIDTH_20, &opclass, &channel))
            _opClassAddChannel(priv, opclass, channel, max_txpower);

        if (!ieee80211Freq2ChannelExt(primary_freq, 0, NL80211_CHAN_WIDTH_20_NOHT, &opclass, &channel))
            _opClassAddChannel(priv, opclass, channel, max_txpower);
    }

    if (allowed_bw & ALLOW_CHAN_WIDTH_40P) {
        if (!ieee80211Freq2ChannelExt(primary_freq, 1, NL80211_CHAN_WIDTH_40, &opclass, &channel))
            _opClassAddChannel(priv, opclass, channel, max_txpower);
    }

    if (allowed_bw & ALLOW_CHAN_WIDTH_40M) {
        if (!ieee80211Freq2ChannelExt(primary_freq, -1, NL80211_CHAN_WIDTH_40, &opclass, &channel))
            _opClassAddChannel(priv, opclass, channel, max_txpower);
    }

    if (allowed_bw & ALLOW_CHAN_WIDTH_80) {
        if (!ieee80211Freq2ChannelExt(primary_freq, 0, NL80211_CHAN_WIDTH_80, &opclass, &channel))
            _opClassAddChannel(priv, opclass,
                primaryChannel2CenterChannel(channel, NL80211_CHAN_WIDTH_80), max_txpower);
    }

    if (allowed_bw & ALLOW_CHAN_WIDTH_160) {
        if (!ieee80211Freq2ChannelExt(primary_freq, 0, NL80211_CHAN_WIDTH_160, &opclass, &channel))
            _opClassAddChannel(priv, opclass,
                primaryChannel2CenterChannel(channel, NL80211_CHAN_WIDTH_160), max_txpower);

        if (!ieee80211Freq2ChannelExt(primary_freq, 0, NL80211_CHAN_WIDTH_80P80, &opclass, &channel))
            _opClassAddChannel(priv, opclass, channel, max_txpower);
    }
}

static void _channelAddToOpclass6G(struct wiphy_private * priv, uint32_t primary_freq,
                uint32_t max_txpower, uint32_t allowed_bw)
{
    uint8_t opclass[5] = {131, 132, 133, 134, 135};
    uint8_t channel = freq2Channel(primary_freq);

    if ((allowed_bw & ALLOW_CHAN_WIDTH_20) ||
        (allowed_bw & ALLOW_CHAN_WIDTH_40P) ||
        (allowed_bw & ALLOW_CHAN_WIDTH_40M) ||
        (allowed_bw & ALLOW_CHAN_WIDTH_80) ||
        (allowed_bw & ALLOW_CHAN_WIDTH_160)) {
        OPCLASSES_ADD_CHANNELS(priv, ARRAY_SIZE(opclass), opclass, channel, max_txpower);
    }

    return;
}

static int _sendRadioInfo(struct wiphy_private *priv)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    int i, j;

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_radio);
    p = msgaPutBin(p, attr_radio_mac, priv->mac, MACLEN);
    p = msgaPutU8(p, attr_radio_id, priv->idx);
    p = msgaPutStr(p, attr_radio_name, (char *)priv->name);
    p = msgaPutU8(p, attr_band_idx, priv->current_band_idx);
    p = msgaPutBin(p, attr_radio_capa, priv->capa, sizeof(priv->capa));

    for (i = 0; i < band_max_idx; i++) {
        uint8_t htcapa = 0;
        uint16_t vhtcapa = 0;
        uint16_t hecapa = 0;

        //only report uci specific band information
        if (priv->band_capa[i].band==band_none)
            continue;
        if ((priv->band!=band_none) && (priv->band!=priv->band_capa[i].band))
            continue;
        p = msgaPutU8(p, attr_band, priv->band_capa[i].band);
        htcapa = transfer2htcapa(priv->band_capa[i].ht_capa, priv->band_capa[i].ht_mcsset);
        p = msgaPutU8(p, attr_ht_capa, htcapa);
        vhtcapa = transfer2vhtcapa(priv->band_capa[i].vht_capa, priv->band_capa[i].vht_rx_mcs, priv->band_capa[i].vht_tx_mcs);
        p = msgaPutU16(p, attr_vht_capa, vhtcapa);
        p = msgaPutU16(p, attr_vht_rx_mcs, priv->band_capa[i].vht_rx_mcs);
        p = msgaPutU16(p, attr_vht_tx_mcs, priv->band_capa[i].vht_tx_mcs);
        hecapa = transfer2hecapa(priv->band_capa[i].he_mac_cap, priv->band_capa[i].he_phy_cap, priv->band_capa[i].he_mcs);
        p = msgaPutU16(p, attr_he_capa, hecapa);
        p = msgaPutBin(p, attr_he_mcs, priv->band_capa[i].he_mcs, priv->band_capa[i].he_mcs_len);
    }

    for (i = 0; i < priv->opclass_num; i++) {
        //only report uci specified operating class
        if ((priv->band!=band_none) && (priv->band!=opclass2Band(priv->opclasses[i].op_class)))
            continue;
        p = msgaPutU8(p, attr_opclass_id, priv->opclasses[i].op_class);
        p = msgaPutU8(p, attr_opclass_max_txpower, priv->opclasses[i].max_tx_power);
        for (j = 0; j < priv->opclasses[i].num_support_chan; j++) {
            p = msgaPutU8(p, attr_opclass_chan_id, priv->opclasses[i].channels[j].id);
            p = msgaPutU8(p, attr_opclass_chan_pref, priv->opclasses[i].channels[j].pref);
            p = msgaPutU8(p, attr_opclass_chan_reason, priv->opclasses[i].channels[j].reason);
            p = msgaPutU8(p, attr_opclass_chan_freq_separation, priv->opclasses[i].channels[j].freq_separation);
        }
    }

    p = msgaPutU8(p, attr_chan_scan_on_boot, priv->scan_capa.scan_bootonly);
    p = msgaPutU8(p, attr_chan_scan_impact, priv->scan_capa.impact_mode);
    p = msgaPutU32(p, attr_chan_scan_min_interval, priv->scan_capa.min_scan_interval);

    /* vbss capabilities */
    p = msgaPutU8(p, attr_max_vbss, priv->vbss_capa.max_vbss);
    p = msgaPutU8(p, attr_vbss_subtract, priv->vbss_capa.vbss_subtract);
    p = msgaPutU8(p, attr_vbssid_restrictions, priv->vbss_capa.vbssid_restrictions);
    p = msgaPutU8(p, attr_matched_and_mask_restrictions, priv->vbss_capa.match_and_mask_restrictions);
    p = msgaPutU8(p, attr_fixed_bits_restrictions, priv->vbss_capa.fixed_bits_restrictions);
    p = msgaPutBin(p, attr_fixed_bits_mask, priv->vbss_capa.fixed_bits_mask, MACLEN);
    p = msgaPutBin(p, attr_fixed_bits_value, priv->vbss_capa.fixed_bits_value, MACLEN);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

static uint8_t *_addBssInfo(uint8_t *p, struct wifi_interface_private *bss)
{

    p = msgaPutBin(p, attr_radio_mac, bss->radio->mac, MACLEN);
    p = msgaPutBin(p, attr_if_mac, bss->mac, MACLEN);
    if (bss->name)
        p = msgaPutBin(p, attr_if_name, (uint8_t *)(bss->name), strlen(bss->name)+1);
    p = msgaPutU32(p, attr_if_idx, bss->index);
    p = msgaPutU8(p, attr_bss_role, bss->role);
    p = msgaPutU8(p, attr_wds, bss->wds);
    if (bss->power_level > 0)
        p = msgaPutU8(p, attr_power_lvl, bss->power_level);
    if (bss->opclass > 0)
        p = msgaPutU8(p, attr_opclass, bss->opclass);
    if (bss->channel > 0) {
        p = msgaPutU8(p, attr_channel, bss->channel);
        p = msgaPutU8(p, attr_bandwidth, bss->bandwidth);
    }
    if (bss->radio->current_band_idx > 0)
        p =  msgaPutU8(p, attr_band_idx, bss->radio->current_band_idx);
    /* ssid need follow current_band_idx for checkBackhaul */
    p = msgaPutBin(p, attr_ssid, (uint8_t *)(bss->ssid), strlen(bss->ssid));
    return p;
}

static int _sendSingleBssInfo(struct wifi_interface_private *bss)
{
    uint8_t *buf = NULL;
    uint8_t *p;

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_bss);

    p = _addBssInfo(p, bss);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

static int _sendBssInfo(struct cls_driver_ctx *ctx)
{
    struct wifi_interface_private *bss;
    uint8_t *buf = NULL;
    uint8_t *p;

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_bss);

    dlist_for_each(bss, ctx->bsses, l) {
        p = _addBssInfo(p, bss);
    }

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

static int _wiphyHandler(struct nl_msg *msg, void *arg)
{
    char *macs=NULL;
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)arg;
    int len;

    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                genlmsg_attrlen(gnlh, 0), NULL);

    if (tb_msg[NL80211_ATTR_WIPHY_NAME]) {
        int idx;
        char *name = nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]);
        uint8_t mac[MACLEN];
        char path[100];
        struct wiphy_private *priv;

#ifdef USE_UCI
        idx = tb_msg[NL80211_ATTR_WIPHY] ? nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]):_phy2PhyIdx(name);
#else
        idx = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
#endif
        if (idx < 0) {
            DEBUG_ERROR("cannot get index for wiphy[%s]\n", name);
            goto bail;
        }

        snprintf(path, 100, "/sys/class/ieee80211/%s/%s", name, "macaddress");
        if (!(macs = _readStrFile(path))) {
            DEBUG_ERROR("cannot get mac address for wiphy[%s]\n", name);
            goto bail;
        }
        if (!ether_aton_r(macs, (struct ether_addr *)mac))
            goto bail;

        if (!(priv = _findWiphyIdx(ctx, idx))) {
            priv = calloc(1, sizeof(struct wiphy_private));
            if (!priv)
                goto bail;
            dlist_head_init(&priv->last_scan_list_head);
            dlist_add_tail(&ctx->radios, &priv->l);

            MACCPY(priv->mac, mac);
            priv->name = strdup(name);
            priv->idx = idx;
            priv->capa[0] |= RADIO_CAPA_SAE;
#ifdef USE_UCI
            priv->uci_name = _getWiphyUCIName(priv->idx, &priv->band);
#endif
        }

        // TODO: driver and nl80211 need add scan capability attribute

        if (tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
            struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
            struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
            struct nlattr *nl_band, *nl_freq;
            int rem_band, rem_freq;

            nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS],
                                rem_band){
                uint8_t band = band_none, band_idx;

                nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
                        nla_len(nl_band), NULL);

                band = bandType2Band(nl_band->nla_type);
                if (band==band_none)
                    continue;
                band_idx = band2BandIdx(band);
                priv->band_capa[band_idx].band = band;

                if (tb_band[NL80211_BAND_ATTR_FREQS]) {
                    uint32_t primary_freq = 0;
                    uint32_t max_txpower = 0;
                    uint8_t allowed_bw = ~0;

                    nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) {
                        nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX,
                              nla_data(nl_freq), nla_len(nl_freq), NULL);

                        if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ] ||
                            tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
                            continue;

                        primary_freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);
                        max_txpower = MBM_TO_DBM(nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]));

                        if (tb_freq[NL80211_FREQUENCY_ATTR_NO_10MHZ])
                            allowed_bw &= ~ALLOW_CHAN_WIDTH_10;
                        if (tb_freq[NL80211_FREQUENCY_ATTR_NO_20MHZ])
                            allowed_bw &= ~ALLOW_CHAN_WIDTH_20;
                        if (tb_freq[NL80211_FREQUENCY_ATTR_NO_HT40_PLUS])
                            allowed_bw &= ~ALLOW_CHAN_WIDTH_40P;
                        if (tb_freq[NL80211_FREQUENCY_ATTR_NO_HT40_MINUS])
                            allowed_bw &= ~ALLOW_CHAN_WIDTH_40M;
                        if (tb_freq[NL80211_FREQUENCY_ATTR_NO_80MHZ])
                            allowed_bw &= ~ALLOW_CHAN_WIDTH_80;
                        if (tb_freq[NL80211_FREQUENCY_ATTR_NO_160MHZ])
                            allowed_bw &= ~ALLOW_CHAN_WIDTH_160;

                        _channelAddToOpclass(priv, primary_freq, max_txpower, allowed_bw);
                        if (band == band_6g)
                            _channelAddToOpclass6G(priv, primary_freq, max_txpower, allowed_bw);
                    }
                }

                if (tb_band[NL80211_BAND_ATTR_HT_CAPA]) {
                    priv->band_capa[band_idx].ht_support = 1;
                    priv->band_capa[band_idx].ht_capa = nla_get_u16(tb_band[NL80211_BAND_ATTR_HT_CAPA]);
                    if (tb_band[NL80211_BAND_ATTR_HT_MCS_SET]) {
                        len = nla_len(tb_band[NL80211_BAND_ATTR_HT_MCS_SET]);
                        if (len > MAX_HT_MCS_SIZE)
                            len = MAX_HT_MCS_SIZE;
                        memcpy(priv->band_capa[band_idx].ht_mcsset, nla_data(tb_band[NL80211_BAND_ATTR_HT_MCS_SET]), len);
                    }
                }

                if (tb_band[NL80211_BAND_ATTR_VHT_CAPA] &&
                        tb_band[NL80211_BAND_ATTR_VHT_MCS_SET]) {
                    uint8_t *mcs = nla_data(tb_band[NL80211_BAND_ATTR_VHT_MCS_SET]);

                    priv->band_capa[band_idx].vht_support = 1;
                    len = nla_len(tb_band[NL80211_BAND_ATTR_VHT_CAPA]);
                    if (len > 4)
                        len = 4;
                    memcpy(priv->band_capa[band_idx].vht_capa, nla_data(tb_band[NL80211_BAND_ATTR_VHT_CAPA]), len);
                    priv->band_capa[band_idx].vht_rx_mcs = (uint16_t)IEEE80211_VHTCAP_GET_RX_MCS_NSS(mcs);
                    priv->band_capa[band_idx].vht_tx_mcs = (uint16_t)IEEE80211_VHTCAP_GET_TX_MCS_NSS(mcs);
                }

                if (tb_band[NL80211_BAND_ATTR_IFTYPE_DATA]) {
                    struct nlattr *tb[NL80211_BAND_IFTYPE_ATTR_MAX + 1];
                    struct nlattr *nl_iftype;
                    int rem;
                    /* TODO: we only store one mode. */
                    nla_for_each_nested(nl_iftype, tb_band[NL80211_BAND_ATTR_IFTYPE_DATA], rem) {
                        nla_parse(tb, NL80211_BAND_IFTYPE_ATTR_MAX,
                              nla_data(nl_iftype), nla_len(nl_iftype), NULL);

                        priv->band_capa[band_idx].he_support = 1;

                        if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]) {
                            len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]);

                            if (len > sizeof(priv->band_capa[band_idx].he_phy_cap))
                                len = sizeof(priv->band_capa[band_idx].he_phy_cap);
                            memcpy(priv->band_capa[band_idx].he_phy_cap,
                                  nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]),
                                  len);
                        }

                        if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC]) {
                            len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC]);

                            if (len > sizeof(priv->band_capa[band_idx].he_mac_cap))
                                len = sizeof(priv->band_capa[band_idx].he_mac_cap);
                            memcpy(priv->band_capa[band_idx].he_mac_cap,
                                  nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC]),
                                  len);
                        }

                        if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET]) {
                            len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET]);

                            if (len > MAX_HE_MCS_SIZE)
                                len = MAX_HE_MCS_SIZE;
                            priv->band_capa[band_idx].he_mcs_len = len;
                            memcpy(priv->band_capa[band_idx].he_mcs,
                                  nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET]),
                                  len);
                        }

                        if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE]) {
                            len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE]);

                            if (len > sizeof(priv->band_capa[band_idx].he_ppet))
                                len = sizeof(priv->band_capa[band_idx].he_ppet);
                            memcpy(&priv->band_capa[band_idx].he_ppet,
                                  nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE]),
                                  len);
                        }

                        if (tb[NL80211_BAND_IFTYPE_ATTR_HE_6GHZ_CAPA]) {
                            priv->band_capa[band_idx].he_6ghz_capa = nla_get_u16(tb[NL80211_BAND_IFTYPE_ATTR_HE_6GHZ_CAPA]);
                        }
                    }
                }
            }
        }
    }

bail:
    return NL_SKIP;
}

void _getWiphyTimerHandler(void *data)
{
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)data;
    struct wiphy_private *priv = NULL;

    ctx->timer = NULL;

    _issueCMD(ctx->cmd_ctrl, NULL, -1, -1, -1, (ctx->split_wiphy_dump ? NL80211_ATTR_SPLIT_WIPHY_DUMP:-1),
                    NL80211_CMD_GET_WIPHY, NLM_F_DUMP,
                    _wiphyHandler, ctx);

    dlist_for_each(priv, ctx->radios, l) {
        /* get radio vbss capabilities by cls_api */
        _getWiphyVBSSCapability(priv);
        /* notify datamodel radio info */
        _sendRadioInfo(priv);
    }
}

void _startGetWiphyTimer(struct cls_driver_ctx *ctx)
{
    if (ctx) {
        if (!ctx->timer)
            ctx->timer = platformAddTimer(3000, 0, _getWiphyTimerHandler, ctx);
    }
}

static int _wifiInterfaceHandler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)arg;
    struct wiphy_private *priv = NULL;
    struct wifi_interface_private *wif_priv = NULL;
    uint32_t wiphy_id = -1;

    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                genlmsg_attrlen(gnlh, 0), NULL);

    if (tb_msg[NL80211_ATTR_WIPHY])
        wiphy_id = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
    else
        goto bail;

    priv = _findWiphyIdx(ctx, wiphy_id);
    if (!priv) {
        DEBUG_ERROR("cannot find radio by id: %u, try to get Wiphy info again\n", wiphy_id);
        _startGetWiphyTimer(ctx);
        goto bail;
    }

    if (tb_msg[NL80211_ATTR_MAC]) {
        int ifidx;
        uint8_t iftype;

        if ((!tb_msg[NL80211_ATTR_IFINDEX]) || (!tb_msg[NL80211_ATTR_IFTYPE]))
            goto bail;

        iftype = nla_get_u32(tb_msg[NL80211_ATTR_IFTYPE]);
        if ((iftype!=NL80211_IFTYPE_AP) && (iftype!=NL80211_IFTYPE_STATION))
            goto bail;

        ifidx = nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]);
        wif_priv = _wifiInterfaceNew(ctx, ifidx, nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));

        MACCPY(wif_priv->mac, nla_data(tb_msg[NL80211_ATTR_MAC]));
        wif_priv->radio = priv;

        if (iftype == NL80211_IFTYPE_AP)
            wif_priv->role = role_ap;
        else
            wif_priv->role = role_sta;

        if (tb_msg[NL80211_ATTR_IFNAME]) {
            char *if_name = nla_get_string(tb_msg[NL80211_ATTR_IFNAME]);
            if (!priv->primary_name)
                priv->primary_name = strdup(if_name);
            else if (strcmp(wif_priv->name, if_name)) {
                free(priv->primary_name);
                priv->primary_name = strdup(if_name);
            }
        }

        if (tb_msg[NL80211_ATTR_SSID]) {
            if (nla_len(tb_msg[NL80211_ATTR_SSID]) > MAX_SSID_LEN)
                goto bail;
            memcpy(wif_priv->ssid, nla_data(tb_msg[NL80211_ATTR_SSID]),
                    nla_len(tb_msg[NL80211_ATTR_SSID]));
        }

        if (tb_msg[NL80211_ATTR_4ADDR])
            wif_priv->wds = nla_get_u8(tb_msg[NL80211_ATTR_4ADDR]);

        if (tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL])
            wif_priv->power_level = MBM_TO_DBM(nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]));

        if ((tb_msg[NL80211_ATTR_WIPHY_FREQ]) && (tb_msg[NL80211_ATTR_CENTER_FREQ1])
            && (tb_msg[NL80211_ATTR_CHANNEL_WIDTH])) {
            uint32_t freq = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]);
            int sec_chan = 0;
            wif_priv->radio->current_band_idx = freq2BandIdx(freq);
            wif_priv->bandwidth = nla_get_u32(tb_msg[NL80211_ATTR_CHANNEL_WIDTH]);

            if (wif_priv->bandwidth==NL80211_CHAN_WIDTH_40) {
                uint32_t center_freq = nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ1]);
                sec_chan = (freq<=center_freq) ? 1:-1;
            }

            /* calculate current opclass and channel */
            if (ieee80211Freq2ChannelExt(freq, sec_chan, wif_priv->bandwidth, &wif_priv->opclass, &wif_priv->channel)) {
                DEBUG_WARNING("ieee80211Freq2ChannelExt return fail\n");
            }
#if 0 //maynot need transfer to central channel
            /* transfer primary channel to center channel if opclass >= 128 */
            if (wif_priv->opclass >= 128)
                wif_priv->channel = primaryChannel2CenterChannel(wif_priv->channel, wif_priv->bandwidth);
#endif
        }
    }

bail:
    return NL_SKIP;
}

static void _nl80211ChanSwitchEventHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    struct wifi_interface_private *priv = NULL;
    int ifidx;
    uint8_t *buf = NULL;
    uint8_t *p;
    uint32_t bw, freq, cfreq;
    uint8_t opclass, channel;
    int sec_chan = 0;

    if (!tb[NL80211_ATTR_WIPHY_FREQ] ||
        !tb[NL80211_ATTR_IFINDEX] ||
        !tb[NL80211_ATTR_CHANNEL_WIDTH]||
        !tb[NL80211_ATTR_CENTER_FREQ1])
        return;

    DEBUG_INFO("receive NL80211_CMD_CH_SWITCH_NOTIFY event on interface(%d)\n", nla_get_u32(tb[NL80211_ATTR_IFINDEX]));

    ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
    if (!(priv = _findWifiInterfaceIdx(ctx, ifidx)))
        return;
    buf = msgGetBuf(0);
    p = buf;

    freq = nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]);
    cfreq = nla_get_u32(tb[NL80211_ATTR_CENTER_FREQ1]);
    bw = nla_get_u32(tb[NL80211_ATTR_CHANNEL_WIDTH]);
    if (bw == NL80211_CHAN_WIDTH_40) {
        sec_chan = (freq<=cfreq) ? 1:-1;
    }
    /* calculate current opclass and channel */
    if (ieee80211Freq2ChannelExt(freq, sec_chan, bw, &opclass, &channel)) {
        DEBUG_ERROR("can not convert opclass for freq=%d, bw=%d\n", freq, bw);
    }

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_radio);

    p = msgaPutBin(p, attr_radio_mac, priv->radio->mac, MACLEN);
    p = msgaPutU8(p, attr_channel, channel);
    p = msgaPutU8(p, attr_opclass, opclass);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return;
}

static void _nl80211AssocReqHandle(struct cls_driver_ctx *ctx, struct wifi_interface_private *wif,
                                uint8_t *frame, uint32_t frame_len) {
    uint8_t *bssid;
    uint8_t *client_mac;
    struct ieee80211_header *mgmt = (struct ieee80211_header *)frame;

    bssid = mgmt->bssid;
    client_mac = mgmt->sa;

    _assocReqFrameAdd(ctx, bssid, client_mac, frame, frame_len);
    DEBUG_INFO("add assoc req frame. client: "MACFMT", bssid: "MACFMT"\n", MACARG(client_mac), MACARG(bssid));

    return;
}

/* monitor associate response frame tx status if status code in associate response then notify datamodel add station */
static void _nl80211FrameTxStatusHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    uint8_t *client_mac = NULL;
    uint8_t *bssid = NULL;
    struct assoc_req_data *assoc = NULL;
    struct ieee80211_header *mgmt;
    struct wifi_interface_private *wif = NULL;

    if (!tb[NL80211_ATTR_FRAME])
        return;

    mgmt = (struct ieee80211_header *)nla_data(tb[NL80211_ATTR_FRAME]);

    switch (mgmt->fc[0] & IEEE80211_FC0_SUBTYPE_MASK) {
        case IEEE80211_FC0_SUBTYPE_ASSOC_RESP:
        case IEEE80211_FC0_SUBTYPE_REASSOC_RESP:
            uint16_t *status_code = (uint16_t *)((uint8_t *)mgmt + sizeof(struct ieee80211_header) + 2);
            DEBUG_INFO("receive assoc/reassoc response tx status handle, status_code: %u\n", *status_code);
            bssid = mgmt->bssid;
            client_mac = mgmt->da;
            if (*status_code != WLAN_STATUS_SUCCESS)
                return;
            break;
        default:
            return;
            break;
    }

    if (!client_mac || !bssid)
        return;

    wif = _findWifiInterface(ctx, bssid);
    if (!wif) {
        DEBUG_ERROR("wifi interface("MACFMT") does not exist.\n", MACARG(bssid));
        return;
    }

    assoc = _assocReqFrameFind(ctx, client_mac);
    if(!assoc) {
        DEBUG_ERROR("can't find assoc req of sta("MACFMT") return.\n", MACARG(client_mac));
        return;
    }

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_station);

    p = msgaPutU32(p, attr_if_idx, wif->index);
    p = msgaPutBin(p, attr_mac, client_mac, MACLEN);
    if (assoc && assoc->frame) {
        p = msgaPutU32(p, attr_ts, assoc->ts);
        p = msgaPutBin(p, attr_frame, assoc->frame, assoc->frame_len);
        _assocReqFrameDel(ctx, client_mac);
    }

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return;
}

static void _nl80211DeleteStationHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    int ifidx;
    uint8_t *mac;
    uint16_t reason;
    struct wifi_interface_private *wif = NULL;

    if (!tb[NL80211_ATTR_IFINDEX] ||
        !tb[NL80211_ATTR_MAC])
        return;

    ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
    mac = nla_data(tb[NL80211_ATTR_MAC]);

    /* FIXME if bug-672 is resolved, now we fix the reason_code to 2, PRIO_AUTH_NOT_VALID */
//    reason = nla_get_u16(tb[NL80211_ATTR_REASON_CODE]);
    reason = 2;

    wif = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif) {
        DEBUG_ERROR("wifi interface(%d) does not exist.\n", ifidx);
        return;
    }

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_station_delete);

    p = msgaPutU32(p, attr_if_idx, wif->index);
    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutU16(p, attr_reason_code, reason);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    _assocReqFrameDel(ctx, mac);

    return;
}

static void _nl80211NewScanResultsHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    int ifidx;
    struct wifi_interface_private *wif = NULL;

    if (!tb[NL80211_ATTR_IFINDEX])
        return;

    ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);

    wif = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif || !wif->radio) {
        DEBUG_ERROR("wifi interface(%d) does not exist.\n", ifidx);
        return;
    }

    wif->radio->channel_scan_status = CHANNEL_SCAN_FINISHED;

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_chan_scan_finished);

    p = msgaPutU32(p, attr_if_idx, wif->index);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return;
}

static void _nl80211ConnectHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    int ifidx;
    struct wifi_interface_private *wif = NULL;

    if ((!tb[NL80211_ATTR_IFINDEX]) || (!tb[NL80211_ATTR_STATUS_CODE]) ||
        (!tb[NL80211_ATTR_MAC]))
        return;

    ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);

    wif = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif || !wif->radio || (wif->role!=role_sta)) {
        DEBUG_ERROR("wifi interface(%d) does not exist.\n", ifidx);
        return;
    }

    if (nla_get_u16(tb[NL80211_ATTR_STATUS_CODE])!=0)
        return;

    buf = msgGetBuf(0);
    p = buf;
    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_connect);
    p = msgaPutU32(p, attr_if_idx, wif->index);
    p = msgaPutBin(p, attr_bssid, nla_data(tb[NL80211_ATTR_MAC]), MACLEN);

    msgSend(buf, p - buf);
    msgPutBuf(buf);
}

static void _nl80211DisConnectHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    int ifidx;
    struct wifi_interface_private *wif = NULL;

    if (!tb[NL80211_ATTR_IFINDEX])
        return;

    ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);

    wif = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif || !wif->radio || (wif->role!=role_sta)) {
        DEBUG_ERROR("wifi interface(%d) does not exist.\n", ifidx);
        return;
    }

    buf = msgGetBuf(0);
    p = buf;
    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_disconnect);
    p = msgaPutU32(p, attr_if_idx, wif->index);

    msgSend(buf, p - buf);
    msgPutBuf(buf);
}

static void _sendRegisterdMgmtFrame(struct cls_driver_ctx *ctx, struct wifi_interface_private *wif,
                                                uint8_t *frame, uint32_t frame_len)
{
    uint8_t *p, *buf;

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_mgmt_frame);

    p = msgaPutU32(p, attr_if_idx, wif->index);
    p = msgaPutBin(p, attr_frame, frame, frame_len);

    msgSend(buf, p - buf);
    msgPutBuf(buf);

    return;
}

#if 0
static uint8_t _frame_is_registered(struct cls_driver_ctx *ctx, int ifIdx, uint8_t *frame, uint16_t frame_len)
{
    struct ieee80211_hdr *wifi_hdr = (struct ieee80211_hdr *)frame;
    struct frame_registered_item *reg_frm;
    uint8_t *cursor = frame + sizeof(struct ieee80211_hdr);

    dlist_for_each(reg_frm, ctx->registered_frames, l) {
        if (ifIdx != reg_frm->if_idx)
            continue;
        if ((frame_len - sizeof(struct ieee80211_hdr)) < reg_frm->match_len)
            continue;
        if ((wifi_hdr->fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == reg_frm->f_type) {
            if (0 == memcmp(cursor, reg_frm->match, reg_frm->match_len))
                return 1;
        }
    }
    return 0;
}

static void _recordRegisteredFrame(struct cls_driver_ctx *ctx, int ifIdx, uint32_t match_len,
                                                    char *match, uint16_t frame_type)
{
    struct frame_registered_item *reg_frm;
    void *new_match = NULL;
    bool is_new_frm = true;

    /* the frame_type is already recored */
    dlist_for_each(reg_frm, ctx->registered_frames, l) {
        if (reg_frm->f_type == frame_type) {
            if (reg_frm->match_len < match_len) { /* original space is NOT enough */
                new_match = realloc(reg_frm->match, match_len);
                if (!new_match) /* the original registered frame will NOT be affect */
                    return;
                reg_frm->match = new_match;
                memcpy(reg_frm->match, match, match_len);
            }
            else {
                if (match_len) {  /* original space is enough */
                    memset(reg_frm->match, 0, reg_frm->match_len);
                    memcpy(reg_frm->match, match, match_len);
                }
            }
            reg_frm->match_len = match_len;
            reg_frm->if_idx = ifIdx;
            is_new_frm = false;
        }
    }
     /* new frame_type */
    if (is_new_frm) {
        reg_frm = calloc(1, sizeof(struct frame_registered_item));
        if (!reg_frm)
            return;
        reg_frm->if_idx = ifIdx;
        reg_frm->f_type = frame_type;
        reg_frm->match_len = match_len;
        if (match_len) {
            reg_frm->match = calloc(1, match_len);
            if (!reg_frm->match) {
                free(reg_frm);
                return;
            }
            memcpy(reg_frm->match, match, match_len);
        }
        dlist_add_tail(&ctx->registered_frames, &reg_frm->l);
    }
}
#endif

static void _nl80211RegisterFrameHandle(struct cls_driver_ctx *ctx, struct nlattr **tb)
{
    int ifidx;
    uint8_t *frame;
    uint32_t frame_len;
    struct ieee80211_header *mgmt;
    struct wifi_interface_private *wif = NULL;

    if (!tb[NL80211_ATTR_IFINDEX] ||
        !tb[NL80211_ATTR_FRAME])
        return;

    ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
    frame = nla_data(tb[NL80211_ATTR_FRAME]);
    frame_len = nla_len(tb[NL80211_ATTR_FRAME]);
    mgmt = (struct ieee80211_header *)frame;

    wif = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif) {
        DEBUG_ERROR("wifi interface(%d) does not exist.\n", ifidx);
        return;
    }

    if (IS_ZERO_MAC(mgmt->bssid) || IS_ZERO_MAC(mgmt->sa)) {
        DEBUG_ERROR("illegal bssid or source mac: bssid["MACFMT"], sa["MACFMT"].\n",
            MACARG(mgmt->bssid), MACARG(mgmt->sa));
        return;
    }

    switch (mgmt->fc[0] & IEEE80211_FC0_SUBTYPE_MASK) {
        case IEEE80211_FC0_SUBTYPE_ASSOC_REQ:
        case IEEE80211_FC0_SUBTYPE_REASSOC_REQ:
            DEBUG_INFO("receive assoc/reassoc request\n");
            _nl80211AssocReqHandle(ctx, wif, frame, frame_len);
            break;
        default:
            break;
    }
    if (0 == (mgmt->fc[0] & IEEE80211_FC0_TYPE_MASK)) {
        _sendRegisterdMgmtFrame(ctx, wif, frame, frame_len);
    }
    return;
}

static int _eventHandler(struct nl_msg *msg, void *arg)
{
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)arg;
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    int ifidx = -1, wiphy_idx = -1;

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);

    if (tb[NL80211_ATTR_IFINDEX])
        ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
    else if (tb[NL80211_ATTR_WIPHY])
        wiphy_idx = nla_get_u32(tb[NL80211_ATTR_WIPHY]);

    DEBUG_DETAIL("receive nl80211 event %s, wiphy=%d, if=%d\n", getNlCmdName(gnlh->cmd), wiphy_idx, ifidx);

    if (ifidx < 0 && wiphy_idx < 0)
        return NL_SKIP;

    switch (gnlh->cmd) {
    case NL80211_CMD_CH_SWITCH_NOTIFY:
        _nl80211ChanSwitchEventHandle(ctx, tb);
        break;
    case NL80211_CMD_FRAME:
        _nl80211RegisterFrameHandle(ctx, tb);
        break;
    case NL80211_CMD_FRAME_TX_STATUS:
        _nl80211FrameTxStatusHandle(ctx, tb);
        break;
    case NL80211_CMD_DEL_STATION:
        _nl80211DeleteStationHandle(ctx, tb);
        break;
    case NL80211_CMD_NEW_SCAN_RESULTS:
        _nl80211NewScanResultsHandle(ctx, tb);
        break;
    case NL80211_CMD_CONNECT:
        _nl80211ConnectHandle(ctx, tb);
        break;
    case NL80211_CMD_DISCONNECT:
        _nl80211DisConnectHandle(ctx, tb);
        break;
    default:
        break;
    }

    return NL_SKIP;
}

static int _nlFeatureHandler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)arg;

    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);

    if (tb_msg[NL80211_ATTR_PROTOCOL_FEATURES]) {
        uint32_t feat = nla_get_u32(tb_msg[NL80211_ATTR_PROTOCOL_FEATURES]);

        if (feat & NL80211_PROTOCOL_FEATURE_SPLIT_WIPHY_DUMP) {
            ctx->split_wiphy_dump = 1;
        }
    }

    return NL_SKIP;
}

static void *_nl80211EventLoop(void *data)
{
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)data;
    struct sock_ctrl *ctrl = ctx->event_ctrl;
    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);

    _registerHandler(ctrl, _eventHandler, ctx);

    nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, _eventNoSeqCheck, NULL);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _cmdValid, ctrl);

    while (1) {
        nl_recvmsgs(ctrl->sock, cb);
    }

    return NULL;
}

void _startGetWifInterfaceTimer(struct cls_driver_ctx *ctx, struct wifi_interface_private *wif_priv);
static int _nl80211GetStation(struct cls_driver_ctx *ctx, uint32_t ifidx, uint8_t *mac);

void _rtmLinkTimerHandle(void *data)
{
    struct wifi_interface_private *bss = (struct wifi_interface_private *)data;
    struct cls_driver_ctx *ctx = bss->ctx;

    bss->timer = NULL;

    if (bss->delete) {
        _wifiInterfaceDelete(bss);
        return;
    };

    _issueCMD(ctx->cmd_ctrl, NULL, -1, bss->index, -1, -1,
                NL80211_CMD_GET_INTERFACE, 0, _wifiInterfaceHandler, ctx);

    /* if get ssid or current channel failed need restart the timer to try again util get all params successfully */
    if (!WIFI_INTERFACE_INFO_CORRECT(bss)) {
        DEBUG_ERROR("bss(index=%d, ifname:%s, ssid:%s, chan:%u, try again!\n", bss->index, bss->name,
                bss->ssid, bss->channel);
        _startGetWifInterfaceTimer(ctx, bss);
    }
    else {
        _addHostapdListener(bss);
        _sendSingleBssInfo(bss);
        if (bss->role == role_ap)
            _nl80211GetStation(ctx, bss->index, NULL);
    }
}

void _startGetWifInterfaceTimer(struct cls_driver_ctx *ctx,  struct wifi_interface_private *wif_priv)
{
    if (wif_priv) {
        if (!wif_priv->timer)
            wif_priv->timer = platformAddTimer(3000, 0, _rtmLinkTimerHandle, wif_priv);
    }
}

static int _rtmProessDelLink(struct cls_driver_ctx *ctx, struct ifinfomsg *ifi)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    struct wifi_interface_private *priv = _findWifiInterfaceIdx(ctx, ifi->ifi_index);

    if (priv) {
        _wifiInterfaceDelete(priv);

        buf = msgGetBuf(0);
        p = buf;

        MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_bss_delete);
        p = msgaPutU32(p, attr_if_idx, ifi->ifi_index);

        msgSend(buf, p-buf);
        msgPutBuf(buf);
    }

    return 0;
}

static int _isWifiInterface(char *ifname)
{
    char path[256] = {0};
    snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", ifname);

    return (access(path, F_OK) == 0);
}

static int _rtmProcessNewLink(struct cls_driver_ctx *ctx, struct ifinfomsg *ifi, uint8_t *buf, size_t len)
{

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP (1<<16)
#endif
    int attrlen = len;
    struct rtattr *attr = (struct rtattr *) buf;
    char name[IFNAMSIZ + 1] = {0};
    if_indextoname(ifi->ifi_index, name);
    while (RTA_OK(attr, attrlen)) {
        switch(attr->rta_type) {
            case IFLA_LINKMODE:
                if (((ifi->ifi_flags & (IFF_RUNNING|IFF_LOWER_UP | IFF_UP))==(IFF_RUNNING |IFF_LOWER_UP| IFF_UP)) &&
                    (_isWifiInterface(name))) {
                    struct wifi_interface_private *wif_priv = _wifiInterfaceNew(ctx, ifi->ifi_index, name);
                    /* add timer to get wifi interface */
                    _startGetWifInterfaceTimer(ctx, wif_priv);
                }
                break;
            default:
                break;
        }

        attr = RTA_NEXT(attr, attrlen);
    }


    return 0;
}

static int _rtmHandler(struct nl_msg *msg, void *arg)
{
    struct nlmsghdr *h = nlmsg_hdr(msg);
    struct ifinfomsg *ifi;
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)arg;

    switch (h->nlmsg_type) {
        case RTM_NEWLINK:
            ifi = NLMSG_DATA(h);
            _rtmProcessNewLink(ctx, ifi, (uint8_t *) NLMSG_DATA(h) + NLMSG_ALIGN(sizeof(struct ifinfomsg)),
                    NLMSG_PAYLOAD(h, sizeof(struct ifinfomsg)));
            break;
        case RTM_DELLINK:
            ifi = NLMSG_DATA(h);
            _rtmProessDelLink(ctx, ifi);
            break;
        default:
            break;
    }

    return NL_SKIP;
}

static void *_rtmEventLoop(void *data)
{
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)data;
    struct sock_ctrl *ctrl = ctx->rtm_ctrl;
    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
    int ret;

    _registerHandler(ctx->rtm_ctrl, _rtmHandler, ctx);

    nl_cb_err(cb, NL_CB_CUSTOM, _cmdError, &ret);
    nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, _eventNoSeqCheck, NULL);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _cmdValid, ctrl);

    while (1) {
        nl_recvmsgs(ctrl->sock, cb);
    }

    return NULL;
}

static int _issueCMD(struct sock_ctrl *ctrl, struct nl_msg *msg, int wiphy, int intf, int wdev,
                    int ext_flag, int8_t cmd, int flag,
                    int (*handler)(struct nl_msg *, void *), void *data)
{
    struct nl_cb *cb, *s_cb;
    int err = 0;
    NL_SOCK_MUTEX_LOCK(ctrl);

    if (!msg) {
        msg = nlmsg_alloc();

        genlmsg_put(msg, 0, 0, ctrl->id, 0,
                        flag, cmd, 0);
        if (ext_flag >= 0)
            nla_put_flag(msg, ext_flag);

        if (wiphy >= 0)
            NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, wiphy);

        if (intf >= 0)
            NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, intf);

        if (wdev >= 0)
            NLA_PUT_U64(msg, NL80211_ATTR_WDEV, wdev);
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    s_cb = nl_cb_alloc(NL_CB_DEFAULT);

    nl_socket_set_cb(ctrl->sock, s_cb);

    err = nl_send_auto_complete(ctrl->sock, msg);

    if (err<0) {
        DEBUG_ERROR("nl_send_auto_complete() failed: (%d)%s", err, nl_geterror(err));
        /* Need to convert libnl error code to an errno value. For now,
         * just hardcode this to EBADF; the real error reason is shown
         * in that error print above. */
        err = -EBADF;
        goto bail;
    }

    _registerHandler(ctrl, handler, data);

    nl_cb_err(cb, NL_CB_CUSTOM, _cmdError, &err);
    nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, _cmdSeqCheck, msg);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, _cmdFinish, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, _cmdAck, &err);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _cmdValid, ctrl);

    while (err > 0) {
        int res = nl_recvmsgs(ctrl->sock, cb);
        if (res == -NLE_DUMP_INTR) {
            /* Most likely one of the nl80211 dump routines hit a
             * case where internal results changed while the dump
             * was being sent. The most common known case for this
             * is scan results fetching while associated were every
             * received Beacon frame from the AP may end up
             * incrementing bss_generation. This
             * NL80211_CMD_GET_SCAN case tries again in the caller;
             * other cases (of which there are no known common ones)
             * will stop and return an error. */
            err = -EAGAIN;
        } else if (res < 0) {
            DEBUG_ERROR("nl_recvmsgs failed: %d (%s) \n",
                res, nl_geterror(res));
        }
    }

bail:
    nl_cb_put(cb);
    nl_cb_put(s_cb);
    nlmsg_free(msg);
    NL_SOCK_MUTEX_UNLOCK(ctrl);
    return err;
nla_put_failure:
    nlmsg_free(msg);
    NL_SOCK_MUTEX_UNLOCK(ctrl);
    return err;
}

static int _issueCMDNoWait(struct sock_ctrl *ctrl, struct nl_msg *msg, int wiphy, int intf, int wdev,
                    int ext_flag, int8_t cmd, int flag)
{
    struct nl_cb *cb, *s_cb;
    int err, ret = 0;


    NL_SOCK_MUTEX_LOCK(ctrl);

    if (!msg) {
        msg = nlmsg_alloc();

        genlmsg_put(msg, 0, 0, ctrl->id, 0,
                        flag, cmd, 0);
        if (ext_flag >= 0)
            nla_put_flag(msg, ext_flag);

        if (wiphy >= 0)
            NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, wiphy);

        if (intf >= 0)
            NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, intf);

        if (wdev >= 0)
            NLA_PUT_U64(msg, NL80211_ATTR_WDEV, wdev);
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    s_cb = nl_cb_alloc(NL_CB_DEFAULT);

    nl_socket_set_cb(ctrl->sock, s_cb);

    err = nl_send_auto_complete(ctrl->sock, msg);

    if (err<0) {
        DEBUG_ERROR("nl_send_auto_complete failed, err=%d\n", err);
        ret = err;
        goto bail;
    }

bail:
    nl_cb_put(cb);
    nl_cb_put(s_cb);
    nlmsg_free(msg);
    NL_SOCK_MUTEX_UNLOCK(ctrl);
    return ret;
nla_put_failure:
    nlmsg_free(msg);
    NL_SOCK_MUTEX_UNLOCK(ctrl);
    return ret;
}

#ifdef USE_UCI
static int _uciAddAP(struct wiphy_private * priv, struct bss_info *bss, struct bss_info *backhaul_bss)
{
    int ret = -1;
    struct uci_context *ctx;
    struct uci_package *wireless;
    struct uci_ptr ptr = {0};
    int multi_ap = 0;
    char str[16], sec_str[16];
    int vlanstart = 0;
    const char *uci_crypt = get_uci_encryption(bss->auth, bss->encrypt);

    if (!uci_crypt)
        return ret;

    ctx = _getUCICtx();
    ret = uci_load(ctx, "wireless", &wireless);
    ptr.p = wireless;

    snprintf(sec_str, sizeof(sec_str), "wifinet%d_%d", priv->idx, ++priv->ap_num);

    UCI_ADD_SECTION(ctx, ptr, sec_str, "wifi-iface");

    UCI_ADD_OPTION(ctx, ptr, "device", priv->uci_name);
    UCI_ADD_OPTION(ctx, ptr, "network", "lan");
    UCI_ADD_OPTION(ctx, ptr, "mode", "ap");
    UCI_ADD_OPTION(ctx, ptr, "ssid", (const char *)bss->ssid.ssid);
    UCI_ADD_OPTION(ctx, ptr, "encryption", uci_crypt);
    UCI_ADD_OPTION(ctx, ptr, "bss_transition", "1");
    UCI_ADD_OPTION(ctx, ptr, "rrm_neighbor_report", "1");

    //FIXME, add key to hexadecimal string for 64 byte key
    UCI_ADD_OPTION(ctx, ptr, "key", (const char *)bss->key.key);

    if (bss->backhaul) {
        int i=0;
        UCI_ADD_OPTION(ctx, ptr, "wds", "1");
        UCI_ADD_OPTION(ctx, ptr, "hidden", "1");
        multi_ap |= 0x01;

        while (bss->vlan_map[i]) {
            if (!vlanstart) {
                UCI_START_OPTION_LIST(ptr, "brvlan");
                vlanstart = 1;
            }
            if (0 == i)
                snprintf(str, sizeof(str), "%d:t*", bss->vlan_map[i]);
            else
                snprintf(str, sizeof(str), "%d:t", bss->vlan_map[i]);
            UCI_ADD_OPTION_LIST(ctx, ptr, str);
            i++;
        }
    } else {
         if (bss->vlan_map[0]) {
            snprintf(str, sizeof(str), "%d:u*", bss->vlan_map[0]);
            UCI_START_OPTION_LIST(ptr, "brvlan");
            UCI_ADD_OPTION_LIST(ctx, ptr, str);
        }
    }

    if (bss->fronthaul) {
        multi_ap |= 0x02;
    }

    if (multi_ap > 0) {
        snprintf(str, sizeof(str), "%d", multi_ap);
        UCI_ADD_OPTION(ctx, ptr, "multi_ap", (const char *)str);
        snprintf(str, sizeof(str), "%d", local_config.profile);
        UCI_ADD_OPTION(ctx, ptr, "multi_ap_profile", (const char *)str);
    }

    //TODO add backhaul bss configuration to uci
    if (bss->fronthaul && backhaul_bss && backhaul_bss->ssid.len > 0) {
        UCI_ADD_OPTION(ctx, ptr, "wps_pushbutton", "1");
        UCI_ADD_OPTION(ctx, ptr, "multi_ap_backhaul_ssid", (const char *)backhaul_bss->ssid.ssid);
        UCI_ADD_OPTION(ctx, ptr, "multi_ap_backhaul_key", (const char *)backhaul_bss->key.key);
    }

    ret = uci_save(ctx, wireless);
    ret = uci_commit(ctx, &wireless, true);

    ret = uci_unload(ctx, wireless);

    return ret;
}

static int _uciAddSTA(struct wiphy_private * priv, struct bss_info *bss)
{
    int ret = -1;
    struct uci_context *ctx;
    struct uci_package *wireless;
    struct uci_section *sec;
    struct uci_element *e;
    struct uci_ptr ptr = {0};
    const char *uci_crypt = get_uci_encryption(bss->auth, bss->encrypt);
    char str[16], sec_str[16];

    if (!uci_crypt)
        return ret;

    ctx = _getUCICtx();
    ret = uci_load(ctx, "wireless", &wireless);
    ptr.p = wireless;

    snprintf(str, sizeof(str), "%d", local_config.profile);

    uci_foreach_element(&wireless->sections, e) {
        sec = uci_to_section(e);
        if (strcmp(sec->type, "wifi-iface") == 0) {
            const char *mode;
            const char *dev;

            mode = uci_lookup_option_string(ctx, sec, "mode");
            dev = uci_lookup_option_string(ctx, sec, "device");

            if ((mode) && (dev) && (!strcmp(mode, "sta")) && (!strcmp(dev, priv->uci_name))) {
                ptr.s = sec;
                UCI_ADD_OPTION(ctx, ptr, "ssid", (const char *)bss->ssid.ssid);
                UCI_ADD_OPTION(ctx, ptr, "encryption", uci_crypt);
                UCI_ADD_OPTION(ctx, ptr, "key", (const char *)bss->key.key);
                UCI_ADD_OPTION(ctx, ptr, "wds", "1");
                UCI_ADD_OPTION(ctx, ptr, "multi_ap", "1");
                snprintf(str, sizeof(str), "%d", local_config.profile);
                UCI_ADD_OPTION(ctx, ptr, "multi_ap_profile", (const char *)str);
                goto out;
            }
        }
    }
    //add new sta interface and make it disabled
    snprintf(sec_str, sizeof(sec_str), "wlan%d", priv->idx);

    UCI_ADD_SECTION(ctx, ptr, sec_str, "wifi-iface");
    UCI_ADD_OPTION(ctx, ptr, "ifname", "wlan-sta");
    UCI_ADD_OPTION(ctx, ptr, "device", priv->uci_name);
    UCI_ADD_OPTION(ctx, ptr, "network", "lan");
    UCI_ADD_OPTION(ctx, ptr, "mode", "sta");
    UCI_ADD_OPTION(ctx, ptr, "disabled", "1");
    UCI_ADD_OPTION(ctx, ptr, "ssid", (const char *)bss->ssid.ssid);
    UCI_ADD_OPTION(ctx, ptr, "encryption", uci_crypt);
    UCI_ADD_OPTION(ctx, ptr, "key", (const char *)bss->key.key);
    UCI_ADD_OPTION(ctx, ptr, "wds", "1");
    UCI_ADD_OPTION(ctx, ptr, "multi_ap", "1");
    UCI_ADD_OPTION(ctx, ptr, "multi_ap_profile", (const char *)str);

out:
    ret = uci_save(ctx, wireless);
    ret = uci_commit(ctx, &wireless, true);

    ret = uci_unload(ctx, wireless);

    return ret;
}
static int _uciAddVlan(uint16_t vlan, uint8_t primary, char **interface_names)
{
    struct uci_context *ctx;
    struct uci_package *network;
    struct uci_ptr ptr = {0};
    char str[16], sec_str[16];
    char **intf_name = interface_names;
    int ret = -1;

    ctx = _getUCICtx();
    ret = uci_load(ctx, "network", &network);
    ptr.p = network;
    snprintf(sec_str, sizeof(sec_str), "%d", vlan);
    UCI_ADD_SECTION(ctx, ptr, sec_str, "bridge-vlan");
    UCI_ADD_OPTION(ctx, ptr, "device", "br-lan");
    snprintf(str, sizeof(str), "%d", vlan);
    UCI_ADD_OPTION(ctx, ptr, "vlan", str);
    if (primary) {
        UCI_ADD_OPTION(ctx, ptr, "primary", "1");
    }

    UCI_START_OPTION_LIST(ptr, "ports");
    while(*(intf_name)) {
        if (primary)
            snprintf(str, sizeof(str), "%s:u", *intf_name);
        else
            snprintf(str, sizeof(str), "%s:t", *intf_name);
        UCI_ADD_OPTION_LIST(ctx, ptr, str);
        intf_name++;
    }
    ret = uci_save(ctx, network);
    ret = uci_commit(ctx, &network, true);
    ret = uci_unload(ctx, network);

    return ret;
}

static int _uciVlanClear()
{
    int ret = -1;
    struct uci_context *ctx;
    struct uci_package *network;
    struct uci_section *s;
    struct uci_element *e, *e_tmp;

    ctx = _getUCICtx();
    uci_load(ctx, "network", &network);

    uci_foreach_element_safe(&network->sections, e_tmp, e) {
        s = uci_to_section(e);
        if (!s)
            continue;
        if (s->type && !strcmp(s->type, "bridge-vlan")) {
            struct uci_ptr ptr = { .p = network, .s = s };
            uci_delete(ctx, &ptr);
        }
    }
    uci_save(ctx, network);
    ret = uci_commit(ctx, &network, false);
    uci_unload(ctx, network);

    return ret;
}

static int _uciTearDown(struct wiphy_private *priv)
{
    int ret = -1;
    struct uci_context *ctx;
    struct uci_package *wireless;
    struct uci_section *s;
    struct uci_element *e, *e_tmp;

    ctx = _getUCICtx();
    uci_load(ctx, "wireless", &wireless);

    uci_foreach_element_safe(&wireless->sections, e_tmp, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "wifi-iface")) {
            const char *t_s;
            if ((t_s = uci_lookup_option_string(ctx, s, "device"))) {
                if (!strcmp(t_s, priv->uci_name)) {
                    const char *m_s;
                    if ((m_s = uci_lookup_option_string(ctx, s, "mode"))) {
                        if (!strcmp(m_s, "ap")) {
                            struct uci_ptr ptr = { .p = wireless, .s = s };
                            uci_delete(ctx, &ptr);
                        }
                    }
                }
            }
        }
    }
    uci_save(ctx, wireless);
    uci_commit(ctx, &wireless, 1);
    uci_unload(ctx, wireless);
    priv->ap_num = 0;

    ret = 0;
    return ret;
}

static int _uciCommit()
{
    system("ubus call network reload");
    return 0;
}

static void _uciResetVapVlan(void)
{
    struct uci_ptr ptr_del = {0};
    struct uci_context *ctx;
    struct uci_package *package;
    struct uci_section *section;
    struct uci_option *option;
    struct uci_element *e;

    ctx = _getUCICtx();
    uci_load(ctx, "wireless", &package);

    uci_foreach_element(&package->sections, e) {
        section = uci_to_section(e);
        if (!strcmp(section->type, "wifi-iface")) {
            option = uci_lookup_option(ctx, section, "brvlan");
            if (!option)
                continue;

            ptr_del.p = package;
            ptr_del.s = section;
            ptr_del.o = option;
            uci_delete(ctx, &ptr_del);
        }
    }
    uci_save(ctx, package);
    uci_commit(ctx, &package, true);
    uci_unload(ctx, package);
}

static int _uciStaIfApplyVID(uint16_t vid, bool is_defvlan)
{
    struct uci_context *ctx;
    struct uci_package *wireless;
    struct uci_section *s;
    struct uci_element *e;
    struct uci_ptr ptr = {0};
    int ret = -1;
    char str[16] = {0};
    const char *mode = NULL;
    const char *value = NULL;
    char *map_enable = NULL, *map_profile= NULL;
    int enable_value = 0, profile_value = 0;
    bool staif_existed = false;

    ctx = _getUCICtx();
    ret = uci_load(ctx, "wireless", &wireless);
    ptr.p = wireless;

    uci_foreach_element(&wireless->sections, e) {
        s = uci_to_section(e);
        if (!strcmp(s->type, "wifi-iface")) {
            mode = uci_lookup_option_string(ctx, s, "mode");
            if (mode && !strcmp(mode, "sta")) {
                if (NULL != (value = uci_lookup_option_string(ctx, s, "multi_ap")))
                    map_enable = strdup(value);
                else
                    continue;

                if (NULL != (value = uci_lookup_option_string(ctx, s, "multi_ap_profile")))
                    map_profile = strdup(value);
                else
                    continue;

                if (!map_enable || !map_profile) {
                    DEBUG_WARNING("NO space for string dup\n");
                    uci_unload(ctx, wireless);
                    return -1;
                }
                enable_value = atoi(map_enable);
                profile_value = atoi(map_profile);
                free(map_enable);
                free(map_profile);

                if (!enable_value || (profile_value < 2)) {
                    continue;
                }
                staif_existed = true;
                break;
            }
        }
    }
    if (false == staif_existed) {
        DEBUG_WARNING("can NOT found backhaul station interface\n");
        uci_unload(ctx, wireless);
        return -1;
    }

    ptr.s = s;
    if(is_defvlan)
        snprintf(str, sizeof(str), "%d:t*", vid);
    else
        snprintf(str, sizeof(str), "%d:t", vid);
    UCI_START_OPTION_LIST(ptr, "brvlan");
    UCI_ADD_OPTION_LIST(ctx, ptr, str);

    ret = uci_save(ctx, wireless);
    ret = uci_commit(ctx, &wireless, true);
    ret = uci_unload(ctx, wireless);

    return ret;
}
#endif
static int _hostapd_ubus_cli(const char *name, const char *cmd, const char *content)
{
    FILE *fp;
    char buf[256] = {};
    char ret_buf[256] = {};

    snprintf(buf, sizeof(buf) - 1, "ubus call %s %s %s", name, cmd, content);
    DEBUG_INFO("hostapd ubus cli: %s\n", buf);

    if (!(fp = popen(buf, "r")))
        return -1;

    fread(ret_buf, 1, sizeof(ret_buf) - 1, fp);
    pclose(fp);

    if (strlen(ret_buf) > 0) {
        DEBUG_ERROR("hostapd ubus cli return: %s\n", ret_buf);
        return -1;
    }

    return 0;
}


static int _nl80211SetTxpower(struct cls_driver_ctx *ctx, struct wiphy_private *priv,
                                uint8_t txpower)
{
    struct nl_msg *msg;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0,
                    0, NL80211_CMD_SET_WIPHY, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, priv->idx);
    NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_TX_POWER_SETTING, NL80211_TX_POWER_FIXED);
    /* dbm to mbm */
    NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_TX_POWER_LEVEL, DBM_TO_MBM(((uint32_t)txpower)));

    if (_issueCMD(ctx->cmd_ctrl, msg, -1, -1, -1, -1, NL80211_CMD_SET_WIPHY, 0,
        NULL, NULL) < 0) {
        DEBUG_ERROR("set txpower: %u failed\n", txpower);
        return -1;
    }

    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}


static int _hostapdSwitchChannel(struct wiphy_private *priv,
        enum nl80211_chan_width chanWidth, uint32_t control_freq, uint32_t center_freq1)
{
    char ubus_name[32] = {};
    char content[200] = {};
    char band_width = 0;
    uint8_t band_idx = freq2BandIdx(control_freq);

    if (band_idx >= band_max_idx) {
        DEBUG_ERROR("freq2BandIdx return error index: %u, control_freq: %u\n", band_idx, control_freq);
        return -1;
    }

    snprintf(ubus_name, sizeof(ubus_name) - 1, "hostapd.%s", priv->primary_name);

    switch(chanWidth) {
    case NL80211_CHAN_WIDTH_20_NOHT:
    case NL80211_CHAN_WIDTH_20:
        band_width = 20;
        break;
    case NL80211_CHAN_WIDTH_40:
        band_width = 40;
        break;
    case NL80211_CHAN_WIDTH_80:
        band_width = 80;
        break;
    case NL80211_CHAN_WIDTH_80P80:
    case NL80211_CHAN_WIDTH_160:
        band_width = 160;
        break;
    default:
        DEBUG_ERROR("unsupport chanWidth: %u\n", chanWidth);
        return -1;
    }

    snprintf(content, sizeof(content) - 1, "'{"
            "\"freq\": %u, "
            "\"bcn_count\": %d, "
            "\"center_freq1\": %u, "
            "\"bandwidth\": %u, "
            "\"ht\": %d, "
            "\"vht\": %d, "
            "\"he\": %d, "
            "\"block_tx\": %d"
            "}'",
            control_freq, 1, center_freq1, band_width,
            priv->band_capa[band_idx].ht_support,
            priv->band_capa[band_idx].vht_support,
            priv->band_capa[band_idx].he_support,
            0);

    return _hostapd_ubus_cli(ubus_name, "switch_chan", content);
}

static int _SetChannel(struct cls_driver_ctx *ctx, struct wiphy_private *priv,
                        uint8_t opclass, uint8_t channel)
{
    enum nl80211_chan_width chanWidth;
    enum nl80211_channel_type type;
    uint32_t control_freq, center_freq1;

    DEBUG_INFO("set opclass: %u, channel: %u\n", opclass, channel);

    if (!isOpclassChannelValid(opclass, channel)) {
        DEBUG_ERROR("channel(%u) not in opclass(%u) allowed channel list\n", channel, opclass);
        return -1;
    }

    chanWidth = opclass2nlBandwidth(opclass, &type);
    control_freq = channel2Freq(channel, opclass);

    if (!control_freq) {
        DEBUG_ERROR("invalid control_freq: %u, channel(%u) opclass(%u)\n",
            control_freq, channel, opclass);
        return -1;
    }

    center_freq1 = getCf1(chanWidth, control_freq, type);

    if (_hostapdSwitchChannel(priv, chanWidth, control_freq, center_freq1) != 0)
        return -1;

    DEBUG_INFO("set channel: %u successfully\n", channel);

    return 0;
}

static int _nl80211RegisterFrame(struct cls_driver_ctx *ctx, int ifIdx,
                uint16_t type, const char *match, size_t match_len)
{
    struct nl_msg *msg;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->event_ctrl->id, 0,
                    0, NL80211_CMD_REGISTER_FRAME, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifIdx);
    NLA_PUT_U16(msg, NL80211_ATTR_FRAME_TYPE, type);
    NLA_PUT(msg, NL80211_ATTR_FRAME_MATCH, match_len, match);

    if (_issueCMDNoWait(ctx->event_ctrl, msg, -1, -1, -1, -1, NL80211_CMD_REGISTER_FRAME, 0) < 0) {
        DEBUG_ERROR("_nl80211RegisterFrame on wifi interface(%d) failed\n", ifIdx);
        return -1;
    }

    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static struct nla_policy policy[NL80211_STA_INFO_MAX + 1] = {
    [NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
    [NL80211_STA_INFO_SIGNAL_AVG] = { .type = NLA_U8 },
    [NL80211_STA_INFO_CONNECTED_TIME] = { .type = NLA_U32 },
    [NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
    [NL80211_STA_INFO_RX_BYTES64] = { .type = NLA_U64 },
    [NL80211_STA_INFO_TX_BYTES64] = { .type = NLA_U64 },
    [NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
    [NL80211_STA_INFO_RX_DROP_MISC] = { .type = NLA_U32 },
    [NL80211_STA_INFO_AIRTIME_WEIGHT] = { .type = NLA_U16 },
    [NL80211_STA_INFO_RX_MPDUS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_FCS_ERROR_COUNT] = { .type = NLA_U32 },
    [NL80211_STA_INFO_AIRTIME_LINK_METRIC] = { .type = NLA_U32 },
};

static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
    [NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
    [NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
    [NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
    [NL80211_RATE_INFO_VHT_MCS] = { .type = NLA_U8 },
    [NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
    [NL80211_RATE_INFO_VHT_NSS] = { .type = NLA_U8 },
};

static int _getStationHandler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

    struct dlist_head * sta_list = (struct dlist_head *)arg;
    struct station_stats *stats;

    struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
    struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];

    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb_msg[NL80211_ATTR_MAC])
        return NL_SKIP;

    if (!tb_msg[NL80211_ATTR_STA_INFO] ||
        nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
                 tb_msg[NL80211_ATTR_STA_INFO], policy))
        return NL_SKIP;

    stats = calloc(1, sizeof(struct station_stats));
    MACCPY(stats->mac, nla_data(tb_msg[NL80211_ATTR_MAC]));

    dlist_add_tail(sta_list, &stats->l);

    if (sinfo[NL80211_STA_INFO_SIGNAL]) {
        stats->signal = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
        stats->rcpi_ul = rssi2Rcpi(stats->signal);
        stats->flags |= FLAG_SIGNAL;
        stats->flags |= FLAG_RCPI_UL;
    }

    if (sinfo[NL80211_STA_INFO_SIGNAL_AVG]) {
        stats->signal_avg = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]);
        stats->flags |= FLAG_SIGNAL_AVG;
    }

    if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
        if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
                     sinfo[NL80211_STA_INFO_TX_BITRATE],
                     rate_policy)) {
            stats->rate_ul = 0;
        } else {
            if (rinfo[NL80211_RATE_INFO_BITRATE]) {
                stats->rate_ul = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]) * 100;
                stats->flags |= FLAG_RATE_UL;
            }
        }
    }

    if (sinfo[NL80211_STA_INFO_RX_BITRATE]) {
        if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
                     sinfo[NL80211_STA_INFO_RX_BITRATE],
                     rate_policy)) {
            stats->rate_dl = 0;
        } else {
            if (rinfo[NL80211_RATE_INFO_BITRATE]) {
                stats->rate_dl = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]) * 100;
                stats->flags |= FLAG_RATE_DL;
            }
        }
    }

    if (sinfo[NL80211_STA_INFO_CONNECTED_TIME]) {
        stats->connected_time = nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]);
        stats->flags |= FLAG_CONNECTED_TIME;
    }

    if (sinfo[NL80211_STA_INFO_INACTIVE_TIME]) {
        stats->inactive_time = nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]);
        stats->flags |= FLAG_INACTIVE_TIME;
    }

    if (sinfo[NL80211_STA_INFO_RX_BYTES64]) {
        stats->rx_bytes = nla_get_u64(sinfo[NL80211_STA_INFO_RX_BYTES]);
        stats->flags |= FLAG_RX_BYTES;
    }

    if (sinfo[NL80211_STA_INFO_TX_BYTES64]) {
        stats->tx_bytes = nla_get_u64(sinfo[NL80211_STA_INFO_TX_BYTES]);
        stats->flags |= FLAG_TX_BYTES;
    }

    if (sinfo[NL80211_STA_INFO_RX_PACKETS]) {
        stats->rx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);
        stats->flags |= FLAG_RX_PACKETS;
    }

    if (sinfo[NL80211_STA_INFO_TX_PACKETS]) {
        stats->tx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);
        stats->flags |= FLAG_TX_PACKETS;
    }

    if (sinfo[NL80211_STA_INFO_TX_RETRIES]) {
        stats->tx_retries = nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]);
        stats->flags |= FLAG_TX_RETRIES;
    }

    if (sinfo[NL80211_STA_INFO_TX_FAILED]) {
        stats->tx_failed = nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]);
        stats->flags |= FLAG_TX_FAILED;
    }

    if (sinfo[NL80211_STA_INFO_RX_DROP_MISC]) {
        stats->rx_dropped_misc = nla_get_u32(sinfo[NL80211_STA_INFO_RX_DROP_MISC]);
        stats->flags |= FLAG_RX_DROPPED_MISC;
    }

    if (sinfo[NL80211_STA_INFO_AIRTIME_WEIGHT]) {
        stats->airtime_weight = nla_get_u16(sinfo[NL80211_STA_INFO_AIRTIME_WEIGHT]);
        stats->flags |= FLAG_AIRTIME_WEIGHT;
    }

    if (sinfo[NL80211_STA_INFO_RX_MPDUS]) {
        stats->rx_mpdu_count = nla_get_u32(sinfo[NL80211_STA_INFO_RX_MPDUS]);
        stats->flags |= FLAG_RX_MPDU_COUNT;
    }

    if (sinfo[NL80211_STA_INFO_FCS_ERROR_COUNT]) {
        stats->fcs_err_count = nla_get_u32(sinfo[NL80211_STA_INFO_FCS_ERROR_COUNT]);
        stats->flags |= FLAG_FCS_ERR_COUNT;
    }

    if (sinfo[NL80211_STA_INFO_AIRTIME_LINK_METRIC]) {
        stats->airtime_link_metric = nla_get_u32(sinfo[NL80211_STA_INFO_AIRTIME_LINK_METRIC]);
        stats->flags |= FLAG_AIRTIME_LINK_METRIC;
    }

    if (sinfo[NL80211_STA_INFO_TID_STATS]) {
        // TODO
    }

    return NL_SKIP;
}

static int _getChanScanResultsHandler(struct nl_msg *msg, void *arg)
{
    uint16_t caps;
    struct wifi_interface_private *wif = arg;
    struct scanlist_entry *e = NULL;
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *bss[NL80211_BSS_MAX + 1];
    uint8_t *pos, *ie, *beacon_ie;
    uint32_t ie_len, beacon_ie_len;

    static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
        [NL80211_BSS_TSF]                  = { .type = NLA_U64 },
        [NL80211_BSS_FREQUENCY]            = { .type = NLA_U32 },
        [NL80211_BSS_BSSID]                = { 0 },
        [NL80211_BSS_BEACON_INTERVAL]      = { .type = NLA_U16 },
        [NL80211_BSS_CAPABILITY]           = { .type = NLA_U16 },
        [NL80211_BSS_INFORMATION_ELEMENTS] = { 0 },
        [NL80211_BSS_SIGNAL_MBM]           = { .type = NLA_U32 },
        [NL80211_BSS_SIGNAL_UNSPEC]        = { .type = NLA_U8  },
        [NL80211_BSS_STATUS]               = { .type = NLA_U32 },
        [NL80211_BSS_SEEN_MS_AGO]          = { .type = NLA_U32 },
        [NL80211_BSS_LAST_SEEN_BOOTTIME]   = { .type = NLA_U64 },
        [NL80211_BSS_BEACON_IES]           = { 0 },
    };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[NL80211_ATTR_BSS] ||
        nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS],
                         bss_policy) ||
        !bss[NL80211_BSS_BSSID]){
        DEBUG_ERROR("receive invalid scan result.\n");
        return NL_SKIP;
    }

    if (bss[NL80211_BSS_CAPABILITY])
        caps = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
    else
        caps = 0;

    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
        ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
        ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
    } else {
        ie = NULL;
        ie_len = 0;
    }
    if (bss[NL80211_BSS_BEACON_IES]) {
        beacon_ie = nla_data(bss[NL80211_BSS_BEACON_IES]);
        beacon_ie_len = nla_len(bss[NL80211_BSS_BEACON_IES]);
    } else {
        beacon_ie = NULL;
        beacon_ie_len = 0;
    }

    e = calloc(1, sizeof(struct scanlist_entry) + ie_len + beacon_ie_len);
    dlist_add_tail(&wif->radio->last_scan_list_head, &e->l);

    MACCPY(e->bssid, nla_data(bss[NL80211_BSS_BSSID]));
    e->caps = caps;

    if (bss[NL80211_BSS_FREQUENCY])
        e->channel = freq2Channel(nla_get_u32(bss[NL80211_BSS_FREQUENCY]));

    if (bss[NL80211_BSS_SIGNAL_MBM])
        e->signal = (uint8_t)MBM_TO_DBM(nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]));

    if (bss[NL80211_BSS_SEEN_MS_AGO])
        e->last_seen_ms = PLATFORM_GET_TIMESTAMP(nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]));

//  if (bss[NL80211_BSS_LAST_SEEN_BOOTTIME])
//      e->last_seen_ms = nla_get_u32(bss[NL80211_BSS_LAST_SEEN_BOOTTIME])/1000000;

    e->ie_len = ie_len;
    e->beacon_ie_len = beacon_ie_len;
    pos = (uint8_t *)(e + 1);

    if (ie) {
        memcpy(pos, ie, ie_len);
        pos += ie_len;
    }

    if (beacon_ie)
        memcpy(pos, beacon_ie, beacon_ie_len);

    return NL_SKIP;
}

/* May be use more than one msg to send the Results. */
static void _sendScanResults(struct wifi_interface_private *wif, uint32_t start_num,
            uint8_t *finished_flag)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    uint32_t num = 0;
    struct scanlist_entry *e;

    *finished_flag = 1;
    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_chan_scan_results);
    p = msgaPutU32(p, attr_if_idx, wif->index);
    p = msgaPutU32(p, attr_opclass, wif->radio->last_scan_opclass);

    dlist_for_each(e, wif->radio->last_scan_list_head, l) {
        if ((start_num != 0) && (num < start_num)) {
            num++;
            continue;
        }

        if (p - buf >= msgGetMaxBufSize()/2 - 48 - e->ie_len - e->beacon_ie_len) {
            *finished_flag = 0;
            break;
        }

        num++;
        p = msgaPutBin(p, attr_bssid, e->bssid, MACLEN);
        p = msgaPutU8(p, attr_channel, e->channel);
        p = msgaPutU8(p, attr_signal, e->signal);
        p = msgaPutU32(p, attr_ts, e->last_seen_ms);
        if (e->ie_len > 0)
            p = msgaPutBin(p, attr_ies, (uint8_t *)(e + 1), e->ie_len);
        if (e->beacon_ie_len > 0)
            p = msgaPutBin(p, attr_beacon_ies, (uint8_t *)(e + 1) + e->ie_len, e->beacon_ie_len);
    }
    p = msgaPutU32(p, attr_start_num, start_num + num);
    p = msgaPutU8(p, attr_finished_flag, *finished_flag);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return;
}

static int _nl80211GetStation(struct cls_driver_ctx *ctx, uint32_t ifidx, uint8_t *mac)
{
    struct nl_msg *msg;
    struct station_stats *stats;
    DEFINE_DLIST_HEAD(sta_list);
    int flag = 0;
    int ret;

    if (!mac)
        flag = NLM_F_DUMP;
    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0,
                    flag, NL80211_CMD_GET_STATION, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);

    if (mac)
        NLA_PUT(msg, NL80211_ATTR_MAC, MACLEN, mac);

    ret = _issueCMD(ctx->cmd_ctrl, msg, -1, ifidx, -1, -1, NL80211_CMD_GET_STATION, flag,
                    _getStationHandler, &sta_list);

    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_station);
    p = msgaPutU32(p, attr_if_idx, ifidx);

    /* if get single sta stats failed. but maybe sta_list is empty. */
    if (dlist_empty(&sta_list) && mac && (ret < 0)) {
        p = msgaPutBin(p, attr_mac, mac, MACLEN);
        p = msgaPutU32(p, attr_ts, PLATFORM_GET_TIMESTAMP(0));
        p = msgaPutU8(p, attr_ret_code, 1);
    }

    dlist_for_each(stats, sta_list, l) {
        p = msgaPutBin(p, attr_mac, stats->mac, MACLEN);
        p = msgaPutU32(p, attr_ts, PLATFORM_GET_TIMESTAMP(0));
        if (ret < 0)
            p = msgaPutU8(p, attr_ret_code, 1);
        else {
            p = msgaPutU8(p, attr_ret_code, 0);
            if (stats->flags & FLAG_RATE_DL)
                p = msgaPutU32(p, attr_station_rate_dl, stats->rate_dl);
            if (stats->flags & FLAG_RATE_UL)
                p = msgaPutU32(p, attr_station_rate_ul, stats->rate_ul);
            if (stats->flags & FLAG_RCPI_UL)
                p = msgaPutU8(p, attr_station_rcpi_ul, stats->rcpi_ul);
            if (stats->flags & FLAG_RX_BYTES)
                p = msgaPutU32(p, attr_station_rx_bytes, stats->rx_bytes);
            if (stats->flags & FLAG_TX_BYTES)
                p = msgaPutU32(p, attr_station_tx_bytes, stats->tx_bytes);
            if (stats->flags & FLAG_RX_PACKETS)
                p = msgaPutU32(p, attr_station_rx_packets, stats->rx_packets);
            if (stats->flags & FLAG_TX_PACKETS)
                p = msgaPutU32(p, attr_station_tx_packets, stats->tx_packets);
            if (stats->flags & FLAG_RX_DROPPED_MISC) // TODO: to be confirmed if this rx errors
                p = msgaPutU32(p, attr_station_rx_errors, stats->rx_dropped_misc);
            if (stats->flags & FLAG_TX_FAILED) // TODO: to be confirmed if this tx errors
                p = msgaPutU32(p, attr_station_tx_errors, stats->tx_failed);
            if (stats->flags & FLAG_TX_RETRIES) // TODO: to be confirmed if this tx tries
                p = msgaPutU32(p, attr_station_tx_tries, stats->tx_retries);
            if (stats->flags & FLAG_LAST_DATARATE_DL) // TODO
                p = msgaPutU32(p, attr_station_last_datarate_dl, stats->last_datarate_dl);
            if (stats->flags & FLAG_LAST_DATARATE_UL) // TODO
                p = msgaPutU32(p, attr_station_last_datarate_ul, stats->last_datarate_ul);
            if (stats->flags & FLAG_UTILIZATION_RX) // TODO
                p = msgaPutU32(p, attr_station_utilization_rx, stats->utilization_rx);
            if (stats->flags & FLAG_UTILIZATION_TX) // TODO
                p = msgaPutU32(p, attr_station_utilization_tx, stats->utilization_tx);
        }
    }
    msgSend(buf, p-buf);
    msgPutBuf(buf);

    dlist_free_items(&sta_list, struct station_stats, l);
    return ret;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static char _hostapd_buf[256];

static char *_hostapdCliCall(uint8_t is_ap, const char *ifname, const char *cmd)
{
    FILE *pipe;
    const char *cli = is_ap ? "hostapd_cli" : "wpa_cli";
    int n;
    char *ret = NULL;

    if (ifname)
        snprintf(_hostapd_buf, sizeof(_hostapd_buf), "%s -i %s %s", cli, ifname, cmd);
    else
        snprintf(_hostapd_buf, sizeof(_hostapd_buf), "%s %s", cli, cmd);
    DEBUG_INFO("hostapdCall %s\n", _hostapd_buf);

    if ((!(pipe = popen(_hostapd_buf, "r"))) ||
        (feof(pipe) || (n=fread(_hostapd_buf, 1, sizeof(_hostapd_buf) - 1, pipe)) <= 0)) {
        DEBUG_ERROR("cmd(%s) failed\n", cmd);
    } else {
        //remove trailer \n
        while ((n>0) && (_hostapd_buf[n-1]=='\n'))
            n--;
        _hostapd_buf[n] = '\0';

        if ((n<=0) || (!strncmp(_hostapd_buf,"FAIL",4)) ||
            (!strncmp(_hostapd_buf,"Unknown command",15))){
            DEBUG_ERROR("cmd(%s) return failed\n", cmd);
        } else {
            ret = _hostapd_buf;
            DEBUG_DETAIL("cmd return %s\n", _hostapd_buf);
        }
    }

    if (pipe)
        pclose(pipe);

    return ret;
}

static void processTearDown(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct wiphy_private *priv;

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_radio_mac))
        return;

    priv = _findWiphy(ctx, msgaGetBin(&attrs[attr_radio_mac]));

    if (!priv)
        return;
#ifdef USE_UCI
    _uciTearDown(priv);
#endif
}

static void processAddBSS(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
    struct bss_info bss_info = {0}, backhaul_bss_info = {0};
    struct msg_attr attrs[attr_drv_max] = {0};
    struct wiphy_private *priv;

    msgaParse(attrs, attr_drv_max, msg, len);

    if ((!hasMsga(attrs, attr_radio_mac)) ||
        (!hasMsga(attrs, attr_ssid)) ||
        (!hasMsga(attrs, attr_bss_role)))
        return;

    priv = _findWiphy(ctx, msgaGetBin(&attrs[attr_radio_mac]));
    if (!priv)
        return;

    bss_info.ssid.len =  msgaGetLen(&attrs[attr_ssid]);
    memcpy(bss_info.ssid.ssid, msgaGetBin(&attrs[attr_ssid]),
            bss_info.ssid.len);

    if (hasMsga(attrs, attr_key)) {
        bss_info.key.len =  msgaGetLen(&attrs[attr_key]);
        memcpy(bss_info.key.key, msgaGetBin(&attrs[attr_key]),
                bss_info.key.len);
    }

    if (hasMsga(attrs, attr_auth))
        bss_info.auth = msgaGetU8(&attrs[attr_auth]);

    if (hasMsga(attrs, attr_encrypt))
        bss_info.encrypt = msgaGetU8(&attrs[attr_encrypt]);

    if (msgaGetU8(&attrs[attr_bss_role])==role_sta) {
#ifdef USE_UCI
        _uciAddSTA(priv, &bss_info);
#endif
        return;
    }

    if (hasMsga(attrs, attr_backhaul)) {
        bss_info.backhaul = 1;
        if (hasMsga(attrs, attr_vlan_0)) {
            int i=0;
            do {
                bss_info.vlan_map[i] = msgaGetU16(&attrs[attr_vlan_0+i]);
            } while(hasMsga(attrs, attr_vlan_0+(++i)));
        }
    } else {
        if (hasMsga(attrs, attr_vlan_0)) {
            bss_info.vlan_map[0] = msgaGetU16(&attrs[attr_vlan_0]);
        }
    }

    if (hasMsga(attrs, attr_fronthaul)) {
        bss_info.fronthaul = 1;
        if (hasMsga(attrs, attr_backhaul_ssid)) {
            backhaul_bss_info.ssid.len = msgaGetLen(&attrs[attr_backhaul_ssid]);
            memcpy(backhaul_bss_info.ssid.ssid, msgaGetBin(&attrs[attr_backhaul_ssid]), backhaul_bss_info.ssid.len);
        }
        if (hasMsga(attrs, attr_backhaul_auth))
            backhaul_bss_info.auth = msgaGetU8(&attrs[attr_backhaul_auth]);
        if (hasMsga(attrs, attr_backhaul_encrypt))
            backhaul_bss_info.encrypt = msgaGetU8(&attrs[attr_backhaul_encrypt]);
        if (hasMsga(attrs, attr_backhaul_key)) {
            backhaul_bss_info.key.len = msgaGetLen(&attrs[attr_backhaul_key]);
            memcpy(backhaul_bss_info.key.key, msgaGetBin(&attrs[attr_backhaul_key]), backhaul_bss_info.key.len);
        }
    }
#ifdef USE_UCI
    _uciAddAP(priv, &bss_info, &backhaul_bss_info);
#endif
    DEBUG_INFO("processAddBSS radio("MACFMT") bss_info: {ssid: %s, auth: %d, encrypt: %d, fronthaul: %d, "
               "backhaul: %d, key: %s, backhaul_bss: {ssid: %s, auth: %d, encrypt: %d, key: %s}} \n",
               MACARG(priv->mac), bss_info.ssid.ssid, bss_info.auth, bss_info.encrypt, bss_info.fronthaul,
               bss_info.backhaul, bss_info.key.key, backhaul_bss_info.ssid.ssid, backhaul_bss_info.auth,
               backhaul_bss_info.encrypt, backhaul_bss_info.key.key);
}

static void processEthVLAN(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
#ifdef USE_UCI
    int type;
    struct msg_attr attr[1];
    uint16_t vlan = 0;
    int type_mem = 0;
    char *intf_names[VLAN_MAX+1] = {0};
    int intf_num = 0;
    uint16_t msg_len = len;

    _uciVlanClear();
    _uciResetVapVlan();
    while ((type = msgaParseOne(attr, &msg, &msg_len)) != attr_none) {
        switch (type) {
            case attr_if_name:
                intf_names[intf_num++] = (char *)msgaGetBin(attr);
                break;
            default:
                if ((type>=attr_vlan_0) && (type<=attr_vlan_n)) {
                    if ((vlan) && (*intf_names)){
                        _uciAddVlan(vlan, (type_mem==attr_vlan_0), intf_names);
                    }
                    vlan = msgaGetU16(attr);
                    type_mem = type;
                    if (attr_vlan_0 == type)
                        _uciStaIfApplyVID(vlan, true);
                    else
                        _uciStaIfApplyVID(vlan, false);
                }
                break;
        }
    }

    if ((vlan) && (*intf_names)){
        _uciAddVlan(vlan, (type_mem==attr_vlan_0), intf_names);
    }
#endif
}

static void processSetChannelTXPower(struct cls_driver_ctx *ctx,
                uint8_t *msg, uint32_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct wiphy_private *priv;

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_radio_mac))
        return;

    if (!(priv = _findWiphy(ctx, msgaGetBin(&attrs[attr_radio_mac]))))
        return;

    if (hasMsga(attrs, attr_power_lvl)) {
       _nl80211SetTxpower(ctx, priv, msgaGetU8(&attrs[attr_power_lvl]));
    }

    if ((hasMsga(attrs, attr_channel)) &&
        (hasMsga(attrs, attr_opclass))) {
       _SetChannel(ctx, priv, msgaGetU8(&attrs[attr_opclass]), msgaGetU8(&attrs[attr_channel]));
    }

}

static void processCommit(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
#ifdef USE_UCI
    _uciCommit();
#endif
}

static void processSendMgmtFrame(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
    struct nl_msg *msg_buf;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_if_idx) ||
        !hasMsga(attrs, attr_frame))
        return;

    msg_buf = nlmsg_alloc();
    genlmsg_put(msg_buf, 0, 0, ctx->cmd_ctrl->id, 0,
                    0, NL80211_CMD_FRAME, 0);

    NLA_PUT_U32(msg_buf, NL80211_ATTR_IFINDEX, msgaGetU32(&attrs[attr_if_idx]));
    NLA_PUT(msg_buf, NL80211_ATTR_FRAME, msgaGetLen(&attrs[attr_frame]), msgaGetBin(&attrs[attr_frame]));

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg_buf, -1, -1, -1, -1, NL80211_CMD_FRAME, 0) < 0) {
        DEBUG_ERROR("send frame failed\n");
        return;
    }

    DEBUG_INFO("send frame successfully\n");

    return;

nla_put_failure:
    nlmsg_free(msg_buf);
    return;
}

static void processRegisterMgmtFrame(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
    int ifIdx;
    uint16_t frame_type;
    char *match = NULL;
    uint32_t match_len = 0;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_if_idx) ||
        !hasMsga(attrs, attr_frame_type))
        return;

    ifIdx = msgaGetU32(&attrs[attr_if_idx]);
    frame_type = msgaGetU16(&attrs[attr_frame_type]);
    if (hasMsga(attrs, attr_match)) {
        match = (char *)msgaGetBin(&attrs[attr_match]);
        match_len = msgaGetLen(&attrs[attr_match]);
    }

    DEBUG_INFO("ifIdx: %d, frame_type: %u, match: %s\n", ifIdx, frame_type, match);

    if (_nl80211RegisterFrame(ctx, ifIdx, IEEE80211_FC0(IEEE80211_FC0_TYPE_MGT, frame_type), match, match_len) < 0) {
        DEBUG_ERROR("register mgmt frame failed\n");
        return;
    }

    DEBUG_INFO("register mgmt frame successfully\n");

    //_recordRegisteredFrame(ctx, ifIdx, match_len, match, frame_type);
    return;
}

static void processGetStationStats(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len)
{
    uint8_t *mac = NULL;
    uint32_t ifidx = -1;
    int ret = 0;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_if_idx))
        return;

    ifidx = msgaGetU32(&attrs[attr_if_idx]);
    if (!hasMsga(attrs, attr_mac))
        mac = msgaGetBin(&attrs[attr_mac]);

    if ((ret = _nl80211GetStation(ctx, ifidx, mac)) < 0) {
        DEBUG_ERROR("get bss(ifidx: %u) station statistics failed. return: %d\n", ifidx, ret);
        return;
    }

    return;
}

static void processTriggerScan(struct cls_driver_ctx *ctx, uint8_t *msg, uint32_t len_input)
{
    struct wifi_interface_private *wif_private = NULL;
    struct nl_msg *msg_buf;
    //struct nlattr *rates;
    struct nlattr *frequencies;
    uint16_t len = (uint16_t)len_input;
    uint32_t ifidx;
    uint8_t opclass = 0;
    uint32_t scan_chan_num = 0;
    uint8_t scan_channels[MAX_CHANNEL_PER_OPCLASS];
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);
    struct msg_attr attr[1];
    int type;
    int i;
    uint32_t scan_flags = 0;

    msg_buf = nlmsg_alloc();
    genlmsg_put(msg_buf, 0, 0, ctx->cmd_ctrl->id, 0,
                    0, NL80211_CMD_TRIGGER_SCAN, 0);

    scan_flags |= NL80211_SCAN_FLAG_OFF_CHANNEL;

#if 0
    /* no cck rate */
    rates = nla_nest_start(msg_buf, NL80211_ATTR_SCAN_SUPP_RATES);
    if (rates == NULL)
        goto nla_put_failure;

    /*
     * Remove 2.4 GHz rates 1, 2, 5.5, 11 Mbps from supported rates
     * by masking out everything else apart from the OFDM rates 6,
     * 9, 12, 18, 24, 36, 48, 54 Mbps from non-MCS rates. All 5 GHz
     * rates are left enabled.
     */
    if (nla_put(msg_buf, NL80211_BAND_2GHZ, 8,
            "\x0c\x12\x18\x24\x30\x48\x60\x6c"))
        goto nla_put_failure;
    nla_nest_end(msg_buf, rates);

    if (nla_put_flag(msg_buf, NL80211_ATTR_TX_NO_CCK_RATE))
        goto nla_put_failure;
#endif
    while ((type = msgaParseOne(attr, &msg, &len)) != attr_none) {
        switch (type) {
            case attr_if_idx:
                ifidx = msgaGetU32(attr);
                NLA_PUT_U32(msg_buf, NL80211_ATTR_IFINDEX, ifidx);
                break;
            case attr_opclass:
                opclass = msgaGetU32(attr);
                break;
            case attr_channel:
                if (scan_chan_num >= MAX_CHANNEL_PER_OPCLASS - 1)
                    break;
                scan_channels[scan_chan_num++] = msgaGetU8(attr);
                break;
            default:
                break;
        }
    }

    if (opclass == 0)
        goto nla_put_failure;

    wif_private = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif_private || !wif_private->radio)
        goto nla_put_failure;

    /* check if last scan finished or wait time overdue */
    if (wif_private->radio->channel_scan_status == CHANNEL_SCAN_STARTED &&
        (current_ts - wif_private->radio->last_scan_trigger_ts)/1000 < CHANNEL_SCAN_MAX_WAIT_TIME) {
        DEBUG_ERROR("trigger scan on radio: %s, opclass: %u, does not finished. try after 60s.\n",
            wif_private->radio->name, opclass);
        goto nla_put_failure;
    }

    wif_private->radio->last_scan_opclass = opclass;
    wif_private->radio->last_scan_trigger_ts = current_ts;
    wif_private->radio->channel_scan_status = CHANNEL_SCAN_STARTED;

    DEBUG_INFO("trigger scan on(ifidex: %u, opclass:%u, scan_chan_num: %u)\n",
        ifidx, opclass, scan_chan_num);

    /* frequencies */
    if (scan_chan_num > 0) {
        frequencies = nla_nest_start(msg_buf, NL80211_ATTR_SCAN_FREQUENCIES);
        if (frequencies == NULL)
            goto nla_put_failure;

        for (i = 0; i < scan_chan_num; i++) {
            DEBUG_INFO("channel: %u, frequency: %u\n", scan_channels[i],
                channel2Freq(scan_channels[i], opclass));
            if (nla_put_u32(msg_buf, i + 1, channel2Freq(scan_channels[i], opclass)))
                goto nla_put_failure;
        }

        nla_nest_end(msg_buf, frequencies);
    }

    nla_put_u32(msg_buf, NL80211_ATTR_SCAN_FLAGS, scan_flags); /* enable off-channel scan */

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg_buf, -1, ifidx, -1, -1, NL80211_CMD_TRIGGER_SCAN, 0) < 0) {
        DEBUG_ERROR("trigger scan on(ifidex: %u, opclass:%u, scan_chan_num: %u) failed.\n",
            ifidx, opclass, scan_chan_num);
        return;
    }

    return;

nla_put_failure:
    nlmsg_free(msg_buf);
    return;
}

static void processGetChanScanResults(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct wifi_interface_private *wif_private = NULL;
    uint32_t ifidx = -1;
    struct msg_attr attrs[attr_drv_max] = {0};
    struct nl_msg *msg_buf;
    uint32_t start_num = 0;
    uint8_t finished_flag = 0;

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_if_idx))
        return;

    ifidx = msgaGetU32(&attrs[attr_if_idx]);
    wif_private = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif_private || !wif_private->radio)
        return;

    if (hasMsga(attrs, attr_start_num))
        start_num = msgaGetU32(&attrs[attr_start_num]);

    /* first time get total data from driver */
    if (start_num == 0) {
        msg_buf = nlmsg_alloc();
        genlmsg_put(msg_buf, 0, 0, ctx->cmd_ctrl->id, 0,
                        NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);

        NLA_PUT_U32(msg_buf, NL80211_ATTR_IFINDEX, ifidx);

        if (_issueCMD(ctx->cmd_ctrl, msg_buf, -1, ifidx, -1, -1, NL80211_CMD_GET_SCAN, NLM_F_DUMP,
                    _getChanScanResultsHandler, wif_private) < 0) {
            DEBUG_ERROR("processGetChanScanResults on %u interface failed.\n", ifidx);
            return;
        }
    }

    _sendScanResults(wif_private, start_num, &finished_flag);

    if (finished_flag)
        dlist_free_items(&wif_private->radio->last_scan_list_head, struct scanlist_entry, l);

    return;

nla_put_failure:
    nlmsg_free(msg_buf);
    return;
}

static void processSTADeauth(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct wifi_interface_private *wif_private = NULL;
    struct msg_attr attrs[attr_drv_max] = {0};
    uint32_t ifidx = -1;
    uint8_t *mac = NULL;
    char cmd[64] = {0};
    uint16_t reason = WLAN_STATUS_UNSPECIFIED_FAILURE;

    msgaParse(attrs, attr_drv_max, msg, len);
    if ((!hasMsga(attrs, attr_if_idx)) || (!hasMsga(attrs, attr_mac)))
        return;
    ifidx = msgaGetU32(&attrs[attr_if_idx]);
    mac = msgaGetBin(&attrs[attr_mac]);
    if (hasMsga(attrs, attr_reason_code))
        reason = msgaGetU16(&attrs[attr_reason_code]);

    wif_private = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif_private)
        return;
    sprintf(cmd, "DEAUTHENTICATE "MACFMT" reason=%d", MACARG(mac), reason);
    _hostapdCliCall(1, wif_private->name, cmd);
}

static void processSTADeny(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len, uint8_t evt)
{
    struct wifi_interface_private *wif_private = NULL;
    struct msg_attr attrs[attr_drv_max] = {0};
    uint32_t ifidx = -1;
    uint8_t *mac = NULL;
    char cmd[64] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);
    if ((!hasMsga(attrs, attr_if_idx)) || (!hasMsga(attrs, attr_mac)))
        return;
    ifidx = msgaGetU32(&attrs[attr_if_idx]);
    mac = msgaGetBin(&attrs[attr_mac]);

    wif_private = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif_private || !wif_private->radio)
        return;
    if (evt == drv_cmd_deny_sta_add)
        sprintf(cmd, "DENY_ACL ADD_MAC "MACFMT, MACARG(mac));
    else
        sprintf(cmd, "DENY_ACL DEL_MAC "MACFMT, MACARG(mac));
    _hostapdCliCall(1, wif_private->name, cmd);
}

static void processStartWPS(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct wifi_interface_private *wif_private = NULL;
    struct msg_attr attrs[attr_drv_max] = {0};
    uint32_t ifidx = -1;
    uint8_t role;
    char *name = NULL;

    msgaParse(attrs, attr_drv_max, msg, len);
    if ((!hasMsga(attrs, attr_if_idx))
        ||(!hasMsga(attrs, attr_bss_role)))
        return;
    ifidx = msgaGetU32(&attrs[attr_if_idx]);
    role = msgaGetU8(&attrs[attr_bss_role]);
    if (hasMsga(attrs, attr_if_name)) {
        name = msgaGetStr(&attrs[attr_if_name]);
    } else if ((wif_private = _findWifiInterfaceIdx(ctx, ifidx))) {
        name = wif_private->name;
    } else
        return;

    if (role == role_ap)
        _hostapdCliCall(1, name, "WPS_PBC");
    else
        _hostapdCliCall(0, name, "WPS_PBC multi_ap=1");
}

#define QUEUE_CONF_TMP_FILE "/tmp/cls_queue.conf"
static void processQueueAction(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    FILE *tmp_file = NULL;
    char cmd[128] = {0};
    struct msg_attr attr[1];
    int type;
    int port = -1, qid = -1, weight = -1;
    char conf_file_name[64] = {0};
    uint16_t msg_len = len;

    if (isRegistrar())
        sprintf(conf_file_name, "%s", QUEUE_CONF_TMP_FILE);
    else
        sprintf(conf_file_name, "%s", VIP_QUEUE_CONF_PATH);

    tmp_file = fopen(conf_file_name, "w+");
    if (!tmp_file) {
        DEBUG_WARNING("can NOT creat tmp file\n");
        return;
    }

    system("echo reset >> /sys/kernel/debug/cls_npe/queue_weight");

    while ((type = msgaParseOne(attr, &msg, &msg_len)) != attr_none) {
       switch (type) {
           case attr_queue_port:
                port = msgaGetU8(attr);
                break;
           case attr_queue_id:
                qid = msgaGetU8(attr);
                break;
           case attr_queue_weight:
                weight = msgaGetU8(attr);
                break;
           default:
                break;
       }
        if (port >= 0 && qid >= 0 && weight >= 0) {
            if (fprintf(tmp_file, "%d %d %d\n", port, qid, weight) > 0) {
                DEBUG_INFO("set to driver: port_id=%d: queue_id=%d, weight=%d\n", port, qid, weight);
                port = -1;
                qid = -1;
                weight = -1;
            }
        }
   }

    fclose(tmp_file);
    sprintf(cmd, "cat %s >> %s", conf_file_name, DEBUGFS_QUEUE_CONF_PATH);
    system(cmd);
    system("echo apply >> /sys/kernel/debug/cls_npe/queue_weight");
}


#define DSCP_CONF_TMP_FILE "/tmp/cls_dscp.conf"
static void processDSCPAction(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct msg_attr attr[1];
    int type;
    int tid = -1, qid = -1, dscp = -1, dft_tid = -1, dft_qid = -1;
    char cmd[128] = {0};
    FILE *tmp = NULL;
    char conf_file_name[64] = {0};
    uint16_t msg_len = len;

    if (isRegistrar())
        sprintf(conf_file_name, "%s", DSCP_CONF_TMP_FILE);
    else
        sprintf(conf_file_name, "%s", VIP_DSCP_CONF_PATH);

    tmp = fopen(conf_file_name, "w+");
    if (!tmp) {
        DEBUG_WARNING("can NOT creat tmp file\n");
        return;
    }

    system("echo reset >> /sys/kernel/debug/cls_npe/dscp_map");

    while ((type = msgaParseOne(attr, &msg, &msg_len)) != attr_none) {
        switch (type) {
            case attr_dscp_value:
                 dscp = msgaGetU8(attr);
                 break;
            case attr_dscp_qid:
                 qid = msgaGetU8(attr);
                 break;
            case attr_dscp_tid:
                 tid = msgaGetU8(attr);
                 break;
            case attr_dscp_dft_tid:
                 dft_tid = msgaGetU8(attr);
                 break;
            case attr_dscp_dft_qid:
                 dft_qid = msgaGetU8(attr);
                 break;
            default:
                 break;
        }
        if (dscp >= 0 && qid >= 0 && tid >= 0) {
            if (fprintf(tmp, "%d %d %d\n", dscp, tid, qid) > 0) {
                DEBUG_INFO("set to driver: dscp=%d, tid=%d, queue_id=%d\n", dscp, tid, qid);
                dscp = -1;
                tid = -1;
                qid = -1;
            }
        }
    }

    if (dft_tid >= 0 && dft_qid >= 0) { /* default config field for dscp */
        DEBUG_INFO("set to driver: dscp default queue_id=%d, tid=%d\n", dft_qid, dft_tid);
        fprintf(tmp, "%d %d %d\n", -1, dft_tid, dft_qid);
    }
    fclose(tmp);

    sprintf(cmd, "cat %s >> %s", conf_file_name, DEBUGFS_DSCP_CONF_PATH);
    system(cmd);
    system("echo apply >> /sys/kernel/debug/cls_npe/dscp_map");
}


#define TC_CONF_TMP_FILE "/tmp/cls_tc.conf"
static void processTCAction(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    FILE *tmp_file = NULL;
    char cmd[128] = {0};
    struct msg_attr attr[1];
    int type;
    int tid = -1, qid = -1, tc = -1, dft_tid = -1, dft_qid = -1;
    char conf_file_name[64] = {0};
    uint16_t msg_len = len;

    if (isRegistrar())
     sprintf(conf_file_name, "%s", TC_CONF_TMP_FILE);
    else
     sprintf(conf_file_name, "%s", VIP_TC_CONF_PATH);

    tmp_file = fopen(conf_file_name, "w+");
    if (!tmp_file) {
     DEBUG_WARNING("can NOT creat tmp file\n");
     return;
    }

     system("echo reset >> /sys/kernel/debug/cls_npe/tc_map");

     while ((type = msgaParseOne(attr, &msg, &msg_len)) != attr_none) {
        switch (type) {
            case attr_tc_value:
                 tc = msgaGetU8(attr);
                 break;
            case attr_queue_id:
                 qid = msgaGetU8(attr);
                 break;
            case attr_tc_tid:
                 tid = msgaGetU8(attr);
                 break;
            case attr_tc_dft_tid:
                 dft_tid = msgaGetU8(attr);
                 break;
            case attr_tc_dft_qid:
                 dft_qid = msgaGetU8(attr);
                 break;
            default:
                 break;
        }
         if (tc >= 0 && qid >= 0 && tid >= 0) {
             if (fprintf(tmp_file, "%d %d %d\n", tc, tid, qid) > 0) {
                 DEBUG_INFO("set to driver: tc=%d, tid=%d, queue_id=%d\n", tc, tid, qid);
                 tc = -1;
                 tid = -1;
                 qid = -1;
             }
         }
    }

    if (dft_tid >= 0 && dft_qid >= 0) { /* default config field for tc */
        DEBUG_INFO("set to driver: TC default queue_id=%d, tid=%d\n", dft_qid, dft_tid);
        fprintf(tmp_file, "%d %d %d\n", -1, dft_tid, dft_qid);
    }

    fclose(tmp_file);
    sprintf(cmd, "cat %s >> %s", conf_file_name, DEBUGFS_TC_CONF_PATH);
    system(cmd);
    system("echo apply >> /sys/kernel/debug/cls_npe/tc_map");
}

#define CLS_VENDOR_ID 0xd04433
#define CLS_VENDOR_SUBCMD_ADD_VIP   0x3f
#define CLS_VENDOR_SUBCMD_DEL_VIP   0x40
#define CLS_VENDOR_SUBCMD_CLEAR_ALL_VIPS 0x41

static int _nl80211ClearAllVips(struct cls_driver_ctx *ctx)
{
    struct nl_msg *msg;
    uint32_t ifidx = 0;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0, 0, NL80211_CMD_VENDOR, 0);

    ifidx = if_nametoindex("wlan1"); /* choose any vap, it will NOT take effect, just a param */
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, CLS_VENDOR_ID);

    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, CLS_VENDOR_SUBCMD_CLEAR_ALL_VIPS);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_DATA, 0x00);

    DEBUG_INFO("clear all VIP stations\n");

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg, -1, -1, -1, -1, NL80211_CMD_VENDOR, 0) < 0) {
        DEBUG_ERROR("clear all VIPs failed\n");
        return -1;;
    }
    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static int _nl80211SetVipSta(struct cls_driver_ctx *ctx, uint8_t action, uint8_t *mac)
{
    struct nl_msg *msg;
    uint32_t subcmd = 0;
    uint32_t ifidx = 0;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0, 0, NL80211_CMD_VENDOR, 0);

    ifidx = if_nametoindex("wlan1"); /* choose any vap, it will NOT take effect, just a param */
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, CLS_VENDOR_ID);

    if (HAL_ADD_VIP == action)
        subcmd = CLS_VENDOR_SUBCMD_ADD_VIP;
    else
        subcmd = CLS_VENDOR_SUBCMD_DEL_VIP;

    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, subcmd);
    NLA_PUT(msg, NL80211_ATTR_VENDOR_DATA, MACLEN, mac);

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg, -1, -1, -1, -1, NL80211_CMD_VENDOR, 0) < 0) {
        DEBUG_ERROR("%s vip sta["MACFMT"] failed\n", action ? "DEL" : "ADD", MACARG(mac));
        return -1;;
    }
    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static void processVipAdd(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct msg_attr attr[1];
    int type;
    uint8_t *mac;
    int ret; /* result for cfg80211 */
    uint16_t msg_len = len;

    _nl80211ClearAllVips(ctx);

    while ((type = msgaParseOne(attr, &msg, &msg_len)) != attr_none) {
       switch (type) {
           case attr_mac:
                mac = msgaGetBin(attr);
                DEBUG_INFO("ADD VIP["MACFMT"] to driver\n", MACARG(mac));
                ret = _nl80211SetVipSta(ctx, HAL_ADD_VIP, mac);
                if (ret < 0)
                    DEBUG_WARNING("ADD VIP["MACFMT"] failed\n", MACARG(mac));
                break;
           default:
                break;
       }
    }
}

static void processSetNeighbor(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct msg_attr attr[1];
    struct wifi_interface_private *wif;
    uint8_t flag = 0, *nr=NULL, *bssid=NULL;
    char *ssid=NULL;
    uint16_t nr_len;
    char cmd[256];
    int ret, type;
    uint16_t msg_len = len;

    while ((type = msgaParseOne(attr, &msg, &msg_len)) != attr_none) {
        switch (type) {
            case attr_bssid:
                if (bssid) {
                    if (flag==FLAG_ADD) {
                        if ((ssid) && (nr)) {
                            ret = sprintf(cmd, "SET_NEIGHBOR "MACFMT" ssid=\\\"%s\\\" nr=", MACARG(bssid), ssid);
                            ret += hex2String(nr, cmd+ret, nr_len);
                            cmd[ret] = 0;
                            dlist_for_each(wif, ctx->bsses, l) {
                                _hostapdCliCall(1, wif->name, cmd);
                            }
                        }
                    } else if (flag==FLAG_DEL) {
                        ret = sprintf(cmd, "REMOVE_NEIGHBOR "MACFMT, MACARG(bssid));
                        dlist_for_each(wif, ctx->bsses, l) {
                            _hostapdCliCall(1, wif->name, cmd);
                        }
                    }
                }
                flag = 0;
                ssid = NULL;
                nr = NULL;
                bssid = msgaGetBin(attr);
                break;
            case attr_ssid:
                ssid = msgaGetStr(attr);
                break;
            case attr_flag:
                flag = msgaGetU8(attr);
                break;
            case attr_nr:
                nr = msgaGetBin(attr);
                nr_len = msgaGetLen(attr);
                break;
        }
    }

    if (bssid) {
        if (flag==FLAG_ADD) {
            if ((ssid) && (nr)) {
                ret = sprintf(cmd, "SET_NEIGHBOR "MACFMT" ssid=\\\"%s\\\" nr=", MACARG(bssid), ssid);
                ret += hex2String(nr, cmd+ret, nr_len);
                cmd[ret] = 0;
                dlist_for_each(wif, ctx->bsses, l) {
                    _hostapdCliCall(1, wif->name, cmd);
                }
            }
        } else if (flag==FLAG_DEL) {
            ret = sprintf(cmd, "REMOVE_NEIGHBOR "MACFMT, MACARG(bssid));
            dlist_for_each(wif, ctx->bsses, l) {
                _hostapdCliCall(1, wif->name, cmd);
            }
        }
    }

}

static int _nl80211SetNacEnable(struct cls_driver_ctx *ctx, char *ifname, uint8_t enable)
{
    struct nl_msg *msg;
    struct nlattr *params;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0, 0, NL80211_CMD_VENDOR, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(ifname));
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, CLSEMI_OUI);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, CLS_NL80211_CMD_SET_NAC_ENABLE);
    params = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    NLA_PUT_U32(msg, CLS_NL80211_ATTR_NAC_ENABLE, enable);
    nla_nest_end(msg, params);

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg, -1, -1, -1, -1, NL80211_CMD_VENDOR, 0) < 0) {
        DEBUG_ERROR("set nac %s failed\n", enable == 1 ? "enable" : "disable");
        return -1;;
    }

    DEBUG_INFO("set nac %s successfully\n", enable == 1 ? "enable" : "disable");
    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static int _nl80211FlushNacSta(struct cls_driver_ctx *ctx, char *ifname)
{
    struct nl_msg *msg;
    int ifidx = 0;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0, 0, NL80211_CMD_VENDOR, 0);

    ifidx = if_nametoindex(ifname);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, CLSEMI_OUI);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, CLS_NL80211_CMD_FLUSH_ALL_STA);

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg, -1, -1, -1, -1, NL80211_CMD_VENDOR, 0) < 0) {
        DEBUG_ERROR("flush nac sta failed\n");
        return -1;;
    }

    DEBUG_INFO("flush nac sta successfully\n");
    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static int _nl80211SetNacSta(struct cls_driver_ctx *ctx, int wiphyIdx, int ifidx, uint8_t flag, uint8_t *mac)
{
    struct nl_msg *msg;
    struct nlattr *params;
    uint32_t subcmd = 0;

    msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, ctx->cmd_ctrl->id, 0, 0, NL80211_CMD_VENDOR, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, wiphyIdx);
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, CLSEMI_OUI);

    if (0 == flag)
        subcmd = CLS_NL80211_CMD_ADD_NAC_MONITOR_STA;
    else
        subcmd = CLS_NL80211_CMD_DEL_NAC_MONITOR_STA;

    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, subcmd);
    params = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    NLA_PUT(msg, CLS_NL80211_ATTR_MAC_ADDR, MACLEN, mac);
    nla_nest_end(msg, params);

    if (_issueCMDNoWait(ctx->cmd_ctrl, msg, -1, ifidx, -1, -1, NL80211_CMD_VENDOR, 0) < 0) {
        DEBUG_ERROR("%s NAC sta["MACFMT"] failed\n", flag ? "DEL" : "ADD", MACARG(mac));
        return -1;;
    }

    DEBUG_INFO("%s NAC sta["MACFMT"] successfully\n", flag ? "DEL" : "ADD", MACARG(mac));
    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return -1;
}

static void processSetNacSta(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    int ret = 0;
    uint8_t channel = 0;
    uint8_t *mac = NULL;
    uint8_t flag = 0; //0: ADD, 1: DEL
    int ifidx = 0;
    int wiphyIdx = 0;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_channel) || !hasMsga(attrs, attr_mac) || !hasMsga(attrs, attr_flag))
        return;

    channel = msgaGetU8(&attrs[attr_channel]);
    mac = msgaGetBin(&attrs[attr_mac]);
    flag = msgaGetU8(&attrs[attr_flag]);

    /* channel to if index */
    ifidx = if_nametoindex(GET_DEFAULT_IFNAME_BY_CHANNEL(channel));
    wiphyIdx = _findWiphyIdxByIfidx(ctx, ifidx);

    if (wiphyIdx < 0) {
        DEBUG_ERROR("%s NAC sta("MACFMT") on %s failed for cannot find wiphy idx\n", flag == 0 ? "ADD":"DEL",
            MACARG(mac), GET_DEFAULT_IFNAME_BY_CHANNEL(channel));
        return;
    }

    if ((ret = _nl80211SetNacSta(ctx, wiphyIdx, ifidx, flag, mac)) < 0) {
        DEBUG_ERROR("%s NAC sta("MACFMT") on %s failed. return: %d\n",
            flag == 0 ? "ADD":"DEL", MACARG(mac), GET_DEFAULT_IFNAME_BY_CHANNEL(channel), ret);
    }
}

static void processSetNacEnable(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_channel) || !hasMsga(attrs, attr_flag))
        return;

    _nl80211SetNacEnable(ctx, GET_DEFAULT_IFNAME_BY_CHANNEL(msgaGetU8(&attrs[attr_channel])),
        msgaGetU8(&attrs[attr_flag]));
}

static void processFlushNacSta(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_channel))
        return;

    _nl80211FlushNacSta(ctx, GET_DEFAULT_IFNAME_BY_CHANNEL(msgaGetU8(&attrs[attr_channel])));
}

struct chan_survey_item {
    dlist_item l;
    uint8_t chan;
    int8_t noise;
    uint32_t busy_time;
    uint32_t dwell_time;
};

static DEFINE_DLIST_HEAD(chan_phyinfo_list);

static void _sendChanSurvey(struct wifi_interface_private *wif)
{
    uint8_t *buf = NULL;
    uint8_t *p;
    struct chan_survey_item *e;

    if (dlist_empty(&chan_phyinfo_list))
        return;

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, drv_evt_chan_survey);
    p = msgaPutU32(p, attr_if_idx, wif->index);

    dlist_for_each(e, chan_phyinfo_list, l) {
        p = msgaPutU8(p, attr_opclass, chanSurveyFindOpclass(e->chan));
        p = msgaPutU8(p, attr_channel, e->chan);
        p = msgaPutU8(p, attr_chan_noise, rssi2Rcpi(e->noise));
        p = msgaPutU32(p, attr_chan_busytime, e->busy_time);
        p = msgaPutU32(p, attr_chan_dwell, e->dwell_time);
    }
    msgSend(buf, p-buf);
    msgPutBuf(buf);

    dlist_free_items(&chan_phyinfo_list, struct chan_survey_item, l);

    return;
}

#define DFT_SCAN_DUR 30
static int _getChannelPhyInfo(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
    struct chan_survey_item *e;
    char dev[20];

    static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
        [NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
        [NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
        [NL80211_SURVEY_INFO_TIME] = { .type = NLA_U64},
        [NL80211_SURVEY_INFO_TIME_BUSY] = { .type = NLA_U64},
    };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
          genlmsg_attrlen(gnlh, 0), NULL);

    if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
    DEBUG_INFO("Survey data from %s\n", dev);

    if (!tb[NL80211_ATTR_SURVEY_INFO]) {
        DEBUG_WARNING("survey data missing!\n");
        return NL_SKIP;
    }

    if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX, tb[NL80211_ATTR_SURVEY_INFO], survey_policy) < 0) {
        DEBUG_WARNING("failed to parse nested attributes!\n");
        return NL_SKIP;
    }

    e = calloc(1, sizeof(struct chan_survey_item));
    if (!e) {
        DEBUG_WARNING("NO space for channel survey\n");
        return -1;
    }

    if (sinfo[NL80211_SURVEY_INFO_FREQUENCY])
            e->chan = (uint8_t)freq2Channel(nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]));
    if (sinfo[NL80211_SURVEY_INFO_NOISE])
            e->noise = (int8_t)(nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]));
    if (sinfo[NL80211_SURVEY_INFO_TIME_BUSY])
            e->busy_time = (uint32_t)nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME_BUSY]);
    if (sinfo[NL80211_SURVEY_INFO_TIME])
            e->dwell_time = (uint32_t)nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME]);

    if (0 == e->dwell_time)
        e->dwell_time = DFT_SCAN_DUR;

    DEBUG_INFO("chan survey from driver: ch[%d], noise=%d, busy_time=%d, dwell_time=%d\n",
                e->chan, e->noise, e->busy_time, e->dwell_time);

    dlist_add_tail(&chan_phyinfo_list, &e->l);
    return 0;
}

static void getChannelSurvey(struct wifi_interface_private *wif_private, struct sock_ctrl *cmd_ctrl,
                                    uint32_t ifidx)
{
    struct nl_msg *msg_buf;

    msg_buf = nlmsg_alloc();
    genlmsg_put(msg_buf, 0, 0, cmd_ctrl->id, 0, NLM_F_DUMP, NL80211_CMD_GET_SURVEY, 0);

    nla_put_u32(msg_buf, NL80211_ATTR_IFINDEX, ifidx);
    nla_put_u32(msg_buf, NL80211_ATTR_SURVEY_RADIO_STATS, 1);
    DEBUG_INFO("go to get channel survey data \n");
    if (_issueCMD(cmd_ctrl, msg_buf, -1, -1, -1, -1, NL80211_CMD_GET_SURVEY, NLM_F_DUMP,
                _getChannelPhyInfo, wif_private) < 0) {
        DEBUG_ERROR("get channel survey on interface-%u failed.\n", ifidx);
        return;
    }
    _sendChanSurvey(wif_private);
}

static void processGetChanSurvey(struct cls_driver_ctx * ctx, uint8_t * msg, uint32_t len)
{
    struct wifi_interface_private *wif_private = NULL;
    uint32_t ifidx = -1;
    struct msg_attr attrs[attr_drv_max] = {0};

    msgaParse(attrs, attr_drv_max, msg, len);

    if (!hasMsga(attrs, attr_if_idx))
        return;

    ifidx = msgaGetU32(&attrs[attr_if_idx]);
    wif_private = _findWifiInterfaceIdx(ctx, ifidx);
    if (!wif_private || !wif_private->radio)
        return;
    getChannelSurvey(wif_private, ctx->cmd_ctrl, ifidx);
}

static void processDrvCmd(void *data, uint8_t *msg, uint32_t len)
{
    uint8_t   evt = msg[0];
    struct cls_driver_ctx *ctx = data;

    switch(evt) {
        case drv_cmd_teardown:
            processTearDown(ctx, msg+1, len-1);
            break;
        case drv_cmd_bss:
            processAddBSS(ctx, msg+1, len-1);
            break;
        case drv_cmd_commit:
            processCommit(ctx, msg+1, len-1);
            break;
        case drv_cmd_set_channel_txpower:
            processSetChannelTXPower(ctx, msg+1, len-1);
            break;
        case drv_cmd_send_mgmt_frame:
            processSendMgmtFrame(ctx, msg+1, len-1);
            break;
        case drv_cmd_register_mgmt_frame:
            processRegisterMgmtFrame(ctx, msg+1, len-1);
            break;
        case drv_cmd_get_station_stats:
            processGetStationStats(ctx, msg+1, len-1);
            break;
        case drv_cmd_trigger_scan:
            processTriggerScan(ctx, msg+1, len-1);
            break;
        case drv_cmd_get_chan_scan_results:
            processGetChanScanResults(ctx, msg+1, len-1);
            break;
        case drv_cmd_deauth_sta:
            processSTADeauth(ctx, msg+1, len-1);
            break;
        case drv_cmd_deny_sta_add:
        case drv_cmd_deny_sta_del:
            processSTADeny(ctx, msg+1, len-1, evt);
            break;
        case drv_cmd_start_wps:
            processStartWPS(ctx, msg+1, len-1);
            break;
        case drv_cmd_queue_action:
            processQueueAction(ctx, msg+1, len-1);
            break;
        case drv_cmd_dscp_action:
            processDSCPAction(ctx, msg+1, len-1);
            break;
        case drv_cmd_vip_add:
            processVipAdd(ctx, msg+1, len-1);
            break;
        case drv_cmd_tc_action:
            processTCAction(ctx, msg+1, len-1);
            break;
        case drv_cmd_vip_clear:
            _nl80211ClearAllVips(ctx);
            break;
        case drv_cmd_set_neighbor:
            processSetNeighbor(ctx, msg+1, len-1);
            break;
        case drv_cmd_eth_vlan:
            processEthVLAN(ctx, msg+1, len-1);
            break;
        case drv_cmd_set_nac_sta:
            processSetNacSta(ctx, msg+1, len-1);
            break;
        case drv_cmd_set_nac_enable:
            processSetNacEnable(ctx, msg+1, len-1);
            break;
        case drv_cmd_flush_nac_sta:
            processFlushNacSta(ctx, msg+1, len-1);
            break;
        case drv_cmd_get_chan_survey:
            processGetChanSurvey(ctx, msg+1, len-1);
            break;
        default:
            DEBUG_WARNING("Unknown driver cmd type (%d)\n", evt);
            break;
    }
}

static const struct blobmsg_policy service_watch_policy = { "config", BLOBMSG_TYPE_STRING };

static int serviceEventHandler(struct ubus_context *ctx, struct ubus_object *obj,
                           struct ubus_request_data *req, const char *method,
                           struct blob_attr *msg)
{
    struct blob_attr *attr;
    const char *config;
    uint8_t *buf, *p;
    uint32_t evt = drv_evt_config_change;

    if (strcmp(method, "config.change"))
        return 0;

    blobmsg_parse(&service_watch_policy, 1, &attr, blob_data(msg), blob_len(msg));
    if (!attr)
        return 0;
    config = blobmsg_get_string(attr);

    if (strcmp(config, "wireless")) {
        if (0 == strcmp(config, "cls-qos"))
            evt = drv_evt_vip_conf_changed;
        else
            return 0;
    }

    buf = msgGetBuf(0);
    p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_evt, evt);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

static int _addServiceListener(struct cls_driver_ctx *ctx)
{
    int ret;
    struct ubus_context *ubus = PLATFORM_GET_UBUS();
    struct notification_ctx *service = &ctx->service_notifcation;

    service->suscriber[0].cb = serviceEventHandler;

    if ((ret = ubus_lookup_id(ubus, "service", &service->id))) {
        DEBUG_ERROR("can not listen on ubus:service!\n");
        goto bail;
    }

    if ((ret = ubus_register_subscriber(ubus, &service->suscriber[0]))) {
        DEBUG_ERROR("can not register subscriber\n");
        goto bail;
    }

    if ((ret = ubus_subscribe(ubus, &service->suscriber[0], service->id))) {
        DEBUG_ERROR("can not subscribe\n");
        goto bail;
    }

    ret = 0;
bail:
    return ret;
}

static int _clsDriverStart(void *data, char *cmdline)
{
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)data;
    struct wiphy_private *priv;
    struct wifi_interface_private *wif_priv = NULL;
    int flag = 0, ret;

    ret = _issueCMD(ctx->cmd_ctrl, NULL, -1, -1, -1, -1, NL80211_CMD_GET_PROTOCOL_FEATURES, flag,
            _nlFeatureHandler, ctx);

    if (!cmdline)
        flag = NLM_F_DUMP;

    ret = _issueCMD(ctx->cmd_ctrl, NULL, -1, -1, -1, (ctx->split_wiphy_dump ? NL80211_ATTR_SPLIT_WIPHY_DUMP:-1),
                    NL80211_CMD_GET_WIPHY, flag,
                    _wiphyHandler, ctx);

    dlist_for_each(priv, ctx->radios, l) {
        /* get radio vbss capabilities by cls_api */
        _getWiphyVBSSCapability(priv);
        /* get wifi interface info */
        ret = _issueCMD(ctx->cmd_ctrl, NULL, priv->idx, -1, -1, -1,
                NL80211_CMD_GET_INTERFACE, flag, _wifiInterfaceHandler, ctx);
        /* notify datamodel radio info */
        _sendRadioInfo(priv);
    }

    /* notify datamodel wifi interface info */
    ret = _sendBssInfo(ctx);

    /* check if wifi interface info correctly if not try get again util correctly */
    dlist_for_each(wif_priv, ctx->bsses, l) {
        if (wif_priv->role != role_ap)
            continue;
        /* CLEAR deny acl sta list & flush nac sta*/
        if (wif_priv->name) {
            _hostapdCliCall(1, wif_priv->name, "DENY_ACL CLEAR");
            _nl80211FlushNacSta(ctx, wif_priv->name);
            _nl80211SetNacEnable(ctx, wif_priv->name, 0);
        }
        if (WIFI_INTERFACE_INFO_CORRECT(wif_priv)) {
            _addHostapdListener(wif_priv);
            _nl80211GetStation(ctx, wif_priv->index, NULL);
        } else
            _startGetWifInterfaceTimer(ctx, wif_priv);
    }

    msgRegisterFamily(msg_family_driver_cmd, processDrvCmd, ctx);
    ctx->nl80211evt_task = platformAddTask(_nl80211EventLoop, ctx);
    ctx->rtmevt_task = platformAddTask(_rtmEventLoop, ctx);

    _addServiceListener(ctx);

    return ret;
}

static void _clsDriverStop(void *data)
{
}

static void _clsDriverDeinit(void *data)
{
    struct cls_driver_ctx *ctx = (struct cls_driver_ctx *)data;
    struct wiphy_private *radio, *r;
    struct wifi_interface_private *bss, *b;
    struct frame_registered_item *reg, *tmp_reg;
    DEBUG_WARNING("_clsDriverDeinit\n");

    if (ctx) {
        if (ctx->cmd_ctrl)
            _disconnectDriver(ctx->cmd_ctrl);
        if (ctx->event_ctrl)
            _disconnectDriver(ctx->event_ctrl);
        if (ctx->rtm_ctrl)
            _disconnectDriver(ctx->rtm_ctrl);

        dlist_for_each_safe(bss, b, ctx->bsses, l) {
            _wifiInterfaceDelete(bss);
        }

        dlist_for_each_safe(radio, r, ctx->radios, l) {
            _wiphyDelete(radio);
        }

        dlist_for_each_safe(reg, tmp_reg, ctx->registered_frames, l) {
            dlist_remove(&reg->l);
            if (reg->match)
                free(reg->match);
            free(reg);
        }

        free(ctx);
    }
}

static struct extension_ops _cls_driver_ops = {
    .init = _clsDriverInit,
    .start = _clsDriverStart,
    .stop = _clsDriverStop,
    .deinit = _clsDriverDeinit,
};


void clsDriverLoad()
{
    registerExtension(&_cls_driver_ops);
}
