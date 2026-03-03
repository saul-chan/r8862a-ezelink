/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "clsapi_base.h"
#include "clsapi_wifi.h"
#include "clsapi_common.h"
#include "nl80211_adapter.h"
#include "wpa_adapter.h"

#define CLS_WIFI_RADIO_NAME_0	"radio0"
#define CLS_WIFI_RADIO_NAME_1	"radio1"

#define CLS_WIFI_DEF_SSID		"Clourney"
#define CLS_WIFI_DEF_SSID_5G	"Clourney-5G"

const char *cls_wifi_phyname_fmt = "phy%d";
const char *cls_wifi_primary_ifname_fmt = "wlan%d";
const char *cls_wifi_ifname_fmt = "wlan%d-%d";

#define HOSTAPD_CTRL_IFACE_DIR			"/var/run/hostapd/"
#define WPA_SUPPLICANT_CTRL_IFACE_DIR	"/var/run/wpa_supplicant/"

#define LINUX_KERNEL_IEEE_PATH		"/sys/class/ieee80211/"

#define WIRELESS_CONFIGURATION_PATH		"/etc/config/wireless"

static const char *clsapi_supported_country_code_tbl[] = {
	"CN",
	"US",	// for debug purpose now. If US region is supported later, remove this comment.
};

enum wifi_sec_chan_offset {
	WIFI_SEC_CHAN_OFFSET_BEWLOW = -1,
	WIFI_SEC_CHAN_OFFSET_ABOVE = 1,
	WIFI_SEC_CHAN_OFFSET_NO_SEC = 0,
};

/** Get secondary channel offset by band and channel
 */
int get_sec_chan_offset(enum clsapi_wifi_band band, int primary_chan)
{
	switch (band) {
	case CLSAPI_BAND_2GHZ:
		switch (primary_chan) {
		case 1:
		case 2:
		case 3:
		case 4:
			// for 1~4 primary channeles, 2nd channel must be ABOVE primary.
			return WIFI_SEC_CHAN_OFFSET_ABOVE;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			// for 5~9 primary channeles, 2nd channel could be above or below, we select below.
			return WIFI_SEC_CHAN_OFFSET_BEWLOW;
		case 10:
		case 11:
		case 12:
		case 13:
			// for 10~13 primary channeles, 2nd channel must be BELOW primary.
			return WIFI_SEC_CHAN_OFFSET_BEWLOW;
		default:
			return WIFI_SEC_CHAN_OFFSET_NO_SEC;
		}
		break;

	case CLSAPI_BAND_5GHZ:
		switch (primary_chan) {
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
			return WIFI_SEC_CHAN_OFFSET_ABOVE;
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
			return WIFI_SEC_CHAN_OFFSET_BEWLOW;
		case 165:
		default:
			return WIFI_SEC_CHAN_OFFSET_NO_SEC;
		}
		break;

	default:
		return WIFI_SEC_CHAN_OFFSET_NO_SEC;
	}
}

const int center_chans_5ghz_40M[]  = {38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159, 167, 175};
const int center_chans_5ghz_80M[]  = {42, 58, 106, 122, 138, 155, 171};
const int center_chans_5ghz_160M[] = {50, 114, 163};
const int center_chans_6ghz_40M[]  = {3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123, 131, 139, 147,
	155, 163, 171, 179, 187, 195, 203, 211, 219, 227};

const int center_chans_6ghz_80M[] =  {7, 23, 39, 55, 71, 87, 103, 119,135, 151, 167, 183, 199, 215};
const int center_chans_6ghz_160M[] = {15, 47, 79, 111, 143, 175, 207};

/** Get center frequency (return in channel number) of primary channel with
 * given bandwidth and secondary channel offset.
 * Returns:
 *   < 0: Errors
 *   > 0: channel number of center frequency
 */
static int get_center_freq_chan(const enum clsapi_wifi_band band, const uint32_t pri_chan,
				const enum wifi_sec_chan_offset sec_chan_offset, const enum clsapi_wifi_bw chan_bw)
{
	int i = 0, num_chan = 0;
	const int *center_chans = NULL;

	switch (chan_bw) {
	case CLSAPI_WIFI_BW_20_NOHT:
	case CLSAPI_WIFI_BW_20:
		return pri_chan;

	case CLSAPI_WIFI_BW_40:
		if (band == CLSAPI_BAND_2GHZ) {
			return pri_chan + sec_chan_offset * 2;
		} else if (band == CLSAPI_BAND_5GHZ) {
			center_chans = center_chans_5ghz_40M;
			num_chan = ARRAY_SIZE(center_chans_5ghz_40M);
		} else if (band == CLSAPI_BAND_6GHZ) {
			center_chans = center_chans_6ghz_40M;
			num_chan = ARRAY_SIZE(center_chans_6ghz_40M);
		} else {
			DBG_ERROR("Invalid parameter--band not supported! band: %d\n", band);
			return -1;
		}

		for (i = 0; i < num_chan; i++) {
			// In 40 MHz (except 2.4GHz bands), the bandwidth "spans" 4 channels (e.g., 36-40),
			// so the center channel is 2 channels away from the start/end.
			if (pri_chan >= center_chans[i] - 2 && pri_chan <= center_chans[i] + 2)
				return center_chans[i];
		}
		break;

	case CLSAPI_WIFI_BW_80:
		if (band == CLSAPI_BAND_5GHZ) {
			center_chans = center_chans_5ghz_80M;
			num_chan = ARRAY_SIZE(center_chans_5ghz_80M);
		} else if (band == CLSAPI_BAND_6GHZ) {
			center_chans = center_chans_6ghz_80M;
			num_chan = ARRAY_SIZE(center_chans_6ghz_80M);
		} else {
			DBG_ERROR("Invalid parameter--band not supported! band: %d\n", band);
			return -1;
		}

		for (i = 0; i < num_chan; i++) {
			// In 80 MHz, the bandwidth "spans" 12 channels (e.g., 36-48),
			// so the center channel is 6 channels away from the start/end.
			if (pri_chan >= center_chans[i] - 6 && pri_chan <= center_chans[i] + 6)
				return center_chans[i];
		}
		break;

	case CLSAPI_WIFI_BW_160:
		if (band == CLSAPI_BAND_5GHZ) {
			center_chans = center_chans_5ghz_160M;
			num_chan = ARRAY_SIZE(center_chans_5ghz_160M);
		} else if (band == CLSAPI_BAND_6GHZ) {
			center_chans = center_chans_6ghz_160M;
			num_chan = ARRAY_SIZE(center_chans_6ghz_160M);
		} else {
			DBG_ERROR("Invalid parameter--band not supported! band: %d\n", band);
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
		DBG_ERROR("Invalid parameter--bandwidth not supported! band: %d\n", chan_bw);
		return -1;
	}

	return -1;
}

/** Parse ifname and retrieve radio index and interface/bss index
 * Note: radio_idx and/or if_idx returns -1 means they are NOT scanfed.
 * Returns:
 *   >0: OK
 *   <0: Error
 */
static inline int parse_ifname(const char *ifname, int *radio_idx, int *if_idx)
{
	int scan_cnt;
	int local_phyidx = -1, local_ifidx = -1;

	if (!ifname) {
		DBG_ERROR("Invalid parameter--ifname is not existed.\n");
		return -1;
	}

	scan_cnt = sscanf(ifname, cls_wifi_ifname_fmt, &local_phyidx, &local_ifidx);
	if (scan_cnt <= 0)
		return -1;

	if (radio_idx)
		*radio_idx = local_phyidx;
	if (if_idx)
		*if_idx = local_ifidx;

	return scan_cnt;
}

#ifdef CLSAPI_PLAT_OPENWRT
static struct {
	char section[32];
	char ifname[16];
} vif_info[CLS_MAX_VAP_PER_RADIO];

static void load_vif_info(int phy_idx)
{
	char path[32];
	FILE *fp = NULL;
	int i = 0;

	memset(vif_info, 0, sizeof(vif_info));
	snprintf(path, sizeof(path), "/var/run/vif_info-phy%d.conf", phy_idx);
	fp = fopen(path, "r");
	if (!fp)
		return;

	while (fscanf(fp, "%31s %15s %*s\n", vif_info[i].section, vif_info[i].ifname) == 2)
		++i;

	fclose(fp);
}

static int get_sct_by_ifname_from_vif_info(const char *ifname, char *sct, int len)
{
	int i;

	for (i = 0; i < CLS_MAX_VAP_PER_RADIO; ++i) {
		if (!vif_info[i].ifname[0])
			break;
		if (!strcmp(vif_info[i].ifname, ifname)) {
			if (sct)
				cls_strncpy(sct, vif_info[i].section, len);
			return 1;
		}
	}

	return 0;
}

static int get_ifname_by_sct_from_vif_info(const char *sct, char *ifname, int len)
{
	int i;

	for (i = 0; i < CLS_MAX_VAP_PER_RADIO; ++i) {
		if (!vif_info[i].section[0])
			break;
		if (!strcmp(vif_info[i].section, sct)) {
			if (ifname)
				cls_strncpy(ifname, vif_info[i].ifname, len);
			return 1;
		}
	}

	return 0;
}

/*
 * convert Wi-Fi ifname to section name of uci 'wireless'.
 * OpenWrt ifname generation rules:
 *   o OpenWrt default Wi-Fi ifname = 'wlan<radio_idx>[-if_idx]', omit [-if_idx] if if_idx=0
 *   o if 'ifname' option presented ==> ifname = value of 'ifname' option
 * Inputs:
 *   ifname: Wi-Fi interface name
 *   radio_sct:	buffer to carry the radio section for the ifname; if it's NULL, omit radio section
 *   rdo_sec_len: buffer len of radio_sct
 *   bss_sct: 	buffer to carry the interface/bss section for the ifname; if it's NULL, omit radio section
 *   bss_sec_len: buffer len of bss_sct
 * Returns:
 *   0:  Success and section name in 'radio_sct' and/or ''bss_sct' param
 *   !0: Errors
 */
static int ifname_to_uci_section(const char *ifname, char *radio_sct, const int rdo_sct_len,
				char *bss_sct, const int bss_sct_len)

{
	int radio_idx = -1, if_idx = 0;
	int ret = CLSAPI_OK;
	char radio_name_fmt[] = "radio%d";
	char target_dev[32] = {0};
	string_32 local_bss_sct = {0};
	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	struct uci_element *e = NULL;
	string_32 local_ifname;

	if (parse_ifname(ifname, &radio_idx, NULL) <= 0) {
		bool found = false;

		for (int i = 0; i < 2; i++) {
			radio_idx = i;
			load_vif_info(radio_idx);

			if (get_sct_by_ifname_from_vif_info(ifname, local_bss_sct, sizeof(local_bss_sct))) {
				found = true;
				break;
			}
		}

		if (!found) {
			ret = clsconf_find_section_by_param(CLSCONF_CFG_WIRELESS, "wifi-iface",
					local_bss_sct, 2, "ifname", ifname);
			if (ret < 0)
				return ret;
		}

		goto out;
	}

	sprintf(target_dev, radio_name_fmt, radio_idx);

	load_vif_info(radio_idx);

	if (get_sct_by_ifname_from_vif_info(ifname, local_bss_sct, sizeof(local_bss_sct)))
		goto out;

	ctx = uci_alloc_context();
	if (!ctx) {
		DBG_ERROR("UCI error--uci_alloc_context() fail.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	uci_load(ctx, CLSCONF_CFG_WIRELESS, &pkg);
	if (!pkg) {
		DBG_ERROR("UCI error--uci_load() fail.\n");
		ret = -1;
		goto out;
	}

	uci_foreach_element(&pkg->sections, e) {
		const char *opt_ifname, *opt_dev;
		struct uci_section *s = NULL;

		s = uci_to_section(e);

		if (strcmp(s->type, "wifi-iface") != 0)
			continue;

		opt_dev = uci_lookup_option_string(ctx, s, "device");
		if (!opt_dev || strcmp(opt_dev, target_dev))
			continue;

		opt_ifname = uci_lookup_option_string(ctx, s, "ifname");
		if (!opt_ifname) {
			if (!get_ifname_by_sct_from_vif_info(s->e.name, local_ifname, sizeof(local_ifname))) {
				while (1) {
					if (if_idx == 0)
						sprintf(local_ifname, "wlan%d", radio_idx);
					else
						sprintf(local_ifname, cls_wifi_ifname_fmt, radio_idx, if_idx);
					++if_idx;

					if (!get_sct_by_ifname_from_vif_info(local_ifname, NULL, 0))
						break;
				}
			}
			opt_ifname = local_ifname;
		}

		if (!strcmp(opt_ifname, ifname)) {
			cls_strncpy(local_bss_sct, s->e.name, sizeof(local_bss_sct));
			goto out;
		}
	}

	DBG_ERROR("Invalid data--could not find any UCI section.\n");
	ret = -CLSAPI_ERR_NOT_FOUND;

out:
	if (radio_sct)
		cls_strncpy(radio_sct, target_dev, rdo_sct_len);
	if (bss_sct)
		cls_strncpy(bss_sct, local_bss_sct, sizeof(local_bss_sct));
	if (pkg)
		uci_unload(ctx, pkg);
	if (ctx)
		uci_free_context(ctx);

	return ret;
}

/*
 * Convert wifi-iface section name in uci 'wireless' to Wi-Fi ifname (e.g. wlan0-1).
 * OpenWrt ifname generation rules:
 *	 o OpenWrt default Wi-Fi ifname = 'wlan<radio_idx>[-if_idx]', omit [-if_idx] if if_idx=0
 *	 o if 'ifname' option presented ==> ifname = value of 'ifname' option
 * Inputs:
 *   section_name:	wifi-iface section name in uci 'wireless'
 *   ifname:      	buffer to carry converted Wi-Fi interface name
 * Returns:
 *	 0:  Success and ifname name in 'ifname' param
 *	 !0: Errors
 */
static int uci_wifi_section_name_to_ifname(const char *section_name, string_32 ifname)
{
	int radio_idx = -1, if_idx = 0;
	int ret = CLSAPI_OK;
	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	struct uci_element *e = NULL;
	struct uci_section *s = NULL;
	const char *target_dev = NULL, *opt_ifname, *opt_dev, *opt_disabled;
	string_32 local_ifname;

	if (!section_name || strlen(section_name) <= 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx) {
		DBG_ERROR("Internal error--there is no enough memory, uci_alloc_context() fail.\n");
		return -CLSAPI_ERR_NO_MEM;
	}

	uci_load(ctx, CLSCONF_CFG_WIRELESS, &pkg);
	if (!pkg) {
		DBG_ERROR("UCI error--uci_load() fail.\n");
		ret = -CLSAPI_ERR_UCI;
		goto out;
	}

	uci_foreach_element(&pkg->sections, e) {
		s = uci_to_section(e);
		if (!strcmp(s->e.name, section_name)) {
			target_dev = uci_lookup_option_string(ctx, s, "device");
			break;
		}
	}

	if (!target_dev) {
		ret = -CLSAPI_ERR_INVALID_PHYNAME;
		goto out;
	}

	if (sscanf(target_dev, "radio%d", &radio_idx) != 1 || radio_idx < 0 || radio_idx > CLS_RADIO_NUM) {
		ret = -CLSAPI_ERR_INVALID_PHYNAME;
		goto out;
	}

	load_vif_info(radio_idx);

	uci_foreach_element(&pkg->sections, e) {
		s = uci_to_section(e);

		if (strcmp(s->type, "wifi-iface") != 0)
			continue;

		opt_dev = uci_lookup_option_string(ctx, s, "device");
		if (!opt_dev || strcmp(opt_dev, target_dev))
			continue;

		opt_disabled = uci_lookup_option_string(ctx, s, "disabled");
		if (opt_disabled && !strcmp(opt_disabled, "1"))
			continue;

		opt_ifname = uci_lookup_option_string(ctx, s, "ifname");
		if (!opt_ifname) {
			if (!get_ifname_by_sct_from_vif_info(s->e.name, local_ifname, sizeof(local_ifname))) {
				while (1) {
					if (if_idx == 0)
						sprintf(local_ifname, "wlan%d", radio_idx);
					else
						sprintf(local_ifname, cls_wifi_ifname_fmt, radio_idx, if_idx);
					++if_idx;

					if (!get_sct_by_ifname_from_vif_info(local_ifname, NULL, 0))
						break;
				}
			}
			opt_ifname = local_ifname;
		}

		if (!strcmp(s->e.name, section_name)) {
			cls_strncpy(ifname, opt_ifname, sizeof(string_32));
			goto out;
		}
	}

	DBG_ERROR("Invalid data--could not find any UCI section.\n");
	ret = -CLSAPI_ERR_NOT_FOUND;

out:
	if (pkg)
		uci_unload(ctx, pkg);
	if (ctx)
		uci_free_context(ctx);

	return ret;
}

static inline int phyname_to_radio_name(const char *phyname, string_32 radio_name);
static int uci_wifi_get_mesh_bss_ifname(const char *phyname, struct mesh_bss_info *info)
{
	int ret = CLSAPI_OK, i, j;
	int bh_count = 0, fh_count = 0, bh_sta_count = 0;
	string_32 radio;
	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	struct uci_element *e = NULL;
	struct uci_section *s = NULL;
	struct {
		string_16 device;
		string_32 ssid;
		string_16 ifname;
	}bh_list[16], fh_list[16], bh_sta_list[16];

	ctx = uci_alloc_context();
	if (!ctx) {
		DBG_ERROR("Internal error--there is no enough memory, uci_alloc_context() fail.\n");
		return -CLSAPI_ERR_NO_MEM;
	}

	uci_load(ctx, CLSCONF_CFG_WIRELESS, &pkg);
	if (!pkg) {
		DBG_ERROR("UCI error--uci_load() fail.\n");
		ret = -CLSAPI_ERR_UCI;
	}

	uci_foreach_element(&pkg->sections, e) {
		s = uci_to_section(e);
		if (strcmp(s->type, "wifi-iface") != 0)
			continue;

		const char *multi_ap = uci_lookup_option_string(ctx, s, "multi_ap");
		if (!multi_ap)
			continue;

		const char *device = uci_lookup_option_string(ctx, s, "device");
		const char *ssid = uci_lookup_option_string(ctx, s, "ssid");
		const char *mode = uci_lookup_option_string(ctx, s, "mode");
		string_32 ifname = { 0 };

		uci_wifi_section_name_to_ifname(s->e.name, ifname);

		if (!strcmp(multi_ap, "1")) {
			if (!strcmp(mode, "ap")) {
				cls_strncpy(bh_list[bh_count].device, device, sizeof(bh_list[bh_count].device));
				cls_strncpy(bh_list[bh_count].ssid, ssid, sizeof(bh_list[bh_count].ssid));
				cls_strncpy(bh_list[bh_count].ifname, ifname, sizeof(bh_list[bh_count].ifname));
				bh_count++;
			} else if (!strcmp(mode, "sta")) {
				cls_strncpy(bh_sta_list[bh_sta_count].device, device, sizeof(bh_sta_list[bh_sta_count].device));
				cls_strncpy(bh_sta_list[bh_sta_count].ifname, ifname, sizeof(bh_sta_list[bh_sta_count].ifname));
				bh_sta_count++;
			}
		} else if (!strcmp(multi_ap, "2")) {
			const char *bh_ssid = uci_lookup_option_string(ctx, s, "multi_ap_backhaul_ssid");
			if (!bh_ssid)
				continue;

			cls_strncpy(fh_list[fh_count].device, device, sizeof(fh_list[fh_count].device));
			cls_strncpy(fh_list[fh_count].ssid, bh_ssid, sizeof(fh_list[fh_count].ssid));
			cls_strncpy(fh_list[fh_count].ifname, ifname, sizeof(fh_list[fh_count].ifname));
			fh_count++;
		}
	}

	if(phyname_to_radio_name(phyname, radio)) {
		ret = -CLSAPI_ERR_INVALID_PHYNAME;
		goto out;
	}

	for (i = 0; i < fh_count; i++) {
		for (j = 0; j < bh_count; j++) {
			if (!strcmp(fh_list[i].ssid, bh_list[j].ssid) &&
				!strcmp(fh_list[i].device, bh_list[j].device) &&
				!strcmp(bh_list[j].device, radio)) {
				cls_strncpy(info->fh_ifname, fh_list[i].ifname, sizeof(info->fh_ifname));
				cls_strncpy(info->bh_ifname, bh_list[j].ifname, sizeof(info->bh_ifname));
			}
		}
	}

	for (i = 0; i < bh_sta_count; i++) {
		if (!strcmp(bh_sta_list[i].device, radio)) {
			cls_strncpy(info->bh_sta_ifname, bh_sta_list[i].ifname, sizeof(info->bh_sta_ifname));
			goto out;
		}
	}

	ret = -CLSAPI_ERR_NOT_FOUND;
out:
	if (pkg)
		uci_unload(ctx, pkg);
	if (ctx)
		uci_free_context(ctx);

	return ret;
}

#endif //#ifdef CLSAPI_PLAT_OPENWRT

/** Get section name in "wireless" from ifname
 * Inputs:
 *   ifname:      Wi-Fi interface name
 *   radio_sct:   buffer to carry the radio section for the ifname; if it's NULL, omit radio section
 *   rdo_sec_len: buffer len of radio_sct
 *   bss_sct:     buffer to carry the interface/bss section for the ifname; if it's NULL, omit radio section
 *   bss_sec_len: buffer len of bss_sct
 * Returns:
 *   0:  Success and section name in 'radio_sct' and/or ''bss_sct' param
 *   !0: Errors
 */
static int clsconf_ifname_to_section(const char *ifname, char *radio_sct, const int rdo_sct_len,
				char *bss_sct, const int bss_sct_len)

{
#ifdef CLSAPI_PLAT_OPENWRT
	return ifname_to_uci_section(ifname, radio_sct, rdo_sct_len, bss_sct, bss_sct_len);
#else
	#error No platform defined
#endif
}

#define clsconf_ifname_to_radio_section(ifname, radio_sct, rdo_sct_len) \
			clsconf_ifname_to_section(ifname, radio_sct, rdo_sct_len, NULL, 0)

#define clsconf_ifname_to_bss_section(ifname, bss_sct, bss_sct_len) \
				clsconf_ifname_to_section(ifname, NULL, 0, bss_sct, bss_sct_len)

/** Get Wi-Fi ifname from CLS-Conf section name.
 * Inputs:
 *   section_name:	section name in CLS-Conf
 *   ifname:      	Wi-Fi interface name
 * Returns:
 *	 0:  Success and ifname name in 'ifname' param
 *	 !0: Errors
 */
static int clsconf_wifi_section_name_to_ifname(const char *section_name, string_32 ifname)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_wifi_section_name_to_ifname(section_name, ifname);
#else
	#error No platform defined
#endif
}

/** Get basic information in Mesh mode.
 * Inputs:
 *   info: mesh bss information
 * Returns:
 *	 0:  Success and basic information in 'info' param
 *	 !0: Errors
 */
static int clsconf_wifi_mesh_bss_ifname(const char* phyname, struct mesh_bss_info *info)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_wifi_get_mesh_bss_ifname(phyname, info);
#else
	return -CLSAPI_ERR_NOT_SUPPORTED;
#endif
}


/** Return phy idx from phyname
 * Valid phy name is "phyN", where N is 0~(CLS_RADIO_NUM - 1). E.g. "phy0", "phy1".
 * Returns:
 *   >= 0: phyname passed validation and return it's phy idx
 *   < 0:  fails in validation
 */
static inline int phyname_to_idx(const char *phyname)
{
	int local_phy_idx = -1;

	if (sscanf(phyname, cls_wifi_phyname_fmt, &local_phy_idx) != 1) {
		DBG_ERROR("Invalid parameter--phyname is not valid.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	if (local_phy_idx < 0 || local_phy_idx >= CLS_RADIO_NUM) {
		DBG_ERROR("Invalid parameter--radio id is out of the range.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	return local_phy_idx;
}

static int ifname_to_phy(const char *ifname, clsapi_ifname phyname, int *phy_idx)
{
	char phyname_path[256], *str_phy;

	if (!phyname) {
		DBG_ERROR("Invalid parameter--radio id is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	snprintf(phyname_path, sizeof(phyname_path), "/sys/class/net/%s/phy80211/name", ifname);
	str_phy = get_one_line_from_file(phyname_path);
	if (str_phy == NULL) {
		DBG_DEBUG("Invalid parameter--ifname '%s' is not ready yet, skip.\n", ifname);
		return -CLSAPI_ERR_NOT_FOUND;
	}

	cls_strncpy(phyname, str_phy, sizeof(clsapi_ifname));

	if (phy_idx)
		*phy_idx = phyname_to_idx(phyname);

	return CLSAPI_OK;
}

/* Get Wi-Fi interface names of the whole device, among all radios.
 * Returns:
 *   >= 0: number of interface names
 *   <  0: Errors
 */
static int _get_all_ifnames(const enum clsapi_wifi_iftype iftype, string_32 *ifname_list, const int list_len)
{
	struct dirent **ifname_dirent = NULL;
	int intf_dir_cnt = 0, ifname_cnt = 0;
	char *intf_dir = NULL, *ifname = NULL;

	if (!ifname_list || list_len < 1)
		return -CLSAPI_ERR_NULL_POINTER;

	if (iftype == CLSAPI_WIFI_IFTYPE_AP)
		intf_dir = HOSTAPD_CTRL_IFACE_DIR;
	else if (iftype == CLSAPI_WIFI_IFTYPE_STA)
		intf_dir = WPA_SUPPLICANT_CTRL_IFACE_DIR;
	else {
		DBG_DEBUG("Invalid parameter--interface type is not supported.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	intf_dir_cnt = scandir(intf_dir, &ifname_dirent, ignore_dots_dir_filter, alphasort);
	if (intf_dir_cnt < 0)
		return -CLSAPI_ERR_HOSTAPD;

	for (int i = 0; i < intf_dir_cnt; i++) {
		// ignore "global"
		ifname = ifname_dirent[i]->d_name;
		if (strcmp(ifname, "global") == 0) {
			free(ifname_dirent[i]);
			continue;
		}

		if (ifname_cnt < list_len)
			cls_strncpy(ifname_list[ifname_cnt++], ifname, sizeof(string_32));
		free(ifname_dirent[i]);
	}
	free(ifname_dirent);

	return ifname_cnt;
}

/** Validate Wi-Fi ifname, and/or return band of it.
 * Validate ifname is in format of "wlanM<-N>", where M is 0~(CLS_RADIO_NUM - 1),
 * N is 1~(CLS_MAX_VAP_PER_RADIO - 1). E.g. "phy0", "phy1".
 * Returns:
 *   CLSAPI_OK: pass validation and is_primary and band is set when they are non-NULL.
 *   others:        fail in validation
 */
static int _validate_ifname(const char *ifname, enum clsapi_wifi_band *band, enum clsapi_wifi_iftype *iftype)
{
	int ret = CLSAPI_OK, found = 0, local_phyidx = -1;
	string_256 wpa_ifname_path;
	clsapi_ifname phyname;
	struct stat dummy_buf;

	if (!ifname || strcmp(ifname, "global") == 0) {
		DBG_ERROR("Invalid parameter--no ifname or ifname is global.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	snprintf(wpa_ifname_path, sizeof(wpa_ifname_path), "%s/%s", HOSTAPD_CTRL_IFACE_DIR, ifname);
	if (stat(wpa_ifname_path, &dummy_buf) == 0) {
		if (iftype)
			*iftype = CLSAPI_WIFI_IFTYPE_AP;
		found = 1;
	}

	if (found == 0) {
		snprintf(wpa_ifname_path, sizeof(wpa_ifname_path), "%s/%s", WPA_SUPPLICANT_CTRL_IFACE_DIR, ifname);
		if (stat(wpa_ifname_path, &dummy_buf) == 0) {
			if (iftype)
				*iftype = CLSAPI_WIFI_IFTYPE_STA;
			found = 1;
		}
	}

	if (found == 0) {
		DBG_ERROR("Invalid parameter--ifname: %s is not found.\n", ifname);
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	ret = ifname_to_phy(ifname, phyname, &local_phyidx);
	if (ret < 0)
		return -CLSAPI_ERR_UNKNOWN_ERROR;

	if (band)
		*band = RADIO_IDX_TO_BAND(local_phyidx);

	return CLSAPI_OK;
}

#define validate_ifname(ifname)	_validate_ifname(ifname, NULL, NULL)

/** Validate phy name by check existence of /sys/class/ieee80211/<phyname>.
 * Returns:
 *   CLSAPI_OK: pass validation
 *   others:    fail in validation
 */
static inline int validate_phyname(const char *phyname)
{
	string_256 phyname_path = {0};
	struct stat dummy_buf;

	if (!phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	snprintf(phyname_path, sizeof(phyname_path), "/sys/class/ieee80211/%s", phyname);
	if (stat(phyname_path, &dummy_buf) == 0)
		return CLSAPI_OK;
	else
		return -CLSAPI_ERR_INVALID_PHYNAME;
}

#define get_phy_band(phyname)	RADIO_IDX_TO_BAND(phyname_to_idx(phyname))

/** Validate phyname (e.g. "phy0") and return it's radio name (e.g. "radio0").
 * Returns:
 *   CLSAPI_OK: pass validation and return radio name in radio_name
 *   others:    Errors
 */
static inline int phyname_to_radio_name(const char *phyname, string_32 radio_name)
{
	int phy_idx = phyname_to_idx(phyname);

	if (phy_idx < 0)
		return phy_idx;

	sprintf(radio_name, "radio%d", phy_idx);

	return CLSAPI_OK;
}

/* Get radio name of the VAP attached to.
 * Returns:
 *   0: OK and radio name "phy%d" in phyname
 *   !0: Errors
 */
static inline int get_phyname_by_ifname(const char *ifname, string_32 phyname)
{
	int ret = CLSAPI_OK, phy_idx = -1;

	if (!ifname || !phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = parse_ifname(ifname, &phy_idx, NULL);
	if (ret <= 0) {
		DBG_ERROR("Invalid parameter--could not parse interface name.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}
	if (phy_idx < 0 || phy_idx >= CLS_RADIO_NUM) {
		DBG_ERROR("Invalid parameter--phy radio is out of the range.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	sprintf(phyname, cls_wifi_phyname_fmt, phy_idx);

	return CLSAPI_OK;
}

/* Get primary interface name of the ifname.
 * Returns:
 *   0: OK and primary ifname in primary_ifname
 *   !0: Errors
 */
static inline int get_primary_ifname_by_ifname(const char *ifname, clsapi_ifname primary_ifname)
{
	int ret = CLSAPI_OK, phy_idx = -1;

	if (!ifname || !primary_ifname) {
		DBG_ERROR("Invalid parameter--no ifname or no primary ifname.\n", ifname);
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = parse_ifname(ifname, &phy_idx, NULL);
	if (ret <= 0)
		return -CLSAPI_ERR_INVALID_IFNAME;

	if (phy_idx < 0 || phy_idx >= CLS_RADIO_NUM) {
		DBG_ERROR("Invalid ifname--phy ID is invalid.\n", ifname);
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	sprintf(primary_ifname, cls_wifi_primary_ifname_fmt, phy_idx);

	return CLSAPI_OK;
}

/* Filter interface names by phy and append to ifname_list
 * Returns:
 *	 >= 0: next available index in ifname_list
 *	 <	0: Errors
 */
static int filter_ifname_by_phy(const string_32 *all_ifnames, const int all_cnt, const char *phyname,
	string_32 *ifname_list, int start_idx, int list_len)
{
	int i = 0, local_idx = start_idx;
	string_512 phyname_path;
	char *str_phy = NULL;

	for (i = 0; i < all_cnt; i++) {
		// check if interface is based on target phy
		snprintf(phyname_path, sizeof(phyname_path), "/sys/class/net/%s/phy80211/name", all_ifnames[i]);
		str_phy = get_one_line_from_file(phyname_path);
		if (str_phy == NULL) {
			DBG_DEBUG("ifname '%s' is not ready yet, skip.\n", all_ifnames[i]);
			continue;
		}

		if (strcmp(str_phy, phyname) == 0) {
			// iterface is based on target phy
			if (local_idx < list_len)
				cls_strncpy(ifname_list[local_idx++], all_ifnames[i], sizeof(string_32));
		}
	}

	return local_idx;
}

/* Get ifname list under hostapd/wpa_supplicant ctrl iface, except global.
 * Returns:
 *	 >= 0: interface names added to ifname_list
 *	 <	0: Errors
 */
int local_wifi_get_ifnames(const char *phyname, enum clsapi_wifi_iftype iftype, string_32 *ifname_list,
	int start_idx, int list_len)
{
	int ret = CLSAPI_OK;
	int all_ifname_cnt = 0, intf_cnt = 0;
	string_32 local_ifnames[CLS_MAX_VAP_PER_RADIO * CLS_RADIO_NUM];

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	if (!ifname_list || list_len < 1) {
		DBG_ERROR("Invalid parameter--list_len less than 1 or ifname_list is null.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = _get_all_ifnames(iftype, local_ifnames, ARRAY_SIZE(local_ifnames));
	if (ret < 0)
		return ret;
	all_ifname_cnt = ret;

	ret = filter_ifname_by_phy(local_ifnames, all_ifname_cnt, phyname, ifname_list, start_idx, list_len);
	if (ret < 0)
		return ret;
	intf_cnt = ret;

	return intf_cnt;
}

static inline int validate_ap_ifname(const char *ifname)
{
	enum clsapi_wifi_iftype wifi_iftype = CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE;
	int ret = _validate_ifname(ifname, NULL, &wifi_iftype);

	if (ret < 0)
		return ret;

	if (wifi_iftype == CLSAPI_WIFI_IFTYPE_AP)
		return CLSAPI_OK;
	else
		return -CLSAPI_ERR_ONLY_ON_AP;
}

static inline int is_interface_up(const char *ifname)
{
	int sockfd;
	struct ifreq ifr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		DBG_ERROR("Internal error--socket creation failed.\n");
		return -CLSAPI_ERR_INTERNAL_ERR;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
		DBG_ERROR("Internal error-- ioctrl failed.\n");
		close(sockfd);
		return -CLSAPI_ERR_INTERNAL_ERR;
	}

	close(sockfd);

	return (ifr.ifr_flags & IFF_UP) ? 1 : 0;
}

/* Validate phyname and get primary interface name (the first interface name) by phyname.
 * The primary interface of a radio is the smallest ifindex interface based on the radio.
 * Returns:
 *   0: OK and primary ifname in primary_ifname
 *   !0: Errors
 */
static inline int phyname_to_primary_ifname(const char *phyname, clsapi_ifname primary_ifname)
{
	int ret = CLSAPI_OK, i = 0;
	string_32 ifnames[CLS_MAX_VAP_PER_RADIO] = {0};
	int list_len = ARRAY_SIZE(ifnames), start_idx = 0, intf_num = 0;
	char *str_ifindex = NULL;
	string_512 ifindex_path;
	int smallest_ifindex = -1, int_ifindex = -1;
	clsapi_ifname smallest_ifname = {0};

	if (!phyname || !primary_ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = validate_phyname(phyname);
	if (ret)
		return -CLSAPI_ERR_INVALID_PHYNAME;

	// get AP ifnames of target phyname
	ret = local_wifi_get_ifnames(phyname, CLSAPI_WIFI_IFTYPE_AP, ifnames, start_idx, list_len);
	if (ret < 0)
		return ret;
	start_idx = ret;

	// get STA ifnames of target phyname
	ret = local_wifi_get_ifnames(phyname, CLSAPI_WIFI_IFTYPE_STA, ifnames, start_idx, list_len);
	if (ret < 0)
		return ret;
	intf_num = ret;

	// find the smallest ifindex
	for (i = 0; i < intf_num; i++) {
		if (!is_interface_up(ifnames[i]))
			continue;

		snprintf(ifindex_path, sizeof(ifindex_path), "/sys/class/net/%s/ifindex", ifnames[i]);
		str_ifindex = get_one_line_from_file(ifindex_path);
		if (str_ifindex == NULL) {
			DBG_ERROR("Invalid parameter--could not find any interface name.\n");
			return -CLSAPI_ERR_INVALID_IFNAME;
		}

		int_ifindex = atoi(str_ifindex);
		if (smallest_ifindex == -1 || int_ifindex < smallest_ifindex) {
			smallest_ifindex = int_ifindex;
			cls_strncpy(smallest_ifname, ifnames[i], sizeof(smallest_ifname));
		}
	}
	if (smallest_ifindex > -1)
		cls_strncpy(primary_ifname, smallest_ifname, sizeof(clsapi_ifname));

	return CLSAPI_OK;
}

/* Validate if radio is in AP mode. There are 3 modes for a radio:
 *   - AP mode:       All interfaces on this radio are AP interface.
 *   - STA mode:      There is only one interface (wlan0) on this radio, and wlan0 is STA interface.
 *   - Repeater mode: There are multiple interfaces on this radio, wlan0 is STA interface, and other
 *                    interfaces are AP interface.
 * Returns:
 *   0: OK
 *   !0: Errors
 */
static int validate_ap_mode(const char *phyname)
{
	clsapi_ifname primary_ifname = {0};
	enum clsapi_wifi_iftype wifi_iftype = CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE;
	int ret = phyname_to_primary_ifname(phyname, primary_ifname);

	if (ret < 0)
		return ret;

	ret = clsapi_wifi_get_iftype(primary_ifname, &wifi_iftype);
	if (ret < 0)
		return ret;

	if (wifi_iftype != CLSAPI_WIFI_IFTYPE_AP) {
		DBG_ERROR("Invalid parameter--interface type is not AP mode.\n");
		return -CLSAPI_ERR_NEED_AP_MODE;
	}

	return CLSAPI_OK;
}

/** Validate channel number with given band and bw.
 * Returns:
 *   CLSAPI_OK: pass validation
 *   others:        fail in validation
 */
static int validate_channel(const char *phyname, const uint8_t channel, const enum clsapi_wifi_band band, const enum clsapi_wifi_bw bw)
{
	uint8_t local_channels[255] = {0}, local_chan_len = ARRAY_SIZE(local_channels);
	clsapi_ifname primary_ifname = {0};
	int ret = CLSAPI_OK;
	bool found = false;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = nl80211_get_chan_list(primary_ifname, band, bw, local_channels, &local_chan_len);
	if (ret < 0)
		return ret;

	if (channel == 0)
		found = true;

	for (int i = 0; i < local_chan_len && local_channels[i] != 0; i++)
		if (channel == local_channels[i]) {
			found = true;
			break;
		}

	if (found)
		return CLSAPI_OK;
	else {
		DBG_ERROR("Invalid parameter--channel not found.\n");
		return -CLSAPI_ERR_INVALID_CHANNEL;
	}
}

/** Check if given band is supported by Wi-Fi interface/radio.
 * Returns:
 *   CLSAPI_OK: pass validation
 *   others:        fail in validation
 */
static int validate_band(const char *phyname, enum clsapi_wifi_band band)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band *bands = NULL;
	int bands_len = 0;
	bool found = 0;

	ret = clsapi_wifi_get_supported_bands(phyname, &bands);
	if (ret < 0)
		return ret;

	bands_len = ret;
	if (bands_len > 0 && bands == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int j = 0; j < bands_len; j++) {
		if (band == bands[j]) {
			found = 1;
			break;
		}
	}

	if (bands)
		free(bands);

	if (found == 0) {
		DBG_ERROR("Invalid parameter--band is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return CLSAPI_OK;
}

/** Validate hwmode (operating standard, IEEE 802.11a/b/g/n/ac/ax/be) with given band.
 * Returns:
 *   CLSAPI_OK: pass validation
 *   others:        fail in validation
 */
static int validate_hwmode(enum clsapi_wifi_hwmode hwmode, const enum clsapi_wifi_band band)
{
	if (band == CLSAPI_BAND_2GHZ) {
		if (hwmode & CLSAPI_HWMODE_IEEE80211_A || hwmode & CLSAPI_HWMODE_IEEE80211_AC)
			return -CLSAPI_ERR_INVALID_HWMODE;
	} else if (band == CLSAPI_BAND_5GHZ) {
		if (hwmode & CLSAPI_HWMODE_IEEE80211_B)
			return -CLSAPI_ERR_INVALID_HWMODE;
	} else {
		DBG_ERROR("Invalid parameter--band is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return CLSAPI_OK;
}

static inline int validate_bw(const enum clsapi_wifi_hwmode hwmode, const enum clsapi_wifi_band band, const enum clsapi_wifi_bw bw)
{
	switch (hwmode) {
	case CLSAPI_HWMODE_IEEE80211_A:
	case CLSAPI_HWMODE_IEEE80211_B:
	case CLSAPI_HWMODE_IEEE80211_G:
		// legacy, fixed NON_HT_20
		if (bw != CLSAPI_WIFI_BW_20_NOHT)
			return -CLSAPI_ERR_INVALID_BW;

	case CLSAPI_HWMODE_IEEE80211_N:
		if (bw != CLSAPI_WIFI_BW_20 && bw != CLSAPI_WIFI_BW_40) {
			DBG_ERROR("Invalid parameter--bandwidth is not supported in IEEE80211N.\n");
			return -CLSAPI_ERR_INVALID_BW;
		}
		break;

	case CLSAPI_HWMODE_IEEE80211_AC:
		if (bw != CLSAPI_WIFI_BW_20 && bw != CLSAPI_WIFI_BW_40 &&
				bw != CLSAPI_WIFI_BW_80 && bw != CLSAPI_WIFI_BW_160) {
			DBG_ERROR("Invalid parameter--bandwidth is not supported in IEEE80211AC.\n");

			return -CLSAPI_ERR_INVALID_BW;
		}
		break;

	case CLSAPI_HWMODE_IEEE80211_AX:
		if (band == CLSAPI_BAND_2GHZ) {
			if (bw != CLSAPI_WIFI_BW_20 && bw != CLSAPI_WIFI_BW_40) {
				DBG_ERROR("Invalid parameter--bandwidth is not supported in IEEE80211AX.\n");

				return -CLSAPI_ERR_INVALID_BW;
			}
		} else {
			if (bw != CLSAPI_WIFI_BW_20 && bw != CLSAPI_WIFI_BW_40 &&
				bw != CLSAPI_WIFI_BW_80 && bw != CLSAPI_WIFI_BW_160) {
				DBG_ERROR("Invalid parameter--bandwidth is not supported in IEEE80211AX.\n");
				return -CLSAPI_ERR_INVALID_BW;
			}
		}
		break;
	default:
		DBG_ERROR("Invalid parameter--hardware mode is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return CLSAPI_OK;
}

static inline int validate_encryption(const enum clsapi_wifi_encryption encryption)
{
	char * str_encryption = encryption_enum2str(encryption);

	if (str_encryption == NULL || strcmp(str_encryption, STR_CLSAPI_WIFI_ENCRYPTION_UNKNOWN) == 0) {
		DBG_ERROR("Invalid parameter--encryption is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return CLSAPI_OK;
}

/* Validate ASCII passphase against authentication mode.
 * If passphrase is provided, validate the length.
 * Returns:
 *   CLSAPI_OK:   passed the validation
 *   -CLSAPI_ERR_: failed the validation
 */
static inline int validate_passphrase(const char *ifname, const char *passphrase)
{
	if (passphrase && (strlen(passphrase) < 8 || strlen(passphrase) > 64)) {
		DBG_ERROR("Invalid parameter--passphrase is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM_LEN;
	}

	return CLSAPI_OK;
}

static inline int validate_wep_key_mode(const char *ifname)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret)
		return ret;

	if (encryption == CLSAPI_WIFI_ENCRYPTION_WEP_MIXED)
		return CLSAPI_OK;

	DBG_ERROR("Invalid parameter--encryption is not supported.\n");
	return -CLSAPI_ERR_NOT_SUPPORTED;
}

/* Convert htmode in format like "HT40+" to enum hwmode and bw
 * Inputs:
 *   htmode: htmode, possible values are "HT20", "HT40", "HT40-", "VHT20", "VHT40", "VHT80", "VHT160",
 *           "HE20", "HE40", "HE80", "HE160", "NOHT"
 * Returns:
 *   CLSAPI_OK: OK
 *   Others: Error
 */
static inline int htmode_to_hwmode_bw(const char *htmode, enum clsapi_wifi_hwmode *hwmode, enum clsapi_wifi_bw *bw)
{
	int scan_cnt = 0, local_bw = 0;
	char str_hwmode[32] = "", sec_offset = 0;

	*hwmode = 0;
	*bw = CLSAPI_WIFI_BW_DEFAULT;

	scan_cnt = sscanf(htmode, "%[^0-9+-]%d%c", str_hwmode, &local_bw, &sec_offset);
	if (scan_cnt == 0) {
		DBG_ERROR("Invalid parameter--htmode is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	if (strcmp(str_hwmode, "NOHT") == 0)
		*hwmode = CLSAPI_HWMODE_IEEE80211_A | CLSAPI_HWMODE_IEEE80211_B | CLSAPI_HWMODE_IEEE80211_G;
	else if (strcmp(str_hwmode, "HT") == 0)
		*hwmode = CLSAPI_HWMODE_IEEE80211_N;
	else if (strcmp(str_hwmode, "VHT") == 0)
		*hwmode = CLSAPI_HWMODE_IEEE80211_AC;
	else if (strcmp(str_hwmode, "HE") == 0)
		*hwmode = CLSAPI_HWMODE_IEEE80211_AX;
	else {
		DBG_ERROR("Invalid parameter--hwmode is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	*bw = bw_int2enum(local_bw);
	if (*bw == CLSAPI_WIFI_BW_DEFAULT) {
		DBG_ERROR("Invalid parameter--bandwidth is not supported.\n");

		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return CLSAPI_OK;
}

static inline int local_txpower_ratio_to_offset(enum CLSAPI_WIFI_TXPWR_RATIO txpower_ratio)
{
	switch (txpower_ratio) {
	case CLSAPI_WIFI_TXPWR_RATIO_100:
		return 0;
	case CLSAPI_WIFI_TXPWR_RATIO_75:
		return -2;
	case CLSAPI_WIFI_TXPWR_RATIO_50:
		return -3;
	case CLSAPI_WIFI_TXPWR_RATIO_25:
		return -6;
	case __CLSAPI_WIFI_TXPWR_RATIO_MAX:
	default:
		return 127;
	}
}

int clsapi_wifi_get_supported_max_vap(const char *phyname, uint16_t *max_vap)
{
	if (validate_phyname(phyname)) {
		DBG_ERROR("Invalid parameter--phyname is not valid.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	if (!max_vap)
		return -CLSAPI_ERR_NULL_POINTER;

	*max_vap = CLS_MAX_VAP_PER_RADIO;

	return CLSAPI_OK;
}

int clsapi_wifi_get_supported_max_sta(const char *phyname, uint16_t *max_sta)
{
	if (validate_phyname(phyname)) {
		DBG_ERROR("Invalid parameter--phyname is not valid.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	if (!max_sta)
		return -CLSAPI_ERR_NULL_POINTER;

	*max_sta = CLS_MAX_STA_PER_VAP;
	return CLSAPI_OK;
}

int clsapi_wifi_get_radio_enabled(const char *phyname, bool *enable)
{
	int ret = CLSAPI_OK;
	string_32 section;
	string_1024 status;

	if (!enable) {
		DBG_ERROR("Invalid parameter--enable is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = phyname_to_radio_name(phyname, section);
	if (ret)
		return -CLSAPI_ERR_INVALID_PHYNAME;

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "disabled", status);
	if (ret && ret != -CLSAPI_ERR_NOT_FOUND)
		return ret;

	if (strcmp(status, "1") == 0)
		*enable = 0;
	else if (strcmp(status, "0") == 0 || ret == -CLSAPI_ERR_NOT_FOUND)
		*enable = 1;
	else
		return -CLSAPI_ERR_UCI;

	return CLSAPI_OK;
}

int clsapi_wifi_enable_radio(const char *phyname, const bool enable)
{
	int ret = CLSAPI_OK;
	bool old_enable = 0;
	string_32 section;

	if (enable != 0 && enable != 1) {
		DBG_ERROR("Invalid parameter--enable is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = phyname_to_radio_name(phyname, section);
	if (ret)
		return -CLSAPI_ERR_INVALID_PHYNAME;

	ret = clsapi_wifi_get_radio_enabled(phyname, &old_enable);
	if (ret < 0)
		return ret;
	if (enable == old_enable)
		return CLSAPI_OK;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "disabled", enable ? "0" : "1");

	return ret;
}

int clsapi_wifi_get_supported_hwmodes(const char *phyname, enum clsapi_wifi_hwmode *hwmode)
{
	clsapi_ifname primary_ifname = {0};
	int ret = CLSAPI_OK, local_hwmodes;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	ret = nl80211_get_hwmodelist(primary_ifname, &local_hwmodes);

	*hwmode = 0;
	if (local_hwmodes & NL80211_80211_A)
		*hwmode |= CLSAPI_HWMODE_IEEE80211_A;
	if (local_hwmodes & NL80211_80211_B)
		*hwmode |= CLSAPI_HWMODE_IEEE80211_B;
	if (local_hwmodes & NL80211_80211_G)
		*hwmode |= CLSAPI_HWMODE_IEEE80211_G;
	if (local_hwmodes & NL80211_80211_N)
		*hwmode |= CLSAPI_HWMODE_IEEE80211_N;
	if (local_hwmodes & NL80211_80211_AC)
		*hwmode |= CLSAPI_HWMODE_IEEE80211_AC;
	if (local_hwmodes & NL80211_80211_AX)
		*hwmode |= CLSAPI_HWMODE_IEEE80211_AX;

	return CLSAPI_OK;

}

int clsapi_wifi_get_hwmode(const char *phyname, enum clsapi_wifi_hwmode *hwmode)
{
	long ht, vht, he;
	int ret = CLSAPI_OK;
	char status_reply[2048];
	clsapi_ifname primary_ifname = {0};
	size_t status_reply_len = sizeof(status_reply);

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	// get ht/vht/he status
	ret = hostapd_ctrl_cmd_get(primary_ifname, "STATUS", sizeof("STATUS"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0) {
		DBG_ERROR("Internal error--hostapd command send failed.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	ret = get_long_value_by_key(status_reply, "ieee80211n", &ht, 10);
	ret = get_long_value_by_key(status_reply, "ieee80211ac", &vht, 10);
	ret = get_long_value_by_key(status_reply, "ieee80211ax", &he, 10);
	if (ret < 0)
		return -CLSAPI_ERR_INTERNAL_ERR;

	if (he)
		*hwmode = CLSAPI_HWMODE_IEEE80211_AX;
	else if (vht)
		*hwmode = CLSAPI_HWMODE_IEEE80211_AC;
	else if (ht)
		*hwmode = CLSAPI_HWMODE_IEEE80211_N;
	else {
		uint32_t freq;
		long tmp_freq;
		enum clsapi_wifi_band band;
		ret = get_long_value_by_key(status_reply, "freq", &tmp_freq, 10);
		if (ret < 0) {
			DBG_ERROR("Internal error--could not get freguency from hostapd.\n");
			return -CLSAPI_ERR_INTERNAL_ERR;
		}

		freq = (uint32_t)tmp_freq;
		band = freq_mhz_to_band(freq);
		if (band == CLSAPI_BAND_2GHZ)
			*hwmode = CLSAPI_HWMODE_IEEE80211_G;
		else if (band == CLSAPI_BAND_5GHZ)
			*hwmode = CLSAPI_HWMODE_IEEE80211_A;
	}

	return CLSAPI_OK;
}

int clsapi_wifi_set_hwmode(const char *phyname, const enum clsapi_wifi_hwmode hwmode)
{
	int ret = CLSAPI_OK, phy_idx = -1;
	clsapi_ifname primary_ifname = {0};
	enum clsapi_wifi_hwmode old_hwmode;
	enum clsapi_wifi_band band;
	string_32 section;
	string_1024 new_htmode;

	ret = validate_phyname(phyname);
	if (ret)
		return ret;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	phy_idx = phyname_to_idx(phyname);
	band = RADIO_IDX_TO_BAND(phy_idx);
	ret = validate_hwmode(hwmode, band);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_radio_section(primary_ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	ret = clsapi_wifi_get_hwmode(phyname, &old_hwmode);
	if (ret < 0)
		return ret;

	if (old_hwmode == hwmode)
		return CLSAPI_OK;

	switch (hwmode) {
	case CLSAPI_HWMODE_IEEE80211_A:
	case CLSAPI_HWMODE_IEEE80211_B:
	case CLSAPI_HWMODE_IEEE80211_G:
		// legacy, remove 'htmode'
		strcpy(new_htmode, "");
		break;
	case CLSAPI_HWMODE_IEEE80211_N:
		strcpy(new_htmode, "HT20");
		break;
	case CLSAPI_HWMODE_IEEE80211_AC:
		strcpy(new_htmode, "VHT160");
		break;
	case CLSAPI_HWMODE_IEEE80211_AX:
		if (band == CLSAPI_BAND_2GHZ)
			strcpy(new_htmode, "HE20");
		else
			strcpy(new_htmode, "HE160");
		break;
	default:
		DBG_ERROR("Invalid parameter--hwmode is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "htmode", new_htmode);

	return ret;
}

int clsapi_wifi_get_beacon_intl(const char *phyname, uint16_t *beacon_int)
{
	int ret = CLSAPI_OK;
	string_2048 status_reply;
	size_t status_reply_len = sizeof(status_reply);
	clsapi_ifname primary_ifname = {0};
	long tmp_beacon_int = 0;

	if (!beacon_int)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = hostapd_ctrl_cmd_get(primary_ifname, "STATUS", sizeof("STATUS"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0) {
		DBG_ERROR("Invalid data--could not get reply from hostapd.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	ret = get_long_value_by_key(status_reply, "beacon_int", &tmp_beacon_int, 10);
	if (ret != CLSAPI_OK) {
		DBG_ERROR("Invalid data--could not get beacon interval from hostapd.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	*beacon_int = (uint16_t)tmp_beacon_int;

	if (15 <= *beacon_int)
		return CLSAPI_OK;
	else
		return -CLSAPI_ERR_INVALID_DATA;
}

int clsapi_wifi_set_beacon_intl(const char *phyname, const uint16_t beacon_int)
{
	int ret = CLSAPI_OK;
	string_32 radio_sct;

	if (beacon_int < 15) {
		DBG_ERROR("Invalid parameter--beacon interval is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (validate_phyname(phyname)) {
		DBG_ERROR("Invalid parameter--phyname is not valid.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	phyname_to_radio_name(phyname, radio_sct);
	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, radio_sct, "beacon_int", beacon_int);

	return ret;
}

int clsapi_wifi_get_rts(const char *phyname, int *rts)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	if (!rts) {
		DBG_ERROR("Invalid parameter--rts is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	nl80211_get_attr_value(primary_ifname, NL80211_CMD_GET_WIPHY, NULL, NL80211_ATTR_WIPHY_RTS_THRESHOLD, rts, sizeof(int));
	if (ret < 0)
		return ret;

	if (-1 <= *rts && *rts <= 65535)
		return CLSAPI_OK;
	else
		return -CLSAPI_ERR_INTERNAL_ERR;
}

int clsapi_wifi_set_rts(const char *phyname, const int rts)
{
	int ret = CLSAPI_OK;
	string_32 radio_sct;

	if (rts < -1 || rts > 2347) {
		DBG_ERROR("Invalid parameter--rts is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (validate_phyname(phyname))
		return -CLSAPI_ERR_INVALID_PHYNAME;

	phyname_to_radio_name(phyname, radio_sct);

	clsconf_defer_apply_int_param(CLSCONF_CFG_WIRELESS, radio_sct, "rts", rts);

	return ret;
}

int clsapi_wifi_get_dtim(const char *ifname, uint8_t *dtim)
{
	long tmp_dtim = 0;
	int ret = CLSAPI_OK;
	string_2048 status_reply;
	size_t status_reply_len = sizeof(status_reply);

	if (!dtim) {
		DBG_ERROR("Invalid parameter--dtim is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	ret = hostapd_ctrl_cmd_get(ifname, "STATUS", sizeof("STATUS"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0) {
		DBG_ERROR("Invalid data--could not get reply from hostapd.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	ret = get_long_value_by_key(status_reply, "dtim_period", &tmp_dtim, 10);
	if (ret != CLSAPI_OK) {
		DBG_ERROR("Invalid data--could not get dtim from hostapd.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	*dtim = (uint8_t)tmp_dtim;

	if (1 <= *dtim)
		return CLSAPI_OK;
	else
		return -CLSAPI_ERR_INTERNAL_ERR;
}

int clsapi_wifi_set_dtim(const char *ifname, const uint8_t dtim)
{
	string_32 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);

	if (dtim < 1) {
		DBG_ERROR("Invalid parameter--dtim is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	clsconf_ifname_to_bss_section(ifname, section, section_len);
	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, section, "dtim_period", dtim);

	return ret;
}

int clsapi_wifi_get_txpower_limit(const char *phyname, int8_t *txpower)
{
	int ret = CLSAPI_OK, dbm_max = 0;
	clsapi_ifname primary_ifname;
	uint8_t ch_cur = 0;
	string_32 radio_sct;

	if (!txpower) {
		DBG_ERROR("Invalid parameter--txpower is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_phyname(phyname))
		return -CLSAPI_ERR_INVALID_PHYNAME;

	phyname_to_radio_name(phyname, radio_sct);
	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, radio_sct, "txpower", *txpower);
	if (ret < 0) {
		ret = phyname_to_primary_ifname(phyname, primary_ifname);
		if (ret)
			return ret;

		ret = clsapi_wifi_get_channel(phyname, &ch_cur);
		if (ret < 0)
			return ret;

		ret = nl80211_get_max_txpower(primary_ifname, (int)ch_cur, &dbm_max);
		*txpower = dbm_max;
	}

	return ret;
}

int clsapi_wifi_get_max_allow_sta(const char *ifname, uint16_t *max_sta)
{
	int ret = CLSAPI_OK;
	string_32 section;
	int section_len = sizeof(section);

	if (!max_sta) {
		DBG_ERROR("Invalid parameter--max_sta is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, section, section_len);
	if (ret < 0)
		return ret;

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "maxassoc", *max_sta);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		*max_sta = CLS_MAX_STA_PER_VAP; //default value
		return CLSAPI_OK;
	} else
		if (ret)
			return ret;

	if (1 <= *max_sta && *max_sta <= CLS_MAX_STA_PER_VAP)
		return CLSAPI_OK;
	else {
		DBG_ERROR("Invalid data--max_sta is invalid.\n");
		return -CLSAPI_ERR_INVALID_DATA;
	}
}

int clsapi_wifi_set_max_allow_sta(const char *ifname, const uint16_t max_sta)
{
	int ret = CLSAPI_OK;
	string_32 section;
	int section_len = sizeof(section);

	if (max_sta < 1 || max_sta > CLS_MAX_STA_PER_VAP) {
		DBG_ERROR("Invalid parameter--max_sta is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	clsconf_ifname_to_bss_section(ifname, section, section_len);
	if (ret < 0)
		return ret;

	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, section, "maxassoc", max_sta);

	return ret;
}

int clsapi_wifi_set_hidden_ssid(const char *ifname, const enum clsapi_wifi_hidden_ssid hidden_ssid)
{
	int ret = CLSAPI_OK, cmd_len = 0;
	string_64 cmd, section;

	if (hidden_ssid >= CLSAPI_WIFI_HIDDEN_SSID_MAX) {
		DBG_ERROR("Invalid parameter--hidden_ssid is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}
	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	cmd_len = snprintf(cmd, sizeof(cmd), "HIDDEN_SSID %d", hidden_ssid) + 1;

	ret = hostapd_ctrl_cmd_set(ifname, cmd, cmd_len);
	if (ret < 0)
		return ret;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	clsconf_set_int_param(CLSCONF_CFG_WIRELESS, section, "hidden", hidden_ssid);

	return ret;
}

int clsapi_wifi_get_hidden_ssid(const char *ifname, enum clsapi_wifi_hidden_ssid *enable)
{
	int ret = CLSAPI_OK;
	string_64 section;

	if (!enable) {
		DBG_ERROR("Invalid parameter--enable is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "hidden", *enable);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		*enable = 0; //default value
	}

	return ret;
}

int clsapi_wifi_get_short_preamble(const char *ifname, bool *short_preamble)
{
	int ret = CLSAPI_OK;
	string_64 section;

	if (!short_preamble) {
		DBG_ERROR("Invalid parameter--short_preamble is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "short_preamble", *short_preamble);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		*short_preamble = 1; //default value
	}

	return ret;
}

int clsapi_wifi_set_short_preamble(const char *ifname, const bool short_preamble)
{
	int ret;
	string_64 section;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	clsconf_defer_apply_int_param(CLSCONF_CFG_WIRELESS, section, "short_preamble", short_preamble);

	return ret;
}


int clsapi_wifi_get_short_gi(const char *phyname, const enum clsapi_wifi_bw bw, bool *onoff)
{
#define CLSAPI_HT20_SHORT_GI	BIT(5)
#define CLSAPI_HT40_SHORT_GI	BIT(6)
#define CLSAPI_VHT80_SHORT_GI	BIT(5)
#define CLSAPI_VHT160_SHORT_GI	BIT(6)

	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band = CLSAPI_BAND_NOSUCH_BAND;
	enum clsapi_wifi_hwmode hwmode = 0;
	string_2048 status_reply;
	size_t status_reply_len = sizeof(status_reply);
	long ht_caps = 0, vht_caps = 0, ht = -1, vht = -1;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	band = get_phy_band(phyname);

	clsapi_wifi_get_hwmode(phyname, &hwmode);
	if ((ret = validate_bw(hwmode, band, bw)) != CLSAPI_OK)
		return ret;

	ret = hostapd_ctrl_cmd_get(primary_ifname, "STATUS", sizeof("STATUS"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0) {
		DBG_ERROR("Invalid data--could not get reply from hostapd.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	get_long_value_by_key(status_reply, "ieee80211n", &ht, 10);
	get_long_value_by_key(status_reply, "ieee80211ac", &vht, 10);
	get_long_value_by_key(status_reply, "ht_caps_info", &ht_caps, 16);
	get_long_value_by_key(status_reply, "vht_caps_info", &vht_caps, 16);

	if (bw == CLSAPI_WIFI_BW_20 && ht == 1 && ht_caps & CLSAPI_HT20_SHORT_GI)
		*onoff = 1;
	else if (bw == CLSAPI_WIFI_BW_40 && ht == 1 && ht_caps & CLSAPI_HT40_SHORT_GI)
		*onoff = 1;
	else if (bw == CLSAPI_WIFI_BW_80 && vht == 1 && vht_caps & CLSAPI_VHT80_SHORT_GI)
		*onoff = 1;
	else if (bw == CLSAPI_WIFI_BW_160 && vht == 1 && vht_caps & CLSAPI_VHT160_SHORT_GI)
		*onoff = 1;
	else
		*onoff = 0;

	return CLSAPI_OK;
}

int clsapi_wifi_set_short_gi(const char *phyname, const enum clsapi_wifi_bw bw, const bool onoff)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band = CLSAPI_BAND_NOSUCH_BAND;
	enum clsapi_wifi_hwmode hwmode = 0;
	string_32 radio_sct, short_gi_option;

	if (onoff != 0 && onoff != 1) {
		DBG_ERROR("Invalid parameter--onoff is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	band = get_phy_band(phyname);
	clsapi_wifi_get_hwmode(phyname, &hwmode);
	if ((ret = validate_bw(hwmode, band, bw)) != CLSAPI_OK)
		return ret;

	phyname_to_radio_name(phyname, radio_sct);
	sprintf(short_gi_option, "short_gi_%d", bw_enum2int(bw));

	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, radio_sct, short_gi_option, onoff);

	return ret;
}

int clsapi_wifi_get_country_code(const char *phyname, string_8 country_code)
{
	int ret = CLSAPI_OK;
	char status_reply[2048];
	size_t status_reply_len = sizeof(status_reply);
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!country_code) {
		DBG_ERROR("Invalid parameter--countrycode is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = hostapd_ctrl_cmd_get(primary_ifname, "STATUS-DRIVER", sizeof("STATUS-DRIVER"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0) {
		DBG_ERROR("Invalid data--could not get reply from hostapd.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	ret = get_str_value_by_key(status_reply, "country", country_code, 9);
	if (ret < 0)
		return -CLSAPI_ERR_NOT_FOUND;

	return CLSAPI_OK;
}

int clsapi_wifi_set_country_code(const char *phyname, const char *country_code)
{
	int ret = CLSAPI_OK, is_supported_code = 0;
	string_32 radio_sct;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	if (!country_code || strlen(country_code) < 1) {
		DBG_ERROR("Invalid parameter--countrycode is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	for (int i = 0; i < ARRAY_SIZE(clsapi_supported_country_code_tbl); i++) {
		if (strcmp(country_code, clsapi_supported_country_code_tbl[i]) == 0) {
			is_supported_code = 1;
			break;
		}
	}

	if (is_supported_code != 1)
		return -CLSAPI_ERR_INVALID_PARAM;

	phyname_to_radio_name(phyname, radio_sct);

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, radio_sct, "country", country_code);

	return ret;
}

static int local_wifi_get_wmm_enabled(const char *ifname, bool *onoff)
{
	int ret = CLSAPI_OK;
	string_64 section = {0};

	if (!onoff)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "wmm", *onoff);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		*onoff = 1; //default is enabled
	}

	return ret;
}

int clsapi_wifi_get_wmm_enabled(const char *ifname, bool *onoff)
{
	return local_wifi_get_wmm_enabled(ifname, onoff);
}

int clsapi_wifi_enable_wmm(const char *ifname, const bool onoff)
{
	int ret = CLSAPI_OK;
	bool old_wmm = 0;
	string_64 section = {0};

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = local_wifi_get_wmm_enabled(ifname, &old_wmm);
	if (ret < 0)
		return ret;
	if (onoff == old_wmm)
		return CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, section, "wmm", onoff);

	return ret;
}

static int local_wifi_get_pmf_enabled(const char *ifname, enum clsapi_wifi_pmf_enable *enable_cfg)
{
	int ret = CLSAPI_OK;
	string_64 section = {0};

	if (!enable_cfg)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "ieee80211w", *enable_cfg);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		*enable_cfg = CLSAPI_WIFI_PMF_DISABLED; //default is disabled
	}

	return ret;
}

int clsapi_wifi_get_pmf_enabled(const char *ifname, enum clsapi_wifi_pmf_enable *enable_cfg)
{
	return local_wifi_get_pmf_enabled(ifname, enable_cfg);
}

int clsapi_wifi_enable_pmf(const char *ifname, const enum clsapi_wifi_pmf_enable enable_cfg)
{
	int ret = CLSAPI_OK;
	string_64 section = {0};
	enum clsapi_wifi_pmf_enable old_pmf_enble = CLSAPI_WIFI_PMF_MAX;

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	if (enable_cfg >= CLSAPI_WIFI_PMF_MAX)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = local_wifi_get_pmf_enabled(ifname, &old_pmf_enble);
	if (ret < 0)
		return ret;
	if (enable_cfg == old_pmf_enble)
		return CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, section, "ieee80211w", enable_cfg);

	return ret;
}

static int local_wifi_get_sta_max_inactivity(const char *ifname, uint32_t *max_inactivity)
{
	int ret = CLSAPI_OK;
	string_64 section = {0};

	if (!max_inactivity) {
		DBG_ERROR("Invalid parameter--max_inactivity is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return -CLSAPI_ERR_INVALID_IFNAME;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "max_inactivity", *max_inactivity);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		*max_inactivity = 300; //default value
	}

	return ret;
}

int clsapi_wifi_get_sta_max_inactivity(const char *ifname, uint32_t *max_inactivity)
{
	return local_wifi_get_sta_max_inactivity(ifname, max_inactivity);
}

int clsapi_wifi_set_sta_max_inactivity(const char *ifname, const uint32_t max_inactivity)
{
	int ret = CLSAPI_OK;
	string_64 section = {0};
	uint32_t old_mac_inactivity = 0;

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	if (max_inactivity == 0) {
		DBG_ERROR("Invalid parameter--max_inactivity is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = local_wifi_get_sta_max_inactivity(ifname, &old_mac_inactivity);
	if (ret < 0)
		return ret;
	if (max_inactivity == old_mac_inactivity)
		return CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_uint_param(CLSCONF_CFG_WIRELESS, section, "max_inactivity", max_inactivity);

	return ret;
}

int clsapi_wifi_get_iftype(const char *ifname, enum clsapi_wifi_iftype *wifi_iftype)
{
	int ret = CLSAPI_OK;
	enum nl80211_iftype iftype;

	if (!ifname || !wifi_iftype) {
		DBG_ERROR("Invalid parameter--ifname is not existed or wifi_iftype is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_ifname(ifname)) {
		DBG_ERROR("Invalid parameter--ifname is invalid.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	ret = nl80211_get_attr_value(ifname, NL80211_CMD_GET_INTERFACE, NULL, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
	if (ret < 0)
		return ret;

	if (iftype == NL80211_IFTYPE_AP)
		*wifi_iftype = CLSAPI_WIFI_IFTYPE_AP;
	else if (iftype == NL80211_IFTYPE_STATION)
		*wifi_iftype = CLSAPI_WIFI_IFTYPE_STA;
	else {
		*wifi_iftype = CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE;
		ret = -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return ret;
}

int clsapi_wifi_get_ifnames(const char *phyname, enum clsapi_wifi_iftype iftype, clsapi_ifname (**ifname_array))
{
	int ret = CLSAPI_OK;
	string_32 ifname_list[CLS_MAX_VAP_PER_RADIO] = {0};
	int ifname_num = 0;

	if (iftype < CLSAPI_WIFI_IFTYPE_AP || iftype >= CLSAPI_WIFI_IFTYPE_MAX) {
		DBG_ERROR("Invalid parameter--wifi_iftype is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = local_wifi_get_ifnames(phyname, iftype, ifname_list, 0, ARRAY_SIZE(ifname_list));
	if (ret < 0)
		return ret;
	ifname_num = ret;

	*ifname_array = (clsapi_ifname (*))calloc(ifname_num, sizeof(clsapi_ifname));
	if (*ifname_array == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < ifname_num; i++)
		cls_strncpy((*ifname_array)[i], ifname_list[i], sizeof(clsapi_ifname));

	return ifname_num;
}

int clsapi_wifi_get_assoc_list(const char *ifname, uint8_t (**sta_array)[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	uint8_t sta_macs[CLS_MAX_STA_PER_VAP][ETH_ALEN];
	int list_len = ARRAY_SIZE(sta_macs);

	ret = validate_ap_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = nl80211_get_assoc_list(ifname, sta_macs, &list_len);
	if (ret < 0)
		return ret;

	*sta_array = (uint8_t (*)[ETH_ALEN])calloc(list_len, ETH_ALEN);
	if (*sta_array == NULL)
		return -CLSAPI_ERR_NO_MEM;

	memcpy(*sta_array, sta_macs, list_len * ETH_ALEN);

	return list_len;
}

int clsapi_wifi_deauth_sta(const char *ifname, const uint8_t mac[ETH_ALEN], const uint16_t reason_code)
{
	int ret = CLSAPI_OK;
	string_64 cmd;
	int cmd_len = 0;

	if (!mac) {
		DBG_ERROR("Invalid parameter--mac is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	cmd_len = snprintf(cmd, sizeof(cmd), "DEAUTHENTICATE "MACFMT" reason=%d", MACARG(mac), reason_code) + 1;
	return hostapd_ctrl_cmd_set(ifname, cmd, cmd_len);
}

int clsapi_wifi_disassoc_sta(const char *ifname, const uint8_t mac[ETH_ALEN], const uint16_t reason_code)
{
	int ret = CLSAPI_OK;
	string_64 cmd;
	int cmd_len = 0;

	if (!mac) {
		DBG_ERROR("Invalid parameter--mac is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	cmd_len = snprintf(cmd, sizeof(cmd), "DISASSOCIATE "MACFMT" reason=%d", MACARG(mac), reason_code) + 1;
	return hostapd_ctrl_cmd_set(ifname, cmd, cmd_len);
}

static int local_wifi_get_macfilter_policy(const char *ifname, enum clsapi_wifi_macfilter_policy *policy)
{
	int ret = CLSAPI_OK;
	string_64 bss_section = {0};
	string_1024 local_policy = {0};

	if (!policy) {
		DBG_ERROR("Invalid parameter--policy is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, bss_section, sizeof(bss_section));
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, bss_section, "macfilter", local_policy);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		strcpy(local_policy, "disable"); //default is disable
	}
	if (ret < 0)
		return ret;

	*policy = macfilter_policy_str2enum(local_policy);
	if (*policy >= CLSAPI_WIFI_MACFILTER_POLICY_MAX) {
		DBG_ERROR("Invalid data--policy is invalid.\n");
		return -CLSAPI_ERR_INVALID_DATA;
	}

	return ret;
}

int clsapi_wifi_get_macfilter_policy(const char *ifname, enum clsapi_wifi_macfilter_policy *policy)
{
	return local_wifi_get_macfilter_policy(ifname, policy);
}

int clsapi_wifi_set_macfilter_policy(const char *ifname, const enum clsapi_wifi_macfilter_policy policy)
{
	int ret = CLSAPI_OK;
	string_64 bss_section = {0};
	enum clsapi_wifi_macfilter_policy old_policy = {0};

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	if (policy >= CLSAPI_WIFI_MACFILTER_POLICY_MAX) {
		DBG_ERROR("Invalid parameter--policy is over the maximum of policies.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = local_wifi_get_macfilter_policy(ifname, &old_policy);
	if (ret < 0)
		return ret;

	if (policy == old_policy)
		return CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, bss_section, sizeof(bss_section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, bss_section, "macfilter", macfilter_policy_enum2str(policy));

	return ret;
}

static int local_wifi_get_macfilter_maclist(const char *ifname, uint8_t (*maclist)[ETH_ALEN], int *maclist_len)
{
	int ret = CLSAPI_OK;
	string_64 bss_section = {0};
	string_1024 *str_maclist = NULL;
	int str_maclist_len = 0, i = 0;

	if (!maclist || !maclist_len || *maclist_len < 1) {
		DBG_ERROR("Invalid parameter--maclist is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, bss_section, sizeof(bss_section));
	if (ret < 0)
		return ret;

	str_maclist = malloc(sizeof(string_1024) * *maclist_len);
	str_maclist_len = *maclist_len;
	if (str_maclist == NULL)
		return -CLSAPI_ERR_NO_MEM;

	ret = clsconf_get_list(CLSCONF_CFG_WIRELESS, bss_section, "maclist", str_maclist, &str_maclist_len);
	if (ret && ret != -CLSAPI_ERR_NOT_FOUND) {
		goto RETURN;
	}

	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = CLSAPI_OK;
		*maclist_len = 0;
		goto RETURN;
	}

	for (i = 0; i < str_maclist_len && i < *maclist_len; i++) {
		ret = mac_aton(str_maclist[i], maclist[i]);
		if (ret) {
			ret = -CLSAPI_ERR_INVALID_DATA;
			goto RETURN;
		}
	}

	*maclist_len = i;
	ret = CLSAPI_OK;

RETURN:
	if (str_maclist)
		free(str_maclist);

	return ret;
}

int clsapi_wifi_get_macfilter_maclist(const char *ifname, uint8_t (**mac_array)[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	uint8_t maclist[CLS_MAX_STA_PER_VAP][ETH_ALEN] = {0};
	int maclist_len = ARRAY_SIZE(maclist);

	ret = local_wifi_get_macfilter_maclist(ifname, maclist, &maclist_len);
	if (ret < 0)
		return ret;

	*mac_array = (uint8_t (*)[ETH_ALEN])calloc(maclist_len, ETH_ALEN);
	if (*mac_array == NULL)
		return -CLSAPI_ERR_NO_MEM;

	memcpy(*mac_array, maclist, maclist_len * ETH_ALEN);

	return maclist_len;
}

/* Check whether STA in MAC filter list
 * Returns:
 *   <0: Errors
 *   0: Not exist in MAC list
 *   1: Exist in MAC list
 */
static int is_in_macfilter_list(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	uint8_t mac_list[1024][ETH_ALEN];
	int list_num = ARRAY_SIZE(mac_list);
	int ret = local_wifi_get_macfilter_maclist(ifname, mac_list, &list_num);

	if (ret < 0)
		return -ret;

	for (int i = 0; i < list_num; i++) {
		if (memcmp(sta_mac, mac_list[i], ETH_ALEN) == 0)
			return 1;
	}

	return 0;
}

int clsapi_wifi_add_macfilter_mac(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	string_64 bss_section = {0};
	string_32 str_mac = {0};

	if (!sta_mac) {
		DBG_ERROR("Invalid parameter--station macaddress is not existed.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = validate_unicast_macaddr(sta_mac);
	if (ret) {
		DBG_ERROR("Invalid parameter--station macaddress is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = is_in_macfilter_list(ifname, sta_mac);
	if (ret < 0)
		return -CLSAPI_ERR_INTERNAL_ERR;
	else if (ret == 1) {
		DBG_ERROR("Invalid parameter--station macaddress has already been in macfilter.\n");
		return -CLSAPI_ERR_EXISTED;
	}

	ret = clsconf_ifname_to_bss_section(ifname, bss_section, sizeof(bss_section));
	if (ret < 0)
		return ret;

	snprintf(str_mac, sizeof(str_mac), MACFMT, MACARG(sta_mac));
	clsconf_defer_add_apply_list(CLSCONF_CFG_WIRELESS, bss_section, "maclist", str_mac);

	return ret;
}

int clsapi_wifi_del_macfilter_mac(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	string_64 bss_section = {0};
	string_32 str_mac = {0};

	if (!sta_mac) {
		DBG_ERROR("Invalid parameter--station macaddress is not existed.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = validate_unicast_macaddr(sta_mac);
	if (ret) {
		DBG_ERROR("Invalid parameter--station macaddress is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = is_in_macfilter_list(ifname, sta_mac);
	if (ret < 0)
		return -CLSAPI_ERR_INTERNAL_ERR;
	else if (ret == 0) {
		DBG_ERROR("Invalid parameter--station macaddress is not in macfilter.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	ret = clsconf_ifname_to_bss_section(ifname, bss_section, sizeof(bss_section));
	if (ret < 0)
		return ret;

	snprintf(str_mac, sizeof(str_mac), MACFMT, MACARG(sta_mac));
	clsconf_defer_del_apply_list(CLSCONF_CFG_WIRELESS, bss_section, "maclist", str_mac);

	return ret;
}

int clsapi_wifi_start_adv_scan(const char *phyname,
		struct clsapi_wifi_scan_params *params,
		enum clsapi_wifi_band band,
		const uint8_t channels[],
		const int channels_len)
{
	int ret = CLSAPI_OK;
	uint32_t adv_scan_enable = 1, work_duration = 0;
	clsapi_ifname primary_ifname = {0};
	struct nl80211_attr_id_value attr[4] = {0};
	struct nl80211_attr_tbl put_attrs = {
		.n_attr = ARRAY_SIZE(attr),
		.attrs = attr
	};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	work_duration = params->work_duration * 1000;

	attr[0].id = CLS_NL80211_ATTR_SCAN_EXT_ENABLE;
	attr[0].value = (void *)&adv_scan_enable;
	attr[0].val_len = sizeof(uint32_t);

	attr[1].id = CLS_NL80211_ATTR_SCAN_EXT_RX_FILTER;
	attr[1].value = (void *)&params->rx_filter;
	attr[1].val_len = sizeof(uint32_t);

	attr[2].id = CLS_NL80211_ATTR_SCAN_EXT_WORK_DURATION;
	attr[2].value = (void *)&work_duration;
	attr[2].val_len = sizeof(uint32_t);

	attr[3].id = CLS_NL80211_ATTR_SCAN_EXT_SCAN_INTERVAL;
	attr[3].value = (void *)&params->scan_interval;
	attr[3].val_len = sizeof(uint32_t);

	ret = nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_SCAN_EXT, &put_attrs);

	if (ret < 0)
		return ret;

	ret = clsapi_wifi_start_scan(phyname, params->flags, band, channels, channels_len);

	return ret;
}

int clsapi_wifi_start_scan(const char *phyname, enum clsapi_wifi_scan_flags scan_flags,
		enum clsapi_wifi_band band, const uint8_t channels[], const int channels_len)
{
	int ret = CLSAPI_OK, i = 0;
	enum clsapi_wifi_band org_band = CLSAPI_BAND_DEFAULT;
	uint32_t freqs[255] = {0};
	uint8_t freqs_len = ARRAY_SIZE(freqs);
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	org_band = get_phy_band(phyname);
	if (band != CLSAPI_BAND_DEFAULT) {
		// Check if target band is supported
		ret = validate_band(phyname, band);
		if (ret < 0)
			return ret;
	} else
		band = org_band;
	if (channels && channels_len > 0) {
		if (channels_len > freqs_len) {
			DBG_ERROR("Invalid parameter--channels length or frequency length too long.\n");
			return -CLSAPI_ERR_TOO_LARGE;
		}
		for (i = 0; channels[i] && i < channels_len; i++) {
			// Check if target channel is supported
			ret = get_sec_chan_offset(band, channels[i]);
			if (ret == WIFI_SEC_CHAN_OFFSET_NO_SEC)
				return -CLSAPI_ERR_INVALID_PARAM;

			freqs[i] = channel_to_freq_mhz(channels[i], band);
			if (freqs[i] == 0)
				return -CLSAPI_ERR_INVALID_PARAM;
		}
		freqs_len = i;
	}

	if (channels_len)
		ret = nl80211_trigger_scan(primary_ifname, freqs, freqs_len, scan_flags);
	else
		ret = nl80211_trigger_scan(primary_ifname, NULL, 0, scan_flags);

	return ret;
}

static int local_get_scan_status(const char *ifname, string_32 status)
{
	FILE *scan_status_fh = NULL;
	string_128 scan_status_file = {0};

	if (!status)
		return -CLSAPI_ERR_NULL_POINTER;

	snprintf(scan_status_file, sizeof(scan_status_file), "%s%s", CLSAPI_WIFI_SCAN_STATUS_FILE, ifname);

	scan_status_fh = fopen(scan_status_file, "r");
	if (scan_status_fh == NULL) {
		DBG_ERROR("Internal error--failed to open %s.\n", scan_status_file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	status[0] = '\0';
	if (fgets(status, sizeof(string_32), scan_status_fh) == NULL) {
		fclose(scan_status_fh);
		return -CLSAPI_ERR_SCAN_NOT_START;
	}


	fclose(scan_status_fh);
	return CLSAPI_OK;
}

static int local_wifi_get_scan_result_bytes(const char *ifname)
{
	FILE *scan_result_fh = NULL;
	string_128 scan_result_file = {0};
	uint32_t file_size = 0;

	snprintf(scan_result_file, sizeof(scan_result_file), "%s%s", CLSAPI_WIFI_SCAN_RESULT_FILE, ifname);
	scan_result_fh = fopen(scan_result_file, "r");
	if (scan_result_fh == NULL) {
		DBG_ERROR("Internal error--failed to open %s.\n", scan_result_file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	fseek(scan_result_fh, 0, SEEK_END);
	file_size = ftell(scan_result_fh);

	fclose(scan_result_fh);

	return file_size;
}

static int local_wifi_get_scan_count(const char *phyname, uint32_t *ap_cnt)
{
	int ret = CLSAPI_OK, scan_result_bytes = 0;
	string_32 scan_status = {0};
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = local_get_scan_status(primary_ifname, scan_status);
	if (ret < 0)
		return ret;

	if (strcmp(scan_status, CLSAPI_WIFI_SCAN_STATUS_SCANNING) == 0)
		return -CLSAPI_ERR_SCAN_SCANNING;
	else if (strcmp(scan_status, CLSAPI_WIFI_SCAN_STATUS_ABORTED) == 0)
		return -CLSAPI_ERR_SCAN_ABORTED;
	else if (strcmp(scan_status, CLSAPI_WIFI_SCAN_STATUS_COMPLETE) != 0)
		return -CLSAPI_ERR_INTERNAL_ERR;

	// get scan cache size and malloc() buffer
	scan_result_bytes = local_wifi_get_scan_result_bytes(primary_ifname);

	*ap_cnt = scan_result_bytes / sizeof(struct clsapi_scan_ap_info);

	return CLSAPI_OK;
}

int clsapi_wifi_get_scan_count(const char *phyname, uint32_t *ap_cnt)
{
	return local_wifi_get_scan_count(phyname, ap_cnt);
}

static int local_wifi_get_entry_from_cache(const char *ifname, const int ap_idx,
			const int ap_cnt, struct clsapi_scan_ap_info *scan_ap_array)
{
	int ret = CLSAPI_OK, read_len = -1;
	FILE *scan_result_fh = NULL;
	string_128 scan_result_file = {0};

	snprintf(scan_result_file, sizeof(scan_result_file), "%s%s", CLSAPI_WIFI_SCAN_RESULT_FILE, ifname);
	scan_result_fh = fopen(scan_result_file, "r");
	if (scan_result_fh == NULL) {
		DBG_ERROR("Internal error--failed to open %s.\n", scan_result_file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	if (ap_idx >= 0) {
		// Only get one AP info
		ret = fseek(scan_result_fh, ap_idx * sizeof(struct clsapi_scan_ap_info), SEEK_SET);
		if (ret) {
			DBG_ERROR("Internal error--failed to seek %s.\n", scan_result_file);
			ret = -CLSAPI_ERR_INVALID_DATA;
			goto RETURN;
		}

		read_len = fread(scan_ap_array, 1, sizeof(struct clsapi_scan_ap_info), scan_result_fh);
		if (read_len != sizeof(struct clsapi_scan_ap_info)) {
			ret = -CLSAPI_ERR_INVALID_DATA;
			DBG_ERROR("Internal error--failed to read %s.\n", scan_result_file);
			goto RETURN;
		}
	} else {
		// Get all AP infos
		for (int i = 0; i < ap_cnt; i++) {
			read_len = fread(scan_ap_array + i, 1, sizeof(struct clsapi_scan_ap_info), scan_result_fh);
			if (read_len != sizeof(struct clsapi_scan_ap_info)) {
				DBG_ERROR("Internal error--failed to read %s.\n", scan_result_fh);
				ret = -CLSAPI_ERR_INVALID_DATA;
				goto RETURN;
			}
		}
	}

	ret = CLSAPI_OK;
RETURN:
	fclose(scan_result_fh);
	return ret;
}

static int local_wifi_get_scan_entry(const char *phyname, const int ap_idx,
	struct clsapi_scan_ap_info **scan_ap_array)
{
	int ret = CLSAPI_OK;
	uint8_t *buffer = NULL;
	int buf_len = -1, ret_ap_cnt = 0;
	uint32_t ap_cnt = 0;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	// get scan cache size and malloc() buffer
	ret = local_wifi_get_scan_count(phyname, &ap_cnt);
	if (ret < 0)
		return ret;

	if (ap_idx >= 0 && ap_idx < ap_cnt)
		ret_ap_cnt = 1;
	else if (ap_idx == -1)
		ret_ap_cnt = ap_cnt;
	else
		return -CLSAPI_ERR_INVALID_PARAM_LEN;

	buf_len = ret_ap_cnt * sizeof(struct clsapi_scan_ap_info);
	buffer = malloc(buf_len);
	if (buffer == NULL) {
		DBG_ERROR("Internal error--failed to malloc() %s bytes for scan result!!\n", buf_len);
		return -CLSAPI_ERR_NO_MEM;
	}

	//scan_ap_entry->ap_info = (struct clsapi_scan_ap_info *)((uint8_t *)scan_ap_entry + offsetof(struct clsapi_scan_ap_entry, ap_info));
	ret = local_wifi_get_entry_from_cache(primary_ifname, ap_idx, ap_cnt, (struct clsapi_scan_ap_info *)buffer);
	if (ret < 0) {
		if (buffer)
			free(buffer);
		return ret;
	}

	*scan_ap_array = (struct clsapi_scan_ap_info *)buffer;

	return ret_ap_cnt;
}

int clsapi_wifi_get_scan_entry(const char *phyname, const int ap_idx,
	struct clsapi_scan_ap_info **scan_ap_array)
{
	return local_wifi_get_scan_entry(phyname, ap_idx, scan_ap_array);
}

int clsapi_wifi_get_supported_bws(const char *phyname, enum clsapi_wifi_bw **bws)
{
	int htmodes = 0, bws_cnt = 0;
	int ret = CLSAPI_OK;
	enum clsapi_wifi_bw local_bws[CLSAPI_WIFI_BW_MAX] = {CLSAPI_WIFI_BW_DEFAULT};
	clsapi_ifname primary_ifname = {0};

	if (phyname_to_primary_ifname(phyname, primary_ifname)) {
		DBG_ERROR("Internal parameter--parse phyname to primary interface name fail.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	ret = nl80211_get_htmodelist(primary_ifname, &htmodes);
		if (ret < 0)
			return -CLSAPI_ERR_UNKNOWN_ERROR;

	if (htmodes & (NL80211_HTMODE_HT20 | NL80211_HTMODE_VHT20 | NL80211_HTMODE_HE20))
		local_bws[bws_cnt++] = CLSAPI_WIFI_BW_20;
	if (htmodes & (NL80211_HTMODE_HT40 | NL80211_HTMODE_VHT40 | NL80211_HTMODE_HE40))
		local_bws[bws_cnt++] = CLSAPI_WIFI_BW_40;
	if (htmodes & (NL80211_HTMODE_VHT80 | NL80211_HTMODE_HE80))
		local_bws[bws_cnt++] = CLSAPI_WIFI_BW_80;
	if (htmodes & (NL80211_HTMODE_VHT160 | NL80211_HTMODE_HE160))
		local_bws[bws_cnt++] = CLSAPI_WIFI_BW_160;

	*bws = (enum clsapi_wifi_bw *)calloc(bws_cnt, sizeof(enum clsapi_wifi_bw));
	if (*bws == NULL)
		return -CLSAPI_ERR_NO_MEM;

	memcpy(*bws, local_bws, bws_cnt * sizeof(enum clsapi_wifi_bw));

	return bws_cnt;
}

int clsapi_wifi_get_bw(const char *phyname, enum clsapi_wifi_bw *bw)
{
	int ret = CLSAPI_OK;
	enum nl80211_chan_width nl80211_bw;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;
	if (!bw) {
		DBG_ERROR("Internal parameter--bw is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = nl80211_get_attr_value(primary_ifname, NL80211_CMD_GET_INTERFACE, NULL, NL80211_ATTR_CHANNEL_WIDTH, &nl80211_bw,
				sizeof(uint32_t));
	if (ret < 0)
		return ret;

	switch(nl80211_bw) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		*bw = CLSAPI_WIFI_BW_20;
		break;
	case NL80211_CHAN_WIDTH_40:
		*bw = CLSAPI_WIFI_BW_40;
		break;
	case NL80211_CHAN_WIDTH_80:
		*bw = CLSAPI_WIFI_BW_80;
		break;
	case NL80211_CHAN_WIDTH_160:
		*bw = CLSAPI_WIFI_BW_160;
		break;
	default:
		*bw = CLSAPI_WIFI_BW_DEFAULT;
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	return CLSAPI_OK;
}

int local_wifi_switch_bw(const char *phyname, const uint8_t channel, const enum clsapi_wifi_bw bw)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band;
	char cmd[256], status_reply[2048];
	int cmd_len = 0, csa_cnt = 10;
	size_t status_reply_len = sizeof(status_reply);
	long ht, vht, he;
	int sec_chan_offset = 0, freq = 0, center_chan1 = 0, center_freq1 = 0;
	const char *cmd_fmt_bw_non80_80 =
	"CHAN_SWITCH %d %d sec_channel_offset=%d center_freq1=%d center_freq2=0 bandwidth=%d blocktx %s %s %s";
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	band = get_phy_band(phyname);
	ret = validate_channel(phyname, channel, band, bw);
	if (ret < 0) {
		DBG_ERROR("Invalid parameter--channel is invalid\n");
		return -CLSAPI_ERR_INVALID_CHANNEL;
	}

	freq = channel_to_freq_mhz(channel, band);
	if (!freq) {
		DBG_ERROR("Invalid parameter--frequency convert to channel failed\n");
		return -CLSAPI_ERR_INVALID_CHANNEL;
	}

	// get ht/vht/he status
	ret = hostapd_ctrl_cmd_get(primary_ifname, "STATUS", sizeof("STATUS"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0)
		return -CLSAPI_ERR_NOT_FOUND;
	ret = get_long_value_by_key(status_reply, "ieee80211n", &ht, 10);
	ret = get_long_value_by_key(status_reply, "ieee80211ac", &vht, 10);
	ret = get_long_value_by_key(status_reply, "ieee80211ax", &he, 10);

	if (bw == CLSAPI_WIFI_BW_20 || bw == CLSAPI_WIFI_BW_20_NOHT)
		sec_chan_offset = WIFI_SEC_CHAN_OFFSET_NO_SEC;
	else
		sec_chan_offset = get_sec_chan_offset(band, channel);
	center_chan1 = (get_center_freq_chan(band, channel, sec_chan_offset, bw));
	center_freq1 = channel_to_freq_mhz(center_chan1, band);

	cmd_len = snprintf(cmd, sizeof(cmd), cmd_fmt_bw_non80_80, csa_cnt, freq,
					sec_chan_offset, center_freq1, bw_enum2int(bw), ht?"ht":"", vht?"vht":"", he?"he":"");

	return hostapd_ctrl_cmd_set(primary_ifname, cmd, cmd_len);
}

int clsapi_wifi_set_bw(const char *phyname, const enum clsapi_wifi_bw bw)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band = CLSAPI_WIFI_BW_DEFAULT;
	enum clsapi_wifi_hwmode hwmode;
	string_32 section;
	string_1024 str_htmode;
	enum clsapi_wifi_bw old_bw;
	uint8_t channel = 0;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	band = get_phy_band(phyname);
	phyname_to_radio_name(phyname, section);

	clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "htmode", str_htmode);
	htmode_to_hwmode_bw(str_htmode, &hwmode, &old_bw);

	ret = validate_bw(hwmode, band, bw);
	if (ret < 0)
		return ret;

	switch (hwmode) {
	case CLSAPI_HWMODE_IEEE80211_A:
	case CLSAPI_HWMODE_IEEE80211_B:
	case CLSAPI_HWMODE_IEEE80211_G:
		// legacy, no bw
		return -CLSAPI_ERR_INVALID_BW;
	case CLSAPI_HWMODE_IEEE80211_N:
		sprintf(str_htmode, "HT%d", bw_enum2int(bw));
		break;
	case CLSAPI_HWMODE_IEEE80211_AC:
		sprintf(str_htmode, "VHT%d", bw_enum2int(bw));
		break;
	case CLSAPI_HWMODE_IEEE80211_AX:
		sprintf(str_htmode, "HE%d", bw_enum2int(bw));
		break;
	default:
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	ret = clsapi_wifi_get_channel(phyname, &channel);
	if (ret < 0)
		return ret;
#if 0
	ret = local_wifi_switch_bw(phyname, channel, bw);
	if (ret < 0)
		return ret;

	clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "htmode", str_htmode);
#else
	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "htmode", str_htmode);
#endif

	return ret;
}

int local_wifi_get_chans(const char *phyname, const enum clsapi_wifi_band band,
		const enum clsapi_wifi_bw bw, uint8_t **channels)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band current_band = CLSAPI_BAND_DEFAULT;
	enum clsapi_wifi_band tgt_band = CLSAPI_BAND_DEFAULT;
	enum clsapi_wifi_bw tgt_bw = CLSAPI_WIFI_BW_DEFAULT;
	uint8_t local_channels[255] = {0}, local_chan_len = ARRAY_SIZE(local_channels);
	enum clsapi_wifi_hwmode hwmode = 0;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	current_band = get_phy_band(phyname);

	if (band == CLSAPI_BAND_DEFAULT)
		tgt_band = current_band;
	else {
		// Check if target band is supported
		ret = validate_band(phyname, band);
		if (ret < 0)
			return ret;

		tgt_band = band;
	}

	if (bw == CLSAPI_WIFI_BW_DEFAULT) {
		ret = clsapi_wifi_get_bw(phyname, &tgt_bw);
		if (ret < 0)
			return ret;
	} else
		tgt_bw = bw;

	ret = clsapi_wifi_get_hwmode(phyname, &hwmode);
	if (ret)
		return ret;

	ret = validate_bw(hwmode, band, tgt_bw);
	if (ret)
		return ret;

	ret = nl80211_get_chan_list(primary_ifname, tgt_band, tgt_bw, local_channels, &local_chan_len);
	if (ret < 0)
		return ret;

	*channels = (uint8_t *)calloc(local_chan_len, sizeof(uint8_t));
	if (*channels == NULL)
		return -CLSAPI_ERR_NO_MEM;

	memcpy(*channels, local_channels, local_chan_len);

	return local_chan_len;
}

int clsapi_wifi_get_supported_channels(const char *phyname, const enum clsapi_wifi_band band,
		const enum clsapi_wifi_bw bw, uint8_t **channels)
{
	return local_wifi_get_chans(phyname, band, bw, channels);
}

int clsapi_wifi_get_channel(const char *phyname, uint8_t *channel)
{
	int ret = CLSAPI_OK;
	int freq = -1;
	clsapi_ifname primary_ifname = {0};
	string_1024 current_chan = {0};
	string_32 radio_section;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!channel) {
		DBG_ERROR("Invalid parameter--channel is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = nl80211_get_attr_value(primary_ifname, NL80211_CMD_GET_INTERFACE, NULL, NL80211_ATTR_WIPHY_FREQ, &freq, sizeof(uint32_t));
	if (ret < 0) {
		phyname_to_radio_name(phyname, radio_section);
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, radio_section, "channel", current_chan);
		if (ret < 0)
			return ret;

		*channel = atoi(current_chan);
	} else
		*channel = freq_mhz_to_channel(freq);

	return CLSAPI_OK;
}

int local_wifi_switch_chan(const char *phyname, const uint8_t channel, const enum clsapi_wifi_bw bw)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band;
	char cmd[256], status_reply[2048];
	int cmd_len = 0, csa_cnt = 10;
	size_t status_reply_len = sizeof(status_reply);
	long ht, vht, he;
	int sec_chan_offset = 0, freq = 0, center_chan1 = 0, center_freq1 = 0;
	const char *cmd_fmt_bw_non80_80 = "CHAN_SWITCH %d %d sec_channel_offset=%d center_freq1=%d center_freq2=0 bandwidth=%d blocktx %s %s %s";
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	band = get_phy_band(phyname);
	ret = validate_channel(phyname, channel, band, bw);
	if (ret < 0) {
		DBG_ERROR("Invalid parameter--channel is invalid\n");
		return -CLSAPI_ERR_INVALID_CHANNEL;
	}

	freq = channel_to_freq_mhz(channel, band);
	if (!freq) {
		DBG_ERROR("Invalid parameter--frequency convert to channel failed\n");
		return -CLSAPI_ERR_INVALID_CHANNEL;
	}

	// get ht/vht/he status
	ret = hostapd_ctrl_cmd_get(primary_ifname, "STATUS", sizeof("STATUS"), status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0)
		return -CLSAPI_ERR_NOT_FOUND;
	ret = get_long_value_by_key(status_reply, "ieee80211n", &ht, 10);
	ret = get_long_value_by_key(status_reply, "ieee80211ac", &vht, 10);
	ret = get_long_value_by_key(status_reply, "ieee80211ax", &he, 10);

	if (bw == CLSAPI_WIFI_BW_20 || bw == CLSAPI_WIFI_BW_20_NOHT)
		sec_chan_offset = WIFI_SEC_CHAN_OFFSET_NO_SEC;
	else
		sec_chan_offset = get_sec_chan_offset(band, channel);
	center_chan1 = (get_center_freq_chan(band, channel, sec_chan_offset, bw));
	center_freq1 = channel_to_freq_mhz(center_chan1, band);

	cmd_len = snprintf(cmd, sizeof(cmd), cmd_fmt_bw_non80_80, csa_cnt, freq,
					sec_chan_offset, center_freq1, bw_enum2int(bw), ht?"ht":"", vht?"vht":"", he?"he":"");

	return hostapd_ctrl_cmd_set(primary_ifname, cmd, cmd_len);
}

int clsapi_wifi_set_channel(const char *phyname, const uint8_t channel, const enum clsapi_wifi_band band,
		const enum clsapi_wifi_bw bw)
{
	int ret, int_old_chan = 0;
	enum clsapi_wifi_band old_band, tgt_band;
	string_32 radio_section;
	string_1024 str_old_chan;
	enum clsapi_wifi_bw tgt_bw = CLSAPI_WIFI_BW_DEFAULT;
	enum clsapi_wifi_hwmode hwmode = 0;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	old_band = get_phy_band(phyname);

	// CLSAPI_BAND_DEFAULT means current operating band.
	if (band == CLSAPI_BAND_DEFAULT) {
		tgt_band = old_band;
	} else {
		// Check if target band is supported
		ret = validate_band(phyname, band);
		if (ret < 0)
			return ret;

		tgt_band = band;
	}

	ret = validate_ap_mode(phyname);
	if (ret < 0)
		return ret;

	if (bw == CLSAPI_WIFI_BW_DEFAULT) {
		ret = clsapi_wifi_get_bw(phyname, &tgt_bw);
		if (ret < 0)
			return ret;
	} else
		tgt_bw = bw;

	ret = clsapi_wifi_get_hwmode(phyname, &hwmode);
	if (ret)
		return ret;

	ret = validate_bw(hwmode, tgt_band, tgt_bw);
	if (ret)
		return ret;

	ret = validate_channel(phyname, channel, tgt_band, tgt_bw);
	if (ret < 0)
		return ret;

	phyname_to_radio_name(phyname, radio_section);

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, radio_section, "channel", str_old_chan);
	if (ret < 0)
		return ret;

	if (strcmp(str_old_chan, "auto") == 0) {
		// old channel is "auto"
		int_old_chan = 0;
	} else {
		int_old_chan = atoi(str_old_chan);
		if (int_old_chan == 0) // "0" or non-number, both invalid
			return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	if (channel) {
		// TODO: disable ACS
		// switch to given channel runtimely.
#if 0
		ret = local_wifi_switch_chan(phyname, channel, tgt_bw);
		if (ret < 0)
			return ret;

		clsconf_set_int_param(CLSCONF_CFG_WIRELESS, radio_section, "channel", channel);
#else
		clsconf_defer_apply_int_param(CLSCONF_CFG_WIRELESS, radio_section, "channel", channel);
#endif
	} else {
		// TODO, call runtime ACS enabling API.
		clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, radio_section, "channel", "auto");
	}
	/* set bw */
	string_1024 str_htmode;

		switch (hwmode) {
		case CLSAPI_HWMODE_IEEE80211_A:
		case CLSAPI_HWMODE_IEEE80211_B:
		case CLSAPI_HWMODE_IEEE80211_G:
			// legacy, no bw
			return -CLSAPI_ERR_INVALID_BW;
		case CLSAPI_HWMODE_IEEE80211_N:
			sprintf(str_htmode, "HT%d", bw_enum2int(tgt_bw));
			break;
		case CLSAPI_HWMODE_IEEE80211_AC:
			sprintf(str_htmode, "VHT%d", bw_enum2int(tgt_bw));
			break;
		case CLSAPI_HWMODE_IEEE80211_AX:
			sprintf(str_htmode, "HE%d", bw_enum2int(tgt_bw));
			break;
		default:
			return -CLSAPI_ERR_NOT_SUPPORTED;
	}
#if 0
	clsconf_set_param(CLSCONF_CFG_WIRELESS, radio_section, "htmode", str_htmode);
#else
	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, radio_section, "htmode", str_htmode);
#endif

	return ret;
}

int clsapi_wifi_get_ssid(const char *ifname, string_32 ssid)
{
	int ret = CLSAPI_OK;
	string_1024 local_ssid = {0};

	if (!ifname || !ssid) {
		DBG_ERROR("Invalid parameter--interface name or ssid is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_ifname(ifname)) {
		DBG_ERROR("Invalid parameter--interface name is not validated.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	ret = nl80211_get_attr_value(ifname, NL80211_CMD_GET_INTERFACE, NULL, NL80211_ATTR_SSID, local_ssid,
			sizeof(string_1024));
	if (ret < 0)
		return ret;

	cls_strncpy(ssid, local_ssid, sizeof(string_32));

	return ret;
}

int clsapi_wifi_set_ssid(const char *ifname, const char *ssid)
{
	int ret = CLSAPI_OK;
	string_64 section;

	if (!ifname || !ssid) {
		DBG_ERROR("Invalid parameter--interface name or ssid is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_ifname(ifname)) {
		DBG_ERROR("Invalid parameter--interface name is not validated.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "ssid", ssid);

	return ret;
}

int clsapi_wifi_add_bss(const char *phyname, const uint8_t bssid[ETH_ALEN], string_32 created_ifname)
{
	int ret =CLSAPI_OK, phy_idx = -1;
	string_32 section_name = {0};
	string_32 radio_name, def_ssid, str_bssid;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	// add bss cfg section
	ret = clsconf_add_section(CLSCONF_CFG_WIRELESS, "wifi-iface", CLSCONF_SECNAME_USE_ID, section_name);

	// prepare and set default parameters
	ret = phyname_to_radio_name(phyname, radio_name);
	if (ret < 0)
		return ret;

	phy_idx = phyname_to_idx(phyname);
	if (phy_idx == CLS_2G_RADIO_IDX)
		strcpy(def_ssid, CLS_WIFI_DEF_SSID);
	else if (phy_idx == CLS_5G_RADIO_IDX)
		strcpy(def_ssid, CLS_WIFI_DEF_SSID_5G);
	else {
		DBG_ERROR("Invalid parameter--phyname is not validated, phy_id is invalid.\n");
		return -CLSAPI_ERR_INVALID_PHYNAME;
	}

	clsconf_set_param(CLSCONF_CFG_WIRELESS, section_name, "device", radio_name);
	if (bssid && !validate_unicast_macaddr(bssid)) {
		sprintf(str_bssid, MACFMT, MACARG(bssid));
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section_name, "macaddr", str_bssid);
	}
	clsconf_set_param(CLSCONF_CFG_WIRELESS, section_name, "network", "lan");
	clsconf_set_param(CLSCONF_CFG_WIRELESS, section_name, "mode", "ap");
	clsconf_set_param(CLSCONF_CFG_WIRELESS, section_name, "ssid", def_ssid);
	clsconf_set_param(CLSCONF_CFG_WIRELESS, section_name, "encryption", "psk2");
	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section_name, "key", "12345678");
	if (ret < 0)
		return ret;

	// return back created ifname
	ret = clsconf_wifi_section_name_to_ifname(section_name, created_ifname);

	return ret;
}

int clsapi_wifi_del_bss(const char *ifname)
{
	int ret = CLSAPI_OK;
	string_32 bss_section;
	int sec_len = sizeof(bss_section);

	ret = validate_ap_ifname(ifname);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, bss_section, sec_len);
	if (ret < 0)
		return ret;

	clsconf_defer_del_section(CLSCONF_CFG_WIRELESS, bss_section);

	return ret;
}

int clsapi_wifi_get_bssid(const char *ifname, uint8_t bssid[ETH_ALEN])
{
	int ret = validate_ap_ifname(ifname);

	if (ret < 0)
		return ret;

	if (!bssid) {
		DBG_ERROR("Invalid parameter--bssid is not validated.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = nl80211_get_attr_value(ifname, NL80211_CMD_GET_INTERFACE, NULL, NL80211_ATTR_MAC, bssid, ETH_ALEN);

	return ret;
}

int clsapi_wifi_get_bss_enabled(const char *ifname, bool *onoff)
{
	int ret = CLSAPI_OK;
	string_32 section;
	string_1024 status;

	if (!onoff) {
		DBG_ERROR("Invalid parameter--onoff is not validated.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "disabled", status);
	if (ret && ret != -CLSAPI_ERR_NOT_FOUND) {
		DBG_ERROR("Invalid parameter--status is not validated.\n");
		return ret;
	}

	if (strcmp(status, "1") == 0)
		*onoff = 0;
	else if (strcmp(status, "0") == 0 || ret == -CLSAPI_ERR_NOT_FOUND)
		*onoff = 1;
	else {
		DBG_ERROR("Invalid parameter--status: %s is not validated.\n", status);
		return -CLSAPI_ERR_UCI;
	}

	return CLSAPI_OK;
}

int clsapi_wifi_enable_bss(const char *ifname, const bool onoff)
{
	int ret = CLSAPI_OK;
	bool old_onoff = 0;
	string_32 section;

	if (onoff != 0 && onoff != 1)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_get_bss_enabled(ifname, &old_onoff);
	if (ret < 0)
		return ret;
	if (onoff == old_onoff) {
		return CLSAPI_OK;
	}

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "disabled", onoff ? "0" : "1");

	return ret;
}

int clsapi_wifi_get_encryption(const char *ifname, enum clsapi_wifi_encryption *encryption)
{
	int ret = CLSAPI_OK;
	string_64 section;
	string_1024 str_value;

	if (!encryption) {
		DBG_ERROR("Invalid parameter--encryption is invalid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_ifname(ifname)) {
		DBG_ERROR("Invalid parameter--interface name is invalid.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "encryption", str_value);
	if (ret < 0)
		return ret;

	*encryption = encryption_str2enum(str_value);
	if (*encryption >= CLSAPI_WIFI_ENCRYPTION_MAX) {
		DBG_ERROR("Invalid data--encryption is over the range.\n");
		return -CLSAPI_ERR_INVALID_DATA;
	}

	return ret;
}

int clsapi_wifi_set_encryption(const char *ifname, const enum clsapi_wifi_encryption encryption)
{
	int ret = CLSAPI_OK;
	string_64 section;
	string_1024 str_value;

	if (validate_encryption(encryption))
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));

	switch (encryption) {
	//process for no encryption, do nothing
	case CLSAPI_WIFI_ENCRYPTION_OPEN_NONE:
	case CLSAPI_WIFI_ENCRYPTION_OWE:
		break;

	//process for preshared key
	case CLSAPI_WIFI_ENCRYPTION_WEP_MIXED:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP:
	// WAPI mode maybe need to be changed later
	case CLSAPI_WIFI_ENCRYPTION_WAPI_PSK:
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "key", str_value);
		if (ret == -CLSAPI_ERR_NOT_FOUND) {
			DBG_ERROR("Invalid data--key value is not found.\n");
			return -CLSAPI_ERR_NO_PASSPHRASE;
		} else if (ret != CLSAPI_OK)
			return ret;

		if (encryption == CLSAPI_WIFI_ENCRYPTION_WEP_MIXED) {
			int key_type = atoi(str_value);

			if (key_type <= 0 || key_type > 4) {
				DBG_ERROR("Internal error--wep-mixed mode needs specific key type.\n");
				return -CLSAPI_ERR_INVALID_DATA;
			}
			string_32 wep_key_type = {0};

			snprintf(wep_key_type, sizeof(wep_key_type), "key%d", key_type);

			memset(str_value, 0, sizeof(str_value));
			ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, wep_key_type, str_value);
			if (ret == -CLSAPI_ERR_NOT_FOUND) {
				DBG_ERROR("Invalid data--wep key value is not found.\n");
				return -CLSAPI_ERR_NO_PASSPHRASE;
			} else if (ret != CLSAPI_OK)
				return ret;
			if (strlen(str_value) != 7 && strlen(str_value) != 15 &&
					strlen(str_value) != 10 && strlen(str_value) != 26) {
				DBG_ERROR("Internal error--wep-mixed mode key length is not correct.\n");
				return -CLSAPI_ERR_INVALID_DATA;
			}
		} else {
			ret = validate_passphrase(ifname, str_value);
			if (ret != CLSAPI_OK)
				return ret;
		}

		break;

	//process for radius configuration
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP:
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "auth_server", str_value);
		if (ret == -CLSAPI_ERR_NOT_FOUND)
			return -CLSAPI_ERR_NO_PASSPHRASE;
		else if (ret != CLSAPI_OK)
			return ret;
		break;
	default:
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (validate_ifname(ifname)) {
		DBG_ERROR("Invalid parameter--interface name is not valid.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "encryption", encryption_enum2str(encryption));

	return ret;
}

int clsapi_wifi_get_passphrase(const char *ifname, string_64 passphrase)
{
	int ret = CLSAPI_OK;
	string_64 section;
	string_1024 str_value;

	if (!passphrase) {
		DBG_ERROR("Invalid parameter--passphrase is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}
	if ((ret = validate_passphrase(ifname, NULL)) != CLSAPI_OK)
		return ret;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "key", str_value);
	if (ret < 0)
		return ret;

	cls_strncpy(passphrase, str_value, sizeof(string_64));
	return CLSAPI_OK;
}

int clsapi_wifi_set_passphrase(const char *ifname, const char *passphrase)
{
	int ret = CLSAPI_OK;
	string_64 section;
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if (!passphrase) {
		DBG_ERROR("Invalid parameter--passphrase is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}
	if ((ret = validate_passphrase(ifname, passphrase)) != CLSAPI_OK)
		return ret;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (encryption == CLSAPI_WIFI_ENCRYPTION_WEP_MIXED) {
		//delete encryption option/ key
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "key1", "");
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "key2", "");
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "key3", "");
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "key4", "");
		ret = clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "encryption", "none");
		if (ret < 0)
			return ret;
	}
	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "key", passphrase);

	return ret;
}

int clsapi_wifi_get_wep_key(const char *ifname, int *idx_in_use, char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64])
{
	int ret = CLSAPI_OK;
	string_64 section, str_key_option;
	string_1024 str_value;

	if (!idx_in_use || !wep_key) {
		DBG_ERROR("Invalid parameter--idx_in_use/wep_key is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}
	if ((ret = validate_wep_key_mode(ifname)) != CLSAPI_OK)
		return ret;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "key", str_value);
	*idx_in_use = atoi(str_value) - 1; // in conf, idx starting from 1
	if (*idx_in_use < 0 || *idx_in_use >= CLSAPI_WIFI_MAX_PRESHARED_KEY) {
		DBG_ERROR("Invalid parameter--idx_in_use is not in range.\n");
		return -CLSAPI_ERR_METHOD_NOT_FOUND;
	}

	for (int i = 0; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++) {
		sprintf(str_key_option, "key%d", i + 1);
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, str_key_option, str_value);
		if (ret == CLSAPI_OK)
			cls_strncpy(wep_key[i], str_value, sizeof(wep_key[0]));
		else if (ret == -CLSAPI_ERR_NOT_FOUND) {
			if (i == *idx_in_use) { // key in use MUST NOT empty
				DBG_ERROR("Invalid parameter--idx_in_use is not found.\n");
				return -CLSAPI_ERR_NOT_FOUND;
			} else
				strcpy(wep_key[i], "");
		} else
			return ret;
	}

	return CLSAPI_OK;
}

int clsapi_wifi_set_wep_key(const char *ifname, const int idx_to_use, const char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64])
{
	int ret = CLSAPI_OK, defer_mode;
	string_64 section, str_key_option, str_value;
	string_64 str_wep_key = {0};
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if (idx_to_use <= 0 || idx_to_use > CLSAPI_WIFI_MAX_PRESHARED_KEY) {
		DBG_ERROR("Invalid parameter--idx_in_use is not in range.\n");
		return -CLSAPI_ERR_METHOD_NOT_FOUND;
	}

	if (!wep_key || strcmp(wep_key[idx_to_use - 1], "") == 0) {
		DBG_ERROR("Invalid parameter--wep_key is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	clsapi_base_get_defer_mode(&defer_mode);
	sprintf(str_value, "%d", idx_to_use); // in conf, idx starting from 1

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	if (encryption != CLSAPI_WIFI_ENCRYPTION_WEP_MIXED) {
		ret = clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "encryption", "none");
		if (ret < 0)
			return ret;
	}

	ret = clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "key", str_value);
	if (ret < 0)
		return ret;
	for (int i = 0; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++) {
		snprintf(str_key_option, sizeof(str_key_option), "key%d", i + 1);
		if (strcmp(wep_key[i], "") == 0)
			continue;

		if (strlen(wep_key[i]) == 5 || strlen(wep_key[i]) == 13) {
			snprintf(str_wep_key, sizeof(str_wep_key), "s:%s", wep_key[i]);
		} else if (strlen(wep_key[i]) == 10 || strlen(wep_key[i]) == 26) {
			snprintf(str_wep_key, sizeof(str_wep_key), "%s", wep_key[i]);
		} else {
			DBG_ERROR("Invalid parameter--wep key value is not valid.\n");
			return -CLSAPI_ERR_INVALID_PARAM;
		}

		ret = clsconf_set_param(CLSCONF_CFG_WIRELESS, section, str_key_option, str_wep_key);
		if (ret < 0)
			return ret;
	}

	if (defer_mode == 0)
		ret = clsconf_apply_cfg(CLSCONF_CFG_WIRELESS);

	return ret;
}

int clsapi_wifi_get_vbss_enabled(const char *phyname, bool *onoff)
{
	return nl80211_get_cls_attr_value(phyname, CLS_NL80211_CMD_GET_VBSS_ENABLED, NULL,
				CLS_NL80211_ATTR_VBSS_ENABLED, onoff, sizeof(bool));
}


int clsapi_wifi_set_vbss_enabled(const char *phyname, const bool onoff)
{
	int ret;
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_VBSS_ENABLED, .value = (void *)&onoff, .val_len = sizeof(bool)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	ret = hostapd_set_vbss_enabled(phyname, onoff);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(phyname, CLS_NL80211_CMD_SET_VBSS_ENABLED, &put_attrs);
}


int clsapi_wifi_get_vbss_vap(const char *ifname, struct vbss_vap_info *vap)
{
	int ret = CLSAPI_OK;

	if (!ifname)
		return -CLSAPI_ERR_INVALID_PARAM;
	ret = nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_GET_VBSS_VAP, NULL,
				CLS_NL80211_ATTR_VBSS_VAP_INFO, vap, sizeof(struct vbss_vap_info));
	if (ret < 0)
		return ret;
	ret = hostapd_get_vbss_vap(ifname, vap);
	return ret;
}


int clsapi_wifi_add_vbss_vap(const struct vbss_vap_info *vap, char *new_ifname, const int name_len)
{
	uint16_t aid = 0;
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_VBSS_VAP_INFO, .value = (void *)vap, .val_len = sizeof(struct vbss_vap_info)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};


	if (!vap || !new_ifname || !name_len) {
		DBG_ERROR("Invalid parameter--vap/new_ifname/name_len is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	aid = ((vap->bssid[4] << 8) | vap->bssid[5]) & 0x7ff;

	if (aid < 1 || aid > 2007) {
		DBG_ERROR("Invalid parameter--%s:aid:0x%04x is not in range.\n", __func__, aid);
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (vap->auth_type == WPA_AUTHORIZE_TYPE_WPA2
		&& (strlen(vap->pwd) > 63 || strlen(vap->pwd) < 8)) {
		DBG_ERROR("Invalid parameter--invalid passwd(out of valid length).\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = hostapd_add_vbss_vap(vap, new_ifname, name_len);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(new_ifname, CLS_NL80211_CMD_SET_VBSS_VAP, &put_attrs);

	return ret;
}

int clsapi_wifi_stop_vbss_vap_txq(const char *ifname)
{
	uint8_t stop = 1;
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_VAP_TXQ, .value = &stop, .val_len = sizeof(uint8_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_set_cls_attr_value(ifname, CLS_NL80211_CMD_STOP_VAP_TXQ, &put_attrs);
}

int clsapi_wifi_del_vbss_vap(const char *ifname)
{
	if (!ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	return hostapd_del_vbss_vap(ifname);
}


int clsapi_wifi_get_vbss_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN],
		struct vbss_sta_info *sta)
{
	int ret;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	ret = hostapd_get_vbss_sta_info(ifname, sta_mac, sta);
	if (ret < 0)
		return ret;

	ret = nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_GET_VBSS_STA, &put_attrs,
				CLS_NL80211_ATTR_VBSS_STA_INFO, &sta->driver_sta, sizeof(struct cls_vbss_driver_sta_info));

	return ret;
}


int clsapi_wifi_add_vbss_sta(const char *ifname, const struct vbss_sta_info *sta)
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr[] = {
		{.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta->mac_addr, .val_len = ETH_ALEN},
		{.id = CLS_NL80211_ATTR_VBSS_STA_INFO, .value = (void *)(&sta->driver_sta),
			.val_len = sizeof(struct cls_vbss_driver_sta_info)}
	};
	struct nl80211_attr_tbl put_attrs = { .n_attr = ARRAY_SIZE(attr), .attrs = attr};

	if (!ifname || !sta)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = hostapd_add_vbss_sta(ifname, sta);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(ifname, CLS_NL80211_CMD_SET_VBSS_STA, &put_attrs);
}


int clsapi_wifi_del_vbss_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	if (!ifname || !sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	return hostapd_del_vbss_sta(ifname, sta_mac);
}


int clsapi_wifi_trigger_vbss_switch(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!ifname || !sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = hostapd_trigger_vbss_switch(ifname, sta_mac);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(ifname, CLS_NL80211_CMD_TRG_VBSS_SWITCH, &put_attrs);
}


int clsapi_wifi_get_vbss_roam_result(const char *ifname, const uint8_t sta_mac[ETH_ALEN], uint32_t *roam_result)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!ifname || !sta_mac || !roam_result)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_GET_VBSS_ROAM_RESULT, &put_attrs,
				CLS_NL80211_ATTR_ROAM_RESULT, roam_result, sizeof(uint32_t));
}


int clsapi_wifi_set_vbss_switch_done(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	if (!ifname || !sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	return hostapd_set_vbss_switch_done(ifname, sta_mac);
}


int clsapi_wifi_get_vbss_nthresh(const char *phyname, uint32_t *n_thresh)
{
	if (!phyname || !n_thresh)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(phyname, CLS_NL80211_CMD_GET_VBSS_NTHRESH, NULL,
				CLS_NL80211_ATTR_VBSS_NTHRESH, n_thresh, sizeof(uint32_t));
}


int clsapi_wifi_set_vbss_nthresh(const char *phyname, const uint32_t n_thresh)
{
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_VBSS_NTHRESH, .value = (void *)&n_thresh, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = { .n_attr = 1, .attrs = &attr };

	if (!phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_set_cls_attr_value(phyname, CLS_NL80211_CMD_SET_VBSS_NTHRESH, &put_attrs);
}


int clsapi_wifi_get_vbss_mthresh(const char *phyname, uint32_t *m_thresh)
{
	if (!phyname || !m_thresh)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(phyname, CLS_NL80211_CMD_GET_VBSS_MTHRESH, NULL,
				CLS_NL80211_ATTR_VBSS_MTHRESH, m_thresh, sizeof(uint32_t));
}


int clsapi_wifi_set_vbss_mthresh(const char *phyname, const uint32_t m_thresh)
{
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_VBSS_MTHRESH, .value = (void *)&m_thresh, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = { .n_attr = 1, .attrs = &attr };

	if (!phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_set_cls_attr_value(phyname, CLS_NL80211_CMD_SET_VBSS_MTHRESH, &put_attrs);
}


int clsapi_wifi_get_rssi_smoothness_factor(const char *phyname, uint8_t *factor)
{
	if (!phyname || !factor)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(phyname, CLS_NL80211_CMD_GET_RSSI_SMOOTHNESS_FACTOR, NULL,
				CLS_NL80211_ATTR_RSSI_SMOOTHNESS_FACTOR, factor, sizeof(uint8_t));
}


int clsapi_wifi_set_rssi_smoothness_factor(const char *phyname, const uint8_t factor)
{
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_RSSI_SMOOTHNESS_FACTOR, .value = (void *)&factor, .val_len = sizeof(uint8_t)};
	struct nl80211_attr_tbl put_attrs = { .n_attr = 1, .attrs = &attr };

	if (!phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_set_cls_attr_value(phyname, CLS_NL80211_CMD_SET_RSSI_SMOOTHNESS_FACTOR, &put_attrs);
}


int clsapi_wifi_get_sta_info(const char *ifname, const uint8_t sta_mac[ETH_ALEN], struct sta_info *sta)
{
	if (!ifname || !sta_mac || !sta)
		return -CLSAPI_ERR_NULL_POINTER;

	memset(sta, 0, sizeof(struct sta_info));

	return nl80211_get_assoc_info(ifname, sta_mac, sta);
}


int clsapi_wifi_get_sinr(const char *ifname, const uint8_t sta_mac[ETH_ALEN], int8_t *sinr)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!ifname || !sta_mac || !sinr)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_GET_SINR, &put_attrs,
				CLS_NL80211_ATTR_SINR, sinr, sizeof(*sinr));
}


int clsapi_wifi_get_rssi(const char *ifname, const uint8_t sta_mac[ETH_ALEN], int8_t *rssi)
{
	int ret;
	struct sta_info sta_info;

	if (!ifname || !sta_mac || !rssi)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = nl80211_get_assoc_info(ifname, sta_mac, &sta_info);
	if (ret < 0)
		return ret;

	*rssi = sta_info.signal;
	return CLSAPI_OK;
}

int local_get_survey(const char *phyname, struct survey_entry **survey_entry)
{
	int ret = CLSAPI_OK, len;
	char buf[SURVEY_ENTRY_BUFSIZE];
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = nl80211_get_survey(primary_ifname, buf, &len);
	if (ret < 0)
		return -CLSAPI_ERR_UNKNOWN_ERROR;

	if (len % (sizeof(struct survey_entry)))
		return -CLSAPI_ERR_INTERNAL_ERR;

	*survey_entry = (struct survey_entry *)malloc(len);
	if (*survey_entry == NULL)
		return -CLSAPI_ERR_NO_MEM;

	memcpy(*survey_entry, buf, len);

	return len / sizeof(struct survey_entry);
}

int clsapi_wifi_get_survey(const char *phyname, struct survey_entry **survey_entry)
{
	return local_get_survey(phyname, survey_entry);
}

int clsapi_wifi_get_noise(const char *phyname, int8_t *noise)
{
	int i = 0;
	uint32_t freq;
	uint8_t channel;
	int ret = CLSAPI_OK;
	int survey_entry_len = 0;
	enum clsapi_wifi_band band;
	struct survey_entry *survey = NULL;

	if (!noise) {
		DBG_ERROR("Invalid parameter--noise is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	ret = validate_phyname(phyname);
	if (ret)
		return ret;

	ret = clsapi_wifi_get_channel(phyname, &channel);
	if (ret)
		return ret;

	band = get_phy_band(phyname);
	freq = channel_to_freq_mhz(channel, band);
	if (!freq) {
		DBG_ERROR("Invalid data--frequency is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	survey_entry_len = local_get_survey(phyname, &survey);
	for (i = 0; i < survey_entry_len; i++)
		if (survey[i].mhz == freq) {
			*noise = survey[i].noise;
			break;
		}

	/* does NOT match freq, return CLSAPI_ERR_NO_DATA */
	if (i == survey_entry_len)
		ret = -CLSAPI_ERR_NO_DATA;

	if (survey)
		free(survey);

	return ret;
}

int clsapi_wifi_get_vbss_ap_stats(const char *ifname, struct vbss_ap_stats *stats)
{
	int ret = CLSAPI_OK;

	if (!ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_GET_VBSS_AP_STATS, NULL,
				CLS_NL80211_ATTR_VBSS_AP_STATS, stats, sizeof(struct vbss_ap_stats));

	return ret;
}

/*
 * score = 10^(chan_nf/5)+(busy_time-tx_time)/(active_time-tx_time)×2^(10^(chan_nf/10)+10^(band_min_nf/10))
 */
double calc_channel_score(uint64_t active_time, uint64_t busy_time, uint64_t tx_time,
						int chan_nf, int band_min_nf)
{
	if (active_time <= tx_time)
		return pow(10.0, chan_nf / 5.0);

	double busy_ratio = (double)(busy_time - tx_time) / (active_time - tx_time);
	double nf_chan_db10 = pow(10.0, chan_nf / 10.0);
	double nf_band_db10 = pow(10.0, band_min_nf / 10.0);
	double noise_multiplier = pow(2.0, nf_chan_db10 + nf_band_db10);

	if (busy_ratio < 0.0)
		busy_ratio = 0.0;

	if (busy_ratio > 1.0)
		busy_ratio = 1.0;

	double score = pow(10.0, chan_nf / 5.0) + busy_ratio * noise_multiplier;

	return score;
}

int clsapi_wifi_get_chan_score(const char *phyname, const enum clsapi_wifi_band band,
	struct chan_score **score_entries)
{
	int ret = CLSAPI_OK;
	double d_scores, score_min = 0, score_max = 0;
	int band_min_nf = 0;
	int channel_len;
	struct survey_entry *survey = NULL;
	struct chan_score *tmp_score = NULL;

	ret = validate_phyname(phyname);
	if (ret)
		return ret;

	if (band != CLSAPI_BAND_DEFAULT)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	ret = clsapi_wifi_get_survey(phyname, &survey);
	if (ret < 0)
		return ret;
	channel_len = ret;

	band_min_nf = survey[0].noise;
	for (int i = 0; i < channel_len; i++) {
		if (survey[i].noise < band_min_nf)
			band_min_nf = survey[i].noise;
		else
			continue;
	}

	tmp_score = (struct chan_score *)calloc(channel_len, sizeof(struct chan_score));
	if (!tmp_score)
		return -CLSAPI_ERR_NO_MEM;

	*score_entries = tmp_score;

	score_max = score_min = calc_channel_score(survey[0].active_time, survey[0].busy_time,
			survey[0].txtime, survey[0].noise, band_min_nf);

	for (int i = 0; i < channel_len; i++) {
		d_scores = calc_channel_score(survey[i].active_time, survey[i].busy_time,
				survey[i].txtime, survey[i].noise, band_min_nf);
		if (d_scores > score_max)
			score_max = d_scores;
		else if (d_scores < score_min)
			score_min = d_scores;
		else
			continue;
	}

	for (int i = 0; i < channel_len; i++) {
		tmp_score->chan = freq_mhz_to_channel(survey[i].mhz);
		d_scores = calc_channel_score(survey[i].active_time, survey[i].busy_time,
				survey[i].txtime, survey[i].noise, band_min_nf);
		tmp_score->score = (uint16_t)(65535.0 * (d_scores - score_min) / (score_max - score_min));
		tmp_score++;
	}

	if (survey)
		free(survey);

	return channel_len;
}


int clsapi_wifi_get_supported_bands(const char *phyname, enum clsapi_wifi_band **band_array)
{
	int phy_idx = -1;
	enum clsapi_wifi_band band = CLSAPI_BAND_NOSUCH_BAND;
	int bands_cnt = 0;

	if (validate_phyname(phyname) < 0)
		return -CLSAPI_ERR_INVALID_PHYNAME;

	phy_idx = phyname_to_idx(phyname);
	band = RADIO_IDX_TO_BAND(phy_idx);

	// for AP, band is fixed per radio
	bands_cnt = 1;
	*band_array = (enum clsapi_wifi_band *)calloc(bands_cnt, sizeof(enum clsapi_wifi_band));
	if (*band_array == NULL)
		return -CLSAPI_ERR_NO_MEM;

	(*band_array)[0] = band;

	// for STA, TODO

	return bands_cnt;
}

int clsapi_wifi_get_band(const char *phyname, enum clsapi_wifi_band *band)
{
	int ret;
	uint32_t freq;
	clsapi_ifname primary_ifname;

	if (!band)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = nl80211_get_attr_value(primary_ifname, NL80211_CMD_GET_INTERFACE, NULL,
			NL80211_ATTR_WIPHY_FREQ, &freq, sizeof(uint32_t));
	if (ret < 0)
		return ret;

	*band = freq_mhz_to_band(freq);

	return CLSAPI_OK;
}

int clsapi_wifi_add_vbss_monitor_sta(const  char *phyname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!phyname || !sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = nl80211_set_cls_attr_value(phyname, CLS_NL80211_CMD_ADD_MONITOR_STA, &put_attrs);

	return ret;
}

int clsapi_wifi_del_vbss_monitor_sta(const  char *phyname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR, .value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!phyname || !sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = nl80211_set_cls_attr_value(phyname, CLS_NL80211_CMD_REMOVE_MONITOR_STA, &put_attrs);

	return ret;
}

int clsapi_wifi_get_mesh_role(enum clsapi_wifi_mesh_role *role)
{
	int ret = CLSAPI_OK;
	string_1024 mode;

	ret = clsconf_get_param(CLSCONF_CFG_CLS_MESH, CLS_MESH_SECT_DEFAULT, CLS_MESH_PARAM_MODE, mode);
	if (ret < 0)
		return ret;

	*role = mesh_role_str2enum(mode);
	if (*role >= CLSAPI_WIFI_MESH_ROLE_MAX) {
		DBG_ERROR("Invalid parameter--role is over the range.\n");
		return -CLSAPI_ERR_INVALID_DATA;
	}

	return ret;
}

int clsapi_wifi_set_mesh_role(const enum clsapi_wifi_mesh_role role)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_mesh_role old_role;

	/* "auto" will be supported later */
	if (role == CLSAPI_WIFI_MESH_ROLE_AUTO) {
		DBG_ERROR("Invalid parameter--role is auto, the auto is not supported.\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	ret = clsapi_wifi_get_mesh_role(&old_role);
	if (ret < 0)
		return ret;

	if (old_role == role)
		return CLSAPI_OK;

	/* Param validation */
	if (role >= CLSAPI_WIFI_MESH_ROLE_MAX) {
		DBG_ERROR("Invalid parameter--role is over the range.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	clsconf_defer_apply_param(CLSCONF_CFG_CLS_MESH, CLS_MESH_SECT_DEFAULT, CLS_MESH_PARAM_MODE, mesh_role_enum2str(role));
	return ret;
}

int clsapi_wifi_set_vbss_stop_rekey(const char *ifname)
{
	int ret = CLSAPI_OK;

	if (!ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = hostapd_set_vbss_stop_rekey(ifname);
	return ret;
}

#define CLSAPI_WPS_LABEL        "wps_label"
#define CLSAPI_WPS_DISPLAY      "wps_display"
#define CLSAPI_WPS_KEYPAD       "wps_keypad"
#define CLSAPI_WPS_PUSHBUTTON   "wps_pushbutton"
#define CLSAPI_WPS_PIN          "wps_pin"
#define CLSAPI_WPS_UUID         "wps_uuid"

static struct {
	const char *config_method;
	enum clsapi_wifi_wps_config_method method;
} wps_config_methods[] = {
	{CLSAPI_WPS_LABEL,      CLSAPI_WIFI_WPS_CFG_METHOD_LABEL},
	{CLSAPI_WPS_DISPLAY,    CLSAPI_WIFI_WPS_CFG_METHOD_DISPLAY},
	{CLSAPI_WPS_KEYPAD,     CLSAPI_WIFI_WPS_CFG_METHOD_KEYPAD},
	{CLSAPI_WPS_PUSHBUTTON, CLSAPI_WIFI_WPS_CFG_METHOD_PUSHBUTTON}
};

int clsapi_wifi_set_wps_config_method(const char *ifname, enum clsapi_wifi_wps_config_method wps_config_method)
{
	int i = 0, defer_mode = 0;
	int ret = CLSAPI_OK;
	char section[32] = {0};

	if (wps_config_method & ~wps_config_methods_mask) {
		DBG_ERROR("Invalid parameter--wps_config_method is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	while (i < ARRAY_SIZE(wps_config_methods)) {
		if (wps_config_method & wps_config_methods[i].method) {
			ret = clsconf_set_param(CLSCONF_CFG_WIRELESS, section,
					wps_config_methods[i].config_method, "1");
			if (ret < 0)
				return ret;
		} else {
			ret = clsconf_set_param(CLSCONF_CFG_WIRELESS, section,
					wps_config_methods[i].config_method, "0");
			if (ret < 0)
				return ret;
		}

		i++;
	}

	clsapi_base_get_defer_mode(&defer_mode);
	if (defer_mode == 0)
		ret = clsconf_apply_cfg(CLSCONF_CFG_WIRELESS);
	if (ret < 0)
		return ret;

	ret = clsapi_wifi_set_wps_state(ifname, CLSAPI_WIFI_WPS_STATE_CONFIGURED);

	return ret;
}

int clsapi_wifi_get_wps_config_method(const char *ifname, enum clsapi_wifi_wps_config_method *wps_config_method)
{
	int i = 0;
	int ret = CLSAPI_OK;
	char section[32] = {0};
	string_1024 method;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	*wps_config_method = CLSAPI_WIFI_WPS_CFG_METHOD_NOCONFIG;
	while (i < ARRAY_SIZE(wps_config_methods)) {
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, wps_config_methods[i].config_method, method);
		if (!ret)
			if (strcmp(method, "1") == 0)
				*wps_config_method |= wps_config_methods[i].method;

		i++;
	}

	/* return 0 even though clsconf_get_param() returns fail
	 * that means it can NOT found this method.
	 */
	return 0;
}

int clsapi_wifi_get_supported_wps_config_methods(enum clsapi_wifi_wps_config_method *supported_wps_config_method)
{
	/* only supported PUSHBOTTON and PIN(label/display/keypad). */
	*supported_wps_config_method = CLSAPI_WIFI_WPS_CFG_METHOD_LABEL
                                    | CLSAPI_WIFI_WPS_CFG_METHOD_DISPLAY
                                    | CLSAPI_WIFI_WPS_CFG_METHOD_KEYPAD
                                    | CLSAPI_WIFI_WPS_CFG_METHOD_PUSHBUTTON;

	return CLSAPI_OK;
}

int clsapi_wifi_get_wps_status(const char *ifname, struct clsapi_wifi_wps_status *wps_status)
{
	int ret = CLSAPI_OK;

	if (!wps_status || !ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname)) {
		DBG_ERROR("Invalid parameter--ifname is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = hostapd_get_wps_status(ifname, wps_status);

	return ret;
}

int clsapi_wifi_get_current_channel_utilization(const char *phyname, uint8_t *channel_utilization)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};
	struct clsapi_channel_survey channel_survey;

	if (!channel_utilization || !phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = hostapd_get_channel_survey(primary_ifname, &channel_survey);
	if (ret < 0)
		return ret;

	*channel_utilization = channel_survey.channel_utilization;

	return ret;
}

#define CLS_WIFI_UUID_LEN (32+4)

int wps_check_macaddr(const char *mac)
{
	if (mac == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strcmp(mac, "any") == 0)
		return CLSAPI_OK;

	//check macaddr
	// Ensure correct length
	if (strlen(mac) != 17)
		return 0;

	// Check format XX:XX:XX:XX:XX:XX or XX-XX-XX-XX-XX-XX
	for (int i = 0; i < 17; i++) {
		if (i % 3 == 2) { // Every third character should be ':' or '-'
			if (mac[i] != ':')
				return -CLSAPI_ERR_INVALID_PARAM;
		} else { // Other positions should be valid hex digits (0-9, A-F, a-f)
			if (!isxdigit(mac[i]))
				return -CLSAPI_ERR_INVALID_PARAM;
		}
	}

	return CLSAPI_OK;
}

int wps_check_uuid(const char *uuid)
{
	if (uuid == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strcmp(uuid, "any") == 0)
		return CLSAPI_OK;

	if (strlen(uuid) != CLS_WIFI_UUID_LEN)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!((uuid[8] == '-' && uuid[13] == '-' && uuid[18] == '-' && uuid[23] == '-') ||
				(uuid[8] == ' ' && uuid[13] == ' ' && uuid[18] == ' ' && uuid[23] == ' '))) {
		DBG_ERROR("uuid = %s, return invalid param\n", uuid);
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	for (int i = 0; i < CLS_WIFI_UUID_LEN; i++) {
		if ((i == 8 || i == 13 || i == 18 || i == 23))
			continue;

		if (!isxdigit(uuid[i]))
			return -CLSAPI_ERR_INVALID_PARAM;
	}

	return CLSAPI_OK;
}

int wps_covert_uuid_format(char *uuid)
{
	if (uuid == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strcmp(uuid, "any") == 0)
		return CLSAPI_OK;

	if (strlen(uuid) != CLS_WIFI_UUID_LEN)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (uuid[8] == ' ' || uuid[13] == ' ' || uuid[18] == ' ' || uuid[23] == ' ') {
		uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-';
		return CLSAPI_OK;
	}

	return CLSAPI_OK;
}

int wps_covert_pin_format(char *pin)
{
	if (pin == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strlen(pin) < WPS_AP_PIN_MIN_LEN)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strlen(pin) > WPS_AP_PIN_MAX_LEN) {
		if (pin[4] == ' ' || pin[4] == '-') {
			memmove(&pin[4], &pin[5], strlen(pin) - 4);
			return CLSAPI_OK;
		}
	}

	return CLSAPI_OK;
}

int clsapi_wifi_set_wps_uuid(const char *ifname, const char *uuid)
{
	int ret = CLSAPI_OK;
	char section[32] = {0};

	if (uuid == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (wps_check_uuid(uuid))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = wps_covert_uuid_format((char *)uuid);
	if (ret < 0)
		return ret;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, CLSAPI_WPS_UUID, uuid);

	return ret;
}

int clsapi_wifi_get_wps_uuid(const char *ifname, string_1024 uuid)
{
	char section[32] = {0};
	int ret = CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, CLSAPI_WPS_UUID, uuid);
	if (ret < 0)
		return ret;

	if (strlen(uuid) == 0)
		return -CLSAPI_ERR_PARSE_ERROR;

	return ret;
}

int clsapi_wifi_set_wps_static_pin(const char *ifname, const char *ap_pin)
{
	int ret = CLSAPI_OK;
	char section[32] = {0};

	if (ap_pin == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (wps_covert_pin_format((char *)ap_pin))
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strlen(ap_pin) > WPS_AP_PIN_MAX_LEN
		|| strlen(ap_pin) < WPS_AP_PIN_MIN_LEN) {
		DBG_ERROR("Invalid parameter--ap_pin is not valid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (clsapi_wifi_check_wps_pin(atoi(ap_pin)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, CLSAPI_WPS_PIN, ap_pin);

	return ret;
}

int clsapi_wifi_get_wps_static_pin(const char *ifname, string_1024 ap_pin)
{
	char section[32] = {0};
	int ret = CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, CLSAPI_WPS_PIN, ap_pin);
	if (ret < 0)
		return ret;

	if (strlen(ap_pin) > WPS_AP_PIN_MAX_LEN
			|| strlen(ap_pin) < WPS_AP_PIN_MIN_LEN) {
		DBG_ERROR("Invalid parameter--ap_pin is over the range.\n");
		return -CLSAPI_ERR_PARSE_ERROR;
	}

	return ret;
}

int clsapi_wifi_check_wps_pin(const uint64_t pin)
{
	uint64_t accum = 0;

	accum += 3 * ((pin / 10000000) % 10);
	accum += 1 * ((pin / 1000000) % 10);
	accum += 3 * ((pin / 100000) % 10);
	accum += 1 * ((pin / 10000) % 10);
	accum += 3 * ((pin / 1000) % 10);
	accum += 1 * ((pin / 100) % 10);
	accum += 3 * ((pin / 10) % 10);
	accum += 1 * ((pin / 1) % 10);

	if (0 == (accum % 10))
		return CLSAPI_OK; /* Checksum is OK */
	else {
		DBG_ERROR("Invalid parameter--pin is not correct because of accum.\n");
		return -CLSAPI_ERR_INVALID_PARAM; /* Checksum Failed */
	}
}

int clsapi_wifi_cancel_wps(const char *ifname)
{
	int ret = CLSAPI_OK;

	ret = hostapd_cancel_wps(ifname);
	return ret;
}

int clsapi_wifi_trigger_wps_pbc_connection(const char *ifname)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_wps_config_method wps_config_method = 0;
	enum clsapi_wifi_iftype wifi_iftype = CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	ret = clsapi_wifi_get_iftype(ifname, &wifi_iftype);
	if (ret < 0)
		return ret;

	if (wifi_iftype == CLSAPI_WIFI_IFTYPE_AP) {
		ret = clsapi_wifi_get_wps_config_method(ifname, &wps_config_method);
		if (ret)
			return ret;

		if (wps_config_method & CLSAPI_WIFI_WPS_CFG_METHOD_PUSHBUTTON) {
			ret = hostapd_start_ap_wps_pbc(ifname);
		} else {
			DBG_ERROR("trigger wps_pbc failed, wps_config_method = %d\n", wps_config_method);
			return -CLSAPI_ERR_INVALID_PARAM;
		}
	} else if (wifi_iftype == CLSAPI_WIFI_IFTYPE_STA)
		ret = wpad_start_sta_wps_pbc(ifname);
	else
		ret = -CLSAPI_ERR_NOT_SUPPORTED;

	return ret;
}

int clsapi_wifi_trigger_wps_pin_connection(const char *ifname, const char *uuid, const char *pin)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_wps_config_method wps_config_method = 0;
	enum clsapi_wifi_iftype wifi_iftype = CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	if (ifname == NULL || uuid == NULL || pin == NULL)
		return -CLSAPI_ERR_NULL_POINTER;

	if (strlen(uuid) == 0 || strlen(pin) == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_get_iftype(ifname, &wifi_iftype);
	if (ret < 0)
		return ret;

	if (wifi_iftype == CLSAPI_WIFI_IFTYPE_AP) {
		ret = clsapi_wifi_get_wps_config_method(ifname, &wps_config_method);
		if (ret)
			return ret;
		if (!((wps_config_method & CLSAPI_WIFI_WPS_CFG_METHOD_LABEL) ||
					(wps_config_method & CLSAPI_WIFI_WPS_CFG_METHOD_DISPLAY) ||
					(wps_config_method & CLSAPI_WIFI_WPS_CFG_METHOD_KEYPAD))) {
			DBG_ERROR("trigger wps_pin failed, wps_config_method = %d\n", wps_config_method);
			return -CLSAPI_ERR_INVALID_PARAM;
		}

	}

	if (wps_covert_pin_format((char *)pin))
		return -CLSAPI_ERR_INVALID_PARAM;

	if (clsapi_wifi_check_wps_pin(atoi(pin)))
		return -CLSAPI_ERR_INVALID_PARAM;

	if (wifi_iftype == CLSAPI_WIFI_IFTYPE_STA) {
		if (wps_check_macaddr(uuid))
			return -CLSAPI_ERR_INVALID_PARAM;

		ret = wpad_start_sta_wps_pin(ifname, uuid, pin);
	} else if (wifi_iftype == CLSAPI_WIFI_IFTYPE_AP) {
		if (wps_check_uuid(uuid))
			return -CLSAPI_ERR_INVALID_PARAM;

		if (wps_covert_uuid_format((char *)uuid))
			return -CLSAPI_ERR_INVALID_PARAM;

		ret = hostapd_start_ap_wps_pin(ifname, uuid, pin);
	} else
		ret = -CLSAPI_ERR_NOT_SUPPORTED;

	return ret;
}

uint64_t generate_random(uint8_t num_of_digit)
{
    uint64_t random = 0;

    for (int i = 0; i < num_of_digit; i++) {
        int digit = rand() % 10;
        random = random * 10 + digit;
    }

    return random;
}

uint64_t get_wps_pin_checksum(uint64_t pin)
{
    uint64_t accum = 0;

    while (pin) {
        accum += 3 * (pin % 10);
        pin /= 10;
        accum += pin % 10;
        pin /= 10;
    }

    return (10 - accum % 10) % 10;
}

/**
 * wps_generate_pin - Generate a random WPS PIN
 * Eight digit PIN (i.e., including the checksum digit)
 */
int clsapi_wifi_generate_wps_pin(uint64_t *pin)
{
	uint64_t val;

	if (pin == NULL)
		return -CLSAPI_ERR_NULL_POINTER;

	/* Init seed */
	srand(time(NULL));

	/* Generate 7 random digits for the PIN */
	val = generate_random(WPS_AP_PIN_MAX_LEN - 1);
	val %= 10000000;

	/* Append checksum digit */
	*pin = val * 10 + get_wps_pin_checksum(val);

	return 0;
}

int clsapi_wifi_set_wps_dynamic_pin(const char *ifname, const char *pin)
{
	int cmd_len = 0;
	int ret = CLSAPI_OK;
	char cmd[256], status_reply[16];
	size_t status_reply_len = sizeof(status_reply);

	if (ifname == NULL || pin == NULL)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	if (clsapi_wifi_check_wps_pin(atoi(pin)))
		return -CLSAPI_ERR_PIN_CHECKSUM;

	cmd_len = snprintf(cmd, sizeof(cmd), "%s set %s",
			CLS_HOSTAPD_CMD_WPS_AP_PIN, pin) + 1;

	ret = hostapd_ctrl_cmd_get(ifname, cmd, cmd_len, status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0)
		return -CLSAPI_ERR_NOT_FOUND;

	return ret;
}

int clsapi_wifi_get_wps_dynamic_pin(const char *ifname, string_8 pin)
{
	int cmd_len = 0;
	int ret = CLSAPI_OK;
	char cmd[256], status_reply[16];
	size_t status_reply_len = sizeof(status_reply);

	if (ifname == NULL || pin == NULL)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	cmd_len = snprintf(cmd, sizeof(cmd), "%s get %s",
			CLS_HOSTAPD_CMD_WPS_AP_PIN, pin) + 1;

	ret = hostapd_ctrl_cmd_get(ifname, cmd, cmd_len, status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0)
		return -CLSAPI_ERR_NOT_FOUND;

	memset(pin, 0x00, sizeof(string_8));
	cls_strncpy(pin, status_reply, sizeof(string_8));
	return ret;
}

int clsapi_wifi_disable_wps_dynamic_pin(const char *ifname)
{
	int ret = CLSAPI_OK;

	if (ifname == NULL)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	ret = hostapd_disable_wifi_runtime_wps_ap_pin(ifname);
	return ret;
}

int clsapi_wifi_set_wps_random_dynamic_pin(const char *ifname, string_8 random_pin)
{
	int ret = CLSAPI_OK;

	if (ifname == NULL || random_pin == NULL)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	memset(random_pin, 0x00, sizeof(string_8));
	ret = hostapd_set_wifi_random_runtime_wps_ap_pin(ifname, random_pin);
	return ret;
}

#define CLSAPI_WPS_STATE                "wps_state"
#define CLSAPI_WPS_STATE_DISABLED       "disabled"
#define CLSAPI_WPS_STATE_NOT_CONFIGURED "not configured"
#define CLSAPI_WPS_STATE_CONFIGURED     "configured"

int clsapi_wifi_set_wps_state(const char *ifname, enum clsapi_wifi_wps_state wps_state)
{
	string_32 section;
	int ret = CLSAPI_OK;

	ret = clsconf_ifname_to_bss_section(ifname, section, sizeof(section));
	if (ret < 0)
		return ret;

	if (wps_state < CLSAPI_WIFI_WPS_STATE_DISABLED
			|| wps_state > CLSAPI_WIFI_WPS_STATE_CONFIGURED) {
		DBG_ERROR("Invalid parameter--wps_state is not correct.\n");
		return -CLSAPI_ERR_INVALID_IFNAME;
	}

	clsconf_defer_apply_int_param(CLSCONF_CFG_WIRELESS, section, CLSAPI_WPS_STATE, wps_state);

	return ret;
}

int clsapi_wifi_get_wps_state(const char *ifname, enum clsapi_wifi_wps_state *wps_state)
{
	string_64 section;
	int ret = CLSAPI_OK;

	if (!wps_state)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	clsconf_ifname_to_bss_section(ifname, section, sizeof(section));

	clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, CLSAPI_WPS_STATE, *wps_state);

	return ret;
}

int clsapi_wifi_get_current_wps_state(const char *ifname, enum clsapi_wifi_wps_state *wps_state)
{
	char status_reply[1024];
	int ret = CLSAPI_OK;
	string_32 wps_state_pos;
	const char *wps_state_id = "wps_state";
	size_t status_reply_len = sizeof(status_reply);

	if (!wps_state)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	ret = hostapd_ctrl_cmd_get(ifname, "GET_CONFIG", strlen("GET_CONFIG"),
								status_reply, &status_reply_len);
	if (ret < 0 || status_reply_len <= 0)
		return -CLSAPI_ERR_NOT_FOUND;

	ret = get_str_value_by_key(status_reply, wps_state_id, wps_state_pos, 32);
	if (ret < 0)
		return -CLSAPI_ERR_NOT_FOUND;

	if (strncasecmp(wps_state_pos, CLSAPI_WPS_STATE_DISABLED,
				strlen(CLSAPI_WPS_STATE_DISABLED)) == 0)
		*wps_state = CLSAPI_WIFI_WPS_STATE_DISABLED;
	else if (strncasecmp(wps_state_pos, CLSAPI_WPS_STATE_NOT_CONFIGURED,
				strlen(CLSAPI_WPS_STATE_NOT_CONFIGURED)) == 0)
		*wps_state = CLSAPI_WIFI_WPS_STATE_NOT_CONFIGURED;
	else if (strncasecmp(wps_state_pos, CLSAPI_WPS_STATE_CONFIGURED,
				strlen(CLSAPI_WPS_STATE_CONFIGURED)) == 0)
		*wps_state = CLSAPI_WIFI_WPS_STATE_CONFIGURED;
	else {
		DBG_ERROR("Invalid data--wps_state_pos is not correct.\n");
		ret = -CLSAPI_ERR_PARSE_ERROR;
	}

	return ret;
}

/******************************************************************************/
/**************************	Wi-Fi Features APIs	*******************************/
/******************************************************************************/

/**************************	Anti Mgmt Attack APIs	***************************/
int clsapi_wifi_enable_anti_mgmt_attack(const char *ifname, const bool enable)
{
	uint32_t u32_onoff = enable;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_ANTI_ATTACK_EN,
		.value = (void *)&u32_onoff, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	return nl80211_set_cls_attr_value(ifname, CLS_NL80211_CMD_SET_ANTI_AUTH_ASSOC_ATTACK_EN, &put_attrs);
}

int clsapi_wifi_set_anti_mgmt_attack_interval(const char *ifname, const uint32_t interval)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_ANTI_ATTACK_INTERVAL,
		.value = (void *)&interval, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	return nl80211_set_cls_attr_value(ifname, CLS_NL80211_CMD_SET_ATTACK_TO_TXDATA_INTERVAL, &put_attrs);
}


/******************************************	CSI APIs	***************************************/

int clsapi_wifi_enable_csi(const char *phyname, const bool enable)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_CSI_ENABLE,
		.value = (void *)&enable, .val_len = sizeof(uint8_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_CSI, &put_attrs);
}

int clsapi_wifi_get_csi_enabled(const char *phyname, bool *enable)
{
	uint32_t attr_id = CLS_NL80211_ATTR_CSI_ENABLE;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_ATTR_ID,
		.value = (void *)&attr_id, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!enable)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(primary_ifname, CLS_NL80211_CMD_GET_CSI, &put_attrs,
				attr_id, enable, sizeof(uint8_t));
}

int clsapi_wifi_enable_csi_non_assoc_sta(const char *phyname, const bool enable)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA,
		.value = (void *)&enable, .val_len = sizeof(uint8_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_CSI, &put_attrs);
}

int clsapi_wifi_get_csi_non_assoc_sta_enabled(const char *phyname, bool *enable)
{
	uint32_t attr_id = CLS_NL80211_ATTR_CSI_ENABLE_NON_ASSOC_STA;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_ATTR_ID,
		.value = (void *)&attr_id, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!enable)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(primary_ifname, CLS_NL80211_CMD_GET_CSI, &put_attrs,
				attr_id, enable, sizeof(uint8_t));
}

int clsapi_wifi_enable_csi_he_smooth(const char *phyname, const bool enable)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH,
		.value = (void *)&enable, .val_len = sizeof(uint8_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_CSI, &put_attrs);
}

int clsapi_wifi_get_csi_he_smooth_enabled(const char *phyname, bool *enable)
{
	uint32_t attr_id = CLS_NL80211_ATTR_CSI_ENABLE_HE_SMOOTH;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_ATTR_ID,
		.value = (void *)&attr_id, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!enable)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(primary_ifname, CLS_NL80211_CMD_GET_CSI, &put_attrs,
				attr_id, enable, sizeof(uint8_t));
}

int clsapi_wifi_set_csi_report_period(const char *phyname, const uint16_t report_period)
{
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_CSI_REPORT_PERIOD,
		.value = (void *)&report_period, .val_len = sizeof(uint16_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (report_period < 10)
		return -CLSAPI_ERR_INVALID_PARAM_LEN;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_CSI, &put_attrs);
}

int clsapi_wifi_get_csi_report_period(const char *phyname, uint16_t *report_period)
{
	uint32_t attr_id = CLS_NL80211_ATTR_CSI_REPORT_PERIOD;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_ATTR_ID,
		.value = (void *)&attr_id, .val_len = sizeof(uint32_t)};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!report_period)
		return -CLSAPI_ERR_NULL_POINTER;

	return nl80211_get_cls_attr_value(primary_ifname, CLS_NL80211_CMD_GET_CSI, &put_attrs,
				attr_id, report_period, sizeof(uint16_t));
}

int clsapi_wifi_add_csi_sta(const char *phyname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR,
		.value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_ADD_CSI_STA_MAC, &put_attrs);
	return ret;
}

int clsapi_wifi_del_csi_sta(const char *phyname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR,
		.value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	if (!sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_DEL_CSI_STA_MAC, &put_attrs);
	return ret;
}

int clsapi_wifi_get_csi_sta_list(const char *phyname, uint8_t (**sta_array)[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	uint8_t sta_macs[CLS_MAX_STA_PER_VAP][ETH_ALEN];
	int list_len = ARRAY_SIZE(sta_macs);
	clsapi_ifname primary_ifname = {0};

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	ret = nl80211_get_csi_sta_list(primary_ifname, sta_macs, &list_len);
	if (ret < 0)
		return ret;

	*sta_array = (uint8_t (*)[ETH_ALEN])calloc(list_len, ETH_ALEN);
	if (*sta_array == NULL)
		return -CLSAPI_ERR_NO_MEM;

	memcpy(*sta_array, sta_macs, list_len * ETH_ALEN);

	return list_len;
}
/**********************************	Maintenance and Testing APIs	*******************************/

int clsapi_wifi_get_radio_stats(const char *phyname, struct clsapi_wifi_radio_stats *radio_stats)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};
	struct clsapi_wifi_get_mib_cfm cfm;

	if (!phyname || !radio_stats)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	memset(radio_stats, 0, sizeof(struct clsapi_wifi_radio_stats));
	ret = nl80211_get_cls_attr_value(primary_ifname, CLS_NL80211_CMD_DFX_GET_RADIO, NULL,
			CLS_NL80211_ATTR_DFX_RADIO, (void *)&cfm, sizeof(struct clsapi_wifi_get_mib_cfm));

	radio_stats->tkip_decrypt_err = cfm.mib.basic.info[0];
	radio_stats->ccmp128_decrypt_err = cfm.mib.basic.info[1];
	radio_stats->ccmp256_decrypt_err = cfm.mib.basic.info[2];
	radio_stats->ccmp128_decrypt_err = cfm.mib.basic.info[3];
	radio_stats->ccmp256_decrypt_err = cfm.mib.basic.info[4];
	radio_stats->wapi_decrypt_err = cfm.mib.basic.info[5];
	radio_stats->rx_fcs_err_packets = cfm.mib.basic.info[6];
	radio_stats->rx_phy_err_packets = cfm.mib.basic.info[7];

	return ret;
}

int local_wifi_reset_stats(enum clsapi_wifi_stats_type stat_type, const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr[2] = {
		{.id = CLS_NL80211_ATTR_DFX_RESET, .value = (void *)&stat_type, .val_len = sizeof(stat_type)},
	};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = attr};

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	if (stat_type == CLSAPI_WIFI_STATS_TYPE_PEER_STA) {
		if (!sta_mac)
			return -CLSAPI_ERR_NULL_POINTER;

		attr[1].id = CLS_NL80211_ATTR_MAC_ADDR;
		attr[1].value = (void *)sta_mac;
		attr[1].val_len = ETH_ALEN;
		put_attrs.n_attr = 2;
	}

	ret = nl80211_set_cls_attr_value(ifname, CLS_NL80211_CMD_DFX_RESET, &put_attrs);

	return ret;
}

int clsapi_wifi_reset_radio_stats(const char *phyname)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};

	if (!phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	return local_wifi_reset_stats(CLSAPI_WIFI_STATS_TYPE_RADIO, primary_ifname, NULL);
}

int clsapi_wifi_get_wpu_stats(const char *ifname, struct clsapi_wifi_wpu_stats *wpu_stats)
{
	int ret = CLSAPI_OK;

	if (!ifname || !wpu_stats)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	memset(wpu_stats, 0, sizeof(struct clsapi_wifi_wpu_stats));
	ret = nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_DFX_GET_WPU, NULL, CLS_NL80211_ATTR_DFX_WPU,
			(void *)wpu_stats, sizeof(struct clsapi_wifi_wpu_stats));

	return ret;
}

int clsapi_wifi_reset_wpu_stats(const char *ifname)
{
	if (!ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	return local_wifi_reset_stats(CLSAPI_WIFI_STATS_TYPE_WPU, ifname, NULL);
}

int clsapi_wifi_get_vap_stats(const char *ifname, struct clsapi_wifi_vap_stats *vap_stats)
{
	int ret = CLSAPI_OK;
	struct _clsm_vap_extstats dfx_stats;

	if (!ifname || !vap_stats)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	memset(vap_stats, 0, sizeof(struct clsapi_wifi_vap_stats));
	ret = nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_DFX_GET_VAP, NULL, CLS_NL80211_ATTR_DFX_VAP,
			(void *)&dfx_stats, sizeof(struct _clsm_vap_extstats));

	vap_stats->tx_unicast = dfx_stats.tx_unicast;
	vap_stats->tx_broadcast = dfx_stats.tx_broadcast;
	vap_stats->tx_multicast = dfx_stats.tx_multicast;

	vap_stats->rx_unicast = dfx_stats.rx_unicast;
	vap_stats->rx_broadcast = dfx_stats.rx_broadcast;
	vap_stats->rx_multicast = dfx_stats.rx_multicast;

	//management packets statistics
	vap_stats->tx_probersppkts = dfx_stats.mgmt_stats.tx_stats.tx_probersppkts;
	vap_stats->tx_authpkts = dfx_stats.mgmt_stats.tx_stats.tx_authpkts;
	vap_stats->tx_deauthpkts = dfx_stats.mgmt_stats.tx_stats.tx_deauthpkts;
	vap_stats->tx_assocreqpkts = dfx_stats.mgmt_stats.tx_stats.tx_assocreqpkts;
	vap_stats->tx_assocrsppkts = dfx_stats.mgmt_stats.tx_stats.tx_assocrsppkts;
	vap_stats->tx_reascreqpkts = dfx_stats.mgmt_stats.tx_stats.tx_reascreqpkts;
	vap_stats->tx_reascrsppkts = dfx_stats.mgmt_stats.tx_stats.tx_reascrsppkts;
	vap_stats->tx_disassocpkts = dfx_stats.mgmt_stats.tx_stats.tx_disassocpkts;
	vap_stats->tx_bcnpkts = dfx_stats.mgmt_stats.tx_stats.tx_bcnpkts;

	vap_stats->rx_probersppkts =	dfx_stats.mgmt_stats.rx_stats.rx_probersppkts;
	vap_stats->rx_authpkts = dfx_stats.mgmt_stats.rx_stats.rx_authpkts;
	vap_stats->rx_deauthpkts = dfx_stats.mgmt_stats.rx_stats.rx_deauthpkts;
	vap_stats->rx_assocreqpkts = dfx_stats.mgmt_stats.rx_stats.rx_assocreqpkts;
	vap_stats->rx_assocrsppkts = dfx_stats.mgmt_stats.rx_stats.rx_assocrsppkts;
	vap_stats->rx_reascreqpkts = dfx_stats.mgmt_stats.rx_stats.rx_reascreqpkts;
	vap_stats->rx_reascrsppkts = dfx_stats.mgmt_stats.rx_stats.rx_reascrsppkts;
	vap_stats->rx_disassocpkts = dfx_stats.mgmt_stats.rx_stats.rx_disassocpkts;

	return ret;
}

int clsapi_wifi_reset_vap_stats(const char *ifname)
{
	if (!ifname)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	return local_wifi_reset_stats(CLSAPI_WIFI_STATS_TYPE_VAP, ifname, NULL);
}

int clsapi_wifi_get_sta_stats(clsapi_ifname ifname, const uint8_t sta_mac[ETH_ALEN],
		struct clsapi_wifi_sta_stats *sta_stats)
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {.id = CLS_NL80211_ATTR_MAC_ADDR,
		.value = (void *)sta_mac, .val_len = ETH_ALEN};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	if (!sta_mac || !sta_stats)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	memset(sta_stats, 0, sizeof(struct clsapi_wifi_sta_stats));
	ret = nl80211_get_cls_attr_value(ifname, CLS_NL80211_CMD_DFX_GET_PEER_STA,
			&put_attrs, CLS_NL80211_ATTR_DFX_PEER_STA,
			(void *)sta_stats, sizeof(struct clsapi_wifi_sta_stats));

	return ret;
}

int clsapi_wifi_reset_sta_stats(clsapi_ifname ifname, const uint8_t sta_mac[ETH_ALEN])
{
	if (!sta_mac)
		return -CLSAPI_ERR_NULL_POINTER;

	if (validate_ifname(ifname))
		return -CLSAPI_ERR_INVALID_IFNAME;

	return local_wifi_reset_stats(CLSAPI_WIFI_STATS_TYPE_PEER_STA, ifname, sta_mac);
}

int clsapi_wifi_set_dynamic_cca_cs_threshold(const char *phyname, int8_t threshold, int8_t delta)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname;
	struct nl80211_attr_id_value attr[2] = {0};
	struct nl80211_attr_tbl put_attrs = {0};

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	if (threshold >= 0 || delta >= -threshold || delta <= threshold) {
		DBG_ERROR("Invalid parameter--delta is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	attr[0].id = CLS_NL80211_ATTR_CS_INBDPOW1PUPTHR;
	attr[0].value = (void *)&threshold;
	attr[0].val_len = sizeof(int8_t);

	attr[1].id = CLS_NL80211_ATTR_CS_CCADELTA;
	attr[1].value = (void *)&delta;
	attr[1].val_len = sizeof(int8_t);

	put_attrs.n_attr = ARRAY_SIZE(attr);
	put_attrs.attrs = attr;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_CCA_CS_THR, &put_attrs);
}

int clsapi_wifi_get_dynamic_cca_cs_threshold(const char *phyname, int8_t *threshold)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname;

	if (!threshold || !phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	return nl80211_get_cls_attr_value(primary_ifname, CLS_NL80211_CMD_GET_CCA_CS_THR,
			NULL, CLS_NL80211_ATTR_CS_INBDPOW1PUPTHR, threshold, sizeof(int8_t));

}

int clsapi_wifi_set_dynamic_cca_ed_threshold(const char *phyname, struct clsapi_cca_ed_config_req *ed_req)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname;
	struct nl80211_attr_id_value attr[4] = {0};
	struct nl80211_attr_tbl put_attrs = {0};

	if (!ed_req || !phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	if (ed_req->cca20p_risethr >= 0 || ed_req->cca20p_fallthr >= 0 ||
			ed_req->cca20s_risethr >= 0 || ed_req->cca20s_fallthr >= 0) {
		DBG_ERROR("Invalid parameter--ed_req is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	attr[0].id = CLS_NL80211_ATTR_ED_CCA20PRISETHR;
	attr[0].value = (void *)&ed_req->cca20p_risethr;
	attr[0].val_len = sizeof(uint8_t);

	attr[1].id = CLS_NL80211_ATTR_ED_CCA20PFALLTHR;
	attr[1].value = (void *)&ed_req->cca20p_fallthr;
	attr[1].val_len = sizeof(uint8_t);

	attr[2].id = CLS_NL80211_ATTR_ED_CCA20SRISETHR;
	attr[2].value = (void *)&ed_req->cca20s_risethr;
	attr[2].val_len = sizeof(uint8_t);

	attr[3].id = CLS_NL80211_ATTR_ED_CCA20SFALLTHR;
	attr[3].value = (void *)&ed_req->cca20s_fallthr;
	attr[3].val_len = sizeof(uint8_t);

	put_attrs.n_attr = ARRAY_SIZE(attr);
	put_attrs.attrs = attr;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_CCA_ED_THR, &put_attrs);
}

int clsapi_wifi_get_dynamic_cca_ed_threshold(const char *phyname, struct clsapi_cca_ed_config_req *ed_req)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname;

	if (!ed_req || !phyname)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	ret = nl80211_get_dynamic_cca_ed_thr(primary_ifname, ed_req);

	return ret;
}

int clsapi_wifi_set_txpower(const char *phyname, const int8_t txpower)
{
	int ret = CLSAPI_OK, dbm_max;
	uint8_t ch_cur;
	int8_t txpower_limit;
	clsapi_ifname primary_ifname;
	enum nl80211_tx_power_setting type = NL80211_TX_POWER_FIXED;
	struct nl80211_attr_id_value attr[2] = {};
	struct nl80211_attr_tbl put_attrs = {.n_attr = ARRAY_SIZE(attr), .attrs = attr};
	int mbm = txpower * 100;

	attr[0].id = NL80211_ATTR_WIPHY_TX_POWER_SETTING;
	attr[0].value = (void *)&type;
	attr[0].val_len = sizeof(enum nl80211_tx_power_setting);

	attr[1].id = NL80211_ATTR_WIPHY_TX_POWER_LEVEL;
	attr[1].value = (void *)&mbm;
	attr[1].val_len = sizeof(int);

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	ret = clsapi_wifi_get_channel(phyname, &ch_cur);
	if (ret < 0)
		return ret;

	ret = nl80211_get_max_txpower(primary_ifname, (int)ch_cur, &dbm_max);
	if (ret < 0)
		return -CLSAPI_ERR_NL80211;

	ret = clsapi_wifi_get_txpower_limit(phyname,  &txpower_limit);
	if (ret < 0)
		return ret;

	if (txpower > dbm_max || txpower > txpower_limit) {
		DBG_ERROR("Invalid parameter--txpower is beyond out of max_txpower.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return nl80211_set_attr_value(phyname, NL80211_CMD_SET_WIPHY, &put_attrs);
}

int clsapi_wifi_get_txpower(const char *phyname, int8_t *txpower)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname;

	if (!txpower) {
		DBG_ERROR("Invalid parameter--txpower is not valid.\n");
		return -CLSAPI_ERR_NULL_POINTER;
	}

	if (validate_phyname(phyname))
		return -CLSAPI_ERR_INVALID_PHYNAME;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	ret = nl80211_get_txpower(primary_ifname, txpower);

	return ret;
}

int clsapi_wifi_set_txpower_auto(const char *phyname)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname;
	enum nl80211_tx_power_setting type = NL80211_TX_POWER_AUTOMATIC;
	struct nl80211_attr_id_value attr = {};
	struct nl80211_attr_tbl put_attrs = {.n_attr = 1, .attrs = &attr};

	attr.id = NL80211_ATTR_WIPHY_TX_POWER_SETTING;
	attr.value = (void *)&type;
	attr.val_len = sizeof(enum nl80211_tx_power_setting);

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret)
		return ret;

	return nl80211_set_attr_value(phyname, NL80211_CMD_SET_WIPHY, &put_attrs);
}

int clsapi_wifi_set_ampdu_protection(const char *phyname, enum clsapi_wifi_ampdu_protection method)
{
	int ret = CLSAPI_OK;
	string_32 value = {0};
	string_64 ampdu_path = {0};
	FILE *fp = NULL;

	if (validate_phyname(phyname))
		return -CLSAPI_ERR_INVALID_PHYNAME;

	switch (method) {
	case CLSAPI_WIFI_AMPDU_PRO_RTS_CTS:
		strncpy(value, "2",  sizeof(value));
		break;
	case CLSAPI_WIFI_AMPDU_PRO_CTS_ONLY:
		strncpy(value, "1",  sizeof(value));
		break;
	case CLSAPI_WIFI_AMPDU_PRO_NONE:
		strncpy(value, "0",  sizeof(value));
		break;
	default:
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	snprintf(ampdu_path, sizeof(ampdu_path), "/sys/kernel/debug/ieee80211/%s/cls_wifi/ampdu_prot", phyname);

	fp = fopen(ampdu_path, "w+");
	if (!fp)
		return -CLSAPI_ERR_FILE_OPERATION;

	ret = fwrite(value, sizeof(value), 1, fp);
	ret > 0 ? ret = CLSAPI_OK : -CLSAPI_ERR_FILE_OPERATION;

	fflush(fp);
	fclose(fp);

	return ret;
}

int clsapi_wifi_set_intelligent_ant_param(const char *phyname, struct mm_smart_antenna_req *param)
{
	int ret = CLSAPI_OK;
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_SMTANT_CFG,
		.value = (void *)param,
		.val_len = sizeof(struct mm_smart_antenna_req)};

	struct nl80211_attr_tbl put_attrs = {
		.n_attr = 1,
		.attrs = &attr
	};
       clsapi_ifname primary_ifname = {0};

       ret = phyname_to_primary_ifname(phyname, primary_ifname);
       if (ret < 0)
               return ret;

	return nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_SMT_ANTENNA_CFG, &put_attrs);
}

int clsapi_wifi_set_radius_authentification(const char *ifname, struct clsapi_radius_configure *conf)
{
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (!conf)
		return -CLSAPI_ERR_NULL_POINTER;

	if (conf->server_port < 0 || conf->server_port > 65535) {
		DBG_ERROR("Invalid parameter-server port is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	clsconf_ifname_to_bss_section(ifname, section, section_len);
	clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "auth_server", conf->server_ip);
	clsconf_set_int_param(CLSCONF_CFG_WIRELESS, section, "auth_port", conf->server_port);
	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "auth_secret", conf->server_passphrase);

	return ret;
}

int clsapi_wifi_get_radius_authentification(const char *ifname, struct clsapi_radius_configure *conf)
{
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (!conf)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	switch (encryption) {
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP:
		string_1024 server_ip;
		string_1024 server_passphrase;

		clsconf_ifname_to_bss_section(ifname, section, section_len);
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "auth_server", server_ip);
		if (ret < 0)
			return ret;
		strncpy(conf->server_ip, server_ip, sizeof(conf->server_ip));
		clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "auth_port", conf->server_port);
		if (ret < 0)
			return ret;
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "auth_secret", server_passphrase);
		if (ret < 0)
			return ret;
		strncpy(conf->server_passphrase, server_passphrase, sizeof(conf->server_passphrase));
		break;
	case CLSAPI_WIFI_ENCRYPTION_OPEN_NONE:
	case CLSAPI_WIFI_ENCRYPTION_OWE:
	case CLSAPI_WIFI_ENCRYPTION_WEP_MIXED:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_PSK:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_CERT:
	case CLSAPI_WIFI_ENCRYPTION_MAX:
		DBG_ERROR("Internal error--invalid parameter: ifname is invalid  encryption not supported.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_wifi_set_radius_acct(const char *ifname, struct clsapi_radius_configure *conf)
{
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (!conf)
		return -CLSAPI_ERR_NULL_POINTER;

	if (conf->server_port < 0 || conf->server_port > 65535) {
		DBG_ERROR("Invalid parameter-server port is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;
	switch (encryption) {
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP:
		clsconf_ifname_to_bss_section(ifname, section, section_len);
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "acct_server", conf->server_ip);
		clsconf_set_int_param(CLSCONF_CFG_WIRELESS, section, "acct_port", conf->server_port);
		clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "acct_secret", conf->server_passphrase);
		break;
	case CLSAPI_WIFI_ENCRYPTION_OPEN_NONE:
	case CLSAPI_WIFI_ENCRYPTION_OWE:
	case CLSAPI_WIFI_ENCRYPTION_WEP_MIXED:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_PSK:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_CERT:
	case CLSAPI_WIFI_ENCRYPTION_MAX:
		DBG_ERROR("Internal error--invalid parameter: ifname is invalid  encryption not supported.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_wifi_get_radius_acct(const char *ifname, struct clsapi_radius_configure *conf)
{
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (!conf)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	switch (encryption) {
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP:
		string_1024 server_ip;
		string_1024 server_passphrase;

		clsconf_ifname_to_bss_section(ifname, section, section_len);
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "acct_server", server_ip);
		if (ret < 0)
			return ret;
		strncpy(conf->server_ip, server_ip, sizeof(conf->server_ip));
		clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "acct_port", conf->server_port);
		if (ret < 0)
			return ret;
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "acct_secret", server_passphrase);
		if (ret < 0)
			return ret;
		strncpy(conf->server_passphrase, server_passphrase, sizeof(conf->server_passphrase));
		break;
	case CLSAPI_WIFI_ENCRYPTION_OPEN_NONE:
	case CLSAPI_WIFI_ENCRYPTION_OWE:
	case CLSAPI_WIFI_ENCRYPTION_WEP_MIXED:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_PSK:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_CERT:
	case CLSAPI_WIFI_ENCRYPTION_MAX:
		DBG_ERROR("Internal error--invalid parameter: ifname is invalid  encryption not supported.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_wifi_set_radius_dae(const char *ifname, struct clsapi_radius_configure *conf)
{
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (!conf)
		return -CLSAPI_ERR_NULL_POINTER;

	if (conf->server_port < 0 || conf->server_port > 65535) {
		DBG_ERROR("Invalid parameter-server port is invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	switch (encryption) {
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP:
		clsconf_ifname_to_bss_section(ifname, section, section_len);
		clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "dae_client", conf->server_ip);
		clsconf_set_int_param(CLSCONF_CFG_WIRELESS, section, "dae_port", conf->server_port);
		clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "dae_secret", conf->server_passphrase);
		break;
	case CLSAPI_WIFI_ENCRYPTION_OPEN_NONE:
	case CLSAPI_WIFI_ENCRYPTION_OWE:
	case CLSAPI_WIFI_ENCRYPTION_WEP_MIXED:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_PSK:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_CERT:
	case CLSAPI_WIFI_ENCRYPTION_MAX:
		DBG_ERROR("Internal error--invalid parameter: ifname is invalid  encryption not supported.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_wifi_get_radius_dae(const char *ifname, struct clsapi_radius_configure *conf)
{
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (!conf)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = clsapi_wifi_get_encryption(ifname, &encryption);
	if (ret < 0)
		return ret;

	switch (encryption) {
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP:
		string_1024 server_ip;
		string_1024 server_passphrase;

		clsconf_ifname_to_bss_section(ifname, section, section_len);
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "dae_client", server_ip);
		if (ret < 0)
			return ret;
		strncpy(conf->server_ip, server_ip, sizeof(conf->server_ip));
		clsconf_get_int_param(CLSCONF_CFG_WIRELESS, section, "dae_port", conf->server_port);
		if (ret < 0)
			return ret;
		ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "dae_secret", server_passphrase);
		if (ret < 0)
			return ret;
		strncpy(conf->server_passphrase, server_passphrase, sizeof(conf->server_passphrase));
		break;
	case CLSAPI_WIFI_ENCRYPTION_OPEN_NONE:
	case CLSAPI_WIFI_ENCRYPTION_OWE:
	case CLSAPI_WIFI_ENCRYPTION_WEP_MIXED:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_PSK:
	case CLSAPI_WIFI_ENCRYPTION_WAPI_CERT:
	case CLSAPI_WIFI_ENCRYPTION_MAX:
		DBG_ERROR("Internal error--invalid parameter: ifname is invalid  encryption not supported.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_wifi_get_assoc_ap_by_station(const uint8_t sta_macaddr[ETH_ALEN], clsapi_ifname apname)
{
	char status_reply[1024];
	size_t status_reply_len = sizeof(status_reply);
	int ret = CLSAPI_OK;
	int cmd_len = 0;
	string_64 str_sta_macaddr = {0};
	clsapi_ifname *ifname = NULL;
	int ifname_nums = 0;
	struct dirent *dir_entry = NULL;
	int radio_num = 0, i = 0;
	string_32 *phyname = NULL;
	bool found = false;

	DIR *dp = opendir(LINUX_KERNEL_IEEE_PATH);

	if (dp == NULL) {
		DBG_ERROR("Internal error-- directory open fail.\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while ((dir_entry = readdir(dp)) != NULL)
		if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
			radio_num++;

	phyname = (string_32 *)calloc(radio_num, sizeof(string_32));
	closedir(dp);

	dp = opendir(LINUX_KERNEL_IEEE_PATH);
	if (dp == NULL) {
		DBG_ERROR("Internal error-- directory open fail.\n");
		goto out;
	}

	while ((dir_entry = readdir(dp)) != NULL) {
		if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
			strncpy(phyname[i], dir_entry->d_name, sizeof(string_32));
			i++;
		}
	}
	closedir(dp);

	cmd_len = snprintf(str_sta_macaddr, sizeof(str_sta_macaddr), "STA "MACFMT"", MACARG(sta_macaddr)) + 1;

	for (int i = 0; i < radio_num ; i++) {
		ret = clsapi_wifi_get_ifnames(phyname[i], CLSAPI_WIFI_IFTYPE_AP, &ifname);
		if (ret < 0) {
			DBG_ERROR("Internal error-- clsapi_wifi_get_ifnames failed: %d.\n", ret);
			goto out;
		}

		ifname_nums = ret;

		for (int j = 0; j < ifname_nums; j++) {
			ret = hostapd_ctrl_cmd_get(ifname[j], str_sta_macaddr, cmd_len, status_reply, &status_reply_len);

			if (strncmp(status_reply, "FAIL", strlen("FAIL")) == 0 || ret < 0)
				continue;
			found = true;
			strncpy(apname, ifname[j], sizeof(clsapi_ifname));
			break;
		}

		if (found == true)
			break;
	}

out:
	if (phyname)
		free(phyname);

	if (ifname)
		free(ifname);

	if (found == true)
		return CLSAPI_OK;
	else
		return -CLSAPI_ERR_NOT_FOUND;
}

int clsapi_wifi_set_mode_of_interface(const char *ifname, enum clsapi_wifi_interface_mode mode)
{
	string_32 string_mode;
	string_64 section;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section);

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (mode == CLSAPI_WIFI_INTERFACE_MODE_STA)
		strncpy(string_mode, "sta", sizeof(string_mode));
	else if (mode == CLSAPI_WIFI_INTERFACE_MODE_AP)
		strncpy(string_mode, "ap", sizeof(string_mode));
	else {
		DBG_ERROR("Invalid data--mode not supported! mode: %s\n", string_mode);
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	clsconf_ifname_to_bss_section(ifname, section, section_len);
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, section, "mode", string_mode);

	return ret;
}

int clsapi_wifi_enable_station_association(const char *ifname, struct clsapi_wifi_rootap_info *info)
{
	string_1024 cls_opmode;
	int ret = CLSAPI_OK;

	if ((ret = validate_ifname(ifname)) != CLSAPI_OK)
		return ret;

	if (ret < 0)
		return ret;

	if (!info)
		return -CLSAPI_ERR_NULL_POINTER;

	ret = clsconf_get_param(CLSCONF_CFG_OPMODE, CLSCONF_SEC_GLOBALS, "mode", cls_opmode);

	if (strcmp(cls_opmode, "repeater") != 0) {
		DBG_ERROR("Internal error--mode not supported! mode: %s\n", cls_opmode);
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	if (validate_encryption(info->encryption) == 0 && info->encryption)
		clsconf_set_param(CLSCONF_CFG_WIRELESS, CLSCONF_SEC_STATION_IFACE, "encryption", encryption_enum2str(info->encryption));


	if (info->encryption != CLSAPI_WIFI_ENCRYPTION_OPEN_NONE) {
		if (validate_passphrase(ifname, info->password) == 0 &&
				info->password)
			clsconf_set_param(CLSCONF_CFG_WIRELESS, CLSCONF_SEC_STATION_IFACE, "key", info->password);
	}

	if ((strcmp(info->ssid, "") != 0) && info->ssid) {
		clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, CLSCONF_SEC_STATION_IFACE, "ssid", info->ssid);
	} else {
		DBG_ERROR("Invalid parameter: No SSID inputed\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_wifi_get_uplink_info(const char *sta_ifname, struct uplink_info *info)
{
	int ret = CLSAPI_OK;
	uint8_t rootap_mac[1][ETH_ALEN];
	int list_len = ARRAY_SIZE(rootap_mac);
	uint8_t ap_macaddr[ETH_ALEN];

	if (!sta_ifname || !info)
		return -CLSAPI_ERR_NULL_POINTER;

	memset(info, 0, sizeof(struct uplink_info));

	//get AP MAC address
	ret = nl80211_get_assoc_list(sta_ifname, rootap_mac, &list_len);
	if (ret < 0)
		return ret;

	memcpy(ap_macaddr, rootap_mac, list_len * ETH_ALEN);
	memcpy(info->rootap_mac, rootap_mac, list_len * ETH_ALEN);

	return nl80211_get_assoc_info(sta_ifname, ap_macaddr, (struct sta_info *)info);
}

int clsapi_wifi_restore_wireless_configuration(void)
{
	int ret = CLSAPI_OK;

	ret = remove(WIRELESS_CONFIGURATION_PATH);
	if (ret < 0)
		return -CLSAPI_ERR_FILE_OPERATION;

	sync();

	if (system("wifi config") == -1)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (system("wifi reload") == -1)
		return -CLSAPI_ERR_INTERNAL_ERR;

	return ret;
}

int clsapi_wifi_get_mesh_bss_ifname(const char *phyname, struct mesh_bss_info *info)
{
	int ret = CLSAPI_OK;

	if (!phyname || !info)
		return -CLSAPI_ERR_NULL_POINTER;

	memset(info, 0, sizeof(*info));

	ret = clsconf_wifi_mesh_bss_ifname(phyname, info);
	if (ret < 0)
		return ret;

	return ret;
}

int clsapi_wifi_get_txpower_ratio(const char *phyname, enum CLSAPI_WIFI_TXPWR_RATIO *txpower_ratio)
{
	int ret = CLSAPI_OK;
	string_32 section = {0};
	string_1024 txpower_ratio_string = {0};
	int8_t txpower = 0, txpower_limit = 0;
	int8_t txpower_offset = 0;

	ret = phyname_to_radio_name(phyname, section);
	if (ret)
		return -CLSAPI_ERR_INVALID_PHYNAME;

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, section, "txpower_ratio", txpower_ratio_string);
	if (ret < 0)
		return ret;

	ret = clsapi_wifi_get_txpower(phyname, &txpower);
	if (ret < 0)
		return ret;

	ret = clsapi_wifi_get_txpower_limit(phyname, &txpower_limit);
	if (ret < 0)
		return ret;

	*txpower_ratio = atoi(txpower_ratio_string);
	//Get the txpower_offset of data frame
	txpower_offset = local_txpower_ratio_to_offset(*txpower_ratio);

	//Check whether the txpower_offset of management frame is equal with data frame's
	return txpower_offset == txpower - txpower_limit ? CLSAPI_OK : -CLSAPI_ERR_INVALID_DATA;
}

int clsapi_wifi_set_txpower_offset(const char *phyname, int8_t txpower_offset)
{
	int ret = CLSAPI_OK;
	clsapi_ifname primary_ifname = {0};
	int8_t current_txpower = 0;
	struct nl80211_attr_id_value attr = {
		.id = CLS_NL80211_ATTR_PWR_OFFSET,
		.value = (void *)&txpower_offset,
		.val_len = sizeof(int8_t)};

	struct nl80211_attr_tbl put_attrs = {
		.n_attr = 1,
		.attrs = &attr
	};

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	//Get maximum txpower of management frame
	ret = clsapi_wifi_get_txpower_limit(phyname, &current_txpower);
	if (ret < 0)
		return ret;

	current_txpower += txpower_offset;
	//Set txpower offset for management frame
	ret = clsapi_wifi_set_txpower(phyname, current_txpower);
	if (ret < 0)
		return ret;

	ret = phyname_to_primary_ifname(phyname, primary_ifname);
	if (ret < 0)
		return ret;

	//Set txpower offset for data frame
	ret = nl80211_set_cls_attr_value(primary_ifname, CLS_NL80211_CMD_SET_DYN_PWR_OFFSET, &put_attrs);

	return ret;
}

int clsapi_wifi_set_txpower_ratio(const char *phyname, enum CLSAPI_WIFI_TXPWR_RATIO txpower_ratio)
{
	int ret = CLSAPI_OK;
	int8_t offset = 0;
	string_32 section;

	ret = validate_phyname(phyname);
	if (ret < 0)
		return ret;

	offset = local_txpower_ratio_to_offset(txpower_ratio);
	if (offset > 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = phyname_to_radio_name(phyname, section);
	if (ret)
		return -CLSAPI_ERR_INVALID_PHYNAME;

	clsconf_set_int_param(CLSCONF_CFG_WIRELESS, section, "txpower_ratio", txpower_ratio);
	if (ret < 0)
		return ret;

	return clsapi_wifi_set_txpower_offset(phyname, offset);
}

int clsapi_wifi_set_txpower_table(const bool highpwr_mode)
{
	DIR *dir = NULL;
	struct stat st;
	string_32 section = {0};
	int ret = CLSAPI_OK, i = 0;
	struct dirent *entry = NULL;
	string_1024 tmp_full_path = {0};
	string_1024 fullpath_of_phy[CLS_RADIO_NUM] = {0};

	dir = opendir(LINUX_KERNEL_IEEE_PATH);
	if (!dir)
		return -CLSAPI_ERR_FILE_OPERATION;

	//get all the phy from /sys/class/ieee80211/
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(tmp_full_path, sizeof(string_1024), "%s/%s", LINUX_KERNEL_IEEE_PATH, entry->d_name);

		if (stat(tmp_full_path, &st) == 0 && S_ISDIR(st.st_mode))
			strncpy(fullpath_of_phy[i++], entry->d_name, sizeof(string_1024));
	}

	for (int j = 0; j < i; j++) {
		ret = phyname_to_radio_name(fullpath_of_phy[j], section);

		if (highpwr_mode)
			//foreach every phy add pwrmode
			clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "pwrmode", "high");
		else
			//foreach every phy delete pwrmode
			clsconf_set_param(CLSCONF_CFG_WIRELESS, section, "pwrmode", "");
	}
	//add globals options highpwr
	clsconf_defer_apply_param(CLSCONF_CFG_WIRELESS, "globals", "highpwr", highpwr_mode ? "1" : "0");

	return ret;
}

int clsapi_wifi_get_txpower_table(bool *highpwr_mode)
{
	int ret = CLSAPI_OK;
	string_1024 mode = {0};

	ret = clsconf_get_param(CLSCONF_CFG_WIRELESS, "globals", "highpwr", mode);
	if (ret == CLSAPI_OK) {
		if (strcmp(mode, "1") == 0)
			*highpwr_mode = true;
		else if (strcmp(mode, "0") == 0)
			*highpwr_mode = false;
		else
			ret = -CLSAPI_ERR_INVALID_DATA;
	} else if (ret == -CLSAPI_ERR_NOT_FOUND) {
		*highpwr_mode = false;
		ret = CLSAPI_OK;
	} else
		return ret;

	return ret;
}
