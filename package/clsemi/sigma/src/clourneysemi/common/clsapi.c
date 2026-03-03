/****************************************************************************
*
* Copyright (c) 2023  Clourney Semiconductor Co.,Ltd.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "common/csigma_common.h"
#include "common/csigma_log.h"
#include "common/cls_cmd_parser.h"
#include "common/cls_defconf.h"
#include "cls_dut_common.h"
#include "cls/clsapi.h"

static char gCmdStr[512];

int clsapi_cmd_exec(const char *cmd, char *result)
{
	FILE *pipe;
	char buf[128] = {0};
	int len = 0;
	int ret = -1;
	int n;

	if ((!(pipe = popen(cmd, "r"))) ||
		(feof(pipe) || (n = fread(buf, 1, sizeof(buf) - 1, pipe)) <= 0)) {
		cls_error("execute command failed: %s\n", cmd);
    } else {
		len = strlen(buf);
		strncpy(result, buf, len);
		while ((n > 0) && (result[n-1]=='\n'))
			n--;
		result[n] = '\0';

		if ((n <= 0) || (!strncmp(result,"FAIL",4)) || (!strncmp(result,"Unknown command",15))) {
			cls_error("command return failed: %s\n", cmd);
		} else {
			cls_log("command return result:%s\n", result);
			ret = 0;
		}
	}
	if (pipe)
		pclose(pipe);
	cls_log("command return result: %s\n", result);

	return ret;
}

int clsapi_wifi_set_key_passphrase(
	const char *ifname,
	const string_64 passphrase)
{
	int ret = 0;
	int len = 0;
	char passphrase_buffer[128] = "";
	char specialChar[] = "&()|\\\";'<>`";

	if (ifname ==NULL || passphrase == NULL){
		return -EINVAL;
	}

	len = strlen(passphrase);
	for (int i = 0, j = 0; i < len; i++, j++) {
		if (strchr(specialChar, passphrase[i])) {
			passphrase_buffer[j++] = '\\';
			passphrase_buffer[j] = passphrase[i];
		} else {
			passphrase_buffer[j] = passphrase[i];
		}
	}

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa_passphrase %s",
		ifname, passphrase_buffer);
	PRINT("%s:%d, %s: cls set %s wpa_passphrase %s\n",
		__FILE__, __LINE__, __func__, ifname, passphrase_buffer);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_get_chan_list(
	const int band_idx,
	struct clsapi_data_256bytes *chan_list,
	uint8_t *chan_num,
	const uint32_t flags)
{
	int ret = 0;
	char line[16];
	FILE *fp;
	int i = 0;

	if (flags == clsapi_chlist_flag_available)
		sprintf(gCmdStr,
			"iw phy%d channels | awk '/\\* [0-9]+ MHz/{if($NF != \"(disabled)\") {gsub(/\\[|\\]/, \"\", $4); print $4}}' > /tmp/channels_list.txt", band_idx);
	else if (flags == clsapi_chlist_flag_disabled)
		sprintf(gCmdStr,
			"iw phy%d channels | awk '/\\* [0-9]+ MHz/{if($NF == \"(disabled)\") {gsub(/\\[|\\]/, \"\", $4); print $4}}' > /tmp/channels_list.txt", band_idx);
	ret = system(gCmdStr);
	if (ret != 0) {
		cls_error("execute command failed: %s", gCmdStr);
		return -ENODATA;
	}
	fp = fopen("/tmp/channels_list.txt", "r+");
	if (fp == NULL) {
		cls_error("Open /tmp/channels_list.txt failed");
		return -ENODATA;
	}
	while (fgets(line, 16, fp) != NULL) {
		chan_list->data[i] = atoi(line);
		i++;
	}
	fclose(fp);
	*chan_num = i;
	return 0;
}

int clsapi_wifi_set_beacon_type(const char *ifname, const char *p_new_beacon)
{
	PRINT("entering clsapi_wifi_set_beacon_type");
	int ret = 0;
	if (ifname ==NULL || p_new_beacon == NULL){
		return -EINVAL;
	}
	if (strcmp(p_new_beacon, "Basic") == 0) {
		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa %d",
			ifname, clsapi_beacon_basic);
		PRINT("%s:%d, %s: cls set %s wpa %d (beacon type is basic)\n",
			__FILE__, __LINE__, __func__, ifname, clsapi_beacon_basic);
    } else if (strcmp(p_new_beacon, "WPA") == 0) {
        snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa %d", ifname, clsapi_beacon_wpa);
		PRINT("%s:%d, %s: cls set %s wpa %d (beacon type is wpa)\n",
			__FILE__, __LINE__, __func__, ifname, clsapi_beacon_wpa);
    } else if (strcmp(p_new_beacon, "11i") == 0) {
        snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa %d", ifname, clsapi_beacon_11i);
		PRINT("%s:%d, %s: cls set %s wpa %d (beacon type is 11i)\n",
			__FILE__, __LINE__, __func__, ifname, clsapi_beacon_11i);
	} else if (strcmp(p_new_beacon, "WPAand11i") == 0) {
		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa %d",
			ifname, clsapi_beacon_wpaand11i);
		PRINT("%s:%d, %s: cls set %s wpa %d (beacon type is wpaand11i)\n",
			__FILE__, __LINE__, __func__, ifname, clsapi_beacon_wpaand11i);
	} else {
		return -EINVAL;
	}
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_WEP_key(const char *ifname, const string_64 wepkey)
{
	int ret = 0;
	if (ifname ==NULL || wepkey == NULL){
		return -EINVAL;
	}
	snprintf(gCmdStr, sizeof(gCmdStr),
		"cls set %s auth_algs 1 && cls set %s wep_default_key 0 && cls set %s wep_key0 %s",
		ifname, ifname, ifname, wepkey);
	PRINT("%s:%d, %s: cls set %s wep_key0 %s\n",
		__FILE__, __LINE__, __func__, ifname, wepkey);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_security_ent_params(const char *ifname)
{
	int ret = 0;
	if (ifname ==NULL)
		return -EINVAL;

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s ieee8021x 1", ifname);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}
	return 0;
}

int clsapi_wifi_set_WPA_authentication_mode(const char *ifname, const string_32 authentication_mode)
{
	int ret = 0;
	if (ifname ==NULL || authentication_mode == NULL){
		return -EINVAL;
	}
	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa_key_mgmt %s",
		ifname, authentication_mode);
	PRINT("%s:%d, %s: cls set %s wpa_key_mgmt %s\n",
		__FILE__, __LINE__, __func__, ifname, authentication_mode);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}
	if (0 == strcmp(authentication_mode, "WPA-EAP"))
		clsapi_wifi_set_security_ent_params(ifname);

	return 0;
}

int clsapi_wifi_set_WPA_encryption_modes(const char *ifname, const string_32 encryption_modes)
{
	int ret = 0;
	if (ifname ==NULL || encryption_modes == NULL) {
		return -EINVAL;
	}
	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wpa_pairwise %s", ifname, encryption_modes);
	PRINT("%s:%d, %s: cls set %s wpa_pairwise %s\n",
		__FILE__, __LINE__, __func__, ifname, encryption_modes);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_hs20_params(const char *ifname,
    const string_32 hs_param,
    const string_64 value1,
    const string_64 value2,
    const string_64 value3,
    const string_64 value4,
    const string_64 value5,
    const string_64 value6)
{
	return 0;
}

int clsapi_set_params(const char *ifname,
		const clsapi_SSID SSID_substr,
		const struct clsapi_set_parameters *set_params)
{
	return 0;
}

int clsapi_wifi_set_pmf(const char *ifname, int pmf_cap)
{
	int ret = 0;
	if (ifname ==NULL || pmf_cap > 2) {
		return -EINVAL;
	}
	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s ieee80211w %d", ifname, pmf_cap);
	PRINT("%s:%d, %s: cls set %s ieee80211w %d\n",
		__FILE__, __LINE__, __func__, ifname, pmf_cap);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_radio_get_primary_interface(const clsapi_unsigned_int radio_id,
						char *ifname, size_t maxlen)
{
	int ret = 0;

	snprintf(ifname, maxlen, "wlan%d", radio_id);
	PRINT("%s:%d, %s: primary_interface is %s\n", __FILE__, __LINE__, __func__, ifname);

	return 0;
}

int clsapi_interface_get_status(const char *ifname, char *interface_status)
{
	return 0;
}

int clsapi_wifi_get_chan(const char *ifname,
			clsapi_unsigned_int *p_current_channel,
			clsapi_unsigned_int *p_current_bw,
			clsapi_unsigned_int *p_current_band)
{
	int ret = 0;
	unsigned int radio_id;
	char tmp_buff[16];
	FILE *tmpfd;
	FILE *tmpfd2;

	sprintf(gCmdStr,
		"ret=`iw dev %s info |grep channel` && echo $ret | awk '{print $2}' > /tmp/get_chan.txt",
		ifname);
	PRINT("%s:%d, %s: ret=`iw dev %s info |grep channel` && echo $ret | awk '{print $2}' > /tmp/get_chan.txt\n",
		__FILE__, __LINE__, __func__, ifname);
	ret = system(gCmdStr);
	PRINT("%s:%d, %s: cmd execution status return = %d\n",
		__FILE__, __LINE__, __func__, ret);
	if (ret != 0) {
		return -ENODATA;
	}
	tmpfd = fopen("/tmp/get_chan.txt", "r+");
	if (tmpfd == NULL) {
		cls_error("Open /tmp/get_chan.txt failed");
		PRINT("%s:%d, %s: Open /tmp/get_chan.txt failed\n", __FILE__, __LINE__, __func__);
		return -ENODATA;
	}
	fread(tmp_buff, sizeof(char), sizeof(tmp_buff), tmpfd);
	PRINT("%s:%d, %s: get_chan buff = %s\n",
		__FILE__, __LINE__, __func__, tmp_buff);
	if (strlen(tmp_buff) == 0) {
		fclose(tmpfd);
		return -ENODATA;
	}
	*p_current_channel = atoi(tmp_buff);
	if (*p_current_channel == 0) {
		fclose(tmpfd);
		return -ENODATA;
	}
	PRINT("%s:%d, %s: %s current_channel is %d\n",
		__FILE__, __LINE__, __func__, ifname, *p_current_channel);
	fclose(tmpfd);

	sprintf(gCmdStr,
		"ret=`iw dev %s info |grep channel` && echo $ret | awk '{print $6}'> /tmp/get_bw.txt",
		ifname);
	PRINT("%s:%d, %s: ret=`iw dev %s info |grep channel` && echo $ret | awk '{print $6}'> /tmp/get_bw.txt\n",
		__FILE__, __LINE__, __func__, ifname);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}
	tmpfd2 = fopen("/tmp/get_bw.txt", "r+");
	if (tmpfd2 == NULL) {
		cls_error("Open /tmp/get_bw.txt failed");
		PRINT("%s:%d, %s: Open /tmp/get_bw.txt failed\n", __FILE__, __LINE__, __func__);
		return -ENODATA;
	}
	fread(tmp_buff, sizeof(char), sizeof(tmp_buff), tmpfd2);
	if (strlen(tmp_buff) == 0) {
		fclose(tmpfd2);
		return -ENODATA;
	}
	*p_current_bw = atoi(tmp_buff);
	if (*p_current_bw == 0) {
		fclose(tmpfd2);
		return -ENODATA;
	}
	PRINT("%s:%d, %s: %s current_bw is %d\n",
		__FILE__, __LINE__, __func__, ifname, *p_current_bw);
	fclose(tmpfd2);

	ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
	if (ret < 0) {
		cls_error("can't get radio_id, ifname: %s, error %d", ifname, ret);
		PRINT("%s:%d, %s: can't get radio_id, ifname: %s, error %d\n",
			__FILE__, __LINE__, __func__, ifname, ret);
		return -ENODATA;
	}

	switch (radio_id) {
	case 0: *p_current_band = clsapi_freq_band_2pt4_ghz;
		break;
	case 1: *p_current_band = clsapi_freq_band_5_ghz;
		break;
	case 2: *p_current_band = clsapi_freq_band_6_ghz;
		break;
	}
	PRINT("%s:%d, %s: %s current_band is %d\n",
		__FILE__, __LINE__, __func__, ifname, *p_current_band);
	return 0;
}

/**
void cls_set_sigma_first_active_band_idx(enum cls_dut_band_index band_index)
{
	PRINT("entering cls_set_sigma_first_active_band_idx ==>\n");
}
**/

int clsapi_interface_get_mac_addr(const char *ifname, clsapi_mac_addr device_mac_addr)
{
	char tmp1[512];
	char *tmp2;
	char current_mac[18] = {0};
	FILE *tmpfd;
	int ret = 0;

	sprintf(gCmdStr, "ifconfig %s > /tmp/ifconfig.txt", ifname);
	//sprintf(tmp2, "ifconfig |grep ether |awk '{print $2}");
	ret = system(gCmdStr);
	if (ret != 0){
		return -ENODATA;
	}
	tmpfd = fopen("/tmp/ifconfig.txt", "r+");
	fread(tmp1, sizeof(char), sizeof(tmp1), tmpfd);
	tmp2 = strstr(tmp1, "HWaddr");
	if (!tmp2){
		return -ENODATA;
	}
	memcpy(current_mac, tmp2 + 7, 17);
	sscanf(current_mac,
		"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&device_mac_addr[0], &device_mac_addr[1],
		&device_mac_addr[2], &device_mac_addr[3],
		&device_mac_addr[4], &device_mac_addr[5]);
	fclose(tmpfd);
	return 0;
}

int clsapi_wifi_get_ieee80211r_mobility_domain(const char *ifname, string_16 p_value)
{
	return 0;
}

int clsapi_wifi_wait_scan_completes(const char *ifname, time_t timeout)
{
	return 0;
}

int clsapi_wifi_add_radius_auth_server_cfg(const char *ifname, char *radius_ip,
		char *radius_port, char *radius_password)
{
	int ret = 0;

	cls_log("Set radius interface: %s Radius IP: %s, Radius port: %s, Radius password: %s",
		ifname, radius_ip, radius_port, radius_password);
	PRINT("%s:%d, %s: Set radius interface: %s IP: %s, port: %s, password: %s\n",
		__FILE__, __LINE__, __func__, ifname, radius_ip, radius_port, radius_password);

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s auth_server_addr %s,%s,%s",
		ifname, radius_ip, radius_port, radius_password);
	PRINT("%s:%d, %s: cls set %s auth_server_addr %s,%s,%s\n",
		__FILE__, __LINE__, __func__, ifname, radius_ip, radius_port, radius_password);

	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_get_radio_from_ifname(const char *ifname, unsigned int *radio_id)
{
	char tmp_buff[128];
	FILE *tmpfd;
	int ret;
	int channel;

	if (ifname ==NULL) {
		return -EINVAL;
	}

	*radio_id = strstr(ifname, "_") == NULL ? ifname[strlen(ifname) - 1] - '0' : ifname[strlen(ifname) - 3] - '0';
	PRINT("%s:%d, %s: %d\n", __FILE__, __LINE__, __func__, *radio_id);

	if (*radio_id > 1) {
		sprintf(gCmdStr,
		"ret=`iw dev %s info |grep channel` && echo $ret | awk '{print $2}'> /tmp/get_radio.txt",
		ifname);
		PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, gCmdStr);

		ret = system(gCmdStr);
		if (ret != 0)
			return -ENODATA;

		tmpfd = fopen("/tmp/get_radio.txt", "r+");
		if (tmpfd == NULL) {
			cls_error("Open /tmp/get_radio.txt failed");
			PRINT("%s:%d, %s: Open /tmp/get_radio.txt failed\n",
				__FILE__, __LINE__, __func__);
			return -ENODATA;
		}

		fread(tmp_buff, sizeof(char), sizeof(tmp_buff), tmpfd);
		if (strlen(tmp_buff) == 0) {
			fclose(tmpfd);
			cls_error("Get freq failed!");
			PRINT("%s:%d, %s: Get freq failed\n",
				__FILE__, __LINE__, __func__);
			return -ENODATA;
		}
		channel = atoi(tmp_buff);
		PRINT("%s:%d, %s: channel is %d\n", __FILE__, __LINE__, __func__, channel);
		if (channel >= 1 && channel <= 14)
			*radio_id = 0;
		else if (channel >= 34 && channel <= 165)
			*radio_id = 1;
		else {
			fclose(tmpfd);
			return -EINVAL;
		}
		fclose(tmpfd);
		PRINT("%s:%d, %s: channel is %d, radio_id is %d\n",
				__FILE__, __LINE__, __func__, channel, *radio_id);
	}

	return 0;
}

int clsapi_wifi_create_bss(const char *ifname, unsigned int radio_id,
	const clsapi_mac_addr mac_addr)
{
	int ret = 0;
	char primary_vap[16] = {0};

	if (ifname ==NULL) {
		return -EINVAL;
	}

	if (mac_addr == NULL || strlen(mac_addr) == 0) {
		snprintf(gCmdStr, sizeof(gCmdStr), "cls create_bss %s %d", ifname, radio_id);
		PRINT("%s:%d, %s: cls create_bss %s %d\n",
			__FILE__, __LINE__, __func__, ifname, radio_id);
	} else {
		snprintf(gCmdStr, sizeof(gCmdStr),
			"cls create_bss %s %d mac %02x:%02x:%02x:%02x:%02x:%02x",
			ifname, radio_id, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3],
			mac_addr[4], mac_addr[5]);
		PRINT("%s:%d, %s: cls create_bss %s %d mac %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FILE__, __LINE__, __func__, ifname, radio_id, mac_addr[0], mac_addr[1],
			mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	}

	ret = system(gCmdStr);

	if (ret != 0)
		return -ENODATA;

	sprintf(primary_vap, "wlan%d", radio_id);
	snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli -i %s create_bss %s", primary_vap, ifname);
	ret = system(gCmdStr);
	PRINT("%s:%d, %s: hostapd_cli -i %s create_bss %s\n",
			__FILE__, __LINE__, __func__, primary_vap, ifname);

	if (ret != 0)
		return -ENODATA;

	return 0;
}

int clsapi_pm_set_mode(int mode)
{
	return 0;
}

int clsapi_wifi_set_SSID(const char *ifname, const clsapi_SSID SSID_str)
{
	int ret = 0;
	int len = 0;
	char ssid_str[128] = "";
	char specialChar[] = "&()|\\\";'<>`";

	if (ifname ==NULL || SSID_str == NULL) {
		return -EINVAL;
	}
	len = strlen(SSID_str);
	for (int i = 0, j = 0; i < len; i++, j++) {
		if (strchr(specialChar, SSID_str[i])) {
			ssid_str[j++] = '\\';
			ssid_str[j] = SSID_str[i];
		} else {
			ssid_str[j] = SSID_str[i];
		}
	}

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s ssid %s", ifname, ssid_str);
	PRINT("%s:%d, %s: cls set %s ssid %s\n", __FILE__, __LINE__, __func__, ifname, ssid_str);
	ret = system(gCmdStr);

	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_config_update_parameter(const char *ifname, const char *param_name,
						const char *param_value)
{
	return 0;
}

int clsapi_wifi_get_regulatory_region(const char *ifname, char *region_by_name)
{
	return 0;
}

int clsapi_wifi_set_bw(const char *ifname, const clsapi_unsigned_int bw, const char *phy_mode)
{
	int ret = 0;
	int vendor_bw = 0;
	if (ifname == NULL) {
		return -EINVAL;
	}
/*
	const int is_2p4_interface = cls_is_2p4_interface(ifname);

	if (is_2p4_interface) {
		if (strcasecmp(phy_mode, "11ax") == 0)
			phy_mode = "11axng";
	}
*/
	cls_log("Set wireless bandwidth interface: %s, bandwidth: %u,",
		ifname, bw);
	PRINT("%s:%d, %s: Set wireless bandwidth interface: %s, bandwidth: %u\n",
		__FILE__, __LINE__, __func__, ifname, bw);

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s bw %u", ifname, bw);
	PRINT("%s:%d, %s: cls set bw: %s\n",
		__FILE__, __LINE__, __func__, gCmdStr);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}
	switch (bw) {
		case 20:
			vendor_bw = 0;
			break;
		case 40:
			vendor_bw = 1;
			break;
		case 80:
			vendor_bw = 2;
			break;
		case 160:
			vendor_bw = 3;
			break;
		default:
	}
	snprintf(gCmdStr, sizeof(gCmdStr), "iw dev %s vendor send 0xD04433 0x02 0x0f 0x%02x", ifname, vendor_bw);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_option(const char *ifname, clsapi_option_type clsapi_option, int new_option)
{
	return 0;
}

int clsapi_wifi_qos_set_param(const char *ifname, int bss, int the_queue, int the_param,
		int acm_flag, int value)
{
	uint8_t byte0;
	uint8_t byte1;
	int ret;

	if (acm_flag) {
		snprintf(gCmdStr, sizeof(gCmdStr),
			"iw dev %s vendor send 0xD04433 0x08 0x%02X 0x%02X 0x%02X 0x%02X",
			ifname, bss, the_queue, the_param, value);
		PRINT("%s:%d, %s: acm_flag = 1, %s\n",
			__FILE__, __LINE__, __func__, gCmdStr);
		ret = system(gCmdStr);
		if (ret != 0) {
			PRINT("%s:%d, %s: acm_flag = 1, failed to set qos\n",
				__FILE__, __LINE__, __func__);
			return -ENODATA;
		}
	} else {
		byte0 = (uint8_t)value; // 低位字节
		byte1 = (uint8_t)(value >> 8); // 高位字节
		snprintf(gCmdStr, sizeof(gCmdStr),
			"iw dev %s vendor send 0xD04433 0x08 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
			ifname, bss, the_queue, the_param, byte0, byte1);
		PRINT("%s:%d, %s: acm_flag = 0, %s\n",
			__FILE__, __LINE__, __func__, gCmdStr);
		ret = system(gCmdStr);
		if (ret != 0) {
			PRINT("%s:%d, %s: acm_flag = 0, failed to set qos\n",
				__FILE__, __LINE__, __func__);
			return -ENODATA;
		}
	}
	return 0;
}

int clsapi_wifi_set_rts_threshold(const char  *ifname, clsapi_unsigned_int rts_threshold)
{
	int ret = 0;
	unsigned int radio_id;
	char tmpbuf[64];

	if (ifname == NULL)
		return -EINVAL;

	ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
	if (ret < 0) {
		cls_error("can't get radio_id, ifname: %s, error %d", ifname, ret);
		PRINT("%s:%d, %s: can't get radio_id, ifname: %s, error %d\n",
			__FILE__, __LINE__, __func__, ifname, ret);
		return -ENODATA;
	}

	snprintf(tmpbuf, sizeof(tmpbuf), "iw phy phy%u set rts %u", radio_id, rts_threshold);
	PRINT("%s:%d, %s: set rts_threshold: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
	ret = system(tmpbuf);

	if (ret != 0) {
		cls_error("set rts_threshold failed, result: %d", ret);
		PRINT("%s:%d, %s: set rts_threshold failed, result: %d\n",
			__FILE__, __LINE__, __func__, ret);
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_beacon_interval(const char *ifname, clsapi_unsigned_int new_intval)
{
	int ret = 0;
	if (ifname == NULL) {
		return -EINVAL;
	}

	cls_log("Set beacon interval interface: %s, interval: %u", ifname, new_intval);
	PRINT("%s:%d, %s: Set beacon interval interface: %s, interval: %u\n",
		__FILE__, __LINE__, __func__, ifname, new_intval);

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s beacon_int %u", ifname, new_intval);
	PRINT("%s:%d, %s: cls set %s beacon_int %u\n",
		__FILE__, __LINE__, __func__, ifname, new_intval);
	ret = system(gCmdStr);

	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_dtim(const char *ifname, const clsapi_unsigned_int new_dtim)
{
	return 0;
}

int clsapi_wifi_get_option(const char *ifname, clsapi_option_type clsapi_option,
	int *p_current_option)
{
	return 0;
}

int clsapi_wifi_set_rxba_decline(const char *ifname, clsapi_unsigned_int value)
{
	char tmpbuf[128];
	int result;

	if (ifname == NULL)
		return -EINVAL;

	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s vendor send 0xD04433 0x02 0x0a 0x%02x",
		ifname, value);
	PRINT("%s:%d, %s: set_rxba_decline: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
	result = system(tmpbuf);

	if (result != 0) {
		cls_error("Set rxba_decline failed, result: %d", result);
		PRINT("%s:%d, %s: Set ldpc failed, result: %d\n",
			__FILE__, __LINE__, __func__, result);
	}
	return result;
}

int clsapi_wifi_set_txba_disable(const char *ifname, int disable)
{
	return 0;
}

int clsapi_wifi_get_channel(const char *ifname, clsapi_unsigned_int *p_current_channel)
{
	char result[16] = {0};
	int ret = -1;

	if(!ifname || (strlen(ifname) == 0))
		snprintf(gCmdStr, sizeof(gCmdStr), "iw dev wlan1 info | grep -i channel | awk -F ' ' '{print $2}'");
	else
		snprintf(gCmdStr, sizeof(gCmdStr), "iw dev %s info | grep -i channel | awk -F ' ' '{print $2}'", ifname);
	ret = clsapi_cmd_exec(gCmdStr, result);
	if (ret || strlen(result) == 0) {
		cls_error("Get channel failed");
		return -1;
	}
	*p_current_channel = atoi(result);

	return 0;
}

int clsapi_wifi_set_sec_chan(const char *ifname, int chan, int offset)
{
	return 0;
}

int clsapi_wifi_set_ieee80211r(const char *ifname, char *value)
{
	return 0;
}

int clsapi_wifi_set_ieee80211r_ft_over_ds(const char *ifname, char *value)
{
	return 0;
}

int clsapi_wifi_set_ieee80211r_nas_id(const char *ifname, char *value)
{
	return 0;
}

int clsapi_wifi_set_11r_r1_key_holder(const char *ifname, char *value)
{
	return 0;
}

int clsapi_wifi_add_11r_neighbour(const char *ifname, char *mac, char *nas_id,
		char *key, char *r1kh_id)
{
	return 0;
}

int clsapi_wifi_set_ac_agg_hold_time(const char *ifname, uint32_t ac, uint32_t agg_hold_time)
{
	return 0;
}

int clsapi_wifi_get_phy_mode(const char *ifname, char *p_wifi_phy_mode)
{
	char tmp_buff[128];
	char tmp_buff2[128];
	FILE *tmpfd;
	int ret;

	sprintf(gCmdStr,
		"ret=`ubus call iwinfo info {\\\"device\\\":\\\"%s\\\"}` && echo $ret |awk '{print $53}'> /tmp/get_mode.txt",
		ifname);
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, gCmdStr);
	ret = system(gCmdStr);

	if (ret != 0)
		return -ENODATA;

	tmpfd = fopen("/tmp/get_mode.txt", "r+");
	if (tmpfd == NULL) {
		cls_error("Open /tmp/get_mode.txt failed");
		PRINT("%s:%d, %s: Open /tmp/get_mode.txt failed\n", __FILE__, __LINE__, __func__);
		return -ENODATA;
	}
	fread(tmp_buff, sizeof(char), sizeof(tmp_buff), tmpfd);
	if (strlen(tmp_buff) == 0) {
		fclose(tmpfd);
		return -ENODATA;
	}

	int i;
	int j;
	tmp_buff2[0] = '1';
	tmp_buff2[1] = '1';

	for (i = 0, j = 2; tmp_buff[i] != '\0'; i++) {
		if (tmp_buff[i] != ' ' && tmp_buff[i] != '\"' && tmp_buff[i] != ',' && tmp_buff[i] != '\n')
			tmp_buff2[j++] = tmp_buff[i];
	}
	tmp_buff2[j] = '\0';
	memcpy(p_wifi_phy_mode, tmp_buff2, strlen(tmp_buff2) + 1);
//	p_wifi_phy_mode = tmp_buff;
	PRINT("%s:%d, %s: %s phymode is %s\n", __FILE__, __LINE__, __func__,
		ifname, p_wifi_phy_mode);
	fclose(tmpfd);

	return 0;
}

int clsapi_wifi_get_bw(const char *ifname, clsapi_unsigned_int *p_bw)
{
	FILE *tmpfd;
	int ret;
	char tmp_buff[16];

	sprintf(gCmdStr,
		"ret=`iw dev %s info |grep channel` && echo $ret | awk '{print $6}'> /tmp/get_bw.txt",
		ifname);
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, gCmdStr);
	ret = system(gCmdStr);
	if (ret != 0)
		return -ENODATA;

	tmpfd = fopen("/tmp/get_bw.txt", "r+");
	if (tmpfd == NULL) {
		cls_error("Open /tmp/get_bw.txt failed");
		PRINT("%s:%d, %s: Open /tmp/get_bw.txt failed\n", __FILE__, __LINE__, __func__);
		return -ENODATA;
	}
	fread(tmp_buff, sizeof(char), sizeof(tmp_buff), tmpfd);
	if (strlen(tmp_buff) == 0) {
		fclose(tmpfd);
		return -ENODATA;
	}
	*p_bw = atoi(tmp_buff);
	if (*p_bw == 0) {
		fclose(tmpfd);
		return -ENODATA;
	}
	fclose(tmpfd);
	PRINT("%s:%d, %s: %s bw is %d\n", __FILE__, __LINE__, __func__,
		ifname, *p_bw);

	return 0;
}

int clsapi_wifi_set_phy_mode(const char *ifname, const char *new_phy_mode)
{
	int ret = 0;
	if (ifname == NULL || new_phy_mode == NULL) {
		return -EINVAL;
	}

	cls_log("Set phy_mode interface: %s, new_phy_mode: %s", ifname, new_phy_mode);
	PRINT("%s:%d, %s: Set phy_mode interface: %s, new_phy_mode: %s\n",
		__FILE__, __LINE__, __func__, ifname, new_phy_mode);

	snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s mode %s", ifname, new_phy_mode);
	PRINT("%s:%d, %s: cls set %s mode %s\n",
		__FILE__, __LINE__, __func__, ifname, new_phy_mode);
	ret = system(gCmdStr);
	if (ret != 0) {
		return -ENODATA;
	}

	return 0;
}

int clsapi_wifi_set_chan(const char *ifname, const clsapi_unsigned_int new_channel,
	const clsapi_unsigned_int new_bw, const clsapi_unsigned_int new_band, const char *phy_mode)
{
	int ret = 0;
	if (ifname == NULL) {
		return -EINVAL;
	}

	if (new_bw == 0) {
		cls_log("Set wifi channel interface: %s, new_channel: %u, mode: %s",
			ifname, new_channel, phy_mode);
		PRINT("%s:%d, %s: Set wifi channel interface: %s, new_channel: %u, mode: %s\n",
			__FILE__, __LINE__, __func__, ifname, new_channel, phy_mode);

		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s mode %s channel %u",
			ifname, phy_mode, new_channel);
		PRINT("%s:%d, %s: cls set %s mode %s channel %u\n",
			__FILE__, __LINE__, __func__, ifname, phy_mode, new_channel);
		ret = system(gCmdStr);
		if (ret != 0)
			return -ENODATA;

	} else {
		cls_log("Set wifi channel interface: %s, new_channel: %u, mode: %s, new_bw: %u",
			ifname, new_channel, phy_mode, new_bw);
		PRINT("%s:%d, %s: Set wifi chan interface: %s, new_channel: %u, mode: %s, new_bw: %u\n",
			__FILE__, __LINE__, __func__, ifname, new_channel, phy_mode, new_bw);

		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s mode %s channel %u width %u",
			ifname, phy_mode, new_channel, new_bw);
		PRINT("%s:%d, %s: cls set %s mode %s channel %u width %u\n",
			__FILE__, __LINE__, __func__, ifname, phy_mode, new_channel, new_bw);
		ret = system(gCmdStr);
		if (ret != 0)
			return -ENODATA;

	}

	return 0;
}

int clsapi_wifi_get_mode(const char *ifname, clsapi_wifi_mode *p_wifi_mode)
{
	return clsapi_access_point;
}

int clsapi_wifi_set_nss_cap(const char *ifname, const clsapi_mimo_type modulation,
	 const clsapi_unsigned_int nss)
{
	return 0;
}

int clsapi_wifi_set_tx_amsdu(const char *ifname, int enable)
{
	return 0;
}

int clsapi_wifi_set_mcs_rate(const char *ifname, const clsapi_mcs_rate new_mcs_rate)
{
	return 0;
}

int clsapi_wifi_set_ieee80211r_mobility_domain(const char *ifname, char *value)
{
	return 0;
}

int clsapi_wifi_get_parameter(const char *ifname, clsapi_wifi_param_type type, int *p_value)
{
	return 0;
}

int clsapi_radio_get_board_parameter(const clsapi_unsigned_int radio_id,
        clsapi_board_parameter_type board_param, string_64 p_buffer)
{
	return 0;
}

int clsapi_wifi_get_associated_device_mac_addr(const char *ifname,
                        const clsapi_unsigned_int device_index,
                        clsapi_mac_addr device_mac_addr)
{
	return 0;
}

int clsapi_wifi_get_count_associations(const char *ifname,
                    clsapi_unsigned_int *p_association_count)
{
	return 0;
}

int clsapi_wifi_get_cac_status(const char *ifname, int *cacstatus)
{
	*cacstatus = 1;
	return 0;
}

int clsapi_wifi_get_radius_auth_server_cfg(const char *ifname, string_1024 radius_auth_server_cfg)
{
	return 0;
}

int clsapi_wifi_del_radius_auth_server_cfg(const char *ifname,
	const char *radius_auth_server_ipaddr, const char *constp_radius_port)
{
	return 0;
}

int clsapi_remove_app_ie(const char *ifname, const clsapi_unsigned_int frametype,
	const clsapi_unsigned_int index)
{
	return 0;
}

int clsapi_wifi_rfstatus(clsapi_unsigned_int *rf_status)
{
	*rf_status = 0;
	return 0;
}

int clsapi_radio_rfstatus(const char *ifname, clsapi_unsigned_int *rfstatus)
{
	*rfstatus = 0;
	return 0;
}

int clsapi_wifi_set_vht(const char *ifname, const clsapi_unsigned_int the_vht)
{
	return 0;
}

int clsapi_wifi_scs_enable(const char *ifname, uint16_t enable_val)
{
	return 0;
}

int clsapi_wifi_set_channel(const char *ifname, const clsapi_unsigned_int new_channel)
{
	return 0;
}

int clsapi_wifi_set_interworking(const char *ifname, const string_32 interworking)
{
	return 0;
}

int clsapi_wifi_set_80211u_params(const char *ifname, const string_32 param,
	const string_256 value1, const string_32 valuse2)
{
	return 0;
}

int clsapi_wifi_set_hs20_status(const char *ifname, const string_32 hs20_val)
{
	return 0;
}

int clsapi_security_add_oper_friendly_name(const char *ifname, const char *lang_code,
	const char *oper_friendly_name)
{
	return 0;
}

int clsapi_security_add_roaming_consortium(const char *ifname, const char *p_value)
{
	return 0;
}

int clsapi_wifi_disable_wps(const char *ifname, int disable_wps)
{
	return 0;
}

int clsapi_wifi_remove_bss(const char *ifname)
{
	return 0;
}

int clsapi_wifi_get_rx_chains(const char *ifname, clsapi_unsigned_int *p_rx_chains)
{
	*p_rx_chains = 2;
	return 0;
}

int clsapi_regulatory_set_regulatory_region(const char *ifname, const char *region_by_name)
{
	return 0;
}

int clsapi_interface_enable(const char *ifname, const int enable_flag)
{
	return 0;
}

int clsapi_radio_verify_repeater_mode(const clsapi_unsigned_int radio_id)
{
	return 0;
}

int clsapi_wifi_get_ssid(const char *ifname, char *ssid)
{
	int ret = -1;

	if(!ifname || (strlen(ifname) == 0))
		snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli -iwlan1 get_config | grep -i ^SSID | awk -F'=' '{print $2}'");
	else
		snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli -i%s get_config | grep -i ^SSID | awk -F'=' '{print $2}'", ifname);
	ret = clsapi_cmd_exec(gCmdStr, ssid);
	return ret;
}

int clsapi_wifi_get_bssid(const char *ifname, char *bssid)
{
	int ret = -1;

	if(!ifname || (strlen(ifname) == 0))
		snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli -iwlan1 get_config | grep -i ^bssid | awk -F'=' '{print $2}'");
	else
		snprintf(gCmdStr, sizeof(gCmdStr), "hostapd_cli -i%s get_config | grep -i bssid | awk -F'=' '{print $2}'", ifname);
	ret = clsapi_cmd_exec(gCmdStr, bssid);
	return ret;
}

int clsapi_get_wps_action_ifname(char *ifname, int band_idx)
{
	int ret = 0;
	char line[64];
	char *target = NULL;
	FILE *fp;

	sprintf(gCmdStr, "clsapi get mesh_bss_ifname phy%d> /tmp/wps_ifname.txt", band_idx);
	ret = system(gCmdStr);
	if (ret != 0) {
		cls_error("execute command failed: %s", gCmdStr);
		return -ENODATA;
	}
	fp = fopen("/tmp/wps_ifname.txt", "r+");
	if (fp == NULL) {
		cls_error("Open /tmp/wps_ifname.txt failed");
		return -ENODATA;
	}

	while (fgets(line, 32, fp) != NULL) {
		if ((target = strstr(line, "Backhaul STA: "))) {
			snprintf(ifname, IFNAMSIZ, "%s", target+14);
			cls_error("found sta-if:%s\n", ifname);
			break;
		}
	}
	fclose(fp);
	return 0;
}

