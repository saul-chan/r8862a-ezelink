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

#ifndef _IPC_SHARED_H_
#define _IPC_SHARED_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "lmac_mac.h"
/*
 * DEFINES AND MACROS
 ****************************************************************************************
 */
#define CO_BIT(pos) (1U<<(pos))

/* FIXME: are there confliction with the values defined in cls_wifi_core.h? */
#define IPC_TXQUEUE_CNT	 CLS_TXQ_CNT

/*
 * Number of shared descriptors available for Data RX handling
 */
//#define IPC_RXDESC_CNT		(CLS_MU_USER_MAX * 128)

#define IPC_RXBUF_CNT_NR(_MU_USER)	(_MU_USER * 128)
#define IPC_RXDESC_CNT_NR(_MU_USER)	(_MU_USER * 128)

/*
 * Number of Host buffers available for Radar events handling (through DMA)
 */
#define IPC_RADARBUF_CNT	16

/*
 * Number of Host buffers available for HE MU DL Map events handling (through DMA)
 */
#define IPC_HEMUBUF_CNT		16

/*
 * Number of Host buffers available for unsupported Rx vectors handling (through DMA)
 */
#define IPC_UNSUPRXVECBUF_CNT	8

/*
 *  Size of RxVector
 */
#define IPC_RXVEC_SIZE		16

/*
 * Number of Host buffers available for Emb->App MSGs sending (through DMA)
 */
#if defined(MERAK2000)
#define IPC_MSGE2A_BUF_CNT	64
#else
///1~65535
#define IPC_MSGE2A_BUF_CNT	1024
#endif
/*
 * Number of Host buffers available for Debug Messages sending (through DMA)
 */
#if defined(MERAK2000)
#define IPC_DBGBUF_CNT		32
#define CMN_DBG_CNT         4
#define CMN_E2A_MSGBUF_NUM  (4)
#define CMN_MSGBUF_SIZE     (16/4)
#define CMN_RXBUF_CNT_2G      4
#define CMN_RXDESC_CNT_MAX_2G  4
#define CMN_RXBUF_CNT_5G      4
#define CMN_RXDESC_CNT_MAX_5G  4
#else
#define IPC_DBGBUF_CNT		1024
#define CMN_DBG_CNT         32
#define CMN_E2A_MSGBUF_NUM  (64)
#define CMN_MSGBUF_SIZE     (2048/4)
#define CMN_RXBUF_CNT_2G       256
#define CMN_RXDESC_CNT_MAX_2G  256
#define CMN_RXBUF_CNT_5G       512
#define CMN_RXDESC_CNT_MAX_5G  512
#endif

/*
 * Number of CSI buffers available for CSI events handling (through DMA)
 */
#define IPC_CSIBUF_CNT		10

/*
 * Length used in MSGs structures
 */
#define IPC_A2E_MSG_BUF_SIZE	127 // size in 4-byte words
#define IPC_E2A_MSG_SIZE_BASE	256 // size in 4-byte words
#define IPC_E2A_MSG_PARAM_SIZE	IPC_E2A_MSG_SIZE_BASE

/*
 * Debug messages buffers size (in bytes)
 */
#define IPC_DBG_PARAM_SIZE	256

/*
 * Define used for Rx hostbuf validity.
 * This value should appear only when hostbuf was used for a Reception.
 */
#define RX_DMA_OVER_PATTERN         0xAAAAAA00
#define RX_DMA_OVER_PATTERN_LST     0xCACACA00

/*
 * Define used for MSG buffers validity.
 * This value will be written only when a MSG buffer is used for sending from Emb to App.
 */
#define IPC_MSGE2A_VALID_PATTERN	0xADDEDE2A

/*
 * Define used for Debug messages buffers validity.
 * This value will be written only when a DBG buffer is used for sending from Emb to App.
 */
#define IPC_DBG_VALID_PATTERN	0x000CACA0
#define IPC_DBG_CMN_VALID_PATTERN	0x000CACA1

/*
 *  Length of the receive vectors, in bytes
 */
#define DMA_HDR_PHYVECT_LEN	36

/*
 * Maximum number of payload addresses and lengths present in the descriptor
 */
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
#define CLS_TX_PAYLOAD_MAX	7
#else
#define CLS_TX_PAYLOAD_MAX	1
#endif

/*
 * Maximum number of MSDUs supported in one received A-MSDU
 */
#define CLS_MAX_MSDU_PER_RX_AMSDU	8

/*
 * Message struct/ID API version
 */
#define MSG_API_VER		63

/*
 ****************************************************************************************
 */
/// Descriptor filled by the Host
struct hostdesc {
	u32_l hostid;
	/// Pointers to packet payloads
	u32_l packet_addr[CLS_TX_PAYLOAD_MAX];
	/// Sizes of the MPDU/MSDU payloads
	u16_l packet_len[CLS_TX_PAYLOAD_MAX];
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
	/// Number of payloads forming the MPDU
	u8_l packet_cnt;
#endif //CONFIG_CLS_WIFI_SPLIT_TX_BUF

#if CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD == 1
	/// Destination Address
	struct mac_addr eth_dest_addr[CLS_TX_PAYLOAD_MAX];
	/// Source Address
	struct mac_addr eth_src_addr[CLS_TX_PAYLOAD_MAX];
	/// Ethernet Type
	u16_l ethertype[CLS_TX_PAYLOAD_MAX];
#else
	/// Destination Address
	struct mac_addr eth_dest_addr;
	/// Source Address
	struct mac_addr eth_src_addr;
	/// Ethernet Type
	u16_l ethertype;
#endif
	/// TX flags (@ref txu_cntrl_flags)
	u16_l flags;
	/// Sequence number to use for the transmission if flag TXU_CNTRL_REUSE_SN is set
	u16_l sn_for_retry;
	/// Packet TID (0xFF if not a QoS frame)
	u8_l tid;
	/// Interface Id
	u8_l vif_idx;
	/// Station Id (0xFF if station is unknown)
	u16_l sta_idx;
#ifdef CONFIG_CLS_WIFI_HEMU_TX
	/// RUA map index
	u8_l rua_map_idx;
	/// User index within the RUA map
	u8_l user_idx;
#endif
};

struct txdesc_api {
	/// Information provided by Host
	struct hostdesc host;
};

/// Additional temporary control information passed from the host to the emb regarding
/// the TX descriptor
struct txdesc_ctrl {
	/// HW queue in which the TX descriptor shall be pushed
	u32_l hwq;
};

/// Descriptor used for Host/Emb TX frame information exchange
struct txdesc_host {
	/// API of the embedded part
	struct txdesc_api api;

	/// Additional control information about the descriptor
	struct txdesc_ctrl ctrl;

	/// Flag indicating if the TX descriptor is ready (different from 0) or not (equal to 0)
	/// Shall be the last element of the structure, i.e. downloaded at the end
	u32_l ready;
};

struct dma_desc {
	u32_l			src;
	u32_l			dest;
	u16_l			length;
	u16_l			ctrl;
	u32_l			next;
};

// Comes from la.h
/// Length of the configuration data of a logic analyzer
#define LA_CONF_LEN		  10

/// Structure containing the configuration data of a logic analyzer
struct la_conf_tag {
	u32_l conf[LA_CONF_LEN];
	u32_l trace_len;
	u32_l diag_conf;
};

/// Size of a logic analyzer memory
#define LA_MEM_LEN	   (1024 * 1024)

/// Type of errors
enum {
	/// Recoverable error, not requiring any action from Upper MAC
	DBG_ERROR_RECOVERABLE = 0,
	/// Fatal error, requiring Upper MAC to reset Lower MAC and HW and restart operation
	DBG_ERROR_FATAL
};

/// Maximum length of the SW diag trace
#define DBG_SW_DIAG_MAX_LEN   1024

/// Maximum length of the error trace
#define DBG_ERROR_TRACE_SIZE  256

/// Number of MAC diagnostic port banks
#define DBG_DIAGS_MAC_MAX	 48

/// Number of PHY diagnostic port banks
#define DBG_DIAGS_PHY_MAX	 32

/// Maximum size of the RX header descriptor information in the debug dump
#define DBG_RHD_MEM_LEN	  (5 * 1024)

/// Maximum size of the RX buffer descriptor information in the debug dump
#define DBG_RBD_MEM_LEN	  (5 * 1024)

/// Maximum size of the TX header descriptor information in the debug dump
#define DBG_THD_MEM_LEN	  (10 * 1024)

/// Structure containing the information about the PHY channel that is used
struct phy_channel_info {
	/// PHY channel information 1
	u32_l info1;
	/// PHY channel information 2
	u32_l info2;
};

/// Debug information forwarded to host when an error occurs
struct dbg_debug_info_tag {
	/// Type of error (0: recoverable, 1: fatal)
	u32_l error_type;
	/// Pointer to the first RX Header Descriptor chained to the MAC HW
	u32_l rhd;
	/// Size of the RX header descriptor buffer
	u32_l rhd_len;
	/// Pointer to the first RX Buffer Descriptor chained to the MAC HW
	u32_l rbd;
	/// Size of the RX buffer descriptor buffer
	u32_l rbd_len;
	/// Pointer to the first TX Header Descriptors chained to the MAC HW
	u32_l thd[CLS_TXQ_CNT];
	/// Size of the TX header descriptor buffer
	u32_l thd_len[CLS_TXQ_CNT];
	/// MAC HW diag configuration
	u32_l hw_diag;
	/// Error message
	u32_l error[DBG_ERROR_TRACE_SIZE/4];
	/// SW diag configuration length
	u32_l sw_diag_len;
	/// SW diag configuration
	u32_l sw_diag[DBG_SW_DIAG_MAX_LEN/4];
	/// PHY channel information
	struct phy_channel_info chan_info;
	/// Embedded LA configuration
	struct la_conf_tag la_conf[2];
	/// MAC diagnostic port state
	u16_l diags_mac[DBG_DIAGS_MAC_MAX];
	/// PHY diagnostic port state
	u16_l diags_phy[DBG_DIAGS_PHY_MAX];
	/// MAC HW RX Header descriptor pointer
	u32_l rhd_hw_ptr;
	/// MAC HW RX Buffer descriptor pointer
	u32_l rbd_hw_ptr;
};

/// Full debug dump that is forwarded to host in case of error
struct dbg_debug_dump_tag {
	/// Debug information
	struct dbg_debug_info_tag dbg_info;

	/// RX header descriptor memory
	u32_l rhd_mem[DBG_RHD_MEM_LEN/4];

	/// RX buffer descriptor memory
	u32_l rbd_mem[DBG_RBD_MEM_LEN/4];

	/// TX header descriptor memory
	u32_l thd_mem[CLS_TXQ_CNT][DBG_THD_MEM_LEN/4];

	/// Logic analyzer memory
	u32_l la_mem[2][LA_MEM_LEN/4];
};

struct radar_pulse_raw {
	unsigned pri:21;             // in unit of us
	unsigned width:11;           // in unit of 0.1us
	signed freq_start:15;      // in unit of kHz
	signed freq_end:15;        // in unit of kHz
	unsigned timeout:1;
	unsigned reserved:1;
};

/// Number of pulses in a radar event structure
#define RADAR_PULSE_MAX   127

/// Definition of an array of radar pulses
struct radar_pulse_array_desc {
	/// Buffer containing the radar pulses
	struct radar_pulse_raw pulse[RADAR_PULSE_MAX];
	/// Number of valid pulses in the buffer
	u32_l cnt;
};

/// Bit mapping inside a radar pulse element
struct radar_pulse {
	s32_l freq:6; /** Freq (resolution is 2Mhz range is [-Fadc/4 .. Fadc/4]) */
	u32_l fom:4;  /** Figure of Merit */
	u32_l len:6;  /** Length of the current radar pulse (resolution is 2us) */
	u32_l rep:16; /** Time interval between the previous radar event and the current one (in us) */
};

/// Number of entry elements in a DL Map array
#define TX_HE_MU_MAP_STATID_MAX	   16

/// Definition of a DL Map array entry element
struct he_statid_desc {
	/// STA Index
	u16_l sta_idx;
	/// Map indicating which TID(s) are associated with the sta_idx field, in the DL Map
	/// TID0 is LSB
	u16_l tidmap;
	/// Index of the STA, within the STA local array
	u8_l userid;
	/// Credit of data that RUA requests the driver to push in order to support
	/// the DL map user allocation split.
	u16_l credit;
	/// This field is the amount of data payload bytes that can be carried within the
	/// allocation that is linked to the sta_idx by the RUA algo.
	/// This amount of bytes corresponds to the max PPDU duration (5.6ms).
	u32_l psdu_len_max;
};

/// Definition of a DL Map array
struct he_mu_map_array_desc {
	/// Buffer containing the STA/TID traffic info
	struct he_statid_desc sta_alloc[CLS_MU_USER_MAX];
	/// Buffer containing the STA allocation info
	/// Map id context to use by the Drv when pushing txdesc related to this array descriptor
	u8_l idx;
	/// Index of the AP VIF to which RUA applies
	u8_l vif_index;
	/// Number of distinct STAs
	u8_l cnt_sta;
};

/// Definition of a RX vector descriptor
struct rx_vector_desc {
	/// PHY channel information
	struct phy_channel_info phy_info;

	/// RX vector 1
	u32_l rx_vect1[IPC_RXVEC_SIZE/4];

	/// Used to print a valid rx vector
	u32_l pattern;
};

#define IPC_RXDESC_FLAGS_RADIO_OPS  (0)
#define IPC_RXDESC_FLAGS_RADIO		(0x3U << IPC_RXDESC_FLAGS_RADIO_OPS)
#define IPC_RXDESC_FLAGS_CMN_OPS    (2)
#define IPC_RXDESC_FLAGS_CMN		CO_BIT(IPC_RXDESC_FLAGS_CMN_OPS)
#define IPC_RXDESC_FLAGS_BUF_NB_OPS  (8)
#define IPC_RXDESC_FLAGS_BUF_NB_MASK (0xFFU << IPC_RXDESC_FLAGS_BUF_NB_OPS)

///
struct rxdesc_tag {
	/// Host Buffer Address
	u32_l host_id;
	/// Length
	u32_l frame_len;
	/// Private Information
	u16_l flags;
	/// Status
	u16_l status;
};

/**
 ****************************************************************************************
 *  @defgroup IPC_MISC IPC Misc
 *  @ingroup IPC
 *  @brief IPC miscellaneous functions
 ****************************************************************************************
 */
/** IPC header structure.  This structure is stored at the beginning of every IPC message.
 * @warning This structure's size must NOT exceed 4 bytes in length.
 */
struct ipc_header {
	/// IPC message type.
	u16_l type;
	/// IPC message size in number of bytes.
	u16_l size;
};

struct ipc_msg_elt {
	/// Message header (alignment forced on word size, see allocation in shared env).
	struct ipc_header header __aligned(4);
};

/// Message structure for MSGs from Emb to App
struct ipc_e2a_msg {
	u16_l id;				///< Message id.
	u16_l dummy_dest_id;
	u16_l dummy_src_id;
	u16_l param_len;		 ///< Parameter embedded struct length.
	u32_l param[IPC_E2A_MSG_PARAM_SIZE];  ///< Parameter embedded struct. Must be word-aligned.
	u32_l pattern;		   ///< Used to stamp a valid MSG buffer
};

/// Message structure for Debug messages from Emb to App
struct ipc_dbg_msg {
	u32_l string[IPC_DBG_PARAM_SIZE/4]; ///< Debug string
	u32_l pattern;					///< Used to stamp a valid buffer
};

/// Message structure for MSGs from App to Emb.
/// Actually a sub-structure will be used when filling the messages.
struct ipc_a2e_msg {
	u32_l dummy_word;				// used to cope with kernel message structure
	u32_l msg[IPC_A2E_MSG_BUF_SIZE]; // body of the msg
};

struct ipc_shared_rx_buf {
	/// < ptr to hostbuf client (ipc_host client) structure
	u32_l hostid;
	/// < ptr to real hostbuf dma address
	u32_l dma_addr;
};

struct ipc_shared_rx_desc {
	/// DMA Address
	u32_l dma_addr;
};

/// Structure containing FW characteristics for compatibility checking
struct compatibility_tag {
	/// Size of IPC shared memory
	u32_l ipc_shared_size;
	/// Message struct/ID API version
	u8_l msg_api;
	/// Version of IPC shared
	u8_l ipc_shared_version;
	/// Number of host buffers available for Emb->App MSGs sending
	u16_l msge2a_buf_cnt;
	/// Number of host buffers available for Debug Messages sending
	u32_l dbgbuf_cnt;
	/// Number of host buffers available for Radar events handling
	u8_l radarbuf_cnt;
	/// Number of host buffers available for HE MU Map events handling
	u8_l hemubuf_cnt;
	/// Number of host buffers available for unsupported Rx vectors handling
	u8_l unsuprxvecbuf_cnt;
	///bus flag
	u8_l ipc_flag;
	/// Number of shared descriptors available for Data RX handling
	u16_l rxdesc_cnt;
	/// Number of host buffers available for Data Rx handling
	u16_l rxbuf_cnt;
	/// Number of descriptors in BK TX queue (power of 2, min 4, max 64)
	u16_l bk_txq;
	/// Number of descriptors in BE TX queue (power of 2, min 4, max 64)
	u16_l be_txq;
	/// Number of descriptors in VI TX queue (power of 2, min 4, max 64)
	u16_l vi_txq;
	/// Number of descriptors in VO TX queue (power of 2, min 4, max 64)
	u16_l vo_txq;
	/// Number of descriptors in BCN TX queue (power of 2, min 4, max 64)
	u16_l bcn_txq;
	/// Phase of A-MSDU Tx Offload feature
	u8_l amsdu_tx_offload_phase;
};

/*
 * TYPE and STRUCT DEFINITIONS
 ****************************************************************************************
 */

#define MAX_TXDESC_CNT  (128 / 2)
#define MAX_RXDESC_CNT  (96 / 2)
struct ipc_shared_dbg_cnt {
    volatile u32_l main_loop;

    ///drv push hostbuf count
    volatile u32_l ipc_hostbuf_get_cnt;
    ///Number of firmware processes
    volatile u32_l hostbuf_ac_cnt[CLS_TXQ_CNT];
    ///tx msdu count
    volatile u32_l tx_msdu_cnt;

    ///statistics tx AMPDU, AMPDU contains the number of MPDUs
    volatile u32_l tx_ampdu_cnt[AC_MAX][MAX_TXDESC_CNT + 1];
    ///receive BA frame count
    volatile u32_l rx_ba_cnt[AC_MAX];
    ///BA bitmap show succeed MPDU number
    volatile u32_l tx_ampdu_ba_succ[AC_MAX][MAX_TXDESC_CNT + 1];
    volatile u32_l tx_ampdu_edca_faild[AC_MAX][MAX_TXDESC_CNT + 1];
    volatile u32_l tx_ampdu_ba_loss_faild[AC_MAX][MAX_TXDESC_CNT + 1];
    volatile u32_l tx_ampdu_partial_ba_faild[AC_MAX][MAX_TXDESC_CNT + 1];

    ///isr tx ampdu statistics
    volatile u32_l tx_ampdu_last_mpdu_cnt;
    volatile u32_l tx_ampdu_done_cnt;

    ///tx cfm to drv count
    volatile u32_l tx_cfm_cnt;
    ///tx cfm count include to drv count and repush pkt count
    volatile u32_l tx_cfm_total_cnt;

    ///tx mpdu retry
    volatile u32_l tx_mpdu_hwretry_cnt;
    volatile u32_l tx_ampdu_hwretry_cnt[AC_MAX];

    ///tx repush mpdu count
    volatile u32_l tx_repush_mpdu_cnt;

	uint32_t tx_multicast;
	uint32_t tx_multicast_failed;

    volatile u32_l tx_ampdu_rtscts_retry_limit[AC_MAX];
    volatile u32_l tx_ampdu_rtscts_retry_limit_ignore[AC_MAX];
    volatile u32_l tx_ampdu_rtscts_retry_limit_expire[AC_MAX];
    volatile u32_l tx_ampdu_split_time[AC_MAX][2];

    ///timer free wmac MTHD count
    volatile u32_l timer_fwm_cnt[AC_MAX];

    ///RX-AMPDU MAX  <0:7> mpdu_cnt <8:15> AMPDU_CNT
    volatile u16_l rx_ampdu_hw_cnt;
    ///LAST RX-AMPDU MAX  <0:7> mpdu_cnt <8:15> AMPDU_CNT
    volatile u16_l rx_ampdu_hw_cnt_last;

    ///rx AMPDU count
    volatile u32_l rx_ampdu_cnt[MAX_RXDESC_CNT + 1];
    ///rx msdu count
    volatile u32_l rx_msdu_cnt;
    ///rx mpdu count
    volatile u32_l rx_mpdu_cnt;
    volatile u32_l rx_data_mpdu_cnt;
    /* rx puncture mpdu */
    volatile u32_l rx_punc_mpdu;

    ///tx isr count
    volatile u32_l tx_isr;
    ///tx done isr count
    volatile u32_l tx_done_isr;
    ///rx interrupt num
    volatile u32_l rx_done_isr;
    volatile u32_l reord_dup_rx;

    ///The number of times the receive handler function is executed.
    volatile u32_l rx_evt;

	volatile u32_l tx_trigger_frame_cnt;
	volatile u32_l rx_tb_mpdu_err_cnt;
	volatile u32_l rx_tb_mpdu_cnt;
	volatile u32_l rx_tb_qos_data_cnt;
	volatile u32_l rx_tb_qos_null_cnt;

    //dma max desc cnt per res
    volatile u32_l dma_txdesc_maxdesccnt;
    volatile u32_l dma_txdata_maxdesccnt;
    volatile u32_l dma_txctl_maxdesccnt;
    volatile u32_l dma_rxdata_maxdesccnt;
    volatile u32_l dma_rx_ctlcfm_maxdesccnt;
    /// RX DMA DEAD related counter
    volatile u32_l rx_hdr_dma_dead_cnt;
    volatile u32_l rx_payl_dma_dead_cnt;
    volatile u32_l ac0_tx_dma_dead_cnt;
    volatile u32_l ac1_tx_dma_dead_cnt;
    volatile u32_l ac2_tx_dma_dead_cnt;
    volatile u32_l ac3_tx_dma_dead_cnt;
#if defined(CFG_MERAK3000)
    volatile u32_l ac4_tx_dma_dead_cnt;
#endif
    volatile u32_l bcn_tx_dma_dead_cnt;
    volatile u32_l tx_phy_err_data_len;
    volatile u32_l tx_ampdu_bw_drop;
    volatile u32_l tx_ppdu_drop_twt_ps;
    volatile u32_l tx_ppdu_drop_omi;
    volatile u32_l tx_ampdu_rts_cts_war;
    volatile u32_l phy_rx_ofdm_timeout;
    volatile u32_l phy_rx_dsss_timeout;

    volatile u32_l rxbuf1_hw_read_delay_update;
    volatile u32_l rxbuf2_hw_read_delay_update;
    volatile u32_l rxbuf_new_read_exceed;
    volatile u32_l rxbuf_new_read_mismatch;
    volatile u32_l rx_mismatch_amsdu_disagg;

    /*
     * bit0: rx buf empty
     * bit1: tx cfm agg-desc
    */
    uint32_t bitmap;

    /* CSI related counter */
    uint32_t csi_enter_cnt;
    uint32_t csi_avail_cnt;
    uint32_t csi_reported_cnt;
    uint32_t csi_skipped_cnt;
    uint32_t csi_info_no_avail;
    uint32_t csi_info_invalid;
    uint32_t csi_timer_callback;
    uint32_t csi_dma_triggered;

    /* ATF related counter */
    uint32_t atf_sched_timer_callback;
    uint32_t atf_stats_timer_callback;

    ///RX buffer overflow
    uint16_t rxbuf1_overflow;
    uint16_t rxbuf2_overflow;

    ////WMAC FCS ERROR statistor
    volatile u32_l rx_fcs_error_cnt;
    volatile u32_l rx_undef_error_cnt;
    volatile u32_l rx_decry_error_cnt;   ///Decryption error
    volatile u32_l rx_ndp_cnt;
    volatile u32_l rxdrop;
    volatile u32_l phyif_overflow;

    /* MU related */
    uint32_t try_mu_group;
    uint32_t mu_select_buddy;
    uint32_t mu_lack_sta_num;
    uint32_t mu_airtime_overlength;
    uint32_t mu_buddy_select_mu;
    uint32_t mu_buddy_select_su;
    uint32_t mu_buddy_fail_1st;
    uint32_t mu_buddy_fail_ps;
    uint32_t mu_buddy_fail_he;
    uint32_t mu_buddy_fail_bytes;
    uint32_t mu_buddy_fail_mpdu;
    uint32_t mu_buddy_fail_bw;
    uint32_t mu_buddy_fail_rc_format;
    uint32_t mu_buddy_fail_mcs;
    uint32_t mu_user_num[16];
    uint32_t mu_rua;
    uint32_t mu_rua_failed;
    uint32_t mu_format_ppdu;
    uint32_t mu_format_with_qos_null;
    uint32_t mu_format_seq_bar;
    uint32_t mu_format_mu_bar;
    uint32_t mu_format_unknown_bar;
    uint32_t mu_chain_mu_bar;
    uint32_t mu_chain_seq_bar;
    uint32_t mu_chain_ppdu;
    uint32_t mu_release_pending;
    uint32_t mu_abnormal_no_desc;
    uint32_t mu_done_ppdu_failed;
    uint32_t mu_done_ppdu_retran;
    uint32_t mu_done_ppdu_all_done;
    uint32_t mu_done_trig_done;
    uint32_t mu_done_trig_hang;
    uint32_t mu_done_trig_success;
    uint32_t mu_done_trig_failed;
    uint32_t mu_format_mu_bar_ba_cnt;
    uint32_t mu_format_mu_bar_no_ba;
    uint32_t mu_done_bar_all_done;
    uint32_t mu_done_bar_next_atomic;
    uint32_t mu_done_bar_no_next;
    uint32_t mu_tx_mpdu_cnt;
    uint32_t mu_tx_ampdu_cnt;
    uint32_t mu_tx_mpdu_failed_cnt;

    /* Reset Counter */
    uint32_t reset_ppdu_cnt;
    uint32_t reset_mpdu_cnt;
    uint32_t reset_count;
    uint32_t reset_count_with_mpdu;
    uint32_t reset_with_frequent_int;
    /* DPD WMAC Tx */
    uint32_t dpd_wmac_tx_ampdu;
    uint32_t dpd_wmac_tx_smpdu;

    /* sounding related count */
    uint32_t sound_cnt;
    uint32_t sound_push_cnt;
    uint32_t rx_cbf_cnt;
    uint32_t cbf_upload_cnt;
    uint32_t cbf_download_cnt;
    uint32_t cbf_timeout;
    uint32_t tx_bf_used;

    uint32_t txop_enabled;
    uint32_t txop_disabled;

    uint32_t msgq_txdesc;
    uint32_t msgq_rxdesc;
};


// Indexes are defined in the MIB shared structure
struct ipc_shared_env_tag {
	volatile struct compatibility_tag comp_info; //FW characteristics

	volatile struct ipc_a2e_msg msg_a2e_buf; // room for MSG to be sent from App to Emb

	// Fields for MSGs sending from Emb to App
	volatile struct ipc_e2a_msg msg_e2a_buf; // room to build the MSG to be DMA Xferred
	volatile struct dma_desc msg_dma_desc;   // DMA descriptor for Emb->App MSGs Xfers
	//volatile u32_l msg_e2a_hostbuf_addr[IPC_MSGE2A_BUF_CNT_D2K]; // buffers @ for DMA Xfers

	// Fields for Debug MSGs sending from Emb to App
	volatile struct ipc_dbg_msg dbg_buf; // room to build the MSG to be DMA Xferred
	volatile struct dma_desc dbg_dma_desc;   // DMA descriptor for Emb->App MSGs Xfers
	//volatile u32_l dbg_hostbuf_addr[IPC_DBGBUF_CNT_D2K]; // buffers @ for MSGs DMA Xfers
	volatile u32_l la_dbginfo_addr; // Host buffer address for the debug information
	// WIFI-DRV & FW H-ack
	u32_l heart_ack;
	// WIFI-DRV & FW H-rdy
	u32_l heart_rdy;
	// WIFI-DRV & FW UART-Info
	u32_l uart_info;
	volatile u32_l thd_pattern_addr;
	volatile u32_l tbd_pattern_addr;
	volatile u32_l radarbuf_hostbuf[IPC_RADARBUF_CNT]; // buffers @ for Radar Events
	volatile u32_l hemubuf_hostbuf[IPC_HEMUBUF_CNT]; // buffers @ for DL HE MU Map Events
	volatile u32_l unsuprxvecbuf_hostbuf[IPC_UNSUPRXVECBUF_CNT]; // buffers @ for unsupported Rx vectors
	volatile u32_l csibuf_hostbuf[IPC_CSIBUF_CNT]; // buffers @ for CSI report

	volatile uint32_t atf_stats_hostbuf;
	volatile uint32_t atf_quota_hostbuf;
	//u32_l buffered[CLS_REMOTE_STA_MAX][TID_MAX];

	volatile uint32_t trace_pattern;
	volatile uint32_t trace_start;
	volatile uint32_t trace_end;
	volatile uint32_t trace_size;
	volatile uint32_t trace_offset;
	volatile uint32_t trace_nb_compo;
	volatile uint32_t trace_offset_compo;
	volatile uint32_t cal_enabled;
	volatile u32_l data_a2e_buf[12000];
	volatile u32_l data_e2a_buf[8000];
	volatile uint32_t dbg_cnt_addr;
	volatile uint32_t dump_wmac_info_flag;
	volatile u32_l bcmc_traffic_msg_cnt_put;
	volatile u32_l bcmc_traffic_msg_cnt_get;

//#ifdef DUBHE_PLATFORM_DBG
	//volatile struct ipc_shared_dbg_cnt dbg_cnt;
//#endif
	volatile uint32_t ipc_pattern;  ///synchronization
	volatile uint32_t wpu_ipc_pattern;  ///synchronization
	// nr is different from 2.4G to 5G, from different chipsets
	// volatile u32_l txdesc_hostbuf_addr[IPC_TXDESC_CNT];
	// volatile u32_l txcfm_hostbuf_addr[IPC_TXCFM_CNT];
	// volatile struct ipc_shared_rx_desc host_rxdesc[IPC_RXDESC_CNT];
	// volatile struct ipc_shared_rx_buf  host_rxbuf[IPC_RXBUF_CNT];
};

#define IPC_PATTERN_MAGIC			(0x1234CCCCU)
#define IPC_PATTERN_INIT_MAGIC	   (0x55C5C5C5U)
#define WPU_IPC_PATTERN_MAGIC		(0x5678CCCCU)
#define WPU_IPC_INIT_PATTERN_MAGIC   (0x56780000U)
#define CLS_WIFI_LOCAL_SYS_ENABLE_VALUE_ENABLE	(0x5a6d7c8d)
#define CLS_WIFI_LOCAL_SYS_ENABLE_VALUE_DISABLE	(0xa1b2c3d4)

struct cmn_compatibility_tag {
	u32_l ipc_shared_size;
	/// Message struct/ID API version
	u8_l msg_api;
	/// Version of IPC shared
	u8_l version;
	/// Number of host buffers available for Emb->App MSGs sending
	u16_l msge2a_buf_cnt;
};

struct cmn_ipc_shared_env_tag {
	volatile u32_l ipc_pattern;
	volatile struct cmn_compatibility_tag comp;
	volatile u32_l fw_ipc_pattern;

	volatile u32_l dbg_hostbuf_addr[CMN_DBG_CNT];
	volatile u32_l a2e_msg[CMN_MSGBUF_SIZE];
	volatile u32_l e2a_msg_buf[CMN_E2A_MSGBUF_NUM];

	/// RX Descriptors Array
	volatile struct ipc_shared_rx_desc host_rxdesc_2g[CMN_RXDESC_CNT_MAX_2G];
	volatile struct ipc_shared_rx_desc host_rxdesc_5g[CMN_RXDESC_CNT_MAX_5G];
	/// RX Buffers Array
	volatile struct ipc_shared_rx_buf host_rxbuf_2g[CMN_RXBUF_CNT_2G];
	volatile struct ipc_shared_rx_buf host_rxbuf_5g[CMN_RXBUF_CNT_5G];
};

/*
 * TYPE and STRUCT DEFINITIONS
 ****************************************************************************************
 */

// IRQs from app to emb
#define IPC_IRQ_A2E_TXDESC		CO_BIT(7)
#define IPC_IRQ_A2E_RXBUF_BACK		CO_BIT(5)
#define IPC_IRQ_A2E_RXDESC_BACK		CO_BIT(4)

#define IPC_IRQ_A2E_MSG			CO_BIT(1)
#define IPC_IRQ_A2E_DBG			CO_BIT(0)

#define IPC_IRQ_A2E_ALL			(IPC_IRQ_A2E_TXDESC|IPC_IRQ_A2E_MSG|IPC_IRQ_A2E_DBG)

// IRQs from emb to app
#define IPC_IRQ_E2A_TXCFM_POS		7

#define IPC_IRQ_E2A_TXCFM		(((1 << CLS_TXQ_CNT) - 1) << IPC_IRQ_E2A_TXCFM_POS)
#define IPC_IRQ_E2A_CMN_DBG 		CO_BIT(15)
#define IPC_IRQ_E2A_CMN_RXDESC		CO_BIT(14)
#define IPC_IRQ_E2A_ATF_STATS		CO_BIT(13)
#define IPC_IRQ_E2A_CSI			CO_BIT(IPC_IRQ_E2A_TXCFM_POS + CLS_TXQ_CNT)
#define IPC_IRQ_E2A_HE_MU_DESC		CO_BIT(6)
#define IPC_IRQ_E2A_UNSUP_RX_VEC	CO_BIT(5)
#define IPC_IRQ_E2A_RADAR		CO_BIT(4)
#define IPC_IRQ_E2A_RXDESC		CO_BIT(3)
#define IPC_IRQ_E2A_MSG_ACK		CO_BIT(2)
#define IPC_IRQ_E2A_MSG			CO_BIT(1)
#define IPC_IRQ_E2A_DBG			CO_BIT(0)

#define IPC_IRQ_E2A_ALL		 (IPC_IRQ_E2A_TXCFM	\
					| IPC_IRQ_E2A_RXDESC		\
					| IPC_IRQ_E2A_MSG_ACK		\
					| IPC_IRQ_E2A_MSG		\
					| IPC_IRQ_E2A_DBG		\
					| IPC_IRQ_E2A_RADAR		\
					| IPC_IRQ_E2A_UNSUP_RX_VEC	\
					| IPC_IRQ_E2A_HE_MU_DESC	\
					| IPC_IRQ_E2A_CSI		\
					| IPC_IRQ_E2A_ATF_STATS)

// FLAGS for RX desc
#define IPC_RX_FORWARD		  CO_BIT(1)
#define IPC_RX_INTRABSS		 CO_BIT(0)


// IPC message TYPE
enum {
	IPC_MSG_NONE = 0,
	IPC_MSG_WRAP,
	IPC_MSG_KMSG,

	IPC_DBG_STRING,

};

#define IPC_DUMP_FLAG_WMAC		CO_BIT(2)
#define IPC_DUMP_FLAG_DBGCNT		CO_BIT(3)

#endif // _IPC_SHARED_H_

