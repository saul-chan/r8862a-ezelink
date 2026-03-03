/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 */
#ifndef _CALI_STRUCT_H_
#define _CALI_STRUCT_H_

//
// Include Ceva header files.
//

/* The index should be same as radio_list[][8] @cls_wifi_cali_debugfs.c */
enum cali_phy_band {
	CALI_PHY_BAND_2G4,
	CALI_PHY_BAND_5G,
	CALI_PHY_BAND_5G_2,
	CALI_PHY_BAND_MAX
};

enum cali_phy_bw {
	CALI_PHY_BW_20,
	CALI_PHY_BW_40,
	CALI_PHY_BW_80,
	CALI_PHY_BW_160,
	CALI_PHY_BW_MAX
};

enum cali_ba_ack_policy {
	CALI_BA_ACK_POLICY_NO_ACK,
	CALI_BA_ACK_POLICY_NORMAL,
	CALI_BA_ACK_POLICY_IMPLICIT,
	CALI_BA_ACK_POLICY_MAX
};

enum cali_he_ltf_type {
	CALI_HE_LTF_1X = 1,
	CALI_HE_LTF_2X = 2,
	CALI_HE_LTF_4X = 4,
};

enum cali_eht_ltf_type {
	CALI_EHT_LTF_1X = 1,
	CALI_EHT_LTF_2X = 2,
	CALI_EHT_LTF_4X = 4,
};

enum cali_mu_type {
	CALI_MU_DL_OFDMA,
	CALI_MU_DL_MIMO,
	CALI_MU_UL_OFDMA,
	CALI_MU_UL_MIMO,
	CALI_MU_TYPE_MAX,
};

enum cali_ack_policy {
	CALI_ACK_POLICY_NORMAL_ACK,
	CALI_ACK_POLICY_NO_ACK,
	CALI_ACK_POLICY_NO_EXPLICIT_ACK,
	CALI_ACK_POLICY_BLOCK_ACK,
	CALI_ACK_POLICY_MAX
};

enum cali_fec_coding {
	CALI_FEC_CODING_BCC,
	CALI_FEC_CODING_LDPC,
	CALI_FEC_CODING_MAX
};

enum cali_mac_mode {
	CALI_MAC_MODE_AP,
	CALI_MAC_MODE_STA,
	CALI_MAC_MODE_MAX
};

enum cali_phy_mode {
	CALI_PHY_MODE_11B,
	CALI_PHY_MODE_11A,
	CALI_PHY_MODE_11G,
	CALI_PHY_MODE_11N,
	CALI_PHY_MODE_11N_5G,
	CALI_PHY_MODE_11AC,
	CALI_PHY_MODE_11AX,
#if defined(CFG_MERAK3000)
	CALI_PHY_MODE_11BE,
#endif
	CALI_PHY_MODE_MAX
};

enum cali_prot_nav_frm_ex {
	CALI_PROT_NAV_FRM_EX_NONE,
	CALI_PROT_NAV_FRM_EX_SELF_CTS,
	CALI_PROT_NAV_FRM_EX_RTS_CTS,
	CALI_PROT_NAV_FRM_EX_RTS_CTS_QAP,
	CALI_PROT_NAV_FRM_EX_STBC,
	CALI_PROT_NAV_FRM_EX_MAX
};

/* The index should be same as ciper_list[][8] @cls_wifi_cali_debugfs.c */
enum cali_cipher_type {
	CALI_CIPHER_INDEX_NONE,
	CALI_CIPHER_INDEX_WEP,
	CALI_CIPHER_INDEX_TKIP,
	CALI_CIPHER_INDEX_CCMP,
	CALI_CIPHER_INDEX_WPI,
	CALI_CIPHER_INDEX_GCMP,
	CALI_CIPHER_INDEX_BIP_CMAC_128,
	CALI_CIPHER_INDEX_BIP_CMAC_256,
	CALI_CIPHER_INDEX_WAPI_GCM,
	CALI_CIPHER_INDEX_SOFT_GCM,
	CALI_CIPHER_INDEX_BIP_GMAC_128,
	CALI_CIPHER_INDEX_BIP_GMAC_256,
	CALI_CIPHER_INDEX_MAX
};

enum cali_format_mod {
	CALI_FORMAT_MOD_NON_HT,
	CALI_FORMAT_MOD_NON_HT_DUP_OFDM,
	CALI_FORMAT_MOD_HT_MF,
	CALI_FORMAT_MOD_HT_GF,
	CALI_FORMAT_MOD_VHT,
	CALI_FORMAT_MOD_HE_SU,
	CALI_FORMAT_MOD_HE_MU,
	CALI_FORMAT_MOD_HE_ER,
	CALI_FORMAT_MOD_HE_TB,
#if defined(CFG_MERAK3000)
	CALI_FORMAT_MOD_EHT_MU,
	CALI_FORMAT_MOD_EHT_TB,
	CALI_FOTMAT_MOD_EHT_ER,
#endif
	CALI_FORMAT_MOD_MAX
};

enum cali_mac_ppdu_type {
	CALI_MAC_PPDU_TYPE_EHT_MU_OFDMA,
	CALI_MAC_PPDU_TYPE_EHT_MU_SU,
	CALI_MAC_PPDU_TYPE_EHT_MU_MIMO,
	CALI_MAC_PPDU_TYPE_MAX
};

enum cali_packet_ext {
	CALI_PACKET_EXT_0US = 0,
	CALI_PACKET_EXT_8US = 8,
	CALI_PACKET_EXT_16US = 16,
	CALI_PACKET_EXT_20US = 20,
};

enum cali_preamble_type {
	CALI_PREAMBLE_TYPE_SHORT,
	CALI_PREAMBLE_TYPE_LONG,
	CALI_PREAMBLE_TYPE_MAX
};

enum cali_sounding_ng {
	CALI_SOUNDING_NG_4,
	CALI_SOUNDING_NG_16,
	CALI_SOUNDING_NG_MAX
};

enum cali_sounding_type {
	CALI_SOUNDING_TYPE_VHT,
	CALI_SOUNDING_TYPE_HE,
	CALI_SOUNDING_TYPE_MAX
};

enum cali_sounding_feedback_type {
	CALI_SOUNDING_FEEDBACK_TYPE_SU,
	CALI_SOUNDING_FEEDBACK_TYPE_MU,
	CALI_SOUNDING_FEEDBACK_TYPE_MAX
};

enum CALI_MU_ACK_POLICY {
	CALI_MU_ACK_POLICY_SEQ_BAR,
	CALI_MU_ACK_POLICY_MU_BAR,
	CALI_MU_ACK_POLICY_EMBEDDED_TRIG,
	CALI_MU_ACK_POLICY_TRS,
	CALI_MU_ACK_POLICY_INVALID,
};


#define CALI_TID_NUM			8
#define	CALI_AC_NUM			4
#define CALI_MU_USER_NUM		16
#define CALI_UL_MU_START_NUM		CALI_MU_USER_NUM
#define CALI_BI_MU_USER_NUM		32
/* TODO:In Merak2000 add IPC command to pass CBF from HOST to WPU */
#if defined(CFG_MERAK3000)
#define CALI_MAX_CBF_REPORT_SIE		3016
#elif defined(MERAK2000) || defined(CFG_MERAK2000)
#define CALI_MAX_CBF_REPORT_SIE	    0
#else
#define CALI_MAX_CBF_REPORT_SIE		1016
#endif
#define CALI_CFG_MAX_IGTK_NUM		2
#define CALI_CFG_MAX_VAP_NUM 		32
#define CALI_INVALID_VALUE		0xADADADAD


#if defined(CFG_MERAK3000)
/*
 * The relationship between mld_id & mld_tbl_idx on cali-mode:
 *	On AP side, the ap_mld_id == ap_mld_tbl_idx
 *
 *	On STA side, it's "ap_mld_id" indicates which AP-MLD it belongs to.
 *		In general sta_mld_tbl_idx is same as "ap_mld_id".
 *		And when we config "ap_mld_id", also update "sta_mld_tbl_idx".
 *		But we can overwrite sta_mld_tbl_idx(config it after ap_mld_id)
 *		To test other entris.
 *
 */
#define CALI_CFG_MAX_AP_MLD_NUM		32
#define CALI_CFG_MAX_LINK_NUM		16
#define CALI_CFG_INVALID_LINK_ID	15

#define CALI_CFG_MAX_STA_MLD_NUM	CALI_MU_USER_NUM

#define CALI_LINK_ID_2G			0
#define CALI_LINK_ID_5G			1
#define CALI_LINK_ID_6G			2 /* 5G or 6G in another M3K */

enum CALI_MLO_MODE {
	CALI_MLO_MODE_NONE = 0,	// Not MLO operation.
	CALI_MLO_MODE_STR,
	CALI_MLO_MODE_NSTR,
	CALI_MLO_MODE_MLSR,
	CALI_MLO_MODE_EMLSR,
	CALI_MLO_MODE_EMLMR,
	CALI_MLO_MODE_MAX,
};

enum CALI_EML_INIT_CTRL_TYPE {
	CALI_EML_INIT_CTRL_TYPE_MU_RTS,
	CALI_EML_INIT_CTRL_TYPE_BSRP,
	CALI_EML_INIT_CTRL_TYPE_QOS_NULL,
};

#endif // CFG_MERAK3000

#define CALI_ADDR_COPY(_d, _s)	{_d[0] = _s[0]; _d[1] = _s[1]; _d[2] = _s[2]; _d[3] = _s[3]; _d[4] = _s[4]; _d[5] = _s[5]; }

#define CAL_CFG_LOCAL_ADDR(_cfg)	((_cfg)->bss_info.local_addr)
#define CAL_CFG_BSSID_AP(_cfg)		CAL_CFG_LOCAL_ADDR(_cfg)
#define CAL_CFG_BSSID_STA(_cfg)		((_cfg)->su_user_info.peer_addr)

#define CAL_CFG_VAP_BSSID_AP(_cfg, idx)	((_cfg)->bss_info.mbss_info[idx].local_addr)
#define CSI_MAX_SUPPORTED_USER 8
struct cali_csi_info_tag {
	/* 0:off; 1:on */
	uint32_t csi_on;
	/* 1 bit for each format, default 0xFFFFFFFF */
	uint32_t format_on;
	/* 1 : upload csi info of BA/ACK/TB frame*/
	uint8_t csi_flag;
//	uint32_t user_on[CSI_MAX_SUPPORTED_USER];

	/* Base address to store CSI */
	uint32_t base_addr;
	uint32_t bank_size;
	uint32_t bank_num;

	/* PHY configuration */
	uint32_t smp_mode_sel;
	uint32_t smp_cont_sel0;
	uint32_t smp_cont_sel1;
	uint32_t csi_smpout_send_wait;

	/* PPDU threshold in data symbol */
	uint32_t rx_ppdu_symb_thresh;
	/* sample period (20us * N)*/
	uint32_t rx_time_thresh;

	/* sampling granularity */
	uint8_t l_ltf_gran;
	uint8_t non_he_ltf_gran[CALI_PHY_BW_MAX];
	uint8_t he_ltf_gran[CALI_PHY_BW_MAX];
};

struct cali_key_info_tag {
	uint8_t		key_addr[6];

	// key_len=number
	uint16_t	key_len;
	// key_index = number
	uint16_t	key_index;

	uint16_t	key_hw_index;	// Pointer to the HW key-index. Move to a current-configure structure(In firmware). TBD.

	// "cipher=string". Valid string: 0: none / 1: WEP / 2: TKIP / 3: CCMP / 4: WAPI / 5: GCMP
	// "6: BIP-CMAC-128, 7: BIP-CMAC-256"
	// @ref enum cali_cipher_type
	uint16_t	cipher;

	// "key=01 23 45 .....", depends on key_len, max 256 bits.
	uint8_t		key[256 / 8];

	uint8_t		bw_signaling_force;
};

struct cali_ba_info_tag {
	//"ba_tid=number"
	uint32_t ba_tid;

	// "ba_tx_size=number"
	uint32_t ba_tx_size;

	// "ba_rx_size=number"
	uint32_t ba_rx_size;

	// ack or back: Depends on ppdu type: singletone / a-mpdu
	//"ba_ack_policy=string". Valid string: 0: no_ack / 1: normal / 2: implicit
	// @ref enum cali_ba_ack_policy
	uint32_t ba_ack_policy;

	// "ba_ssn=number"
	uint32_t ba_ssn;
};

//
// 802.11 MAC frame format:
//	 2 Byte: FC
//	 2 Byte: Duration(2 Bytes)
//  18 Byte: Addr-1 / Addr-2 / Addr-3
//   2 Byte: Sequence Control
//   2 Byte: Addr-4
//   2 Byte: QoS Control
//   2 Byte: HT Control
struct cali_ppdu_info_tag {
	// "protection=0/1".	Send protection frame or not.
	//  If it's 1, need to set prot_xxx(WMAC/Phy parameters).
	uint32_t	protection;

	// "frame_ctrl=Hex_value". See 802.11 for type/subtype definition.
	// Only set Type/Subtype.
	uint32_t	frame_ctrl; // 0xXX

	// "duration=number", If it's CALI_INVALID_VALUE, don't use this value.
	// Maybe Hw will set it when Tx-ing.
	uint32_t	duration;

	// "seq_control=Hex_value". Not sure we need to configure it.
	uint32_t	seq_ctrl;	// TBD

	// "qos_ctrl=Hex-value"
	uint32_t	qos_ctrl;	// Not sure we need to configure it directly. It depends on the frame-type & A-MSDU & TID.

	// "ht_ctrl=Hex_value"
	uint32_t	ht_ctrl;

	// "ampdu_num=number". 0 means no A-MPDU. Will send singleton MPDU.
	uint32_t	ampdu_num;

	// "amsdu_num=number". 0 means no A-MSDU. Encap msdu only
	uint32_t	amsdu_num;

	// "msdu_len=number"
	uint32_t	msdu_len;

	//"apep_len=number"
	uint32_t	apep_len;

	// "payload_pattern=Hex-value", The payload value.
	uint32_t	payload_pattern;

	// "ack_policy=0/1/2/3"		// 0:normal ack, 1:no ack, 2:no explicit ack, 3:block ack
	// @ref enum cali_ack_policy
	uint32_t	ack_policy;

	// "ssn=number"
	uint32_t	ssn;

	// "ssn_reset=0/1" control the set of ssn
	uint32_t	ssn_reset;

	// "ppdu_bf=0/1". With or Without Matrix.
	uint32_t	ppdu_bf;

	// "ppdu_encry=0/1".
	uint32_t	ppdu_encry;

	// "ignore_cca=0/1"
	uint32_t	ignore_cca;

	//"non_qos_ack=0/1" // 0:no ack, 1:ack
	uint32_t	non_qos_ack;

	//"bw_signal_en=number"
	uint32_t	bw_signal_en;

	//"sr_disallow=0/1" // 0:allow, 1:disallow
	uint8_t 	sr_disallow;
#if defined(CFG_MERAK3000)
	//"hw_llc_encap_en=0/1" // 0:disable, 1:enable
	uint8_t 	hw_llc_encap_en;
	//"sr_drop_pwr=0/1" // 0:disable, 1:enable
	uint8_t 	sr_drop_pwr_en;
	//"sr_adj_mcs_delta=xx" // sr adj mcs delta
	uint8_t 	sr_adj_mcs_delta;
#endif
};

// "cal set tid=number aifsn=number ecw_min=number ecw_max=number"
struct cali_wmm_params_tag {
	uint32_t	aifsn;
	uint32_t	cw_min;
	uint32_t	cw_max;
};

struct cali_sounding_info_tag {
	//"type=0/1/2". 0: VHT; 1: HE, 2: EHT
	// @ref enum cali_sounding_type
	uint32_t type;

	//"nb_sta=number". sounding stations 1 time
	uint32_t nb_sta;

	//"feedback_type=0/1". 0: SU; 1: MU
	// @ref enum cali_sounding_feedback_type
	uint32_t feedback_type;

	//"nc_idx=number". Value of Nc
	uint32_t nc_idx;

	//"he: ng=0/1". 0: Ng4; 1:Ng16
	//"vht: ng=0/1/2". 0: Ng1; 1:Ng2; 2:Ng4
	// @ref enum cali_sounding_ng
	uint32_t ng;

	// "eht: cb=0/1"
	uint32_t codebook_size;

	//"debug sequence: 0x1: NDPA; 0x2: NDP; 0x4: BFRP; 0x8: report"
	uint32_t sequence;
};

struct cali_pppc_info_tag {
	uint8_t pppc_en;
	int32_t template_power_ppducnt[10][2];
	uint8_t template_num;
};


struct cali_mu_info_tag {
	// "mu_type=0/1/2/3: 0: DL OFDMA; 1: DL MIMO; 2: UL OFDMA 3: UL MIMO"
	uint8_t mu_type;

	// "mu_ack=0/1/2/3: 0: seqbar; 1: mubar; 2: embtrig; 3: trs"
	uint8_t mu_ack;

	// "mu_prot=0/1". 0: disable MU protection; 1: enable MU protection
	uint8_t mu_prot;

	// "tb_hw=0/1: 0: tb hw calculation off; 1: tb hw calculation on"
	uint8_t tb_hw;

	// "ul TB time (unit: 0.1 us)"
	uint32_t ul_duration;
	uint32_t en_sample;

	// "ul mu station start number"
	uint32_t ul_mu_start_num;

	// "pre_fec_pad_factor"
	uint32_t pre_fec_pad_factor;

	// "ldp_extra_symbol"
	uint32_t ldpc_extra_sym;

	// "pe_disambiguity"
	uint32_t pe_disambiguity;

	// "nsymb_choose=0/1: 0: round down; 1: round up"
	uint8_t nsymb_choose;
};


struct cali_user_info_tag {
	uint8_t		tx_tid;
	uint8_t		lock_csi;
	uint8_t		peer_addr[6];

	uint32_t	paid;				// Maybe we can calculate it.
	uint32_t	aid;
	uint32_t	sta_index;

	// BA information for tx_tid.
	// Need to reset ba_info if tx_tid be changed.
	struct cali_ba_info_tag		ba_info;

	struct cali_ppdu_info_tag	ppdu_info;

	//struct cal_mu_info_tag		mu_info;

	uint32_t	spp;

#if defined(CFG_MERAK3000)
	// "mld_id=number"
	// Which MLD this STA belong to.
	uint8_t ap_mld_id;

	// "link_id=number"
	uint8_t link_id;
	uint8_t is_eht;
	uint8_t mld_pad[1];
#endif
};

struct cali_per_user_mac_phy_params_tag {
	// 1 bit. FEC coding, ldpc or bcc
	// "fec=0/1".  0: BCC, 1: LDPC
	// @ref enum enum cali_fec_coding
	uint32_t	fec;

	// 3 bits. Packet Extension Value(us)
	// "packet_extension=0/8/16".
	//		 0: 0us.  --> PE_0US(=0)
	//		 8: 8us.  --> PE_8US(=2)
	//		16: 16us. --> PE_16US(=4).
	// @ref enum cali_packet_ext
	uint32_t	packet_ext;

	// 3 bits. Minimum MPDU start Spacing for the AMPDU
	// "min_mpdu_space=string".
	// Valid string: 0: "0" / 1: "0.25" / 2: "0.5" /3: 1/ 4: "2" / 5: "4" / 6: "8" / 7: "16"
	// Map to MMSS_0US/.../MMSS_16US
	uint32_t	min_mpdu_space;

	// "eof padding=n bytes"
	// add to the end of AMPDU
	uint32_t eof_padding;
	// rtsThreshold / shortRetryLimit / LongRetryLimit

	// 1bit, 0 or 1
	// decrease retry mcs in policy table
	uint32_t retry_rate_dec;

	// 1 bit. dcm
	// "dcm=0/1"
	uint32_t	dcm;

	// mcs & index.
	// "mcs=number". Format : For HT, it's MCS index.
	//			  For VHT/HE/EHT, it's 0xAB(A: nSS - 1, B: MCS index)
	uint32_t	mcs;

	// "mcs_legacy=number". Format : For legacy-rate, it's 1/2/5/11/6/9/12/18/24/36/48/54.
	uint32_t	mcs_legacy;

	// "cbf_host_addr=addr"
	uint32_t	cbf_host_addr;
#if defined(CFG_MERAK3000)
	uint8_t		cbf_report[0];
#else
	uint8_t		cbf_report[CALI_MAX_CBF_REPORT_SIE];
#endif
	// "cbf_report_len=number"
	uint32_t	cbf_report_len;

	// "user_bf: 0/1"
	uint32_t	user_bf;
	//
	// MU related parameters
	//

	// 2 bits. User position
	// "user_pos=number"
	uint32_t	user_pos;

	// 3 bits. Starting STS
	// "start_sts=number"
	uint32_t	start_sts;

	// "ss_num=number"
	uint32_t	ss_num;		// For 2x2, seems needn't configure it.

	// "ru_index=number"
	uint32_t	ru_index;

	// "ru_size=number"
	uint32_t	ru_size;

	// "ru_offset=number"
	uint32_t	ru_offset;

	// "ru_power_factor=number"
	uint32_t	ru_power_factor;

	// "dropMCSEnTx=0/1"
	uint32_t 	dropMCSEnTx;

	//"pow_split_en=0-7, 3 bit"
	uint32_t pow_split_en;
};

//
// number : Hex(0xXX) or Dec.
//
struct cali_mac_phy_params_tag {
	// MAC parameters

#if defined(CFG_MERAK3000)
	// 2 bits. mac_ppdu_type
	// "mac_ppdu_type=string". Valid string: 0: EHT-MU-OFDMA / 1: EHT-MU-SU / 2: EHT-MU-MIMO
	// @ref enum cali_mac_ppdu_type
	uint32_t	mac_ppdu_type;

	// 1 bit. use_non_standard_ta_func_en
	// "non_standard_ta_func=0/1"
	uint8_t	non_standard_ta_func;

	// 1 bit. non_standard_ta
	// "non_standard_ta=0/1"
	uint8_t	non_standard_ta;

	// 1 bit. dyn_bw
	// "dyn_bw=0/1"
	uint8_t	dyn_bw;

	// 1 bit. txbf filter enable
	// "txbf_filter_en=0/1"
	uint8_t	txbf_filter_en;

	// 12 bit. txbf alpha
	// "txbf_alpha=value"
	uint16_t	txbf_alpha;

	// 12 bit. txbf smoothing
	// "txbf_smooth=0/1"
	uint8_t	txbf_smooth;
#endif

	// Phy parameters.
	//
	// 4 bits. format-mod
	// "format_mod=string". Valid string: 0: NON-HT / 1: NON-HT-DUP-OFDM / 2: HT-MF / 3: HT-GF
	//					4: VHT / 5: HE-SU / 6: HE-MU / 7: HE-ER-SU
	//					8: HE-TB / 9: EHT-MU
	// @ref enum cali_format_mod
	uint32_t	format_mod;

	// 4 bits. Spatial Reuse information.
	// "spatial_resue=number"
	uint32_t	spatial_reuse;

	// 3 bits. Number of Transmit Chains for protection frame.
	// "n_tx_prot=number"
	uint32_t	n_tx_prot;

	// 3 bits. Number of Transmit Chains for PPDU.
	// "n_tx=number"
	uint32_t	n_tx;

	// TBD.
	// 1 bit. Midamble periodicity.
	// "midable=0/1"
	uint32_t	midable;

	// 1 bit. Doppler.
	// "doppler=0/1"
	uint32_t	doppler;

	// 1 bit. Space Time Block Coding.
	// "stbc=0/1"
	uint32_t	stbc;

	// 2 bits. Number of Extension Spatial Streams
	// "num_ext_nss=number"
	uint32_t	num_ext_nss;

	// 6 bits.	BSS color
	// "bss_color=number"
	uint32_t	bss_color;

	// "group_id=number". Range: 0~63
	uint32_t	group_id;

	// 8 Bits. smm_index
	// "smm_index=""
	uint32_t	smm_index;	// Keep it as 0. Reserved for future use.

	// 8 bits. antenna set
	// "antenna_set=number"
	uint32_t	antenna_set;

	// 8 bits. Tx Power Level for the PPDU.
	// "tx_power_level=number"
	// E.g. 0x80(-128 dBm), 0xFF(-1 dBm), 0x00(0 dBm), 0x1(1 dBm), 0x3F(127dBm).
	uint32_t	tx_power_level;

	// 2 bits. HE-LTF Type.
	// "he_ltf=value".  Valid value: 1/2/4
	// Map to HE_LTF_1X(=0, 3.2us), HE_LTF_2X(1, 6.4us), HE_LTF_4X(2, 12.8us)
	// @ref enum cali_he_ltf_type
	uint32_t	he_ltf;

	// 2 bit.
	// "gi=string". Valid string: 0: "long" / 1: "short" / 2: "0.8" / 3: "1.6" / 4: "3.2"
	uint32_t	gi;

	// 1 bit. Preamble Type(DSSS/CCK).
	// "preamble_type=0/1"
	// @ref enum cali_preamble_type
	uint32_t	preamble_type;

	// 3 bits. band width of PPDU
	// "bw_ppdu=number". Valid value: 20/40/80/160/320/8080(80 + 80)
	uint32_t	bw_ppdu;

	//
	// Protection rate-control(RTS-CTS)
	//
	// 3 bits. NAV protection frame exchange.
	// "prot_nav_frm_ex=string". Valid string: 0: "none" / 1: "self_cts" / 2: "rts_cts" / 3: "rts_cts_qap" / 4: "stbc"
	// @ref enum cali_prot_nav_frm_ex
	uint32_t	prot_nav_frm_ex;

	// 8 bits. Tx power for prot-frame.
	// "prot_tx_power=value".  See "tx_power"
	uint32_t	prot_tx_power;

	// 4 bits. format and modulation of protection frame.
	// "prod_format_mod=string", see "format_mod"
	uint32_t	prot_format_mod;

	// 1 bit. preamble type of protection frame.
	// "prot_preamble_type=0/1", see "preamble_type"
	uint32_t	prot_preamble_type;

	// 3 bits. Band width of protection frame.
	// "prot_bw=string", see "bw_ppdu"
	uint32_t	prot_bw;

	// 7 bits. MCS index of protection frame.
	// "prot_mcs=value", see "mcs"
	uint32_t	prot_mcs;

	//
	// MU related parameters
	//
	// 4 bits. Dynamic preamble puncturing type
	// "dyn_pre_punc_type=value", TBD.
	uint32_t	dyn_pre_punc_type;

	// 3 bits. number of HE-LTF symbols
	// "num_he_ltf=number". = "Number of HE-LTF symbols - 1"
	uint32_t	num_he_ltf;

	// 2 bits. number of EHT-LTF symbols
	// "num_eht_ltf=number". = "Number of EHT-LTF symbols - 1"
	uint32_t	num_eht_ltf;

	// 2 bits. Center26ToneRU
	// "center_26tone_ru=number"
	uint32_t center_26tone_ru;

	// 1 bit.
	// "sigB_comp=0/1".
	uint32_t	sigB_comp;

	// 1 bit.
	// "sigB_dcm=0/1"
	uint32_t	sigB_dcm;

	// 3 bits.
	// "sigB_mcs=number"
	uint32_t	sigB_mcs;

	// 64-bits.
	// sigB_ru_alloc_1=hex-value" / "sigB_ru_alloc=hex-value"
	uint32_t	sigB_ru_alloc_1;
	uint32_t	sigB_ru_alloc_2;

	// 1 bit
	// "beamformed=0/1"
	uint32_t	beamformed;

	// 8bits
	// Indicates the 20 MHz subchannels that are punctured from lowest 20MHz frequency
	uint32_t	inact_subchan_bitmap;

	// "dropbwentx=0/1"
	uint32_t	dropbwentx;

	// "txopdurationen=0/1"
	uint32_t 	txopdurationen;

	// "txopduration=number" 7bit
	uint32_t 	txopduration;

	// "txop_txtime_overcut_en=0/1"
	uint32_t txop_txtime_overcut_en;

	// "hw_retry_en=0/1"
	uint32_t hw_retry_en;

	// smart antenna enable
	uint32_t smart_ant_en;

	// smart antenna parameter 0
	uint32_t smart_ant_param0;

	// smart antenna parameter 1
	uint32_t smart_ant_param1;

	// 4 bits.
	// "sr_ppdu_min_mcs=number"
	uint32_t	sr_ppdu_min_mcs;

	// 4 bits.
	// "mpdu_retry_limit=number"
	uint8_t	mpdu_retry_limit;

	// 4 bits.
	// "no_just_hard_retry_limit=number"
	uint8_t	no_just_hard_retry_limit;

	// 4 bits.
	// "prot_retry_limit=number"
	uint8_t	prot_retry_limit;
};

struct cali_vap_tag {
	// "mac_mode=string". Valid string: 0: "AP" / 1: "STA"
	// @ref enum cali_mac_mode
	uint32_t	mac_mode;

	// "local_addr=00:01:02:03:04:05"
	uint8_t		local_addr[6];
	uint8_t		resv[2];

	struct cali_key_info_tag	key_gtk;
};

struct cali_bss_tag {
	// "radio=2g/5g". Valid string: 0: "2g" / 1: "5g"
	uint32_t	radio;			// enum mac_chan_band

	// "mac_mode=string". Valid string: 0: "AP" / 1: "STA"
	// @ref enum cali_mac_mode
	uint32_t	mac_mode;

	// "phy_mode=string", valid string:
	// 0: "11a" / 1: "11b" / 2: "11g" / 3: "11n" / 4: "11ac" / 5: "11ax" / 6: "11be"
	// @ref enum cali_phy_mode
	uint32_t	phy_mode;

	// "local_addr=00:01:02:03:04:05"
	uint8_t		local_addr[6];
	uint8_t		vap_num;
	uint8_t		mfp;

	// "bw=number".	20/40/80/160/320/8080. (8080=80+80)
	uint32_t	bw;		// enum mac_chan_bandwidth

	// "chan=number".		// IEEE no: 1--14 for 2.4G or 36/40/....
	uint32_t	chan_ieee;

	// "bw_change=0/1/2" 0: keep same, 1: ascent, 2: descent
	uint32_t bw_change;

	struct cali_key_info_tag	key_gtk;
	struct cali_key_info_tag	key_igtk[CALI_CFG_MAX_IGTK_NUM];
	struct cali_vap_tag		mbss_info[CALI_CFG_MAX_VAP_NUM - 1];

#if defined(CFG_MERAK3000)
	// "ap_mld_id=number"
	// Which MLD this BSS affiliated with.
	uint8_t ap_mld_id;

	// "link_id=number"
	uint8_t link_id;

	uint8_t mld_pad[2];
#endif
};

#if defined(CFG_MERAK3000)
struct cali_ap_mld_tag {
	//"mld_addr=00:01:02:03:04:05"
	uint8_t		mld_addr[6];

	//"mld_id=number" : number < CALI_CFG_MAX_MLD_NUM
	uint8_t		ap_mld_id;

	// The store-entry.
	// Needn't set it, it's equal with ap_mld_id;
	uint8_t 	ap_mld_tbl_idx;

	//"valid_links=number" For example: 5 means this MLD contains link-0 / link-1 / link-2.
	// If it's non-ZERO, this MLD is active, otherwise, WPU wouldn't config this MLD.
	uint16_t	valid_links;

	// Needn't config it, when set other fields, will set it to 1.
	uint8_t		is_mld;

	uint8_t		pad_2[1];
};

struct cali_sta_mld_tag {
	//"mld_addr=00:01:02:03:04:05"
	uint8_t		mld_addr[6];

	// Neednt config it.
	uint8_t		is_mld;

	// Following for STA-MLD.
	// "sta_mlo_mode=str/nstr/mlsr/emlsr/emlmr"
	uint8_t		sta_mlo_mode;

	//"valid_links=number" For example: 5 means this MLD contains link-0 / link-1 / link-2.
	// If it's non-ZERO, this MLD is active, otherwise, WPU wouldn't config this MLD.
	uint16_t	valid_links;

	// "link_pair_bmp=number" : For NSTR / STR. Eg. 3 = link_id_0 + link_id_1
	//uint16_t	link_pair_bmp;

	// "sta_mld_tbl_idx=number"
	// The store-entry. If not set, same as ap_mld_id
	uint16_t	sta_mld_tbl_idx;


	// "eml_padding_delay=number" : 0/32/64/128/256 us
	// The minimum MAC padding duration of the init-control-frame.
	// STA's capability.
	uint16_t	eml_padding_delay;

	// "eml_transition_delay=number" : 0/32/64/128/256 us
	// The transition delay time needed by a STA-MLD to switch from exchanging PPDUs
	// on one of the enabled links to the listening operation on the enabled links.
	// STA's capability.
	uint16_t	eml_transition_delay;

	// "eml_init_frame_type=mu_rts/bsrt/qos_null"
	// this is a global-ctrl, not per-sta.
	uint8_t		eml_init_frame_type;

	//"mld_id=number" : number < CALI_CFG_MAX_MLD_NUM
	// Only support "one AP-MLD + multi STA-MLDs", so, use working_ap_mld_id.
	// If need to support "multi-AP + multi STA", improve it later.
	// uint8_t		ap_mld_id;
};

struct cali_tdma_tag {
	// 1: enable tdma
	uint8_t tdma_en;
	// 0: AP mode / 1: STA or repeater mode
	uint8_t tdma_mode;
	// duration of wait ack
	uint16_t tdma_ack_duration;
};

#endif // CFG_MERAK3000


#define CALI_CFG_MAGIC		0x6a7b8c9d
struct cali_config_tag {
	// "mu_users=number". How many users be used when Tx/Rx MU.
	uint32_t	mu_users;

	// "tx_ppdu_cnt=number". How many ppdus we want to Tx, 0 means for-ever.
	uint32_t	tx_ppdu_cnt;

	// "tx_interval=number". The delay bwetwen to PPDU. Unit: us.
	uint32_t	tx_interval;

	uint32_t	current_ac;

	// enable/diasble get rx statistics
	uint32_t	en_rxstats;

	struct cali_wmm_params_tag			wmm[CALI_AC_NUM];

	struct cali_bss_tag				bss_info;

	struct cali_key_info_tag		key_ptk[CALI_MU_USER_NUM];

	struct cali_mac_phy_params_tag	mac_phy_params;

	struct cali_user_info_tag			su_user_info;
	struct cali_per_user_mac_phy_params_tag		su_mac_phy_params;
	uint32_t	magic;

	struct cali_mu_info_tag				mu_info;
	struct cali_user_info_tag			mu_user_info[CALI_BI_MU_USER_NUM];
	struct cali_per_user_mac_phy_params_tag		mu_mac_phy_params[CALI_BI_MU_USER_NUM];
	struct cali_sounding_info_tag			sounding_info;
	struct cali_csi_info_tag	csi_info;
	struct cali_pppc_info_tag	pppc_info;

	// For non-MLD test, chipset_id is 0.
	// For MLD test(M3K + M3K), the chipset_id is 0 / 1.
	// On chipset-0 : 2.4G + 5G.
	// On chipset-1 : 5G(6G) only, 2.4G is inactive.
	//
	uint32_t			chipset_id;

#if defined(CFG_MERAK3000)
	// "mlo_enable=0/1"
	uint8_t				mlo_enable;

	// "chipset_num=number"   1 : Single M3K.   2: M3K + M3K
	uint8_t				chipset_number;

	// "working_ap_mld_id=number"
	// If config several MLD-AP, the one is working one and tx/rx via this MLD.
	// Default : 0.
	uint8_t				working_ap_mld_id;

	uint8_t				mld_pad;

	// "=number" : If value is 128/256/512, its us. If value is 0/1/2/4/8/16/32, its TUs.
	// The Transition Timeout dicicates the timeout value for EML OMN frame exchange in
	// EMLSR/EMLMR mode.
	// After successful transmission of the EML OMN frame from the STA-MLD,
	// The STA-MLD init the transition timeout timer with the value in the Transition Timeout
	// subfield of the Basic ML element received from AP-MLD.
	// After transmitting the Ack frame solicited by the EML OMN frame from STA-MLD,
	// The AP MLD init the transition timeout timer with the valule in the Transition Timeout
	// subfield of the Basic ML element announced by the AP MLD.
	// See 35.3.18.
	uint16_t			transition_timeout;

	// hw_agg=0/1
	uint8_t				hw_agg_enable;
	// hw_agg_dup=0/1
	uint8_t				hw_agg_dup;

	struct cali_ap_mld_tag		ap_mld_info[CALI_CFG_MAX_AP_MLD_NUM];

	// And use "su_user_info" as link-STA
	struct cali_sta_mld_tag		sta_mld_info[CALI_CFG_MAX_STA_MLD_NUM];

	// str / nstr / emlsr / emlmr be set in "sta_mld_info"

	// "group_win_start=number"
	uint32_t	group_win_start;
	// "group_win_end=number"
	uint32_t	group_win_end;

	struct cali_tdma_tag tdma_info;
#endif // CFG_MERAK3000
};

void cali_set_default_config(struct cali_config_tag *cali_cfg_ptr, enum cali_phy_band phy_band);

void cali_cfg_to_local(struct cali_config_tag *cali_cfg_ptr, enum cali_phy_band phy_band);

extern struct cali_config_tag local_cali_cfg;

#ifdef CALI_DEF_CONFIG_IMPL

#if defined(CFG_MERAK3000)
static const uint8_t cali_def_self_mld_mac[6] = {0x00, 0xaa, 0xaa, 0x00, 0x00, 0x00};
static const uint8_t cali_def_peer_mld_mac[6] = {0x00, 0x55, 0x55, 0x00, 0x00, 0x00};
#endif // CFG_MERAK3000

static const uint8_t cali_def_self_mac[6] = {0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
static const uint8_t cali_def_peer_mac[6] = {0x00, 0x55, 0x55, 0x55, 0x55, 0x00};
static const uint8_t cali_def_mu_peer_mac[CALI_MU_USER_NUM][6] = {
	{0x00, 0x55, 0x55, 0x55, 0x55, 0x00}, /*Same as cali_def_peer_mac */
	{0x00, 0x66, 0x66, 0x66, 0x66, 0x00},
	{0x00, 0x77, 0x77, 0x77, 0x77, 0x00},
	{0x00, 0x88, 0x88, 0x88, 0x88, 0x00},

	{0x00, 0x99, 0x99, 0x99, 0x99, 0x00},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x01},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x02},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x03},

	{0x00, 0x99, 0x99, 0x99, 0x99, 0x04},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x05},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x06},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x07},

	{0x00, 0x99, 0x99, 0x99, 0x99, 0x08},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x09},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x0a},
	{0x00, 0x99, 0x99, 0x99, 0x99, 0x0b}
	};
static const uint8_t cali_broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void cali_set_default_ppdu_params(struct cali_ppdu_info_tag *ppdu_ptr, enum cali_phy_band phy_band)
{
	ppdu_ptr->protection = 0;		// No CTS-RTS
	ppdu_ptr->frame_ctrl = 0x88;		// QoS Data
	ppdu_ptr->duration = 0;			// HW update it.
	ppdu_ptr->seq_ctrl = 0;
	ppdu_ptr->ampdu_num = 1;
	ppdu_ptr->amsdu_num = 1;		// No A-MSDU
	ppdu_ptr->msdu_len = 128;
	ppdu_ptr->ack_policy = 1;		//NO Ack
	ppdu_ptr->ppdu_bf = 0;
	ppdu_ptr->ppdu_encry = 0;
	ppdu_ptr->sr_disallow = 1;
#if defined(CFG_MERAK3000)
	ppdu_ptr->hw_llc_encap_en = 0;
	ppdu_ptr->sr_drop_pwr_en = 1;
	ppdu_ptr->sr_adj_mcs_delta = 0;
#endif
}

#if defined(CFG_MERAK3000)
uint8_t cali_phy_band_to_link_id(uint32_t chipset_id, enum cali_phy_band phy_band)
{
	// TODO. Change it after discuessing with Tiger later...
	switch (phy_band) {
	case CALI_PHY_BAND_2G4:
		if (chipset_id) {
			// For M3K + M3K, only one 2.4G is active
			// and still return 0.
		}
		return 0;
	case CALI_PHY_BAND_5G:
		return chipset_id ? 2 : 1;
	default:
		return 0xFF;
	break;
	}
}

uint32_t cali_get_ap_mld_tbl_idx_by_mld_id(const struct cali_config_tag *cali_cfg, uint32_t ap_mld_id)
{
	uint32_t i;

	for (i = 0; i < CALI_CFG_MAX_AP_MLD_NUM; i++) {
		if ((cali_cfg->ap_mld_info[i].is_mld) &&
					(cali_cfg->ap_mld_info[i].ap_mld_id == ap_mld_id))
			return cali_cfg->ap_mld_info[i].ap_mld_tbl_idx;
	}

	return 0xFFFFFFFF;
}

void cali_set_default_tdma_info(struct cali_tdma_tag *tdma_info)
{
	memset((void *)tdma_info, 0, sizeof(struct cali_tdma_tag));
	// must be >= 9
	tdma_info->tdma_ack_duration = 9;
}

#endif // CFG_MERAK3000

void cali_set_default_user_info(struct cali_user_info_tag *user_info_ptr,
		const uint8_t *mac_addr, enum cali_phy_band phy_band, int user_index,
		uint32_t chipset_id)
{
	CALI_ADDR_COPY(user_info_ptr->peer_addr, mac_addr);

	user_info_ptr->tx_tid = 0;		// User tid 0 to Tx MPDU.

	user_info_ptr->paid = 0x0;		// Need to calculate it later.

	user_info_ptr->aid = 100 + user_index;
	user_info_ptr->ba_info.ba_tid = user_info_ptr->tx_tid;

	user_info_ptr->ba_info.ba_tx_size = 64;
	user_info_ptr->ba_info.ba_rx_size = 64;

	user_info_ptr->ba_info.ba_ack_policy = 1;	// Ack/Back
	user_info_ptr->ba_info.ba_ssn = 0;

	cali_set_default_ppdu_params(&user_info_ptr->ppdu_info, phy_band);

#if defined(CFG_MERAK3000)
	user_info_ptr->ap_mld_id = (user_index >= CALI_UL_MU_START_NUM) ?
					user_index - CALI_UL_MU_START_NUM : user_index;
	user_info_ptr->link_id = cali_phy_band_to_link_id(chipset_id, phy_band);
#endif
}

void cali_set_default_mac_phy_params(struct cali_mac_phy_params_tag *mac_phy_ptr, enum cali_phy_band phy_band)
{
	mac_phy_ptr->format_mod = (phy_band == CALI_PHY_BAND_2G4) ? 5 : 5; // 11AX.

	mac_phy_ptr->spatial_reuse = 0;
	mac_phy_ptr->n_tx_prot = 0;
	mac_phy_ptr->n_tx = 1;
	mac_phy_ptr->midable = 0;
	mac_phy_ptr->doppler = 0;
	mac_phy_ptr->stbc = 0;
	mac_phy_ptr->antenna_set = (phy_band == CALI_PHY_BAND_2G4) ? 3 : 7;
	mac_phy_ptr->tx_power_level = 13;	// 13dBm.
	mac_phy_ptr->he_ltf = 4;	// 4x
	mac_phy_ptr->gi = 2;	// GI 0.8us
	mac_phy_ptr->bw_ppdu = (phy_band == CALI_PHY_BAND_2G4) ? 20 : 20;
	mac_phy_ptr->prot_nav_frm_ex = CALI_PROT_NAV_FRM_EX_RTS_CTS; // rts - cts
	mac_phy_ptr->prot_format_mod = CALI_FORMAT_MOD_NON_HT_DUP_OFDM; // NON-HT-DUP
	mac_phy_ptr->prot_preamble_type = CALI_PREAMBLE_TYPE_LONG; // long preamble
	mac_phy_ptr->prot_bw = 20;
	mac_phy_ptr->prot_mcs = 6;
#if defined(DUBHE2000) && DUBHE2000
	mac_phy_ptr->hw_retry_en = 1;
#else
	mac_phy_ptr->hw_retry_en = 0;
#endif

	/* Set SMM index to 2 by default */
	mac_phy_ptr->smm_index = 2;
	mac_phy_ptr->smart_ant_en = 0;
	mac_phy_ptr->sr_ppdu_min_mcs = 0;
#if defined(CFG_MERAK3000)
	mac_phy_ptr->mpdu_retry_limit = 4;
	mac_phy_ptr->no_just_hard_retry_limit = 0;
	mac_phy_ptr->prot_retry_limit = 7;
#endif
}

void cali_set_default_per_user_mac_phy_params(struct cali_per_user_mac_phy_params_tag *user_mac_phy_ptr, enum cali_phy_band phy_band)
{
	user_mac_phy_ptr->fec = 1;		// ldpc
	user_mac_phy_ptr->packet_ext = 8;	// 8us
	user_mac_phy_ptr->min_mpdu_space = 0;	// 0us
	user_mac_phy_ptr->eof_padding = 0; // add 0bytes EoF padding
	user_mac_phy_ptr->dcm = 0;
	user_mac_phy_ptr->mcs = 0x07;
	user_mac_phy_ptr->mcs_legacy = (phy_band == CALI_PHY_BAND_2G4) ? 1 : 54;
	user_mac_phy_ptr->retry_rate_dec = 0;
	user_mac_phy_ptr->dropMCSEnTx = 1;

	// TODO. Init MU params later.
}

/* CSI related default parameters */
void cali_set_default_csi_param(struct cali_csi_info_tag *csi_info)
{
	int i;

	/* TODO: add base address for CSI buffer */
	csi_info->base_addr = 0;
	csi_info->bank_num = 8;
	csi_info->csi_flag = 0;
	/* Set default bank size to 32KBytes */
	csi_info->bank_size = 32 * 1024;

	/* PHY configuration */
	csi_info->smp_mode_sel = 1;
	/* enable smoothing for H Matrix */
	csi_info->smp_cont_sel0 = 1;
	csi_info->smp_cont_sel1 = 1;
	csi_info->csi_smpout_send_wait = 30;

	/* pass all PPDU to RAM */
	csi_info->rx_ppdu_symb_thresh = 0;

	/* sampling granularity */
	csi_info->l_ltf_gran = 0;
	for (i = 0; i <CALI_PHY_BW_MAX; i++) {
		csi_info->non_he_ltf_gran[i] = 0;
		csi_info->he_ltf_gran[i] = 0;
	}

}

void cali_set_default_pppc_info(struct cali_pppc_info_tag *pppc_info_ptr)
{
	int i;

	pppc_info_ptr->pppc_en = 0;
	for (i = 0; i < 10; i++) {
		pppc_info_ptr->template_power_ppducnt[i][0] = -10 + i*2;
		pppc_info_ptr->template_power_ppducnt[i][1] = 100;
	}
	pppc_info_ptr->template_num = 10;
}

void cali_set_default_config_internal(struct cali_config_tag *cali_cfg_ptr, enum cali_phy_band phy_band)
{
	int i;
	int ac;

	if (!cali_cfg_ptr)
		return;

	memset((void *)cali_cfg_ptr, 0, sizeof(*cali_cfg_ptr));

#if defined(CFG_MERAK3000)
	// Init Global-parameters.
	cali_cfg_ptr->mlo_enable = 0;	// Enable it after test on EMU later.

	cali_cfg_ptr->chipset_number = 1; // one M3K
	cali_cfg_ptr->chipset_id = 0;
	cali_cfg_ptr->working_ap_mld_id = 0;
	cali_cfg_ptr->transition_timeout = 0; // TODO

	cali_cfg_ptr->hw_agg_enable = 0;
	cali_cfg_ptr->hw_agg_dup = 0;

	cali_cfg_ptr->group_win_start = 0;
	cali_cfg_ptr->group_win_end = 0;

	// Init muti-AP-MLD
	for (i = 0; i < CALI_CFG_MAX_AP_MLD_NUM; i++) {
		struct cali_ap_mld_tag *ap_ptr = &cali_cfg_ptr->ap_mld_info[i];

		ap_ptr->is_mld = 0;
		ap_ptr->ap_mld_id = i;
		ap_ptr->ap_mld_tbl_idx = i;

		CALI_ADDR_COPY(ap_ptr->mld_addr, cali_def_self_mld_mac);
		ap_ptr->mld_addr[5] = i;

		ap_ptr->valid_links = (cali_cfg_ptr->chipset_number > 1) ?
			0x7 : 0x3;
	}

	// Init STA-MLD
	for (i = 0; i < CALI_CFG_MAX_STA_MLD_NUM; i++) {
		struct cali_sta_mld_tag *sta_ptr = &cali_cfg_ptr->sta_mld_info[i];

		sta_ptr->is_mld = 0;

		CALI_ADDR_COPY(sta_ptr->mld_addr, cali_def_peer_mld_mac);
		sta_ptr->mld_addr[5] = i;
		sta_ptr->valid_links = cali_cfg_ptr->ap_mld_info[0].valid_links;
		sta_ptr->sta_mld_tbl_idx = i;
		sta_ptr->sta_mlo_mode = CALI_MLO_MODE_STR;

		sta_ptr->eml_padding_delay = 0;
		sta_ptr->eml_transition_delay = 0;
		sta_ptr->eml_init_frame_type = 0;
	}
#endif // CFG_MERAK3000

	cali_cfg_ptr->mu_users = 0;		// No MU
	cali_cfg_ptr->tx_ppdu_cnt = 1;
	cali_cfg_ptr->tx_interval = 100;	// Delay 100us after sent one PPDU.

	for (ac = 0; ac < CALI_AC_NUM; ac++) {
		// Adjust it later.
		cali_cfg_ptr->wmm[ac].aifsn = 3;
		cali_cfg_ptr->wmm[ac].cw_min = 4;
		cali_cfg_ptr->wmm[ac].cw_max = 6;
	}

	// Init BSS info.
	cali_cfg_ptr->bss_info.radio = (phy_band == CALI_PHY_BAND_2G4) ? CALI_PHY_BAND_2G4 : CALI_PHY_BAND_5G;
	cali_cfg_ptr->bss_info.mac_mode = 0;	// AP.
	cali_cfg_ptr->bss_info.vap_num = 1;
	cali_cfg_ptr->bss_info.phy_mode = (phy_band == CALI_PHY_BAND_2G4) ? CALI_PHY_MODE_11B : CALI_PHY_MODE_11A;
	cali_cfg_ptr->bss_info.bw = (phy_band == CALI_PHY_BAND_2G4) ? 40 : 160;		// 160M on 5G and 40M on 2.4G
	cali_cfg_ptr->bss_info.chan_ieee = (phy_band == CALI_PHY_BAND_2G4) ? 1 : 36;	// Primary-Channel = 36(5G), or 1(2.4G)

	CALI_ADDR_COPY(cali_cfg_ptr->bss_info.local_addr, cali_def_self_mac);

#if defined(CFG_MERAK3000)
	cali_cfg_ptr->bss_info.ap_mld_id = 0;
	cali_cfg_ptr->bss_info.link_id = cali_phy_band_to_link_id(cali_cfg_ptr->chipset_id,
								phy_band);
#endif

	cali_set_default_mac_phy_params(&cali_cfg_ptr->mac_phy_params, phy_band);

	cali_set_default_user_info(&cali_cfg_ptr->su_user_info, cali_def_peer_mac, phy_band, 0,
		cali_cfg_ptr->chipset_id);
	cali_set_default_per_user_mac_phy_params(&cali_cfg_ptr->su_mac_phy_params, phy_band);

	cali_cfg_ptr->bss_info.key_gtk.cipher = 0;
	CALI_ADDR_COPY(cali_cfg_ptr->bss_info.key_gtk.key_addr, cali_broadcast_mac);
	CALI_ADDR_COPY(cali_cfg_ptr->key_ptk[0].key_addr, cali_def_peer_mac);

	/* init MU */
	cali_cfg_ptr->mu_info.mu_type = CALI_MU_DL_MIMO;
	cali_cfg_ptr->mu_info.mu_ack = CALI_MU_ACK_POLICY_INVALID;
	cali_cfg_ptr->mu_info.mu_prot = 0;
	cali_cfg_ptr->mu_info.tb_hw = 1;
	cali_cfg_ptr->mu_info.nsymb_choose = 1;
	cali_cfg_ptr->mu_info.pre_fec_pad_factor = 4;
	cali_cfg_ptr->mu_info.ldpc_extra_sym = 1;
	cali_cfg_ptr->mu_info.pe_disambiguity = 1;
	cali_cfg_ptr->mu_info.ul_mu_start_num = 0;

	for (i = 0; i < CALI_MU_USER_NUM; i++) {
		cali_cfg_ptr->key_ptk[i].cipher = 0;	// Cipher is none.

		CALI_ADDR_COPY(cali_cfg_ptr->key_ptk[i].key_addr, cali_def_mu_peer_mac[i]);
	}

	for (i = 0; i < CALI_MU_USER_NUM; i++) {
		cali_cfg_ptr->mu_user_info[i].sta_index = i;
		cali_set_default_user_info(&cali_cfg_ptr->mu_user_info[i],
				&cali_def_mu_peer_mac[i][0], phy_band, i,
				cali_cfg_ptr->chipset_id);
		cali_set_default_per_user_mac_phy_params(&cali_cfg_ptr->mu_mac_phy_params[i],
				phy_band);

		cali_cfg_ptr->mu_user_info[i + CALI_UL_MU_START_NUM].sta_index = i;
		cali_set_default_user_info(&cali_cfg_ptr->mu_user_info[i + CALI_UL_MU_START_NUM],
				&cali_def_mu_peer_mac[i][0], phy_band, i,
				cali_cfg_ptr->chipset_id);
		cali_set_default_per_user_mac_phy_params(
				&cali_cfg_ptr->mu_mac_phy_params[i + CALI_UL_MU_START_NUM],
				phy_band);
	}

	cali_set_default_csi_param(&cali_cfg_ptr->csi_info);
	cali_set_default_pppc_info(&cali_cfg_ptr->pppc_info);
#if defined(CFG_MERAK3000)
	cali_set_default_tdma_info(&cali_cfg_ptr->tdma_info);
#endif
}

#else // NON-CALI_DEF_CONFIG_IMPL
uint32_t cali_get_ap_mld_tbl_idx_by_mld_id(const struct cali_config_tag *cali_cfg,
					uint32_t ap_mld_id);
#endif // CALI_DEF_CONFIG_IMPL


#endif // _CALI_STRUCT_H_
