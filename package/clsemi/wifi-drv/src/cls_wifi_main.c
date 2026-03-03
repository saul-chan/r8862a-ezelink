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
#include <linux/pci.h>
#include <linux/inetdevice.h>
#include <net/cfg80211.h>
#include <net/ip.h>
#include <linux/etherdevice.h>
#ifdef CONFIG_CLS_HIGH_TEMP
#include <linux/thermal/pvt.h>
#endif
#include "cls_wifi_defs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_tx.h"
#include "hal_desc.h"
#include "cls_wifi_debugfs.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_radar.h"
#include "cls_wifi_version.h"
#ifdef CONFIG_CLS_WIFI_BFMER
#include "cls_wifi_bfmer.h"
#endif //(CONFIG_CLS_WIFI_BFMER)
#include "cls_wifi_tdls.h"
#include "cls_wifi_events.h"
#include "cls_wifi_compat.h"
#include "cls_wifi_cali.h"
#include "cls_wifi_main.h"
#include "vendor.h"
#include "vbss.h"
#include "cls_wifi_dif_sm.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_heartbeat.h"
#include "cls_wifi_power_tbl.h"
#include "nac.h"

uint32_t bands_enable = CLS_WIFI_BAND_CAP_2G | CLS_WIFI_BAND_CAP_5G;
module_param(bands_enable, uint, 0644);
EXPORT_SYMBOL(bands_enable);

uint32_t bands_reverse = 0;
module_param(bands_reverse, uint, 0644);
EXPORT_SYMBOL(bands_reverse);

uint32_t process_logs = 0;
module_param(process_logs, uint, 0644);

uint32_t use_msgq;
module_param(use_msgq, uint, 0644);
EXPORT_SYMBOL(use_msgq);
uint32_t tx_wq = 0;
module_param(tx_wq, uint, 0644);

uint32_t txfree_wq = 0;
module_param(txfree_wq, uint, 0644);

uint32_t txq_ctrl_stop = 200;
module_param(txq_ctrl_stop, uint, 0644);

uint32_t txq_ctrl_start = 100;
module_param(txq_ctrl_start, uint, 0644);

uint32_t sigma_enable = 0;
module_param(sigma_enable, uint, 0644);

struct cls_wifi_anti_attack cls_anti_attack;
struct list_head g_cls_wifi_hw_list;
struct mutex g_cls_wifi_hw_mutex;
#define CLS_WIFI_DRV_DESCRIPTION  "Clourney Semiconductor 11nax driver for Linux cfg80211"
#define CLS_WIFI_DRV_COPYRIGHT	"Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved."

#define CLS_WIFI_PRINT_CFM_ERR(req) \
		printk(KERN_CRIT "%s: Status Error(%d)\n", #req, (&req##_cfm)->status)

#define CLS_WIFI_HT_CAPABILITIES									\
{															   \
	.ht_supported   = true,									 \
	.cap			= 0,										\
	.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,			   \
	.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,			 \
	.mcs		= {											 \
		.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },		\
		.rx_highest = cpu_to_le16(65),						  \
		.tx_params = IEEE80211_HT_MCS_TX_DEFINED,			   \
	},														  \
}

#define CLS_WIFI_VHT_CAPABILITIES								   \
{															   \
	.vht_supported = false,									 \
	.cap		   = 0,										 \
	.vht_mcs	   = {										  \
		.rx_mcs_map = 0,										\
		.rx_highest = 0,										\
		.tx_mcs_map = 0,										\
		.tx_highest = 0,										\
	}														   \
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
#define CLS_WIFI_HE_CAPABILITIES									\
{															   \
	.has_he = false,											\
	.he_cap_elem = {											\
		.mac_cap_info[0] = 0,								   \
		.mac_cap_info[1] = 0,								   \
		.mac_cap_info[2] = 0,								   \
		.mac_cap_info[3] = 0,								   \
		.mac_cap_info[4] = 0,								   \
		.mac_cap_info[5] = 0,								   \
		.phy_cap_info[0] = 0,								   \
		.phy_cap_info[1] = 0,								   \
		.phy_cap_info[2] = 0,								   \
		.phy_cap_info[3] = 0,								   \
		.phy_cap_info[4] = 0,								   \
		.phy_cap_info[5] = 0,								   \
		.phy_cap_info[6] = 0,								   \
		.phy_cap_info[7] = 0,								   \
		.phy_cap_info[8] = 0,								   \
		.phy_cap_info[9] = 0,								   \
		.phy_cap_info[10] = 0,								  \
	},														  \
	.he_mcs_nss_supp = {										\
		.rx_mcs_80 = cpu_to_le16(0xfffa),					   \
		.tx_mcs_80 = cpu_to_le16(0xfffa),					   \
		.rx_mcs_160 = cpu_to_le16(0xffff),					  \
		.tx_mcs_160 = cpu_to_le16(0xffff),					  \
		.rx_mcs_80p80 = cpu_to_le16(0xffff),					\
		.tx_mcs_80p80 = cpu_to_le16(0xffff),					\
	},														  \
	.ppe_thres = {0x00},										\
}

#define CLS_WIFI_HE_6G_CAPABILITIES								 \
{															   \
	.capa = 0,												  \
}
#endif

#define RATE(_bitrate, _hw_rate, _flags) {	  \
	.bitrate	= (_bitrate),				   \
	.flags	  = (_flags),					 \
	.hw_value   = (_hw_rate),				   \
}

#define CHAN(_freq) {						   \
	.center_freq	= (_freq),				  \
	.max_power  = 30, /* FIXME */			   \
}

static struct ieee80211_rate cls_wifi_ratetable[] = {
	RATE(10,  0x00, 0),
	RATE(20,  0x01, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(55,  0x02, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(110, 0x03, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(60,  0x04, 0),
	RATE(90,  0x05, 0),
	RATE(120, 0x06, 0),
	RATE(180, 0x07, 0),
	RATE(240, 0x08, 0),
	RATE(360, 0x09, 0),
	RATE(480, 0x0A, 0),
	RATE(540, 0x0B, 0),
};

static struct ieee80211_rate cls_wifi_5ghz_ratetable[] = {
	RATE(10,  0x00, 0),
	RATE(20,  0x01, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(55,  0x02, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(110, 0x03, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(60,  0x04, 0),
	RATE(90,  0x05, 0),
	RATE(120, 0x06, 0),
	RATE(180, 0x07, 0),
	RATE(240, 0x08, 0),
	RATE(360, 0x09, 0),
	RATE(480, 0x0A, 0),
	RATE(540, 0x0B, 0),
};


/* The channels indexes here are not used anymore */
static struct ieee80211_channel cls_wifi_2ghz_channels[] = {
	CHAN(2412),
	CHAN(2417),
	CHAN(2422),
	CHAN(2427),
	CHAN(2432),
	CHAN(2437),
	CHAN(2442),
	CHAN(2447),
	CHAN(2452),
	CHAN(2457),
	CHAN(2462),
	CHAN(2467),
	CHAN(2472),
	CHAN(2484),
	// Extra channels defined only to be used for PHY measures.
	// Enabled only if custregd and custchan parameters are set
	CHAN(2390),
	CHAN(2400),
	CHAN(2410),
	CHAN(2420),
	CHAN(2430),
	CHAN(2440),
	CHAN(2450),
	CHAN(2460),
	CHAN(2470),
	CHAN(2480),
	CHAN(2490),
	CHAN(2500),
	CHAN(2510),
};

static struct ieee80211_channel cls_wifi_5ghz_channels[] = {
	CHAN(5160),			 // 32 -   20MHz
	CHAN(5180),			 // 36 -   20MHz
	CHAN(5200),			 // 40 -   20MHz
	CHAN(5220),			 // 44 -   20MHz
	CHAN(5240),			 // 48 -   20MHz
	CHAN(5260),			 // 52 -   20MHz
	CHAN(5280),			 // 56 -   20MHz
	CHAN(5300),			 // 60 -   20MHz
	CHAN(5320),			 // 64 -   20MHz
	CHAN(5340),			 // 68 -   20MHz
	CHAN(5500),			 // 100 -  20MHz
	CHAN(5520),			 // 104 -  20MHz
	CHAN(5540),			 // 108 -  20MHz
	CHAN(5560),			 // 112 -  20MHz
	CHAN(5580),			 // 116 -  20MHz
	CHAN(5600),			 // 120 -  20MHz
	CHAN(5620),			 // 124 -  20MHz
	CHAN(5640),			 // 128 -  20MHz
	CHAN(5660),			 // 132 -  20MHz
	CHAN(5680),			 // 136 -  20MHz
	CHAN(5700),			 // 140 -  20MHz
	CHAN(5720),			 // 144 -  20MHz
	CHAN(5745),			 // 149 -  20MHz
	CHAN(5765),			 // 153 -  20MHz
	CHAN(5785),			 // 157 -  20MHz
	CHAN(5805),			 // 161 -  20MHz
	CHAN(5825),			 // 165 -  20MHz
	CHAN(5845),			 // 168 -  20MHz
	CHAN(5865),			 // 173 -  20MHz
	CHAN(5885),			 // 177 -  20MHz
	// Extra channels defined only to be used for PHY measures.
	// Enabled only if custregd and custchan parameters are set
	CHAN(5190),
	CHAN(5210),
	CHAN(5230),
	CHAN(5250),
	CHAN(5270),
	CHAN(5290),
	CHAN(5310),
	CHAN(5330),
	CHAN(5340),
	CHAN(5350),
	CHAN(5360),
	CHAN(5370),
	CHAN(5380),
	CHAN(5390),
	CHAN(5400),
	CHAN(5410),
	CHAN(5420),
	CHAN(5430),
	CHAN(5440),
	CHAN(5450),
	CHAN(5460),
	CHAN(5470),
	CHAN(5480),
	CHAN(5490),
	CHAN(5510),
	CHAN(5530),
	CHAN(5550),
	CHAN(5570),
	CHAN(5590),
	CHAN(5610),
	CHAN(5630),
	CHAN(5650),
	CHAN(5670),
	CHAN(5690),
	CHAN(5710),
	CHAN(5730),
	CHAN(5750),
	CHAN(5760),
	CHAN(5770),
	CHAN(5780),
	CHAN(5790),
	CHAN(5800),
	CHAN(5810),
	CHAN(5820),
	CHAN(5830),
	CHAN(5840),
	CHAN(5850),
	CHAN(5860),
	CHAN(5870),
	CHAN(5880),
	CHAN(5890),
	CHAN(5900),
	CHAN(5910),
	CHAN(5920),
	CHAN(5930),
	CHAN(5940),
	CHAN(5950),
	CHAN(5960),
	CHAN(5970),
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
static struct ieee80211_sband_iftype_data cls_wifi_he_capa[] = {
	{
		.types_mask = BIT(NL80211_IFTYPE_STATION),
		.he_cap = CLS_WIFI_HE_CAPABILITIES,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		.he_6ghz_capa = CLS_WIFI_HE_6G_CAPABILITIES,
#endif
	},
	{
		.types_mask = BIT(NL80211_IFTYPE_AP),
		.he_cap = CLS_WIFI_HE_CAPABILITIES,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		.he_6ghz_capa = CLS_WIFI_HE_6G_CAPABILITIES,
#endif
	},
};

static struct ieee80211_sband_iftype_data cls_wifi_5ghz_he_capa[] = {
	{
		.types_mask = BIT(NL80211_IFTYPE_STATION),
		.he_cap = CLS_WIFI_HE_CAPABILITIES,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		.he_6ghz_capa = CLS_WIFI_HE_6G_CAPABILITIES,
#endif
	},
	{
		.types_mask = BIT(NL80211_IFTYPE_AP),
		.he_cap = CLS_WIFI_HE_CAPABILITIES,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		.he_6ghz_capa = CLS_WIFI_HE_6G_CAPABILITIES,
#endif
	},
};
#endif

static struct ieee80211_supported_band cls_wifi_band_2GHz = {
	.channels   = cls_wifi_2ghz_channels,
	.n_channels = ARRAY_SIZE(cls_wifi_2ghz_channels) - 13, // -13 to exclude extra channels
	.bitrates   = cls_wifi_ratetable,
	.n_bitrates = ARRAY_SIZE(cls_wifi_ratetable),
	.ht_cap	 = CLS_WIFI_HT_CAPABILITIES,
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	.iftype_data = cls_wifi_he_capa,
	.n_iftype_data = ARRAY_SIZE(cls_wifi_he_capa),
	#endif
};

static struct ieee80211_supported_band cls_wifi_band_5GHz = {
	.channels   = cls_wifi_5ghz_channels,
	.n_channels = ARRAY_SIZE(cls_wifi_5ghz_channels) - 59, // -59 to exclude extra channels
	.bitrates   = &cls_wifi_5ghz_ratetable[4],
	.n_bitrates = ARRAY_SIZE(cls_wifi_5ghz_ratetable) - 4,
	.ht_cap	 = CLS_WIFI_HT_CAPABILITIES,
	.vht_cap	= CLS_WIFI_VHT_CAPABILITIES,
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	.iftype_data = cls_wifi_5ghz_he_capa,
	.n_iftype_data = ARRAY_SIZE(cls_wifi_5ghz_he_capa),
	#endif
};

void cls_wifi_iface_combinations_init(struct cls_wifi_hw *cls_wifi_hw, uint16_t *array_size)
{

	struct ieee80211_iface_limit *cls_wifi_limits = &cls_wifi_hw->if_cfg80211.limits[0];
	struct ieee80211_iface_limit *cls_wifi_limits_dfs = &cls_wifi_hw->if_cfg80211.limits[1];
	struct ieee80211_iface_combination *cls_wifi_combinations = cls_wifi_hw->if_cfg80211.combinations;

	memset(cls_wifi_hw->if_cfg80211.combinations, 0, sizeof(cls_wifi_hw->if_cfg80211.combinations));
	memset(cls_wifi_hw->if_cfg80211.limits, 0, sizeof(cls_wifi_hw->if_cfg80211.limits));
	*array_size = 1;

	cls_wifi_limits->max = hw_vdev_max(cls_wifi_hw);
	cls_wifi_limits->types = BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_STATION);

	if (cls_wifi_hw->plat->hw_params.band_cap[cls_wifi_hw->radio_idx] != CLS_WIFI_BAND_CAP_2G) {
		cls_wifi_limits_dfs->max = hw_vdev_max(cls_wifi_hw);
		cls_wifi_limits_dfs->types = BIT(NL80211_IFTYPE_AP);
	}

	cls_wifi_combinations[0].limits = cls_wifi_limits;
	cls_wifi_combinations[0].n_limits = 1;
	cls_wifi_combinations[0].num_different_channels = CLS_CHAN_CTXT_CNT;
	cls_wifi_combinations[0].max_interfaces = hw_vdev_max(cls_wifi_hw);

	if (cls_wifi_hw->plat->hw_params.band_cap[cls_wifi_hw->radio_idx] != CLS_WIFI_BAND_CAP_2G) {
		/* Keep this dfs combination as the last one */
		cls_wifi_combinations[1].limits = cls_wifi_limits_dfs;
		cls_wifi_combinations[1].n_limits = 1;
		cls_wifi_combinations[1].num_different_channels = 1;
		cls_wifi_combinations[1].max_interfaces = hw_vdev_max(cls_wifi_hw);
		cls_wifi_combinations[1].radar_detect_widths = (BIT(NL80211_CHAN_WIDTH_20_NOHT) |
									BIT(NL80211_CHAN_WIDTH_20) |
									BIT(NL80211_CHAN_WIDTH_40) |
									BIT(NL80211_CHAN_WIDTH_80));
		*array_size = NUM_CFG80211_COMBINATIONS;
	}
}


/* There isn't a lot of sense in it, but you can transmit anything you like */
static struct ieee80211_txrx_stypes
cls_wifi_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ACTION >> 4) |
			   BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
			   BIT(IEEE80211_STYPE_AUTH >> 4)),
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
			   BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
			   BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
			   BIT(IEEE80211_STYPE_DISASSOC >> 4) |
			   BIT(IEEE80211_STYPE_AUTH >> 4) |
			   BIT(IEEE80211_STYPE_DEAUTH >> 4) |
			   BIT(IEEE80211_STYPE_ACTION >> 4)),
	},
	[NL80211_IFTYPE_AP_VLAN] = {
		/* copy AP */
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
			   BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
			   BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
			   BIT(IEEE80211_STYPE_DISASSOC >> 4) |
			   BIT(IEEE80211_STYPE_AUTH >> 4) |
			   BIT(IEEE80211_STYPE_DEAUTH >> 4) |
			   BIT(IEEE80211_STYPE_ACTION >> 4)),
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ACTION >> 4) |
			   BIT(IEEE80211_STYPE_PROBE_REQ >> 4)),
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
			   BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
			   BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
			   BIT(IEEE80211_STYPE_DISASSOC >> 4) |
			   BIT(IEEE80211_STYPE_AUTH >> 4) |
			   BIT(IEEE80211_STYPE_DEAUTH >> 4) |
			   BIT(IEEE80211_STYPE_ACTION >> 4)),
	},
	[NL80211_IFTYPE_P2P_DEVICE] = {
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ACTION >> 4) |
			   BIT(IEEE80211_STYPE_PROBE_REQ >> 4)),
	},
	[NL80211_IFTYPE_MESH_POINT] = {
		.tx = 0xffff,
		.rx = (BIT(IEEE80211_STYPE_ACTION >> 4) |
			   BIT(IEEE80211_STYPE_AUTH >> 4) |
			   BIT(IEEE80211_STYPE_DEAUTH >> 4)),
	},
};


static u32 cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	0, // reserved entries to enable AES-CMAC, GCMP-128/256, CCMP-256, SMS4
	0,
	0,
	0,
	0,
};

#define NB_RESERVED_CIPHER 5;

static const int cls_wifi_ac2hwq[1][NL80211_NUM_ACS] = {
	{
		[NL80211_TXQ_Q_VO] = CLS_WIFI_HWQ_VO,
		[NL80211_TXQ_Q_VI] = CLS_WIFI_HWQ_VI,
		[NL80211_TXQ_Q_BE] = CLS_WIFI_HWQ_BE,
		[NL80211_TXQ_Q_BK] = CLS_WIFI_HWQ_BK
	}
};

const int cls_wifi_tid2hwq[IEEE80211_NUM_TIDS] = {
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BK,
	CLS_WIFI_HWQ_BK,
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_VI,
	CLS_WIFI_HWQ_VI,
	CLS_WIFI_HWQ_VO,
	CLS_WIFI_HWQ_VO,
	/* TID_8 is used for management frames */
	CLS_WIFI_HWQ_VO,
	/* At the moment, all others TID are mapped to BE */
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BE,
	CLS_WIFI_HWQ_BE,
};

static const int cls_wifi_hwq2uapsd[NL80211_NUM_ACS] = {
	[CLS_WIFI_HWQ_VO] = IEEE80211_WMM_IE_STA_QOSINFO_AC_VO,
	[CLS_WIFI_HWQ_VI] = IEEE80211_WMM_IE_STA_QOSINFO_AC_VI,
	[CLS_WIFI_HWQ_BE] = IEEE80211_WMM_IE_STA_QOSINFO_AC_BE,
	[CLS_WIFI_HWQ_BK] = IEEE80211_WMM_IE_STA_QOSINFO_AC_BK,
};

char cls_wifi_op_mode_name[CLS_WIFI_MODE_MAX + 1][16] = {"AP", "Station", "Repeater"};
/*********************************************************************
 * helper
 *********************************************************************/
/**
 * cls_wifi_get_vif - Retrieve poitner to VIF struct for firmware Index
 * @cls_wifi_hw: Main driver data
 * @vif_idx: VIF index at firmware level.
 *
 * @return pointer to VIF struct or NULL if it doesn't exist
 * Note: It cannot be used, as is, for AP_VLAN interface index as vif_idx
 * is compared against CLS_VIRT_DEV_MAX and not CLS_ITF_MAX.
 */
struct cls_wifi_vif *cls_wifi_get_vif(struct cls_wifi_hw *cls_wifi_hw, unsigned int vif_idx)
{
	struct cls_wifi_vif *vif;
	if (vif_idx >= hw_vdev_max(cls_wifi_hw))
		return NULL;
	vif = cls_wifi_hw->vif_table[vif_idx];
	if (!vif || !vif->up || vif->going_stop)
		return NULL;
	return vif;
}

/**
 * cls_wifi_get_sta - Retrieve STA struct for firmware Index
 * @cls_wifi_hw: Main driver data
 * @sta_idx: STA index at firmware level
 * @vif: VIF associated to the STA. If NULL, station is searched accross all VIFs.
 * @allow_invalid: If true STA doesn't have to be valid.
 *
 * @return pointer to STA struct or NULL if:
 * - it doesn't exist,
 * - it is invalid and allow_invalid is not set
 * - it is valid but doesn't belong to the provided VIF.
 */
struct cls_wifi_sta *cls_wifi_get_sta(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx,
							  struct cls_wifi_vif * vif, bool allow_invalid)
{
	struct cls_wifi_sta *sta;
	if (sta_idx >= hw_all_sta_max(cls_wifi_hw))
		return NULL;

	sta = &cls_wifi_hw->sta_table[sta_idx];
	if (!sta->valid && !allow_invalid)
		return NULL;

	if (vif && sta->valid && (sta->vif_idx != vif->vif_index))
		return NULL;

	return sta;
}

/**
 * cls_wifi_get_sta_from_mac - Retrieve STA struct for MAC address
 * @cls_wifi_hw: Main driver data
 * @mac_addr: STA's MAC address
 *
 * @return pointer to STA struct or NULL if it doesn't exist
 */
struct cls_wifi_vif *cls_wifi_get_vif_from_mac(struct cls_wifi_hw *cls_wifi_hw, const u8 *mac_addr)
{
	int i;
	struct cls_wifi_vif *vif;

	for (i = 0; i < CLS_ITF_MAX; i++) {
		vif = cls_wifi_hw->vif_table[i];
		if (vif && (memcmp(mac_addr, vif->ndev->dev_addr, 6) == 0))
			return vif;
	}

	return NULL;
}

/**
 * cls_wifi_get_sta_from_mac - Retrieve STA struct for MAC address
 * @cls_wifi_hw: Main driver data
 * @mac_addr: STA's MAC address
 *
 * @return pointer to STA struct or NULL if it doesn't exist
 */
struct cls_wifi_sta *cls_wifi_get_sta_from_mac(struct cls_wifi_hw *cls_wifi_hw, const u8 *mac_addr)
{
	int i;

	for (i = 0; i < hw_remote_sta_max(cls_wifi_hw); i++) {
		struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[i];
		if (sta->valid && (memcmp(mac_addr, &sta->mac_addr, 6) == 0))
			return sta;
	}

	return NULL;
}

// return repeater-sta vif
struct cls_wifi_vif *cls_wifi_get_repeater_sta_vif(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_vif *vif, *sta_vif;
	int sta_num = 0, ap_num = 0;

	sta_vif = NULL;

	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		if (!vif)
			continue;

		if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_AP)
			ap_num++;
		else if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_STATION) {
			sta_vif = vif;
			sta_num++;
		}
	}

	if (sta_num == 1 && ap_num)
		return sta_vif;
	else
		return NULL;
}

bool cls_wifi_is_repeater_mode(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->op_mode == CLS_WIFI_REPEATER_MODE);
}

// check if repeater-sta is connected
bool cls_wifi_repeater_is_connected(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_vif *sta_vif = cls_wifi_get_repeater_sta_vif(cls_wifi_hw);

	return (sta_vif && sta_vif->sta.ap && sta_vif->sta.ap->valid);
}

bool cls_wifi_vif_is_active(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, unsigned int vif_idx)
{
    if (vif) {
        if (vif->going_stop)
            return false;
    } else if (cls_wifi_hw) {
        vif = cls_wifi_get_vif(cls_wifi_hw, vif_idx);
        if (vif == NULL)
            return false;

        if (vif->going_stop)
            return false;
    }

    return true;
}

int cls_wifi_freq_to_idx(struct cls_wifi_hw *cls_wifi_hw, int freq)
{
	struct ieee80211_supported_band *sband;
	int band, ch, idx = 0;

	for (band = NL80211_BAND_2GHZ; band < NUM_NL80211_BANDS; band++) {
		sband = &cls_wifi_hw->if_cfg80211.sbands[band];
		if (!sband)
			continue;

		for (ch = 0; ch < sband->n_channels; ch++, idx++) {
			if (sband->channels[ch].center_freq == freq)
				goto exit;
		}
	}

	BUG_ON(1);

exit:
	// Channel has been found, return the index
	return idx;
}

void cls_wifi_enable_wapi(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_hw->if_cfg80211.cipher_suites[cls_wifi_hw->wiphy->n_cipher_suites++] = WLAN_CIPHER_SUITE_SMS4;
	cls_wifi_hw->wiphy->flags |= WIPHY_FLAG_CONTROL_PORT_PROTOCOL;
}

void cls_wifi_enable_mfp(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_hw->if_cfg80211.cipher_suites[cls_wifi_hw->wiphy->n_cipher_suites++] = WLAN_CIPHER_SUITE_AES_CMAC;
}

void cls_wifi_enable_ccmp_256(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_hw->if_cfg80211.cipher_suites[cls_wifi_hw->wiphy->n_cipher_suites++] = WLAN_CIPHER_SUITE_CCMP_256;
}

void cls_wifi_enable_gcmp(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_hw->if_cfg80211.cipher_suites[cls_wifi_hw->wiphy->n_cipher_suites++] = WLAN_CIPHER_SUITE_GCMP;
	cls_wifi_hw->if_cfg80211.cipher_suites[cls_wifi_hw->wiphy->n_cipher_suites++] = WLAN_CIPHER_SUITE_GCMP_256;
}

u8 *cls_wifi_build_bcn(struct cls_wifi_bcn *bcn, struct cfg80211_beacon_data *new)
{
	u8 *buf, *pos;

	if (new->head) {
		u8 *head = kmalloc(new->head_len, GFP_KERNEL);

		if (!head)
			return NULL;

		if (bcn->head)
			kfree(bcn->head);

		bcn->head = head;
		bcn->head_len = new->head_len;
		memcpy(bcn->head, new->head, new->head_len);
	}
	if (new->tail) {
		u8 *tail = kmalloc(new->tail_len, GFP_KERNEL);

		if (!tail)
			return NULL;

		if (bcn->tail)
			kfree(bcn->tail);

		bcn->tail = tail;
		bcn->tail_len = new->tail_len;
		memcpy(bcn->tail, new->tail, new->tail_len);
	}

	if (!bcn->head)
		return NULL;

	bcn->tim_len = 6;
	bcn->len = bcn->head_len + bcn->tail_len + bcn->ies_len + bcn->tim_len;

	buf = kmalloc(bcn->len, GFP_KERNEL);
	if (!buf)
		return NULL;

	// Build the beacon buffer
	pos = buf;
	memcpy(pos, bcn->head, bcn->head_len);
	pos += bcn->head_len;
	*pos++ = WLAN_EID_TIM;
	*pos++ = 4;
	*pos++ = 0;
	*pos++ = bcn->dtim;
	*pos++ = 0;
	*pos++ = 0;
	if (bcn->tail) {
		memcpy(pos, bcn->tail, bcn->tail_len);
		pos += bcn->tail_len;
	}
	if (bcn->ies) {
		memcpy(pos, bcn->ies, bcn->ies_len);
	}

	return buf;
}

static void cls_wifi_del_bcn(struct cls_wifi_bcn *bcn)
{
	if (bcn->head) {
		kfree(bcn->head);
		bcn->head = NULL;
	}
	bcn->head_len = 0;

	if (bcn->tail) {
		kfree(bcn->tail);
		bcn->tail = NULL;
	}
	bcn->tail_len = 0;

	if (bcn->ies) {
		kfree(bcn->ies);
		bcn->ies = NULL;
	}
	bcn->ies_len = 0;
	bcn->tim_len = 0;
	bcn->dtim = 0;
	bcn->len = 0;
}

/**
 * Link channel ctxt to a vif and thus increments count for this context.
 */
void cls_wifi_chanctx_link(struct cls_wifi_vif *vif, u8 ch_idx,
					   struct cfg80211_chan_def *chandef)
{
	struct cls_wifi_chanctx *ctxt;

	if (ch_idx >= CLS_CHAN_CTXT_CNT) {
		WARN(1, "Invalid channel ctxt id %d", ch_idx);
		return;
	}

	vif->ch_index = ch_idx;
	ctxt = &vif->cls_wifi_hw->chanctx_table[ch_idx];
	ctxt->count++;

	// For now chandef is NULL for STATION interface
	if (chandef) {
		if (!ctxt->chan_def.chan)
			ctxt->chan_def = *chandef;
		else {
			// TODO. check that chandef is the same as the one already
			// set for this ctxt
		}
	}
}

/**
 * Unlink channel ctxt from a vif and thus decrements count for this context
 */
void cls_wifi_chanctx_unlink(struct cls_wifi_vif *vif)
{
	struct cls_wifi_chanctx *ctxt;

	if (vif == NULL)
		return;

	if (vif->ch_index == CLS_WIFI_CH_NOT_SET)
		return;

	ctxt = &vif->cls_wifi_hw->chanctx_table[vif->ch_index];

	if (ctxt->count == 0) {
		WARN(1, "Chan ctxt ref count is already 0");
	} else {
		ctxt->count--;
	}

	if (ctxt->count == 0) {
		if (vif->ch_index == vif->cls_wifi_hw->cur_chanctx) {
			/* If current chan ctxt is no longer linked to a vif
			   disable radar detection (no need to check if it was activated) */
			cls_wifi_radar_detection_enable(&vif->cls_wifi_hw->radar,
										CLS_WIFI_RADAR_DETECT_DISABLE,
										CLS_WIFI_RADAR_RIU);
		}
		/* set chan to null, so that if this ctxt is relinked to a vif that
		   don't have channel information, don't use wrong information */
		ctxt->chan_def.chan = NULL;
	}
	vif->ch_index = CLS_WIFI_CH_NOT_SET;
}

int cls_wifi_chanctx_valid(struct cls_wifi_hw *cls_wifi_hw, u8 ch_idx)
{
	if (ch_idx >= CLS_CHAN_CTXT_CNT ||
		cls_wifi_hw->chanctx_table[ch_idx].chan_def.chan == NULL) {
		return 0;
	}

	return 1;
}

static void cls_wifi_del_csa(struct cls_wifi_vif *vif)
{
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	struct cls_wifi_csa *csa = vif->ap.csa;

	if (!csa)
		return;

	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &csa->buf);
	cls_wifi_del_bcn(&csa->bcn);
	kfree(csa);
	vif->ap.csa = NULL;
}

static void cls_wifi_csa_finish(struct work_struct *ws)
{
	struct cls_wifi_csa *csa = container_of(ws, struct cls_wifi_csa, work);
	struct cls_wifi_vif *vif = csa->vif;
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	int error = csa->status;

	if (!error)
		error = cls_wifi_send_bcn_change(cls_wifi_hw, vif->vif_index, csa->buf.dma_addr,
									 csa->bcn.len, csa->bcn.head_len,
									 csa->bcn.tim_len, NULL, 0);

	if (error)
		cfg80211_stop_iface(cls_wifi_hw->wiphy, &vif->wdev, GFP_KERNEL);
	else {
		mutex_lock(&vif->wdev.mtx);
		__acquire(&vif->wdev.mtx);
		/* TODO: enable boot cali after optimization done */
		if (!cls_wifi_is_repeater_mode(cls_wifi_hw) && vif->vif_index == 0)
			cls_wifi_hw->csa_fin_flag = true;

		spin_lock_bh(&cls_wifi_hw->cb_lock);
		cls_wifi_chanctx_unlink(vif);
		cls_wifi_chanctx_link(vif, csa->ch_idx, &csa->chandef);
		if (cls_wifi_hw->cur_chanctx == csa->ch_idx) {
			cls_wifi_radar_detection_enable_on_cur_channel(cls_wifi_hw, vif);
			cls_wifi_txq_vif_start(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
		} else
			cls_wifi_txq_vif_stop(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
		spin_unlock_bh(&cls_wifi_hw->cb_lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 2)
		cfg80211_ch_switch_notify(vif->ndev, &csa->chandef);
#else
		cfg80211_ch_switch_notify(vif->ndev, &csa->chandef, 0);
#endif
		mutex_unlock(&vif->wdev.mtx);
		__release(&vif->wdev.mtx);
		/*
		if (cls_wifi_mod_params.dpd_online_en) {
			pr_info("%s dpd_fbdelay_work scheduled in 2000ms\n", __func__);
			cls_wifi_hw->plat->dif_sch->g_pwr_ctrl_en = 0;
			schedule_delayed_work(&cls_wifi_hw->dpd_fbdelay_work, msecs_to_jiffies(DPD_ONLINE_TASK_DELAY_TIME_MS));
		}
		*/
		cls_wifi_sync_pppc_txpower_req(cls_wifi_hw, &csa->chandef);
	}
	cls_wifi_del_csa(vif);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_ARCH_CLOURNEY)
static void cls_wifi_del_cca(struct cls_wifi_vif *vif)
{
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	struct cls_wifi_cca *cca = vif->ap.cca;

	if (!cca)
		return;

	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cca->buf);
	cls_wifi_del_bcn(&cca->bcn);
	kfree(cca);
	vif->ap.cca = NULL;
}

static void cls_wifi_cca_finish(struct work_struct *ws)
{
	struct cls_wifi_cca *cca = container_of(ws, struct cls_wifi_cca, work);
	struct cls_wifi_vif *vif = cca->vif;
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	int error = cca->status;

	if (!error) {
		error = cls_wifi_send_bcn_change(cls_wifi_hw, vif->vif_index, cca->buf.dma_addr,
				cca->bcn.len, cca->bcn.head_len,
				cca->bcn.tim_len, NULL, 0);
	}

	if (error)
		cfg80211_stop_iface(cls_wifi_hw->wiphy, &vif->wdev, GFP_KERNEL);
	else
		cfg80211_color_change_notify(vif->ndev);

	cls_wifi_del_cca(vif);
}
#endif

/**
 * cls_wifi_external_auth_enable - Enable external authentication on a vif
 *
 * @vif: VIF on which external authentication must be enabled
 *
 * External authentication requires to start TXQ for unknown STA in
 * order to send auth frame pusehd by user space.
 * Note: It is assumed that fw is on the correct channel.
 */
void cls_wifi_external_auth_enable(struct cls_wifi_vif *vif)
{
	vif->sta.flags |= CLS_WIFI_STA_EXT_AUTH;
	cls_wifi_txq_unk_vif_init(vif);
	cls_wifi_txq_start(cls_wifi_txq_vif_get(vif, CLS_UNK_TXQ_TYPE), 0);
}

/**
 * cls_wifi_external_auth_disable - Disable external authentication on a vif
 *
 * @vif: VIF on which external authentication must be disabled
 */
void cls_wifi_external_auth_disable(struct cls_wifi_vif *vif)
{
	if (!(vif->sta.flags & CLS_WIFI_STA_EXT_AUTH))
		return;

	vif->sta.flags &= ~CLS_WIFI_STA_EXT_AUTH;
	cls_wifi_txq_unk_vif_deinit(vif);
}

/**
 * cls_wifi_update_mesh_power_mode -
 *
 * @vif: mesh VIF  for which power mode is updated
 *
 * Does nothing if vif is not a mesh point interface.
 * Since firmware doesn't support one power save mode per link select the
 * most "active" power mode among all mesh links.
 * Indeed as soon as we have to be active on one link we might as well be
 * active on all links.
 *
 * If there is no link then the power mode for next peer is used;
 */
void cls_wifi_update_mesh_power_mode(struct cls_wifi_vif *vif)
{
	enum nl80211_mesh_power_mode mesh_pm;
	struct cls_wifi_sta *sta;
	struct mesh_config mesh_conf;
	struct mesh_update_cfm cfm;
	u32 mask;

	if (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MESH_POINT)
		return;

	if (list_empty(&vif->ap.sta_list)) {
		mesh_pm = vif->ap.next_mesh_pm;
	} else {
		mesh_pm = NL80211_MESH_POWER_DEEP_SLEEP;
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			if (sta->valid && (sta->mesh_pm < mesh_pm)) {
				mesh_pm = sta->mesh_pm;
			}
		}
	}

	if (mesh_pm == vif->ap.mesh_pm)
		return;

	mask = BIT(NL80211_MESHCONF_POWER_MODE - 1);
	mesh_conf.power_mode = mesh_pm;

	if (cls_wifi_send_mesh_update_req(vif->cls_wifi_hw, vif, mask, &mesh_conf, &cfm) ||
		cfm.status)
		return;

	vif->ap.mesh_pm = mesh_pm;
}

/**
 * cls_wifi_save_assoc_ie_for_ft - Save association request elements if Fast
 * Transition has been configured.
 *
 * @vif: VIF that just connected
 * @sme: Connection info
 */
void cls_wifi_save_assoc_info_for_ft(struct cls_wifi_vif *vif,
								 struct cfg80211_connect_params *sme)
{
	int ies_len = sme->ie_len + sme->ssid_len + 2;
	u8 *pos;

	if (!vif->sta.ft_assoc_ies) {
		if (!cfg80211_find_ie(WLAN_EID_MOBILITY_DOMAIN, sme->ie, sme->ie_len))
			return;

		vif->sta.ft_assoc_ies_len = ies_len;
		vif->sta.ft_assoc_ies = kmalloc(ies_len, GFP_KERNEL);
	} else if (vif->sta.ft_assoc_ies_len < ies_len) {
		kfree(vif->sta.ft_assoc_ies);
		vif->sta.ft_assoc_ies = kmalloc(ies_len, GFP_KERNEL);
	}

	if (!vif->sta.ft_assoc_ies)
		return;

	// Also save SSID (as an element) in the buffer
	pos = vif->sta.ft_assoc_ies;
	*pos++ = WLAN_EID_SSID;
	*pos++ = sme->ssid_len;
	memcpy(pos, sme->ssid, sme->ssid_len);
	pos += sme->ssid_len;
	memcpy(pos, sme->ie, sme->ie_len);
	vif->sta.ft_assoc_ies_len = ies_len;
}

/**
 * cls_wifi_rsne_to_connect_params - Initialise cfg80211_connect_params from
 * RSN element.
 *
 * @rsne: RSN element
 * @sme: Structure cfg80211_connect_params to initialize
 *
 * The goal is only to initialize enough for cls_wifi_send_sm_connect_req
 */
int cls_wifi_rsne_to_connect_params(const struct element *rsne,
								struct cfg80211_connect_params *sme)
{
	int len = rsne->datalen;
	int clen;
	const u8 *pos = rsne->data ;

	if (len < 8)
		return 1;

	sme->crypto.control_port_no_encrypt = false;
	sme->crypto.control_port = true;
	sme->crypto.control_port_ethertype = cpu_to_be16(ETH_P_PAE);

	pos += 2;
	sme->crypto.cipher_group = ntohl(*((u32 *)pos));
	pos += 4;
	clen = le16_to_cpu(*((u16 *)pos)) * 4;
	pos += 2;
	len -= 8;
	if (len < clen + 2)
		return 1;
	// only need one cipher suite
	sme->crypto.n_ciphers_pairwise = 1;
	sme->crypto.ciphers_pairwise[0] = ntohl(*((u32 *)pos));
	pos += clen;
	len -= clen;

	// no need for AKM
	clen = le16_to_cpu(*((u16 *)pos)) * 4;
	pos += 2;
	len -= 2;
	if (len < clen)
		return 1;
	pos += clen;
	len -= clen;

	if (len < 4)
		return 0;

	pos += 2;
	clen = le16_to_cpu(*((u16 *)pos)) * 16;
	len -= 4;
	if (len > clen)
		sme->mfp = NL80211_MFP_REQUIRED;

	return 0;
}

/**
 * cls_wifi_scan_done - Indicate cfg80211 that scan procedure is complete
 *
 * @cls_wifi_hw: Main driver data
 * @aborted: Whether scan has been aborted
 */
void cls_wifi_scan_done(struct cls_wifi_hw *cls_wifi_hw, bool aborted)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
		struct cfg80211_scan_info info = {
			.aborted = aborted,
		};
#endif

		cls_wifi_dif_mutex_unlock(cls_wifi_hw->plat);

		if (!cls_wifi_hw->scan_request) {
			wiphy_err(cls_wifi_hw->wiphy, "Calling scan done without scan request\n");
			return;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
		cfg80211_scan_done(cls_wifi_hw->scan_request, &info);
#else
		cfg80211_scan_done(cls_wifi_hw->scan_request, aborted);
#endif
		cls_wifi_hw->scan_request = NULL;

}

/*********************************************************************
 * netdev callbacks
 ********************************************************************/
/**
 * int (*ndo_open)(struct net_device *dev);
 *	 This function is called when network device transistions to the up
 *	 state.
 *
 * - Start FW if this is the first interface opened
 * - Add interface at fw level
 */
static int cls_wifi_open(struct net_device *dev)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct mm_add_if_cfm add_if_cfm;
	int error = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	// Check if it is the first opened VIF
	if (cls_wifi_hw->vif_started == 0)
	{
		// Start the FW
	   if ((error = cls_wifi_send_start(cls_wifi_hw)))
		   return error;

	   /* Device is now started */
	   set_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags);
	}

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP_VLAN) {
		/* For AP_vlan use same fw and drv indexes. We ensure that this index
		   will not be used by fw for another vif by taking index >= CLS_VIRT_DEV_MAX */
		add_if_cfm.inst_nbr = cls_wifi_vif->drv_vif_index;
		netif_tx_stop_all_queues(dev);
	} else {
		/* Forward the information to the LMAC,
		 *	 p2p value not used in FMAC configuration, iftype is sufficient */
		if ((error = cls_wifi_send_add_if(cls_wifi_hw, dev->dev_addr,
									  CLS_WIFI_VIF_TYPE(cls_wifi_vif), false, &add_if_cfm)))
			return error;

		if (add_if_cfm.status != 0) {
			CLS_WIFI_PRINT_CFM_ERR(add_if);
			return -EIO;
		}
	}

	/* Save the index retrieved from LMAC */
	spin_lock_bh(&cls_wifi_hw->cb_lock);
	cls_wifi_vif->vif_index = add_if_cfm.inst_nbr;
	cls_wifi_vif->up = true;
	cls_wifi_hw->vif_started++;
	cls_wifi_hw->vif_table[add_if_cfm.inst_nbr] = cls_wifi_vif;
	spin_unlock_bh(&cls_wifi_hw->cb_lock);

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_MONITOR) {
		cls_wifi_hw->monitor_vif = cls_wifi_vif->vif_index;
		if (cls_wifi_vif->ch_index != CLS_WIFI_CH_NOT_SET) {
			//Configure the monitor channel
			error = cls_wifi_send_config_monitor_req(cls_wifi_hw,
												 &cls_wifi_hw->chanctx_table[cls_wifi_vif->ch_index].chan_def,
												 NULL);
		}
	}

	netif_carrier_off(dev);

	return error;
}

/**
 * int (*ndo_stop)(struct net_device *dev);
 *	 This function is called when network device transistions to the down
 *	 state.
 *
 * - Remove interface at fw level
 * - Reset FW if this is the last interface opened
 */
static int cls_wifi_close(struct net_device *dev)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
#ifdef CONFIG_CLS_FWT
	u32 sub_port;
	u16 node_idx, vif_idx;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	netdev_info(dev, "CLOSE");

	cls_wifi_radar_cancel_cac(&cls_wifi_hw->radar);

	/* Abort scan request on the vif */
	if (cls_wifi_hw->scan_request &&
		cls_wifi_hw->scan_request->wdev == &cls_wifi_vif->wdev)
		cls_wifi_scan_done(cls_wifi_hw, true);

	cls_wifi_send_remove_if(cls_wifi_hw, cls_wifi_vif->vif_index);

	if (cls_wifi_hw->roc && (cls_wifi_hw->roc->vif == cls_wifi_vif)) {
		kfree(cls_wifi_hw->roc);
		cls_wifi_hw->roc = NULL;
	}

	/* Ensure that we won't process disconnect ind */
	spin_lock_bh(&cls_wifi_hw->cb_lock);

	cls_wifi_vif->up = false;
	if (netif_carrier_ok(dev)) {
		if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_STATION ||
			CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_P2P_CLIENT) {
			if (cls_wifi_vif->sta.ft_assoc_ies) {
				kfree(cls_wifi_vif->sta.ft_assoc_ies);
				cls_wifi_vif->sta.ft_assoc_ies = NULL;
				cls_wifi_vif->sta.ft_assoc_ies_len = 0;
			}
			cfg80211_disconnected(dev, WLAN_REASON_DEAUTH_LEAVING,
								  NULL, 0, true, GFP_ATOMIC);
#ifdef CONFIG_CLS_FWT
			if (cls_fwt_g_enable) {
				node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, cls_wifi_vif->sta.ap->sta_idx);
				node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

				vif_idx = CLS_IEEE80211_VIF_IDX_MAP(cls_wifi_vif->vif_index);

				sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(
					node_idx, vif_idx, CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE, 0);

				// vlan.0
				br_fdb_delete_by_subport_hook(cls_wifi_vif->ndev, sub_port);
			}
#endif

			if (cls_wifi_vif->sta.ap) {
				cls_wifi_txq_sta_deinit(cls_wifi_hw, cls_wifi_vif->sta.ap);
				cls_wifi_txq_tdls_vif_deinit(cls_wifi_vif);
			}
			netif_tx_stop_all_queues(dev);
			netif_carrier_off(dev);
		} else if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP_VLAN) {
			netif_carrier_off(dev);
		} else {
			netdev_warn(dev, "AP not stopped when disabling interface");
		}
	}

	cls_wifi_hw->vif_table[cls_wifi_vif->vif_index] = NULL;
	spin_unlock_bh(&cls_wifi_hw->cb_lock);

	cls_wifi_chanctx_unlink(cls_wifi_vif);

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_MONITOR)
		cls_wifi_hw->monitor_vif = CLS_WIFI_INVALID_VIF;

	cls_wifi_hw->vif_started--;
	if (cls_wifi_hw->vif_started == 0) {
		/* This also lets both ipc sides remain in sync before resetting */
		cls_wifi_ipc_tx_drain(cls_wifi_hw);

		cls_wifi_send_reset(cls_wifi_hw);

		// Set parameters to firmware
		cls_wifi_send_me_config_req(cls_wifi_hw);

		// Set channel parameters to firmware
		cls_wifi_send_me_chan_config_req(cls_wifi_hw);

#ifdef CONFIG_CLS_WIFI_HEMU_TX
		cls_wifi_send_mm_ul_parameters(cls_wifi_hw);
		cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
#endif

		cls_wifi_dif_sm_set_event(cls_wifi_hw, DIF_EVT_CALI_RESET);

		clear_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags);
	}
	netdev_info(dev, "CLOSE exit");

	return 0;
}

/**
 * struct net_device_stats* (*ndo_get_stats)(struct net_device *dev);
 *	Called when a user wants to get the network device usage
 *	statistics. Drivers must do one of the following:
 *	1. Define @ndo_get_stats64 to fill in a zero-initialised
 *	   rtnl_link_stats64 structure passed by the caller.
 *	2. Define @ndo_get_stats to update a net_device_stats structure
 *	   (which should normally be dev->stats) and return a pointer to
 *	   it. The structure may be changed asynchronously only if each
 *	   field is written atomically.
 *	3. Update dev->stats asynchronously and atomically, and define
 *	   neither operation.
 */
static struct net_device_stats *cls_wifi_get_stats(struct net_device *dev)
{
	struct cls_wifi_vif *vif = netdev_priv(dev);

	return &vif->net_stats;
}

/**
 * u16 (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb,
 *						 struct net_device *sb_dev);
 *	Called to decide which queue to when device supports multiple
 *	transmit queues.
 */
u16 cls_wifi_select_queue(struct net_device *dev, struct sk_buff *skb,
					  struct net_device *sb_dev)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	return cls_wifi_select_txq(cls_wifi_vif, skb);
}

/**
 * int (*ndo_set_mac_address)(struct net_device *dev, void *addr);
 *	This function  is called when the Media Access Control address
 *	needs to be changed. If this interface is not defined, the
 *	mac address can not be changed.
 */
static int cls_wifi_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *sa = addr;
	int ret;

	ret = eth_mac_addr(dev, sa);

	return ret;
}

static const struct net_device_ops cls_wifi_netdev_ops = {
	.ndo_open			   = cls_wifi_open,
	.ndo_stop			   = cls_wifi_close,
	.ndo_start_xmit		 = cls_wifi_start_xmit,
	.ndo_get_stats		  = cls_wifi_get_stats,
	/* txq will be updated in .ndo_start_xmit() */
//	.ndo_select_queue	   = cls_wifi_select_queue,
	.ndo_set_mac_address	= cls_wifi_set_mac_address
//	.ndo_set_features	   = cls_wifi_set_features,
//	.ndo_set_rx_mode		= cls_wifi_set_multicast_list,
};

static const struct net_device_ops cls_wifi_netdev_monitor_ops = {
	.ndo_open			   = cls_wifi_open,
	.ndo_stop			   = cls_wifi_close,
	.ndo_get_stats		  = cls_wifi_get_stats,
	.ndo_set_mac_address	= cls_wifi_set_mac_address,
};

static void cls_wifi_netdev_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->priv_flags &= ~IFF_TX_SKB_SHARING;
	dev->netdev_ops = &cls_wifi_netdev_ops;
#if LINUX_VERSION_CODE <  KERNEL_VERSION(4, 12, 0)
	dev->destructor = free_netdev;
#else
	dev->needs_free_netdev = true;
#endif
	dev->watchdog_timeo = CLS_WIFI_TX_LIFETIME_MS;

	dev->needed_headroom = CLS_WIFI_TX_MAX_HEADROOM;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	dev->needed_headroom = max(dev->needed_headroom,
							   (unsigned short)(sizeof(struct cls_wifi_amsdu_txhdr)
												+ sizeof(struct ethhdr) + 4
												+ sizeof(rfc1042_header) + 2));
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */

	dev->hw_features = 0;
}

/*********************************************************************
 * Cfg80211 callbacks (and helper)
 *********************************************************************/
static struct wireless_dev *cls_wifi_interface_add(struct cls_wifi_hw *cls_wifi_hw,
											   const char *name,
											   unsigned char name_assign_type,
											   enum nl80211_iftype type,
											   struct vif_params *params)
{
	struct net_device *ndev;
	struct cls_wifi_vif *vif;
	int min_idx, max_idx;
	int vif_idx = -1;
	int i;

	// Look for an available VIF
	if (type == NL80211_IFTYPE_AP_VLAN) {
		min_idx = hw_vdev_max(cls_wifi_hw);
		max_idx = CLS_ITF_MAX;
	} else {
		min_idx = 0;
		max_idx = hw_vdev_max(cls_wifi_hw);
	}

	for (i = min_idx; i < max_idx; i++) {
		if ((cls_wifi_hw->avail_idx_map[i / 16]) & BIT(i % 16)) {
			vif_idx = i;
			break;
		}
	}
	if (vif_idx < 0)
		return NULL;

	#ifndef CONFIG_CLS_WIFI_MON_DATA
	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		// Check if monitor interface already exists or type is monitor
		if ((CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_MONITOR) ||
			(type == NL80211_IFTYPE_MONITOR)) {
			wiphy_err(cls_wifi_hw->wiphy,
					"Monitor+Data interface support (MON_DATA) disabled\n");
			return NULL;
		}
	}
	#endif

	/*
	 * To avoid memory overflow,  only one txq will be alloced.
	 * It means that idx of txq in the driver ise inconsistent with that in the kernel.
	 * And netif_stop_subqueue/netif_wake_subqueue cannot be used in the driver.
	 */
	ndev = alloc_netdev_mqs(sizeof(*vif), name, name_assign_type,
							cls_wifi_netdev_setup, 1, 1);
	if (!ndev)
		return NULL;

	vif = netdev_priv(ndev);
	ndev->ieee80211_ptr = &vif->wdev;
	vif->wdev.wiphy = cls_wifi_hw->wiphy;
	vif->cls_wifi_hw = cls_wifi_hw;
	vif->ndev = ndev;
	vif->ndev_fp = NULL;
	vif->drv_vif_index = vif_idx;
	SET_NETDEV_DEV(ndev, wiphy_dev(vif->wdev.wiphy));
	vif->wdev.netdev = ndev;
	vif->wdev.iftype = type;
	vif->up = false;
	vif->going_stop = false;
	vif->ch_index = CLS_WIFI_CH_NOT_SET;
	vif->generation = 0;
	memset(&vif->net_stats, 0, sizeof(vif->net_stats));

#ifdef WIFI_DRV_TXWQ_PERF
	vif->tx_wq_start = false;
	init_completion(&vif->complete);
#endif
	INIT_WORK(&vif->tx_work, cls_wifi_tx_work_handler);
	skb_queue_head_init(&vif->tx_skb_queue);
	vif->txwq_vif_allow = true;

	switch (type) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
#ifdef CONFIG_CLS_3ADDR_BR
		vif->use_3addr_br = true;
		vif->ndev->ieee80211_ptr->use_3addr_br = true;
#endif
#ifdef CONFIG_CLS_FWT
		vif->auto_4addr = false;
		vif->force_4addr = false;
		vif->m2u_3addr_resend = false;
		vif->log_enable = false;
#endif
		vif->dump_pkt = false;
		vif->sta.flags = 0;
		vif->sta.ap = NULL;
		vif->sta.tdls_sta = NULL;
		vif->sta.ft_assoc_ies = NULL;
		vif->sta.ft_assoc_ies_len = 0;
		break;
	case NL80211_IFTYPE_MESH_POINT:
		INIT_LIST_HEAD(&vif->ap.mpath_list);
		INIT_LIST_HEAD(&vif->ap.proxy_list);
		vif->ap.mesh_pm = NL80211_MESH_POWER_ACTIVE;
		vif->ap.next_mesh_pm = NL80211_MESH_POWER_ACTIVE;
		fallthrough;
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_P2P_GO:
#ifdef CONFIG_CLS_FWT
		vif->auto_4addr = true;
		vif->force_4addr = false;
		vif->m2u_3addr_resend = true;
		vif->log_enable = false;
#endif
		vif->dump_pkt = false;
		INIT_LIST_HEAD(&vif->ap.sta_list);
		memset(&vif->ap.bcn, 0, sizeof(vif->ap.bcn));
		vif->ap.flags = 0;
		break;
	case NL80211_IFTYPE_AP_VLAN:
	{
		struct cls_wifi_vif *master_vif;
		bool found = false;
		list_for_each_entry(master_vif, &cls_wifi_hw->vifs, list) {
			if ((CLS_WIFI_VIF_TYPE(master_vif) == NL80211_IFTYPE_AP) &&
				!(!memcmp(master_vif->ndev->dev_addr, params->macaddr,
						   ETH_ALEN))) {
				 found=true;
				 break;
			}
		}

		if (!found)
			goto err;

		 vif->ap_vlan.master = master_vif;
		 vif->ap_vlan.sta_4a = NULL;
		 break;
	}
	case NL80211_IFTYPE_MONITOR:
		ndev->type = ARPHRD_IEEE80211_RADIOTAP;
		ndev->netdev_ops = &cls_wifi_netdev_monitor_ops;
		break;
	default:
		break;
	}

	if (params && is_valid_ether_addr(params->macaddr))
		memcpy(ndev->dev_addr, params->macaddr, ETH_ALEN);

	if (params) {
		vif->use_4addr = params->use_4addr;
		ndev->ieee80211_ptr->use_4addr = params->use_4addr;
	} else
		vif->use_4addr = false;

	//use_4addr is true only for sta and legacy 4addr ap
	if (vif->use_4addr)
		vif->auto_4addr = false;

	if (cfg80211_register_netdevice(ndev))
		goto err;

#ifdef CONFIG_CLS_FWT
	if (cls_wifi_hw->radio_idx == RADIO_2P4G_INDEX)
		ndev->if_port = CLS_FWT_PORT_WLAN0;
	else if (cls_wifi_hw->radio_idx & RADIO_5G_INDEX)
		ndev->if_port = CLS_FWT_PORT_WLAN1;
	else
		ndev->if_port = CLS_FWT_PORT_LHOST;
	pr_info("[%s] %s if_port=%d\n", __func__, ndev->name, ndev->if_port);
#endif

	spin_lock_bh(&cls_wifi_hw->cb_lock);
	list_add_tail(&vif->list, &cls_wifi_hw->vifs);
	spin_unlock_bh(&cls_wifi_hw->cb_lock);
	cls_wifi_hw->avail_idx_map[vif_idx / 16] &= ~BIT(vif_idx % 16);
	cls_wifi_hw->vif_num++;
	pr_warn(">>> %s() vif_idx: %d\n", __func__, vif_idx);

	return &vif->wdev;

err:
	free_netdev(ndev);
	return NULL;
}

/**
 * @brief Retrieve the cls_wifi_sta object allocated for a given MAC address
 * and a given role.
 */
static struct cls_wifi_sta *cls_wifi_retrieve_sta(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_vif *cls_wifi_vif, u8 *addr,
										  __le16 fc, bool ap)
{
	if (ap) {
		/* only deauth, disassoc and action are bufferable MMPDUs */
		bool bufferable = ieee80211_is_deauth(fc) ||
						  ieee80211_is_disassoc(fc) ||
						  ieee80211_is_action(fc);

		/* Check if the packet is bufferable or not */
		if (bufferable)
		{
			/* Check if address is a broadcast or a multicast address */
			if (is_broadcast_ether_addr(addr) || is_multicast_ether_addr(addr)) {
				/* Returned STA pointer */
				struct cls_wifi_sta *cls_wifi_sta = &cls_wifi_hw->sta_table[cls_wifi_vif->ap.bcmc_index];

				if (cls_wifi_sta->valid)
					return cls_wifi_sta;
			} else {
				/* Returned STA pointer */
				struct cls_wifi_sta *cls_wifi_sta;

				/* Go through list of STAs linked with the provided VIF */
				list_for_each_entry(cls_wifi_sta, &cls_wifi_vif->ap.sta_list, list) {
					if (cls_wifi_sta->valid &&
						ether_addr_equal(cls_wifi_sta->mac_addr, addr)) {
						/* Return the found STA */
						return cls_wifi_sta;
					}
				}
			}
		}
	} else {
		return cls_wifi_vif->sta.ap;
	}

	return NULL;
}

/**
 * @add_virtual_intf: create a new virtual interface with the given name,
 *	must set the struct wireless_dev's iftype. Beware: You must create
 *	the new netdev in the wiphy's network namespace! Returns the struct
 *	wireless_dev, or an ERR_PTR. For P2P device wdevs, the driver must
 *	also set the address member in the wdev.
 */
static struct wireless_dev *cls_wifi_cfg80211_add_iface(struct wiphy *wiphy,
													const char *name,
													unsigned char name_assign_type,
													enum nl80211_iftype type,
													struct vif_params *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct wireless_dev *wdev;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	pr_warn(">>> %s()\n", __func__);

	wdev = cls_wifi_interface_add(cls_wifi_hw, name, name_assign_type, type, params);
	pr_warn(">>> %s() exit\n", __func__);

	if (!wdev)
		return ERR_PTR(-EINVAL);

	return wdev;
}

/**
 * @del_virtual_intf: remove the virtual interface
 */
static int cls_wifi_cfg80211_del_iface(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct net_device *dev = wdev->netdev;
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct sk_buff *skb;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	netdev_info(dev, "Remove Interface");

	cls_wifi_vif->going_stop = true;

#if WIFI_DRV_OPTI_SPIN_LOCK_CFG
	cls_wifi_vif->txwq_vif_allow = false;
	spin_lock_bh(&cls_wifi_hw->txwq_lock);
	cancel_work_sync(&cls_wifi_vif->tx_work);
	spin_unlock_bh(&cls_wifi_hw->txwq_lock);

	///do free
	while ((skb = skb_dequeue(&cls_wifi_vif->tx_skb_queue)) != NULL) {
		consume_skb(skb);
	}
#endif
	pr_warn(">>> %s() drv_vif_index: %u\n", __func__, cls_wifi_vif->drv_vif_index);

	if (dev->reg_state == NETREG_REGISTERED) {
		/* Will call cls_wifi_close if interface is UP */
#if LINUX_VERSION_CODE <  KERNEL_VERSION(5, 11, 0)
		unregister_netdevice(dev);
#else
		cfg80211_unregister_netdevice(dev);
#endif
	} else
		pr_warn(">>>[error] %s() drv_vif_index: %u\n", __func__, cls_wifi_vif->drv_vif_index);

	spin_lock_bh(&cls_wifi_hw->cb_lock);
	list_del(&cls_wifi_vif->list);
	spin_unlock_bh(&cls_wifi_hw->cb_lock);
	cls_wifi_hw->avail_idx_map[cls_wifi_vif->drv_vif_index / 16] |= BIT(cls_wifi_vif->drv_vif_index % 16);
	cls_wifi_vif->ndev = NULL;
	if (cls_wifi_hw->vif_num)
		cls_wifi_hw->vif_num--;

	/* Clear the priv in adapter */
	dev->ieee80211_ptr = NULL;
	pr_warn(">>> %s() drv_vif_index: %u exit\n", __func__, cls_wifi_vif->drv_vif_index);

	return 0;
}

/**
 * @change_virtual_intf: change type/configuration of virtual interface,
 *	keep the struct wireless_dev's iftype updated.
 */
static int cls_wifi_cfg80211_change_iface(struct wiphy *wiphy,
									  struct net_device *dev,
									  enum nl80211_iftype type,
									  struct vif_params *params)
{
#ifndef CONFIG_CLS_WIFI_MON_DATA
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
#endif
	struct cls_wifi_vif *vif = netdev_priv(dev);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* FIXME: any other params want to be changed for sta interface in up state? */
	if (vif->up && type == NL80211_IFTYPE_STATION && params->use_4addr != -1) {
		vif->use_4addr = params->use_4addr;
		vif->auto_4addr = false;
		return 0;
	}

	if (vif->up)
		return (-EBUSY);

#ifndef CONFIG_CLS_WIFI_MON_DATA
	if ((type == NL80211_IFTYPE_MONITOR) &&
	   (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MONITOR)) {
		struct cls_wifi_vif *vif_el;
		list_for_each_entry(vif_el, &cls_wifi_hw->vifs, list) {
			// Check if data interface already exists
			if ((vif_el != vif) &&
			   (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MONITOR)) {
				wiphy_err(cls_wifi_hw->wiphy,
						"Monitor+Data interface support (MON_DATA) disabled\n");
				return -EIO;
			}
		}
	}
#endif

	// Reset to default case (i.e. not monitor)
	dev->type = ARPHRD_ETHER;
	dev->netdev_ops = &cls_wifi_netdev_ops;

	switch (type) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
#ifdef CONFIG_CLS_FWT
		vif->auto_4addr = false;
		vif->force_4addr = false;
		vif->m2u_3addr_resend = false;
		vif->log_enable = false;
#endif
		vif->dump_pkt = false;
		vif->sta.flags = 0;
		vif->sta.ap = NULL;
		vif->sta.tdls_sta = NULL;
		vif->sta.ft_assoc_ies = NULL;
		vif->sta.ft_assoc_ies_len = 0;
		break;
	case NL80211_IFTYPE_MESH_POINT:
		INIT_LIST_HEAD(&vif->ap.mpath_list);
		INIT_LIST_HEAD(&vif->ap.proxy_list);
		fallthrough;
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_P2P_GO:
#ifdef CONFIG_CLS_FWT
		vif->auto_4addr = true;
		vif->force_4addr = false;
		vif->m2u_3addr_resend = true;
		vif->log_enable = false;
#endif
		vif->dump_pkt = false;
		INIT_LIST_HEAD(&vif->ap.sta_list);
		memset(&vif->ap.bcn, 0, sizeof(vif->ap.bcn));
		vif->ap.flags = 0;
		break;
	case NL80211_IFTYPE_AP_VLAN:
		return -EPERM;
	case NL80211_IFTYPE_MONITOR:
		dev->type = ARPHRD_IEEE80211_RADIOTAP;
		dev->netdev_ops = &cls_wifi_netdev_monitor_ops;
		break;
	default:
		break;
	}

	vif->generation = 0;
	vif->wdev.iftype = type;
	if (params->use_4addr != -1)
		vif->use_4addr = params->use_4addr;

	//use_4addr is true only for sta and legacy 4addr ap
	if (vif->use_4addr)
		vif->auto_4addr = false;

	return 0;
}

/**
 * @scan: Request to do a scan. If returning zero, the scan request is given
 *	the driver, and will be valid until passed to cfg80211_scan_done().
 *	For scan results, call cfg80211_inform_bss(); you can call this outside
 *	the scan/scan_done bracket too.
 */
static int cls_wifi_cfg80211_scan(struct wiphy *wiphy,
							  struct cfg80211_scan_request *request)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = container_of(request->wdev, struct cls_wifi_vif, wdev);
	int error;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Scan & roc are incompatibile, reject concurrent request */
	if (cls_wifi_hw->roc)
		return -EBUSY;

	if ((error = cls_wifi_send_scanu_req(cls_wifi_hw, cls_wifi_vif, request)))
		return error;

	cls_wifi_hw->scan_request = request;
	cls_wifi_dif_mutex_lock(cls_wifi_hw->plat);
	return 0;
}

/**
 * @abort_scan: Tell the driver to abort an ongoing scan. The driver shall
 *	indicate the status of the scan through cfg80211_scan_done().
 */
static void cls_wifi_cfg80211_abort_scan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = container_of(wdev, struct cls_wifi_vif, wdev);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw->scan_request || cls_wifi_hw->scan_request->wdev != &cls_wifi_vif->wdev) {
		wiphy_err(wiphy, "Try to abort invalid scan request\n");
		return;
	}

	cls_wifi_send_scanu_abort_req(cls_wifi_hw, cls_wifi_vif);
}

/**
 * @add_key: add a key with the given parameters. @mac_addr will be %NULL
 *	when adding a group key.
 */
static int cls_wifi_cfg80211_add_key(struct wiphy *wiphy, struct net_device *netdev,
								 u8 key_index, bool pairwise, const u8 *mac_addr,
								 struct key_params *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(netdev);
	int i, error = 0;
	struct mm_key_add_cfm key_add_cfm;
	u8_l cipher = 0;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_key *cls_wifi_key;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (mac_addr) {
		sta = cls_wifi_get_sta_from_mac(cls_wifi_hw, mac_addr);
		if (!sta)
			return -EINVAL;
		cls_wifi_key = &sta->key;
	}
	else
		cls_wifi_key = &vif->key[key_index];

	if (sta)
		memcpy(&sta->key_info, params, sizeof(struct key_params));
	/* Retrieve the cipher suite selector */
	switch (params->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
		cipher = MAC_CIPHER_WEP40;
		break;
	case WLAN_CIPHER_SUITE_WEP104:
		cipher = MAC_CIPHER_WEP104;
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		cipher = MAC_CIPHER_TKIP;
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		cipher = MAC_CIPHER_CCMP;
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
		cipher = MAC_CIPHER_BIP_CMAC_128;
		break;
	case WLAN_CIPHER_SUITE_SMS4:
	{
		// Need to reverse key order
		u8 tmp, *key = (u8 *)params->key;
		cipher = MAC_CIPHER_WPI_SMS4;
		for (i = 0; i < WPI_SUBKEY_LEN/2; i++) {
			tmp = key[i];
			key[i] = key[WPI_SUBKEY_LEN - 1 - i];
			key[WPI_SUBKEY_LEN - 1 - i] = tmp;
		}
		for (i = 0; i < WPI_SUBKEY_LEN/2; i++) {
			tmp = key[i + WPI_SUBKEY_LEN];
			key[i + WPI_SUBKEY_LEN] = key[WPI_KEY_LEN - 1 - i];
			key[WPI_KEY_LEN - 1 - i] = tmp;
		}
		break;
	}
	case WLAN_CIPHER_SUITE_GCMP:
		cipher = MAC_CIPHER_GCMP_128;
		break;
	case WLAN_CIPHER_SUITE_GCMP_256:
		cipher = MAC_CIPHER_GCMP_256;
		break;
	case WLAN_CIPHER_SUITE_CCMP_256:
		cipher = MAC_CIPHER_CCMP_256;
		break;
	default:
		return -EINVAL;
	}

	// wait eapol 4/4 to be completed for station mode
	if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_STATION)
#if defined(CFG_M3K_FPGA)
		msleep(max(10, cls_wifi_hw->radio_params->amsdu_agg_timeout / 1000));
#else
		msleep(10);
#endif

	if ((error = cls_wifi_send_key_add(cls_wifi_hw, vif->vif_index,
								   (sta ? sta->sta_idx : CLS_WIFI_INVALID_STA), pairwise,
								   (u8 *)params->key, params->key_len,
								   key_index, cipher, &key_add_cfm)))
		return error;

	if (key_add_cfm.status != 0) {
		CLS_WIFI_PRINT_CFM_ERR(key_add);
		return -EIO;
	}

	/* Save the index retrieved from LMAC */
	cls_wifi_key->hw_idx = key_add_cfm.hw_key_idx;

	return 0;
}

/**
 * @get_key: get information about the key with the given parameters.
 *	@mac_addr will be %NULL when requesting information for a group
 *	key. All pointers given to the @callback function need not be valid
 *	after it returns. This function should return an error if it is
 *	not possible to retrieve the key, -ENOENT if it doesn't exist.
 *
 */
static int cls_wifi_cfg80211_get_key(struct wiphy *wiphy, struct net_device *netdev,
								 u8 key_index, bool pairwise, const u8 *mac_addr,
								 void *cookie,
								 void (*callback)(void *cookie, struct key_params*))
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	return -1;
}


/**
 * @del_key: remove a key given the @mac_addr (%NULL for a group key)
 *	and @key_index, return -ENOENT if the key doesn't exist.
 */
static int cls_wifi_cfg80211_del_key(struct wiphy *wiphy, struct net_device *netdev,
								 u8 key_index, bool pairwise, const u8 *mac_addr)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(netdev);
	int error;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_key *cls_wifi_key;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	if (mac_addr) {
		sta = cls_wifi_get_sta_from_mac(cls_wifi_hw, mac_addr);
		if (!sta)
			return -EINVAL;
		cls_wifi_key = &sta->key;
	}
	else
		cls_wifi_key = &vif->key[key_index];

	error = cls_wifi_send_key_del(cls_wifi_hw, cls_wifi_key->hw_idx);

	return error;
}

/**
 * @set_default_key: set the default key on an interface
 */
static int cls_wifi_cfg80211_set_default_key(struct wiphy *wiphy,
										 struct net_device *netdev,
										 u8 key_index, bool unicast, bool multicast)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	return 0;
}

/**
 * @set_default_mgmt_key: set the default management frame key on an interface
 */
static int cls_wifi_cfg80211_set_default_mgmt_key(struct wiphy *wiphy,
											  struct net_device *netdev,
											  u8 key_index)
{
	return 0;
}

/**
 * @connect: Connect to the ESS with the specified parameters. When connected,
 *	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
 *	If the connection fails for some reason, call cfg80211_connect_result()
 *	with the status from the AP.
 *	(invoked with the wireless_dev mutex held)
 */
static int cls_wifi_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
								 struct cfg80211_connect_params *sme)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct sm_connect_cfm sm_connect_cfm;
	int error = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* For SHARED-KEY authentication, must install key first */
	if (sme->auth_type == NL80211_AUTHTYPE_SHARED_KEY && sme->key)
	{
		struct key_params key_params;
		key_params.key = sme->key;
		key_params.seq = NULL;
		key_params.key_len = sme->key_len;
		key_params.seq_len = 0;
		key_params.cipher = sme->crypto.cipher_group;
		cls_wifi_cfg80211_add_key(wiphy, dev, sme->key_idx, false, NULL, &key_params);
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	else if ((sme->auth_type == NL80211_AUTHTYPE_SAE) &&
			 !(sme->flags & CONNECT_REQ_EXTERNAL_AUTH_SUPPORT)) {
		netdev_err(dev, "Doesn't support SAE without external authentication\n");
		return -EINVAL;
	}
#endif

	//TODO: online_cali/dpd should be enabled on repeater mode
	cls_wifi_dif_boot_cali_clear(cls_wifi_hw);
	cls_wifi_dif_boot_cali(cls_wifi_hw);

	/* Forward the information to the LMAC */
	if ((error = cls_wifi_send_sm_connect_req(cls_wifi_hw, cls_wifi_vif, sme, &sm_connect_cfm)))
		return error;

	// Check the status
	switch (sm_connect_cfm.status)
	{
		case CO_OK:
			cls_wifi_save_assoc_info_for_ft(cls_wifi_vif, sme);
			error = 0;
			break;
		case CO_BUSY:
			error = -EINPROGRESS;
			break;
		case CO_BAD_PARAM:
			error = -EINVAL;
			break;
		case CO_OP_IN_PROGRESS:
			error = -EALREADY;
			break;
		default:
			error = -EIO;
			break;
	}

	return error;
}

/**
 * @disconnect: Disconnect from the BSS/ESS.
 *	(invoked with the wireless_dev mutex held)
 */
static int cls_wifi_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev,
									u16 reason_code)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	int ret;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	ret = cls_wifi_send_sm_disconnect_req(cls_wifi_hw, cls_wifi_vif, reason_code);

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
/**
 * @external_auth: indicates result of offloaded authentication processing from
 *	 user space
 */
static int cls_wifi_cfg80211_external_auth(struct wiphy *wiphy, struct net_device *dev,
									   struct cfg80211_external_auth_params *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);

	if (!(cls_wifi_vif->sta.flags & CLS_WIFI_STA_EXT_AUTH))
		return -EINVAL;

	cls_wifi_external_auth_disable(cls_wifi_vif);
	return cls_wifi_send_sm_external_auth_required_rsp(cls_wifi_hw, cls_wifi_vif,
												   params->status);
}
#endif

/**
 * @add_station: Add a new station.
 */
static int cls_wifi_cfg80211_add_station(struct wiphy *wiphy, struct net_device *dev,
									 const u8 *mac, struct station_parameters *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct me_sta_add_cfm me_sta_add_cfm;
#ifdef CONFIG_CLS_FWT
	u32 sub_port;
	u16 node_idx, vif_idx;
#endif
	int error = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	WARN_ON(CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP_VLAN);

	if (!cls_wifi_vif->ap.ap_started) {
		pr_err("[%s] ap not started: Failed to add sta [%pM]\n", __func__, mac);
		return -EINVAL;
	}

	/* Do not add TDLS station */
	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER))
		return 0;

	/* Indicate we are in a STA addition process - This will allow handling
	 * potential PS mode change indications correctly
	 */
	set_bit(CLS_WIFI_DEV_ADDING_STA, &cls_wifi_hw->flags);

	/* Forward the information to the LMAC */
	if ((error = cls_wifi_send_me_sta_add(cls_wifi_hw, params, mac, cls_wifi_vif->vif_index,
									  &me_sta_add_cfm)))
		return error;

	// Check the status
	switch (me_sta_add_cfm.status)
	{
		case CO_OK:
		{
			struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[me_sta_add_cfm.sta_idx];
			int tid;

			memset(sta, 0, sizeof(*sta));
			sta->aid = params->aid;
			sta->sta_idx = me_sta_add_cfm.sta_idx;
			sta->ch_idx = cls_wifi_vif->ch_index;
			sta->vif_idx = cls_wifi_vif->vif_index;
			sta->vlan_idx = sta->vif_idx;
			sta->qos = (params->sta_flags_set & BIT(NL80211_STA_FLAG_WME)) != 0;
			sta->ht = params->ht_capa ? 1 : 0;
			sta->vht = params->vht_capa ? 1 : 0;
			sta->he = params->he_capa ? 1 : 0;
			sta->acm = 0;
			sta->listen_interval = params->listen_interval;

			if (params->local_pm != NL80211_MESH_POWER_UNKNOWN)
				sta->mesh_pm = params->local_pm;
			else
				sta->mesh_pm = cls_wifi_vif->ap.next_mesh_pm;
			cls_wifi_update_mesh_power_mode(cls_wifi_vif);

			for (tid = 0; tid < CLS_NB_TXQ_PER_STA; tid++) {
				int uapsd_bit = cls_wifi_hwq2uapsd[cls_wifi_tid2hwq[tid]];
				if (params->uapsd_queues & uapsd_bit)
					sta->uapsd_tids |= 1 << tid;
				else
					sta->uapsd_tids &= ~(1 << tid);
			}
			memcpy(sta->mac_addr, mac, ETH_ALEN);


			/* Ensure that we won't process PS change or channel switch ind*/
			spin_lock_bh(&cls_wifi_hw->cb_lock);
			cls_wifi_txq_sta_init(cls_wifi_hw, sta, cls_wifi_txq_vif_get_status(cls_wifi_vif));
			spin_lock_bh(&cls_wifi_hw->rosource_lock);
			list_add_tail(&sta->list, &cls_wifi_vif->ap.sta_list);
			spin_unlock_bh(&cls_wifi_hw->rosource_lock);

#ifdef CONFIG_CLS_FWT
			/* sub_port */
			if (cls_fwt_g_enable) {
				// TODO: cls_wifi_hw->radio_idx ??
				node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, sta->sta_idx);
                		node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

				vif_idx = CLS_IEEE80211_VIF_IDX_MAP(cls_wifi_vif->vif_index);

				sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(node_idx, vif_idx,
					CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE, 0);

				br_fdb_update_const_hook(dev, sta->mac_addr, br_port_get_pvid(dev), sub_port);
			}
#endif

			cls_wifi_vif->generation++;
			sta->valid = true;
			cls_wifi_ps_bh_enable(cls_wifi_hw, sta, sta->ps.active || me_sta_add_cfm.pm_state);
			spin_unlock_bh(&cls_wifi_hw->cb_lock);
			cls_wifi_dbgfs_register_sta(cls_wifi_hw, sta);
			cls_wtm_reset_sta_peak_data(sta);

			error = 0;

#ifdef CONFIG_CLS_WIFI_BFMER
			if (cls_wifi_hw->radio_params->bfmer)
				cls_wifi_send_bfmer_enable(cls_wifi_hw, sta, params);
#endif /* CONFIG_CLS_WIFI_BFMER */

			cls_wifi_vif->assoc_sta_count++;
			cls_wifi_add_associated_csi_sta(cls_wifi_hw, sta);

#define PRINT_STA_FLAG(f)	(params->sta_flags_set & BIT(NL80211_STA_FLAG_##f) ? "["#f"]" : "")

			netdev_info(dev, "Add sta %d (%pM) flags=%s%s%s%s%s%s%s",
						sta->sta_idx, mac,
						PRINT_STA_FLAG(AUTHORIZED),
						PRINT_STA_FLAG(SHORT_PREAMBLE),
						PRINT_STA_FLAG(WME),
						PRINT_STA_FLAG(MFP),
						PRINT_STA_FLAG(AUTHENTICATED),
						PRINT_STA_FLAG(TDLS_PEER),
						PRINT_STA_FLAG(ASSOCIATED));
#undef PRINT_STA_FLAG
		sta->sta_flags_set = params->sta_flags_set;
			break;
		}
		default:
			error = -EBUSY;
			break;
	}

	clear_bit(CLS_WIFI_DEV_ADDING_STA, &cls_wifi_hw->flags);

	return error;
}

/**
 * @del_station: Remove a station
 */
static int cls_wifi_cfg80211_del_station(struct wiphy *wiphy,
									 struct net_device *dev,
									 struct station_del_parameters *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_sta *cur, *tmp;
	int error = 0, found = 0;
	const u8 *mac = NULL;
#ifdef CONFIG_CLS_FWT
	u32 sub_port;
	u16 node_idx, vif_idx;
#endif

	if (params)
		mac = params->mac;

	list_for_each_entry_safe(cur, tmp, &cls_wifi_vif->ap.sta_list, list) {
		if ((!mac) || (!memcmp(cur->mac_addr, mac, ETH_ALEN))) {
			netdev_info(dev, "Del sta(%px) %d (%pM) %s", cur, cur->sta_idx, cur->mac_addr, mac ? "" : "cb");
			/* Ensure that we won't process PS change ind */
			spin_lock_bh(&cls_wifi_hw->cb_lock);
			cur->ps.active = false;
			cur->valid = false;
			spin_unlock_bh(&cls_wifi_hw->cb_lock);

			if (cur->vif_idx != cur->vlan_idx) {
				struct cls_wifi_vif *vlan_vif;
				vlan_vif = cls_wifi_hw->vif_table[cur->vlan_idx];
				if (vlan_vif->up) {
					if ((CLS_WIFI_VIF_TYPE(vlan_vif) == NL80211_IFTYPE_AP_VLAN) &&
						(vlan_vif->use_4addr)) {
						vlan_vif->ap_vlan.sta_4a = NULL;
					} else {
						WARN(1, "Deleting sta belonging to VLAN other than AP_VLAN 4A");
					}
				}
			}

			cls_wifi_txq_sta_deinit(cls_wifi_hw, cur);
			error = cls_wifi_send_me_sta_del(cls_wifi_hw, cur->sta_idx, false);
			if ((error != 0) && (error != -EPIPE))
				return error;

#ifdef CONFIG_CLS_WIFI_BFMER
			// Disable Beamformer if supported
			cls_wifi_bfmer_report_del(cls_wifi_hw, cur);
#endif /* CONFIG_CLS_WIFI_BFMER */
			if (cls_wifi_vif->assoc_sta_count)
				cls_wifi_vif->assoc_sta_count--;
			spin_lock_bh(&cls_wifi_hw->rosource_lock);
			list_del(&cur->list);
			spin_unlock_bh(&cls_wifi_hw->rosource_lock);
			cls_wifi_vif->generation++;
			cls_wifi_dbgfs_unregister_sta(cls_wifi_hw, cur);

#ifdef CONFIG_CLS_FWT
			if (cls_fwt_g_enable) {
				// TODO: cls_wifi_hw->radio_idx ??
				node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, cur->sta_idx);

				node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

				vif_idx = CLS_IEEE80211_VIF_IDX_MAP(cls_wifi_vif->vif_index);

				sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(
					node_idx, vif_idx, CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE, 0);

				// vlan.0
				br_fdb_delete_by_subport_hook(dev, sub_port);
			}
#endif

			found ++;
			break;
		}
	}

	if (mac && !found)
		return -ENOENT;

	cls_wifi_update_mesh_power_mode(cls_wifi_vif);

	return 0;
}

/**
 * @change_station: Modify a given station. Note that flags changes are not much
 *	validated in cfg80211, in particular the auth/assoc/authorized flags
 *	might come to the driver in invalid combinations -- make sure to check
 *	them, also against the existing state! Drivers must call
 *	cfg80211_check_station_change() to validate the information.
 */
static int cls_wifi_cfg80211_change_station(struct wiphy *wiphy, struct net_device *dev,
										const u8 *mac, struct station_parameters *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_sta *sta;
#ifdef CONFIG_CLS_FWT
	u32 sub_port;
	u16 node_idx, vif_idx;
#endif

	sta = cls_wifi_get_sta_from_mac(cls_wifi_hw, mac);
	if (!sta)
	{
		/* Add the TDLS station */
		if (params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER))
		{
			struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
			struct me_sta_add_cfm me_sta_add_cfm;
			int error = 0;

			/* Indicate we are in a STA addition process - This will allow handling
			 * potential PS mode change indications correctly
			 */
			set_bit(CLS_WIFI_DEV_ADDING_STA, &cls_wifi_hw->flags);

			/* Forward the information to the LMAC */
			if ((error = cls_wifi_send_me_sta_add(cls_wifi_hw, params, mac, cls_wifi_vif->vif_index,
											  &me_sta_add_cfm)))
				return error;

			// Check the status
			switch (me_sta_add_cfm.status)
			{
				case CO_OK:
				{
					int tid;
					sta = &cls_wifi_hw->sta_table[me_sta_add_cfm.sta_idx];
					sta->aid = params->aid;
					sta->sta_idx = me_sta_add_cfm.sta_idx;
					sta->ch_idx = cls_wifi_vif->ch_index;
					sta->vif_idx = cls_wifi_vif->vif_index;
					sta->vlan_idx = sta->vif_idx;
					sta->qos = (params->sta_flags_set & BIT(NL80211_STA_FLAG_WME)) != 0;
					sta->ht = params->ht_capa ? 1 : 0;
					sta->vht = params->vht_capa ? 1 : 0;
					sta->he = params->he_capa ? 1 : 0;
					sta->acm = 0;
					for (tid = 0; tid < CLS_NB_TXQ_PER_STA; tid++) {
						int uapsd_bit = cls_wifi_hwq2uapsd[cls_wifi_tid2hwq[tid]];
						if (params->uapsd_queues & uapsd_bit)
							sta->uapsd_tids |= 1 << tid;
						else
							sta->uapsd_tids &= ~(1 << tid);
					}
					memcpy(sta->mac_addr, mac, ETH_ALEN);
					cls_wifi_dbgfs_register_sta(cls_wifi_hw, sta);
					cls_wtm_reset_sta_peak_data(sta);

					/* Ensure that we won't process PS change or channel switch ind*/
					spin_lock_bh(&cls_wifi_hw->cb_lock);
					cls_wifi_txq_sta_init(cls_wifi_hw, sta, cls_wifi_txq_vif_get_status(cls_wifi_vif));
					if (cls_wifi_vif->tdls_status == TDLS_SETUP_RSP_TX) {
						cls_wifi_vif->tdls_status = TDLS_LINK_ACTIVE;
						sta->tdls.initiator = true;
						sta->tdls.active = true;
					}
					/* Set TDLS channel switch capability */
					if ((params->ext_capab[3] & WLAN_EXT_CAPA4_TDLS_CHAN_SWITCH) &&
						!cls_wifi_vif->tdls_chsw_prohibited)
						sta->tdls.chsw_allowed = true;
					cls_wifi_vif->sta.tdls_sta = sta;
					sta->valid = true;
					spin_unlock_bh(&cls_wifi_hw->cb_lock);
#ifdef CONFIG_CLS_WIFI_BFMER
					if (cls_wifi_hw->radio_params->bfmer)
						cls_wifi_send_bfmer_enable(cls_wifi_hw, sta, params);
#endif /* CONFIG_CLS_WIFI_BFMER */

#ifdef CONFIG_CLS_FWT
					/* sub_port */
					if (cls_fwt_g_enable) {
						// TODO: cls_wifi_hw->radio_idx ??
						node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, sta->sta_idx);
						node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

						vif_idx = CLS_IEEE80211_VIF_IDX_MAP(cls_wifi_vif->vif_index);

						sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(node_idx, vif_idx,
							       CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE, 0);

						br_fdb_update_const_hook(dev, sta->mac_addr, br_port_get_pvid(dev), sub_port);
					}
#endif

					#define PRINT_STA_FLAG(f)							   \
						(params->sta_flags_set & BIT(NL80211_STA_FLAG_##f) ? "["#f"]" : "")

					netdev_info(dev, "Add %s TDLS sta %d (%pM) flags=%s%s%s%s%s%s%s",
								sta->tdls.initiator ? "initiator" : "responder",
								sta->sta_idx, mac,
								PRINT_STA_FLAG(AUTHORIZED),
								PRINT_STA_FLAG(SHORT_PREAMBLE),
								PRINT_STA_FLAG(WME),
								PRINT_STA_FLAG(MFP),
								PRINT_STA_FLAG(AUTHENTICATED),
								PRINT_STA_FLAG(TDLS_PEER),
								PRINT_STA_FLAG(ASSOCIATED));
					#undef PRINT_STA_FLAG

					break;
				}
				default:
					error = -EBUSY;
					break;
			}

			clear_bit(CLS_WIFI_DEV_ADDING_STA, &cls_wifi_hw->flags);
		} else  {
			return -EINVAL;
		}
	}
	if (params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
		cls_wifi_send_me_set_control_port_req(cls_wifi_hw,
			(params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) != 0,
			sta->sta_idx);
		sta->last_data_frame_rx = jiffies;
		sta->jiff_authorized = jiffies;
	}

	if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_MESH_POINT) {
		if (params->sta_modify_mask & STATION_PARAM_APPLY_PLINK_STATE) {
			if (params->plink_state < NUM_NL80211_PLINK_STATES) {
				cls_wifi_send_mesh_peer_update_ntf(cls_wifi_hw, vif, sta->sta_idx, params->plink_state);
			}
		}

		if (params->local_pm != NL80211_MESH_POWER_UNKNOWN) {
			sta->mesh_pm = params->local_pm;
			cls_wifi_update_mesh_power_mode(vif);
		}
	}

	if (params->vlan) {
		uint8_t vlan_idx;

		vif = netdev_priv(params->vlan);
		vlan_idx = vif->vif_index;

		if (sta->vlan_idx != vlan_idx) {
			struct cls_wifi_vif *old_vif;
			old_vif = cls_wifi_hw->vif_table[sta->vlan_idx];
			cls_wifi_txq_sta_switch_vif(sta, old_vif, vif);

			pr_info("[%s] sta %d vlan_idx changed: %d > %d\n", __func__,
							sta->sta_idx, sta->vlan_idx, vlan_idx);
			sta->vlan_idx = vlan_idx;

			if ((CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_AP_VLAN) &&
				(vif->use_4addr)) {
				WARN((vif->ap_vlan.sta_4a),
					 "4A AP_VLAN interface with more than one sta");
				vif->ap_vlan.sta_4a = sta;
			}

			if ((CLS_WIFI_VIF_TYPE(old_vif) == NL80211_IFTYPE_AP_VLAN) &&
				(old_vif->use_4addr)) {
				old_vif->ap_vlan.sta_4a = NULL;
			}
		}
	}
	sta->sta_flags_set = params->sta_flags_set;
	return 0;
}

/**
 * @start_ap: Start acting in AP mode defined by the parameters.
 */
static int cls_wifi_cfg80211_start_ap(struct wiphy *wiphy, struct net_device *dev,
								  struct cfg80211_ap_settings *settings)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct apm_start_cfm apm_start_cfm;
	struct cls_wifi_sta *sta;
	int error = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	// ignore it if repeater-sta disconnected
	if (cls_wifi_is_repeater_mode(cls_wifi_hw) && !cls_wifi_repeater_is_connected(cls_wifi_hw))
		pr_info("[%s] start_ap %s, ignore repeater disconnected.\n", __func__, cls_wifi_vif->ndev->name);

	if ((error = cls_wifi_send_apm_start_req(cls_wifi_hw, cls_wifi_vif, settings,
										 &apm_start_cfm)))
		goto end;

	// Check the status
	switch (apm_start_cfm.status)
	{
		case CO_OK:
		{
			u8 txq_status = 0;

			// The second start_ap may be happend before APM handling first MM_SET_VIF_STATE_CFM
			if (cls_wifi_is_repeater_mode(cls_wifi_hw))
				msleep(10);

			cls_wifi_vif->going_stop = false;
			cls_wifi_vif->ap.bcmc_index = apm_start_cfm.bcmc_idx;
			cls_wifi_vif->ap.flags = 0;
			cls_wifi_vif->ap.bcn_interval = settings->beacon_interval;
			sta = &cls_wifi_hw->sta_table[apm_start_cfm.bcmc_idx];
			sta->valid = true;
			sta->aid = 0;
			sta->sta_idx = apm_start_cfm.bcmc_idx;
			sta->ch_idx = apm_start_cfm.ch_idx;
			sta->vif_idx = cls_wifi_vif->vif_index;
			sta->qos = false;
			sta->acm = 0;
			sta->ps.active = false;
			sta->listen_interval = 5;
			sta->ht = settings->ht_cap ? 1 : 0;
			sta->vht = settings->vht_cap ? 1 : 0;
			sta->he = settings->he_cap ? 1 : 0;

			// In repeater mode, the dif_boot_cali will only be triggered by repeater-sta before AUTH
			if ((cls_wifi_vif->vif_index == 0) && !cls_wifi_is_repeater_mode(cls_wifi_hw)) {
				cls_wifi_dif_boot_cali_clear(cls_wifi_hw);
				cls_wifi_dif_boot_cali(cls_wifi_hw);
			}

			spin_lock_bh(&cls_wifi_hw->cb_lock);
			cls_wifi_chanctx_link(cls_wifi_vif, apm_start_cfm.ch_idx,
							  &settings->chandef);
			if (cls_wifi_hw->cur_chanctx != apm_start_cfm.ch_idx) {
				txq_status = CLS_WIFI_TXQ_STOP_CHAN;
			}
			cls_wifi_txq_vif_init(cls_wifi_hw, cls_wifi_vif, txq_status);
			spin_unlock_bh(&cls_wifi_hw->cb_lock);
#ifdef CONFIG_CLS_VBSS
			clsemi_vbss_vap_init(cls_wifi_hw, cls_wifi_vif);
#endif
			netif_tx_start_all_queues(dev);
			netif_carrier_on(dev);
			error = 0;
			/* If the AP channel is already the active, we probably skip radar
			   activation on MM_CHANNEL_SWITCH_IND (unless another vif use this
			   ctxt). In anycase retest if radar detection must be activated
			 */
			if (txq_status == 0) {
				cls_wifi_radar_detection_enable_on_cur_channel(cls_wifi_hw, cls_wifi_vif);
			}

			cls_wifi_sync_pppc_txpower_req(cls_wifi_hw, &settings->chandef);

			cls_wifi_vif->ap.ap_started = true;

			break;
		}
		case CO_BUSY:
			error = -EINPROGRESS;
			break;
		case CO_OP_IN_PROGRESS:
			error = -EALREADY;
			break;
		default:
			error = -EIO;
			break;
	}

end:
	if (error) {
		netdev_info(dev, "Failed to start AP (%d)", error);
	} else {
		netdev_info(dev, "AP started: ch_ctxt_idx=%d, bcmc_idx=%d",
					cls_wifi_vif->ch_index, cls_wifi_vif->ap.bcmc_index);
	}

	return error;
}


/**
 * @change_beacon: Change the beacon parameters for an access point mode
 *	interface. This should reject the call when AP mode wasn't started.
 */
static int cls_wifi_cfg80211_change_beacon(struct wiphy *wiphy, struct net_device *dev,
									   struct cfg80211_beacon_data *info)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_bcn *bcn = &vif->ap.bcn;
	struct cls_wifi_ipc_buf buf;
	u8 *bcn_buf;
	int error = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	// Build the beacon
	bcn_buf = cls_wifi_build_bcn(bcn, info);
	if (!bcn_buf)
		return -ENOMEM;

	// Sync buffer for FW
	if ((error = cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &buf, bcn_buf, bcn->len))) {
		netdev_err(dev, "Failed to allocate IPC buf for new beacon\n");
		kfree(bcn_buf);
		return error;
	}

	// Forward the information to the LMAC
	error = cls_wifi_send_bcn_change(cls_wifi_hw, vif->vif_index, buf.dma_addr,
								 bcn->len, bcn->head_len, bcn->tim_len, NULL, 0);

	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &buf);
	return error;
}

/**
 * * @stop_ap: Stop being an AP, including stopping beaconing.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 2)
static int cls_wifi_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *dev)
#else
static int cls_wifi_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *dev, unsigned int link_id)
#endif
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	int ret;
	struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[cls_wifi_vif->ap.bcmc_index];

	cls_wifi_vif->ap.ap_started = false;

	if ((sta->sta_idx == cls_wifi_vif->ap.bcmc_index)
		&& (sta->vif_idx == cls_wifi_vif->vif_index)
		&& (sta->qos == false))
		sta->valid = false;

	netdev_info(dev, "AP Stopped  enter");

	cls_wifi_vif->going_stop = true;

	/* Delete any remaining STA before stopping the AP to ensure correct flush
	   of pending TX buffers.
	   TODO: Check if a mechanism is needed to prevent new connection until AP
	   is stopped */
	while (!list_empty(&cls_wifi_vif->ap.sta_list)) {
		ret = cls_wifi_cfg80211_del_station(wiphy, dev, NULL);
		if (ret != 0)
			pr_err("%s vif_index: %u del_sta failed\n", __func__, cls_wifi_vif->vif_index);
	}

	cls_wifi_radar_cancel_cac(&cls_wifi_hw->radar);
	cls_wifi_send_apm_stop_req(cls_wifi_hw, cls_wifi_vif);
	spin_lock_bh(&cls_wifi_hw->cb_lock);
	cls_wifi_chanctx_unlink(cls_wifi_vif);
	spin_unlock_bh(&cls_wifi_hw->cb_lock);

	cls_wifi_txq_vif_deinit(cls_wifi_hw, cls_wifi_vif);
	cls_wifi_del_bcn(&cls_wifi_vif->ap.bcn);
	cls_wifi_del_csa(cls_wifi_vif);

	netif_tx_stop_all_queues(dev);
	netif_carrier_off(dev);

	netdev_info(dev, "AP Stopped");

	return 0;
}

/**
 * @set_monitor_channel: Set the monitor mode channel for the device. If other
 *	interfaces are active this callback should reject the configuration.
 *	If no interfaces are active or the device is down, the channel should
 *	be stored for when a monitor interface becomes active.
 *
 * Also called internaly with chandef set to NULL simply to retrieve the channel
 * configured at firmware level.
 */
static int cls_wifi_cfg80211_set_monitor_channel(struct wiphy *wiphy,
											 struct cfg80211_chan_def *chandef)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif;
	struct me_config_monitor_cfm cfm;
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (cls_wifi_hw->monitor_vif == CLS_WIFI_INVALID_VIF)
		return -EINVAL;

	cls_wifi_vif = cls_wifi_hw->vif_table[cls_wifi_hw->monitor_vif];

	// Do nothing if monitor interface is already configured with the requested channel
	if (cls_wifi_chanctx_valid(cls_wifi_hw, cls_wifi_vif->ch_index)) {
		struct cls_wifi_chanctx *ctxt;
		ctxt = &cls_wifi_vif->cls_wifi_hw->chanctx_table[cls_wifi_vif->ch_index];
		if (chandef && cfg80211_chandef_identical(&ctxt->chan_def, chandef))
			return 0;
	}

	// Always send command to firmware. It allows to retrieve channel context index
	// and its configuration.
	if (cls_wifi_send_config_monitor_req(cls_wifi_hw, chandef, &cfm))
		return -EIO;

	// Always re-set channel context info
	cls_wifi_chanctx_unlink(cls_wifi_vif);



	// If there is also a STA interface not yet connected then monitor interface
	// will only have a channel context after the connection of the STA interface.
	if (cfm.chan_index != CLS_WIFI_CH_NOT_SET)
	{
		struct cfg80211_chan_def mon_chandef;

		if (cls_wifi_hw->vif_started > 1) {
			// In this case we just want to update the channel context index not
			// the channel configuration
			cls_wifi_chanctx_link(cls_wifi_vif, cfm.chan_index, NULL);
			return -EBUSY;
		}

		memset(&mon_chandef, 0, sizeof(mon_chandef));
		mon_chandef.chan = ieee80211_get_channel(wiphy, cfm.chan.prim20_freq);
		mon_chandef.center_freq1 = cfm.chan.center1_freq;
		mon_chandef.center_freq2 = cfm.chan.center2_freq;
		mon_chandef.width =  chnl2bw[cfm.chan.type];
		cls_wifi_chanctx_link(cls_wifi_vif, cfm.chan_index, &mon_chandef);
	}

	return 0;
}

/**
 * @probe_client: probe an associated client, must return a cookie that it
 *	later passes to cfg80211_probe_status().
 */
int cls_wifi_cfg80211_probe_client(struct wiphy *wiphy, struct net_device *dev,
							   const u8 *peer, u64 *cookie)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_sta *sta = NULL;
	struct apm_probe_client_cfm cfm;

	if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP) &&
		(CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP_VLAN) &&
		(CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_P2P_GO) &&
		(CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MESH_POINT))
		return -EINVAL;

	list_for_each_entry(sta, &vif->ap.sta_list, list) {
		if (sta->valid && ether_addr_equal(sta->mac_addr, peer))
			break;
	}

	if (!sta)
		return -EINVAL;

	cls_wifi_send_apm_probe_req(cls_wifi_hw, vif, sta, &cfm);

	if (cfm.status != CO_OK)
		return -EINVAL;

	*cookie = (u64)cfm.probe_id;
	return 0;
}

/**
 * @set_wiphy_params: Notify that wiphy parameters have changed;
 *	@changed bitfield (see &enum wiphy_params_flags) describes which values
 *	have changed. The actual parameter values are available in
 *	struct wiphy. If returning an error, no value should be changed.
 */
static int cls_wifi_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	return 0;
}


/**
 * @set_tx_power: set the transmit power according to the parameters,
 *	the power passed is in mBm, to get dBm use MBM_TO_DBM(). The
 *	wdev may be %NULL if power was set for the wiphy, and will
 *	always be %NULL unless the driver supports per-vif TX power
 *	(as advertised by the nl80211 feature flag.)
 */
static int cls_wifi_cfg80211_set_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev,
									  enum nl80211_tx_power_setting type, int mbm)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif;
	s8 pwr;
	int res = 0;

	if (type == NL80211_TX_POWER_AUTOMATIC) {
		pwr = 0x7f;
	} else {
		pwr = MBM_TO_DBM(mbm);
	}

	if (wdev) {
		vif = container_of(wdev, struct cls_wifi_vif, wdev);
		res = cls_wifi_send_set_power(cls_wifi_hw, vif->vif_index, pwr, NULL);
	} else {
		list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
			res = cls_wifi_send_set_power(cls_wifi_hw, vif->vif_index, pwr, NULL);
			if (res)
				break;
		}
	}

	return res;
}

/**
 * @get_tx_power: get the transmit power
 */
static int cls_wifi_cfg80211_get_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev,
						int *dbm)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif;
	struct mm_get_power_cfm cfm;
	int res = -1;

	if (wdev) {
		vif = container_of(wdev, struct cls_wifi_vif, wdev);
		res = cls_wifi_send_get_power(cls_wifi_hw, vif->vif_index, &cfm);
	}
	if (res == 0)
		*dbm = cfm.power;

	return res;
}

/**
 * @set_power_mgmt: set the power save to one of those two modes:
 *  Power-save off
 *  Power-save on - Dynamic mode
 */
static int cls_wifi_cfg80211_set_power_mgmt(struct wiphy *wiphy,
										struct net_device *dev,
										bool enabled, int timeout)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	u8 ps_mode;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	if (timeout >= 0)
		netdev_info(dev, "Ignore timeout value %d", timeout);

	if (!(cls_wifi_hw->version_cfm.features & BIT(MM_FEAT_PS_BIT)))
		enabled = false;

	if (enabled) {
		/* Switch to Dynamic Power Save */
		ps_mode = MM_PS_MODE_ON_DYN;
	} else {
		/* Exit Power Save */
		ps_mode = MM_PS_MODE_OFF;
	}

	return cls_wifi_send_me_set_ps_mode(cls_wifi_hw, ps_mode);
}

/**
 * @set_txq_params: Set TX queue parameters
 */
static int cls_wifi_cfg80211_set_txq_params(struct wiphy *wiphy, struct net_device *dev,
										struct ieee80211_txq_params *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	u8 hw_queue, aifs, cwmin, cwmax;
	u32 param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	hw_queue = cls_wifi_ac2hwq[0][params->ac];

	aifs  = params->aifs;
	cwmin = fls(params->cwmin);
	cwmax = fls(params->cwmax);

	/* Store queue information in general structure */
	param  = (u32) (aifs << 0);
	param |= (u32) (cwmin << 4);
	param |= (u32) (cwmax << 8);
	param |= (u32) (params->txop) << 12;

	/* Send the MM_SET_EDCA_REQ message to the FW */
	return cls_wifi_send_set_edca(cls_wifi_hw, hw_queue, param, false, cls_wifi_vif->vif_index);
}


/**
 * @remain_on_channel: Request the driver to remain awake on the specified
 *	channel for the specified duration to complete an off-channel
 *	operation (e.g., public action frame exchange). When the driver is
 *	ready on the requested channel, it must indicate this with an event
 *	notification by calling cfg80211_ready_on_channel().
 */
static int
cls_wifi_cfg80211_remain_on_channel(struct wiphy *wiphy, struct wireless_dev *wdev,
								struct ieee80211_channel *chan,
								unsigned int duration, u64 *cookie)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(wdev->netdev);
	struct cls_wifi_roc *roc;
	int error;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* For debug purpose (use ftrace kernel option) */
	trace_roc(cls_wifi_vif->vif_index, chan->center_freq, duration);

	/* Scan & roc are incompatibile, reject concurrent request */
	if (cls_wifi_hw->scan_request)
		return -EBUSY;

	/* Check that no other RoC procedure has been launched */
	if (cls_wifi_hw->roc)
		return -EBUSY;

	/* Allocate a temporary RoC element */
	roc = kmalloc(sizeof(struct cls_wifi_roc), GFP_KERNEL);
	if (!roc)
		return -ENOMEM;

	/* Initialize the RoC information element */
	roc->vif = cls_wifi_vif;
	roc->chan = chan;
	roc->duration = duration;
	roc->internal = false;
	roc->on_chan = false;
	roc->tx_cnt = 0;
	memset(roc->tx_cookie, 0, sizeof(roc->tx_cookie));

	/* Initialize the OFFCHAN TX queue to allow off-channel transmissions */
	cls_wifi_txq_offchan_init(cls_wifi_vif);

	/* Forward the information to the FMAC */
	cls_wifi_hw->roc = roc;
	error = cls_wifi_send_roc(cls_wifi_hw, cls_wifi_vif, chan, duration);

	if (error) {
		kfree(roc);
		cls_wifi_hw->roc = NULL;
		cls_wifi_txq_offchan_deinit(cls_wifi_vif);
	} else if (cookie)
		*cookie = (u64)(uintptr_t)roc;

	return error;
}

/**
 * @cancel_remain_on_channel: Cancel an on-going remain-on-channel operation.
 *	This allows the operation to be terminated prior to timeout based on
 *	the duration value.
 */
static int cls_wifi_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
												  struct wireless_dev *wdev,
												  u64 cookie)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(wdev->netdev);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	trace_cancel_roc(cls_wifi_vif->vif_index);

	if (!cls_wifi_hw->roc)
		return 0;

	if (cookie != (u64)(uintptr_t)cls_wifi_hw->roc)
		return -EINVAL;

	/* Forward the information to the FMAC */
	return cls_wifi_send_cancel_roc(cls_wifi_hw);
}

/**
 * @dump_survey: get site survey information.
 */
static int cls_wifi_cfg80211_dump_survey(struct wiphy *wiphy, struct net_device *netdev,
									 int idx, struct survey_info *info)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct ieee80211_supported_band *sband;
	struct cls_wifi_survey_info *cls_wifi_survey;

	//CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (idx >= ARRAY_SIZE(cls_wifi_hw->survey))
		return -ENOENT;

	cls_wifi_survey = &cls_wifi_hw->survey[idx];

	// Check if provided index matches with a supported 2.4GHz channel
	sband = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ];
	if (sband && idx >= sband->n_channels) {
		idx -= sband->n_channels;
		sband = NULL;
	}

	if (!sband) {
		// Check if provided index matches with a supported 5GHz channel
		sband = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ];

		if (!sband || idx >= sband->n_channels)
			return -ENOENT;
	}

	// Fill the survey
	info->channel = &sband->channels[idx];
	info->filled = cls_wifi_survey->filled;

	if (cls_wifi_survey->filled != 0) {
		info->time = (u64)cls_wifi_survey->chan_time_ms;
		info->time_busy = (u64)cls_wifi_survey->chan_time_busy_ms;
		info->noise = cls_wifi_survey->noise_dbm;

		// TODO: clear survey after some time ?
	}

	return 0;
}

/**
 * @get_channel: Get the current operating channel for the virtual interface.
 *	For monitor interfaces, it should return %NULL unless there's a single
 *	current monitoring channel.
 */
static int cls_wifi_cfg80211_get_channel(struct wiphy *wiphy,
									 struct wireless_dev *wdev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 2)
#else
									 unsigned int link_id,
#endif
									 struct cfg80211_chan_def *chandef) {
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = container_of(wdev, struct cls_wifi_vif, wdev);
	struct cls_wifi_chanctx *ctxt;

	if (!cls_wifi_vif->up) {
		return -ENODATA;
	}

	if (cls_wifi_vif->vif_index == cls_wifi_hw->monitor_vif)
	{
		//retrieve channel from firmware
		cls_wifi_cfg80211_set_monitor_channel(wiphy, NULL);
	}

	//Check if channel context is valid
	if(!cls_wifi_chanctx_valid(cls_wifi_hw, cls_wifi_vif->ch_index)){
		return -ENODATA;
	}

	ctxt = &cls_wifi_hw->chanctx_table[cls_wifi_vif->ch_index];
	*chandef = ctxt->chan_def;

	return 0;
}

/**
 * @mgmt_tx: Transmit a management frame.
 */
int cls_wifi_cfg80211_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			 struct cfg80211_mgmt_tx_params *params, u64 *cookie)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(wdev->netdev);
	struct cls_wifi_sta *cls_wifi_sta;
	struct ieee80211_mgmt *mgmt = (void *)params->buf;
	bool ap = false;
	bool offchan = false;
	int res;

	switch (CLS_WIFI_VIF_TYPE(cls_wifi_vif)) {
		case NL80211_IFTYPE_AP_VLAN:
			cls_wifi_vif = cls_wifi_vif->ap_vlan.master;
			fallthrough;
		case NL80211_IFTYPE_AP:
		case NL80211_IFTYPE_P2P_GO:
		case NL80211_IFTYPE_MESH_POINT:
			ap = true;
			break;
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_P2P_CLIENT:
		default:
			break;
	}

	// Get STA on which management frame has to be sent
	cls_wifi_sta = cls_wifi_retrieve_sta(cls_wifi_hw, cls_wifi_vif, mgmt->da,
								 mgmt->frame_control, ap);

	if (params->offchan) {
		if (!params->chan)
			return -EINVAL;

		offchan = true;
		if (cls_wifi_chanctx_valid(cls_wifi_hw, cls_wifi_vif->ch_index)) {
			struct cls_wifi_chanctx *ctxt = &cls_wifi_hw->chanctx_table[cls_wifi_vif->ch_index];
			if (ctxt->chan_def.chan->center_freq == params->chan->center_freq)
				offchan = false;
		}
	}

	trace_mgmt_tx((offchan) ? params->chan->center_freq : 0,
				  cls_wifi_vif->vif_index, (cls_wifi_sta) ? cls_wifi_sta->sta_idx : 0xFF,
				  mgmt);

	if (offchan) {
		// Offchannel transmission, need to start a RoC
		if (cls_wifi_hw->roc) {
			/*# CLS: RoC re-reuse feature incomplete, disable for now.
			// Test if current RoC can be re-used
			if ((cls_wifi_hw->roc->vif != cls_wifi_vif) ||
				(cls_wifi_hw->roc->chan->center_freq != params->chan->center_freq))
				return -EINVAL;
			// TODO: inform FW to increase RoC duration
			#*/
			return -EBUSY;
		} else {
			int error;
			unsigned int duration = 30;

			/* Start a new ROC procedure */
			if (params->wait)
				duration = params->wait;

			error = cls_wifi_cfg80211_remain_on_channel(wiphy, wdev, params->chan,
													duration, NULL);
			if (error)
				return error;

			// internal RoC, no need to inform user space about it
			cls_wifi_hw->roc->internal = true;
		}
	}

	res = cls_wifi_start_mgmt_xmit(cls_wifi_vif, cls_wifi_sta, params, offchan, cookie);
	if (offchan) {
		if (cls_wifi_hw->roc->tx_cnt < CLS_ROC_TX)
			cls_wifi_hw->roc->tx_cookie[cls_wifi_hw->roc->tx_cnt] = *cookie;
		else
			wiphy_warn(wiphy, "%d frames sent within the same Roc (> CLS_ROC_TX)",
					   cls_wifi_hw->roc->tx_cnt + 1);
		cls_wifi_hw->roc->tx_cnt++;
	}
	return res;
}

int cls_wifi_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   u64 cookie)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	int i, nb_tx_cookie = 0;

	if (!cls_wifi_hw->roc || !cls_wifi_hw->roc->tx_cnt)
		return 0;

	for (i = 0; i < CLS_ROC_TX; i++) {
		if (!cls_wifi_hw->roc->tx_cookie)
			continue;

		nb_tx_cookie++;
		if (cls_wifi_hw->roc->tx_cookie[i] == cookie) {
			cls_wifi_hw->roc->tx_cookie[i] = 0;
			cls_wifi_hw->roc->tx_cnt--;
			break;
		}
	}

	if (i == CLS_ROC_TX) {
		// Didn't find the cookie but this frame may still have been sent within this
		// Roc if more than CLS_ROC_TX frame have been sent
		if (nb_tx_cookie != cls_wifi_hw->roc->tx_cnt)
			cls_wifi_hw->roc->tx_cnt--;
		else
			return 0;
	}

	// Stop the RoC if started to send TX frame and all frames have been "wait cancelled"
	if ((!cls_wifi_hw->roc->internal) || (cls_wifi_hw->roc->tx_cnt > 0))
		return 0;

	return cls_wifi_cfg80211_cancel_remain_on_channel(wiphy, wdev, (u64)(uintptr_t)cls_wifi_hw->roc);
}

/**
 * @start_radar_detection: Start radar detection in the driver.
 */
static int cls_wifi_cfg80211_start_radar_detection(struct wiphy *wiphy,
											   struct net_device *dev,
											   struct cfg80211_chan_def *chandef,
											   u32 cac_time_ms)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct apm_start_cac_cfm cfm;

	pr_info("[%s] enter: chandef(%d %d)\n", __func__, chandef->chan->center_freq, chandef->width);

	/* check need skip dfs cac or NOT */
	if (cls_wifi_get_skip_dfs_cac(wiphy) || cls_wifi_is_repeater_mode(cls_wifi_hw)) {
		/* if skip dfs cac, set cac time to 1000ms. (Can not less than 1000ms) */
		cac_time_ms = 1000;
		pr_warn("-----> %s skip dfs cac is 1, %s mode, cac_time_ms = %d\n", __func__,
			cls_wifi_is_repeater_mode(cls_wifi_hw) ? "repeater" : "ap", cac_time_ms);

		/* set skip dfs_cac flag to 0 */
		cls_wifi_set_skip_dfs_cac(wiphy, 0);

		memcpy(&cls_wifi_hw->radar.skip_chandef, chandef, sizeof(struct cfg80211_chan_def));

		cls_wifi_hw->radar.skip_fw = true;
	} else
		cls_wifi_hw->radar.skip_fw = false;

#ifdef CONFIG_CLS_EMU_ADAPTER
	pr_warn("-----> %s cac_time_ms: 0x%x\n", __FUNCTION__, cac_time_ms);
	if (cac_time_ms > 50) {
		cac_time_ms = 50;
	}
#endif

	if (!cls_wifi_is_repeater_mode(cls_wifi_hw) && cls_wifi_vif->vif_index == 0)
		cls_wifi_dif_boot_cali_clear(cls_wifi_hw);

#if defined(MERAK2000) && MERAK2000
	cls_wifi_send_set_rd_agc_war_req(cls_wifi_hw, cls_wifi_hw->radio_params->rd_agc_war_en);
#endif

	cls_wifi_radar_start_cac(&cls_wifi_hw->radar, cac_time_ms, cls_wifi_vif);

	if (cls_wifi_hw->radar.skip_fw)
		return 0;

	cls_wifi_send_apm_start_cac_req(cls_wifi_hw, cls_wifi_vif, chandef, &cfm,
					cls_wifi_hw->radio_params->dfs_enable);

#ifdef CONFIG_CLS_EMU_ADAPTER
	pr_warn("-----> APM_START_CAC_CFM cfm.status: 0x%x\n", cfm.status);
#endif

	if (cfm.status == CO_OK) {
		spin_lock_bh(&cls_wifi_hw->cb_lock);
		cls_wifi_chanctx_link(cls_wifi_vif, cfm.ch_idx, chandef);
		if (cls_wifi_hw->cur_chanctx == cls_wifi_vif->ch_index &&
			cls_wifi_hw->radio_params->dfs_enable)
			cls_wifi_radar_detection_enable(&cls_wifi_hw->radar,
										CLS_WIFI_RADAR_DETECT_REPORT,
										CLS_WIFI_RADAR_RIU);
		spin_unlock_bh(&cls_wifi_hw->cb_lock);
	} else {
		return -EIO;
	}

	return 0;
}

/**
 * @update_ft_ies: Provide updated Fast BSS Transition information to the
 *	driver. If the SME is in the driver/firmware, this information can be
 *	used in building Authentication and Reassociation Request frames.
 */
static int cls_wifi_cfg80211_update_ft_ies(struct wiphy *wiphy,
									   struct net_device *dev,
									   struct cfg80211_update_ft_ies_params *ftie)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(dev);
	const struct element *rsne = NULL, *mde = NULL, *fte = NULL, *elem;
	bool ft_in_non_rsn = false;
	int fties_len = 0;
	u8 *ft_assoc_ies, *pos;

	if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_STATION) ||
		(vif->sta.ft_assoc_ies == NULL))
		return 0;

	for_each_element(elem, ftie->ie, ftie->ie_len) {
		if (elem->id == WLAN_EID_RSN)
			rsne = elem;
		else if (elem->id == WLAN_EID_MOBILITY_DOMAIN)
			mde = elem;
		else if (elem->id == WLAN_EID_FAST_BSS_TRANSITION)
			fte = elem;
		else
			netdev_warn(dev, "Unexpected FT element %d\n", elem->id);
	}
	if (!mde) {
		// maybe just test MDE for
		netdev_warn(dev, "Didn't find Mobility_Domain Element\n");
		return 0;
	} else if (!rsne && !fte) {
		// not sure this happen in real life ...
		ft_in_non_rsn = true;
	} else if (!rsne || !fte) {
		netdev_warn(dev, "Didn't find RSN or Fast Transition Element\n");
		return 0;
	}

	for_each_element(elem, vif->sta.ft_assoc_ies, vif->sta.ft_assoc_ies_len) {
		if ((elem->id == WLAN_EID_RSN) ||
			(elem->id == WLAN_EID_MOBILITY_DOMAIN) ||
			(elem->id == WLAN_EID_FAST_BSS_TRANSITION))
			fties_len += elem->datalen + sizeof(struct element);
	}

	ft_assoc_ies = kmalloc(vif->sta.ft_assoc_ies_len - fties_len + ftie->ie_len,
						GFP_KERNEL);
	if (!ft_assoc_ies) {
		netdev_warn(dev, "Fail to allocate buffer for association elements");
	}

	// Recopy current Association Elements one at a time and replace FT
	// element with updated version.
	pos = ft_assoc_ies;
	for_each_element(elem, vif->sta.ft_assoc_ies, vif->sta.ft_assoc_ies_len) {
		if (elem->id == WLAN_EID_RSN) {
			if (ft_in_non_rsn) {
				netdev_warn(dev, "Found RSN element in non RSN FT");
				goto abort;
			} else if (!rsne) {
				netdev_warn(dev, "Found several RSN element");
				goto abort;
			} else {
				memcpy(pos, rsne, sizeof(*rsne) + rsne->datalen);
				pos += sizeof(*rsne) + rsne->datalen;
				rsne = NULL;
			}
		} else if (elem->id == WLAN_EID_MOBILITY_DOMAIN) {
			if (!mde) {
				netdev_warn(dev, "Found several Mobility Domain element");
				goto abort;
			} else {
				memcpy(pos, mde, sizeof(*mde) + mde->datalen);
				pos += sizeof(*mde) + mde->datalen;
				mde = NULL;
			}
		}
		else if (elem->id == WLAN_EID_FAST_BSS_TRANSITION) {
			if (ft_in_non_rsn) {
				netdev_warn(dev, "Found Fast Transition element in non RSN FT");
				goto abort;
			} else if (!fte) {
				netdev_warn(dev, "found several Fast Transition element");
				goto abort;
			} else {
				memcpy(pos, fte, sizeof(*fte) + fte->datalen);
				pos += sizeof(*fte) + fte->datalen;
				fte = NULL;
			}
		}
		else {
			// Put FTE after MDE if non present in Association Element
			if (fte && !mde) {
				memcpy(pos, fte, sizeof(*fte) + fte->datalen);
				pos += sizeof(*fte) + fte->datalen;
				fte = NULL;
			}
			memcpy(pos, elem, sizeof(*elem) + elem->datalen);
			pos += sizeof(*elem) + elem->datalen;
		}
	}
	if (fte) {
		memcpy(pos, fte, sizeof(*fte) + fte->datalen);
		pos += sizeof(*fte) + fte->datalen;
		fte = NULL;
	}

	kfree(vif->sta.ft_assoc_ies);
	vif->sta.ft_assoc_ies = ft_assoc_ies;
	vif->sta.ft_assoc_ies_len = pos - ft_assoc_ies;

	if (vif->sta.flags & CLS_WIFI_STA_FT_OVER_DS) {
		struct sm_connect_cfm sm_connect_cfm;
		struct cfg80211_connect_params sme;

		memset(&sme, 0, sizeof(sme));
		rsne = cfg80211_find_elem(WLAN_EID_RSN, vif->sta.ft_assoc_ies,
								  vif->sta.ft_assoc_ies_len);
		if (rsne && cls_wifi_rsne_to_connect_params(rsne, &sme)) {
			netdev_warn(dev, "FT RSN parsing failed\n");
			return 0;
		}

		sme.ssid_len = vif->sta.ft_assoc_ies[1];
		sme.ssid = &vif->sta.ft_assoc_ies[2];
		sme.bssid = vif->sta.ft_target_ap;
		sme.ie = &vif->sta.ft_assoc_ies[2 + sme.ssid_len];
		sme.ie_len = vif->sta.ft_assoc_ies_len - (2 + sme.ssid_len);
		sme.auth_type = NL80211_AUTHTYPE_FT;
		cls_wifi_send_sm_connect_req(cls_wifi_hw, vif, &sme, &sm_connect_cfm);
		vif->sta.flags &= ~CLS_WIFI_STA_FT_OVER_DS;

	} else if (vif->sta.flags & CLS_WIFI_STA_FT_OVER_AIR) {
		uint8_t ssid_len;
		vif->sta.flags &= ~CLS_WIFI_STA_FT_OVER_AIR;

		// Skip the first element (SSID)
		ssid_len = vif->sta.ft_assoc_ies[1] + 2;
		if (cls_wifi_send_sm_ft_auth_rsp(cls_wifi_hw, vif, &vif->sta.ft_assoc_ies[ssid_len],
									 vif->sta.ft_assoc_ies_len - ssid_len))
			netdev_err(dev, "FT Over Air: Failed to send updated assoc elem\n");
	}

	return 0;

abort:
	kfree(ft_assoc_ies);
	return 0;
}

/**
 * @set_cqm_rssi_config: Configure connection quality monitor RSSI threshold.
 */
static int cls_wifi_cfg80211_set_cqm_rssi_config(struct wiphy *wiphy,
											 struct net_device *dev,
											 int32_t rssi_thold, uint32_t rssi_hyst)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);

	return cls_wifi_send_cfg_rssi_req(cls_wifi_hw, cls_wifi_vif->vif_index, rssi_thold, rssi_hyst);
}

/**
 *
 * @channel_switch: initiate channel-switch procedure (with CSA). Driver is
 *	responsible for veryfing if the switch is possible. Since this is
 *	inherently tricky driver may decide to disconnect an interface later
 *	with cfg80211_stop_iface(). This doesn't mean driver can accept
 *	everything. It should do it's best to verify requests and reject them
 *	as soon as possible.
 */
static int cls_wifi_cfg80211_channel_switch(struct wiphy *wiphy,
										struct net_device *dev,
										struct cfg80211_csa_settings *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_ipc_buf buf;
	struct cls_wifi_bcn *bcn, *bcn_after;
	struct cls_wifi_csa *csa;
	u16 csa_oft[BCN_MAX_CSA_CPT];
	u8 *bcn_buf;
	int i, error = 0;

	if (vif->ap.csa)
		return -EBUSY;

	if (params->n_counter_offsets_beacon > BCN_MAX_CSA_CPT)
		return -EINVAL;

	/* Build the new beacon with CSA IE */
	bcn = &vif->ap.bcn;
	bcn_buf = cls_wifi_build_bcn(bcn, &params->beacon_csa);
	if (!bcn_buf)
		return -ENOMEM;

	memset(csa_oft, 0, sizeof(csa_oft));
	for (i = 0; i < params->n_counter_offsets_beacon; i++)
	{
		csa_oft[i] = params->counter_offsets_beacon[i] + bcn->head_len +
			bcn->tim_len;
	}

	/* If count is set to 0 (i.e anytime after this beacon) force it to 2 */
	if (params->count == 0) {
		params->count = 2;
		for (i = 0; i < params->n_counter_offsets_beacon; i++)
		{
			bcn_buf[csa_oft[i]] = 2;
		}
	}

	if ((error = cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &buf, bcn_buf, bcn->len))) {
		netdev_err(dev, "Failed to allocate IPC buf for CSA beacon\n");
		kfree(bcn_buf);
		return error;
	}

	/* Build the beacon to use after CSA. It will only be sent to fw once
	   CSA is over, but do it before sending the beacon as it must be ready
	   when CSA is finished. */
	csa = kzalloc(sizeof(struct cls_wifi_csa), GFP_KERNEL);
	if (!csa) {
		error = -ENOMEM;
		goto end;
	}

	bcn_after = &csa->bcn;
	bcn_buf = cls_wifi_build_bcn(bcn_after, &params->beacon_after);
	if (!bcn_buf) {
		error = -ENOMEM;
		cls_wifi_del_csa(vif);
		goto end;
	}

	if ((error = cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &csa->buf, bcn_buf, bcn_after->len))) {
		netdev_err(dev, "Failed to allocate IPC buf for after CSA beacon\n");
		kfree(bcn_buf);
		goto end;
	}

	vif->ap.csa = csa;
	csa->vif = vif;
	csa->chandef = params->chandef;

	/* Send new Beacon. FW will extract channel and count from the beacon */
	error = cls_wifi_send_bcn_change(cls_wifi_hw, vif->vif_index, buf.dma_addr,
								 bcn->len, bcn->head_len, bcn->tim_len, csa_oft, 0);

	if (error) {
		cls_wifi_del_csa(vif);
	} else {
		if (!cls_wifi_is_repeater_mode(cls_wifi_hw) && vif->vif_index == 0)
			cls_wifi_dif_boot_cali_clear(cls_wifi_hw);
		INIT_WORK(&csa->work, cls_wifi_csa_finish);
		cfg80211_ch_switch_started_notify(dev, &csa->chandef, params->count,
										  params->block_tx);
	}

  end:
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &buf);
	return error;
}

enum wifi_sec_chan_offset {
	WIFI_SEC_CHAN_OFFSET_BEWLOW = -1,
	WIFI_SEC_CHAN_OFFSET_ABOVE = 1,
	WIFI_SEC_CHAN_OFFSET_NO_SEC = 0,
};

/** Get secondary channel offset by band and channel
 */
static int get_sec_chan_offset(int radio, int prim20_freq)
{
	int prim20_chan = ieee80211_frequency_to_channel(prim20_freq);


	switch (radio) {
	case 0: //RADIO_2G_INDEX
		switch (prim20_chan) {
		case 1:
		case 2:
		case 3:
		case 4:
			// for 1~4 primary channeles, 2nd channel must be ABOVE primary.
			return WIFI_SEC_CHAN_OFFSET_ABOVE;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			// for 5~9 primary channeles, 2nd channel could be above or below, we select below.
			return WIFI_SEC_CHAN_OFFSET_BEWLOW;
		case 10:
		case 11:
		case 12:
		case 13:
			// for 10~13 primary channeles, 2nd channel must be BELOW primary.
			return WIFI_SEC_CHAN_OFFSET_BEWLOW;
		default:
			return WIFI_SEC_CHAN_OFFSET_NO_SEC;
		}
		break;

	case 1: //RADIO_5G_INDEX
		switch (prim20_chan) {
		case 36:
		case 44:
		case 52:
		case 60:
		case 100:
		case 108:
		case 116:
		case 124:
		case 132:
		case 140:
		case 149:
		case 157:
			return WIFI_SEC_CHAN_OFFSET_ABOVE;
		case 40:
		case 48:
		case 56:
		case 64:
		case 104:
		case 112:
		case 120:
		case 128:
		case 136:
		case 144:
		case 153:
		case 161:
			return WIFI_SEC_CHAN_OFFSET_BEWLOW;
		case 165:
		default:
			return WIFI_SEC_CHAN_OFFSET_NO_SEC;
		}
		break;

	default:
		return WIFI_SEC_CHAN_OFFSET_NO_SEC;
	}
}

void cls_wifi_uevent_channel_switch(struct cls_wifi_vif *vif, struct cls_wifi_uevent_cs *uevent_cs)
{
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	/*
	Usage:
		hostapd_cli -i <vif name> chan_switch
		usage: <cs_count> <freq> [sec_channel_offset=] [center_freq1=] [center_freq2=] [bandwidth=] [blocktx] [ht|vht]
	*/
	char *envp[15] = {"EVENT=drv_chan_switch", "reason=REPEATER_CSA",
				"cmd_patch=/usr/sbin/hostapd_cli",
				"vifname=", "cs_count=",
				"freq=", "sec_channel_offset=", "center_freq1=", "center_freq2=",
				"bandwidth=", "blocktx=", "ht=", "vht=", "he=",
				NULL};
	char temp_arg[11][32];
	int cs_bw, sec_offset;
	int i, j;

	i = 3;//envp index
	j = 0;// temp_arg index

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "vifname=%s", vif->ndev->name);
	envp[i++] = temp_arg[j++];

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "cs_count=%d", uevent_cs->csa_count);
	envp[i++] = temp_arg[j++];

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "freq=%d", uevent_cs->freq);
	envp[i++] = temp_arg[j++];

	switch (uevent_cs->bw) {
	case PHY_CHNL_BW_20:
		cs_bw = 20;
		sec_offset = 0;
		break;
	case PHY_CHNL_BW_40:
		cs_bw = 40;
		sec_offset = get_sec_chan_offset(cls_wifi_hw->radio_idx, uevent_cs->freq);
		break;
	case PHY_CHNL_BW_80:
		cs_bw = 80;
		sec_offset = get_sec_chan_offset(cls_wifi_hw->radio_idx, uevent_cs->freq);
		break;
	case PHY_CHNL_BW_160:
		cs_bw = 160;
		sec_offset = get_sec_chan_offset(cls_wifi_hw->radio_idx, uevent_cs->freq);
		break;
	default:
		pr_err("[%s] invalid csa bw\n", __func__);
		cs_bw = 20;
		sec_offset = 0;
		break;
	}

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "sec_channel_offset=%d", sec_offset);
	envp[i++] = temp_arg[j++];

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "center_freq1=%d", uevent_cs->center1_freq);
	envp[i++] = temp_arg[j++];

	// ignore center2
	snprintf(temp_arg[j], sizeof(temp_arg[j]), " ");
	envp[i++] = temp_arg[j++];

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "bandwidth=%d", cs_bw);
	envp[i++] = temp_arg[j++];

	if (uevent_cs->blocktx)
		snprintf(temp_arg[j], sizeof(temp_arg[j]), "blocktx=%s", "blocktx");
	else
		snprintf(temp_arg[j], sizeof(temp_arg[j]), "blocktx=%s", "");
	envp[i++] = temp_arg[j++];

	snprintf(temp_arg[j], sizeof(temp_arg[j]), "ht=%s", "ht");
	envp[i++] = temp_arg[j++];

	if (cls_wifi_hw->radio_idx == 0) //RADIO_2G_INDEX
		snprintf(temp_arg[j], sizeof(temp_arg[j]), "vht=%s", "");
	else
		snprintf(temp_arg[j], sizeof(temp_arg[j]), "vht=%s", "vht");

	envp[i++] = temp_arg[j++];

	if (cls_wifi_hw->sta_table[vif->ap.bcmc_index].he)
		snprintf(temp_arg[j], sizeof(temp_arg[j]), "he=%s", "he");
	else
		snprintf(temp_arg[j], sizeof(temp_arg[j]), "he=%s", "");

	envp[i++] = temp_arg[j++];

	kobject_uevent_env(&cls_wifi_plat->dev->kobj, KOBJ_CHANGE, envp);
}

/**
 * @@tdls_mgmt: Transmit a TDLS management frame.
 */
static int cls_wifi_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
								   const u8 *peer, u8 action_code,  u8 dialog_token,
								   u16 status_code, u32 peer_capability,
								   bool initiator, const u8 *buf, size_t len)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	int ret = 0;

	/* make sure we support TDLS */
	if (!(wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS))
		return -ENOTSUPP;

	/* make sure we are in station mode (and connected) */
	if ((CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_STATION) ||
		(!cls_wifi_vif->up) || (!cls_wifi_vif->sta.ap))
		return -ENOTSUPP;

	/* only one TDLS link is supported */
	if ((action_code == WLAN_TDLS_SETUP_REQUEST) &&
		(cls_wifi_vif->sta.tdls_sta) &&
		(cls_wifi_vif->tdls_status == TDLS_LINK_ACTIVE)) {
		printk("%s: only one TDLS link is supported!\n", __func__);
		return -ENOTSUPP;
	}

	if ((action_code == WLAN_TDLS_DISCOVERY_REQUEST) &&
		(cls_wifi_hw->radio_params->ps_on)) {
		printk("%s: discovery request is not supported when "
				"power-save is enabled!\n", __func__);
		return -ENOTSUPP;
	}

	switch (action_code) {
	case WLAN_TDLS_SETUP_RESPONSE:
		/* only one TDLS link is supported */
		if ((status_code == 0) &&
			(cls_wifi_vif->sta.tdls_sta) &&
			(cls_wifi_vif->tdls_status == TDLS_LINK_ACTIVE)) {
			printk("%s: only one TDLS link is supported!\n", __func__);
			status_code = WLAN_STATUS_REQUEST_DECLINED;
		}
		fallthrough;
	case WLAN_TDLS_SETUP_REQUEST:
	case WLAN_TDLS_TEARDOWN:
	case WLAN_TDLS_DISCOVERY_REQUEST:
	case WLAN_TDLS_SETUP_CONFIRM:
	case WLAN_PUB_ACTION_TDLS_DISCOVER_RES:
		ret = cls_wifi_tdls_send_mgmt_packet_data(cls_wifi_hw, cls_wifi_vif, peer, action_code,
				dialog_token, status_code, peer_capability, initiator, buf, len, 0, NULL);
		break;

	default:
		printk("%s: Unknown TDLS mgmt/action frame %pM\n",
				__func__, peer);
		ret = -EOPNOTSUPP;
		break;
	}

	if (action_code == WLAN_TDLS_SETUP_REQUEST) {
		cls_wifi_vif->tdls_status = TDLS_SETUP_REQ_TX;
	} else if (action_code == WLAN_TDLS_SETUP_RESPONSE) {
		cls_wifi_vif->tdls_status = TDLS_SETUP_RSP_TX;
	} else if ((action_code == WLAN_TDLS_SETUP_CONFIRM) && (ret == CO_OK)) {
		cls_wifi_vif->tdls_status = TDLS_LINK_ACTIVE;
		/* Set TDLS active */
		cls_wifi_vif->sta.tdls_sta->tdls.active = true;
	}

	return ret;
}

/**
 * @tdls_oper: Perform a high-level TDLS operation (e.g. TDLS link setup).
 */
static int cls_wifi_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
								   const u8 *peer, enum nl80211_tdls_operation oper)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	int error;

	if (oper != NL80211_TDLS_DISABLE_LINK)
		return 0;

	if (!cls_wifi_vif->sta.tdls_sta) {
		printk("%s: TDLS station %pM does not exist\n", __func__, peer);
		return -ENOLINK;
	}

	if (memcmp(cls_wifi_vif->sta.tdls_sta->mac_addr, peer, ETH_ALEN) == 0) {
		/* Disable Channel Switch */
		if (!cls_wifi_send_tdls_cancel_chan_switch_req(cls_wifi_hw, cls_wifi_vif,
												   cls_wifi_vif->sta.tdls_sta,
												   NULL))
			cls_wifi_vif->sta.tdls_sta->tdls.chsw_en = false;

		netdev_info(dev, "Del TDLS sta %d (%pM)",
				cls_wifi_vif->sta.tdls_sta->sta_idx,
				cls_wifi_vif->sta.tdls_sta->mac_addr);
		/* Ensure that we won't process PS change ind */
		spin_lock_bh(&cls_wifi_hw->cb_lock);
		cls_wifi_vif->sta.tdls_sta->ps.active = false;
		cls_wifi_vif->sta.tdls_sta->valid = false;
		spin_unlock_bh(&cls_wifi_hw->cb_lock);
		cls_wifi_txq_sta_deinit(cls_wifi_hw, cls_wifi_vif->sta.tdls_sta);
		error = cls_wifi_send_me_sta_del(cls_wifi_hw, cls_wifi_vif->sta.tdls_sta->sta_idx, true);
		if ((error != 0) && (error != -EPIPE))
			return error;

#ifdef CONFIG_CLS_WIFI_BFMER
			// Disable Beamformer if supported
			cls_wifi_bfmer_report_del(cls_wifi_hw, cls_wifi_vif->sta.tdls_sta);
#endif /* CONFIG_CLS_WIFI_BFMER */

		/* Set TDLS not active */
		cls_wifi_vif->sta.tdls_sta->tdls.active = false;
		cls_wifi_dbgfs_unregister_sta(cls_wifi_hw, cls_wifi_vif->sta.tdls_sta);
		// Remove TDLS station
		cls_wifi_vif->tdls_status = TDLS_LINK_IDLE;
		cls_wifi_vif->sta.tdls_sta = NULL;
	}

	return 0;
}

/**
 *  @tdls_channel_switch: Start channel-switching with a TDLS peer. The driver
 *	is responsible for continually initiating channel-switching operations
 *	and returning to the base channel for communication with the AP.
 */
static int cls_wifi_cfg80211_tdls_channel_switch(struct wiphy *wiphy,
											 struct net_device *dev,
											 const u8 *addr, u8 oper_class,
											 struct cfg80211_chan_def *chandef)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_sta *cls_wifi_sta = cls_wifi_vif->sta.tdls_sta;
	struct tdls_chan_switch_cfm cfm;
	int error;

	if ((!cls_wifi_sta) || (memcmp(addr, cls_wifi_sta->mac_addr, ETH_ALEN))) {
		printk("%s: TDLS station %pM doesn't exist\n", __func__, addr);
		return -ENOLINK;
	}

	if (!cls_wifi_sta->tdls.chsw_allowed) {
		printk("%s: TDLS station %pM does not support TDLS channel switch\n", __func__, addr);
		return -ENOTSUPP;
	}

	error = cls_wifi_send_tdls_chan_switch_req(cls_wifi_hw, cls_wifi_vif, cls_wifi_sta,
										   cls_wifi_sta->tdls.initiator,
										   oper_class, chandef, &cfm);
	if (error)
		return error;

	if (!cfm.status) {
		cls_wifi_sta->tdls.chsw_en = true;
		return 0;
	} else {
		printk("%s: TDLS channel switch already enabled and only one is supported\n", __func__);
		return -EALREADY;
	}
}

/**
 * @tdls_cancel_channel_switch: Stop channel-switching with a TDLS peer. Both
 *	peers must be on the base channel when the call completes.
 */
static void cls_wifi_cfg80211_tdls_cancel_channel_switch(struct wiphy *wiphy,
													 struct net_device *dev,
													 const u8 *addr)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_sta *cls_wifi_sta = cls_wifi_vif->sta.tdls_sta;
	struct tdls_cancel_chan_switch_cfm cfm;

	if (!cls_wifi_sta)
		return;

	if (!cls_wifi_send_tdls_cancel_chan_switch_req(cls_wifi_hw, cls_wifi_vif,
											   cls_wifi_sta, &cfm))
		cls_wifi_sta->tdls.chsw_en = false;
}

/**
 * @change_bss: Modify parameters for a given BSS (mainly for AP mode).
 */
static int cls_wifi_cfg80211_change_bss(struct wiphy *wiphy, struct net_device *dev,
									struct bss_parameters *params)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	int res =  -EOPNOTSUPP;

	if (((CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP) ||
		 (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_P2P_GO)) &&
		(params->ap_isolate > -1)) {

		if (params->ap_isolate)
			cls_wifi_vif->ap.flags |= CLS_WIFI_AP_ISOLATE;
		else
			cls_wifi_vif->ap.flags &= ~CLS_WIFI_AP_ISOLATE;

		res = 0;
	}

	return res;
}

const struct cls_wifi_legrate legrates[] = {
	[0]  = { .idx = 0,  .rate = 10 },
	[1]  = { .idx = 1,  .rate = 20 },
	[2]  = { .idx = 2,  .rate = 55 },
	[3]  = { .idx = 3,  .rate = 110 },
	[4]  = { .idx = 4,  .rate = 60 },
	[5]  = { .idx = 5,  .rate = 90 },
	[6]  = { .idx = 6,  .rate = 120 },
	[7]  = { .idx = 7,  .rate = 180 },
	[8]  = { .idx = 8,  .rate = 240 },
	[9]  = { .idx = 9,  .rate = 360 },
	[10] = { .idx = 10, .rate = 480 },
	[11] = { .idx = 11, .rate = 540 },
};

static int cls_wifi_fill_station_info(struct cls_wifi_sta *sta, struct cls_wifi_vif *vif,
								  struct station_info *sinfo)
{
	struct cls_wifi_sta_stats *stats = &sta->stats;
	struct rx_vector_1 *rx_vect1 = &stats->last_rx.rx_vect1;

	// Generic info
	sinfo->generation = vif->generation;

	sinfo->inactive_time = jiffies_to_msecs(jiffies - stats->last_act);
	sinfo->rx_bytes = stats->rx_bytes;
	sinfo->tx_bytes = stats->tx_bytes;
	sinfo->tx_packets = stats->tx_pkts;
	sinfo->rx_packets = stats->rx_pkts;
	sinfo->signal = rx_vect1->rssi1;
	sinfo->connected_time = jiffies_to_msecs(jiffies - sta->jiff_authorized) / MSEC_PER_SEC;

	switch (rx_vect1->ch_bw) {
		case PHY_CHNL_BW_20:
			sinfo->rxrate.bw = RATE_INFO_BW_20;
			break;
		case PHY_CHNL_BW_40:
			sinfo->rxrate.bw = RATE_INFO_BW_40;
			break;
		case PHY_CHNL_BW_80:
			sinfo->rxrate.bw = RATE_INFO_BW_80;
			break;
		case PHY_CHNL_BW_160:
			sinfo->rxrate.bw = RATE_INFO_BW_160;
			break;
		default:
			sinfo->rxrate.bw = RATE_INFO_BW_HE_RU;
			break;
	}

	switch (rx_vect1->format_mod) {
		case FORMATMOD_NON_HT:
		case FORMATMOD_NON_HT_DUP_OFDM:
			sinfo->rxrate.flags = 0;
			sinfo->rxrate.legacy = legrates_lut[rx_vect1->leg_rate].rate;
			break;
		case FORMATMOD_HT_MF:
		case FORMATMOD_HT_GF:
			sinfo->rxrate.flags = RATE_INFO_FLAGS_MCS;
			if (rx_vect1->ht.short_gi)
				sinfo->rxrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
			sinfo->rxrate.mcs = rx_vect1->ht.mcs;
			break;
		case FORMATMOD_VHT:
			sinfo->rxrate.flags = RATE_INFO_FLAGS_VHT_MCS;
			if (rx_vect1->vht.short_gi)
				sinfo->rxrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
			sinfo->rxrate.mcs = rx_vect1->vht.mcs;
			sinfo->rxrate.nss = rx_vect1->vht.nss + 1;
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
		case FORMATMOD_HE_MU:
			sinfo->rxrate.he_ru_alloc = rx_vect1->he.ru_size;
			fallthrough;
		case FORMATMOD_HE_SU:
		case FORMATMOD_HE_ER:
		case FORMATMOD_HE_TB:
			sinfo->rxrate.flags = RATE_INFO_FLAGS_HE_MCS;
			sinfo->rxrate.mcs = rx_vect1->he.mcs;
			sinfo->rxrate.nss = rx_vect1->he.nss + 1;
			sinfo->rxrate.he_gi = rx_vect1->he.gi_type;
			sinfo->rxrate.he_dcm = rx_vect1->he.dcm;
			break;
#endif
		default :
			return -EINVAL;
	}

	switch (RC_RATE_GET(BW, stats->last_tx)) {
	case PHY_CHNL_BW_20:
		sinfo->txrate.bw = RATE_INFO_BW_20;
		break;
	case PHY_CHNL_BW_40:
		sinfo->txrate.bw = RATE_INFO_BW_40;
		break;
	case PHY_CHNL_BW_80:
		sinfo->txrate.bw = RATE_INFO_BW_80;
		break;
	case PHY_CHNL_BW_160:
		sinfo->txrate.bw = RATE_INFO_BW_160;
		break;
	default:
		sinfo->txrate.bw = RATE_INFO_BW_HE_RU;
		break;
	}

	switch (RC_RATE_GET(FORMAT_MOD, stats->last_tx)) {
	case FORMATMOD_NON_HT:
	case FORMATMOD_NON_HT_DUP_OFDM:
		sinfo->txrate.flags = 0;
		sinfo->txrate.legacy = legrates[RC_RATE_GET(MCS, stats->last_tx)].rate;
		break;
	case FORMATMOD_HT_MF:
	case FORMATMOD_HT_GF:
		sinfo->txrate.flags = RATE_INFO_FLAGS_MCS;
		if (RC_RATE_GET(GI, stats->last_tx))
			sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
		sinfo->txrate.mcs = RC_RATE_GET(MCS, stats->last_tx);
		break;
	case FORMATMOD_VHT:
		sinfo->txrate.flags = RATE_INFO_FLAGS_VHT_MCS;
		if (RC_RATE_GET(GI, stats->last_tx))
			sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
		sinfo->txrate.mcs = RC_RATE_GET(MCS, stats->last_tx);
		sinfo->txrate.nss = RC_RATE_GET(NSS, stats->last_tx) + 1;
		break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	case FORMATMOD_HE_MU:
		sinfo->txrate.he_ru_alloc = RC_RATE_GET(RU_SIZE, stats->last_tx);
		fallthrough;
	case FORMATMOD_HE_SU:
	case FORMATMOD_HE_ER:
	case FORMATMOD_HE_TB:
		sinfo->txrate.flags = RATE_INFO_FLAGS_HE_MCS;
		sinfo->txrate.mcs = RC_RATE_GET(MCS, stats->last_tx);
		sinfo->txrate.nss = RC_RATE_GET(NSS, stats->last_tx) + 1;
		sinfo->txrate.he_gi = RC_RATE_GET(GI, stats->last_tx);
		sinfo->txrate.he_dcm = RC_RATE_GET(DCM, stats->last_tx);
		break;
#endif
	default:
		return -EINVAL;
	}

	sinfo->filled = (BIT(NL80211_STA_INFO_INACTIVE_TIME) |
					 BIT(NL80211_STA_INFO_RX_BYTES64)	|
					 BIT(NL80211_STA_INFO_TX_BYTES64)	|
					 BIT(NL80211_STA_INFO_RX_PACKETS)	|
					 BIT(NL80211_STA_INFO_TX_PACKETS)	|
					 BIT(NL80211_STA_INFO_SIGNAL)		|
					 BIT(NL80211_STA_INFO_RX_BITRATE)	|
					 BIT(NL80211_STA_INFO_CONNECTED_TIME)	|
					 BIT(NL80211_STA_INFO_TX_BITRATE));

	// Mesh specific info
	if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_MESH_POINT)
	{
		struct mesh_peer_info_cfm peer_info_cfm;
		if (cls_wifi_send_mesh_peer_info_req(vif->cls_wifi_hw, vif, sta->sta_idx,
										 &peer_info_cfm))
			return -ENOMEM;

		peer_info_cfm.last_bcn_age = peer_info_cfm.last_bcn_age / 1000;
		if (peer_info_cfm.last_bcn_age < sinfo->inactive_time)
			sinfo->inactive_time = peer_info_cfm.last_bcn_age;

		sinfo->llid = peer_info_cfm.local_link_id;
		sinfo->plid = peer_info_cfm.peer_link_id;
		sinfo->plink_state = peer_info_cfm.link_state;
		sinfo->local_pm = peer_info_cfm.local_ps_mode;
		sinfo->peer_pm = peer_info_cfm.peer_ps_mode;
		sinfo->nonpeer_pm = peer_info_cfm.non_peer_ps_mode;

		sinfo->filled |= (BIT(NL80211_STA_INFO_LLID) |
						  BIT(NL80211_STA_INFO_PLID) |
						  BIT(NL80211_STA_INFO_PLINK_STATE) |
						  BIT(NL80211_STA_INFO_LOCAL_PM) |
						  BIT(NL80211_STA_INFO_PEER_PM) |
						  BIT(NL80211_STA_INFO_NONPEER_PM));
	}

	return 0;
}

/**
 * @get_station: get station information for the station identified by @mac
 */
static int cls_wifi_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
									 const u8 *mac, struct station_info *sinfo)
{
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_sta *sta = NULL;

	if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_MONITOR)
		return -EINVAL;
	else if ((CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_STATION) ||
			 (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_P2P_CLIENT)) {
		if (vif->sta.ap && ether_addr_equal(vif->sta.ap->mac_addr, mac))
			sta = vif->sta.ap;
	}
	else
	{
		struct cls_wifi_sta *sta_iter;
		list_for_each_entry(sta_iter, &vif->ap.sta_list, list) {
			if (sta_iter->valid && ether_addr_equal(sta_iter->mac_addr, mac)) {
				sta = sta_iter;
				break;
			}
		}
	}

	if (sta)
		return cls_wifi_fill_station_info(sta, vif, sinfo);

	return -EINVAL;
}

/**
 * @dump_station: dump station callback -- resume dump at index @idx
 */
static int cls_wifi_cfg80211_dump_station(struct wiphy *wiphy, struct net_device *dev,
									  int idx, u8 *mac, struct station_info *sinfo)
{
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_sta *sta = NULL;

	if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_MONITOR)
		return -EINVAL;
	else if ((CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_STATION) ||
			 (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_P2P_CLIENT)) {
		if ((idx == 0) && vif->sta.ap && vif->sta.ap->valid)
			sta = vif->sta.ap;
	} else {
		struct cls_wifi_sta *sta_iter;
		int i = 0;
		list_for_each_entry(sta_iter, &vif->ap.sta_list, list) {
			if (i == idx) {
				sta = sta_iter;
				break;
			}
			i++;
		}
	}

	if (sta == NULL)
		return -ENOENT;

	/* Copy peer MAC address */
	memcpy(mac, &sta->mac_addr, ETH_ALEN);

	return cls_wifi_fill_station_info(sta, vif, sinfo);
}

/**
 * @add_mpath: add a fixed mesh path
 */
static int cls_wifi_cfg80211_add_mpath(struct wiphy *wiphy, struct net_device *dev,
								   const u8 *dst, const u8 *next_hop)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct mesh_path_update_cfm cfm;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	return cls_wifi_send_mesh_path_update_req(cls_wifi_hw, cls_wifi_vif, dst, next_hop, &cfm);
}

/**
 * @del_mpath: delete a given mesh path
 */
static int cls_wifi_cfg80211_del_mpath(struct wiphy *wiphy, struct net_device *dev,
								   const u8 *dst)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct mesh_path_update_cfm cfm;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	return cls_wifi_send_mesh_path_update_req(cls_wifi_hw, cls_wifi_vif, dst, NULL, &cfm);
}

/**
 * @change_mpath: change a given mesh path
 */
static int cls_wifi_cfg80211_change_mpath(struct wiphy *wiphy, struct net_device *dev,
									  const u8 *dst, const u8 *next_hop)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct mesh_path_update_cfm cfm;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	return cls_wifi_send_mesh_path_update_req(cls_wifi_hw, cls_wifi_vif, dst, next_hop, &cfm);
}

/**
 * @get_mpath: get a mesh path for the given parameters
 */
static int cls_wifi_cfg80211_get_mpath(struct wiphy *wiphy, struct net_device *dev,
								   u8 *dst, u8 *next_hop, struct mpath_info *pinfo)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_mesh_path *mesh_path = NULL;
	struct cls_wifi_mesh_path *cur;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	list_for_each_entry(cur, &cls_wifi_vif->ap.mpath_list, list) {
		/* Compare the path target address and the provided destination address */
		if (memcmp(dst, &cur->tgt_mac_addr, ETH_ALEN)) {
			continue;
		}

		mesh_path = cur;
		break;
	}

	if (mesh_path == NULL)
		return -ENOENT;

	/* Copy next HOP MAC address */
	if (mesh_path->nhop_sta)
		memcpy(next_hop, &mesh_path->nhop_sta->mac_addr, ETH_ALEN);

	/* Fill path information */
	pinfo->filled = 0;
	pinfo->generation = cls_wifi_vif->generation;

	return 0;
}

/**
 * @dump_mpath: dump mesh path callback -- resume dump at index @idx
 */
static int cls_wifi_cfg80211_dump_mpath(struct wiphy *wiphy, struct net_device *dev,
									int idx, u8 *dst, u8 *next_hop,
									struct mpath_info *pinfo)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_mesh_path *mesh_path = NULL;
	struct cls_wifi_mesh_path *cur;
	int i = 0;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	list_for_each_entry(cur, &cls_wifi_vif->ap.mpath_list, list) {
		if (i < idx) {
			i++;
			continue;
		}

		mesh_path = cur;
		break;
	}

	if (mesh_path == NULL)
		return -ENOENT;

	/* Copy target and next hop MAC address */
	memcpy(dst, &mesh_path->tgt_mac_addr, ETH_ALEN);
	if (mesh_path->nhop_sta)
		memcpy(next_hop, &mesh_path->nhop_sta->mac_addr, ETH_ALEN);

	/* Fill path information */
	pinfo->filled = 0;
	pinfo->generation = cls_wifi_vif->generation;

	return 0;
}

/**
 * @get_mpp: get a mesh proxy path for the given parameters
 */
static int cls_wifi_cfg80211_get_mpp(struct wiphy *wiphy, struct net_device *dev,
								 u8 *dst, u8 *mpp, struct mpath_info *pinfo)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_mesh_proxy *mesh_proxy = NULL;
	struct cls_wifi_mesh_proxy *cur;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	list_for_each_entry(cur, &cls_wifi_vif->ap.proxy_list, list) {
		if (cur->local) {
			continue;
		}

		/* Compare the path target address and the provided destination address */
		if (memcmp(dst, &cur->ext_sta_addr, ETH_ALEN)) {
			continue;
		}

		mesh_proxy = cur;
		break;
	}

	if (mesh_proxy == NULL)
		return -ENOENT;

	memcpy(mpp, &mesh_proxy->proxy_addr, ETH_ALEN);

	/* Fill path information */
	pinfo->filled = 0;
	pinfo->generation = cls_wifi_vif->generation;

	return 0;
}

/**
 * @dump_mpp: dump mesh proxy path callback -- resume dump at index @idx
 */
static int cls_wifi_cfg80211_dump_mpp(struct wiphy *wiphy, struct net_device *dev,
								  int idx, u8 *dst, u8 *mpp, struct mpath_info *pinfo)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_mesh_proxy *mesh_proxy = NULL;
	struct cls_wifi_mesh_proxy *cur;
	int i = 0;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	list_for_each_entry(cur, &cls_wifi_vif->ap.proxy_list, list) {
		if (cur->local) {
			continue;
		}

		if (i < idx) {
			i++;
			continue;
		}

		mesh_proxy = cur;
		break;
	}

	if (mesh_proxy == NULL)
		return -ENOENT;

	/* Copy target MAC address */
	memcpy(dst, &mesh_proxy->ext_sta_addr, ETH_ALEN);
	memcpy(mpp, &mesh_proxy->proxy_addr, ETH_ALEN);

	/* Fill path information */
	pinfo->filled = 0;
	pinfo->generation = cls_wifi_vif->generation;

	return 0;
}

/**
 * @get_mesh_config: Get the current mesh configuration
 */
static int cls_wifi_cfg80211_get_mesh_config(struct wiphy *wiphy, struct net_device *dev,
										 struct mesh_config *conf)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	return 0;
}

/**
 * @update_mesh_config: Update mesh parameters on a running mesh.
 */
static int cls_wifi_cfg80211_update_mesh_config(struct wiphy *wiphy, struct net_device *dev,
											u32 mask, const struct mesh_config *nconf)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct mesh_update_cfm cfm;
	int status;

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	if (mask & CO_BIT(NL80211_MESHCONF_POWER_MODE - 1)) {
		cls_wifi_vif->ap.next_mesh_pm = nconf->power_mode;

		if (!list_empty(&cls_wifi_vif->ap.sta_list)) {
			// If there are mesh links we don't want to update the power mode
			// It will be updated with cls_wifi_update_mesh_power_mode() when the
			// ps mode of a link is updated or when a new link is added/removed
			mask &= ~BIT(NL80211_MESHCONF_POWER_MODE - 1);

			if (!mask)
				return 0;
		}
	}

	status = cls_wifi_send_mesh_update_req(cls_wifi_hw, cls_wifi_vif, mask, nconf, &cfm);

	if (!status && (cfm.status != 0))
		status = -EINVAL;

	return status;
}

/**
 * @join_mesh: join the mesh network with the specified parameters
 * (invoked with the wireless_dev mutex held)
 */
static int cls_wifi_cfg80211_join_mesh(struct wiphy *wiphy, struct net_device *dev,
								   const struct mesh_config *conf, const struct mesh_setup *setup)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct mesh_start_cfm mesh_start_cfm;
	int error = 0;
	u8 txq_status = 0;
	/* STA for BC/MC traffic */
	struct cls_wifi_sta *sta;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) != NL80211_IFTYPE_MESH_POINT)
		return -ENOTSUPP;

	/* Forward the information to the UMAC */
	if ((error = cls_wifi_send_mesh_start_req(cls_wifi_hw, cls_wifi_vif, conf, setup, &mesh_start_cfm))) {
		return error;
	}

	/* Check the status */
	switch (mesh_start_cfm.status) {
		case CO_OK:
			cls_wifi_vif->ap.bcmc_index = mesh_start_cfm.bcmc_idx;
			cls_wifi_vif->ap.flags = 0;
			cls_wifi_vif->ap.bcn_interval = setup->beacon_interval;
			cls_wifi_vif->use_4addr = true;
			if (setup->user_mpm)
				cls_wifi_vif->ap.flags |= CLS_WIFI_AP_USER_MESH_PM;

			sta = &cls_wifi_hw->sta_table[mesh_start_cfm.bcmc_idx];
			sta->valid = true;
			sta->aid = 0;
			sta->sta_idx = mesh_start_cfm.bcmc_idx;
			sta->ch_idx = mesh_start_cfm.ch_idx;
			sta->vif_idx = cls_wifi_vif->vif_index;
			sta->qos = true;
			sta->acm = 0;
			sta->ps.active = false;
			sta->listen_interval = 5;
			spin_lock_bh(&cls_wifi_hw->cb_lock);
			cls_wifi_chanctx_link(cls_wifi_vif, mesh_start_cfm.ch_idx,
							  (struct cfg80211_chan_def *)(&setup->chandef));
			if (cls_wifi_hw->cur_chanctx != mesh_start_cfm.ch_idx) {
				txq_status = CLS_WIFI_TXQ_STOP_CHAN;
			}
			cls_wifi_txq_vif_init(cls_wifi_hw, cls_wifi_vif, txq_status);
			spin_unlock_bh(&cls_wifi_hw->cb_lock);

			netif_tx_start_all_queues(dev);
			netif_carrier_on(dev);

			/* If the AP channel is already the active, we probably skip radar
			   activation on MM_CHANNEL_SWITCH_IND (unless another vif use this
			   ctxt). In anycase retest if radar detection must be activated
			 */
			if (cls_wifi_hw->cur_chanctx == mesh_start_cfm.ch_idx) {
				cls_wifi_radar_detection_enable_on_cur_channel(cls_wifi_hw, cls_wifi_vif);
			}
			break;

		case CO_BUSY:
			error = -EINPROGRESS;
			break;

		default:
			error = -EIO;
			break;
	}

	/* Print information about the operation */
	if (error) {
		netdev_info(dev, "Failed to start MP (%d)", error);
	} else {
		netdev_info(dev, "MP started: ch=%d, bcmc_idx=%d",
					cls_wifi_vif->ch_index, cls_wifi_vif->ap.bcmc_index);
	}

	return error;
}

/**
 * @leave_mesh: leave the current mesh network
 * (invoked with the wireless_dev mutex held)
 */
static int cls_wifi_cfg80211_leave_mesh(struct wiphy *wiphy, struct net_device *dev)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct mesh_stop_cfm mesh_stop_cfm;
	int error = 0;

	error = cls_wifi_send_mesh_stop_req(cls_wifi_hw, cls_wifi_vif, &mesh_stop_cfm);

	if (error == 0) {
		/* Check the status */
		switch (mesh_stop_cfm.status) {
			case CO_OK:
				spin_lock_bh(&cls_wifi_hw->cb_lock);
				cls_wifi_chanctx_unlink(cls_wifi_vif);
				cls_wifi_radar_cancel_cac(&cls_wifi_hw->radar);
				spin_unlock_bh(&cls_wifi_hw->cb_lock);
				/* delete BC/MC STA */
				cls_wifi_txq_vif_deinit(cls_wifi_hw, cls_wifi_vif);
				cls_wifi_del_bcn(&cls_wifi_vif->ap.bcn);

				netif_tx_stop_all_queues(dev);
				netif_carrier_off(dev);

				break;

			default:
				error = -EIO;
				break;
		}
	}

	if (error) {
		netdev_info(dev, "Failed to stop MP");
	} else {
		netdev_info(dev, "MP Stopped");
	}

	return 0;
}

static int cls_wifi_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
			  struct net_device *dev, const u8 *peer,
			  const struct cfg80211_bitrate_mask *mask)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	enum nl80211_band band = NUM_NL80211_BANDS;
	int i = 0;

	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
		band = NL80211_BAND_2GHZ;
	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
		band = NL80211_BAND_5GHZ;
	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_6G)
		band = NL80211_BAND_6GHZ;

	if (band == NUM_NL80211_BANDS)
		return -ENOTSUPP;

	if (mask->control[band].legacy)
		cls_wifi_hw->control.legacy = mask->control[band].legacy;


	for (; i < IEEE80211_HT_MCS_MASK_LEN; i++) {
		if (mask->control[band].ht_mcs[i])
			cls_wifi_hw->control.ht_mcs[i] = mask->control[band].ht_mcs[i];
	}
	for (i = 0; i < NL80211_VHT_NSS_MAX; i++) {
		if (mask->control[band].vht_mcs[i])
			cls_wifi_hw->control.vht_mcs[i] = mask->control[band].vht_mcs[i];
	}

	for (i = 0; i < NL80211_HE_NSS_MAX; i++) {
		if (mask->control[band].he_mcs[i])
			cls_wifi_hw->control.he_mcs[i] = mask->control[band].he_mcs[i];
	}
	cls_wifi_hw->control.gi = mask->control[band].gi;
	cls_wifi_hw->control.he_gi = mask->control[band].he_gi;
	cls_wifi_hw->control.he_ltf = mask->control[band].he_ltf;

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_ARCH_CLOURNEY)
static int cls_wifi_cfg80211_color_change(struct wiphy *wiphy,
		struct net_device *dev,
		struct cfg80211_color_change_settings *params)
{

	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *vif = netdev_priv(dev);
	struct cls_wifi_ipc_buf buf;
	struct cls_wifi_bcn *bcn, *bcn_after;
	struct cls_wifi_cca *cca;
	u16 cca_oft;
	u8 *bcn_buf;
	int error = 0;

	if (vif->ap.csa || vif->ap.cca)
		return -EBUSY;

	/* Build the new beacon with CCA IE */
	bcn = &vif->ap.bcn;
	bcn_buf = cls_wifi_build_bcn(bcn, &params->beacon_color_change);
	if (!bcn_buf)
		return -ENOMEM;

	cca_oft = params->counter_offset_beacon + bcn->head_len + bcn->tim_len;

	if ((error = cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &buf, bcn_buf, bcn->len))) {
		netdev_err(dev, "Failed to allocate IPC buf for CCA beacon\n");
		kfree(bcn_buf);
		return error;
	}

	/* Build the beacon to use after CCA. It will only be sent to fw once
	   CCA is over, but do it before sending the beacon as it must be ready
	   when CCA is finished. */
	cca = kzalloc(sizeof(struct cls_wifi_cca), GFP_KERNEL);
	if (!cca) {
		error = -ENOMEM;
		goto end;
	}

	bcn_after = &cca->bcn;
	bcn_buf = cls_wifi_build_bcn(bcn_after, &params->beacon_next);
	if (!bcn_buf) {
		error = -ENOMEM;
		cls_wifi_del_cca(vif);
		goto end;
	}

	if ((error = cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &cca->buf, bcn_buf, bcn_after->len))) {
		netdev_err(dev, "Failed to allocate IPC buf for after CSA beacon\n");
		kfree(bcn_buf);
		goto end;
	}

	vif->ap.cca = cca;
	cca->vif = vif;
	cca->count = params->count;
	cca->color = params->color;

	/* Send new Beacon. FW will extract channel and count from the beacon */
	error = cls_wifi_send_bcn_change(cls_wifi_hw, vif->vif_index, buf.dma_addr,
			bcn->len, bcn->head_len, bcn->tim_len, NULL, cca_oft);

	if (error) {
		cls_wifi_del_cca(vif);
	} else {
		INIT_WORK(&cca->work, cls_wifi_cca_finish);
		cfg80211_color_change_started_notify(dev, params->count);
	}

end:
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &buf);
	return error;
}
#endif

static struct cfg80211_ops cls_wifi_cfg80211_ops = {
	.add_virtual_intf = cls_wifi_cfg80211_add_iface,
	.del_virtual_intf = cls_wifi_cfg80211_del_iface,
	.change_virtual_intf = cls_wifi_cfg80211_change_iface,
	.scan = cls_wifi_cfg80211_scan,
	.abort_scan = cls_wifi_cfg80211_abort_scan,
	.connect = cls_wifi_cfg80211_connect,
	.disconnect = cls_wifi_cfg80211_disconnect,
	.add_key = cls_wifi_cfg80211_add_key,
	.get_key = cls_wifi_cfg80211_get_key,
	.del_key = cls_wifi_cfg80211_del_key,
	.set_default_key = cls_wifi_cfg80211_set_default_key,
	.set_default_mgmt_key = cls_wifi_cfg80211_set_default_mgmt_key,
	.add_station = cls_wifi_cfg80211_add_station,
	.del_station = cls_wifi_cfg80211_del_station,
	.change_station = cls_wifi_cfg80211_change_station,
	.mgmt_tx = cls_wifi_cfg80211_mgmt_tx,
	.mgmt_tx_cancel_wait = cls_wifi_cfg80211_mgmt_tx_cancel_wait,
	.start_ap = cls_wifi_cfg80211_start_ap,
	.change_beacon = cls_wifi_cfg80211_change_beacon,
	.stop_ap = cls_wifi_cfg80211_stop_ap,
	.set_monitor_channel = cls_wifi_cfg80211_set_monitor_channel,
	.probe_client = cls_wifi_cfg80211_probe_client,
	.set_wiphy_params = cls_wifi_cfg80211_set_wiphy_params,
	.set_txq_params = cls_wifi_cfg80211_set_txq_params,
	.set_tx_power = cls_wifi_cfg80211_set_tx_power,
	.get_tx_power = cls_wifi_cfg80211_get_tx_power,
	.set_power_mgmt = cls_wifi_cfg80211_set_power_mgmt,
	.get_station = cls_wifi_cfg80211_get_station,
	.dump_station = cls_wifi_cfg80211_dump_station,
	.remain_on_channel = cls_wifi_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = cls_wifi_cfg80211_cancel_remain_on_channel,
	.dump_survey = cls_wifi_cfg80211_dump_survey,
	.get_channel = cls_wifi_cfg80211_get_channel,
	.start_radar_detection = cls_wifi_cfg80211_start_radar_detection,
	.update_ft_ies = cls_wifi_cfg80211_update_ft_ies,
	.set_cqm_rssi_config = cls_wifi_cfg80211_set_cqm_rssi_config,
	.channel_switch = cls_wifi_cfg80211_channel_switch,
	.tdls_channel_switch = cls_wifi_cfg80211_tdls_channel_switch,
	.tdls_cancel_channel_switch = cls_wifi_cfg80211_tdls_cancel_channel_switch,
	.tdls_mgmt = cls_wifi_cfg80211_tdls_mgmt,
	.tdls_oper = cls_wifi_cfg80211_tdls_oper,
	.change_bss = cls_wifi_cfg80211_change_bss,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	.external_auth = cls_wifi_cfg80211_external_auth,
#endif
	.set_bitrate_mask = cls_wifi_cfg80211_set_bitrate_mask,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_ARCH_CLOURNEY)
	.color_change = cls_wifi_cfg80211_color_change,
#endif
};


/*********************************************************************
 * Init/Exit functions
 *********************************************************************/
static void cls_wifi_wdev_unregister(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_vif *cls_wifi_vif, *tmp;

	rtnl_lock();
	list_for_each_entry_safe(cls_wifi_vif, tmp, &cls_wifi_hw->vifs, list) {
		cls_wifi_cfg80211_del_iface(cls_wifi_hw->wiphy, &cls_wifi_vif->wdev);
	}
	rtnl_unlock();
}

static void cls_wifi_set_vers(struct cls_wifi_hw *cls_wifi_hw)
{
	u32 vers = cls_wifi_hw->version_cfm.version_lmac;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	snprintf(cls_wifi_hw->wiphy->fw_version,
			 sizeof(cls_wifi_hw->wiphy->fw_version), "%d.%d.%d.%d",
			 (vers & (0xff << 24)) >> 24, (vers & (0xff << 16)) >> 16,
			 (vers & (0xff <<  8)) >>  8, (vers & (0xff <<  0)) >>  0);

	cls_wifi_hw->machw_type = cls_wifi_machw_type(cls_wifi_hw->version_cfm.version_machw_2);
}

static void cls_wifi_reg_notifier(struct wiphy *wiphy,
							  struct regulatory_request *request)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);

	// For now trust all initiator
	cls_wifi_radar_set_domain(&cls_wifi_hw->radar, request->dfs_region, request->alpha2);
	cls_wifi_send_me_chan_config_req(cls_wifi_hw);
}

static void cls_wifi_enable_mesh(struct cls_wifi_hw *cls_wifi_hw)
{
	struct wiphy *wiphy = cls_wifi_hw->wiphy;

	if (!cls_wifi_hw->radio_params->mesh)
		return;

	cls_wifi_cfg80211_ops.add_mpath = cls_wifi_cfg80211_add_mpath;
	cls_wifi_cfg80211_ops.del_mpath = cls_wifi_cfg80211_del_mpath;
	cls_wifi_cfg80211_ops.change_mpath = cls_wifi_cfg80211_change_mpath;
	cls_wifi_cfg80211_ops.get_mpath = cls_wifi_cfg80211_get_mpath;
	cls_wifi_cfg80211_ops.dump_mpath = cls_wifi_cfg80211_dump_mpath;
	cls_wifi_cfg80211_ops.get_mpp = cls_wifi_cfg80211_get_mpp;
	cls_wifi_cfg80211_ops.dump_mpp = cls_wifi_cfg80211_dump_mpp;
	cls_wifi_cfg80211_ops.get_mesh_config = cls_wifi_cfg80211_get_mesh_config;
	cls_wifi_cfg80211_ops.update_mesh_config = cls_wifi_cfg80211_update_mesh_config;
	cls_wifi_cfg80211_ops.join_mesh = cls_wifi_cfg80211_join_mesh;
	cls_wifi_cfg80211_ops.leave_mesh = cls_wifi_cfg80211_leave_mesh;

	wiphy->flags |= (WIPHY_FLAG_MESH_AUTH | WIPHY_FLAG_IBSS_RSN);
	wiphy->features |= NL80211_FEATURE_USERSPACE_MPM;
	wiphy->interface_modes |= BIT(NL80211_IFTYPE_MESH_POINT);

	cls_wifi_hw->if_cfg80211.limits[0].types |= BIT(NL80211_IFTYPE_MESH_POINT);
	cls_wifi_hw->if_cfg80211.limits[1].types |= BIT(NL80211_IFTYPE_MESH_POINT);
}

int cls_wifi_wait_wpu_ready(struct cls_wifi_hw *cls_wifi_hw, uint32_t timeout)
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
		if(wpu_ipc_pattern != ((uint32_t)WPU_IPC_PATTERN_MAGIC)){
			msleep(10);
			wait_time += 10;
			if(wait_time < wait_timeout){
				continue;
			}else{
				if(wpu_ipc_pattern != ((uint32_t)WPU_IPC_PATTERN_MAGIC)){
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

static void cls_wifi_pps_cal_timer_cb(struct timer_list *t)
{
	struct cls_wifi_hw *cls_wifi_hw = from_timer(cls_wifi_hw, t, pps_cal_timer);
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	uint32_t new_pps;

	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_AP) {
			list_for_each_entry(sta, &vif->ap.sta_list, list) {
				if (sta->stats.tx_pkts_old == 0) {
					sta->stats.tx_pkts_old = sta->stats.tx_pkts;
					continue;
				}
				new_pps = sta->stats.tx_pkts - sta->stats.tx_pkts_old;
				sta->stats.tx_pps = (new_pps * 64 + sta->stats.tx_pps * 64) / 128;
				sta->stats.tx_pkts_old = sta->stats.tx_pkts;
			}
		}
	}

	mod_timer(t, jiffies + CLS_WIFI_PPS_CAL_INTERVAL);
}

void cls_wifi_start_pps_cal_timer(struct cls_wifi_hw *wifi_hw, bool enable)
{
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;

	if (enable) {
		list_for_each_entry(vif, &wifi_hw->vifs, list) {
			if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_AP) {
				list_for_each_entry(sta, &vif->ap.sta_list, list) {
					sta->stats.tx_pkts_old = 0;
					sta->stats.tx_pps = 0;
				}
			}
		}
		if (!timer_pending(&wifi_hw->pps_cal_timer))
			mod_timer(&wifi_hw->pps_cal_timer, jiffies + CLS_WIFI_PPS_CAL_INTERVAL);
	} else {
		del_timer_sync(&wifi_hw->pps_cal_timer);
	}
}

/**
 *
 */
int cls_wifi_cfg80211_init(struct cls_wifi_plat *cls_wifi_plat, void **platform_data, u8 radio_idx)
{
	struct cls_wifi_hw *cls_wifi_hw;
	struct cls_wifi_conf_file init_conf;
	int ret = 0;
	struct wiphy *wiphy;
	struct wireless_dev *wdev;
	int i;
	uint16_t combinations_array_size;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* create a new wiphy for use with cfg80211 */
	wiphy = wiphy_new(&cls_wifi_cfg80211_ops, sizeof(struct cls_wifi_hw));

	if (!wiphy) {
		dev_err(cls_wifi_plat->dev, "Failed to create new wiphy\n");
		ret = -ENOMEM;
		goto err_out;
	}

	cls_wifi_hw = wiphy_priv(wiphy);
	cls_wifi_hw->wiphy = wiphy;
	cls_wifi_hw->plat = cls_wifi_plat;
	cls_wifi_hw->dev = cls_wifi_plat->dev;
	cls_wifi_hw->radio_idx = radio_idx;
	cls_wifi_hw->band_cap = cls_wifi_plat->hw_params.band_cap[radio_idx];
	cls_wifi_plat->cls_wifi_hw[radio_idx] = cls_wifi_hw;

	cls_wifi_hw->radio_params = kzalloc(sizeof(struct cls_wifi_mod_params), GFP_KERNEL);
	if (!cls_wifi_hw->radio_params) {
		dev_err(cls_wifi_plat->dev, "Failed to create new mod_params\n");
		ret = -ENOMEM;
		goto err_out;
	}
	memcpy(cls_wifi_hw->radio_params, &cls_wifi_mod_params, sizeof(cls_wifi_mod_params));

	cls_wifi_hw->tcp_pacing_shift = 7;

	/* set device pointer for wiphy */
	set_wiphy_dev(wiphy, cls_wifi_hw->dev);

	/* Create cache to allocate sw_txhdr */
	cls_wifi_hw->sw_txhdr_cache = KMEM_CACHE(cls_wifi_sw_txhdr, 0);
	if (!cls_wifi_hw->sw_txhdr_cache) {
		wiphy_err(wiphy, "Cannot allocate cache for sw TX header\n");
		ret = -ENOMEM;
		goto err_cache;
	}

	if ((ret = cls_wifi_parse_configfile(cls_wifi_hw, CLS_WIFI_CONFIG_FW_NAME, &init_conf))) {
		wiphy_err(wiphy, "cls_wifi_parse_configfile failed\n");
		goto err_config;
	}

	INIT_WORK(&cls_wifi_hw->dbg_work, cls_wifi_dbg_work);
	INIT_WORK(&cls_wifi_hw->dbg_cmn_work, cls_wifi_dbg_cmn_work);
	INIT_WORK(&cls_wifi_hw->csi_work, cls_wifi_csi_work);

	cls_wifi_hw->vif_started = 0;
	cls_wifi_hw->monitor_vif = CLS_WIFI_INVALID_VIF;

	cls_wifi_hw->scan_ie.addr = NULL;

	for (i = 0; i < CLS_ITF_MAX; i++)
		cls_wifi_hw->avail_idx_map[i / 16] |= BIT(i % 16);

	cls_wifi_hwq_init(cls_wifi_hw);

	if (cls_wifi_statable_prepare(cls_wifi_hw))
		goto err_config;
	if (cls_wifi_txq_prepare(cls_wifi_hw))
		goto err_statable;
	if (cls_wifi_rxbuf_prepare(cls_wifi_hw))
		goto err_txq;

	cls_wifi_he_mu_init(cls_wifi_hw);

	cls_wifi_txpower_entry_init(cls_wifi_hw);

	cls_wifi_hw->roc = NULL;

	memcpy(wiphy->perm_addr, init_conf.mac_addr, ETH_ALEN);

	memcpy(cls_wifi_hw->if_cfg80211.mgmt_stypes, &cls_wifi_default_mgmt_stypes, sizeof(cls_wifi_default_mgmt_stypes));
	wiphy->mgmt_stypes = cls_wifi_hw->if_cfg80211.mgmt_stypes;
	if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
	{
		memcpy(&cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ], &cls_wifi_band_2GHz, sizeof(cls_wifi_band_2GHz));
		wiphy->bands[NL80211_BAND_2GHZ] = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ];
	}
	else
	{
		memset(&cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ], 0, sizeof(cls_wifi_band_2GHz));
		wiphy->bands[NL80211_BAND_2GHZ] = NULL;
	}

	if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
	{
		memcpy(&cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ], &cls_wifi_band_5GHz, sizeof(cls_wifi_band_5GHz));
		wiphy->bands[NL80211_BAND_5GHZ] = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ];
	}
	else
	{
		memset(&cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ], 0, sizeof(cls_wifi_band_5GHz));
		wiphy->bands[NL80211_BAND_5GHZ] = NULL;
	}

	wiphy->interface_modes =
		BIT(NL80211_IFTYPE_STATION)	 |
		BIT(NL80211_IFTYPE_AP)		  |
		BIT(NL80211_IFTYPE_AP_VLAN)	 |
		BIT(NL80211_IFTYPE_P2P_CLIENT)  |
		BIT(NL80211_IFTYPE_P2P_GO)	  |
		BIT(NL80211_IFTYPE_MONITOR);
	wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL |
		WIPHY_FLAG_HAS_CHANNEL_SWITCH |
		WIPHY_FLAG_4ADDR_STATION |
		WIPHY_FLAG_4ADDR_AP |
		WIPHY_FLAG_REPORTS_OBSS |
		WIPHY_FLAG_OFFCHAN_TX;

	wiphy->max_scan_ssids = SCAN_SSID_MAX;
	wiphy->max_scan_ie_len = SCANU_MAX_IE_LEN;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
	wiphy->support_mbssid = 1;
#endif

	wiphy->max_num_csa_counters = BCN_MAX_CSA_CPT;

	wiphy->max_remain_on_channel_duration = cls_wifi_hw->radio_params->roc_dur_max;

	wiphy->features |= NL80211_FEATURE_NEED_OBSS_SCAN |
		NL80211_FEATURE_SK_TX_STATUS |
		NL80211_FEATURE_VIF_TXPOWER |
		NL80211_FEATURE_ACTIVE_MONITOR |
		NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	wiphy->features |= NL80211_FEATURE_SAE;
#endif

	wiphy->features |= NL80211_FEATURE_MAC_ON_CREATE;

	cls_wifi_iface_combinations_init(cls_wifi_hw, &combinations_array_size);

	wiphy->iface_combinations   = cls_wifi_hw->if_cfg80211.combinations;
	wiphy->n_iface_combinations = combinations_array_size - 1;
	wiphy->reg_notifier = cls_wifi_reg_notifier;

	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

	memset(&cls_wifi_hw->if_cfg80211.cipher_suites, 0, sizeof(cls_wifi_hw->if_cfg80211.cipher_suites));
	memcpy(&cls_wifi_hw->if_cfg80211.cipher_suites, cipher_suites, sizeof(cipher_suites));

	wiphy->cipher_suites = cls_wifi_hw->if_cfg80211.cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites) - NB_RESERVED_CIPHER;

	cls_wifi_hw->ext_capa[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING;
	cls_wifi_hw->ext_capa[2] = WLAN_EXT_CAPA3_MULTI_BSSID_SUPPORT;
	cls_wifi_hw->ext_capa[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF;
	// Max number of MSDUs in A-MSDU = 3 (=> 8 subframes max)
	cls_wifi_hw->ext_capa[7] |= WLAN_EXT_CAPA8_MAX_MSDU_IN_AMSDU_LSB;
	cls_wifi_hw->ext_capa[8] = WLAN_EXT_CAPA9_MAX_MSDU_IN_AMSDU_MSB;

	wiphy->extended_capabilities = cls_wifi_hw->ext_capa;
	wiphy->extended_capabilities_mask = cls_wifi_hw->ext_capa;
	wiphy->extended_capabilities_len = ARRAY_SIZE(cls_wifi_hw->ext_capa);

	wiphy->n_vendor_commands = clsemi_vendor_cmds_size; ///CLSEMI_VENDOR_SUBCMDS_LAST - 1;
	wiphy->vendor_commands = clsemi_vendor_cmds;
	wiphy->n_vendor_events = clsemi_vendor_events_size; ///CLSEMI_VENDOR_SUBCMDS_LAST - 1;
	wiphy->vendor_events = clsemi_vendor_events;
	INIT_LIST_HEAD(&cls_wifi_hw->vifs);

	mutex_init(&cls_wifi_hw->dbgdump.mutex);
	spin_lock_init(&cls_wifi_hw->tx_lock);
	spin_lock_init(&cls_wifi_hw->cb_lock);
	spin_lock_init(&cls_wifi_hw->rosource_lock);

	if (cls_wifi_hw->radio_params->debug_mode &&
			cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
		for (i = 0; i < CLS_WIFI_MAX_RADIOS - 1; i++) {
			cls_wifi_hw->plat->hw_params.vdev_max[i] = CLS_WIFI_MERAK2000_CALI_VDEV_MAX;
			cls_wifi_hw->plat->hw_params.sta_max[i] = CLS_WIFI_MERAK2000_CALI_STA_MAX;
		}
	}

	if ((ret = cls_wifi_platform_on(cls_wifi_hw, NULL)))
		goto err_rxbuf;

	cls_wifi_update_uart_info(cls_wifi_plat, cls_wifi_hw->radio_idx);

	cls_wifi_wait_wpu_ready(cls_wifi_hw, 60 * 1000);

	/* Set platform params */
	if ((ret = cls_wifi_send_plat_param(cls_wifi_hw)) || (ret = cls_wifi_ipc_init(cls_wifi_hw, 1)))
		goto err_platon;

	/* Reset FW */
	if ((ret = cls_wifi_send_reset(cls_wifi_hw)))
		goto err_lmac_reqs;

	if ((ret = cls_wifi_send_version_req(cls_wifi_hw, &cls_wifi_hw->version_cfm)))
		goto err_lmac_reqs;

	cls_wifi_set_vers(cls_wifi_hw);

	if ((ret = cls_wifi_handle_dynparams(cls_wifi_hw, cls_wifi_hw->wiphy)))
		goto err_lmac_reqs;

	cls_wifi_enable_mesh(cls_wifi_hw);
	cls_wifi_radar_detection_init(&cls_wifi_hw->radar);

	/* Set parameters to firmware */
	cls_wifi_send_me_config_req(cls_wifi_hw);

	/* Only monitor mode supported when custom channels are enabled */
	if (cls_wifi_hw->radio_params->custchan) {
		cls_wifi_hw->if_cfg80211.limits[0].types = BIT(NL80211_IFTYPE_MONITOR);
		cls_wifi_hw->if_cfg80211.limits[1].types = BIT(NL80211_IFTYPE_MONITOR);
	}

	if ((ret = wiphy_register(wiphy))) {
		wiphy_err(wiphy, "Could not register wiphy device\n");
		goto err_register_wiphy;
	}
	cls_wifi_hw->wiphy_registered = true;

	/* Work to defer processing of rx buffer */
	INIT_WORK(&cls_wifi_hw->defer_rx.work, cls_wifi_rx_deferred);
	skb_queue_head_init(&cls_wifi_hw->defer_rx.sk_list);

#if WIFI_DRV_OPTI_SPIN_LOCK_CFG
	cls_wifi_hw->tx_free_skb_workqueue = alloc_workqueue("TXSKB_FREE_WQ", WQ_HIGHPRI, 0);
	INIT_WORK(&cls_wifi_hw->tx_free_skb_work, cls_wifi_tx_free_skb_work_handler);
	skb_queue_head_init(&cls_wifi_hw->tx_free_skb_queue);
#endif
	cls_wifi_hw->txwq_workqueue = alloc_workqueue("TXSKB_FREE_WQ", WQ_HIGHPRI, 0);
	spin_lock_init(&cls_wifi_hw->txwq_lock);

	cls_wifi_hw->snr_workqueue = alloc_workqueue("TXSKB_FREE_WQ", WQ_HIGHPRI, 0);

	/* Update regulatory (if needed) and set channel parameters to firmware
	   (must be done after WiPHY registration) */
	cls_wifi_custregd(cls_wifi_hw, wiphy);
	cls_wifi_send_me_chan_config_req(cls_wifi_hw);
	cls_wifi_csi_init(cls_wifi_hw);
#ifdef CONFIG_CLS_WIFI_HEMU_TX
	cls_wifi_hw->ul_params.ul_dbg_level = 0;
	cls_wifi_hw->ul_params.ul_duration_max = 4000;
	cls_wifi_hw->ul_params.nss_max = 1;
	if (radio_idx == 0) {
		//RADIO_2G_INDEX
		cls_wifi_hw->ul_params.mcs = 7;
		cls_wifi_hw->ul_params.ul_bw = 1;
	} else {
		cls_wifi_hw->ul_params.mcs = 7;
		cls_wifi_hw->ul_params.ul_bw = 3;
	}
	cls_wifi_hw->ul_params.fec_allowed = true;
	cls_wifi_hw->ul_params.ul_duration_force = false;
	cls_wifi_hw->ul_params.ul_on = 0;
	cls_wifi_hw->ul_params.trigger_type = 0; //HE_TRIG_TYPE_BASIC;
	cls_wifi_hw->ul_params.user_num = 4;
	cls_wifi_hw->ul_params.ul_duration = 20000;
	cls_wifi_hw->ul_params.tf_period = 4000;
	cls_wifi_hw->ul_params.sched_mode = 0;
	cls_wifi_hw->ul_params.work_mode = 1;
	cls_wifi_hw->ul_params.gi_ltf_mode = 2; //3.2GI+4xLTF

	cls_wifi_send_mm_ul_parameters(cls_wifi_hw);

	cls_wifi_hw->dl_params.ppdu_bw = PHY_CHNL_BW_80;
	cls_wifi_hw->dl_params.max_ampdu_subfrm = 16;
	cls_wifi_hw->dl_params.nss_max = 1;
	cls_wifi_hw->dl_params.mcs = 0x7;
	cls_wifi_hw->dl_params.fec_allowed = true;
	if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
		cls_wifi_hw->dl_params.dl_on = 1;
	else
		cls_wifi_hw->dl_params.dl_on = 0;

	cls_wifi_hw->dl_params.ack_type = vendor_ofdma_ack_type_max;
	cls_wifi_hw->dl_params.trigger_txbf = 0x00;
	cls_wifi_hw->dl_params.user_num = 4;
	cls_wifi_hw->dl_params.dl_20and80 = false;
	cls_wifi_hw->dl_params.work_mode = 0;
	cls_wifi_hw->dl_params.gi = 1;
	cls_wifi_hw->dl_params.ltf_type = 1;
	cls_wifi_hw->dl_params.log_level = 1;
	cls_wifi_hw->dl_params.pkt_len_threshold = 400;
	pr_info("%s he_dl_on %u he_ul_on %u\n", __func__, cls_wifi_mod_params.he_dl_on, cls_wifi_mod_params.he_ul_on);
	cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
#endif

	cls_wifi_hw->csa_fin_flag = false;
	cls_wifi_bf_init(cls_wifi_hw);
	*platform_data = cls_wifi_hw;

	if (cls_wifi_dbgfs_prepare(cls_wifi_hw))
		goto err_register_wiphy;

	if ((ret = cls_wifi_dbgfs_register(cls_wifi_hw, "cls_wifi"))) {
		wiphy_err(wiphy, "Failed to register debugfs entries");
		goto err_debugfs;
	}

	cls_wifi_hw->op_mode = CLS_WIFI_AP_MODE;

#if defined(MERAK2000)
	if (!cls_wifi_hw->radio_params->debug_mode) {
#endif
	rtnl_lock();
	wiphy_lock(wiphy);

	/* Add an initial interface */
	wdev = cls_wifi_interface_add(cls_wifi_hw, "wlan%d", NET_NAME_UNKNOWN,
			   cls_wifi_hw->radio_params->custchan ? NL80211_IFTYPE_MONITOR :
			   NL80211_IFTYPE_STATION, NULL);

	wiphy_unlock(wiphy);
	rtnl_unlock();

	if (!wdev) {
		wiphy_err(wiphy, "Failed to instantiate a network device\n");
		ret = -ENOMEM;
		goto err_add_interface;
	}

	wiphy_info(wiphy, "New interface create %s", wdev->netdev->name);
#if defined(MERAK2000)
	}
#endif

	if (cls_wifi_hw->radio_params->debug_mode) {
		if (cls_wifi_cal_init(cls_wifi_hw)) {
			wiphy_err(wiphy, "Failed to initialize calibration function");
			goto err_add_interface;
		}
	} else {
		g_radio_cls_wifi_hw[cls_wifi_hw->radio_idx] = cls_wifi_hw;
		cali_debugfs_init();
	}

#ifdef CONFIG_CLS_VBSS
	clsemi_vbss_init(cls_wifi_hw);
#endif
#ifdef CONFIG_CLS_NAC
	init_nac_sta_head(cls_wifi_hw);
#endif

	if (sigma_enable)
		cls_wifi_init_sigma(cls_wifi_hw);

	//DPD_ONLINE_INTERVAL_ADJUST的值n表示pd按照dif-pd调度的默认周期的n倍执行
	cls_wifi_hw->dpd_timer_interval = DPD_ONLINE_INTERVAL_ADJUST;
	cls_wifi_hw->dpd_timer_enabled = 1;

	cls_wifi_dpd_online_schedule_init(cls_wifi_hw);
	cls_wifi_csa_delay_cali_init(cls_wifi_hw);

	memset(&cls_anti_attack, 0, sizeof(struct cls_wifi_anti_attack));
	wiphy_info(wiphy, "----- init anti attack %d,%d", cls_anti_attack.en_anti_attack,
			cls_anti_attack.internal_attack_to_lastdata);
	cls_wifi_atf_init(cls_wifi_hw);
	cls_wifi_hw->amsdu_enabled = 1;
	cls_wifi_irf_save_work_init(cls_wifi_hw);
	if (cls_wifi_hw->radio_params->debug_mode)
		cls_wifi_cal_save_work_init(cls_wifi_hw);
	cls_wtm_add_phy(cls_wifi_hw);

	timer_setup(&cls_wifi_hw->pps_cal_timer, cls_wifi_pps_cal_timer_cb, 0);
	mutex_lock(&g_cls_wifi_hw_mutex);
	list_add_tail(&cls_wifi_hw->list, &g_cls_wifi_hw_list);
	mutex_unlock(&g_cls_wifi_hw_mutex);

	return 0;

err_add_interface:
	cls_wifi_dbgfs_unregister(cls_wifi_hw);
err_debugfs:
	cls_wifi_dbgfs_free(cls_wifi_hw);
err_register_wiphy:
	if (cls_wifi_hw->wiphy_registered) {
		wiphy_unregister(cls_wifi_hw->wiphy);
		cls_wifi_hw->wiphy_registered = false;
	}
err_lmac_reqs:
	cls_wifi_fw_trace_dump(cls_wifi_hw);
err_platon:
	cls_wifi_platform_off(cls_wifi_hw, NULL);
err_rxbuf:
	cls_wifi_rxbuf_free(cls_wifi_hw);
err_txq:
	cls_wifi_txq_free(cls_wifi_hw);
err_statable:
	cls_wifi_statable_free(cls_wifi_hw);
err_config:
	kmem_cache_destroy(cls_wifi_hw->sw_txhdr_cache);
err_cache:
	kfree(cls_wifi_hw->radio_params);
	if (cls_wifi_hw->wiphy) {
		wiphy_free(cls_wifi_hw->wiphy);
		cls_wifi_hw->wiphy = NULL;
	}
err_out:
	mutex_lock(&g_cls_wifi_hw_mutex);
	list_del(&cls_wifi_hw->list);
	mutex_unlock(&g_cls_wifi_hw_mutex);
	return ret;
}

/**
 *
 */
void cls_wifi_cfg80211_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
#if WIFI_DRV_OPTI_SPIN_LOCK_CFG
    struct sk_buff *skb;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (NULL == cls_wifi_hw)
		return;

	mutex_lock(&g_cls_wifi_hw_mutex);
	list_del(&cls_wifi_hw->list);
	mutex_unlock(&g_cls_wifi_hw_mutex);
	cls_wifi_dpd_online_schedule_deinit(cls_wifi_hw);
	cls_wtm_del_phy(cls_wifi_hw);
	if (cls_wifi_hw->defer_rx.work.func)
		cancel_work_sync(&cls_wifi_hw->defer_rx.work);
#if WIFI_DRV_OPTI_SPIN_LOCK_CFG
	if (cls_wifi_hw->tx_free_skb_work.func)
		cancel_work_sync(&cls_wifi_hw->tx_free_skb_work);
	if (cls_wifi_hw->tx_free_skb_workqueue) {
		destroy_workqueue(cls_wifi_hw->tx_free_skb_workqueue);
		while ((skb = skb_dequeue(&cls_wifi_hw->tx_free_skb_queue)) != NULL)
			consume_skb(skb);
	}
#endif
	if (cls_wifi_hw->snr_workqueue)
		destroy_workqueue(cls_wifi_hw->snr_workqueue);

	if (cls_wifi_hw->dbg_work.func)
		cancel_work_sync(&cls_wifi_hw->dbg_work);
	if (cls_wifi_hw->dbg_cmn_work.func)
		cancel_work_sync(&cls_wifi_hw->dbg_cmn_work);
	if (cls_wifi_hw->csi_work.func)
		cancel_work_sync(&cls_wifi_hw->csi_work);
	cls_wifi_irf_save_work_deinit(cls_wifi_hw);
	if (cls_wifi_hw->radio_params && cls_wifi_hw->radio_params->debug_mode)
		cls_wifi_cal_save_work_deinit(cls_wifi_hw);

	if (cls_wifi_hw->radio_params && cls_wifi_hw->radio_params->debug_mode)
		cls_wifi_cal_deinit(cls_wifi_hw);
	else
		cali_debugfs_deinit();
	cls_wifi_dbgfs_unregister(cls_wifi_hw);
	cls_wifi_dbgfs_free(cls_wifi_hw);
	del_timer_sync(&cls_wifi_hw->txq_cleanup);
	cls_wifi_wdev_unregister(cls_wifi_hw);
	if (cls_wifi_hw->txwq_workqueue) {
		flush_workqueue(cls_wifi_hw->txwq_workqueue);
		destroy_workqueue(cls_wifi_hw->txwq_workqueue);
	}
	if (cls_wifi_hw->wiphy_registered) {
		wiphy_unregister(cls_wifi_hw->wiphy);
		cls_wifi_hw->wiphy_registered = false;
	}
	cls_wifi_radar_detection_deinit(&cls_wifi_hw->radar);
	cls_wifi_platform_off(cls_wifi_hw, NULL);
	cls_wifi_rxbuf_free(cls_wifi_hw);
	cls_wifi_txq_free(cls_wifi_hw);
	cls_wifi_statable_free(cls_wifi_hw);
#ifdef CONFIG_CLS_NAC
	dinit_nac_sta_head(cls_wifi_hw);
#endif
	cls_wifi_irf_deinit(cls_wifi_hw);
	cls_wifi_csa_delay_cali_deinit(cls_wifi_hw);

	kmem_cache_destroy(cls_wifi_hw->sw_txhdr_cache);
	kfree(cls_wifi_hw->radio_params);
	if (cls_wifi_hw->wiphy) {
		wiphy_free(cls_wifi_hw->wiphy);
		cls_wifi_hw->wiphy = NULL;
	}
}

#ifdef CONFIG_CLS_HIGH_TEMP
extern struct cls_tsens_dev *tsens_dev;
void cls_wifi_edca_coefficient_set(u32 coefficient)
{
	struct cls_wifi_hw *hw;

#define EDCA_COEFFICIENT 0xB0
	mutex_lock(&g_cls_wifi_hw_mutex);
	list_for_each_entry(hw, &g_cls_wifi_hw_list, list) {
		cls_wifi_send_wmm_lock(hw, coefficient + EDCA_COEFFICIENT);
	}
	mutex_unlock(&g_cls_wifi_hw_mutex);
}
int cls_wifi_high_temp_work(struct cls_tsens_dev *dev)
{
	struct cls_tsens_dev tsens;
	unsigned long flags;
	char *envp_high[2] = {"EVENT=TEMP_HIGH", NULL};
	char *envp_resume[2] = {"EVENT=TEMP_RESUME", NULL};

	if (!cls_wifi_mod_params.high_temp_op) {
		pr_warn("%s %d, alarma 0x%x alarmb 0x%x, no op.", __func__, __LINE__,
				dev->alarma_flag, dev->alarmb_flag);
		return 0;
	}
	spin_lock_irqsave(&dev->alarm_lock, flags);
	memcpy(&tsens, dev, sizeof(tsens));
	spin_unlock_irqrestore(&dev->alarm_lock, flags);
	if (tsens.alarma_flag & BIT(1))
		kobject_uevent_env(&tsens.dev->kobj, KOBJ_CHANGE, envp_high);
	else if (tsens.alarma_flag & BIT(2))
		cls_wifi_edca_coefficient_set(1);
	else if (tsens.alarmb_flag & BIT(1))
		kobject_uevent_env(&tsens.dev->kobj, KOBJ_CHANGE, envp_resume);
	else if (tsens.alarmb_flag & BIT(2))
		cls_wifi_edca_coefficient_set(0);
	pr_warn("%s %d, alarma 0x%x alarmb 0x%x, op done.", __func__, __LINE__,
			tsens.alarma_flag, tsens.alarmb_flag);
	return 0;
}
#endif

/**
 *
 */
static int __init cls_wifi_mod_init(void)
{
#ifdef CONFIG_CLS_HIGH_TEMP
	struct cls_tsens_dev **tsens;

	tsens = symbol_get(tsens_dev);
	if (tsens) {
		(*tsens)->handler_hooks = cls_wifi_high_temp_work;
		(*tsens)->alarm_work_enable = true;
		symbol_put(tsens_dev);
		pr_warn("%s %d, tsens work registered.\n", __func__, __LINE__);
	}
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	cls_wifi_print_version();
	cls_wtm_init_debugfs();
	INIT_LIST_HEAD(&g_cls_wifi_hw_list);
	mutex_init(&g_cls_wifi_hw_mutex);
	return 0;
}

/**
 *
 */
static void __exit cls_wifi_mod_exit(void)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	cls_wtm_deinit_debugfs();
}

module_init(cls_wifi_mod_init);
module_exit(cls_wifi_mod_exit);

MODULE_DESCRIPTION(CLS_WIFI_DRV_DESCRIPTION);
MODULE_VERSION("1.0.0");
MODULE_AUTHOR(CLS_WIFI_DRV_COPYRIGHT);
MODULE_LICENSE("GPL");
