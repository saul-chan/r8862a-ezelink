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

#include "cls_dut_sta_handler.h"
#include "common/cls_cmd_parser.h"
#include "common/cls_dut_common.h"
#include "common/cls_defconf.h"
#include "common/cls_misc.h"

#include "common/csigma_log.h"
#include "common/csigma_tags.h"
#include "common/csigma_common.h"
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_cmds.h"

/* this header should be included last */

extern struct cls_npu_config cls_dut_npu_cfg;
static char g_cmdbuf[CLS_MAX_CMD_BUF];

static void _mac_str_trim(char *str, char delimiter)
{
	int i;
#define MAC_LEN 17
	for (i = 0; i < MAC_LEN; i++) {
		if (str[i] == delimiter)
			memmove(str+i, str+i+1, MAC_LEN-i);
	}
}

static int set_sta_encryption(const char *ifname, const char* ssid, const char *enc)
{
	int i;

	static const struct {
		const char *sigma_enc;
		const char *encryption;
	} map[] = {
		{ .sigma_enc = "tkip",          .encryption = "TKIPEncryption"},
		{ .sigma_enc = "aes-ccmp-128",      .encryption = "AESEncryption"},
		{ .sigma_enc = "aes-ccmp-256",      .encryption = "AESCCMP256"},
		{ .sigma_enc = "aes-ccmp",      .encryption = "AESEncryption"},
		{ .sigma_enc = "aes-ccmp-tkip", .encryption = "TKIPandAESEncryption"},
		{ .sigma_enc = "aes-gcmp-256",  .encryption = "AESGCMP256"},
		{NULL}
	};

	for (i = 0; map[i].sigma_enc != NULL; ++i) {
		if (strcasecmp(enc, map[i].sigma_enc) == 0) {
			break;
		}
	}

	if (map[i].sigma_enc == NULL) {
		return -EINVAL;
	}

	return clsapi_SSID_set_encryption_modes(ifname, ssid, map[i].encryption);
}

static int set_sta_group_encryption(const char *ifname, const char *ssid, const char *enc)
{
	int i;
	static const struct {
		const char *sigma_enc;
		const char *encryption;
	} map[] = {
		{ .sigma_enc = "tkip",          .encryption = "TKIP"},
		{ .sigma_enc = "aes-ccmp-128",      .encryption = "CCMP"},
		{ .sigma_enc = "aes-ccmp-256",      .encryption = "CCMP-256"},
		{ .sigma_enc = "aes-ccmp",      .encryption = "CCMP"},
		{ .sigma_enc = "aes-ccmp-tkip", .encryption = "CCMP TKIP"},
		{ .sigma_enc = "AES-GCMP-256",  .encryption = "GCMP-256"},
		{ NULL}
	};

	for (i = 0; map[i].sigma_enc != NULL; ++i) {
		if (strcasecmp(enc, map[i].sigma_enc) == 0)
			break;
	}

	if (map[i].sigma_enc == NULL)
		return -EINVAL;

	return clsapi_SSID_set_group_encryption(ifname, ssid, map[i].encryption);
}

static int set_sta_keymgmt(const char *ifname, const char *ssid, const char *type)
{
	int i;
	static const struct {
		const char *keymgnt;
		const char *auth;
	} keymgnt_map[] = {
		{ .keymgnt = "NONE",	.auth = "NONE"},
		{ .keymgnt = "OPEN",	.auth = "NONE"},
		{ .keymgnt = "WPA-PSK",	.auth = "PSKAuthentication"},
		{ .keymgnt = "SAE",	.auth = "SAEAuthentication"},
		{ .keymgnt = "PSK-SAE",	.auth = "SAEandPSKAuthentication"},
		{ .keymgnt = "OWE",	.auth = "OPENandOWEAuthentication"},
		{ .keymgnt = "WPA-EAP-SUITE-B-192", .auth = "EAPSUITEBAuthen"},
		{ .keymgnt = "WPA-EAP", .auth = "EAPAuthentication"},
		{ NULL}
	};

	for (i = 0; keymgnt_map[i].keymgnt != NULL; ++i) {
		if (strcasecmp(type, keymgnt_map[i].keymgnt) == 0)
			break;
	}

	if (keymgnt_map[i].keymgnt == NULL)
		return -EINVAL;

	return clsapi_SSID_set_authentication_mode(ifname, ssid, keymgnt_map[i].auth);

}

static int set_sta_protocol(const char *ifname, const char *ssid, const char *keymgmt)
{
	int i;
	static const struct {
		const char *keymgmt;
		const char *proto;
	} proto_map[] = {
		{ .keymgmt = "WPA",		.proto = "WPAand11i"},
		{ .keymgmt = "WPA-PSK",		.proto = "WPAand11i"},
		{ .keymgmt = "WPA2-WPA-PSK",	.proto = "WPAand11i"},
		{ .keymgmt = "SAE",		.proto = "11i"},
		{ .keymgmt = "OWE",		.proto = "11i"},
		{ .keymgmt = "WPA2",     .proto = "11i"},
		{ NULL}
	};

	if (strcmp(keymgmt, "") == 0)
		return 0;

	for (i = 0; proto_map[i].keymgmt != NULL; ++i) {
		if (strcasecmp(keymgmt, proto_map[i].keymgmt) == 0)
			break;
	}

	if (proto_map[i].keymgmt == NULL)
		return 0;

	return clsapi_SSID_set_protocol(ifname, ssid, proto_map[i].proto);
}

static int set_sta_openssl_ciphers(const char *ifname, const char *ssid, const char *cert_type)
{
	int i;
	struct clsapi_set_parameters set_params;

	static const struct {
		const char *cert_type;
		const char *ciphers;
	} ciphers_map[] = {
		{ .cert_type = "ECC",
			.ciphers = "ECDHE-ECDSA-AES256-GCM-SHA384"},
		{ .cert_type = "RSA",
			.ciphers = "ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384"},
		{ NULL}
	};

	if (strcmp(cert_type, "") == 0)
		return 0;

	for (i = 0; ciphers_map[i].cert_type != NULL; ++i) {
		if (strcasecmp(cert_type, ciphers_map[i].cert_type) == 0)
			break;
	}

	if (ciphers_map[i].cert_type == NULL)
		return 0;

	memset(&set_params, 0, sizeof(set_params));
	strncpy(set_params.param[0].key, "openssl_ciphers",
			sizeof(set_params.param[0].key) - 1);
	strncpy(set_params.param[0].value, ciphers_map[i].ciphers,
			sizeof(set_params.param[0].value) - 1);

	return clsapi_set_params(ifname, ssid, &set_params);
}

void cnat_sta_device_list_interfaces(int tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_dut_response rsp = { 0 };
	char ifname[IFNAMSIZ];

	for (int radio_id = 0; radio_id < CLS_MAX_RADIO_ID; ++radio_id) {
		if (clsapi_radio_get_primary_interface(radio_id, ifname, sizeof(ifname)) == 0) {
			size_t used = strlen(rsp.ap_info.interface_list);
			size_t left = sizeof(rsp.ap_info.interface_list) - used;
			snprintf(rsp.ap_info.interface_list + used, left, "%s%s",
				used == 0 ? "" : " ", ifname);
			/*
			 * UCC will use all interfaces in list for configuration. For example:
			 *  ---> sta_reset_default,interface,wifi0_0 wifi2_0,prog,VHT,type,DUT
			 * WAR: keep only 1 interface in list for now,
			 * need to add more later for 11n DBDC test cases.
			 */
			break;
		}
	}

	rsp.status = STATUS_COMPLETE;
	wfaEncodeTLV(tag, sizeof(rsp), (BYTE *) & rsp, out);

	*out_len = WFA_TLV_HDR_LEN + sizeof(rsp);
}

void cls_handle_sta_reset_default(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	clsapi_wifi_mode current_mode;
	char ifname[IFNAMSIZ];
	char cert_prog[16];
	char conf_type[16];
	struct cls_dut_config *conf;
	enum cls_dut_band_index band_idx;
	int freq_band;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	cls_init_sigma_interfaces();

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	clsapi_unsigned_int rf_status;
	if (clsapi_wifi_rfstatus(&rf_status) == 0 && rf_status == 0) {
		int rf_enable_timeout = 5;
		int timeout = 60;

		cls_set_rf_enable_timeout(1, rf_enable_timeout);

		while (timeout-- > 0 && rf_status == 0) {
			sleep(1);
			clsapi_wifi_rfstatus(&rf_status);
			cls_log("wait RF, status %d, timeout %d", rf_status, timeout);
		}
	}

	if ((ret = clsapi_wifi_get_mode(ifname, &current_mode)) < 0) {
		cls_error("can't get mode, error %d", ret);
		status = STATUS_ERROR;
		goto respond;
	}

	if (current_mode != clsapi_station) {
		cls_error("mode %d is wrong, should be STA", current_mode);
		status = STATUS_ERROR;
		ret = -clsapi_only_on_STA;
		goto respond;
	}

	/* disassociate to be sure that we start disassociated. possible error is ignored. */
	clsapi_wifi_disassociate(ifname);

	/* mandatory certification program, e.g. VHT */
	if (cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, cert_prog, sizeof(cert_prog)) <= 0
		&& cls_get_value_text(&cmd_req, CLS_TOK_PROG, cert_prog, sizeof(cert_prog)) <= 0) {
		ret = -EINVAL;
		status = STATUS_ERROR;
		goto respond;
	}

	/* optional configuration type, e.g. DUT or Testbed */
	if (cls_get_value_text(&cmd_req, CLS_TOK_TYPE, conf_type, sizeof(conf_type)) <= 0) {
		/* not specified */
		*conf_type = 0;
	}

	conf = cls_dut_get_config(ifname);
	if (conf) {
		conf->testbed_enable = (strcasecmp(conf_type, "Testbed") == 0);
	}

	freq_band = cls_get_sigma_band_info_from_interface(ifname);
	band_idx = cls_get_sigma_interface_band_idx(freq_band);

	cls_set_sigma_first_active_band_idx(band_idx);

	/* Certification program shall be: PMF, WFD, P2P, VHT or HE */
	if (strcasecmp(cert_prog, "HE") == 0) {
		if (strcasecmp(conf_type, "Testbed") == 0) {
			ret = cls_defconf_he_testbed_sta(ifname);
		} else {
			ret = cls_defconf_he_dut_sta(ifname);
		}

		if (ret < 0) {
			cls_error("error: default configuration, errcode %d", ret);
			status = STATUS_ERROR;
			goto respond;
		}
	} else if (strcasecmp(cert_prog, "VHT") == 0) {
		if (strcasecmp(conf_type, "Testbed") == 0) {
			ret = cls_defconf_vht_testbed_sta(ifname);
		} else {
			ret = cls_defconf_vht_dut_sta(ifname);
		}

		if (ret < 0) {
			cls_error("error: default configuration, errcode %d", ret);
			status = STATUS_ERROR;
			goto respond;
		}
	} else if (strcasecmp(cert_prog, "11n") == 0) {
		if (strcasecmp(conf_type, "Testbed") == 0) {
			ret = cls_defconf_11n_testbed(ifname);
		} else {
			ret = cls_defconf_11n_dut(ifname, "11na");
		}

		if (ret < 0) {
			cls_error("error: default configuration, errcode %d", ret);
			status = STATUS_ERROR;
			goto respond;
		}
	} else if (strcasecmp(cert_prog, "PMF") == 0) {
		ret = cls_defconf_pmf_dut(ifname);
		if (ret < 0) {
			cls_error("error: default configuration, errcode %d", ret);
			status = STATUS_ERROR;
			goto respond;
		}
	} else if (strcasecmp(cert_prog, "WPA3") == 0) {
		ret = cls_defconf_wpa3_dut_sta(ifname);
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
		/* TODO: processing for other programs */
		cls_error("error: prog %s is not supported", cert_prog);
		ret = -ENOTSUP;
		status = STATUS_ERROR;
		goto respond;
	}

	/* TODO: Other options */
	ret = clsapi_wifi_set_option(ifname, clsapi_autorate_fallback, 1);
	if (ret < 0) {
		cls_error("error: cannot set autorate, errcode %d", ret);
		status = STATUS_ERROR;
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_disconnect(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname[IFNAMSIZ];
	char maintain_profile[16] = "0";

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_MAINTAIN_PROFILE,
				maintain_profile, sizeof(maintain_profile)) <= 0)
		cls_log("no maintain_profile");

	cls_log("maintain_profile is %s", maintain_profile);

	if (strcmp(maintain_profile, "1") != 0) {
		ret = clsapi_wifi_disassociate(ifname);
		if (ret < 0)
			cls_error("can't disassociate interface %s, error %d", ifname, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_send_addba(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname[IFNAMSIZ];
	char cmd[128];
	int tid;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_TID, &tid) <= 0) {
		cls_error("no TID in request");
		status = STATUS_INVALID;
		goto respond;
	}

	snprintf(cmd, sizeof(cmd), "iwpriv %s htba_addba %d", ifname, tid);
	ret = system(cmd);
	if (ret != 0) {
		cls_log("can't send addba using [%s], error %d", cmd, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_preset_testparameters(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	char ifname_buf[IFNAMSIZ];
	const char *ifname;
	char val_buf[128];
	int ret;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));
	ifname = (ret > 0) ? ifname_buf : cls_get_sigma_interface();

	ret = cls_get_value_text(&cmd_req, CLS_TOK_MODE, val_buf, sizeof(val_buf));
	if (ret > 0) {
		const char *phy_mode = val_buf;
		clsapi_unsigned_int old_bw;
		if (clsapi_wifi_get_bw(ifname, &old_bw) < 0) {
			old_bw = 80;
		}

		ret = cls_set_phy_mode(ifname, phy_mode);

		if (ret < 0) {
			status = STATUS_ERROR;
			goto respond;
		}

		cls_log("try to restore %d mode since phy change", old_bw);
		if (clsapi_wifi_set_bw(ifname, old_bw) < 0)
			cls_error("failed to set bandwidth to %dMHz", old_bw);

		if (strcasecmp(phy_mode, "11ac") == 0 || strcasecmp(phy_mode, "11ax") == 0) {
			// restore 80 MHz bandwidth unless it is configured explictly
			cls_log("restore 80MHz mode since phy is 11ac");
			if (clsapi_wifi_set_bw(ifname, cls_is_2p4_interface(ifname) ? 20 : 80 ) < 0)
				cls_error("failed to set bandwidth to 80MHz");
		}
	}

	ret = cls_get_value_text(&cmd_req, CLS_TOK_WMM, val_buf, sizeof(val_buf));
	if (ret > 0) {
		char tmpbuf[64];
		int wmm_on = (strncasecmp(val_buf, "on", 2) == 0) ? 1 : 0;

		/* TODO: clsapi specifies enable/disable WMM only for AP */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s wmm %d", ifname, wmm_on);
		system(tmpbuf);
	}

	ret = cls_get_value_text(&cmd_req, CLS_TOK_NOACK, val_buf, sizeof(val_buf));
	if (ret > 0) {
		/* BE_Policy:BK_Policy:VI_Policy:VO_Policy */
		cls_log("CLS_TOK_NOACK: %s\n", val_buf);
		char* policy_saveptr;
		char* policy = strtok_r(val_buf, ":", &policy_saveptr);
		int i;

		static const int stream_classes[] = { WME_AC_BE, WME_AC_BK, WME_AC_VI, WME_AC_VO };

		for (i = 0; i < 4 && policy != NULL; ++i, policy = strtok_r(NULL, ":", &policy_saveptr)) {

			int noackpolicy = (strcasecmp(policy, "enable") == 0);

			ret = clsapi_wifi_qos_set_param(ifname, 2,
								stream_classes[i],
								IEEE80211_WMMPARAMS_NOACKPOLICY,
								0,
								noackpolicy);
			if (ret < 0) {
				cls_error("error: can't set noackpolicy to %d, error %d",
						noackpolicy,
						ret);
			}
		}
	}

	/* TODO: RTS FRGMNT
	 *   sta_preset_testparameters,interface,rtl8192s ,supplicant,ZeroConfig,mode,11ac,RTS,500
	 *   sta_preset_testparameters,interface,eth0,supplicant,ZeroConfig,mode,11ac,FRGMNT,2346
	 */

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_get_mac_address(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname_buf[16];
	const char *ifname;
	unsigned char macaddr[IEEE80211_ADDR_LEN];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));

	ifname = (ret > 0) ? ifname_buf : cls_get_sigma_interface();

	ret = clsapi_interface_get_mac_addr(ifname, macaddr);

	if (ret < 0) {
		status = STATUS_ERROR;
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_macaddr(cmd_tag, status, ret, macaddr, out_len, out);
}

void cls_handle_sta_get_info(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname_buf[16];
	char info_buf[128] = {0};
	int info_len = 0;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));

	ret = snprintf(info_buf + info_len, sizeof(info_buf) - info_len,
			"vendor,%s,build_name,%s", "Clourneysemi", CDRV_BLD_NAME);

	if (ret < 0) {
		status = STATUS_ERROR;
		goto respond;
	}

	info_len += ret;

	/* TODO: add other information */

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_vendor_info(cmd_tag, status, ret, info_buf, out_len, out);
}

void cls_handle_sta_set_wireless(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_ERROR;
	int ret;
	char ifname_buf[16];
	const char *ifname;
	char cert_prog[32];
	int he_prog;
	int vht_prog;
	int feature_enable;
	int feature_val;
	char val_buf[128];
	int conv_err = 0;
	struct cls_dut_config *conf;
	enum cls_dut_band_index band_idx;
	int freq_band;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);

	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));

	ifname = (ret > 0) ? ifname_buf : cls_get_sigma_interface();
	conf = cls_dut_get_config(ifname);

	ret = cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, cert_prog, sizeof(cert_prog));
	if (ret <= 0) {
		ret = cls_get_value_text(&cmd_req, CLS_TOK_PROG, cert_prog, sizeof(cert_prog));
	}
	if (ret <= 0) {
		/* mandatory parameter */
		cls_error("PROGRAM parameter is mandatory");
		goto respond;
	}

	freq_band = cls_get_sigma_band_info_from_interface(ifname);
	band_idx = cls_get_sigma_interface_band_idx(freq_band);

	cls_set_sigma_first_active_band_idx(band_idx);

	he_prog = (strcasecmp(cert_prog, "HE") == 0) ? 1 : 0;
	vht_prog = (strcasecmp(cert_prog, "VHT") == 0) ? 1 : 0;

	/* ADDBA_REJECT, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_ADDBA_REJECT, &feature_enable, &conv_err) > 0) {
		/* ADDBA_REJECT:enabled => ADDBA.Request:disabled */
		clsapi_wifi_set_rxba_decline(ifname, feature_enable);
	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* AMPDU, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_AMPDU, &feature_enable, &conv_err) > 0) {
		cls_set_ampdu(ifname, feature_enable);

		/* TODO: check if AuC is able to make AMSDU aggregation for VHT single AMPDU */

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* AMSDU, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_AMSDU, &feature_enable, &conv_err) > 0) {
		ret = cls_set_amsdu(ifname, feature_enable);

		if (ret < 0) {
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* STBC_RX, int (0/1) */
	if (cls_get_value_int(&cmd_req, CLS_TOK_STBC_RX, &feature_val) > 0) {
		/* enable/disable STBC */
		ret = clsapi_wifi_set_option(ifname, clsapi_stbc, feature_val);

		if (ret < 0) {
			goto respond;
		}

		if (feature_val > 0) {
			/* TODO: set number of STBC Receive Streams */
		}
	}

	/* WIDTH, int (80/40/20) */
	if (cls_get_value_int(&cmd_req, CLS_TOK_WIDTH, &feature_val) > 0) {
		if (!conf) {
			ret = -EFAULT;
			goto respond;
		}

		if (feature_val == 0) {
			if (cls_get_value_text(&cmd_req, CLS_TOK_WIDTH,
								val_buf, sizeof(val_buf)) > 0) {
				if (strcasecmp(val_buf, "auto") == 0) {
					ret = clsapi_wifi_get_bw(ifname,
							(clsapi_unsigned_int *)&feature_val);
				} else {
					ret = -EOPNOTSUPP;
					goto respond;
				}
			} else {
				ret = -EFAULT;
				goto respond;
			}
		}

		/* channel width */
		ret = clsapi_wifi_set_bw(ifname, (unsigned) feature_val);
		if (ret < 0) {
			cls_log("can't set width to %d, error %d", feature_val, ret);
			goto respond;
		}

		cls_set_fixed_bw(ifname, feature_val);
	}

	/* SMPS, SM Power Save Mode, NOT Supported */
	if (cls_get_value_int(&cmd_req, CLS_TOK_SMPS, &feature_val) > 0) {
		ret = -EOPNOTSUPP;

		if (ret < 0) {
			goto respond;
		}
	}

	/* TXSP_STREAM, (1SS/2SS/3SS) */
	if (cls_get_value_text(&cmd_req, CLS_TOK_TXSP_STREAM, val_buf, sizeof(val_buf)) > 0) {
		int nss = 0;
		/* TODO: recheck for HE program */
		clsapi_mimo_type mt = he_prog || vht_prog ? clsapi_mimo_vht : clsapi_mimo_ht;

		ret = sscanf(val_buf, "%dSS", &nss);

		if (ret != 1) {
			ret = -EINVAL;
			goto respond;
		}

		ret = cls_set_nss_cap(ifname, mt, nss);

		if (ret < 0) {
			goto respond;
		}
	}

	/* RXSP_STREAM, (1SS/2SS/3SS) */
	if (cls_get_value_text(&cmd_req, CLS_TOK_RXSP_STREAM, val_buf, sizeof(val_buf)) > 0) {
		int nss = 0;
		/* TODO: recheck for HE program */
		clsapi_mimo_type mt = he_prog || vht_prog ? clsapi_mimo_vht : clsapi_mimo_ht;

		ret = sscanf(val_buf, "%dSS", &nss);

		if (ret != 1) {
			ret = -EINVAL;
			goto respond;
		}

		ret = cls_set_nss_cap(ifname, mt, nss);

		if (ret < 0) {
			goto respond;
		}
	}

	/* Band, NOT Supported */
	if (cls_get_value_text(&cmd_req, CLS_TOK_BAND, val_buf, sizeof(val_buf)) > 0) {
		ret = -EOPNOTSUPP;

		if (ret < 0) {
			goto respond;
		}
	}

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
		status = STATUS_ERROR;
		goto respond;
	}

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


	if (conf && conf->update_settings) {
		cls_set_rts_settings(ifname, conf);
	}

	/* SGI80, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_SGI80, &feature_enable, &conv_err) > 0) {
		/* disable dynamic GI selection */
		ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 0);
		if (ret < 0) {
			cls_error("can't disable dynamic GI selection, error %d", ret);
			ret = 0;
			/* ^^ ignore error since clsapi_GI_probing does not work for RFIC6 */
		}


		/* TODO: it sets general capability for short GI, not only SGI80 */
		ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, feature_enable);

		if (ret < 0) {
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* TxBF, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_TXBF, &feature_enable, &conv_err) > 0) {
		/* TODO: check, that we enable/disable SU TxBF beamformee capability
		 * with explicit feedback */
		ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, feature_enable);

		if (ret < 0) {
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* LDPC, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_LDPC, &feature_enable, &conv_err) > 0) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_ldpc %d", ifname, feature_enable);
		system(tmpbuf);

		/* TODO: what about IEEE80211_PARAM_LDPC_ALLOW_NON_CLS ?
		 *       Allow non CLS nodes to use LDPC */

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* Opt_md_notif_ie, (NSS=1 & BW=20Mhz => 1;20) */
	if (cls_get_value_text(&cmd_req, CLS_TOK_OPT_MD_NOTIF_IE, val_buf, sizeof(val_buf)) > 0) {
		int nss = 0;
		int bw = 0;
		uint8_t chwidth;
		uint8_t rxnss;
		uint8_t rxnss_type = 0;
		uint8_t vhtop_notif_mode;
		char tmpbuf[64];

		ret = sscanf(val_buf, "%d;%d", &nss, &bw);

		if (ret != 2) {
			ret = -EINVAL;
			goto respond;
		}

		switch (bw) {
		case 20:
			chwidth = IEEE80211_CWM_WIDTH20;
			break;
		case 40:
			chwidth = IEEE80211_CWM_WIDTH40;
			break;
		case 80:
			chwidth = IEEE80211_CWM_WIDTH80;
			break;
		default:
			ret = -EINVAL;
			goto respond;
		}

		if ((nss < 1) || (nss > IEEE80211_AC_MCS_NSS_MAX)) {
			ret = -EINVAL;
			goto respond;
		}

		rxnss = nss - 1;

		vhtop_notif_mode = SM(chwidth, IEEE80211_VHT_OPMODE_CHWIDTH) |
				SM(rxnss, IEEE80211_VHT_OPMODE_RXNSS) |
				SM(rxnss_type, IEEE80211_VHT_OPMODE_RXNSS_TYPE);

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
				ifname,
				vhtop_notif_mode);
		system(tmpbuf);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MCS_FIXEDRATE, &feature_val) > 0) {
		if ((ret = cls_set_fixed_mcs_rate(ifname, feature_val)) < 0) {
			goto respond;
		}
	}

	/* nss_mcs_cap, (nss_capabilty;mcs_capability => 2;0-9) */
	if (cls_get_value_text(&cmd_req, CLS_TOK_NSS_MCS_CAP, val_buf, sizeof(val_buf)) > 0) {
		int nss = 0;
		int mcs_high = 0;

		ret = sscanf(val_buf, "%d;0-%d", &nss, &mcs_high);

		if (ret != 2) {
			ret = -EINVAL;
			goto respond;
		}

		ret = cls_set_nss_mcs_cap(ifname, nss, 0, mcs_high);

		if (ret < 0) {
			goto respond;
		}

	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MAXHE_MCS_1SS_RXMAPLTE80, &feature_val) > 0) {
		cls_set_mcs_cap(ifname, feature_val);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MAXHE_MCS_2SS_RXMAPLTE80, &feature_val) > 0) {
		cls_set_mcs_cap(ifname, feature_val);
	}

	/* Tx_lgi_rate, int (0) */
	if (cls_get_value_int(&cmd_req, CLS_TOK_TX_LGI_RATE, &feature_val) > 0) {
		/* setting Tx Highest Supported Long GI Data Rate
		 */
		if (feature_val != 0) {
			/* we support only 0 */
			ret = -EOPNOTSUPP;
			goto respond;
		}
	}

	/* Zero_crc (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_ZERO_CRC, &feature_enable, &conv_err) > 0) {
		/* setting VHT SIGB CRC to fixed value (e.g. all "0") not supported
		 * for current hardware platform
		 * VHT SIGB CRC is always calculated
		 * tests: 4.2.26
		 */

		ret = -EOPNOTSUPP;

		if (ret < 0) {
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* Vht_tkip (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_VHT_TKIP, &feature_enable, &conv_err) > 0) {
		/* enable TKIP in VHT mode
		 * Tests: 4.2.44
		 * Testbed Wi-Fi CERTIFIED ac with the capability of setting TKIP and VHT
		 * and ability to generate a probe request.
		 */
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_tkip %d",
				ifname, feature_enable);
		system(tmpbuf);

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* Vht_wep, (enable/disable), NOT USED IN TESTS (as STA testbed) */

	/* BW_SGNL, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_BW_SGNL, &feature_enable, &conv_err) > 0) {
		/* Tests: 4.2.51
		 * STA1: Testbed Wi-Fi CERTIFIED ac STA supporting the optional feature RTS
		 *       with BW signaling
		 */

		struct cls_dut_config *conf = cls_dut_get_config(ifname);

		if (conf) {
			conf->bws_enable = (unsigned char)feature_enable;
		} else {
			ret = -EFAULT;
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* MU_TxBF, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_MU_TXBF, &feature_enable, &conv_err) > 0) {
		/* TODO: enable/disable Multi User (MU) TxBF beamformee capability
		 * with explicit feedback
		 *
		 * Tests: 4.2.56
		 */
		int su_status = 0;
		if (feature_enable &&
			clsapi_wifi_get_option(ifname, clsapi_beamforming, &su_status) >= 0
			&& su_status == 0) {
			/* have to have SU enabled if we enable MU */
			ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 1);
			if (ret < 0) {
				cls_error("can't enable beamforming, error %d", ret);
				ret = 0;
			}
		}

		ret = cls_set_mu_enable(ifname, (unsigned)feature_enable);

		if (ret < 0) {
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	/* CTS_WIDTH, int (0) */
	if (cls_get_value_int(&cmd_req, CLS_TOK_CTS_WIDTH, &feature_val) > 0) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_cts_bw %d",
				ifname, feature_val);
		system(tmpbuf);
	}


	/* RTS_BWS, (enable/disable) */
	if (cls_get_value_enable(&cmd_req, CLS_TOK_RTS_BWS, &feature_enable, &conv_err) > 0) {
		/* TODO: enable RTS with Bandwidth Signaling Feature
		 *
		 * Tests: 4.2.59
		 */

		ret = -EOPNOTSUPP;

		if (ret < 0) {
			goto respond;
		}

	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_NSS_MCS_OPT, val_buf, sizeof(val_buf)) > 0) {
		if ((ret = cls_set_nss_mcs_opt(ifname, val_buf)) < 0) {
			cls_error("can't set nss_mcs_opt to %s, if_name %s, error %d",
					val_buf, ifname, ret);
			goto respond;
		}
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_TXBANDWIDTH, &feature_val) > 0 &&
		(ret = set_tx_bandwidth(ifname, feature_val)) < 0) {
		cls_error("can't set bandwidth to %d, error %d", feature_val, ret);
		status = STATUS_ERROR;
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_rfeature(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname[IFNAMSIZ];
	char val_str[128];
	int feature_val;
	int conv_err;
	char he_ltf_str[8];
	char he_gi_str[8];
	int omi_rxnss;
	int omi_chwidth;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0)
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());

	status = STATUS_ERROR;

	if (cls_get_value_text(&cmd_req, CLS_TOK_NSS_MCS_OPT, val_str, sizeof(val_str)) > 0) {
		if ((ret = cls_set_nss_mcs_opt(ifname, val_str)) < 0) {
			cls_error("can't set nss_mcs_opt to %s, if_name %s, error %d",
					val_str, ifname, ret);
			goto respond;
		}
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SECCHOFFSET, val_str, sizeof(val_str)) > 0) {
		clsapi_unsigned_int current_channel;
		clsapi_wifi_wait_scan_completes(ifname, CLS_SCAN_TIMEOUT_SEC);
		ret = clsapi_wifi_get_channel(ifname, &current_channel);
		if (ret < 0) {
			cls_error("can't get current channel, error %d", ret);
			goto respond;
		}

		if (strcasecmp(val_str, "40above") == 0) {
			ret = clsapi_wifi_set_sec_chan(ifname, current_channel, 0);
		} else if (strcasecmp(val_str, "40below") == 0) {
			ret = clsapi_wifi_set_sec_chan(ifname, current_channel, 1);
		} else if (strcasecmp(val_str, "20") == 0) {
			ret = clsapi_wifi_set_bw(ifname, 20);
		}

		if (ret < 0) {
			cls_error("can't set sec_chan, current ch %d, offset %s, error %d",
				current_channel, val_str, ret);
			/* ignore error since UCC can configure secondary channel for 5GHz too. */
			ret = 0;
		}
	}

	if (cls_get_value_enable(&cmd_req, CLS_TOK_UAPSD, &feature_val, &conv_err) > 0) {
		ret = clsapi_wifi_set_option(ifname, clsapi_uapsd, feature_val);
		if (ret < 0) {
			cls_error("can't set uapsd to %d, error %d", feature_val, ret);
			goto respond;
		}
	} else if (conv_err < 0) {
		ret = conv_err;
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_OMI_RXNSS, &omi_rxnss) > 0 &&
			cls_get_value_int(&cmd_req, CLS_TOK_OMI_CHWIDTH, &omi_chwidth) > 0) {
		char tmpbuf[64];

		snprintf(tmpbuf, sizeof(tmpbuf), "send_he_opmode %s 0 -r %d -b %d",
							ifname, omi_rxnss + 1, omi_chwidth);
		system(tmpbuf);
	}

	/* HE LTF+GI */
	if (cls_get_value_text(&cmd_req, CLS_TOK_LTF, he_ltf_str, sizeof(he_ltf_str)) > 0 &&
		cls_get_value_text(&cmd_req, CLS_TOK_GI, he_gi_str, sizeof(he_gi_str)) > 0) {
		if (cls_set_he_ltf_gi(ifname, he_ltf_str, he_gi_str, 0) < 0) {
			cls_error("can't set %sus GI with %sus HE-LTF", he_gi_str, he_ltf_str);
			goto respond;
		}
	}

	status = STATUS_COMPLETE;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_ip_config(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	struct cls_dut_config *conf;
	int status;
	int ret;
	char ifname[IFNAMSIZ];
	char str_val[128];
	int int_val;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	status = STATUS_ERROR;

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", CLSAPI_PRIMARY_WIFI_IFNAME);
	}

	conf = cls_dut_get_config(ifname);
	if (!conf) {
		ret = -EFAULT;
		cls_error("can't find configuration for %s", ifname);
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_IP, str_val, sizeof(str_val)) > 0) {
		snprintf(conf->sta_ip, sizeof(conf->sta_ip), "%.15s", str_val);
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_MASK, str_val, sizeof(str_val)) > 0) {
		snprintf(conf->sta_mask, sizeof(conf->sta_mask), "%.15s", str_val);
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_DHCP, &int_val) > 0) {
		conf->sta_dhcp_enabled = int_val;
	}

	status = STATUS_COMPLETE;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_get_ip_config(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	struct cls_dut_config *conf;
	int status = STATUS_COMPLETE;
	int ret;
	char ifname[IFNAMSIZ];
	char info_buf[128] = {0};

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", CLSAPI_PRIMARY_WIFI_IFNAME);
	}

	conf = cls_dut_get_config(ifname);
	if (!conf) {
		cls_error("can't fing configuration for %s", ifname);
		ret = snprintf(info_buf, sizeof(info_buf),
			"dhcp,%d,ip,%s,mask,%s,primary-dns,127.0.0.1",
			0, "127.0.0.1", "255.255.0.0");
	} else {
		ret = snprintf(info_buf, sizeof(info_buf),
			"dhcp,%d,ip,%s,mask,%s,primary-dns,127.0.0.1",
			conf->sta_dhcp_enabled, conf->sta_ip, conf->sta_mask);
	}
	if (ret < 0) {
		goto respond;
	}

	ret = 0;

respond:
	cls_dut_make_response_vendor_info(cmd_tag, status, ret, info_buf, out_len, out);
}


void cls_handle_sta_set_psk(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret;
	char ifname[IFNAMSIZ];
	char ssid_str[128];
	char pass_str[128];
	char key_type[128];
	char enc_type[128];
	char pmf_type[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid_str, sizeof(ssid_str)) <= 0) {
		cls_error("can't get ssid");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_PASSPHRASE, pass_str, sizeof(pass_str)) <= 0) {
		cls_error("can't get pass phrase");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_KEYMGMTTYPE, key_type, sizeof(key_type)) <= 0) {
		cls_error("can't get pass key_type");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_ENCPTYPE, enc_type, sizeof(enc_type)) <= 0) {
		cls_error("can't get enc_type");
		goto respond;
	}

	status = STATUS_ERROR;

	if (clsapi_SSID_verify_SSID(ifname, ssid_str) < 0 &&
			(ret = clsapi_SSID_create_SSID(ifname, ssid_str)) < 0) {
		cls_error("can't create SSID %s, error %d", ssid_str, ret);
		goto respond;
	}

	if ((ret = set_sta_encryption(ifname, ssid_str, enc_type)) < 0) {
		cls_error("can't set enc to %s, error %d", enc_type, ret);
		goto respond;
	}

	if ((ret = clsapi_SSID_set_authentication_mode(ifname, ssid_str, "PSKAuthentication")) < 0) {
		cls_error("can't set PSK authentication, error %d", ret);
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_PMF, pmf_type, sizeof(pmf_type)) > 0) {
		int pmf_cap = -1;
		/* pmf_cap values according to wpa_supplicant manual:
			0 = disabled (default unless changed with the global pmf parameter)
			1 = optional
			2 = required
		*/

		if (strcasecmp(pmf_type, "Required") == 0
			|| strcasecmp(pmf_type, "Forced_Required") == 0) {
			pmf_cap = 2;
		} else if (strcasecmp(pmf_type, "Optional") == 0) {
			pmf_cap = 1;
		} else if (strcasecmp(pmf_type, "Disable") == 0
			|| strcasecmp(pmf_type, "Forced_Disabled") == 0) {
			pmf_cap = 0;
		}

		if (pmf_cap != -1 && (ret = clsapi_SSID_set_pmf(ifname, ssid_str, pmf_cap)) < 0) {
			cls_error("can't set pmf to %d, error %d, ssid %s", pmf_cap, ret, ssid_str);
			goto respond;
		}

		if (pmf_cap > 0 && (ret = clsapi_SSID_set_authentication_mode(
			ifname, ssid_str, "SHA256PSKAuthenticationMixed")) < 0) {
			cls_error("can't set authentication for PMF, error %d, ssid %s",
					ret, ssid_str);
			goto respond;
		}
	}

	/* possible values for key_type: wpa/wpa2/wpa-psk/wpa2-psk/wpa2-ft/wpa2-wpa-psk */
	const int is_psk = strcasecmp(key_type, "wpa-psk") == 0 ||
				strcasecmp(key_type, "wpa2-psk") == 0 ||
				strcasecmp(key_type, "wpa2-wpa-psk") == 0;

	if (is_psk && (ret = clsapi_SSID_set_pre_shared_key(ifname, ssid_str, 0, pass_str)) < 0) {
		cls_error("can't set psk: ifname %s, ssid %s, key_type %s, pass %s, error %d",
			ifname, ssid_str, key_type, pass_str, ret);
	} else if (!is_psk &&
			(ret = clsapi_SSID_set_key_passphrase(ifname, ssid_str, 0, pass_str)) < 0) {
		cls_error("can't set pass: ifname %s, ssid %s, key_type %s, pass %s, error %d",
			ifname, ssid_str, key_type, pass_str, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_eaptls(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret;
	char ifname[IFNAMSIZ];
	char ssid_str[64] = {0};
	char key_type[64] = {0};
	char enc_type[64] = {0};
	char user_name[64] = {0};
	char ca_cert[64] = {0};
	char client_cert[64] = {0};
	struct clsapi_set_parameters set_params;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
		return;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0)
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());

	if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid_str, sizeof(ssid_str)) <= 0) {
		cls_error("can't get ssid");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_USERNAME, user_name, sizeof(user_name)) <= 0) {
		cls_error("can't get username");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_TRUSTEDROOTCA, ca_cert, sizeof(ca_cert)) <= 0) {
		cls_error("can't get trustedRootCA");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_CLIENTCERTIFICATE, client_cert,
						sizeof(client_cert)) <= 0) {
		cls_error("can't get clientCertificate");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_ENCPTYPE, enc_type, sizeof(enc_type)) <= 0) {
		cls_error("can't get enc_type");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_KEYMGMTTYPE, key_type, sizeof(key_type)) <= 0) {
		cls_error("can't get pass key_type");
		goto respond;
	}

	if (clsapi_SSID_verify_SSID(ifname, ssid_str) < 0) {
		ret = clsapi_SSID_create_SSID(ifname, ssid_str);
		if (ret < 0) {
			cls_error("can't create SSID %s, error %d", ssid_str, ret);
			goto respond;
		}
	}

	ret = set_sta_protocol(ifname, ssid_str, key_type);
	if (ret < 0) {
		cls_error("can't set protocol for ssid %s, error %d", ssid_str, ret);
		goto respond;
	}

	ret = set_sta_keymgmt(ifname, ssid_str, "WPA-EAP");
	if (strcmp(enc_type, "") != 0) {
		ret = set_sta_encryption(ifname, ssid_str, enc_type);
		if (ret < 0) {
			cls_error("can't set enc to %s, error %d", enc_type, ret);
			goto respond;
		}
	}

	memset(&set_params, 0, sizeof(set_params));

	strncpy(set_params.param[0].key, "eap", sizeof(set_params.param[0].key) - 1);
	strncpy(set_params.param[0].value, "TLS", sizeof(set_params.param[0].value) - 1);
	ret = clsapi_set_params(ifname, ssid_str, &set_params);
	if (ret < 0) {
		cls_error("can't set eap error %d", ret);
		goto respond;
	}

	if (strcmp(user_name, "") != 0) {
		memset(&set_params, 0, sizeof(set_params));
		strncpy(set_params.param[0].key, "identity",
					sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, user_name,
					sizeof(set_params.param[0].value) - 1);
		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set identity %s", user_name);
			goto respond;
		}
	}

	if (strcmp(ca_cert, "") != 0) {
		char full_cert_path[64] = {0};

		memset(&set_params, 0, sizeof(set_params));
		strcpy(full_cert_path, CERT_ROOT_PATH);
		strcat(full_cert_path, ca_cert);

		strncpy(set_params.param[0].key, "ca_cert",
				sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, full_cert_path,
				sizeof(set_params.param[0].value) - 1);
		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set ca_cert %s", ca_cert);
			goto respond;
		}
	}

	if (strcmp(client_cert, "") != 0) {
		char full_clt_cert[64] = {0};

		strcpy(full_clt_cert, CERT_ROOT_PATH);
		strcat(full_clt_cert, client_cert);

		memset(&set_params, 0, sizeof(set_params));
		strncpy(set_params.param[0].key, "client_cert",
				sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, full_clt_cert,
				sizeof(set_params.param[0].value) - 1);
		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set clientcert %s", client_cert);
			goto respond;
		}

		memset(&set_params, 0, sizeof(set_params));
		strncpy(set_params.param[0].key, "private_key",
				sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, full_clt_cert,
				sizeof(set_params.param[0].value) - 1);
		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set private_key %s", client_cert);
			goto respond;
		}
	}

respond:
	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_associate(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname[IFNAMSIZ];
	char ssid_str[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid_str, sizeof(ssid_str)) <= 0) {
		cls_error("can't get ssid");
		status = STATUS_INVALID;
		goto respond;
	}

	if ((ret = clsapi_wifi_associate(ifname, ssid_str)) < 0) {
		cls_error("can't associate, ifname %s, ssid %s, error %d", ifname, ssid_str, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_encryption(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char ssid_str[128];
	char encryption[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid_str, sizeof(ssid_str)) <= 0) {
		cls_error("can't get ssid");
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_ENCPTYPE, encryption, sizeof(encryption)) <= 0) {
		cls_error("can't get encryption");
		status = STATUS_INVALID;
		goto respond;
	}

	status = STATUS_ERROR;

	if (strcasecmp(encryption, "wep") == 0) {
		cls_log("wep is not supported");
		ret = -EINVAL;
		goto respond;
	}

	if (clsapi_SSID_verify_SSID(ifname, ssid_str) < 0 &&
			(ret = clsapi_SSID_create_SSID(ifname, ssid_str)) < 0) {
		cls_error("can't create SSID %s, error %d", ssid_str, ret);
		goto respond;
	}

	if ((strcasecmp(encryption, "none") == 0) &&
			(ret = clsapi_SSID_set_authentication_mode(ifname, ssid_str, "NONE")) < 0) {
		cls_log("can't set authentication to %s, ssid %s error %d",
				encryption, ssid_str, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_security(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret;
	char ifname[IFNAMSIZ];
	char ssid_str[64] = {0};
	char type_str[64] = {0};
	char pass_str[128] = {0};
	char keymgmt_type[64] = {0};
	char enc_type[64] = {0};
	char ecc_grps[64] = {0};
	char user_name[64] = {0};
	char ca_cert[64] = {0};
	char cert_type[64] = {0};
	char client_cert[64] = {0};
	char pairwise_cipher[64] = {0};
	char group_cipher[64] = {0};
	char group_mgnt_cipher[64] = {0};
	struct clsapi_set_parameters set_params;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0)
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());

	if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid_str, sizeof(ssid_str)) <= 0) {
		cls_error("can't get ssid");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_TYPE, type_str, sizeof(type_str)) <= 0) {
		cls_error("can't get type");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_KEYMGMTTYPE, keymgmt_type,
				sizeof(keymgmt_type)) <= 0)
		cls_log("no keymgmt_type");

	if (cls_get_value_text(&cmd_req, CLS_TOK_ENCPTYPE, enc_type, sizeof(enc_type)) <= 0)
		cls_log("no enc_type");

	if (cls_get_value_text(&cmd_req, CLS_TOK_PASSPHRASE, pass_str, sizeof(pass_str)) <= 0)
		cls_log("no pass_phrase");

	if (cls_get_value_text(&cmd_req, CLS_TOK_ECGROUPID, ecc_grps, sizeof(ecc_grps)) <= 0)
		cls_log("no ecc group");

	if (cls_get_value_text(&cmd_req, CLS_TOK_USERNAME, user_name, sizeof(user_name)) <= 0)
		cls_log("no identity");

	if (cls_get_value_text(&cmd_req, CLS_TOK_TRUSTEDROOTCA, ca_cert, sizeof(ca_cert)) <= 0)
		cls_log("no ca cert");

	if (cls_get_value_text(&cmd_req, CLS_TOK_CERTTYPE, cert_type, sizeof(cert_type)) <= 0)
		cls_log("no Cert Type");

	if (cls_get_value_text(&cmd_req, CLS_TOK_CLIENTCERTIFICATE, client_cert,
				sizeof(client_cert)) <= 0)
		cls_log("no client Certificate");

	if (cls_get_value_text(&cmd_req, CLS_TOK_PAIRWISECIPHER, pairwise_cipher,
				sizeof(pairwise_cipher)) <= 0)
		cls_log("no Pairwise Cipher");

	if (cls_get_value_text(&cmd_req, CLS_TOK_GROUPCIPHER, group_cipher,
				sizeof(group_cipher)) <= 0)
		cls_log("no Group Cipher");

	if (cls_get_value_text(&cmd_req, CLS_TOK_GROUPMGNTCIPHER, group_mgnt_cipher,
				sizeof(group_mgnt_cipher)) <= 0)
		cls_log("no GroupMgntCipher");

	status = STATUS_ERROR;

	ret = clsapi_wifi_set_security_defer_mode(ifname, 1);
	if (ret < 0) {
		cls_error("can't enable security defer mode error %d",  ret);
		goto respond;
	}

	if (clsapi_SSID_verify_SSID(ifname, ssid_str) < 0) {
		ret = clsapi_SSID_create_SSID(ifname, ssid_str);
		if (ret < 0) {
			cls_error("can't create SSID %s, error %d", ssid_str, ret);
			goto respond;
		}
	}

	if (strcmp(pass_str, "") != 0) {
		ret = clsapi_SSID_set_key_passphrase(ifname, ssid_str, 0, pass_str);
		if (ret < 0) {
			cls_error("can't set passphrase: ifname %s, passphrase %s, error %d",
					ifname, pass_str, ret);
		}
	}

	if (strcmp(enc_type, "") != 0) {
		ret = set_sta_encryption(ifname, ssid_str, enc_type);
		if (ret < 0) {
			cls_error("can't set enc to %s, error %d", enc_type, ret);
			goto respond;
		}
	}

	if (strcmp(pairwise_cipher, "") != 0) {
		ret = set_sta_encryption(ifname, ssid_str, pairwise_cipher);
		if (ret < 0) {
			cls_error("can't set pairwise to %s, error %d", pairwise_cipher, ret);
			goto respond;
		}
	}

	if (strcmp(group_cipher, "") != 0) {
		ret = set_sta_group_encryption(ifname, ssid_str, group_cipher);
		if (ret < 0) {
			cls_error("can't set group to %s, error %d", group_cipher, ret);
			goto respond;
		}
	}

	if (strcmp(group_mgnt_cipher, "") != 0) {
		memset(&set_params, 0, sizeof(set_params));
		strncpy(set_params.param[0].key, "group_mgmt",
				sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, group_mgnt_cipher,
				sizeof(set_params.param[0].value) - 1);
		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set cmd.GroupMgntCipher to %s, error %d",
					group_mgnt_cipher, ret);
			goto respond;
		}
	}

	ret = set_sta_protocol(ifname, ssid_str, keymgmt_type);
	if (ret < 0) {
		cls_error("can't set protocol for ssid %s, error %d", ssid_str, ret);
		goto respond;
	}

	if (strcasecmp(type_str, "psk") == 0)
		ret = set_sta_keymgmt(ifname, ssid_str, "WPA-PSK");
	else if (strcasecmp(type_str, "eaptls") == 0) {
		memset(&set_params, 0, sizeof(set_params));

		strncpy(set_params.param[0].key, "eap", sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, "TLS", sizeof(set_params.param[0].value) - 1);
		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set eap to %s, error %d", type_str, ret);
			goto respond;
		}

		if (strcasecmp(keymgmt_type, "SuiteB") == 0)
			ret = set_sta_keymgmt(ifname, ssid_str, "WPA-EAP-SUITE-B-192");
		else if (strcasecmp(keymgmt_type, "WPA2") == 0) {
			ret = clsapi_SSID_set_pmf(ifname, ssid_str, clsapi_pmf_optional);
			if (ret < 0) {
				cls_error("can't set pmf, error %d, ssid %s", ret, ssid_str);
				goto respond;
			}
			ret = set_sta_keymgmt(ifname, ssid_str, "WPA-EAP");
		}
	} else
		ret = set_sta_keymgmt(ifname, ssid_str, type_str);
	if (ret < 0) {
		cls_error("can't set keymgmt to %s, error %d", type_str, ret);
		goto respond;
	}

	if ((strcmp(ecc_grps, "") != 0) &&
			((strcasecmp(type_str, "owe") == 0) ||
			 (strcasecmp(type_str, "sae") == 0))) {
		int i;

		memset(&set_params, 0, sizeof(set_params));

		if (strcasecmp(type_str, "sae") == 0)
			strncpy(set_params.param[0].key, "sae_groups",
						sizeof(set_params.param[0].key) - 1);
		else if (strcasecmp(type_str, "owe") == 0)
			strncpy(set_params.param[0].key, "owe_group",
						sizeof(set_params.param[0].key) - 1);

		strncpy(set_params.param[0].value, ecc_grps,
					sizeof(set_params.param[0].value) - 1);
		for (i = 0; i < sizeof(set_params.param[0].value); i++) {
			if (set_params.param[0].value[i] == ' ')
				set_params.param[0].value[i] = ',';
		}

		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set ecc groups %s for keymgmt %s", ecc_grps, type_str);
			goto respond;
		}
	}

	if (strcasecmp(type_str, "owe") == 0) {
		memset(&set_params, 0, sizeof(set_params));

		strncpy(set_params.param[0].key, "auth_alg", sizeof(set_params.param[0].key) - 1);
		strncpy(set_params.param[0].value, "OPEN", sizeof(set_params.param[0].value) - 1);

		ret = clsapi_set_params(ifname, ssid_str, &set_params);
		if (ret < 0) {
			cls_error("can't set auth_alg for owe, ret %d", ret);
			goto respond;
		}
	}

	if ((strcasecmp(type_str, "sae") == 0) || (strcasecmp(type_str, "owe") == 0) ||
		(strcasecmp(keymgmt_type, "SuiteB") == 0)) {
		cls_log("forcing pmf as required");
		ret = clsapi_SSID_set_pmf(ifname, ssid_str, clsapi_pmf_required);
		if (ret < 0) {
			cls_error("can't set pmf, error %d, ssid %s", ret, ssid_str);
			goto respond;
		}
	}

	if (strcasecmp(type_str, "psk-sae") == 0) {
		cls_log("Set pmf as optional for transition mode ");
		ret = clsapi_SSID_set_pmf(ifname, ssid_str, clsapi_pmf_optional);
		if (ret < 0) {
			cls_error("can't set pmf, error %d, ssid %s", ret, ssid_str);
			goto respond;
		}
	}

	if (strcasecmp(type_str, "eaptls") == 0) {
		if (strcmp(user_name, "") != 0) {
			memset(&set_params, 0, sizeof(set_params));

			strncpy(set_params.param[0].key, "identity",
					sizeof(set_params.param[0].key) - 1);
			strncpy(set_params.param[0].value, user_name,
				sizeof(set_params.param[0].value) - 1);
			ret = clsapi_set_params(ifname, ssid_str, &set_params);
			if (ret < 0) {
				cls_error("can't set identity %s for %s", user_name, type_str);
				goto respond;
			}
		}

		if (strcmp(cert_type, "") != 0) {
			ret = set_sta_openssl_ciphers(ifname, ssid_str, cert_type);
			if (ret < 0) {
				cls_error("can't set openssl_ciphers to %s, error %d",
						keymgmt_type, ret);
				goto respond;
			}
		}

		if (strcmp(ca_cert, "") != 0) {
			char full_cert_path[64] = {0};

			memset(&set_params, 0, sizeof(set_params));
			strcpy(full_cert_path, CERT_ROOT_PATH);
			strcat(full_cert_path, ca_cert);

			strncpy(set_params.param[0].key, "ca_cert",
					sizeof(set_params.param[0].key) - 1);
			strncpy(set_params.param[0].value, full_cert_path,
					sizeof(set_params.param[0].value) - 1);
			ret = clsapi_set_params(ifname, ssid_str, &set_params);
			if (ret < 0) {
				cls_error("can't set ca_cert %s for %s", ca_cert, type_str);
				goto respond;
			}
		}

		if (strcmp(client_cert, "") != 0) {
			char full_clt_cert[64] = {0};

			strcpy(full_clt_cert, CERT_ROOT_PATH);
			strcat(full_clt_cert, client_cert);

			memset(&set_params, 0, sizeof(set_params));
			strncpy(set_params.param[0].key, "client_cert",
					sizeof(set_params.param[0].key) - 1);
			strncpy(set_params.param[0].value, full_clt_cert,
					sizeof(set_params.param[0].value) - 1);
			ret = clsapi_set_params(ifname, ssid_str, &set_params);
			if (ret < 0) {
				cls_error("can't set clientcert %s for %s", client_cert, type_str);
				goto respond;
			}

			memset(&set_params, 0, sizeof(set_params));
			strncpy(set_params.param[0].key, "private_key",
					sizeof(set_params.param[0].key) - 1);
			strncpy(set_params.param[0].value, full_clt_cert,
					sizeof(set_params.param[0].value) - 1);
			ret = clsapi_set_params(ifname, ssid_str, &set_params);
			if (ret < 0) {
				cls_error("can't set private_key %s for %s", client_cert, type_str);
				goto respond;
			}
		}
	}

	ret = clsapi_wifi_set_security_defer_mode(ifname, 0);
	if (ret < 0) {
		cls_error("can't disable security defer mode error %d",  ret);
		goto respond;
	}
	ret = clsapi_wifi_apply_security_config(ifname);
	if (ret < 0) {
		cls_error("can't apply security config error %d",  ret);
		goto respond;
	}
	status = STATUS_COMPLETE;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

static
int cls_send_vht_opmode_action(const char* ifname, const unsigned char *dest_mac, int cbw, int nss)
{
	struct iwreq iwr;
	unsigned char frame_buf[64];
	struct app_action_frame_buf *action_frm = (struct app_action_frame_buf*)frame_buf;
	int ioctl_sock;
	uint8_t chwidth;
	uint8_t rxnss;
	uint8_t rxnss_type = 0;
	uint8_t vhtop_notif_mode;
	int ret;

	switch (cbw) {
	case 20:
		chwidth = IEEE80211_CWM_WIDTH20;
		break;
	case 40:
		chwidth = IEEE80211_CWM_WIDTH40;
		break;
	case 80:
		chwidth = IEEE80211_CWM_WIDTH80;
		break;
	default:
		return -EINVAL;
	}

	if ((nss < 1) || (nss > IEEE80211_AC_MCS_NSS_MAX)) {
		return -EINVAL;
	}

	rxnss = nss - 1;

	vhtop_notif_mode = SM(chwidth, IEEE80211_VHT_OPMODE_CHWIDTH) |
			SM(rxnss, IEEE80211_VHT_OPMODE_RXNSS) |
			SM(rxnss_type, IEEE80211_VHT_OPMODE_RXNSS_TYPE);

	ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);

	if (ioctl_sock < 0)
		return -errno;

	action_frm->cat = IEEE80211_ACTION_CAT_VHT;
	action_frm->action = IEEE80211_ACTION_VHT_OPMODE_NOTIFICATION;
	memcpy(action_frm->dst_mac_addr, dest_mac, IEEE80211_ADDR_LEN);
	action_frm->frm_payload.length = 1;
	action_frm->frm_payload.data[0] = vhtop_notif_mode;

	/* send action frame */
	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ - 1);

	iwr.u.data.flags = SIOCDEV_SUBIO_SEND_ACTION_FRAME;
	iwr.u.data.pointer = action_frm;
	iwr.u.data.length = sizeof(struct app_action_frame_buf) + action_frm->frm_payload.length;

	ret = ioctl(ioctl_sock, IEEE80211_IOCTL_EXT, &iwr);
	if (ret < 0) {
		cls_error("failed to send action frame");
	}

	close(ioctl_sock);

	return ret;
}


#ifdef QDOCK2
static int cls_mbo_send_bcnrep_request(struct cls_bcnreport_req *bcnreq_param)
{
#define IS_EMPTY_PARAM(_param) (memcmp(_param, "\x00", 1) == 0)
	int i, ret = 0;
	char cmd[CLS_DEFCONF_CMDBUF_LEN] = {0};

	if (!IS_EMPTY_PARAM(bcnreq_param->chans)) {
		for (i = 0; i < strlen(bcnreq_param->chans); i++) {
			if (bcnreq_param->chans[i] == '_')
				bcnreq_param->chans[i] = ',';
		}
	}

	if (!IS_EMPTY_PARAM(bcnreq_param->reqinfo)) {
		for (i = 0; i < strlen(bcnreq_param->reqinfo); i++) {
			if (bcnreq_param->reqinfo[i] == '_')
				bcnreq_param->reqinfo[i] = ',';
		}
	}

	BUILD_MBO_CMD_HEAD("beacon_req");
	APPEND_CMD(cmd, sizeof(cmd), ", \"params\":");
	APPEND_CMD(cmd, sizeof(cmd), "{\"mac\":\"%s\", ", bcnreq_param->dest_mac);
	APPEND_CMD(cmd, sizeof(cmd), "\"opclass\":%d, ", bcnreq_param->opclass);
	APPEND_CMD(cmd, sizeof(cmd), "\"chan\":%d, ", bcnreq_param->chan);
	APPEND_CMD(cmd, sizeof(cmd), "\"interval\":%d, ", bcnreq_param->interval);
	APPEND_CMD(cmd, sizeof(cmd), "\"duration\":%d, ", bcnreq_param->duration);
	APPEND_CMD(cmd, sizeof(cmd), "\"mode\":\"%s\", ", bcnreq_param->mode);
	APPEND_CMD(cmd, sizeof(cmd), "\"detail\":%d, ", bcnreq_param->detail);
	if (!IS_EMPTY_PARAM(bcnreq_param->ssid)) {
		APPEND_CMD(cmd, sizeof(cmd), "\"ssid\":\"%s\", ",
				bcnreq_param->ssid+strlen("ssid="));
	}
	APPEND_CMD(cmd, sizeof(cmd), "\"bssid\":\"%s\", ", bcnreq_param->bssid);
	if (!IS_EMPTY_PARAM(bcnreq_param->chans)) {
		APPEND_CMD(cmd, sizeof(cmd), "\"chans\":\"%s\", ",
				bcnreq_param->chans+strlen("chans="));
	}
	if (!IS_EMPTY_PARAM(bcnreq_param->reqinfo)) {
		APPEND_CMD(cmd, sizeof(cmd), "\"requested\":\"%s\", ",
				bcnreq_param->reqinfo+strlen("info="));
	}
	APPEND_CMD(cmd, sizeof(cmd), "\"is_last\":%d}}\' ",
			bcnreq_param->last_beacon_rpt_ind);

	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to send bcnrep request");

	cls_log("send cmd: %s\n", cmd);
	return ret;
}

static int cls_mbo_del_neighbor(char *bssid)
{
	char cmd[CLS_DEFCONF_CMDBUF_LEN] = {0};
	int ret;

	BUILD_MBO_CMD_HEAD("del_neigh");
	APPEND_CMD(cmd, sizeof(cmd), ", \"params\":");
	APPEND_CMD(cmd, sizeof(cmd), "{\"mac\":\"%s\"}}\'", bssid);

	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to del neighbor");

	cls_log("send cmd: %s\n", cmd);
	return ret;
}

static int cls_mbo_send_btm_request(const char *ifname, char *dest_mac, int cand_list)
{
	char str[256] = {0}, cmd[CLS_DEFCONF_CMDBUF_LEN] = {0};
	unsigned long val = 0;
	FILE *filep = NULL;
	int ret = 0;

	snprintf(str, sizeof(str), "iwpriv %s get_btm_delay", ifname);
	filep = popen(str, "r");
	if (!filep) {
		cls_error("failed: %s", str);
		return -EINVAL;
	}

	if (fgets(str, 128, filep) != NULL)
		cls_log("iwpriv get btm delay value[%s]", str);

	pclose(filep);

	if (str[0] != '\0') {
		char *token = strtok(str, ":");

		while (token) {
			val = strtoul(token, NULL, 10);
			token = strtok(NULL, ":");
		}
	}

	if (cls_mbo_get_disassoc_imm()) {
		char mstr[64];
		unsigned char addr[IEEE80211_ADDR_LEN];

		if (clsapi_interface_get_mac_addr(ifname, addr) < 0) {
			cls_error("can't get macaddr failed");
			return -EINVAL;
		}

		snprintf(mstr, sizeof(mstr), "%02x:%02x:%02x:%02x:%02x:%02x",
				addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

		ret = cls_mbo_del_neighbor(mstr);
		if (ret < 0)
			cls_error("failed to del neighbor %s", mstr);
	}

	if (val == 5) {
		APPEND_CMD(cmd, sizeof(cmd), "ifconfig %s down", ifname);
	} else {
		BUILD_MBO_CMD_HEAD("btm_req");
		APPEND_CMD(cmd, sizeof(cmd), ", \"params\":");
		APPEND_CMD(cmd, sizeof(cmd), "{\"mac\":\"%s\", ", dest_mac);
		APPEND_CMD(cmd, sizeof(cmd), "\"candidates\":%d, ", cand_list);
		APPEND_CMD(cmd, sizeof(cmd), "\"disassoc_imminent\":%d}}\'",
				cls_mbo_get_disassoc_imm());
	}
	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to send btm request");

	cls_log("send cmd: %s\n", cmd);
	return ret;
}
#else
static
int cls_send_bcnrep_request(const char *ifname, struct cls_bcnreport_req *bcnreq_param)
{
#define IS_EMPTY_PARAM(_param) (memcmp(_param, "\x00", 1) == 0)
	int i, ret = 0;
	char cmd[256];

	if (access(CLS_MBO_TEST_CLI, X_OK) != 0) {
		cls_error("failed to send bcnrep request: %s can't access", CLS_MBO_TEST_CLI);
		return -EINVAL;
	}

	if (!IS_EMPTY_PARAM(bcnreq_param->chans)) {
		for (i = 0; i < strlen(bcnreq_param->chans); ++i) {
			if (bcnreq_param->chans[i] == '_')
				bcnreq_param->chans[i] = ',';
		}
	}

	if (!IS_EMPTY_PARAM(bcnreq_param->reqinfo)) {
		for (i = 0; i < strlen(bcnreq_param->reqinfo); ++i) {
			if (bcnreq_param->reqinfo[i] == '_')
				bcnreq_param->reqinfo[i] = ',';
		}
	}

#pragma GCC diagnostic push
#if GCC_DIAGNOSTIC_AWARE
#	pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
	ret = snprintf(cmd, sizeof(cmd),
		"%s test beacon_req %s opclass=%d chan=%d bssid=%s interval=%d duration=%d mode=%s detail=%d %s %s %s",
		CLS_MBO_TEST_CLI, bcnreq_param->dest_mac, bcnreq_param->opclass, bcnreq_param->chan,
		bcnreq_param->bssid, bcnreq_param->interval, bcnreq_param->duration,
		bcnreq_param->mode, bcnreq_param->detail,
		IS_EMPTY_PARAM(bcnreq_param->ssid) ? "" : bcnreq_param->ssid,
		IS_EMPTY_PARAM(bcnreq_param->chans) ? "" : bcnreq_param->chans,
		IS_EMPTY_PARAM(bcnreq_param->reqinfo) ? "" : bcnreq_param->reqinfo);
#pragma GCC diagnostic pop

	if (bcnreq_param->last_beacon_rpt_ind)
		strcat(cmd, " last_bcn=1");

	cls_log("MBO test cmd[%s]", cmd);

	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to send bcnrep request: system command error");

	return ret;
}

static int cls_send_btm_request(const char *ifname, char *dest_mac, int cand_list)
{
	char str[256], cmd[128];
	unsigned long val = 0;
	FILE *filep = NULL;
	uint8_t auto_candidate = cls_dut_get_mbo_auto_candidate_flag();
	int ret = 0;

	if (access(CLS_MBO_TEST_CLI, X_OK) != 0) {
		cls_error("failed to send BTM request: %s can't access", CLS_MBO_TEST_CLI);
		return -EINVAL;
	}

	snprintf(str, sizeof(str), "iwpriv %s get_btm_delay", ifname);
	filep = popen(str, "r");
	if (!filep) {
		cls_error("failed: %s", str);
		return -EINVAL;
	} else {
		if (fgets(str, 128, filep) != NULL)
			cls_log("iwpriv get btm delay value[%s]", str);
		pclose(filep);
	}

	if (str[0] != '\0') {
		char *token = strtok(str, ":");
		while (token) {
			val = strtoul(token, NULL, 10);
			token = strtok(NULL, ":");
		}
	}

	if (val == 5)
		ret = snprintf(cmd, sizeof(cmd), "ifconfig %s down", ifname);
	else if (auto_candidate == 1)
		ret = snprintf(cmd, sizeof(cmd), "%s test actuate 11v %s",
			CLS_MBO_TEST_CLI, dest_mac);
	else	/* manual populates candidate list */
		ret = snprintf(cmd, sizeof(cmd), "%s test wfa_btmreq %s",
			CLS_MBO_TEST_CLI, dest_mac);

	cls_log("MBO test cmd[%s %d]", cmd, cand_list);

	ret = system(cmd);
	if (ret != 0)
		cls_error("failed to send btm request: system command error");

	return ret;
}
#endif

static int local_check_mgmt_frame_type(const char *frame_name, unsigned int *mgmt_type)
{
	if (!frame_name)
		return -1;

	if (strcasecmp(frame_name, "disassoc") == 0)
		*mgmt_type = CLS_PKTGEN_TYPE_MGMT_DISASSOC;
	else if (strcasecmp(frame_name, "deauth") == 0)
		*mgmt_type = CLS_PKTGEN_TYPE_MGMT_DEAUTH;
	else
		return -1;

	return 0;
}


void cls_handle_dev_configure_ie(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char ifname_buf[IFNAMSIZ];
	const char *ifname;
	char ie_name[16];
	char contents[128];
	enum cls_dut_band_index band;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	*ifname_buf = 0;
	ret = cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname_buf, sizeof(ifname_buf));
	ifname = (ret > 0) ? ifname_buf : cls_get_sigma_interface();

	ifname = cls_get_sigma_interface_name_from_cmd(ifname, 0);
	band = cls_get_sigma_interface_band_idx_from_cmd(ifname);

	status = STATUS_ERROR;

	ret = cls_get_value_text(&cmd_req, CLS_TOK_IE_NAME, ie_name, sizeof(ie_name));
	if (ret <= 0) {
		/* mandatory parameter */
		cls_error("can't get IE_Name");
		status = STATUS_ERROR;
		goto respond;
	}

	ret = cls_get_value_text(&cmd_req, CLS_TOK_CONTENTS, contents, sizeof(contents));
	if (ret <= 0) {
		/* mandatory parameter */
		cls_error("can't get contents");
		status = STATUS_ERROR;
		goto respond;
	}

	ret = clsapi_add_app_ie(ifname, IEEE80211_APPIE_FRAME_BEACON, 1, contents);
	if (ret < 0) {
		cls_error("can't add ie to beacon, err: %d", ret);
		status = STATUS_ERROR;
		goto respond;
	}

	ret = clsapi_add_app_ie(ifname, IEEE80211_APPIE_FRAME_PROBE_RESP, 1, contents);
	if (ret < 0) {
		cls_error("can't add ie to probe response, err: %d", ret);
		status = STATUS_ERROR;
		goto respond;
	}

	cls_dut_set_configure_ie(band);

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

#define MAC_ADDR_STR_LEN 18

void cls_handle_sta_reassoc(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char bssid[64];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_BSSID, bssid, sizeof(bssid)) <= 0) {
		cls_error("can't get bssid");
		goto respond;
	}

	if ((ret = clsapi_wifi_reassociate(ifname)) < 0) {
		cls_error("can't reassociate, error %d", ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_systime(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char cmd[128];
	int month;
	int date;
	int year;
	int hours;
	int minutes;
	int seconds;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_YEAR, &year) <= 0) {
		cls_error("can't get year");
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MONTH, &month) <= 0) {
		cls_error("can't get month");
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_DATE, &date) <= 0) {
		cls_error("can't get date");
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_HOURS, &hours) <= 0) {
		cls_error("can't get hours");
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MINUTES, &minutes) <= 0) {
		cls_error("can't get minutes");
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_SECONDS, &seconds) <= 0) {
		cls_error("can't get seconds");
		goto respond;
	}

	snprintf(cmd, sizeof(cmd), "date -s %2.2d%2.2d%2.2d%2.2d%4.4d.%2.2d",
		month, date, hours, minutes, year, seconds);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("can't set time. error %d, cmd %s", ret, cmd);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_radio(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char mode[64];
	int rf_enable_timeout_sec = 5;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_MODE, mode, sizeof(mode)) <= 0) {
		cls_error("can't get mode");
		goto respond;
	}

	if ((ret = cls_set_rf_enable_timeout(strcasecmp(mode, "On") == 0 ? 1 : 0, rf_enable_timeout_sec)) < 0) {
		cls_error("can't set rf to %s, error %d", mode, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_macaddr(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char mac_str[64];
	clsapi_mac_addr mac;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		cls_error("can't get ifname");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_MAC, mac_str, sizeof(mac_str)) <= 0) {
		cls_error("can't get mac");
		goto respond;
	}

	if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
		cls_error("can't parse mac_str %s", mac_str);
		goto respond;
	}

	cls_log("try to set mac on %s to %s", ifname, mac_str);

	if ((ret = clsapi_interface_set_mac_addr(ifname, mac)) < 0) {
		cls_error("can't set mac to %s, error %d", mac_str, ret);
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_uapsd(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char cmd[128];
	int maxsplength;
	int acbe;
	int acbk;
	int acvi;
	int acvo;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		cls_error("can't get ifname");
		goto respond;
	}

	if (cls_get_value_int(&cmd_req, CLS_TOK_MAXSPLENGTH, &maxsplength) <= 0)
		maxsplength = 4;

	if (cls_get_value_int(&cmd_req, CLS_TOK_ACBE, &acbe) <= 0)
		acbe = 1;

	if (cls_get_value_int(&cmd_req, CLS_TOK_ACBK, &acbk) <= 0)
		acbk = 1;

	if (cls_get_value_int(&cmd_req, CLS_TOK_ACVI, &acvi) <= 0)
		acvi = 1;

	if (cls_get_value_int(&cmd_req, CLS_TOK_ACVO, &acvo) <= 0)
		acvo = 1;

	uint8_t uapsdinfo = WME_CAPINFO_UAPSD_EN;
	if (acbe) {
		uapsdinfo |= WME_CAPINFO_UAPSD_BE;
	}

	if (acbk) {
		uapsdinfo |= WME_CAPINFO_UAPSD_BK;
	}

	if (acvi) {
		uapsdinfo |= WME_CAPINFO_UAPSD_VI;
	}

	if (acvo) {
		uapsdinfo |= WME_CAPINFO_UAPSD_VO;
	}

	uapsdinfo |= (maxsplength & WME_CAPINFO_UAPSD_MAXSP_MASK) << WME_CAPINFO_UAPSD_MAXSP_SHIFT;

	snprintf(cmd, sizeof(cmd), "iwpriv %s setparam %d %d",
			ifname, IEEE80211_PARAM_UAPSDINFO, uapsdinfo);
	ret = system(cmd);

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_reset_parm(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char arp[64];
	char cmd[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		cls_error("can't get ifname");
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_ARP, arp, sizeof(arp)) <= 0) {
		cls_error("can't get arp");
		goto respond;
	}

	if (strcasecmp(arp, "all") == 0) {
		snprintf(cmd, sizeof(cmd), "for ip in `grep %s /proc/net/arp | awk '{print $1}'`; "
				"do arp -i %s -d $ip; done", ifname, ifname);
	} else {
		snprintf(cmd, sizeof(cmd), "arp -i %s -d %s", ifname, arp);
	}

	ret = system(cmd);
	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_11n(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char width_str[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		cls_error("can't get ifname");
		goto respond;
	}

	status = STATUS_ERROR;

	if (cls_get_value_text(&cmd_req, CLS_TOK_WIDTH, width_str, sizeof(width_str)) <= 0) {
		clsapi_unsigned_int bw;
		if (strcasecmp(width_str, "auto") == 0) {
			bw = 40;
		} else {
			sscanf(width_str, "%u", &bw);
		}

		if ((ret = clsapi_wifi_set_bw(ifname, bw)) < 0) {
			cls_error("can't set bw to %d, error %d", bw, ret);
			goto respond;
		}
	}

	int tx_ss;
	int rx_ss;

	if (cls_get_value_int(&cmd_req, CLS_TOK_TXSP_STREAM, &tx_ss) <= 0)
		tx_ss = -1;

	if (cls_get_value_int(&cmd_req, CLS_TOK_RXSP_STREAM, &rx_ss) <= 0)
		rx_ss = -1;

	if (tx_ss == rx_ss && tx_ss != -1) {
		/* sta_set_11n is used only for 11n, so hardcode clsapi_mimo_ht */
		ret = cls_set_nss_cap(ifname, clsapi_mimo_ht, tx_ss);
		if (ret < 0) {
			cls_error("can't set NSS to %d, error %d", tx_ss, ret);
		}
	} else if (tx_ss != -1 || rx_ss != -1) {
		cls_error("can't handle number of SS separatly for RX and TX");
		ret = -EINVAL;
	}

	status = ret >= 0 ? STATUS_COMPLETE : STATUS_ERROR;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_power_save(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char val_str[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		cls_error("can't get ifname");
		goto respond;
	}

	status = STATUS_ERROR;

	if (cls_get_value_text(&cmd_req, CLS_TOK_POWERSAVE, val_str, sizeof(val_str)) > 0) {
		if (strcasecmp(val_str, "off") == 0) {
			// power save does not exist by default
			ret = 0;
		} else {
			cls_error("can't set power save to %s since poser save is not supported",
				val_str);
			ret = -EOPNOTSUPP;
			goto respond;
		}
	}

	status = STATUS_COMPLETE;
respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_sta_set_sleep(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char cmd[128];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());

	}

	status = STATUS_ERROR;

	snprintf(cmd, sizeof(cmd), "iwpriv %s sleep 0", ifname);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("can't set sleep, error %d", ret);
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);

}

void cls_handle_sta_scan(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0) {
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());

	}

	status = STATUS_ERROR;

	ret = clsapi_wifi_start_scan(ifname);
	if (ret < 0) {
		cls_error("can't start active scan on %s, error %d", ifname, ret);
		status = STATUS_ERROR;
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_device_get_info(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret;
	char info_buf[256] = {0};
	string_64 hw_version = {0};
	static const char vendor[] = "Clourneysemi";
	FILE *model;

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

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

	/* TODO: use hard-coded hw_version for now, fix it when new HW version is available */
	ret = snprintf(info_buf, sizeof(info_buf),
			"vendor,%s,model,%s,version,%s", vendor, "DB-1000", CDRV_BLD_NAME);
	if (ret < 0) {
		status = STATUS_ERROR;
		goto respond;
	}

	status = STATUS_COMPLETE;

respond:
	cls_dut_make_response_vendor_info(cmd_tag, status, ret, info_buf, out_len, out);
}

void cls_handle_dev_exec_action(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_INVALID;
	int ret = 0;
	char ifname[IFNAMSIZ];
	char program[16] = {0};
	int resp_len = 0;
	char *resp = NULL;

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_get_value_text(&cmd_req, CLS_TOK_INTERFACE, ifname, sizeof(ifname)) <= 0)
		snprintf(ifname, sizeof(ifname), "%s", cls_get_sigma_interface());

	ret = cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, program, sizeof(program));
	if (ret <= 0) {
		/* mandatory parameter */
		cls_error("can't get program");
		status = STATUS_ERROR;
		goto respond;
	}

	resp = (char *)calloc(1, sizeof(char) * CLS_MAX_BUF_LEN);
	if (!resp) {
		cls_error("can't alloc resp string");
		status = STATUS_ERROR;
		goto respond;
	}

	if (strcasecmp(program, "DPP") == 0) {
		ret = cls_handle_dpp_dev_action(&cmd_req, ifname, resp, &resp_len);
		if (ret) {
			/* mandatory parameter */
			cls_error("failed to execute DPP dev action");
			status = STATUS_ERROR;
			goto respond;
		}
		status = STATUS_COMPLETE;
	} else {
		cls_error("dev_exec_action: no other program except DPP supported");
		status = STATUS_INVALID;
	}

respond:
	if (resp_len)
		cls_dut_make_response_str(cmd_tag, status, ret, resp, resp_len, out_len, out);
	else
		cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);

	if (resp)
		free(resp);
}

static int system_cli_call(const char *cli, char *cli_suffix, char *resp, int resp_len)
{
	FILE *fp;
	char buf[CLS_MAP_MAX_BUF], cmd[4*CLS_MAX_CMD_BUF+128];

	sprintf(cmd, "%s %s", cli, cli_suffix);
	cls_log("%s %s", __func__, cmd);

	resp[0] = 0;
	fp = popen(cmd, "r");
	if (fp) {
		while (fgets(buf, CLS_MAP_MAX_BUF, fp) != NULL) {
			if (strlen(resp) + strlen(buf) > resp_len)
				break;
			strcat(resp, buf);
		}
		pclose(fp);
		fp = NULL;
	} else {
		cls_error("faile to execute %s command, popen error", cmd);
		return -EINVAL;
	}
	return 0;
}

static int cls_get_macaddr_by_ssid(char *ifname, char *ssid,
	char *param, unsigned char *macaddr)
{
	int ret = -1;
	uint8_t intf_idx;
	clsapi_SSID curr_SSID;
	clsapi_wifi_mode mode = clsapi_nosuch_mode;

	cls_log("%s input ifname(%s), ssid(%s) param(%s)", __func__, ifname, ssid, param);

	for (intf_idx = 0; intf_idx < CLS_MAX_BSS; intf_idx++) {
		ifname[strlen(ifname)-1] = intf_idx + '0';
        mode = clsapi_nosuch_mode;
		clsapi_wifi_get_mode(ifname, &mode);
		if (mode != clsapi_access_point)
			continue;
		ret = clsapi_wifi_get_SSID(ifname, curr_SSID);
		if (ret == 0 && !strcmp(curr_SSID, ssid)) {
			if (!strcasecmp(param, "macaddr"))
				ret = clsapi_interface_get_mac_addr(ifname, macaddr);
			else
				ret = clsapi_wifi_get_BSSID(ifname, macaddr);
			goto end;
		}
	}

end:
	return ret;
}

static int cls_handle_def8021q_tlv(char *cmd, size_t cmd_len,
		struct cls_cmd_request *cmd_req, int is_npu)
{
	char tlv_type[6], tlv_length[8], tlv_value[CLS_MAX_TLV_BUF];
	char *next = NULL, *str;

	if (cls_get_value_text(cmd_req, CLS_TOK_TLV_TYPE1, tlv_type, sizeof(tlv_type)) <= 0)
		goto handle_out;

	if (cls_get_value_text(cmd_req, CLS_TOK_TLV_LENGTH1, tlv_length, sizeof(tlv_length)) <= 0)
		goto handle_out;

	if (cls_get_value_text(cmd_req, CLS_TOK_TLV_VALUE1, tlv_value, sizeof(tlv_value)) <= 0)
		goto handle_out;

#define TLV_TYPE_DEFAULT_8021Q_SETTINGS 0xB5
	if (strtoul(tlv_type, NULL, 0) != TLV_TYPE_DEFAULT_8021Q_SETTINGS)
		goto handle_out;

	str = tlv_value;
	if (*str == '{')
		str++;
	if (!is_npu) {
		unsigned int val;

		val = (unsigned int)strtoul(str, &next, 0);
		if (!next)
			goto handle_out;
		APPEND_CMD(cmd, cmd_len, "\"primary_vid\":%u, ", val);
		str = next;
		while (isspace(*str))
			str++;
		val = (unsigned int)strtoul(str, NULL, 0);
		APPEND_CMD(cmd, cmd_len, "\"default_pcp\":%u, ", val);
	}

	return 1;
handle_out:
	return 0;
}

static int cls_handle_ssid2vid_table(char *cmd, size_t cmd_len,
		struct cls_cmd_request *cmd_req, int is_npu)
{
	char tlv_type[6], tlv_length[8], tlv_value[CLS_MAX_TLV_BUF];
	char *next = NULL, *str;
	unsigned int count = 1;
	unsigned int i;
	unsigned int ssid_len;
	char ssid[32] = {0};
	unsigned int vid;

	if (cls_get_value_text(cmd_req, CLS_TOK_TLV_TYPE2, tlv_type, sizeof(tlv_type)) <= 0)
		goto handle_out;

	if (cls_get_value_text(cmd_req, CLS_TOK_TLV_LENGTH2, tlv_length, sizeof(tlv_length)) <= 0)
		goto handle_out;

	if (cls_get_value_text(cmd_req, CLS_TOK_TLV_VALUE2, tlv_value, sizeof(tlv_value)) <= 0)
		goto handle_out;

#define TLV_TYPE_TRAFFIC_SEPARATION_POLICY 0xB6
	if (strtoul(tlv_type, NULL, 0) != TLV_TYPE_TRAFFIC_SEPARATION_POLICY)
		goto handle_out;

	str = tlv_value;
	if (*str == '{')
		str++;
	count = (unsigned int)strtoul(str, &next, 0);
	str = next;

	if (count <= 0)
		goto handle_out;

	for (i = 0; i < count; i++) {
		while (*str == '{' || isspace(*str))
			str++;
		ssid_len = (unsigned int)strtoul(str, &next, 0);
		str = next;
		while (isspace(*str))
			str++;
		memcpy(ssid, str, ssid_len);
		str += ssid_len;
		while (isspace(*str))
			str++;
		vid = (unsigned int)strtoul(str, &next, 0);
		str = next;
		while (*str == '}' || isspace(*str))
			str++;
		if (!is_npu) {
			APPEND_CMD(cmd, cmd_len, "{\"ssid\":\"%s\", ", ssid);
			APPEND_CMD(cmd, cmd_len, "\"vid\":%u},", vid);
		}
	}

	return 1;
handle_out:
	return 0;
}

static int cls_handle_traf_sep_tlv(char *cmd, size_t cmd_len,
		struct cls_cmd_request *cmd_req, int is_npu)
{
	if (!is_npu)
		APPEND_CMD(cmd, cmd_len, "\"ssid2vid_map\":[");

	if (cls_handle_ssid2vid_table(cmd, cmd_len, cmd_req, is_npu) == 0)
		goto handle_out;

	if (cmd[strlen(cmd)-1] == ',')
		cmd[strlen(cmd)-1] = ']';

	return 1;
handle_out:
	return 0;
}

static void cls_update_wps_interface(char *ifname)
{
	unsigned char bssid[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char zero_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static char radio_name[IFNAMSIZ];
	unsigned int rd, radio_idx;
	int ret = 0, is_repeater = 0;

	ret = clsapi_get_radio_from_ifname(ifname, &radio_idx);
	if (ret < 0) {
		cls_error("can't get radio index for interface(%s)", ifname);
		return;
	}

	// check is there any sta interface connected
	for (rd = 0; rd < CLS_MAX_RADIO_ID; ++rd) {
		ret = clsapi_radio_get_primary_interface(rd, radio_name, sizeof(radio_name));
		if (ret < 0)
			continue;

		is_repeater = clsapi_radio_verify_repeater_mode(rd);
		if (is_repeater) {
			ret = clsapi_wifi_get_BSSID(radio_name, bssid);
			if (!ret && memcmp(bssid, zero_mac, 6)
					&& clsapi_radio_verify_repeater_mode(radio_idx)) {
				ifname[strlen(ifname) - 1] = '1';
				break;
			}
		}
	}
}

#if 0
void cls_handle_dev_set_rfeature(int cmd_tag, int len, unsigned char *params, int *out_len,
		unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status = STATUS_COMPLETE;
	int ret = 0;
	char type_str[64] = {0};
	char bssid_str[64] = {0};
	int assoc_disallow = 0;
	char cmd[CLS_MAP_MAX_BUF] = {0};
	char resp[256] = {0};


	cls_log("%s", __func__);

	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0) {
		status = STATUS_INVALID;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_TYPE, type_str, sizeof(type_str)) <= 0) {
		cls_error("can't get type");
		ret = -EINVAL;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_BSSID, bssid_str, sizeof(bssid_str)) <= 0) {
		cls_error("can't get bssid");
		ret = -EINVAL;
		goto respond;
	}

	if (cls_get_value_enable(&cmd_req, CLS_TOK_ASSOC_DISALLOW, &assoc_disallow, &ret) <= 0) {
		cls_error("can't get Assoc_Disallow");
		goto respond;
	}

	if (!strncasecmp(type_str, "MBO", 3)) {
		//FIXME: check BSSID and assoc_disallowd invalid
		//
		_mac_str_trim(bssid_str, ':');

		cls_log("set_assoc_status bssid(%s) disallow(%d)\n", bssid_str, assoc_disallow);
		snprintf(cmd, sizeof(cmd),
		"call qdock.map set_assoc_status \'{\"bssid_status\":[{\"bssid_s\":\"%s\", \"status\":%d}]}\'",
		bssid_str, !assoc_disallow);
		ret = system_cli_call(CLS_UBUS_TEST_CLI, cmd,
				resp, sizeof(resp));
	}

respond:
	if (status != STATUS_INVALID)
		status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_dev_reset_default(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	int status;
	int ret = 0;
	char cert_prog[16];
	char dev_role[16];
	char conf_type[16];
	char cmd[256+256];
	char reset[128] = {0};
	char ifname[IFNAMSIZ] = {0};

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	/* mandatory certification program, e.g. MAP */
	if (cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, cert_prog, sizeof(cert_prog)) <= 0) {
		ret = -EINVAL;
		goto respond;
	}
	if (cls_get_value_text(&cmd_req, CLS_TOK_DEV_ROLE, dev_role, sizeof(dev_role)) <= 0) {
		ret = -EINVAL;
		goto respond;
	}
	/* configuration type, e.g. DUT or Testbed */
	if (cls_get_value_text(&cmd_req, CLS_TOK_TYPE, conf_type, sizeof(conf_type)) <= 0) {
		/* not specified */
		*conf_type = 0;
	}

	if (!strcmp(cert_prog, "mapr5")) {
		ifname[0] = 0;
		for (enum cls_dut_band_index i = CLS_DUT_2P4G_BAND; i < CLS_DUT_BANDS_NUM; i++) {
			if(clsapi_radio_get_primary_interface(i, ifname, sizeof(ifname)) < 0)
				continue;

			if(ifname[0] == 0)
				break;
			snprintf(reset, sizeof(reset), "cls reset %s type dut program MBO", ifname);
			system(reset);
		}
	}

	if (strcasecmp(dev_role, "agent") == 0) {
		if (cls_dut_npu_cfg.npu_topology == 1) {
			snprintf(cmd, sizeof(cmd), "%s%s "
				"\'sh -c \"start_mapagent_npu restart >/dev/null 2>&1\" &\'",
				cls_dut_npu_cfg.ssh_cli, cls_dut_npu_cfg.br_ipaddr);
			ret = system(cmd);
		} else {
			system("killall clemshd");
			sleep(2);
			ret = system("/usr/sbin/clmeshd -m "MACFMT" -i eth1,eth2,eth3,eth4 -s &", MACARG(cls_dut_npu_cfg.al_macaddr));
		}
		if (ret < 0) {
			cls_error("start map agent failed, errcode %d", ret);
			goto respond;
		}
	} else if (strcasecmp(dev_role, "controller") == 0) {
		if (cls_dut_npu_cfg.npu_topology == 1) {
			snprintf(cmd, sizeof(cmd), "%s%s "
				"\'sh -c \"start_mapcontroller_npu restart >/dev/null 2>&1\" &\'",
				cls_dut_npu_cfg.ssh_cli, cls_dut_npu_cfg.br_ipaddr);
			ret = system(cmd);
		} else {
			system("killall clemshd");
			sleep(2);
			ret = system("/usr/sbin/clmeshd -m "MACFMT" -i eth1,eth2,eth3,eth4 -S &", MACARG(cls_dut_npu_cfg.al_macaddr));
		}
		if (ret < 0) {
			cls_error("start map controller failed, errcode %d", ret);
			goto respond;
		}
	}

respond:
	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_dev_get_parameter(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	char cert_prog[16];
	char parameter[16];
	char ruid[16];
	char ssid[33];
	char ifname[IFNAMSIZ] = "br0";
	unsigned char mac[ETH_ALEN];
	char buf[128];
	int status;
	int ret = 0;

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	/* mandatory certification program, e.g. MAP */
	if (cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, cert_prog, sizeof(cert_prog)) <= 0) {
		ret = -EINVAL;
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_PARAMETER, parameter, sizeof(parameter)) <= 0) {
		ret = -EINVAL;
		goto respond;
	}

	if (strcasecmp(parameter, "ALid") == 0) {
		if (cls_dut_npu_cfg.npu_topology == 1)
			memcpy(mac, cls_dut_npu_cfg.al_macaddr, ETH_ALEN);
		else
			ret = clsapi_interface_get_mac_addr(ifname, mac);
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_RUID, ruid, sizeof(ruid)) <= 0
		|| strlen(ruid) != 14) {
		cls_error("cannot get ruid, error");
		ret = -EINVAL;
		goto respond;
	}

	snprintf(buf, sizeof(buf), "%.2s:%.2s:%.2s:%.2s:%.2s:%.2s",
		ruid+2, ruid+4, ruid+6, ruid+8, ruid+10, ruid+12);
	if (ether_aton_r(buf, (struct ether_addr *)mac)) {
		uint8_t radio_idx;
		unsigned char if_mac[ETH_ALEN];
		/* find bSTA's ifname by ruid parameter */
		for (radio_idx = 0; radio_idx < CLS_MAX_RADIO_ID; radio_idx++) {
			char tmp_name[IFNAMSIZ];

			snprintf(tmp_name, sizeof(tmp_name), "wifi%u_0", radio_idx);
			ret = clsapi_interface_get_mac_addr(tmp_name, if_mac);
			if (ret < 0) {
				cls_error("get %s mac address failed, error %d", tmp_name, ret);
				continue;
			}
			if (!memcmp(mac, if_mac, ETH_ALEN)) {
				memcpy(ifname, tmp_name, IFNAMSIZ);
				break;
			}
		}
	} else {
		ret = -EINVAL;
		goto respond;
	}

	if ((strcasecmp(parameter, "macaddr") == 0) || strcasecmp(parameter, "bssid") == 0) {
		if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid, sizeof(ssid)) <= 0) {
			ret = clsapi_interface_get_mac_addr(ifname, mac);
			goto respond;
		}

		ret = cls_get_macaddr_by_ssid(ifname, ssid, parameter, mac);
	} else {
		ret = -EINVAL;
		cls_error("cannot get parameter string, error %d", ret);
		goto respond;
	}

respond:
	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_macaddr(cmd_tag, status, ret, mac, out_len, out);
}

void cls_handle_dev_send_1905(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
#define MAX_SUPP_TLV_NUM	5
	struct cls_cmd_request cmd_req;
	int status;
	int i = MAX_SUPP_TLV_NUM, ret = 0;
	char dest_alid[18], msg_type[8];
	char tlv_type[6], tlv_length[8], tlv_value[CLS_MAX_TLV_BUF];
	char npu_cli[72], cli_suffix[CLS_MAP_MAX_BUF+1024] = {0};
	char resp[256] = {0};
	char mid[32] = {0};

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_dut_npu_cfg.npu_topology == 1)
		snprintf(npu_cli, sizeof(npu_cli), "%s%s ubus",
			cls_dut_npu_cfg.ssh_cli, cls_dut_npu_cfg.br_ipaddr);

	ret = cls_handle_policy_config(cli_suffix, sizeof(cli_suffix), resp, sizeof(resp),
			&cmd_req, cls_dut_npu_cfg.npu_topology, npu_cli);
	if (ret) {
		ret = get_mid_field_value(resp, "\"mid\":", mid);
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_DEST_ALID, dest_alid, sizeof(dest_alid)) > 0) {
		if (cls_get_value_text(&cmd_req, CLS_TOK_MESSAGE_TYPE_VALUE,
			msg_type, sizeof(msg_type)) <= 0) {
			ret = -EINVAL;
			goto respond;
		}
		if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_TYPE,
				tlv_type, sizeof(tlv_type)) > 0) {
			if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_LENGTH,
				tlv_length, sizeof(tlv_length)) <= 0) {
				ret = -EINVAL;
				goto respond;
			}
			if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_VALUE,
				tlv_value, sizeof(tlv_value)) <= 0) {
				ret = -EINVAL;
				goto respond;
			}
			cls_rebuid_tlv_value(tlv_value);
			if (cls_dut_npu_cfg.npu_topology == 1) {
				snprintf(cli_suffix, sizeof(cli_suffix),
					"call map.cli send_1905 \\\'{\\\"DestALid\\\":\\\"%s\\\","
					"\\\"MessageTypeValue\\\":%lu,"
					"\\\"TLVs\\\":[{\\\"tlv_type\\\":%lu,\\\"tlv_length\\\":"
					"%lu,\\\"tlv_value\\\":\\\"%s\\\"},]}\\\'",
					dest_alid, strtoul(msg_type, NULL, 0),
					strtoul(tlv_type, NULL, 0), strtoul(tlv_length, NULL, 0),
					tlv_value);
				ret = system_cli_call(npu_cli, cli_suffix, resp, sizeof(resp));
			} else {
				sprintf(cli_suffix, "call clmesh.sigma dev_send_1905 \'{\"DestALid\":\"%s\","
					"\"MessageTypeValue\":%lu, \"tlv_type\":%lu,"
					"\"tlv_length\":%lu,\"tlv_value\":\"{%s}\"}\'",
					dest_alid, strtoul(msg_type, NULL, 0),
					strtoul(tlv_type, NULL, 0),
					strtoul(tlv_length, NULL, 0), tlv_value);
				cls_log("sigma MAP send 1905: %s\n", cli_suffix);
				ret = system_cli_call(CLS_UBUS_CLI_PREFIX, cli_suffix,
					resp, sizeof(resp));
			}
		} else if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_TYPE1,
				tlv_type, sizeof(tlv_type)) > 0) {
			if (cls_dut_npu_cfg.npu_topology == 1) {
				snprintf(cli_suffix, sizeof(cli_suffix),
					"call map.cli send_1905 \\\'{\\\"DestALid\\\":\\\"%s\\\","
					"\\\"MessageTypeValue\\\":%lu,",
					dest_alid, strtoul(msg_type, NULL, 0));
				strcat(cli_suffix, "\\\"TLVs\\\":[");
				do {
					enum cls_token type_tok =
						(enum cls_token)((int)CLS_TOK_TLV_TYPE5 - i + 1);
					enum cls_token length_tok =
						(enum cls_token)((int)CLS_TOK_TLV_LENGTH5 - i + 1);
					enum cls_token value_tok =
						(enum cls_token)((int)CLS_TOK_TLV_VALUE5 - i + 1);
					char string[CLS_MAX_TLV_BUF+1024] = {0,};

					if (cls_get_value_text(&cmd_req, type_tok,
							tlv_type, sizeof(tlv_type)) > 0) {
						strcat(cli_suffix, "{");
						if (cls_get_value_text(&cmd_req, length_tok,
							tlv_length, sizeof(tlv_length)) <= 0) {
							ret = -EINVAL;
							goto respond;
						}
						if (cls_get_value_text(&cmd_req, value_tok,
							tlv_value, sizeof(tlv_value)) <= 0) {
							ret = -EINVAL;
							goto respond;
						}
						cls_rebuid_tlv_value(tlv_value);
						sprintf(string, "\\\"tlv_type\\\":%lu,"
							"\\\"tlv_length\\\":"
							"%lu,\\\"tlv_value\\\":\\\"%s\\\"},",
							strtoul(tlv_type, NULL, 0),
							strtoul(tlv_length, NULL, 0), tlv_value);
						strcat(cli_suffix, string);
					}
				} while (i--);
				strcat(cli_suffix, "]}\\\'");
				ret = system_cli_call(npu_cli, cli_suffix, resp, sizeof(resp));
			} else {
				sprintf(cli_suffix, "call clmesh.sigma dev_send_1905 \'{\"DestALid\":\"%s\","
					"\"MessageTypeValue\":%lu,",
					dest_alid, strtoul(msg_type, NULL, 0));
				do {
					enum cls_token type_tok =
						(enum cls_token)((int)CLS_TOK_TLV_TYPE5 - i + 1);
					enum cls_token length_tok =
						(enum cls_token)((int)CLS_TOK_TLV_LENGTH5 - i + 1);
					enum cls_token value_tok =
						(enum cls_token)((int)CLS_TOK_TLV_VALUE5 - i + 1);
					char string[CLS_MAX_TLV_BUF+1024] = {0,};

					if (cls_get_value_text(&cmd_req, type_tok,
							tlv_type, sizeof(tlv_type)) > 0) {
						strcat(cli_suffix, "{");
						if (cls_get_value_text(&cmd_req, length_tok,
							tlv_length, sizeof(tlv_length)) <= 0) {
							ret = -EINVAL;
							goto respond;
						}
						if (cls_get_value_text(&cmd_req, value_tok,
							tlv_value, sizeof(tlv_value)) <= 0) {
							ret = -EINVAL;
							goto respond;
						}
						cls_rebuid_tlv_value(tlv_value);
						sprintf(string, "\"tlv_type\":%lu,\"tlv_length\":"
							"%lu,\"tlv_value\":\"{%s\"},",
							strtoul(tlv_type, NULL, 0),
							strtoul(tlv_length, NULL, 0), tlv_value);
						strcat(cli_suffix, string);
					}
				} while (i--);
				strcat(cli_suffix, "]}\'");
				cls_log("sigma MAP send 1905: %s\n", cli_suffix);
				ret = system_cli_call(CLS_UBUS_CLI_PREFIX, cli_suffix,
					resp, sizeof(resp));
			}
		} else {
			if (cls_dut_npu_cfg.npu_topology == 1) {
				snprintf(cli_suffix, sizeof(cli_suffix),
					"call map.cli send_1905 \\\'{\\\"DestALid\\\":\\\"%s\\\","
					"\\\"MessageTypeValue\\\":%lu}\\\'",
					dest_alid, strtoul(msg_type, NULL, 0));
				ret = system_cli_call(npu_cli, cli_suffix, resp, sizeof(resp));
			} else {
				sprintf(cli_suffix, "call clmesh.sigma dev_send_1905 \'{\"DestALid\":\"%s\","
					"\"MessageTypeValue\":%lu}\'",
					dest_alid, strtoul(msg_type, NULL, 0));
				ret = system_cli_call(CLS_UBUS_CLI_PREFIX,
					cli_suffix, resp, sizeof(resp));
			}
		}
		ret = get_mid_field_value(resp, "\"mid\":", mid);

	} else {
		ret = -EINVAL;
	}
respond:
	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_mid(cmd_tag, status, ret, mid, out_len, out);
}

static int cls_handle_policy_config(char *cmd, size_t cmd_len, char *resp, int resp_len,
		struct cls_cmd_request *cmd_req, int is_npu, char *npu_cli)
{
	char dest_alid[18], msg_type[8];

	// FIXME now only for request_policy_config set vlan configuration
	//
	if (cls_get_value_text(cmd_req, CLS_TOK_DEST_ALID, dest_alid, sizeof(dest_alid)) <= 0)
		goto handle_out;

	if (cls_get_value_text(cmd_req, CLS_TOK_MESSAGE_TYPE_VALUE,
				msg_type, sizeof(msg_type)) <= 0)
		goto handle_out;

#define CMDU_TYPE_MAP_POLICY_CONFIG_REQUEST 0x8003
	if (strtoul(msg_type, NULL, 0) == CMDU_TYPE_MAP_POLICY_CONFIG_REQUEST) {
		APPEND_CMD(cmd, cmd_len, "call qdock.map.controller request_policy_config ");
		if (!is_npu) {
			_mac_str_trim(dest_alid, ':');
			APPEND_CMD(cmd, cmd_len, "\'{\"dst_alid_s\":\"%s\", ", dest_alid);
			if (cls_handle_def8021q_tlv(cmd, cmd_len, cmd_req, is_npu) == 0)
				goto handle_out;

			if (cls_handle_traf_sep_tlv(cmd, cmd_len, cmd_req, is_npu) == 0)
				goto handle_out;

			APPEND_CMD(cmd, cmd_len, "}\'");
			if (system_cli_call(CLS_UBUS_TEST_CLI, cmd, resp, resp_len) < 0)
				goto handle_out;
		}
		return 1;
	}

handle_out:
	return 0;
}

#endif

