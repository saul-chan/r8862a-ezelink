/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

#ifndef _CLS_NPE_EDMA_H_
#define _CLS_NPE_EDMA_H_
#include "cls_npe.h"

#define CPU_LHOST	0
#define CPU_2DOT4G	1
#define CPU_5G		2
#define CPU_PCIE	3

#define LOCAL_CPU	CPU_LHOST

#define CLS_NPE_FIFO_DEPTH_LEVEL	16

/* 1. CONFIG REG */
#define CLS_NPE_MODULE_ENABLE		0x0000 // RW

/* TX REGISTER: to cpu part */
#define CLS_NPE_TX_BASE_REG0		0x0058 // RW
#define DEVICE_BRANCH_EN_BIT		0
#define SOFT_BMU_EN_BIT			4
#define TOKEN_FLOW_CTRL_EN_BIT		5
#define PAGE_NUM_BIT			7
#define MIRROR_FRAME_LEN_BIT		16

#define CLS_NPE_TX_DESCR_FIFO_DEPTH	0x0068 // RW
#define CLS_NPE_TX_STATUS_FIFO_DEPTH	0x006C // RW

/* routing reg for to cput part */
#define CLS_NPE_TX_FRAME_TYPE0	0x0070 // RW
#define CLS_NPE_TX_FRAME_TYPE1	0x0074 // RW

/* To CPU Split REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_TX_SPLIT_REG		(0x0104 + 0x50 * LOCAL_CPU) // RW
#define SPLIT_MODE_BIT			0

#define CLS_NPE_TX_HEAD_BLOCK_REG	(0x0108 + 0x50 * LOCAL_CPU) // RW
#define HEAD_BLOCK_OFFSET_BIT		0
#define HEAD_AWCACHE_BIT		16

#define CLS_NPE_TX_BODY_BLOCK_REG	(0x010C + 0x50 * LOCAL_CPU) // RW
#define BODY_BLOCK_OFFSET_BIT		0
#define BODY_AWCACHE_BIT		16

/* To CPU Desc for software BMU REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_TX_DESCR_FIFO_STATUS		(0x0110 + 0x50 * LOCAL_CPU) // RO
#define CLS_NPE_TX_DESCRIPTION0		(0x0114 + 0x50 * LOCAL_CPU) // RW
#define CLS_NPE_TX_DESCRIPTION1		(0x0118 + 0x50 * LOCAL_CPU) // RW

#define CLS_NPE_TX_DESCRIPTION2		(0x011C + 0x50 * LOCAL_CPU) // RW
#define TX_DESCRIPTION2_BIT			0 // TX SW-BMU PKT ID
#define TX_DESCR_CFG_EN_BIT			16 // W1P TX Descr Enable

/* To CPU Status Queue mng REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_TX_QUEUE_STATUS0		(0x0120 + 0x50 * LOCAL_CPU) // RO
#define TX_QUEUE_SIZE_BIT			0
#define TX_STATUS_INVALID_BIT			13
#define TX_PKT_ERR_BIT				14
#define IS_SPLIT_BIT				15 // 1: split
#define TX_PACKET_ID				16

#define CLS_NPE_TX_QUEUE_DEL_INSTR		(0x0128 + 0x50 * LOCAL_CPU) // RW
#define CLS_NPE_TX_QUEUE_HOLD_INSTR		(0x012C + 0x50 * LOCAL_CPU) // RW

/* To CPU Interrupt REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_TX_INTERRUPT_EN		(0x0134 + 0x50 * LOCAL_CPU) // RW

/* To CPU Desc REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_RX_DESCR_FIFO_DEPTH		0x0A40
#define CLS_NPE_RX_STATUS_FIFO_DEPTH		0x0A44 // RW

#define CLS_NPE_RX_DESCR_FIFO_STATUS		(0x0A4C + 0x20 * LOCAL_CPU) // RO
#define CLS_NPE_RX_DESCRIPTION0		(0x0A50 + 0x20 * LOCAL_CPU) // RW
#define CLS_NPE_RX_DESCRIPTION1		(0x0A54 + 0x20 * LOCAL_CPU) // RW
#define RX_PKT_LEN_BIT				0
#define TAG_IND_BIT				15
#define RX_PKT_ID_BIT				16
#define RX_DESCR_CFG_EN				31

/* To CPU Status Queue REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_RX_QUEUE_STATUS		(0x0A58 + 0x20 * LOCAL_CPU) // RO
#define RX_QUEUE_QSIZE_BIT			0
#define RX_PKT_ERR_BIT				12
#define RX_QUEUE_PKT_BIT			16
#define RX_STATUS_INVALID			31

#define CLS_NPE_RX_QUEUE_DEL_INSTR		(0x0A5C + 0x20 * LOCAL_CPU) // RW
#define RX_QUEUE_DEL_NUM_BIT			0
#define RX_QUEUE_DEL_EN_BIT			12

/* To CPU Interrupt REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define CLS_NPE_RX_INTERRUPT_EN		(0x0A60 + 0x20 * LOCAL_CPU) // RW

#define CLS_NPE_MIRROR_FRAME_MAX_LEN		1600

#define CLS_NPE_AWCACHE_EN    0xF // 4'b1111
#define CLS_NPE_AWCACHE_DIS   0x3 // 4'b0011

int cls_edma_bmu_config(struct udevice *dev, u32 pkt_id, u32 head_base_addr, u32 body_base_addr);
int cls_eth_edma_init(struct udevice *dev);
#endif
