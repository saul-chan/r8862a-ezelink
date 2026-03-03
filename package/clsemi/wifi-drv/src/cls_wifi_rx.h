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

#ifndef _CLS_WIFI_RX_H_
#define _CLS_WIFI_RX_H_

#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include "hal_desc.h"
#include "ipc_shared.h"
#include "cls_wifi_core.h"

enum rx_status_bits
{
	/// The buffer can be forwarded to the networking stack
	RX_STAT_FORWARD = 1 << 0,
	/// A new buffer has to be allocated
	RX_STAT_ALLOC = 1 << 1,
	/// The buffer has to be deleted
	RX_STAT_DELETE = 1 << 2,
	/// The length of the buffer has to be updated
	RX_STAT_LEN_UPDATE = 1 << 3,
	/// The length in the Ethernet header has to be updated
	RX_STAT_ETH_LEN_UPDATE = 1 << 4,
	/// Simple copy
	RX_STAT_COPY = 1 << 5,
	/// Spurious frame (inform upper layer and discard)
	RX_STAT_SPURIOUS = 1 << 6,
	/// packet for monitor interface
	RX_STAT_MONITOR = 1 << 7,
	/// packet for scan ext
	RX_STAT_SCANEXT = 1 << 8,
};

/* Maximum number of rx buffer the fw may use at the same time
   (must be at least IPC_RXBUF_CNT) */
#if defined(DUBHE2000)
#define CLS_WIFI_RXBUFF_MAX_2G4_CMN   (64 * CLS_WIFI_DUBHE2000_STA_MAX_0)
#define CLS_WIFI_RXBUFF_MAX_5G_CMN    (64 * CLS_WIFI_DUBHE2000_STA_MAX_1)
#elif defined(MERAK2000)
#define CLS_WIFI_RXBUFF_MAX_2G4_CMN   (64 * CLS_WIFI_MERAK2000_STA_MAX_0)
#define CLS_WIFI_RXBUFF_MAX_5G_CMN    (64 * CLS_WIFI_MERAK2000_STA_MAX_1)
#endif

/**
 * struct cls_wifi_skb_cb - Control Buffer structure for RX buffer
 *
 * @hostid: Buffer identifier. Written back by fw in RX descriptor to identify
 * the associated rx buffer
 */
struct cls_wifi_skb_cb {
	uint32_t hostid;
};

#define CLS_WIFI_RXBUFF_HOSTID_SET(buf, val)								\
	((struct cls_wifi_skb_cb *)((struct sk_buff *)buf->addr)->cb)->hostid = val

#define CLS_WIFI_RXBUFF_HOSTID_GET(buf)										\
	((struct cls_wifi_skb_cb *)((struct sk_buff *)buf->addr)->cb)->hostid

/* Used to ensure that hostid set to fw is never 0 */
#define CLS_WIFI_RXBUFF_IDX_TO_HOSTID(idx) ((idx) + 1)
#define CLS_WIFI_RXBUFF_HOSTID_TO_IDX(hostid) ((hostid) - 1)

#define RX_MACHDR_BACKUP_LEN	64

/// MAC header backup descriptor
struct mon_machdrdesc
{
	/// Length of the buffer
	u32 buf_len;
	/// Buffer containing mac header, LLC and SNAP
	u8 buffer[RX_MACHDR_BACKUP_LEN];
};

/// UMAC specific flags set for host layer when uploading a frame.
enum rx_flags_bf
{
	/// Whether the frame is an AMSDU
	BF_FIELD(RX_FLAGS, AMSDU, 0, 1),
	/// Whether the frame format is 802.11 (i.e. not 802.3)
	BF_FIELD(RX_FLAGS, 80211_MPDU, 1, 1),
	/// Whether the frame has been sent using 4 addresses mode
	BF_FIELD(RX_FLAGS, 4_ADDR, 2, 1),
	/// Whether the frame has been sent by a new MESH peer
	BF_FIELD(RX_FLAGS, NEW_MESH_PEER, 3, 1),
	/// TID used to send the frame
	BF_FIELD(RX_FLAGS, USER_PRIO, 4, 3),
	/// VIF index for which the frame was send to. INVALID_VIF_IDX if frame is broadcast.
	BF_FIELD(RX_FLAGS, VIF_IDX, 8, 4),
	/// STA index that send the frame. INVALID_STA_IDX if sending STA is unknown
	BF_FIELD(RX_FLAGS, STA_IDX, 12, 10),
	/// STA index for which the frame is destinated.
	/// Set for data frame received by AP/MESH interface to the index of the final
	/// destination STA if part of the (MESH-)BSS and to INVALID_STA_IDX otherwise.\n
	/// Set to 0 in all other cases (i.e. mgmt frames or other type of interface)
	BF_FIELD(RX_FLAGS, DST_STA_IDX, 22, 10),
};
/// Wrapper around @ref BF_GET for @ref rx_flags_bf bitfield
#define RX_FLAGS_GET(field, bf_val) BF_GET(RX_FLAGS, field, bf_val)
/// Wrapper around @ref BF_VAL for @ref rx_flags_bf bitfield
#define RX_FLAGS_VAL(field, field_val) BF_VAL(RX_FLAGS, field, field_val)

struct hw_rxhdr {
	/** RX vector */
	struct hw_vect hwvect;

	/** PHY channel information */
	struct phy_channel_info_desc phy_info;

	/** RX flags defined by rx_flags_bf */
	u32 flags;

	u32 amsdu_hostids[CLS_MAX_MSDU_PER_RX_AMSDU - 1];
	u16 amsdu_len[CLS_MAX_MSDU_PER_RX_AMSDU];
#ifdef CONFIG_CLS_WIFI_MON_DATA
	/// MAC header backup descriptor (used only for MSDU when there is a monitor and a data interface)
	struct mon_machdrdesc mac_hdr_backup;
#endif
	/** Pattern indicating if the buffer is available for the driver */
	u32 pattern;
	/** TSF Low */
	__le32 tsf_lo;
	/** TSF High */
	__le32 tsf_hi;
	/** Receive Vector 2 */
	struct rx_vector_2 rx_vect2;
};

/**
 * struct cls_wifi_defer_rx - Defer rx buffer processing
 *
 * @skb: List of deferred buffers
 * @work: work to defer processing of this buffer
 */
struct cls_wifi_defer_rx {
	struct sk_buff_head sk_list;
	struct work_struct work;
};

/**
 * struct cls_wifi_defer_rx_cb - Control buffer for deferred buffers
 *
 * @vif: VIF that received the buffer
 */
struct cls_wifi_defer_rx_cb {
	struct cls_wifi_vif *vif;
};

u8 cls_wifi_unsup_rx_vec_ind(void *pthis, void *hostid);
u8 cls_wifi_rxdataind(void *pthis, void *hostid, u8 rx_from_cmn);
void cls_wifi_rx_deferred(struct work_struct *ws);
void cls_wifi_rx_defer_skb(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
					   struct sk_buff *skb);

#endif /* _CLS_WIFI_RX_H_ */
