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
#include <net/mac80211.h>
#include <net/cfg80211.h>
#include <linux/ieee80211.h>
#include <linux/list.h>
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_msg_rx.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_rx.h"
#include "vendor.h"
#include "nac.h"

#ifdef CONFIG_CLS_NAC

#define EWMA_DIV	(256)
#define EWMA_LEVEL	(64)

struct list_head nac_sta_head[CLS_WIFI_NAC_RADIO_MAX];
int nac_mode_flag[CLS_WIFI_NAC_RADIO_MAX]; //1: nac enable; 0: nac disable
int nac_sta_num[CLS_WIFI_NAC_RADIO_MAX];

static int clsemi_nac_enable_promiscuous_mode(struct cls_wifi_hw *cls_wifi_hw, uint8_t enable);

int init_nac_sta_head(struct cls_wifi_hw *cls_wifi_hw)
{
	INIT_LIST_HEAD(&nac_sta_head[cls_wifi_hw->radio_idx]);
	nac_sta_num[cls_wifi_hw->radio_idx] = 0;
	nac_mode_flag[cls_wifi_hw->radio_idx] = 0;
	wiphy_info(cls_wifi_hw->wiphy, "nac sta init\n");
	return 0;
}

int dinit_nac_sta_head(struct cls_wifi_hw *cls_wifi_hw)
{
	struct NAC_STA_DATA *cur, *tmp;

	if ((nac_sta_head[cls_wifi_hw->radio_idx].next == NULL) && (nac_sta_head[cls_wifi_hw->radio_idx].prev == NULL))
		goto nac_sta_exit;

	list_for_each_entry_safe(cur, tmp, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		list_del(&cur->list);
		kfree(cur);
	}
	if (list_empty(&nac_sta_head[cls_wifi_hw->radio_idx]))
		pr_err("--dinit nac:nac_sta_head is empty.\n");

nac_sta_exit:
	nac_sta_num[cls_wifi_hw->radio_idx] = 0;
	nac_mode_flag[cls_wifi_hw->radio_idx] = 0;
	return 0;

}

uint32_t nac_ewma(uint32_t old_val, uint32_t new_val, uint32_t weight)
{
	return ((new_val * (EWMA_DIV - weight) + old_val * weight) / EWMA_DIV);
}


/* allow un-encrypted, TA != BSSID packets to be forwarded to Host side */
static int clsemi_nac_enable_promiscuous_mode(struct cls_wifi_hw *cls_wifi_hw, uint8_t enable)
{
#if 1
	uint32_t rx_filter = 0;

	if (enable)
		rx_filter = CLS_MAC_ACCEPT_UNICAST_BIT | CLS_MAC_ACCEPT_OTHER_BSSID_BIT;

	clsemi_send_rx_filter(cls_wifi_hw, rx_filter);
#endif
	return 0;
}

//success :0 ; fail : 1
int add_nac_sta(struct cls_wifi_hw *cls_wifi_hw, u8 *mac)
{
	struct NAC_STA_DATA *new_data;
	struct NAC_STA_DATA *data = NULL;

	if (nac_sta_num[cls_wifi_hw->radio_idx] == MAX_NAC_STA_NUM) {
		pr_err("%s: check nac sta num failed\n", __func__);
		return -1;
	}

	list_for_each_entry(data, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		if (!memcmp(data->sta.mac_addr, mac, ETH_ALEN)) {
			pr_err("%s: mac[%pM] exist in list\n", __func__, mac);
			return -1;
		}
	}

	new_data = kmalloc(sizeof(struct NAC_STA_DATA), GFP_KERNEL);
	memset(new_data, 0, sizeof(struct NAC_STA_DATA));

	memcpy(new_data->sta.mac_addr, mac, ETH_ALEN);

	list_add_tail(&new_data->list, &nac_sta_head[cls_wifi_hw->radio_idx]);

	nac_sta_num[cls_wifi_hw->radio_idx]++;

	return 0;
}

int del_nac_sta(struct cls_wifi_hw *cls_wifi_hw, u8 *mac)
{
	struct NAC_STA_DATA *data = NULL;
	struct NAC_STA_DATA *pdel = NULL;

	if (nac_sta_num[cls_wifi_hw->radio_idx] == 0)
		return 0;
	list_for_each_entry(data, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		if (!memcmp(data->sta.mac_addr, mac, ETH_ALEN)) {
			pdel = data;
			break;
		}
	}
	if (pdel != NULL) {
		list_del(&pdel->list);
		kfree(pdel);
		nac_sta_num[cls_wifi_hw->radio_idx]--;
	}
	return 0;
}

int get_nac_sta(struct cls_wifi_hw *cls_wifi_hw, struct nac_monitor_sta *sta_info)
{
	struct NAC_STA_DATA *node = NULL;

	list_for_each_entry(node, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		if (!memcmp(node->sta.mac_addr, sta_info->mac_addr, ETH_ALEN)) {
			memcpy(sta_info, &node->sta, sizeof(*sta_info));
			break;
		}
	}
	return 0;
}

int set_nac_enabled(struct cls_wifi_hw *cls_wifi_hw, u32 enabled)
{
	if (nac_mode_flag[cls_wifi_hw->radio_idx] != enabled)
	{
		nac_mode_flag[cls_wifi_hw->radio_idx] = enabled;
		pr_err("radio[%d]:set nac promiscuous %s\n", cls_wifi_hw->radio_idx,
				(nac_mode_flag[cls_wifi_hw->radio_idx] == 1) ? "enabled" : "disabled");
		clsemi_nac_enable_promiscuous_mode(cls_wifi_hw, nac_mode_flag[cls_wifi_hw->radio_idx]);
	}

	return 0;
}

/* Get the nac station info */
static int clsemi_nac_get_sta_info(struct cls_wifi_hw *cls_wifi_hw,
		const uint8_t *mac, struct nac_monitor_sta *sta_info)
{
	struct NAC_STA_DATA *node = NULL;

	list_for_each_entry(node, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		if (!memcmp(node->sta.mac_addr, mac, ETH_ALEN)) {
			memcpy(sta_info, &node->sta, sizeof(*sta_info));
			break;
		}
	}
	return 0;
}

int clsemi_vndr_cmds_set_nac_enable(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int rc;
	int ret = 0;
	u32 enabled = 0;

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid nac ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_NAC_ENABLE]) {
		pr_warn("%s Invalid nac sta info ATTR\n", __func__);

		return -EINVAL;
	}

	enabled = nla_get_u32(tb[CLS_NL80211_ATTR_NAC_ENABLE]);

	set_nac_enabled(cls_wifi_hw, enabled);

	return ret;
}

/* Get full STA info with specific struct */
int clsemi_vndr_cmds_get_nac_sta_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct nac_monitor_sta sta_info;
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	u8 mac_addr[ETH_ALEN];
	struct sk_buff *msg;
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid nac ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		pr_warn("%s Invalid nac sta info ATTR\n", __func__);

		return -EINVAL;
	}

	/* Get STA info*/
	memset(&sta_info, 0, sizeof(sta_info));
	ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
	ether_addr_copy(sta_info.mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
	pr_warn("----get sta[%pM] \n", sta_info.mac_addr);
	clsemi_nac_get_sta_info(cls_wifi_hw, mac_addr, &sta_info);
	/* format reply message */
	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(struct nac_monitor_sta));
	if (!msg)
		return -ENOMEM;
	pr_warn("sta_info mac[%pM], rssi[%d], sinr[%d], last_timer[%d]",
			sta_info.mac_addr, sta_info.rssi, sta_info.sinr, sta_info.last_rx_time);

	nla_put(msg, CLS_NL80211_ATTR_NAC_MONITOR_STA,
			sizeof(struct nac_monitor_sta), &sta_info);
	rc = cfg80211_vendor_cmd_reply(msg);

	return 0;
}

int clsemi_vndr_cmds_add_nac_monitor_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	u8 mac_addr[ETH_ALEN];
	int rc;
	int ret = 0;

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid nac ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		pr_warn("%s Invalid nac sta info ATTR\n", __func__);

		return -EINVAL;
	}

	memcpy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]), ETH_ALEN);

	add_nac_sta(cls_wifi_hw, mac_addr);

	return ret;
}

int clsemi_vndr_cmds_remove_nac_monitor_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	u8 mac_addr[ETH_ALEN];
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid nac ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		pr_warn("%s Invalid nac sta info ATTR\n", __func__);

		return -EINVAL;
	}
	memcpy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]), ETH_ALEN);

	del_nac_sta(cls_wifi_hw, mac_addr);

	return 0;

}

int clsemi_vndr_cmds_flush_all_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct NAC_STA_DATA *cur, *tmp;
	struct cls_wifi_hw *cls_wifi_hw = NULL;

	cls_wifi_hw = wiphy_priv(wiphy);

	if ((nac_sta_head[cls_wifi_hw->radio_idx].next == NULL) && (nac_sta_head[cls_wifi_hw->radio_idx].prev == NULL)) {
		pr_err("nac_sta_head is empty.\n");
		return 0;
	}
	list_for_each_entry_safe(cur, tmp, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		list_del(&cur->list);
		kfree(cur);
	}
	if (list_empty(&nac_sta_head[cls_wifi_hw->radio_idx]))
		pr_err("flush all sta,nac_sta_head is empty.\n");
	nac_sta_num[cls_wifi_hw->radio_idx] = 0;
	return 0;
}

static struct nac_monitor_sta *nac_get_monitor_sta(struct cls_wifi_hw *cls_wifi_hw, uint8_t *sta_mac)
{
	struct nac_monitor_sta *sta = NULL;
	struct NAC_STA_DATA *data = NULL;

	list_for_each_entry(data, &nac_sta_head[cls_wifi_hw->radio_idx], list) {
		if (!memcmp(data->sta.mac_addr, sta_mac, ETH_ALEN)) {
			sta = &data->sta;
			break;
		}
	}
	return sta;
}

uint8_t get_sinr(struct rx_vector_2 *rx_vect2)
{
	int i = 0;
	u32 sum = 0;
	uint8_t val;

	for (; i < 16; i++)
	{
		sum += (rx_vect2->sinr[i] >> 8) & 0xff;
		sum += rx_vect2->sinr[i] & 0xff;
	}

	val = (sum / 32) & 0xff;
	return val;
}


/* Report per packet info to vendor */
void nac_monitor_per_pkt_info(struct cls_wifi_hw *cls_wifi_hw, struct hw_rxhdr *hw_rxhdr, struct rx_vector_2 *rx_vect2)
{
	struct nac_monitor_sta *sta;
	struct ieee80211_hdr *hdr;
	int8_t rssi;

	if (!nac_mode_flag[cls_wifi_hw->radio_idx])
		return;

	if (nac_sta_num[cls_wifi_hw->radio_idx] == 0)
		return;

	if (!RX_FLAGS_GET(80211_MPDU, hw_rxhdr->flags))
		return;

	/* TODO: get RSSI/SINR for pending monitor stations */
	hdr = (struct ieee80211_hdr *)((uint8_t *)hw_rxhdr + sizeof(struct hw_rxhdr));
	sta = nac_get_monitor_sta(cls_wifi_hw, hdr->addr2);
	if (sta == NULL) {
		return;
	}

	sta->last_rx_time = jiffies;
	rssi = hw_rxhdr->hwvect.rx_vect1.rssi_leg;
	sta->rssi = rssi;
	/* No sinr info in rxvec of Dubhe1000 */
	if (!rx_vect2)
		sta->sinr = 0;
	else
		sta->sinr = get_sinr(rx_vect2);
}

#endif // CLS_nac

