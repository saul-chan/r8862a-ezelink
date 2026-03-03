/**************************************
 * 
 *  Elena Agostini elena.ago@gmail.com
 * 	802.11 Information Elements
 * 
 ***************************************/
#include "CWWTP.h"

/* +++++++++++++++++++ ASSEMBLE +++++++++++++++++++++ */
/* FIXED LEN IE */
CWBool CW80211AssembleIEFrameControl(char * frame, int * offset, int frameType, int frameSubtype) {
	
	short int val = IEEE80211_FC(frameType, frameSubtype);
	
	CW_COPY_MEMORY(frame, &(val), LEN_IE_FRAME_CONTROL);
	(*offset) += LEN_IE_FRAME_CONTROL;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEFrameControlData(char * frame, int * offset, int frameType, int frameSubtype, int toDS, int fromDS) {
		
	short int val = IEEE80211_FC(frameType, frameSubtype);
	if(toDS == 1)
		SETBIT(val,8);
	else
		CLEARBIT(val,8);
		
	if(fromDS == 1)
		SETBIT(val,9);
	else
		CLEARBIT(val,9);

	CW_COPY_MEMORY(frame, &(val), LEN_IE_FRAME_CONTROL);
	(*offset) += LEN_IE_FRAME_CONTROL;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEDuration(char * frame, int * offset, int value) {
	
	short int val = htons(host_to_le16(value));
	
	CW_COPY_MEMORY(frame, &(val), LEN_IE_DURATION);
	(*offset) += LEN_IE_DURATION;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIESequenceNumber(char * frame, int * offset, int value) {
	
	short int val = htons(host_to_le16(value));
	
	CW_COPY_MEMORY(frame, &(val), LEN_IE_SEQ_CTRL);
	(*offset) += LEN_IE_SEQ_CTRL;
	
	return CW_TRUE;
}


CWBool CW80211AssembleIEAddr(char * frame, int * offset, char * value) {
	//Broadcast
	if(value == NULL)
		memset(frame, 0xff, ETH_ALEN);
	else
		CW_COPY_MEMORY(frame, value, ETH_ALEN);
		
	(*offset) += ETH_ALEN;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEBeaconInterval(char * frame, int * offset, short int value) {
	
	short int val = htons(host_to_le16(value));
	
	CW_COPY_MEMORY(frame, &(val), LEN_IE_BEACON_INT);
	(*offset) += LEN_IE_BEACON_INT;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIECapability(char * frame, int * offset, short int value) {

	CW_COPY_MEMORY(frame, &(value), LEN_IE_CAPABILITY);
	(*offset) += LEN_IE_CAPABILITY;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEAuthAlgoNum(char * frame, int * offset, short int value) {

	CW_COPY_MEMORY(frame, &(value), LEN_IE_AUTH_ALG);
	(*offset) += LEN_IE_AUTH_ALG;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEAuthTransNum(char * frame, int * offset, short int value) {

	CW_COPY_MEMORY(frame, &(value), LEN_IE_AUTH_TRANS);
	(*offset) += LEN_IE_AUTH_TRANS;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEStatusCode(char * frame, int * offset, short int value) {

	CW_COPY_MEMORY(frame, &(value), LEN_IE_STATUS_CODE);
	(*offset) += LEN_IE_STATUS_CODE;
	
	return CW_TRUE;
}

CWBool CW80211SetAssociationID(short int * assID) {
	
	srand(time(NULL));
	(*assID) = rand()%256;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEAssID(char * frame, int * offset, short int value) {
	value |= BIT(14);
	value |= BIT(15);
	CW_COPY_MEMORY(frame, &(value), LEN_IE_ASSOCIATION_ID);
	(*offset) += LEN_IE_ASSOCIATION_ID;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEERP(char * frame, int * offset, short int value) {
	//Type
	unsigned char val=IE_TYPE_ERP;	
	CW_COPY_MEMORY(frame, &(val), IE_TYPE_LEN);
	(*offset) += IE_TYPE_LEN;

	//len
	val=1;
	CW_COPY_MEMORY((frame+IE_TYPE_LEN), &(val), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;

	//value
	value=0x00;
	CW_COPY_MEMORY((frame+IE_TYPE_LEN+IE_SIZE_LEN), &(value), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;
	
	return CW_TRUE;
}

/* VARIABLE LEN IE */

CWBool CW80211AssembleIESSID(char * frame, int * offset, char * value) {
	//Type
	unsigned char val=IE_TYPE_SSID;	
	CW_COPY_MEMORY(frame, &(val), IE_TYPE_LEN);
	(*offset) += IE_TYPE_LEN;
	
	//len
	val=strlen(value);
	CW_COPY_MEMORY((frame+IE_TYPE_LEN), &(val), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;
	
	//value
	CW_COPY_MEMORY((frame+IE_TYPE_LEN+IE_SIZE_LEN), value, strlen(value));
	(*offset) += strlen(value);

	return CW_TRUE;
}

CWBool CW80211AssembleEapolHeader(char *frame, int *offset) {
	unsigned char rfc1042_header[LEN_EAPOL_HEADER] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
	CW_COPY_MEMORY(frame, rfc1042_header, LEN_EAPOL_HEADER);
	(*offset) += LEN_EAPOL_HEADER;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolVersion(char *frame, int *offset, char value) {
	CW_COPY_MEMORY(frame, &(value), LEN_EAPOL_VERSION);
	(*offset) += LEN_EAPOL_VERSION;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolType(char *frame, int *offset, char value) {
	CW_COPY_MEMORY(frame, &(value), LEN_EAPOL_TYPE);
	(*offset) += LEN_EAPOL_TYPE;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolLen(char *frame, int *offset, short int value) {
	CW_COPY_MEMORY(frame, &(value), LEN_EAPOL_LEN);
	(*offset) += LEN_EAPOL_LEN;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyType(char *frame, int *offset, char value) {
	CW_COPY_MEMORY(frame, &(value), LEN_EAPOL_KEY_TYPE);
	(*offset) += LEN_EAPOL_KEY_TYPE;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyInfo(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_INFO);
	(*offset) += LEN_KEY_INFO;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyLen(char *frame, int *offset, short int value) {
	CW_COPY_MEMORY(frame, &(value), LEN_KEY_LEN);
	(*offset) += LEN_KEY_LEN;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolReplayCounter(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_REPLAY_COUNTER_LEN);
	(*offset) += LEN_KEY_REPLAY_COUNTER_LEN;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyNonce(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_NONCE);
	(*offset) += LEN_KEY_NONCE;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyIV(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_IV);
	(*offset) += LEN_KEY_IV;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyRSC(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_RSC);
	(*offset) += LEN_KEY_RSC;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyID(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_ID);
	(*offset) += LEN_KEY_ID;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyMIC(char *frame, int *offset, unsigned char *value) {
	CW_COPY_MEMORY(frame, value, LEN_KEY_MIC);
	(*offset) += LEN_KEY_MIC;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyDataLen(char *frame, int *offset, short int value) {
	CW_COPY_MEMORY(frame, &(value), LEN_KEY_DATA_LEN);
	(*offset) += LEN_KEY_DATA_LEN;

	return CW_TRUE;
}

CWBool CW80211AssembleEapolKeyData(char *frame, int *offset, unsigned char *value, int len) {
	CW_COPY_MEMORY(frame, value, len);
	(*offset) += len;

	return CW_TRUE;
}

float mapSupportedRatesValues(float rate, short int mode)
{
	
	if(mode == CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME)
	{
		if(rate == 1)
				return 2;
		if(rate == 2)
				return 4;
		if(rate == 5.5)
				return 11;
		if(rate == 6)
				return 12;
		if(rate == 9)
				return 18;
		if(rate == 11)
				return 22;
		if(rate == 12)
				return 24;
		if(rate == 18)
				return 36;
		if(rate == 22)
				return 44;
		if(rate == 24)
				return 48;
		if(rate == 33)
				return 66;
		if(rate == 36)
				return 72;
		if(rate == 48)
				return 96;
		if(rate == 54)
				return 108;
	}
	
	if(mode == CW_80211_SUPP_RATES_CONVERT_FRAME_TO_VALUE)
	{
		if(rate == 2)
				return 1;
		if(rate == 4)
				return 2;
		if(rate == 11)
				return 5.5;
		if(rate == 12)
				return 6;
		if(rate == 18)
				return 9;
		if(rate == 22)
				return 11;
		if(rate == 24)
				return 12;
		if(rate == 36)
				return 18;
		if(rate == 44)
				return 22;
		if(rate == 48)
				return 24;
		if(rate == 66)
				return 33;
		if(rate == 72)
				return 36;
		if(rate == 96)
				return 48;
		if(rate == 108)
				return 54;
	}
	
	return -1;
}

CWBool CW80211AssembleIESupportedRates(char * frame, int * offset, char * value, int numRates) {
	
	char val=IE_TYPE_SUPP_RATES;	
	CW_COPY_MEMORY(frame, &(val), IE_TYPE_LEN);
	(*offset) += IE_TYPE_LEN;
	
	if(numRates <= 0)
		return CW_FALSE;
		
	CW_COPY_MEMORY((frame+IE_TYPE_LEN), &(numRates), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;
	/*int i=0;
	for(i=0; i<numRates; i++)
	{
		//CWLog("value prima: %u %x", value[i],value[i]);
		value[i] = (char)(((int)value[i])+128);
		//CWLog("value dopo: %u %x", value[i], value[i]);		
	}*/
	CW_COPY_MEMORY((frame+IE_TYPE_LEN+IE_SIZE_LEN), value, numRates);
	(*offset) += numRates;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEExtendedSupportedRates(char * frame, int * offset, char * value, int numRates) {
	
	char val=IE_TYPE_EXT_SUPP_RATES;	
	CW_COPY_MEMORY(frame, &(val), IE_TYPE_LEN);
	(*offset) += IE_TYPE_LEN;
	
	if(numRates <= 0)
		return CW_FALSE;
		
	CW_COPY_MEMORY((frame+IE_TYPE_LEN), &(numRates), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;
	
	CW_COPY_MEMORY((frame+IE_TYPE_LEN+IE_SIZE_LEN), value, numRates);
	(*offset) += numRates;
	
	return CW_TRUE;
}

CWBool CW80211AssembleIEDSSS(char * frame, int * offset, char value) {
	
	char val=IE_TYPE_DSSS;	
	CW_COPY_MEMORY(frame, &(val), IE_TYPE_LEN);
	(*offset) += IE_TYPE_LEN;
	
	val=1;
	CW_COPY_MEMORY((frame+IE_TYPE_LEN), &(val), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;
	
	CW_COPY_MEMORY((frame+IE_TYPE_LEN+IE_SIZE_LEN), &(value), 1);
	(*offset) += 1;

	return CW_TRUE;
}

CWBool CW80211AssembleIEMaxIdlePeriod(char * frame, int * offset, short int value) {
	
	char val=IE_TYPE_BSS_MAX_IDLE_PERIOD;	
	CW_COPY_MEMORY(frame, &(val), IE_TYPE_LEN);
	(*offset) += IE_TYPE_LEN;
	
	val=300;
	CW_COPY_MEMORY((frame+IE_TYPE_LEN), &(val), IE_SIZE_LEN);
	(*offset) += IE_SIZE_LEN;
	
/*
		unsigned int val;
		*pos++ = WLAN_EID_BSS_MAX_IDLE_PERIOD;
		*pos++ = 3;
		val = hapd->conf->ap_max_inactivity;
		if (val > 68000)
			val = 68000;
		val *= 1000;
		val /= 1024;
		if (val == 0)
			val = 1;
		if (val > 65535)
			val = 65535;
		WPA_PUT_LE16(pos, val);
		pos += 2;
		*pos++ = 0x00; // TODO: Protected Keep-Alive Required
 */
	CW_COPY_MEMORY((frame+IE_TYPE_LEN+IE_SIZE_LEN), &(value), 1);
	(*offset) += 1;

	return CW_TRUE;
}

//802.3
CWBool CW8023AssembleHdrLength(char * frame, int * offset, short int value) {

	CW_COPY_MEMORY(frame, &(value), 2);
	(*offset) += 2;
	
	return CW_TRUE;
}
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* ------------------ PARSE ---------------------- */
CWBool CW80211ParseFrameIEControl(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_FRAME_CONTROL);
	(*offsetFrameReceived) += LEN_IE_FRAME_CONTROL;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEDuration(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_DURATION);
	(*offsetFrameReceived) += LEN_IE_DURATION;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEAddr(char * frameReceived, int * offsetFrameReceived, unsigned char * addr) {
	
	CW_COPY_MEMORY(addr, frameReceived, ETH_ALEN);
	(*offsetFrameReceived) += ETH_ALEN;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIESeqCtrl(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_SEQ_CTRL);
	(*offsetFrameReceived) += LEN_IE_SEQ_CTRL;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEStatusCode(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_STATUS_CODE);
	(*offsetFrameReceived) += LEN_IE_STATUS_CODE;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEReasonCode(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_REASON_CODE);
	(*offsetFrameReceived) += LEN_IE_REASON_CODE;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEAuthAlgo(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_AUTH_ALG);
	(*offsetFrameReceived) += LEN_IE_AUTH_ALG;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEAuthTransaction(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_AUTH_TRANS);
	(*offsetFrameReceived) += LEN_IE_AUTH_TRANS;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIECapability(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_CAPABILITY);
	(*offsetFrameReceived) += LEN_IE_CAPABILITY;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEAssID(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_ASSOCIATION_ID);
	(*offsetFrameReceived) += LEN_IE_ASSOCIATION_ID;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEListenInterval(char * frameReceived, int * offsetFrameReceived, short int * value) {
	
	CW_COPY_MEMORY(value,frameReceived, LEN_IE_LISTEN_INTERVAL);
	(*offsetFrameReceived) += LEN_IE_LISTEN_INTERVAL;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIESSID(char * frameReceived, int * offsetFrameReceived, char ** value) {
	
	if(frameReceived[0] != IE_TYPE_SSID)
		return CW_FALSE;
	
	(*offsetFrameReceived)++;
	
	short int len = frameReceived[1];
	if(len == 0)
		return CW_FALSE;
		
	(*offsetFrameReceived)++;
	
	CW_CREATE_ARRAY_CALLOC_ERR((*value), len+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return CW_FALSE;});
	CW_COPY_MEMORY((*value),&(frameReceived[2]), len);
	(*offsetFrameReceived) += len;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIESupportedRates(char * frameReceived, int * offsetFrameReceived, char ** value, int * lenIE) {
	
	if(frameReceived[0] != IE_TYPE_SUPP_RATES)
		return CW_FALSE;
	
	(*offsetFrameReceived)++;
	
	short int len = frameReceived[1];
	if(len == 0)
		return CW_FALSE;
	
	(*offsetFrameReceived)++;	

	(*lenIE) = len;
	
	CW_CREATE_ARRAY_CALLOC_ERR((*value), len+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return CW_FALSE;});
	CW_COPY_MEMORY((*value),&(frameReceived[2]), len);
	(*offsetFrameReceived) += len;
	
	return CW_TRUE;
}

CWBool CW80211ParseFrameIEExtendedSupportedRates(char * frameReceived, int * offsetFrameReceived, char ** value, int * lenIE) {
	
	if(frameReceived[0] != IE_TYPE_EXT_SUPP_RATES)
		return CW_FALSE;
	
	(*offsetFrameReceived)++;
	
	short int len = frameReceived[1];
	if(len == 0)
		return CW_FALSE;
	
	(*offsetFrameReceived)++;	

	(*lenIE) = len;
	
	CW_CREATE_ARRAY_CALLOC_ERR((*value), len+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return CW_FALSE;});
	CW_COPY_MEMORY((*value),&(frameReceived[2]), len);
	(*offsetFrameReceived) += len;
	
	return CW_TRUE;
}


/*
 * Scan ieee80211 frame body arguments
 */
ParseRes ieee802_11_parse_elems(const u8 *start, size_t len,
				struct ieee802_11_elems *elems,
				int show_errors)
{
	size_t left = len;
	const u8 *pos = start;
	int unknown = 0;

	os_memset(elems, 0, sizeof(*elems));

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			if (show_errors) {
				CWLog("IEEE 802.11 element "
					   "parse failed (id=%d elen=%d "
					   "left=%lu)",
					   id, elen, (unsigned long) left);
				//wpa_hexdump(MSG_MSGDUMP, "IEs", start, len);
			}
			return ParseFailed;
		}

		switch (id) {
		case WLAN_EID_SSID:
			elems->ssid = pos;
			elems->ssid_len = elen;
			CWLog("SSID[0]: %c", elems->ssid[0]);
			break;
		case WLAN_EID_SUPP_RATES:
			elems->supp_rates = pos;
			elems->supp_rates_len = elen;
			CWLog("SUPP RATES[0]: %c", elems->supp_rates[0]);
			break;
		case WLAN_EID_DS_PARAMS:
			elems->ds_params = pos;
			elems->ds_params_len = elen;
			break;
/*	
		case WLAN_EID_CF_PARAMS:
		case WLAN_EID_TIM:
			break;
		case WLAN_EID_CHALLENGE:
			elems->challenge = pos;
			elems->challenge_len = elen;
			break;
		case WLAN_EID_ERP_INFO:
			elems->erp_info = pos;
			elems->erp_info_len = elen;
			break;
		case WLAN_EID_EXT_SUPP_RATES:
			elems->ext_supp_rates = pos;
			elems->ext_supp_rates_len = elen;
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (ieee802_11_parse_vendor_specific(pos, elen,
							     elems,
							     show_errors))
				unknown++;
			break;
		
		case WLAN_EID_RSN:
			elems->rsn_ie = pos;
			elems->rsn_ie_len = elen;
			break;
		case WLAN_EID_PWR_CAPABILITY:
			break;
		case WLAN_EID_SUPPORTED_CHANNELS:
			elems->supp_channels = pos;
			elems->supp_channels_len = elen;
			break;
		case WLAN_EID_MOBILITY_DOMAIN:
			elems->mdie = pos;
			elems->mdie_len = elen;
			break;
		case WLAN_EID_FAST_BSS_TRANSITION:
			elems->ftie = pos;
			elems->ftie_len = elen;
			break;
		case WLAN_EID_TIMEOUT_INTERVAL:
			elems->timeout_int = pos;
			elems->timeout_int_len = elen;
			break;
		case WLAN_EID_HT_CAP:
			elems->ht_capabilities = pos;
			elems->ht_capabilities_len = elen;
			break;
		case WLAN_EID_HT_OPERATION:
			elems->ht_operation = pos;
			elems->ht_operation_len = elen;
			break;
		case WLAN_EID_VHT_CAP:
			elems->vht_capabilities = pos;
			elems->vht_capabilities_len = elen;
			break;
		case WLAN_EID_VHT_OPERATION:
			elems->vht_operation = pos;
			elems->vht_operation_len = elen;
			break;
		case WLAN_EID_VHT_OPERATING_MODE_NOTIFICATION:
			if (elen != 1)
				break;
			elems->vht_opmode_notif = pos;
			break;
		case WLAN_EID_LINK_ID:
			if (elen < 18)
				break;
			elems->link_id = pos;
			break;
		case WLAN_EID_INTERWORKING:
			elems->interworking = pos;
			elems->interworking_len = elen;
			break;
		case WLAN_EID_QOS_MAP_SET:
			if (elen < 16)
				break;
			elems->qos_map_set = pos;
			elems->qos_map_set_len = elen;
			break;
		case WLAN_EID_EXT_CAPAB:
			elems->ext_capab = pos;
			elems->ext_capab_len = elen;
			break;
		case WLAN_EID_BSS_MAX_IDLE_PERIOD:
			if (elen < 3)
				break;
			elems->bss_max_idle_period = pos;
			break;
		case WLAN_EID_SSID_LIST:
			elems->ssid_list = pos;
			elems->ssid_list_len = elen;
			break;
*/
		default:
			unknown++;
			if (!show_errors)
				break;
			CWLog("IEEE 802.11 element parse "
				   "ignored unknown element (id=%d elen=%d)",
				   id, elen);
			break;
		}

		left -= elen;
		pos += elen;
	}

	if (left)
		return ParseFailed;

	return unknown ? ParseUnknown : ParseOK;
}

/* ----------------------------------------------- */

/* +++++++++++++++++++++ ASSEMBLE +++++++++++++++++++++++ */
//Genera beacon frame
char * CW80211AssembleBeacon(WTPBSSInfo * WTPBSSInfoPtr, int *offset) {

	char * beaconFrame;
	CW_CREATE_ARRAY_CALLOC_ERR(beaconFrame, (MGMT_FRAME_FIXED_LEN_BEACON+MGMT_FRAME_IE_FIXED_LEN+strlen(WTPBSSInfoPtr->interfaceInfo->SSID)+1), char, { CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;}); //MAC80211_HEADER_FIXED_LEN+MAC80211_BEACON_BODY_MANDATORY_MIN_LEN+2+strlen(interfaceInfo->SSID)+10+1), char, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	(*offset)=0;
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(beaconFrame[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_BEACON))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(beaconFrame[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte. Broadcast
	if(!CW80211AssembleIEAddr(&(beaconFrame[(*offset)]), offset, NULL))
			return NULL;
	
	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(beaconFrame[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->MACaddr))
			return NULL;

	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(beaconFrame[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->BSSID))
			return NULL;
	
	//2 (sequence ctl) + 8 (timestamp): vengono impostati in automatico
	(*offset) += LEN_IE_SEQ_CTRL;
	(*offset) += LEN_IE_TIMESTAMP;
	
	//beacon interval: 2 byte
	if(!CW80211AssembleIEBeaconInterval(&(beaconFrame[(*offset)]), offset, 100))
			return NULL;
	
	//capability: 2 byte
	if(!CW80211AssembleIECapability(&(beaconFrame[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->capabilityBit))
			return NULL;
			
	//SSID
	if(!CW80211AssembleIESSID(&(beaconFrame[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->SSID))
		return NULL;
	
	return beaconFrame;
}

//Genera probe response
char * CW80211AssembleProbeResponse(WTPBSSInfo * WTPBSSInfoPtr, struct CWFrameProbeRequest *request, int *offset)
{
	if(request == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble Probe response per SSID: %s", WTPBSSInfoPtr->interfaceInfo->ifName);
	(*offset)=0;
	/* ***************** PROBE RESPONSE FRAME FIXED ******************** */
	char * frameProbeResponse;
	CW_CREATE_ARRAY_CALLOC_ERR(frameProbeResponse, MGMT_FRAME_FIXED_LEN_PROBE_RESP+MGMT_FRAME_IE_FIXED_LEN*3+strlen(WTPBSSInfoPtr->interfaceInfo->SSID)+(CW_80211_MAX_SUPP_RATES*2)+1+1+3, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameProbeResponse[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_PROBE_RESP))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameProbeResponse[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(request)
	{
		if(!CW80211AssembleIEAddr(&(frameProbeResponse[(*offset)]), offset, request->SA))
			return NULL;
	}
	else
	{
		if(!CW80211AssembleIEAddr(&(frameProbeResponse[(*offset)]), offset, NULL))
			return NULL;
	}

	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(frameProbeResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->MACaddr))
			return NULL;

	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(frameProbeResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->BSSID))
			return NULL;
	
	//2 (sequence ctl) + 8 (timestamp): vengono impostati in automatico
	(*offset) += LEN_IE_SEQ_CTRL;
	(*offset) += LEN_IE_TIMESTAMP;
	
	//beacon interval: 2 byte
	if(!CW80211AssembleIEBeaconInterval(&(frameProbeResponse[(*offset)]), offset, 100))
			return NULL;
	
	//capability: 2 byte
	if(!CW80211AssembleIECapability(&(frameProbeResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->capabilityBit))
			return NULL;

	/* *************************************************** */
		
	//SSID
	if(!CW80211AssembleIESSID(&(frameProbeResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->SSID))
		return NULL;
	
	//Supported Rates
	int indexRates=0;
	unsigned char suppRate[request->supportedRatesLen];
	for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < request->supportedRatesLen; indexRates++)
	{
		suppRate[indexRates] =  (char) request->supportedRates[indexRates];
		if(
			suppRate[indexRates] == 2 ||
			suppRate[indexRates] == 4 ||
			suppRate[indexRates] == 11 ||
			suppRate[indexRates] == 22
		)
			suppRate[indexRates] += 128;
		CWLog("supp rate1: %d - rate2: %d", request->supportedRates[indexRates], suppRate[indexRates]);
	}

	if(!CW80211AssembleIESupportedRates(&(frameProbeResponse[(*offset)]), offset, suppRate, indexRates))
		return NULL;
		
	CWLog("Ext Rate Len: %d", request->extSupportedRatesLen);
	if(request->extSupportedRatesLen > 0)
	{
		indexRates=0;
		unsigned char extSuppRate[request->extSupportedRatesLen];
		for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < request->extSupportedRatesLen; indexRates++)
		{
			extSuppRate[indexRates] =  (char) request->extSupportedRates[indexRates];
			CWLog("ext rate1: %d - rate2: %d", request->extSupportedRates[indexRates], extSuppRate[indexRates]);
		}
		
		if(!CW80211AssembleIEExtendedSupportedRates(&(frameProbeResponse[(*offset)]), offset, extSuppRate, indexRates))
		return NULL;
	}
	/*
	//Supported Rates
	int indexRates=0;
	unsigned char suppRate[CW_80211_MAX_SUPP_RATES];
	for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < WTPBSSInfoPtr->phyInfo->lenSupportedRates; indexRates++)
		suppRate[indexRates] = (char) mapSupportedRatesValues(WTPBSSInfoPtr->phyInfo->phyMbpsSet[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
	
	if(!CW80211AssembleIESupportedRates(&(frameProbeResponse[(*offset)]), offset, suppRate, indexRates))
		return NULL;
*/
	//DSSS
	unsigned char channel = CW_WTP_DEFAULT_RADIO_CHANNEL+1;
	if(!CW80211AssembleIEDSSS(&(frameProbeResponse[(*offset)]), offset, channel))
		return NULL;
	
	//ERP7
	//aggiungi+3
	if(!CW80211AssembleIEERP(&(frameProbeResponse[(*offset)]), offset, 0x04))
		return CW_FALSE;
		
	return frameProbeResponse;
}

//Genera auth response
char * CW80211AssembleAuthResponse(char * addrAP, struct CWFrameAuthRequest *request, int *offset)
{
	if(request == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble Auth response");
	(*offset)=0;

	/* ***************** FRAME FIXED ******************** */
	char * frameAuthResponse;
	CW_CREATE_ARRAY_CALLOC_ERR(frameAuthResponse, MGMT_FRAME_FIXED_LEN_AUTH, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameAuthResponse[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_AUTH))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameAuthResponse[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(request)
	{
		if(!CW80211AssembleIEAddr(&(frameAuthResponse[(*offset)]), offset, request->SA))
			return NULL;
	}
	else
	{
		if(!CW80211AssembleIEAddr(&(frameAuthResponse[(*offset)]), offset, NULL))
			return NULL;
	}
	
	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAuthResponse[(*offset)]), offset, addrAP))
			return NULL;
	
	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAuthResponse[(*offset)]), offset, addrAP))
			return NULL;
	
	//2 (sequence ctl)
	(*offset) += LEN_IE_SEQ_CTRL;
	
	//Auth Algorithm Number: 2 byte
	if(!CW80211AssembleIEAuthAlgoNum(&(frameAuthResponse[(*offset)]), offset, IE_AUTH_OPEN_SYSTEM))
			return NULL;

	//Auth Algorithm Number: 2 byte (valore seq: 2)
	if(!CW80211AssembleIEAuthTransNum(&(frameAuthResponse[(*offset)]), offset, 2))
		return NULL;

	//Status Code: 2 byte (valore: 0 status code success)
	if(!CW80211AssembleIEStatusCode(&(frameAuthResponse[(*offset)]), offset, IE_STATUS_CODE_SUCCESS))
		return NULL;
	
	/* ************************************************* */
		
	return frameAuthResponse;
}


//Genera association response
char * CW80211AssembleAssociationResponse(WTPBSSInfo * WTPBSSInfoPtr, WTPSTAInfo * thisSTA, struct CWFrameAssociationRequest *request, int *offset)
{
	if(request == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble Association response");
	
	(*offset)=0;
	
	/* ***************** FRAME FIXED ******************** */
	char * frameAssociationResponse;
	CW_CREATE_ARRAY_CALLOC_ERR(frameAssociationResponse, MGMT_FRAME_FIXED_LEN_ASSOCIATION+MGMT_FRAME_IE_FIXED_LEN*3+(2*CW_80211_MAX_SUPP_RATES)+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameAssociationResponse[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_ASSOC_RESP))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameAssociationResponse[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(request)
	{
		if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, request->SA))
			return NULL;
	}
	else
	{
		if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, NULL))
			return NULL;
	}
	
	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->MACaddr))
			return NULL;
	
	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->BSSID))
			return NULL;
	
	//2 (sequence ctl)
	(*offset) += LEN_IE_SEQ_CTRL;
	
	//capability: 2 byte
	if(!CW80211AssembleIECapability(&(frameAssociationResponse[(*offset)]), offset, thisSTA->/*WTPBSSInfoPtr->interfaceInfo->*/capabilityBit))
			return NULL;
	/* ************************************************* */

	//Status Code: 2 byte (valore: 0 status code success)
	if(!CW80211AssembleIEStatusCode(&(frameAssociationResponse[(*offset)]), offset, IE_STATUS_CODE_SUCCESS))
		return NULL;
	
	//Association ID: 2 byte
	if(!CW80211AssembleIEAssID(&(frameAssociationResponse[(*offset)]), offset, thisSTA->staAID))
		return NULL;
	
	//Supported Rates
	//TODO: Capability Re-set? Use STA capability value?
	int indexRates=0;
	unsigned char suppRate[thisSTA->lenSupportedRates];
	for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < thisSTA->lenSupportedRates; indexRates++)
	{
		suppRate[indexRates] =  (char) thisSTA->supportedRates[indexRates]; //(char) mapSupportedRatesValues(thisSTA->supportedRates[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
		if(
			suppRate[indexRates] == 2 ||
			suppRate[indexRates] == 4 ||
			suppRate[indexRates] == 11 ||
			suppRate[indexRates] == 22
		)
			suppRate[indexRates] += 128;
			
		CWLog("sup rate1: %d - rate2: %d", thisSTA->supportedRates[indexRates], suppRate[indexRates]);
	}
		//(char) thisSTA->supportedRates[indexRates];//(char) mapSupportedRatesValues(WTPBSSInfoPtr->phyInfo->phyMbpsSet[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
	
/*	unsigned char suppRate[CW_80211_MAX_SUPP_RATES];
	for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < WTPBSSInfoPtr->phyInfo->lenSupportedRates; indexRates++)
		suppRate[indexRates] = (char) mapSupportedRatesValues(WTPBSSInfoPtr->phyInfo->phyMbpsSet[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
*/	
	if(!CW80211AssembleIESupportedRates(&(frameAssociationResponse[(*offset)]), offset, suppRate, indexRates))
		return NULL;
		
	
	if(thisSTA->extSupportedRatesLen > 0)
	{
		indexRates=0;
		unsigned char extSuppRate[thisSTA->extSupportedRatesLen];
		for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < thisSTA->extSupportedRatesLen; indexRates++)
		{
			extSuppRate[indexRates] =  (char) thisSTA->extSupportedRates[indexRates]; //(char) mapSupportedRatesValues(thisSTA->supportedRates[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
			CWLog("ext rate1: %d - rate2: %d", thisSTA->extSupportedRates[indexRates], extSuppRate[indexRates]);
		}
		
		if(!CW80211AssembleIEExtendedSupportedRates(&(frameAssociationResponse[(*offset)]), offset, extSuppRate, indexRates))
		return NULL;
	}
	
	/*
	 * idle timeout
	if(!CW80211AssembleIESupportedRates(&(frameAssociationResponse[(*offset)]), offset, suppRate, indexRates))
		return NULL;
	*/

	return frameAssociationResponse;
}

//Genera reassociation response
char * CW80211AssembleReassociationResponse(WTPBSSInfo * WTPBSSInfoPtr, WTPSTAInfo * thisSTA, struct CWFrameAssociationRequest *request, int *offset)
{
	if(request == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble Association response");
	
	(*offset)=0;
	
	/* ***************** FRAME FIXED ******************** */
	char * frameAssociationResponse;
	CW_CREATE_ARRAY_CALLOC_ERR(frameAssociationResponse, MGMT_FRAME_FIXED_LEN_ASSOCIATION+MGMT_FRAME_IE_FIXED_LEN*3+CW_80211_MAX_SUPP_RATES+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameAssociationResponse[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_REASSOC_RESP))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameAssociationResponse[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(request)
	{
		if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, request->SA))
			return NULL;
	}
	else
	{
		if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, NULL))
			return NULL;
	}
	
	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->MACaddr))
			return NULL;
	
	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, WTPBSSInfoPtr->interfaceInfo->BSSID))
			return NULL;
	
	//2 (sequence ctl)
	(*offset) += LEN_IE_SEQ_CTRL;
	
	//capability: 2 byte
	if(!CW80211AssembleIECapability(&(frameAssociationResponse[(*offset)]), offset, thisSTA->/*WTPBSSInfoPtr->interfaceInfo->*/capabilityBit))
			return NULL;
	/* ************************************************* */

	//Status Code: 2 byte (valore: 0 status code success)
	if(!CW80211AssembleIEStatusCode(&(frameAssociationResponse[(*offset)]), offset, IE_STATUS_CODE_SUCCESS))
		return NULL;
	
	//Association ID: 2 byte
	if(!CW80211AssembleIEAssID(&(frameAssociationResponse[(*offset)]), offset, thisSTA->staAID))
		return NULL;
	
	//Supported Rates
	int indexRates=0;
	unsigned char suppRate[thisSTA->lenSupportedRates];
	for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < thisSTA->lenSupportedRates; indexRates++)
	{
		suppRate[indexRates] =  (char) thisSTA->supportedRates[indexRates]; //(char) mapSupportedRatesValues(thisSTA->supportedRates[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
		if(
			suppRate[indexRates] == 2 ||
			suppRate[indexRates] == 4 ||
			suppRate[indexRates] == 11 ||
			suppRate[indexRates] == 22
		)
			suppRate[indexRates] += 128;
			
		CWLog("rate1: %d - rate2: %d", thisSTA->supportedRates[indexRates], suppRate[indexRates]);
	}
	
	if(!CW80211AssembleIESupportedRates(&(frameAssociationResponse[(*offset)]), offset, suppRate, indexRates))
		return NULL;
		
		
	if(thisSTA->extSupportedRatesLen > 0)
	{
		indexRates=0;
		unsigned char extSuppRate[thisSTA->extSupportedRatesLen];
		for(indexRates=0; indexRates < WTP_NL80211_BITRATE_NUM && indexRates < CW_80211_MAX_SUPP_RATES && indexRates < thisSTA->extSupportedRatesLen; indexRates++)
		{
			extSuppRate[indexRates] =  (char) thisSTA->extSupportedRates[indexRates]; //(char) mapSupportedRatesValues(thisSTA->supportedRates[indexRates], CW_80211_SUPP_RATES_CONVERT_VALUE_TO_FRAME);
			CWLog("rate1: %d - rate2: %d", thisSTA->extSupportedRates[indexRates], extSuppRate[indexRates]);
		}
		
		if(!CW80211AssembleIEExtendedSupportedRates(&(frameAssociationResponse[(*offset)]), offset, extSuppRate, indexRates))
		return NULL;
	}
	
	return frameAssociationResponse;
}

char * CW80211AssembleAssociationResponseAC(unsigned char * MACAddr, unsigned char * BSSID,  short int capabilityBit, short int staAID, unsigned char * suppRate, int suppRatesLen, struct CWFrameAssociationRequest *request, int *offset)
{
	if(request == NULL || BSSID == NULL || MACAddr == NULL || suppRate == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble Association response AC side");
	
	(*offset)=0;
	
	/* ***************** FRAME FIXED ******************** */
	char * frameAssociationResponse;
	CW_CREATE_ARRAY_CALLOC_ERR(frameAssociationResponse, MGMT_FRAME_FIXED_LEN_ASSOCIATION+MGMT_FRAME_IE_FIXED_LEN+CW_80211_MAX_SUPP_RATES+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameAssociationResponse[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_ASSOC_RESP))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameAssociationResponse[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, request->SA))
		return NULL;
	
	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, MACAddr))
			return NULL;

	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, BSSID))
			return NULL;

	//2 (sequence ctl)
	(*offset) += LEN_IE_SEQ_CTRL;
	
	//capability: 2 byte
	if(!CW80211AssembleIECapability(&(frameAssociationResponse[(*offset)]), offset, capabilityBit))
			return NULL;
	/* ************************************************* */

	//Status Code: 2 byte (valore: 0 status code success)
	if(!CW80211AssembleIEStatusCode(&(frameAssociationResponse[(*offset)]), offset, IE_STATUS_CODE_SUCCESS))
		return NULL;
	
	//Association ID: 2 byte
	if(!CW80211AssembleIEAssID(&(frameAssociationResponse[(*offset)]), offset, staAID))
		return NULL;
	
	if(suppRatesLen > 0)
	{
		//Supported Rates
		if(!CW80211AssembleIESupportedRates(&(frameAssociationResponse[(*offset)]), offset, suppRate, suppRatesLen))
			return NULL;
	}
	
	return frameAssociationResponse;
}

char * CW80211AssembleReassociationResponseAC(unsigned char * MACAddr, unsigned char * BSSID,  short int capabilityBit, short int staAID, unsigned char * suppRate, int suppRatesLen, struct CWFrameAssociationRequest *request, int *offset)
{
	if(request == NULL || BSSID == NULL || MACAddr == NULL || suppRate == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble Reassociation response AC side");
	
	(*offset)=0;
	
	/* ***************** FRAME FIXED ******************** */
	char * frameAssociationResponse;
	CW_CREATE_ARRAY_CALLOC_ERR(frameAssociationResponse, MGMT_FRAME_FIXED_LEN_ASSOCIATION+MGMT_FRAME_IE_FIXED_LEN+CW_80211_MAX_SUPP_RATES+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameAssociationResponse[(*offset)]), offset, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_REASSOC_RESP))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameAssociationResponse[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, request->SA))
		return NULL;
	
	//sa: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, MACAddr))
			return NULL;

	//bssid: 6 byte
	if(!CW80211AssembleIEAddr(&(frameAssociationResponse[(*offset)]), offset, BSSID))
			return NULL;

	//2 (sequence ctl)
	(*offset) += LEN_IE_SEQ_CTRL;
	
	//capability: 2 byte
	if(!CW80211AssembleIECapability(&(frameAssociationResponse[(*offset)]), offset, capabilityBit))
			return NULL;
	/* ************************************************* */

	//Status Code: 2 byte (valore: 0 status code success)
	if(!CW80211AssembleIEStatusCode(&(frameAssociationResponse[(*offset)]), offset, IE_STATUS_CODE_SUCCESS))
		return NULL;
	
	//Association ID: 2 byte
	if(!CW80211AssembleIEAssID(&(frameAssociationResponse[(*offset)]), offset, staAID))
		return NULL;
	
	//Supported Rates
	if(!CW80211AssembleIESupportedRates(&(frameAssociationResponse[(*offset)]), offset, suppRate, suppRatesLen))
		return NULL;
	
	return frameAssociationResponse;
}

char *CW80211AssembleEapol1AC(char *addr, char *key, int key_len, int *offset){
	short int len = 0;
	unsigned char keyinfo[LEN_KEY_INFO] = {0x00, 0x8a};
	unsigned char replaycounter[LEN_KEY_REPLAY_COUNTER_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
	unsigned char keyNonce[LEN_KEY_NONCE] = {0xb4, 0xf9, 0x93, 0x28, 0xcf, 0xc0, 0xfc, 0x2e, 0xd0, 0x7d, 0xac, 0xe2, 0xe0, 0xa6, 0xaa, 0x10,
		0xa6, 0x7d, 0x24, 0x0a, 0x25, 0x53, 0x7d, 0xf9, 0x72, 0xbe, 0xab, 0xb8, 0xea, 0xf0, 0x38, 0x4f};
	unsigned char keyIv[LEN_KEY_IV] = {0x00};
	unsigned char keyRsc[LEN_KEY_RSC] = {0x00};
	unsigned char keyId[LEN_KEY_ID] = {0x00};
	unsigned char keyMic[LEN_KEY_MIC] = {0x00};

	CWLog("[CW80211] Assemble EAPOL1 test code");
	(*offset)=0;

	/* ***************** FRAME FIXED ******************** */
	char *frameEapol;
	len = LEN_EAPOL_HEADER+ETH_ALEN+LEN_EAPOL_FIX_LEN+LEN_EAPOL_KEY_INFO_FIX_LEN+key_len+2+1;
	CW_CREATE_ARRAY_CALLOC_ERR(frameEapol, len, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});

	//frame eapol header: 6 bytes
	if(!CW80211AssembleEapolHeader(&(frameEapol[(*offset)]), offset))
		goto fail;
	//frame sta addr: 6 bytes
	if(!CW80211AssembleIEAddr(&(frameEapol[(*offset)]), offset, addr))
		goto fail;
	//frame version: 1 byte
	if(!CW80211AssembleEapolVersion(&(frameEapol[(*offset)]), offset, EAPOL_VERSION))
		goto fail;
	//frame type: 1 byte
	if(!CW80211AssembleEapolType(&(frameEapol[(*offset)]), offset, EAPOL_TYPE))
		goto fail;
	//frame len: 2 bytes
	len = LEN_EAPOL_KEY_INFO_FIX_LEN + key_len;
	if(!CW80211AssembleEapolLen(&(frameEapol[(*offset)]), offset, bswap_16(len)))
		goto fail;
	//frame key type: 1 byte
	if(!CW80211AssembleEapolKeyType(&(frameEapol[(*offset)]), offset, EAPOL_KEY_TYPE))
		goto fail;
	//frame key info: 2 bytes
	if(!CW80211AssembleEapolKeyInfo(&(frameEapol[(*offset)]), offset, keyinfo))
		goto fail;
	//frame key len: 2 bytes
	len = 16;
	if(!CW80211AssembleEapolKeyLen(&(frameEapol[(*offset)]), offset, bswap_16(len)))
		goto fail;
	//frame key replay counter: 8 bytes
	if(!CW80211AssembleEapolReplayCounter(&(frameEapol[(*offset)]), offset, replaycounter))
		goto fail;
	//frame key nonce: 32 bytes
	if(!CW80211AssembleEapolKeyNonce(&(frameEapol[(*offset)]), offset, keyNonce))
		goto fail;
	//frame key IV: 16 bytes
	if(!CW80211AssembleEapolKeyIV(&(frameEapol[(*offset)]), offset, keyIv))
		goto fail;
	//frame key RSC: 8 bytes
	if(!CW80211AssembleEapolKeyRSC(&(frameEapol[(*offset)]), offset, keyRsc))
		goto fail;
	//frame key ID: 8 bytes
	if(!CW80211AssembleEapolKeyID(&(frameEapol[(*offset)]), offset, keyId))
		goto fail;
	//frame key MIC: 16 bytes
	if(!CW80211AssembleEapolKeyMIC(&(frameEapol[(*offset)]), offset, keyMic))
		goto fail;
	//frame key data len: 2 bytes
	len = 0;
	CW80211AssembleEapolKeyDataLen(&(frameEapol[(*offset)]), offset, bswap_16(len));
	CWDump(frameEapol, *offset);
	return frameEapol;
fail:
	CW_FREE_OBJECT(frameEapol);
	return NULL;
}

char *CW80211AssembleEapol3AC(char *addr, char *key, int key_len, int *offset) {
	short int len = 0;
	unsigned char keyinfo[LEN_KEY_INFO] = {0x13, 0xca};
	unsigned char replaycounter[LEN_KEY_REPLAY_COUNTER_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
	unsigned char keyNonce[LEN_KEY_NONCE] = {0xb4, 0xf9, 0x93, 0x28, 0xcf, 0xc0, 0xfc, 0x2e, 0xd0, 0x7d, 0xac, 0xe2, 0xe0, 0xa6, 0xaa, 0x10,
		0xa6, 0x7d, 0x24, 0x0a, 0x25, 0x53, 0x7d, 0xf9, 0x72, 0xbe, 0xab, 0xb8, 0xea, 0xf0, 0x38, 0x4f};
	unsigned char keyIv[LEN_KEY_IV] = {0x00};
	unsigned char keyRsc[LEN_KEY_RSC] = {0x00};
	unsigned char keyId[LEN_KEY_ID] = {0x00};
	unsigned char keyMic[LEN_KEY_MIC] = {0xbf, 0xb0, 0xbc, 0x2d, 0x25, 0x23, 0x4b, 0x40, 0xc6, 0x9a, 0x8f, 0x03, 0xeb, 0x23, 0xb0, 0x21};
	unsigned char data[56] = {0x42, 0x9a, 0xd2, 0xcc, 0x14, 0x25, 0x72, 0x98, 0x16, 0x15, 0x50, 0x44, 0x6c, 0x10, 0x4e, 0x56,
		0x9e, 0xd9, 0x49, 0x9a, 0x67, 0xdf, 0xdd, 0xa0, 0xfd, 0x25, 0x50, 0xce, 0x8a, 0x5c, 0x80, 0xbe,
		0x67, 0x9f, 0x60, 0xfb, 0x94, 0x5e, 0x77, 0x46, 0x60, 0x0c, 0x67, 0x8f, 0xff, 0xfa, 0xb1, 0xc5,
		0x7a, 0xcf, 0x42, 0xcc, 0xdd, 0x76, 0x99, 0xb2};

	CWLog("[CW80211] Assemble EAPOL3 test code");
	(*offset)=0;

	/* ***************** FRAME FIXED ******************** */
	char *frameEapol;
	len = LEN_EAPOL_HEADER+ETH_ALEN+LEN_EAPOL_FIX_LEN+LEN_EAPOL_KEY_INFO_FIX_LEN+key_len+2+56+1;
	CW_CREATE_ARRAY_CALLOC_ERR(frameEapol, len, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});

	//frame eapol header: 6 bytes
	if(!CW80211AssembleEapolHeader(&(frameEapol[(*offset)]), offset))
		goto fail;
	//frame sta addr: 6 bytes
	if(!CW80211AssembleIEAddr(&(frameEapol[(*offset)]), offset, addr))
		goto fail;
	//frame version: 1 byte
	if(!CW80211AssembleEapolVersion(&(frameEapol[(*offset)]), offset, EAPOL_VERSION))
		goto fail;
	//frame type: 1 byte
	if(!CW80211AssembleEapolType(&(frameEapol[(*offset)]), offset, EAPOL_TYPE))
		goto fail;
	//frame len: 2 bytes
	len = LEN_EAPOL_KEY_INFO_FIX_LEN + key_len;
	if(!CW80211AssembleEapolLen(&(frameEapol[(*offset)]), offset, bswap_16(len)))
		goto fail;
	//frame key type: 1 byte
	if(!CW80211AssembleEapolKeyType(&(frameEapol[(*offset)]), offset, EAPOL_KEY_TYPE))
		goto fail;
	//frame key info: 2 bytes
	if(!CW80211AssembleEapolKeyInfo(&(frameEapol[(*offset)]), offset, keyinfo))
		goto fail;
	//frame key len: 2 bytes
	len = 16;
	if(!CW80211AssembleEapolKeyLen(&(frameEapol[(*offset)]), offset, bswap_16(len)))
		goto fail;
	//frame key replay counter: 8 bytes
	if(!CW80211AssembleEapolReplayCounter(&(frameEapol[(*offset)]), offset, replaycounter))
		goto fail;
	//frame key nonce: 32 bytes
	if(!CW80211AssembleEapolKeyNonce(&(frameEapol[(*offset)]), offset, keyNonce))
		goto fail;
	//frame key IV: 16 bytes
	if(!CW80211AssembleEapolKeyIV(&(frameEapol[(*offset)]), offset, keyIv))
		goto fail;
	//frame key RSC: 8 bytes
	if(!CW80211AssembleEapolKeyRSC(&(frameEapol[(*offset)]), offset, keyRsc))
		goto fail;
	//frame key ID: 8 bytes
	if(!CW80211AssembleEapolKeyID(&(frameEapol[(*offset)]), offset, keyId))
		goto fail;
	//frame key MIC: 16 bytes
	if(!CW80211AssembleEapolKeyMIC(&(frameEapol[(*offset)]), offset, keyMic))
		goto fail;
	//frame key data len: 2 bytes
	len = 56;
	CW80211AssembleEapolKeyDataLen(&(frameEapol[(*offset)]), offset, bswap_16(len));
	CW80211AssembleEapolKeyData(&(frameEapol[(*offset)]), offset, data, len);
	CWDump(frameEapol, *offset);
	return frameEapol;
fail:
	CW_FREE_OBJECT(frameEapol);
	return NULL;
}

char *  CW80211AssembleACK(WTPBSSInfo * WTPBSSInfoPtr, char * DA, int *offset) {
	if(DA == NULL)
		return NULL;
		
	CWLog("[CW80211] Assemble ACK response per SSID: %s", WTPBSSInfoPtr->interfaceInfo->ifName);
	(*offset)=0;
	/* ***************** PROBE RESPONSE FRAME FIXED ******************** */
	char * frameACK;
	CW_CREATE_ARRAY_CALLOC_ERR(frameACK, DATA_FRAME_FIXED_LEN_ACK+1, char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControl(&(frameACK[(*offset)]), offset, WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_ACK))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameACK[(*offset)]), offset, 0))
		return NULL;
	
	//da: 6 byte
	if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, DA))
		return NULL;
	
	return frameACK;
}

unsigned char *  CW80211AssembleDataFrameHdr(unsigned char * SA, unsigned char * DA, unsigned char * BSSID, short int seqctl, int *offset, int toDS, int fromDS) {
	if(DA == NULL || SA == NULL)
		return NULL;
	
//	CWLog("****** 802.11 FRAME HDR ******");
	(*offset)=0;

	unsigned char * frameACK;
	CW_CREATE_ARRAY_CALLOC_ERR(frameACK, HLEN_80211, unsigned char, {CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); return NULL;});
	
	//frame control: 2 byte
	if(!CW80211AssembleIEFrameControlData(&(frameACK[(*offset)]), offset, WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA, toDS, fromDS))
		return NULL;
	
	//duration: 2 byte
	if(!CW80211AssembleIEDuration(&(frameACK[(*offset)]), offset, 0))
		return NULL;
	
	if(toDS == 1 && fromDS == 0)
	{
		//BSSID: 6 byte
		if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, BSSID))
			return NULL;
/*		if(BSSID != NULL)
			CWLog("** BSSID: %02x:%02x:%02x:%02x:%02x", (int)BSSID[0], (int)BSSID[1], (int)BSSID[2], (int)BSSID[3], (int)BSSID[4], (int)BSSID[5]);
*/
		//SA: 6 byte
		if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, SA))
			return NULL;
//		CWLog("** SA: %02x:%02x:%02x:%02x:%02x", (int)SA[0], (int)SA[1], (int)SA[2], (int)SA[3], (int)SA[4], (int)SA[5]);
		
		//DA: 6 byte
		if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, DA))
			return NULL;
//		CWLog("** DA: %02x:%02x:%02x:%02x:%02x", (int)DA[0], (int)DA[1], (int)DA[2], (int)DA[3], (int)DA[4], (int)DA[5]);
	}
	else if(fromDS == 1 && toDS == 0)
	{
		//DA: 6 byte
		if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, DA))
			return NULL;
//		CWLog("** DA: %02x:%02x:%02x:%02x:%02x", (int)DA[0], (int)DA[1], (int)DA[2], (int)DA[3], (int)DA[4], (int)DA[5]);
		
		//BSSID: 6 byte
		if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, BSSID))
			return NULL;
		
/*		if(BSSID != NULL)
			CWLog("** BSSID: %02x:%02x:%02x:%02x:%02x", (int)BSSID[0], (int)BSSID[1], (int)BSSID[2], (int)BSSID[3], (int)BSSID[4], (int)BSSID[5]);
*/
		//SA: 6 byte
		if(!CW80211AssembleIEAddr(&(frameACK[(*offset)]), offset, SA))
			return NULL;
//		CWLog("** SA: %02x:%02x:%02x:%02x:%02x", (int)SA[0], (int)SA[1], (int)SA[2], (int)SA[3], (int)SA[4], (int)SA[5]);
	}
	else 
		return NULL;
	
	//CWLog("SeqCtl: %d nhtons: %d", seqctl, ntohs(seqctl));
	//2 (sequence ctl)
	if(!CW80211AssembleIESequenceNumber(&(frameACK[(*offset)]), offset, seqctl))
		return NULL;
		
	return frameACK;
}
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* -------------------- PARSE -------------------- */
CWBool CW80211ParseProbeRequest(char * frame, struct CWFrameProbeRequest * probeRequest) {
	int offset=0;
	
	if(probeRequest == NULL)
		return CW_FALSE;
	
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(probeRequest->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(probeRequest->duration)))
		return CW_FALSE;
		
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), probeRequest->DA))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), probeRequest->SA))
		return CW_FALSE;
		
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), probeRequest->BSSID))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(probeRequest->seqCtrl)))
		return CW_FALSE;
	
	//Add parsing variable elements
	if(!CW80211ParseFrameIESSID((frame+offset), &(offset), &(probeRequest->SSID)))
		return CW_FALSE;
		
	//Supported Rates
	if(!CW80211ParseFrameIESupportedRates((frame+offset), &(offset), &(probeRequest->supportedRates),  &(probeRequest->supportedRatesLen)))
		return CW_FALSE;
		
	//Extended Supported Rates
	if(!CW80211ParseFrameIEExtendedSupportedRates((frame+offset), &(offset), &(probeRequest->extSupportedRates),  &(probeRequest->extSupportedRatesLen)))
		probeRequest->extSupportedRatesLen=0;
		
	return CW_TRUE;
}

CWBool CW80211ParseAuthRequest(char * frame, struct CWFrameAuthRequest * authRequest) {
	int offset=0;
	
	if(authRequest == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(authRequest->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(authRequest->duration)))
		return CW_FALSE;
		
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), authRequest->DA))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), authRequest->SA))
		return CW_FALSE;
		
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), authRequest->BSSID))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(authRequest->seqCtrl)))
		return CW_FALSE;
	
	//Auth Algo
	if(!CW80211ParseFrameIEAuthAlgo((frame+offset), &(offset), &(authRequest->authAlg)))
		return CW_FALSE;
		
	//Auth Trans
	if(!CW80211ParseFrameIEAuthTransaction((frame+offset), &(offset), &(authRequest->authTransaction)))
		return CW_FALSE;

	//Status Code
	if(!CW80211ParseFrameIEStatusCode((frame+offset), &(offset), &(authRequest->statusCode)))
		return CW_FALSE;
	
	return CW_TRUE;
}

CWBool CW80211ParseAuthResponse(char * frame, struct CWFrameAuthResponse * authResponse) {
	int offset=0;
	
	if(authResponse == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(authResponse->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEDuration((frame+offset), &(offset), &(authResponse->duration)))
		return CW_FALSE;
	
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), &(authResponse->DA)))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), authResponse->SA))
		return CW_FALSE;
		
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), authResponse->BSSID))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(authResponse->seqCtrl)))
		return CW_FALSE;
	
	//Auth Algo
	if(!CW80211ParseFrameIEAuthAlgo((frame+offset), &(offset), &(authResponse->authAlg)))
		return CW_FALSE;
		
	//Auth Trans
	if(!CW80211ParseFrameIEAuthTransaction((frame+offset), &(offset), &(authResponse->authTransaction)))
		return CW_FALSE;

	//Status Code
	if(!CW80211ParseFrameIEStatusCode((frame+offset), &(offset), &(authResponse->statusCode)))
		return CW_FALSE;
	
	return CW_TRUE;
}

CWBool CW80211ParseAssociationRequest(char * frame, struct CWFrameAssociationRequest * assocRequest) {
	int offset=0;
	
	if(assocRequest == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(assocRequest->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(assocRequest->duration)))
		return CW_FALSE;
		
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), assocRequest->DA))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), assocRequest->SA))
		return CW_FALSE;
		
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), assocRequest->BSSID))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(assocRequest->seqCtrl)))
		return CW_FALSE;
	
	//Capability	
	if(!CW80211ParseFrameIECapability((frame+offset), &(offset), &(assocRequest->capabilityBit)))
		return CW_FALSE;
	
	//Listen Interval	
	if(!CW80211ParseFrameIEListenInterval((frame+offset), &(offset), &(assocRequest->listenInterval)))
		return CW_FALSE;
	
	//SSID		
	if(!CW80211ParseFrameIESSID((frame+offset), &(offset), &(assocRequest->SSID)))
		return CW_FALSE;
	
	//Supported Rates
	if(!CW80211ParseFrameIESupportedRates((frame+offset), &(offset), &(assocRequest->supportedRates),  &(assocRequest->supportedRatesLen)))
		return CW_FALSE;

	//Extended Supported Rates
	if(!CW80211ParseFrameIEExtendedSupportedRates((frame+offset), &(offset), &(assocRequest->extSupportedRates),  &(assocRequest->extSupportedRatesLen)))
		assocRequest->extSupportedRatesLen=0;

	return CW_TRUE;
}

CWBool CW80211ParseAssociationResponse(char * frame, struct CWFrameAssociationResponse * assocResponse) {
	int offset=0;
	
	if(assocResponse == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(assocResponse->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(assocResponse->duration)))
		return CW_FALSE;
		
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), assocResponse->DA))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), assocResponse->SA))
		return CW_FALSE;
		
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), assocResponse->BSSID))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(assocResponse->seqCtrl)))
		return CW_FALSE;
	
	//Capability	
	if(!CW80211ParseFrameIECapability((frame+offset), &(offset), &(assocResponse->capabilityBit)))
		return CW_FALSE;
	
	//Status Code	
	if(!CW80211ParseFrameIEStatusCode((frame+offset), &(offset), &(assocResponse->statusCode)))
		return CW_FALSE;
		
	//Ass ID	
	if(!CW80211ParseFrameIEAssID((frame+offset), &(offset), &(assocResponse->assID)))
		return CW_FALSE;
	
	//Supported Rates
	if(!CW80211ParseFrameIESupportedRates((frame+offset), &(offset), &(assocResponse->supportedRates),  &(assocResponse->supportedRatesLen)))
		return CW_FALSE;

	
	return CW_TRUE;
}

CWBool CW80211ParseDeauthDisassociationRequest(char * frame, struct CWFrameDeauthDisassociationRequest * disassocRequest) {
	int offset=0;
	
	if(disassocRequest == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(disassocRequest->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(disassocRequest->duration)))
		return CW_FALSE;
		
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), disassocRequest->DA))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), disassocRequest->SA))
		return CW_FALSE;
		
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), disassocRequest->BSSID))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(disassocRequest->seqCtrl)))
		return CW_FALSE;
	
	//Reason Code
	if(!CW80211ParseFrameIEReasonCode((frame+offset), &(offset), &(disassocRequest->reasonCode)))
		return CW_FALSE;

	return CW_TRUE;
}

CWBool CW80211ParseDataFrameToDS(char * frame, struct CWFrameDataHdr * dataFrame) {
	int offset=0;
	
	if(dataFrame == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(dataFrame->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(dataFrame->duration)))
		return CW_FALSE;
	
	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), dataFrame->BSSID))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), dataFrame->SA))
		return CW_FALSE;
	
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), dataFrame->DA))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(dataFrame->seqCtrl)))
		return CW_FALSE;
		
	CWLog("Parse seqctl: %d, ntohs: %d", dataFrame->seqCtrl, ntohs(dataFrame->seqCtrl));

	return CW_TRUE;
}


CWBool CW80211ParseDataFrameFromDS(char * frame, struct CWFrameDataHdr * dataFrame) {
	int offset=0;
	
	if(dataFrame == NULL)
		return CW_FALSE;
		
	//Frame Control
	if(!CW80211ParseFrameIEControl(frame, &(offset), &(dataFrame->frameControl)))
		return CW_FALSE;
	
	//Duration
	if(!CW80211ParseFrameIEControl((frame+offset), &(offset), &(dataFrame->duration)))
		return CW_FALSE;
	
	//DA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), dataFrame->DA))
		return CW_FALSE;

	//BSSID
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), dataFrame->BSSID))
		return CW_FALSE;
	
	//SA
	if(!CW80211ParseFrameIEAddr((frame+offset), &(offset), dataFrame->SA))
		return CW_FALSE;
	
	//Seq Ctrl
	if(!CW80211ParseFrameIESeqCtrl((frame+offset), &(offset), &(dataFrame->seqCtrl)))
		return CW_FALSE;

	return CW_TRUE;
}

int isEAPOL_Frame(unsigned char *buf){
	unsigned char rfc1042_header[LEN_EAPOL_HEADER] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
	int i;

	for(i=0; i<6; i++) {
		if(rfc1042_header[i]!=buf[i + HLEN_80211])
			return 0;
	}
	return 1;
}

int isEAPOL_FrameNoHeader(unsigned char *buf) {
	unsigned char rfc1042_header[LEN_EAPOL_HEADER] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
	int i;

	for(i=0; i<6; i++) {
		if(rfc1042_header[i]!=buf[i])
			return 0;
	}
	return 1;
}

int isEAPOL4(unsigned char *buf){
	unsigned char key_info[LEN_KEY_INFO] = {0x03, 0x0a};
	int i;

	for(i=0; i<LEN_KEY_INFO; i++) {
		if(key_info[i]!=buf[i])
			return 0;
	}
	return 1;
}

