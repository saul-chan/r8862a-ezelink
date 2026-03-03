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

#include "common/csigma_log.h"
#include "common/csigma_tags.h"
#include "common/list.h"
#include "cls_dut_ap_handler.h"
#include "cls_dut_sta_handler.h"
#include "cls_dut_dev_handler.h"

extern struct list_head neigh_list;

void cls_handle_dut_init()
{
	INIT_LIST_HEAD(&neigh_list);
}

void cls_handle_dut_req(int tag, int len, unsigned char *params, int *out_len, unsigned char *out)
{
	cls_log("handle cmd, tag %x, len %x, out %p, out_len %x", tag, len, out, *out_len);

	switch (tag) {
	case CSIGMA_AP_GET_INFO_TAG:
		cls_handle_ap_get_info(len, params, out_len, out);
		break;
	case CSIGMA_AP_SET_RADIUS_TAG:
		cls_handle_ap_set_radius(len, params, out_len, out);
		break;
	case CSIGMA_AP_SET_WIRELESS_TAG:
		cls_handle_ap_set_wireless(len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_SECURITY_TAG:
		cls_handle_ap_set_security(len, params, out_len, out);
		break;

	case CSIGMA_AP_RESET_DEFAULT_TAG:
		cls_handle_ap_reset_default(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_11N_WIRELESS_TAG:
		cls_handle_ap_set_11n_wireless(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_REBOOT_TAG:
		cls_handle_ap_reset(len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_APQOS_TAG:
		cls_handle_ap_set_qos(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_STAQOS_TAG:
		cls_handle_ap_set_qos(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_CONFIG_COMMIT_TAG:
		cls_handle_ap_config_commit(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_CA_VERSION_TAG:
		cls_handle_ca_version(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_GET_MAC_ADDRESS_TAG:
		cls_handle_ap_get_mac_address(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_GET_PARAMETER_TAG:
		cls_handle_ap_get_parameter(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_HS2_TAG:
		cls_handle_ap_set_hs2(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_DEAUTH_STA_TAG:
		cls_handle_ap_deauth_sta(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_11D_TAG:
		cls_handle_ap_set_11d(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_11H_TAG:
		cls_handle_ap_set_11h(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_RFEATURE_TAG:
		cls_handle_ap_set_rfeature(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_SET_PMF_TAG:
		cls_handle_ap_set_pmf(len, params, out_len, out);
		break;

	case CSIGMA_AP_SEND_ADDBA_REQ_TAG:
		cls_handle_ap_send_addba_req(tag, len, params, out_len, out);
		break;

	case CSIGMA_AP_PRESET_TESTPARAMETERS_TAG:
		cls_handle_ap_preset_testparameters(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_CA_GET_VERSION_TAG:
		cls_handle_ca_version(tag, len, params, out_len, out);
		break;

#if 0
	case CSIGMA_STA_RESET_DEFAULT_TAG:
		cls_handle_sta_reset_default(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_DEVICE_LIST_INTERFACES_TAG:
		cnat_sta_device_list_interfaces(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_PRESET_TESTPARAMETERS_TAG:
		cls_handle_sta_preset_testparameters(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_DISCONNECT_TAG:
		cls_handle_sta_disconnect(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SEND_ADDBA_TAG:
		cls_handle_sta_send_addba(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_GET_MAC_ADDRESS_TAG:
		cls_handle_sta_get_mac_address(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_GET_INFO_TAG:
		cls_handle_sta_get_info(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_WIRELESS_TAG:
		cls_handle_sta_set_wireless(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_RFEATURE_TAG:
		cls_handle_sta_set_rfeature(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_IP_CONFIG_TAG:
		cls_handle_sta_set_ip_config(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_GET_IP_CONFIG_TAG:
		cls_handle_sta_get_ip_config(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SCAN_TAG:
		cls_handle_sta_scan(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_PSK_TAG:
		cls_handle_sta_set_psk(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_ASSOCIATE_TAG:
		cls_handle_sta_associate(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_ENCRYPTION_TAG:
		cls_handle_sta_set_encryption(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_SECURITY_TAG:
		cls_handle_sta_set_security(tag, len, params, out_len, out);
		break;
#endif

	case CSIGMA_DEV_SEND_FRAME_TAG:
		cls_handle_dev_send_frame(tag, len, params, out_len, out);
		break;
#if 0
	case CSIGMA_STA_REASSOC_TAG:
		cls_handle_sta_reassoc(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_SYSTIME_TAG:
		cls_handle_sta_set_systime(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_RADIO_TAG:
		cls_handle_sta_set_radio(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_MACADDR_TAG:
		cls_handle_sta_set_macaddr(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_UAPSD_TAG:
		cls_handle_sta_set_uapsd(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_RESET_PARM_TAG:
		cls_handle_sta_reset_parm(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_11N_TAG:
		cls_handle_sta_set_11n(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_POWER_SAVE_TAG:
		cls_handle_sta_set_power_save(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_SLEEP_TAG:
		cls_handle_sta_set_sleep(tag, len, params, out_len, out);
		break;
#endif
	case CSIGMA_DEVICE_GET_INFO_TAG:
		cls_handle_device_get_info(tag, len, params, out_len, out);
		break;
#if 0
	case CSIGMA_DEV_EXEC_ACTION_TAG:
		cls_handle_dev_exec_action(tag, len, params, out_len, out);
		break;

	case CSIGMA_DEV_CONFIGURE_IE_TAG:
		cls_handle_dev_configure_ie(tag, len, params, out_len, out);
		break;

	case CSIGMA_STA_SET_EAPTLS_TAG:
		cls_handle_sta_set_eaptls(tag, len, params, out_len, out);
		break;

#endif
	case CSIGMA_START_WPS_REGISTRATION_TAG:
		cls_handle_start_wps_registration(tag, len, params, out_len, out);
		break;

	case CSIGMA_DEV_SET_CONFIG_TAG:
		cls_handle_dev_set_config(tag, len, params, out_len, out);
		break;

	case CSIGMA_DEV_GET_PARAMETER_TAG:
		cls_handle_dev_get_parameter(tag, len, params, out_len, out);
		break;

	case CSIGMA_DEV_SET_RFEATURE_TAG:
		cls_handle_dev_set_rfeature(tag, len, params, out_len, out);
		break;

	case CSIGMA_DEV_SEND_1905_TAG:
		cls_handle_dev_send_1905(tag, len, params, out_len, out);
		break;

	case CSIGMA_DEV_RESET_DEFAULT_TAG:
		cls_handle_dev_reset_default(tag, len, params, out_len, out);
		break;

	default:
		cls_log("unsupported command %x, use default handler", tag);
		cls_handle_unknown_command(tag, len, params, out_len, out);
		break;
	}
}
