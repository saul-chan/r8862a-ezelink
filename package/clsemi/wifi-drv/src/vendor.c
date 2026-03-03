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
#include <net/netlink.h>
#include <net/cfg80211.h>
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_main.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_tx.h"
#include "lmac_mac.h"
#include "vendor.h"
#include "vbss.h"
#include "clsm_wifi_dfx.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_csi.h"
#include "nac.h"
#include "lmac_msg.h"

#define CLS_VIP_QOS
#ifdef CLS_VIP_QOS

#define CLS_VIP_QOS_DBG

static DEFINE_SPINLOCK(vip_qos_spinlock);
DEFINE_HASHTABLE(mac_hash, 10); // 2^second argument = max slot
#define MAX_VIP_QOS_MAC		64	//max must < 2^10 = 1024
static s32 cls_vip_qos_count;

static inline u32 ether_addr_hash(const u8 *addr)
{
	u32 hash = 0;
	u32 i;

	for (i = 0; i < ETH_ALEN; ++i)
		hash ^= (u32)addr[i] << ((i & 3) << 3);

	return hash;
}

struct vip_qos_entry {
	struct hlist_node node;
	u8 mac[6];
	// other data fields
};

static s32 cls_add_vip_qos_mac(u8 *mac_addr)
{
	struct vip_qos_entry *entry;
	struct vip_qos_entry *existing_entry;

	if (cls_vip_qos_count >= MAX_VIP_QOS_MAC) {
		pr_info("[full]VIP QOS Mac List is full for %pM\n", mac_addr);
		return -ENOSPC;
	}

	spin_lock_bh(&vip_qos_spinlock);
	// Check if entry with the same MAC address already exists
	hash_for_each_possible(mac_hash, existing_entry, node, ether_addr_hash(mac_addr)) {
		if (memcmp(existing_entry->mac, mac_addr, ETH_ALEN) == 0) {
			pr_info("[duplicate]MAC address:%pM already exists in VIP QOS Mac List\n", mac_addr);
			spin_unlock_bh(&vip_qos_spinlock);
			return -EEXIST; // Return an error code indicating duplicate entry
		}
	}
	spin_unlock_bh(&vip_qos_spinlock);

	// Create an entry
	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	memcpy(entry->mac, mac_addr, ETH_ALEN);

	spin_lock_bh(&vip_qos_spinlock);
	/*TODO
	 *call cls_dummy_add_vip_qos_mac_func
	 */

	hash_add(mac_hash, &entry->node, ether_addr_hash(mac_addr));
	cls_vip_qos_count++;
	spin_unlock_bh(&vip_qos_spinlock);

	pr_info("[add]vip qos mac %pM to hash table\n", mac_addr);
	return 0;
}

static s32 cls_del_vip_qos_mac(u8 *mac_addr)
{
	struct hlist_node *next;
	struct vip_qos_entry *existing_entry;

	if (cls_vip_qos_count <= 0) {
		pr_info("[empty]VIP QOS Mac List is empty for %pM cls_vip_qos_count=%d\n",
			   mac_addr, cls_vip_qos_count);
		return -EPERM;
	}

	spin_lock_bh(&vip_qos_spinlock);
	hash_for_each_possible_safe(mac_hash, existing_entry, next, node, ether_addr_hash(mac_addr)) {
		if (memcmp(existing_entry->mac, mac_addr, ETH_ALEN) == 0) {
			hash_del(&existing_entry->node);
			kfree(existing_entry);
			cls_vip_qos_count--;
			/*TODO TODO
			 *call cls_dummy_del_vip_qos_mac_func
			 */
			spin_unlock_bh(&vip_qos_spinlock);
			pr_info("[delete]MAC address %pM from VIP QOS Mac List\n", mac_addr);
			return 0;
		}
	}
	spin_unlock_bh(&vip_qos_spinlock);

	pr_info("[error] MAC address:%pM not found in VIP QOS Mac List\n", mac_addr);
	return -ENOENT;
}

static s32 cls_clear_vip_qos_mac(void)
{
	struct hlist_node *next;
	struct vip_qos_entry *existing_entry;
	int total_qos_mac = cls_vip_qos_count;
	int i;

	if (cls_vip_qos_count <= 0) {
		pr_info("[empty]VIP QOS Mac List is empty already\n");
		return -EPERM;
	}

	spin_lock_bh(&vip_qos_spinlock);
	hash_for_each_safe(mac_hash, i, next, existing_entry, node) {
		pr_info("[delete]MAC address %pM from VIP QOS Mac List\n", existing_entry->mac);
		hash_del(&existing_entry->node);
		kfree(existing_entry);
		cls_vip_qos_count--;
		/*TODO TODO
		 *call cls_dummy_clear_vip_qos_mac_func
		 */
	}
	spin_unlock_bh(&vip_qos_spinlock);

	pr_info("[clear] VIP QOS Mac List total %d mac is cleared, remain %d\n", total_qos_mac, cls_vip_qos_count);
	return 0;
}

static s32 clsemi_vndr_cls_qos_add_vip_mac(struct wiphy *wiphy,
                                            struct wireless_dev *wdev,
                                            const void *data, s32 data_len)
{
	const u8 *pdata = (u8 *)data;
	u8 mac_addr[ETH_ALEN] = {};

#ifdef CLS_VIP_QOS_DBG
	print_hex_dump(KERN_INFO, "raw data: ", DUMP_PREFIX_ADDRESS,
			16, 1, data, data_len, true);
	printk(KERN_INFO "mac_add data_len=%d\n", data_len);
#endif

	if (data_len != ETH_ALEN)
		return 0;

	memcpy(mac_addr, pdata, ETH_ALEN);

	cls_add_vip_qos_mac(mac_addr);

	return 0;
}

static int clsemi_vndr_cls_qos_del_vip_mac(struct wiphy *wiphy,
                                            struct wireless_dev *wdev,
                                            const void *data, s32 data_len)
{
	const u8 *pdata = (u8 *)data;
	u8 mac_addr[ETH_ALEN] = {};

#ifdef CLS_VIP_QOS_DBG
	print_hex_dump(KERN_INFO, "raw data: ", DUMP_PREFIX_ADDRESS,
			16, 1, data, data_len, true);
	printk(KERN_INFO "mac_del data_len=%d\n", data_len);
#endif

	if (data_len != ETH_ALEN)
		return 0;

	memcpy(mac_addr, pdata, ETH_ALEN);

	cls_del_vip_qos_mac(mac_addr);

	return 0;
}

static int clsemi_vndr_cls_qos_clear_vip_mac(struct wiphy *wiphy,
                                            struct wireless_dev *wdev,
                                            const void *data, s32 data_len)
{
#ifdef CLS_VIP_QOS_DBG
	print_hex_dump(KERN_INFO, "raw data: ", DUMP_PREFIX_ADDRESS,
			16, 1, data, data_len, true);
	printk(KERN_INFO "mac_clear data_len=%d\n", data_len);
#endif

	cls_clear_vip_qos_mac();

	return 0;
}
#endif


#define LEAST_SIZE 2
#define LEAST_QOS_SIZE 4
extern struct cls_wifi_anti_attack cls_anti_attack;

static const int mcs_map_to_rate[4][3] = {
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_7] = 65,
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_8] = 78,
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_9] = 78,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_7] = 135,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_8] = 162,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_9] = 180,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_7] = 292,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_8] = 351,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_9] = 390,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_7] = 585,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_8] = 702,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_9] = 780,
};

static const int cls_wifi_ac2hwq[1][NL80211_NUM_ACS] = {
	{
		[NL80211_TXQ_Q_VO] = CLS_WIFI_HWQ_VO,
		[NL80211_TXQ_Q_VI] = CLS_WIFI_HWQ_VI,
		[NL80211_TXQ_Q_BE] = CLS_WIFI_HWQ_BE,
		[NL80211_TXQ_Q_BK] = CLS_WIFI_HWQ_BK
	}
};

#define MAX_VHT_RATE(map, nss, bw) (mcs_map_to_rate[bw][map] * (nss))

int clsemi_enable_mu_tx(struct wiphy *wiphy, u8 enable)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);

	if (cls_wifi_hw->radio_params->mutx == enable)
		return 0;

	cls_wifi_hw->radio_params->mutx = enable;
	/* TODO: add more when MU Tx is verified */

	return 0;
}

int clsemi_enable_mu_rx(struct wiphy *wiphy, u8 enable)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);

	if (cls_wifi_hw->radio_params->murx == enable)
		return 0;

	cls_wifi_hw->radio_params->murx = enable;
	/* TODO: add more when MU Rx is verified */

	return 0;
}

static int clsemi_set_ldpc(struct wiphy *wiphy, struct wireless_dev *wdev, u8 ldpc)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct ieee80211_supported_band *sband = NULL;
	bool ldpc_on = (ldpc == 0x01) ? true:false;

	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
		sband = cls_wifi_hw->wiphy->bands[NL80211_BAND_5GHZ];
	else if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
		sband = cls_wifi_hw->wiphy->bands[NL80211_BAND_2GHZ];

	if (!sband)
		return -EINVAL;

	if (cls_wifi_hw->radio_params->ldpc_on != ldpc_on) {
		cls_wifi_hw->radio_params->ldpc_on = ldpc_on;
		if (cls_wifi_hw->radio_params->ldpc_on) {
			if (cls_wifi_hw->radio_params->ht_on) {
				sband->ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;
			} else if (cls_wifi_hw->radio_params->vht_on && (cls_wifi_hw->band_cap &
					CLS_WIFI_BAND_CAP_5G)) {
				sband->vht_cap.cap |= IEEE80211_VHT_CAP_RXLDPC;
			} if (cls_wifi_hw->radio_params->he_on) {
				int i = 0;
				struct ieee80211_sta_he_cap *he_cap;

				for (; i < sband->n_iftype_data; i++) {
					he_cap = (struct ieee80211_sta_he_cap *)&sband->
							iftype_data[i].he_cap;
					he_cap->he_cap_elem.phy_cap_info[1] |=
						IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
				}
			}
		} else {
			if (cls_wifi_hw->radio_params->ht_on) {
				sband->ht_cap.cap &= ~IEEE80211_HT_CAP_LDPC_CODING;
			} else if (cls_wifi_hw->radio_params->vht_on && (cls_wifi_hw->band_cap &
					CLS_WIFI_BAND_CAP_5G)) {
				sband->vht_cap.cap &= ~IEEE80211_VHT_CAP_RXLDPC;
			} if (cls_wifi_hw->radio_params->he_on) {
				int i = 0;
				struct ieee80211_sta_he_cap *he_cap;

				for (; i < sband->n_iftype_data; i++) {
					he_cap = (struct ieee80211_sta_he_cap *)&sband->
							iftype_data[i].he_cap;
					he_cap->he_cap_elem.phy_cap_info[1] &=
						~IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
					cls_wifi_hw->radio_params->he_mcs_map =
						min_t(int, cls_wifi_hw->radio_params->he_mcs_map,
							IEEE80211_HE_MCS_SUPPORT_0_9);
				}
			}
		}
	}
	return 0;
}

int clsemi_set_beamformer(struct wiphy *wiphy,
						 struct wireless_dev *wdev, u8 beamformer)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	int nss = cls_wifi_hw->radio_params->nss;
	struct ieee80211_supported_band *sband = NULL;
	struct ieee80211_sta_he_cap *he_cap;
	bool bfmer_on = (beamformer == 0x01) ? true:false;

	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
		sband = cls_wifi_hw->wiphy->bands[NL80211_BAND_5GHZ];
	else if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
		sband = cls_wifi_hw->wiphy->bands[NL80211_BAND_2GHZ];

	if (!sband)
		return -EINVAL;

	if (cls_wifi_hw->radio_params->bfmer != bfmer_on) {
		cls_wifi_hw->radio_params->bfmer = bfmer_on;
		he_cap = (struct ieee80211_sta_he_cap *)&sband->iftype_data[0].he_cap;
		if (cls_wifi_hw->radio_params->bfmer) {
			if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G) {
				sband->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
				sband->vht_cap.cap |=
						(nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
			}

			he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_SU_BEAMFORMER;
			he_cap->he_cap_elem.phy_cap_info[5] |=
					IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_2;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[5] |=
					IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_ABOVE_80MHZ_2;
			he_cap->he_cap_elem.phy_cap_info[4] |=
					IEEE80211_HE_PHY_CAP4_MU_BEAMFORMER;
		} else {
			if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G) {
				sband->vht_cap.cap &= ~IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
				sband->vht_cap.cap &=
					~((nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT);
			}

			he_cap->he_cap_elem.phy_cap_info[3] &=
					~IEEE80211_HE_PHY_CAP3_SU_BEAMFORMER;
			he_cap->he_cap_elem.phy_cap_info[5] &=
					~IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_2;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[5] &=
					~IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_ABOVE_80MHZ_2;
			he_cap->he_cap_elem.phy_cap_info[4] &= ~IEEE80211_HE_PHY_CAP4_MU_BEAMFORMER;
		}
	}

	return 0;
}

static int clsemi_set_beamformee(struct wiphy *wiphy,
						 struct wireless_dev *wdev, int beamformee)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct ieee80211_supported_band *sband = NULL;
	struct ieee80211_sta_he_cap *he_cap;
	bool bfmee_on = (beamformee == 0x01) ? true:false;

	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
		sband = cls_wifi_hw->wiphy->bands[NL80211_BAND_5GHZ];
	else if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
		sband = cls_wifi_hw->wiphy->bands[NL80211_BAND_2GHZ];

	if (!sband)
		return -EINVAL;

	if (cls_wifi_hw->radio_params->bfmee != bfmee_on) {
		cls_wifi_hw->radio_params->bfmee = bfmee_on;

		he_cap = (struct ieee80211_sta_he_cap *)&sband->iftype_data[0].he_cap;
		if (cls_wifi_hw->radio_params->bfmee) {
			if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G) {
				sband->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
				sband->vht_cap.cap |= 3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
			}

			he_cap->he_cap_elem.phy_cap_info[4] |=
					IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;
			he_cap->he_cap_elem.phy_cap_info[4] |=
					IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[4] |=
					IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_ABOVE_80MHZ_4;
		} else {
			if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G) {
				sband->vht_cap.cap &= ~IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
				sband->vht_cap.cap &= ~(3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT);
			}

			he_cap->he_cap_elem.phy_cap_info[4] &=
					~IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;
			he_cap->he_cap_elem.phy_cap_info[4] &=
					~IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[4] &=
					~IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_ABOVE_80MHZ_4;
		}
	}

	return 0;
}

static int clsemi_set_11nsgi20(struct wiphy *wiphy,
						 struct wireless_dev *wdev, int enabled)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	bool sgi = cls_wifi_hw->radio_params->sgi;
	struct ieee80211_supported_band *band_to_set;
	bool enable = enabled ? true : false;

	if (cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
		band_to_set = wiphy->bands[NL80211_BAND_2GHZ];
	else
		band_to_set = wiphy->bands[NL80211_BAND_5GHZ];

	if (!band_to_set)
		return -EINVAL;

	if (sgi) {
		if (enable)
			band_to_set->ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;
		else
			band_to_set->ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_20;
	}
	return 0;
}

static int clsemi_set_vhtsgi80(struct wiphy *wiphy,
						 struct wireless_dev *wdev, int enabled)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);

	bool sgi = cls_wifi_hw->radio_params->sgi;
	struct ieee80211_supported_band *band_5GHz = wiphy->bands[NL80211_BAND_5GHZ];
	bool enable = enabled ? true : false;

	if (sgi) {
		if (enable)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
		else
			band_5GHz->vht_cap.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_80;
	}
	return 0;
}

static int clsemi_set_spatical_rxtx_stream(struct wiphy *wiphy,
					struct wireless_dev *wdev, int rxtx_nss)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	int nss = cls_wifi_hw->radio_params->nss;
	struct ieee80211_supported_band *band_5GHz = wiphy->bands[NL80211_BAND_5GHZ];
	struct ieee80211_supported_band *band_2GHz = wiphy->bands[NL80211_BAND_2GHZ];
	int i = 0;
	int mcs_map;
	int bw_max;

	if (nss != rxtx_nss) {
		cls_wifi_hw->radio_params->nss = rxtx_nss;
		nss = cls_wifi_hw->radio_params->nss;
		//set ht capability
		if (cls_wifi_hw->radio_params->ht_on) {
			if (cls_wifi_hw->radio_params->use_2040) {
				band_2GHz->ht_cap.mcs.rx_mask[4] = 0x1; /* MCS32 */
				band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
				band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(135 * nss);
			} else {
				band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(65 * nss);
		}
			if (nss > 1)
				band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;
			if (cls_wifi_hw->radio_params->sgi) {
				band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;
				if (cls_wifi_hw->radio_params->use_2040) {
					band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;
					band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(150 * nss);
				} else
					band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(72 * nss);
			}
			for (i = 0; i < nss; i++)
				band_2GHz->ht_cap.mcs.rx_mask[i] = 0xFF;
			for (i = nss; i < IEEE80211_HT_MCS_MASK_LEN; i++)
				band_2GHz->ht_cap.mcs.rx_mask[i] = 0x00;
			band_5GHz->ht_cap = band_2GHz->ht_cap;
		}
		//set vht capability
		if (cls_wifi_hw->radio_params->vht_on) {
			if (nss > 1)
				band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_TXSTBC;
			else
				band_5GHz->vht_cap.cap &= ~IEEE80211_VHT_CAP_TXSTBC;

			if (cls_wifi_hw->radio_params->bfmer) {
				/* Set number of sounding dimensions */
				band_5GHz->vht_cap.cap |=
					(nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
			}
			// Get max supported BW
			if (cls_wifi_hw->radio_params->use_160)
				bw_max = PHY_CHNL_BW_160;
			else if (cls_wifi_hw->radio_params->use_80)
				bw_max = PHY_CHNL_BW_80;
			else if (cls_wifi_hw->radio_params->use_2040)
				bw_max = PHY_CHNL_BW_40;
			else
				bw_max = PHY_CHNL_BW_20;

			// 1 SS
			if (bw_max == PHY_CHNL_BW_20)
				// VHT spec doesn't allow MCS9 at 20MHz/1SS
				mcs_map = min_t(int, cls_wifi_hw->radio_params->mcs_map,
						IEEE80211_VHT_MCS_SUPPORT_0_8);
			else
				mcs_map = min_t(int, cls_wifi_hw->radio_params->mcs_map,
						IEEE80211_VHT_MCS_SUPPORT_0_9);
			band_5GHz->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(mcs_map);
			band_5GHz->vht_cap.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			band_5GHz->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(mcs_map);
			band_5GHz->vht_cap.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);

			if (nss > 1) {
				// 2 SS
				// Don't reset mcs_map as limitation on 20MHz is also valid for 2SS.
				// Moreover, For FPGA testing limit MCS for bw >= 80MHz:
				//   - in RX, 2SS, we support up to MCS7
				//   - in TX, 2SS, we support up to MCS8
				if (bw_max >= PHY_CHNL_BW_80)
					mcs_map = min_t(int, mcs_map, IEEE80211_VHT_MCS_SUPPORT_0_8);
				band_5GHz->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << 2);
				band_5GHz->vht_cap.vht_mcs.tx_highest =
							MAX_VHT_RATE(mcs_map, nss, bw_max);
				if (bw_max >= PHY_CHNL_BW_80)
					mcs_map = min_t(int, mcs_map, IEEE80211_VHT_MCS_SUPPORT_0_7);
				band_5GHz->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << 2);
				band_5GHz->vht_cap.vht_mcs.rx_highest =
							MAX_VHT_RATE(mcs_map, nss, bw_max);
			}

			if (nss > 2) {
				// > 2SS, no more limitation (neither in spec at 20MHz nor in FPGA)
				mcs_map = min_t(int, cls_wifi_hw->radio_params->mcs_map,
						IEEE80211_VHT_MCS_SUPPORT_0_9);
				band_5GHz->vht_cap.vht_mcs.tx_highest =
						MAX_VHT_RATE(mcs_map, nss, bw_max);
				band_5GHz->vht_cap.vht_mcs.rx_highest =
						MAX_VHT_RATE(mcs_map, nss, bw_max);
				for (i = 2; i < nss; i++) {
					band_5GHz->vht_cap.vht_mcs.tx_mcs_map |=
						cpu_to_le16(mcs_map << (i*2));
					band_5GHz->vht_cap.vht_mcs.rx_mcs_map |=
						cpu_to_le16(mcs_map << (i*2));
				}
			}

			// Unsupported SS
			for (i = nss; i < 8; i++) {
				band_5GHz->vht_cap.vht_mcs.tx_mcs_map |=
					cpu_to_le16(IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
				band_5GHz->vht_cap.vht_mcs.rx_mcs_map |=
					cpu_to_le16(IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
			}
		}

	}
	return 0;
}


static int clsemi_set_amsdu(struct wiphy *wiphy,
					struct wireless_dev *wdev, int enable)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);

	if (cls_wifi_hw) {
		if (cls_wifi_hw->amsdu_enabled != enable)
			cls_wifi_hw->amsdu_enabled = enable;
	}

	return 0;
}
int cls_set_fixed_rate_for_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)
{
	u32 rate_config = 0;
	int i = 0;
	int j = 0;
	int error = 0;

	if (!sta || !sta->valid || !sta->he)
		return 0;
	if (cls_wifi_hw->ppdu_tx_type < vendor_ofdma_ppdu_tx_max) {
		switch (cls_wifi_hw->ppdu_tx_type) {
		case 0x00:
			RC_RATE_SET(FORMAT_MOD, rate_config, FORMATMOD_HE_SU);
			break;
		case 0x01:
			RC_RATE_SET(FORMAT_MOD, rate_config, FORMATMOD_HE_MU);
			break;
		case 0x02:
			RC_RATE_SET(FORMAT_MOD, rate_config, FORMATMOD_HE_SU);
			break;
		case 0x03:
			RC_RATE_SET(FORMAT_MOD, rate_config, FORMATMOD_HE_TB);
			break;
		case 0x04:
			RC_RATE_SET(FORMAT_MOD, rate_config, FORMATMOD_NON_HT);
			break;
		default:
			;
		}
	}

	RC_RATE_SET(GI, rate_config, cls_wifi_hw->control.he_gi);

	for (i = 0; i < NL80211_HE_NSS_MAX; i++) {
		j = 0;
		if (cls_wifi_hw->control.he_mcs[i]) {
			for (; j < 16; j++) {
				if (cls_wifi_hw->control.he_mcs[i] & (1 << j)) {
					RC_RATE_SET(NSS, rate_config, i);
					RC_RATE_SET(MCS, rate_config, j);
					break;
				}
			}
		}
	}
	if (i == NL80211_HE_NSS_MAX) {
		RC_RATE_SET(NSS, rate_config, 1);
		RC_RATE_SET(MCS, rate_config, 7);
	}


	if (cls_wifi_hw->txbw < 0x04)
		RC_RATE_SET(BW, rate_config, cls_wifi_hw->txbw);
	else
		RC_RATE_SET(BW, rate_config, 2);

	if (cls_wifi_hw->ppdu_tx_type >= 0x01)
		RC_RATE_SET(RU_SIZE, rate_config, 1);

	error = cls_wifi_send_mm_rc_set_rate(cls_wifi_hw, sta->sta_idx, rate_config, true);
	if (error)
		return error;

	return 0;
}

static u8 g_skip_dfs_cac = 0;
int cls_wifi_set_skip_dfs_cac(struct wiphy *wiphy, u8 skip_dfs_cac)
{
	if (skip_dfs_cac == 1)
		g_skip_dfs_cac = 1;
	else
		g_skip_dfs_cac = 0;

	return 0;
}

u8 cls_wifi_get_skip_dfs_cac(struct wiphy *wiphy)
{
	return g_skip_dfs_cac;
}

static int clsemi_vndr_cmds_set_skip_dfs_cac(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *wifi_hw = NULL;
	int ret = 0;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid CLS_NL80211_ATTR_SET_SKIP_DFS_CAC\n", __func__);
		return rc;
	}

	if (tb[CLS_NL80211_ATTR_SET_SKIP_DFS_CAC]) {
		u8 skip_dfs_cac = nla_get_u8(tb[CLS_NL80211_ATTR_SET_SKIP_DFS_CAC]);

		cls_wifi_set_skip_dfs_cac(wiphy, skip_dfs_cac);
	} else
		ret = -1;

	return ret;
}

static int clsemi_vndr_cmds_cancel_dfs_cac(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	int rc;
	int ret = 0;
	struct cls_wifi_hw *wifi_hw = NULL;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid CLS_NL80211_ATTR_CANCEL_DFS_CAC\n", __func__);
		return rc;
	}

	if (tb[CLS_NL80211_ATTR_CANCEL_DFS_CAC]) {
		u8 cancel_dfs_cac = nla_get_u8(tb[CLS_NL80211_ATTR_CANCEL_DFS_CAC]);

		if (cancel_dfs_cac == 1)
			cls_wifi_radar_cancel_cac(&wifi_hw->radar);
	} else
		ret = -1;

	return ret;
}

static int clsemi_cfg80211_vndr_cmds_ofdma_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	int i = 0;
	int j = 0;
	u8 type = 0x00;
	u8 *pdata = (u8 *)data;
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	static u8 ofdma_type = 0xff;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_vif *vif = NULL;
	u8 val;

	if (len < LEAST_SIZE)
		return 0;

	cls_wifi_hw = wiphy_priv(wiphy);
	if (!cls_wifi_hw)
		return 0;
	while (i <= len - 2) {
		type = *(pdata + i);
		val = *(pdata + i + 1);

		pr_warn("%s: type=[%d], val=[%d]\n", __func__, type, val);

		switch (type) {
		case VENDOR_OFDMA_ACK_TYPE:
			cls_wifi_hw->dl_params.ack_type = val;
			cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			break;
		case VENDOR_OFDMA_DISTRIGGER_TYPE:
			if (val < vendor_ofdma_trigger_max) {
				//send ipc message to fw
//				cls_wifi_hw->ul_params.distriggertype = val;
				cls_wifi_send_mm_ul_parameters(cls_wifi_hw);
				//cls_wifi_send_disabletriggertype_config(cls_wifi_hw, val);
			}
			break;
		case VENDOR_OFDMA_PPPDUTX_TYPE:
			if (val < vendor_ofdma_ppdu_tx_max) {
				cls_wifi_hw->ppdu_tx_type = val;

				for (j = 0; j < hw_remote_sta_max(cls_wifi_hw); j++) {
					sta = &cls_wifi_hw->sta_table[j];
					cls_set_fixed_rate_for_sta(cls_wifi_hw, sta);
				}
			}
			break;
		case VENDOR_OFDMA_TRIGGER_TYPE:
			if (val < vendor_ofdma_trigger_max) {
				vif = container_of(wdev, struct cls_wifi_vif, wdev);
				cls_wifi_hw->ul_params.user_num = vif->assoc_sta_count;
				cls_wifi_hw->ul_params.trigger_type = val;
				cls_wifi_send_mm_ul_parameters(cls_wifi_hw);
				//cls_wifi_send_triggertype_config(cls_wifi_hw, val);
			}
			break;
		case VENDOR_OFDMA_TRIGGER_TXBF:
			cls_wifi_hw->dl_params.trigger_txbf = val;
			cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			//option case
			break;
		case VENDOR_OFDMA_NUM_USERS:
			//send num to fw
			if (ofdma_type == 0x00) {
				cls_wifi_hw->dl_params.user_num = val;
				cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			} else if (ofdma_type == 0x01) {
				cls_wifi_hw->ul_params.user_num = val;
				cls_wifi_send_mm_ul_parameters(cls_wifi_hw);
			} else if (ofdma_type == 0x02) {
				cls_wifi_hw->dl_params.user_num = val;
				cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			}
			ofdma_type = 0xff;
			break;
		case VENDOR_OFDMA_TYPE:
			if (val < vendor_ofdma_type_max) {
				ofdma_type = val;
				if (val == 0x00) {
					cls_wifi_hw->dl_params.dl_on = true;
					cls_wifi_hw->dl_params.dl_20and80 = false;
					cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
					//cls_wifi_send_ofdmatype_config(cls_wifi_hw, val);
				} else if (val == 0x01) {
					cls_wifi_hw->ul_params.ul_on = true;
					cls_wifi_hw->ul_params.user_num = 128;
					cls_wifi_hw->ul_params.ul_bw = irf_get_curr_bw(cls_wifi_hw);
					cls_wifi_send_mm_ul_parameters(cls_wifi_hw);
				} else if (val == 0x02) {
					cls_wifi_hw->dl_params.dl_on = true;
					cls_wifi_hw->dl_params.dl_20and80 = true;
					cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
				}
			} else {
				cls_wifi_hw->dl_params.dl_on = false;
				cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
				cls_wifi_hw->ul_params.ul_on = false;
				cls_wifi_send_mm_ul_parameters(cls_wifi_hw);
			}
			break;
		case VENDOR_OFDMA_DL_GI:
			if (val < VENDOR_GI_VALUE_MAX) {
				cls_wifi_hw->dl_params.gi = val;
				cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			}
			break;
		case VENDOR_OFDMA_DL_LTF:
			if (val < VENDOR_LTF_VALUE_MAX) {
				cls_wifi_hw->dl_params.ltf_type = val;
				cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			}
			break;
		case VENDOR_OFDMA_DL_NSS:
			cls_wifi_hw->dl_params.nss_max = val;
			cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			break;
		case VENDOR_OFDMA_DL_MCS:
			cls_wifi_hw->dl_params.mcs = val;
			cls_wifi_send_mm_dl_parameters(cls_wifi_hw);
			break;
		default:
			;
		}
		i += 2;
	}
	return 0;
}

int clsemi_set_txq_params(struct wiphy *wiphy, struct net_device *dev,
					struct ieee80211_txq_params *params)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	u8 hw_queue, aifs, cwmin, cwmax;
	u32 param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	hw_queue = cls_wifi_ac2hwq[0][params->ac];

	aifs = params->aifs;
	cwmin = fls(params->cwmin);
	cwmax = fls(params->cwmax);

	/* Store queue information in general structure */
	param = (u32) (aifs << 0);
	param |= (u32) (cwmin << 4);
	param |= (u32) (cwmax << 8);
	param |= (u32) (params->txop) << 12;

	/* Send the MM_SET_EDCA_REQ message to the FW */
	return cls_wifi_send_set_edca(cls_wifi_hw, hw_queue, param, false, cls_wifi_vif->vif_index);
}

static int clsemi_cfg80211_vndr_cmds_nac_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	int cmd =0;
	u8 *pdata = (u8 *)data;
	u8 mac[6];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct nac_monitor_sta sta_info;
	u32 enabled = 0;

	cmd = pdata[0];
	memcpy(mac, pdata + 1, 6);

	cls_wifi_hw = wiphy_priv(wiphy);
	if (!cls_wifi_hw)
		return 0;

	if (cmd == 1)
		add_nac_sta(cls_wifi_hw, mac);
	else if (cmd == 2)
		del_nac_sta(cls_wifi_hw, mac);
	else if (cmd == 3) {
		memset(&sta_info, 0, sizeof(sta_info));
		memcpy(sta_info.mac_addr, pdata + 1, 6);
		get_nac_sta(cls_wifi_hw, &sta_info);
	} else if (cmd == 4) {
		enabled = pdata[1];
		set_nac_enabled(cls_wifi_hw, enabled);
	}


	return 0;
}

#if CONFIG_CLS_SMTANT
static int clsemi_cfg80211_vndr_cmds_smtant_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	u8 *pdata = (u8 *)data;
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct mm_smart_antenna_req req;
	u8 enable = 0;
	u8 update_mode = 0;
	u8 gpio_mode  = 0;

	cls_wifi_hw = wiphy_priv(wiphy);
	if (!cls_wifi_hw)
		return 0;

	enable = pdata[0];

	update_mode = pdata[1];

	gpio_mode = pdata[2];

	if (gpio_mode > 3 || enable > 1 || update_mode > 1) {
		pr_err("enable, update_mode or gpio mode erro!\n");
		return 0;
	}

	req.enable = gpio_mode << 2 | update_mode << 1 |enable << 0;
	req.curval_lo = *((uint32_t *)(pdata + 3));
	req.curval_hi = *((uint32_t *)(pdata + 7));
	req.rstval_lo = *((uint32_t *)(pdata + 11));
	req.rstval_hi = *((uint32_t *)(pdata + 15));

	pr_err("%s : req.enable = 0x%x\n", __func__, req.enable);

	pr_err("%s : req.cur val lo = 0x%08x, hi =0x%0x\n", __func__, req.curval_lo, req.curval_hi);

	pr_err("%s : req.rstv val lo = 0x%08x, hi =0x%0x\n", __func__, req.rstval_lo, req.rstval_hi);

	return cls_wifi_send_mm_set_smant(cls_wifi_hw, &req);
}

static int clsemi_vndr_cmds_set_smtant_cfg(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct mm_smart_antenna_req smtant_info;
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid nac ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_SMTANT_CFG]) {
		pr_warn("%s Invalid nac sta info ATTR\n", __func__);

		return -EINVAL;
	}

	/* Get smart antenna info*/
	memset(&smtant_info, 0, sizeof(smtant_info));

	memcpy(&smtant_info, nla_data(tb[CLS_NL80211_ATTR_SMTANT_CFG]), sizeof(smtant_info));

	return cls_wifi_send_mm_set_smant(cls_wifi_hw, &smtant_info);

}
#endif

static int clsemi_cfg80211_vndr_cmds_qos_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	int qos_type;
	u8 val = 0x00;
	u8 type = 0x00;
	u8 *pdata = (u8 *)data;
	struct ieee80211_txq_params params;

	if (len < LEAST_QOS_SIZE) {
		pr_err("invalid qos command\n");
		return 0;
	}

	val = *pdata;
	qos_type = val;

	params.ac = NL80211_NUM_ACS;
	val = *(pdata + 1);
	switch (val) {
	case 0x00:
		params.ac = NL80211_AC_VO;
		break;
	case 0x01:
		params.ac = NL80211_AC_BK;
		break;
	case 0x02:
		params.ac = NL80211_AC_VI;
		break;
	case 0x03:
		params.ac = NL80211_AC_VO;
		break;
	default:
		;
	}

	type = *(pdata + 2);
	switch (type) {
	case 0x00:
		params.aifs = *(u16 *)(pdata + 3) & 0xff;
		break;
	case 0x01:
		params.cwmax = *(u16 *)(pdata + 3);
		break;
	case 0x02:
		params.cwmin = *(u16 *)(pdata + 3);
		break;
	case 0x03:
		params.txop = *(u16 *)(pdata + 3);
		break;
	case 0x04:
		break;
	default:
		;
	}

	return clsemi_set_txq_params(wiphy, wdev->netdev, &params);
}


static int clsemi_cfg80211_vndr_cmds_generic_handler(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	int i = 0;
	u8 val;
	int j = 0;
	u8 type = 0x00;
	u8 *pdata = (u8 *)data;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *cls_wifi_hw = NULL;

	if (len < LEAST_SIZE)
		return 0;

	cls_wifi_hw = wiphy_priv(wiphy);
	while (i <= len - 2) {
		type = *(pdata + i);
		val = *(pdata + i + 1);
		pr_warn("%s: type=[%d], val=[%d]\n", __func__, type, val);
		switch (type) {
		case VENDOR_GENERIC_TXBW_TYPE:
			if (val < 0x05) {
				cls_wifi_hw->txbw = val;

			for (j = 0; j < hw_remote_sta_max(cls_wifi_hw); j++) {
				sta = &cls_wifi_hw->sta_table[j];
				cls_set_fixed_rate_for_sta(cls_wifi_hw, sta);
			}
		}
			break;
		case VENDOR_GENERIC_LDPC_TYPE:
			clsemi_set_ldpc(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_BEAMFORMER:
			clsemi_set_beamformer(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_BEAMFORMEE:
			clsemi_set_beamformee(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_11NSGI20:
			clsemi_set_11nsgi20(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_SPATIAL_RX_STREAM:
			clsemi_set_spatical_rxtx_stream(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_SPATIAL_TX_STREAM:
			clsemi_set_spatical_rxtx_stream(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_AMSDU_CONFIG:
			clsemi_set_amsdu(wiphy, wdev, val);
			break;
		case VENDOR_GENERIC_AMPDU_CONFIG:
			//the value in case is the same as default config
			break;
		case VENDOR_GENERIC_ADDBA_REJECT:
			//the value in case is the same as default config
			break;
		case VENDOR_GENERIC_SECBAND_OFFSET:
			// the function can be done in application
			break;
		case VENDOR_GENERIC_BW_SGNL:
			//the case is optional
			break;
		case VENDOR_GENERIC_VHTSGI80:
			clsemi_set_vhtsgi80(wiphy, wdev, val);
			break;
		default:
			;
		}
		i += 2;
	}
	return 0;

}

int cls_wifi_init_sigma(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_hw->amsdu_enabled = 1;
	cls_wifi_hw->ppdu_tx_type = vendor_ofdma_ppdu_tx_max;
	cls_wifi_hw->txbw = VENDOR_TXBW_MAX;
	memset(&cls_wifi_hw->control, 0, sizeof(cls_wifi_hw->control));
	cls_wifi_hw->control.he_gi = 0xff;
	cls_wifi_hw->control.he_ltf = 0xff;
	return 0;
}

int cls_vendor_set_anti_attack_en(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int rc;
	int en_anti_attack = 0;

	pr_warn("enter %s:anti attack enable=%d\n", __func__, cls_anti_attack.en_anti_attack);
	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ANTI ATTACK ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ANTI_ATTACK_EN]) {
		pr_warn("%s Invalid ANTI ATTACK EN ATTR\n", __func__);

		return -EINVAL;
	}

	en_anti_attack = nla_get_u32(tb[CLS_NL80211_ATTR_ANTI_ATTACK_EN]);
	if (cls_anti_attack.en_anti_attack != en_anti_attack)
		cls_anti_attack.en_anti_attack = en_anti_attack;
	pr_warn("%s:anti attack enable=%d\n", __func__, cls_anti_attack.en_anti_attack);


	return 0;
}

int cls_vendor_set_anti_attack_interval(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int rc;
	int internal_attack_to_lastdata = 0;

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ANTI ATTACK ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ANTI_ATTACK_INTERVAL]) {
		pr_warn("%s Invalid ANTI ATTACK INTERVAL ATTR\n", __func__);

		return -EINVAL;
	}

	internal_attack_to_lastdata = nla_get_u32(tb[CLS_NL80211_ATTR_ANTI_ATTACK_INTERVAL]);
	if (cls_anti_attack.internal_attack_to_lastdata != internal_attack_to_lastdata)
		cls_anti_attack.internal_attack_to_lastdata = internal_attack_to_lastdata;

	pr_warn("%s:anti interval=%d\n", __func__, cls_anti_attack.internal_attack_to_lastdata);
	return 0;
}

int cls_wifi_get_sta_ps_mode(struct cls_wifi_hw *cls_wifi_hw, const u8 *mac_addr, uint8_t *ps)
{
	int i;

	if (!cls_wifi_hw || !mac_addr || !ps)
		return -1;

	for (i = 0; i < hw_remote_sta_max(cls_wifi_hw); i++) {
		struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[i];

		if (sta->valid && (memcmp(mac_addr, &sta->mac_addr, 6) == 0)) {
			*ps = sta->ps.active;
			return 0;
		}
	}

	return -1;
}

int cls_vendor_get_sta_ps_mode(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	int rc;
	uint8_t ps_mode;
	uint8_t mac_addr[6];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc)
		return rc;

	if (tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
		if (cls_wifi_get_sta_ps_mode(cls_wifi_hw, mac_addr, &ps_mode))
			return -EINVAL;
	} else
		rc = -1;

	rc = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_STA_PS_MODE,
			sizeof(ps_mode), &ps_mode);

	return rc;
}

int cls_vendor_get_bctx_pn(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct mm_bctx_pn_cfm cfm;
	struct cls_wifi_vif *vif = NULL;
	struct sk_buff *msg;
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);

	if (wdev)
		vif = container_of(wdev, struct cls_wifi_vif, wdev);

	if (!vif)
		return -EINVAL;
	cls_wifi_send_get_bctx_pn(cls_wifi_hw, vif->vif_index, &cfm);

	/* Format reply message */
	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(uint64_t));
	if (!msg)
		return -ENOMEM;

	nla_put(msg, CLS_NL80211_ATTR_BCTX_PN, sizeof(uint64_t), &(cfm.tx_pn));

	rc = cfg80211_vendor_cmd_reply(msg);

	return rc;
}

int clsemi_vndr_cmds_set_scan_ext(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	int rc;

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data, len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ANTI ATTACK ATTR\n", __func__);
		return rc;
	}

	if (tb[CLS_NL80211_ATTR_SCAN_EXT_ENABLE])
		cls_wifi_hw->scan_ext.ext_enabled =
				nla_get_u32(tb[CLS_NL80211_ATTR_SCAN_EXT_ENABLE]);
	if (tb[CLS_NL80211_ATTR_SCAN_EXT_RX_FILTER])
		cls_wifi_hw->scan_ext.rx_filter =
				nla_get_u32(tb[CLS_NL80211_ATTR_SCAN_EXT_RX_FILTER]);
	if (tb[CLS_NL80211_ATTR_SCAN_EXT_WORK_DURATION])
		cls_wifi_hw->scan_ext.work_duration =
				nla_get_u32(tb[CLS_NL80211_ATTR_SCAN_EXT_WORK_DURATION]);
	if (tb[CLS_NL80211_ATTR_SCAN_EXT_SCAN_INTERVAL])
		cls_wifi_hw->scan_ext.scan_interval =
				nla_get_u32(tb[CLS_NL80211_ATTR_SCAN_EXT_SCAN_INTERVAL]);

	pr_warn("ext_enabled %u rx_filter 0x%x work_duration %u scan_interval%u",
			cls_wifi_hw->scan_ext.ext_enabled, cls_wifi_hw->scan_ext.rx_filter,
			cls_wifi_hw->scan_ext.work_duration, cls_wifi_hw->scan_ext.scan_interval);
	return 0;
}

int cls_vendor_raw_tx(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cfg80211_chan_def chandef;
	struct cfg80211_mgmt_tx_params params;
	u64 cookie;
	int frame_len;

	memset(&params, 0, sizeof(params));
	memset(&chandef, 0, sizeof(chandef));

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc)
		return rc;

	if (!tb[CLS_NL80211_ATTR_FRAME])
		return -EINVAL;

	frame_len = nla_len(tb[CLS_NL80211_ATTR_FRAME]);
	if (frame_len < 24 + 1)
		return -EINVAL;

	if (tb[CLS_NL80211_ATTR_DURATION]) {
		params.wait = nla_get_u32(tb[CLS_NL80211_ATTR_DURATION]);
		if (params.wait < NL80211_MIN_REMAIN_ON_CHANNEL_TIME ||
		    params.wait > wiphy->max_remain_on_channel_duration)
			return -EINVAL;
	}

	if (tb[CLS_NL80211_ATTR_WIPHY_FREQ]) {
		u32 control_freq;

		control_freq = MHZ_TO_KHZ(nla_get_u32(tb[CLS_NL80211_ATTR_WIPHY_FREQ]));
		chandef.chan = ieee80211_get_channel_khz(wiphy, control_freq);
		chandef.width = NL80211_CHAN_WIDTH_20_NOHT;
		chandef.center_freq1 = KHZ_TO_MHZ(control_freq);
		chandef.freq1_offset = control_freq % 1000;
		chandef.center_freq2 = 0;

		if (!chandef.chan || chandef.chan->flags & IEEE80211_CHAN_DISABLED)
			return -EINVAL;
		if (!cfg80211_chandef_valid(&chandef))
			return -EINVAL;
		if (!cfg80211_chandef_usable(wiphy, &chandef, IEEE80211_CHAN_DISABLED))
			return -EINVAL;
		params.offchan = true;
	}

	params.chan = chandef.chan;
	params.buf = nla_data(tb[CLS_NL80211_ATTR_FRAME]);
	params.len = frame_len;

	rc = cls_wifi_cfg80211_mgmt_tx(wiphy, wdev, &params, &cookie);

	return rc;
}

int cls_vendor_set_dyn_pwr_offset(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	int rc;

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data, len, cls_vnd_cmd_policy, NULL);
	if (rc)
		return rc;

	if (tb[CLS_NL80211_ATTR_PWR_OFFSET]) {
		int8_t offset = nla_get_s8(tb[CLS_NL80211_ATTR_PWR_OFFSET]);
		return cls_wifi_send_dyn_pwr_offset_req(cls_wifi_hw, offset);
	}

	return -EINVAL;
}

/* NL80211 vendor command policy */
const struct nla_policy cls_vnd_cmd_policy[CLS_NL80211_ATTR_MAX + 1] = {
	[CLS_NL80211_ATTR_VBSS_ENABLED] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_MAC_ADDR] = { .type = NLA_BINARY, .len = ETH_ALEN },
	[CLS_NL80211_ATTR_VBSS_VAP_INFO] = { .type = NLA_BINARY,
						.len = sizeof(struct cls_vbss_vap_info) },
	[CLS_NL80211_ATTR_VBSS_STA_INFO] = { .type = NLA_BINARY,
						.len = sizeof(struct cls_vbss_driver_sta_info) },
	[CLS_NL80211_ATTR_ROAM_RESULT] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_VBSS_NTHRESH] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_VBSS_MTHRESH] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_RSSI_SMOOTHNESS_FACTOR] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_VBSS_AP_STATS] = { .type = NLA_BINARY,
						.len = sizeof(struct vbss_ap_stats) },
	[CLS_NL80211_ATTR_SINR] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_RSSI] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_VAP_TXQ] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_ANTI_ATTACK_EN] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_ANTI_ATTACK_INTERVAL] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_PPPC_ENABLE] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_PPPC_MODE] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_PPPC_RSSI_THRESH] = { .type = NLA_S8 },
	[CLS_NL80211_ATTR_PPPC_PPS_THRESH] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_PPPC_MCS_THRESH] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_PPPC_PROBE_STEP] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_PPPC_STA_TXPWR] = { .type = NLA_S8 },

	[CLS_NL80211_ATTR_DFX_VAP_IDX] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_DFX_PEER_STA_IDX] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_DFX_RESET] = { .type = NLA_U32 },

	[CLS_NL80211_ATTR_CS_INBDPOW1PUPTHR] = { .type = NLA_S8 },
	[CLS_NL80211_ATTR_CS_CCADELTA] = { .type = NLA_S8 },
	[CLS_NL80211_ATTR_ED_CCA20PRISETHR] = { .type = NLA_S8 },
	[CLS_NL80211_ATTR_ED_CCA20PFALLTHR] = { .type = NLA_S8 },
	[CLS_NL80211_ATTR_ED_CCA20SRISETHR] = { .type = NLA_S8 },
	[CLS_NL80211_ATTR_ED_CCA20SFALLTHR] = { .type = NLA_S8 },

	[CLS_NL80211_ATTR_ATTR_ID] = { .type = NLA_U32 },

	[CLS_NL80211_ATTR_CSI_ENABLE] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_CSI_REPORT_PERIOD] = { .type = NLA_U16 },
	[CLS_NL80211_ATTR_CSI_STA_MACS] = { .type = NLA_NESTED },
	[CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH] = { .type = NLA_U8 },

	[CLS_NL80211_ATTR_ATF_ENABLE] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_ATF_MODE] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_ATF_GRAN] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_ATF_BSS_QUOTA] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_ATF_STA_QUOTA] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_ATF_SCHED_PERIOD] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_ATF_STATS_PERIOD] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_ATF_STATS] = { .type = NLA_BINARY,
						.len = sizeof(struct atf_airtime_stats) },
	[CLS_NL80211_ATTR_BCTX_PN] = { .type = NLA_U64},
	[CLS_NL80211_ATTR_SET_SKIP_DFS_CAC] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_CANCEL_DFS_CAC] = { .type = NLA_U8 },

	[CLS_NL80211_ATTR_NAC_MONITOR_STA] = { .type = NLA_BINARY,
						.len = sizeof(struct nac_monitor_sta) },
	[CLS_NL80211_ATTR_NAC_ENABLE] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_SMTANT_CFG] = { .type = NLA_BINARY,
					.len = sizeof(struct mm_smart_antenna_req) },
	[CLS_NL80211_ATTR_STA_PS_MODE] = { .type = NLA_U8 },
	[CLS_NL80211_ATTR_SCAN_EXT_ENABLE] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[CLS_NL80211_ATTR_SCAN_EXT_RX_FILTER] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[CLS_NL80211_ATTR_SCAN_EXT_WORK_DURATION] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[CLS_NL80211_ATTR_SCAN_EXT_SCAN_INTERVAL] = { .type = NLA_U32, .len = sizeof(uint32_t) },

	/* RAW TX */
	[CLS_NL80211_ATTR_FRAME] = { .type = NLA_BINARY },
	[CLS_NL80211_ATTR_DURATION] = { .type = NLA_U32 },
	[CLS_NL80211_ATTR_WIPHY_FREQ] = { .type = NLA_U32 },

	[CLS_NL80211_ATTR_PWR_OFFSET] = { .type = NLA_S8 },
};

const struct wiphy_vendor_command clsemi_vendor_cmds[] = {
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLSEMI_VENDOR_SUBCMDS_OFDMA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_cfg80211_vndr_cmds_ofdma_handler,
		.policy = VENDOR_CMD_RAW_DATA

	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLSEMI_VENDOR_SUBCMDS_GENERIC
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_cfg80211_vndr_cmds_generic_handler,
		.policy = VENDOR_CMD_RAW_DATA
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLSEMI_VENDOR_SUBCMDS_QOS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_cfg80211_vndr_cmds_qos_handler,
		.policy = VENDOR_CMD_RAW_DATA
	},
	// CLSEMI_VENDOR_SUBCMDS_NAC
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLSEMI_VENDOR_SUBCMDS_NAC
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_cfg80211_vndr_cmds_nac_handler,
		.policy = VENDOR_CMD_RAW_DATA
	},
	//CLSEMI_VENDOR_SUBCMDS_SMANT
#if CONFIG_CLS_SMTANT
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLSEMI_VENDOR_SUBCMDS_SMANT
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_cfg80211_vndr_cmds_smtant_handler,
		.policy = VENDOR_CMD_RAW_DATA
	},
#endif
#ifdef CONFIG_CLS_VBSS

	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_GET_VBSS_ENABLED
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_vbss_enabled,
		.policy = VENDOR_CMD_RAW_DATA
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_VBSS_ENABLED
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_vbss_enabled,
		.policy = VENDOR_CMD_RAW_DATA
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_GET_VBSS_STA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_sta_info,
		.policy = VENDOR_CMD_RAW_DATA
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_GET_VBSS_VAP
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_vap_info,
		.policy = VENDOR_CMD_RAW_DATA
	},
#endif
#if CLSM_WIFI_DFX_CFG
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_DFX_GET_RADIO
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_dfx_get_radio_info,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_DFX_GET_VAP
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_dfx_get_vap_info,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_DFX_GET_PEER_STA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_dfx_get_peer_sta_info,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_DFX_GET_WPU
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_dfx_get_wpu_info,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_DFX_RESET
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_dfx_reset,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
#endif
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_CCA_CS_THR
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_cca_cs_thr,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy

	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_GET_CCA_CS_THR
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_cca_cs_thr,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_CCA_ED_THR
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_cca_ed_thr,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_GET_CCA_ED_THR
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_cca_ed_thr,
		.maxattr = CLS_NL80211_ATTR_MAX,
		.policy = cls_vnd_cmd_policy
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_ANTI_AUTH_ASSOC_ATTACK_EN},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = cls_vendor_set_anti_attack_en,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_ATTACK_TO_TXDATA_INTERVAL},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = cls_vendor_set_anti_attack_interval,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_CSI},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_csi_set,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_GET_CSI},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_csi_get,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_ADD_CSI_STA_MAC},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_add_csi_sta,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_DEL_CSI_STA_MAC},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_del_csi_sta,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
//#if CLS_ATF
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_ENABLE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_enable,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_MODE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_mode,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_GRAN
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_granularity,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_BSS_QUOTA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_bss_quota,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_STA_QUOTA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_sta_quota,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_UPDATE_ATF_QUOTA_TABLE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_update_atf_quota_table,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_SCHED_PERIOD
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_sched_period,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_ATF_STATS_PERIOD
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_atf_stats_period,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		{
			.vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_GET_ATF_STATS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_atf_stats,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
//#endif //CLS_ATF
#ifdef CLS_VIP_QOS
	{
		.info = { .vendor_id = CLSEMI_OUI,
			  .subcmd = CLS_NL80211_CMD_ADD_VIP_QOS_MAC},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cls_qos_add_vip_mac,
		.policy = VENDOR_CMD_RAW_DATA,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			  .subcmd = CLS_NL80211_CMD_DEL_VIP_QOS_MAC},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cls_qos_del_vip_mac,
		.policy = VENDOR_CMD_RAW_DATA,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			  .subcmd = CLS_NL80211_CMD_CLR_VIP_QOS_MAC},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cls_qos_clear_vip_mac,
		.policy = VENDOR_CMD_RAW_DATA,
	},

#endif
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_GET_BCTX_PN},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = cls_vendor_get_bctx_pn,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_SKIP_DFS_CAC},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_skip_dfs_cac,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_CANCEL_DFS_CAC},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_cancel_dfs_cac,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},

#ifdef CONFIG_CLS_NAC
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_ADD_NAC_MONITOR_STA},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_add_nac_monitor_sta,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_DEL_NAC_MONITOR_STA},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_remove_nac_monitor_sta,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_GET_NAC_MONITOR_STA_INFO},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_get_nac_sta_info,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_NAC_ENABLE},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_nac_enable,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_FLUSH_ALL_STA},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_flush_all_sta,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},

#endif
#if CONFIG_CLS_SMTANT
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_SMT_ANTENNA_CFG},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_smtant_cfg,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
#endif
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_GET_STA_PS_MODE},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = cls_vendor_get_sta_ps_mode,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			.subcmd = CLS_NL80211_CMD_SET_SCAN_EXT},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = clsemi_vndr_cmds_set_scan_ext,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_RAW_TX},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV |
			 WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = cls_vendor_raw_tx,
		.policy = VENDOR_CMD_RAW_DATA,
	},
	{
		.info = { .vendor_id = CLSEMI_OUI,
			 .subcmd = CLS_NL80211_CMD_SET_DYN_PWR_OFFSET},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = cls_vendor_set_dyn_pwr_offset,
		.policy = cls_vnd_cmd_policy,
		.maxattr = CLS_NL80211_ATTR_MAX,
	},
};

/* Advertise support vendor specific events */
const struct nl80211_vendor_cmd_info clsemi_vendor_events[CLS_NL80211_CMD_MAX + 1] = {
	[CLS_NL80211_CMD_REPORT_CSI] = { .vendor_id = CLSEMI_OUI,
		.subcmd = CLS_NL80211_CMD_REPORT_CSI },
	[CLS_NL80211_CMD_REPORT_ATF_STATS] = { .vendor_id = CLSEMI_OUI,
		.subcmd = CLS_NL80211_CMD_REPORT_ATF_STATS },
};


uint32_t clsemi_vendor_cmds_size = sizeof(clsemi_vendor_cmds) / sizeof(struct wiphy_vendor_command);
uint32_t clsemi_vendor_events_size = sizeof(clsemi_vendor_events) / sizeof(struct nl80211_vendor_cmd_info);

