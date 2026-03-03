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

/**
 * INCLUDE FILES
 ******************************************************************************
 */

#include <linux/slab.h>
#include "lmac_msg.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_bfmer.h"

/**
 * FUNCTION DEFINITIONS
 ******************************************************************************
 */

int cls_wifi_bfmer_report_add(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
						  unsigned int length)
{
	gfp_t flags;
	struct cls_wifi_bfmer_report *bfm_report;

	if (in_softirq())
		flags = GFP_ATOMIC;
	else
		flags = GFP_KERNEL;

	/* Allocate a structure that will contain the beamforming report */
	bfm_report = kmalloc(sizeof(*bfm_report) + length, flags);


	/* Check report allocation */
	if (!bfm_report) {
		/* Do not use beamforming */
		return -1;
	}

	/* Store report length */
	bfm_report->length = length;

	/*
	 * Need to provide a Virtual Address to the MAC so that it can
	 * upload the received Beamforming Report in driver memory
	 */
	bfm_report->dma_addr = dma_map_single(cls_wifi_hw->dev, &bfm_report->report[0],
										  length, DMA_FROM_DEVICE);

	/* Check DMA mapping result */
	if (dma_mapping_error(cls_wifi_hw->dev, bfm_report->dma_addr)) {
		/* Free allocated report */
		kfree(bfm_report);
		/* And leave */
		return -1;
	}

	/* Store report structure */
	cls_wifi_sta->bfm_report = bfm_report;

	return 0;
}

void cls_wifi_bfmer_report_del(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta)
{
	/* Verify if a report has been allocated */
	if (cls_wifi_sta->bfm_report) {
		struct cls_wifi_bfmer_report *bfm_report = cls_wifi_sta->bfm_report;

		/* Unmap DMA region */
		dma_unmap_single(cls_wifi_hw->dev, bfm_report->dma_addr,
						 bfm_report->length, DMA_BIDIRECTIONAL);

		/* Free allocated report structure and clean the pointer */
		kfree(bfm_report);
		cls_wifi_sta->bfm_report = NULL;
	}
}

u8 cls_wifi_bfmer_get_rx_nss(u16 rx_mcs_map)
{
	int i;
	u8 rx_nss = 0;

	for (i = 7; i >= 0; i--) {
		u8 mcs = (rx_mcs_map >> (2 * i)) & 3;

		if (mcs != IEEE80211_VHT_MCS_NOT_SUPPORTED) {
			rx_nss = i + 1;
			break;
		}
	}

	return rx_nss;
}

#define CBF_FILE_SIZE 512
void cls_wifi_bfmer_save_sta_cbf(struct cls_wifi_hw *wifi_hw, struct cls_wifi_sta *sta)
{
	struct cls_wifi_bfmer_report *cbf;
	u8 buf[CBF_FILE_SIZE];
	uint16_t buf_size = 0;
	struct file *cbf_file;
	int i;

	if (sta == NULL || sta->bfm_report == NULL)
		return;

	snprintf(buf, CBF_FILE_SIZE,"/tmp/cbf_sta_%02x_%02x_%02x_%02x_%02x_%02x",
		sta->mac_addr[0],sta->mac_addr[1],sta->mac_addr[2],
		sta->mac_addr[3],sta->mac_addr[4],sta->mac_addr[5]);
	cbf_file = filp_open(buf, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (IS_ERR(cbf_file)) {
		pr_warn("Open target file fail: %s\n", buf);

		return;
	}

	buf_size += snprintf(&buf[buf_size], CBF_FILE_SIZE - buf_size,
		"STA MAC: %pM format %s\n", &sta->mac_addr[0], (sta->he ? "HE": "VHT"));
	kernel_write(cbf_file, buf, buf_size, &cbf_file->f_pos);
	buf_size = 0;
	cbf = sta->bfm_report;
	for (i = 0 ; i < cbf->length; i++) {
		if ((i % 16) == 0)
			buf_size += snprintf(&buf[buf_size], CBF_FILE_SIZE - buf_size, "\n");

		buf_size += snprintf(&buf[buf_size], CBF_FILE_SIZE - buf_size,
				" %02x", cbf->report[i]);
		kernel_write(cbf_file, buf, buf_size, &cbf_file->f_pos);
		buf_size = 0;
	}

	filp_close(cbf_file, NULL);
}

/* Loop STAs associated, save CBF into file */
void cls_wifi_bfmer_save_cbf(struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_sta *sta;
	int i;

	for (i = 0; i < hw_remote_sta_max(wifi_hw); i++) {
		sta = &wifi_hw->sta_table[i];
		if (sta->valid && sta->bfm_report && sta->bfm_report->length)
			cls_wifi_bfmer_save_sta_cbf(wifi_hw, sta);
	}
}

void cls_wifi_bfmer_cmd_handler(struct cls_wifi_hw *wifi_hw, int cmd, int value)
{
	struct bf_parameters_req *bf_req;
	int send_cmd = 1;

	bf_req = &wifi_hw->bf_param.bf_req;
	bf_req->command = cmd;
	switch (cmd) {
	case BF_CMD_ENABLE:
		bf_req->enabled = value;
		break;
	case BF_CMD_SND_MODE:
		bf_req->snd_mode = value;
		break;
	case BF_CMD_SND_MAX_GRP_USER:
		bf_req->max_grp_user = value;
		break;
	case BF_CMD_SND_MAX_STA_NUM:
		bf_req->max_snd_sta = value;
		break;
	case BF_CMD_SND_PERIOD:
		bf_req->snd_period = value;
		break;
	case BF_CMD_SND_FEEDBACK_TYPE:
		bf_req->feedback_type = value;
		break;
	case BF_CMD_CBF_LIFETIME:
		bf_req->cbf_lifetime = value;
		break;
	case BF_CMD_SND_LOG_LEVEL:
		bf_req->snd_log_level = value;
		break;
	case BF_CMD_BF_LOG_LEVEL:
		bf_req->bf_log_level = value;
		break;
	case BF_CMD_DUMP_CBF:
		send_cmd = 0;
		cls_wifi_bfmer_save_cbf(wifi_hw);
		break;
	case BF_CMD_NDP_POWER:
		bf_req->ndp_power = value;
		break;
	case BF_CMD_NDP_GI:
		bf_req->ndp_gi = value;
		break;
	case BF_CMD_NDP_BW:
		bf_req->ndp_bw = value;
		break;
	case BF_CMD_NDP_TIME_CSD:
		bf_req->ndp_time_csd = value;
		break;
	case BF_CMD_NDP_SMM_IDX:
		bf_req->ndp_smm_idx = value;
		break;
	case BF_CMD_SUPPORT_2SS:
		bf_req->support_2ss = value;
		break;
	case BF_CMD_ENABLE_SMOOTH:
		bf_req->enable_smooth = value;
		break;
	case BF_CMD_ENABLE_FILTER:
		bf_req->enable_fiter = value;
		break;
	case BF_CMD_ALPHA:
		bf_req->alpha = value;
		break;
	case BF_CMD_SNAPSHOT:
		bf_req->snapshot = value;
		break;
	default:
		pr_info("%s unknown command\n", __func__);
	}

	if (send_cmd)
		cls_wifi_send_bf_parameters_req(wifi_hw, bf_req);

}

void cls_wifi_bf_init(struct cls_wifi_hw *wifi_hw)
{
	wifi_hw->bf_param.bf_req.enabled = 0;
	wifi_hw->bf_param.bf_req.snd_mode = 2;
	wifi_hw->bf_param.bf_req.max_grp_user = 1;
	wifi_hw->bf_param.bf_req.max_snd_sta = 4;
	wifi_hw->bf_param.bf_req.feedback_type = 0;
	wifi_hw->bf_param.bf_req.support_2ss = 0;
	wifi_hw->bf_param.bf_req.snd_period = 200000;
	wifi_hw->bf_param.bf_req.cbf_lifetime = 300000;
	wifi_hw->bf_param.bf_req.snd_log_level = 1;
	wifi_hw->bf_param.bf_req.bf_log_level = 1;
	wifi_hw->bf_param.bf_req.ndp_power = 17;
	wifi_hw->bf_param.bf_req.ndp_gi = 1;
	wifi_hw->bf_param.bf_req.ndp_bw = 0xFF;
	wifi_hw->bf_param.bf_req.ndp_time_csd = 0;
	wifi_hw->bf_param.bf_req.ndp_smm_idx = 0;
	wifi_hw->bf_param.bf_req.support_2ss = 0;
	wifi_hw->bf_param.bf_req.enable_smooth = 0;
	wifi_hw->bf_param.bf_req.enable_fiter = 0;
	wifi_hw->bf_param.bf_req.alpha = 2048;
	wifi_hw->bf_param.bf_req.snapshot = 0;
	cls_wifi_send_bf_parameters_req(wifi_hw, &wifi_hw->bf_param.bf_req);
}
