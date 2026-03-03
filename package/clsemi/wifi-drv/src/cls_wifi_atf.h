/**
 ****************************************************************************************
 *
 * @file cls_wifi_atf.h
 *
 * @brief File containing the ATF related definition and functions.
 *
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 ****************************************************************************************
 */
#ifndef _CLS_WIFI_ATF_H_
#define _CLS_WIFI_ATF_H_


#define CLS_ATF_QUOTA_DEFAULT 1000
#define CLS_ATF_SCHED_PERIOD_DEFAULT (20 * 1000)

/* The Requirement of SRRC Interference adaptive:
 * Once Interference appear, the Channel occupancy does not exceed 10% in 50ms.
 * So the default stats period is 2ms.
 */
#define CLS_ATF_STATS_PERIOD_DEFAULT		(2000)
#define CLS_ATF_STATS_BUSY_THRES_DEFAULT	(1200)
#define CLS_ATF_STATS_CLEAR_THRES_DEFAULT	(800)
#define CLS_ATF_STATS_CLEAR_TIMES_DEFAULT	(10)
#define CLS_ATF_STATS_CLEAR_DURATION_DEFAULT	(CLS_ATF_STATS_PERIOD_DEFAULT * CLS_ATF_STATS_CLEAR_TIMES_DEFAULT)

enum cls_atf_granularity {
	CLS_ATF_GRAN_RADIO,
	CLS_ATF_GRAN_BSS,
	CLS_ATF_GRAN_STA,
	CLS_ATF_GRAN_MAX,
};

enum cls_atf_mode {
	CLS_ATF_MODE_MONITOR,
	CLS_ATF_MODE_ENFORCED,
	CLS_ATF_MODE_MAX,
};

enum {
	ATF_CMD_ENABLE,
	ATF_CMD_MODE,
	ATF_CMD_GRAN,
	ATF_CMD_BSS_QUOTA,
	ATF_CMD_STA_QUOTA,
	ATF_CMD_UPDATE_QUOTA_TABLE,
	ATF_CMD_SCHED_PERIOD,
	ATF_CMD_STATS_PERIOD,
	ATF_CMD_GET_STATS,
	ATF_CMD_STATS_BUSY_THRES,
	ATF_CMD_STATS_CLEAR_THRES,
	ATF_CMD_STATS_CLEAR_DURATION,
	ATF_CMD_SRRC_ENABLE,
	ATF_CMD_LOG_ENABLE,
	ATF_CMD_MAX,
};

enum atf_chan_type {
	ATF_CHAN_TYPE_TRAFFIC = 0,
	ATF_CHAN_TYPE_OFF_CHAN_SCAN,
	ATF_CHAN_TYPE_NORMAL_SCAN,

	ATF_CHAN_TYPE_UNKNOWN = 0xF,
};

struct atf_per_radio_airtime_stats {
	/* CCA busy duration, from WMAC registers */
	uint32_t cca_busy_prim20;
	uint32_t cca_busy_sec20;
	uint32_t cca_busy_sec40;
	uint32_t cca_busy_sec80;

	/* CCA busy on primary 20MHz (removed Rx time )*/
	uint32_t air_cca_busy;

	/* UL data frame stats (from unknown STA) */
	uint32_t unknown_ul_data_airtime;

	/* UL data frame stats (from associated STA) */
	uint32_t ul_data_airtime;

	/* mgmt/cntrl frame airtime stats (from unknown STA) */
	uint32_t mgmt_ctrl_aritime;
};

struct atf_per_bss_airtime_stats {
	/* UL data frame airtime stats */
	uint32_t ul_data_airtime_stats;
	/* DL data frame airtime_stats */
	uint32_t dl_data_airtime_stats;
	/* DL mgmt/ctrl frame airtime stats */
	uint32_t dl_mgmt_ctrl_aritime_stats;
};

struct atf_per_sta_airtime_stats {
	/* UL data frame stats */
	uint32_t ul_data_airtime_stats;
	/* dl_data_airtime_stats */
	uint32_t dl_data_airtime_stats;
};

/* this structure is aligned 4 Bytes */
struct atf_airtime_info {
	uint32_t mode;
	uint32_t period;
	uint32_t freq;
	uint32_t start_time;
	uint32_t end_time;
	int32_t noise_dbm;
};
struct atf_airtime_stats {
	struct atf_airtime_info *info;
	struct atf_per_radio_airtime_stats *radio_acc;
	struct atf_per_radio_airtime_stats *radio;
	struct atf_per_bss_airtime_stats *bss;
	struct atf_per_sta_airtime_stats *sta;
};

struct atf_per_bss_params {
	uint32_t quota;
	uint32_t quota_us;
};

struct atf_per_sta_params {
	uint32_t quota;
	uint32_t quota_us;
};

struct atf_quota_table {
	uint32_t *quota_us;
};

struct cls_atf_params {
	struct atf_quota_table quota_table;
	struct atf_airtime_stats atf_stats;
	uint8_t enabled;
	uint8_t srrc_enable;
	uint8_t atf_log;
	enum cls_atf_mode mode;
	enum cls_atf_granularity granularity;
	uint32_t sched_period_us;
	uint32_t stats_period_us;
	uint32_t stats_busy_thres_us;
	uint32_t stats_clear_thres_us;
	uint32_t stats_clear_duration_us;
};

void cls_wifi_atf_set_bss_quota(struct cls_wifi_vif *vif, uint32_t quota);
void cls_wifi_atf_set_sta_quota(struct cls_wifi_sta *sta, uint32_t quota);
void cls_wifi_atf_update_vif_quota_us(struct cls_wifi_hw *wifi_hw);
void cls_wifi_atf_update_sta_quota_us(struct cls_wifi_hw *wifi_hw);

void cls_wifi_atf_set_enable(struct cls_wifi_hw *wifi_hw, uint8_t enable);
void cls_wifi_atf_set_mode(struct cls_wifi_hw *wifi_hw, uint8_t mode);
void cls_wifi_atf_set_granularity(struct cls_wifi_hw *wifi_hw, uint8_t gran);
void cls_wifi_atf_set_bss_quota_from_mac(struct cls_wifi_hw *wifi_hw,
						uint8_t *bss_mac_addr, uint32_t quota);
void cls_wifi_atf_set_sta_quota_from_mac(struct cls_wifi_hw *wifi_hw,
						uint8_t *sta_mac_addr, uint32_t quota);
void cls_wifi_atf_update_quota_table(struct cls_wifi_hw *wifi_hw);
void cls_wifi_atf_set_sched_period(struct cls_wifi_hw *wifi_hw, uint32_t sched_period);
void cls_wifi_atf_set_stats_period(struct cls_wifi_hw *wifi_hw, uint32_t stats_period);
void cls_wifi_atf_set_stats_busy_thres(struct cls_wifi_hw *wifi_hw, uint32_t stats_thres);
void cls_wifi_atf_set_stats_clear_thres(struct cls_wifi_hw *wifi_hw, uint32_t stats_thres);
void cls_wifi_atf_set_stats_clear_duration(struct cls_wifi_hw *wifi_hw, uint32_t stats_thres);
void cls_wifi_atf_set_srrc_enable(struct cls_wifi_hw *wifi_hw, uint8_t srrc_enable);
void cls_wifi_atf_set_log_enable(struct cls_wifi_hw *wifi_hw, uint8_t atf_log);
void cls_wifi_atf_get_stats(struct cls_wifi_hw *wifi_hw);

void cls_wifi_atf_dump_quota_table(struct cls_wifi_hw *wifi_hw);
void cls_wifi_atf_dump_history_stats(struct cls_wifi_hw *wifi_hw);
void cls_wifi_atf_stats_process(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_atf_init(struct cls_wifi_hw *wifi_hw);

void cls_wifi_vndr_event_report_atf_stats(struct cls_wifi_hw *wifi_hw);

int clsemi_vndr_cmds_set_atf_enable(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_atf_mode(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_atf_granularity(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_atf_bss_quota(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_atf_sta_quota(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_update_atf_quota_table(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_atf_sched_period(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_atf_stats_period(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_get_atf_stats(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);

uint16_t cls_wifi_quota_table_size(struct cls_wifi_hw *wifi_hw);
uint16_t cls_wifi_airtime_stats_size(struct cls_wifi_hw *wifi_hw);
#endif // _CLS_WIFI_ATF_H_
