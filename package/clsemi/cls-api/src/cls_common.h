/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

/* ClourneySemi common definitions.
 * move to global .h when it's designed later.
 */

#ifndef _CLS_COMMON_H
#define _CLS_COMMON_H

#define STRUCT_PACKED __attribute__ ((packed))


#define CLS_RADIO_NUM					3
#define CLS_2G_RADIO_IDX				0
#define CLS_5G_RADIO_IDX				1
#define CLS_MAX_VAP_PER_RADIO			8
#define CLS_MAX_STA_PER_VAP				(128)
#define RADIO_IDX_TO_BAND(idx)			(((idx) == CLS_2G_RADIO_IDX) ? CLSAPI_BAND_2GHZ :\
										(((idx) >= CLS_5G_RADIO_IDX) ? CLSAPI_BAND_5GHZ : CLSAPI_BAND_NOSUCH_BAND))

#ifndef BIT
#define BIT(x)	(1U << (x))
#endif

/* Wi-Fi feature bit map */
#define CLS_WIFI_FEATURE_BIT_MESH		BIT(0)
#define CLS_WIFI_FEATURE_BIT_DUALBAND	BIT(1)


#define CLS_WIFI_FEATURE_COMMON			(CLS_WIFI_FEATURE_BIT_DUALBAND)

#endif /* _CLS_COMMON_H */

