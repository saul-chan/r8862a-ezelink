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

#ifndef _CLS_WIFI_TXQ_H_
#define _CLS_WIFI_TXQ_H_

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/ieee80211.h>

/**
 * Fullmac TXQ configuration:
 *  - STA: 1 TXQ per TID (limited to 8)
 *		 1 TXQ for bufferable MGT frames
 *  - VIF: 1 TXQ for Multi/Broadcast +
 *		 1 TXQ for MGT for unknown STAs or non-bufferable MGT frames
 *  - 1 TXQ for offchannel transmissions
 *
 *
 * Txq mapping looks like
 * for CLS_REMOTE_STA_MAX=10 and CLS_VIRT_DEV_MAX=4
 *
 * | TXQ | NDEV_ID | VIF |   STA |  TID | HWQ |
 * |-----+---------+-----+-------+------+-----|-
 * |   0 |	   0 |	 |	 0 |	0 |   1 | 9 TXQ per STA
 * |   1 |	   1 |	 |	 0 |	1 |   0 | (8 data + 1 mgmt)
 * |   2 |	   2 |	 |	 0 |	2 |   0 |
 * |   3 |	   3 |	 |	 0 |	3 |   1 |
 * |   4 |	   4 |	 |	 0 |	4 |   2 |
 * |   5 |	   5 |	 |	 0 |	5 |   2 |
 * |   6 |	   6 |	 |	 0 |	6 |   3 |
 * |   7 |	   7 |	 |	 0 |	7 |   3 |
 * |   8 |	 N/A |	 |	 0 | MGMT |   3 |
 * |-----+---------+-----+-------+------+-----|-
 * | ... |		 |	 |	   |	  |	 | Same for all STAs
 * |-----+---------+-----+-------+------+-----|-
 * |  90 |	  80 |   0 | BC/MC |	0 | 1/4 | 1 TXQ for BC/MC per VIF
 * | ... |		 |	 |	   |	  |	 |
 * |  93 |	  80 |   3 | BC/MC |	0 | 1/4 |
 * |-----+---------+-----+-------+------+-----|-
 * |  94 |	 N/A |   0 |   N/A | MGMT |   3 | 1 TXQ for unknown STA per VIF
 * | ... |		 |	 |	   |	  |	 |
 * |  97 |	 N/A |   3 |   N/A | MGMT |   3 |
 * |-----+---------+-----+-------+------+-----|-
 * |  98 |	 N/A |	 |   N/A | MGMT |   3 | 1 TXQ for offchannel frame
 */
#define CLS_NB_TID_PER_STA 8
#define CLS_NB_TXQ_PER_STA (CLS_NB_TID_PER_STA + 1)
#define CLS_NB_TXQ_PER_VIF 2
#define CLS_NB_TXQ(vdev_max, sta_max) ((CLS_NB_TXQ_PER_STA * sta_max) +	\
				   (CLS_NB_TXQ_PER_VIF * vdev_max) + 1)


inline uint16_t bcmc_txq_ndev_idx(struct cls_wifi_hw *cls_wifi_hw);



inline uint16_t off_chan_txq_idx(struct cls_wifi_hw *cls_wifi_hw);
#define CLS_BCMC_TXQ_TYPE 0
#define CLS_UNK_TXQ_TYPE  1

/**
 * Each data TXQ is a netdev queue. TXQ to send MGT are not data TXQ as
 * they did not recieved buffer from netdev interface.
 * Need to allocate the maximum case.
 * AP : all STAs + 1 BC/MC
 */
#define CLS_STA_NDEV_IDX(tid, sta_idx) ((tid) + (sta_idx) * CLS_NB_TID_PER_STA)
#define NDEV_NO_TXQ 0xffff

/* stop netdev queue when number of queued buffers if greater than this  */
#define CLS_WIFI_NDEV_FLOW_CTRL_STOP	200
/* restart netdev queue when number of queued buffers is lower than this */
#define CLS_WIFI_NDEV_FLOW_CTRL_RESTART 100

#define TXQ_INACTIVE 0xffff


#define CLS_TXQ_INITIAL_CREDITS 4

#define CLS_WIFI_TXQ_CLEANUP_INTERVAL (10 * HZ) //10s in jiffies
#define CLS_WIFI_TXQ_MAX_QUEUE_JIFFIES (20 * HZ)

/**
 * TXQ tid sorted by decreasing priority
 */
extern const int cls_tid_prio[CLS_NB_TXQ_PER_STA];

/**
 * struct cls_wifi_hwq - Structure used to save information relative to
 *				   an AC TX queue (aka HW queue)
 * @list: List of TXQ, that have buffers ready for this HWQ
 * @credits: available credit for the queue (i.e. nb of buffers that
 *		   can be pushed to FW )
 * @id: Id of the HWQ among CLS_WIFI_HWQ_....
 * @size: size of the queue
 * @need_processing: Indicate if hwq should be processed
 * @size_limit: Maximum number of bytes that should be pushed at once.
 * Only not null for HE-MU queue
 * @size_pushed: Number of bytes currenlty pushed.
 * @len: number of packet ready to be pushed to fw for this HW queue
 * @len_stop: threshold to stop mac80211(i.e. netdev) queues. Stop queue when
 *		   driver has more than @len_stop packets ready.
 * @len_start: threshold to wake mac8011 queues. Wake queue when driver has
 *			less than @len_start packets ready.
 */
struct cls_wifi_hwq {
	struct list_head list;
	u16 credits;
	u16 size;
	u8 id;
	bool need_processing;
#ifdef CONFIG_CLS_WIFI_HEMU_TX
	u32 size_limit;
	u32 size_pushed;
#endif // CONFIG_CLS_WIFI_HEMU_TX
};

/**
 * enum cls_wifi_push_flags - Flags of pushed buffer
 *
 * @CLS_WIFI_PUSH_RETRY Pushing a buffer for retry
 * @CLS_WIFI_PUSH_IMMEDIATE Pushing a buffer without queuing it first
 */
enum cls_wifi_push_flags {
	CLS_WIFI_PUSH_RETRY  = BIT(0),
	CLS_WIFI_PUSH_IMMEDIATE = BIT(1),
};

/**
 * enum cls_wifi_txq_flags - TXQ status flag
 *
 * @CLS_WIFI_TXQ_IN_HWQ_LIST: The queue is scheduled for transmission
 * @CLS_WIFI_TXQ_STOP_FULL: No more credits for the queue
 * @CLS_WIFI_TXQ_STOP_CSA: CSA is in progress
 * @CLS_WIFI_TXQ_STOP_STA_PS: Destiniation sta is currently in power save mode
 * @CLS_WIFI_TXQ_STOP_VIF_PS: Vif owning this queue is currently in power save mode
 * @CLS_WIFI_TXQ_STOP_CHAN: Channel of this queue is not the current active channel
 * @CLS_WIFI_TXQ_STOP_MU: In MU-MIMO TXQ is stopped waiting for all the buffers pushed
 * to fw to be confirmed. In HE-MU TXQ is stopped if the associated station is
 * HE MU capable but this queue is not included in the current MU DL map or MU
 * HWQ is being flushed.
 * @CLS_WIFI_TXQ_STOP_TWT: There is a TWT agreement with the destiation sta and there
 * is currently no active Service Period.
 * @CLS_WIFI_TXQ_STOP: All possible reason to have a txq stopped
 * @CLS_WIFI_TXQ_NDEV_FLOW_CTRL: associated netdev queue is currently stopped.
 *						  Note: when a TXQ is flowctrl it is NOT stopped
 */
enum cls_wifi_txq_flags {
	CLS_WIFI_TXQ_IN_HWQ_LIST  = BIT(0),
	CLS_WIFI_TXQ_STOP_FULL	= BIT(1),
	CLS_WIFI_TXQ_STOP_CSA	 = BIT(2),
	CLS_WIFI_TXQ_STOP_STA_PS  = BIT(3),
	CLS_WIFI_TXQ_STOP_VIF_PS  = BIT(4),
	CLS_WIFI_TXQ_STOP_CHAN	= BIT(5),
	CLS_WIFI_TXQ_STOP_MU	  = BIT(6),
	CLS_WIFI_TXQ_STOP_TWT	 = BIT(7),
	CLS_WIFI_TXQ_STOP		 = (CLS_WIFI_TXQ_STOP_FULL | CLS_WIFI_TXQ_STOP_CSA |
							 CLS_WIFI_TXQ_STOP_STA_PS | CLS_WIFI_TXQ_STOP_VIF_PS |
							 CLS_WIFI_TXQ_STOP_CHAN | CLS_WIFI_TXQ_STOP_MU |
							 CLS_WIFI_TXQ_STOP_TWT),
	CLS_WIFI_TXQ_NDEV_FLOW_CTRL = BIT(8),
};

#define CLS_WIFI_TXQ_MAX_QUEUE_LEN	8192
/**
 * struct cls_wifi_txq - Structure used to save information relative to
 *				   a RA/TID TX queue
 *
 * @idx: Unique txq idx. Set to TXQ_INACTIVE if txq is not used.
 * @status: bitfield of @cls_wifi_txq_flags.
 * @init_credits: initial available credit for the queue (i.e. nb of buffers that
 * @credits: available credit for the queue (i.e. nb of buffers that
 *		   can be pushed to FW).
 * @pkt_sent: number of consecutive pkt sent without leaving HW queue list
 * @pkt_pushed: number of pkt currently pending for transmission confirmation
 * @pkt_fc_drop: number of pkt dropped by CLS_WIFI_NDEV_FLOW_CTRL_STOP
 * @sched_list: list node for HW queue schedule list (cls_wifi_hwq.list)
 * @sk_list: list of buffers to push to fw
 * @last_retry_skb: pointer on the last skb in @sk_list that is a retry.
 *				  (retry skb are stored at the beginning of the list)
 *				  NULL if no retry skb is queued in @sk_list
 * @nb_retry: Number of retry packet queued.
 * @hwq: Pointer on the associated HW queue.
 * @push_limit: number of packet to push before removing the txq from hwq list.
 *			  (we always have push_limit < skb_queue_len(sk_list))
 * @tid: TID
 *
 * SOFTMAC specific:
 * @baw: Block Ack window information
 * @amsdu_anchor: pointer to cls_wifi_sw_txhdr of the first subframe of the A-MSDU.
 *				NULL if no A-MSDU frame is in construction
 * @amsdu_ht_len_cap:
 * @amsdu_vht_len_cap:
 * @nb_ready_mac80211: Number of buffer ready in mac80211 txq
 *
 * FULLMAC specific
 * @ps_id: Index to use for Power save mode (LEGACY or UAPSD)
 * @ndev_idx: txq idx from netdev point of view (0xFF for non netdev queue)
 * @ndev: pointer to ndev of the corresponding vif
 * @amsdu: pointer to cls_wifi_sw_txhdr of the first subframe of the A-MSDU.
 *		 NULL if no A-MSDU frame is in construction
 * @amsdu_len: Maximum size allowed for an A-MSDU. 0 means A-MSDU not allowed
 */
struct cls_wifi_txq {
	u16 idx;
	u16 status;
	s16 init_credits;
	s16 credits;
	u16 hwq_credits_quota;
	u16 pkt_sent;
	u16 pkt_pushed;
	u16 pkt_fc_drop;
	u16 pkt_vip_drop;
	struct list_head sched_list;
	struct sk_buff_head sk_list;
	struct sk_buff *last_retry_skb;
	struct cls_wifi_hwq *hwq;
	int nb_retry;
	u16 push_limit;
	u8 tid;
#ifdef CONFIG_MAC80211_TXQ
	unsigned long nb_ready_mac80211;
#endif
	struct cls_wifi_sta *sta;
	u8 ps_id;
	u16 ndev_idx;
	struct net_device *ndev;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	struct cls_wifi_sw_txhdr *amsdu;
	struct cls_wifi_sw_txhdr *wait2txq;
	u16 amsdu_len;
	struct hrtimer amsdu_agg_timer;
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */
};

struct cls_wifi_sta;
struct cls_wifi_vif;
struct cls_wifi_hw;
struct cls_wifi_sw_txhdr;

static inline bool cls_wifi_txq_is_stopped(struct cls_wifi_txq *txq)
{
	return (txq->status & CLS_WIFI_TXQ_STOP);
}

static inline bool cls_wifi_txq_is_full(struct cls_wifi_txq *txq)
{
	return (txq->status & CLS_WIFI_TXQ_STOP_FULL);
}

static inline bool cls_wifi_txq_is_ps(struct cls_wifi_txq *txq)
{
	return (txq->status & CLS_WIFI_TXQ_STOP_STA_PS);
}

static inline bool cls_wifi_txq_is_scheduled(struct cls_wifi_txq *txq)
{
	return (txq->status & CLS_WIFI_TXQ_IN_HWQ_LIST);
}

static inline u16 cls_wifi_txq_get_hwq_credits(struct cls_wifi_txq *txq)
{
	return min_t(u16, txq->hwq_credits_quota, txq->hwq->credits);
}

/**
 * cls_wifi_txq_is_ready_for_push - Check if a TXQ is ready for push
 *
 * @txq: txq pointer
 *
 * if
 * - txq is not stopped (or has a push limit)
 * - and hwq has credits
 * - and there is no buffer queued
 * then a buffer can be immediately pushed without having to queue it first
 *
 * Note: txq push limit is incremented as soon as a PS buffer is pushed by
 * mac80211.
 * @return: true if the 3 conditions are met and false otherwise.
 */
static inline bool cls_wifi_txq_is_ready_for_push(struct cls_wifi_txq *txq)
{
	return ((!cls_wifi_txq_is_stopped(txq) || txq->push_limit) &&
			(cls_wifi_txq_get_hwq_credits(txq) > 0) &&
			skb_queue_empty(&txq->sk_list));
}

/**
 * cls_wifi_hwq_is_flushed - Check whether there are pending buffer pending
 * in firmware for a given HWQ
 *
 * @hwq: HWQ to check
 */
static inline bool cls_wifi_hwq_is_flushed(struct cls_wifi_hwq *hwq)
{
	return (hwq->size == hwq->credits);
}

/**
 * foreach_sta_txq - Macro to iterate over all TXQ of a STA in increasing
 *				   TID order
 *
 * @sta: pointer to cls_wifi_sta
 * @txq: pointer to cls_wifi_txq updated with the next TXQ at each iteration
 * @tid: int updated with the TXQ tid at each iteration
 * @cls_wifi_hw: main driver data
 */
#ifdef CONFIG_MAC80211_TXQ
#define foreach_sta_txq(sta, txq, tid, cls_wifi_hw)						 \
	for (tid = 0, txq = cls_wifi_txq_sta_get(sta, 0);					   \
		 tid < CLS_NB_TXQ_PER_STA;									   \
		 tid++, txq = cls_wifi_txq_sta_get(sta, tid))
#else
#define foreach_sta_txq(sta, txq, tid, cls_wifi_hw)						  \
	for (tid = 0, txq = cls_wifi_txq_sta_get(sta, 0, cls_wifi_hw);			   \
		 tid < (is_multicast_sta(cls_wifi_hw, sta->sta_idx) ? 1 : CLS_NB_TXQ_PER_STA); \
		 tid++, txq++)

#endif

/**
 * foreach_sta_txq_prio - Macro to iterate over all TXQ of a STA in
 *						decreasing priority order
 *
 * @sta: pointer to cls_wifi_sta
 * @txq: pointer to cls_wifi_txq updated with the next TXQ at each iteration
 * @tid: int updated with the TXQ tid at each iteration
 * @i: int updated with ieration count
 * @cls_wifi_hw: main driver data
 *
 * Note: For fullmac txq for mgmt frame is skipped
 */
#define foreach_sta_txq_prio(sta, txq, tid, i, cls_wifi_hw)						  \
	for (i = 0, tid = cls_tid_prio[0], txq = cls_wifi_txq_sta_get(sta, tid, cls_wifi_hw); \
		 i < CLS_NB_TXQ_PER_STA;												  \
		 i++, tid = cls_tid_prio[i % CLS_NB_TXQ_PER_STA], txq = cls_wifi_txq_sta_get(sta, tid, cls_wifi_hw))

/**
 * foreach_vif_txq - Macro to iterate over all TXQ of a VIF (in AC order)
 *
 * @vif: pointer to cls_wifi_vif
 * @txq: pointer to cls_wifi_txq updated with the next TXQ at each iteration
 * @ac:  int updated with the TXQ ac at each iteration
 */
#ifdef CONFIG_MAC80211_TXQ
#define foreach_vif_txq(vif, txq, ac)								   \
	for (ac = CLS_WIFI_HWQ_BK, txq = cls_wifi_txq_vif_get(vif, ac);			 \
		 ac < CLS_NB_TXQ_PER_VIF;										\
		 ac++, txq = cls_wifi_txq_vif_get(vif, ac))

#else
#define foreach_vif_txq(vif, txq, ac)								   \
	for (ac = CLS_WIFI_HWQ_BK, txq = &vif->txqs[0];						 \
		 ac < CLS_NB_TXQ_PER_VIF;										\
		 ac++, txq++)
#endif

struct cls_wifi_txq *cls_wifi_txq_sta_get(struct cls_wifi_sta *sta, u8 tid,
								  struct cls_wifi_hw * cls_wifi_hw);
struct cls_wifi_txq *cls_wifi_txq_vif_get(struct cls_wifi_vif *vif, u8 type);

/**
 * cls_wifi_txq_vif_get_status - return status bits related to the vif
 *
 * @cls_wifi_vif: Pointer to vif structure
 */
static inline u8 cls_wifi_txq_vif_get_status(struct cls_wifi_vif *cls_wifi_vif)
{
	struct cls_wifi_txq *txq = cls_wifi_txq_vif_get(cls_wifi_vif, 0);
	return (txq->status & (CLS_WIFI_TXQ_STOP_CHAN | CLS_WIFI_TXQ_STOP_VIF_PS));
}

void cls_wifi_txq_vif_init(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_vif *vif,
					   u8 status);
void cls_wifi_txq_vif_deinit(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_vif *vif);
void cls_wifi_txq_sta_init(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
					   u8 status);
void cls_wifi_txq_sta_deinit(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta);
void cls_wifi_txq_unk_vif_init(struct cls_wifi_vif *cls_wifi_vif);
void cls_wifi_txq_unk_vif_deinit(struct cls_wifi_vif *vif);
void cls_wifi_txq_offchan_init(struct cls_wifi_vif *cls_wifi_vif);
void cls_wifi_txq_offchan_deinit(struct cls_wifi_vif *cls_wifi_vif);
void cls_wifi_txq_tdls_vif_init(struct cls_wifi_vif *cls_wifi_vif);
void cls_wifi_txq_tdls_vif_deinit(struct cls_wifi_vif *vif);
void cls_wifi_txq_tdls_sta_start(struct cls_wifi_vif *cls_wifi_vif, u16 reason,
							 struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_tdls_sta_stop(struct cls_wifi_vif *cls_wifi_vif, u16 reason,
							struct cls_wifi_hw *cls_wifi_hw);

int cls_wifi_txq_prepare(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_statable_prepare(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_rxbuf_prepare(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_statable_free(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_rxbuf_free(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_free(struct cls_wifi_hw *cls_wifi_hw);


void cls_wifi_txq_add_to_hw_list(struct cls_wifi_txq *txq);
void cls_wifi_txq_del_from_hw_list(struct cls_wifi_txq *txq);
void cls_wifi_txq_stop(struct cls_wifi_txq *txq, u16 reason);
void cls_wifi_txq_start(struct cls_wifi_txq *txq, u16 reason);
void cls_wifi_txq_vif_start(struct cls_wifi_vif *vif, u16 reason,
						struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_vif_stop(struct cls_wifi_vif *vif, u16 reason,
					   struct cls_wifi_hw *cls_wifi_hw);

void cls_wifi_txq_sta_start(struct cls_wifi_sta *sta, u16 reason,
						struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_sta_stop(struct cls_wifi_sta *sta, u16 reason,
					   struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_sta_start_he_mu(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
							 struct cls_wifi_hwq *mu_hwq, uint16_t tid_map);
void cls_wifi_txq_sta_stop_he_mu(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta);
void cls_wifi_txq_sta_disable_he_mu(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta);

void cls_wifi_txq_offchan_start(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_sta_switch_vif(struct cls_wifi_sta *sta, struct cls_wifi_vif *old_vif,
							 struct cls_wifi_vif *new_vif);

int cls_wifi_txq_queue_skb(struct sk_buff *skb, struct cls_wifi_txq *txq,
					   struct cls_wifi_hw *cls_wifi_hw,  bool retry,
					   struct sk_buff *skb_prev);
void cls_wifi_txq_confirm_any(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq,
						  struct cls_wifi_hwq *hwq, struct cls_wifi_sw_txhdr *sw_txhdr);
void cls_wifi_txq_drop_skb(struct cls_wifi_txq *txq,  struct sk_buff *skb,
					   struct cls_wifi_hw *cls_wifi_hw, bool retry_packet);

void cls_wifi_hwq_init(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_hwq_process(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_hwq *hwq);
void cls_wifi_hwq_process_all(struct cls_wifi_hw *cls_wifi_hw);
struct cls_wifi_hwq *cls_wifi_hwq_mu_reset(struct cls_wifi_hw *cls_wifi_hw, unsigned int id,
								   unsigned int size, u32 psdu_len);
struct cls_wifi_hwq *cls_wifi_hwq_mu_sta_get(struct cls_wifi_hw * cls_wifi_hw, struct cls_wifi_sta *sta);

#endif /* _CLS_WIFI_TXQ_H_ */
