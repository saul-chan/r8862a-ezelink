#ifdef CONFIG_VBSS

#include "platform.h"
#include "extension.h"
#include "feature/feature.h"
#include "datamodel.h"
#include "platform_os.h"
#include "al_send.h"
#include "al_utils.h"
#include "al_msg.h"
#include "al_action.h"
#include "wifi.h"
#include "clsapi_wifi.h"
#include "vbss.h"

// TODO: need add age timer to release node
static DEFINE_DLIST_HEAD(vbss_switch_head);
mac_address client_mac;
    mac_address bssid;
    mac_address src_al_mac;
    mac_address dst_al_mac;
    mac_address src_ruid;
    mac_address dst_ruid;
    enum vbss_state_e state;

static void _vbssSwitchEntryPrint(struct vbss_switch_entry *e)
{
    DEBUG_INFO("VBSS Entry->client_mac: "MACFMT"\n", MACARG(e->client_mac));
    DEBUG_INFO("VBSS Entry->bssid     : "MACFMT"\n", MACARG(e->bssid));
    DEBUG_INFO("VBSS Entry->src_al_mac: "MACFMT"\n", MACARG(e->src_al_mac));
    DEBUG_INFO("VBSS Entry->dst_al_mac: "MACFMT"\n", MACARG(e->dst_al_mac));
    DEBUG_INFO("VBSS Entry->src_ruid  : "MACFMT"\n", MACARG(e->src_ruid));
    DEBUG_INFO("VBSS Entry->dst_ruid  : "MACFMT"\n", MACARG(e->dst_ruid));
    DEBUG_INFO("VBSS Entry->state     : %u\n", e->state);
}

static struct vbss_switch_entry *_vbssSwitchEntryFind(uint8_t *bssid)
{
    struct vbss_switch_entry *e = NULL;

    dlist_for_each(e, vbss_switch_head, l) {
        if (!MACCMP(e->bssid, bssid))
            return e;
    }

    return NULL;
}

static struct vbss_switch_entry *_vbssSwitchEntryAdd(uint8_t *client_mac, uint8_t *bssid,
        uint8_t *src_al_mac, uint8_t *dst_al_mac, uint8_t *src_ruid, uint8_t *dst_ruid)
{
    struct vbss_switch_entry *e = NULL;

    e = _vbssSwitchEntryFind(bssid);
    if (!e)
        e = (struct vbss_switch_entry *)calloc(1, sizeof(struct vbss_switch_entry));
    if (!e)
        return NULL;

    MACCPY(e->client_mac, client_mac);
    MACCPY(e->bssid, bssid);
    MACCPY(e->src_al_mac, src_al_mac);
    MACCPY(e->dst_al_mac, dst_al_mac);
    MACCPY(e->src_ruid, src_ruid);
    MACCPY(e->dst_ruid, dst_ruid);
    VBSS_STATE_SWITCH(e, VBSS_STATE_START);

    dlist_add_head(&vbss_switch_head, &e->l);

    return e;
}

static void _vbssSwitchEntryDel(struct vbss_switch_entry *e)
{
    if (!e)
        return;

    dlist_remove(&e->l);
    free(e);
    e = NULL;

    return;
}

static int _vbssStartEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct client *sta = NULL;
    struct radio *target_r = NULL;
    struct al_device *src_al_device = NULL;
    struct al_device *dst_al_device = NULL;
    struct vbss_switch_entry *e = NULL;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_mac) || !hasMsga(attrs, attr_al_mac) || !hasMsga(attrs, attr_radio_mac))
        return -1;

    target_r = radioFind(NULL, msgaGetBin(&attrs[attr_radio_mac]));
    if (!target_r)
        return -1;

    src_al_device = alDeviceFindAny(msgaGetBin(&attrs[attr_al_mac]));
    dst_al_device = target_r->d;
    if (!src_al_device)
        return -1;

    sta = clientFind(src_al_device, NULL, msgaGetBin(&attrs[attr_mac]));
    if (!sta || !sta->wif)
        return -1;

    e = _vbssSwitchEntryAdd(sta->mac, sta->wif->bssInfo.bssid, src_al_device->al_mac,
                dst_al_device->al_mac, sta->wif->radio->uid, target_r->uid);
    if (!e)
        return -1;

    /* record dest agent info on source */
    MACCPY(sta->vbss_ctx.target_ruid, target_r->uid);
    MACCPY(sta->vbss_ctx.source_agent, src_al_device->al_mac);

    VBSS_STATE_SWITCH(e, VBSS_STATE_PREPARE);

    _vbssSwitchEntryPrint(e);

    return sendClientSecurityContextRequest(idx2InterfaceName(src_al_device->recv_intf_idx), getNextMid(),
        src_al_device->al_mac, sta->wif->bssInfo.bssid, sta->mac);
}

static int _vbssClientSecurityReqEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct client *sta = NULL;
    struct wifi_interface *wif = NULL;
    struct al_device *d = NULL;
    struct vbss_sta_info sta_info;
    struct vvData ptk;
    struct vvData gtk;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_cmdu_id) || !hasMsga(attrs, attr_al_mac) ||
        !hasMsga(attrs, attr_bssid) || !hasMsga(attrs, attr_mac))
        return -1;

    d = alDeviceFindAny(msgaGetBin(&attrs[attr_al_mac]));
    wif = (struct wifi_interface *)interfaceFind(local_device, msgaGetBin(&attrs[attr_bssid]), interface_type_wifi);
    if (!wif) {
        DEBUG_ERROR("can not find wifi interface: sta["MACFMT"], bssid["MACFMT"] on local device\n",
            MACARG(msgaGetBin(&attrs[attr_mac])), MACARG(msgaGetBin(&attrs[attr_bssid])));
        return -1;
    }

    sta = clientFind(NULL, wif, msgaGetBin(&attrs[attr_mac]));
    if (!sta)
        return -1;

    // get info by clsapi
    if (CLSAPI_GET_VBSS_STA(wif->i.name, msgaGetBin(&attrs[attr_mac]), &sta_info) < 0) {
        DEBUG_ERROR("clsapi_wifi_get_vbss_sta failed: sta("MACFMT") bssid("MACFMT")\n",
            MACARG(msgaGetBin(&attrs[attr_mac])), MACARG(msgaGetBin(&attrs[attr_bssid])));
        return -1;
    }

    ptk.len = WPA_TK_MAX_LEN;
    ptk.datap = sta_info.key_info.tk;
    gtk.len = sta_info.key_info.GTK_len;
    gtk.datap = sta_info.key_info.GTK[0];

    // TODO: sta info has no group tx packet number
    sendClientSecurityContextResponse(idx2InterfaceName(d->recv_intf_idx), msgaGetU16(&attrs[attr_cmdu_id]),
        d->al_mac, sta_info.bssid, sta_info.mac_addr, &ptk, &gtk, sta->traffic_stats.packets_tx, 0);

    return 0;
}

static int _vbssCreationEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct al_device *d = NULL;
    struct radio *r = NULL;
    struct wifi_interface *wif = NULL;
    struct client *c = NULL;
    struct vbss_vap_info vap_info = {0};
    struct vbss_sta_info sta_info = {0};
    uint8_t client_assoc_flag = 0;
    uint8_t *ptk = NULL;
    uint16_t ptk_len = 0;
    uint8_t *gtk = NULL;
    uint16_t gtk_len = 0;
    uint8_t *sta_assoc_req = NULL;
    uint16_t sta_assoc_req_len = 0;
    char ifname[32] = {0};

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_cmdu_id) || !hasMsga(attrs, attr_al_mac) || !hasMsga(attrs, attr_radio_mac) ||
        !hasMsga(attrs, attr_bssid) || !hasMsga(attrs, attr_ssid) || !hasMsga(attrs, attr_client_assoc) ||
        !hasMsga(attrs, attr_station_tx_packets) || !hasMsga(attrs, attr_station_tx_group_packets)) {
        DEBUG_WARNING("lack params\n");
        return -1;
    }

    DEBUG_INFO("radio("MACFMT"), bssid("MACFMT"), ssid(%s)\n", MACARG(msgaGetBin(&attrs[attr_radio_mac])),
        MACARG(msgaGetBin(&attrs[attr_bssid])), msgaGetBin(&attrs[attr_ssid]));

    /* TODO: judge if bssid match local mask */

    client_assoc_flag = msgaGetU8(&attrs[attr_client_assoc]);
    if (hasMsga(attrs, attr_key)) {
        ptk = msgaGetBin(&attrs[attr_key]);
        ptk_len = msgaGetLen(&attrs[attr_key]);
    }
    if (hasMsga(attrs, attr_group_key)) {
        gtk = msgaGetBin(&attrs[attr_group_key]);
        gtk_len = msgaGetLen(&attrs[attr_group_key]);
    }
    if (hasMsga(attrs, attr_frame)) {
        sta_assoc_req = msgaGetBin(&attrs[attr_frame]);
        sta_assoc_req_len = msgaGetLen(&attrs[attr_frame]);
    }
    d = alDeviceFindAny(msgaGetBin(&attrs[attr_al_mac]));

    r = radioFind(local_device, msgaGetBin(&attrs[attr_radio_mac]));
    if (!r) {
        DEBUG_WARNING("radio("MACFMT") not exist\n", MACARG(msgaGetBin(&attrs[attr_radio_mac])));
        goto add_wif_failed;
    }

    /* when use clsapi if set phy0 use wlan0 instead, if set phy1 use wlan1 instead. */
    char intf_name[16] = {0};
    snprintf(intf_name, sizeof(intf_name) - 1, "wlan%d", r->index);

    /* if dubhe1000 clear all vaps first */
#ifdef DUBHE1000
    DEBUG_INFO("DUBHE1000 only support one vap, delete others first.\n");
    for (int i = r->configured_bsses.len; i > 0; i--) {
        if (!r->configured_bsses.p[i-1]->i.name) {
            DEBUG_WARNING("wifi interface("MACFMT")'s ifname is NULL\n", MACARG(r->configured_bsses.p[i-1]->bssInfo.bssid));
            continue;
        }
        /* keep base wlan */
        if (!strcmp(r->configured_bsses.p[i-1]->i.name, intf_name))
            continue;
        if (CLSAPI_DEL_VBSS_VAP(r->configured_bsses.p[i-1]->i.name) < 0) {
            DEBUG_WARNING("del vap %s("MACFMT") failed\n", r->configured_bsses.p[i-1]->i.name,
                MACARG(r->configured_bsses.p[i-1]->bssInfo.bssid));
        }
        if (r->current_vbss_num > 0)
            r->current_vbss_num -= 1;
        wifiInterfaceDelete(r->configured_bsses.p[i-1]);
    }
#endif

    wif = wifiInterfaceAdd(local_device, r, msgaGetBin(&attrs[attr_bssid]));
    if (!wif) {
        DEBUG_WARNING("wifi interface("MACFMT") create failed\n", MACARG(msgaGetBin(&attrs[attr_bssid])));
        goto add_wif_failed;
    }
    wif->radio = r;
    wif->is_vbss = true;
    REPLACE_STR(wif->i.name, strdup(ifname));
    MACCPY(wif->bssInfo.bssid, msgaGetBin(&attrs[attr_bssid]));
    wif->bssInfo.ssid.len = msgaGetLen(&attrs[attr_ssid]);
    memcpy(wif->bssInfo.ssid.ssid, msgaGetBin(&attrs[attr_ssid]), msgaGetLen(&attrs[attr_ssid]));
    wif->bssInfo.auth = WPA_AUTHORIZE_TYPE_NONE;

    /* call clsapi add vap */
    /* if client_assoc flag is 0 means need to call clsapi enable vbss first */
    if (!client_assoc_flag) {
        DEBUG_INFO("client_assoc flag is 0. then call vbss enable clsapi first.\n");
        snprintf(ifname, sizeof(ifname) - 1, "%s-1", intf_name);
        vap_info.role = 0; // VBSS_VAP_ORIGIN

        if(CLSAPI_SET_VBSS_ENABLE(intf_name, true) < 0) {
            DEBUG_WARNING("radio("MACFMT"): clsapi_wifi_set_vbss_enabled failed\n", MACARG(r->uid));
            goto add_vap_failed;
        }
    }
    else {
        if (!hasMsga(attrs, attr_mac)) {
            DEBUG_WARNING("lack client mac when create vbss with client info\n");
            goto add_vap_failed;
        }

        c = clientNew(wif, msgaGetBin(&attrs[attr_mac]));
        if (!c) {
            DEBUG_WARNING("add sta("MACFMT") failed\n", MACARG(msgaGetBin(&attrs[attr_mac])));
            goto add_vap_failed;
        }
        c->traffic_stats.packets_rx = msgaGetU64(&attrs[attr_station_tx_packets]);
        if (sta_assoc_req) {
            REPLACE_VVDATA(c->last_assoc, sta_assoc_req, sta_assoc_req_len);
            parseAssocFrame(c, 0);
        }

        vap_info.role = 1; // VBSS_VAP_ROAMING
        snprintf(ifname, sizeof(ifname) - 1, "%s-%d", intf_name, r->configured_bsses.len);
        MACCPY(sta_info.mac_addr, msgaGetBin(&attrs[attr_mac]));
        MACCPY(sta_info.bssid, msgaGetBin(&attrs[attr_bssid]));
        /* parse assoc req frame to fill the sta info */
        sta_info.aid = (sta_info.bssid[4] << 8 | sta_info.bssid[5]) & 0x7ff;
        sta_info.capability = ntohs(*((uint16_t *)sta_assoc_req));
        if (c->ies.supported_rates && c->ies.supported_rates[1] <= WLAN_SUPP_RATES_MAX) {
            sta_info.supported_rates_len = c->ies.supported_rates[1];
            for (int i = 0; i < c->ies.supported_rates[1]; i++) {
                sta_info.supported_rates[i] = c->ies.supported_rates[2+i];
            }
        }
        if (c->ies.ht_cap) {
            memcpy(&sta_info.ht_capabilities, c->ies.ht_cap, sizeof(struct ieee80211_ht_capabilities));
        }
        if (c->ies.vht_cap) {
            memcpy(&sta_info.vht_capabilities, c->ies.vht_cap, sizeof(struct ieee80211_vht_capabilities));
        }
        if (c->ies.he_cap) {
            memcpy(&sta_info.he_capab, c->ies.he_cap, sizeof(struct ieee80211_he_capabilities));
        }

        if (ptk)
            memcpy(sta_info.key_info.tk, ptk, ptk_len);
        if (gtk) {
            sta_info.key_info.GTK_len = gtk_len;
            memcpy(sta_info.key_info.GTK[0], gtk, gtk_len);
        }
        /* TODO: no tx stats field in vap_sta_info */
    }

    /* fill vap info. TODO: pwd attr need to be filled */
    MACCPY(vap_info.bssid, msgaGetBin(&attrs[attr_bssid]));
    memcpy(vap_info.ssid, msgaGetBin(&attrs[attr_ssid]), msgaGetLen(&attrs[attr_ssid]));
    //TODO: need confirm, set to default first.
    vap_info.auth_type = WPA_AUTHORIZE_TYPE_NONE;
    memcpy(vap_info.ifname, ifname, strlen(ifname)+1);

    if (CLSAPI_ADD_VBSS_VAP(&vap_info, ifname, sizeof(ifname)) < 0) {
        DEBUG_WARNING("add vap("MACFMT") failed\n", MACARG(vap_info.bssid));
        goto add_vap_failed;
    }
    r->current_vbss_num += 1;

    DEBUG_INFO("add vap("MACFMT"), ifname(%s) success\n", MACARG(vap_info.bssid), vap_info.ifname);

    if (client_assoc_flag){
        wif->vbss_client_context = wifiInterfaceAddVbssClientContext(wif, sta_info.mac_addr,
            msgaGetU64(&attrs[attr_station_tx_packets]), msgaGetU64(&attrs[attr_station_tx_group_packets]),
            ptk, ptk_len, gtk, gtk_len);

        if (CLSAPI_ADD_VBSS_STA(ifname, &sta_info) < 0) {
            DEBUG_WARNING("add vbss sta("MACFMT") failed\n", MACARG(sta_info.mac_addr));
            goto add_sta_failed;
        }

        if (CLSAPI_TRIGGER_SWITCH(ifname, sta_info.mac_addr) < 0) {
            DEBUG_WARNING("trigger sta("MACFMT") switch failed\n", MACARG(sta_info.mac_addr));
            goto trigger_switch_failed;
        }

        DEBUG_INFO("trigger vbss sta("MACFMT") switch success.\n", MACARG(sta_info.mac_addr));
    }

    /* Agent shall send a 1905 Topology Nofitication message */
    if (c)
        sendTopologyNotification(getNextMid(), c, 1);

    sendVBSSResponse(idx2InterfaceName(d->recv_intf_idx), msgaGetU16(&attrs[attr_cmdu_id]),
        d->al_mac, msgaGetBin(&attrs[attr_radio_mac]), msgaGetBin(&attrs[attr_bssid]), 1);

    /* Agent shall send a 1905 Topology Response message after operating vbss */
    return sendTopologyResponse(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);

trigger_switch_failed:
    CLSAPI_DEL_VBSS_STA(ifname, sta_info.mac_addr);

add_sta_failed:
    if (r->current_vbss_num > 0)
        r->current_vbss_num -= 1;
    CLSAPI_DEL_VBSS_VAP(ifname);
    if (c)
        clientDelete(c);

add_vap_failed:
    wifiInterfaceDelete(wif);

add_wif_failed:
    return sendVBSSResponse(idx2InterfaceName(d->recv_intf_idx), msgaGetU16(&attrs[attr_cmdu_id]),
        d->al_mac, msgaGetBin(&attrs[attr_radio_mac]), msgaGetBin(&attrs[attr_bssid]), 0);
}

static int _vbssDestructionEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct al_device *d = NULL;
    struct radio *r = NULL;
    struct wifi_interface *wif = NULL;
    struct client *c = NULL;
    struct client *tmp = NULL;
    uint8_t client_disassoc_flag = 0;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_cmdu_id) || !hasMsga(attrs, attr_al_mac) || !hasMsga(attrs, attr_radio_mac) ||
        !hasMsga(attrs, attr_bssid) || !hasMsga(attrs, attr_client_disassoc))
        return -1;

    client_disassoc_flag = msgaGetU8(&attrs[attr_client_disassoc]);
    d = alDeviceFindAny(msgaGetBin(&attrs[attr_al_mac]));

    r = radioFind(local_device, msgaGetBin(&attrs[attr_radio_mac]));
    if (!r) {
        DEBUG_INFO("radio("MACFMT") not exist.\n", MACARG(msgaGetBin(&attrs[attr_radio_mac])));
        goto destruction_failed;
    }

    wif = (struct wifi_interface *)interfaceFind(local_device, msgaGetBin(&attrs[attr_bssid]), interface_type_wifi);
    if (!wif) {
        DEBUG_INFO("bss("MACFMT") does not exist\n", MACARG(msgaGetBin(&attrs[attr_bssid])));
        goto destruction_failed;
    }

    /* one vap one client */
    dlist_for_each_safe(c, tmp, wif->clients, l) {
        DEBUG_INFO("bss("MACFMT") find vbss sta("MACFMT")\n", MACARG(msgaGetBin(&attrs[attr_bssid])),
            MACARG(c->mac));
        if (client_disassoc_flag) {
            DEBUG_INFO("del client and send topology notification\n");
            /* Agent shall send a 1905 Topology Nofitication message */
            sendTopologyNotification(getNextMid(), c, 0);

            CLSAPI_SWITCH_DONE(wif->i.name, msgaGetBin(&attrs[attr_mac]));
            CLSAPI_DEL_VBSS_STA(wif->i.name, c->mac);

            clientDelete(c);
        }
        break;
    }
    DEBUG_INFO("\n");

    if (CLSAPI_DEL_VBSS_VAP(wif->i.name) < 0) {
        DEBUG_WARNING("del vap("MACFMT") failed\n", MACARG(wif->bssInfo.bssid));
        // TODO: ingore fail now
        //goto destruction_failed;
    }
    DEBUG_INFO("\n");
    wifiInterfaceDelete(wif);
    DEBUG_INFO("\n");
    DEBUG_INFO("del vap("MACFMT") success\n", MACARG(msgaGetBin(&attrs[attr_bssid])));

    if (r->current_vbss_num > 0)
        r->current_vbss_num -= 1;

    sendVBSSResponse(idx2InterfaceName(d->recv_intf_idx), msgaGetU16(&attrs[attr_cmdu_id]),
        d->al_mac, msgaGetBin(&attrs[attr_radio_mac]), msgaGetBin(&attrs[attr_bssid]), 1);
    DEBUG_INFO("\n");

    /* Agent shall send a 1905 Topology Response message after operating vbss */
    sendTopologyResponse(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
    DEBUG_INFO("\n");

destruction_failed:
    return sendVBSSResponse(idx2InterfaceName(d->recv_intf_idx), msgaGetU16(&attrs[attr_cmdu_id]),
        d->al_mac, msgaGetBin(&attrs[attr_radio_mac]), msgaGetBin(&attrs[attr_bssid]), 0);
}

static int _vbssResponseEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct vbss_switch_entry *e = NULL;
    struct al_device *dst_agent = NULL;
    struct al_device *src_agent = NULL;
    struct radio *dst_radio = NULL;
    uint8_t success = 0;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_al_mac) || !hasMsga(attrs, attr_radio_mac) ||
        !hasMsga(attrs, attr_bssid) || !hasMsga(attrs, attr_success))
        return -1;

    e = _vbssSwitchEntryFind(msgaGetBin(&attrs[attr_bssid]));
    if (!e)
        return 0;

    _vbssSwitchEntryPrint(e);

    success = msgaGetU8(&attrs[attr_success]);
    src_agent = alDeviceFindAny(e->src_al_mac);
    dst_agent = alDeviceFindAny(e->dst_al_mac);
    if (!src_agent || !dst_agent) {
        DEBUG_WARNING("cannot find src agent("MACFMT") or dst agent("MACFMT")\n",
            MACARG(e->src_al_mac), MACARG(e->dst_al_mac));
        return -1;
    }

    dst_radio = radioFind(dst_agent, e->dst_ruid);
    if (!dst_radio) {
        DEBUG_WARNING("cannot find dst radio("MACFMT")\n", MACARG(e->dst_ruid));
        return -1;
    }

    /* receive vbss response from dst agent */
    if (!MACCMP(msgaGetBin(&attrs[attr_al_mac]), e->dst_al_mac)) {
        VBSS_STATE_SWITCH(e, VBSS_STATE_CREATION_VAP);
        /* dst agent add vap success then notify src agent adjust channel */
        if (success) {
            VBSS_STATE_SWITCH(e, VBSS_STATE_TRIGGER_CSA);
            sendTriggerCSARequest(idx2InterfaceName(src_agent->recv_intf_idx), getNextMid(), src_agent->al_mac,
                e->src_ruid, e->bssid, e->client_mac, dst_radio->channel, dst_radio->opclass);
        }
        /* switch to dst agent failed then notify src agent cancel vbss */
        else {
            VBSS_STATE_SWITCH(e, VBSS_STATE_CANCEL);
            sendVbssMoveCancelRequest(idx2InterfaceName(src_agent->recv_intf_idx), getNextMid(), src_agent->al_mac,
                e->bssid, e->client_mac);
        }

        _vbssSwitchEntryPrint(e);
        return 0;
    }
    /* receive vbss response from src agent */
    else if (!MACCMP(msgaGetBin(&attrs[attr_al_mac]), e->src_al_mac)) {
        VBSS_STATE_SWITCH(e, VBSS_STATE_DESTRUCTION_VAP);
        _vbssSwitchEntryPrint(e);
        _vbssSwitchEntryDel(e);
        return 0;
    }

    return 0;
}

static int _vbssTriggerCSAResponseEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct vbss_switch_entry *e = NULL;
    struct al_device *src_agent = NULL;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_al_mac) || !hasMsga(attrs, attr_bssid) || !hasMsga(attrs, attr_mac))
        return -1;

    e = _vbssSwitchEntryFind(msgaGetBin(&attrs[attr_bssid]));
    if (!e)
        return 0;

    _vbssSwitchEntryPrint(e);

    src_agent = alDeviceFindAny(e->src_al_mac);
    if (!src_agent) {
        DEBUG_WARNING("cannot find src agent("MACFMT")\n", MACARG(e->src_al_mac));
        return -1;
    }

    sendVBSSDestructionRequest(idx2InterfaceName(src_agent->recv_intf_idx), getNextMid(), src_agent->al_mac,
        e->src_ruid, e->bssid, 1, NULL);

    return 0;
}

static int _vbssStart(void *p, char *cmdline)
{
    /* Controller: vbss process start event */
    featSuscribeEvent(feat_evt_vbss_start, _vbssStartEventHandler, NULL);
    /* Source Agent: receive vbss client security context request */
    featSuscribeEvent(feat_evt_vbss_client_security_context_request, _vbssClientSecurityReqEventHandler, NULL);
    /* Agent: receive vbss creation request */
    featSuscribeEvent(feat_evt_vbss_creation_request, _vbssCreationEventHandler, NULL);
    /* Agent: receive vbss destruction request */
    featSuscribeEvent(feat_evt_vbss_destruction_request, _vbssDestructionEventHandler, NULL);
    /* Controller: receive vbss response*/
    featSuscribeEvent(feat_evt_vbss_response, _vbssResponseEventHandler, NULL);
    /* Controller: receive trigger CSA response*/
    featSuscribeEvent(feat_evt_vbss_trigger_csa_response, _vbssTriggerCSAResponseEventHandler, NULL);

    return 0;
}

static struct extension_ops _vbss_ops = {
    .init = NULL,
    .start = _vbssStart,
    .stop = NULL,
    .deinit = NULL,
};


void vbssFeatLoad()
{
    registerExtension(&_vbss_ops);
}
#endif
