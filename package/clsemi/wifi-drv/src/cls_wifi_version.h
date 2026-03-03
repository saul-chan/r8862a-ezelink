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

#ifndef _CLS_WIFI_VERSION_H_
#define _CLS_WIFI_VERSION_H_

// Those constant strings are defined in cls_wifi_version.c that is automatically
// generated at compilation time
extern const char cls_wifi_mod_version[];
extern const char cls_wifi_build_date[];
extern const char cls_wifi_build_version[];

static inline void cls_wifi_print_version(void)
{
	pr_info("cls_wifi %s - build: %s - %s\n", cls_wifi_mod_version,
			cls_wifi_build_date, cls_wifi_build_version);
}
#endif /* _CLS_WIFI_VERSION_H_ */
