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
#include <cwmp/cwmp.h>
#include <cwmp/log.h>
#include "data_model_util.h"

struct wifi_interface_list ap_list[WIFI_BAND_MAX][WIFI_MAX_VAP];
struct wifi_interface_list sta_list[WIFI_BAND_MAX][WIFI_MAX_VAP];
int ap_list_update = 0;

/*
 * convert MAC address in string format "11:22:33:dd:ee:ff" to 6 byte hex.
 */
int macstrtohex(char *strmac, unsigned char hexmac[ETH_ALEN])
{
#define STR_MAC_FORMAT	"aa:bb:cc:dd:ee:ff"
	int ret;
	char mac[CWMP_STR_LEN_32] = {0};

	if (strlen(strmac) != sizeof(STR_MAC_FORMAT) -1)
		return -1;

	strcpy(mac, strmac);
	ret = sscanf(mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hexmac[0], &hexmac[1], &hexmac[2],
		&hexmac[3], &hexmac[4], &hexmac[5]);
	if (ret == ETH_ALEN)
		return 0;
	else
		return -1;
}

void wifi_interface_list_init()
{
	int i = 0;
	int j = 0;

	for(i = 0; i < WIFI_BAND_MAX; i++) {
		memset(&ap_list[i], 0, WIFI_MAX_VAP*sizeof(struct wifi_interface_list));
		memset(&sta_list[i], 0, WIFI_MAX_VAP*sizeof(struct wifi_interface_list));
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			memset(ap_list[i][j].phyname, 0, CWMP_STR_LEN_16);
			memset(ap_list[i][j].ifname, 0, CWMP_IFNAME_LEN);
			memset(ap_list[i][j].macaddr, 0, CWMP_STR_LEN_32);
			ap_list[i][j].assoc_sta.stalist = NULL;
			ap_list[i][j].assoc_sta.stanum = 0;
			memset(sta_list[i][j].phyname, 0, CWMP_STR_LEN_16);
			memset(sta_list[i][j].ifname, 0, CWMP_IFNAME_LEN);
			memset(sta_list[i][j].macaddr, 0, CWMP_STR_LEN_32);
		}
	}
	return;
}

void wifi_interface_list_print()
{
	int i = 0;
	int j = 0;

	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(ap_list[i][j].index != 0)
				cwmp_log_debug("ap_list[%d][%d]:phyname=%s, ifname=%s, index=%d, stanum=%d\n", i, j, ap_list[i][j].phyname,
					ap_list[i][j].ifname, ap_list[i][j].index, ap_list[i][j].assoc_sta.stanum);
		}
	}
	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(sta_list[i][j].index != 0)
				cwmp_log_debug("sta_list[%d][%d]:phyname=%s, ifname=%s, index=%d\n", i, j, sta_list[i][j].phyname, sta_list[i][j].ifname, sta_list[i][j].index);
		}
	}
	return;
}

void wifi_interface_list_load()
{
	char phyname[CWMP_STR_LEN_16] = {0};
	char ifname[CWMP_IFNAME_LEN] = {0};
	enum clsapi_wifi_iftype wifi_iftype;
	unsigned char addr[ETH_ALEN];
	int i = 0;
	int j = 0;
	int apindex = 0;
	int staindex = 0;

	wifi_interface_list_init();
	for(i = 0; i < WIFI_BAND_MAX; i++) {
		if(i == WIFI_BAND_2GHZ)
			strcpy(phyname, WIFI_BAND_2GHZ_PHYNAME);
		else if(i == WIFI_BAND_5GHZ)
			strcpy(phyname, WIFI_BAND_5GHZ_PHYNAME);
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(j == 0)
				snprintf(ifname, sizeof(ifname), "wlan%d", i);
			else
				snprintf(ifname, sizeof(ifname), "wlan%d-%d", i, j);
			if(clsapi_net_get_macaddr(ifname, addr) != CLSAPI_OK) {
				continue;
			}

			if(clsapi_wifi_get_iftype(ifname, &wifi_iftype) != CLSAPI_OK) {
				continue;
			}
			if(wifi_iftype == CLSAPI_WIFI_IFTYPE_AP) {
				strcpy(ap_list[i][apindex].phyname, phyname);
				strcpy(ap_list[i][apindex].ifname, ifname);
				ap_list[i][apindex].index = apindex + 1;
				snprintf(ap_list[i][apindex].macaddr, sizeof(ap_list[i][apindex].macaddr), MACFMT, MACARG(addr));
				if(ap_list[i][apindex].assoc_sta.stalist) {
					free(ap_list[i][apindex].assoc_sta.stalist);
					ap_list[i][apindex].assoc_sta.stalist = NULL;
				}
				ap_list[i][apindex].assoc_sta.stanum = clsapi_wifi_get_assoc_list(ifname, &ap_list[i][apindex].assoc_sta.stalist);
				if(ap_list[i][apindex].assoc_sta.stalist == NULL)
					ap_list[i][apindex].assoc_sta.stanum = 0;
				apindex++;
			} else if(wifi_iftype == CLSAPI_WIFI_IFTYPE_STA) {
				strcpy(sta_list[i][staindex].phyname, phyname);
				strcpy(sta_list[i][staindex].ifname, ifname);
				sta_list[i][staindex].index = staindex + 1;
				snprintf(sta_list[i][staindex].macaddr, sizeof(sta_list[i][staindex].macaddr), MACFMT, MACARG(addr));
				staindex++;
			} else {
				cwmp_log_error("unsupported wifi type:%d\n", wifi_iftype);
				continue;
			}
		}
	}
	wifi_interface_list_print();
	return;
}

int wifi_ap_list_get_new(int *phyindex, int *intfindex, int *newindex)
{
	int i = 0;
	int j = 0;
	int index = 1;

	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(ap_list[i][j].index > index) {
				index = ap_list[i][j].index;
			}
		}
	}
	*newindex = index+1;
	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(ap_list[i][j].index == 0) {
				*phyindex = i;
				*intfindex = j;
				return FAULT_CODE_OK;
			}
		}
	}
	return FAULT_CODE_9003;
}

int wifi_ap_list_add(int phyindex, int intfindex, int index)
{
	char phyname[CWMP_STR_LEN_16] = {0};
	char ifname[CWMP_IFNAME_LEN] = {0};
	unsigned char addr[ETH_ALEN];

	if(phyindex == WIFI_BAND_2GHZ)
		strcpy(phyname, WIFI_BAND_2GHZ_PHYNAME);
	else if(phyindex == WIFI_BAND_5GHZ)
		strcpy(phyname, WIFI_BAND_5GHZ_PHYNAME);
	if(intfindex == 0)
		snprintf(ifname, sizeof(ifname), "wlan%d", phyindex);
	else
		snprintf(ifname, sizeof(ifname), "wlan%d-%d", phyindex, intfindex);
	strcpy(ap_list[phyindex][intfindex].phyname, phyname);
	strcpy(ap_list[phyindex][intfindex].ifname, ifname);
	memset(ap_list[phyindex][intfindex].macaddr, 0, CWMP_STR_LEN_32);
	if(clsapi_net_get_macaddr(ifname, addr) == CLSAPI_OK) {
		snprintf(ap_list[phyindex][intfindex].macaddr, sizeof(ap_list[phyindex][intfindex].macaddr), MACFMT, MACARG(addr));
	}
	ap_list[phyindex][intfindex].index = index;
	ap_list[phyindex][intfindex].assoc_sta.stalist = NULL;
	ap_list[phyindex][intfindex].assoc_sta.stanum = 0;
	wifi_interface_list_print();
	return FAULT_CODE_OK;
}

int wifi_ap_list_del(int index)
{
	int i = 0;
	int j = 0;

	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(ap_list[i][j].index == index) {
				memset(ap_list[i][j].ifname, 0, CWMP_IFNAME_LEN);
				memset(ap_list[i][j].macaddr, 0, CWMP_STR_LEN_32);
				ap_list[i][j].index = 0;
				if(ap_list[i][j].assoc_sta.stalist) {
					free(ap_list[i][j].assoc_sta.stalist);
					ap_list[i][j].assoc_sta.stalist = NULL;
				}
				ap_list[i][j].assoc_sta.stanum = 0;
				return FAULT_CODE_OK;
			}
		}
	}

	return FAULT_CODE_9003;
}

int wifi_ap_list_reload()
{
	char phyname[CWMP_STR_LEN_16] = {0};
	char ifname[CWMP_IFNAME_LEN] = {0};
	enum clsapi_wifi_iftype wifi_iftype;
	unsigned char addr[ETH_ALEN];
	int i = 0;
	int j = 0;
	int index = 0;
	int ret = FAULT_CODE_9003;

	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			memset(ap_list[i][j].ifname, 0, CWMP_IFNAME_LEN);
			memset(ap_list[i][j].macaddr, 0, CWMP_STR_LEN_32);
			ap_list[i][j].index = 0;
			if(ap_list[i][j].assoc_sta.stalist) {
				free(ap_list[i][j].assoc_sta.stalist);
				ap_list[i][j].assoc_sta.stalist = NULL;
			}
			ap_list[i][j].assoc_sta.stanum = 0;
		}
	}

	for(i = 0; i < WIFI_BAND_MAX; i++) {
		if(i == WIFI_BAND_2GHZ)
			strcpy(phyname, WIFI_BAND_2GHZ_PHYNAME);
		else if(i == WIFI_BAND_5GHZ)
			strcpy(phyname, WIFI_BAND_5GHZ_PHYNAME);
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(j == 0)
				snprintf(ifname, sizeof(ifname), "wlan%d", i);
			else
				snprintf(ifname, sizeof(ifname), "wlan%d-%d", i, j);
			if(clsapi_net_get_macaddr(ifname, addr) != CLSAPI_OK) {
				continue;
			}
			if(clsapi_wifi_get_iftype(ifname, &wifi_iftype) != CLSAPI_OK) {
				continue;
			}
			if(wifi_iftype == CLSAPI_WIFI_IFTYPE_AP) {
				strcpy(ap_list[i][index].phyname, phyname);
				strcpy(ap_list[i][index].ifname, ifname);
				ap_list[i][index].index = index + 1;
				snprintf(ap_list[i][index].macaddr, sizeof(ap_list[i][index].macaddr), MACFMT, MACARG(addr));
				ap_list[i][index].assoc_sta.stanum = clsapi_wifi_get_assoc_list(ifname, &ap_list[i][index].assoc_sta.stalist);
				if(ap_list[i][index].assoc_sta.stalist == NULL)
					ap_list[i][index].assoc_sta.stanum = 0;
				index++;
			}
		}
	}
	wifi_interface_list_print();
	return ret;
}

int radio_index_to_phyname(int index, char *phyname)
{
	int ret = FAULT_CODE_OK;

	if(index == 1) {
		strcpy(phyname, WIFI_BAND_2GHZ_PHYNAME);
	} else if(index == 2) {
		strcpy(phyname, WIFI_BAND_5GHZ_PHYNAME);
	} else {
		ret = FAULT_CODE_9003;
	}
	return ret;
}

int radio_index_to_band(int index, enum clsapi_wifi_band *band)
{
	int ret = FAULT_CODE_OK;

	if(index == 1) {
		*band = CLSAPI_BAND_2GHZ;
	} else if(index == 2) {
		*band = CLSAPI_BAND_5GHZ;
	} else {
		ret = FAULT_CODE_9003;
	}
	return ret;
}

int ssid_index_to_ifname(int index, char *ifname, enum clsapi_wifi_iftype iftype)
{
	int i = 0;
	int j = 0;
	int ret = FAULT_CODE_9003;

	if(ap_list_update == 1) {
		wifi_ap_list_reload();
		ap_list_update = 0;
	}
	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(iftype == CLSAPI_WIFI_IFTYPE_AP) {
				if(ap_list[i][j].index == index) {
					ret = FAULT_CODE_OK;
					strcpy(ifname, ap_list[i][j].ifname);
					return ret;
				}
			} else if(iftype == CLSAPI_WIFI_IFTYPE_STA) {
				if(sta_list[i][j].index == index) {
					ret = FAULT_CODE_OK;
					strcpy(ifname, sta_list[i][j].ifname);
					return ret;
				}
			}
		}
	}

	return ret;
}

int softversion_update(cwmp_t *cwmp)
{
	char version[CWMP_STR_LEN_1025] = {0};
	int ret;

	ret = clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_VERSION, version);
	if (ret && (ret != -CLSAPI_ERR_NOT_FOUND)) {
		return FAULT_CODE_9002;
	}
	if (ret || strcmp(cwmp->cpe_version, version)) {
		if(clsapi_base_set_apply_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_VERSION, cwmp->cpe_version))
			return FAULT_CODE_9002;
		if(ret == -CLSAPI_ERR_NOT_FOUND)
			return FAULT_CODE_OK;
		cwmp_event_set_value(cwmp, INFORM_VALUECHANGE, EVENT_REF_UNTIL_REBOOTED, NULL, 0, 0, 0);
		cwmp->new_request = CWMP_YES;
	}

	return FAULT_CODE_OK;
}

