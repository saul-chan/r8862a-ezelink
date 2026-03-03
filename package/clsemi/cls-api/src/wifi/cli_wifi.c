/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <linux/nl80211.h>
#include "clsapi_wifi.h"
#include "wifi_common.h"
#include "clsapi_cli.h"
#include "wifi_common.h"
#include "autogened_cli_wifi.h"

static int cli_set_txpwr_table(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	bool highpwr_mode = false;

	if (strcmp(argv[0], "highpwr") == 0)
		highpwr_mode = true;
	else if (strcmp(argv[0], "default") == 0)
		highpwr_mode = false;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_set_txpower_table(highpwr_mode);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_txpwr_table(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	bool mode = false;
	int ret = CLSAPI_OK;

	ret = clsapi_wifi_get_txpower_table(&mode);

	if (ret < 0)
		return ret;

	if (mode)
		cli_print(output, "highpwr_mode\n");
	else
		cli_print(output, "default_mode\n");

	return ret;
}

static int cli_enable_station_association(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_wifi_rootap_info info;

	strncpy(info.ssid, argv[1], sizeof(info.ssid));
	info.encryption = encryption_str2enum(argv[2]);
	if (argv[3])
		strncpy(info.password, argv[3], sizeof(info.password));

	ret = clsapi_wifi_enable_station_association(argv[0], &info);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_interface_mode(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_interface_mode mode = CLSAPI_WIFI_INTERFACE_MODE_UNSPECIFIED;

	if (strcmp(argv[1], "sta"))
		mode = CLSAPI_WIFI_INTERFACE_MODE_STA;
	else if (strcmp(argv[1], "ap"))
		mode = CLSAPI_WIFI_INTERFACE_MODE_AP;
	else
		cli_print(output, "Unknown\n");

	ret = clsapi_wifi_set_mode_of_interface(argv[0], mode);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_radius_dae(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_radius_configure conf;

	ret = clsapi_wifi_get_radius_dae(argv[0], &conf);
	if (ret < 0)
		return ret;

	cli_print(output, "RADIUS dae client IP :%s\n", conf.server_ip);
	cli_print(output, "RADIUS dae Port :%d\n", conf.server_port);
	cli_print(output, "RADIUS dae passphrase :%s\n", conf.server_passphrase);

	return ret;
}

static int cli_set_radius_dae(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_radius_configure conf;

	strncpy(conf.server_ip, argv[1], sizeof(conf.server_ip));
	conf.server_port = atoi(argv[2]);
	strncpy(conf.server_passphrase, argv[3], sizeof(conf.server_passphrase));

	ret = clsapi_wifi_set_radius_dae(argv[0], &conf);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_radius_acct(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_radius_configure conf;

	ret = clsapi_wifi_get_radius_acct(argv[0], &conf);
	if (ret < 0)
		return ret;

	cli_print(output, "RADIUS acct server IP :%s\n", conf.server_ip);
	cli_print(output, "RADIUS acct server Port :%d\n", conf.server_port);
	cli_print(output, "RADIUS acct server passphrase :%s\n", conf.server_passphrase);

	return ret;
}

static int cli_set_radius_acct(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_radius_configure conf;

	strncpy(conf.server_ip, argv[1], sizeof(conf.server_ip));
	conf.server_port = atoi(argv[2]);
	strncpy(conf.server_passphrase, argv[3], sizeof(conf.server_passphrase));

	ret = clsapi_wifi_set_radius_acct(argv[0], &conf);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_radius_authentification(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_radius_configure conf;

	ret = clsapi_wifi_get_radius_authentification(argv[0], &conf);
	if (ret < 0)
		return ret;

	cli_print(output, "RADIUS server IP :%s\n", conf.server_ip);
	cli_print(output, "RADIUS server Port :%d\n", conf.server_port);
	cli_print(output, "RADIUS server passphrase :%s\n", conf.server_passphrase);

	return ret;
}

static int cli_set_radius_authentification(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_radius_configure conf;

	strncpy(conf.server_ip, argv[1], sizeof(conf.server_ip));
	conf.server_port = atoi(argv[2]);
	strncpy(conf.server_passphrase, argv[3], sizeof(conf.server_passphrase));

	ret = clsapi_wifi_set_radius_authentification(argv[0], &conf);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_intelligent_ant_param(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct mm_smart_antenna_req req;

	if (strcmp(argv[1], "0") != 0 &&strcmp(argv[1], "1") != 0 ) {
		DBG_ERROR("Internal error--enable parameter invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (strcmp(argv[2], "0") != 0 &&strcmp(argv[2], "1") != 0 ) {
		DBG_ERROR("Internal error--update mode parameter invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (strcmp(argv[3], "0") != 0 &&strcmp(argv[3], "1") != 0 ) {
		DBG_ERROR("Internal error--gpio mode parameter invalid.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	req.enable = atoi(argv[1]) << 0 | atoi(argv[2]) << 1 | atoi(argv[3]) << 2;
	req.curval_hi = atoi(argv[4]);
	req.curval_lo = atoi(argv[5]);
	req.rstval_hi = atoi(argv[6]);
	req.rstval_lo = atoi(argv[7]);

	ret = clsapi_wifi_set_intelligent_ant_param(argv[0], &req);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_apname_by_station(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t sta_mac[ETH_ALEN];
	string_64 apname;

	ret = mac_aton(argv[0], sta_mac);
	if (ret)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_get_assoc_ap_by_station(sta_mac, apname);
	if (ret < 0)
		return ret;

	cli_print(output, "AP name %s\n", apname);

	return ret;
}

static int cli_get_dynamic_cca_ed_thr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_cca_ed_config_req ed_req = {0};

	ret = clsapi_wifi_get_dynamic_cca_ed_threshold(argv[0], &ed_req);
	if (ret < 0)
		return ret;

	cli_print(output, "Dynamic CCA ED 20P risethr %d\n", ed_req.cca20p_risethr);
	cli_print(output, "Dynamic CCA ED 20P fallthr %d\n", ed_req.cca20p_fallthr);
	cli_print(output, "Dynamic CCA ED 20S risethr %d\n", ed_req.cca20s_risethr);
	cli_print(output, "Dynamic CCA ED 20S fallthr %d\n", ed_req.cca20s_fallthr);

	return ret;
}

static int cli_set_ampdu_protection(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_ampdu_protection method = __CLSAPI_WIFI_AMPDU_PRO_MAX;

	if (strcmp(argv[1], "rts_cts") == 0)
		method = CLSAPI_WIFI_AMPDU_PRO_RTS_CTS;
	else if (strcmp(argv[1], "cts_only") == 0)
		method = CLSAPI_WIFI_AMPDU_PRO_CTS_ONLY;
	else if (strcmp(argv[1], "none") == 0)
		method = CLSAPI_WIFI_AMPDU_PRO_NONE;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_set_ampdu_protection(argv[0], method);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_dynamic_cca_ed_thr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_cca_ed_config_req ed_req = {0};

	ed_req.cca20p_risethr = atoi(argv[1]);
	ed_req.cca20p_fallthr = atoi(argv[2]);
	ed_req.cca20s_risethr = atoi(argv[3]);
	ed_req.cca20s_fallthr = atoi(argv[4]);

	ret = clsapi_wifi_set_dynamic_cca_ed_threshold(argv[0], &ed_req);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_radio_stats(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_wifi_radio_stats radio_stats = {0};

	ret = clsapi_wifi_get_radio_stats(argv[0], &radio_stats);
	if (ret < 0)
		return ret;

	cli_print(output, "Statistics of radio: %s\n", argv[0]);

	cli_print(output, CLSAPI_CLI_INDENT"TKIP decrypt err:     %u\n", radio_stats.tkip_decrypt_err);
	cli_print(output, CLSAPI_CLI_INDENT"CCMP128 decrypt err:  %u\n", radio_stats.ccmp128_decrypt_err);
	cli_print(output, CLSAPI_CLI_INDENT"CCMP256 decrypt err:  %u\n", radio_stats.ccmp256_decrypt_err);
	cli_print(output, CLSAPI_CLI_INDENT"GCMP128 decrypt err:  %u\n", radio_stats.ccmp128_decrypt_err);
	cli_print(output, CLSAPI_CLI_INDENT"GCMP256 decrypt err:  %u\n", radio_stats.ccmp256_decrypt_err);
	cli_print(output, CLSAPI_CLI_INDENT"WAPI decrypt err:     %u\n", radio_stats.wapi_decrypt_err);
	cli_print(output, CLSAPI_CLI_INDENT"RX FCS err pkts:  %u\n", radio_stats.rx_fcs_err_packets);
	cli_print(output, CLSAPI_CLI_INDENT"RX phyerr pkts:   %u\n", radio_stats.rx_phy_err_packets);

	return CLSAPI_OK;
}

static int cli_get_wpu_stats(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_wifi_wpu_stats wpu_stats = {0};

	ret = clsapi_wifi_get_wpu_stats(argv[0], &wpu_stats);
	if (ret < 0)
		return ret;

	//cli_print(output, "Statistics of WPU: phy%d\n", wpu_stats.radio_name);
	cli_print(output, "CPU idle: %u%\n", wpu_stats.cpu_idle_percent);

	return CLSAPI_OK;
}

static int cli_get_vap_stats(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_wifi_vap_stats vap_stats = {0};

	ret = clsapi_wifi_get_vap_stats(argv[0], &vap_stats);
	if (ret < 0)
		return ret;

	cli_print(output, "Interface : %s\n", argv[0]);
	cli_print(output, CLSAPI_CLI_INDENT "TX Unicast: %u\n", vap_stats.tx_unicast);
	cli_print(output, CLSAPI_CLI_INDENT "TX Broadcast: %u\n", vap_stats.tx_broadcast);
	cli_print(output, CLSAPI_CLI_INDENT "TX Multicast: %u\n", vap_stats.tx_multicast);
	cli_print(output, CLSAPI_CLI_INDENT "RX Unicast: %u\n", vap_stats.rx_unicast);
	cli_print(output, CLSAPI_CLI_INDENT "RX Broadcast: %u\n", vap_stats.rx_broadcast);
	cli_print(output, CLSAPI_CLI_INDENT "RX Multicast: %u\n", vap_stats.rx_multicast);

	cli_print(output, "Management Packets statistics:\n");
	cli_print(output, CLSAPI_CLI_INDENT "TX Beacon: %u\n", vap_stats.tx_bcnpkts);

	cli_print(output, CLSAPI_CLI_INDENT "TX Probe response: %u\n", vap_stats.tx_probersppkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Authentication: %u\n", vap_stats.tx_authpkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Deauthentication: %u\n", vap_stats.tx_deauthpkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Association request: %u\n", vap_stats.tx_assocreqpkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Association response: %u\n", vap_stats.tx_assocrsppkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Reassociation request: %u\n", vap_stats.tx_reascreqpkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Reassociation response: %u\n", vap_stats.tx_reascrsppkts);
	cli_print(output, CLSAPI_CLI_INDENT "TX Disassociation : %u\n", vap_stats.tx_disassocpkts);

	cli_print(output, CLSAPI_CLI_INDENT "RX Probe response: %u\n", vap_stats.rx_probersppkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Authentication: %u\n", vap_stats.rx_authpkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Deauthentication: %u\n", vap_stats.rx_deauthpkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Association request: %u\n", vap_stats.rx_assocreqpkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Association response: %u\n", vap_stats.rx_assocrsppkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Reassociation request: %u\n", vap_stats.rx_reascreqpkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Reassociation response: %u\n", vap_stats.rx_reascrsppkts);
	cli_print(output, CLSAPI_CLI_INDENT "RX Disassociation : %u\n", vap_stats.rx_disassocpkts);

	return CLSAPI_OK;
}

static int cli_get_sta_stats(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_wifi_sta_stats sta_stats = {0};
	uint8_t sta_mac[ETH_ALEN];

	ret = mac_aton(argv[1], sta_mac);
	if (ret)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_get_sta_stats(argv[0], sta_mac, &sta_stats);
	if (ret < 0)
		return ret;

	cli_print(output, "TX Multicast packets: %d\n", sta_stats.tx_multicast);
	cli_print(output, "TX Broadcast packets: %d\n", sta_stats.tx_broadcast);
	cli_print(output, "TX Unicast packets: %d\n", sta_stats.tx_unicast);

	cli_print(output, "RX Multicast packets: %d\n", sta_stats.rx_multicast);
	cli_print(output, "RX Broadcast packets: %d\n", sta_stats.rx_broadcast);
	cli_print(output, "RX Unicast packets: %d\n", sta_stats.rx_unicast);

	return CLSAPI_OK;
}

static int cli_reset_sta_stats(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t sta_mac[ETH_ALEN];

	ret = mac_aton(argv[1], sta_mac);
	if (ret)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_reset_sta_stats(argv[0], sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static inline enum clsapi_wifi_scan_flags strflag_to_enum(const char *str_flags)
{
	if (strcmp(str_flags, "flush_mode") == 0)
		return CLSAPI_WIFI_SCAN_FLAG_FLUSH;
	else if (strcmp(str_flags, "off_channel") == 0)
		return CLSAPI_WIFI_SCAN_FLAG_OFF_CHANNEL;
	else
		return __CLSAPI_WIFI_SCAN_FLAG_MAX;
}

static inline enum clsapi_wifi_band strband_to_enum(const char *str_band)
{
	if (strcmp(str_band, "2") == 0 || strcasecmp(str_band, "2GHz") == 0 || strcasecmp(str_band, "2G") == 0)
		return CLSAPI_BAND_2GHZ;
	else if (strcmp(str_band, "5") == 0 || strcasecmp(str_band, "5GHz") == 0 || strcasecmp(str_band, "5G") == 0)
		return CLSAPI_BAND_5GHZ;
	else if (strcasecmp(str_band, "6") == 0 || strcasecmp(str_band, "6GHz") == 0 || strcasecmp(str_band, "6G") == 0)
		return CLSAPI_BAND_6GHZ;
	else if (strcmp(str_band, "0") == 0 || strcasecmp(str_band, "current") == 0 || strcasecmp(str_band, "default") == 0)
		return CLSAPI_BAND_DEFAULT;
	else
		return CLSAPI_BAND_NOSUCH_BAND;
}

static int cli_get_ifnames(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_iftype iftype = CLSAPI_WIFI_IFTYPE_AP;
	clsapi_ifname *ifnames = NULL;
	int list_num = 0;
	int ret;

	if (argc >= 2) {
		if (strcmp(argv[1], "ap") == 0)
			iftype = CLSAPI_WIFI_IFTYPE_AP;
		else if (strcmp(argv[1], "sta") == 0)
			iftype = CLSAPI_WIFI_IFTYPE_STA;
		else
			return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_get_ifnames(argv[0], iftype, &ifnames);
	if (ret < 0)
		return ret;

	list_num = ret;
	if (list_num > 0 && ifnames == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < list_num; i++)
		cli_print(output, "%s\n", ifnames[i]);

	if (ifnames)
		free(ifnames);

	return CLSAPI_OK;
}

static int cli_get_assoc_list(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	uint8_t (*sta_mac_list)[ETH_ALEN] = NULL;
	int list_num = 0;
	int ret = clsapi_wifi_get_assoc_list(argv[0], &sta_mac_list);

	if (ret < 0)
		return ret;

	list_num = ret;
	if (list_num > 0 && sta_mac_list == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < list_num; i++)
		cli_print(output, "  %03d: "MACFMT"\n", i, MACARG(sta_mac_list[i]));

	if (sta_mac_list)
		free(sta_mac_list);

	return CLSAPI_OK;
}

static int cli_deauth_sta(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	uint16_t reason_code = 2; //Previous authentication no longer valid.
	uint8_t mac[ETH_ALEN];
	int ret;

	ret = mac_aton(argv[1], mac);
	if (ret < 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (argc >= 3)
		reason_code = atoi(argv[2]);

	ret = clsapi_wifi_deauth_sta(argv[0], mac, reason_code);
	return clsapi_cli_report_complete(ret, output);
}

static int cli_disassociate_sta(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	uint16_t reason_code = 2; //Previous authentication no longer valid.
	uint8_t mac[ETH_ALEN];
	int ret;

	ret = mac_aton(argv[1], mac);
	if (ret < 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (argc >= 3)
		reason_code = atoi(argv[2]);

	ret = clsapi_wifi_disassoc_sta(argv[0], mac, reason_code);
	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_chans(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t *chan_list = NULL;
	uint8_t list_len = 0;
	enum clsapi_wifi_band band = strband_to_enum(argv[1]);
	enum clsapi_wifi_bw bw = bw_int2enum(atoi(argv[2]));

	ret = clsapi_wifi_get_supported_channels(argv[0], band, bw, &chan_list);
	if (ret < 0)
		return ret;

	list_len = ret;
	if (list_len > 0 && chan_list == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < list_len; i++)
		cli_print(output, "%d ", chan_list[i]);

	cli_print(output, "\n");

	if (chan_list)
		free(chan_list);

	return CLSAPI_OK;
}

static int cli_set_chan(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;
	enum clsapi_wifi_bw bw = CLSAPI_WIFI_BW_DEFAULT;

	if (atoi(argv[1]) == 0 && strcmp(argv[1], "auto") != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (argc > 2)
		band = strband_to_enum(argv[2]);

	if (argc > 3)
		bw = bw_int2enum(atoi(argv[3]));

	ret = clsapi_wifi_set_channel(argv[0], atoi(argv[1]), band, bw);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_hwmodes(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	char buf[32];
	enum clsapi_wifi_hwmode hwmodes = 0;
	int ret = clsapi_wifi_get_supported_hwmodes(argv[0], &hwmodes);

	if (ret < 0 || hwmodes <= 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "IEEE802.11%s%s%s%s%s%s",
			(hwmodes & CLSAPI_HWMODE_IEEE80211_A) ? "a" : "",
			(hwmodes & CLSAPI_HWMODE_IEEE80211_B) ? "b" : "",
			(hwmodes & CLSAPI_HWMODE_IEEE80211_G) ? "g" : "",
			(hwmodes & CLSAPI_HWMODE_IEEE80211_N) ? "n" : "",
			(hwmodes & CLSAPI_HWMODE_IEEE80211_AC) ? "ac" : "",
			(hwmodes & CLSAPI_HWMODE_IEEE80211_AX) ? "ax" : "");

	return clsapi_cli_report_str_value(ret, output, buf);
}

static int cli_get_hwmode(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	char buf[32];
	enum clsapi_wifi_hwmode hwmode = 0;
	int ret = clsapi_wifi_get_hwmode(argv[0], &hwmode);

	if (ret < 0 || hwmode <= 0)
		strcpy(buf, "unknown");
	else if (hwmode & CLSAPI_HWMODE_IEEE80211_A)
		snprintf(buf, sizeof(buf), "IEEE802.11%s", "a");
	else if (hwmode & CLSAPI_HWMODE_IEEE80211_B)
		snprintf(buf, sizeof(buf), "IEEE802.11%s", "b");
	else if (hwmode & CLSAPI_HWMODE_IEEE80211_G)
		snprintf(buf, sizeof(buf), "IEEE802.11%s", "g");
	else if (hwmode & CLSAPI_HWMODE_IEEE80211_N)
		snprintf(buf, sizeof(buf), "IEEE802.11%s", "n");
	else if (hwmode & CLSAPI_HWMODE_IEEE80211_AC)
		snprintf(buf, sizeof(buf), "IEEE802.11%s", "ac");
	else if (hwmode & CLSAPI_HWMODE_IEEE80211_AX)
		snprintf(buf, sizeof(buf), "IEEE802.11%s", "ax");
	else
		strcpy(buf, "unknown");

	return clsapi_cli_report_str_value(ret, output, buf);
}

static int cli_set_hwmode(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	char *str_hwmode = argv[1];
	enum clsapi_wifi_hwmode hwmode = 0;
	int ret;

	if (strcmp(str_hwmode, "a") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_A;
	else if (strcmp(str_hwmode, "b") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_B;
	else if (strcmp(str_hwmode, "g") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_G;
	else if (strcmp(str_hwmode, "n") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_N;
	else if (strcmp(str_hwmode, "ac") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_AC;
	else if (strcmp(str_hwmode, "ax") == 0)
		hwmode = CLSAPI_HWMODE_IEEE80211_AX;
	else
		return clsapi_cli_report_error(CLSAPI_ERR_INVALID_PARAM, output);

	ret = clsapi_wifi_set_hwmode(argv[0], hwmode);
	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_hidden_ssid(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_hidden_ssid hidden_ssid;
	int ret = clsapi_wifi_get_hidden_ssid(argv[0], &hidden_ssid);

	return clsapi_cli_report_uint_value(ret, output, hidden_ssid);
}

static int cli_set_hidden_ssid(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret;

	ret = clsapi_wifi_set_hidden_ssid(argv[0], atoi(argv[1]));
	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_bws(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_bw *bws = NULL;
	int bws_len = 0;
	int ret = clsapi_wifi_get_supported_bws(argv[0], &bws);

	if (ret < 0)
		return ret;

	bws_len = ret;
	if (bws_len > 0 && bws == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < bws_len; i++)
		cli_print(output, "%dMHz\n", bw_enum2int(bws[i]));

	if (bws)
		free(bws);

	return CLSAPI_OK;
}

static int cli_get_bw(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_bw bw = CLSAPI_WIFI_BW_DEFAULT;
	int ret = clsapi_wifi_get_bw(argv[0], &bw);

	if (ret)
		return clsapi_cli_report_error(ret, output);

	cli_print(output, "%dMHz\n", bw_enum2int(bw));

	return CLSAPI_OK;
}

static int cli_set_bw(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_bw bw = CLSAPI_WIFI_BW_DEFAULT;
	int ret = CLSAPI_OK;

	bw = bw_int2enum(atoi(argv[1]));
	ret = clsapi_wifi_set_bw(argv[0], bw);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_short_gi(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const enum clsapi_wifi_bw bw = bw_int2enum(atol(argv[1]));
	bool onoff = 0;

	ret = clsapi_wifi_get_short_gi(phyname, bw, &onoff);

	return clsapi_cli_report_uint_value(ret, output, onoff);
}

static int cli_set_short_gi(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const enum clsapi_wifi_bw bw = bw_int2enum(atol(argv[1]));
	const bool onoff = atol(argv[2]);

	ret = clsapi_wifi_set_short_gi(phyname, bw, onoff);

	return clsapi_cli_report_complete(ret, output);
}

const static char *str_clsapi_wifi_iftype[] = {
	"Unknown Wi-Fi interface type",		//CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE
	"Access Point",						//CLSAPI_WIFI_IFTYPE_AP
	"Station"							//CLSAPI_WIFI_IFTYPE_STA
};

static int cli_get_iftype(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_iftype iftype;
	int ret = clsapi_wifi_get_iftype(argv[0], &iftype);

	return clsapi_cli_report_str_value(ret, output, str_clsapi_wifi_iftype[iftype]);
}

static int cli_get_bssid(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	uint8_t bssid[ETH_ALEN];
	string_32 str_bssid;
	int ret = clsapi_wifi_get_bssid(argv[0], bssid);

	if (ret < 0)
		return ret;

	sprintf(str_bssid, MACFMT, MACARG(bssid));

	return clsapi_cli_report_str_value(ret, output, str_bssid);
}

static int cli_get_vbss_vap(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	struct vbss_vap_info vbss_vap = {0};

	ret = clsapi_wifi_get_vbss_vap(ifname, &vbss_vap);
	if (ret < 0)
		return ret;

	cli_print(output, "bssid: "MACFMT"\n", MACARG(vbss_vap.bssid));
	cli_print(output, "ssid: %s\n", vbss_vap.ssid);
	cli_print(output, "ifname: %s\n", vbss_vap.ifname);
	cli_print(output, "auth_type: %u\n", vbss_vap.auth_type);
	cli_print(output, "pwd: %s\n", vbss_vap.pwd);
	cli_print(output, "role: %u\n", vbss_vap.role);

	return ret;
}


static int cli_add_vbss_vap(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct vbss_vap_info vbss_vap = {0};

	if (!strcmp(argv[1], "roam"))
		vbss_vap.role = VBSS_VAP_ROAMING;
	cls_strncpy(vbss_vap.ifname, argv[0], sizeof(vbss_vap.ifname));
	ret = mac_aton(argv[2], vbss_vap.bssid);
	if (ret)
		return -CLSAPI_ERR_INVALID_PARAM;
	cls_strncpy(vbss_vap.ssid, argv[3], sizeof(vbss_vap.ssid));

	if (!strcmp(argv[4], "wpa2"))
		vbss_vap.auth_type = WPA_AUTHORIZE_TYPE_WPA2;
	if (argc > 5 && vbss_vap.auth_type != WPA_AUTHORIZE_TYPE_NONE)
		cls_strncpy(vbss_vap.pwd, argv[5], sizeof(vbss_vap.pwd));

	ret = clsapi_wifi_add_vbss_vap(&vbss_vap, vbss_vap.ifname, sizeof(vbss_vap.ifname));
	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_uplink_info(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char *sta_ifname = argv[0];
	struct uplink_info sta = {0};

	ret = clsapi_wifi_get_uplink_info(sta_ifname, &sta);
	if (ret < 0)
		return ret;

	cli_print(output, "rootap MAC address: "MACFMT"\n", MACARG(sta.rootap_mac));
	cli_print(output, "signal: %d\n", sta.signal);
	cli_print(output, "tx_packets: %lu\n", sta.tx_packets);
	cli_print(output, "rx_packets: %lu\n", sta.rx_packets);
	cli_print(output, "tx_bytes: %lu\n", sta.tx_bytes);
	cli_print(output, "rx_bytes: %lu\n", sta.rx_bytes);
	cli_print(output, "tx_airtime: %lu\n", sta.tx_airtime);
	cli_print(output, "rx_airtime: %lu\n", sta.rx_airtime);
	cli_print(output, "bytes_64bit: %d\n", sta.bytes_64bit);
	cli_print(output, "tx_airtime: %lu\n", sta.tx_airtime);
	cli_print(output, "rx_mcs: %u\n", sta.rx_mcs);
	cli_print(output, "tx_mcs: %u\n", sta.tx_mcs);
	cli_print(output, "rx_vhtmcs: %u\n", sta.rx_vhtmcs);
	cli_print(output, "tx_vhtmcs: %u\n", sta.tx_vhtmcs);
	cli_print(output, "rx_hemcs: %u\n", sta.rx_he_mcs);
	cli_print(output, "tx_hemcs: %u\n", sta.tx_he_mcs);
	cli_print(output, "rx_vht_nss: %u\n", sta.rx_vht_nss);
	cli_print(output, "tx_vht_nss: %u\n", sta.tx_vht_nss);
	cli_print(output, "rx_he_nss: %u\n", sta.rx_he_nss);
	cli_print(output, "tx_he_nss: %u\n", sta.tx_he_nss);
	cli_print(output, "bandwidth: %d\n", bw_enum2int(sta.bandwidth));
	cli_print(output, "current_tx_rate: %lu\n", sta.current_tx_rate);
	cli_print(output, "current_rx_rate: %lu\n", sta.current_rx_rate);
	cli_print(output, "inactive_msec: %lu\n", sta.inactive_msec);
	cli_print(output, "connected_sec: %lu\n", sta.connected_sec);
	cli_print(output, "flags: 0x%lx\n", sta.flags);
	cli_print(output, "num_ps_buf_frames: %lu\n", sta.num_ps_buf_frames);
	cli_print(output, "tx_retry_failed: %lu\n", sta.tx_retry_failed);
	cli_print(output, "tx_retry_count: %lu\n", sta.tx_retry_count);
	cli_print(output, "last_ack_rssi: %d\n", sta.last_ack_rssi);
	cli_print(output, "backlog_packets: %lu\n", sta.backlog_packets);
	cli_print(output, "backlog_bytes: %lu\n", sta.backlog_bytes);

	return ret;
}

static int cli_get_mesh_bss_ifname(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = argv[0];
	struct mesh_bss_info info;

	ret = clsapi_wifi_get_mesh_bss_ifname(phyname, &info);
	if (ret < 0)
		return ret;

	cli_print(output, "Fronthaul: %s\n", info.fh_ifname);
	cli_print(output, "Backhaul AP: %s\n", info.bh_ifname);
	cli_print(output, "Backhaul STA: %s\n", info.bh_sta_ifname);

	return ret;
}

static int cli_get_sta_info(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char *ifname = argv[0];
	uint8_t sta_mac[ETH_ALEN];
	struct sta_info sta = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_get_sta_info(ifname, sta_mac, &sta);
	if (ret < 0)
		return ret;

	cli_print(output, "mac: "MACFMT"\n", MACARG(sta.mac));
	cli_print(output, "signal: %d\n", sta.signal);
	cli_print(output, "tx_packets: %lu\n", sta.tx_packets);
	cli_print(output, "rx_packets: %lu\n", sta.rx_packets);
	cli_print(output, "tx_bytes: %lu\n", sta.tx_bytes);
	cli_print(output, "rx_bytes: %lu\n", sta.rx_bytes);
	cli_print(output, "tx_airtime: %lu\n", sta.tx_airtime);
	cli_print(output, "rx_airtime: %lu\n", sta.rx_airtime);
	cli_print(output, "bytes_64bit: %d\n", sta.bytes_64bit);
	cli_print(output, "tx_airtime: %lu\n", sta.tx_airtime);
	cli_print(output, "current_tx_rate: %lu\n", sta.current_tx_rate);
	cli_print(output, "current_rx_rate: %lu\n", sta.current_rx_rate);
	cli_print(output, "inactive_msec: %lu\n", sta.inactive_msec);
	cli_print(output, "connected_sec: %lu\n", sta.connected_sec);
	cli_print(output, "flags: 0x%lx\n", sta.flags);
	cli_print(output, "num_ps_buf_frames: %lu\n", sta.num_ps_buf_frames);
	cli_print(output, "tx_retry_failed: %lu\n", sta.tx_retry_failed);
	cli_print(output, "tx_retry_count: %lu\n", sta.tx_retry_count);
	cli_print(output, "last_ack_rssi: %d\n", sta.last_ack_rssi);
	cli_print(output, "backlog_packets: %lu\n", sta.backlog_packets);
	cli_print(output, "backlog_bytes: %lu\n", sta.backlog_bytes);
	cli_print(output, "rx_mcs: %u\n", sta.rx_mcs);
	cli_print(output, "tx_mcs: %u\n", sta.tx_mcs);
	cli_print(output, "rx_vhtmcs: %u\n", sta.rx_vhtmcs);
	cli_print(output, "tx_vhtmcs: %u\n", sta.tx_vhtmcs);
	cli_print(output, "rx_hemcs: %u\n", sta.rx_he_mcs);
	cli_print(output, "tx_hemcs: %u\n", sta.tx_he_mcs);
	cli_print(output, "rx_vht_nss: %u\n", sta.rx_vht_nss);
	cli_print(output, "tx_vht_nss: %u\n", sta.tx_vht_nss);
	cli_print(output, "rx_he_nss: %u\n", sta.rx_he_nss);
	cli_print(output, "tx_he_nss: %u\n", sta.tx_he_nss);
	cli_print(output, "bandwidth: %d\n", bw_enum2int(sta.bandwidth));

	return ret;
}


static int cli_get_survey(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char *ifname = argv[0];
	struct survey_entry *survey = NULL;
	int survey_entry_len = 0;

	ret = clsapi_wifi_get_survey(ifname, &survey);

	if (ret < 0)
		return ret;
	survey_entry_len = ret;

	if (survey_entry_len > 0 && survey == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < survey_entry_len; i++) {
		cli_print(output, "%02d ", i);
		cli_print(output, "active_time: %lu ", survey[i].active_time);
		cli_print(output, "busy_time: %lu ", survey[i].busy_time);
		cli_print(output, "busy_time_ext: %lu ", survey[i].busy_time_ext);
		cli_print(output, "rxtime: %lu ", survey[i].rxtime);
		cli_print(output, "txtime: %lu ", survey[i].txtime);
		cli_print(output, "mhz: %u ", survey[i].mhz);
		cli_print(output, "noise: %d\n", survey[i].noise);
	}

	if (survey)
		free(survey);

	return CLSAPI_OK;
}


static int cli_get_vbss_ap_stats(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry,
	int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	struct vbss_ap_stats stats = {0};

	ret = clsapi_wifi_get_vbss_ap_stats(ifname, &stats);
	if (ret < 0)
		return ret;

	cli_print(output, "tx_mpdu_num: %u\n", stats.tx_mpdu_num);
	cli_print(output, "tx_bytes: %u\n", stats.tx_bytes);
	cli_print(output, "rx_mpdu_num: %u\n", stats.rx_mpdu_num);
	cli_print(output, "rx_bytes: %u\n", stats.rx_bytes);

	return ret;
}


static int cli_get_chan_score(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry,
	int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;
	struct chan_score *score = NULL;
	int score_len = 0;

	if (argc > 1)
		band = strband_to_enum(argv[1]);

	ret = clsapi_wifi_get_chan_score(argv[0], band, &score);

	if (ret < 0)
		return ret;

	score_len = ret;
	if (score_len > 0 && score == NULL)
		return -CLSAPI_ERR_NO_MEM;

	cli_print(output, "note: max=65535, the less the better\noff-channel scan is necessary before getting survey\n");
	for (int i = 0; i < score_len; i++) {
		cli_print(output, "%02d ", i);
		cli_print(output, "chan: %u ", score[i].chan);
		cli_print(output, "score: %u\n", score[i].score);
	}

	if (score)
		free(score);

	return CLSAPI_OK;
}

static inline void get_str_band(const enum clsapi_wifi_band band, char str_band[16])
{
	if (band == CLSAPI_BAND_2GHZ)
		strcpy(str_band, "2GHz");
	else if (band == CLSAPI_BAND_5GHZ)
		strcpy(str_band, "5GHz");
	else if (band == CLSAPI_BAND_6GHZ)
		strcpy(str_band, "6GHz");
	else
		strcpy(str_band, "unknown");
}

static int cli_get_bands(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = argv[0];
	enum clsapi_wifi_band *bands = NULL;
	int bands_len = 0;
	string_16 str_band;

	ret = clsapi_wifi_get_supported_bands(phyname, &bands);
	if (ret < 0)
		return ret;

	bands_len = ret;
	if (bands_len > 0 && bands == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < bands_len; i++) {
		get_str_band(bands[i], str_band);
		cli_print(output, "%s ", str_band);
	}

	cli_print(output, "\n");

	if (bands)
		free(bands);

	return CLSAPI_OK;
}

static int cli_get_band(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	char str_band[16];
	enum clsapi_wifi_band band = CLSAPI_BAND_NOSUCH_BAND;

	ret = clsapi_wifi_get_band(ifname, &band);
	if (ret < 0)
		return ret;

	get_str_band(band, str_band);
	return clsapi_cli_report_str_value(ret, output, str_band);
}

static int cli_get_encryption(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_encryption encryption = CLSAPI_WIFI_ENCRYPTION_MAX;

	ret = clsapi_wifi_get_encryption(argv[0], &encryption);
	if (ret < 0)
		return clsapi_cli_report_error(ret, output);

	cli_print(output, "%s\n", encryption_enum2str(encryption));

	return CLSAPI_OK;
}

static int cli_set_encryption(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;

	ret = clsapi_wifi_set_encryption(argv[0], encryption_str2enum(argv[1]));

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_wep_key(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK, idx_in_use;
	char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64];

	ret = clsapi_wifi_get_wep_key(argv[0], &idx_in_use, wep_key);
	if (ret < 0)
		return clsapi_cli_report_error(ret, output);

	for (int i = 0; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++) {
		cli_print(output, "%s key%d: %s\n", (i == idx_in_use) ? " (*)" : "    ", i + 1, wep_key[i]);
	}

	return CLSAPI_OK;
}

static int cli_set_wep_key(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64];

	for (int i = 0; i < CLSAPI_WIFI_MAX_PRESHARED_KEY; i++) {
		if (i < argc - 2)
			strcpy(wep_key[i], argv[i + 2]);
		else
			strcpy(wep_key[i], "");
	}

	ret = clsapi_wifi_set_wep_key(argv[0], atoi(argv[1]), wep_key);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_bss(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint8_t bssid[ETH_ALEN] = {0};
	char created_ifname[33] = {0};

	if (argc > 1) {
		ret = mac_aton(argv[1], bssid);
		if (ret)
			return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_add_bss(phyname, bssid, created_ifname);

	return clsapi_cli_report_str_value(ret, output, created_ifname);
}

static int cli_get_pmf_enabled(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	enum clsapi_wifi_pmf_enable pmf_enabled;
	int ret = clsapi_wifi_get_pmf_enabled(argv[0], &pmf_enabled);

	if (ret < 0)
		return ret;

	if (pmf_enabled == CLSAPI_WIFI_PMF_DISABLED)
		cli_print(output, "%d: Disabled\n", pmf_enabled);
	else if (pmf_enabled == CLSAPI_WIFI_PMF_OPTIONAL)
		cli_print(output, "%d: Optional\n", pmf_enabled);
	else if (pmf_enabled == CLSAPI_WIFI_PMF_REQUIRED)
		cli_print(output, "%d: Required\n", pmf_enabled);
	else
		ret = -CLSAPI_ERR_INTERNAL_ERR;

	return ret;
}

static int cli_enable_pmf(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret;

	ret = clsapi_wifi_enable_pmf(argv[0], atoi(argv[1]));
	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_mesh_role(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_mesh_role role;

	ret = clsapi_wifi_get_mesh_role(&role);

	return clsapi_cli_report_str_value(ret, output, mesh_role_enum2str(role));
}

static int cli_set_mesh_role(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *role = (argv[0]);

	ret = clsapi_wifi_set_mesh_role(mesh_role_str2enum(role));

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_macfilter_policy(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_macfilter_policy policy = CLSAPI_WIFI_MACFILTER_POLICY_MAX;

	ret = clsapi_wifi_get_macfilter_policy(argv[0], &policy);

	return clsapi_cli_report_str_value(ret, output, macfilter_policy_enum2str(policy));
}

static int cli_set_macfilter_policy(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = clsapi_wifi_set_macfilter_policy(argv[0], macfilter_policy_str2enum(argv[1]));

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_macfilter_maclist(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	uint8_t (*mac_list)[ETH_ALEN] = NULL;
	int list_num = 0;
	int ret = clsapi_wifi_get_macfilter_maclist(argv[0], &mac_list);

	if (ret < 0)
		return ret;

	list_num = ret;
	if (list_num > 0 && mac_list == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < list_num; i++)
		cli_print(output, "  %03d: "MACFMT"\n", i, MACARG(mac_list[i]));

	if (mac_list)
		free(mac_list);

	return CLSAPI_OK;
}

static int cli_start_adv_scan(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK, j = 0, i = 0;
	uint8_t tgt_channels[255] = {0};
	uint8_t chans_len = ARRAY_SIZE(tgt_channels);
	int *flag_enable = NULL;
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;
	struct clsapi_wifi_scan_params params = {0};

	params.rx_filter = (int)atoi(argv[1]);
	params.work_duration = (int)atoi(argv[2]);
	params.scan_interval = (int)atoi(argv[3]);

	flag_enable = (int *)calloc(argc, sizeof(int));

	for (; i < argc; i++) {
		if (strcmp(argv[i], "off_channel") == 0) {
			params.flags |= CLSAPI_WIFI_SCAN_FLAG_OFF_CHANNEL;
			flag_enable[i] = 1;
		}
	}

	i = 3;
	for (int p = 0; p < argc; p++) {
		if (flag_enable[p])
			i++;
	}
	free(flag_enable);

	if (argc > ++i)
		band = strband_to_enum(argv[i++]);

	if (argc > i) {
		// scan on target channel list
		for (j = i; j < argc && j < chans_len; j++)
			tgt_channels[j - i] = atoi(argv[j]);

		chans_len = j - i;
	} else
		chans_len = 0;

	ret = clsapi_wifi_start_adv_scan(argv[0], &params, band, tgt_channels, chans_len);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_start_scan(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK, i = 0, argv_idx = 0;
	uint8_t tgt_channels[255] = {0};
	uint8_t chans_len = ARRAY_SIZE(tgt_channels);
	enum clsapi_wifi_scan_flags flags = 0;
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;

	if (argc > 1)
		flags = strflag_to_enum(argv[1]);

	if (flags == __CLSAPI_WIFI_SCAN_FLAG_MAX) {
		argv_idx = 1;
		flags = 0;
	} else
		argv_idx = 2;

	if (argc > argv_idx)
		band = strband_to_enum(argv[argv_idx++]);

	if (argc > argv_idx) {
		// scan on target channel list
		for (i = 0; i < argc - argv_idx && i < chans_len; i++)
			tgt_channels[i] = atoi(argv[i + argv_idx]);

		chans_len = i;
	} else
		chans_len = 0;

	ret = clsapi_wifi_start_scan(argv[0], flags, band, tgt_channels, chans_len);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_scan_summary(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK, i = 0;
	struct clsapi_scan_ap_info *scan_ap_entry = NULL;
	int scan_ap_cnt = 0;

	ret = clsapi_wifi_get_scan_entry(argv[0], -1, &scan_ap_entry);
	if (ret < 0)
		return ret;
	scan_ap_cnt = ret;

	if (scan_ap_entry == NULL)
		return -CLSAPI_ERR_INVALID_DATA;

	cli_print(output, "Total AP: %u\n", scan_ap_cnt);

	for (i = 0; i < scan_ap_cnt; i++)
		cli_print(output, "%3d BSSID "MACFMT"  Channel: %-3u SSID: %s\n", i,
			MACARG(scan_ap_entry[i].bssid), scan_ap_entry[i].channel, scan_ap_entry[i].ssid);

	if (scan_ap_entry)
		free(scan_ap_entry);

	return CLSAPI_OK;
}

static char * format_enc_ciphers(int ciphers)
{
	static char str[256] = {0};
	char *pos = str;

	if (ciphers & CLSAPI_WIFI_CIPHER_WEP40)
		pos += sprintf(pos, "WEP-40, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_WEP104)
		pos += sprintf(pos, "WEP-104, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_TKIP)
		pos += sprintf(pos, "TKIP, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_CCMP)
		pos += sprintf(pos, "CCMP, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_CCMP256)
		pos += sprintf(pos, "CCMP-256, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_GCMP)
		pos += sprintf(pos, "GCMP, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_GCMP256)
		pos += sprintf(pos, "GCMP-256, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_WRAP)
		pos += sprintf(pos, "WRAP, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_BIP_CMAC128)
		pos += sprintf(pos, "BIP-CMAC-128, ");

	if (ciphers & CLSAPI_WIFI_CIPHER_CKIP)
		pos += sprintf(pos, "CKIP, ");

	if (!ciphers || (ciphers & CLSAPI_WIFI_CIPHER_NONE))
		pos += sprintf(pos, "NONE, ");

	// clear last ", "
	*(pos - 2) = '\0';

	return str;
}

static char * format_enc_suites(int suites)
{
	static char str[128] = { 0 };
	char *pos = str;

	if (suites & CLSAPI_WIFI_KEY_MGMT_PSK)
		pos += sprintf(pos, "PSK/");

	if (suites & CLSAPI_WIFI_KEY_MGMT_8021X)
		pos += sprintf(pos, "802.1X/");

	if (suites & CLSAPI_WIFI_KEY_MGMT_SAE)
		pos += sprintf(pos, "SAE/");

	if (suites & CLSAPI_WIFI_KEY_MGMT_OWE)
		pos += sprintf(pos, "OWE/");

	if (!suites || (suites & CLSAPI_WIFI_KEY_MGMT_NONE))
		pos += sprintf(pos, "NONE/");

	// clear last '/'
	*(pos - 1) = '\0';

	return str;
}

static char *format_crypto(struct clsapi_wifi_sanned_crypto *crypto)
{
	static string_256 buffer;

	if (!crypto)
		snprintf(buffer, sizeof(buffer), "unknown");

	if (crypto->sec_protos == CLSAPI_WIFI_SEC_PROTO_WEP) {
		snprintf(buffer, sizeof(buffer), "WEP %s", format_enc_ciphers(crypto->pair_ciphers));
	} else if (crypto->sec_protos & (CLSAPI_WIFI_SEC_PROTO_WPA | CLSAPI_WIFI_SEC_PROTO_WPA2 | CLSAPI_WIFI_SEC_PROTO_WPA3)) {
		int wpa_cnt = 0;
		string_32 str_wpa;
		char *pos = str_wpa;

		for (int i = 0; i < 3; i++) {
			if (crypto->sec_protos & (CLSAPI_WIFI_SEC_PROTO_WPA << i)) {
				wpa_cnt++;
				if (i == 0)
					pos += sprintf(pos, "WPA/"); // WPA
				else
					pos += sprintf(pos, "WPA%d/", i + 1); // WPA2, WPA3
			}
		}
		*(pos - 1) = '\0'; // remove last '/'.

		sprintf(buffer, "%s%s %s (%s)", (wpa_cnt > 1) ? "mixed " : "", str_wpa, format_enc_suites(crypto->auth_suites),
			format_enc_ciphers(crypto->pair_ciphers | crypto->group_ciphers));
	} else
		snprintf(buffer, sizeof(buffer), "None");

	return buffer;
}

static int cli_get_scan_entry(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK, i = 0, ap_idx = -1;
	string_32 buf;
	struct clsapi_scan_ap_info *scan_ap_entry = NULL;
	int scan_ap_cnt = 0;
	char *endptr = NULL;

	if (strcmp(argv[1], "all") != 0) {
		ap_idx = strtol(argv[1], &endptr, 10);
		if (*endptr != '\0')
			return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_get_scan_entry(argv[0], ap_idx, &scan_ap_entry);
	if (ret < 0)
		return ret;

	scan_ap_cnt = ret;
	if (scan_ap_cnt > 0 && scan_ap_entry == NULL) {
		DBG_ERROR("%p\n", scan_ap_entry);
		return -CLSAPI_ERR_INVALID_DATA;
	}

	cli_print(output, "Total AP: %u\n", scan_ap_cnt);

	for (i = 0; i < scan_ap_cnt; i++) {
		cli_print(output, "\n%3d BSS "MACFMT"\n", i + 1, MACARG(scan_ap_entry[i].bssid));
		cli_print(output, "    SSID:\t%s\n", scan_ap_entry[i].ssid);
		cli_print(output, "    Channel:\t%u\n", scan_ap_entry[i].channel);
		if (scan_ap_entry[i].max_bw == CLSAPI_WIFI_BW_80_80)
			cli_print(output, "    Max BW:\t160MHz/80+80MHz\n");
		else
			cli_print(output, "    Max BW:\t%d\n", bw_enum2int(scan_ap_entry[i].max_bw));
		cli_print(output, "    RSSI:\t%ddBm\n", scan_ap_entry[i].rssi);
		snprintf(buf, sizeof(buf), "IEEE 802.11%s%s%s%s%s%s",
			(scan_ap_entry[i].hwmodes & CLSAPI_HWMODE_IEEE80211_A) ? "a/" : "",
			(scan_ap_entry[i].hwmodes & CLSAPI_HWMODE_IEEE80211_B) ? "b/" : "",
			(scan_ap_entry[i].hwmodes & CLSAPI_HWMODE_IEEE80211_G) ? "g/" : "",
			(scan_ap_entry[i].hwmodes & CLSAPI_HWMODE_IEEE80211_N) ? "n/" : "",
			(scan_ap_entry[i].hwmodes & CLSAPI_HWMODE_IEEE80211_AC) ? "ac/" : "",
			(scan_ap_entry[i].hwmodes & CLSAPI_HWMODE_IEEE80211_AX) ? "ax/" : "");
		buf[strlen(buf) - 1] = '\0';
		cli_print(output, "    HWModes:\t%s\n", buf);
		cli_print(output, "    Encryption:\t%s\n", format_crypto(&scan_ap_entry[i].crypto));
	}

	if (scan_ap_entry)
		free(scan_ap_entry);

	return CLSAPI_OK;
}

static int cli_get_csi_sta_list(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry,
	int argc, char **argv)
{
	uint8_t (*sta_mac_list)[ETH_ALEN] = NULL;
	int list_num = 0;
	int ret = clsapi_wifi_get_csi_sta_list(argv[0], &sta_mac_list);

	if (ret < 0)
		return ret;

	list_num = ret;
	if (list_num > 0 && sta_mac_list == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < list_num; i++)
		cli_print(output, "  %03d: "MACFMT"\n", i, MACARG(sta_mac_list[i]));

	if (sta_mac_list)
		free(sta_mac_list);

	return CLSAPI_OK;
}

static int cli_set_wps_config_method(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	const enum clsapi_wifi_wps_config_method wps_config_method = atol(argv[1]);

	ret = clsapi_wifi_set_wps_config_method(ifname, wps_config_method);

	return clsapi_cli_report_complete(ret, output);
}


static int cli_get_wps_config_method(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	enum clsapi_wifi_wps_config_method wps_config_method = 0;

	ret = clsapi_wifi_get_wps_config_method(ifname, &wps_config_method);
	if (ret == CLSAPI_OK)
		return clsapi_cli_report_uint_value(ret, output, wps_config_method);
	return ret;
}

static int cli_get_supported_wps_config_methods(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_wps_config_method supported_wps_config_methods = 0;

	ret = clsapi_wifi_get_supported_wps_config_methods(&supported_wps_config_methods);

	return clsapi_cli_report_uint_value(ret, output, supported_wps_config_methods);
}

const char *str_wps_status[] = {
	"WPS Not Start",        //CLSAPI_WIFI_WPS_STATUS_NOT_START
	"WPS Successful",       //CLSAPI_WIFI_WPS_STATUS_SUCCESS
	"WPS Failed"            //CLSAPI_WIFI_WPS_STATUS_FAILURE
};

const char *str_wps_pbc_status[] = {
	"WPS PBC Disabled",     //CLSAPI_WIFI_WPS_PBC_STATUS_DISABLE
	"WPS PBC Active",       //CLSAPI_WIFI_WPS_PBC_STATUS_ACTIVE
	"WPS PBC Timeout",      //CLSAPI_WIFI_WPS_PBC_STATUS_TIMEOUT
	"WPS PBC Overlap",      //CLSAPI_WIFI_WPS_PBC_STATUS_OVERLAP
	"WPS PBC Error"         //CLSAPI_WIFI_WPS_PBC_STATUS_ERROR
};

const char *str_wps_failure_reason[] = {
	"No Error",            //CLSAPI_WIFI_WPS_EI_NO_ERROR
	"TKIP Prohibited",     //CLSAPI_WIFI_WPS_EI_SECURITY_TKIP_ONLY_PROHIBITED
	"WEP Prohibited",      //CLSAPI_WIFI_WPS_EI_SECURITY_WEP_PROHIBITED
	"Auth Failure"         //CLSAPI_WIFI_WPS_EI_AUTH_FAILURE
};

static int cli_get_wps_status(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	struct clsapi_wifi_wps_status wps_status;

	ret = clsapi_wifi_get_wps_status(ifname, &wps_status);
	if (ret < 0)
		return -CLSAPI_ERR_WPA;

	cli_print(output, "WPS Status: %s\n",
			str_wps_status[wps_status.status]);

	if (wps_status.status == CLSAPI_WIFI_WPS_STATUS_FAILURE)
		cli_print(output, "failure_reason: %s\n",
				str_wps_failure_reason[wps_status.failure_reason]);

	cli_print(output, "PBC Status: %s\n",
			str_wps_pbc_status[wps_status.pbc_status]);

	cli_print(output, "Peer MAC: "MACFMT"\n",
			MACARG(wps_status.peer_addr));

	return 0;
}

static int cli_get_current_channel_utilization(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t channel_utilization = 0;

	cli_print(output, "note: off_channel scan is necessary before getting channel utilization.\n");

	ret = clsapi_wifi_get_current_channel_utilization(argv[0], &channel_utilization);
	if (ret < 0)
		return -CLSAPI_ERR_WPA;

	cli_print(output, "channel_utilization: %d\n", channel_utilization);

	return 0;
}

static int cli_check_wps_pin(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int64_t pin = atoi(argv[0]);

	if (!clsapi_wifi_check_wps_pin(pin))
		cli_print(output, "WPS PIN: %ld Checksum is OK\n", pin);
	else
		cli_print(output, "WPS PIN: %ld Checksum Failed\n", pin);

	return CLSAPI_OK;
}

static int cli_set_wps_state(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_wps_state wps_state;

	if (strcasecmp(argv[1], "Disabled") == 0)
		wps_state = CLSAPI_WIFI_WPS_STATE_DISABLED;
	else if (strcasecmp(argv[1], "Not Configured") == 0)
		wps_state = CLSAPI_WIFI_WPS_STATE_NOT_CONFIGURED;
	else if (strcasecmp(argv[1], "Configured") == 0)
		wps_state = CLSAPI_WIFI_WPS_STATE_CONFIGURED;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_set_wps_state(argv[0], wps_state);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_wps_state(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_wifi_wps_state wps_state;

	ret = clsapi_wifi_get_wps_state(argv[0], &wps_state);
	if (ret < 0)
		return clsapi_cli_report_error(ret, output);

	if (wps_state == CLSAPI_WIFI_WPS_STATE_DISABLED)
		cli_print(output, "Disabled");
	else if (wps_state == CLSAPI_WIFI_WPS_STATE_NOT_CONFIGURED)
		cli_print(output, "Not Configured");
	else if (wps_state == CLSAPI_WIFI_WPS_STATE_CONFIGURED)
		cli_print(output, "Configured");
	else
		cli_print(output, "ERROR");

	return CLSAPI_OK;
}

static int cli_get_txpwr_ratio(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum CLSAPI_WIFI_TXPWR_RATIO txpower_ratio = __CLSAPI_WIFI_TXPWR_RATIO_MAX;

	ret = clsapi_wifi_get_txpower_ratio(argv[0], &txpower_ratio);
	if (ret < 0)
		return ret;

	switch (txpower_ratio) {
	case CLSAPI_WIFI_TXPWR_RATIO_100:
		cli_print(output, "100");
		break;
	case CLSAPI_WIFI_TXPWR_RATIO_75:
		cli_print(output, "75");
		break;
	case CLSAPI_WIFI_TXPWR_RATIO_50:
		cli_print(output, "50");
		break;
	case CLSAPI_WIFI_TXPWR_RATIO_25:
		cli_print(output, "25");
		break;
	case __CLSAPI_WIFI_TXPWR_RATIO_MAX:
	default:
		return -CLSAPI_ERR_INVALID_DATA;
	}

	return ret;
}

static int cli_set_txpwr_ratio(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum CLSAPI_WIFI_TXPWR_RATIO txpower_ratio = __CLSAPI_WIFI_TXPWR_RATIO_MAX;
	int ratio = 0;

	ratio = atoi(argv[1]);
	switch (ratio) {
	case 100:
		txpower_ratio = CLSAPI_WIFI_TXPWR_RATIO_100;
		break;
	case 75:
		txpower_ratio = CLSAPI_WIFI_TXPWR_RATIO_75;
		break;
	case 50:
		txpower_ratio = CLSAPI_WIFI_TXPWR_RATIO_50;
		break;
	case 25:
		txpower_ratio = CLSAPI_WIFI_TXPWR_RATIO_25;
		break;
	default:
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = clsapi_wifi_set_txpower_ratio(argv[0], txpower_ratio);

	return clsapi_cli_report_complete(ret, output);
}

static struct clsapi_cli_entry clsapi_cli_entry_wifi_manual[] = {
	{"get ifnames", 			1, 2, "<phyname: e.g. phy0> [iftype: ap|sta]", cli_get_ifnames},
	{"get assoc_list", 			1, 1, "<ifname>", cli_get_assoc_list},
	{"deauth sta",				2, 3, "<ifname> <mac> [reason code]", cli_deauth_sta},
	{"disassoc sta",			2, 3, "<ifname> <mac> [reason code]", cli_disassociate_sta},
	{"get supported_channels",	3, 3, "<phyname> <band: 0=current, 2=2.4G, 5=5G> <bw> ", cli_get_chans},
	{"set channel",				2, 4, "<phyname> <chan: 'auto'; >0: fixed chan> <band: 0=current, 2=2.4G, 5=5G> <bw: 0=current or valid bw>", cli_set_chan},
	{"get supported_hwmodes",	1, 1, "<phyname>", cli_get_hwmodes},
	{"get hwmode",				1, 1, "<phyname>", cli_get_hwmode},
	{"set hwmode",				2, 2, "<phyname> <hwmode: a|b|g|n|ac|ax>", cli_set_hwmode},
	{"get hidden_ssid",			1, 1, "<ifname>", cli_get_hidden_ssid},
	{"set hidden_ssid",			2, 2, "<ifname> <enable: 0: disable, 1: enable>", cli_set_hidden_ssid},
	{"get supported_bws",		1, 1, "<phyname>", cli_get_bws},
	{"get bw",					1, 1, "<phyname>", cli_get_bw},
	{"set bw",					2, 2, "<phyname> <bw: 20/40/80/160>", cli_set_bw},
	{"get short_gi",			2, 2, "<phyname> <bw: 20/40/80/160>", cli_get_short_gi},
	{"set short_gi",			3, 3, "<phyname> <bw: 20/40/80/160> <enable>", cli_set_short_gi},
	{"get iftype",				1, 1, "<ifname>", cli_get_iftype},
	{"get bssid",				1, 1, "<ifname>", cli_get_bssid},
	{"get vbss_vap",			1, 1, "<ifname>", cli_get_vbss_vap},
	{"add vbss_vap",			5, 6, "<ifname> <roam | origin> <bssid> <ssid> <encryption> [pwd]", cli_add_vbss_vap},
	{"get sta_info",			2, 2, "<ifname> <sta_mac>", cli_get_sta_info},
	{"get uplink_info",			1, 1, "<ifname>", cli_get_uplink_info},
	{"get survey",				1, 1, "<phyname>", cli_get_survey},
	{"get vbss_ap_stats",		1, 1, "<ifname>", cli_get_vbss_ap_stats},
	{"get chan_score",			1, 2, "<phyname> [band: 0=current, 2=2.4G, 5=5G] max=65535 the less the better", cli_get_chan_score},
	{"get supported_bands",		1, 1, "<phyname>", cli_get_bands},
	{"get band",				1, 1, "<phyname>", cli_get_band},
	{"get encryption",			1, 1, "<ifname>", cli_get_encryption},
	{"set encryption",			2, 2, "<ifname> <encryption: none|wep-mixed|psk|psk2|sae|wpa|wpa2|wpa3|owe, etc.>", cli_set_encryption},
	{"get wep_key",				1, 1, "<ifname>", cli_get_wep_key},
	{"set wep_key",				3, 6, "<ifname> <idx_to_use> <key1> [key2] ...", cli_set_wep_key},
	{"add bss",					1, 2, "<phyname: e.g. phy0> [bssid]", cli_add_bss},
	{"get pmf_enabled",			1, 1, "<ifname>", cli_get_pmf_enabled},
	{"enable pmf",				2, 2, "<ifname> <enable_cfg: 0: disabled, 1: optional, 2: required>", cli_enable_pmf},
	{"set mesh_role",			1, 1, "<role: none|controller|agent>", cli_set_mesh_role},
	{"get mesh_role",			0, 0, "", cli_get_mesh_role},
	{"get macfilter_policy",	1, 1, "<ifname>", cli_get_macfilter_policy},
	{"set macfilter_policy",	2, 2, "<ifname> <policy: disable|allow|deny>", cli_set_macfilter_policy},
	{"get macfilter_maclist",	1, 1, "<ifname>", cli_get_macfilter_maclist},
	{"start scan",				1, 255, "<phyname> [flush_mode | off_channel] [band: 0=current, 2=2.4G, 5=5G] [channel list]", cli_start_scan},
	{"start adv_scan",			4, 255, "<phyname> <rx_filter> <work_duration> <scan_interval> [flags: off_channel] [band: 0=current, 2=2.4G, 5=5G] [channel list]", cli_start_adv_scan},
	{"get scan_summary",		1, 1, "<phyname>", cli_get_scan_summary},
	{"get scan_entry",			2, 2, "<phyname> <ap_idx: >=0 get one; 'all' get all>", cli_get_scan_entry},
	{"get radio_stats",			1, 1, "<phyname>", cli_get_radio_stats},
	{"get wpu_stats",			1, 1, "<ifname>", cli_get_wpu_stats},
	{"get vap_stats",			1, 1, "<ifname>", cli_get_vap_stats},
	{"get sta_stats",			2, 2, "<ifname> <sta_mac>", cli_get_sta_stats},
	{"reset sta_stats",			2, 2, "<ifname> <sta_mac>", cli_reset_sta_stats},
	{"get csi_sta_list",		1, 1, "<phyname>", cli_get_csi_sta_list},
	{"set wps_config_method",	2, 2, "<ifname> <wps_config_method>", cli_set_wps_config_method},
	{"get wps_config_method",	1, 1, "<ifname>", cli_get_wps_config_method},
	{"get supported_wps_config_methods",	0, 0, "", cli_get_supported_wps_config_methods},
	{"get wps_status",			1, 1, "<ifname>", cli_get_wps_status},
	{"check wps_pin",		    1, 1, "<pin_code>", cli_check_wps_pin},
	{"set wps_state",		    2, 2, "<ifname> <wps_state:Disabled|Not Configured|Configured>", cli_set_wps_state},
	{"get wps_state",		    1, 1, "<ifname>", cli_get_wps_state},
	{"get current_channel_utilization",		1, 1, "<phyname>", cli_get_current_channel_utilization},
	{"set dynamic_cca_ed_threshold",		    5, 5,
		"<phyname> <20p_risethr> <20p_fallthr> <20s_risethr> <20s_fallthr>", cli_set_dynamic_cca_ed_thr},
	{"get dynamic_cca_ed_threshold",		    1, 1, "<phyname>", cli_get_dynamic_cca_ed_thr},
	{"set ampdu_protection",		2, 2, "<phyname> <method:rts_cts|cts_only|none>",
		cli_set_ampdu_protection},
	{"set intelligent_ant_param",		8, 8, "<phyname> <enable: 0|1> <update_mode: 0|1> <gpio_mode: 0|1> <current_param1> <current_param2> <rst_param1> <rst_param2>", cli_set_intelligent_ant_param},
	{"get radius_authentification",			1, 1, "<ifname> ", cli_get_radius_authentification},
	{"set radius_authentification",			4, 4, "<ifname> <server_ip> <server_port> <server_passphrase>", cli_set_radius_authentification},
	{"get radius_acct",			1, 1, "<ifname> ", cli_get_radius_acct},
	{"set radius_acct",			4, 4, "<ifname> <acct_ip> <acct_port> <acct_passphrase>", cli_set_radius_acct},
	{"get radius_dae",			1, 1, "<ifname> ", cli_get_radius_dae},
	{"set radius_dae",			4, 4, "<ifname> <client_ip> <dae_port> <dae_passphrase>", cli_set_radius_dae},
	{"get assoc_ap_by_station",		    1, 1, "<station macaddr>", cli_get_apname_by_station},
	{"set mode_of_interface",		    2, 2, "<ifname> <mode>", cli_set_interface_mode},
	{"enable station_association",		    3, 4, "<ifname> <ssid> <encryption> <key>", cli_enable_station_association},
	{"get mesh_bss_ifname",		1, 1, "<phyname>", cli_get_mesh_bss_ifname},
	{"set txpower_ratio",		2, 2, "<phyname> <txpwr_ratio: 100/75/50/25>", cli_set_txpwr_ratio},
	{"get txpower_ratio",		1, 1, "<phyname>", cli_get_txpwr_ratio},
	{"set txpower_table",		1, 1, "<mode: highpwr/default>", cli_set_txpwr_table},
	{"get txpower_table",		0, 0, "<mode: highpwr_mode/default_mode>", cli_get_txpwr_table},
	{}
};


int clsapi_cli_wifi_init(struct clsapi_cli_entry *cli_entry_tbl[])
{
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_WIFI_AUTO] = clsapi_cli_entry_wifi_auto;
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_WIFI_MANUAL] = clsapi_cli_entry_wifi_manual;

	return CLSAPI_OK;
}


struct clsapi_cli_plugin clsapi_cli_plugin = {
	.init = clsapi_cli_wifi_init
};


