#include <arpa/inet.h>
#include "al_action.h"
#include "al_send.h"
#include "platform_os.h"
#include "wifi.h"
#include "al_driver.h"
#include "feature/feature_helper.h"
#include "feature/ubus/ubus_helper.h"
#include "feature/steering/record.h"


// FIXME. most of above functions are only for record.
// the interface function need be carefully designed.

#if 0

uint8_t _do_opportunity_steer(struct btm_target_item *item_target, dlist_head *list_candidate,
                                    uint8_t mode, uint16_t timer_disassoc)
{
    struct mac_item *item_mac;
    uint8_t *raw_frame;
    uint32_t len_frame = 0;

    /* opportunity window is expired or agent sent the steer complete msg, do NOT steer */
    if (!local_device->btm_exclusive_timer)
        return 0;

    /* target sta in in local disallowed list, do NOT steer */
    dlist_for_each(item_mac, local_device->local_disallow_list, l) {
        if (!MACCMP(item_mac, item_target->sta_info->mac))
            return 0;
    }
    if (false == _sta_need_steer(item_target->sta_info->mac)) {
        DEBUG_WARNING("sta should NOT be steered\n");
        return 0;
    }
    raw_frame = calloc(1, MAX_LEN_MMPDU);
    if (!raw_frame) {
        DEBUG_WARNING("NO space for BTM request --- opportunity\n");
        return 0;
    }
    len_frame = _build_BTM_request_frame(raw_frame, item_target->sta_info, timer_disassoc,
                                            BIT_IS_SET(mode, STEER_REQ_MODE_DISASSOC_IMM_SHIFT),
                                            BIT_IS_SET(mode, STEER_REQ_MODE_ABRIDGED_SHIFT), list_candidate);
    bssSendMgmtFrame(item_target->sta_info->wif->i.index, raw_frame, len_frame);
    free(raw_frame);
    return 1;
}




uint8_t *_find_suitable_candidate(void)
{
    /* To do, find the mac of the most suitable candidate */
    return NULL;
}



void _do_mandantory_steer(struct btm_target_item *item_target, dlist_head *list_candidate,
                                    uint8_t mode, uint16_t timer_disassoc)
{
    uint8_t *raw_frame = calloc(1, MAX_LEN_MMPDU);
    uint32_t len_frame = 0;

    if (!raw_frame) {
        DEBUG_WARNING("NO space for beacon request\n");
        return;
    }
    len_frame = _build_BTM_request_frame(raw_frame, item_target->sta_info, timer_disassoc,
                                            BIT_IS_SET(mode, STEER_REQ_MODE_DISASSOC_IMM_SHIFT),
                                            BIT_IS_SET(mode, STEER_REQ_MODE_ABRIDGED_SHIFT), list_candidate);
    bssSendMgmtFrame(item_target->sta_info->wif->i.index, raw_frame, len_frame);
    free(raw_frame);
}

dlist_head * _get_candidate_list(dlist_head *list_req)
{
    /* To do, find the real candidate bssid list, it may not equal with the bssid list in BTM req */
    /* the wildcard mac will be discard, it will not appear in actual list for opportunity steering */
    /* return the actual candidate we will apply to this sta */
    /* will return the list_req(target bssid in BTM req) for now */
    return list_req;
}

bool _sta_need_steer(uint8_t *mac)
{
    /* To do, check the target sta whether need to be steer */
    /* will always return true for now */
    return true;
}

uint8_t decide_measurement_mode(struct client* sta)
{
    if (sta->cap.support_active_radio_measure)
        return MEASURE_MODE_ACTIVE;
    if (sta->cap.support_passive_radio_measure)
        return MEASURE_MODE_PASSIVE;
    if (sta->cap.support_beacon_table_measure)
        return MEASURE_MODE_BEACON_TABLE;
    return MEASURE_MODE_BEACON_TABLE;
}
#endif
int _buildDeauthFrame(uint8_t *frame, struct client *sta, uint16_t reason_code)
{
    uint8_t *p;
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)frame;

    if (!mgmt) {
        DEBUG_WARNING("frame is NULL\n");
        return 0;
    }
    FILL_80211_HEADER(mgmt, IEEE80211_FC0(IEEE80211_FC0_TYPE_MGT, IEEE80211_FC0_SUBTYPE_DEAUTH),
                        sta->mac, sta->wif->i.mac, sta->wif->i.mac);
    mgmt->u.deauth.reason_code =reason_code;
    p = mgmt->u.deauth.variable;
    return p - frame;
}

int doSteerByDeauth(struct client *sta, uint16_t reason_code)
{
#if 0
    uint8_t *frame;
    int len = 0;
    int ret = -1;
    frame = calloc(1, MAX_LEN_MMPDU);
    if (!frame)
        return ret;
    len = _buildDeauthFrame(frame, sta, reason_code);
    if (len)
        bssSendMgmtFrame(sta->wif->i.index, frame, len);
    free(frame);
#else

    if (!sta->wif)
        return -1;

    bssDeauth(sta->wif, sta, reason_code);
    sendTopologyNotification(getNextMid(), sta, 0);
    clientDelete(sta);
#endif
    return 0;
}

int _buildBTMReqFrame(uint8_t *frame, struct client *sta, uint8_t disassoc_imm, uint8_t abridged,
                        uint16_t disassoc_timer, uint8_t *target, uint8_t opclass, uint8_t channel)
{
    uint8_t *p;
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)frame;

    if (!mgmt) {
        DEBUG_WARNING("frame is NULL\n");
        return 0;
    }
    FILL_80211_HEADER(mgmt, IEEE80211_FC0(IEEE80211_FC0_TYPE_MGT, IEEE80211_FC0_SUBTYPE_ACTION),
                        sta->mac, sta->wif->i.mac, sta->wif->i.mac);
    FILL_ACTION_HEADER(mgmt, WLAN_CATEGORY_WNM, WLAN_ACTION_BTM_REQ);

    mgmt->u.action.u.btm_req.token = sta->btm_ctx.token;
    mgmt->u.action.u.btm_req.disassoc_timer = htons(disassoc_timer);
    if (disassoc_imm)
        mgmt->u.action.u.btm_req.req_mode |= BIT(MODE_DISASSOC_IMM_SHIFT);
    if (target)
        mgmt->u.action.u.btm_req.req_mode |= BIT(MODE_CANDI_INCLUDE_SHIFT);
    if (abridged)
        mgmt->u.action.u.btm_req.req_mode |= BIT(MODE_ABRIDGED_SHIFT);

    p = mgmt->u.action.u.btm_req.variable;

    if (target) {
        uint8_t subelement[3] = {SUBELEMENT_ID_BTM_CANDI_PREF, 1, 255};
        uint32_t bss_info = 0;
        uint8_t phy_type = 0;
        p = addNeighborReport(p, target, opclass, channel, bss_info, phy_type, subelement, 3);
    }

    return p - frame;
}

int doSteerByBTM(struct client *sta, uint8_t *target, uint8_t opclass, uint8_t channel,
                        uint8_t disassoc_imm, uint8_t abridged, uint16_t disassoc_timer)
{
    int ret = -1;
    uint8_t *frame;
    int len;

    //TODO: check conditions and prepare context
    sta->btm_ctx.token = getNextToken(sta->token);
    if (target)
        MACCPY(sta->btm_ctx.target, target);
    else
        memset(sta->btm_ctx.target, 0xff, MACLEN);

    frame = calloc(1, MAX_LEN_MMPDU);
    if (!frame)
        goto fail;

    len = _buildBTMReqFrame(frame, sta, disassoc_imm, abridged, disassoc_timer,
                            target, opclass, channel);

    if (len)
        bssSendMgmtFrame(sta->wif->i.index, frame, len);

    sta->btm_ctx.send_btm_ts = PLATFORM_GET_TIMESTAMP(0);

    free(frame);
    return 0;
fail:
    return ret;
}

int doMandantorySteer(struct client *sta, uint8_t *target, uint8_t opclass, uint8_t channel,
                        uint8_t param, uint16_t disassoc_timer, uint8_t reason)
{
    /* 1. if sta supported btm */
    if (!checkBtmSupported(sta->ies.extcap)) {
        DEBUG_INFO("STA("MACFMT") does not support BTM. send deauthentication frame to it.\n",
            MACARG(sta->mac));
        doSteerByDeauth(sta, WLAN_REASON_DISASSOC_QAP_NO_BANDWIDTH);
        return 0;
    }

    /* 2. if sta in BTM Disallowed sta list */
    if (staInBTMDisallowedList(sta->mac)) {
        DEBUG_INFO("STA("MACFMT") in BTM Disallowed sta list no need to be RCPI steered\n",
            MACARG(sta->mac));
        return 0;
    }

    return doSteerByBTM(sta, target, opclass, channel, BIT_IS_SET(param, STEER_REQ_MODE_DISASSOC_IMM_SHIFT),
                            BIT_IS_SET(param, STEER_REQ_MODE_ABRIDGED_SHIFT), disassoc_timer);
}

void _steerComplete(void *data)
{
    if (registrar)
        sendClientSteeringComplete(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac);
}

int doOpportunitySteer(struct client *sta, uint8_t param, uint16_t disassoc_timer, uint16_t window)
{
    /* just send steering completed msg for Testplan */
    /* FIXME if opportunity steer is real needed. */
    uint32_t sec_countdown = 5; /* agent will send steering completed msg 5 sec later after steering req received */

    if (window < sec_countdown)
        sec_countdown = window;
     platformAddTimer(sec_countdown*1000, 0, _steerComplete, NULL);
    return 0;
}

int doStaDenyAdd(struct wifi_interface *wif, uint8_t *sta_mac)
{
    if (!wif)
        return -1;
    return staDeny(wif, sta_mac, 1);
}

int doStaDenyDel(struct wifi_interface *wif, uint8_t *sta_mac)
{
    if (!wif)
        return -1;
    return staDeny(wif, sta_mac, 0);
}

int doClientAssociationControl(struct wifi_interface *wif, struct client *sta, uint8_t mode, uint16_t time)
{
    if (!wif || !sta)
        return -1;

    if (mode == ASSOC_CTRL_TIMED_BLOCK) {
        struct deny_sta_info *param = (struct deny_sta_info *)calloc(1, sizeof(struct deny_sta_info));
        if (!param)
            return -1;
        MACCPY(param->bssid, wif->i.mac);
        MACCPY(param->mac, sta->mac);
        doStaDenyAdd(wif, sta->mac);
        sendTopologyNotification(getNextMid(), sta, 0);
        clientDelete(sta);
        platformAddTimer(g_steer.deny_acl_period*1000, 0, denyStaTimerHandle, param);
        DEBUG_WARNING("add sta["MACFMT"] to deny acl list\n", MACARG(sta->mac));
    }

    return 0;
}

int doMetricsReport(struct al_device *d)
{
    if (!d)
        return -1;
    if (d->metrics_rpt_timer) {
        platformCancelTimer(d->metrics_rpt_timer);
        d->metrics_rpt_timer = NULL;
    }

    if (d->metrics_rpt_interval) {
        d->metrics_rpt_timer =
            platformAddTimer(d->metrics_rpt_interval*1000, TIMER_FLAG_PERIODIC, metricsReportTimerHandler, d);
    }

    return 0;
}

int _buildBeaconReqFrame(uint8_t *frame, struct client *sta, struct bcnMetricQueryTLV *qry)
{
    uint8_t *p;
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)frame;
    struct chanRptStruct *chanrpt_s;
    uint8_t mode = 0;

    if (!mgmt) {
        DEBUG_WARNING("frame is NULL\n");
        return 0;
    }
    FILL_80211_HEADER(mgmt, IEEE80211_FC0(IEEE80211_FC0_TYPE_MGT, IEEE80211_FC0_SUBTYPE_ACTION),
                        sta->mac, sta->wif->i.mac, sta->wif->i.mac);
    FILL_ACTION_HEADER(mgmt, WLAN_CATEGORY_RADIO_MEASUREMENT, WLAN_ACTION_RADIO_MEASURE_REQ);

    mgmt->u.action.u.radio_meas_req.token = sta->beacon_ctx.token;
    mgmt->u.action.u.radio_meas_req.repetition = 0;

    p = mgmt->u.action.u.radio_meas_req.variable;

    if (qry->channel!=255) {
        p = addBeaconRequest(p, sta->beacon_ctx.token, mode, qry->opclass, 1, &qry->channel, qry->bssid,
                                &qry->ssid, qry->detail, &qry->eids);
    } else {
        dlist_for_each(chanrpt_s, qry->tlv.s.t.childs[0], s.t.l) {
            p = addBeaconRequest(p, sta->beacon_ctx.token, mode, chanrpt_s->chanreport.datap[0],
                                    chanrpt_s->chanreport.len-1, &chanrpt_s->chanreport.datap[1],
                                    qry->bssid, &qry->ssid, qry->detail, &qry->eids);
        }
    }
    return p-frame;
}

int doBeaconRequest(struct client *sta, struct bcnMetricQueryTLV *qry)
{
    int ret = -1;
    uint8_t *frame;
    int len;

    sta->beacon_ctx.token = getNextToken(sta->token);

    frame = calloc(1, MAX_LEN_MMPDU);
    if (!frame)
        goto fail;

    len = _buildBeaconReqFrame(frame, sta, qry);

    if (len)
        bssSendMgmtFrame(sta->wif->i.index, frame, len);

    free(frame);
    return 0;
fail:
    return ret;
}

int cbBTMResponse(struct frame_match_desc *desc, uint8_t *p, uint16_t len)
{
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)p;
    struct wifi_interface *wif;
    uint8_t *sta_mac = mgmt->sa;
    struct client *c;
    uint8_t *target = NULL;
    uint8_t status_code;

    if (!(wif = (struct wifi_interface *)interfaceFind(local_device, mgmt->bssid, interface_type_wifi)))
        return -1;

    if (!(c = clientFind(local_device, wif, sta_mac))) {
        return -1;
    }

    if (c->btm_ctx.token != mgmt->u.action.u.btm_resp.token)
        return 0;

    c->btm_ctx.token = 0;
    status_code = mgmt->u.action.u.btm_resp.status;
    if (status_code == BTM_STATUS_CODE_SUCCESS) {
        target = mgmt->u.action.u.btm_resp.variable;
        DEBUG_INFO("BTM Resp: sta["MACFMT"] ["MACFMT"]=>["MACFMT"]\n", MACARG(sta_mac), MACARG(mgmt->bssid),
                    MACARG(target));
    }

    sendBtmResponseEvent(local_device->al_mac, c->mac, c->wif->bssInfo.bssid, status_code, target);

    if ((!isRegistrar()) && (registrar))
        sendClientSteeringBTMReport(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac,
                                     wif->i.mac, sta_mac, status_code, target);
    return 0;
}

int cbBeaconReport(struct frame_match_desc *desc, uint8_t *p, uint16_t len)
{
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)p;
    struct wifi_interface *wif;
    uint8_t *sta_mac = mgmt->sa;
    struct client *c;
    uint8_t *report = NULL;

    if (!(wif = (struct wifi_interface *)interfaceFind(local_device, mgmt->bssid, interface_type_wifi)))
        return -1;

    if (!(c = clientFind(local_device, wif, sta_mac))) {
        return -1;
    }

    if (c->beacon_ctx.token != mgmt->u.action.u.radio_meas_report.token) {
        DEBUG_WARNING("Beacon Report: token not match\n");
    }
    c->beacon_ctx.token = 0;

    report = mgmt->u.action.u.radio_meas_report.variable;
    len -= (report-p);

    if ((!isRegistrar()) && (registrar))
        sendBeaconMetricsResponse(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac,
                                    sta_mac, report, len);
    return 0;
}

#if 0
int cbDisassociateSta(struct frame_match_desc *desc, uint8_t *p, uint16_t len)
{
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)p;
    struct wifi_interface *wif;
    uint8_t *sta_mac = mgmt->sa;
    struct client *c;

    DEBUG_INFO("Receive Disassociate request from sta["MACFMT"]\n", MACARG(sta_mac));

    if (!(wif = (struct wifi_interface *)interfaceFind(local_device, mgmt->bssid, interface_type_wifi)))
        return -1;

    if (!(c = clientFind(local_device, wif, sta_mac))) {
        DEBUG_WARNING("sta["MACFMT"] does not exist!\n", MACARG(sta_mac));
        return -1;
    }

    clientDelete(c);

    return 0;
}
#endif

void startChannelScan(struct chscan_req *req)
{
    struct radio *r = NULL;
    struct wifi_interface *wintf = NULL;
    struct chscan_req_item *first = NULL;

    if (!req || !req->r)
        goto fail;

    r = req->r;

    /* get first bss on this radio to trigger scan */
    if (r->configured_bsses.len < 1) {
        DEBUG_WARNING("cannot trigger channel scan for no bss on radio(%s)\n", r->name);
        goto fail;
    }

    wintf = (struct wifi_interface *)r->configured_bsses.p[0];
    if (!wintf)
        goto fail;

    DEBUG_INFO("start channel scan on radio(%s)\n", r->name);
    first = container_of(dlist_get_first(&req->h), struct chscan_req_item, l);
    if (first) {
        bssTriggerChannelScan(wintf->i.index, first->opclass, first->chans, first->ch_num);
        first->status = CHANNEL_SCAN_STATUS_TRIGGERRED;
        /* start chan scan 10s timer for one req item*/
        PLATFORM_CANCEL_TIMER(req->timer);
        req->timer = platformAddTimer(10*1000, 0, chanScanTimerHandle, first);
    }

    return;

fail:
    chscanReqDelete(req);
    if (r)
        r->chscan_req = NULL;

    return;
}

void chanScanTimerHandle(void *data)
{
    struct chscan_req_item *item = (struct chscan_req_item *)data;
    struct chscan_req_item *next;
    struct radio *r;
    struct chscan_req *req;
    struct wifi_interface *wintf;
    uint32_t age;

    if (!item)
        return;

    req = item->req;
    r = (req == NULL ? NULL : req->r);

    if (!req || !r)
        return;

    /* get first bss on this radio to trigger scan */
    if (r->configured_bsses.len < 1) {
        DEBUG_WARNING("cannot trigger channel scan for no bss on radio(%s)\n", r->name);
        goto scan_done;
    }

    wintf = (struct wifi_interface *)r->configured_bsses.p[0];
    if (!wintf)
        goto scan_done;

    item->status = CHANNEL_SCAN_STATUS_ABORTED;

    next = container_of(dlist_get_next(&req->h, &item->l), struct chscan_req_item, l);
    /* judge if all items finished. if yes send scan report, if no scan next item */
    if (next) {
        DEBUG_INFO("trigger next item channel scan on radio(%s)\n", r->name);
        bssTriggerChannelScan(wintf->i.index, next->opclass, next->chans, next->ch_num);
        next->status = CHANNEL_SCAN_STATUS_TRIGGERRED;
        PLATFORM_CANCEL_TIMER(req->timer);
        req->timer = platformAddTimer(10*1000, 0, chanScanTimerHandle, req);
    }
    else {
        goto scan_done;
    }

    /* judge if this req does not finished more than 5minutes. */
    age = PLATFORM_GET_TIMESTAMP(0) - req->ts;
    if (age >= 5*60*1000)
        goto scan_done;

    return;

scan_done:
    reportChanScanResult(req);
    r->chscan_req = NULL;

    if (r->chscan_req_pend) {
        r->chscan_req = r->chscan_req_pend;
        r->chscan_req_pend = NULL;
        startChannelScan(r->chscan_req);
    }

    return;
}


void metricsReportTimerHandler(void *data)
{
    struct al_device *d = (struct al_device *)data;

    if ((!d) || (!registrar))
        return;

    sendApMetricsResponse(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac, NULL, 0);
}

int doChannelSelection(struct radio *r)
{
    int ret = 0;
    uint8_t opclass = r->opclass, channel = r->channel;

    if (((r->change_channel) && (selectBestChannel(r, &opclass, &channel)>0)) ||
        (r->change_power)) {
        DEBUG_INFO("do ch-sel: opc=%d, ch=%d, txpwr=%d\n", opclass, channel, r->tx_power);
        ret = radioSetChannelTxPower(r, opclass, channel, r->tx_power);
    }
    return ret;
}

int doVipAction(void)
{
    if (isRegistrar())
        SET_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_STA_SHIFT);
    return VipStaAction();
}

void _vipConfChangedTriggerMsg(void)
{
    struct al_device *dev;

    dlist_for_each(dev, local_network, l) {
        if (dev->is_controller) /* jump the controller itself */
            continue;
        if (dev->is_agent)
            sendMapPolicyConfigRequest(idx2InterfaceName(dev->recv_intf_idx), getNextMid(), dev->al_mac, &local_policy);
    }
}

int doMappingConfAction(struct DSCP_mapping_conf *conf)
{
    if (isRegistrar())
        SET_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_MAPPING_SHIFT);
    return mappingConfAction(conf);
}

void syncMappingConf(void)
{
    _vipConfChangedTriggerMsg();
    CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_MAPPING_SHIFT);
}

int doQueueConfAction(struct dlist_head *conf)
{
    if (isRegistrar())
        SET_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_QUEUE_SHIFT);
    return queueConfAction(conf);
}

void syncQueueConf(void)
{
    _vipConfChangedTriggerMsg();
    CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_QUEUE_SHIFT);
}


int doTcConfAction(struct tc_mapping_conf *conf)
{
    if (isRegistrar())
        SET_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_TC_SHIFT);
    return tcConfAction(conf);
}

void syncTcConf(void)
{
    _vipConfChangedTriggerMsg();
    CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_TC_SHIFT);
}

void syncVIPSta(void)
{
    _vipConfChangedTriggerMsg();
    CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_STA_SHIFT);
}

static void _autoRoleHandle(void *data)
{
    local_device->autorole_timer = NULL;

    if (registrar)
        return;

    /* if no controller check if local device can be controller*/
    if (local_config.auto_role) {
        setRegistrar(local_device);
        local_device->configured = 1;
        local_device->is_controller = 1;
        sendAPAutoconfigurationRenew(getNextMid(), IEEE80211_FREQUENCY_BAND_2_4_GHZ, NULL);
        /* send ubus event */
        struct mesh_ubus_event_request req;
        req.u.role_change.old_role = 0; // auto
        req.u.role_change.new_role = 1; // controller
        sendUbusEvent(EVENT_ROLE_CHANGE, &req);
        DEBUG_INFO("change to Controller. auto role done! \n");
    }
}

void doAutoRole(void)
{
    PLATFORM_CANCEL_TIMER(local_device->autorole_timer);
    local_device->autorole_timer = platformAddTimer(PLATFORM_RANDOM(20, 40) * 1000, 0, _autoRoleHandle, NULL);
}

