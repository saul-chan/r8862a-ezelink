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
#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>

#include "cls_wifi_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_version.h"
#include "cali_struct.h"
#include "cali_rxstats.h"
#include "cls_wifi_cali.h"
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_csi.h"

#define CLS_MAC_TSF_LO_ADDR	0x00b085dc
#define CLS_MAC_TSF_HI_ADDR	0x00b085e0

enum {
	CLS_WIFI_UC_TEST_STATE_INIT,
	CLS_WIFI_UC_TEST_STATE_LHOST_DONE,
	CLS_WIFI_UC_TEST_STATE_WPU_DONE,
};

struct cls_wifi_uncache_test_info {
	uint32_t len;
	uint32_t state;
};
#endif

static int cls_wifi_init_cal_sounding(struct cls_wifi_hw *cls_wifi_hw);
static int cls_wifi_rx_cal_param_config_cfm(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_rx_stats_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_send_cal_rx_status_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_tx_su_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_tx_su_res_ind(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_tx_stats_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_tx_mu_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_sounding_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
// Log
static int cls_wifi_recv_cal_log_set_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);
static int cls_wifi_recv_cal_log_get_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);

static int cls_wifi_cal_mem_op_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx,
		int op_type, uint32_t addr, int req_words, void *buf);
static int cls_wifi_update_cbf_report(struct cls_wifi_hw *cls_wifi_hw);
static int cls_wifi_send_cal_get_csi_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg);

msg_cb_fct cal_hdlrs[MSG_I(CAL_MSG_MAX)] = {
	[MSG_I(CAL_PARAM_CONFIG_CFM)] = cls_wifi_rx_cal_param_config_cfm,
	[MSG_I(CAL_RX_STATS_CFM)] = cls_wifi_recv_cal_rx_stats_cfm,
	[MSG_I(CAL_RX_STATUS_CFM)] = cls_wifi_send_cal_rx_status_cfm,
	[MSG_I(CAL_TX_SU_CFM)] = cls_wifi_recv_cal_tx_su_cfm,
	[MSG_I(CAL_TX_SU_RES_IND)] = cls_wifi_recv_cal_tx_su_res_ind,
	[MSG_I(CAL_TX_STATS_CFM)] = cls_wifi_recv_cal_tx_stats_cfm,
	[MSG_I(CAL_TX_MU_CFM)] = cls_wifi_recv_cal_tx_mu_cfm,
	[MSG_I(CAL_SOUNDING_CFM)] = cls_wifi_recv_cal_sounding_cfm,
	[MSG_I(CAL_LOG_SET_CFM)] = cls_wifi_recv_cal_log_set_cfm,
	[MSG_I(CAL_LOG_GET_CFM)] = cls_wifi_recv_cal_log_get_cfm,
	[MSG_I(CAL_GET_CSI_CFM)] = cls_wifi_send_cal_get_csi_cfm,
};

#ifdef __KERNEL__
struct cls_csi_report g_cali_csi_report = {0};
struct cal_per_radio_stats g_rx_stats = {0};
struct cal_per_radio_status g_rx_status = {0};
struct cal_tx_stats g_tx_stats = {0};
struct cali_config_tag g_cal_config[MAX_RADIO_NUM];
struct cls_wifi_hw *g_radio_cls_wifi_hw[MAX_RADIO_NUM] =
#if MAX_RADIO_NUM == 2
{0, 0};
#endif

int current_radio = -1;

#define WPU_REGS_SECTION	6
struct wpu_regs_map wpu_regs_map[MAX_RADIO_NUM][WPU_REGS_SECTION] = {
	/* 2.4G RADIO */
	{
		{0x48940000, 832, NULL},		/* CRM */
		{0x48B00000, 1380, NULL},		/* MAC_CORE */
		{0x48B08000, 1436, NULL},		/* MAC_PL */
		{0x48C00000, 4696, NULL},		/* MDM */
		{0x48C01258, 164, NULL},		/* Clourney TMOD_PARA */
		{0x48C0B000, 1284, NULL},		/* RIU */
	},

	/* 5G RADIO */
	{
		{0x4A940000, 832, NULL},		/* CRM */
		{0x4AB00000, 1380, NULL},		/* MAC_CORE */
		{0x4AB08000, 1436, NULL},		/* MAC_PL */
		{0x4AC00000, 4696, NULL},		/* MDM */
		{0x4AC01258, 164, NULL},		/* Clourney TMOD_PARA */
		{0x4AC0B000, 1284, NULL},		/* RIU */
	}
};
#endif
#ifdef __KERNEL__
int cls_wifi_cal_init(struct cls_wifi_hw *cls_wifi_hw)
{
	int radio_index;
	int phy_band;

#if !defined(MERAK2000)
	int i;
#endif

	if (!cls_wifi_hw || !cls_wifi_hw->dev)
		return -1;

	/* set the calibration environment */
	cls_wifi_hw->cal_env = (struct cls_wifi_cal_env_tag *)
		kzalloc(sizeof(struct cls_wifi_cal_env_tag), GFP_KERNEL);
	if (!cls_wifi_hw->cal_env)
		return -ENOMEM;

	if ((sizeof(struct cali_config_tag) % 4) != 0)
		pr_err("cali_config_tag is not multiple by 4: %zu\n",
			sizeof(struct cali_config_tag));

	pr_err("Calibration Task ID: %d, Config ID: %d, CLen=%zu\n",
		TASK_CAL, CAL_PARAM_CONFIG_REQ, sizeof(struct cali_config_tag));

	radio_index = cls_wifi_hw->radio_idx;
	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
		phy_band = CALI_PHY_BAND_5G;
	else if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
		phy_band = CALI_PHY_BAND_2G4;
	g_radio_cls_wifi_hw[radio_index] = cls_wifi_hw;
	pr_err("radio index %d band %d\n", radio_index, phy_band);

	cls_wifi_hw->cal_env->radio_idx = radio_index; // saved for later use
	cali_set_default_config(&g_cal_config[radio_index], phy_band);
	g_cal_config[radio_index].magic = CALI_CFG_MAGIC;

	set_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags);

#if !defined(MERAK2000)
	for (i = 0; i < WPU_REGS_SECTION; i++) {
		struct wpu_regs_map *preg_map = &wpu_regs_map[radio_index][i];

		preg_map->io_base_addr =
				ioremap(preg_map->reg_base_addr, preg_map->reg_size);
		pr_info("WPU reg mapping[0x%x-0x%lx]\n", preg_map->reg_base_addr,
				(unsigned long)preg_map->io_base_addr);
	}
#endif

	/*init cali debugfs*/
	cali_debugfs_init();
	cls_wifi_init_cal_sounding(cls_wifi_hw);
	current_radio = radio_index;
	cls_wifi_csi_init(cls_wifi_hw);

	cls_wifi_cal_fw_init_req(cls_wifi_hw, radio_index);

	/* define bss bw and chan_ieee when start up */
	g_cal_config[radio_index].bss_info.bw =
		(phy_band == CALI_PHY_BAND_2G4) ? cls_wifi_mod_params.bw_2g : cls_wifi_mod_params.bw_5g;
	g_cal_config[radio_index].bss_info.bw = (1 << g_cal_config[radio_index].bss_info.bw) * 20;
	g_cal_config[radio_index].bss_info.chan_ieee =
		(phy_band == CALI_PHY_BAND_2G4) ? cls_wifi_mod_params.chan_ieee_2g : cls_wifi_mod_params.chan_ieee_5g;
	cls_wifi_send_cal_param_config_req(cls_wifi_hw,  radio_index, CAL_PARAM_BASIC,
		sizeof(g_cal_config[0]), &g_cal_config[radio_index], 0);

	return 0;
}

void cls_wifi_cal_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	kfree(cls_wifi_hw->cal_env);
	cali_debugfs_deinit();
}

struct wpu_regs_map *cls_wifi_cal_get_wpu_regs_map(int radio, uint32_t reg_addr)
{
	int i, radio_index;
	uint32_t min_reg_addr;
	uint32_t max_reg_addr;

	for (i = 0; i < MAX_RADIO_NUM; i++) {
		min_reg_addr = wpu_regs_map[i][0].reg_base_addr;
		max_reg_addr =
			wpu_regs_map[i][WPU_REGS_SECTION - 1].reg_base_addr +
			wpu_regs_map[i][WPU_REGS_SECTION - 1].reg_size;
		if ((reg_addr >= min_reg_addr) && (reg_addr < min_reg_addr))
			break;
	}

	if (i == MAX_RADIO_NUM) {
		pr_err("REG address(0x%x) is invalid\n", reg_addr);
		return NULL;
	}
	radio_index = i;

	for (i = 0; i < (WPU_REGS_SECTION - 1); i++) {
		struct wpu_regs_map *preg_map = &wpu_regs_map[radio_index][i + 1];

		if (!preg_map->io_base_addr)
			continue;
		if (reg_addr < preg_map->reg_base_addr) {
			preg_map = &wpu_regs_map[radio_index][i];
			if (reg_addr < (preg_map->reg_base_addr + preg_map->reg_size))
				return preg_map;
			else
				return NULL;
		}
	}

	if (wpu_regs_map[radio_index][i].io_base_addr)
		return &wpu_regs_map[radio_index][i];
	else
		return NULL;
}
#endif
int cls_wifi_send_cal_param_config_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx,
		int type, int param_len,
		void *param, int dma)
{
#ifndef __KERNEL__
	int i;
	struct ipc_shared_env_tag *ipc_shared_env;
#endif
	struct cal_param_config_req req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw || !cls_wifi_hw->cal_env || !param || (param_len <= 0))
		return -EINVAL;

	if (!cls_wifi_hw->ipc_env)
		return -EINVAL;

#ifdef __KERNEL__
	cls_wifi_update_cbf_report(cls_wifi_hw);
#endif
#ifndef __KERNEL__
	ipc_shared_env = cls_wifi_hw->ipc_env->shared;
	if (!ipc_shared_env)
		return -EINVAL;
#endif
	/* Build the CAL_PARAM_CONFIG_REQ message */
	req.radio = radio_idx;
	req.param_type = type;
	req.param_len = param_len;
	req.use_dma = dma;

#if CAL_DBG
	pr_err("radio=%d, param_len=%d, type=%d\n", req.radio,
		req.param_len, req.param_type);
#endif

	/* Build the message's data */
	if (param_len) {
#ifdef __KERNEL__
		if (dma) {
			if (cls_wifi_ipc_buf_a2e_alloc(cls_wifi_hw,
					&cls_wifi_hw->cal_env->cal_param_ipc_buf,
					param_len, param)) {
				dev_err(cls_wifi_hw->dev,
						"Failed to allocate IPC buffer for Calibration parameter(type=%d)\n", type);
				return -ENOMEM;
			}

			req.param_dma_addr = cls_wifi_hw->cal_env->cal_param_ipc_buf.dma_addr;
		} else {
			cls_wifi_hw->ipc_env->ops->writen(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx,
					offsetof(struct ipc_shared_env_tag, data_a2e_buf), param, param_len);
		}
#else
	for (i = 0; i < ((param_len + 3) / 4); i++)
		ipc_shared_env->data_a2e_buf[i] = *((uint32_t *)param + i);
#endif
	}

	return cls_wifi_send_cal_msg_req(cls_wifi_hw,
				CAL_PARAM_CONFIG_REQ, sizeof(req), &req,
				CAL_PARAM_CONFIG_CFM, &cls_wifi_hw->cal_env->cal_param_cfm);
}

int cls_wifi_send_cal_param_update_only_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx,
		int type, int param_len, void *param, int dma)
{
#ifndef __KERNEL__
	int i;
	struct ipc_shared_env_tag *ipc_shared_env;
#endif
	struct cal_param_config_req req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw || !cls_wifi_hw->cal_env || !param || (param_len <= 0))
		return -EINVAL;

	if (!cls_wifi_hw->ipc_env)
		return -EINVAL;

#ifdef __KERNEL__
	cls_wifi_update_cbf_report(cls_wifi_hw);
#endif
#ifndef __KERNEL__
	ipc_shared_env = cls_wifi_hw->ipc_env->shared;
	if (!ipc_shared_env)
		return -EINVAL;
#endif
	/* Build the CAL_PARAM_CONFIG_REQ message */
	req.radio = radio_idx;
	req.param_type = type;
	req.param_len = param_len;
	req.use_dma = dma;

#if CAL_DBG
	pr_err("radio=%d, param_len=%d, type=%d\n", req.radio,
		req.param_len, req.param_type);
#endif

	/* Build the message's data */
	if (param_len) {
#ifdef __KERNEL__
		if (dma) {
			if (cls_wifi_ipc_buf_a2e_alloc(cls_wifi_hw,
					&cls_wifi_hw->cal_env->cal_param_ipc_buf,
					param_len, param)) {
				dev_err(cls_wifi_hw->dev,
					"Failed to allocate IPC buffer for Calibration parameter(type=%d)\n", type);
				return -ENOMEM;
			}

			req.param_dma_addr = cls_wifi_hw->cal_env->cal_param_ipc_buf.dma_addr;
		} else {
			cls_wifi_hw->ipc_env->ops->writen(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx,
					offsetof(struct ipc_shared_env_tag, data_a2e_buf), param, param_len);
		}
#else
	for (i = 0; i < ((param_len + 3) / 4); i++)
		ipc_shared_env->data_a2e_buf[i] = *((uint32_t *)param + i);
#endif
	}

	return cls_wifi_send_cal_msg_req(cls_wifi_hw,
				CAL_PARAM_UPDATE_ONLY_REQ, sizeof(req), &req,
				CAL_PARAM_UPDATE_ONLY_CFM, &cls_wifi_hw->cal_env->cal_param_cfm);
}

int cls_wifi_cal_mem_read(int radio_idx, uint32_t addr, int req_words, void *buf)
{
	if (!buf || !addr)
		return -EINVAL;
	if (radio_idx >= MAX_RADIO_NUM)
		return -EINVAL;
	if ((req_words <= 0) || (req_words > CAL_MEM_OP_RD_LEN_MAX))
		return -EINVAL;

	return cls_wifi_cal_mem_op_req(g_radio_cls_wifi_hw[radio_idx], radio_idx,
			CAL_MEM_OP_RD, addr, req_words, buf);
}

int cls_wifi_cal_mem_write(int radio_idx, uint32_t addr, int req_words, void *buf)
{
	if (!buf || !addr)
		return -EINVAL;
	if (radio_idx >= MAX_RADIO_NUM)
		return -EINVAL;
	if ((req_words <= 0) || (req_words > CAL_MEM_OP_WR_LEN_MAX))
		return -EINVAL;

	return cls_wifi_cal_mem_op_req(g_radio_cls_wifi_hw[radio_idx], radio_idx,
			CAL_MEM_OP_WR, addr, req_words, buf);
}


static int cls_wifi_cal_leaf_timer_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx,
		int op_type, uint32_t *us_lo_ptr, uint32_t *us_hi_ptr, uint32_t *ns_ptr)
{
	int ret;
#ifndef __KERNEL__
	struct ipc_shared_env_tag *ipc_shared_env;
#endif
	struct cal_leaf_timer_req req;
	struct cal_leaf_timer_cfm cfm;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#ifndef __KERNEL__
	if (!cls_wifi_hw->cal_env || !cls_wifi_hw->ipc_env)
		return -EINVAL;

	ipc_shared_env = cls_wifi_hw->ipc_env->shared;
	if (!ipc_shared_env)
		return -EINVAL;
#endif

	/* Build the CAL_MEM_OP_REQ message */
	req.radio = radio_idx;
	req.op_type = op_type;

#if CAL_DBG
	pr_err("%s: radio=%d, get leaf-timer\n", __func__, req.radio);
#endif

	ret = cls_wifi_send_cal_msg_req(cls_wifi_hw, CAL_LEAF_TIMER_GET_REQ, sizeof(req), &req,
			CAL_LEAF_TIMER_GET_CFM, &cfm);
	if (ret) {
		pr_err("%s: ret=%d\n", __func__, ret);
		return ret;
	}

	if (cfm.status == 0) {
		*us_hi_ptr = cfm.us_hi;
		*us_lo_ptr = cfm.us_low;
		*ns_ptr = cfm.ns;
	}

	return 0;
}

int cls_wifi_cal_leaf_timer_read(int radio_idx, uint32_t *us_lo_ptr, uint32_t *us_hi_ptr, uint32_t *ns_ptr)
{
	*us_hi_ptr = 0;
	*us_lo_ptr = 0;
	*ns_ptr = 0;

	if (radio_idx >= MAX_RADIO_NUM)
		return -EINVAL;

	return cls_wifi_cal_leaf_timer_req(g_radio_cls_wifi_hw[radio_idx], radio_idx,
			0, us_lo_ptr, us_hi_ptr, ns_ptr);
}

int cls_wifi_cal_fw_init_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx)
{
	int ret;
#ifndef __KERNEL__
	struct ipc_shared_env_tag *ipc_shared_env;
#endif
	struct cal_ipc_comm_req req;
	struct cal_ipc_comm_cfm cfm;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw || !cls_wifi_hw->cal_env || !cls_wifi_hw->ipc_env)
		return -EINVAL;
#ifndef __KERNEL__
	ipc_shared_env = cls_wifi_hw->ipc_env->shared;
	if (!ipc_shared_env)
		return -EINVAL;
#endif
	/* Build the CAL_MEM_OP_REQ message */
	req.radio = radio_idx;
	req.param_len = 0;

#if CAL_DBG
	pr_err("%s: radio=%d\n", __func__, req.radio);
#endif

	ret = cls_wifi_send_cal_msg_req(cls_wifi_hw, CAL_FW_INIT_REQ, sizeof(req), &req,
			CAL_FW_INIT_CFM, &cfm);
	if (ret) {
		pr_err("%s: ret=%d\n", __func__, ret);
		return ret;
	}

#if CAL_DBG
	pr_err("%s: radio=%d, status=%d\n", __func__, cfm.radio, cfm.status);
#endif
	return 0;
}

static int cls_wifi_rx_cal_param_config_cfm(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_ipc_comm_cfm *cfm = (struct cal_ipc_comm_cfm *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#if CAL_DBG
	pr_err("cal_param_config_cfm: radio=%d, status=%d\n",
		cfm->radio, cfm->status);
#endif
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_recv_cal_rx_stats_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_rx_stats_cfm *cfm = (struct cal_rx_stats_cfm *)msg->param;

	if (cfm->length != sizeof(*cfm)) {
		pr_err("%s: wrong length %u, expect %zu\n", __func__, cfm->length, sizeof(*cfm));
		return -1;
	}

	if (cfm->status < 0) {
		pr_info("%s: status=%d\n", __func__, cfm->status);
		return -1;
	}

	cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->ipc_env->plat, cls_wifi_hw->ipc_env->radio_idx,
			offsetof(struct ipc_shared_env_tag, data_e2a_buf), &g_rx_stats, sizeof(g_rx_stats));
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_send_cal_rx_status_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_rx_status_cfm *cfm = (struct cal_rx_status_cfm *)msg->param;
	uint32_t rx_mpdu_len;
	u8 *rx_mpdu;
#if !defined(MERAK2000)
	int ret;
#endif

	if (cfm->length != sizeof(*cfm)) {
		pr_err("%s: wrong length %u, expect %zu\n", __func__, cfm->length, sizeof(*cfm));
		return -1;
	}

	if (cfm->status < 0) {
		pr_info("%s: status=%d\n", __func__, cfm->status);
		return -1;
	}

	rx_mpdu_len = cls_wifi_hw->ipc_env->ops->read32(cls_wifi_hw->ipc_env->plat, cls_wifi_hw->ipc_env->radio_idx,
			(offsetof(struct ipc_shared_env_tag, data_e2a_buf) +
			offsetof(struct cal_per_radio_status, mac_status.rx_mpdu_len)));
	if (rx_mpdu_len) {
		rx_mpdu = kzalloc(rx_mpdu_len, GFP_KERNEL);
		cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->ipc_env->plat, cls_wifi_hw->ipc_env->radio_idx,
			offsetof(struct ipc_shared_env_tag, data_e2a_buf) +
			offsetof(struct cal_per_radio_status, mac_status.rx_mpdu),
			rx_mpdu, rx_mpdu_len);
#if defined(MERAK2000)
		cls_wifi_hw->cal_file.rx_mpdu = rx_mpdu;
		cls_wifi_hw->cal_file.rx_mpdu_len = rx_mpdu_len;
		schedule_work(&cls_wifi_hw->cal_file.rx_last_mpdu_save_bin_work);
#else
		ret = cls_wifi_save_buf_to_file(RX_LAST_MPDU_FILE, rx_mpdu, rx_mpdu_len);
		kfree(rx_mpdu);
		if (ret < 0)
			pr_err("%s: fail to save last MPDU as %s\n", __func__, RX_LAST_MPDU_FILE);
#endif
	}
	cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->ipc_env->plat, cls_wifi_hw->ipc_env->radio_idx,
			offsetof(struct ipc_shared_env_tag, data_e2a_buf), &g_rx_status, sizeof(g_rx_status));
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_recv_cal_tx_su_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_tx_su_cfm *cfm = (struct cal_tx_su_cfm *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_info("cal_tx_su_cfm: %d\n", cfm->status);
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_recv_cal_tx_su_res_ind(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_tx_su_res_ind *ind = (struct cal_tx_su_res_ind *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_info("cal_tx_su_res_ind: %d\n", ind->txstatus);
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_recv_cal_tx_stats_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_tx_stats_cfm *cfm = (struct cal_tx_stats_cfm *)msg->param;

	if (cfm->length != sizeof(*cfm)) {
		pr_err("%s: wrong length %u, expect %zu\n", __func__, cfm->length, sizeof(*cfm));
		return -1;
	}

	if (cfm->status < 0) {
		pr_info("%s: status=%d\n", __func__, cfm->status);
		return -1;
	}

	cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->ipc_env->plat, cls_wifi_hw->ipc_env->radio_idx,
			offsetof(struct ipc_shared_env_tag, data_e2a_buf), &g_tx_stats, sizeof(g_tx_stats));
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_recv_cal_tx_mu_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_tx_mu_cfm *cfm = (struct cal_tx_mu_cfm *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_info("cal_tx_mu_cfm: %d\n", cfm->status);
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_recv_cal_sounding_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_sounding_cfm *cfm = (struct cal_sounding_cfm *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_info("cal_sounding_cfm: %d\n", cfm->status);
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}


static int cls_wifi_recv_cal_log_set_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_log_set_cfm *cfm = (struct cal_log_set_cfm *)msg->param;

#if CAL_DBG
	pr_info("%s: status=%d\n", __func__, cfm->status);
#endif
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}

static int cls_wifi_send_cal_get_csi_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_get_csi_cfm *cfm = (struct cal_get_csi_cfm *)msg->param;
	struct cls_csi_report *report;
	uint32_t csi_bank_len;
	uint8_t *rx_csi_iq;
	uint8_t *rx_csi;
	int ret;

	if (cfm->length != sizeof(*cfm)) {
		pr_err("%s: wrong length %u, expect %zu\n", __func__, cfm->length, sizeof(*cfm));

		return 0;
	}

	if (cfm->status < 0) {
		pr_info("%s: status=%d\n", __func__, cfm->status);

		return 0;
	}

	if (cfm->buff_len > sizeof(struct cls_csi_report)) {
		pr_info("%s buff_len %u error\n", __func__, cfm->buff_len);

		return 0;
	}

	rx_csi = kzalloc(cfm->buff_len, GFP_KERNEL);
	cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->ipc_env->plat,
		cls_wifi_hw->ipc_env->radio_idx,
		(offsetof(struct ipc_shared_env_tag, data_e2a_buf)),
		rx_csi, cfm->buff_len);

	memcpy(&g_cali_csi_report, rx_csi, sizeof(struct cls_csi_report));
	report = (struct cls_csi_report *)rx_csi;
	if (report->hdr.csi_length == 0) {
		pr_err("%s csi_length %u error\n", __func__, report->hdr.csi_length);

		return 0;
	}

	csi_bank_len = report->hdr.csi_length * 4;
	rx_csi_iq = rx_csi + offsetof(struct cls_csi_report, csi_iq);
	ret = cls_wifi_save_buf_to_file(RX_LAST_CSI_FILE, rx_csi_iq, csi_bank_len);
	kfree(rx_csi);
	if (ret < 0)
		pr_err("%s: fail to save last CSI as %s\n", __func__, RX_LAST_CSI_FILE);
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}


static int cls_wifi_recv_cal_log_get_cfm(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
#ifdef __KERNEL__
	struct cal_log_get_cfm *cfm = (struct cal_log_get_cfm *)msg->param;

#if CAL_DBG
	pr_info("%s: status=%d\n", __func__, cfm->status);
#endif
#else
#if CAL_DBG
	yc_printf(CLS_WIFI_FN_ENTRY_STR);
#endif
#endif
	return 0;
}
#ifdef __KERNEL__
char cbf_file_name[256];
static char *cls_wifi_cali_get_cbf_report_file(struct cls_wifi_hw *cls_wifi_hw,
		struct cali_per_user_mac_phy_params_tag *phy_param, uint8_t user_idx)
{
	uint8_t radio;
	uint8_t nb_user;
	uint32_t format_mod;
	char *fname;
	uint32_t bw;
	int is_mu = 0;
	uint8_t antenna_set;

	fname = &cbf_file_name[0];
	memset(fname, 0, sizeof(cbf_file_name));
	radio = cls_wifi_hw->cal_env->radio_idx;
	nb_user = g_cal_config[radio].mu_users;
	format_mod = g_cal_config[radio].mac_phy_params.format_mod;
	bw = g_cal_config[radio].mac_phy_params.bw_ppdu;
	antenna_set = g_cal_config[radio].mac_phy_params.antenna_set;

	pr_info("%s bw %u users %u foramt %u nss mcs %02x antenna %x\n",
		__func__, bw, nb_user, format_mod, phy_param->mcs, antenna_set);
	fname += sprintf(fname, "%s", "/root/cbf/m2k/");
	if (antenna_set == 7)
		fname += sprintf(fname, "%s", "3T/");
	else
		fname += sprintf(fname, "%s", "2T/");

	fname += sprintf(fname, "%s", "cbf");
	if (format_mod == CALI_FORMAT_MOD_VHT) {
		fname += sprintf(fname, "%s", "_vht");
	} else if (format_mod == CALI_FORMAT_MOD_HE_MU
		|| format_mod == CALI_FORMAT_MOD_HE_SU) {
		fname += sprintf(fname, "%s", "_he");
#if defined(CFG_MERAK3000)
	} else if (format_mod == CALI_FORMAT_MOD_EHT_MU) {
		fname += sprintf(fname, "%s", "_eht");
#endif
	}else {
		pr_err("******* Error unsupported FORMAT_MOD **********\n");

		return NULL;
	}

	if ((nb_user > 1) && (g_cal_config[radio].mu_info.mu_type == 1)) {
		/* MU-MIMO */
		fname += sprintf(fname, "%s", "_mu");
		is_mu = 1;
	} else {
		/* SU, OFDMA per user */
		fname += sprintf(fname, "%s", "_su");
	}

	fname += sprintf(fname, "%s", "_bw");
	fname += sprintf(fname, "%d", bw);
	if (is_mu)
		fname += sprintf(fname, "_u%d", (user_idx + 1));

	if (phy_param->mcs & 0xF0) {
		/* 2SS */
		fname += sprintf(fname, "%s", "_2ss");
	}

	fname += sprintf(fname, "%s", ".dat");
	fname = &cbf_file_name[0];
	pr_info("%s file name %s", __func__, fname);

	return fname;
}

static int cls_wifi_cali_read_cbf_report(struct cls_wifi_hw *cls_wifi_hw, char *file_name, uint8_t user_idx)
{
	struct cal_bf_req *bf_req;
	uint32_t *report_addr;
	unsigned long res = 0;
	uint32_t file_len;
	struct file *fp;
	char tmp_str[9] = {0};
	char delimiter;
	int len;
	int i;

	bf_req = &cls_wifi_hw->cal_env->tx_bf_req;
	if (bf_req  == NULL) {
		pr_err("bf_req: %p\n", report_addr);

		return -1;
	}

	fp = filp_open(file_name, O_RDONLY, 0666);
	if (IS_ERR(fp)) {
		pr_err("%s ***** open file %s error ********\n", __func__, file_name);

		return -1;
	}

	file_len = fp->f_inode->i_size;
	len = (file_len + sizeof(tmp_str) - 1)/sizeof(tmp_str);
	report_addr = (uint32_t *)bf_req->host_report_addr[user_idx];
	if (report_addr  == NULL) {
		pr_err("report_addr: %px\n", report_addr);

		goto failed;
	}

	for (i = 0; i < len; i++) {
		kernel_read(fp, tmp_str, sizeof(tmp_str) - 1, &fp->f_pos);
		if (kstrtoul(tmp_str, 16, &res))
			break;

		*(report_addr + i) = res;
		kernel_read(fp, &delimiter, 1, &fp->f_pos);
	}

	bf_req->host_report_len[user_idx] = len * 4;
	pr_info("CBF user %u file_len %u len %u\n", user_idx, file_len, bf_req->host_report_len[user_idx]);

failed:
	filp_close(fp, NULL);

	return 0;
}

static int cls_wifi_update_cbf_report(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cal_bf_req *bf_req;
	int beamformed;
	uint8_t radio;
	int ret;
	int i;
	uint8_t nb_user;
	char *cbf_file;
	int is_mu = 1;
	struct cali_per_user_mac_phy_params_tag *phy_param;
#if !defined(CFG_MERAK3000)
	void *dst;
#endif
	uint32_t *src;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	if (!cls_wifi_hw || !cls_wifi_hw->cal_env)
		return -EINVAL;

	bf_req = &cls_wifi_hw->cal_env->tx_bf_req;
	if (bf_req  == NULL) {
		pr_err("bf_req is NULL\n");

		return -1;
	}

	radio = cls_wifi_hw->cal_env->radio_idx;
	beamformed = g_cal_config[radio].mac_phy_params.beamformed;
	nb_user = g_cal_config[radio].mu_users;
	if (!beamformed)
		return 0;

	if (nb_user <= 1) {
		is_mu = 0;
		nb_user = 1;
	}

	pr_info("radio %u beamformed %u nb_user %u\n", radio, beamformed, nb_user);
	for (i = 0; i < nb_user; i++) {
		if (is_mu)
			phy_param = &g_cal_config[radio].mu_mac_phy_params[i];
		else
			phy_param = &g_cal_config[radio].su_mac_phy_params;

		src = bf_req->host_report_addr[i];
#if !defined(CFG_MERAK3000)
		dst = &phy_param->cbf_report[0];
#endif
		/* Get CBF report name */
		cbf_file = cls_wifi_cali_get_cbf_report_file(cls_wifi_hw, phy_param, i);
		if (cbf_file == NULL)
			return -1;

		ret = cls_wifi_cali_read_cbf_report(cls_wifi_hw, cbf_file, i);
		if (ret < 0)
			return ret;


#if defined(CFG_MERAK3000)
		if (src == NULL)
#else
		pr_info("dst %px src %px len %u", dst, src, bf_req->host_report_len[i]);
		if (dst == NULL || src == NULL)
#endif
			return -1;

		if (bf_req->host_report_len[i] == 0)
			return -1;

		if (bf_req->host_report_len[i] > CALI_MAX_CBF_REPORT_SIE)
			bf_req->host_report_len[i] = CALI_MAX_CBF_REPORT_SIE;

		if (bf_req->dma_addr[i])
			dma_unmap_single(cls_wifi_hw->dev, bf_req->dma_addr[i],
				CALI_MAX_CBF_REPORT_SIE, DMA_TO_DEVICE);

		bf_req->dma_addr[i] = dma_map_single(cls_wifi_hw->dev, bf_req->host_report_addr[i],
			CALI_MAX_CBF_REPORT_SIE, DMA_TO_DEVICE);
		if (dma_mapping_error(cls_wifi_hw->dev, bf_req->dma_addr[i])) {
			pr_warn("%s %d failed on DMA mapping\n", __func__, __LINE__);

			return -EIO;
		}

		phy_param->cbf_report_len = bf_req->host_report_len[i];
		phy_param->cbf_host_addr = bf_req->dma_addr[i];
#if !defined(CFG_MERAK3000)
		memcpy(dst, src, bf_req->host_report_len[i]);
#endif
		pr_info("%s user %u dma addr 0x%08x cbf_host_addr 0x%08x report size %u\n",
			__func__, i, bf_req->dma_addr[i],
			phy_param->cbf_host_addr, bf_req->host_report_len[i]);
	}

	return 0;
}

#endif
static int cls_wifi_init_cal_sounding(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cal_sounding_req *snd_req;
	struct cal_bf_req *bf_req;
	uint32_t cali_cbf_size = CLS_WIFI_BFMER_REPORT_MAX_LEN;
	int i;
	uint8_t radio;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#ifndef __KERNEL__
	if (!cls_wifi_hw)
#else
	if (!cls_wifi_hw || !cls_wifi_hw->cal_env)
#endif
		return -EINVAL;

#ifndef CFG_CHIP_SHELL
	radio = cls_wifi_hw->cal_env->radio_idx;
	/* sounding related CBF */
	snd_req = &cls_wifi_hw->cal_env->tx_sounding_req;
	for (i = 0; i < CLS_MU_USER_MAX; i++) {
		snd_req->host_report_addr[i] = kzalloc(cali_cbf_size, GFP_KERNEL);
		snd_req->dma_addr[i] = dma_map_single(cls_wifi_hw->dev, snd_req->host_report_addr[i],
			cali_cbf_size, DMA_FROM_DEVICE);
	}

	/* beamforming related CBF */
	bf_req = &cls_wifi_hw->cal_env->tx_bf_req;
	for (i = 0; i < CLS_MU_USER_MAX; i++) {
		bf_req->host_report_addr[i] = kzalloc(cali_cbf_size, GFP_KERNEL);
		memset(bf_req->host_report_addr[i], 0xA5, cali_cbf_size);
	}
#endif

	return 0;
}
static int cls_wifi_cal_mem_op_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx,
		int op_type, uint32_t addr, int req_words, void *buf)
{
	int ret;
#ifndef __KERNEL__
	struct ipc_shared_env_tag *ipc_shared_env;
#endif
	struct cal_mem_op_req req;
	struct cal_mem_op_cfm cfm;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#ifndef __KERNEL__
	if (!cls_wifi_hw->cal_env || !cls_wifi_hw->ipc_env)
		return -EINVAL;

	ipc_shared_env = cls_wifi_hw->ipc_env->shared;
	if (!ipc_shared_env)
		return -EINVAL;
#endif

	/* Build the CAL_MEM_OP_REQ message */
	req.radio = radio_idx;
	req.op_type = op_type;
	req.mem_op_addr = addr;
	req.mem_op_len = req_words;
	if (req.op_type == CAL_MEM_OP_WR)
		memcpy(req.buf, buf, req_words * 4);

#if CAL_DBG
	pr_err("%s: radio=%d, type=%d, len=%d, addr=0x%x\n", __func__, req.radio,
		req.op_type, req.mem_op_len, req.mem_op_addr);
#endif

	ret = cls_wifi_send_cal_msg_req(cls_wifi_hw, CAL_MEM_OP_REQ, sizeof(req), &req,
			CAL_MEM_OP_CFM, &cfm);
	if (ret) {
		pr_err("%s: ret=%d\n", __func__, ret);
		return ret;
	}

	if (req.op_type == CAL_MEM_OP_RD) {
#ifdef __KERNEL__
		cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->ipc_env->plat,
			cls_wifi_hw->ipc_env->radio_idx,
			(offsetof(struct ipc_shared_env_tag, data_e2a_buf)),
			buf, (req_words * 4));
#else
		memcpy(buf, (void *)ipc_shared_env->data_e2a_buf, req_words * 4);
#endif
	}
	return 0;
}
#ifdef __KERNEL__
int cls_wifi_save_buf_to_file(const char *filename, const uint8_t *buf, uint32_t buf_len)
{
	struct file *fp;
	loff_t pos = 0;
	ssize_t len;

	fp = filp_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (IS_ERR(fp))
		return -EPERM;

	len = kernel_write(fp, buf, buf_len, &pos);
	if (len < buf_len)
		pr_warn("%s: warning: only %zd/%u written\n", __func__, len, buf_len);

	filp_close(fp, NULL);
	return 0;
}

int cls_wifi_save_radar_int_detect_result_file(const char *filename, const uint32_t *buf, uint32_t buf_len)
{
	struct file *fp;
	char tmp_str[11] = {0};
	u32 i;

	fp = filp_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (IS_ERR(fp))
		return -EPERM;

	for (i = 0; i < buf_len; i++) {
		sprintf(tmp_str, "%08x\r\n", buf[i]);
		kernel_write(fp, tmp_str, strlen(tmp_str), &fp->f_pos);
	}

	filp_close(fp, NULL);
	return 0;
}

int cls_wifi_test_uncache_rw(struct cls_wifi_hw *cls_wifi_hw,
		struct cal_test_uncache_rw_req *req)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	uint32_t buff_offset = offsetof(struct ipc_shared_env_tag, data_a2e_buf);
	uint32_t info_offset = buff_offset +
		40000 +	/* data_a2e_buf */
		40000 -	/* data_e2a_buf */
		sizeof(struct cls_wifi_uncache_test_info);
	uint32_t idx;
	uint32_t len = req->len >> 2;
	uint32_t val;

	/* Initialize buf */
	for (idx = 0; idx < len; idx++) {
		cls_wifi_plat->ep_ops->write32(cls_wifi_plat,
			cls_wifi_hw->radio_idx,
			buff_offset,
			idx);
		buff_offset += 4;
	}
	// Write len
	cls_wifi_plat->ep_ops->write32(cls_wifi_plat,
		cls_wifi_hw->radio_idx,
		info_offset,
		req->len);
	// Write state
	cls_wifi_plat->ep_ops->write32(cls_wifi_plat,
		cls_wifi_hw->radio_idx,
		info_offset + 4,
		CLS_WIFI_UC_TEST_STATE_LHOST_DONE);
	// Wait for WPU to clear buf
	do {
		val = cls_wifi_plat->ep_ops->read32(cls_wifi_plat,
			cls_wifi_hw->radio_idx,
			info_offset + 4);
	} while (val != CLS_WIFI_UC_TEST_STATE_WPU_DONE);
	return 0;
}

static void cls_wifi_rx_last_mpdu_save_bin_file(struct work_struct *ws)
{
	struct cls_wifi_cal_file *param = container_of(ws, struct cls_wifi_cal_file,
					rx_last_mpdu_save_bin_work);

	spin_lock_bh(&param->rx_last_mpdu_save_bin_lock);

	if (!param->rx_mpdu || !param->rx_mpdu_len)
		goto out;

	cls_wifi_save_buf_to_file(RX_LAST_MPDU_FILE, param->rx_mpdu, param->rx_mpdu_len);

	kfree(param->rx_mpdu);
	param->rx_mpdu_len = 0;
out:
	spin_unlock_bh(&param->rx_last_mpdu_save_bin_lock);
}

void cls_wifi_cal_save_work_init(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_cal_file *cal_file = &cls_wifi_hw->cal_file;

	spin_lock_init(&cal_file->rx_last_mpdu_save_bin_lock);

	INIT_WORK(&cal_file->rx_last_mpdu_save_bin_work, cls_wifi_rx_last_mpdu_save_bin_file);
}
EXPORT_SYMBOL(cls_wifi_cal_save_work_init);

void cls_wifi_cal_save_work_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_cal_file *cal_file = &cls_wifi_hw->cal_file;

	cancel_work_sync(&cal_file->rx_last_mpdu_save_bin_work);
}
EXPORT_SYMBOL(cls_wifi_cal_save_work_deinit);
#endif

