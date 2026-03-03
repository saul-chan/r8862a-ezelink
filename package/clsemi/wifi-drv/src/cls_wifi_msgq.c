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

#include "cls_wifi_defs.h"
#include "cls_wifi_msgq.h"

static u32 cls_wifi_msgq_read(struct cls_wifi_hw *cls_wifi_hw, u32 offset)
{
	return cls_wifi_hw->ipc_env->ops->msgq_read32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
			offset);
}

static void cls_wifi_msgq_write(struct cls_wifi_hw *cls_wifi_hw, u32 offset, u32 value)
{
	cls_wifi_hw->ipc_env->ops->msgq_write32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
			offset, value);
}

#if defined(__aarch64__) || defined(__x86_64__)
static void cls_wifi_msgq_write64(struct cls_wifi_hw *cls_wifi_hw, u32 offset, u64 value)
{
	cls_wifi_hw->ipc_env->ops->msgq_write64(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
			offset, value);
}
#endif

static bool cls_wifi_msgq_index_valid(u32 queue_index)
{
	if (likely(queue_index < CLS_WIFI_MSGQ_QUE_NB))
		return true;

	pr_warn("Unsupported queue index %d.\n", queue_index);
	return false;
}

void cls_wifi_msgq_init(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, bool int_enable,
		u32 int_th, u32 que_depth)
{
	static s32 left = CLS_WIFI_MSGQ_TOTAL_MAX;
	static u32 que_base;
	static u32 inited;
	u32 val = 0;

	if (!cls_wifi_msgq_index_valid(queue_index))
		return;

	if (queue_index == 0) {
		left = CLS_WIFI_MSGQ_TOTAL_MAX;
		que_base = 0;
		inited = 0;
	}

	if (que_depth > CLS_WIFI_MSGQ_SINGLE_MAX) {
		pr_warn("Init failed, que_depth %d > max %d.\n", que_depth, CLS_WIFI_MSGQ_SINGLE_MAX);
		return;
	}

	if (que_depth > left) {
		pr_warn("Init failed, que_depth %d > left %d.\n", que_depth, left);
		return;
	}

	if ((1 << queue_index) & inited) {
		pr_warn("Init failed, queue %d has inited before.\n", queue_index);
		return;
	}

	val |= que_depth > 0 ? CLS_WIFI_MSGQ_QUE_EN_VAL : 0;
	val |= que_depth > 0 ? CLS_WIFI_MSGQ_POP_EN_VAL : 0;
	val |= (int_th << CLS_WIFI_MSGQ_INTR_TH_OFFSET) & CLS_WIFI_MSGQ_INTR_TH_MASK;
	val |= (que_base << CLS_WIFI_MSGQ_QUE_BASE_OFFSET) & CLS_WIFI_MSGQ_QUE_BASE_MASK;
	val |= ((que_depth - 1) << CLS_WIFI_MSGQ_QUE_DEPTH_OFFSET)
			& CLS_WIFI_MSGQ_QUE_DEPTH_MASK;
	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_QUE_CTRL_REG(queue_index), val);
	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_INT_CTRL_REG(queue_index), int_enable ?
			0 : CLS_WIFI_MSGQ_INT_MASK_VAL);

	left -= que_depth;
	que_base += que_depth;
	inited |= 1 << queue_index;
}

void cls_wifi_msgq_dump(struct cls_wifi_hw *cls_wifi_hw)
{
	u32 i, val;

	pr_warn("index\treg\t\tenable\tpop\tint_th\tdepth\t\n");
	for (i = 0; i < CLS_WIFI_MSGQ_QUE_NB; i++) {
		val = cls_wifi_msgq_read(cls_wifi_hw, CLS_WIFI_MSGQ_QUE_CTRL_REG(i));
		pr_warn("%3d\t0x%08x\t%3d\t%2d\t%4d\t%4d\t\n", i, val,
			val & CLS_WIFI_MSGQ_QUE_EN_VAL ? 1 : 0,
			val & CLS_WIFI_MSGQ_POP_EN_VAL ? 1 : 0,
			(val & CLS_WIFI_MSGQ_INTR_TH_MASK) >> CLS_WIFI_MSGQ_INTR_TH_OFFSET,
			((val & CLS_WIFI_MSGQ_QUE_DEPTH_MASK) >> CLS_WIFI_MSGQ_QUE_DEPTH_OFFSET) + 1);
	}

	pr_warn("index\treg\t\tread\twrite\tfull\tempty\t\n");
	for (i = 0; i < CLS_WIFI_MSGQ_QUE_NB; i++) {
		val = cls_wifi_msgq_read(cls_wifi_hw, CLS_WIFI_MSGQ_QUE_STATUS_REG(i));
		pr_warn("%3d\t0x%08x\t%4d\t%4d\t%1d\t%1d\t\n", i, val,
			(val & CLS_WIFI_MSGQ_STATUS_RD_MASK) >> CLS_WIFI_MSGQ_STATUS_RD_OFFSET,
			(val & CLS_WIFI_MSGQ_STATUS_WR_MASK) >> CLS_WIFI_MSGQ_STATUS_WR_OFFSET,
			(val & CLS_WIFI_MSGQ_STATUS_FULL_MASK) >> CLS_WIFI_MSGQ_STATUS_FULL_OFFSET,
			(val & CLS_WIFI_MSGQ_STATUS_EMP_MASK) >> CLS_WIFI_MSGQ_STATUS_EMP_OFFSET);
	}

#ifdef CONFIG_CLS_MSGQ_TEST
	{
		u32 int_cnt[8];

		cls_wifi_hw->plat->ep_ops->readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				offsetof(struct ipc_shared_env_tag, msg_e2a_buf.param),
				int_cnt, sizeof(int_cnt));

		pr_warn("int cnt: %d %d %d %d %d %d %d %d\n",
				int_cnt[0], int_cnt[1], int_cnt[2], int_cnt[3],
				int_cnt[4], int_cnt[5], int_cnt[6], int_cnt[7]);

		memset(int_cnt, 0, sizeof(int_cnt));
		cls_wifi_hw->plat->ep_ops->writen(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				offsetof(struct ipc_shared_env_tag, msg_e2a_buf.param),
				int_cnt, sizeof(int_cnt));
	}
#endif
}

u32 cls_wifi_msgq_peek(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 item_index)
{
	if (!cls_wifi_msgq_index_valid(queue_index))
		return 0;
	return cls_wifi_msgq_read(cls_wifi_hw, CLS_WIFI_MSGQ_PEEK_REG(queue_index, item_index));
}

void cls_wifi_msgq_peek_invalid(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 item_num)
{
	if (!cls_wifi_msgq_index_valid(queue_index))
		return;
	if (item_num > CLS_WIFI_MSGQ_PEEK_INVALID_MAX) {
		pr_warn("Peek invalid number %d is not valid, max is %d.\n", item_num,
				CLS_WIFI_MSGQ_PEEK_INVALID_MAX);
		return;
	}
	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_PEEK_INVALID_REG(queue_index), item_num);
}

u32 cls_wifi_msgq_pop(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index)
{
	if (!cls_wifi_msgq_index_valid(queue_index))
		return 0;
	return cls_wifi_msgq_read(cls_wifi_hw, CLS_WIFI_MSGQ_POP_REG(queue_index));
}

void cls_wifi_msgq_push(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 value)
{
	if (!cls_wifi_msgq_index_valid(queue_index))
		return;
	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_PUSH_REG(queue_index), value);
}

#if defined(__aarch64__) || defined(__x86_64__)
void cls_wifi_msgq_push2(struct cls_wifi_hw *cls_wifi_hw, u32 queue_index, u32 value1, u32 value2)
{
	if (!cls_wifi_msgq_index_valid(queue_index))
		return;
	cls_wifi_msgq_write64(cls_wifi_hw, CLS_WIFI_MSGQ_PUSH_REG(queue_index), (u64)value2 << 32 | value1);
}
#endif

static bool cls_wifi_bmu_lpool_index_valid(u32 pool_index)
{
	if (likely(pool_index < CLS_WIFI_MSGQ_BMU_LPOOL_NB))
		return true;

	pr_warn("Unsupported logic pool index %d.\n", pool_index);
	return false;
}

static bool cls_wifi_bmu_ppool_mask_valid(u32 pool_mask)
{
	if (likely(pool_mask > 0 && pool_mask < (1 << CLS_WIFI_MSGQ_BMU_PPOOL_NB)))
		return true;

	pr_warn("Unsupported physical pool mask %d.\n", pool_mask);
	return false;
}

void cls_wifi_bmu_init(struct cls_wifi_hw *cls_wifi_hw, u32 pool_index, u32 size, u32 nr_per_pool,
		u32 p_pool_mask, u32 start_addr)
{
	u32 val = 0;
	u32 end_addr;
	u32 offset;
	u32 i;

	if (!cls_wifi_bmu_lpool_index_valid(pool_index))
		return;
	if (!cls_wifi_bmu_ppool_mask_valid(p_pool_mask))
		return;

	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_CTRL_OFFSET, CLS_WIFI_MSGQ_BMU_ENABLE_VAL);
	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_BMU_L_CTRL_REG(pool_index),
			size & CLS_WIFI_MSGQ_BMU_LSIZE_MASK);

	offset = nr_per_pool * (1 << (size & CLS_WIFI_MSGQ_BMU_LSIZE_MASK)) / 2;
	if (offset < 1)
		offset = 1;
	for (i = 0; i < CLS_WIFI_MSGQ_BMU_PPOOL_NB; i++) {
		if (!(p_pool_mask & (1 << i)))
			continue;

		if (offset > 1)
			end_addr = start_addr + offset - 1;
		else
			end_addr = start_addr + 1;

		val = (end_addr << CLS_WIFI_MSGQ_BMU_ADDR_OFFSET) & CLS_WIFI_MSGQ_BMU_ADDR_MASK;
		cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_BMU_P_END_REG(i), val);

		val = (start_addr << CLS_WIFI_MSGQ_BMU_ADDR_OFFSET) & CLS_WIFI_MSGQ_BMU_ADDR_MASK;
		val |= CLS_WIFI_MSGQ_BMU_P_EN_VAL;
		val |= (nr_per_pool << CLS_WIFI_MSGQ_BMU_P_NUM_OFFSET) & CLS_WIFI_MSGQ_BMU_P_NUM_MASK;
		val |= (pool_index << CLS_WIFI_MSGQ_BMU_P_IDX_OFFSET) & CLS_WIFI_MSGQ_BMU_P_IDX_MASK;
		cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_BMU_P_CTRL_REG(i), val);

		start_addr += offset;
	}
}

u32 cls_wifi_bmu_get(struct cls_wifi_hw *cls_wifi_hw, u32 pool_index)
{
	if (!cls_wifi_bmu_lpool_index_valid(pool_index))
		return 0;

	return cls_wifi_msgq_read(cls_wifi_hw, CLS_WIFI_MSGQ_BMU_REG(pool_index));
}

void cls_wifi_bmu_put(struct cls_wifi_hw *cls_wifi_hw, u32 pool_index, u32 value)
{
	if (!cls_wifi_bmu_lpool_index_valid(pool_index))
		return;

	cls_wifi_msgq_write(cls_wifi_hw, CLS_WIFI_MSGQ_BMU_REG(pool_index), value);
}
