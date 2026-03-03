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

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/delay.h>

#include "cls_wifi_platform.h"
#include "hal_desc.h"
#include "cls_wifi_main.h"
#include "ipc_host.h"

/**
 * rwmx_platform_save_config() - Save hardware config before reload
 *
 * @cls_wifi_plat: Pointer to platform data
 *
 * Return configuration registers values.
 */
static void* cls_wifi_platform_save_config(struct cls_wifi_hw *cls_wifi_hw)
{
	const u32 *reg_list;
	u32 *reg_value, *res;
	int i, size = 0;

	if (cls_wifi_hw->plat->if_ops->get_config_reg) {
		size = cls_wifi_hw->plat->if_ops->get_config_reg(cls_wifi_hw->plat, &reg_list);
	}

	if (size <= 0)
		return NULL;

	res = kmalloc(sizeof(u32) * size, GFP_KERNEL);
	if (!res)
		return NULL;

	reg_value = res;
	for (i = 0; i < size; i++) {
		*reg_value++ = cls_wifi_hw->plat->ep_ops->sys_read32(cls_wifi_hw->plat,
				cls_wifi_hw->radio_idx, *reg_list++);
	}

	return res;
}

/**
 * rwmx_platform_restore_config() - Restore hardware config after reload
 *
 * @cls_wifi_plat: Pointer to platform data
 * @reg_value: Pointer of value to restore
 * (obtained with rwmx_platform_save_config())
 *
 * Restore configuration registers value.
 */
static void cls_wifi_platform_restore_config(struct cls_wifi_hw *cls_wifi_hw,
									 u32 *reg_value)
{
	const u32 *reg_list;
	int i, size = 0;

	if (!reg_value || !cls_wifi_hw->plat->if_ops->get_config_reg)
		return;

	size = cls_wifi_hw->plat->if_ops->get_config_reg(cls_wifi_hw->plat, &reg_list);

	for (i = 0; i < size; i++) {
		cls_wifi_hw->plat->ep_ops->sys_write32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				*reg_list++, *reg_value++);
	}
}

static int cls_wifi_check_fw_compatibility(struct cls_wifi_hw *cls_wifi_hw)
{
	//struct ipc_shared_env_tag *shared = cls_wifi_hw->ipc_env->shared;
	struct wiphy *wiphy = cls_wifi_hw->wiphy;
	int ipc_shared_version = 50;
	int res = 0;

	struct compatibility_tag comp_info; //FW characteristics

	cls_wifi_hw->ipc_env->ops->readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx, \
		offsetof(struct ipc_shared_env_tag, comp_info), &comp_info, sizeof(comp_info));

	if (comp_info.ipc_shared_version != ipc_shared_version)
	{
		wiphy_err(wiphy, "Different versions of IPC shared version between driver and FW (%d != %d)\n ",
				  ipc_shared_version, comp_info.ipc_shared_version);
		res = -1;
	}

	if (comp_info.radarbuf_cnt != IPC_RADARBUF_CNT)
	{
		wiphy_err(wiphy, "Different number of host buffers available for Radar events handling "\
				  "between driver and FW (%d != %d)\n", IPC_RADARBUF_CNT,
				  comp_info.radarbuf_cnt);
		res = -1;
	}

	if (comp_info.hemubuf_cnt != IPC_HEMUBUF_CNT)
	{
		wiphy_err(wiphy, "Different number of host buffers available for HE MU Map events handling "\
				  "between driver and FW (%d != %d)\n", IPC_HEMUBUF_CNT,
				  comp_info.hemubuf_cnt);
		res = -1;
	}

	if (comp_info.unsuprxvecbuf_cnt != IPC_UNSUPRXVECBUF_CNT)
	{
		wiphy_err(wiphy, "Different number of host buffers available for unsupported Rx vectors "\
				  "handling between driver and FW (%d != %d)\n", IPC_UNSUPRXVECBUF_CNT,
				  comp_info.unsuprxvecbuf_cnt);
		res = -1;
	}

	if (comp_info.rxdesc_cnt != cls_wifi_hw->ipc_env->rxdesc_nb)
	{
		wiphy_err(wiphy, "Different number of shared descriptors available for Data RX handling "\
				  "between driver and FW (%d != %d)\n",
				  cls_wifi_hw->ipc_env->rxdesc_nb,
				  comp_info.rxdesc_cnt);
		res = -1;
	}

	if (comp_info.rxbuf_cnt != cls_wifi_hw->ipc_env->rxbuf_nb)
	{
		wiphy_err(wiphy, "Different number of host buffers available for Data Rx handling "\
				  "between driver and FW (%d != %d)\n",
				  cls_wifi_hw->ipc_env->rxbuf_nb,
				  comp_info.rxbuf_cnt);
		res = -1;
	}

	if (comp_info.msge2a_buf_cnt != cls_wifi_hw->ipc_env->e2amsg_nb)
	{
		wiphy_err(wiphy, "Different number of host buffers available for Emb->App MSGs "\
				  "sending between driver and FW (%d != %d)\n",
				  cls_wifi_hw->ipc_env->e2amsg_nb,
				  comp_info.msge2a_buf_cnt);
		res = -1;
	}

	if (comp_info.dbgbuf_cnt != cls_wifi_hw->ipc_env->dbgbuf_nb)
	{
		wiphy_err(wiphy, "Different number of host buffers available for debug messages "\
				  "sending between driver and FW (%d != %d)\n",
				  cls_wifi_hw->ipc_env->dbgbuf_nb,
				  comp_info.dbgbuf_cnt);
		res = -1;
	}

	if (comp_info.bk_txq != cls_wifi_hw->plat->hw_params.txdesc_cnt0[cls_wifi_hw->radio_idx])
	{
		wiphy_err(wiphy, "Driver and FW have different sizes of BK TX queue (%d != %d)\n",
				  cls_wifi_hw->plat->hw_params.txdesc_cnt0[cls_wifi_hw->radio_idx],
				  comp_info.bk_txq);
		res = -1;
	}

	if (comp_info.be_txq != cls_wifi_hw->plat->hw_params.txdesc_cnt1[cls_wifi_hw->radio_idx])
	{
		wiphy_err(wiphy, "Driver and FW have different sizes of BE TX queue (%d != %d)\n",
				  cls_wifi_hw->plat->hw_params.txdesc_cnt1[cls_wifi_hw->radio_idx],
				  comp_info.be_txq);
		res = -1;
	}

	if (comp_info.vi_txq != cls_wifi_hw->plat->hw_params.txdesc_cnt2[cls_wifi_hw->radio_idx])
	{
		wiphy_err(wiphy, "Driver and FW have different sizes of VI TX queue (%d != %d)\n",
				  cls_wifi_hw->plat->hw_params.txdesc_cnt2[cls_wifi_hw->radio_idx],
				  comp_info.vi_txq);
		res = -1;
	}

	if (comp_info.vo_txq != cls_wifi_hw->plat->hw_params.txdesc_cnt3[cls_wifi_hw->radio_idx])
	{
		wiphy_err(wiphy, "Driver and FW have different sizes of VO TX queue (%d != %d)\n",
				  cls_wifi_hw->plat->hw_params.txdesc_cnt3[cls_wifi_hw->radio_idx],
				  comp_info.vo_txq);
		res = -1;
	}

	#if CLS_TXQ_CNT == 5
	if (comp_info.bcn_txq != cls_wifi_hw->plat->hw_params.txdesc_cnt4[cls_wifi_hw->radio_idx])
	{
		wiphy_err(wiphy, "Driver and FW have different sizes of BCN TX queue (%d != %d)\n",
				cls_wifi_hw->plat->hw_params.txdesc_cnt4[cls_wifi_hw->radio_idx],
				comp_info.bcn_txq);
		res = -1;
	}
	#else
	if (comp_info.bcn_txq > 0)
	{
		wiphy_err(wiphy, "BCMC enabled in firmware but disabled in driver\n");
		res = -1;
	}
	#endif /* CLS_TXQ_CNT == 5 */

	if (comp_info.ipc_shared_size != cls_wifi_hw->ipc_env->ipc_shared_size)
	{
		wiphy_err(wiphy, "Different sizes of IPC shared between driver and FW (%d != %d)\n",
				  cls_wifi_hw->ipc_env->ipc_shared_size, comp_info.ipc_shared_size);
		res = -1;
	}

	if (comp_info.msg_api != MSG_API_VER)
	{
		wiphy_err(wiphy, "Different supported message API versions between "\
				  "driver and FW (%d != %d)\n", MSG_API_VER, comp_info.msg_api);
		res = -1;
	}

	if (comp_info.amsdu_tx_offload_phase != CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD) {
		wiphy_err(wiphy, "Different phase of A-MSDU Tx Offload feature between driver and FW (%d != %d)\n",
				  CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD, comp_info.amsdu_tx_offload_phase);
		if (!cls_wifi_hw->radio_params->debug_mode)
			res = -1;
	} else {
		wiphy_err(wiphy, "A-MSDU Tx Offload feature phase %d\n",
				  CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD);
	}

	if (res == 0) {
		if (comp_info.ipc_flag & 0x1)
			cls_wifi_hw->ipc_env->pp_valid = true;
		else
			cls_wifi_hw->ipc_env->pp_valid = false;
	}

	return res;
}

int cls_wifi_wait_wpu_init_ipc_env(struct cls_wifi_hw *cls_wifi_hw, uint32_t timeout)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	uint64_t wait_time = 0;
	uint64_t wait_timeout = timeout;
	u32 wpu_ipc_pattern;
	int ret = 0;
	pr_warn(">>> %s: %d, wait_timeout:%llu(%d)\n", __func__, __LINE__, wait_timeout,timeout);

	while(1)
	{
		wpu_ipc_pattern = ipc_host_wpu_ipc_pattern_get(cls_wifi_plat, cls_wifi_hw->radio_idx);
		if((wpu_ipc_pattern != ((uint32_t)WPU_IPC_PATTERN_MAGIC))
			 && (wpu_ipc_pattern != ((uint32_t)WPU_IPC_INIT_PATTERN_MAGIC))){
#ifdef CONFIG_CLS_EMU_ADAPTER
			msleep(10);
			wait_time += 10;
#else
			msleep(1000);
			wait_time += 1000;
#endif
			pr_warn(">>> %s: %d, wait_time:%llu,wpu_ipc_pattern: %08x,ret:%d\n", __func__, __LINE__, wait_time,wpu_ipc_pattern,ret);

			if(wait_time < wait_timeout){
				continue;
			}else{
				if((wpu_ipc_pattern != ((uint32_t)WPU_IPC_PATTERN_MAGIC))
					&& (wpu_ipc_pattern != ((uint32_t)WPU_IPC_INIT_PATTERN_MAGIC))){
					ret = -1;
					pr_warn(">>> %s: %d, timeout:%llu\n", __func__, __LINE__, wait_time);
				}
				pr_warn(">>> %s: %d, wait_time:%llu(%llu),wpu_ipc_pattern: %08x,ret:%d\n", __func__, __LINE__, wait_time,wait_timeout,wpu_ipc_pattern,ret);
				break;
			}

		}else{
			pr_warn(">>> %s: %d, wait_time:%llu(%llu),wpu_ipc_pattern: %08x,ret:%d\n", __func__, __LINE__, wait_time,wait_timeout,wpu_ipc_pattern,ret);
			break;
		}
	}

	pr_warn(">>> %s: %d, wait_time:%llu(%llu),wpu_ipc_pattern: %08x,ret:%d\n", __func__, __LINE__, wait_time,wait_timeout,wpu_ipc_pattern,ret);

	return ret;
}

/**
 * cls_wifi_platform_on() - Start the platform
 *
 * @cls_wifi_hw: Main driver data
 * @config: Config to restore (NULL if nothing to restore)
 *
 * It starts the platform :
 * - load fw and ucodes
 * - initialize IPC
 * - boot the fw
 * - enable link communication/IRQ
 *
 * Called by 802.11 part
 */
int cls_wifi_platform_on(struct cls_wifi_hw *cls_wifi_hw, void *config)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	int ret;
	int retry = 0;

	if (cls_wifi_hw->enabled)
		return 0;

	/* By default, we consider that there is only one RF in the system */
	cls_wifi_hw->phy.cnt = 1;

	ret = cls_wifi_ipc_init(cls_wifi_hw, 0);
	if (ret)
		return ret;

	if ((ret = cls_wifi_plat->if_ops->enable(cls_wifi_hw)))
		return ret;
#if defined(MERAK2000) && MERAK2000
	cls_wifi_fw_trace_config_filters(cls_wifi_hw, cls_wifi_get_shared_trace_buf(cls_wifi_hw),
			cls_wifi_hw->radio_params->ftl);
#endif
	cls_wifi_wait_wpu_init_ipc_env(cls_wifi_hw, 60 * 1000);
	while ((ret = cls_wifi_check_fw_compatibility(cls_wifi_hw)))
	{
#ifdef CONFIG_CLS_EMU_ADAPTER
		msleep(10);
#else
		msleep(1000);
#endif
		barrier();
		retry++;
		pr_warn("\n--- %s: %d, comp_info error, retry %d\n", __func__, __LINE__, retry);

		if(retry > 30)
		{
			cls_wifi_hw->plat->if_ops->disable(cls_wifi_hw);
			cls_wifi_ipc_deinit(cls_wifi_hw);
			pr_warn("\n--- %s: %d, comp_info too many errors\n", __func__, __LINE__);
			return ret;
		}
	}
	pr_warn("\n--- %s: %d, comp_info done, retry %d\n", __func__, __LINE__, retry);

	if (config)
		cls_wifi_platform_restore_config(cls_wifi_hw, config);

	cls_wifi_ipc_start(cls_wifi_hw);

	cls_wifi_hw->enabled = true;

	return 0;
}

/**
 * cls_wifi_cmn_platform_on() - Start the platform
 *
 * @cls_wifi_hw: Main driver data
 * @config: Config to restore (NULL if nothing to restore)
 *
 * It starts the platform :
 * - load fw and ucodes
 * - initialize IPC
 * - boot the fw
 * - enable link communication/IRQ
 *
 * Called by 802.11 part
 */
int cls_wifi_cmn_platform_on(struct cls_wifi_plat *cls_wifi_plat, void *config)
{
	int ret;
	struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_plat->cmn_hw;
	struct ipc_host_cmn_env_tag *ipc_host_cmn_env;

	if (cmn_hw->enabled)
		return 0;

	ipc_host_cmn_env = cmn_hw->ipc_host_cmn_env;
	ipc_host_cmn_env->ops = cls_wifi_plat->ep_ops;
	ipc_host_cmn_env->dev = cls_wifi_plat->dev;
	ipc_host_cmn_env->radio_idx = cmn_hw->radio_idx;
	if ((ret = cls_wifi_cmn_ipc_init(cls_wifi_plat, ipc_host_cmn_env)))
		return ret;

	//if ((ret = cls_wifi_plat->if_ops->enable(cls_wifi_hw)))
	//	return ret;

	cmn_hw->enabled = true;

	return 0;
}

/**
 * cls_wifi_cmn_platform_off() - Stop the platform
 *
 * @cls_wifi_hw: Main driver data
 *
 * It stops the platform :
 * - deinitialize IPC
 *
 * Called by 802.11 part
 */
void cls_wifi_cmn_platform_off(struct cls_wifi_plat *cls_wifi_plat)
{
	struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_plat->cmn_hw;
	struct ipc_host_cmn_env_tag *ipc_host_cmn_env;

	if (!cmn_hw->enabled)
		return;

	ipc_host_cmn_env = cmn_hw->ipc_host_cmn_env;
	ipc_host_cmn_env->ops = NULL;
	ipc_host_cmn_env->dev = NULL;
	cls_wifi_cmn_ipc_deinit(cls_wifi_plat);

	cmn_hw->enabled = false;
}


/**
 * cls_wifi_platform_off() - Stop the platform
 *
 * @cls_wifi_hw: Main driver data
 * @config: Updated with pointer to config, to be able to restore it with
 * cls_wifi_platform_on(). It's up to the caller to free the config. Set to NULL
 * if configuration is not needed.
 *
 * Called by 802.11 part
 */
void cls_wifi_platform_off(struct cls_wifi_hw *cls_wifi_hw, void **config)
{
	if (!cls_wifi_hw->enabled) {
		if (config)
			*config = NULL;
		return;
	}

	cls_wifi_ipc_stop(cls_wifi_hw);

	if (config)
		*config = cls_wifi_platform_save_config(cls_wifi_hw);

	cls_wifi_hw->plat->if_ops->disable(cls_wifi_hw);

	cls_wifi_ipc_deinit(cls_wifi_hw);

	cls_wifi_hw->enabled = false;
}

/**
 * cls_wifi_platform_init() - Initialize the platform
 *
 * @cls_wifi_plat: platform data (already updated by platform driver)
 * @platform_data: Pointer to store the main driver data pointer (aka cls_wifi_hw)
 *				That will be set as driver data for the platform driver
 * Return: 0 on success, < 0 otherwise
 *
 * Called by the platform driver after it has been probed
 */
int cls_wifi_platform_init(struct cls_wifi_plat *cls_wifi_plat, void **platform_data, u8 radio_idx)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	return cls_wifi_cfg80211_init(cls_wifi_plat, platform_data, radio_idx);
}
EXPORT_SYMBOL(cls_wifi_platform_init);

int cls_wifi_cmn_platform_init(struct cls_wifi_plat *cls_wifi_plat, void **platform_data, u8 radio_idx)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	return cls_wifi_cmn_platform_on(cls_wifi_plat, NULL);
}
EXPORT_SYMBOL(cls_wifi_cmn_platform_init);

/**
 * cls_wifi_platform_deinit() - Deinitialize the platform
 *
 * @cls_wifi_hw: main driver data
 *
 * Called by the platform driver after it is removed
 */
void cls_wifi_platform_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_cfg80211_deinit(cls_wifi_hw);
}
EXPORT_SYMBOL(cls_wifi_platform_deinit);

void cls_wifi_cmn_platform_deinit(struct cls_wifi_plat *cls_wifi_plat)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	return cls_wifi_cmn_platform_off(cls_wifi_plat);
}
EXPORT_SYMBOL(cls_wifi_cmn_platform_deinit);

