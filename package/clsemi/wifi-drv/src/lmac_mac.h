/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#ifndef _LMAC_MAC_H_
#define _LMAC_MAC_H_

#include "lmac_types.h"

/// Interface types
enum mac_vif_type {
	/// ESS STA interface
	VIF_STA,
	/// IBSS STA interface
	VIF_IBSS,
	/// AP interface
	VIF_AP,
	/// Mesh Point interface
	VIF_MESH_POINT,
	/// Monitor interface
	VIF_MONITOR,
	/// Unknown type
	VIF_UNKNOWN,
};

/// MAC address length in bytes.
#define MAC_ADDR_LEN 6

/// MAC address structure.
struct mac_addr {
	/// Array of 16-bit words that make up the MAC address.
	u16_l array[MAC_ADDR_LEN/2];
};

/// SSID maximum length.
#define MAC_SSID_LEN 32

/// SSID.
struct mac_ssid {
	/// Actual length of the SSID.
	u8_l length;
	/// Array containing the SSID name.
	u8_l array[MAC_SSID_LEN];
};

/// BSS type
enum mac_bss_type {
	INFRASTRUCTURE_MODE = 1,
	INDEPENDENT_BSS_MODE,
	ANY_BSS_MODE,
};

/// Channel Band
enum mac_chan_band {
	/// 2.4GHz Band
	PHY_BAND_2G4,
	/// 5GHz band
	PHY_BAND_5G,
	/// Number of bands
	PHY_BAND_MAX,
};

/// Operating Channel Bandwidth
enum mac_chan_bandwidth {
	/// 20MHz BW
	PHY_CHNL_BW_20,
	/// 40MHz BW
	PHY_CHNL_BW_40,
	/// 80MHz BW
	PHY_CHNL_BW_80,
	/// 160MHz BW
	PHY_CHNL_BW_160,
	/// 80+80MHz BW
	PHY_CHNL_BW_80P80,
	/// Reserved BW
	PHY_CHNL_BW_OTHER,
};

/// max number of channels in the 2.4 GHZ band
#define MAC_DOMAINCHANNEL_24G_MAX 14

/// max number of channels in the 5 GHZ band
#define MAC_DOMAINCHANNEL_5G_MAX 28

/// Channel Flag
enum mac_chan_flags {
	/// Cannot initiate radiation on this channel
	CHAN_NO_IR = BIT(0),
	/// Channel is not allowed
	CHAN_DISABLED = BIT(1),
	/// Radar detection required on this channel
	CHAN_RADAR = BIT(2),
	/// Can be primary channel of HT40- operating channel
	CHAN_HT40M = BIT(3),
	/// Can be primary channel of HT40+ operating channel
	CHAN_HT40P = BIT(4),
	/// Can be primary channel of VHT80 (1st position 10/70) operating channel
	CHAN_VHT80_10_70 = BIT(5),
	/// Can be primary channel of VHT80 (2nd position 30/50) operating channel
	CHAN_VHT80_30_50 = BIT(6),
	/// Can be primary channel of VHT80 (3rd position 50/30) operating channel
	CHAN_VHT80_50_30 = BIT(7),
	/// Can be primary channel of VHT80 (4th position 70/10) operating channel
	CHAN_VHT80_70_10 = BIT(8)
};

/// Primary Channel definition
struct mac_chan_def {
	/// Frequency of the channel (in MHz)
	u16_l freq;
	/// RF band (@ref mac_chan_band)
	u8_l band;
	/// Max transmit power allowed on this channel (dBm)
	s8_l tx_power;
	/// Additional information (@ref mac_chan_flags)
	u16_l flags;
};

/// Operating Channel
struct mac_chan_op {
	/// Band (@ref mac_chan_band)
	u8_l band;
	/// Channel type (@ref mac_chan_bandwidth)
	u8_l type;
	/// Frequency for Primary 20MHz channel (in MHz)
	u16_l prim20_freq;
	/// Frequency center of the contiguous channel or center of Primary 80+80 (in MHz)
	u16_l center1_freq;
	/// Frequency center of the non-contiguous secondary 80+80 (in MHz)
	u16_l center2_freq;
	/// Additional information (@ref mac_chan_flags)
	u16_l flags;
	/// Max transmit power allowed on this channel (dBm)
	s8_l tx_power;
};

/// Cipher suites
enum mac_cipher_suite {
	/// 00-0F-AC 1
	MAC_CIPHER_WEP40,
	/// 00-0F-AC 2
	MAC_CIPHER_TKIP,
	/// 00-0F-AC 4 (aka CCMP-128)
	MAC_CIPHER_CCMP,
	/// 00-0F-AC 5
	MAC_CIPHER_WEP104,
	/// 00-14-72 1
	MAC_CIPHER_WPI_SMS4,
	/// 00-0F-AC 6 (aka AES_CMAC)
	MAC_CIPHER_BIP_CMAC_128,
	/// 00-0F-AC 08
	MAC_CIPHER_GCMP_128,
	/// 00-0F-AC 09
	MAC_CIPHER_GCMP_256,
	/// 00-0F-AC 10
	MAC_CIPHER_CCMP_256,

	// following cipher are not supported by MACHW
	/// 00-0F-AC 11
	MAC_CIPHER_BIP_GMAC_128,
	/// 00-0F-AC 12
	MAC_CIPHER_BIP_GMAC_256,
	/// 00-0F-AC 13
	MAC_CIPHER_BIP_CMAC_256,

	MAC_CIPHER_INVALID = 0xFF,
};

/// Authentication and Key Management suite
enum mac_akm_suite {
	/// No security
	MAC_AKM_NONE,
	/// Pre RSN (WEP or WPA)
	MAC_AKM_PRE_RSN,
	/// 00-0F-AC 1
	MAC_AKM_8021X,
	/// 00-0F-AC 2
	MAC_AKM_PSK,
	/// 00-0F-AC 3
	MAC_AKM_FT_8021X,
	/// 00-0F-AC 4
	MAC_AKM_FT_PSK,
	/// 00-0F-AC 5
	MAC_AKM_8021X_SHA256,
	/// 00-0F-AC 6
	MAC_AKM_PSK_SHA256,
	/// 00-0F-AC 7
	MAC_AKM_TDLS,
	/// 00-0F-AC 8
	MAC_AKM_SAE,
	/// 00-0F-AC 9
	MAC_AKM_FT_OVER_SAE,
	/// 00-0F-AC 11
	MAC_AKM_8021X_SUITE_B,
	/// 00-0F-AC 12
	MAC_AKM_8021X_SUITE_B_192,
	/// 00-0F-AC 14
	MAC_AKM_FILS_SHA256,
	/// 00-0F-AC 15
	MAC_AKM_FILS_SHA384,
	/// 00-0F-AC 16
	MAC_AKM_FT_FILS_SHA256,
	/// 00-0F-AC 17
	MAC_AKM_FT_FILS_SHA384,
	/// 00-0F-AC 18
	MAC_AKM_OWE,

	/// 00-14-72 1
	MAC_AKM_WAPI_CERT,
	/// 00-14-72 2
	MAC_AKM_WAPI_PSK,

	/// 50-6F-9A 2
	MAC_AKM_DPP,
};

/// Scan result element, parsed from beacon or probe response frames.
struct mac_scan_result {
	/// Scan result is valid
	bool valid_flag;
	/// Network BSSID.
	struct mac_addr bssid;
	/// Network name.
	struct mac_ssid ssid;
	/// Network type (@ref mac_bss_type).
	u16_l bsstype;
	/// Network channel.
	struct mac_chan_def *chan;
	/// Supported AKM (bit-field of @ref mac_akm_suite)
	u32_l akm;
	/// Group cipher (bit-field of @ref mac_cipher_suite)
	u16_l group_cipher;
	/// Group cipher (bit-field of @ref mac_cipher_suite)
	u16_l pairwise_cipher;
	/// RSSI of the scanned BSS (in dBm)
	s8_l rssi;
	/// Multi-BSSID index (0 if this is the reference (i.e. transmitted) BSSID)
	u8_l multi_bssid_index;
	/// Maximum BSSID indicator
	u8_l max_bssid_indicator;
	/// FTM support
	bool ftm_support;
};

/// Legacy rate 802.11 definitions as found in (Extended) Supported Rates Elements
enum mac_legacy_rates {
	/// DSSS/CCK 1Mbps
	MAC_RATE_1MBPS   =   2,
	/// DSSS/CCK 2Mbps
	MAC_RATE_2MBPS   =   4,
	/// DSSS/CCK 5.5Mbps
	MAC_RATE_5_5MBPS =  11,
	/// OFDM 6Mbps
	MAC_RATE_6MBPS   =  12,
	/// OFDM 9Mbps
	MAC_RATE_9MBPS   =  18,
	/// DSSS/CCK 11Mbps
	MAC_RATE_11MBPS  =  22,
	/// OFDM 12Mbps
	MAC_RATE_12MBPS  =  24,
	/// OFDM 18Mbps
	MAC_RATE_18MBPS  =  36,
	/// OFDM 24Mbps
	MAC_RATE_24MBPS  =  48,
	/// OFDM 36Mbps
	MAC_RATE_36MBPS  =  72,
	/// OFDM 48Mbps
	MAC_RATE_48MBPS  =  96,
	/// OFDM 54Mbps
	MAC_RATE_54MBPS  = 108,

	/// BSS Membership Selector: HT PHY
	MAC_BSS_MEMBERSHIP_HT_PHY = 127,
	/// BSS Membership Selector: VHT PHY
	MAC_BSS_MEMBERSHIP_VHT_PHY = 126,
	/// BSS Membership Selector: HE PHY
	MAC_BSS_MEMBERSHIP_HE_PHY = 122,
};

/// MAC rateset maximum length
#define MAC_RATESET_LEN 12

/// Structure containing the legacy rateset of a station
struct mac_rateset {
	/// Number of legacy rates supported
	u8_l length;
	/// Array of legacy rates
	u8_l array[MAC_RATESET_LEN];
};

/// MAC Security Key maximum length
#define MAC_SEC_KEY_LEN 32  // TKIP keys 256 bits (max length) with MIC keys

/// Structure defining a security key
struct mac_sec_key {
	/// Key material length
	u8_l length;
	/// Key material
	u32_l array[MAC_SEC_KEY_LEN/4];
};

/// Access Category enumeration
enum mac_ac {
	/// Background
	AC_BK = 0,
	/// Best-effort
	AC_BE,
	/// Video
	AC_VI,
	/// Voice
	AC_VO,
	/// Number of access categories
	AC_MAX,
};

/// Traffic ID enumeration
enum mac_tid {
	/// TID_0. Mapped to @ref AC_BE as per 802.11 standard.
	TID_0,
	/// TID_1. Mapped to @ref AC_BK as per 802.11 standard.
	TID_1,
	/// TID_2. Mapped to @ref AC_BK as per 802.11 standard.
	TID_2,
	/// TID_3. Mapped to @ref AC_BE as per 802.11 standard.
	TID_3,
	/// TID_4. Mapped to @ref AC_VI as per 802.11 standard.
	TID_4,
	/// TID_5. Mapped to @ref AC_VI as per 802.11 standard.
	TID_5,
	/// TID_6. Mapped to @ref AC_VO as per 802.11 standard.
	TID_6,
	/// TID_7. Mapped to @ref AC_VO as per 802.11 standard.
	TID_7,
	/// Non standard Management TID used internally
	TID_MGT,
	/// Number of TID supported
	TID_MAX,
};

/// MCS bitfield maximum size (in bytes)
#define MAX_MCS_LEN 16 // 16 * 8 = 128

/// MAC HT capability information element
struct mac_htcapability {
	/// HT capability information
	u16_l ht_capa_info;
	/// A-MPDU parameters
	u8_l a_mpdu_param;
	/// Supported MCS
	u8_l mcs_rate[MAX_MCS_LEN];
	/// HT extended capability information
	u16_l ht_extended_capa;
	/// Beamforming capability information
	u32_l tx_beamforming_capa;
	/// Antenna selection capability information
	u8_l asel_capa;
};

/// MAC VHT capability information element
struct mac_vhtcapability {
	/// VHT capability information
	u32_l vht_capa_info;
	/// RX MCS map
	u16_l rx_mcs_map;
	/// RX highest data rate
	u16_l rx_highest;
	/// TX MCS map
	u16_l tx_mcs_map;
	/// TX highest data rate
	u16_l tx_highest;
};

/// Length (in bytes) of the MAC HE capability field
#define MAC_HE_MAC_CAPA_LEN 6
/// Length (in bytes) of the PHY HE capability field
#define MAC_HE_PHY_CAPA_LEN 11
/// Maximum length (in bytes) of the PPE threshold data
#define MAC_HE_PPE_THRES_MAX_LEN 25

/// Structure listing the per-NSS, per-BW supported MCS combinations
struct mac_he_mcs_nss_supp {
	/// per-NSS supported MCS in RX, for BW <= 80MHz
	u16_l rx_mcs_80;
	/// per-NSS supported MCS in TX, for BW <= 80MHz
	u16_l tx_mcs_80;
	/// per-NSS supported MCS in RX, for BW = 160MHz
	u16_l rx_mcs_160;
	/// per-NSS supported MCS in TX, for BW = 160MHz
	u16_l tx_mcs_160;
	/// per-NSS supported MCS in RX, for BW = 80+80MHz
	u16_l rx_mcs_80p80;
	/// per-NSS supported MCS in TX, for BW = 80+80MHz
	u16_l tx_mcs_80p80;
};

/// MAC HE capability information element
struct mac_hecapability {
	/// MAC HE capabilities
	u8_l mac_cap_info[MAC_HE_MAC_CAPA_LEN];
	/// PHY HE capabilities
	u8_l phy_cap_info[MAC_HE_PHY_CAPA_LEN];
	/// Supported MCS combinations
	struct mac_he_mcs_nss_supp mcs_supp;
	/// PPE Thresholds data
	u8_l ppe_thres[MAC_HE_PPE_THRES_MAX_LEN];
};

/// Length (in bytes) of the MAC EHT capability field
#define MAC_EHT_MAC_CAPA_LEN 2
/// Length (in bytes) of the PHY EHT capability field
#define MAC_EHT_PHY_CAPA_LEN 9
/* TODO: add it when needed. */
/// Maximum length (in bytes) of the PPE threshold data
#define MAC_EHT_PPE_THRES_MAX_LEN 0

enum {
	EHT_MCS_NSS_MAP_20_80M,
	EHT_MCS_NSS_MAP_160M,
	EHT_MCS_NSS_MAP_MAX,
};

enum eht_20m_only_mcs_map {
	EHT_20M_MCS_MAP_0_7 = 0,
	EHT_20M_MCS_MAP_8_9 = 1,
	EHT_20M_MCS_MAP_10_11 = 2,
	EHT_20M_MCS_MAP_12_13 = 3,
	EHT_20M_MCS_MAP_MAX,
};

enum eht_mcs_map_nss_range {
	EHT_MCS_MAP_NSS_NOT_SUPPORT = 0,
	EHT_MCS_MAP_NSS1 = 1,
	EHT_MCS_MAP_NSS2 = 2,
	EHT_MCS_MAP_NSS_MAX = EHT_MCS_MAP_NSS2,
};

enum eht_mcs_map_mcs_range {
	EHT_MCS_MAP_0_9 = 0,
	EHT_MCS_MAP_10_11 = 1,
	EHT_MCS_MAP_12_13 = 2,
	EHT_MCS_MAP_NOT_SUPPORTED = 3,
	EHT_MCS_MAP_MAX = EHT_MCS_MAP_NOT_SUPPORTED,
};

enum eht_mcs_nss_txrx_range {
	EHT_MCS_MAP_0_9_RX = 0,
	EHT_MCS_MAP_0_9_TX = 1,
	EHT_MCS_MAP_10_11_RX = 2,
	EHT_MCS_MAP_10_11_TX = 3,
	EHT_MCS_MAP_12_13_RX = 4,
	EHT_MCS_MAP_12_13_TX = 5,
	EHT_MCS_MAP_RX_TX_MAX,
};

/// Structure listing the per-NSS, per-BW supported MCS combinations
struct mac_eht_mcs_nss_supp
{
//	uint32_t mcs_20m_only;
	/* MCS Less or Equal to 80MHz (including 20M only)*/
	uint32_t mcs_le_80m;
	uint32_t mcs_160m;
//	uint32_t mcs_320m;
};

/// MAC EHT capability information element
struct mac_eht_capability
{
    /// MAC EHT capabilities (@ref eht_capa_mac_bf)
    uint8_t mac_cap_info[MAC_EHT_MAC_CAPA_LEN];
    /// PHY EHT capabilities (@ref eht_capa_phy_bf)
    uint8_t phy_cap_info[MAC_EHT_PHY_CAPA_LEN];
    /// Supported MCS combinations
    struct mac_eht_mcs_nss_supp mcs_supp;
    /// PPE Thresholds data (@ref he_capa_ppe_threshold_bf)
    uint8_t ppe_thres[MAC_EHT_PPE_THRES_MAX_LEN];
};

/// Station flags
enum mac_sta_flags {
	/// Bit indicating that a STA has QoS (WMM) capability
	STA_QOS_CAPA = BIT(0),
	/// Bit indicating that a STA has HT capability
	STA_HT_CAPA = BIT(1),
	/// Bit indicating that a STA has VHT capability
	STA_VHT_CAPA = BIT(2),
	/// Bit indicating that a STA has MFP capability
	STA_MFP_CAPA = BIT(3),
	/// Bit indicating that the STA included the Operation Notification IE
	STA_OPMOD_NOTIF = BIT(4),
	/// Bit indicating that a STA has HE capability
	STA_HE_CAPA = BIT(5),
	/// Bit Inidcating supprot for short Preamble in ERP
	STA_SHORT_PREAMBLE_CAPA = BIT(6),
	/* Bit indicating that a STA has EHT capability */
	STA_EHT_CAPA = BIT(7),
};

/// Station RC changed
enum mac_sta_rc_changed {
	/// Bit indicating that the bandwidth has been changed
	STA_RC_BW_CHANGED = BIT(0),
	/// Bit indicating that the SMPS has been changed
	STA_RC_SMPS_CHANGED = BIT(1),
	/// Bit indicating that the supported rates has been changed
	STA_RC_SUPP_RATES_CHANGED = BIT(2),
	/// Bit indicating that the NSS has been changed
	STA_RC_NSS_CHANGED = BIT(3),
	/// Bit indicating that the HE-ER has been changed
	STA_RC_HE_ER_CHANGED = BIT(4),
};

/// BSS Info Changed
enum mac_bss_changed {
	/// CTS protection changed
	BSS_CHG_CTS_PROT = BIT(0),
	/// Preamble changed
	BSS_CHG_ERP_PREAMBLE = BIT(1),
	/// Basic rateset changed
	BSS_CHG_BASIC_RATES = BIT(2),
	/// BSS HE color changed (SoftMAC only)
	BSS_CHG_HE_COLOR = BIT(3),
};

/// Connection flags
enum mac_connection_flags {
	/// Flag indicating whether the control port is controlled by host or not
	CONTROL_PORT_HOST = BIT(0),
	/// Flag indicating whether the control port frame shall be sent unencrypted
	CONTROL_PORT_NO_ENC = BIT(1),
	/// Flag indicating whether HT and VHT shall be disabled or not
	DISABLE_HT = BIT(2),
	/// Flag indicating whether a pairwise key has to be used
	USE_PAIRWISE_KEY = BIT(3),
	/// Flag indicating whether MFP is in use
	MFP_IN_USE = BIT(4),
	/// Flag indicating whether Reassociation should be used instead of Association
	REASSOCIATION = BIT(5),
	/// Flag indicating Connection request if part of FT over DS
	FT_OVER_DS = BIT(6),
	/// Flag indicating whether encryption is used or not on the connection
	USE_PRIVACY = BIT(7),
	/// Flag indicating whether usage of SPP A-MSDUs is required
	REQUIRE_SPP_AMSDU = BIT(8),
};

/// Max number of FTM responders per request
#define FTM_RSP_MAX 5

/// FTM results
struct mac_ftm_results {
	/// Number of FTM responders for which we have a measurement
	u8_l nb_ftm_rsp;
	/// Per-responder measurements
	struct {
		/// MAC Address of the FTM responder
		struct mac_addr addr;
		/// Round Trip Time (in ps)
		u32_l rtt;
	} meas[FTM_RSP_MAX];
};

#endif // _LMAC_MAC_H_
