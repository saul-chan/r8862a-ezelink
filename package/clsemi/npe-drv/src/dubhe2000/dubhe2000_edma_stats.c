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
/* dubhe1000_edma_stats.c
 * Shared functions for accessing and configuring the Statistics of edma
 */

#include "dubhe2000.h"
#include "dubhe2000_edma_stats.h"

struct dubhe1000_edma_stats edma_stats;

// 1. Maintenance and Testing Report
void dubhe1000_edma_update_maint_test(struct dubhe1000_adapter *adapter, struct dubhe1000_edma_maint_test *maint_test)
{
	u32 value[7];

	value[0] = edma_debug_r32(FSM_RPT);
	value[1] = edma_debug_r32(TX_FIFO_RPT);
	value[2] = edma_debug_r32(RX_FIFO_RPT);
	value[3] = edma_debug_r32(BUS_READY_RPT);
	value[4] = edma_debug_r32(DOWNSTREAM_RPT0);
	value[5] = edma_debug_r32(DOWNSTREAM_RPT1);
	value[6] = edma_debug_r32(UPSTREAM_RPT);

	memcpy(maint_test, value, sizeof(value));
}

void dubhe1000_edma_dump_maint_test(struct dubhe1000_adapter *adapter, struct dubhe1000_edma_maint_test *maint_test)
{
	u32 value[7];

	memcpy(value, maint_test, sizeof(value));

	pr_info("== 1. Maintenance and Testing Report ==\n");
	pr_info("1.1 FSM_RPT                     0x%x\n", value[0]);
	pr_info("    tx_dma_cstate:              0x%x\n", maint_test->fsm_rpt.tx_dma_cstate);
	pr_info("    rx_dma_cstate:              0x%x\n", maint_test->fsm_rpt.rx_dma_cstate);

	pr_info("1.2 TX_FIFO_RPT                 0x%x\n", value[1]);
	pr_info("    FieldName        FULL      EMPTY\n");
	pr_info("      tx_data:        0x%x        0x%x\n", maint_test->tx_fifo_rpt.tx_data_fifo_full,
		maint_test->tx_fifo_rpt.tx_data_fifo_empty);
	pr_info("      tx_tag:         0x%x        0x%x\n", maint_test->tx_fifo_rpt.tx_tag_fifo_full,
		maint_test->tx_fifo_rpt.tx_tag_fifo_empty);
	pr_info("      bmu_para:       0x%x        0x%x\n", maint_test->tx_fifo_rpt.bmu_para_fifo_full,
		maint_test->tx_fifo_rpt.bmu_para_fifo_empty);
	pr_info("      tx_req:         0x%x        0x%x\n", maint_test->tx_fifo_rpt.tx_req_fifo_full,
		maint_test->tx_fifo_rpt.tx_req_fifo_empty);
	pr_info("      head_ack:       0x%x        0x%x\n", maint_test->tx_fifo_rpt.head_ack_fifo_full,
		maint_test->tx_fifo_rpt.head_ack_fifo_empty);
	pr_info("      body_ack:       0x%x        0x%x\n", maint_test->tx_fifo_rpt.body_ack_fifo_full,
		maint_test->tx_fifo_rpt.body_ack_fifo_empty);
	pr_info("      buf_free:       0x%x        0x%x\n", maint_test->tx_fifo_rpt.buf_free_fifo_full,
		maint_test->tx_fifo_rpt.buf_free_fifo_empty);
	pr_info("      tx_descr:       0x%x        0x%x\n", maint_test->tx_fifo_rpt.tx_descr_fifo_full,
		maint_test->tx_fifo_rpt.tx_descr_fifo_empty);
	pr_info("      tx_status:      0x%x        0x%x\n", maint_test->tx_fifo_rpt.tx_status_fifo_full,
		maint_test->tx_fifo_rpt.tx_status_fifo_empty);
	pr_info("    tx_halt:          0x%x\n", maint_test->tx_fifo_rpt.tx_halt);

	pr_info("1.3 RX_FIFO_RPT                 0x%x\n", value[2]);
	pr_info("    FieldName        FULL      EMPTY\n");
	pr_info("      rx_data:        0x%x        0x%x\n", maint_test->rx_fifo_rpt.rx_data_fifo_full,
		maint_test->rx_fifo_rpt.rx_data_fifo_empty);
	pr_info("      rx_descr:       0x%x        0x%x\n", maint_test->rx_fifo_rpt.rx_descr_fifo_full,
		maint_test->rx_fifo_rpt.rx_descr_fifo_empty);
	pr_info("      rx_status:      0x%x        0x%x\n", maint_test->rx_fifo_rpt.rx_status_fifo_full,
		maint_test->rx_fifo_rpt.rx_status_fifo_empty);
	pr_info("    rx_halt:          0x%x\n", maint_test->rx_fifo_rpt.rx_halt);

	pr_info("1.4 BUS_READY_RPT               0x%x\n", value[3]);
	pr_info("    awready_rpt:                0x%x\n", maint_test->bus_ready_rpt.awready_rpt);
	pr_info("    wready_rpt:                 0x%x\n", maint_test->bus_ready_rpt.wready_rpt);
	pr_info("    bready_rpt:                 0x%x\n", maint_test->bus_ready_rpt.bready_rpt);
	pr_info("    arready_rpt:                0x%x\n", maint_test->bus_ready_rpt.arready_rpt);
	pr_info("    rready_rpt:                 0x%x\n", maint_test->bus_ready_rpt.rready_rpt);

	pr_info("1.5 DOWNSTREAM_RPT0             0x%x\n", value[4]);
	pr_info("    dwstrm_r_state:             0x%x\n", maint_test->downstream_rpt0.dwstrm_r_state);
	pr_info("    dwstrm_ar_state:            0x%x\n", maint_test->downstream_rpt0.dwstrm_ar_state);
	pr_info("    dwstrm_context_tag:         0x%x\n", maint_test->downstream_rpt0.dwstrm_context_tag);
	pr_info("    dwstrm_channel_req:         0x%x\n", maint_test->downstream_rpt0.dwstrm_channel_req);
	pr_info("    dwstrm_channel_sel:         0x%x\n", maint_test->downstream_rpt0.dwstrm_channel_sel);
	pr_info("    dwstrm_channel_tag:         0x%x\n", maint_test->downstream_rpt0.dwstrm_channel_tag);
	pr_info("    dwstrm_context_remain:      0x%x\n", maint_test->downstream_rpt0.dwstrm_context_remain);

	pr_info("1.6 DOWNSTREAM_RPT1             0x%x\n", value[5]);
	pr_info("    dwstrm_rready:              0x%x\n", maint_test->downstream_rpt1.dwstrm_rready);
	pr_info("    dwstrm_rvalid:              0x%x\n", maint_test->downstream_rpt1.dwstrm_rvalid);
	pr_info("    dwstrm_rlast:               0x%x\n", maint_test->downstream_rpt1.dwstrm_rlast);
	pr_info("    dwstrm_rresp:               0x%x\n", maint_test->downstream_rpt1.dwstrm_rresp);
	pr_info("    dwstrm_rid:                 0x%x\n", maint_test->downstream_rpt1.dwstrm_rid);
	pr_info("    dwstrm_arready:             0x%x\n", maint_test->downstream_rpt1.dwstrm_arready);
	pr_info("    dwstrm_arvalid:             0x%x\n", maint_test->downstream_rpt1.dwstrm_arvalid);
	pr_info("    dwstrm_arid:                0x%x\n", maint_test->downstream_rpt1.dwstrm_arid);
	pr_info("    dwstrm_error:               0x%x\n", maint_test->downstream_rpt1.dwstrm_error);
	pr_info("    dwstrm_state:               0x%x\n", maint_test->downstream_rpt1.dwstrm_state);
	pr_info("    dwstrm_count:               0x%x\n", maint_test->downstream_rpt1.dwstrm_count);
	pr_info("    dwstrm_bus_ready:           0x%x\n", maint_test->downstream_rpt1.dwstrm_bus_ready);
	pr_info("    dwstrm_data_en:             0x%x\n", maint_test->downstream_rpt1.dwstrm_data_en);
	pr_info("    dwstrm_start:               0x%x\n", maint_test->downstream_rpt1.dwstrm_start);
	pr_info("    dwstrm_stall:               0x%x\n", maint_test->downstream_rpt1.dwstrm_stall);

	pr_info("1.7 UPSTREAM_RPT                0x%x\n", value[6]);
	pr_info("    upstrm_aw_state:            0x%x\n", maint_test->upstream_rpt.upstrm_aw_state);
	pr_info("    upstrm_b_state:             0x%x\n", maint_test->upstream_rpt.upstrm_b_state);
	pr_info("    upstrm_pipe_wr_ready:       0x%x\n", maint_test->upstream_rpt.upstrm_pipe_wr_ready);
	pr_info("    upstrm_send_req_rdy:        0x%x\n", maint_test->upstream_rpt.upstrm_send_req_rdy);
	pr_info("    upstrm_datai_rdy:           0x%x\n", maint_test->upstream_rpt.upstrm_datai_rdy);
	pr_info("    upstrm_ot_fifo_empty:       0x%x\n", maint_test->upstream_rpt.upstrm_ot_fifo_empty);
	pr_info("    upstrm_ot_fifo_full:        0x%x\n", maint_test->upstream_rpt.upstrm_ot_fifo_full);
	pr_info("    upstrm_req_push_rdy:        0x%x\n", maint_test->upstream_rpt.upstrm_req_push_rdy);
	pr_info("    upstrm_ack_pop_vld:         0x%x\n", maint_test->upstream_rpt.upstrm_ack_pop_vld);
	pr_info("    upstrm_last_ack:            0x%x\n", maint_test->upstream_rpt.upstrm_last_ack);
	pr_info("    upstrm_state:               0x%x\n", maint_test->upstream_rpt.upstrm_state);
	pr_info("    upstrm_count:               0x%x\n", maint_test->upstream_rpt.upstrm_count);
	pr_info("    upstrm_bus_ready:           0x%x\n", maint_test->upstream_rpt.upstrm_bus_ready);
	pr_info("    upstrm_bus_trans:           0x%x\n", maint_test->upstream_rpt.upstrm_bus_trans);
	pr_info("    upstrm_start:               0x%x\n", maint_test->upstream_rpt.upstrm_start);
	pr_info("    upstrm_pause:               0x%x\n", maint_test->upstream_rpt.upstrm_pause);

	pr_info("=======================================\n");
}

// 2. Alarm Report
void dubhe1000_edma_update_alarm_report(struct dubhe1000_adapter *adapter,
					struct dubhe1000_edma_alarm_report *alarm_report)
{
	int i;
	u32 value[17];

	value[0] = edma_debug_r32(CFG_ALM);

	for (i = 0; i < DUBHE1000_Q_DIV32; i++)
		value[1 + i] = edma_debug_r32(TX_TOKEN_ERROR_ALM + 4 * i);

	memcpy(alarm_report, value, sizeof(value));
}

void dubhe1000_edma_dump_alarm_report(struct dubhe1000_adapter *adapter,
				      struct dubhe1000_edma_alarm_report *alarm_report)
{
	int i;
	u32 value[2];

	memcpy(value, alarm_report, sizeof(value));

	pr_info("=========== 2. Alarm Report ===========\n");

	pr_info("2.1 CFG_ALM                     0x%x\n", value[0]);
	pr_info("    head_block_size_alm:        0x%x\n", alarm_report->cfg_alm.head_block_size_alm);
	pr_info("    body_block_size_alm:        0x%x\n", alarm_report->cfg_alm.body_block_size_alm);
	pr_info("    tx_descr_fifo_depth_alm:    0x%x\n", alarm_report->cfg_alm.tx_descr_fifo_depth_alm);
	pr_info("    rx_descr_fifo_depth_alm:    0x%x\n", alarm_report->cfg_alm.rx_descr_fifo_depth_alm);
	pr_info("    tx_status_fifo_depth_alm:   0x%x\n", alarm_report->cfg_alm.tx_status_fifo_depth_alm);
	pr_info("    rx_status_fifo_depth_alm:   0x%x\n", alarm_report->cfg_alm.rx_status_fifo_depth_alm);
	pr_info("    tx_bus_err_alm:             0x%x\n", alarm_report->cfg_alm.tx_bus_err_alm);
	pr_info("    rx_bus_err_alm:             0x%x\n", alarm_report->cfg_alm.rx_bus_err_alm);
	pr_info("    tx_token_cfg_err_alm:       0x%x\n", alarm_report->cfg_alm.tx_token_cfg_err_alm);
	pr_info("    tx_buf_clr_err_alm:         0x%x\n", alarm_report->cfg_alm.tx_buf_clr_err_alm);
	pr_info("    tx_descr_wr_err_alm:        0x%x\n", alarm_report->cfg_alm.tx_descr_wr_err_alm);
	pr_info("    rx_descr_wr_err_alm:        0x%x\n", alarm_report->cfg_alm.rx_descr_wr_err_alm);
	pr_info("    tx_status_clr_err_alm:      0x%x\n", alarm_report->cfg_alm.tx_status_clr_err_alm);
	pr_info("    rx_status_clr_err_alm:      0x%x\n", alarm_report->cfg_alm.rx_status_clr_err_alm);
	pr_info("    tx_status_0_ddr_alm:        0x%x\n", alarm_report->cfg_alm.tx_status_0_ddr_alm);
	pr_info("    tx_status_1_ddr_alm:        0x%x\n", alarm_report->cfg_alm.tx_status_1_ddr_alm);
	pr_info("    tx_status_2_ddr_alm:        0x%x\n", alarm_report->cfg_alm.tx_status_2_ddr_alm);
	pr_info("    tx_status_3_ddr_alm:        0x%x\n", alarm_report->cfg_alm.tx_status_3_ddr_alm);
	pr_info("    tx_hw_bmu_err_alm:          0x%x\n", alarm_report->cfg_alm.tx_hw_bmu_err_alm);
	pr_info("2.2 TX_TOKEN_ERROR_ALM:\n");
	for (i = 0; i < DUBHE1000_Q_DIV32; i++) {
		if (alarm_report->tx_token_error_alm.token_underflow_alm[i])
			pr_info("    token_underflow_alm[0x%x]:     0x%x\n", i,
				alarm_report->tx_token_error_alm.token_underflow_alm[i]);
	}

	pr_info("=======================================\n");
}

// 3. Abnormal Report
void dubhe1000_edma_update_abnormal_report(struct dubhe1000_adapter *adapter,
					   struct dubhe1000_edma_abnormal_report *abnormal_report)
{
	u32 value[1];

	value[0] = edma_debug_r32(ABNOR_ALM);

	memcpy(abnormal_report, value, sizeof(value));
}

void dubhe1000_edma_dump_abnormal_report(struct dubhe1000_adapter *adapter,
					 struct dubhe1000_edma_abnormal_report *abnormal_report)
{
	u32 value[1];

	memcpy(value, abnormal_report, sizeof(value));

	pr_info("========== 3. Abnormal Report =========\n");
	pr_info("3.1 ABNOR_ALM                   0x%x\n", value[0]);
	pr_info("    tx_timeout_abn:             0x%x\n", abnormal_report->abnor_alm.tx_timeout_abn);
	pr_info("    rx_timeout_abn:             0x%x\n", abnormal_report->abnor_alm.rx_timeout_abn);
	pr_info("=======================================\n");
}

// 4. Abnormal Statistics and Report
void dubhe1000_edma_update_abnormal_stats(struct dubhe1000_adapter *adapter,
					  struct dubhe1000_edma_abnormal_stats *abnormal_stats)
{
	u32 value[9];

	value[0] = edma_debug_r32(TX_ERROR_CNT);
	value[1] = edma_debug_r32(TX_BMU_ERROR_CNT);
	value[2] = edma_debug_r32(TX_BLK_ERROR_CNT);
	value[3] = edma_debug_r32(TX_PKT_LEN_ERROR_CNT);
	value[4] = edma_debug_r32(TX_TOKEN_FETCH_ERROR_CNT);
	value[5] = edma_debug_r32(TX_EOP_ERROR_CNT);
	value[6] = edma_debug_r32(TX_FREE_ERROR_CNT);
	value[7] = edma_debug_r32(RX_ERROR_CNT);
	value[8] = edma_debug_r32(RX_PKT_LEN_ERROR_CNT);

	memcpy(abnormal_stats, value, sizeof(value));
}

void dubhe1000_edma_dump_abnormal_stats(struct dubhe1000_adapter *adapter,
					struct dubhe1000_edma_abnormal_stats *abnormal_stats)
{
	u32 value[9];

	memcpy(value, abnormal_stats, sizeof(value));

	pr_info("========== 4. Abnormal Stats ==========\n");

	pr_info("4.1 TX_ERROR_CNT                0x%x\n", value[0]);
	pr_info("    tx_err_pkt_cnt:             0x%x\n", abnormal_stats->tx_error_cnt.tx_err_pkt_cnt);
	pr_info("4.2 TX_BMU_ERROR_CNT            0x%x\n", value[1]);
	pr_info("    alloc_err_cnt:              0x%x\n", abnormal_stats->tx_bmu_error_cnt.alloc_err_cnt);
	pr_info("4.3 TX_BLK_ERROR_CNT            0x%x\n", value[2]);
	pr_info("    blk_ovf_cnt:                0x%x\n", abnormal_stats->tx_blk_error_cnt.blk_ovf_cnt);
	pr_info("4.4 TX_PKT_LEN_ERROR_CNT        0x%x\n", value[3]);
	pr_info("    tx_length_err_cnt:          0x%x\n", abnormal_stats->tx_pkt_len_error_cnt.tx_length_err_cnt);
	pr_info("4.5 TX_TOKEN_FETCH_ERROR_CNT    0x%x\n", value[4]);
	pr_info("    token_fetch_err_cnt:        0x%x\n", abnormal_stats->tx_token_fetch_error_cnt.token_fetch_err_cnt);
	pr_info("4.6 TX_EOP_ERROR_CNT            0x%x\n", value[5]);
	pr_info("    tx_early_eop_err_cnt:       0x%x\n", abnormal_stats->tx_eop_error_cnt.tx_early_eop_err_cnt);
	pr_info("    tx_late_eop_err_cnt:        0x%x\n", abnormal_stats->tx_eop_error_cnt.tx_late_eop_err_cnt);
	pr_info("4.7 TX_FREE_ERROR_CNT           0x%x\n", value[6]);
	pr_info("    tx_free_err_cnt:            0x%x\n", abnormal_stats->tx_free_error_cnt.tx_free_err_cnt);
	pr_info("4.8 RX_ERROR_CNT                0x%x\n", value[7]);
	pr_info("    rx_err_pkt_cnt:             0x%x\n", abnormal_stats->rx_error_cnt.rx_err_pkt_cnt);
	pr_info("4.9 RX_PKT_LEN_ERROR_CNT        0x%x\n", value[8]);
	pr_info("    rx_pkt_len_err_cnt:         0x%x\n", abnormal_stats->rx_pkt_len_error_cnt.rx_pkt_len_err_cnt);

	pr_info("=======================================\n");
}

// 5. Normal Statistics and Report
void dubhe1000_edma_update_normal_stats(struct dubhe1000_adapter *adapter,
					struct dubhe1000_edma_normal_stats *normal_stats)
{
	u32 value[29];

	value[0] = edma_debug_r32(TX_PKT_CNT);
	value[1] = edma_debug_r32(TX_PKT_BYTE_CNT_LO);
	value[2] = edma_debug_r32(TX_PKT_BYTE_CNT_HI);
	value[3] = edma_debug_r32(TX_MIRROR_PKT_CNT);
	value[4] = edma_debug_r32(TX_PKT_NORMAL_CNT);
	value[5] = edma_debug_r32(RX_PKT_CNT);
	value[6] = edma_debug_r32(RX_PKT_BYTE_CNT_LO);
	value[7] = edma_debug_r32(RX_PKT_BYTE_CNT_HI);
	value[8] = edma_debug_r32(RX_PKT_NORMAL_CNT);

	value[9] = edma_debug_r32(STATUS_0_FULL_DROP_CNT);
	value[10] = edma_debug_r32(STATUS_1_FULL_DROP_CNT);
	value[11] = edma_debug_r32(STATUS_2_FULL_DROP_CNT);
	value[12] = edma_debug_r32(STATUS_3_FULL_DROP_CNT);
	value[13] = edma_debug_r32(SHORT_PKT_DROP_CNT);
	value[14] = edma_debug_r32(SHORT_PKT_LEN_DROP_CNT);
	value[15] = edma_debug_r32(DDR_EMPTY);
	value[16] = edma_debug_r32(DDR_FULL);
	value[17] = edma_debug_r32(STATUS_0_IN_OCCUPANCY);
	value[18] = edma_debug_r32(STATUS_1_IN_OCCUPANCY);
	value[19] = edma_debug_r32(STATUS_2_IN_OCCUPANCY);
	value[20] = edma_debug_r32(STATUS_3_IN_OCCUPANCY);
	value[21] = edma_debug_r32(DDR_STATUS_0_OCCUPANCY);
	value[22] = edma_debug_r32(DDR_STATUS_1_OCCUPANCY);
	value[23] = edma_debug_r32(DDR_STATUS_2_OCCUPANCY);
	value[24] = edma_debug_r32(DDR_STATUS_3_OCCUPANCY);
	value[25] = edma_debug_r32(HOST_PKT_CNT);
	value[26] = edma_debug_r32(WIFI40_PKT_CNT);
	value[27] = edma_debug_r32(WIFI160_PKT_CNT);
	value[28] = edma_debug_r32(PCIE_PKT_CNT);
	memcpy(normal_stats, value, sizeof(value));
}

void dubhe1000_edma_dump_normal_stats(struct dubhe1000_adapter *adapter,
				      struct dubhe1000_edma_normal_stats *normal_stats)
{
	u32 value[29];

	memcpy(value, normal_stats, sizeof(value));

	pr_info("=========== 5. Normal Stats ===========\n");
	pr_info("5.1 TX_PKT_CNT                  0x%x\n", value[0]);
	pr_info("    tx_pkt_cnt:                 0x%x\n", normal_stats->tx_pkt_cnt.tx_pkt_cnt);
	pr_info("5.2 TX_PKT_BYTE_CNT_LO          0x%x\n", value[1]);
	pr_info("    tx_pkt_byte_cnt_l:          0x%x\n", normal_stats->tx_pkt_byte_cnt_lo.tx_pkt_byte_cnt_l);
	pr_info("5.3 TX_PKT_BYTE_CNT_HI          0x%x\n", value[2]);
	pr_info("    tx_pkt_byte_cnt_h:          0x%x\n", normal_stats->tx_pkt_byte_cnt_hi.pkt_byte_cnt_h);
	pr_info("    tx_pkt_byte_cnt_clr:        0x%x\n", normal_stats->tx_pkt_byte_cnt_hi.pkt_byte_cnt_clr);
	pr_info("5.4 TX_MIRROR_PKT_CNT           0x%x\n", value[3]);
	pr_info("    tx_mirror_pkt_cnt:          0x%x\n", normal_stats->tx_mirror_pkt_cnt.tx_mirror_pkt_cnt);
	pr_info("5.5 TX_PKT_NORMAL_CNT           0x%x\n", value[4]);
	pr_info("    tx_normal_pkt_cnt:          0x%x\n", normal_stats->tx_pkt_normal_cnt.tx_normal_pkt_cnt);
	pr_info("5.6 RX_PKT_CNT                  0x%x\n", value[5]);
	pr_info("    rx_pkt_cnt:                 0x%x\n", normal_stats->rx_pkt_cnt.rx_pkt_cnt);
	pr_info("5.7 RX_PKT_BYTE_CNT_LO          0x%x\n", value[6]);
	pr_info("    rx_pkt_byte_cnt_l:          0x%x\n", normal_stats->rx_pkt_byte_cnt_lo.rx_pkt_byte_cnt_l);
	pr_info("5.8 RX_PKT_BYTE_CNT_HI          0x%x\n", value[7]);
	pr_info("    rx_pkt_byte_cnt_h:          0x%x\n", normal_stats->rx_pkt_byte_cnt_hi.pkt_byte_cnt_h);
	pr_info("    rx_pkt_byte_cnt_clr:        0x%x\n", normal_stats->rx_pkt_byte_cnt_hi.pkt_byte_cnt_clr);
	pr_info("5.9 RX_PKT_NORMAL_CNT           0x%x\n", value[8]);
	pr_info("    rx_normal_pkt_cnt:          0x%x\n", normal_stats->rx_pkt_normal_cnt.rx_normal_pkt_cnt);
	pr_info("5.10 STATUS_0_FULL_DROP_CNT     0x%x\n", value[9]);
	pr_info("    tx_status_0_full_drop_cnt:  0x%x\n",
		normal_stats->tx_status_0_full_drop_cnt.tx_status_0_full_drop_cnt);
	pr_info("5.11 STATUS_1_FULL_DROP_CNT     0x%x\n", value[10]);
	pr_info("    tx_status_1_full_drop_cnt:  0x%x\n",
		normal_stats->tx_status_1_full_drop_cnt.tx_status_1_full_drop_cnt);
	pr_info("5.12 STATUS_2_FULL_DROP_CNT     0x%x\n", value[11]);
	pr_info("    tx_status_2_full_drop_cnt:  0x%x\n",
		normal_stats->tx_status_2_full_drop_cnt.tx_status_2_full_drop_cnt);
	pr_info("5.13 STATUS_3_FULL_DROP_CNT     0x%x\n", value[12]);
	pr_info("    tx_status_3_full_drop_cnt:  0x%x\n",
		normal_stats->tx_status_3_full_drop_cnt.tx_status_3_full_drop_cnt);
	pr_info("5.14 SHORT_PKT_DROP_CNT         0x%x\n", value[13]);
	pr_info("    tx_short_pkt_drop_cnt:      0x%x\n", normal_stats->tx_short_pkt_drop_cnt.tx_short_pkt_drop_cnt);
	pr_info("5.15 SHORT_PKT_LEN_DROP_CNT     0x%x\n", value[14]);
	pr_info("    tx_short_pkt_len_drop_cnt:  0x%x\n",
		normal_stats->tx_short_pkt_len_drop_cnt.tx_short_pkt_len_drop_cnt);
	pr_info("5.16 DDR_EMPTY                  0x%x\n", value[15]);
	pr_info("    tx_status_0_ddr_empty:      0x%x\n", normal_stats->tx_status_ddr_empty.tx_status_0_ddr_empty);
	pr_info("    tx_status_1_ddr_empty:      0x%x\n", normal_stats->tx_status_ddr_empty.tx_status_1_ddr_empty);
	pr_info("    tx_status_2_ddr_empty:      0x%x\n", normal_stats->tx_status_ddr_empty.tx_status_2_ddr_empty);
	pr_info("    tx_status_3_ddr_empty:      0x%x\n", normal_stats->tx_status_ddr_empty.tx_status_3_ddr_empty);
	pr_info("5.17 DDR_FULL                   0x%x\n", value[16]);
	pr_info("    tx_status_0_ddr_full:       0x%x\n", normal_stats->tx_status_ddr_full.tx_status_0_ddr_full);
	pr_info("    tx_status_1_ddr_full:       0x%x\n", normal_stats->tx_status_ddr_full.tx_status_1_ddr_full);
	pr_info("    tx_status_2_ddr_full:       0x%x\n", normal_stats->tx_status_ddr_full.tx_status_2_ddr_full);
	pr_info("    tx_status_3_ddr_full:       0x%x\n", normal_stats->tx_status_ddr_full.tx_status_3_ddr_full);
	pr_info("5.18 STATUS_0_IN_OCCUPANCY      0x%x\n", value[17]);
	pr_info("    status_0_in_occupancy:      0x%x\n", normal_stats->status_0_in_occupancy.status_0_in_occupancy);
	pr_info("5.19 STATUS_1_IN_OCCUPANCY      0x%x\n", value[18]);
	pr_info("    status_1_in_occupancy:      0x%x\n", normal_stats->status_1_in_occupancy.status_1_in_occupancy);
	pr_info("5.20 STATUS_2_IN_OCCUPANCY      0x%x\n", value[19]);
	pr_info("    status_2_in_occupancy:      0x%x\n", normal_stats->status_2_in_occupancy.status_2_in_occupancy);
	pr_info("5.21 STATUS_3_IN_OCCUPANCY      0x%x\n", value[20]);
	pr_info("    status_3_in_occupancy:      0x%x\n", normal_stats->status_3_in_occupancy.status_3_in_occupancy);
	pr_info("5.22 DDR_STATUS_0_OCCUPANCY     0x%x\n", value[21]);
	pr_info("    ddr_status_0_occupancy:     0x%x\n", normal_stats->ddr_status_0_occupancy.ddr_status_0_occupancy);
	pr_info("5.23 DDR_STATUS_1_OCCUPANCY     0x%x\n", value[22]);
	pr_info("    ddr_status_1_occupancy:     0x%x\n", normal_stats->ddr_status_1_occupancy.ddr_status_1_occupancy);
	pr_info("5.24 DDR_STATUS_2_OCCUPANCY     0x%x\n", value[23]);
	pr_info("    ddr_status_2_occupancy:     0x%x\n", normal_stats->ddr_status_2_occupancy.ddr_status_2_occupancy);
	pr_info("5.25 DDR_STATUS_3_OCCUPANCY     0x%x\n", value[24]);
	pr_info("    ddr_status_3_occupancy:     0x%x\n", normal_stats->ddr_status_3_occupancy.ddr_status_3_occupancy);
	pr_info("5.26 HOST_PKT_CNT               0x%x\n", value[25]);
	pr_info("    host_pkt_cnt:               0x%x\n", normal_stats->host_pkt_cnt.host_pkt_cnt);
	pr_info("5.27 WIFI40_PKT_CNT             0x%x\n", value[26]);
	pr_info("    wifi40_pkt_cnt:             0x%x\n", normal_stats->wifi40_pkt_cnt.wifi40_pkt_cnt);
	pr_info("5.28 WIFI160_PKT_CNT            0x%x\n", value[27]);
	pr_info("    wifi160_pkt_cnt:            0x%x\n", normal_stats->wifi160_pkt_cnt.wifi160_pkt_cnt);
	pr_info("5.29 PCIE_PKT_CNT               0x%x\n", value[28]);
	pr_info("    pcie_pkt_cnt:               0x%x\n", normal_stats->pcie_pkt_cnt.pcie_pkt_cnt);
	pr_info("=======================================\n");
}

void dubhe1000_edma_stats_dump(struct dubhe1000_adapter *adapter, u8 option)
{
	pr_info("\n=============== EDMA STATISTICS ===============\n");

	if (option < EDMA_STATS_OPTION_MIN || option > EDMA_STATS_OPTION_MAX)
		option = EDMA_STATS_OPTION_ALL;

	if (option == EDMA_STATS_OPTION_MAINT_TEST || option == EDMA_STATS_OPTION_ALL) {
		dubhe1000_edma_update_maint_test(adapter, &edma_stats.maint_test);
		dubhe1000_edma_dump_maint_test(adapter, &edma_stats.maint_test);
	}

	if (option == EDMA_STATS_OPTION_ALARM_REPORT || option == EDMA_STATS_OPTION_ALL) {
		dubhe1000_edma_update_alarm_report(adapter, &edma_stats.alarm_report);
		dubhe1000_edma_dump_alarm_report(adapter, &edma_stats.alarm_report);
	}

	if (option == EDMA_STATS_OPTION_ABNORMAL_REPORT || option == EDMA_STATS_OPTION_ALL) {
		dubhe1000_edma_update_abnormal_report(adapter, &edma_stats.abnormal_report);
		dubhe1000_edma_dump_abnormal_report(adapter, &edma_stats.abnormal_report);
	}

	if (option == EDMA_STATS_OPTION_ABNORMAL_STATS || option == EDMA_STATS_OPTION_ALL) {
		dubhe1000_edma_update_abnormal_stats(adapter, &edma_stats.abnormal_stats);
		dubhe1000_edma_dump_abnormal_stats(adapter, &edma_stats.abnormal_stats);
	}

	if (option == EDMA_STATS_OPTION_NORMAL_STATS || option == EDMA_STATS_OPTION_ALL) {
		dubhe1000_edma_update_normal_stats(adapter, &edma_stats.normal_stats);
		dubhe1000_edma_dump_normal_stats(adapter, &edma_stats.normal_stats);
	}

	pr_info("===============================================\n");
}
