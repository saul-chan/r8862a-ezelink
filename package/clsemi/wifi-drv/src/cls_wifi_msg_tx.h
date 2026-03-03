/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#ifndef _CLS_WIFI_MSG_TX_H_
#define _CLS_WIFI_MSG_TX_H_
#ifdef __KERNEL__
#include "cls_wifi_defs.h"
#define CLS_MAC_EN_DUPLICATE_DETECTION_BIT         ((uint32_t)0x80000000)
#define CLS_MAC_ACCEPT_UNKNOWN_BIT                 ((uint32_t)0x40000000)
#define CLS_MAC_ACCEPT_OTHER_DATA_FRAMES_BIT       ((uint32_t)0x20000000)
#define CLS_MAC_ACCEPT_QO_S_NULL_BIT               ((uint32_t)0x10000000)
#define CLS_MAC_ACCEPT_QCFWO_DATA_BIT              ((uint32_t)0x08000000)
#define CLS_MAC_ACCEPT_Q_DATA_BIT                  ((uint32_t)0x04000000)
#define CLS_MAC_ACCEPT_CFWO_DATA_BIT               ((uint32_t)0x02000000)
#define CLS_MAC_ACCEPT_DATA_BIT                    ((uint32_t)0x01000000)
#define CLS_MAC_ACCEPT_OTHER_CNTRL_FRAMES_BIT      ((uint32_t)0x00800000)
#define CLS_MAC_ACCEPT_CF_END_BIT                  ((uint32_t)0x00400000)
#define CLS_MAC_ACCEPT_ACK_BIT                     ((uint32_t)0x00200000)
#define CLS_MAC_ACCEPT_CTS_BIT                     ((uint32_t)0x00100000)
#define CLS_MAC_ACCEPT_RTS_BIT                     ((uint32_t)0x00080000)
#define CLS_MAC_ACCEPT_PS_POLL_BIT                 ((uint32_t)0x00040000)
#define CLS_MAC_ACCEPT_BA_BIT                      ((uint32_t)0x00020000)
#define CLS_MAC_ACCEPT_BAR_BIT                     ((uint32_t)0x00010000)
#define CLS_MAC_ACCEPT_OTHER_MGMT_FRAMES_BIT       ((uint32_t)0x00008000)
#define CLS_MAC_ACCEPT_BFMEE_FRAMES_BIT            ((uint32_t)0x00004000)
#define CLS_MAC_ACCEPT_ALL_BEACON_BIT              ((uint32_t)0x00002000)
#define CLS_MAC_ACCEPT_NOT_EXPECTED_BA_BIT         ((uint32_t)0x00001000)
#define CLS_MAC_ACCEPT_DECRYPT_ERROR_FRAMES_BIT    ((uint32_t)0x00000800)
#define CLS_MAC_ACCEPT_BEACON_BIT                  ((uint32_t)0x00000400)
#define CLS_MAC_ACCEPT_PROBE_RESP_BIT              ((uint32_t)0x00000200)
#define CLS_MAC_ACCEPT_PROBE_REQ_BIT               ((uint32_t)0x00000100)
#define CLS_MAC_ACCEPT_MY_UNICAST_BIT              ((uint32_t)0x00000080)
#define CLS_MAC_ACCEPT_UNICAST_BIT                 ((uint32_t)0x00000040)
#define CLS_MAC_ACCEPT_ERROR_FRAMES_BIT            ((uint32_t)0x00000020)
#define CLS_MAC_ACCEPT_OTHER_BSSID_BIT             ((uint32_t)0x00000010)
#define CLS_MAC_ACCEPT_BROADCAST_BIT               ((uint32_t)0x00000008)
#define CLS_MAC_ACCEPT_MULTICAST_BIT               ((uint32_t)0x00000004)
#define CLS_MAC_DONT_DECRYPT_BIT                   ((uint32_t)0x00000002)
#define CLS_MAC_EXC_UNENCRYPTED_BIT                ((uint32_t)0x00000001)
#endif

struct cls_wifi_msg
{
	u16 id;
	u16 dest_id;
	u16 src_id;
	u16 param_len;
	u32 param[];
};

#ifdef __KERNEL__
int cls_wifi_send_start(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_version_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_version_cfm *cfm);
int cls_wifi_send_add_if(struct cls_wifi_hw *cls_wifi_hw, const unsigned char *mac,
					 enum nl80211_iftype iftype, bool p2p, struct mm_add_if_cfm *cfm);
int cls_wifi_send_remove_if(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index);
int cls_wifi_send_set_channel(struct cls_wifi_hw *cls_wifi_hw, int phy_idx,
						  struct mm_set_channel_cfm *cfm);
int cls_wifi_send_key_add(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u16 sta_idx, bool pairwise,
					  u8 *key, u8 key_len, u8 key_idx, u8 cipher_suite,
					  struct mm_key_add_cfm *cfm);
int cls_wifi_send_key_del(struct cls_wifi_hw *cls_wifi_hw, uint8_t hw_key_idx);
int cls_wifi_send_bcn_change(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u32 bcn_addr,
						 u16 bcn_len, u16 tim_oft, u16 tim_len, u16 *csa_oft, u16 cca_oft);
int cls_wifi_send_tim_update(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u16 aid,
						 u8 tx_status);
int cls_wifi_send_roc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
				  struct ieee80211_channel *chan, unsigned int duration);
int cls_wifi_send_cancel_roc(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_set_power(struct cls_wifi_hw *cls_wifi_hw,  u8 vif_idx, s8 pwr,
						struct mm_set_power_cfm *cfm);
int cls_wifi_send_set_edca(struct cls_wifi_hw *cls_wifi_hw, u8 hw_queue, u32 param,
					   bool uapsd, u8 inst_nbr);
int cls_wifi_send_tdls_chan_switch_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
								   struct cls_wifi_sta *cls_wifi_sta, bool sta_initiator,
								   u8 oper_class, struct cfg80211_chan_def *chandef,
								   struct tdls_chan_switch_cfm *cfm);
int cls_wifi_send_tdls_cancel_chan_switch_req(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_vif *cls_wifi_vif,
										  struct cls_wifi_sta *cls_wifi_sta,
										  struct tdls_cancel_chan_switch_cfm *cfm);

int cls_wifi_send_mm_rc_stats(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx,
						  struct mm_rc_stats_cfm *cfm);
int cls_wifi_send_mm_rc_set_rate(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx,
							 u32 rate_config, bool fix_rate);
int cls_wifi_send_get_power(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx,
				struct mm_get_power_cfm *cfm);
int cls_wifi_send_get_bctx_pn(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx,
				struct mm_bctx_pn_cfm *cfm);
#if CONFIG_CLS_SMTANT
int cls_wifi_send_mm_set_smant(struct cls_wifi_hw *cls_wifi_hw, struct mm_smart_antenna_req *smant_cfg);
#endif

int cls_wifi_send_set_puncture_info(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u8 inact_bitmap);
#ifdef CONFIG_CLS_WIFI_HEMU_TX
int cls_wifi_send_mm_ul_parameters(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_mm_dl_parameters(struct cls_wifi_hw *cls_wifi_hw);
#endif /* CONFIG_CLS_WIFI_HEMU_TX */

#ifdef CONFIG_CLS_WIFI_P2P_DEBUGFS
int cls_wifi_send_p2p_oppps_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
							u8 ctw, struct mm_set_p2p_oppps_cfm *cfm);
int cls_wifi_send_p2p_noa_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
						  int count, int interval, int duration,
						  bool dyn_noa, struct mm_set_p2p_noa_cfm *cfm);
#endif /* CONFIG_CLS_WIFI_P2P_DEBUGFS */

int cls_wifi_send_me_config_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_me_chan_config_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_me_set_control_port_req(struct cls_wifi_hw *cls_wifi_hw, bool opened,
									  u16 sta_idx);
int cls_wifi_send_me_sta_add(struct cls_wifi_hw *cls_wifi_hw, struct station_parameters *params,
						 const u8 *mac, u8 inst_nbr, struct me_sta_add_cfm *cfm);
int cls_wifi_send_me_sta_del(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx, bool tdls_sta);
int cls_wifi_send_me_traffic_ind(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx, bool uapsd, u8 tx_status);
int cls_wifi_send_twt_request(struct cls_wifi_hw *cls_wifi_hw,
						  u8 setup_type, u8 vif_idx,
						  struct twt_conf_tag *conf,
						  struct twt_setup_cfm *cfm);
int cls_wifi_send_twt_teardown(struct cls_wifi_hw *cls_wifi_hw,
						   struct twt_teardown_req *twt_teardown,
						   struct twt_teardown_cfm *cfm);
int cls_wifi_send_me_set_ps_mode(struct cls_wifi_hw *cls_wifi_hw, u8 ps_mode);
int cls_wifi_send_sm_connect_req(struct cls_wifi_hw *cls_wifi_hw,
							 struct cls_wifi_vif *cls_wifi_vif,
							 struct cfg80211_connect_params *sme,
							 struct sm_connect_cfm *cfm);
int cls_wifi_send_sm_disconnect_req(struct cls_wifi_hw *cls_wifi_hw,
								struct cls_wifi_vif *cls_wifi_vif,
								u16 reason);
int cls_wifi_send_sm_external_auth_required_rsp(struct cls_wifi_hw *cls_wifi_hw,
											struct cls_wifi_vif *cls_wifi_vif,
											u16 status);
int cls_wifi_send_sm_ft_auth_rsp(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
							 uint8_t *ie, int ie_len);
int cls_wifi_send_apm_start_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							struct cfg80211_ap_settings *settings,
							struct apm_start_cfm *cfm);
int cls_wifi_send_apm_stop_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif);
int cls_wifi_send_apm_probe_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							struct cls_wifi_sta *sta, struct apm_probe_client_cfm *cfm);
int cls_wifi_send_scanu_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
						struct cfg80211_scan_request *param);
int cls_wifi_send_scanu_abort_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif);
int cls_wifi_send_apm_start_cac_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
								struct cfg80211_chan_def *chandef,
								struct apm_start_cac_cfm *cfm, bool rd_enable);
int cls_wifi_send_apm_stop_cac_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif);
int cls_wifi_send_tdls_peer_traffic_ind_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif);
int cls_wifi_send_config_monitor_req(struct cls_wifi_hw *cls_wifi_hw,
								 struct cfg80211_chan_def *chandef,
								 struct me_config_monitor_cfm *cfm);
int cls_wifi_send_mesh_start_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							 const struct mesh_config *conf, const struct mesh_setup *setup,
							 struct mesh_start_cfm *cfm);
int cls_wifi_send_mesh_stop_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							struct mesh_stop_cfm *cfm);
int cls_wifi_send_mesh_update_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							  u32 mask, const struct mesh_config *p_mconf, struct mesh_update_cfm *cfm);
int cls_wifi_send_mesh_peer_info_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
								 u16 sta_idx, struct mesh_peer_info_cfm *cfm);
void cls_wifi_send_mesh_peer_update_ntf(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
									u16 sta_idx, u8 mlink_state);
void cls_wifi_send_mesh_path_create_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, u8 *tgt_addr);
int cls_wifi_send_mesh_path_update_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, const u8 *tgt_addr,
								   const u8 *p_nhop_addr, struct mesh_path_update_cfm *cfm);
void cls_wifi_send_mesh_proxy_add_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, u8 *ext_addr);
int cls_wifi_send_check_alarm_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_check_alm_req *check_alm_req);

#ifdef CONFIG_CLS_WIFI_BFMER
void cls_wifi_send_bfmer_enable(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
							struct station_parameters *params);
#endif /* CONFIG_CLS_WIFI_BFMER */

/* Debug messages */
int cls_wifi_send_dbg_trigger_req(struct cls_wifi_hw *cls_wifi_hw, char *msg);
int cls_wifi_send_dbg_mem_read_req(struct cls_wifi_hw *cls_wifi_hw, u32 mem_addr,
							   struct dbg_mem_read_cfm *cfm);
int cls_wifi_send_dbg_mem_write_req(struct cls_wifi_hw *cls_wifi_hw, u32 mem_addr,
								u32 mem_data);
int cls_wifi_send_dbg_set_mod_filter_req(struct cls_wifi_hw *cls_wifi_hw, u32 filter);
int cls_wifi_send_dbg_set_sev_filter_req(struct cls_wifi_hw *cls_wifi_hw, u32 filter);
int cls_wifi_send_dbg_get_sys_stat_req(struct cls_wifi_hw *cls_wifi_hw,
								   struct dbg_get_sys_stat_cfm *cfm);
int cls_wifi_send_dbg_get_mib_req(struct cls_wifi_hw *cls_wifi_hw,
								   struct dbg_get_mib_cfm *cfm);
int cls_wifi_send_dbg_get_dbgcnt_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_dbg_get_phy_dfx_req(struct cls_wifi_hw *cls_wifi_hw,
								   struct dbg_get_phy_dfx_cfm *cfm);
int cls_wifi_send_dbg_get_dbgcnt_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_dbg_pct_stat_request(struct cls_wifi_hw *cls_wifi_hw,
					struct dbg_pct_stat_req *conf, struct dbg_pct_stat_cfm *cfm);
int cls_wifi_send_dbg_profile_stat_request(struct cls_wifi_hw *cls_wifi_hw,
					struct dbg_profile_stat_req *conf, struct dbg_profile_stat_cfm *cfm);
int cls_wifi_send_dbg_get_mgmt_req(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index,
		struct dbg_get_mgmt_stats_cfm *cfm);
int cls_wifi_send_dbg_reset_mgmt_req(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index);
int cls_wifi_send_cfg_rssi_req(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index, int rssi_thold, u32 rssi_hyst);

int cls_wifi_send_cal_msg_req(struct cls_wifi_hw *cls_wifi_hw,
						lmac_msg_id_t msg_id, int msg_len, void *msg,
						lmac_msg_id_t cfm_id, void *cfm);
int cls_wifi_send_cal_rx_stats_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id, uint32_t clear);
int cls_wifi_send_cal_rx_status_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id);

int cls_wifi_sync_pppc_txpower_req(struct cls_wifi_hw *cls_wifi_hw, struct cfg80211_chan_def *chandef);
int cls_wifi_send_dyn_pwr_offset_req(struct cls_wifi_hw *cls_wifi_hw, int8_t offset);
int cls_wifi_pppc_txpower_manually_req(struct cls_wifi_hw *cls_wifi_hw, struct dht_set_pppc_req *tmp);
int cls_wifi_pppc_txpower_show_record_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_set_rd_max_num_thrd_req(struct cls_wifi_hw *cls_wifi_hw, u8 thrd);
int cls_wifi_send_get_rd_max_num_thrd_req(struct cls_wifi_hw *cls_wifi_hw,
				struct mm_rd_max_num_thrd_cfm *cfm);
int cls_wifi_send_set_rd_channel_req(struct cls_wifi_hw *cls_wifi_hw, u8 channel);
int cls_wifi_send_get_rd_channel_req(struct cls_wifi_hw *cls_wifi_hw,
				struct mm_rd_chan_cfm *cfm);
int cls_wifi_send_set_rd_dbg_req(struct cls_wifi_hw *cls_wifi_hw, bool lvl);
int cls_wifi_send_force_soft_reset_hw_req(struct cls_wifi_hw *cls_wifi_hw,
				struct dht_force_reset_hw_req_op *reset_req);
int cls_wifi_send_set_rd_agc_war_req(struct cls_wifi_hw *cls_wifi_hw, u8 enable);
int cls_wifi_send_get_rd_agc_war_req(struct cls_wifi_hw *cls_wifi_hw,
				struct mm_rd_agc_war_cfm *cfm);
#if defined(MERAK2000) && MERAK2000
int cls_wifi_send_set_trace_loglevel(struct cls_wifi_hw *cls_wifi_hw, u8 radio, int mod, int val,
				struct dbg_trace_loglevel_set_cfm *cfm);
int cls_wifi_send_get_trace_loglevel(struct cls_wifi_hw *cls_wifi_hw, u8 radio,
				struct dbg_trace_loglevel_get_cfm *cfm);
#endif
#endif

/* IRF messages */
#ifdef __KERNEL__
int cls_wifi_send_irf_smp_lms_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_smp_lms_req *smp_lms);
int cls_wifi_send_irf_write_lms_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_write_lms_req *lms_config,
									struct irf_write_lms_cfm *cfm);
int cls_wifi_send_irf_set_calc_step_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_calc_step_req *calc_setp_req);
#endif

int cls_wifi_send_message(struct cls_wifi_hw *cls_wifi_hw, u8 *buf);
int cls_wifi_send_irf_hw_init_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_hw_init_req *hw_init,
								struct irf_hw_init_cfm *cfm);
int cls_wifi_send_irf_hw_cfg_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_hw_cfg_req *hw_cfg_req);
int cls_wifi_send_irf_set_mode_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_mode_req *set_mode_req);
int cls_wifi_send_irf_show_tbl_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_show_tbl_req *show_tbl_req);
int cls_wifi_send_irf_show_status_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_show_req *show_status_req);
int cls_wifi_send_irf_dif_eq_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_dif_eq_req *dif_eq_req);
int cls_wifi_send_irf_dif_eq_save_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_dif_eq_save_req *dif_eq_req);
int cls_wifi_send_irf_run_task_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_run_task_req *run_task_req);
int cls_wifi_send_irf_calib_evt_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_evt_req *calib_evt_req,
								struct irf_set_cali_evt_cfm *cfm);
int cls_wifi_send_irf_start_sched_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_start_schedule_req *start_sched_req);
int cls_wifi_send_irf_eq_data_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_eq_data_req *send_data_req);
int cls_wifi_send_irf_smp_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_smp_config_req *smp_config,
								struct irf_smp_config_cfm *cfm);
int cls_wifi_send_irf_smp_start_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_smp_start_req *smp_start);
int cls_wifi_send_irf_send_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_config_req *send_config,
								struct irf_send_config_cfm *cfm);
int cls_wifi_send_irf_send_start_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_start_req *send_start,
								struct irf_send_start_cfm *cfm);
int cls_wifi_send_irf_send_stop_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_stop_req *send_stop,
								struct irf_send_stop_cfm *cfm);
int cls_wifi_send_irf_dcoc_calc_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_fb_dcoc_req *fb_dcoc_req, lmac_msg_id_t dcoc_type);
int cls_wifi_send_irf_xtal_cal_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_xtal_cali_req *xtal_cali_req,
							   struct irf_xtal_cali_cfm *xtal_cali_cfm);
int cls_wifi_send_irf_init_txcali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *init_txcali_req);
int cls_wifi_send_irf_txcali_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *txcali_pwr_req);
int cls_wifi_send_irf_fbcali_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *fbcali_pwr_req);
int cls_wifi_send_irf_tx_cali_save_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *tx_cali_save_req);
int cls_wifi_send_irf_init_rxcali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *init_rxcali_req);
int cls_wifi_send_irf_rxcali_rssi_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *rxcali_pwr_req);
int cls_wifi_send_irf_rssi_cali_save_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *rssi_cali_save_req);
int cls_wifi_send_irf_set_rx_gain_lvl_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_set_rx_gain_lvl_req *rx_gain_lvl_req);
int cls_wifi_send_irf_afe_cfg_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_afe_cfg_req *afe_cfg_req);
int cls_wifi_send_irf_radar_detect_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_radar_detect_req *radar_detect_req);
int cls_wifi_send_irf_interference_detect_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_interference_detect_req *interference_detect_req,
							   struct irf_interference_detect_cfm *interference_detect_cfm);
int cls_wifi_send_irf_set_task_param_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_task_param_req *task_thres_req);
int cls_wifi_send_irf_tx_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_tx_power_req *tx_power);
int cls_wifi_set_irf_pppc_switch(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_pppc_switch_req *pppc_req);
int cls_wifi_send_irf_show_capacity_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_show_capacity_req *capacity_req);
int cls_wifi_send_irf_set_subband_idx_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_subband_idx_req *set_subband_req);
int cls_wifi_send_cal_get_csi_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id);
int cls_wifi_send_plat_param(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_reset(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_send_wmm_lock(struct cls_wifi_hw *cls_wifi_hw, uint32_t lock_edca);
int cls_wifi_send_cal_get_csi_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id);
int cls_wifi_send_irf_afe_cmd_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_afe_cmd_req *afe_cmd_req);
int cls_wifi_send_xtal_ctrim_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_xtal_ctrim_config_req *xtal_ctrim_config,
									struct irf_xtal_ctrim_config_cfm *cfm);
int cls_wifi_send_irf_cca_cs_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_cca_cs_config_req *cca_cs_config,
									struct irf_cca_cs_config_cfm *cfm);
int cls_wifi_send_irf_cca_ed_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_cca_ed_config_req *cca_ed_config,
									struct irf_cca_ed_config_cfm *cfm);
int cls_wifi_send_irf_fb_gain_err_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *fb_err_req);
int cls_wifi_send_irf_tx_fcomp_cali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_tx_fcomp_cali_req *tx_fcomp_cali_req);
int cls_wifi_send_irf_tx_act_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_tx_cur_pwr_req *tx_act_pwr_req);
int cls_wifi_send_irf_tx_loop_power_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_tx_cur_pwr_req *tx_loop_pwr_req);
int cls_wifi_send_irf_rx_gain_lvl_cali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_rx_gain_lvl_cali_req *rx_gain_lvl_cali_req);
int cls_wifi_send_irf_rx_gain_freq_cali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_rx_gain_freq_cali_req *rx_gain_freq_cali_req);
int cls_wifi_send_irf_save_rx_gain_freq_ofst_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *save_rx_gain_freq_ofst_req);
int cls_wifi_send_irf_gain_dbg_lvl_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_gain_dbg_lvl_req *dbg_lvl_req);
int cls_wifi_send_irf_tx_gain_err_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *tx_err_req);
int cls_wifi_send_irf_tx_loop_pwr_init_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *loop_init_req);
int cls_wifi_send_bf_parameters_req(struct cls_wifi_hw *cls_wifi_hw,
		struct bf_parameters_req *cbf_req);
int cls_wifi_send_irf_agc_reload_req(struct cls_wifi_hw *cls_wifi_hw,
		uint8_t radio_idx, uint32_t len);
int cls_wifi_send_th_wall_req(struct cls_wifi_hw *cls_wifi_hw,
			struct irf_th_wall_req *th_wall_req, struct irf_th_wall_cfm *cfm);

#ifdef __KERNEL__
#ifdef CONFIG_CLS_VBSS
int clsemi_set_sta_seq_num_req(struct cls_wifi_hw *cls_wifi_hw, struct vbss_set_sta_seq_num_req *req);
int clsemi_get_sta_seq_num_req(struct cls_wifi_hw *cls_wifi_hw, struct vbss_get_sta_seq_num_cfm *cfm);
#endif
int clsemi_send_rx_filter(struct cls_wifi_hw *cls_wifi_hw, uint8_t rx_filter);
#endif

int cls_wifi_send_ampdu_max_size_req(struct cls_wifi_hw *cls_wifi_hw, int ampdu_max);
int cls_wifi_send_smm_idx_req(struct cls_wifi_hw *cls_wifi_hw, int smm_indx);
int cls_wifi_send_ampdu_prot_req(struct cls_wifi_hw *cls_wifi_hw, int prot_type);
int cls_wifi_send_txop_en_req(struct cls_wifi_hw *cls_wifi_hw, int txop_en);
int cls_wifi_log_to_uart_req(struct cls_wifi_hw *cls_wifi_hw, int enable);
int cls_wifi_rts_cts_dbg_req(struct cls_wifi_hw *cls_wifi_hw, int enable);
int cls_wifi_dump_edma_info_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_m3k_boc_reg_req(struct cls_wifi_hw *cls_wifi_hw, int enable);
#ifdef __KERNEL__
int cls_wifi_send_csi_cmd_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_csi_params_req *csi);
int cls_wifi_send_atf_update_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_atf_params_req *atf);
int cls_wifi_send_dpd_wmac_tx_cmd_req(struct cls_wifi_hw *cls_wifi_hw,
		struct mm_dpd_wmac_tx_params_req *tx_req);

int cls_wifi_send_irf_dcoc_soft_dbg_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_fb_dcoc_req *fb_dcoc_req, lmac_msg_id_t dcoc_type);
int cls_wifi_send_clicmd_cdf_req(struct cls_wifi_hw *cls_wifi_hw, void *para);
int cls_wifi_set_tx_rx_loop_online_en_req(struct cls_wifi_hw *cls_wifi_hw,
						struct tx_rx_loop_online_en_req *tx_rx_loop_online_en_config);
#endif
int cls_wifi_mem_ops(struct cls_wifi_hw *cls_wifi_hw, uint8_t op, uint8_t region, u32 offset,
		void *addr, u32 len);
int cls_wifi_irf_set_aci_det_para_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_aci_det_para_req *aci_det_para_req);
int cls_wifi_irf_get_aci_det_para_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_aci_det_para_req *aci_det_para_req);
int cls_wifi_send_irf_set_rx_gain_cali_para_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_cali_para_req *cali_para_req);
int cls_wifi_send_irf_get_rx_gain_cali_para_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_cali_para_req *cali_para_req);
int cls_wifi_send_irf_rx_gain_cali_prep_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_cali_prep_req *cali_prep_req);
int cls_wifi_send_irf_rx_gain_lvl_bist_cali_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_lvl_bist_cali_req *bist_cali_req);
int cls_wifi_send_irf_rx_gain_imd3_test_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_imd3_test_req *imd3_test_req);
int cls_wifi_send_set_irf_pwr_ctrl_thre_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_ctrl_thre_req *irf_pwr_ctrl_thre_req);
int cls_wifi_send_get_irf_pwr_ctrl_thre_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_ctrl_thre_req *irf_pwr_ctrl_thre_req);
int cls_wifi_send_set_irf_pwr_prec_offset_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_prec_offset_req *pwr_prec_offset);
int cls_wifi_send_get_irf_pwr_prec_offset_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_prec_offset_req *pwr_prec_offset);
int cls_wifi_send_set_irf_comp_stub_bitmap_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_comp_stub_bitmap_req *comp_stub_bitmap);
int cls_wifi_send_get_irf_comp_stub_bitmap_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_comp_stub_bitmap_req *comp_stub_bitmap);
int cls_wifi_send_irf_set_digital_tx_gain_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_set_digital_tx_gain_req *imd3_test_req);

#endif /* _CLS_WIFI_MSG_TX_H_ */
