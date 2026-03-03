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

#ifndef _CLS_WIFI_MU_H_
#define _CLS_WIFI_MU_H_

#ifdef CONFIG_CLS_WIFI_HEMU_TX
#include "ipc_shared.h"

/**
 * struct cls_wifi_he_mu - HE MU configuration
 *
 * @map_pending: Whether a new DL map has been received but not yet applied or not
 * @lock: Lock to prevent concurrent access to HE-MU configuration. For now all
 * functions (i.e. cls_wifi_he_mu_process_dl_map and cls_wifi_he_mu_cfm_hwq) are called from
 * CLS_WIFI IRQ bottom handler so this doesn't seems needed (TBC)
 * @map: Local copy of the last DL map sent by firmware (the one to apply if
 * map_pending is true)
 * @active_sta: List of active stations in the current DL map
 * @active_user: Bitmap of user position active in the current DL map.
 # @map_hwq: The AC category to use when sending HE-MU frame with the current DL map
 */
struct cls_wifi_he_mu {
	bool map_pending;
	spinlock_t lock;
	struct he_mu_map_array_desc map;
	struct list_head active_sta;
	u16 active_user;
	u16 map_hwq;
};

void cls_wifi_he_mu_init(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_he_mu_process_dl_map(struct cls_wifi_hw *cls_wifi_hw,
							   struct he_mu_map_array_desc *map);
void cls_wifi_he_mu_cfm_hwq(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_hwq *hwq,
						struct cls_wifi_sw_txhdr *sw_txhdr);
void cls_wifi_he_mu_sta_ps_update(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
							  bool ps_active);
u16 cls_wifi_he_mu_set_user(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sw_txhdr *sw_txhdr);
#else // ! CONFIG_CLS_WIFI_HEMU_TX

struct cls_wifi_he_mu {
};

static inline void cls_wifi_he_mu_init(struct cls_wifi_hw *cls_wifi_hw) {}
static inline void cls_wifi_he_mu_process_dl_map(struct cls_wifi_hw *cls_wifi_hw,
											 struct he_mu_map_array_desc *map) {}
static inline void cls_wifi_he_mu_cfm_hwq(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_hwq *hwq,
									  struct cls_wifi_sw_txhdr *sw_txhdr) {}
static inline void cls_wifi_he_mu_sta_ps_update(struct cls_wifi_hw *cls_wifi_hw,
											struct cls_wifi_sta *sta, bool ps_active) {}
#endif // CONFIG_CLS_WIFI_HEMU_TX
#endif // _CLS_WIFI_MU_H_
