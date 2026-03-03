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

#include <linux/dma-mapping.h>
#include <linux/ieee80211.h>
#include <linux/etherdevice.h>
#include <net/ieee80211_radiotap.h>
#include <net/sock.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_rx.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_prof.h"
#include "ipc_host.h"
#include "cls_wifi_utils.h"
#include "cls_wifi_events.h"
#include "cls_wifi_compat.h"
#include "vendor.h"
#include "nac.h"

#ifdef CONFIG_CLS_3ADDR_BR
skb_rx_handler_t cls_3addr_br_rx_callback = NULL;
EXPORT_SYMBOL(cls_3addr_br_rx_callback);
#endif

extern struct cls_wifi_anti_attack cls_anti_attack;
struct vendor_radiotap_hdr {
	u8 oui[3];
	u8 subns;
	u16 len;
	u8 data[];
};

extern uint32_t process_logs;
int cls_wifi_sta_snr_record(struct cls_wifi_hw *cls_wifi_hw,struct cls_wifi_sta *sta,u16 *snr,u8 format,u8 nss,u8 mcs,u8 gi,u8 bw);

/**
 * cls_wifi_rx_statistic - save some statistics about received frames
 *
 * @cls_wifi_hw: main driver data.
 * @hw_rxhdr: Rx Hardware descriptor of the received frame.
 * @sta: STA that sent the frame.
 */
static void cls_wifi_rx_statistic(struct cls_wifi_hw *cls_wifi_hw, struct hw_rxhdr *hw_rxhdr,
							  struct cls_wifi_sta *sta)
{
	struct cls_wifi_stats *stats = &cls_wifi_hw->stats;
#ifdef CONFIG_CLS_WIFI_DEBUGFS
	struct cls_wifi_rate_stats *rate_stats = &sta->stats.rx_rate;
	struct rx_vector_1 *rxvect = &hw_rxhdr->hwvect.rx_vect1;
	int mpdu, ampdu, mpdu_prev, rate_idx;
	int mcs = 0, bw = 0, gi = 0, nss = 0, preamble = 0;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, sta->vif_idx);
	u32 rate_config;
	u8 format_mod;
	int i;

	/* update ampdu rx stats */
	mpdu = hw_rxhdr->hwvect.mpdu_cnt;
	ampdu = hw_rxhdr->hwvect.ampdu_cnt;
	ampdu = ampdu & (RXMAP_MAX - 1);
	mpdu_prev = stats->ampdus_rx_map[ampdu];

	/* work-around, for MACHW that incorrectly return 63 for last MPDU of A-MPDU or S-MPDU */
	if (mpdu == 63) {
		if (ampdu == stats->ampdus_rx_last)
			mpdu = mpdu_prev + 1;
		else
			mpdu = 0;
	}

	if (ampdu != stats->ampdus_rx_last) {
		stats->ampdus_rx[mpdu_prev]++;
		stats->ampdus_rx_miss += mpdu;
	} else {
		if (mpdu <= mpdu_prev) {
			/* lost 4 (or a multiple of 4) complete A-MPDU/S-MPDU */
			stats->ampdus_rx_miss += mpdu;
		} else {
			stats->ampdus_rx_miss += mpdu - mpdu_prev - 1;
		}
	}

	stats->ampdus_rx_map[ampdu] = mpdu;
	stats->ampdus_rx_last = ampdu;

	/* update rx rate statistic */
	if (!rate_stats->size)
		return;

	bw = rxvect->ch_bw;
	format_mod = rxvect->format_mod;
	switch (rxvect->format_mod) {
		case FORMATMOD_NON_HT:
		case FORMATMOD_NON_HT_DUP_OFDM:
			mcs = legrates_lut[rxvect->leg_rate].idx;
			preamble = rxvect->pre_type;
			break;
		case FORMATMOD_HT_MF:
		case FORMATMOD_HT_GF:
			mcs = rxvect->ht.mcs % 8;
			nss = rxvect->ht.mcs / 8;
			gi = rxvect->ht.short_gi;
			break;
		case FORMATMOD_VHT:
			mcs = rxvect->vht.mcs;
			nss = rxvect->vht.nss;
			gi = rxvect->vht.short_gi;
			break;
		case FORMATMOD_HE_SU:
			mcs = rxvect->he.mcs;
			nss = rxvect->he.nss;
			gi = rxvect->he.gi_type;
			break;
		case FORMATMOD_HE_MU:
			mcs = rxvect->he.mcs;
			nss = rxvect->he.nss;
			gi = rxvect->he.gi_type;
			bw = rxvect->he.ru_size;
			break;
		case FORMATMOD_HE_ER:
			mcs = rxvect->he.mcs;
			gi = rxvect->he.gi_type;
			bw = rxvect->he.ru_size;
			break;
#ifdef CONFIG_CLS_WIFI_MACHW_HE_AP
		case FORMATMOD_HE_TB:
			mcs = rxvect->he_tb.mcs;
			nss = rxvect->he_tb.nss;
			gi = rxvect->he_tb.gi_type;
			bw = rxvect->he_tb.ru_size;
			break;
#endif
		case FORMATMOD_OFFLOAD:
			rate_config = rxvect->offload_vector1.rate_config;
			format_mod = RC_RATE_GET(FORMAT_MOD, rate_config);
			bw = RC_RATE_GET(BW, rate_config);
			mcs = RC_RATE_GET(MCS, rate_config);
			nss = RC_RATE_GET(NSS, rate_config);
			gi = RC_RATE_GET(GI, rate_config);
			break;
#if defined(CFG_MERAK3000)
		case FORMATMOD_EHT_MU_SU:
			mcs = rxvect->he.mcs;
			nss = rxvect->he.nss;
			gi = rxvect->he.gi_type;
			break;
		case FORMATMOD_EHT_TB:
			mcs = rxvect->he_tb.mcs;
			nss = rxvect->he_tb.nss;
			gi = rxvect->he_tb.gi_type;
			break;
#endif
		default:
			break;
	}
	rate_idx = cls_wifi_dbgfs_rate_idx(format_mod, bw, mcs, gi, nss, preamble);

	if ((rate_idx < 0) || rate_idx > (rate_stats->size)) {
		wiphy_err(cls_wifi_hw->wiphy, "RX: Invalid index conversion: format_mod=%d mcs=%d bw=%d sgi=%d nss=%d\n",
				  format_mod, mcs, bw, gi, nss);
	} else {
		if (!rate_stats->table[rate_idx])
			rate_stats->rate_cnt++;
		rate_stats->table[rate_idx]++;
		rate_stats->cpt++;
	}
#endif

	if (hw_rxhdr->pattern == RX_DMA_OVER_PATTERN_LST) {
		struct rx_vector_2 *rx_vect2;

		rx_vect2 = &hw_rxhdr->rx_vect2;
		sta->stats.last_hw_rx_vect2 = *rx_vect2;
		if(sta->stats.sinr_cnt > 100000 || sta->stats.sinr_cnt == 0) {
			memset(sta->stats.sinr_total, 0, sizeof(sta->stats.sinr_total));
			sta->stats.sinr_cnt = 0;
		}
		for (i = 0; i < 16; i++) {
			sta->stats.sinr_total[i][0] += sta->stats.last_hw_rx_vect2.sinr[i] & 0xFF;
			sta->stats.sinr_total[i][1] += (sta->stats.last_hw_rx_vect2.sinr[i] >> 8) & 0xFF;
		}
		sta->stats.sinr_cnt++;
		sta->stats.hw_rx_vect2_valid = true;
		cls_wifi_sta_snr_record(cls_wifi_hw, sta, &sta->stats.last_hw_rx_vect2.sinr[0],
						format_mod,nss,mcs,gi,bw);
	} else {
		sta->stats.hw_rx_vect2_valid = false;
	}
	/* Always save complete hwvect */
	sta->stats.last_rx = hw_rxhdr->hwvect;

	sta->stats.rx_pkts ++;
	sta->stats.rx_bytes += hw_rxhdr->hwvect.len;
	sta->stats.last_act = jiffies;
	cls_wtm_update_stats(cls_wifi_hw, vif, sta, hw_rxhdr->hwvect.len, rxvect->rssi1, true);
}

#ifdef CONFIG_CLS_FWT
void cls_wifi_rx_update_skb_sub_port(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
                            struct sk_buff *skb,  u32 rxhdr_flags)
{
	int sta_idx;
	u32 sub_port = 0;
	u16 node_idx, vif_idx;

	sta_idx = RX_FLAGS_GET(STA_IDX, rxhdr_flags);
	if (sta_idx != CLS_WIFI_INVALID_STA && sta_idx < hw_remote_sta_max(cls_wifi_hw)) {
		struct ethhdr *eth_header = (struct ethhdr *)skb->data;
		struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[sta_idx];
		bool is_wds;
		uint8_t entry_type = CLS_FWT_ENTRY_TYPE_WIRELESS_INVALID;

		node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, sta_idx);
		node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

		vif_idx = CLS_IEEE80211_VIF_IDX_MAP(cls_wifi_vif->vif_index);

		if(memcmp(eth_header->h_source, sta->mac_addr, ETH_ALEN))
			is_wds = 1;
		else
			is_wds = 0;

		if (cls_wifi_vif->wdev.iftype == NL80211_IFTYPE_AP)
			entry_type = is_wds ? CLS_FWT_ENTRY_TYPE_WDS_IN_AP_MODE : CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE;
		else if (cls_wifi_vif->wdev.iftype == NL80211_IFTYPE_STATION)
			entry_type = is_wds ? CLS_FWT_ENTRY_TYPE_WDS_IN_STA_MODE : CLS_FWT_ENTRY_TYPE_AP_IN_STA_MODE;

		sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(node_idx, vif_idx, entry_type,
							RX_FLAGS_GET(4_ADDR, rxhdr_flags));
	}

	skb->src_port = sub_port;
	if (cls_wifi_vif->log_enable)
		pr_warn("%s %d src_port 0x%x\n", __func__, __LINE__, skb->src_port);
}
#endif

/**
 * cls_wifi_rx_defer_skb - Defer processing of a SKB
 *
 * @cls_wifi_hw: main driver data
 * @cls_wifi_vif: vif that received the buffer
 * @skb: buffer to defer
 */
void cls_wifi_rx_defer_skb(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
					   struct sk_buff *skb)
{
	struct cls_wifi_defer_rx_cb *rx_cb = (struct cls_wifi_defer_rx_cb *)skb->cb;

	// for now don't support deferring the same buffer on several interfaces
	if (skb_shared(skb))
		return;

	// Increase ref count to avoid freeing the buffer until it is processed
	skb_get(skb);

	rx_cb->vif = cls_wifi_vif;
	skb_queue_tail(&cls_wifi_hw->defer_rx.sk_list, skb);
	schedule_work(&cls_wifi_hw->defer_rx.work);
}

int dev_xmit_bypass_qdisc(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	const struct net_device_ops *ops = dev->netdev_ops;

	skb_reset_mac_header(skb);

	if (ops->ndo_start_xmit(skb, dev) != NETDEV_TX_OK)
		return NET_XMIT_DROP;

	return NET_XMIT_SUCCESS;
}

void cls_wifi_rx_dfx_stats(struct cls_wifi_vif *vif, struct cls_wifi_sta *sta, bool bc, bool mc)
{
	if (bc) {
		vif->dfx_stats.rx_broadcast++;
		if (sta && sta->valid)
			sta->dfx_stats.rx_broadcast++;
	} else if (mc) {
		vif->dfx_stats.rx_multicast++;
		if (sta && sta->valid)
			sta->dfx_stats.rx_multicast++;
	} else {
		vif->dfx_stats.rx_unicast++;
		if (sta && sta->valid)
			sta->dfx_stats.rx_unicast++;
	}
}

#ifdef CONFIG_CLS_3ADDR_BR
static bool cls_wifi_rx_skb_check_bounce(struct cls_wifi_vif *cls_wifi_vif, struct ethhdr *eth)
{
	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_STATION) {
		if (unlikely(is_multicast_ether_addr(eth->h_dest))) {
			if (!memcmp(eth->h_source, cls_wifi_vif->ndev->dev_addr, ETH_ALEN)) {
					pr_debug("wifi rx %pM > %pM proto 0x%x, should drop bounce pkt\n",
						eth->h_source, eth->h_dest, ntohs(eth->h_proto));
				return true;
			}
		}
	}
	return false;
}
#endif

/**
 * cls_wifi_rx_data_skb - Process one data frame
 *
 * @cls_wifi_hw: main driver data
 * @vlan_vif: vif that received the buffer
 * @skb: skb received
 * @rxhdr: HW rx descriptor
 * @return Number of buffer processed (can only be >1 for A_MSDU)
 *
 * If buffer is an A-MSDU, then each subframe is added in a list of skb
 * (and A-MSDU header is converted to ethernet header)
 * Then each skb may be:
 * - forwarded to upper layer
 * - resent on wireless interface
 *
 * When vif is a STA interface, every skb is only forwarded to upper layer.
 * When vif is an AP interface, multicast skb are forwarded and resent, whereas
 * skb for other BSS's STA are only resent.
 *
 * Whether it has been forwarded and/or resent the skb is always consumed
 * and as such it shall no longer be used after calling this function.
 */
static int cls_wifi_rx_data_skb(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vlan_vif,
		struct cls_wifi_sta *sta, struct sk_buff *skb, struct hw_rxhdr *rxhdr,
		u8 cmn_rxbuf_flag, unsigned int vif_idx)
{
	struct cls_wifi_vif *cls_wifi_vif = cls_wifi_get_master_vif(vlan_vif);
	struct sk_buff_head list;
	struct sk_buff *rx_skb;
	/* rxhdr may be freed after forwarding */
	u32 rxhdr_flags = rxhdr->flags;
	bool amsdu = RX_FLAGS_GET(AMSDU, rxhdr_flags);
	bool resend = false, resend_multi = false, forward = true, g_forward = true, is_m2u = false;
	int skip_after_eth_hdr = 0;
	int res = 1;
	u32 len_offset;
	struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_hw->plat->cmn_hw;
	u8 radio_idx = cls_wifi_hw->radio_idx;
	struct device *dev;
#ifdef CONFIG_CLS_FWT
	/* XXX: remember to parse SKB to get the correct VLAN ID if you care about VLAN ID in this function */
	u16 vlan_id = 0;
	struct vlan_ethhdr *vhdr;
#endif
#ifdef CONFIG_CLS_3ADDR_BR
	int ret = 0;
#endif

	if (cmn_rxbuf_flag) {
		len_offset = cmn_hw->ipc_host_cmn_env->rxbuf_sz - sizeof(struct hw_rxhdr);
		dev = cmn_hw->dev;
	} else {
		len_offset = cls_wifi_hw->ipc_env->rxbuf_sz - sizeof(struct hw_rxhdr);
		dev = cls_wifi_hw->dev;
	}

	skb->dev = cls_wifi_vif->ndev;

	__skb_queue_head_init(&list);

	if (amsdu) {
		int count = 0;
		u32 hostid;

		if (!rxhdr->amsdu_len[0] || (rxhdr->amsdu_len[0] > len_offset))
			g_forward = false;
		else
			skb_put(skb, rxhdr->amsdu_len[0]);
		__skb_queue_tail(&list, skb);

		while ((count < ARRAY_SIZE(rxhdr->amsdu_hostids)) &&
			   (hostid = rxhdr->amsdu_hostids[count++])) {
			struct cls_wifi_ipc_buf *ipc_buf;
			if (cmn_rxbuf_flag) {
				ipc_buf = cls_wifi_cmn_ipc_rxbuf_from_hostid(cmn_hw, hostid, radio_idx, 0);
			} else {
				ipc_buf = cls_wifi_ipc_rxbuf_from_hostid(cls_wifi_hw, hostid);
			}

			if (!ipc_buf) {
				wiphy_err(cls_wifi_hw->wiphy, "Invalid hostid 0x%x for A-MSDU subframe\n",
						  hostid);
				continue;
			}

			rx_skb = ipc_buf->addr;
			// Index for amsdu_len is different (+1) than the one for amsdu_hostids
			if (!rxhdr->amsdu_len[count] || (rxhdr->amsdu_len[count] > len_offset))
				g_forward = false;
			else
				skb_put(rx_skb, rxhdr->amsdu_len[count]);
			rx_skb->priority = skb->priority;
			rx_skb->dev = skb->dev;
			__skb_queue_tail(&list, rx_skb);
			cls_wifi_ipc_buf_e2a_release(cls_wifi_hw, ipc_buf);
			res++;
		}

		cls_wifi_hw->stats.amsdus_rx[count - 1]++;
		if (!g_forward) {
			printk(KERN_DEBUG "%s amsdu_len[%u,%u,%u,%u,%u,%u,%u,%u],amsdu_hostids[%u,%u,%u,%u,%u,%u,%u]\n",
				"A-MSDU truncated, skip it!!!", rxhdr->amsdu_len[0], rxhdr->amsdu_len[1],
				rxhdr->amsdu_len[2], rxhdr->amsdu_len[3], rxhdr->amsdu_len[4],
				rxhdr->amsdu_len[5], rxhdr->amsdu_len[6], rxhdr->amsdu_len[7],
				rxhdr->amsdu_hostids[0], rxhdr->amsdu_hostids[1], rxhdr->amsdu_hostids[2],
				rxhdr->amsdu_hostids[3], rxhdr->amsdu_hostids[4], rxhdr->amsdu_hostids[5],
				rxhdr->amsdu_hostids[6]);
			goto start_forward;
		}
	} else {
		u32 frm_len = le32_to_cpu(rxhdr->hwvect.len);

		__skb_queue_tail(&list, skb);
		cls_wifi_hw->stats.amsdus_rx[0]++;

		if (frm_len > len_offset) {
			printk(KERN_DEBUG "%s %d frm_len %d len_offset %d\n",
					__func__, __LINE__, frm_len, len_offset);
			printk(KERN_DEBUG "MSDU truncated, skip it\n");
			cls_wifi_hw->stats.rx_msdu_truncated += 1;
			g_forward = false;
			goto start_forward;
		}
		skb_put(skb, le32_to_cpu(rxhdr->hwvect.len));
	}

start_forward:

	is_m2u = ((CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP) ||
			 (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP_VLAN) ||
			 (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_P2P_GO)) &&
			!(cls_wifi_vif->ap.flags & CLS_WIFI_AP_ISOLATE);

	while (!skb_queue_empty(&list)) {
		int dst_sta_idx;
		struct ethhdr *eth;

		dst_sta_idx = RX_FLAGS_GET(DST_STA_IDX, rxhdr_flags);
		rx_skb = __skb_dequeue(&list);
		skb_reset_mac_header(rx_skb);
		eth = eth_hdr(rx_skb);

		resend_multi = false;
		resend = false;
		forward = true;

#ifdef CONFIG_CLS_3ADDR_BR
		if (cls_wifi_rx_skb_check_bounce(cls_wifi_vif, eth)) {
			dev_kfree_skb(rx_skb);
			/* fix sit #8873, need return handled pkt number.
			 * then upper layer will release buf to wpu.
			 * wpu disable rx IRQ when no available rx buff to use
			 * 3/4 address repeater both meet this problem
			 */
			continue;
		}
#endif

		/* if A-MSDU/MSDU truncated, all packets will not be forwarded and be directly released */
		if (!g_forward) {
			dev_kfree_skb(rx_skb);
			continue;
		}

		/* m2u */
		if (is_m2u) {
			if (unlikely(is_multicast_ether_addr(eth->h_dest))) {
				/* multi_pkt need to be forwared to upper layer and resent on wireless interface */
				resend_multi = true;
			}
		}  else if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_MESH_POINT) {
			/* The following process has not been verified */
			if (dst_sta_idx != CLS_WIFI_INVALID_STA) {
				resend = true;

				if (is_multicast_ether_addr(eth->h_dest)) {
					// MC/BC frames are uploaded with mesh control and LLC/snap
					// (so they can be mesh forwarded) that need to be removed.
					uint8_t *mesh_ctrl = (uint8_t *)(eth + 1);

					skip_after_eth_hdr = 8 + 6;

					if ((*mesh_ctrl & MESH_FLAGS_AE) == MESH_FLAGS_AE_A4)
						skip_after_eth_hdr += ETH_ALEN;
					else if ((*mesh_ctrl & MESH_FLAGS_AE) == MESH_FLAGS_AE_A5_A6)
						skip_after_eth_hdr += 2 * ETH_ALEN;
				} else {
					forward = false;
				}
			}
		}

		/* resend pkt(multi) on wireless interface */
		if (resend || resend_multi) {
			struct sk_buff *skb_copy;
			/* always need to copy buffer even when forward=0 to get enough headrom for txdesc */
			skb_copy = skb_copy_expand(rx_skb, CLS_WIFI_TX_MAX_HEADROOM, 0, GFP_ATOMIC);
			if (skb_copy) {
				int rest;
				skb_copy->protocol = htons(ETH_P_802_3);
				skb_reset_network_header(skb_copy);
				skb_reset_mac_header(skb_copy);

#ifdef CONFIG_CLS_FWT
				if (cls_wifi_vif->log_enable)
					pr_warn("%s %d 4addr %d m2u3 %d auto4 %d\n", __func__, __LINE__,
							RX_FLAGS_GET(4_ADDR, rxhdr_flags),
							cls_wifi_vif->m2u_3addr_resend,
							cls_wifi_vif->auto_4addr);

				if (resend || (resend_multi && ((!RX_FLAGS_GET(4_ADDR, rxhdr_flags) &&
						cls_wifi_vif->m2u_3addr_resend) || (RX_FLAGS_GET(4_ADDR, rxhdr_flags) &&
						cls_wifi_vif->auto_4addr))))
					cls_wifi_rx_update_skb_sub_port(cls_wifi_hw, cls_wifi_vif, skb_copy, rxhdr_flags);
#endif
				cls_wifi_vif->is_resending = true;
				if (cls_wifi_get_vif(cls_wifi_hw, vif_idx)) {
					skb_copy->dev = cls_wifi_vif->ndev;
					rest = dev_queue_xmit(skb_copy);
					cls_wifi_vif->is_resending = false;
					/* note: buffer is always consummed by dev_queue_xmit */
					if (rest == NET_XMIT_DROP) {
						cls_wifi_vif->net_stats.rx_dropped++;
						cls_wifi_vif->net_stats.tx_dropped++;
					} else if (rest != NET_XMIT_SUCCESS) {
						netdev_err(cls_wifi_vif->ndev,
							   "Failed to re-send buffer to driver (rest=%d)", rest);
						cls_wifi_vif->net_stats.tx_errors++;
					}
				} else {
					pr_err("%s vif_index: %u\n", __func__, vif_idx);
					dev_kfree_skb_any(skb_copy);
				}
			} else {
				netdev_err(cls_wifi_vif->ndev, "Failed to copy skb");
			}
		}

		/* forward pkt to upper layer */
		if (forward) {
#ifdef CONFIG_CLS_FWT
			static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);
			uint16_t smac_index, dmac_index;
			struct net_device *smac_outdev = NULL;
			struct net_device *dmac_outdev = NULL;
			u32 s_subport, d_subport;
			bool skb_forwarded = false;
#endif
			bool is_bc = is_broadcast_ether_addr(eth->h_dest);
			bool is_mc = is_multicast_ether_addr(eth->h_dest);
			int len = rx_skb->len;
			int rest;

			if (cls_wifi_vif->dump_pkt) {
				pr_warn("receive data frame on %s\n", cls_wifi_vif->ndev->name);
				print_hex_dump(KERN_WARNING, "RX-SKB: ", DUMP_PREFIX_OFFSET, 16, 1, rx_skb->data, rx_skb->len, false);
			}

			if(NULL != cls_wifi_vif->ndev_fp)
			{
				rx_skb->dev = cls_wifi_vif->ndev_fp;
				if (cls_wifi_vif->fp_bypass_qdisc)
					rest = dev_xmit_bypass_qdisc(rx_skb);
				else
					rest = dev_queue_xmit(rx_skb);

				if(NET_XMIT_SUCCESS == rest)
				{
					/* Update statistics */
					cls_wifi_vif->net_stats.rx_packets++;
					cls_wifi_vif->net_stats.rx_bytes += len;
					cls_wifi_rx_dfx_stats(cls_wifi_vif, sta, is_bc, is_mc);
					if(process_logs & 0x1)
					netdev_err(cls_wifi_vif->ndev, "Succeed to fp skb, res %d, dev %px\n", rest, cls_wifi_vif->ndev_fp);
				}
				else
				{
					cls_wifi_vif->net_stats.tx_errors++;
					netdev_err(cls_wifi_vif->ndev, "Failed to fp skb, res %d, dev %px\n", rest, cls_wifi_vif->ndev_fp);
				}
			} else {
#ifdef CONFIG_CLS_FWT
				if (cls_fwt_g_enable)
					cls_wifi_rx_update_skb_sub_port(cls_wifi_hw, cls_wifi_vif, rx_skb, rxhdr_flags);

				if (clsemi_fast_fwd_enabled()) {
					if (clsemi_fast_fwd(rx_skb, cls_wifi_vif->ndev, false)) {
						cls_wifi_vif->net_stats.rx_packets++;
						cls_wifi_vif->net_stats.rx_bytes += len;
						cls_wifi_rx_dfx_stats(cls_wifi_vif, sta, is_bc,
								is_mc);
						skb_forwarded = true;
					}
				} else {
					/*
					 * 1. fwt only support unicast
					 * 2. when da/sa existed in fwt
					 * 3. smac's outdev UNCHANGE,
					 * The packets that meet the above conditions can bypass network stack.
					 */
					if (eth->h_proto == htons(ETH_P_8021Q)) {
						vhdr = (struct vlan_ethhdr *)eth;
						vlan_id = ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK;
					}

					if (!vlan_id)
						vlan_id = br_port_get_pvid(cls_wifi_vif->ndev);

					if (!is_multicast_ether_addr(eth->h_dest)
						&& g_get_fwt_entry_info_hook
						&& (smac_outdev = g_get_fwt_entry_info_hook(eth->h_source,
							vlan_id, &smac_index, &s_subport))
						&& (dmac_outdev = g_get_fwt_entry_info_hook(eth->h_dest,
							vlan_id, &dmac_index, &d_subport))
						&& (smac_outdev == cls_wifi_vif->ndev)) {

						rx_skb->dev = dmac_outdev;
						rx_skb->dest_port = d_subport;

						// smac is active, reset smac's timestamp. dmac not
						if (g_reset_fwt_ageing_hook)
							g_reset_fwt_ageing_hook(smac_index);

						if (cls_fwt_dbg_enable)
							pr_info("Forward Packet([%pM] (%s) -> [%pM]) to %s\n",
									eth->h_source, cls_wifi_vif->ndev->name,
									eth->h_dest, dmac_outdev->name);

						skb_reset_network_header(rx_skb);
						skb_reset_mac_header(rx_skb);
						//TODO: which function can work better
						//1. .ndo_select_queue + .ndo_start_xmit
						//2. dev_queue_xmit;
						rest = dev_queue_xmit(rx_skb);
						if (rest != NET_XMIT_SUCCESS) {
							if (__ratelimit(&rl))
								pr_err("[%s] FWT FAILED(%d):  [mac=%pM vid=%d] to %s\n",
									__func__, rest,
									eth->h_dest, vlan_id, dmac_outdev->name);
									cls_wifi_vif->net_stats.rx_errors++;
						} else {
							/* Update statistics */
							//TODO: fwt counter
							cls_wifi_vif->net_stats.rx_packets++;
							cls_wifi_vif->net_stats.rx_bytes += len;
							cls_wifi_rx_dfx_stats(cls_wifi_vif, sta,
									is_bc, is_mc);
						}
						skb_forwarded = true;
					}
				}

				if (!skb_forwarded) {
#endif
					rx_skb->protocol = eth_type_trans(rx_skb, cls_wifi_vif->ndev);
					memset(rx_skb->cb, 0, sizeof(rx_skb->cb));

					// Special case for MESH when BC/MC is uploaded and resend
					if (unlikely(skip_after_eth_hdr)) {
						memmove(skb_mac_header(rx_skb) + skip_after_eth_hdr,
								skb_mac_header(rx_skb), sizeof(struct ethhdr));
						__skb_pull(rx_skb, skip_after_eth_hdr);
						skb_reset_mac_header(rx_skb);
						skip_after_eth_hdr = 0;
					}

					/* Update statistics */
					cls_wifi_vif->net_stats.rx_packets++;
					cls_wifi_vif->net_stats.rx_bytes += len;
					cls_wifi_rx_dfx_stats(cls_wifi_vif, sta, is_bc, is_mc);

					REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_IEEE80211RX);
#ifdef CONFIG_CLS_3ADDR_BR
					/*STA mode && use_4addr = 0 And cls_3addr_br_tx_callback != NULL, hack rx pkt*/
					if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_STATION
							&& !cls_wifi_vif->use_4addr
							&& cls_3addr_br_rx_callback) {
							ret = cls_3addr_br_rx_callback(cls_wifi_vif, rx_skb);
					}
#endif
					netif_receive_skb(rx_skb);
					REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_IEEE80211RX);

#ifdef CONFIG_CLS_FWT
				}
#endif
			}
		} else {
			dev_kfree_skb(rx_skb);
		}
	}

	return res;
}
/**
 * cls_wifi_rx_mgmt - Process one 802.11 management frame
 *
 * @cls_wifi_hw: main driver data
 * @cls_wifi_vif: vif to upload the buffer to
 * @skb: skb received
 * @rxhdr: HW rx descriptor
 *
 * Forward the management frame to a given interface.
 */
static void cls_wifi_rx_mgmt(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
						 struct sk_buff *skb,  struct hw_rxhdr *hw_rxhdr)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	struct rx_vector_1 *rxvect = &hw_rxhdr->hwvect.rx_vect1;

	if (cls_wifi_vif->dump_pkt) {
		pr_warn("receive mgmt frame on %s, fc 0x%x\n", cls_wifi_vif->ndev->name, mgmt->frame_control);
		print_hex_dump(KERN_WARNING, "RX-MGMT: ", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len, false);
	}

	if (ieee80211_is_beacon(mgmt->frame_control)) {
		if ((CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_MESH_POINT) &&
			RX_FLAGS_GET(NEW_MESH_PEER, hw_rxhdr->flags)) {
			cfg80211_notify_new_peer_candidate(cls_wifi_vif->ndev, mgmt->sa,
											   mgmt->u.beacon.variable,
											   skb->len - offsetof(struct ieee80211_mgmt,
																   u.beacon.variable),
											   rxvect->rssi1, GFP_ATOMIC);
		} else {
			cfg80211_report_obss_beacon(cls_wifi_hw->wiphy, skb->data, skb->len,
										hw_rxhdr->phy_info.phy_prim20_freq,
										rxvect->rssi1);
		}
	} else if ((ieee80211_is_deauth(mgmt->frame_control) ||
				ieee80211_is_disassoc(mgmt->frame_control)) &&
			   (mgmt->u.deauth.reason_code == WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA ||
				mgmt->u.deauth.reason_code == WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA)) {
		cfg80211_rx_unprot_mlme_mgmt(cls_wifi_vif->ndev, skb->data, skb->len);
	} else if ((CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_STATION) &&
			   (ieee80211_is_action(mgmt->frame_control) &&
				(mgmt->u.action.category == 6))) {
		// Wpa_supplicant will ignore the FT action frame if reported via cfg80211_rx_mgmt
		// and cannot call cfg80211_ft_event from atomic context so defer message processing
		cls_wifi_rx_defer_skb(cls_wifi_hw, cls_wifi_vif, skb);
#ifdef CONFIG_CLS_VBSS
	} else if (ieee80211_is_assoc_req(mgmt->frame_control)
			|| ieee80211_is_reassoc_req(mgmt->frame_control)) {
		if (cls_wifi_vif->max_assoc_sta == cls_wifi_vif->assoc_sta_count) {
		/* reached limit, not report the assoc request */
		pr_info("assoc num reached limit %u\n", cls_wifi_vif->max_assoc_sta);

		return;
	}
#endif
	} else {
		cfg80211_rx_mgmt(&cls_wifi_vif->wdev, hw_rxhdr->phy_info.phy_prim20_freq,
						 rxvect->rssi1, skb->data, skb->len, 0);
	}
}

/**
 * cls_wifi_rx_mgmt_any - Process one 802.11 management frame
 *
 * @cls_wifi_hw: main driver data
 * @skb: skb received
 * @rxhdr: HW rx descriptor
 *
 * Process the management frame and free the corresponding skb.
 * If vif is not specified in the rx descriptor, the the frame is uploaded
 * on all active vifs.
 */
static void cls_wifi_rx_mgmt_any(struct cls_wifi_hw *cls_wifi_hw, struct sk_buff *skb,
							 struct hw_rxhdr *hw_rxhdr)
{
	struct cls_wifi_vif *cls_wifi_vif;
	int vif_idx = RX_FLAGS_GET(VIF_IDX, hw_rxhdr->flags);
	int sta_idx = RX_FLAGS_GET(STA_IDX, hw_rxhdr->flags);
	u32 frm_len = le32_to_cpu(hw_rxhdr->hwvect.len);
	u32 len_offset = cls_wifi_hw->ipc_env->rxbuf_sz - sizeof(struct hw_rxhdr);

	if (frm_len > len_offset) {
		// frame has been truncated by firmware, skip it
		printk(KERN_DEBUG "%s %d frm_len %d len_offset %d\n",
				__func__, __LINE__, frm_len, len_offset);
		printk(KERN_DEBUG "MMPDU truncated, skip it\n");
		cls_wifi_hw->stats.rx_mmpdu_truncated += 1;
		goto end;
	}

	skb_put(skb, frm_len);
	trace_mgmt_rx(hw_rxhdr->phy_info.phy_prim20_freq, vif_idx,
				  sta_idx, (struct ieee80211_mgmt *)skb->data);

	if (vif_idx == CLS_WIFI_INVALID_VIF) {
		list_for_each_entry(cls_wifi_vif, &cls_wifi_hw->vifs, list) {
			if (! cls_wifi_vif->up)
				continue;
			cls_wifi_rx_mgmt(cls_wifi_hw, cls_wifi_vif, skb, hw_rxhdr);
		}
	} else {
		cls_wifi_vif = cls_wifi_get_vif(cls_wifi_hw, vif_idx);
		if (cls_wifi_vif)
			cls_wifi_rx_mgmt(cls_wifi_hw, cls_wifi_vif, skb, hw_rxhdr);
	}

end:
	dev_kfree_skb(skb);
}

/**
 * cls_wifi_rx_rtap_hdrlen - Return radiotap header length
 *
 * @rxvect: Rx vector used to fill the radiotap header
 * @has_vend_rtap: boolean indicating if vendor specific data is present
 *
 * Compute the length of the radiotap header based on @rxvect and vendor
 * specific data (if any).
 */
static u8 cls_wifi_rx_rtap_hdrlen(struct rx_vector_1 *rxvect,
							  bool has_vend_rtap)
{
	u8 rtap_len;

	/* Compute radiotap header length */
	rtap_len = sizeof(struct ieee80211_radiotap_header) + 8;

	// Check for multiple antennas
	if (hweight32(rxvect->antenna_set) > 1)
		// antenna and antenna signal fields
		rtap_len += 4 * hweight8(rxvect->antenna_set);

	// TSFT
	if (!has_vend_rtap) {
		rtap_len = ALIGN(rtap_len, 8);
		rtap_len += 8;
	}

	// IEEE80211_HW_SIGNAL_DBM
	rtap_len++;

	// Check if single antenna
	if (hweight32(rxvect->antenna_set) == 1)
		rtap_len++; //Single antenna

	// padding for RX FLAGS
	rtap_len = ALIGN(rtap_len, 2);

	// Check for HT frames
	if ((rxvect->format_mod == FORMATMOD_HT_MF) ||
		(rxvect->format_mod == FORMATMOD_HT_GF))
		rtap_len += 3;

	// Check for AMPDU
	if (!(has_vend_rtap) && ((rxvect->format_mod >= FORMATMOD_VHT) ||
							 ((rxvect->format_mod > FORMATMOD_NON_HT_DUP_OFDM) &&
													 (rxvect->ht.aggregation)))) {
		rtap_len = ALIGN(rtap_len, 4);
		rtap_len += 8;
	}

	// Check for VHT frames
	if (rxvect->format_mod == FORMATMOD_VHT) {
		rtap_len = ALIGN(rtap_len, 2);
		rtap_len += 12;
	}

	// Check for HE frames
	if (rxvect->format_mod == FORMATMOD_HE_SU) {
		rtap_len = ALIGN(rtap_len, 2);
		rtap_len += sizeof(struct ieee80211_radiotap_he);
	}

	// Check for multiple antennas
	if (hweight32(rxvect->antenna_set) > 1) {
		// antenna and antenna signal fields
		rtap_len += 2 * hweight8(rxvect->antenna_set);
	}

	// Check for vendor specific data
	if (has_vend_rtap) {
		/* vendor presence bitmap */
		rtap_len += 4;
		/* alignment for fixed 6-byte vendor data header */
		rtap_len = ALIGN(rtap_len, 2);
	}

	return rtap_len;
}

/**
 * cls_wifi_rx_add_rtap_hdr - Add radiotap header to sk_buff
 *
 * @cls_wifi_hw: main driver data
 * @skb: skb received (will include the radiotap header)
 * @rxvect: Rx vector
 * @phy_info: Information regarding the phy
 * @hwvect: HW Info (NULL if vendor specific data is available)
 * @rtap_len: Length of the radiotap header
 * @vend_rtap_len: radiotap vendor length (0 if not present)
 * @vend_it_present: radiotap vendor present
 *
 * Builds a radiotap header and add it to @skb.
 */
static void cls_wifi_rx_add_rtap_hdr(struct cls_wifi_hw* cls_wifi_hw,
								 struct sk_buff *skb,
								 struct rx_vector_1 *rxvect,
								 struct phy_channel_info_desc *phy_info,
								 struct hw_vect *hwvect,
								 int rtap_len,
								 u8 vend_rtap_len,
								 u32 vend_it_present)
{
	struct ieee80211_radiotap_header *rtap;
	u8 *pos, rate_idx;
	__le32 *it_present;
	u32 it_present_val = 0;
	bool fec_coding = false;
	bool short_gi = false;
	bool stbc = false;
	bool aggregation = false;
	__le32 tsf_lo;
	__le32 tsf_hi;

	rtap = (struct ieee80211_radiotap_header *)skb_push(skb, rtap_len);
	memset((u8*) rtap, 0, rtap_len);

	rtap->it_version = 0;
	rtap->it_pad = 0;
	rtap->it_len = cpu_to_le16(rtap_len + vend_rtap_len);

	it_present = &rtap->it_present;

	// Check for multiple antennas
	if (hweight32(rxvect->antenna_set) > 1) {
		int chain;
		unsigned long chains = rxvect->antenna_set;

		for_each_set_bit(chain, &chains, IEEE80211_MAX_CHAINS) {
			it_present_val |=
				BIT(IEEE80211_RADIOTAP_EXT) |
				BIT(IEEE80211_RADIOTAP_RADIOTAP_NAMESPACE);
			put_unaligned_le32(it_present_val, it_present);
			it_present++;
			it_present_val = BIT(IEEE80211_RADIOTAP_ANTENNA) |
							 BIT(IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
		}
	}

	// Check if vendor specific data is present
	if (vend_rtap_len) {
		it_present_val |= BIT(IEEE80211_RADIOTAP_VENDOR_NAMESPACE) |
						  BIT(IEEE80211_RADIOTAP_EXT);
		put_unaligned_le32(it_present_val, it_present);
		it_present++;
		it_present_val = vend_it_present;
	}

	put_unaligned_le32(it_present_val, it_present);
	pos = (void *)(it_present + 1);

	// IEEE80211_RADIOTAP_TSFT
	if (hwvect) {
		struct hw_rxhdr *rxhdr = container_of(hwvect,struct hw_rxhdr, hwvect);
		if (rxhdr->pattern == RX_DMA_OVER_PATTERN_LST) {
			tsf_lo = rxhdr->tsf_lo;
			tsf_hi = rxhdr->tsf_hi;
		} else {
			tsf_lo = 0;
			tsf_hi = 0;
		}

		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_TSFT);
		// padding
		while ((pos - (u8 *)rtap) & 7)
			*pos++ = 0;
		put_unaligned_le64((((u64)le32_to_cpu(tsf_hi) << 32) +
				(u64)le32_to_cpu(tsf_lo)), pos);
		pos += 8;
	}

	// IEEE80211_RADIOTAP_FLAGS
	rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_FLAGS);
	if (hwvect && hwvect->status.fcs_err)
		*pos |= IEEE80211_RADIOTAP_F_BADFCS;
	if (!rxvect->pre_type && (rxvect->format_mod <= FORMATMOD_NON_HT_DUP_OFDM))
		*pos |= IEEE80211_RADIOTAP_F_SHORTPRE;
	pos++;

	// IEEE80211_RADIOTAP_RATE
	// check for HT, VHT or HE frames
	if (rxvect->format_mod >= FORMATMOD_HE_SU) {
		rate_idx = rxvect->he.mcs;
		fec_coding = rxvect->he.fec;
		stbc = rxvect->he.stbc;
		aggregation = true;
		*pos = 0;
	} else if (rxvect->format_mod == FORMATMOD_VHT) {
		rate_idx = rxvect->vht.mcs;
		fec_coding = rxvect->vht.fec;
		short_gi = rxvect->vht.short_gi;
		stbc = rxvect->vht.stbc;
		aggregation = true;
		*pos = 0;
	} else if (rxvect->format_mod > FORMATMOD_NON_HT_DUP_OFDM) {
		rate_idx = rxvect->ht.mcs;
		fec_coding = rxvect->ht.fec;
		short_gi = rxvect->ht.short_gi;
		stbc = rxvect->ht.stbc;
		aggregation = rxvect->ht.aggregation;
		*pos = 0;
	} else {
		struct ieee80211_supported_band* band =
				&cls_wifi_hw->if_cfg80211.sbands[phy_info->phy_band];
		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_RATE);
		BUG_ON((rate_idx = legrates_lut[rxvect->leg_rate].idx) == -1);
		if (phy_info->phy_band == NL80211_BAND_5GHZ)
			rate_idx -= 4;  /* cls_wifi_ratetable_5ghz[0].hw_value == 4 */
		*pos = DIV_ROUND_UP(band->bitrates[rate_idx].bitrate, 5);
	}
	pos++;

	// IEEE80211_RADIOTAP_CHANNEL
	rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_CHANNEL);
	put_unaligned_le16(phy_info->phy_prim20_freq, pos);
	pos += 2;

	if (phy_info->phy_band == NL80211_BAND_5GHZ)
		put_unaligned_le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ, pos);
	else if (rxvect->format_mod > FORMATMOD_NON_HT_DUP_OFDM)
		put_unaligned_le16(IEEE80211_CHAN_DYN | IEEE80211_CHAN_2GHZ, pos);
	else
		put_unaligned_le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ, pos);
	pos += 2;

	if (hweight32(rxvect->antenna_set) == 1) {
		// IEEE80211_RADIOTAP_DBM_ANTSIGNAL
		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
		*pos++ = rxvect->rssi1;

		// IEEE80211_RADIOTAP_ANTENNA
		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_ANTENNA);
		*pos++ = rxvect->antenna_set;
	}

	// IEEE80211_RADIOTAP_LOCK_QUALITY is missing
	// IEEE80211_RADIOTAP_DB_ANTNOISE is missing

	// IEEE80211_RADIOTAP_RX_FLAGS
	rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_RX_FLAGS);
	// 2 byte alignment
	if ((pos - (u8 *)rtap) & 1)
		*pos++ = 0;
	put_unaligned_le16(0, pos);
	//Right now, we only support fcs error (no RX_FLAG_FAILED_PLCP_CRC)
	pos += 2;

	// Check if HT
	if ((rxvect->format_mod == FORMATMOD_HT_MF) ||
		(rxvect->format_mod == FORMATMOD_HT_GF)) {
		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_MCS);
		*pos++ = (IEEE80211_RADIOTAP_MCS_HAVE_MCS |
				  IEEE80211_RADIOTAP_MCS_HAVE_GI |
				  IEEE80211_RADIOTAP_MCS_HAVE_BW |
				  IEEE80211_RADIOTAP_MCS_HAVE_FMT |
				  IEEE80211_RADIOTAP_MCS_HAVE_FEC |
				  IEEE80211_RADIOTAP_MCS_HAVE_STBC);

		pos++;
		*pos = 0;
		if (short_gi)
			*pos |= IEEE80211_RADIOTAP_MCS_SGI;
		if (rxvect->ch_bw  == PHY_CHNL_BW_40)
			*pos |= IEEE80211_RADIOTAP_MCS_BW_40;
		if (rxvect->format_mod == FORMATMOD_HT_GF)
			*pos |= IEEE80211_RADIOTAP_MCS_FMT_GF;
		if (fec_coding)
			*pos |= IEEE80211_RADIOTAP_MCS_FEC_LDPC;
		*pos++ |= stbc << IEEE80211_RADIOTAP_MCS_STBC_SHIFT;
		*pos++ = rate_idx;
	}

	// check for HT or VHT frames
	if (aggregation && hwvect) {
		// 4 byte alignment
		while ((pos - (u8 *)rtap) & 3)
			pos++;
		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_AMPDU_STATUS);
		put_unaligned_le32(hwvect->ampdu_cnt, pos);
		pos += 4;
		put_unaligned_le32(0, pos);
		pos += 4;
	}

	// Check for VHT frames
	if (rxvect->format_mod == FORMATMOD_VHT) {
		u16 vht_details = IEEE80211_RADIOTAP_VHT_KNOWN_GI |
						  IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH;
		u8 vht_nss = rxvect->vht.nss + 1;

		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_VHT);

		if ((rxvect->ch_bw == PHY_CHNL_BW_160)
				&& phy_info->phy_center2_freq)
			vht_details &= ~IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH;
		put_unaligned_le16(vht_details, pos);
		pos += 2;

		// flags
		if (short_gi)
			*pos |= IEEE80211_RADIOTAP_VHT_FLAG_SGI;
		if (stbc)
			*pos |= IEEE80211_RADIOTAP_VHT_FLAG_STBC;
		pos++;

		// bandwidth
		if (rxvect->ch_bw == PHY_CHNL_BW_40)
			*pos++ = 1;
		if (rxvect->ch_bw == PHY_CHNL_BW_80)
			*pos++ = 4;
		else if ((rxvect->ch_bw == PHY_CHNL_BW_160)
				&& phy_info->phy_center2_freq)
			*pos++ = 0; //80P80
		else if  (rxvect->ch_bw == PHY_CHNL_BW_160)
			*pos++ = 11;
		else // 20 MHz
			*pos++ = 0;

		// MCS/NSS
		*pos++ = (rate_idx << 4) | vht_nss;
		*pos++ = 0;
		*pos++ = 0;
		*pos++ = 0;
		if (fec_coding)
			*pos |= IEEE80211_RADIOTAP_CODING_LDPC_USER0;
		pos++;
		// group ID
		pos++;
		// partial_aid
		pos += 2;
	}

	// Check for HE frames
	if (rxvect->format_mod >= FORMATMOD_HE_SU) {
		struct ieee80211_radiotap_he he;
		#define HE_PREP(f, val) cpu_to_le16(FIELD_PREP(IEEE80211_RADIOTAP_HE_##f, val))
		#define D1_KNOWN(f) cpu_to_le16(IEEE80211_RADIOTAP_HE_DATA1_##f##_KNOWN)
		#define D2_KNOWN(f) cpu_to_le16(IEEE80211_RADIOTAP_HE_DATA2_##f##_KNOWN)

		he.data1 = D1_KNOWN(BSS_COLOR) | D1_KNOWN(BEAM_CHANGE) |
				   D1_KNOWN(UL_DL) | D1_KNOWN(STBC) |
				   D1_KNOWN(DOPPLER) | D1_KNOWN(DATA_DCM);
		he.data2 = D2_KNOWN(GI) | D2_KNOWN(TXBF) | D2_KNOWN(TXOP);

		he.data3 |= HE_PREP(DATA3_BSS_COLOR, rxvect->he.bss_color);
		he.data3 |= HE_PREP(DATA3_BEAM_CHANGE, rxvect->he.beam_change);
		he.data3 |= HE_PREP(DATA3_UL_DL, rxvect->he.uplink_flag);
		he.data3 |= HE_PREP(DATA3_BSS_COLOR, rxvect->he.bss_color);
		he.data3 |= HE_PREP(DATA3_DATA_DCM, rxvect->he.dcm);

		he.data5 |= HE_PREP(DATA5_GI, rxvect->he.gi_type);
		he.data5 |= HE_PREP(DATA5_TXBF, rxvect->he.beamformed);
		he.data5 |= HE_PREP(DATA5_LTF_SIZE, rxvect->he.he_ltf_type + 1);

		he.data6 |= HE_PREP(DATA6_DOPPLER, rxvect->he.doppler);
		he.data6 |= HE_PREP(DATA6_TXOP, rxvect->he.txop_duration);

		if (rxvect->format_mod != FORMATMOD_HE_TB) {
			he.data1 |= (D1_KNOWN(DATA_MCS) | D1_KNOWN(CODING) |
						 D1_KNOWN(SPTL_REUSE) | D1_KNOWN(BW_RU_ALLOC));

			if (stbc) {
				he.data6 |= HE_PREP(DATA6_NSTS, 2);
				he.data3 |= HE_PREP(DATA3_STBC, 1);
			} else {
				he.data6 |= HE_PREP(DATA6_NSTS, rxvect->he.nss);
			}

			he.data3 |= HE_PREP(DATA3_DATA_MCS, rxvect->he.mcs);
			he.data3 |= HE_PREP(DATA3_CODING, rxvect->he.fec);

			he.data4 = HE_PREP(DATA4_SU_MU_SPTL_REUSE, rxvect->he.spatial_reuse);

			if (rxvect->format_mod == FORMATMOD_HE_MU) {
				he.data1 |= IEEE80211_RADIOTAP_HE_DATA1_FORMAT_MU;
				he.data5 |= HE_PREP(DATA5_DATA_BW_RU_ALLOC,
									rxvect->he.ru_size +
									IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_26T);
			} else {
				if (rxvect->format_mod == FORMATMOD_HE_SU)
					he.data1 |= IEEE80211_RADIOTAP_HE_DATA1_FORMAT_SU;
				else
					he.data1 |= IEEE80211_RADIOTAP_HE_DATA1_FORMAT_EXT_SU;

				switch (rxvect->ch_bw) {
					case PHY_CHNL_BW_20:
						he.data5 |= HE_PREP(DATA5_DATA_BW_RU_ALLOC,
											IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_20MHZ);
						break;
					case PHY_CHNL_BW_40:
						he.data5 |= HE_PREP(DATA5_DATA_BW_RU_ALLOC,
											IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_40MHZ);
						break;
					case PHY_CHNL_BW_80:
						he.data5 |= HE_PREP(DATA5_DATA_BW_RU_ALLOC,
											IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_80MHZ);
						break;
					case PHY_CHNL_BW_160:
						he.data5 |= HE_PREP(DATA5_DATA_BW_RU_ALLOC,
											IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_160MHZ);
						break;
					default:
						WARN_ONCE(1, "Invalid SU BW %d\n", rxvect->ch_bw);
				}
			}
		} else {
			he.data1 |= IEEE80211_RADIOTAP_HE_DATA1_FORMAT_TRIG;
		}

		/* ensure 2 bytes alignment */
		while ((pos - (u8 *)rtap) & 1)
			pos++;
		rtap->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_HE);
		memcpy(pos, &he, sizeof(he));
		pos += sizeof(he);
	}

	// Rx Chains
	if (hweight32(rxvect->antenna_set) > 1) {
		int chain;
		unsigned long chains = rxvect->antenna_set;
		u8 rssis[4] = {rxvect->rssi1, rxvect->rssi1, rxvect->rssi1, rxvect->rssi1};

		for_each_set_bit(chain, &chains, IEEE80211_MAX_CHAINS) {
			*pos++ = rssis[chain];
			*pos++ = chain;
		}
	}
}

#ifdef CONFIG_CLS_WIFI_MON_DATA
/**
 * cls_wifi_rx_dup_for_monitor - Duplicate RX skb for monitor path
 *
 * @cls_wifi_hw: main driver data
 * @rxhdr: RX header
 * @skb: RX buffer
 * @rtap_len: Length to reserve for radiotap header
 * @nb_buff: Updated with the number of skb processed (can only be >1 for A-MSDU)
 * @return 'duplicated' skb for monitor path and %NULL in case of error
 *
 * Use when RX buffer is forwarded to net layer and a monitor interface is active,
 * a 'copy' of the RX buffer is done for the monitor path.
 * This is not a simple copy as:
 * - Headroom is reserved for Radiotap header
 * - For MSDU, MAC header (included in RX header) is re-added in the buffer.
 * - A-MSDU subframes are gathered in a single buffer (adding A-MSDU and LLC/SNAP headers)
 */
static struct sk_buff * cls_wifi_rx_dup_for_monitor(struct cls_wifi_hw *cls_wifi_hw,
												struct sk_buff *skb,  struct hw_rxhdr *rxhdr,
												u8 rtap_len, int *nb_buff, u8 cmn_rxbuf_flag)
{
	struct sk_buff *skb_dup = NULL;
	u16 frm_len = le32_to_cpu(rxhdr->hwvect.len);
	int skb_count = 1;

	if (RX_FLAGS_GET(80211_MPDU, rxhdr->flags)) {
		if (frm_len > skb_tailroom(skb))
			frm_len = skb_tailroom(skb);
		skb_put(skb, frm_len);
		skb_dup = skb_copy_expand(skb, rtap_len, 0, GFP_ATOMIC);
	} else {
		// For MSDU, need to re-add the MAC header
		u16 machdr_len = rxhdr->mac_hdr_backup.buf_len;
		u8* machdr_ptr = rxhdr->mac_hdr_backup.buffer;
		int tailroom = 0;
		int headroom = machdr_len + rtap_len;
		int amsdu = RX_FLAGS_GET(AMSDU, rxhdr->flags);

		if (amsdu) {
			int subfrm_len;
			subfrm_len = rxhdr->amsdu_len[0];
			// Set tailroom to store all subframes. frm_len is the size
			// of the A-MSDU as received by MACHW (i.e. with LLC/SNAP and padding)
			tailroom = frm_len - subfrm_len;
			if (tailroom < 0)
				goto end;
			headroom += sizeof(rfc1042_header) + 2;

			if (subfrm_len > skb_tailroom(skb))
				subfrm_len = skb_tailroom(skb);
			skb_put(skb, subfrm_len);

		} else {
			// Pull Ethernet header from skb
			if (frm_len > skb_tailroom(skb))
				frm_len = skb_tailroom(skb);
			skb_put(skb, frm_len);
			skb_pull(skb, sizeof(struct ethhdr));
		}

		// Copy skb and extend for adding the radiotap header and the MAC header
		skb_dup = skb_copy_expand(skb, headroom, tailroom, GFP_ATOMIC);
		if (!skb_dup)
			goto end;

		if (amsdu) {
			// recopy subframes in a single buffer, and add SNAP/LLC if needed
			struct ethhdr *eth_hdr, *amsdu_hdr;
			int count = 0, padding;
			u32 hostid;

			eth_hdr = (struct ethhdr *)skb_dup->data;
			if (ntohs(eth_hdr->h_proto) >= 0x600) {
				// Re-add LLC/SNAP header
				int llc_len =  sizeof(rfc1042_header) + 2;
				amsdu_hdr = skb_push(skb_dup, llc_len);
				memmove(amsdu_hdr, eth_hdr, sizeof(*eth_hdr));
				amsdu_hdr->h_proto = htons(rxhdr->amsdu_len[0] +
										   llc_len - sizeof(*eth_hdr));
				amsdu_hdr++;
				memcpy(amsdu_hdr, rfc1042_header, sizeof(rfc1042_header));
			}
			padding = AMSDU_PADDING(rxhdr->amsdu_len[0]);

			while ((count < ARRAY_SIZE(rxhdr->amsdu_hostids)) &&
				   (hostid = rxhdr->amsdu_hostids[count++])) {
				struct cls_wifi_ipc_buf *subfrm_ipc;
				struct sk_buff *subfrm_skb;
				void *src;
				int subfrm_len, llc_len = 0, truncated = 0;
				struct device *dev;

				if (cmn_rxbuf_flag) {
					struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_hw->plat->cmn_hw;
					u8 radio_idx = cls_wifi_hw->radio_idx;

					dev = cmn_hw->dev;
					subfrm_ipc = cls_wifi_cmn_ipc_rxbuf_from_hostid(cmn_hw, hostid, radio_idx, 0);
				} else {
					dev = cls_wifi_hw->dev;
					subfrm_ipc = cls_wifi_ipc_rxbuf_from_hostid(cls_wifi_hw, hostid);
				}

				if (!subfrm_ipc)
					continue;

				// Cannot use e2a_release here as it will delete the pointer to the skb
				cls_wifi_ipc_buf_e2a_sync(dev, subfrm_ipc, 0);

				subfrm_skb = subfrm_ipc->addr;
				eth_hdr = (struct ethhdr *)subfrm_skb->data;
				subfrm_len = rxhdr->amsdu_len[count];
				if (subfrm_len > skb_tailroom(subfrm_skb))
					truncated = skb_tailroom(subfrm_skb) - subfrm_len;

				if (ntohs(eth_hdr->h_proto) >= 0x600)
					llc_len = sizeof(rfc1042_header) + 2;

				if (skb_tailroom(skb_dup) < padding + subfrm_len + llc_len) {
					dev_kfree_skb(skb_dup);
					skb_dup = NULL;
					goto end;
				}

				if (padding)
					skb_put(skb_dup, padding);
				if (llc_len) {
					int move_oft = offsetof(struct ethhdr, h_proto);
					amsdu_hdr = skb_put(skb_dup, sizeof(struct ethhdr));
					memcpy(amsdu_hdr, eth_hdr, move_oft);
					amsdu_hdr->h_proto = htons(subfrm_len + llc_len
											   - sizeof(struct ethhdr));
					memcpy(skb_put(skb_dup, sizeof(rfc1042_header)),
						   rfc1042_header, sizeof(rfc1042_header));

					src = &eth_hdr->h_proto;
					subfrm_len -= move_oft;
				} else {
					src = eth_hdr;
				}
				if (!truncated) {
					memcpy(skb_put(skb_dup, subfrm_len), src, subfrm_len);
				} else {
					memcpy(skb_put(skb_dup, subfrm_len - truncated), src, subfrm_len - truncated);
					memset(skb_put(skb_dup, truncated), 0, truncated);
				}
				skb_count++;
			}
		}

		// Copy MAC Header in new headroom
		memcpy(skb_push(skb_dup, machdr_len), machdr_ptr, machdr_len);
	}

end:
	// Reset original state for skb
	skb->data = (void*) rxhdr;
	__skb_set_length(skb, 0);
	*nb_buff = skb_count;
	return skb_dup;
}
#endif // CONFIG_CLS_WIFI_MON_DATA

/**
 * cls_wifi_rx_monitor - Build radiotap header for skb and send it to netdev
 *
 * @cls_wifi_hw: main driver data
 * @cls_wifi_vif: vif that received the buffer
 * @skb: sk_buff received
 * @rxhdr: Pointer to HW RX header
 * @rtap_len: Radiotap Header length
 *
 * Add radiotap header to the received skb and send it to netdev
 */
static void cls_wifi_rx_monitor(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
							struct sk_buff *skb,  struct hw_rxhdr *rxhdr,
							u8 rtap_len)
{
	skb->dev = cls_wifi_vif->ndev;

	/* Add RadioTap Header */
	cls_wifi_rx_add_rtap_hdr(cls_wifi_hw, skb, &rxhdr->hwvect.rx_vect1,
						 &rxhdr->phy_info, &rxhdr->hwvect,
						 rtap_len, 0, 0);

	skb_reset_mac_header(skb);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = htons(ETH_P_802_2);

	netif_receive_skb(skb);
}

/**
 * cls_wifi_unsup_rx_vec_ind() - IRQ handler callback for %IPC_IRQ_E2A_UNSUP_RX_VEC
 *
 * FMAC has triggered an IT saying that a rx vector of an unsupported frame has been
 * captured and sent to upper layer.
 * If no monitor interface is active ignore it, otherwise add a radiotap header with a
 * vendor specific header and upload it on the monitor interface.
 *
 * @pthis: Pointer to main driver data
 * @arg: Pointer to IPC buffer
 */
u8 cls_wifi_unsup_rx_vec_ind(void *pthis, void *arg) {
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *ipc_buf = arg;
	struct rx_vector_desc *rx_desc;
	struct sk_buff *skb;
	struct rx_vector_1 *rx_vect1;
	struct phy_channel_info_desc *phy_info;
	struct vendor_radiotap_hdr *rtap;
	u16 ht_length;
	struct cls_wifi_vif *cls_wifi_vif;
	struct rx_vector_desc rx_vect_desc;
	u8 rtap_len, vend_rtap_len = sizeof(*rtap);

	cls_wifi_ipc_buf_e2a_sync(cls_wifi_hw->dev, ipc_buf, sizeof(struct rx_vector_desc));

	skb = ipc_buf->addr;
	if (((struct rx_vector_desc *)(skb->data))->pattern == 0) {
		cls_wifi_ipc_buf_e2a_sync_back(cls_wifi_hw->dev, ipc_buf, sizeof(struct rx_vector_desc));
		return -1;
	}

	if (cls_wifi_hw->monitor_vif == CLS_WIFI_INVALID_VIF) {
		cls_wifi_ipc_unsuprxvec_repush(cls_wifi_hw, ipc_buf);
		return -1;
	}

	cls_wifi_vif = cls_wifi_hw->vif_table[cls_wifi_hw->monitor_vif];
	skb->dev = cls_wifi_vif->ndev;
	memcpy(&rx_vect_desc, skb->data, sizeof(rx_vect_desc));
	rx_desc = &rx_vect_desc;

	rx_vect1 = (struct rx_vector_1 *) (rx_desc->rx_vect1);
	phy_info = (struct phy_channel_info_desc *) (&rx_desc->phy_info);
	if (rx_vect1->format_mod >= FORMATMOD_VHT)
		ht_length = 0;
	else
		ht_length = (u16) le32_to_cpu(rx_vect1->ht.length);

	// Reserve space for radiotap
	skb_reserve(skb, RADIOTAP_HDR_MAX_LEN);

	/* Fill vendor specific header with fake values */
	rtap = (struct vendor_radiotap_hdr *) skb->data;
	rtap->oui[0] = 0x00;
	rtap->oui[1] = 0x25;
	rtap->oui[2] = 0x3A;
	rtap->subns  = 0;
	rtap->len = sizeof(ht_length);
	put_unaligned_le16(ht_length, rtap->data);
	vend_rtap_len += rtap->len;
	skb_put(skb, vend_rtap_len);

	/* Copy fake data */
	put_unaligned_le16(0, skb->data + vend_rtap_len);
	skb_put(skb, UNSUP_RX_VEC_DATA_LEN);

	/* Get RadioTap Header length */
	rtap_len = cls_wifi_rx_rtap_hdrlen(rx_vect1, true);

	/* Check headroom space */
	if (skb_headroom(skb) < rtap_len) {
		netdev_err(cls_wifi_vif->ndev, "not enough headroom %d need %d\n",
				   skb_headroom(skb), rtap_len);
		cls_wifi_ipc_unsuprxvec_repush(cls_wifi_hw, ipc_buf);
		return -1;
	}

	/* Add RadioTap Header */
	cls_wifi_rx_add_rtap_hdr(cls_wifi_hw, skb, rx_vect1, phy_info, NULL,
						 rtap_len, vend_rtap_len, BIT(0));

	skb_reset_mac_header(skb);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = htons(ETH_P_802_2);

	cls_wifi_ipc_buf_e2a_release(cls_wifi_hw, ipc_buf);
	netif_receive_skb(skb);

	/* Allocate and push a new buffer to fw to replace this one */
	cls_wifi_ipc_unsuprxvec_alloc(cls_wifi_hw, ipc_buf);
	return 0;
}

/**
 * cls_wifi_rx_amsdu_free() - Free RX buffers used (if any) to upload A-MSDU subframes
 *
 * @cls_wifi_hw: Main driver data
 * @rxhdr: RX header for the buffer (Must have been synced)
 * @return: Number of A-MSDU subframes (including the first one), or 1 for non
 * A-MSDU frame
 *
 * If this RX header correspond to an A-MSDU then all the Rx buffer used to
 * upload subframes are freed.
 */
static int cls_wifi_rx_amsdu_free(struct cls_wifi_hw *cls_wifi_hw, struct hw_rxhdr *rxhdr, u8 cmn_rxbuf_flag)
{
	int count = 0;
	u32 hostid;
	int res = 1;

	if (!RX_FLAGS_GET(AMSDU, rxhdr->flags))
		return res;

	while ((count < ARRAY_SIZE(rxhdr->amsdu_hostids)) &&
		   (hostid = rxhdr->amsdu_hostids[count++])) {
		struct cls_wifi_ipc_buf *ipc_buf;
		struct device *dev;
		if (cmn_rxbuf_flag) {
			struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_hw->plat->cmn_hw;
			ipc_buf = cls_wifi_cmn_ipc_rxbuf_from_hostid(cmn_hw, hostid, cls_wifi_hw->radio_idx, 0);
			dev = cmn_hw->dev;
		} else {
			ipc_buf = cls_wifi_ipc_rxbuf_from_hostid(cls_wifi_hw, hostid);
			dev = cls_wifi_hw->dev;
		}

		if (!ipc_buf)
			continue;
		cls_wifi_ipc_rxbuf_dealloc(dev, ipc_buf);
		res++;
	}
	return res;
}
/**
 * cls_wifi_check_mgmt_attack - check whether the received authentication or association
 * frame is an attack
 *
 *@cls_wifi_hw: Main driver data
 *@skb: skb received
 *@sta: the station info sending the mgmt frame
 *@enable: enable/disable the check
 *@interval: the set value(s)
 *@return : 1(true) or 0 (false)
 *
 * If the time bewteen receiving auth or assoc mgmt frame and receiving the last data frame
 * from the same station is less than the set value, the frame is an attack
 */

int cls_wifi_check_mgmt_attack(struct cls_wifi_hw *cls_wifi_hw, struct sk_buff *skb,
				struct cls_wifi_sta *sta, int enable, int interval)
{
	struct ieee80211_mgmt *mgmt = NULL;
	struct cls_wifi_vif *vif = NULL;
	int msec  = 0;

	if (!enable || !cls_wifi_hw || !skb || !sta || !interval)
		return 0;

	mgmt = (struct ieee80211_mgmt *)skb->data;
	if (mgmt && (ieee80211_is_auth(mgmt->frame_control)
		|| ieee80211_is_assoc_req(mgmt->frame_control)
		|| ieee80211_is_reassoc_req(mgmt->frame_control))) {
		if (sta && (sta->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED))
			&& !(sta->sta_flags_set & BIT(NL80211_STA_FLAG_MFP))) {
			vif = cls_wifi_hw->vif_table[sta->vif_idx];
			if (vif) {
				if (!memcmp(mgmt->da, vif->ndev->dev_addr, ETH_ALEN) &&
					!memcmp(mgmt->sa, sta->mac_addr, ETH_ALEN) &&
					!memcmp(mgmt->bssid, vif->ndev->dev_addr, ETH_ALEN)) {
					msec = jiffies_to_msecs(jiffies - sta->last_data_frame_rx);
					pr_warn("%s seq =0x%x, msec=%d\n", __func__,
						mgmt->seq_ctrl, msec);
					if (msec < interval * 1000)
						return 1;
				}
			}
		}
	}
	return 0;
}

/**
 * cls_wifi_rxdataind - Process rx buffer
 *
 * @pthis: Pointer to the object attached to the IPC structure
 *		 (points to struct cls_wifi_hw is this case)
 * @arg: Address of the RX descriptor
 *
 * This function is called for each buffer received by the fw
 *
 */
ktime_t uptime_ktime;
u8 cls_wifi_rxdataind(void *pthis, void *arg, u8 rx_from_cmn)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *ipc_desc = arg;
	struct cls_wifi_ipc_buf *ipc_buf;
	struct hw_rxhdr *hw_rxhdr = NULL;
	struct rxdesc_tag *rxdesc;
	struct cls_wifi_vif *cls_wifi_vif;
	struct sk_buff *skb = NULL;
	int msdu_offset = sizeof(struct hw_rxhdr);
	int nb_buff = 1;
	u16_l status;
	u16_l flags;
	struct device *dev;
	struct cls_wifi_cmn_hw *cmn_hw;
	u8 radio_idx;
	u8 cmn_rxbuf_flag = 0;
	unsigned int vif_idx;

	uptime_ktime = ktime_get_boottime();
	cmn_hw = cls_wifi_hw->plat->cmn_hw;
	radio_idx = cls_wifi_hw->radio_idx;
	if (rx_from_cmn) {
		dev = cmn_hw->dev;
	} else {
		dev = cls_wifi_hw->dev;
	}

	REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_CLS_WIFIDATAIND);

	cls_wifi_ipc_buf_e2a_sync(dev, ipc_desc, sizeof(struct rxdesc_tag));

	rxdesc = ipc_desc->addr;
	status = rxdesc->status;
	flags = rxdesc->flags;

	if (rx_from_cmn || flags & IPC_RXDESC_FLAGS_CMN)
		cmn_rxbuf_flag = 1;

	if (!status) {
		/* frame is not completely uploaded, give back ownership of the descriptor */
		cls_wifi_ipc_buf_e2a_sync_back(dev, ipc_desc, sizeof(struct rxdesc_tag));
		return -1;
	}

	if (cmn_rxbuf_flag) {
		ipc_buf = cls_wifi_cmn_ipc_rxbuf_from_hostid(cmn_hw, rxdesc->host_id, radio_idx, status);
	} else {
		ipc_buf = cls_wifi_ipc_rxbuf_from_hostid(cls_wifi_hw, rxdesc->host_id);
	}
	if (!ipc_buf) {
		if (rx_from_cmn && status == RX_STAT_ALLOC) {
			nb_buff = (flags & IPC_RXDESC_FLAGS_BUF_NB_MASK) >> IPC_RXDESC_FLAGS_BUF_NB_OPS;
		}
		goto check_alloc;
	}

	skb = ipc_buf->addr;

	/* Check if we need to delete the buffer */
	if (status & RX_STAT_DELETE) {
		hw_rxhdr = (struct hw_rxhdr *)skb->data;
		cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, msdu_offset);
		nb_buff = cls_wifi_rx_amsdu_free(cls_wifi_hw, hw_rxhdr, cmn_rxbuf_flag);
		cls_wifi_ipc_rxbuf_dealloc(dev, ipc_buf);
		goto check_alloc;
	}

	/* Check if we need to forward the buffer for scan ext */
	if (status & RX_STAT_SCANEXT) {
		u16 frm_len;

		// should never happen
		if (!cls_wifi_hw->scan_ext.ext_enabled) {
			pr_warn("%s %d radio %d\n", __func__, __LINE__, cls_wifi_hw->radio_idx);
			hw_rxhdr = (struct hw_rxhdr *)skb->data;
			cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, msdu_offset);
			cls_wifi_ipc_rxbuf_dealloc(dev, ipc_buf);
			goto check_alloc;
		}

		status |= RX_STAT_ALLOC;
		cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, sizeof(hw_rxhdr));
		hw_rxhdr = (struct hw_rxhdr *)skb->data;
		skb_reserve(skb, msdu_offset);
		frm_len = le32_to_cpu(hw_rxhdr->hwvect.len);
		cls_wifi_ipc_buf_e2a_release(cls_wifi_hw, ipc_buf);

		if (frm_len > skb_tailroom(skb))
			frm_len = skb_tailroom(skb);
		skb_put(skb, frm_len);

		// todo start: for test only, call rx handler for customer instead
		//print_hex_dump(KERN_WARNING, "", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len,
		//		false);
		cls_wifi_vif = cls_wifi_get_vif(cls_wifi_hw, 1);
		if (!cls_wifi_vif) {
			dev_kfree_skb(skb);
			goto check_alloc;
		}
		skb->dev = cls_wifi_vif->ndev;
		skb_reset_mac_header(skb);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->pkt_type = PACKET_OTHERHOST;
		skb->protocol = htons(ETH_P_802_2);
		netif_receive_skb(skb);
		// todo end: for test only, call rx handler for customer instead
	}

	/* Check if we need to forward the buffer coming from a monitor interface */
	if (status & RX_STAT_MONITOR) {
		struct sk_buff *skb_monitor = NULL;
		struct hw_rxhdr hw_rxhdr_copy;
		u8 rtap_len;
		u16 frm_len;
		struct rx_vector_2 *rx_vect2 = NULL;
		u32 rxvect2_flag = 0;

		// Check if monitor interface exists and is open
		cls_wifi_vif = cls_wifi_get_vif(cls_wifi_hw, cls_wifi_hw->monitor_vif);
		if (!cls_wifi_vif || (cls_wifi_vif->wdev.iftype != NL80211_IFTYPE_MONITOR)) {
			hw_rxhdr = (struct hw_rxhdr *)skb->data;
			rxvect2_flag = (hw_rxhdr->pattern == RX_DMA_OVER_PATTERN_LST) ? 1 : 0;
			if (rxvect2_flag) {
				rx_vect2 = &hw_rxhdr->rx_vect2;
			}
			cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, msdu_offset);
			nac_monitor_per_pkt_info(cls_wifi_hw, hw_rxhdr, rx_vect2);
			nb_buff = cls_wifi_rx_amsdu_free(cls_wifi_hw, hw_rxhdr, cmn_rxbuf_flag);
			cls_wifi_ipc_rxbuf_dealloc(dev, ipc_buf);

			goto check_len_update;
		}

		cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, sizeof(hw_rxhdr));
		hw_rxhdr = (struct hw_rxhdr *)skb->data;
		rtap_len = cls_wifi_rx_rtap_hdrlen(&hw_rxhdr->hwvect.rx_vect1, false);

		skb_reserve(skb, msdu_offset);
		frm_len = le32_to_cpu(hw_rxhdr->hwvect.len);

		if (status == RX_STAT_MONITOR) {
			status |= RX_STAT_ALLOC;

			cls_wifi_ipc_buf_e2a_release(cls_wifi_hw, ipc_buf);

			if (frm_len > skb_tailroom(skb))
				frm_len = skb_tailroom(skb);
			skb_put(skb, frm_len);

			memcpy(&hw_rxhdr_copy, hw_rxhdr, sizeof(hw_rxhdr_copy));
			hw_rxhdr = &hw_rxhdr_copy;

			if (rtap_len > msdu_offset) {
				if (skb_end_offset(skb) < frm_len + rtap_len) {
					// not enough space in the buffer need to re-alloc it
					if (pskb_expand_head(skb, rtap_len, 0, GFP_ATOMIC)) {
						dev_kfree_skb(skb);
						goto check_alloc;
					}
				} else {
					// enough space but not enough headroom, move data (instead of re-alloc)
					int delta = rtap_len - msdu_offset;
					memmove(skb->data, skb->data + delta, frm_len);
					skb_reserve(skb, delta);
				}
			}
			skb_monitor = skb;
		}
		else
		{
#ifdef CONFIG_CLS_WIFI_MON_DATA
			if (status & RX_STAT_FORWARD)
				// OK to release here, and other call to release in forward will do nothing
				cls_wifi_ipc_buf_e2a_release(cls_wifi_hw, ipc_buf);
			else
				cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, 0);

			// Use reserved field to save info that RX vect has already been converted
			hw_rxhdr->hwvect.reserved = 1;
			skb_monitor = cls_wifi_rx_dup_for_monitor(cls_wifi_hw, skb, hw_rxhdr, rtap_len, &nb_buff, cmn_rxbuf_flag);
			if (!skb_monitor) {
				hw_rxhdr = NULL;
				goto check_len_update;
			}
#else
			wiphy_err(cls_wifi_hw->wiphy, "RX status %d is invalid when MON_DATA is disabled\n", status);
			goto check_len_update;
#endif
		}

		cls_wifi_rx_monitor(cls_wifi_hw, cls_wifi_vif, skb_monitor, hw_rxhdr, rtap_len);
	}

check_len_update:
	/* Check if we need to update the length */
	if (status & RX_STAT_LEN_UPDATE) {
		int sync_len = msdu_offset + sizeof(struct ethhdr);

		cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, sync_len);

		hw_rxhdr = (struct hw_rxhdr *)skb->data;
		hw_rxhdr->hwvect.len = rxdesc->frame_len;

		if (status & RX_STAT_ETH_LEN_UPDATE) {
			/* Update Length Field inside the Ethernet Header */
			struct ethhdr *hdr = (struct ethhdr *)((u8 *)hw_rxhdr + msdu_offset);
			hdr->h_proto = htons(rxdesc->frame_len - sizeof(struct ethhdr));
		}

		cls_wifi_ipc_buf_e2a_sync_back(dev, ipc_buf, sync_len);
		goto end;
	}

	/* Check if it must be discarded after informing upper layer */
	if (status & RX_STAT_SPURIOUS) {
		struct ieee80211_hdr *hdr;
		size_t sync_len =  msdu_offset + sizeof(*hdr);

		/* Read mac header to obtain Transmitter Address */
		cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, sync_len);

		hw_rxhdr = (struct hw_rxhdr *)skb->data;
		hdr = (struct ieee80211_hdr *)(skb->data + msdu_offset);
		cls_wifi_vif = cls_wifi_get_vif(cls_wifi_hw, RX_FLAGS_GET(VIF_IDX, hw_rxhdr->flags));
		if (cls_wifi_vif) {
			cfg80211_rx_spurious_frame(cls_wifi_vif->ndev, hdr->addr2, GFP_ATOMIC);
		}
		cls_wifi_ipc_buf_e2a_sync_back(dev, ipc_buf, sync_len);
		if (cmn_rxbuf_flag)
			cls_wifi_cmn_ipc_rxbuf_repush(cmn_hw, ipc_buf, radio_idx);
		else
			cls_wifi_ipc_rxbuf_repush(cls_wifi_hw, ipc_buf);
		goto end;
	}

	/* Check if we need to forward the buffer */
	if (status & RX_STAT_FORWARD) {
		struct cls_wifi_sta *sta = NULL;
		int sta_idx;
		struct rx_vector_2 *rx_vect2 = NULL;
		int rxvect2_flag;

		cls_wifi_ipc_buf_e2a_release(cls_wifi_hw, ipc_buf);

		hw_rxhdr = (struct hw_rxhdr *)skb->data;
		rxvect2_flag = (hw_rxhdr->pattern == RX_DMA_OVER_PATTERN_LST) ? 1 : 0;
		if (rxvect2_flag) {
			rx_vect2 = &hw_rxhdr->rx_vect2;
		}

		skb_reserve(skb, msdu_offset);

		sta_idx = RX_FLAGS_GET(STA_IDX, hw_rxhdr->flags);
		if (sta_idx != CLS_WIFI_INVALID_STA) {
			sta = &cls_wifi_hw->sta_table[sta_idx];
			if (!cmn_rxbuf_flag || (hw_rxhdr->hwvect.rx_vect1.format_mod == FORMATMOD_OFFLOAD))
				cls_wifi_rx_statistic(cls_wifi_hw, hw_rxhdr, sta);
		}

		if (RX_FLAGS_GET(80211_MPDU, hw_rxhdr->flags)) {
			if (sta && cls_wifi_check_mgmt_attack(cls_wifi_hw, skb, sta,
				cls_anti_attack.en_anti_attack,
				cls_anti_attack.internal_attack_to_lastdata)) {
				dev_kfree_skb(skb);
			} else
				cls_wifi_rx_mgmt_any(cls_wifi_hw, skb, hw_rxhdr);
		} else {
			vif_idx = RX_FLAGS_GET(VIF_IDX, hw_rxhdr->flags);
			cls_wifi_vif = cls_wifi_get_vif(cls_wifi_hw, vif_idx);

			if (!cls_wifi_vif) {
				dev_err(cls_wifi_hw->dev, "Frame received but no active vif (%d)",
						RX_FLAGS_GET(VIF_IDX, hw_rxhdr->flags));
				nb_buff = cls_wifi_rx_amsdu_free(cls_wifi_hw, hw_rxhdr, cmn_rxbuf_flag);
				dev_kfree_skb(skb);
				goto check_alloc;
			}

			if (sta) {
				if (sta->vlan_idx != cls_wifi_vif->vif_index) {
					cls_wifi_vif = cls_wifi_hw->vif_table[sta->vlan_idx];
					if (!cls_wifi_vif) {
						nb_buff = cls_wifi_rx_amsdu_free(cls_wifi_hw, hw_rxhdr, cmn_rxbuf_flag);
						dev_kfree_skb(skb);
						goto check_alloc;
					}
				}

				if (RX_FLAGS_GET(4_ADDR, hw_rxhdr->flags)) {
					// for auto_4addr, use sta->is_4addr to identify 4addr sta
					if (cls_wifi_vif->auto_4addr)
						sta->is_4addr = true;
					else if (!cls_wifi_vif->use_4addr)
						cfg80211_rx_unexpected_4addr_frame(cls_wifi_vif->ndev,
													   sta->mac_addr, GFP_ATOMIC);
				}
				sta->last_data_frame_rx = jiffies;
			}

			skb->priority = 256 + RX_FLAGS_GET(USER_PRIO, hw_rxhdr->flags);

			nb_buff = cls_wifi_rx_data_skb(cls_wifi_hw, cls_wifi_vif, sta, skb,
					hw_rxhdr, cmn_rxbuf_flag, vif_idx);
		}
	}

check_alloc:
	/* Check if we need to allocate a new buffer */
	if (status & RX_STAT_ALLOC) {
		if (!hw_rxhdr && skb) {
			// True for buffer uploaded with only RX_STAT_ALLOC
			// (e.g. MPDU received out of order in a BA)
			hw_rxhdr = (struct hw_rxhdr *)skb->data;
			cls_wifi_ipc_buf_e2a_sync(dev, ipc_buf, msdu_offset);
			if (RX_FLAGS_GET(AMSDU, hw_rxhdr->flags)) {
				int i;
				for (i = 0; i < ARRAY_SIZE(hw_rxhdr->amsdu_hostids); i++) {
					if (!hw_rxhdr->amsdu_hostids[i])
						break;
					nb_buff++;
				}
			}
		}

		while (nb_buff--) {
			if (cmn_rxbuf_flag)
				cls_wifi_cmn_ipc_rxbuf_alloc(cmn_hw, radio_idx);
			else
				cls_wifi_ipc_rxbuf_alloc(cls_wifi_hw);
		}
	}

end:
	REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_CLS_WIFIDATAIND);

	/* Reset and repush descriptor to FW */
	if (rx_from_cmn)
		cls_wifi_cmn_ipc_rxdesc_repush(cmn_hw, ipc_desc, radio_idx);
	else
		cls_wifi_ipc_rxdesc_repush(cls_wifi_hw, ipc_desc);

	return 0;
}

/**
 * cls_wifi_rx_deferred - Work function to defer processing of buffer that cannot be
 * done in cls_wifi_rxdataind (that is called in atomic context)
 *
 * @ws: work field within struct cls_wifi_defer_rx
 */
void cls_wifi_rx_deferred(struct work_struct *ws)
{
	struct cls_wifi_defer_rx *rx = container_of(ws, struct cls_wifi_defer_rx, work);
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&rx->sk_list)) != NULL) {
		// Currently only management frame can be deferred
		struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
		struct cls_wifi_defer_rx_cb *rx_cb = (struct cls_wifi_defer_rx_cb *)skb->cb;

		if (ieee80211_is_action(mgmt->frame_control) &&
			(mgmt->u.action.category == 6)) {
			struct cfg80211_ft_event_params ft_event;
			struct cls_wifi_vif *vif = rx_cb->vif;
			u8 *action_frame = (u8 *)&mgmt->u.action;
			u8 action_code = action_frame[1];
			u16 status_code = *((u16 *)&action_frame[2 + 2 * ETH_ALEN]);

			if ((action_code == 2) && (status_code == 0)) {
				ft_event.target_ap = action_frame + 2 + ETH_ALEN;
				ft_event.ies = action_frame + 2 + 2 * ETH_ALEN + 2;
				ft_event.ies_len = skb->len - (ft_event.ies - (u8 *)mgmt);
				ft_event.ric_ies = NULL;
				ft_event.ric_ies_len = 0;
				cfg80211_ft_event(rx_cb->vif->ndev, &ft_event);
				vif->sta.flags |= CLS_WIFI_STA_FT_OVER_DS;
				memcpy(vif->sta.ft_target_ap, ft_event.target_ap, ETH_ALEN);
			}
		} else if (ieee80211_is_auth(mgmt->frame_control)) {
			struct cfg80211_ft_event_params ft_event;
			struct cls_wifi_vif *vif = rx_cb->vif;
			ft_event.target_ap = vif->sta.ft_target_ap;
			ft_event.ies = mgmt->u.auth.variable;
			ft_event.ies_len = (skb->len -
								offsetof(struct ieee80211_mgmt, u.auth.variable));
			ft_event.ric_ies = NULL;
			ft_event.ric_ies_len = 0;
			cfg80211_ft_event(rx_cb->vif->ndev, &ft_event);
			vif->sta.flags |= CLS_WIFI_STA_FT_OVER_AIR;
		} else {
			netdev_warn(rx_cb->vif->ndev, "Unexpected deferred frame fctl=0x%04x",
						mgmt->frame_control);
		}

		dev_kfree_skb(skb);
	}
}
