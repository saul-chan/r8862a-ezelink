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

#ifndef _LMAC_MSG_H_
#define _LMAC_MSG_H_
#ifdef __KERNEL__
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
// for MAC related elements (mac_addr, mac_ssid...)
#include "lmac_mac.h"
#include "cls_wifi_afe.h"
#include "ipc_shared.h"

/*
 ****************************************************************************************
 */
/////////////////////////////////////////////////////////////////////////////////
// COMMUNICATION WITH LMAC LAYER
/////////////////////////////////////////////////////////////////////////////////
/* Task identifiers for communication between LMAC and DRIVER */
enum {
	TASK_NONE = (u8_l)-1,

	// MAC Management task.
	TASK_MM = 0,
	// DEBUG task
	TASK_DBG,
	/// SCAN task
	TASK_SCAN,
	/// TDLS task
	TASK_TDLS,
	/// SCANU task
	TASK_SCANU,
	/// ME task
	TASK_ME,
	/// SM task
	TASK_SM,
	/// APM task
	TASK_APM,
	/// BAM task
	TASK_BAM,
	/// MESH task
	TASK_MESH,
	/// RXU task
	TASK_RXU,
	/// RM task
	TASK_RM,
	/// TWT task
	TASK_TWT,
	/// FTM task
	TASK_FTM,
	/// Dubhe1000_test task
	TASK_DHT,
	/// IRF task
	TASK_IRF,
	/// TX scheduler
	TASK_TX_SCH,
	/// Calibration task
	TASK_CAL,
	// This is used to define the last task that is running on the EMB processor
	TASK_LAST_EMB = TASK_CAL,
	// nX API task
	TASK_API,
	TASK_MAX,
};


/// For MAC HW States copied from "hal_machw.h"
enum {
	/// MAC HW IDLE State.
	HW_IDLE = 0,
	/// MAC HW RESERVED State.
	HW_RESERVED,
	/// MAC HW DOZE State.
	HW_DOZE,
	/// MAC HW ACTIVE State.
	HW_ACTIVE
};

/// Power Save mode setting
enum mm_ps_mode_state {
	MM_PS_MODE_OFF,
	MM_PS_MODE_ON,
	MM_PS_MODE_ON_DYN,
};

/// Status/error codes used in the MAC software.
enum {
	CO_OK,
	CO_FAIL,
	CO_EMPTY,
	CO_FULL,
	CO_BAD_PARAM,
	CO_NOT_FOUND,
	CO_NO_MORE_ELT_AVAILABLE,
	CO_NO_ELT_IN_USE,
	CO_BUSY,
	CO_OP_IN_PROGRESS,
};

/// Remain on channel operation codes
enum mm_remain_on_channel_op {
	MM_ROC_OP_START = 0,
	MM_ROC_OP_CANCEL,
};

#define DRV_TASK_ID 100
#endif /*__KERNEL__*/
/// Message Identifier. The number of messages is limited to 0xFFFF.
/// The message ID is divided in two parts:
/// - bits[15..10] : task index (no more than 64 tasks supported).
/// - bits[9..0] : message index (no more that 1024 messages per task).
typedef u16 lmac_msg_id_t;

typedef u32 lmac_task_id_t;

#ifdef __KERNEL__
/// Build the first message ID of a task.
#define LMAC_FIRST_MSG(task) ((lmac_msg_id_t)((task) << 10))

#define MSG_T(msg) ((lmac_task_id_t)((msg) >> 10))

#define MSG_I(msg) ((msg) & ((1<<10)-1))
#endif

/// Message structure.
struct lmac_msg {
	lmac_msg_id_t	 id;		 ///< Message id.
	lmac_task_id_t	dest_id;	///< Destination kernel identifier.
	lmac_task_id_t	src_id;	 ///< Source kernel identifier.
	u16		param_len;  ///< Parameter embedded struct length.
	u32		param[];   ///< Parameter embedded struct. Must be word-aligned.
};

#ifdef __KERNEL__
/// List of messages related to the task.
enum mm_msg_tag {
	/// RESET Request.
	MM_RESET_REQ = LMAC_FIRST_MSG(TASK_MM),
	/// RESET Confirmation.
	MM_RESET_CFM,
	/// START Request.
	MM_START_REQ,
	/// START Confirmation.
	MM_START_CFM,
	/// Read Version Request.
	MM_VERSION_REQ,
	/// Read Version Confirmation.
	MM_VERSION_CFM,
	/// ADD INTERFACE Request.
	MM_ADD_IF_REQ,
	/// ADD INTERFACE Confirmation.
	MM_ADD_IF_CFM,
	/// REMOVE INTERFACE Request.
	MM_REMOVE_IF_REQ,
	/// REMOVE INTERFACE Confirmation.
	MM_REMOVE_IF_CFM,
	/// STA ADD Request.
	MM_STA_ADD_REQ,
	/// STA ADD Confirm.
	MM_STA_ADD_CFM,
	/// STA DEL Request.
	MM_STA_DEL_REQ,
	/// STA DEL Confirm.
	MM_STA_DEL_CFM,
	/// RX FILTER CONFIGURATION Request.
	MM_SET_FILTER_REQ,
	/// RX FILTER CONFIGURATION Confirmation.
	MM_SET_FILTER_CFM,
	/// CHANNEL CONFIGURATION Request.
	MM_SET_CHANNEL_REQ,
	/// CHANNEL CONFIGURATION Confirmation.
	MM_SET_CHANNEL_CFM,
	/// DTIM PERIOD CONFIGURATION Request.
	MM_SET_DTIM_REQ,
	/// DTIM PERIOD CONFIGURATION Confirmation.
	MM_SET_DTIM_CFM,
	/// BEACON INTERVAL CONFIGURATION Request.
	MM_SET_BEACON_INT_REQ,
	/// BEACON INTERVAL CONFIGURATION Confirmation.
	MM_SET_BEACON_INT_CFM,
	/// BASIC RATES CONFIGURATION Request.
	MM_SET_BASIC_RATES_REQ,
	/// BASIC RATES CONFIGURATION Confirmation.
	MM_SET_BASIC_RATES_CFM,
	/// BSSID CONFIGURATION Request.
	MM_SET_BSSID_REQ,
	/// BSSID CONFIGURATION Confirmation.
	MM_SET_BSSID_CFM,
	/// EDCA PARAMETERS CONFIGURATION Request.
	MM_SET_EDCA_REQ,
	/// EDCA PARAMETERS CONFIGURATION Confirmation.
	MM_SET_EDCA_CFM,
	/// ABGN MODE CONFIGURATION Request.
	MM_SET_MODE_REQ,
	/// ABGN MODE CONFIGURATION Confirmation.
	MM_SET_MODE_CFM,
	/// Request setting the VIF active state (i.e associated or AP started)
	MM_SET_VIF_STATE_REQ,
	/// Confirmation of the @ref MM_SET_VIF_STATE_REQ message.
	MM_SET_VIF_STATE_CFM,
	/// SLOT TIME PARAMETERS CONFIGURATION Request.
	MM_SET_SLOTTIME_REQ,
	/// SLOT TIME PARAMETERS CONFIGURATION Confirmation.
	MM_SET_SLOTTIME_CFM,
	/// Power Mode Change Request.
	MM_SET_IDLE_REQ,
	/// Power Mode Change Confirm.
	MM_SET_IDLE_CFM,
	/// KEY ADD Request.
	MM_KEY_ADD_REQ,
	/// KEY ADD Confirm.
	MM_KEY_ADD_CFM,
	/// KEY DEL Request.
	MM_KEY_DEL_REQ,
	/// KEY DEL Confirm.
	MM_KEY_DEL_CFM,
	/// Block Ack agreement info addition
	MM_BA_ADD_REQ,
	/// Block Ack agreement info addition confirmation
	MM_BA_ADD_CFM,
	/// Block Ack agreement info deletion
	MM_BA_DEL_REQ,
	/// Block Ack agreement info deletion confirmation
	MM_BA_DEL_CFM,
	/// Indication of the primary TBTT to the upper MAC. Upon the reception of this
	// message the upper MAC has to push the beacon(s) to the beacon transmission queue.
	MM_PRIMARY_TBTT_IND,
	/// Indication of the secondary TBTT to the upper MAC. Upon the reception of this
	// message the upper MAC has to push the beacon(s) to the beacon transmission queue.
	MM_SECONDARY_TBTT_IND,
	/// Request for changing the TX power
	MM_SET_POWER_REQ,
	/// Confirmation of the TX power change
	MM_SET_POWER_CFM,
	/// Request to the LMAC to trigger the embedded logic analyzer and forward the debug
	/// dump.
	MM_DBG_TRIGGER_REQ,
	/// Set Power Save mode
	MM_SET_PS_MODE_REQ,
	/// Set Power Save mode confirmation
	MM_SET_PS_MODE_CFM,
	/// Request to add a channel context
	MM_CHAN_CTXT_ADD_REQ,
	/// Confirmation of the channel context addition
	MM_CHAN_CTXT_ADD_CFM,
	/// Request to delete a channel context
	MM_CHAN_CTXT_DEL_REQ,
	/// Confirmation of the channel context deletion
	MM_CHAN_CTXT_DEL_CFM,
	/// Request to link a channel context to a VIF
	MM_CHAN_CTXT_LINK_REQ,
	/// Confirmation of the channel context link
	MM_CHAN_CTXT_LINK_CFM,
	/// Request to unlink a channel context from a VIF
	MM_CHAN_CTXT_UNLINK_REQ,
	/// Confirmation of the channel context unlink
	MM_CHAN_CTXT_UNLINK_CFM,
	/// Request to update a channel context
	MM_CHAN_CTXT_UPDATE_REQ,
	/// Confirmation of the channel context update
	MM_CHAN_CTXT_UPDATE_CFM,
	/// Request to schedule a channel context
	MM_CHAN_CTXT_SCHED_REQ,
	/// Confirmation of the channel context scheduling
	MM_CHAN_CTXT_SCHED_CFM,
	/// Request to change the beacon template in LMAC
	MM_BCN_CHANGE_REQ,
	/// Confirmation of the beacon change
	MM_BCN_CHANGE_CFM,
	/// Request to update the TIM in the beacon (i.e to indicate traffic bufferized at AP)
	MM_TIM_UPDATE_REQ,
	/// Confirmation of the TIM update
	MM_TIM_UPDATE_CFM,
	/// Connection loss indication
	MM_CONNECTION_LOSS_IND,
	/// Channel context switch indication to the upper layers
	MM_CHANNEL_SWITCH_IND,
	/// Channel context pre-switch indication to the upper layers
	MM_CHANNEL_PRE_SWITCH_IND,
	/// Request to remain on channel or cancel remain on channel
	MM_REMAIN_ON_CHANNEL_REQ,
	/// Confirmation of the (cancel) remain on channel request
	MM_REMAIN_ON_CHANNEL_CFM,
	/// Remain on channel expired indication
	MM_REMAIN_ON_CHANNEL_EXP_IND,
	/// Indication of a PS state change of a peer device
	MM_PS_CHANGE_IND,
	/// Indication that some buffered traffic should be sent to the peer device
	MM_TRAFFIC_REQ_IND,
	/// Request to modify the STA Power-save mode options
	MM_SET_PS_OPTIONS_REQ,
	/// Confirmation of the PS options setting
	MM_SET_PS_OPTIONS_CFM,
	/// Indication of PS state change for a P2P VIF
	MM_P2P_VIF_PS_CHANGE_IND,
	/// Indication that CSA counter has been updated
	MM_CSA_COUNTER_IND,
	/// Channel occupation report indication
	MM_CHANNEL_SURVEY_IND,
	/// Message containing Beamformer Information
	MM_BFMER_ENABLE_REQ,
	/// Request to Start/Stop/Update NOA - GO Only
	MM_SET_P2P_NOA_REQ,
	/// Request to Start/Stop/Update Opportunistic PS - GO Only
	MM_SET_P2P_OPPPS_REQ,
	/// Start/Stop/Update NOA Confirmation
	MM_SET_P2P_NOA_CFM,
	/// Start/Stop/Update Opportunistic PS Confirmation
	MM_SET_P2P_OPPPS_CFM,
	/// P2P NoA Update Indication - GO Only
	MM_P2P_NOA_UPD_IND,
	/// Request to set RSSI threshold and RSSI hysteresis
	MM_CFG_RSSI_REQ,
	/// Indication that RSSI level is below or above the threshold
	MM_RSSI_STATUS_IND,
	/// Indication that CSA is done
	MM_CSA_FINISH_IND,
	/// Indication that CSA is in prorgess (resp. done) and traffic must be stopped (resp. restarted)
	MM_CSA_TRAFFIC_IND,
	/// Indication that Repeater-STA CSA is in prorgess and host should trigger CSA on all Repeater-AP
	MM_REPEATER_CSA_IND,
	/// Request to initialize the antenna diversity algorithm
	MM_ANT_DIV_INIT_REQ,
	/// Request to stop the antenna diversity algorithm
	MM_ANT_DIV_STOP_REQ,
	/// Request to update the antenna switch status
	MM_ANT_DIV_UPDATE_REQ,
	/// Request to switch the antenna connected to path_0
	MM_SWITCH_ANTENNA_REQ,
	/// Indication that a packet loss has occurred
	MM_PKTLOSS_IND,
	/// MU EDCA PARAMETERS Configuration Request.
	MM_SET_MU_EDCA_REQ,
	/// MU EDCA PARAMETERS Configuration Confirmation.
	MM_SET_MU_EDCA_CFM,
	/// UORA PARAMETERS Configuration Request.
	MM_SET_UORA_REQ,
	/// UORA PARAMETERS Configuration Confirmation.
	MM_SET_UORA_CFM,
	/// TXOP RTS THRESHOLD Configuration Request.
	MM_SET_TXOP_RTS_THRES_REQ,
	/// TXOP RTS THRESHOLD Configuration Confirmation.
	MM_SET_TXOP_RTS_THRES_CFM,
	/// HE BSS Color Configuration Request.
	MM_SET_BSS_COLOR_REQ,
	/// HE BSS Color Configuration Confirmation.
	MM_SET_BSS_COLOR_CFM,
	/// Update RC parameters Request
	MM_STA_RC_UPDATE_REQ,
	/// Update RC parameters Confirmation
	MM_STA_RC_UPDATE_CFM,
	/// Request to update Policy Table for a station
	MM_POL_TBL_UPDATE_REQ,
	/// Confirmation to update Policy Table for a station
	MM_POL_TBL_UPDATE_CFM,
	/// Request RC statistics to a station
	MM_RC_STATS_REQ,
	/// RC statistics confirmation
	MM_RC_STATS_CFM,
	/// RC fixed rate request
	MM_RC_SET_RATE_REQ,
	/// Request to configure the UL OFDMA parameters
	MM_UL_PARAMETERS_REQ,
	/// Confirmation of the UL OFDMA parameters configuration
	MM_UL_PARAMETERS_CFM,
	/// Request to configure the DL OFDMA parameters
	MM_DL_PARAMETERS_REQ,
	/// Confirmation of the DL OFDMA parameters configuration
	MM_DL_PARAMETERS_CFM,
	/* Set STA sequence number info */
	MM_SET_SEQ_REQ,
	/* Confirm the Sequence number set request */
	MM_SET_SEQ_CFM,
	/* Get STA sequence number info */
	MM_GET_SEQ_REQ,
	/* Confirm the Sequence number get request */
	MM_GET_SEQ_CFM,
	/* Indication that CCA counter has been updated */
	MM_CCA_COUNTER_IND,
	/* Indication that CCA is done */
	MM_CCA_FINISH_IND,

	/* CSI related commands request */
	MM_CSI_CMD_REQ,
	/* CSI related commands confirm */
	MM_CSI_CMD_CFM,
	/* ATF related */
	MM_SET_ATF_REQ,
	MM_SET_ATF_CFM,

	/* Request for getting the TX power */
	MM_GET_POWER_REQ,
	/* Confirmation of getting the TX power */
	MM_GET_POWER_CFM,

	/* memory operation request */
	MM_MEM_CMD_REQ,
	/* memory operation confirm */
	MM_MEM_CMD_CFM,

	/* DPD WMAC Tx */
	MM_DPD_WMAC_TX_CMD_REQ,
	MM_DPD_WMAC_TX_CMD_IND,

	MM_VIP_NODE_IND,

	/* Request to configure the BF parameters */
	MM_BF_PARAMETERS_REQ,
	/* Confirmation of the BF parameters configuration */
	MM_BF_PARAMETERS_CFM,

	/* Set puncture info */
	MM_SET_PUNCTURE_REQ,
	MM_SET_PUNCTURE_CFM,

	/// Request for getting the broadcast/multicast packet number
	MM_GET_BCTX_PN_REQ,
	/// Confirmation of getting the broadcast/multicast packet number
	MM_GET_BCTX_PN_CFM,

	/// Request for setting the platform params
	MM_PLAT_PARAM_REQ,
	/// Confirmation of getting the platform params
	MM_PLAT_PARAM_CFM,

	/// Request for setting the radar max number threshold params
	MM_RD_MAX_NUM_THRD_REQ,
	/// Confirmation of setting the radar max number threshold params
	MM_RD_MAX_NUM_THRD_CFM,

	/// Request for setting/getting the current channel radar detect used
	MM_RD_CHAN_REQ,
	/// Confirmation of setting/getting the current channel radar detect used
	MM_RD_CHAN_CFM,

	/// Request for setting the radar debug level
	MM_RD_DBG_REQ,
	/// Confirmation of setting the radar debug level
	MM_RD_DBG_CFM,

	/// Request for setting/getting whether agc war is applied
	MM_RD_AGC_WAR_REQ,
	/// Confirmation of setting/getting whether agc war is applied
	MM_RD_AGC_WAR_CFM,

#if CONFIG_CLS_SMTANT
	/// Request for setting the smart antenna configure
	MM_SET_SMANT_CFG_REQ,
	/// Confirmation of setting the smart antenna configure
	MM_SET_SMANT_CFG_CFM,
#endif
	/// MAX number of messages
	MM_MAX,
};

/// Interface types
enum {
	/// ESS STA interface
	MM_STA,
	/// IBSS STA interface
	MM_IBSS,
	/// AP interface
	MM_AP,
	// Mesh Point interface
	MM_MESH_POINT,
	// Monitor interface
	MM_MONITOR,
};

///BA agreement types
enum {
	///BlockAck agreement for TX
	BA_AGMT_TX,
	///BlockAck agreement for RX
	BA_AGMT_RX,
};

///BA agreement related status
enum {
	///Correct BA agreement establishment
	BA_AGMT_ESTABLISHED,
	///BA agreement already exists for STA+TID requested, cannot override it (should have been deleted first)
	BA_AGMT_ALREADY_EXISTS,
	///Correct BA agreement deletion
	BA_AGMT_DELETED,
	///BA agreement for the (STA, TID) doesn't exist so nothing to delete
	BA_AGMT_DOESNT_EXIST,
};

/// Features supported by LMAC - Positions
enum mm_features {
	/// Beaconing
	MM_FEAT_BCN_BIT		 = 0,
	/// Radar Detection
	MM_FEAT_RADAR_BIT,
	/// Power Save
	MM_FEAT_PS_BIT,
	/// UAPSD
	MM_FEAT_UAPSD_BIT,
	/// A-MPDU
	MM_FEAT_AMPDU_BIT,
	/// A-MSDU
	MM_FEAT_AMSDU_BIT,
	/// P2P
	MM_FEAT_P2P_BIT,
	/// P2P Go
	MM_FEAT_P2P_GO_BIT,
	/// UMAC Present
	MM_FEAT_UMAC_BIT,
	/// VHT support
	MM_FEAT_VHT_BIT,
	/// Beamformee
	MM_FEAT_BFMEE_BIT,
	/// Beamformer
	MM_FEAT_BFMER_BIT,
	/// WAPI
	MM_FEAT_WAPI_BIT,
	/// MFP
	MM_FEAT_MFP_BIT,
	/// MU-MIMO RX support
	MM_FEAT_MU_MIMO_RX_BIT,
	/// MU TX support
	MM_FEAT_MU_TX_BIT,
	/// Wireless Mesh Networking
	MM_FEAT_MESH_BIT,
	/// TDLS support
	MM_FEAT_TDLS_BIT,
	/// Antenna Diversity support
	MM_FEAT_ANT_DIV_BIT,
	/// UF support
	MM_FEAT_UF_BIT,
	/// A-MSDU maximum size (bit0)
	MM_AMSDU_MAX_SIZE_BIT0,
	/// A-MSDU maximum size (bit1)
	MM_AMSDU_MAX_SIZE_BIT1,
	/// MON_DATA support
	MM_FEAT_MON_DATA_BIT,
	/// HE (802.11ax) support
	MM_FEAT_HE_BIT,
	/// TWT support
	MM_FEAT_TWT_BIT,
	/// FTM initiator support
	MM_FEAT_FTM_INIT_BIT,
	/// Fake FTM responder support
	MM_FEAT_FAKE_FTM_RSP_BIT,
	/// HW-assisted LLC/SNAP insertion support
	MM_FEAT_HW_LLCSNAP_INS_BIT,
};

/// Maximum number of words in the configuration buffer
#define PHY_CFG_BUF_SIZE	 16

/// Structure containing the parameters of the PHY configuration
struct phy_cfg_tag {
	/// Buffer containing the parameters specific for the PHY used
	u32_l parameters[PHY_CFG_BUF_SIZE];
};

/// Structure containing the parameters of the @ref MM_PLAT_PARAM_REQ message
struct mm_plat_param_req {
	/// use msgq
	u32_l use_msgq;
};

/// Structure containing the parameters of the @ref MM_START_REQ message
struct mm_start_req {
	/// PHY configuration
	struct phy_cfg_tag phy_cfg;
	/// UAPSD timeout
	u32_l uapsd_timeout;
	/// Local LP clock accuracy (in ppm)
	u16_l lp_clk_accuracy;
	/// Array of TX timeout values (in ms, one per TX queue) - 0 sets default value
	u16_l tx_timeout[AC_MAX];
	/// Size of Host RX buffer (in bytes)
	u16_l rx_hostbuf_size;
};

/// Structure containing the parameters of the @ref MM_SET_CHANNEL_REQ message
struct mm_set_channel_req {
	/// Channel information
	struct mac_chan_op chan;
	/// Index of the RF for which the channel has to be set (0: operating (primary), 1: secondary
	/// RF (used for additional radar detection). This parameter is reserved if no secondary RF
	/// is available in the system
	u8_l index;
};

/// Structure containing the parameters of the @ref MM_SET_CHANNEL_CFM message
struct mm_set_channel_cfm {
	/// Radio index to be used in policy table
	u8_l radio_idx;
	/// TX power configured (in dBm)
	s8_l power;
};

/// Structure containing the parameters of the @ref MM_SET_DTIM_REQ message
struct mm_set_dtim_req {
	/// DTIM period
	u8_l dtim_period;
};

/// Structure containing the parameters of the @ref MM_SET_POWER_REQ message
struct mm_set_power_req {
	/// Index of the interface for which the parameter is configured
	u8_l inst_nbr;
	/// TX power (in dBm)
	s8_l power;
};

/// Structure containing the parameters of the @ref MM_GET_POWER_REQ message
struct mm_get_power_req
{
	/// Index of the interface for which the parameter is inquired
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_GET_POWER_CFM message
struct mm_get_power_cfm
{
	/// Radio index to be used in policy table
	u8_l radio_idx;
	/// TX power (in dBm)
	s8_l power;
};

/// Structure containing the parameters of the @ref MM_SET_POWER_CFM message
struct mm_set_power_cfm {
	/// Radio index to be used in policy table
	u8_l radio_idx;
	/// TX power configured (in dBm)
	s8_l power;
};

/// Structure containing the parameters of the @ref MM_SET_BEACON_INT_REQ message
struct mm_set_beacon_int_req {
	/// Beacon interval
	u16_l beacon_int;
	/// Index of the interface for which the parameter is configured
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_SET_BASIC_RATES_REQ message
struct mm_set_basic_rates_req {
	/// Basic rate set (as expected by bssBasicRateSet field of Rates MAC HW register)
	u32_l rates;
	/// Index of the interface for which the parameter is configured
	u8_l inst_nbr;
	/// Band on which the interface will operate
	u8_l band;
};

/// Structure containing the parameters of the @ref MM_SET_BSSID_REQ message
struct mm_set_bssid_req {
	/// BSSID to be configured in HW
	struct mac_addr bssid;
	/// Index of the interface for which the parameter is configured
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_SET_FILTER_REQ message
struct mm_set_filter_req {
	/// RX filter to be put into rxCntrlReg HW register
	u32_l filter;
};

/// Structure containing the parameters of the @ref MM_ADD_IF_REQ message.
struct mm_add_if_req {
	/// Type of the interface (AP, STA, ADHOC, ...)
	u8_l type;
	/// MAC ADDR of the interface to start
	struct mac_addr addr;
	/// P2P Interface
	bool p2p;
	/// BA buffer size, 0 - 64,  1 - 128, 2 - 256
	u8_l ba_buf_size;
};

/// Structure containing the parameters of the @ref MM_SET_EDCA_REQ message
struct mm_set_edca_req {
	/// EDCA parameters of the queue (as expected by edcaACxReg HW register)
	u32_l ac_param;
	/// Flag indicating if UAPSD can be used on this queue
	bool uapsd;
	/// HW queue for which the parameters are configured
	u8_l hw_queue;
	/// Index of the interface for which the parameters are configured
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_SET_MU_EDCA_REQ message
struct mm_set_mu_edca_req {
	/// MU EDCA parameters of the different HE queues
	u32_l param[AC_MAX];
};

/// Structure containing the parameters of the @ref MM_SET_UORA_REQ message
struct mm_set_uora_req {
	/// Minimum exponent of OFDMA Contention Window.
	u8_l eocw_min;
	/// Maximum exponent of OFDMA Contention Window.
	u8_l eocw_max;
};

/// Structure containing the parameters of the @ref MM_SET_TXOP_RTS_THRES_REQ message
struct mm_set_txop_rts_thres_req {
	/// TXOP RTS threshold
	u16_l txop_dur_rts_thres;
	/// Index of the interface for which the parameter is configured
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_SET_BSS_COLOR_REQ message
struct mm_set_bss_color_req {
	/// HE BSS color, formatted as per BSS_COLOR MAC HW register
	u32_l bss_color;
};

struct mm_set_idle_req {
	u8_l hw_idle;
};

/// Structure containing the parameters of the @ref MM_SET_SLOTTIME_REQ message
struct mm_set_slottime_req {
	/// Slot time expressed in us
	u8_l slottime;
};

/// Structure containing the parameters of the @ref MM_SET_MODE_REQ message
struct mm_set_mode_req {
	/// abgnMode field of macCntrl1Reg register
	u8_l abgnmode;
};

/// Structure containing the parameters of the @ref MM_SET_VIF_STATE_REQ message
struct mm_set_vif_state_req {
	/// Association Id received from the AP (valid only if the VIF is of STA type)
	u16_l aid;
	/// Flag indicating if the VIF is active or not
	bool active;
	/// Interface index
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_ADD_IF_CFM message.
struct mm_add_if_cfm {
	/// Status of operation (different from 0 if unsuccessful)
	u8_l status;
	/// Interface index assigned by the LMAC
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_REMOVE_IF_REQ message.
struct mm_remove_if_req {
	/// Interface index assigned by the LMAC
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_VERSION_CFM message.
struct mm_version_cfm {
	/// Version of the LMAC FW
	u32_l version_lmac;
	/// Version1 of the MAC HW (as encoded in version1Reg MAC HW register)
	u32_l version_machw_1;
	/// Version2 of the MAC HW (as encoded in version2Reg MAC HW register)
	u32_l version_machw_2;
	/// Version1 of the PHY (depends on actual PHY)
	u32_l version_phy_1;
	/// Version2 of the PHY (depends on actual PHY)
	u32_l version_phy_2;
	/// Supported Features
	u32_l features;
	/// Maximum number of supported stations
	u16_l max_sta_nb;
	/// Maximum number of supported virtual interfaces
	u8_l max_vif_nb;
	/// Free form string with build information
	u8_l build_info[128];
};

/// Station capability information
struct mm_sta_capa_info {
	/// Legacy rate set supported by the STA
	struct mac_rateset rate_set;
	/// HT capabilities
	struct mac_htcapability ht_cap;
	/// VHT capabilities
	struct mac_vhtcapability vht_cap;
	/// HE capabilities
	struct mac_hecapability he_cap;
	/// Maximum PHY channel bandwidth supported by the STA
	u8_l phy_bw_max;
	/// Current channel bandwidth for the STA
	u8_l bw_cur;
	/// Bit field indicating which queues have to be delivered upon U-APSD trigger
	u8_l uapsd_queues;
	/// Maximum size, in frames, of a APSD service period
	u8_l max_sp_len;
	/// Maximum number of spatial streams supported for STBC reception
	u8_l stbc_nss;
	/// A-MSDU configuration
	u8_l amsdu;
};

/// Structure containing the parameters of the @ref MM_STA_ADD_REQ message.
struct mm_sta_add_req {
	/// Bitfield showing some capabilities of the STA (@ref enum mac_sta_flags)
	u32_l capa_flags;
	/// Maximum A-MPDU size, in bytes, for HE frames
	u32_l ampdu_size_max_he;
	/// Maximum A-MPDU size, in bytes, for VHT frames
	u32_l ampdu_size_max_vht;
	/// PAID/GID
	u32_l paid_gid;
	/// Maximum A-MPDU size, in bytes, for HT frames
	u16_l ampdu_size_max_ht;
	/// MAC address of the station to be added
	struct mac_addr mac_addr;
	/// A-MPDU spacing, in us
	u8_l ampdu_spacing_min;
	/// Interface index
	u8_l inst_nbr;
	/// TDLS station
	bool tdls_sta;
	/// Indicate if the station is TDLS link initiator station
	bool tdls_sta_initiator;
	/// Indicate if the TDLS Channel Switch is allowed
	bool tdls_chsw_allowed;
	/// nonTransmitted BSSID index, set to the BSSID index in case the STA added is an AP
	/// that is a nonTransmitted BSSID. Should be set to 0 otherwise
	u8_l bssid_index;
	/// Maximum BSSID indicator, valid if the STA added is an AP that is a nonTransmitted
	/// BSSID
	u8_l max_bssid_ind;
	/// Station capability information
	struct mm_sta_capa_info info;
};

/// Structure containing the parameters of the @ref MM_STA_ADD_CFM message.
struct mm_sta_add_cfm {
	/// Status of the operation (different from 0 if unsuccessful)
	u8_l status;
	/// Index assigned by the LMAC to the newly added station
	u16_l sta_idx;
};

/// Structure containing the parameters of the @ref MM_STA_DEL_REQ message.
struct mm_sta_del_req {
	/// Index of the station to be deleted
	u16_l sta_idx;
};

/// Structure containing the parameters of the @ref MM_STA_DEL_CFM message.
struct mm_sta_del_cfm {
	/// Status of the operation (different from 0 if unsuccessful)
	u8_l status;
};

/// Structure containing the parameters of the @ref MM_STA_RC_UPDATE_REQ message.
struct mm_sta_rc_update_req {
	/// STA index
	u16_l sta_idx;
	/// Bitfield showing what parameter has been changed (@ref mac_sta_rc_changed)
	u8_l changed;
	/// New Channel Bandwidth for the STA (@ref mac_chan_bandwidth)
	u8_l bw;
	/// New Number of spatial streams supported by the STA
	u8_l nss;
	/// Whether HE-ER is enabled or not
	bool he_er;
};

/// Structure containing the parameters of the @ref MM_POL_TBL_UPDATE_REQ message.
struct mm_pol_tbl_update_req {
	/// VIF index
	u8_l vif_idx;
	/// Bitfield showing what parameter has been changed (@ref mac_bss_changed)
	u8_l bss_info_changed;
	/// CTS protection
	bool erp_cts_prot;
	/// Preamble
	bool erp_preamble;
	/// Basic rateset
	u8_l basic_rates;
	/// BSS color
	u8_l he_bss_color;
	/// BSS color partial
	bool he_bss_color_partial;
	/// BSS Color Disabled
	bool he_bss_color_disabled;
};

/// Structure containing the parameters of the @ref MM_RC_STATS_REQ message.
struct mm_rc_stats_req {
	/// Index of the station for which the RC statistics are requested
	u16_l sta_idx;
};

/// Structure containing the parameters of the @ref MM_UL_PARAMETERS_REQ message.
struct mm_ul_parameters_req {
	/// UL duration maximum value (in us)
	u16_l ul_duration_max;
	/// Max number of spatial streams
	u8_l nss_max;
	/// MCS (0xFF <=> auto)
	u8_l mcs;
	/// Flag indicating whether FEC coding can be used (true <=> auto)
	u8_l fec_allowed;
	/// Flag indicating whether UL duration shall be forced to the maximum value
	u8_l ul_duration_force;
	/// Flag indicating whether UL OFDMA is enabled or not
	u8_l ul_on;
	/* Type of trigger frame */
	u8_l trigger_type;
	/* Schedule mode */
	u8_l sched_mode;
	/* user number in per PPDU */
	u8_l user_num;
	/* MU group ID */
	u8_l grp_id;
	/* 0: auto; 1: manual; 2:trial */
	u8_l work_mode;
	u8_l ul_bw;
	/* 0: 1.6GI 1xLTF; 1: 1.6GI 2XLTF 2: 3.2GI 4XLTF */
	u8_l gi_ltf_mode;
	/* the UL duration for TB PPDU */
	u32_l ul_duration;
	/* Trigger period (us) */
	u32_l tf_period;
	u32_l ul_dbg_level;
};

/// Structure containing the parameters of the @ref MM_DL_PARAMETERS_REQ message.
struct mm_dl_parameters_req {
    /// Max number of spatial streams
    u8_l nss_max;
    /// MCS (0xFF <=> auto)
    u8_l mcs;
    /// Flag indicating whether FEC coding can be used (true <=> auto)
    uint8_t fec_allowed;
    /// Flag indicating whether DL OFDMA is enabled or not
    uint8_t dl_on;
    /* MU ack type */
    uint8_t ack_type;
    /* user number in per PPDU */
    uint8_t user_num;
    /* MU group ID */
    uint8_t grp_id;
    /* 0: OFDMA; 1: MIMO */
    uint8_t mu_type;
    /* 0: auto; 1: manual; 2:trial */
    uint8_t work_mode;
    /* PPDU bandwidth */
    uint8_t ppdu_bw;
    /* GI */
    uint8_t gi;
    /* HE LTF type*/
    uint8_t ltf_type;
    /* DL with 20MHz and 80Mhz combined */
    uint8_t dl_20and80;
    /// trigger txbf
    uint8_t trigger_txbf;
    uint8_t max_ampdu_subfrm;
    uint8_t log_level;
    uint16_t pkt_len_threshold;
};

/// Rate configuration local to Rate controller
enum rc_rate_bf {
	/// Modulation format (@ref hw_format_mod)
	BF_FIELD(RC_RATE, FORMAT_MOD, 0, 4),
	/// MCS index or rate index for legacy rates.
	BF_FIELD(RC_RATE, MCS, 4, 4),
	/// Number of Spatial Stream minus 1
	BF_FIELD(RC_RATE, NSS, 8, 3),
	/// Guard interval (@ref hw_guard_interval)
	BF_FIELD(RC_RATE, GI, 11, 2),
	/// Whether long preamble is mandatory for CCK/DSSS rates
	BF_FIELD(RC_RATE, LONG_PREAMBLE, 13, 1),
	/// Bandwidth (@ref hw_bandwidth)
	BF_FIELD(RC_RATE, BW, 14, 3),
	/// Dual Carrier Modulation (for HE modulation)
	BF_FIELD(RC_RATE, DCM, 17, 1),
	/// RU Size
	BF_FIELD(RC_RATE, RU_SIZE, 18, 4),
	BF_FIELD(RC_RATE, RU_SIZE_VALID, 22, 1),
};

/// Wrapper around @ref BF_GET for @ref rc_rate_bf bitfield
#define RC_RATE_GET(field, val) BF_GET(RC_RATE, field, val)
/// Wrapper around @ref BF_SET for @ref rc_rate_bf bitfield
#define RC_RATE_SET(field, val, field_val) BF_SET(RC_RATE, field, val, field_val)
/// Specific value to indicate an invalid/unknown rate configuration
#define RC_UNKNOWN_RATE 0xFFFFFFFF

/// Structure containing the rate control statistics
struct rc_rate_stats {
	/// Number of attempts (per sampling interval)
	u16_l attempts;
	/// Number of success (per sampling interval)
	u16_l success;
	/// Estimated probability of success (EWMA)
	u16_l probability;
	/// Rate configuration of the sample (@ref rc_rate_bf)
	u32_l rate_config;
#if !defined(MERAK2000)
	u16_l edca_failed;
 	u16_l trial_cnt;
 	u16_l ampdu_cnt;
 	u16_l submpdu_cnt;
 	u16_l ampdu_fail_ba_notrecv;
 	u16_l ampdu_fail_ba_invalid;
 	u16_l ampdu_fail_ba_allzero;
	u16_l collision_punish;
	u16_l coll_cont;
	u16_l coll_cont_max;
	u16_l collision_cnt;
	u16_l collision_attempts;
#endif
	union {
		struct {
			/// Number of times the sample has been skipped (per sampling interval)
			u8_l sample_skipped;
			/// Whether the old probability is available
			bool old_prob_available;
			/// Whether the rate can be used in the retry chain
			bool rate_allowed;
		};
		struct {
			/// UL length received in the latest HE trigger frame
			u16_l ul_length;
		};
	};
};

/// Number of RC samples
#define RC_MAX_N_SAMPLE 10
/// Index of the HE statistics element in the table
#define RC_HE_STATS_IDX RC_MAX_N_SAMPLE

/// Structure containing the parameters of the @ref MM_RC_STATS_CFM message.
struct mm_rc_stats_cfm {
	/// Index of the station for which the RC statistics are provided
	u16_l sta_idx;
	/// Number of samples used in the RC algorithm
	u16_l no_samples;
	/// Number of MPDUs transmitted (per sampling interval)
	u16_l ampdu_len;
	/// Number of AMPDUs transmitted (per sampling interval)
	u16_l ampdu_packets;
	/// Average number of MPDUs in each AMPDU frame (EWMA)
	u32_l avg_ampdu_len;
	// Current step 0 of the retry chain
	u8_l sw_retry_step;
	/// Trial transmission period
	u8_l sample_wait;
	/// Retry chain steps
	u8_l retry_step_idx[4];
	/// RC statistics - Max number of RC samples, plus one for the HE TB statistics
	struct rc_rate_stats rate_stats[RC_MAX_N_SAMPLE + 1];
	/// Throughput - Max number of RC samples, plus one for the HE TB statistics
	u32_l tp[RC_MAX_N_SAMPLE + 1];
};

/// Structure containing the parameters of the @ref MM_RC_SET_RATE_REQ message.
struct mm_rc_set_rate_req {
	/// Index of the station for which the fixed rate is requested
	u16_l sta_idx;
	/// Whether RC should apply a fixed rate (provided in @p rate_config) or
	/// dynamically select the best rate.
	bool fix_rate;
	/// Fixed rate configuration to use if @p fix_rate is true (@ref rc_rate_bf).
	u32_l rate_config;
};

/// Structure containing the parameters of the @ref MM_KEY_ADD REQ message.
struct mm_key_add_req {
	/// Key index (valid only for default keys)
	u8_l key_idx;
	/// STA index (valid only for pairwise or mesh group keys)
	u16_l sta_idx;
	/// Key material
	struct mac_sec_key key;
	/// Cipher suite (WEP64, WEP128, TKIP, CCMP)
	u8_l cipher_suite;
	/// Index of the interface for which the key is set (valid only for default keys or mesh group keys)
	u8_l inst_nbr;
	/// A-MSDU SPP parameter
	u8_l spp;
	/// Indicate if provided key is a pairwise key or not
	bool pairwise;
};

/// Structure containing the parameters of the @ref MM_KEY_ADD_CFM message.
struct mm_key_add_cfm {
	/// Status of the operation (different from 0 if unsuccessful)
	u8_l status;
	/// HW index of the key just added
	u16_l hw_key_idx;
};

/// Structure containing the parameters of the @ref MM_KEY_DEL_REQ message.
struct mm_key_del_req {
	/// HW index of the key to be deleted
	u16_l hw_key_idx;
};

/// Structure containing the parameters of the @ref MM_BA_ADD_REQ message.
struct mm_ba_add_req {
	///Type of agreement (0: TX, 1: RX)
	u8_l type;
	///Index of peer station with which the agreement is made
	u16_l sta_idx;
	///TID for which the agreement is made with peer station
	u8_l tid;
	///Buffer size - number of MPDUs that can be held in its buffer per TID
	u8_l bufsz;
	/// Start sequence number negotiated during BA setup - the one in first aggregated MPDU counts more
	u16_l ssn;
};

/// Structure containing the parameters of the @ref MM_BA_ADD_CFM message.
struct mm_ba_add_cfm {
	///Index of peer station for which the agreement is being confirmed
	u16_l sta_idx;
	///TID for which the agreement is being confirmed
	u8_l tid;
	/// Status of ba establishment
	u8_l status;
};

/// Structure containing the parameters of the @ref MM_BA_DEL_REQ message.
struct mm_ba_del_req {
	///Type of agreement (0: TX, 1: RX)
	u8_l type;
	///Index of peer station for which the agreement is being deleted
	u16_l sta_idx;
	///TID for which the agreement is being deleted
	u8_l tid;
};

/// Structure containing the parameters of the @ref MM_BA_DEL_CFM message.
struct mm_ba_del_cfm {
	///Index of peer station for which the agreement deletion is being confirmed
	u16_l sta_idx;
	///TID for which the agreement deletion is being confirmed
	u8_l tid;
	/// Status of ba deletion
	u8_l status;
};

/// Structure containing the parameters of the @ref MM_CHAN_CTXT_ADD_REQ message
struct mm_chan_ctxt_add_req {
	/// Operating channel
	struct mac_chan_op chan;
};

/// Structure containing the parameters of the @ref MM_CHAN_CTXT_ADD_REQ message
struct mm_chan_ctxt_add_cfm {
	/// Status of the addition
	u8_l status;
	/// Index of the new channel context
	u8_l index;
};


/// Structure containing the parameters of the @ref MM_CHAN_CTXT_DEL_REQ message
struct mm_chan_ctxt_del_req {
	/// Index of the new channel context to be deleted
	u8_l index;
};


/// Structure containing the parameters of the @ref MM_CHAN_CTXT_LINK_REQ message
struct mm_chan_ctxt_link_req {
	/// VIF index
	u8_l vif_index;
	/// Channel context index
	u8_l chan_index;
	/// Indicate if this is a channel switch (unlink current ctx first if true)
	u8_l chan_switch;
};

/// Structure containing the parameters of the @ref MM_CHAN_CTXT_UNLINK_REQ message
struct mm_chan_ctxt_unlink_req {
	/// VIF index
	u8_l vif_index;
};

/// Structure containing the parameters of the @ref MM_CHAN_CTXT_UPDATE_REQ message
struct mm_chan_ctxt_update_req {
	/// Channel context index
	u8_l chan_index;
	/// New channel information
	struct mac_chan_op chan;
};

/// Structure containing the parameters of the @ref MM_CHAN_CTXT_SCHED_REQ message
struct mm_chan_ctxt_sched_req {
	/// VIF index
	u8_l vif_index;
	/// Channel context index
	u8_l chan_index;
	/// Type of the scheduling request (0: normal scheduling, 1: derogatory
	/// scheduling)
	u8_l type;
};

/// Structure containing the parameters of the @ref MM_CHANNEL_SWITCH_IND message
struct mm_channel_switch_ind {
	/// Index of the channel context we will switch to
	u8_l chan_index;
	/// Indicate if the switch has been triggered by a Remain on channel request
	bool roc;
	/// VIF on which remain on channel operation has been started (if roc == 1)
	u8_l vif_index;
	/// Indicate if the switch has been triggered by a TDLS Remain on channel request
	bool roc_tdls;
	/// Duration, in us, during which channel context will be active (if roc == 1)
	u32_l duration_us;
	/// Frequency of the channel context's primary channel
	u16_l freq;
};

/// Structure containing the parameters of the @ref MM_CHANNEL_PRE_SWITCH_IND message
struct mm_channel_pre_switch_ind {
	/// Index of the channel context we will switch to
	u8_l chan_index;
};

/// Structure containing the parameters of the @ref MM_CONNECTION_LOSS_IND message.
struct mm_connection_loss_ind {
	/// VIF instance number
	u8_l inst_nbr;
};


/// Structure containing the parameters of the @ref MM_DBG_TRIGGER_REQ message.
struct mm_dbg_trigger_req {
	/// Error trace to be reported by the LMAC
	char error[64];
};

/// Structure containing the parameters of the @ref MM_SET_PS_MODE_REQ message.
struct mm_set_ps_mode_req {
	/// Power Save is activated or deactivated
	u8_l  new_state;
};

/// Structure containing the parameters of the @ref MM_BCN_CHANGE_REQ message.
#define BCN_MAX_CSA_CPT 2
struct mm_bcn_change_req {
	/// Pointer, in host memory, to the new beacon template
	u32_l bcn_ptr;
	/// Length of the beacon template
	u16_l bcn_len;
	/// Offset of the TIM IE in the beacon
	u16_l tim_oft;
	/// Length of the TIM IE
	u8_l tim_len;
	/// Index of the VIF for which the beacon is updated
	u8_l inst_nbr;
	/// Offset of CSA (channel switch announcement) counters (0 means no counter)
	u8_l csa_oft[BCN_MAX_CSA_CPT];
	/// Offset of CCA (color change announcement) counter (0 means no counter)
	u8_l cca_oft;
};


/// Structure containing the parameters of the @ref MM_TIM_UPDATE_REQ message.
struct mm_tim_update_req {
	/// Association ID of the STA the bit of which has to be updated (0 for BC/MC traffic)
	u16_l aid;
	/// Flag indicating the availability of data packets for the given STA
	u8_l tx_avail;
	/// Index of the VIF for which the TIM is updated
	u8_l inst_nbr;
};

/// Structure containing the parameters of the @ref MM_REMAIN_ON_CHANNEL_REQ message.
struct mm_remain_on_channel_req {
	/// Operation Code
	u8_l op_code;
	/// VIF Index
	u8_l vif_index;
	/// Channel parameter
	struct mac_chan_op chan;
	/// Duration (in ms)
	u32_l duration_ms;
};

/// Structure containing the parameters of the @ref MM_REMAIN_ON_CHANNEL_CFM message
struct mm_remain_on_channel_cfm {
	/// Operation Code
	u8_l op_code;
	/// Status of the operation
	u8_l status;
	/// Channel Context index
	u8_l chan_ctxt_index;
};

/// Structure containing the parameters of the @ref MM_REMAIN_ON_CHANNEL_EXP_IND message
struct mm_remain_on_channel_exp_ind {
	/// VIF Index
	u8_l vif_index;
	/// Channel Context index
	u8_l chan_ctxt_index;
	/// Frequency of the channel
	u16_l freq;
};

/// Structure containing the parameters of the @ref MM_SET_UAPSD_TMR_REQ message.
struct mm_set_uapsd_tmr_req {
	/// action: Start or Stop the timer
	u8_l  action;
	/// timeout value, in milliseconds
	u32_l  timeout;
};

/// Structure containing the parameters of the @ref MM_SET_UAPSD_TMR_CFM message.
struct mm_set_uapsd_tmr_cfm {
	/// Status of the operation (different from 0 if unsuccessful)
	u8_l	 status;
};


/// Structure containing the parameters of the @ref MM_PS_CHANGE_IND message
struct mm_ps_change_ind {
	/// Index of the peer device that is switching its PS state
	u16_l sta_idx;
	/// New PS state of the peer device (0: active, 1: sleeping)
	u8_l ps_state;
};

/// Structure containing the parameters of the @ref MM_P2P_VIF_PS_CHANGE_IND message
struct mm_p2p_vif_ps_change_ind {
	/// Index of the P2P VIF that is switching its PS state
	u8_l vif_index;
	/// New PS state of the P2P VIF interface (0: active, 1: sleeping)
	u8_l ps_state;
};

/// Structure containing the parameters of the @ref MM_TRAFFIC_REQ_IND message
struct mm_traffic_req_ind {
	/// Index of the peer device that needs traffic
	u16_l sta_idx;
	/// Number of packets that need to be sent (if 0, all buffered traffic shall be sent and
	/// if set to @ref PS_SP_INTERRUPTED, it means that current service period has been interrupted)
	u8_l pkt_cnt;
	/// Flag indicating if the traffic request concerns U-APSD queues or not
	bool uapsd;
};

/// Structure containing the parameters of the @ref MM_SET_PS_OPTIONS_REQ message.
struct mm_set_ps_options_req {
	/// VIF Index
	u8_l vif_index;
	/// Listen interval (0 if wake up shall be based on DTIM period)
	u16_l listen_interval;
	/// Flag indicating if we shall listen the BC/MC traffic or not
	bool dont_listen_bc_mc;
};

/// Structure containing the parameters of the @ref MM_CSA_COUNTER_IND message
struct mm_csa_counter_ind {
	/// Index of the VIF
	u8_l vif_index;
	/// Updated CSA counter value
	u8_l csa_count;
};

/// Structure containing the parameters of the @ref MM_CCA_COUNTER_IND message
struct mm_cca_counter_ind
{
	/// Index of the VIF
	u8_l vif_index;
	/// Updated CCA counter value
	u8_l cca_count;
};

/// Structure containing the parameters of the @ref MM_CHANNEL_SURVEY_IND message
struct mm_channel_survey_ind {
	/// Frequency of the channel
	u16_l freq;
	/// Noise in dbm
	s8_l noise_dbm;
	/// Amount of time spent of the channel (in ms)
	u32_l chan_time_ms;
	/// Amount of time the primary channel was sensed busy
	u32_l chan_time_busy_ms;
};

/// Structure containing the parameters of the @ref MM_BFMER_ENABLE_REQ message.
struct mm_bfmer_enable_req {
	/**
	 * Address of the beamforming report space allocated in host memory
	 * (Valid only if vht_su_bfmee is true)
	 */
	u32_l host_bfr_addr;
	/**
	 * Size of the beamforming report space allocated in host memory. This space should
	 * be twice the maximum size of the expected beamforming reports as the FW will
	 * divide it in two in order to be able to upload a new report while another one is
	 * used in transmission
	 */
	u16_l host_bfr_size;
	/// AID
	u16_l aid;
	/// Station Index
	u16_l sta_idx;
	/// Maximum number of spatial streams the station can receive
	u8_l rx_nss;
	/**
	 * Indicate if peer STA is MU Beamformee capable
	 */
	bool mu_bfmee;
};

/// Structure containing the parameters of the @ref MM_SET_P2P_NOA_REQ message.
struct mm_set_p2p_noa_req {
	/// VIF Index
	u8_l vif_index;
	/// Allocated NOA Instance Number - Valid only if count = 0
	u8_l noa_inst_nb;
	/// Count
	u8_l count;
	/// Indicate if NoA can be paused for traffic reason
	bool dyn_noa;
	/// Duration (in us)
	u32_l duration_us;
	/// Interval (in us)
	u32_l interval_us;
	/// Start Time offset from next TBTT (in us)
	u32_l start_offset;
};

/// Structure containing the parameters of the @ref MM_SET_P2P_OPPPS_REQ message.
struct mm_set_p2p_oppps_req {
	/// VIF Index
	u8_l vif_index;
	/// CTWindow
	u8_l ctwindow;
};

/// Structure containing the parameters of the @ref MM_SET_P2P_NOA_CFM message.
struct mm_set_p2p_noa_cfm {
	/// Request status
	u8_l status;
};

/// Structure containing the parameters of the @ref MM_SET_P2P_OPPPS_CFM message.
struct mm_set_p2p_oppps_cfm {
	/// Request status
	u8_l status;
};

/// Structure containing the parameters of the @ref MM_P2P_NOA_UPD_IND message.
struct mm_p2p_noa_upd_ind {
	/// VIF Index
	u8_l vif_index;
	/// NOA Instance Number
	u8_l noa_inst_nb;
	/// NoA Type
	u8_l noa_type;
	/// Count
	u8_l count;
	/// Duration (in us)
	u32_l duration_us;
	/// Interval (in us)
	u32_l interval_us;
	/// Start Time
	u32_l start_time;
};

/// Structure containing the parameters of the @ref MM_CFG_RSSI_REQ message
struct mm_cfg_rssi_req {
	/// Index of the VIF
	u8_l vif_index;
	/// RSSI threshold
	s8_l rssi_thold;
	/// RSSI hysteresis
	u8_l rssi_hyst;
};

/// Structure containing the parameters of the @ref MM_RSSI_STATUS_IND message
struct mm_rssi_status_ind {
	/// Index of the VIF
	u8_l vif_index;
	/// Status of the RSSI
	bool rssi_status;
	/// Current RSSI
	s8_l rssi;
};

/// Structure containing the parameters of the @ref MM_PKTLOSS_IND message
struct mm_pktloss_ind {
	/// Index of the VIF
	u8_l vif_index;
	/// Address of the STA for which there is a packet loss
	struct mac_addr mac_addr;
	/// Number of packets lost
	u32 num_packets;
};

/// Structure containing the parameters of the @ref MM_CSA_FINISH_IND message
struct mm_csa_finish_ind {
	/// Index of the VIF
	u8_l vif_index;
	/// Status of the operation
	u8_l status;
	/// New channel ctx index
	u8_l chan_idx;
};

/// Structure containing the parameters of the @ref MM_CCA_FINISH_IND message
struct mm_cca_finish_ind
{
	/// Index of the VIF
	u8_l vif_index;
	/// Status of the operation
	u8_l status;
};

/// Structure containing the parameters of the @ref MM_CSA_TRAFFIC_IND message
struct mm_csa_traffic_ind {
	/// Index of the VIF
	u8_l vif_index;
	/// Is tx traffic enable or disable
	bool enable;
};

/// Structure containing the parameters of the @ref MM_REPEATER_CSA_IND message
struct mm_repeater_csa_ind
{
	/// Index of the VIF
	uint8_t vif_index;
	/// CSA mode
	uint8_t blocktx;
	/// CSA count
	uint8_t csa_count;
	/// Bandwidth: enum mac_chan_bandwidth
	uint8_t bw;
	/// Frequency for Primary 20MHz channel (in MHz)
	uint16_t freq;
	/// Frequency center of the contiguous channel or center of Primary 80+80 (in MHz)
	uint16_t center1_freq;
	/// Frequency center of the non-contiguous secondary 80+80 (in MHz)
	uint16_t center2_freq;
};

/// Structure containing the parameters of the @ref MM_GET_BCTX_PN_REQ message.
struct mm_bctx_pn_req {
	/// VIF instance number
	u8_l inst_nbr;
};

struct mm_bctx_pn_cfm {
	/// VIF instance number
	u8_l inst_nbr;
	/// tx packet number
	uint64_t tx_pn;
};

struct mm_vip_node_ind {
	bool enable;
	u8_l traffic_ratio;
	u16_l vip_node_idx;
	u16_l pps_thresh;
};

struct mm_puncture_params_req {
	uint8_t vif_idx;
	uint8_t inact_bitmap;
	uint8_t padding[2];
};

///////////////////////////////////////////////////////////////////////////////
/////////// For Scan messages
///////////////////////////////////////////////////////////////////////////////
enum scan_msg_tag {
	/// Scanning start Request.
	SCAN_START_REQ = LMAC_FIRST_MSG(TASK_SCAN),
	/// Scanning start Confirmation.
	SCAN_START_CFM,
	/// End of scanning indication.
	SCAN_DONE_IND,
	/// Cancel scan request
	SCAN_CANCEL_REQ,
	/// Cancel scan confirmation
	SCAN_CANCEL_CFM,

	/// MAX number of messages
	SCAN_MAX,
};

/// Maximum number of SSIDs in a scan request
#define SCAN_SSID_MAX   2

/// Maximum number of channels in a scan request
#define SCAN_CHANNEL_MAX (MAC_DOMAINCHANNEL_24G_MAX + MAC_DOMAINCHANNEL_5G_MAX)

/// Maximum length of the ProbeReq IEs (SoftMAC mode)
#define SCAN_MAX_IE_LEN 300

/// Maximum number of PHY bands supported
#define SCAN_BAND_MAX 2

/// Structure containing the parameters of the @ref SCAN_START_REQ message
struct scan_start_req {
	/// List of channel to be scanned
	struct mac_chan_def chan[SCAN_CHANNEL_MAX];
	/// List of SSIDs to be scanned
	struct mac_ssid ssid[SCAN_SSID_MAX];
	/// BSSID to be scanned
	struct mac_addr bssid;
	/// Pointer (in host memory) to the additional IEs that need to be added to the ProbeReq
	/// (following the SSID element)
	u32_l add_ies;
	/// Length of the additional IEs
	u16_l add_ie_len;
	/// Index of the VIF that is scanning
	u8_l vif_idx;
	/// Number of channels to scan
	u8_l chan_cnt;
	/// Number of SSIDs to scan for
	u8_l ssid_cnt;
	/// no CCK - For P2P frames not being sent at CCK rate in 2GHz band.
	bool no_cck;
	/// Scan duration, in us
	u32_l duration;
};

/// Structure containing the parameters of the @ref SCAN_START_CFM message
struct scan_start_cfm {
	/// Status of the request
	u8_l status;
};

/// Structure containing the parameters of the @ref SCAN_CANCEL_REQ message
struct scan_cancel_req {
};

/// Structure containing the parameters of the @ref SCAN_START_CFM message
struct scan_cancel_cfm {
	/// Status of the request
	u8_l status;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For Scanu messages
///////////////////////////////////////////////////////////////////////////////
/// Messages that are logically related to the task.
enum {
	/// Scan request from host.
	SCANU_START_REQ = LMAC_FIRST_MSG(TASK_SCANU),
	/// Scanning start Confirmation.
	SCANU_START_CFM,
	/// Join request
	SCANU_JOIN_REQ,
	/// Join confirmation.
	SCANU_JOIN_CFM,
	/// Scan result indication.
	SCANU_RESULT_IND,
	/// Get Scan result request.
	SCANU_GET_SCAN_RESULT_REQ,
	/// Scan result confirmation.
	SCANU_GET_SCAN_RESULT_CFM,
	/// Abort current scan request
	SCANU_ABORT_REQ,
	/// Abort current scan confirmation
	SCANU_ABORT_CFM,

	/// MAX number of messages
	SCANU_MAX,
};

/// Maximum length of the additional ProbeReq IEs (FullMAC mode)
#define SCANU_MAX_IE_LEN  200

/// Structure containing the parameters of the @ref SCANU_START_REQ message
struct scanu_start_req {
	/// List of channel to be scanned
	struct mac_chan_def chan[SCAN_CHANNEL_MAX];
	/// List of SSIDs to be scanned
	struct mac_ssid ssid[SCAN_SSID_MAX];
	/// BSSID to be scanned (or WILDCARD BSSID if no BSSID is searched in particular)
	struct mac_addr bssid;
	/// Address (in host memory) of the additional IEs that need to be added to the ProbeReq
	/// (following the SSID element)
	u32_l add_ies;
	/// Length of the additional IEs
	u16_l add_ie_len;
	/// Index of the VIF that is scanning
	u8_l vif_idx;
	/// Number of channels to scan
	u8_l chan_cnt;
	/// Number of SSIDs to scan for
	u8_l ssid_cnt;
	/// no CCK - For P2P frames not being sent at CCK rate in 2GHz band.
	bool no_cck;
	/// Scan duration, in us
	u32_l duration;
	// off-chan scan, true means off-chan scan, false means normal scan
	bool off_channel;
	/// ext_enabled
	u32 ext_enabled;
	/// mask for rx packets type, valid only when ext_enabled is true
	u32 rx_filter;
	/// duration for work channel, in us, valid only when ext_enabled is true
	u32 work_duration;
	/// channel nr after which return to work channel, valid only when ext_enabled is true
	u32 scan_interval;
};

/// Scan Status for @ref scanu_start_cfm::status
enum scanu_status {
	/// Scan request has been completed
	SCANU_DONE,
	/// Scan request has been aborted
	SCANU_ABORTED,
	/// Scan request cannot start as another request is already on-going
	SCANU_BUSY,
	/// Scan undefined error
	SCAN_ERROR,
};

/// Structure containing the parameters of the @ref SCANU_START_CFM message
struct scanu_start_cfm {
	/// Index of the VIF that was scanning
	u8_l vif_idx;
	/// Status of the request (@ref scanu_status)
	u8_l status;
	/// Number of scan results available
	u8_l result_cnt;
};

/// Rx frame legacy information
struct rx_leg_info {
	/// Format Modulation
	u32_l	format_mod	 : 4;
	/// Channel Bandwidth
	u32_l	ch_bw		  : 3;
	/// Preamble Type
	u32_l	pre_type	   : 1;
	/// Legacy Length
	u32_l	leg_length	 :12;
	/// Legacy rate
	u32_l	leg_rate	   : 4;
} __packed;

/// Parameters of the @SCANU_RESULT_IND message
struct scanu_result_ind {
	/// Length of the frame
	u16_l length;
	/// Frame control field of the frame.
	u16_l framectrl;
	/// Center frequency on which we received the packet
	u16_l center_freq;
	/// PHY band
	u8_l band;
	/// Index of the station that sent the frame. 0xFF if unknown.
	u16_l sta_idx;
	/// Index of the VIF that received the frame. 0xFF if unknown.
	u8_l inst_nbr;
	/// RSSI of the received frame.
	s8_l rssi;
	/// Rx frame legacy information
	struct rx_leg_info rx_leg_inf;
	/// Frame payload.
	u32_l payload[];
};

/// Structure containing the parameters of the message.
struct scanu_abort_req {
	/// Index of the VIF that was scanning
	u8_l vif_idx;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For ME messages
///////////////////////////////////////////////////////////////////////////////
/// Messages that are logically related to the task.
enum {
	/// Configuration request from host.
	ME_CONFIG_REQ = LMAC_FIRST_MSG(TASK_ME),
	/// Configuration confirmation.
	ME_CONFIG_CFM,
	/// Configuration request from host.
	ME_CHAN_CONFIG_REQ,
	/// Configuration confirmation.
	ME_CHAN_CONFIG_CFM,
	/// Set control port state for a station.
	ME_SET_CONTROL_PORT_REQ,
	/// Control port setting confirmation.
	ME_SET_CONTROL_PORT_CFM,
	/// TKIP MIC failure indication.
	ME_TKIP_MIC_FAILURE_IND,
	/// Add a station to the FW (AP mode)
	ME_STA_ADD_REQ,
	/// Confirmation of the STA addition
	ME_STA_ADD_CFM,
	/// Delete a station from the FW (AP mode)
	ME_STA_DEL_REQ,
	/// Confirmation of the STA deletion
	ME_STA_DEL_CFM,
	/// Indication of a TX RA/TID queue credit update
	ME_TX_CREDITS_UPDATE_IND,
	/// Request indicating to the FW that there is traffic buffered on host
	ME_TRAFFIC_IND_REQ,
	/// Confirmation that the @ref ME_TRAFFIC_IND_REQ has been executed
	ME_TRAFFIC_IND_CFM,
	/// Configure monitor interface
	ME_CONFIG_MONITOR_REQ,
	/// Configure monitor interface response
	ME_CONFIG_MONITOR_CFM,
	/// Setting power Save mode request from host
	ME_SET_PS_MODE_REQ,
	/// Set power Save mode confirmation
	ME_SET_PS_MODE_CFM,
	/// MAX number of messages
	ME_MAX,
};

/// Structure containing the parameters of the @ref ME_CONFIG_REQ message
struct me_config_req {
	/// HT Capabilities
	struct mac_htcapability ht_cap;
	/// VHT Capabilities
	struct mac_vhtcapability vht_cap;
	/// HE capabilities
	struct mac_hecapability he_cap;
	/* EHT capabilities */
	struct mac_eht_capability eht_cap;
	/// Lifetime of packets sent under a BlockAck agreement (expressed in TUs)
	u16_l tx_lft;
	/// Maximum supported BW
	u8_l phy_bw_max;
	/// Boolean indicating if HT is supported or not
	bool ht_supp;
	/// Boolean indicating if VHT is supported or not
	bool vht_supp;
	/// Boolean indicating if HE is supported or not
	bool he_supp;
	/// Boolean indicating if EHT is supported or not
	bool eht_supp;
	/// Boolean indicating if PS mode shall be enabled or not
	bool ps_on;
	/// Boolean indicating if Antenna Diversity shall be enabled or not
	bool ant_div_on;
	/// Boolean indicating if Dynamic PS mode shall be used or not
	bool dpsm;
	/// Indicates whether AMSDU shall be forced or not (0-if advertised, 1-yes, 2-no)
	int amsdu_tx;
};

/// Structure containing the parameters of the @ref ME_CHAN_CONFIG_REQ message
struct me_chan_config_req {
	/// List of 2.4GHz supported channels
	struct mac_chan_def chan2G4[MAC_DOMAINCHANNEL_24G_MAX];
	/// List of 5GHz supported channels
	struct mac_chan_def chan5G[MAC_DOMAINCHANNEL_5G_MAX];
	/// Number of 2.4GHz channels in the list
	u8_l chan2G4_cnt;
	/// Number of 5GHz channels in the list
	u8_l chan5G_cnt;
};

/// Structure containing the parameters of the @ref ME_SET_CONTROL_PORT_REQ message
struct me_set_control_port_req {
	/// Index of the station for which the control port is opened
	u16_l sta_idx;
	/// Control port state
	bool control_port_open;
};

/// Structure containing the parameters of the @ref ME_TKIP_MIC_FAILURE_IND message
struct me_tkip_mic_failure_ind {
	/// Address of the sending STA
	struct mac_addr addr;
	/// TSC value
	u64_l tsc;
	/// Boolean indicating if the packet was a group or unicast one (true if group)
	bool ga;
	/// Key Id
	u8_l keyid;
	/// VIF index
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref ME_STA_ADD_REQ message
struct me_sta_add_req {
	/// MAC address of the station to be added
	struct mac_addr mac_addr;
	/// Supported legacy rates
	struct mac_rateset rate_set;
	/// HT Capabilities
	struct mac_htcapability ht_cap;
	/// VHT Capabilities
	struct mac_vhtcapability vht_cap;
	/// HE capabilities
	struct mac_hecapability he_cap;
	/// EHT capabilities
	struct mac_eht_capability eht_cap;
	/// Flags giving additional information about the station (@ref mac_sta_flags)
	u32_l flags;
	/// Association ID of the station
	u16_l aid;
	/// Bit field indicating which queues have U-APSD enabled
	u8_l uapsd_queues;
	/// Maximum size, in frames, of a APSD service period
	u8_l max_sp_len;
	/// Operation mode information (valid if bit @ref STA_OPMOD_NOTIF is
	/// set in the flags)
	u8_l opmode;
	/// Index of the VIF the station is attached to
	u8_l vif_idx;
	/// Whether the station is TDLS station
	bool tdls_sta;
	/// Indicate if the station is TDLS link initiator station
	bool tdls_sta_initiator;
	/// Indicate if the TDLS Channel Switch is allowed
	bool tdls_chsw_allowed;
};

/// Structure containing the parameters of the @ref ME_STA_ADD_CFM message
struct me_sta_add_cfm {
	/// Station index
	u16_l sta_idx;
	/// Status of the station addition
	u8_l status;
	/// PM state of the station
	u8_l pm_state;
};

/// Structure containing the parameters of the @ref ME_STA_DEL_REQ message.
struct me_sta_del_req {
	/// Index of the station to be deleted
	u16_l sta_idx;
	/// Whether the station is TDLS station
	bool tdls_sta;
};

/// Structure containing the parameters of the @ref ME_TX_CREDITS_UPDATE_IND message.
struct me_tx_credits_update_ind {
	/// Index of the station for which the credits are updated
	u16_l sta_idx;
	/// TID for which the credits are updated
	u16_l tid;
	/// Offset to be applied on the credit count
	s16_l credits;
};

/// Structure containing the parameters of the @ref ME_TRAFFIC_IND_REQ message.
struct me_traffic_ind_req {
	/// Index of the station for which UAPSD traffic is available on host
	u16_l sta_idx;
	/// Flag indicating the availability of UAPSD packets for the given STA
	u8_l tx_avail;
	/// Indicate if traffic is on uapsd-enabled queues
	bool uapsd;
};

/// Structure containing the parameters of the @ref ME_CONFIG_MONITOR_REQ message.
struct me_config_monitor_req {
	/// Channel to configure
	struct mac_chan_op chan;
	/// Is channel data valid
	bool chan_set;
	/// Enable report of unsupported HT frames
	bool uf;
};

/// Structure containing the parameters of the @ref ME_CONFIG_MONITOR_CFM message.
struct me_config_monitor_cfm {
	/// Channel context index
	u8_l chan_index;
	/// Channel parameters
	struct mac_chan_op chan;
};

/// Structure containing the parameters of the @ref ME_SET_PS_MODE_REQ message.
struct me_set_ps_mode_req {
	/// Power Save is activated or deactivated
	u8_l  ps_state;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For TWT messages
///////////////////////////////////////////////////////////////////////////////
/// Message API of the TWT task
enum {
	/// Request Individual TWT Establishment
	TWT_SETUP_REQ = LMAC_FIRST_MSG(TASK_TWT),
	/// Confirm Individual TWT Establishment
	TWT_SETUP_CFM,
	/// Indicate TWT Setup response from peer
	TWT_SETUP_IND,
	/// Request to destroy a TWT Establishment or all of them
	TWT_TEARDOWN_REQ,
	/// Confirm to destroy a TWT Establishment or all of them
	TWT_TEARDOWN_CFM,
	/// Indicate TWT Service Period start/end
	TWT_SP_IND,

	/// MAX number of messages
	TWT_MAX,
};

///TWT Group assignment
struct twt_grp_assignment_tag {
	/// TWT Group assignment byte array
	u8_l grp_assignment[9];
};

///TWT Flow configuration
struct twt_conf_tag {
	/// Flow Type (0: Announced, 1: Unannounced)
	u8_l flow_type;
	/// Wake interval Exponent
	u8_l wake_int_exp;
	/// Unit of measurement of TWT Minimum Wake Duration (0:256us, 1:tu)
	bool wake_dur_unit;
	/// Nominal Minimum TWT Wake Duration
	u8_l min_twt_wake_dur;
	/// TWT Wake Interval Mantissa
	u16_l wake_int_mantissa;
};

///TWT Setup request message structure
struct twt_setup_req {
	/// VIF Index
	u8_l vif_idx;
	/// Setup request type
	u8_l setup_type;
	/// TWT Setup configuration
	struct twt_conf_tag conf;
};

///TWT Setup confirmation message structure
struct twt_setup_cfm {
	/// Status (0 = TWT Setup Request has been transmitted to peer)
	u8_l status;
};

/// TWT Setup command
enum twt_setup_types {
	MAC_TWT_SETUP_REQ = 0,
	MAC_TWT_SETUP_SUGGEST,
	MAC_TWT_SETUP_DEMAND,
	MAC_TWT_SETUP_GROUPING,
	MAC_TWT_SETUP_ACCEPT,
	MAC_TWT_SETUP_ALTERNATE,
	MAC_TWT_SETUP_DICTATE,
	MAC_TWT_SETUP_REJECT,
};

///TWT Setup indication message structure
struct twt_setup_ind {
	/// Response type
	u8_l resp_type;
	/// STA Index
	u16_l sta_idx;
	/// TWT Setup configuration
	struct twt_conf_tag conf;
};

/// TWT Teardown request message structure
struct twt_teardown_req {
	/// TWT Negotiation type
	u8_l neg_type;
	/// All TWT
	u8_l all_twt;
	/// TWT flow ID
	u8_l id;
	/// VIF Index
	u8_l vif_idx;
};

///TWT Teardown confirmation message structure
struct twt_teardown_cfm {
	/// Status (0 = TWT Teardown Request has been transmitted to peer)
	u8_l status;
};

/// TWT Service Period Indication
struct twt_sp_ind {
	/// STA Index
	u16_l sta_idx;
	/// TWT Flow ID
	u8_l flow_id;
	/// Whether the SP started or ended
	bool active;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For SM messages
///////////////////////////////////////////////////////////////////////////////
/// Message API of the SM task
enum sm_msg_tag {
	/// Request to connect to an AP
	SM_CONNECT_REQ = LMAC_FIRST_MSG(TASK_SM),
	/// Confirmation of connection
	SM_CONNECT_CFM,
	/// Indicates that the SM associated to the AP
	SM_CONNECT_IND,
	/// Request to disconnect
	SM_DISCONNECT_REQ,
	/// Confirmation of disconnection
	SM_DISCONNECT_CFM,
	/// Indicates that the SM disassociated the AP
	SM_DISCONNECT_IND,
	/// Request to start external authentication
	SM_EXTERNAL_AUTH_REQUIRED_IND,
	/// Response to external authentication request
	SM_EXTERNAL_AUTH_REQUIRED_RSP,
	/// Request to update assoc elements after FT over the air authentication
	SM_FT_AUTH_IND,
	/// Response to FT authentication with updated assoc elements
	SM_FT_AUTH_RSP,

	/// MAX number of messages
	SM_MAX,
};

/// Structure containing the parameters of @ref SM_CONNECT_REQ and SM_FT_AUTH_RSP message.
struct sm_connect_req {
	/// SSID to connect to
	struct mac_ssid ssid;
	/// BSSID to connect to (if not specified, set this field to WILDCARD BSSID)
	struct mac_addr bssid;
	/// Channel on which we have to connect (if not specified, set -1 in the chan.freq field)
	struct mac_chan_def chan;
	/// Connection flags (see @ref mac_connection_flags)
	u32_l flags;
	/// Control port Ethertype (in network endianness)
	u16_l ctrl_port_ethertype;
	/// Listen interval to be used for this connection
	u16_l listen_interval;
	/// Flag indicating if the we have to wait for the BC/MC traffic after beacon or not
	bool dont_wait_bcmc;
	/// Authentication type
	u8_l auth_type;
	/// UAPSD queues (bit0: VO, bit1: VI, bit2: BE, bit3: BK)
	u8_l uapsd_queues;
	/// VIF index
	u8_l vif_idx;
	/// Length of the association request IEs
	u16_l ie_len;
	/// Buffer containing the additional information elements to be put in the
	/// association request
	u32_l ie_buf[0];
};

/// Structure containing the parameters of the @ref SM_CONNECT_CFM message.
struct sm_connect_cfm {
	/// Status. If 0, it means that the connection procedure will be performed and that
	/// a subsequent @ref SM_CONNECT_IND message will be forwarded once the procedure is
	/// completed
	u8_l status;
};

#define SM_ASSOC_IE_LEN   800
/// Structure containing the parameters of the @ref SM_CONNECT_IND message.
struct sm_connect_ind {
	/// Status code of the connection procedure
	u16_l status_code;
	/// BSSID
	struct mac_addr bssid;
	/// Flag indicating if the indication refers to an internal roaming or from a host request
	bool roamed;
	/// Index of the VIF for which the association process is complete
	u8_l vif_idx;
	/// Index of the STA entry allocated for the AP
	u16_l ap_sta_idx;
	/// Index of the LMAC channel context the connection is attached to
	u8_l ch_idx;
	/// Flag indicating if the AP is supporting QoS
	bool qos;
	/// ACM bits set in the AP WMM parameter element
	u8_l acm;
	/// Length of the AssocReq IEs
	u16_l assoc_req_ie_len;
	/// Length of the AssocRsp IEs
	u16_l assoc_rsp_ie_len;
	/// Association Id allocated by the AP for this connection
	u16_l aid;
	/// AP operating channel
	struct mac_chan_op chan;
	/// EDCA parameters
	u32_l ac_param[AC_MAX];
	/// IE buffer
	u32_l assoc_ie_buf[0];
};

/// Structure containing the parameters of the @ref SM_DISCONNECT_REQ message.
struct sm_disconnect_req {
	/// Reason of the deauthentication.
	u16_l reason_code;
	/// Index of the VIF.
	u8_l vif_idx;
};

/// Structure containing the parameters of SM_ASSOCIATION_IND the message
struct sm_association_ind {
	// MAC ADDR of the STA
	struct mac_addr	 me_mac_addr;
};


/// Structure containing the parameters of the @ref SM_DISCONNECT_IND message.
struct sm_disconnect_ind {
	/// Reason of the disconnection.
	u16_l reason_code;
	/// Index of the VIF.
	u8_l vif_idx;
	/// Disconnection happen before a re-association
	bool reassoc;
};

/// Structure containing the parameters of the @ref SM_EXTERNAL_AUTH_REQUIRED_IND
struct sm_external_auth_required_ind {
	/// Index of the VIF.
	u8_l vif_idx;
	/// SSID to authenticate to
	struct mac_ssid ssid;
	/// BSSID to authenticate to
	struct mac_addr bssid;
	/// AKM suite of the respective authentication
	u32_l akm;
};

/// Structure containing the parameters of the @ref SM_EXTERNAL_AUTH_REQUIRED_RSP
struct sm_external_auth_required_rsp {
	/// Index of the VIF.
	u8_l vif_idx;
	/// Authentication status
	u16_l status;
};

/// Structure containing the parameters of the @ref SM_FT_AUTH_IND
struct sm_ft_auth_ind {
	/// Index of the VIF.
	u8_l vif_idx;
	/// Size of the FT elements
	u16_l ft_ie_len;
	/// Fast Transition elements in the authentication
	u32_l ft_ie_buf[0];
};

///////////////////////////////////////////////////////////////////////////////
/////////// For APM messages
///////////////////////////////////////////////////////////////////////////////
/// Message API of the APM task
enum apm_msg_tag {
	/// Request to start the AP.
	APM_START_REQ = LMAC_FIRST_MSG(TASK_APM),
	/// Confirmation of the AP start.
	APM_START_CFM,
	/// Request to stop the AP.
	APM_STOP_REQ,
	/// Confirmation of the AP stop.
	APM_STOP_CFM,
	/// Request to start CAC
	APM_START_CAC_REQ,
	/// Confirmation of the CAC start
	APM_START_CAC_CFM,
	/// Request to stop CAC
	APM_STOP_CAC_REQ,
	/// Confirmation of the CAC stop
	APM_STOP_CAC_CFM,
	/// Request to Probe Client
	APM_PROBE_CLIENT_REQ,
	/// Confirmation of Probe Client
	APM_PROBE_CLIENT_CFM,
	/// Indication of Probe Client status
	APM_PROBE_CLIENT_IND,

	/// MAX number of messages
	APM_MAX,
};

/// Structure containing the parameters of the @ref APM_START_REQ message.
struct apm_start_req {
	/// Basic rate set
	struct mac_rateset basic_rates;
	/// Operating channel on which we have to enable the AP
	struct mac_chan_op chan;
	/// Address, in host memory, to the beacon template
	u32_l bcn_addr;
	/// Length of the beacon template
	u16_l bcn_len;
	/// Offset of the TIM IE in the beacon
	u16_l tim_oft;
	/// Beacon interval
	u16_l bcn_int;
	/// Flags (@ref mac_connection_flags)
	u32_l flags;
	/// Control port Ethertype
	u16_l ctrl_port_ethertype;
	/// Length of the TIM IE
	u8_l tim_len;
	/// Index of the VIF for which the AP is started
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref APM_START_CFM message.
struct apm_start_cfm {
	/// Status of the AP starting procedure
	u8_l status;
	/// Index of the VIF for which the AP is started
	u8_l vif_idx;
	/// Index of the channel context attached to the VIF
	u8_l ch_idx;
	/// Index of the STA used for BC/MC traffic
	u8_l bcmc_idx;
};

/// Structure containing the parameters of the @ref APM_STOP_REQ message.
struct apm_stop_req {
	/// Index of the VIF for which the AP has to be stopped
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref APM_START_CAC_REQ message.
struct apm_start_cac_req {
	/// Channel configuration
	struct mac_chan_op chan;
	/// Index of the VIF for which the CAC is started
	u8_l vif_idx;
	/// Radar detect enable
	bool rd_enable;
};

/// Structure containing the parameters of the @ref APM_START_CAC_CFM message.
struct apm_start_cac_cfm {
	/// Status of the CAC starting procedure
	u8_l status;
	/// Index of the channel context attached to the VIF for CAC
	u8_l ch_idx;
};

/// Structure containing the parameters of the @ref APM_STOP_CAC_REQ message.
struct apm_stop_cac_req {
	/// Index of the VIF for which the CAC has to be stopped
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref APM_PROBE_CLIENT_REQ message.
struct apm_probe_client_req {
	/// Index of the VIF
	u8_l vif_idx;
	/// Index of the Station to probe
	u16_l sta_idx;
};

/// Structure containing the parameters of the @ref APM_PROBE_CLIENT_CFM message.
struct apm_probe_client_cfm {
	/// Status of the probe request
	u8_l status;
	/// Unique ID to distinguish @ref APM_PROBE_CLIENT_IND message
	u32_l probe_id;
};

/// Structure containing the parameters of the @ref APM_PROBE_CLIENT_CFM message.
struct apm_probe_client_ind {
	/// Index of the VIF
	u8_l vif_idx;
	/// Index of the Station to probe
	u16_l sta_idx;
	/// Whether client is still present or not
	bool client_present;
	/// Unique ID as returned in @ref APM_PROBE_CLIENT_CFM
	u32_l probe_id;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For MESH messages
///////////////////////////////////////////////////////////////////////////////

/// Maximum length of the Mesh ID
#define MESH_MESHID_MAX_LEN	 (32)

/// Message API of the MESH task
enum mesh_msg_tag {
	/// Request to start the MP
	MESH_START_REQ = LMAC_FIRST_MSG(TASK_MESH),
	/// Confirmation of the MP start.
	MESH_START_CFM,

	/// Request to stop the MP.
	MESH_STOP_REQ,
	/// Confirmation of the MP stop.
	MESH_STOP_CFM,

	// Request to update the MP
	MESH_UPDATE_REQ,
	/// Confirmation of the MP update
	MESH_UPDATE_CFM,

	/// Request information about a given link
	MESH_PEER_INFO_REQ,
	/// Response to the MESH_PEER_INFO_REQ message
	MESH_PEER_INFO_CFM,

	/// Request automatic establishment of a path with a given mesh STA
	MESH_PATH_CREATE_REQ,
	/// Confirmation to the MESH_PATH_CREATE_REQ message
	MESH_PATH_CREATE_CFM,

	/// Request a path update (delete path, modify next hop mesh STA)
	MESH_PATH_UPDATE_REQ,
	/// Confirmation to the MESH_PATH_UPDATE_REQ message
	MESH_PATH_UPDATE_CFM,

	/// Indication from Host that the indicated Mesh Interface is a proxy for an external STA
	MESH_PROXY_ADD_REQ,

	/// Indicate that a connection has been established or lost
	MESH_PEER_UPDATE_IND,
	/// Notification that a connection has been established or lost (when MPM handled by userspace)
	MESH_PEER_UPDATE_NTF = MESH_PEER_UPDATE_IND,

	/// Indicate that a path is now active or inactive
	MESH_PATH_UPDATE_IND,
	/// Indicate that proxy information have been updated
	MESH_PROXY_UPDATE_IND,

	/// MAX number of messages
	MESH_MAX,
};

/// Structure containing the parameters of the @ref MESH_START_REQ message.
struct mesh_start_req {
	/// Basic rate set
	struct mac_rateset basic_rates;
	/// Operating channel on which we have to enable the AP
	struct mac_chan_op chan;
	/// DTIM Period
	u8_l dtim_period;
	/// Beacon Interval
	u16_l bcn_int;
	/// Index of the VIF for which the MP is started
	u8_l vif_index;
	/// Length of the Mesh ID
	u8_l mesh_id_len;
	/// Mesh ID
	u8_l mesh_id[MESH_MESHID_MAX_LEN];
	/// Address of the IEs to download
	u32_l ie_addr;
	/// Length of the provided IEs
	u8_l ie_len;
	/// Indicate if Mesh Peering Management (MPM) protocol is handled in userspace
	bool user_mpm;
	/// Indicate if Mesh Point is using authentication
	bool is_auth;
	/// Indicate which authentication method is used
	u8_l auth_id;
};

/// Structure containing the parameters of the @ref MESH_START_CFM message.
struct mesh_start_cfm {
	/// Status of the MP starting procedure
	u8_l status;
	/// Index of the VIF for which the MP is started
	u8_l vif_idx;
	/// Index of the channel context attached to the VIF
	u8_l ch_idx;
	/// Index of the STA used for BC/MC traffic
	u8_l bcmc_idx;
};

/// Structure containing the parameters of the @ref MESH_STOP_REQ message.
struct mesh_stop_req {
	/// Index of the VIF for which the MP has to be stopped
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref MESH_STOP_CFM message.
struct mesh_stop_cfm {
	/// Index of the VIF for which the MP has to be stopped
	u8_l vif_idx;
   /// Status
	u8_l status;
};

/// Bit fields for mesh_update_req message's flags value
enum mesh_update_flags_bit {
	/// Root Mode
	MESH_UPDATE_FLAGS_ROOT_MODE_BIT = 0,
	/// Gate Mode
	MESH_UPDATE_FLAGS_GATE_MODE_BIT,
	/// Mesh Forwarding
	MESH_UPDATE_FLAGS_MESH_FWD_BIT,
	/// Local Power Save Mode
	MESH_UPDATE_FLAGS_LOCAL_PSM_BIT,
};

/// Structure containing the parameters of the @ref MESH_UPDATE_REQ message.
struct mesh_update_req {
	/// Flags, indicate fields which have been updated
	u8_l flags;
	/// VIF Index
	u8_l vif_idx;
	/// Root Mode
	u8_l root_mode;
	/// Gate Announcement
	bool gate_announ;
	/// Mesh Forwarding
	bool mesh_forward;
	/// Local PS Mode
	u8_l local_ps_mode;
};

/// Structure containing the parameters of the @ref MESH_UPDATE_CFM message.
struct mesh_update_cfm {
	/// Status
	u8_l status;
};

/// Structure containing the parameters of the @ref MESH_PEER_INFO_REQ message.
struct mesh_peer_info_req {
	///Index of the station allocated for the peer
	u16_l sta_idx;
};

/// Structure containing the parameters of the @ref MESH_PEER_INFO_CFM message.
struct mesh_peer_info_cfm {
	/// Response status
	u8_l status;
	/// Index of the station allocated for the peer
	u16_l sta_idx;
	/// Local Link ID
	u16_l local_link_id;
	/// Peer Link ID
	u16_l peer_link_id;
	/// Local PS Mode
	u8_l local_ps_mode;
	/// Peer PS Mode
	u8_l peer_ps_mode;
	/// Non-peer PS Mode
	u8_l non_peer_ps_mode;
	/// Link State
	u8_l link_state;
	/// Time elapsed since last received beacon (in us)
	u32_l last_bcn_age;
};

/// Structure containing the parameters of the @ref MESH_PATH_CREATE_REQ message.
struct mesh_path_create_req {
	/// Index of the interface on which path has to be created
	u8_l vif_idx;
	/// Indicate if originator MAC Address is provided
	bool has_orig_addr;
	/// Path Target MAC Address
	struct mac_addr tgt_mac_addr;
	/// Originator MAC Address
	struct mac_addr orig_mac_addr;
};

/// Structure containing the parameters of the @ref MESH_PATH_CREATE_CFM message.
struct mesh_path_create_cfm {
	/// Confirmation status
	u8_l status;
	/// VIF Index
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref MESH_PATH_UPDATE_REQ message.
struct mesh_path_update_req {
	/// Indicate if path must be deleted
	bool delete;
	/// Index of the interface on which path has to be created
	u8_l vif_idx;
	/// Path Target MAC Address
	struct mac_addr tgt_mac_addr;
	/// Next Hop MAC Address
	struct mac_addr nhop_mac_addr;
};

/// Structure containing the parameters of the @ref MESH_PATH_UPDATE_CFM message.
struct mesh_path_update_cfm {
	/// Confirmation status
	u8_l status;
	/// VIF Index
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref MESH_PROXY_ADD_REQ message.
struct mesh_proxy_add_req {
	/// VIF Index
	u8_l vif_idx;
	/// MAC Address of the External STA
	struct mac_addr ext_sta_addr;
};

/// Structure containing the parameters of the @ref MESH_PROXY_UPDATE_IND
struct mesh_proxy_update_ind {
	/// Indicate if proxy information has been added or deleted
	bool delete;
	/// Indicate if we are a proxy for the external STA
	bool local;
	/// VIF Index
	u8_l vif_idx;
	/// MAC Address of the External STA
	struct mac_addr ext_sta_addr;
	/// MAC Address of the proxy (only valid if local is false)
	struct mac_addr proxy_mac_addr;
};

/// Structure containing the parameters of the @ref MESH_PEER_UPDATE_IND message.
struct mesh_peer_update_ind {
	/// Indicate if connection has been established or lost
	bool estab;
	/// VIF Index
	u8_l vif_idx;
	/// STA Index
	u16_l sta_idx;
	/// Peer MAC Address
	struct mac_addr peer_addr;
};

/// Structure containing the parameters of the @ref MESH_PEER_UPDATE_NTF message.
struct mesh_peer_update_ntf {
	/// VIF Index
	u8_l vif_idx;
	/// STA Index
	u16_l sta_idx;
	/// Mesh Link State
	u8_l state;
};

/// Structure containing the parameters of the @ref MESH_PATH_UPDATE_IND message.
struct mesh_path_update_ind {
	/// Indicate if path is deleted or not
	bool delete;
	/// Indicate if path is towards an external STA (not part of MBSS)
	bool ext_sta;
	/// VIF Index
	u8_l vif_idx;
	/// Path Index
	u8_l path_idx;
	/// Target MAC Address
	struct mac_addr tgt_mac_addr;
	/// External STA MAC Address (only if ext_sta is true)
	struct mac_addr ext_sta_mac_addr;
	/// Next Hop STA Index
	u16_l nhop_sta_idx;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For Debug messages
///////////////////////////////////////////////////////////////////////////////

/// Messages related to Debug Task
enum dbg_msg_tag {
	/// Memory read request
	DBG_MEM_READ_REQ = LMAC_FIRST_MSG(TASK_DBG),
	/// Memory read confirm
	DBG_MEM_READ_CFM,
	/// Memory write request
	DBG_MEM_WRITE_REQ,
	/// Memory write confirm
	DBG_MEM_WRITE_CFM,
	/// Module filter request
	DBG_SET_MOD_FILTER_REQ,
	/// Module filter confirm
	DBG_SET_MOD_FILTER_CFM,
	/// Severity filter request
	DBG_SET_SEV_FILTER_REQ,
	/// Severity filter confirm
	DBG_SET_SEV_FILTER_CFM,
	/// LMAC/MAC HW fatal error indication
	DBG_ERROR_IND,
	/// Request to get system statistics
	DBG_GET_SYS_STAT_REQ,
	/// COnfirmation of system statistics
	DBG_GET_SYS_STAT_CFM,
	/// Timer allowing resetting the system statistics periodically to avoid
	/// wrap around of timer
	DBG_SYS_STAT_TIMER,

	DBG_GET_SYS_MIB_REG,
	DBG_GET_SYS_MIB_CFM,
	DBG_GET_SYS_PHY_DFX_REG,
	DBG_GET_SYS_PHY_DFX_CFM,

	DBG_GET_DBGCNT_REG,
	DBG_GET_DBGCNT_CFM,

	/// Request for getting Performance Counters Status Statistics
	DBG_PCT_STATS_REQ,
	/// Confirmation of getting Performance Counters Status Statistics
	DBG_PCT_STATS_CFM,

	/// Request for getting Profiling Counter Status Statistics
	DBG_PROFILE_STATS_REQ,
	/// Confirmation of getting Profiling Counter Status Statistics
	DBG_PROFILE_STATS_CFM,

#if defined(MERAK2000) && MERAK2000
	DBG_SET_TRACE_LOG_LEVEL_REQ,
	DBG_SET_TRACE_LOG_LEVEL_CFM,

	DBG_GET_TRACE_LOG_LEVEL_REQ,
	DBG_GET_TRACE_LOG_LEVEL_CFM,
#endif
	/* get mgmt frame stats info */
	DBG_GET_MGMT_STATS_REQ,
	DBG_GET_MGMT_STATS_CFM,

	/* reset mgmt frame stats info */
	DBG_RESET_MGMT_STATS_REQ,
	DBG_RESET_MGMT_STATS_CFM,

	/// Max number of Debug messages
	DBG_MAX,
};

/// Structure containing the parameters of the @ref DBG_MEM_READ_REQ message.
struct dbg_mem_read_req {
	u32_l memaddr;
};

/// Structure containing the parameters of the @ref DBG_MEM_READ_CFM message.
struct dbg_mem_read_cfm {
	u32_l memaddr;
	u32_l memdata;
};

/// Structure containing the parameters of the @ref DBG_MEM_WRITE_REQ message.
struct dbg_mem_write_req {
	u32_l memaddr;
	u32_l memdata;
};

/// Structure containing the parameters of the @ref DBG_MEM_WRITE_CFM message.
struct dbg_mem_write_cfm {
	u32_l memaddr;
	u32_l memdata;
};

/// Structure containing the parameters of the @ref DBG_SET_MOD_FILTER_REQ message.
struct dbg_set_mod_filter_req {
	/// Bit field indicating for each module if the traces are enabled or not
	u32_l mod_filter;
};

/// Structure containing the parameters of the @ref DBG_SEV_MOD_FILTER_REQ message.
struct dbg_set_sev_filter_req {
	/// Bit field indicating the severity threshold for the traces
	u32_l sev_filter;
};

/// Structure containing the parameters of the @ref DBG_GET_SYS_STAT_CFM message.
struct dbg_get_sys_stat_cfm {
	/// Time spent in CPU sleep since last reset of the system statistics
	u64_l cpu_sleep_time;
	/// Time spent in DOZE since last reset of the system statistics
	u64_l doze_time;
	/// Total time spent since last reset of the system statistics
	u64_l stats_time;
};

/// Structure containing the parameters of the @ref DBG_PCT_STATS_REQ message
struct dbg_pct_stat_req {
	u16_l cnt0_cc_idx;
	u16_l cnt1_cc_idx;
	u8_l ku_mode;
	u8_l exception;
	u8_l interrupt;
	u8_l uflag_op;
	u8_l uflag0;
	u8_l uflag1;
	u8_l runflag;
	u8_l res;
};

/// Structure containing the parameters of the @ref DBG_PCT_STATS_CFM message
struct dbg_pct_stat_cfm {
	char cnt0_name[8];
	char cnt1_name[8];
	u32_l cnt0_l;
	u32_l cnt0_h;
	u32_l cnt1_l;
	u32_l cnt1_h;
	u8_l run_sec;
	u8_l status;
};

struct dbg_profile_info {
	u32_l buf_size;
	u8_l enable;
	u8_l run;
	u8_l dump;
	u8_l res;
};

/// Structure containing the parameters of the @ref DBG_PROFILE_STATS_REQ message
struct dbg_profile_stat_req {
	u32_l cmd_val;
	u8_l cmd;
	u8_l res;
};

#define PROFILE_BUF_MAX 128
/// Structure containing the parameters of the @ref DBG_PCT_STATS_CFM message
struct dbg_profile_stat_cfm {
	u32_l cmd_val;
	u8_l cmd;
	u8_l status;
	u8_l enable;
	u8_l run;
	u16_l buf_count;
	u16_l buf_start;
	u16_l buf_end;
	u32_l buf_size;
	u32_l buf_point;
	u32_l buf[PROFILE_BUF_MAX];
};

/*MIB MACRO*/
#define	BASIC_MIB_SET_START     0
#define	BASIC_MIB_SET_NUM       18
#define	EDCA_MIB_SET_START      20
#define	EDCA_MIB_SET_NUM        88
#define	TRIGGER_MIB_SET_START   108
#define	TRIGGER_MIB_SET_NUM     11
#define	AMPDU_MIB_SET_START     119
#define	AMPDU_MIB_SET_NUM       16
#define	BW_TXRX_MIB_SET_START   144
#define	BW_TXRX_MIB_SET_NUM     19
#define	BF_MIB_SET_START        172
#define	BF_MIB_SET_NUM          4

#define EDCA_MIB_SET_NUM_ITEM   11


#define MIB_ITEM_NB  (BASIC_MIB_SET_NUM + EDCA_MIB_SET_NUM + TRIGGER_MIB_SET_NUM \
                        + AMPDU_MIB_SET_NUM + BW_TXRX_MIB_SET_NUM + BF_MIB_SET_NUM)

struct basic_mib_info {
	u32_l info[BASIC_MIB_SET_NUM];
};

struct edca_mib_info {
	u32_l info[EDCA_MIB_SET_NUM];
};

struct trigger_based_mib_info {
	u32_l info[TRIGGER_MIB_SET_NUM];
};

struct ampdu_mib_info {
	u32_l info[AMPDU_MIB_SET_NUM];
};

struct bw_mib_info {
	u32_l info[BW_TXRX_MIB_SET_NUM];
};

struct bfr_mib_info {
	u32_l info[BF_MIB_SET_NUM];
};

struct mib_infor {
	struct basic_mib_info basic;
	struct edca_mib_info edca;
	struct trigger_based_mib_info tb;
	struct ampdu_mib_info ampdu;
	struct bw_mib_info bw;
	struct bfr_mib_info bfr;
};

enum MIB_UPDATE_FLAG
{
	MIB_UPDATE_BASIC = 1,
	MIB_UPDATE_EDCA = 2,
	MIB_UPDATE_TB = 4,
	MIB_UPDATE_AMPDU = 8,
	MIB_UPDATE_BW = 0x10,
	MIB_UPDATE_BFR = 0x20,
	MIB_UPDATE_STATIC_REG = 0x40,
	MIB_MAX_ITEM = 7,

	MIB_DUMP_VALUE = 0x400,

	MIB_UPDATE_ALL = 0x7F
};

struct dbg_get_mib_req {
    u16_l get_mib_flag;  ///enum MIB_UPDATE_FLAG
    u16_l not_ignore_cfm;
};

struct dbg_get_dbgcnt_req {
	u32_l reset;
};

struct dbg_get_dbgcnt_cfm {
	u32_l success;
};

#define DBG_MACREG_ITEM   12
struct dbg_mac_reg_info{
    u32_l mpif_underflow_dbg;
    u32_l mpif_tb_rx_err_dbg;
    u32_l mac_rx_hang_ctrl;
    u32_l mac_rx_hang_dbg0;
    u32_l mac_rx_hang_dbg1;
    u32_l mpif_underflow_dbg2;
    u32_l mpif_underflow_dbg3;
    u32_l rx_vector1_dbg[5];
};

struct dbg_get_mib_cfm {
    u32_l mib_flag;   ///enum MIB_UPDATE_FLAG
    u64_l cur_wpu_time;
    struct mib_infor mib;
    struct dbg_mac_reg_info mac_reg;
};

#define PHYDFX_COMM_DFX   (3)
#define PHYDFX_PPDU_TRX   (12)
#define PHYDFX_ABN_RPT    (5)
#define PHYDFX_TXV        (12)

#define PHYDFX_ITEM_NB    (PHYDFX_COMM_DFX + PHYDFX_PPDU_TRX + PHYDFX_ABN_RPT + PHYDFX_TXV)

enum PHYDFX_UPDATE_FLAG
{
	PHYDFX_UPDATE_BASIC = 0x1,

	PHYDFX_UPDATE_ALL = 0xFF
};

struct dbg_get_phy_dfx_req {
    u16_l get_mib_flag;  ///enum MIB_UPDATE_FLAG
    u16_l not_ignore_cfm;
};


struct phy_dfx_comm_info{
    u32_l comm_info_rpt[3];
};

struct phydfx_ppdu_static{
    u16_l tx_ofdm_start_cnt;
    u16_l tx_dsss_start_cnt;
    u16_l tx_ppdu_end_cnt;

    u16_l rx_ofdm_start_cnt;
    u16_l rx_dsss_start_cnt;
    u16_l rx_ppdu_timingend_cnt;
    u16_l rx_ppdu_end_cnt;
    u16_l rx_ppdu_fd_done_cnt;
    u16_l rx_ppdu_sfo_done_cnt;
    u16_l rx_ppdu_td_done_cnt;

    u16_l tx_err_ppdu_cnt;
    u16_l rx_err_ppdu_cnt;
};

struct phydfx_abn_rpt {
    u32_l rx_abn_rpt_aa70;
    u32_l rx_abn_rpt_aa74;
    u32_l rx_time_rpt_aa78;
    u32_l rx_time_rpt_aa7c;
    u32_l rx_ppdu_idx_aa80;
};

struct phydfx_ppdu_txv {
    u32_l txv[9];
    u32_l txv_abn;
    u32_l txbf_abn;
    u32_l txv_mask;
};


struct phy_dfx_infor {
    struct phy_dfx_comm_info comm;
    struct phydfx_ppdu_static ppdu_tx_rx;
    struct phydfx_abn_rpt abn_rpt;
    struct phydfx_ppdu_txv txv;
};

struct dbg_get_phy_dfx_cfm {
    u32_l mib_flag;   ///enum PHYDFX_UPDATE_FLAG
    u32_l cur_wpu_time;
    struct phy_dfx_infor dfx;
};

struct dbg_trace_loglevel_set_req {
	uint8_t mod_index;
	uint32_t value;
};

struct dbg_trace_loglevel_set_cfm {
	uint32_t status;
};

struct dbg_trace_loglevel_get_req {
	uint8_t radio;
};

struct dbg_trace_loglevel_get_cfm {
	uint8_t mod_num;
	uint32_t mod_level_list[128];
	uint32_t status;
};

struct dbg_get_mgmt_stats_req {
	uint8_t vif_index;
};

struct dbg_reset_mgmt_stats_req {
	uint8_t vif_index;
};

struct dbg_vap_mgmt_tx_stats {
	/**
	 * tx beacon frame count
	 */
	uint64_t tx_bcnpkts;
	/**
	 * tx probe response frame count
	 */
	uint64_t tx_probersppkts;
	/**
	 * tx auth frame count
	 */
	uint64_t tx_authpkts;
	/**
	 * tx deauth frame count
	 */
	uint64_t tx_deauthpkts;
	/**
	 * tx association request fram count
	 */
	uint64_t tx_assocreqpkts;
	/**
	 * tx association response frame count
	 */
	uint64_t tx_assocrsppkts;
	/**
	 * tx reassociation request fram count
	 */
	uint64_t tx_reascreqpkts;
	/**
	 * tx reassociation response frame count
	 */
	uint64_t tx_reascrsppkts;
	/**
	 * tx disassociation frame count
	 */
	uint64_t tx_disassocpkts;
};

struct dbg_vap_mgmt_rx_stats {
	/**
	 * rx probe response frame count
	 */
	uint64_t rx_probersppkts;
	/**
	 * rx auth frame count
	 */
	uint64_t rx_authpkts;
	/**
	 * rx deauth frame count
	 */
	uint64_t rx_deauthpkts;
	/**
	 * rx association request fram count
	 */
	uint64_t rx_assocreqpkts;
	/**
	 * rx association response frame count
	 */
	uint64_t rx_assocrsppkts;
	/**
	 * rx reassociation request fram count
	 */
	uint64_t rx_reascreqpkts;
	/**
	 * rx reassociation response frame count
	 */
	uint64_t rx_reascrsppkts;
	/**
	 * rx disassociation frame count
	 */
	uint64_t rx_disassocpkts;
};

struct dbg_get_mgmt_stats_cfm {
	/**
	 * mgmt frame tx stats
	 */
	struct dbg_vap_mgmt_tx_stats tx_stats;
	/**
	 * mgmt frame rx stats
	 */
	struct dbg_vap_mgmt_rx_stats rx_stats;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For TDLS messages
///////////////////////////////////////////////////////////////////////////////

/// List of messages related to the task.
enum tdls_msg_tag {
	/// TDLS channel Switch Request.
	TDLS_CHAN_SWITCH_REQ = LMAC_FIRST_MSG(TASK_TDLS),
	/// TDLS channel switch confirmation.
	TDLS_CHAN_SWITCH_CFM,
	/// TDLS channel switch indication.
	TDLS_CHAN_SWITCH_IND,
	/// TDLS channel switch to base channel indication.
	TDLS_CHAN_SWITCH_BASE_IND,
	/// TDLS cancel channel switch request.
	TDLS_CANCEL_CHAN_SWITCH_REQ,
	/// TDLS cancel channel switch confirmation.
	TDLS_CANCEL_CHAN_SWITCH_CFM,
	/// TDLS peer power save indication.
	TDLS_PEER_PS_IND,
	/// TDLS peer traffic indication request.
	TDLS_PEER_TRAFFIC_IND_REQ,
	/// TDLS peer traffic indication confirmation.
	TDLS_PEER_TRAFFIC_IND_CFM,
	/// MAX number of messages
	TDLS_MAX
};

/// Structure containing the parameters of the @ref TDLS_CHAN_SWITCH_REQ message
struct tdls_chan_switch_req {
	/// Index of the VIF
	u8_l vif_index;
	/// STA Index
	u16_l sta_idx;
	/// MAC address of the TDLS station
	struct mac_addr peer_mac_addr;
	/// Flag to indicate if the TDLS peer is the TDLS link initiator
	bool initiator;
	/// Channel configuration
	struct mac_chan_op chan;
	/// Operating class
	u8_l op_class;
};

/// Structure containing the parameters of the @ref TDLS_CANCEL_CHAN_SWITCH_REQ message
struct tdls_cancel_chan_switch_req {
	/// Index of the VIF
	u8_l vif_index;
	/// STA Index
	u16_l sta_idx;
	/// MAC address of the TDLS station
	struct mac_addr peer_mac_addr;
};


/// Structure containing the parameters of the @ref TDLS_CHAN_SWITCH_CFM message
struct tdls_chan_switch_cfm {
	/// Status of the operation
	u8_l status;
};

/// Structure containing the parameters of the @ref TDLS_CANCEL_CHAN_SWITCH_CFM message
struct tdls_cancel_chan_switch_cfm {
	/// Status of the operation
	u8_l status;
};

/// Structure containing the parameters of the @ref TDLS_CHAN_SWITCH_IND message
struct tdls_chan_switch_ind {
	/// VIF Index
	u8_l vif_index;
	/// Channel Context Index
	u8_l chan_ctxt_index;
	/// Status of the operation
	u8_l status;
};

/// Structure containing the parameters of the @ref TDLS_CHAN_SWITCH_BASE_IND message
struct tdls_chan_switch_base_ind {
	/// VIF Index
	u8_l vif_index;
	/// Channel Context index
	u8_l chan_ctxt_index;
};

/// Structure containing the parameters of the @ref TDLS_PEER_PS_IND message
struct tdls_peer_ps_ind {
	/// VIF Index
	u8_l vif_index;
	/// STA Index
	u16_l sta_idx;
	/// MAC ADDR of the TDLS STA
	struct mac_addr peer_mac_addr;
	/// Flag to indicate if the TDLS peer is going to sleep
	bool ps_on;
};

/// Structure containing the parameters of the @ref TDLS_PEER_TRAFFIC_IND_REQ message
struct tdls_peer_traffic_ind_req {
	/// VIF Index
	u8_l vif_index;
	/// STA Index
	u16_l sta_idx;
	// MAC ADDR of the TDLS STA
	struct mac_addr peer_mac_addr;
	/// Dialog token
	u8_l dialog_token;
	/// TID of the latest MPDU transmitted over the TDLS direct link to the TDLS STA
	u8_l last_tid;
	/// Sequence number of the latest MPDU transmitted over the TDLS direct link
	/// to the TDLS STA
	u16_l last_sn;
};

/// Structure containing the parameters of the @ref TDLS_PEER_TRAFFIC_IND_CFM message
struct tdls_peer_traffic_ind_cfm {
	/// Status of the operation
	u8_l status;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For FTM messages
///////////////////////////////////////////////////////////////////////////////

/// Maximum number of FTM responders handled in one command
#define FTM_RSP_MAX 5

/// List of messages related to the task.
enum ftm_msg_tag {
	/// Setup a FTM measurement
	FTM_START_REQ = LMAC_FIRST_MSG(TASK_FTM),
	/// Confirmation of the MP start.
	FTM_START_CFM,
	/// FTM measurement available
	FTM_DONE_IND,
	/// MAX number of messages
	FTM_MAX
};

/// Structure containing the parameters of the @ref FTM_START_REQ message.
struct ftm_start_req {
	/// Vif Index
	u8_l fhost_vif_idx;
};

/// Structure containing the parameters of the @ref FTM_START_CFM message.
struct ftm_start_cfm {
	/// Status of the FTM starting procedure
	u8_l status;
	/// Index of the VIF for which the FTM is started
	u8_l vif_idx;
};

/// Structure containing the parameters of the @ref FTM_DONE_IND message.
struct ftm_done_ind {
	/// Index of the VIF for which the FTM is started
	u8_l vif_idx;
	/// Results
	struct mac_ftm_results results;
};

/// Messages that are logically related to the task.
enum {
	IRF_REG_INIT_REQ = LMAC_FIRST_MSG(TASK_IRF),
	IRF_REG_INIT_CFM,
	IRF_REG_WRITE,
	IRF_REG_READ,
	IRF_BUFF_WRITE,
	IRF_BUFF_READ,
	IRF_SAMPLE_CONFIG_REQ,
	IRF_SAMPLE_CONFIG_CFM,
	IRF_SAMPLE_START_REQ,
	IRF_SAMPLE_START_IND,
	IRF_SAMPLE_LMS_REQ,
	IRF_SAMPLE_LMS_IND,
	IRF_SEND_CONFIG_REQ,
	IRF_SEND_CONFIG_CFM,
	IRF_SEND_START_REQ,
	IRF_SEND_START_CFM,
	IRF_SEND_STOP_REQ,
	IRF_SEND_STOP_CFM,
	IRF_WRITE_LMS_REQ,
	IRF_WRITE_LMS_CFM,
	IRF_RUN_TASK_REQ,
	IRF_RUN_TASK_CFM,
	IRF_START_EQ_REQ,
	IRF_START_EQ_CFM,
	IRF_START_EQ_IND,
	IRF_SHOW_TBL_REQ,
	IRF_SHOW_TBL_CFM,
	IRF_SET_MODE_REQ,
	IRF_SET_MODE_CFM,
	IRF_SHOW_STATUS_REQ,
	IRF_SHOW_STATUS_CFM,
	IRF_LOAD_DATA_REQ,
	IRF_LOAD_DATA_CFM,
	IRF_SET_CALC_STEP_REQ,
	IRF_SET_CALC_STEP_CFM,
	IRF_SET_CALIB_EVT_REQ,
	IRF_SET_CALIB_EVT_CFM,
	IRF_SET_CALIB_EVT_IND,
	IRF_START_SCHEDULE_REQ,
	IRF_START_SCHEDULE_CFM,
	IRF_HW_CFG_REQ,
	IRF_HW_CFG_CFM,
	IRF_SEND_EQ_DATA_REQ,
	IRF_SEND_EQ_DATA_CFM,
	IRF_FB_DCOC_REQ,
	IRF_FB_DCOC_DONE_IND,
	IRF_RX_DCOC_REQ,
	IRF_RX_DCOC_DONE_IND,
	IRF_TX_CALI_INIT_REQ,
	IRF_TX_CALI_INIT_CFM,
	IRF_TX_CALI_OFFSET_REQ,
	IRF_TX_CALI_OFFSET_CFM,
	IRF_TX_CALI_SAVE_REQ,
	IRF_TX_CALI_SAVE_IND,
	IRF_RX_CALI_INIT_REQ,
	IRF_RX_CALI_INIT_CFM,
	IRF_RX_CALI_OFFSET_REQ,
	IRF_RX_CALI_OFFSET_CFM,
	IRF_RX_CALI_SAVE_REQ,
	IRF_RX_CALI_SAVE_IND,
	IRF_RX_LEVEL_SET_REQ,
	IRF_RX_LEVEL_SET_IND,
	IRF_XTAL_CALI_REQ,
	IRF_XTAL_CALI_CFM,
	IRF_AFE_CFG_REQ,
	IRF_AFE_CFG_CFM,
	IRF_SET_TASK_PARAM_REQ,
	IRF_SET_TASK_PARAM_CFM,
	IRF_TX_POWER_REQ,
	IRF_TX_POWER_CFM,
	IRF_FB_CALI_OFFSET_REQ,
	IRF_FB_CALI_OFFSET_CFM,
	IRF_SHOW_CAPACITY_REQ,
	IRF_SHOW_CAPACITY_CFM,
	IRF_SET_CALI_BAND_IDX_REQ,
	IRF_SET_CALI_BAND_IDX_CFM,
	IRF_AFE_CMD_REQ,
	IRF_AFE_CMD_CFM,
	IRF_RADAR_DETECT_REQ,
	IRF_RADAR_DETECT_CFM,
	IRF_INTERFERENCE_DETECT_REQ,
	IRF_INTERFERENCE_DETECT_CFM,
	IRF_DIF_CALI_SAVE_REQ,
	IRF_DIF_CALI_SAVE_IND,
	IRF_XTAL_CTRIM_CONFIG_REQ,
	IRF_XTAL_CTRIM_CONFIG_CFM,
	IRF_CCA_CS_CONFIG_REQ,
	IRF_CCA_CS_CONFIG_CFM,
	IRF_CCA_ED_CONFIG_REQ,
	IRF_CCA_ED_CONFIG_CFM,
	IRF_RX_DCOC_SOFT_DBG_REQ,
	IRF_FB_ERR_CALI_REQ,
	IRF_FB_ERR_CALI_CFM,
	IRF_FB_ERR_CALI_IND,
	IRF_TX_FCOMP_CALI_REQ,
	IRF_TX_FCOMP_CALI_CFM,
	IRF_TX_CUR_PWR_REQ,
	IRF_TX_CUR_PWR_CFM,
	IRF_TX_LOOP_PWR_REQ,
	IRF_TX_LOOP_PWR_CFM,
	IRF_RX_GAIN_LVL_CALI_REQ,
	IRF_RX_CALI_DONE_IND,
	IRF_RX_GAIN_FREQ_CALI_REQ,
	IRF_CALI_DBG_LVL_REQ,
	IRF_TX_ERR_CALI_REQ,
	IRF_TX_ERR_CALI_CFM,
	IRF_TX_ERR_CALI_IND,
	IRF_TX_LOOP_PWR_INIT_REQ,
	IRF_TX_LOOP_PWR_INIT_CFM,
	IRF_CHECK_ALM_REQ,
	IRF_CHECK_ALM_CFM,
	IRF_PPPC_SWITCH_REQ,
	IRF_PPPC_SWITCH_CFM,
	IRF_AGC_RELOAD_REQ,
	IRF_AGC_RELOAD_CFM,
	IRF_TH_WALL_REQ,
	IRF_TH_WALL_CFM,
	IRF_SET_ACI_DET_PARA_REQ,
	IRF_GET_ACI_DET_PARA_REQ,
	IRF_SET_RX_GAIN_CALI_PARA_REQ,
	IRF_GET_RX_GAIN_CALI_PARA_REQ,
	IRF_RX_GAIN_CALI_PREP_REQ,
	IRF_RX_GAIN_BIST_CALI_REQ,
	IRF_RX_GAIN_IMD3_TEST_REQ,
	IRF_SET_PWR_CTRL_THRE_REQ,
	IRF_GET_PWR_CTRL_THRE_REQ,
	IRF_SET_PWR_PREC_OFFSET_REQ,
	IRF_GET_PWR_PREC_OFFSET_REQ,
	IRF_SET_COMP_STUB_BITMAP_REQ,
	IRF_GET_COMP_STUB_BITMAP_REQ,
	IRF_SET_DIGITAL_TX_GAIN_REQ,
	/// MAX number of messages
	IRF_MAX
};

struct irf_hw_init_req {
	u32_l bw;   //0:20M,1:40M,2:80M,3:160M
	u32_l bitmap;
};

struct irf_hw_init_cfm {
	u8_l status;
};

struct irf_smp_start_req {
	u32_l node_mask;
	u32_l timeout;
};

struct irf_smp_config_req {
	u32_l node;
	u32_l mux_data;
	u32_l mux_trg;
	u32_l len;
	u32_l width;
	u32_l sync;
	u32_l interval;
	u32_l trg_sel;
	u32_l delay;
	u32_l update;
	u32_l band;
	u32_l channel;
	u32_l irf_smp_buf_addr;
	u32_l smp_trg;
	u32_l smp_mod;
};

#define irf_smp_lms_req irf_smp_config_req
#define irf_smp_config_cfm irf_hw_init_cfm

struct irf_send_start_req {
	s32_l node_mask;
	u32_l band;
	u32_l channel;
	s32_l src_node;
};

struct irf_send_stop_req {
	struct irf_send_start_req send_stop_req;
	u8_l wmac_handle_flag;
};

#define irf_send_start_cfm irf_hw_init_cfm
#define irf_send_stop_cfm irf_hw_init_cfm

struct irf_xtal_ctrim_config_req {
	uint8_t ctrim;
};

struct irf_xtal_ctrim_config_cfm {
	int8_t status;
};

struct irf_cca_cs_config_req {
	u8_l get_config;
	s8_l inbdpow1_pupthr;
	s8_l cca_delta;
};

struct irf_cca_cs_config_cfm {
	int8_t status;
	int8_t inbdpow1_pupthr;
};

struct irf_cca_ed_config_req {
	u8_l get_config;
	u8_l cca20p_risethr;
	u8_l cca20p_fallthr;
	u8_l cca20s_risethr;
	u8_l cca20s_fallthr;
	u8_l cca40s_risethr;
	u8_l cca40s_fallthr;
	u8_l cca80s_risethr;
	u8_l cca80s_fallthr;
};

struct irf_cca_ed_config_cfm {
	int8_t status;
	int8_t cca20p_risethr;
	int8_t cca20p_fallthr;
	int8_t cca20s_risethr;
	int8_t cca20s_fallthr;
	int8_t cca40s_risethr;
	int8_t cca40s_fallthr;
	int8_t cca80s_risethr;
	int8_t cca80s_fallthr;
};

#define IRF_SEND_CFG_MAX_LEN (0x100000)
#define IRF_DDR_SEND_CFG_MAX_LEN (0x7fffff)

struct irf_send_config_req {
	u32_l node;
	u32_l interval;
	u32_l trg_sel;
	u32_l delay;
	u32_l len;
	u32_l loop;
	u32_l ppdu_front_len;
	u32_l ppdu_back_len;
	u32_l ppdu_gap_len;
	u32_l width;
	u32_l irf_send_buf_addr;
	u32_l dat_vld;
	u32_l snd_mod;
};

#define irf_send_config_cfm irf_hw_init_cfm

#define IRF_MAX_SMP_DESC_NUM 4

struct irf_smp_addr_desc {
	u32_l irf_smp_buf_addr;
	u32_l len;
	u32_l node;
};

struct irf_smp_start_ind {
	u8_l status;
	u8_l sel_bitmap;
	struct irf_smp_addr_desc smp_addr_desc[IRF_MAX_SMP_DESC_NUM];
	u8_l multi_phase;
	u8_l extend_ram_flag;
};

struct irf_smp_lms_ind {
	u8_l status;
	u32_l irf_smp_buf_addr;
};

#define LMS_MAX_LEN 60
struct irf_lms_desc {
	u32_l lms_node;
	u32_l update_node;
	u32_l len; //word
	u32_l lms_data[LMS_MAX_LEN];
	u32_l update_data[LMS_MAX_LEN];
};

struct irf_write_lms_req
{
	u32_l channel;
	u32_l irf_lms_data_addr;
	u32_l len;
};

#define irf_write_lms_cfm irf_hw_init_cfm
#endif /*__KERNEL__*/
///////////////////////////////////////////////////////////////////////////////
/////////// For IRF messages
///////////////////////////////////////////////////////////////////////////////

struct irf_hw_cfg_req
{
	uint8_t bw;  // 0:20MHz,1:40MHz,2:80MHz,3:160MHz
	uint8_t ant_mask; // BIT0-ant0,BIT1-ant1...
	int8_t tx_power_level; //E.g. 0x80(-128 dBm), 0xFF(-1 dBm), 0x00(0 dBm), 0x1(1 dBm), 0x7F(127dBm).
	uint8_t mode_11b;
	uint16_t prim20_freq; // Frequency for Primary 20MHz channel (in MHz)
};


struct irf_set_mode_req{
	uint8_t mode;
};

#ifdef __KERNEL__
struct irf_show_tbl_req{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t table_type;
	uint32_t channel;
	uint32_t freq;
	uint32_t bw;
	uint32_t power_stage;
};

struct irf_show_req{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;
	uint32_t type;
	uint32_t param;
};


#define IRF_TASK_LEN_MAX        256
struct irf_run_task_req{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;
	uint32_t run_time;
	char task_list[IRF_TASK_LEN_MAX];
};

struct irf_dif_eq_req
{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;
	uint32_t freq;
	uint32_t bw;
	uint32_t stage;
	uint32_t cali_mask;  //bit0:tx-fb, bit1: tx-rx
};

struct irf_dif_eq_save_req
{
    uint32_t radio_id;   //0--2.4G, 1--5G/6G
};

struct irf_dif_eq_ind
{
	uint8_t status;
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t data_size; //equipment table actual size
};

struct irf_load_data_req{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t data_type;
	uint32_t data_addr;
	uint32_t data_size;
};
struct irf_load_data_cfm {
	int32_t status;
};

struct irf_set_calc_step_req
{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	char name[32];
	int step;
};
struct irf_set_calc_step_cfm {
	int32_t status;
};

struct irf_set_cali_evt_req
{
    uint32_t radio_id;   //0--2.4G, 1--5G/6G
    uint32_t evt_cmd;
    uint32_t task_type;
};

struct irf_set_cali_evt_ind {
    uint8_t status;
    uint32_t radio_id;   //0--2.4G, 1--5G/6G
    uint32_t evt_cmd;
    uint32_t dif_sm_state;
    uint32_t cause;
	bool fbdelay_drop_flag;
	bool pd_task_success_flag;
};

#define irf_set_cali_evt_cfm  irf_set_cali_evt_ind


struct irf_start_schedule_req
{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t run_type;  //0--stop,  1--dif self test data, 2--cali data
};

struct irf_start_schedule_cfm {
	int32_t status;
};

struct irf_send_eq_data_req{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;
	uint32_t data_type;
	uint32_t stop;
};


struct vbss_sta_seq_num {
	uint16_t sta_idx;
	uint32_t tx_pn;
	uint32_t rx_pn[TID_MAX];
	uint32_t tx_seq_num[TID_MAX];
	uint32_t rx_seq_num[TID_MAX];
};

struct vbss_get_sta_seq_num_req {
	uint32_t sta_idx;
};

struct vbss_get_sta_seq_num_cfm {
	uint32_t status;
	struct vbss_sta_seq_num seq_info;
};

struct vbss_set_sta_seq_num_req {
	struct vbss_sta_seq_num seq_info;
};

struct vbss_set_sta_seq_num_cfm {
	uint32_t status;
};

struct vbss_cca_report_ind {
	/* Current working channel */
	struct mac_chan_op chan;
	/* Cahnnel duration */
	uint32_t chan_dur;
	/* CCA busy time (us) on each subchannel */
	uint32_t cca_busy_dur;
	uint32_t cca_busy_dur_sec20;
	uint32_t cca_busy_dur_sec40;
	uint32_t cca_busy_dur_sec80;
};

struct cdf_stats {
	uint8_t used;
	uint8_t offset;
	uint8_t index;
	uint8_t mask;
	uint32_t reg_addr;
	uint32_t dump;
	uint32_t ops;
};


struct irf_fb_dcoc_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t high_temp;
};

struct irf_fb_dcoc_done_ind {
	u8_l status;
	u32_l radio_id;
	u32_l high_temp;
	u32_l fb_dcoc_addr;
	u32_l fb_dcoc_size;
};

struct irf_xtal_cali_req {
	uint8_t mode;
	uint8_t thd;
	uint8_t init_step;
	int32_t freq_err;
	int32_t ppm_err;
};

struct irf_xtal_cali_cfm {
	uint8_t status;
	uint8_t xtal_cal_status;
	struct irf_tbl_head head;
	uint8_t ctrim;
};

struct irf_set_cali_param_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	int8_t gain_comp;   //unit: 0.1db
	int16_t tx_act_pwr; //unit: 0.1db
};

struct irf_set_tx_power_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;    //0--ch0, 1--ch1
	int8_t tx_power;   //unit: 1db
};

struct irf_set_pppc_switch_req {
	uint32_t radio_id;
	uint8_t pppc_switch;
};

struct irf_check_alm_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;    //0--ch0, 1--ch1
};

struct irf_th_wall_req {
	uint32_t radio_id;  // 0--2.4G, 1--5G/6G
#define IRF_SET_TH_WALL 0
#define IRF_GET_TH_WALL 1
	uint8_t option;     // IRF_SET_TH_WALL or IRF_GET_TH_WALL
	uint8_t enable;     // 0--disable, 1--enable
};

struct irf_th_wall_cfm {
	uint32_t status;  // 0--2.4G, 1--5G/6G
	uint8_t enable;   // 0--disable, 1--enable
};

struct irf_tx_cali_save_ind
{
	uint8_t status;
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t tx_data_addr;
	uint32_t tx_data_size; //equipment table actual size
	uint32_t fb_data_addr;
	uint32_t fb_data_size; //equipment table actual size
};

struct irf_rx_cali_save_ind
{
	uint8_t status;
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t rx_data_addr;
	uint32_t rx_data_size; //equipment table actual size
	uint8_t rx_cali_type;
};

struct irf_set_rx_gain_lvl_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	int8_t gain;
};

struct irf_afe_cfg_req {
	/* AFE module id */
	uint32_t mod_id;
	/* AFE sub-module or sub-function id */
	uint32_t sub_mod_id;
	uint32_t radio_id;
	uint32_t chan;
	uint32_t bw;
	uint32_t freq;
	uint32_t val;
	uint32_t tsensor_id;
};

struct irf_afe_cmd_req {
	/* BAND_2G/5G */
	uint32_t radio_id;
	/* cmd cnt */
	uint32_t argc;
	/* afe comand str spite with space */
	char cmd_lines_str[256];
};

struct irf_radar_detect_req {
	bool enable;
	uint32_t phymode;
	uint32_t trgt_pulse_num;
	uint32_t cac_mode;
};

struct irf_interference_detect_req {
	bool get_result;
	uint32_t phymode;
	uint32_t hw_det;
};

struct irf_interference_detect_cfm {
	int32_t status;
};

struct irf_task_param_req
{
	char name[32];
	uint32_t para_type;
	uint32_t gate;
	uint32_t train_len;
	int32_t eq_coef_fact;
	int16_t min_d_thrshld;
	int16_t min_d_coef_adj;
	int16_t max_d_thrshld;
	int16_t max_d_coef_adj;
	int32_t cgl_calc_len;
	int32_t c_fact;
	int32_t ff_noise_coef;
	int32_t fb_noise_coef;
};

struct irf_show_capacity_req
{
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
};

struct irf_set_subband_idx_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint8_t subband_idx;
};

struct irf_gain_err_cali_ind
{
	uint8_t status;
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t gain_err_data_addr;
	uint32_t gain_err_data_size; //equipment table actual size
};

struct irf_tx_fcomp_cali_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	int8_t fb_gain_level;
	uint8_t band_idx;
};

struct irf_tx_cur_pwr_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	int16_t tx_act_pwr;
};

struct irf_rx_gain_lvl_cali_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	/* unit 0.1dBm */
	int16_t in_pwr[IRF_MAX_ANT_NUM];
	uint8_t chan_cali_mod;
	uint8_t gain_intv_idx;
};

struct irf_rx_cali_done_ind
{
	uint8_t status;
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	uint8_t rx_cali_type;
};

struct irf_rx_gain_freq_cali_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	int32_t gain;
	uint8_t band_idx;
};

struct irf_agc_reload_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t agc_len;    //agc bin length
};

struct irf_agc_reload_cfm {
	uint8_t status;   //0: OK, non 0: failure
};

struct irf_gain_dbg_lvl_req {
	uint32_t dbg_lvl;
};

enum {
	RX_GAIN_CALI_ILNA_OUT_PWR = 0,
	RX_GAIN_CALI_TX_RX_PWR_ERR,
	RX_GAIN_CALI_IMD3_THRS,
	RX_GAIN_CALI_UNUSED_ILNA_LST,
	RX_GAIN_CALI_IMD3_CALC_ILNA_LST,
	RX_GAIN_CALI_INPUT_PWR_LST,
};

struct irf_rx_gain_cali_ilna_out_pwr_req {
	int8_t gain_lvl;
	/* unit 0.1dBm */
	int16_t out_pwr;
	uint8_t res;
};

struct irf_rx_gain_cali_tx_rx_pwr_err_req {
	int16_t pwr_err;
	uint8_t res[2];
};

struct irf_rx_gain_cali_imd3_thrs_req {
	int16_t thrs_val;
	uint8_t res[2];
};

struct irf_rx_gain_cali_unused_ilna_lst_req {
	char ilna_lst[32];
};

struct irf_rx_gain_cali_imd3_calc_ilna_lst_req {
	char ilna_lst[32];
};

struct irf_rx_gain_cali_para_req {
	uint32_t radio_id;
	uint32_t para_type;
	union {
		struct irf_rx_gain_cali_ilna_out_pwr_req ilna_out_pwr;
		struct irf_rx_gain_cali_tx_rx_pwr_err_req tx_rx_pwr_err;
		struct irf_rx_gain_cali_imd3_thrs_req imd3_thrs;
		struct irf_rx_gain_cali_unused_ilna_lst_req unused_ilna_lst;
		struct irf_rx_gain_cali_imd3_calc_ilna_lst_req imd3_calc_ilna_lst;
	};
};

#define irf_rx_gain_cali_prep_req irf_show_capacity_req

enum {
	RX_GAIN_SIG_ANT_CALI = 0,
	RX_GAIN_MULT_ANT_CALI,
};

struct irf_rx_gain_lvl_bist_cali_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	uint8_t chan_cali_mod;
};

struct irf_rx_gain_imd3_test_req {
	uint32_t radio_id;   //0--2.4G, 1--5G/6G
	uint32_t channel;	//0--ch0, 1--ch1
	int8_t gain;
	/* unit 0.1dBm */
	int16_t in_pwr;
};

struct irf_pwr_ctrl_thre_req {
	uint32_t radio_id;
	int lower_thre;
	int upper_thre;
};

struct irf_pwr_prec_offset_req {
	uint32_t radio_id;
	int pwr_prec_offset;
};

struct irf_comp_stub_bitmap_req {
	uint32_t radio_id;
	uint8_t bitmap;
};


struct irf_set_digital_tx_gain_req {
	uint32_t radio_id;
	uint32_t channel;
	int gain;    //unit 0.1 dBm
};

struct mm_atf_params_req {
	/* 0: invalid; 1: enable; 2: disable; 3: update params; 4: update quota */
	uint8_t command;

	uint8_t enable;
	uint8_t mode;
	uint8_t srrc_enable;
	uint32_t sched_period_us;
	uint32_t stats_period_us;
	uint32_t stats_busy_thres_us;
	uint32_t stats_clear_thres_us;
	uint32_t stats_clear_duration_us;
};

struct mm_atf_params_cfm {
	uint32_t status;
};

struct irf_aci_det_para_req {
	uint32_t radio_id;
	int16_t rssi_thrs;
	/* unit: 0.1db */
	int16_t aci_pwr_thrs;
	int16_t pos_freq_beg_idx;
	int16_t pos_freq_end_idx;
	int16_t neg_freq_beg_idx;
	int16_t neg_freq_end_idx;
	int16_t intf_freq_beg_idx;
	int16_t intf_freq_end_idx;
};

///////////////////////////////////////////////////////////////////////////////
/////////// For DHT messages
///////////////////////////////////////////////////////////////////////////////
#define DHT_KEMSG_NUM(num)   (LMAC_FIRST_MSG(TASK_DHT) + (num))

/// Message API of the DHT task
enum {
	DHT_D2K_SOFT_RESET_REQ = DHT_KEMSG_NUM(0x31),
	DHT_D2K_SET_AMPDU_PRT_REQ = DHT_KEMSG_NUM(0x44),
	DHT_D2K_SET_TXOP_EN_REQ = DHT_KEMSG_NUM(0x45),
	DHT_RTS_CTS_DBG_REQ = DHT_KEMSG_NUM(0x4b),
	DHT_DUMP_EDMA_INFO_REQ = DHT_KEMSG_NUM(0x4c),
	DHT_SET_AMPDU_MAX_REQ = DHT_KEMSG_NUM(0x60),
	DHT_SET_M3K_BOC_REG_REQ = DHT_KEMSG_NUM(0x61),
	DHT_SET_SMM_IDX_DBG_REQ = DHT_KEMSG_NUM(0x73),
	DHT_LOG_TO_UART_REQ = DHT_KEMSG_NUM(0x74),

	DHT_SET_CMCC_PPPC_REQ = DHT_KEMSG_NUM(0x80),
	DHT_DUMP_CMCC_PPPC_TXPWR_RECORD_REQ = DHT_KEMSG_NUM(0x81),
	DHT_SYNC_CMCC_PPPC_TXPWR_REQ = DHT_KEMSG_NUM(0x82),

	DHT_CDF_STATS_REQ = DHT_KEMSG_NUM(0x85),
	DHT_TX_RX_LOOP_ONLINE_REQ = DHT_KEMSG_NUM(0x86),
	DHT_WMM_LOCK_REQ = DHT_KEMSG_NUM(0x9C),
	DHT_SET_DYN_PWR_OFFSET_REQ = DHT_KEMSG_NUM(0xA1),
};

struct dht_force_reset_hw_req_op {
#define DHTF_RST_HW_NONE   0
#define DHTF_RST_HW_RST_A  1
#define DHTF_RST_HW_RST_B  2
#define DHTF_RST_HW_RST_C  3
#define DHTF_RST_HW_RST_D  4
#define DHTF_RST_HW_RST_E  5
#define DHTF_RST_HW_RST_F  6
	uint8_t reset_type;

#define DHTF_WMAC_STATE_POS          0
#define DHTF_WMAC_STATE_MASK         (0x7 << DHTF_WMAC_STATE_POS)
#define DHTF_WMAC_STATE_DEF          0x0
#define DHTF_WMAC_STATE_IDLE         0x1
#define DHTF_WMAC_STATE_DONZ         0x2
#define DHTF_WMAC_STATE_ACTIVE       0x3
#define DHTF_WMAC_STATE_KEEP_RESET   0x4

#define DHTF_COMM_RESET_POS         4
#define DHTF_COMM_RESET_MASK        (0xF << DHTF_COMM_RESET_POS)
#define DHTF_COMM_RESET_SMP         (1 << (DHTF_COMM_RESET_POS))
#define DHTF_COMM_RESET_DPD_FB_LMS  (1 << (DHTF_COMM_RESET_POS + 1))
#define DHTF_COMM_RESET_INTFDET     (1 << (DHTF_COMM_RESET_POS + 2))
#define DHTF_COMM_RESET_BISTFFT     (1 << (DHTF_COMM_RESET_POS + 3))
	uint8_t reset_sub_type;

#define DHTF_SEND_MSG_OP           (1U << 0)
#define DHTF_DBG_CB_OP             (1U << 1)
	uint16_t op;
};

struct tx_rx_loop_online_en_req {
	uint8_t tx_rx_loop_online_en;
};

struct dht_set_ampdu_prt_req {
	uint32_t type;
};

struct dht_set_txop_en_req {
	uint32_t enable;
};

struct dht_set_ampdu_max_req {
	uint32_t ampdu_max_size;
};

struct dht_set_smm_idx_req {
	uint32_t smm_idx;
};

struct dht_set_pppc_req
{
    uint32_t enable;
    uint32_t with_param;
    uint32_t format;
    uint32_t bw;
    uint32_t start_mcs;
    uint32_t end_mcs;
    uint32_t txpower;
};

struct dht_wmm_lock_req {
	uint32_t enable;
};

struct dht_log_to_uart_req {
	uint32_t enable;
};

struct dht_rts_cts_dbg_req
{
	uint32_t enable_rts_cts_dump;
};

struct dht_dump_edma_info_req {
	uint32_t command;
};

struct dht_m3k_boc_reg_req {
	uint32_t enable;
};

struct dht_dyn_pwr_offset_req {
	int8_t offset;
};

struct mm_csi_params_req {
	uint8_t command;
	uint8_t enable;
	uint8_t log_level;
	uint8_t smooth;
	uint16_t sta_idx;
	uint16_t hw_key_idx;
	uint8_t mac_addr[6];
	/* period(ms) */
	uint16_t period;
	uint32_t format_mask;
};

struct mm_csi_params_cfm {
	uint32_t command;
	uint32_t status;
};

struct mm_rd_max_num_thrd_req {
#define RD_MAX_NUM_THRD_OPT_SET 0
#define RD_MAX_NUM_THRD_OPT_GET 1
	uint8_t option;
	uint8_t rd_max_num_thrd;
};

struct mm_rd_max_num_thrd_cfm {
	uint8_t rd_max_num_thrd;
	uint32_t status;
};

struct mm_rd_chan_req {
#define RD_CHAN_OPT_SET 0
#define RD_CHAN_OPT_GET 1
	uint8_t option;
	uint8_t channel;
};

struct mm_rd_chan_cfm {
	uint8_t channel;
	uint32_t status;
};

struct mm_rd_dbg_req {
	uint8_t rd_dbg_level;
};

struct mm_rd_dbg_cfm {
	uint32_t status;
};

struct mm_rd_agc_war_req {
#define RD_AGC_WAR_OPT_SET 0
#define RD_AGC_WAR_OPT_GET 1
	uint8_t option;
	uint8_t enable;
};

struct mm_rd_agc_war_cfm {
	uint8_t enable;
	uint32_t status;
};

#define MM_MEM_REQ_LEN_MAX (IPC_A2E_MSG_BUF_SIZE - sizeof(struct lmac_msg) / sizeof(uint32_t) - 12)
#define MM_MEM_CFM_LEN_MAX (IPC_E2A_MSG_PARAM_SIZE - 4)
#define MM_MEM_SET_LEN_MAX 0xFFFFFFFF

enum MM_MEM_OP {
	MM_MEM_OP_WRITE = 1,
	MM_MEM_OP_SET = 2,
	MM_MEM_OP_READ = 3,
};

enum MM_MEM_REGION {
	MM_MEM_REGION_WIFI = 1,
	MM_MEM_REGION_IRAM = 2,
};

enum {
	DPD_WMAC_TX_RANDOM,
	DPD_WMAC_TX_FIXED,
	DPD_WMAC_TX_FROM_FILE,
};

struct mm_mem_params_req {
	uint8_t radio_id;
	uint8_t op;
	uint8_t region;
	uint8_t isend;
	uint32_t offset;
	uint32_t len;
	uint32_t payload[MM_MEM_REQ_LEN_MAX];
};

struct mm_mem_params_cfm {
	uint32_t status;
	uint32_t payload[MM_MEM_CFM_LEN_MAX];
};

struct mm_dpd_wmac_tx_params_req {
	/* Numbe of PPDU to be sent */
	uint16_t ppdu_num;
	/* interval between consecutive PPDU (us) */
	uint16_t ppdu_interval;
	uint8_t tx_power;
	/* NSS (high 4 bits), MCS (low 4 bits) */
	uint8_t nss_mcs;
	uint8_t bw;
	/* 0: local random; 1: fixed 0xa5; 2: from file */
	uint8_t source;
	uint16_t mpdu_num;
	uint16_t mpdu_payload_size;
	uint32_t mpdu_ddr_addr[48];
};

struct mm_dpd_wmac_tx_ind {
	uint32_t status;
};

/// Structure containing the parameters of the @ref MM_DL_PARAMETERS_REQ message.
struct bf_parameters_req {
	uint8_t command;
	/* 0: disabled, 1=snd enabled, 2 sound/bf enabled */
	uint8_t enabled;
	/* 0=auto; 1=one-shot, 2=periodical */
	uint8_t snd_mode;
	/* The max number of STAs in one sounding */
	uint8_t max_grp_user;
	/* max number of STA can transmit with beamforming */
	uint8_t max_snd_sta;
	/* firmware print log level */
	uint8_t snd_log_level;
	uint8_t bf_log_level;

	/* Sounding related parameters */
	uint8_t feedback_type;
	uint8_t snapshot;
	uint8_t ndp_power;
	uint8_t ndp_gi;
	uint8_t ndp_bw;
	uint8_t ndp_time_csd;
	uint8_t ndp_smm_idx;

	/* BF related parameters */
	uint8_t support_2ss;
	uint8_t enable_smooth;
	uint8_t enable_fiter;
	uint16_t alpha;

	/* the period to trigger sounding (us) */
	uint32_t snd_period;
	/* the lifetime of cbf report before aging out (us) */
	uint32_t cbf_lifetime;
};

struct bf_parameters_cfm {
	uint32_t status;
};

struct mm_smart_antenna_req {
	// bit0: enable; bit 1:update mode(0:sw, 1:hw); bit(2-3):gpio mode
	uint8_t enable;
	uint32_t curval_hi;
	uint32_t curval_lo;
	uint32_t rstval_hi;
	uint32_t rstval_lo;
};

struct mm_smart_antenna_cfm {
	uint32_t status;
};
#endif /*__KERNEL__*/

#endif // _LMAC_MSG_H_
