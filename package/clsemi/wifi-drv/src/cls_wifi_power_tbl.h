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

#ifndef _CLS_WIFI_POWER_TBL_H_
#define _CLS_WIFI_POWER_TBL_H_

#include <linux/types.h>
#include "cls_wifi_defs.h"
#include "cls_wifi_msg_tx.h"
#include "lmac_msg.h"

#define LEGACY_MCS_LEN		12 //1 - 54, will be parsed in fw
#define N_MCS_LEN		8
#define AC_MCS_LEN		10
#define AX_MCS_LEN		12
#define MCS_MAX_LEN		AX_MCS_LEN

#define BW_2G_LEGACY_LEN	1
#define BW_5G_LEGACY_LEN	1
#define BW_2G_11N_LEN	2
#define BW_5G_11N_LEN	2
#define BW_2G_11AC_LEN	2
#define BW_5G_11AC_LEN	4
#define BW_2G_11AX_LEN	2
#define BW_5G_11AX_LEN	4

struct pppc_5g_power_list {
	int8_t legacy_txpower[BW_5G_LEGACY_LEN][LEGACY_MCS_LEN];
	int8_t n_txpower[BW_5G_11N_LEN][N_MCS_LEN];
	int8_t ac_txpower[BW_5G_11AC_LEN][AC_MCS_LEN];
	int8_t ax_txpower[BW_5G_11AX_LEN][AX_MCS_LEN];
};

struct cls_pppc_5g_txpower_entry {
	uint8_t chan;
	struct pppc_5g_power_list power_list;
};

struct pppc_2g_power_list {
	int8_t legacy_txpower[BW_2G_LEGACY_LEN][LEGACY_MCS_LEN];
	int8_t n_txpower[BW_2G_11N_LEN][N_MCS_LEN];
	int8_t ac_txpower[BW_2G_11AC_LEN][AC_MCS_LEN];
	int8_t ax_txpower[BW_2G_11AX_LEN][AX_MCS_LEN];
};

struct cls_pppc_2g_txpower_entry {
	uint8_t chan;
	struct pppc_2g_power_list power_list;
};

union dht_sync_pppc_txpower_req {
	struct pppc_5g_power_list pppc_5g_list;
	struct pppc_2g_power_list pppc_2g_list;
};

#define PPPC_5G_TX_POWER_SIZE	25
#define PPPC_2G_TX_POWER_SIZE	14

extern struct cls_pppc_5g_txpower_entry cls_pppc_5g_tx_power_list[];
extern struct cls_pppc_2g_txpower_entry cls_pppc_2g_tx_power_list[];

#define CMCC_PPPC_INVALID_POWER	127 //0x3F
#define CLS_CMCC_PPPC_DEFAULT_TXPOWER	CMCC_PPPC_INVALID_POWER

enum PPPC_PHYMODE {
	PPPC_PHYMODE_MIN = 0,
	PPPC_PHYMODE_LEGACY = PPPC_PHYMODE_MIN,
	PPPC_PHYMODE_11N,
	PPPC_PHYMODE_11AC,
	PPPC_PHYMODE_11AX,
	PPPC_PHYMODE_MAX = PPPC_PHYMODE_11AX,
};

struct txpower_db_entry {
	uint8_t radio;
	uint8_t chan;
	uint8_t phymode;
	uint8_t bw;
	int8_t power_list[12];
};

void cls_wifi_txpower_entry_init(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_tx_power_table_config(struct cls_wifi_hw *cls_wifi_hw, char *buf);
int cls_wifi_pppc_tx_power_set(struct cls_wifi_hw *cls_wifi_hw, char *buf);

#endif /* _CLS_WIFI_POWER_TBL_H_ */
