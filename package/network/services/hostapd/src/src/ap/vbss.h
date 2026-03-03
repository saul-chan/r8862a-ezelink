/*
 * hostapd / VBSS definition
 * Copyright (c) 2023, Clourney Semi
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
#ifndef VBSS_H
#define VBSS_H


#include "wpa_auth.h"


/***************************	CLS VBSS ctrl iface cmds	************************/

#define HOSTAPD_REPLY_OK	"OK\n"		// ends with '\n', no '\0'
#define HOSTAPD_REPLY_FAIL	"FAIL\n"	// ends with '\n', no '\0'


/* Request format:	"CLS add_vbss_vap\0" + <bin of struct vbss_vap_info>
 * Reply format:
 *	success: string of ifname which new created
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_ADD_VBSS_VAP	"add_vbss_vap "

/* Request format:	"CLS get_vbss_vap\0" + <vap mac addr>
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_GET_VBSS_VAP	"get_vbss_vap "

/* DEL_VBSS_VAP cmd format:
 *		"CLS del_vbss_vap \0"
 * reply format:
 *		success: HOSTAPD_REPLY_OK
 *		fail:    HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_DEL_VBSS_VAP	"del_vbss_vap "

/* Request format:	"CLS add_vbss_sta\0" + <bin of struct vbss_sta_info>
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_ADD_VBSS_STA	"add_vbss_sta "

/* DEL_VBSS_STA cmd format:
 *		"CLS del_vbss_sta <STA MAC in format xx:xx:xx:xx:xx:xx>\0"
 * reply format:
 *		success: HOSTAPD_REPLY_OK
 *		fail:    HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_DEL_VBSS_STA	"del_vbss_sta "

/* Request format:	"CLS get_vbss_enabled <phy name>"
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_GET_VBSS_ENABLED	"get_vbss_enabled "

/* Request format:	"CLS set_vbss_enabled <phy name> <0 | 1>"
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_SET_VBSS_ENABLED	"set_vbss_enabled "

/* Request format:	"CLS get_vbss_sta <STA MAC in aa:bb...>"
 * reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_GET_VBSS_STA	"get_vbss_sta "

/* Request format:	"CLS trg_vbss_switch <STA mac in aa:bb...>"
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_TRG_VBSS_SWITCH	"trg_vbss_switch "

/* Request format:	"CLS set_vbss_switch_done <STA mac in aa:bb...>"
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_SET_VBSS_SWITCH_DONE	"set_vbss_switch_done "

/* Request format:	"CLS disable_vap_rekey\0" + <vap mac addr>
 * Reply format:
 *	success: HOSTAPD_REPLY_OK
 *	fail:	 HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_DISABLE_VBSS_REKEY	"disable_vbss_rekey "


struct vbss_sta_info {
	/* STA mac address */
	uint8_t mac_addr[ETH_ALEN];
	/* The BSSID associated with STA */
	uint8_t bssid[ETH_ALEN];
	/* AID for the sta */
	uint16_t aid;
	/* sta_info flags */
	uint32_t flags;
	/* support rates */
	u8 supported_rates[WLAN_SUPP_RATES_MAX];
	int supported_rates_len;
	/* Capability related */
	uint16_t capability;
	struct ieee80211_ht_capabilities ht_capabilities;
	struct ieee80211_vht_capabilities vht_capabilities;
	struct ieee80211_vht_operation vht_operation;
	uint8_t vht_opmode;
	struct ieee80211_he_capabilities he_capab;
	size_t he_capab_len;
	struct ieee80211_he_6ghz_band_cap he_6ghz_capab;

	struct vbss_key_info key_info;
};

enum vbss_roaming_role {
	VBSS_AP_DEFAULT,
	VBSS_AP_SOURCE,
	VBSS_AP_DESTINATION,
	VBSS_AP_ROLE_MAX
};

enum vbss_vap_role {
	VBSS_VAP_ORIGIN,
	VBSS_VAP_ROAMING,
	VBSS_VAP_ROLE_MAX
};

struct vbss_roaming_info {
	enum vbss_roaming_role role;
	uint8_t bssid[ETH_ALEN];
	uint8_t sta_addr[ETH_ALEN];
};

struct vbss_hostapd_data_info {
	enum vbss_vap_role role;
};

/* VBSS control block in Hostapd */
struct hostapd_vbss_cb {
	uint8_t enabled;
	uint32_t flags;

	/* TODO: We currently only support 1 roaming.
	 * Need to support multiple concurrent roaming later
	 */
	struct vbss_roaming_info roaming;
};

/*
 * Infos of VBSS VAP
 */
struct vbss_vap_info {
	uint8_t bssid[ETH_ALEN];
	char ssid[33];
	char ifname[33];
	uint32_t auth_type;
	char pwd[65];
	uint8_t role;
};

enum WPA_AUTHORIZE_TYPE {
	WPA_AUTHORIZE_TYPE_NONE = 0,
	WPA_AUTHORIZE_TYPE_WPA,
	WPA_AUTHORIZE_TYPE_WPA2,
	WPA_AUTHORIZE_TYPE_SAE,
	WPA_AUTHORIZE_TYPE_WPA2_PSK_SAE,
	WPA_AUTHORIZE_TYPE_WPA2_PSK_MIXED,
	WPA_AUTHORIZE_TYPE_WEP,
	WPA_AUTHORIZE_TYPE_OWE
};


int vbss_init_bss(struct hostapd_data *hapd, int first);
int vbss_get_enabled(char *cmd, char *buf, size_t buflen);
int vbss_set_enabled(struct hapd_interfaces *interfaces, char *cmd,
		char *buf, size_t buflen);
int vbss_get_sta_info(struct hostapd_data *hapd, char *cmd, char *buf, size_t buflen);
int vbss_get_vap_info(struct hostapd_data *hapd, char *buf, char *reply, size_t reply_len);
int vbss_init(void);
int vbss_set_hidden_ssid(struct hostapd_data *hapd, int authorized);
int vbss_add_sta(struct hostapd_data *hapd, struct vbss_sta_info *vbss_sta);
int vbss_get_aid(struct hostapd_data *hapd, struct sta_info *sta);
int vbss_check_enabled(struct hostapd_data *hapd);
int vbss_disable_rekey(struct hostapd_data *hapd, char *cmd,
		char *buf, size_t buflen);
struct hostapd_data *get_hapd_by_bssid(struct hostapd_iface *iface,
					    const u8 *bssid);
int vbss_disable_wep_rekey(struct hostapd_data *hapd);

/* Trigger VBSS switch now.
 * Input:
 *	hapd:	hostapd per-BSS data structure
 *	buf:	input cmd buffer
 * Return values:
 *	0:	OK		(hostapd_ctrl_iface_receive_process() will fill "OK\n" to reply.)
 *	<0:	Fail	(hostapd_ctrl_iface_receive_process() will fill "FAIL\n" to reply.)
 */
int vbss_trigger_switch(struct hostapd_data *hapd, char *buf);

/* Set VBSS switch done.
 * Input:
 *	hapd:	hostapd per-BSS data structure
 *	buf:	input cmd buffer
 * Return values:
 *	0:	OK		(hostapd_ctrl_iface_receive_process() will fill "OK\n" to reply.)
 *	<0:	Fail	(hostapd_ctrl_iface_receive_process() will fill "FAIL\n" to reply.)
 */
int vbss_set_switch_done(struct hostapd_data *hapd, char *buf);

/* Handler of delete VBSS VAP.
 * Input:
 *	buf:	NULL
 * Return value:
 *	<0:	error
 *	=0: OK
 */
int vbss_del_vap(struct hostapd_data *hapd, char *buf);

/* Handler of delete VBSS STA.
 * Input:
 *	buf:	"<STA MAC in format xx:xx:xx:xx:xx:xx>"
 * Return value:
 *	<0:	error
 *	=0: OK
 */
int vbss_del_sta(struct hostapd_data *hapd, char *buf);

int vbss_set_hostapd_data_info(struct hostapd_data *hapd, int role);
#endif
