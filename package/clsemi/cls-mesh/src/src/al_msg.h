#ifndef _AL_MSG_H
#define _AL_MSG_H

#include <stdint.h>

#define RADIO_CAPA_SAE  (0x01)

struct msg_attr {
    uint16_t len;
    uint8_t *data;
};

enum {
    attr_none = -1,
};

enum msg_family {
    msg_family_driver_evt = 0,
    msg_family_driver_cmd = 0x1,
    msg_family_feat_evt = 0x2,
    msg_family_feat_cmd = 0x3,
    msg_family_max,
};

enum drv_evt {
    drv_evt_packet = 0,
    drv_evt_radio,
    drv_evt_radio_delete,
    drv_evt_bss,
    drv_evt_bss_delete,
    drv_evt_station,
    drv_evt_station_delete,
    drv_evt_chan_scan_finished,
    drv_evt_chan_scan_results,
    drv_evt_mgmt_frame,
    drv_evt_connect,
    drv_evt_disconnect,
    drv_evt_config_change,
    drv_evt_disallow_notification,
    drv_evt_vip_conf_changed,
    drv_evt_nac_stainfo,
    drv_evt_chan_survey,

    drv_evt_max,
};

enum drv_cmd {
    drv_cmd_teardown = 0,
    drv_cmd_bss,
    drv_cmd_set_channel_txpower,
    drv_cmd_send_mgmt_frame,
    drv_cmd_register_mgmt_frame,
    drv_cmd_get_station_stats,
    drv_cmd_trigger_scan,
    drv_cmd_get_chan_scan_results,
    drv_cmd_deauth_sta,
    drv_cmd_deny_sta_add,
    drv_cmd_deny_sta_del,
    drv_cmd_commit,
    drv_cmd_start_wps,
    drv_cmd_vip_add,
    drv_cmd_queue_action,
    drv_cmd_dscp_action,
    drv_cmd_tc_action,
    drv_cmd_vip_clear,
    drv_cmd_set_neighbor,
    drv_cmd_eth_vlan,
    drv_cmd_set_nac_sta,
    drv_cmd_set_nac_enable,
    drv_cmd_flush_nac_sta,
    drv_cmd_get_chan_survey,

    drv_cmd_max,
};

enum drv_attr {
    attr_ret_code = 0,
    attr_al_mac,
    attr_if_mac,
    attr_if_idx,
    attr_if_name,
    attr_bssid,
    attr_bss_role,
    attr_ssid,
    attr_signal,
    attr_encrypt,
    attr_auth,
    attr_key,
    attr_backhaul,
    attr_fronthaul,
    attr_backhaul_ssid,
    attr_backhaul_encrypt,
    attr_backhaul_auth,
    attr_backhaul_key,
    attr_ethertype,
    attr_packet,
    attr_radio_id,
    attr_radio_name,
    attr_radio_mac,
    attr_radio_capa,
    attr_band,
    attr_ht_capa,
    attr_vht_capa,
    attr_vht_rx_mcs,
    attr_vht_tx_mcs,
    attr_he_capa,
    attr_he_mcs,
    attr_power_lvl,
    attr_bandwidth,
    attr_channel,
    attr_opclass,
    attr_opclass_id,
    attr_opclass_max_txpower,
    attr_opclass_chan_id,
    attr_opclass_chan_pref,
    attr_opclass_chan_reason,
    attr_opclass_chan_freq_separation,
    attr_band_idx,
    attr_chan_scan_on_boot,
    attr_chan_scan_impact,
    attr_chan_scan_min_interval,
    attr_frame,
    attr_frame_type,
    attr_match,
    attr_mac,
    attr_ts,
    attr_ies,
    attr_beacon_ies,
    attr_station_last_ts,
    attr_station_rate_dl,
    attr_station_rate_ul,
    attr_station_rcpi_ul,
    attr_station_last_rcpi_ul,
    attr_station_rx_bytes,
    attr_station_tx_bytes,
    attr_station_rx_packets,
    attr_station_tx_packets,
    attr_station_tx_group_packets,
    attr_station_rx_errors,
    attr_station_tx_errors,
    attr_station_tx_tries,
    attr_station_last_datarate_dl,  // TODO
    attr_station_last_datarate_ul,  // TODO
    attr_station_utilization_rx,  // TODO
    attr_station_utilization_tx,  // TODO
    attr_start_num,
    attr_finished_flag,
    attr_reason_code,
    attr_chan_util,
    attr_rcpi_threshhold,
    attr_chan_util_threshhold,
    attr_steering_policy_mode,
    attr_max_vbss,
    attr_vbss_subtract,
    attr_vbssid_restrictions,
    attr_matched_and_mask_restrictions,
    attr_fixed_bits_restrictions,
    attr_fixed_bits_mask,
    attr_fixed_bits_value,
    attr_btm_req_token,
    attr_btm_rsp_token,
    attr_btm_status_code,
    attr_disallow,
    attr_vip_result,
    attr_dscp_value,
    attr_dscp_tid,
    attr_dscp_qid,
    attr_dscp_dft_tid,
    attr_dscp_dft_qid,
    attr_queue_port,
    attr_queue_id,
    attr_queue_weight,
    attr_tc_tid,
    attr_tc_value,
    attr_tc_dft_tid,
    attr_tc_dft_qid,
    attr_nr,
    attr_flag,
    attr_pwd,
    attr_group_key,
    attr_client_assoc,
    attr_client_disassoc,
    attr_success,
    attr_cmdu_id,
    attr_vlan_0,
    attr_vlan_n = attr_vlan_0+VLAN_MAX-1,
    attr_wds,
    attr_chan_noise,
    attr_chan_dwell,
    attr_chan_busytime,
    //add new here
    attr_drv_max,
};

#define FLAG_ADD (1)
#define FLAG_DEL (2)

enum feat_evt {
    feat_evt_dm_device_update = 0,
    feat_evt_dm_device_delete,
    feat_evt_dm_radio_update,
    feat_evt_dm_radio_delete,
    feat_evt_dm_bss_update,
    feat_evt_dm_bss_delete,
    feat_evt_dm_sta_update,
    feat_evt_dm_sta_delete,
    feat_evt_dm_seen_update,
    feat_evt_dm_seen_delete,
    feat_evt_dm_recv_sta_btm_response,
    feat_evt_recv_sta_asso,
    feat_evt_recv_associated_sta_link_query_rsp,
    feat_evt_vbss_start,                // start vbss process
    feat_evt_vbss_client_security_context_request, // vbss client security request
    feat_evt_vbss_creation_request,     // vbss creation request
    feat_evt_vbss_destruction_request,  // vbss destruction request
    feat_evt_vbss_response,             // vbss response
    feat_evt_vbss_trigger_csa_response,  // vbss trigger csa response

    feat_evt_max,
};


#define hasMsga(_attrs, _type) ((_attrs)[_type].data)

#define ACTION_FAILED   1
#define ACTION_SUCCESS  0

uint32_t msgGetMaxBufSize(void);
int msgaParseOne(struct msg_attr *attrs, uint8_t **pbuf, uint16_t *plen);
int msgaParse(struct msg_attr *attrs, uint16_t max, uint8_t *buf, uint16_t len);
uint8_t *msgaPutAttrHeader(uint8_t *p, uint16_t type, uint16_t len);
uint16_t msgaGetLen(struct msg_attr *attr);
uint8_t msgaGetU8(struct msg_attr *attr);
uint8_t *msgaPutU8(uint8_t *p, uint16_t type, uint8_t value);
uint16_t msgaGetU16(struct msg_attr *attr);
uint8_t *msgaPutU16(uint8_t *p, uint16_t type, uint16_t value);
uint32_t msgaGetU32(struct msg_attr *attr);
uint8_t *msgaPutU32(uint8_t *p, uint16_t type, uint32_t value);
uint32_t msgaGetU64(struct msg_attr *attr);
uint8_t *msgaPutU64(uint8_t *p, uint16_t type, uint64_t value);
uint8_t *msgaGetBin(struct msg_attr *attr);
uint8_t *msgaPutBin(uint8_t *p, uint16_t type, uint8_t *buf, uint16_t len);
char *msgaGetStr(struct msg_attr *attr);
uint8_t *msgaPutStr(uint8_t *p, uint16_t type, char *str);
uint8_t *msgaPutFlag(uint8_t *p, uint16_t type);

int msgInit();
int msgRegisterFamily(uint8_t family, void (*handler)(void *, uint8_t *, uint32_t), void *data);
int msgSend(uint8_t *msg, uint16_t len);
uint8_t * msgGetBuf(int size);
void msgPutBuf(uint8_t *buf);

#define MSG_PUT_HEADER(_p, _family, _evt) \
    do {\
        *(_p++) = _family;\
        *(_p++) = _evt;\
    }while (0)


#endif
