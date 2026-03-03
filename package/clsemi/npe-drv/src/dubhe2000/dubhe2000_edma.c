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

/* e1000_edma.c
 * Shared functions for accessing and configuring the EDMA
 */

#include "dubhe2000_edma.h"
#if defined(__ENABLE_FWD_TEST__)
#include "dubhe2000_fwd_test.h"
#endif

void __iomem *conf_base_addr = NULL;
void __iomem *debug_base_addr = NULL;

/* four DMAC Range Group
 * Mask 1: not care
 */
s32 dubhe1000_2cpu_dmac_range_add(s32 group, u32 left_lo, u32 left_hi, u32 right_lo, u32 right_hi, u32 mask_lo,
				  u32 mask_hi)
{
	// to CPU MAC Range (m: 0 ~ 3)
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE0, group * 0x20, left_lo);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE1, group * 0x20, left_hi);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE2, group * 0x20, right_lo);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE3, group * 0x20, right_hi);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_MASK1, group * 0x20, mask_lo);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_MASK2, group * 0x20, mask_hi);

	return 0;
}

s32 dubhe1000_2cpu_dmac_range_del(s32 group)
{
	// to CPU MAC Range (m: 0 ~ 3)
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE0, group * 0x20, 0);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE1, group * 0x20, 0);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE2, group * 0x20, 0);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_RANGE3, group * 0x20, 0);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_MASK1, group * 0x20, 0);
	DUBHE1000_WRITE_REG_ARRAY(TX_DMAC_MASK2, group * 0x20, 0);

	return 0;
}

/* To Host route table(512 entries)
 *      bit0-15: DMAC_High
 *      bit16-17: cfg_route_type
 *          0: Host
 *          1: WIFI_40M
 *          2: WIFI_60M
 *          3: PCIE
 *      bit20:    cfg_instr_type (0: add, 1: del)
 *
 *  return 0: Free or Success, 1: Busy , 2: Failed
 */
s32 dubhe1000_route_config(u32 dmac_lo, u32 dmac_hi, u32 route_type, u32 del)
{
	s32 val = dmac_hi;
	s32 status = 0;

	status = er32(TX_ROUTE_CFG_STATUS);
	// 0: Free or Success, 1: Busy , 2: Failed
	if (status == 1) { // busy
		return status;
	}
	val |= (route_type << CFG_ROUTE_TYPE_BIT);
	val |= (del << CFG_INSTR_TYPE_BIT);
	val |= (1 << ROUTE_CFG_EN_BIT);

	ew32(TX_ROUTE_CFG_INSTR0, dmac_lo);
	ew32(TX_ROUTE_CFG_INSTR1, val);

	// 0: Free or Success, 1: Busy , 2: Failed
	status = er32(TX_ROUTE_CFG_STATUS);

	return status;
}

/* op: 0: init token, 1: free token, 2: supplement token
 * TX_TOKEN_STATUS
 *    bit0: init:
 *       0: Success
 *       1: Busy
 *       2: Failed
 *
 *    bit2: free
 *    bit3: supp
 *
 *  return 0: Success, 1: Failed
 */
s32 dubhe1000_token_config(s32 op, s32 token_id, s32 tokenn_num)
{
	s32 ret = 0;
	s32 status = 0;
	s32 val = 0;

	switch (op) {
	case 0: // init
		val = (tokenn_num << TOKEN_INIT_NUM_BIT);
		val |= (token_id << TOKEN_INIT_TID_BIT);
		val |= (1 << TOKEN_INIT_EN_BIT);
		ew32(TX_TOKEN_INIT, val);
		break;

	case 1: // free
		val = (tokenn_num << TOKEN_INIT_NUM_BIT);
		val |= (token_id << TOKEN_INIT_TID_BIT);
		val |= (1 << TOKEN_INIT_EN_BIT);
		ew32(TX_TOKEN_FREE, val);
		break;

	case 2: // suppl
		val = (tokenn_num << TOKEN_INIT_NUM_BIT);
		val |= (token_id << TOKEN_INIT_TID_BIT);
		val |= (1 << TOKEN_INIT_EN_BIT);
		ew32(TX_TOKEN_SUPPLEMENT, val);
		break;

	default:
		goto err_op;
	}

	status = er32(TX_TOKEN_STATUS);
	ret = status;
err_op:
	ret = 1;
	return ret;
}

s32 dubhe1000_edma_bmu_config(struct dubhe1000_adapter *adapter, u32 pkt_id, u32 head_base_addr, u32 body_base_addr,
			      u32 tx_descr_addr, u32 *tx_descr_num)
{
	s32 val = 0;
	s32 desc_fifo_status = 0;
	static int cnt = 0;

#if EDMA_HW
#ifdef EDMA_HOST_ONLY
	status_0_req_pkt_num = er32(STATUS_0_REQ_PKT_NUM);
	ew32(DDR_STATUS_0_BASE, tx_descr_addr);
	ew32(DDR_STATUS_0_PKT_NUM, tx_descr_num - status_0_req_pkt_num);

	ew32(DDR_STATUS_1_PKT_NUM, 0);
	ew32(DDR_STATUS_2_PKT_NUM, 0);
	ew32(DDR_STATUS_3_PKT_NUM, 0);
#else
	int num = 0, i;

	for (i = 0; i < 4; i++) {
		if (tx_descr_num[i]) {
			ew32_x(i, STATUS_REQ_PKT_NUM, DEFAULT_STATUS_PKT_NUM);
			ew32_x(i, DDR_STATUS_BASE, num + tx_descr_addr);
			ew32_x(i, DDR_STATUS_PKT_NUM, tx_descr_num[i] - DEFAULT_STATUS_PKT_NUM);
			num += tx_descr_num[i] * 8;
		}
	}
#endif

	ew32(RX_RATE_CFG, DEFLAULT_RX_OUT_CYCLE << RX_OUT_CYCLE_BIT | DEFLAULT_RX_RATE_PERIOD);

	/* 1. sw malloc buffer for the rx_ring[i]
	 * 2. writing the buffer address into the TX_DESCRIPTION0/1(head/body base address)
	 */
	if (adapter->soft_bmu_en) {
		desc_fifo_status = er32(TX_DESCR_FIFO_STATUS);
		cnt++;

		/* allocate receive descriptors */
		ew32(TX_DESCRIPTION1, body_base_addr);

		/* 0: no split, 1: tag split, 2: l2 header, 3: l3 header, 4: l4 header*/
		//if(adapter->split_mode)
		{
			ew32(TX_DESCRIPTION0, head_base_addr);
		}

		/* bit 0~15: TX SW-BMU PKT ID
		 * bit16:    W1P TX Descr Enable
		 */
		val = ((1 << TX_DESCR_CFG_EN_BIT) | pkt_id);
		ew32(TX_DESCRIPTION2, val);
	} else {
		/* for the HW BMU case
		 * only config the block size and the base address and the Hardware manage the buffer self,
		 * no SW involve
		 * DUBHE1000_TX_BASE_REG1, for HW BMU only
		 * bit0-12:	head_block_size = 0
		 * bit16-28: body_block_size = 0 or 2048(0x800)  ??? 1600 from hw doc???
		 */
		val = (adapter->body_block_size << BODY_BLOCK_SIZE_BIT);

		// body_base_addr = 0x8000000, for HW BMU only
		ew32(TX_BASE_REG3, body_base_addr); // the buffer base address

		/* 0: no split, 1: tag split, 2: l2 header, 3: l3 header, 4: l4 header*/
		if (adapter->split_mode) {
			// head_base_addr = 0, for HW BMU only
			ew32(TX_BASE_REG2, head_base_addr); // the buffer base address

			val |= (adapter->head_block_size << HEAD_BLOCK_SIZE_BIT);
		}
		ew32(TX_BASE_REG1, val); // block-size for body and head(if split enabled)
	}
#endif

	return 0;
}

void dubhe1000_init_dscp_map(struct dubhe1000_adapter *adapter)
{
	int i;

	memset(adapter->dscp_map, 0, sizeof(adapter->dscp_map));
	for (i = 0; i < 64; ++i) {
		adapter->dscp_map[i].tid = i >> 3;
		adapter->dscp_map[i].queue = i >> 3;
		DUBHE1000_WRITE_REG_ARRAY(DSCP_TID, i * 0x4, i >> 3);
	}
}

void dubhe1000_init_tc_map(struct dubhe1000_adapter *adapter)
{
	int i;

	memset(adapter->tc_map, 0, sizeof(adapter->tc_map));
	for (i = 0; i < 256; ++i) {
		adapter->tc_map[i].tid = i >> 5;
		adapter->tc_map[i].queue = i >> 5;
		DUBHE1000_WRITE_REG_ARRAY(TC_TID, i * 0x4, i >> 5);
	}
}

void dunhe2000_edma_port_bypass_init(struct dubhe1000_adapter *adapter)
{
	int index;

	/* Note:
	 * 1. PORT_BYPASS will control cpu-meta routing/TID_BYPASS;
	 * ROUTE_BYPASS will select mac routing or cpu-meta routing;
	 * TID_BYPASS will control tid-mapping function.
	 * The PORT_BYPASS/ROUTE_BYPASS/TID_BYPASS need to be consistent in meaning
	 * 2. DEF_TID only work when TID_BYPASS = 0 (tid-mapping enable) and fail to mapping
	 */
	//PORT_BYPASS: 0 means enable cputag-meta routing and tid mapping function
	ew32(PORT_BYPASS, 0);

	//ROUTE_BYPASS: 1 means cputag-meta routing will work, not mac route
	ew32(ROUTE_BYPASS, 1);

	//TID_BYPASS: 0 means dot1p/dscp/tc imapping will work
	ew32(TID_BYPASS, 0);

	//DEFAULT_TID: TID0
	ew32(DEF_TID, 0);

	//QOS_SEL: who first? DOT1P or DSCP/TC
	ew32(QOS_SEL, 1); // the priority of DSCP/TC is higher than Dot1p

	//DOT1P_TID: dot1p in vlan header to TID
	for (index = 0; index < 8; index++)
		DUBHE1000_WRITE_REG_ARRAY(DOT1P_TID, index * 0x4, index & 0x7);

	dubhe1000_init_dscp_map(adapter);
	dubhe1000_init_tc_map(adapter);
}

/**
 * dubhe1000_init_edma - Performs basic configuration of the edma.
 *
 */
s32 dubhe1000_edma_init(struct dubhe1000_adapter *adapter)
{
	// bit0: tx_module_en(to cpu), bit1: rx_module_en(from cpu)
	u32 module_enable = 3;
	u32 val = 0;
	u32 token_id = 0;

	debug_base_addr = adapter->edma_dbg_regs;
	conf_base_addr = adapter->edma_regs;

#if EDMA_HW
	/* 2. Config BMU mode(Software BMU or HW BMU)
	 * DUBHE1000_TX_BASE_REG0
	 * bit0-2:	device_branch_en  bit0: wifi40M en, bit1: wifi160M en, bit2: PCIE en
	 * bit4:	soft_bmu_en
	 * bit5:	token_flow_ctrl_en
	 * bit7-15:	page_num
	 * bit16-31:	mirror_frame_len (1600 byte )
	 */
	int i;

	for (i = 1; i < 4; i++)
		if (adapter->txq_status_num[i])
			val |= BIT(i - 1); /* Enable wifi40M,wifi160M,PCIE */

	printk(KERN_ERR "====>Enable EDMA %d\n", val);

	val |= (adapter->soft_bmu_en << SOFT_BMU_EN_BIT); // enable SW BMU
#ifdef ENABLE_EDMA_TOKEN
	val |= (adapter->token_fc_en << TOKEN_FLOW_CTRL_EN_BIT); // enable Token Flow Contrl
#endif
	if (!adapter->soft_bmu_en)
		val |= (adapter->page_num << PAGE_NUM_BIT); // enable Token Flow Contrl

	/* mirror frame len: 0x640=1600 */
	val |= (DUBHE1000_MIRROR_FRAME_MAX_LEN << MIRROR_FRAME_LEN_BIT);
	ew32(TX_BASE_REG0, val);

	// 3. Paket Type TO CPU
#ifdef EDMA_HOST_ONLY
	ew32(TX_FRAME_TYPE0, 0x3FFFFF);
	ew32(TX_FRAME_TYPE1, 0xF);
#else
	ew32(TX_FRAME_TYPE1, 0x1);
	ew32(TX_FRAME_TYPE1, 0x0);
#endif

	// 4. Forwarding Table

	// 5. Head & Body SPLIT
	ew32(TX_SPLIT_REG, adapter->split_mode); // disable split

	if (adapter->split_mode) { // split enable
		/* DUBHE1000_TX_HEAD_BLOCK_REG
		 * bit0-11:  head_block_offset = DUBHE1000_HEADROOM
		 * bit16-19: head_awcache
		 */
		if (adapter->head_awcachable)
			val = (DUBHE1000_AWCACHE_EN << HEAD_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << HEAD_AWCACHE_BIT);

		// bit0-11:  head_block_offset = DUBHE1000_HEADROOM
		val |= (adapter->head_offset + DUBHE1000_HEADROOM);
		ew32(TX_HEAD_BLOCK_REG, val);

		/* DUBHE1000_TX_BODY_BLOCK_REG
		 * bit0-11:  body_block_offset = 192(0xC0)
		 * bit16-19: body_awcache
		 */
		if (adapter->body_awcachable)
			val = (DUBHE1000_AWCACHE_EN << BODY_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << BODY_AWCACHE_BIT);

		val |= (adapter->body_offset << BODY_BLOCK_OFFSET_BIT);
		ew32(TX_BODY_BLOCK_REG, val);
	} else { // split disable
		/* DUBHE1000_TX_HEAD_BLOCK_REG
		 * bit0-11:  head_block_offset = 0
		 * bit16-19: head_awcache
		 */
		if (adapter->head_awcachable)
			val = (DUBHE1000_AWCACHE_EN << HEAD_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << HEAD_AWCACHE_BIT);

		ew32(TX_HEAD_BLOCK_REG, val);

		/* DUBHE1000_TX_BODY_BLOCK_REG
		 * bit0-11:  body_block_offset = 192(0xC0)
		 * bit16-19: body_awcache
		 */
		if (adapter->body_awcachable)
			val = (DUBHE1000_AWCACHE_EN << BODY_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << BODY_AWCACHE_BIT);

		// bit0-11:  body_block_offset = DUBHE1000_HEADROOM
		val |= (adapter->body_offset + DUBHE1000_HEADROOM);

		ew32(TX_BODY_BLOCK_REG, val);

#if defined(__ENABLE_FWD_TEST__)
		ew32_x(1, TX_BODY_BLOCK_REG, (val & ~0xfff) | 0x10);
		ew32_x(2, TX_BODY_BLOCK_REG, (val & ~0xfff) | 0x20);
		ew32_x(3, TX_BODY_BLOCK_REG, (val & ~0xfff) | 0x30);
#else
		ew32_x(1, TX_BODY_BLOCK_REG, val);
		ew32_x(2, TX_BODY_BLOCK_REG, val);
		ew32_x(3, TX_BODY_BLOCK_REG, val);
#endif
	}

	// 6. QoS Control -- Token Init (0 ~ 511)
	for (token_id = 0; token_id < 512; token_id++) {
#ifdef ENABLE_EDMA_TOKEN
#if !defined(__ENABLE_FWD_TEST__)
		val = (0x0FFF << TOKEN_INIT_NUM_BIT);
		val |= (token_id << TOKEN_INIT_TID_BIT);
		val |= (1 << TOKEN_INIT_EN_BIT);
		ew32(TX_TOKEN_INIT, val);
#endif
#endif
	}

	dunhe2000_edma_port_bypass_init(adapter);

	// 7. enable EDMA module
	ew32(MODULE_ENABLE, module_enable);

	// 8. enable RX/TX Interrupt
	ew32(RX_INTERRUPT_EN, 1);
	ew32(TX_INTERRUPT_EN, 1); // enable tx interrupt
#endif

	return 0;
}

s32 dubhe1000_edma_destroy(void)
{
	// 1. disable EDMA module
	ew32(MODULE_ENABLE, 0);

	// 2. free related Memory

	return 0;
}
