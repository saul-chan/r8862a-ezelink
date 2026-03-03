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

#include "cls_ap.h"
#include "common/csigma_log.h"
#include "common/csigma_tags.h"
#include "common/cls_ca_common.h"
#include "common/cls_dut_common.h"
//#include "wfa_types.h"
#include "wfa_tlv.h"
#include "wfa_sock.h"

extern char gRespStr[];
extern int gCaSockfd;

int cnat_ap_get_info_tlv(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("start cnat_ap_get_info_tlv");

	if (aBuf == NULL)
		return WFA_FAILURE;

	wfaEncodeTLV(CSIGMA_AP_GET_INFO_TAG, 0, NULL, aBuf);

	*aLen = 4;

	return WFA_SUCCESS;
}

int cnat_ap_get_info_resp(unsigned char *cmd_buf)
{
	struct cls_dut_response *getverResp = (struct cls_dut_response *)(cmd_buf + 4);
	cls_log("ap_get_info responce");

	sprintf(gRespStr, "status,COMPLETE,interface,%s,firmware,%s,agent,%s\r\n",
		getverResp->ap_info.interface_list,
		getverResp->ap_info.firmware_version, getverResp->ap_info.agent_version);

	wfaCtrlSend(gCaSockfd, (unsigned char *)gRespStr, strlen(gRespStr));
	return WFA_SUCCESS;
}

int cnat_ap_set_radius_request(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };
	char *save;

	request.ap_radius.vap_index = -1;

	for (char *str = strtok_r(pcmdStr, ",", &save); str && str[0];
		str = strtok_r(NULL, ",", &save)) {

		if (strcasecmp(str, "IPADDR") == 0 && (str = strtok_r(NULL, ",", &save))) {

			snprintf(request.ap_radius.ip, sizeof(request.ap_radius.ip), "%s", str);

		} else if (str && strcasecmp(str, "PORT") == 0 &&
				(str = strtok_r(NULL, ",", &save))) {

			sscanf(str, "%hu", &request.ap_radius.port);

		} else if (str && strcasecmp(str, "PASSWORD") == 0 &&
				(str = strtok_r(NULL, ",", &save))) {

			snprintf(request.ap_radius.password,
				sizeof(request.ap_radius.password), "%s", str);

		} else if (str && strcasecmp(str, "INTERFACE") == 0 &&
				(str = strtok_r(NULL, ",", &save))) {

			snprintf(request.ap_radius.if_name,
				sizeof(request.ap_radius.if_name), "%s", str);

		} else if (str && strcasecmp(str, "WLAN_TAG") == 0 &&
				(str = strtok_r(NULL, ",", &save))) {

			request.ap_radius.vap_index = strtol(str, NULL, 10) - 1;

		}
	}

	wfaEncodeTLV(CSIGMA_AP_SET_RADIUS_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

void local_set_band_info(struct cls_dut_request *request, char *value)
{
	if (request && value) {
		if (strcasecmp(value, "6G") == 0)
			request->wireless.chan_band[0].band = clsapi_freq_band_6_ghz;
		else if (strcasecmp(value, "5G") == 0)
			request->wireless.chan_band[0].band = clsapi_freq_band_5_ghz;
		else if (strcasecmp(value, "24G") == 0)
			request->wireless.chan_band[0].band = clsapi_freq_band_2pt4_ghz;
/*
	//6GHz need disscuss later;
		if (strstr(value, ";") != NULL) {
			request->wireless.chan_band[1].band = clsapi_freq_band_5_ghz;
			request->wireless.chan_band[0].band = clsapi_freq_band_2pt4_ghz;
		} else {
			if (strcasecmp(value, "11ax") == 0 ||strcasecmp(value, "11ac") == 0 || strcasecmp(value, "11na") == 0)
			request->wireless.chan_band[0].band = clsapi_freq_band_5_ghz;
			else if (strcasecmp(value, "11axng") == 0 ||strcasecmp(value, "11acng") == 0 ||strcasecmp(value, "11n") == 0
			|| strcasecmp(value, "11b") == 0 || strcasecmp(value, "11bg") == 0 ||strcasecmp(value, "11bgn") == 0
			|| strcasecmp(value, "11g") == 0)
			request->wireless.chan_band[0].band = clsapi_freq_band_2pt4_ghz;
		}
*/
	}
}

void legacy_mode_get_band_info(int32_t channel, uint8_t *band)
{
	if (channel >= CLSAPI_MIN_CHANNEL) {
		if (channel < CLSAPI_MIN_CHANNEL_5G)
			*band = clsapi_freq_band_2pt4_ghz;

		if (channel >= CLSAPI_MIN_CHANNEL_5G)
			*band = clsapi_freq_band_5_ghz;
	}

}

int cnat_ap_set_wireless_request(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };
	char *save;
	char *name;
	char *value;
	//uint32_t chnlfreq;
	//unsigned int band;
	//unsigned int channel;
	//int ret;

	for (name = strtok_r(pcmdStr, ",", &save), value = strtok_r(NULL, ",", &save);
		name && *name && value;
		name = strtok_r(NULL, ",", &save), value = strtok_r(NULL, ",", &save)) {

		cls_log("%s -> %s", name, value);
		if (strcasecmp(name, "NAME") == 0) {
			snprintf(request.wireless.Name,
				sizeof(request.wireless.Name), "%s", value);
		} else if (strcasecmp(name, "Program") == 0) {
			snprintf(request.wireless.programm,
				sizeof(request.wireless.programm), "%s", value);
		} else if (strcasecmp(name, "INTERFACE") == 0) {
			local_set_band_info(&request, value);

			if (strcasecmp(value, "5G") != 0 || strcasecmp(value, "6G") != 0) {
				snprintf(request.wireless.if_name,
					sizeof(request.wireless.if_name), "%s", value);
			}
		} else if (strcasecmp(name, "SSID") == 0) {
			snprintf(request.wireless.ssid, sizeof(request.wireless.ssid), "%s", value);
		} else if (strcasecmp(name, "CHANNEL") == 0) {
			// channel number
			// ; separated for dual band
			// 36;6
			/*if (sscanf(value, "%d;%d", &request.wireless.chan_band[1].chan,
					&request.wireless.chan_band[0].chan) != 2)
				sscanf(value, "%d", &request.wireless.chan_band[0].chan);*/
			if (strstr(value, ";") != NULL) {
				sscanf(value, "%d;%d", &request.wireless.chan_band[1].chan,
					&request.wireless.chan_band[0].chan);
			} else {
				sscanf(value, "%d", &request.wireless.chan_band[0].chan);
			}
		} else if (strcasecmp(name, "CHNLFREQ") == 0) {
			//chnlfreq = strtol(value, NULL, 10);
			//ret = clsapi_wifi_freq2chan_and_band(chnlfreq, &band, &channel);
			//ret = 0;
			//if (ret >= 0) {
			//	request.wireless.chan_band[0].chan = channel;
			//	request.wireless.chan_band[0].band = band;
			//}
		} else if (strcasecmp(name, "MODE") == 0) {
			local_set_band_info(&request, value);

			// 11b, 11bg, 11bgn, 11a, 11na,11ac
			// or 11ac;11ng
			if (strstr(value, ";") != NULL) {
				sscanf(value, "%[^;];%[^;]", request.wireless.mode[1],
						request.wireless.mode[0]);
			} else {
				snprintf(request.wireless.mode[0], sizeof(request.wireless.mode[0]),
						"%s", value);
			}
		} else if (strcasecmp(name, "WME") == 0) {
			// WME on/off , String
			request.wireless.wmm = strcasecmp(value, "off") != 0;
			request.wireless.has_wmm = 1;
		} else if (strcasecmp(name, "WMMPS") == 0) {
			// APSD on/off
			request.wireless.apsd = strcasecmp(value, "off") != 0;
			request.wireless.has_apsd = 1;
		} else if (strcasecmp(name, "RTS") == 0) {
			// Threshold, Short Integer
			sscanf(value, "%d", &request.wireless.rts_threshold);
			request.wireless.has_rts_threshold = 1;
		} else if (strcasecmp(name, "FRGMNT") == 0) {
			// Fragmentation, Short Integer
		} else if (strcasecmp(name, "PWRSAVE") == 0) {
			// Power Save, String
			request.wireless.power_save = strcasecmp(value, "off") != 0;
			request.wireless.has_power_save = 1;
		} else if (strcasecmp(name, "BCNINT") == 0) {
			// Beacon Interval
			sscanf(value, "%u", &request.wireless.beacon_interval);
			request.wireless.has_beacon_interval = 1;
		} else if (strcasecmp(name, "RADIO") == 0) {
			// On/Off the radio of given interface
			request.wireless.rf_enable = strcasecmp(value, "off") != 0;
			request.wireless.has_rf_enable = 1;
		} else if (strcasecmp(name, "ADDBA_REJECT") == 0) {
			// Reject any ADDBA request by sending ADDBA response with status decline
			request.wireless.has_addba_reject = 1;
			request.wireless.addba_reject = strcasecmp(value, "Disable") != 0;
		} else if (strcasecmp(name, "AMPDU") == 0) {
			//AMPDU Aggregation: Enable, Disable
			request.wireless.has_ampdu = 1;
			request.wireless.ampdu = strcasecmp(value, "Disable") != 0;
		} else if (strcasecmp(name, "AMPDU_EXP") == 0) {
			// Maximum AMPDU Exponent: Short Integer
		} else if (strcasecmp(name, "AMSDU") == 0) {
			// AMSDU Aggregation: Enable, Disable
			request.wireless.amsdu = strcasecmp(value, "Disable") != 0;
			request.wireless.has_amsdu = 1;
		} else if (strcasecmp(name, "OFFSET") == 0) {
			// Secondary channel offset: Above, Below
			request.wireless.has_offset = 1;
			request.wireless.offset = strcasecmp(value, "Above") == 0 ? 2 : 1;
		} else if (strcasecmp(name, "MCS_FIXEDRATE") == 0) {
			// Depending upon the MODE' parameter, two options - For mode=11na - MCS rate varies from 0 to 31 and
			// For mode=11ac, MCS rate varies from 0 to 9.
			request.wireless.mcs_rate = strtoul(value, NULL, 0);
			request.wireless.has_mcs_rate = request.wireless.mcs_rate != 0;
		} else if (strcasecmp(name, "SPATIAL_RX_STREAM") == 0) {
			// Depending upon the MODE' parameter, two options.
			// For mode=11na - Sets the Rx spacial streams of the AP and
			// which means the Rx MCS Rates capability.
			// For mode=11ac - Sets the Rx spacial streams of the AP.
			// No inter-dependency of number of spatial streams and MCS rates.
			snprintf(request.wireless.nss_rx, sizeof(request.wireless.nss_rx),
				"%s", value);
		} else if (strcasecmp(name, "SPATIAL_TX_STREAM") == 0) {
			// Depending upon the MODE' parameter, two options.
			// For mode=11na - Sets the Tx spacial streams of the AP and which means the Tx MCS Rates capability.
			// For mode=11ac - Sets the Tx spacial streams of the AP. No inter-dependency of number of spatial streams and MCS rates.
			snprintf(request.wireless.nss_tx, sizeof(request.wireless.nss_tx),
				"%s", value);
		} else if (strcasecmp(name, "MPDU_MIN_START_SPACING") == 0) {
			// Minimum MPDU Start Spacing, Short Integer
		} else if (strcasecmp(name, "RIFS_TEST") == 0) {
			// Set Up (Tear Down) RIFS Transmission Test as instructed within Appendix H of the 11n Test Plan
		} else if (strcasecmp(name, "SGI20") == 0) {
			// Short Guard Interval
		} else if (strcasecmp(name, "STBC_TX") == 0) {
			request.wireless.has_stbc_tx = sscanf(value, "%d;%d",
				&request.wireless.stbc_tx[0], &request.wireless.stbc_tx[1]) == 2;
			// STBC Transmit Streams
		} else if (strcasecmp(name, "WIDTH") == 0) {
			// 20, 40, 80,160,Auto
			if (strcasecmp(value, "auto") == 0) {
				request.wireless.bandwidth = 0;
				request.wireless.has_bandwidth = 1;
			} else if (sscanf(value, "%hu", &request.wireless.bandwidth) == 1) {
				request.wireless.has_bandwidth = 1;
			}
		} else if (strcasecmp(name, "WIDTH_SCAN") == 0) {
			// BSS channel width trigger scan interval in seconds
		} else if (strcasecmp(name, "ChannelUsage") == 0) {
			// Set the channel usage, String
		} else if (strcasecmp(name, "DTIM") == 0) {
			// DTIM Count
			request.wireless.has_dtim =
				sscanf(value, "%u", &request.wireless.dtim) == 1;
		} else if (strcasecmp(name, "DYN_BW_SGNL") == 0) {
			// If DYN_BW_SGNL is enabled then the AP sends the RTS frame with dynamic
			// bandwidth signaling, otherwise AP sends RTS with static bandwidth signaling.
			// BW signaling is enabled on the use of this command
			request.wireless.has_dyn_bw_sgnl = 1;
			request.wireless.dyn_bw_sgnl = strcasecmp(value, "Disable") != 0;
		} else if (strcasecmp(name, "SGI80") == 0) {
			// Enable Short guard interval at 80 Mhz. String Enable/Disable
			request.wireless.short_gi = strcasecmp(value, "Disable") != 0;
			request.wireless.has_short_gi = 1;

		} else if (strcasecmp(name, "TxBF") == 0) {
			// To enable or disable SU TxBF beamformer capability with explicit feedback.
			// String: Enable/Disable
			request.wireless.su_beamformer = strcasecmp(value, "Enable") == 0;
			request.wireless.has_su_beamformer = 1;
		} else if (strcasecmp(name, "LDPC") == 0) {
			// Enable the use of LDPC code at the physical layer for both Tx and Rx side.
			// String: Enable/Disable
			request.wireless.ldpc = strcasecmp(value, "Enable") == 0;
			request.wireless.has_ldpc = 1;
		} else if (strcasecmp(name, "BCC") == 0) {
			// Disable the use of LDPC code if BCC is enabled.
			// String: Enable/Disable
			request.wireless.ldpc = strcasecmp(value, "Enable") != 0;
			request.wireless.has_ldpc = 1;
		} else if (strcasecmp(name, "nss_mcs_cap") == 0) {
			// Refers to nss_capabilty;mcs_capability. This parameter gives a description
			// of the supported spatial streams and mcs rate capabilities of the STA
			// String. For example  If a STA supports 2SS with MCS 0-9, then nss_mcs_cap,2;0-9
			request.wireless.has_nss_mcs_cap =
				sscanf(value, "%d;0-%d",
					&request.wireless.nss_cap,
					&request.wireless.mcs_high_cap) == 2;
		} else if (strcasecmp(name, "Tx_lgi_rate") == 0) {
			// Used to set the Tx Highest Supported Long Gi Data Rate subfield
			// Integer
		} else if (strcasecmp(name, "SpectrumMgt") == 0) {
			// Enable/disable Spectrum management feature with minimum number of beacons
			// with switch announcement IE = 10, channel switch mode = 1
			// String: Enable/Disable
		} else if (strcasecmp(name, "Vht_tkip") == 0) {
			// To enable TKIP in VHT mode.
			// String: Enable/Disable
			request.wireless.vht_tkip = strcasecmp(value, "Enable") == 0;
			request.wireless.has_vht_tkip = 1;
		} else if (strcasecmp(name, "Vht_wep") == 0) {
			// To enable WEP in VHT mode.
			// String: Enable/Disable
		} else if (strcasecmp(name, "BW_SGNL") == 0) {
			// To enable/disable the ability to send out RTS with bandwidth signaling
			// String: Enable/Disable
			request.wireless.bw_sgnl = strcasecmp(value, "Enable") == 0;
			request.wireless.has_bw_sgnl = 1;
		} else if (strcasecmp(name, "HTC-VHT") == 0) {
			// Indicates support for receiving a VHT variant HT control field
			// String: Enable/Disable
		} else if (strcasecmp(name, "Zero_crc") == 0) {
			// To set the CRC field to all 0s
			// String: Enable/Disable
		} else if (strcasecmp(name, "CountryCode") == 0) {
			// 2 character country code in Country Information Element
			// String: For Example  US
			snprintf(request.wireless.country_code,
				sizeof(request.wireless.country_code), "%s", value);
		} else if (strcasecmp(name, "Protect_mode") == 0) {
			// To enable/disable the default protection mode between 2 VHT devices
			// i.e turn RTS/CTS and CTS-to-Self on/off
			// String: Enable/Disable
		} else if (strcasecmp(name, "MU_TxBF") == 0) {
			// To enable or disable Multi User (MU) TxBF beamformer capability
			// eith explicit feedback
			// String: Enable/Disable
			request.wireless.mu_beamformer = strcasecmp(value, "Enable") == 0;
			request.wireless.has_mu_beamformer = 1;
		} else if (strcasecmp(name, "GROUP_ID") == 0) {
			// Command to set preconfigured Group ID after SSID is configured.
			// Integer- Range 0 to 63
			request.wireless.group_id = strtol(value, NULL, 10);
			request.wireless.has_group_id = 1;
		} else if (strcasecmp(name, "RTS_FORCE") == 0) {
			// Force STA to send RTS
			// String: Enable/Disable
			request.wireless.rts_force = strcasecmp(value, "Enable") == 0;
			request.wireless.has_rts_force = 1;
		} else if (strcasecmp(name, "MU_NDPA_FrameFormat") == 0) {
			// NDPA Frame Format Settings:
			//	VHT,              TA=0 set parameter as 0
			//	Non-HT-Duplicate, TA=1 set parameter as 1
			//	Non-HT-Duplicate, TA=0 set parameter to 2
			request.wireless.mu_ndpa_format = strtol(value, NULL, 10);
			request.wireless.has_mu_ndpa_format = 1;
		} else if (strcasecmp(name, "NoAck") == 0) {
			request.wireless.noackpolicy = strcasecmp(value, "on") == 0;
			request.wireless.has_noackpolicy = 1;
		} else if (strcasecmp(name, "WLAN_TAG") == 0) {
			// VAP index
			request.wireless.vap_index = strtol(value, NULL, 10) - 1;
		} else if (strcasecmp(name, "MaxHE-MCS_1SS_RxMapLTE80") == 0) {
			request.wireless.maxhe_mcs_1ss_rxmaplte80 = strtol(value, NULL, 10);
			request.wireless.has_maxhe_mcs_1ss_rxmaplte80 = 1;
		} else if (strcasecmp(name, "MaxHE-MCS_2SS_RxMapLTE80") == 0) {
			request.wireless.maxhe_mcs_2ss_rxmaplte80 = strtol(value, NULL, 10);
			request.wireless.has_maxhe_mcs_2ss_rxmaplte80 = 1;
		} else if (strcasecmp(name, "PPDUTxType") == 0) {
			request.wireless.has_ppdutxtype = 1;
			request.wireless.ppdutxtype =
				strcasecmp(value, "MU") == 0 ? CLS_CA_PPDUTXTYPE_MU :
				strcasecmp(value, "ER") == 0 ? CLS_CA_PPDUTXTYPE_ER :
				strcasecmp(value, "TB") == 0 ? CLS_CA_PPDUTXTYPE_TB :
				CLS_CA_PPDUTXTYPE_SU;
		} else if (strcasecmp(name, "OFDMA") == 0) {
			request.wireless.has_ofdma = 1;
			request.wireless.ofdma = strcasecmp(value, "UL") == 0;
			if (strcasecmp(value, "DL-20and80") == 0)
				request.wireless.ofdma = 2;
		} else if (strcasecmp(name, "MIMO") == 0) {
			request.wireless.has_mimo = 1;
			request.wireless.mimo = strcasecmp(value, "UL") == 0;
		} else if (strcasecmp(name, "NumUsersOFDMA") == 0) {
			request.wireless.has_numusersofdma = 1;
			request.wireless.numusersofdma = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "NumSoundDim") == 0) {
			request.wireless.has_numsounddim = 1;
			request.wireless.numsounddim = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "LTF_GI") == 0) {
			request.wireless.has_ltf_gi = 1;
			request.wireless.ltf_gi =
				strcasecmp(value, "2:0.8") == 0 ? CLS_HE_NDP_GI_LTF_TYPE_0 :
				strcasecmp(value, "2:1.6") == 0 ? CLS_HE_NDP_GI_LTF_TYPE_1 :
				strcasecmp(value, "4:3.2") == 0 ? CLS_HE_NDP_GI_LTF_TYPE_2 :
				CLS_HE_NDP_GI_LTF_TYPE_1;
		} else if (strcasecmp(name, "RUAllocTones") == 0) {
			request.wireless.rualloctones_ctr = sscanf(value, "%d:%d:%d:%d",
				&request.wireless.rualloctones[0],
				&request.wireless.rualloctones[1],
				&request.wireless.rualloctones[2],
				&request.wireless.rualloctones[3]);

			request.wireless.has_rualloctones = request.wireless.rualloctones_ctr >= 2;
		} else if (strcasecmp(name, "HE_TXOPDurRTSThr") == 0) {
			request.wireless.has_he_txopdurrtsthr = 1;
			request.wireless.he_txopdurrtsthr = strcasecmp(value, "Enable") == 0;
		} else if (strcasecmp(name, "numnontxbss") == 0) {
			request.wireless.has_numnontxbss = 1;
			request.wireless.numnontxbss = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "mbssid") == 0) {
			request.wireless.has_mbssid = 1;
			request.wireless.mbssid = strcasecmp(value, "Enable") == 0;
		} else if (strcasecmp(name, "nontxbssindex") == 0) {
			request.wireless.has_nontxbssindex = 1;
			request.wireless.nontxbssindex = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "MinMPDUStartSpacing") == 0) {
			request.wireless.has_minmpdustartspacing = 1;
			request.wireless.minmpdustartspacing = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "UnsolicitedProbeResp") == 0) {
			if (strcasecmp(value, "Enable") == 0)
				request.wireless.unsolicitedproberesp = 1;
		} else if (strcasecmp(name, "Cadence_UnsolicitedProbeResp") == 0) {
			request.wireless.cadence_unsolicitedproberesp = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "FT_OA") == 0) {
			request.wireless.has_ieee80211r = 1;
			request.wireless.ft_over_air = strcasecmp(value, "Enable") == 0;
		} else if (strcasecmp(name, "FT_DS") == 0) {
			request.wireless.has_ieee80211r = 1;
			request.wireless.ft_over_ds = strcasecmp(value, "Enable") == 0;
		} else if (strcasecmp(name, "FT_BSS_LIST") == 0) {
			snprintf(request.wireless.ft_bss_list,
				sizeof(request.wireless.ft_bss_list), "%s", value);
		} else if (strcasecmp(name, "DOMAIN") == 0) {
			snprintf(request.wireless.domain,
				sizeof(request.wireless.domain), "%s", value);
		} else if (strcasecmp(name, "Reg_Domain") == 0) {
			snprintf(request.wireless.reg_domain,
				sizeof(request.wireless.reg_domain), "%s", value);
		} else if (strcasecmp(name, "ADDBAReq_BufSize") == 0) {
			request.wireless.has_addba_req_bufsize = 1;
			request.wireless.addba_req_bufsize_gt64 = strcasecmp(value, "gt64") == 0;
		} else if (strcasecmp(name, "BroadcastTWT") == 0) {
			request.wireless.bctwt = strcasecmp(value, "Enable") == 0;
			request.wireless.has_bctwt = 1;
		} else if (strcasecmp(name, "Band6Gonly") == 0) {
			request.wireless.has_band_6G_only = 1;
			request.wireless.band_6G_only = strcasecmp(value, "Enable") == 0;
		} else if (strcasecmp(name, "BSS_Max_Idle") == 0) {
			request.wireless.has_bss_max_idle = 1;
			request.wireless.bss_max_idle_enable = strcasecmp(value, "Enable") == 0;
		} else if (strcasecmp(name, "BSS_Max_Idle_Period") == 0) {
			request.wireless.bss_max_idle_period = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "TWT_RespSupport") == 0) {
			request.wireless.twt_respsupport = strcasecmp(value, "Enable") == 0;
			request.wireless.has_twt_respsupport = 1;
		} else if (strcasecmp(name, "MU_EDCA") == 0) {
			request.wireless.muedca = strcasecmp(value, "override") == 0;
			request.wireless.has_muedca = 1;
		} else if (strcasecmp(name, "ERSUDisable") == 0) {
			request.wireless.ersudisable = strtol(value, NULL, 10);
			request.wireless.has_ersudisable = 1;
		} else if (strcasecmp(name, "OMCtrl_ULMUDataDisableRx") == 0) {
			request.wireless.omctrl_ulmudatadisablerx =
					strcasecmp(value, "enable") == 0;
			request.wireless.has_omctrl_ulmudatadisablerx = 1;
		} else if (strcasecmp(name, "FullBW_ULMUMIMO") == 0) {
			request.wireless.fullbw_ulmumimo = strcasecmp(value, "enable") == 0;
			request.wireless.has_fullbw_ulmumimo = 1;
		}
	}

	legacy_mode_get_band_info(request.wireless.chan_band[0].chan,
		&request.wireless.chan_band[0].band);
	legacy_mode_get_band_info(request.wireless.chan_band[1].chan,
		&request.wireless.chan_band[1].band);

	wfaEncodeTLV(CSIGMA_AP_SET_WIRELESS_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

int cnat_ap_set_security(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };
	char *save;
	char *name;
	char *value;

	// NAME
	// KEYMGNT
	// ----
	// INTERFACE
	// PSK
	// WEPKEY
	// SSID
	// PMF
	// SHA256AD
	// ENCRYPT

	for (name = strtok_r(pcmdStr, ",", &save), value = strtok_r(NULL, ",", &save);
		name && *name && value;
		name = strtok_r(NULL, ",", &save), value = strtok_r(NULL, ",", &save)) {

		if (strcasecmp(name, "NAME") == 0) {
		} else if (strcasecmp(name, "KEYMGNT") == 0) {
			snprintf(request.secutiry.keymgnt, sizeof(request.secutiry.keymgnt),
				"%s", value);
		} else if (strcasecmp(name, "INTERFACE") == 0) {
			if (strcasecmp(value, "5G") != 0 || strcasecmp(value, "6G") != 0) {
				snprintf(request.secutiry.if_name,
					sizeof(request.secutiry.if_name), "%s", value);
			}
		} else if (strcasecmp(name, "PSK") == 0) {
			snprintf(request.secutiry.passphrase, sizeof(request.secutiry.passphrase),
				"%s", value);
		} else if (strcasecmp(name, "WEPKEY") == 0) {
			snprintf(request.secutiry.wepkey, sizeof(request.secutiry.wepkey),
				"%s", value);
		} else if (strcasecmp(name, "SSID") == 0) {
			snprintf(request.secutiry.ssid, sizeof(request.secutiry.ssid), "%s", value);
		} else if (strcasecmp(name, "PMF") == 0) {
			request.secutiry.has_pmf = 1;
			if (strcasecmp(value, "Required") == 0) {
				request.secutiry.pmf = clsapi_pmf_required;
			} else if (strcasecmp(value, "Optional") == 0) {
				request.secutiry.pmf = clsapi_pmf_optional;
			} else if (strcasecmp(value, "Disabled") == 0) {
				request.secutiry.pmf = clsapi_pmf_disabled;
			} else {
				request.secutiry.has_pmf = 0;
			}
		} else if (strcasecmp(name, "SHA256AD") == 0) {
			request.secutiry.has_sha256ad = 1;
			if (strcasecmp(value, "Enabled") == 0) {
				request.secutiry.sha256ad = 1;
			} else if (strcasecmp(value, "Disabled") == 0) {
				request.secutiry.sha256ad = 0;
			} else {
				request.secutiry.has_sha256ad = 0;
			}
		} else if (strcasecmp(name, "ENCRYPT") == 0) {
			snprintf(request.secutiry.encryption, sizeof(request.secutiry.encryption),
				"%s", value);
		} else if (strcasecmp(name, "WLAN_TAG") == 0) {
			// VAP index
			request.secutiry.vap_index = strtol(value, NULL, 10) - 1;
		} else if (strcasecmp(name, "nontxbssindex") == 0) {
			request.secutiry.has_nontxbssindex = 1;
			request.secutiry.nontxbssindex = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "ECGROUPID") == 0) {
			snprintf(request.secutiry.ecc_grps,
					sizeof(request.secutiry.ecc_grps), "%s", value);
		} else if (strcasecmp(name, "SAE_PWE") == 0) {
			snprintf(request.secutiry.sae_pwe, sizeof(request.secutiry.sae_pwe),
				"%s", value);
		} else if (strcasecmp(name, "GroupMgntCipher") == 0) {
			snprintf(request.secutiry.GroupMgntCipher, sizeof(request.secutiry.GroupMgntCipher),
				"%s", value);
		} else if (strcasecmp(name, "AKMSuiteType") == 0) {
			snprintf(request.secutiry.AKMSuiteType,
					sizeof(request.secutiry.AKMSuiteType), "%s", value);
		} else if (strcasecmp(name, "SAEPasswords") == 0) {
			snprintf(request.secutiry.SAEPasswords,
					sizeof(request.secutiry.SAEPasswords), "%s", value);
		} else if (strcasecmp(name, "Transition_Disable") == 0) {
			request.secutiry.transition_disable = strtol(value, NULL, 10);
		} else if (strcasecmp(name, "Transition_Disable_Index") == 0) {
			request.secutiry.transition_disable_index = strtol(value, NULL, 10);
		}
	}

	wfaEncodeTLV(CSIGMA_AP_SET_SECURITY_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

int cnat_ap_ca_version(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };

	wfaEncodeTLV(CSIGMA_AP_CA_VERSION_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

int cnat_ap_set_pmf(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };

	wfaEncodeTLV(CSIGMA_AP_SET_PMF_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

int cnat_ap_reboot(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };

	wfaEncodeTLV(CSIGMA_AP_REBOOT_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

int cnat_ap_deauth_sta(char *pcmdStr, unsigned char *aBuf, int *aLen)
{
	cls_log("run %s", __FUNCTION__);
	struct cls_dut_request request = { {0} };

	wfaEncodeTLV(CSIGMA_AP_DEAUTH_STA_TAG, sizeof(request), (BYTE *) & request, aBuf);

	*aLen = 4 + sizeof(request);

	return WFA_SUCCESS;
}

int cnat_ap_ca_version_resp(unsigned char *cmd_buf)
{
	struct cls_dut_response *resp = (struct cls_dut_response *)(cmd_buf + 4);
	struct cls_ca_version ca_version;

	memcpy(&ca_version, &resp->ca_version, sizeof(ca_version));
	cls_log("run %s", __FUNCTION__);

	sprintf(gRespStr, "status,COMPLETE,version,%s\r\n", ca_version.version);

	wfaCtrlSend(gCaSockfd, (unsigned char *)gRespStr, strlen(gRespStr));
	return WFA_SUCCESS;
}

int cnat_ap_generic_resp(unsigned char *cmd_buf)
{
	struct cls_dut_response *getverResp = (struct cls_dut_response *)(cmd_buf + 4);
	cls_log("%s", __FUNCTION__);

	if (getverResp->status == STATUS_COMPLETE) {
		sprintf(gRespStr, "status,COMPLETE\r\n");
	} else {
		sprintf(gRespStr, "status,ERROR,errorCode,%d\r\n", getverResp->clsapi_error);
	}

	wfaCtrlSend(gCaSockfd, (unsigned char *)gRespStr, strlen(gRespStr));
	return WFA_SUCCESS;
}

int cnat_ap_reboot_resp(unsigned char *cmd_buf)
{
	cls_log("%s", __FUNCTION__);

	sprintf(gRespStr, "status,COMPLETE,after_action,wait_reboot\r\n");

	wfaCtrlSend(gCaSockfd, (unsigned char *)gRespStr, strlen(gRespStr));
	return WFA_SUCCESS;
}

int cls_ca_encode_ap_reset_default(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_RESET_DEFAULT_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_11n_wireless(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_11N_WIRELESS_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_apqos(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_APQOS_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_staqos(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_STAQOS_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_config_commit(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_CONFIG_COMMIT_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_get_mac_address(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_GET_MAC_ADDRESS_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_get_parameter(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_GET_PARAMETER_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_deauth_sta(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_DEAUTH_STA_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_11d(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_11D_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_11h(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_11H_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_rfeature(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_RFEATURE_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_set_hs2(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SET_HS2_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_send_addba_req(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __FUNCTION__);

	return cls_ca_encode_cmd(CSIGMA_AP_SEND_ADDBA_REQ_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_ap_preset_testparameters(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __func__);

	return cls_ca_encode_cmd(CSIGMA_AP_PRESET_TESTPARAMETERS_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_respond_ap(unsigned char *tlv_buf)
{
	int ret;
	char out_buf[512];

	cls_log("%s", __FUNCTION__);

	ret = cls_ca_make_response(tlv_buf, out_buf, sizeof(out_buf));

	if (ret != WFA_SUCCESS)
		return ret;

	wfaCtrlSend(gCaSockfd, (unsigned char *)out_buf, strlen(out_buf));

	return WFA_SUCCESS;
}
