
#ifndef _CLSAPI_WPA_ADAPTER_H
#define _CLSAPI_WPA_ADAPTER_H

#include <sys/un.h>
#include "clsapi_wifi.h"

struct wpa_ctrl {
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};


#ifndef CONFIG_CTRL_IFACE_DIR_HOSTAPD
#define CONFIG_CTRL_IFACE_DIR_HOSTAPD "/var/run/hostapd"
#endif

#ifndef CONFIG_CTRL_IFACE_DIR_WPA_SUPPLICANT
#define CONFIG_CTRL_IFACE_DIR_WPA_SUPPLICANT "/var/run/wpa_supplicant"
#endif

#ifndef CONFIG_CTRL_IFACE_GLOBAL
#define CONFIG_CTRL_IFACE_GLOBAL "global"
#endif

#define CONFIG_CTRL_IFACE_CLIENT_DIR "/tmp"
#define CONFIG_CTRL_IFACE_CLIENT_PREFIX "clsapi_wpa_ctrl_"


/* CLS hostapd private cmds */
#define CLS_HOSTAPD_CMD_PREFIX			"CLS"
#define CLS_HOSTAPD_REPLY_OK			"OK\n"		/* copied from hostapd OK reply, ends with '\n', no '\0'*/
#define CLS_HOSTAPD_REPLY_FAIL			"FAIL\n"	/* copy from hostapd fail reply, ends with '\n', no '\0'*/

/* VBSS cmds */
/* ADD_VBSS_VAP cmd format:
 *		"CLS add_vbss_vap\0" + <bin of struct vbss_vap_info>
 * reply format:
 *		success: string of ifname which new created
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_ADD_VBSS_VAP	"add_vbss_vap "

/* VBSS cmds */
/* GET_VBSS_VAP cmd format:
 *		"CLS add_vbss_vap\0" + <vap mac addr>
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_GET_VBSS_VAP	"get_vbss_vap "

/* DEL_VBSS_VAP cmd format:
 *		"CLS del_vbss_vap \0"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail:    CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_DEL_VBSS_VAP	"del_vbss_vap "

/* ADD_VBSS_STA cmd format:
 *		"CLS add_vbss_sta\0" + <bin of struct vbss_sta_info>
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_ADD_VBSS_STA	"add_vbss_sta "

/* DEL_VBSS_STA cmd format:
 *		"CLS del_vbss_sta <STA MAC in format xx:xx:xx:xx:xx:xx>"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail:    CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_DEL_VBSS_STA	"del_vbss_sta "

/* SET_VBSS_ENABLED cmd format:
 *		"CLS set_vbss_enabled\0" + <phy name> + <on/off>
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_SET_VBSS_ENABLED	"set_vbss_enabled "


/* GET_VBSS_STA cmd format:
 *		"CLS get_vbss_sta\0" + <interface name> + <MAC address>
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_GET_VBSS_STA	"get_vbss_sta "


/* TRG_VBSS_SWITCH cmd format:
 *		"CLS trg_vbss_switch <STA mac in aa:bb...>"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK //no '\0'
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_TRG_VBSS_SWITCH	"trg_vbss_switch "


/* SET_VBSS_SWITCH_DONE cmd format:
 *		"CLS set_vbss_switch_done <STA mac in aa:bb...>"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK //no '\0'
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_SET_VBSS_SWITCH_DONE	"set_vbss_switch_done "

/* VBSS cmds */
/* disable_vbss_rekey cmd format:
 *		"CLS disable_vbss_rekey <VAP mac in aa:bb...>"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_CMD_DISABLE_VBSS_REKEY	"disable_vbss_rekey "


/* WPS cmds */
/* GET_WPS_STATUS cmd format:
 *		"CLS GET_WPS_STATUS"
 * reply format:
 *		success: string of wps status
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_GET_WPS_STATUS "GET_WPS_STATUS"

/* WPS cmds */
/* WPS_CANCEL cmd format:
 *		"CLS WPS_CANCEL"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_WPS_CANCEL "WPS_CANCEL"

/* AP_WPS_PBC cmd format:
 *		"CLS AP_WPS_PBC"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_AP_WPS_PBC "AP_WPS_PBC"

/* AP_WPS_PIN cmd format:
 *		"WPS_PIN"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_AP_WPS_PIN "WPS_PIN"

/* WPS_AP_PIN cmd format:
 *		"WPS_AP_PIN"
 * reply format:
 *		success: CLS_HOSTAPD_REPLY_OK
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_WPS_AP_PIN "WPS_AP_PIN"

/* cls cmds */
/* CHANNEL_SURVEY cmd format:
 *		"CLS CHANNEL_SURVEY"
 * reply format:
 *		success: string of wps status
 *		fail: CLS_HOSTAPD_REPLY_FAIL
 */
#define CLS_HOSTAPD_CMD_CHANEL_SURVEY "CHANNEL_SURVEY"

/*
 * Add VBSS VAP via hostapd ctrl iface
 * Input:
 *	vap:		vap info used to create the VBSS vap
 *	new_ifname:	created ifname of the vap
 *	name_len:	buffer len of the new_ifname string buffer
 * Return:
 *	0:	OK
 *	!0:	Error
 */
extern int hostapd_add_vbss_vap(const struct vbss_vap_info *vap, char *new_ifname, const int name_len);

/*
 * Delete VBSS VAP via hostapd ctrl iface
 * Input:
 *	ifname: ifname of the vap
 * Return:
 *	0:	OK
 *	!0:	Error
 */
extern int hostapd_del_vbss_vap(const char *ifname);

/*
 * Get VBSS VAP via hostapd ctrl iface
 * Input:
 *	ifname: ifname of the vap
 * Return:
 *	0:	OK
 *	!0:	Error
 */

extern int hostapd_get_vbss_vap(const char *new_ifname, struct vbss_vap_info *vap);


/*
 * Add VBSS STA via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 *	sta:	STA info used to cerate the VBSS STA
 * Return:
 *	0:	OK
 *	!0:	Error
 */
extern int hostapd_add_vbss_sta(const char *ifname, const struct vbss_sta_info *sta);

/*
 * Delete VBSS STA via hostapd ctrl iface
 * Input:
 *	ifname: vap ifname which the STA associated with
 * Return:
 *	0:	OK
 *	!0:	Error
 */
extern int hostapd_del_vbss_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

/*
 * Set VBSS enabled via hostapd ctrl iface
 * Input:
 *	phyname:	phy which the STA attached to
 *	onoff:	turn on or off the VBSS mode
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_set_vbss_enabled(const char *phyname, const bool onoff);

/*
 * Get VBSS sta information via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 *	sta_mac:	The MAC address of the STA
 *	sta:	The STA information get from hostapd ctrl interface
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_get_vbss_sta_info(const char *ifname, const uint8_t sta_mac[ETH_ALEN],
		struct vbss_sta_info *sta);

/*
 * Trigger STA do VBSS switch now via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 *	sta_mac:	The MAC address of the STA
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_trigger_vbss_switch(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

/*
 * Set STA VBSS switch done via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 *	sta_mac:	The MAC address of the STA
 * Return:
 *	0:	OK
 *	!0:	Error
 */

 /*
 * Set VBSS disable via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_set_vbss_stop_rekey(const char *ifname);

int hostapd_set_vbss_switch_done(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

int hostapd_ctrl_cmd_get(const char *ifname, const char *cmd_buf, const int cmd_len, void *reply, size_t *reply_len);
int hostapd_ctrl_cmd_set(const char *ifname, const char *cmd_buf, const int cmd_len);
int hostapd_ctrl_clscmd_get(const char *ifname, const char *clscmd_buf, const int cmd_len, void *reply, size_t *reply_len);
int hostapd_ctrl_clscmd_set(const char *ifname, const char *clscmd_buf, const int cmd_len);

/*
 * Get current WPS status via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_get_wps_status(const char *ifname, struct clsapi_wifi_wps_status *wps_status);

/*
 * Cancel WPS via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_cancel_wps(const char *ifname);

/*
 * Start AP WPS PBC via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_start_ap_wps_pbc(const char *ifname);

/*
 * Start AP WPS PIN via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_start_ap_wps_pin(const char *ifname, const char *uuid, const char *pin);

/*
 * Start STA WPS PBC via wpa_supplicant ctrl iface
 * Input:
 *	ifname:	ifname which the STA is
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int wpad_start_sta_wps_pbc(const char *ifname);

/*
 * Start STA WPS PIN via wpa_supplicant ctrl iface
 * Input:
 *	ifname:	ifname which the STA is
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int wpad_start_sta_wps_pin(const char *ifname, const char *macaddr, const char* pin);

/*
 * Disable runtime WPS AP PIN via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_disable_wifi_runtime_wps_ap_pin(const char *ifname);

/*
 * Get runtime random WPS AP PIN via hostapd ctrl iface
 * Input:
 *	ifname:	ifname which the STA attached to
 * Return:
 *	0:	OK
 *	!0:	Error
 */
int hostapd_set_wifi_random_runtime_wps_ap_pin(const char *ifname, string_8 pin);
int hostapd_get_channel_survey(const char *ifname, struct clsapi_channel_survey *channel_survey);

extern int create_new_bss_config(char *ifname, const struct vbss_vap_info *vap);
extern int local_get_radio_id(const char *ifname, unsigned int *radio_id);
extern int get_primary_vap_name(int radio, char *name, int len);
extern int get_interface_mac(const char *ifname, uint8_t *mac);
extern int remove_bss(int radio, const char *ifname);
#endif /* _CLSAPI_WPA_ADAPTER_H */

