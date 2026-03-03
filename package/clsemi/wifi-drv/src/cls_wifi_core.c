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
#include <linux/slab.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_core.h"

static const struct cls_wifi_hw_params cls_wifi_hw_params[] = {
	{
		.hw_rev = CLS_WIFI_HW_DUBHE2000,
		.name = CLS_WIFI_DUBHE2000_NAME,
		.root_path = CLS_WIFI_DUBHE2000_ROOT_PATH,
		.firmware_path = CLS_WIFI_DUBHE2000_FIRMARE_PATH,
		.radio_base = CLS_WIFI_DUBHE2000_RADIO_BASE,
		.radio_max = CLS_WIFI_DUBHE2000_RADIO_MAX,
		.vdev_max = {CLS_WIFI_DUBHE2000_VDEV_MAX_0, CLS_WIFI_DUBHE2000_VDEV_MAX_1, 0},
		.sta_max = {CLS_WIFI_DUBHE2000_STA_MAX_0, CLS_WIFI_DUBHE2000_STA_MAX_1, 0},
		.band_cap = {CLS_WIFI_BAND_CAP_2G, CLS_WIFI_BAND_CAP_5G, 0},
		.mu_user = {CLS_WIFI_DUBHE2000_MU_USER_0, CLS_WIFI_DUBHE2000_MU_USER_1, 0},
		.txdesc_cnt0 = {TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT0_0), TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT0_1)},
		.txdesc_cnt1 = {TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT1_0), TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT1_1)},
		.txdesc_cnt2 = {TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT2_0), TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT2_1)},
		.txdesc_cnt3 = {TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT3_0), TXDESC_EXPAND(CLS_WIFI_DUBHE2000_TXDESC_CNT3_1)},
		.txdesc_cnt4 = {CLS_WIFI_DUBHE2000_TXDESC_CNT4_0, CLS_WIFI_DUBHE2000_TXDESC_CNT4_1},
		.txdesc_mu_cnt = {CLS_WIFI_DUBHE2000_TXDESC_MU_CNT_0, CLS_WIFI_DUBHE2000_TXDESC_MU_CNT_1},
		.firmware_file_name = {CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_DUBHE2000_FWNAME_0,
				CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_DUBHE2000_FWNAME_1,
				CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_DUBHE2000_FWNAME_C},
		.agc_file_name = {CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_DUBHE2000_AGCNAME_0,
				CLS_WIFI_DUBHE2000_FIRMARE_PATH CLS_WIFI_DUBHE2000_AGCNAME_1},
		.sys_mem_offset = {CLS_WIFI_DUBHE2000_SYS_OFFSET_0, CLS_WIFI_DUBHE2000_SYS_OFFSET_1, CLS_WIFI_DUBHE2000_SYS_OFFSET_C},
		.sys_mem_size = {CLS_WIFI_DUBHE2000_SYS_SIZE_0, CLS_WIFI_DUBHE2000_SYS_SIZE_1, CLS_WIFI_DUBHE2000_SYS_SIZE_C},
		.shared_mem_offset = {CLS_WIFI_DUBHE2000_MEM_OFFSET_0, CLS_WIFI_DUBHE2000_MEM_OFFSET_1, CLS_WIFI_DUBHE2000_MEM_OFFSET_C},
		.shared_mem_size = {CLS_WIFI_DUBHE2000_MEM_SIZE_0, CLS_WIFI_DUBHE2000_MEM_SIZE_1, CLS_WIFI_DUBHE2000_MEM_SIZE_C},
		.ipc_in_offset = {CLS_WIFI_DUBHE2000_IPCI_OFFSET_0, CLS_WIFI_DUBHE2000_IPCI_OFFSET_1},
		.ipc_out_offset = {CLS_WIFI_DUBHE2000_IPCO_OFFSET_0, CLS_WIFI_DUBHE2000_IPCO_OFFSET_1},
		.irf_mem_offset = {CLS_WIFI_DUBHE2000_IRF_OFFSET, CLS_WIFI_DUBHE2000_IRF_OFFSET},
		.irf_mem_size = {CLS_WIFI_DUBHE2000_IRF_SIZE, CLS_WIFI_DUBHE2000_IRF_SIZE},
		.irf_table_mem_offset = {CLS_WIFI_DUBHE2000_IRF_TB_OFFSET, CLS_WIFI_DUBHE2000_IRF_TB_OFFSET},
		.irf_table_mem_size = {CLS_WIFI_DUBHE2000_IRF_TB_SIZE, CLS_WIFI_DUBHE2000_IRF_TB_SIZE},
		.sys_reg_offset = {CLS_WIFI_DUBHE2000_REG_OFFSET_0, CLS_WIFI_DUBHE2000_REG_OFFSET_1},
		.sys_reg_size = {CLS_WIFI_DUBHE2000_REG_SIZE_0, CLS_WIFI_DUBHE2000_REG_SIZE_1},
		.riu_mem_offset = {CLS_WIFI_DUBHE2000_RIU_OFFSET_0, CLS_WIFI_DUBHE2000_RIU_OFFSET_1},
		.riu_mem_size = {CLS_WIFI_DUBHE2000_RIU_SIZE_0, CLS_WIFI_DUBHE2000_RIU_SIZE_1},
		.irf_snd_smp_mem_offset = {CLS_WIFI_DUBHE2000_IRF_SND_SMP_OFFSET, CLS_WIFI_DUBHE2000_IRF_SND_SMP_OFFSET},
		.irf_snd_smp_mem_size = {CLS_WIFI_DUBHE2000_IRF_SND_SMP_SIZE, CLS_WIFI_DUBHE2000_IRF_SND_SMP_SIZE},
	},
	{
		.hw_rev = CLS_WIFI_HW_MERAK2000,
		.name = CLS_WIFI_MERAK2000_NAME,
		.root_path = CLS_WIFI_MERAK2000_ROOT_PATH,
		.firmware_path = CLS_WIFI_MERAK2000_FIRMARE_PATH,
		.radio_base = CLS_WIFI_MERAK2000_RADIO_BASE,
		.radio_max = CLS_WIFI_MERAK2000_RADIO_MAX,
		.vdev_max = {CLS_WIFI_MERAK2000_VDEV_MAX_0, CLS_WIFI_MERAK2000_VDEV_MAX_1, 0},
		.sta_max = {CLS_WIFI_MERAK2000_STA_MAX_0,CLS_WIFI_MERAK2000_STA_MAX_1, 0},
		.max_irqs = CLS_WIFI_MERAK2000_IRQ_MAX,
		.band_cap = {CLS_WIFI_BAND_CAP_2G, CLS_WIFI_BAND_CAP_5G},
		.mu_user = {CLS_WIFI_MERAK2000_MU_USER_0, CLS_WIFI_MERAK2000_MU_USER_1},
		.txdesc_cnt0 = {TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT0_0), TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT0_1)},
		.txdesc_cnt1 = {TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT1_0), TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT1_1)},
		.txdesc_cnt2 = {TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT2_0), TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT2_1)},
		.txdesc_cnt3 = {TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT3_0), TXDESC_EXPAND(CLS_WIFI_MERAK2000_TXDESC_CNT3_1)},
		.txdesc_cnt4 = {CLS_WIFI_MERAK2000_TXDESC_CNT4_0, CLS_WIFI_MERAK2000_TXDESC_CNT4_1},
		.txdesc_mu_cnt = {CLS_WIFI_MERAK2000_TXDESC_MU_CNT_0, CLS_WIFI_MERAK2000_TXDESC_MU_CNT_1},
		.irq_start = {CLS_WIFI_MERAK2000_IRQ_START_0, CLS_WIFI_MERAK2000_IRQ_START_1},
		.irq_end = {CLS_WIFI_MERAK2000_IRQ_END_0, CLS_WIFI_MERAK2000_IRQ_END_1},
		.firmware_file_name = {CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_MERAK2000_FWNAME_0,
				CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_MERAK2000_FWNAME_1},
		.cali_firmware_file_name = {CLS_WIFI_MERAK2000_CALI_FW_PATH CLS_WIFI_MERAK2000_FWNAME_0,
				CLS_WIFI_MERAK2000_CALI_FW_PATH CLS_WIFI_MERAK2000_FWNAME_1},
		.agc_file_name = {CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_MERAK2000_AGCNAME_0,
				CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_MERAK2000_AGCNAME_1},
		.shared_mem_offset = {CLS_WIFI_MERAK2000_MEM_OFFSET_0, CLS_WIFI_MERAK2000_MEM_OFFSET_1},
		.shared_mem_size = {CLS_WIFI_MERAK2000_MEM_SIZE_0, CLS_WIFI_MERAK2000_MEM_SIZE_1},
		.msgq_reg_offset = {CLS_WIFI_MERAK2000_MSGQ_OFFSET_0, CLS_WIFI_MERAK2000_MSGQ_OFFSET_1},
		.msgq_reg_size = {CLS_WIFI_MERAK2000_MSGQ_SIZE_0, CLS_WIFI_MERAK2000_MSGQ_SIZE_1},
		.ipc_in_offset = {CLS_WIFI_MERAK2000_IPCI_OFFSET_0,
				CLS_WIFI_MERAK2000_IPCI_OFFSET_1},
		.ipc_out_offset = {CLS_WIFI_MERAK2000_IPCO_OFFSET_0,
				CLS_WIFI_MERAK2000_IPCO_OFFSET_1},
		.irf_mem_offset = {CLS_WIFI_MERAK2000_IRF_OFFSET_0, CLS_WIFI_MERAK2000_IRF_OFFSET_1},
		.irf_mem_size = {CLS_WIFI_MERAK2000_IRF_SIZE_0, CLS_WIFI_MERAK2000_IRF_SIZE_1},
		.irf_snd_smp_mem_offset = {CLS_WIFI_MERAK2000_IRF_SND_SMP_OFFSET_0,
				CLS_WIFI_MERAK2000_IRF_SND_SMP_OFFSET_1},
		.irf_snd_smp_mem_size = {CLS_WIFI_MERAK2000_IRF_SND_SMP_SIZE_0,
				CLS_WIFI_MERAK2000_IRF_SND_SMP_SIZE_1},
		.irf_table_mem_offset = {CLS_WIFI_MERAK2000_IRF_TB_OFFSET_0,
				CLS_WIFI_MERAK2000_IRF_TB_OFFSET_1},
		.irf_table_mem_size = {CLS_WIFI_MERAK2000_IRF_TB_SIZE_0,
				CLS_WIFI_MERAK2000_IRF_TB_SIZE_1},
		.msgq_txdesc = {CLS_WIFI_MERAK2000_MSGQ_TXDESC_0, CLS_WIFI_MERAK2000_MSGQ_TXDESC_1},
		.msgq_rxdesc = {CLS_WIFI_MERAK2000_MSGQ_RXDESC_0, CLS_WIFI_MERAK2000_MSGQ_RXDESC_1},
		.msgq_rxbuf = {CLS_WIFI_MERAK2000_MSGQ_RXBUF_0, CLS_WIFI_MERAK2000_MSGQ_RXBUF_1},
		.log_offset = {CLS_WIFI_MERAK2000_LOG_OFFSET_0, CLS_WIFI_MERAK2000_LOG_OFFSET_1},
		.log_size = {CLS_WIFI_MERAK2000_LOG_SIZE_0, CLS_WIFI_MERAK2000_LOG_SIZE_1},
	},
	{
		.hw_rev = CLS_WIFI_HW_MERAK3000,
		.name = CLS_WIFI_MERAK3000_NAME,
		.root_path = CLS_WIFI_MERAK3000_ROOT_PATH,
		.firmware_path = CLS_WIFI_MERAK3000_FIRMARE_PATH,
		.radio_base = CLS_WIFI_MERAK3000_RADIO_BASE,
		.radio_max = CLS_WIFI_MERAK3000_RADIO_MAX,
		.vdev_max = {CLS_WIFI_MERAK3000_VDEV_MAX_0, CLS_WIFI_MERAK3000_VDEV_MAX_1, 0},
		.sta_max = {CLS_WIFI_MERAK3000_STA_MAX_0, CLS_WIFI_MERAK3000_STA_MAX_1, 0},
		//.cali_vdev_max = {CLS_WIFI_MERAK3000_CALI_VDEV_MAX_0,
		//        CLS_WIFI_MERAK3000_CALI_VDEV_MAX_1, 0},
		//.cali_sta_max = {CLS_WIFI_MERAK3000_CALI_STA_MAX_0,
		//        CLS_WIFI_MERAK3000_CALI_STA_MAX_1, 0},
		//.cali_mu_user = {CLS_WIFI_MERAK3000_CALI_MU_USER_0,
		//        CLS_WIFI_MERAK3000_CALI_MU_USER_1},
		.max_irqs = CLS_WIFI_MERAK3000_IRQ_MAX,
		.band_cap = {CLS_WIFI_BAND_CAP_2G, CLS_WIFI_BAND_CAP_5G},
		.mu_user = {CLS_WIFI_MERAK3000_MU_USER_0, CLS_WIFI_MERAK3000_MU_USER_1},
		.txdesc_cnt0 = {TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT0_0), TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT0_1)},
		.txdesc_cnt1 = {TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT1_0), TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT1_1)},
		.txdesc_cnt2 = {TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT2_0), TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT2_1)},
		.txdesc_cnt3 = {TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT3_0), TXDESC_EXPAND(CLS_WIFI_MERAK3000_TXDESC_CNT3_1)},
		.txdesc_cnt4 = {CLS_WIFI_MERAK3000_TXDESC_CNT4_0, CLS_WIFI_MERAK3000_TXDESC_CNT4_1},
		.txdesc_mu_cnt = {CLS_WIFI_MERAK3000_TXDESC_MU_CNT_0,
			CLS_WIFI_MERAK3000_TXDESC_MU_CNT_1},
		.irq_start = {CLS_WIFI_MERAK3000_IRQ_START_0, CLS_WIFI_MERAK3000_IRQ_START_1},
		.irq_end = {CLS_WIFI_MERAK3000_IRQ_END_0, CLS_WIFI_MERAK3000_IRQ_END_1},
		.firmware_file_name = {CLS_WIFI_MERAK3000_FIRMARE_PATH CLS_WIFI_MERAK3000_FWNAME_0,
				CLS_WIFI_MERAK3000_FIRMARE_PATH CLS_WIFI_MERAK3000_FWNAME_1},
		.cali_firmware_file_name = {
			CLS_WIFI_MERAK3000_CALI_FW_PATH CLS_WIFI_MERAK3000_FWNAME_0,
			CLS_WIFI_MERAK3000_CALI_FW_PATH CLS_WIFI_MERAK3000_FWNAME_1
		},
		.agc_file_name = {CLS_WIFI_MERAK3000_FIRMARE_PATH CLS_WIFI_MERAK3000_AGCNAME_0,
				CLS_WIFI_MERAK3000_FIRMARE_PATH CLS_WIFI_MERAK3000_AGCNAME_1},
		.shared_mem_offset = {CLS_WIFI_MERAK3000_MEM_OFFSET_0,
			CLS_WIFI_MERAK3000_MEM_OFFSET_1},
		.shared_mem_size = {CLS_WIFI_MERAK3000_MEM_SIZE_0,
			CLS_WIFI_MERAK3000_MEM_SIZE_1},
		.msgq_reg_offset = {CLS_WIFI_MERAK3000_MSGQ_OFFSET_0,
			CLS_WIFI_MERAK3000_MSGQ_OFFSET_1},
		.msgq_reg_size = {CLS_WIFI_MERAK3000_MSGQ_SIZE_0, CLS_WIFI_MERAK3000_MSGQ_SIZE_1},
		.ipc_in_offset = {CLS_WIFI_MERAK3000_IPCI_OFFSET_0,
				CLS_WIFI_MERAK3000_IPCI_OFFSET_1},
		.ipc_out_offset = {CLS_WIFI_MERAK3000_IPCO_OFFSET_0,
			CLS_WIFI_MERAK3000_IPCO_OFFSET_1},
		.irf_mem_offset = {CLS_WIFI_MERAK3000_IRF_OFFSET_0,
			CLS_WIFI_MERAK3000_IRF_OFFSET_1},
		.irf_mem_size = {CLS_WIFI_MERAK3000_IRF_SIZE_0, CLS_WIFI_MERAK3000_IRF_SIZE_1},
		.irf_snd_smp_mem_offset = {CLS_WIFI_MERAK3000_IRF_SND_SMP_OFFSET_0,
				CLS_WIFI_MERAK3000_IRF_SND_SMP_OFFSET_1},
		.irf_snd_smp_mem_size = {CLS_WIFI_MERAK3000_IRF_SND_SMP_SIZE_0,
				CLS_WIFI_MERAK3000_IRF_SND_SMP_SIZE_1},
		.irf_table_mem_offset = {CLS_WIFI_MERAK3000_IRF_TB_OFFSET_0,
				CLS_WIFI_MERAK3000_IRF_TB_OFFSET_1},
		.irf_table_mem_size = {CLS_WIFI_MERAK3000_IRF_TB_SIZE_0,
				CLS_WIFI_MERAK3000_IRF_TB_SIZE_1},
		.msgq_txdesc = {CLS_WIFI_MERAK3000_MSGQ_TXDESC_0, CLS_WIFI_MERAK3000_MSGQ_TXDESC_1},
		.msgq_rxdesc = {CLS_WIFI_MERAK3000_MSGQ_RXDESC_0, CLS_WIFI_MERAK3000_MSGQ_RXDESC_1},
		.msgq_rxbuf = {CLS_WIFI_MERAK3000_MSGQ_RXBUF_0, CLS_WIFI_MERAK3000_MSGQ_RXBUF_1},
		.log_offset = {CLS_WIFI_MERAK3000_LOG_OFFSET_0, CLS_WIFI_MERAK3000_LOG_OFFSET_1},
		.log_size = {CLS_WIFI_MERAK3000_LOG_SIZE_0, CLS_WIFI_MERAK3000_LOG_SIZE_1},
	},
};

int cls_wifi_init_hw_params(struct cls_wifi_plat *cls_wifi_plat)
{
	const struct cls_wifi_hw_params *hw_params = NULL;
	int i;

	for (i = 0; i < ARRAY_SIZE(cls_wifi_hw_params); i++) {
		hw_params = &cls_wifi_hw_params[i];

		if (hw_params->hw_rev == cls_wifi_plat->hw_rev)
			break;
	}

	if (i == ARRAY_SIZE(cls_wifi_hw_params)) {
		dev_err(cls_wifi_plat->dev, "Unsupported hardware version: 0x%x\n", cls_wifi_plat->hw_rev);
		return -EINVAL;
	}

	cls_wifi_plat->hw_params = *hw_params;

	dev_err(cls_wifi_plat->dev, "%s\n", cls_wifi_plat->hw_params.name);
	return 0;
}
EXPORT_SYMBOL(cls_wifi_init_hw_params);

#ifdef CLS_WIFI_DUBHE_ETH
void cls_wifi_init_eth_info(struct cls_wifi_plat *cls_wifi_plat)
{
	void (*eth_tx_info)(bool *soft_bmu_en, dma_addr_t *body_dma, int *vaddr_size, void **hw_data);

	eth_tx_info = symbol_get(cls_npe_eth_tx_info_get);
	if (eth_tx_info) {
		eth_tx_info(&cls_wifi_plat->eth_info.soft_bmu_en, &cls_wifi_plat->eth_info.tx_addr_phy,
				&cls_wifi_plat->eth_info.tx_size_virt, &cls_wifi_plat->eth_info.tx_addr_virt);
		dev_err(cls_wifi_plat->dev, "%s %d, symbol cls_npe_eth_tx_info_get found\n",
				__func__, __LINE__);
		dev_err(cls_wifi_plat->dev, "%s %d, soft_bmu %d, addr_phy %pad, size %x, addr_virt %p\n",
				__func__, __LINE__, cls_wifi_plat->eth_info.soft_bmu_en,
				&cls_wifi_plat->eth_info.tx_addr_phy, cls_wifi_plat->eth_info.tx_size_virt,
				cls_wifi_plat->eth_info.tx_addr_virt);
		symbol_put(cls_npe_eth_tx_info_get);
	} else {
		cls_wifi_plat->eth_info.soft_bmu_en = true;
		dev_err(cls_wifi_plat->dev, "%s %d, symbol cls_npe_eth_tx_info_get not found\n",
				__func__, __LINE__);
	}
}
EXPORT_SYMBOL(cls_wifi_init_eth_info);
#endif
