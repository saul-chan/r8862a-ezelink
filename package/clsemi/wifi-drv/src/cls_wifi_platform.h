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

#ifndef _CLS_WIFI_PLATFORM_H_
#define _CLS_WIFI_PLATFORM_H_

#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/nl80211.h>
#ifdef CFG_PCIE_SHM
#include "cls_wifi_pci_shm.h"
#endif

#define CLS_WIFI_CONFIG_FW_NAME			 "cls_wifi_settings.ini"
#define CLS_WIFI_AGC_FW_NAME				"agcram.bin"
#define CLS_WIFI_AGC_FW_2G_NAME			"agcram_2g.bin"

/**
 * Type of memory to access (cf cls_wifi_plat.get_address)
 *
 * @CLS_WIFI_ADDR_CPU To access memory of the embedded CPU
 * @CLS_WIFI_ADDR_SYSTEM To access memory/registers of one subsystem of the
 * embedded system
 * @CLS_WIFI_ADDR_IRF To access specific memory reserved for IRF
 *
 */
enum cls_wifi_platform_addr {
	CLS_WIFI_ADDR_CPU,
	CLS_WIFI_ADDR_SHARED,
	CLS_WIFI_ADDR_SYSTEM,
	CLS_WIFI_ADDR_IPC_IN,
	CLS_WIFI_ADDR_IPC_OUT,
	CLS_WIFI_ADDR_LSYS,
	CLS_WIFI_ADDR_MSGQ,
	CLS_WIFI_ADDR_IRF,
	CLS_WIFI_ADDR_IRF_PHY,
	CLS_WIFI_ADDR_IRF_TBL,
	CLS_WIFI_ADDR_IRF_TBL_PHY,
	CLS_WIFI_ADDR_RIU,
	CLS_WIFI_ADDR_IRF_SND_SMP,
	CLS_WIFI_ADDR_IRF_SND_SMP_PHY,
	CLS_WIFI_ADDR_MAX,
};

struct cls_wifi_hw;

#define CLS_WIFI_MAX_RADIOS	3
#define CLS_WIFI_MAX_IRQS	32

enum cls_wifi_bus_type {
	CLS_WIFI_BUS_TYPE_SOC,
	CLS_WIFI_BUS_TYPE_PCI,
};

enum cls_wifi_hw_rev {
	CLS_WIFI_HW_DUBHE2000,
	CLS_WIFI_HW_MERAK2000,
	CLS_WIFI_HW_MERAK3000,
	CLS_WIFI_HW_MAX_INVALID,
};

enum cls_wifi_band_cap {
	CLS_WIFI_BAND_CAP_2G = 1 << NL80211_BAND_2GHZ,
	CLS_WIFI_BAND_CAP_5G = 1 << NL80211_BAND_5GHZ,
	CLS_WIFI_BAND_CAP_60G = 1 << NL80211_BAND_60GHZ,
	CLS_WIFI_BAND_CAP_6G = 1 << NL80211_BAND_6GHZ,
	CLS_WIFI_BAND_CAP_S1G = 1 << NL80211_BAND_S1GHZ,
};

struct cls_wifi_hw_params {
	u16 hw_rev;
	const char *name;
	const char *firmware_file_name[CLS_WIFI_MAX_RADIOS];
	const char *cali_firmware_file_name[CLS_WIFI_MAX_RADIOS];
	const char *agc_file_name[CLS_WIFI_MAX_RADIOS];
	const char *root_path;
	const char *firmware_path;
	u8 radio_base;
	u8 radio_max;
	u8 max_irqs;
	u8 vdev_max[CLS_WIFI_MAX_RADIOS];
	u8 sta_max[CLS_WIFI_MAX_RADIOS];
	u32 mu_user[CLS_WIFI_MAX_RADIOS];
	u32 irq_start[CLS_WIFI_MAX_RADIOS];
	u32 irq_end[CLS_WIFI_MAX_RADIOS];
	u32 sys_mem_offset[CLS_WIFI_MAX_RADIOS];
	u32 sys_mem_size[CLS_WIFI_MAX_RADIOS];
	u32 shared_mem_offset[CLS_WIFI_MAX_RADIOS];
	u32 shared_mem_size[CLS_WIFI_MAX_RADIOS];
	u32 irf_mem_offset[CLS_WIFI_MAX_RADIOS];
	u32 irf_mem_size[CLS_WIFI_MAX_RADIOS];
	u32 irf_table_mem_offset[CLS_WIFI_MAX_RADIOS];
	u32 irf_table_mem_size[CLS_WIFI_MAX_RADIOS];
	u32 ipc_in_offset[CLS_WIFI_MAX_RADIOS];
	u32 ipc_in_size[CLS_WIFI_MAX_RADIOS];
	u32 ipc_out_offset[CLS_WIFI_MAX_RADIOS];
	u32 ipc_out_size[CLS_WIFI_MAX_RADIOS];
	u32 sys_reg_offset[CLS_WIFI_MAX_RADIOS];
	u32 sys_reg_size[CLS_WIFI_MAX_RADIOS];
	u32 msgq_reg_offset[CLS_WIFI_MAX_RADIOS];
	u32 msgq_reg_size[CLS_WIFI_MAX_RADIOS];
	u32 msgq_txdesc[CLS_WIFI_MAX_RADIOS];
	u32 msgq_rxdesc[CLS_WIFI_MAX_RADIOS];
	u32 msgq_rxbuf[CLS_WIFI_MAX_RADIOS];
	u32 txdesc_cnt0[CLS_WIFI_MAX_RADIOS];
	u32 txdesc_cnt1[CLS_WIFI_MAX_RADIOS];
	u32 txdesc_cnt2[CLS_WIFI_MAX_RADIOS];
	u32 txdesc_cnt3[CLS_WIFI_MAX_RADIOS];
	u32 txdesc_cnt4[CLS_WIFI_MAX_RADIOS];
	u32 txdesc_mu_cnt[CLS_WIFI_MAX_RADIOS];
	u32 riu_mem_offset[CLS_WIFI_MAX_RADIOS];
	u32 riu_mem_size[CLS_WIFI_MAX_RADIOS];
	u32 irf_snd_smp_mem_offset[CLS_WIFI_MAX_RADIOS];
	u32 irf_snd_smp_mem_size[CLS_WIFI_MAX_RADIOS];
	u32 log_offset[CLS_WIFI_MAX_RADIOS];
	u32 log_size[CLS_WIFI_MAX_RADIOS];
	enum cls_wifi_band_cap band_cap[CLS_WIFI_MAX_RADIOS];
};

#ifdef CLS_WIFI_DUBHE_ETH
struct cls_wifi_eth_info {
	bool soft_bmu_en;
	dma_addr_t tx_addr_phy;
	int tx_size_virt;
	void *tx_addr_virt;
};
#endif

struct cls_wifi_irq_task {
	struct cls_wifi_hw *cls_wifi_hw;
	unsigned int irq;
	struct tasklet_struct task;
	void (*cb)(struct cls_wifi_hw *cls_wifi_hw);
};

struct cls_wifi_close_loop_cali_sch {
	u8 cycle;
	u8 cnt;
	u8 exp_radio;
};

struct cls_wifi_dif_sch{
	u8 radio;
	u8 function;
	u8 g_online_zif_en;
	u8 g_pwr_ctrl_en;
	u8 dif_drv_state;
	u8 dif_evt_cmd;
	u8 mode;
	u8 reload_table_flag;
	int dif_sm_pause_cnt;
	int sm_cnt;
	u32 time[CLS_WIFI_MAX_RADIOS];
	u32 online_zif_cycle[CLS_WIFI_MAX_RADIOS];
	s8 tsensor_temp[CLS_WIFI_MAX_RADIOS];
	s8 zif_temp_record[CLS_WIFI_MAX_RADIOS];
	u8 zif_trig_h_thres[CLS_WIFI_MAX_RADIOS];
	u8 zif_trig_l_thres[CLS_WIFI_MAX_RADIOS];
	struct timer_list sch_timer;
	struct timer_list xtal_set_timer;
	struct work_struct dif_work;
	struct work_struct xtal_set_work;

	struct timer_list tsensor_timer;
	struct work_struct tsensor_work;

	void *plat[CLS_WIFI_MAX_RADIOS];
	struct mutex mutex;
	bool sch_is_lock;
	u8 pd_sch_times[CLS_WIFI_MAX_RADIOS];
	u8 g_aci_det_en;
	u32 dif_aci_det_cycle;
	struct cls_wifi_close_loop_cali_sch sch_close_loop_cali;
};

struct cls_wifi_path_info {
	char *cal_path;
	char *irf_path;
};

/**
 * struct cls_wifi_plat - Operation pointers for CLS_WIFI PCI platform
 *
 * @pci_dev: pointer to pci dev
 * @enabled: Set if embedded platform has been enabled (i.e. fw loaded and
 *		  ipc started)
 * @enable: Configure communication with the fw (i.e. configure the transfers
 *		 enable and register interrupt)
 * @disable: Stop communication with the fw
 * @deinit: Free all ressources allocated for the embedded platform
 * @get_address: Return the virtual address to access the requested address on
 *			  the platform.
 * @ack_irq: Acknowledge the irq at link level.
 * @get_config_reg: Return the list (size + pointer) of registers to restore in
 * order to reload the platform while keeping the current configuration.
 *
 * @priv Private data for the link driver
 */
struct cls_wifi_plat {
	enum cls_wifi_bus_type bus_type;
	struct cls_wifi_hw_params hw_params;
	enum cls_wifi_hw_rev hw_rev;
#ifdef CLS_WIFI_DUBHE_ETH
	struct cls_wifi_eth_info eth_info;
#endif
	struct cls_wifi_path_info path_info;
	struct device *dev;
	struct cls_wifi_irq_task irq_task[CLS_WIFI_MAX_RADIOS][CLS_WIFI_MAX_IRQS];
	struct cls_wifi_hw *cls_wifi_hw[CLS_WIFI_MAX_RADIOS];
	struct cls_wifi_cmn_hw *cmn_hw;
	const struct cls_wifi_ep_ops *ep_ops;
	const struct cls_wifi_if_ops *if_ops;
	struct cls_wifi_dif_sch *dif_sch;
	u32 bands_enabled;
	u32 *chip_id;
#ifdef CFG_PCIE_SHM
	pcie_shm_pool_st *pcie_shm_pools[SHM_POOL_IDX_MAX];
	void *irf_tbl_virt_addr;
#endif
	u8 priv[0] __aligned(sizeof(void *));
};

struct cls_wifi_if_ops {
	int (*firmware_on)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, bool final);
	int (*enable)(struct cls_wifi_hw *cls_wifi_hw);
	int (*disable)(struct cls_wifi_hw *cls_wifi_hw);
	u8* (*get_address)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
				int addr_name, unsigned int offset);
	u32 (*get_phy_address)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
				int addr_name, unsigned int offset);
	void (*ack_irq)(struct cls_wifi_plat *cls_wifi_plat);
	int (*get_config_reg)(struct cls_wifi_plat *cls_wifi_plat, const u32 **list);
};

struct cls_wifi_ep_ops {
	u32 (*read32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset);
	void (*write32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value);
	void (*readn)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *dst,
			u32 length);
	void (*writen)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src,
			u32 length);
	void (*cpu_readn)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *dst,
			u32 length);
	void (*cpu_writen)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, void *src,
			u32 length);
	u32 (*sys_read32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset);
	void (*sys_write32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value);
	u32 (*riu_read32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset);
	void (*riu_write32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value);
	u32 (*msgq_read32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset);
	void (*msgq_write32)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u32 value);
#if defined(__aarch64__) || defined(__x86_64__)
	void (*msgq_write64)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset, u64 value);
#endif
	void (*irf_readn)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
			void *dst, u32 length);
	void (*irf_writen)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
			void *src, u32 length);
	void (*irf_table_readn)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
			void *dst, u32 length);
	void (*irf_table_writen)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
			void *src, u32 length);
	void (*irq_enable)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value);
	void (*irq_disable)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value);
	u32 (*irq_get_status)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
	u32 (*irq_get_raw_status)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
	void (*irq_ack)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value);
	void (*irq_trigger)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value);
	void (*irf_snd_smp_readn)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
			void *dst, u32 length);
	void (*irf_snd_smp_writen)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
			void *src, u32 length);
	int (*reload_agc)(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);
	int (*tsensor_get)(struct cls_wifi_plat *cls_wifi_plat);
};

#define CLS_REG_RAW_READ8(base, offset)	(*(volatile u8*)(base + offset))
#define CLS_REG_RAW_WRITE8(base, offset, val)	(*(volatile u8*)(base + offset) = val)

#define CLS_REG_RAW_READ32(base, offset)	(*(volatile u32*)(base + offset))
#define CLS_REG_RAW_WRITE32(base, offset, val)	(*(volatile u32*)(base + offset) = val)

#define CLS_REG_RAW_READ64(base, offset)	(*(volatile u64*)(base + offset))
#define CLS_REG_RAW_WRITE64(base, offset, val)	(*(volatile u64*)(base + offset) = val)

int cls_wifi_platform_init(struct cls_wifi_plat *cls_wifi_plat, void **platform_data, u8 radio_idx);
void cls_wifi_platform_deinit(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_cmn_platform_init(struct cls_wifi_plat *cls_wifi_plat, void **platform_data, u8 radio_idx);
void cls_wifi_cmn_platform_deinit(struct cls_wifi_plat *cls_wifi_plat);

int cls_wifi_platform_on(struct cls_wifi_hw *cls_wifi_hw, void *config);
void cls_wifi_platform_off(struct cls_wifi_hw *cls_wifi_hw, void **config);

#endif /* _CLS_WIFI_PLATFORM_H_ */
