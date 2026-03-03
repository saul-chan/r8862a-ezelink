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

#ifndef _HAL_DESC_H_
#define _HAL_DESC_H_

#include "lmac_types.h"

/******************************************************************************
 * HW type
 ******************************************************************************/
#define CLS_WIFI_MACHW_HE_AP 3

/******************************************************************************
 * TX
 ******************************************************************************/
/* Rate and policy table */

#define N_CCK   8
#define N_OFDM  8
#define N_HT	(8 * 2 * 2 * 4)  // MCS * BW * GI * NSS
#define N_VHT   (10 * 4 * 2 * 8) // MCS * BW * GI * NSS
#define N_HE_SU (12 * 4 * 3 * 8) // MCS * BW * GI * NSS
#define N_HE_MU (12 * 6 * 3 * 8) // MCS * RU * GI * NSS
#define N_HE_ER (3 * 3 + 3)	  // (MCS0-2 * GI) (RU242) + (MCS0 * GI) (RU106)
#define N_HE_TB (12 * 6 * 3 * 8) // MCS * RU * GI * NSS
#define N_EHT_SU (16 * 4 * 3 * 2) // MCS * BW * GI * NSS
#define N_EHT_MU (16 * 12 * 3 * 2) // MCS * RU * GI * NSS
#define N_EHT_TB (16 * 12 * 3 * 2) // MCS * RU * GI * NSS

/* conversion table from NL80211 to MACHW enum */
extern const int chnl2bw[];

/* conversion table from MACHW to NL80211 enum */
extern const int bw2chnl[];

/* conversion table for legacy rate */
struct cls_wifi_legrate {
	s16 idx;
	u16 rate;  // in 100Kbps
};
extern const struct cls_wifi_legrate legrates_lut[];

/* Values for formatModTx */
#define FORMATMOD_NON_HT		  0
#define FORMATMOD_NON_HT_DUP_OFDM 1
#define FORMATMOD_HT_MF		   2
#define FORMATMOD_HT_GF		   3
#define FORMATMOD_VHT			 4
#define FORMATMOD_HE_SU		   5
#define FORMATMOD_HE_MU		   6
#define FORMATMOD_HE_ER		   7
#define FORMATMOD_HE_TB		   8
#define FORMATMOD_EHT_MU_SU	   9
#define FORMATMOD_EHT_TB	   10
#define FORMATMOD_EHT_ER	   11
#define FORMATMOD_OFFLOAD	   12

/* Values for navProtFrmEx */
#define NAV_PROT_NO_PROT_BIT				 0
#define NAV_PROT_SELF_CTS_BIT				1
#define NAV_PROT_RTS_CTS_BIT				 2
#define NAV_PROT_RTS_CTS_WITH_QAP_BIT		3
#define NAV_PROT_STBC_BIT					4

/* THD MACCTRLINFO2 fields, used in  struct umacdesc umac.flags */
/// WhichDescriptor definition - contains aMPDU bit and position value
/// Offset of WhichDescriptor field in the MAC CONTROL INFO 2 word
#define WHICHDESC_OFT					 19
/// Mask of the WhichDescriptor field
#define WHICHDESC_MSK					 (0x07 << WHICHDESC_OFT)
/// Only 1 THD possible, describing an unfragmented MSDU
#define WHICHDESC_UNFRAGMENTED_MSDU	   (0x00 << WHICHDESC_OFT)
/// THD describing the first MPDU of a fragmented MSDU
#define WHICHDESC_FRAGMENTED_MSDU_FIRST   (0x01 << WHICHDESC_OFT)
/// THD describing intermediate MPDUs of a fragmented MSDU
#define WHICHDESC_FRAGMENTED_MSDU_INT	 (0x02 << WHICHDESC_OFT)
/// THD describing the last MPDU of a fragmented MSDU
#define WHICHDESC_FRAGMENTED_MSDU_LAST	(0x03 << WHICHDESC_OFT)
/// THD for extra descriptor starting an AMPDU
#define WHICHDESC_AMPDU_EXTRA			 (0x04 << WHICHDESC_OFT)
/// THD describing the first MPDU of an A-MPDU
#define WHICHDESC_AMPDU_FIRST			 (0x05 << WHICHDESC_OFT)
/// THD describing intermediate MPDUs of an A-MPDU
#define WHICHDESC_AMPDU_INT			   (0x06 << WHICHDESC_OFT)
/// THD describing the last MPDU of an A-MPDU
#define WHICHDESC_AMPDU_LAST			  (0x07 << WHICHDESC_OFT)

/// aMPDU bit offset
#define AMPDU_OFT						 21
/// aMPDU bit
#define AMPDU_BIT						 CO_BIT(AMPDU_OFT)

union cls_wifi_mcs_index {
	struct {
		u32 mcs : 3;
		u32 nss : 2;
	} ht;
	struct {
		u32 mcs : 4;
		u32 nss : 3;
	} vht;
	struct {
		u32 mcs : 4;
		u32 nss : 3;
	} he;
	u32 legacy : 7;
};

union cls_wifi_rate_ctrl_info {
	struct {
		u32 mcsIndexTx	  : 7;
		u32 bwTx			: 2;
		u32 giAndPreTypeTx  : 2;
		u32 formatModTx	 : 3;
		u32 dcmTx		   : 1;
	};
	u32 value;
};

union cls_wifi_pol_phy_ctrl_info_1 {
	struct {
		u32 rsvd1	 : 3;
		u32 bfFrmEx   : 1;
		u32 numExtnSS : 2;
		u32 fecCoding : 1;
		u32 stbc	  : 2;
		u32 rsvd2	 : 5;
		u32 nTx	   : 3;
		u32 nTxProt   : 3;
	};
	u32 value;
};

union cls_wifi_pol_phy_ctrl_info_2 {
	struct {
		u32 antennaSet : 8;
		u32 smmIndex   : 8;
		u32 beamFormed : 1;
	};
	u32 value;
};

/**
 * enum txu_cntrl_flags -  TX Flags passed by the host to UMAC
 *
 * @TXU_CNTRL_MORE_DATA: More data are buffered on host side for this STA after this packet
 * @TXU_CNTRL_MGMT: TX Frame is a management frame (i.e. MAC header is present)
 * @TXU_CNTRL_MGMT_NO_CCK: No CCK rate can be used (valid only if TXU_CNTRL_MGMT is set)
 * @TXU_CNTRL_AMSDU: The frame is an A-MSDU
 * @TXU_CNTRL_MGMT_ROBUST: The frame is a robust management frame
 * @TXU_CNTRL_USE_4ADDR: The frame shall be transmitted using a 4 address MAC Header
 * @TXU_CNTRL_EOSP: The frame is the last of the UAPSD service period
 * @TXU_CNTRL_MESH_FWD: This frame is forwarded
 * @TXU_CNTRL_TDLS: This frame is sent directly to a TDLS peer
 * @TXU_CNTRL_REUSE_SN: Re-use SN passed by host instead of computing a new one
 * @TXU_CNTRL_SKIP_LOGIC_PORT: The data frame should not be doprpped if Encryption key is not yet available
 */
enum txu_cntrl_flags {
	TXU_CNTRL_MORE_DATA = BIT(0),
	TXU_CNTRL_MGMT = BIT(1),
	TXU_CNTRL_MGMT_NO_CCK = BIT(2),
	TXU_CNTRL_AMSDU = BIT(3),
	TXU_CNTRL_MGMT_ROBUST = BIT(4),
	TXU_CNTRL_USE_4ADDR = BIT(5),
	TXU_CNTRL_EOSP = BIT(6),
	TXU_CNTRL_MESH_FWD = BIT(7),
	TXU_CNTRL_TDLS = BIT(8),
	TXU_CNTRL_REUSE_SN = BIT(9),
	TXU_CNTRL_SKIP_LOGIC_PORT = BIT(10),
};

/**
 * enum cls_wifi_txu_status_info_bf - Bitfield of confirmation status
 *
 * @DONE: Packet has been processed by the firmware.
 * @RETRY_REQUIRED: Packet has been transmitted but not acknoledged.
 * Driver must repush it.
 * @SW_RETRY_REQUIRED: Packet has not been transmitted (FW wasn't able to push
 * it when it received it: not active channel ...). Driver must repush it.
 * @ACKNOWLEDGED: Packet has been acknowledged by peer
 * @SN: Sequence number used to send the frame.  If retry is required ti shall
 * be reused when re-pushing the frame
 */
enum txu_status_info_bf {
	BF_FIELD(TXU_STATUS, DONE, 0, 1),
	BF_FIELD(TXU_STATUS, RETRY_REQUIRED, 1, 1),
	BF_FIELD(TXU_STATUS, SW_RETRY_REQUIRED, 2, 1),
	BF_FIELD(TXU_STATUS, ACKNOWLEDGED, 3, 1),
	BF_FIELD(TXU_STATUS, SN, 4, 12),
};

#define TXU_STATUS_GET(field, val) BF_GET(TXU_STATUS, field, val)
#define TXU_STATUS_IS_SET(field, val) BF_IS_SET(TXU_STATUS, field, val)

/**
 * struct tx_cfm_tag - Structure indicating the status and other
 * information about the transmission
 *
 * @credits: Number of credits to be reallocated for the txq that push this
 * buffer (can be 0 or 1)
 * @ampdu_size: Size of the ampdu in which the frame has been transmitted if
 * this was the last frame of the a-mpdu, and 0 if the frame is not the last
 * frame on a a-mdpu.
 * 1 means that the frame has been transmitted as a singleton.
 * @amsdu_size: Size, in bytes, allowed to create a-msdu.
 * @status: transmission status (@ref txu_status_info_bf)
 * @rate_config: TX rate config used for transmission (@ref rc_rate_bf)
 * @hostid: Host id to retrieve TX buffer associated to this confimration
 */
struct tx_cfm_tag {
	s16_l credits;
	u8_l ampdu_size;
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
	u16_l amsdu_size;
#endif
	u32_l status;
	u32_l rate_config;
	u32_l hostid;
};

/******************************************************************************
 * RX
 ******************************************************************************/
struct rx_leg_vect {
	u16 dyn_bw_in_non_ht : 1;
	u16 chn_bw_in_non_ht : 2;
	u16 rsvd			 : 4;
	u16 lsig_valid	   : 1;
} __packed;

struct rx_ht_vect {
	u16 sounding	: 1;
	u16 smoothing   : 1;
	u16 short_gi	: 1;
	u16 aggregation : 1;
	u16 stbc		: 1;
	u16 num_extn_ss : 2;
	u16 lsig_valid  : 1;
	u16 rsvd1	   : 8;

	u16 mcs   : 7;
	u16 rsvd2 : 8;
	u16 fec   : 1;

	u16 length :16;
} __packed;

struct rx_vht_vect {
	u16 sounding		 : 1;
	u16 beamformed	   : 1;
	u16 short_gi		 : 1;
	u16 rsvd1			: 1;
	u16 stbc			 : 1;
	u16 doze_not_allowed : 1;
	u16 first_user	   : 1;
	u16 rsvd2			: 1;
	u16 group_id		 : 6;
	u16 rsvd3			: 2;

	u16 mcs   : 4;
	u16 rsvd4 : 8;
	u16 nss   : 3;
	u16 fec   : 1;

	u32 length	  :20;
	u32 rsvd5	   : 3;
	u32 partial_aid : 9;
} __packed;

struct rx_he_vect {
	u16 sounding	: 1;
	u16 beamformed  : 1;
	u16 gi_type	 : 2;
	u16 stbc		: 1;
	u16 rsvd1	   : 3;
	u16 uplink_flag : 1;
	u16 beam_change : 1;
	u16 rsvd2	   : 1;
	u16 he_ltf_type : 2;
	u16 doppler	 : 1;
	u16 rsvd3	   : 2;

	u16 bss_color	 : 6;
	u16 rsvd4		 : 2;
	u16 txop_duration : 7;
	u16 rsvd5		 : 1;

#if defined(CFG_MERAK3000)
	u16 rx_vect_sub_channel : 8;
	u16 rsvd8      : 8;
#endif

	u16 pe_duration	 : 4;
	u16 spatial_reuse   : 4;
	u16 ru_size		 : 3;
	u16 sig_b_comp_mode : 1;
	u16 mcs_sig_b	   : 3;
	u16 dcm_sig_b	   : 1;

	u16 mcs   : 4;
	u16 rsvd6 : 7;
	u16 dcm   : 1;
	u16 nss   : 3;
	u16 fec   : 1;

	u32 length :22;
	u32 rsvd7  :10;
} __packed;

struct rx_he_tb_vect {
	u16 sounding	: 1;
	u16 beamformed  : 1;
	u16 gi_type	 : 2;
	u16 stbc		: 1;
	u16 rsvd1	   : 3;
	u16 uplink_flag : 1;
	u16 beam_change : 1;
	u16 rsvd2	   : 1;
	u16 he_ltf_type : 2;
	u16 doppler	 : 1;
	u16 rsvd3	   : 2;

	u16 bss_color	 : 6;
	u16 rsvd4		 : 2;
	u16 txop_duration : 7;
	u16 rsvd5		 : 1;

#if defined(CFG_MERAK3000)
	u16 rx_vect_sub_channel : 8;
	u16 rsvd_he10      : 8;
#endif

	u16 spatial_reuse1 : 4;
	u16 spatial_reuse2 : 4;
	u16 spatial_reuse3 : 4;
	u16 spatial_reuse4 : 4;

	u16 n_user   : 8;
	u16 rsvd_he6 : 8;

	u16 mcs	  : 4;
	u16 ru_size  : 3;
	u16 rsvd_he7 : 4;
	u16 dcm	  : 1;
	u16 nss	  : 3;
	u16 fec	  : 1;

	u32 length   : 22;
	u32 rsvd_he8 : 10;

	u16 staid	: 11;
	u16 rsvd_he9 : 5;
} __packed;

struct rx_vector1_offload {
	u16 rate_idx;
	u32 rate_config;
}__packed;

struct rx_vector_1 {
	u16 format_mod  : 4;
	u16 ch_bw	   : 3;
	u16 pre_type	: 1;
	u16 antenna_set : 8;

	s16 rssi_leg : 8;
	s16 rssi1	: 8;

	u16 leg_length :12;
	u16 leg_rate   : 4;

	union {
		struct rx_leg_vect leg;
		struct rx_ht_vect ht;
		struct rx_vht_vect vht;
		struct rx_he_vect he;
		struct rx_he_tb_vect he_tb;
		struct rx_vector1_offload offload_vector1;
	};
} __packed;


struct rx_vector_2 {
	u16 buf_idx     : 15;
	u16 buf_idx_vld : 1;
	u16 ppdu_id;
	u16 sinr[16];
#if defined(CFG_MERAK3000)
	u8 agc[12];
	u16 symbol_num;
	u16 corr_ant[3];
	u32 rssi_ant	  : 24;
	u32 pdp_info	  : 4;
	u32 rsvd_vect2_1 : 4;
	u32 ppdu_tstamp[2];
	u32 noise_info;
	u8 rsvd_vect2_2[24];
#else
	u16 agc[4];
	u16 symbol_num;
	u16 reserved;
#endif
};

struct mpdu_status {
	u32 rx_vect2_valid	 : 1;
	u32 resp_frame		 : 1;
	u32 decr_type		  : 4;
	u32 decr_err		   : 1;
	u32 undef_err		  : 1;
	u32 fcs_err			: 1;
	u32 addr_mismatch	  : 1;
	u32 ga_frame		   : 1;
	u32 current_ac		 : 2;
	u32 frm_successful_rx  : 1;
	u32 desc_done_rx	   : 1;
	u32 key_sram_index	 : 10;
	u32 key_sram_v		 : 1;
	u32 type			   : 2;
	u32 subtype			: 4;
};

/*
 *  Used for both Hardware type (but for CLS Hardware rx_vectx and status fields
 *  must be converted first using cls_wifi_xxxx_convert function).
 *  It is ok to use same structure for both HW type because both version
 *  of 'RX vectors' and 'MPDU Status' have the same size
 */
struct hw_vect {
	/** Total length for the MPDU transfer */
	u32 len				   :16;
	/** AMPDU Status Information */
	u32 mpdu_cnt	      : 8;
	u32 ampdu_cnt	      : 8;

#if defined(CFG_MERAK3000)
	/* AMSDU related status info */
	u32 amsdu_stat_info   :16;
	u32 reserved          :16;
#endif

	/** Receive Vector 1 */
	struct rx_vector_1 rx_vect1;

	/** MPDU status information */
	struct mpdu_status status;
#if defined(CFG_MERAK3000)
	uint32_t machdr[15];
#endif
};

struct phy_channel_info_desc {
	/** PHY channel information 1 */
	u32	phy_band		   : 8;
	u32	phy_channel_type   : 8;
	u32	phy_prim20_freq	: 16;

	/** PHY channel information 2 */
	u32	phy_center1_freq   : 16;
	u32	phy_center2_freq   : 16;
};

static inline int cls_wifi_machw_type(uint32_t machw_version_2) { return CLS_WIFI_MACHW_HE_AP; }

/******************************************************************************
 * Modem
 ******************************************************************************/
#define MDM_PHY_CONFIG_VERSION	   2

// MODEM features (from reg_mdm_stat.h)
/// MUMIMOTX field bit
#define MDM_MUMIMOTX_BIT	((u32)0x80000000)
/// MUMIMOTX field position
#define MDM_MUMIMOTX_POS	31
/// MUMIMORX field bit
#define MDM_MUMIMORX_BIT	((u32)0x40000000)
/// MUMIMORX field position
#define MDM_MUMIMORX_POS	30
/// BFMER field bit
#define MDM_BFMER_BIT	   ((u32)0x20000000)
/// BFMER field position
#define MDM_BFMER_POS	   29
/// BFMEE field bit
#define MDM_BFMEE_BIT	   ((u32)0x10000000)
/// BFMEE field position
#define MDM_BFMEE_POS	   28
/// LDPCDEC field bit
#define MDM_LDPCDEC_BIT	 ((u32)0x08000000)
/// LDPCDEC field position
#define MDM_LDPCDEC_POS	 27
/// LDPCENC field bit
#define MDM_LDPCENC_BIT	 ((u32)0x04000000)
/// LDPCENC field position
#define MDM_LDPCENC_POS	 26
/// CHBW field mask
#define MDM_CHBW_MASK	   ((u32)0x03000000)
/// CHBW field LSB position
#define MDM_CHBW_LSB		24
/// CHBW field width
#define MDM_CHBW_WIDTH	  ((u32)0x00000002)
/// DSSSCCK field bit
#define MDM_DSSSCCK_BIT	 ((u32)0x00800000)
/// DSSSCCK field position
#define MDM_DSSSCCK_POS	 23
/// VHT field bit
#define MDM_VHT_BIT		 ((u32)0x00400000)
/// VHT field position
#define MDM_VHT_POS		 22
/// HE field bit
#define MDM_HE_BIT		  ((u32)0x00200000)
/// HE field position
#define MDM_HE_POS		  21
/// ESS field bit
#define MDM_ESS_BIT		 ((u32)0x00100000)
/// ESS field position
#define MDM_ESS_POS		 20
/// RFMODE field mask
#define MDM_RFMODE_MASK	 ((u32)0x000F0000)
/// RFMODE field LSB position
#define MDM_RFMODE_LSB	  16
/// RFMODE field width
#define MDM_RFMODE_WIDTH	((u32)0x00000004)
/// NSTS field mask
#define MDM_NSTS_MASK	   ((u32)0x0000F000)
/// NSTS field LSB position
#define MDM_NSTS_LSB		12
/// NSTS field width
#define MDM_NSTS_WIDTH	  ((u32)0x00000004)
/// NSS field mask
#define MDM_NSS_MASK		((u32)0x00000F00)
/// NSS field LSB position
#define MDM_NSS_LSB		 8
/// NSS field width
#define MDM_NSS_WIDTH	   ((u32)0x00000004)
/// NTX field mask
#define MDM_NTX_MASK		((u32)0x000000F0)
/// NTX field LSB position
#define MDM_NTX_LSB		 4
/// NTX field width
#define MDM_NTX_WIDTH	   ((u32)0x00000004)
/// NRX field mask
#define MDM_NRX_MASK		((u32)0x0000000F)
/// NRX field LSB position
#define MDM_NRX_LSB		 0
/// NRX field width
#define MDM_NRX_WIDTH	   ((u32)0x00000004)

#define CLS_WIFI_MAC_GCMP_BIT		BIT(9)
#define CLS_WIFI_MAC_BFMER_BIT		BIT(18)

#define __MDM_PHYCFG_FROM_VERS(v)  (((v) & MDM_RFMODE_MASK) >> MDM_RFMODE_LSB)

#define RIU_FCU_PRESENT_MASK	   ((u32)0xFF000000)
#define RIU_FCU_PRESENT_LSB		24

#define __RIU_FCU_PRESENT(v)  (((v) & RIU_FCU_PRESENT_MASK) >> RIU_FCU_PRESENT_LSB == 5)

/// AGC load version field mask
#define RIU_AGC_LOAD_MASK		  ((u32)0x00C00000)
/// AGC load version field LSB position
#define RIU_AGC_LOAD_LSB		   22

#define __RIU_AGCLOAD_FROM_VERS(v) (((v) & RIU_AGC_LOAD_MASK) >> RIU_AGC_LOAD_LSB)

#define __FPGA_TYPE(v)			 (((v) & 0xFFFF0000) >> 16)

#define __MDM_MAJOR_VERSION(v) (((v) & 0xFF000000) >> 24)
#define __MDM_MINOR_VERSION(v) (((v) & 0x00FF0000) >> 16)
#define __MDM_VERSION(v)	   ((__MDM_MAJOR_VERSION(v) + 2) * 10 + __MDM_MINOR_VERSION(v))


#endif // _HAL_DESC_H_
