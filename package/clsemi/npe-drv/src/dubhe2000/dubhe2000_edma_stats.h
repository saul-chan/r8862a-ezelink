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

#ifndef _DUBHE1000_EDMA_STATS_H_
#define _DUBHE1000_EDMA_STATS_H_

#include "dubhe2000.h"

#define DUBHE1000_Q_DIV32			16

#define EDMA_STATS_OPTION_ALL			0
#define EDMA_STATS_OPTION_MIN			EDMA_STATS_OPTION_MAINT_TEST
#define EDMA_STATS_OPTION_MAINT_TEST		1
#define EDMA_STATS_OPTION_ALARM_REPORT		2
#define EDMA_STATS_OPTION_ABNORMAL_REPORT	3
#define EDMA_STATS_OPTION_ABNORMAL_STATS	4
#define EDMA_STATS_OPTION_NORMAL_STATS		5
#define EDMA_STATS_OPTION_MAX			EDMA_STATS_OPTION_NORMAL_STATS

//edma debug macro
/* 1. Maintenance and Testing Report */

/* To CPU BMU Report REG(read only), arrary k=256 */
#define DUBHE1000_TX_BMU_ALLOCATE0		0x0000  // RO
#define DUBHE1000_TX_BMU_ALLOCATE1		0x0004  // RO
#define DUBHE1000_TX_BMU_ALLOCATE2		0x0008  // RO
#define DUBHE1000_TX_BMU_ALLOCATE3		0x000C  // RO
#define DUBHE1000_TX_BMU_ALLOCATE4		0x0010  // RO
#define DUBHE1000_TX_BMU_ALLOCATE5		0x0014  // RO
#define DUBHE1000_TX_BMU_ALLOCATE6		0x0018  // RO
#define DUBHE1000_TX_BMU_ALLOCATE7		0x001C  // RO

/* To CPU BMU Page Report REG(read only) arrary n8=8 */
#define DUBHE1000_TX_BMU_PAGE_BITMAP		0x4000  // RO

/* To CPU Routing Table Report REG(read only) arrary q=512 same to the token */
#define DUBHE1000_TX_ROUTE_CAM0			0x4040  // RO
#define DUBHE1000_TX_ROUTE_CAM1			0x4044  // RO

/* To CPU Routing Table Enable Report REG(read only) arrary n16=16 */
#define DUBHE1000_TX_ROUTE_BITMAP		0x5040  // RO

#define DUBHE1000_FSM_RPT			0x00005080
#define DUBHE1000_TX_FIFO_RPT			0x00005084

#define DUBHE1000_RX_FIFO_RPT			0x00005088
#define DUBHE1000_BUS_READY_RPT			0x0000508C
#define DUBHE1000_DOWNSTREAM_RPT0		0x00005090

#define DUBHE1000_DOWNSTREAM_RPT1		0x00005094
#define DUBHE1000_UPSTREAM_RPT			0x00005098

/* 2. Alarm Report */
#define DUBHE1000_CFG_ALM			0x0000509C  // WC

/* Error Report REG (WC: write clear) array q_div32 = 16(token/32) */
#define DUBHE1000_TX_TOKEN_ERROR_ALM		0x000050A0

/* 3. Abnormal Report */
#define DUBHE1000_ABNOR_ALM			0x000050E0

/* 4. Abnormal Statistics and Reporting */
#define DUBHE1000_TX_ERROR_CNT			0x000050E4
#define DUBHE1000_TX_BMU_ERROR_CNT		0x000050E8
#define DUBHE1000_TX_BLK_ERROR_CNT		0x000050EC
#define DUBHE1000_TX_PKT_LEN_ERROR_CNT		0x000050F0
#define DUBHE1000_TX_TOKEN_FETCH_ERROR_CNT	0x000050F4
#define DUBHE1000_TX_EOP_ERROR_CNT		0x000050F8
#define DUBHE1000_TX_FREE_ERROR_CNT		0x000050FC
#define DUBHE1000_RX_ERROR_CNT			0x00005100
#define DUBHE1000_RX_PKT_LEN_ERROR_CNT		0x00005104

/* 5. Normal Statistics and Reporting */
#define DUBHE1000_TX_PKT_CNT			0x00005108
#define DUBHE1000_TX_PKT_BYTE_CNT_LO		0x0000510C
#define DUBHE1000_TX_PKT_BYTE_CNT_HI		0x00005110
#define DUBHE1000_TX_MIRROR_PKT_CNT		0x00005114
#define DUBHE1000_TX_PKT_NORMAL_CNT		0x00005118
#define DUBHE1000_RX_PKT_CNT			0x0000511C
#define DUBHE1000_RX_PKT_BYTE_CNT_LO		0x00005120
#define DUBHE1000_RX_PKT_BYTE_CNT_HI		0x00005124
#define DUBHE1000_RX_PKT_NORMAL_CNT		0x00005128
#define DUBHE1000_DDR_EMPTY			0x5154 // DDR empty indication
#define DUBHE1000_DDR_FULL			0x5158 // DDR full indication
#define DUBHE1000_DDR_STATUS_0_OCCUPANCY	0x516C // DDR status 0 remaining data
#define DUBHE1000_DDR_STATUS_1_OCCUPANCY	0x5170 // DDR status 1 remaining data
#define DUBHE1000_DDR_STATUS_2_OCCUPANCY	0x5174 // DDR status 2 remaining data
#define DUBHE1000_DDR_STATUS_3_OCCUPANCY	0x5178 // DDR status 3 remaining data
#define DUBHE1000_HOST_PKT_CNT			0x517C // Count of packets sent to HOST
#define DUBHE1000_PCIE_PKT_CNT			0x5188 // Count of packets sent to PCIE
#define DUBHE1000_SHORT_PKT_DROP_CNT		0x5140 // Count of dropped packets less than 60B
#define DUBHE1000_SHORT_PKT_LEN_DROP_CNT	0x5144 // Count of dropped packets that pkt_len in cputag less than 60B
#define DUBHE1000_STATUS_0_FULL_DROP_CNT	0x5130 // Count of dropped packets due to status 0 full
#define DUBHE1000_STATUS_0_IN_OCCUPANCY		0x515C // Status 0 IN FIFO remaining data
#define DUBHE1000_STATUS_1_FULL_DROP_CNT	0x5134 // Count of dropped packets due to status 1 full
#define DUBHE1000_STATUS_1_IN_OCCUPANCY		0x5160 // Status 1 IN FIFO remaining data
#define DUBHE1000_STATUS_2_FULL_DROP_CNT	0x5138 // Count of dropped packets due to status 2 full
#define DUBHE1000_STATUS_2_IN_OCCUPANCY		0x5164 // Status 2 IN FIFO remaining data
#define DUBHE1000_STATUS_3_FULL_DROP_CNT	0x513C // Count of dropped packets due to status 3 full
#define DUBHE1000_STATUS_3_IN_OCCUPANCY		0x5168 // Status 3 IN FIFO remaining data
#define DUBHE1000_WIFI160_PKT_CNT		0x5184 // Count of packets sent to WIFI 160
#define DUBHE1000_WIFI40_PKT_CNT		0x5180 // Count of packets sent to WIFI 40
//edma dbg struct

/* Maintenance and Testing Report */
struct dubhe1000_fsm_rpt {
	u32 tx_dma_cstate:3;
	u32 Reserved1:1;
	u32 rx_dma_cstate:2;
	u32 Reserved2:26;
};

struct dubhe1000_tx_fifo_rpt {
	u32 tx_data_fifo_full:1;
	u32 tx_data_fifo_empty:1;
	u32 tx_tag_fifo_full:1;
	u32 tx_tag_fifo_empty:1;
	u32 bmu_para_fifo_full:1;
	u32 bmu_para_fifo_empty:1;
	u32 tx_req_fifo_full:1;
	u32 tx_req_fifo_empty:1;
	u32 head_ack_fifo_full:1;
	u32 head_ack_fifo_empty:1;
	u32 body_ack_fifo_full:1;
	u32 body_ack_fifo_empty:1;
	u32 buf_free_fifo_full:1;
	u32 buf_free_fifo_empty:1;
	u32 tx_descr_fifo_full:4;
	u32 tx_descr_fifo_empty:4;
	u32 tx_status_fifo_full:4;
	u32 tx_status_fifo_empty:4;
	u32 tx_halt:1;
	u32 Reserved:1;
};

struct dubhe1000_rx_fifo_rpt {
	u32 rx_data_fifo_full:1;
	u32 rx_data_fifo_empty:1;
	u32 rx_descr_fifo_full:4;
	u32 rx_descr_fifo_empty:4;
	u32 rx_status_fifo_full:4;
	u32 rx_status_fifo_empty:4;
	u32 rx_halt:1;
	u32 Reserved:13;
};

struct dubhe1000_bus_ready_rpt {
	u32 awready_rpt:1;
	u32 wready_rpt:1;
	u32 bready_rpt:1;
	u32 arready_rpt:1;
	u32 rready_rpt:1;
	u32 Reserved:27;
};

struct dubhe1000_downstream_rpt0 {
	u32 dwstrm_r_state:4;
	u32 dwstrm_ar_state:2;
	u32 dwstrm_context_tag:4;
	u32 dwstrm_channel_req:1;
	u32 dwstrm_channel_sel:1;
	u32 dwstrm_channel_tag:4;
	u32 dwstrm_context_remain:16;
};

struct dubhe1000_downstream_rpt1 {
	u32 dwstrm_rready:1;
	u32 dwstrm_rvalid:1;
	u32 dwstrm_rlast:1;
	u32 dwstrm_rresp:2;
	u32 dwstrm_rid:4;
	u32 dwstrm_arready:1;
	u32 dwstrm_arvalid:1;
	u32 dwstrm_arid:4;
	u32 dwstrm_error:1;
	u32 dwstrm_state:3;
	u32 dwstrm_count:9;
	u32 dwstrm_bus_ready:1;
	u32 dwstrm_data_en:1;
	u32 dwstrm_start:1;
	u32 dwstrm_stall:1;
};

struct dubhe1000_upstream_rpt {
	u32 upstrm_aw_state:3;
	u32 upstrm_b_state:2;
	u32 upstrm_pipe_wr_ready:1;
	u32 upstrm_send_req_rdy:1;
	u32 upstrm_datai_rdy:1;
	u32 upstrm_ot_fifo_empty:1;
	u32 upstrm_ot_fifo_full:1;
	u32 upstrm_req_push_rdy:2;
	u32 upstrm_ack_pop_vld:2;
	u32 upstrm_last_ack:2;
	u32 upstrm_state:3;
	u32 upstrm_count:9;
	u32 upstrm_bus_ready:1;
	u32 upstrm_bus_trans:1;
	u32 upstrm_start:1;
	u32 upstrm_pause:1;
};

struct dubhe1000_edma_maint_test {
	struct dubhe1000_fsm_rpt fsm_rpt;
	struct dubhe1000_tx_fifo_rpt tx_fifo_rpt;
	struct dubhe1000_rx_fifo_rpt rx_fifo_rpt;
	struct dubhe1000_bus_ready_rpt bus_ready_rpt;
	struct dubhe1000_downstream_rpt0 downstream_rpt0;
	struct dubhe1000_downstream_rpt1 downstream_rpt1;
	struct dubhe1000_upstream_rpt upstream_rpt;
};

/* 2. Alarm Report */
struct dubhe1000_edma_cfg_alm {
	u32 head_block_size_alm:1;
	u32 body_block_size_alm:1;
	u32 tx_descr_fifo_depth_alm:1;
	u32 rx_descr_fifo_depth_alm:1;
	u32 tx_status_fifo_depth_alm:1;
	u32 rx_status_fifo_depth_alm:1;
	u32 tx_bus_err_alm:1;
	u32 rx_bus_err_alm:1;
	u32 tx_token_cfg_err_alm:1;
	u32 tx_buf_clr_err_alm:1;
	u32 tx_descr_wr_err_alm:4;
	u32 rx_descr_wr_err_alm:4;
	u32 tx_status_clr_err_alm:4;
	u32 rx_status_clr_err_alm:4;
	u32 tx_status_0_ddr_alm:1;
	u32 tx_status_1_ddr_alm:1;
	u32 tx_status_2_ddr_alm:1;
	u32 tx_status_3_ddr_alm:1;
	u32 tx_hw_bmu_err_alm:1;
	u32 Reserved:6;
};

struct dubhe1000_edma_tx_token_error_alm {
	u32 token_underflow_alm[DUBHE1000_Q_DIV32];
};

struct dubhe1000_edma_alarm_report {
	struct dubhe1000_edma_cfg_alm cfg_alm;
	struct dubhe1000_edma_tx_token_error_alm tx_token_error_alm;
};

// Abnormal Report
struct dubhe1000_edma_abnor_alm {
	u32 tx_timeout_abn:1;
	u32 rx_timeout_abn:1;
	u32 Reserved:30;
};

struct dubhe1000_edma_abnormal_report {
	struct dubhe1000_edma_abnor_alm abnor_alm;
};

// Abnormal Statistics and Report
struct dubhe1000_edma_tx_error_cnt {
	u32 tx_err_pkt_cnt;
};

struct dubhe1000_edma_tx_bmu_error_cnt {
	u32 alloc_err_cnt;
};

struct dubhe1000_edma_tx_blk_error_cnt {
	u32 blk_ovf_cnt:8;
	u32 Reserved:24;
};

struct dubhe1000_edma_tx_pkt_len_error_cnt {
	u32 tx_length_err_cnt:8;
	u32 Reserved:24;
};

struct dubhe1000_edma_tx_token_fetch_error_cnt {
	u32 token_fetch_err_cnt:8;
	u32 Reserved:24;
};

struct dubhe1000_edma_tx_eop_error_cnt {
	u32 tx_early_eop_err_cnt:8;
	u32 tx_late_eop_err_cnt:8;
	u32 Reserved:16;
};

struct dubhe1000_edma_tx_free_error_cnt {
	u32 tx_free_err_cnt:8;
	u32 Reserved:24;
};

struct dubhe1000_edma_rx_error_cnt {
	u32 rx_err_pkt_cnt:16;
	u32 Reserved:16;
};

struct dubhe1000_edma_rx_pkt_len_error_cnt {
	u32 rx_pkt_len_err_cnt:8;
	u32 Reserved:24;
};

struct dubhe1000_edma_abnormal_stats {
	struct dubhe1000_edma_tx_error_cnt tx_error_cnt;
	struct dubhe1000_edma_tx_bmu_error_cnt tx_bmu_error_cnt;
	struct dubhe1000_edma_tx_blk_error_cnt tx_blk_error_cnt;
	struct dubhe1000_edma_tx_pkt_len_error_cnt tx_pkt_len_error_cnt;
	struct dubhe1000_edma_tx_token_fetch_error_cnt tx_token_fetch_error_cnt;
	struct dubhe1000_edma_tx_eop_error_cnt tx_eop_error_cnt;
	struct dubhe1000_edma_tx_free_error_cnt tx_free_error_cnt;
	struct dubhe1000_edma_rx_error_cnt rx_error_cnt;
	struct dubhe1000_edma_rx_pkt_len_error_cnt rx_pkt_len_error_cnt;
};

// Normal Statistics and Report
struct dubhe1000_edma_tx_pkt_cnt {
	u32 tx_pkt_cnt;
};

struct dubhe1000_edma_tx_pkt_byte_cnt_lo {
	u32 tx_pkt_byte_cnt_l;
};

struct dubhe1000_edma_pkt_byte_cnt_hi {
	u32 pkt_byte_cnt_h:31;
	u32 pkt_byte_cnt_clr:1;
};

struct dubhe1000_edma_tx_mirror_pkt_cnt {
	u32 tx_mirror_pkt_cnt:12;
	u32 Reserved:20;
};

struct dubhe1000_edma_tx_pkt_normal_cnt {
	u32 tx_normal_pkt_cnt;
};

struct dubhe1000_edma_rx_pkt_cnt {
	u32 rx_pkt_cnt;
};

struct dubhe1000_edma_rx_pkt_byte_cnt_lo {
	u32 rx_pkt_byte_cnt_l;
};

struct dubhe1000_edma_rx_pkt_normal_cnt {
	u32 rx_normal_pkt_cnt;
};

struct dubhe1000_edma_tx_status_0_full_drop_cnt {
	u32 tx_status_0_full_drop_cnt;
};
struct dubhe1000_edma_tx_status_1_full_drop_cnt {
	u32 tx_status_1_full_drop_cnt;
};
struct dubhe1000_edma_tx_status_2_full_drop_cnt {
	u32 tx_status_2_full_drop_cnt;
};
struct dubhe1000_edma_tx_status_3_full_drop_cnt {
	u32 tx_status_3_full_drop_cnt;
};
struct dubhe1000_edma_tx_short_pkt_drop_cnt {
	u32 tx_short_pkt_drop_cnt;
};
struct dubhe1000_edma_tx_short_pkt_len_drop_cnt {
	u32 tx_short_pkt_len_drop_cnt;
};
struct dubhe1000_edma_tx_status_ddr_empty {
	u32 tx_status_0_ddr_empty:1;
	u32 tx_status_1_ddr_empty:1;
	u32 tx_status_2_ddr_empty:1;
	u32 tx_status_3_ddr_empty:1;
};
struct dubhe1000_edma_tx_status_ddr_full {
	u32 tx_status_0_ddr_full:1;
	u32 tx_status_1_ddr_full:1;
	u32 tx_status_2_ddr_full:1;
	u32 tx_status_3_ddr_full:1;
};
struct dubhe1000_edma_status_0_in_occupancy {
	u32 status_0_in_occupancy;
};
struct dubhe1000_edma_status_1_in_occupancy {
	u32 status_1_in_occupancy;
};
struct dubhe1000_edma_status_2_in_occupancy {
	u32 status_2_in_occupancy;
};
struct dubhe1000_edma_status_3_in_occupancy {
	u32 status_3_in_occupancy;
};
struct dubhe1000_edma_ddr_status_0_occupancy {
	u32 ddr_status_0_occupancy;
};
struct dubhe1000_edma_ddr_status_1_occupancy {
	u32 ddr_status_1_occupancy;
};
struct dubhe1000_edma_ddr_status_2_occupancy {
	u32 ddr_status_2_occupancy;
};
struct dubhe1000_edma_ddr_status_3_occupancy {
	u32 ddr_status_3_occupancy;
};
struct dubhe1000_edma_host_pkt_cnt {
	u32 host_pkt_cnt;
};
struct dubhe1000_edma_wifi40_pkt_cnt {
	u32 wifi40_pkt_cnt;
};
struct dubhe1000_edma_wifi160_pkt_cnt {
	u32 wifi160_pkt_cnt;
};
struct dubhe1000_edma_pcie_pkt_cnt {
	u32 pcie_pkt_cnt;
};
struct dubhe1000_edma_normal_stats {
	struct dubhe1000_edma_tx_pkt_cnt tx_pkt_cnt;
	struct dubhe1000_edma_tx_pkt_byte_cnt_lo tx_pkt_byte_cnt_lo;
	struct dubhe1000_edma_pkt_byte_cnt_hi tx_pkt_byte_cnt_hi;
	struct dubhe1000_edma_tx_mirror_pkt_cnt tx_mirror_pkt_cnt;
	struct dubhe1000_edma_tx_pkt_normal_cnt tx_pkt_normal_cnt;
	struct dubhe1000_edma_rx_pkt_cnt rx_pkt_cnt;
	struct dubhe1000_edma_rx_pkt_byte_cnt_lo rx_pkt_byte_cnt_lo;
	struct dubhe1000_edma_pkt_byte_cnt_hi rx_pkt_byte_cnt_hi;
	struct dubhe1000_edma_rx_pkt_normal_cnt rx_pkt_normal_cnt;
	struct dubhe1000_edma_tx_status_0_full_drop_cnt tx_status_0_full_drop_cnt;
	struct dubhe1000_edma_tx_status_1_full_drop_cnt tx_status_1_full_drop_cnt;
	struct dubhe1000_edma_tx_status_2_full_drop_cnt tx_status_2_full_drop_cnt;
	struct dubhe1000_edma_tx_status_3_full_drop_cnt tx_status_3_full_drop_cnt;
	struct dubhe1000_edma_tx_short_pkt_drop_cnt tx_short_pkt_drop_cnt;
	struct dubhe1000_edma_tx_short_pkt_len_drop_cnt tx_short_pkt_len_drop_cnt;
	struct dubhe1000_edma_tx_status_ddr_empty tx_status_ddr_empty;
	struct dubhe1000_edma_tx_status_ddr_full tx_status_ddr_full;
	struct dubhe1000_edma_status_0_in_occupancy status_0_in_occupancy;
	struct dubhe1000_edma_status_1_in_occupancy status_1_in_occupancy;
	struct dubhe1000_edma_status_2_in_occupancy status_2_in_occupancy;
	struct dubhe1000_edma_status_3_in_occupancy status_3_in_occupancy;
	struct dubhe1000_edma_ddr_status_0_occupancy ddr_status_0_occupancy;
	struct dubhe1000_edma_ddr_status_1_occupancy ddr_status_1_occupancy;
	struct dubhe1000_edma_ddr_status_2_occupancy ddr_status_2_occupancy;
	struct dubhe1000_edma_ddr_status_3_occupancy ddr_status_3_occupancy;
	struct dubhe1000_edma_host_pkt_cnt host_pkt_cnt;
	struct dubhe1000_edma_wifi40_pkt_cnt wifi40_pkt_cnt;
	struct dubhe1000_edma_wifi160_pkt_cnt wifi160_pkt_cnt;
	struct dubhe1000_edma_pcie_pkt_cnt pcie_pkt_cnt;
};

struct dubhe1000_edma_stats {
	// Maintenance and Testing Report
	struct dubhe1000_edma_maint_test maint_test;
	// Alarm Report
	struct dubhe1000_edma_alarm_report alarm_report;
	// Abnormal Report
	struct dubhe1000_edma_abnormal_report abnormal_report;
	// Abnormal Statistics and Report
	struct dubhe1000_edma_abnormal_stats abnormal_stats;
	// Normal Statistics and Report
	struct dubhe1000_edma_normal_stats normal_stats;
};

void dubhe1000_edma_stats_dump(struct dubhe1000_adapter *adapter, u8 option);
#endif /* _DUBHE1000_EDMA_STATS_H_ */
