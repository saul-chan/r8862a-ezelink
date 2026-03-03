/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_WIFI_H
#define _CLSAPI_WIFI_H

#include <linux/if_ether.h>
#include <linux/nl80211.h>
#include <arpa/inet.h>
#include "clsapi_common.h"

#ifdef CLSAPI_PLAT_OPENWRT
#include <uci.h>
#endif
/*****************************	Variables & MARCROs definitions	**************************/
#define TID_MAX 9
#define CLS_WIFI_RADIO0			"phy0"
#define CLS_WIFI_RADIO1			"phy1"

#define CLSAPI_WIFI_SCAN_STATUS_FILE		"/tmp/.clsapi_scan_status_"
#define CLSAPI_WIFI_SCAN_RESULT_FILE		"/tmp/.clsapi_scan_result_"

#define CLSAPI_WIFI_SCAN_STATUS_RESET		""
#define CLSAPI_WIFI_SCAN_STATUS_SCANNING	"Scanning"
#define CLSAPI_WIFI_SCAN_STATUS_ABORTED		"Aborted"
#define CLSAPI_WIFI_SCAN_STATUS_COMPLETE	"Completed"

#define CLSAPI_WIFI_MAX_PRESHARED_KEY	4
#define CLSAPI_SSID_MAX_LEN				32

/******************	Configurations	******************/
#define CLSCONF_CFG_WIRELESS		"wireless"
#define CLSCONF_CFG_OPMODE		"cls-opmode"

#define CLSCONF_SEC_GLOBALS		"globals"

#define CLSCONF_CFG_CLS_MESH		"cls-mesh"
#define CLS_MESH_SECT_DEFAULT		"default"
#define CLS_MESH_PARAM_MODE			"mode"

#define CLSCONF_SEC_STATION_IFACE	"station_iface"

/*****************************	Data type definitions	**************************/

enum CLSAPI_WIFI_TXPWR_RATIO {
	CLSAPI_WIFI_TXPWR_RATIO_100 = 100,
	CLSAPI_WIFI_TXPWR_RATIO_75 = 75,
	CLSAPI_WIFI_TXPWR_RATIO_50 = 50,
	CLSAPI_WIFI_TXPWR_RATIO_25 = 25,
	__CLSAPI_WIFI_TXPWR_RATIO_MAX = 127
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

enum WPA_CIPHER_TYPE {
	WPA_CIPHER_NONE,
	WPA_CIPHER_AES,
	WPA_CIPHER_TKIP,
	WPA_CIPHER_TKIP_AES
};

enum clsapi_wifi_ampdu_protection {
	CLSAPI_WIFI_AMPDU_PRO_RTS_CTS = 0,
	CLSAPI_WIFI_AMPDU_PRO_CTS_ONLY,
	CLSAPI_WIFI_AMPDU_PRO_NONE,
	__CLSAPI_WIFI_AMPDU_PRO_MAX,
};

enum clsapi_wifi_wps_config_method {
	CLSAPI_WIFI_WPS_CFG_METHOD_NOCONFIG = 0,
	CLSAPI_WIFI_WPS_CFG_METHOD_LABEL = BIT(0),
	CLSAPI_WIFI_WPS_CFG_METHOD_DISPLAY = BIT(1),
	CLSAPI_WIFI_WPS_CFG_METHOD_KEYPAD = BIT(2),
	CLSAPI_WIFI_WPS_CFG_METHOD_PUSHBUTTON = BIT(3),
	CLSAPI_WIFI_WPS_CFG_METHOD_MAX
};

enum {
	VBSS_VAP_ORIGIN,
	VBSS_VAP_ROAMING,
	VBSS_VAP_ROLE_MAX
};

enum clsapi_wifi_wps_state {
	CLSAPI_WIFI_WPS_STATE_DISABLED,
	CLSAPI_WIFI_WPS_STATE_NOT_CONFIGURED,
	CLSAPI_WIFI_WPS_STATE_CONFIGURED
};

enum clsapi_wifi_interface_mode {
	CLSAPI_WIFI_INTERFACE_MODE_STA,
	CLSAPI_WIFI_INTERFACE_MODE_AP,
	CLSAPI_WIFI_INTERFACE_MODE_UNSPECIFIED
};

enum clsapi_wifi_scan_flags {
	//CLSAPI_WIFI_SCAN_FLAG_LOW_PRIORITY		= 1<<0,
	CLSAPI_WIFI_SCAN_FLAG_FLUSH		= 1<<1,
	//CLSAPI_WIFI_SCAN_FLAG_AP		= 1<<2,
	//CLSAPI_WIFI_SCAN_FLAG_RANDOM_ADDR			= 1<<3,
	//CLSAPI_WIFI_SCAN_FLAG_FILS_MAX_CHANNEL_TIME		= 1<<4,
	//CLSAPI_WIFI_SCAN_FLAG_ACCEPT_BCAST_PROBE_RESP		= 1<<5,
	//CLSAPI_WIFI_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE		= 1<<6,
	//CLSAPI_WIFI_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION		= 1<<7,
	//CLSAPI_WIFI_SCAN_FLAG_LOW_SPAN		= 1<<8,
	//CLSAPI_WIFI_SCAN_FLAG_LOW_POWER		= 1<<9,
	//CLSAPI_WIFI_SCAN_FLAG_HIGH_ACCURACY		= 1<<10,
	//CLSAPI_WIFI_SCAN_FLAG_RANDOM_SN		= 1<<11,
	//CLSAPI_WIFI_SCAN_FLAG_MIN_PREQ_CONTENT		= 1<<12,
	//CLSAPI_WIFI_SCAN_FLAG_FREQ_KHZ		= 1<<13,
	//CLSAPI_WIFI_SCAN_FLAG_COLOCATED_6GHZ		= 1<<14,
	CLSAPI_WIFI_SCAN_FLAG_OFF_CHANNEL		= 1<<15,
	__CLSAPI_WIFI_SCAN_FLAG_MAX				= 0xFF,
};

/** The parameters of scan operation */
struct clsapi_wifi_scan_params {
	/** The mask when filtering rx packets, default 0 means no filter.
     \n CLS_MAC_EN_DUPLICATE_DETECTION_BIT         ((uint32_t)0x80000000)
     \n CLS_MAC_ACCEPT_UNKNOWN_BIT                 ((uint32_t)0x40000000)
     \n CLS_MAC_ACCEPT_OTHER_DATA_FRAMES_BIT       ((uint32_t)0x20000000)
     \n CLS_MAC_ACCEPT_QO_S_NULL_BIT               ((uint32_t)0x10000000)
     \n CLS_MAC_ACCEPT_QCFWO_DATA_BIT              ((uint32_t)0x08000000)
     \n CLS_MAC_ACCEPT_Q_DATA_BIT                  ((uint32_t)0x04000000)
     \n CLS_MAC_ACCEPT_CFWO_DATA_BIT               ((uint32_t)0x02000000)
     \n CLS_MAC_ACCEPT_DATA_BIT                    ((uint32_t)0x01000000)
     \n CLS_MAC_ACCEPT_OTHER_CNTRL_FRAMES_BIT      ((uint32_t)0x00800000)
     \n CLS_MAC_ACCEPT_CF_END_BIT                  ((uint32_t)0x00400000)
     \n CLS_MAC_ACCEPT_ACK_BIT                     ((uint32_t)0x00200000)
     \n CLS_MAC_ACCEPT_CTS_BIT                     ((uint32_t)0x00100000)
     \n CLS_MAC_ACCEPT_RTS_BIT                     ((uint32_t)0x00080000)
     \n CLS_MAC_ACCEPT_PS_POLL_BIT                 ((uint32_t)0x00040000)
     \n CLS_MAC_ACCEPT_BA_BIT                      ((uint32_t)0x00020000)
     \n CLS_MAC_ACCEPT_BAR_BIT                     ((uint32_t)0x00010000)
     \n CLS_MAC_ACCEPT_OTHER_MGMT_FRAMES_BIT       ((uint32_t)0x00008000)
     \n CLS_MAC_ACCEPT_BFMEE_FRAMES_BIT            ((uint32_t)0x00004000)
     \n CLS_MAC_ACCEPT_ALL_BEACON_BIT              ((uint32_t)0x00002000)
     \n CLS_MAC_ACCEPT_NOT_EXPECTED_BA_BIT         ((uint32_t)0x00001000)
     \n CLS_MAC_ACCEPT_DECRYPT_ERROR_FRAMES_BIT    ((uint32_t)0x00000800)
     \n CLS_MAC_ACCEPT_BEACON_BIT                  ((uint32_t)0x00000400)
     \n CLS_MAC_ACCEPT_PROBE_RESP_BIT              ((uint32_t)0x00000200)
     \n CLS_MAC_ACCEPT_PROBE_REQ_BIT               ((uint32_t)0x00000100)
     \n CLS_MAC_ACCEPT_MY_UNICAST_BIT              ((uint32_t)0x00000080)
     \n CLS_MAC_ACCEPT_UNICAST_BIT                 ((uint32_t)0x00000040)
     \n CLS_MAC_ACCEPT_ERROR_FRAMES_BIT            ((uint32_t)0x00000020)
     \n CLS_MAC_ACCEPT_OTHER_BSSID_BIT             ((uint32_t)0x00000010)
     \n CLS_MAC_ACCEPT_BROADCAST_BIT               ((uint32_t)0x00000008)
     \n CLS_MAC_ACCEPT_MULTICAST_BIT               ((uint32_t)0x00000004)
     \n CLS_MAC_DONT_DECRYPT_BIT                   ((uint32_t)0x00000002)
     \n CLS_MAC_EXC_UNENCRYPTED_BIT                ((uint32_t)0x00000001)
     */
	uint32_t rx_filter;
	/** The duration of scan-work per scan, uint in ms */
	uint32_t work_duration;
	/** The interval of every scan operation */
	uint32_t scan_interval;
	/** The flags of scan */
	uint32_t flags;
};

/** Basic information in Mesh mode, such as backhaul bss ifname.. */
struct mesh_bss_info {
	/** Backhaul bss ifname */
	string_32 bh_ifname;

	/** Fronthaul bss ifname */
	string_32 fh_ifname;

	/** Backhaul sta ifname */
	string_32 bh_sta_ifname;
};

/** Infos of VBSS VAP */
struct vbss_vap_info {
	/** The MAC address of VAP */
	uint8_t bssid[ETH_ALEN];

	/** The name of VBSS */
	string_32 ssid;

	/** Interface name, such as wlan0, wlan1, Identify the network interface where this VAP is located */
	string_32 ifname;

	/** Security authentication method */
	enum WPA_AUTHORIZE_TYPE auth_type;

	/** Wireless password, up to 64 characters long */
	string_64 pwd;

	/** The role of VAP, eg: AP, STA */
	uint8_t role;
};

/** Statistics of management frame data transferred by vap */
struct clsapi_wifi_vap_mgmt_tx_stats {
	/** tx beacon frame count */
	uint64_t tx_bcnpkts;

	/** tx probe response frame count */
	uint64_t tx_probersppkts;

	/** tx auth frame count */
	uint64_t tx_authpkts;

	/** tx deauth frame count */
	uint64_t tx_deauthpkts;

	/** tx association request fram count */
	uint64_t tx_assocreqpkts;

	/** tx association response frame count */
	uint64_t tx_assocrsppkts;

	/** tx reassociation request fram count */
	uint64_t tx_reascreqpkts;

	/** tx reassociation response frame count */
	uint64_t tx_reascrsppkts;

	/** tx disassociation frame count */
	uint64_t tx_disassocpkts;
};

/** Statistics of management frame data received by vap */
struct clsapi_wifi_vap_mgmt_rx_stats {
	/** rx probe response frame count */
	uint64_t rx_probersppkts;

	/** rx auth frame count */
	uint64_t rx_authpkts;

	/** rx deauth frame count */
	uint64_t rx_deauthpkts;

	/** rx association request fram count */
	uint64_t rx_assocreqpkts;

	/** rx association response frame count */
	uint64_t rx_assocrsppkts;

	/** rx reassociation request fram count */
	uint64_t rx_reascreqpkts;

	/** rx reassociation response frame count */
	uint64_t rx_reascrsppkts;

	/** rx disassociation frame count */
	uint64_t rx_disassocpkts;
};

/** Get management frame statistics information */
struct clsapi_wifi_get_mgmt_stats_cfm {
	/** mgmt frame tx stats */
	struct clsapi_wifi_vap_mgmt_tx_stats tx_stats;

	/** mgmt frame rx stats */
	struct clsapi_wifi_vap_mgmt_rx_stats rx_stats;
};

/** Additional status information for vap */
struct _clsm_vap_extstats {
	/** The number of transmitted unicast packets to the vap */
	uint64_t tx_unicast;

	/** The number of transmitted multicast packets to the vap */
	uint64_t tx_multicast;

	/** The number of transmitted broadcast packets to the vap */
	uint64_t tx_broadcast;

	/** The number of received unicast packets from the vap */
	uint64_t rx_unicast;

	/** The number of received multicast packets from the vap */
	uint64_t rx_multicast;

	/** The number of received broadcast packets form the vap */
	uint64_t rx_broadcast;

	/** tx succeed of num */
	uint32_t tx_trans;

	/** tx failed(include retry failed) of num */
	uint32_t tx_ftrans;

	/** tx succeed but it have retry of num */
	uint32_t tx_retries;

	/** tx succeed but it have once retry of num */
	uint32_t tx_sretries;

	/** tx AMPDU of number, if send SMPDU, this add 0, other add 1 */
	uint32_t tx_aggpkts;

	/** tx no-ACK PPDU of number */
	uint32_t tx_toutpkts;

	/** mgmt tx stats & rx stats */
	struct clsapi_wifi_get_mgmt_stats_cfm mgmt_stats;
};
#define WPA_TK_MAX_LEN 32
#define WPA_GMK_LEN 32
#define WPA_GTK_MAX_LEN 32
#define WPA_IGTK_MAX_LEN 32
#define SAE_PMK_LEN 32
#define SAE_PMKID_LEN 16
#define WLAN_SUPP_RATES_MAX 32

/* WPS related definitions */
#define WPS_AP_PIN_MIN_LEN   (4)
#define WPS_AP_PIN_MAX_LEN   (8)

#define wps_config_methods_mask (CLSAPI_WIFI_WPS_CFG_METHOD_LABEL | CLSAPI_WIFI_WPS_CFG_METHOD_DISPLAY \
                                 | CLSAPI_WIFI_WPS_CFG_METHOD_KEYPAD |CLSAPI_WIFI_WPS_CFG_METHOD_PUSHBUTTON)

/** Store Wi-Fi security keys and algorithm information related to a specific vbss */
struct vbss_key_info {
	/** Authentication algorithm */
	uint16_t auth_alg;

	/** WPA algorithm */
	uint16_t wpa_alg;

	/** Temporal key for unicast encryption */
	uint8_t tk[WPA_TK_MAX_LEN];

	/** Group master key */
	uint8_t GMK[WPA_GMK_LEN];

	/** Group temporal key */
	uint8_t GTK[2][WPA_GTK_MAX_LEN];

	/** The length of group temporal key */
	int GTK_len;

	/** The number of group key */
	int GN;

	/** The index of group master key */
	int GM;

	/** Integrity group temporal key, Used when 802.11w enabled */
	uint8_t IGTK[2][WPA_IGTK_MAX_LEN];

	/** Beacon integrity group temporal key */
	uint8_t BIGTK[2][WPA_IGTK_MAX_LEN];

	/** IGTK group number */
	int GN_igtk;

	/** IGTK group master index */
	int GM_igtk;

	/** BIGTK group number, when beacon protection enabled */
	int GN_bigtk;

	/** BIGTK group master index */
	int GM_bigtk;

	/** Pairwise master key */
	uint8_t pmk[SAE_PMK_LEN];

	/** PMK identifier */
	uint8_t pmkid[SAE_PMKID_LEN];
};

/** High Throughput capabilities */
struct ieee80211_ht_capabilities {
	/** A 16-bit bitfield containing various HT capability flags */
	uint16_t ht_capabilities_info;

	/** Parameters related to A-MPDU (Aggregate MAC Protocol Data Unit) reception */
	uint8_t a_mpdu_params;

	/** A 16-byte (128-bit) bitmask set representing the supported Modulation and Coding Scheme (MCS) rates */
	uint8_t supported_mcs_set[16];

	/** A 16-bit bitfield containing extended HT capabilities added in later revisions of the standard */
	uint16_t ht_extended_capabilities;

	/** A 32-bit bitfield describing the Transmit Beamforming (TxBF) capabilities */
	uint32_t tx_bf_capability_info;

	/** An 8-bit bitfield indicating Antenna Selection (ASEL) capability flags */
	uint8_t asel_capabilities;
} STRUCT_PACKED;

/** Very High Throughput capabilities */
struct ieee80211_vht_capabilities {
	/** A 32-bit bitfield containing core VHT capabilities */
	uint32_t vht_capabilities_info;

	/** VHT-MCS rate information */
	struct {
		/** The supported receive VHT-MCSs for different numbers of spatial streams */
		uint16_t rx_map;

		/** Indicates the highest data rate the STA can receive */
		uint16_t rx_highest;

		/** The supported transmit VHT-MCSs for different numbers of spatial streams */
		uint16_t tx_map;

		/** Indicates the highest data rate the STA can transmit */
		uint16_t tx_highest;
	} vht_supported_mcs_set;
} STRUCT_PACKED;

/** Operational parameters of VHT */
struct ieee80211_vht_operation {
	/** The operating channel width */
	uint8_t vht_op_info_chwidth;

	/** The channel center frequency index for segment0 */
	uint8_t vht_op_info_chan_center_freq_seg0_idx;

	/** The channel center frequency index for segment1 */
	uint8_t vht_op_info_chan_center_freq_seg1_idx;

	/** Indicating the VHT-MCS values that all STAs associated with this BSS must support (the Basic VHT-MCS set) */
	uint16_t vht_basic_mcs_set;
} STRUCT_PACKED;

/** High Efficiency (HE / 802.11ax / Wi-Fi 6) capabilities */
struct ieee80211_he_capabilities {
	/** A 6-byte array containing bitfields for HE MAC capabilities */
	uint8_t he_mac_capab_info[6];

	/** An 11-byte array containing bitfields for HE PHY capabilities */
	uint8_t he_phy_capab_info[11];

	/** Optional capabilities */
	uint8_t optional[37];
} STRUCT_PACKED;
#define IEEE80211_HE_CAPAB_MIN_LEN (6 + 11)

/** HE capabilities specific to operation in the 6 GHz band */
struct ieee80211_he_6ghz_band_cap {
	/** A 16-bit bitfield containing capabilities specific to the 6 GHz band */
	uint16_t capab;
} STRUCT_PACKED;

/** The sequence number contained in the data frame belonging to a vbss sent by sta */
struct vbss_sta_seq_num {
	/** Station index */
	uint16_t sta_idx;

	/** Transmit packet number */
	uint64_t tx_pn;

	/** Receive packet number per TID
	 *
	 * TID: Traffic identifier
	 */
	uint64_t rx_pn[TID_MAX];

	/** Transmit sequence number per TID */
	uint32_t tx_seq_num[TID_MAX];

	/** Receive Sequence number per TID */
	uint32_t rx_seq_num[TID_MAX];
};

/** Parameters related to smart antenna configuration requests */
struct mm_smart_antenna_req {
	/** bit0: enable;
	 *
	 * bit 1: update mode(0:sw, 1:hw);
	 *
	 * bit(2-3): gpio mode
	 */
	uint8_t enable;

	/** The high 32 bits of the "current value"_hi for the smart antenna configuration */
	uint32_t curval_hi;

	/** The low 32 bits of the "current value"_hi for the smart antenna configuration */
	uint32_t curval_lo;

	/** The high 32 bits of the smart antenna configuration "reset value" */
	uint32_t rstval_hi;

	/** The low 32 bits of the smart antenna configuration "reset value" */
	uint32_t rstval_lo;
};

/** STA info set to driver or get from driver */
struct cls_vbss_driver_sta_info {
	/**  store the MAC address of the wireless station */
	uint8_t mac[ETH_ALEN];

	/** Contains sequence number information related to station */
	struct vbss_sta_seq_num sta_seq_info;
};

/** Comprehensive status information of wireless stations associated with vbss */
struct vbss_sta_info {
	/** The MAC address of the wireless station */
	uint8_t mac_addr[ETH_ALEN];

	/** The MAC address of vbss */
	uint8_t bssid[ETH_ALEN];

	/** Association ID for the sta */
	uint16_t aid;

	/** represent various status flags for the station
	 *
	 * e.g. associated, authenticated, power save mode, WMM/QoS support, etc
	 */
	uint32_t flags;

	/** Support rates */
	uint8_t supported_rates[WLAN_SUPP_RATES_MAX];

	/** The number of valid rates in the supported_rates array */
	int supported_rates_len;

	/** Ability information fields in associated request/response or beacon frames */
	uint16_t capability;

	/** The High Throughput (802.11n) capability information of the station */
	struct ieee80211_ht_capabilities ht_capabilities;

	/** The Very High Throughput (802.11ac) capability information of the station */
	struct ieee80211_vht_capabilities vht_capabilities;

	/** The current VHT operational parameters being used with this station */
	struct ieee80211_vht_operation vht_operation;

	/** VHT operation mode notification field */
	uint8_t vht_opmode;

	/** The High Efficiency (802.11ax / Wi-Fi 6) capability information of the station */
	struct ieee80211_he_capabilities he_capab;

	/** The length of the HE capabilities data */
	size_t he_capab_len;

	/** The HE capability information of the station specifically for the 6GHz band (Wi-Fi 6E) */
	struct ieee80211_he_6ghz_band_cap he_6ghz_capab;

	/** Containing security key information associated with this station in the VBSS context */
	struct vbss_key_info key_info;

	/** STA info set to driver or get from driver */
	struct cls_vbss_driver_sta_info driver_sta;
};

/** An entry containing the results of a wireless channel survey */
struct survey_entry {
	/** The total duration the survey was active on this specific channe */
	uint64_t active_time;

	/** The total duration the radio detected the channel was busy */
	uint64_t busy_time;

	/** The total duration the radio detected activity specifically on an extension channel being busy */
	uint64_t busy_time_ext;

	/** The total duration the radio was actively in the receiving state on this channel */
	uint64_t rxtime;

	/** The total duration the radio was actively in the transmitting state on this channel */
	uint64_t txtime;

	/** The center frequency of the channel to which statistics apply */
	uint32_t mhz;

	/** The average noise floor level measured on this channel */
	int8_t noise;
};

/** Per AP/Radio channel related info */
struct mac_chan_op {
	/** Radio band */
	uint8_t band;

	/** Channel type (refer to driver layer mac_chan_bandwidth) */
	uint8_t bw;

	/** Frequency for Primary 20MHz channel (in MHz) */
	uint16_t prim20_freq;

	/** Frequency center of the contiguous channel or center of Primary 80+80 (in MHz) */
	uint16_t center1_freq;

	/** Frequency center of the non-contiguous secondary 80+80 (in MHz) */
	uint16_t center2_freq;

	/** Additional information (refer to driver layer mac_chan_flags) */
	uint16_t flags;

	/** Max transmit power allowed on this channel (dBm) */
	int8_t tx_power;
};


/** CCA information */
struct vbss_cca_report {
	/** Current working channel */
	struct mac_chan_op chan;

	/** Channel duration */
	uint32_t chan_dur;

	/** CCA busy time (us) on each subchannel */
	uint32_t cca_busy_dur;

	/** CCA busy duration for a secondary 20 MHz subchannel */
	uint32_t cca_busy_dur_sec20;

	/** CCA busy duration for a secondary 40 MHz subchannel */
	uint32_t cca_busy_dur_sec40;

	/** CCA busy duration for a secondary 80 MHz subchannel */
	uint32_t cca_busy_dur_sec80;
};

/** Per AP/radio stats */
struct vbss_ap_stats {
	/** CCA information */
	struct vbss_cca_report cca_info;

	/** The number of transmitted (Tx) MAC Protocol Data Units */
	uint32_t tx_mpdu_num;

	/**  The total number of bytes transmitted */
	uint32_t tx_bytes;

	/** The number of received MPDUs */
	uint32_t rx_mpdu_num;

	/** The total number of bytes received */
	uint32_t rx_bytes;
};

/** Channel scores */
struct chan_score {
	/** Channel number */
	uint8_t chan;

	/** The calculated score for this channel */
	uint16_t score;
};

/** Encapsulates the information required to configure a RADIUS server */
struct clsapi_radius_configure {
	/** The ip address of server */
	string_32 server_ip;

	/** The port of server */
	uint32_t server_port;

	/** The shared secret or passphrase used for authenticating communication between the device and the RADIUS server */
	string_32 server_passphrase;
};

/* bandwidth enum definition, mapping to enum nl80211_chan_width but filtered un-supported ones */
enum clsapi_wifi_bw {
	CLSAPI_WIFI_BW_20_NOHT,
	CLSAPI_WIFI_BW_20,
	CLSAPI_WIFI_BW_40,
	CLSAPI_WIFI_BW_80,

	/* CLS Wi-Fi doesn't support 80+80MHz, only use it in capability of scanned other APs */
	CLSAPI_WIFI_BW_80_80,
	CLSAPI_WIFI_BW_160,

	/* Keep MAX at bottom */
	CLSAPI_WIFI_BW_MAX,
	CLSAPI_WIFI_BW_DEFAULT = 0xFF,
};

#define STA_DRV_DATA_TX_HE_MCS BIT(0)
#define STA_DRV_DATA_RX_HE_MCS BIT(1)
#define STA_DRV_DATA_TX_VHT_MCS BIT(2)
#define STA_DRV_DATA_RX_VHT_MCS BIT(3)
#define STA_DRV_DATA_TX_VHT_NSS BIT(4)
#define STA_DRV_DATA_RX_VHT_NSS BIT(5)
#define STA_DRV_DATA_TX_SHORT_GI BIT(6)
#define STA_DRV_DATA_RX_SHORT_GI BIT(7)
#define STA_DRV_DATA_LAST_ACK_RSSI BIT(8)
#define STA_DRV_DATA_CONN_TIME BIT(9)
#define STA_DRV_DATA_TX_HE_NSS BIT(10)
#define STA_DRV_DATA_RX_HE_NSS BIT(11)
#define STA_DRV_DATA_TX_MCS BIT(12)
#define STA_DRV_DATA_RX_MCS BIT(13)

/** Store relevant information and statistical data of a device connected to its upstream link root ap */
struct uplink_info {
	/** The mac address of root ap */
	uint8_t rootap_mac[ETH_ALEN];

	/** Number of packets transmitted */
	unsigned long tx_packets;

	/** Number of packets received */
	unsigned long rx_packets;

	/** Total number of bytes transmitted */
	unsigned long tx_bytes;

	/** Total number of bytes received */
	unsigned long rx_bytes;

	/** Amount of airtime consumed for transmissions */
	unsigned long tx_airtime;

	/** Amount of airtime consumed by receptions */
	unsigned long rx_airtime;

	/** Whether 64-bit byte counters are supported */
	int bytes_64bit;

	/** The current data rate used for transmissions */
	unsigned long current_tx_rate;

	/** The current data rate observed for receptions */
	unsigned long current_rx_rate;

	/** Time in milliseconds since the last activity was detected on the uplink connection */
	unsigned long inactive_msec;

	/** Duration of established connection */
	unsigned long connected_sec;

	/** Bitfield of STA_DRV_DATA_* */
	unsigned long flags;

	/** Number of frames buffered because the root ap is in power save mode */
	unsigned long num_ps_buf_frames;

	/** Number of transmitted packets that ultimately failed after exhausting retry attempts */
	unsigned long tx_retry_failed;

	/** Total number of transmission retries performed */
	unsigned long tx_retry_count;

	/** RSSI of the last received Acknowledgement frame */
	int8_t last_ack_rssi;

	/** Number of packets currently queued for transmission */
	unsigned long backlog_packets;

	/** Total size in bytes of the queued packets */
	unsigned long backlog_bytes;

	/** General signal strength indicator for the connection */
	int8_t signal;

	/** The VHT MCS index observed for reception */
	uint8_t rx_vhtmcs;

	/** The VHT MCS index observed for transmission */
	uint8_t tx_vhtmcs;

	/** The HE MCS index observed for reception */
	uint8_t rx_he_mcs;

	/** The HE MCS index observed for transmission */
	uint8_t tx_he_mcs;

	/** The MCS index observed for reception */
	uint8_t rx_mcs;

	/** The MCS index observed for transmission */
	uint8_t tx_mcs;

	/** The HE NSS observed for reception */
	uint8_t rx_he_nss;

	/** The HE NSS observed for transmission */
	uint8_t tx_he_nss;

	/** The VHT NSS observed for reception */
	uint8_t rx_vht_nss;

	/** The VHT NSS observed for transmission */
	uint8_t tx_vht_nss;

	/** Channel bandwidth */
	enum clsapi_wifi_bw bandwidth;
};

/** Store information and statistical data related to specific wireless client stations connected to the current device */
struct sta_info {
	/** The mac address of sta */
	uint8_t mac[ETH_ALEN];

	/** Number of packets transmitted */
	unsigned long tx_packets;

	/** Number of packets received */
	unsigned long rx_packets;

	/** Total number of bytes transmitted */
	unsigned long tx_bytes;

	/** Total number of bytes received */
	unsigned long rx_bytes;

	/** Amount of airtime consumed for transmissions */
	unsigned long tx_airtime;

	/** Amount of airtime consumed by receptions */
	unsigned long rx_airtime;

	/** Whether 64-bit byte counters are supported */
	int bytes_64bit;

	/** The current data rate used for transmissions */
	unsigned long current_tx_rate;

	/** The current data rate observed for receptions */
	unsigned long current_rx_rate;

	/** Time in milliseconds since the last activity was detected on the uplink connection */
	unsigned long inactive_msec;

	/** Duration of established connection */
	unsigned long connected_sec;

	/** Bitfield of STA_DRV_DATA_* */
	unsigned long flags;

	/** Number of frames buffered because the root ap is in power save mode */
	unsigned long num_ps_buf_frames;

	/** Number of transmitted packets that ultimately failed after exhausting retry attempts */
	unsigned long tx_retry_failed;

	/** Total number of transmission retries performed */
	unsigned long tx_retry_count;

	/** RSSI of the last received Acknowledgement frame */
	int8_t last_ack_rssi;

	/** Number of packets currently queued for transmission */
	unsigned long backlog_packets;

	/** Total size in bytes of the queued packets */
	unsigned long backlog_bytes;

	/** General signal strength indicator for the connection */
	int8_t signal;

	/** The VHT MCS index observed for reception */
	uint8_t rx_vhtmcs;

	/** The VHT MCS index observed for transmission */
	uint8_t tx_vhtmcs;

	/** The HE MCS index observed for reception */
	uint8_t rx_he_mcs;

	/** The HE MCS index observed for transmission */
	uint8_t tx_he_mcs;

	/** The MCS index observed for reception */
	uint8_t rx_mcs;

	/** The MCS index observed for transmission */
	uint8_t tx_mcs;

	/** The HE NSS observed for reception */
	uint8_t rx_he_nss;

	/** The HE NSS observed for transmission */
	uint8_t tx_he_nss;

	/** The VHT NSS observed for reception */
	uint8_t rx_vht_nss;

	/** The VHT NSS observed for transmission */
	uint8_t tx_vht_nss;

	/** Channel bandwidth */
	enum clsapi_wifi_bw bandwidth;
};

enum clsapi_wifi_iftype {
	CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE,
	CLSAPI_WIFI_IFTYPE_AP,
	CLSAPI_WIFI_IFTYPE_STA,

	CLSAPI_WIFI_IFTYPE_MAX
};

// mapping to kernel definition
enum operating_standard {
	OPERATING_STANDARD_11A = 0,
	OPERATING_STANDARD_11B,
	OPERATING_STANDARD_11G,
	OPERATING_STANDARD_11N,
	OPERATING_STANDARD_11AC,
	OPERATING_STANDARD_11AX,
	OPERATING_STANDARD_11BE,
	OPERATING_STANDARD_UNKNOWN
};

// Wi-Fi hwmode (operating standard, IEEE802.11a/b/g/n/ac/ax)
enum clsapi_wifi_hwmode {
	CLSAPI_HWMODE_IEEE80211_A  = (1 << OPERATING_STANDARD_11A),
	CLSAPI_HWMODE_IEEE80211_B  = (1 << OPERATING_STANDARD_11B),
	CLSAPI_HWMODE_IEEE80211_G  = (1 << OPERATING_STANDARD_11G),
	CLSAPI_HWMODE_IEEE80211_N  = (1 << OPERATING_STANDARD_11N),
	CLSAPI_HWMODE_IEEE80211_AC = (1 << OPERATING_STANDARD_11AC),
	CLSAPI_HWMODE_IEEE80211_AX = (1 << OPERATING_STANDARD_11AX),
};

/* band enum definition, mapping to enum nl80211_band but filtered un-supported ones */
enum clsapi_wifi_band {
	CLSAPI_BAND_2GHZ,
	CLSAPI_BAND_5GHZ,
	CLSAPI_BAND_RSV,
	CLSAPI_BAND_6GHZ,
	CLSAPI_BAND_MAX,
	CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX,
	CLSAPI_BAND_DEFAULT = 0xFF,
};

/* Send empty SSID in beacons and ignore probe request frames that do not
specify full SSID, mapping to hostapd.conf "ignore_broadcast_ssid" */
enum clsapi_wifi_hidden_ssid {
	/* Disable hidden ssid, that is broadcast SSID normally */
	CLSAPI_WIFI_HIDDEN_SSID_DISABLE,

	/* send empty (length=0) SSID in beacon and ignore probe request for broadcast SSID */
	CLSAPI_WIFI_HIDDEN_SSID_EMPTY,

	/* clear SSID (ASCII 0), but keep the original length (this may be required with some
	   clients that do not support empty SSID) and ignore probe requests for broadcast SSID */
	CLSAPI_WIFI_HIDDEN_SSID_CLEAR,

	CLSAPI_WIFI_HIDDEN_SSID_MAX
};

enum clsapi_wifi_pmf_enable {
	CLSAPI_WIFI_PMF_DISABLED,
	CLSAPI_WIFI_PMF_OPTIONAL,
	CLSAPI_WIFI_PMF_REQUIRED,
	CLSAPI_WIFI_PMF_MAX,
};

enum clsapi_wifi_mesh_role {
	/* Non-Mesh enabled on this device */
	CLSAPI_WIFI_MESH_ROLE_NONE,

	/* Device is Mesh controller */
	CLSAPI_WIFI_MESH_ROLE_CONTROLLER,

	/* Device is Mesh agent */
	CLSAPI_WIFI_MESH_ROLE_AGENT,

	/* Mesh role is automatically determined by algorithm */
	CLSAPI_WIFI_MESH_ROLE_AUTO,

	/* Mesh role max, keep it at last */
	CLSAPI_WIFI_MESH_ROLE_MAX,
};

enum clsapi_wifi_macfilter_policy {
	/* Wi-Fi MAC filter is disabled */
	CLSAPI_WIFI_MACFILTER_POLICY_DISABLED,

	/* Wi-Fi MAC filter policy is ALLOW: STAs in the list are allowed (then facing cridential check);
	 * others will be denied directly */
	CLSAPI_WIFI_MACFILTER_POLICY_ALLOW,

	/* Wi-Fi MAC filter policy is DENY: STAs in the list are denied directly;
	 * others will be allowed (then facing cridential check) */
	CLSAPI_WIFI_MACFILTER_POLICY_DENY,

	/* Keep this at bottom */
	CLSAPI_WIFI_MACFILTER_POLICY_MAX
};

/** Scanned encrypted information used by wireless networks */
struct clsapi_wifi_sanned_crypto {
#define CLSAPI_WIFI_SEC_PROTO_OPEN_NONE	(0)			// No authentication, No encryption
#define CLSAPI_WIFI_SEC_PROTO_WEP		(1 << 0)	// WEP
#define CLSAPI_WIFI_SEC_PROTO_WPA		(1 << 1)	// WPA
#define CLSAPI_WIFI_SEC_PROTO_WPA2		(1 << 2)	// WPA2
#define CLSAPI_WIFI_SEC_PROTO_WPA3		(1 << 3)	// WPA3
#define CLSAPI_WIFI_SEC_PROTO_OWE		(1 << 4)	// OWE
	/** Bitmap of Wi-Fi security protocol:
	 *
	 * CLSAPI_WIFI_SEC_PROTO_OPEN_NONE (0): No authentication, No encryption
	 *
	 * CLSAPI_WIFI_SEC_PROTO_WEP (1 << 0): WEP
	 *
	 * CLSAPI_WIFI_SEC_PROTO_WPA (1 << 1): WPA
	 *
	 * CLSAPI_WIFI_SEC_PROTO_WPA2 (1 << 2): WPA2
	 *
	 * CLSAPI_WIFI_SEC_PROTO_WPA3 (1 << 3): WPA3
	 *
	 * CLSAPI_WIFI_SEC_PROTO_OWE (1 << 4): OWE
	 */
	uint32_t sec_protos;

#define CLSAPI_WIFI_AUTH_ALG_OPEN		(1 << 0)	// Open System Authentication, OPEN-NONE/WPA-PSK/WPA2-PSK/WPA1~3-EAP
#define CLSAPI_WIFI_AUTH_ALG_SHARED		(1 << 1)	// Shared Key Authentication, only for WEP
#define CLSAPI_WIFI_AUTH_ALG_SAE		(1 << 2)	// SAE, WPA3-SAE
	/** Bitmap of authentication modes:
	 *
	 * CLSAPI_WIFI_AUTH_ALG_OPEN (1 << 0): Open System Authentication, OPEN-NONE/WPA-PSK/WPA2-PSK/WPA1~3-EAP
	 *
	 * CLSAPI_WIFI_AUTH_ALG_SHARED (1 << 1): Shared Key Authentication, only for WEP
	 *
	 * CLSAPI_WIFI_AUTH_ALG_SAE (1 << 2): SAE, WPA3-SAE
	 */
	uint32_t auth_modes;

#define CLSAPI_WIFI_KEY_MGMT_NONE		0			// No key management
#define CLSAPI_WIFI_KEY_MGMT_8021X		(1 << 1)	// Enterprise or EAP
#define CLSAPI_WIFI_KEY_MGMT_PSK		(1 << 2)	// Pre-Shared Key
#define CLSAPI_WIFI_KEY_MGMT_PSK256		(1 << 6)	// Pre-Shared Key using SHA256
#define CLSAPI_WIFI_KEY_MGMT_SAE		(1 << 8)	// SAE
#define CLSAPI_WIFI_KEY_MGMT_OWE		(1 << 18)	// OWE
	/** Bitmap of Key management suites:
	 *
	 * CLSAPI_WIFI_KEY_MGMT_NONE (0): No key management
	 *
	 * CLSAPI_WIFI_KEY_MGMT_8021X (1 << 1): Enterprise or EAP
	 *
	 * CLSAPI_WIFI_KEY_MGMT_PSK (1 << 2): Pre-Shared Key
	 *
	 * CLSAPI_WIFI_KEY_MGMT_PSK256 (1 << 6): Pre-Shared Key using SHA256
	 *
	 * CLSAPI_WIFI_KEY_MGMT_SAE (1 << 8): SAE
	 *
	 * CLSAPI_WIFI_KEY_MGMT_OWE (1 << 18): OWE
	 */
	uint32_t auth_suites;

#define CLSAPI_WIFI_CIPHER_NONE			0			// No cipher needed
#define CLSAPI_WIFI_CIPHER_WEP40		(1 << 1)	// WEP40, 5-digit WEP key
#define CLSAPI_WIFI_CIPHER_TKIP			(1 << 2)	// TKIP
#define CLSAPI_WIFI_CIPHER_WRAP			(1 << 3)	// WRAP
#define CLSAPI_WIFI_CIPHER_CCMP			(1 << 4)	// CCMP
#define CLSAPI_WIFI_CIPHER_WEP104		(1 << 5)	// WEP104, 13-digit WEP key
#define CLSAPI_WIFI_CIPHER_BIP_CMAC128	(1 << 6)	// BIP-CMAC-128
#define CLSAPI_WIFI_CIPHER_CKIP			(1 << 7)	// CKIP
#define CLSAPI_WIFI_CIPHER_GCMP			(1 << 8)	// GCMP
#define CLSAPI_WIFI_CIPHER_GCMP256		(1 << 9)	// GCMP256
#define CLSAPI_WIFI_CIPHER_CCMP256		(1 << 10)	// CCMP256
#define CLSAPI_WIFI_CIPHER_BIP_GMAC128	(1 << 11)	// BIP-GMAC-128
#define CLSAPI_WIFI_CIPHER_BIP_GMAC256	(1 << 12)	// BIP-GMAC-256
#define CLSAPI_WIFI_CIPHER_BIP_CMAC256	(1 << 13)	// BIP-CMAC-256
	/** Bitmap of cipher (for group_ciphers and pair_ciphers) suite selector:
	 *
	 * CLSAPI_WIFI_CIPHER_NONE	(0): No cipher needed
	 *
	 * CLSAPI_WIFI_CIPHER_WEP40	(1 << 1): WEP40, 5-digit WEP key
	 *
	 * CLSAPI_WIFI_CIPHER_TKIP	(1 << 2): TKIP
	 *
	 * CLSAPI_WIFI_CIPHER_WRAP  (1 << 3): WRAP
	 *
	 * CLSAPI_WIFI_CIPHER_CCMP	(1 << 4): CCMP
	 *
	 * CLSAPI_WIFI_CIPHER_WEP104 (1 << 5): WEP104, 13-digit WEP key
	 *
	 * CLSAPI_WIFI_CIPHER_BIP_CMAC128  (1 << 6): BIP-CMAC-128
	 *
	 * CLSAPI_WIFI_CIPHER_CKIP  (1 << 7): CKIP
	 *
	 * CLSAPI_WIFI_CIPHER_GCMP  (1 << 8): GCMP
	 *
	 * CLSAPI_WIFI_CIPHER_GCMP256 (1 << 9): GCMP256
	 *
	 * CLSAPI_WIFI_CIPHER_CCMP256 (1 << 10): CCMP256
	 *
	 * CLSAPI_WIFI_CIPHER_BIP_GMAC128  (1 << 11): BIP-GMAC-128
	 *
	 * CLSAPI_WIFI_CIPHER_BIP_GMAC256  (1 << 12): BIP-GMAC-256
	 *
	 * CLSAPI_WIFI_CIPHER_BIP_CMAC256  (1 << 13): BIP-CMAC-256
	 */
	uint32_t group_ciphers;

	/* Bitmap of cipher suits for unicast frames, refer to CLSAPI_WIFI_CIPHER_xx */
	uint32_t pair_ciphers;
};

/** struct to carry info of one scanned AP entry */
struct clsapi_scan_ap_info {
	/** BSSID */
	uint8_t bssid[ETH_ALEN];

	/** SSID */
	char ssid[CLSAPI_SSID_MAX_LEN + 1];

	/** Channel */
	uint8_t channel;

	/** RSSI */
	int8_t rssi;

	/** Bitmap of supported hwmodes (operating standard/protocol, IEEE802.11a/b/g/n/ac/ax),
	 * refer to enum clsapi_wifi_hwmode
	 */
	uint16_t hwmodes;

	/** Maximun supported bandwidth */
	uint16_t max_bw;

#define CLSAPI_WIFI_AP_FLAG_AP_MODE		(1 << 0)	// Scanned device is in AP mode.
#define CLSAPI_WIFI_AP_FLAG_WDS_MODE	(1 << 1)	// Scanned device is in WDS mode.
#define CLSAPI_WIFI_AP_FLAG_WPS_CAPA	(1 << 2)	// Have WPS capability or enabled
	/** Bitmap of AP flags:
	 *
	 * CLSAPI_WIFI_AP_FLAG_AP_MODE (1 << 0): Scanned device is in AP mode
	 *
	 * CLSAPI_WIFI_AP_FLAG_WDS_MODE (1 << 1): Scanned device is in WDS mode
	 *
	 * CLSAPI_WIFI_AP_FLAG_WPS_CAPA (1 << 2): Have WPS capability or enabled
	 */
	uint32_t flags;

	/** AP supported security capabilities */
	struct clsapi_wifi_sanned_crypto crypto;
};

/* CLS-API supported Wi-Fi security configuration. Wi-Fi security configuration is combination of:
 *   o Security protocol/standard:
 *       - None, WEP, WPA, WPA2, WPA3, OWE
 *   o Authentication type / key management:
 *       - PSK (Personal), 802.1X (EAP), SAE, OWE, ...
 *   o Cipher suites:
 *       - WEP, TKIP, AES/CCMP, ...
 */
enum clsapi_wifi_encryption {
	/* security protocol: None,	auth type/Key mgmt: None, cipher: None */
	CLSAPI_WIFI_ENCRYPTION_OPEN_NONE = 0,

	/* Security protocol: WEP, auth algorithm/mode: Shared KeyKey mgmt:/Key mgmt: None, cipher: WEP40/104 */
	CLSAPI_WIFI_ENCRYPTION_WEP_MIXED,

	/* Security protocol: WPA Personal,					auth type/Key mgmt: PSK,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP,

	/* Security protocol: WPA Personal,					auth type/Key mgmt: PSK,	cipher: TKIP */
	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP,

	/* Security protocol: WPA Personal,					auth type/Key mgmt: PSK,	cipher: TKIP/CCMP mixed*/
	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP,

	/* Security protocol: WPA Enterprise,				auth type/Key mgmt: EAP,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP,

	/* Security protocol: WPA Enterprise,				auth type/Key mgmt: EAP,	cipher: TKIP */
	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP,

	/* Security protocol: WPA Enterprise,				auth type/Key mgmt: EAP,	cipher: TKIP/CCMP mixed*/
	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP,

	/* Security protocol: WPA2 Personal,				auth type/Key mgmt: PSK,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP,

	/* Security protocol: WPA2 Personal,				auth type/Key mgmt: PSK,	cipher: TKIP */
	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP,

	/* Security protocol: WPA2 Personal,				auth type/Key mgmt: PSK,	cipher: TKIP/CCMP mixed*/
	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP,

	/* Security protocol: WPA2 Enterprise,				auth type/Key mgmt: EAP,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP,

	/* Security protocol: WPA2 Enterprise,				auth type/Key mgmt: EAP,	cipher: TKIP */
	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP,

	/* Security protocol: WPA2 Enterprise,				auth type/Key mgmt: EAP,	cipher: TKIP/CCMP mixed*/
	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP,

	/* Security protocol: WPA/WPA2 Personal mixed,		auth type/Key mgmt: PSK,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP,

	/* Security protocol: WPA/WPA2 Personal mixed,		auth type/Key mgmt: PSK,	cipher: TKIP */
	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP,

	/* Security protocol: WPA/WPA2 Personal mixed,		auth type/Key mgmt: PSK,	cipher: TKIP/CCMP mixed*/
	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP,

	/* Security protocol: WPA/WPA2 Enterprise mixed,	auth type/Key mgmt: EAP,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP,

	/* Security protocol: WPA/WPA2 Enterprise mixed,	auth type/Key mgmt: EAP,	cipher: TKIP */
	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP,

	/* Security protocol: WPA/WPA2 Enterprise mixed,	auth type/Key mgmt: EAP,	cipher: TKIP/CCMP mixed*/
	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP,

	/* Security protocol: WPA3 Personal,				auth type/Key mgmt: SAE,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP,

	/* Security protocol: WPA2/WPA3 Personal mixed,		auth type/Key mgmt: PSK/SAE, cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP,

	/* Security protocol: WPA3 Enterprise,				auth type/Key mgmt: EAP,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP,

	/* Security protocol: WPA2/WPA3 Enterprise mixed,	auth type/Key mgmt: EAP,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP,

	/* Security protocol: Enhanced Open/OWE,			auth type/Key mgmt: OWE,	cipher: CCMP */
	CLSAPI_WIFI_ENCRYPTION_OWE,

	/* Security protocol: WAI,			auth type/Key mgmt: WAPI,	cipher: SMS4-OFB*/
	CLSAPI_WIFI_ENCRYPTION_WAPI_PSK,

	/* Security protocol: WAI,			auth type/Key mgmt: Certificate-Based cipher: SMS4-OFB*/
	CLSAPI_WIFI_ENCRYPTION_WAPI_CERT,

	/* Keep CLSAPI_WIFI_ENCRYPTION_MAX at the bottom */
	CLSAPI_WIFI_ENCRYPTION_MAX
};

/**
 * These defintions are copied from cls_defines.h in hostapd.
 * Please be careful to change this file and must keep consistent
 * with cls_defines.h
 */
enum clsapi_wifi_wps_error_indication {
	CLSAPI_WIFI_WPS_EI_NO_ERROR,
	CLSAPI_WIFI_WPS_EI_SECURITY_TKIP_ONLY_PROHIBITED,
	CLSAPI_WIFI_WPS_EI_SECURITY_WEP_PROHIBITED,
	CLSAPI_WIFI_WPS_EI_AUTH_FAILURE,
	NUM_CLSAPI_WIFI_WPS_EI_VALUES
};

enum clsapi_wps_status {
	CLSAPI_WIFI_WPS_STATUS_NOT_START,
	CLSAPI_WIFI_WPS_STATUS_SUCCESS,
	CLSAPI_WIFI_WPS_STATUS_FAILURE
};

enum clsapi_wifi_wps_pbc_status {
	CLSAPI_WIFI_WPS_PBC_STATUS_DISABLE,
	CLSAPI_WIFI_WPS_PBC_STATUS_ACTIVE,
	CLSAPI_WIFI_WPS_PBC_STATUS_TIMEOUT,
	CLSAPI_WIFI_WPS_PBC_STATUS_OVERLAP,
	CLSAPI_WIFI_WPS_PBC_STATUS_ERROR
};

/** The status information of the Wi-Fi Protected Setup (WPS) process */
struct clsapi_wifi_wps_status {
	/** The current status of the wps */
	enum clsapi_wps_status status;

	/** Indicates the specific reason for a wps failure */
	enum clsapi_wifi_wps_error_indication failure_reason;

	/** The status of wps pbc */
	enum clsapi_wifi_wps_pbc_status pbc_status;

	/** Stores the MAC address of the other device involved in the current or most recent wps session */
	uint8_t peer_addr[ETH_ALEN];
};

enum clsapi_wifi_stats_type {
	CLSAPI_WIFI_STATS_TYPE_RADIO = 1,
	CLSAPI_WIFI_STATS_TYPE_VAP = 2,
	CLSAPI_WIFI_STATS_TYPE_PEER_STA = 4,
	CLSAPI_WIFI_STATS_TYPE_WPU = 8,

	CLSM_WIFI_STATS_RESET_REPLY = 0x40,
	__CLS_WIFI_STATS_TYPE_MAX,
};

/** structures to show the channel survey, include channel utilization and related period and frequencies*/
struct clsapi_channel_survey {
	/** The current frequency of this phy*/
	uint32_t freq;
	/** The current channel utilization of this phy*/
	uint8_t channel_utilization;
	/** The bss load update period*/
	uint8_t bss_load_update_period;
	/** The channel utilization average update period*/
	uint8_t chan_util_avg_period;
};

/** struct to contain statistics of Wi-Fi radio */
struct clsapi_wifi_radio_stats {
	/** radio index */
	uint32_t radio_index;

	/** The timestamp in seconds since system boot up */
	uint32_t	tstamp;

	/** Associated Station count or if Station is associated */
	uint32_t	assoc_sta_num;

	/** Current active channel */
	uint32_t	channel;

	/** Attenuation */
	uint32_t	atten;

	/** Total CCA */
	uint32_t	cca_total;

	/** Transmit CCA */
	uint32_t	cca_tx;

	/** Receive CCA */
	uint32_t	cca_rx;

	/** CCA interference */
	uint32_t	cca_int;

	/** CCA Idle */
	uint32_t	cca_idle;

	/** Received packets counter */
	uint32_t	rx_pkts;

	/** Receive gain in dBm */
	uint32_t	rx_gain;

	/** Received packet counter with frame check error */
	uint32_t	rx_cnt_crc;

	/** Received noise level in dBm */
	uint32_t	rx_noise;

	/** Transmitted packets counter */
	uint32_t	tx_pkts;

	/** Deferred packet counter in transmission */
	uint32_t	tx_defers;

	/** Time-out counter for transimitted packets */
	uint32_t	tx_touts;

	/** Retried packets counter in transmission */
	uint32_t	tx_retries;

	/** Counter of short preamble errors */
	uint32_t	cnt_sp_fail;

	/** Counter of long preamble errors */
	uint32_t	cnt_lp_fail;

	/** MCS index for last received packet */
	uint32_t	last_rx_mcs;

	/** MCS index for last transimtted packet */
	uint32_t	last_tx_mcs;

	/** Received signal strength indicator in dBm */
	int32_t		last_rssi;

	/** TKIP decrypt error packets */
	uint32_t tkip_decrypt_err;

	/** CCMP128 decrypt error packets */
	uint32_t ccmp128_decrypt_err;

	/** CCMP256 decrypt error packets */
	uint32_t ccmp256_decrypt_err;

	/** GCMP128 decrypt error packets */
	uint32_t gcmp128_decrypt_err;

	/** GCMP256 decrypt error packets */
	uint32_t gcmp256_decrypt_err;

	/** WAPI decrypt error packets */
	uint32_t wapi_decrypt_err;

	/** FCS error packets */
	uint32_t rx_fcs_err_packets;

	/** PHY error packets */
	uint32_t rx_phy_err_packets;
};

/** Virtual access point status */
struct clsapi_wifi_vap_stats {
	/** interface name of VAP */
	//char ifname[33];

	/** tx succeed of num */
	uint32_t tx_trans;

	/** tx failed(include retry failed) of num */
	uint32_t tx_ftrans;

	/** tx succeed but it have retry of num */
	uint32_t tx_retries;

	/** tx succeed but it have once retry of num */
	uint32_t tx_sretries;

	/** tx AMPDU of number, if send SMPDU, this add 0, other add 1 */
	uint32_t tx_aggpkts;

	/** tx no-ACK PPDU of number */
	uint32_t tx_toutpkts;

	/** The number of received unicast packets from the node */
	uint64_t rx_unicast;

	/** The number of received multicast packets from the node */
	uint64_t rx_multicast;

	/** The number of received broadcast packets form the node */
	uint64_t rx_broadcast;

	/** The number of transmitted unicast packets to the node */
	uint64_t tx_unicast;

	/** The number of transmitted multicast packets to the node */
	uint64_t tx_multicast;

	/** The number of transmitted broadcast packets to the node */
	uint64_t tx_broadcast;

	/** tx beacon frame count */
	uint64_t tx_bcnpkts;

	/** tx probe response frame count */
	uint64_t tx_probersppkts;

	/** tx auth frame count */
	uint64_t tx_authpkts;

	/** tx deauth frame count */
	uint64_t tx_deauthpkts;

	/** tx association request fram count */
	uint64_t tx_assocreqpkts;

	/** tx association response frame count */
	uint64_t tx_assocrsppkts;

	/** tx reassociation request fram count */
	uint64_t tx_reascreqpkts;

	/** tx reassociation response frame count */
	uint64_t tx_reascrsppkts;

	/** tx disassociation frame count */
	uint64_t tx_disassocpkts;

	/** rx probe response frame count */
	uint64_t rx_probersppkts;

	/** rx auth frame count */
	uint64_t rx_authpkts;

	/** rx deauth frame count */
	uint64_t rx_deauthpkts;

	/** rx association request fram count */
	uint64_t rx_assocreqpkts;

	/** rx association response frame count */
	uint64_t rx_assocrsppkts;

	/** rx reassociation request fram count */
	uint64_t rx_reascreqpkts;

	/** rx reassociation response frame count */
	uint64_t rx_reascrsppkts;

	/** rx disassociation frame count */
	uint64_t rx_disassocpkts;
};

/** Station status */
struct clsapi_wifi_sta_stats {
	/** The number of transmitted bytes to the node */
	uint64_t	tx_bytes;

	/** The number of transmitted packets to the node */
	uint32_t	tx_pkts;

	/** The number of transmit discards to the node */
	uint32_t	tx_discard;

	/** The number of data packets transmitted through
	 * wireless media for each Access Classes(AC)
	 */
	uint32_t	tx_wifi_sent[4];

	/** The number of dropped data packets failed to transmit through
	 * wireless media for each Access Classes(AC)
	 */
	uint32_t	tx_wifi_drop[4];

	/** The number of transmit errors to the node */
	uint32_t	tx_err;

	/** The number of transmitted unicast packets to the node */
	uint64_t	tx_unicast;

	/** The number of transmitted multicast packets to the node */
	uint64_t	tx_multicast;

	/** The number of transmitted broadcast packets to the node */
	uint64_t	tx_broadcast;

	/** TX PHY rate in megabits per second (Mbps) */
	uint32_t	tx_phy_rate;

	/** The number of transmitted management frames to the node */
	uint32_t	tx_mgmt;

	/** The number of received bytes from the node */
	uint64_t	rx_bytes;

	/** The number of received packets from the node */
	uint32_t	rx_pkts;

	/** The numbder of received packets discarded from the node */
	uint32_t	rx_discard;

	/** The number of received packets in error from the node */
	uint32_t	rx_err;

	/** The number of received unicast packets from the node */
	uint64_t	rx_unicast;

	/** The number of received multicast packets from the node */
	uint64_t	rx_multicast;

	/** The number of received broadcast packets form the node */
	uint64_t	rx_broadcast;

	/** The number of received unknown packets from the node */
	uint32_t	rx_unknown;

	/** RX PHY rate in megabits per second (MBPS) */
	uint32_t	rx_phy_rate;

	/** The number of received management frames from the node */
	uint32_t	rx_mgmt;

	/** The number of received control from the node */
	uint32_t	rx_ctrl;

	/** The MAC address of the node */
	uint8_t mac_addr[6];

	/** The rssi of the node */
	int8_t		rssi;

	/** The bandwidth of the node */
	int8_t		bw;

	/** Last Tx MCS index */
	uint8_t		tx_mcs;

	/** Last Rx MCS index */
	uint8_t		rx_mcs;

	/** Number of spatial streams used in last transmission */
	uint8_t		tx_nss;

	/** Number of spatial streams received in last reception */
	uint8_t		rx_nss;

	/** Bandwidth used in last transmission */
	uint8_t		tx_bw;

	/** Bandwidth used in last reception */
	uint8_t		rx_bw;

	/** Was short guard interval used in last transmission? */
	uint8_t		tx_sgi;

	/** Was short guard interval used in last reception? */
	uint8_t		rx_sgi;
};

/** WPU status */
struct clsapi_wifi_wpu_stats {
	/** radio name */
	//char radio_name[33];

	/** cpu idle %: */
	uint32_t cpu_idle_percent;
};

/** CCA energy detect config request */
struct clsapi_cca_ed_config_req {
	/** CCA 20MHz primary rise threshold in unit of dBm */
	int8_t cca20p_risethr;

	/** CCA 20MHz primary fall threshold in unit of dBm */
	int8_t cca20p_fallthr;

	/** CCA 20MHz second rise threshold in unit of dBm */
	int8_t cca20s_risethr;

	/** CCA 20MHz second fall threshold in unit of dBm */
	int8_t cca20s_fallthr;
};

/** Structures to include rootap information */
struct clsapi_wifi_rootap_info {
	/** The ssid of root AP will be connected */
	string_128 ssid;

	/** Root AP encryption method */
	enum clsapi_wifi_encryption encryption;

	/** Root AP password, if encryption is NULL, the password is NULL */
	string_64 password;
};

/**
 * The end of defintions are copied from cls_defines.h in hostapd.
 */

/*!\addtogroup Wifi
 *  @{
*/

/*************************	C-Call functions declarations	**********************/

/******************************************************************************************************************/
/************************************************	Radio/Phy APIs	***********************************************/
/******************************************************************************************************************/

/**
 * \brief Get enabled/disabled status of Wi-Fi radio.
 * \details Get enabled/disabled status of Wi-Fi radio.
 * \param phyname [In] The physical name of the radio.
 * \param enable [Out] Enabled/disabled status of Wi-Fi radio.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_radio_enabled(const char *phyname, bool *enable);

/**
 * \brief Enable/disable Wi-Fi radio.
 * \details Enable/disable Wi-Fi radio.
 * \param phyname [In] The physical name of the radio.
 * \param enable [In] Enabled/disabled status of Wi-Fi radio.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_radio(const char *phyname, const bool enable);

/**
 * \brief Get supported operating standards of Wi-Fi hardware device.
 * \details Get supported operating standards (hwmodes) of Wi-Fi hardware device , e.g. IEEE 802.11a/b/g/n/ac/ax.
 * If multiple standards are supported, return bitwise OR value.
 * \param phyname [In] The physical name of the radio.
 * \param hwmodes [Out] Wi-Fi HW supported operating standard(s). If multiple standards are supported, return bitwise OR value.
 \n	HWMODE_IEEE80211_A  = 0
 \n	HWMODE_IEEE80211_B  = 2
 \n	HWMODE_IEEE80211_G  = 4
 \n	HWMODE_IEEE80211_N  = 8
 \n	HWMODE_IEEE80211_AC = 16
 \n	HWMODE_IEEE80211_AX = 32
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_supported_hwmodes(const char *phyname, enum clsapi_wifi_hwmode *hwmodes);

/**
 * \brief Get Wi-Fi current operating standard.
 * \details Get Wi-Fi current operating standard (hwmode), e.g. IEEE 802.11a/b/g/n/ac/ax.
 * \param phyname [In] The physical name of the radio.
 * \param hwmode [Out] Wi-Fi current operating standard.
 \n	HWMODE_IEEE80211_A  = 0
 \n	HWMODE_IEEE80211_B  = 2
 \n	HWMODE_IEEE80211_G  = 4
 \n	HWMODE_IEEE80211_N  = 8
 \n	HWMODE_IEEE80211_AC = 16
 \n	HWMODE_IEEE80211_AX = 32
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_hwmode(const char *phyname, enum clsapi_wifi_hwmode *hwmode);

/**
 * \brief Set Wi-Fi current operating standard.
 * \details Set Wi-Fi current operating standard (hwmode), e.g. IEEE 802.11a/b/g/n/ac/ax. Operating
 * standard set by this API is forward compatible.
 * \param phyname [In] The physical name of the radio.
 * \param hwmode [In] New Wi-Fi operating standard.
 \n	HWMODE_IEEE80211_A  = 0
 \n	HWMODE_IEEE80211_B  = 2
 \n	HWMODE_IEEE80211_G  = 4
 \n	HWMODE_IEEE80211_N  = 8
 \n	HWMODE_IEEE80211_AC = 16
 \n	HWMODE_IEEE80211_AX = 32
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_hwmode(const char *phyname, const enum clsapi_wifi_hwmode hwmode);

/**
 * \brief List supported bands of radio.
 * \details List all supported bands of the Wi-Fi radio.
 * \param phyname [In] The physical name of the radio.
 * \param band_array [Out] The supported bands. Caller MUST free() after using.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_supported_bands(const char *phyname, enum clsapi_wifi_band **band_array);

/**
 * \brief Get current operating band of Wi-Fi radio.
 * \details Get current operating band of Wi-Fi radio.
 * \param phyname [In] The physical name of the radio.
 * \param band [Out] Current Wi-Fi operating band. Band is one of enum clsapi_wifi_band.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * \return N on success or others on error.
 */
int clsapi_wifi_get_band(const char *phyname, enum clsapi_wifi_band *band);

/**
 * \brief Get supported bandwidth list.
 * \details Get supported bandwidth list.
 * \param phyname [In] The physical name of the radio.
 * \param bw_array [Out] The supported bws. Caller MUST free() after using.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_supported_bws(const char *phyname, enum clsapi_wifi_bw **bw_array);

/**
 * \brief Get current radio bandwidth.
 * \details Get current radio bandwidth.
 * \param phyname [In] The physical name of the radio.
 * \param bw [Out] The current radio bandwidth.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_bw(const char *phyname, enum clsapi_wifi_bw *bw);

/**
 * \brief Set Wi-Fi radio bandwidth.
 * \details Set new bandwidth in current band and operating standard, e.g. VHT80 to VHT160.
 * \param phyname [In] The physical name of the radio.
 * \param bw [In] The new radio bandwidth.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_bw(const char *phyname, const enum clsapi_wifi_bw bw);

/**
 * \brief Get supported channel list in given band and bandwidth.
 * \details Get supported channel list in given band and bandwidth. The supported channels are restrained by band, bw and regulatory.
 * \param phyname [In] The physical name of the radio.
 * \param band [In] Band which channel list belongs to. CLSAPI_BAND_DEFAULT means current band.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * \param bw [In] Bandwidth to filter channels.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \param channel_array [Out] The supported channels. Caller MUST free() after using.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_supported_channels(const char *phyname, const enum clsapi_wifi_band band,
		const enum clsapi_wifi_bw bw, uint8_t **channel_array);

/**
 * \brief Get runtime channel number.
 * \details Get runtime operation channel of the Wi-Fi radio. The channel number is in current band. If radio is configured as fixed channel, the operation channel is the configured one. If auto channel selection is enabled, the operation channel is automaticlly selected.
 * \param phyname [In] The physical name of the radio.
 * \param channel [Out] Channel number in current band.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_channel(const char *phyname, uint8_t *channel);

/**
 * \brief Set operating channel number.
 * \details Set operating channel number of the Wi-Fi radio. channel=0 means enabling ACS (Auto
 * Channel Selection) mechanism and the operating channel is selected and set by the algorithm at runtime.
 * \param phyname [In] The physical name of the radio.
 * \param channel [In] New channel number. 0: enable ACS; other valid channel: fixed channel
 * \param band [In] New band. CLSAPI_BAND_DEFAULT means current band.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * \param bw [In] New bw. CLSAPI_WIFI_BW_DEFAULT means current bw.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_channel(const char *phyname, const uint8_t channel, const enum clsapi_wifi_band band,
		const enum clsapi_wifi_bw bw);

/**
 * \brief Get beacon interval, in unit of 1.024ms.
 * \details Get beacon interval. Beacon interval is time interval between two consecutive beacon frames.
 * Beacon interval is in unit of ms. Normally it's integral multiple of 100ms (100x1.024ms).
 * \param phyname [In] The physical name of the radio.
 * \param beacon_int [Out] Value of beacon interval, in unit of 1.024ms, default: 100; range from 15 to 65535.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_beacon_intl(const char *phyname, uint16_t *beacon_int);

/**
 * \brief Set beacon interval, in unit of 1.024ms.
 * \details Set beacon interval. Beacon interval is time interval between two consecutive beacon frames.
 * Beacon interval is in unit of ms. Normally it's integral multiple of 100ms (100x1.024ms).
 * \param phyname [In] The physical name of the radio.
 * \param beacon_int [In] Value of beacon interval, in unit of 1.024ms, default: 100; range from 15 to 65535.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_beacon_intl(const char *phyname, const uint16_t beacon_int);

/**
 * \brief Get RTS threshold, in unit of bytes.
 * \details Get RTS (Request To Send) threshold, in unit of bytes. The RTS threshold is a packet size
 * threshold at which the RTS/CTS mechanism is used for channel. When a device wants
 * to send data, it checks the packet size against the RTS threshold value. If the packet size
 * exceeds the threshold, the device will use the RTS/CTS mechanism to reserve the channel
 * before transmitting the data.
 * \param phyname [In] The physical name of the radio.
 * \param rts [Out] Value of RTS threshold, in unit of bytes. Range from -1 to 65535, -1 means disable RTS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_rts(const char *phyname, int *rts);

/**
 * \brief Set RTS threshold, in unit of bytes.
 * \details Set RTS (Request To Send) threshold, in unit of bytes. The RTS threshold is a packet size
 * threshold at which the RTS/CTS mechanism is used for channel access. When a device wants
 * to send data, it checks the packet size against the RTS threshold value. If the packet size
 * exceeds the threshold, the device will use the RTS/CTS mechanism to reserve the channel
 * before transmitting the data.
 * \param phyname [In] The physical name of the radio.
 * \param rts [In] Value of RTS threshold, in unit of bytes. Range from -1 to 65535, -1 means disable RTS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_rts(const char *phyname, const int rts);

/**
 * \brief Get current fixed transmit power.
 * \details Get current fixed transmit power.
 * Tx power is restrained by regulatory, band, channel, hardware capability, etc.
 * \param phyname [In] The physical name of the radio.
 * \param txpower [Out] Value of Tx power, in unit of dBm.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_txpower(const char *phyname, int8_t *txpower);

/**
 * \brief Get configured maxmium transmit power.
 * \details Get configured maxmium transmit power.
 * Tx power is restrained by regulatory, band, channel, hardware capability, etc.
 * \param phyname [In] The physical name of the radio.
 * \param txpower [Out] Value of Tx power, in unit of dBm.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_txpower_limit(const char *phyname, int8_t *txpower);

/**
 * \brief Get enabled/disabled status of short GI.
 * \details Get enabled/disabled status of short GI for HT and VHT. For HE, GI is bundled with data rate. And it's managed by rate adapation algorithm at runtime.
 * \param phyname [In] The physical name of the radio.
 * \param bw [In] The radio bandwidth.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \param enable [Out] Short GI enabled/disabled status.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_short_gi(const char *phyname, const enum clsapi_wifi_bw bw, bool *enable);

/**
 * \brief Set enabled/disabled status of short GI.
 * \details Set enabled/disabled status of short GI for HT and VHT. For HE, GI is bundled with data rate.
 * And it's managed by rate adapation algorithm at runtime.
 * \param phyname [In] The physical name of the radio.
 * \param bw [In] The radio bandwidth.
 \n	CLSAPI_WIFI_BW_20 = 1
 \n	CLSAPI_WIFI_BW_40 = 2
 \n	CLSAPI_WIFI_BW_80 = 3
 \n	CLSAPI_WIFI_BW_160 = 5
 * \param enable [In] Short GI enabled/disabled status.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_short_gi(const char *phyname, const enum clsapi_wifi_bw bw, const bool enable);

/**
 * \brief Get country code of the Wi-Fi radio.
 * \details Get country code of the Wi-Fi radio. Country code indicates country in which the radio is operating. This can limit available channels and transmit power. Country code is two octets in ISO/IEC 3166-1, e.g. CN for China, US for America.
 * \param phyname [In] The physical name of the radio.
 * \param country_code [Out] Buffer to carry returned country code, two octets in ISO/IEC 3166-1.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_country_code(const char *phyname, string_8 country_code);

/**
 * \brief Set country code of the Wi-Fi device.
 * \details Set country code of the Wi-Fi device. Country code indicates country in which the device is operating. This will limit available channels and transmit power. Country code is two octets in ISO/IEC 3166-1, e.g. CN for China, US for America.
 * \param phyname [In] The physical name of the radio.
 * \param country_code [In] Country code, two octets in ISO/IEC 3166-1.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_country_code(const char *phyname, const char *country_code);

/**
 * \brief Get Wi-Fi survey info.
 * \details Get Wi-Fi survey info per channel, includes noise and various kinds of time.
 * Off-channel scan is necessary before getting scan survey. If off-channel scan on some specific channels, this API will still show all channels surveys.
 * Note that only those specific channels surveys are useful.
 * \param phyname [In] The physical name of the radio.
 * \param survey_array [Out] CLS-API malloc()ed array to carry the survey entries. Caller MUST free() after using.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_survey(const char *phyname, struct survey_entry **survey_array);

/**
 * \brief Get Wi-Fi noise.
 * \details Get Wi-Fi noise of current channel.
 * \param phyname [In] The physical name of the radio.
 * \param noise [Out] The noise of current channel.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_noise(const char *phyname, int8_t *noise);

/**
 * \brief Get channel scores of the band.
 * \details Get channel scores of the band, the max score is 65535 and the less the better.
 * Results based on the below. Off-channel scan is necessary before getting scan scores.
 * If off-channel scan on some specific channels, this API will still show all the channels scores, but only those specific channels scores are useful.
 * score = 10^(chan_nf/5)+(busy_time-tx_time)/(active_time-tx_time)×2^(10^(chan_nf/10)+10^(band_min_nf/10))
 * \param phyname [In] The physical name of the radio.
 * \param band [In] Wi-Fi band to get score on. Currently, band is fixed per radio.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * band (CLSAPI_BAND_DEFAULT = 0xFF).
 * \param score_array [Out] The channel score entries. Caller MUST free() after using.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_chan_score(const char *phyname, const enum clsapi_wifi_band band, struct chan_score **score_array);

/**
 * \brief Get max supported VAP number of the Wi-Fi radio.
 * \details Get max supported VAP number of the Wi-Fi radio.
 * \param phyname [In] The physical name of the radio.
 * \param max_vap [Out] Max supported VAP number.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_supported_max_vap(const char *phyname, uint16_t *max_vap);

/**
 * \brief Get maximum allowed association STAs per AP.
 * \details Get maximum allowed association STAs per AP.
 * \param phyname [In] The physical name of the radio.
 * \param max_sta [Out] Max supported STAs.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_supported_max_sta(const char *phyname, uint16_t *max_sta);

/**
 * \brief Add a new Wi-Fi BSS and let driver return the created interface name.
 * \details Add a new AP-mode virtual interfacew with a set of default parameters. Besides, let driver generate and return the created interface name. Default parameters are:
 * |Class	|Content|
 * |--------|-------|
 * |SSID	|"Clourney" for 2GHz; "Clourney-5G" for 5GHz|
 * |Encryption|	WPA2-PSK-CCMP|
 * |Key|12345678|
 * |Network|Add to Br-lan|
 * \param phyname [In] The radio name on which new BSS will be created, e.g. "phy0"
 * \param bssid [In] BSSID of the new BSS. If bssid is NULL, the driver default value will be filled.
 * \param created_ifname [Out] Buffer to carry new created ifname.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_add_bss(const char *phyname, const uint8_t bssid[ETH_ALEN], clsapi_ifname created_ifname);

/**
 * \brief Delete a Wi-Fi BSS.
 * \details Delete a Wi-Fi BSS.
 * \param ifname [In] The Wi-Fi interface name.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_del_bss(const char *ifname);

/**
 * \brief Get enabled/disabled status of the BSS.
 * \details Get enabled/disabled status of the BSS.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [Out] Enabled/disabled status of the BSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_bss_enabled(const char *ifname, bool *enable);

/**
 * \brief Enable/disable the BSS.
 * \details Enable/disable the BSS.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [In] Enabled/disabled status of the BSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_bss(const char *ifname, const bool enable);

/**
 * \brief Get BSSID of a VAP.
 * \details Get BSSID of a VAP.
 * \param ifname [In] The Wi-Fi interface name.
 * \param bssid [Out] Buffer to carry the returned BSSID.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_bssid(const char *ifname, uint8_t bssid[ETH_ALEN]);

/**
 * \brief Get SSID of a Wi-Fi interface.
 * \details Get SSID of the Wi-Fi interface.
 * \param ifname [In] The Wi-Fi interface name.
 * \param ssid [Out] SSID of the vap.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_ssid(const char *ifname, string_32 ssid);

/**
 * \brief Set SSID of a Wi-Fi interface.
 * \details Set SSID of VAP.
 * \param ifname [In] The Wi-Fi interface name.
 * \param ssid [In] SSID of the vap.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_ssid(const char *ifname, const char *ssid);

/**
 * \brief Get Wi-Fi authentication and encryption configuration.
 * \details Get Wi-Fi authentication and encryption configuration. It's is combination of:
 *	|Security protocol|Auth type|Cipher|
 *	|-------------------|---------------|-----------|
 *	| None				| None			|None		|
 *	| WEP				| None			|WEP40/104	|
 *	| WPA Personal		| PSK			| CCMP		|
 *	| WPA Personal		| PSK			| TKIP		|
 *	| WPA Personal		| PSK			| TKIP/CCMP mixed|
 *	| WPA Enterprise	| EAP			| CCMP		|
 *	| WPA Enterprise	| EAP			| TKIP		|
 *	| WPA Enterprise	| EAP			| TKIP/CCMP mixed|
 *	| WPA2 Personal		| PSK			| CCMP		|
 *	| WPA2 Personal		| PSK			| TKIP 		|
 *	| WPA2 Personal		| PSK			| TKIP/CCMP mixed|
 *	| WPA2 Enterprise	| EAP			| CCMP		|
 *	| WPA2 Enterprise	| EAP			| TKIP 		|
 *	| WPA2 Enterprise	| EAP			| TKIP/CCMP mixed|
 *	| WPA/WPA2 Personal mixed	| PSK			| CCMP		|
 *	| WPA/WPA2 Personal mixed	| PSK			| TKIP 		|
 *	| WPA/WPA2 Personal mixed	| PSK			| TKIP/CCMP mixed|
 *	| WPA/WPA2 Enterprise mixed	| EAP			| CCMP		|
 *	| WPA/WPA2 Enterprise mixed	| EAP			| TKIP 		|
 *	| WPA/WPA2 Enterprise mixed	| EAP			| TKIP/CCMP mixed|
 *	| WPA3 Personal		| SAE			| CCMP		|
 *	| WPA2/WPA3 Personal mixed	| PSK/SAE		| CCMP		|
 *	| WPA3 Enterprise	| EAP			| CCMP		|
 *	| WPA2/WPA3 Enterprise mixed| EAP			| CCMP		|
 *	| Enhanced Open/OW	| OWE			| CCMP		|
 *	| WAI				| WAPI			| SMS4-OFB	|
 \n	CLSAPI_WIFI_ENCRYPTION_OPEN_NONE = 0
 \n	CLSAPI_WIFI_ENCRYPTION_WEP_MIXED
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_OWE
 \n CLSAPI_WIFI_ENCRYPTION_WAPI_PSK
 * \param ifname [In] The Wi-Fi interface name.
 * \param encryption [Out] Encryption mode and cipher type.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_encryption(const char *ifname, enum clsapi_wifi_encryption *encryption);

/**
 * \brief Set Wi-Fi authentication and encryption configuration.
 * \details Set Wi-Fi authentication and encryption configuration. It's is combination of:
 *	|Security protocol|Auth type|Cipher|
 *	|-------------------|---------------|-----------|
 *	| None				| None			|None		|
 *	| WEP				| None 			|WEP40/104	|
 *	| WPA Personal		| PSK			| CCMP		|
 *	| WPA Personal		| PSK			| TKIP		|
 *	| WPA Personal		| PSK			| TKIP/CCMP mixed|
 *	| WPA Enterprise	| EAP			| CCMP		|
 *	| WPA Enterprise	| EAP			| TKIP		|
 *	| WPA Enterprise	| EAP			| TKIP/CCMP mixed|
 *	| WPA2 Personal		| PSK			| CCMP		|
 *	| WPA2 Personal		| PSK			| TKIP 		|
 *	| WPA2 Personal		| PSK			| TKIP/CCMP mixed|
 *	| WPA2 Enterprise	| EAP			| CCMP		|
 *	| WPA2 Enterprise	| EAP			| TKIP 		|
 *	| WPA2 Enterprise	| EAP			| TKIP/CCMP mixed|
 *	| WPA/WPA2 Personal mixed	| PSK			| CCMP		|
 *	| WPA/WPA2 Personal mixed	| PSK			| TKIP 		|
 *	| WPA/WPA2 Personal mixed	| PSK			| TKIP/CCMP mixed|
 *	| WPA/WPA2 Enterprise mixed	| EAP			| CCMP		|
 *	| WPA/WPA2 Enterprise mixed	| EAP			| TKIP 		|
 *	| WPA/WPA2 Enterprise mixed	| EAP			| TKIP/CCMP mixed|
 *	| WPA3 Personal		| SAE			| CCMP		|
 *	| WPA2/WPA3 Personal mixed	| PSK/SAE		| CCMP		|
 *	| WPA3 Enterprise	| EAP			| CCMP		|
 *	| WPA2/WPA3 Enterprise mixed| EAP			| CCMP		|
 *	| Enhanced Open/OW	| OWE			| CCMP		|
 *	| WAI				| WAPI			| SMS4-OFB	|
 \n	CLSAPI_WIFI_ENCRYPTION_OPEN_NONE = 0
 \n	CLSAPI_WIFI_ENCRYPTION_WEP_MIXED
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_PSK_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_EAP_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_EAP_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_PSK_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA_WPA2_EAP_TKIP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA3_SAE_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_PSK_WPA3_SAE_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA3_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_WPA2_WPA3_EAP_CCMP
 \n	CLSAPI_WIFI_ENCRYPTION_OWE
 \n CLSAPI_WIFI_ENCRYPTION_WAPI_PSK
 * \param ifname [In] The Wi-Fi interface name.
 * \param encryption [In] Encryption mode and cipher type.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_encryption(const char *ifname, const enum clsapi_wifi_encryption encryption);

/**
 * \brief Get the passphrase of a Wi-Fi interface.
 * \details Get the passphrase of a Wi-Fi interface.
 * \param ifname [In] The Wi-Fi interface name.
 * \param passphrase [Out] ASCII passphrase (8..63 characters).
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_passphrase(const char *ifname, string_64 passphrase);

/**
 * \brief Set the passphrase for a Wi-Fi interface.
 * \details Set the passphrase for a Wi-Fi interface.
 * \param ifname [In] The Wi-Fi interface name.
 * \param passphrase [In] ASCII passphrase (8..63 characters).
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_passphrase(const char *ifname, const char *passphrase);

/**
 * \brief Get the WEP key list and index of a Wi-Fi interface.
 * \details Get the WEP key list and index of a Wi-Fi interface. Keys in use couldn't be empty (""). The WEP key must be 5-13 ASCII characters or 10-26 hexadecimal digits.
 * \param ifname [In] The Wi-Fi interface name.
 * \param idx_in_use [Out] Key index in use, it's index of preshared_key, starting from 0.
 * \param wep_key [Out] Preshared key table, set element to empty ("") if not configured.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_wep_key(const char *ifname, int *idx_in_use, char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64]);

/**
 * \brief Set WEP key for a Wi-Fi interface.
 * \details Set WEP key list and index to use for a Wi-Fi interface. Keys which will be used couldn't be empty (""). The WEP key must be 5-13 ASCII characters, or 10-26 hexadecimal digits.
 * \param ifname [In] The Wi-Fi interface name.
 * \param idx_to_use [In] Key index to use, it's index of preshared_key, starting from 0.
 * \param wep_key [In] Preshared key table, set element to empty ("") if not configured.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_wep_key(const char *ifname, const int idx_to_use, const char wep_key[CLSAPI_WIFI_MAX_PRESHARED_KEY][64]);

/**
 * \brief Get the associated STA list.
 * \details Get the STA MAC addresss list which is associated with the given AP.
 * \param ifname [In] Wi-Fi interface name, e.g. wlan0, wlan1-1.
 * \param sta_array [In] The associated STA MAC list. Caller MUST free() after using.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_assoc_list(const char *ifname, uint8_t (**sta_array)[ETH_ALEN]);

/**
 * \brief Get the maximum number of stations that can be allocated.
 * \details Get the maximum number of stations that can be allocated.
 * \param ifname [In] The Wi-Fi interface name.
 * \param max_sta [Out] Value of max allowed STAs.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_max_allow_sta(const char *ifname, uint16_t *max_sta);

/**
 * \brief Set the maximum number of stations that can be allocated.
 * \details Set the maximum number of stations that can be allocated.
 * \param ifname [In] The Wi-Fi interface name.
 * \param max_sta [In] Value of max allowed STAs.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_max_allow_sta(const char *ifname, const uint16_t max_sta);

/**
 * \brief Get DTIM period.
 * \details Get DTIM (Delivery Traffic Indication Message Interval) period, in unit of beacon count.
 * \param ifname [In] The Wi-Fi interface name.
 * \param dtim [Out] Value of DTIM period, in unit of beacon count, range from 1 to 255.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_dtim(const char *ifname, uint8_t *dtim);

/**
 * \brief Set DTIM period.
 * \details Set DTIM (Delivery Traffic Indication Message Interval) period, in unit of beacon count.
 * \param ifname [In] The Wi-Fi interface name.
 * \param dtim [In] Value of DTIM period, in unit of beacon count, range from 1 to 255.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_dtim(const char *ifname, const uint8_t dtim);

/**
 * \brief Get enabled/disabled status of hidden SSID.
 * \details Get enabled/disabled status of hidden SSID.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [Out] Enable/disable status of hidden SSID.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_hidden_ssid(const char *ifname, enum clsapi_wifi_hidden_ssid *enable);

/**
 * \brief Enable/disable hidden SSID.
 * \details Send empty SSID in beacons and ignore probe request frames which does not include full SSID.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [In] New value to enabled/disabled hidden SSID.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_hidden_ssid(const char *ifname, const enum clsapi_wifi_hidden_ssid enable);

/**
 * \brief Get WMM enabled/disabled status of the VAP.
 * \details Get WMM (Wi-Fi Multimedia) enabled/disabled status of the VAP. WMM is Wi-Fi Alliance defined QoS feature to IEEE 802.11 networks, based on IEEE 802.11e standard. WMM prioritizes traffic according to four Access Categories (AC):
 \n	- Voice
 \n	- Video
 \n	- Best effort
 \n	- Background
 \n	Where WMM is disabled, STAs might be limited to legacy rates. WMM is requried for IEEE802.11n/ac/ax.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [Out] WMM enabled/disabled status.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_wmm_enabled(const char *ifname, bool *enable);

/**
 * \brief Enable/Disable WMM status of the VAP.
 * \details Enable/Disable WMM status of the VAP. WMM is Wi-Fi Alliance defined QoS feature to
 * IEEE 802.11 networks, based on IEEE 802.11e standard. WMM prioritizes traffic according to
 * four Access Categories (AC):
 \n	- Voice
 \n	- Video
 \n	- Best effort
 \n	- Background
 \n	Where WMM is disabled, STAs might be limited to legacy rates. WMM is requried for IEEE802.11n/ac/ax.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [In] WMM enabled/disabled status.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_wmm(const char *ifname, const bool enable);

/**
 * \brief Get PMF enabled/disabled configuration.
 * \details Get PMF (Wi-Fi Protected Management Frames) enabled/disabled configuration. PMF is a security feature that aims to protect the management frames exchanged between Wi-Fi devices from various types of attacks. There are 3 configurations for PMF:
 \n	- Disabled
 \n	- Optional
 \n	- Required
 \n	PMF is an optional Wi-Fi feature. The final protection state of management frames are determined by enable state of AP and STA, according following table:
 * |AP PMF | STA PMF | Protection state of management frames|
 * | ------------- | ------------- | ---------------|
 * |Reqired | Disabled | AP rejects the STA's association request|
 * |Optional | Disabled | AP accepts the STA's association request, but management frames are not protected|
 * |Disabled | Optional or Disabled |  AP accepts the STA's association request, but management frames are not protected|
 * |Disabled | Required | STA rejects associating the AP|
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable_cfg [Out] PMF enable state.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_pmf_enabled(const char *ifname, enum clsapi_wifi_pmf_enable *enable_cfg);

/**
 * \brief Set PMF enabled/disabled configuration.
 * \details Set PMF (Wi-Fi Protected Management Frames) enabled/disabled configuration. PMF is a security
 * feature that aims to protect the management frames exchanged between Wi-Fi devices from various
 * types of attacks. There are 3 configurations for PMF:
 \n	- Disabled
 \n	- Optional
 \n	- Required
 \n	PMF is an optional Wi-Fi feature. The final protection state of management frames are determined by enable state of AP and STA, according following table:
 * |AP PMF | STA PMF | Protection state of management frames|
 * | ------------- | ------------- | ---------------|
 * |Reqired | Disabled | AP rejects the STA's association request|
 * |Optional | Disabled | AP accepts the STA's association request, but management frames are not protected|
 * |Disabled | Optional or Disabled |  AP accepts the STA's association request, but management frames are not protected|
 * |Disabled | Required | STA rejects associating the AP|
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable_cfg [In] PMF enable state.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_pmf(const char *ifname, const enum clsapi_wifi_pmf_enable enable_cfg);

/**
 * \brief Get max inactivity time limit of STA.
 * \details Get max inactivity limit in seconds of STA. If a STA doesn't send anything in this
 * time limit, the STA will be disassociated and deauthenticated by its associated AP. This feature
 * is used to clear station table of old entries when STAs are out of the AP's coverage.
 * \param ifname [In] The Wi-Fi interface name.
 * \param max_inactivity [Out] Max inactivity time limit in seconds for STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_sta_max_inactivity(const char *ifname, uint32_t *max_inactivity);

/**
 * \brief Set max inactivity time limit for STA.
 * \details Set max inactivity limit in seconds for STA. If a STA doesn't send anything in this
 * time limit, the STA will be disassociated and deauthenticated by its associated AP. This feature
 * is used to clear station table of old entries when STAs are out of the AP's coverage.
 * \param ifname [In] The Wi-Fi interface name.
 * \param max_inactivity [In] New value of max inactivity time limit in seconds for STA, default: 300.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_sta_max_inactivity(const char *ifname, const uint32_t max_inactivity);

/************************************************	Scan APIs	*******************************************************/
/* This section describes APIs that scan and report surrounding AP properties. Wi-Fi scanning is
 * operation that Wi-Fi radio switches to target channels one by one, and dwells to listening other
 * APs' Beacon frame for recording AP properties.
 * The duration time of scanning depends on target channel numbers and dwell time on each channel.
 * To avoid caller of CLS-API waiting, scan APIs are designed to tigger-cache-get flavor:
 *   - clsapi_wifi_start_scan() triggers the driver to start scanning.
 *   - CLS-API will cache the scan result in memory.
 *   - clsapi_wifi_get_scan_count() check whether scan is finished, if so get the count of scanned APs.
 *   - clsapi_wifi_get_scan_entry() gets scanned AP informations.
 */

/**
 * \brief Start advanced scan surrounding APs.
 * \details Trigger radio to start advanced scanning for surrounding APs with given band and channels.
 * Custom parameters could be set, refer to structure clsapi_wifi_scan_params.
 * Need to wait until the scan is done, time depends on how many channels will be scanned.
 * \param phyname [In] The physical name of the radio.
 * \param params [In] The params will be inputed when scanning.
 * \param band [In] Band to be scanned. CLSAPI_BAND_DEFAULT means current operating band.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * \param channels [In] Array of target channels to be scanned. channels = NULL or channels_len=0 means scan all available channels in current operating band, note that is an array.
 * \param channels_len [In] Length of channels array.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_start_adv_scan(const char *phyname,
		struct clsapi_wifi_scan_params *params,
		const enum clsapi_wifi_band band,
		const uint8_t channels[],
		const int channels_len);

/**
 * \brief Start to scan surrounding APs.
 * \details Trigger radio to start scanning for surrounding APs with given band and channels.
 * Different flag could be set, such as flush_mode means cfg80211 won't keep any old records.
 * Need to wait until the scan is done, time depends on how many channels will be scanned.
 * \param phyname [In] The physical name of the radio.
 * \param scan_flags [In] The scan flags for this scan, please refer to clsapi_wifi_scan_flags.
 * \param band [In] Band to be scanned. CLSAPI_BAND_DEFAULT means current operating band.
 \n	CLSAPI_BAND_2GHZ = 0
 \n	CLSAPI_BAND_5GHZ = 1
 \n	CLSAPI_BAND_RSV = 2
 \n	CLSAPI_BAND_6GHZ = 3
 \n CLSAPI_BAND_MAX = 4
 \n CLSAPI_BAND_NOSUCH_BAND = CLSAPI_BAND_MAX
 \n CLSAPI_BAND_DEFAULT = 0xFF
 * \param channels [In] Array of target channels to be scanned. channels=NULL or channels_len=0 means scan all available channels in current operating band, note it is an array.
 * \param channels_len [In] Length of channels array.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_start_scan(const char *phyname, enum clsapi_wifi_scan_flags scan_flags, enum clsapi_wifi_band band,
		const uint8_t channels[], const int channels_len);

/**
 * \brief Get the status and scanned AP number of an AP scanning.
 * \details Check whether a scanning is finished, get the number of scanned AP if so.
 * \param phyname [In] The physical name of the radio.
 * \param ap_cnt [Out] Return parameter of the count of scanned AP.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_scan_count(const char *phyname, uint32_t *ap_cnt);

/**
 * \brief Get all AP entry from scanned AP list.
 * \details Get all AP information from scanned AP list.
 * \param phyname [In] The physical name of the radio.
 * \param ap_idx [In] The index of scanned AP list. -1 means get all AP info.
 * \param scan_ap_array [Out] The scanned AP(s) information. Caller MUST free() after using.
 \n	If ap_idx >= 0, returns a scanned AP info.
 \n	If ap_idx = -1, returns all scanned APs info.
 * \return number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_scan_entry(const char *phyname, const int ap_idx,
	struct clsapi_scan_ap_info **scan_ap_array);

/**
 * \brief Get Wi-Fi MAC filter policy.
 * \details Get Wi-Fi MAC filter policy. When MAC filter is enabled, STAs will be checked against the MAC list according to the policy. 3 Wi-Fi MAC filter policies are supported:
 \n	- "disable":	MAC filter is disabled;
 \n	- "allow":	STAs in the list are allowed;
 \n	- "deny":	STAs in the list are denied directly;

 * \param ifname [In] The Wi-Fi interface name.
 * \param policy [Out] Wi-Fi MAC filter policy.
 \n	CLSAPI_WIFI_MACFILTER_POLICY_DISABLED = 0
 \n	CLSAPI_WIFI_MACFILTER_POLICY_ALLOW
 \n	CLSAPI_WIFI_MACFILTER_POLICY_DENY
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_macfilter_policy(const char *ifname, enum clsapi_wifi_macfilter_policy *policy);

/**
 * \brief Set Wi-Fi MAC filter policy.
 * \details Set Wi-Fi MAC filter policy. When MAC filter is enabled, STAs will be checked against the MAC list according to the policy. 3 Wi-Fi MAC filter policies are supported:
 \n	- "disable":	MAC filter is disabled;
 \n	- "allow":	STAs in the list are allowed;
 \n	- "deny":	STAs in the list are denied directly;
 * \param ifname [In] The Wi-Fi interface name.
 * \param policy [In] New value of MAC filter policy.
 \n	CLSAPI_WIFI_MACFILTER_POLICY_DISABLED = 0
 \n	CLSAPI_WIFI_MACFILTER_POLICY_ALLOW
 \n	CLSAPI_WIFI_MACFILTER_POLICY_DENY
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_macfilter_policy(const char *ifname, const enum clsapi_wifi_macfilter_policy policy);

/**
 * \brief Get STA MAC list of Wi-Fi MAC filter.
 * \details Get STA MAC list of Wi-Fi MAC filter.
 * \param ifname [In] The Wi-Fi interface name.
 * \param mac_array [Out] The MAC list. Caller MUST free() after using.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_macfilter_maclist(const char *ifname, uint8_t (**mac_array)[ETH_ALEN]);

/**
 * \brief Add a STA MAC to the MAC filter list.
 * \details Add a STA MAC to the MAC filter list.
 * \param ifname [In] The Wi-Fi interface name.
 * \param sta_mac [In] The STA MAC will be added to the MAC filter list.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_add_macfilter_mac(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Delete a STA MAC from the MAC filter list.
 * \details Delete a STA MAC from the MAC filter list.
 * \param ifname [In] The Wi-Fi interface name.
 * \param sta_mac [In] The STA MAC will be deleted from the MAC filter list.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_del_macfilter_mac(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);


/******************************************************************************************************************/
/************************************************	AP&STA APIs	***************************************************/
/******************************************************************************************************************/

/**
 * \brief Get AP or STA interface name list.
 * \details Get interface name list which is Wi-Fi AP or STA on given radio.
 * \note
 *	During CAC period of 5GHz, interfaces except first one are not ready in driver. So they are absent in the list.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param iftype [In] The interface type, AP or STA.
 \n	CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE,
 \n	CLSAPI_WIFI_IFTYPE_AP,
 \n	CLSAPI_WIFI_IFTYPE_STA,
 * \param ifname_array [Out] The ifname list. Caller MUST free() after using.
 \n	If iftype = AP,	return AP ifname list.
 \n	If iftype = STA,	return STA ifname list.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_ifnames(const char *phyname, enum clsapi_wifi_iftype iftype, clsapi_ifname (**ifname_array));

/**
 * \brief Get the type of the Wi-Fi interface.
 * \details Get type of the Wi-Fi interface, like AP, STA, etc.
 * \param ifname [In] Wi-Fi interface name, e.g. wlan0, wlan1-1.
 * \param wifi_iftype [Out] The (virtual) interface type.
 \n	CLSAPI_WIFI_IFTYPE_NOSUCH_TYPE,
 \n	CLSAPI_WIFI_IFTYPE_AP,
 \n	CLSAPI_WIFI_IFTYPE_STA,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_iftype(const char *ifname, enum clsapi_wifi_iftype *wifi_iftype);

/**
 * \brief Get STA info.
 * \details Get STA info.
 * \param ifname [In] The name of the Wi-Fi interface which STA associates with, e.g wlan1-1
 * \param sta_mac [In] The MAC address of the STA.
 * \param sta [Out] The structure to carry the STA info out.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_sta_info(const char *ifname, const uint8_t sta_mac[ETH_ALEN], struct sta_info *sta);

/**
 * \brief Get STA SINR.
 * \details Get STA SINR.
 * \param ifname [In] The name of Wi-Fi interface, wlan0, wlan1, ...
 * \param sta_mac [In] MAC address of the STA.
 * \param sinr [Out] STA SINR.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_sinr(const char *ifname, const uint8_t sta_mac[ETH_ALEN], int8_t *sinr);

/**
 * \brief Get STA RSSI.
 * \details Get STA RSSI.
 * \param ifname [In] The name of Wi-Fi interface, wlan0, wlan1, ...
 * \param sta_mac [In] MAC address of the STA.
 * \param rssi [Out] STA RSSI.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_rssi(const char *ifname, const uint8_t sta_mac[ETH_ALEN], int8_t *rssi);

/**
 * \brief Deauthenticate a STA from VAP.
 * \details Deauthenticate a STA from VAP and send the reason code.
 * \param ifname [In] The Wi-Fi interface name.
 * \param sta_mac [In] MAC address of the STA.
 * \param reason_code [In] Reason code in Wi-Fi spec.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_deauth_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN], const uint16_t reason_code);

/**
 * \brief Disassociate a STA from VAP.
 * \details Disassociate a STA from VAP and send the reason code.
 * \param ifname [In] The Wi-Fi interface name.
 * \param sta_mac [In] MAC address of the STA.
 * \param reason_code [In] Reason code in Wi-Fi spec.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_disassoc_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN], const uint16_t reason_code);

/************************************************	WPS APIs	*******************************************************/
/**
 * \brief Set current WPS config method.
 * \details Set current WPS config method.
 * \param ifname [In] The Wi-Fi interface name
 * \param wps_config_method [In] The config method, which is a combination, that used for iface-X, refer to enum clsapi_wifi_wps_config_method, such as:
 \n	CLSAPI_WIFI_WPS_CFG_METHOD_LABEL
 \n	CLSAPI_WIFI_WPS_CFG_METHOD_PUSHBUTTON
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_wps_config_method(const char *ifname, const enum clsapi_wifi_wps_config_method wps_config_method);

/**
 * \brief Get current WPS config method.
 * \details Get current WPS config method.
 * \param ifname [In] The Wi-Fi interface name
 * \param wps_config_methods [Out] The config method that used for iface-X, this value is an enumeration, please refer to enum clsapi_wifi_wps_config_method for details.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_wps_config_method(const char *ifname, enum clsapi_wifi_wps_config_method *wps_config_methods);

/**
 * \brief Get suported WPS config method.
 * \details Get suported WPS config method.
 * \param supported_wps_config_methods [Out] The supported WPS config method that used for iface-X, and this value is obtained through or operations on enumerated values.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_supported_wps_config_methods(enum clsapi_wifi_wps_config_method *supported_wps_config_methods);

/**
 * \brief Get current WPS status.
 * \details Get current WPS status.
 * \param ifname [In] The Wi-Fi interface name.
 * \param wps_status [Out] The current status of WPS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_wps_status(const char *ifname, struct clsapi_wifi_wps_status *wps_status);

/**
 * \brief Validate the checksum of wps pin code.
 * \details Validate the checksum of wps pin code.
 * \param pin_code [In] The unsigned long type pin_code.
 */
int clsapi_wifi_check_wps_pin(const uint64_t pin_code);

/**
 * \brief Cancel current WPS session.
 * \details Cancel current WPS session.
 * \param ifname [In] The Wi-Fi interface name.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_cancel_wps(const char *ifname);

/**
 * \brief Start a WPS PBC session on a Wi-Fi interface.
 * \details Start a WPS PBC session on a a Wi-Fi interface.
 * \param ifname [In] The Wi-Fi interface name
 * \return 0 on success or others on error.
 */
int clsapi_wifi_trigger_wps_pbc_connection(const char *ifname);

/**
 * \brief Trigger AP and station connection by WPS.
 * \details Trigger AP and station connection by WPS.
 * \param ifname [In] The Wi-Fi interface name.
 * \param uuid [In] The UUID of WPS_PIN.
 * \param pin [In] The PIN_CODE of WPS_PIN.
 */
int clsapi_wifi_trigger_wps_pin_connection(const char *ifname, const char *uuid, const char *pin);

/**
 * \brief Get WPS static ap_pin code from config.
 * \details Get WPS static ap_pin code from config.
 * \param ifname [In] The Wi-Fi interface name.
 * \param ap_pin [Out] The WPS PIN code for "ap_pin" which is in hostapd.conf and /etc/config/wireless.
 */
int clsapi_wifi_get_wps_static_pin(const char *ifname, string_1024 ap_pin);

/**
 * \brief Set WPS static ap_pin code into config.
 * \details Set WPS static ap_pin code into config.
 * \param ifname [In] The Wi-Fi interface name.
 * \param ap_pin [In] The WPS PIN code for "ap_pin" which is in hostapd.conf.
 */
int clsapi_wifi_set_wps_static_pin(const char *ifname, const char *ap_pin);

/**
 * \brief Generate a random WPS PIN.
 * \details Generate a random WPS PIN.
 * \param pin [Out] Eight digit WPS PIN.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_generate_wps_pin(uint64_t *pin);

/**
 * \brief Get WPS uuid from config file.
 * \details Get WPS uuid from /etc/config/wireless.
 * \param ifname [In] The Wi-Fi interface name.
 * \param uuid [Out] The WPS uuid for "uuid" which is in hostapd.conf and /etc/config/wireless.
 */
int clsapi_wifi_get_wps_uuid(const char *ifname, string_1024 uuid);

/**
 * \brief Set WPS uuid into config file.
 * \details Set WPS uuid into /etc/config/wireless.
 * \param ifname [In] The Wi-Fi interface name.
 * \param uuid [In] The WPS uuid for "uuid" which is in hostapd.conf.
 */
int clsapi_wifi_set_wps_uuid(const char *ifname, const char *uuid);

/**
 * \brief Set WPS state
 * \details Set WPS state.
 * \param ifname [In] The Wi-Fi interface name.
 * \param wps_state [In] The value of wps state
 \n	CLSAPI_WIFI_WPS_STATE_DISABLED = 0
 \n	CLSAPI_WIFI_WPS_STATE_NOT_CONFIGURED
 \n	CLSAPI_WIFI_WPS_STATE_CONFIGURED
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_wps_state(const char *ifname, enum clsapi_wifi_wps_state wps_state);

/**
 * \brief Get WPS state.
 * \details Get WPS state.
 * \param ifname [In] The Wi-Fi interface name
 * \param wps_state [Out] The value of wps state
 \n	CLSAPI_WIFI_WPS_STATE_DISABLED = 0
 \n	CLSAPI_WIFI_WPS_STATE_NOT_CONFIGURED
 \n	CLSAPI_WIFI_WPS_STATE_CONFIGURED
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_wps_state(const char *ifname, enum clsapi_wifi_wps_state *wps_state);

/**
 * \brief Get Mesh role.
 * \details Get Mesh role.
 * \param role [Out] Mesh role.
 \n	CLSAPI_WIFI_MESH_ROLE_NONE,
 \n	CLSAPI_WIFI_MESH_ROLE_CONTROLLER,
 \n	CLSAPI_WIFI_MESH_ROLE_AGENT,
 \n	CLSAPI_WIFI_MESH_ROLE_AUTO,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_mesh_role(enum clsapi_wifi_mesh_role *role);

/**
 * \brief Set Mesh role.
 * \details Set Mesh role.
 * \param role [In] Mesh role.
 \n	CLSAPI_WIFI_MESH_ROLE_NONE,
 \n	CLSAPI_WIFI_MESH_ROLE_CONTROLLER,
 \n	CLSAPI_WIFI_MESH_ROLE_AGENT,
 \n	CLSAPI_WIFI_MESH_ROLE_AUTO,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_mesh_role(const enum clsapi_wifi_mesh_role role);


/**************************	Anti Mgmt Attack APIs	***************************/
/**
 * \brief Enable/Disable Anti Management Attack feature.
 * \details Enable/Disable Anti Management Attack feature. If time interval between management frame
 * (Authentication or Association Request) and last data frame is less than a threshold, the pattern
 * is recogonised as management attack. Then those management frames will be dropped.
 * \param ifname [In] The Wi-Fi interface name.
 * \param enable [In] New value of enabled/disabled Anti Management Attack feature.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_anti_mgmt_attack(const char *ifname, const bool enable);

/**
 * \brief Set time interval threshold of Anti Management Attack feature.
 * \details Set time interval threshold of Anti Management Attack feature. If interval between
 * management frame (Authentication or Association Request) and last data frame is less than the
 * threshold, the pattern is recogonised as management attack. Then those management frames will
 * be dropped.
 * \param ifname [In] The Wi-Fi interface name.
 * \param interval [In] New value of time threshold, in unit of second, between mgmt frame and
 * last data frame.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_anti_mgmt_attack_interval(const char *ifname, const uint32_t interval);

/**
 * \brief Get RSSI smoothness factor.
 * \details Get RSSI smoothness factor.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param factor [Out] RSSI smoothness factor.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_rssi_smoothness_factor(const char *phyname, uint8_t *factor);

/**
 * \brief Set RSSI smoothness factor.
 * \details Set RSSI smoothness factor.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param factor [In] RSSI smoothness factor.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_rssi_smoothness_factor(const char *phyname, const uint8_t factor);

/**
 * \brief Get statistics of the Wi-Fi radio.
 * \details Get statistics of the Wi-Fi radio.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param radio_stats [Out] Returned statistics of the Wi-Fi radio.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_radio_stats(const char *phyname, struct clsapi_wifi_radio_stats *radio_stats);

/**
 * \brief Reset statistics of the Wi-Fi radio.
 * \details Reset statistics of the Wi-Fi radio.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \return 0 on success or others on error.
 */
int clsapi_wifi_reset_radio_stats(const char *phyname);

/**
 * \brief Get statistics of the WPU.
 * \details Get statistics of the WPU. WPU (Wi-Fi Process Unit) is core dediated to handle Wi-Fi lower MAC.
 * \param ifname [In] The Wi-Fi primary interface name of the radio, used to identify the radio, e.g. wlan0, wlan1.
 * \param wpu_stats [Out] The returned statistics of the WPU.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_wpu_stats(const char *ifname, struct clsapi_wifi_wpu_stats *wpu_stats);

/**
 * \brief Reset statistics of the WPU.
 * \details Reset statistics of the WPU. WPU (Wi-Fi Process Unit) is core dediated to handle Wi-Fi lower MAC.
 * \param ifname [In] The Wi-Fi primary interface name, used to identify the WPU, e.g. wlan0, wlan1.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_reset_wpu_stats(const char *ifname);

/**
 * \brief Get statistics of the Wi-Fi VAP/BSS.
 * \details Get statistics of the Wi-Fi VAP/BSS.
 * \param ifname [In] The interface name of VAP, e.g. wlan0, wlan1-1.
 * \param vap_stats [Out] The returned statistics of the Wi-Fi VAP/BSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vap_stats(const char *ifname, struct clsapi_wifi_vap_stats *vap_stats);

/**
 * \brief Reset statistics of the Wi-Fi VAP/BSS.
 * \details Reset statistics of the Wi-Fi VAP/BSS.
 * \param ifname [In] The interface name of VAP, e.g. wlan0, wlan1-1.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_reset_vap_stats(const char *ifname);

/**
 * \brief Get statistics of the associated Wi-Fi STA.
 * \details Get statistics of the associated Wi-Fi STA.
 * \param ifname [In] The interface name of VAP, e.g. wlan0, wlan1-1.
 * \param sta_mac [In] MAC address of the STA.
 * \param sta_stats [Out] The returned statistics of the associated Wi-Fi STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_sta_stats(clsapi_ifname ifname, const uint8_t sta_mac[ETH_ALEN],
		struct clsapi_wifi_sta_stats *sta_stats);

/**
 * \brief Reset statistics of the associated Wi-Fi STA.
 * \details Reset statistics of the associated Wi-Fi STA.
 * \param ifname [In] The interface name of VAP, e.g. wlan0, wlan1-1.
 * \param sta_mac [In] MAC address of the STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_reset_sta_stats(clsapi_ifname ifname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Set dynamic CCA carrier sense threshold.
 * \details Set dynamic CCA carrier sense threshold.
 * \param phyname [In] The physical name of the radio.
 * \param threshold [In] The threshold of carrier sense threshold.
 * It's usually negative and in uint of dBm. By default is -60dBm.
 * \param delta [In] The delta value of threshold used to control threshold dynamiclly. By default is 0.
 * (threshold < delta < -threshold).
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_dynamic_cca_cs_threshold(const char *phyname, const int8_t threshold, const int8_t delta);

/**
 * \brief Get dynamic CCA carrier sense threshold.
 * \details Get dynamic CCA carrier sense threshold.
 * \param phyname [In] The physical name of the radio.
 * \param threshold [Out] The threshold of carrier sense threshold. It's usually negative and in uint of dBm.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_dynamic_cca_cs_threshold(const char *phyname, int8_t *threshold);

/**
 * \brief Set dynamic CCA energy detection threshold of the specified raido.
 * \details Set dynamic CCA energy detection threshold of 20MHz primary/secondary channel.
 * \param phyname [In] The physical name of the radio.
 * \param ed_req [In] The structure of cca_ed_config aims to set 20MHz primary/secondary threshold, in negative usually.
 * By default:
 \n CLS_NL80211_ATTR_CCA20PRISETHR:-62dBm
 \n CLS_NL80211_ATTR_CCA20PFALLTHR:-65dBm
 \n CLS_NL80211_ATTR_CCA20SRISETHR:-72dBm
 \n CLS_NL80211_ATTR_CCA20SFALLTHR:-75dBm
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_dynamic_cca_ed_threshold(const char *phyname, struct clsapi_cca_ed_config_req *ed_req);

/**
 * \brief Get dynamic CCA energy detection threshold of the specified radio.
 * \details Get dynamic CCA energy detection threshold of 20MHz primary/secondary channel.
 * \param phyname [In] The physical name of the radio.
 * \param ed_req [Out] The structure of cca_ed_config aims to set 20MHz primary/secondary
 * threshold, in negative usually.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_dynamic_cca_ed_threshold(const char *phyname, struct clsapi_cca_ed_config_req *ed_req);

/**
 * \brief Set protection method of ampdu.
 * \details Set protection method of ampdu, only for ampdu.
 * \param phyname [In] The physical name of the radio.
 * \param method [In] The method of protection, includes RTS_CTS/CTS-self/None.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_ampdu_protection(const char *phyname, enum clsapi_wifi_ampdu_protection method);

/**
 * \brief Configure intelligent antenna.
 * \details Configure intelligent antenna.
 * \param phyname [In] The physical name of the radio.
 * \param param [In] The intelligent antenna request parameter of intelligent antenna.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_intelligent_ant_param(const char *phyname, struct mm_smart_antenna_req *param);

/**
 * \brief Configure radius parameters.
 * \details Configure radius parameters, only if the encryption is wpa/wpa2....
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param conf [In] The radius configure parameters includes server IP, port. passphrase,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_radius_authentification(const char *ifname, struct clsapi_radius_configure *conf);

/**
 * \brief Get the configure of radius parameters.
 * \details Get the  configure of radius parameters, only if the encryption is wpa/wpa2....
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param conf [Out] The radius configure parameters includes server IP, port. passphrase,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_radius_authentification(const char *ifname, struct clsapi_radius_configure *conf);

/**
 * \brief Get the configure of radius acct parameters.
 * \details Get the  configure of radius acct parameters, only if the encryption is wpa/wpa2....
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param conf [Out] The radius acct configure parameters includes server IP, port. passphrase,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_radius_acct(const char *ifname, struct clsapi_radius_configure *conf);

/**
 * \brief Configure radius acct parameters.
 * \details Configure radius acct parameters, only if the encryption is wpa/wpa2....
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param conf [In] The radius acct configure parameters includes server IP, port. passphrase,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_radius_acct(const char *ifname, struct clsapi_radius_configure *conf);

/**
 * \brief Get the configure of radius dae parameters.
 * \details Get the  configure of radius dae parameters, only if the encryption is wpa/wpa2....
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param conf [Out] The radius acct configure parameters includes server IP, port. passphrase,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_radius_dae(const char *ifname, struct clsapi_radius_configure *conf);

/**
 * \brief Configure radius dae parameters.
 * \details Configure radius dae parameters, only if the encryption is wpa/wpa2....
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param conf [In] The radius acct configure parameters includes server IP, port. passphrase,
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_radius_dae(const char *ifname, struct clsapi_radius_configure *conf);

/**
 * \brief Get AP name by its station MAC address.
 * \details Get AP name by its station MAC address.
 * \param sta_macaddr [In] The MAC address of the station.
 * \param apname [Out] The name of AP which is associated with station.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_assoc_ap_by_station(const uint8_t sta_macaddr[ETH_ALEN], clsapi_ifname apname);

/**
 * \brief Set interface mode.
 * \details Set the mode of the interface, station or ap.
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \param mode [In] The mode of interface.
 \n CLSAPI_WIFI_INTERFACE_MODE_STA = 0
 \n CLSAPI_WIFI_INTERFACE_MODE_AP = 1
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_mode_of_interface(const char *ifname, enum clsapi_wifi_interface_mode mode);

/**
 * \brief Fill root ap information to enable station association.
 * \details Fill root ap information to enable station association.
 * \param ifname [In] The station interface name of Wi-Fi radio, wlan0, wlan1.
 * \param info [In] The information of rootap.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_station_association(const char *ifname, struct clsapi_wifi_rootap_info *info);

/**
 * \brief Retrieve uplink transmit/receive rate info for a station.
 * \details Retrieve uplink transmit/receive rate info for a station.
 * \param sta_ifname [In] The station interface name of Wi-Fi radio, wlan0, wlan1.
 * \param info [Out] The information of the station interface associate with rootap.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_uplink_info(const char *sta_ifname, struct uplink_info *info);

/**
 * \brief Restore wireless configuration
 * \details Restore wireless configuration
 * \return 0 on success or others on error.
 */
int clsapi_wifi_restore_wireless_configuration(void);

/**
 * \brief Get tx power ratio of the radio.
 * \details Get the tx power ratio of radio. refer to enum CLSAPI_WIFI_TXPWR_RATIO, Tx power
 * is restricted by regulator, fem, band, channel, etc.
 * \param phyname [In] The physical name of the radio.
 * \param txpower_ratio [Out] The ratio of txpower, refer to enum CLSAPI_WIFI_TXPWR_RATIO.
 \n CLSAPI_WIFI_TXPWR_RATIO_100 = 0
 \n	CLSAPI_WIFI_TXPWR_RATIO_75 = -2
 \n	CLSAPI_WIFI_TXPWR_RATIO_50 = -3
 \n	CLSAPI_WIFI_TXPWR_RATIO_25 = -6
 * \return 0 on success.
 */
int clsapi_wifi_get_txpower_ratio(const char *phyname, enum CLSAPI_WIFI_TXPWR_RATIO *txpower_ratio);

/**
 * \brief Set tx power ratio of the radio.
 * \details Set the tx power ratio of radio. refer to enum CLSAPI_WIFI_TXPWR_RATIO, Tx power
 * is restricted by regulator, fem, band, channel, etc.
 * \param phyname [In] The physical name of the radio.
 * \param txpower_ratio [In] The ratio of txpower, refer to enum CLSAPI_WIFI_TXPWR_RATIO.
 \n CLSAPI_WIFI_TXPWR_RATIO_100 = 0
 \n	CLSAPI_WIFI_TXPWR_RATIO_75 = -2
 \n	CLSAPI_WIFI_TXPWR_RATIO_50 = -3
 \n	CLSAPI_WIFI_TXPWR_RATIO_25 = -6
 * \return 0 on success.
 */
int clsapi_wifi_set_txpower_ratio(const char *phyname, enum CLSAPI_WIFI_TXPWR_RATIO txpower_ratio);

/**
 * \brief Choose which txpwr table will be used.
 * \details Choose which txpwr table will be used.
 * \param highpwr_mode [In] The mode of txpower, default means common txpower table, none-default means high txpower table.
 * \return 0 on success.
 */
int clsapi_wifi_set_txpower_table(const bool highpwr_mode);

/**
 * \brief Get which txpwr table will be used.
 * \details Get which txpwr table will be used.
 * \param highpwr_mode [Out] The mode of txpower, default means common txpower table, none-default means high txpower table.
 * \return 0 on success.
 */
int clsapi_wifi_get_txpower_table(bool *highpwr_mode);

/*!@} */


/*!\addtogroup wifi_csi
 *  @{
 */

/******************************************	CSI APIs	***************************************/

/**
 * \brief Enable/Disable CSI sampling and report.
 * \details Enable/Disable CSI sampling and report.
 * \param phyname [In] The physical name of the radio.
 * \param enable [In] New enabled/disabled setting of CSI sampling and report.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_csi(const char *phyname, const bool enable);

/** \brief Get enabled/disabled status of CSI sampling and report.
 * \details Get enabled/disabled status of CSI sampling and report.
 * \param phyname [In] The physical name of the radio.
 * \param enable [Out] Enable/disable status of CSI sampling and report.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_csi_enabled(const char *phyname, bool *enable);

/**
 * \brief Enable/Disable CSI sampling and report for non-associated STA.
 * \details Enable/Disable CSI sampling and report for non-associated STA.
 * \note
 *	Enabling CSI sampling for non-associated STA will enable promiscuous mode on this Wi-Fi radio.
 * This will \b DRAMATICALLY reduce Wi-Fi radio performance.
 * \param phyname [In] The physical name of the radio.
 * \param enable [In] New enabled/disabled setting of CSI sampling and report for non-associated STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_csi_non_assoc_sta(const char *phyname, const bool enable);

/**
 * \brief Get enabled/disabled status of CSI sampling and report for non-associated STA.
 * \details Get enabled/disabled status of CSI sampling and report for non-associated STA.
 * \param phyname [In] The physical name of the radio.
 * \param enable [Out] Enable/disable status of CSI sampling and report for non-associated STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_csi_non_assoc_sta_enabled(const char *phyname, bool *enable);

/**
 * \brief Enable/Disable smoothness for HE CSI.
 * \details Enable/Disable smoothness for HE CSI.
 * \param phyname [In] The physical name of the radio.
 * \param enable [In] New enabled/disabled setting of smooth for HE CSI.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_enable_csi_he_smooth(const char *phyname, const bool enable);

/**
 *\brief Get enabled/disabled status of smooth for HE CSI.
 * \details Get enabled/disabled status of smooth for HE CSI.
 * \param phyname [In] The physical name of the radio.
 * \param enable [Out] Enable/disable status of smooth for HE CSI.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_csi_he_smooth_enabled(const char *phyname, bool *enable);

/**
 * \brief Set CSI report period.
 * \details Set CSI report period, in unit of ms.
 * \param phyname [In] The physical name of the radio.
 * \param report_period [In] CSI report period, in unit of ms, range from 10 to 65535.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_csi_report_period(const char *phyname, const uint16_t report_period);

/**
 * \brief Get CSI report period.
 * \details Get CSI report period, in unit of ms.
 * \param phyname [In] The physical name of the radio.
 * \param report_period [Out] CSI report period, in unit of ms, range from 10 to 65535.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_csi_report_period(const char *phyname, uint16_t *report_period);

/**
 * \brief Add a STA to the CSI sampling list.
 * \details Add a STA to the CSI sampling list.
 * \param phyname [In] The physical name of the radio.
 * \param sta_mac [In] MAC address of STA to be CIS sampling.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_add_csi_sta(const char *phyname, const uint8_t sta_mac[ETH_ALEN]);

/** \brief Delete a STA from the CSI sampling list.
 * \details Delete a STA from the CSI sampling list.
 * \param phyname [In] The physical name of the radio.
 * \param sta_mac [In] MAC address of STA to be CIS sampling.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_del_csi_sta(const char *phyname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Get STA list of CSI sampling.
 * \details Get STA list of CSI sampling.
 * \param phyname [In] The physical name of the radio.
 * \param sta_array [Out] The STA MAC list. Caller MUST free() after using.
 * \return Number of obtained entires on success or negative error code on error.
 */
int clsapi_wifi_get_csi_sta_list(const char *phyname, uint8_t (**sta_array)[ETH_ALEN]);

/*!@} */

/************************************************	VBSS APIs	***************************************************/
/*!\addtogroup wifi_vbss
 *  @{
 */

/**
 * \brief Get on/off status of Wi-Fi VBSS.
 * \details Get on/off status of Wi-Fi VBSS.
 * \param phyname [In] The phy name of Wi-Fi radio, phy0, phy1, ...
 * \param enable [Out] On/off status of Wi-Fi VBSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_enabled(const char *phyname, bool *enable);

/**
 * \brief Enable/disable Wi-Fi VBSS.
 * \details Enable/disable Wi-Fi VBSS.
 * \param phyname [In] The phy name of Wi-Fi radio, phy0, phy1, ...
 * \param enable [In] Enable/disable Wi-Fi VBSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_vbss_enabled(const char *phyname, const bool enable);


/**
 * \brief Get Wi-Fi VBSS VAP information.
 * \details Get Wi-Fi VBSS VAP information.
 * \param ifname [In] The name of the Wi-Fi interface which info will be fectched from, e.g wlan0
 * \param vap [Out] The struct to carry the VAP info
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_vap(const char *ifname, struct vbss_vap_info *vap);

/**
 * \brief Add Wi-Fi VBSS VAP with given information.
 * \details Add Wi-Fi VBSS VAP with given information.
 * \param vap [In] The structure of the VAP info
 * \param new_ifname [In] Created Wi-Fi interface name, e.g wlan0-1
 * \param new_ifname_len [In] Length of ifname
 * \return 0 on success or others on error.
 */
int clsapi_wifi_add_vbss_vap(const struct vbss_vap_info *vap, char *new_ifname, const int new_ifname_len);

/**
 * \brief Get the Wi-Fi VBSS AP stats.
 * \details Get the Wi-Fi VBSS AP stats.
 * \param ifname [In] The name of the Wi-Fi interface to get AP stats, e.g wlan0
 * \param stats [Out] The struct to carry the VBSS per band stats
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_ap_stats(const char *ifname, struct vbss_ap_stats *stats);

/**
 * \brief Stop Wi-Fi VBSS VAP Tx function.
 * \details Stop Wi-Fi VBSS VAP Tx function.
 * \param ifname [In] The name of the Wi-Fi interface which the information will be fectched from, e.g wlan0
 * \return 0 on success or others on error.
 */
int clsapi_wifi_stop_vbss_vap_txq(const char *ifname);

/**
 * \brief Delete Wi-Fi VBSS VAP.
 * \details Delete Wi-Fi VBSS VAP.
 * \param ifname [In] Wi-Fi interface name, e.g wlan0
 * \return 0 on success or others on error.
 */
int clsapi_wifi_del_vbss_vap(const char *ifname);

/**
 * __NO_CLI_ENTRY__
 * \brief Get Wi-Fi VBSS STA info.
 * \details Get Wi-Fi VBSS STA info.
 * \param ifname [In] The name of the Wi-Fi interface which STA info will be fectched from, e.g wlan0
 * \param sta_mac [In] The mac address of the STA
 * \param sta [Out] The struct to carry the STA info
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN], struct vbss_sta_info *sta);

/**
 * __NO_CLI_ENTRY__
 * \brief Add Wi-Fi VBSS STA with given info.
 * \details Add Wi-Fi VBSS STA with given info.
 * \param ifname [In] The name of Wi-Fi interface which the STA will be created on, e.g wlan0
 * \param sta [In] The struct of the STA info
 * \return 0 on success or others on error.
 */
int clsapi_wifi_add_vbss_sta(const char *ifname, const struct vbss_sta_info *sta);

/**
 * \brief Delete Wi-Fi VBSS STA.
 * \details Delete Wi-Fi VBSS STA.
 * \param ifname [In] Wi-Fi interface name, e.g wlan0
 * \param sta_mac [In] The mac address of the STA
 * \return 0 on success or others on error.
 */
int clsapi_wifi_del_vbss_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Trigger VBSS switch now.
 * \details Trigger VBSS switch now.
 * \param ifname [In] The name of the Wi-Fi interface which the STA associated, e.g wlan0
 * \param sta_mac [In] The mac address of the STA
 * \return 0 on success or others on error.
 */
int clsapi_wifi_trigger_vbss_switch(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Get VBSS roam result.
 * \details Get VBSS roam result.
 * \param ifname [In] The name of the Wi-Fi interface which the STA associated, e.g wlan0
 * \param sta_mac [In] The mac address of the STA
 * \param roam_result [Out] The result of the VBSS roam
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_roam_result(const char *ifname, const uint8_t sta_mac[ETH_ALEN], uint32_t *roam_result);

/**
 * \brief Set VBSS switch done.
 * \details Set VBSS switch done.
 * \param ifname [In] The name of the Wi-Fi interface which the STA associated, e.g wlan0
 * \param sta_mac [In] The mac address of the STA
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_vbss_switch_done(const char *ifname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Get VBSS N_thresh. N_thresh is threshold of pkt cnt which pkt is decrypted successful consecutively.
 * \details Get VBSS N_thresh. N_thresh is threshold of pkt cnt which pkt is decrypted successful consecutively.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param n_thresh [Out] N_thresh of VBSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_nthresh(const char *phyname, uint32_t *n_thresh);

/**
 * \brief Set VBSS N_thresh.
 * \details Set VBSS N_thresh.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param n_thresh [In] N_thresh of VBSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_vbss_nthresh(const char *phyname, const uint32_t n_thresh);

/**
 * \brief Get VBSS M_thresh. M_thresh is threshold of pkt cnt which pkt is decrypted failed consecutively.
 * \details Get VBSS M_thresh. M_thresh is threshold of pkt cnt which pkt is decrypted failed consecutively.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param m_thresh [Out] M_thresh of VBSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_vbss_mthresh(const char *phyname, uint32_t *m_thresh);

/**
 * \brief Set VBSS M_thresh.
 * \details Set VBSS M_thresh.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param m_thresh [In] M_thresh of VBSS.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_vbss_mthresh(const char *phyname, const uint32_t m_thresh);

/**
 * \brief Monitor STA, e.g. RSSI, SINR, on non-associated AP.
 * \details  Monitor STA, e.g. RSSI, SINR, on non-associated AP.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param sta_mac [In] MAC address of the STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_add_vbss_monitor_sta(const char *phyname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Stop monitor STA, e.g. RSSI, SINR, on non-associated AP.
 * \details Stop monitor STA, e.g. RSSI, SINR, on non-associated AP.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param sta_mac [In] MAC address of the STA.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_del_vbss_monitor_sta(const char *phyname, const uint8_t sta_mac[ETH_ALEN]);

/**
 * \brief Disable VBSS rekey.
 * \details Disable VBSS rekey.
 * \param ifname [In] The interface name of Wi-Fi radio, wlan0, wlan1, ...
 * \return 0 on success or others on error.
 */
int clsapi_wifi_set_vbss_stop_rekey(const char *ifname);

/**
 * \brief Get basic information on mesh mode.
 * \details Get basic information on mesh mode, such as backhaul bss ifname.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param info [Out] mesh bss ifname
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_mesh_bss_ifname(const char *phyname, struct mesh_bss_info *info);

/**
 * \brief Get current channel utilization.
 * \details Get current channel utilization.
 * Include current frequency, bss load update period and channel utilization average update period.
 * Off-channel scan is necessary before getting utilization and this API is only for clsemi hostapd.
 * \param phyname [In] The name of Wi-Fi radio, phy0, phy1, ...
 * \param channel_utilization [Out] The current channel utiliazation.
 * \return 0 on success or others on error.
 */
int clsapi_wifi_get_current_channel_utilization(const char *phyname, uint8_t *channel_utilization);

/*!@} */

#endif /* _CLSAPI_WIFI_H */

