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
#include <linux/etherdevice.h>
#include <net/sock.h>
#include <linux/random.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_mesh.h"
#include "cls_wifi_events.h"
#include "cls_wifi_compat.h"
#include "cls_wifi_main.h"

/******************************************************************************
 * Power Save functions
 *****************************************************************************/
/**
 * cls_wifi_set_traffic_status - Inform FW if traffic is available for STA in PS
 *
 * @cls_wifi_hw: Driver main data
 * @sta: Sta in PS mode
 * @available: whether traffic is buffered for the STA
 * @ps_id: type of PS data requested (@LEGACY_PS_ID or @UAPSD_ID)
  */
void cls_wifi_set_traffic_status(struct cls_wifi_hw *cls_wifi_hw,
							 struct cls_wifi_sta *sta,
							 bool available,
							 u8 ps_id)
{
	if (sta->tdls.active) {
		cls_wifi_send_tdls_peer_traffic_ind_req(cls_wifi_hw,
											cls_wifi_hw->vif_table[sta->vif_idx]);
	} else {
		bool uapsd = (ps_id != LEGACY_PS_ID);
		cls_wifi_send_me_traffic_ind(cls_wifi_hw, sta->sta_idx, uapsd, available);
		trace_ps_traffic_update(sta->sta_idx, available, uapsd);
	}
}

/**
 * cls_wifi_ps_bh_enable - Enable/disable PS mode for one STA
 *
 * @cls_wifi_hw: Driver main data
 * @sta: Sta which enters/leaves PS mode
 * @enable: PS mode status
 *
 * This function will enable/disable PS mode for one STA.
 * When enabling PS mode:
 *  - Stop all STA's txq for CLS_WIFI_TXQ_STOP_STA_PS reason
 *  - Count how many buffers are already ready for this STA
 *  - For BC/MC sta, update all queued SKB to use hw_queue BCMC
 *  - Update TIM if some packet are ready
 *
 * When disabling PS mode:
 *  - Start all STA's txq for CLS_WIFI_TXQ_STOP_STA_PS reason
 *  - For BC/MC sta, update all queued SKB to use hw_queue AC_BE
 *  - Update TIM if some packet are ready (otherwise fw will not update TIM
 *	in beacon for this STA)
 *
 * All counter/skb updates are protected from TX path by taking tx_lock
 *
 * NOTE: _bh_ in function name indicates that this function is called
 * from a bottom_half tasklet.
 */
void cls_wifi_ps_bh_enable(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
					   bool enable)
{
	struct cls_wifi_txq *txq;

	if (enable) {
		trace_ps_enable(sta);

		spin_lock_bh(&cls_wifi_hw->tx_lock);
		sta->ps.active = true;
		sta->ps.sp_cnt[LEGACY_PS_ID] = 0;
		sta->ps.sp_cnt[UAPSD_ID] = 0;
		cls_wifi_txq_sta_stop(sta, CLS_WIFI_TXQ_STOP_STA_PS, cls_wifi_hw);
		cls_wifi_he_mu_sta_ps_update(cls_wifi_hw, sta, true);

		if (is_multicast_sta(cls_wifi_hw, sta->sta_idx)) {
			txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
			sta->ps.pkt_ready[LEGACY_PS_ID] = skb_queue_len(&txq->sk_list);
			sta->ps.pkt_ready[UAPSD_ID] = 0;
			txq->hwq = &cls_wifi_hw->hwq[CLS_WIFI_HWQ_BCMC];
		} else {
			int i;
			sta->ps.pkt_ready[LEGACY_PS_ID] = 0;
			sta->ps.pkt_ready[UAPSD_ID] = 0;
			foreach_sta_txq(sta, txq, i, cls_wifi_hw) {
				sta->ps.pkt_ready[txq->ps_id] += skb_queue_len(&txq->sk_list);
			}
		}

		spin_unlock_bh(&cls_wifi_hw->tx_lock);

		if (sta->ps.pkt_ready[LEGACY_PS_ID])
			cls_wifi_set_traffic_status(cls_wifi_hw, sta, true, LEGACY_PS_ID);

		if (sta->ps.pkt_ready[UAPSD_ID])
			cls_wifi_set_traffic_status(cls_wifi_hw, sta, true, UAPSD_ID);
	} else {
		trace_ps_disable(sta->sta_idx);

		spin_lock_bh(&cls_wifi_hw->tx_lock);
		sta->ps.active = false;

		if (is_multicast_sta(cls_wifi_hw, sta->sta_idx)) {
			txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
			txq->hwq = &cls_wifi_hw->hwq[CLS_WIFI_HWQ_BE];
			txq->push_limit = 0;
		} else {
			int i;
			foreach_sta_txq(sta, txq, i, cls_wifi_hw) {
				txq->push_limit = 0;
			}
		}

		cls_wifi_txq_sta_start(sta, CLS_WIFI_TXQ_STOP_STA_PS, cls_wifi_hw);
		cls_wifi_he_mu_sta_ps_update(cls_wifi_hw, sta, false);
		spin_unlock_bh(&cls_wifi_hw->tx_lock);

		if (sta->ps.pkt_ready[LEGACY_PS_ID]) {
			sta->ps.pkt_ready[LEGACY_PS_ID] = 0;
			cls_wifi_set_traffic_status(cls_wifi_hw, sta, false, LEGACY_PS_ID);
		}

		if (sta->ps.pkt_ready[UAPSD_ID]) {
			sta->ps.pkt_ready[UAPSD_ID] = 0;
			cls_wifi_set_traffic_status(cls_wifi_hw, sta, false, UAPSD_ID);
		}
	}
}

/**
 * cls_wifi_ps_bh_traffic_req - Handle traffic request for STA in PS mode
 *
 * @cls_wifi_hw: Driver main data
 * @sta: Sta which enters/leaves PS mode
 * @pkt_req: number of pkt to push
 * @ps_id: type of PS data requested (@LEGACY_PS_ID or @UAPSD_ID)
 *
 * This function will make sure that @pkt_req are pushed to fw
 * whereas the STA is in PS mode.
 * If request is 0, send all traffic
 * If request is greater than available pkt, reduce request
 * Note: request will also be reduce if txq credits are not available
 *
 * All counter updates are protected from TX path by taking tx_lock
 *
 * NOTE: _bh_ in function name indicates that this function is called
 * from the bottom_half tasklet.
 */
void cls_wifi_ps_bh_traffic_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
							u16 pkt_req, u8 ps_id)
{
	int pkt_ready_all;
	struct cls_wifi_txq *txq;

	if ((!is_multicast_sta(cls_wifi_hw, sta->sta_idx)) && (!sta->aid)) {
		if (WARN(!sta->ps.active, "sta %pM is not in Power Save mode",
				 sta->mac_addr))
			return;
	}

	trace_ps_traffic_req(sta, pkt_req, ps_id);

	spin_lock(&cls_wifi_hw->tx_lock);

	/* Fw may ask to stop a service period with PS_SP_INTERRUPTED. This only
	   happens for p2p-go interface if NOA starts during a service period */
	if ((pkt_req == PS_SP_INTERRUPTED) && (ps_id == UAPSD_ID)) {
		int tid;
		sta->ps.sp_cnt[ps_id] = 0;
		foreach_sta_txq(sta, txq, tid, cls_wifi_hw) {
			txq->push_limit = 0;
		}
		goto done;
	}

	pkt_ready_all = (sta->ps.pkt_ready[ps_id] - sta->ps.sp_cnt[ps_id]);

	/* Don't start SP until previous one is finished or we don't have
	   packet ready (which must not happen for U-APSD) */
	if (sta->ps.sp_cnt[ps_id] || pkt_ready_all <= 0) {
		goto done;
	}

	/* Adapt request to what is available. */
	if (pkt_req == 0 || pkt_req > pkt_ready_all) {
		pkt_req = pkt_ready_all;
	}

	/* Reset the SP counter */
	sta->ps.sp_cnt[ps_id] = 0;

	/* "dispatch" the request between txq */
	if (is_multicast_sta(cls_wifi_hw, sta->sta_idx)) {
		txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
		if ((txq->credits <= 0) || (txq->idx == TXQ_INACTIVE))
			goto done;
		if (pkt_req > txq->credits)
			pkt_req = txq->credits;
		txq->push_limit = pkt_req;
		sta->ps.sp_cnt[ps_id] = pkt_req;
		if (!skb_queue_empty(&txq->sk_list))
			cls_wifi_txq_add_to_hw_list(txq);
	} else {
		int i, tid;

		foreach_sta_txq_prio(sta, txq, tid, i, cls_wifi_hw) {
			u16 txq_len = skb_queue_len(&txq->sk_list);

			if ((txq->ps_id != ps_id) || (txq->idx == TXQ_INACTIVE))
				continue;

			if (txq_len > txq->credits)
				txq_len = txq->credits;

			if (txq_len == 0)
				continue;

			if (txq_len < pkt_req) {
				/* Not enough pkt queued in this txq, add this
				   txq to hwq list and process next txq */
				pkt_req -= txq_len;
				txq->push_limit = txq_len;
				sta->ps.sp_cnt[ps_id] += txq_len;
				cls_wifi_txq_add_to_hw_list(txq);
			} else {
				/* Enough pkt in this txq to comlete the request
				   add this txq to hwq list and stop processing txq */
				txq->push_limit = pkt_req;
				sta->ps.sp_cnt[ps_id] += pkt_req;
				cls_wifi_txq_add_to_hw_list(txq);
				break;
			}
		}
	}

  done:
	spin_unlock(&cls_wifi_hw->tx_lock);
}

/******************************************************************************
 * TX functions
 *****************************************************************************/
#define PRIO_STA_NULL 0xAA

static const int cls_wifi_down_hwq2tid[3] = {
	[CLS_WIFI_HWQ_BK] = 2,
	[CLS_WIFI_HWQ_BE] = 3,
	[CLS_WIFI_HWQ_VI] = 5,
};

/**
 * cls_wifi_downgrade_ac - Downgrade selected priority as long as corresponding AC
 * requires ACM (Admission Control Mandatory).
 * @sta: Destination station
 * @skb: Buffer to send
 *
 * If destination station requires ACM for the selected AC since this is not
 * supported reduce skb priority to use the lower AC.
 */
static void cls_wifi_downgrade_ac(struct cls_wifi_sta *sta, struct sk_buff *skb)
{
	int8_t ac = cls_wifi_tid2hwq[skb->priority];

	if (WARN((ac > CLS_WIFI_HWQ_VO),
			 "Unexepcted ac %d for skb before downgrade", ac))
		ac = CLS_WIFI_HWQ_VO;

	while (sta->acm & BIT(ac)) {
		if (ac == CLS_WIFI_HWQ_BK) {
			skb->priority = 1;
			return;
		}
		ac--;
		skb->priority = cls_wifi_down_hwq2tid[ac];
	}
}

/**
 * cls_wifi_tx_statistic - Update TX statistics after packet transmission
 * @vif: Interface that sent the packet
 * @txq: TXQ used to send the packet
 * @status: Transmission status
 * @data_len: Size of the packet payload
 */
static void cls_wifi_tx_statistic(struct cls_wifi_vif *vif, struct cls_wifi_txq *txq,
							  u32 status, u32 rate_config, unsigned int data_len,int is_mgmt)
{
	struct cls_wifi_sta *sta = txq->sta;
#ifdef CONFIG_CLS_WIFI_DEBUGFS
	struct cls_wifi_rate_stats *rate_stats;
	int mcs, bw, gi, preamble, nss, format_mod, rate_idx;
#endif

	if (!TXU_STATUS_IS_SET(ACKNOWLEDGED, status))
		return;

	if (!sta)
		return;

	if(is_mgmt) {
		sta->stats.tx_mgmt_pkts++;
		return;
	}

	sta->stats.tx_pkts++;
	sta->stats.tx_bytes += data_len;
	sta->stats.last_act = jiffies;
	cls_wtm_update_stats(vif->cls_wifi_hw, vif, sta, data_len, 0, false);

#ifdef CONFIG_CLS_WIFI_DEBUGFS
	if (rate_config == RC_UNKNOWN_RATE)
		return;

	rate_stats = &sta->stats.tx_rate;
	format_mod = RC_RATE_GET(FORMAT_MOD, rate_config);
	mcs = RC_RATE_GET(MCS, rate_config);
	nss = RC_RATE_GET(NSS, rate_config);
	preamble = RC_RATE_GET(LONG_PREAMBLE, rate_config);
	gi = RC_RATE_GET(GI, rate_config);
	if ((format_mod == FORMATMOD_HE_MU) ||
		(format_mod == FORMATMOD_HE_TB))
		bw = RC_RATE_GET(RU_SIZE, rate_config);
	else
		bw = RC_RATE_GET(BW, rate_config);
	rate_idx = cls_wifi_dbgfs_rate_idx(format_mod, bw, mcs, gi, nss, preamble);

	if ((rate_idx < 0) || rate_idx > (rate_stats->size)) {
		netdev_err(vif->ndev, "TX: Invalid index conversion: format_mod=%d mcs=%d bw=%d sgi=%d nss=%d\n",
				  format_mod, mcs, bw, gi, nss);
	} else {

		if (!rate_stats->table[rate_idx])
			rate_stats->rate_cnt++;
		rate_stats->table[rate_idx]++;
		rate_stats->cpt++;
	}

#endif
	sta->stats.last_tx = rate_config;
}

/**
 * cls_wifi_tx_update_frame_len - Update frame_len in a TX descriptor of a buffer
 * already queued.
 * @cls_wifi_hw: Driver main
 * @sw_txhdr: TX descriptor to update
 * @frame_len: New frame length to configure
 */
static inline void cls_wifi_tx_update_frame_len(struct cls_wifi_hw *cls_wifi_hw,
											struct cls_wifi_sw_txhdr *sw_txhdr,
											size_t frame_len)
{
	int delta = frame_len - sw_txhdr->frame_len;

	cls_wifi_ipc_sta_buffer(cls_wifi_hw, sw_txhdr->txq->sta, sw_txhdr->txq->tid, delta);
	sw_txhdr->frame_len = frame_len;
}

/**
 * cls_wifi_select_txq - Select TXQ to used for a packet transmission based on
 * data to send
 * @cls_wifi_vif: Interface used for the transmission
 * @skb: Packet to send
 * @return netdev ID of the selected TXQ
 *
 * The function first compute the station to which this packet must be transmitted.
 * Then if destination support QoS the TID is read from the packet data,
 * and saved in skb->priority (so that it can be retrieved by cls_wifi_get_tx_info).
 *
 * This function is a netdev callback and is called by network stack before it is
 * pushed to the driver (via cls_wifi_start_xmit)
 */
u16 cls_wifi_select_txq(struct cls_wifi_vif *cls_wifi_vif, struct sk_buff *skb)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct wireless_dev *wdev = &cls_wifi_vif->wdev;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_txq *txq;
	u16 netdev_queue;
	bool tdls_mgmgt_frame = false;

#ifdef CONFIG_CLS_FWT
	if (skb->npe_meta_data.valid && CLS_IEEE80211_VIF_IDX_VALID(skb->dest_port)) {
		int sta_idx;

		skb->priority = skb->npe_meta_data.tid;
		if (skb->npe_meta_data.destWPU == 0)
			sta_idx = CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(skb->dest_port);
		else
			sta_idx = skb->npe_meta_data.sta_index_or_flow_id;
		sta = &cls_wifi_hw->sta_table[sta_idx];
		if (sta && sta->valid != false) {
			if (sta->qos) {
				return CLS_STA_NDEV_IDX(skb->priority, sta_idx);
			} else {
				skb->priority = 0xFF;
				txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
				netdev_queue = txq->ndev_idx;

				return netdev_queue;
			}
		}
		sta = NULL;
	}
#endif

	switch (wdev->iftype) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
	{
		struct ethhdr *eth;
		eth = (struct ethhdr *)skb->data;
		if (eth->h_proto == cpu_to_be16(ETH_P_TDLS)) {
			tdls_mgmgt_frame = true;
		}
		if ((cls_wifi_vif->tdls_status == TDLS_LINK_ACTIVE) &&
			(cls_wifi_vif->sta.tdls_sta != NULL) &&
			(memcmp(eth->h_dest, cls_wifi_vif->sta.tdls_sta->mac_addr, ETH_ALEN) == 0))
			sta = cls_wifi_vif->sta.tdls_sta;
		else
			sta = cls_wifi_vif->sta.ap;
		break;
	}
	case NL80211_IFTYPE_AP_VLAN:
		if (cls_wifi_vif->ap_vlan.sta_4a) {
			sta = cls_wifi_vif->ap_vlan.sta_4a;
			break;
		}

		/* AP_VLAN interface is not used for a 4A STA,
		   fallback searching sta amongs all AP's clients */
		cls_wifi_vif = cls_wifi_vif->ap_vlan.master;
		fallthrough;
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_P2P_GO:
	{
		struct cls_wifi_sta *cur;
		struct ethhdr *eth = (struct ethhdr *)skb->data;
#ifdef CONFIG_CLS_FWT
		u32 vlan_id = 0;
		u16 entry_index = 0;
		u32 entry_sub_port = 0;
		int sta_idx;
		struct vlan_ethhdr *vhdr;

		if (eth->h_proto == htons(ETH_P_8021Q)) {
			vhdr = (struct vlan_ethhdr *)eth;
			vlan_id = ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK;
		}

		if (!vlan_id)
			vlan_id = br_port_get_pvid(skb->dev);
#endif

		if (is_multicast_ether_addr(eth->h_dest)) {
			sta = &cls_wifi_hw->sta_table[cls_wifi_vif->ap.bcmc_index];
		} else {
#ifdef CONFIG_CLS_FWT
			if (CLS_IEEE80211_NODE_IDX_VALID(skb->dest_port)) {//forward fron npe directly
				sta_idx = CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(skb->dest_port);
				if (sta_idx < hw_all_sta_max(cls_wifi_hw))
					sta = &cls_wifi_hw->sta_table[sta_idx];
			} else if (g_get_fwt_entry_info_hook &&
				g_get_fwt_entry_info_hook(eth->h_dest, vlan_id,
								&entry_index, &entry_sub_port)) {
				sta_idx = CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(entry_sub_port);
				if (sta_idx < hw_all_sta_max(cls_wifi_hw))
					sta = &cls_wifi_hw->sta_table[sta_idx];
			}
#endif
			if ((sta == NULL) || (sta && sta->valid == false)) {
				sta = NULL;
				list_for_each_entry(cur, &cls_wifi_vif->ap.sta_list, list) {
					if (!memcmp(cur->mac_addr, eth->h_dest, ETH_ALEN)) {
						sta = cur;
						break;
					}
				}
			}
		}

		break;
	}
	case NL80211_IFTYPE_MESH_POINT:
	{
		struct ethhdr *eth = (struct ethhdr *)skb->data;

		if (!cls_wifi_vif->is_resending) {
			/*
			 * If ethernet source address is not the address of a mesh wireless interface, we are proxy for
			 * this address and have to inform the HW
			 */
			if (memcmp(&eth->h_source[0], &cls_wifi_vif->ndev->perm_addr[0], ETH_ALEN)) {
				/* Check if LMAC is already informed */
				if (!cls_wifi_get_mesh_proxy_info(cls_wifi_vif, (u8 *)&eth->h_source, true)) {
					cls_wifi_send_mesh_proxy_add_req(cls_wifi_hw, cls_wifi_vif, (u8 *)&eth->h_source);
				}
			}
		}

		if (is_multicast_ether_addr(eth->h_dest)) {
			sta = &cls_wifi_hw->sta_table[cls_wifi_vif->ap.bcmc_index];
		} else {
			/* Path to be used */
			struct cls_wifi_mesh_path *p_mesh_path = NULL;
			struct cls_wifi_mesh_path *p_cur_path;
			/* Check if destination is proxied by a peer Mesh STA */
			struct cls_wifi_mesh_proxy *p_mesh_proxy = cls_wifi_get_mesh_proxy_info(cls_wifi_vif, (u8 *)&eth->h_dest, false);
			/* Mesh Target address */
			struct mac_addr *p_tgt_mac_addr;

			if (p_mesh_proxy) {
				p_tgt_mac_addr = &p_mesh_proxy->proxy_addr;
			} else {
				p_tgt_mac_addr = (struct mac_addr *)&eth->h_dest;
			}

			/* Look for path with provided target address */
			list_for_each_entry(p_cur_path, &cls_wifi_vif->ap.mpath_list, list) {
				if (!memcmp(&p_cur_path->tgt_mac_addr, p_tgt_mac_addr, ETH_ALEN)) {
					p_mesh_path = p_cur_path;
					break;
				}
			}

			if (p_mesh_path) {
				sta = p_mesh_path->nhop_sta;
			} else {
				cls_wifi_send_mesh_path_create_req(cls_wifi_hw, cls_wifi_vif, (u8 *)p_tgt_mac_addr);
			}
		}

		break;
	}
	default:
		break;
	}

	if (sta && sta->qos)
	{
		if (tdls_mgmgt_frame) {
			skb_set_queue_mapping(skb, CLS_STA_NDEV_IDX(skb->priority, sta->sta_idx));
		} else {
			skb->priority = cfg80211_classify8021d(skb, NULL) & IEEE80211_QOS_CTL_TAG1D_MASK;
		}
		if (sta->acm)
			cls_wifi_downgrade_ac(sta, skb);

		txq = cls_wifi_txq_sta_get(sta, skb->priority, cls_wifi_hw);
		netdev_queue = txq->ndev_idx;
	}
	else if (sta)
	{
		skb->priority = 0xFF;
		txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
		netdev_queue = txq->ndev_idx;
	}
	else
	{
		/* This packet will be dropped in xmit function, still need to select
		   an active queue for xmit to be called. As it most likely to happen
		   for AP interface, select BCMC queue
		   (TODO: select another queue if BCMC queue is stopped) */
		skb->priority = PRIO_STA_NULL;
		netdev_queue = bcmc_txq_ndev_idx(cls_wifi_hw);
	}

	if (netdev_queue >= nb_ndev_txq(cls_wifi_hw)) {
#ifdef CONFIG_CLS_FWT
		pr_warn("%s %d radio%d queue %d nb_ndev_txq %d sta %px prio %d, dest_port: %x\n", __func__, __LINE__,
				cls_wifi_hw->radio_idx, netdev_queue, nb_ndev_txq(cls_wifi_hw), sta,
				skb->priority, skb->dest_port);
#else
		pr_warn("%s %d radio%d queue %d nb_ndev_txq %d sta %px prio %d\n", __func__, __LINE__,
				cls_wifi_hw->radio_idx, netdev_queue, nb_ndev_txq(cls_wifi_hw), sta, skb->priority);
#endif
		if (sta)
			pr_warn("%s %d sta idx %d qos %d acm %d mac %pM, valid: %d, size: %zu\n", __func__, __LINE__,
					sta->sta_idx, sta->qos, sta->acm, sta->mac_addr, sta->valid, sizeof(*sta));
		if (txq)
			pr_warn("%s %d txq:%px, idx: 0x%x(%u)\n", __func__, __LINE__,
				txq, txq->idx, txq->idx);
		print_hex_dump(KERN_WARNING, "skb ", DUMP_PREFIX_OFFSET, 16, 1, skb->data,
				skb->len, false);
	}
	BUG_ON(netdev_queue >= nb_ndev_txq(cls_wifi_hw));

	return netdev_queue;
}

/**
 * cls_wifi_set_more_data_flag - Update MORE_DATA flag in tx sw desc
 *
 * @cls_wifi_hw: Driver main data
 * @sw_txhdr: Header for pkt to be pushed
 *
 * If STA is in PS mode
 *  - Set EOSP in case the packet is the last of the UAPSD service period
 *  - Set MORE_DATA flag if more pkt are ready for this sta
 *  - Update TIM if this is the last pkt buffered for this sta
 *
 * note: tx_lock already taken.
 */
static inline void cls_wifi_set_more_data_flag(struct cls_wifi_hw *cls_wifi_hw,
										   struct cls_wifi_sw_txhdr *sw_txhdr)
{
	struct cls_wifi_sta *sta = sw_txhdr->cls_wifi_sta;
	struct cls_wifi_vif *vif = sw_txhdr->cls_wifi_vif;
	struct cls_wifi_txq *txq = sw_txhdr->txq;

	if (unlikely(sta->ps.active)) {
		sta->ps.pkt_ready[txq->ps_id]--;
		sta->ps.sp_cnt[txq->ps_id]--;

		trace_ps_push(sta);

		if (((txq->ps_id == UAPSD_ID) || (vif->wdev.iftype == NL80211_IFTYPE_MESH_POINT) || (sta->tdls.active))
				&& !sta->ps.sp_cnt[txq->ps_id]) {
			sw_txhdr->desc.api.host.flags |= TXU_CNTRL_EOSP;
		}

		if (sta->ps.pkt_ready[txq->ps_id]) {
			sw_txhdr->desc.api.host.flags |= TXU_CNTRL_MORE_DATA;
		} else {
			cls_wifi_set_traffic_status(cls_wifi_hw, sta, false, txq->ps_id);
		}
	}
}

/**
 * cls_wifi_get_tx_info - Get STA and tid for one skb
 *
 * @cls_wifi_vif: vif ptr
 * @skb: skb
 * @tid: pointer updated with the tid to use for this skb
 *
 * @return: pointer on the destination STA (may be NULL)
 *
 * skb has already been parsed in cls_wifi_select_queue function
 * simply re-read information form skb.
 */
static struct cls_wifi_sta *cls_wifi_get_tx_info(struct cls_wifi_vif *cls_wifi_vif,
										 struct sk_buff *skb,
										 u8 *tid)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct cls_wifi_sta *sta;
	u16 sta_idx;

	*tid = skb->priority;
	if (unlikely(skb->priority == PRIO_STA_NULL)) {
		return NULL;
	} else {
		int ndev_idx = skb_get_queue_mapping(skb);

		if (ndev_idx == bcmc_txq_ndev_idx(cls_wifi_hw))
			sta_idx = hw_remote_sta_max(cls_wifi_hw) + master_vif_idx(cls_wifi_vif);
		else
			sta_idx = ndev_idx / CLS_NB_TID_PER_STA;

		sta = &cls_wifi_hw->sta_table[sta_idx];
	}

	return sta;
}

/**
 * cls_wifi_prep_dma_tx - Prepare buffer for DMA transmission
 *
 * @cls_wifi_hw: Driver main data
 * @sw_txhdr: Software Tx descriptor
 * @frame_start: Pointer to the beginning of the frame that needs to be DMA mapped
 * @return: 0 on success, -1 on error
 *
 * Map the frame for DMA transmission and save its ipc address in the tx descriptor
 */
static int cls_wifi_prep_dma_tx(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sw_txhdr *sw_txhdr,
							void *frame_start)
{
	struct txdesc_api *desc = &sw_txhdr->desc.api;

	if (cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &sw_txhdr->ipc_data, frame_start,
							  sw_txhdr->frame_len))
		return -1;

	/* Update DMA addresses and length in tx descriptor */
	desc->host.packet_len[0] = sw_txhdr->frame_len;
	desc->host.packet_addr[0] = sw_txhdr->ipc_data.dma_addr;
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
	desc->host.packet_cnt = 1;
#endif

	return 0;
}

static void cls_wifi_tx_retry(struct cls_wifi_hw *cls_wifi_hw, struct sk_buff *skb,
						  struct cls_wifi_sw_txhdr *sw_txhdr, u32 status);
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
static void cls_wifi_amsdu_update_len(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq,
								  u16 amsdu_len);
static void cls_wifi_amsdu_del_subframe_header(struct cls_wifi_hw *cls_wifi_hw,
										   struct cls_wifi_amsdu_txhdr *amsdu_txhdr);
#endif

/**
 *  cls_wifi_tx_push - Push one packet to fw
 *
 * @cls_wifi_hw: Driver main data
 * @txhdr: tx desc of the buffer to push
 * @flags: push flags (see @cls_wifi_push_flags)
 *
 * Push one packet to fw. Sw desc of the packet has already been updated.
 * Only MORE_DATA flag will be set if needed.
 */
void cls_wifi_tx_push(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txhdr *txhdr, int flags)
{
	struct cls_wifi_sw_txhdr *sw_txhdr = txhdr->sw_hdr;
	struct sk_buff *skb = sw_txhdr->skb;
	struct cls_wifi_txq *txq = sw_txhdr->txq;
	u16 hw_queue = txq->hwq->id;

	lockdep_assert_held(&cls_wifi_hw->tx_lock);

	/* RETRY flag is not always set so retest here */
	if (txq->nb_retry) {
		flags |= CLS_WIFI_PUSH_RETRY;
		txq->nb_retry--;
		if (txq->nb_retry == 0) {
			WARN(skb != txq->last_retry_skb,
				 "last retry buffer is not the expected one");
			txq->last_retry_skb = NULL;
		}
	} else if (!(flags & CLS_WIFI_PUSH_RETRY)) {
		txq->pkt_sent++;
	}

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (txq->amsdu == sw_txhdr && sw_txhdr->amsdu.nb > 0) {
		WARN((flags & CLS_WIFI_PUSH_RETRY), "End A-MSDU on a retry");
		cls_wifi_hw->stats.amsdus[sw_txhdr->amsdu.nb - 1].done++;
		txq->amsdu = NULL;
	} else if (!(flags & CLS_WIFI_PUSH_RETRY) &&
			   !(sw_txhdr->desc.api.host.flags & TXU_CNTRL_AMSDU)) {
		cls_wifi_hw->stats.amsdus[0].done++;
	}
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */

	/* Wait here to update hw_queue, as for multicast STA hwq may change
	   between queue and push (because of PS) */
	sw_txhdr->hw_queue = hw_queue;

	if (sw_txhdr->cls_wifi_sta) {
		/* only for AP mode */
		cls_wifi_set_more_data_flag(cls_wifi_hw, sw_txhdr);
	}

#ifdef CONFIG_CLS_WIFI_HEMU_TX
	hw_queue = cls_wifi_he_mu_set_user(cls_wifi_hw, sw_txhdr);
#endif
	cls_wifi_ipc_txdesc_push(cls_wifi_hw, sw_txhdr, skb, hw_queue,
			(txq->credits - 1), (txq->hwq->credits - 1), (txq->pkt_pushed + 1));
	trace_push_desc(skb, sw_txhdr, flags);

	txq->credits--;
	if (txq->credits <= 0)
		cls_wifi_txq_stop(txq, CLS_WIFI_TXQ_STOP_FULL);
	if (txq->push_limit)
		txq->push_limit--;
	txq->pkt_pushed++;
	txq->hwq->credits--;
	txq->hwq_credits_quota--;
	cls_wifi_hw->stats.cfm_balance[hw_queue]++;
}


/**
 * cls_wifi_tx_retry - Re-queue a pkt that has been postponed by firmware
 *
 * @cls_wifi_hw: Driver main data
 * @skb: pkt to re-push
 * @sw_txhdr: software TX desc of the pkt to re-push
 * @status: Status on the transmission
 *
 * Called when a packet needs to be repushed to the firmware, because firmware
 * wasn't able to process it when first pushed (e.g. the station enter PS after
 * the driver first pushed this packet to the firmware).
 */
static void cls_wifi_tx_retry(struct cls_wifi_hw *cls_wifi_hw, struct sk_buff *skb,
						  struct cls_wifi_sw_txhdr *sw_txhdr, u32 status)
{
	struct cls_wifi_txq *txq = sw_txhdr->txq;
	u16 sn = 4096;

	/* MORE_DATA will be re-set if needed when pkt will be repushed */
	sw_txhdr->desc.api.host.flags &= ~TXU_CNTRL_MORE_DATA;

	if (TXU_STATUS_IS_SET(RETRY_REQUIRED, status)) {
		// Firmware already tried to send the buffer but cannot retry it now
		// On next push, firmware needs to re-use the same SN
		sn = TXU_STATUS_GET(SN, status);
		sw_txhdr->desc.api.host.flags |= TXU_CNTRL_REUSE_SN;
		sw_txhdr->desc.api.host.sn_for_retry = sn;
	}

	txq->credits++;
	trace_skb_retry(skb, txq, sn);
	if (txq->credits > 0)
		cls_wifi_txq_start(txq, CLS_WIFI_TXQ_STOP_FULL);

	/* Queue the buffer */
	cls_wifi_txq_queue_skb(skb, txq, cls_wifi_hw, true, NULL);
}


#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
/**
 * cls_wifi_amsdu_subframe_length - return size of A-MSDU subframe including
 * header but without padding
 *
 * @eth: Ethernet Header of the frame
 * @frame_len: Length of the ethernet frame (including ethernet header)
 * @return: length of the A-MSDU subframe
 */
static inline int cls_wifi_amsdu_subframe_length(__be16 h_proto, int frame_len)
{
	/* ethernet header is replaced with amdsu header that have the same size
	   Only need to check if LLC/SNAP header will be added */
	int len = frame_len;

	if (eth_proto_is_802_3(h_proto)) {
		len += sizeof(rfc1042_header) + 2;
	}

	return len;
}

static inline bool cls_wifi_amsdu_is_aggregable(struct sk_buff *skb)
{
	/* need to add some check on buffer to see if it can be aggregated ? */
	return true;
}

/**
 * cls_wifi_amsdu_del_subframe_header - remove AMSDU header
 *
 * @cls_wifi_hw: Driver main data
 * @amsdu_txhdr: amsdu tx descriptor
 *
 * Move back the ethernet header at the "beginning" of the data buffer.
 * (which has been moved in @cls_wifi_amsdu_add_subframe_header)
 */
static void cls_wifi_amsdu_del_subframe_header(struct cls_wifi_hw *cls_wifi_hw,
										   struct cls_wifi_amsdu_txhdr *amsdu_txhdr)
{
	struct sk_buff *skb = amsdu_txhdr->skb;
	struct ethhdr *eth;
	u8 *pos;
	__be16 h_proto;
#if ETH_OPTI_DDR_CFG
	struct ethhdr eth_temp;
#endif

	BUG_ON(skb == NULL);
	pos = skb->data;
	pos += sizeof(struct cls_wifi_amsdu_txhdr);
	if (!cls_wifi_hw->has_hw_llcsnap_insert) {
		eth = (struct ethhdr*)pos;
		#if ETH_OPTI_DDR_CFG
		memcpy(&eth_temp, eth, sizeof(*eth));
		h_proto = eth_temp.h_proto;
		#else
		h_proto = eth->h_proto;
		#endif
		pos += amsdu_txhdr->pad + sizeof(struct ethhdr);

		if (eth_proto_is_802_3(h_proto)) {
			pos += sizeof(rfc1042_header) + 2;
		}

		#if ETH_OPTI_DDR_CFG
		memmove(pos, &eth_temp, sizeof(*eth));
		#else
		memmove(pos, eth, sizeof(*eth));
		#endif
	}
	skb_pull(skb, (pos - skb->data));
}

static inline void cls_wifi_skb2sk_list(struct cls_wifi_sw_txhdr *sw_txhdr,
		struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq)
{
	if (sw_txhdr == NULL)
		return;

	if ((cls_wifi_hw == NULL) || (txq == NULL))
		return;

	if (!sw_txhdr->queued) {
		if (cls_wifi_txq_queue_skb(sw_txhdr->skb, txq, cls_wifi_hw, false, NULL))
			cls_wifi_hwq_process(cls_wifi_hw, txq->hwq);
	}
}

static inline void cls_wifi_amsdu_agg_handler(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq)
{
	struct cls_wifi_sw_txhdr *sw_txhdr;

	sw_txhdr = txq->wait2txq;
	if (sw_txhdr != NULL) {
		cls_wifi_skb2sk_list(sw_txhdr, cls_wifi_hw, txq);
		txq->wait2txq = NULL;
	}
}

void cls_wifi_amsdu_agg_timer_stop(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq)
{
	if (txq == NULL)
		return;

	if (cls_wifi_hw == NULL)
		return;

	hrtimer_try_to_cancel(&txq->amsdu_agg_timer);

	cls_wifi_amsdu_agg_handler(cls_wifi_hw, txq);
}

enum hrtimer_restart cls_wifi_amsdu_agg_timeout(struct hrtimer *timer)
{
	struct cls_wifi_txq *txq = container_of(timer, struct cls_wifi_txq, amsdu_agg_timer);
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(txq->ndev);
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;

	spin_lock_bh(&cls_wifi_hw->tx_lock);
	cls_wifi_amsdu_agg_handler(cls_wifi_hw, txq);
	spin_unlock_bh(&cls_wifi_hw->tx_lock);

	return HRTIMER_NORESTART;
}


/**
 * cls_wifi_amsdu_add_subframe_header - Add AMSDU header and link subframe
 *
 * @cls_wifi_hw Driver main data
 * @skb Buffer to aggregate
 * @sw_txhdr Tx descriptor for the first A-MSDU subframe
 *
 * return 0 on success, -1 otherwise
 *
 * In case the HW/FW does not support the automatic insertion of the
 * LLC/SNAP Header, this function adds A-MSDU header and LLC/SNAP header
 * in the buffer .
 *
 *
 *			Before		   After
 *		 +-------------+  +-------------+
 *		 | HEADROOM	|  | HEADROOM	|
 *		 |			 |  +-------------+ <- data
 *		 |			 |  | amsdu_txhdr |
 *		 |			 |  | * pad size  |
 *		 |			 |  +-------------+
 *		 |			 |  | ETH hdr	 | keep original eth hdr
 *		 |			 |  |			 | to restore it once transmitted
 *		 |			 |  +-------------+ <- packet_addr[x]
 *		 |			 |  | Pad		 |
 *		 |			 |  +-------------+
 * data -> +-------------+  | AMSDU HDR   |
 *		 | ETH hdr	 |  +-------------+
 *		 |			 |  | LLC/SNAP	|
 *		 +-------------+  +-------------+
 *		 | DATA		|  | DATA		|
 *		 |			 |  |			 |
 *		 +-------------+  +-------------+
 *
 * In case the LLC/SNAP (and length) is automatically inserted by the HW,
 * only the AMSDU TX header is added in the headroom:
 *
 *			Before		   After
 *		 +-------------+  +-------------+
 *		 | HEADROOM	|  | HEADROOM	|
 *		 |			 |  +-------------+ <- data
 *		 |			 |  | amsdu_txhdr |
 * data -> +-------------+  +-------------+ <- packet_addr[x]
 *		 | ETH hdr	 |  | ETH hdr	 |
 *		 +-------------+  +-------------+
 *		 | DATA		|  | DATA		|
 *		 |			 |  |			 |
 *		 |			 |  |			 |
 *		 +-------------+  +-------------+
 *
 * The function then updates sw_txhdr of the first subframe to link this
 * buffer.
 * If an error happens, the buffer will be queued as a normal buffer.
 * Called with tx_lock hold
 */
static int cls_wifi_amsdu_add_subframe_header(struct cls_wifi_hw *cls_wifi_hw,
										  struct sk_buff *skb,
										  struct cls_wifi_sw_txhdr *sw_txhdr,
										  struct ethhdr *eth_t)
{
	struct cls_wifi_amsdu *amsdu = &sw_txhdr->amsdu;
	struct cls_wifi_amsdu_txhdr *amsdu_txhdr;
#if !CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD
	struct ethhdr *amsdu_hdr, *eth = (struct ethhdr *)skb->data;
#else
	struct ethhdr *eth = (struct ethhdr *)skb->data;
#endif
	int headroom_need, msdu_len, amsdu_len, map_len, llc_snap_len = 0;
	u8 *pos, *amsdu_start;
	__be16 h_proto;

	if (eth_t)
		h_proto = eth_t->h_proto;
	else
		h_proto = eth->h_proto;

	if (eth_proto_is_802_3(h_proto))
		llc_snap_len = sizeof(rfc1042_header) + 2;

	msdu_len = skb->len;
	amsdu_len = msdu_len + amsdu->pad + llc_snap_len;
	headroom_need = sizeof(*amsdu_txhdr);

#if !CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD
	if (!cls_wifi_hw->has_hw_llcsnap_insert) {
		headroom_need += amsdu->pad + sizeof(*amsdu_hdr) + llc_snap_len;
		msdu_len = msdu_len - sizeof(*eth) + llc_snap_len;
	}
#endif

	/* we should have enough headroom (checked in xmit) */
	if (WARN_ON(skb_headroom(skb) < headroom_need)) {
		return -1;
	}

	/* allocate headroom */
	pos = skb_push(skb, headroom_need);
	amsdu_txhdr = (struct cls_wifi_amsdu_txhdr *)pos;
	pos += sizeof(*amsdu_txhdr);

#if !CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD
	if (!cls_wifi_hw->has_hw_llcsnap_insert) {
		/* move eth header */
		if (eth_t)
			memmove(pos, eth_t, sizeof(*eth));
		else
			memmove(pos, eth, sizeof(*eth));
		skb_set_mac_header(skb, pos - skb->data);
		eth = (struct ethhdr *)pos;
		pos += sizeof(*eth);

		/* Add padding from previous subframe */
		amsdu_start = pos;
		memset(pos, 0, amsdu->pad);
		pos += amsdu->pad;

		/* Add AMSDU hdr */
		amsdu_hdr = (struct ethhdr *)pos;
		if (eth_t) {
			memcpy(amsdu_hdr->h_dest, eth_t->h_dest, ETH_ALEN);
			memcpy(amsdu_hdr->h_source, eth_t->h_source, ETH_ALEN);
		} else {
			memcpy(amsdu_hdr->h_dest, eth->h_dest, ETH_ALEN);
			memcpy(amsdu_hdr->h_source, eth->h_source, ETH_ALEN);
		}
		amsdu_hdr->h_proto = htons(msdu_len);
		pos += sizeof(*amsdu_hdr);

		if (llc_snap_len) {
			memcpy(pos, rfc1042_header, sizeof(rfc1042_header));
			pos += llc_snap_len;
		}

		map_len = amsdu_len;
	} else {
#endif
		amsdu_start = pos;
		map_len = msdu_len;
#if !CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD
	}
#endif

	/* Prepare IPC buffer for DMA transfer */
	if (cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &amsdu_txhdr->ipc_data, amsdu_start, map_len)) {
		netdev_err(skb->dev, "Failed to add A-MSDU header\n");
		if (!cls_wifi_hw->has_hw_llcsnap_insert) {
			pos -= sizeof(*eth);
			if (eth_t)
				memmove(pos, eth_t, sizeof(*eth));
			else
				memmove(pos, eth, sizeof(*eth));
		}
		skb_pull(skb, headroom_need);
		return -1;
	}

	/* update amdsu_txhdr */
	amsdu_txhdr->skb = skb;
	amsdu_txhdr->pad = amsdu->pad;

	///for SITS #4933
	if (amsdu->nb != sw_txhdr->desc.api.host.packet_cnt) {
		pr_err("[error]%s amsdu(%px)->nb: %d,sw_txhdr(%px)->desc.api.host.packet_cnt: %d,txq:%px,sta_idx: %d,idx: 0x%x,credits:%d,ps_id:%d, list_empty:%d\n",
			__func__, amsdu, amsdu->nb, sw_txhdr, sw_txhdr->desc.api.host.packet_cnt,
			sw_txhdr->txq, sw_txhdr->txq->sta->sta_idx, sw_txhdr->txq->idx,
			sw_txhdr->txq->credits, sw_txhdr->txq->ps_id, list_empty(&amsdu->hdrs));
	}

	/* update cls_wifi_sw_txhdr (of the first subframe) */
	BUG_ON(amsdu->nb != sw_txhdr->desc.api.host.packet_cnt);
	sw_txhdr->desc.api.host.packet_addr[amsdu->nb] = amsdu_txhdr->ipc_data.dma_addr;
	sw_txhdr->desc.api.host.packet_len[amsdu->nb] = map_len;
	sw_txhdr->desc.api.host.packet_cnt++;
#if CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD == 1
	memcpy(&sw_txhdr->desc.api.host.eth_dest_addr[amsdu->nb], eth->h_dest, ETH_ALEN);
	memcpy(&sw_txhdr->desc.api.host.eth_src_addr[amsdu->nb], eth->h_source, ETH_ALEN);
	sw_txhdr->desc.api.host.ethertype[amsdu->nb] = eth->h_proto;
#endif
	cls_wifi_tx_update_frame_len(cls_wifi_hw, sw_txhdr, sw_txhdr->frame_len + amsdu_len);
	amsdu->nb++;
	amsdu->pad = AMSDU_PADDING(amsdu_len - amsdu->pad);
	list_add_tail(&amsdu_txhdr->list, &amsdu->hdrs);

	trace_amsdu_subframe(sw_txhdr);
	return 0;
}

/**
 * cls_wifi_amsdu_add_subframe - Add this buffer as an A-MSDU subframe if possible
 *
 * @cls_wifi_hw Driver main data
 * @skb Buffer to aggregate if possible
 * @sta Destination STA
 * @txq sta's txq used for this buffer
 *
 * Try to aggregate the buffer in an A-MSDU. If it succeed then the
 * buffer is added as a new A-MSDU subframe with AMSDU and LLC/SNAP
 * headers added (so FW won't have to modify this subframe).
 *
 * To be added as subframe :
 * - sta must allow amsdu
 * - buffer must be aggregable (to be defined)
 * - at least one other aggregable buffer is pending in the queue
 *  or an a-msdu (with enough free space) is currently in progress
 *
 * returns true if buffer has been added as A-MDSP subframe, false otherwise
 *
 */
static bool cls_wifi_amsdu_add_subframe(struct cls_wifi_hw *cls_wifi_hw, struct sk_buff *skb,
				struct cls_wifi_sta *sta, struct cls_wifi_txq *txq,
				struct ethhdr *eth_t)
{
	bool res = false;
	struct ethhdr *eth;
	struct cls_wifi_sw_txhdr *sw_txhdr;

	/* Adjust the maximum number of MSDU allowed in A-MSDU */
	cls_wifi_adjust_amsdu_maxnb(cls_wifi_hw);

	/* immediately return if amsdu are not allowed for this sta */
	if (!txq->amsdu_len || cls_wifi_hw->radio_params->amsdu_maxnb < 2 ||
		!cls_wifi_amsdu_is_aggregable(skb))
		return false;

	spin_lock_bh(&cls_wifi_hw->tx_lock);
	if (txq->idx == TXQ_INACTIVE) {
		if (txq->wait2txq)
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
		spin_unlock_bh(&cls_wifi_hw->tx_lock);
		res = true;
		dev_kfree_skb_any(skb);
		return res;
	}

	if (txq->amsdu != NULL) {
		/* aggregation already in progress, add this buffer if enough space
		   available, otherwise end the current amsdu */
		struct cls_wifi_sw_txhdr *sw_txhdr = txq->amsdu;
		__be16 h_proto;

		if (eth_t)
			eth = eth_t;
		else
			eth = (struct ethhdr *)(skb->data);

		h_proto = eth->h_proto;

		if (cls_wifi_hw->has_hw_llcsnap_insert &&
			unlikely(eth_proto_is_802_3(h_proto) != sw_txhdr->amsdu.is_8023)) {
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			cls_wifi_hw->stats.amsdus[sw_txhdr->amsdu.nb - 1].done++;
			txq->amsdu = NULL;
			goto end;
		}

		if (((sw_txhdr->frame_len + sw_txhdr->amsdu.pad +
			  cls_wifi_amsdu_subframe_length(h_proto, skb->len)) > txq->amsdu_len) ||
			cls_wifi_amsdu_add_subframe_header(cls_wifi_hw, skb, sw_txhdr, eth_t)) {
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			cls_wifi_hw->stats.amsdus[sw_txhdr->amsdu.nb - 1].done++;
			txq->amsdu = NULL;
			goto end;
		}

		if (sw_txhdr->amsdu.nb >= cls_wifi_hw->radio_params->amsdu_maxnb) {
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			cls_wifi_hw->stats.amsdus[sw_txhdr->amsdu.nb - 1].done++;
			/* max number of subframes reached */
			txq->amsdu = NULL;
		}
	} else {
		/* Check if a new amsdu can be started with the previous buffer
		   (if any) and this one */
		struct sk_buff *skb_prev = NULL;
		struct cls_wifi_txhdr *txhdr;
		int len1, len2;
		bool is_8023;
		__be16 h_proto;

		if (txq->wait2txq != NULL) {
			if ((txq->wait2txq->skb) && (!txq->wait2txq->queued))
				skb_prev = txq->wait2txq->skb;
		}

		if (skb_prev == NULL)
			skb_prev = skb_peek_tail(&txq->sk_list);

		if (!skb_prev || !cls_wifi_amsdu_is_aggregable(skb_prev)) {
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			goto end;
		}

		txhdr = (struct cls_wifi_txhdr *)skb_prev->data;
		sw_txhdr = txhdr->sw_hdr;
		if ((sw_txhdr->amsdu.nb) ||
			(sw_txhdr->desc.api.host.flags & TXU_CNTRL_REUSE_SN)) {
			/* previous buffer is already a complete amsdu or a retry */
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			goto end;
		}

		eth = (struct ethhdr *)skb_mac_header(skb_prev);
		h_proto = eth->h_proto;
		len1 = cls_wifi_amsdu_subframe_length(h_proto, sw_txhdr->frame_len);
		is_8023 = eth_proto_is_802_3(h_proto);

		if (eth_t)
			eth = eth_t;
		else
			eth = (struct ethhdr *)(skb->data);

		h_proto = eth->h_proto;
		len2 = cls_wifi_amsdu_subframe_length(h_proto, skb->len);

		if (cls_wifi_hw->has_hw_llcsnap_insert &&
			unlikely(eth_proto_is_802_3(h_proto) != is_8023)) {
			/* As FW has the info on the frame type only for the first one, let's force
			 * A-MSDU subframes to all have the same type. Indeed FW has to know that
			 * to correctly configure the DMA for the LLC/SNAP insertion.
			 */
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			goto end;
		}

		if (len1 + AMSDU_PADDING(len1) + len2 > txq->amsdu_len) {
			/* not enough space to aggregate those two buffers */
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			goto end;
		}

		/* Add subframe header.
		   Note: Fw will take care of adding AMDSU header for the first
		   subframe while generating 802.11 MAC header */
		INIT_LIST_HEAD(&sw_txhdr->amsdu.hdrs);
		cls_wifi_tx_update_frame_len(cls_wifi_hw, sw_txhdr, len1);
		sw_txhdr->amsdu.nb = 1;
		sw_txhdr->amsdu.pad = AMSDU_PADDING(len1);
		sw_txhdr->amsdu.is_8023 = is_8023;
		if (cls_wifi_amsdu_add_subframe_header(cls_wifi_hw, skb, sw_txhdr, eth_t)) {
			sw_txhdr->amsdu.nb = 0;
			cls_wifi_tx_update_frame_len(cls_wifi_hw, sw_txhdr,
									 sw_txhdr->desc.api.host.packet_len[0]);
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			goto end;
		}
		sw_txhdr->desc.api.host.flags |= TXU_CNTRL_AMSDU;

		if (sw_txhdr->amsdu.nb < cls_wifi_hw->radio_params->amsdu_maxnb)
			txq->amsdu = sw_txhdr;
		else {
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
			cls_wifi_hw->stats.amsdus[sw_txhdr->amsdu.nb - 1].done++;
		}
	}

	res = true;

  end:
	spin_unlock_bh(&cls_wifi_hw->tx_lock);
	return res;
}

/**
 * cls_wifi_amsdu_dismantle - Dismantle an already formatted A-MSDU
 *
 * @cls_wifi_hw Driver main data
 * @sw_txhdr_main Software descriptor of the A-MSDU to dismantle.
 *
 * The a-mdsu is always fully dismantled (i.e don't try to reduce it's size to
 * fit the new limit).
 * The DMA mapping can be re-used as cls_wifi_amsdu_add_subframe_header ensure that
 * enough data in the skb bufer are 'DMA mapped'.
 * It would have been slightly simple to unmap/re-map but it is a little faster like this
 * and not that much more complicated to read.
 */
static void cls_wifi_amsdu_dismantle(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sw_txhdr *sw_txhdr_main)
{
	struct cls_wifi_amsdu_txhdr *amsdu_txhdr, *next;
	struct sk_buff *skb_prev = sw_txhdr_main->skb;

	trace_amsdu_dismantle(sw_txhdr_main);

	cls_wifi_hw->stats.amsdus[sw_txhdr_main->amsdu.nb - 1].done--;
	sw_txhdr_main->amsdu.nb = 0;
	sw_txhdr_main->desc.api.host.flags &= ~TXU_CNTRL_AMSDU;
	sw_txhdr_main->desc.api.host.packet_cnt = 1;
	cls_wifi_tx_update_frame_len(cls_wifi_hw, sw_txhdr_main,
							 sw_txhdr_main->desc.api.host.packet_len[0]);

	list_for_each_entry_safe(amsdu_txhdr, next, &sw_txhdr_main->amsdu.hdrs, list) {
		struct sk_buff *skb = amsdu_txhdr->skb;
		struct cls_wifi_txhdr *txhdr;
		struct cls_wifi_sw_txhdr *sw_txhdr;
		size_t data_oft;
		size_t frame_len;

		list_del(&amsdu_txhdr->list);
		cls_wifi_amsdu_del_subframe_header(cls_wifi_hw, amsdu_txhdr);

		frame_len = CLS_WIFI_TX_DMA_MAP_LEN(skb);

		sw_txhdr = kmem_cache_alloc(cls_wifi_hw->sw_txhdr_cache, GFP_ATOMIC);
		if (unlikely((skb_headroom(skb) < CLS_WIFI_TX_HEADROOM) ||
					 (sw_txhdr == NULL) || (frame_len > amsdu_txhdr->ipc_data.size))) {
			dev_err(cls_wifi_hw->dev, "Failed to dismantle A-MSDU\n");
			if (sw_txhdr)
				kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);
			cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &amsdu_txhdr->ipc_data);
			dev_kfree_skb_any(skb);
			continue;
		}

		// Offset between DMA mapping for an A-MSDU subframe and a simple MPDU
		data_oft = amsdu_txhdr->ipc_data.size - frame_len;

		memcpy(sw_txhdr, sw_txhdr_main, sizeof(*sw_txhdr));
		sw_txhdr->frame_len = frame_len;
		sw_txhdr->skb = skb;
		sw_txhdr->ipc_data = amsdu_txhdr->ipc_data; // It's OK to re-use amsdu_txhdr ptr
		sw_txhdr->desc.api.host.packet_addr[0] = sw_txhdr->ipc_data.dma_addr + data_oft;
		sw_txhdr->desc.api.host.packet_len[0] = frame_len;
		sw_txhdr->desc.api.host.packet_cnt = 1;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
		sw_txhdr->queued = false;
#endif

		txhdr = skb_push(skb, CLS_WIFI_TX_HEADROOM);
		txhdr->sw_hdr = sw_txhdr;

		cls_wifi_txq_queue_skb(skb, sw_txhdr->txq, cls_wifi_hw, false, skb_prev);
		skb_prev = skb;
	}
}


/**
 * cls_wifi_amsdu_update_len - Update length allowed for A-MSDU on a TXQ
 *
 * @cls_wifi_hw Driver main data.
 * @txq The TXQ.
 * @amsdu_len New length allowed ofr A-MSDU.
 *
 * If this is a TXQ linked to a STA and the allowed A-MSDU size is reduced it is
 * then necessary to disassemble all A-MSDU currently queued on all STA' txq that
 * are larger than this new limit.
 * Does nothing if the A-MSDU limit increase or stay the same.
 */
static void cls_wifi_amsdu_update_len(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq,
								  u16 amsdu_len)
{
	struct cls_wifi_sta *sta = txq->sta;
	int tid;

	if (amsdu_len != txq->amsdu_len)
		trace_amsdu_len_update(txq->sta, amsdu_len);

	if (amsdu_len >= txq->amsdu_len) {
		txq->amsdu_len = amsdu_len;
		return;
	}

	if (!sta) {
		netdev_err(txq->ndev, "Non STA txq(%d) with a-amsdu len %d\n",
				   txq->idx, amsdu_len);
		txq->amsdu_len = 0;
		return;
	}

	/* A-MSDU size has been reduced by the firmware, need to dismantle all
	   queued a-msdu that are too large. Need to do this for all txq of the STA. */
	foreach_sta_txq(sta, txq, tid, cls_wifi_hw) {
		struct sk_buff *skb, *skb_next;

		if (txq->amsdu_len <= amsdu_len)
			continue;

		cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);

		if (txq->last_retry_skb)
			skb = txq->last_retry_skb->next;
		else
			skb = txq->sk_list.next;

		skb_queue_walk_from_safe(&txq->sk_list, skb, skb_next) {
			struct cls_wifi_txhdr *txhdr = (struct cls_wifi_txhdr *)skb->data;
			struct cls_wifi_sw_txhdr *sw_txhdr = txhdr->sw_hdr;
			if ((sw_txhdr->desc.api.host.flags & TXU_CNTRL_AMSDU) &&
				(sw_txhdr->frame_len > amsdu_len))
				cls_wifi_amsdu_dismantle(cls_wifi_hw, sw_txhdr);

			if (txq->amsdu == sw_txhdr)
				txq->amsdu = NULL;
		}

		txq->amsdu_len = amsdu_len;
	}
}

#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */

static uint8_t cls_wifi_vip_node_tx_drop_check(uint16_t sta_idx, struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_sta *sta, *sta_vip;
	uint32_t sta_txpkts, vip_txpkts;
	uint8_t ret = 0;

	if (!cls_wifi_hw->vip_node.enable ||
			cls_wifi_hw->vip_node.node_idx == CLS_WIFI_INVALID_STA)
		return 0;

	if (sta_idx == cls_wifi_hw->vip_node.node_idx)
		return 0;

	sta = &cls_wifi_hw->sta_table[sta_idx];
	sta_vip = &cls_wifi_hw->sta_table[cls_wifi_hw->vip_node.node_idx];
	vip_txpkts = sta_vip->stats.tx_pkts - sta_vip->stats.tx_pkts_old;
	sta_txpkts = sta->stats.tx_pkts - sta->stats.tx_pkts_old;

	if (sta_vip->stats.tx_pps <= cls_wifi_hw->vip_node.pps_thresh ||
			sta_txpkts * cls_wifi_hw->vip_node.traffic_ratio < vip_txpkts)
		ret = 0;
	else
		ret = 1;

	return ret;
}

#ifdef CONFIG_CLS_FWT
#define CAP_NON_HT  0
#define CAP_HT      1
#define CAP_VHT     2
#define CAP_HE      3
#define CAP_MAX     4
#define CAP_AMSDU   1
#define CAP_SMSDU   0
#define CAP_MSDU_MAX 2
#define CUR_STA_NUM  2
#define CUR_AC_MAX   (CLS_WIFI_HWQ_BCMC + 1 + 1)
#define FLAG_OFFSET_IDX (CUR_AC_MAX - 1)
#define FLAG_2G_BUF_LIMIT_EX 0x1U  ///buf_limit_ex
uint16_t cache_limit_hwq[CAP_MAX][CAP_MSDU_MAX][CUR_STA_NUM][CUR_AC_MAX] = {
    [CAP_NON_HT] = {
        [CAP_SMSDU] = {
                ///BK BE VI VO BCMC flag
            [0] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX},
                ///STA num >= 2
            [1] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX}
            },
        [CAP_AMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX}
            },
    },
    [CAP_HT] = {
        [CAP_SMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX}
            },
        [CAP_AMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {512, 512, 512, 512, 200, FLAG_2G_BUF_LIMIT_EX}
            },
    },
    [CAP_VHT] = {
        [CAP_SMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {320, 320, 320, 320, 320, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {320, 320, 320, 320, 320, 0}
            },
        [CAP_AMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {320, 320, 320, 320, 200, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {224, 200, 200, 200, 224, 0}
            },
    },
    [CAP_HE] = {
        [CAP_SMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {2048, 2048, 2048, 2048, 200, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {2048, 2048, 2048, 2048, 2048, FLAG_2G_BUF_LIMIT_EX}
            },
        [CAP_AMSDU] = {
                ///BK BE VI VO BCMC
            [0] = {2048, 2048, 2048, 2048, 200, FLAG_2G_BUF_LIMIT_EX},
            ///STA num >= 2
            [1] = {2048, 2048, 2048, 2048, 2048, FLAG_2G_BUF_LIMIT_EX}
            },
    },
};

static bool cls_wifi_skb_deep_limit_num(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
	struct cls_wifi_sta *sta, struct cls_wifi_txq *txq)
{
	uint32_t skb_len  = skb_queue_len(&txq->sk_list);
	uint32_t multi_sta = (cls_wifi_vif->assoc_sta_count >= 2) ? 1 : 0;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	uint32_t is_amsdu = (txq->amsdu_len) ? 1 : 0;
#else
	uint32_t is_amsdu = 0;
#endif
	uint32_t cap_mode = CAP_NON_HT;
	uint32_t cache_max_len;
	int ac = txq->hwq->id;
	int flag;

	if (cls_wifi_mod_params.txq_max_len)
		return (skb_len > cls_wifi_mod_params.txq_max_len);

	ac = (ac > CLS_WIFI_HWQ_BCMC) ? CLS_WIFI_HWQ_BE : ac;
	if (sta->he)
		cap_mode = CAP_HE;
	else if (sta->vht)
		cap_mode = CAP_VHT;
	else if (sta->ht)
		cap_mode = CAP_HT;

	cache_max_len = cache_limit_hwq[cap_mode][is_amsdu][multi_sta][ac];
	flag = cache_limit_hwq[cap_mode][is_amsdu][multi_sta][FLAG_OFFSET_IDX];
	if ((cls_wifi_hw->radio_idx == 0) && (flag & FLAG_2G_BUF_LIMIT_EX))
		cache_max_len = cache_max_len >> 1;
	if (skb_len > cache_max_len)
		return true;

	return false;
}
#endif

#ifdef CONFIG_CLS_3ADDR_BR
skb_tx_handler_t cls_3addr_br_tx_callback = NULL;
EXPORT_SYMBOL(cls_3addr_br_tx_callback);
#endif

static netdev_tx_t __cls_wifi_start_xmit_single(struct sk_buff *skb,
		struct net_device *dev, struct cls_wifi_sta *sta, u8 tid, struct cls_wifi_txq *txq)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct cls_wifi_txhdr *txhdr;
	struct cls_wifi_sw_txhdr *sw_txhdr = NULL;
#if ETH_OPTI_DDR_CFG
	struct ethhdr eth_t;
#endif
	struct vlan_ethhdr *p_vlan_eth = NULL;
	bool skip_logic_port = false;
	struct ethhdr *p_eth = NULL;
	struct ethhdr *eth;
	struct txdesc_api *desc;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	bool drop_skb = false;
#endif

	sk_pacing_shift_update(skb->sk, cls_wifi_hw->tcp_pacing_shift);

	// If buffer is shared (or may be used by another interface) need to make a
	// copy as TX infomration is stored inside buffer's headroom
	if (skb_shared(skb) || (skb_headroom(skb) < CLS_WIFI_TX_MAX_HEADROOM) ||
		(skb_cloned(skb) && (dev->priv_flags & IFF_BRIDGE_PORT))) {
		struct sk_buff *newskb = skb_copy_expand(skb, CLS_WIFI_TX_MAX_HEADROOM, 0, GFP_ATOMIC);
		if (unlikely(newskb == NULL))
			goto free;

		dev_kfree_skb_any(skb);
		skb = newskb;
	}

#ifdef CONFIG_CLS_3ADDR_BR
	/*STA mode && use_4addr == 0 And cls_3addr_br_tx_callback != NULL, hack pkt*/
	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_STATION
			&& !cls_wifi_vif->use_4addr
			&& cls_3addr_br_tx_callback) {
		cls_3addr_br_tx_callback(cls_wifi_vif, skb);
	}
#endif

#if (ETH_OPTI_DDR_CFG)
	eth_t = *(struct ethhdr *)skb->data;
	p_eth = &eth_t;
#endif

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (cls_wifi_hw->amsdu_enabled) {
		if (cls_wifi_amsdu_add_subframe(cls_wifi_hw, skb, sta, txq, p_eth))
			return NETDEV_TX_OK;
	}
#endif

	sw_txhdr = kmem_cache_alloc(cls_wifi_hw->sw_txhdr_cache, GFP_ATOMIC);
	if (unlikely(sw_txhdr == NULL))
		goto free;

	sw_txhdr->txq	   = txq;
	sw_txhdr->frame_len = CLS_WIFI_TX_DMA_MAP_LEN(skb);
	sw_txhdr->cls_wifi_sta  = sta;
	sw_txhdr->cls_wifi_vif  = cls_wifi_vif;
	sw_txhdr->skb	   = skb;
	sw_txhdr->jiffies   = jiffies;

	/* Prepare IPC buffer for DMA transfer */
	eth = (struct ethhdr *)skb->data;
	if (unlikely(cls_wifi_prep_dma_tx(cls_wifi_hw, sw_txhdr, eth)))
		goto free;

	/* Fill-in the API descriptor for the MACSW */
	desc = &sw_txhdr->desc.api;
#if (ETH_OPTI_DDR_CFG)
	memcpy(&desc->host.eth_dest_addr, eth_t.h_dest, ETH_ALEN);
	memcpy(&desc->host.eth_src_addr, eth_t.h_source, ETH_ALEN);
#if CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD == 1
	desc->host.ethertype[0] = eth_t.h_proto;
#else
	desc->host.ethertype = eth_t.h_proto;
#endif
	if (eth_t.h_proto == htons(ETH_P_8021Q)) {
		p_vlan_eth = (struct vlan_ethhdr *)skb->data;
		if (p_vlan_eth->h_vlan_encapsulated_proto == htons(ETH_P_PAE))
			skip_logic_port = true;
	}

#else
	memcpy(&desc->host.eth_dest_addr, eth->h_dest, ETH_ALEN);
	memcpy(&desc->host.eth_src_addr, eth->h_source, ETH_ALEN);
	desc->host.ethertype = eth->h_proto;

	if (eth->h_proto == htons(ETH_P_8021Q)) {
		p_vlan_eth = (struct vlan_ethhdr *)eth;
		if (p_vlan_eth->h_vlan_encapsulated_proto == htons(ETH_P_PAE))
			skip_logic_port = true;
	}
#endif
	desc->host.sta_idx = sta->sta_idx;
	desc->host.tid = (sta->qos) ? tid : 0xFF;
#ifdef CONFIG_CLS_WIFI_HEMU_TX
	desc->host.user_idx = 0;
	desc->host.rua_map_idx = CLS_WIFI_INVALID_RUA_MAP;
#endif
	if (unlikely(cls_wifi_vif->wdev.iftype == NL80211_IFTYPE_AP_VLAN))
		desc->host.vif_idx = cls_wifi_vif->ap_vlan.master->vif_index;
	else
		desc->host.vif_idx = cls_wifi_vif->vif_index;

	desc->host.flags = skip_logic_port ? TXU_CNTRL_SKIP_LOGIC_PORT : 0;

#if (ETH_OPTI_DDR_CFG)
	// for auto_4addr, use sta->is_4addr to idenfy 4addr sta
	if (((cls_wifi_vif->auto_4addr && sta->is_4addr &&
			(memcmp(eth_t.h_dest, sta->mac_addr, ETH_ALEN) || cls_wifi_vif->force_4addr)) ||
			cls_wifi_vif->use_4addr) && (sta->sta_idx < hw_remote_sta_max(cls_wifi_hw)))
		desc->host.flags |= TXU_CNTRL_USE_4ADDR;

	if (cls_wifi_vif->log_enable)
		pr_warn("Packet SA[%pM] DA[%pM] RA[%pM] 4addr %d)\n",
				eth_t.h_source, eth_t.h_dest, sta->mac_addr, desc->host.flags & TXU_CNTRL_USE_4ADDR);
#else
	// for auto_4addr, use sta->is_4addr to idenfy 4addr sta
	if (((cls_wifi_vif->auto_4addr && sta->is_4addr &&
			(memcmp(eth->h_dest, sta->mac_addr, ETH_ALEN) || cls_wifi_vif->force_4addr)) ||
			cls_wifi_vif->use_4addr) && (sta->sta_idx < hw_remote_sta_max(cls_wifi_hw)))
		desc->host.flags |= TXU_CNTRL_USE_4ADDR;

	if (cls_wifi_vif->log_enable)
		pr_warn("Packet SA[%pM] DA[%pM] RA[%pM] 4addr %d)\n",
				eth->h_source, eth->h_dest, sta->mac_addr, desc->host.flags & TXU_CNTRL_USE_4ADDR);
#endif

	if ((cls_wifi_vif->tdls_status == TDLS_LINK_ACTIVE) &&
		cls_wifi_vif->sta.tdls_sta &&
#if CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD == 1
		(memcmp(desc->host.eth_dest_addr[0].array, cls_wifi_vif->sta.tdls_sta->mac_addr, ETH_ALEN) == 0))
#else
		(memcmp(desc->host.eth_dest_addr.array, cls_wifi_vif->sta.tdls_sta->mac_addr, ETH_ALEN) == 0))
#endif
	{
		desc->host.flags |= TXU_CNTRL_TDLS;
		cls_wifi_vif->sta.tdls_sta->tdls.last_tid = desc->host.tid;
		cls_wifi_vif->sta.tdls_sta->tdls.last_sn = 0; //TODO: set this on confirm ?
	}

	if ((cls_wifi_vif->wdev.iftype == NL80211_IFTYPE_MESH_POINT) &&
		(cls_wifi_vif->is_resending))
		desc->host.flags |= TXU_CNTRL_MESH_FWD;

	/* store Tx info in skb headroom */
	txhdr = skb_push(skb, CLS_WIFI_TX_HEADROOM);
	txhdr->sw_hdr = sw_txhdr;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	sw_txhdr->amsdu.nb = 0;
	sw_txhdr->queued = false;
	spin_lock_bh(&cls_wifi_hw->tx_lock);
	if (txq->wait2txq)
		cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);

	if (txq->idx != TXQ_INACTIVE)
		txq->wait2txq = sw_txhdr;
	else
		drop_skb = true;
	spin_unlock_bh(&cls_wifi_hw->tx_lock);
#endif

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	/* FIXME: may no need amsdu agg timeout when amsdu_len = 0 */
	if ((!drop_skb) && (cls_wifi_hw->radio_params->amsdu_agg_timeout > 0)) {
		if (likely(!hrtimer_is_queued(&txq->amsdu_agg_timer))) {
			hrtimer_start(&txq->amsdu_agg_timer,
					ns_to_ktime(cls_wifi_hw->radio_params->amsdu_agg_timeout * NSEC_PER_USEC),
					HRTIMER_MODE_REL_SOFT);
		} else {
			pr_warn("amsdu agg timer is queued\n");
		}
	} else
#endif
	{
		/* queue the buffer */
		spin_lock_bh(&cls_wifi_hw->tx_lock);
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
		txq->wait2txq = NULL;
#endif
		/* Ensure that TXQ is active */
		if (cls_wifi_txq_queue_skb(skb, txq, cls_wifi_hw, false, NULL))
			cls_wifi_hwq_process(cls_wifi_hw, txq->hwq);
		spin_unlock_bh(&cls_wifi_hw->tx_lock);
	}

	return NETDEV_TX_OK;

free:
	if (sw_txhdr)
		kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);
	dev_kfree_skb_any(skb);

	return NETDEV_TX_OK;
}

static netdev_tx_t __cls_wifi_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct cls_wifi_sta *sta;
	struct cls_wifi_txq *txq;
	u8 tid;
#ifdef CONFIG_CLS_FWT
	struct sk_buff *skb_copy;
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	struct wireless_dev *wdev = &cls_wifi_vif->wdev;
	bool txrx_in_vif = false, txrx_in_radio = false;
	struct cls_wifi_sta *src_sta = NULL;

	if (cls_wifi_vif->log_enable) {
		pr_warn("%s %d src_port 0x%x\n", __func__, __LINE__, skb->src_port);
		pr_warn("ri %d cri %d vv %d nv %d vi %d cvi %d ni %d invi %d 4addr %d m2u3 %d auto4 %d mul %d\n",
				CLS_IEEE80211_RADIO_IDX_FROM_SUBPORT(skb->src_port),
				cls_wifi_hw->radio_idx,
				CLS_IEEE80211_VIF_IDX_VALID(skb->src_port),
				CLS_IEEE80211_NODE_IDX_VALID(skb->src_port),
				CLS_IEEE80211_VIF_IDX_FROM_SUBPORT(skb->src_port),
				cls_wifi_vif->vif_index,
				CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(skb->src_port),
				CLS_WIFI_INVALID_STA,
				CLS_IEEE80211_IS_4ADDR_SUPPORT(skb->src_port),
				cls_wifi_vif->m2u_3addr_resend,
				cls_wifi_vif->auto_4addr,
				is_multicast_ether_addr(eth->h_dest));
	}

	if ((CLS_IEEE80211_VIF_IDX_VALID(skb->src_port) &&
			CLS_IEEE80211_NODE_IDX_VALID(skb->src_port) &&
			CLS_IEEE80211_RADIO_IDX_FROM_SUBPORT(skb->src_port) == cls_wifi_hw->radio_idx &&
			CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(skb->src_port) != CLS_WIFI_INVALID_STA)) {
		txrx_in_radio = true;

		src_sta = cls_wifi_get_sta(cls_wifi_hw,
			(CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(skb->src_port) & 0xFFF), NULL, false);

		if (CLS_IEEE80211_VIF_IDX_FROM_SUBPORT(skb->src_port) == cls_wifi_vif->vif_index)
			txrx_in_vif = true;
	}
	//TODO: src_sta in defferent radio

	if (unlikely(txrx_in_vif &&
			((!CLS_IEEE80211_IS_4ADDR_SUPPORT(skb->src_port) && cls_wifi_vif->m2u_3addr_resend) ||
			(CLS_IEEE80211_IS_4ADDR_SUPPORT(skb->src_port) && cls_wifi_vif->auto_4addr)) &&
			is_multicast_ether_addr(eth->h_dest))) {
		spin_lock_bh(&cls_wifi_hw->rosource_lock);

		list_for_each_entry(sta, &cls_wifi_vif->ap.sta_list, list) {
			if (!cls_wifi_vif_is_active(NULL, cls_wifi_vif, 0))
				break;
			//todo: remove 0xFFF
			if ((sta->sta_idx == (CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(skb->src_port) & 0xFFF))
				|| (sta->valid == false))
				continue;

			/* For ap_vlan's stations, bypass defferent vlan_idx */
			if (cls_wifi_mod_params.dynamic_vlan) {
				if (src_sta
					&& ((cls_wifi_sta_in_ap_vlan(cls_wifi_hw, src_sta)
						|| cls_wifi_sta_in_ap_vlan(cls_wifi_hw, sta))
					&& src_sta->vlan_idx != sta->vlan_idx))
					continue;
			}

			if (cls_wifi_vif->log_enable)
				pr_warn("%s %d m2u to [%pM], 4addr %d\n", __func__, __LINE__,
						sta->mac_addr, sta->is_4addr);
			skb_copy = skb_copy_expand(skb, CLS_WIFI_TX_MAX_HEADROOM, 0, GFP_ATOMIC);
			if (unlikely(skb_copy == NULL)) {
				pr_warn("%s %d, sta idx: %u\n", __func__, __LINE__, sta->sta_idx);
				continue;
			}

			if (!sta->is_4addr) {
				eth = (struct ethhdr *)skb_copy->data;
				memcpy(eth->h_dest, sta->mac_addr, ETH_ALEN);
			}
			if (sta->qos) {
				/* use the data classifier to determine what 802.1d tag the
				 * data frame has */
				skb_copy->priority = cfg80211_classify8021d(skb_copy, NULL) &
						IEEE80211_QOS_CTL_TAG1D_MASK;
				if (sta->acm)
					cls_wifi_downgrade_ac(sta, skb_copy);
				txq = cls_wifi_txq_sta_get(sta, skb_copy->priority, cls_wifi_hw);
			} else {
				skb_copy->priority = 0xFF;
				txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
			}
			__cls_wifi_start_xmit_single(skb_copy, dev, sta, txq->tid, txq);
		}
		spin_unlock_bh(&cls_wifi_hw->rosource_lock);
		dev_kfree_skb_any(skb);
	}else {
		if (cls_wifi_vif->log_enable)
			pr_warn("%s %d\n", __func__, __LINE__);

		if (wdev && (wdev->iftype == NL80211_IFTYPE_AP)
				&& list_empty(&cls_wifi_vif->ap.sta_list)
				&& is_multicast_ether_addr(eth->h_dest)) {
			if (cls_wifi_vif->log_enable) {
				pr_warn("%s empty sta_list. Drop BCMC\n", cls_wifi_vif->ndev->name);
			}
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}
#endif
		/* Get the STA id and TID information */
		sta = cls_wifi_get_tx_info(cls_wifi_vif, skb, &tid);
		if (!sta) {
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}

#ifdef CONFIG_CLS_FWT
		/* For ap_vlan's stations, bypass defferent vlan_idx */
		if ((txrx_in_vif && cls_wifi_mod_params.dynamic_vlan == CLS_WIFI_DYN_VLAN_PER_VIF)
			|| (txrx_in_radio && cls_wifi_mod_params.dynamic_vlan == CLS_WIFI_DYN_VLAN_PER_RADIO)) {
			if (src_sta
				&& ((cls_wifi_sta_in_ap_vlan(cls_wifi_hw, src_sta)
					|| cls_wifi_sta_in_ap_vlan(cls_wifi_hw, sta))
				&& src_sta->vlan_idx != sta->vlan_idx)) {
				dev_kfree_skb_any(skb);
				return NETDEV_TX_OK;
			}
		}

		if ((!sta->qos) && (skb->bmu_flag & CLS_BMU_FLAG_NON_TXQ)) {
			if (sta->sta_idx < cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx])
				tid = 0;  ///follow cls_wifi_select_txq
		}
#endif

		txq = cls_wifi_txq_sta_get(sta, tid, cls_wifi_hw);
		if ((txq->idx == TXQ_INACTIVE) || (sta->valid == false)) {
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}

		/* When txq is FLOW_CTRL, we should block this type of skb: stored in host and high TP.
		 * (1) from wifi and stored in host
		 * (2) form edma and will stored in host(skb_copy __cls_wifi_start_xmit_single)
		 */
		if (txq->status & CLS_WIFI_TXQ_NDEV_FLOW_CTRL) {
			bool do_free = true;

#ifdef CONFIG_CLS_FWT
			// case1: from wifi and stored in host
			if (CLS_IEEE80211_VIF_IDX_VALID(skb->src_port)
				// case2: if queue_len exceeds edma_size(8K). it means bmu skb have been copied.
				|| ((cls_wifi_skb_deep_limit_num(cls_wifi_hw, cls_wifi_vif, sta, txq)) && (skb->bmu_flag & CLS_BMU_FLAG_ETH)))
				do_free = true;
			 else
				do_free = false;
#endif

			if (do_free) {
				txq->pkt_fc_drop++;
				dev_kfree_skb_any(skb);
				return NETDEV_TX_OK;
			}
		}

		if (cls_wifi_vip_node_tx_drop_check(sta->sta_idx, cls_wifi_hw)) {
			txq->pkt_vip_drop++;
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}

		return __cls_wifi_start_xmit_single(skb, dev, sta, txq->tid, txq);
#ifdef CONFIG_CLS_FWT
	}

	return NETDEV_TX_OK;
#endif
}

/**
 * netdev_tx_t (*ndo_start_xmit)(struct sk_buff *skb,
 *                               struct net_device *dev);
 *	Called when a packet needs to be transmitted.
 *	Must return NETDEV_TX_OK , NETDEV_TX_BUSY.
 *        (can also return NETDEV_TX_LOCKED if NETIF_F_LLTX)
 *
 *  - Initialize the desciptor for this pkt (stored in skb before data)
 *  - Push the pkt in the corresponding Txq
 *  - If possible (i.e. credit available and not in PS) the pkt is pushed
 *    to fw
 */
netdev_tx_t cls_wifi_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct cls_wifi_vif *cls_wifi_vif = netdev_priv(dev);
    struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
    netdev_tx_t ret = NETDEV_TX_OK;
    bool send_failed = false;
    struct ethhdr *eth_header = (struct ethhdr *)skb->data;
    u16 netdev_queue;

#ifdef CONFIG_CLS_FWT
    /* If this skb has been forward by fwt, txq is ready and skb is ready to be xmited */
    if (!(skb->bmu_flag & CLS_BMU_FLAG_NON_TXQ)) {
#endif
        netdev_queue = cls_wifi_select_txq(cls_wifi_vif, skb);
        skb_set_queue_mapping(skb, netdev_queue);
        if (cls_wifi_vif->log_enable)
               pr_warn("Call select_txq: vif=%s queue_mapping=%d priority=%d: %pM -> %pM\n",
                       dev->name, skb->queue_mapping, skb->priority, eth_header->h_source, eth_header->h_dest);
#ifdef CONFIG_CLS_FWT
    } else {
	if (cls_wifi_vif->log_enable)
               pr_warn("No select_txq: vif=%s queue_mapping=%d priority=%d: %pM -> %pM\n",
                       dev->name, skb->queue_mapping, skb->priority, eth_header->h_source, eth_header->h_dest);
    }
#endif

    if (cls_wifi_vif->dump_pkt) {
        pr_warn("send data frame on %s skb %px, len %d\n", dev->name, skb->data, skb->len);
        print_hex_dump(KERN_WARNING, "TX-SKB: ", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len, false);
    }

    if (!cls_wifi_vif_is_active(NULL, cls_wifi_vif, 0)) {
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;
    }

    if (tx_wq > 0 && tx_wq < 3) {
	if (skb_queue_len(&cls_wifi_vif->tx_skb_queue) > txq_ctrl_stop) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}
        spin_lock_bh(&cls_wifi_hw->txwq_lock);
        if(cls_wifi_vif->txwq_vif_allow) {
            skb_queue_tail(&cls_wifi_vif->tx_skb_queue, skb);
#ifdef WIFI_DRV_TXWQ_PERF
		if (!cls_wifi_vif->tx_wq_start) {
#endif
			queue_work_on((tx_wq - 1), cls_wifi_hw->txwq_workqueue, &cls_wifi_vif->tx_work);
#ifdef WIFI_DRV_TXWQ_PERF
			cls_wifi_vif->tx_wq_start = true;
		} else
			complete(&cls_wifi_vif->complete);
#endif
        }else{
            send_failed = true;
        }
        spin_unlock_bh(&cls_wifi_hw->txwq_lock);

        if(send_failed == true) {
            dev_kfree_skb_any(skb);
            return NETDEV_TX_OK;
        }
    } else {
        ret = __cls_wifi_start_xmit(skb, dev);
    }

    return ret;
}

void cls_wifi_tx_work_handler(struct work_struct *work)
{
    struct cls_wifi_vif *cls_wifi_vif = container_of(work, struct cls_wifi_vif, tx_work);
    struct sk_buff *skb;
#ifdef WIFI_DRV_TXWQ_PERF
	while (1) {
#endif
		while ((skb = skb_dequeue(&cls_wifi_vif->tx_skb_queue)) != NULL) {
			if (!cls_wifi_vif->txwq_vif_allow) {
				consume_skb(skb);
				break;
			}
			__cls_wifi_start_xmit(skb, cls_wifi_vif->ndev);
		}
#ifdef WIFI_DRV_TXWQ_PERF
		wait_for_completion(&cls_wifi_vif->complete);
	}
#endif
}

/**
 * cls_wifi_start_mgmt_xmit - Transmit a management frame
 *
 * @vif: Vif that send the frame
 * @sta: Destination of the frame. May be NULL if the destiantion is unknown
 *	   to the AP.
 * @params: Mgmt frame parameters
 * @offchan: Indicate whether the frame must be send via the offchan TXQ.
 *		   (is is redundant with params->offchan ?)
 * @cookie: updated with a unique value to identify the frame with upper layer
 *
 */
int cls_wifi_start_mgmt_xmit(struct cls_wifi_vif *vif, struct cls_wifi_sta *sta,
						 struct cfg80211_mgmt_tx_params *params, bool offchan,
						 u64 *cookie)
{
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	struct cls_wifi_txhdr *txhdr;
	struct cls_wifi_sw_txhdr *sw_txhdr;
	struct txdesc_api *desc;
	struct sk_buff *skb;
	size_t frame_len;
	u8 *data;
	struct cls_wifi_txq *txq;
	bool robust;

	frame_len = params->len;

	/* Set TID and Queues indexes */
	if (sta) {
		txq = cls_wifi_txq_sta_get(sta, 8, cls_wifi_hw);
	} else {
		if (offchan)
			txq = cls_wifi_hw->txq + off_chan_txq_idx(cls_wifi_hw);
		else
			txq = cls_wifi_txq_vif_get(vif, CLS_UNK_TXQ_TYPE);
	}

	/* Ensure that TXQ is active */
	if (txq->idx == TXQ_INACTIVE) {
		netdev_dbg(vif->ndev, "TXQ inactive\n");
		return -EBUSY;
	}

	if (!cls_wifi_vif_is_active(NULL, vif, 0)) {
		pr_warn("%s cls_wifi_vif(%px) invalid,going_stop: %d\n", __func__, vif, vif->going_stop);
		return -EBUSY;
	}

	/* Create a SK Buff object that will contain the provided data */
	skb = dev_alloc_skb(CLS_WIFI_TX_HEADROOM + frame_len + NET_SKB_PAD * 3);
	if (!skb)
		return -ENOMEM;


	*cookie = (unsigned long)skb;

	sw_txhdr = kmem_cache_alloc(cls_wifi_hw->sw_txhdr_cache, GFP_ATOMIC);
	if (unlikely(sw_txhdr == NULL)) {
		dev_kfree_skb(skb);
		return -ENOMEM;
	}

	/* Reserve headroom in skb. Do this so that we can easily re-use ieee80211
	   functions that take skb with 802.11 frame as parameter */
	skb_reserve(skb, NET_SKB_PAD);
	skb_reset_mac_header(skb);

	/* Copy data in skb buffer */
	data = skb_put(skb, frame_len);
	memcpy(data, params->buf, frame_len);
	robust = ieee80211_is_robust_mgmt_frame(skb);

	/* Update CSA counter if present */
	if (unlikely(params->n_csa_offsets) &&
		vif->wdev.iftype == NL80211_IFTYPE_AP &&
		vif->ap.csa) {
		int i;
		for (i = 0; i < params->n_csa_offsets ; i++) {
			data[params->csa_offsets[i]] = vif->ap.csa->count;
		}
	}

	sw_txhdr->txq = txq;
	sw_txhdr->frame_len = frame_len;
	sw_txhdr->cls_wifi_sta = sta;
	sw_txhdr->cls_wifi_vif = vif;
	sw_txhdr->skb = skb;
	sw_txhdr->jiffies = jiffies;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	sw_txhdr->amsdu.nb = 0;
	sw_txhdr->queued = false;
#endif

	/* Prepare IPC buffer for DMA transfer */
	if (unlikely(cls_wifi_prep_dma_tx(cls_wifi_hw, sw_txhdr, data))) {
		kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);
		dev_kfree_skb(skb);
		return -EBUSY;
	}

	/* Fill-in the API Descriptor for the MACSW */
	desc = &sw_txhdr->desc.api;
	desc->host.sta_idx = (sta) ? sta->sta_idx : CLS_WIFI_INVALID_STA;
	desc->host.vif_idx = vif->vif_index;
	desc->host.tid = 0xFF;
	desc->host.flags = TXU_CNTRL_MGMT;
#ifdef CONFIG_CLS_WIFI_HEMU_TX
	desc->host.user_idx = 0;
	desc->host.rua_map_idx = CLS_WIFI_INVALID_RUA_MAP;
#endif

	if (robust)
		desc->host.flags |= TXU_CNTRL_MGMT_ROBUST;

	if (params->no_cck)
		desc->host.flags |= TXU_CNTRL_MGMT_NO_CCK;

	/* store Tx info in skb headroom */
	txhdr = skb_push(skb, CLS_WIFI_TX_HEADROOM);
	txhdr->sw_hdr = sw_txhdr;

	/* queue the buffer */
	spin_lock_bh(&cls_wifi_hw->tx_lock);
	if (cls_wifi_txq_queue_skb(skb, txq, cls_wifi_hw, false, NULL))
		cls_wifi_hwq_process(cls_wifi_hw, txq->hwq);
	spin_unlock_bh(&cls_wifi_hw->tx_lock);

	return 0;
}

void cls_wifi_tx_dfx_stats(struct cls_wifi_vif *vif, struct cls_wifi_sta *sta, bool bc, bool mc)
{
	if (bc) {
		vif->dfx_stats.tx_broadcast++;
		if (sta && sta->valid)
			sta->dfx_stats.tx_broadcast++;
	} else if (mc) {
		vif->dfx_stats.tx_multicast++;
		if (sta && sta->valid)
			sta->dfx_stats.tx_multicast++;
	} else {
		vif->dfx_stats.tx_unicast++;
		if (sta && sta->valid)
			sta->dfx_stats.tx_unicast++;
	}
}

/**
 * cls_wifi_txdatacfm - FW callback for TX confirmation
 *
 * @pthis: Pointer to the object attached to the IPC structure
 *		 (points to struct cls_wifi_hw is this case)
 * @arg: IPC buffer with the TX confirmation
 *
 * This function is called for each confimration of transmission by the fw.
 * Called with tx_lock hold
 *
 */
int cls_wifi_txdatacfm(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
#if WIFI_DRV_OPTI_SPIN_LOCK_CFG
	void **argt = arg;
	struct freeskb_tab {
		unsigned int skb_array_size;
		void* temp_free_skb;
	};
	struct freeskb_tab *p_freeskb = argt[1];
	unsigned int skb_array_size_max = p_freeskb ? p_freeskb->skb_array_size : 0;
	void **free_skb = p_freeskb ? p_freeskb->temp_free_skb : NULL;
	unsigned int skb_array_offset = 0;
	struct cls_wifi_ipc_buf *ipc_cfm = argt[0];
#else
	struct cls_wifi_ipc_buf *ipc_cfm = arg;
#endif
	struct tx_cfm_tag *cfm = ipc_cfm->addr;
	struct sk_buff *skb;
	struct cls_wifi_sw_txhdr *sw_txhdr;
	struct cls_wifi_hwq *hwq;
	struct cls_wifi_txq *txq;
	struct ethhdr *eth;
	bool is_bc;
	bool is_mc;

	skb = cls_wifi_ipc_get_skb_from_cfm(cls_wifi_hw, ipc_cfm);
	if (!skb)
		return -1;

	sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_hdr;
	txq = sw_txhdr->txq;
	/* don't use txq->hwq as it may have changed between push and confirm */
	hwq = &cls_wifi_hw->hwq[sw_txhdr->hw_queue];

	spin_lock(&cls_wifi_hw->tx_lock);
	cls_wifi_txq_confirm_any(cls_wifi_hw, txq, hwq, sw_txhdr);

	/* Update txq and HW queue credits */
	if (sw_txhdr->desc.api.host.flags & TXU_CNTRL_MGMT) {
		trace_mgmt_cfm(sw_txhdr->cls_wifi_vif->vif_index,
					   (sw_txhdr->cls_wifi_sta) ? sw_txhdr->cls_wifi_sta->sta_idx : CLS_WIFI_INVALID_STA,
					   TXU_STATUS_IS_SET(ACKNOWLEDGED, cfm->status));

		if (cls_wifi_vif_is_active(cls_wifi_hw, NULL, sw_txhdr->desc.api.host.vif_idx)) {
			/* Confirm transmission to CFG80211 */
			cfg80211_mgmt_tx_status(&sw_txhdr->cls_wifi_vif->wdev,
								(unsigned long)skb, skb_mac_header(skb),
								sw_txhdr->frame_len,
								TXU_STATUS_IS_SET(ACKNOWLEDGED, cfm->status),
								GFP_ATOMIC);
		} else {
			pr_warn("[warn]%s cls_wifi_vif(%px) invalid, going_stop: %d, vif_idx: %d\n", __func__,
				sw_txhdr->cls_wifi_vif, sw_txhdr->cls_wifi_vif->going_stop, sw_txhdr->desc.api.host.vif_idx);
		}
	} else if ((txq->idx != TXQ_INACTIVE) && TXU_STATUS_IS_SET(SW_RETRY_REQUIRED, cfm->status)) {
		/* firmware postponed this buffer */
		cls_wifi_tx_retry(cls_wifi_hw, skb, sw_txhdr, cfm->status);
#if (WIFI_DRV_OPTI_SPIN_LOCK_CFG)
		if (p_freeskb)
			p_freeskb->skb_array_size = 0;
#endif

		spin_unlock(&cls_wifi_hw->tx_lock);
		return 0;
	}

	trace_skb_confirm(skb, txq, hwq, cfm);

	/* STA may have disconnect (and txq stopped) when buffers were stored
	   in fw. In this case do nothing when they're returned */
	if (txq->idx != TXQ_INACTIVE) {
		if (cfm->credits) {
			txq->credits += cfm->credits;
			if (txq->credits <= 0)
				cls_wifi_txq_stop(txq, CLS_WIFI_TXQ_STOP_FULL);
			else if (txq->credits > 0)
				cls_wifi_txq_start(txq, CLS_WIFI_TXQ_STOP_FULL);
		}

		/* continue service period */
		if (unlikely(txq->push_limit && !cls_wifi_txq_is_full(txq))) {
			if (!skb_queue_empty(&txq->sk_list))
				cls_wifi_txq_add_to_hw_list(txq);
		}
	}

	if (cfm->ampdu_size && (cfm->ampdu_size < IEEE80211_MAX_AMPDU_BUF))
		cls_wifi_hw->stats.ampdus_tx[cfm->ampdu_size - 1]++;

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (!TXU_STATUS_IS_SET(ACKNOWLEDGED, cfm->status)) {
		if (sw_txhdr->desc.api.host.flags & TXU_CNTRL_AMSDU)
			cls_wifi_hw->stats.amsdus[sw_txhdr->amsdu.nb - 1].failed++;
		else if (!sw_txhdr->cls_wifi_sta || !is_multicast_sta(cls_wifi_hw, sw_txhdr->cls_wifi_sta->sta_idx))
			cls_wifi_hw->stats.amsdus[0].failed++;
	}

	if (txq->idx != TXQ_INACTIVE)
		cls_wifi_amsdu_update_len(cls_wifi_hw, txq, cfm->amsdu_size);
#endif
	spin_unlock(&cls_wifi_hw->tx_lock);

	/* Release SKBs */
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (sw_txhdr->desc.api.host.flags & TXU_CNTRL_AMSDU) {
		struct cls_wifi_amsdu_txhdr *amsdu_txhdr, *tmp;
		list_for_each_entry_safe(amsdu_txhdr, tmp, &sw_txhdr->amsdu.hdrs, list) {
			cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &amsdu_txhdr->ipc_data);
			if (!(sw_txhdr->desc.api.host.flags & TXU_CNTRL_MGMT) &&
					TXU_STATUS_IS_SET(ACKNOWLEDGED, cfm->status)) {
				sw_txhdr->cls_wifi_vif->net_stats.tx_packets++;
				sw_txhdr->cls_wifi_vif->net_stats.tx_bytes += amsdu_txhdr->skb->len;
				cls_wifi_tx_dfx_stats(sw_txhdr->cls_wifi_vif,
						sw_txhdr->cls_wifi_sta, 0, 0);
			}
#if (WIFI_DRV_OPTI_SPIN_LOCK_CFG)
			if (free_skb && (skb_array_size_max > 1)
				&& (skb_array_size_max > skb_array_offset)) {
				free_skb[skb_array_offset] = (void*)amsdu_txhdr->skb;
				skb_array_offset++;
			}else {
				consume_skb(amsdu_txhdr->skb);
			}
#else
			consume_skb(amsdu_txhdr->skb);
#endif
		}
	}
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */

	if (!(sw_txhdr->desc.api.host.flags & TXU_CNTRL_MGMT) &&
			TXU_STATUS_IS_SET(ACKNOWLEDGED, cfm->status)) {
		sw_txhdr->cls_wifi_vif->net_stats.tx_packets++;
		sw_txhdr->cls_wifi_vif->net_stats.tx_bytes += skb->len;
		eth = (struct ethhdr *)skb_mac_header(skb);
		is_bc = is_broadcast_ether_addr(eth->h_dest);
		is_mc = is_multicast_ether_addr(eth->h_dest);
		cls_wifi_tx_dfx_stats(sw_txhdr->cls_wifi_vif,
				sw_txhdr->cls_wifi_sta, is_bc, is_mc);
	}

	cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &sw_txhdr->ipc_data);
	cls_wifi_ipc_sta_buffer(cls_wifi_hw, txq->sta, txq->tid, -sw_txhdr->frame_len);
	cls_wifi_tx_statistic(sw_txhdr->cls_wifi_vif, txq, cfm->status, cfm->rate_config,
		sw_txhdr->frame_len, (sw_txhdr->desc.api.host.flags & TXU_CNTRL_MGMT));

	kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);
	skb_pull(skb, CLS_WIFI_TX_HEADROOM);
#if (WIFI_DRV_OPTI_SPIN_LOCK_CFG)
	if(free_skb && (skb_array_size_max > 1)
		&& (skb_array_size_max > skb_array_offset)) {
		free_skb[skb_array_offset] = (void*)skb;
		skb_array_offset++;
	}else {
		consume_skb(skb);
	}

	if(p_freeskb) {
		p_freeskb->skb_array_size = skb_array_offset;
	}
#else
	consume_skb(skb);
#endif

	return 0;
}

/**
 * cls_wifi_txq_credit_update - Update credit for one txq
 *
 * @cls_wifi_hw: Driver main data
 * @sta_idx: STA idx
 * @tid: TID
 * @update: offset to apply in txq credits
 *
 * Called when fw send ME_TX_CREDITS_UPDATE_IND message.
 * Apply @update to txq credits, and stop/start the txq if needed
 */
void cls_wifi_txq_credit_update(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx, u8 tid, s16 update)
{
	struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[sta_idx];
	struct cls_wifi_txq *txq;
	u16 pre_credits;

	txq = cls_wifi_txq_sta_get(sta, tid, cls_wifi_hw);

	spin_lock(&cls_wifi_hw->tx_lock);

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
#endif

	if (txq->idx != TXQ_INACTIVE) {
		pre_credits = txq->credits;
		txq->credits += update;
		pr_warn("update credits sta %d, tid %d, credits %d->%d, update %d\n",
				sta_idx, tid, pre_credits, txq->credits, update);
		trace_credit_update(txq, update);
		if (txq->credits <= 0)
			cls_wifi_txq_stop(txq, CLS_WIFI_TXQ_STOP_FULL);
		else
			cls_wifi_txq_start(txq, CLS_WIFI_TXQ_STOP_FULL);
	} else {
		spin_unlock(&cls_wifi_hw->tx_lock);
		return;
	}

	// Drop all the retry packets of a BA that was deleted
	if (update < txq->init_credits) {
		int packet;

		for (packet = 0; packet < txq->nb_retry; packet++) {
			cls_wifi_txq_drop_skb(txq, skb_peek(&txq->sk_list), cls_wifi_hw, true);
		}

		if ((skb_queue_empty(&txq->sk_list)) && (txq->status & CLS_WIFI_TXQ_IN_HWQ_LIST)) {
			///for debug SITS#5152
			pr_err("[error]%s sta_idx:%d, tid: %d, update: %d,txq: %px, packet: %d,txq->idx:%x\n",
				__func__, sta_idx, tid, update, txq, packet, txq->idx);
			///cls_wifi_txq_del_from_hw_list(txq);
		}
	}

	spin_unlock(&cls_wifi_hw->tx_lock);
}

int cls_wifi_hwq_credit_try_update(struct cls_wifi_hw *wifi_hw, u8 ac, int dec, uint8_t update)
{
	struct cls_wifi_hwq *hwq;

	hwq = &wifi_hw->hwq[ac];
	spin_lock_bh(&wifi_hw->tx_lock);
	if (dec && hwq->credits < update) {
		spin_unlock_bh(&wifi_hw->tx_lock);
		pr_info("%s ac %u credits %u\n", __func__, ac, hwq->credits);

		return -1;
	} else if (dec) {
		hwq->credits -= update;
	} else {
		hwq->credits += update;
	}

	spin_unlock_bh(&wifi_hw->tx_lock);

	return 0;
}

int cls_wifi_read_mpdu_payload(struct cls_wifi_hw *wifi_hw)
{
	struct file *fp = NULL;
	int len = 0;
	int i = 0;
	int j = 0;
	char tmp_str[3] = { 0 };
	loff_t pos = 0;
	uint8_t *buf;

	fp = filp_open("/tmp/wmactx.dat", O_RDWR, 0644);
	if (IS_ERR(fp)) {
		pr_err("%s: open file failed\n", __func__);

		return 0;
	}

	len = fp->f_inode->i_size;
	if (len < 0) {
		pr_err("%s: open file failed with len %d\n", __func__, len);

		return 0;
	}

	buf = kzalloc(len, GFP_KERNEL);
	if (!buf) {
		filp_close(fp, NULL);

		return 0;
	}

	for (; i < len - 1; i++) {
		if (kernel_read(fp, tmp_str, sizeof(tmp_str), &pos) < 2)
			break;

		buf[j++] = simple_strtoul(tmp_str, NULL, 16);
		if (j >= CLS_DPD_WMAC_TX_MAX_MPDU_LEN)
			break;
	}

	memcpy(&wifi_hw->dpd_wmac_tx_params.mpdu_payload[0], buf, j);
	pr_info("tx_data_len %d payload %02x %02x\n", j,
		wifi_hw->dpd_wmac_tx_params.mpdu_payload[0],
		wifi_hw->dpd_wmac_tx_params.mpdu_payload[1]);
	kfree(buf);
	filp_close(fp, NULL);

	return j;
}


dma_addr_t cls_wifi_allocate_tx_buffer(struct cls_wifi_hw *wifi_hw, int tx_buffer_size, int index)
{
	gfp_t flags;
	void *tx_buffer;
	dma_addr_t dma_addr;

	if (in_softirq())
		flags = GFP_ATOMIC;
	else
		flags = GFP_KERNEL;

	/* Allocate a structure that will contain the beamforming report */
	tx_buffer = kmalloc(tx_buffer_size, flags);
	if (!tx_buffer) {
		pr_info("%s failed to kmalloc size %d\n", __func__, tx_buffer_size);

		return -1;
	}

	wifi_hw->dpd_wmac_tx_params.mpdu_host_addr[index] = tx_buffer;
	if (wifi_hw->dpd_wmac_tx_params.req.source == DPD_WMAC_TX_RANDOM)
		get_random_bytes(tx_buffer, tx_buffer_size);
	else if (wifi_hw->dpd_wmac_tx_params.req.source == DPD_WMAC_TX_FIXED)
		memset(tx_buffer, 0xA5, tx_buffer_size);
	else
		memcpy(tx_buffer, wifi_hw->dpd_wmac_tx_params.mpdu_payload, tx_buffer_size);

	dma_addr = dma_map_single(wifi_hw->dev, tx_buffer, tx_buffer_size, DMA_TO_DEVICE);
	if (dma_mapping_error(wifi_hw->dev, dma_addr)) {
		kfree(tx_buffer);
		pr_info("%s dma_mapping_error\n", __func__);

		return -1;
	}

	return dma_addr;
}

int cls_wifi_dpd_wmac_tx_handler(struct cls_wifi_hw *wifi_hw, struct mm_dpd_wmac_tx_params_req *req)
{
	int i;

	if (cls_wifi_hwq_credit_try_update(wifi_hw, AC_BE, 1, req->mpdu_num) < 0) {
		pr_info("%s failed. No enough credits\n", __func__);

		return -1;
	}

	if (wifi_hw->dpd_wmac_tx_params.req.source == DPD_WMAC_TX_FROM_FILE)
		req->mpdu_payload_size = cls_wifi_read_mpdu_payload(wifi_hw);

	for (i = 0; i < req->mpdu_num; i++)
		req->mpdu_ddr_addr[i] = cls_wifi_allocate_tx_buffer(wifi_hw, req->mpdu_payload_size, i);

	cls_wifi_send_dpd_wmac_tx_cmd_req(wifi_hw, req);

	return 0;
}

int cls_wifi_dpd_wmac_tx_post_process(struct cls_wifi_hw *wifi_hw,
		struct mm_dpd_wmac_tx_ind *ind)
{
	int i;

	//pr_info("%s status %u\n", __func__, ind->status);
	cls_wifi_hwq_credit_try_update(wifi_hw, AC_BE, 0, wifi_hw->dpd_wmac_tx_params.req.mpdu_num);
	for (i = 0; i < wifi_hw->dpd_wmac_tx_params.req.mpdu_num; i++) {
		if (wifi_hw->dpd_wmac_tx_params.req.mpdu_ddr_addr[i])
			dma_unmap_single(wifi_hw->dev, wifi_hw->dpd_wmac_tx_params.req.mpdu_ddr_addr[i],
				wifi_hw->dpd_wmac_tx_params.req.mpdu_payload_size,
				DMA_TO_DEVICE);

		if (wifi_hw->dpd_wmac_tx_params.mpdu_host_addr[i])
			kfree(wifi_hw->dpd_wmac_tx_params.mpdu_host_addr[i]);

		wifi_hw->dpd_wmac_tx_params.req.mpdu_ddr_addr[i] = 0;
		wifi_hw->dpd_wmac_tx_params.mpdu_host_addr[i] = NULL;
	}

	return 0;
}

