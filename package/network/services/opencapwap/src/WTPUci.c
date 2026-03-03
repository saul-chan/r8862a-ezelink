/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */
#include "clsapi_wifi.h"
#include "clsapi_base.h"
#include "WTPUci.h"

CWBool WTPUciAddAPInterface(int radioIndex, int wlanIndex, WTPInterfaceInfo *interfaceInfo) {
	char phy_name[8] = {0};
	char created_ifname[CW_NAME_LEN] = {0};
	enum clsapi_wifi_encryption encryption;

	clsapi_base_set_defer_mode(1);
	sprintf(phy_name, "phy%d", radioIndex);

	clsapi_wifi_add_bss(phy_name, interfaceInfo->BSSID, created_ifname);
	CWDebugLog("created wifi ifname: %s", created_ifname);
	if (interfaceInfo->ifName) {
		CW_FREE_OBJECT(interfaceInfo->ifName);
	}
	CW_CREATE_STRING_FROM_STRING_ERR(interfaceInfo->ifName, created_ifname, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	clsapi_wifi_set_ssid(created_ifname, interfaceInfo->SSID);
	if (interfaceInfo->authType == NL80211_AUTHTYPE_OPEN_SYSTEM) {
		encryption = CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP;
	} else if (interfaceInfo->authType == NL80211_AUTHTYPE_SHARED_KEY) {
		encryption = CLSAPI_WIFI_ENCRYPTION_WEP_MIXED;
	} else if (interfaceInfo->authType == NL80211_AUTHTYPE_SAE) {
		encryption = CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP;
	}
	clsapi_wifi_set_encryption(created_ifname, encryption);
	clsapi_wifi_enable_bss(created_ifname, CW_ENABLE);
	clsapi_base_set_defer_mode(0);
	clsapi_base_apply_conf_cfg(CLSCONF_CFG_WIRELESS);

	return CW_TRUE;
}

CWBool WTPUciDelAPInterface(char *ifname) {
	if (!ifname)
		return CW_FALSE;

	clsapi_wifi_del_bss(ifname);
	return CW_TRUE;
}

