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
#include <netinet/in.h>
#include <netinet/ether.h>
#include <pthread.h>
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "common/csigma_common.h"
#include "common/csigma_log.h"
#include "common/cls_cmd_parser.h"
#include "common/cls_defconf.h"
#include "cls_dut_common.h"
#include <net80211/ieee80211.h>
#include "common/cls_timer.h"

#define CLS_DUT_CONFIG_TABSIZE		8

static struct cls_dut_config cls_dut_config_table[CLS_DUT_CONFIG_TABSIZE];

static int cls_dut_config_head = 0;
static int cls_dut_config_tail = 0;

static char cls_dut_sigma_interfaces[CLS_DUT_BANDS_NUM][IFNAMSIZ];
static char cls_sigma_tag_interface_map[CLS_DUT_TAG_NUM][IFNAMSIZ];
static struct radio_band radio_band_map[CLS_DUT_BANDS_NUM];
static int cls_first_active_interface_band_idx;
static int cls_dut_configure_ie_flag[CLS_DUT_BANDS_NUM];
static int cls_dut_reg_domain_global_flag;
static int cls_dut_num_snd_dim[CLS_DUT_BANDS_NUM];
static struct cls_vap vap_5g[3];
static struct cls_vap vap_24g[3];

struct list_head neigh_list;
struct cls_ap_btm_req btm_req;
struct ubus_subscriber_ctx cls_ubus_notifcation;
struct ubus_context *mbo_ubus_ctx = NULL;
static char cls_ubus_listen_ifname[16] = {0};
static pthread_t ubus_listen_thread = 0;
struct cls_non_pref_entry nonpref_info;

extern struct cls_ap_bss_term_params g_bss_term_params;

static
int cls_dut_print_base_response(char *buf_ptr, int buf_size, int status, int err_code)
{
	int rsp_len;
	const char *status_str;
	int need_err_code = 0;

	switch (status) {
	case STATUS_RUNNING:
		status_str = "RUNNING";
		break;

	case STATUS_INVALID:
		status_str = "INVALID";
		need_err_code = (err_code != 0);
		break;

	case STATUS_ERROR:
		status_str = "ERROR";
		need_err_code = (err_code != 0);
		break;

	case STATUS_COMPLETE:
		status_str = "COMPLETE";
		break;

	default:
		status_str = "INVALID";
		break;
	}

	if (need_err_code)
		rsp_len = snprintf(buf_ptr, buf_size, "%s,errorCode,%d", status_str, err_code);
	else
		rsp_len = snprintf(buf_ptr, buf_size, "%s", status_str);

	return rsp_len;
}

void cls_dut_make_response_none(int tag, int status, int err_code, int *out_len,
	unsigned char *out_buf)
{
	char rsp_buf[128];
	int rsp_len;

	rsp_len = cls_dut_print_base_response(rsp_buf, sizeof(rsp_buf), status, err_code);

	if (rsp_len > 0) {
		wfaEncodeTLV(tag, rsp_len, (BYTE *) rsp_buf, out_buf);
		*out_len = WFA_TLV_HDR_LEN + rsp_len;
	}
}

void cls_dut_make_response_macaddr(int tag, int status, int err_code, const unsigned char *macaddr,
	int *out_len, unsigned char *out_buf)
{
	char rsp_buf[128];
	int rsp_len;

	rsp_len = cls_dut_print_base_response(rsp_buf, sizeof(rsp_buf), status, err_code);

	if (rsp_len > 0) {
		if (status == STATUS_COMPLETE) {
			int len = snprintf(rsp_buf + rsp_len, sizeof(rsp_buf) - rsp_len,
				",mac,%02x:%02x:%02x:%02x:%02x:%02x",
				macaddr[0], macaddr[1], macaddr[2],
				macaddr[3], macaddr[4], macaddr[5]);

			if (len > 0)
				rsp_len += len;
		}

		wfaEncodeTLV(tag, rsp_len, (BYTE *) rsp_buf, out_buf);
		*out_len = WFA_TLV_HDR_LEN + rsp_len;
	}
}

void cls_dut_make_response_vendor_info(int tag, int status, int err_code, const char *vendor_info,
	int *out_len, unsigned char *out_buf)
{
	char rsp_buf[512];
	int rsp_len;

	rsp_len = cls_dut_print_base_response(rsp_buf, sizeof(rsp_buf), status, err_code);
	if (rsp_len > 0) {
		if ((status == STATUS_COMPLETE) && vendor_info && *vendor_info) {
			int len = snprintf(rsp_buf + rsp_len, sizeof(rsp_buf) - rsp_len,
				",%s", vendor_info);

			if (len > 0)
				rsp_len += len;
		}

		wfaEncodeTLV(tag, rsp_len, (BYTE *) rsp_buf, out_buf);
		*out_len = WFA_TLV_HDR_LEN + rsp_len;
	}
}

void cls_dut_make_response_mid(int tag, int status, int err_code, char *mid,
											int *out_len, unsigned char *out_buf)
{
	char rsp_buf[512];
	int rsp_len;

	rsp_len = cls_dut_print_base_response(rsp_buf, sizeof(rsp_buf), status, err_code);
	if (rsp_len > 0) {
		if ((status == STATUS_COMPLETE)) {
			int len = snprintf(rsp_buf + rsp_len, sizeof(rsp_buf) - rsp_len,
				",mid,%s", mid);

			if (len > 0)
				rsp_len += len;
		}

		wfaEncodeTLV(tag, rsp_len, (BYTE *) rsp_buf, out_buf);
		*out_len = WFA_TLV_HDR_LEN + rsp_len;
	}
}

void cls_set_sigma_interface(const char *interface, enum cls_dut_band_index band_index)
{
	strncpy(cls_dut_sigma_interfaces[band_index], interface,
		sizeof(cls_dut_sigma_interfaces[band_index]));

	cls_dut_sigma_interfaces[band_index][sizeof(cls_dut_sigma_interfaces[band_index]) - 1] = '\0';
	PRINT("%s:%d, %s: cls_dut_sigma_interfaces:%s\n", __FILE__, __LINE__, __func__,
					cls_dut_sigma_interfaces[band_index]);
}

void cls_init_sigma_interfaces(void)
{
	static char if_name_buf[IFNAMSIZ];
	int ret = 0;
	enum cls_dut_band_index band = CLS_DUT_2P4G_BAND;
	enum cls_dut_band_index band_idx = CLS_DUT_2P4G_BAND;
	clsapi_unsigned_int band_value;
	clsapi_unsigned_int channel_value;
	clsapi_unsigned_int bandwidth_value;

	PRINT("%s:%d, Entering %s =====>\n",
		__FILE__, __LINE__, __func__);
	cls_clear_sigma_radio_band_info();
	PRINT("%s:%d, %s: cls_clear_sigma_radio_band_info\n",
		__FILE__, __LINE__, __func__);
	cls_clear_sigma_tag_interface_map();
	PRINT("%s:%d, %s: cls_clear_sigma_tag_interface_map\n",
		__FILE__, __LINE__, __func__);

	for (int radio_id = 0; radio_id < CLS_MAX_RADIO_ID; ++radio_id) {
		ret = clsapi_radio_get_primary_interface(radio_id, if_name_buf, sizeof(if_name_buf));
		if (ret < 0)
			continue;

#if 0
		char status[32] = {0};
		ret = clsapi_interface_get_status(if_name_buf, status);
		if (ret < 0)
			continue;
#endif

		ret = clsapi_wifi_get_chan(if_name_buf,
					&channel_value, &bandwidth_value, &band_value);
		PRINT("%s:%d, %s: clsapi_wifi_get_chan: channel: %d, bandwidth: %d, band: %d\n",
			__FILE__, __LINE__, __func__, channel_value, bandwidth_value, band_value);
		if (ret != 0)
			continue;

		cls_set_sigma_interface_radio_band_info(band_idx++,
			radio_id, band_value, if_name_buf);

		cls_set_sigma_interface(if_name_buf, band++);

		if (band == CLS_DUT_BANDS_NUM)
			break;
	}

	cls_set_sigma_first_active_band_idx(CLS_DUT_BANDS_NUM);
}

void cls_set_sigma_first_active_band_idx(enum cls_dut_band_index band_index)
{
	cls_first_active_interface_band_idx = band_index;
	PRINT("%s:%d, %s: %d\n", __FILE__, __LINE__, __func__,
					band_index);
}

int cls_get_sigma_first_active_band_idx(void)
{
	return cls_first_active_interface_band_idx;
}

int cls_get_sigma_interface_band_idx(enum clsapi_freq_band band)
{
	int found_entry = 0;
	unsigned int iter;
	PRINT("%s:%d, %s: freq_band=%d\n", __FILE__, __LINE__, __func__,
					band);
	for (iter = 0; iter < CLS_DUT_BANDS_NUM
	&& radio_band_map[iter].band != clsapi_freq_band_unknown && found_entry == 0; iter++) {
		PRINT("%s:%d, %s: iter=%d, radio_band_map.band=%d\n", __FILE__, __LINE__, __func__,
			iter, radio_band_map[iter].band);
		if(radio_band_map[iter].band == band) {
			found_entry = 1;
			PRINT("%s:%d, %s: %d\n", __FILE__, __LINE__, __func__,
					iter);
			return iter;
		}
	}

	PRINT("%s:%d, %s: %d\n", __FILE__, __LINE__, __func__,
					CLS_DUT_BANDS_NUM);
	return CLS_DUT_BANDS_NUM;

}

const char *cls_get_sigma_interface_for_band(enum cls_dut_band_index band_index)
{
	if (band_index >= CLS_DUT_BANDS_NUM)
		band_index = CLS_DUT_BANDS_NUM;

	PRINT("%s:%d, %s:dut_sigma_interfaces = %s, %s, %s\n", __FILE__, __LINE__, __func__,
		cls_dut_sigma_interfaces[0],
		cls_dut_sigma_interfaces[1],
		cls_dut_sigma_interfaces[2]);
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__,
		cls_dut_sigma_interfaces[band_index]);
	return cls_dut_sigma_interfaces[band_index];
}

void cls_set_sigma_vap_interface(const char *ifname,
	unsigned int vap_index, enum cls_dut_band_index band_index)
{
	if (band_index == 0) {
		vap_24g[vap_index].band = band_index;
		strcpy(vap_24g[vap_index].name, ifname);
		PRINT("%s:%d, %s: band%d: %s\n", __FILE__, __LINE__, __func__,
				vap_24g[vap_index].band, vap_24g[vap_index].name);
	} else if (band_index == 1) {
		vap_5g[vap_index].band = band_index;
		strcpy(vap_5g[vap_index].name, ifname);
		PRINT("%s:%d, %s: band%d: %s\n", __FILE__, __LINE__, __func__,
				vap_5g[vap_index].band, vap_5g[vap_index].name);
	}
}

const char *cls_get_sigma_vap_interface(unsigned int vap_index, enum cls_dut_band_index band_index)
{
	if (band_index == 0) {
		PRINT("%s:%d, %s: band%d: %s\n", __FILE__, __LINE__, __func__,
			vap_24g[vap_index].band, vap_24g[vap_index].name);
		return vap_24g[vap_index].name;
	} else if (band_index == 1) {
		PRINT("%s:%d, %s: band%d: %s\n", __FILE__, __LINE__, __func__,
			vap_5g[vap_index].band, vap_5g[vap_index].name);
		return vap_5g[vap_index].name;
	}
	return NULL;
}

const int cls_get_sigma_verify_channel_for_band(int band_idx, clsapi_unsigned_int channel_value)
{
	printf("entering cls_get_sigma_verify_channel_for_band ==>\n");
	struct clsapi_data_256bytes list_of_channels;
	const char *if_name;
	int result;
	int indx;
	uint8_t chan_num;

	if_name = cls_get_sigma_interface_for_band(band_idx);
	cls_log("cls_get_sigma_interface_for_band: if_name: %s\n", if_name);

	result = clsapi_wifi_get_chan_list(band_idx, &list_of_channels, &chan_num, clsapi_chlist_flag_available);
	if (result < 0) {
		cls_error("can't get channels list from: %s, error %d", if_name, result);
		return -1;
	}

	for (indx = 0; indx < chan_num; indx++) {
		if (channel_value == list_of_channels.data[indx])
			return 0;
	}

	cls_error("required channel %d is not in the channels list for %s", channel_value, if_name);

	return -1;
}

const int cls_is_dcdc_configuration(void)
{
	const int ch1 = DEFAULT_VHT_CHANNEL_DCDC_1;
	const int ch2 = DEFAULT_VHT_CHANNEL_DCDC_0;

	int band1_has_ch1 = !cls_get_sigma_verify_channel_for_band(CLS_DUT_5G_BAND, ch1);
	int band1_has_ch2 = !cls_get_sigma_verify_channel_for_band(CLS_DUT_5G_BAND, ch2);
	int band2_has_ch1 = !cls_get_sigma_verify_channel_for_band(CLS_DUT_2P4G_BAND, ch1);
	int band2_has_ch2 = !cls_get_sigma_verify_channel_for_band(CLS_DUT_2P4G_BAND, ch2);

	PRINT("%s:%d, %s: band1_has_ch100=%d, band2_has_ch36=%d, band1_has_ch36=%d, band2_has_ch100=%d\n",
		__FILE__, __LINE__, __func__,
		band1_has_ch2, band2_has_ch1, band1_has_ch1, band2_has_ch2);
	if (band1_has_ch2 && band2_has_ch1
			&& !band1_has_ch1 && !band2_has_ch2)
		return 1;

	return 0;
}

int cls_dut_sigma_dual_band(void)
{
	PRINT("%s:%d, %s: 2nd_band=%d, 3rd_band=%d\n", __FILE__, __LINE__, __func__,
		cls_dut_sigma_interfaces[CLS_DUT_2P4G_BAND][0],
		cls_dut_sigma_interfaces[CLS_DUT_6G_BAND][0]);
	return (cls_dut_sigma_interfaces[CLS_DUT_2P4G_BAND][0] != '\0' &&
		cls_dut_sigma_interfaces[CLS_DUT_6G_BAND][0] == '\0');
}

int cls_dut_sigma_tri_band(void)
{
	PRINT("%s:%d, %s: 2nd_band=%d, 3rd_band=%d\n", __FILE__, __LINE__, __func__,
					cls_dut_sigma_interfaces[CLS_DUT_2P4G_BAND][0],
					cls_dut_sigma_interfaces[CLS_DUT_6G_BAND][0]);
	return (cls_dut_sigma_interfaces[CLS_DUT_2P4G_BAND][0] != '\0' &&
		cls_dut_sigma_interfaces[CLS_DUT_6G_BAND][0] != '\0');
}

const char *cls_get_sigma_interface(void)
{
	int band_idx = cls_get_sigma_first_active_band_idx();
	PRINT("%s:%d, %s: band_index=%d\n", __FILE__, __LINE__, __func__,
					band_idx);
	return cls_get_sigma_interface_for_band(band_idx);
}

const char *cls_get_sigma_interface_name_from_cmd(const char *cmd, unsigned int vap_index)
{
	enum cls_dut_band_index band;
	const char *ifname;

	if (strcasecmp(cmd, "6G") == 0) {
		band = cls_get_sigma_interface_band_idx(clsapi_freq_band_6_ghz);
		ifname = cls_get_sigma_vap_interface(vap_index, band);
	} else if (strcasecmp(cmd, "5G") == 0 || strcasecmp(cmd, "5.0") == 0 ||
			strcasecmp(cmd, "50G") == 0) {
		band = cls_get_sigma_interface_band_idx(clsapi_freq_band_5_ghz);
		ifname = cls_get_sigma_vap_interface(vap_index, band);
	} else if (strcasecmp(cmd, "24G") == 0 || strcasecmp(cmd, "2.4G") == 0 ||
			strcasecmp(cmd, "2G") == 0 || strcasecmp(cmd, "2.4") == 0) {
		band = cls_get_sigma_interface_band_idx(clsapi_freq_band_2pt4_ghz);
		ifname = cls_get_sigma_vap_interface(vap_index, band);
	} else {
		ifname = cmd;
	}

	return ifname;
}

void cls_get_sigma_primary_ifname_from_cmd(char *cmd, char *ifname)
{
	if (strcasecmp(cmd, "6G") == 0) {
		sprintf(ifname, "wlan%d", CLS_DUT_6G_BAND);
	} else if (strcasecmp(cmd, "5G") == 0 || strcasecmp(cmd, "5.0") == 0 ||
		strcasecmp(cmd, "50G") == 0) {
		sprintf(ifname, "wlan%d", CLS_DUT_5G_BAND);
	} else if (strcasecmp(cmd, "24G") == 0 || strcasecmp(cmd, "2.4G") == 0 ||
		strcasecmp(cmd, "2G") == 0 || strcasecmp(cmd, "2.4") == 0) {
		sprintf(ifname, "wlan%d", CLS_DUT_2P4G_BAND);
	} else {
		ifname = cmd;
	}

	return;
}

int cls_get_sigma_band_info_from_interface(const char *ifname)
{
	int found_entry = 0;
	unsigned int iter;

	if (!ifname)
		return clsapi_freq_band_unknown;

	cls_log("%s, %s\n", __func__, ifname);
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, ifname);

	for (iter = 0; iter < CLS_DUT_BANDS_NUM && found_entry == 0; iter++) {
		if (strcasecmp(radio_band_map[iter].ifname, ifname) == 0) {
			found_entry = 1;
			PRINT("%s:%d, %s: %d\n", __FILE__, __LINE__, __func__,
				radio_band_map[iter].band);
			return radio_band_map[iter].band;
		}
	}

	return clsapi_freq_band_unknown;
}

void cls_set_sigma_tag_interface_map(const char *interface,
				enum cls_dut_tag_index tag_index)
{
	if (!interface || interface[0] == '\0' || tag_index < CLS_DUT_FIRST_TAG
			|| tag_index >= CLS_DUT_UNKNOWN_TAG)
		return;

	strncpy(cls_sigma_tag_interface_map[tag_index - 1], interface,
		sizeof(cls_sigma_tag_interface_map[tag_index - 1]));
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__,
		cls_sigma_tag_interface_map[tag_index - 1]);

	cls_sigma_tag_interface_map[tag_index - 1][
			sizeof(cls_sigma_tag_interface_map[tag_index - 1]) - 1] = '\0';
}

const char *cls_get_sigma_tag_interface_map(enum cls_dut_tag_index tag_index)
{
	if (tag_index < CLS_DUT_FIRST_TAG || tag_index >= CLS_DUT_UNKNOWN_TAG)
		return NULL;

	if (!cls_sigma_tag_interface_map[tag_index - 1][0] ||
				cls_sigma_tag_interface_map[tag_index - 1][0] == '\0')
		return NULL;
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__,
		cls_sigma_tag_interface_map[tag_index - 1]);
	return cls_sigma_tag_interface_map[tag_index - 1];
}

void cls_dut_reset_config(struct cls_dut_config *conf)
{
	if (conf) {
		conf->bws_enable = 0;
		conf->bws_dynamic = 0;
		conf->force_rts = 0;
		conf->update_settings = 0;
		conf->testbed_enable = 0;
		conf->dut_enable = 0;
		if (conf->dpp_config)
			free(conf->dpp_config);
		conf->dpp_config = NULL;
	}
}

struct cls_dut_config *cls_dut_get_config(const char *ifname)
{
	cls_log("cls_dut_get_config ifname %s", ifname);
	PRINT("%s:%d, %s: ifname=%s\n", __FILE__, __LINE__, __func__, ifname);
	int len = strlen(ifname);

	if ((len > 0) && (len < sizeof(((struct cls_dut_config*)NULL)->ifname))) {
		int i;
		struct cls_dut_config *conf;

		for (i = cls_dut_config_head;
				i != cls_dut_config_tail;
				i = (i + 1) % CLS_DUT_CONFIG_TABSIZE) {

			conf = &cls_dut_config_table[i];

			if (strncasecmp(conf->ifname, ifname, len) == 0)
				if (strlen(conf->ifname) == len)
					return conf;
		}

		/* there is no config, allocate next */
		conf = &cls_dut_config_table[cls_dut_config_tail];

		cls_dut_reset_config(conf);

		strncpy(conf->ifname, ifname, len);
		conf->ifname[len] = 0;

		cls_dut_config_tail = (cls_dut_config_tail + 1) % CLS_DUT_CONFIG_TABSIZE;

		if (cls_dut_config_head == cls_dut_config_tail) {
			cls_dut_config_head = (cls_dut_config_head + 1) % CLS_DUT_CONFIG_TABSIZE;
		}

		return conf;
	}

	return NULL;
}

void cls_clear_sigma_radio_band_info(void)
{
	memset(radio_band_map, 0, sizeof(radio_band_map));
}

void cls_clear_sigma_tag_interface_map(void)
{
	memset(cls_sigma_tag_interface_map, 0, sizeof(cls_sigma_tag_interface_map));
}

void cls_set_sigma_interface_radio_band_info(enum cls_dut_band_index band_idx,
		int radio_id, enum clsapi_freq_band band_value, const char *ifname)
{
	int ifname_len;

	if (!ifname || ifname[0] == '\0')
		return;

	ifname_len = strlen(ifname);
	radio_band_map[band_idx].radio = radio_id;
	radio_band_map[band_idx].band = band_value;
	strncpy(radio_band_map[band_idx].ifname, ifname, MIN(IFNAMSIZ -1, ifname_len));
	cls_log("%s, radio: %d, band: %d, interface: %s\n", __func__,
		radio_id, band_value, radio_band_map[band_idx].ifname);
	PRINT("%s:%d, %s: radio: %d, band: %d, interface: %s\n", __FILE__, __LINE__, __func__,
			radio_id, band_value, radio_band_map[band_idx].ifname);
}

void cls_dut_set_reg_domain_global_flag(int enable)
{
	cls_dut_reg_domain_global_flag = enable;
}

int cls_dut_get_reg_domain_global_flag(void)
{
	return cls_dut_reg_domain_global_flag;
}

char *cls_get_sigma_interface_name(enum clsapi_freq_band band)
{
	int found_entry = 0;
	unsigned int iter;

	for (iter = 0; iter < CLS_DUT_BANDS_NUM
	&& radio_band_map[iter].band != clsapi_freq_band_unknown && found_entry == 0; iter++) {
		if (radio_band_map[iter].band == band) {
			found_entry = 1;
			PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__,
				radio_band_map[iter].ifname);
			return radio_band_map[iter].ifname;
		}
	}
	PRINT("%s:%d, %s: interface not found\n", __FILE__, __LINE__, __func__);
	return NULL;
}

int cls_set_rf_enable(int enable)
{
	char cmd[128];
//TODO later
	snprintf(cmd, sizeof(cmd), "enable rf");
	PRINT("%s:%d, %s: enable rf\n", __FILE__, __LINE__, __func__);
	system(cmd);
}

int cls_set_rf_enable_timeout(int enable, int timeout_secs)
{
	PRINT("entering cls_set_rf_enable_timeout ==>\n");
	int ret = cls_set_rf_enable(enable);

	sleep(timeout_secs);

	return ret;
}

int cls_set_mcs_cap(const char *ifname, unsigned int mcs_set)
{
	char tmpbuf[128];

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_mcs_cap %d", ifname, mcs_set);
	PRINT("%s:%d, %s: iwpriv %s set_vht_mcs_cap %d\n",
		__FILE__, __LINE__, __func__, ifname, mcs_set);
	system(tmpbuf);

	if (cls_is_11ax_mode(ifname)) {
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_he_mcs_cap %d", ifname, mcs_set);
		PRINT("%s:%d, %s: iwpriv %s set_he_mcs_cap %d\n",
			__FILE__, __LINE__, __func__, ifname, mcs_set);
		system(tmpbuf);
	}

	return 0;
}

int cls_is_2p4_interface(const char *ifname)
{
	unsigned int radio_id;
	int ret = 0;
	char tmp[64];

	clsapi_get_radio_from_ifname(ifname, &radio_id);

	if (radio_id == 0){
		ret = 1;
		PRINT("%s:%d, %s: True\n", __FILE__, __LINE__, __func__);
	}
	return ret;
}

int cls_is_11ax_mode(const char *ifname)
{
	char phy_mode[32] = {0};
	int ret = clsapi_wifi_get_phy_mode(ifname, phy_mode);

	return ret == 0 && strstr(phy_mode, "ax") != NULL;
}

int cls_set_nss_mcs_cap(const char *ifname, int nss_cap, int mcs_low, int mcs_high)
{
	int mcs_cap = 0;

	if (mcs_low != 0) {
		return -EINVAL;
	}

	if (cls_is_11ax_mode( ifname)) {
		switch (mcs_high) {
		case 7:
			mcs_cap = IEEE80211_HE_MCS_0_7;
			break;
		case 9:
			mcs_cap = IEEE80211_HE_MCS_0_9;
			break;
		case 11:
			mcs_cap = IEEE80211_HE_MCS_0_11;
			break;
		default:
			return -EINVAL;
		}
	} else {
		switch (mcs_high) {
		case 7:
			mcs_cap = IEEE80211_VHT_MCS_0_7;
			break;
		case 8:
			mcs_cap = IEEE80211_VHT_MCS_0_8;
			break;
		case 9:
			mcs_cap = IEEE80211_VHT_MCS_0_9;
			break;
		default:
			return -EINVAL;
		}
	}
	PRINT("%s:%d, %s: mcs_cap=%d\n", __FILE__, __LINE__, __func__, mcs_cap);
	return cls_set_mcs_cap(ifname, mcs_cap);
}

int cls_dut_disable_mu_bf(const char *ifname)
{
	char tmpbuf[64];
	unsigned int id;

	clsapi_get_radio_from_ifname(ifname, &id);

	snprintf(tmpbuf, sizeof(tmpbuf), "mu %d disable", id);
	PRINT("%s:%d, %s: %s, mu %d disable\n", __FILE__, __LINE__, __func__, ifname, id);
	system(tmpbuf);

	return clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
}

void cls_dut_backup_num_snd_dim(int num_snd_dim, enum cls_dut_band_index band_idx)
{
	cls_dut_num_snd_dim[band_idx] = num_snd_dim;
}

int cls_set_fixed_bw(const char *ifname, int bw)
{
	char tmpbuf[128];

	if (bw) {
		snprintf(tmpbuf, sizeof(tmpbuf), "set_fixed_bw %s -b %d", ifname, bw);
		PRINT("%s:%d, %s: set_fixed_bw %s -b %d\n",
			__FILE__, __LINE__, __func__, ifname, bw);
	} else {
		snprintf(tmpbuf, sizeof(tmpbuf), "set_fixed_bw %s -b auto", ifname);
		PRINT("%s:%d, %s: set_fixed_bw %s -b auto\n",
			__FILE__, __LINE__, __func__, ifname);
	}

	return system(tmpbuf);
}

void cls_set_rts_settings(const char *ifname, struct cls_dut_config *conf)
{
	char tmpbuf[128];
	/* RTS with BW signaling:
		iwpriv wifi0 set_rts_bw 0xXYZ
	where:
		X: 1  --  do not force RTS sendding;
		Y: 2  --  80MHz, 1   -- 40MHz, 0  -- 20MHz
		Z: 1 -- dynamic,  0  -- static  */

	uint16_t bw_sign = 0x100;

	if (conf->bws_enable) {
		bw_sign |= 0x20;
	}

	if (conf->bws_enable && conf->bws_dynamic) {
		bw_sign |= 1;
	}

	if (conf->force_rts) {
		bw_sign &= -0x100;
	}

	if (conf->bws_enable && !conf->bws_enable) {
		/* WFA expect that when static bw signalling is unsed DUT will stay only at fixed bw
		i.e 80MHz  */
		cls_set_fixed_bw(ifname, 80);
	} else {
		cls_set_fixed_bw(ifname, 0);
	}

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_rts_bw 0x%x", ifname, bw_sign);
	PRINT("%s:%d, %s: iwpriv %s set_rts_bw 0x%x\n",
		__FILE__, __LINE__, __func__, ifname, bw_sign);
	system(tmpbuf);
	conf->update_settings = 0;
}

int cls_set_phy_mode(const char *ifname, const char *phy_mode)
{
	/*const int is_2p4_interface = cls_is_2p4_interface( ifname);

	if (is_2p4_interface) {
		if (strcasecmp(phy_mode, "11ax") == 0) {
			phy_mode = "11axng";
		}
	}
	*/

	PRINT("%s:%d, %s: phy_mode: %s\n", __FILE__, __LINE__, __func__, phy_mode);

	return clsapi_wifi_set_phy_mode(ifname, phy_mode);
}

int cls_set_nss_cap(const char *ifname, int mimo_type, unsigned int nss)
{
	PRINT("entering cls_set_nss_cap ==>\n");
	int ret;

	if(mimo_type ==clsapi_mimo_ht)
		return clsapi_wifi_set_nss_cap(ifname, clsapi_mimo_ht, nss);

	ret = clsapi_wifi_set_nss_cap(ifname, clsapi_mimo_vht, nss);
	if (cls_is_11ax_mode(ifname) || mimo_type == clsapi_mimo_he) {
		/*assume same NSS configuration for HE and VHT since CAPI does really
		specify which capabilities should be updated.  */
		ret = clsapi_wifi_set_nss_cap(ifname, clsapi_mimo_he, nss);
	}

	return ret;
}

int cls_set_mu_enable(const char *ifname, int enable)
{
	char tmp[128];
	clsapi_wifi_mode wifi_mode = clsapi_access_point;

	/* limit NSS to 2*/
	unsigned int sta_nss = 2;
	unsigned int id;

	clsapi_get_radio_from_ifname(ifname, &id);

	if (clsapi_wifi_get_mode(ifname, &wifi_mode) < 0) {
		cls_log("Can't get wifi mode, assume AP");
		PRINT("%s:%d, %s: Can't get wifi mode, assume AP\n", __FILE__, __LINE__, __func__);
		wifi_mode = clsapi_access_point;
	}

	if (wifi_mode == clsapi_station) {
		FILE *f =fopen("/tmp/test.txt", "r");
		if (f) {
			if (fscanf(f, "%d", &sta_nss) != 1)
				sta_nss = 2;
			fclose(f);
		}

		if (cls_set_nss_cap(ifname, clsapi_mimo_vht, sta_nss) < 0)
			cls_log("Can't set NSS capability");
		PRINT("%s:%d, %s: Can't set NSS capability\n", __FILE__, __LINE__, __func__);
	}

	snprintf(tmp, sizeof(tmp), "mu %d %s", id, enable ? "enable" : "disable");
	PRINT("%s:%d, %s: mu %d %s\n", __FILE__, __LINE__, __func__,
		id, enable ? "enable" : "disable");
	return system(tmp);
}

void cls_set_ampdu(const char *ifname, int enable)
{
	char tmpbuf[64];

	int ba_control = enable ? 0xFFFF : 0;
	int implicit_ba = enable ? 0x1 : 0;

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ba_control %d", ifname, ba_control);
	PRINT("%s:%d, %s: iwpriv %s ba_control %d\n",
		__FILE__, __LINE__, __func__, ifname, ba_control);
	system(tmpbuf);
	clsapi_wifi_set_txba_disable(ifname, !enable);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s implicit_ba %d", ifname, ba_control);
	PRINT("%s:%d, %s: iwpriv %s implicit_ba %d\n",
		__FILE__, __LINE__, __func__, ifname, ba_control);
	system(tmpbuf);
}

int cls_set_amsdu(const char *ifname, int enable)
{
	struct cls_dut_config *conf = cls_dut_get_config(ifname);
	const int testbed = conf && conf->testbed_enable;
	char tmpbuf[128];

	snprintf(tmpbuf, sizeof(tmpbuf),
	"iw dev %s vendor send 0xD04433 0x02 0x08 0x%02x", ifname, enable);
	int result = system(tmpbuf);

	PRINT("%s:%d, %s: Set AMSDU: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
	if (result != 0) {
		cls_error("Failed to set amsdu for %s value %d", ifname, enable);
		PRINT("%s:%d, %s: Failed to set amsdu for %s value %d\n",
			__FILE__, __LINE__, __func__, ifname, enable);
		return -ENODATA;
	}

	return result;
}

#define IEEE80211_AX_MCS_VAL_MASK	0x0F
#define IEEE80211_AX_MCS_NSS_MASK	0xF0
#define IEEE80211_11AX_MCS_NSS_SHIFT	4

int cls_set_fixed_mcs_rate(const char *ifname, int tx_rate, const char *phy_mode)
{

	PRINT("entering cls_set_fixed_mcs_rate ==>\n");
	clsapi_unsigned_int nss = 1;

	PRINT("%s:%d, %s: phy_mode is %s\n",
		__FILE__, __LINE__, __func__, phy_mode);

	if (strstr(phy_mode, "ax")) {
		nss = ((tx_rate & IEEE80211_AX_MCS_NSS_MASK) >> IEEE80211_11AX_MCS_NSS_SHIFT) + 1;
		tx_rate &= IEEE80211_AX_MCS_VAL_MASK;
	}

	return cls_set_fixed_tx_mcs_nss(ifname, nss, tx_rate, phy_mode);
}

int cls_set_fixed_tx_mcs_nss(const char *ifname, int num_ss, int mcs, const char *phy_mode)
{
	char val_str[128];
	clsapi_mcs_rate mcs_rate;
	unsigned int radio_id;
	int ret;

	ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
	if (ret < 0) {
		cls_error("Failed to get radio id from interface name ifname = %s", ifname);
		PRINT("%s:%d, %s: Failed to get radio id from interface name ifname = %s\n",
			__FILE__, __LINE__, __func__, ifname);
		return ret;
	}

	if (strstr(phy_mode, "ax")) {
		uint32_t tx_rate = (mcs & IEEE80211_AX_MCS_VAL_MASK |
			((num_ss - 1) << IEEE80211_11AX_MCS_NSS_SHIFT));

		switch(radio_id) {
		case 0:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates he-mcs-%s %d:%d",
			ifname, "2.4", num_ss, tx_rate);
		break;
		case 1:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates he-mcs-%s %d:%d",
			ifname, "5", num_ss, tx_rate);
		break;
		case 2:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates he-mcs-%s %d:%d",
			ifname, "6", num_ss, tx_rate);
		break;
		}

	} else if (strstr(phy_mode, "ac")) {
		switch(radio_id) {
		case 0:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates vht-mcs-%s %d:%d",
			ifname, "2.4", num_ss, mcs);
		break;
		case 1:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates vht-mcs-%s %d:%d",
			ifname, "5", num_ss, mcs);
		break;
		case 2:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates vht-mcs-%s %d:%d",
			ifname, "6", num_ss, mcs);
		break;
		}

	} else if (strstr(phy_mode, "n")) {
		/*assume 11n or below */
		switch(radio_id) {
		case 0:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates ht-mcs-%s %d",
			ifname, "2.4", mcs);
		break;
		case 1:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates ht-mcs-%s %d",
			ifname, "5", mcs);
		break;
		}
	} else {
		switch(radio_id) {
		case 0:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates legacy-%s %d",
			ifname, "2.4", mcs);
		break;
		case 1:
		snprintf(val_str, sizeof(val_str), "iw dev %s set bitrates legacy-%s %d",
			ifname, "5",  mcs);
		break;
		}
	}

	ret = system(val_str);
	PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, val_str);

	return ret;

}

int cls_set_nss_mcs_opt(const char *ifname, const char *nss_mcs_opt, const char *phy_mode)
{
	int mcs;
	int nss;
	/* nss_mcs_opt can be (N and M are Number of SS and MCS):
	 *	N;M
	 *	N;def
	 *	def;def
	 */

	if (sscanf(nss_mcs_opt, "%d;%d", &nss, &mcs) == 2) {
		return cls_set_fixed_tx_mcs_nss(ifname, nss, mcs, phy_mode);
	} else if (sscanf(nss_mcs_opt, "%d;def", &nss) == 1) {
		return cls_set_nss_cap(ifname, clsapi_mimo_vht, nss);
	}

	/* assume def;def here. Just return to auto-rate */
	return clsapi_wifi_set_option(ifname, clsapi_autorate_fallback, 1);

}

int cls_set_he_ltf_gi(const char *ifname, char *he_ltf_str, char *he_gi_str, int ofdma_val)
{
	char val_str[128];
	const char he_ltf_32us[] = "3.2";
	const char he_ltf_64us[] = "6.4";
	const char he_ltf_128us[] = "12.8";
	const char he_gi_08us[] = "0.8";
	const char he_gi_16us[] = "1.6";
	const char he_gi_32us[] = "3.2";
	int ret = 0;
	unsigned int radio_id;
	uint32_t he_ltf;
	uint32_t ndp_gi_ltf = CLS_HE_NDP_GI_LTF_TYPE_MAX;

	/* The HE PHY provides support for 3.2 μs (1x), 6.4 μs (2x), and 12.8 μs (4x) HE-LTF durations. */
	/* CLS_HE_LTF_1		1x HE-LTF */
	/* CLS_HE_LTF_2,	2x HE-LTF */
	/* CLS_HE_LTF_4,	4x HE-LTF */

	if (strncasecmp(he_ltf_str, he_ltf_32us, sizeof(he_ltf_32us)) == 0) {
		he_ltf =  CLS_HE_NDP_GI_LTF_TYPE_0; /* 1x HE-LTF */
	} else if (strncasecmp(he_ltf_str, he_ltf_64us, sizeof(he_ltf_64us)) == 0) {
		he_ltf = CLS_HE_NDP_GI_LTF_TYPE_1; /* 2x HE-LTF */
	} else if (strncasecmp(he_ltf_str, he_ltf_128us, sizeof(he_ltf_128us)) == 0) {
		he_ltf = CLS_HE_NDP_GI_LTF_TYPE_2; /* 4x HE-LTF */
	} else {
		cls_error("error: unsupported HE LTF mode");
		PRINT("%s:%d, %s: error: unsupported HE LTF mode\n",
			__FILE__, __LINE__, __func__);
		return -EINVAL;
	}

	if (strncasecmp(he_gi_str, he_gi_08us, sizeof(he_gi_08us)) == 0) {
		ndp_gi_ltf = CLS_HE_NDP_GI_LTF_TYPE_0;
	} else if (strncasecmp(he_gi_str, he_gi_16us, sizeof(he_gi_16us)) == 0) {
		ndp_gi_ltf = CLS_HE_NDP_GI_LTF_TYPE_1;
	} else if (strncasecmp(he_gi_str, he_gi_32us, sizeof(he_gi_32us)) == 0) {
		ndp_gi_ltf = CLS_HE_NDP_GI_LTF_TYPE_2;
	} else {
		cls_error("error: unsupported HE GI mode");
		PRINT("%s:%d, %s: error: unsupported HE GI mode\n",
			__FILE__, __LINE__, __func__);
		return -EINVAL;
	}
	/**
	if (for_ndp && ndp_gi_ltf != CLS_HE_NDP_GI_LTF_TYPE_MAX) {
		snprintf(val_str, sizeof(val_str), "iwpriv %s he_ndp_gi_ltf %d",
										ifname, ndp_gi_ltf);
		return system(val_str);
	}
	**/
	/* set HE-LTF and GI */
	ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
	if (ret < 0) {
		cls_error("Failed to get radio id from interface name ifname = %s", ifname);
		PRINT("%s:%d, %s: Failed to get radio id from interface name ifname = %s\n",
			__FILE__, __LINE__, __func__, ifname);
		return ret;
	}
	if (ofdma_val == 0 || ofdma_val == 2) {
		snprintf(val_str, sizeof(val_str),
			"iw dev %s vendor send 0xD04433 0x01 0x08 0x%02x && \
			iw dev %s vendor send 0xD04433 0x01 0x09 0x%02x",
			ifname, ndp_gi_ltf, ifname, he_ltf);
		PRINT("%s:%d, %s: %s\n",
			__FILE__, __LINE__, __func__, val_str);
	} else {
		switch(radio_id){
			case 0:
			snprintf(val_str, sizeof(val_str),
				"iw dev %s set bitrates he-gi-%s %s he-ltf-%s %u",
				ifname, "2.4", he_gi_str, "2.4", he_ltf);
			PRINT("%s:%d, %s: iw dev %s set bitrates he-gi-%s %s he-ltf-%s %u\n",
				__FILE__, __LINE__, __func__, ifname, "2.4", he_gi_str, "2.4", he_ltf);
			break;
			case 1:
			snprintf(val_str, sizeof(val_str),
				"iw dev %s set bitrates he-gi-%s %s he-ltf-%s %u",
				ifname, "5", he_gi_str, "5", he_ltf);
			PRINT("%s:%d, %s: iw dev %s set bitrates he-gi-%s %s he-ltf-%s %u\n",
				__FILE__, __LINE__, __func__, ifname, "5", he_gi_str, "5", he_ltf);
			break;
			case 2:
			snprintf(val_str, sizeof(val_str),
				"iw dev %s set bitrates he-gi-%s %s he-ltf-%s %u",
				ifname, "6", he_gi_str, "6", he_ltf);
			PRINT("%s:%d, %s: iw dev %s set bitrates he-gi-%s %s he-ltf-%s %u\n",
				__FILE__, __LINE__, __func__, ifname, "6", he_gi_str, "6", he_ltf);
			break;
		}
	}
	ret = system(val_str);
	if ( ret != 0) {
		cls_error("Failed to set he-ltf-gi to %s", ifname);
		PRINT("%s:%d, %s: Failed to set he-ltf-gi to %s\n",
			__FILE__, __LINE__, __func__, ifname);
		return -ENODATA;
	}
	return ret;
}

int set_tx_bandwidth(const char *ifname, unsigned int bandwidth, const char *phy_mode)
{
	clsapi_unsigned_int current_bw;
	char tmpbuf[128];
	int ret;
	char md5[64];
	unsigned int radio_id;
	FILE *fp;
	if (clsapi_wifi_get_bw(ifname, &current_bw) < 0) {
		current_bw = 0;
		cls_error("can't get current bw");
		PRINT("%s:%d, %s: can't get current bw\n", __FILE__, __LINE__, __func__);
	}

	/* change bw only when current is not wide enough */
	if (bandwidth > current_bw) {
		ret = clsapi_wifi_set_bw(ifname, bandwidth, phy_mode);
		if (ret != 0) {
			cls_error("can't set bandwidth to %d, error %d", bandwidth, ret);
			PRINT("%s:%d, %s: can't set bandwidth to %d, error %d\n",
				__FILE__, __LINE__, __func__, bandwidth, ret);
			return ret;
		}
		ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
		if (ret < 0) {
			cls_error("error: couldn't get radio_id, ifname %s, error %d",
					ifname, ret);
			PRINT("%s:%d, %s: error: couldn't get radio_id, ifname %s, error %d\n",
				__FILE__, __LINE__, __func__, ifname, ret);
			return ret;
		}
		snprintf(tmpbuf, sizeof(tmpbuf),
			"md5sum /tmp/run/hostapd-phy%d.conf | cut -d' ' -f1",
			radio_id);
		fp = popen(tmpbuf, "r");
		if (fp == NULL) {
			PRINT("%s:%d, %s: error: couldn't get md5sum of hostapd-phy%d\n",
				__FILE__, __LINE__, __func__, radio_id);
			return -1;
		}
		fgets(md5, sizeof(md5), fp);
		pclose(fp);
		snprintf(tmpbuf, sizeof(tmpbuf),
			"cls set %s radio_config_id %s",
			ifname, md5);
		PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
		system(tmpbuf);
		snprintf(tmpbuf, sizeof(tmpbuf),
			"hostapd_cli -i %s reconfigure",
			ifname);
		PRINT("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
		system(tmpbuf);
		if (ret != 0) {
			cls_error("can't set bandwidth to %d, error %d", bandwidth, ret);
			PRINT("%s:%d, %s: can't set bandwidth to %d, error %d\n",
				__FILE__, __LINE__, __func__, bandwidth, ret);
			return -ENODATA;
		}
	}

	/* force RA to use only specified bandwidth */
	//cls_set_fixed_bw(ifname, bandwidth);

	return 0;
}

int cls_wifi_get_rf_chipid(unsigned int radio_id, int *chipid)
{
	int ret;
	string_64 str_value;

	ret = clsapi_radio_get_board_parameter(radio_id, clsapi_rf_chipid, str_value);
	if (ret < 0) {
		return ret;
	}

	if (sscanf(str_value, "%d", chipid) != 1) {
		return -EINVAL;
	}

	return 0;
}

int cls_parse_mac(const char *mac_str, unsigned char *mac)
{
	unsigned int tmparray[IEEE80211_ADDR_LEN];

	if (mac_str == NULL)
		return -EINVAL;

	if (sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
			&tmparray[0],
			&tmparray[1],
			&tmparray[2],
			&tmparray[3], &tmparray[4], &tmparray[5]) != IEEE80211_ADDR_LEN) {
		return -EINVAL;
	}

	mac[0] = tmparray[0];
	mac[1] = tmparray[1];
	mac[2] = tmparray[2];
	mac[3] = tmparray[3];
	mac[4] = tmparray[4];
	mac[5] = tmparray[5];

	return 0;
}

int cls_dut_get_configure_ie(enum cls_dut_band_index band_idx)
{	
	return cls_dut_configure_ie_flag[band_idx];
}

void cls_dut_clear_configure_ie(enum cls_dut_band_index band_idx)
{
	cls_dut_configure_ie_flag[band_idx] = 0;
}

void cls_clear_sigma_interfaces()
{
	memset(cls_dut_sigma_interfaces, 0, sizeof(cls_dut_sigma_interfaces));
}

const int cls_get_sigma_default_channel(const char *ifname)
{
	if (!strcasecmp(ifname, "wlan0_0"))
		return DEFAULT_HT_CHANNEL;
	else
		return DEFAULT_VHT_CHANNEL;
}

static void cls_init_btm_req_info(struct cls_ap_btm_req *btm_req, uint8_t bintval)
{
	memset(btm_req->sta_mac, 0, MAC_ADDR_STR_LEN);
	btm_req->disassoc_timer = 48; /* 5s, same to or less than TSF value, MBO-TestCase-4.2.5.4 */
	btm_req->validity_interval = 10000 / bintval;
	btm_req->nums_neigh = 0;
	memset(btm_req->neighs, 0, sizeof(btm_req->neighs));
	btm_req->mbo_reason = 0;
	btm_req->cell_pref = 0; /* Clourney DUT is NOT cellular awared, MBO-TestCase-4.2.5.4 */
	btm_req->subel_len = 0;
}

void cls_clear_neigh_list()
{
	if (list_empty(&neigh_list))
		return;
	list_free_items(&neigh_list, struct cls_neigh_report_entry, l);
}

void cls_add_neighbor(struct list_head *list, char *neigh_bssid, struct cls_neigh_report_entry **item)
{
	struct cls_neigh_report_entry *neigh = NULL;

	if (!list_empty(list)) {
		list_for_each_entry(neigh, list, l) {
			if (strncmp(neigh->neigh.bssid, neigh_bssid, MAC_ADDR_STR_LEN) == 0) {
				return;
			}
		}
	}
	neigh = (struct cls_neigh_report_entry *)calloc(1, sizeof(struct cls_neigh_report_entry));
	if (!neigh)
		return;
	strncpy(neigh->neigh.bssid, neigh_bssid, MAC_ADDR_STR_LEN);
	list_add_tail(&neigh->l, list);
	*item = neigh;
	return;
}

uint8_t cls_getself_candidates(const char *ifname, struct cls_neigh_report_info *neigh)
{
	int ret = -1;
	enum clsapi_freq_band band_info = clsapi_freq_band_unknown;
	clsapi_unsigned_int channel = 0;

	clsapi_wifi_get_bssid(ifname, neigh->bssid);
	neigh->opclass = 115;
	ret = clsapi_wifi_get_channel(ifname, &channel);
	if (ret)
		neigh->channel = 36;
	else
		neigh->channel = channel;
	neigh->phytype = PHY_TYPE_HT;
	band_info = cls_get_sigma_band_info_from_interface(ifname);
	if (band_info == clsapi_freq_band_5_ghz) {
		neigh->phytype = PHY_TYPE_VHT;
	}
	neigh->prefer = 255;
	cls_log("device info: ssid=%s, channel=%d", neigh->bssid, neigh->channel);
	return 0;
}

uint8_t cls_get_candidates(struct cls_neigh_report_info *neigh, struct list_head *candidates, uint8_t check_nonpref)
{
	struct cls_neigh_report_entry *item = NULL;
	uint8_t nums = 0;
	struct cls_neigh_report_info *report = NULL;

	if (!list_empty(candidates)) {
		list_for_each_entry(item, candidates, l) {
			item->neigh.non_pref_by_sta = 0; /* reset the station's non prefered of neighbor */
			if (check_nonpref) {
				if ((nonpref_info.opclass == item->neigh.opclass) && (nonpref_info.channel == item->neigh.channel))
					item->neigh.non_pref_by_sta = 1;
			}
			report = neigh + nums;
			*report = item->neigh;
			if (++nums >= CLS_NUMS_NEIGH_REPORT)
				break;
		}
	}
	return nums;
}

static int cls_mbo_send_btm_request(const char *ifname, char *dest_mac, int cand_list, int unsolicited)
{
	int ret = 0;
	char cmd[CLS_DEFCONF_CMDBUF_LEN] = {0};
	int i = 0;
	uint16_t reassoc_delay = 0;
	uint8_t pref = 0, check_nonpref = 0;
	char local_bssid[MAC_ADDR_STR_LEN] = {0};

	BUILD_MBO_CMD_HEAD("hostapd_cli");
	APPEND_CMD(cmd, sizeof(cmd), " -i %s bss_tm_req", ifname);
	cls_init_btm_req_info(&btm_req, DEAFULT_BEACON_INTERVAL);
	strncpy(btm_req.sta_mac, dest_mac, MAC_ADDR_STR_LEN);
	if (nonpref_info.valid && (0 == strncmp(nonpref_info.sta_mac, btm_req.sta_mac, MAC_ADDR_STR_LEN)))
		check_nonpref = 1;
	if (cand_list) {
		btm_req.nums_neigh = cls_get_candidates(btm_req.neighs, &neigh_list, check_nonpref);
		if (btm_req.nums_neigh == 0) {
			cls_getself_candidates(ifname, btm_req.neighs);
			btm_req.nums_neigh = 1;
		}
	}
	APPEND_CMD(cmd, sizeof(cmd), " %s", btm_req.sta_mac);
	/* disassoc_timer is only valid when disassoc imminent=1, according to IEEE80211-2020-Chap9.6.13.9 */
	/* if the timer is set, hostapd will deauth the station. This behavior will interrupt the MBO-TestCase-4.2.5.3 */
	if (btm_req.disassoc_imnt)
		APPEND_CMD(cmd, sizeof(cmd), " disassoc_timer=%d", btm_req.disassoc_timer);
	APPEND_CMD(cmd, sizeof(cmd), " valid_int=%d", btm_req.validity_interval);
	APPEND_CMD(cmd, sizeof(cmd), " disassoc_imminent=%d", btm_req.disassoc_imnt);
	if (cand_list)
		APPEND_CMD(cmd, sizeof(cmd), " pref=1");
	if (g_bss_term_params.include == 1) {
		APPEND_CMD(cmd, sizeof(cmd), " bss_term=%d,%d", g_bss_term_params.tsf, g_bss_term_params.duration);
	}
	clsapi_wifi_get_bssid(ifname, local_bssid);
	for (i = 0; i < btm_req.nums_neigh; i++) {
		APPEND_CMD(cmd, sizeof(cmd), " neighbor=%s,%d,%d,%d,%d", btm_req.neighs[i].bssid, btm_req.neighs[i].bssid_info,
			btm_req.neighs[i].opclass, btm_req.neighs[i].channel, btm_req.neighs[i].phytype);
		/* candidate preference subelement */
		pref = btm_req.neighs[i].prefer;
		if ((unsolicited && (0 == strncmp(local_bssid, btm_req.neighs[i].bssid, MAC_ADDR_STR_LEN)))
				|| btm_req.neighs[i].non_pref_by_sta) /* the pref of self bssid && non-prefered by station should be 0 */
			pref = 0;
		APPEND_CMD(cmd, sizeof(cmd), ",%02x%02x%02x", IEEE80211_BTMREQ_SUBELEMID_CANDI_PREF, 1, pref);
	}
	if (btm_req.disassoc_imnt)
		reassoc_delay = 30;
	APPEND_CMD(cmd, sizeof(cmd), " mbo=%u:%u:%u", btm_req.mbo_reason, reassoc_delay, btm_req.cell_pref);

	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to send btm request");

	if (g_bss_term_params.include == 1) {
		char cli_cmd[64] = {0};

		sprintf(cli_cmd, "hostapd_cli -i %s disassociate %s", ifname, btm_req.sta_mac);
		/* testbed sniffer only care the disassoc frame is found within TSF period, dont need exact timestamp */
		dut_register_timer(g_bss_term_params.tsf - 4, cli_cmd);
		memset(cli_cmd, 0, 64);
		sprintf(cli_cmd, "hostapd_cli -i %s disable", ifname);
		/* testbed sniffer will set GTE=ts of BTMreq + 5.5s, we should move the silient period forward as early as possible*/
		/* we shoule avoid GTE earlier than the silient period due to media access delay in noisy env */
		dut_register_timer(g_bss_term_params.tsf - 3, cli_cmd);
		memset(cli_cmd, 0, 64);
		sprintf(cli_cmd, "hostapd_cli -i %s enable", ifname);
		/* the silent period should be NOT less than 119s in testbed sniffer, GTE+119=LTE */
		dut_register_timer(g_bss_term_params.tsf + 60 * g_bss_term_params.duration, cli_cmd);
	}
	else { /* for BTM-req with disassoc_imnt=1 && bss_term params are not set */
		if (btm_req.disassoc_timer && btm_req.disassoc_imnt) {
			char cli_cmd[64] = {0};
			sprintf(cli_cmd, "hostapd_cli -i %s disassociate %s", ifname, btm_req.sta_mac);
			dut_register_timer(btm_req.disassoc_timer * 100 * 128 / (125 * 1000), cli_cmd); /* TU to second */
		}
	}
	cls_log("send cmd: %s\n", cmd);
	return ret;
}

enum {
	NON_PREF_STA_MAC,
	NON_PREF_ATTR_OPCLASS,
	NON_PREF_ATTR_CHANNEL,
	NON_PREF_ATTR_PREF,
	NON_PREF_ATTR_REASON,

	NUM_NON_PREF_ATTRS,
};

static const struct blobmsg_policy non_pref_policy[] = {
	[NON_PREF_STA_MAC] = { .name = "sta_mac", .type = BLOBMSG_TYPE_STRING },
	[NON_PREF_ATTR_OPCLASS] = { .name = "opclass", .type = BLOBMSG_TYPE_INT8 },
	[NON_PREF_ATTR_CHANNEL] = { .name = "channel", .type = BLOBMSG_TYPE_INT8 },
	[NON_PREF_ATTR_PREF] = { .name = "pref_value", .type = BLOBMSG_TYPE_INT8 },
	[NON_PREF_ATTR_REASON] = { .name = "reason", .type = BLOBMSG_TYPE_INT8 },
};

static int non_pref_handler(struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_NON_PREF_ATTRS];
	char *mac = NULL;
	int i;

	blobmsg_parse(non_pref_policy, NUM_NON_PREF_ATTRS, tb, blob_data(msg), blob_len(msg));
	if (!tb[NON_PREF_STA_MAC] || !tb[NON_PREF_ATTR_OPCLASS]
		|| !tb[NON_PREF_ATTR_CHANNEL]
		|| !tb[NON_PREF_ATTR_PREF]
		|| !tb[NON_PREF_ATTR_REASON]) {
		cls_error("key info is NOT included\n");
		return -1;
	}

	mac =  blobmsg_get_string(tb[NON_PREF_STA_MAC]);
	if (!nonpref_info.valid || (nonpref_info.valid && (0 == strncmp(nonpref_info.sta_mac, mac, MAC_ADDR_STR_LEN)))) {
		strncpy(nonpref_info.sta_mac, mac, MAC_ADDR_STR_LEN);
		nonpref_info.opclass = blobmsg_get_u8(tb[NON_PREF_ATTR_OPCLASS]);
		nonpref_info.channel = blobmsg_get_u8(tb[NON_PREF_ATTR_CHANNEL]);
		nonpref_info.pref = blobmsg_get_u8(tb[NON_PREF_ATTR_PREF]);
		nonpref_info.reason = blobmsg_get_u8(tb[NON_PREF_ATTR_REASON]);
		if (!nonpref_info.valid)
			nonpref_info.valid = 1;

		cls_log("sigma recv station's non-pref mac=%s, opclass=%d, channel=%d, pref=%d, reason=%d\n",
				nonpref_info.sta_mac, nonpref_info.opclass, nonpref_info.channel, nonpref_info.pref, nonpref_info.reason);
	}
	else
		cls_log("this non-pref is NOT for sigma\n");

	return 0;
}

enum {
	BTM_QUERY_ATTR_ADDRESS,
	BTM_QUERY_ATTR_DIALOG_TOKEN,
	BTM_QUERY_ATTR_REASON,
	BTM_QUERY_ATTR_CANDIDATE_LIST,
	NUM_BTM_QUERY_ATTRS,
};

static const struct blobmsg_policy btm_query_policy[] = {
	[BTM_QUERY_ATTR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
	[BTM_QUERY_ATTR_DIALOG_TOKEN] = { .name = "dialog-token", .type = BLOBMSG_TYPE_INT8 },
	[BTM_QUERY_ATTR_REASON] = { .name = "reason", .type = BLOBMSG_TYPE_INT8 },
	[BTM_QUERY_ATTR_CANDIDATE_LIST] = { .name = "candidate-list", .type = BLOBMSG_TYPE_STRING },
};

static int btm_query_handler(struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_BTM_QUERY_ATTRS];
	char *mac = NULL;
	uint8_t addr[MACLEN] ={0};
	uint8_t dialog = 0;
	uint8_t reason = 0;
	char *cand_list = NULL;

	blobmsg_parse(btm_query_policy, NUM_BTM_QUERY_ATTRS, tb, blob_data(msg), blob_len(msg));
	if (!tb[BTM_QUERY_ATTR_ADDRESS]
		|| !tb[BTM_QUERY_ATTR_DIALOG_TOKEN]
		|| !tb[BTM_QUERY_ATTR_REASON]) {
		cls_error("Parameters address, dialog_token, reason should be included\n");
		return -1;
	}
	mac = blobmsg_get_string(tb[BTM_QUERY_ATTR_ADDRESS]);
	ether_aton_r(mac, (struct ether_addr *)addr);
	dialog = blobmsg_get_u8(tb[BTM_QUERY_ATTR_DIALOG_TOKEN]);
	reason = blobmsg_get_u8(tb[BTM_QUERY_ATTR_REASON]);
	if (tb[BTM_QUERY_ATTR_CANDIDATE_LIST]) {
		cand_list = blobmsg_get_string(tb[BTM_QUERY_ATTR_CANDIDATE_LIST]);
		cls_log("cadidate list isn't NULL, cand_list=%s\n", cand_list);
	}
	cls_log("mac=%s, dialog=%d, reason=%d\n", mac, dialog, reason);
	cls_mbo_send_btm_request(cls_ubus_listen_ifname, mac, 1, 0);

	return 0;
}

static int mbo_info_handler(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	if (!strcmp(method, "bss-transition-query"))
		btm_query_handler(msg);
	else if (!strcmp(method, "sta_non_pref"))
		non_pref_handler( msg);
	else
		cls_log("Not registed, ignore\n");
	return 0;
}

void cls_clear_mbo_listen() {
	if (mbo_ubus_ctx) {
		ubus_free(mbo_ubus_ctx);
		mbo_ubus_ctx = NULL;
		uloop_done();
	}
	if (cls_ubus_notifcation.suscriber.cb != NULL) {
		cls_ubus_notifcation.suscriber.cb = NULL;
		cls_ubus_notifcation.id = 0;
	}
	memset(cls_ubus_listen_ifname, 0, sizeof(cls_ubus_listen_ifname));
	if (ubus_listen_thread == 0) {
		cls_log("No ubus listen thread");
		return;
	}
	if (pthread_cancel(ubus_listen_thread) != 0) {
		cls_error("Cancel ubus listen thread(%d) failed", ubus_listen_thread);
		return;
	}
	if (pthread_join(ubus_listen_thread, NULL) != 0) {
		cls_error("Join ubus listen thread(%d) failed", ubus_listen_thread);
		return;
	}
	cls_log("Ubus listen thread has finided");
	ubus_listen_thread = 0;
	return;
}

void *mbo_listen_hander(void *ifname)
{
	char *ifname_buf = NULL;
	char objname[32] = {0};
	int ret = -1;

	ifname_buf = (char *)ifname;

	uloop_init();
	mbo_ubus_ctx = ubus_connect(NULL);
	if (!mbo_ubus_ctx) {
		cls_error("Connect to ubus failed\n");
		return NULL;
	}
	ubus_add_uloop(mbo_ubus_ctx);
	cls_ubus_notifcation.suscriber.cb = mbo_info_handler;
	if (ifname_buf) {
		strncpy(cls_ubus_listen_ifname, ifname_buf, sizeof(cls_ubus_listen_ifname));
	}else {
		ifname_buf = (char *)cls_get_sigma_interface();
	}
	snprintf(objname, sizeof(objname), "hostapd.%s", ifname_buf);

	if ((ret = ubus_lookup_id(mbo_ubus_ctx, objname, &cls_ubus_notifcation.id))) {
		cls_error("Look up id failed on hostapd.wlanx!\n");
		goto fail;
	}
	if ((ret = ubus_register_subscriber(mbo_ubus_ctx, &cls_ubus_notifcation.suscriber))) {
		cls_error("Register subscriber failed\n");
		goto fail;
	}
	if ((ret = ubus_subscribe(mbo_ubus_ctx, &cls_ubus_notifcation.suscriber, cls_ubus_notifcation.id))) {
		cls_error("ubus subscribe failed\n");
		goto fail;
	}
	uloop_run();
	return NULL;
fail:
	ubus_free(mbo_ubus_ctx);
	mbo_ubus_ctx = NULL;
	cls_ubus_notifcation.suscriber.cb = NULL;
	cls_ubus_notifcation.id = 0;
	return NULL;
}

int cls_add_mbo_listen(char *ifname)
{
	if (mbo_ubus_ctx) {
		cls_log("Ubus is already connected\n");
		return 0;
	}
	if (pthread_create(&ubus_listen_thread, NULL, mbo_listen_hander, (void *)ifname)) {
		cls_error("Create thread failed for btm query ubus listen\n");
		return -1;
	}
	return 0;
}

uint32_t get_measreq_beacon_frame(struct cls_bcnreport_req *bcnreq_param, uint8_t **frm)
{
	uint32_t len = 0;
	uint8_t *buf = NULL;

	len = sizeof(struct ieee80211_ie_measreq_beacon) + 1 + 1 + 2 + 1 + 1 + 1; /*Beacon Reporting subelement and Reporting Detail subelement: tag number, length, value */

	if (!IS_EMPTY_PARAM(bcnreq_param->ssid)) {
		len += (1 + 1 + strlen(bcnreq_param->ssid));
	}
	if (bcnreq_param->num_chans) {
		len += (1 + 1 + 1 + bcnreq_param->num_chans);
	}
	if (bcnreq_param->nums_eid) {
		len += (1 + 1 + bcnreq_param->nums_eid);
	}
	if (bcnreq_param->last_beacon_rpt_ind) {
		len += 1 + 1 + 1;
	}
	buf = malloc(len);
	if (!buf)
		return 0;
	*frm = buf;
	return len;
}

int cls_mbo_send_bcnrep_request(char *ifname, struct cls_bcnreport_req *bcnreq_param)
{
	int i = 0;
	struct ieee80211_ie_measreq_beacon *beacon_req = NULL;
	uint8_t *frm = NULL;
	uint8_t *subelem = NULL;
	uint8_t frmlen = 0;
	uint8_t len = 0;
	char cmd[CLS_DEFCONF_CMDBUF_LEN] = {0};
	char *hex_str = NULL;

	BUILD_MBO_CMD_HEAD("hostapd_cli");
	APPEND_CMD(cmd, sizeof(cmd), " -i%s req_beacon", ifname);
	APPEND_CMD(cmd, sizeof(cmd), " %s", bcnreq_param->dest_mac);
	if (!IS_EMPTY_PARAM(bcnreq_param->chans)) {
		for (i = 0; i < strlen(bcnreq_param->chans); i++) {
			if (bcnreq_param->chans[i] == '_')
				bcnreq_param->chans[i] = ',';
		}
	}
	frmlen = get_measreq_beacon_frame(bcnreq_param, &frm);
	if (!frm) {
		cls_error("Get frame failed.");
		return -1;
	}
	beacon_req = (struct ieee80211_ie_measreq_beacon *)frm;
	beacon_req->opclass = bcnreq_param->opclass;
	beacon_req->chan = bcnreq_param->chan;
	beacon_req->interval = bcnreq_param->interval;
	beacon_req->duration = bcnreq_param->duration;
	if (strcasecmp(bcnreq_param->mode, "Passive") == 0)
		beacon_req->measure_mode = IEEE80211_BEACONREQ_MEASMODE_PASSIVE;
	else if (strcasecmp(bcnreq_param->mode, "Active") == 0)
		beacon_req->measure_mode = IEEE80211_BEACONREQ_MEASMODE_ACTIVE;
	else if (strcasecmp(bcnreq_param->mode, "Table") == 0)
		beacon_req->measure_mode = IEEE80211_BEACONREQ_MEASMODE_TABLE;
	APPEND_CMD(cmd, sizeof(cmd), " %02x%02x%04x%04x%02x", beacon_req->opclass, beacon_req->chan,
		htons(beacon_req->interval), htons(beacon_req->duration), beacon_req->measure_mode);
	cls_macstr_to_array(bcnreq_param->bssid, beacon_req->bssid);
	cls_data_to_hex((void *)beacon_req->bssid, IEEE80211_ADDR_LEN, &hex_str);
	APPEND_CMD(cmd, sizeof(cmd), "%s", hex_str);
	free(hex_str);

	subelem = beacon_req->data;
	/* Beacon Reporting subelement */
	*subelem++ = IEEE80211_BEACONREQ_SUBELEMID_BCNREPORTING;
	*subelem++ = 2;
	*subelem++ = 0;
	*subelem++ = 0;
	/* Reporting Detail subelement */
	*subelem++ = IEEE80211_BEACONREQ_SUBELEMID_DETAIL;
	*subelem++ = 1;
	*subelem++ = bcnreq_param->detail;
	/* Last beacon report indication request subelement */
	if (bcnreq_param->last_beacon_rpt_ind) {
		*subelem++ = IEEE80211_BEACONREQ_SUBELEMID_LASTBCN_INDI;
		*subelem++ = 1;
		*subelem++ = bcnreq_param->last_beacon_rpt_ind;
	}
	/* SSID subelement */
	if (!IS_EMPTY_PARAM(bcnreq_param->ssid)) {
		*subelem++ = IEEE80211_BEACONREQ_SUBELEMID_SSID;
		len = strlen(bcnreq_param->ssid);
		*subelem++ = len;
		memcpy(subelem, bcnreq_param->ssid, len);
		subelem += len;
	}
	if (bcnreq_param->nums_eid != 0) {
		*subelem++ = IEEE80211_BEACONREQ_SUBELEMID_REQUEST;
		*subelem++ = bcnreq_param->nums_eid;
		for (int i = 0; i < bcnreq_param->nums_eid; i++) {
			*subelem++ = bcnreq_param->eids[i];
		}
	}
	if (bcnreq_param->num_chans != 0) {
		*subelem++ = IEEE80211_BEACONREQ_SUBELEMID_CHAN_REPORT;
		*subelem++ = bcnreq_param->num_chans + 1;
		*subelem++ = bcnreq_param->opclass;
		for (int i = 0; i < bcnreq_param->num_chans; i++) {
			*subelem++ = bcnreq_param->chans[i];
		}
	}
	len = frmlen-sizeof(struct ieee80211_ie_measreq_beacon);
	cls_data_to_hex((void *)beacon_req->data, len, &hex_str);
	APPEND_CMD(cmd, sizeof(cmd), "%s", hex_str);
	free(hex_str);
	system(cmd);
	cls_log("send cmd: %s\n", cmd);

	free(frm);
	return 0;
}

void cls_handle_dev_send_frame(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	struct cls_bcnreport_req bcnreq_param;
	int status;
	int ret;
	char ifname_buf[IFNAMSIZ];
	char *ifname;
	char program[16];
	char tmpbuf[128];
	unsigned char dest_mac[IEEE80211_ADDR_LEN];
	char mac_str[MAC_ADDR_STR_LEN];
	char reqinfo[CLS_BCNRPT_STR_LEN];
	char *ptr = NULL;
	int value = 0;
	int cand_list;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));
	ifname = (ret > 0) ? ifname_buf : (char *)cls_get_sigma_interface();
	cls_get_sigma_primary_ifname_from_cmd(ifname_buf, ifname);

	ret = cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, program, sizeof(program));
	if (ret <= 0) {
		/* mandatory parameter */
		cls_error("can't get program");
		status = STATUS_ERROR;
		goto respond;
	}

	if (strcasecmp(program, "MBO") == 0 ||
		strcasecmp(program, "HE") == 0) {
		/* process FrameName */
		if (cls_get_value_text(&cmd_req, CLS_TOK_FRAMENAME,
			tmpbuf,	sizeof(tmpbuf)) <= 0) {
			cls_error("can't get frame_name");
			status = STATUS_ERROR;
			goto respond;
		}
		if (strcasecmp(tmpbuf, "BTMReq") == 0) {
			if (cls_get_value_text(&cmd_req, CLS_TOK_DEST_MAC,
				mac_str, sizeof(mac_str)) <= 0) {
				cls_error("can't get dest_mac");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_int(&cmd_req, CLS_TOK_CANDIDATE_LIST,
				&cand_list) <= 0) {
				cls_error("can't get candidate list");
				status = STATUS_ERROR;
				goto respond;
			}

			/* send BTM request */
			ret = cls_mbo_send_btm_request(ifname, mac_str, cand_list, 1);
			if (ret < 0) {
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(tmpbuf, "BcnRptReq") == 0) {
			memset(&bcnreq_param, 0, sizeof(bcnreq_param));
			if (cls_get_value_text(&cmd_req, CLS_TOK_DEST_MAC,
				bcnreq_param.dest_mac, sizeof(bcnreq_param.dest_mac)) <= 0) {
				cls_error("can't get dest_mac");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_int(&cmd_req, CLS_TOK_REGULATORY_CLASS,
				&bcnreq_param.opclass) <= 0) {
				cls_error("can't get regulatory class");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_int(&cmd_req, CLS_TOK_CHANNEL,
				&bcnreq_param.chan) <= 0) {
				cls_error("can't get channel number");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_int(&cmd_req, CLS_TOK_RAND_INTERVAL,
				&bcnreq_param.interval) <= 0) {
				cls_error("can't get randomization interval");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_int(&cmd_req, CLS_TOK_MEAS_DURATION,
				&bcnreq_param.duration) <= 0) {
				cls_error("can't get measurement duration");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_text(&cmd_req, CLS_TOK_MEAS_MODE,
				bcnreq_param.mode, sizeof(bcnreq_param.mode)) <= 0) {
				cls_error("can't get measurement mode");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_text(&cmd_req, CLS_TOK_BSSID,
				bcnreq_param.bssid, sizeof(bcnreq_param.bssid)) <= 0) {
				cls_error("can't get BSSID");
				status = STATUS_ERROR;
				goto respond;
			}
			if (cls_get_value_int(&cmd_req, CLS_TOK_REPORT_DETAIL,
				&bcnreq_param.detail) <= 0) {
				cls_error("can't get reporting detail");
				status = STATUS_ERROR;
				goto respond;
			}

			/* optional configuration type, ssid=Wi-Fi */
			if (cls_get_value_text(&cmd_req, CLS_TOK_SSID,
				bcnreq_param.ssid, sizeof(bcnreq_param.ssid)) > 0) {
				/* SSID Shouldn't be included for ZeroLength value */
				if (strcasecmp(bcnreq_param.ssid, "ZeroLength") == 0) {
					bcnreq_param.ssid[0] = 0;
					cls_log("SSID Parameter skipped for ZeroLength value.");
				}
			}
			/* optional configuration type, chans=36_48 */
			cls_get_value_text(&cmd_req, CLS_TOK_AP_CHAN_REPORT, reqinfo, sizeof(reqinfo));
			if (!IS_EMPTY_PARAM(reqinfo)) {
				ptr = reqinfo;
				bcnreq_param.num_chans = 0;
				do {
					value = strtoul(ptr, &ptr, 0);
					if (value < 0 || value > 255)
						continue;
					bcnreq_param.chans[bcnreq_param.num_chans++] = value;
				} while ('_' == *ptr++ && bcnreq_param.num_chans <= MAX_NUM_CLS_BCNRPT_CH);
				memset(reqinfo, 0, sizeof(reqinfo));
			}
			/* optional configuration type, info=0_221 */
			cls_get_value_text(&cmd_req, CLS_TOK_REQUEST_INFO, reqinfo, sizeof(reqinfo));
			if (!IS_EMPTY_PARAM(reqinfo)) {
				ptr = reqinfo;
				bcnreq_param.nums_eid = 0;
				do {
					value = strtoul(ptr, &ptr, 0);
					if (value < 0 || value > 255)
						continue;
					bcnreq_param.eids[bcnreq_param.nums_eid++] = value;
				} while ('_' == *ptr++ && bcnreq_param.nums_eid < NUMS_BEACON_REQUESTED_EID);
			}
			/* optional configuration type, LastBeaconRptIndication=1 */
			if (cls_get_value_int(&cmd_req, CLS_TOK_LAST_BEACON_REPORT_INDICATION,
				&bcnreq_param.last_beacon_rpt_ind) <= 0)
				bcnreq_param.last_beacon_rpt_ind = 0;

			/* send beacon report request */
			ret = cls_mbo_send_bcnrep_request(ifname, &bcnreq_param);
			if (ret < 0) {
				status = STATUS_ERROR;
				goto respond;
			}
		}
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

