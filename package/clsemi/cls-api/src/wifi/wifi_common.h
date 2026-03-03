/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _WIFI_UTIL_H
#define _WIFI_UTIL_H

#include "clsapi_wifi.h"


#ifndef MHZ_TO_KHZ
#define MHZ_TO_KHZ(freq) ((freq) * 1000)
#endif

#ifndef KHZ_TO_MHZ
#define KHZ_TO_MHZ(freq) ((freq) / 1000)
#endif

/** Wi-Fi channel to frequency in unit of MHz.
 * Returns:
 *   = 0: Errors
 *   > 0: OK and return frequency in unit of MHz.
 */
uint32_t channel_to_freq_mhz(int chan, enum clsapi_wifi_band band);


/** Wi-Fi frequency in unit of MHz to channel.
 * Returns:
 *   = 0: Errors
 *   > 0: OK and channel number is returned
 */
uint8_t freq_mhz_to_channel(uint32_t freq);


/** Wi-Fi frequency (center freq of channel) in unit of MHz to enum clsapi_wifi_band.
 * Returns:
 *   ! CLSAPI_BAND_NOSUCH_BAND: valid band
 *   = CLSAPI_BAND_NOSUCH_BAND: Errors
 */
enum clsapi_wifi_band freq_mhz_to_band(uint32_t freq);

/* Convert Wi-Fi bandwidth in enum clsapi_wifi_bw to integer */
static inline int bw_enum2int(const enum clsapi_wifi_bw clsapi_bw)
{
	switch (clsapi_bw) {
	case CLSAPI_WIFI_BW_20_NOHT:
	case CLSAPI_WIFI_BW_20:
		return 20;

	case CLSAPI_WIFI_BW_40:
		return 40;

	case CLSAPI_WIFI_BW_80:
		return 80;

	case CLSAPI_WIFI_BW_160:
		return 160;

	default:
		return 0;
	}
}

static inline enum clsapi_wifi_bw bw_int2enum(int int_bw)
{
	switch (int_bw) {
	case 20:
		return CLSAPI_WIFI_BW_20;

	case 40:
		return CLSAPI_WIFI_BW_40;

	case 80:
		return CLSAPI_WIFI_BW_80;

	case 160:
		return CLSAPI_WIFI_BW_160;

	default:
		return CLSAPI_WIFI_BW_DEFAULT;
	}

	return CLSAPI_WIFI_BW_DEFAULT;
}

#define CLSAPI_MESH_ROLE_STR_CONTROLLER	"controller"
#define CLSAPI_MESH_ROLE_STR_AGENT		"agent"
#define CLSAPI_MESH_ROLE_STR_NONE		"none"

static inline enum clsapi_wifi_mesh_role mesh_role_str2enum(const char *role)
{
	if (strcmp(role, CLSAPI_MESH_ROLE_STR_NONE) == 0)
		return CLSAPI_WIFI_MESH_ROLE_NONE;
	else if (strcmp(role, CLSAPI_MESH_ROLE_STR_CONTROLLER) == 0)
		return CLSAPI_WIFI_MESH_ROLE_CONTROLLER;
	else if (strcmp(role, CLSAPI_MESH_ROLE_STR_AGENT) == 0)
		return CLSAPI_WIFI_MESH_ROLE_AGENT;
	else
		return CLSAPI_WIFI_MESH_ROLE_MAX;
}

static inline char * mesh_role_enum2str(const enum clsapi_wifi_mesh_role role)
{
	static string_32 str_role = {0};

	switch (role) {
	case CLSAPI_WIFI_MESH_ROLE_NONE:
		cls_strncpy(str_role, CLSAPI_MESH_ROLE_STR_NONE, sizeof(str_role));
		break;

	case CLSAPI_WIFI_MESH_ROLE_CONTROLLER:
		cls_strncpy(str_role, CLSAPI_MESH_ROLE_STR_CONTROLLER, sizeof(str_role));
		break;

	case CLSAPI_WIFI_MESH_ROLE_AGENT:
		cls_strncpy(str_role, CLSAPI_MESH_ROLE_STR_AGENT, sizeof(str_role));
		break;

	default:
		cls_strncpy(str_role, "unknown", sizeof(str_role));
		break;
	}

	return str_role;
}


#define CLSAPI_WIFI_MACFILTER_POLICY_STR_DISABLE	"disable"
#define CLSAPI_WIFI_MACFILTER_POLICY_STR_ALLOW		"allow"
#define CLSAPI_WIFI_MACFILTER_POLICY_STR_DENY		"deny"

static inline enum clsapi_wifi_macfilter_policy macfilter_policy_str2enum(const char *policy)
{
	if (strcmp(policy, CLSAPI_WIFI_MACFILTER_POLICY_STR_DISABLE) == 0)
		return CLSAPI_WIFI_MACFILTER_POLICY_DISABLED;
	else if (strcmp(policy, CLSAPI_WIFI_MACFILTER_POLICY_STR_ALLOW) == 0)
		return CLSAPI_WIFI_MACFILTER_POLICY_ALLOW;
	else if (strcmp(policy, CLSAPI_WIFI_MACFILTER_POLICY_STR_DENY) == 0)
		return CLSAPI_WIFI_MACFILTER_POLICY_DENY;
	else
		return CLSAPI_WIFI_MACFILTER_POLICY_MAX;
}

static inline char *macfilter_policy_enum2str(enum clsapi_wifi_macfilter_policy policy)
{
	static string_16 str_policy = {0};

	switch (policy) {
	case CLSAPI_WIFI_MACFILTER_POLICY_DISABLED:
		cls_strncpy(str_policy, CLSAPI_WIFI_MACFILTER_POLICY_STR_DISABLE, sizeof(str_policy));
		break;

	case CLSAPI_WIFI_MACFILTER_POLICY_ALLOW:
		cls_strncpy(str_policy, CLSAPI_WIFI_MACFILTER_POLICY_STR_ALLOW, sizeof(str_policy));
		break;

	case CLSAPI_WIFI_MACFILTER_POLICY_DENY:
		cls_strncpy(str_policy, CLSAPI_WIFI_MACFILTER_POLICY_STR_DENY, sizeof(str_policy));
		break;

	default:
		cls_strncpy(str_policy, "unknown", sizeof(str_policy));
		break;
	}

	return str_policy;
}

static const struct wifi_encryption_enum_str {
	enum clsapi_wifi_encryption enum_encryption;
	const char *str_encryption;
} clsapi_wifi_encryption_tbl[] = {
	/* Open None */
	{CLSAPI_WIFI_ENCRYPTION_OPEN_NONE,				"none"},

	/* WEP */
	{CLSAPI_WIFI_ENCRYPTION_WEP_MIXED,				"wep-mixed"},

	/* WPA Personal */
	{CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP,			"psk"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP,			"psk+aes"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP,			"psk+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP,			"psk+tkip"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP,		"psk+tkip+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP,		"psk+tkip+aes"},

	/* WPA Enterprise */
	{CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP,			"wpa"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP,			"wpa+aes"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP,			"wpa+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP,			"wpa+tkip"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP,		"wpa+tkip+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP,		"wpa+tkip+aes"},

	/* WPA2 Personal */
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP,			"psk2"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP,			"psk2+aes"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP,			"psk2+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP,			"psk2+tkip"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP,		"psk2+tkip+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP,		"psk2+tkip+aes"},

	/* WPA2 Enterprise */
	{CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP,			"wpa2"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP,			"wpa2+aes"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP,			"wpa2+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP,			"wpa2+tkip"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP,		"wpa2+tkip+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP,		"wpa2+tkip+aes"},

	/* WPA/WPA2 Personal Mixed */
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP,		"psk-mixed"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP,		"psk-mixed+aes"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP,		"psk-mixed+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP,		"psk-mixed+tkip"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP,	"psk-mixed+tkip+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP,	"psk-mixed+tkip+aes"},

	/* WPA/WPA2 Enterprise Mixed */
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP,		"wpa-mixed"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP,		"wpa-mixed+aes"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP,		"wpa-mixed+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP,		"wpa-mixed+tkip"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP,	"wpa-mixed+tkip+ccmp"},
	{CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP,	"wpa-mixed+tkip+aes"},

	/* WPA3 Personal */
	{CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP,			"sae"},

	/* WPA3 Enterprise */
	{CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP,			"wpa3"},

	/* WPA2/WPA3 Personal (PSK/SAE) Mixed */
	{CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP,	"sae-mixed"},

	/* WPA2/WPA3 Enterprise Mixed */
	{CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP,		"wpa3-mixed"},

	/* OWE */
	{CLSAPI_WIFI_ENCRYPTION_OWE,					"owe"},

	/* WAPI-PSK */
	{CLSAPI_WIFI_ENCRYPTION_WAPI_PSK,					"wapi-psk"},

	/* WAPI-CERT */
	{CLSAPI_WIFI_ENCRYPTION_WAPI_CERT,					"wapi-cert"},
};

static inline enum clsapi_wifi_encryption encryption_str2enum(const char *encryption)
{
	for (int i = 0; i < ARRAY_SIZE(clsapi_wifi_encryption_tbl); i++)
		if (strcmp(encryption, clsapi_wifi_encryption_tbl[i].str_encryption) == 0)
			return clsapi_wifi_encryption_tbl[i].enum_encryption;

	return CLSAPI_WIFI_ENCRYPTION_MAX;
}

#define STR_CLSAPI_WIFI_ENCRYPTION_UNKNOWN	"un-supported"

static inline char *encryption_enum2str(const enum clsapi_wifi_encryption encryption)
{
	static string_32 str_encryption = {0};

	for (int i = 0; i < ARRAY_SIZE(clsapi_wifi_encryption_tbl); i++)
		if (encryption == clsapi_wifi_encryption_tbl[i].enum_encryption) {
			cls_strncpy(str_encryption, clsapi_wifi_encryption_tbl[i].str_encryption, sizeof(str_encryption));
			return str_encryption;
		}

	cls_strncpy(str_encryption, STR_CLSAPI_WIFI_ENCRYPTION_UNKNOWN, sizeof(str_encryption));
	return str_encryption;
}


#endif /* _WIFI_UTIL_H */

