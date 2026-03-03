/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "autogened_ubus_wifi.h"
#include <rpcd/plugin.h>

static inline enum clsapi_wifi_band strband_to_enum(const char *str_band)
{
	if (strcmp(str_band, "2") == 0 || strcasecmp(str_band, "2GHz") == 0 || strcasecmp(str_band, "2G") == 0)
		return CLSAPI_BAND_2GHZ;
	else if (strcmp(str_band, "5") == 0 || strcasecmp(str_band, "5GHz") == 0 || strcasecmp(str_band, "5G") == 0)
		return CLSAPI_BAND_5GHZ;
	else if (strcasecmp(str_band, "6") == 0 || strcasecmp(str_band, "6GHz") == 0 || strcasecmp(str_band, "6G") == 0)
		return CLSAPI_BAND_6GHZ;
	else if (strcmp(str_band, "0") == 0 || strcasecmp(str_band, "current") == 0 || strcasecmp(str_band, "default") == 0)
		return CLSAPI_BAND_DEFAULT;
	else
		return CLSAPI_BAND_NOSUCH_BAND;
}

static inline void get_str_band(const enum clsapi_wifi_band band, char str_band[16])
{
	if (band == CLSAPI_BAND_2GHZ)
		strcpy(str_band, "2GHz");
	else if (band == CLSAPI_BAND_5GHZ)
		strcpy(str_band, "5GHz");
	else if (band == CLSAPI_BAND_6GHZ)
		strcpy(str_band, "6GHz");
	else
		strcpy(str_band, "unknown");
}

enum {
	WIFI_GET_CSI_STA_LIST_PRIMARY_IFNAME,
	__WIFI_GET_CSI_STA_LIST_MAX,
};

static const struct blobmsg_policy wifi_get_csi_sta_list_policy[__WIFI_GET_CSI_STA_LIST_MAX] = {
	[WIFI_GET_CSI_STA_LIST_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_csi_sta_list(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int list_num = 0;
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_CSI_STA_LIST_MAX];
	uint8_t (*sta_mac_list)[ETH_ALEN] = NULL;
	string_32 str_mac = {0};

	blobmsg_parse(wifi_get_csi_sta_list_policy, __WIFI_GET_CSI_STA_LIST_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_CSI_STA_LIST_PRIMARY_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_csi_sta_list(blobmsg_get_string(tb[WIFI_GET_CSI_STA_LIST_PRIMARY_IFNAME]), &sta_mac_list);
	if (ret < 0)
		return -ret;

	list_num = ret;
	if (list_num > 0 && sta_mac_list == NULL)
		return -CLSAPI_ERR_NO_MEM;

	blob_buf_init(&buf, 0);
	void *ptr_sta_mac_array = blobmsg_open_array(&buf, "station array");

	for (int i = 0; i < list_num; i++) {
		snprintf(str_mac, sizeof(str_mac), MACFMT, MACARG(sta_mac_list[i]));
		blobmsg_add_string(&buf, "macaddr", str_mac);
	}
	blobmsg_close_array(&buf, ptr_sta_mac_array);

	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_CHECK_WPS_PIN_PIN_CODE,
	__WIFI_CHECK_WPS_PIN_MAX,
};

static const struct blobmsg_policy wifi_check_wps_pin_policy[__WIFI_CHECK_WPS_PIN_MAX] = {
	[WIFI_CHECK_WPS_PIN_PIN_CODE] = { .name = "pin_code", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_wifi_check_wps_pin(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_CHECK_WPS_PIN_MAX];

	blobmsg_parse(wifi_check_wps_pin_policy, __WIFI_CHECK_WPS_PIN_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_CHECK_WPS_PIN_PIN_CODE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_check_wps_pin(blobmsg_get_u32(tb[WIFI_CHECK_WPS_PIN_PIN_CODE]));

	return -ret;
}

enum {
	WIFI_GET_STA_INFO_IFNAME,
	WIFI_GET_STA_INFO_STA_MAC,
	__WIFI_GET_STA_INFO_MAX,
};

static const struct blobmsg_policy wifi_get_sta_info_policy[__WIFI_GET_STA_INFO_MAX] = {
	[WIFI_GET_STA_INFO_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_GET_STA_INFO_STA_MAC] = { .name = "sta_mac", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_sta_info(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_STA_INFO_MAX];
	struct sta_info sta;
	uint8_t sta_mac[ETH_ALEN];
	string_32 str_mac = {0};

	blobmsg_parse(wifi_get_sta_info_policy, __WIFI_GET_STA_INFO_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_STA_INFO_IFNAME] || !tb[WIFI_GET_STA_INFO_STA_MAC])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = mac_aton(blobmsg_get_string(tb[WIFI_GET_STA_INFO_STA_MAC]), sta_mac);
	if (ret)
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_sta_info(blobmsg_get_string(tb[WIFI_GET_STA_INFO_IFNAME]), sta_mac, &sta);
	blob_buf_init(&buf, 0);
	void *ptr_sta = blobmsg_open_table(&buf, "sta");

	snprintf(str_mac, sizeof(str_mac), MACFMT, MACARG(sta.mac));
	blobmsg_add_string(&buf, "mac", str_mac);
	blobmsg_add_u64(&buf, "tx_packets", sta.tx_packets);
	blobmsg_add_u64(&buf, "rx_packets", sta.rx_packets);
	blobmsg_add_u64(&buf, "tx_bytes", sta.tx_bytes);
	blobmsg_add_u64(&buf, "rx_bytes", sta.rx_bytes);
	blobmsg_add_u64(&buf, "tx_airtime", sta.tx_airtime);
	blobmsg_add_u64(&buf, "rx_airtime", sta.rx_airtime);
	blobmsg_add_u32(&buf, "bytes_64bit", sta.bytes_64bit);
	blobmsg_add_u64(&buf, "current_tx_rate", sta.current_tx_rate);
	blobmsg_add_u64(&buf, "current_rx_rate", sta.current_rx_rate);
	blobmsg_add_u64(&buf, "inactive_msec", sta.inactive_msec);
	blobmsg_add_u64(&buf, "connected_sec", sta.connected_sec);
	blobmsg_add_u64(&buf, "flags", sta.flags);
	blobmsg_add_u64(&buf, "num_ps_buf_frames", sta.num_ps_buf_frames);
	blobmsg_add_u64(&buf, "tx_retry_failed", sta.tx_retry_failed);
	blobmsg_add_u64(&buf, "tx_retry_count", sta.tx_retry_count);
	blobmsg_add_u32(&buf, "last_ack_rssi", sta.last_ack_rssi);
	blobmsg_add_u64(&buf, "backlog_packets", sta.backlog_packets);
	blobmsg_add_u64(&buf, "backlog_bytes", sta.backlog_bytes);
	blobmsg_add_u32(&buf, "signal", sta.signal);
	blobmsg_add_u32(&buf, "rx_vhtmcs", sta.rx_vhtmcs);
	blobmsg_add_u32(&buf, "tx_vhtmcs", sta.tx_vhtmcs);
	blobmsg_add_u32(&buf, "rx_mcs", sta.rx_mcs);
	blobmsg_add_u32(&buf, "tx_mcs", sta.tx_mcs);
	blobmsg_add_u32(&buf, "rx_vht_nss", sta.rx_vht_nss);
	blobmsg_add_u32(&buf, "tx_vht_nss", sta.tx_vht_nss);
	blobmsg_close_table(&buf, ptr_sta);

	ret = ubus_send_reply(ctx, req, buf.head);
	if (ret != CLSAPI_OK)
		ret = UBUS_STATUS_SYSTEM_ERROR;

	return ret;
}

enum {
	WIFI_GET_MACFILTER_MACLIST_IFNAME,
	__WIFI_GET_MACFILTER_MACLIST_MAX,
};

static const struct blobmsg_policy wifi_get_macfilter_maclist_policy[__WIFI_GET_MACFILTER_MACLIST_MAX] = {
	[WIFI_GET_MACFILTER_MACLIST_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_macfilter_maclist(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int list_num = 0;
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_MACFILTER_MACLIST_MAX];
	uint8_t (*mac_list)[ETH_ALEN] = NULL;
	string_32 str_mac = {0};

	blobmsg_parse(wifi_get_macfilter_maclist_policy, __WIFI_GET_MACFILTER_MACLIST_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_MACFILTER_MACLIST_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_macfilter_maclist(blobmsg_get_string(tb[WIFI_GET_MACFILTER_MACLIST_IFNAME]), &mac_list);
	if (ret < 0)
		return -ret;

	list_num = ret;
	if (list_num > 0 && mac_list == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);
	void *ptr_mac_array = blobmsg_open_array(&buf, "mac list");

	for (int i = 0; i < list_num; i++) {
		snprintf(str_mac, sizeof(str_mac), MACFMT, MACARG(mac_list[i]));
		blobmsg_add_string(&buf, "", str_mac);
	}
	blobmsg_close_array(&buf, ptr_mac_array);
	ret = ubus_send_reply(ctx, req, buf.head);

	if (mac_list)
		free(mac_list);

	return ret;
}

enum {
	WIFI_GET_BSSID_IFNAME,
	__WIFI_GET_BSSID_MAX,
};

static const struct blobmsg_policy wifi_get_bssid_policy[__WIFI_GET_BSSID_MAX] = {
	[WIFI_GET_BSSID_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_bssid(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_BSSID_MAX];
	uint8_t bssid[ETH_ALEN];
	string_32 str_bssid = {0};

	blobmsg_parse(wifi_get_bssid_policy, __WIFI_GET_BSSID_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_BSSID_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_bssid(blobmsg_get_string(tb[WIFI_GET_BSSID_IFNAME]), bssid);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		snprintf(str_bssid, sizeof(str_bssid), MACFMT, MACARG(bssid));
		blobmsg_add_string(&buf, "BSSID", str_bssid);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return ret;
}

enum {
	WIFI_GET_SCAN_ENTRY_PRIMARY_IFNAME,
	WIFI_GET_SCAN_ENTRY_AP_IDX,
	__WIFI_GET_SCAN_ENTRY_MAX,
};

static const struct blobmsg_policy wifi_get_scan_entry_policy[__WIFI_GET_SCAN_ENTRY_MAX] = {
	[WIFI_GET_SCAN_ENTRY_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_GET_SCAN_ENTRY_AP_IDX] = { .name = "ap_idx", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_wifi_get_scan_entry(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_SCAN_ENTRY_MAX];
	struct clsapi_scan_ap_info *scan_ap_array;
	int scan_ap_cnt = 0;

	blobmsg_parse(wifi_get_scan_entry_policy, __WIFI_GET_SCAN_ENTRY_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_SCAN_ENTRY_PRIMARY_IFNAME] || !tb[WIFI_GET_SCAN_ENTRY_AP_IDX])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_scan_entry(blobmsg_get_string(tb[WIFI_GET_SCAN_ENTRY_PRIMARY_IFNAME]), blobmsg_get_u32(tb[WIFI_GET_SCAN_ENTRY_AP_IDX]), &scan_ap_array);
	if (ret < 0)
		return -ret;
	scan_ap_cnt = ret;

	if (scan_ap_array == NULL)
		return UBUS_STATUS_SYSTEM_ERROR;

	blob_buf_init(&buf, 0);

	void *ptr_scan_ap_array = blobmsg_open_array(&buf, "scan_ap_array");
	for (int i = 0; i < scan_ap_cnt; i++) {
		void *ptr_scan_ap_tab = blobmsg_open_table(&buf, "scan_ap");
		char mac_str[18];

		snprintf(mac_str, sizeof(mac_str), MACFMT, MACARG(scan_ap_array[i].bssid));
		blobmsg_add_string(&buf, "bssid", mac_str);
		blobmsg_add_string(&buf, "ssid", scan_ap_array[i].ssid);
		blobmsg_add_u32(&buf, "channel", scan_ap_array[i].channel);
		blobmsg_add_u32(&buf, "rssi", scan_ap_array[i].rssi);
		blobmsg_add_u32(&buf, "hwmodes", scan_ap_array[i].hwmodes);
		blobmsg_add_u32(&buf, "max_bw", scan_ap_array[i].max_bw);
		blobmsg_add_u32(&buf, "flags", scan_ap_array[i].flags);
		void *ptr_crypto = blobmsg_open_table(&buf, "crypto");

		blobmsg_add_u32(&buf, "sec_protos", scan_ap_array[i].crypto.sec_protos);
		blobmsg_add_u32(&buf, "auth_modes", scan_ap_array[i].crypto.auth_modes);
		blobmsg_add_u32(&buf, "auth_suites", scan_ap_array[i].crypto.auth_suites);
		blobmsg_add_u32(&buf, "group_ciphers", scan_ap_array[i].crypto.group_ciphers);
		blobmsg_add_u32(&buf, "pair_ciphers", scan_ap_array[i].crypto.pair_ciphers);
		blobmsg_close_table(&buf, ptr_crypto);

		blobmsg_close_table(&buf, ptr_scan_ap_tab);
	}

	blobmsg_close_array(&buf, ptr_scan_ap_array);
	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}


enum {
	WIFI_DISASSOC_STA_IFNAME,
	WIFI_DISASSOC_STA_MAC,
	WIFI_DISASSOC_STA_REASON_CODE,
	__WIFI_DISASSOC_STA_MAX,
};

static const struct blobmsg_policy wifi_disassoc_sta_policy[__WIFI_DISASSOC_STA_MAX] = {
	[WIFI_DISASSOC_STA_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING},
	[WIFI_DISASSOC_STA_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING},
	[WIFI_DISASSOC_STA_REASON_CODE] = { .name = "reason_code", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_wifi_disassoc_sta(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_DISASSOC_STA_MAX];
	uint8_t mac[ETH_ALEN];

	blobmsg_parse(wifi_disassoc_sta_policy, __WIFI_DISASSOC_STA_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_DISASSOC_STA_IFNAME] || !tb[WIFI_DISASSOC_STA_MAC] || !tb[WIFI_DISASSOC_STA_REASON_CODE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = mac_aton(blobmsg_get_string(tb[WIFI_DISASSOC_STA_MAC]), mac);
	if (ret < 0)
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_disassoc_sta(blobmsg_get_string(tb[WIFI_DISASSOC_STA_IFNAME]), mac, blobmsg_get_u32(tb[WIFI_DISASSOC_STA_REASON_CODE]));

	return -ret;
}

enum {
	WIFI_DEAUTH_STA_IFNAME,
	WIFI_DEAUTH_STA_MAC,
	WIFI_DEAUTH_STA_REASON_CODE,
	__WIFI_DEAUTH_STA_MAX,
};

static const struct blobmsg_policy wifi_deauth_sta_policy[__WIFI_DEAUTH_STA_MAX] = {
	[WIFI_DEAUTH_STA_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING},
	[WIFI_DEAUTH_STA_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING},
	[WIFI_DEAUTH_STA_REASON_CODE] = { .name = "reason_code", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_wifi_deauth_sta(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	uint8_t mac[ETH_ALEN];
	struct blob_attr *tb[__WIFI_DEAUTH_STA_MAX];

	blobmsg_parse(wifi_deauth_sta_policy, __WIFI_DEAUTH_STA_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_DEAUTH_STA_IFNAME] || !tb[WIFI_DEAUTH_STA_MAC] || !tb[WIFI_DEAUTH_STA_REASON_CODE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = mac_aton(blobmsg_get_string(tb[WIFI_DEAUTH_STA_MAC]), mac);
	if (ret < 0)
		return UBUS_STATUS_INVALID_ARGUMENT;
	ret = clsapi_wifi_deauth_sta(blobmsg_get_string(tb[WIFI_DEAUTH_STA_IFNAME]), mac, blobmsg_get_u32(tb[WIFI_DEAUTH_STA_REASON_CODE]));

	return -ret;
}

enum {
	WIFI_SET_WEP_KEY_IFNAME,
	WIFI_SET_WEP_KEY_IDX_TO_USE,
	WIFI_SET_WEP_KEY_WEP_KEY,
	__WIFI_SET_WEP_KEY_MAX,
};

static const struct blobmsg_policy wifi_set_wep_key_policy[__WIFI_SET_WEP_KEY_MAX] = {
	[WIFI_SET_WEP_KEY_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_SET_WEP_KEY_IDX_TO_USE] = { .name = "idx_to_use", .type = BLOBMSG_TYPE_INT32 },
	[WIFI_SET_WEP_KEY_WEP_KEY] = { .name = "wep_key", .type = BLOBMSG_TYPE_ARRAY },
};

static int ubus_wifi_set_wep_key(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	struct blob_attr *cur;
	int ret = CLSAPI_OK, i = 0, rem = 0;
	struct blob_attr *tb[__WIFI_SET_WEP_KEY_MAX];
	char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64];

	blobmsg_parse(wifi_set_wep_key_policy, __WIFI_SET_WEP_KEY_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[WIFI_SET_WEP_KEY_IFNAME] || !tb[WIFI_SET_WEP_KEY_IDX_TO_USE] || !tb[WIFI_SET_WEP_KEY_WEP_KEY])
		return UBUS_STATUS_INVALID_ARGUMENT;


	blobmsg_for_each_attr(cur, tb[WIFI_SET_WEP_KEY_WEP_KEY], rem) {
		if (i >= CLSAPI_WIFI_MAX_PRESHARED_KEY)
			return UBUS_STATUS_INVALID_ARGUMENT;

		if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING) {
			cls_strncpy(wep_key[i], blobmsg_get_string(cur), 63);
			wep_key[i][63] = '\0'; // Ensure null termination
		} else
			return UBUS_STATUS_INVALID_ARGUMENT;

		i++;
	}

	for (; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++)
		cls_strncpy(wep_key[i], "", 64);

	ret = clsapi_wifi_set_wep_key(blobmsg_get_string(tb[WIFI_SET_WEP_KEY_IFNAME]), blobmsg_get_u32(tb[WIFI_SET_WEP_KEY_IDX_TO_USE]), wep_key);

	return -ret;
}

enum {
	WIFI_SET_SSID_IFNAME,
	WIFI_SET_SSID_SSID,
	__WIFI_SET_SSID_MAX,
};

static const struct blobmsg_policy wifi_set_ssid_policy[__WIFI_SET_SSID_MAX] = {
	[WIFI_SET_SSID_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_SET_SSID_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_set_ssid(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_SET_SSID_MAX];

	blobmsg_parse(wifi_set_ssid_policy, __WIFI_SET_SSID_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_SET_SSID_IFNAME] || !tb[WIFI_SET_SSID_SSID])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_set_ssid(blobmsg_get_string(tb[WIFI_SET_SSID_IFNAME]), blobmsg_get_string(tb[WIFI_SET_SSID_SSID]));

	return -ret;
}

enum {
	WIFI_ADD_BSS_PHYNAME,
	WIFI_ADD_BSS_BSSID,
	__WIFI_ADD_BSS_MAX,
};

static const struct blobmsg_policy wifi_add_bss_policy[__WIFI_ADD_BSS_MAX] = {
	[WIFI_ADD_BSS_PHYNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_ADD_BSS_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_add_bss(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_ADD_BSS_MAX];
	string_32 created_ifname;

	blobmsg_parse(wifi_add_bss_policy, __WIFI_ADD_BSS_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_ADD_BSS_PHYNAME] || !tb[WIFI_ADD_BSS_BSSID])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_add_bss(blobmsg_get_string(tb[WIFI_ADD_BSS_PHYNAME]), handle_get_mac_string_to_int(blobmsg_get_string(tb[WIFI_ADD_BSS_BSSID])), created_ifname);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		blobmsg_add_string(&buf, "ifname", (char *)created_ifname);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return ret;
}

enum {
	WIFI_GET_CHAN_SCORE_PRIMARY_IFNAME,
	WIFI_GET_CHAN_SCORE_BAND,
	__WIFI_GET_CHAN_SCORE_MAX,
};

static const struct blobmsg_policy wifi_get_chan_score_policy[__WIFI_GET_CHAN_SCORE_MAX] = {
	[WIFI_GET_CHAN_SCORE_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_GET_CHAN_SCORE_BAND] = { .name = "band", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_wifi_get_chan_score(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int score_len = 0;
	int ret = CLSAPI_OK;
	struct chan_score *score_array;
	struct blob_attr *tb[__WIFI_GET_CHAN_SCORE_MAX];

	blobmsg_parse(wifi_get_chan_score_policy, __WIFI_GET_CHAN_SCORE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_CHAN_SCORE_PRIMARY_IFNAME] || !tb[WIFI_GET_CHAN_SCORE_BAND])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_chan_score(blobmsg_get_string(tb[WIFI_GET_CHAN_SCORE_PRIMARY_IFNAME]), strband_to_enum(blobmsg_get_string(tb[WIFI_GET_CHAN_SCORE_BAND])), &score_array);
	if (ret < 0)
		return -ret;

	score_len = ret;
	if (score_len > 0 && score_array == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);

	void *ptr_score_array = blobmsg_open_array(&buf, "score_array");
	for (int i = 0; i < score_len; i++) {
		void *ptr_score_tab = blobmsg_open_table(&buf, "score");

		blobmsg_add_u32(&buf, "chan", score_array->chan);
		blobmsg_add_u32(&buf, "score", score_array->score);
		blobmsg_close_table(&buf, ptr_score_tab);
	}

	blobmsg_close_array(&buf, ptr_score_array);
	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_GET_SURVEY_PRIMARY_IFNAME,
	__WIFI_GET_SURVEY_MAX,
};

static const struct blobmsg_policy wifi_get_survey_policy[__WIFI_GET_SURVEY_MAX] = {
	[WIFI_GET_SURVEY_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_survey(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	uint8_t list_len = 0;
	struct blob_attr *tb[__WIFI_GET_SURVEY_MAX];
	struct survey_entry *survey_array;

	blobmsg_parse(wifi_get_survey_policy, __WIFI_GET_SURVEY_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_SURVEY_PRIMARY_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_survey(blobmsg_get_string(tb[WIFI_GET_SURVEY_PRIMARY_IFNAME]), &survey_array);
	if (ret < 0)
		return -ret;

	list_len = ret;
	if (list_len > 0 && survey_array == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);
	void *ptr_survery_array = blobmsg_open_array(&buf, "survey array");
	for (int i = 0; i < list_len; i++) {
		void *ptr_survey_tab = blobmsg_open_table(&buf, "survey_array");

		blobmsg_add_u64(&buf, "active_time", survey_array->active_time);
		blobmsg_add_u64(&buf, "busy_time", survey_array->busy_time);
		blobmsg_add_u64(&buf, "busy_time_ext", survey_array->busy_time_ext);
		blobmsg_add_u64(&buf, "rxtime", survey_array->rxtime);
		blobmsg_add_u64(&buf, "txtime", survey_array->txtime);
		blobmsg_add_u32(&buf, "mhz", survey_array->mhz);
		blobmsg_add_u32(&buf, "noise", survey_array->noise);
		blobmsg_close_table(&buf, ptr_survey_tab);

	}
	blobmsg_close_array(&buf, ptr_survery_array);

	if (survey_array)
		free(survey_array);
	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_GET_SUPPORTED_CHANNELS_PRIMARY_IFNAME,
	WIFI_GET_SUPPORTED_CHANNELS_BAND,
	WIFI_GET_SUPPORTED_CHANNELS_BW,
	__WIFI_GET_SUPPORTED_CHANNELS_MAX,
};

static const struct blobmsg_policy wifi_get_supported_channels_policy[__WIFI_GET_SUPPORTED_CHANNELS_MAX] = {
	[WIFI_GET_SUPPORTED_CHANNELS_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_GET_SUPPORTED_CHANNELS_BAND] = { .name = "band", .type = BLOBMSG_TYPE_INT32 },
	[WIFI_GET_SUPPORTED_CHANNELS_BW] = { .name = "bw", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_wifi_get_supported_channels(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_SUPPORTED_CHANNELS_MAX];
	uint8_t *chan_array = NULL;
	uint8_t list_len = 0;

	blobmsg_parse(wifi_get_supported_channels_policy, __WIFI_GET_SUPPORTED_CHANNELS_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_SUPPORTED_CHANNELS_PRIMARY_IFNAME] || !tb[WIFI_GET_SUPPORTED_CHANNELS_BAND] || !tb[WIFI_GET_SUPPORTED_CHANNELS_BW])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_supported_channels(blobmsg_get_string(tb[WIFI_GET_SUPPORTED_CHANNELS_PRIMARY_IFNAME]), blobmsg_get_u32(tb[WIFI_GET_SUPPORTED_CHANNELS_BAND]), blobmsg_get_u32(tb[WIFI_GET_SUPPORTED_CHANNELS_BW]), &chan_array);
	if (ret < 0)
		return -ret;

	list_len = ret;
	if (list_len > 0 && chan_array == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);
	void *ptr_chan_list = blobmsg_open_array(&buf, "channels list");
	for (int i = 0; i < list_len; i++) {
		blobmsg_add_u32(&buf, "chan_array", chan_array[i]);
	}
	blobmsg_close_array(&buf, ptr_chan_list);

	if (chan_array)
		free(chan_array);

	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_GET_SUPPORTED_BWS_PRIMARY_IFNAME,
	__WIFI_GET_SUPPORTED_BWS_MAX,
};

static const struct blobmsg_policy wifi_get_supported_bws_policy[__WIFI_GET_SUPPORTED_BWS_MAX] = {
	[WIFI_GET_SUPPORTED_BWS_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_supported_bws(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_SUPPORTED_BWS_MAX];
	enum clsapi_wifi_bw *bw_array;
	uint8_t bands_len = 0;

	blobmsg_parse(wifi_get_supported_bws_policy, __WIFI_GET_SUPPORTED_BWS_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_SUPPORTED_BWS_PRIMARY_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_supported_bws(blobmsg_get_string(tb[WIFI_GET_SUPPORTED_BWS_PRIMARY_IFNAME]), &bw_array);
	if (ret < 0)
		return -ret;

	bands_len = ret;
	if (bands_len > 0 && bw_array == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);
	void *ptr_bw_list = blobmsg_open_array(&buf, "band width list");
	for (int i = 0; i < bands_len; i++) {
		blobmsg_add_u32(&buf, "bw_array", bw_array[i]);
	}
	blobmsg_close_array(&buf, ptr_bw_list);

	if (bw_array)
		free(bw_array);

	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_GET_SUPPORTED_BANDS_PRIMARY_IFNAME,
	__WIFI_GET_SUPPORTED_BANDS_MAX,
};

static const struct blobmsg_policy wifi_get_supported_bands_policy[__WIFI_GET_SUPPORTED_BANDS_MAX] = {
	[WIFI_GET_SUPPORTED_BANDS_PRIMARY_IFNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_supported_bands(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_SUPPORTED_BANDS_MAX];
	enum clsapi_wifi_band *band_array = NULL;
	int bands_len = 0;
	string_16 str_band = {0};

	blobmsg_parse(wifi_get_supported_bands_policy, __WIFI_GET_SUPPORTED_BANDS_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_SUPPORTED_BANDS_PRIMARY_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_supported_bands(blobmsg_get_string(tb[WIFI_GET_SUPPORTED_BANDS_PRIMARY_IFNAME]), &band_array);

	if (ret < 0)
		return -ret;

	bands_len = ret;
	if (bands_len > 0 && band_array == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);
	void *ptr_band_list = blobmsg_open_array(&buf, "band list");
	for (int i = 0; i < bands_len; i++) {
		get_str_band(band_array[i], str_band);
		blobmsg_add_string(&buf, "band", str_band);
	}

	blobmsg_close_array(&buf, ptr_band_list);

	if (band_array)
		free(band_array);

	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_GET_SSID_IFNAME,
	__WIFI_GET_SSID_MAX,
};

static const struct blobmsg_policy wifi_get_ssid_policy[__WIFI_GET_SSID_MAX] = {
	[WIFI_GET_SSID_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_ssid(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_SSID_MAX];
	string_1024 ssid = {0};

	blobmsg_parse(wifi_get_ssid_policy, __WIFI_GET_SSID_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_SSID_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_ssid(blobmsg_get_string(tb[WIFI_GET_SSID_IFNAME]), ssid);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		blobmsg_add_string(&buf, "ssid", (char *)ssid);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return ret;
}

enum {
	WIFI_GET_WEP_KEY_IFNAME,
	__WIFI_GET_WEP_KEY_MAX,
};

static const struct blobmsg_policy wifi_get_wep_key_policy[__WIFI_GET_WEP_KEY_MAX] = {
	[WIFI_GET_WEP_KEY_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_wep_key(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK, idx_in_use;
	struct blob_attr *tb[__WIFI_GET_WEP_KEY_MAX];
	char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64];

	blobmsg_parse(wifi_get_wep_key_policy, __WIFI_GET_WEP_KEY_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_WEP_KEY_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_wep_key(blobmsg_get_string(tb[WIFI_GET_WEP_KEY_IFNAME]), &idx_in_use, wep_key);
	if (ret < 0)
		return UBUS_STATUS_NOT_SUPPORTED;

	blob_buf_init(&buf, 0);
	void *ptr_wep_list = blobmsg_open_array(&buf, "wep key list");

	for (int i = 0; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++) {
		if (i == idx_in_use)
			blobmsg_add_string(&buf, " (*)", wep_key[i]);
		else
			blobmsg_add_string(&buf, "    ", wep_key[i]);
	}

	blobmsg_close_array(&buf, ptr_wep_list);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	WIFI_GET_ASSOC_LIST_IFNAME,
	__WIFI_GET_ASSOC_LIST_MAX,
};

static const struct blobmsg_policy wifi_get_assoc_list_policy[__WIFI_GET_ASSOC_LIST_MAX] = {
	[WIFI_GET_ASSOC_LIST_IFNAME] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_assoc_list(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	int list_num = 0;
	uint8_t (*sta_mac_list)[ETH_ALEN] = NULL;
	struct blob_attr *tb[__WIFI_GET_ASSOC_LIST_MAX];
	string_32 str_macaddr = {0};

	blobmsg_parse(wifi_get_assoc_list_policy, __WIFI_GET_ASSOC_LIST_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_ASSOC_LIST_IFNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_assoc_list(blobmsg_get_string(tb[WIFI_GET_ASSOC_LIST_IFNAME]), &sta_mac_list);
	if (ret < 0)
		return -ret;

	list_num = ret;
	if (list_num > 0 && sta_mac_list == NULL)
		return UBUS_STATUS_NO_MEMORY;

	void *ptr_sta_list = blobmsg_open_array(&buf, "assoc list");
	for (int i = 0; i < list_num; i++) {
		snprintf(str_macaddr, sizeof(str_macaddr), MACFMT, MACARG(sta_mac_list[i]));
		blobmsg_add_string(&buf, "MAC", str_macaddr);
	}
	blobmsg_close_array(&buf, ptr_sta_list);

	ret = ubus_send_reply(ctx, req, buf.head);

	if (sta_mac_list)
		free(sta_mac_list);

	return ret;
}

enum {
	WIFI_GET_IFNAMES_PHYNAME,
	WIFI_GET_IFNAMES_IFTYPE,
	__WIFI_GET_IFNAMES_MAX,
};

static const struct blobmsg_policy wifi_get_ifnames_policy[__WIFI_GET_IFNAMES_MAX] = {
	[WIFI_GET_IFNAMES_PHYNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_GET_IFNAMES_IFTYPE] = { .name = "iftype", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_wifi_get_ifnames(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_IFNAMES_MAX];
	enum clsapi_wifi_iftype iftype = CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE;
	clsapi_ifname *ifnames = NULL;
	int list_num = 0;

	blobmsg_parse(wifi_get_ifnames_policy, __WIFI_GET_IFNAMES_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_IFNAMES_PHYNAME] || !tb[WIFI_GET_IFNAMES_IFTYPE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (strcmp(blobmsg_get_string(tb[WIFI_GET_IFNAMES_IFTYPE]), "ap") == 0)
		iftype = CLSAPI_WIFI_IFTYPE_AP;
	else if (strcmp(blobmsg_get_string(tb[WIFI_GET_IFNAMES_IFTYPE]), "sta") == 0)
		iftype = CLSAPI_WIFI_IFTYPE_STA;
	else
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_ifnames(blobmsg_get_string(tb[WIFI_GET_IFNAMES_PHYNAME]), iftype, &ifnames);
	if (ret < 0)
		return -ret;

	list_num = ret;
	if (list_num > 0 && ifnames == NULL)
		return UBUS_STATUS_NO_MEMORY;

	blob_buf_init(&buf, 0);
	void *ptr_ifname_array = blobmsg_open_array(&buf, "ifname array");
	for (int i = 0; i < list_num; i++)
		blobmsg_add_string(&buf, "ifname", ifnames[i]);

	blobmsg_close_array(&buf, ptr_ifname_array);

	if (ifnames)
		free(ifnames);

	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	WIFI_START_SCAN_PHYNAME,
	WIFI_START_SCAN_FLAGS,
	WIFI_START_SCAN_BAND,
	WIFI_START_SCAN_CHANNELS,
	WIFI_START_SCAN_CHANNELS_LEN,
	__WIFI_START_SCAN_MAX,
};

static const struct blobmsg_policy wifi_start_scan_policy[__WIFI_START_SCAN_MAX] = {
	[WIFI_START_SCAN_PHYNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
	[WIFI_START_SCAN_FLAGS] = { .name = "flags", .type = BLOBMSG_TYPE_INT32 },
	[WIFI_START_SCAN_BAND] = { .name = "band", .type = BLOBMSG_TYPE_INT32 },
	[WIFI_START_SCAN_CHANNELS] = { .name = "channels", .type = BLOBMSG_TYPE_ARRAY },
	[WIFI_START_SCAN_CHANNELS_LEN] = { .name = "channels_len", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_wifi_start_scan(struct ubus_context *ctx, struct ubus_object *obj,
                    struct ubus_request_data *req, const char *method,
                    struct blob_attr *msg)
{
    int ret = CLSAPI_OK;
    struct blob_attr *tb[__WIFI_START_SCAN_MAX];

    blobmsg_parse(wifi_start_scan_policy, __WIFI_START_SCAN_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_START_SCAN_PHYNAME] || !tb[WIFI_START_SCAN_BAND])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_start_scan(blobmsg_get_string(tb[WIFI_START_SCAN_PHYNAME]),
			blobmsg_get_u32(tb[WIFI_START_SCAN_FLAGS]), blobmsg_get_u32(tb[WIFI_START_SCAN_BAND]),
			blobmsg_get_integer_array(tb[WIFI_START_SCAN_CHANNELS]),
			blobmsg_get_u32(tb[WIFI_START_SCAN_CHANNELS_LEN]));
    return -ret;
}

enum {
	WIFI_GET_TXPOWER_RATIO_PHYNAME,
	__WIFI_GET_TXPOWER_RATIO_MAX,
};

static const struct blobmsg_policy wifi_get_txpower_ratio_policy[__WIFI_GET_TXPOWER_RATIO_MAX] = {
	[WIFI_GET_TXPOWER_RATIO_PHYNAME] = { .name = "phyname", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_wifi_get_txpower_ratio(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__WIFI_GET_TXPOWER_RATIO_MAX];
	enum CLSAPI_WIFI_TXPWR_RATIO txpower_ratio = -127;

	blobmsg_parse(wifi_get_txpower_ratio_policy, __WIFI_GET_TXPOWER_RATIO_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WIFI_GET_TXPOWER_RATIO_PHYNAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_wifi_get_txpower_ratio(blobmsg_get_string(tb[WIFI_GET_TXPOWER_RATIO_PHYNAME]), &txpower_ratio);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		blobmsg_add_u32(&buf, "txpower_ratio", txpower_ratio);
		ret = ubus_send_reply(ctx, req, buf.head);
	}
	return -ret;
}

/****************	Generate automatically, DO NOT CHANGE	***************/
struct ubus_method clsapi_wifi_methods[] = {
	UBUS_METHOD("get_radio_enabled",	ubus_wifi_get_radio_enabled,	wifi_get_radio_enabled_policy),
	UBUS_METHOD("enable_radio",	ubus_wifi_enable_radio,	wifi_enable_radio_policy),
	UBUS_METHOD("get_supported_hwmodes",	ubus_wifi_get_supported_hwmodes,
			wifi_get_supported_hwmodes_policy),
	UBUS_METHOD("get_hwmode",	ubus_wifi_get_hwmode,	wifi_get_hwmode_policy),
	UBUS_METHOD("set_hwmode",	ubus_wifi_set_hwmode,	wifi_set_hwmode_policy),
	UBUS_METHOD("get_supported_bands",	ubus_wifi_get_supported_bands,	wifi_get_supported_bands_policy),
	UBUS_METHOD("get_band",	ubus_wifi_get_band,	wifi_get_band_policy),
	UBUS_METHOD("get_supported_bws",	ubus_wifi_get_supported_bws,	wifi_get_supported_bws_policy),
	UBUS_METHOD("get_bw",	ubus_wifi_get_bw,	wifi_get_bw_policy),
	UBUS_METHOD("set_bw",	ubus_wifi_set_bw,	wifi_set_bw_policy),
	UBUS_METHOD("get_supported_channels",	ubus_wifi_get_supported_channels,
			wifi_get_supported_channels_policy),
	UBUS_METHOD("get_channel",	ubus_wifi_get_channel,	wifi_get_channel_policy),
	UBUS_METHOD("set_channel",	ubus_wifi_set_channel,	wifi_set_channel_policy),
	UBUS_METHOD("get_beacon_intl",	ubus_wifi_get_beacon_intl,	wifi_get_beacon_intl_policy),
	UBUS_METHOD("set_beacon_intl",	ubus_wifi_set_beacon_intl,	wifi_set_beacon_intl_policy),
	UBUS_METHOD("get_rts",	ubus_wifi_get_rts,	wifi_get_rts_policy),
	UBUS_METHOD("set_rts",	ubus_wifi_set_rts,	wifi_set_rts_policy),
	UBUS_METHOD("get_txpower",	ubus_wifi_get_txpower,	wifi_get_txpower_policy),
	UBUS_METHOD("set_txpower",	ubus_wifi_set_txpower,	wifi_set_txpower_policy),
	UBUS_METHOD("get_short_gi",	ubus_wifi_get_short_gi,	wifi_get_short_gi_policy),
	UBUS_METHOD("set_short_gi",	ubus_wifi_set_short_gi,	wifi_set_short_gi_policy),
	UBUS_METHOD("get_country_code",	ubus_wifi_get_country_code,	wifi_get_country_code_policy),
	UBUS_METHOD("set_country_code",	ubus_wifi_set_country_code,	wifi_set_country_code_policy),
	UBUS_METHOD("get_survey",	ubus_wifi_get_survey,	wifi_get_survey_policy),
	UBUS_METHOD("get_noise",	ubus_wifi_get_noise,	wifi_get_noise_policy),
	UBUS_METHOD("get_chan_score",	ubus_wifi_get_chan_score,	wifi_get_chan_score_policy),
	UBUS_METHOD("get_supported_max_vap",	ubus_wifi_get_supported_max_vap,
			wifi_get_supported_max_vap_policy),
	UBUS_METHOD("get_supported_max_sta",	ubus_wifi_get_supported_max_sta,
			wifi_get_supported_max_sta_policy),
	UBUS_METHOD("add_bss",	ubus_wifi_add_bss,	wifi_add_bss_policy),
	UBUS_METHOD("del_bss",	ubus_wifi_del_bss,	wifi_del_bss_policy),
	UBUS_METHOD("get_bss_enabled",	ubus_wifi_get_bss_enabled,	wifi_get_bss_enabled_policy),
	UBUS_METHOD("enable_bss",	ubus_wifi_enable_bss,	wifi_enable_bss_policy),
	UBUS_METHOD("get_bssid",	ubus_wifi_get_bssid,	wifi_get_bssid_policy),
	UBUS_METHOD("get_ssid",	ubus_wifi_get_ssid,	wifi_get_ssid_policy),
	UBUS_METHOD("set_ssid",	ubus_wifi_set_ssid,	wifi_set_ssid_policy),
	UBUS_METHOD("get_encryption",	ubus_wifi_get_encryption,	wifi_get_encryption_policy),
	UBUS_METHOD("set_encryption",	ubus_wifi_set_encryption,	wifi_set_encryption_policy),
	UBUS_METHOD("get_passphrase",	ubus_wifi_get_passphrase,	wifi_get_passphrase_policy),
	UBUS_METHOD("set_passphrase",	ubus_wifi_set_passphrase,	wifi_set_passphrase_policy),
	UBUS_METHOD("get_wep_key",	ubus_wifi_get_wep_key,	wifi_get_wep_key_policy),
	UBUS_METHOD("set_wep_key",	ubus_wifi_set_wep_key,	wifi_set_wep_key_policy),
	UBUS_METHOD("get_assoc_list",	ubus_wifi_get_assoc_list,	wifi_get_assoc_list_policy),
	UBUS_METHOD("get_max_allow_sta",	ubus_wifi_get_max_allow_sta,
			wifi_get_max_allow_sta_policy),
	UBUS_METHOD("set_max_allow_sta",	ubus_wifi_set_max_allow_sta,
			wifi_set_max_allow_sta_policy),
	UBUS_METHOD("get_dtim",	ubus_wifi_get_dtim,	wifi_get_dtim_policy),
	UBUS_METHOD("set_dtim",	ubus_wifi_set_dtim,	wifi_set_dtim_policy),
	UBUS_METHOD("get_hidden_ssid",	ubus_wifi_get_hidden_ssid,	wifi_get_hidden_ssid_policy),
	UBUS_METHOD("set_hidden_ssid",	ubus_wifi_set_hidden_ssid,	wifi_set_hidden_ssid_policy),
	UBUS_METHOD("get_wmm_enabled",	ubus_wifi_get_wmm_enabled,	wifi_get_wmm_enabled_policy),
	UBUS_METHOD("enable_wmm",	ubus_wifi_enable_wmm,	wifi_enable_wmm_policy),
	UBUS_METHOD("get_pmf_enabled",	ubus_wifi_get_pmf_enabled,	wifi_get_pmf_enabled_policy),
	UBUS_METHOD("enable_pmf",	ubus_wifi_enable_pmf,	wifi_enable_pmf_policy),
	UBUS_METHOD("get_sta_max_inactivity",	ubus_wifi_get_sta_max_inactivity,
			wifi_get_sta_max_inactivity_policy),
	UBUS_METHOD("set_sta_max_inactivity",	ubus_wifi_set_sta_max_inactivity,
			wifi_set_sta_max_inactivity_policy),
	UBUS_METHOD("start_scan",	ubus_wifi_start_scan,	wifi_start_scan_policy),
	UBUS_METHOD("get_scan_count",	ubus_wifi_get_scan_count,	wifi_get_scan_count_policy),
	UBUS_METHOD("get_scan_entry",	ubus_wifi_get_scan_entry,	wifi_get_scan_entry_policy),
	UBUS_METHOD("get_macfilter_policy",	ubus_wifi_get_macfilter_policy,
			wifi_get_macfilter_policy_policy),
	UBUS_METHOD("set_macfilter_policy",	ubus_wifi_set_macfilter_policy,
			wifi_set_macfilter_policy_policy),
	UBUS_METHOD("get_macfilter_maclist",	ubus_wifi_get_macfilter_maclist,
			wifi_get_macfilter_maclist_policy),
	UBUS_METHOD("add_macfilter_mac",	ubus_wifi_add_macfilter_mac,
			wifi_add_macfilter_mac_policy),
	UBUS_METHOD("del_macfilter_mac",	ubus_wifi_del_macfilter_mac,
			wifi_del_macfilter_mac_policy),
	UBUS_METHOD("get_ifnames",	ubus_wifi_get_ifnames,	wifi_get_ifnames_policy),
	UBUS_METHOD("get_iftype",	ubus_wifi_get_iftype,	wifi_get_iftype_policy),
	UBUS_METHOD("get_sta_info",	ubus_wifi_get_sta_info,	wifi_get_sta_info_policy),
	UBUS_METHOD("get_sinr",	ubus_wifi_get_sinr,	wifi_get_sinr_policy),
	UBUS_METHOD("get_rssi",	ubus_wifi_get_rssi,	wifi_get_rssi_policy),
	UBUS_METHOD("deauth_sta",	ubus_wifi_deauth_sta,	wifi_deauth_sta_policy),
	UBUS_METHOD("disassoc_sta",	ubus_wifi_disassoc_sta,	wifi_disassoc_sta_policy),
	UBUS_METHOD("set_wps_config_method",	ubus_wifi_set_wps_config_method,
			wifi_set_wps_config_method_policy),
	UBUS_METHOD("get_wps_config_method",	ubus_wifi_get_wps_config_method,
			wifi_get_wps_config_method_policy),
	UBUS_METHOD_NOARG("get_supported_wps_config_methods",
			ubus_wifi_get_supported_wps_config_methods),
	UBUS_METHOD("get_wps_status",	ubus_wifi_get_wps_status,	wifi_get_wps_status_policy),
	UBUS_METHOD("check_wps_pin",	ubus_wifi_check_wps_pin,	wifi_check_wps_pin_policy),
	UBUS_METHOD("cancel_wps",	ubus_wifi_cancel_wps,	wifi_cancel_wps_policy),
	UBUS_METHOD("start_wps_pbc",	ubus_wifi_start_wps_pbc,	wifi_start_wps_pbc_policy),
	UBUS_METHOD_NOARG("generate_wps_pin",	ubus_wifi_generate_wps_pin),
	UBUS_METHOD("set_wps_state",	ubus_wifi_set_wps_state,	wifi_set_wps_state_policy),
	UBUS_METHOD("get_wps_state",	ubus_wifi_get_wps_state,	wifi_get_wps_state_policy),
	UBUS_METHOD_NOARG("get_mesh_role",	ubus_wifi_get_mesh_role),
	UBUS_METHOD("set_mesh_role",	ubus_wifi_set_mesh_role,	wifi_set_mesh_role_policy),
	UBUS_METHOD("enable_anti_mgmt_attack",	ubus_wifi_enable_anti_mgmt_attack,
			wifi_enable_anti_mgmt_attack_policy),
	UBUS_METHOD("set_anti_mgmt_attack_interval",	ubus_wifi_set_anti_mgmt_attack_interval,
			wifi_set_anti_mgmt_attack_interval_policy),
	UBUS_METHOD("enable_csi",	ubus_wifi_enable_csi,	wifi_enable_csi_policy),
	UBUS_METHOD("get_csi_enabled",	ubus_wifi_get_csi_enabled,	wifi_get_csi_enabled_policy),
	UBUS_METHOD("enable_csi_non_assoc_sta",	ubus_wifi_enable_csi_non_assoc_sta,
			wifi_enable_csi_non_assoc_sta_policy),
	UBUS_METHOD("get_csi_non_assoc_sta_enabled",	ubus_wifi_get_csi_non_assoc_sta_enabled,
			wifi_get_csi_non_assoc_sta_enabled_policy),
	UBUS_METHOD("enable_csi_he_smooth",	ubus_wifi_enable_csi_he_smooth,
			wifi_enable_csi_he_smooth_policy),
	UBUS_METHOD("get_csi_he_smooth_enabled",	ubus_wifi_get_csi_he_smooth_enabled,
			wifi_get_csi_he_smooth_enabled_policy),
	UBUS_METHOD("set_csi_report_period",	ubus_wifi_set_csi_report_period,
			wifi_set_csi_report_period_policy),
	UBUS_METHOD("get_csi_report_period",	ubus_wifi_get_csi_report_period,
			wifi_get_csi_report_period_policy),
	UBUS_METHOD("add_csi_sta",	ubus_wifi_add_csi_sta,	wifi_add_csi_sta_policy),
	UBUS_METHOD("del_csi_sta",	ubus_wifi_del_csi_sta,	wifi_del_csi_sta_policy),
	UBUS_METHOD("get_csi_sta_list",	ubus_wifi_get_csi_sta_list,	wifi_get_csi_sta_list_policy),
	UBUS_METHOD("get_vbss_enabled",	ubus_wifi_get_vbss_enabled,	wifi_get_vbss_enabled_policy),
	UBUS_METHOD("set_vbss_enabled",	ubus_wifi_set_vbss_enabled,	wifi_set_vbss_enabled_policy),
	UBUS_METHOD("get_vbss_vap",	ubus_wifi_get_vbss_vap,	wifi_get_vbss_vap_policy),
	UBUS_METHOD("add_vbss_vap",	ubus_wifi_add_vbss_vap,	wifi_add_vbss_vap_policy),
	UBUS_METHOD("get_vbss_ap_stats",	ubus_wifi_get_vbss_ap_stats,
			wifi_get_vbss_ap_stats_policy),
	UBUS_METHOD("stop_vbss_vap_txq",	ubus_wifi_stop_vbss_vap_txq,
			wifi_stop_vbss_vap_txq_policy),
	UBUS_METHOD("del_vbss_vap",	ubus_wifi_del_vbss_vap,	wifi_del_vbss_vap_policy),
	UBUS_METHOD("get_vbss_sta",	ubus_wifi_get_vbss_sta,	wifi_get_vbss_sta_policy),
	UBUS_METHOD("add_vbss_sta",	ubus_wifi_add_vbss_sta,	wifi_add_vbss_sta_policy),
	UBUS_METHOD("del_vbss_sta",	ubus_wifi_del_vbss_sta,	wifi_del_vbss_sta_policy),
	UBUS_METHOD("trigger_vbss_switch",	ubus_wifi_trigger_vbss_switch,
			wifi_trigger_vbss_switch_policy),
	UBUS_METHOD("get_vbss_roam_result",	ubus_wifi_get_vbss_roam_result,
			wifi_get_vbss_roam_result_policy),
	UBUS_METHOD("set_vbss_switch_done",	ubus_wifi_set_vbss_switch_done,
			wifi_set_vbss_switch_done_policy),
	UBUS_METHOD("get_vbss_nthresh",	ubus_wifi_get_vbss_nthresh,	wifi_get_vbss_nthresh_policy),
	UBUS_METHOD("set_vbss_nthresh",	ubus_wifi_set_vbss_nthresh,	wifi_set_vbss_nthresh_policy),
	UBUS_METHOD("get_vbss_mthresh",	ubus_wifi_get_vbss_mthresh,	wifi_get_vbss_mthresh_policy),
	UBUS_METHOD("set_vbss_mthresh",	ubus_wifi_set_vbss_mthresh,	wifi_set_vbss_mthresh_policy),
	UBUS_METHOD("add_vbss_monitor_sta",	ubus_wifi_add_vbss_monitor_sta,
			wifi_add_vbss_monitor_sta_policy),
	UBUS_METHOD("del_vbss_monitor_sta",	ubus_wifi_del_vbss_monitor_sta,
			wifi_del_vbss_monitor_sta_policy),
	UBUS_METHOD("set_vbss_stop_rekey",	ubus_wifi_set_vbss_stop_rekey,
			wifi_set_vbss_stop_rekey_policy),
	UBUS_METHOD("get_rssi_smoothness_factor",	ubus_wifi_get_rssi_smoothness_factor,
			wifi_get_rssi_smoothness_factor_policy),
	UBUS_METHOD("set_rssi_smoothness_factor",	ubus_wifi_set_rssi_smoothness_factor,
			wifi_set_rssi_smoothness_factor_policy),
	UBUS_METHOD("get_radio_stats",	ubus_wifi_get_radio_stats,	wifi_get_radio_stats_policy),
	UBUS_METHOD("reset_radio_stats",	ubus_wifi_reset_radio_stats,
			wifi_reset_radio_stats_policy),
	UBUS_METHOD("get_wpu_stats",	ubus_wifi_get_wpu_stats,	wifi_get_wpu_stats_policy),
	UBUS_METHOD("reset_wpu_stats",	ubus_wifi_reset_wpu_stats,	wifi_reset_wpu_stats_policy),
	UBUS_METHOD("get_vap_stats",	ubus_wifi_get_vap_stats,	wifi_get_vap_stats_policy),
	UBUS_METHOD("reset_vap_stats",	ubus_wifi_reset_vap_stats,	wifi_reset_vap_stats_policy),
	UBUS_METHOD("get_sta_stats",	ubus_wifi_get_sta_stats,	wifi_get_sta_stats_policy),
	UBUS_METHOD("reset_sta_stats",	ubus_wifi_reset_sta_stats,	wifi_reset_sta_stats_policy),
};

/****************	Wi-Fi ubus object	*****************/

static struct ubus_object_type clsapi_wifi_type =
	UBUS_OBJECT_TYPE("clsapi", clsapi_wifi_methods);

static struct ubus_object clsapi_wifi_obj = {
	.name = "clsapi.wifi",
	.type = &clsapi_wifi_type,
	.methods = clsapi_wifi_methods,
	.n_methods = ARRAY_SIZE(clsapi_wifi_methods),
};

int clsapi_wifi_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	return ubus_add_object(ctx, &clsapi_wifi_obj);
}

struct rpc_plugin rpc_plugin = {
	.init = clsapi_wifi_init
};
