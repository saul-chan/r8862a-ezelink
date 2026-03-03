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
#include "cls_dut_dev_handler.h"
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


static char g_cmdbuf[CLS_MAX_CMD_BUF] = {0};

static void _mac_str_trim(char *str, char delimiter)
{
	int i;
#define MAC_LEN 17
	for (i = 0; i < MAC_LEN; i++) {
		if (str[i] == delimiter)
			memmove(str+i, str+i+1, MAC_LEN-i);
	}
}

void insertsubstr(char *string, char *sub, int pos)
{
	int sublen = strlen(sub);
	char *product = (char *)malloc(strlen(string) + sublen + 1);

	strncpy(product, string, pos);
	product[pos] = '\0';
	strcat(product, sub);
	strcat(product, string + pos);

	strcpy(string, product);
	free(product);
}

void remsubstr(char *string, char *sub)
{
	char *match;
	int len = strlen(sub);

	while ((match = strstr(string, sub))) {
		*match = '\0';
		strcat(string, match+len);
	}
}

static int get_mid_field_value(char *in, char *field, char *out)
{
	char *rc, *pos;
	char mid_str[32] = {0,};
	uint16_t mid;

	if (!in || !field)
		return -1;

	rc = strstr(in, "\"rc\"");
	pos = strstr(in, field);

	if (!rc || !pos)
		return -1;

	rc = rc + strlen("\"rc\"");
	if (strncmp(rc, ": \"Successed\"", 13))
		return -1;

	pos = pos + strlen(field);
	remsubstr(pos, " ");
	remsubstr(pos, "}");
	mid = strtoul(pos, NULL, 10);
	cls_log("get mid %u", mid);
	sprintf(mid_str, "%0x", mid);
	insertsubstr(mid_str, "0x", 0);
	strncpy(out, mid_str, strlen(mid_str));

	return 0;
}

int syscall_with_ret(const char *cli, char *cli_suffix, char *resp, int resp_len)
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

void cls_handle_dev_send_1905(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
#define MAX_SUPP_TLV_NUM	5
	struct cls_cmd_request cmd_req;
	int status;
	int i, ret = 0;
	char dest_alid[18], msg_type[8];
	char tlv_type[6], tlv_length[8], tlv_value[CLS_MAX_TLV_BUF];
	char resp[256] = {0};
	char mid[32] = {0};
	char sub_tlv_name[3][16] = {0};

	cls_error("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	memset(g_cmdbuf, 0, sizeof(g_cmdbuf));

	if (cls_get_value_text(&cmd_req, CLS_TOK_DEST_ALID, dest_alid, sizeof(dest_alid)) > 0) {
		if (cls_get_value_text(&cmd_req, CLS_TOK_MESSAGE_TYPE_VALUE,
			msg_type, sizeof(msg_type)) <= 0) {
			ret = -EINVAL;
			goto respond;
		}

		/* topology query / AP capability query / channel preference query contain zero TLV. */
		if (0 == strcasecmp("0x0002", msg_type) || 0 == strcasecmp("0x8001", msg_type)
				|| 0 == strcasecmp("0x8004", msg_type)) {
			sprintf(g_cmdbuf, "clmesh.sigma dev_send_1905 \'{\"DestALid\":\"%s\","
				"\"MessageTypeValue\":\"%s\"}\'", dest_alid, msg_type);
				ret = syscall_with_ret(CLS_UBUS_CLI_PREFIX, g_cmdbuf, resp, sizeof(resp));
				goto respond;
		}

		sprintf(g_cmdbuf, "clmesh.sigma dev_send_1905 \'{\"DestALid\":\"%s\","
			"\"MessageTypeValue\":\"%s\", ", dest_alid, msg_type);

		for (i = 0; i < MAX_SUPP_TLV_NUM; i++) {
			if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_TYPE1 + i,
					tlv_type, sizeof(tlv_type)) > 0) {
				if (i)
					strcat(g_cmdbuf, ", ");
				sprintf(sub_tlv_name[0], "\"tlv_type%d\":\"", i);
				strcat(g_cmdbuf, sub_tlv_name[0]);
				strcat(g_cmdbuf, tlv_type);
				strcat(g_cmdbuf, "\", ");
				if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_LENGTH1 + i,
					tlv_length, sizeof(tlv_length)) <= 0) {
					ret = -EINVAL;
					goto respond;
				}
				sprintf(sub_tlv_name[1], "\"tlv_length%d\":\"", i);
				strcat(g_cmdbuf, sub_tlv_name[1]);
				strcat(g_cmdbuf, tlv_length);
				strcat(g_cmdbuf, "\", ");
				if (cls_get_value_text(&cmd_req, CLS_TOK_TLV_VALUE1 + i,
					tlv_value, sizeof(tlv_value)) <= 0) {
					ret = -EINVAL;
					goto respond;
				}
				sprintf(sub_tlv_name[2], "\"tlv_value%d\":\"", i);
				strcat(g_cmdbuf, sub_tlv_name[2]);
				strcat(g_cmdbuf, tlv_value);
				strcat(g_cmdbuf, "\"");
			}
			else
				break;
		}
		strcat(g_cmdbuf, "}\'");
		ret = syscall_with_ret(CLS_UBUS_CLI_PREFIX, g_cmdbuf, resp, sizeof(resp));
	} else {
		cls_error("can NOT find alid dest\n");
		ret = -EINVAL;
	}
respond:
	ret = get_mid_field_value(resp, "\"mid\":", mid);

	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_mid(cmd_tag, status, ret, mid, out_len, out);
}

void cls_handle_start_wps_registration(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	char ifname[IFNAMSIZ];
	char band[16];
	int band_idx;
	char wps_method[16];
	char cli_suffix[256];
	char resp[256];
	int status;
	int ret = 0;

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_get_value_text(&cmd_req, CLS_TOK_BAND, band, sizeof(band)) > 0) {
		if (cls_get_value_text(&cmd_req, CLS_TOK_WPS_CONFIG_METHOD,
			wps_method, sizeof(wps_method)) <= 0) {
			ret = -EINVAL;
			goto respond;
		}
		if (NULL == strstr(wps_method, "PBC")) {
			ret = -EINVAL;
			goto respond;
		}

		if ((strncasecmp(band, "5GL", 3) == 0) || (strncasecmp(band, "5GH", 3) == 0))
			band_idx = CLS_DUT_5G_BAND;
		else
			band_idx = CLS_DUT_2P4G_BAND;

		clsapi_get_wps_action_ifname(ifname, band_idx);
		cls_log("start wps on ifname(%s) band(%d)", ifname, band_idx);

		snprintf(cli_suffix, sizeof(cli_suffix), "ubus call clmesh.api start_wps \'{\"ifname\":\"%s\"}\'", ifname);
		system(cli_suffix);
	}

respond:
	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_dev_set_config(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	char cert_prog[16], backhaul[16];
	char bss_info[128];
	int status, ret = 0, i;
	char resp[256];

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	/* mandatory certification program, e.g. MAP */
	if (cls_get_value_text(&cmd_req, CLS_TOK_PROGRAM, cert_prog, sizeof(cert_prog)) <= 0) {
		ret = -EINVAL;
		goto respond;
	}
	sprintf(g_cmdbuf, "%s", "clmesh.sigma dev_set_config \'{\"program\":\"map\", ");
	for (i = CLS_TOK_BSS_INFO1; i < CLS_TOK_BSS_INFO12; i++) {
		if (cls_get_value_text(&cmd_req, i, bss_info, sizeof(bss_info)) > 0) {
			if (i > CLS_TOK_BSS_INFO1)
				strcat(g_cmdbuf, ", ");
			strcat(g_cmdbuf, "\"bss_info\":");
			strcat(g_cmdbuf, "\"");
			strcat(g_cmdbuf, bss_info);
			strcat(g_cmdbuf, "\"");
		}
		else
			break;
	}
	if (cls_get_value_text(&cmd_req, CLS_TOK_BACKHAUL, backhaul, sizeof(backhaul)) > 0) {
#if 0
		if (0 != strcasecmp(backhaul, "eth")) {
			system("uci set cls-opmode.globals.mode='repeater'");
			system("uci commit");
			system("ubus call network reload");
			sleep(20);
		}
#endif
		strcat(g_cmdbuf, "\"backhaul\":");
		strcat(g_cmdbuf, "\"");
		strcat(g_cmdbuf, backhaul);
		strcat(g_cmdbuf, "\"");
	}
	strcat(g_cmdbuf, "}\'");
	ret = syscall_with_ret(CLS_UBUS_CLI_PREFIX, g_cmdbuf, resp, sizeof(resp));

respond:
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
	char cmd[256] = {0};
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

	if (strcasestr(cert_prog, "map")) {
		system("clsapi restore wireless_configuration");
		sleep(10);
		system("uci set wireless.radio1.smart_dfs_cac=1");
		system("uci commit");

		system("ubus call clmesh.sigma reset_ifname");
		sleep(15);
		system("/etc/init.d/cls-mesh stop");

		snprintf(cmd, sizeof(cmd), "uci set cls-mesh.default.mode=%s", dev_role);
		system(cmd);
		system("uci commit");
		system("/etc/init.d/cls-mesh start");
	}
respond:
	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}

void cls_handle_dev_get_parameter(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out)
{
	struct cls_cmd_request cmd_req;
	char parameter[16];
	char ruid[16];
	char ssid_param[33], ssid_tmp[33];
	char ifname[IFNAMSIZ] = "br-lan";
	unsigned char mac[ETH_ALEN];
	char buf[128];
	int status;
	int ret = 0;

	cls_log("%s", __func__);
	ret = cls_init_cmd_request(&cmd_req, cmd_tag, params, len);
	if (ret != 0)
		goto respond;

	if (cls_get_value_text(&cmd_req, CLS_TOK_PARAMETER, parameter, sizeof(parameter)) <= 0) {
		ret = -EINVAL;
		goto respond;
	}

	if (strcasecmp(parameter, "ALid") == 0) {
		ret = clsapi_interface_get_mac_addr(ifname, mac);
		goto respond;
	}

	if (cls_get_value_text(&cmd_req, CLS_TOK_RUID, ruid, sizeof(ruid)) <= 0
		|| strlen(ruid) != 14) {
		cls_error("cannot get ruid, error");
		ret = -EINVAL;
		goto respond;
	}

	/* use radio uid to find the ifname */
	snprintf(buf, sizeof(buf), "%.2s:%.2s:%.2s:%.2s:%.2s:%.2s",
		ruid+2, ruid+4, ruid+6, ruid+8, ruid+10, ruid+12);
	if (ether_aton_r(buf, (struct ether_addr *)mac)) {
		uint8_t radio_idx;
		unsigned char if_mac[ETH_ALEN];
		/* find bSTA's ifname by ruid parameter */
		for (radio_idx = 0; radio_idx < CLS_MAX_RADIO_ID; radio_idx++) {
			char tmp_name[IFNAMSIZ];

			snprintf(tmp_name, sizeof(tmp_name), "wlan%u", radio_idx);
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
		if (cls_get_value_text(&cmd_req, CLS_TOK_SSID, ssid_param, sizeof(ssid_param)) <= 0) {
			ret = clsapi_interface_get_mac_addr(ifname, mac);
			if (0 == ret) { /* use ssid to double check */
				clsapi_wifi_get_ssid(ifname, ssid_tmp);
				ret = strcasecmp(ssid_param, ssid_tmp);
				goto respond;
			}
		}
	} else {
		ret = -EINVAL;
		cls_error("cannot get parameter string, error %d", ret);
		goto respond;
	}

respond:
	status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_macaddr(cmd_tag, status, ret, mac, out_len, out);
}

int find_ifname_by_radio_uid(char *uid, char *ifname)
{
#define RADIO_NAME_SIZE 5
	char cmd[64] = {0};
	char one_line[128] = {0};
	char *ptr_line = NULL, *ptr_mac_prefix = NULL;
	FILE *tmpfd;
	int ret;

	if (ifname ==NULL) {
		return -EINVAL;
	}

	sprintf(cmd, "ifconfig |grep HWaddr |grep wlan > /tmp/radio_uid.txt", ifname);
	ret = system(cmd);
	if (ret != 0){
		return -ENODATA;
	}
	tmpfd = fopen("/tmp/radio_uid.txt", "r");
	while (NULL != (ptr_line = fgets(one_line, 128, tmpfd))) {
		ptr_mac_prefix = strstr(ptr_line, "HWaddr");
		if (!ptr_mac_prefix) {
			cls_log("wrong content\n");
			fclose(tmpfd);
			return;
		}
		if (0 == strcmp(ptr_mac_prefix+7, uid)) {
			snprintf(ifname, RADIO_NAME_SIZE, "%s", ptr_line);
			break;
		}
	}
	fclose(tmpfd);
	return 0;
}

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
	char ifname[IFNAMSIZ] = {0};

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

	if (0 == strncasecmp(type_str, "MBO", 3)) {
		find_ifname_by_radio_uid(bssid_str, ifname);

		snprintf(cmd, sizeof(cmd), "hostapd_cli -i %s set mbo_assoc_disallow %d", ifname, assoc_disallow);
		PRINT("set_assoc_status radio(%s) disallow(%d), cmd=%s\n", ifname, assoc_disallow, cmd);
		system(cmd);
	}

respond:
	if (status != STATUS_INVALID)
		status = ret < 0 ? STATUS_ERROR : STATUS_COMPLETE;
	cls_dut_make_response_none(cmd_tag, status, ret, out_len, out);
}
