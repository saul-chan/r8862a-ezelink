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
#ifndef _NAC_H_
#define _NAC_H_

#define CLS_WIFI_NAC_RADIO0               0
#define CLS_WIFI_NAC_RADIO1               1
#define CLS_WIFI_NAC_RADIO_MAX			2
#define MAX_NAC_STA_NUM		64
#define MIN_NAC_STA_NUM		0

struct nac_monitor_sta {
	uint8_t mac_addr[6];
	int8_t rssi;
	uint8_t sinr;
	uint32_t last_rx_time;
};

struct NAC_STA_DATA {
	struct nac_monitor_sta sta;
	struct list_head list;
};

extern int pro_mode_flag[CLS_WIFI_NAC_RADIO_MAX];
extern int nac_sta_num[CLS_WIFI_NAC_RADIO_MAX];
extern struct list_head nac_sta_head[CLS_WIFI_NAC_RADIO_MAX];

int clsemi_vndr_cmds_add_nac_monitor_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);

int clsemi_vndr_cmds_remove_nac_monitor_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);

int clsemi_vndr_cmds_get_nac_sta_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int clsemi_vndr_cmds_set_nac_enable(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int clsemi_vndr_cmds_flush_all_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int init_nac_sta_head(struct cls_wifi_hw *cls_wifi_hw);
int dinit_nac_sta_head(struct cls_wifi_hw *cls_wifi_hw);
int add_nac_sta(struct cls_wifi_hw *cls_wifi_hw, u8 *mac);
int del_nac_sta(struct cls_wifi_hw *cls_wifi_hw, u8 *mac);
int set_nac_enabled(struct cls_wifi_hw *cls_wifi_hw, u32 enabled);
int get_nac_sta(struct cls_wifi_hw *cls_wifi_hw, struct nac_monitor_sta *sta_info);

void nac_monitor_per_pkt_info(struct cls_wifi_hw *cls_wifi_hw, struct hw_rxhdr *hw_rxhdr, struct rx_vector_2 *rx_vect2);
#endif /* _NAC_H_ */

