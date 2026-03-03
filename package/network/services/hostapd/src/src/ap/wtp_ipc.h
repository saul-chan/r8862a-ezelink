/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef WPT_IPC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dirent.h>
#include <stdbool.h>
#include "list.h"
#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "common/wpa_common.h"
#include "ap/wpa_auth.h"
#include "ap/wpa_auth_i.h"

#define HOSTAPD_EVENT_PATH "/var/run/hostapd_wtp_event"
#define WTP_EVENT_SERVER_PATH "/var/run/wtp/wtp_hostapd_event"
#define HOSTAPD_DIR "/var/run/hostapd"
#define HOSTAPD_DATA_PREFIX "/var/run/hostapd_data"
#define WTP_DATA_SERVER_PATH "/var/run/wtp/wtp_hostapd_data"
#define STA_EAPOL_PATH "/var/run/wtp/hostapd_eapol"
#define WTP_RESPONSE_MAX_LEN 512
#define WTP_CONNECT_MAX_RETRY 5
#define MAX_FILE_PATH 64

#define RESPONSE_SUCCESS "success"
#define RESPONSE_FAIL "fail"

extern int capwap_enable;
extern int capwap_auth_enable;

struct hostapd_wtp_conn {
	int sock;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};

struct hostapd_wtp_conn_item {
	struct dl_list list;
	char ifname[32];
	struct hostapd_wtp_conn conn_sock;
};

struct hostapd_wtp_event_msg {
	int event;
	char data[256];
};

enum wtp_ipc_event {
	HOSTAPD_WTP_INTERFACE_INIT = 1,
	HOSTAPD_WTP_STA_DISCONNECTED
};

struct hostapd_wtp_conn *hostapd_wtp_conn_add(char *ifname);
int hostapd_wtp_conn_del(char *ifname);
int hostapd_send_frame_to_wtp(char *ifname, const struct ieee80211_mgmt *data, size_t len);
struct dl_list *hostapd_wtp_data_socket_init(void);
int hostapd_wtp_event_send(int event, char *data);
int hostapd_wtp_event_socket_init(void);
int hostapd_wtp_eapol_socket_init(struct wpa_state_machine *sm);
int hostapd_send_eapol_to_wtp(struct wpa_state_machine *sm, const u8 *data, size_t data_len);
size_t hostapd_receive_eapol_from_wtp(struct wpa_state_machine *sm, char *data);

#endif

