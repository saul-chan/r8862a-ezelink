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

#ifndef _CLS_WIFI_PROF_H_
#define _CLS_WIFI_PROF_H_

#include "cls_wifi_platform.h"

#ifdef CONFIG_CLS_WIFI_SW_PROFILING
static inline void cls_wifi_prof_set(struct cls_wifi_hw *cls_wifi_hw, int val)
{
	cls_wifi_hw->plat->ep_ops->sys_write32(cls_wifi_hw->plat, 1/*cls_wifi_hw->radio_idx*/,
				0x00B08570, val);
}

static inline void cls_wifi_prof_clear(struct cls_wifi_hw *cls_wifi_hw, int val)
{
	cls_wifi_hw->plat->ep_ops->sys_write32(cls_wifi_hw->plat, 1/*cls_wifi_hw->radio_idx*/,
				0x00B08574, val);
}
#endif

enum {
	SW_PROF_HOSTBUF_IDX = 12,
	/****** IPC IRQs related signals ******/
	/* E2A direction */
	SW_PROF_IRQ_E2A_RXDESC = 16,	// to make sure we let 16 bits available for LMAC FW
	SW_PROF_IRQ_E2A_TXCFM,
	SW_PROF_IRQ_E2A_DBG,
	SW_PROF_IRQ_E2A_MSG,
	SW_PROF_IPC_MSGPUSH,
	SW_PROF_MSGALLOC,
	SW_PROF_MSGIND,
	SW_PROF_DBGIND,

	/* A2E direction */
	SW_PROF_IRQ_A2E_TXCFM_BACK,

	/****** Driver functions related signals ******/
	SW_PROF_WAIT_QUEUE_STOP,
	SW_PROF_WAIT_QUEUE_WAKEUP,
	SW_PROF_CLS_WIFIDATAIND,
	SW_PROF_CLS_WIFI_IPC_IRQ_HDLR,
	SW_PROF_CLS_WIFI_IPC_THR_IRQ_HDLR,
	SW_PROF_IEEE80211RX,
	SW_PROF_CLS_WIFI_PATTERN,
	SW_PROF_MAX
};

// [LT]For debug purpose only
#if (0)
#define SW_PROF_CHAN_CTXT_CFM_HDL_BIT	   (21)
#define SW_PROF_CHAN_CTXT_CFM_BIT		   (22)
#define SW_PROF_CHAN_CTXT_CFM_SWDONE_BIT	(23)
#define SW_PROF_CHAN_CTXT_PUSH_BIT		  (24)
#define SW_PROF_CHAN_CTXT_QUEUE_BIT		 (25)
#define SW_PROF_CHAN_CTXT_TX_BIT			(26)
#define SW_PROF_CHAN_CTXT_TX_PAUSE_BIT	  (27)
#define SW_PROF_CHAN_CTXT_PSWTCH_BIT		(28)
#define SW_PROF_CHAN_CTXT_SWTCH_BIT		 (29)

// TO DO: update this

#define REG_SW_SET_PROFILING_CHAN(env, bit)			 \
	cls_wifi_prof_set((struct cls_wifi_hw*)env, BIT(bit))

#define REG_SW_CLEAR_PROFILING_CHAN(env, bit) \
	cls_wifi_prof_clear((struct cls_wifi_hw*)env, BIT(bit))

#else
#define SW_PROF_CHAN_CTXT_CFM_HDL_BIT	   (0)
#define SW_PROF_CHAN_CTXT_CFM_BIT		   (0)
#define SW_PROF_CHAN_CTXT_CFM_SWDONE_BIT	(0)
#define SW_PROF_CHAN_CTXT_PUSH_BIT		  (0)
#define SW_PROF_CHAN_CTXT_QUEUE_BIT		 (0)
#define SW_PROF_CHAN_CTXT_TX_BIT			(0)
#define SW_PROF_CHAN_CTXT_TX_PAUSE_BIT	  (0)
#define SW_PROF_CHAN_CTXT_PSWTCH_BIT		(0)
#define SW_PROF_CHAN_CTXT_SWTCH_BIT		 (0)

#define REG_SW_SET_PROFILING_CHAN(env, bit)			do {} while (0)
#define REG_SW_CLEAR_PROFILING_CHAN(env, bit)		  do {} while (0)
#endif

#ifdef CONFIG_CLS_WIFI_SW_PROFILING
/* Macros for SW PRofiling registers access */
#define REG_SW_SET_PROFILING(env, bit)				  \
	cls_wifi_prof_set((struct cls_wifi_hw*)env, BIT(bit))

#define REG_SW_SET_HOSTBUF_IDX_PROFILING(env, val)	  \
	cls_wifi_prof_set((struct cls_wifi_hw*)env, val<<(SW_PROF_HOSTBUF_IDX))

#define REG_SW_CLEAR_PROFILING(env, bit)				\
	cls_wifi_prof_clear((struct cls_wifi_hw*)env, BIT(bit))

#define REG_SW_CLEAR_HOSTBUF_IDX_PROFILING(env)						 \
	cls_wifi_prof_clear((struct cls_wifi_hw*)env,0x0F<<(SW_PROF_HOSTBUF_IDX))

#else
#define REG_SW_SET_PROFILING(env, value)			do {} while (0)
#define REG_SW_CLEAR_PROFILING(env, value)		  do {} while (0)
#define REG_SW_SET_HOSTBUF_IDX_PROFILING(env, val)  do {} while (0)
#define REG_SW_CLEAR_HOSTBUF_IDX_PROFILING(env)	 do {} while (0)
#endif

#endif /* _CLS_WIFI_PROF_H_ */
