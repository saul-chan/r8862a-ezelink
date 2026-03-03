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

#ifndef CSIGMA_COMMON_H
#define CSIGMA_COMMON_H

#include <stdint.h>

#define MAC_ADDR_STR_LEN 18
#define MACLEN  (6)

enum {
	CLS_VERSION_LEN = 128,
	CLS_PROGRAMM_NAME_LEN = 16,
	CLS_IP_LEN = sizeof("xxx.xxx.xxx.xxx"),
	CLS_PASSWORD_LEN = 128,
	CLS_INTERFACE_LEN = 32,
	QNT_SSID_LEN = 64,
	QNT_ENCRYPTION_LEN = 32,
	QNT_NS_LEN = 64,
	QNT_WIFI_MODE_LEN = 64,
	CLS_MX_INTERFACES = 4,
	CLS_INTERFACE_LIST_LEN = CLS_INTERFACE_LEN * CLS_MX_INTERFACES,
	CLS_KEYMGNT_LEN = 64,
	CLS_COUNTRY_CODE_LEN = 16,
	CLS_SCAN_TIMEOUT_SEC = 100,
	CLS_MAX_RADIO_ID = 3,
	QNT_HE_MBSSID_IDX_START = 1,
	QNT_HE_MBSSID_IDX_MAX = QNT_HE_MBSSID_IDX_START + 2,
	CLS_HE_MBSSID_IDX_MAX = 8,
	CLS_SAE_PWE_LEN = 16,
	CLS_REG_DOMAIN_LEN = 16,
	CLS_GROUPMGNT_CIPHER_LEN = 32,
	CLS_ADDR_LEN = 18,
	CLS_MAX_NAS_ID = 32,
	CLS_MAX_AKMSUITE_LEN = 10
};

struct cls_ca_version {
	char version[CLS_VERSION_LEN];
};

struct cls_ap_get_info_result {
	char interface_list[CLS_INTERFACE_LIST_LEN];
	char agent_version[CLS_VERSION_LEN];
	char firmware_version[CLS_VERSION_LEN];
};

struct cls_ap_set_radius {
	char if_name[CLS_INTERFACE_LEN];
	char ip[CLS_IP_LEN];
	uint16_t port;
	char password[CLS_PASSWORD_LEN];
	int32_t vap_index;
};

enum cls_ca_ppdutxtype {
	CLS_CA_PPDUTXTYPE_SU = 0,
	CLS_CA_PPDUTXTYPE_MU = 1,
	CLS_CA_PPDUTXTYPE_ER = 2,
	CLS_CA_PPDUTXTYPE_TB = 3,
};

struct cls_chan_band {
	uint8_t band;
	int32_t chan;
};

struct cls_ap_set_wireless {
	char Name[CLS_PROGRAMM_NAME_LEN];
	char programm[CLS_PROGRAMM_NAME_LEN];
	char if_name[CLS_INTERFACE_LEN];
	char ssid[QNT_SSID_LEN];
	struct cls_chan_band chan_band[2];
	char mode[2][QNT_WIFI_MODE_LEN];

	uint8_t wmm;		//  wireless multimedia extensions: 0 - disable
	uint8_t has_wmm;

	uint8_t apsd;
	uint8_t has_apsd;

	int32_t rts_threshold;
	uint8_t has_rts_threshold;

	uint8_t power_save;	// 0 - disable
	uint8_t has_power_save;

	uint32_t beacon_interval;
	uint8_t has_beacon_interval;

	uint8_t rf_enable;	// 0 - disable, 1 - enable
	uint8_t has_rf_enable;

	uint8_t amsdu;
	uint8_t has_amsdu;

	uint32_t mcs_rate;
	uint8_t has_mcs_rate;

	char nss_rx[QNT_NS_LEN];
	char nss_tx[QNT_NS_LEN];

	int stbc_tx[2];		/* space-time block coding,
				 * [0] is number of spatial stream,  [1] - space time streams */
	uint8_t has_stbc_tx;

	uint16_t bandwidth;	// 0 - auto
	uint8_t has_bandwidth;

	uint16_t band_6G_only;
	uint8_t has_band_6G_only;

	uint32_t dtim;
	uint8_t has_dtim;

	uint8_t short_gi;	// Short GI
	uint8_t has_short_gi;

	uint8_t su_beamformer;
	uint8_t has_su_beamformer;

	uint8_t mu_beamformer;
	uint8_t has_mu_beamformer;

	char country_code[CLS_COUNTRY_CODE_LEN];

	uint8_t has_addba_reject;
	uint8_t addba_reject;

	uint8_t has_ampdu;
	uint8_t ampdu;

	uint8_t has_dyn_bw_sgnl;
	uint8_t dyn_bw_sgnl;

	uint8_t has_vht_tkip;
	uint8_t vht_tkip;

	uint8_t has_bw_sgnl;
	uint8_t bw_sgnl;

	uint8_t has_group_id;
	uint8_t group_id;

	uint8_t has_rts_force;
	uint8_t rts_force;

	uint8_t has_offset;
	uint8_t offset; /* 2 means above and 1 means below */

	uint8_t has_ldpc;
	uint8_t ldpc;

	uint8_t has_mu_ndpa_format;
	uint8_t mu_ndpa_format;

	uint8_t has_noackpolicy;
	uint8_t noackpolicy;

	uint8_t has_nss_mcs_cap;
	int nss_cap;
	int mcs_high_cap;

	uint8_t has_maxhe_mcs_1ss_rxmaplte80;
	uint8_t maxhe_mcs_1ss_rxmaplte80;

	uint8_t has_maxhe_mcs_2ss_rxmaplte80;
	uint8_t maxhe_mcs_2ss_rxmaplte80;

	uint8_t has_ppdutxtype;
	uint8_t ppdutxtype; /* see cls_ca_ppdutxtype */

	uint8_t has_ofdma;
	uint8_t ofdma; /* DL = 0, UL = 1, 2 = DL-20and80 */

	uint8_t has_mimo;
	uint8_t mimo; /* DL = 0, UL = 1 */

	uint8_t has_numusersofdma;
	uint8_t numusersofdma;

	uint8_t has_numsounddim;
	uint8_t numsounddim;

	uint8_t has_ltf_gi; /* HE NDP GI LTF */
	uint8_t ltf_gi; /* CLS_HE_NDP_GI_LTF_TYPE_0, CLS_HE_NDP_GI_LTF_TYPE_1 */

	uint8_t has_rualloctones;
	int8_t rualloctones_ctr;
	int rualloctones[4];

	uint8_t has_he_txopdurrtsthr;
	uint8_t he_txopdurrtsthr;

	uint8_t has_numnontxbss;
	uint8_t numnontxbss;

	uint8_t has_mbssid;
	uint8_t mbssid;

	
	uint8_t has_mbssidset;
	uint8_t mbssidset;

	uint8_t has_mmbssid;
	uint8_t mmbssid;

	uint8_t has_mmbssidset;
	uint8_t mmbssidset;

	uint8_t has_mbssid_profilelen;
	uint8_t mbssid_profilelen_min;
	uint8_t mbssid_profilelen_max;

	uint8_t has_nontxbssindex;
	uint8_t nontxbssindex;

	uint8_t has_minmpdustartspacing;
	uint8_t minmpdustartspacing;

	uint32_t vap_index;
	uint8_t unsolicitedproberesp;
	uint32_t cadence_unsolicitedproberesp;

	uint8_t has_ieee80211r;
	uint8_t ft_over_air;
	uint8_t ft_over_ds;
	char ft_bss_list[CLS_ADDR_LEN];
	char reg_domain[CLS_REG_DOMAIN_LEN];
	char domain[CLS_REG_DOMAIN_LEN];

	uint8_t has_addba_req_bufsize;
	uint8_t addba_req_bufsize_gt64;

	uint8_t has_bctwt;
	uint8_t bctwt;

	uint8_t has_bss_max_idle;
	uint8_t bss_max_idle_enable;
	uint16_t bss_max_idle_period;
	uint8_t has_twt_respsupport;
	uint8_t twt_respsupport;
	uint8_t has_muedca;
	uint8_t muedca;
	uint8_t ersudisable;
	uint8_t has_ersudisable;
	uint8_t has_omctrl_ulmudatadisablerx;
	uint8_t omctrl_ulmudatadisablerx;
	uint8_t has_fullbw_ulmumimo;
	uint8_t fullbw_ulmumimo;

	uint8_t has_filsdscv;
	uint8_t filsdscv;
	uint8_t has_bcn_frm_size;
	uint32_t bcn_frm_size;
};

enum cls_ca_trans_disable_idx {
	CLS_CA_TRANS_DISABLE_IDX_WPA3_PERSONAL = 0,
	CLS_CA_TRANS_DISABLE_IDX_SAE_PK = 1,
};

struct cls_ap_set_security {
	char keymgnt[CLS_KEYMGNT_LEN];
	char if_name[CLS_INTERFACE_LEN];
	char passphrase[CLS_PASSWORD_LEN];
	char wepkey[CLS_PASSWORD_LEN];
	char ssid[QNT_SSID_LEN];
	char ecc_grps[32];

	uint8_t pmf;		// same as clsapi_pmf
	uint8_t has_pmf;

	uint8_t sha256ad;	// 0 - disable
	uint8_t has_sha256ad;

	uint8_t has_nontxbssindex;
	uint8_t nontxbssindex;

	char encryption[QNT_ENCRYPTION_LEN];
	uint32_t vap_index;
	char sae_pwe[CLS_SAE_PWE_LEN];
	char GroupMgntCipher[CLS_GROUPMGNT_CIPHER_LEN];
	char AKMSuiteType[CLS_MAX_AKMSUITE_LEN];
	char SAEPasswords[CLS_PASSWORD_LEN];

	uint8_t transition_disable;
	uint8_t transition_disable_index; /* see cls_ca_trans_disable_idx */
};

struct cls_neigh_report_info {
	char bssid[MAC_ADDR_STR_LEN];
	uint32_t bssid_info;
	uint8_t opclass;
	uint8_t channel;
	uint8_t phytype;
	uint8_t prefer;
	uint8_t non_pref_by_sta;
};

struct cls_ap_bss_term_params {
	int include;
	int duration;
	int tsf;
};

struct cls_ap_set_rfeature {
	struct cls_ap_bss_term_params term;
	struct cls_neigh_report_info neighbor;
};

struct cls_dut_request {
	union {
		int empty_placeholder;
		struct cls_ap_set_radius ap_radius;
		struct cls_ap_set_wireless wireless;
		struct cls_ap_set_security secutiry;
		struct cls_ap_set_rfeature rfeat;
	};
};

struct cls_dut_response {
	int32_t status;
	int32_t clsapi_error;
	union {
		struct cls_ap_get_info_result ap_info;
		struct cls_ca_version ca_version;
	};
};

struct cls_vap {
	int band;
	char name[16];
};

extern int need_clear_reserve_all_slots;

#endif
