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

/* dubhe1000_edma.h
 * Structures, enums, and macros for the EDMA
 */

#ifndef _DUBHE1000_EDMA_H_
#define _DUBHE1000_EDMA_H_

#include "dubhe2000_osdep.h"
#include "dubhe2000.h"

#define CPU_LHOST  0
#define CPU_2DOT4G 1
#define CPU_5G	   2
#define CPU_PCIE   3

#define LOCAL_CPU CPU_LHOST

#define DUBHE1000_FIFO_DEPTH_LEVEL 16

/* 1. CONFIG REG */

#define DUBHE1000_MODULE_ENABLE 0x0000 // RW
#define TX_MODULE_EN_BIT	0
#define RX_MODULE_EN_BIT	1

#define DUBHE1000_BUS_CONFIG		     0x0004 // RW
#define DUBHE1000_CFG_ALM_INTR_EN	     0x0008 // RW
#define DUBHE1000_TX_TOKEN_ERROR_ALM_INTR_EN 0x000C // RW
#define DUBHE1000_TX_TIMEOUT_CFG	     0x004C // RW
#define DUBHE1000_RX_TIMEOUT_CFG	     0x0050 // RW
#define DUBHE1000_STAT_CNT_EN		     0x0054 // RW

/* TX REGISTER: to cpu part */
#define DUBHE1000_TX_BASE_REG0 0x0058 // RW
#define DEVICE_BRANCH_EN_BIT   0
#define SOFT_BMU_EN_BIT	       4
#define TOKEN_FLOW_CTRL_EN_BIT 5
#define PAGE_NUM_BIT	       7
#define MIRROR_FRAME_LEN_BIT   16

#define DUBHE1000_TX_BASE_REG1 0x005C // RW
#define HEAD_BLOCK_SIZE_BIT    0
#define BODY_BLOCK_SIZE_BIT    16

#define DUBHE1000_TX_BASE_REG2 0x0060 // RW
#define DUBHE1000_TX_BASE_REG3 0x0064 // RW

#define DUBHE1000_TX_DESCR_FIFO_DEPTH  0x0068 // RW
#define DUBHE1000_TX_STATUS_FIFO_DEPTH 0x006C // RW

/* routing reg for to cput part */
#define DUBHE1000_TX_FRAME_TYPE0 0x0070 // RW
#define DUBHE1000_TX_FRAME_TYPE1 0x0074 // RW

/* routing to Host REG, with m=4 groups */
#define DUBHE1000_TX_DMAC_RANGE0 0x0078 // RW
#define DUBHE1000_TX_DMAC_RANGE1 0x007C // RW
#define DUBHE1000_TX_DMAC_RANGE2 0x0080 // RW
#define DUBHE1000_TX_DMAC_RANGE3 0x0084 // RW

#define DUBHE1000_TX_DMAC_MASK1 0x0088 // RW
#define DUBHE1000_TX_DMAC_MASK2 0x008C // RW

/* To CPU routing */
#define DUBHE1000_TX_ROUTE_CFG_INSTR0 0x00F8 // RW
#define CFG_DMAC_LO_BIT		      0

#define DUBHE1000_TX_ROUTE_CFG_INSTR1 0x00FC // RW
#define CFG_DMAC_HI_BIT		      0
/*
 *   0: Host
 *   1: WIFI_40M
 *   2: WIFI_60M
 *   3: PCIE
 */
#define CFG_ROUTE_TYPE_BIT	      16
#define CFG_INSTR_TYPE_BIT	      20 // 0: add, 1: del
#define ROUTE_CFG_EN_BIT	      21 // W1P

#define DUBHE1000_TX_ROUTE_CFG_STATUS 0x0100 // RO

/* To CPU Split REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_TX_SPLIT_REG (0x0104 + 0x50 * LOCAL_CPU) // RW
#define SPLIT_MODE_BIT	       0

#define DUBHE1000_TX_HEAD_BLOCK_REG (0x0108 + 0x50 * LOCAL_CPU) // RW
#define HEAD_BLOCK_OFFSET_BIT	    0
#define HEAD_AWCACHE_BIT	    16

#define DUBHE1000_TX_BODY_BLOCK_REG (0x010C + 0x50 * LOCAL_CPU) // RW
#define BODY_BLOCK_OFFSET_BIT	    0
#define BODY_AWCACHE_BIT	    16

/* To CPU Desc for software BMU REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_TX_DESCR_FIFO_STATUS (0x0110 + 0x50 * LOCAL_CPU) // RO
#define DUBHE1000_TX_DESCRIPTION0      (0x0114 + 0x50 * LOCAL_CPU) // RW
#define DUBHE1000_TX_DESCRIPTION1      (0x0118 + 0x50 * LOCAL_CPU) // RW

#define DUBHE1000_TX_DESCRIPTION2 (0x011C + 0x50 * LOCAL_CPU) // RW
#define TX_DESCRIPTION2_BIT	  0			      // TX SW-BMU PKT ID
#define TX_DESCR_CFG_EN_BIT	  16			      // W1P TX Descr Enable

/* To CPU Status Queue mng REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_TX_QUEUE_STATUS0 (0x0120 + 0x50 * LOCAL_CPU) // RO
#define TX_QUEUE_SIZE_BIT	   0
#define TX_STATUS_INV_BIT	   13
#define TX_PKT_ERR_BIT		   14
#define IS_SPLIT_BIT		   15 // 1: split
#define TX_PACKET_ID		   16

#define DUBHE1000_TX_QUEUE_STATUS1 (0x0124 + 0x50 * LOCAL_CPU) // RO
#define TX_BUF_BID_BIT		   0			       // block id
#define TX_BUF_PID_BIT		   8			       // page id
#define TX_PACKET_TOKEN_ID_BIT	   17			       // packet token id
#define TX_BLK_NUM_BIT		   26			       // Block num
#define TX_TAG_QOS_TID_BIT	   29			       // QoS TID

#define DUBHE1000_TX_QUEUE_DEL_INSTR  (0x0128 + 0x50 * LOCAL_CPU) // RW
#define DUBHE1000_TX_QUEUE_HOLD_INSTR (0x012C + 0x50 * LOCAL_CPU) // RW
#define DUBHE1000_TX_BUF_FREE_INSTR   (0x0130 + 0x50 * LOCAL_CPU) // RW
#define FREE_TX_BUF_BID_BIT	      0				  // block id
#define FREE_TX_BUF_PID_BIT	      8				  // page id
#define FREE_TX_BUF_TOKEN_ID_BIT      17			  // packet token id
#define FREE_TX_BLK_NUM_BIT	      26			  // Block num
#define FREE_TX_BUF_FREE_EN_BIT	      31			  // tx_buf_free_en

/* To CPU Interrupt REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_TX_INTERRUPT_EN	(0x0134 + 0x50 * LOCAL_CPU) // RW
#define DUBHE1000_TX_INTERRUPT_RAW_RPT	(0x0138 + 0x50 * LOCAL_CPU) // WC
#define DUBHE1000_TX_INTERRUPT_MASK_RPT (0x013C + 0x50 * LOCAL_CPU) // WC

/* To CPU Token mng REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_TX_TOKEN_INIT (0x0140 + 0x50 * LOCAL_CPU) // RW
#define TOKEN_INIT_NUM_BIT	0
#define TOKEN_INIT_TID_BIT	12
#define TOKEN_INIT_EN_BIT	21

#define DUBHE1000_TX_TOKEN_FREE	      (0x0144 + 0x50 * LOCAL_CPU) // RW
#define DUBHE1000_TX_TOKEN_SUPPLEMENT (0x0148 + 0x50 * LOCAL_CPU) // RW

#define DUBHE1000_TX_TOKEN_STATUS (0x014C + 0x50 * LOCAL_CPU) // RO
#define TOKEN_INIT_STATUS_BIT	  0
#define TOKEN_FREE_STATUS_BIT	  2
#define TOKEN_SUPP_STATUS_BIT	  4

/* To CPU Token Counter Report, 512 REGs */
#define DUBHE1000_TX_TOKEN_COUNTER 0x0240 // RO

/* To CPU Desc REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_RX_DESCR_FIFO_DEPTH 0x0A40

#define DUBHE1000_RX_STATUS_FIFO_DEPTH 0x0A44 // RW
#define DUBHE1000_RX_RATE_CFG	       0x0A48 //RW
#define DEFLAULT_RX_RATE_PERIOD	       0xf
#define DEFLAULT_RX_OUT_CYCLE	       0x4
#define RX_RATE_PERIOD_BIT	       0
#define RX_OUT_CYCLE_BIT	       16
#define DUBHE1000_RX_DESCR_FIFO_STATUS (0x0A4C + 0x20 * LOCAL_CPU) // RO
#define DUBHE1000_RX_DESCRIPTION0      (0x0A50 + 0x20 * LOCAL_CPU) // RW
#define DUBHE1000_RX_DESCRIPTION1      (0x0A54 + 0x20 * LOCAL_CPU) // RW
#define RX_PKT_LEN_BIT		       0
#define TAG_IND_BIT		       15
#define RX_PKT_ID_BIT		       16
#define RX_DESCR_CFG_EN		       31

/* To CPU Status Queue REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_RX_QUEUE_STATUS (0x0A58 + 0x20 * LOCAL_CPU) // RO
#define RX_QUEUE_QSIZE_BIT	  0
#define RX_PKT_ERR_BIT		  12
#define RX_QUEUE_PKT_BIT	  16
#define RX_STATUS_INV_BIT	  31

#define DUBHE1000_RX_QUEUE_DEL_INSTR (0x0A5C + 0x20 * LOCAL_CPU) // RW
#define RX_QUEUE_DEL_NUM_BIT	     0
#define RX_QUEUE_DEL_EN_BIT	     12

/* To CPU Interrupt REG
 * these REG for each cpu respectivly, (ext. LHost, 2.4G, 5G, PCIE)
 */
#define DUBHE1000_RX_INTERRUPT_EN	(0x0A60 + 0x20 * LOCAL_CPU) // RW
#define DUBHE1000_RX_INTERRUPT_RAW_RPT	(0x0A64 + 0x20 * LOCAL_CPU) // WC
#define DUBHE1000_RX_INTERRUPT_MASK_RPT (0x0A68 + 0x20 * LOCAL_CPU) // RO

#define DUBHE1000_TX_SPLIT_REG_X(cpu_id)	 (DUBHE1000_TX_SPLIT_REG + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_HEAD_BLOCK_REG_X(cpu_id)	 (DUBHE1000_TX_HEAD_BLOCK_REG + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_BODY_BLOCK_REG_X(cpu_id)	 (DUBHE1000_TX_BODY_BLOCK_REG + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_DESCR_FIFO_STATUS_X(cpu_id) (DUBHE1000_TX_DESCR_FIFO_STATUS + (0x50 * ((cpu_id)-LOCAL_CPU))) // RO
#define DUBHE1000_TX_DESCRIPTION0_X(cpu_id)	 (DUBHE1000_TX_DESCRIPTION0 + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_DESCRIPTION1_X(cpu_id)	 (DUBHE1000_TX_DESCRIPTION1 + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_DESCRIPTION2_X(cpu_id)	 (DUBHE1000_TX_DESCRIPTION2 + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_QUEUE_STATUS0_X(cpu_id)	 (DUBHE1000_TX_QUEUE_STATUS0 + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RO
#define DUBHE1000_TX_QUEUE_STATUS1_X(cpu_id)	 (DUBHE1000_TX_QUEUE_STATUS1 + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RO
#define DUBHE1000_TX_QUEUE_DEL_INSTR_X(cpu_id)	 (DUBHE1000_TX_QUEUE_DEL_INSTR + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_QUEUE_HOLD_INSTR_X(cpu_id)	 (DUBHE1000_TX_QUEUE_HOLD_INSTR + (0x50 * ((cpu_id)-LOCAL_CPU)))  // RW
#define DUBHE1000_TX_BUF_FREE_INSTR_X(cpu_id)	 (DUBHE1000_TX_BUF_FREE_INSTR + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_INTERRUPT_EN_X(cpu_id)	 (DUBHE1000_TX_INTERRUPT_EN + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_INTERRUPT_RAW_RPT_X(cpu_id) (DUBHE1000_TX_INTERRUPT_RAW_RPT + (0x50 * ((cpu_id)-LOCAL_CPU))) // WC
#define DUBHE1000_TX_INTERRUPT_MASK_RPT_X(cpu_id)                                                                      \
	(DUBHE1000_TX_INTERRUPT_MASK_RPT + (0x50 * ((cpu_id)-LOCAL_CPU)))					  // WC
#define DUBHE1000_TX_TOKEN_INIT_X(cpu_id)	 (DUBHE1000_TX_TOKEN_INIT + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_TOKEN_FREE_X(cpu_id)	 (DUBHE1000_TX_TOKEN_FREE + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_TX_TOKEN_SUPPLEMENT_X(cpu_id)	 (DUBHE1000_TX_TOKEN_SUPPLEMENT + (0x50 * ((cpu_id)-LOCAL_CPU)))  // RW
#define DUBHE1000_TX_TOKEN_STATUS_X(cpu_id)	 (DUBHE1000_TX_TOKEN_STATUS + (0x50 * ((cpu_id)-LOCAL_CPU)))	  // RO
#define DUBHE1000_RX_DESCR_FIFO_STATUS_X(cpu_id) (DUBHE1000_RX_DESCR_FIFO_STATUS + (0x20 * ((cpu_id)-LOCAL_CPU))) // RO
#define DUBHE1000_RX_DESCRIPTION0_X(cpu_id)	 (DUBHE1000_RX_DESCRIPTION0 + (0x20 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_RX_DESCRIPTION1_X(cpu_id)	 (DUBHE1000_RX_DESCRIPTION1 + (0x20 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_RX_QUEUE_STATUS_X(cpu_id)	 (DUBHE1000_RX_QUEUE_STATUS + (0x20 * ((cpu_id)-LOCAL_CPU)))	  // RO
#define DUBHE1000_RX_QUEUE_DEL_INSTR_X(cpu_id)	 (DUBHE1000_RX_QUEUE_DEL_INSTR + (0x20 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_RX_INTERRUPT_EN_X(cpu_id)	 (DUBHE1000_RX_INTERRUPT_EN + (0x20 * ((cpu_id)-LOCAL_CPU)))	  // RW
#define DUBHE1000_RX_INTERRUPT_RAW_RPT_X(cpu_id) (DUBHE1000_RX_INTERRUPT_RAW_RPT + (0x20 * ((cpu_id)-LOCAL_CPU))) // WC
#define DUBHE1000_RX_INTERRUPT_MASK_RPT_X(cpu_id)                                                                      \
	(DUBHE1000_RX_INTERRUPT_MASK_RPT + (0x20 * ((cpu_id)-LOCAL_CPU))) // RO

#define DUBHE1000_ALM_INTR_EN		  0x0ACC // RW
#define TX_ALM_INTR_EN_BIT		  0
#define RX_ALM_INTR_EN_BIT		  1
#define DEFAULT_STATUS_PKT_NUM		  0x40
#define DUBHE1000_DDR_STATUS_BASE_X(i)	  (0x000B0C + (i * 4)) // DDR base address of status X
#define DUBHE1000_DDR_STATUS_PKT_NUM_X(i) (0x001548 + (i * 4)) // Number of pkts in DDR of status X
#define DUBHE1000_STATUS_REQ_PKT_NUM_X(i) (0x001568 + (i * 4)) // Number of pkts requesting to send R/W DDR in status X

#define DUBHE1000_DDR_STATUS_0_BASE    0x000B0C // DDR base address of status 0
#define DUBHE1000_DDR_STATUS_0_PKT_NUM 0x001548 // Number of packets in DDR of status 0
#define DUBHE1000_DDR_STATUS_1_BASE    0x000B10 // DDR base address of status 1
#define DUBHE1000_DDR_STATUS_1_PKT_NUM 0x00154C // Number of packets in DDR of status 1
#define DUBHE1000_DDR_STATUS_2_BASE    0x000B14 // DDR base address of status 2
#define DUBHE1000_DDR_STATUS_2_PKT_NUM 0x001550 // Number of packets in DDR of status 2
#define DUBHE1000_DDR_STATUS_3_BASE    0x000B18 // DDR base address of status 3
#define DUBHE1000_DDR_STATUS_3_PKT_NUM 0x001554 // Number of packets in DDR of status 3
#define DUBHE1000_DEF_TID	       0x00155C // Default tid
#define DUBHE1000_PORT_BYPASS	       0x000B04 // Port bypass
#define DUBHE1000_QOS_SEL	       0x001544 // Source of QoS selection
#define DUBHE1000_ROUTE_BYPASS	       0x000B00 // MAC routing bypass
#define DUBHE1000_SHORT_PKT_DROP_EN    0x001564 // Enable dropping packets that less than 60B
#define DUBHE1000_STATUS_0_REQ_PKT_NUM 0x001568 // Number of packets requesting to send R/W DDR in status 0
#define DUBHE1000_STATUS_1_REQ_PKT_NUM 0x00156C // Number of packets requesting to send R/W DDR in status 1
#define DUBHE1000_STATUS_2_REQ_PKT_NUM 0x001570 // Number of packets requesting to send R/W DDR in status 2
#define DUBHE1000_STATUS_3_REQ_PKT_NUM 0x001574 // Number of packets requesting to send R/W DDR in status 3
#define DUBHE1000_DOT1P_TID	       0x000B1C // Mapping between 802.1P PCP and TID
#define DUBHE1000_DSCP_TID	       0x000B3C // Mapping between IPv4 DSCP and TID
#define DUBHE1000_TC_TID	       0x000C3C // Mapping between IPv6 Traffic Class and TID
#define DUBHE1000_TID_BYPASS	       0x001558

s32 dubhe1000_edma_init(struct dubhe1000_adapter *adapter);
s32 dubhe1000_edma_destroy(void);
s32 dubhe1000_edma_bmu_config(struct dubhe1000_adapter *adapter, u32 pkt_id, u32 head_base_addr, u32 body_base_addr,
			      u32 tx_descr_addr, u32 *tx_descr_num);
s32 dubhe1000_route_config(u32 dmac_lo, u32 dmac_hi, u32 route_type, u32 del);

s32 dubhe1000_2cpu_dmac_range_del(s32 group);
s32 dubhe1000_2cpu_dmac_range_add(s32 group, u32 left_lo, u32 left_hi, u32 right_lo, u32 right_hi, u32 mask_lo,
				  u32 mask_hi);

s32 dubhe1000_token_config(s32 op, s32 token_id, s32 tokenn_num);

void dubhe1000_init_dscp_map(struct dubhe1000_adapter *adapter);
void dubhe1000_init_tc_map(struct dubhe1000_adapter *adapter);

#endif /* _DUBHE1000_EDMA_H_ */
