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

#ifndef CLSQOS_H
#define CLSQOS_H

#define UCI_CLSQOS_PACKAGE	"cls-qos"
#define UCI_VIPQOS_SECTION	"vip_qos"
#define UCI_VIPQOS_SECTION_TYPE	"vip_qos"

typedef unsigned long long u64;
union clsmark {
	u64 clsmark;
	struct {
		u64	cls_ori_set_queue:1,
			cls_ori_queue:3,
			cls_ori_set_pcp:1,
			cls_ori_pcp:3,
			cls_ori_set_dscp:1,
			cls_ori_dscp:6,
			cls_rep_set_queue:1,
			cls_rep_queue:3,
			cls_rep_set_pcp:1,
			cls_rep_pcp:3,
			cls_rep_set_dscp:1,
			cls_rep_dscp:6,
			cls_is_vip:1,
			cls_need_recall:1,
			cls_is_reverse:1,
			cls_no_accel_for_uft:1,
			cls_uft_done:1,
			cls_no_accel_for_qos:1,
			cls_no_accel_for_ctl:1,
			cls_dpi_done:1,
			cls_app_id:5,
			cls_reserve:21;
	};
};
#endif
