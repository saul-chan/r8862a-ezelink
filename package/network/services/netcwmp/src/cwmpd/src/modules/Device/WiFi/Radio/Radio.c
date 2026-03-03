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
#include "modules/data_model_util.h"

int cpe_refresh_dev_wifi_radio(cwmp_t *cwmp, parameter_node_t *param_node, callback_register_func_t callback_reg)
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
            cwmp_log_info("refresh Radio node, delete param %s\n", tmp_param->name);
            tmp_node = tmp_param->next_sibling;
            cwmp_model_delete_parameter(tmp_param);
            tmp_param = tmp_node;
        }
        child_param->next_sibling = NULL;
        cwmp_model_refresh_object(cwmp, param_node, 0, callback_reg);
    }
    
    return FAULT_CODE_OK;
}

/* Device.WiFi.Radio.{i}.Enable */
int cpe_get_dev_wifi_radio_Enable(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	bool enable;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_radio_enabled(phyname, &enable))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", enable);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_Enable(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_enable_radio(phyname, atoi(value)))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	return ret;
}

/* Device.WiFi.Radio.{i}.SupportedFrequencyBands */
int cpe_get_dev_wifi_radio_SupportedFrequencyBands(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_band *bands = NULL;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;
	int bandslen = 0;
	char buf[CWMP_STR_LEN_32] = {0};
	int len = 0;
	int i = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		bandslen = clsapi_wifi_get_supported_bands(phyname, &bands);
		if(bandslen < 0)
			return FAULT_CODE_9002;
		if(bandslen > 0 && bands == NULL)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	for (i = 0; i < bandslen; i++) {
		if (bands[i] == CLSAPI_BAND_2GHZ) {
			strcat(buf, "2.4GHz,");
		}
		if (bands[i] == CLSAPI_BAND_5GHZ) {
			strcat(buf, "5GHz,");
		}
		if (bands[i] == CLSAPI_BAND_6GHZ) {
			strcat(buf, "6GHz,");
		}
	}
	len = strlen(buf);
	buf[len-1] = '\0';
	*value = PSTRDUP(buf);
	if (bands)
		free(bands);
	return ret;
}

/* Device.WiFi.Radio.{i}.OperatingFrequencyBand */
int cpe_get_dev_wifi_radio_OperatingFrequencyBand(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_band band = CLSAPI_BAND_NOSUCH_BAND;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;
	char buf[CWMP_STR_LEN_32] = {0};

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_band(phyname, &band))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	if (band == CLSAPI_BAND_2GHZ)
		strcpy(buf, "2.4GHz");
	else if (band == CLSAPI_BAND_5GHZ)
		strcpy(buf, "5GHz");
	else if (band == CLSAPI_BAND_6GHZ)
		strcpy(buf, "6GHz");
	else
		strcpy(buf, "unknown");
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.Radio.{i}.SupportedStandards */
int cpe_get_dev_wifi_radio_SupportedStandards(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_hwmode hwmodes = 0;
	char buf[CWMP_STR_LEN_32] = {0};
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int len = 0;
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_supported_hwmodes(phyname, &hwmodes))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	if(hwmodes & CLSAPI_HWMODE_IEEE80211_A) {
		strcat(buf, "a,");
	}
	if(hwmodes & CLSAPI_HWMODE_IEEE80211_B) {
		strcat(buf, "b,");
	}
	if(hwmodes & CLSAPI_HWMODE_IEEE80211_G) {
		strcat(buf, "g,");
	}
	if(hwmodes & CLSAPI_HWMODE_IEEE80211_N) {
		strcat(buf, "n,");
	}
	if(hwmodes & CLSAPI_HWMODE_IEEE80211_AC) {
		strcat(buf, "ac,");
	}
	if(hwmodes & CLSAPI_HWMODE_IEEE80211_AX) {
		strcat(buf, "ax,");
	}
	len = strlen(buf);
	buf[len-1] = '\0';
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.Radio.{i}.OperatingStandards */
int cpe_get_dev_wifi_radio_OperatingStandards(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_hwmode hwmode = CLSAPI_BAND_NOSUCH_BAND;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;
	char buf[CWMP_STR_LEN_32] = {0};

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_hwmode(phyname, &hwmode))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	if (hwmode == CLSAPI_HWMODE_IEEE80211_A)
		strcpy(buf, "a");
	else if (hwmode == CLSAPI_HWMODE_IEEE80211_B)
		strcpy(buf, "b");
	else if (hwmode == CLSAPI_HWMODE_IEEE80211_G)
		strcpy(buf, "g");
	else if (hwmode == CLSAPI_HWMODE_IEEE80211_N)
		strcpy(buf, "n");
	else if (hwmode == CLSAPI_HWMODE_IEEE80211_AC)
		strcpy(buf, "ac");
	else if (hwmode == CLSAPI_HWMODE_IEEE80211_AX)
		strcpy(buf, "ax");
	else
		strcpy(buf, "unknown");
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_OperatingStandards(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	enum clsapi_wifi_hwmode hwmode = CLSAPI_BAND_NOSUCH_BAND;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}
	if (strcmp(value, "a") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_A;
	else if (strcmp(value, "b") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_B;
	else if (strcmp(value, "g") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_G;
	else if (strcmp(value, "n") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_N;
	else if (strcmp(value, "ac") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_AC;
	else if (strcmp(value, "ax") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_AX;
	else
		return FAULT_CODE_9003;
	if (clsapi_wifi_set_hwmode(phyname, hwmode))
		ret = FAULT_CODE_9002;
	return ret;
}

/* Device.WiFi.Radio.{i}.PossibleChannels */
int cpe_get_dev_wifi_radio_PossibleChannels(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;
	char buf[CWMP_STR_LEN_128] = {0};
	unsigned char *chan_list = NULL;
	char chan[CWMP_STR_LEN_16] = {0};
	int list_len = 0;
	int len = 0;
	int i = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		ret = radio_index_to_band(index, &band);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}

	list_len = clsapi_wifi_get_supported_channels(phyname, band, CLSAPI_WIFI_BW_20, &chan_list);
	if(list_len < 0)
		return FAULT_CODE_9002;
	if(list_len > 0 && chan_list == NULL)
		return FAULT_CODE_9002;
	for(i = 0; i < list_len; i++) {
		snprintf(chan, sizeof(chan), "%d,", chan_list[i]);
		strcat(buf, chan);
	}
	len = strlen(buf);
	buf[len-1] = '\0';
	*value = PSTRDUP(buf);
	if (chan_list)
		free(chan_list);
	return ret;
}

/* Device.WiFi.Radio.{i}.Channel */
int cpe_get_dev_wifi_radio_Channel(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;
	char buf[CWMP_STR_LEN_32] = {0};
	unsigned char channel = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_channel(phyname, &channel))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	snprintf(buf, sizeof(buf), "%d", channel);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_Channel(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		ret = radio_index_to_band(index, &band);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}
	if(clsapi_wifi_set_channel(phyname, atoi(value), band, CLSAPI_WIFI_BW_DEFAULT))
		ret = FAULT_CODE_9002;

	return ret;
}

/* Device.WiFi.Radio.{i}.AutoChannelSupported */
int cpe_get_dev_wifi_radio_AutoChannelSupported(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	*value = PSTRDUP("1");
	return FAULT_CODE_OK;
}

/* Device.WiFi.Radio.{i}.AutoChannelEnable */
int cpe_get_dev_wifi_radio_AutoChannelEnable(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	char channel[CWMP_STR_LEN_1025] = {0};
	char section[CWMP_STR_LEN_16] = {0};
	int index = 0;
	int radio_index = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		radio_index = index - 1;
		snprintf(section, sizeof(section), "%s%d", UCI_SECTION_RADIO, radio_index);
		if(clsapi_base_get_conf_param(UCI_CFG_WIRELESS, section, UCI_PARA_CHANNEL, channel))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	if(strcmp(channel, "auto") == 0)
		*value = PSTRDUP("1");
	else
		*value = PSTRDUP("0");
	return FAULT_CODE_OK;
}

int cpe_set_dev_wifi_radio_AutoChannelEnable(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	char channel[CWMP_STR_LEN_16] = {0};
	char section[CWMP_STR_LEN_16] = {0};
	int index = 0;
	int radio_index = 0;
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		radio_index = index - 1;
		snprintf(section, sizeof(section), "%s%d", UCI_SECTION_RADIO, radio_index);
		if(strcmp(value, "1") == 0) {
			snprintf(channel, sizeof(channel), "auto");
		} else if(strcmp(value, "0") == 0) {
			if(radio_index == 0)
				snprintf(channel, sizeof(channel), TR181_VALUE_WIFI_2G_CHANNEL);
			else
				snprintf(channel, sizeof(channel), TR181_VALUE_WIFI_5G_CHANNEL);
		} else {
			return FAULT_CODE_9003;
		}
		ret = clsapi_base_set_apply_conf_param(UCI_CFG_WIRELESS, section, UCI_PARA_CHANNEL, channel);
		if(ret)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	return FAULT_CODE_OK;
}

/* Device.WiFi.Radio.{i}.MaxSupportedSSIDs */
int cpe_get_dev_wifi_radio_MaxSupportedSSIDs(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
	unsigned short maxvap = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_supported_max_vap(phyname, &maxvap))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", maxvap);
	*value = PSTRDUP(buf);
	return ret;
}

/* Device.WiFi.Radio.{i}.MaxSupportedAssociations */
int cpe_get_dev_wifi_radio_MaxSupportedAssociations(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
	unsigned short maxsta = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_supported_max_sta(phyname, &maxsta))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", maxsta);
	*value = PSTRDUP(buf);
	return ret;
}


/* Device.WiFi.Radio.{i}.SupportedOperatingChannelBandwidths */
int cpe_get_dev_wifi_radio_SupportedOperatingChannelBandwidths(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_bw *bw_array = NULL;
	int bws_len = 0;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int len = 0;
	int ret = FAULT_CODE_OK;
	int i = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		bws_len = clsapi_wifi_get_supported_bws(phyname, &bw_array);
		if(bws_len < 0)
			return FAULT_CODE_9002;
		if(bws_len > 0 && bw_array == NULL)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	for(i = 0; i < bws_len; i++) {
		if(bw_array[i] == CLSAPI_WIFI_BW_20_NOHT || (bw_array[i] == CLSAPI_WIFI_BW_20))
			strcat(buf, "20MHz,");
		if(bw_array[i] == CLSAPI_WIFI_BW_40)
			strcat(buf, "40MHz,");
		if(bw_array[i] == CLSAPI_WIFI_BW_80)
			strcat(buf, "80MHz,");
		if(bw_array[i] == CLSAPI_WIFI_BW_160)
			strcat(buf, "160MHz,");
	}
	len = strlen(buf);
	buf[len-1] = '\0';
	*value = PSTRDUP(buf);
	if (bw_array)
		free(bw_array);
	return FAULT_CODE_OK;
}

/* Device.WiFi.Radio.{i}.OperatingChannelBandwidth */
int cpe_get_dev_wifi_radio_OperatingChannelBandwidth(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	enum clsapi_wifi_bw bw = CLSAPI_WIFI_BW_DEFAULT;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		ret = clsapi_wifi_get_bw(phyname, &bw);
		if(ret)
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}
	if(bw == CLSAPI_WIFI_BW_20_NOHT || (bw == CLSAPI_WIFI_BW_20))
		strcat(buf, "20MHz");
	else if(bw == CLSAPI_WIFI_BW_40)
		strcat(buf, "40MHz");
	else if(bw == CLSAPI_WIFI_BW_80)
		strcat(buf, "80MHz");
	else if(bw == CLSAPI_WIFI_BW_160)
		strcat(buf, "160MHz");
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_OperatingChannelBandwidth(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	enum clsapi_wifi_bw bw = CLSAPI_WIFI_BW_DEFAULT;
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}
	if (strcmp(value, "20MHz") == 0)
		bw = CLSAPI_WIFI_BW_20;
	else if (strcmp(value, "40MHz") == 0)
		bw = CLSAPI_WIFI_BW_40;
	else if (strcmp(value, "80MHz") == 0)
		bw = CLSAPI_WIFI_BW_80;
	else if (strcmp(value, "160MHz") == 0)
		bw = CLSAPI_WIFI_BW_160;
	else
		return FAULT_CODE_9003;
	if (clsapi_wifi_set_bw(phyname, bw))
		ret = FAULT_CODE_9002;
	return ret;
}

/* Device.WiFi.Radio.{i}.CurrentOperatingChannelBandwidth */
int cpe_get_dev_wifi_radio_CurrentOperatingChannelBandwidth(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int ret = FAULT_CODE_OK;

	ret = cpe_get_dev_wifi_radio_OperatingChannelBandwidth(cwmp, name, value, pool);
	return ret;
}

/* Device.WiFi.Radio.{i}.TransmitPower */
int cpe_get_dev_wifi_radio_TransmitPower(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
	signed char txpower = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_txpower(phyname, &txpower))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", txpower);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_TransmitPower(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}
	if(clsapi_wifi_set_txpower(phyname, atoi(value)))
		ret = FAULT_CODE_9002;

	return ret;
}

/* Device.WiFi.Radio.{i}.RTSThreshold */
int cpe_get_dev_wifi_radio_RTSThreshold(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
	int rts = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_rts(phyname, &rts))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", rts);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_RTSThreshold(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}
	if(clsapi_wifi_set_rts(phyname, atoi(value)))
		ret = FAULT_CODE_9002;

	return ret;
}

/* Device.WiFi.Radio.{i}.BeaconPeriod */
int cpe_get_dev_wifi_radio_BeaconPeriod(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
	unsigned short interval = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_beacon_intl(phyname, &interval))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", interval);
	*value = PSTRDUP(buf);
	return ret;
}

int cpe_set_dev_wifi_radio_BeaconPeriod(cwmp_t *cwmp, const char *name, const char *value, int length, callback_register_func_t callback_reg)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	int ret = FAULT_CODE_OK;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
	} else {
		return FAULT_CODE_9003;
	}
	if(clsapi_wifi_set_beacon_intl(phyname, atoi(value)))
		ret = FAULT_CODE_9002;

	return ret;
}

/* Device.WiFi.Radio.{i}.Stats.Noise */
int cpe_get_dev_wifi_radio_Stats_Noise(cwmp_t *cwmp, const char *name, char **value, pool_t *pool)
{
	int index = 0;
	char phyname[CWMP_STR_LEN_16] = {0};
	char buf[CWMP_STR_LEN_32] = {0};
	int ret = FAULT_CODE_OK;
	signed char noise = 0;

	if(cwmp_model_get_index(name, &index) == FAULT_CODE_OK) {
		ret = radio_index_to_phyname(index, phyname);
		if(ret != FAULT_CODE_OK)
			return ret;
		if(clsapi_wifi_get_noise(phyname, &noise))
			return FAULT_CODE_9002;
	} else {
		return FAULT_CODE_9003;
	}

	snprintf(buf, sizeof(buf), "%d", noise);
	*value = PSTRDUP(buf);
	return ret;
}

