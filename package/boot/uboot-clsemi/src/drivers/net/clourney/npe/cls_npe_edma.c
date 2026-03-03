// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2006 Intel Corporation. */

#include "cls_npe_edma.h"

int cls_edma_bmu_config(struct udevice *dev, u32 pkt_id, u32 head_base_addr, u32 body_base_addr)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	s32 val = 0;
	s32 desc_fifo_status = 0;

	if (priv_data->soft_bmu_en) {
		desc_fifo_status = cls_eth_r32(base_addr, CLS_NPE_TX_DESCR_FIFO_STATUS);
		if (desc_fifo_status == (priv_data->lhost_fifo_depth * CLS_NPE_FIFO_DEPTH_LEVEL)) {
			cls_eth_print("[%s] ERR! invalid rx_fifo_size: depth=%d\n",
					__func__, priv_data->lhost_fifo_depth);
			return -1;
		}

		// writing the buffer address into the TX_DESCRIPTION0/1(head/body base address)
		cls_eth_w32(base_addr, CLS_NPE_TX_DESCRIPTION1, body_base_addr);
		cls_eth_w32(base_addr, CLS_NPE_TX_DESCRIPTION0, head_base_addr);

		/* bit 0~15: TX SW-BMU PKT ID
		 * bit16:    W1P TX Descr Enable
		 */
		val = ((1 << TX_DESCR_CFG_EN_BIT) | pkt_id);
		cls_eth_w32(base_addr, CLS_NPE_TX_DESCRIPTION2, val);
	} else {
		cls_eth_print("[%s] ERR! token_fc_en = 1!\n", __func__);
		return -1;
	}

	return 0;
}

int cls_eth_edma_init(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	u8 fifo_depth = priv_data->lhost_fifo_depth;// 8 bits
	bool soft_bmu_en = priv_data->soft_bmu_en;
	bool token_fc_en = priv_data->token_fc_en;
	u8 split_mode = priv_data->split_mode;
	u32 val = 0, body_offset = 0;

	/* 1. Configure fifo depth */
	cls_eth_w32(base_addr, CLS_NPE_RX_DESCR_FIFO_DEPTH, fifo_depth);
	cls_eth_w32(base_addr, CLS_NPE_RX_STATUS_FIFO_DEPTH, fifo_depth);
	cls_eth_w32(base_addr, CLS_NPE_TX_DESCR_FIFO_DEPTH, fifo_depth);
	cls_eth_w32(base_addr, CLS_NPE_TX_STATUS_FIFO_DEPTH, fifo_depth);

	/* 2. Config soft BMU mode
	 *
	 * CLS_NPE_TX_BASE_REG0
	 *    bit0-2:	device_branch_en bit0: wifi40M en, bit1: wifi160M en, bit2: PCIE en
	 *    bit4:	soft_bmu_en
	 *    bit5:	token_flow_ctrl_en
	 *    bit6:	range_mask_route_en
	 *    bit7-15:	page_num(soft bmu disable)
	 *    bit16-31:	mirror_frame_len (1600 byte)
	 */
	val = 3;
	val |= (soft_bmu_en << SOFT_BMU_EN_BIT);// enable SW BMU
	val |= (token_fc_en << TOKEN_FLOW_CTRL_EN_BIT);// enable Token Flow Contrl
	val |= (CLS_NPE_MIRROR_FRAME_MAX_LEN << MIRROR_FRAME_LEN_BIT);
	cls_eth_w32(base_addr, CLS_NPE_TX_BASE_REG0, val);

	/* 3. Paket Type TO CPU */
	cls_eth_w32(base_addr, CLS_NPE_TX_FRAME_TYPE0, 0x3FFFFF);
	cls_eth_w32(base_addr, CLS_NPE_TX_FRAME_TYPE1, 0xF);

	/* 4. Forwarding Table */
	// Unsupported!

	/* 5. Head & Body SPLIT */
	cls_eth_w32(base_addr, CLS_NPE_TX_SPLIT_REG, split_mode);
	if (split_mode) {// Unsupported!
		cls_eth_print("[%s] ERR! split_mode = 1!\n", __func__);
		return -1;
	} else {
		/* CLS_NPE_TX_HEAD_BLOCK_REG
		 *    bit0-11:  head_block_offset = 0
		 *    bit16-19: head_awcache
		 */
		val = (CLS_NPE_AWCACHE_DIS << HEAD_AWCACHE_BIT);
		cls_eth_w32(base_addr, CLS_NPE_TX_HEAD_BLOCK_REG, val);

		/* CLS_NPE_TX_BODY_BLOCK_REG
		 *    bit0-11:  body_block_offset = 192(0xC0)
		 *    bit16-19: body_awcache
		 */
		val = (CLS_NPE_AWCACHE_EN << BODY_AWCACHE_BIT);
		val |= body_offset;
		cls_eth_w32(base_addr, CLS_NPE_TX_BODY_BLOCK_REG, val);
	}

	/* 6. QoS Control */
	if (token_fc_en) {// Unsupported!
		cls_eth_print("[%s] ERR! token_fc_en = 1!\n", __func__);
		return -1;
	}

	cls_eth_w32(base_addr, CLS_NPE_MODULE_ENABLE, CLS_NPE_ENABLE_EDMA_TX_RX);

	/* 7. Disable RX/TX Interrupt */
	cls_eth_w32(base_addr, CLS_NPE_RX_INTERRUPT_EN, 0);
	cls_eth_w32(base_addr, CLS_NPE_TX_INTERRUPT_EN, 0);

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] done!\n", __func__);
#endif

	return 0;
}
