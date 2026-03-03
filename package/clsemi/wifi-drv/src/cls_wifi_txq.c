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

#include "cls_wifi_defs.h"
#include "cls_wifi_tx.h"
#include "ipc_host.h"
#include "cls_wifi_events.h"
#include "cls_wifi_core.h"
#include "cls_wifi_main.h"

/******************************************************************************
 * Utils functions
 *****************************************************************************/
const int cls_tid_prio[CLS_NB_TXQ_PER_STA] = {8, 7, 6, 5, 4, 3, 0, 2, 1};

inline uint16_t first_vif_txq_idx(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx] * CLS_NB_TXQ_PER_STA);
}

inline uint16_t first_bcmc_txq_idx(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx] * CLS_NB_TID_PER_STA);
}

inline uint16_t first_unk_txq_idx(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx] * CLS_NB_TXQ_PER_STA
			+ cls_wifi_hw->plat->hw_params.vdev_max[cls_wifi_hw->radio_idx]);
}

inline uint16_t off_chan_txq_idx(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx] * CLS_NB_TXQ_PER_STA
			+ cls_wifi_hw->plat->hw_params.vdev_max[cls_wifi_hw->radio_idx] * CLS_NB_TXQ_PER_VIF);
}

inline uint16_t bcmc_txq_ndev_idx(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx] * CLS_NB_TID_PER_STA);
}

static inline int cls_wifi_txq_sta_idx(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta, u8 tid)
{
	if (is_multicast_sta(cls_wifi_hw, sta->sta_idx))
		return first_vif_txq_idx(cls_wifi_hw) + sta->vif_idx;
	else
		return (sta->sta_idx * CLS_NB_TXQ_PER_STA) + tid;
}

static inline int cls_wifi_txq_vif_idx(struct cls_wifi_vif *vif, u8 type)
{
	struct cls_wifi_hw *cls_wifi_hw = vif->cls_wifi_hw;
	return first_vif_txq_idx(vif->cls_wifi_hw) + master_vif_idx(vif)
						+ (type * cls_wifi_hw->plat->hw_params.vdev_max[cls_wifi_hw->radio_idx]);
}

struct cls_wifi_txq *cls_wifi_txq_sta_get(struct cls_wifi_sta *sta, u8 tid,
								  struct cls_wifi_hw * cls_wifi_hw)
{
	if (tid >= CLS_NB_TXQ_PER_STA)
		tid = 0;

	return (cls_wifi_hw->txq + cls_wifi_txq_sta_idx(cls_wifi_hw, sta, tid));
}

struct cls_wifi_txq *cls_wifi_txq_vif_get(struct cls_wifi_vif *vif, u8 type)
{
	if (type > CLS_UNK_TXQ_TYPE)
		type = CLS_BCMC_TXQ_TYPE;

	return &vif->cls_wifi_hw->txq[cls_wifi_txq_vif_idx(vif, type)];
}

static inline struct cls_wifi_sta *cls_wifi_txq_2_sta(struct cls_wifi_txq *txq)
{
	return txq->sta;
}

/**
 * cls_wifi_txq_sta_get_mu_hwq - Return MU HWQ assocaited to a station
 * @sta: station to retrieve HWQ for
 * @return MU HWQ associated to the station and NULL is STA is not currently
 * associated to a MU HWQ
 */
struct cls_wifi_hwq *cls_wifi_hwq_mu_sta_get(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_sta *sta)
{
	struct cls_wifi_txq *txq;

	txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);

	if (!txq || (txq->hwq->id < CLS_WIFI_HWQ_USER_BASE))
		return NULL;

	return txq->hwq;
}

/**
 * cls_wifi_hwq_ready - Test Whether an HWQ is ready to be scheduled
 *
 * @hwq: HWQ to test
 *
 * An HWQ is ready to be scheduled is it has credit, and for HE_MU HWQ if
 * the size limit is not reached.
 */
static inline bool cls_wifi_hwq_ready(struct cls_wifi_hwq *hwq)
{
	if (!hwq->credits)
		return false;

#ifdef CONFIG_CLS_WIFI_HEMU_TX
	if (hwq->size_limit && (hwq->size_pushed > hwq->size_limit))
		return false;
#endif

	return true;
}

static inline int cls_wifi_hwq_credits_multipe(uint8_t ac)
{
	if (ac < AC_MAX)
		return TXDESC_EXPAND_MULTIPLE;
	else
		return 1;
}

/******************************************************************************
 * Init/Deinit functions
 *****************************************************************************/
/**
 * cls_wifi_txq_init - Initialize a TX queue
 *
 * @txq: TX queue to be initialized
 * @idx: TX queue index
 * @status: TX queue initial status
 * @hwq: Associated HW queue
 * @ndev: Net device this queue belongs to
 *		(may be null for non netdev txq)
 *
 * Each queue is initialized with the credit of @CLS_TXQ_INITIAL_CREDITS.
 */
static void cls_wifi_txq_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq, int idx, u8 status,
						  struct cls_wifi_hwq *hwq, int tid,
						  struct cls_wifi_sta *sta, struct net_device *ndev
						  )
{
	txq->idx = idx;
	txq->status = status;
	txq->init_credits =
		cls_wifi_mod_params.txq_init_credits ? cls_wifi_mod_params.txq_init_credits : CLS_TXQ_INITIAL_CREDITS;
	txq->credits = txq->init_credits;
	txq->hwq_credits_quota = (hwq->credits / (cls_wifi_hwq_credits_multipe(hwq->id)));
	txq->pkt_sent = 0;
	skb_queue_head_init(&txq->sk_list);
	txq->last_retry_skb = NULL;
	txq->nb_retry = 0;
	txq->hwq = hwq;
	txq->sta = sta;
	txq->pkt_pushed = 0;
	txq->push_limit = 0;
	txq->tid = tid;
	txq->pkt_fc_drop = 0;
#ifdef CONFIG_MAC80211_TXQ
	txq->nb_ready_mac80211 = 0;
#endif

	txq->ps_id = LEGACY_PS_ID;
	if (idx < first_vif_txq_idx(cls_wifi_hw)) {
		u16 sta_idx = sta->sta_idx;
		int tid = idx - (sta_idx * CLS_NB_TXQ_PER_STA);
		if (tid < CLS_NB_TID_PER_STA)
			txq->ndev_idx = CLS_STA_NDEV_IDX(tid, sta_idx);
		else
			txq->ndev_idx = NDEV_NO_TXQ;
	} else if (idx < first_unk_txq_idx(cls_wifi_hw)) {
		txq->ndev_idx = first_bcmc_txq_idx(cls_wifi_hw);
	} else {
		txq->ndev_idx = NDEV_NO_TXQ;
	}
	txq->ndev = ndev;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	txq->amsdu = NULL;
	txq->wait2txq = NULL;
	txq->amsdu_len = 0;
	hrtimer_init(&txq->amsdu_agg_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_SOFT);
	txq->amsdu_agg_timer.function = cls_wifi_amsdu_agg_timeout;
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */
}

/**
 * cls_wifi_txq_drop_skb - Drop the buffer skb from the TX queue
 *
 * @txq:		  TX queue
 * @skb:		  skb packet that should be dropped.
 * @cls_wifi_hw:	  Driver main data
 * @retry_packet: Is it a retry packet
 *
 */
void cls_wifi_txq_drop_skb(struct cls_wifi_txq *txq, struct sk_buff *skb, struct cls_wifi_hw *cls_wifi_hw, bool retry_packet)
{
	struct cls_wifi_sw_txhdr *sw_txhdr;
	unsigned long queued_time = 0;

	skb_unlink(skb, &txq->sk_list);

	sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_hdr;

	queued_time = jiffies - sw_txhdr->jiffies;
	trace_txq_drop_skb(skb, txq, queued_time);

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (sw_txhdr->desc.api.host.packet_cnt > 1) {
		struct cls_wifi_amsdu_txhdr *amsdu_txhdr, *next;
		list_for_each_entry_safe(amsdu_txhdr, next, &sw_txhdr->amsdu.hdrs, list) {
			cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &amsdu_txhdr->ipc_data);
			dev_kfree_skb_any(amsdu_txhdr->skb);
		}
		if (txq->amsdu == sw_txhdr)
			txq->amsdu = NULL;
	}
#endif
	cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &sw_txhdr->ipc_data);
	kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);

	if (retry_packet) {
		txq->nb_retry--;
		if (txq->nb_retry == 0) {
			WARN(skb != txq->last_retry_skb,
				 "last dropped retry buffer is not the expected one");
			txq->last_retry_skb = NULL;
		}
	}

	dev_kfree_skb_any(skb);
}

/**
 * cls_wifi_txq_flush - Flush all buffers queued for a TXQ
 *
 * @cls_wifi_hw: main driver data
 * @txq: txq to flush
 */
void cls_wifi_txq_flush(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq)
{
	while(!skb_queue_empty(&txq->sk_list)) {
		cls_wifi_txq_drop_skb(txq, skb_peek(&txq->sk_list), cls_wifi_hw, txq->nb_retry);
	}

	if (txq->pkt_pushed && !cls_wifi_hw->heartbeat_uevent)
		dev_warn(cls_wifi_hw->dev, "TXQ[%d]: %d skb still pushed to the FW",
				 txq->idx, txq->pkt_pushed);
}

/**
 * cls_wifi_txq_deinit - De-initialize a TX queue
 *
 * @cls_wifi_hw: Driver main data
 * @txq: TX queue to be de-initialized
 * Any buffer stuck in a queue will be freed.
 */
static void cls_wifi_txq_deinit(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq)
{
	if (txq->idx == TXQ_INACTIVE)
		return;

	spin_lock_bh(&cls_wifi_hw->tx_lock);
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
#endif
	cls_wifi_txq_del_from_hw_list(txq);
	txq->idx = TXQ_INACTIVE;
	spin_unlock_bh(&cls_wifi_hw->tx_lock);

	cls_wifi_txq_flush(cls_wifi_hw, txq);
}

/**
 * cls_wifi_txq_vif_init - Initialize all TXQ linked to a vif
 *
 * @cls_wifi_hw: main driver data
 * @cls_wifi_vif: Pointer on VIF
 * @status: Intial txq status
 *
 * Softmac : 1 VIF TXQ per HWQ
 *
 * Fullmac : 1 VIF TXQ for BC/MC
 *		   1 VIF TXQ for MGMT to unknown STA
 */
void cls_wifi_txq_vif_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
					   u8 status)
{
	struct cls_wifi_txq *txq;
	int idx;

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_BCMC_TXQ_TYPE);
	idx = cls_wifi_txq_vif_idx(cls_wifi_vif, CLS_BCMC_TXQ_TYPE);
	cls_wifi_txq_init(cls_wifi_hw, txq, idx, status, &cls_wifi_hw->hwq[CLS_WIFI_HWQ_BE], 0,
				  &cls_wifi_hw->sta_table[cls_wifi_vif->ap.bcmc_index], cls_wifi_vif->ndev);

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	idx = cls_wifi_txq_vif_idx(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	cls_wifi_txq_init(cls_wifi_hw, txq, idx, status, &cls_wifi_hw->hwq[CLS_WIFI_HWQ_VO], TID_MGT,
				  NULL, cls_wifi_vif->ndev);
}

/**
 * cls_wifi_txq_vif_deinit - Deinitialize all TXQ linked to a vif
 *
 * @cls_wifi_hw: main driver data
 * @cls_wifi_vif: Pointer on VIF
 */
void cls_wifi_txq_vif_deinit(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_txq *txq;

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_BCMC_TXQ_TYPE);
	cls_wifi_txq_deinit(cls_wifi_hw, txq);

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	cls_wifi_txq_deinit(cls_wifi_hw, txq);
}


/**
 * cls_wifi_txq_sta_init - Initialize TX queues for a STA
 *
 * @cls_wifi_hw: Main driver data
 * @cls_wifi_sta: STA for which tx queues need to be initialized
 * @status: Intial txq status
 *
 * This function initialize all the TXQ associated to a STA.
 * Softmac : 1 TXQ per TID
 *
 * Fullmac : 1 TXQ per TID (limited to 8)
 *		   1 TXQ for MGMT
 */
void cls_wifi_txq_sta_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
					   u8 status)
{
	struct cls_wifi_txq *txq;
	int tid, idx;
	struct cls_wifi_vif *cls_wifi_vif = cls_wifi_hw->vif_table[cls_wifi_sta->vif_idx];
	idx = cls_wifi_txq_sta_idx(cls_wifi_hw, cls_wifi_sta, 0);

	foreach_sta_txq(cls_wifi_sta, txq, tid, cls_wifi_hw) {
		cls_wifi_txq_init(cls_wifi_hw, txq, idx, status, &cls_wifi_hw->hwq[cls_wifi_tid2hwq[tid]], tid,
					  cls_wifi_sta, cls_wifi_vif->ndev);
		txq->ps_id = cls_wifi_sta->uapsd_tids & (1 << tid) ? UAPSD_ID : LEGACY_PS_ID;
		idx++;
	}

	cls_wifi_ipc_sta_buffer_init(cls_wifi_hw, cls_wifi_sta->sta_idx);
}

/**
 * cls_wifi_txq_sta_deinit - Deinitialize TX queues for a STA
 *
 * @cls_wifi_hw: Main driver data
 * @cls_wifi_sta: STA for which tx queues need to be deinitialized
 */
void cls_wifi_txq_sta_deinit(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta)
{
	struct cls_wifi_txq *txq;
	int tid;

	foreach_sta_txq(cls_wifi_sta, txq, tid, cls_wifi_hw) {
		cls_wifi_txq_deinit(cls_wifi_hw, txq);
	}
}

/**
 * cls_wifi_txq_sta_start_he_mu - Start station's TXQ for HE MU transmission
 *
 * @cls_wifi_hw: Main driver data
 * @sta: STA for which tx queues need to be started
 * @mu_hwq: MU HWQ to use for HE MU
 * @tid_map: Bitmap of TID that should be enabled
 *
 * This function starts HE MU for a given station, meaning that:
 * - Its TXQ are now linked to a MU HWQ (and not ACx HWQ anymore)
 * - Its TXQ, whose tid is present in the TID map, are started for the reason
 *   CLS_WIFI_TXQ_STOP_MU. Other TXQ are stopped for the same reason.
 * - The TXQ for MGMT remains linked to AC HWQ (TODO: update for softmac).
 */
void cls_wifi_txq_sta_start_he_mu(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
							  struct cls_wifi_hwq *mu_hwq, uint16_t tid_map)
{
	struct cls_wifi_txq *txq;
	int tid;

	foreach_sta_txq(sta, txq, tid, cls_wifi_hw) {
		if (tid == 8)
			break;

		txq->hwq = mu_hwq;
		if (tid_map & BIT(tid))
			cls_wifi_txq_start(txq, CLS_WIFI_TXQ_STOP_MU);
		else
			cls_wifi_txq_stop(txq, CLS_WIFI_TXQ_STOP_MU);
	}
}

/**
 * cls_wifi_txq_sta_stop_he_mu - Stop TXQ of a station because it is a HE MU station
 * that is not currrently scheduled for HE MU transmission.
 * @cls_wifi_hw: Main driver data
 * @sta: STA for which tx queues need to be stopped
 *
 * The TXQ for MGMT frame is not stopped as it is not taken into account for HE MU
 * transmission. (TODO: update for softmac)
 * This function is called on HE stations scheduled for HE MU when a new DL map is
 * received (independently of whether it is present in the new DL map or not).
 * It is also called when HE MU is activated on all HE stations (i.e. at this time
 * HE stations are linked to ACx HWQ like any non-HE stations).
 */
void cls_wifi_txq_sta_stop_he_mu(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)
{
	struct cls_wifi_txq *txq;
	int tid;

	foreach_sta_txq(sta, txq, tid, cls_wifi_hw) {
		if (tid == 8)
			break;
		cls_wifi_txq_stop(txq, CLS_WIFI_TXQ_STOP_MU);
	}
}

/**
 * cls_wifi_txq_sta_disable_he_mu - Reconfigure station for non HE-MU transmission
 * (i.e. using ACx HWQ) as HE MU as been disabled
 *
 * @cls_wifi_hw: Main driver data
 * @sta: STA for which tx queues need to be associate to ACx HWQ
 */
void cls_wifi_txq_sta_disable_he_mu(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)
{
	struct cls_wifi_txq *txq;
	int tid;

	foreach_sta_txq(sta, txq, tid, cls_wifi_hw) {
		txq->hwq = &cls_wifi_hw->hwq[cls_wifi_tid2hwq[tid]];
		cls_wifi_txq_start(txq, CLS_WIFI_TXQ_STOP_MU);
	}
}

/**
 * cls_wifi_txq_unk_vif_init - Initialize TXQ for unknown STA linked to a vif
 *
 * @cls_wifi_vif: Pointer on VIF
 */
void cls_wifi_txq_unk_vif_init(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct cls_wifi_txq *txq;
	int idx;

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	idx = cls_wifi_txq_vif_idx(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	cls_wifi_txq_init(cls_wifi_hw, txq, idx, 0, &cls_wifi_hw->hwq[CLS_WIFI_HWQ_VO], TID_MGT, NULL, cls_wifi_vif->ndev);
}

/**
 * cls_wifi_txq_unk_vif_deinit - Deinitialize TXQ for unknown STA linked to a vif
 *
 * @cls_wifi_vif: Pointer on VIF
 */
void cls_wifi_txq_unk_vif_deinit(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_txq *txq;

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	cls_wifi_txq_deinit(cls_wifi_vif->cls_wifi_hw, txq);
}

/**
 * cls_wifi_txq_offchan_init - Initialize TX queue for the transmission on a offchannel
 *
 * @vif: Interface for which the queue has to be initialized
 *
 * NOTE: Offchannel txq is only active for the duration of the ROC
 */
void cls_wifi_txq_offchan_init(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;
	struct cls_wifi_txq *txq;

	txq = cls_wifi_hw->txq + off_chan_txq_idx(cls_wifi_hw);
	cls_wifi_txq_init(cls_wifi_hw, txq, off_chan_txq_idx(cls_wifi_hw), CLS_WIFI_TXQ_STOP_CHAN,
				  &cls_wifi_hw->hwq[CLS_WIFI_HWQ_VO], TID_MGT, NULL, cls_wifi_vif->ndev);
}

/**
 * cls_wifi_deinit_offchan_txq - Deinitialize TX queue for offchannel
 *
 * @vif: Interface that manages the STA
 *
 * This function deintialize txq for one STA.
 * Any buffer stuck in a queue will be freed.
 */
void cls_wifi_txq_offchan_deinit(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_txq *txq;
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;

	txq = cls_wifi_hw->txq + off_chan_txq_idx(cls_wifi_hw);
	cls_wifi_txq_deinit(cls_wifi_vif->cls_wifi_hw, txq);
}


/**
 * cls_wifi_txq_tdls_vif_init - Initialize TXQ vif for TDLS
 *
 * @cls_wifi_vif: Pointer on VIF
 */
void cls_wifi_txq_tdls_vif_init(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;

	if (!(cls_wifi_hw->wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS))
		return;

	cls_wifi_txq_unk_vif_init(cls_wifi_vif);
}

/**
 * cls_wifi_txq_tdls_vif_deinit - Deinitialize TXQ vif for TDLS
 *
 * @cls_wifi_vif: Pointer on VIF
 */
void cls_wifi_txq_tdls_vif_deinit(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_vif->cls_wifi_hw;

	if (!(cls_wifi_hw->wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS))
		return;

	cls_wifi_txq_unk_vif_deinit(cls_wifi_vif);
}

/**
 * cls_wifi_txq_drop_old_traffic - Drop pkt queued for too long in a TXQ
 *
 * @txq: TXQ to process
 * @cls_wifi_hw: Driver main data
 * @skb_timeout: Max queue duration, in jiffies, for this queue
 * @dropped: Updated to inidicate if at least one skb was dropped
 *
 * @return Whether there is still pkt queued in this queue.
 */
static bool cls_wifi_txq_drop_old_traffic(struct cls_wifi_txq *txq, struct cls_wifi_hw *cls_wifi_hw,
									  unsigned long skb_timeout, bool *dropped)
{
	struct sk_buff *skb, *skb_next;
	bool pkt_queued = false;
	int retry_packet = txq->nb_retry;

	if (txq->idx == TXQ_INACTIVE)
		return false;

	spin_lock_bh(&cls_wifi_hw->tx_lock);

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
#endif

	skb_queue_walk_safe(&txq->sk_list, skb, skb_next) {

		struct cls_wifi_sw_txhdr *sw_txhdr;

		if (retry_packet) {
			// Don't drop retry packets
			retry_packet--;
			continue;
		}

		sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_hdr;

		if (!time_after(jiffies, sw_txhdr->jiffies + skb_timeout)) {
			pkt_queued = true;
			break;
		}

		*dropped = true;
		cls_wifi_txq_drop_skb(txq, skb, cls_wifi_hw, false);
		if (txq->sta && txq->sta->ps.active) {
			txq->sta->ps.pkt_ready[txq->ps_id]--;
			if (txq->sta->ps.pkt_ready[txq->ps_id] == 0)
				cls_wifi_set_traffic_status(cls_wifi_hw, txq->sta, false, txq->ps_id);

			// drop packet during PS service period ...
			if (txq->sta->ps.sp_cnt[txq->ps_id]) {
				txq->sta->ps.sp_cnt[txq->ps_id] --;
				if (txq->push_limit)
					txq->push_limit--;
				if (WARN(((txq->ps_id == UAPSD_ID) &&
						  (txq->sta->ps.sp_cnt[txq->ps_id] == 0)),
						 "Drop last packet of UAPSD service period")) {
					// TODO: inform FW to end SP
				}
			}
			trace_ps_drop(txq->sta);
		}
	}

	if (skb_queue_empty(&txq->sk_list)) {
		cls_wifi_txq_del_from_hw_list(txq);
		txq->pkt_sent = 0;
	}

	spin_unlock_bh(&cls_wifi_hw->tx_lock);

	/* restart netdev queue if number no more queued buffer */
	if (unlikely(txq->status & CLS_WIFI_TXQ_NDEV_FLOW_CTRL) &&
		skb_queue_empty(&txq->sk_list)) {
		txq->status &= ~CLS_WIFI_TXQ_NDEV_FLOW_CTRL;
		//netif_wake_subqueue(txq->ndev, txq->ndev_idx);
		trace_txq_flowctrl_restart(txq);
	}

	return pkt_queued;
}

/**
 * cls_wifi_txq_drop_ap_vif_old_traffic - Drop pkt queued for too long in TXQs
 * linked to an "AP" vif (AP, MESH, P2P_GO)
 *
 * @vif: Vif to process
 * @return Whether there is still pkt queued in any TXQ.
 */
static bool cls_wifi_txq_drop_ap_vif_old_traffic(struct cls_wifi_vif *vif)
{
	struct cls_wifi_sta *sta;
	unsigned long timeout = (vif->ap.bcn_interval * HZ * 3) >> 10;
	bool pkt_queued = false;
	bool pkt_dropped = false;

	// Should never be needed but still check VIF queues
	cls_wifi_txq_drop_old_traffic(cls_wifi_txq_vif_get(vif, CLS_BCMC_TXQ_TYPE),
							  vif->cls_wifi_hw, CLS_WIFI_TXQ_MAX_QUEUE_JIFFIES, &pkt_dropped);
	if (pkt_dropped)
		pr_warn("packet dropped in radio %d vif %d queue %d",
			vif->cls_wifi_hw->radio_idx, vif->vif_index, CLS_BCMC_TXQ_TYPE);
	pkt_dropped = false;

	cls_wifi_txq_drop_old_traffic(cls_wifi_txq_vif_get(vif, CLS_UNK_TXQ_TYPE),
							  vif->cls_wifi_hw, CLS_WIFI_TXQ_MAX_QUEUE_JIFFIES, &pkt_dropped);
	if (pkt_dropped)
		pr_warn("packet dropped in radio %d vif %d queue %d",
			vif->cls_wifi_hw->radio_idx, vif->vif_index, CLS_UNK_TXQ_TYPE);

	spin_lock_bh(&vif->cls_wifi_hw->rosource_lock);
	list_for_each_entry(sta, &vif->ap.sta_list, list) {
		struct cls_wifi_txq *txq;
		int tid;
		foreach_sta_txq(sta, txq, tid, vif->cls_wifi_hw) {
			pkt_dropped = false;
			pkt_queued |= cls_wifi_txq_drop_old_traffic(txq, vif->cls_wifi_hw,
								timeout * sta->listen_interval,
								&pkt_dropped);
			if (pkt_dropped)
				pr_warn("packet dropped txq %d in radio %d vif %d sta %d (%pM)",
					txq->idx, vif->cls_wifi_hw->radio_idx,
					vif->vif_index, sta->sta_idx, sta->mac_addr);
		}
	}
	spin_unlock_bh(&vif->cls_wifi_hw->rosource_lock);

	return pkt_queued;
}

/**
 * cls_wifi_txq_drop_sta_vif_old_traffic - Drop pkt queued for too long in TXQs
 * linked to a "STA" vif. In theory this should not be required as there is no
 * case where traffic can accumulate in a STA interface.
 *
 * @vif: Vif to process
 * @return Whether there is still pkt queued in any TXQ.
 */
static bool cls_wifi_txq_drop_sta_vif_old_traffic(struct cls_wifi_vif *vif)
{
	struct cls_wifi_txq *txq;
	bool pkt_queued = false, pkt_dropped = false;
	int tid;

	if (vif->tdls_status == TDLS_LINK_ACTIVE) {
		txq = cls_wifi_txq_vif_get(vif, CLS_UNK_TXQ_TYPE);
		pkt_queued |= cls_wifi_txq_drop_old_traffic(txq, vif->cls_wifi_hw,
												CLS_WIFI_TXQ_MAX_QUEUE_JIFFIES,
												&pkt_dropped);
		foreach_sta_txq(vif->sta.tdls_sta, txq, tid, vif->cls_wifi_hw) {
			pkt_queued |= cls_wifi_txq_drop_old_traffic(txq, vif->cls_wifi_hw,
													CLS_WIFI_TXQ_MAX_QUEUE_JIFFIES,
													&pkt_dropped);
		}
	}

	if (vif->sta.ap) {
		foreach_sta_txq(vif->sta.ap, txq, tid, vif->cls_wifi_hw) {
			pkt_queued |= cls_wifi_txq_drop_old_traffic(txq, vif->cls_wifi_hw,
													CLS_WIFI_TXQ_MAX_QUEUE_JIFFIES,
													&pkt_dropped);
		}
	}

	if (pkt_dropped)
		netdev_warn(vif->ndev, "Dropped packet in STA interface TXQs");
	return pkt_queued;
}

/**
 * cls_wifi_txq_cleanup_timer_cb - callack for TXQ cleaup timer
 * Used to prevent pkt to accumulate in TXQ. The main use case is for AP
 * interface with client in Power Save mode but just in case all TXQs are
 * checked.
 *
 * @t: timer structure
 */
static void cls_wifi_txq_cleanup_timer_cb(struct timer_list *t)
{
	struct cls_wifi_hw *cls_wifi_hw = from_timer(cls_wifi_hw, t, txq_cleanup);
	struct cls_wifi_vif *vif;
	bool pkt_queue = false;

	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		switch (CLS_WIFI_VIF_TYPE(vif)) {
			case NL80211_IFTYPE_AP:
			case NL80211_IFTYPE_P2P_GO:
			case NL80211_IFTYPE_MESH_POINT:
				pkt_queue |= cls_wifi_txq_drop_ap_vif_old_traffic(vif);
				break;
			case NL80211_IFTYPE_STATION:
			case NL80211_IFTYPE_P2P_CLIENT:
				 pkt_queue |= cls_wifi_txq_drop_sta_vif_old_traffic(vif);
				 break;
			case NL80211_IFTYPE_AP_VLAN:
			case NL80211_IFTYPE_MONITOR:
			default:
				continue;
		}
	}

	if (pkt_queue)
		mod_timer(t, jiffies + CLS_WIFI_TXQ_CLEANUP_INTERVAL);
}

/**
 * cls_wifi_txq_start_cleanup_timer - Start 'cleanup' timer if not started
 *
 * @cls_wifi_hw: Driver main data
 */
void cls_wifi_txq_start_cleanup_timer(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)
{
	if (sta && !is_multicast_sta(cls_wifi_hw, sta->sta_idx) &&
		!timer_pending(&cls_wifi_hw->txq_cleanup))
		mod_timer(&cls_wifi_hw->txq_cleanup, jiffies + CLS_WIFI_TXQ_CLEANUP_INTERVAL);
}

/**
 * cls_wifi_txq_prepare - Global initialization of txq
 *
 * @cls_wifi_hw: Driver main data
 */
int cls_wifi_txq_prepare(struct cls_wifi_hw *cls_wifi_hw)
{
	int i;
	uint32_t txq_nb = CLS_NB_TXQ(hw_vdev_max(cls_wifi_hw), hw_remote_sta_max(cls_wifi_hw));

	cls_wifi_hw->txq = kmalloc(txq_nb * sizeof(struct cls_wifi_txq), GFP_KERNEL);
	if (!cls_wifi_hw->txq)
		return -1;

	for (i = 0; i < txq_nb; i++) {
		cls_wifi_hw->txq[i].idx = TXQ_INACTIVE;
	}

	timer_setup(&cls_wifi_hw->txq_cleanup, cls_wifi_txq_cleanup_timer_cb, 0);
	return 0;
}

void cls_wifi_txq_free(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->txq) {
		kfree(cls_wifi_hw->txq);
		cls_wifi_hw->txq = NULL;
	}
	return;
}

int cls_wifi_statable_prepare(struct cls_wifi_hw *cls_wifi_hw)
{
	//int i;
	uint32_t statable_nb = hw_all_sta_max(cls_wifi_hw);

	cls_wifi_hw->sta_table = kmalloc(statable_nb * sizeof(struct cls_wifi_sta), GFP_KERNEL);

	if (!cls_wifi_hw->sta_table)
		return -1;

	memset(cls_wifi_hw->sta_table, 0, (statable_nb * sizeof(struct cls_wifi_sta)));
	return 0;
}

void cls_wifi_statable_free(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->sta_table) {
		kfree(cls_wifi_hw->sta_table);
		cls_wifi_hw->sta_table = NULL;
	}
	return;
}

/**
 * cls_wifi_hwq_prepare - Global initialization ofm rxbuf
 *
 * @cls_wifi_hw: Driver main data
 */
int cls_wifi_rxbuf_prepare(struct cls_wifi_hw *cls_wifi_hw)
{
#define RXBUF_PER_STA (64)
#define RXBUF_PER_MU (256)
#define REORDBUF (256)
#define STA_MIN (2)
#define RXBUFF_MIN (CLS_MAX_MSDU_PER_RX_AMSDU * REORDBUF * STA_MIN)
#define RXBUFF_MAX (8192)

	cls_wifi_hw->rxbufs_nb = RXBUF_PER_STA * hw_remote_sta_max(cls_wifi_hw);
	if (cls_wifi_hw->rxbufs_nb < RXBUF_PER_MU * hw_mu_user_max(cls_wifi_hw))
		cls_wifi_hw->rxbufs_nb = RXBUF_PER_MU * hw_mu_user_max(cls_wifi_hw);
	if (cls_wifi_hw->rxbufs_nb < RXBUFF_MIN)
		cls_wifi_hw->rxbufs_nb = RXBUFF_MIN;
	if (cls_wifi_hw->rxbufs_nb > RXBUFF_MAX)
		cls_wifi_hw->rxbufs_nb = RXBUFF_MAX;
	cls_wifi_hw->rxbufs = kmalloc(cls_wifi_hw->rxbufs_nb * sizeof(struct cls_wifi_ipc_buf), GFP_KERNEL);

	if (!cls_wifi_hw->rxbufs)
		return -1;

	memset(cls_wifi_hw->rxbufs, 0, (cls_wifi_hw->rxbufs_nb * sizeof(struct cls_wifi_ipc_buf)));
	return 0;
}

void cls_wifi_rxbuf_free(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->rxbufs) {
		kfree(cls_wifi_hw->rxbufs);
		cls_wifi_hw->rxbufs = NULL;
	}
	return;
}

/******************************************************************************
 * Start/Stop functions
 *****************************************************************************/
/**
 * cls_wifi_txq_add_to_hw_list - Add TX queue to a HW queue schedule list.
 *
 * @txq: TX queue to add
 *
 * Add the TX queue if not already present in the HW queue list.
 * To be called with tx_lock hold
 */
void cls_wifi_txq_add_to_hw_list(struct cls_wifi_txq *txq)
{
	if (!(txq->status & CLS_WIFI_TXQ_IN_HWQ_LIST)) {
		trace_txq_add_to_hw(txq);
		txq->status |= CLS_WIFI_TXQ_IN_HWQ_LIST;
		list_add_tail(&txq->sched_list, &txq->hwq->list);
		txq->hwq->need_processing = true;
	}
}

/**
 * cls_wifi_txq_del_from_hw_list - Delete TX queue from a HW queue schedule list.
 *
 * @txq: TX queue to delete
 *
 * Remove the TX queue from the HW queue list if present.
 * To be called with tx_lock hold
 */
void cls_wifi_txq_del_from_hw_list(struct cls_wifi_txq *txq)
{
	if (txq->status & CLS_WIFI_TXQ_IN_HWQ_LIST) {
		trace_txq_del_from_hw(txq);
		txq->status &= ~CLS_WIFI_TXQ_IN_HWQ_LIST;
		list_del(&txq->sched_list);
	}
}

/**
 * cls_wifi_txq_skb_ready - Check if skb are available for the txq
 *
 * @txq: Pointer on txq
 * @return True if there are buffer ready to be pushed on this txq,
 * false otherwise
 */
static inline bool cls_wifi_txq_skb_ready(struct cls_wifi_txq *txq)
{
#ifdef CONFIG_MAC80211_TXQ
	if (txq->nb_ready_mac80211 != NOT_MAC80211_TXQ)
		return ((txq->nb_ready_mac80211 > 0) || !skb_queue_empty(&txq->sk_list));
	else
#endif
	return !skb_queue_empty(&txq->sk_list);
}

/**
 * cls_wifi_txq_start - Try to Start one TX queue
 *
 * @txq: TX queue to start
 * @reason: reason why the TX queue is started (among CLS_WIFI_TXQ_STOP_xxx)
 *
 * Re-start the TX queue for one reason.
 * If after this the txq is no longer stopped and some buffers are ready,
 * the TX queue is also added to HW queue list.
 * To be called with tx_lock hold
 */
void cls_wifi_txq_start(struct cls_wifi_txq *txq, u16 reason)
{
	BUG_ON(txq==NULL);
	if (txq->idx != TXQ_INACTIVE && (txq->status & reason))
	{
		trace_txq_start(txq, reason);
		txq->status &= ~reason;
		if (!cls_wifi_txq_is_stopped(txq) && cls_wifi_txq_skb_ready(txq))
			cls_wifi_txq_add_to_hw_list(txq);
	}
}

/**
 * cls_wifi_txq_stop - Stop one TX queue
 *
 * @txq: TX queue to stop
 * @reason: reason why the TX queue is stopped (among CLS_WIFI_TXQ_STOP_xxx)
 *
 * Stop the TX queue. It will remove the TX queue from HW queue list
 * To be called with tx_lock hold
 */
void cls_wifi_txq_stop(struct cls_wifi_txq *txq, u16 reason)
{
	BUG_ON(txq==NULL);
	if (txq->idx != TXQ_INACTIVE)
	{
		trace_txq_stop(txq, reason);
		txq->status |= reason;
		cls_wifi_txq_del_from_hw_list(txq);
	}
}


/**
 * cls_wifi_txq_sta_start - Start all the TX queue linked to a STA
 *
 * @sta: STA whose TX queues must be re-started
 * @reason: Reason why the TX queue are restarted (among CLS_WIFI_TXQ_STOP_xxx)
 * @cls_wifi_hw: Driver main data
 *
 * This function will re-start all the TX queues of the STA for the reason
 * specified. It can be :
 * - CLS_WIFI_TXQ_STOP_STA_PS: the STA is no longer in power save mode
 * - CLS_WIFI_TXQ_STOP_VIF_PS: the VIF is in power save mode (p2p absence)
 * - CLS_WIFI_TXQ_STOP_CHAN: the STA's VIF is now on the current active channel
 *
 * Any TX queue with buffer ready and not Stopped for other reasons, will be
 * added to the HW queue list
 * To be called with tx_lock hold
 */
void cls_wifi_txq_sta_start(struct cls_wifi_sta *cls_wifi_sta, u16 reason,
						struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_txq *txq;
	int tid;

	trace_txq_sta_start(cls_wifi_sta->sta_idx);

	foreach_sta_txq(cls_wifi_sta, txq, tid, cls_wifi_hw) {
		cls_wifi_txq_start(txq, reason);
	}
}


/**
 * cls_wifi_stop_sta_txq - Stop all the TX queue linked to a STA
 *
 * @sta: STA whose TX queues must be stopped
 * @reason: Reason why the TX queue are stopped (among CLS_WIFI_TX_STOP_xxx)
 * @cls_wifi_hw: Driver main data
 *
 * This function will stop all the TX queues of the STA for the reason
 * specified. It can be :
 * - CLS_WIFI_TXQ_STOP_STA_PS: the STA is in power save mode
 * - CLS_WIFI_TXQ_STOP_VIF_PS: the VIF is in power save mode (p2p absence)
 * - CLS_WIFI_TXQ_STOP_CHAN: the STA's VIF is not on the current active channel
 *
 * Any TX queue present in a HW queue list will be removed from this list.
 * To be called with tx_lock hold
 */
void cls_wifi_txq_sta_stop(struct cls_wifi_sta *cls_wifi_sta, u16 reason,
						struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_txq *txq;
	int tid;

	if (!cls_wifi_sta)
		return;

	trace_txq_sta_stop(cls_wifi_sta->sta_idx);
	foreach_sta_txq(cls_wifi_sta, txq, tid, cls_wifi_hw) {
		cls_wifi_txq_stop(txq, reason);
	}
}

void cls_wifi_txq_tdls_sta_start(struct cls_wifi_vif *cls_wifi_vif, u16 reason,
							 struct cls_wifi_hw *cls_wifi_hw)
{
	trace_txq_vif_start(cls_wifi_vif->vif_index);
	spin_lock_bh(&cls_wifi_hw->tx_lock);

	if (cls_wifi_vif->sta.tdls_sta)
		cls_wifi_txq_sta_start(cls_wifi_vif->sta.tdls_sta, reason, cls_wifi_hw);

	spin_unlock_bh(&cls_wifi_hw->tx_lock);
}

void cls_wifi_txq_tdls_sta_stop(struct cls_wifi_vif *cls_wifi_vif, u16 reason,
							struct cls_wifi_hw *cls_wifi_hw)
{
	trace_txq_vif_stop(cls_wifi_vif->vif_index);

	spin_lock_bh(&cls_wifi_hw->tx_lock);

	if (cls_wifi_vif->sta.tdls_sta)
		cls_wifi_txq_sta_stop(cls_wifi_vif->sta.tdls_sta, reason, cls_wifi_hw);

	spin_unlock_bh(&cls_wifi_hw->tx_lock);
}

static inline
void cls_wifi_txq_vif_for_each_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
							   void (*f)(struct cls_wifi_sta *, u16, struct cls_wifi_hw *),
							   u16 reason)
{

	switch (CLS_WIFI_VIF_TYPE(cls_wifi_vif)) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
	{
		if (cls_wifi_vif->tdls_status == TDLS_LINK_ACTIVE)
			f(cls_wifi_vif->sta.tdls_sta, reason, cls_wifi_hw);
		if (!WARN_ON(cls_wifi_vif->sta.ap == NULL))
			f(cls_wifi_vif->sta.ap, reason, cls_wifi_hw);
		break;
	}
	case NL80211_IFTYPE_AP_VLAN:
		cls_wifi_vif = cls_wifi_vif->ap_vlan.master;
		fallthrough;
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_P2P_GO:
	{
		struct cls_wifi_sta *sta;
		list_for_each_entry(sta, &cls_wifi_vif->ap.sta_list, list) {
			f(sta, reason, cls_wifi_hw);
		}
		break;
	}
	default:
		BUG();
		break;
	}
}

/**
 * cls_wifi_txq_vif_start - START TX queues of all STA associated to the vif
 *					  and vif's TXQ
 *
 * @vif: Interface to start
 * @reason: Start reason (CLS_WIFI_TXQ_STOP_CHAN or CLS_WIFI_TXQ_STOP_VIF_PS)
 * @cls_wifi_hw: Driver main data
 *
 * Iterate over all the STA associated to the vif and re-start them for the
 * reason @reason
 * Take tx_lock
 */
void cls_wifi_txq_vif_start(struct cls_wifi_vif *cls_wifi_vif, u16 reason,
						struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_txq *txq;

	trace_txq_vif_start(cls_wifi_vif->vif_index);

	spin_lock_bh(&cls_wifi_hw->tx_lock);

	//Reject if monitor interface
	if (cls_wifi_vif->wdev.iftype == NL80211_IFTYPE_MONITOR)
		goto end;

	if (cls_wifi_vif->roc_tdls && cls_wifi_vif->sta.tdls_sta && cls_wifi_vif->sta.tdls_sta->tdls.chsw_en) {
		cls_wifi_txq_sta_start(cls_wifi_vif->sta.tdls_sta, reason, cls_wifi_hw);
	}
	if (!cls_wifi_vif->roc_tdls) {
		cls_wifi_txq_vif_for_each_sta(cls_wifi_hw, cls_wifi_vif, cls_wifi_txq_sta_start, reason);
	}

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_BCMC_TXQ_TYPE);
	cls_wifi_txq_start(txq, reason);
	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	cls_wifi_txq_start(txq, reason);

end:

	spin_unlock_bh(&cls_wifi_hw->tx_lock);
}


/**
 * cls_wifi_txq_vif_stop - STOP TX queues of all STA associated to the vif
 *
 * @vif: Interface to stop
 * @arg: Stop reason (CLS_WIFI_TXQ_STOP_CHAN or CLS_WIFI_TXQ_STOP_VIF_PS)
 * @cls_wifi_hw: Driver main data
 *
 * Iterate over all the STA associated to the vif and stop them for the
 * reason CLS_WIFI_TXQ_STOP_CHAN or CLS_WIFI_TXQ_STOP_VIF_PS
 * Take tx_lock
 */
void cls_wifi_txq_vif_stop(struct cls_wifi_vif *cls_wifi_vif, u16 reason,
					   struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_txq *txq;

	trace_txq_vif_stop(cls_wifi_vif->vif_index);
	spin_lock_bh(&cls_wifi_hw->tx_lock);

	//Reject if monitor interface
	if (cls_wifi_vif->wdev.iftype == NL80211_IFTYPE_MONITOR)
		goto end;

	cls_wifi_txq_vif_for_each_sta(cls_wifi_hw, cls_wifi_vif, cls_wifi_txq_sta_stop, reason);

	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_BCMC_TXQ_TYPE);
	cls_wifi_txq_stop(txq, reason);
	txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
	cls_wifi_txq_stop(txq, reason);

end:

	spin_unlock_bh(&cls_wifi_hw->tx_lock);
}

/**
 * cls_wifi_start_offchan_txq - START TX queue for offchannel frame
 *
 * @cls_wifi_hw: Driver main data
 */
void cls_wifi_txq_offchan_start(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_txq *txq;

	txq = cls_wifi_hw->txq + off_chan_txq_idx(cls_wifi_hw);
	spin_lock_bh(&cls_wifi_hw->tx_lock);
	cls_wifi_txq_start(txq, CLS_WIFI_TXQ_STOP_CHAN);
	spin_unlock_bh(&cls_wifi_hw->tx_lock);
}

/**
 * cls_wifi_switch_vif_sta_txq - Associate TXQ linked to a STA to a new vif
 *
 * @sta: STA whose txq must be switched
 * @old_vif: Vif currently associated to the STA (may no longer be active)
 * @new_vif: vif which should be associated to the STA for now on
 *
 * This function will switch the vif (i.e. the netdev) associated to all STA's
 * TXQ. This is used when AP_VLAN interface are created.
 * If one STA is associated to an AP_vlan vif, it will be moved from the master
 * AP vif to the AP_vlan vif.
 * If an AP_vlan vif is removed, then STA will be moved back to mastert AP vif.
 *
 */
void cls_wifi_txq_sta_switch_vif(struct cls_wifi_sta *sta, struct cls_wifi_vif *old_vif,
							 struct cls_wifi_vif *new_vif)
{
	struct cls_wifi_hw *cls_wifi_hw = new_vif->cls_wifi_hw;
	struct cls_wifi_txq *txq;
	int i;

	/* start TXQ on the new interface, and update ndev field in txq */
	if (!netif_carrier_ok(new_vif->ndev))
		netif_carrier_on(new_vif->ndev);
	txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);
	for (i = 0; i < CLS_NB_TID_PER_STA; i++, txq++) {
		txq->ndev = new_vif->ndev;
		//netif_wake_subqueue(txq->ndev, txq->ndev_idx);
	}
}

/******************************************************************************
 * TXQ queue/schedule functions
 *****************************************************************************/
/**
 * cls_wifi_txq_queue_skb - Queue a buffer in a TX queue
 *
 * @skb: Buffer to queue
 * @txq: TX Queue in which the buffer must be added
 * @cls_wifi_hw: Driver main data
 * @retry: Should it be queued in the retry list
 * @skb_prev: If not NULL insert buffer after this skb instead of the tail
 * of the list (ignored if retry is true)
 *
 * @return: Return 1 if txq has been added to hwq list, 0 otherwise
 *
 * Add a buffer in the buffer list of the TX queue
 * and add this TX queue in the HW queue list if the txq is not stopped.
 * If this is a retry packet it is added after the last retry packet or at the
 * beginning if there is no retry packet queued.
 *
 * If the STA is in PS mode and this is the first packet queued for this txq
 * update TIM.
 *
 * To be called with tx_lock hold
 */
int cls_wifi_txq_queue_skb(struct sk_buff *skb, struct cls_wifi_txq *txq,
					   struct cls_wifi_hw *cls_wifi_hw,  bool retry,
					   struct sk_buff *skb_prev)
{
	struct cls_wifi_sw_txhdr *sw_txhdr;

	sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_hdr;

	if (txq->idx == TXQ_INACTIVE) {
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
		if ((txq->wait2txq) && (txq->wait2txq == sw_txhdr)) {
			txq->wait2txq = NULL;
			cls_wifi_amsdu_agg_timer_stop(cls_wifi_hw, txq);
		}
		sw_txhdr->queued = true;
		if (sw_txhdr->desc.api.host.packet_cnt > 1) {
			struct cls_wifi_amsdu_txhdr *amsdu_txhdr, *next;
			list_for_each_entry_safe(amsdu_txhdr, next, &sw_txhdr->amsdu.hdrs, list) {
				cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &amsdu_txhdr->ipc_data);
				dev_kfree_skb_any(amsdu_txhdr->skb);
			}
		}
#endif
		cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &sw_txhdr->ipc_data);
		kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);

		dev_kfree_skb_any(skb);

		return 0;
	}

	if (unlikely(txq->sta && txq->sta->ps.active)) {
		txq->sta->ps.pkt_ready[txq->ps_id]++;
		trace_ps_queue(txq->sta);

		if (txq->sta->ps.pkt_ready[txq->ps_id] == 1) {
			cls_wifi_set_traffic_status(cls_wifi_hw, txq->sta, true, txq->ps_id);
		}
	}

	if (!retry) {
		/* add buffer in the sk_list */
		if (skb_prev)
			skb_append(skb_prev, skb, &txq->sk_list);
		else
			skb_queue_tail(&txq->sk_list, skb);

		cls_wifi_ipc_sta_buffer(cls_wifi_hw, txq->sta, txq->tid,
							((struct cls_wifi_txhdr *)skb->data)->sw_hdr->frame_len);
		cls_wifi_txq_start_cleanup_timer(cls_wifi_hw, txq->sta);
	} else {
		if (txq->last_retry_skb)
			skb_append(txq->last_retry_skb, skb, &txq->sk_list);
		else
			skb_queue_head(&txq->sk_list, skb);

		txq->last_retry_skb = skb;
		txq->nb_retry++;
	}

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (!sw_txhdr->queued)
		sw_txhdr->queued = true;
	else if (!retry) {
		pr_warn("[warn]skb: %px, sw_txhdr: %px, queued: %d != 0, txq(%px)->idx: %d, sta_idx: %d\n",
			skb, sw_txhdr, sw_txhdr->queued, txq, txq->idx, txq->sta->sta_idx);
	}
#endif

	trace_txq_queue_skb(skb, txq, retry);

	/* Flowctrl corresponding netdev queue if needed */
	/* If too many buffer are queued for this TXQ stop netdev queue */
	if ((txq->ndev_idx != NDEV_NO_TXQ) &&
		(skb_queue_len(&txq->sk_list) > txq_ctrl_stop)) {
		txq->status |= CLS_WIFI_TXQ_NDEV_FLOW_CTRL;
		//netif_stop_subqueue(txq->ndev, txq->ndev_idx);
		trace_txq_flowctrl_stop(txq);
	}

	/* add it in the hwq list if not stopped and not yet present */
	if (!cls_wifi_txq_is_stopped(txq)) {
		cls_wifi_txq_add_to_hw_list(txq);
		return 1;
	}

	return 0;
}

/**
 * cls_wifi_txq_confirm_any - Process buffer confirmed by fw
 *
 * @cls_wifi_hw: Driver main data
 * @txq: TX Queue
 * @hwq: HW Queue
 * @sw_txhdr: software descriptor of the confirmed packet
 *
 * Process a buffer returned by the fw. It doesn't check buffer status
 * and only does systematic counter update:
 * - hw credit
 * - buffer pushed to fw
 *
 * To be called with tx_lock hold
 */
void cls_wifi_txq_confirm_any(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq,
						  struct cls_wifi_hwq *hwq, struct cls_wifi_sw_txhdr *sw_txhdr)
{
	if (txq->pkt_pushed)
		txq->pkt_pushed--;

	hwq->credits++;
	txq->hwq_credits_quota++;
	cls_wifi_hw->stats.cfm_balance[hwq->id]--;
	cls_wifi_he_mu_cfm_hwq(cls_wifi_hw, hwq, sw_txhdr);

	if (cls_wifi_hwq_ready(hwq))
		hwq->need_processing = true;
}

/******************************************************************************
 * HWQ processing
 *****************************************************************************/
/**
 * cls_wifi_txq_get_credits - Get the number of pkt (i.e. credits) that can be
 * dequeued from the TXQ
 *
 * @txq: TXQ from which buffer will be dequeue
 * @hwq: HWQ to which buffer will be send
 * @size_avail: Updated, if hwq is an HE-MU queue, with the remaining size to
 * push other packets.
 * @return Maximum number of buffer that should be dequeued from txq
 *
 * This functions first get the number of available credits by checking what is
 * available in the TXQ (possibly limited by push_limit) and what is available
 * in the HWQ.
 * Then, as HWQ used for HE-MU also have a size limit, the list of currently
 * queued buffer is scanned until\n:
 * - it is empty: In this case size_avail is updated with the remaining size
 * in case packet from other source can be dequeue (from mac80211 txq)
 * - the size limit is reached: In this case the credits is reduced so that
 * it correspond to the number of buffer to dequeue to reach the limit and
 * size_avail is set to -1.
 */
static int cls_wifi_txq_get_credits(struct cls_wifi_txq *txq, struct cls_wifi_hwq *hwq,
								int *size_avail)
{
	int cred = txq->credits;
	u16 hwq_credits = cls_wifi_txq_get_hwq_credits(txq);

	if (txq->push_limit && (cred > txq->push_limit)) {
		/* if destination is in PS mode, push_limit indicates the maximum
		   number of packet that can be pushed on this txq. */
		cred = txq->push_limit;
	}
	if (hwq_credits < cred)
		cred = hwq_credits;

#ifdef CONFIG_CLS_WIFI_HEMU_TX
	if (hwq->size_limit) {
		struct sk_buff *skb;
		int size_limit = hwq->size_limit - hwq->size_pushed;
		int i = 1;

		skb_queue_walk(&txq->sk_list, skb) {
			struct cls_wifi_sw_txhdr *sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_hdr;
			size_limit -= sw_txhdr->frame_len;
			if (size_limit <= 0) {
				cred = i;
				size_limit = -1;
				break;
			} else if (i == cred) {
				break;
			}
			i++;
		}
		*size_avail = size_limit;
	}
#endif

	return cred;
}

/**
 * skb_queue_extract - Extract buffer from skb list
 *
 * @list: List of skb to extract from
 * @head: List of skb to append to
 * @nb_elt: Number of skb to extract
 *
 * extract the first @nb_elt of @list and append them to @head
 * It is assumed that:
 * - @list contains more that @nb_elt
 * - There is no need to take @list nor @head lock to modify them
 */
static inline void skb_queue_extract(struct sk_buff_head *list,
									 struct sk_buff_head *head, int nb_elt)
{
	int i;
	struct sk_buff *first, *last, *ptr;

	if (nb_elt <= 0)
		return;

	first = ptr = list->next;
	for (i = 0; i < nb_elt; i++) {
		ptr = ptr->next;
	}
	last = ptr->prev;

	/* unlink nb_elt in list */
	list->qlen -= nb_elt;
	list->next = ptr;
	ptr->prev = (struct sk_buff *)list;

	/* append nb_elt at end of head */
	head->qlen += nb_elt;
	last->next = (struct sk_buff *)head;
	head->prev->next = first;
	first->prev = head->prev;
	head->prev = last;
}


#ifdef CONFIG_MAC80211_TXQ
/**
 * cls_wifi_txq_mac80211_dequeue - Dequeue buffer from mac80211 txq and
 *							 add them to push list
 *
 * @cls_wifi_hw: Main driver data
 * @sk_list: List of buffer to push (initialized without lock)
 * @txq: TXQ to dequeue buffers from
 * @max: Max number of buffer to dequeue
 * @size_limit: If not 0, maximum number of bytes to dequeue (<0 means dequeue nothing).
 * The size limit can only be set on TXQ scheduled for HE-MU transmission.
 *
 * Dequeue buffer from mac80211 txq, prepare them for transmission and chain them
 * to the list of buffer to push.
 *
 * @return true if no more buffer are queued in mac80211 txq and false otherwise.
 */
static bool cls_wifi_txq_mac80211_dequeue(struct cls_wifi_hw *cls_wifi_hw,
									  struct sk_buff_head *sk_list,
									  struct cls_wifi_txq *txq, int max, int size_limit)
{
	struct ieee80211_txq *mac_txq;
	struct sk_buff *skb;
	unsigned long mac_txq_len;

	if (txq->nb_ready_mac80211 == NOT_MAC80211_TXQ)
		return true;

#ifdef CONFIG_CLS_WIFI_HEMU_TX
	if (size_limit < 0)
		return (txq->nb_ready_mac80211 == 0);
#endif

	mac_txq = container_of((void *)txq, struct ieee80211_txq, drv_priv);

	for (; max > 0; max--) {
		skb = cls_wifi_tx_dequeue_prep(cls_wifi_hw, mac_txq);
		if (skb == NULL)
			return true;

		__skb_queue_tail(sk_list, skb);

#ifdef CONFIG_CLS_WIFI_HEMU_TX
		if (size_limit) {
			struct cls_wifi_sw_txhdr *sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_txhdr;
			size_limit -= sw_txhdr->frame_len;
			if (size_limit <= 0)
				break;
		}
#endif
	}

	/* re-read mac80211 txq current length.
	   It is mainly for debug purpose to trace dropped packet. There is no
	   problems to have nb_ready_mac80211 != actual mac80211 txq length */
	ieee80211_txq_get_depth(mac_txq, &mac_txq_len, NULL);
	if (txq->nb_ready_mac80211 > mac_txq_len)
		trace_txq_drop(txq, txq->nb_ready_mac80211 - mac_txq_len);
	txq->nb_ready_mac80211 = mac_txq_len;

	return (txq->nb_ready_mac80211 == 0);
}
#endif

/**
 * cls_wifi_txq_get_skb_to_push() - Get list of buffer to push for one txq
 *
 * @cls_wifi_hw: main driver data
 * @hwq: HWQ on wich buffers will be pushed
 * @txq: TXQ to get buffers from
 * @sk_list_push: list to update
 *
 * This function will returned a list of buffer to push for one txq.
 * It will take into account the number of credit of the HWQ and TXQ,
 * push_limit for station in PS and HWQ size limit for HE-MU HWQ.
 * This allow to get a list that can be pushed without having to test for
 * hwq/txq status after each push
 *
 * @return true if txq no longer have buffer ready after the ones returned.
 *		 false otherwise
 */
static
bool cls_wifi_txq_get_skb_to_push(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_hwq *hwq,
							  struct cls_wifi_txq *txq,
							  struct sk_buff_head *sk_list_push)
{
	int nb_ready = skb_queue_len(&txq->sk_list);
	int size_limit = 0;
	int credits = cls_wifi_txq_get_credits(txq, hwq, &size_limit);
	bool res = false;

	__skb_queue_head_init(sk_list_push);

	if (credits >= nb_ready) {
		skb_queue_splice_init(&txq->sk_list, sk_list_push);
#ifdef CONFIG_MAC80211_TXQ
		res = cls_wifi_txq_mac80211_dequeue(cls_wifi_hw, sk_list_push, txq, credits - nb_ready,
										size_limit);
#else
		res = true;
#endif
	} else {
		skb_queue_extract(&txq->sk_list, sk_list_push, credits);

		/* When processing PS service period (i.e. push_limit != 0), no longer
		   process this txq if the buffers extracted will complete the SP for
		   this txq */
		if (txq->push_limit && (credits == txq->push_limit))
			res = true;
	}

	return res;
}

/**
 * cls_wifi_hwq_process - Process one HW queue list
 *
 * @cls_wifi_hw: Driver main data
 * @hw_queue: HW queue index to process
 *
 * The function will iterate over all the TX queues linked in this HW queue
 * list. For each TX queue, push as many buffers as possible in the HW queue.
 * (NB: TX queue have at least 1 buffer, otherwise it wouldn't be in the list)
 * - If TX queue no longer have buffer, remove it from the list and check next
 *   TX queue
 * - If TX queue no longer have credits or has a push_limit (PS mode) and it
 *   is reached , remove it from the list and check next TX queue
 * - If HW queue is full, update list head to start with the next TX queue on
 *   next call if current TX queue already pushed "too many" pkt in a row, and
 *   return
 *
 * To be called when HW queue list is modified:
 * - when a buffer is pushed on a TX queue
 * - when new credits are received
 * - when a STA returns from Power Save mode or receives traffic request.
 * - when Channel context change
 *
 * To be called with tx_lock hold
 */
void cls_wifi_hwq_process(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_hwq *hwq)
{
	struct cls_wifi_txq *txq, *next;

	trace_process_hw_queue(hwq);

	hwq->need_processing = false;

	list_for_each_entry_safe(txq, next, &hwq->list, sched_list) {
		struct cls_wifi_txhdr *txhdr = NULL;
		struct sk_buff_head sk_list_push;
		struct sk_buff *skb;
		bool txq_empty;

		trace_process_txq(txq);

		/* sanity check for debug */
		BUG_ON(!(txq->status & CLS_WIFI_TXQ_IN_HWQ_LIST));
		BUG_ON(txq->idx == TXQ_INACTIVE);
		BUG_ON(txq->credits <= 0);
		if (!cls_wifi_txq_skb_ready(txq)) {
			pr_warn("%s txq: %px ,txq->credits:%d,txq->idx:0x%x, tid:%d,sta_idx: %d,push_limit:%d,pkt_ready:%d,ps_id:%d\n",
				__func__, txq, txq->credits, txq->idx, txq->tid, txq->sta->sta_idx, txq->push_limit,
				txq->sta->ps.pkt_ready[txq->ps_id], txq->ps_id);
		}
		WARN(!cls_wifi_txq_skb_ready(txq), "txq sk_list is empty!!!");
		if (!cls_wifi_txq_skb_ready(txq))
			cls_wifi_txq_del_from_hw_list(txq);

		if (!cls_wifi_hwq_ready(hwq))
			break;

		if (txq->hwq_credits_quota == 0)
			continue;

		txq_empty = cls_wifi_txq_get_skb_to_push(cls_wifi_hw, hwq, txq, &sk_list_push);

		while ((skb = __skb_dequeue(&sk_list_push)) != NULL) {
			txhdr = (struct cls_wifi_txhdr *)skb->data;
			cls_wifi_tx_push(cls_wifi_hw, txhdr, 0);
		}

		if (txq_empty) {
			cls_wifi_txq_del_from_hw_list(txq);
			txq->pkt_sent = 0;
		} else if ((cls_wifi_txq_get_hwq_credits(txq) == 0) &&
				   cls_wifi_txq_is_scheduled(txq)) {
			/* txq not empty,
			   - To avoid starving need to process other txq in the list
			   - For better aggregation, need to send "as many consecutive
			   pkt as possible" for he same txq
			   ==> Add counter to trigger txq switch
			*/
			if (txq->pkt_sent > (hwq->size / cls_wifi_hwq_credits_multipe(hwq->id))) {
				txq->pkt_sent = 0;
				list_rotate_left(&hwq->list);
			}
		}

		/* Unable to complete PS traffic request because of hwq credit */
		if (txq->push_limit && txq->sta) {
			if (txq->ps_id == LEGACY_PS_ID) {
				/* for legacy PS abort SP and wait next ps-poll */
				txq->sta->ps.sp_cnt[txq->ps_id] -= txq->push_limit;
				txq->push_limit = 0;

				cls_wifi_txq_del_from_hw_list(txq);
				txq->pkt_sent = 0;
			}
			/* for u-apsd need to complete the SP to send EOSP frame */
		}

		/* restart netdev queue if number of queued buffer is below threshold */
		if (unlikely(txq->status & CLS_WIFI_TXQ_NDEV_FLOW_CTRL) &&
			skb_queue_len(&txq->sk_list) < txq_ctrl_start) {
			txq->status &= ~CLS_WIFI_TXQ_NDEV_FLOW_CTRL;
			//netif_wake_subqueue(txq->ndev, txq->ndev_idx);
			trace_txq_flowctrl_restart(txq);
		}
	}
}

/**
 * cls_wifi_hwq_process_all - Process all HW queue list
 *
 * @cls_wifi_hw: Driver main data
 *
 * Loop over all HWQ, and process them if needed
 * To be called with tx_lock hold
 */
void cls_wifi_hwq_process_all(struct cls_wifi_hw *cls_wifi_hw)
{
	int id;

	for (id = ARRAY_SIZE(cls_wifi_hw->hwq) - 1; id >= 0 ; id--) {
		if (cls_wifi_hw->hwq[id].need_processing) {
			cls_wifi_hwq_process(cls_wifi_hw, &cls_wifi_hw->hwq[id]);
		}
	}
}
EXPORT_SYMBOL(cls_wifi_hwq_process_all);

/**
 * cls_wifi_hwq_init - Initialize all hwq structures
 *
 * @cls_wifi_hw: Driver main data
 *
 */
void cls_wifi_hwq_init(struct cls_wifi_hw *cls_wifi_hw)
{
	int i;
	struct cls_wifi_hw_params *hw_params =  &cls_wifi_hw->plat->hw_params;

	memset(cls_wifi_hw->hwq, 0, sizeof(cls_wifi_hw->hwq));

	for (i = 0; i < ARRAY_SIZE(cls_wifi_hw->hwq); i++) {
		struct cls_wifi_hwq *hwq = &cls_wifi_hw->hwq[i];

		hwq->id = i;
		INIT_LIST_HEAD(&hwq->list);

		if (i >= CLS_WIFI_HWQ_USER_BASE)
			// credit/size of MU HWQ are initialized by cls_wifi_hwq_mu_reset
			continue;

		// ACx HWQ
		switch (i) {
		case 0:
			hwq->credits = hw_params->txdesc_cnt0[cls_wifi_hw->radio_idx];
			break;
		case 1:
			hwq->credits = hw_params->txdesc_cnt1[cls_wifi_hw->radio_idx];
			break;
		case 2:
			hwq->credits = hw_params->txdesc_cnt2[cls_wifi_hw->radio_idx];
			break;
		case 3:
			hwq->credits = hw_params->txdesc_cnt3[cls_wifi_hw->radio_idx];
			break;
		case 4:
			hwq->credits = hw_params->txdesc_cnt4[cls_wifi_hw->radio_idx];
			break;
		default:
			continue;
		}
		hwq->size = hwq->credits;
	}
}

#ifdef CONFIG_CLS_WIFI_HEMU_TX
/**
 * cls_wifi_hwq_mu_reset - Reset size/credits of an MU HWQ
 *
 * @cls_wifi_hw: Driver main data
 * @id: MU HWQ id
 * @size: Size, in buffers, allowed for this HWQ
 * @return Pointer to the HWQ that has been initialized.
 */
struct cls_wifi_hwq *cls_wifi_hwq_mu_reset(struct cls_wifi_hw *cls_wifi_hw, unsigned int id,
								   unsigned int size, u32 psdu_len)
{
	struct cls_wifi_hwq *hwq;

	if (id < CLS_WIFI_HWQ_USER_BASE || id >= CLS_WIFI_HWQ_NB)
		return NULL;

	hwq = &cls_wifi_hw->hwq[id];
	if (!cls_wifi_hwq_is_flushed(hwq))
		dev_err(cls_wifi_hw->dev, "Reset MU HWQ %d whereas pkt have not yet been "
				"confirmed (size=%d, credits=%d)\n",
				id - CLS_WIFI_HWQ_USER_BASE, hwq->size, hwq->credits);

	hwq->size = size;
	hwq->credits = size;
	hwq->size_pushed = 0;
	hwq->size_limit = 2 * psdu_len;

	return hwq;
}
#endif // CONFIG_CLS_WIFI_HEMU_TX
