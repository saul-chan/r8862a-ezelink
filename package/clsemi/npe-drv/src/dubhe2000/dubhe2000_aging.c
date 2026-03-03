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

/* dubhe2000_aging.c
 * Shared functions for accessing and configuring the aging
 */

#include "dubhe2000_aging.h"

void __iomem *aging_base_addr = NULL;

#define AGING_STATUS_TBL_LARGE0_DES_INDEX		0
#define AGING_STATUS_TBL_SMALL0_DES_INDEX		1
#define AGING_STATUS_TBL_TCAM0_DES_INDEX		2
#define AGING_STATUS_TBL_INVALID_DES_INDEX		3

char *aging_status_tbl_des[] = { "large0", "small0", "tcam0", "invalid" };

/* will not check index */
int dubhe2000_aging_status_tbl_get_des(u32 index)
{
	if (index >= AGING_STATUS_TBL_LARGE0_START && index <= AGING_STATUS_TBL_LARGE0_END)
		return AGING_STATUS_TBL_LARGE0_DES_INDEX;
	else if (index >= AGING_STATUS_TBL_SMALL0_START && index <= AGING_STATUS_TBL_SMALL0_END)
		return AGING_STATUS_TBL_SMALL0_DES_INDEX;
	else if (index >= AGING_STATUS_TBL_TCAM0_START && index <= AGING_STATUS_TBL_TCAM0_END)
		return AGING_STATUS_TBL_TCAM0_DES_INDEX;
	else
		return AGING_STATUS_TBL_INVALID_DES_INDEX;
}

void dubhe2000_aging_status_tbl_config(struct dubhe1000_adapter *adapter, bool is_add, u32 index)
{
	u32 val;
	u64 address;

	if (index >= AGING_STATUS_TBL_MAX) {
		pr_info("[%s] invalid index\n", __func__);
		return;
	}

	if (is_add)
		val = 0xFF;
	else
		val = 0;

	address = AGING_STATUS_TBL + index * AGING_STATUS_TBL_ADDR_PER_ENTRY;

	aging_w32(address, val);

	pr_info("[%s] %s %s-tbl[%d]: 0x%x\n", __func__, is_add ? "add" : "del",
		aging_status_tbl_des[dubhe2000_aging_status_tbl_get_des(index)], index, val);
}

void dubhe2000_aging_status_tbl_dump(struct dubhe1000_adapter *adapter, u32 index)
{
	int k = 0;
	u32 val = 0;
	u64 address;
	u32 start = 0, end = AGING_STATUS_TBL_MAX - 1;

	if (index < AGING_STATUS_TBL_MAX) {
		start = index;
		end = index + 1;
	}

	for (k = start; k < end; k++) {
		address = AGING_STATUS_TBL + k * AGING_STATUS_TBL_ADDR_PER_ENTRY;
		val = aging_r32(address);
		if (val && (val != AGING_STATUS_TBL_INVALID))
			pr_info("[%s] offset 0x%llx %s-tbl[%d]: 0x%08x\n", __func__, address,
				aging_status_tbl_des[dubhe2000_aging_status_tbl_get_des(index)], k, val);
	}
}

void dubhe2000_aging_cfg_1ms_timer_config(struct dubhe1000_adapter *adapter, u32 value)
{
	u32 val;
	u64 address = AGING_CFG_1MS_TIMER;

	val = value & AGING_CFG_1MS_TIMER_MASK;

	aging_w32(address, val);

	pr_info("[%s] offset 0x%llx val 0x%X\n", __func__, address, val);
}

void dubhe2000_aging_cfg_1ms_timer_dump(struct dubhe1000_adapter *adapter)
{
	u32 val;
	u64 address = AGING_CFG_1MS_TIMER;

	val = aging_r32(address);

	pr_info("[%s] offset 0x%llx val 0x%X\n", __func__, address, val);
}

void dubhe2000_aging_cfg_aging_timer_config(struct dubhe1000_adapter *adapter, u32 value)
{
	u32 val;
	u64 address = AGING_CFG_AGING_TIMER;

	val = value & AGING_CFG_AGING_TIMER_BIT_MASK;

	aging_w32(address, val);

	pr_info("[%s] offset 0x%llx val 0x%X\n", __func__, address, val);
}

void dubhe2000_aging_cfg_aging_timer_dump(struct dubhe1000_adapter *adapter)
{
	u32 val;
	u64 address = AGING_CFG_AGING_TIMER;

	val = aging_r32(address);

	pr_info("[%s] offset 0x%llx val 0x%X\n", __func__, address, val);
}

void dubhe2000_aging_cfg_ctrl_config(struct dubhe1000_adapter *adapter, u32 value)
{
	u32 val;
	u64 address = AGING_CFG_CTRL;

	val = value & AGING_CFG_CTRL_MASK;
	aging_w32(address, val);

	pr_info("[%s] offset 0x%llx val 0x%X\n", __func__, address, val);
}

void dubhe2000_aging_cfg_ctrl_dump(struct dubhe1000_adapter *adapter)
{
	u32 val;
	u64 address = AGING_CFG_CTRL;

	val = aging_r32(address);

	pr_info("[%s] offset 0x%llx val 0x%X\n", __func__, address, val);
}

s32 dubhe2000_aging_init(struct dubhe1000_adapter *adapter)
{
	/* Note:
	 * Only aging0_regs(Ingress ACL0-NAT) is active.
	 * Ignore aging1_regs (Egress ACL0-NAT Aging) here.
	 */
	aging_base_addr = adapter->aging0_regs;

	dubhe2000_aging_status_tbl_dump(adapter, AGING_STATUS_TBL_MAX);
	dubhe2000_aging_cfg_1ms_timer_dump(adapter);
	dubhe2000_aging_cfg_aging_timer_dump(adapter);
	dubhe2000_aging_cfg_ctrl_dump(adapter);

	return 0;
}
