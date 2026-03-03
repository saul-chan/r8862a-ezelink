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

#ifndef _CLS_WIFI_TX_H_
#define _CLS_WIFI_TX_H_

#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <linux/netdevice.h>
#include "lmac_types.h"
#include "ipc_shared.h"
#include "cls_wifi_txq.h"
#include "hal_desc.h"
#include "cls_wifi_utils.h"


///optimization for ddr data
#define ETH_OPTI_DDR_CFG   1

///optimization for txcfm spin lock
#define WIFI_DRV_OPTI_SPIN_LOCK_CFG      1


#define CLS_WIFI_HWQ_BK		0
#define CLS_WIFI_HWQ_BE		1
#define CLS_WIFI_HWQ_VI		2
#define CLS_WIFI_HWQ_VO		3
#define CLS_WIFI_HWQ_BCMC	  4
#define CLS_WIFI_HWQ_USER_BASE 5
#define CLS_WIFI_HWQ_USER(x)   (CLS_WIFI_HWQ_USER_BASE + x)
#ifdef CONFIG_CLS_WIFI_HEMU_TX
#define CLS_WIFI_HWQ_NB		CLS_TXQ_CNT + 16
#else
#define CLS_WIFI_HWQ_NB		CLS_TXQ_CNT
#endif

#define CLS_WIFI_HWQ_ALL_ACS (CLS_WIFI_HWQ_BK | CLS_WIFI_HWQ_BE | CLS_WIFI_HWQ_VI | CLS_WIFI_HWQ_VO)
#define CLS_WIFI_HWQ_ALL_ACS_BIT ( BIT(CLS_WIFI_HWQ_BK) | BIT(CLS_WIFI_HWQ_BE) |	\
							   BIT(CLS_WIFI_HWQ_VI) | BIT(CLS_WIFI_HWQ_VO) )

#define CLS_WIFI_TX_LIFETIME_MS  100
#define CLS_WIFI_TX_MAX_RATES	CLS_TX_MAX_RATES

extern const int cls_wifi_tid2hwq[IEEE80211_NUM_TIDS];

/**
 * struct cls_wifi_amsdu_txhdr - Structure added in skb headroom (instead of
 * cls_wifi_txhdr) for amsdu subframe buffer (except for the first subframe
 * that has a normal cls_wifi_txhdr)
 *
 * @list: List of other amsdu subframe (cls_wifi_sw_txhdr.amsdu.hdrs)
 * @ipc_data: IPC buffer for the A-MSDU subframe
 * @skb: skb
 * @pad: padding added before this subframe
 * (only use when amsdu must be dismantled)
 */
struct cls_wifi_amsdu_txhdr {
	struct list_head list;
	struct cls_wifi_ipc_buf ipc_data;
	struct sk_buff *skb;
	u16 pad;
};

/**
 * struct cls_wifi_amsdu - Structure to manage creation of an A-MSDU, updated
 * only In the first subframe of an A-MSDU
 *
 * @hdrs: List of subframe of cls_wifi_amsdu_txhdr
 * @nb: Number of subframe in the amsdu, 0 means that no amsdu is in progress
 * @pad: Padding to add before adding a new subframe
 * @is_8023: Flag indicating whether the first subframe is of type 802.3 or not
 */
struct cls_wifi_amsdu {
	struct list_head hdrs;
	u8 nb;
	u8 pad;
	bool is_8023;
};

/**
 * struct cls_wifi_sw_txhdr - Software part of tx header
 *
 * @cls_wifi_sta: sta to which this buffer is addressed
 * @cls_wifi_vif: vif that send the buffer
 * @txq: pointer to TXQ used to send the buffer
 * @hw_queue: Index of the HWQ used to push the buffer.
 *		   May be different than txq->hwq->id on confirmation.
 * @frame_len: Size, in bytes, of the frame pushed to firmware.
 * For MSDU it includes the size of the Ethernet header and data.
 * For A-MSDU it includes all subframes (with A-MSDU headers, padding and data).
 * For MGMT frame it includes the 802.11 MAC header and data.
 * @amsdu: Description of amsdu whose first subframe is this buffer
 *		(amsdu.nb = 0 means this buffer is not part of amsdu)
 * @skb: skb received from transmission
 * @ipc_data: IPC buffer for the frame
 * @ipc_desc: IPC buffer for the TX descriptor
 * @jiffies: Jiffies when this buffer has been pushed to the driver
 * @desc: TX descriptor downloaded by firmware
 */
struct cls_wifi_sw_txhdr {
	struct cls_wifi_sta *cls_wifi_sta;
	struct cls_wifi_vif *cls_wifi_vif;
	struct cls_wifi_txq *txq;
	u8 hw_queue;
	u16 frame_len;
#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	struct cls_wifi_amsdu amsdu;
#endif
	struct sk_buff *skb;
	struct cls_wifi_ipc_buf ipc_data;
	struct cls_wifi_ipc_buf ipc_desc;
	unsigned long jiffies;
	struct txdesc_host desc;
	bool queued;
};

/**
 * struct cls_wifi_txhdr - Structure to control transmission of packet
 * (Added in skb headroom)
 *
 * @sw_hdr: Information from driver
 */
struct cls_wifi_txhdr {
	struct cls_wifi_sw_txhdr *sw_hdr;
};

/**
 * CLS_WIFI_TX_HEADROOM - Headroom to use to store struct cls_wifi_txhdr
 */
#define CLS_WIFI_TX_HEADROOM sizeof(struct cls_wifi_txhdr)

/**
 * CLS_WIFI_TX_AMSDU_HEADROOM - Maximum headroom need for an A-MSDU sub frame
 * Need to store struct cls_wifi_amsdu_txhdr, A-MSDU header (14)
 * optional padding (4) and LLC/SNAP header (8)
 */
#if CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD == 0
#define CLS_WIFI_TX_AMSDU_HEADROOM (sizeof(struct cls_wifi_amsdu_txhdr) + 14 + 4 + 8)
#elif CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD == 1
#define CLS_WIFI_TX_AMSDU_HEADROOM (sizeof(struct cls_wifi_amsdu_txhdr))
#else
#define CLS_WIFI_TX_AMSDU_HEADROOM 0
#endif

/**
 * CLS_WIFI_TX_MAX_HEADROOM - Maximum size needed in skb headroom to prepare a buffer
 * for transmission
 */
#define CLS_WIFI_TX_MAX_HEADROOM max(CLS_WIFI_TX_HEADROOM, CLS_WIFI_TX_AMSDU_HEADROOM)

/**
 * CLS_WIFI_TX_DMA_MAP_LEN - Length, in bytes, to map for DMA transfer
 * To be called with skb BEFORE reserving headroom to store struct cls_wifi_txhdr.
 */
#define CLS_WIFI_TX_DMA_MAP_LEN(skb) (skb->len)

/**
 * SKB buffer format before it is pushed to MACSW
 *
 * For DATA frame
 *					|--------------------|
 *					| headroom		   |
 *	skb->data ----> |--------------------|
 *					| struct cls_wifi_txhdr  |
 *					| * cls_wifi_sw_txhdr	|
 *			   +--> |--------------------| <---- desc.host.packet_addr[0]
 *			   :	| 802.3 Header	   |
 *			   :	|--------------------|
 *	 memory	:	| Data			   |
 *	 mapped	:	|					|
 *	 for DMA   :	|					|
 *			   :	|					|
 *			   +--> |--------------------|
 *					| tailroom		   |
 *					|--------------------|
 *
 *
 * For MGMT frame (skb is created by the driver so buffer is always aligned
 *				 with no headroom/tailroom)
 *
 *	skb->data ----> |--------------------|
 *					| struct cls_wifi_txhdr  |
 *					| * cls_wifi_sw_txhdr	|
 *					|					|
 *			   +--> |--------------------| <---- desc.host.packet_addr[0]
 *	 memory	:	| 802.11 HDR		 |
 *	 mapped	:	|--------------------|
 *	 for DMA   :	| Data			   |
 *			   :	|					|
 *			   +--> |--------------------|
 *
 */

u16 cls_wifi_select_txq(struct cls_wifi_vif *cls_wifi_vif, struct sk_buff *skb);
int cls_wifi_start_xmit(struct sk_buff *skb, struct net_device *dev);
int cls_wifi_start_mgmt_xmit(struct cls_wifi_vif *vif, struct cls_wifi_sta *sta,
						 struct cfg80211_mgmt_tx_params *params, bool offchan,
						 u64 *cookie);
int cls_wifi_txdatacfm(void *pthis, void *host_id);

struct cls_wifi_hw;
struct cls_wifi_sta;
void cls_wifi_set_traffic_status(struct cls_wifi_hw *cls_wifi_hw,
							 struct cls_wifi_sta *sta,
							 bool available,
							 u8 ps_id);
void cls_wifi_ps_bh_enable(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
					   bool enable);
void cls_wifi_ps_bh_traffic_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
							u16 pkt_req, u8 ps_id);

void cls_wifi_switch_vif_sta_txq(struct cls_wifi_sta *sta, struct cls_wifi_vif *old_vif,
							 struct cls_wifi_vif *new_vif);

int cls_wifi_dbgfs_print_sta(char *buf, size_t size, struct cls_wifi_sta *sta,
						 struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_txq_credit_update(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx, u8 tid,
							s16 update);
void cls_wifi_tx_push(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txhdr *txhdr, int flags);

void cls_wifi_tx_work_handler(struct work_struct *work);

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
void cls_wifi_amsdu_agg_timer_stop(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_txq *txq);
enum hrtimer_restart cls_wifi_amsdu_agg_timeout(struct hrtimer *timer);
#endif
int cls_wifi_dpd_wmac_tx_handler(struct cls_wifi_hw *wifi_hw, struct mm_dpd_wmac_tx_params_req *req);
int cls_wifi_dpd_wmac_tx_post_process(struct cls_wifi_hw *wifi_hw,
		struct mm_dpd_wmac_tx_ind *ind);

#endif /* _CLS_WIFI_TX_H_ */
