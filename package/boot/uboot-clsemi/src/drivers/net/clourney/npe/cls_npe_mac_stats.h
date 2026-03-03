/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _CLS_NPE_MAC_STATS_H_
#define _CLS_NPE_MAC_STATS_H_

#include "cls_npe.h"

struct cls_xgmac_stats_param {
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

void cls_xgmac_stats_dump_per_index(struct cls_eth_priv *adapter,
			u8 xgmac_index, u8 option);
#endif /* _CLS_NPE_MAC_STATS_H_ */
