/*
 * hostapd- wapi GB 15629.11
 * Copyright (c) 2024, Clourney Semi
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WAPI_AUTH_I_H
#define WAPI_AUTH_I_H
#include <sys/un.h>

#define WAPI_CERT_TIMEOUT	31
#define WAPI_UNI_KEY_TIMEOUT	30
#define WAPI_MULTI_KEY_TIMEOUT	30

#define COMMAND_WAPI_CONFIG	1
#define COMMAND_STA_WAPI_SET_UNI_KEY	2
#define COMMAND_STA_WAPI_SET_MULTI_KEY	3

#define EVENT_STA_HAPD_DEAUTH	4
#define EVENT_STA_HAPD_ASSOC_SUCCESS	5
#define EVENT_STA_HAPD_ASSOC_FAIL	6
#define EVENT_STA_WAPI_CERT_AUTH_SUCCESS	7
#define EVENT_STA_WAPI_PORT_OPEN	8
#define EVENT_STA_WAPI_DEAUTH	99

#define MESSAGE_REQUEST	0
#define MESSAGE_REPLY	1

#define REPLY_OK	0
#define REPLY_FAIL	1

#define WAPI_KEY_ALG_SMS4	0x01
#define WAPI_KEY_CRYPT_MODE_OFB_CBC	0x0
#define WAPI_KEY_ADD	0x01
#define WAPI_KEY_DEL	0x00

#define MINI_WAPI_DATA_LEN	5
#define WAPI_REPLY_LEN 1

struct wapi_auth_callbacks {
	void (*disconnect)(void *ctx, const u8 *addr, u16 reason);
	int (*set_key)(void *ctx, int vlan_id, enum wpa_alg alg,
		const u8 *addr, int idx, u8 *key, size_t key_len,
		enum key_flag key_flag);
};

enum WAPI_AUTH_STATE {
		WAPI_AUTH_CERT, WAPI_AUTH_PTK,
		WAPI_AUTH_GROUP, WAPI_AUTH_AUTHORIZED, WAPI_DEAUTH
};

struct wapi_config {
	int wapi;
	int wapi_key_mgmt;
	int wapi_pre_auth;
	int wapi_pairwise;
};

struct wapi_authenticator {
	u8 addr[ETH_ALEN];
	u8 *wapi_ie;
	size_t wapi_ie_len;
	struct wapi_auth_callbacks *cb;
	struct wapi_config wapi_config;
	struct sockaddr_storage wapid_addr;
	socklen_t socklen;
	void *cb_ctx;
};

struct wapi_state_machine {
	struct wapi_authenticator *wapi_auth;
	u8 addr[ETH_ALEN];
	enum WAPI_AUTH_STATE wapi_auth_state;
	int wapi_cert_timeout;
	int wapi_uni_key_timeout;
	int wapi_multi_key_timeout;
	u16 disconnect_reason;
};

struct sta_state {
	u8 ap_mac[6];
	u8 sta_mac[6];
	u8 status; //0: success; 1:fail
	u16 reason_code; // meaningfull for message from wapid  to hostapd
};

struct wapi_sta_info {
	u16 type;
	u16 data_len;
	u8 ap_mac[6];
	u8 pad1[2];
	u8 sta_mac[6];
	u8 pad2[2];
	u8 data_packet_num[16]; /*mcast notofier sequence number*/
	u8 wie[256];
};

struct wapi_key_info {
	u8 action;
	u8 key_alg;
	u8 key_crypt_mode;
	u8 key_index;
	u32 key_len;
	u8 key[64];
	u8 ap_mac[6];
	u8 sta_mac[6];
};


struct wapid_config {
	char iface[IFNAMSIZ + 1];
	u32 wie_len;
	u8 wie[128];
};


struct wapi_reply {
	u8 status;
};
enum wapi_event {WAPI_EVENT_CERT_SUCCESS, WAPI_EVENT_AUTH_FAIL, WAPI_EVENT_PTK,
	WAPI_EVENT_GTK, WAPI_EVENT_DEAUTH, WAPI_EVENT_DIASSOC, WAPI_EVENT_ASSOC};

#endif
