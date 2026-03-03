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

#ifndef _CLS_WIFI_DEFS_H_
#define _CLS_WIFI_DEFS_H_

#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/skbuff.h>
#include <net/cfg80211.h>
#include <linux/slab.h>
#ifdef CONFIG_CLS_FWT
#include "../net/bridge/br_fwt.h"
#endif
#include "cls_wifi_compat.h"
#include "cls_wifi_mod_params.h"
#include "cls_wifi_debugfs.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_rx.h"
#include "cls_wifi_radar.h"
#include "cls_wifi_utils.h"
#include "cls_wifi_he_mu.h"
#include "cls_wifi_platform.h"
#include "cls_wifi_cmds.h"
#include "ipc_host.h"
#include "cls_wifi_cali.h"
#include "cls_wifi_dif_sm.h"
#include "cls_wifi_csi.h"
#include "cls_wifi_atf.h"
#include "cls_wifi_bfmer.h"
#include "cls_wifi_tm.h"
#include "clsm_wifi_dfx.h"

#ifdef CONFIG_CLS_3ADDR_BR
#include "cls_3addr_bridge.h"
#endif

#define WPI_HDR_LEN	18
#define WPI_PN_LEN	 16
#define WPI_PN_OFST	 2
#define WPI_MIC_LEN	16
#define WPI_KEY_LEN	32
#define WPI_SUBKEY_LEN 16 // WPI key is actually two 16bytes key

#define LEGACY_PS_ID   0
#define UAPSD_ID	   1

#define PS_SP_INTERRUPTED  255

// Maximum number of interface at driver level
// At max we can have one AP_VLAN per station, but we also limit the
// maximum number of interface to 64 (to fit in avail_idx_map)
#define CLS_ITF_MAX 64

// Maximum number of AP_VLAN interfaces allowed.
/* #define MAX_AP_VLAN_ITF (((CLS_ITF_MAX - CLS_VIRT_DEV_MAX) > CLS_REMOTE_STA_MAX) ? \
 * 	CLS_REMOTE_STA_MAX : (CLS_ITF_MAX - CLS_VIRT_DEV_MAX))
 */

// Maximum number of AP_VLAN interfaces bitmap array length.
#define ITF_BITMAP_ARRAY_LEN ((CLS_ITF_MAX % 16) ? (CLS_ITF_MAX / 16 + 1) : \
	(CLS_ITF_MAX / 16))

#define AMSDU_PADDING(x) ((4 - ((x) & 0x3)) & 0x3)

#define CLS_WIFI_PPS_CAL_INTERVAL (HZ) //1s in jiffies

/**
 * struct cls_wifi_bcn - Information of the beacon in used (AP mode)
 *
 * @head: head portion of beacon (before TIM IE)
 * @tail: tail portion of beacon (after TIM IE)
 * @ies: extra IEs (not used ?)
 * @head_len: length of head data
 * @tail_len: length of tail data
 * @ies_len: length of extra IEs data
 * @tim_len: length of TIM IE
 * @len: Total beacon len (head + tim + tail + extra)
 * @dtim: dtim period
 */
struct cls_wifi_bcn {
	u8 *head;
	u8 *tail;
	u8 *ies;
	size_t head_len;
	size_t tail_len;
	size_t ies_len;
	size_t tim_len;
	size_t len;
	u8 dtim;
};

/**
 * struct cls_wifi_key - Key information
 *
 * @hw_idx: Index of the key from hardware point of view
 */
struct cls_wifi_key {
	u16 hw_idx;
};

/**
 * struct cls_wifi_mesh_path - Mesh Path information
 *
 * @list: List element of cls_wifi_vif.mesh_paths
 * @path_idx: Path index
 * @tgt_mac_addr: MAC Address it the path target
 * @nhop_sta: Next Hop STA in the path
 */
struct cls_wifi_mesh_path {
	struct list_head list;
	u8 path_idx;
	struct mac_addr tgt_mac_addr;
	struct cls_wifi_sta *nhop_sta;
};

/**
 * struct cls_wifi_mesh_path - Mesh Proxy information
 *
 * @list: List element of cls_wifi_vif.mesh_proxy
 * @ext_sta_addr: Address of the External STA
 * @proxy_addr: Proxy MAC Address
 * @local: Indicate if interface is a proxy for the device
 */
struct cls_wifi_mesh_proxy {
	struct list_head list;
	struct mac_addr ext_sta_addr;
	struct mac_addr proxy_addr;
	bool local;
};

/**
 * struct cls_wifi_csa - Information for CSA (Channel Switch Announcement)
 *
 * @vif: Pointer to the vif doing the CSA
 * @bcn: Beacon to use after CSA
 * @buf: IPC buffer to send the new beacon to the fw
 * @chandef: defines the channel to use after the switch
 * @count: Current csa counter
 * @status: Status of the CSA at fw level
 * @ch_idx: Index of the new channel context
 * @work: work scheduled at the end of CSA
 */
struct cls_wifi_csa {
	struct cls_wifi_vif *vif;
	struct cls_wifi_bcn bcn;
	struct cls_wifi_ipc_buf buf;
	struct cfg80211_chan_def chandef;
	int count;
	int status;
	int ch_idx;
	struct work_struct work;
};

/**
 * struct cls_wifi_cca - Information for CCA (BSS Color Change Announcement)
 *
 * @vif: Pointer to the vif doing the CCA
 * @bcn: Beacon to use after CCA
 * @buf: IPC buffer to send the new beacon to the fw
 * @count: Current cca counter
 * @color: Current bss color
 * @status: Status of the CCA at fw level
 * @work: work scheduled at the end of CCA
 */
struct cls_wifi_cca {
	struct cls_wifi_vif *vif;
	struct cls_wifi_bcn bcn;
	struct cls_wifi_ipc_buf buf;
	int count;
	int color;
	int status;
	struct work_struct work;
};

/**
 * enum tdls_status_tag - States of the TDLS link
 *
 * @TDLS_LINK_IDLE: TDLS link is not active (no TDLS peer connected)
 * @TDLS_SETUP_REQ_TX: TDLS Setup Request transmitted
 * @TDLS_SETUP_RSP_TX: TDLS Setup Response transmitted
 * @TDLS_LINK_ACTIVE: TDLS link is active (TDLS peer connected)
 */
enum tdls_status_tag {
		TDLS_LINK_IDLE,
		TDLS_SETUP_REQ_TX,
		TDLS_SETUP_RSP_TX,
		TDLS_LINK_ACTIVE,
		TDLS_STATE_MAX
};

/**
 * struct cls_wifi_tdls - Information relative to the TDLS peer
 *
 * @active: Whether TDLS link is active or not
 * @initiator: Whether TDLS peer is the TDLS initiator or not
 * @chsw_en: Whether channel switch is enabled or not
 * @chsw_allowed: Whether TDLS channel switch is allowed or not
 * @last_tid: TID of the latest MPDU transmitted over the TDLS link
 * @last_sn: Sequence number of the latest MPDU transmitted over the TDLS link
 * @ps_on: Whether power save mode is enabled on the TDLS peer or not
 */
struct cls_wifi_tdls {
	bool active;
	bool initiator;
	bool chsw_en;
	u8 last_tid;
	u16 last_sn;
	bool ps_on;
	bool chsw_allowed;
};

/**
 * enum cls_wifi_ap_flags - AP flags
 *
 * @CLS_WIFI_AP_ISOLATE: Isolate clients (i.e. Don't bridge packets transmitted by
 * one client to another one
 * @CLS_WIFI_AP_USER_MESH_PM: Mesh Peering Management is done by user space
 * @CLS_WIFI_AP_CREATE_MESH_PATH: A Mesh path is currently being created at fw level
 */
enum cls_wifi_ap_flags {
	CLS_WIFI_AP_ISOLATE = BIT(0),
	CLS_WIFI_AP_USER_MESH_PM = BIT(1),
	CLS_WIFI_AP_CREATE_MESH_PATH = BIT(2),
};

/**
 * enum cls_wifi_sta_flags - STATION flags
 *
 * @CLS_WIFI_STA_EXT_AUTH: External authentication is in progress
 */
enum cls_wifi_sta_flags {
	CLS_WIFI_STA_EXT_AUTH = BIT(0),
	CLS_WIFI_STA_FT_OVER_DS = BIT(1),
	CLS_WIFI_STA_FT_OVER_AIR = BIT(2),
};

/**
 * struct cls_wifi_vif - VIF information
 *
 * @list: List element for cls_wifi_hw->vifs
 * @cls_wifi_hw: Pointer to driver main data
 * @wdev: Wireless device
 * @ndev: Pointer to the associated net device
 * @net_stats: Stats structure for the net device
 * @key: Conversion table between protocol key index and MACHW key index
 * @drv_vif_index: VIF index at driver level (only use to identify active
 * vifs in cls_wifi_hw->avail_idx_map)
 * @vif_index: VIF index at fw level (used to index cls_wifi_hw->vif_table, and
 * cls_wifi_sta->vif_idx)
 * @ch_index: Channel context index (within cls_wifi_hw->chanctx_table)
 * @up: Indicate if associated netdev is up (i.e. Interface is created at fw level)
 * @use_4addr: Whether 4address mode should be use or not
 * @is_resending: Whether a frame is being resent on this interface
 * @roc_tdls: Indicate if the ROC has been called by a TDLS station
 * @tdls_status: Status of the TDLS link
 * @tdls_chsw_prohibited: Whether TDLS Channel Switch is prohibited or not
 * @generation: Generation ID. Increased each time a sta is added/removed
 *
 * STA / P2P_CLIENT interfaces
 * @flags: see cls_wifi_sta_flags
 * @ap: Pointer to the peer STA entry allocated for the AP
 * @tdls_sta: Pointer to the TDLS station
 * @ft_assoc_ies: Association Request Elements (only allocated for FT connection)
 * @ft_assoc_ies_len: Size, in bytes, of the Association request elements.
 * @ft_target_ap: Target AP for a BSS transition for FT over DS
 *
 * AP/P2P GO/ MESH POINT interfaces
 * @flags: see cls_wifi_ap_flags
 * @sta_list: List of station connected to the interface
 * @bcn: Beacon data
 * @bcn_interval: beacon interval in TU
 * @bcmc_index: Index of the BroadCast/MultiCast station
 * @csa: Information about current Channel Switch Announcement (NULL if no CSA)
 * @mpath_list: List of Mesh Paths (MESH Point only)
 * @proxy_list: List of Proxies Information (MESH Point only)
 * @mesh_pm: Mesh power save mode currently set in firmware
 * @next_mesh_pm: Mesh power save mode for next peer
 *
 * AP_VLAN interfaces
 * @mater: Pointer to the master interface
 * @sta_4a: When AP_VLAN interface are used for WDS (i.e. wireless connection
 * between several APs) this is the 'gateway' sta to 'master' AP
 */
struct cls_wifi_vif {
	struct list_head list;
	struct cls_wifi_hw *cls_wifi_hw;
	struct wireless_dev wdev;
	struct net_device *ndev;
	struct net_device *ndev_fp;
	bool fp_bypass_qdisc;
	struct net_device_stats net_stats;
	struct _clsm_vap_extstats dfx_stats;
#ifdef CONFIG_WIFI_TRAFFIC_MONITOR
	struct wtm_stats wtm_stats;
#endif
	struct cls_wifi_key key[6];
	u8 drv_vif_index;
	u8 vif_index;
	u8 ch_index;
	bool up;
	bool going_stop;
	// solid 4addr mode, for sta and legacy 4addr ap
	bool use_4addr;
	// auto 4addr, support 3addr and 4addr at the same time, for ap
	bool auto_4addr;
	// force to use 4addr to tx for is_4addr stas, for ap
	bool force_4addr;
#ifdef CONFIG_CLS_3ADDR_BR
	bool use_3addr_br;
#endif
	// m2u resend 3 addr packets, for ap
	bool m2u_3addr_resend;
	// log enable
	bool log_enable;
	// dump packet
	bool dump_pkt;
	bool is_resending;
	bool roc_tdls;
	u8 tdls_status;
	bool tdls_chsw_prohibited;
	int generation;
	union
	{
		struct
		{
			u32 flags;
			struct cls_wifi_sta *ap;
			struct cls_wifi_sta *tdls_sta;
			u8 *ft_assoc_ies;
			int ft_assoc_ies_len;
			u8 ft_target_ap[ETH_ALEN];
		} sta;
		struct
		{
			u32 flags;
			struct list_head sta_list;
			struct cls_wifi_bcn bcn;
			int bcn_interval;
			u8 bcmc_index;
			struct cls_wifi_csa *csa;
			struct cls_wifi_cca *cca;

			struct list_head mpath_list;
			struct list_head proxy_list;
			enum nl80211_mesh_power_mode mesh_pm;
			enum nl80211_mesh_power_mode next_mesh_pm;
			bool ap_started;
		} ap;
		struct
		{
			struct cls_wifi_vif *master;
			struct cls_wifi_sta *sta_4a;
		} ap_vlan;
	};
	uint32_t max_assoc_sta;
	uint8_t assoc_sta_count;

	bool txwq_vif_allow;
	struct work_struct tx_work;
	struct sk_buff_head tx_skb_queue;

	struct atf_per_bss_params atf;
#ifdef WIFI_DRV_TXWQ_PERF
	bool tx_wq_start;
	struct completion complete;
#endif
#ifdef CONFIG_CLS_3ADDR_BR
//	uint16_t br_isolate;
//	uint16_t br_isolate_vid;

	struct net_device *br_dev;
	struct cls_wifi_br bridge_table;
	struct ip_mac_mapping *ic_ip_mac_mapping;
	u_int8_t ni_brcm_flags;                 /* Broadcom feature flags */
//	uint32_t ic_vendor_fix;
#endif
};

#define CLS_WIFI_INVALID_VIF 0xF
#define CLS_WIFI_VIF_TYPE(vif) (vif->wdev.iftype)

/**
 * Structure used to store information relative to PS mode.
 *
 * @active: True when the sta is in PS mode.
 *		  If false, other values should be ignored
 * @pkt_ready: Number of packets buffered for the sta in drv's txq
 *			 (1 counter for Legacy PS and 1 for U-APSD)
 * @sp_cnt: Number of packets that remain to be pushed in the service period.
 *		  0 means that no service period is in progress
 *		  (1 counter for Legacy PS and 1 for U-APSD)
 */
struct cls_wifi_sta_ps {
	bool active;
	u16 pkt_ready[2];
	u16 sp_cnt[2];
};

/**
 * struct cls_wifi_rx_rate_stats - Store statistics for RX rates
 *
 * @table: Table indicating how many frame has been receive which each
 * rate index. Rate index is the same as the one used by RC algo for TX
 * @size: Size of the table array
 * @cpt: number of frames received
 * @rate_cnt: number of rate for which at least one frame has been received
 */
struct cls_wifi_rate_stats {
	int *table;
	int size;
	int cpt;
	int rate_cnt;
};

/**
 * struct cls_wifi_sta_stats - Structure Used to store statistics specific to a STA
 *
 * @rx_pkts: Number of MSDUs and MMPDUs received from this STA
 * @tx_pkts: Number of MSDUs and MMPDUs sent to this STA
 * @rx_bytes: Total received bytes (MPDU length) from this STA
 * @tx_bytes: Total transmitted bytes (MPDU length) to this STA
 * @last_act: Timestamp (jiffies) when the station was last active (i.e. sent a
 frame: data, mgmt or ctrl )
 * @last_rx: Hardware vector of the last received frame
 * @last_tx: Hardware rate config of the last tx confirmed frame
 * @rx_rate: Statistics of the received rates
 * @tx_rate: Statistics of the transmitted rates
 */
struct cls_wifi_sta_stats {
	u32 rx_pkts;
	u32 tx_pkts;
	u64 rx_bytes;
	u64 tx_bytes;
	unsigned long last_act;
	struct hw_vect last_rx;
	struct rx_vector_2 last_hw_rx_vect2;
	bool hw_rx_vect2_valid;
	int sinr_total[16][2];
	int sinr_cnt;
	u32 last_tx;
	u32 tx_mgmt_pkts;
	u32 tx_pkts_old;
	u32 tx_pps;
#ifdef CONFIG_CLS_WIFI_DEBUGFS
	struct cls_wifi_rate_stats rx_rate;
	struct cls_wifi_rate_stats tx_rate;
#endif
};

/**
 * struct cls_wifi_sta - Managed STATION information
 *
 * @list: List element of cls_wifi_vif->ap.sta_list
 * @valid: Flag indicating if the entry is valid
 * @mac_addr:  MAC address of the station
 * @aid: association ID
 * @sta_idx: Firmware identifier of the station
 * @vif_idx: Firmware of the VIF the station belongs to
 * @vlan_idx: Identifier of the VLAN VIF the station belongs to (= vif_idx if
 * no vlan in used)
 * @band: Band (only used by TDLS, can be replaced by channel context)
 * @width: Channel width
 * @center_freq: Center frequency
 * @center_freq1: Center frequency 1
 * @center_freq2: Center frequency 2
 * @ch_idx: Identifier of the channel context linked to the station
 * @qos: Flag indicating if the station supports QoS
 * @acm: Bitfield indicating which queues have AC mandatory
 * @uapsd_tids: Bitfield indicating which tids are subject to UAPSD
 * @key: Information on the pairwise key install for this station
 * @ps: Information when STA is in PS (AP only)
 * @bfm_report: Beamforming report to be used for VHT TX Beamforming
 * @group_info: MU grouping information for the STA
 * @ht: Flag indicating if the station supports HT
 * @vht: Flag indicating if the station supports VHT
 * @he: Flag indicating if the station supports HE
 * @ac_param[AC_MAX]: EDCA parameters
 * @tdls: TDLS station information
 * @stats: Station statistics
 * @mesh_pm: link-specific mesh power save mode
 * @listen_interval: listen interval (only for AP client)
 * @twt_ind: TWT Setup indication
 * @he_mu: Node for HE MU releated list
 */
struct cls_wifi_sta {
	struct list_head list;
	bool valid;
	u8 mac_addr[ETH_ALEN];
	u16 aid;
	u16 sta_idx;
	u8 vif_idx;
	u8 vlan_idx;
	enum nl80211_band band;
	enum nl80211_chan_width width;
	u16 center_freq;
	u32 center_freq1;
	u32 center_freq2;
	u8 ch_idx;
	bool qos;
	u8 acm;
	u16 uapsd_tids;
	struct cls_wifi_key key;
	struct cls_wifi_sta_ps ps;
#ifdef CONFIG_CLS_WIFI_BFMER
	struct cls_wifi_bfmer_report *bfm_report;
#endif /* CONFIG_CLS_WIFI_BFMER */
	bool ht;
	bool vht;
	bool he;
	u32 ac_param[AC_MAX];
	struct cls_wifi_tdls tdls;
	struct cls_wifi_sta_stats stats;
	struct _clsm_persta_extstats dfx_stats;
#ifdef CONFIG_WIFI_TRAFFIC_MONITOR
	struct wtm_stats wtm_stats;
#endif
	enum nl80211_mesh_power_mode mesh_pm;
	int listen_interval;
	struct twt_setup_ind twt_ind;
	struct list_head he_mu;
	struct key_params key_info;
	bool is_4addr;
	unsigned long last_data_frame_rx;
	u32 sta_flags_set;
	struct atf_per_sta_params atf;

	struct work_struct sta_snr_work;
	struct cls_sta_snr_info *snr_list;
	struct cls_sta_snr_info *snr_list_last;
	struct cls_sta_snr_info *free_list;
	u16  free_cnt;
	struct file *snrlog;
	int snr_format_type;
	spinlock_t snr_lock;
	struct file *csilog;
	unsigned long jiff_authorized;
};

#define CLS_WIFI_INVALID_STA 0x3FF

/**
 * cls_wifi_sta_addr - Return MAC address of a STA
 *
 * @sta: Station whose address is returned
 */
static inline const u8 *cls_wifi_sta_addr(struct cls_wifi_sta *sta) {
	return sta->mac_addr;
}

/**
 * cls_wifi_sta_is_he - Return if a station is HE capable
 *
 * @sta: Station to test
 * @return true is station associate with HE capabilities element
 * and false otherwise.
 * For now only works for stations connected to an AP interface.
 */
static inline bool cls_wifi_sta_is_he(struct cls_wifi_sta *sta) {
	return sta->he;
}

#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
/**
 * struct cls_wifi_amsdu_stats - A-MSDU statistics
 *
 * @done: Number of A-MSDU push the firmware
 * @failed: Number of A-MSDU that failed to transit
 */
struct cls_wifi_amsdu_stats {
	int done;
	int failed;
};
#endif

/**
 * struct cls_wifi_stats - Global statistics
 *
 * @cfm_balance: Number of buffer currently pushed to firmware per HW queue
 * @ampdus_tx: Number of A-MPDU transmitted (indexed by A-MPDU length)
 * @ampdus_rx: Number of A-MPDU received (indexed by A-MPDU length)
 * @ampdus_rx_map: Internal variable to distinguish A-MPDU
 * @ampdus_rx_miss: Number of MPDU non missing while receiving a-MPDU
 * @ampdu_rx_last: Index (of ampdus_rx_map) of the last A-MPDU received.
 * @amsdus: statistics of a-MSDU transmitted
 * @amsdus_rx: Number of A-MSDU received (indexed by A-MSDU length)
 */
#define RXMAP_MAX    4
struct cls_wifi_stats {
	int cfm_balance[CLS_TXQ_CNT];
	int ampdus_tx[IEEE80211_MAX_AMPDU_BUF];
	int ampdus_rx[IEEE80211_MAX_AMPDU_BUF];
	int ampdus_rx_map[RXMAP_MAX];
	int ampdus_rx_miss;
	int ampdus_rx_last;
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
	struct cls_wifi_amsdu_stats amsdus[CLS_TX_PAYLOAD_MAX];
#endif
	int amsdus_rx[64];
	unsigned int rx_msdu_truncated;
	unsigned int rx_mmpdu_truncated;
};

struct cls_ipc_stats {
	int ipc_tx_push;
	int ipc_tx_cfm;
};


// maximum number of TX frame per RoC
#define CLS_ROC_TX 5
/**
 * struct cls_wifi_roc - Remain On Channel information
 *
 * @vif: VIF for which RoC is requested
 * @chan: Channel to remain on
 * @duration: Duration in ms
 * @internal: Whether RoC has been started internally by the driver (e.g. to send
 * mgmt frame) or requested by user space
 * @on_chan: Indicate if we have switch on the RoC channel
 * @tx_cnt: Number of MGMT frame sent using this RoC
 * @tx_cookie: Cookie as returned by cls_wifi_cfg80211_mgmt_tx if Roc has been started
 * to send mgmt frame (valid only if internal is true)
 */
struct cls_wifi_roc {
	struct cls_wifi_vif *vif;
	struct ieee80211_channel *chan;
	unsigned int duration;
	bool internal;
	bool on_chan;
	int tx_cnt;
	u64 tx_cookie[CLS_ROC_TX];
};

/**
 * struct cls_wifi_survey_info - Channel Survey Information
 *
 * @filled: filled bitfield as per struct survey_info
 * @chan_time_ms: Amount of time in ms the radio spent on the channel
 * @chan_time_busy_ms: Amount of time in ms the primary channel was sensed busy
 * @noise_dbm: Noise in dbm
 */
struct cls_wifi_survey_info {
	u32 filled;
	u32 chan_time_ms;
	u32 chan_time_busy_ms;
	s8 noise_dbm;
};

struct cls_wifi_scan_ext_info {
	u32 ext_enabled;
	u32 rx_filter;
	u32 work_duration;
	u32 scan_interval;
};

/**
 * cls_wifi_chanctx - Channel context information
 *
 * @chan_def: Channel description
 * @count: number of vif using this channel context
 */
struct cls_wifi_chanctx {
	struct cfg80211_chan_def chan_def;
	u8 count;
};

#define CLS_WIFI_CH_NOT_SET 0xFF

/**
 * cls_wifi_phy_info - Phy information
 *
 * @cnt: Number of phy interface
 * @cfg: Configuration send to firmware
 * @sec_chan: Channel configuration of the second phy interface (if phy_cnt > 1)
 * @limit_bw: Set to true to limit BW on requested channel. Only set to use
 * VHT with old radio that don't support 80MHz (deprecated)
 */
struct cls_wifi_phy_info {
	u8 cnt;
	struct phy_cfg_tag cfg;
	struct mac_chan_op sec_chan;
	bool limit_bw;
};

#define CLS_DPD_WMAC_TX_MPDU_NUM 48
#define CLS_DPD_WMAC_TX_MAX_MPDU_LEN 11454
struct cls_wifi_dpd_wmac_tx_params {
	struct mm_dpd_wmac_tx_params_req req;
	void *mpdu_host_addr[CLS_DPD_WMAC_TX_MPDU_NUM];
	uint8_t mpdu_payload[CLS_DPD_WMAC_TX_MAX_MPDU_LEN];
};

struct cls_wifi_irf_file {
	struct irf_smp_start_ind irf_smp_ind;
	u32 rx_dcoc_size;
	u32 rx_dcoc_high_temp;
	u32 rx_dcoc_offset;
	u32 dif_eq_size;
	u32 tx_data_size;
	u32 fb_data_size;
	u32 rx_data_gain_lvl_size;
	u32 rx_data_freq_comp_size;
	u32 tx_gain_err_data_size;
	u32 fb_gain_err_data_size;
	struct work_struct smp_save_dat_work;
	struct work_struct dif_cali_save_bin_work;
	struct work_struct rx_dcoc_save_bin_work;
	struct work_struct tx_cali_save_bin_work;
	struct work_struct rx_cali_gain_lvl_save_bin_work;
	struct work_struct rx_cali_fcomp_save_bin_work;
	struct work_struct tx_err_cali_save_bin_work;
	struct work_struct fb_err_cali_save_bin_work;
	spinlock_t smp_lock;
	spinlock_t rx_dcoc_lock;
	spinlock_t dif_eq_lock;
	spinlock_t tx_cali_lock;
	spinlock_t rx_cali_gain_lvl_lock;
	spinlock_t rx_cali_fcomp_lock;
	spinlock_t tx_gain_err_lock;
	spinlock_t fb_gain_err_lock;
};

struct cls_wifi_cal_file {
	/* parameters */
	uint8_t *rx_mpdu;
	uint32_t rx_mpdu_len;
	/* work queues */
	struct work_struct rx_last_mpdu_save_bin_work;
	/* locks */
	spinlock_t rx_last_mpdu_save_bin_lock;
};

struct vip_node_ctrl {
	bool enable;
	uint8_t traffic_ratio;
	uint16_t node_idx;
	uint16_t pps_thresh;
};

struct cls_wifi_uevent_cs {
	uint8_t band;
	/// CSA mode
	uint8_t blocktx;
	// CSA count
	uint8_t csa_count;
	/// Bandwidth
	uint8_t bw;
	/// Frequency for Primary 20MHz channel (in MHz)
	uint16_t freq;
	/// Frequency center of the contiguous channel or center of Primary 80+80 (in MHz)
	uint16_t center1_freq;
	/// Frequency center of the non-contiguous secondary 80+80 (in MHz)
	uint16_t center2_freq;
};

enum cls_wifi_op_mode {
	CLS_WIFI_AP_MODE,
	CLS_WIFI_STA_MODE,
	CLS_WIFI_REPEATER_MODE,
	CLS_WIFI_MODE_MAX = CLS_WIFI_REPEATER_MODE,
};

enum cls_wifi_dynamic_vlan_mode {
	CLS_WIFI_DYN_VLAN_DISABLE = 0,
	CLS_WIFI_DYN_VLAN_PER_VIF,
	CLS_WIFI_DYN_VLAN_PER_RADIO,
	CLS_WIFI_DYN_VLAN_GLOBLE,
	CLS_WIFI_DYN_VLAN_UNKNOWN = 0xF,
};

extern char cls_wifi_op_mode_name[CLS_WIFI_MODE_MAX + 1][16];

#define CLS_WIFI_INVALID_RUA_MAP 0xFF

#define NUM_CFG80211_COMBINATIONS   2
#define NUM_CFG80211_CIPHER_SUITES  9

/**
 * struct cls_wifi_hw - CLS_WIFI driver main data
 *
 * @dev: Device structure
 *
 * @plat: Underlying CLS_WIFI platform information
 * @phy: PHY Configuration
 * @version_cfm: MACSW/HW versions (as obtained via MM_VERSION_REQ)
 * @machw_type: Type of MACHW (see CLS_WIFI_MACHW_xxx)
 *
 * @radio_params: Radio parameters
 * @flags: Global flags, see enum cls_wifi_dev_flag
 * @task: Tasklet to execute firmware IRQ bottom half
 * @wiphy: Wiphy structure
 * @ext_capa: extended capabilities supported
 *
 * @vifs: List of VIFs currently created
 * @vif_table: Table of all possible VIFs, indexed with fw id.
 * (CLS_REMOTE_STA_MAX extra elements for AP_VLAN interface)
 * @vif_started: Number of vif created at firmware level
 * @avail_idx_map: Bitfield of created interface (indexed by cls_wifi_vif.drv_vif_index)
 * @monitor_vif:  FW id of the monitor interface (CLS_WIFI_INVALID_VIF if no monitor vif)
 *
 * @sta_table: Table of all possible Stations
 * (CLS_VIRT_DEV_MAX] extra elements for BroadCast/MultiCast stations)
 *
 * @chanctx_table: Table of all possible Channel contexts
 * @cur_chanctx: Currently active Channel context
 * @survey: Table of channel surveys
 * @roc: Information of current Remain on Channel request (NULL if Roc requested)
 * @scan_request: Information of current scan request
 * @radar: Radar detection information
 *
 * @tx_lock: Spinlock to protect TX path
 * @txq: Table of STA/TID TX queues
 * @hwq: Table of MACHW queues
 * @txq_cleanup: Timer list to drop packet queued too long in TXQs
 * @sw_txhdr_cache: Cache to allocate
 * @tcp_pacing_shift: TCP buffering configuration (buffering is ~ 2^(10-tps) ms)
 * @mu: Information for Multiuser TX groups
 * @rosource_lock: Used to protect vap.sta_list.
 *  This lock must be acquired before tx_lock can be acquired.
 *
 * @defer_rx: Work to defer RX processing out of IRQ tasklet. Only use for specific mgmt frames
 *
 * @ipc_env: IPC environment
 * @cmd_mgr: FW command manager
 * @cb_lock: Spinlock to protect code from FW confirmation/indication message
 *
 * @msgbuf_pool: Pool of shared buffers to retrieve FW messages
 * @dbgbuf_pool: Pool of shared buffers to retrieve FW debug messages
 * @radar_pool: Pool of shared buffers to retrieve FW radar events
 * @tx_pattern: Shared buffer for the FW to download end of TX buf pattern from
 * @dbgdump: Shared buffer to retrieve FW debug dump
 * @rxdesc_pool: Pool of shared buffers to retrieve RX descriptors
 * @rxbufs: Table of shared buffers to retrieve RX data
 * @rxbuf_idx: Index of the last allocated buffer in rxbufs table
 * @unsuprxvecs: Table of shared buffers to retrieve FW unsupported frames
 * @scan_ie: Shared buffer, allocated to push probe request elements to the FW
 *
 * @ul_params: UL OFDMA parameters that can be set to FW
 * @dl_params: DL OFDMA parameters that can be set to FW
 *
 * @debugfs: Debug FS entries
 * @stats: global statistics
 */
struct cls_wifi_hw {
	struct list_head list;
	struct device *dev;

	u8 radio_idx;
	bool enabled;
	enum cls_wifi_band_cap band_cap;
	struct
	{
		struct ieee80211_supported_band sbands[NUM_NL80211_BANDS];
		struct ieee80211_sband_iftype_data iftype[NUM_NL80211_BANDS][NUM_NL80211_IFTYPES];
		struct ieee80211_txrx_stypes mgmt_stypes[NUM_NL80211_IFTYPES];
		struct ieee80211_iface_combination combinations[NUM_CFG80211_COMBINATIONS];
		struct ieee80211_iface_limit limits[NUM_CFG80211_COMBINATIONS];
		u32 cipher_suites[NUM_CFG80211_CIPHER_SUITES];
	} if_cfg80211;

	// Hardware info
	struct cls_wifi_plat *plat;
	struct cls_wifi_phy_info phy;
	struct mm_version_cfm version_cfm;
	int machw_type;
	bool has_hw_llcsnap_insert;

	// Global wifi config
	struct cls_wifi_mod_params *radio_params;
	unsigned long flags;
	struct wiphy *wiphy;
	bool wiphy_registered;
	u8 ext_capa[10];

	// VIFs
	struct list_head vifs;
	struct cls_wifi_vif *vif_table[CLS_ITF_MAX];
	int vif_started;
	u16 avail_idx_map[ITF_BITMAP_ARRAY_LEN];
	u8 monitor_vif;
	uint16_t vif_num;

	// Stations
	//struct cls_wifi_sta sta_table[CLS_REMOTE_STA_MAX + CLS_VIRT_DEV_MAX];
	struct cls_wifi_sta *sta_table;

	// Channels
	struct cls_wifi_chanctx chanctx_table[CLS_CHAN_CTXT_CNT];
	u8 cur_chanctx;
	struct cls_wifi_survey_info survey[SCAN_CHANNEL_MAX];
	struct cls_wifi_roc *roc;
	struct cfg80211_scan_request *scan_request;
	struct cls_wifi_scan_ext_info scan_ext;
	struct cls_wifi_radar radar;

	// TX path
	spinlock_t tx_lock;
//	struct cls_wifi_txq txq[CLS_NB_TXQ];
	struct cls_wifi_txq *txq;
	struct cls_wifi_hwq hwq[CLS_WIFI_HWQ_NB];
	struct timer_list txq_cleanup;
	struct kmem_cache *sw_txhdr_cache;
	u32 tcp_pacing_shift;
	struct cls_wifi_he_mu he_mu;
	spinlock_t rosource_lock;

	// RX path
	struct cls_wifi_defer_rx defer_rx;

	// IRQ
	struct tasklet_struct task;

	// work queue
	struct work_struct dbg_work;
	struct work_struct dbg_cmn_work;
	struct work_struct csi_work;

	// IPC
	struct ipc_host_env_tag *ipc_env;
	struct ipc_host_cmn_env_tag *ipc_cmn_env;
	struct cls_wifi_cmd_mgr cmd_mgr;
	spinlock_t cb_lock;

	// Shared buffers
	struct cls_wifi_ipc_buf_pool msgbuf_pool;
	struct cls_wifi_ipc_buf_pool dbgbuf_pool;
	struct cls_wifi_ipc_buf_pool radar_pool;
	struct cls_wifi_ipc_buf_pool he_mu_map_pool;
	struct cls_wifi_ipc_buf thd_pattern;
	struct cls_wifi_ipc_buf tbd_pattern;
	struct cls_wifi_ipc_dbgdump dbgdump;
	struct cls_wifi_ipc_buf_pool rxdesc_pool;

	struct cls_wifi_ipc_buf *rxbufs;
	uint32_t rxbufs_nb;
	//struct cls_wifi_ipc_buf rxbufs[CLS_WIFI_RXBUFF_MAX];
	int rxbuf_idx;
	struct cls_wifi_ipc_buf unsuprxvecs[IPC_UNSUPRXVECBUF_CNT];
	struct cls_wifi_ipc_buf scan_ie;
	struct cls_wifi_ipc_buf_pool txcfm_pool;
	struct cls_wifi_ipc_buf_pool csibuf_pool;
	struct cls_wifi_ipc_buf atf_stats_buf;
	struct cls_wifi_ipc_buf atf_quota_buf;

#ifdef CONFIG_CLS_WIFI_HEMU_TX
	struct mm_ul_parameters_req ul_params;
	struct mm_dl_parameters_req dl_params;
#endif /* CONFIG_CLS_WIFI_HEMU_TX */

	/* BF related parameters */
	struct cls_bf_parameters bf_param;

	// Calibration
	struct cls_wifi_cal_env_tag *cal_env;

	struct workqueue_struct *tx_free_skb_workqueue;
	struct work_struct tx_free_skb_work;
	struct sk_buff_head tx_free_skb_queue;

	struct workqueue_struct *txwq_workqueue;
	spinlock_t txwq_lock;

	struct workqueue_struct *snr_workqueue;

	// Debug FS and stats
	struct cls_wifi_debugfs debugfs;
	struct cls_wifi_stats stats;
	struct cls_ipc_stats ipc_stats;
#ifdef CONFIG_WIFI_TRAFFIC_MONITOR
	struct wtm_stats wtm_stats;
#endif
	struct {
		u32 legacy;
		u8 ht_mcs[IEEE80211_HT_MCS_MASK_LEN];
		u16 vht_mcs[NL80211_VHT_NSS_MAX];
		u16 he_mcs[NL80211_HE_NSS_MAX];
		enum nl80211_txrate_gi gi;
		enum nl80211_he_gi he_gi;
		enum nl80211_he_ltf he_ltf;
	} control;
	u8 ppdu_tx_type;
	u8 txbw;
	uint8_t vbss_enabled;
	u32 amsdu_enabled;
	u8 dpd_timer_enabled;
	u32 dpd_timer_interval;
	u8 pppc_enabled;
	struct cls_wifi_dif_fw_sm dif_sm;
	struct cls_wifi_csi_params csi_params;
	struct cls_atf_params atf;
	struct cls_wifi_dpd_wmac_tx_params dpd_wmac_tx_params;
	struct cls_wifi_irf_file irf_file;
	struct cls_wifi_cal_file cal_file;
	s8_l current_power;
	struct work_struct dpd_restore_work;
	// heartbeat
	struct timer_list heartbeat_timer;
	struct work_struct heartbeat_reset_work;
	spinlock_t heartbeat_reset_lock;
	u32 heartbeat_uevent;
	u32 heartbeat_cmdcrash_recover;
	struct vip_node_ctrl vip_node;
	struct timer_list pps_cal_timer;
	struct cls_wifi_ipc_buf dbg_cnt_buf;
	bool csa_fin_flag;
	struct work_struct csa_delay_cali_work;
	enum cls_wifi_op_mode op_mode;
};

struct cls_wifi_cmn_hw {
	struct device *dev;
	struct ipc_host_cmn_env_tag *ipc_host_cmn_env;

	u8 radio_idx;
	bool enabled;

	struct cls_wifi_ipc_buf_pool dbgbuf_pool;
	struct cls_wifi_ipc_buf_pool rxdesc_pool_2g;
	struct cls_wifi_ipc_buf_pool rxdesc_pool_5g;
	struct cls_wifi_ipc_buf rxbufs_2g[CLS_WIFI_RXBUFF_MAX_2G4_CMN];
	struct cls_wifi_ipc_buf rxbufs_5g[CLS_WIFI_RXBUFF_MAX_5G_CMN];
	int rxbuf_idx_2g;
	int rxbuf_idx_5g;
};


u8 *cls_wifi_build_bcn(struct cls_wifi_bcn *bcn, struct cfg80211_beacon_data *new);

void cls_wifi_chanctx_link(struct cls_wifi_vif *vif, u8 idx,
						struct cfg80211_chan_def *chandef);
void cls_wifi_chanctx_unlink(struct cls_wifi_vif *vif);
int  cls_wifi_chanctx_valid(struct cls_wifi_hw *cls_wifi_hw, u8 idx);

struct cls_wifi_vif *cls_wifi_get_vif(struct cls_wifi_hw *cls_wifi_hw, unsigned int vif_idx);
static inline uint8_t master_vif_idx(struct cls_wifi_vif *vif)
{
	if (unlikely(vif->wdev.iftype == NL80211_IFTYPE_AP_VLAN)) {
		return vif->ap_vlan.master->vif_index;
	} else {
		return vif->vif_index;
	}
}

static inline struct cls_wifi_vif *cls_wifi_get_master_vif(struct cls_wifi_vif *cls_wifi_vif)
{
	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) == NL80211_IFTYPE_AP_VLAN)
		return cls_wifi_vif->ap_vlan.master;
	else
		return cls_wifi_vif;
}

struct cls_wifi_sta *cls_wifi_get_sta(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx,
							  struct cls_wifi_vif * vif, bool allow_invalid);
struct cls_wifi_sta *cls_wifi_get_sta_from_mac(struct cls_wifi_hw *cls_wifi_hw, const u8 *mac_addr);
struct cls_wifi_vif *cls_wifi_get_vif_from_mac(struct cls_wifi_hw *cls_wifi_hw, const u8 *mac_addr);
bool cls_wifi_vif_is_active(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, unsigned int vif_idx);
static inline bool is_multicast_sta(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx)
{
	return (sta_idx >= cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx]);
}

static inline void *cls_wifi_get_shared_trace_buf(struct cls_wifi_hw *cls_wifi_hw)
{
	return (void *)&(cls_wifi_hw->debugfs.fw_trace.buf);
}

void cls_wifi_external_auth_enable(struct cls_wifi_vif *vif);
void cls_wifi_external_auth_disable(struct cls_wifi_vif *vif);

void cls_wifi_scan_done(struct cls_wifi_hw *cls_wifi_hw, bool aborted);

void cls_wifi_start_pps_cal_timer(struct cls_wifi_hw *cls_wifi_hw, bool enable);
static inline uint16_t hw_all_sta_max(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx]
			+ cls_wifi_hw->plat->hw_params.vdev_max[cls_wifi_hw->radio_idx]);
}

static inline uint16_t hw_remote_sta_max(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx]);
}

static inline uint8_t hw_vdev_max(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.vdev_max[cls_wifi_hw->radio_idx]);
}

static inline uint8_t hw_mu_user_max(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.mu_user[cls_wifi_hw->radio_idx]);
}

static inline uint16_t nb_ndev_txq(struct cls_wifi_hw *cls_wifi_hw)
{
	return (CLS_NB_TID_PER_STA * cls_wifi_hw->plat->hw_params.sta_max[cls_wifi_hw->radio_idx] + 1);
}

static inline bool is_band_enabled(struct cls_wifi_plat *plat, u8 radio_idx)
{
	return (plat->bands_enabled & (1 << radio_idx));
}

static inline bool cls_wifi_sta_in_ap_vlan(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)
{
	return (sta->vlan_idx >= hw_vdev_max(cls_wifi_hw));
}

void cls_wifi_uevent_channel_switch(struct cls_wifi_vif *vif, struct cls_wifi_uevent_cs *uevent_cs);
bool cls_wifi_is_repeater_mode(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_freq_to_idx(struct cls_wifi_hw *cls_wifi_hw, int freq);
#endif /* _CLS_WIFI_DEFS_H_ */
