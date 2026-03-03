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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM cls_wifi

#if !defined(_CLS_WIFI_EVENTS_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _CLS_WIFI_EVENTS_H_

#include <linux/tracepoint.h>
#include "cls_wifi_tx.h"
#include "cls_wifi_he_mu.h"
#include "cls_wifi_compat.h"

/*****************************************************************************
 * TRACE function for MGMT TX (FULLMAC)
 ****************************************************************************/
#include "linux/ieee80211.h"
#if defined(CONFIG_TRACEPOINTS) && defined(CREATE_TRACE_POINTS)
#include <linux/trace_seq.h>

/* P2P Public Action Frames Definitions (see WiFi P2P Technical Specification, section 4.2.8) */
/* IEEE 802.11 Public Action Usage Category - Define P2P public action frames */
#define MGMT_ACTION_PUBLIC_CAT			  (0x04)
/* Offset of OUI Subtype field in Vendor Public Action Frame format */
#define MGMT_ACTION_OUI_SUBTYPE_OFFSET (5)
/* P2P Public Action Frame Types */
enum p2p_action_type {
	P2P_ACTION_GO_NEG_REQ   = 0,	/* GO Negociation Request */
	P2P_ACTION_GO_NEG_RSP,		  /* GO Negociation Response */
	P2P_ACTION_GO_NEG_CFM,		  /* GO Negociation Confirmation */
	P2P_ACTION_INVIT_REQ,		   /* P2P Invitation Request */
	P2P_ACTION_INVIT_RSP,		   /* P2P Invitation Response */
	P2P_ACTION_DEV_DISC_REQ,		/* Device Discoverability Request */
	P2P_ACTION_DEV_DISC_RSP,		/* Device Discoverability Response */
	P2P_ACTION_PROV_DISC_REQ,	   /* Provision Discovery Request */
	P2P_ACTION_PROV_DISC_RSP,	   /* Provision Discovery Response */
};

const char *ftrace_print_mgmt_info(struct trace_seq *p, u16 frame_control, u8 cat, u8 type, u8 vendor) {
	const char *ret = trace_seq_buffer_ptr(p);

	switch (frame_control & IEEE80211_FCTL_STYPE) {
		case (IEEE80211_STYPE_ASSOC_REQ): trace_seq_printf(p, "Association Request"); break;
		case (IEEE80211_STYPE_ASSOC_RESP): trace_seq_printf(p, "Association Response"); break;
		case (IEEE80211_STYPE_REASSOC_REQ): trace_seq_printf(p, "Reassociation Request"); break;
		case (IEEE80211_STYPE_REASSOC_RESP): trace_seq_printf(p, "Reassociation Response"); break;
		case (IEEE80211_STYPE_PROBE_REQ): trace_seq_printf(p, "Probe Request"); break;
		case (IEEE80211_STYPE_PROBE_RESP): trace_seq_printf(p, "Probe Response"); break;
		case (IEEE80211_STYPE_BEACON): trace_seq_printf(p, "Beacon"); break;
		case (IEEE80211_STYPE_ATIM): trace_seq_printf(p, "ATIM"); break;
		case (IEEE80211_STYPE_DISASSOC): trace_seq_printf(p, "Disassociation"); break;
		case (IEEE80211_STYPE_AUTH): trace_seq_printf(p, "Authentication"); break;
		case (IEEE80211_STYPE_DEAUTH): trace_seq_printf(p, "Deauthentication"); break;
		case (IEEE80211_STYPE_ACTION):
			trace_seq_printf(p, "Action");
			if (vendor)
				// vendor != 0 means the frame is a Public Action frame, with type set to 'Vendor Specific'
				// In this case 'vendor' is the OUI subtype (assuming OUI==WFA),
				// 'cat' is the 1 first after OUI and 'type' the second byte
				if (vendor == 0x09) {
					switch (cat) {
						case (P2P_ACTION_GO_NEG_REQ): trace_seq_printf(p, ": GO Negociation Request"); break;
						case (P2P_ACTION_GO_NEG_RSP): trace_seq_printf(p, ": GO Negociation Response"); break;
						case (P2P_ACTION_GO_NEG_CFM): trace_seq_printf(p, ": GO Negociation Confirmation"); break;
						case (P2P_ACTION_INVIT_REQ): trace_seq_printf(p, ": P2P Invitation Request"); break;
						case (P2P_ACTION_INVIT_RSP): trace_seq_printf(p, ": P2P Invitation Response"); break;
						case (P2P_ACTION_DEV_DISC_REQ): trace_seq_printf(p, ": Device Discoverability Request"); break;
						case (P2P_ACTION_DEV_DISC_RSP): trace_seq_printf(p, ": Device Discoverability Response"); break;
						case (P2P_ACTION_PROV_DISC_REQ): trace_seq_printf(p, ": Provision Discovery Request"); break;
						case (P2P_ACTION_PROV_DISC_RSP): trace_seq_printf(p, ": Provision Discovery Response"); break;
						default: trace_seq_printf(p, "Unknown p2p %d", type); break;
					}
				} else if (vendor == 0x1a) {
					trace_seq_printf(p, ": DPP %d", type); break;
				} else {
					trace_seq_printf(p, "Unknown vendor Public action  0x%x:%d", vendor, type); break;
				}
			else {
				switch (cat) {
					case 0: trace_seq_printf(p, ":Spectrum %d", type); break;
					case 1: trace_seq_printf(p, ":QOS %d", type); break;
					case 2: trace_seq_printf(p, ":DLS %d", type); break;
					case 3: trace_seq_printf(p, ":BA %d", type); break;
					case 4: trace_seq_printf(p, ":Public %d", type); break;
					case 5: trace_seq_printf(p, ":Radio Measure %d", type); break;
					case 6: trace_seq_printf(p, ":Fast BSS %d", type); break;
					case 7: trace_seq_printf(p, ":HT Action %d", type); break;
					case 8: trace_seq_printf(p, ":SA Query %d", type); break;
					case 9: trace_seq_printf(p, ":Protected Public %d", type); break;
					case 10: trace_seq_printf(p, ":WNM %d", type); break;
					case 11: trace_seq_printf(p, ":Unprotected WNM %d", type); break;
					case 12: trace_seq_printf(p, ":TDLS %d", type); break;
					case 13: trace_seq_printf(p, ":Mesh %d", type); break;
					case 14: trace_seq_printf(p, ":MultiHop %d", type); break;
					case 15: trace_seq_printf(p, ":Self Protected %d", type); break;
					case 126: trace_seq_printf(p, ":Vendor protected"); break;
					case 127: trace_seq_printf(p, ":Vendor"); break;
					default: trace_seq_printf(p, ":Unknown category %d", cat); break;
				}
			}
			break;
		default: trace_seq_printf(p, "Unknown subtype %d", frame_control & IEEE80211_FCTL_STYPE); break;
	}

	trace_seq_putc(p, 0);

	return ret;
}
#endif /* defined(CONFIG_TRACEPOINTS) && defined(CREATE_TRACE_POINTS) */

#undef __print_mgmt_info
#define __print_mgmt_info(frame_control, cat, type, p2p) ftrace_print_mgmt_info(p, frame_control, cat, type, p2p)

TRACE_EVENT(
	roc,
	TP_PROTO(u8 vif_idx, u16 freq, unsigned int duration),
	TP_ARGS(vif_idx, freq, duration),
	TP_STRUCT__entry(
		__field(u8, vif_idx)
		__field(u16, freq)
		__field(unsigned int, duration)
					 ),
	TP_fast_assign(
		__entry->vif_idx = vif_idx;
		__entry->freq = freq;
		__entry->duration = duration;
				   ),
	TP_printk("f=%d vif=%d dur=%d",
			__entry->freq, __entry->vif_idx, __entry->duration)
);

TRACE_EVENT(
	cancel_roc,
	TP_PROTO(u8 vif_idx),
	TP_ARGS(vif_idx),
	TP_STRUCT__entry(
		__field(u8, vif_idx)
					 ),
	TP_fast_assign(
		__entry->vif_idx = vif_idx;
				   ),
	TP_printk("vif=%d", __entry->vif_idx)
);

TRACE_EVENT(
	roc_exp,
	TP_PROTO(u8 vif_idx),
	TP_ARGS(vif_idx),
	TP_STRUCT__entry(
		__field(u8, vif_idx)
					 ),
	TP_fast_assign(
		__entry->vif_idx = vif_idx;
				   ),
	TP_printk("vif=%d", __entry->vif_idx)
);

TRACE_EVENT(
	switch_roc,
	TP_PROTO(u8 vif_idx),
	TP_ARGS(vif_idx),
	TP_STRUCT__entry(
		__field(u8, vif_idx)
					 ),
	TP_fast_assign(
		__entry->vif_idx = vif_idx;
				   ),
	TP_printk("vif=%d", __entry->vif_idx)
);

DECLARE_EVENT_CLASS(
	mgmt_template,
	TP_PROTO(u16 freq, u8 vif_idx, u16 sta_idx, struct ieee80211_mgmt *mgmt),
	TP_ARGS(freq, vif_idx, sta_idx, mgmt),
	TP_STRUCT__entry(
		__field(u16, freq)
		__field(u8, vif_idx)
		__field(u16, sta_idx)
		__field(u16, frame_control)
		__field(u8, action_cat)
		__field(u8, action_type)
		__field(u8, action_vendor)
					 ),
	TP_fast_assign(
		__entry->freq = freq;
		__entry->vif_idx = vif_idx;
		__entry->sta_idx = sta_idx;
		__entry->frame_control = mgmt->frame_control;
		if ((mgmt->u.action.category == MGMT_ACTION_PUBLIC_CAT) &&
			(mgmt->u.action.u.wme_action.action_code == 0x9)) {
			__entry->action_vendor = *((u8 *)&mgmt->u.action.category
									   + MGMT_ACTION_OUI_SUBTYPE_OFFSET);
			__entry->action_cat = *((u8 *)&mgmt->u.action.category
									 + MGMT_ACTION_OUI_SUBTYPE_OFFSET + 1);
			__entry->action_type = *((u8 *)&mgmt->u.action.category
									 + MGMT_ACTION_OUI_SUBTYPE_OFFSET + 2);
		} else {
			__entry->action_cat = mgmt->u.action.category;
			__entry->action_type = mgmt->u.action.u.wme_action.action_code;
			__entry->action_vendor = 0;
		}
				   ),
	TP_printk("f=%d vif=%d sta=%d -> %s",
			__entry->freq, __entry->vif_idx, __entry->sta_idx,
			  __print_mgmt_info(__entry->frame_control, __entry->action_cat,
								__entry->action_type, __entry->action_vendor))
);

DEFINE_EVENT(mgmt_template, mgmt_tx,
			 TP_PROTO(u16 freq, u8 vif_idx, u16 sta_idx, struct ieee80211_mgmt *mgmt),
			 TP_ARGS(freq, vif_idx, sta_idx, mgmt));

DEFINE_EVENT(mgmt_template, mgmt_rx,
			 TP_PROTO(u16 freq, u8 vif_idx, u16 sta_idx, struct ieee80211_mgmt *mgmt),
			 TP_ARGS(freq, vif_idx, sta_idx, mgmt));

TRACE_EVENT(
	mgmt_cfm,
	TP_PROTO(u8 vif_idx, u16 sta_idx, bool acked),
	TP_ARGS(vif_idx, sta_idx, acked),
	TP_STRUCT__entry(
		__field(u8, vif_idx)
		__field(u16, sta_idx)
		__field(bool, acked)
					 ),
	TP_fast_assign(
		__entry->vif_idx = vif_idx;
		__entry->sta_idx = sta_idx;
		__entry->acked = acked;
				   ),
	TP_printk("vif=%d sta=%d ack=%d",
			__entry->vif_idx, __entry->sta_idx, __entry->acked)
);

/*****************************************************************************
 * TRACE function for TXQ
 ****************************************************************************/
#if defined(CONFIG_TRACEPOINTS) && defined(CREATE_TRACE_POINTS)

#include <linux/trace_seq.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
#include <linux/trace_events.h>
#else
#include <linux/ftrace_event.h>
#endif

const char *
ftrace_print_txq(struct trace_seq *p, int txq_idx) {
	const char *ret = trace_seq_buffer_ptr(p);
#if 0
	// CLS_ macro is deleted, cannot be compiled for ubuntu system,
	// delete the code 

	if (txq_idx == TXQ_INACTIVE) {
		trace_seq_printf(p, "[INACTIVE]");
	} else if (txq_idx < CLS_FIRST_VIF_TXQ_IDX) {
		trace_seq_printf(p, "[STA %d/%d]",
						 txq_idx / CLS_NB_TXQ_PER_STA,
						 txq_idx % CLS_NB_TXQ_PER_STA);
	} else if (txq_idx < CLS_FIRST_UNK_TXQ_IDX) {
		trace_seq_printf(p, "[BC/MC %d]",
						 txq_idx - CLS_FIRST_BCMC_TXQ_IDX);
	} else if (txq_idx < CLS_OFF_CHAN_TXQ_IDX) {
		trace_seq_printf(p, "[UNKNOWN %d]",
						 txq_idx - CLS_FIRST_UNK_TXQ_IDX);
	} else if (txq_idx == CLS_OFF_CHAN_TXQ_IDX) {
		trace_seq_printf(p, "[OFFCHAN]");
	} else {
		trace_seq_printf(p, "[ERROR %d]", txq_idx);
	}

	trace_seq_putc(p, 0);
#endif

	return ret;
}

const char *
ftrace_print_sta(struct trace_seq *p, u16 sta_idx) {
	const char *ret = trace_seq_buffer_ptr(p);

#if 0
	if (sta_idx < CLS_REMOTE_STA_MAX) {
		trace_seq_printf(p, "[STA %d]", sta_idx);
	} else {
		trace_seq_printf(p, "[BC/MC %d]", sta_idx - CLS_REMOTE_STA_MAX);
	}

	trace_seq_putc(p, 0);
#endif

	return ret;
}

const char *
ftrace_print_hwq(struct trace_seq *p, int hwq_idx) {
	const char *ret = trace_seq_buffer_ptr(p);
#if 0

	if (hwq_idx < CLS_WIFI_HWQ_USER_BASE) {
		static const struct trace_print_flags symbols[] =
			{{CLS_WIFI_HWQ_BK, "BK"},
			 {CLS_WIFI_HWQ_BE, "BE"},
			 {CLS_WIFI_HWQ_VI, "VI"},
			 {CLS_WIFI_HWQ_VO, "VO"},
			 {CLS_WIFI_HWQ_BCMC, "BCMC"},
			 { -1, NULL }};

		return trace_print_symbols_seq(p, hwq_idx, symbols);
	}

	trace_seq_printf(p, "USER %d", hwq_idx - CLS_WIFI_HWQ_USER_BASE);
	trace_seq_putc(p, 0);
#endif
	return ret;
}

const char *
ftrace_print_hwq_size(struct trace_seq *p, int hwq_idx, int size) {
	const char *ret = trace_seq_buffer_ptr(p);
#if 0

	if (hwq_idx >= CLS_WIFI_HWQ_USER_BASE) {
		trace_seq_printf(p, "hw_size=%d ", size);
	}

	trace_seq_putc(p, 0);
#endif
	return ret;
}

const char *
ftrace_print_amsdu(struct trace_seq *p, u16 nb_pkt) {
	const char *ret = trace_seq_buffer_ptr(p);

	if (nb_pkt > 1)
		trace_seq_printf(p, "(AMSDU %d)", nb_pkt);

	trace_seq_putc(p, 0);
	return ret;
}

const char *
ftrace_print_sn(struct trace_seq *p, u16 sn) {
	const char *ret = trace_seq_buffer_ptr(p);

	if (sn < 4096)
		trace_seq_printf(p, "(SN %d) ", sn);

	trace_seq_putc(p, 0);
	return ret;
}

const char *
ftrace_print_he_mu_map(struct trace_seq *p, u8 cnt, u16* id, u16 *tid, u8* pos, u16 *cred, u32 *len) {
	const char *ret = trace_seq_buffer_ptr(p);
	int i;

	for (i = 0 ; i < cnt; i++) {
		if (i > 0)
			trace_seq_printf(p, ", ");
		trace_seq_printf(p, "[STA %d] user=%d credits=%d tid=0x%x psdu_len=%d",
						 id[i], pos[i], cred[i], tid[i], len[i]);
	}

	trace_seq_putc(p, 0);
	return ret;
}

#undef __print_txq
#define __print_txq(txq_idx) ftrace_print_txq(p, txq_idx)

#undef __print_sta
#define __print_sta(sta_idx) ftrace_print_sta(p, sta_idx)

#undef __print_hwq
#define __print_hwq(hwq) ftrace_print_hwq(p, hwq)

#undef __print_hwq_size
#define __print_hwq_size(hwq, size) ftrace_print_hwq_size(p, hwq, size)

#undef __print_amsdu
#define __print_amsdu(nb_pkt) ftrace_print_amsdu(p, nb_pkt)

#undef __print_sn
#define __print_sn(sn) ftrace_print_sn(p, sn)

#undef __print_he_mu_map
#define __print_he_mu_map(cnt, id, tid, pos ,cred, len) \
	ftrace_print_he_mu_map(p, cnt, id, tid, pos, cred, len)

#endif /* defined(CONFIG_TRACEPOINTS) && defined(CREATE_TRACE_POINTS) */

TRACE_EVENT(
	txq_select,
	TP_PROTO(int txq_idx, u16 pkt_ready_up, struct sk_buff *skb),
	TP_ARGS(txq_idx, pkt_ready_up, skb),
	TP_STRUCT__entry(
		__field(u16, txq_idx)
		__field(u16, pkt_ready)
		__field(struct sk_buff *, skb)
					 ),
	TP_fast_assign(
		__entry->txq_idx = txq_idx;
		__entry->pkt_ready = pkt_ready_up;
		__entry->skb = skb;
				   ),
	TP_printk("%s pkt_ready_up=%d skb=%p", __print_txq(__entry->txq_idx),
			  __entry->pkt_ready, __entry->skb)
);

DECLARE_EVENT_CLASS(
	hwq_template,
	TP_PROTO(u8 hwq_idx),
	TP_ARGS(hwq_idx),
	TP_STRUCT__entry(
		__field(u8, hwq_idx)
					 ),
	TP_fast_assign(
		__entry->hwq_idx = hwq_idx;
				   ),
	TP_printk("%s", __print_hwq(__entry->hwq_idx))
);

DEFINE_EVENT(hwq_template, hwq_flowctrl_stop,
			 TP_PROTO(u8 hwq_idx),
			 TP_ARGS(hwq_idx));

DEFINE_EVENT(hwq_template, hwq_flowctrl_start,
			 TP_PROTO(u8 hwq_idx),
			 TP_ARGS(hwq_idx));


DECLARE_EVENT_CLASS(
	txq_template,
	TP_PROTO(struct cls_wifi_txq *txq),
	TP_ARGS(txq),
	TP_STRUCT__entry(
		__field(u16, txq_idx)
					 ),
	TP_fast_assign(
		__entry->txq_idx = txq->idx;
				   ),
	TP_printk("%s", __print_txq(__entry->txq_idx))
);

DEFINE_EVENT(txq_template, txq_add_to_hw,
			 TP_PROTO(struct cls_wifi_txq *txq),
			 TP_ARGS(txq));

DEFINE_EVENT(txq_template, txq_del_from_hw,
			 TP_PROTO(struct cls_wifi_txq *txq),
			 TP_ARGS(txq));

DEFINE_EVENT(txq_template, txq_flowctrl_stop,
			 TP_PROTO(struct cls_wifi_txq *txq),
			 TP_ARGS(txq));

DEFINE_EVENT(txq_template, txq_flowctrl_restart,
			 TP_PROTO(struct cls_wifi_txq *txq),
			 TP_ARGS(txq));

TRACE_EVENT(
	process_txq,
	TP_PROTO(struct cls_wifi_txq *txq),
	TP_ARGS(txq),
	TP_STRUCT__entry(
		__field(u16, txq_idx)
		__field(u16, len)
		__field(u16, len_retry)
		__field(s8, credit)
		__field(u16, limit)
					 ),
	TP_fast_assign(
		__entry->txq_idx = txq->idx;
		__entry->len = skb_queue_len(&txq->sk_list);
		#ifdef CONFIG_MAC80211_TXQ
		__entry->len += txq->nb_ready_mac80211;
		#endif
		__entry->len_retry = txq->nb_retry;
		__entry->credit = txq->credits;
		__entry->limit = txq->push_limit;
				   ),

	TP_printk("%s txq_credits=%d, len=%d, retry_len=%d, push_limit=%d",
			  __print_txq(__entry->txq_idx), __entry->credit,
			  __entry->len, __entry->len_retry, __entry->limit)
);

DECLARE_EVENT_CLASS(
	txq_reason_template,
	TP_PROTO(struct cls_wifi_txq *txq, u16 reason),
	TP_ARGS(txq, reason),
	TP_STRUCT__entry(
		__field(u16, txq_idx)
		__field(u16, reason)
		__field(u16, status)
					 ),
	TP_fast_assign(
		__entry->txq_idx = txq->idx;
		__entry->reason = reason;
		__entry->status = txq->status;
				   ),
	TP_printk("%s reason=%s status=%s",
			  __print_txq(__entry->txq_idx),
			  __print_symbolic(__entry->reason,
							   {CLS_WIFI_TXQ_STOP_FULL, "FULL"},
							   {CLS_WIFI_TXQ_STOP_CSA, "CSA"},
							   {CLS_WIFI_TXQ_STOP_STA_PS, "PS"},
							   {CLS_WIFI_TXQ_STOP_VIF_PS, "VPS"},
							   {CLS_WIFI_TXQ_STOP_CHAN, "CHAN"},
							   {CLS_WIFI_TXQ_STOP_MU, "MU"},
							   {CLS_WIFI_TXQ_STOP_TWT, "TWT"}),
			  __print_flags(__entry->status, "|",
							{CLS_WIFI_TXQ_IN_HWQ_LIST, "IN LIST"},
							{CLS_WIFI_TXQ_STOP_FULL, "FULL"},
							{CLS_WIFI_TXQ_STOP_CSA, "CSA"},
							{CLS_WIFI_TXQ_STOP_STA_PS, "PS"},
							{CLS_WIFI_TXQ_STOP_VIF_PS, "VPS"},
							{CLS_WIFI_TXQ_STOP_CHAN, "CHAN"},
							{CLS_WIFI_TXQ_STOP_MU, "MU"},
							{CLS_WIFI_TXQ_STOP_TWT, "TWT"},
							{CLS_WIFI_TXQ_NDEV_FLOW_CTRL, "FLW_CTRL"}))
);

DEFINE_EVENT(txq_reason_template, txq_start,
			 TP_PROTO(struct cls_wifi_txq *txq, u16 reason),
			 TP_ARGS(txq, reason));

DEFINE_EVENT(txq_reason_template, txq_stop,
			 TP_PROTO(struct cls_wifi_txq *txq, u16 reason),
			 TP_ARGS(txq, reason));

TRACE_EVENT(
	push_desc,
	TP_PROTO(struct sk_buff *skb, struct cls_wifi_sw_txhdr *sw_txhdr, int push_flags),

	TP_ARGS(skb, sw_txhdr, push_flags),

	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(unsigned int, len)
		__field(u16, tx_queue)
		__field(u8, hw_queue)
		__field(u8, push_flag)
		__field(u32, flag)
		__field(s8, txq_cred)
		__field(u8, hwq_cred)
		__field(u8, txq_length)
		__field(u16, pkt_cnt)
		__field(u32, hostid)
#ifdef CONFIG_CLS_WIFI_HEMU_TX
		__field(int, remain_hwq)
#endif
					 ),
	TP_fast_assign(
		__entry->skb = skb;
		__entry->tx_queue = sw_txhdr->txq->idx;
		__entry->push_flag = push_flags;
		__entry->hw_queue = sw_txhdr->txq->hwq->id;
		__entry->txq_cred = sw_txhdr->txq->credits;
		__entry->hwq_cred = sw_txhdr->txq->hwq->credits;
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
		__entry->pkt_cnt =  sw_txhdr->desc.api.host.packet_cnt;
#else
		__entry->pkt_cnt =  1;
#endif
		__entry->txq_length = skb_queue_len(&sw_txhdr->txq->sk_list);
		__entry->flag = sw_txhdr->desc.api.host.flags;
		__entry->len = sw_txhdr->frame_len;
		__entry->hostid = sw_txhdr->desc.api.host.hostid;
#ifdef CONFIG_CLS_WIFI_HEMU_TX
		__entry->remain_hwq = sw_txhdr->txq->hwq->size_limit - sw_txhdr->txq->hwq->size_pushed;
#endif
				   ),

	TP_printk("%s skb=%p hostid=%d (len=%d) txq_length=%d txq_credits=%d hw_queue=%s hw_credits=%d %sflag=%s %s%s%s",
			  __print_txq(__entry->tx_queue), __entry->skb, __entry->hostid, __entry->len,
			  __entry->txq_length, __entry->txq_cred, __print_hwq(__entry->hw_queue), __entry->hwq_cred,
#ifdef CONFIG_CLS_WIFI_HEMU_TX
			  __print_hwq_size(__entry->hw_queue, __entry->remain_hwq),
#else
			  "",
#endif
			  __print_flags(__entry->flag, "|",
							{TXU_CNTRL_MORE_DATA, "MOREDATA"},
							{TXU_CNTRL_MGMT, "MGMT"},
							{TXU_CNTRL_MGMT_NO_CCK, "NO_CCK"},
							{TXU_CNTRL_MGMT_ROBUST, "ROBUST"},
							{TXU_CNTRL_AMSDU, "AMSDU"},
							{TXU_CNTRL_USE_4ADDR, "4ADDR"},
							{TXU_CNTRL_EOSP, "EOSP"},
							{TXU_CNTRL_MESH_FWD, "MESH_FWD"},
							{TXU_CNTRL_TDLS, "TDLS"},
							{TXU_CNTRL_REUSE_SN, "SN"},
							{TXU_CNTRL_SKIP_LOGIC_PORT, "SKIP_LOGIC"}),
			  (__entry->push_flag & CLS_WIFI_PUSH_IMMEDIATE) ? "(IMMEDIATE)" : "",
			  (__entry->push_flag & CLS_WIFI_PUSH_RETRY) ? "(SW_RETRY)" : "",
			  __print_amsdu(__entry->pkt_cnt))
);


TRACE_EVENT(
	txq_queue_skb,
	TP_PROTO(struct sk_buff *skb, struct cls_wifi_txq *txq, bool retry),
	TP_ARGS(skb, txq, retry),
	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(s8, credit)
		__field(u16, q_len)
		__field(u16, q_len_retry)
		__field(bool, retry)
					 ),
	TP_fast_assign(
		__entry->skb = skb;
		__entry->txq_idx = txq->idx;
		__entry->credit = txq->credits;
		__entry->q_len = skb_queue_len(&txq->sk_list);
		__entry->q_len_retry = txq->nb_retry;
		__entry->retry = retry;
				   ),

	TP_printk("%s skb=%p retry=%d txq_credits=%d txq_length=%d (retry = %d)",
			  __print_txq(__entry->txq_idx), __entry->skb, __entry->retry,
			  __entry->credit, __entry->q_len, __entry->q_len_retry)
);


TRACE_EVENT(
	txq_drop_skb,
	TP_PROTO(struct sk_buff *skb, struct cls_wifi_txq *txq, unsigned long queued_time),
	TP_ARGS(skb, txq, queued_time),
	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(unsigned long, queued_time)
		__field(u16, q_len)
		__field(u16, q_len_retry)
					 ),
	TP_fast_assign(
		__entry->skb = skb;
		__entry->txq_idx = txq->idx;
		__entry->q_len = skb_queue_len(&txq->sk_list);
		__entry->q_len_retry = txq->nb_retry;
		__entry->queued_time = queued_time;
				   ),

	TP_printk("%s skb=%p time_queued=%dms txq_length=%d (retry = %d)",
			  __print_txq(__entry->txq_idx), __entry->skb,
			  jiffies_to_msecs(__entry->queued_time), __entry->q_len, __entry->q_len_retry)
);

#ifdef CONFIG_MAC80211_TXQ
TRACE_EVENT(
	txq_wake,
	TP_PROTO(struct cls_wifi_txq *txq),
	TP_ARGS(txq),
	TP_STRUCT__entry(
		__field(u16, txq_idx)
		__field(u16, q_len)
					 ),
	TP_fast_assign(
		__entry->txq_idx = txq->idx;
		__entry->q_len = txq->nb_ready_mac80211;
				   ),

	TP_printk("%s mac80211_txq_length=%d", __print_txq(__entry->txq_idx), __entry->q_len)
);

TRACE_EVENT(
	txq_drop,
	TP_PROTO(struct cls_wifi_txq *txq, unsigned long nb_drop),
	TP_ARGS(txq, nb_drop),
	TP_STRUCT__entry(
		__field(u16, txq_idx)
		__field(u16, nb_drop)
					 ),
	TP_fast_assign(
		__entry->txq_idx = txq->idx;
		__entry->nb_drop = nb_drop;
				   ),

	TP_printk("%s %u pkt have been dropped by codel in mac80211 txq",
			  __print_txq(__entry->txq_idx), __entry->nb_drop)
);

#endif


DECLARE_EVENT_CLASS(
	idx_template,
	TP_PROTO(u16 idx),
	TP_ARGS(idx),
	TP_STRUCT__entry(
		__field(u16, idx)
					 ),
	TP_fast_assign(
		__entry->idx = idx;
				   ),
	TP_printk("idx=%d", __entry->idx)
);


DEFINE_EVENT(idx_template, txq_vif_start,
			 TP_PROTO(u16 idx),
			 TP_ARGS(idx));

DEFINE_EVENT(idx_template, txq_vif_stop,
			 TP_PROTO(u16 idx),
			 TP_ARGS(idx));

TRACE_EVENT(
	process_hw_queue,
	TP_PROTO(struct cls_wifi_hwq *hwq),
	TP_ARGS(hwq),
	TP_STRUCT__entry(
		__field(u16, hwq)
		__field(u8, credits)
					 ),
	TP_fast_assign(
		__entry->hwq = hwq->id;
		__entry->credits = hwq->credits;
				   ),
	TP_printk("hw_queue=%s hw_credits=%d",
			  __print_hwq(__entry->hwq), __entry->credits)
);

DECLARE_EVENT_CLASS(
	sta_idx_template,
	TP_PROTO(u16 idx),
	TP_ARGS(idx),
	TP_STRUCT__entry(
		__field(u16, idx)
					 ),
	TP_fast_assign(
		__entry->idx = idx;
				   ),
	TP_printk("%s", __print_sta(__entry->idx))
);

DEFINE_EVENT(sta_idx_template, txq_sta_start,
			 TP_PROTO(u16 idx),
			 TP_ARGS(idx));

DEFINE_EVENT(sta_idx_template, txq_sta_stop,
			 TP_PROTO(u16 idx),
			 TP_ARGS(idx));

DEFINE_EVENT(sta_idx_template, ps_disable,
			 TP_PROTO(u16 idx),
			 TP_ARGS(idx));

TRACE_EVENT(
	skb_confirm,
	TP_PROTO(struct sk_buff *skb, struct cls_wifi_txq *txq, struct cls_wifi_hwq *hwq,
			 struct tx_cfm_tag *cfm
			 ),

	TP_ARGS(skb, txq, hwq, cfm
			),

	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(u8, hw_queue)
		__field(u8, hw_credit)
		__field(s8, sw_credit)
		__field(s8, sw_credit_up)
		__field(u8, ampdu_size)
		__field(u16, sn)
		__field(u32, hostid)
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
		__field(u16, amsdu)
#endif /* CONFIG_CLS_WIFI_SPLIT_TX_BUF */
					 ),

	TP_fast_assign(
		__entry->skb = skb;
		__entry->txq_idx = txq->idx;
		__entry->hw_queue = hwq->id;
		__entry->hw_credit = hwq->credits;
		__entry->sw_credit = txq->credits;
		__entry->sw_credit_up = cfm->credits;
		__entry->ampdu_size = cfm->ampdu_size;
		__entry->sn = TXU_STATUS_GET(SN, cfm->status);
		__entry->hostid = cfm->hostid;
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
		__entry->amsdu = cfm->amsdu_size;
#endif

				   ),

	TP_printk("%s skb=%p hw_queue=%s, hw_credits=%d, txq_credits=%d (+%d)"
			  " hostid=%d sn=%u ampdu=%d"
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
			  " amsdu=%u"
#endif

			  , __print_txq(__entry->txq_idx), __entry->skb,
			  __print_hwq(__entry->hw_queue),
			  __entry->hw_credit, __entry->sw_credit, __entry->sw_credit_up
			  , __entry->hostid, __entry->sn, __entry->ampdu_size
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
			  , __entry->amsdu
#endif
			  )
);

TRACE_EVENT(
	skb_retry,
	TP_PROTO(struct sk_buff *skb, struct cls_wifi_txq *txq, uint16_t sn),

	TP_ARGS(skb, txq, sn),

	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(s8, txq_credit)
		__field(u16, sn)
					 ),

	TP_fast_assign(
		__entry->skb = skb;
		__entry->txq_idx = txq->idx;
		__entry->txq_credit = txq->credits;
		__entry->sn = sn;
				   ),

	TP_printk("%s skb=%p txq_credits=%d %s",
			   __print_txq(__entry->txq_idx), __entry->skb,
			  __entry->txq_credit, __print_sn(__entry->sn))
);

TRACE_EVENT(
	credit_update,
	TP_PROTO(struct cls_wifi_txq *txq, s8_l cred_up),

	TP_ARGS(txq, cred_up),

	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(s8, sw_credit)
		__field(s8, sw_credit_up)
					 ),

	TP_fast_assign(
		__entry->txq_idx = txq->idx;
		__entry->sw_credit = txq->credits;
		__entry->sw_credit_up = cred_up;
				   ),

	TP_printk("%s txq_credits=%d (%+d)", __print_txq(__entry->txq_idx),
			  __entry->sw_credit, __entry->sw_credit_up)
)

DECLARE_EVENT_CLASS(
	ps_template,
	TP_PROTO(struct cls_wifi_sta *sta),
	TP_ARGS(sta),
	TP_STRUCT__entry(
		__field(u16, idx)
		__field(u16, ready_ps)
		__field(u16, sp_ps)
		__field(u16, ready_uapsd)
		__field(u16, sp_uapsd)
					 ),
	TP_fast_assign(
		__entry->idx  = sta->sta_idx;
		__entry->ready_ps = sta->ps.pkt_ready[LEGACY_PS_ID];
		__entry->sp_ps = sta->ps.sp_cnt[LEGACY_PS_ID];
		__entry->ready_uapsd = sta->ps.pkt_ready[UAPSD_ID];
		__entry->sp_uapsd = sta->ps.sp_cnt[UAPSD_ID];
				   ),

	TP_printk("%s [PS] ready=%d sp=%d [UAPSD] ready=%d sp=%d",
			  __print_sta(__entry->idx), __entry->ready_ps, __entry->sp_ps,
			  __entry->ready_uapsd, __entry->sp_uapsd)
);

DEFINE_EVENT(ps_template, ps_queue,
			 TP_PROTO(struct cls_wifi_sta *sta),
			 TP_ARGS(sta));

DEFINE_EVENT(ps_template, ps_drop,
			 TP_PROTO(struct cls_wifi_sta *sta),
			 TP_ARGS(sta));

DEFINE_EVENT(ps_template, ps_push,
			 TP_PROTO(struct cls_wifi_sta *sta),
			 TP_ARGS(sta));

DEFINE_EVENT(ps_template, ps_enable,
			 TP_PROTO(struct cls_wifi_sta *sta),
			 TP_ARGS(sta));

TRACE_EVENT(
	ps_traffic_update,
	TP_PROTO(u16 sta_idx, u8 traffic, bool uapsd),

	TP_ARGS(sta_idx, traffic, uapsd),

	TP_STRUCT__entry(
		__field(u16, sta_idx)
		__field(u8, traffic)
		__field(bool, uapsd)
					 ),

	TP_fast_assign(
		__entry->sta_idx = sta_idx;
		__entry->traffic = traffic;
		__entry->uapsd = uapsd;
				   ),

	TP_printk("%s %s%s traffic available ", __print_sta(__entry->sta_idx),
			  __entry->traffic ? "" : "no more ",
			  __entry->uapsd ? "U-APSD" : "legacy PS")
);

TRACE_EVENT(
	ps_traffic_req,
	TP_PROTO(struct cls_wifi_sta *sta, u16 pkt_req, u8 ps_id),
	TP_ARGS(sta, pkt_req, ps_id),
	TP_STRUCT__entry(
		__field(u16, idx)
		__field(u16, pkt_req)
		__field(u8, ps_id)
		__field(u16, ready)
		__field(u16, sp)
					 ),
	TP_fast_assign(
		__entry->idx  = sta->sta_idx;
		__entry->pkt_req  = pkt_req;
		__entry->ps_id  = ps_id;
		__entry->ready = sta->ps.pkt_ready[ps_id];
		__entry->sp = sta->ps.sp_cnt[ps_id];
				   ),

	TP_printk("%s %s traffic request %d pkt (ready=%d, sp=%d)",
			  __print_sta(__entry->idx),
			  __entry->ps_id == UAPSD_ID ? "U-APSD" : "legacy PS" ,
			  __entry->pkt_req, __entry->ready, __entry->sp)
);


#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
TRACE_EVENT(
	amsdu_subframe,
	TP_PROTO(struct cls_wifi_sw_txhdr *sw_txhdr),
	TP_ARGS(sw_txhdr),
	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(u8, nb)
		__field(u32, len)
					 ),
	TP_fast_assign(
		__entry->skb = sw_txhdr->skb;
		__entry->nb = sw_txhdr->amsdu.nb;
		__entry->len = sw_txhdr->frame_len;
		__entry->txq_idx = sw_txhdr->txq->idx;
				   ),

	TP_printk("%s skb=%p %s nb_subframe=%d, len=%u",
			  __print_txq(__entry->txq_idx), __entry->skb,
			  (__entry->nb == 2) ? "Start new AMSDU" : "Add subframe",
			  __entry->nb, __entry->len)
);

TRACE_EVENT(
	amsdu_dismantle,
	TP_PROTO(struct cls_wifi_sw_txhdr *sw_txhdr),
	TP_ARGS(sw_txhdr),
	TP_STRUCT__entry(
		__field(struct sk_buff *, skb)
		__field(u16, txq_idx)
		__field(u8, nb)
		__field(u32, len)
					 ),
	TP_fast_assign(
		__entry->skb = sw_txhdr->skb;
		__entry->nb = sw_txhdr->amsdu.nb;
		__entry->len = sw_txhdr->frame_len;
		__entry->txq_idx = sw_txhdr->txq->idx;
				   ),

	TP_printk("%s skb=%p nb_subframe=%d, len=%u",
			  __print_txq(__entry->txq_idx), __entry->skb,
			  __entry->nb, __entry->len)
);

TRACE_EVENT(
	amsdu_len_update,
	TP_PROTO(struct cls_wifi_sta *sta, int amsdu_len),
	TP_ARGS(sta, amsdu_len),
	TP_STRUCT__entry(
		__field(u16, sta_idx)
		__field(u16, amsdu_len)
					 ),
	TP_fast_assign(
		__entry->sta_idx = sta->sta_idx;
		__entry->amsdu_len = amsdu_len;
				   ),

	TP_printk("[Sta %d] A-MSDU len = %d", __entry->sta_idx, __entry->amsdu_len)
);
#endif

TRACE_EVENT(
	he_mu_apply_dl_map,
	TP_PROTO(struct he_mu_map_array_desc *map, u8 ac),

	TP_ARGS(map, ac),

	TP_STRUCT__entry(
		__field(u8, map_idx)
		__field(u8, sta_cnt)
		__array(u16, sta_idx, 16)
		__array(u8, user_id, 16)
		__array(u16, credit, 16)
		__array(u16, tid, 16)
		__array(u32, len, 16)
		__field(u8, ac)
					 ),
	TP_fast_assign(
		int i;
		__entry->map_idx = map->idx;
		__entry->sta_cnt = map->cnt_sta;
		__entry->ac = ac;
		for (i = 0; i < map->cnt_sta; i++) {
			__entry->sta_idx[i] = map->sta_alloc[i].sta_idx;
			__entry->tid[i] = map->sta_alloc[i].tidmap;
			__entry->user_id[i] = map->sta_alloc[i].userid;
			__entry->credit[i] = map->sta_alloc[i].credit;
			__entry->len[i] = map->sta_alloc[i].psdu_len_max;
		}
				   ),

	TP_printk("Apply HE-MU MAP %d(%d):%s: %s", __entry->map_idx, __entry->sta_cnt,
			  __print_hwq(__entry->ac),
			  __print_he_mu_map(__entry->sta_cnt, __entry->sta_idx, __entry->tid,
								__entry->user_id, __entry->credit, __entry->len))
);

TRACE_EVENT(
	he_mu_dl_map,
	TP_PROTO(u32 idx, u16 sta_cnt),

	TP_ARGS(idx, sta_cnt),

	TP_STRUCT__entry(
		__field(u32, map_idx)
		__field(u16, sta_cnt)
					 ),
	TP_fast_assign(
		__entry->map_idx = idx;
		__entry->sta_cnt = sta_cnt;
				   ),

	TP_printk("received HE-MU MAP %d(%d)", __entry->map_idx, __entry->sta_cnt)
);

TRACE_EVENT(
	he_mu_disable,
	TP_PROTO(u8 idx),

	TP_ARGS(idx),

	TP_STRUCT__entry(
		__field(u8, map_idx)
					 ),
	TP_fast_assign(
		__entry->map_idx = idx;
				   ),

	TP_printk("Disable HE-MU with MAP %d", __entry->map_idx)
);

/*****************************************************************************
 * TRACE functions for MESH
 ****************************************************************************/
DECLARE_EVENT_CLASS(
	mesh_path_template,
	TP_PROTO(struct cls_wifi_mesh_path *mesh_path),
	TP_ARGS(mesh_path),
	TP_STRUCT__entry(
		__field(u8, idx)
		__field(u8, next_hop_sta)
		__array(u8, tgt_mac, ETH_ALEN)
					 ),

	TP_fast_assign(
		__entry->idx = mesh_path->path_idx;
		memcpy(__entry->tgt_mac, &mesh_path->tgt_mac_addr, ETH_ALEN);
		if (mesh_path->nhop_sta)
			__entry->next_hop_sta = mesh_path->nhop_sta->sta_idx;
		else
			__entry->next_hop_sta = 0xff;
				   ),

	TP_printk("Mpath(%d): target=%pM next_hop=STA-%d",
			  __entry->idx, __entry->tgt_mac, __entry->next_hop_sta)
);

DEFINE_EVENT(mesh_path_template, mesh_create_path,
			 TP_PROTO(struct cls_wifi_mesh_path *mesh_path),
			 TP_ARGS(mesh_path));

DEFINE_EVENT(mesh_path_template, mesh_delete_path,
			 TP_PROTO(struct cls_wifi_mesh_path *mesh_path),
			 TP_ARGS(mesh_path));

DEFINE_EVENT(mesh_path_template, mesh_update_path,
			 TP_PROTO(struct cls_wifi_mesh_path *mesh_path),
			 TP_ARGS(mesh_path));

/*****************************************************************************
 * TRACE functions for RADAR
 ****************************************************************************/
#ifdef CONFIG_CLS_WIFI_RADAR
TRACE_EVENT(
	radar_pulse,
	TP_PROTO(u8 chain, struct radar_pulse *pulse),
	TP_ARGS(chain, pulse),
	TP_STRUCT__entry(
		__field(u8, chain)
		__field(s16, freq)
		__field(u16, pri)
		__field(u8, len)
		__field(u8, fom)
					 ),
	TP_fast_assign(
		__entry->freq = pulse->freq * 2;
		__entry->len = pulse->len * 2;
		__entry->fom = pulse->fom * 6;
		__entry->pri = pulse->rep;
		__entry->chain = chain;
				   ),

	TP_printk("%s: PRI=%.5d LEN=%.3d FOM=%.2d%% freq=%dMHz ",
			  __print_symbolic(__entry->chain,
							   {CLS_WIFI_RADAR_RIU, "RIU"},
							   {CLS_WIFI_RADAR_FCU, "FCU"}),
			  __entry->pri, __entry->len, __entry->fom, __entry->freq)
			);

TRACE_EVENT(
	radar_detected,
	TP_PROTO(u8 chain, u8 region, s16 freq, u8 type, u16 pri),
	TP_ARGS(chain, region, freq, type, pri),
	TP_STRUCT__entry(
		__field(u8, chain)
		__field(u8, region)
		__field(s16, freq)
		__field(u8, type)
		__field(u16, pri)
					 ),
	TP_fast_assign(
		__entry->chain = chain;
		__entry->region = region;
		__entry->freq = freq;
		__entry->type = type;
		__entry->pri = pri;
				   ),
	TP_printk("%s: region=%s type=%d freq=%dMHz (pri=%dus)",
			  __print_symbolic(__entry->chain,
							   {CLS_WIFI_RADAR_RIU, "RIU"},
							   {CLS_WIFI_RADAR_FCU, "FCU"}),
			  __print_symbolic(__entry->region,
							   {NL80211_DFS_UNSET, "UNSET"},
							   {NL80211_DFS_FCC, "FCC"},
							   {NL80211_DFS_ETSI, "ETSI"},
							   {NL80211_DFS_JP, "JP"}),
			  __entry->type, __entry->freq, __entry->pri)
);

TRACE_EVENT(
	radar_set_region,
	TP_PROTO(u8 region),
	TP_ARGS(region),
	TP_STRUCT__entry(
		__field(u8, region)
					 ),
	TP_fast_assign(
		__entry->region = region;
				   ),
	TP_printk("region=%s",
			  __print_symbolic(__entry->region,
							   {NL80211_DFS_UNSET, "UNSET"},
							   {NL80211_DFS_FCC, "FCC"},
							   {NL80211_DFS_ETSI, "ETSI"},
							   {NL80211_DFS_JP, "JP"}))
);

TRACE_EVENT(
	radar_enable_detection,
	TP_PROTO(u8 region, u8 enable, u8 chain),
	TP_ARGS(region, enable, chain),
	TP_STRUCT__entry(
		__field(u8, region)
		__field(u8, chain)
		__field(u8, enable)
					 ),
	TP_fast_assign(
		__entry->chain = chain;
		__entry->enable = enable;
		__entry->region = region;
				   ),
	TP_printk("%s: %s radar detection %s",
			   __print_symbolic(__entry->chain,
							   {CLS_WIFI_RADAR_RIU, "RIU"},
							   {CLS_WIFI_RADAR_FCU, "FCU"}),
			  __print_symbolic(__entry->enable,
							   {CLS_WIFI_RADAR_DETECT_DISABLE, "Disable"},
							   {CLS_WIFI_RADAR_DETECT_ENABLE, "Enable (no report)"},
							   {CLS_WIFI_RADAR_DETECT_REPORT, "Enable"}),
			  __entry->enable == CLS_WIFI_RADAR_DETECT_DISABLE ? "" :
			  __print_symbolic(__entry->region,
							   {NL80211_DFS_UNSET, "UNSET"},
							   {NL80211_DFS_FCC, "FCC"},
							   {NL80211_DFS_ETSI, "ETSI"},
							   {NL80211_DFS_JP, "JP"}))
);
#endif /* CONFIG_CLS_WIFI_RADAR */

/*****************************************************************************
 * TRACE functions for IPC message
 ****************************************************************************/
#include "cls_wifi_strs.h"

DECLARE_EVENT_CLASS(
	ipc_msg_template,
	TP_PROTO(u16 id),
	TP_ARGS(id),
	TP_STRUCT__entry(
		__field(u16, id)
					 ),
	TP_fast_assign(
		__entry->id  = id;
				   ),

	TP_printk("%s (%d - %d)", CLS_WIFI_ID2STR(__entry->id),
			  MSG_T(__entry->id), MSG_I(__entry->id))
);

DEFINE_EVENT(ipc_msg_template, msg_send,
			 TP_PROTO(u16 id),
			 TP_ARGS(id));

DEFINE_EVENT(ipc_msg_template, msg_recv,
			 TP_PROTO(u16 id),
			 TP_ARGS(id));



#endif /* !defined(_CLS_WIFI_EVENTS_H_) || defined(TRACE_HEADER_MULTI_READ) */

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE cls_wifi_events
#include <trace/define_trace.h>
