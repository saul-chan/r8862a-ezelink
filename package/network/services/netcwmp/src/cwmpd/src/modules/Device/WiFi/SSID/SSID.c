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


int  cpe_refresh_dev_wifi_ssid(cwmp_t *cwmp, parameter_node_t *param_node, callback_register_func_t callback_reg)
{
    FUNCTION_TRACE();

    if(!param_node)
    {
        return FAULT_CODE_9002;
    }
    parameter_node_t * tmp_param, *tmp_node, *child_param;
    child_param = param_node->child;
    if(child_param)
    {
        for(tmp_param=child_param->next_sibling; tmp_param; )
        {
            cwmp_log_info("refresh SSID node, delete param %s\n", tmp_param->name);
            tmp_node = tmp_param->next_sibling;
            cwmp_model_delete_parameter(tmp_param);
            tmp_param = tmp_node;
        }
        child_param->next_sibling = NULL;
        cwmp_model_refresh_object(cwmp, param_node, 0, callback_reg);
    }
    
    return FAULT_CODE_OK;
}

int cpe_add_dev_wifi_ssid(cwmp_t *cwmp, parameter_node_t *param_node, int *pinstance_number, callback_register_func_t callback_reg)
{
	char phyname[CWMP_STR_LEN_16] = {0};
	char ifname[CWMP_IFNAME_LEN] = {0};
	int i = 0;
	int j = 0;
	int index = 0;
	int phyindex = 0;
	int intfindex = 0;
	int ret = FAULT_CODE_OK;

	ret = wifi_ap_list_get_new(&i, &j, &index);
	if(ret)
		return FAULT_CODE_9003;

	if(i == WIFI_BAND_2GHZ)
		strcpy(phyname, WIFI_BAND_2GHZ_PHYNAME);
	else if(i == WIFI_BAND_5GHZ)
		strcpy(phyname, WIFI_BAND_5GHZ_PHYNAME);
	ret = clsapi_wifi_add_bss(phyname, NULL, ifname);
	if(ret)
		return FAULT_CODE_9003;
	sscanf(ifname, "wlan%d-%d", &phyindex, &intfindex);
	if(phyindex != i || intfindex != j) {
		cwmp_log_error("created ifname isn't match with ap list\n");
		return FAULT_CODE_9002;
	}
	wifi_ap_list_add(phyindex, intfindex, index);
	*pinstance_number = index;
	return FAULT_CODE_OK;
}

int cpe_del_dev_wifi_ssid(cwmp_t *cwmp, parameter_node_t *param_node, int instance_number,  callback_register_func_t callback_reg)
{
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	ret = ssid_index_to_ifname(instance_number, ifname, CLSAPI_WIFI_IFTYPE_AP);
	if(ret != FAULT_CODE_OK)
		return ret;
	if(clsapi_wifi_del_bss(ifname))
		return FAULT_CODE_9002;
	return FAULT_CODE_OK;
}

/* Device.WiFi.SSID.{i}.Enable */
int cpe_get_dev_wifi_ssid_Enable(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	bool enable;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_bss_enabled(ifname, &enable))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", enable);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_ssid_Enable(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_enable_bss(ifname, atoi(value)))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.SSID.{i}.BSSID */
int cpe_get_dev_wifi_ssid_BSSID(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	unsigned char bssid[ETH_ALEN];
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	char bssidstr[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_bssid(ifname, bssid))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	snprintf(bssidstr, sizeof(bssidstr), MACFMT, MACARG(bssid));
	snprintf(buf, sizeof(buf), "%s", bssidstr);
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.SSID.{i}.MACAddress */
int cpe_get_dev_wifi_ssid_MACAddress(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	return cpe_get_dev_wifi_ssid_BSSID(cwmp, name, value, pool);
}

/* Device.WiFi.SSID.{i}.SSID */
int cpe_get_dev_wifi_ssid_SSID(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char ssid[CWMP_STR_LEN_32+1] = {0};
	char buf[CWMP_STR_LEN_32+1] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_ssid(ifname, ssid))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%s", ssid);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_ssid_SSID(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_set_ssid(ifname, value))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}


