/*
 * hostapd- GB 15629.11/WAPI authenticator
 * Copyright (c) 2024, Clourney Semi
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "wapi_auth.h"
#include "ieee802_1x.h"
#include "ap_drv_ops.h"
#include "beacon.h"

void wapi_cert_timeout_handle(void *eloop_ctx, void *user_ctx)
{
	struct wapi_state_machine *sm = (struct wapi_state_machine *)eloop_ctx;

	wpa_printf(MSG_INFO, "%s:\n", __func__);
	eloop_cancel_timeout(wapi_cert_timeout_handle, sm, NULL);
	sm->wapi_auth_state = WAPI_DEAUTH;
	if (sm->wapi_auth->cb->disconnect)
		(sm->wapi_auth->cb->disconnect)(sm->wapi_auth->cb_ctx, sm->addr, sm->disconnect_reason);
}

void wapi_unikey_timeout_handle(void *eloop_ctx, void *user_ctx)
{
	struct wapi_state_machine *sm = (struct wapi_state_machine *)eloop_ctx;

	wpa_printf(MSG_INFO, "%s\n", __func__);
	eloop_cancel_timeout(wapi_unikey_timeout_handle, sm, NULL);
	sm->wapi_auth_state = WAPI_DEAUTH;
	if (sm->wapi_auth->cb->disconnect)
		(sm->wapi_auth->cb->disconnect)(sm->wapi_auth->cb_ctx, sm->addr, sm->disconnect_reason);

}

void wapi_multikey_timeout_handle(void *eloop_ctx, void *user_ctx)
{
	struct wapi_state_machine *sm = (struct wapi_state_machine *)eloop_ctx;

	wpa_printf(MSG_INFO, "%s\n", __func__);

	eloop_cancel_timeout(wapi_multikey_timeout_handle, sm, NULL);
	sm->wapi_auth_state = WAPI_DEAUTH;
	if (sm->wapi_auth->cb->disconnect)
		(sm->wapi_auth->cb->disconnect)(sm->wapi_auth->cb_ctx, sm->addr, sm->disconnect_reason);

}

int hostapd_wapi_auth_set_key(void *ctx, int vlan_id, enum wpa_alg alg,
				    const u8 *addr, int idx, u8 *key,
				    size_t key_len, enum key_flag key_flag)
{
	struct hostapd_data *hapd = ctx;
	const char *ifname = hapd->conf->iface;

	if (vlan_id > 0) {
		ifname = hostapd_get_vlan_id_ifname(hapd->conf->vlan, vlan_id);
		if (!ifname) {
			if (!(hapd->iface->drv_flags &
				WPA_DRIVER_FLAGS_VLAN_OFFLOAD))
				return -1;
			ifname = hapd->conf->iface;
		}
	}
	if (!(alg == WPA_ALG_SMS4 || alg == WPA_ALG_NONE) || key_len != 32)
		return -1;
	return hostapd_drv_set_key(ifname, hapd, alg, addr, idx, vlan_id, 1,
				NULL, 0, key, key_len, key_flag);
}

struct wapi_authenticator *wapi_init(const u8 *addr, struct wapi_config *config,
						struct wapi_auth_callbacks *cb, void *cb_ctx)
{
	struct wapi_authenticator *wapi_auth;
	int ret = 0;

	wapi_auth = os_zalloc(sizeof(struct wapi_authenticator));
	if (!wapi_auth)
		return NULL;
	os_memcpy(wapi_auth->addr, addr, ETH_ALEN);
	os_memcpy(&wapi_auth->wapi_config, config, sizeof(*config));
	wapi_auth->cb_ctx = cb_ctx;
	wapi_auth->cb = cb;
	//wapi_ie will be config later
	return wapi_auth;
}

struct wapi_state_machine *
wapi_auth_sta_init(struct wapi_authenticator *wapi_auth, const u8 *addr, const u8 *p2p_dev_addr)
{
	struct wapi_state_machine *sm;

	if (0 == wapi_auth->wapi_config.wapi_key_mgmt)
		return NULL;
	sm = os_zalloc(sizeof(struct wapi_state_machine));
	if (!sm)
		return NULL;
	os_memcpy(sm->addr, addr, ETH_ALEN);

	sm->wapi_auth = wapi_auth;
	sm->wapi_cert_timeout = WAPI_CERT_TIMEOUT;
	sm->wapi_uni_key_timeout = WAPI_UNI_KEY_TIMEOUT;
	sm->wapi_multi_key_timeout = WAPI_MULTI_KEY_TIMEOUT;
	if (sm->wapi_auth->wapi_config.wapi_key_mgmt & WPA_KEY_MGMT_WAPI_CERT)
		sm->wapi_auth_state = WAPI_AUTH_CERT;
	if (sm->wapi_auth->wapi_config.wapi_key_mgmt & WPA_KEY_MGMT_WAPI_PSK)
		sm->wapi_auth_state = WAPI_AUTH_PTK;
	return sm;
}

u8 *wapi_eid_wapi(struct wapi_authenticator *wapi_auth, u8 *eid, int len)
{
	u8 *pos = eid;
	int enabled = 0;
	u16 n_akm = 0x0001;
	u16 n_uk = 0x0001;

	if (!wapi_auth)
		return eid;

	*pos++ = WLAN_EID_BSS_AC_ACCESS_DELAY;
	//len
	*pos++;
	//version
	*pos++ = 0x01;
	*pos++ = 0x00;
	//akm num
	*pos++ = n_akm & 0xff;
	*pos++ = n_akm >> 8;
	// oui
	*pos++ = 0x00;
	*pos++ = 0x14;
	*pos++ = 0x72;
	if (wapi_auth->wapi_config.wapi_key_mgmt & WPA_KEY_MGMT_WAPI_CERT)
		*pos++ = 0x1;
	if (wapi_auth->wapi_config.wapi_key_mgmt & WPA_KEY_MGMT_WAPI_PSK)
		*pos++ = 0x2;
	// unicast key num
	*pos++ = n_uk & 0xff;
	*pos++ = n_uk >> 8;
	//oui
	*pos++ = 0x00;
	*pos++ = 0x14;
	*pos++ = 0x72;
	*pos++ = 0x1;
	//multicast key
	*pos++ = 0x00;
	*pos++ = 0x14;
	*pos++ = 0x72;
	*pos++ = 0x01;
	if (wapi_auth->wapi_config.wapi_pre_auth)
		*pos++ = 0x01;
	else
		*pos++ = 0x00;
	*pos++ = 0x00;
	if ((pos - eid) > len)
		return eid;
	*(eid + 1) = pos - eid - 2;
	return pos;
}

int wapi_auth_gen_wapi_ie(struct wapi_authenticator *wapi_auth)
{
	char buf[64];
	char *pos;

	if (!wapi_auth)
		return -1;
	pos = buf;
	if (wapi_auth->wapi_config.wapi) {
		pos = wapi_eid_wapi(wapi_auth, buf, sizeof(buf));
		if (pos == buf)
			return -1;

		os_free(wapi_auth->wapi_ie);
		wapi_auth->wapi_ie = os_malloc(pos - buf);
		if (wapi_auth->wapi_ie == NULL)
			return -1;
		os_memcpy(wapi_auth->wapi_ie, buf, pos - buf);
		wapi_auth->wapi_ie_len = pos - buf;

	}
	return 0;
}

int hostapd_get_wapi_config(struct hostapd_data *hapd, struct wapi_config *config)
{
	config->wapi = hapd->conf->wapi;
	config->wapi_key_mgmt  = hapd->conf->wapi_key_mgmt;
	config->wapi_pairwise = hapd->conf->wapi_pairwise;
	config->wapi_pre_auth = hapd->conf->wapi_pre_auth;
	return 0;
}

int hostapd_get_config_from_wie(struct wapi_config *config, u8 *data, int data_len)
{
	int offset = 0;
	uint16_t version = 0;
	uint16_t n_akm = 0;
	uint8_t val_akm = 0;
	uint16_t i = 0;
	u32 oui_type = 0;
	u8 val_pairwise = 0;
	uint16_t n_pairwise = 0;

	if (data_len < MIN_WAPI_IE_LEN)
		return -1;

	version = WPA_GET_LE16(data + offset);
	if (version != 0x0001)
		return -1;
	offset += 2;
	//AKM
	n_akm = WPA_GET_LE16(data + offset);
	if (n_akm == 0)
		return -1;
	offset += 2;
	for (i = 0; i < n_akm; i++) {
		oui_type = WPA_GET_BE24(data + offset + 4 * i);
		if (oui_type != OUI_WAPI) {
			config->wapi_key_mgmt = 0;
			return -1;
		}
		val_akm = *(data + offset + 4 * i + 3);
		if (val_akm != WAPI_CERT && val_akm != WAPI_PSK) {
			config->wapi_key_mgmt = 0;
			return -1;
		}
		if (val_akm == WAPI_CERT)
			config->wapi_key_mgmt |= WPA_KEY_MGMT_WAPI_CERT;
		if (val_akm == WAPI_PSK)
			config->wapi_key_mgmt |= WPA_KEY_MGMT_WAPI_PSK;
	}
	offset += n_akm * 4;
	// unicast
	n_pairwise = WPA_GET_LE16(data + offset);
	if (n_pairwise == 0)
		return -1;
	offset += 2;
	for (i = 0; i < n_pairwise; i++) {
		oui_type = WPA_GET_BE24(data + offset + 4 * i);
		if (oui_type != OUI_WAPI) {
			config->wapi_key_mgmt = 0;
			config->wapi_pairwise = 0;
			return -1;
		}
		val_pairwise = *(data + offset + 4 * i + 3);
		if (val_pairwise != WPI_SMS4) {
			config->wapi_key_mgmt = 0;
			config->wapi_pairwise = 0;
			return -1;
		}
		if (val_pairwise == WPI_SMS4)
			config->wapi_pairwise |= WPA_CIPHER_SMS4;
	}
	offset += n_pairwise * 4;
	//broadcast
	oui_type = WPA_GET_BE24(data + offset);
	if (oui_type != OUI_WAPI) {
		config->wapi_key_mgmt = 0;
		config->wapi_pairwise = 0;
		return -1;
	}
	offset  += 3;
	val_pairwise = *(data + offset);
	if (val_pairwise != WPI_SMS4) {
		config->wapi_key_mgmt = 0;
		config->wapi_pairwise = 0;
		return -1;
	}
	offset += 1;
	//
	wpa_printf(MSG_INFO, "pre_auth = %d\n", config->wapi_pre_auth);
	if (*(data + offset) & (1 << 0))
		config->wapi_pre_auth = 1;

	wpa_printf(MSG_INFO, "%s:enable =%d, key_m= 0x%08x, pair_wise =0x%08x, pre_auth=%d\n",
			__func__, config->wapi, config->wapi_key_mgmt,
			config->wapi_pairwise, config->wapi_pre_auth);
	return 0;
}

int hostapd_get_wie_from_wapid(struct hostapd_data *hapd, struct wapid_config *config)
{
	struct wapi_authenticator *wapi_auth;

	wapi_auth = hapd->wapi_auth;
	if (!wapi_auth)
		return -1;

	if (!os_strcmp(hapd->conf->iface, config->iface)) {
		wapi_auth->wapi_ie = os_malloc(config->wie_len);
		if (!wapi_auth->wapi_ie)
			return -1;
		wapi_auth->wapi_ie_len = config->wie_len;
		os_memcpy(wapi_auth->wapi_ie, config->wie, config->wie_len);
		return 0;
	}
	return -1;
}

static void hostapd_wapi_auth_disconnect(void *ctx, const u8 *addr,
					u16 reason)
{
	struct hostapd_data *hapd = ctx;

	wpa_printf(MSG_INFO, "%s: WAPI authenticator requests disconnect: "
			"STA " MACSTR " reason %d",
			__func__, MAC2STR(addr), reason);
	ap_sta_disconnect(hapd, NULL, addr, reason);
}

int hostapd_setup_wapi(struct hostapd_data *hapd)
{
	struct wapi_config config;

	hostapd_get_wapi_config(hapd, &config);

	static struct wapi_auth_callbacks cb = {
		.disconnect = hostapd_wapi_auth_disconnect,
		.set_key = hostapd_wapi_auth_set_key,
	};
	hapd->wapi_auth = wapi_init(hapd->own_addr, &config, &cb, hapd);
	if (hapd->wapi_auth)
		wpa_printf(MSG_INFO, "%s: init wapi auth[%p]\n", hapd->wapi_auth);
	return 0;
}

void wapi_deinit(struct wapi_authenticator *wapi_auth)
{
	os_free(wapi_auth->wapi_ie);
	os_free(wapi_auth);
}

void wapi_auth_sta_deinit(struct wapi_state_machine *sm)
{
	if (!sm)
		return;
	sm->wapi_auth = NULL;
	os_free(sm);
}

void hostapd_deinit_wapi(struct hostapd_data *hapd)
{
	if (hapd->wapi_auth) {
		wapi_deinit(hapd->wapi_auth);
		hapd->wapi_auth = NULL;
	}
	if (hapd->wapid_buff) {
		os_free(hapd->wapid_buff);
		hapd->wapid_buff = NULL;
	}
	hapd->wapid_buff_len = 0;
}

const u8 *wapi_auth_get_wapi_ie(struct wapi_authenticator *wapi_auth, size_t *len)
{
	if (!wapi_auth)
		return NULL;
	*len = wapi_auth->wapi_ie_len;
	return wapi_auth->wapi_ie;
}

void hostapd_get_wapid_addr(struct wapi_authenticator *wapi_auth, u8 cmd, u8 dir,
					struct sockaddr_storage *sock_addr, socklen_t socklen)
{
	if (!wapi_auth)
		return;
	if (cmd == COMMAND_WAPI_CONFIG && dir == MESSAGE_REQUEST) {
		wpa_printf(MSG_INFO, "%s:get_wapid addr", __func__);

		if (sock_addr->ss_family == AF_UNIX)
			wpa_printf(MSG_INFO, "family is %d, sun_path=%s", sock_addr->ss_family,
					((struct sockaddr_un *)sock_addr)->sun_path);

		os_memcpy(&wapi_auth->wapid_addr, sock_addr, socklen);
		wapi_auth->socklen = socklen;
		wpa_printf(MSG_INFO, "---family is %d, sun_path=%s", sock_addr->ss_family,
				((struct sockaddr_un *)sock_addr)->sun_path);

	}
}

int wapi_auth_sm_event(struct wapi_state_machine *sm, enum wapi_event event, void *event_data)
{
	enum WAPI_AUTH_STATE current_state;
	struct sta_state *p_sta_state = NULL;
	struct wapi_key_info *p_key_info = NULL;
	struct sta_info *sta = NULL;

	if (!sm)
		return -1;
	current_state = sm->wapi_auth_state;

	wpa_printf(MSG_INFO, "%s: hostapd wapi auth sm event[%d]", __func__, event);
	switch (event) {
	case WAPI_EVENT_CERT_SUCCESS:
		if (current_state == WAPI_AUTH_CERT) {
			sm->wapi_auth_state = WAPI_AUTH_PTK;
			eloop_cancel_timeout(wapi_cert_timeout_handle, sm, NULL);
			// init timeout
			eloop_register_timeout(sm->wapi_uni_key_timeout, 0, wapi_unikey_timeout_handle, sm, NULL);
		}
		break;
	case WAPI_EVENT_AUTH_FAIL:
		p_sta_state = (struct sta_state *)event_data;
		sm->disconnect_reason = p_sta_state->reason_code;
		// cancel timeout function
		if (current_state == WAPI_AUTH_CERT)
			eloop_cancel_timeout(wapi_cert_timeout_handle, sm, NULL);
		if (current_state == WAPI_AUTH_PTK)
			eloop_cancel_timeout(wapi_unikey_timeout_handle, sm, NULL);
		if (current_state == WAPI_AUTH_GROUP)
			eloop_cancel_timeout(wapi_multikey_timeout_handle, sm, NULL);
		if (sm->wapi_auth->cb->disconnect)
			(sm->wapi_auth->cb->disconnect)(sm->wapi_auth->cb_ctx, sm->addr, sm->disconnect_reason);
		break;
	case WAPI_EVENT_PTK:
		if (current_state == WAPI_AUTH_PTK) {
			p_key_info = (struct wapi_key_info *)event_data;
			sm->wapi_auth_state = WAPI_AUTH_GROUP;
			eloop_cancel_timeout(wapi_unikey_timeout_handle, sm, NULL);
			/*set uni key to driver*/
			if (sm->wapi_auth->cb->set_key)
				(sm->wapi_auth->cb->set_key)(sm->wapi_auth->cb_ctx, 0, WPA_ALG_SMS4, sm->addr,
							p_key_info->key_index, p_key_info->key,
							p_key_info->key_len, KEY_FLAG_PAIRWISE_RX_TX);
			eloop_register_timeout(sm->wapi_multi_key_timeout, 0, wapi_multikey_timeout_handle, sm, NULL);
		}
		if (current_state == WAPI_AUTH_AUTHORIZED) {
			p_key_info = (struct wapi_key_info *)event_data;
			if (sm->wapi_auth->cb->set_key)
				(sm->wapi_auth->cb->set_key)(sm->wapi_auth->cb_ctx, 0, WPA_ALG_SMS4, sm->addr,
							p_key_info->key_index, p_key_info->key,
							p_key_info->key_len, KEY_FLAG_PAIRWISE_RX_TX);
		}
		break;
	case WAPI_EVENT_GTK:
		if (current_state == WAPI_AUTH_GROUP) {
			sm->wapi_auth_state = WAPI_AUTH_AUTHORIZED;
			eloop_cancel_timeout(wapi_multikey_timeout_handle, sm, NULL);
			p_sta_state = (struct sta_state *)event_data;
			if (p_sta_state) {
				sta = ap_get_sta(sm->wapi_auth->cb_ctx, p_sta_state->sta_mac);
				if (sta) {
					sta->flags |= WLAN_STA_AUTHORIZED;
					ieee802_1x_set_sta_authorized(sm->wapi_auth->cb_ctx,
									sta, 1);
					wpa_printf(MSG_INFO, "sta->flags= 0x%08x\n", sta->flags);
				}
			}
		}
		break;
	case WAPI_EVENT_DEAUTH:
		sm->wapi_auth_state = WAPI_DEAUTH;
		if (current_state == WAPI_AUTH_CERT)
			eloop_cancel_timeout(wapi_cert_timeout_handle, sm, NULL);
		if (current_state == WAPI_AUTH_PTK)
			eloop_cancel_timeout(wapi_unikey_timeout_handle, sm, NULL);
		if (current_state == WAPI_AUTH_GROUP)
			eloop_cancel_timeout(wapi_multikey_timeout_handle, sm, NULL);

		break;
	case WAPI_EVENT_DIASSOC:
		sm->wapi_auth_state = WAPI_DEAUTH;
		if (current_state == WAPI_AUTH_CERT)
			eloop_cancel_timeout(wapi_cert_timeout_handle, sm, NULL);
		if (current_state == WAPI_AUTH_PTK)
			eloop_cancel_timeout(wapi_unikey_timeout_handle, sm, NULL);
		if (current_state == WAPI_AUTH_GROUP)
			eloop_cancel_timeout(wapi_multikey_timeout_handle, sm, NULL);

		break;
	case WAPI_EVENT_ASSOC:
		if (sm->wapi_auth->wapi_config.wapi_key_mgmt & WPA_KEY_MGMT_WAPI_CERT) {
			sm->wapi_auth_state = WAPI_AUTH_CERT;
			eloop_register_timeout(sm->wapi_cert_timeout, 0, wapi_cert_timeout_handle, sm, NULL);
		}
		if (sm->wapi_auth->wapi_config.wapi_key_mgmt & WPA_KEY_MGMT_WAPI_PSK) {
			sm->wapi_auth_state = WAPI_AUTH_PTK;
			eloop_register_timeout(sm->wapi_uni_key_timeout, 0, wapi_unikey_timeout_handle, sm, NULL);
		}
		break;
	}

	return 0;
}
