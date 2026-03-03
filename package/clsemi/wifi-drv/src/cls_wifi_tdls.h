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

#ifndef _CLS_WIFI_TDLS_H_
#define _CLS_WIFI_TDLS_H_

#include "cls_wifi_defs.h"

struct ieee_types_header {
	u8 element_id;
	u8 len;
} __packed;

struct ieee_types_bss_co_2040 {
	struct ieee_types_header ieee_hdr;
	u8 bss_2040co;
} __packed;

struct ieee_types_extcap {
	struct ieee_types_header ieee_hdr;
	u8 ext_capab[8];
} __packed;

struct ieee_types_vht_cap {
	struct ieee_types_header ieee_hdr;
	struct ieee80211_vht_cap vhtcap;
} __packed;

struct ieee_types_vht_oper {
	struct ieee_types_header ieee_hdr;
	struct ieee80211_vht_operation vhtoper;
} __packed;

struct ieee_types_aid {
	struct ieee_types_header ieee_hdr;
	u16 aid;
} __packed;

int cls_wifi_tdls_send_mgmt_packet_data(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
						 const u8 *peer, u8 action_code, u8 dialog_token,
						 u16 status_code, u32 peer_capability, bool initiator,
						 const u8 *extra_ies, size_t extra_ies_len, u8 oper_class,
						 struct cfg80211_chan_def *chandef);

#endif /* _CLS_WIFI_TDLS_H_ */
