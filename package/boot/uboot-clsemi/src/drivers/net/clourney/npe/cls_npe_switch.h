/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

#ifndef _CLS_NPE_SWITCH_H_
#define _CLS_NPE_SWITCH_H_
#include "cls_npe.h"
#include "cls_npe_switch_regs.h"

#define CLS_NPE_CPU_PORT				5

void cls_eth_switch_init(struct udevice *dev);
void cls_router_port_macAddr_table_del(struct udevice *dev,
		u8 *macAddr, u8 *macMask);
void cls_ingress_port_packet_type_filter(struct udevice *dev, bool accept);
#endif
