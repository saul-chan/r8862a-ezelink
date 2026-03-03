/**
 ****************************************************************************************
 *
 * @brief File containing the definitions related to calibration rx task.
 *
 * File containing the definitions related to calibration rx task.
 *
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 ****************************************************************************************
 */

#ifndef _CALI_RXSTATS_H_
#define _CALI_RXSTATS_H_

struct cal_rx_vector_non_ht {
	/* Length of the PPDU */
	uint16_t length;
	/* 1/2/5.5/11/6/9/12/18/24/36/48/54 Mbps */
	uint8_t rate;
	/*
	 * Dynamic Bandwidth
	 * 0 - Static bandwidth operation
	 * 1 - Dynamic bandwidth operation
	 */
	uint8_t dyn_bw;
	/* Channel Bandwidth in Non HT used in case of Bandwidth Signaling */
	uint8_t chn_bw;
};

struct cal_rx_vector_ht {
	/* 0: long, 1: short */
	uint8_t gi_type;
	/* Space Time Block Coding */
	uint8_t stbc;
	/* Modulation Coding Scheme */
	uint8_t mcs;
	/* 0: BCC, 1: LDPC */
	uint8_t fec;
	/* Length of the HT PPDU */
	uint16_t length;
};

struct cal_rx_vector_vht {
	/* 0: long, 1: short */
	uint8_t gi_type;
	/* Space Time Block Coding */
	uint8_t stbc;
	/* Group ID */
	uint8_t group_id;
	/* Modulation Coding Scheme */
	uint8_t mcs;
	/* Number of Spatial Streams */
	uint8_t nss;
	/* 0: BCC, 1: LDPC */
	uint8_t fec;
	/* Length of the VHT PPDU */
	uint32_t length;
};

struct cal_rx_vector_he {
	/* 0: 0.8us, 1: 1.6us, 2: 3.2us */
	uint8_t gi_type;
	/* Space Time Block Coding */
	uint8_t stbc;
	/* Type of HE-LTF */
	uint8_t he_ltf_type;
	/* Modulation Coding Scheme */
	uint8_t mcs;
	/* Dual Carrier Modulation */
	uint8_t dcm;
	/* Number of Spatial Streams */
	uint8_t nss;
	/* 0: BCC, 1: LDPC */
	uint8_t fec;
	/* Length of the HE PPDU */
	uint32_t length;
};

struct cal_rx_vector_eht {
	/* 0: 0.8us, 1: 1.6us, 2: 3.2us */
	uint8_t gi_type;
	/* Space Time Block Coding */
	uint8_t stbc;
	/* Type of HE-LTF */
	uint8_t he_ltf_type;
	/* Modulation Coding Scheme */
	uint8_t mcs;
	/* Number of Spatial Streams */
	uint8_t nss;
	/* 0: BCC, 1: LDPC */
	uint8_t fec;
	/* Length of the EHT PPDU */
	uint32_t length;
	uint16_t sta_idx;
	uint16_t usersymlen;
};

struct cal_rx_vector {
	/* common part */
	/* Format Modulation */
	uint8_t format_mod;
	/* Channel Bandwidth */
	uint8_t ch_bandwidth;
	/* format specific part */
	union {
		struct cal_rx_vector_non_ht non_ht;
		struct cal_rx_vector_ht ht;
		struct cal_rx_vector_vht vht;
		struct cal_rx_vector_he he;
		struct cal_rx_vector_eht eht;
	};
};

/* Per STA MAC layer statistics */
struct cal_mac_sta_stats {
	uint8_t mac_addr[6];
	/* Number of A-MPDU received */
	uint32_t rx_ampdu;
	/* Number of MPDU received */
	uint32_t rx_mpdu;
	/* Number of MSDU received */
	uint32_t rx_msdu;
	/* Number of MPDU received with FCS error */
	uint32_t rx_fcs_err;
	/*
	 * Status of FCS check of MPDU received.
	 * Each bit corresponds to an MPDU, and bit 0 corresponds to the latest MPDU.
	 * For each bit, a value of 1 means FCS check failed, while 0 means FCS check passed.
	 */
	uint32_t rx_fcs_status;

#if defined(CFG_MERAK3000)
	uint32_t is_mld;	// dummy on Host side
	uint32_t rx_mld_ppdu;
	uint32_t rx_mld_mpdu;
#endif
};

/* Per radio MAC layer statistics */
struct cal_mac_radio_stats {
	/* Number of A-MPDU received */
	uint32_t rx_ampdu;
	/* Number of MPDU received */
	uint32_t rx_mpdu;
	/* Number of MSDU received */
	uint32_t rx_msdu;

	/* Number of 802.11 Management frames received */
	uint32_t rx_type_mgmt[16];
	/* Number of 802.11 Control frames received */
	uint32_t rx_type_ctrl[16];
	/* Number of 802.11 Data frames received */
	uint32_t rx_type_data[16];

	/* Number of 802.11 Management frames received with FCS error */
	uint32_t rx_fcs_err_mgmt;
	/* Number of 802.11 Control frames received with FCS error */
	uint32_t rx_fcs_err_ctrl;
	/* Number of 802.11 Data frames received with FCS error */
	uint32_t rx_fcs_err_data;

	/* Number of MPDU received with decryption passed */
	uint32_t rx_decr_pass;
	/* Number of MPDU received with decryption failed */
	uint32_t rx_decr_fail;

	/* Number of MPDU received with other error than FCS or Decryption error */
	uint32_t rx_undef_err;
	/* Number of MFP received */
	uint32_t rx_mfp_cnt;

#if defined(CFG_MERAK3000)
	uint32_t rx_mld_ppdu;
	uint32_t rx_mld_mpdu;
#endif
};

/* Per radio PHY layer statistics */
struct cal_phy_radio_stats {
	/* Non-HT format */
	uint32_t rx_fmt_non_ht;
	/* Non-HT duplicate OFDM format */
	uint32_t rx_fmt_non_ht_dup_ofdm;
	/* HT mixed mode format */
	uint32_t rx_fmt_ht_mf;
	/* VHT format */
	uint32_t rx_fmt_vht;
	/* HE SU format */
	uint32_t rx_fmt_he_su;
	/* HE MU format */
	uint32_t rx_fmt_he_mu;
	/* HE Extended Range SU format */
	uint32_t rx_fmt_he_er_su;
	/* HE Trigger Based format */
	uint32_t rx_fmt_he_tb;
	/* EHT MU(SU) formart */
	uint32_t rx_fmt_eht_mu_su;
	/* EHT TB formart */
	uint32_t rx_fmt_eht_tb;
	/* EHT ER preamble */
	uint32_t rx_fmt_eht_er;

	/* Number of PSDU received with CRC check passed */
	uint32_t rx_crc_pass;
	/* Number of PSDU received with CRC check failed */
	uint32_t rx_crc_fail;
	/* Number of CSI received */
	uint32_t rx_csi_cnt;

	int sinr_total[2][2];
	uint32_t sinr_cnt;
	uint16_t sinr[16];
};

/* Per radio MAC layer status */
struct cal_mac_radio_status {
	/*
	 * Status of FCS check of MPDU received.
	 * Each bit corresponds to an MPDU, and bit 0 corresponds to the latest MPDU.
	 * For each bit, a value of 1 means FCS check failed, while 0 means FCS check passed.
	 */
	uint32_t rx_fcs_status;
	/* Type of latest 802.11 frame received */
	uint32_t rx_type_curr;
	/*
	 * Status of decryption of MPDU received.
	 * Each bit corresponds to an MPDU, and bit 0 corresponds to the latest MPDU.
	 * For each bit, a value of 1 means decryption failed, while 0 means decryption passed.
	 */
	uint32_t rx_decr_status;
	/* Packet Error Rate */
	uint32_t rx_per;
	/* Rx Vector including PPDU format/BW/NSS/MCS/Group ID/GI/Coding etc. */
	struct cal_rx_vector rx_vector;
	/* Index of MPDU subframe in A-MPDU */
	uint16_t rx_mpdu_idx;
	/* Size of latest MPDU received */
	uint32_t rx_mpdu_len;
	/* Time of latest MPDU received */
	uint32_t rx_mpdu_ts;
	/* Data of latest MPDU received, must be last field */
	uint8_t rx_mpdu[0];
};

/* Per radio PHY layer status */
struct cal_phy_radio_status {
	/* Format of latest PPDU received */
	uint32_t rx_fmt_curr;
	/*
	 * Status of CRC check of PSDU received.
	 * Each bit corresponds to an PSDU, and bit 0 corresponds to the latest PSDU.
	 * For each bit, a value of 1 means CRC check failed, while 0 means CRC check passed.
	 */
	uint32_t rx_crc_status;
};

struct cal_per_radio_stats {
	struct cal_mac_sta_stats   sta_stats[CALI_MU_USER_NUM];
	struct cal_mac_radio_stats mac_stats;
	struct cal_phy_radio_stats phy_stats;
};

struct cal_per_radio_status {
	struct cal_phy_radio_status phy_status;
	struct cal_mac_radio_status mac_status;
};

#endif /* _CALI_RXSTATS_H_ */
