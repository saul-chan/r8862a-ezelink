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

#ifndef _CLS_WIFI_MAIN_H_
#define _CLS_WIFI_MAIN_H_

#include "cls_wifi_defs.h"

extern uint32_t bands_enable;
extern uint32_t bands_reverse;
extern uint32_t tx_wq;
extern uint32_t txfree_wq;

extern uint32_t sigma_enable;
extern uint32_t txq_ctrl_stop;
extern uint32_t txq_ctrl_start;
int cls_wifi_cfg80211_init(struct cls_wifi_plat *cls_wifi_plat, void **platform_data, u8 radio_idx);
void cls_wifi_cfg80211_deinit(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_cfg80211_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			 struct cfg80211_mgmt_tx_params *params, u64 *cookie);

#endif /* _CLS_WIFI_MAIN_H_ */
