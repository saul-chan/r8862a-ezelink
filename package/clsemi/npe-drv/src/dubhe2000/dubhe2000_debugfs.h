/*
 * Definitions for Clourney DUBHE1000 network controller.
 *
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 */
#ifndef _DUBHE1000_DEBUGFS_H_
#define _DUBHE1000_DEBUGFS_H_

#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/net_tstamp.h>
#include <linux/phy.h>
#include <linux/phylink.h>
#include <net/flow_offload.h>
#include <net/page_pool.h>
#include <linux/bpf.h>
#include <net/xdp.h>

#include "dubhe2000.h"

extern u32 g_max_txrx;

void dubhe1000_dump_regs(struct dubhe1000_adapter *adapter, void __iomem *address);
void dubhe1000_port_dbg_init(struct dubhe1000_mac *port);
void dubhe1000_dbg_init(struct dubhe1000_adapter *adapter);
void dubhe1000_dbg_exit(struct dubhe1000_adapter *adapter);

#endif
