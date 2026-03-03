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

#ifndef _CLS_WIFI_MESH_H_
#define _CLS_WIFI_MESH_H_

#include "cls_wifi_defs.h"

struct cls_wifi_mesh_proxy *cls_wifi_get_mesh_proxy_info(struct cls_wifi_vif *p_cls_wifi_vif, u8 *p_sta_addr, bool local);

#endif /* _CLS_WIFI_MESH_H_ */
