/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _MAC_STATS_H_
#define _MAC_STATS_H_

#include "dubhe1000.h"

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

void dubhe1000_xgmac_stats_dump_per_index(struct dubhe1000_adapter *adapter,
			u8 xgmac_index, u8 option);
#endif /* _MAC_STATS_H_ */
