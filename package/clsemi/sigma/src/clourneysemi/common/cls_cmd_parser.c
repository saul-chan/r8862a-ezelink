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
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "cls_cmd_parser.h"
#include "csigma_tags.h"
#include "csigma_log.h"

#define N_ARRAY(arr)			(sizeof(arr)/sizeof(arr[0]))

static
const struct cls_token_desc cls_token_table[] = {
	{CLS_TOK_NAME,         "NAME"},
	{CLS_TOK_PROGRAM,      "PROGRAM"},
	{CLS_TOK_PROG,         "PROG"},
	{CLS_TOK_INTERFACE,    "INTERFACE"},
	{CLS_TOK_TYPE,         "TYPE"},
	{CLS_TOK_SSID,         "SSID"},
	{CLS_TOK_CHANNEL,      "CHANNEL"},
	{CLS_TOK_OPERCHN,	"OPER_CHN"},
	{CLS_TOK_MODE,         "MODE"},
	{CLS_TOK_WME,          "WME"},
	{CLS_TOK_WMMPS,        "WMMPS"},
	{CLS_TOK_RTS,          "RTS"},
	{CLS_TOK_FRGMNT,       "FRGMNT"},
	{CLS_TOK_PWRSAVE,      "PWRSAVE"},
	{CLS_TOK_BCNINT,       "BCNINT"},
	{CLS_TOK_RADIO,        "RADIO"},
	{CLS_TOK_40_INTOLERANT, "40_INTOLERANT"},
	{CLS_TOK_ADDBA_REJECT,  "ADDBA_REJECT"},
	{CLS_TOK_AMPDU,        "AMPDU"},
	{CLS_TOK_AMPDU_EXP,    "AMPDU_EXP"},
	{CLS_TOK_AMSDU,        "AMSDU"},
	{CLS_TOK_GREENFIELD,   "GREENFIELD"},
	{CLS_TOK_OFFSET,       "OFFSET"},
	{CLS_TOK_MCS_32,       "MCS_32"},
	{CLS_TOK_MCS_FIXEDRATE, "MCS_FIXEDRATE"},
	{CLS_TOK_SPATIAL_RX_STREAM, "SPATIAL_RX_STREAM"},
	{CLS_TOK_SPATIAL_TX_STREAM, "SPATIAL_TX_STREAM"},
	{CLS_TOK_MPDU_MIN_START_SPACING, "MPDU_MIN_START_SPACING"},
	{CLS_TOK_RIFS_TEST,    "RIFS_TEST"},
	{CLS_TOK_SGI20,        "SGI20"},
	{CLS_TOK_STBC_TX,      "STBC_TX"},
	{CLS_TOK_WIDTH,        "WIDTH"},
	{CLS_TOK_WIDTH_SCAN,   "WIDTH_SCAN"},
	{CLS_TOK_CWMIN_VO,     "CWMIN_VO"},
	{CLS_TOK_CWMIN_VI,     "CWMIN_VI"},
	{CLS_TOK_CWMIN_BE,     "CWMIN_BE"},
	{CLS_TOK_CWMIN_BK,     "CWMIN_BK"},
	{CLS_TOK_CWMAX_VO,     "CWMAX_VO"},
	{CLS_TOK_CWMAX_VI,     "CWMAX_VI"},
	{CLS_TOK_CWMAX_BE,     "CWMAX_BE"},
	{CLS_TOK_CWMAX_BK,     "CWMAX_BK"},
	{CLS_TOK_AIFS_VO,      "AIFS_VO"},
	{CLS_TOK_AIFS_VI,      "AIFS_VI"},
	{CLS_TOK_AIFS_BE,      "AIFS_BE"},
	{CLS_TOK_AIFS_BK,      "AIFS_BK"},
	{CLS_TOK_TxOP_VO,      "TxOP_VO"},
	{CLS_TOK_TxOP_VO,      "STA_WMMPE_TXOP_VO"},
	{CLS_TOK_TxOP_VI,      "TxOP_VI"},
	{CLS_TOK_TxOP_VI,      "STA_WMMPE_TXOP_VI"},
	{CLS_TOK_TxOP_BE,      "TxOP_BE"},
	{CLS_TOK_TxOP_BE,      "STA_WMMPE_TXOP_BE"},
	{CLS_TOK_TxOP_BK,      "TxOP_BK"},
	{CLS_TOK_TxOP_BK,      "STA_WMMPE_TXOP_BK"},
	{CLS_TOK_ACM_VO,       "ACM_VO"},
	{CLS_TOK_ACM_VI,       "ACM_VI"},
	{CLS_TOK_ACM_BE,       "ACM_BE"},
	{CLS_TOK_ACM_BK,       "ACM_BK"},
	{CLS_TOK_STA_MAC_ADDRESS, "STA_MAC_ADDRESS"},
	{CLS_TOK_MINORCODE,    "MINORCODE"},
	{CLS_TOK_REGULATORY_MODE, "REGULATORY_MODE"},
	{CLS_TOK_COUNTRY_CODE, "CountryCode"},
	{CLS_TOK_DFS_MODE,     "DFS_MODE"},
	{CLS_TOK_DFS_CHAN,     "DFS_CHAN"},
	{CLS_TOK_NSS_MCS_OPT,  "NSS_MCS_OPT"},
	{CLS_TOK_OPT_MD_NOTIF_IE, "OPT_MD_NOTIF_IE"},
	{CLS_TOK_CHNUM_BAND,   "CHNUM_BAND"},
	{CLS_TOK_TID,          "TID"},
	{CLS_TOK_DEST_MAC,     "DEST_MAC"},
	{CLS_TOK_SUPPLICANT,   "SUPPLICANT"},
	{CLS_TOK_PREAMBLE,     "PREAMBLE"},
	{CLS_TOK_POWERSAVE,    "POWERSAVE"},
	{CLS_TOK_NOACK,        "NOACK"},
	{CLS_TOK_IGNORECHSWITCHPROHIBIT, "IGNORECHSWITCHPROHIBIT"},
	{CLS_TOK_TDLS,         "TDLS"},
	{CLS_TOK_TDLSMODE,     "TDLSMODE"},
	{CLS_TOK_WFDDEVTYPE,   "WFDDEVTYPE"},
	{CLS_TOK_UIBC_GEN,     "UIBC_GEN"},
	{CLS_TOK_UIBC_HID,     "UIBC_HID"},
	{CLS_TOK_UI_INPUT,     "UI_INPUT"},
	{CLS_TOK_UIBC_PREPARE, "UIBC_PREPARE"},
	{CLS_TOK_HDCP,         "HDCP"},
	{CLS_TOK_FRAMESKIP,    "FRAMESKIP"},
	{CLS_TOK_AVCHANGE,     "AVCHANGE"},
	{CLS_TOK_STANDBY,      "STANDBY"},
	{CLS_TOK_INPUTCONTENT, "INPUTCONTENT"},
	{CLS_TOK_VIDEOFORMAT,  "VIDEOFORMAT"},
	{CLS_TOK_AUDIOFORMAT,  "AUDIOFORMAT"},
	{CLS_TOK_I2C,          "I2C"},
	{CLS_TOK_VIDEORECOVERY, "VIDEORECOVERY"},
	{CLS_TOK_PREFDISPLAY,  "PREFDISPLAY"},
	{CLS_TOK_SERVICEDISCOVERY, "SERVICEDISCOVERY"},
	{CLS_TOK_3DVIDEO,      "3DVIDEO"},
	{CLS_TOK_MULTITXSTREAM, "MULTITXSTREAM"},
	{CLS_TOK_TIMESYNC,     "TIMESYNC"},
	{CLS_TOK_EDID,         "EDID"},
	{CLS_TOK_COUPLEDCAP,   "COUPLEDCAP"},
	{CLS_TOK_OPTIONALFEATURE, "OPTIONALFEATURE"},
	{CLS_TOK_SESSIONAVAILABILITY, "SESSIONAVAILABILITY"},
	{CLS_TOK_DEVICEDISCOVERABILITY, "DEVICEDISCOVERABILITY"},
	{CLS_TOK_WMM,          "WMM"},
	{CLS_TOK_STBC_RX,      "STBC_RX"},
	{CLS_TOK_MCS32,        "MCS32"},
	{CLS_TOK_SMPS,         "SMPS"},
	{CLS_TOK_TXSP_STREAM,  "TXSP_STREAM"},
	{CLS_TOK_RXSP_STREAM,  "RXSP_STREAM"},
	{CLS_TOK_BAND,         "BAND"},
	{CLS_TOK_DYN_BW_SGNL,  "DYN_BW_SGNL"},
	{CLS_TOK_SGI80,        "SGI80"},
	{CLS_TOK_TXBF,         "TXBF"},
	{CLS_TOK_LDPC,         "LDPC"},
	{CLS_TOK_BCC,          "BCC"},
	{CLS_TOK_NSS_MCS_CAP,  "NSS_MCS_CAP"},
	{CLS_TOK_TX_LGI_RATE,  "TX_LGI_RATE"},
	{CLS_TOK_ZERO_CRC,     "ZERO_CRC"},
	{CLS_TOK_VHT_TKIP,     "VHT_TKIP"},
	{CLS_TOK_VHT_WEP,      "VHT_WEP"},
	{CLS_TOK_BW_SGNL,      "BW_SGNL"},
	{CLS_TOK_PASSPHRASE,   "PASSPHRASE"},
	{CLS_TOK_PASSWORD,     "PASSWORD"},
	{CLS_TOK_KEYMGMTTYPE,  "KEYMGMTTYPE"},
	{CLS_TOK_ENCPTYPE,     "ENCPTYPE"},
	{CLS_TOK_PMF,          "PMF"},
	{CLS_TOK_MICALG,       "MICALG"},
	{CLS_TOK_PREFER,       "PREFER"},
	{CLS_TOK_FRAMENAME,    "FRAMENAME"},
	{CLS_TOK_PROTECTED,    "PROTECTED"},
	{CLS_TOK_SENDER,	"SENDER"},
	{CLS_TOK_STATIONID,    "STATIONID"},
	{CLS_TOK_CHANNEL_WIDTH,"CHANNEL_WIDTH"},
	{CLS_TOK_NSS,          "NSS"},
	{CLS_TOK_BSSID,        "BSSID"},
	{CLS_TOK_MONTH,        "MONTH"},
	{CLS_TOK_DATE,         "DATE"},
	{CLS_TOK_YEAR,         "YEAR"},
	{CLS_TOK_HOURS,        "HOURS"},
	{CLS_TOK_MINUTES,      "MINUTES"},
	{CLS_TOK_SECONDS,      "SECONDS"},
	{CLS_TOK_MAC,          "MAC"},
	{CLS_TOK_MAXSPLENGTH,  "MAXSPLENGTH"},
	{CLS_TOK_ACBE,         "ACBE"},
	{CLS_TOK_ACBK,         "ACBK"},
	{CLS_TOK_ACVI,         "ACVI"},
	{CLS_TOK_ACVO,         "ACVO"},
	{CLS_TOK_MU_TXBF,      "MU_TXBF"},
	{CLS_TOK_RTS_BWS,      "RTS_BWS"},
	{CLS_TOK_ARP,          "ARP"},
	{CLS_TOK_MU_DBG_GROUP_STA0, "MU_DBG_GROUP_STA0"},
	{CLS_TOK_MU_DBG_GROUP_STA1, "MU_DBG_GROUP_STA1"},
	{CLS_TOK_CTS_WIDTH,    "CTS_WIDTH"},
	{CLS_TOK_RTS_FORCE,    "RTS_FORCE"},
	{CLS_TOK_NDPA_STAINFO_MAC, "NDPA_STAINFO_MAC"},
	{CLS_TOK_INTERWORKING, "INTERWORKING"},
	{CLS_TOK_ACCS_NET_TYPE, "ACCS_NET_TYPE"},
	{CLS_TOK_INTERNET, "INTERNET"},
	{CLS_TOK_VENUE_GRP, "VENUE_GRP"},
	{CLS_TOK_VENUE_TYPE, "VENUE_TYPE"},
	{CLS_TOK_HESSID, "HESSID"},
	{CLS_TOK_ROAMING_CONS, "ROAMING_CONS"},
	{CLS_TOK_DGAF_DISABLE, "DGAF_DISABLE"},
	{CLS_TOK_ANQP, "ANQP"},
	{CLS_TOK_NET_AUTH_TYPE, "NET_AUTH_TYPE"},
	{CLS_TOK_NAI_REALM_LIST, "NAI_REALM_LIST"},
	{CLS_TOK_DOMAIN_LIST, "DOMAIN_LIST"},
	{CLS_TOK_OPER_NAME, "OPER_NAME"},
	{CLS_TOK_VENUE_NAME, "VENUE_NAME"},
	{CLS_TOK_GAS_CB_DELAY, "GAS_CB_DELAY"},
	{CLS_TOK_MIH, "MIH"},
	{CLS_TOK_L2_TRAFFIC_INSPECT, "L2_TRAFFIC_INSPECT"},
	{CLS_TOK_BCST_UNCST, "BCST_UNCST"},
	{CLS_TOK_PLMN_MCC, "PLMN_MCC"},
	{CLS_TOK_PLMN_MNC, "PLMN_MNC"},
	{CLS_TOK_PROXY_ARP, "PROXY_ARP"},
	{CLS_TOK_WAN_METRICS, "WAN_METRICS"},
	{CLS_TOK_CONN_CAP, "CONN_CAP"},
	{CLS_TOK_IP_ADD_TYPE_AVAIL, "IP_ADD_TYPE_AVAIL"},
	{CLS_TOK_ICMPV4_ECHO, "ICMPV4_ECHO"},
	{CLS_TOK_CHSWITCHMODE, "CHSWITCHMODE"},
	{CLS_TOK_PEER, "PEER"},
	{CLS_TOK_OFFCHNUM, "OFFCHNUM"},
	{CLS_TOK_SECCHOFFSET, "SECCHOFFSET"},
	{CLS_TOK_UAPSD, "UAPSD"},
	{CLS_TOK_WLAN_TAG, "WLAN_TAG"},
	{CLS_TOK_OSU_SERVER_URI, "OSU_SERVER_URI"},
	{CLS_TOK_OSU_SSID, "OSU_SSID"},
	{CLS_TOK_OSU_PROVIDER_LIST, "OSU_PROVIDER_LIST"},
	{CLS_TOK_QOS_MAP_SET, "QoS_MAP_SET"},
	{CLS_TOK_TXBANDWIDTH, "TXBANDWIDTH"},
	{CLS_TOK_IP, "IP"},
	{CLS_TOK_MASK, "MASK"},
	{CLS_TOK_DHCP, "DHCP"},
	{CLS_TOK_MU_NDPA_FRAMEFORMAT, "MU_NDPA_FRAMEFORMAT"},
	{CLS_TOK_LTF, "LTF"},
	{CLS_TOK_GI, "GI"},
	{CLS_TOK_MAXHE_MCS_1SS_RXMAPLTE80, "MaxHE-MCS_1SS_RxMapLTE80"},
	{CLS_TOK_MAXHE_MCS_2SS_RXMAPLTE80, "MaxHE-MCS_2SS_RxMapLTE80"},
	{CLS_TOK_TRIGGER_TXBF, "Trigger_TxBF"},
	{CLS_TOK_RUALLOCTONES, "RUAllocTones"},
	{CLS_TOK_ACKPOLICY, "AckPolicy"},
	{CLS_TOK_OMI_RXNSS, "OMCtrl_RxNSS"},
	{CLS_TOK_OMI_CHWIDTH, "OMCtrl_ChnlWidth"},
	{CLS_TOK_TRIGGERTYPE, "TriggerType"},
	{CLS_TOK_DISABLETRIGGERTYPE, "DisableTriggerType"},
	{CLS_TOK_TRIG_COMINFO_GI_LTF, "Trig_ComInfo_GI-LTF"},
	{CLS_TOK_TRIG_COMINFO_BW, "Trig_ComInfo_BW"},
	{CLS_TOK_TRIG_USRINFO_RUALLOC, "Trig_UsrInfo_RUAlloc"},
	{CLS_TOK_TRIG_USRINFO_SSALLOC_RA_RU, "Trig_UsrInfo_SSAlloc_RA-RU"},
	{CLS_TOK_TRIGGERCODING, "TriggerCoding"},
	{CLS_TOK_REGULATORY_CLASS, "REGCLASS"},
	{CLS_TOK_RAND_INTERVAL, "RANDINT"},
	{CLS_TOK_MEAS_DURATION, "MEADUR"},
	{CLS_TOK_MEAS_MODE, "MEAMODE"},
	{CLS_TOK_REPORT_DETAIL, "RPTDET"},
	{CLS_TOK_AP_CHAN_REPORT, "APCHANRPT"},
	{CLS_TOK_REQUEST_INFO, "REQINFO"},
	{CLS_TOK_LAST_BEACON_REPORT_INDICATION, "LASTBEACONRPTINDICATION"},
	{CLS_TOK_CANDIDATE_LIST, "CAND_LIST"},
	{CLS_TOK_BTMREQ_DISASSOC_IMNT, "BTMREQ_DISASSOC_IMNT"},
	{CLS_TOK_BTMREQ_TERMINATION_BIT, "BTMREQ_TERM_BIT"},
	{CLS_TOK_BTMREQ_TERMINATION_DUR, "BSS_TERM_DURATION"},
	{CLS_TOK_BTMREQ_TERMINATION_TSF, "BSS_TERM_TSF"},
	{CLS_TOK_ASSOC_DISALLOW, "ASSOC_DISALLOW"},
	{CLS_TOK_NEIGHBOR_BSSID, "NEBOR_BSSID"},
	{CLS_TOK_NEIGHBOR_OP_CLASS, "NEBOR_OP_CLASS"},
	{CLS_TOK_NEIGHBOR_OP_CH, "NEBOR_OP_CH"},
	{CLS_TOK_NEIGHBOR_PREF, "NEBOR_PREF"},
	{CLS_TOK_NONTXBSSINDEX, "NONTXBSSINDEX"},
	{CLS_TOK_HE_TXOPDURRTSTHR, "HE_TXOPDURRTSTHR"},
	{CLS_TOK_PPDUTXTYPE, "PPDUTxType"},
	{CLS_TOK_TRIG_INTERVAL, "TRIG_INTERVAL"},
	{CLS_TOK_NAV_UPDATE, "NAV_Update"},
	{CLS_TOK_TRIG_COMINFO_ULLENGTH, "Trig_ComInfo_ULLength"},
	{CLS_TOK_MPDU_MU_SPACINGFACTOR, "MPDU_MU_SpacingFactor"},
	{CLS_TOK_STA_MUEDCA_ECWMIN_BE, "STA_MUEDCA_ECWmin_BE"},
	{CLS_TOK_STA_MUEDCA_ECWMIN_VI, "STA_MUEDCA_ECWmin_VI"},
	{CLS_TOK_STA_MUEDCA_ECWMIN_VO, "STA_MUEDCA_ECWmin_VO"},
	{CLS_TOK_STA_MUEDCA_ECWMIN_BK, "STA_MUEDCA_ECWmin_BK"},
	{CLS_TOK_STA_MUEDCA_ECWMAX_BE, "STA_MUEDCA_ECWmax_BE"},
	{CLS_TOK_STA_MUEDCA_ECWMAX_VI, "STA_MUEDCA_ECWmax_VI"},
	{CLS_TOK_STA_MUEDCA_ECWMAX_VO, "STA_MUEDCA_ECWmax_VO"},
	{CLS_TOK_STA_MUEDCA_ECWMAX_BK, "STA_MUEDCA_ECWmax_BK"},
	{CLS_TOK_STA_MUEDCA_AIFSN_BE, "STA_MUEDCA_AIFSN_BE"},
	{CLS_TOK_STA_MUEDCA_AIFSN_VI, "STA_MUEDCA_AIFSN_VI"},
	{CLS_TOK_STA_MUEDCA_AIFSN_VO, "STA_MUEDCA_AIFSN_VO"},
	{CLS_TOK_STA_MUEDCA_AIFSN_BK, "STA_MUEDCA_AIFSN_BK"},
	{CLS_TOK_STA_MUEDCA_TIMER_BE, "STA_MUEDCA_Timer_BE"},
	{CLS_TOK_STA_MUEDCA_TIMER_VI, "STA_MUEDCA_Timer_VI"},
	{CLS_TOK_STA_MUEDCA_TIMER_VO, "STA_MUEDCA_Timer_VO"},
	{CLS_TOK_STA_MUEDCA_TIMER_BK, "STA_MUEDCA_Timer_BK"},
	{CLS_TOK_TXOPDUR, "TXOPDur"},
	{CLS_TOK_INTERVAL, "interval"},
	{CLS_TOK_ECGROUPID, "ECGROUPID"},
	{CLS_TOK_DPP_ACTTYPE, "DPPACTIONTYPE"},
	{CLS_TOK_DPP_CRYPTOID, "DPPCRYPTOIDENTIFIER"},
	{CLS_TOK_DPP_BS, "DPPBS"},
	{CLS_TOK_DPP_LISTENCHAN, "DPPLISTENCHANNEL"},
	{CLS_TOK_DPP_BSDATA, "DPPBOOTSTRAPPINGDATA"},
	{CLS_TOK_DPP_AUTHROLE, "DPPAUTHROLE"},
	{CLS_TOK_DPP_CONFIDX, "DPPCONFINDEX"},
	{CLS_TOK_DPP_AUTHDIR, "DPPAUTHDIRECTION"},
	{CLS_TOK_DPP_PROVROLE, "DPPPROVISIONINGROLE"},
	{CLS_TOK_DPP_SIGNKEYECC, "DPPSIGNINGKEYECC"},
	{CLS_TOK_DPP_CONFENROLLEE, "DPPCONFENROLLEE"},
	{CLS_TOK_DPP_TIMEOUT, "DPPTIMEOUT"},
	{CLS_TOK_DPP_WAITCONNECT, "DPPWAITFORCONNECT"},
	{CLS_TOK_DPP_SELFCONFIG, "DPPSELFCONFIGURE"},
	{CLS_TOK_DPP_PKEXCODE, "DPPPKEXCODE"},
	{CLS_TOK_DPP_PKEXCODEIDENTIFIER, "DPPPKEXCODEIDENTIFIER"},
	{CLS_TOK_DPP_CONFENROLLEEROLE, "DPPCONFENROLLEEROLE"},
	{CLS_TOK_DPP2_CHIRP, "DPPCHIRP"},
	{CLS_TOK_DPP2_CHIRP_ITER, "DPPCHIRPITER"},
	{CLS_TOK_DPP2_CHIRP_CHAN, "DPPCHIRPCHANNEL"},
	{CLS_TOK_DPP2_IPADDR, "DPPCONFIGURATORADDRESS"},
	{CLS_TOK_DPP2_PKHASH, "DPPCONFIGURATORPKHASH"},
	{CLS_TOK_DPP2_OVERTCP, "DPPOverTCP"},
	{CLS_TOK_MAINTAIN_PROFILE,    "MAINTAIN_PROFILE"},
	{CLS_TOK_DEV_ROLE, "DEVROLE"},
	{CLS_TOK_PARAMETER, "PARAMETER"},
	{CLS_TOK_RUID, "RUID"},
	{CLS_TOK_BACKHAUL, "BACKHAUL"},
	{CLS_TOK_WPS_ROLE, "WPSROLE"},
	{CLS_TOK_WPS_CONFIG_METHOD, "WPSCONFIGMETHOD"},
	{CLS_TOK_DEST_ALID, "DESTALID"},
	{CLS_TOK_MESSAGE_TYPE_VALUE, "MESSAGETYPEVALUE"},
	{CLS_TOK_TLV_TYPE, "TLV_TYPE"},
	{CLS_TOK_TLV_TYPE1, "TLV_TYPE1"},
	{CLS_TOK_TLV_TYPE2, "TLV_TYPE2"},
	{CLS_TOK_TLV_TYPE3, "TLV_TYPE3"},
	{CLS_TOK_TLV_TYPE4, "TLV_TYPE4"},
	{CLS_TOK_TLV_TYPE5, "TLV_TYPE5"},
	{CLS_TOK_TLV_LENGTH, "TLV_LENGTH"},
	{CLS_TOK_TLV_LENGTH1, "TLV_LENGTH1"},
	{CLS_TOK_TLV_LENGTH2, "TLV_LENGTH2"},
	{CLS_TOK_TLV_LENGTH3, "TLV_LENGTH3"},
	{CLS_TOK_TLV_LENGTH4, "TLV_LENGTH4"},
	{CLS_TOK_TLV_LENGTH5, "TLV_LENGTH5"},
	{CLS_TOK_TLV_VALUE, "TLV_VALUE"},
	{CLS_TOK_TLV_VALUE1, "TLV_VALUE1"},
	{CLS_TOK_TLV_VALUE2, "TLV_VALUE2"},
	{CLS_TOK_TLV_VALUE3, "TLV_VALUE3"},
	{CLS_TOK_TLV_VALUE4, "TLV_VALUE4"},
	{CLS_TOK_TLV_VALUE5, "TLV_VALUE5"},
	{CLS_TOK_BSS_INFO1, "BSS_INFO1"},
	{CLS_TOK_BSS_INFO2, "BSS_INFO2"},
	{CLS_TOK_BSS_INFO3, "BSS_INFO3"},
	{CLS_TOK_BSS_INFO4, "BSS_INFO4"},
	{CLS_TOK_BSS_INFO5, "BSS_INFO5"},
	{CLS_TOK_BSS_INFO6, "BSS_INFO6"},
	{CLS_TOK_BSS_INFO7, "BSS_INFO7"},
	{CLS_TOK_BSS_INFO8, "BSS_INFO8"},
	{CLS_TOK_BSS_INFO9, "BSS_INFO9"},
	{CLS_TOK_BSS_INFO10, "BSS_INFO10"},
	{CLS_TOK_BSS_INFO11, "BSS_INFO11"},
	{CLS_TOK_BSS_INFO12, "BSS_INFO12"},
	{CLS_TOK_ACKTYPE, "ACKTYPE"},
	{CLS_TOK_UNSOLICITEDPROBERESP, "UnsolicitedProbeResp"},
	{CLS_TOK_CADENCE_UNSOLICITEDPROBERESP, "Cadence_UnsolicitedProbeResp"},
	{CLS_TOK_FILSDSCV, "FILSDscv"},
	{CLS_TOK_IE_NAME, "IE_Name"},
	{CLS_TOK_CONTENTS, "Contents"},
	{CLS_TOK_USERNAME, "USERNAME"},
	{CLS_TOK_TRUSTEDROOTCA, "trustedRootCA"},
	{CLS_TOK_CERTTYPE, "CertType"},
	{CLS_TOK_CLIENTCERTIFICATE, "clientCertificate"},
	{CLS_TOK_PAIRWISECIPHER, "PairwiseCipher"},
	{CLS_TOK_GROUPCIPHER, "GroupCipher"},
	{CLS_TOK_GROUPMGNTCIPHER, "GroupMgntCipher"},
	{CLS_TOK_BCTWT, "BroadcastTWT"},
	{CLS_TOK_BCTWT_FLOWTYPE, "FlowType"},
	{CLS_TOK_BCTWT_ID, "BTWT_ID"},
	{CLS_TOK_BCTWT_WAKEDUR, "NominalMinWakeDur"},
	{CLS_TOK_BCTWT_PERSIST, "BTWT_Persistence"},
	{CLS_TOK_BCTWT_RECOMM, "BTWT_Recommendation"},
	{CLS_TOK_BCTWT_TRIGGER, "TWT_Trigger"},
	{CLS_TOK_BCTWT_INTVEXP, "WakeIntervalExp"},
	{CLS_TOK_BCTWT_INTVMAN, "WakeIntervalMantissa"},
	{CLS_TOK_BCTWT_TDOWNALL, "TeardownAllTWT"},
	{CLS_TOK_BCTWT_TWTELEM, "TWTElement"},
	{CLS_TOK_PREAMBLEPUNCTURE, "PreamblePunctMode"},
	{CLS_TOK_PUNCCHANNEL, "PunctChannel"},

};

static
const char *cls_resp_status_table[] = {
	"COMPLETE",
	"ERROR",
	"INVALID",
	"RUNNING"
};

const struct cls_token_desc *cls_lookup_token_by_id(const enum cls_token tok)
{
	int i;

	for (i = 0; i < N_ARRAY(cls_token_table); i++) {
		if (cls_token_table[i].tok_id == tok)
			return &cls_token_table[i];
	}

	return NULL;
}

static
const struct cls_token_desc *cls_lookup_token_by_name(const char *name, int len)
{
	int i;

	if (name && *name && (len > 0)) {
		for (i = 0; i < N_ARRAY(cls_token_table); i++) {
			if (strncasecmp(cls_token_table[i].tok_text, name, len) == 0)
				if (strlen(cls_token_table[i].tok_text) == len)
					return &cls_token_table[i];
		}
	}

	return NULL;
}

static
int cls_parse_params(const char *params_ptr, struct cls_cmd_param *param_tab_ptr,
		int param_tab_size, int *error_count)
{
	const char *delim;
	const char *name_ptr;
	int name_len;
	const char *val_ptr;
	int val_len;
	const struct cls_token_desc *token;
	struct cls_cmd_param *param;
	int param_count = 0;
	int err_count = 0;
	const char *pair = params_ptr;

	while (pair && *pair && (param_count < param_tab_size)) {
		delim = strchr(pair, ',');

		if (!delim)
			break;

		name_ptr = pair;
		name_len = delim - pair;
		val_ptr = delim + 1;

		delim = strchr(val_ptr, ',');

		if (delim) {
			val_len = delim - val_ptr;
			pair = delim + 1;
		} else {
			val_len = strlen(val_ptr);
			pair = NULL;
		}

		while ((name_ptr[0] == ' ') && (name_len > 0)) {
			name_ptr++;
			name_len--;
		}

		if (name_len == 0) {
			err_count++;
			break;
		}

		while ((name_len > 0) && (name_ptr[name_len - 1] == ' ')) {
			name_len--;
		}

		token = cls_lookup_token_by_name(name_ptr, name_len);

		if (token) {
			param = &param_tab_ptr[param_count];
			param->key_tok = token->tok_id;
			param->val_pos = val_ptr - params_ptr;
			param->val_len = val_len;

			param_count++;
		} else
			err_count++;
	}

	if (error_count)
		*error_count = err_count;

	return param_count;
}

int cls_parse_params_encode_request(const char *params, char *buf_ptr, int buf_size)
{
	struct cls_cmd_param param_tab[CLS_CMD_MAX_PARAM_COUNT];
	int param_count;
	int error_count;
	int buf_len = 0;
	int i;
	char key_buf[8];
	int key_len;

	param_count = cls_parse_params(params, param_tab, N_ARRAY(param_tab), &error_count);

	for (i = 0; i < param_count; i++) {
		struct cls_cmd_param *param = &param_tab[i];

		key_len = snprintf(key_buf, sizeof(key_buf), "%03d", (int)param->key_tok);

		if (key_len > 0) {
			if (buf_size > (buf_len + key_len + 1 + param->val_len + 1)) {
				strncpy(buf_ptr + buf_len, key_buf, key_len);
				buf_len += key_len;

				buf_ptr[buf_len] = ',';
				buf_len++;

				strncpy(buf_ptr + buf_len, params + param->val_pos, param->val_len);
				buf_len += param->val_len;

				buf_ptr[buf_len] = ',';
				buf_len++;
			} else
				return -ENOMEM;
		}
	}

	buf_ptr[buf_len] = 0;

	return buf_len;
}

static
const char* cls_search_char(const char *start, const char *end, const char c)
{
	while (start < end) {
		if (*start == c)
			return start;
		start++;
	}

	return NULL;
}


static
int cls_parse_request(const char *req_ptr, int req_len, struct cls_cmd_param *param_tab_ptr,
		int param_tab_size, int *error_count)
{
	const char *delim;
	const char *name_ptr;
	int name_len;
	const char *val_ptr;
	int val_len;
	char key_buf[8];
	long key_tok;
	struct cls_cmd_param *param;
	int param_count = 0;
	int err_count = 0;
	const char *pair = req_ptr;
	const char *req_end = req_ptr + req_len;

	while (pair && (pair < req_end) && (param_count < param_tab_size)) {
		delim = cls_search_char(pair, req_end, ',');

		if (!delim)
			break;

		name_ptr = pair;
		name_len = delim - pair;
		val_ptr = delim + 1;

		delim = cls_search_char(val_ptr, req_end, ',');

		if (delim) {
			val_len = delim - val_ptr;
			pair = delim + 1;
		} else {
			val_len = req_end - val_ptr;
			pair = NULL;
		}

		if ((name_len <= 0) || (name_len >= sizeof(key_buf))) {
			err_count++;
			break;
		}

		strncpy(key_buf, name_ptr, name_len);
		key_buf[name_len] = 0;

		key_tok = strtol(key_buf, NULL, 10);

		if ((key_tok == 0) || (key_tok == LONG_MAX) || (key_tok == LONG_MIN)) {
			err_count++;
			break;
		}

		param = &param_tab_ptr[param_count];
		param->key_tok = (enum cls_token)key_tok;
		param->val_pos = val_ptr - req_ptr;
		param->val_len = val_len;

		param_count++;
	}

	if (error_count)
		*error_count = err_count;

	return param_count;
}

int cls_init_cmd_request(struct cls_cmd_request *cmd_req, int cmd_tag,
		const unsigned char *req_ptr, int req_len)
{
	if (!cmd_req)
		return -EINVAL;

	if (req_ptr && (req_len > 0)) {
		int error_count;

		cmd_req->param_count = cls_parse_request((const char*)req_ptr, req_len,
				cmd_req->param_tab, N_ARRAY(cmd_req->param_tab),
				&error_count);

		cmd_req->req_ptr = (const char*)req_ptr;
		cmd_req->req_len = req_len;
	} else {
		cmd_req->param_count = 0;
		cmd_req->req_ptr = NULL;
		cmd_req->req_len = 0;
	}

	return 0;
}

static
int cls_get_value(const struct cls_cmd_request *cmd_req, enum cls_token tok, const char **val_ptr,
	int *val_len)
{
	int i;
	const struct cls_cmd_param *param;

	if (!cmd_req)
		return -EINVAL;

	for (i = 0; i < cmd_req->param_count; i++) {
		param = &cmd_req->param_tab[i];

		if (param->key_tok == tok) {
			*val_ptr = cmd_req->req_ptr + param->val_pos;
			*val_len = param->val_len;
			return 0;
		}
	}

	return -ENODATA;
}

int cls_get_value_text(const struct cls_cmd_request *cmd_req, enum cls_token tok, char *buf_ptr,
	int buf_size)
{
	const char *val_ptr;
	int val_len;
	int ret;

	if (!buf_ptr || (buf_size <= 0))
		return -EINVAL;

	ret = cls_get_value(cmd_req, tok, &val_ptr, &val_len);

	if (ret != 0)
		return ret;

	if (val_len >= buf_size)
		return -ENOMEM;

	if (val_len > 0)
		strncpy(buf_ptr, val_ptr, val_len);

	buf_ptr[val_len] = 0;

	return val_len;
}

int cls_get_value_int(const struct cls_cmd_request *cmd_req, enum cls_token tok, int *value)
{
	const char *val_ptr;
	int val_len;
	char val_buf[32];
	int ret;

	if (!value)
		return -EINVAL;

	ret = cls_get_value(cmd_req, tok, &val_ptr, &val_len);

	if (ret != 0)
		return ret;

	if (val_len >= sizeof(val_buf))
		return -ENOMEM;

	if (val_len > 0) {
		strncpy(val_buf, val_ptr, val_len);
		val_buf[val_len] = 0;
		*value = strtol(val_buf, NULL, 10);
	}

	return val_len;
}

/*
 * return Enable/Disable value
 */
int cls_get_value_enable(const struct cls_cmd_request *cmd_req, enum cls_token tok, int *enable,
	int *conv_error)
{
	const char *val_ptr;
	int val_len;
	char val_buf[32];
	int ret;

	if (conv_error)
		*conv_error = 0;

	if (!enable)
		return -EINVAL;

	ret = cls_get_value(cmd_req, tok, &val_ptr, &val_len);

	if (ret != 0)
		return ret;

	if (val_len >= sizeof(val_buf))
		return -EINVAL;

	if (val_len > 0) {
		strncpy(val_buf, val_ptr, val_len);
		val_buf[val_len] = 0;

		if (strcasecmp(val_buf, "Enable") == 0)
			*enable = 1;
		else if (strcasecmp(val_buf, "Disable") == 0)
			*enable = 0;
		else {
			/* complain about unrecognized value */
			val_len = -EINVAL;

			if (conv_error)
				*conv_error = -EINVAL;
		}
	}

	return val_len;
}

int cls_validate_response_get_length(const char *buf_ptr, int buf_len)
{
	int i;
	int tok_len = 0;
	int tok_found = 0;
	int resp_len;

	if (!buf_ptr || (buf_len <= 0))
		return -EINVAL;

	for (i = 0; i < N_ARRAY(cls_resp_status_table); i++) {
		tok_len = strlen(cls_resp_status_table[i]);

		if (strncasecmp(buf_ptr, cls_resp_status_table[i], tok_len) == 0) {
			tok_found = 1;
			break;
		}
	}

	if (tok_found) {
		if (buf_len > tok_len) {
			if (buf_ptr[tok_len] == 0) {
				/* received NULL-terminated string */
				return tok_len;

			} else if (buf_ptr[tok_len] == ',') {
				/* there are additional response parameters */
				resp_len = 0;
				while (buf_ptr[resp_len] && (resp_len < buf_len))
					resp_len++;
				return resp_len;
			}
		} else
			return tok_len;
	}

	return -EINVAL;
}

void cls_macstr_to_array(char *macstr, uint8_t *macarray)
{
	char *token;
	int i = 0;

	token = strtok((char*)macstr, ":");
	while (token != NULL && i < 6) {
		macarray[i] = (unsigned char)strtol(token, NULL, 16);
		token = strtok(NULL, ":");
		i++;
	}
}

void cls_data_to_hex(uint8_t *data, uint8_t size, char **hex_str)
{
	int i = 0;
	char *hex_buf = NULL;

	hex_buf = (char *)malloc(size * 2 + 1);
	for (i = 0; i < size; i++) {
		sprintf(hex_buf + i * 2, "%02X", data[i]);
	}
	hex_buf[size * 2] = '\0';
	*hex_str = hex_buf;
	return;
}

