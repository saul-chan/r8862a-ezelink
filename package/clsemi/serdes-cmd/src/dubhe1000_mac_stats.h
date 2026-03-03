/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _DUBHE1000_MAC_STATS_H_
#define _DUBHE1000_MAC_STATS_H_
#include <stdint.h>
#include <stdbool.h>
#define uint uint32_t
#define u8   uint8_t
#define u16   uint16_t
#define u32 uint32_t
#define u64 uint64_t

struct dubhe1000_xgmac_stats_param {
	char name[64];
#define XGMAC_STATS_PARAM_TYPE_64_BIT	0
#define XGMAC_STATS_PARAM_TYPE_32_BIT	1
	u8 type;// 0 - 64bits; 1 - 32bits
	u64 addr;
	u64 value;
};

#define XGMAC_STATS_OPTION_ALL		0
#define XGMAC_STATS_OPTION_TX		1
#define XGMAC_STATS_OPTION_RX		2

void dubhe1000_xgmac_stats_dump_per_index(uint32_t address,
			u8 xgmac_index, u8 option);
#endif /* _DUBHE1000_MAC_STATS_H_ */
