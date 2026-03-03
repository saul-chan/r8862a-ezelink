/*
 * hostapd- wapi GB 15629.11
 * Copyright (c) 2024, Clourney Semi
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WAPI_AUTH_H
#define WAPI_AUTH_H
#include "common/defs.h"
#include "hostapd.h"
#include "wapi_auth_i.h"


void wapi_auth_timeout_handle(void *eloop_ctx, void *user_ctx);
void wapi_unikey_timeout_handle(void *eloop_ctx, void *user_ctx);
void wapi_multikey_timeout_handle(void *eloop_ctx, void *user_ctx);
int wapi_auth_sm_event(struct wapi_state_machine *sm, enum wapi_event event, void *event_data);
struct wapi_authenticator *wapi_init(const u8 *addr, struct wapi_config *config,
						struct wapi_auth_callbacks *cb, void *ctx);

struct wapi_state_machine *
wapi_auth_sta_init(struct wapi_authenticator *wapi_auth, const u8 *addr,
		  const u8 *p2p_dev_addr);
u8 *wapi_eid_wapi(struct wapi_authenticator *wapi_auth, u8 *eid, int len);
int hostapd_setup_wapi(struct hostapd_data *hapd);
int wapi_auth_gen_wapi_ie(struct wapi_authenticator *wapi_auth);
void wapi_deinit(struct wapi_authenticator *wapi_auth);
void wapi_auth_sta_deinit(struct wapi_state_machine *sm);
void hostapd_deinit_wapi(struct hostapd_data *hapd);
const u8 *wapi_auth_get_wapi_ie(struct wapi_authenticator *wapi_auth, size_t *len);
void send_sta_assoc_sta(struct hostapd_data *hapd, struct sta_info *sta);
int hostapd_get_wie_from_wapid(struct hostapd_data *hapd, struct wapid_config *config);
int hostapd_get_config_from_wie(struct wapi_config *config, u8 *data, int data_len);
void hostapd_get_wapid_addr(struct wapi_authenticator *wapi_auth, u8 cmd, u8 typ,
					struct sockaddr_storage *sock_addr, socklen_t len);
int hostapd_wapi_auth_set_key(void *ctx, int vlan_id, enum wpa_alg alg,
				    const u8 *addr, int idx, u8 *key,
				    size_t key_len, enum key_flag key_flag);
#endif

