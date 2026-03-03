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

#ifndef _CLS_WIFI_MOD_PARAM_H_
#define _CLS_WIFI_MOD_PARAM_H_

struct cls_wifi_mod_params {
	bool ht_on;
	bool vht_on;
	bool he_on;
	int mcs_map;
	int he_mcs_map;
	bool he_ul_on;
	bool he_dl_on;
	bool ldpc_on;
	bool stbc_on;
	bool gf_rx_on;
	int phy_cfg;
	int uapsd_timeout;
	bool ap_uapsd_on;
	bool sgi;
	bool sgi80;
	bool use_2040;
	bool use_80;
	bool use_160;
	bool custregd;
	bool custchan;
	int nss;
	int amsdu_rx_max;
	bool bfmee;
	bool bfmer;
	bool mesh;
	bool murx;
	bool mutx;
	bool mutx_on;
	unsigned int roc_dur_max;
	int listen_itv;
	bool listen_bcmc;
	int lp_clk_ppm;
	bool ps_on;
	int tx_lft;
	int amsdu_maxnb;
	int amsdu_agg_timeout;
	int uapsd_queues;
	bool tdls;
	bool uf;
	char *ftl;
	bool dpsm;
	int tx_to_bk;
	int tx_to_be;
	int tx_to_vi;
	int tx_to_vo;
	int amsdu_tx;
	bool ant_div;
	bool amsdu_require_spp;
	int debug_mode;
	bool afe_enable;
	bool dfs_enable;
	bool boot_cali_enable;
	int ba_buffer_size;
	int density;
	bool xtal_ctrim_set;
	int bw_2g;
	int bw_5g;
	int chan_ieee_2g;
	int chan_ieee_5g;
	int ant_mask;
	int afe_dig_ctrl;
	int afe_fem_en;
	int afe_feat_mask;
	int ant_num_2g;
	int ant_num_5g;
	int dpd_tx_power_5g;
	int dpd_tx_power_2g_20;
	int dpd_tx_power_2g_40;
	int dpd_tx_nss;
	bool dpd_online_en;
	bool dpd_disable_cca;
	bool heartbeat_en;
	int heartbeat_skip;
	int heartbeat_cnt;
	int heartbeat_recover;
	bool debug_pci;
	bool gain_tcomp_en;
	bool zif_online_en;
	int txq_max_len;
	int txq_init_credits;
	int irq_force_exit_time;
	bool rd_agc_war_en;
	bool low_pwr_en;
	char *cal_path_cs8862;
	char *cal_path_cs8662;
	char *cal_path_m3k;
	char *irf_path_cs8862;
	char *irf_path_cs8662;
	char *irf_path_m3k;
	bool high_temp_op;
	bool load_rx_cali_tbl;
	bool ibex_en;
	int dynamic_vlan;
};

extern struct cls_wifi_mod_params cls_wifi_mod_params;

struct cls_wifi_hw;
struct wiphy;

int cls_wifi_handle_dynparams(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy);
void cls_wifi_custregd(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy);
void cls_wifi_enable_wapi(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_enable_mfp(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_enable_ccmp_256(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_enable_gcmp(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_adjust_amsdu_maxnb(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_limit_amsdu_maxnb(struct cls_wifi_hw *cls_wifi_hw, uint8_t amsdu_nb);

#endif /* _CLS_WIFI_MOD_PARAM_H_ */
