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

extern struct wifi_interface_list ap_list[WIFI_BAND_MAX][WIFI_MAX_VAP];

int cpe_refresh_dev_wifi_accesspoint(cwmp_t * cwmp, parameter_node_t * param_node, callback_register_func_t callback_reg)
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
            cwmp_log_info("refresh AccessPoint node, delete param %s\n", tmp_param->name);
            tmp_node = tmp_param->next_sibling;
            cwmp_model_delete_parameter(tmp_param);
            tmp_param = tmp_node;
        }
        child_param->next_sibling = NULL;
        cwmp_model_refresh_object(cwmp, param_node, 0, callback_reg);
    }
    
    return FAULT_CODE_OK;
}

int cpe_add_dev_wifi_accesspoint(cwmp_t *cwmp, parameter_node_t *param_node, int *pinstance_number, callback_register_func_t callback_reg)
{
	return cpe_add_dev_wifi_ssid(cwmp, param_node, pinstance_number, callback_reg);
}

int cpe_del_dev_wifi_accesspoint(cwmp_t *cwmp, parameter_node_t *param_node, int instance_number,  callback_register_func_t callback_reg)
{
	return cpe_del_dev_wifi_ssid(cwmp, param_node, instance_number, callback_reg);
}

/* Device.WiFi.AccessPoint.{i}.Enable */
int cpe_get_dev_wifi_accesspoint_Enable(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	return cpe_get_dev_wifi_ssid_Enable(cwmp, name, value, pool);
}
int cpe_set_dev_wifi_accesspoint_Enable(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	return cpe_set_dev_wifi_ssid_Enable(cwmp, name, value, length, callback_reg);
}

/* Device.WiFi.AccessPoint.{i}.SSIDReference */
int cpe_get_dev_wifi_accesspoint_SSIDReference(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		snprintf(buf, sizeof(buf), "Device.WiFi.SSID.%d.", index);
	} else {
		return FAULT_CODE_9003;
	}

	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.SSIDAdvertisementEnabled */
int cpe_get_dev_wifi_accesspoint_SSIDAdvertisementEnabled(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_hidden_ssid enable;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_hidden_ssid(ifname, &enable))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	if(enable == CLSAPI_WIFI_HIDDEN_SSID_DISABLE) {
		snprintf(buf, sizeof(buf), "1");
	} else {
		snprintf(buf, sizeof(buf), "0");
	}
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_SSIDAdvertisementEnabled(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	enum clsapi_wifi_hidden_ssid enable;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(atoi(value) == 1) {
			enable = CLSAPI_WIFI_HIDDEN_SSID_DISABLE;
		} else {
			enable = CLSAPI_WIFI_HIDDEN_SSID_EMPTY;
		}
		if(clsapi_wifi_set_hidden_ssid(ifname, enable))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.WMMCapability */
int cpe_get_dev_wifi_accesspoint_WMMCapability(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	*value = PSTRDUP("1");
	return FAULT_CODE_OK;
}

/* Device.WiFi.AccessPoint.{i}.WMMEnable */
int cpe_get_dev_wifi_accesspoint_WMMEnable(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
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
		if(clsapi_wifi_get_wmm_enabled(ifname, &enable))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	snprintf(buf, sizeof(buf), "%d", enable);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_WMMEnable(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_enable_wmm(ifname, atoi(value)))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDeviceNumberOfEntries */
int cpe_get_dev_wifi_accesspoint_AssociatedDeviceNumberOfEntries(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	unsigned char (*stalist)[ETH_ALEN] = NULL;
	int listnum = 0;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		listnum = clsapi_wifi_get_assoc_list(ifname, &stalist);
		if(listnum < 0)
			return FAULT_CODE_9002;
		if(listnum > 0 && stalist == NULL)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", listnum);
	*value = PSTRDUP(buf);
	if (stalist)
		free(stalist);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.MaxAssociatedDevices */
int cpe_get_dev_wifi_accesspoint_MaxAssociatedDevices(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	unsigned short maxsta;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_max_allow_sta(ifname, &maxsta))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	snprintf(buf, sizeof(buf), "%d", maxsta);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_MaxAssociatedDevices(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_set_max_allow_sta(ifname, atoi(value)))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.MaxAllowedAssociations */
int cpe_get_dev_wifi_accesspoint_MaxAllowedAssociations(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	return cpe_get_dev_wifi_accesspoint_MaxAssociatedDevices(cwmp, name, value, pool);
}

int cpe_set_dev_wifi_accesspoint_MaxAllowedAssociations(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	return cpe_set_dev_wifi_accesspoint_MaxAssociatedDevices(cwmp, name, value, length, callback_reg);
}

/* Device.WiFi.AccessPoint.{i}.MACAddressControlEnabled */
int cpe_get_dev_wifi_accesspoint_MACAddressControlEnabled(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_macfilter_policy policy = CLSAPI_WIFI_MACFILTER_POLICY_MAX;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_macfilter_policy(ifname, &policy))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	if(policy == CLSAPI_WIFI_MACFILTER_POLICY_DISABLED)
		snprintf(buf, sizeof(buf), "0");
	else if(policy == CLSAPI_WIFI_MACFILTER_POLICY_ALLOW
		|| policy == CLSAPI_WIFI_MACFILTER_POLICY_DENY)
		snprintf(buf, sizeof(buf), "1");
	else
		return FAULT_CODE_9002;
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_MACAddressControlEnabled(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	enum clsapi_wifi_macfilter_policy policy = CLSAPI_WIFI_MACFILTER_POLICY_MAX;
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(atoi(value) == 0)
			policy = CLSAPI_WIFI_MACFILTER_POLICY_DISABLED;
		else if(atoi(value) == 1)
			policy = CLSAPI_WIFI_MACFILTER_POLICY_ALLOW;
		else
			return FAULT_CODE_9003;
		if(clsapi_wifi_set_macfilter_policy(ifname, policy))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AllowedMACAddress */
int cpe_get_dev_wifi_accesspoint_AllowedMACAddress(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	unsigned char (*maclist)[ETH_ALEN] = NULL;
	int listnum = 0;
	char macstr[CWMP_STR_LEN_32] = {0};
	char buf[CWMP_STR_LEN_1025] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int len = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		listnum = clsapi_wifi_get_macfilter_maclist(ifname, &maclist);
		if(listnum < 0)
			return FAULT_CODE_9002;
		if(listnum > 0 && maclist == NULL)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	for(i = 0; i < listnum; i++) {
		snprintf(macstr, sizeof(macstr), MACFMT",", MACARG(maclist[i]));
		strcat(buf, macstr);
	}
	len = strlen(buf);
	buf[len-1] = '\0';
	*value = PSTRDUP(buf);
	if (maclist)
		free(maclist);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_AllowedMACAddress(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	unsigned char (*maclist)[ETH_ALEN] = NULL;
	int listnum = 0;
	char buf[CWMP_STR_LEN_1025] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	const char delim[] = ",";
	char *ptr = NULL;
	char *sub_node = NULL;
	char *saveptr = NULL;
	unsigned char macaddr[ETH_ALEN];

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		listnum = clsapi_wifi_get_macfilter_maclist(ifname, &maclist);
		if(listnum < 0)
			return FAULT_CODE_9002;
		if(listnum > 0 && maclist == NULL)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	for(i = 0; i < listnum; i++) {
		clsapi_wifi_del_macfilter_mac(ifname, maclist[i]);
	}
	if (maclist)
		free(maclist);

	strcpy(buf, value);
	ptr = buf;
	sub_node = strtok_r(ptr, delim, &saveptr);
	while (sub_node != NULL) {
		ret = macstrtohex(sub_node, macaddr);
		if(ret)
			return FAULT_CODE_9003;
		if(clsapi_wifi_add_macfilter_mac(ifname, macaddr))
			return FAULT_CODE_9002;
		sub_node = strtok_r(NULL, delim, &saveptr);
	}
	return FAULT_CODE_OK;
}

/* Device.WiFi.AccessPoint.{i}.Security.ModesSupported */
int cpe_get_dev_wifi_accesspoint_security_ModesSupported(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_256] = WIFI_SECURITY_MODE_SUPPORT;
	*value = PSTRDUP(buf);
	return FAULT_CODE_OK;
}

/* Device.WiFi.AccessPoint.{i}.Security.ModeEnabled */
int cpe_get_dev_wifi_accesspoint_security_ModeEnabled(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_encryption(ifname, &encryption))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	if(encryption == CLSAPI_WIFI_ENCRYPTION_OPEN_NONE)
		snprintf(buf, sizeof(buf), "None");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WEP_MIXED)
		snprintf(buf, sizeof(buf), "WEP-64");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP)
		snprintf(buf, sizeof(buf), "WPA-Personal");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP)
		snprintf(buf, sizeof(buf), "WPA2-Personal");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP)
		snprintf(buf, sizeof(buf), "WPA3-Personal");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP)
		snprintf(buf, sizeof(buf), "WPA-WPA2-Personal");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP)
		snprintf(buf, sizeof(buf), "WPA2-WPA3-Personal");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP)
		snprintf(buf, sizeof(buf), "WPA-Enterprise");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP)
		snprintf(buf, sizeof(buf), "WPA2-Enterprise");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP)
		snprintf(buf, sizeof(buf), "WPA3-Enterprise");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP)
		snprintf(buf, sizeof(buf), "WPA-WPA2-Enterprise");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP)
		snprintf(buf, sizeof(buf), "WPA2-WPA3-Enterprise");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_OWE)
		snprintf(buf, sizeof(buf), "OWE");
	else if(encryption == CLSAPI_WIFI_ENCRYPTION_WAPI_PSK
		|| encryption == CLSAPI_WIFI_ENCRYPTION_WAPI_CERT)
		snprintf(buf, sizeof(buf), "WAPI");
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_security_ModeEnabled(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(!strcmp(value, "None"))
			encryption = CLSAPI_WIFI_ENCRYPTION_OPEN_NONE;
		else if(!strcmp(value, "WEP-64"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WEP_MIXED;
		else if(!strcmp(value, "WPA-Personal"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP;
		else if(!strcmp(value, "WPA2-Personal"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP;
		else if(!strcmp(value, "WPA3-Personal"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP;
		else if(!strcmp(value, "WPA-WPA2-Personal"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP;
		else if(!strcmp(value, "WPA2-WPA3-Personal"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP;
		else if(!strcmp(value, "WPA-Enterprise"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP;
		else if(!strcmp(value, "WPA2-Enterprise"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP;
		else if(!strcmp(value, "WPA3-Enterprise"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP;
		else if(!strcmp(value, "WPA-WPA2-Enterprise"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP;
		else if(!strcmp(value, "WPA2-WPA3-Enterprise"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP;
		else if(!strcmp(value, "OWE"))
			encryption = CLSAPI_WIFI_ENCRYPTION_OWE;
		else if(!strcmp(value, "WAPI"))
			encryption = CLSAPI_WIFI_ENCRYPTION_WAPI_PSK;
		if(clsapi_wifi_set_encryption(ifname, encryption))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.Security.WEPKey */
int cpe_get_dev_wifi_accesspoint_security_WEPKey(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
#if 0
	char wepkey[CLSAPI_WIFI_MAX_PRESHARED_KEY][64];
	int useindex = 0;
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int i = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_wep_key(ifname, &useindex, wepkey))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	for (int i = 0; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++) {
		if(i == useindex)
			snprintf(buf, sizeof(buf), "%d", wepkey[i]);
	}
	*value = PSTRDUP(buf);
#else
	*value = PSTRDUP(buf);
#endif
	return ret;
}

int cpe_set_dev_wifi_accesspoint_security_WEPKey(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	char wepkey[CLSAPI_WIFI_MAX_PRESHARED_KEY][64];
	int useindex = 0;
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_wep_key(ifname, &useindex, wepkey))
			return FAULT_CODE_9002;
		strcpy(wepkey[useindex], value);
		if(clsapi_wifi_set_wep_key(ifname, useindex, wepkey))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.Security.KeyPassphrase */
int cpe_get_dev_wifi_accesspoint_security_KeyPassphrase(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char passphrase[CWMP_STR_LEN_64+1] = {0};
	char buf[CWMP_STR_LEN_64+1] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_passphrase(ifname, passphrase))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%s", passphrase);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_security_KeyPassphrase(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_set_passphrase(ifname, value))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.Security.RadiusServerIPAddr */
int cpe_get_dev_wifi_accesspoint_security_RadiusServerIPAddr(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	struct clsapi_radius_configure conf;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%s", conf.server_ip);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_security_RadiusServerIPAddr(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	struct clsapi_radius_configure conf;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
		strcpy(conf.server_ip, value);
		if(clsapi_wifi_set_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.Security.RadiusServerPort */
int cpe_get_dev_wifi_accesspoint_security_RadiusServerPort(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	struct clsapi_radius_configure conf;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", conf.server_port);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_security_RadiusServerPort(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	struct clsapi_radius_configure conf;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
		conf.server_port = atoi(value);
		if(clsapi_wifi_set_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.Security.RadiusSecret */
int cpe_get_dev_wifi_accesspoint_security_RadiusSecret(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	struct clsapi_radius_configure conf;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%s", conf.server_passphrase);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_accesspoint_security_RadiusSecret(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	struct clsapi_radius_configure conf;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
		strcpy(conf.server_passphrase, value);
		if(clsapi_wifi_set_radius_authentification(ifname, &conf))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.MACAddress */
int cpe_get_dev_wifi_accesspoint_assocdev_MACAddress(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						snprintf(buf, sizeof(buf), MACFMT, MACARG(ap_list[i][j].assoc_sta.stalist[dev_index-1]));
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.AuthenticationState */
int cpe_get_dev_wifi_accesspoint_assocdev_AuthenticationState(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	*value = PSTRDUP("1");
	return FAULT_CODE_OK;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.LastDataDownlinkRate */
int cpe_get_dev_wifi_accesspoint_assocdev_LastDataDownlinkRate(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.current_rx_rate);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.LastDataUplinkRate */
int cpe_get_dev_wifi_accesspoint_assocdev_LastDataUplinkRate(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.current_tx_rate);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.AssociationTime */
int get_uptime(unsigned long connsec, char *uptime)
{
	struct timeval tv;
	long online_tm;
	struct tm *tm;
	time_t rawtime;

	gettimeofday(&tv, NULL);
	online_tm = tv.tv_sec - connsec;
	rawtime = online_tm;
	tm = localtime(&rawtime);
	sprintf(uptime, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	return FAULT_CODE_OK;
}
int cpe_get_dev_wifi_accesspoint_assocdev_AssociationTime(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						get_uptime(sta.connected_sec, buf);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.SignalStrength */
int cpe_get_dev_wifi_accesspoint_assocdev_SignalStrength(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%d", sta.signal);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Retransmissions */
int cpe_get_dev_wifi_accesspoint_assocdev_Retransmissions(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	unsigned long retrans = 0;
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						retrans = sta.tx_retry_failed + sta.tx_retry_count;
						snprintf(buf, sizeof(buf), "%ld", retrans);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Active */
int cpe_get_dev_wifi_accesspoint_assocdev_Active(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	*value = PSTRDUP("1");
	return FAULT_CODE_OK;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.BytesSent */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_BytesSent(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.tx_bytes);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.BytesReceived */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_BytesReceived(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.rx_bytes);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.PacketsSent */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_PacketsSent(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.tx_packets);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.PacketsReceived */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_PacketsReceived(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.rx_packets);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.ErrorsSent */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_ErrorsSent(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct clsapi_wifi_sta_stats sta_stats = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_stats(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta_stats);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%d", sta_stats.tx_err);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.RetransCount */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_RetransCount(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	return cpe_get_dev_wifi_accesspoint_assocdev_Retransmissions(cwmp, name, value, pool);
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.FailedRetransCount */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_FailedRetransCount(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.tx_retry_failed);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats.RetryCount */
int cpe_get_dev_wifi_accesspoint_assocdev_stats_RetryCount(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char buf[CWMP_STR_LEN_64] = {0};
	int index = 0;
	int dev_index = 0;
	char ifname[CWMP_IFNAME_LEN] = {0};
	struct sta_info sta = {0};
	int ret = FAULT_CODE_OK;
	int i = 0;
	int j = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = ssid_index_to_ifname(index, ifname, CLSAPI_WIFI_IFTYPE_AP);
		if(ret != FAULT_CODE_OK)
			return ret;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if(!strcmp(ap_list[i][j].ifname, ifname)) {
					if(cwmp_model_get_index_by_level(name, &dev_index, 2) == FAULT_CODE_OK) {
						if (ap_list[i][j].assoc_sta.stalist == NULL)
							ap_list[i][j].assoc_sta.stanum = 0;
						if (ap_list[i][j].assoc_sta.stanum <= 0 || dev_index > ap_list[i][j].assoc_sta.stanum) {
							return FAULT_CODE_9003;
						}
						ret = clsapi_wifi_get_sta_info(ifname, ap_list[i][j].assoc_sta.stalist[dev_index-1], &sta);
						if(ret)
							return FAULT_CODE_9003;
						snprintf(buf, sizeof(buf), "%ld", sta.tx_retry_count);
						goto end;
					} else {
						return FAULT_CODE_9002;
					}
				}
			}
		}
	} else {
		return FAULT_CODE_9003;
	}
end:
	*value = PSTRDUP(buf);
	return ret;
}

