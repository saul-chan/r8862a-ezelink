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

#ifndef _CLS_WIFI_BFMER_H_
#define _CLS_WIFI_BFMER_H_

/**
 * INCLUDE FILES
 ******************************************************************************
 */
#ifdef __KERNEL__
#include "cls_wifi_defs.h"
#endif

/**
 * DEFINES
 ******************************************************************************
 */

/// Maximal supported report length (in bytes)
#if defined(CFG_MERAK3000)
#define CLS_WIFI_BFMER_REPORT_MAX_LEN	3016
#else
#define CLS_WIFI_BFMER_REPORT_MAX_LEN	2048
#endif

/// Size of the allocated report space (twice the maximum report length)
#define CLS_WIFI_BFMER_REPORT_SPACE_SIZE  (CLS_WIFI_BFMER_REPORT_MAX_LEN * 2)

enum {
	BF_CMD_ENABLE,
	BF_CMD_SND_MODE,
	BF_CMD_SND_MAX_GRP_USER,
	BF_CMD_SND_MAX_STA_NUM,
	BF_CMD_SND_PERIOD,
	BF_CMD_SND_FEEDBACK_TYPE,
	BF_CMD_CBF_LIFETIME,
	BF_CMD_SND_LOG_LEVEL,
	BF_CMD_BF_LOG_LEVEL,
	BF_CMD_DUMP_CBF, // 10
	BF_CMD_NDP_POWER,
	BF_CMD_NDP_GI,
	BF_CMD_NDP_BW,
	BF_CMD_NDP_TIME_CSD,
	BF_CMD_NDP_SMM_IDX,
	BF_CMD_SUPPORT_2SS,
	BF_CMD_ENABLE_SMOOTH,
	BF_CMD_ENABLE_FILTER,
	BF_CMD_ALPHA,
	BF_CMD_SNAPSHOT,
};

struct cls_bf_parameters {
	/* To sync up with firmware */
	struct bf_parameters_req bf_req;

};

/**
 * TYPE DEFINITIONS
 ******************************************************************************
 */

/*
 * Structure used to store a beamforming report.
 */
struct cls_wifi_bfmer_report {
	dma_addr_t dma_addr;	/* Virtual address provided to MAC for DMA transfer of the Beamforming Report */
	unsigned int length;	/* Report Length */
	u8 report[1];		   /* Report to be used for VHT TX Beamforming */
};

#ifdef __KERNEL__
/**
 * FUNCTION DECLARATIONS
 ******************************************************************************
 */

/**
 ******************************************************************************
 * @brief Allocate memory aiming to contains the Beamforming Report received
 * from a Beamformee capable.
 * The providing length shall be large enough to contain the VHT Compressed
 * Beaforming Report and the MU Exclusive part.
 * It also perform a DMA Mapping providing an address to be provided to the HW
 * responsible for the DMA transfer of the report.
 * If successful a struct cls_wifi_bfmer_report object is allocated, it's address
 * is stored in cls_wifi_sta->bfm_report.
 *
 * @param[in] cls_wifi_hw   PHY Information
 * @param[in] cls_wifi_sta  Peer STA Information
 * @param[in] length	Memory size to be allocated
 *
 * @return 0 if operation is successful, else -1.
 ******************************************************************************
 */
int cls_wifi_bfmer_report_add(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
						  unsigned int length);

/**
 ******************************************************************************
 * @brief Free a previously allocated memory intended to be used for
 * Beamforming Reports.
 *
 * @param[in] cls_wifi_hw   PHY Information
 * @param[in] cls_wifi_sta  Peer STA Information
 *
 ******************************************************************************
 */
void cls_wifi_bfmer_report_del(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta);

/**
 ******************************************************************************
 * @brief Parse a Rx VHT-MCS map or HE-MCS and NSS set in order to deduce the maximum
 * number of Spatial Streams supported by a beamformee.
 *
 * @param[in] rx_mcs_map  Received supported VHT-MCS Map or HE-MCS and NSS Set Capability
 *						field.
 *
 ******************************************************************************
 */
u8 cls_wifi_bfmer_get_rx_nss(u16 rx_mcs_map);
void cls_wifi_bf_init(struct cls_wifi_hw *wifi_hw);
void cls_wifi_bfmer_cmd_handler(struct cls_wifi_hw *wifi_hw, int cmd, int value);
#endif /* __KERNEL__ */

#endif /* _CLS_WIFI_BFMER_H_ */
