/* Automatically generated file; DO NOT EDIT. */
/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _AUTOGENED_CLI_WIFI_H
#define _AUTOGENED_CLI_WIFI_H

#include <stdio.h>
#include <stdlib.h>

static int cli_get_radio_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_radio_enabled(phyname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_enable_radio(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_radio(phyname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_beacon_intl(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint16_t beacon_int = 0;

	ret = clsapi_wifi_get_beacon_intl(phyname, &beacon_int);

	return clsapi_cli_report_uint_value(ret, output, beacon_int);
}

static int cli_set_beacon_intl(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const uint16_t beacon_int = atol(argv[1]);

	ret = clsapi_wifi_set_beacon_intl(phyname, beacon_int);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_txpower(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	int8_t txpower = 0;

	ret = clsapi_wifi_get_txpower(phyname, &txpower);

	return clsapi_cli_report_int_value(ret, output, txpower);
}

static int cli_get_txpower_limit(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	int8_t txpower = 0;

	ret = clsapi_wifi_get_txpower_limit(phyname, &txpower);

	return clsapi_cli_report_int_value(ret, output, txpower);
}

static int cli_get_country_code(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	char country_code[9] = {0};

	ret = clsapi_wifi_get_country_code(phyname, country_code);

	return clsapi_cli_report_str_value(ret, output, country_code);
}

static int cli_get_noise(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	int8_t noise = 0;

	ret = clsapi_wifi_get_noise(phyname, &noise);

	return clsapi_cli_report_int_value(ret, output, noise);
}

static int cli_get_supported_max_vap(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint16_t max_vap = 0;

	ret = clsapi_wifi_get_supported_max_vap(phyname, &max_vap);

	return clsapi_cli_report_uint_value(ret, output, max_vap);
}

static int cli_get_supported_max_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint16_t max_sta = 0;

	ret = clsapi_wifi_get_supported_max_sta(phyname, &max_sta);

	return clsapi_cli_report_uint_value(ret, output, max_sta);
}

static int cli_get_bss_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_bss_enabled(ifname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_enable_bss(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_bss(ifname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_ssid(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	char ssid[33] = {0};

	ret = clsapi_wifi_get_ssid(ifname, ssid);

	return clsapi_cli_report_str_value(ret, output, ssid);
}

static int cli_get_passphrase(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	char passphrase[65] = {0};

	ret = clsapi_wifi_get_passphrase(ifname, passphrase);

	return clsapi_cli_report_str_value(ret, output, passphrase);
}

static int cli_get_max_allow_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint16_t max_sta = 0;

	ret = clsapi_wifi_get_max_allow_sta(ifname, &max_sta);

	return clsapi_cli_report_uint_value(ret, output, max_sta);
}

static int cli_set_max_allow_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	const uint16_t max_sta = atol(argv[1]);

	ret = clsapi_wifi_set_max_allow_sta(ifname, max_sta);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_wmm_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_wmm_enabled(ifname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_enable_wmm(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_wmm(ifname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_macfilter_mac(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_add_macfilter_mac(ifname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_macfilter_mac(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_del_macfilter_mac(ifname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_sinr(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;
	int8_t sinr = 0;

	ret = clsapi_wifi_get_sinr(ifname, sta_mac, &sinr);

	return clsapi_cli_report_int_value(ret, output, sinr);
}

static int cli_get_rssi(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;
	int8_t rssi = 0;

	ret = clsapi_wifi_get_rssi(ifname, sta_mac, &rssi);

	return clsapi_cli_report_int_value(ret, output, rssi);
}

static int cli_trigger_wps_pin_connection(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	const char *uuid = (argv[1]);
	const char *pin = (argv[2]);

	ret = clsapi_wifi_trigger_wps_pin_connection(ifname, uuid, pin);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_generate_wps_pin(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint64_t pin = 0;

	ret = clsapi_wifi_generate_wps_pin(&pin);

	return clsapi_cli_report_ulong_value(ret, output, pin);
}

static int cli_enable_anti_mgmt_attack(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_anti_mgmt_attack(ifname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_pppc_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	bool onoff = 0;

	ret = clsapi_wifi_get_pppc_enabled(phyname, &onoff);

	return clsapi_cli_report_int_value(ret, output, onoff);
}

static int cli_enable_pppc(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const bool onoff = atol(argv[1]);

	ret = clsapi_wifi_enable_pppc(phyname, onoff);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_pppc_threshold_rssi(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	int8_t rssi = 0;

	ret = clsapi_wifi_get_pppc_threshold_rssi(phyname, &rssi);

	return clsapi_cli_report_int_value(ret, output, rssi);
}

static int cli_get_pppc_sta_pwr(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[0], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;
	const char *phyname = (argv[1]);
	int8_t pwr = 0;

	ret = clsapi_wifi_get_pppc_sta_pwr(sta_mac, phyname, &pwr);

	return clsapi_cli_report_int_value(ret, output, pwr);
}

static int cli_set_pppc_sta_pwr(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[0], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;
	const char *phyname = (argv[1]);
	const int8_t pwr = atol(argv[2]);

	ret = clsapi_wifi_set_pppc_sta_pwr(sta_mac, phyname, pwr);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_dynamic_cca_cs_threshold(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const int8_t threshold = atol(argv[1]);
	const int8_t delta = atol(argv[2]);

	ret = clsapi_wifi_set_dynamic_cca_cs_threshold(phyname, threshold, delta);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_dynamic_cca_cs_threshold(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	int8_t threshold = 0;

	ret = clsapi_wifi_get_dynamic_cca_cs_threshold(phyname, &threshold);

	return clsapi_cli_report_int_value(ret, output, threshold);
}

static int cli_restore_wireless_configuration(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;

	ret = clsapi_wifi_restore_wireless_configuration();

	return clsapi_cli_report_complete(ret, output);
}

static int cli_enable_csi(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_csi(phyname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_csi_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_csi_enabled(phyname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_enable_csi_non_assoc_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_csi_non_assoc_sta(phyname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_csi_non_assoc_sta_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_csi_non_assoc_sta_enabled(phyname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_enable_csi_he_smooth(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_enable_csi_he_smooth(phyname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_csi_he_smooth_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_csi_he_smooth_enabled(phyname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_set_csi_report_period(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const uint16_t report_period = atol(argv[1]);

	ret = clsapi_wifi_set_csi_report_period(phyname, report_period);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_csi_report_period(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint16_t report_period = 0;

	ret = clsapi_wifi_get_csi_report_period(phyname, &report_period);

	return clsapi_cli_report_uint_value(ret, output, report_period);
}

static int cli_add_csi_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_add_csi_sta(phyname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_csi_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_del_csi_sta(phyname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_vbss_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	bool enable = 0;

	ret = clsapi_wifi_get_vbss_enabled(phyname, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_set_vbss_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_wifi_set_vbss_enabled(phyname, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_vbss_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_del_vbss_sta(ifname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_trigger_vbss_switch(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_trigger_vbss_switch(ifname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_vbss_roam_result(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;
	uint32_t roam_result = 0;

	ret = clsapi_wifi_get_vbss_roam_result(ifname, sta_mac, &roam_result);

	return clsapi_cli_report_uint_value(ret, output, roam_result);
}

static int cli_set_vbss_switch_done(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_set_vbss_switch_done(ifname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_vbss_monitor_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_add_vbss_monitor_sta(phyname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_vbss_monitor_sta(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *phyname = (argv[0]);
	uint8_t sta_mac[ETH_ALEN] = {0};

	ret = mac_aton(argv[1], sta_mac);
	if (ret) return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_wifi_del_vbss_monitor_sta(phyname, sta_mac);

	return clsapi_cli_report_complete(ret, output);
}


struct clsapi_cli_entry clsapi_cli_entry_wifi_auto[] = {
	{"get radio_enabled", 1, 1, "<phyname>", cli_get_radio_enabled},
	{"enable radio", 2, 2, "<phyname> <enable>", cli_enable_radio},
	{"get channel", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u8ptr, clsapi_wifi_get_channel)},
	{"get beacon_intl", 1, 1, "<phyname>", cli_get_beacon_intl},
	{"set beacon_intl", 2, 2, "<phyname> <beacon_int>", cli_set_beacon_intl},
	{"get rts", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_s32ptr, clsapi_wifi_get_rts)},
	{"set rts", 2, 2, "<phyname> <rts>", cli_generic_set, C_API(clsapi_set_charptr_s32, clsapi_wifi_set_rts)},
	{"get txpower", 1, 1, "<phyname>", cli_get_txpower},
	{"get txpower_limit", 1, 1, "<phyname>", cli_get_txpower_limit},
	{"get country_code", 1, 1, "<phyname>", cli_get_country_code},
	{"set country_code", 2, 2, "<phyname> <country_code>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_wifi_set_country_code)},
	{"get noise", 1, 1, "<phyname>", cli_get_noise},
	{"get supported_max_vap", 1, 1, "<phyname>", cli_get_supported_max_vap},
	{"get supported_max_sta", 1, 1, "<phyname>", cli_get_supported_max_sta},
	{"del bss", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_del_bss)},
	{"get bss_enabled", 1, 1, "<ifname>", cli_get_bss_enabled},
	{"enable bss", 2, 2, "<ifname> <enable>", cli_enable_bss},
	{"get ssid", 1, 1, "<ifname>", cli_get_ssid},
	{"set ssid", 2, 2, "<ifname> <ssid>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_wifi_set_ssid)},
	{"get passphrase", 1, 1, "<ifname>", cli_get_passphrase},
	{"set passphrase", 2, 2, "<ifname> <passphrase>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_wifi_set_passphrase)},
	{"get max_allow_sta", 1, 1, "<ifname>", cli_get_max_allow_sta},
	{"set max_allow_sta", 2, 2, "<ifname> <max_sta>", cli_set_max_allow_sta},
	{"get dtim", 1, 1, "<ifname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u8ptr, clsapi_wifi_get_dtim)},
	{"set dtim", 2, 2, "<ifname> <dtim>", cli_generic_set, C_API(clsapi_set_charptr_u8, clsapi_wifi_set_dtim)},
	{"get wmm_enabled", 1, 1, "<ifname>", cli_get_wmm_enabled},
	{"enable wmm", 2, 2, "<ifname> <enable>", cli_enable_wmm},
	{"get sta_max_inactivity", 1, 1, "<ifname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u32ptr, clsapi_wifi_get_sta_max_inactivity)},
	{"set sta_max_inactivity", 2, 2, "<ifname> <max_inactivity>", cli_generic_set, C_API(clsapi_set_charptr_u32, clsapi_wifi_set_sta_max_inactivity)},
	{"get scan_count", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u32ptr, clsapi_wifi_get_scan_count)},
	{"add macfilter_mac", 2, 2, "<ifname> <sta_mac>", cli_add_macfilter_mac},
	{"del macfilter_mac", 2, 2, "<ifname> <sta_mac>", cli_del_macfilter_mac},
	{"get sinr", 2, 2, "<ifname> <sta_mac>", cli_get_sinr},
	{"get rssi", 2, 2, "<ifname> <sta_mac>", cli_get_rssi},
	{"cancel wps", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_cancel_wps)},
	{"trigger wps_pbc_connection", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_trigger_wps_pbc_connection)},
	{"trigger wps_pin_connection", 3, 3, "<ifname> <uuid> <pin>", cli_trigger_wps_pin_connection},
	{"get wps_static_pin", 1, 1, "<ifname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_char1024, clsapi_wifi_get_wps_static_pin)},
	{"set wps_static_pin", 2, 2, "<ifname> <ap_pin>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_wifi_set_wps_static_pin)},
	{"generate wps_pin", 0, 0, "", cli_generate_wps_pin},
	{"get wps_uuid", 1, 1, "<ifname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_char1024, clsapi_wifi_get_wps_uuid)},
	{"set wps_uuid", 2, 2, "<ifname> <uuid>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_wifi_set_wps_uuid)},
	{"enable anti_mgmt_attack", 2, 2, "<ifname> <enable>", cli_enable_anti_mgmt_attack},
	{"set anti_mgmt_attack_interval", 2, 2, "<ifname> <interval>", cli_generic_set, C_API(clsapi_set_charptr_u32, clsapi_wifi_set_anti_mgmt_attack_interval)},
	{"get rssi_smoothness_factor", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u8ptr, clsapi_wifi_get_rssi_smoothness_factor)},
	{"set rssi_smoothness_factor", 2, 2, "<phyname> <factor>", cli_generic_set, C_API(clsapi_set_charptr_u8, clsapi_wifi_set_rssi_smoothness_factor)},
	{"reset radio_stats", 1, 1, "<phyname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_reset_radio_stats)},
	{"reset wpu_stats", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_reset_wpu_stats)},
	{"reset vap_stats", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_reset_vap_stats)},
	{"get pppc_enabled", 1, 1, "<phyname>", cli_get_pppc_enabled},
	{"enable pppc", 2, 2, "<phyname> <onoff>", cli_enable_pppc},
	{"get pppc_mode", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u8ptr, clsapi_wifi_get_pppc_mode)},
	{"set pppc_mode", 2, 2, "<phyname> <mode>", cli_generic_set, C_API(clsapi_set_charptr_u8, clsapi_wifi_set_pppc_mode)},
	{"get pppc_threshold_rssi", 1, 1, "<phyname>", cli_get_pppc_threshold_rssi},
	{"set pppc_threshold_rssi", 2, 2, "<phyname> <rssi>", cli_generic_set, C_API(clsapi_set_charptr_s8, clsapi_wifi_set_pppc_threshold_rssi)},
	{"get pppc_threshold_mcs", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u8ptr, clsapi_wifi_get_pppc_threshold_mcs)},
	{"set pppc_threshold_mcs", 2, 2, "<phyname> <mcs>", cli_generic_set, C_API(clsapi_set_charptr_u8, clsapi_wifi_set_pppc_threshold_mcs)},
	{"get pppc_threshold_pps", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u32ptr, clsapi_wifi_get_pppc_threshold_pps)},
	{"set pppc_threshold_pps", 2, 2, "<phyname> <pps>", cli_generic_set, C_API(clsapi_set_charptr_u32, clsapi_wifi_set_pppc_threshold_pps)},
	{"get pppc_sta_pwr", 2, 2, "<sta_mac> <phyname>", cli_get_pppc_sta_pwr},
	{"set pppc_sta_pwr", 3, 3, "<sta_mac> <phyname> <pwr>", cli_set_pppc_sta_pwr},
	{"set dynamic_cca_cs_threshold", 3, 3, "<phyname> <threshold> <delta>", cli_set_dynamic_cca_cs_threshold},
	{"get dynamic_cca_cs_threshold", 1, 1, "<phyname>", cli_get_dynamic_cca_cs_threshold},
	{"restore wireless_configuration", 0, 0, "", cli_restore_wireless_configuration},
	{"enable csi", 2, 2, "<phyname> <enable>", cli_enable_csi},
	{"get csi_enabled", 1, 1, "<phyname>", cli_get_csi_enabled},
	{"enable csi_non_assoc_sta", 2, 2, "<phyname> <enable>", cli_enable_csi_non_assoc_sta},
	{"get csi_non_assoc_sta_enabled", 1, 1, "<phyname>", cli_get_csi_non_assoc_sta_enabled},
	{"enable csi_he_smooth", 2, 2, "<phyname> <enable>", cli_enable_csi_he_smooth},
	{"get csi_he_smooth_enabled", 1, 1, "<phyname>", cli_get_csi_he_smooth_enabled},
	{"set csi_report_period", 2, 2, "<phyname> <report_period>", cli_set_csi_report_period},
	{"get csi_report_period", 1, 1, "<phyname>", cli_get_csi_report_period},
	{"add csi_sta", 2, 2, "<phyname> <sta_mac>", cli_add_csi_sta},
	{"del csi_sta", 2, 2, "<phyname> <sta_mac>", cli_del_csi_sta},
	{"get vbss_enabled", 1, 1, "<phyname>", cli_get_vbss_enabled},
	{"set vbss_enabled", 2, 2, "<phyname> <enable>", cli_set_vbss_enabled},
	{"stop vbss_vap_txq", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_stop_vbss_vap_txq)},
	{"del vbss_vap", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_del_vbss_vap)},
	{"del vbss_sta", 2, 2, "<ifname> <sta_mac>", cli_del_vbss_sta},
	{"trigger vbss_switch", 2, 2, "<ifname> <sta_mac>", cli_trigger_vbss_switch},
	{"get vbss_roam_result", 2, 2, "<ifname> <sta_mac>", cli_get_vbss_roam_result},
	{"set vbss_switch_done", 2, 2, "<ifname> <sta_mac>", cli_set_vbss_switch_done},
	{"get vbss_nthresh", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u32ptr, clsapi_wifi_get_vbss_nthresh)},
	{"set vbss_nthresh", 2, 2, "<phyname> <n_thresh>", cli_generic_set, C_API(clsapi_set_charptr_u32, clsapi_wifi_set_vbss_nthresh)},
	{"get vbss_mthresh", 1, 1, "<phyname>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u32ptr, clsapi_wifi_get_vbss_mthresh)},
	{"set vbss_mthresh", 2, 2, "<phyname> <m_thresh>", cli_generic_set, C_API(clsapi_set_charptr_u32, clsapi_wifi_set_vbss_mthresh)},
	{"add vbss_monitor_sta", 2, 2, "<phyname> <sta_mac>", cli_add_vbss_monitor_sta},
	{"del vbss_monitor_sta", 2, 2, "<phyname> <sta_mac>", cli_del_vbss_monitor_sta},
	{"set vbss_stop_rekey", 1, 1, "<ifname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_wifi_set_vbss_stop_rekey)},

	{}
};

#endif /* _AUTOGENED_CLI_WIFI_H */