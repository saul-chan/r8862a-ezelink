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

#include <string.h>

#include "wfa_types.h"
#include "wfa_agtctrl.h"
#include "common/csigma_log.h"
#include "common/csigma_tags.h"
#include "ap/cls_ap.h"
#include "sta/cls_sta.h"
#include "dev/cls_dev.h"
#include <cls/clsapi.h>

typeNameStr_t cls_capi_to_tlv[] = {
	/* AP */
	{CSIGMA_AP_GET_INFO_TAG, "AP_GET_INFO", cnat_ap_get_info_tlv}
	,
	{CSIGMA_AP_SET_RADIUS_TAG, "AP_SET_RADIUS", cnat_ap_set_radius_request}
	,
	{CSIGMA_AP_SET_WIRELESS_TAG, "AP_SET_WIRELESS", cnat_ap_set_wireless_request}
	,
	{CSIGMA_AP_RESET_DEFAULT_TAG, "AP_RESET_DEFAULT", cls_ca_encode_ap_reset_default}
	,
	{CSIGMA_AP_SET_11N_WIRELESS_TAG, "AP_SET_11N_WIRELESS", cls_ca_encode_ap_set_11n_wireless}
	,
	{CSIGMA_AP_SET_SECURITY_TAG, "AP_SET_SECURITY", cnat_ap_set_security}
	,
	{CSIGMA_AP_REBOOT_TAG, "AP_REBOOT", cnat_ap_reboot}
	,
	{CSIGMA_AP_SET_APQOS_TAG, "AP_SET_APQOS", cls_ca_encode_ap_set_apqos}
	,
	{CSIGMA_AP_SET_STAQOS_TAG, "AP_SET_STAQOS", cls_ca_encode_ap_set_staqos}
	,
	{CSIGMA_AP_CONFIG_COMMIT_TAG, "AP_CONFIG_COMMIT", cls_ca_encode_ap_config_commit}
	,
	{CSIGMA_AP_DEAUTH_STA_TAG, "AP_DEAUTH_STA", cls_ca_encode_ap_deauth_sta}
	,
	{CSIGMA_AP_GET_MAC_ADDRESS_TAG, "AP_GET_MAC_ADDRESS", cls_ca_encode_ap_get_mac_address}
	,
	{CSIGMA_AP_GET_PARAMETER_TAG, "AP_GET_PARAMETER", cls_ca_encode_ap_get_parameter}
	,
	{CSIGMA_AP_SET_11D_TAG, "AP_SET_11D", cls_ca_encode_ap_set_11d}
	,
	{CSIGMA_AP_SET_11H_TAG, "AP_SET_11H", cls_ca_encode_ap_set_11h}
	,
	{CSIGMA_AP_CA_VERSION_TAG, "AP_CA_VERSION", cnat_ap_ca_version}
	,
	{CSIGMA_AP_SET_PMF_TAG, "AP_SET_PMF", cnat_ap_set_pmf}
	,
	{CSIGMA_AP_SET_RFEATURE_TAG, "AP_SET_RFEATURE", cls_ca_encode_ap_set_rfeature}
	,
	{CSIGMA_AP_SET_HS2_TAG, "AP_SET_HS2", cls_ca_encode_ap_set_hs2}
	,
	{CSIGMA_AP_SEND_ADDBA_REQ_TAG, "ap_send_addba_req", cls_ca_encode_ap_send_addba_req}
	,
	{CSIGMA_AP_PRESET_TESTPARAMETERS_TAG, "ap_preset_testparameters",
		cls_ca_encode_ap_preset_testparameters}
	,

	/* STA */
	{CSIGMA_STA_CA_GET_VERSION_TAG, "CA_GET_VERSION", cnat_sta_ca_get_version}
	,
	{CSIGMA_STA_RESET_DEFAULT_TAG, "STA_RESET_DEFAULT", cls_sta_encode_ap_reset_default}
	,
	{CSIGMA_STA_DEVICE_LIST_INTERFACES_TAG, "DEVICE_LIST_INTERFACES",
		cnat_sta_device_list_interfaces_request}
	,
	{CSIGMA_STA_PRESET_TESTPARAMETERS_TAG, "STA_PRESET_TESTPARAMETERS",
		cls_ca_encode_sta_preset_testparameters}
	,
	{CSIGMA_STA_DISCONNECT_TAG, "STA_DISCONNECT", cls_ca_encode_sta_disconnect}
	,
	{CSIGMA_STA_SEND_ADDBA_TAG, "STA_SEND_ADDBA", cls_ca_encode_sta_send_addba}
	,
	{CSIGMA_STA_GET_MAC_ADDRESS_TAG, "STA_GET_MAC_ADDRESS", cls_ca_encode_sta_get_mac_address}
	,
	{CSIGMA_STA_GET_INFO_TAG, "STA_GET_INFO", cls_ca_encode_sta_get_info}
	,
	{CSIGMA_STA_SET_WIRELESS_TAG, "STA_SET_WIRELESS", cls_ca_encode_sta_set_wireless}
	,
	{CSIGMA_STA_SET_RFEATURE_TAG, "STA_SET_RFEATURE", cls_ca_encode_sta_set_rfeature}
	,
	{CSIGMA_STA_SET_IP_CONFIG_TAG, "STA_SET_IP_CONFIG", cls_ca_encode_sta_set_ip_config}
	,
	{CSIGMA_STA_SET_PSK_TAG, "STA_SET_PSK", cls_ca_encode_sta_set_psk}
	,
	{CSIGMA_STA_ASSOCIATE_TAG, "STA_ASSOCIATE", cls_ca_encode_sta_associate}
	,
	{CSIGMA_STA_SET_ENCRYPTION_TAG, "STA_SET_ENCRYPTION", cls_ca_encode_sta_set_encryption}
	,
	{CSIGMA_STA_SET_SECURITY_TAG, "STA_SET_SECURITY", cls_ca_encode_sta_set_security}
	,
	{CSIGMA_DEV_SEND_FRAME_TAG, "DEV_SEND_FRAME", cls_ca_encode_dev_send_frame}
	,
	{CSIGMA_STA_REASSOC_TAG, "STA_REASSOC", cls_ca_encode_sta_reassoc}
	,
	{CSIGMA_STA_SET_SYSTIME_TAG, "STA_SET_SYSTIME", cls_ca_encode_sta_set_systime}
	,
	{CSIGMA_STA_SET_RADIO_TAG, "STA_SET_RADIO", cls_ca_encode_sta_set_radio}
	,
	{CSIGMA_STA_SET_MACADDR_TAG, "STA_SET_MACADDR", cls_ca_encode_sta_set_macaddr}
	,
	{CSIGMA_STA_SET_UAPSD_TAG, "STA_SET_UAPSD", cls_ca_encode_sta_set_uapsd}
	,
	{CSIGMA_STA_RESET_PARM_TAG, "STA_RESET_PARM", cls_ca_encode_sta_reset_parm}
	,
	{CSIGMA_STA_SET_11N_TAG, "STA_SET_11N", cls_ca_encode_sta_set_11n}
	,
	{CSIGMA_STA_SET_POWER_SAVE_TAG, "SET_POWER_SAVE", cls_ca_encode_sta_set_power_save}
	,
	{CSIGMA_STA_SET_SLEEP_TAG, "STA_SET_SLEEP", cls_ca_encode_sta_set_sleep}
	,
	{CSIGMA_STA_GET_IP_CONFIG_TAG, "STA_GET_IP_CONFIG", cls_ca_encode_sta_get_ip_config}
	,
	{CSIGMA_STA_SCAN_TAG, "STA_SCAN", cls_ca_encode_sta_scan}
	,
	{CSIGMA_DEVICE_GET_INFO_TAG, "DEVICE_GET_INFO", cls_ca_encode_device_get_info}
	,
	{CSIGMA_DEV_EXEC_ACTION_TAG, "DEV_EXEC_ACTION", cls_ca_encode_dev_exec_action}
	,
	{CSIGMA_START_WPS_REGISTRATION_TAG, "START_WPS_REGISTRATION",
		cls_ca_encode_start_wps_registration}
	,
	{CSIGMA_DEV_CONFIGURE_IE_TAG, "DEV_CONFIGURE_IE", cls_ca_encode_dev_configure_ie}
	,
	{CSIGMA_STA_SET_EAPTLS_TAG, "STA_SET_EAPTLS", cls_ca_encode_sta_set_eaptls}
	,
	{CSIGMA_DEV_SET_RFEATURE_TAG, "DEV_SET_RFEATURE", cls_ca_encode_dev_set_rfeature}
	,
	{CSIGMA_DEV_RESET_DEFAULT_TAG, "DEV_RESET_DEFAULT", cls_ca_encode_dev_reset_default}
	,
	{CSIGMA_DEV_GET_PARAMETER_TAG, "DEV_GET_PARAMETER", cls_ca_encode_dev_get_parameter}
	,
	{CSIGMA_DEV_SET_CONFIG_TAG, "DEV_SET_CONFIG", cls_ca_encode_dev_set_config}
	,
	{CSIGMA_DEV_SEND_1905_TAG, "DEV_SEND_1905", cls_ca_encode_dev_send_1905}
	,
	{-1, "", NULL}
	,
};

dutCommandRespFuncPtr cls_dut_responce_map[] = {
	[CSIGMA_AP_GET_INFO_TAG - CSIGMA_TAG_BASE] = cnat_ap_get_info_resp,
	[CSIGMA_AP_SET_RADIUS_TAG - CSIGMA_TAG_BASE] = cnat_ap_generic_resp,
	[CSIGMA_AP_SET_WIRELESS_TAG - CSIGMA_TAG_BASE] = cnat_ap_generic_resp,
	[CSIGMA_AP_RESET_DEFAULT_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SET_11N_WIRELESS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SET_SECURITY_TAG - CSIGMA_TAG_BASE] = cnat_ap_generic_resp,
	[CSIGMA_AP_SET_APQOS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SET_STAQOS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_CONFIG_COMMIT_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_DEAUTH_STA_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_GET_MAC_ADDRESS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_GET_PARAMETER_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SET_11H_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SET_11D_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_CA_VERSION_TAG - CSIGMA_TAG_BASE] = cnat_ap_ca_version_resp,
	[CSIGMA_AP_SET_PMF_TAG - CSIGMA_TAG_BASE] = cnat_ap_generic_resp,
	[CSIGMA_AP_REBOOT_TAG - CSIGMA_TAG_BASE] = cnat_ap_reboot_resp,
	[CSIGMA_AP_SET_RFEATURE_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SET_HS2_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_SEND_ADDBA_REQ_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,
	[CSIGMA_AP_PRESET_TESTPARAMETERS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_ap,

	/* STA */
	[CSIGMA_STA_CA_GET_VERSION_TAG - CSIGMA_TAG_BASE] = cnat_sta_ca_get_version_resp,
	[CSIGMA_STA_RESET_DEFAULT_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_DEVICE_LIST_INTERFACES_TAG - CSIGMA_TAG_BASE] =
		cnat_sta_device_list_interfaces_resp,
	[CSIGMA_STA_PRESET_TESTPARAMETERS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_DISCONNECT_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SEND_ADDBA_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_GET_MAC_ADDRESS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_GET_INFO_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_WIRELESS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_RFEATURE_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_IP_CONFIG_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_PSK_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_ASSOCIATE_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_ENCRYPTION_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_SECURITY_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_DEV_SEND_FRAME_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_REASSOC_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_SYSTIME_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_RADIO_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_MACADDR_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_UAPSD_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_RESET_PARM_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_11N_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_POWER_SAVE_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_SLEEP_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_GET_IP_CONFIG_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SCAN_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_DEVICE_GET_INFO_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_DEV_EXEC_ACTION_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_START_WPS_REGISTRATION_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_DEV_CONFIGURE_IE_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,
	[CSIGMA_STA_SET_EAPTLS_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_sta,

	/* dev */
	[CSIGMA_DEV_SET_RFEATURE_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_dev,
	[CSIGMA_DEV_RESET_DEFAULT_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_dev,
	[CSIGMA_DEV_GET_PARAMETER_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_dev,
	[CSIGMA_DEV_SET_CONFIG_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_dev,
	[CSIGMA_DEV_SEND_1905_TAG - CSIGMA_TAG_BASE] = cls_ca_respond_dev,
};

void cls_handle_dut_responce(unsigned tag, unsigned char *data)
{
	int index = tag - CSIGMA_TAG_BASE;
	if (tag >= CSIGMA_TAG_BASE && index < ARRAY_SIZE(cls_dut_responce_map) &&
		cls_dut_responce_map[index]) {
		cls_log("handle dut response for tag 0x%x", tag);
		cls_dut_responce_map[index] (data);
	} else {
		cls_error("can't handle dut response for tag 0x%x, index %d", tag, index);
	}
}
