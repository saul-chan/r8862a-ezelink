#ifndef _CLSAPI_H
#define _CLSAPI_H

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <net/ethernet.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

/*
 * The following #defines come from the TR-098 standard.
 */
#ifndef IEEE80211_CHAN_MAX
#define IEEE80211_CHAN_MAX		255
#endif

#ifndef IW_ESSID_MAX_SIZE
#define IW_ESSID_MAX_SIZE		32
#endif

#define IW_ESSID_STR_MAX_SIZE		(IW_ESSID_MAX_SIZE * 2 + 1)
#define CLSAPI_SSID_MAXLEN	(IW_ESSID_MAX_SIZE + 1)
#define CLSAPI_SSID_MAXNUM	32
#define CLSAPI_STATUS_MAXLEN	12
#define CLSAPI_SSID_MAX_RECORDS (6)

#define CLSAPI_SET_PARAMS_ARG_MAX	8

#define CLS_HE_SIG_TXBF_ENABLE_PRECODE_APPLIED 0
#define CLS_HE_SIG_TXBF_ENABLE_CBF_RECEIVED 1
#define CLS_HE_SIG_TXBF_ENABLE_ALWAYS 2
#define CLS_HE_SIG_TXBF_DISABLE_ALWAYS 3
#define CLS_HE_SIG_TXBF_CTL_MAX 4

typedef char clsapi_SSID[ CLSAPI_SSID_MAXLEN ];

struct clsapi_data_256bytes {
	uint8_t data[256];
};

enum clsapi_chlist_flags {
	/**
	 * List of channels supported in the hardware.
	 */
	clsapi_chlist_flag_available	= 0x00000001,
	/**
	 * List of disabled channels. This includes
	 * the channels disabled by the user.
	 */
	clsapi_chlist_flag_disabled	= 0x00000002,
	/**
	 * List of active scan channels. This includes
	 * the channels in the active list but excludes
	 * the ones disabled from the scan list by the user.
	 */
	clsapi_chlist_flag_scan		= 0x00000004,
	/**
	 * List of active channels. This includes
	 * the channels supported in the current regulatory region for
	 * the current operating bandwidth and also supported by the hardware
	 * but excludes the channels in the disabled list.
	 */
	clsapi_chlist_flag_active	= 0x00000008,
	/**
	 * List of OCAC off-channels.
	 */
	clsapi_chlist_flag_ocac_off	= 0x00000010,
};

typedef enum
{
	/**
	 * The PMF capability is disbled.
	 */
	clsapi_pmf_disabled = 0,
	/**
	 * The PMF capability is optional.
	 */
	clsapi_pmf_optional = 1,
	/**
	 * The PMF capability is required.
	 */
	clsapi_pmf_required = 2

} clsapi_pmf;



enum {
	CLS_HE_NDP_GI_LTF_TYPE_0 = 0,	/* 0.8us GI with 2x HE-LTF */
	CLS_HE_NDP_GI_LTF_TYPE_1,	/* 1.6us GI with 2x HE-LTF */
	CLS_HE_NDP_GI_LTF_TYPE_2,	/* 3.2us GI with 4x HE-LTF */
	CLS_HE_NDP_GI_LTF_TYPE_MAX
};



/* Channel 0 means channel auto */
#define CLSAPI_ANY_CHANNEL	0
#define CLSAPI_MIN_CHANNEL	1
#define CLSAPI_MIN_CHANNEL_5G	36
#define CLSAPI_MAX_CHANNEL	255
#define CLSAPI_MAX_CHANNEL_EXT	511

enum clsapi_freq_band {
	clsapi_freq_band_2pt4_ghz	= 0,
	clsapi_freq_band_default	= 1, /* 2.4GHz or 5GHz */
	//clsapi_freq_band_4_ghz		= 2,
	clsapi_freq_band_5_ghz		= 2,
	clsapi_freq_band_6_ghz		= 3,
	clsapi_freq_band_unknown	= 0xFF
};

enum clsapi_beacon_type {
	clsapi_beacon_basic	= 0,
	clsapi_beacon_wpa	= 1,
	clsapi_beacon_11i	= 2,
	clsapi_beacon_wpaand11i	= 3
};

typedef enum
{
	/**
	 * The default bandwidth of the device
	 */
	clsapi_bw_default = 0,
	/**
	 * The device is operating in 20MHz mode.
	 */
	clsapi_bw_20MHz = 20,
	/**
	 * The device is operating in 40MHz mode.
	 */
	clsapi_bw_40MHz = 40,
	/**
	 * The device is operating in 80MHz mode.
	 */
	clsapi_bw_80MHz = 80,
	/**
	 * The device is operating in 160MHz mode.
	 */
	clsapi_bw_160MHz = 160,
	/**
	 * Placeholder - unknown bandwidth (indicates error).
	 */
	clsapi_nosuch_bw
} clsapi_bw;


#define MACFILTERINGMACFMT	"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDR_STRING_LENGTH	18
#define MAC_ADDR_LIST_SIZE	8


#define MAC_ADDR_SET_LOCAL(_a)		(*(_a) |= (1 << 1))

enum cls_he_ru_size {
	CLS_HE_RU_SIZE_26 = 0,
	CLS_HE_RU_SIZE_52,
	CLS_HE_RU_SIZE_106,
	CLS_HE_RU_SIZE_242,
	CLS_HE_RU_SIZE_484,
	CLS_HE_RU_SIZE_996,
	CLS_HE_RU_SIZE_996x2,
	CLS_HE_RU_SIZE_MAX
};

#define CLSAPI_PM_MODE_AUTO	-1

typedef	unsigned int	clsapi_unsigned_int;
#define CLS_PKTGEN_STREAMS_MAX		4
#define CLS_PKTGEN_INTERVAL_MIN_US	IEEE80211_MS_TO_USEC(5)
#define CLS_PKTGEN_INTERVAL_MAX_US	IEEE80211_MS_TO_USEC(5000)
#define CLS_PKTGEN_INTERVAL_DEFAULT_US	IEEE80211_TU_TO_USEC(20)
#define CLS_PKTGEN_PENDING_PKTS_MAX	8

#define	IEEE80211_MLME_DEAUTH		3	/* deauthenticate station */

#define CDRV_BLD_NAME "1.0"
#define SIOCDEV_SUBIO_BASE			(0)
#define SIOCDEV_SUBIO_SET_MARK_DFS_CHAN	(SIOCDEV_SUBIO_BASE + 14)

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#define SIOCIWFIRSTPRIV	0x8BE0
#define	IEEE80211_IOCTL_SETMLME		(SIOCIWFIRSTPRIV+16)

typedef unsigned char clsapi_mac_addr[6];

typedef enum {
	clsapi_hw_revision = 1,
	clsapi_hw_id,
	clsapi_hw_desc,
	clsapi_rf_chipid,
	clsapi_bond_opt,
	clsapi_vht,
	clsapi_bandwidth,
	clsapi_spatial_stream,
	clsapi_interface_types,
	clsapi_rf_chip_verid,
	clsapi_name,
} clsapi_board_parameter_type;

enum clsapi_errno{
	clsapi_errno_base = 1000,
	clsapi_system_not_started = clsapi_errno_base,
	clsapi_parameter_not_found = clsapi_errno_base + 1,
	clsapi_SSID_not_found = clsapi_errno_base +2,
	clsapi_only_on_AP = clsapi_errno_base +3,
	clsapi_only_on_STA = clsapi_errno_base + 4,
	clsapi_configuration_error = clsapi_errno_base + 5,
} ;

/**
 * \brief Enumeration used in the option set/get API.
 *
 * \sa clsapi_wifi_get_option
 * \sa clsapi_wifi_set_option
 */
typedef enum {
	clsapi_channel_refresh = 1,		/* 2.4 GHz only */
	clsapi_DFS,				/* 5 GHz only */
	clsapi_wmm,				/* wireless multimedia extensions */
	clsapi_mac_address_control,
	clsapi_beacon_advertise,
	clsapi_wifi_radio,
	clsapi_autorate_fallback,
	clsapi_security,
	clsapi_SSID_broadcast,
	clsapi_802_11d,
	clsapi_wireless_isolation,
	clsapi_short_GI,
	clsapi_802_11h,
	clsapi_dfs_fast_channel_switch,
	clsapi_dfs_avoid_dfs_scan,
	clsapi_uapsd,
	clsapi_tpc_query,
	clsapi_sta_dfs,
	clsapi_specific_scan,
	clsapi_GI_probing,
	clsapi_GI_fixed,
	clsapi_stbc,
	clsapi_beamforming,
	clsapi_short_slot,
	clsapi_short_preamble,
	clsapi_rts_cts,
	clsapi_40M_only,
	clsapi_obss_coexist,
	clsapi_11g_protection,
	clsapi_11n_protection,
	clsapi_qlink,
	clsapi_allow_11b,
	clsapi_dyn_beacon_period,
	clsapi_acs_obss_chk,
	clsapi_sta_dfs_strict,
	clsapi_sync_config,
	/**
	 * Enable (1) or disable (0) automatic change from 20MHz bandwidth to 40MHz
	 * after 40 MHz intolerant stations leave the BSS. Default is 1.
	 */
	clsapi_bw_resume,
	clsapi_subband_radar = 100,
	clsapi_priority_repeater,
} clsapi_option_type;

typedef enum {
	/**
	 * Invalid parameter
	 */
	clsapi_wifi_nosuch_parameter = 0,

	/**
	 * DTIM period
	 */
	clsapi_wifi_param_dtim_period = 1,

	/**
	 * CSA (Channel Switch Announcement) count
	 */
	clsapi_wifi_param_csa_count = 2,

	/**
	 * CSA (Channel Switch Announcement) mode
	 */
	clsapi_wifi_param_csa_mode = 3,

	/**
	 * RRM (Radio Resource Management) Neighbour Reports,
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_rrm_nr = 4,

	/**
	 * Enable radio transmission
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_tx_enable = 5,

	/**
	 * NSM idle timer value
	 */
	clsapi_wifi_param_nsm_timeout = 6,

	/**
	 * Timeout period for an inactive peer station, in seconds.
	 */
	clsapi_wifi_param_inact_timeout = 7,

	/**
	 * Wideband CAC configuration
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_wcac_cfg = 8,

	/**
	 * Configure use of 802.11 4-address headers.
	 * * 0 - disable
	 * * 1 - enable for data frames other than A-MSDUs
	 * * 2 - enable for all data frames
	 * * 3 - enable for all data frames. Also discard Rx frames that have a
	 *  3-address header (FROMDS set and TODS not set) if the RA/DA MAC address
	 *  does not match the VAP MAC address. This option can only be set in STA mode.
	 *
	 * In STA mode, all frames are transmitted according to the above setting.
	 *
	 * In AP mode, this mode applies only to stations from which at least one 802.11 frame
	 *  has been received with a 4-address header.
	 *
	 * 4-address headers are always used to Quantenna devices, regardless of this setting.
	 */
	clsapi_wifi_param_cfg_4addr = 9,

	/**
	 * Repeater beacon configuration
	 * 0 (disable) AP VAPs will transmit beacons even when the uplink is not established
	 * 1 (enable) AP VAPs will only transmit beacons when the uplink is established
	 */
	clsapi_wifi_param_repeater_bcn = 10,

	/**
	 * Singleton RTS threshold configuration
	 */
	clsapi_wifi_param_single_rtsthreshold = 11,

	/**
	 * Aggregate RTS threshold configuration
	 */
	clsapi_wifi_param_aggr_rtsthreshold = 12,

	/**
	 * Max number of BSSes
	 */
	clsapi_wifi_param_max_bss_num = 13,

	/**
	 * WCAC duration time for low 160M band
	 */
	clsapi_wifi_param_wcac_duration = 14,

	/**
	 * WCAC weather channel duration time for up 160M band
	 */
	clsapi_wifi_param_wcac_wea_duration = 15,

	/**
	 * Configure Multiap backhaul station mode
	 * * 0 - disable
	 * * 1 - enable
	 */
	clsapi_wifi_param_multiap_backhaul_sta = 16,

	/**
	 * IEEE802.11v Multiple BSSID support
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_multi_bssid = 17,

	/**
	 * AACS verbosity
	 * * 0 (none) - 5 (maximum)
	 */
	clsapi_wifi_param_aacs_verbose = 18,

	/**
	 * AACS exclusion table
	 */
	clsapi_wifi_param_aacs_excl_tbl = 19,

	/**
	 * AACS start bw
	 * 0, 20, 40, 80, 160 (MHz)
	 */
	clsapi_wifi_param_aacs_startbw = 20,

	/**
	 * AACS current config
	 */
	clsapi_wifi_param_aacs_curr_cfg = 21,

	/**
	 * AACS DL PHY rate average weight
	 * 0 - 100, 100 = use only DL PHY rate
	 */
	clsapi_wifi_param_aacs_avg_dl_phyrate_wgt = 22,

	/**
	 * AACS use SPF in PER metric
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_inc_spf,

	/**
	 * AACS use LPF in PER metric
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_inc_lpf,

	/**
	 * AACS use threshold table
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_use_thres_tbl,

	/**
	 * AACS normal BW control
	 * 0 - 7 (see AACS Confluence page)
	 */
	clsapi_wifi_param_aacs_norm_bwctrl,

	/**
	 * AACS normal BW limit
	 * 20, 40, 80, or 160 (MHz)
	 */
	clsapi_wifi_param_aacs_norm_bwlimit,

	/**
	 * AACS ics BW control
	 * 0 - 7 (see AACS Confluence page)
	 */
	clsapi_wifi_param_aacs_ics_bwctrl,

	/**
	 * AACS ics BW limit
	 * 20, 40, 80, or 160 (MHz)
	 */
	clsapi_wifi_param_aacs_ics_bwlimit,

	/**
	 * AACS always-on DFS BW control
	 * 0 - 7 (see AACS Confluence page)
	 */
	clsapi_wifi_param_aacs_alt_bwctrl,

	/**
	 * AACS always-on DFS BW limit
	 * 20, 40, 80, or 160 (MHz)
	 */
	clsapi_wifi_param_aacs_alt_bwlimit,

	/**
	 * AACS select fast decision mode
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_fastmode,

	/**
	 * AACS switch timebase
	 * time (in seconds)
	 */
	clsapi_wifi_param_aacs_sw_timebase_sec,

	/**
	 * AACS switch time factor
	 * factor (1 or greater)
	 */
	clsapi_wifi_param_aacs_sw_timefactor,

	/**
	 * AACS switch max time
	 * time (in seconds)
	 */
	clsapi_wifi_param_aacs_sw_max_sec,

	/**
	 * AACS switch reset time
	 * time (in seconds)
	 */
	clsapi_wifi_param_aacs_sw_reset_sec,

	/**
	 * AACS minimum scan success rate
	 * 0 - 100 (min pct of success scans)
	 */
	clsapi_wifi_param_aacs_min_scan_succrate,

	/**
	 * AACS decision max time
	 * time (in seconds)
	 */
	clsapi_wifi_param_aacs_dec_max_sec,

	/**
	 * AACS apcnt penalty
	 * 0 - TBD (sets AP count penalty)
	 */
	clsapi_wifi_param_aacs_apcnt_penalty,

	/**
	 * AACS enable virtual nodes
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_use_vnode,

	/**
	 * AACS to allow considering DFS channels
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_sel_dfs,

	/**
	 * AACS allow select DFS channel during ICS
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_ics_dfs,

	/**
	 * AACS allow select DFS channel during normal op
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_norm_dfs,

	/**
	 * AACS allow selct DFS channel in Always-on DFS
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_alt_dfs,

	/**
	 * AACS enable DFS threshold table
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_dfs_thres,

	/**
	 * AACS includes all channels
	 */
	clsapi_wifi_param_aacs_inc_all_chan,

	/**
	 * AACS includes all channels for Always-on DFS
	 */
	clsapi_wifi_param_aacs_alt_inc_all_chan,

	/**
	 * AACS enable Always-on DFS
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_aacs_enable_alt,

	/**
	 * AACS lists Always-on DFS channel list
	 * * 0x10 - Radar on full bandwidth, with channel list unsorted
	 * * 0x20 - Radar on upper 80 MHz (U80), with channel list unsorted
	 * * 0x30 - Radar on lower 80 MHz (L80), with channel list unsorted
	 * * 0x11 - Radar on full bandwidth, with channel list sorted
	 * * 0x21 - Radar on upper 80 MHz (U80), with channel list sorted
	 * * 0x31 - Radar on lower 80 MHz (L80), with channel list sorted
	 */
	clsapi_wifi_param_aacs_alt_list,

	/**
	 * AACS enables/disables STA deficiency
	 * 0 (disabled) or 1 (enabled)
	 */
	clsapi_wifi_param_aacs_enable_sta_d,

	/**
	 * AACS allowed STA deficiency
	 * 0 (any) or 100 (req'd)
	 */
	clsapi_wifi_param_aacs_allow_sta_d,

	/**
	 * AACS allowed OBSS
	 * 0 (disabled) or 1 (enabled)
	 */
	clsapi_wifi_param_aacs_enable_obss,

	/** AACS start channel
	 * 0-255, must be valid channel number for the phy mode
	 */
	clsapi_wifi_param_aacs_startchan,

	/**
	 * Max forwarded broadcast frames per second.
	 * If the number of broadcast frames received on all wired interfaces or on a vap
	 * interface reaches this number within one second, all subsequent packets will be dropped
	 * for the remainder of that second.
	 * This setting is global and applies to all vaps on all radios as well as to the wired
	 * interfaces.
	 * Valid values are from IEEE80211_PPS_MAX_BCAST_MIN to IEEE80211_PPS_MAX_BCAST_MAX.
	 * The default value is 0 (no limit).
	 */
	clsapi_wifi_param_pps_max_bcast,

	/**
	 * Max forwarded SSDP frames per second.
	 * If the number of SSDP frames received on all wired interfaces or on a vap
	 * interface reaches this number within one second, all subsequent packets will be dropped
	 * for the remainder of that second.
	 * This setting is global and applies to all vaps on all radios as well as to the wired
	 * interfaces.
	 * Valid values are from IEEE80211_PPS_MAX_SSDP_MIN to IEEE80211_PPS_MAX_SSDP_MAX.
	 * The default value is 0 (no limit).
	 */
	clsapi_wifi_param_pps_max_ssdp,

	/**
	 * Dual band virtual concurrent (DBVC)
	 * Configure DBVC level for switching channel frequency.
	 * 0 -- disable DBVC, default
	 * 1 -- switch to repeater STA channel once per beacon interval
	 * 2 -- switch to repeater STA channel twice per beacon interval
	 *
	 * Note: this parameter is not supported when SCS or NAC monitor is enabled
	 */
	clsapi_wifi_param_dbvc_level,

	/**
	 * Dual band virtual concurrent (DBVC)
	 * Configure DBVC dwell time on repeater STA channel.
	 */
	clsapi_wifi_param_dbvc_dwell,

	/**
	 * Dual band virtual concurrent (DBVC)
	 *
	 * Configure DBVC switch on same channel.
	 * * 0 - don't switch when on the same channel
	 * * 1 - switch when on the same channel (default)
	 */
	clsapi_wifi_param_dbvc_same_ch_switch,

	/**
	 * AACS channel fading seconds
	 */
	clsapi_wifi_param_aacs_chan_fade_secs,

	/**
	 * AACS client checking type bitmask
	 */
	clsapi_wifi_param_aacs_sta_check_type,
	/**
	 * Minimum rate (in units of 1 Mb/s) allowed in 6GHz band. \aponly
	 */
	clsapi_wifi_param_6g_min_rate,

	/**
	 * Capped Tx power, for setting the maximum Tx power on all channels on the radio.
	 * 0 means no cap. If capped Tx power is lower than board minimum Tx power, then
	 * board minimum Tx power is used.
	 */
	clsapi_wifi_param_tx_power_cap,

	/**
	 * Number of Tx chains.
	 * Valid arguments are 0, 1, 2 or 4.
	 * 0 means the default configuration, ie. the maximum number of Tx chains supported
	 * by the interface.
	 * 1, 2 or 4 means the number of Tx chains to be configured.
	 */
	clsapi_wifi_param_ntx_chains,

	/**
	 * RFIC internal temperure
	 * returns fixed-point scaled signed decimal value
	 * (temperature in degrees C) * 1,000,000
	 * RFICs without internal temperatures sensors will return
	 * a scaled error value (-274 * 1,000,000)
	 */
	clsapi_wifi_param_rfic_int_temp,
	/**
	 * AACS allowed non-PSC channels in 6GHz
	 * 0 (disabled) or 1 (enabled)
	 */
	clsapi_wifi_param_aacs_enable_non_psc,

	/**
	 * AACS off-channel sampling bw
	 * 20(20M), 40(40M), 80(80M) and 160(160M)
	 */
	clsapi_wifi_param_aacs_sample_bw,

	/**
	 * For the VAP interface, the operating bandwidth for the BSS.
	 * For the STA interface, the bandwidth after association.
	 */
	clsapi_wifi_param_bss_bw,
	/**
	 * Configure HE capability based on 802.11ax PHY mode.
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_hecap,

	/**
	 * Configure 802.11 TWT (Target Wait Time)
	 * 0 (disable) or 1 (enable).
	 *
	 * This feature is only supported in 802.11ax PHY mode.
	 */
	clsapi_wifi_param_twt,

	/**
	 * Initialize AACS to default parameters
	 * 1 (reset)
	 */
	clsapi_wifi_param_aacs_init,

	/**
	 * Configure port isolation.
	 * * 0 (no group)
	 * * 1 (bind the radio to the SGMII wired interface)
	 * * 2 (bind the radio to the RGMII wired interface)
	 * Not all combinations of radio and wired interfaces are supported.
	 *
	 * Port isolation must be enabled on two wireless interfaces to become operational.
	 */
	clsapi_wifi_param_port_isolate,

	/**
	 * NFR (No Filter Repeater) level for switching between backhaul and fronthaul radios.
	 */
	clsapi_wifi_param_nfr_level,

	/**
	 * NFR (No Filter Repeater) dwell time on backhaul radio in microseconds.
	 *
	 * Value is between IEEE80211_NFR_DBVC_DWELL_MIN and IEEE80211_NFR_DBVC_DWELL_MAX,
	 * and will automatically be changed to IEEE80211_NFR_DBVC_DWELL_DEF if greater
	 * than this and clsapi_wifi_param_nfr_level is set to 3.
	 *
	 * * IEEE80211_NFR_DBVC_DWELL_DEF
	 * * IEEE80211_NFR_DBVC_DWELL_MIN
	 * * IEEE80211_NFR_DBVC_DWELL_MAX
	 */
	clsapi_wifi_param_nfr_dwell,

	/**
	 * NFR (No Filter Repeater) mode
	 *
	 * Configure NFR switch on same channel
	 * * 0 -- don't switch when on the same channel
	 * * 1 -- switch when on same channel, default
	 */
	clsapi_wifi_param_nfr_same_ch_switch,

	/**
	 * Configure EAPOL VLAN tagging
	 * 0 - Do not tag EAPOL
	 * 1 - Tag EAPOL
	 */
	clsapi_wifi_param_eap_vlan_tag,

	/**
	 * MultiAP profile version - 1 (default) or 2.
	 */
	clsapi_wifi_param_multiap_backhaul_sta_profile,

	/**
	 * Configure bridge mode.
	 * * 0 -- disable bridge mode
	 * * 1 -- enable bridge mode (default)
	 */
	clsapi_wifi_param_bridge_mode,

	/**
	 * Configure the decision priority between subband and WCAC when
	 * radar detection occurs.
	 * 0 (WCAC priority) or 1 (subband priority)
	 */
	clsapi_wifi_param_sub_ov_wcac,

	/**
	 * AACS flag control.
	 *
	 * The set command toggles a bit at given position of the 32-bit
	 * AACS flag bitmap to enable or disable a feature.
	 * A value of 0 causes all bits to be cleared.
	 *
	 * The get command returns bitmap as a single integer.
	 *
	 * Flags are as follows. All flags are cleared by default.
	 * - 1 -- enable channel switching when there is no associated STA
	 * - 2 - 31 -- reserved
	 */
	clsapi_wifi_param_aacs_flag_ctrl,

	/**
	 * Configure WCAC method usage for DFS Management Daemon
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_dfs_mgmt_wcac_en,

	/**
	 * Configure ZCAC method usage for DFS Management Daemon
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_dfs_mgmt_zcac_en,

	/**
	 * Configure OCAC method usage for DFS Management Daemon
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_dfs_mgmt_ocac_en,

	/**
	 * Configure AACS mode
	 * * 0 - AACS_DISABLE:
	 *      AACS is disabled.
	 * * 1 - AACS_DISABLE_ALLOCED:
	 *      AACS performs ICS even though it's disabled.
	 * * 2 - AACS_ENABLE_BACKGROUND:
	 *      AACS performs OCS but does not perform runtime channel decision.
	 * * 3 - AACS_ENABLE_DECISION:
	 *      AACS performs both OCS and runtime channel decision.
	 */
	clsapi_wifi_param_aacs_mode,

	/**
	 * Configure Rx gain mode (2.4GHz radios only).
	 *  * 0 - Use default RX gain.
	 *  * 1 - Apply the lower RX gain.
	 */
	clsapi_wifi_param_reduce_rx_gain,

	/**
	 * From 26.17.1 of P802.11ax_D6.0, 2GHz and 5GHz beacons and probe responses do not have
	 * VHTOP-Info element in HEOP IE when VHTOP IE is present. Enable to force inclusion
	 * of this element for testing.
	 */
	clsapi_wifi_param_force_vhtopi_in_heop,

	/**
	 * Configure dynamic Rx chain control feature
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_rxchain_ctrl,

	/**
	 * Configure RSSI monitoring interval (in seconds) for Rx chain control feature
	 * The value ranges from 1 to 3600
	 * The default value is 10
	 */
	clsapi_wifi_param_rssi_mon_intvl,

	/**
	 * Channel number for SCS statistics report.
	 * This must be a valid channel number in the current operating band.
	 */
	clsapi_wifi_param_scs_stat_chan,

	/**
	 * Configure the minimum RSSI threshold for PPPC (in dBm)
	 *
	 * Default: CLSAPI_WIFI_PARAM_PPPC_RSSI_MIN_TH_MAX
	 * Range  : [CLSAPI_WIFI_PARAM_PPPC_RSSI_MIN_TH_MIN, CLSAPI_WIFI_PARAM_PPPC_RSSI_MIN_TH_MAX]
	 *
	 * PPPC will be disabled if RSSI is lower than this threshold.
	 */
	clsapi_wifi_param_pppc_rssi_min_th,

	/**
	 * Configure AACS metric calculation algorithm.
	 * * 0 - AACS_METRIC_ALGO_ONE_HOP:
	 *      one hop AACS metric calculation.
	 * * 1 - AACS_METRIC_ALGO_MULTI_HOP:
	 *      multiple hop AACS metric calculation.
	 */
	clsapi_wifi_param_aacs_metric_algo,

	/**
	 * Configure AACS report action frame interval.
	 */
	clsapi_wifi_param_aacs_report_intvl,

	/**
	 * AACS report neighbors flag.
	 * * bit 0 - AACS_NBR_RPT_F_SAME_SSID
	 *      only report neighbors whose SSID is equal to our own.
	 * * other bits reserved.
	 */
	clsapi_wifi_param_aacs_nbr_rpt_flag,

	/**
	 * Timeout seconds for AACS collected information.
	 */
	clsapi_wifi_param_aacs_timeout_sec,

	/**
	 * Country Information Element flag.
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_enable_country_ie,

	/**
	 * Enable or disable the flooding of unknown unicast frames to all Wi-Fi interfaces.
	 * 0 (disable) or 1 (enable)
	 * \note \primarywifi
	 */
	clsapi_wifi_param_flood_unknown_ucast,

	/**
	 * Enable or disable non-PSC channels in 6GHz band.
	 * 0 (disabled) or 1 (enabled)
	 */
	clsapi_wifi_param_enable_non_psc,

	/**
	 * Enable or disable BSS Load IE in Beacon frame.
	 * 0 (disable) or 1 (enable)
	 */
	clsapi_wifi_param_bss_load_ie_en,

	clsapi_wifi_param_max
} clsapi_wifi_param_type;

/**
 * \brief The operational WiFi mode of a device.
 *
 * The operational WiFi mode of a device.
 */
typedef enum {
	/** The WiFi mode has not yet been configured. */
	clsapi_mode_not_defined = 1,

	/** The device is operating as an AP. */
	clsapi_access_point,

	/** The device is operating as a STA. */
	clsapi_station,

	/** The device is operating in Wireless Distribution Mode (WDS), or bridged mode. */
	clsapi_wds,

	/**
	 * The device is operating in repeater mode. The primary interface is in STA mode and
	 * secondary interfaces are in AP mode.
	 */
	clsapi_repeater,

	/** The device is operating in ad-hoc mode (not supported). */
	clsapi_adhoc,

	/** Invalid mode. */
	clsapi_nosuch_mode = 0
} clsapi_wifi_mode;

/**
 * \brief Enumeration to represent 802.11 standards
 */
typedef enum {
	/**
	 * 802.11n
	 */
	clsapi_mimo_ht = 1,
	/**
	 * 802.11ac
	 */
	clsapi_mimo_vht,
	/**
	 * 802.11ax
	 */
	clsapi_mimo_he,
} clsapi_mimo_type;


enum cls_pktgen_type {	
	CLS_PKTGEN_TYPE_QOS_NULL = 1,	
	CLS_PKTGEN_TYPE_UPROBE_RESP,	
	CLS_PKTGEN_TYPE_FILS_DISCOVERY,	
	CLS_PKTGEN_TYPE_MGMT_DEAUTH,	
	CLS_PKTGEN_TYPE_MGMT_DISASSOC,	
	CLS_PKTGEN_TYPE_MAX
};

#define CLSAPI_MCS_RATE_MAXLEN	8

/**
 * \brief Type used to contain an MCS definition.
 *
 * CLSAPI MCS rate maximum length is distinct from MaxBitRate in TR-98.
 * TR-98 provides for 4 characters to represent the bit rate in MBPS.
 * CLSAPI MCS rate stores MCS rate specs - e.g. MCS0, MCS6, MCS76, etc.
 * Provide a bit more space for future expansion.
 * As with all CLSAPI maximum length definitions, space for the NULL ('\\0') is included.
 * So only CLSAPI_MCS_RATE_MAXLEN - 1 (7) non-NULL chars are available.
 */
typedef char	clsapi_mcs_rate[ CLSAPI_MCS_RATE_MAXLEN ];


typedef char	string_16[ 17 ];
typedef char	string_32[ 33 ];
typedef char	string_64[ 65 ];
typedef char	string_128[ 129 ];
typedef char	string_256[ 257 ];
typedef char 	string_1024[1025];

#ifndef BIT
#define BIT(x) (1L << (x))
#endif
/*
 * Bitmap bits:
 * bit 0 (0x01): WPA3-Personal (i.e., disable WPA2-Personal = WPA-PSK and only
 *       allow SAE to be used)
 * bit 1 (0x02): SAE-PK (disable SAE without use of SAE-PK)
 * bit 2 (0x04): WPA3-Enterprise (move to requiring PMF)
 * bit 3 (0x08): Enhanced Open (disable use of open network; require OWE)
 * (default: 0 = do not include Transition Disable KDE)
 **/
#define CLSAPI_WPA3_TRANSITION_DISABLE_OFF	0x0
#define CLSAPI_WPA3_TRANSITION_DISABLE_PSK	BIT(0)
#define CLSAPI_WPA3_TRANSITION_DISABLE_PK	BIT(1)
#define CLSAPI_WPA3_TRANSITION_DISABLE_ENT	BIT(2)
#define CLSAPI_WPA3_TRANSITION_DISABLE_OWE	BIT(3)
#define CLSAPI_WPA3_TRANSITION_DISABLE_ALL	(CLSAPI_WPA3_TRANSITION_DISABLE_PSK | \
	CLSAPI_WPA3_TRANSITION_DISABLE_PK | CLSAPI_WPA3_TRANSITION_DISABLE_ENT | \
	CLSAPI_WPA3_TRANSITION_DISABLE_OWE)

#define CLSAPI_SAE_PWE_LOOPING  0
#define CLSAPI_SAE_PWE_H2E  1
#define CLSAPI_SAE_PWE_LOOPING_AND_H2E  2
/**
 * \brief Structure containing parameter and value
 */
struct clsapi_data_pair_string_128bytes {
	string_64 key;
	string_64 value;
};

/**
 * \brief Structure containing the parameters for the API set_params.
 */
struct clsapi_set_parameters {
	struct clsapi_data_pair_string_128bytes param[CLSAPI_SET_PARAMS_ARG_MAX];
};
/**
 * @brief Set the security protocol to be advertised in beacons.
 *
 * Set the security protocol to be advertised in beacons.
 *
 * \note \aponly
 * \note For STA mode, use \ref clsapi_SSID_set_protocol.
 * \note \defer_mode_supported
 * \sa set_security_defer_mode
 *
 * \param ifname \wifi0
 * \param p_new_beacon A value from the <b>authentication protocols</b> table in \ref
 *	CommonSecurityDefinitions.
 *
 * \return \zero_or_negative
 *
 * \callclsapi
 *
 * <c>call_clsapi set_beacon \<WiFi interface\> \<beacon type\></c>
 *
 * \call_clsapi_string_complete
 */
extern int clsapi_wifi_set_beacon_type(const char *ifname, const char *p_new_beacon);

/**
 * \brief Set the default WEP key index for the given interface.
 *
 * Set the default WEP key index for the given interface.
 *
 * \note \aponly
 *
 * \param ifname \wifi0
 * \param key_index default WEP key index
 *
 * \return \zero_or_negative
 *
 * \callclsapi
 *
 * <c>call_clsapi set_wep_key_index \<WiFi interface\> \<index\></c>
 *
 * \call_clsapi_string_complete
 */
extern int clsapi_wifi_set_WEP_key(const char *ifname, const string_64 wepkey);

/**
 * @brief Set the security encryption types.
 *
 * Set the security encryption types.
 *
 * \note \aponly
 *
 * \param ifname \wifi0
 * \param encryption_modes A value from the <b>encryption types</b> table in
 *	\ref CommonSecurityDefinitions.
 *
 * \callclsapi
 *
 * <c>call_clsapi set_wpa_encryption_modes \<WiFi interface\> \<encryption modes\></c>
 *
 * \call_clsapi_string_complete
 */
extern int clsapi_wifi_set_WPA_encryption_modes(const char *ifname,
					const string_32 encryption_modes);

extern int clsapi_wifi_set_WPA_authentication_mode(const char *ifname, 
					const string_32 authentication_mode);

extern int clsapi_wifi_set_hs20_params(const char *ifname,
					const string_32 hs_param,
					const string_64 value1,
					const string_64 value2,
					const string_64 value3,
					const string_64 value4,
					const string_64 value5,
					const string_64 value6);
/**
 * @brief Set the WiFi passphrase (ASCII) for the given interface.
 *
 * Sets the WPA/11i ASCII passphrase. Applies to AP only. For a STA, use
 * \ref clsapi_SSID_set_key_passphrase.
 *
 * By the WPA standard, the passphrase is required to have between 8 and 63 ASCII characters.
 *
 * \note \aponly
 * \note \defer_mode_supported
 * \sa set_security_defer_mode
 *
 * \param ifname \wifi0
 * \param key_index - reserved, set to 0.
 * \param passphrase the NULL terminated passphrase string, 8 - 63 ASCII characters
 *		(NULL termination not included in the count)
 *
 * \return \zero_or_negative
 *
 * \callclsapi
 *
 * <c>call_clsapi set_passphrase \<WiFi interface\> 0 \<passphrase\></c>
 *
 * \call_clsapi_string_complete
 *
 * \sa clsapi_SSID_get_key_passphrase
 */
extern int clsapi_wifi_set_key_passphrase(const char *ifname, const string_64 passphrase);
/**
 * @brief Set the 802.11w (PMF) capability.
 *
 * Set the 802.11w (PMF) capability.
 *
 * \note \aponly
 *
 * \param ifname \wifi0
 * \param pmf_cap PMF capability
 *
 * \return \zero_or_negative
 *
 * The following table shows the most common configuration combinations based on the
 * PMF (protected management frames) certification program. Other combinations may cause
 * association with some stations to fail.
 *
 * <TABLE>
 * <TR> <TD>PMF</TD>	<TD>WPA Key Management</TD> </TR>
 * <TR> <TD>disabled</TD>
 *	<TD>WPA-PSK for WPA2-Personal and WPA-EAP for WPA2-Enterprise</TD> </TR>
 * <TR> <TD>optional</TD>
 *	<TD>WPA-PSK for WPA2-Personal and WPA-EAP for WPA2-Enterprise</TD> </TR>
 * <TR> <TD>required</TD>
 *	<TD>WPA-PSK-SHA256 for WPA2-Personal and WPA-EAP-SHA256 for WPA2-Enterprise</TD> </TR>
 * </TABLE>
 *
 *\note WPA3-Personal-only mode: PMF required and wpa_key_mgmt=SAE.
 *
 * \callclsapi
 *
 * <c>call_clsapi set_pmf \<WiFi interface\> \<pmf_cap\></c>
 *
 * <c>call_clsapi set_pmf \<WiFi interface\> 0</c>
 *
 * \call_clsapi_string_complete
 */
extern int clsapi_wifi_set_pmf(const char *ifname, int pmf_cap);

/**
 * @brief Set parameters.
 *
 * Set values for multiple parameters
 *
 * \param ifname \wifi0
 * \param SSID_substr the ssid of network block in station mode or NULL in AP mode
 * \param set_params pointer to the structure \link clsapi_set_parameters \endlink.
 *
 * \return \zero_or_negative
 *
 * \callclsapi
 *
 * <c>call_clsapi set_params \<WiFi interface\> \<param1\> \<value1\>
 *	[\<param2> <value2\>]... [\<param8\> \<value8\>]</c>
 *
 * <c>call_clsapi ssid_set_params \<WiFi interface\> \<ssid\> \<param1\>
 *	\<val1\> [\<param2> <val2\>]... [\<param8\> \<val8\>]</c>
 *
 * Parameter names and corresponding enum values are shown in \ref clsapi_param.
 *
 * \call_clsapi_string_complete
 */
extern int clsapi_set_params(const char *ifname,
		const clsapi_SSID SSID_substr,
		const struct clsapi_set_parameters *set_params);

/**
 * @brief Get various type of channel list supported on an interface.
 *
 * Get various type of channel lists supported in all frequency bands on an interface
 *
 * \param ifname \wifi0only
 * \param chan_list \valuebuf
 * \param flags represents the type of channel list. See \ref clsapi_chlist_flags.
 *
 * \return \zero_or_negative
 *
 * \callclsapi
 *
 * <c>call_clsapi get_chan_list \<WiFi interface\>
 *	[ {available | disabled | scan | active | ocac_off} ]</c>
 *
 * \note If the API is called without any argument, output will be the 'available' channel list.
 *
 * Unless an error occurs, the output will be the list of WiFi channels per the 802.11 standard.
 *
 * If the API returns success, the array 'chan_list' will contain a bitmap of 1024 (256 x 8) bits.
 * Each bit in this array represents a channel.  The first 256 bits (ie. bytes 0 to 31) represents
 * channel numbers in 2.4 GHz and 5GHz band.  The next 256 bits (ie. bytes 32 to 63) represents
 * channel numbers in 6GHz. Eg:
 * - MSB of byte 0 represents channel 0 (in 2.4GHz or 5GHz band)
 * - MSB of byte 1 represents channel 8 (in 2.4GHz or 5GHz band)
 * - MSB of byte 32 represents channel 0 (in 6GHz band)
 * - MSB of byte 33 represents channel 8 (in 6GHz band)
 */
extern int clsapi_wifi_get_chan_list(const int band_idx, struct clsapi_data_256bytes *chan_list,
		uint8_t *chan_num, const uint32_t flags);

extern int clsapi_interface_get_mac_addr(const char *ifname, clsapi_mac_addr device_mac_addr);

extern int clsapi_wifi_get_ieee80211r_mobility_domain(const char *ifname, string_16 p_value);

extern int clsapi_wifi_wait_scan_completes(const char *ifname, time_t timeout);

extern int clsapi_wifi_get_chan(const char *ifname,
			clsapi_unsigned_int *p_current_channel,
			clsapi_unsigned_int *p_current_bw,
			clsapi_unsigned_int *p_current_band);

extern int clsapi_wifi_set_chan(const char *ifname,
			const clsapi_unsigned_int new_channel,
			const clsapi_unsigned_int new_bw,
			const clsapi_unsigned_int new_band,
			const char *phy_mode);

extern int clsapi_radio_get_primary_interface(const clsapi_unsigned_int radio_id,
						char *ifname, size_t maxlen);

extern int clsapi_get_radio_from_ifname(const char *ifname, unsigned int *radio_id);

extern int clsapi_config_update_parameter(const char *ifname, const char *param_name,
						const char *param_value);

extern int clsapi_wifi_add_radius_auth_server_cfg(const char *ifname, char *radius_ip, 
						char *radius_port, char *radius_password);

extern int clsapi_get_radio_from_ifname(const char *ifname, unsigned int *radio_id);

extern int clsapi_interface_get_status(const char *ifname, char *interface_status);

extern int clsapi_wifi_create_bss(const char *ifname, unsigned int radio_id,
	const clsapi_mac_addr mac_addr);

extern int clsapi_pm_set_mode(int mode);

extern int clsapi_wifi_set_SSID(const char *ifname, const clsapi_SSID SSID_str);

extern int clsapi_wifi_get_regulatory_region(const char *ifname, char *region_by_name);

extern int clsapi_wifi_set_option(const char *ifname, clsapi_option_type clsapi_option, int new_option);

extern int clsapi_wifi_set_rts_threshold(const char  *ifname, clsapi_unsigned_int rts_threshold);

extern int clsapi_wifi_set_beacon_interval(const char * ifname, clsapi_unsigned_int new_intval);

extern int clsapi_wifi_set_dtim(const char * ifname, const clsapi_unsigned_int new_dtim);

extern int clsapi_wifi_get_option(const char *ifname, clsapi_option_type clsapi_option, int *p_current_option);

extern int clsapi_wifi_set_rxba_decline(const char *ifname, clsapi_unsigned_int value);

extern int clsapi_wifi_set_txba_disable(const char *ifname, int disable);

extern int clsapi_wifi_get_channel(const char *ifname, clsapi_unsigned_int *p_current_channel);

extern int clsapi_wifi_set_sec_chan(const char *ifname, int chan, int offset);

extern int clsapi_wifi_set_ieee80211r(const char *ifname, char *value);

extern int clsapi_wifi_set_ieee80211r_ft_over_ds(const char *ifname, char *value);

extern int clsapi_wifi_set_ieee80211r_mobility_domain(const char *ifname, char *value);

extern int clsapi_wifi_set_ieee80211r_nas_id(const char *ifname, char *value);

extern int clsapi_wifi_set_11r_r1_key_holder(const char *ifname, char *value);

extern int clsapi_wifi_add_11r_neighbour(const char * ifname, char * mac, char * nas_id ,char * key, char * r1kh_id);

extern int clsapi_wifi_set_ac_agg_hold_time(const char * ifname, uint32_t ac, uint32_t agg_hold_time);

extern int clsapi_wifi_get_phy_mode(const char *ifname, char *p_wifi_phy_mode);

extern int clsapi_wifi_get_bw(const char* ifname, clsapi_unsigned_int *p_bw);

extern int clsapi_wifi_set_bw(const char *ifname, const clsapi_unsigned_int bw, const char *phy_mode);

extern int clsapi_wifi_set_phy_mode(const char *ifname, const char *new_phy_mode);

extern int clsapi_wifi_get_mode(const char *ifname, clsapi_wifi_mode *p_wifi_mode);

extern int clsapi_wifi_set_nss_cap(const char *ifname, const clsapi_mimo_type modulation, const clsapi_unsigned_int nss);

extern int clsapi_wifi_set_tx_amsdu(const char *ifname, int enable);

extern int clsapi_wifi_set_mcs_rate(const char *ifname, const clsapi_mcs_rate new_mcs_rate);

extern int clsapi_wifi_get_parameter(const char *ifname, clsapi_wifi_param_type type, int *p_value);

extern int clsapi_radio_get_board_parameter(const clsapi_unsigned_int radio_id,
		clsapi_board_parameter_type board_param, string_64 p_buffer);

extern int clsapi_wifi_get_associated_device_mac_addr(const char *ifname,
						const clsapi_unsigned_int device_index,
						clsapi_mac_addr device_mac_addr);

extern int clsapi_wifi_qos_set_param(const char *ifname, int bss,
		int the_queue, int the_param, int acm_flag, int value);

extern int clsapi_wifi_get_count_associations(const char *ifname,
					clsapi_unsigned_int *p_association_count);

extern int clsapi_wifi_get_cac_status(const char *ifname, int *cacstatus);

extern int clsapi_wifi_get_radius_auth_server_cfg(const char *ifname, string_1024 radius_auth_server_cfg);

extern int clsapi_wifi_del_radius_auth_server_cfg(const char *ifname, const char *radius_auth_server_ipaddr,
		const char *constp_radius_port);

extern int clsapi_remove_app_ie(const char *ifname, const clsapi_unsigned_int frametype, const clsapi_unsigned_int index);

extern int clsapi_wifi_rfstatus(clsapi_unsigned_int *rf_status);

extern int clsapi_radio_rfstatus(const char *ifname, clsapi_unsigned_int *rfstatus);

extern int clsapi_wifi_set_vht(const char *ifname, const clsapi_unsigned_int the_vht);

extern int clsapi_wifi_scs_enable(const char *ifname, uint16_t enable_val);

extern int clsapi_wifi_set_channel(const char * ifname, const clsapi_unsigned_int new_channel);

extern int clsapi_wifi_set_interworking(const char *ifname, const string_32 interworking);

extern int clsapi_wifi_set_80211u_params(const char *ifname, const string_32 param, 
		const string_256 value1, const string_32 valuse2);

extern int clsapi_wifi_set_hs20_status(const char *ifname, const string_32 hs20_val);

extern int clsapi_security_add_oper_friendly_name(const char *ifname, const char *lang_code, 
		const char *oper_friendly_name);

extern int clsapi_security_add_roaming_consortium(const char *ifname, const char *p_value);

extern int clsapi_wifi_disable_wps(const char *ifname, int disable_wps);

extern int clsapi_wifi_remove_bss(const char * ifname);

extern int clsapi_wifi_get_rx_chains(const char *ifname, clsapi_unsigned_int *p_rx_chains);

extern int clsapi_regulatory_set_regulatory_region(const char * ifname, const char * region_by_name);

extern int clsapi_interface_enable(const char *ifname, const int enable_flag);

extern int clsapi_radio_verify_repeater_mode(const clsapi_unsigned_int radio_id);

extern int clsapi_wifi_get_ssid(const char *ifname, char *ssid);

extern int clsapi_wifi_get_bssid(const char *ifname, char *bssid);

#endif
