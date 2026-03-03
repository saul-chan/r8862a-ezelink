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
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_defs.h"
#include "vendor.h"
#include "vbss.h"
#ifdef CONFIG_CLS_VBSS


static struct cls_vbss_info vbss_cb;

/* allow un-encrypted, TA != BSSID packets to be forwarded to Host side */
static int clsemi_vbss_enable_promiscuous_mode(struct cls_wifi_hw *cls_wifi_hw, uint8_t enable)
{
	uint32_t rx_filter = 0;

	if (enable)
		rx_filter = CLS_MAC_ACCEPT_MULTICAST_BIT
				| CLS_MAC_ACCEPT_BROADCAST_BIT
				| CLS_MAC_ACCEPT_UNICAST_BIT
				| CLS_MAC_ACCEPT_DECRYPT_ERROR_FRAMES_BIT
				| CLS_MAC_EXC_UNENCRYPTED_BIT
				| CLS_MAC_ACCEPT_OTHER_BSSID_BIT;

	clsemi_send_rx_filter(cls_wifi_hw, rx_filter);

	return 0;
}

/* Enter the VBSS mode, setup the MAC parameters */
static int clsemi_vbss_mode_enter(struct cls_wifi_hw *cls_wifi_hw)
{
	/* clean all VAPs, needs to be done by CLSAPI and hostapd*/

	/* disable channal selection. Not yet ready */

	/* disable BSS color. Disabled by default, add it when BSS color feature ready */

	/* disable Multiple BSSID. Disabled by default, add it when Multi-BSSID  feature ready  */

	/* disable txBF */
	clsemi_set_beamformer(cls_wifi_hw->wiphy, NULL, 0);

	/* disable MU */
	clsemi_enable_mu_tx(cls_wifi_hw->wiphy, 0);
	clsemi_enable_mu_rx(cls_wifi_hw->wiphy, 0);

	clsemi_vbss_enable_promiscuous_mode(cls_wifi_hw, 1);

	return 0;
}

static int clsemi_vbss_mode_exit(struct cls_wifi_hw *cls_wifi_hw)
{
	/* Enable txBF */
	clsemi_set_beamformer(cls_wifi_hw->wiphy, NULL, 1);

	/* Enable MU */
	clsemi_enable_mu_tx(cls_wifi_hw->wiphy, 1);
	clsemi_enable_mu_rx(cls_wifi_hw->wiphy, 1);
	clsemi_vbss_enable_promiscuous_mode(cls_wifi_hw, 0);

	return 0;
}

/* Get the station info used for roaming */
static int clsemi_vbss_get_sta_info(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_sta *sta, struct cls_vbss_sta_info *sta_info)
{
	struct cls_wifi_vif *vif = NULL;

	/* Get VAP of this STA */
	vif = cls_wifi_get_vif(cls_wifi_hw, sta->vif_idx);
	if (vif == NULL) {
		pr_warn("%s can't get VIF from sta\n", __func__);

		return -EINVAL;
	}

	memset(sta_info, 0, sizeof(struct cls_vbss_sta_info));

	/* Get STA capabilities */
	ether_addr_copy(sta_info->sta_mac, sta->mac_addr);
	ether_addr_copy(sta_info->bssid, vif->macaddr);
	memcpy(&sta_info->key_info, &sta->key_info, sizeof(sta_info->key_info));
	clsemi_get_sta_seq_num_req(cls_wifi_hw, &vbss_cb.get_seq_cfm);

	/* Get AMPDU related info */

	/* update STA capabilities */


	return 0;
}


/* Initiate the VAP parameters */
int clsemi_vbss_vap_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif)
{

	/* set max associated STA number */
	vif->max_assoc_sta = 1;
	vif->assoc_sta_count  = 0;

	return 0;
}


/* Get the current VBSS mode: enabled or not */
int clsemi_vndr_cmds_get_vbss_enabled(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct sk_buff *msg;
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);

	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 1);
	if (!msg)
		return -ENOMEM;

	nla_put_u8(msg, CLS_NL80211_ATTR_VBSS_ENABLED, cls_wifi_hw->vbss_enabled);
	rc = cfg80211_vendor_cmd_reply(msg);

	return rc;
}

/* Set the VBSS mode enabled 0/1 */
int clsemi_vndr_cmds_set_vbss_enabled(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint8_t vbss_enabled;
	int rc;
	struct nlattr *tb[CLS_VENDOR_ATTR_VBSS_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_VENDOR_ATTR_VBSS_MAX, data,
			len, cls_vnd_cmd_vbss_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid VBSS ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_VBSS_ENABLED]) {
		pr_warn("%s Invalid VBSS enabled ATTR\n", __func__);

		return -EINVAL;
	}

	vbss_enabled = nla_get_u8(tb[CLS_NL80211_ATTR_VBSS_ENABLED]);
	pr_warn("%s vbss enabled: %u -> %u", __func__, vbss_enabled, cls_wifi_hw->vbss_enabled);
	cls_wifi_hw->vbss_enabled = vbss_enabled;
	if (vbss_enabled)
		clsemi_vbss_mode_enter(cls_wifi_hw);
	else
		clsemi_vbss_mode_exit(cls_wifi_hw);

	return 0;
}

/* Get full STA info with specific struct */
int clsemi_vndr_cmds_get_sta_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_VENDOR_ATTR_VBSS_MAX + 1];
	struct cls_vbss_sta_info *sta_info;
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct cls_wifi_sta *sta = NULL;
	u8 mac_addr[ETH_ALEN];
	struct sk_buff *msg;
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_VENDOR_ATTR_VBSS_MAX, data,
			len, cls_vnd_cmd_vbss_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid VBSS ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_VBSS_MAC_ADDR]) {
		pr_warn("%s Invalid VBSS sta info ATTR\n", __func__);

		return -EINVAL;
	}

	/* Get STA */
	ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_VBSS_MAC_ADDR]));
	sta = cls_wifi_get_sta_from_mac(cls_wifi_hw, mac_addr);
	if (sta == NULL) {
		pr_warn("%s can't get sta from MAC address\n", __func__);

		return -EINVAL;
	}

	sta_info = &vbss_cb.vbss_sta;
	clsemi_vbss_get_sta_info(cls_wifi_hw, sta, sta_info);
	/* format reply message */
	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(struct cls_vbss_sta_info));
	if (!msg)
		return -ENOMEM;

	nla_put(msg, CLS_NL80211_ATTR_VBSS_STA_INFO, sizeof(*sta_info), sta_info);
	rc = cfg80211_vendor_cmd_reply(msg);

	return 0;
}

/* Get full VAP info with specific struct */
int clsemi_vndr_cmds_get_vap_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_VENDOR_ATTR_VBSS_MAX + 1];
	struct cls_vbss_vap_info vap_info;
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct cls_wifi_vif *vif = NULL;
	u8 mac_addr[ETH_ALEN];
	struct sk_buff *msg;
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_VENDOR_ATTR_VBSS_MAX, data,
			len, cls_vnd_cmd_vbss_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid VBSS ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_VBSS_MAC_ADDR]) {
		pr_warn("%s Invalid VBSS sta info ATTR\n", __func__);

		return -EINVAL;
	}

	/* Get VAP */
	ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_VBSS_MAC_ADDR]));
	vif = cls_wifi_get_vif_from_mac(cls_wifi_hw, mac_addr);
	if (vif == NULL) {
		pr_warn("%s can't get VIF from sta\n", __func__);

		return -EINVAL;
	}

	/* Format reply message */
	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(vap_info));
	if (!msg)
		return -ENOMEM;

	nla_put(msg, CLS_NL80211_ATTR_VBSS_VAP_INFO, sizeof(vap_info), &vap_info);

	rc = cfg80211_vendor_cmd_reply(msg);

	return rc;
}

int clsemi_vbss_init(struct cls_wifi_hw *cls_wifi_hw)
{
	pr_warn("%s disable vbss by default\n", __func__);
	cls_wifi_hw->vbss_enabled = 0;

	return 0;
}

#endif // CLS_VBSS

