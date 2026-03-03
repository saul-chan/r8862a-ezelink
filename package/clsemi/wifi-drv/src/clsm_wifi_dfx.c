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
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/sort.h>

#include "cls_wifi_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_radar.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_version.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_msgq.h"
#include "ipc_shared.h"
#include "cls_wifi_irf.h"
#include "clsm_wifi_dfx.h"
#include <net/netlink.h>
#include <net/cfg80211.h>

#include "cls_nl80211_vendor.h"

#if CLSM_WIFI_DFX_CFG

int clsdfx_get_radio_stats(struct cls_wifi_hw *cls_wifi_hw,
		struct _clsm_radio_extstats *radio_stats)
{
	pr_warn("%s cls_wifi_hw: %px\n",__func__,cls_wifi_hw);
	memset(radio_stats, 0, sizeof(struct _clsm_radio_extstats));

	return 0;
}


int clsdfx_get_vap_stats(struct cls_wifi_hw *cls_wifi_hw,int vap_index,
		struct _clsm_vap_extstats *vap_stats)
{
	pr_warn("%s cls_wifi_hw: %px, vap_index:%d\n",__func__,cls_wifi_hw,vap_index);
	memset(vap_stats, 0, sizeof(struct _clsm_vap_extstats));

	if(vap_index < 0) {
		return -1;
	}

	return 0;
}

int clsdfx_get_peer_sta_stats(struct cls_wifi_hw *cls_wifi_hw,int peer_sta_index,char* stamac,
	struct cls_wifi_vif *cls_wifi_vif,struct _clsm_persta_extstats *peer_sta_stats)
{
	struct cls_wifi_sta *sta = NULL;
	struct rx_vector_1 *last_rx;
	const u8 *mac_addr;

	pr_warn("%s cls_wifi_hw: %px,peer_sta_index:%d\n",__func__,cls_wifi_hw,peer_sta_index);
	memset(peer_sta_stats, 0, sizeof(struct _clsm_persta_extstats));
	if((hw_remote_sta_max(cls_wifi_hw) <= peer_sta_index) && (stamac == NULL)) {
		return -1;
	}

	if(stamac == NULL) {
		sta = &cls_wifi_hw->sta_table[peer_sta_index];
	}
	else {
		struct cls_wifi_sta *cur;
		memcpy(peer_sta_stats->mac_addr, stamac, 6);
		if(cls_wifi_vif){
			list_for_each_entry(cur, &cls_wifi_vif->ap.sta_list, list) {
				if (!memcmp(cur->mac_addr, stamac, 6)) {
					sta = cur;
					break;
				}
			}
		}else{
			uint32_t i;
			for(i  = 0; i < hw_remote_sta_max(cls_wifi_hw); i++){
				cur = &cls_wifi_hw->sta_table[i];
				if ((sta->valid) && (!memcmp(cur->mac_addr, stamac, 6))) {
					sta = cur;
					break;
				}
			}
		}
	}

	if(sta == NULL) {
		return -1;
	}

	if (!sta->valid){
		return 0;
	}
	mac_addr = cls_wifi_sta_addr(sta);
	memcpy(peer_sta_stats->mac_addr, mac_addr, 6);

	last_rx = &sta->stats.last_rx.rx_vect1;

	peer_sta_stats->rssi = last_rx->rssi1;


	return 0;
}


int clsdfx_get_wpu_stats(struct cls_wifi_hw *cls_wifi_hw,
		struct _clsm_wpu_extstats *wpu_stats)
{
	pr_warn("%s cls_wifi_hw: %p\n", __func__, cls_wifi_hw);
	wpu_stats->cpu_idle_percent = 200;

	return 0;
}


int clsdfx_reset_stats(struct cls_wifi_hw *cls_wifi_hw,
		struct _clsm_dfx_reset* _type)
{
	pr_warn("%s _type->flag: 0x%x\n",__func__,_type->flag);

	return 0;
}

////////
int clsm_cfg80211_cmd_reply(struct wiphy *wiphy, int attr,int msg_size, void* msg)
{
	struct sk_buff *skb_msg;
	int rc;

	/* Format reply message */
	skb_msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, msg_size);
	if (!skb_msg)
		return -ENOMEM;

	nla_put(skb_msg, attr, msg_size, msg);

	rc = cfg80211_vendor_cmd_reply(skb_msg);

	return rc;
}

extern const struct nla_policy cls_vnd_cmd_policy[CLS_NL80211_ATTR_MAX + 1];
/////////////////////NL80211
int clsemi_vndr_cmds_dfx_get_radio_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct dbg_get_mib_cfm *cfm = kzalloc(sizeof(struct dbg_get_mib_cfm), GFP_ATOMIC);
	int ret;

	if (!cfm)
		return -ENOMEM;

	ret = cls_wifi_send_dbg_get_mib_req(cls_wifi_hw, cfm);
	if (ret) {
		kfree(cfm);
		return ret;
	}

	ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_DFX_RADIO, sizeof(*cfm), cfm);
	kfree(cfm);
	return ret;
}


int clsemi_vndr_cmds_dfx_get_vap_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(wdev->netdev);
	int ret;

	ret = cls_wifi_send_dbg_get_mgmt_req(cls_wifi_hw, cls_wifi_vif->vif_index,
			&cls_wifi_vif->dfx_stats.mgmt_stats);
	if (ret)
		return ret;

	return clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_DFX_VAP,
			sizeof(cls_wifi_vif->dfx_stats), &cls_wifi_vif->dfx_stats);
}

int clsemi_vndr_cmds_dfx_get_peer_sta_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_sta *cls_wifi_sta;
	uint8_t mac_addr[6];
	int tb_size = (CLS_NL80211_ATTR_MAX + 1) * sizeof(struct nlattr*);
	struct nlattr **tb = (struct nlattr **)kmalloc(tb_size, GFP_ATOMIC);
	int ret;

	if (!tb)
		goto RETURN_FREE_TB;

	ret = nla_parse(tb, CLS_NL80211_ATTR_MAX, data, len, cls_vnd_cmd_policy, NULL);
	if (ret)
		goto RETURN_FREE_TB;

	if (!tb[CLS_NL80211_ATTR_MAC_ADDR])
		goto RETURN_FREE_TB;

	ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
	cls_wifi_sta = cls_wifi_get_sta_from_mac(cls_wifi_hw, mac_addr);
	if (!cls_wifi_sta)
		goto RETURN_FREE_TB;

	ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_DFX_PEER_STA,
			sizeof(cls_wifi_sta->dfx_stats), &cls_wifi_sta->dfx_stats);
RETURN_FREE_TB:
	if (tb)
		kfree(tb);
	if (ret)
		pr_warn("%s ret %d.\n", __func__, ret);
	return ret;
}

int clsemi_vndr_cmds_dfx_get_wpu_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int ret;
	struct _clsm_wpu_extstats stats;

	cls_wifi_hw = wiphy_priv(wiphy);

	clsdfx_get_wpu_stats(cls_wifi_hw, &stats);
	ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_DFX_WPU, sizeof(stats), &stats);

	return ret;
}


int clsemi_vndr_cmds_dfx_reset(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = wiphy_priv(wiphy);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(wdev->netdev);
	struct cls_wifi_sta *cls_wifi_sta;
	struct _clsm_dfx_reset _type;
	uint8_t mac_addr[6];
	int tb_size = (CLS_NL80211_ATTR_MAX + 1) * sizeof(struct nlattr*);
	struct nlattr **tb = (struct nlattr **)kmalloc(tb_size, GFP_ATOMIC);
	int ret = -1;

	if (!tb)
		goto RETURN_FREE_TB;

	ret = nla_parse(tb, CLS_NL80211_ATTR_MAX, data, len, cls_vnd_cmd_policy, NULL);
	if (ret)
		goto RETURN_FREE_TB;

	if (!tb[CLS_NL80211_ATTR_DFX_RESET])
		goto RETURN_FREE_TB;

	_type.flag = nla_get_u32(tb[CLS_NL80211_ATTR_DFX_RESET]);

	switch (_type.flag) {
	case CLSM_WIFI_STATS_TYPE_VAP:
		memset(&cls_wifi_vif->dfx_stats, 0, sizeof(cls_wifi_vif->dfx_stats));
		ret = cls_wifi_send_dbg_reset_mgmt_req(cls_wifi_hw, cls_wifi_vif->vif_index);
		if (ret)
			goto RETURN_FREE_TB;
		break;
	case CLSM_WIFI_STATS_TYPE_PEER_STA:
		if (!tb[CLS_NL80211_ATTR_MAC_ADDR])
			goto RETURN_FREE_TB;

		ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
		cls_wifi_sta = cls_wifi_get_sta_from_mac(cls_wifi_hw, mac_addr);
		if (!cls_wifi_sta)
			goto RETURN_FREE_TB;
		memset(&cls_wifi_sta->dfx_stats, 0, sizeof(cls_wifi_sta->dfx_stats));
		break;
	default:
		goto RETURN_FREE_TB;
	}

	ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_DFX_RESET_R, sizeof(_type.flag),
			&_type.flag);

RETURN_FREE_TB:
	if (tb)
		kfree(tb);
	if (ret)
		pr_warn("%s ret %d.\n", __func__, ret);
	return ret;
}


#endif

