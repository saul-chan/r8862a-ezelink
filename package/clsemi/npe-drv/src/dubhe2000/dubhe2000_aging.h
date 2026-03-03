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

/* dubhe2000_aging.h
 */

#ifndef _DUBHE2000_AGING_H_
#define _DUBHE2000_AGING_H_

#include "dubhe2000_osdep.h"
#include "dubhe2000.h"

#define AGING_CONFIG_BASE			0x4C208000

#define AGING_STATUS_TBL			0x0
#define AGING_STATUS_TBL_MASK			0xFF
#define AGING_STATUS_TBL_MAX			3072
#define AGING_STATUS_TBL_ADDR_PER_ENTRY		0x4
#define AGING_STATUS_TBL_INVALID		0xbabeface

#define AGING_STATUS_TBL_LARGE0_MAX		2048
#define AGING_STATUS_TBL_SMALL0_MAX		256
#define AGING_STATUS_TBL_TCAM0_MAX		16

#define AGING_STATUS_TBL_LARGE0_START		0
#define AGING_STATUS_TBL_LARGE0_END		(AGING_STATUS_TBL_LARGE0_MAX - 1)
#define AGING_STATUS_TBL_SMALL0_START		AGING_STATUS_TBL_LARGE0_MAX
#define AGING_STATUS_TBL_SMALL0_END		(AGING_STATUS_TBL_LARGE0_MAX + AGING_STATUS_TBL_SMALL0_MAX - 1)
#define AGING_STATUS_TBL_TCAM0_START		(AGING_STATUS_TBL_LARGE0_MAX + AGING_STATUS_TBL_SMALL0_MAX)
#define AGING_STATUS_TBL_TCAM0_END		(AGING_STATUS_TBL_LARGE0_MAX + AGING_STATUS_TBL_SMALL0_MAX + AGING_STATUS_TBL_TCAM0_MAX - 1)
#define AGING_STATUS_TBL_INVALID_START		(AGING_STATUS_TBL_LARGE0_MAX + AGING_STATUS_TBL_SMALL0_MAX + AGING_STATUS_TBL_TCAM0_MAX)
#define AGING_STATUS_TBL_INVALID_END		(AGING_STATUS_TBL_MAX - 1)


#define AGING_CFG_1MS_TIMER			0x3000
#define AGING_CFG_1MS_TIMER_MASK		0xFFFFF
#define CFG_1MS_TIMER_BIT			0 // bits[0:19]

#define AGING_CFG_AGING_TIMER			0x3004
#define AGING_CFG_AGING_TIMER_BIT_MASK		0xFFFFF
#define CFG_AGING_TIMER_BIT			0 // bits[0:19]

#define AGING_CFG_CTRL				0x3008
#define AGING_CFG_CTRL_MASK			0xC0000000
#define MASK_REFRESH_MASK			30 //bits[30:30]
#define MASK_SCAN_BIT				31 //bits[31:31]

void dubhe2000_aging_status_tbl_config(struct dubhe1000_adapter *adapter, bool is_add, u32 index);
void dubhe2000_aging_status_tbl_dump(struct dubhe1000_adapter *adapter, u32 index);
void dubhe2000_aging_cfg_1ms_timer_config(struct dubhe1000_adapter *adapter, u32 value);
void dubhe2000_aging_cfg_1ms_timer_dump(struct dubhe1000_adapter *adapter);
void dubhe2000_aging_cfg_aging_timer_config(struct dubhe1000_adapter *adapter, u32 value);
void dubhe2000_aging_cfg_aging_timer_dump(struct dubhe1000_adapter *adapter);
void dubhe2000_aging_cfg_ctrl_config(struct dubhe1000_adapter *adapter, u32 value);
void dubhe2000_aging_cfg_ctrl_dump(struct dubhe1000_adapter *adapter);
s32 dubhe2000_aging_init(struct dubhe1000_adapter *adapter);
#endif /* _DUBHE2000_AGING_H_ */
