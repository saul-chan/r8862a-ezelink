/*
 * hostapd / VBSS definition
 * Copyright (c) 2023, Clourney Semi
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"
#include "utils/common.h"
#include "drivers/nl80211_copy.h"
#include "ap/hostapd.h"
#include "ap/ap_drv_ops.h"
#include "ap/wpa_auth.h"

#include "hostapd.h"
#include "vbss.h"
#include "eapol_auth/eapol_auth_sm.h"
#include "eapol_auth/eapol_auth_sm_i.h"
#include "ap/wpa_auth_i.h"
#include "eap_server/eap.h"
#include "sta_info.h"
#include "beacon.h"
#include "ieee802_11.h"
#include "drivers/driver.h"
#include "ap/ieee802_1x.h"
#include "ap_mlme.h"


#ifdef CLS_WIFI_CONFIG
//added for test
u8 ht_cap[26] = {0x7e, 0x10, 0x1b, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00};

u8 vht_cap[12] = {0xfa, 0x04, 0x80, 0x03, 0xaa, 0xaa, 0x00, 0x00, 0xaa, 0xaa, 0x00, 0x00};

u8 he_cap[29] = {0x01, 0x78, 0xc8, 0x1a, 0x40, 0x00, 0x1c, 0xbf, 0xce, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0xfa, 0xff, 0xfa, 0xff, 0xfa, 0xff,
				0xfa, 0xff, 0xfa, 0xff, 0xfa, 0xff};

u8 ext_cap[11] = {0x04, 0x00, 0x4a, 0x02, 0x01, 0x00, 0x40, 0x40, 0x00, 0x01, 0x20};

u8 support_ops_classed[21] = {0x80, 0x51, 0x52, 0x53, 0x54, 0x73, 0x74, 0x75, 0x76, 0x77,
								0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d,
								0x7e, 0x7f, 0x80, 0x81, 0x82};

u8 vendor_spec[7] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};

u8 wpa_ie[22] = {0x30, 0x14, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x04,
				0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00};

#ifdef CLS_VBSS_CONFIG

struct hostapd_vbss_cb vbss_cb[NUM_NL80211_BANDS];

static int vbss_get_interface_band(char *phy)
{
	int band;
	int ret;

	/* phy may be "wlan0"/"wlan1" in OpenWrt, or "phy0"/"phy1" in Buildroot, or "cls0"/"cls1" in future */
	ret = sscanf(phy, "%*[a-z]%d", &band);
	if (ret <= 0) {
		wpa_printf(MSG_ERROR, "failed to get band");

		return -1;
	}

	return band;
}

/* Get the enabled info for hostapd data */
int vbss_check_enabled(struct hostapd_data *hapd)
{
	int band;

	if (hapd == NULL || hapd->conf == NULL)
		return 0;

	band = vbss_get_interface_band(hapd->conf->iface);
	if (band > NL80211_BAND_5GHZ || band < 0)
		return 0;

	return vbss_cb[band].enabled;
}

static int vbss_dump_sta_info(struct vbss_sta_info *sta_info)
{
	wpa_printf(MSG_DEBUG, "sta mac_addr " MACSTR, MAC2STR(sta_info->mac_addr));
	wpa_printf(MSG_DEBUG, "sta bssid " MACSTR, MAC2STR(sta_info->bssid));
	wpa_printf(MSG_DEBUG, "sta aid %u", sta_info->aid);
	wpa_printf(MSG_DEBUG, "sta flags 0x%08x", sta_info->flags);

	if (sta_info->flags & WLAN_STA_VHT) {
		wpa_printf(MSG_DEBUG, "vht_caps_info=0x%08x\n",
				  le_to_host32(sta_info->vht_capabilities.vht_capabilities_info));

		wpa_printf(MSG_DEBUG, "vht mcs set [0x%04x, 0x%04x, 0x%04x, 0x%04x]",
			sta_info->vht_capabilities.vht_supported_mcs_set.rx_map,
			sta_info->vht_capabilities.vht_supported_mcs_set.rx_highest,
			sta_info->vht_capabilities.vht_supported_mcs_set.tx_map,
			sta_info->vht_capabilities.vht_supported_mcs_set.tx_highest);
	}

	if (sta_info->flags & WLAN_STA_HT) {
		wpa_printf(MSG_DEBUG, "ht_caps_info=0x%04x, 0x%04x\n",
					le_to_host16(sta_info->ht_capabilities.ht_capabilities_info),
					sta_info->ht_capabilities.ht_capabilities_info);

		wpa_printf(MSG_DEBUG, "ht_caps ampdu_param:0x%02x\n",
					sta_info->ht_capabilities.a_mpdu_params);

		wpa_hexdump(MSG_DEBUG, "ht mcs set ",
				    &sta_info->ht_capabilities.supported_mcs_set[0], 16);

		wpa_printf(MSG_DEBUG, "ht_caps ht_ext_cab:0x%04x\n",
					sta_info->ht_capabilities.ht_extended_capabilities);

		wpa_printf(MSG_DEBUG, "ht_caps ht_tx_bf:0x%08x\n",
					sta_info->ht_capabilities.tx_bf_capability_info);

		wpa_printf(MSG_DEBUG, "ht_caps ht_tx_bf:0x%02x\n",
					sta_info->ht_capabilities.asel_capabilities);
	}

	if (sta_info->flags & WLAN_STA_HE) {
		wpa_hexdump(MSG_DEBUG, "HE ",
				    &sta_info->he_capab, sta_info->he_capab_len);
	}

	wpa_printf(MSG_DEBUG, "sta auth_alg %u", sta_info->key_info.auth_alg);
	wpa_printf(MSG_DEBUG, "sta wpa_alg %u", sta_info->key_info.wpa_alg);
	wpa_hexdump(MSG_DEBUG, "TK: ", sta_info->key_info.tk, 16);
}

/* Update the STA/VAP info into VBSS control block. When the AC triggers switch, remove them */
static int vbss_update_roaming_info(struct hostapd_data *hapd,
		struct vbss_sta_info *vbss_sta, enum vbss_roaming_role role)
{
	int band;

	if (hapd == NULL || vbss_sta == NULL)
		return -1;

	band = vbss_get_interface_band(hapd->conf->iface);
	if (band > NL80211_BAND_5GHZ || band < 0)
		return -1;

	wpa_printf(MSG_DEBUG, "role %u sta mac " MACSTR, role, MAC2STR(vbss_sta->mac_addr));
	vbss_cb[band].roaming.role = role;
	os_memcpy(vbss_cb[band].roaming.bssid, vbss_sta->bssid, ETH_ALEN);
	os_memcpy(vbss_cb[band].roaming.sta_addr, vbss_sta->mac_addr, ETH_ALEN);

	return 0;
}

/* With STA MAC address, get the associated BSS index in interface  */
static int vbss_get_bss_by_sta(struct hostapd_data *hapd, uint8_t *mac)
{
	struct hostapd_iface *iface;
	struct hostapd_data *bss;
	struct sta_info *sta;
	int i;

	iface = hapd->iface;
	if (iface == NULL) {
		wpa_printf(MSG_INFO, "VBSS: iface is NULL");

		return -1;
	}

	for (i = 0; i < iface->num_bss; i++) {
		bss = iface->bss[i];
		if (bss == NULL) {
			wpa_printf(MSG_INFO, "VBSS: num_bss %d/%lu is NULL", i, iface->num_bss);

			return -1;
		}

		sta = ap_get_sta(bss, mac);
		if (sta)
			return i;
	}

	return -1;
}

/* Add VBSS STA with specific info from user module */
int vbss_add_sta(struct hostapd_data *hapd, struct vbss_sta_info *vbss_sta)
{
	struct sta_info *sta;
	int i = 0;
	u8 def_supported_rate[3] = {0x0c, 0x12, 0x30};
	int def_supported_rate_len = 3;
	uint32_t vbss_sta_flags = vbss_sta->flags;

	sta = ap_get_sta(hapd, vbss_sta->mac_addr);
	if (sta)
		return 0;

	sta = ap_sta_add(hapd, vbss_sta->mac_addr);
	if (sta == NULL)
		return -1;

	wpa_printf(MSG_INFO, "cls add new STA " MACSTR " with sta->flags= 0x%08x 0x%08x, aid=%u",
				MAC2STR(vbss_sta->mac_addr), sta->flags, vbss_sta->flags, vbss_sta->aid);
	sta->flags |= vbss_sta->flags & (WLAN_STA_HT | WLAN_STA_VHT |
			WLAN_STA_HE | WLAN_STA_VHT_OPMODE_ENABLED | WLAN_STA_WMM | WLAN_STA_MFP);
	hostapd_sta_add(hapd, sta->addr, 0, 0,
			def_supported_rate,
			def_supported_rate_len,
			0, NULL, NULL, NULL, 0, NULL, 0, NULL,
			sta->flags, 0, 0, 0, 0, NULL, false);

	for (i = 0; i < vbss_sta->supported_rates_len; i++)
		sta->supported_rates[i] = vbss_sta->supported_rates[i];
	sta->supported_rates_len = vbss_sta->supported_rates_len;
	if (!sta->supported_rates_len) {
		sta->supported_rates_len = 8;
		sta->supported_rates[0] = 0x0c;
		sta->supported_rates[1] = 0x12;
		sta->supported_rates[2] = 0x18;
		sta->supported_rates[3] = 0x24;
		sta->supported_rates[4] = 0x30;
		sta->supported_rates[5] = 0x48;
		sta->supported_rates[6] = 0x60;
		sta->supported_rates[7] = 0x6c;
	}

	//authentication
	sta->flags |= WLAN_STA_AUTH;
	//associate
	sta->listen_interval = 5;

	check_wmm(hapd, sta, vendor_spec, 7);

	//ht (tag 45)
	if (vbss_sta_flags & WLAN_STA_HT)
		copy_sta_ht_capab(hapd, sta, (u8 *)&vbss_sta->ht_capabilities);

	//vht (tag 191)
	if (vbss_sta_flags & WLAN_STA_VHT)
		copy_sta_vht_capab(hapd, sta, (u8 *)&vbss_sta->vht_capabilities);

	//he
	if (vbss_sta_flags & WLAN_STA_HE) {
		if (vbss_sta->he_capab_len)
			copy_sta_he_capab(hapd, sta, IEEE80211_MODE_AP,
					(u8 *)&vbss_sta->he_capab, vbss_sta->he_capab_len);
	}

	if (vbss_sta_flags & WLAN_STA_6GHZ)
		copy_sta_he_6ghz_capab(hapd, sta, (u8 *)&vbss_sta->he_6ghz_capab);

	ap_copy_sta_supp_op_classes(sta, support_ops_classed, 21);

	check_ext_capab(hapd, sta, ext_cap, 11);

	sta->vht_opmode = vbss_sta->vht_opmode;

	sta->aid = vbss_sta->aid;
	sta->capability = vbss_sta->capability;
	sta->flags |= WLAN_STA_ASSOC_REQ_OK;
	sta->added_unassoc = 1;
	//add sta to driver
	add_associated_sta(hapd, sta, 0);
	sta->flags |= WLAN_STA_ASSOC;

	//set pairwise
	sta->auth_alg = vbss_sta->key_info.auth_alg;
	if (vbss_sta->key_info.wpa_alg == WPA_ALG_CCMP) {
		if (sta->wpa_sm == NULL)
			sta->wpa_sm = wpa_auth_sta_init(hapd->wpa_auth,
						sta->addr,
						NULL);

		if (sta->wpa_sm) {
			wpa_validate_wpa_ie(hapd->wpa_auth, sta->wpa_sm,
					  hapd->iface->freq,
					  wpa_ie, 22,
					  wpa_ie,
					  22,
					  NULL, 0,
					  NULL, 0);
			sta->wpa_sm->PTK.installed = 1;
			sta->wpa_sm->wpa = WPA_VERSION_WPA2;
			sta->wpa_sm->wpa_ptk_state = WPA_PTK_PTKINITDONE;
			sta->wpa_sm->wpa_ptk_group_state = WPA_PTK_GROUP_IDLE;
		}

		if (hapd && hapd->wpa_auth && hapd->wpa_auth->group) {
				hapd->wpa_auth->group->GN = vbss_sta->key_info.GN;
				hapd->wpa_auth->group->GTK_len = vbss_sta->key_info.GTK_len;
				os_memcpy(hapd->wpa_auth->group->GTK, vbss_sta->key_info.GTK[vbss_sta->key_info.GN - 1],
						WPA_TK_MAX_LEN);
		}

		wpa_auth_set_key(sta->wpa_sm->wpa_auth, 0,
				vbss_sta->key_info.wpa_alg,
				broadcast_ether_addr, vbss_sta->key_info.GN,
				vbss_sta->key_info.GTK[vbss_sta->key_info.GN - 1],
				vbss_sta->key_info.GTK_len,
				KEY_FLAG_GROUP_TX_DEFAULT);

		if (sta->wpa_sm) {
			os_memcpy(sta->wpa_sm->PTK.tk, vbss_sta->key_info.tk, WPA_TK_MAX_LEN);
			sta->wpa_sm->PTK.tk_len = 16;
		}
		wpa_auth_set_key(sta->wpa_sm->wpa_auth, 0, vbss_sta->key_info.wpa_alg,
				sta->wpa_sm->addr,
				0, vbss_sta->key_info.tk, 16,
				KEY_FLAG_PAIRWISE_RX_TX);
	}
	sta->flags |= WLAN_STA_AUTHORIZED;
	sta->flags = vbss_sta->flags;
	ap_sta_set_authorized(hapd, sta, 1);
	//hostapd_set_sta_flags(hapd, sta);
	ieee802_1x_set_sta_authorized(hapd, sta, 1);

	wpa_printf(MSG_INFO, "------Add new STA " MACSTR " flags=0x%08x ",
				MAC2STR(vbss_sta->mac_addr), sta->flags);
	vbss_update_roaming_info(hapd, vbss_sta, VBSS_AP_DESTINATION);
	return 0;
}

/* Remove STA info from BSS's sta list */
static int vbss_remove_sta(struct hostapd_data *hapd, uint8_t *mac)
{
	struct sta_info *sta;

	wpa_printf(MSG_INFO, "VBSS: %s", __func__);
	sta = ap_get_sta(hapd, mac);
	if (sta == NULL) {
		wpa_printf(MSG_INFO, "VBSS: Not found sta from BSS");

		return -1;
	}

	if (hapd->num_sta != 1) {
		wpa_printf(MSG_INFO, "VBSS: Abnormal STA number %d in BSS", hapd->num_sta);

		return -1;
	}

	/* Remove and free STA related info */
	ap_sta_disconnect(hapd, sta, NULL, WLAN_REASON_BSS_TRANSITION_DISASSOC);
}

/* With STA MAC address, find the BSS and remove it from radio (It would also remove the STA) */
static int vbss_remove_bss_by_sta(struct hostapd_data *hapd, uint8_t *mac)
{
	struct hostapd_data *bss;
	int num_bss;
	int bss_idx;

	bss_idx = vbss_get_bss_by_sta(hapd, mac);
	if (bss_idx < 0) {
		wpa_printf(MSG_ERROR, "VBSS: invalid bss index %d", bss_idx);

		return -1;
	}

	if (hapd->iface && (bss_idx < hapd->iface->num_bss)) {
		bss =  hapd->iface->bss[bss_idx];
	} else {
		wpa_printf(MSG_ERROR, "VBSS: Not fond bss");

		return -1;
	}

	/* Before deleting the BSS, we need remove the STA firstly */
	vbss_remove_sta(bss, mac);
	wpa_printf(MSG_DEBUG, "removing bss idx %d " MACSTR  " by STA " MACSTR,
		bss_idx, MAC2STR(hapd->own_addr), MAC2STR(mac));

	/* Remove the  non-primary BSSes */
	if (bss_idx != 0)
		hostapd_remove_bss(hapd->iface, bss_idx);

	return 0;
}

static int vbss_clean_non_primary_bss(struct hostapd_iface *iface)
{
	int i;
	int num_bss;
	struct hostapd_data *hapd;

	if (iface->conf == NULL) {
		wpa_printf(MSG_ERROR, "BSS with NULL conf");

		return -1;
	}

	num_bss = iface->conf->num_bss;
	wpa_printf(MSG_DEBUG, "num_bss %lu", iface->conf->num_bss);
	for (i = num_bss - 1; i > 0; i--) {
		hapd = iface->bss[i];
		if (hapd == NULL) {
			wpa_printf(MSG_ERROR, "BSS with NULL hostapd data");

			return -1;
		}

		wpa_printf(MSG_DEBUG, "removing bss " MACSTR  " i %d",
			MAC2STR(hapd->own_addr), i);
		hostapd_remove_bss(iface, i);
	}

	return 0;
}


static int vbss_clean_interfaces(struct hapd_interfaces *interface)
{
	return 0;
}

/* Set BSS to hidden SSID after associated with a STA */
int vbss_set_hidden_ssid(struct hostapd_data *hapd, int authorized)
{
	int hide;

	if (vbss_check_enabled(hapd) <= 0)
		return -1;

	if (hapd->conf) {
		hide = authorized ? HIDDEN_SSID_ZERO_LEN : NO_SSID_HIDING;
		hapd->conf->ignore_broadcast_ssid = hide;
		ieee802_11_set_beacon(hapd);
		wpa_printf(MSG_INFO, "%s:%s hide %d", __func__, hapd->conf->iface, hide);
		return 0;
	}

	return -1;
}

/* For each new created BSS, call this function to init capabilities */
int vbss_init_bss(struct hostapd_data *hapd, int first)
{
	struct hostapd_config *conf;
	struct hostapd_bss_config *bss_conf;

	if (vbss_check_enabled(hapd) <= 0)
		return -1;

	if (hapd->iface->conf == NULL || hapd->conf == NULL)
		return -1;

	/* Disable the BSS color */
	conf = hapd->iface->conf;
	conf->he_op.he_bss_color_disabled = 1;

	/* Disable BF */
	conf->vht_capab &= ~VHT_CAP_SU_BEAMFORMER_CAPABLE;
	conf->vht_capab &= ~VHT_CAP_MU_BEAMFORMER_CAPABLE;
	conf->he_phy_capab.he_su_beamformer = 0;
	conf->he_phy_capab.he_mu_beamformer = 0;

	/* Disable multiple-BSSID temporarily */
	if (hapd->conf)
		hapd->conf->ext_capa[2] &= ~0x40;

	bss_conf = hapd->conf;
	bss_conf->max_num_sta = 1;
	bss_conf->no_probe_resp_if_max_sta = 1;

	/* Do not broadcast deauth when removing STA */
	bss_conf->broadcast_deauth = 0;

	return 0;
}

/* Report the VBSS status `Enabled` or `Disabled` */
int vbss_get_enabled(char *cmd, char *buf, size_t buflen)
{
	int band;
	int reply_len;

	band = vbss_get_interface_band(cmd);
	if (band > NL80211_BAND_5GHZ || band < 0)
		return -1;

	wpa_printf(MSG_INFO, "VBSS mode onoff %u", vbss_cb[band].enabled);
	reply_len = snprintf(buf, buflen, (vbss_cb[band].enabled ? "Enabled" : "Disabled"));

	return reply_len;
}

/* Set the VBSS enabled status: onoff=0/1 */
int vbss_set_enabled(struct hapd_interfaces *interfaces, char *cmd,
		char *buf, size_t buflen)
{
	struct hostapd_iface *iface = NULL;
	struct hostapd_data *hapd_data = NULL;
	char *phy_name;
	int name_len;
	int onoff;
	int band;
	int ret = 0;
	int hidden = 0;
	int i;

	phy_name = os_strchr(cmd, ' ');
	name_len = (int)(phy_name - cmd);
	for (i = 0; i < interfaces->count; i++) {
		if (os_strncmp(interfaces->iface[i]->phy, cmd, name_len) == 0) {
			iface = interfaces->iface[i];

			break;
		}
	}

	band = vbss_get_interface_band(cmd);
	if (band > NL80211_BAND_5GHZ || band < 0) {
		ret = -1;
		goto vbss_end;
	}

	cmd = phy_name + 1;
	onoff = atoi(cmd);
	vbss_cb[band].enabled = onoff;
	if (iface) {
		vbss_clean_non_primary_bss(iface);
		/*get primary bss info*/
		hapd_data = iface->bss[0];
		if (hapd_data) {
			hidden = hapd_data->conf->ignore_broadcast_ssid;
			wpa_printf(MSG_INFO, "%s %s:hidden =%d", __func__, hapd_data->conf->iface, hidden);
			if (onoff && hidden != HIDDEN_SSID_ZERO_LEN) {
				/*vbss enable, hidden ssid*/
				hapd_data->conf->ignore_broadcast_ssid = HIDDEN_SSID_ZERO_LEN;
				ieee802_11_set_beacon(hapd_data);
			} else if (!onoff && hidden == HIDDEN_SSID_ZERO_LEN) {
				/*vbss disable, broadcast ssid*/
				hapd_data->conf->ignore_broadcast_ssid = NO_SSID_HIDING;
				ieee802_11_set_beacon(hapd_data);
			}
		}
	}

vbss_end:
	ret = snprintf(buf, buflen, (ret ? HOSTAPD_REPLY_FAIL : HOSTAPD_REPLY_OK));

	return ret;
}

static int vbss_fullfil_sta_info(struct hostapd_data *hapd, struct sta_info *sta,
		struct vbss_sta_info *vbss_sta)
{
	struct wpa_ptk *ptk;
	struct wpa_authenticator *wpa_auth;
	struct wpa_group *group;
	struct wpa_state_machine *wpa_sm;
	int i;

	wpa_printf(MSG_DEBUG, "%s", __func__);
	os_memcpy(vbss_sta->mac_addr, sta->addr, ETH_ALEN);
	os_memcpy(vbss_sta->bssid, hapd->own_addr, ETH_ALEN);
	vbss_sta->aid = sta->aid;
	vbss_sta->flags = sta->flags;
	vbss_sta->capability = sta->capability;
	vbss_sta->supported_rates_len = sta->supported_rates_len;
	for (i = 0; i < sta->supported_rates_len; i++)
		vbss_sta->supported_rates[i] = sta->supported_rates[i];

	if (sta->ht_capabilities)
		memcpy(&vbss_sta->ht_capabilities, sta->ht_capabilities, sizeof(struct ieee80211_ht_capabilities));

	if (sta->vht_capabilities)
		memcpy(&vbss_sta->vht_capabilities, sta->vht_capabilities, sizeof(struct ieee80211_vht_capabilities));

	if (sta->vht_operation)
		memcpy(&vbss_sta->vht_operation, sta->vht_operation, sizeof(struct ieee80211_vht_operation));
	vbss_sta->vht_opmode = sta->vht_opmode;

	if (sta->he_capab) {
		os_memcpy(&vbss_sta->he_capab, sta->he_capab, sta->he_capab_len);
		vbss_sta->he_capab_len = sta->he_capab_len;
	}
	if (sta->he_6ghz_capab)
		os_memcpy(&vbss_sta->he_6ghz_capab, sta->he_6ghz_capab,
			sizeof(struct ieee80211_he_6ghz_band_cap));

	/* Get Key info */
	vbss_get_sta_sec_info(hapd->wpa_auth, sta->wpa_sm, &vbss_sta->key_info);
}

int vbss_get_sta_info(struct hostapd_data *hapd, char *buf, char *reply, size_t reply_len)
{
	uint8_t addr[ETH_ALEN];
	struct sta_info *sta;
	struct vbss_sta_info *vbss_sta;
	int bss_idx;

	wpa_printf(MSG_DEBUG, "%s mac %s buflen %lu", __func__, buf, reply_len);
	if (reply_len < sizeof(*vbss_sta)) {
		wpa_printf(MSG_ERROR, "buffer len < vbss_sta_info size");

		return -1;
	}

	if (hwaddr_aton(buf, addr)) {
		wpa_printf(MSG_ERROR, "failed to get MAC from cmd %s", buf);

		return -1;
	}

	bss_idx = vbss_get_bss_by_sta(hapd, addr);
	if (bss_idx < 0) {
		wpa_printf(MSG_ERROR, "failed to get bss for STA %s", buf);

		return -1;
	}

	if (hapd->iface && (bss_idx < hapd->iface->num_bss)) {
		hapd = hapd->iface->bss[bss_idx];
	} else {
		wpa_printf(MSG_ERROR, "failed to get bss data for STA %s", buf);

		return -1;
	}

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_ERROR, "failed to get sta from %s", buf);

		return -1;
	}

	vbss_sta = (struct vbss_sta_info *)reply;
	os_memset(vbss_sta, 0, sizeof(struct vbss_sta_info));
	vbss_fullfil_sta_info(hapd, sta, vbss_sta);
	vbss_update_roaming_info(hapd, vbss_sta, VBSS_AP_SOURCE);
	vbss_dump_sta_info(vbss_sta);

	return sizeof(*vbss_sta);
}

int vbss_get_vap_info(struct hostapd_data *hapd, char *buf, char *reply, size_t reply_len)
{
	uint8_t addr[ETH_ALEN];
	struct vbss_vap_info *vbss_vap;
	struct hostapd_iface *iface;
	struct hostapd_bss_config *bss_conf;
	size_t bss_idx;
#ifdef CONFIG_WEP
	struct hostapd_wep_keys *wep;
	int i;
#endif

	wpa_printf(MSG_INFO, "%s mac %s buflen %lu", __func__, buf, reply_len);
	if (reply_len < sizeof(*vbss_vap)) {
		wpa_printf(MSG_ERROR, "buffer len < vbss_sta_info size");

		return -1;
	}

	if (hwaddr_aton(buf, addr)) {
		wpa_printf(MSG_ERROR, "failed to get MAC from cmd %s", buf);

		return -1;
	}

	iface = hapd->iface;
	if (!iface)
		return -1;

	for (bss_idx = 0; bss_idx < iface->num_bss; bss_idx++) {
		if (iface->bss[bss_idx]) {
			bss_conf = iface->bss[bss_idx]->conf;
			if (bss_conf && memcmp(bss_conf->bssid, addr, 6) == 0)
				break;
		}
	}

	if (bss_idx == iface->num_bss)
		return -1;

	vbss_vap = (struct vbss_vap_info *)reply;
	if (bss_conf->ssid.wpa_passphrase)
		strncpy(vbss_vap->pwd, bss_conf->ssid.wpa_passphrase, sizeof(vbss_vap->pwd) - 1);
	if (bss_conf->wpa_key_mgmt == WPA_KEY_MGMT_PSK
			&& bss_conf->wpa == WPA_PROTO_RSN)
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_WPA2;
	else if (bss_conf->wpa_key_mgmt == WPA_KEY_MGMT_PSK
			&& bss_conf->wpa == WPA_PROTO_WPA)
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_WPA;
	else if (bss_conf->wpa_key_mgmt == WPA_KEY_MGMT_SAE
			&& bss_conf->wpa == WPA_PROTO_RSN)
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_SAE;
	else if (bss_conf->wpa_key_mgmt & WPA_KEY_MGMT_SAE
			&& bss_conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK
			&& bss_conf-> wpa_pairwise == WPA_CIPHER_CCMP
			&& bss_conf->wpa == WPA_PROTO_RSN)
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_WPA2_PSK_SAE;
	else if (bss_conf->wpa_key_mgmt == WPA_KEY_MGMT_PSK
			&& bss_conf-> wpa_pairwise & WPA_CIPHER_CCMP
			&& bss_conf-> wpa_pairwise & WPA_CIPHER_TKIP
			&& bss_conf->wpa == WPA_PROTO_RSN)
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_WPA2_PSK_MIXED;
	else if (bss_conf->wpa_key_mgmt == WPA_KEY_MGMT_OWE
			&& bss_conf-> wpa_pairwise == WPA_CIPHER_CCMP
			&& bss_conf->wpa == WPA_PROTO_RSN)
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_OWE;
	else if (bss_conf->wpa == 0 && (bss_conf-> wpa_pairwise == WPA_CIPHER_WEP104
			|| bss_conf-> wpa_pairwise == WPA_CIPHER_WEP40))
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_WEP;
	else
		vbss_vap->auth_type = WPA_AUTHORIZE_TYPE_NONE;
#ifdef CONFIG_WEP
	if (vbss_vap->auth_type == WPA_AUTHORIZE_TYPE_WEP) {
		wep = &bss_conf->ssid.wep;
		for (i = 0; i< NUM_WEP_KEYS; i++) {
			if (wep->key[i] != NULL) {
				strncpy(vbss_vap->pwd, wep->key[i], wep->len[i]);
				break;
			}
		}
	}
#endif
	return sizeof(*vbss_vap);
}

/* Handler of trigger VBSS switch.
 * Input:
 *	buf:	"<STA MAC in format 00:11:22:aa:bb:cc>"
 * Return value:
 *	<0:	error
 *	=0: OK
 */
int vbss_trigger_switch(struct hostapd_data *hapd, char *buf)
{
	uint8_t sta_mac[ETH_ALEN];
	int ret = -1;
	int band;

	wpa_printf(MSG_DEBUG, "%s mac %s", __func__, buf);
	if (hwaddr_aton(buf, sta_mac)) {
		wpa_printf(MSG_ERROR, "failed to get STA MAC from cmd %s", buf);
		return -1;
	}

	band = vbss_get_interface_band(hapd->conf->iface);
	if (band > NL80211_BAND_5GHZ || band < 0)
		return -1;

	/* Check if the STA MAC address is correct */
	if (os_memcmp(sta_mac, vbss_cb[band].roaming.sta_addr, ETH_ALEN) != 0) {
		wpa_printf(MSG_ERROR, "failed to get pending roaming STA for sta %s", buf);

		return -1;
	}

	if (vbss_cb[band].roaming.role  == VBSS_AP_SOURCE)
		ret = vbss_remove_bss_by_sta(hapd, sta_mac);
	else if (vbss_cb[band].roaming.role  == VBSS_AP_DESTINATION)
		ret = 0; /* Do nothing now, enabling works in driver and firmware */
	else
		wpa_printf(MSG_DEBUG, "abnormal role %d", vbss_cb[band].roaming.role);

	return ret;
}

/* VBSS switch done handler.
 * Input:
 *	buf:	"<STA MAC in format 00:11:22:aa:bb:cc>"
 * Return value:
 *	<0:	error
 *	=0: OK
 */
int vbss_set_switch_done(struct hostapd_data *hapd, char *buf)
{
	int ret = 0;
	uint8_t sta_mac[ETH_ALEN];

	wpa_printf(MSG_DEBUG, "%s mac %s", __func__, buf);
	if (hwaddr_aton(buf, sta_mac)) {
		wpa_printf(MSG_ERROR, "failed to get STA MAC from cmd %s", buf);
		return -1;
	}

	return ret;
}

size_t get_hapd_by_ifname(struct hostapd_iface *iface,
					    const char *ifname)
{
	size_t i;

	for (i = 0; i < iface->num_bss; i++) {
		if (iface->bss[i]->conf && strncmp(ifname, iface->bss[i]->conf->iface, IFNAMSIZ) == 0)
			return i;
	}
	return iface->num_bss;
}


/* Handler of delete VBSS VAP.
 * Input:
 *	buf:	NULL
 * Return value:
 *	<0:	error
 *	=0: OK
 */
int vbss_del_vap(struct hostapd_data *hapd, char *buf)
{
	int ret = 0;
	size_t idx = hapd->iface->num_bss;

	wpa_printf(MSG_DEBUG, "%s():%s\n", __func__, buf);

	/* To-Do */
	idx = get_hapd_by_ifname(hapd->iface, buf);
	if(idx < hapd->iface->num_bss)
		hostapd_remove_bss(hapd->iface, idx);
	else
		ret = -1;
	return ret;
}

/* Handler of delete VBSS STA.
 * Input:
 *	buf:	"<STA MAC in format xx:xx:xx:xx:xx:xx>"
 * Return value:
 *	<0:	error
 *	=0: OK
 */
int vbss_del_sta(struct hostapd_data *hapd, char *buf)
{
	int ret = 0;
	uint8_t sta_mac[ETH_ALEN];
	struct sta_info *sta;

	wpa_printf(MSG_INFO, "%s mac %s", __func__, buf);
	if (hwaddr_aton(buf, sta_mac)) {
		wpa_printf(MSG_ERROR, "failed to get STA MAC from cmd %s", buf);
		return -1;
	}

	sta = ap_get_sta(hapd, sta_mac);
	if (sta == NULL)
		return -1;
	if (sta->flags & WLAN_STA_AUTH) {
		mlme_deauthenticate_indication(
			hapd, sta, WLAN_REASON_BSS_TRANSITION_DISASSOC);
	}
	wpa_printf(MSG_INFO, "Removing station " MACSTR,
		   MAC2STR(sta->addr));
	ap_free_sta(hapd, sta);

	return ret;
}

int vbss_get_aid(struct hostapd_data *hapd, struct sta_info *sta)
{
	u16 aid = 0;
	struct hostapd_bss_config *conf = NULL;

	if (!hapd || !sta)
		return -1;
	if (hapd->conf)
		conf = hapd->conf;

	if (conf)
		aid = (conf->bssid[4] << 8 | conf->bssid[5]) & 0x7ff;

	if (aid < 1 || aid > 2007)
		return -1;
	sta->aid = aid;
	hapd->sta_aid[(sta->aid -1) / 32] |= BIT((sta->aid -1) % 32);
	return 0;
}

int vbss_disable_rekey(struct hostapd_data *hapd, char *buf,
		char *reply, size_t reply_len)
{
	uint8_t addr[ETH_ALEN];
	struct hostapd_data *hapd_data = NULL;
	struct sta_info *sta = NULL;
	int ret = 0;

	if (hwaddr_aton(buf, addr)) {
		wpa_printf(MSG_ERROR, "failed to get MAC from cmd %s", buf);

		return -1;
	}
	if (os_memcmp(hapd->conf->bssid, addr, ETH_ALEN))
		hapd_data = get_hapd_by_bssid(hapd->iface, addr);
	else
		hapd_data = hapd;

	if (hapd_data) {
			wpa_printf(MSG_DEBUG, "get MAC from cmd %s---" MACSTR, buf,
						MAC2STR(hapd_data->conf->bssid));
		if (hapd_data->wpa_auth) {
			ret = vbss_disable_gmk_rekey(hapd_data->wpa_auth);
			ret = vbss_disable_gtk_rekey(hapd_data->wpa_auth);
		}
		ret = vbss_disable_wep_rekey(hapd_data);

		for (sta = hapd_data->sta_list; sta; sta = sta->next) {
			if (sta->aid < 1 || sta->aid > 2007)
				continue;
			if(sta->wpa_sm && sta->wpa_sm->wpa_auth)
				ret = vbss_disable_ptk_rekey(sta->wpa_sm);
		}
	}
	return ret;
}

/* Set the VBSS enabled status: onoff=0/1 */
int vbss_init(void)
{
	int i = 0;

	wpa_printf(MSG_DEBUG, "%s", __func__);
	for (i = 0; i < NUM_NL80211_BANDS; i++)
		memset(&vbss_cb[i], 0, sizeof(struct hostapd_vbss_cb));

	return 0;
}

int vbss_set_hostapd_data_info(struct hostapd_data *hapd, int role)
{
	if (!hapd)
		return -1;

	if (vbss_check_enabled(hapd) <= 0)
		return -1;

	if (role)
		hapd->vbss_hostapd_info.role = VBSS_VAP_ROAMING;

	wpa_printf(MSG_INFO, "bss[%s]-role[%d]-hostapd_data_info.role[%d]",
			hapd->conf->iface, role, hapd->vbss_hostapd_info.role);
	return 0;
}

#else
int vbss_set_hidden_ssid(struct hostapd_data *hapd, int authorized) { return 0; }
int vbss_init_bss(struct hostapd_data *hapd, int first) { return 0; }
int vbss_get_enabled(char *cmd, char *buf, size_t buflen) { return 0; }
int vbss_set_enabled(struct hapd_interfaces *interfaces, char *cmd, char *buf, size_t buflen) { return 0; }
int vbss_get_sta_info(struct hostapd_data *hapd, char *buf, char *reply, size_t reply_len) { return 0; }
int vbss_add_sta(struct hostapd_data *hapd, struct vbss_sta_info *vbss_sta) { return 0; }
int vbss_init(void) { return 0;}
int vbss_trigger_switch(struct hostapd_data *hapd, char *buf) { return 0; }
int vbss_set_switch_done(struct hostapd_data *hapd, char *buf) { return 0; }
int vbss_del_vap(struct hostapd_data *hapd, char *buf) { return 0; }
int vbss_del_sta(struct hostapd_data *hapd, char *buf) { return 0; }
int vbss_set_hostapd_data_info(struct hostapd_data *hapd, int role) { return 0; }
#endif
#endif

