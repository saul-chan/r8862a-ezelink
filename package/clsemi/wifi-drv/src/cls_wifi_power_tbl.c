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

#include "cls_wifi_power_tbl.h"

uint8_t valid_2G_channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
#define PPPC_2G_TX_POWER_SIZE	14 // the length of valid_2G_channels[]

uint8_t valid_5G_channels[] = {36, 40, 44, 48, 52, 56, 60, 64,
			100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144,
			149, 153, 157, 161, 165};
#define PPPC_5G_TX_POWER_SIZE	25 // the length of valid_2G_channels[]

struct cls_pppc_2g_txpower_entry cls_pppc_2g_tx_power_list[PPPC_2G_TX_POWER_SIZE];
struct cls_pppc_5g_txpower_entry cls_pppc_5g_tx_power_list[PPPC_5G_TX_POWER_SIZE];

#define INIT_TXPOWER_ENTRY(entry, channel) {\
	entry.chan = channel;\
	memset(&entry.power_list, CLS_CMCC_PPPC_DEFAULT_TXPOWER, sizeof(entry.power_list));\
}

void cls_wifi_txpower_entry_init(struct cls_wifi_hw *cls_wifi_hw)
{
	int i;

	pr_info("[%s] radio %d\n", __func__, cls_wifi_hw->radio_idx);

	if (cls_wifi_hw->radio_idx == RADIO_2P4G_INDEX) {
		for (i = 0; i < PPPC_2G_TX_POWER_SIZE; i++)
			INIT_TXPOWER_ENTRY(cls_pppc_2g_tx_power_list[i], valid_2G_channels[i])

	} else {
		for (i = 0; i < PPPC_5G_TX_POWER_SIZE; i++)
			INIT_TXPOWER_ENTRY(cls_pppc_5g_tx_power_list[i], valid_5G_channels[i])
	}
}

int cls_wifi_txpower_get_mcs_len_by_phymode(enum PPPC_PHYMODE phymode)
{
	int ret = MCS_MAX_LEN;

	if (phymode == PPPC_PHYMODE_LEGACY)
		ret = LEGACY_MCS_LEN;
	else if (phymode == PPPC_PHYMODE_11N)
		ret = N_MCS_LEN;
	else if (phymode == PPPC_PHYMODE_11AC)
		ret = AC_MCS_LEN;
	else if (phymode == PPPC_PHYMODE_11AX)
		ret = AX_MCS_LEN;
	else
		pr_err("[%s] invalid phymode: %d\n", __func__, phymode);

	return ret;
}

void cls_wifi_tx_power_table_config(struct cls_wifi_hw *cls_wifi_hw, char *buf) {
	struct txpower_db_entry *db_entry;
	int i;
	bool found = false;
	int mcs_len;

	if (!strncmp(buf, "reset", 5)) {
		cls_wifi_txpower_entry_init(cls_wifi_hw);
		return;
	}

	db_entry = (void *) buf;
	mcs_len = cls_wifi_txpower_get_mcs_len_by_phymode(db_entry->phymode);

	if (cls_wifi_hw->radio_idx != db_entry->radio) {
		pr_err("[%s] invalid radio %d\n", __func__, db_entry->radio);
		return;
	}

	if(cls_wifi_hw->radio_idx == RADIO_2P4G_INDEX) {//2.4G
		if (db_entry->bw > PHY_CHNL_BW_40) {
			pr_err("[%s] radio %d: invalid bw\n", __func__, db_entry->bw);
			return;
		}

		for (i = 0; i < PPPC_2G_TX_POWER_SIZE; i++) {
			if (db_entry->chan == cls_pppc_2g_tx_power_list[i].chan) {
				if (db_entry->phymode == PPPC_PHYMODE_LEGACY)
					memcpy(cls_pppc_2g_tx_power_list[i].power_list.legacy_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);
				else if (db_entry->phymode == PPPC_PHYMODE_11N)
					memcpy(cls_pppc_2g_tx_power_list[i].power_list.n_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);
				else if (db_entry->phymode == PPPC_PHYMODE_11AC)
					memcpy(cls_pppc_2g_tx_power_list[i].power_list.ac_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);
				else if (db_entry->phymode == PPPC_PHYMODE_11AX)
					memcpy(cls_pppc_2g_tx_power_list[i].power_list.ax_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);

				found = true;
				break;
			}
		}
	} else {
		if (db_entry->bw > PHY_CHNL_BW_160) {
			pr_err("[%s] radio %d: invalid bw\n", __func__, db_entry->bw);
			return;
		}
		for (i = 0; i < PPPC_5G_TX_POWER_SIZE; i++) {
			if (db_entry->chan == cls_pppc_5g_tx_power_list[i].chan) {
				if (db_entry->phymode == PPPC_PHYMODE_LEGACY)
					memcpy(cls_pppc_5g_tx_power_list[i].power_list.legacy_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);
				else if (db_entry->phymode == PPPC_PHYMODE_11N)
					memcpy(cls_pppc_5g_tx_power_list[i].power_list.n_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);
				else if (db_entry->phymode == PPPC_PHYMODE_11AC)
					memcpy(cls_pppc_5g_tx_power_list[i].power_list.ac_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);
				else if (db_entry->phymode == PPPC_PHYMODE_11AX)
					memcpy(cls_pppc_5g_tx_power_list[i].power_list.ax_txpower[db_entry->bw],
										db_entry->power_list, mcs_len);

				found = true;
				break;
			}
		}
	}

	if (!found)
		pr_err("[%s] invalid info: radio %d phymode %d bw %d chan %d\n",
				__func__, db_entry->radio, db_entry->phymode, db_entry->bw, db_entry->chan);
}

int cls_wifi_pppc_tx_power_set(struct cls_wifi_hw *cls_wifi_hw, char *buf) {
	struct dht_set_pppc_req req;
	int ret;

	memset(&req, 0, sizeof(struct dht_set_pppc_req));

	ret = sscanf(buf, "%u %u %u %u %u %u %u", &req.enable, &req.with_param,
			&req.format,
			&req.bw,
			&req.start_mcs,
			&req.end_mcs,
			&req.txpower);

	if (ret != 7) {
		pr_err("invalid cmd: <enable> <with_param> <format> <bw> <start_mcs> <end_mcs> <txpower>\n");

		return -EINVAL;
	} else {
		pr_info("[%s] enable %u, with_param %u, format %u, bw %u, mcs%u-%u, txpower %u\n",
				__func__, req.enable, req.with_param, req.format, req.bw,
				req.start_mcs, req.end_mcs, req.txpower);
	}

	return cls_wifi_pppc_txpower_manually_req(cls_wifi_hw, &req);
}
