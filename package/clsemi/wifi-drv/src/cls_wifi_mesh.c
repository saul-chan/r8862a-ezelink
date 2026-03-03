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

#include "cls_wifi_mesh.h"

/**
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

struct cls_wifi_mesh_proxy *cls_wifi_get_mesh_proxy_info(struct cls_wifi_vif *p_cls_wifi_vif, u8 *p_sta_addr, bool local)
{
	struct cls_wifi_mesh_proxy *p_mesh_proxy = NULL;
	struct cls_wifi_mesh_proxy *p_cur_proxy;

	/* Look for proxied devices with provided address */
	list_for_each_entry(p_cur_proxy, &p_cls_wifi_vif->ap.proxy_list, list) {
		if (p_cur_proxy->local != local) {
			continue;
		}

		if (!memcmp(&p_cur_proxy->ext_sta_addr, p_sta_addr, ETH_ALEN)) {
			p_mesh_proxy = p_cur_proxy;
			break;
		}
	}

	/* Return the found information */
	return p_mesh_proxy;
}
