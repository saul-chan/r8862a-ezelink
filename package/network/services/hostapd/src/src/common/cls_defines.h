/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#ifndef _CLS_DEFINES_H
#define _CLS_DEFINES_H

/*
 * This header file defines the many definitions, including struct,
 * enum and other types for Application layer.
 * Please be careful to change this file and must keep consistent
 * with its copy files, such as: cls_defines.h in clsapi module.
 */


/*****************************  Variables & MARCROs definitions **************************/


/*****************************  Data type definitions ************************************/

struct cls_wps_stat {
    enum wps_status status;
    enum wps_error_indication failure_reason;
    enum pbc_status pbc_status;
    uint8_t peer_addr[ETH_ALEN];
};

struct cls_channel_survey {
	uint32_t freq;
	uint8_t channel_utilization;
	uint8_t bss_load_update_period;
	uint8_t chan_util_avg_period;
};

#endif
