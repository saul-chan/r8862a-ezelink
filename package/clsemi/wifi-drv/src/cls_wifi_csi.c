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
#include "cls_nl80211_vendor.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_compat.h"
#include "cls_wifi_utils.h"
#include "ipc_host.h"
#include "cls_wifi_csi.h"
#include "cls_wifi_msg_tx.h"
#include "vendor.h"

static int cls_wifi_is_csi_enabled(struct cls_wifi_hw *wifi_hw)
{
	return (!!wifi_hw->csi_params.enabled);
}

#define CSI_SAVE_FILE   1
#define CSI_BUF_SIZE    512
void cls_wifi_dump_csi_iq_info(struct cls_wifi_hw *wifi_hw,struct cls_csi_report *pcsi)
{
	struct cls_csi_info_per_subcarrier *subc;
	struct cls_csi_report_info_iq *iq = &pcsi->csi_iq;
	uint16_t subc_num;
	int i;

#if CSI_SAVE_FILE
	u8 buf[CSI_BUF_SIZE];
	u8 *mac_addr;
	struct cls_wifi_sta *sta;
	uint16_t buf_size = 0;

	mac_addr = &pcsi->info_aligned.info.sta_mac_addr[0];
	sta = cls_wifi_get_sta_from_mac(wifi_hw, mac_addr);
	if(sta == NULL)
		return;
	if (sta->csilog == NULL)
	{
		snprintf(buf, CSI_BUF_SIZE,"/tmp/sta_csi_%04x_%04x_%04x",
			sta->mac_addr[0],sta->mac_addr[1],sta->mac_addr[2]);
		sta->csilog = filp_open(buf, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (IS_ERR(sta->csilog)) {
			pr_warn("Open target file fail: %s\n", buf);
			return;
		}
		pr_warn("Open target file succeeds: %s\n", buf);
	}
#endif ///CSI_SAVE_FILE

	subc_num = min_t(int, CSI_MAX_SUBCARRIER_NUM, pcsi->info_aligned.info.subcarrier_num);
#if CSI_SAVE_FILE
#if defined(CFG_MERAK3000)
	buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
		"\nppdu id: %-6u"
#if defined(CFG_MERAK3000)
		" csi_format: %-6u"
#endif
		" csi_length: %-6u"
		" timestamp: %-10u %-10u\n",
		pcsi->hdr.ppdu_id
#if defined(CFG_MERAK3000)
		, pcsi->hdr.csi_format
#endif
		, pcsi->hdr.csi_length
		, pcsi->info_aligned.info.timestamp_hi,
		pcsi->info_aligned.info.timestamp_lo);
#else
	buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
		"\nppdu id: %-6u csi_length: %-6u timestamp: %-10u %-10u\n",
		pcsi->hdr.ppdu_id, pcsi->hdr.csi_length,
		pcsi->info_aligned.info.timestamp_hi,
		pcsi->info_aligned.info.timestamp_lo);
#endif
	buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
		"TA MAC: %pM RA MAC: %pM\n",
		&pcsi->info_aligned.info.sta_mac_addr[0],
		&pcsi->info_aligned.info.dest_mac_addr[0]);
	buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
		"rx nss: %u tx nss: %u rssi: %3d chan: %-3u bw: %u subcarrier num: %-3u\n",
		pcsi->info_aligned.info.rx_nss, pcsi->info_aligned.info.tx_nss,
		pcsi->info_aligned.info.rssi, pcsi->info_aligned.info.channel,
		pcsi->info_aligned.info.bw, pcsi->info_aligned.info.subcarrier_num);
	buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
		"format mod: %u ltf: %u xx-ltf: %u agc: %3d %3d %3d %3d %3d %3d %3d %3d\n",
		pcsi->info_aligned.info.ppdu_format,
		pcsi->info_aligned.info.ltf_type,
		pcsi->info_aligned.info.xx_ltf_type,
		pcsi->info_aligned.info.agc[0], pcsi->info_aligned.info.agc[1],
		pcsi->info_aligned.info.agc[2], pcsi->info_aligned.info.agc[3],
		pcsi->info_aligned.info.agc[4], pcsi->info_aligned.info.agc[5],
		pcsi->info_aligned.info.agc[6], pcsi->info_aligned.info.agc[7]);

	kernel_write(sta->csilog, buf, buf_size, &sta->csilog->f_pos);
	buf_size = 0;
#else
	pr_info("%s subc_num %u\n", __func__, subc_num);
#endif ///CSI_SAVE_FILE
	for (i = 0; i < subc_num; i++) {
		subc = &iq->subc[i];
#if CSI_SAVE_FILE
#if defined(CFG_MERAK3000)
		buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
			"%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
			subc->h00_q, subc->h00_i, subc->h10_q, subc->h10_i,
			subc->h20_q, subc->h20_i, subc->h01_q, subc->h01_i,
			subc->h11_q, subc->h11_i, subc->h21_q, subc->h21_i);
#else
		buf_size += snprintf(&buf[buf_size], CSI_BUF_SIZE - buf_size,
			"%04x %04x %04x %04x %04x %04x %04x %04x\n",
			subc->h00_q, subc->h00_i, subc->h01_q, subc->h01_i,
			subc->h10_q, subc->h10_i, subc->h11_q, subc->h11_i);
#endif
		if(buf_size > (CSI_BUF_SIZE - 64)) {
			kernel_write(sta->csilog, buf, buf_size, &sta->csilog->f_pos);
			buf_size = 0;
		}
#else
#if defined(CFG_MERAK3000)
		pr_info("%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
			subc->h00_q, subc->h00_i, subc->h10_q, subc->h10_i,
			subc->h20_q, subc->h20_i, subc->h01_q, subc->h01_i,
			subc->h11_q, subc->h11_i, subc->h21_q, subc->h21_i);
#else
		pr_info("%04x %04x %04x %04x %04x %04x %04x %04x\n",
			subc->h00_q, subc->h00_i, subc->h01_q, subc->h01_i,
			subc->h10_q, subc->h10_i, subc->h11_q, subc->h11_i);
#endif
#endif  ///CSI_SAVE_FILE
	}

#if CSI_SAVE_FILE
	if(buf_size > 0) {
		kernel_write(sta->csilog, buf, buf_size, &sta->csilog->f_pos);
		buf_size = 0;
	}
#endif  ///CSI_SAVE_FILE
}

static void cls_wifi_dump_csi_report(struct cls_wifi_hw *wifi_hw,
		struct cls_csi_report *pcsi)
{
	/* direct print when log level > 2 */
	if (wifi_hw->csi_params.log_level > 2) {
		pr_info("\nppdu id: %-6u csi_length: %-6u timestamp: %-10u %-10u\n",
			pcsi->hdr.ppdu_id, pcsi->hdr.csi_length,
			pcsi->info_aligned.info.timestamp_hi,
			pcsi->info_aligned.info.timestamp_lo);
		pr_info("TA MAC address: %pM\n", &pcsi->info_aligned.info.sta_mac_addr[0]);
		pr_info("RA MAC address: %pM\n", &pcsi->info_aligned.info.dest_mac_addr[0]);
		pr_info("rx nss: %u tx nss: %u rssi: %3d chan: %-3u bw: %u subcarrier num: %-3u\n",
			pcsi->info_aligned.info.rx_nss, pcsi->info_aligned.info.tx_nss,
			pcsi->info_aligned.info.rssi, pcsi->info_aligned.info.channel,
			pcsi->info_aligned.info.bw, pcsi->info_aligned.info.subcarrier_num);
		pr_info("agc: %3d %3d %3d %3d %3d %3d %3d %3d\n",
			pcsi->info_aligned.info.agc[0], pcsi->info_aligned.info.agc[1],
			pcsi->info_aligned.info.agc[2], pcsi->info_aligned.info.agc[3],
			pcsi->info_aligned.info.agc[4], pcsi->info_aligned.info.agc[5],
			pcsi->info_aligned.info.agc[6], pcsi->info_aligned.info.agc[7]);
	} else if (wifi_hw->csi_params.save_csi_to_file) {
		/* Save CSI header and IQ info to file */
		cls_wifi_dump_csi_iq_info(wifi_hw, pcsi);
	}
}

void cls_wifi_vndr_event_report(struct cls_wifi_hw *wifi_hw,
		struct cls_csi_report *report)
{
#ifndef offset
#define offset(type, member) ((long) &((type *) 0)->member)
#endif
#define CLSCSI_REPORT_MSG_FRAG_MAX	(8192)

	struct sk_buff *skb;
	struct wiphy *wiphy;
	struct wireless_dev *wdev;
	u8 *mac_addr;
	struct cls_wifi_sta *sta;
	struct cls_wifi_vif * vif;
	uint32_t remain_len = offset(struct cls_csi_report, csi_iq) + 4 * report->hdr.csi_length;
	uint32_t send_len = 0, frag_offset = 0;
	uint8_t more_frag = 0, *msg_buf = (uint8_t *)report;

	/* send event, event index 2 is CLS_NL80211_CMD_REPORT_CSI */
	wiphy = wifi_hw->wiphy;
	mac_addr = &report->info_aligned.info.sta_mac_addr[0];
	sta = cls_wifi_get_sta_from_mac(wifi_hw, mac_addr);
	if (sta == NULL) {
		pr_info("%s not found sta mac_addr %pM\n", __func__, mac_addr);

		return;
	}

	vif = cls_wifi_get_vif(wifi_hw, sta->vif_idx);
	if (vif == NULL) {
		pr_info("%s not found vif %u\n", __func__, sta->vif_idx);

		return;
	}

	wdev = &vif->wdev;
	while (remain_len > 0) {
		if (remain_len > CLSCSI_REPORT_MSG_FRAG_MAX) {
			send_len = CLSCSI_REPORT_MSG_FRAG_MAX;
			more_frag = 1;
		} else {
			send_len = remain_len;
			more_frag = 0;
		}

		skb = cfg80211_vendor_event_alloc(wiphy, wdev, send_len, CLS_NL80211_CMD_REPORT_CSI, GFP_KERNEL);
		if (skb) {
			nla_put_u32(skb, CLS_NL80211_ATTR_CSI_REPORT_PPDU_ID, report->hdr.ppdu_id);
			nla_put_u8(skb, CLS_NL80211_ATTR_CSI_REPORT_MORE_FRAG, more_frag);
			nla_put_u32(skb, CLS_NL80211_ATTR_CSI_REPORT_FRAG_OFFSET, frag_offset);
			nla_put_u32(skb, CLS_NL80211_ATTR_CSI_REPORT_FRAG_LEN, send_len);
			nla_put(skb, CLS_NL80211_ATTR_CSI_REPORT, send_len, msg_buf + frag_offset);
			cfg80211_vendor_event(skb, GFP_KERNEL);
		}

		frag_offset += send_len;
		remain_len -= send_len;
	}
}

void cls_wifi_process_csi_report(struct cls_wifi_hw *wifi_hw,
		struct cls_csi_report *report)
{
	cls_wifi_dump_csi_report(wifi_hw, report);
	/* Check the STA in monitored STA list */
	if (!wifi_hw->csi_params.save_csi_to_file)
		cls_wifi_vndr_event_report(wifi_hw, report);
}

int cls_wifi_dump_csi_info(struct cls_wifi_hw *wifi_hw)
{
	struct mm_csi_params_req req = {0};
	struct cls_wifi_csi_sta *sta;
	int i;

	req.command = CSI_CMD_DUMP_INFO;

	cls_wifi_send_csi_cmd_req(wifi_hw, &req);
	for (i = 0; i < CLS_CSI_MAX_STA_NUM; i++) {
		if (wifi_hw->csi_params.sta[i].flags & CLS_CSI_STA_ENABLED) {
			sta = &wifi_hw->csi_params.sta[i];
			pr_info("idx %u sta_idx %u flags 0x%x mac_addr %pM\n",
				i, sta->sta_idx, sta->flags, sta->mac_addr);
		}
	}

	return 0;
}

int cls_wifi_enable_csi(struct cls_wifi_hw *wifi_hw, int enable)
{
	struct mm_csi_params_req req = {0};

	pr_info("%s enable %u\n", __func__, enable);
	if (wifi_hw->csi_params.enabled == enable)
		return 0;

	wifi_hw->csi_params.enabled = enable;
	req.command = CSI_CMD_ENABLE;
	req.enable = enable;

	cls_wifi_send_csi_cmd_req(wifi_hw, &req);

	return 0;
}

void cls_wifi_add_associated_csi_sta(struct cls_wifi_hw *wifi_hw, struct cls_wifi_sta *sta)
{
	struct mm_csi_params_req sta_req = {0};

	if (!cls_wifi_is_csi_enabled(wifi_hw))
		return;

	sta_req.command = CSI_CMD_ADD_STA;
	sta_req.sta_idx = sta->sta_idx;
	memcpy(&sta_req.mac_addr[0], sta->mac_addr, ETH_ALEN);
	cls_wifi_send_csi_cmd_req(wifi_hw, &sta_req);
}

int cls_wifi_add_csi_sta(struct cls_wifi_hw *wifi_hw, uint8_t *mac_addr)
{
	struct mm_csi_params_req sta_req = {0};
	struct cls_wifi_sta *sta;
	struct cls_wifi_csi_sta *csi_sta;
	int avail_sta_idx = -1;
	int i;

	pr_info("%s mac_addr %pM\n", __func__, mac_addr);
	for (i = 0; i < CLS_CSI_MAX_STA_NUM; i++) {
		if (wifi_hw->csi_params.sta[i].flags & CLS_CSI_STA_ENABLED) {
			if (memcmp(wifi_hw->csi_params.sta[i].mac_addr , mac_addr, ETH_ALEN) == 0) {
				pr_info("%s repeated sta %pM\n", __func__, mac_addr);

				return -1;
			}
		} else {
			if (avail_sta_idx < 0)
				avail_sta_idx = i;
		}
	}

	if (avail_sta_idx < 0)
		return -1;

	/* Get STA from associated STA list */
	sta = cls_wifi_get_sta_from_mac(wifi_hw, mac_addr);
	/* get available STA index */
	csi_sta = &wifi_hw->csi_params.sta[avail_sta_idx];
	csi_sta->flags |= CLS_CSI_STA_ENABLED;
	if (sta) {
		csi_sta->flags |= CLS_CSI_STA_ASSOCIATED;
		csi_sta->sta = sta;
		sta_req.sta_idx = sta->sta_idx;
		pr_info("%s associated flags %x sta idx %u\n", __func__, csi_sta->flags, sta->sta_idx);
	} else {
		sta_req.sta_idx = 0x3FF;
		pr_info("%s non-associated\n", __func__);
	}
	memcpy(csi_sta->mac_addr, mac_addr, ETH_ALEN);

	sta_req.command = CSI_CMD_ADD_STA;
	/* inform WPU */
	memcpy(&sta_req.mac_addr[0], mac_addr, ETH_ALEN);
	cls_wifi_send_csi_cmd_req(wifi_hw, &sta_req);

	return 0;
}

int cls_wifi_remove_csi_sta(struct cls_wifi_hw *wifi_hw, uint8_t *mac_addr)
{
	struct mm_csi_params_req sta_req = {0};
	struct cls_wifi_csi_sta *csi_sta;
	int i;

	pr_info("%s mac_addr %pM\n", __func__, mac_addr);
	for (i = 0; i < CLS_CSI_MAX_STA_NUM; i++) {
		if (wifi_hw->csi_params.sta[i].flags & CLS_CSI_STA_ENABLED) {
			if (memcmp(wifi_hw->csi_params.sta[i].mac_addr , mac_addr, ETH_ALEN) == 0) {
				pr_info("%s found sta %pM\n", __func__, mac_addr);

				break;
			}
		}
	}

	if (i >= CLS_CSI_MAX_STA_NUM)
		return -1;

	/* inform WPU */
	csi_sta = &wifi_hw->csi_params.sta[i];
	sta_req.sta_idx = csi_sta->sta_idx;
	sta_req.hw_key_idx = csi_sta->hw_key_idx;
	sta_req.command = CSI_CMD_REMOVE_STA;
	memcpy(&sta_req.mac_addr[0], mac_addr, ETH_ALEN);
	cls_wifi_send_csi_cmd_req(wifi_hw, &sta_req);
	csi_sta->flags &= ~CLS_CSI_STA_ENABLED;

	return 0;
}

int cls_wifi_get_csi_sta_macs(struct wiphy *wiphy, struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_csi_sta *csi_sta;
	struct sk_buff *skb_msg;
	struct nlattr *ptemp;
	int rc;
	int i;

	/* Format reply message */
	skb_msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 1024);
	if (!skb_msg)
		return -ENOMEM;

	ptemp = nla_nest_start_noflag(skb_msg, CLS_NL80211_ATTR_CSI_STA_MACS);
	for (i = 0; i < CLS_CSI_MAX_STA_NUM; i++) {
		csi_sta = &wifi_hw->csi_params.sta[i];
		if (!(csi_sta->flags & CLS_CSI_STA_ENABLED))
			continue;

		if (nla_put(skb_msg, i, ETH_ALEN, csi_sta->mac_addr))
			return -1;
		pr_warn("%s() i=%d mac=%02x:%02x\n",
			__func__, i, csi_sta->mac_addr[4], csi_sta->mac_addr[5]);
	}

	nla_nest_end(skb_msg, ptemp);

	rc = cfg80211_vendor_cmd_reply(skb_msg);

	return rc;
}

int cls_wifi_set_csi_period(struct cls_wifi_hw *wifi_hw, int period)
{
	struct mm_csi_params_req csi_req = {0};

	pr_info("%s period %u %u\n", __func__, period, wifi_hw->csi_params.csi_report_period);

	if (wifi_hw->csi_params.csi_report_period == period)
		return -1;

	wifi_hw->csi_params.csi_report_period = period;
	csi_req.command = CSI_CMD_SET_PERIOD;
	csi_req.period = period;
	cls_wifi_send_csi_cmd_req(wifi_hw, &csi_req);

	return 0;
}

int cls_wifi_set_csi_format(struct cls_wifi_hw *wifi_hw, uint32_t format)
{
	struct mm_csi_params_req csi_req = {0};

	pr_info("%s format 0x%x 0x%x\n", __func__, format, wifi_hw->csi_params.format_mask);

	if (wifi_hw->csi_params.format_mask == format)
		return -1;

	wifi_hw->csi_params.format_mask = format;
	csi_req.command = CSI_CMD_SET_FORMAT;
	csi_req.format_mask = format;
	cls_wifi_send_csi_cmd_req(wifi_hw, &csi_req);

	return 0;
}

int cls_wifi_set_csi_log_level(struct cls_wifi_hw *wifi_hw, int log_level)
{
	struct mm_csi_params_req csi_req = {0};

	pr_info("%s log level %u -> %u\n",
		__func__, wifi_hw->csi_params.log_level, log_level);
	if (wifi_hw->csi_params.log_level == log_level)
		return -1;

	wifi_hw->csi_params.log_level = log_level;
	csi_req.command = CSI_CMD_LOG_LEVEL;
	csi_req.log_level = log_level;
	cls_wifi_send_csi_cmd_req(wifi_hw, &csi_req);

	return 0;
}

int cls_wifi_set_he_ltf_smooth(struct cls_wifi_hw *wifi_hw, uint8_t smooth)
{
	struct mm_csi_params_req csi_req = {0};

	pr_info("%s set H matrix smooth %d -> %u\n",
		__func__, wifi_hw->csi_params.csi_smooth, smooth);
	if (wifi_hw->csi_params.csi_smooth == smooth)
		return -1;

	wifi_hw->csi_params.csi_smooth = smooth;
	csi_req.command = CSI_CMD_SET_SMOOTH;
	csi_req.smooth = smooth;
	cls_wifi_send_csi_cmd_req(wifi_hw, &csi_req);

	return 0;
}

int cls_wifi_set_save_file(struct cls_wifi_hw *wifi_hw, uint8_t save_csi_to_file)
{
	pr_info("%s save_csi_to_file %d -> %u\n",
		__func__, wifi_hw->csi_params.save_csi_to_file, save_csi_to_file);
	if (wifi_hw->csi_params.save_csi_to_file == save_csi_to_file)
		return -1;

	wifi_hw->csi_params.save_csi_to_file = save_csi_to_file;

	return 0;
}

int cls_wifi_csi_init(struct cls_wifi_hw *wifi_hw)
{
	pr_info("%s\n", __func__);
	memset(&wifi_hw->csi_params, 0, sizeof(wifi_hw->csi_params));

	wifi_hw->csi_params.format_mask = 0x1FF;
	wifi_hw->csi_params.csi_report_period = 10;
	wifi_hw->csi_params.csi_smooth = 1;

	return 0;
}

int clsemi_vndr_cmds_csi_set(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *wifi_hw = NULL;
	int tb_size = (CLS_NL80211_ATTR_MAX + 1) * sizeof(struct nlattr*);
	struct nlattr **tb = (struct nlattr **)kmalloc(tb_size, GFP_ATOMIC);
	uint8_t enable;
	uint8_t enable_non_assoc;
	uint8_t csi_period;
	uint8_t csi_smooth;
	int ret = 0;
	int rc;

	if(tb == NULL) {
		pr_warn("%s tb: %px\n", __func__,tb);

		return -1;
	}

	wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid\n", __func__);
		if(tb != NULL)
			kfree(tb);

		return rc;
	}

	if(tb[CLS_NL80211_ATTR_CSI_ENABLE]) {
		enable = nla_get_u8(tb[CLS_NL80211_ATTR_CSI_ENABLE]);
		pr_warn("%s tb[CLS_NL80211_ATTR_CSI_ENABLE] = %u\n", __func__, enable);
		cls_wifi_enable_csi(wifi_hw, enable);
	} else if (tb[CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA]) {
		enable_non_assoc = nla_get_u8(tb[CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA]);
		pr_warn("%s tb[CLS_NL80211_ATTR_CSI_NON_ASSOC_STA] = %u\n", __func__, enable_non_assoc);
		wifi_hw->csi_params.enable_non_assoc = enable_non_assoc;
	} else if (tb[CLS_NL80211_ATTR_CSI_REPORT_PERIOD]) {
		csi_period = nla_get_u16(tb[CLS_NL80211_ATTR_CSI_REPORT_PERIOD]);
		pr_warn("%s tb[CLS_NL80211_ATTR_CSI_REPORT_PERIOD] = %u\n", __func__, csi_period);
		cls_wifi_set_csi_period(wifi_hw, csi_period);
	} else if (tb[CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH]) {
		csi_smooth = nla_get_u8(tb[CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH]);
		pr_warn("%s tb[CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH] = %u\n", __func__, csi_smooth);
		wifi_hw->csi_params.csi_smooth = csi_smooth;
	} else {
		ret = -1;
	}

	if(tb != NULL)
		kfree(tb);

	return ret;
}

int clsemi_vndr_cmds_csi_get(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *wifi_hw = NULL;
	int ret = 0;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	pr_warn("%s()\n\n", __func__);

	wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATTR_ID]) {
		pr_warn("%s Attr %d is missed\n", __func__, CLS_NL80211_ATTR_ATTR_ID);
		return -1;
	}

	switch (nla_get_u32(tb[CLS_NL80211_ATTR_ATTR_ID])) {
	case CLS_NL80211_ATTR_CSI_ENABLE:
		pr_warn("%s enable = %u\n", __func__, wifi_hw->csi_params.enabled);
		ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_CSI_ENABLE,
			sizeof(wifi_hw->csi_params.enabled), &wifi_hw->csi_params.enabled);
		break;

	case CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA:
		pr_warn("%s enable_non_assoc %u\n", __func__, wifi_hw->csi_params.enable_non_assoc);
		ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA,
			sizeof(wifi_hw->csi_params.enable_non_assoc), &wifi_hw->csi_params.enable_non_assoc);
		break;

	case CLS_NL80211_ATTR_CSI_REPORT_PERIOD:
		pr_warn("%s enable_non_assoc %u\n", __func__, wifi_hw->csi_params.csi_report_period);
		ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_CSI_REPORT_PERIOD,
			sizeof(wifi_hw->csi_params.csi_report_period), &wifi_hw->csi_params.csi_report_period);
		break;

	case CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH:
		pr_warn("%s csi_smooth %u\n", __func__, wifi_hw->csi_params.csi_smooth);
		ret = clsm_cfg80211_cmd_reply(wiphy, CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH,
			sizeof(wifi_hw->csi_params.csi_smooth), &wifi_hw->csi_params.csi_smooth);
		break;

	case CLS_NL80211_ATTR_CSI_STA_MACS:
		pr_warn("%s CLS_NL80211_ATTR_CSI_STA_MACS\n", __func__);
		ret = cls_wifi_get_csi_sta_macs(wiphy, wifi_hw);
		break;

	default:
		ret = -1;
		pr_warn("%s unknown attr = %u\n", __func__, nla_get_u32(tb[CLS_NL80211_ATTR_ATTR_ID]));
	}

	return ret;
}

int clsemi_vndr_cmds_add_csi_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *wifi_hw = NULL;
	int ret = 0;
	int rc;
	int tb_size = (CLS_NL80211_ATTR_MAX + 1) * sizeof(struct nlattr*);
	struct nlattr **tb = (struct nlattr **)kmalloc(tb_size, GFP_ATOMIC);
	uint8_t mac_addr[6];

	if(tb == NULL) {
		pr_warn("%s tb: %px\n", __func__,tb);

		return -1;
	}

	wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid\n", __func__);
		if(tb != NULL)
			kfree(tb);

		return rc;
	}

	if(tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
		pr_warn("%s tb[CLS_NL80211_ATTR_MAC_ADDR] = %pM\n", __func__, mac_addr);
		cls_wifi_add_csi_sta(wifi_hw, mac_addr);
	} else {
		ret = -1;
	}

	if(tb != NULL)
		kfree(tb);

	return ret;
}

int clsemi_vndr_cmds_del_csi_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *wifi_hw = NULL;
	int ret = 0;
	int rc;
	int tb_size = (CLS_NL80211_ATTR_MAX + 1) * sizeof(struct nlattr*);
	struct nlattr **tb = (struct nlattr **)kmalloc(tb_size, GFP_ATOMIC);
	uint8_t mac_addr[6];

	if(tb == NULL) {
		pr_warn("%s tb: %px\n", __func__,tb);

		return -1;
	}

	wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid\n", __func__);
		if(tb != NULL)
			kfree(tb);

		return rc;
	}

	if(tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		ether_addr_copy(mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));
		pr_warn("%s tb[CLS_NL80211_ATTR_MAC_ADDR] = %pM\n", __func__, mac_addr);
		cls_wifi_remove_csi_sta(wifi_hw, mac_addr);
	} else {
		ret = -1;
	}

	if(tb != NULL)
		kfree(tb);

	return ret;
}

