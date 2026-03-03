/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _CLS_NPE_EDMA_STATS_H_
#define _CLS_NPE_EDMA_STATS_H_

#include "cls_npe.h"

#define CLS_NPE_Q_DIV32			16

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
#define CLS_NPE_TX_BMU_ALLOCATE0		(0x0000)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE1		(0x0004)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE2		(0x0008)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE3		(0x000C)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE4		(0x0010)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE5		(0x0014)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE6		(0x0018)  // RO
#define CLS_NPE_TX_BMU_ALLOCATE7		(0x001C)  // RO

/* To CPU BMU Page Report REG(read only) arrary n8=8 */
#define CLS_NPE_TX_BMU_PAGE_BITMAP		(0x4000)  // RO

/* To CPU Routing Table Report REG(read only) arrary q=512 same to the token */
#define CLS_NPE_TX_ROUTE_CAM0			(0x4040)  // RO
#define CLS_NPE_TX_ROUTE_CAM1			(0x4044)  // RO

/* To CPU Routing Table Enable Report REG(read only) arrary n16=16 */
#define CLS_NPE_TX_ROUTE_BITMAP		(0x5040)  // RO

#define CLS_NPE_FSM_RPT			0x00005080
#define CLS_NPE_TX_FIFO_RPT			0x00005084

#define CLS_NPE_RX_FIFO_RPT			0x00005088
#define CLS_NPE_BUS_READY_RPT			0x0000508C
#define CLS_NPE_DOWNSTREAM_RPT0		0x00005090

#define CLS_NPE_DOWNSTREAM_RPT1		0x00005094
#define CLS_NPE_UPSTREAM_RPT			0x00005098

/* 2. Alarm Report */
#define CLS_NPE_CFG_ALM			0x0000509C  // WC

/* Error Report REG (WC: write clear) array q_div32 = 16(token/32) */
#define CLS_NPE_TX_TOKEN_ERROR_ALM		0x000050A0

/* 3. Abnormal Report */
#define CLS_NPE_ABNOR_ALM			0x000050E0

/* 4. Abnormal Statistics and Reporting */
#define CLS_NPE_TX_ERROR_CNT			0x000050E4
#define CLS_NPE_TX_BMU_ERROR_CNT		0x000050E8
#define CLS_NPE_TX_BLK_ERROR_CNT		0x000050EC
#define CLS_NPE_TX_PKT_LEN_ERROR_CNT		0x000050F0
#define CLS_NPE_TX_TOKEN_FETCH_ERROR_CNT	0x000050F4
#define CLS_NPE_TX_EOP_ERROR_CNT		0x000050F8
#define CLS_NPE_TX_FREE_ERROR_CNT		0x000050FC
#define CLS_NPE_RX_ERROR_CNT			0x00005100
#define CLS_NPE_RX_PKT_LEN_ERROR_CNT		0x00005104

/* 5. Normal Statistics and Reporting */
#define CLS_NPE_TX_PKT_CNT			0x00005108
#define CLS_NPE_TX_PKT_BYTE_CNT_LO		0x0000510C
#define CLS_NPE_TX_PKT_BYTE_CNT_HI		0x00005110
#define CLS_NPE_TX_MIRROR_PKT_CNT		0x00005114
#define CLS_NPE_TX_PKT_NORMAL_CNT		0x00005118
#define CLS_NPE_RX_PKT_CNT			0x0000511C
#define CLS_NPE_RX_PKT_BYTE_CNT_LO		0x00005120
#define CLS_NPE_RX_PKT_BYTE_CNT_HI		0x00005124
#define CLS_NPE_RX_PKT_NORMAL_CNT		0x00005128

//edma dbg struct

/* Maintenance and Testing Report */
struct cls_fsm_rpt {
	u32 tx_dma_cstate:3;
	u32 Reserved1:1;
	u32 rx_dma_cstate:2;
	u32 Reserved2:26;
};

struct cls_tx_fifo_rpt {
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

struct cls_rx_fifo_rpt {
	u32 rx_data_fifo_full:1;
	u32 rx_data_fifo_empty:1;
	u32 rx_descr_fifo_full:4;
	u32 rx_descr_fifo_empty:4;
	u32 rx_status_fifo_full:4;
	u32 rx_status_fifo_empty:4;
	u32 rx_halt:1;
	u32 Reserved:13;
};

struct cls_bus_ready_rpt {
	u32 awready_rpt:1;
	u32 wready_rpt:1;
	u32 bready_rpt:1;
	u32 arready_rpt:1;
	u32 rready_rpt:1;
	u32 Reserved:27;
};

struct cls_downstream_rpt0 {
	u32 dwstrm_r_state:4;
	u32 dwstrm_ar_state:2;
	u32 dwstrm_context_tag:4;
	u32 dwstrm_channel_req:1;
	u32 dwstrm_channel_sel:1;
	u32 dwstrm_channel_tag:4;
	u32 dwstrm_context_remain:16;
};

struct cls_downstream_rpt1 {
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

struct cls_upstream_rpt {
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

struct cls_edma_maint_test {
	struct cls_fsm_rpt fsm_rpt;
	struct cls_tx_fifo_rpt tx_fifo_rpt;
	struct cls_rx_fifo_rpt rx_fifo_rpt;
	struct cls_bus_ready_rpt bus_ready_rpt;
	struct cls_downstream_rpt0 downstream_rpt0;
	struct cls_downstream_rpt1 downstream_rpt1;
	struct cls_upstream_rpt upstream_rpt;
};

/* 2. Alarm Report */
struct cls_edma_cfg_alm {
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
	u32 Reserved:6;
};

struct cls_edma_tx_token_error_alm {
	u32 token_underflow_alm[CLS_NPE_Q_DIV32];
};

struct cls_edma_alarm_report {
	struct cls_edma_cfg_alm  cfg_alm;
	struct cls_edma_tx_token_error_alm tx_token_error_alm;
};

// Abnormal Report
struct cls_edma_abnor_alm {
	u32 tx_timeout_abn:1;
	u32 rx_timeout_abn:1;
	u32 Reserved:30;
};

struct cls_edma_abnormal_report {
	struct cls_edma_abnor_alm abnor_alm;
};

// Abnormal Statistics and Report
struct cls_edma_tx_error_cnt {
	u32 tx_err_pkt_cnt:16;
	u32 Reserved:16;
};

struct cls_edma_tx_bmu_error_cnt {
	u32 alloc_err_cnt:8;
	u32 Reserved:24;
};

struct cls_edma_tx_blk_error_cnt {
	u32 blk_ovf_cnt:8;
	u32 Reserved:24;
};

struct cls_edma_tx_pkt_len_error_cnt {
	u32 tx_length_err_cnt:8;
	u32 Reserved:24;
};

struct cls_edma_tx_token_fetch_error_cnt {
	u32 token_fetch_err_cnt:8;
	u32 Reserved:24;
};

struct cls_edma_tx_eop_error_cnt {
	u32 tx_early_eop_err_cnt:8;
	u32 tx_late_eop_err_cnt:8;
	u32 Reserved:16;
};

struct cls_edma_tx_free_error_cnt {
	u32 tx_free_err_cnt:8;
	u32 Reserved:24;
};

struct cls_edma_rx_error_cnt {
	u32 rx_err_pkt_cnt:16;
	u32 Reserved:16;
};

struct cls_edma_rx_pkt_len_error_cnt {
	u32 rx_pkt_len_err_cnt:8;
	u32 Reserved:24;
};

struct cls_edma_abnormal_stats {
	struct cls_edma_tx_error_cnt tx_error_cnt;
	struct cls_edma_tx_bmu_error_cnt tx_bmu_error_cnt;
	struct cls_edma_tx_blk_error_cnt tx_blk_error_cnt;
	struct cls_edma_tx_pkt_len_error_cnt tx_pkt_len_error_cnt;
	struct cls_edma_tx_token_fetch_error_cnt tx_token_fetch_error_cnt;
	struct cls_edma_tx_eop_error_cnt tx_eop_error_cnt;
	struct cls_edma_tx_free_error_cnt tx_free_error_cnt;
	struct cls_edma_rx_error_cnt rx_error_cnt;
	struct cls_edma_rx_pkt_len_error_cnt rx_pkt_len_error_cnt;
};

// Normal Statistics and Report
struct cls_edma_tx_pkt_cnt {
	u32 tx_pkt_cnt;
};

struct cls_edma_tx_pkt_byte_cnt_lo {
	u32 tx_pkt_byte_cnt_l;
};

struct cls_edma_pkt_byte_cnt_hi {
	u32 pkt_byte_cnt_h:31;
	u32 pkt_byte_cnt_clr:1;
};

struct cls_edma_tx_mirror_pkt_cnt {
	u32 tx_mirror_pkt_cnt:12;
	u32 Reserved:20;
};

struct cls_edma_tx_pkt_normal_cnt {
	u32 tx_normal_pkt_cnt;
};

struct cls_edma_rx_pkt_cnt {
	u32 rx_pkt_cnt;
};

struct cls_edma_rx_pkt_byte_cnt_lo {
	u32 rx_pkt_byte_cnt_l;
};

struct cls_edma_rx_pkt_normal_cnt {
	u32 rx_normal_pkt_cnt;
};

struct cls_edma_normal_stats {
	struct cls_edma_tx_pkt_cnt tx_pkt_cnt;
	struct cls_edma_tx_pkt_byte_cnt_lo tx_pkt_byte_cnt_lo;
	struct cls_edma_pkt_byte_cnt_hi tx_pkt_byte_cnt_hi;
	struct cls_edma_tx_mirror_pkt_cnt tx_mirror_pkt_cnt;
	struct cls_edma_tx_pkt_normal_cnt tx_pkt_normal_cnt;
	struct cls_edma_rx_pkt_cnt rx_pkt_cnt;
	struct cls_edma_rx_pkt_byte_cnt_lo rx_pkt_byte_cnt_lo;
	struct cls_edma_pkt_byte_cnt_hi rx_pkt_byte_cnt_hi;
	struct cls_edma_rx_pkt_normal_cnt rx_pkt_normal_cnt;
};

struct cls_edma_stats {
	// Maintenance and Testing Report
	struct cls_edma_maint_test maint_test;
	// Alarm Report
	struct cls_edma_alarm_report alarm_report;
	// Abnormal Report
	struct cls_edma_abnormal_report abnormal_report;
	// Abnormal Statistics and Report
	struct cls_edma_abnormal_stats abnormal_stats;
	// Normal Statistics and Report
	struct cls_edma_normal_stats normal_stats;
};

void cls_edma_stats_dump(struct cls_eth_priv *adapter, u8 option);
#endif /* _CLS_NPE_EDMA_STATS_H_ */
