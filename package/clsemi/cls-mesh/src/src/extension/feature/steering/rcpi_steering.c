#include "platform.h"
#include "extension.h"
#include "feature/feature.h"
#include "feature/feature_helper.h"
#include "driver/cls/nl80211_cls.h"
#include "datamodel.h"
#include "platform_os.h"
#include "al_send.h"
#include "al_utils.h"
#include "al_msg.h"
#include "al_action.h"
#include "al_driver.h"
#include "wifi.h"
#include "record.h"


struct global_steer_info g_steer;

struct candidate_bss {
    struct wifi_interface *bss;
    uint8_t ul_rcpi;
    int score;
};

struct nac_chan_param {
    struct client *sta;
    uint8_t channel;
    uint8_t mac[MACLEN];
};


int _calculateCandidateScore(struct wifi_interface *bss, uint8_t rcpi_ul, struct client *c)
{
    int score = 0;

    if (rcpi_ul == 0 || rcpi_ul >= 218) {
        DEBUG_WARNING("illegal rcpi_ul(%u)\n", rcpi_ul);
        return 0;
    }
    /* rcpi diff value * 3 */
    score += (rcpi_ul - c->link_metrics.rcpi_ul) * 3;
    /* asso client num diff value * 2 */
    score += (dlist_count(&c->wif->clients) - dlist_count(&bss->clients)) * 2;

    DEBUG_INFO("candidate info[bss: "MACFMT", ul_rcpi: %u, score: %d]\n", MACARG(bss->i.mac), rcpi_ul, score);

    return score;
}

void _localGetNacInfoTimerHandler(void *data)
{
    struct nac_chan_param *param = (struct nac_chan_param *)data;

    if (!param)
        return;

    if (!param->sta)
        goto bail;

    param->sta->unasso_link_metrics.rcpi_ul = nl80211GetNacStaInfo(param->channel, param->mac);
    DEBUG_INFO("Local get nac info rcpi: %u\n", param->sta->unasso_link_metrics.rcpi_ul);

bail:
    free(param);
    return;
}

void _localCollectNacInfo(struct client *c, uint8_t channel)
{
    struct nac_chan_param *nac_param = NULL;

    if (!c)
        return;

    nac_param = (struct nac_chan_param *)calloc(1, sizeof(struct nac_chan_param));
    if (!nac_param)
        return;

    nac_param->sta = c;
    nac_param->channel = channel;
    MACCPY(nac_param->mac, c->mac);
    addNacMonitorSta(channel, c->mac);
    setNacEnable(channel);
    platformAddTimer(1500, 0, _localGetNacInfoTimerHandler, nac_param);

    return;
}

void _staWaitBtmRspTimeHandle(void *data)
{
    struct client *sta = (struct client *)data;
    struct steer_client *steer_c;
    struct steer_record *record;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    if (!sta || !sta->wif || (sta->steer_state != CLIENT_STEER_STATUS_WAIT_BTM))
        return;

    DEBUG_INFO("wait sta["MACFMT"] btm response timeout update steer_state!\n", MACARG(sta->mac));
    client_steer_status_update(sta, CLIENT_STEER_STATUS_FINISHED);

    steer_c = steerClientFind(sta->mac);
    record = steerRecordFindLatest(steer_c);
    if (record && ((current_ts - record->create_ts) >= local_policy.roaming_policy.cooldown*1000)) {
        record = NULL;
    }

    if (record) {
        if (IS_ZERO_MAC(record->final_bss))
            MACCPY(record->final_bss, sta->wif->i.mac);

        if (!MACCMP(record->target_bss, record->final_bss)) {
            record->status = 1;
            record->finish_ts = current_ts;
        }
        STEER_RECORD_PRINT(record);
    }

    return;
}

int _addToDenyAclList(struct client *c, struct wifi_interface *wif)
{
    struct al_device *d = NULL;

    if (!c || !wif)
        return 0;

    d = wif->radio->d;
    // local device
    if (d == local_device) {
        struct deny_sta_info *param = calloc(1, sizeof(struct deny_sta_info));
        if (!param)
            return 0;
        MACCPY(param->bssid, wif->i.mac);
        MACCPY(param->mac, c->mac);
        DEBUG_WARNING("add sta["MACFMT"] to deny acl list\n", MACARG(c->mac));
        doStaDenyAdd(wif, c->mac);
        sendTopologyNotification(getNextMid(), c, 0);
        clientDelete(c);
        platformAddTimer(g_steer.deny_acl_period*1000, 0, denyStaTimerHandle, param);
    } else {
        DEBUG_WARNING("sendSingleClientAssocControl to add sta["MACFMT"] to deny acl list\n", MACARG(c->mac));
        sendSingleClientAssocControl(d, wif->i.mac, c->mac, g_steer.deny_acl_period, 0x02);
    }

    return 1;
}

struct radio * _findVbssBestRadio(struct client *sta)
{
    struct radio *r = NULL;
    struct radio *vbss_best_radio = NULL;
    uint8_t vbss_best_rcpi_ul = 0;
    struct sta_seen *seen = NULL;
    int band_idx, sta_band_idx;

    if (!sta || !sta->wif || !sta->wif->radio || !sta->wif->radio->d)
        return NULL;

    dlist_for_each(seen, sta->seens, l) {
        DEBUG_INFO("sta seen: [rid: ("MACFMT"), opclass: %u, channel: %u, rcpi_ul: %u])\n",
                    MACARG(seen->rid), seen->opclass, seen->channel, seen->rcpi_ul);

        r = radioFind(NULL, seen->rid);
        if (!r) {
            DEBUG_WARNING("can not find radio("MACFMT")\n", MACARG(seen->rid));
            continue;
        }

        /* only check the radio which enable vbss */
        if (r->vbss_capa.max_vbss > 0) {
            /* should find vbss from other devices.*/
            if (r->d == sta->wif->radio->d)
                continue;

            sta_band_idx = nlbw2Idx(opclass2nlBandwidth(sta->wif->radio->opclass, NULL));
            band_idx = nlbw2Idx(opclass2nlBandwidth(r->opclass, NULL));

            /* only support same frequency now */
            if (sta->wif->radio->current_channel[sta_band_idx] != r->current_channel[band_idx]) {
                DEBUG_INFO("radio("MACFMT") sta->wif->radio->current_channel[%u](%u) != r->current_channel[%u](%u), ignore\n",
                    MACARG(seen->rid), sta_band_idx, sta->wif->radio->current_channel[sta_band_idx],
                    band_idx, r->current_channel[band_idx]);
                continue;
            }

            if (r->current_vbss_num >= r->vbss_capa.max_vbss) {
                DEBUG_WARNING("radio("MACFMT") current vbss num(%u) >= max_vbss(%u)\n",
                    MACARG(seen->rid), r->current_vbss_num, r->max_bss);
                continue;
            }

            if (seen->rcpi_ul < sta->link_metrics.rcpi_ul)
                continue;

            if (seen->rcpi_ul > vbss_best_rcpi_ul) {
                vbss_best_rcpi_ul = seen->rcpi_ul;
                vbss_best_radio = r;
            }
        }
    }

    return vbss_best_radio;
}

void _staWaitSeenTimerHandle(void *data)
{
    struct client *sta = (struct client *)data;
    struct al_device *d = NULL;
    struct sta_seen *seen = NULL;
    struct radio *r = NULL;
    struct wifi_interface *bss = NULL;
    struct radio *vbss_best_radio = NULL;
    struct candidate_bss best = {0};
    int score = 0;
    int band_idx;
    int i;

    if (!sta || !sta->wif || !sta->wif->radio)
        return;

    sta->wait_seen_timer = NULL;

    d = alDeviceFindBySta(sta);
    if (!d)
        return;

    /* if sta is vbss sta, do vbss process */
    if (sta->wif->is_vbss) {
        vbss_best_radio = _findVbssBestRadio(sta);

        /* start vbss switch */
        if (vbss_best_radio) {
            DEBUG_INFO("find best vbss radio("MACFMT") for steering vbss sta("MACFMT") send vbss start event!\n",
                    MACARG(vbss_best_radio->uid), MACARG(sta->mac));

            sendVbssStartEvent(sta->mac, d->al_mac, vbss_best_radio->uid);
        }
        else {
            DEBUG_INFO("can't find best vbss radio for steering vbss sta("MACFMT")!\n", MACARG(sta->mac));
        }
        /* doesn't support both vbss sta and normal sta on same radio now */
        goto bail;
    }

    dlist_for_each(seen, sta->seens, l) {
        r = radioFind(NULL, seen->rid);
        if (!r) {
            DEBUG_WARNING("can't find radio("MACFMT")\n", MACARG(seen->rid));
            continue;
        }

        if (seen->rcpi_ul - sta->link_metrics.rcpi_ul < local_policy.roaming_policy.rssi_gain_thresh) {
            DEBUG_INFO("seen->rcpi_ul(%u) - sta->link_metrics.rcpi_ul(%u) < rssi_gain_thresh(%u) ignore it\n",
                seen->rcpi_ul, sta->link_metrics.rcpi_ul, local_policy.roaming_policy.rssi_gain_thresh);
            continue;
        }

        for (i = 0; i < r->configured_bsses.len; i++) {
            if ((bss = r->configured_bsses.p[i])) {
                /* check ssid/auth/encrypt/key target ssid must be same with the pre one*/
                if (!BSS_IS_SAME(sta->wif, bss)) {
                    DEBUG_INFO("staWaitSeenTimerHandle ssid not matched\n");
                    continue;
                }
                /* ignore the bss that the station assocaited now */
                if (!MACCMP(sta->wif->i.mac, bss->i.mac))
                    continue;

                score = _calculateCandidateScore(bss, seen->rcpi_ul, sta);
                if (score <= 0)
                    continue;
                if (!best.bss || best.score < score) {
                    best.bss = bss;
                    best.ul_rcpi = seen->rcpi_ul;
                    best.score = score;
                }
            }
        }
    }

    /* if controller and sta is remote, then calcute if controller has candidate bss */
    struct interface *intf = NULL;
    if (isRegistrar() && d != local_device) {
        dlist_for_each(intf, local_device->interfaces, l) {
            if (intf->type != interface_type_wifi)
                continue;
            bss = (struct wifi_interface *)intf;
            if (!BSS_IS_SAME(sta->wif, bss))
                continue;

            if (sta->unasso_link_metrics.rcpi_ul - sta->link_metrics.rcpi_ul < local_policy.roaming_policy.rssi_gain_thresh) {
                DEBUG_INFO("sta->unasso_link_metrics.rcpi_ul(%u) - sta->link_metrics.rcpi_ul(%u) < rssi_gain_thresh(%u) ignore it\n",
                    sta->unasso_link_metrics.rcpi_ul, sta->link_metrics.rcpi_ul, local_policy.roaming_policy.rssi_gain_thresh);
                continue;
            }

            score = _calculateCandidateScore(bss, sta->unasso_link_metrics.rcpi_ul, sta);
            if (score <= 0)
                continue;
            if (!best.bss || best.score < score) {
                best.bss = bss;
                best.ul_rcpi = sta->unasso_link_metrics.rcpi_ul;
                best.score = score;
            }
            break;
        }
    }

    if (!best.bss) {
        client_steer_status_update(sta, CLIENT_STEER_STATUS_FINISHED);
        DEBUG_WARNING("can't find any candidates from seens for steering sta("MACFMT")\n",
                MACARG(sta->mac));
        goto bail;
    }

    /* if local sta send btm request frame*/
    if (d && d == local_device) {
        /* if sta does not supported btm send deauth frame */
        if (!checkBtmSupported(sta->ies.extcap)) {
            DEBUG_INFO("STA("MACFMT") does not support BTM. send deauthentication frame to it.\n",
                MACARG(sta->mac));
            steerRecordAdd(STEER_TYPE_RCPI, STEER_METHOD_DEAUTH,
                sta->mac, sta->wif->i.mac, best.bss->i.mac, sta->link_metrics.rcpi_ul, best.ul_rcpi);
            doSteerByDeauth(sta, WLAN_REASON_DISASSOC_QAP_NO_BANDWIDTH);
            client_steer_status_update(sta, CLIENT_STEER_STATUS_FINISHED);
            goto bail;
        }

        band_idx = nlbw2Idx(opclass2nlBandwidth(sta->wif->radio->opclass, NULL));
        /* TODO: disassoc imm set to 1 and disassoc_timer need to be confirmed */
        int ret = doSteerByBTM(sta, best.bss->i.mac, sta->wif->radio->current_opclass[band_idx],
            sta->wif->radio->current_channel[band_idx], 1, 0, 0);
        DEBUG_INFO("RCPI-based steering sta("MACFMT") to target BSS("MACFMT") return %d\n",
            MACARG(sta->mac), MACARG(best.bss->i.mac), ret);
        if (ret < 0)
            client_steer_status_update(sta, CLIENT_STEER_STATUS_FINISHED);
        else {
            steerRecordAdd(STEER_TYPE_RCPI, STEER_METHOD_BTM,
                sta->mac, sta->wif->i.mac, best.bss->i.mac, sta->link_metrics.rcpi_ul, best.ul_rcpi);
            client_steer_status_update(sta, CLIENT_STEER_STATUS_WAIT_BTM);
        }
    }
    /* if remote sta, then send client steer request msg */
    else {
        DEFINE_DLIST_HEAD(target_head);
        DEFINE_DLIST_HEAD(sta_head);
        struct mac_item *target_i = (struct mac_item *)calloc(1, sizeof(struct mac_item));
        struct mac_item *sta_i = (struct mac_item *)calloc(1, sizeof(struct mac_item));
        if (target_i && sta_i) {
            MACCPY(target_i->mac, best.bss->i.mac);
            MACCPY(sta_i->mac, sta->mac);
            dlist_add_head(&target_head, &target_i->l);
            dlist_add_head(&sta_head, &sta_i->l);
            sendClientSteeringRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac,
                &target_head, &sta_head, 1, 30, 1, 0, 0, sta->wif->i.mac);
            DEBUG_INFO("sendClientSteeringRequest for STA("MACFMT") try to target bss("MACFMT")\n",
                MACARG(sta->mac), MACARG(best.bss->bssInfo.bssid));
            steerRecordAdd(STEER_TYPE_RCPI, STEER_METHOD_SEND_CLIENT_STEER_REQUEST,
                sta->mac, sta->wif->i.mac, best.bss->i.mac, sta->link_metrics.rcpi_ul, best.ul_rcpi);
            client_steer_status_update(sta, CLIENT_STEER_STATUS_WAIT_BTM);
        }
        free(target_i);
        free(sta_i);
    }

    if (sta->steer_state == CLIENT_STEER_STATUS_WAIT_BTM) {
        sta->btm_ctx.wait_rsp_timer = platformAddTimer(5*1000, 0, _staWaitBtmRspTimeHandle, sta);
    }

bail:
    dlist_free_items(&sta->seens, struct sta_seen, l);

    return;
}

void _staBtmResultCheckTimeHandle(void *data)
{
    struct client *c = (struct client *)data;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);
    struct steer_client *steer_c;
    struct steer_record *record;

    if (!c || !c->wif)
        return;

    steer_c = steerClientFind(c->mac);
    record = steerRecordFindLatest(steer_c);
    if (record && ((current_ts - record->create_ts) >= local_policy.roaming_policy.cooldown*1000)) {
        record = NULL;
    }

    client_steer_status_update(c, CLIENT_STEER_STATUS_FINISHED);

    if (record && MACCMP(c->wif->bssInfo.bssid, record->target_bss) &&
        (c->wif->radio->d == local_device)) {
        DEBUG_WARNING("sta("MACFMT") does not switch to target bss("MACFMT") by BTM so deauth it!\n",
            MACARG(c->mac), MACARG(c->btm_ctx.target));
        record->deauth_ts = current_ts;
        doSteerByDeauth(c, WLAN_REASON_BSS_TRANSITION_DISASSOC);
        STEER_RECORD_PRINT(record);
    }
}


int _doSpecifiedStaRCPIBasedSteering(struct client *sta, bool is_local)
{
    struct radio *r = NULL;
    struct wifi_interface *bss = NULL;
    struct al_device *d = NULL;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);
    struct steer_client *steer_c;
    struct steer_record *record;
    struct unassoc_sta_metrics_query_per_chan_item *chan_item = NULL;
    struct mac_item *mac_item = NULL;
    int band_idx;

    if (!sta || !sta->wif || !sta->wif->radio)
        return -1;

    bss = sta->wif;
    r = bss->radio;

    /* if sta in Local Disallowed sta list */
    if (staInLocalDisallowedList(sta->mac)) {
        DEBUG_INFO("STA("MACFMT") in Local Disallowed sta list no need to be RCPI steered\n",
            MACARG(sta->mac));
        return 0;
    }

    /* if sta in BTM Disallowed sta list */
    if (staInBTMDisallowedList(sta->mac)) {
        DEBUG_INFO("STA("MACFMT") in BTM Disallowed sta list no need to be RCPI steered\n",
            MACARG(sta->mac));
        return 0;
    }

    /* if current opclass or channel is 0. abort steering */
    if (!r->opclass || !r->channel) {
        DEBUG_ERROR("radio current opclass or channel is 0, then abort steering STA("MACFMT").\n",
            MACARG(sta->mac));
        client_steer_status_update(sta, CLIENT_STEER_STATUS_FINISHED);
        return 0;
    }

    steer_c = steerClientFind(sta->mac);
    record = steerRecordFindTypeLatest(steer_c, STEER_TYPE_RCPI);
    if (record) {
        /* if steering success or in steering wait cooldown time */
        if ((IS_ZERO_MAC(record->target_bss) || record->status)) {
            if ((current_ts - record->create_ts <= local_policy.roaming_policy.cooldown*1000)) {
                DEBUG_INFO("client("MACFMT") was rcpi steered less than cooldown ts(%u).\n",
                    MACARG(sta->mac), local_policy.roaming_policy.cooldown);
                return 0;
            }
        }
    }

    band_idx = nlbw2Idx(opclass2nlBandwidth(r->opclass, NULL));

    /* if not local station send unassociatied sta link metric message to all devices */
    DEFINE_DLIST_HEAD(chan_head);
    chan_item = unassocStaChanItemAdd(&chan_head, r->current_channel[band_idx]);
    if (!chan_item)
        return -1;

    mac_item = calloc(1, sizeof(struct mac_item));
    if (!mac_item) {
        dlist_free_items(&chan_head, struct unassoc_sta_metrics_query_per_chan_item, l);
        return -1;
    }

    MACCPY(mac_item->mac, sta->mac);
    dlist_add_tail(&chan_item->sta_list, &mac_item->l);
    dlist_for_each(d, local_network, l) {
        /* do not send to device which sta current connected */
        if ((r->d) && !MACCMP(d->al_mac, r->d->al_mac))
            continue;
        /* local device directly get from driver */
        if (!MACCMP(d->al_mac, local_device->al_mac)) {
            _localCollectNacInfo(sta, r->channel);
            continue;
        }
        sendUnassociatedStaLinkMetricsQuery(idx2InterfaceName(d->recv_intf_idx),
            getNextMid(), d->al_mac, r->current_opclass[band_idx], &chan_head);
        DEBUG_INFO("query STA("MACFMT") link metrics, band_idx: %u, current_opclass: %u\n",
            MACARG(sta->mac), band_idx, r->current_opclass[band_idx]);
    }

    PLATFORM_CANCEL_TIMER(sta->wait_seen_timer);
    sta->wait_seen_timer = platformAddTimer(5*1000, 0, _staWaitSeenTimerHandle, sta);
    client_steer_status_update(sta, CLIENT_STEER_STATUS_WAIT_SEEN);

    dlist_free_items(&chan_item->sta_list, struct mac_item, l);
    dlist_free_items(&chan_head, struct unassoc_sta_metrics_query_per_chan_item, l);

    return 0;
}

int _doSpecifiedStaSwitchBand(struct client *sta, enum e_wifi_band_idx band_idx)
{
    struct radio *r = NULL;
    struct wifi_interface *bss = NULL;
    struct wifi_interface *target_bss = NULL;
    struct steer_client *steer_c;
    struct steer_record *record;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    bss = sta->wif;
    if (!bss)
        return -1;

    r = bss->radio;
    if (!r)
        return -1;

    steer_c = steerClientFind(sta->mac);
    record = steerRecordFindTypeLatest(steer_c, STEER_TYPE_BAND);
    if (record) {
        /* if steering success or in steering wait cooldown time */
        if ((IS_ZERO_MAC(record->target_bss) || record->status)) {
            if ((current_ts - record->create_ts <= local_policy.roaming_policy.cooldown*1000)) {
                DEBUG_INFO("client("MACFMT") was band steered less than cooldown ts(%u).\n",
                    MACARG(sta->mac), local_policy.roaming_policy.cooldown);
                return -1;
            }
        }
    }

    if (rcpi2Rssi(sta->link_metrics.rcpi_ul) > local_policy.roaming_policy.rssi_high)
        DEBUG_INFO("STA("MACFMT") rssi(%d) > %d try switch it to a 5G band BSS\n",
            MACARG(sta->mac), rcpi2Rssi(sta->link_metrics.rcpi_ul), local_policy.roaming_policy.rssi_high);
    else
        DEBUG_INFO("STA("MACFMT") rssi(%d) < %d try switch it to a 2G band BSS\n",
            MACARG(sta->mac), rcpi2Rssi(sta->link_metrics.rcpi_ul), local_policy.roaming_policy.rssi_low);

    /* 1. find target bss on other band */
    target_bss = findSpecificBandBss(sta, band_idx);
    if (!target_bss) {
        DEBUG_WARNING("Can not find any BSS on band_idx(%d) for STA("MACFMT") to switch\n",
            band_idx, MACARG(sta->mac));
        client_steer_status_update(sta, CLIENT_STEER_STATUS_FINISHED);
        return 0;
    }

    DEBUG_INFO("Find target bss successfully. STA("MACFMT") switch it to BSS("MACFMT") intf mac("MACFMT") for band steering\n",
            MACARG(sta->mac), MACARG(target_bss->bssInfo.bssid), MACARG(target_bss->i.mac));

    /* 2. if sta supported btm, if sta does not support btm should deauth it ? --TODO */
    if (!checkBtmSupported(sta->ies.extcap)) {
        DEBUG_INFO("STA("MACFMT") does not support BTM. send deauthentication frame to it.\n",
            MACARG(sta->mac));

        steerRecordAdd(STEER_TYPE_BAND, STEER_METHOD_DEAUTH,
            sta->mac, bss->i.mac, target_bss->i.mac, 0, 0);
        return doSteerByDeauth(sta, WLAN_REASON_BSS_TRANSITION_DISASSOC);
    }

    int ret = doSteerByBTM(sta, target_bss->i.mac, target_bss->radio->opclass, target_bss->radio->channel, 1, 0, 0);
    client_steer_status_update(sta, CLIENT_STEER_STATUS_WAIT_BTM);
    DEBUG_INFO("sta("MACFMT") switch to target BSS("MACFMT") send btm request return %d\n",
        MACARG(sta->mac), MACARG(target_bss->i.mac), ret);
    steerRecordAdd(STEER_TYPE_BAND, STEER_METHOD_BTM,
        sta->mac, bss->i.mac, target_bss->i.mac, 0, 0);

    return ret;
}

static int _rcpiCrossDown(struct wifi_interface *wif, struct client *c, uint8_t last_ul_rcpi, uint8_t ul_rcpi)
{
    struct radio *r = NULL;
    uint8_t hysteresis = DEFAULT_RPT_RCPI_HYSTERSIS;

    if (!c || !wif || !wif->radio)
        return 0;

    r = c->wif->radio;

    if (r->metrics_rpt_policy.sta_rcpi_thresh == 0)
        return 0;
    if (r->metrics_rpt_policy.sta_rcpi_margin > 0)
        hysteresis = r->metrics_rpt_policy.sta_rcpi_margin;

    if (!last_ul_rcpi || !ul_rcpi)
        return 0;

    if (CROSSED_DOWN_THRESHOLD_MARGIN(r->metrics_rpt_policy.sta_rcpi_thresh, hysteresis, last_ul_rcpi, ul_rcpi)) {
        return 1;
    }

    return 0;
}

static int _staUpdateEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct client *c = NULL;
    struct wifi_interface *wif = NULL;
    uint8_t rcpi_ul, last_rcpi_ul;
    int rssi = 0;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_mac) ||
        !hasMsga(attrs, attr_if_mac) ||
        !hasMsga(attrs, attr_station_rcpi_ul) ||
        !hasMsga(attrs, attr_station_last_rcpi_ul))
        return -1;

    rcpi_ul = msgaGetU8(&attrs[attr_station_rcpi_ul]);
    last_rcpi_ul = msgaGetU8(&attrs[attr_station_last_rcpi_ul]);
    rssi = rcpi2Rssi(rcpi_ul);

    wif = (struct wifi_interface *)interfaceFind(local_device, msgaGetBin(&attrs[attr_if_mac]), interface_type_wifi);
    if (!wif || !wif->radio)
        return -1;

    c = clientFind(local_device, wif, msgaGetBin(&attrs[attr_mac]));
    if (!c)
        return -1;

    DEBUG_INFO("receive feat_evt_dm_sta_update sta("MACFMT"), last_rcpi_ul: %u, rcpi_ul: %u, rssi: %d, bsta: %u\n",
        MACARG(msgaGetBin(&attrs[attr_mac])), last_rcpi_ul, rcpi_ul, rssi, STA_IS_BACKHAUL(c));

    /* if backhaul sta skip */
    if (STA_IS_BACKHAUL(c))
        return 0;

    /* Controller */
    if (isRegistrar()) {
        if (local_policy.roaming_policy.rcpi_steer) {
            switch (c->steer_state) {
                case CLIENT_STEER_STATUS_INIT:
                case CLIENT_STEER_STATUS_FINISHED:
                    if (_rcpiCrossDown(wif, c, last_rcpi_ul, rcpi_ul))
                        return _doSpecifiedStaRCPIBasedSteering(c, true);
                    break;
                case CLIENT_STEER_STATUS_WAIT_BTM:
                    if (current_ts - c->btm_ctx.send_btm_ts > 6000) {
                        DEBUG_WARNING("client("MACFMT") wait btm response > 6s then deauth it \n", MACARG(c->mac));
                        client_steer_status_update(c, CLIENT_STEER_STATUS_FINISHED);
                        return doSteerByDeauth(c, WLAN_REASON_BSS_TRANSITION_DISASSOC);
                    }
                    return 0;
                case CLIENT_STEER_STATUS_WAIT_SEEN:
                    return 0;
                default:
                    break;
            }
        }
        goto band_steer;
    }
    /* Agent */
    else {
        if (!registrar) {
            goto band_steer;
        }
    }

band_steer:
    if (local_policy.roaming_policy.band_steer) {
        if (c->steer_state != CLIENT_STEER_STATUS_INIT &&
            c->steer_state != CLIENT_STEER_STATUS_FINISHED) {
            DEBUG_INFO("STA("MACFMT") steer_state(%u) is steering now. ignore it\n",
                MACARG(c->mac), c->steer_state);
            goto bail;
        }
        if (rssi > local_policy.roaming_policy.rssi_high &&
            wif->radio->current_band_idx == band_2g_idx)
            return _doSpecifiedStaSwitchBand(c, band_5g_idx);
        if (rssi < local_policy.roaming_policy.rssi_low &&
            wif->radio->current_band_idx == band_5g_idx)
            return _doSpecifiedStaSwitchBand(c, band_2g_idx);
    }

bail:
    return 0;
}

static int _radioUpdateEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attr[1];
    int type;
    struct radio *r = NULL;

    while ((type = msgaParseOne(attr, &p, &len)) != attr_none) {
        switch (type) {
            case attr_radio_mac:
            {
                r = radioFind(NULL, msgaGetBin(attr));
                if (!r) {
                    DEBUG_ERROR("local radio("MACFMT") does not exist.\n", MACARG(msgaGetBin(attr)));
                    return -1;
                }
                break;
            }
            case attr_rcpi_threshhold:
                if (r)
                    r->steer_policy.rcpi_thresh = msgaGetU8(attr);
                break;
            case attr_chan_util_threshhold:
                if (r)
                    r->steer_policy.chan_util = msgaGetU8(attr);
                break;
            case attr_steering_policy_mode:
                if (r)
                    r->steer_policy.agt_steer_mode = msgaGetU8(attr);
                break;
            default:
                break;
        }
    }

    DEBUG_INFO("recv feat_evt_dm_radio_update event, radio("MACFMT"), rcpi_thresh: %u\n",
        MACARG(r->uid), r->steer_policy.rcpi_thresh);

    return 0;
}

static int _staBtmResponseEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct al_device *d = NULL;
    struct client *c = NULL;
    struct steer_client *steer_c = NULL;
    struct steer_record *record = NULL;
    uint8_t status_code;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_al_mac) ||
        !hasMsga(attrs, attr_mac) ||
        !hasMsga(attrs, attr_if_mac) ||
        !hasMsga(attrs, attr_btm_status_code))
        return -1;

    status_code = msgaGetU8(&attrs[attr_btm_status_code]);

    DEBUG_INFO("receive feat_evt_dm_recv_sta_btm_response sta("MACFMT"), status_code: %u\n",
        MACARG(msgaGetBin(&attrs[attr_mac])), status_code);

    d = alDeviceFindAny(msgaGetBin(&attrs[attr_al_mac]));
    c = clientFind(NULL, NULL, msgaGetBin(&attrs[attr_mac]));

    if (c) {
        PLATFORM_CANCEL_TIMER(c->btm_ctx.wait_rsp_timer);
        client_steer_status_update(c, CLIENT_STEER_STATUS_FINISHED);
    }

    steer_c = steerClientFind(msgaGetBin(&attrs[attr_mac]));
    record = steerRecordFindLatest(steer_c);
    if (record && ((current_ts - record->create_ts) >= local_policy.roaming_policy.cooldown*1000)) {
        record = NULL;
    }

    if (record) {
        record->btm_status_code = status_code;
        record->btm_rsp_ts = current_ts;
        if (hasMsga(attrs, attr_bssid) && !MACCMP(c->wif->bssInfo.bssid, msgaGetBin(&attrs[attr_mac]))) {
            record->finish_ts = current_ts;
        }
    }

    if ((status_code == BTM_STATUS_CODE_SUCCESS)) {
        /* start a one timer to check if switch by btm successfully */
        PLATFORM_CANCEL_TIMER(c->btm_ctx.check_timer);
        c->btm_ctx.check_timer = platformAddTimer(3*1000, 0, _staBtmResultCheckTimeHandle, c);
    } else {
        if ((d == local_device)) {
            if (record) {
                record->deauth_ts = current_ts;
            }
            client_steer_status_update(c, CLIENT_STEER_STATUS_FINISHED);
            return doSteerByDeauth(c, WLAN_REASON_BSS_TRANSITION_DISASSOC);
        }
    }

    if (record)
        STEER_RECORD_PRINT(record);

    return 0;
}

static int _staAssoEventHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct steer_client *steer_c;
    struct steer_record *record;
    struct client *c = NULL;
    uint32_t current_ts = PLATFORM_GET_TIMESTAMP(0);

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_mac) || !hasMsga(attrs, attr_if_mac))
        return -1;

    DEBUG_INFO("receive sta asso event. sta("MACFMT"), bssid: "MACFMT"\n",
        MACARG(msgaGetBin(&attrs[attr_mac])), MACARG(msgaGetBin(&attrs[attr_if_mac])));

    c = clientFind(NULL, NULL, msgaGetBin(&attrs[attr_mac]));
    steer_c = steerClientFind(msgaGetBin(&attrs[attr_mac]));
    record = steerRecordFindLatest(steer_c);
    if (record && ((current_ts - record->create_ts) >= local_policy.roaming_policy.cooldown*1000)) {
        record = NULL;
    }

    if (c) {
        PLATFORM_CANCEL_TIMER(c->btm_ctx.check_timer);
        if (c->steer_state != CLIENT_STEER_STATUS_INIT && c->steer_state != CLIENT_STEER_STATUS_FINISHED)
            client_steer_status_update(c, CLIENT_STEER_STATUS_FINISHED);
    }

    if (record && IS_ZERO_MAC(record->final_bss)) {
        MACCPY(record->final_bss, msgaGetBin(&attrs[attr_if_mac]));
        if (!MACCMP(record->target_bss, record->final_bss)) {
            record->status = 1;
            record->finish_ts = current_ts;
        } else {
            // if stick client add to deny acl list
            if ((record->type == STEER_TYPE_RCPI) &&
                isStickSta(steer_c, c->wif->i.mac) &&
                _addToDenyAclList(c, c->wif)) {
                steer_c->deny_acl_ts = current_ts;
            }
        }
        STEER_RECORD_PRINT(record);
    }

    return 0;
}

static int _staLinkMetricsQueryRspHandler(void *data, uint8_t *p, uint16_t len)
{
    struct msg_attr attrs[attr_drv_max] = {0};
    struct client *c = NULL;
    struct al_device *d = NULL;
    uint8_t rcpi_ul;

    msgaParse(attrs, attr_drv_max, p, len);

    if (!hasMsga(attrs, attr_al_mac) || !hasMsga(attrs, attr_mac) || !hasMsga(attrs, attr_station_rcpi_ul))
        return -1;

    rcpi_ul = msgaGetU8(&attrs[attr_station_rcpi_ul]);

    c = clientFind(NULL, NULL, msgaGetBin(&attrs[attr_mac]));
    if (!c || !c->wif || !c->wif->radio)
        return -1;

    d = alDeviceFindAny(msgaGetBin(&attrs[attr_al_mac]));
    if (!d)
        return -1;

    if (c->steer_state != CLIENT_STEER_STATUS_INIT && c->steer_state != CLIENT_STEER_STATUS_FINISHED) {
        DEBUG_INFO("sta("MACFMT") is in steering now. current state: %u\n", MACARG(c->mac), c->steer_state);
        return 0;
    }

    DEBUG_INFO("Recv link query Response sta("MACFMT") ul_rcpi: %u, radio("MACFMT") rcpi_thresh: %u, bsta: %u\n",
        MACARG(c->mac), rcpi_ul, MACARG(c->wif->radio->uid), local_policy.roaming_policy.rcpi_thresh, c->bsta);

    if (!local_policy.roaming_policy.rcpi_steer) {
        return 0;
    }

    if (c->bsta)
        return 0;

    if (c->link_metrics.rcpi_ul <= local_policy.roaming_policy.rcpi_thresh) {
        DEBUG_INFO("sta("MACFMT"), ul_rcpi(%u) <= rcpi_thresh(%u) then do rcpi steering\n",
            MACARG(msgaGetBin(&attrs[attr_mac])), c->link_metrics.rcpi_ul, local_policy.roaming_policy.rcpi_thresh);
        _doSpecifiedStaRCPIBasedSteering(c, false);
    }

    return 0;
}

static int _rcpiSteerStart(void *p, char *cmdline)
{
    /* receive station BTM Response event */
    featSuscribeEvent(feat_evt_dm_recv_sta_btm_response, _staBtmResponseEventHandler, NULL);
    /* station need rcpi steering event */
    featSuscribeEvent(feat_evt_dm_sta_update, _staUpdateEventHandler, NULL);
    /* MAP steering policy configuration changed event */
    featSuscribeEvent(feat_evt_dm_radio_update, _radioUpdateEventHandler, NULL);
    /* associated sta link metrics query response event */
    featSuscribeEvent(feat_evt_recv_associated_sta_link_query_rsp, _staLinkMetricsQueryRspHandler, NULL);
    /* station asso event */
    featSuscribeEvent(feat_evt_recv_sta_asso, _staAssoEventHandler, NULL);

    steerInit();
    return 0;
}

static struct extension_ops _rcpi_steer_ops = {
    .init = NULL,
    .start = _rcpiSteerStart,
    .stop = NULL,
    .deinit = NULL,
};


void rcpiSteerFeatLoad()
{
    registerExtension(&_rcpi_steer_ops);
}
