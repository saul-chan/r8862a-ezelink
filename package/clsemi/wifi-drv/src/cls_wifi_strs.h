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

#ifndef _CLS_WIFI_STRS_H_
#define _CLS_WIFI_STRS_H_

#include "lmac_msg.h"

#define CLS_WIFI_ID2STR(tag) (((MSG_T(tag) < ARRAY_SIZE(cls_wifi_id2str)) &&		\
						   (cls_wifi_id2str[MSG_T(tag)]) &&		  \
						   ((cls_wifi_id2str[MSG_T(tag)])[MSG_I(tag)])) ?   \
						  (cls_wifi_id2str[MSG_T(tag)])[MSG_I(tag)] : "unknown")

extern const char *const *cls_wifi_id2str[TASK_LAST_EMB + 1];

#endif /* _CLS_WIFI_STRS_H_ */
