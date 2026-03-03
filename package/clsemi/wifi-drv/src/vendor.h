/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 */
#ifndef _vendor_h_
#define _vendor_h_

#include <net/cfg80211.h>
#include "cls_nl80211_vendor.h"

struct cls_wifi_anti_attack {
	int en_anti_attack;
	int internal_attack_to_lastdata;
};

enum vendor_ofdma_para_type {
	VENDOR_OFDMA_INVALID = 0x00,
	VENDOR_OFDMA_ACK_TYPE,
	VENDOR_OFDMA_DISTRIGGER_TYPE,
	VENDOR_OFDMA_PPPDUTX_TYPE,
	VENDOR_OFDMA_TRIGGER_TYPE,
	VENDOR_OFDMA_TRIGGER_TXBF,
	VENDOR_OFDMA_NUM_USERS,
	VENDOR_OFDMA_TYPE,
	VENDOR_OFDMA_DL_GI,
	VENDOR_OFDMA_DL_LTF,
	VENDOR_OFDMA_DL_NSS,
	VENDOR_OFDMA_DL_MCS,
	/* keep last */
	VENDOR_OFDMA_AFTER_LAST,
	VENDOR_OFDMA_MAX = VENDOR_OFDMA_AFTER_LAST - 1
};

enum vendor_ofdma_ack_type {
	vendor_ofdma_ack_type_m_ba = 0x00,
	vendor_ofdma_ack_type_max,
};

enum vendor_ofdma_type {
	vendor_ofdma_type_dl = 0x00,
	vendor_ofdma_type_ul,
	vendor_ofdma_type_dl2080,
	vendor_ofdma_type_max
};

enum vendor_ofdma_trigger_type {
	vendor_ofdma_trigger_basic = 0x00,
	vendor_ofdma_trigger_brp,
	vendor_ofdma_trigger_mu_bar,
	vendor_ofdma_trigger_mu_rts,
	vendor_ofdma_trigger_bsrp,
	vendor_ofdma_trigger_gcr_mu_bar,
	vendor_ofdma_trigger_bqrp,
	vendor_ofdma_trigger_ndp,
	vendor_ofdma_trigger_max,
};

enum vendor_ofdma_ppdu_tx_type {
	vendor_ofdma_ppdu_tx_su = 0x00,
	vendor_ofdma_ppdu_tx_mu,
	vendor_ofdma_ppdu_tx_er_su,
	vendor_ofdma_ppdu_tx_tb,
	vendor_ofdma_ppdu_tx_legacy,
	vendor_ofdma_ppdu_tx_max,
};
enum vendor_generic_para_type {
	VENDOR_GENERIC_INVALID = 0x00,
	VENDOR_GENERIC_TXBW_TYPE,
	VENDOR_GENERIC_LDPC_TYPE,
	VENDOR_GENERIC_BEAMFORMER,
	VENDOR_GENERIC_BEAMFORMEE,
	VENDOR_GENERIC_11NSGI20,
	VENDOR_GENERIC_SPATIAL_RX_STREAM,
	VENDOR_GENERIC_SPATIAL_TX_STREAM,
	VENDOR_GENERIC_AMSDU_CONFIG,
	VENDOR_GENERIC_AMPDU_CONFIG,
	VENDOR_GENERIC_ADDBA_REJECT = 0x0a,
	VENDOR_GENERIC_SECBAND_OFFSET,
	VENDOR_GENERIC_BW_SGNL,
	VENDOR_GENERIC_VHTSGI80,
	VENDOR_GENERIC_AFTER_LAST,
	VENDOR_GENERIC_MAX = VENDOR_GENERIC_AFTER_LAST - 1
};

enum vendor_txbandwidth {
	VENDOR_TXBW_20 = 1,
	VENDOR_TXBW_40,
	VENDOR_TXBW_80,
	VENDOR_TXBW_160,
	VENDOR_TXBW_MAX,
};

enum vendor_param_value {
	VENDOR_GENERIC_FALSE = 0x00,
	VENDOR_GENERIC_TRUE,
};

enum vendor_ltf_value {
	VENDOR_LTF_VALUE_1X = 0x00,
	VENDOR_LTF_VALUE_2X,
	VENDOR_LTF_VALUE_4X,
	VENDOR_LTF_VALUE_MAX
};

enum vendor_gi_value {
	VENDOR_GI_VALUE_0_8 = 0x00,
	VENDOR_GI_VALUE_1_6,
	VENDOR_GI_VALUE_3_2,
	VENDOR_GI_VALUE_MAX
};

extern const struct wiphy_vendor_command clsemi_vendor_cmds[];
extern uint32_t clsemi_vendor_cmds_size;
extern const struct nl80211_vendor_cmd_info clsemi_vendor_events[];
extern uint32_t clsemi_vendor_events_size;

extern const struct nla_policy cls_vnd_cmd_policy[CLS_NL80211_ATTR_MAX + 1];
int clsemi_enable_mu_tx(struct wiphy *wiphy, u8 enable);
int clsemi_enable_mu_rx(struct wiphy *wiphy, u8 enable);

int clsemi_set_beamformer(struct wiphy *wiphy, struct wireless_dev *wdev, u8 beamformer);

int cls_set_fixed_rate_for_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta);
int  cls_wifi_init_sigma(struct cls_wifi_hw *cls_wifi_hw);
int clsm_cfg80211_cmd_reply(struct wiphy *wiphy, int attr,int msg_size, void* msg);

int cls_wifi_set_skip_dfs_cac(struct wiphy *wiphy, u8 skip_dfs_cac);
u8 cls_wifi_get_skip_dfs_cac(struct wiphy *wiphy);

#endif
