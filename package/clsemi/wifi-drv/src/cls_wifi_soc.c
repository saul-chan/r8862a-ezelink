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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_dubhe2000.h"
#include "cls_wifi_core.h"
#include "cls_wifi_cali.h"
#include "cls_wifi_main.h"
#include "hal_desc.h"
#include "ipc_host.h"
#include "cls_wifi_prof.h"
#include "cls_wifi_afe_init.h"
#include "ipc_shared.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_dif_sm.h"
#include "cls_wifi_mod_params.h"
#include "cls_wifi_heartbeat.h"

static const struct of_device_id cls_wifi_soc_of_match[] = {
	{ .compatible = "clourneysemi,dubhe2000-wifi",
	  .data = (void *)CLS_WIFI_HW_DUBHE2000,
	},
	{},
};

MODULE_DEVICE_TABLE(of, cls_wifi_soc_of_match);

struct cls_wifi_soc_paddr {
	u32 cls_wifi_soc_shared_mem_paddr;
	u32 cls_wifi_soc_sys_mem_paddr[CLS_WIFI_MAX_RADIOS];
	u32 cls_wifi_soc_irf_mem_paddr;
	u32 cls_wifi_soc_irf_table_mem_paddr;
	u32 cls_wifi_soc_irf_snd_smp_mem_paddr;
};

struct cls_wifi_soc_vaddr {
	void __iomem *cls_wifi_soc_sys_mem_vaddr[CLS_WIFI_MAX_RADIOS];
	void __iomem *cls_wifi_soc_shared_mem_vaddr[CLS_WIFI_MAX_RADIOS];
	void __iomem *cls_wifi_soc_irf_mem_vaddr;
	void __iomem *cls_wifi_soc_irf_table_mem_vaddr;
	void __iomem *cls_wifi_soc_sys_reg_vaddr;
	void __iomem *cls_wifi_soc_ipc_vaddr;
	void __iomem *cls_wifi_soc_ipc_out_vaddr;
	void __iomem *cls_wifi_soc_riu_base_vaddr;
	void __iomem *cls_wifi_soc_irf_snd_smp_vaddr;
};

struct cls_wifi_soc {
	struct platform_device *pdev;
	struct cls_wifi_soc_paddr soc_paddr;
	struct cls_wifi_soc_vaddr soc_vaddr;
};

static struct cls_wifi_soc *cls_wifi_soc_priv(struct cls_wifi_plat *cls_wifi_plat)
{
	return (struct cls_wifi_soc *)cls_wifi_plat->priv;
}

static u32 cls_wifi_soc_read32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset)
{
	return *((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SHARED, offset)));
}
static void cls_wifi_soc_write32_wmb(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SHARED, offset))) = value;
	wmb();
}
static void cls_wifi_soc_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *dst, u32 length)
{
	if (dst)
		memcpy(dst, (cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SHARED, offset)), length);
}
static void cls_wifi_soc_writen_wmb(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src, u32 length)
{
	if (src)
		memcpy((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SHARED, offset)), src, length);
	else
		memset((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SHARED, offset)), 0, length);
	wmb();
}
static u32 cls_wifi_soc_sys_read32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset)
{
	return *((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SYSTEM, offset)));
}
static void cls_wifi_soc_sys_write32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_SYSTEM, offset))) = value;
}
static void cls_wifi_soc_irf_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *dst, u32 length)
{
	if (dst)
		memcpy(dst, (cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF, offset)), length);
}
static void cls_wifi_soc_irf_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src, u32 length)
{
	if (src)
		memcpy((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF, offset)), src, length);
	else
		memset((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF, offset)), 0, length);
	wmb();
}
static void cls_wifi_soc_irf_table_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *dst, u32 length)
{
	if (dst)
		memcpy(dst, (cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF_TBL, offset)), length);
}
static void cls_wifi_soc_irf_table_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src, u32 length)
{
	if (src)
		memcpy((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF_TBL, offset)), src, length);
	else
		memset((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF_TBL, offset)), 0, length);
	wmb();
}
static void cls_wifi_soc_cpu_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src, u32 length)
{
	if (src)
		memcpy((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_CPU, offset)), src, length);
	else
		memset((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_CPU, offset)), 0, length);
}
static void cls_wifi_soc_sys_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u8 addr_type, u32 offset, void *src, u32 length)
{
	if (src)
		memcpy((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, addr_type, offset)), src, length);
	else
		memset((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, addr_type, offset)), 0, length);
}
#define IPC_EMB2APP_UNMASK_CLEAR_INDEX		0x000
#define IPC_APP2EMB_TRIGGER_INDEX		0x100
#define IPC_EMB2APP_RAWSTATUS_INDEX		0x100
#define IPC_EMB2APP_STATUS_INDEX		0x200
#define IPC_EMB2APP_ACK_INDEX			0x300
#define IPC_EMB2APP_UNMASK_SET_INDEX		0x400

static void cls_wifi_soc_irq_enable(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IPC_IN,
			IPC_EMB2APP_UNMASK_SET_INDEX))) = value;
}
static void cls_wifi_soc_irq_disable(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IPC_IN,
			IPC_EMB2APP_UNMASK_CLEAR_INDEX))) = value;
}
static u32 cls_wifi_soc_irq_get_status(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	return *((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IPC_IN,
			IPC_EMB2APP_STATUS_INDEX)));
}
static u32 cls_wifi_soc_irq_get_raw_status(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	return *((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IPC_IN,
			IPC_EMB2APP_RAWSTATUS_INDEX)));
}
static void cls_wifi_soc_irq_ack(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IPC_IN,
			IPC_EMB2APP_ACK_INDEX))) = value;
}
static void cls_wifi_soc_irq_trigger(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IPC_OUT,
			IPC_APP2EMB_TRIGGER_INDEX))) = value;
}

static u32 cls_wifi_soc_riu_read32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset)
{
	return *((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_RIU, offset)));
}
static void cls_wifi_soc_riu_write32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value)
{
	*((volatile u32 *)(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_RIU, offset))) = value;
}

static void cls_wifi_soc_irf_snd_smp_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *dst, u32 length)
{
	if (dst)
		memcpy(dst, (cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP, offset)), length);
}
static void cls_wifi_soc_irf_snd_smp_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src, u32 length)
{
	if (src)
		memcpy((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP, offset)), src, length);
	else
		memset((cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP, offset)), 0, length);
	wmb();
}

static int cls_wifi_dubhe2000_agc_load(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);

static const struct cls_wifi_ep_ops cls_wifi_ep_ops_dubhe2000 = {
	.read32 = cls_wifi_soc_read32,
	.write32 = cls_wifi_soc_write32_wmb,
	.readn = cls_wifi_soc_readn,
	.writen = cls_wifi_soc_writen_wmb,
	.sys_read32 = cls_wifi_soc_sys_read32,
	.sys_write32 = cls_wifi_soc_sys_write32,
	.riu_read32 = cls_wifi_soc_riu_read32,
	.riu_write32 = cls_wifi_soc_riu_write32,
	.irf_readn = cls_wifi_soc_irf_readn,
	.irf_writen = cls_wifi_soc_irf_writen,
	.irf_table_readn = cls_wifi_soc_irf_table_readn,
	.irf_table_writen = cls_wifi_soc_irf_table_writen,
	.irq_enable = cls_wifi_soc_irq_enable,
	.irq_disable = cls_wifi_soc_irq_disable,
	.irq_get_status = cls_wifi_soc_irq_get_status,
	.irq_get_raw_status = cls_wifi_soc_irq_get_raw_status,
	.irq_ack = cls_wifi_soc_irq_ack,
	.irq_trigger = cls_wifi_soc_irq_trigger,
	.irf_snd_smp_readn = cls_wifi_soc_irf_snd_smp_readn,
	.irf_snd_smp_writen = cls_wifi_soc_irf_snd_smp_writen,
	.reload_agc = cls_wifi_dubhe2000_agc_load,
};

/**
 * cls_wifi_sys_bin_fw_upload() - Load the requested binary FW into embedded side.
 *
 * @cls_wifi_plat: pointer to platform structure
 * @fw_addr: Virtual address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a binary file, into the specified address
 */
static int cls_wifi_sys_bin_fw_upload(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		u32 offset, char *filename, u8 addr_type)
{
	const struct firmware *fw;
	struct device *dev = cls_wifi_plat->dev;
	int err = 0;

	err = request_firmware(&fw, filename, dev);
	if (err)
		return err;

	cls_wifi_soc_sys_writen(cls_wifi_plat, radio_index, addr_type, offset, (void *)fw->data, (u32)fw->size);

	release_firmware(fw);

	return err;
}

/**
 * cls_wifi_dubhe2000_stop_agcfsm() - Stop a AGC state machine
 *
 * @cls_wifi_plat: pointer to platform structure
 * @agg_reg: Address of the agccntl register (within CLS_WIFI_ADDR_SYSTEM)
 * @agcctl: Updated with value of the agccntl rgister before stop
 * @memclk: Updated with value of the clock register before stop
 * @agc_ver: Version of the AGC load procedure
 */
static void cls_wifi_dubhe2000_stop_agcfsm(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		int agc_reg, u32 *agcctl, u32 *memclk, u8 agc_ver)
{
	/* Stop state machine : xxAGCCNTL0[AGCFSMRESET]=1 */
	*agcctl = cls_wifi_plat->ep_ops->riu_read32(cls_wifi_plat, radio_index, agc_reg);
	cls_wifi_plat->ep_ops->riu_write32(cls_wifi_plat, radio_index, agc_reg,
			(*agcctl) | BIT(12));
}

/**
 * cls_wifi_dubhe2000_start_agcfsm() - Restart a AGC state machine
 *
 * @cls_wifi_plat: pointer to platform structure
 * @agg_reg: Address of the agccntl register (within CLS_WIFI_ADDR_SYSTEM)
 * @agcctl: value of the agccntl register to restore
 * @memclk: value of the clock register to restore
 * @agc_ver: Version of the AGC load procedure
 */
static void cls_wifi_dubhe2000_start_agcfsm(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		int agc_reg, u32 agcctl, u32 memclk, u8 agc_ver)
{
	cls_wifi_plat->ep_ops->riu_write32(cls_wifi_plat, radio_index, agc_reg,
			agcctl & ~BIT(12));
}

/**
 * cls_wifi_plat_agc_load() - Load AGC ucode
 *
 * @cls_wifi_plat: platform data
 * c.f Modem UM (AGC/CCA initialization)
 */
static int cls_wifi_dubhe2000_agc_load(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index)
{
	int ret = 0;
	u32 agc = 0, agcctl, memclk;
	u32 rf_ver;
	u8 agc_ver;
	u32 agc_load_ver = 0;
	char *agcbin;

	rf_ver = cls_wifi_plat->ep_ops->sys_read32(cls_wifi_plat, radio_index,
			CLS_DUBHE2000_MDM_HDMCONFIG_ADDR);
	rf_ver = __MDM_PHYCFG_FROM_VERS(rf_ver);
	WARN((rf_ver != MDM_PHY_CONFIG_VERSION), "Unknown PHY version 0x%08x\n", rf_ver);

	agc = CLS_DUBHE2000_RIU_CLS_WIFIAGCCNTL_ADDR;
	/* Read RIU version register */
	agc_load_ver = cls_wifi_plat->ep_ops->riu_read32(cls_wifi_plat, radio_index,
			CLS_DUBHE2000_RIU_CLS_WIFIVERSION_ADDR);
	agc_ver = __RIU_AGCLOAD_FROM_VERS(agc_load_ver);

	cls_wifi_dubhe2000_stop_agcfsm(cls_wifi_plat, radio_index, agc, &agcctl, &memclk, agc_ver);

	if(0 == radio_index){
		agcbin = CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_AGC_FW_2G_NAME;
	} else {
		agcbin = CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_AGC_FW_NAME;
	}
	dev_err(cls_wifi_plat->dev,"agc load:%s\n",agcbin);

	ret = cls_wifi_sys_bin_fw_upload(cls_wifi_plat, radio_index, CLS_DUBHE2000_PHY_AGC_UCODE_ADDR,
			agcbin, CLS_WIFI_ADDR_RIU);

	if (!ret && (agc_ver == 1)) {
		/* Run BIST to ensure that the AGC RAM was correctly loaded */
		cls_wifi_plat->ep_ops->riu_write32(cls_wifi_plat, radio_index,
				CLS_DUBHE2000_RIU_CLS_WIFIDYNAMICCONFIG_ADDR,
				BIT(28));
		while (cls_wifi_plat->ep_ops->riu_read32(cls_wifi_plat, radio_index,
				CLS_DUBHE2000_RIU_CLS_WIFIDYNAMICCONFIG_ADDR) & BIT(28))
			;

		if (!(cls_wifi_plat->ep_ops->riu_read32(cls_wifi_plat, radio_index,
				CLS_DUBHE2000_RIU_AGCMEMBISTSTAT_ADDR) & BIT(0))) {
			dev_err(cls_wifi_plat->dev, "AGC RAM not loaded correctly 0x%08x\n",
					cls_wifi_plat->ep_ops->riu_read32(cls_wifi_plat, radio_index,
					CLS_DUBHE2000_RIU_AGCMEMSIGNATURESTAT_ADDR));
			ret = -EIO;
		}
	}

	cls_wifi_dubhe2000_start_agcfsm(cls_wifi_plat, radio_index, agc, agcctl, memclk, agc_ver);

	return ret;
}

/**
 * cls_wifi_soc_firemware_load() - Load the requested binary FW into firmware side.
 *
 * @cls_wifi_plat: pointer to platform structure
 * @fw_addr: Virtual address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a binary file, into the specified address
 */
static int cls_wifi_soc_firemware_load(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, const char *filename)
{
	const struct firmware *fw;
	struct device *dev = cls_wifi_plat->dev;
	int err = 0;

	err = request_firmware(&fw, filename, dev);
	if (err) {
		dev_err(dev, "\n--- Error %d, %s:%d\n", err, __func__, __LINE__);
		return err;
	}

	cls_wifi_soc_cpu_writen(cls_wifi_plat, radio_index, 0, (u32 *)fw->data, (unsigned int)fw->size);

	release_firmware(fw);

	return err;
}

static void cls_wifi_soc_reg_set(void __iomem *base, u32 offset, u32 value)
{
	if (value != CLS_REG_RAW_READ32(base, offset))
		CLS_REG_RAW_WRITE32(base, offset, value);
}

/**
 * cls_dubhe2000_firmware_on() - Load FW code and on
 *
 * @cls_wifi_plat: platform data
 * @firmware: Name of the fw.
 */
static int cls_dubhe2000_firmware_on(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		bool final)
{
	struct cls_wifi_soc *cls_wifi_soc = cls_wifi_soc_priv(cls_wifi_plat);
	struct device *dev = cls_wifi_plat->dev;
	const char *firmware = cls_wifi_plat->hw_params.firmware_file_name[radio_index];
	void __iomem *cpu_config_addr, *cpu_release_addr, *wifi_config_addr, *sys_config_addr;
	u32 release_addr = cls_wifi_soc->soc_paddr.cls_wifi_soc_shared_mem_paddr +
			cls_wifi_plat->hw_params.sys_mem_offset[radio_index];
	int ret = 0;
	static int clk_afe_sft = 0;
	unsigned long pll480M_timeout;
	u32 eco_rw_word_val;

	dev_err(dev, "\n--- Radio %d, firmware: %s, release_addr: 0x%x\n",
			radio_index, firmware, release_addr);
	ret = cls_wifi_soc_firemware_load(cls_wifi_plat, radio_index, firmware);
	if (ret) {
		dev_err(dev, "\n--- Error %d, %s:%d\n", ret, __func__, __LINE__);
		return ret;
	}
	barrier();

	cpu_config_addr = ioremap(CLS_DUBHE2000_CPU_CONFIG_BASE, CLS_DUBHE2000_CPU_CONFIG_SIZE);
	cpu_release_addr = ioremap(CLS_DUBHE2000_RELEASE_BASE, CLS_DUBHE2000_RELEASE_SIZE);
	wifi_config_addr = ioremap(CLS_DUBHE2000_WIFI_CONFIG_BASE, CLS_DUBHE2000_WIFI_CONFIG_SIZE);
	sys_config_addr = ioremap(CLS_DUBHE2000_SYS_CONFIG_BASE, CLS_DUBHE2000_SYS_CONFIG_SIZE);

	pr_warn("first read PPL 480M status %x\n", CLS_REG_RAW_READ32(wifi_config_addr, 0xC));
	pll480M_timeout = jiffies;
	while(!CLS_REG_RAW_READ32(wifi_config_addr, 0xC)){
		if((pll480M_timeout + 1000) > jiffies) {
			pr_warn("wait PPL 480M ready timeout!!!,cur status 0x%x\n",
				CLS_REG_RAW_READ32(wifi_config_addr, 0xC));
			break;
		}
	}
	pr_warn(" PPL 480M status %x\n",CLS_REG_RAW_READ32(wifi_config_addr, 0xC));

	cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_PLL_OFFSET, CLS_DUBHE2000_WIFI_PLL_VALUE);  ///step4

	if (radio_index == 0) {
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_CG_OFFSET_0, CLS_DUBHE2000_WIFI_CG_VALUE_0);  ///step8-1
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_CG_OFFSET_0, CLS_DUBHE2000_DIF_CG_VALUE_0);///step8-3
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_2G_CG_PARA_1, CLS_DUBHE2000_DIF_2G_CG_PARA_1_VAL);///step8-3
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DPD_CG_OFFSET, CLS_DUBHE2000_DPD_CG_VALUE);  ///step8-5
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DPD_CG_PARA_1, CLS_DUBHE2000_DPD_CG_PARA_1_VAL);  ///step8-5
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_CG_OFFSET_2, CLS_DUBHE2000_DIF_CG_VALUE_2);	///step8-6
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_COM_CG_PARA_1, CLS_DUBHE2000_DIF_COM_CG_PARA_1_VAL);	///step8-6
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_SRC_CG_OFFSET, CLS_DUBHE2000_SRC_CG_VALUE);	///step8-7
		if(clk_afe_sft == 0) {
			clk_afe_sft = 1;
			cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_SRC_RL_OFFSET, CLS_DUBHE2000_SRC_RL_INIT_VALUE);	 ///step9
		}
#ifndef CONFIG_CLS_EMU_ADAPTER
		/* step 10 : initialize AFE, provide 640M clock. */
		if (irf_start_afe(cls_wifi_plat, radio_index)) {
			pr_err("irf_start_afe failure.\n");
			return -1;
		}
#endif
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_640_480MHz_OFFSET, CLS_DUBHE2000_WIFI_640_480MHz_VALUE);///step11-1
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_RL_OFFSET_0, CLS_DUBHE2000_WIFI_RL_VALUE_0);  ///step11-2
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_RL_OFFSET_0, CLS_DUBHE2000_DIF_RL_VALUE_0); ///step11-4
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DPD_RL_OFFSET, CLS_DUBHE2000_DPD_RL_VALUE); ///step12-6
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_RL_OFFSET_2, CLS_DUBHE2000_DIF_RL_VALUE_2); ///step11-7
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_SRC_RL_OFFSET, CLS_DUBHE2000_SRC_RL_VALUE);	 ///step12-8

		eco_rw_word_val = CLS_REG_RAW_READ32(sys_config_addr, CLS_DUBHE2000_ECO_RW_WORD_OFFSET);
		cls_wifi_soc_reg_set(sys_config_addr, CLS_DUBHE2000_ECO_RW_WORD_OFFSET, (eco_rw_word_val | CLS_DUBHE2000_ECO_RW_WORD_2G_VALUE));	 ///ECO

		barrier();
		cls_wifi_dubhe2000_agc_load(cls_wifi_plat, radio_index);
		// Reset WPU 2G WatchDog3
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1,
			CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1) &
			~(CLS_DUBHE2000_2G_WDT_RELEASE_VAL));
		//set address and release core
		CLS_REG_RAW_WRITE32(cpu_config_addr, CLS_DUBHE2000_UNLOCK_OFFSET_0, CLS_DUBHE2000_UNLOCK_VAL_CORE_0);
		CLS_REG_RAW_WRITE32(cpu_config_addr, CLS_DUBHE2000_RESET_OFFSET_0,
				(CLS_REG_RAW_READ32(cpu_config_addr, CLS_DUBHE2000_RESET_OFFSET_0) &
				~CLS_DUBHE2000_RESET_MASK) |
				(release_addr >> CLS_DUBHE2000_RESET_OFFSET));
		// reset core 0 first
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0,
				CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0) &
				~CLS_DUBHE2000_RELEASE_VAL_CORE_0);
		// release core 0
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0,
				CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0) |
				CLS_DUBHE2000_RELEASE_VAL_CORE_0);
		dev_err(dev, "\n--- Released WPU for radio %d,release_reg:%x\n", radio_index,CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0));
		// Release WPU 2G WatchDog3
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1,
			CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1) |
			CLS_DUBHE2000_2G_WDT_RELEASE_VAL);
	} else if (radio_index == 1) {
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_CG_OFFSET_1, CLS_DUBHE2000_WIFI_CG_VALUE_1); ///step8-2
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_CG_OFFSET_1, CLS_DUBHE2000_DIF_CG_VALUE_1);///step8-4
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_5G_CG_PARA_1, CLS_DUBHE2000_DIF_5G_CG_PARA_1_VAL);///step8-4
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DPD_CG_OFFSET, CLS_DUBHE2000_DPD_CG_VALUE);  ///step8-5
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DPD_CG_PARA_1, CLS_DUBHE2000_DPD_CG_PARA_1_VAL);  ///step8-5
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_CG_OFFSET_2, CLS_DUBHE2000_DIF_CG_VALUE_2);   ///step8-6
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_COM_CG_PARA_1, CLS_DUBHE2000_DIF_COM_CG_PARA_1_VAL);	///step8-6
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_SRC_CG_OFFSET, CLS_DUBHE2000_SRC_CG_VALUE);   ///step8-7
		if(clk_afe_sft == 0) {
			clk_afe_sft = 1;
			cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_SRC_RL_OFFSET, CLS_DUBHE2000_SRC_RL_INIT_VALUE);	 ///step9
		}
#ifndef CONFIG_CLS_EMU_ADAPTER
		/* step 10 : initialize AFE, provide 640M clock. */
		if (irf_start_afe(cls_wifi_plat, radio_index)) {
			pr_err("irf_start_afe failure.\n");
			return -1;
		}
#endif
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_640_480MHz_OFFSET, CLS_DUBHE2000_WIFI_640_480MHz_VALUE);///step11-1
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_WIFI_RL_OFFSET_1, CLS_DUBHE2000_WIFI_RL_VALUE_1);  ///step11-3
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_RL_OFFSET_1, CLS_DUBHE2000_DIF_RL_VALUE_1); ///step11-5
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DPD_RL_OFFSET, CLS_DUBHE2000_DPD_RL_VALUE); ///step11-6
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_DIF_RL_OFFSET_2, CLS_DUBHE2000_DIF_RL_VALUE_2); ///step11-7
		cls_wifi_soc_reg_set(wifi_config_addr, CLS_DUBHE2000_SRC_RL_OFFSET, CLS_DUBHE2000_SRC_RL_VALUE);	 ///step11-8

		eco_rw_word_val = CLS_REG_RAW_READ32(sys_config_addr, CLS_DUBHE2000_ECO_RW_WORD_OFFSET);
		cls_wifi_soc_reg_set(sys_config_addr, CLS_DUBHE2000_ECO_RW_WORD_OFFSET, (eco_rw_word_val | CLS_DUBHE2000_ECO_RW_WORD_5G_VALUE));	 ///ECO

		barrier();
		cls_wifi_dubhe2000_agc_load(cls_wifi_plat, radio_index);
		// Reset WPU 5G WatchDog2
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1,
			CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1) &
			~(CLS_DUBHE2000_5G_WDT_RELEASE_VAL));
		//set address and release core
		CLS_REG_RAW_WRITE32(cpu_config_addr, CLS_DUBHE2000_UNLOCK_OFFSET_1, CLS_DUBHE2000_UNLOCK_VAL_CORE_1);
		CLS_REG_RAW_WRITE32(cpu_config_addr, CLS_DUBHE2000_RESET_OFFSET_1,
				(CLS_REG_RAW_READ32(cpu_config_addr, CLS_DUBHE2000_RESET_OFFSET_1) &
				~CLS_DUBHE2000_RESET_MASK) |
				(release_addr >> CLS_DUBHE2000_RESET_OFFSET));
		// reset core 1 first
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1,
				CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1) &
				~CLS_DUBHE2000_RELEASE_VAL_CORE_1);
		// release core 1
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1,
				CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1) |
				CLS_DUBHE2000_RELEASE_VAL_CORE_1);
		dev_err(dev, "\n--- Released WPU for radio %d,release_reg:%x\n", radio_index,CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1));
		// Release WPU 5G WatchDog2
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1,
			CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1) |
			CLS_DUBHE2000_5G_WDT_RELEASE_VAL);
	} else if (radio_index == CLS_WIFI_DUBHE2000_INDEX_C) {
		//set address and release core
		CLS_REG_RAW_WRITE32(cpu_config_addr, CLS_DUBHE2000_UNLOCK_OFFSET_1, CLS_DUBHE2000_UNLOCK_VAL_CORE_1);
		CLS_REG_RAW_WRITE32(cpu_config_addr, CLS_DUBHE2000_RESET_WPU_CMN,
				(CLS_REG_RAW_READ32(cpu_config_addr, CLS_DUBHE2000_RESET_WPU_CMN) &
				~CLS_DUBHE2000_RESET_MASK) |
				(release_addr >> CLS_DUBHE2000_RESET_OFFSET));
		// reset cmn core first
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C,
				CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C) &
				~CLS_DUBHE2000_RELEASE_VAL_WPU_CMN);
		// release cmn core
		CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C,
				CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C) |
				CLS_DUBHE2000_RELEASE_VAL_WPU_CMN);
		dev_err(dev, "\n--- Released WPU for radio %d,release_reg:%x\n",
				radio_index, CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C));
	} else {
		dev_err(dev, "\n--- Unsupported radio index %d\n", radio_index);
	}
	iounmap(cpu_config_addr);
	iounmap(cpu_release_addr);
	iounmap(wifi_config_addr);
	iounmap(sys_config_addr);

	return ret;
}

static int cls_wifi_cmn_init(struct cls_wifi_plat *cls_wifi_plat)
{
	int ret = 0;
	struct ipc_host_cmn_env_tag *ipc_host_cmn_env;

	cls_wifi_plat->cmn_hw->dev = cls_wifi_plat->dev;
	cls_wifi_plat->cmn_hw->radio_idx = CLS_WIFI_DUBHE2000_INDEX_C;
	cls_wifi_plat->cmn_hw->enabled = false;
	ipc_host_cmn_env = kzalloc(sizeof(struct ipc_host_cmn_env_tag), GFP_KERNEL);
	if (!ipc_host_cmn_env)
		return -ENOMEM;

	cls_wifi_plat->cmn_hw->ipc_host_cmn_env = ipc_host_cmn_env;
	ipc_host_cmn_env->plat = cls_wifi_plat;
	/* init ipc share memory!!! */
	cls_wifi_plat->ep_ops->writen(cls_wifi_plat, CLS_WIFI_DUBHE2000_INDEX_C, 0,
		NULL, (uint32_t)sizeof(struct cmn_ipc_shared_env_tag));

	ret = cls_wifi_cmn_platform_init(cls_wifi_plat,
		(void **)&ipc_host_cmn_env, CLS_WIFI_DUBHE2000_INDEX_C);

	cls_wifi_plat->ep_ops->write32(cls_wifi_plat, CLS_WIFI_DUBHE2000_INDEX_C,
		offsetof(struct cmn_ipc_shared_env_tag, ipc_pattern), IPC_PATTERN_INIT_MAGIC);

	return ret;
}

static void cls_wifi_cmn_deinit(struct cls_wifi_plat *cls_wifi_plat)
{
	cls_wifi_cmn_platform_deinit(cls_wifi_plat);

	kfree(cls_wifi_plat->cmn_hw->ipc_host_cmn_env);
}

static int cls_wifi_cmn_plat_init(struct cls_wifi_plat *cls_wifi_plat)
{
	u32_l wpu_ipc_pattern;
	u32_l wait_time;
	u32_l wait_timeout = 10 * 1000;
	int i;
	struct cls_wifi_hw *cls_wifi_hw;

	cls_wifi_plat->ep_ops->write32(cls_wifi_plat, CLS_WIFI_DUBHE2000_INDEX_C,
			offsetof(struct cmn_ipc_shared_env_tag, ipc_pattern),
			IPC_PATTERN_MAGIC);

	for (i = 0; i < CLS_WIFI_DUBHE2000_INDEX_C; i++) {
		if (is_band_enabled(cls_wifi_plat, i)) {
			cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[i];
			cls_wifi_hw->ipc_cmn_env = cls_wifi_plat->cmn_hw->ipc_host_cmn_env;
		}
	}

	while(1) {
		wpu_ipc_pattern = cls_wifi_plat->ep_ops->read32(cls_wifi_plat,
				CLS_WIFI_DUBHE2000_INDEX_C,
				offsetof(struct cmn_ipc_shared_env_tag, fw_ipc_pattern));
		if (wpu_ipc_pattern != ((uint32_t)WPU_IPC_PATTERN_MAGIC)) {
			msleep(10);
			wait_time += 10;
			if (wait_time < wait_timeout)
				continue;

			pr_warn(">>> %s: %d, wait_time: %u(%u), wpu_ipc_pattern: %08x\n",
					__func__, __LINE__, wait_time,  wait_timeout, wpu_ipc_pattern);
			break;
		} else {
			pr_warn(">>> %s: %d, wait_time: %u(%u), wpu_ipc_pattern: %08x\n",
					__func__, __LINE__, wait_time,  wait_timeout, wpu_ipc_pattern);
			break;
		}
	}

	return 0;
}

/**
 * cls_wifi_soc_irq_hdlr - IRQ handler
 */
irqreturn_t cls_wifi_soc_irq_hdlr(int irq, void *arg)
{
	struct cls_wifi_irq_task *irq_task = (struct cls_wifi_irq_task *)arg;

	disable_irq_nosync(irq);
	tasklet_schedule(&irq_task->task);
	return IRQ_HANDLED;
}

/**
 * cls_wifi_soc_task - Bottom half for IRQ handler
 */
void cls_wifi_soc_task(unsigned long data)
{
	struct cls_wifi_irq_task *irq_task = (struct cls_wifi_irq_task *)data;
	struct cls_wifi_hw *cls_wifi_hw = irq_task->cls_wifi_hw;
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	u32 status;

	REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_CLS_WIFI_IPC_IRQ_HDLR);

	/* Ack unconditionnally in case ipc_host_get_status does not see the irq */
	cls_wifi_plat->if_ops->ack_irq(cls_wifi_plat);

	while ((status = ipc_host_get_status(cls_wifi_hw->ipc_env))) {
		barrier();
		ipc_host_irq(cls_wifi_hw, status);
		barrier();
		cls_wifi_plat->if_ops->ack_irq(cls_wifi_plat);
	}

	spin_lock(&cls_wifi_hw->tx_lock);
	barrier();
	cls_wifi_hwq_process_all(cls_wifi_hw);
	barrier();
	spin_unlock(&cls_wifi_hw->tx_lock);

	enable_irq(irq_task->irq);

	REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_CLS_WIFI_IPC_IRQ_HDLR);
}

static int cls_wifi_soc_plat_enable(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_irq_task *irq_task = &cls_wifi_hw->plat->irq_task[cls_wifi_hw->radio_idx][0];
	int ret;

	irq_task->cls_wifi_hw = cls_wifi_hw;
	tasklet_init(&irq_task->task, cls_wifi_soc_task, (unsigned long)irq_task);
	ret = request_irq(irq_task->irq, cls_wifi_soc_irq_hdlr, IRQF_SHARED, "cls_wifi_soc", irq_task);
	if (ret)
		pr_warn("{(C-ERR)%x}\r\n", ret);

	return ret;
}

static int cls_wifi_soc_plat_disable(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_irq_task *irq_task = &cls_wifi_hw->plat->irq_task[cls_wifi_hw->radio_idx][0];

	free_irq(irq_task->irq, irq_task);
	tasklet_kill(&irq_task->task);
	return 0;
}

static u8 *cls_dubhe2000_get_address(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
			int addr_name, unsigned int offset)
{
	struct cls_wifi_soc *cls_wifi_soc = cls_wifi_soc_priv(cls_wifi_plat);

	switch (addr_name) {
	case CLS_WIFI_ADDR_CPU:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0]
			+ cls_wifi_plat->hw_params.sys_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_SHARED:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0]
			+ cls_wifi_plat->hw_params.shared_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_SYSTEM:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_sys_reg_vaddr
			+ cls_wifi_plat->hw_params.sys_reg_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IPC_IN:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_ipc_vaddr
			+ cls_wifi_plat->hw_params.ipc_in_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IPC_OUT:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_ipc_out_vaddr
			+ cls_wifi_plat->hw_params.ipc_out_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0]
			+ cls_wifi_plat->hw_params.irf_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF_TBL:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0]
			+ cls_wifi_plat->hw_params.irf_table_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_RIU:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_riu_base_vaddr
			+ cls_wifi_plat->hw_params.riu_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF_SND_SMP:
		return cls_wifi_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr + offset;
	default:
		dev_err(cls_wifi_plat->dev, "Invalid address %d\n", addr_name);
		return NULL;
	}
}

static u32 cls_dubhe2000_get_phy_address(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
			int addr_name, unsigned int offset)
{
	struct cls_wifi_soc *cls_wifi_soc = cls_wifi_soc_priv(cls_wifi_plat);

	switch (addr_name) {
	case CLS_WIFI_ADDR_IRF_PHY:
		return cls_wifi_soc->soc_paddr.cls_wifi_soc_shared_mem_paddr
			+ cls_wifi_plat->hw_params.irf_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF_TBL_PHY:
		return cls_wifi_soc->soc_paddr.cls_wifi_soc_shared_mem_paddr
			+ cls_wifi_plat->hw_params.irf_table_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF_SND_SMP_PHY:
		return cls_wifi_soc->soc_paddr.cls_wifi_soc_irf_snd_smp_mem_paddr + offset;
	default:
		dev_err(cls_wifi_plat->dev, "Invalid address %d\n", addr_name);
		return 0;
	}
}

static void cls_wifi_soc_plat_ack_irq(struct cls_wifi_plat *cls_wifi_plat)
{

}

static int cls_wifi_soc_plat_get_config_reg(struct cls_wifi_plat *cls_wifi_plat, const u32 **list)
{
	return 0;
}

static const struct cls_wifi_if_ops cls_wifi_if_ops_dubhe2000 = {
	.firmware_on = cls_dubhe2000_firmware_on,
	.enable = cls_wifi_soc_plat_enable,
	.disable = cls_wifi_soc_plat_disable,
	.get_address = cls_dubhe2000_get_address,
	.get_phy_address = cls_dubhe2000_get_phy_address,
	.ack_irq = cls_wifi_soc_plat_ack_irq,
	.get_config_reg = cls_wifi_soc_plat_get_config_reg,
};

static int cls_wifi_soc_probe(struct platform_device *pdev)
{
	struct cls_wifi_plat *cls_wifi_plat = NULL;
	struct cls_wifi_soc *cls_soc;
	struct cls_wifi_hw *cls_wifi_hw;
	const struct of_device_id *of_id;
	struct device_node *np = pdev->dev.of_node;
	struct resource res;
	int ret = 0, i;
	struct cls_wifi_cmn_hw *cls_wifi_cmn_hw = NULL;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_cmn_hw = kzalloc(sizeof(struct cls_wifi_cmn_hw), GFP_KERNEL);
	if (!cls_wifi_cmn_hw)
		return -ENOMEM;

	cls_wifi_plat = kzalloc(sizeof(struct cls_wifi_plat) + sizeof(struct cls_wifi_soc), GFP_KERNEL);
	if (!cls_wifi_plat) {
		kfree(cls_wifi_cmn_hw);
		cls_wifi_cmn_hw = NULL;
		return -ENOMEM;
	}

	cls_wifi_plat->dev = &pdev->dev;
	cls_wifi_plat->bus_type = CLS_WIFI_BUS_TYPE_SOC;
	cls_wifi_plat->cmn_hw = cls_wifi_cmn_hw;

	cls_soc = cls_wifi_soc_priv(cls_wifi_plat);
	cls_soc->pdev = pdev;
	of_id = of_match_device(cls_wifi_soc_of_match, &pdev->dev);
	if (!of_id) {
		dev_err(&pdev->dev, "failed to find matching device tree id\n");
		ret = -EOPNOTSUPP;
		goto out_free;
	}

	cls_wifi_plat->hw_rev = (enum cls_wifi_hw_rev)of_id->data;
	switch (cls_wifi_plat->hw_rev) {
	case CLS_WIFI_HW_DUBHE2000:
		cls_wifi_plat->ep_ops = &cls_wifi_ep_ops_dubhe2000;
		cls_wifi_plat->if_ops = &cls_wifi_if_ops_dubhe2000;
		cls_wifi_plat->path_info.cal_path = cls_wifi_mod_params.cal_path_cs8862;
		cls_wifi_plat->path_info.irf_path = cls_wifi_mod_params.irf_path_cs8862;
		break;
	default:
		dev_err(&pdev->dev, "unsupported device type %d\n", cls_wifi_plat->hw_rev);
		ret = -EOPNOTSUPP;
		goto out_free;
	}

	ret = cls_wifi_init_hw_params(cls_wifi_plat);
	if (ret)
		goto out_free;

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
		goto out_free;
	}

	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		cls_soc->soc_paddr.cls_wifi_soc_shared_mem_paddr = res.start;
		cls_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0] =
				devm_memremap(&pdev->dev, res.start, resource_size(&res), MEMREMAP_WC);

		if (IS_ERR(cls_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0])) {
			dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
			goto out_mem;
		}
	}
	else {
		dev_err(&pdev->dev, "Unsupported hw_dev: %s %d\n", __func__, __LINE__);
		goto out_free;
	}

	cls_soc->soc_vaddr.cls_wifi_soc_sys_reg_vaddr = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(cls_soc->soc_vaddr.cls_wifi_soc_sys_reg_vaddr)) {
		dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
		goto out_mem;
	}

	cls_soc->soc_vaddr.cls_wifi_soc_ipc_vaddr = devm_platform_ioremap_resource(pdev, 2);
	if (IS_ERR(cls_soc->soc_vaddr.cls_wifi_soc_ipc_vaddr)) {
		dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
		goto out_mem;
	}

	cls_soc->soc_vaddr.cls_wifi_soc_riu_base_vaddr = devm_platform_ioremap_resource(pdev, 3);
	if (IS_ERR(cls_soc->soc_vaddr.cls_wifi_soc_riu_base_vaddr)) {
		dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
		goto out_mem;
	}

	ret = of_address_to_resource(np, 4, &res);
	if (ret) {
		dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
		goto out_mem;
	}
	cls_soc->soc_paddr.cls_wifi_soc_irf_snd_smp_mem_paddr = res.start +
			cls_wifi_plat->hw_params.irf_snd_smp_mem_offset[0];
	cls_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr =
			devm_memremap(&pdev->dev, res.start +
					cls_wifi_plat->hw_params.irf_snd_smp_mem_offset[0],
					cls_wifi_plat->hw_params.irf_snd_smp_mem_size[0], MEMREMAP_WC);

	if (IS_ERR(cls_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr)) {
		dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
		goto out_mem;
	}
	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		cls_soc->soc_vaddr.cls_wifi_soc_ipc_out_vaddr = devm_platform_ioremap_resource(pdev, 5);
		if (IS_ERR(cls_soc->soc_vaddr.cls_wifi_soc_ipc_out_vaddr)) {
			dev_err(&pdev->dev, "Error: %s %d\n", __func__, __LINE__);
			goto out_mem;
		}
	}

#ifdef CLS_WIFI_DUBHE_ETH
	cls_wifi_init_eth_info(cls_wifi_plat);
#endif

	cls_wifi_cmn_init(cls_wifi_plat);
	for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
		if (i == CLS_WIFI_DUBHE2000_INDEX_C) {
			ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat,
					CLS_WIFI_DUBHE2000_INDEX_C, false);
			if (ret) {
				dev_err(&pdev->dev, "firmware_on [%d] failed(%d)\n", i, ret);
				goto out_cmn_wpu;
			} else {
				cls_wifi_cmn_plat_init(cls_wifi_plat);
				continue;
			}
		}

		if (!(bands_enable & cls_wifi_plat->hw_params.band_cap[i])) {
			dev_err(&pdev->dev, "radio %d is disabled by ko params\n", i);
			continue;
		}
		ipc_host_ipc_pattern_set(cls_wifi_plat, i, IPC_PATTERN_INIT_MAGIC);
		ipc_host_wpu_ipc_pattern_set(cls_wifi_plat, i, 0);

		if (cls_wifi_irf_init(cls_wifi_plat, i))
			dev_err(&pdev->dev, "%s %d, Failed to initialize irf function\n", __func__, __LINE__);

		ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat, i, false);
		if (ret) {
			dev_err(&pdev->dev, "firmware_on [%d] failed(%d)\n", i, ret);
			goto out_platform;
		}
		cls_wifi_plat->irq_task[i][0].irq = platform_get_irq(pdev, i);
		ret = cls_wifi_platform_init(cls_wifi_plat, (void **)&cls_wifi_hw, i);
		if (ret) {
			cls_wifi_plat->bands_enabled |= 1 << i;
			dev_err(&pdev->dev, "cls_wifi_platform_init[%d] failed(%d)\n", i, ret);
			goto out_platform;
		}
		cls_wifi_plat->bands_enabled |= 1 << i;
		cls_wifi_dif_sm_init(cls_wifi_plat, i);
		cls_wifi_heartbeat_init(cls_wifi_plat, i);
		cls_wifi_irf_smp_send_ram_init(cls_wifi_plat, i);
	}
	cls_wifi_dif_online_schedule_init(cls_wifi_plat);
	cls_wifi_tsensor_timer_init(cls_wifi_plat);
	cls_wifi_afe_xtal_ctrim_set(cls_wifi_plat);
	platform_set_drvdata(pdev, (void *)cls_wifi_plat);
	return 0;

out_platform:
out_cmn_wpu:
	for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
		if (!is_band_enabled(cls_wifi_plat, i)) {
			dev_err(&pdev->dev, "radio %d is disabled by ko params\n", i);
			continue;
		}
		cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[i];
		cls_wifi_heartbeat_deinit(cls_wifi_plat, i);
		cls_wifi_platform_deinit(cls_wifi_hw);
	}
	cls_wifi_tsensor_timer_deinit(cls_wifi_plat);
	cls_wifi_cmn_deinit(cls_wifi_plat);
out_mem:
	if (cls_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr)
		devm_memunmap(&pdev->dev, cls_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr);
	if (cls_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0])
		devm_memunmap(&pdev->dev, cls_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0]);
out_free:
	platform_set_drvdata(pdev, NULL);
	kfree(cls_wifi_plat);
	kfree(cls_wifi_cmn_hw);
	return ret;
}

static int cls_wifi_soc_remove(struct platform_device *pdev)
{
	struct cls_wifi_plat *cls_wifi_plat;
	struct cls_wifi_soc *cls_soc;
	struct cls_wifi_hw *cls_wifi_hw;
	void __iomem *cpu_release_addr;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_plat = platform_get_drvdata(pdev);
	cls_soc = cls_wifi_soc_priv(cls_wifi_plat);
	cls_wifi_dif_online_schedule_deinit(cls_wifi_plat);
	cls_wifi_afe_xtal_ctrim_unset(cls_wifi_plat);
	for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
		if (!is_band_enabled(cls_wifi_plat, i)) {
			dev_err(&pdev->dev, "radio %d is disabled by ko params\n", i);
			continue;
		}
		cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[i];
		cls_wifi_heartbeat_deinit(cls_wifi_plat, i);
		cls_wifi_platform_deinit(cls_wifi_hw);
	}
	cls_wifi_tsensor_timer_deinit(cls_wifi_plat);
	cls_wifi_cmn_deinit(cls_wifi_plat);

	if (cls_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr)
		devm_memunmap(&pdev->dev, cls_soc->soc_vaddr.cls_wifi_soc_irf_snd_smp_vaddr);
	if (cls_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0])
		devm_memunmap(&pdev->dev, cls_soc->soc_vaddr.cls_wifi_soc_shared_mem_vaddr[0]);

	cpu_release_addr = ioremap(CLS_DUBHE2000_RELEASE_BASE, CLS_DUBHE2000_RELEASE_SIZE);
	// Reset WPU 2G
	CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0,
	CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0) &
		~CLS_DUBHE2000_RELEASE_VAL_CORE_0);
	// Reset WPU 2G WatchDog3
	CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1,
	CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1) &
		~(CLS_DUBHE2000_2G_WDT_RELEASE_VAL));
	// Reset WPU 5G
	CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1,
	CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_1) &
		~CLS_DUBHE2000_RELEASE_VAL_CORE_1);
	// Reset WPU 5G WatchDog2
	CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1,
	CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_PERI_SUB_RST_PARA1) &
		~(CLS_DUBHE2000_5G_WDT_RELEASE_VAL));
	// Reset WPU CMN
	CLS_REG_RAW_WRITE32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C,
	CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_C) &
		~CLS_DUBHE2000_RELEASE_VAL_WPU_CMN);
	dev_info(cls_wifi_plat->dev, "\n--- Reset All WPU,release_reg:%x\n",
		CLS_REG_RAW_READ32(cpu_release_addr, CLS_DUBHE2000_RELEASE_OFFSET_0));
	iounmap(cpu_release_addr);

	platform_set_drvdata(pdev, NULL);

	kfree(cls_wifi_plat->cmn_hw);
	kfree(cls_wifi_plat);

	return 0;
}

struct platform_driver cls_wifi_soc_driver = {
	.probe	  = cls_wifi_soc_probe,
	.remove	 = cls_wifi_soc_remove,
	.driver = {
		.name = "cls_wifi_soc",
		.of_match_table = cls_wifi_soc_of_match,
	},
};

static int cls_wifi_soc_init(void)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	return platform_driver_register(&cls_wifi_soc_driver);
}

static void cls_wifi_soc_exit(void)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	platform_driver_unregister(&cls_wifi_soc_driver);
}

MODULE_FIRMWARE(CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_AGC_FW_NAME);
module_init(cls_wifi_soc_init);
module_exit(cls_wifi_soc_exit);
MODULE_LICENSE("GPL");
