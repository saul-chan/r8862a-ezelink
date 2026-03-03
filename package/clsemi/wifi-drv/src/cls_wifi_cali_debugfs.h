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

#ifndef _CLS_WIFI_CALI_DEBUGFS_H_
#define _CLS_WIFI_CALI_DEBUGFS_H_
#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/firmware.h>
#include <linux/if_ether.h>

#include "cali_struct.h"
#include "cali_rxstats.h"
#endif
enum cali_cmd_type {
	TYPE_RADIO,
	TYPE_BSS,
	TYPE_PTK,
	TYPE_GTK,
	TYPE_SU,
	TYPE_MU,
	TYPE_MACPHY,
	TYPE_MUINFO,
	TYPE_SOUNDING,
	TYPE_WMM,
	TYPE_XTAL_CAL,
	TYPE_RXSTATS,
	TYPE_IGTK,
	TYPE_CSI,
	TYPE_PPPC,
	TYPE_MLD,
	TYPE_AP_MLD,
	TYPE_STA_MLD,
	TYPE_TDMA,
	TYPE_MAX
};

enum cali_action_type {
	TYPE_ACTION_TXSU,
	TYPE_ACTION_TXMU,
	TYPE_ACTION_TXSND,
	TYPE_ACTION_DIFSAMPLE,
	TYPE_ACTION_LOG_SET,
	TYPE_ACTION_RADAR_DETECT,
	TYPE_ACTION_INTERFERENCE_DETECT,
	TYPE_ACTION_RSSI,
	TYPE_ACTION_TEST_UNCACHE_RW,
	TYPE_ACTION_MAX
};

struct type_name_list {
	enum cali_cmd_type type;
	int is_multi;
	char **name_list;
};

extern int current_radio;
extern struct cali_config_tag g_cal_config[2];
#ifdef __KERNEL__
extern struct cal_per_radio_stats g_rx_stats;
extern struct cal_per_radio_status g_rx_status;
extern struct cal_tx_stats g_tx_stats;
extern struct cls_csi_report g_cali_csi_report;
#endif
extern enum XTAL_CAL_STAUS g_xtal_cali_status;

#ifndef EINVAL
#define EINVAL 1
#endif

#ifdef __KERNEL__
int set_def_cal_config(struct cali_config_tag *pconf, char *filename, struct device *device);
#endif
void set_param(char *buf, struct cali_config_tag *ptag);
void get_param(char *name, char *out_buf, struct cali_config_tag *ptag);
int set_bss_param(char *name, char *pval, struct cali_config_tag *ptag);
int set_pgtk_param(char *name, char *pval, struct cali_key_info_tag *ptag);
int set_macphy_param(char *name, char *pval, struct cali_config_tag *ptag);
int set_user_ppdu_param(char *name, char *pval, struct cali_ppdu_info_tag *ptag);
int set_user_info_param(char *name, char *pval, struct cali_user_info_tag *ptag);
int set_user_mac_phy_param(char *name, char *pval,
								struct cali_per_user_mac_phy_params_tag *ptag);
void set_single_param(enum cali_cmd_type type, int idx, char *name,
							char *value, struct cali_config_tag *ptag);
void set_all_param(enum cali_cmd_type type, int idx, char *buf,
						int len, struct cali_config_tag *ptag);
int show_param(enum cali_cmd_type type, char *name, char *outbuf,
					int size, struct cali_config_tag *ptag);
int show_status_rx(char *outbuf, int count, struct cal_per_radio_status *pstatus);
int show_stats_tx(char *outbuf, int count, struct cal_tx_stats *pstats);
int show_stats_rx(char *outbuf, int count, struct cal_per_radio_stats *pstats);
int show_stats_rssi(char *outbuf, int count, int32_t status, int8_t rssi);
int show_temperature(char *outbuf, int count, int32_t status, int8_t temp);
int show_stats_rx_mpdu_num(char *outbuf, int count, struct cal_per_radio_stats *pstats);
int show_csi_result(char *outbuf, int count, int32_t status, struct cls_csi_report *pcsi);


#endif //_CLS_WIFI_CALI_DEBUGFS_H_
