/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include "clsapi_wifi.h"
#include "clsapi_base.h"
#include "clsapi_net.h"

#define MACFMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]

#define WIFI_MAX_VAP	8

#define WIFI_BAND_2GHZ_PHYNAME	"phy0"
#define WIFI_BAND_5GHZ_PHYNAME	"phy1"

#define WIFI_SECURITY_MODE_SUPPORT	"None,WEP-64,WPA-Personal,WPA2-Personal,WPA3-Personal,WPA-WPA2-Personal,WPA2-WPA3-Personal,WPA-Enterprise,WPA2-Enterprise,WPA3-Enterprise,WPA-WPA2-Enterprise,WPA2-WPA3-Enterprise,OWE,WAPI"
enum wifi_band_list {
	WIFI_BAND_2GHZ	= 0,
	WIFI_BAND_5GHZ	= 1,
	WIFI_BAND_MAX
};

struct assoc_sta_list {
	unsigned int stanum;
	unsigned char (*stalist)[ETH_ALEN];
};

struct wifi_interface_list {
	char phyname[CWMP_STR_LEN_16];
	char ifname[CWMP_IFNAME_LEN];
	char macaddr[CWMP_STR_LEN_32];
	unsigned int index;
	struct assoc_sta_list assoc_sta;
};

int macstrtohex(char *str_mac, unsigned char hex_mac[ETH_ALEN]);
void wifi_interface_list_load();
int wifi_ap_list_get_new(int *phyindex, int *intfindex, int *newindex);
int wifi_ap_list_add(int phyindex, int intfindex, int index);
int radio_index_to_phyname(int index, char *phyname);
int radio_index_to_band(int index, enum clsapi_wifi_band *band);
int ssid_index_to_ifname(int index, char *ifname, enum clsapi_wifi_iftype iftype);
int softversion_update(cwmp_t *cwmp);

