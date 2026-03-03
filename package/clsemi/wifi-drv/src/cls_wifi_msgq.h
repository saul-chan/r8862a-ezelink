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

#ifndef _CLS_WIFI_MSGQ_H_
#define _CLS_WIFI_MSGQ_H_

#define CLS_WIFI_MSGQ_TOTAL_MAX			1536
#define CLS_WIFI_MSGQ_SINGLE_MAX		1024
#define CLS_WIFI_MSGQ_PEEK_INVALID_MAX		255

#define CLS_WIFI_MSGQ_QUE_NB			6
#define CLS_WIFI_MSGQ_BMU_LPOOL_NB		4
#define CLS_WIFI_MSGQ_BMU_PPOOL_NB		16

#define CLS_WIFI_MSGQ_QUE_SIZE			0x00004000
#define CLS_WIFI_MSGQ_REG_SIZE			0x00000004
#define CLS_WIFI_MSGQ_BMU_SIZE			0x00001000

#define CLS_WIFI_MSGQ_PUSH_OFFSET		0x00000000
#define CLS_WIFI_MSGQ_PEEK_OFFSET		0x00001000
#define CLS_WIFI_MSGQ_POP_OFFSET		0x00002000

#define CLS_WIFI_MSGQ_BMU_OFFSET		0x00018000
#define CLS_WIFI_MSGQ_CFG_OFFSET		0x0001c000

#define CLS_WIFI_MSGQ_CFG_LOCK_OFFSET		0x00000000
#define CLS_WIFI_MSGQ_CTRL_OFFSET		0x00000004
#define CLS_WIFI_MSGQ_QUE_CTRL_OFFSET		0x00000010
#define CLS_WIFI_MSGQ_QUE_STATUS_OFFSET		0x00000030
#define CLS_WIFI_MSGQ_INT_CTRL_OFFSET		0x00000070
#define CLS_WIFI_MSGQ_INT_RAW_OFFSET		0x00000200
#define CLS_WIFI_MSGQ_INT_MASK_OFFSET		0x00000204
#define CLS_WIFI_MSGQ_INT_STATUS_OFFSET		0x00000208
#define CLS_WIFI_MSGQ_BMU_P_CTRL_OFFSET		0x00000400
#define CLS_WIFI_MSGQ_BMU_P_END_OFFSET		0x00000500
#define CLS_WIFI_MSGQ_BMU_L_CTRL_OFFSET		0x00000600

#define CLS_WIFI_MSGQ_INT_MASK_VAL		0x00000004

#define CLS_WIFI_MSGQ_QUE_CTRL_REG(_que_idx) \
	(CLS_WIFI_MSGQ_CFG_OFFSET + CLS_WIFI_MSGQ_QUE_CTRL_OFFSET \
	+ CLS_WIFI_MSGQ_REG_SIZE * _que_idx)

#define CLS_WIFI_MSGQ_QUE_STATUS_REG(_que_idx) \
	(CLS_WIFI_MSGQ_CFG_OFFSET + CLS_WIFI_MSGQ_QUE_STATUS_OFFSET \
	+ CLS_WIFI_MSGQ_REG_SIZE * _que_idx)

#define CLS_WIFI_MSGQ_INT_CTRL_REG(_que_idx) \
	(CLS_WIFI_MSGQ_CFG_OFFSET + CLS_WIFI_MSGQ_INT_CTRL_OFFSET \
	+ CLS_WIFI_MSGQ_REG_SIZE * _que_idx)

#define CLS_WIFI_MSGQ_PEEK_REG(_que_idx, _item_idx) \
	(CLS_WIFI_MSGQ_PEEK_OFFSET + CLS_WIFI_MSGQ_QUE_SIZE * _que_idx \
	+ CLS_WIFI_MSGQ_REG_SIZE * _item_idx)

#define CLS_WIFI_MSGQ_PEEK_INVALID_REG(_que_idx) \
	(CLS_WIFI_MSGQ_PEEK_OFFSET + CLS_WIFI_MSGQ_QUE_SIZE * _que_idx)

#define CLS_WIFI_MSGQ_POP_REG(_que_idx) \
	(CLS_WIFI_MSGQ_POP_OFFSET + CLS_WIFI_MSGQ_QUE_SIZE * _que_idx)

#define CLS_WIFI_MSGQ_PUSH_REG(_que_idx) \
	(CLS_WIFI_MSGQ_PUSH_OFFSET + CLS_WIFI_MSGQ_QUE_SIZE * _que_idx)

#define CLS_WIFI_MSGQ_BMU_P_CTRL_REG(_pool_idx) \
	(CLS_WIFI_MSGQ_BMU_P_CTRL_OFFSET + CLS_WIFI_MSGQ_REG_SIZE * _pool_idx)

#define CLS_WIFI_MSGQ_BMU_P_END_REG(_pool_idx) \
	(CLS_WIFI_MSGQ_BMU_P_END_OFFSET + CLS_WIFI_MSGQ_REG_SIZE * _pool_idx)

#define CLS_WIFI_MSGQ_BMU_L_CTRL_REG(_pool_idx) \
	(CLS_WIFI_MSGQ_BMU_L_CTRL_OFFSET + CLS_WIFI_MSGQ_REG_SIZE * _pool_idx)

#define CLS_WIFI_MSGQ_BMU_REG(_pool_idx) \
	(CLS_WIFI_MSGQ_BMU_OFFSET + CLS_WIFI_MSGQ_BMU_SIZE * _pool_idx)

#define CLS_WIFI_MSGQ_QUE_EN_VAL		0x00000001
#define CLS_WIFI_MSGQ_POP_EN_VAL		0x00000002
#define CLS_WIFI_MSGQ_INTR_TH_OFFSET		2
#define CLS_WIFI_MSGQ_INTR_TH_MASK		0x000000fc
#define CLS_WIFI_MSGQ_QUE_BASE_OFFSET		8
#define CLS_WIFI_MSGQ_QUE_BASE_MASK		0x0007ff00
#define CLS_WIFI_MSGQ_QUE_DEPTH_OFFSET		20
#define CLS_WIFI_MSGQ_QUE_DEPTH_MASK		0x3ff00000

#define CLS_WIFI_MSGQ_STATUS_RD_OFFSET		16
#define CLS_WIFI_MSGQ_STATUS_RD_MASK		0x03ff0000
#define CLS_WIFI_MSGQ_STATUS_WR_OFFSET		0
#define CLS_WIFI_MSGQ_STATUS_WR_MASK		0x000003ff
#define CLS_WIFI_MSGQ_STATUS_FULL_OFFSET	31
#define CLS_WIFI_MSGQ_STATUS_FULL_MASK		0x80000000
#define CLS_WIFI_MSGQ_STATUS_EMP_OFFSET		30
#define CLS_WIFI_MSGQ_STATUS_EMP_MASK		0x40000000

#define CLS_WIFI_MSGQ_BMU_ENABLE_VAL		0x00000002

#define CLS_WIFI_MSGQ_BMU_LSIZE_MASK		0x0000000f

#define CLS_WIFI_MSGQ_BMU_P_EN_VAL		0x00000001
#define CLS_WIFI_MSGQ_BMU_P_NUM_OFFSET		1
#define CLS_WIFI_MSGQ_BMU_P_NUM_MASK		0x0000001e
#define CLS_WIFI_MSGQ_BMU_P_IDX_OFFSET		5
#define CLS_WIFI_MSGQ_BMU_P_IDX_MASK		0x00000060
#define CLS_WIFI_MSGQ_BMU_ADDR_OFFSET		10
#define CLS_WIFI_MSGQ_BMU_ADDR_MASK		0xfffffc00

void cls_wifi_msgq_init(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, bool int_enable,
		u32 int_th, u32 que_depth);
void cls_wifi_msgq_dump(struct cls_wifi_hw *cls_wifi_hw);
u32 cls_wifi_msgq_peek(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 item_index);
void cls_wifi_msgq_peek_invalid(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 item_num);
u32 cls_wifi_msgq_pop(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index);
void cls_wifi_msgq_push(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 value);
#if defined(__aarch64__) || defined(__x86_64__)
void cls_wifi_msgq_push2(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 value1, u32 value2);
#endif
void cls_wifi_bmu_init(struct cls_wifi_hw *cls_wifi_hw, u32 pool_index, u32 size, u32 nr_per_pool,
		u32 p_pool_mask, u32 start_addr);
u32 cls_wifi_bmu_get(struct cls_wifi_hw *cls_wifi_hw, u32 pool_index);
void cls_wifi_bmu_put(struct cls_wifi_hw *cls_wifi_hw, u32 pool_index, u32 value);

#endif /* _CLS_WIFI_MSGQ_H_ */
