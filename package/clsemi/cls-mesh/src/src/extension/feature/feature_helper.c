
#include "platform.h"
#include "datamodel.h"
#include "al_msg.h"
#include "feature_helper.h"

int alDeviceUpdateEvt(uint8_t *mac)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_dm_device_update);

    p = msgaPutBin(p, attr_mac, mac, MACLEN);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendBtmResponseEvent(uint8_t *al_mac, uint8_t *mac, uint8_t *wif_mac, uint8_t status_code, uint8_t *target)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_dm_recv_sta_btm_response);

    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutBin(p, attr_if_mac, wif_mac, MACLEN);
    p = msgaPutU8(p, attr_btm_status_code, status_code);
    if (target)
        p = msgaPutBin(p, attr_bssid, target, MACLEN);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int stationUpdateEvent(uint8_t *mac, uint8_t *wif_mac, uint8_t ul_rcpi, uint8_t last_rcpi)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_dm_sta_update);

    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutBin(p, attr_if_mac, wif_mac, MACLEN);
    p = msgaPutU8(p, attr_station_rcpi_ul, ul_rcpi);
    p = msgaPutU8(p, attr_station_last_rcpi_ul, last_rcpi);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendClientAssoEvent(uint8_t *mac, uint8_t *wif_mac)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_recv_sta_asso);

    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutBin(p, attr_if_mac, wif_mac, MACLEN);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int mapSteeringPolicyChangedEvent(uint8_t *mac, uint8_t rcpi_thresh, uint8_t chan_util, uint8_t policy)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_dm_radio_update);

    p = msgaPutBin(p, attr_radio_mac, mac, MACLEN);
    p = msgaPutU8(p, attr_rcpi_threshhold, rcpi_thresh);
    p = msgaPutU8(p, attr_chan_util_threshhold, chan_util);
    p = msgaPutU8(p, attr_steering_policy_mode, policy);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int assocStaLinkMetricsQueryRspEvent(uint8_t *al_mac, uint8_t *mac, uint8_t rcpi_ul, uint8_t mac_rate_ul, uint8_t mac_rate_dl)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_recv_associated_sta_link_query_rsp);

    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutU8(p, attr_station_rcpi_ul, rcpi_ul);
    p = msgaPutU8(p, attr_station_rate_ul, mac_rate_ul);
    p = msgaPutU8(p, attr_station_rate_dl, mac_rate_dl);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendVbssStartEvent(uint8_t *sta_mac, uint8_t *src_agent, uint8_t *target_ruid)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_vbss_start);

    p = msgaPutBin(p, attr_mac, sta_mac, MACLEN);
    p = msgaPutBin(p, attr_al_mac, src_agent, MACLEN);
    p = msgaPutBin(p, attr_radio_mac, target_ruid, MACLEN);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendVbssClientSecurityContextReqEvent(uint16_t cid, uint8_t *al_mac, uint8_t *bssid, uint8_t *client_mac)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_vbss_client_security_context_request);

    p = msgaPutU16(p, attr_cmdu_id, cid);
    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_bssid, bssid, MACLEN);
    p = msgaPutBin(p, attr_mac, client_mac, MACLEN);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendVbssCreationEvent(uint16_t cid, uint8_t *al_mac, uint8_t *ruid, uint8_t *bssid, struct vvData *ssid,
            struct vvData *wpa_password, uint8_t *client_mac, uint8_t client_assoc, struct vvData *ptk,
            uint64_t tx_packet_number, struct vvData *gtk, uint64_t group_tx_packet_number,
            struct vvData *sta_assoc_req)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_vbss_creation_request);

    p = msgaPutU16(p, attr_cmdu_id, cid);
    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_radio_mac, ruid, MACLEN);
    p = msgaPutBin(p, attr_bssid, bssid, MACLEN);
    p = msgaPutBin(p, attr_ssid, ssid->datap, ssid->len);
    if (wpa_password && wpa_password->len > 0)
        p = msgaPutBin(p, attr_pwd, wpa_password->datap, wpa_password->len);
    if (client_mac)
        p = msgaPutBin(p, attr_mac, client_mac, MACLEN);
    p = msgaPutU8(p, attr_client_assoc, client_assoc);
    if (ptk && ptk->len > 0)
        p = msgaPutBin(p, attr_key, ptk->datap, ptk->len);
    p = msgaPutU64(p, attr_station_tx_packets, tx_packet_number);
    if (gtk && gtk->len > 0)
        p = msgaPutBin(p, attr_group_key, gtk->datap, gtk->len);
    p = msgaPutU64(p, attr_station_tx_group_packets, group_tx_packet_number);
    if (sta_assoc_req && sta_assoc_req->len > 0)
        p = msgaPutBin(p, attr_frame, sta_assoc_req->datap, sta_assoc_req->len);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendVbssDestructionEvent(uint16_t cid, uint8_t *al_mac, uint8_t *ruid, uint8_t *bssid, uint8_t client_disassoc)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_vbss_destruction_request);

    p = msgaPutU16(p, attr_cmdu_id, cid);
    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_radio_mac, ruid, MACLEN);
    p = msgaPutBin(p, attr_bssid, bssid, MACLEN);
    p = msgaPutU8(p, attr_client_disassoc, client_disassoc);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendVbssResponseEvent(uint8_t *al_mac, uint8_t *ruid, uint8_t *bssid, uint8_t success)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_vbss_response);

    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_radio_mac, ruid, MACLEN);
    p = msgaPutBin(p, attr_bssid, bssid, MACLEN);
    p = msgaPutU8(p, attr_success, success);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int sendVbssTriggerCSARspEvent(uint8_t *al_mac, uint8_t *bssid, uint8_t *client_mac)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_feat_evt, feat_evt_vbss_trigger_csa_response);

    p = msgaPutBin(p, attr_al_mac, al_mac, MACLEN);
    p = msgaPutBin(p, attr_bssid, bssid, MACLEN);
    p = msgaPutBin(p, attr_mac, client_mac, MACLEN);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}


