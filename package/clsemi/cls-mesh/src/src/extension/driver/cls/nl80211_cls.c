#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/nl80211.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netinet/ether.h>
#include "cls/cls_nl80211_vendor.h"
#include "nl80211_cls.h"
#include "platform.h"
#include "datamodel.h"
#include "wifi.h"


struct nac_monitor_sta_info {
    uint8_t mac_addr[MACLEN];
    uint8_t rssi;
    uint8_t sinr;
    uint32_t last_rx_time;
};


static int _getInterfaceModeHandler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    int *mode = (int *)arg;

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                genlmsg_attrlen(gnlh, 0), NULL);

    if (tb[NL80211_ATTR_IFTYPE]) {
        enum nl80211_iftype iftype = nla_get_u32(tb[NL80211_ATTR_IFTYPE]);
        switch (iftype) {
            case NL80211_IFTYPE_AP:
                *mode = role_ap;
                break;
            case NL80211_IFTYPE_STATION:
                *mode = role_sta;
                break;
            default:
                *mode = -2;
                break;

        }
    }

    return NL_SKIP;
}

int nl80211GetInterfaceMode(const char *ifname)
{
    struct nl_sock *sock = NULL;
    int family = -1;
    struct nl_msg *msg = NULL;
    int ifindex = if_nametoindex(ifname);
    int ret = 0;
    int mode = -1;
    struct nl_cb *cb;

    if (ifindex == 0) {
        DEBUG_WARNING("Interface %s not found\n", ifname);
        return -1;
    }
    sock = nl_socket_alloc();
    if (!sock) {
        DEBUG_WARNING("Failed to allocate netlink socket\n");
        return -1;
    }
    if (genl_connect(sock)) {
        DEBUG_WARNING("Failed to connect to netlink\n");
        nl_socket_free(sock);
        return -1;
    }
    family = genl_ctrl_resolve(sock, "nl80211");
    if (family < 0) {
        DEBUG_WARNING("nl80211 not found\n");
        nl_socket_free(sock);
        return -1;
    }
    msg = nlmsg_alloc();
    if (!msg) {
        DEBUG_WARNING("Failed to allocate netlink message\n");
        return -1;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        DEBUG_WARNING("Failed to allocate netlink callback\n");
        nlmsg_free(msg);
        return -1;
    }

    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _getInterfaceModeHandler, &mode);
    genlmsg_put(msg, 0, 0, family, 0, 0, NL80211_CMD_GET_INTERFACE, 0);
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifindex);
    ret = nl_send_auto_complete(sock, msg);
    if (ret < 0) {
        DEBUG_WARNING("Failed to send netlink message\n");
        goto out;
    }
    ret = nl_recvmsgs(sock, cb);
    if (ret < 0) {
        DEBUG_WARNING("Failed to receive netlink message\n");
        goto out;
    }

nla_put_failure:
out:
    nl_cb_put(cb);
    nlmsg_free(msg);

    if (sock) nl_socket_free(sock);

    return mode;
}


static int _getNacStaInfoHandler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct nlattr *nl_vendor;
    struct nlattr *tb_vendor[CLS_NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nac_monitor_sta_info nac_sta_info = {0};
    int8_t *rssi = (int8_t *)arg;

    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb_msg[NL80211_ATTR_VENDOR_DATA]) {
        DEBUG_ERROR("get NAC info FAILED for no NL80211_ATTR_VENDOR_DATA\n");
        goto bail;
    }

    nl_vendor = tb_msg[NL80211_ATTR_VENDOR_DATA];

    nla_parse(tb_vendor, CLS_NL80211_ATTR_MAX, nla_data(nl_vendor), nla_len(nl_vendor), NULL);

    if (!tb_vendor[CLS_NL80211_ATTR_NAC_MONITOR_STA]) {
        DEBUG_ERROR("get NAC info FAILED for no CLS_NL80211_ATTR_NAC_MONITOR_STA\n");
        goto bail;
    }

    memcpy(&nac_sta_info, nla_data(tb_vendor[CLS_NL80211_ATTR_NAC_MONITOR_STA]), sizeof(nac_sta_info));
    DEBUG_INFO("get NAC sta["MACFMT"] info: rssi: %d, %u, sinr: %u\n", MACARG(nac_sta_info.mac_addr),
        (int8_t)(nac_sta_info.rssi), nac_sta_info.rssi, nac_sta_info.sinr);

    *rssi = (int8_t)nac_sta_info.rssi;

bail:
    return NL_SKIP;
}

int nl80211GetNacStaInfo(uint8_t channel, uint8_t *mac)
{
    struct nl_sock *sock = NULL;
    int family = -1;
    int ifidx = 0;
    struct nl_msg *msg = NULL;
    struct nlattr *params;
    int ret = 0;
    int8_t rssi = 0;
    struct nl_cb *cb;

    sock = nl_socket_alloc();
    if (!sock) {
        DEBUG_WARNING("Failed to allocate netlink socket\n");
        return -1;
    }
    if (genl_connect(sock)) {
        DEBUG_WARNING("Failed to connect to netlink\n");
        nl_socket_free(sock);
        return -1;
    }
    family = genl_ctrl_resolve(sock, "nl80211");
    if (family < 0) {
        DEBUG_WARNING("nl80211 not found\n");
        nl_socket_free(sock);
        return -1;
    }
    msg = nlmsg_alloc();
    if (!msg) {
        DEBUG_WARNING("Failed to allocate netlink message\n");
        return -1;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        DEBUG_WARNING("Failed to allocate netlink callback\n");
        nlmsg_free(msg);
        return -1;
    }

    /* channel to if index */
    ifidx = if_nametoindex(GET_DEFAULT_IFNAME_BY_CHANNEL(channel));

    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _getNacStaInfoHandler, &rssi);

    genlmsg_put(msg, 0, 0, family, 0, 0, NL80211_CMD_VENDOR, 0);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, CLSEMI_OUI);
    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, CLS_NL80211_CMD_GET_NAC_MONITOR_STA_INFO);
    params = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    NLA_PUT(msg, CLS_NL80211_ATTR_MAC_ADDR, MACLEN, mac);
    nla_nest_end(msg, params);

    ret = nl_send_auto_complete(sock, msg);
    if (ret < 0) {
        DEBUG_WARNING("Failed to send netlink message\n");
        goto out;
    }
    ret = nl_recvmsgs(sock, cb);
    if (ret < 0) {
        DEBUG_WARNING("Failed to receive netlink message\n");
        goto out;
    }

nla_put_failure:
out:
    nl_cb_put(cb);
    nlmsg_free(msg);

    if (sock) nl_socket_free(sock);

    return rssi2Rcpi(rssi);
}

