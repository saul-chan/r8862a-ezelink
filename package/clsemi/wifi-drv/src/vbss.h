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
#ifndef _VBSS_H_
#define _VBSS_H_

typedef char string_32[33];
typedef char string_64[65];

enum cls_vbss_roam_results {
	CLS_VBSS_ROAM_RESULT_FAIL,
	CLS_VBSS_ROAM_RESULT_SUCCESS,
};

enum cls_vbss_sta_roam_sts {
	CLS_VBSS_STA_ROAM_DEFAULT = 0,
	CLS_VBSS_STA_ROAM_ONGOING,
};


enum WPA_AUTHORIZE_TYPE {
	WPA_AUTHORIZE_TYPE_NONE = 0,
	WPA_AUTHORIZE_TYPE_WPA,
	WPA_AUTHORIZE_TYPE_WPA2,
	WPA_AUTHORIZE_TYPE_SAE
};

/* VBSS: the station information to be roamed */
struct cls_vbss_driver_sta_info {
	uint8_t sta_mac[ETH_ALEN];
	struct vbss_sta_seq_num sta_seq_info;
};

enum {
	VBSS_VAP_ORIGIN,
	VBSS_VAP_ROAMING,
	VBSS_VAP_ROLE_MAX
};

/* VBSS: the VAP(BSS) information to be roamed */
struct cls_vbss_vap_info {
	uint8_t bssid[ETH_ALEN];
	string_32 ssid;
	string_32 ifname;
	enum WPA_AUTHORIZE_TYPE auth_type;
	string_64 pwd;
	uint8_t role;
};

struct cls_vbss_vap {
	uint8_t sta_key_valid;
	uint8_t role;
	uint8_t roam_result;
	uint16_t succ_cnt;
	uint16_t fail_cnt;
};


/* The role of this AP in VBSS roaming */
enum vbss_roaming_role {
	VBSS_AP_DEFAULT,
	VBSS_AP_SOURCE,
	VBSS_AP_DESTINATION,
	VBSS_AP_ROLE_MAX
};

struct vbss_ap_stats {
	struct vbss_cca_report_ind cca_info;
	uint32_t tx_mpdu_num;
	uint32_t tx_bytes;
	uint32_t rx_mpdu_num;
	uint32_t rx_bytes;
};

struct vbss_monitor_sta {
	uint8_t mac_addr[6];
	uint8_t valid;
	uint8_t rssi;
	uint8_t sinr;
	uint32_t last_rx_time;
};

#define VBSS_MAX_MONITOR_STA_NUM 8
struct cls_vbss_info {
	/* Common Info */
	uint8_t enabled;
	/* When AP has decrypted the data MPDUs */
	uint16_t n_threshod;
	/* Per radio CCA info */
	struct vbss_cca_report_ind cca_info;

	/* When the roaming done, driver would check the success/failure of received packets */
	/* CLS_NL80211_ATTR_VBSS_NTHRESH */
	uint32_t succ_thresh;
	/* CLS_NL80211_ATTR_VBSS_MTHRESH */
	uint32_t fail_thresh;
	uint8_t rssi_smooth;

	uint16_t succ_cnt;
	uint16_t fail_cnt;

	/* TODO: Add a STA/VAP list if multiple roaming works concurrently.
	 * Currently, we only keep 1 pair STA/VAP.
	 * The following info need to be cleaned after roaming done.
	 */
	uint32_t roam_result;
	struct cls_wifi_sta *sta;
	struct cls_vbss_driver_sta_info vbss_sta;
	struct cls_vbss_vap_info vbss_vap;
	uint32_t roam_sts : 1;

	/* VBSS to monitor specific stations */
	uint32_t num_monitor_sta;
	struct vbss_monitor_sta monitor_sta[VBSS_MAX_MONITOR_STA_NUM];

	struct vbss_get_sta_seq_num_cfm get_seq_cfm;
	struct vbss_set_sta_seq_num_cfm set_seq_cfm;
};

struct cls_vbss_signal_info {
	s8 sinr;
	s8 rssi;
};

int clsemi_vbss_vap_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif);

int clsemi_vndr_cmds_get_vbss_enabled(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_vbss_enabled(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_get_sta_info(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_get_vap_info(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vbss_init(struct cls_wifi_hw *cls_wifi_hw);

#endif /* _VBSS_H_ */

