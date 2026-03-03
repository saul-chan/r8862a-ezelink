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
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "common/csigma_common.h"
#include "common/csigma_log.h"
#include "common/csigma_tags.h"
#include "common/cls_cmd_parser.h"
#include "common/cls_dut_common.h"
#include "common/cls_defconf.h"
#include "common/cls_misc.h"
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "cls/clsapi.h"
//#include <cls/cls_pktgen.h>
#include <linux/wireless.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#include <net80211/ieee80211_qos.h>
#include <sys/ioctl.h>
#include "cls_dut_ap_handler.h"

/* this header should be included last */

#define N_ARRAY(arr)			(sizeof(arr)/sizeof(arr[0]))
#define IEEE80211_TXOP_TO_US(_txop)	(uint32_t)(_txop) << 5
#define FT_NAS1_ID "nas2.example.com"
#define FT_NAS2_ID "nas1.example.com"

int has_6g_configured_channel;
int has_5g_configured_channel;
int has_24g_configured_channel;
int g_numusersofdma;
int g_mbssid_set_num;
int ofdma_val = 1;
char phy_mode_buff[16];
extern struct list_head neigh_list;
extern struct cls_ap_btm_req btm_req;
extern struct cls_non_pref_entry nonpref_info;
char prog_name[16];
bool reset_11n_done = false;
char g_ofdma_defer_ifname[IFNAMSIZ] = {0};

static const char *promote_auth_for_pmf(const char *auth)
{
	if (strcasecmp(auth, "WPA-PSK") == 0)
		return "WPA-PSK-SHA256";

	if (strcasecmp(auth, "WPA-EAP") == 0)
		return "WPA-EAP-SHA256";

	return auth;
}

static int set_keymgnt(const char *if_name, const char *keymgnt, int pmf_required)
{
	int ret = 0;
	char *fname = "/mnt/jffs2/keymgnt_blacklist";
	char gCmdStr[128];
	int result;
	int i;
	struct cls_dut_config *conf = cls_dut_get_config(if_name);
	static const struct {
		const char *keymgnt;
		const char *beacon;
		const char *auth;
		const char *enc;
	} keymgnt_map[] = {
		{
			.keymgnt = "NONE", .beacon = "Basic",
			.auth = "WPA-PSK", .enc = "CCMP"},
		{
			.keymgnt = "WPA-PSK", .beacon = "WPA",
			.auth = "WPA-PSK", .enc = "TKIP"},
		{
			.keymgnt = "WPA-PSK-disabled", .beacon = "WPA",
			.auth = "WPA-PSK", .enc = "TKIP"},
		{
			.keymgnt = "WPA2-PSK", .beacon = "11i",
			.auth = "WPA-PSK", .enc = "CCMP"},
		{
			.keymgnt = "WPA-ENT", .beacon = "WPA",
			.auth = "WPA-EAP", .enc = "TKIP"},
		{
			.keymgnt = "WPA2-ENT", .beacon = "11i",
			.auth = "WPA-EAP", .enc = "CCMP"},
		{
			.keymgnt = "WPA2-PSK-Mixed", .beacon = "WPAand11i",
			.auth = "WPA-PSK", .enc = "\'CCMP TKIP\'"},
		{
			.keymgnt = "WPA2-Mixed", .beacon = "WPAand11i",
			.auth = "WPA-EAP", .enc = "\'CCMP TKIP\'"},
		{
			.keymgnt = "OSEN", .beacon = "Basic",
			.auth = "WPA-EAP", .enc = "CCMP"},
		{
			.keymgnt = "SAE", .beacon = "11i",
			.auth = "SAE", .enc = "CCMP"},
		{
			.keymgnt = "WPA2-PSK-SAE", .beacon = "11i",
			.auth = "\'WPA-PSK SAE\'", .enc = "CCMP"},
		{
			.keymgnt = "WPA2-PSK-SHA256-SAE", .beacon = "11i",
			.auth = "\'SAE WPA-PSK-SHA256\'", .enc = "CCMP"},
		{
			.keymgnt = "WPA2-ENT-Mixed", .beacon = "11i",
			.auth = "\'WPA-EAP WPA-EAP-SHA256\'", .enc = "CCMP"},
		{
			.keymgnt = "OWE", .beacon = "11i",
			.auth = "OWE", .enc = "CCMP"},
		{
			.keymgnt = "SUITEB", .beacon = "11i",
			.auth = "WPA-EAP-SUITE-B-192", .enc = "GCMP-256"},
		{
			NULL
		}
	};

	cls_log("%s: keymgnt = %s, pmf_required = %d\n", __func__, keymgnt, pmf_required);
	PRINT("%s:%d, %s: keymgnt = %s, pmf_required = %d\n",
		__FILE__, __LINE__, __func__, keymgnt, pmf_required);

	for (i = 0; keymgnt_map[i].keymgnt != NULL; ++i) {
		if (strcasecmp(keymgnt, keymgnt_map[i].keymgnt) == 0) {
			break;
		}
	}

	if (keymgnt_map[i].keymgnt == NULL) {
		return -EINVAL;
	}

	const char *auth = keymgnt_map[i].auth;

	if (strstr(keymgnt, "SAE") != NULL || strstr(keymgnt, "OWE") != NULL) {
		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s sae_require_mfp 1", if_name);
		PRINT("%s:%d, %s: cls set %s sae_require_mfp 1\n",
			__FILE__, __LINE__, __func__, if_name);
		ret = system(gCmdStr);
		if (ret != 0)
			return -ENODATA;

		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s sae_pwe 2", if_name);
		PRINT("%s:%d, %s: cls set %s sae_pwe 2\n",
			__FILE__, __LINE__, __func__, if_name);
		ret = system(gCmdStr);
		if (ret != 0)
			return -ENODATA;

		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s wps_cred_add_sae 1", if_name);
		PRINT("%s:%d, %s: cls set %s wps_cred_add_sae 1\n",
			__FILE__, __LINE__, __func__, if_name);
		ret = system(gCmdStr);
		if (ret != 0)
			return -ENODATA;
	}

	if (pmf_required) {
		/* WPA2 don't require SHA256, canceling promoting auth mode */
		//auth = promote_auth_for_pmf(auth);
		if ((result = clsapi_wifi_set_WPA_authentication_mode(if_name, auth)) < 0) {
			cls_error("can't set authentication to %s, error %d", auth, result);
			PRINT("%s:%d, %s: can't set authentication to %s, error %d\n",
				__FILE__, __LINE__, __func__, keymgnt, result);
			return result;
		}
		if ((result = clsapi_wifi_set_pmf(if_name, pmf_required)) < 0) {
			cls_error("can't set pmf to %s, error %d", auth, result);
			PRINT("can't set pmf to %s, error %d", auth, result);
			return result;
		}
	} else {
		result = clsapi_wifi_set_WPA_authentication_mode(if_name, auth);
		if (result < 0) {
			cls_error("can't set authentication to %s, error %d",
				keymgnt_map[i].auth, result);
			PRINT("%s:%d, %s: can't set authentication to %s, error %d\n",
				__FILE__, __LINE__, __func__, keymgnt_map[i].auth, result);
			return result;
		}
	}

	if ((result = clsapi_wifi_set_beacon_type(if_name, keymgnt_map[i].beacon)) < 0) {
		cls_error("can't set beacon_type to %s, error %d",
			keymgnt_map[i].beacon, result);
		PRINT("%s:%d, %s: can't set beacon_type to %s, error %d\n",
			__FILE__, __LINE__, __func__, keymgnt_map[i].beacon, result);
		return result;
	}

	if ((result = clsapi_wifi_set_WPA_encryption_modes(if_name, keymgnt_map[i].enc)) < 0) {
		cls_error("can't set encryption to %s, error %d",
			keymgnt_map[i].enc, result);
		PRINT("%s:%d, %s: can't set encryption to %s, error %d\n",
			__FILE__, __LINE__, __func__, keymgnt_map[i].enc, result);
		return result;
	}
	/*FIXME
	if ((conf && conf->testbed_enable)
		&& (strcasecmp(keymgnt, "WPA-PSK") == 0)) {
		char tmp[64];
		snprintf(tmp, sizeof(tmp), "iwpriv %s set_vht_tkip %d", if_name, 1);
		system(tmp);
	}
	*/
	if (strcasecmp(keymgnt, "OSEN") == 0) {
		result = clsapi_wifi_set_hs20_params(if_name,
				"osen", "1", "", "", "", "", "");
		if (result < 0) {
			cls_error("can't enable OSEN, error %d", result);
			PRINT("%s:%d, %s: can't enable OSEN, error %d\n",
				__FILE__, __LINE__, __func__, result);
			return result;
		}
		result = clsapi_wifi_set_hs20_params(if_name, "disable_dgaf",
				"1", "", "", "", "", "");
		if (result < 0) {
			cls_error("can't disable DGAF, error %d", result);
			PRINT("%s:%d, %s: can't disable DGAF, error %d\n",
				__FILE__, __LINE__, __func__, result);
			return result;
		}
	}

	return result;
}

static int set_ap_encryption(const char *if_name, const char *enc)
{
	int i;
	int result;
	struct cls_dut_config *conf = cls_dut_get_config(if_name);
	static const struct {
		const char *sigma_enc;
		const char *encryption;
	} map[] = {
		{
		.sigma_enc = "WEP",.encryption = "wep"}, {
		.sigma_enc = "TKIP",.encryption = "tkip"}, {
		.sigma_enc = "AES",.encryption = "ccmp"}, {
		NULL}
	};

	for (i = 0; map[i].sigma_enc != NULL; ++i) {
		if (strcasecmp(enc, map[i].sigma_enc) == 0) {
			break;
		}
	}

	if (map[i].sigma_enc == NULL) {
		return -EINVAL;
	}

	if ((conf && conf->testbed_enable)
		&& (strcasecmp(map[i].encryption, "WEP") == 0)) {
		char *beacon = "Basic";
		if ((result = clsapi_wifi_set_beacon_type(if_name, beacon)) < 0) {
			cls_error("can't set beacon_type to %s, error %d", beacon, result);
			PRINT("%s:%d, %s: can't set beacon_type to %s, error %d",
				__FILE__, __LINE__, __func__, beacon, result);
			return result;
		}
		return 0;
		//return clsapi_wifi_set_WEP_key(if_name, 0);
	}
	if (strcasecmp("WEP", map[i].sigma_enc) == 0)
		return 0;
	else 
		return clsapi_wifi_set_WPA_encryption_modes(if_name, map[i].encryption);
}

static int mbo_set_neighbor_bss(struct cls_neigh_report_entry *item, const char *ifname,
	int opclass, int opchan, int prefer)
{
	enum clsapi_freq_band band_info = clsapi_freq_band_unknown;

	item->neigh.opclass = opclass;
	item->neigh.channel = opchan;
	item->neigh.prefer = prefer;
	item->neigh.phytype = PHY_TYPE_HT;
	band_info = cls_get_sigma_band_info_from_interface(ifname);
	if (band_info == clsapi_freq_band_5_ghz)
		item->neigh.phytype = PHY_TYPE_VHT;
	return 0;
}

static int cls_set_hostapd_neighbor_bss(const char *ifname, struct cls_neigh_report_entry *item)
{
	char cmd[128] = {0};
	uint8_t bssid[MACLEN];
	char ssid[WFA_SSID_NAME_LEN] = {0};
	uint8_t *frm = NULL;
	uint8_t frmlen = 0;
	char *hex_str = NULL;
	int ret = -1;

	BUILD_MBO_CMD_HEAD("hostapd_cli");
	APPEND_CMD(cmd, sizeof(cmd), " -i %s set_neighbor", ifname);
	APPEND_CMD(cmd, sizeof(cmd), " %s", item->neigh.bssid);
	ret = clsapi_wifi_get_ssid(ifname, ssid);
	if (ret != 0) {
		cls_error("failed to get ssid");
	} else {
		APPEND_CMD(cmd, sizeof(cmd), " ssid=\\\"%s\\\"", ssid);
	}
	APPEND_CMD(cmd, sizeof(cmd), " nr=");
	/*
	 * Neighbor Report element size = BSSID + BSSID info + op_class + chan +
	 * phy type + wide bandwidth channel subelement.
	 * 6 + 4 + 1 + 1 + 1 + 5
	 */
	cls_macstr_to_array(item->neigh.bssid, bssid);
	cls_data_to_hex((void *)bssid, MACLEN, &hex_str);
	APPEND_CMD(cmd, sizeof(cmd), "%s", hex_str);
	free(hex_str);
	APPEND_CMD(cmd, sizeof(cmd), "%08x%02x%02x%02x", htonl(item->neigh.bssid_info), item->neigh.opclass,
		item->neigh.channel, item->neigh.phytype);

	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to send bcnrep request");
	cls_log("send cmd: %s\n", cmd);
	return ret;
}

static int mbo_set_btmreq_term_params(const char *ifname, int term_dur, int term_tsf)
{
	int ret = 0;
	char cmd[64];

	snprintf(cmd, sizeof(cmd), "iwpriv %s set_bss_dur %d", ifname, term_dur);
	ret = system(cmd);
	if (ret != 0)
		cls_error("set btmreq term duration(%d) failed", term_dur);

	snprintf(cmd, sizeof(cmd), "iwpriv %s set_btm_delay %d", ifname, term_tsf);
	ret = system(cmd);
	if (ret != 0)
		cls_error("set btmreq term delay(%d) failed", term_tsf);

	return ret;
}

static int mbo_set_assoc_disallow(const char *ifname, int assoc_disallow)
{
	int ret = 0;
	unsigned char macaddr[IEEE80211_ADDR_LEN];
	char macstr[64];
	char cmd[128];

	ret = clsapi_interface_get_mac_addr(ifname, macaddr);
	if (ret < 0) {
		cls_error("set assoc disallow: get macaddr failed");
		return ret;
	}
	snprintf(macstr, sizeof(macstr), "%02x:%02x:%02x:%02x:%02x:%02x",
			macaddr[0], macaddr[1], macaddr[2],
			macaddr[3], macaddr[4], macaddr[5]);

	//ret = cls_mbo_set_assoc_disallow(macstr, !assoc_disallow);

	return ret;
}

void cls_handle_ap_get_info(int len, unsigned char *params, int *out_len, unsigned char *out)
{
}

static int cls_handle_ap_set_radius_for_band(unsigned char *params,
				enum cls_dut_band_index band_index)
{
	struct cls_ap_set_radius ap_radius;
	int result = 0;
	const char *if_name = NULL;

	memcpy(&ap_radius, params, sizeof(ap_radius));

	/* the vap_index in some case is NOT set, we should use the root vap's ifname to write hostapd.conf */
	if (-1 == ap_radius.vap_index)
		if_name = cls_get_sigma_interface_for_band(band_index);
	else
		/* interface is optional, so it can be empty */
		if_name = ap_radius.if_name[0] == '\0' ? cls_get_sigma_vap_interface(ap_radius.vap_index, band_index):
					ap_radius.if_name;

	cls_log("Try to set radius: ip %s, port %d, pwd %s, ap_radius.if_name %s, if_name:%s",
		ap_radius.ip, ap_radius.port, ap_radius.password, ap_radius.if_name, if_name);
	PRINT("%s:%d, %s: set radius: ip %s, port %d, pwd %s, ap_radius.if_name %s, if_name:%s\n",
		__FILE__, __LINE__, __func__,
		ap_radius.ip, ap_radius.port, ap_radius.password, ap_radius.if_name, if_name);

	char port_str[16];
	snprintf(port_str, sizeof(port_str), "%d", ap_radius.port);

	result = clsapi_wifi_add_radius_auth_server_cfg(if_name, ap_radius.ip,
		port_str, ap_radius.password);

	if (result == -1)
		cls_error("Can't set radius ip, error %d", result);

	return result;

}

void cls_handle_ap_set_radius(int len, unsigned char *params, int *out_len, unsigned char *out)
{
	struct cls_dut_response rsp = {0};
	int result = 0;
	unsigned int band5g_idx;
	unsigned int band2g_idx;
	int cls_get_sigma_interface_band_idx(enum clsapi_freq_band);

	PRINT("%s:%d, %s: Entering AP_SET_RADIUS==========>\n", __FILE__, __LINE__, __func__);

	band5g_idx = cls_get_sigma_interface_band_idx(clsapi_freq_band_5_ghz);
	PRINT("%s:%d, %s: cls_get_sigma_interface_band_idx, band5g_idx: %u\n",
		__FILE__, __LINE__, __func__, band5g_idx);
	if(band5g_idx != CLS_DUT_BANDS_NUM) {
		result = cls_handle_ap_set_radius_for_band(params, band5g_idx);
		PRINT("%s:%d, %s: cls_handle_ap_set_radius_for_band, result: %d\n",
			__FILE__, __LINE__, __func__, result);
		if (result < 0)
			goto exit;
	}

	band2g_idx = cls_get_sigma_interface_band_idx(clsapi_freq_band_2pt4_ghz);
	PRINT("%s:%d, %s: cls_get_sigma_interface_band_idx, band5g_idx: %u\n",
		__FILE__, __LINE__, __func__, band2g_idx);
	if(band2g_idx != CLS_DUT_BANDS_NUM) {
		result = cls_handle_ap_set_radius_for_band(params, band2g_idx);
		PRINT("%s:%d, %s: cls_handle_ap_set_radius_for_band, result: %d\n",
			__FILE__, __LINE__, __func__, result);
		if (result < 0)
			goto exit;
	}

	exit:
		rsp.status = result == 0 ? STATUS_COMPLETE : STATUS_ERROR;
		rsp.clsapi_error = result;

		wfaEncodeTLV(CSIGMA_AP_SET_RADIUS_TAG, sizeof(rsp), (BYTE *) & rsp, out);
		*out_len = WFA_TLV_HDR_LEN + sizeof(rsp);
}

static void cls_dut_interface_print(int num_of_bands)
{
	cls_log("%s: current interfaces: ", __func__);
	for (int idx = 0; idx < num_of_bands; idx++)
		cls_log("[%s]", cls_get_sigma_interface_for_band(idx));
}

static const char *cls_dut_get_band_name(enum clsapi_freq_band band)
{
	if (band == clsapi_freq_band_6_ghz)
		return "6GHz";
	else if (band == clsapi_freq_band_5_ghz)
		return "5GHz";
	else if (band == clsapi_freq_band_2pt4_ghz)
		return "2.4GHz";

	return "unkonwn band"; 
}

static int is_tag_applied_on_same_interface(uint32_t vap_index)
{
	const char *last_tag_ifname;
	const char *cur_tag_ifname;

	last_tag_ifname = cls_get_sigma_tag_interface_map(vap_index - 1);
	PRINT("%s:%d, %s:cls_get_sigma_tag_interface_map: last_tag_ifname %s\n",
				__FILE__, __LINE__, __func__, last_tag_ifname);

	if (last_tag_ifname) {
		cur_tag_ifname = cls_get_sigma_tag_interface_map(vap_index);
		PRINT("%s:%d, %s:cls_get_sigma_tag_interface_map: cur_tag_ifname %s\n",
				__FILE__, __LINE__, __func__, cur_tag_ifname);

		if (cur_tag_ifname && 
			strcasecmp(last_tag_ifname, cur_tag_ifname) == 0)
		return 1;
	}

	return 0;
}

static int cls_create_vap(uint32_t vap_index, enum cls_dut_band_index band_index,
	 char *if_name_buf)
{
	int ret = 0;

	if(vap_index == 0)
		return 0;

	unsigned radio_id;
	ret = clsapi_get_radio_from_ifname(cls_get_sigma_interface_for_band(band_index), &radio_id);
	PRINT("%s:%d, %s: clsapi_get_radio_from_ifname: radio_id: %d, ret: %d\n",
				__FILE__, __LINE__, __func__,radio_id, ret);

	if (ret < 0) {
		cls_error("Failed to get radio id from interface name: ifname = %s",
			cls_get_sigma_interface_for_band(band_index));
		PRINT("%s:%d, %s: Failed to get radio id from interface name: ifname = %s\n",
				__FILE__, __LINE__, __func__,
				cls_get_sigma_interface_for_band(band_index));

		return ret;
	}

	char if_name[IFNAMSIZ] = {0};
	clsapi_unsigned_int bw;
	int wlan_id = radio_id + vap_index;

	while (wlan_id <= 2)
		wlan_id++;

	sprintf(if_name, "wlan%d", wlan_id);

	while (clsapi_wifi_get_bw(if_name, &bw) == 0) {
		sprintf(if_name, "wlan%u", ++wlan_id);
		PRINT("%s:%d, %s: ifname is %s, wlan_id is %d\n",
				__FILE__, __LINE__, __func__, if_name, wlan_id);
	}
	memcpy(if_name_buf, if_name, strlen(if_name));

	char status[32] = {0};
	ret = clsapi_interface_get_status(if_name_buf, status);
	if (ret > 0)
		return -1;

	uint8_t mac_addr[MACLEN];
	ret = clsapi_interface_get_mac_addr(cls_get_sigma_interface_for_band(band_index), mac_addr);
	PRINT("%s:%d, %s: clsapi_get_mac_addr: mac: %02x:%02x:%02x:%02x:%02x:%02x, ret: %d\n",
		__FILE__, __LINE__, __func__, mac_addr[0], mac_addr[1],
		mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], ret);

	if (ret < 0) {
		cls_error("Failed ot get primary interfaec mac address, ifname = %s",
			cls_get_sigma_interface_for_band(band_index));
		PRINT("%s:%d, %s: Failed ot get primary interfaec mac address, ifname = %s\n",
			__FILE__, __LINE__, __func__,
			cls_get_sigma_interface_for_band(band_index));

		return ret;
	}
	cls_set_sigma_vap_interface(if_name_buf, vap_index, band_index);
	MAC_ADDR_SET_LOCAL(mac_addr);
	mac_addr[0] |= (vap_index << 2);
		return clsapi_wifi_create_bss(if_name_buf, radio_id, mac_addr);
}

static const char *cls_ap_get_mbssid_ifname(const char *if_name, uint8_t offset)
{
	static char mbssid_ifname[128] = {0};
	char *sep;

	snprintf(mbssid_ifname, sizeof(mbssid_ifname), "%s", if_name);
	if_name = strchr(mbssid_ifname, '_' );

	if (sep != NULL) {
		sep++;

		snprintf(sep, sizeof(mbssid_ifname) - (sep - mbssid_ifname), "%d", offset);
		if_name = mbssid_ifname;

		cls_log("mbssid_ifname is [%s]\n", mbssid_ifname);
	}

	return if_name;
}

static int set_country_code(const char *ifname, const char *country_code)
{
	int ret;
	char region[16];

	if ((ret = clsapi_wifi_get_regulatory_region(ifname, region)) < 0) {
		cls_error("Can't get regulatory region, error %d", ret);
		return ret;
	}

	if (strcasecmp(region, country_code) != 0 &&
		(ret = clsapi_config_update_parameter(ifname, "region", country_code)) < 0) {
		cls_error("Can't update regulatoary region, error %d", ret);
		return ret;
	}

	return 0;
}

static int is_primary_ifname(const char *if_name, char *primary_ret, size_t maxlen)
{
	int ret;
	char primary_if_name[IFNAMSIZ];
	unsigned int radio_id;

	ret = clsapi_get_radio_from_ifname(if_name, &radio_id);
	PRINT("%s:%d, %s: clsapi_get_radio_from_ifname: radio_id: %d, ret: %d\n",
				__FILE__, __LINE__, __func__, radio_id, ret);

	if (ret < 0) {
		cls_error("Can't get radio_id, ifname %s, error %d",
			if_name, ret);
		PRINT("%s:%d, %s: Can't get radio_id, ifname %s, error %d\n",
				__FILE__, __LINE__, __func__, if_name, ret);

		return ret;
	}

	ret = clsapi_radio_get_primary_interface(radio_id, primary_if_name, sizeof(primary_if_name));
	if (ret < 0) {
		cls_error("Can't get primary interface, radio_id %d", radio_id);
		PRINT("%s:%d, %s: Can't get primary interface, radio_id %d\n",
				__FILE__, __LINE__, __func__,  radio_id);
		return ret;
	}

	if (primary_ret != NULL)
		strncpy(primary_ret, primary_if_name, maxlen);

	if (strcasecmp(if_name, primary_if_name) != 0) {
		cls_log("Skip %s. ifname %s is not primary %s", __func__, if_name, primary_if_name);
		PRINT("%s:%d, %s: Skip ifname %s is not primary %s\n",
				__FILE__, __LINE__, __func__,  if_name, primary_if_name);
		return -1;
	}

	return 0;
}

static int set_channel(const char *ifname, int channel, uint16_t bw,
			uint8_t band, const char *phy_mode)
{
	int ret = 0;
	char channel_str[16];

	ret = is_primary_ifname(ifname, NULL, 0);
	if (ret < 0) {
		cls_log("Skip %s. ifname %s is not primary", __func__, ifname);
		PRINT("%s:%d, %s: Skip %s. ifname %s is not primary\n",
			__FILE__, __LINE__, __func__, __func__, ifname);
		return 0;
	}

	snprintf(channel_str, sizeof(channel_str), "%d", channel);

	ret = clsapi_wifi_set_chan(ifname, channel, bw, band, phy_mode);
	PRINT("%s:%d, %s: clsapi_wifi_set_chan: ret: %d\n", __FILE__, __LINE__, __func__, ret);

	if (ret < 0) {
		cls_error("Can't set channel to %d, ifname %s, error %d",
			channel, ifname, ret);
		PRINT("%s:%d, %s: Can't set channel to %d, ifname %s, error %d\n",
			__FILE__, __LINE__, __func__, channel, ifname, ret);
	}
#if 0
	if (ret >= 0)
		ret = clsapi_config_update_parameter(ifname, "channel", channel_str);
	else
		cls_error("Can't set channel to %d, ifname %s, error %d",
			channel, ifname, ret);
#endif

	return ret;
}

static int safe_chan_switch(const char *ifname, int channel, uint16_t bw,
			uint8_t band, const char *phy_mode)
{
	int ret = 0;
	int attempts = 3;

	if (band == clsapi_freq_band_6_ghz)
		has_6g_configured_channel =1;
	else if (band == clsapi_freq_band_5_ghz)
		has_5g_configured_channel = 1;
	else
		has_24g_configured_channel = 1;

	ret = set_channel(ifname, channel, bw, band, phy_mode);
	PRINT("%s:%d, %s: set_channel: ret: %d\n", __FILE__, __LINE__, __func__, ret);

	while (ret == -EBUSY && attempts) {
		sleep(1);
		ret = set_channel(ifname, channel, bw, band, phy_mode);
		attempts --;
	}

	if (ret < 0) {
		cls_log("Reduce bw to 20MHz to be able to switch to channel %d", channel);
		PRINT("%s:%d, %s: Reduce bw to 20MHz to be able to switch to channel %d\n",
				__FILE__, __LINE__, __func__, channel);

		ret = set_channel(ifname, channel, clsapi_bw_20MHz, band, phy_mode);
		PRINT("%s:%d, %s: set_channel: ret: %d\n", __FILE__, __LINE__, __func__, ret);
	}

	return ret;
}

static void cls_dut_handle_unsolprbrsp_frame(const char *ifname, int enable, int period)
{
	char buf[128];

	if (ifname == NULL) {
		cls_error("Failed to start/stop uprobe");
		return;
	}

	if (enable == 1)
		snprintf(buf, sizeof(buf), "cdrvcmd pktgen %s set %u 1 %u %s",
			ifname, CLS_PKTGEN_TYPE_UPROBE_RESP,
			period, "ff:ff:ff:ff:ff:ff");
	else
		snprintf(buf, sizeof(buf), "cdrvcmd pktgen %s set %u, 0",
			ifname, CLS_PKTGEN_TYPE_UPROBE_RESP);
	system(buf);
	sleep(1);
}

static void cls_dut_handle_filsdscv_frame(const char *ifname, int enable)
{
	char buf[128];

	if (ifname == NULL) {
		cls_error("Failed to start/stop filsdiscv");
		return;
	}

	if (enable == 1)
		snprintf(buf, sizeof(buf), "cdrvcmd pktgen %s set %u 1 %u %s",
			ifname, CLS_PKTGEN_TYPE_FILS_DISCOVERY,
			CLS_PKTGEN_INTERVAL_DEFAULT_US, "ff:ff:ff:ff:ff:ff");
	else
		snprintf(buf, sizeof(buf), "cdrvcmd pktgen %s set %u, 0",
			ifname, CLS_PKTGEN_TYPE_FILS_DISCOVERY);
	system(buf);
	sleep(1);
}

static int set_phy_mode(const char *ifname, const char *mode)
{
	int ret;
	clsapi_unsigned_int old_bw;

	ret = is_primary_ifname(ifname, NULL, 0);
	PRINT("%s:%d, %s: is_primary_ifname: ret: %d\n",
				__FILE__, __LINE__, __func__, ret);

	if (ret < 0) {
		cls_log("Skip %s. ifname %s is not primary", __func__, ifname);
		PRINT("%s:%d, %s: Skip %s. ifname %s is not primary\n",
				__FILE__, __LINE__, __func__, __func__, ifname);
		return 0;
	}

	if (clsapi_wifi_get_bw(ifname, &old_bw) < 0) {
		if (!strcasecmp(mode, "11ac") || !strcasecmp(mode, "11ax"))
			old_bw = 80;
		else
			old_bw = 40;
	}

	ret = cls_set_phy_mode(ifname, mode);
	PRINT("%s:%d, %s: cls_set_phy_mode: ret %d\n", __FILE__, __LINE__, __func__, ret);

	/*restore old bandwidth */
	if (ret >= 0 && (!strcasecmp(mode, "11ax") || !strcasecmp(mode, "11ac")
		|| !strcasecmp(mode, "11ng") || !strcasecmp(mode, "11na"))) {
		if (clsapi_wifi_set_bw(ifname, old_bw, mode) < 0) {
			cls_error("failed to restore old bandwidth");
			PRINT("%s:%d, %s: failed to restore old bandwidth\n",
				__FILE__, __LINE__, __func__);
		}
	}

	return ret;
}

/* there is NO reset default cmd as the first cmd from UCC in WFA-11N-case , we can only do reset manually */
static void cls_ht_reset_default(void)
{
	char cmd[128] = {0};
	const char *ifname;
	unsigned int band_index;

	for (band_index = clsapi_freq_band_2pt4_ghz; band_index < clsapi_freq_band_unknown; band_index++) {
		if(NULL == (ifname = cls_get_sigma_interface_name(band_index)))
			continue;

		sprintf(cmd, "cls reset %s type dut program HT", ifname);
		system(cmd);
		if (0 == strcasecmp(ifname, "wlan1"))
			sleep(50);
		else
			sleep(10);
	}
}

static int cls_handle_ap_set_wireless_for_band(unsigned char *params,
			int band, enum cls_dut_band_index band_index)
{
	struct cls_ap_set_wireless cmd;
	int result = 0;
	int he_prog = 0;
	int vht_prog = 0;
	const char *phy_mode = NULL;
	const char *if_name;
	char primary_if_name[IFNAMSIZ] = {0};
	char if_name_buf[IFNAMSIZ] = {0};
	int check_tag_interface_status = 0;
	int enable;
	enum cls_dut_band_index band_idx;
	char nas_id[CLS_MAX_NAS_ID];
	char peer_nas_id[CLS_MAX_NAS_ID];
	uint8_t mbssid_offset = 0;
	struct cls_dut_config *dut_cfg = NULL;
	uint8_t prog_is_mbo = 0;

	cls_log("cls_handle_ap_set_wireless_for_band: band: %d band_ix: %d", band, band_index);
	PRINT("%s:%d, %s: band: %d band_ix: %d\n",
			__FILE__, __LINE__, __func__, band, band_index);

	memcpy(&cmd, params, sizeof(cmd));

	if (strcasecmp(cmd.programm, "HE") == 0)
		he_prog = 1;
	else if (strcasecmp(cmd.programm, "VHT") == 0)
		vht_prog = 1;

	if (dut_cfg = cls_dut_get_config(cls_get_sigma_interface_for_band(band_index))) {
		if (0 == strcasecmp(dut_cfg->cert_prog, "MBO"))
			prog_is_mbo = 1;
	}

	PRINT("%s:%d, %s: print vht_prog and he_prog: vht_prog: %d he_prog: %d\n",
			__FILE__, __LINE__, __func__, vht_prog, he_prog);
	PRINT("%s:%d, %s: %d, band=%d, chan=%d, bandwidth=%d\n",
			__FILE__, __LINE__, __func__, band,
			cmd.chan_band[band].band, cmd.chan_band[band].chan,
			cmd.bandwidth);

#if  0
	/*enable FILS/UPB on primary VAP*/
	if (cmd.unsolicitedproberesp == 1)
		cls_dut_handle_unsolprbrsp_frame(cls_get_sigma_interface_name(clsapi_freq_band_6_ghz), 1,
			(cmd.cadence_unsolicitedproberesp ?
			 IEEE80211_TU_TO_USEC(cmd.cadence_unsolicitedproberesp) :
			 CLS_PKTGEN_INTERVAL_DEFAULT_US));

	if (cmd.filsdscv == 1)
		cls_dut_handle_filsdscv_frame(cls_get_sigma_interface_name(clsapi_freq_band_6_ghz), 1);
#endif

	{ /* radio on/off is for dev, action on both radios */
	if (cmd.has_rf_enable && strcasecmp(prog_name, "MBO")) /* radio on/off in MBO is dummy */
		if (cmd.rf_enable) {
			system("hostapd_cli -i wlan0 enable");
			system("hostapd_cli -i wlan1 enable");
		}
		else {
			system("hostapd_cli -i wlan0 disable");
			system("hostapd_cli -i wlan1 disable");
		}
	}

	// Due to phy_mode need use mode but not frequence to do the setting
	if (*cmd.mode[band]) {
		if (strstr(cmd.mode[band], "11n"))
			phy_mode = "11n";
		else
			phy_mode = cmd.mode[band];
		strncpy(phy_mode_buff, phy_mode, sizeof(phy_mode_buff));

	} else if (cmd.chan_band[band].chan > 0) {
		if (cmd.chan_band[band].band == clsapi_freq_band_2pt4_ghz) {
			phy_mode = he_prog ? "11ax" : "11n";
		} else {
			phy_mode = he_prog ? "11ax" : "11ac";
		}
		strncpy(phy_mode_buff, phy_mode, sizeof(phy_mode_buff));
	}

	PRINT("%s:%d, %s: print phy_mode: phy_mode %s\n",
			__FILE__, __LINE__, __func__, phy_mode);

	if ((false == reset_11n_done) && !prog_is_mbo && (strstr(phy_mode_buff, "11n"))) {
		cls_ht_reset_default();
		reset_11n_done = true;
	}

	check_tag_interface_status = is_tag_applied_on_same_interface(cmd.vap_index + 1);
	PRINT("%s:%d, %s: is_tag_applied_on_same_interface: check_tag_interface_status: %d\n",
				__FILE__, __LINE__, __func__, check_tag_interface_status);

	if (cmd.vap_index  && check_tag_interface_status) {
		result = cls_create_vap(cmd.vap_index, band_index, if_name_buf);
		PRINT("%s:%d, %s: cls_create_vap: result %d, if_name_buf: %s\n",
				__FILE__, __LINE__, __func__, result, if_name_buf);
		if_name = if_name_buf;

		if (result <  0) {
			cls_error("failed to create vap, vap_index = %u", cmd.vap_index);
			PRINT("%s:%d, %s: failed to create vap, vap_index = %u\n",
				__FILE__, __LINE__, __func__, cmd.vap_index);
			goto exit;
		}
	} else {
		if_name = cls_get_sigma_interface_for_band(band_index);
		cls_set_sigma_vap_interface(if_name, cmd.vap_index, band_index);
		PRINT("%s:%d, %s: cls_get_sigma_interface_for_band: if_name: %s\n",
				__FILE__, __LINE__, __func__, if_name);
	}

#if 0
	const int rf_enable_timeout_sec = 5;
	if (cmd.has_rf_enable && (result = cls_set_rf_enable_timeout(cmd.rf_enable, rf_enable_timeout_sec)) < 0) {
		cls_error("can't set rf_enable to %d, error %d", cmd.rf_enable, result);
		goto exit;
	}
#endif

#if 0

	if (cmd.has_power_save &&
		(result =
			clsapi_pm_set_mode(cmd.power_save ? CLSAPI_PM_MODE_AUTO :
				//NEED command  CLSAPI_PM_MODE_DISABLE
				1)) < 0) {
		cls_error("can't set pm to %d, error %d", cmd.has_power_save, result);
		goto exit;
	}
#endif

/*
	if_name = cls_get_sigma_vap_interface(cmd.vap_index, band_index);
	PRINT("%s:%d, %s: cls_get_sigma_vap_interface: if_name: %s\n",
				__FILE__, __LINE__, __func__, if_name);
*/

#if 0
	if (cmd.has_mbssid) {
		char tmpbuf[128];
		const clsapi_mac_addr mac_addr = {0};
		char mbss_ifname[CLS_INTERFACE_LEN];
		unsigned int radio_id = 0;
		int i;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s 11v_mbssid %d", if_name, cmd.mbssid);
		system(tmpbuf);

		clsapi_get_radio_from_ifname(if_name, &radio_id);

		if (cmd.mbssid && cmd.chan_band[band].band ==clsapi_freq_band_2pt4_ghz) {
			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s 11axng_vht 0", if_name);
			system(tmpbuf);
		}

		if (cmd.has_numnontxbss) {
			for (i = 1; i <= cmd.numnontxbss; i++) {
				snprintf(mbss_ifname, sizeof(mbss_ifname), "wifi%d_%d", radio_id, i);
				clsapi_wifi_create_bss(mbss_ifname, radio_id, mac_addr);
			}
		}
	} else if (cmd.has_mmbssid) {
		char tmpbuf[128];
		const clsapi_mac_addr mac_addr = {0};
		char mbss_ifname[CLS_INTERFACE_LEN];
		unsigned int radio_id = 0;
		uint8_t start_idx = 1;
		int i;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s 11v_mbssid %d", if_name, cmd.mmbssid);
		system(tmpbuf);

		clsapi_get_radio_from_ifname(if_name, &radio_id);

		if (cmd.has_mbssidset && cmd.has_numnontxbss) {
			if (cmd.mbssidset == 1) {
				if (cmd.numnontxbss == 1) {
					g_mbssid_set_num = 4;
					snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mbssid_set 4", if_name);
					system(tmpbuf);
				} else if (cmd.numnontxbss > 1 && cmd.numnontxbss < 4) {
					g_mbssid_set_num = 2;
					snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mbssid_set 2", if_name);
					system(tmpbuf);
				} else {
					g_mbssid_set_num = 1;
					snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mbssid_set 1", if_name);
					system(tmpbuf);
				}
			} else if (cmd.mbssidset == 2) {
				snprintf(mbss_ifname, sizeof(mbss_ifname), "wifi%d_%d", radio_id, g_mbssid_set_num == 2 ? 4 : 2);
				clsapi_wifi_create_bss(mbss_ifname, radio_id, mac_addr);
			} else if (cmd.mbssidset == 3) {
				snprintf(mbss_ifname, sizeof(mbss_ifname), "wifi%d_%d", radio_id, 4);
				clsapi_wifi_create_bss(mbss_ifname, radio_id, mac_addr);
			} else if (cmd.mbssidset == 4) {
				snprintf(mbss_ifname, sizeof(mbss_ifname), "wifi%d_%d", radio_id, 6);
				clsapi_wifi_create_bss(mbss_ifname, radio_id, mac_addr);
			}

			if (cmd.mbssidset == 1) {
				start_idx = 1;
			} else if (cmd.mbssidset == 2) {
				start_idx = g_mbssid_set_num == 2 ? 5 : 3;
			} else if (cmd.mbssidset == 3) {
				start_idx = 5;
			} else if (cmd.mbssidset == 4) {
				start_idx = 7;
			}

			for (i = start_idx; i < start_idx + cmd.numnontxbss; i++) {
				snprintf(mbss_ifname, sizeof(mbss_ifname), "wifi%d_%d", radio_id, i);
				clsapi_wifi_create_bss(mbss_ifname, radio_id, mac_addr);
			}
		}
	}
#endif

#if 0
	if (cmd.has_nontxbssindex)
		mbssid_offset += cmd.nontxbssindex;
#endif

#if 0
	if (cmd.has_mbssidset) {
		if (cmd.mbssidset == 2)
			mbssid_offset += g_mbssid_set_num == 2 ? 4 : 2;
		else if (cmd.mbssidset == 3)
			mbssid_offset += 4;
		else if (cmd.mbssidset == 4)
			mbssid_offset += 6;
	}
#endif

#if 0
	if (mbssid_offset)
		if_name = cls_ap_get_mbssid_ifname(if_name, mbssid_offset);
#endif

#if 0
	/*enable FILS/UPB on other non-primary trans-VAPs*/
	if (cmd.has_mmbssid && cmd.mmbssid
			&& cmd.has_mbssidset && cmd.mbssidset != 1
			&& !cmd.has_nontxbssindex) {
		if (cmd.unsolicitedproberesp == 1)
			cls_dut_handle_unsolprbrsp_frame(if_name, 1,
					(cmd.cadence_unsolicitedproberesp ?
					 IEEE80211_TU_TO_USEC(cmd.cadence_unsolicitedproberesp) :
					 CLS_PKTGEN_INTERVAL_DEFAULT_US));

		if (cmd.filsdscv == 1)
			cls_dut_handle_filsdscv_frame(if_name, 1);
	}
#endif

#if 0
	if (cmd.has_mbssid_profilelen) {
		char tmpbuf[128];
		uint32_t prof_len_value;

		prof_len_value = 0x80 << 24 |
				cmd.mbssid_profilelen_min << 12 | cmd.mbssid_profilelen_max;
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mbssid_set 0X%08x", if_name,
				prof_len_value);
		system(tmpbuf);
	}
#endif

	if (is_primary_ifname(if_name, primary_if_name, sizeof(primary_if_name)) < 0) {
		cls_log("ifname %s is not primary, using %s for cls_dut_config",
				if_name, primary_if_name);
		PRINT("%s:%d, %s: ifname %s is not primary, using %s for cls_dut_config\n",
				__FILE__, __LINE__, __func__, if_name, primary_if_name);
	}

	struct cls_dut_config *conf = cls_dut_get_config(primary_if_name);
	band_idx = cls_get_sigma_band_info_from_interface(if_name);
	PRINT("%s:%d, %s: cls_get_sigma_band_info_from_interface: bandidx: %d\n",
				__FILE__, __LINE__, __func__, band_idx);

	if (phy_mode &&
	    (result = set_phy_mode(if_name, phy_mode)) < 0) {
		cls_error("can't set phy_mode to %s, error %d", phy_mode, result);
		PRINT("%s:%d, %s: can't set phy_mode to %s, error %d\n",
				__FILE__, __LINE__, __func__, phy_mode, result);
		goto exit;
	}

	if (cmd.ssid[0]) {
		result = clsapi_wifi_set_SSID(if_name, cmd.ssid);
		PRINT("%s:%d, %s: clsapi_wifi_set_SSID: result: %d\n",
				__FILE__, __LINE__, __func__, result);

		unsigned int radio_id = 0;

		clsapi_get_radio_from_ifname(if_name, &radio_id);
		if (result < 0 && cmd.has_nontxbssindex) {
			result = clsapi_wifi_create_bss(if_name, radio_id, NULL);
			PRINT("%s:%d, %s: clsapi_wifi_create_bss: result: %d\n",
				__FILE__, __LINE__, __func__, result);

			if (result >= 0)
				result = clsapi_wifi_set_SSID(if_name, cmd.ssid);
				PRINT("%s:%d, %s: clsapi_wifi_set_SSID: result: %d\n",
				__FILE__, __LINE__, __func__, result);
		}

		if (result < 0) {
			cls_error("can't set ssid to %s, error %d", cmd.ssid, result);
			PRINT("%s:%d, %s: can't set ssid to %s, error %d\n",
				__FILE__, __LINE__, __func__, cmd.ssid, result);
			goto exit;
		}
	}

#if 0
	if (strstr(cmd.ssid, "HE-4.56.1") || strstr(cmd.ssid, "HE-4.56.2")) {
		/*
		 * BRCM STA can send delba after wakeup if it recives out of order frames.
		 * enable reserve_all_slots to prevent reordering on TX side.
		 */
		char tmpbuf[128];
		unsigned int radioid;

		clsapi_get_radio_from_ifname(if_name, &radioid);
		need_clear_reserve_all_slots = 1;
		//TODO need command 
		snprintf(tmpbuf, sizeof(tmpbuf), "mu %d dbg_flg set reserve_all_slots",
									radioid);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.country_code[0] && (result = set_country_code(if_name, cmd.country_code)) < 0) {
		cls_error("can't set country code to %s, error %d", cmd.country_code, result);
		goto exit;
	}
#endif

	if (cmd.chan_band[band].chan >= 0) {
		int bw_cap = 0;

		if (cmd.has_bandwidth) {
			bw_cap = cmd.bandwidth;
			if (bw_cap == 0)
				bw_cap = he_prog || vht_prog ? clsapi_bw_80MHz : clsapi_bw_40MHz;
		}

		if (phy_mode == NULL) {
			if (he_prog == 1)
				phy_mode = "11ax";
			if (vht_prog == 1) {
				unsigned int radio_id = 0;

				clsapi_get_radio_from_ifname(if_name, &radio_id);
				if (radio_id == 0)
					phy_mode = "11n";
				if (radio_id == 1)
					phy_mode = "11ac";
			} else {
				phy_mode = phy_mode_buff;
			}
		}
		PRINT("%s:%d, %s: phymode: %s, phy_mode_buf: %s\n",
				__FILE__, __LINE__, __func__, phy_mode, phy_mode_buff);

		if (!cmd.chan_band[band].chan && cmd.has_bandwidth) {
			result = clsapi_wifi_set_bw(if_name, bw_cap, phy_mode);
			PRINT("%s:%d, %s: clsapi_wifi_set_bw: result: %d\n",
				__FILE__, __LINE__, __func__, result);

			if (result < 0) {
				cls_error("can't set bw %d, error %d",
					bw_cap, result);
				PRINT("%s:%d, %s: can't set bw %d, error %d\n",
				__FILE__, __LINE__, __func__, bw_cap, result);

				goto exit;
			}
		} else if (cmd.chan_band[band].chan) {
			result = safe_chan_switch(if_name, cmd.chan_band[band].chan, bw_cap,
							cmd.chan_band[band].band, phy_mode);
			PRINT("%s:%d, %s: safe_chan_switch: result %d\n",
				__FILE__, __LINE__, __func__, result);

			if (result < 0) {
				cls_error("can't set channel %d, error %d",
					cmd.chan_band[band].chan, result);
				PRINT("%s:%d, %s: can't set channel %d, error %d\n",
				__FILE__, __LINE__, __func__, cmd.chan_band[band].chan, result);

				goto exit;
			}
		}
	}

#if 0
	if (cmd.has_wmm && (result = clsapi_wifi_set_option(if_name, clsapi_wmm, cmd.wmm)) < 0) {
		cls_error("can't set wmm to %d, error %d", cmd.wmm, result);
		goto exit;
	}
#endif

#if 0
	if (cmd.has_muedca) {
		char tmpbuf[128];

		/* Make sure that MU EDCA IE is in beacons */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_edca %d", if_name, cmd.muedca);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_bctwt) {
		char tmpbuf[128];

		/* If BC TWT needs to be enabled, then disable it and enable it again */
		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", if_name);
		system(tmpbuf);

		if (cmd.bctwt) {
			snprintf(tmpbuf, sizeof(tmpbuf), "twt %s enable 1", if_name);
			system(tmpbuf);

			snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 1", if_name);
			system(tmpbuf);

			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_edca 1", if_name);
			system(tmpbuf);
		}
	}
#endif

#if 0
	if (cmd.has_noackpolicy) {
		const int stream_classes[] = { WME_AC_BE, WME_AC_BK, WME_AC_VI, WME_AC_VO };
		for (int i = 0; i < N_ARRAY(stream_classes); ++i) {
			result = clsapi_wifi_qos_set_param(if_name,
								stream_classes[i],
								IEEE80211_WMMPARAMS_NOACKPOLICY,
								0,
								cmd.noackpolicy);
			if (result < 0) {
				cls_error("can't set noackpolicy to %d, error %d",
						cmd.noackpolicy,
						result);
				goto exit;
			}
		}

	}
#endif

#if 0

	if (cmd.has_apsd && (result = clsapi_wifi_set_option(if_name, clsapi_uapsd, cmd.apsd)) < 0) {
		cls_error("can't set apsd to %d, error %d", cmd.apsd, result);
		goto exit;
	}
#endif

	if (cmd.has_rts_threshold
		&& (result = clsapi_wifi_set_rts_threshold(if_name, cmd.rts_threshold)) < 0) {
		cls_error("can't set rts_threshold to %d, error %d", cmd.rts_threshold, result);
		PRINT("%s:%d, %s: can't set rts_threshold to %d, error %d\n",
				__FILE__, __LINE__, __func__, cmd.rts_threshold, result);
		goto exit;
	}

	if (cmd.has_beacon_interval &&
		(result = clsapi_wifi_set_beacon_interval(if_name, cmd.beacon_interval)) < 0) {
		cls_error("can't set beacon_interval to %d, error %d", cmd.beacon_interval, result);
		PRINT("%s:%d, %s: can't set beacon_interval to %d, error %d\n",
				__FILE__, __LINE__, __func__, cmd.beacon_interval, result);
		goto exit;
	}

	if (cmd.has_amsdu && (result = cls_set_amsdu(if_name, cmd.amsdu)) < 0) {
		cls_error("can't set amsdu to %d, error %d", cmd.amsdu, result);
		PRINT("%s:%d, %s: can't set amsdu to %d, error %d\n",
				__FILE__, __LINE__, __func__, cmd.amsdu, result);
		goto exit;
	}

	if (cmd.has_mcs_rate &&
		(result = cls_set_fixed_mcs_rate(if_name, cmd.mcs_rate, phy_mode)) < 0) {
		cls_error("can't set mcs rate to MCS%d, error %d", cmd.mcs_rate, result);
		PRINT("%s:%d, %s: can't set mcs rate to MCS%d, error %d\n",
				__FILE__, __LINE__, __func__, cmd.mcs_rate, result);
		goto exit;
	}

	/* looks like we don't have API to setup NSS separatly for RX and TX */
	int nss_rx;
	int nss_tx;

	if (cmd.nss_rx[0] && cmd.nss_tx[0] && sscanf(cmd.nss_rx, "%d", &nss_rx) == 1 &&
		sscanf(cmd.nss_tx, "%d", &nss_tx) == 1) {
		if (nss_rx != nss_tx) {
			cls_error("can't set different nss for rx %d and tx %d", nss_rx, nss_tx);
			PRINT("%s:%d, %s: can't set different nss for rx %d and tx %d\n",
				__FILE__, __LINE__, __func__, nss_rx, nss_tx);
			result = -EINVAL;
			goto exit;
		} else {
			char tmpbuf[64];

			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x02 0x07 0x%02x", if_name, nss_tx);
			result = system(tmpbuf);
			PRINT("%s:%d, %s: Set tx spatial stream: %s\n",
						__FILE__, __LINE__, __func__, tmpbuf);
			if (result != 0) {
				cls_error(
					"Set %s tx spatial stream failed. nss_tx value: %d, result: %d",
					if_name, nss_tx, result);
				PRINT(
					"%s:%d, %s: Set %s tx spatial stream failed. nss_tx value: %d, result: %d\n",
					__FILE__, __LINE__, __func__, if_name, nss_tx, result);
				goto exit;
			}
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x02 0x06 0x%02x",
				if_name, nss_rx);
			result = system(tmpbuf);
			PRINT("%s:%d, %s: Set rx spatial stream: %s\n",
						__FILE__, __LINE__, __func__, tmpbuf);
			if (result != 0) {
				cls_error(
					"Set %s rx spatial stream failed. nss_rx value: %d, result: %d",
					if_name, nss_rx, result);
				PRINT(
					"%s:%d, %s: Set %s rx spatial stream failed. nss_rx value: %d, result: %d\n",
					__FILE__, __LINE__, __func__, if_name, nss_rx, result);
				goto exit;
			}
		}
	}

#if 0
	if (cmd.has_nss_mcs_cap &&
		(result = cls_set_nss_mcs_cap(if_name, cmd.nss_cap, 0, cmd.mcs_high_cap) < 0)) {
		cls_error("can't set nss_mcs capabilities to nss %d;0-%d, error %d",
				cmd.nss_cap, cmd.mcs_high_cap, result);
		goto exit;
	}
#endif

#if 0
	if (cmd.has_dtim && (result = clsapi_wifi_set_dtim(if_name, cmd.dtim)) < 0) {
		cls_error("can't set dtim to %d, error %d", cmd.dtim, result);
		goto exit;
	}
#endif

	if (cmd.has_short_gi) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x02 0x0d 0x%02x",
				if_name, cmd.short_gi);
		result = system(tmpbuf);
		PRINT("%s:%d, %s: set_sgi: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
		if (result != 0) {
			cls_error("can't set short_gi to %d, error %d", cmd.short_gi, result);
			PRINT("%s:%d, %s: can't set short_gi to %d, error %d\n",
					__FILE__, __LINE__, __func__, cmd.short_gi, result);
			goto exit;
		}
	}

#if 0
	if (cmd.has_su_beamformer) {
		result = clsapi_wifi_set_option(if_name, clsapi_beamforming, cmd.su_beamformer);
		if (result < 0) {
			cls_error("can't enable beamforming, error %d", result);
			result = 0;
		} else {
			char tmpbuf[64];

			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s su_bmfr %d", if_name,
									cmd.su_beamformer);
			system(tmpbuf);
		}

		/* we don't apply BF matrix if STBC is enabled, so disable it */
		if (cmd.su_beamformer && !cmd.has_stbc_tx) {
			result = clsapi_wifi_set_option(if_name, clsapi_stbc, 0);
			if (result < 0) {
				cls_error("can't disable STBC, error %d", result);
				result = 0;
			}
		}
	}
#endif

	if (cmd.has_ppdutxtype && cmd.ppdutxtype == CLS_CA_PPDUTXTYPE_MU) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf),
			"iw dev %s vendor send 0xD04433 0x01 0x03 0x%02x", if_name, 1);
		result = system(tmpbuf);
		PRINT("%s:%d, %s: %s: result: %d\n",
			__FILE__, __LINE__, __func__, tmpbuf, result);

		if (result != 0) {
			cls_error("Set ppdu_tx_type MU failed, error %d", result);
			PRINT("%s:%d, %s: Set ppdu_tx_type MU failed, error %d\n",
				__FILE__, __LINE__, __func__, result);
		}
	}

#if 0
	if (cmd.has_stbc_tx) {
		result = clsapi_wifi_set_option(if_name, clsapi_stbc, 1);
		if (result < 0) {
			cls_error("can't enable STBC, error %d", result);
			result = 0;
		}

		char tmpbuf[128];
		snprintf(tmpbuf, sizeof(tmpbuf), "set_11ac_mcs %s 0x05", if_name);
		system(tmpbuf);
	}
#endif

	if (cmd.has_ldpc) {
		char tmpbuf[128];

		snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s vendor send 0xD04433 0x02 0x02 0x%02x",
			if_name, cmd.ldpc);
		PRINT("%s:%d, %s: iw dev %s vendor send 0xD04433 0x02 0x02 0x%02x\n",
				__FILE__, __LINE__, __func__, if_name, cmd.ldpc);
		result = system(tmpbuf);

		if (result != 0) {
			cls_error("Set ldpc failed, result: %d", result);
			PRINT("%s:%d, %s: Set ldpc failed, result: %d\n",
				__FILE__, __LINE__, __func__, result);
			goto exit;
		}

		if (cmd.ldpc == 0) {
			snprintf(tmpbuf, sizeof(tmpbuf), "cls set %s payload_code bcc", if_name);
			PRINT("%s:%d, %s: cls set %s payload_code bcc\n",
				__FILE__, __LINE__, __func__, if_name);
			result = system(tmpbuf);
			if (result != 0) {
				cls_error("Set ldpc command failed, result: %d", result);
				PRINT("%s:%d, %s: Set ldpc command failed, result: %d\n",
					__FILE__, __LINE__, __func__, result);
				goto exit;
			}
		}
	}

	if (cmd.has_addba_reject) {
		result = clsapi_wifi_set_rxba_decline(if_name, cmd.addba_reject);
		if (result != 0) {
			cls_error("Set addba_reject failed, result: %d", result);
			PRINT("%s:%d, %s: Set addba_reject failed, result: %d\n",
				__FILE__, __LINE__, __func__, result);
			goto exit;
		}
	}

#if 0
	if (cmd.has_ampdu) {
		cls_set_ampdu(if_name, cmd.ampdu);
	}
#endif


	if (conf && (strcasecmp(conf->cert_prog, "wpa3") == 0)) {
		/* WAR: TXBA should be disabled by default to prevent ping failure with QCOM STA */
		clsapi_wifi_set_txba_disable(if_name, 1);
	}

	if (cmd.has_offset) {
		char tmpbuf[128];
		char position[8] = {0};
	/* vendor SUBCMD---VENDOR_GENERIC_SECBAND_OFFSET is not valid. */
	/* USE hostapd to config the sideband */
		if (2 == cmd.offset)
			snprintf(position, sizeof(position), "above");
		else
			snprintf(position, sizeof(position), "below");

		snprintf(tmpbuf, sizeof(tmpbuf), "cls set %s sideband %s", if_name, position);

		PRINT("%s:%d, %s: set offset: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
		result = system(tmpbuf);

		if (result != 0) {
			cls_error("Set offset failed, result: %d", result);
			PRINT("%s:%d, %s: Set offset failed, result: %d\n",
				__FILE__, __LINE__, __func__, result);
			goto exit;
		}
	}

#if 0
	if (cmd.has_dyn_bw_sgnl) {
		if (conf) {
			conf->bws_dynamic = (unsigned char)cmd.dyn_bw_sgnl;
			conf->update_settings = 1;
		} else {
			result = -EFAULT;
			goto exit;
		}
	}
#endif

#if 0
	if (cmd.has_vht_tkip) {
		char tmp[64];

		snprintf(tmp, sizeof(tmp), "iwpriv %s set_vht_tkip %d", if_name, cmd.vht_tkip);
		system(tmp);
	}
#endif

	if (cmd.has_bw_sgnl) {
		char tmpbuf[128];

		snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s vendor send 0xD04433 0x02 0x0c 0x%02x",
			if_name, cmd.bw_sgnl);
		PRINT("%s:%d, %s: set bw_sgnl: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
		result = system(tmpbuf);

		if (result != 0) {
			cls_error("Set bw_sgnl failed, result: %d", result);
			PRINT("%s:%d, %s: Set bw_sgnl failed, result: %d\n",
				__FILE__, __LINE__, __func__, result);
			goto exit;
		}
	}

#if 0
	if (cmd.has_mu_ndpa_format) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_mu_non_ht %d",
				if_name, cmd.mu_ndpa_format);
		system(tmpbuf);
	}
#endif

	if (cmd.has_group_id) {
		/* TODO: implement */
	}

#if 0

	if (cmd.has_rts_force) {
		if (conf) {
			conf->force_rts = (unsigned char)cmd.rts_force;
			conf->update_settings = 1;
		} else {
			result = -EFAULT;
			goto exit;
		}
	}

	if (cmd.has_maxhe_mcs_1ss_rxmaplte80) {
		cls_set_mcs_cap(if_name, cmd.maxhe_mcs_1ss_rxmaplte80);
	}

	if (cmd.has_maxhe_mcs_2ss_rxmaplte80) {
		cls_set_mcs_cap(if_name, cmd.maxhe_mcs_2ss_rxmaplte80);
	}
#endif

	if (cmd.has_ofdma || cmd.has_mimo) {
		conf->use_ul_mumimo = 0;
	}

	if (cmd.has_numusersofdma) {
		g_numusersofdma = cmd.numusersofdma;
	}

	if (cmd.has_ofdma) { /* ofdma params shold be set after hostapd reconf, because there're runtime params of driver */
		ofdma_val = cmd.ofdma;
		if (cmd.ofdma == 0 || cmd.ofdma ==1 || cmd.ofdma == 2) {
				strncpy(g_ofdma_defer_ifname, if_name, sizeof(g_ofdma_defer_ifname));
			if (!cmd.has_numusersofdma) {
				/* in some test cases number of users is not specified. Need to set
				 * some default value.
				 */
				g_numusersofdma = 4;
			}
		}

	} else {
		ofdma_val = 1;
	}

#if 0
	if (cmd.has_numsounddim) {
		char tmpbuf[64];

		if (he_prog) {
			/* WAR for script issue. They can use 3 and 4 to set numsounddim to 4.
			 * Same for 7 and 8.
			 */

			if (cmd.numsounddim == 7)
				cmd.numsounddim = 8;
			else if (cmd.numsounddim == 3)
				cmd.numsounddim = 4;
		}

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_snd_dim %d",
								if_name, cmd.numsounddim - 1);
		system(tmpbuf);

		cls_dut_backup_num_snd_dim(cmd.numsounddim - 1, band_idx);

		if (cmd.numsounddim == 8) {
			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_8sts_snd 1", if_name);
			system(tmpbuf);
		}
	}
#endif

#if 0
	if (cmd.has_ltf_gi) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_ndp_gi_ltf %d",
								if_name, cmd.ltf_gi);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_rualloctones) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_ru_size %s %d", if_name,
					cmd.rualloctones[0] == 26 ? CLS_HE_RU_SIZE_26 :
					cmd.rualloctones[0] == 52 ? CLS_HE_RU_SIZE_52 :
					cmd.rualloctones[0] == 106 ? CLS_HE_RU_SIZE_106 :
					cmd.rualloctones[0] == 242 ? CLS_HE_RU_SIZE_242 :
					cmd.rualloctones[0] == 484 ? CLS_HE_RU_SIZE_484 : 255);
		system(tmpbuf);

		snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_size %s %d",
								if_name, cmd.rualloctones_ctr);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_he_txopdurrtsthr) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s txop_rtsthr %d", if_name,
			cmd.he_txopdurrtsthr ? 512 : IEEE80211_IE_HEOP_RTS_THRESHOLD_DISABLED);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_minmpdustartspacing) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_ampdu_dens %d", if_name,
									cmd.minmpdustartspacing);
		system(tmpbuf);
	}
#endif

#if 0
	if (conf && conf->update_settings) {
		cls_set_rts_settings(if_name, conf);
	}
#endif

	if (strcasecmp(cmd.Name, "ap1mbo") == 0) {
		memcpy(nas_id, FT_NAS1_ID, sizeof(FT_NAS1_ID));
		memcpy(peer_nas_id, FT_NAS2_ID, sizeof(FT_NAS2_ID));
	} else if (strcasecmp(cmd.Name, "ap2mbo") == 0) {
		memcpy(nas_id, FT_NAS2_ID, sizeof(FT_NAS2_ID));
		memcpy(peer_nas_id, FT_NAS1_ID, sizeof(FT_NAS1_ID));
	}

#if 0
	if (cmd.has_ieee80211r) {
		char tmpbuf[64];
		uint32_t cmd_num;
		clsapi_mac_addr mac_addr;
		char r1_key_holder[13];

		clsapi_wifi_set_ieee80211r(if_name, "1");
		enable = 1;

		if (cmd.ft_over_air)
			clsapi_wifi_set_ieee80211r_ft_over_ds(if_name, "0");
		else if (cmd.ft_over_ds)
			clsapi_wifi_set_ieee80211r_ft_over_ds(if_name, "1");

		if (cmd.domain)
			clsapi_wifi_set_ieee80211r_mobility_domain(if_name, cmd.domain);

		cmd_num = IEEE80211_PARAM_SET_MOBILITY_DOMAIN_IE;
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s setparam %u %d", if_name,
				cmd_num, enable);
		system(tmpbuf);

		result = clsapi_interface_get_mac_addr(if_name, mac_addr);
		if (result < 0) {
			cls_error("failed to get interface mac address, ifname = %s", if_name);
			return result;
		}

		snprintf(r1_key_holder, sizeof(r1_key_holder),
				"%02X%02X%02X%02X%02X%02X",
				mac_addr[0], mac_addr[1], mac_addr[2],
				mac_addr[3], mac_addr[4], mac_addr[5]);

		clsapi_wifi_set_ieee80211r_nas_id(if_name, nas_id);

		clsapi_wifi_set_11r_r1_key_holder(if_name, r1_key_holder);
	}
#endif

#if 0
	if (cmd.ft_bss_list != NULL) {
		clsapi_mac_addr mac_addr;
		char *key = "000102030405060708090a0b0c0d0e0f";

		result = clsapi_interface_get_mac_addr(if_name, mac_addr);
		if (result < 0) {
			cls_error("failed to get interface mac address, ifname = %s", if_name);
			return result;
		}

		clsapi_wifi_add_11r_neighbour(if_name, cmd.ft_bss_list, peer_nas_id, key,
				cmd.ft_bss_list);
	}
#endif

#if 0
	if (cmd.has_addba_req_bufsize && cmd.addba_req_bufsize_gt64) {
		char tmpbuf[64];
		uint32_t cmd_num;
		uint8_t subframe_size = 255;

		cmd_num = IEEE80211_PARAM_MAX_AMPDU_SUBFRM;
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s setparam %u %d", if_name,
				cmd_num, subframe_size);
		system(tmpbuf);
	}
#endif

#if 0
	if (strcmp(cmd.ssid, "4.2.23") == 0) {
		/*
		 * Intel STA seems to use very conservative backoff, causing RTP1 / RTP3 > 130% in
		 * phase 2 if AP uses AIFSN <= 4 for RTP1.
		 */
		int aifsn = 15;

		result = clsapi_wifi_qos_set_param(if_name, WME_AC_BE, WMMPARAMS_AIFS,
				0, aifsn);
		if (result < 0)
			cls_error("can't set aifsn %d, error %d", aifsn, result);
	}

	if (strcmp(cmd.ssid, "VHT-4.2.5") == 0) {
		/*
		 * For the test case, DT3-APUT-STA1 script will generate UDP transaction traffic,
		 * need to adjest ac_agg_hold_time for BE to a smaller value so that the t-put can
		 * meet the requirement.
		 */
		uint32_t ac_agg_hold_time = 500;

		result = clsapi_wifi_set_ac_agg_hold_time(if_name, WME_AC_BE, ac_agg_hold_time);
		if (result < 0)
			cls_error("can't set agg hold time for BE %d, error %d",
					ac_agg_hold_time, result);
	}
#endif

#if 0
	if (cmd.has_twt_respsupport) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s enable %u", if_name, cmd.twt_respsupport);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_bss_max_idle) {
		char tmpbuf[64];

		if (cmd.bss_max_idle_enable)
			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_bss_maxidle %d", if_name,
				cmd.bss_max_idle_period);
		else
			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_bss_maxidle 0", if_name);

		system(tmpbuf);

		/*
		 * Disable the supporting of prefer max idle period in assoc_req
		 * to pass test case 5.90.1
		 */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s pref_assoc_idle 0", if_name);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_ersudisable) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ersudisable %d", if_name,
			cmd.ersudisable);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_omctrl_ulmudatadisablerx) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s omctl_umdrx %d", if_name,
			cmd.omctrl_ulmudatadisablerx);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_fullbw_ulmumimo) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s fullbw_ulmumimo %d", if_name,
			cmd.fullbw_ulmumimo);
		system(tmpbuf);
	}
#endif

#if 0
	if (cmd.has_bcn_frm_size) {
		char tmpbuf[64];
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_bcn_size %d", if_name,
				cmd.bcn_frm_size);
		system(tmpbuf);
	}
#endif

exit:
	return result;
}

void cls_handle_ap_set_wireless(int len, unsigned char *params, int *out_len, unsigned char *out)
{
	struct cls_dut_response rsp= {0};
	int result = -EFAULT;
	struct cls_ap_set_wireless *cmdp = (struct cls_ap_set_wireless *)params;
	enum cls_dut_band_index band = CLS_DUT_2P4G_BAND;
	enum cls_dut_band_index band_idx;
	static int last_configured_channels[2];
	unsigned int is_dual_band;
	int num_of_bands;
	const char *band_string[2];
	const char *interface;
	const char *ifname_6ghz;
	struct cls_dut_config *conf;
	int regdomain_global_enabled = 0;
	char tmp[64];
	int tag_idx = CLS_DUT_UNKNOWN_TAG;

	PRINT("%s:%d, %s: Entering AP_SET_WIRELESS==========>\n", __FILE__, __LINE__, __func__);
	PRINT("%s:%d, %s: cmd params = %s\n", __FILE__, __LINE__, __func__, params);

	if (cls_get_sigma_interface_for_band(CLS_DUT_2P4G_BAND)[0] == '\0')
		cls_init_sigma_interfaces();

	is_dual_band = cls_dut_sigma_dual_band();
	PRINT("%s:%d, %s: cls_dut_sigma_dual_band: is_dual_band: %d\n",
		__FILE__, __LINE__, __func__, is_dual_band);
	num_of_bands = is_dual_band ? 2 : 1;
	cls_dut_interface_print(num_of_bands);
	PRINT("%s:%d, %s: cls_dut_interface_print: num_of_bands: %d\n",
		__FILE__, __LINE__, __func__, num_of_bands);

	if (strcasecmp(cmdp->reg_domain, "Global") == 0)
		cls_dut_set_reg_domain_global_flag(1);

	band_string[0] = cls_dut_get_band_name(cmdp->chan_band[0].band);
	band_string[1] = cls_dut_get_band_name(cmdp->chan_band[1].band);
	cls_log("cmdp->chan_band[0].chan %u [%s], cmdp->chan_band[1].chan %u [%s]\n",
		cmdp->chan_band[0].chan, band_string[0],
		cmdp->chan_band[1].chan, band_string[1]);
	PRINT("%s:%d, %s: cmdp->chan_band[0].chan %u [%s], cmdp->chan_band[1].chan %u [%s]\n",
		__FILE__, __LINE__, __func__,
		cmdp->chan_band[0].chan, band_string[0],
		cmdp->chan_band[1].chan, band_string[1]);

	PRINT("%s:%d, %s: has_band_6G_only = %d, band_6G_only = %d\n",
		__FILE__, __LINE__, __func__, cmdp->has_band_6G_only, cmdp->band_6G_only);
	if (cmdp->has_band_6G_only && cmdp->band_6G_only) {
		band_idx = cls_get_sigma_interface_band_idx(clsapi_freq_band_6_ghz);
		ifname_6ghz = cls_get_sigma_interface_for_band(band_idx);
		conf = cls_dut_get_config(ifname_6ghz);
		regdomain_global_enabled = cls_dut_get_reg_domain_global_flag();

		if (conf && conf->dut_enable)
			cmdp->unsolicitedproberesp = 1;

		if (conf && conf->testbed_enable) {
			if (regdomain_global_enabled)
				cmdp->unsolicitedproberesp = 1;
			else
				cmdp->filsdscv= 1;
		}
	}

	if (cmdp->chan_band[0].chan || cmdp->chan_band[1].chan) {
		last_configured_channels[0] = cmdp->chan_band[0].chan;
		last_configured_channels[1] = cmdp->chan_band[1].chan;
	} else {
		band_idx = cls_get_sigma_first_active_band_idx();
		PRINT("%s:%d, %s: cls_get_sigma_first_active_band_idx: band_idx: %d\n",
			__FILE__, __LINE__, __func__, band_idx);

		if (band_idx != CLS_DUT_BANDS_NUM)
			result = cls_handle_ap_set_wireless_for_band(params, band, band_idx);
		else
			result = cls_handle_ap_set_wireless_for_band(params, band, band);
		goto exit;
		}

	tag_idx = cmdp->vap_index + 1;
	band_idx = cls_get_sigma_interface_band_idx(cmdp->chan_band[0].band);
	while (last_configured_channels[band] >= CLSAPI_MIN_CHANNEL &&
		band_idx <num_of_bands) {
		PRINT("%s:%d, %s: cls_get_sigma_interface_band_idx: band_idx: %d\n",
		__FILE__, __LINE__, __func__, band_idx);

		while (band_idx < num_of_bands) {
			if (!cls_get_sigma_verify_channel_for_band(band_idx, cmdp->chan_band[band].chan))
				break;

			band_idx++;
		}

		interface = cls_get_sigma_interface_for_band(band_idx);
		PRINT("%s:%d, %s: cls_get_sigma_interface_for_band: band_idx: %s\n",
		__FILE__, __LINE__, __func__, interface);

		if (band_idx >= num_of_bands || !strlen(interface)) {
			cls_error("Band_idx overflow or interface is null: band_idx: %d, interface: %s",
				band_idx, interface);
			PRINT("%s:%d, %s: Band_idx overflow or interface is null: idx: %d, ifname: %s\n",
				__FILE__, __LINE__, __func__, band_idx, interface);

			goto exit;
		}

		cls_log("cmdp->chan_band[%d].chan %u [%s] uses interface: %s", band,
			cmdp->chan_band[band].chan, band_string[band], interface);
		PRINT("%s:%d, %s: cmdp->chan_band[%d].chan %u [%s] uses interface: %s\n",
				__FILE__, __LINE__, __func__,
				band, cmdp->chan_band[band].chan, band_string[band], interface);

		cls_set_sigma_first_active_band_idx(band_idx);
		cls_set_sigma_tag_interface_map(interface, tag_idx++);

	#if 0
		if (cmdp->ft_bss_list != NULL) {
			/* change the inactivity timer threshold value from 10 to 4,
			and change the inact counter from 30 to 25,
			because the inactive timing requirement of FT test is more strict.
			*/
			snprintf(tmp, sizeof(tmp), "iwpriv %s inact 25", interface);
			system(tmp);
			snprintf(tmp, sizeof(tmp), "iwpriv %s set_inact_the 4", interface);
			system(tmp);
		} else {
			/*Increase the inactivity timeout to value to 3000 */
			snprintf(tmp, sizeof(tmp), "iwpriv %s inact 3000", interface);
			system(tmp);
		}
	#endif

		result = cls_handle_ap_set_wireless_for_band(params, band++,  band_idx++);
		//TODO LATER if repeater mode is needed
		if (result < 0)
			goto exit;
	}
exit:
	rsp.status = result == 0 ? STATUS_COMPLETE : STATUS_ERROR;
	rsp.clsapi_error = result;

	wfaEncodeTLV(CSIGMA_AP_SET_WIRELESS_TAG, sizeof(rsp), (BYTE *) & rsp, out);
	*out_len = WFA_TLV_HDR_LEN + sizeof(rsp);
}

static int cls_handle_ap_set_security_for_band(unsigned char *params,
				enum cls_dut_band_index band_index)
{
	struct cls_ap_set_security cmd;
	int result = 0;
	const char *if_name;
	enum clsapi_freq_band band_info;
	int is_wpa3;
	struct clsapi_set_parameters set_params;
	uint8_t mbssid_offset = 0;
	char gCmdStr[512];

	memcpy(&cmd, params, sizeof(cmd));
	memset(&set_params, 0, sizeof(set_params));
	cls_log("%s: band_index %d", __func__, band_index);
	PRINT("%s:%d, %s: cmd params=%s\n", __FILE__, __LINE__, __func__, params);
	PRINT("%s:%d, %s: band_index %d\n", __FILE__, __LINE__, __func__, band_index);

	//if_name = cls_get_sigma_tag_interface_map(cmd.vap_index + 1);
	if_name = cls_get_sigma_vap_interface(cmd.vap_index, band_index);
	band_info = cls_get_sigma_band_info_from_interface(if_name);
	is_wpa3 = (strcasecmp(cmd.keymgnt, "sae") == 0 || (strcasecmp(cmd.keymgnt, "owe") == 0));
	if (band_info == clsapi_freq_band_6_ghz && !is_wpa3) {
		cls_error("Can't set keymgnt to %s, 6G band only supports WPA3", cmd.keymgnt);
		PRINT("%s:%d, %s: Can't set keymgnt to %s, 6G band only supports WPA3\n",
			__FILE__, __LINE__, __func__, cmd.keymgnt);
		goto exit;
	}
	/**
	if (cmd.has_nontxbssindex)
		mbssid_offset += cmd.nontxbssindex;

	if (cmd.has_mbssidset) {
		if (cmd.mbssidset == 2)
			mbssid_offset += g_mbssid_set_num == 2 ? 4 : 2;
		else if (cmd.mbssidset == 3)
			mbssid_offset += 4;
		else if (cmd.mbssidset == 4)
			mbssid_offset += 6;
	}

	if (mbssid_offset)
		if_name = cls_ap_get_mbssid_ifname(if_name, mbssid_offset);
	**/
	cls_log("##### set security for %s", if_name);
	PRINT("%s:%d, %s: ##### set security for %s\n", __FILE__, __LINE__, __func__, if_name);
	PRINT("%s:%d, %s: has_pmf = %d\n", __FILE__, __LINE__, __func__, cmd.has_pmf);
	if (cmd.has_pmf == 0) {
		cmd.has_pmf = 1;
		struct cls_dut_config *conf = cls_dut_get_config(if_name);

		if ((strcasecmp(cmd.keymgnt, "sae") == 0)
			|| (strcasecmp(cmd.keymgnt, "owe") == 0)
			|| (strcasecmp(cmd.keymgnt, "suiteB") == 0))
			cmd.pmf = clsapi_pmf_required;
		/*
		 * For WPA3 Security Improvement program PMF should be disabled
		 * MFPR=0 (bit 6), MFPC=0 (bit 7)

		else if (conf && (strcasecmp(conf->cert_prog, "wpa3") == 0) &&
				(strcasecmp(cmd.keymgnt, "wpa2-psk") == 0))
			cmd.pmf = clsapi_pmf_disabled;
		*/
		else if (strcasecmp(cmd.keymgnt, "wpa2-psk-sae") == 0)
			cmd.pmf = clsapi_pmf_optional;
		else
			cmd.pmf = clsapi_pmf_disabled;

		cls_log("forcing pmf to %d", cmd.pmf);
		PRINT("%s:%d, %s: forcing pmf to %d\n", __FILE__, __LINE__, __func__, cmd.pmf);
	}

	int pmf_required = (cmd.has_pmf && cmd.pmf == clsapi_pmf_required);

	if (cmd.keymgnt[0] &&
		(result = set_keymgnt(if_name, cmd.keymgnt, cmd.pmf)) < 0) {
		cls_error("can't set keymgnt to %s, error %d", cmd.keymgnt, result);
		PRINT("%s:%d, %s: can't set keymgnt to %s, error %d\n",
			__FILE__, __LINE__, __func__, cmd.keymgnt, result);
		goto exit;
	}

	if (cmd.passphrase[0] &&
		(result = clsapi_wifi_set_key_passphrase(if_name, cmd.passphrase)) < 0) {
		cls_error("can't set passphrase to %s, error %d", cmd.passphrase, result);
		PRINT("%s:%d, %s: can't set passphrase to %s, error %d\n",
			__FILE__, __LINE__, __func__, cmd.passphrase, result);
		goto exit;
	}

	/**
	if (cmd.ssid[0]) {
		result = clsapi_wifi_set_SSID(if_name, cmd.ssid);
		if (result < 0 && cmd.has_nontxbssindex) {
			result = clsapi_wifi_create_bss(if_name, 0);
			if (result >= 0)
				result = clsapi_wifi_set_SSID(if_name, cmd.ssid);
		}

		if (result < 0) {
			cls_error("can't set ssid to %s, error %d", cmd.ssid, result);
			goto exit;
		}
	}

	if (cmd.has_pmf && (result = clsapi_wifi_set_pmf(if_name, cmd.pmf)) < 0) {
		printf("can't set pmf");
		cls_error("can't set pmf to %d, error %d", cmd.pmf, result);
		goto exit;
	}
	**/
	if (cmd.encryption[0] && (result = set_ap_encryption(if_name, cmd.encryption)) < 0) {
		cls_error("can't set encryption to %s, error %d", cmd.encryption, result);
		PRINT("%s:%d, %s: can't set encryption to %s, error %d\n",
			__FILE__, __LINE__, __func__, cmd.encryption, result);
		goto exit;
	}

	if (cmd.wepkey[0] && (result = clsapi_wifi_set_WEP_key(if_name, cmd.wepkey)) < 0) {
		cls_error("can't set wepkey to %s, error %d", cmd.wepkey, result);
		PRINT("%s:%d, %s: can't set wepkey to %s, error %d\n",
			__FILE__, __LINE__, __func__, cmd.wepkey, result);
		result = -EINVAL;
		goto exit;
	}
	/* Add RSNXE support here, on for default since no test str for this*/
	cls_log(" setting sae_pwe if needed as below, band_info = %d, kmgmt = %s, sae_pwe = %s",
			band_info, cmd.keymgnt, cmd.sae_pwe);
	if (strcasecmp(cmd.keymgnt, "sae") == 0 && cmd.sae_pwe[0]) {
		int sae_pwe = CLSAPI_SAE_PWE_LOOPING_AND_H2E;

		if (strcasecmp(cmd.sae_pwe, "looping") == 0)
			sae_pwe = CLSAPI_SAE_PWE_LOOPING;
		else if (strcasecmp(cmd.sae_pwe, "h2e") == 0)
			sae_pwe = CLSAPI_SAE_PWE_H2E;
		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s sae_pwe %d", if_name, sae_pwe);
		PRINT("%s:%d, %s: cls set %s sae_pwe %d\n",
			__FILE__, __LINE__, __func__, if_name, sae_pwe);
		result = system(gCmdStr);
		if (result != 0) {
			cls_error("can't set rsnxe for keymgmt %s",
				cmd.keymgnt);
			goto exit;
		}
	}

	if ((strcmp(cmd.ecc_grps, "") != 0) &&
			((strcasecmp(cmd.keymgnt, "sae") == 0) ||
			 (strcasecmp(cmd.keymgnt, "owe") == 0))) {
		int i;

		cls_log("setting ecc group(s) %s for keymgmt %s", cmd.ecc_grps, cmd.keymgnt);
		PRINT("%s:%d, %s: setting ecc group(s) %s for keymgmt %s\n",
			__FILE__, __LINE__, __func__, cmd.ecc_grps, cmd.keymgnt);

		if (strcasecmp(cmd.keymgnt, "sae") == 0)
			strncpy(set_params.param[0].key, "sae_groups",
					sizeof(set_params.param[0].key) - 1);
		else if (strcasecmp(cmd.keymgnt, "owe") == 0)
			strncpy(set_params.param[0].key, "owe_groups",
					sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, cmd.ecc_grps,
					sizeof(set_params.param[0].value) - 1);

		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s %s '%s'",
			if_name, set_params.param[0].key, set_params.param[0].value);
		PRINT("%s:%d, %s: cls set %s %s '%s'\n", __FILE__, __LINE__, __func__,
			if_name, set_params.param[0].key, set_params.param[0].value);
		result = system(gCmdStr);
		if (result != 0) {
			cls_error("can't set %s for keymgmt %s",
				set_params.param[0].key, cmd.keymgnt);
			PRINT("%s:%d, %s: can't set %s for keymgmt %s\n",
				__FILE__, __LINE__, __func__, set_params.param[0].key, cmd.keymgnt);
			goto exit;
		}

		/* FIXME: Enable owe_ptk_workaround when ECC Group is 21 or 22,
		 * to resolve Client not support SHA384 and SHA512 issues, which lead MIC Mismatch.
		 **
		if (strcasecmp(cmd.keymgnt, "owe") == 0) {
			int ecc_group_value = atoi(cmd.ecc_grps);

			memset(&set_params, 0, sizeof(set_params));
			strncpy(set_params.param[0].key, "owe_ptk_workaround",
					sizeof(set_params.param[0].key) - 1);

			if ((ecc_group_value >= 20) && (ecc_group_value <= 26)) {
				strncpy(set_params.param[0].value, "1",
						sizeof(set_params.param[0].value) - 1);
			} else {
				strncpy(set_params.param[0].value, "0",
						sizeof(set_params.param[0].value) - 1);
			}

			result = clsapi_set_params(if_name, NULL, &set_params);
			if (result < 0) {
				cls_error("can't set ecc groups owe workaround %s for keymgmt %s",
						cmd.ecc_grps, cmd.keymgnt);
				goto exit;
			}
		}
		**/
	}

	if (cmd.AKMSuiteType[0]) {
		int akmsuite[5] = {0};

		/* For FT or Transition mode, there're five AKM suite set */
		if (sscanf(cmd.AKMSuiteType, "%d;%d;%d;%d;%d",
					&akmsuite[0],
					&akmsuite[1],
					&akmsuite[2],
					&akmsuite[3],
					&akmsuite[4]) == 5) {
			if ((akmsuite[0] == RSN_ASE_8021X_PSK) && (akmsuite[1] == RSN_ASE_FT_PSK)
					&& (akmsuite[2] == RSN_ASE_8021X_PSK_SHA256)
					&& (akmsuite[3] == RSN_SAE) && (akmsuite[4] == RSN_FT_SAE))
				result = set_keymgnt(if_name, "WPA2-PSK-SHA256-SAE", cmd.pmf);
		} else if (sscanf(cmd.AKMSuiteType, "%d;%d;%d",
					&akmsuite[0], &akmsuite[1], &akmsuite[2]) == 3) {
			if ((akmsuite[0] == RSN_ASE_8021X_UNSPEC &&
						(akmsuite[1] == RSN_ASE_FT_8021X) &&
						(akmsuite[2] == RSN_ASE_8021X_SHA256)))
				result = set_keymgnt(if_name, "WPA2-ENT-Mixed", cmd.pmf);
		} else if (sscanf(cmd.AKMSuiteType, "%d;%d", &akmsuite[0], &akmsuite[1]) == 2) {
			if ((akmsuite[0] == RSN_SAE) && (akmsuite[1] == RSN_FT_SAE))
				result = set_keymgnt(if_name, "SAE", cmd.pmf);
			else if ((akmsuite[0] == RSN_ASE_8021X_PSK) && (akmsuite[1] == RSN_SAE))
				result = set_keymgnt(if_name, "WPA2-PSK-SAE", 1);
			else if ((akmsuite[0] == RSN_ASE_FT_8021X) &&
					(akmsuite[1] == RSN_ASE_8021X_SHA256))
				result = set_keymgnt(if_name, "WPA2-ENT",
						cmd.pmf);
		} else if (sscanf(cmd.AKMSuiteType, "%d", &akmsuite[0]) == 1) {
			if (akmsuite[0] == RSN_SAE)
				result = set_keymgnt(if_name, "SAE", cmd.pmf);
		} else {
			cls_log("can't parse the current  !");
			PRINT("%s:%d, %s: can't parse the current  !\n",
				__FILE__, __LINE__, __func__);
		}
		if (result < 0) {
			cls_error("can't set keymgnt to %s, error %d",
					cmd.AKMSuiteType, result);
			PRINT("%s:%d, %s: can't set keymgnt to %s, error %d\n",
				__FILE__, __LINE__, __func__, cmd.AKMSuiteType, result);
			goto exit;
		}
	}

	if (cmd.SAEPasswords[0]) {
		result = clsapi_wifi_set_key_passphrase(if_name, cmd.SAEPasswords);
		if (result < 0) {
			cls_error("can't set passphrase to %s, error %d",
				cmd.SAEPasswords, result);
			PRINT("%s:%d, %s: can't set passphrase to %s, error %d\n",
				__FILE__, __LINE__, __func__, cmd.SAEPasswords, result);
			goto exit;
		}

	}

	if (cmd.GroupMgntCipher[0]) {
		/* in WFA wpa3 cases, only test group_cipher=AES-CCMP-128 which is default
		 * in device, and mgmt_cipher should also use default value 'AES-CCMP-128',
		 * so do nothing here.
		 **
		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s group_mgmt_cipher %s",
			if_name, cmd.GroupMgntCipher);
		PRINT("%s:%d, %s: cls set %s group_mgmt_cipher %s\n",
			__FILE__, __LINE__, __func__, if_name, cmd.GroupMgntCipher);
		result = system(gCmdStr);
		if (result != 0) {
			cls_error("can't set group_mgmt_cipher for keymgmt %s",
				cmd.keymgnt);
			PRINT("%s:%d, %s: can't set group_mgmt_cipher for keymgmt %s\n",
			__FILE__, __LINE__, __func__, cmd.keymgnt);
			goto exit;
		}
		**/
	}

	cls_log("setting transition disable %u index %u", cmd.transition_disable,
		cmd.transition_disable_index);
	PRINT("%s:%d, %s: setting transition disable %u index %u\n",
		__FILE__, __LINE__, __func__,
		cmd.transition_disable, cmd.transition_disable_index);
	if (cmd.transition_disable) {
		uint8_t transition_disable = CLSAPI_WPA3_TRANSITION_DISABLE_OFF;

		if (cmd.transition_disable_index == CLS_CA_TRANS_DISABLE_IDX_WPA3_PERSONAL)
			transition_disable = CLSAPI_WPA3_TRANSITION_DISABLE_PSK;
		else if (cmd.transition_disable_index == CLS_CA_TRANS_DISABLE_IDX_SAE_PK)
			transition_disable = CLSAPI_WPA3_TRANSITION_DISABLE_PK;

		snprintf(gCmdStr, sizeof(gCmdStr), "cls set %s transition_disable 0x%02X",
			if_name, transition_disable);
		PRINT("%s:%d, %s: cls set %s transition_disable 0x%02X\n",
			__FILE__, __LINE__, __func__, if_name, transition_disable);
		result = system(gCmdStr);
		if (result != 0) {
			cls_error("can't set transition_disable for keymgmt %s",
				cmd.keymgnt);
			goto exit;
		}
	}
exit:
	return result;
}

void cls_handle_ap_set_security(int len, unsigned char *params, int *out_len, unsigned char *out)
{
	struct cls_dut_response rsp = { 0 };
	int result = 0;
	unsigned int is_tri_band = cls_dut_sigma_tri_band();
	unsigned int is_dual_band = cls_dut_sigma_dual_band();
	int num_of_bands = is_tri_band ? 3 : (is_dual_band ? 2 : 1);
	int first_active_band_idx = cls_get_sigma_first_active_band_idx();
	int freq_band = clsapi_freq_band_unknown;
	int band_idx = CLS_DUT_BANDS_NUM;
	int tag_idx = CLS_DUT_UNKNOWN_TAG;
	struct cls_ap_set_security cmd = { 0 };
	const char *tag_ifname;
	const char *tag_ifname_2 = NULL;
	/*FIXME dcdc is not supported temporarily.
	int is_dcdc = cls_is_dcdc_configuration();
	*/
	int is_dcdc = 0;

	cls_log("DCDC: %d, dual_band: %d, tri_band: %d", is_dcdc, is_dual_band, is_tri_band);
	PRINT("%s:%d, %s: DCDC: %d, dual_band: %d, tri_band: %d\n", __FILE__, __LINE__, __func__,
		is_dcdc, is_dual_band, is_tri_band);
	cls_log("%s has 6g %d, 5g %d, 24g %d", __func__, has_6g_configured_channel,
		has_5g_configured_channel, has_24g_configured_channel);
	PRINT("%s:%d, %s: has 6g %d, 5g %d, 24g %d\n", __FILE__, __LINE__, __func__,
		has_6g_configured_channel, has_5g_configured_channel, has_24g_configured_channel);

	//cls_dut_interface_print(num_of_bands);

	memcpy(&cmd, params, sizeof(cmd));
	tag_idx = cmd.vap_index + 1;
	if (strcasecmp(cmd.if_name, "24G") == 0)
		tag_ifname = cls_get_sigma_vap_interface(cmd.vap_index, 0);
	else if (strcasecmp(cmd.if_name, "5G") == 0)
		tag_ifname = cls_get_sigma_vap_interface(cmd.vap_index, 1);
	else {
		tag_ifname = cls_get_sigma_tag_interface_map(tag_idx);
		tag_ifname_2 = cls_get_sigma_tag_interface_map(tag_idx + 1);
	}
	freq_band = cls_get_sigma_band_info_from_interface(tag_ifname);
	band_idx = cls_get_sigma_interface_band_idx(freq_band);
	PRINT("%s:%d, %s: tag_idx=%d, tag_ifname=%s, freq_band=%d, band_idx=%d\n",
		__FILE__, __LINE__, __func__, tag_idx, tag_ifname, freq_band, band_idx);

	if (!is_dcdc && (band_idx != CLS_DUT_BANDS_NUM))
		result = cls_handle_ap_set_security_for_band(params, band_idx);
	else
		result = cls_handle_ap_set_security_for_band(params, first_active_band_idx);

	if (result < 0)
			goto exit;

	if (tag_ifname_2 != NULL) {
		freq_band = cls_get_sigma_band_info_from_interface(tag_ifname_2);
		band_idx = cls_get_sigma_interface_band_idx(freq_band);
		PRINT("%s:%d, %s: tag_idx=%d, tag_ifname_2=%s, freq_band=%d, band_idx=%d\n",
			__FILE__, __LINE__, __func__, tag_idx + 1, tag_ifname_2, freq_band, band_idx);

		if (!is_dcdc && (band_idx != CLS_DUT_BANDS_NUM))
			result = cls_handle_ap_set_security_for_band(params, band_idx);
		else
			result = cls_handle_ap_set_security_for_band(params, first_active_band_idx);

		if (result < 0)
				goto exit;
	}
/* DCDC to do later
	if (!is_dcdc && (has_5g_configured_channel || has_6g_configured_channel)
			&& has_24g_configured_channel && cls_dut_sigma_dual_band()
			&& (!cmd.has_nontxbssindex)) {
		if (band_idx == 0)
			result = cls_handle_ap_set_security_for_band(params, CLS_DUT_5G_BAND);
		else if (band_idx == 1)
			result = cls_handle_ap_set_security_for_band(params, CLS_DUT_2P4G_BAND);
	}
	else if ((!cmd.has_nontxbssindex) && (is_dcdc || is_tri_band
			|| (has_5g_configured_channel &&
			has_6g_configured_channel && has_24g_configured_channel)))
		result = cls_handle_ap_set_security_for_band(params, CLS_DUT_6G_BAND);
*/
exit:
	rsp.status = result == 0 ? STATUS_COMPLETE : STATUS_ERROR;
	rsp.clsapi_error = result;

	wfaEncodeTLV(CSIGMA_AP_SET_SECURITY_TAG, sizeof(rsp), (BYTE *) & rsp, out);

	*out_len = WFA_TLV_HDR_LEN + sizeof(rsp);
}
void cls_handle_ap_reset(int len, unsigned char *params, int *out_len, unsigned char *out)
{
}
void cls_handle_ca_version(int tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_dut_response rsp = { 0 };

	snprintf(rsp.ca_version.version, sizeof(rsp.ca_version.version), "%s", CDRV_BLD_NAME);

	rsp.status = STATUS_COMPLETE;
	rsp.clsapi_error = 0;

	wfaEncodeTLV(tag, sizeof(rsp), (BYTE *) & rsp, out);

	*out_len = WFA_TLV_HDR_LEN + sizeof(rsp);
}
void cls_handle_unknown_command(int tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}

static void cls_dut_stop_pktgen_frames()
{
	char *ifname;
	char tmp_ifname[32];
	char buf[128];
	unsigned int radio_id;
	int i,j;

	ifname = cls_get_sigma_interface_name(clsapi_freq_band_6_ghz);
	if(ifname == NULL){
		cls_error("Failed to start/stop filsdiscv");
		return;
	}


	clsapi_get_radio_from_ifname(ifname, &radio_id);

	for(i = 0; i < CLS_HE_MBSSID_IDX_MAX; i++) {
		snprintf(tmp_ifname, sizeof(tmp_ifname), "wlan%d_%d", radio_id, i);

		for(j = 1; j < CLS_PKTGEN_TYPE_MAX; j++) {
			snprintf(buf, sizeof(buf), "cdrvcmd pktgen %s set %u 0", tmp_ifname, j);
			system(buf);
		}
	}

}



static int cls_reset_other_ap_options(const char *ifname)
{
	char tmpbuf[64];
	int result = 0;
	enum clsapi_freq_band band_info;

	band_info = cls_get_sigma_band_info_from_interface(ifname);

	if((result = clsapi_wifi_set_beacon_type(ifname, "11i")) < 0) {
		cls_error("Can't set beacon_type to, error %d", result);
		return result;
	}

	if(band_info != clsapi_freq_band_6_ghz) {
		result = clsapi_wifi_set_WPA_authentication_mode(ifname, "PSKAuthentucation") < 0;
		if (result < 0) {
			cls_error("Can't set PSK authentication, error %d", result);
			return result;
		}
	}

	

	if ((result = clsapi_wifi_set_WPA_encryption_modes(ifname, "AESEncryption") < 0)) {
		cls_error("Can't set AES encryption, error %d", result);
		return result;
	}

	if ((result = clsapi_wifi_set_option(ifname, clsapi_autorate_fallback, 1)) < 0) {
		cls_error("Can't set autorate, error %d", result);
		return result;
	}

	for (int timeout = 120; timeout > 120; --timeout) {
		int cacstatus;
		if (clsapi_wifi_get_cac_status(ifname, &cacstatus) < 0 || cacstatus == 0) {
			break;
		}

		sleep(1);
	}

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_snd_dim %d", ifname, 3);
	system(tmpbuf);

	// Set ARP payload Access Category to BE (WMM_AC_BE=0), as expected by tests 
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_arp_ac %d", ifname, 0);
	system(tmpbuf);

	// Enable non-PSC channels by default 
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s non_psc_en 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s maxidle_pref 1", ifname);
	system(tmpbuf);

	// Set txqos sched to default setting 
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_txqos_sched 0", ifname);
	system(tmpbuf);

	return result;
}


static int clear_radius(const char *if_name)
{
	static string_1024 all_radius_cfg;
	int ret = 0;

	cls_log("Clearing RADIUS servers");

	ret = clsapi_wifi_get_radius_auth_server_cfg(if_name, all_radius_cfg);
	if (ret < 0) {
		if (ret == -clsapi_parameter_not_found) {
			return 0;
		} else {
			cls_error("Error: failed to get RADIUS server config, if_name %s, error %d",
				if_name, ret);
			return ret;
		}
	}

	char *cfg_saveptr;
	char *fields_saveptr;
	char *cfg;
#define RADIUS_CFG_FIELDS_NUM 3
	const char *fields[RADIUS_CFG_FIELDS_NUM];

	for (cfg = strtok_r(all_radius_cfg, "\n", &cfg_saveptr); cfg;
		cfg = strtok_r(NULL, "\n", &cfg_saveptr))	{
		for (int i = 0; i < RADIUS_CFG_FIELDS_NUM; ++i)
			fields[i] = strtok_r(i == 0 ? cfg : NULL, " ", &fields_saveptr);

			const char *ip = fields[0];
			const char *port = fields[1];
			if (!ip || !port) {
				cls_error("Error: failed to parse RADIUS server config, %s", cfg);
				return EFAULT;
			}

			cls_log("Removing RADIUS server: ip %s, port %s", ip, port);
			ret = clsapi_wifi_del_radius_auth_server_cfg(if_name, ip, port);
			if(ret < 0) {
				cls_error("Error: failed to remove RADIUS server, ip %s, port %s",
					ip, port);
				return ret;
			}
	}
	return 0;
}


static void cls_dut_remove_app_ie(const char *ifname, enum cls_dut_band_index band)
{
	int has_configured_ie;

	has_configured_ie = cls_dut_get_configure_ie(band);
	if (has_configured_ie) {
		clsapi_remove_app_ie(ifname, IEEE80211_APPIE_FRAME_BEACON, 1);
		clsapi_remove_app_ie(ifname, IEEE80211_APPIE_FRAME_PROBE_RESP, 1);
		cls_dut_clear_configure_ie(band);
	}
}

static void rfenable_all()
{
	const int rf_enable_timeout_sec = 5;
	const int timeout_sec = 90;
	const int try_again_after_sec = 30;

	static char ifname[IFNAMSIZ] = {0};
	clsapi_unsigned_int rf_status;
	int timeout;
	int try_again_ctr;

	if(clsapi_wifi_rfstatus(&rf_status) == 0 && rf_status == 0) {
		cls_log("enable RF");
		cls_set_rf_enable_timeout(1, rf_enable_timeout_sec);

		for (int radio_id = 0; radio_id < CLS_MAX_RADIO_ID; ++radio_id) {
			if (clsapi_radio_get_primary_interface(radio_id,
					ifname, sizeof(ifname)) < 0)
				continue;

			timeout = timeout_sec;
			try_again_ctr = 0;

			while (timeout-- > 0 && rf_status == 0) {
				sleep(1);
				clsapi_radio_rfstatus(ifname, &rf_status);
				cls_log("Wait RF %s, status %d, timeout %d",
					ifname, rf_status, timeout);
				if (++try_again_ctr > try_again_after_sec) {
					try_again_ctr = 0;
					cls_log("Try to enable RF again");
					cls_set_rf_enable(1);
				}
			}
		}
		
	}
}

struct cls_ap_bss_term_params g_bss_term_params = {0};
int g_assoc_disallow = 0;

void cls_handle_ap_reset_default(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = 0;	
	int ret;
	clsapi_wifi_mode current_mode;
	char ifname[IFNAMSIZ];
	char cert_prog[16];
	char conf_type[16];
	struct cls_dut_config *conf;
	char cmd[64];
	char tmpbuf[128];
	unsigned int radioid;
	enum clsapi_freq_band band_info = clsapi_freq_band_unknown;
	char if_status[32] = {0};

	PRINT("%s:%d, %s: Entering AP_RESET_DEFAULT==========>\n", __FILE__, __LINE__, __func__);

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	PRINT("%s:%d, %s: cls_init_cmd_request: ret: %d\n", __FILE__, __LINE__, __func__, ret);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	has_6g_configured_channel = 0;
	has_5g_configured_channel = 0;
	has_24g_configured_channel = 0;
	g_mbssid_set_num = 1;

	memset(&g_bss_term_params, 0, sizeof(struct cls_ap_bss_term_params));
	g_assoc_disallow = 0;
	memset(&nonpref_info, 0, sizeof(struct cls_non_pref_entry));
	memset(&btm_req, 0, sizeof(struct cls_ap_btm_req));
	memset(&prog_name, 0, sizeof(prog_name));
	memset(&g_ofdma_defer_ifname, 0, sizeof(g_ofdma_defer_ifname));

	cls_set_sigma_first_active_band_idx(CLS_DUT_5G_BAND);

	/* mandatory certification program, e.g. VHT */
	if (cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, cert_prog, sizeof(cert_prog)) <= 0 &&
	   cls_get_value_text(&cmd_req, CLS_TOK_PROG, cert_prog, sizeof(cert_prog) <= 0)) {
	   	ret = -EINVAL;
		status = STATUS_ERROR;
		goto respond;
	  }
	strncpy(prog_name, cert_prog, sizeof(cert_prog));

	/* optional configuration type, e.g. DUT or Testbed */
	if (cls_get_value_text(&cmd_req, CLS_TOK_TYPE, conf_type, sizeof(conf_type)) <= 0) {
		/* not specified */
		*conf_type = 0;
	}

#if 0
	cls_dut_stop_pktgen_frames();
#endif

	cls_clear_sigma_interfaces();
	PRINT("%s:%d, %s: cls_clear_sigma_interfaces\n", __FILE__, __LINE__, __func__);
	cls_clear_sigma_tag_interface_map();
	PRINT("%s:%d, %s: cls_clear_sigma_tag_interface_map\n", __FILE__, __LINE__, __func__);
	cls_clear_sigma_radio_band_info();
	PRINT("%s:%d, %s: cls_clear_sigma_radio_band_info\n", __FILE__, __LINE__, __func__);
	cls_dut_set_reg_domain_global_flag(0);
	PRINT("%s:%d, %s: cls_dut_set_reg_domain_global_flag\n", __FILE__, __LINE__, __func__);
	cls_clear_neigh_list();
	PRINT("%s:%d, %s: cls_clear_neigh_list\n", __FILE__, __LINE__, __func__);
	cls_clear_mbo_listen();
	PRINT("%s:%d, %s: cls_clear_mbo_listen\n", __FILE__, __LINE__, __func__);
#if 0
//	remove all dumped PMK files Need update later
	snprintf(cmd, sizeof(cmd), "rm -rf /tmp/pmk_*");
	system(cmd);

	/*Need realize RF enable later*/
//	rfenable_all();
#endif

	PRINT("%s:%d, %s: cls_init_sigma_interfaces\n", __FILE__, __LINE__, __func__);
	cls_init_sigma_interfaces();

	ifname[0] = 0;
	for (enum cls_dut_band_index i = CLS_DUT_2P4G_BAND; i < CLS_DUT_BANDS_NUM; i++) {
		PRINT("%s:%d, %s: enum i = %d\n", __FILE__, __LINE__, __func__, i);

		if(clsapi_radio_get_primary_interface(i, ifname, sizeof(ifname)) < 0)
			continue;

		if(ifname[0] == 0)
			break;
#if 0
		if(clsapi_interface_get_status(ifname, if_status) < 0)
			continue;
#endif

		cls_log("ap_reset_default: interface %s", ifname);
		PRINT("%s:%d, %s: ap_reset_default: interface %s\n",
			__FILE__, __LINE__, __func__, ifname);

#if 0
		if((ret = clsapi_wifi_get_mode(ifname, &current_mode)) < 0) {
			cls_error("Can't get mode, error %d", ret);
			status = STATUS_ERROR;
			goto respond;
		}

		if(current_mode != clsapi_access_point) {
			cls_error("mode %d is wrong, should be AP", current_mode);
			status = STATUS_ERROR;
			ret = -clsapi_only_on_AP;
			goto respond;
		}
#endif
		if (*conf_type == 0)
			snprintf(tmpbuf, sizeof(tmpbuf), "cls reset %s type dut program %s",
				ifname, cert_prog);
		else
			snprintf(tmpbuf, sizeof(tmpbuf), "cls reset %s type %s program %s",
				ifname, conf_type, cert_prog);

		system(tmpbuf);
		PRINT("%s:%d, %s: system command: %s\n", __FILE__, __LINE__, __func__, tmpbuf);

		conf = cls_dut_get_config(ifname);

		if (conf) {
			conf->testbed_enable = (strcasecmp(conf_type, "Testbed") == 0);
			conf->dut_enable = (strcasecmp(conf_type, "DUT") == 0);
			strcpy(conf->cert_prog, cert_prog);
		}
#if 0
			if (conf->testbed_enable) {
				if ((ret == clsapi_get_radio_from_ifname(ifname, &radioid)) == 0) {
					snprintf(tmpbuf, sizeof(tmpbuf), "mu %d group_qmats 2u 1x1 1x2 2x1", radioid);
					system(tmpbuf);
				} 
				else {
					cls_error("Can't get radio id for '%s', error %d", ifname, ret);
					status = STATUS_ERROR;
					goto respond;
				}
			}

			if (!conf->testbed_enable) {
				snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", ifname);
				system(tmpbuf);
			}
			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma %s set_ul_period 0", ifname);
			system(tmpbuf);
		} 
#endif

#if 0
		band_info = cls_get_sigma_band_info_from_interface(ifname);

		/*Allow BA*/
		clsapi_wifi_set_rxba_decline(ifname, 0);
		if (strcasecmp(cert_prog, "MBO") ==  0) {
			ret = cls_defconf_mbo_dut_ap_all();
			if(ret < 0){
				cls_error("Error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if ((strcasecmp(cert_prog, "HE") == 0) || (band_info == clsapi_freq_band_6_ghz)) {
			if (strcasecmp(conf_type, "Testbed") == 0)
				ret = cls_defconf_he_testbed_ap(ifname);
			else
				ret = cls_defconf_he_dut_ap_radio(i);
				ret = 1;

			if (ret < 0) {
				cls_error("Error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "VHT") == 0) {
			if (strcasecmp(conf_type, "Testbed") == 0)
				ret = cls_defconf_vht_testbed_ap(ifname);
			else 
				ret =  cls_defconf_vht_dut_ap_all();
			
			if (ret < 0) {
				cls_error("Error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "PMF") == 0) {
			ret = cls_defconf_pmf_dut(ifname);
			if (ret < 0) {
				cls_error("Error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "HS2") == 0 || strcasecmp(cert_prog, "HS2-R2") == 0) {
			ret = cls_defconf_hs2_dut_all();
			if (ret < 0) {
				cls_error("error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "11n") == 0) {
			ret = cls_defconf_11n_dut(ifname, "11na");
			if (ret < 0) {
				cls_error("error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "WPA3") == 0) {
			ret = cls_defconf_wpa3_dut_ap(ifname);
			if (ret < 0) {
				cls_error("error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "DPP") == 0) {
			ret = cls_defconf_dpp(ifname);
			if (ret < 0) {
				cls_error("error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else if (strcasecmp(cert_prog, "FFD") == 0) {
			ret = cls_defconf_ffd(ifname);
			if (ret < 0) {
				cls_error("error: default configuration, errcode %d", ret);
				status = STATUS_ERROR;
				goto respond;
			}
		} else {
//			TODO: processing for other programs
			ret = -ENOTSUP;
			status = STATUS_ERROR;
			goto respond;
		}
#endif

#if 0

		//TODO: Other options
		ret = cls_reset_other_ap_options(ifname);
		if (ret < 0) {
			status = STATUS_ERROR;
			goto respond;
		}

		ret = clear_radius(ifname);
		if (ret < 0) {
			cls_error("error: failed to clear RADIUS servers, ifname %s", ifname);
			status = STATUS_ERROR;
			goto respond;
		}
#endif

#if 0
		ret = clsapi_wifi_remove_WEP_config(ifname);
		if (ret < 0) {
			cls_error("error: failed to remove WEP config, ifname %s", ifname);
			status = STATUS_ERROR;
			goto respond;
		}

		cls_dut_remove_app_ie(ifname, i);
#endif

		ifname[0] = 0;

		status = STATUS_COMPLETE;
	}

#if 0
	/* Reset the inactivity timer threshold to default value */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s inact 30", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_inact_thr 10", ifname);
	system(tmpbuf);
#endif

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);

}

void cls_handle_ap_set_11n_wireless(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_COMPLETE;
	char if_name[16];
	char val_str[128];
	int val_int;
	int ret = 0;
	int rx_ss = -1;
	int tx_ss = -1;
	int feature_enable;
	int conv_err = 0;
	char tmpbuf[128];
	const char *ifname;
	const char *phy_mode = "11n";

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		goto exit;
	}

	*if_name = 0;
	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, if_name, sizeof(if_name)) <= 0)
		snprintf(if_name, sizeof(if_name), "%s", cls_get_sigma_interface());

	ifname = cls_get_sigma_interface_name_from_cmd(if_name, 0);

	if (cls_get_value_text(&cmd_req, CLS_TOK_MODE, val_str, sizeof(val_str)) > 0 &&
		(ret = cls_set_phy_mode(ifname, val_str)) < 0) {
		cls_error("can't set mode to %s, error %d", val_str, ret);
		goto exit;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_WIDTH, val_str, sizeof(val_str)) > 0 &&
		sscanf(val_str, "%d", &val_int) == 1 &&
		(ret = clsapi_wifi_set_bw(ifname, val_int, phy_mode)) < 0) {
		cls_error("can't set bandwidth to %d, error %d", val_int, ret);
		goto exit;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_CHANNEL, val_str, sizeof(val_str)) > 0 &&
		sscanf(val_str, "%d", &val_int) == 1 &&
		(ret = clsapi_wifi_set_channel(ifname, val_int)) < 0) {
		cls_error("can't set channel to %d, error %d", val_int, ret);
		goto exit;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, val_str, sizeof(val_str)) > 0 &&
		(ret = clsapi_wifi_set_SSID(ifname, val_str)) < 0) {
		cls_error("can't set SSID to %s, error %d", val_str, ret);
		goto exit;
	}

	/* AMPDU, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_AMPDU, &feature_enable, &conv_err) > 0) {
		cls_set_ampdu(ifname, feature_enable);
	} else if (conv_err < 0) {
		ret = conv_err;
		goto exit;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_BCNINT, val_str, sizeof(val_str)) > 0 &&
		sscanf(val_str, "%d", &val_int) == 1 &&
		(ret = clsapi_wifi_set_beacon_interval(ifname, val_int)) < 0) {
		cls_error("can't set beacon interval to %d, error %d", val_int, ret);
		goto exit;
	}

	if (cls_get_value_enable(&cmd_req, CLS_TOK_SGI20, &feature_enable, &conv_err) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf),
			"iw dev %s vendor send 0xD04433 0x02 0x05 0x%02x && cls set %s sgi20 disable",
			ifname, feature_enable, ifname);
		ret = system(tmpbuf);
		PRINT("%s:%d, %s: %s: ret: %d\n",
				__FILE__, __LINE__, __func__, tmpbuf, ret);
		if (ret != 0) {
			cls_error("error: can't set SGI to %d", feature_enable);
			PRINT("%s:%d, %s: error: can't set SGI to %d\n",
				__FILE__, __LINE__, __func__, feature_enable);
			goto exit;
		}
	} else if (conv_err < 0) {
		ret = conv_err;
		goto exit;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SPATIAL_RX_STREAM, val_str, sizeof(val_str)) > 0 &&
		sscanf(val_str, "%d", &val_int) == 1) {
		rx_ss = val_int;
		snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x02 0x06 0x%02x", ifname, rx_ss);
		ret = system(tmpbuf);
		PRINT("%s:%d, %s: %s: ret: %d\n",
				__FILE__, __LINE__, __func__, tmpbuf, ret);
		if (ret != 0) {
			cls_error("can't set SPATIAL_RX_STREAM as %d", rx_ss);
			PRINT("%s:%d, %s: can't set SPATIAL_RX_STREAM as %d\n",
				__FILE__, __LINE__, __func__, rx_ss);
			goto exit;
		}
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SPATIAL_TX_STREAM, val_str, sizeof(val_str)) > 0 &&
		sscanf(val_str, "%d", &val_int) == 1) {
		tx_ss = val_int;
		snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x02 0x07 0x%02x", ifname, tx_ss);
		ret = system(tmpbuf);
		PRINT("%s:%d, %s: %s: ret: %d\n",
				__FILE__, __LINE__, __func__, tmpbuf, ret);
		if (ret != 0) {
			cls_error("can't set SPATIAL_TX_STREAM as %d", rx_ss);
			PRINT("%s:%d, %s: can't set SPATIAL_TX_STREAM as %d\n",
				__FILE__, __LINE__, __func__, tx_ss);
			goto exit;
		}
	}

	if (rx_ss == -1 && tx_ss == -1) {
		/* ignore */
	} else if (tx_ss != rx_ss) {
		cls_error("can't set different nss for rx %d and tx %d", rx_ss, tx_ss);
		ret = -EINVAL;
		goto exit;
	} else if ((ret = cls_set_nss_cap(ifname, clsapi_mimo_ht, rx_ss)) < 0) {
		cls_error("can't set nss to %d, error %d", rx_ss, ret);
		goto exit;
	}

exit:
	if (status != STATUS_INVALID)
		status = ret == 0 ? STATUS_COMPLETE : STATUS_ERROR;

	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

struct cls_qos_desc {
	enum cls_token arg_tok;
	int qos_stream_class;
	int qos_param_id;
};

static
const struct cls_qos_desc cls_qos_table[] = {
	{CLS_TOK_CWMIN_VO, WME_AC_VO, WMMPARAMS_CWMIN},
	{CLS_TOK_CWMIN_VI, WME_AC_VI, WMMPARAMS_CWMIN},
	{CLS_TOK_CWMIN_BE, WME_AC_BE, WMMPARAMS_CWMIN},
	{CLS_TOK_CWMIN_BK, WME_AC_BK, WMMPARAMS_CWMIN},
	{CLS_TOK_CWMAX_VO, WME_AC_VO, WMMPARAMS_CWMAX},
	{CLS_TOK_CWMAX_VI, WME_AC_VI, WMMPARAMS_CWMAX},
	{CLS_TOK_CWMAX_BE, WME_AC_BE, WMMPARAMS_CWMAX},
	{CLS_TOK_CWMAX_BK, WME_AC_BK, WMMPARAMS_CWMAX},
	{CLS_TOK_AIFS_VO, WME_AC_VO, WMMPARAMS_AIFS},
	{CLS_TOK_AIFS_VI, WME_AC_VI, WMMPARAMS_AIFS},
	{CLS_TOK_AIFS_BE, WME_AC_BE, WMMPARAMS_AIFS},
	{CLS_TOK_AIFS_BK, WME_AC_BK, WMMPARAMS_AIFS},
	{CLS_TOK_TxOP_VO, WME_AC_VO, WMMPARAMS_TXOPLIMIT},
	{CLS_TOK_TxOP_VI, WME_AC_VI, WMMPARAMS_TXOPLIMIT},
	{CLS_TOK_TxOP_BE, WME_AC_BE, WMMPARAMS_TXOPLIMIT},
	{CLS_TOK_TxOP_BK, WME_AC_BK, WMMPARAMS_TXOPLIMIT},
	{CLS_TOK_ACM_VO, WME_AC_VO, WMMPARAMS_ACM},
	{CLS_TOK_ACM_VI, WME_AC_VI, WMMPARAMS_ACM},
	{CLS_TOK_ACM_BE, WME_AC_BE, WMMPARAMS_ACM},
	{CLS_TOK_ACM_BK, WME_AC_BK, WMMPARAMS_ACM},
};

void cls_handle_ap_set_qos(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int err_code = 0;
	char ifname_buf[16];
	const char *ifname;
	char param_buf[32];
	int param_val;
	int ret;
	int i;

	int bss = (cmd_tag == CSIGMA_AP_SET_STAQOS_TAG) ? 1 : 2;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		err_code = ret;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));

	ifname = (ret > 0) ? ifname_buf : cls_get_sigma_interface();

	ret = clsapi_wifi_set_option(ifname, clsapi_wmm, 1);
	if (ret < 0) {
		status = STATUS_ERROR;
		err_code = ret;
		goto respond;
	}

	for (i = 0; i < N_ARRAY(cls_qos_table); i++) {
		const struct cls_qos_desc *qos_desc = &cls_qos_table[i];

		*param_buf = 0;
		ret = cls_get_value_text(&cmd_req, qos_desc->arg_tok, param_buf, sizeof(param_buf));

		if (ret > 0) {
			const int acm_flag =
				qos_desc->qos_param_id == WMMPARAMS_ACM ? 1 : 0;

			if (qos_desc->qos_param_id == WMMPARAMS_ACM)
				param_val = (strncasecmp(param_buf, "on", 2) == 0) ? 1 : 0;
			else
				param_val = atoi(param_buf);

			ret = clsapi_wifi_qos_set_param(ifname, bss,
				qos_desc->qos_stream_class,
				qos_desc->qos_param_id, acm_flag, param_val);

			PRINT("%s:%d, %s: bss %d, class %d, param_id %d, value %s, acm_flag %d\n",
				__FILE__, __LINE__, __func__, bss,
				qos_desc->qos_stream_class, qos_desc->qos_param_id,
				param_buf, acm_flag);
			if (ret < 0) {
				cls_error("class %d, param_id %d, value %s, bss %d, error %d",
					qos_desc->qos_stream_class, qos_desc->qos_param_id,
					param_buf, bss, ret);
				status = STATUS_ERROR;
				err_code = ret;
				goto respond;
			}
		}
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, err_code, out_len, out);
}

void defer_set_ofdma_params(char *if_name)
{
	char tmpbuf[64] = {0};

	if (ofdma_val != 0 && ofdma_val != 1 && ofdma_val != 2) {
		cls_error("ofdma_val is invalid\n");
		return;
	}

	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s vendor send 0xD04433 0x01 0x07 0x%02x", if_name, ofdma_val);
	cls_log("%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, tmpbuf);
	system(tmpbuf);

	if (g_numusersofdma) {
		memset(tmpbuf, 0, sizeof(tmpbuf));
		snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s vendor send 0xD04433 0x01 0x06 0x%02x", if_name, g_numusersofdma);
		cls_log("%s:%d, %s\n", __FILE__, __LINE__, __func__, tmpbuf);
		system(tmpbuf);
	}

	memset(if_name, 0, IFNAMSIZ);
}

void cls_handle_ap_config_commit(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = 0;
	int ret;
	int err_code = 0;
	const char *ifname;
	char md5[64];
	char tmpbuf[256];
	FILE *fp;
	unsigned int band_index;
	unsigned int tag_index;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		err_code = ret;
		goto respond;
	}

	for (tag_index = CLS_DUT_FIRST_TAG; tag_index < CLS_DUT_UNKNOWN_TAG; tag_index++) {
		ifname = cls_get_sigma_tag_interface_map(tag_index);
		if (ifname == NULL || !strlen(ifname)) {
			PRINT("%s:%d, %s: ifname is null\n",
				__FILE__, __LINE__, __func__);
			continue;
		}
		band_index = ifname[strlen(ifname) - 1] - '0';
		snprintf(tmpbuf, sizeof(tmpbuf),
			"md5sum /tmp/run/hostapd-phy%d.conf | cut -d' ' -f1",
			band_index);
		fp = popen(tmpbuf, "r");
		if (fp == NULL) {
			status = STATUS_ERROR;
			goto respond;
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
		if (strcasecmp(ifname, "wlan1") == 0)
			sleep(50);
		else
			sleep(10);
	}
	snprintf(tmpbuf, sizeof(tmpbuf),
		"devmem 0x4c410004 32 0 && devmem 0x4c418004 32 0 && \
		devmem 0x4c410000 32 0x400000 && devmem 0x4c418000 32 0x400000 && \
		devmem 0x4c410964 32 0x4000 && devmem 0x4c418964 32 0x4000",
		band_index);
	system(tmpbuf);

	if (0 != g_ofdma_defer_ifname[0])
		defer_set_ofdma_params(g_ofdma_defer_ifname);

	status = STATUS_COMPLETE;
respond:
	cls_dut_make_response_none(cmd_tag, status, err_code, out_len, out);
	if (true == reset_11n_done) /* one 11n case only has one commit normally, we reset the flag for 11n here */
		reset_11n_done = false;
}
void cls_handle_ap_get_mac_address(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	int ret;
	int status;
	int err_code = 0;
	char macaddr[18] = { 0 };
	struct cls_cmd_request cmd_req;
	int wlan_tag;
	int vap_index = 0;
	char ifname_buf[16] = { 0 };
	char dutName[16];
	const char *ifname;


	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len); 
	if (ret != 0) {
		status = STATUS_INVALID;
		err_code = ret;
		goto respond;
	}

	ret = cls_get_value_int(&cmd_req, CLS_TOK_WLAN_TAG, &wlan_tag);
	if (ret > 0) {
		if (wlan_tag < 1) {
			cls_error("invalid wlan tag %d", wlan_tag);
			PRINT("%s:%d, %s: invalid wlan tag %d\n",
				__FILE__, __LINE__, __func__, wlan_tag);
			status = STATUS_ERROR;
			err_code = ret;
			goto respond;
		}
		vap_index = wlan_tag - 1;
	}

	ret = cls_get_value_text(&cmd_req, CLS_TOK_NAME, dutName, sizeof(dutName));
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));
	if (strcasecmp(ifname_buf, "24G") == 0 || strcasecmp(ifname_buf, "2.4G") == 0)
		ifname = cls_get_sigma_vap_interface(vap_index, 0);
	else if (strcasecmp(ifname_buf, "5G") == 0)
		ifname = cls_get_sigma_vap_interface(vap_index, 1);
	else if (strcasecmp(ifname_buf, "6G") == 0)
		ifname = "wlan2";
	else
	{
		status = STATUS_INVALID;
		cls_error("invalid interface %s", ifname_buf);
		PRINT("%s:%d, %s: invalid interface %s\n",
			__FILE__, __LINE__, __func__, ifname_buf);
		goto respond;
	}
	ret = clsapi_interface_get_mac_addr(ifname, macaddr);
	if (ret < 0){
		status = STATUS_ERROR;
		goto respond;
	}
	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_macaddr(cmd_tag, status, err_code, macaddr, out_len, out);
}
void cls_handle_ap_get_parameter(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}
void cls_handle_ap_set_hs2(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}
void cls_handle_ap_deauth_sta(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int err_code = 0;
	int ret;
	char ifname_buf[IFNAMSIZ];
	const char *ifname;
	char tmp_buf[32];
	char gCmdStr[128];
	unsigned char macaddr[IEEE80211_ADDR_LEN];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		err_code = ret;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));
	ifname = (ret > 0) ? ifname_buf : cls_get_sigma_interface();

	ret = cls_get_value_text(&cmd_req, CLS_TOK_STA_MAC_ADDRESS, tmp_buf, sizeof(tmp_buf));
	if (ret <= 0) {
		status = STATUS_ERROR;
		err_code = EINVAL;
		goto respond;
	}

	ret = cls_parse_mac(tmp_buf, macaddr);
	if (ret < 0) {
		cls_log("error: ap_deauth_sta, invalid macaddr");
		status = STATUS_ERROR;
		err_code = EINVAL;
		goto respond;
	}
	snprintf(gCmdStr, sizeof(gCmdStr),
		"hostapd_cli -i %s deauthenticate %02x:%02x:%02x:%02x:%02x:%02x", ifname,
		macaddr[0], macaddr[1],
		macaddr[2], macaddr[3],
		macaddr[4], macaddr[5]);
	ret = system(gCmdStr);
	PRINT("%s:%d, %s: %s: ret: %d\n",
		__FILE__, __LINE__, __func__, gCmdStr, ret);
	if (ret != 0) {
		status = STATUS_ERROR;
		cls_error("error: can't deauthenticate sta");
		PRINT("%s:%d, %s: error: can't deauthenticate sta\n",
			__FILE__, __LINE__, __func__);
		goto respond;
	}
	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, err_code, out_len, out);
}
void cls_handle_ap_set_11d(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}
void cls_handle_ap_set_11h(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}

void cls_handle_validate_ofdma_group(const char *if_name)
{
	if (g_numusersofdma == 0) {
		char tmpbuf[128];
		unsigned int num_clients;
		int ret;

		ret = clsapi_wifi_get_count_associations(if_name, &num_clients);
		if (ret < 0) {
			cls_log("failed clsapi_wifi_get_count_associations");
			return;
		}

		snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_size %s %d",
				if_name, num_clients);
		system(tmpbuf);

		g_numusersofdma = num_clients;
	}
}

static int channel_to_freq(int chan, clsapi_unsigned_int band)
{
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (chan <= 0)
		return 0; /* not supported */
	switch (band) {
	case clsapi_freq_band_2pt4_ghz:
		if (chan == 14)
			return (2484);
		else if (chan < 14)
			return (2407 + chan * 5);
		break;
	case clsapi_freq_band_5_ghz:
		if (chan >= 182 && chan <= 196)
			return (4000 + chan * 5);
		else
			return (5000 + chan * 5);
		break;
	case clsapi_freq_band_6_ghz:
		/* see 802.11ax D6.1 27.3.23.2 */
		if (chan == 2)
			return (5935);
		if (chan <= 233)
			return (5950 + chan * 5);
		break;
	default:
		;
	}

	return 0; /* not supported */
}

static int find_sideband_offset(int radio, int chan)
{
	switch (radio) {
	case clsapi_freq_band_2pt4_ghz: //RADIO_2G_INDEX
		switch (chan) {
		case 1:
		case 2:
		case 3:
		case 4:
			// for 1~4 primary channeles, 2nd channel must be ABOVE primary.
			return 1;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			// for 5~9 primary channeles, 2nd channel could be above or below, we select below.
			return -1;
		case 10:
		case 11:
		case 12:
		case 13:
			// for 10~13 primary channeles, 2nd channel must be BELOW primary.
			return -1;
		default:
			return 0;
		}
		break;

	case clsapi_freq_band_5_ghz: //RADIO_5G_INDEX
		switch (chan) {
		case 36:
		case 44:
		case 52:
		case 60:
		case 100:
		case 108:
		case 116:
		case 124:
		case 132:
		case 140:
		case 149:
		case 157:
			return 1;
		case 40:
		case 48:
		case 56:
		case 64:
		case 104:
		case 112:
		case 120:
		case 128:
		case 136:
		case 144:
		case 153:
		case 161:
			return -1;
		case 165:
		default:
			return 0;
		}
		break;

	default:
		return 0;
	}
}

const int c_chs_5g_40M[]  = {38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159, 167, 175};
const int c_chs_5g_80M[]  = {42, 58, 106, 122, 138, 155, 171};
const int c_chs_5g_160M[] = {50, 114, 163};
const int c_chs_6g_40M[]  = {3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123, 131, 139, 147,
									  155, 163, 171, 179, 187, 195, 203, 211, 219, 227};
const int c_chs_6g_80M[] =  {7, 23, 39, 55, 71, 87, 103, 119,135, 151, 167, 183, 199, 215};
const int c_chs_6g_160M[] = {15, 47, 79, 111, 143, 175, 207};

static int find_central_chan(clsapi_unsigned_int band, uint32_t pri_chan,
										int sec_chan_offset, uint16_t chan_bw)
{
	int i = 0, num_chan = 0;
	const int *center_chans = NULL;

	switch (chan_bw) {
	case 20:
		return pri_chan;

	case 40:
		if (band == clsapi_freq_band_2pt4_ghz) {
			return pri_chan + sec_chan_offset * 2;
		} else if (band == clsapi_freq_band_5_ghz) {
			center_chans = c_chs_5g_40M;
			num_chan = ARRAY_SIZE(c_chs_5g_40M);
		} else if (band == clsapi_freq_band_6_ghz) {
			center_chans = c_chs_6g_40M;
			num_chan = ARRAY_SIZE(c_chs_6g_40M);
		} else {
			cls_error("Invalid parameter--band not supported! band: %d\n", band);
			return -1;
		}

		for (i = 0; i < num_chan; i++) {
			// In 40 MHz (except 2.4GHz bands), the bandwidth "spans" 4 channels (e.g., 36-40),
			// so the center channel is 2 channels away from the start/end.
			if (pri_chan >= center_chans[i] - 2 && pri_chan <= center_chans[i] + 2)
				return center_chans[i];
		}
		break;

	case 80:
		if (band == clsapi_freq_band_5_ghz) {
			center_chans = c_chs_5g_80M;
			num_chan = ARRAY_SIZE(c_chs_5g_80M);
		} else if (band == clsapi_freq_band_6_ghz) {
			center_chans = c_chs_6g_80M;
			num_chan = ARRAY_SIZE(c_chs_6g_80M);
		} else {
			cls_error("Invalid parameter--band not supported! band: %d\n", band);
			return -1;
		}

		for (i = 0; i < num_chan; i++) {
			// In 80 MHz, the bandwidth "spans" 12 channels (e.g., 36-48),
			// so the center channel is 6 channels away from the start/end.
			if (pri_chan >= center_chans[i] - 6 && pri_chan <= center_chans[i] + 6)
				return center_chans[i];
		}
		break;

	case 160:
		if (band == clsapi_freq_band_5_ghz) {
			center_chans = c_chs_5g_160M;
			num_chan = ARRAY_SIZE(c_chs_5g_160M);
		} else if (band == clsapi_freq_band_6_ghz) {
			center_chans = c_chs_6g_160M;
			num_chan = ARRAY_SIZE(c_chs_6g_160M);
		} else {
			cls_error("Invalid parameter--band not supported! band: %d\n", band);
			return -1;
		}

		for (i = 0; i < num_chan; i++) {
			// In 160 MHz, the bandwidth "spans" 28 channels (e.g., 36-64),
			// so the center channel is 14 channels away from the start/end.
			if (pri_chan >= center_chans[i] - 14 && pri_chan <= center_chans[i] + 14)
				return center_chans[i];
		}
		break;

	default:
		cls_error("Invalid parameter--bandwidth not supported! band: %d\n", chan_bw);
		return -1;
	}

	return -1;
}

void do_chan_switch(const char *ifname, int channel, uint16_t bw,
			clsapi_unsigned_int band, const char *phy_mode)
{
	char cmd[128] = {0};
	char mode_cli[8] = {0};
	int sec_chan_offset = 0, freq = 0, center_chan1 = 0, center_freq1 = 0;

	if (strcasecmp(phy_mode, "11ax") == 0)
		strcpy(mode_cli, "he");
	if (strcasecmp(phy_mode, "11ac") == 0)
		strcpy(mode_cli, "vht");
	if (strcasecmp(phy_mode, "11n") == 0)
		strcpy(mode_cli, "ht");

	freq = channel_to_freq(channel, band);
	if (bw <= 20)
		sec_chan_offset = 0;
	else
		sec_chan_offset = find_sideband_offset(band, channel);
	center_chan1 = find_central_chan(band, channel, sec_chan_offset, bw);
	center_freq1 = channel_to_freq(center_chan1, band);

	sprintf(cmd, "hostapd_cli -i %s CHAN_SWITCH %d %d sec_channel_offset=%d center_freq1=%d center_freq2=0 bandwidth=%d blocktx %s",
			ifname, 10, freq, sec_chan_offset, center_freq1, bw, mode_cli);
	cls_log("use csa to switch channel: %s\n", cmd);
	system(cmd);
}

void cls_handle_ap_set_rfeature(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_COMPLETE;
	char val_str[256];
	int ret = 0;
	int channel;
	int bandwidth;
	char mu_dbg_group_sta0[64];
	char mu_dbg_group_sta1[64];
	int feature_enable;
	int feature_val;
	int btmreq_disassoc_imnt = 0;
	int btmreq_term_bit = 0;
	int btmreq_term_dur = 0;
	int btmreq_term_tsf = 0;
	int assoc_disallow = 0;
	char neigh_bssid[MAC_ADDR_STR_LEN];
	int neigh_opclass = 115;
	int neigh_opchan = 36;
	int neigh_pref = 255;
	int conv_err = 0;
	char ifname_buf[16];
	struct cls_dut_config *conf;
	char he_ltf_str[8];
	char he_gi_str[8];
	char ppdutype[16];
	int omi_rxnss;
	int omi_chwidth;
	clsapi_unsigned_int band_value;
	clsapi_unsigned_int bw;
	char tmpbuf[128];
	unsigned int radio_id;

	const char *if_name = cls_get_sigma_interface();
	const char *target_ifname;
	const char *phy_mode;
	int mbssid_enable = 0;
	int mbssid_offset = 0;
	int mbssidset;
	int nontxbssindex;

//	const char *phy_mode = "11ax";

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));
	if_name = (ret > 0) ? ifname_buf : cls_get_sigma_interface();
	conf = cls_dut_get_config(if_name);
	ret = 0;

	target_ifname = if_name;

	/* store the bss term params locally */
	if (cls_get_value_int(&cmd_req, CLS_TOK_BTMREQ_TERMINATION_BIT, &g_bss_term_params.include) > 0
		       && g_bss_term_params.include == 1) {
		if (cls_get_value_int(&cmd_req, CLS_TOK_BTMREQ_TERMINATION_DUR, &g_bss_term_params.duration) > 0) {
			cls_log("set bss termination duration(%d)", g_bss_term_params.duration);
		}

		if (cls_get_value_int(&cmd_req, CLS_TOK_BTMREQ_TERMINATION_TSF, &g_bss_term_params.tsf) > 0) {
			cls_log("set bss termination TSF(%d)", g_bss_term_params.tsf);
		}
	}
	if (cls_get_value_int(&cmd_req, CLS_TOK_BTMREQ_DISASSOC_IMNT, &btm_req.disassoc_imnt) > 0) {
		cls_log("set btm request disassoc imminent bit to %d", btm_req.disassoc_imnt);
	}

	/* assoc disallow is a dynamical configuration, NOT one part of the frame's content from sigma */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_ASSOC_DISALLOW, &assoc_disallow, &conv_err) > 0) {
		g_assoc_disallow = assoc_disallow;
		memset(tmpbuf, 0, 128);
		int index;
		const char *tmp_ifname = NULL;
		for (index = 0; index < CLS_DUT_BANDS_NUM ; index++) {
			tmp_ifname = cls_get_sigma_interface_for_band(index);
			if (tmp_ifname) {
				snprintf(tmpbuf, sizeof(tmpbuf), "hostapd_cli -i %s set mbo_assoc_disallow %d",
							tmp_ifname, g_assoc_disallow);
				system(tmpbuf);
				cls_log("set mbo param: %s", tmpbuf);
			}
		}
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_NEIGHBOR_BSSID,
		neigh_bssid, sizeof(neigh_bssid)) > 0) {
		struct cls_neigh_report_entry *item = NULL;
		cls_log("set steer neighbor bss information ifname:%s", if_name);

		/*
		 * WFA MBO case4.2.5.1: ap_set_rfeature cmd may not includes "Nebor_Op_Class"
		 * and "Nebor_Op_Ch", in this case, do not check and return, rather uses the
		 * default value to add it's own BSS into neighbor
		 */
		cls_add_neighbor(&neigh_list, neigh_bssid, &item);
		if (!item)
			goto respond;
		if (cls_get_value_int(&cmd_req, CLS_TOK_NEIGHBOR_OP_CLASS, &neigh_opclass) <= 0)
			cls_log("neighbor opclass is empty, use default opclass=%d", neigh_opclass);

		if (cls_get_value_int(&cmd_req, CLS_TOK_NEIGHBOR_OP_CH, &neigh_opchan) <= 0)
			cls_log("neighbor opchan is empty, use default opchan=%d", neigh_opchan);

		if (cls_get_value_int(&cmd_req, CLS_TOK_NEIGHBOR_PREF, &neigh_pref) <= 0) {
			cls_error("can't get neighbor prefer");
			status = STATUS_ERROR;
			goto respond;
		}

		ret = mbo_set_neighbor_bss(item, if_name, neigh_opclass, neigh_opchan, neigh_pref);
		if (ret < 0) {
			cls_error("can't set neighbor bss");
			goto respond;
		}
		cls_add_mbo_listen(if_name);
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_TYPE, val_str, sizeof(val_str)) > 0) {
		if (strcasecmp(val_str, "HE") == 0)
			phy_mode = "11ax";
		else if (strcasecmp(val_str, "VHT") == 0)
			phy_mode = cls_is_2p4_interface(if_name) ? "11ng" : "11ac";
	}
	if (cls_get_value_text(&cmd_req, CLS_TOK_CHNUM_BAND, val_str, sizeof(val_str)) > 0 &&
		sscanf(val_str, "%d;%d", &channel, &bandwidth) == 2) {
		clsapi_unsigned_int current_channel;
		cls_log("switch to channel %d, bw %d", channel, bandwidth);
		clsapi_wifi_wait_scan_completes(if_name, CLS_SCAN_TIMEOUT_SEC);
		if (clsapi_wifi_get_chan(if_name, &current_channel, &bw, &band_value) < 0) {
			cls_error("can't get current channel");
			current_channel = 0;
		}
		if (channel != current_channel && (ret = safe_chan_switch(if_name, channel,
					bandwidth, band_value, phy_mode)) < 0) {
			cls_error("can't set channel to %d, bw %d, error %d", channel,
				bandwidth, ret);
			goto respond;
		}
		if (channel != current_channel) /* the above step only store the new ch/bw to hostapd.conf, no other action */
			do_chan_switch(if_name, channel, bandwidth, band_value, phy_mode);
	}
#if 0
	if (cls_get_value_text(&cmd_req, CLS_TOK_NSS_MCS_OPT, val_str, sizeof(val_str)) > 0) {
		if ((ret = cls_set_nss_mcs_opt(if_name, val_str)) < 0) {
			cls_error("can't set nss_mcs_opt to %s, if_name %s, error %d",
					val_str, if_name, ret);
			goto respond;
		}
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_NDPA_STAINFO_MAC, val_str, sizeof(val_str)) > 0) {
		cls_log("set NDPA_STAINFO_MAC to %s", val_str);
		snprintf(tmpbuf, sizeof(tmpbuf), "mu sta0 %.17s", val_str);
		system(tmpbuf);
	}
#endif
	if (cls_get_value_enable(&cmd_req, CLS_TOK_RTS_FORCE, &feature_enable, &conv_err) > 0) {
		if (conf) {
			conf->force_rts = (unsigned char)feature_enable;
			conf->update_settings = 1;
		} else {
			ret = -EFAULT;
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}
#if 0
	/* DYN_BW_SGNL, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_DYN_BW_SGNL, &feature_enable, &conv_err) > 0) {
		if (conf) {
			conf->bws_dynamic = (unsigned char)feature_enable;
			conf->update_settings = 1;
		} else {
			ret = -EFAULT;
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* BW_SGNL, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_BW_SGNL, &feature_enable, &conv_err) > 0) {
		if (conf) {
			conf->bws_enable = (unsigned char)feature_enable;
			conf->update_settings = 1;
		} else {
			ret = -EFAULT;
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* CTS_WIDTH, int (0) */
	if (cls_get_value_int(&cmd_req, CLS_TOK_CTS_WIDTH, &feature_val) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_cts_bw %d",
				if_name, feature_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MU_NDPA_FRAMEFORMAT, &feature_val) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_mu_non_ht %d",
				if_name, feature_val);
		system(tmpbuf);
	}

	if (conf && conf->update_settings) {
		cls_set_rts_settings(if_name, conf);
	}
#endif

	if (cls_get_value_int(&cmd_req, CLS_TOK_TXBANDWIDTH, &feature_val) > 0 &&
		(ret = set_tx_bandwidth(if_name, feature_val, phy_mode)) != 0) {
		cls_error("can't set bandwidth to %d, error %d", feature_val, ret);
		PRINT("%s:%d, %s: can't set bandwidth to %d, error %d\n",
			__FILE__, __LINE__, __func__, feature_val, ret);
		goto respond;
	}

	/* Enable/disable trigger based sounding for SU feedback */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_TRIGGER_TXBF, &feature_enable, &conv_err) > 0){
		snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s vendor send 0xD04433 0x01 0x05 0x%02x",
				if_name, feature_enable);
		PRINT("%s:%d, %s: set trigger_txbf by %s\n",
			__FILE__, __LINE__, __func__, tmpbuf);
		ret = system(tmpbuf);
		if (ret != 0) {
			cls_error("can't set txbf trigger for %s", if_name);
			PRINT("%s:%d, %s: can't set txbf trigger for %s\n",
				__FILE__, __LINE__, __func__, if_name);
			goto respond;
		}
	}

	/* HE LTF+GI */
	if (cls_get_value_text(&cmd_req, CLS_TOK_LTF, he_ltf_str, sizeof(he_ltf_str)) > 0 &&
		cls_get_value_text(&cmd_req, CLS_TOK_GI, he_gi_str, sizeof(he_gi_str)) > 0) {
		ret = cls_set_he_ltf_gi(if_name, he_ltf_str, he_gi_str, ofdma_val);
		if (ret != 0) {
			cls_error("can't set %sus GI with %sus HE-LTF",
								he_gi_str, he_ltf_str);
			PRINT("%s:%d, %s: can't set %sus GI with %sus HE-LTF\n",
				__FILE__, __LINE__, __func__, he_gi_str, he_ltf_str);
			goto respond;
		}
	}
#if 0
	if (cls_get_value_text(&cmd_req, CLS_TOK_RUALLOCTONES, val_str, sizeof(val_str)) > 0) {
		int num_users;
		int rualloctones[4] = {0};

		num_users = sscanf(val_str, "%d:%d:%d:%d",
			&rualloctones[0],
			&rualloctones[1],
			&rualloctones[2],
			&rualloctones[3]);

		cls_log("config ru alloc: [%s], num_users %d, ru_size %d",
							val_str, num_users, rualloctones[0]);
		if (num_users >= 2) {
			char tmpbuf[64];

			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_ru_size %s %d", if_name,
						rualloctones[0] == 26 ? CLS_HE_RU_SIZE_26 :
						rualloctones[0] == 52 ? CLS_HE_RU_SIZE_52 :
						rualloctones[0] == 106 ? CLS_HE_RU_SIZE_106 :
						rualloctones[0] == 242 ? CLS_HE_RU_SIZE_242 :
						rualloctones[0] == 484 ? CLS_HE_RU_SIZE_484 : 255);
			system(tmpbuf);

			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_size %s %d",
										if_name, num_users);
			system(tmpbuf);
		}
	}
#endif
	int ack_policy = -1;

	if (cls_get_value_int(&cmd_req, CLS_TOK_ACKPOLICY, &feature_val) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf),
			"iw dev %s vendor send 0xD04433 0x01 0x08 0x%02x",
			if_name, feature_val);
		PRINT("%s:%d, %s: set ack_policy by %s\n",
			__FILE__, __LINE__, __func__, tmpbuf);
		ret = system(tmpbuf);
		if (ret != 0) {
			cls_error("can't set ack_policy as %d for %s", feature_val, if_name);
			PRINT("%s:%d, %s: can't set ack_policy as %d for %s\n",
				__FILE__, __LINE__, __func__, feature_val, if_name);
			goto respond;
		}
	}
#if 0
	if (cls_get_value_int(&cmd_req, CLS_TOK_OMI_RXNSS, &omi_rxnss) > 0 &&
			cls_get_value_int(&cmd_req, CLS_TOK_OMI_CHWIDTH, &omi_chwidth) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "send_he_opmode %s 0 -r %d -b %d",
							if_name, omi_rxnss + 1, omi_chwidth);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_TRIG_COMINFO_GI_LTF, &feature_val) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_gi_ltf %s %d", if_name, feature_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MPDU_MU_SPACINGFACTOR, &feature_val) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_mmsf %s %d", if_name, feature_val);
		system(tmpbuf);
	}
#endif
	if (cls_get_value_int(&cmd_req, CLS_TOK_DISABLETRIGGERTYPE, &feature_val) > 0) {
		if (feature_val >= 0 && feature_val <= 7) {
			/* basic trigger frame */
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x02 0x%02x",
				if_name, feature_val);
			PRINT("%s:%d, %s: set disable_trigger_type by %s\n",
				__FILE__, __LINE__, __func__, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set disable_trigger_type as %d for %s",
					feature_val, if_name);
				PRINT("%s:%d, %s: can't set disable_trigger_type as %d for %s\n",
					__FILE__, __LINE__, __func__, feature_val, if_name);
				goto respond;
			}
		}
		else
		{
			status = STATUS_INVALID;
			cls_error("error: wrong disable_trigger_type value %d", feature_val);
			PRINT("%s:%d, %s: error: wrong disable_trigger_type value %d\n",
				__FILE__, __LINE__, __func__, feature_val);
			goto respond;
		}
	}

	if (clsapi_wifi_get_parameter(if_name,
				clsapi_wifi_param_multi_bssid,
				&mbssid_enable) < 0)
		mbssid_enable = 0;

	if (mbssid_enable) {
		if (cls_get_value_int(&cmd_req, CLS_TOK_NONTXBSSINDEX, &nontxbssindex) > 0)
			mbssid_offset += nontxbssindex;

		if (cls_get_value_int(&cmd_req, CLS_TOK_MBSSIDSET, &mbssidset) > 0) {
			if (mbssidset == 2)
				mbssid_offset += g_mbssid_set_num == 2 ? 4 : 2;
			else if (mbssidset == 3)
				mbssid_offset += 4;
			else if (mbssidset == 4)
				mbssid_offset += 6;
		}

		target_ifname = cls_ap_get_mbssid_ifname(if_name, mbssid_offset);

	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_TRIGGERTYPE, &feature_val) > 0) {
		if (cls_get_value_text(&cmd_req, CLS_TOK_PPDUTXTYPE,
				ppdutype, sizeof(ppdutype)) > 0) {
			/* select PPDU format for trigger frame */
			//const int use_he_ppdu = (strcasecmp(val_str, "HE-SU") == 0);
			int vendor_value;
			if (strcasecmp(ppdutype, "SU") == 0)
				vendor_value = 0;
			else if (strcasecmp(ppdutype, "MU") == 0)
				vendor_value = 1;
			else if (strcasecmp(ppdutype, "ER-SU") == 0)
				vendor_value = 2;
			else if (strcasecmp(ppdutype, "TB") == 0)
				vendor_value = 3;
			else if (strcasecmp(ppdutype, "Legacy") == 0)
				vendor_value = 4;
			else
			{
				status = STATUS_INVALID;
				cls_error("error: wrong ppdutype %s", ppdutype);
				PRINT("%s:%d, %s:error: wrong ppdutype %s\n",
					__FILE__, __LINE__, __func__, ppdutype);
				goto respond;
			}
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x03 0x%02x",
				if_name, vendor_value);
			PRINT("%s:%d, %s: set ppdu_tx_type=%s by %s\n",
				__FILE__, __LINE__, __func__, ppdutype, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set ppdu_tx_type as %s for %s",
					ppdutype, if_name);
				PRINT("%s:%d, %s: can't set ppdu_tx_type as %s for %s\n",
					__FILE__, __LINE__, __func__, ppdutype, if_name);
				goto respond;
			}
		}

		ret = clsapi_get_radio_from_ifname(if_name, &radio_id);
		if (ret < 0) {
			cls_error("error: couldn't get radio_id, ifname %s, error %d",
					if_name, ret);
			PRINT("%s:%d, %s: error: couldn't get radio_id, ifname %s, error %d\n",
				__FILE__, __LINE__, __func__, if_name, ret);
			goto respond;
		}

		if (feature_val == 0) {
			/* basic trigger frame for UL OFDMA */
			const int is_2p4g = cls_is_2p4_interface(if_name);
			int trig_cominfo_bw;
			int trig_interval;
			int ul_len;
			int ru_sze = is_2p4g ? 1 : 3;
			int fill_bw_ru_size = is_2p4g ? 3 : 5;
			int group_bw = is_2p4g ? 20 : 80;
			int allow_ldpc = 1;
			int ssalloc_ra_ru = 1;
			int force_msta_ba = 0;
			int ret;

			if (group_bw > 20) {
				clsapi_unsigned_int current_bw;

				if (clsapi_wifi_get_bw(if_name, &current_bw) == 0) {
					if (group_bw > current_bw) {
						group_bw = current_bw;
						ru_sze = 1;
						fill_bw_ru_size = 3;
					}
				}
			}
#if 0
			ret = cls_get_value_int(&cmd_req, CLS_TOK_TRIG_COMINFO_BW,
								&trig_cominfo_bw);
			if (ret > 0) {
				if (trig_cominfo_bw == 1) {
					group_bw = 40;
					ru_sze = 2;
					fill_bw_ru_size = 4;
				}
			}

			ret = cls_get_value_text(&cmd_req, CLS_TOK_TRIGGERCODING,
								val_str, sizeof(val_str));
			if (ret > 0) {
				if (strcasecmp(val_str, "BCC") == 0)
					allow_ldpc = 0;
			}

			if (conf->testbed_enable && mbssid_enable && is_2p4g)
				allow_ldpc = 0;

			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_ldpc %s %d",
									if_name, allow_ldpc);
			system(tmpbuf);

			ret = cls_get_value_text(&cmd_req, CLS_TOK_TRIG_USRINFO_RUALLOC,
								val_str, sizeof(val_str));
			if (ret > 0) {
				if (strstr(val_str, "996")) {
					ru_sze = 5;
					fill_bw_ru_size = 5;
				} else if (strstr(val_str, "484:")) {
					ru_sze = 4;
				} else if (strstr(val_str, "106:")) {
					ru_sze = 2;
					fill_bw_ru_size = 4;
				} else if (strstr(val_str, "52:")) {
					ru_sze = 1;
					fill_bw_ru_size = 3;
				} else if (strstr(val_str, "26:")) {
					ru_sze = 0;
					fill_bw_ru_size = 2;
				}
			}
#endif
			ret = cls_get_value_text(&cmd_req, CLS_TOK_ACKTYPE,
								val_str, sizeof(val_str));
			if (ret > 0){
				force_msta_ba = !!strstr(val_str, "M-BA");
				snprintf(tmpbuf, sizeof(tmpbuf),
					"iw dev %s vendor send 0xD04433 0x01 0x01 0x%02x",
					if_name, force_msta_ba);
				PRINT("%s:%d, %s: basic trigger type, set ack_type(M-BA) by %s\n",
					__FILE__, __LINE__, __func__, tmpbuf);
				ret = system(tmpbuf);
				if (ret != 0) {
					cls_error("can't set ack_type(M-BA) for %s", if_name);
					PRINT("%s:%d, %s: can't set ack_type(M-BA) for %s\n",
						__FILE__, __LINE__, __func__, if_name);
					goto respond;
				}
			}
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x04 0x%02x",
				if_name, feature_val);
			PRINT("%s:%d, %s: set trigger_type=%d by %s\n",
				__FILE__, __LINE__, __func__, feature_val, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set trigger_type as %d for %s",
					feature_val, if_name);
				PRINT("%s:%d, %s: can't set trigger_type as %d for %s\n",
					__FILE__, __LINE__, __func__, feature_val, if_name);
				goto respond;
			}

			if (conf->use_ul_mumimo) {
				snprintf(tmpbuf, sizeof(tmpbuf),
					"create_ul_mumimo %s %d 0 1",
					if_name, group_bw);
				ret = system(tmpbuf);
				if (ret != 0) {
					cls_error("can't create_ul_mumimo for %s", if_name);
					PRINT("%s:%d, %s: can't create_ul_mumimo for %s\n",
						__FILE__, __LINE__, __func__, if_name);
					goto respond;
				}
			}
#if 0
			else {
				ret = cls_get_value_int(&cmd_req,
						CLS_TOK_TRIG_USRINFO_SSALLOC_RA_RU, &ssalloc_ra_ru);

				if (mbssid_enable) {
					snprintf(tmpbuf, sizeof(tmpbuf),
							"ofdma set_ul_carousel %s %d", if_name, 0);
					system(tmpbuf);

					/* Increase the inactivity timeout value to 3000 */
					snprintf(tmpbuf, sizeof(tmpbuf),
							"iwpriv %s inact 60000", target_ifname);
					system(tmpbuf);
				}

				snprintf(tmpbuf, sizeof(tmpbuf),
						"create_ul_ofdma %s %d %d %d %d %d %d %s",
						if_name, group_bw, ru_sze, fill_bw_ru_size,
						force_msta_ba, ssalloc_ra_ru, mbssid_enable,
						target_ifname ? target_ifname : "NULL");
				system(tmpbuf);
			}
#endif
#if 0
			ret = cls_get_value_int(&cmd_req, CLS_TOK_TRIG_INTERVAL,
								&trig_interval);
			if (ret > 0) {
				snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_period %s %d",
								if_name, trig_interval);
				system(tmpbuf);
			}

			ret = cls_get_value_enable(&cmd_req, CLS_TOK_NAV_UPDATE, &feature_enable,
							&conv_err);
			if (ret > 0) {
				snprintf(tmpbuf, sizeof(tmpbuf), "ofdma ul_ignore_nav %s %d",
								if_name, !feature_enable);
				system(tmpbuf);
			}

			ret = cls_get_value_int(&cmd_req, CLS_TOK_TRIG_COMINFO_ULLENGTH,
								&ul_len);
			if (ret > 0) {
				snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_lsig_len %s %d",
								if_name, ul_len);
				system(tmpbuf);
			}
#endif
		} else if (feature_val == 1) {
#define NUM_STA	2
			uint8_t sta_mac_addr[NUM_STA][ETH_ALEN];
			int i;
			unsigned int radioid;

			clsapi_get_radio_from_ifname(if_name, &radioid);
			if (conf && conf->dut_enable) {
				/* TC 4.53.1: Need to enable BF to make MU BFRP work */
				snprintf(tmpbuf, sizeof(tmpbuf),
					"iw dev %s vendor send 0xD04433 0x02 0x03 0x01",
					if_name);
				PRINT("%s:%d, %s: make MU BFRP work by %s\n",
					__FILE__, __LINE__, __func__, tmpbuf);
				ret = system(tmpbuf);
				if (ret != 0) {
					cls_error("can't enable BF for %s", if_name);
					PRINT("%s:%d, %s: can't enable BF for %s\n",
						__FILE__, __LINE__, __func__, if_name);
					goto respond;
				}
			} else if (conf && conf->testbed_enable) {
				/* TC 5.81.1: Need to send MU BFRP, create the MU grp manually */
				for (i = 0; i < NUM_STA; i++) {
					ret = clsapi_wifi_get_associated_device_mac_addr(if_name,
							i, sta_mac_addr[i]);
					if (ret < 0)
						cls_error("failed to get sta mac addr, index = %u",
								i);
					else
						cls_error(" assoc sta %d %s\n",
							i, sta_mac_addr[i]);
				}
				snprintf(tmpbuf, sizeof(tmpbuf), "mu %d set "
					MACFILTERINGMACFMT " " MACFILTERINGMACFMT, radioid,
					sta_mac_addr[0][0], sta_mac_addr[0][1], sta_mac_addr[0][2],
					sta_mac_addr[0][3], sta_mac_addr[0][4], sta_mac_addr[0][5],
					sta_mac_addr[1][0], sta_mac_addr[1][1], sta_mac_addr[1][2],
					sta_mac_addr[1][3], sta_mac_addr[1][4],
					sta_mac_addr[1][5]);
				ret = system(tmpbuf);
				if (ret != 0) {
					cls_error("can't create the MU grp manually for %s",
						if_name);
					PRINT("%s:%d, %s: create the MU grp manually for %s\n",
						__FILE__, __LINE__, __func__, if_name);
					goto respond;
				}
			}
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x04 0x%02x",
				if_name, feature_val);
			PRINT("%s:%d, %s: set trigger_type=%d by %s\n",
				__FILE__, __LINE__, __func__, feature_val, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set trigger_type as %d for %s",
					feature_val, if_name);
				PRINT("%s:%d, %s: can't set trigger_type as %d for %s\n",
					__FILE__, __LINE__, __func__, feature_val, if_name);
				goto respond;
			}
		} else if (feature_val == 2) {
			/* MU BAR */
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x04 0x%02x",
				if_name, feature_val);
			PRINT("%s:%d, %s: set trigger_type=%d by %s\n",
				__FILE__, __LINE__, __func__, feature_val, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set trigger_type as %d for %s",
					feature_val, if_name);
				PRINT("%s:%d, %s: can't set trigger_type as %d for %s\n",
					__FILE__, __LINE__, __func__, feature_val, if_name);
				goto respond;
			}
		} else if (feature_val == 3) {
			/* HE MU-RTS */
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x04 0x%02x",
				if_name, feature_val);
			PRINT("%s:%d, %s: set trigger_type=%d by %s\n",
				__FILE__, __LINE__, __func__, feature_val, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set trigger_type as %d for %s",
					feature_val, if_name);
				PRINT("%s:%d, %s: can't set trigger_type as %d for %s\n",
					__FILE__, __LINE__, __func__, feature_val, if_name);
				goto respond;
			}
		} else if (feature_val == 4) {
			/* BSRP */
			snprintf(tmpbuf, sizeof(tmpbuf),
				"iw dev %s vendor send 0xD04433 0x01 0x04 0x%02x",
				if_name, feature_val);
			PRINT("%s:%d, %s: set trigger_type=%d by %s\n",
				__FILE__, __LINE__, __func__, feature_val, tmpbuf);
			ret = system(tmpbuf);
			if (ret != 0) {
				cls_error("can't set trigger_type as %d for %s",
					feature_val, if_name);
				PRINT("%s:%d, %s: can't set trigger_type as %d for %s\n",
					__FILE__, __LINE__, __func__, feature_val, if_name);
				goto respond;
			}
		}
	}
#if 0
	if (cls_get_value_enable(&cmd_req, CLS_TOK_UNSOLICITEDPROBERESP,
						&feature_enable, &conv_err) > 0) {
		int period = CLS_PKTGEN_INTERVAL_DEFAULT_US;

		if (cls_get_value_int(&cmd_req, CLS_TOK_CADENCE_UNSOLICITEDPROBERESP,
						&feature_val) > 0)
			period = IEEE80211_TU_TO_USEC(feature_val);

		if (cls_get_value_int(&cmd_req, CLS_TOK_NONTXBSSINDEX, &nontxbssindex) <= 0)
			cls_dut_handle_unsolprbrsp_frame(target_ifname, feature_enable, period);
	}

	if (cls_get_value_enable(&cmd_req, CLS_TOK_FILSDSCV, &feature_enable, &conv_err) > 0) {
		if (cls_get_value_int(&cmd_req, CLS_TOK_NONTXBSSINDEX, &nontxbssindex) <= 0)
			cls_dut_handle_filsdscv_frame(target_ifname, feature_enable);
	}

	/*For 5.75.1, if MMBSSID is enabled, MUEDCA params should be configured per Trans-VAP*/
	if (cls_get_value_int(&cmd_req, CLS_TOK_HE_TXOPDURRTSTHR, &feature_val) > 0) {
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s txop_rtsthr %d", target_ifname, feature_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMIN_BE, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BE << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMIN_VI, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VI << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMIN_VO, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VO << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMIN_BK, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BK << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMAX_BE, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BE << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMAX_VI, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VI << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMAX_VO, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VO << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_ECWMAX_BK, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BK << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_AIFSN_BE, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BE << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_AIFSN_VI, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VI << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_AIFSN_VO, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VO << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_AIFSN_BK, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BK << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_TIMER_BE, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BE << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_TIMER_VI, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VI << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_TIMER_VO, &feature_val) > 0) {
		uint32_t set_val = WME_AC_VO << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_STA_MUEDCA_TIMER_BK, &feature_val) > 0) {
		uint32_t set_val = WME_AC_BK << 16 | feature_val;

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x%x", target_ifname, set_val);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_TxOP_VI, &feature_val) > 0) {
		cls_log("set STAQOS TXOP_VI to %d", feature_val);
		ret = clsapi_wifi_qos_set_param(if_name,
				WME_AC_VI,
				WMMPARAMS_TXOPLIMIT, 1,
				IEEE80211_TXOP_TO_US(feature_val));
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_TxOP_BE, &feature_val) > 0) {
		cls_log("set STAQOS TXOP_BE to %d", feature_val);
		ret = clsapi_wifi_qos_set_param(if_name,
				WME_AC_BE,
				WMMPARAMS_TXOPLIMIT, 1,
				IEEE80211_TXOP_TO_US(feature_val));
	}

	/* Broadcast TWT Configuration */
	if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_ID, &feature_val) > 0) {
		int dur;
		int man;
		int exp;
		int trig;
		int ann;

		if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_WAKEDUR, &dur) <= 0)
			goto respond;

		if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_INTVMAN, &man) <= 0)
			goto respond;

		if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_INTVEXP, &exp) <= 0)
			goto respond;

		if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_TRIGGER, &trig) <= 0)
			goto respond;

		if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_FLOWTYPE, &ann) <= 0)
			goto respond;

		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s add_flow broadcast %d all", if_name,
							feature_val);
		system(tmpbuf);
		snprintf(tmpbuf, sizeof(tmpbuf),
					"twt %s set_flow broadcast %d all %d %d %d 1 %d 0 %d",
					if_name, feature_val, dur, man, exp,
					!trig, !ann);
		system(tmpbuf);
		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s start_flow broadcast %d all", if_name,
							feature_val);
		system(tmpbuf);

		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s show", if_name);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_PREAMBLEPUNCTURE, &feature_val) > 0) {
		int punc_channel;

		cls_log("set Preamble Puncture to %d", feature_val);
		if (cls_get_value_int(&cmd_req, CLS_TOK_PUNCCHANNEL, &punc_channel) > 0) {
			int punc_mode;

			cls_log("set Puncture channel to %d", punc_channel);
			switch (feature_val) {
			case 4:
				if (punc_channel == 40)
					punc_mode = 2;
				else if (punc_channel == 33)
					punc_mode = 1;
				else
					punc_mode = 0;
				break;
			case 5:
				if ((punc_channel == 41) || (punc_channel == 44))
					punc_mode = 3;
				else if ((punc_channel == 45) || (punc_channel == 48))
					punc_mode = 4;
				else
					punc_mode = 0;
				break;
			default:
				punc_mode = 0;
			}

			cls_log("preamble puncture type to %d", punc_mode);
			snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s prembl_punc 0x%x", if_name,
									punc_mode);
			system(tmpbuf);
			cls_handle_validate_ofdma_group(if_name);

			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_grouping_mode %s manual",
									if_name);
			system(tmpbuf);

			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_grouping_mode %s trivial",
									if_name);
			system(tmpbuf);

			snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ofdma_su_share %s 1 0",
									if_name);
			system(tmpbuf);
		}
	}
#endif	
#if 0
	if (cls_get_value_int(&cmd_req, CLS_TOK_BCTWT_TDOWNALL, &feature_val) > 0) {
		if (feature_val == 1) {
			snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", if_name);
			system(tmpbuf);
		}
	}
#endif
	int sta0_err = cls_get_value_text(&cmd_req, CLS_TOK_MU_DBG_GROUP_STA0,
			mu_dbg_group_sta0, sizeof(mu_dbg_group_sta0));

	int sta1_err = cls_get_value_text(&cmd_req, CLS_TOK_MU_DBG_GROUP_STA1,
			mu_dbg_group_sta1, sizeof(mu_dbg_group_sta1));

	if ((sta0_err > 0) && (sta1_err > 0)) {
		unsigned int assoc_count;

		ret = clsapi_wifi_get_count_associations(if_name, &assoc_count);
		if (ret < 0) {
			cls_log("failed clsapi_wifi_get_count_associations");
			goto respond;
		}

		if (assoc_count >= 2) {
			unsigned char sta0_addr[MACLEN];
			unsigned char sta1_addr[MACLEN];
			char cmd_buf[256];

			ret = cls_parse_mac(mu_dbg_group_sta0, sta0_addr);
			if (ret < 0) {
				cls_log("failed cls_parse_mac(sta0)");
			}

			ret = cls_parse_mac(mu_dbg_group_sta1, sta1_addr);
			if (ret < 0) {
				cls_log("failed cls_parse_mac(sta1)");
			}

			system("mu disable");
			sleep(1);
#if 0
			/* set mode for manual rank */
			snprintf(cmd_buf, sizeof(cmd_buf), "iwpriv %s dsp_dbg_flg_set 2", if_name);
			system(cmd_buf);
			sleep(1);
#endif
			/* set which STA will be used first in sounding sequence */
			snprintf(cmd_buf, sizeof(cmd_buf), "mu sta0 " MACFILTERINGMACFMT,
					sta0_addr[0], sta0_addr[1], sta0_addr[2],
					sta0_addr[3], sta0_addr[4], sta0_addr[5]);
			system(cmd_buf);

			sleep(1);

			/* set rank */
			snprintf(cmd_buf, sizeof(cmd_buf), "mu set "
					MACFILTERINGMACFMT " " MACFILTERINGMACFMT " 1 30",
					sta0_addr[0], sta0_addr[1], sta0_addr[2],
					sta0_addr[3], sta0_addr[4], sta0_addr[5],
					sta1_addr[0], sta1_addr[1], sta1_addr[2],
					sta1_addr[3], sta1_addr[4], sta1_addr[5]);
			system(cmd_buf);

			system("mu enable");
			sleep(2);
			ret = 0;
		}
	} else if ((sta0_err > 0) || (sta1_err > 0)) {
		cls_log("both sta0 & sta1 required");
		ret = -EINVAL;
	}

	respond:
	if (status != STATUS_INVALID)
		status = ret != 0 ? STATUS_ERROR : STATUS_COMPLETE;

	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}
void cls_handle_ap_set_pmf(int len, unsigned char *params, int *out_len, unsigned char *out)
{
	struct cls_dut_response rsp = { 0 };
	rsp.status = STATUS_COMPLETE;
	rsp.clsapi_error = 0;

	/* From test case 8.19:
	This command is used to configure the AP PMF setting. .If an AP device already handles PMF setting through AP_SET_SECURITY,
	this command shall be ignored.*/

	wfaEncodeTLV(CSIGMA_AP_SET_PMF_TAG, sizeof(rsp), (BYTE *) & rsp, out);
	*out_len = WFA_TLV_HDR_LEN + sizeof(rsp);
}
void cls_handle_ap_send_addba_req(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}
void cls_handle_ap_preset_testparameters(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
}

void cls_handle_device_get_info(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char info_buf[256] = {0};
	char version_buf[128] = {0};
	char version[64] = {0};
	//string_64 hw_version = {0};
	static const char vendor[] = "Clourneysemi";
	//FILE *model;
	FILE *fp;
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}
	fp = popen("cat /etc/openwrt_release |grep RELEASE", "r");
	if (fp == NULL) {
		status = STATUS_ERROR;
		goto respond;
	}
	fgets(version_buf, sizeof(version_buf), fp);
	PRINT("%s:%d, %s: release: %s\n", __FILE__, __LINE__, __func__, version_buf);
	pclose(fp);
	char *start = strchr(version_buf, '\'');
	if (start !=NULL) {
		char *end = strchr(start + 1, '\'');
		if (end != NULL) {
			int length = end - start - 1;
			strncpy(version, start + 1, length);
			version[length] = '\0';
			PRINT("%s:%d, %s: version is %s\n", __FILE__, __LINE__, __func__, version);
		}
	}
#if 0
	model = fopen("/mnt/jffs2/model", "r");
	if (model) {
		if (fscanf(model, "%64s\n", hw_version) != 1) {
			cls_log("/mnt/jffs2/model exists, but it might be empty");
		}
		fclose(model);
	}

	if (hw_version[0] == '\0') {
		ret = clsapi_get_board_parameter(clsapi_hw_id, hw_version);
		if (ret < 0) {
			cls_error("can't get HW id, error %d", ret);
			status = STATUS_ERROR;
			goto respond;
		}
	}
#endif
	/* TODO: use hard-coded hw_version for now, fix it when new HW version is available */
	ret = snprintf(info_buf, sizeof(info_buf),
			"vendor,%s,model,%s,version,%s", vendor, "D2K", version);
	if (ret < 0) {
		status = STATUS_ERROR;
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_vendor_info(cmd_tag, status, ret, info_buf, out_len, out);
}
