// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2023 Clourneysemi Corporation. */
/* cls_edma_stats.c
 * Shared functions for accessing and configuring the Statistics of edma
 */

#include "cls_npe.h"
#include "cls_npe_edma_stats.h"
#include "cls_npe_osdep.h"


struct cls_edma_stats edma_stats;

// 1. Maintenance and Testing Report
void cls_edma_update_maint_test(struct cls_eth_priv *adapter,
		struct cls_edma_maint_test *maint_test)
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

void cls_edma_dump_maint_test(struct cls_eth_priv *adapter,
		struct cls_edma_maint_test *maint_test)
{
	u32 value[7];

	memcpy(value, maint_test, sizeof(value));

	printk("## 1. Maintenance and Testing Report ##\n");
	printk("1.1 FSM_RPT                     0x%x\n", value[0]);
	printk("    tx_dma_cstate:              %d\n", maint_test->fsm_rpt.tx_dma_cstate);
	printk("    rx_dma_cstate:              %d\n", maint_test->fsm_rpt.rx_dma_cstate);

	printk("1.2 TX_FIFO_RPT                 0x%x\n", value[1]);
	printk("    FieldName        FULL      EMPTY\n");
	printk("      tx_data:        %d        %d\n",
			maint_test->tx_fifo_rpt.tx_data_fifo_full, maint_test->tx_fifo_rpt.tx_data_fifo_empty);
	printk("      tx_tag:         %d        %d\n",
			maint_test->tx_fifo_rpt.tx_tag_fifo_full, maint_test->tx_fifo_rpt.tx_tag_fifo_empty);
	printk("      bmu_para:       %d        %d\n",
			maint_test->tx_fifo_rpt.bmu_para_fifo_full, maint_test->tx_fifo_rpt.bmu_para_fifo_empty);
	printk("      tx_req:         %d        %d\n",
			maint_test->tx_fifo_rpt.tx_req_fifo_full, maint_test->tx_fifo_rpt.tx_req_fifo_empty);
	printk("      head_ack:       %d        %d\n",
			maint_test->tx_fifo_rpt.head_ack_fifo_full, maint_test->tx_fifo_rpt.head_ack_fifo_empty);
	printk("      body_ack:       %d        %d\n",
			maint_test->tx_fifo_rpt.body_ack_fifo_full, maint_test->tx_fifo_rpt.body_ack_fifo_empty);
	printk("      buf_free:       %d        %d\n",
			maint_test->tx_fifo_rpt.buf_free_fifo_full, maint_test->tx_fifo_rpt.buf_free_fifo_empty);
	printk("      tx_descr:       %d        %d\n",
			maint_test->tx_fifo_rpt.tx_descr_fifo_full, maint_test->tx_fifo_rpt.tx_descr_fifo_empty);
	printk("      tx_status:      %d        %d\n",
			maint_test->tx_fifo_rpt.tx_status_fifo_full, maint_test->tx_fifo_rpt.tx_status_fifo_empty);
	printk("    tx_halt:          %d\n", maint_test->tx_fifo_rpt.tx_halt);

	printk("1.3 RX_FIFO_RPT                 0x%x\n", value[2]);
	printk("    FieldName        FULL      EMPTY\n");
	printk("      rx_data:        %d        %d\n",
			maint_test->rx_fifo_rpt.rx_data_fifo_full, maint_test->rx_fifo_rpt.rx_data_fifo_empty);
	printk("      rx_descr:       %d        %d\n",
			maint_test->rx_fifo_rpt.rx_descr_fifo_full, maint_test->rx_fifo_rpt.rx_descr_fifo_empty);
	printk("      rx_status:      %d        %d\n",
			maint_test->rx_fifo_rpt.rx_status_fifo_full, maint_test->rx_fifo_rpt.rx_status_fifo_empty);
	printk("    rx_halt:          %d\n", maint_test->rx_fifo_rpt.rx_halt);

	printk("1.4 BUS_READY_RPT               0x%x\n", value[3]);
	printk("    awready_rpt:                %d\n", maint_test->bus_ready_rpt.awready_rpt);
	printk("    wready_rpt:                 %d\n", maint_test->bus_ready_rpt.wready_rpt);
	printk("    bready_rpt:                 %d\n", maint_test->bus_ready_rpt.bready_rpt);
	printk("    arready_rpt:                %d\n", maint_test->bus_ready_rpt.arready_rpt);
	printk("    rready_rpt:                 %d\n", maint_test->bus_ready_rpt.rready_rpt);

	printk("1.5 DOWNSTREAM_RPT0             0x%x\n", value[4]);
	printk("    dwstrm_r_state:             %d\n", maint_test->downstream_rpt0.dwstrm_r_state);
	printk("    dwstrm_ar_state:            %d\n", maint_test->downstream_rpt0.dwstrm_ar_state);
	printk("    dwstrm_context_tag:         %d\n", maint_test->downstream_rpt0.dwstrm_context_tag);
	printk("    dwstrm_channel_req:         %d\n", maint_test->downstream_rpt0.dwstrm_channel_req);
	printk("    dwstrm_channel_sel:         %d\n", maint_test->downstream_rpt0.dwstrm_channel_sel);
	printk("    dwstrm_channel_tag:         %d\n", maint_test->downstream_rpt0.dwstrm_channel_tag);
	printk("    dwstrm_context_remain:      %d\n", maint_test->downstream_rpt0.dwstrm_context_remain);

	printk("1.6 DOWNSTREAM_RPT1             0x%x\n", value[5]);
	printk("    dwstrm_rready:              %d\n", maint_test->downstream_rpt1.dwstrm_rready);
	printk("    dwstrm_rvalid:              %d\n", maint_test->downstream_rpt1.dwstrm_rvalid);
	printk("    dwstrm_rlast:               %d\n", maint_test->downstream_rpt1.dwstrm_rlast);
	printk("    dwstrm_rresp:               %d\n", maint_test->downstream_rpt1.dwstrm_rresp);
	printk("    dwstrm_rid:                 %d\n", maint_test->downstream_rpt1.dwstrm_rid);
	printk("    dwstrm_arready:             %d\n", maint_test->downstream_rpt1.dwstrm_arready);
	printk("    dwstrm_arvalid:             %d\n", maint_test->downstream_rpt1.dwstrm_arvalid);
	printk("    dwstrm_arid:                %d\n", maint_test->downstream_rpt1.dwstrm_arid);
	printk("    dwstrm_error:               %d\n", maint_test->downstream_rpt1.dwstrm_error);
	printk("    dwstrm_state:               %d\n", maint_test->downstream_rpt1.dwstrm_state);
	printk("    dwstrm_count:               %d\n", maint_test->downstream_rpt1.dwstrm_count);
	printk("    dwstrm_bus_ready:           %d\n", maint_test->downstream_rpt1.dwstrm_bus_ready);
	printk("    dwstrm_data_en:             %d\n", maint_test->downstream_rpt1.dwstrm_data_en);
	printk("    dwstrm_start:               %d\n", maint_test->downstream_rpt1.dwstrm_start);
	printk("    dwstrm_stall:               %d\n", maint_test->downstream_rpt1.dwstrm_stall);

	printk("1.7 UPSTREAM_RPT                0x%x\n", value[6]);
	printk("    upstrm_aw_state:            %d\n", maint_test->upstream_rpt.upstrm_aw_state);
	printk("    upstrm_b_state:             %d\n", maint_test->upstream_rpt.upstrm_b_state);
	printk("    upstrm_pipe_wr_ready:       %d\n", maint_test->upstream_rpt.upstrm_pipe_wr_ready);
	printk("    upstrm_send_req_rdy:        %d\n", maint_test->upstream_rpt.upstrm_send_req_rdy);
	printk("    upstrm_datai_rdy:           %d\n", maint_test->upstream_rpt.upstrm_datai_rdy);
	printk("    upstrm_ot_fifo_empty:       %d\n", maint_test->upstream_rpt.upstrm_ot_fifo_empty);
	printk("    upstrm_ot_fifo_full:        %d\n", maint_test->upstream_rpt.upstrm_ot_fifo_full);
	printk("    upstrm_req_push_rdy:        %d\n", maint_test->upstream_rpt.upstrm_req_push_rdy);
	printk("    upstrm_ack_pop_vld:         %d\n", maint_test->upstream_rpt.upstrm_ack_pop_vld);
	printk("    upstrm_last_ack:            %d\n", maint_test->upstream_rpt.upstrm_last_ack);
	printk("    upstrm_state:               %d\n", maint_test->upstream_rpt.upstrm_state);
	printk("    upstrm_count:               %d\n", maint_test->upstream_rpt.upstrm_count);
	printk("    upstrm_bus_ready:           %d\n", maint_test->upstream_rpt.upstrm_bus_ready);
	printk("    upstrm_bus_trans:           %d\n", maint_test->upstream_rpt.upstrm_bus_trans);
	printk("    upstrm_start:               %d\n", maint_test->upstream_rpt.upstrm_start);
	printk("    upstrm_pause:               %d\n", maint_test->upstream_rpt.upstrm_pause);

	printk("#######################################\n");
}

// 2. Alarm Report
void cls_edma_update_alarm_report(struct cls_eth_priv *adapter,
		struct cls_edma_alarm_report *alarm_report)
{
	int i;
	u32 value[17];

	value[0] = edma_debug_r32(CFG_ALM);

	for (i = 0; i < CLS_NPE_Q_DIV32; i++)
		value[1 + i] = edma_debug_r32(TX_TOKEN_ERROR_ALM + 4 * i);

	memcpy(alarm_report, value, sizeof(value));
}

void cls_edma_dump_alarm_report(struct cls_eth_priv *adapter,
		struct cls_edma_alarm_report *alarm_report)
{
	int i;
	u32 value[2];

	memcpy(value, alarm_report, sizeof(value));

	printk("########### 2. Alarm Report ###########\n");

	printk("2.1 CFG_ALM                     0x%x\n", value[0]);
	printk("    head_block_size_alm:        %d\n", alarm_report->cfg_alm.head_block_size_alm);
	printk("    body_block_size_alm:        %d\n", alarm_report->cfg_alm.body_block_size_alm);
	printk("    tx_descr_fifo_depth_alm:    %d\n", alarm_report->cfg_alm.tx_descr_fifo_depth_alm);
	printk("    rx_descr_fifo_depth_alm:    %d\n", alarm_report->cfg_alm.rx_descr_fifo_depth_alm);
	printk("    tx_status_fifo_depth_alm:   %d\n", alarm_report->cfg_alm.tx_status_fifo_depth_alm);
	printk("    rx_status_fifo_depth_alm:   %d\n", alarm_report->cfg_alm.rx_status_fifo_depth_alm);
	printk("    tx_bus_err_alm:             %d\n", alarm_report->cfg_alm.tx_bus_err_alm);
	printk("    rx_bus_err_alm:             %d\n", alarm_report->cfg_alm.rx_bus_err_alm);
	printk("    tx_token_cfg_err_alm:       %d\n", alarm_report->cfg_alm.tx_token_cfg_err_alm);
	printk("    tx_buf_clr_err_alm:         %d\n", alarm_report->cfg_alm.tx_buf_clr_err_alm);
	printk("    tx_descr_wr_err_alm:        %d\n", alarm_report->cfg_alm.tx_descr_wr_err_alm);
	printk("    rx_descr_wr_err_alm:        %d\n", alarm_report->cfg_alm.rx_descr_wr_err_alm);
	printk("    tx_status_clr_err_alm:      %d\n", alarm_report->cfg_alm.tx_status_clr_err_alm);
	printk("    rx_status_clr_err_alm:      %d\n", alarm_report->cfg_alm.rx_status_clr_err_alm);

	printk("2.2 TX_TOKEN_ERROR_ALM:\n");
	for (i = 0; i < CLS_NPE_Q_DIV32; i++) {
		if (alarm_report->tx_token_error_alm.token_underflow_alm[i])
			printk("    token_underflow_alm[%d]:     %d\n", i, alarm_report->tx_token_error_alm.token_underflow_alm[i]);
	}

	printk("#######################################\n");
}

// 3. Abnormal Report
void cls_edma_update_abnormal_report(struct cls_eth_priv *adapter,
		struct cls_edma_abnormal_report *abnormal_report)
{
	u32 value[1];

	value[0] = edma_debug_r32(ABNOR_ALM);

	memcpy(abnormal_report, value, sizeof(value));
}

void cls_edma_dump_abnormal_report(struct cls_eth_priv *adapter,
		struct cls_edma_abnormal_report *abnormal_report)
{
	u32 value[1];

	memcpy(value, abnormal_report, sizeof(value));

	printk("########## 3. Abnormal Report #########\n");
	printk("3.1 ABNOR_ALM                   0x%x\n", value[0]);
	printk("    tx_timeout_abn:             %d\n", abnormal_report->abnor_alm.tx_timeout_abn);
	printk("    rx_timeout_abn:             %d\n", abnormal_report->abnor_alm.rx_timeout_abn);
	printk("#######################################\n");
}

// 4. Abnormal Statistics and Report
void cls_edma_update_abnormal_stats(struct cls_eth_priv *adapter,
		struct cls_edma_abnormal_stats *abnormal_stats)
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

void cls_edma_dump_abnormal_stats(struct cls_eth_priv *adapter,
		struct cls_edma_abnormal_stats *abnormal_stats)
{
	u32 value[9];

	memcpy(value, abnormal_stats, sizeof(value));

	printk("########## 4. Abnormal Stats ##########\n");

	printk("4.1 TX_ERROR_CNT                0x%x\n", value[0]);
	printk("    tx_err_pkt_cnt:             %d\n", abnormal_stats->tx_error_cnt.tx_err_pkt_cnt);
	printk("4.2 TX_BMU_ERROR_CNT            0x%x\n", value[1]);
	printk("    alloc_err_cnt:              %d\n", abnormal_stats->tx_bmu_error_cnt.alloc_err_cnt);
	printk("4.3 TX_BLK_ERROR_CNT            0x%x\n", value[2]);
	printk("    blk_ovf_cnt:                %d\n", abnormal_stats->tx_blk_error_cnt.blk_ovf_cnt);
	printk("4.4 TX_PKT_LEN_ERROR_CNT        0x%x\n", value[3]);
	printk("    tx_length_err_cnt:          %d\n", abnormal_stats->tx_pkt_len_error_cnt.tx_length_err_cnt);
	printk("4.5 TX_TOKEN_FETCH_ERROR_CNT    0x%x\n", value[4]);
	printk("    token_fetch_err_cnt:        %d\n", abnormal_stats->tx_token_fetch_error_cnt.token_fetch_err_cnt);
	printk("4.6 TX_EOP_ERROR_CNT            0x%x\n", value[5]);
	printk("    tx_early_eop_err_cnt:       %d\n", abnormal_stats->tx_eop_error_cnt.tx_early_eop_err_cnt);
	printk("    tx_late_eop_err_cnt:        %d\n", abnormal_stats->tx_eop_error_cnt.tx_late_eop_err_cnt);
	printk("4.7 TX_FREE_ERROR_CNT           0x%x\n", value[6]);
	printk("    tx_free_err_cnt:            %d\n", abnormal_stats->tx_free_error_cnt.tx_free_err_cnt);
	printk("4.8 RX_ERROR_CNT                0x%x\n", value[7]);
	printk("    rx_err_pkt_cnt:             %d\n", abnormal_stats->rx_error_cnt.rx_err_pkt_cnt);
	printk("4.9 RX_PKT_LEN_ERROR_CNT        0x%x\n", value[8]);
	printk("    rx_pkt_len_err_cnt:         %d\n", abnormal_stats->rx_pkt_len_error_cnt.rx_pkt_len_err_cnt);

	printk("#######################################\n");
}

// 5. Normal Statistics and Report
void cls_edma_update_normal_stats(struct cls_eth_priv *adapter,
		struct cls_edma_normal_stats *normal_stats)
{
	u32 value[9];

	value[0] = edma_debug_r32(TX_PKT_CNT);
	value[1] = edma_debug_r32(TX_PKT_BYTE_CNT_LO);
	value[2] = edma_debug_r32(TX_PKT_BYTE_CNT_HI);
	value[3] = edma_debug_r32(TX_MIRROR_PKT_CNT);
	value[4] = edma_debug_r32(TX_PKT_NORMAL_CNT);
	value[5] = edma_debug_r32(RX_PKT_CNT);
	value[6] = edma_debug_r32(RX_PKT_BYTE_CNT_LO);
	value[7] = edma_debug_r32(RX_PKT_BYTE_CNT_HI);
	value[8] = edma_debug_r32(RX_PKT_NORMAL_CNT);

	memcpy(normal_stats, value, sizeof(value));
}

void cls_edma_dump_normal_stats(struct cls_eth_priv *adapter,
		struct cls_edma_normal_stats *normal_stats)
{
	u32 value[9];

	memcpy(value, normal_stats, sizeof(value));

	printk("########### 5. Normal Stats ###########\n");
	printk("5.1 TX_PKT_CNT                  0x%x\n", value[0]);
	printk("    tx_pkt_cnt:                 %d\n", normal_stats->tx_pkt_cnt.tx_pkt_cnt);
	printk("5.2 TX_PKT_BYTE_CNT_LO          0x%x\n", value[1]);
	printk("    tx_pkt_byte_cnt_l:          %d\n", normal_stats->tx_pkt_byte_cnt_lo.tx_pkt_byte_cnt_l);
	printk("5.3 TX_PKT_BYTE_CNT_HI          0x%x\n", value[2]);
	printk("    tx_pkt_byte_cnt_h:          %d\n", normal_stats->tx_pkt_byte_cnt_hi.pkt_byte_cnt_h);
	printk("    tx_pkt_byte_cnt_clr:        %d\n", normal_stats->tx_pkt_byte_cnt_hi.pkt_byte_cnt_clr);
	printk("5.4 TX_MIRROR_PKT_CNT           0x%x\n", value[3]);
	printk("    tx_mirror_pkt_cnt:          %d\n", normal_stats->tx_mirror_pkt_cnt.tx_mirror_pkt_cnt);
	printk("5.5 TX_PKT_NORMAL_CNT           0x%x\n", value[4]);
	printk("    tx_normal_pkt_cnt:          %d\n", normal_stats->tx_pkt_normal_cnt.tx_normal_pkt_cnt);
	printk("5.6 RX_PKT_CNT                  0x%x\n", value[5]);
	printk("    rx_pkt_cnt:                 %d\n", normal_stats->rx_pkt_cnt.rx_pkt_cnt);
	printk("5.7 RX_PKT_BYTE_CNT_LO          0x%x\n", value[6]);
	printk("    rx_pkt_byte_cnt_l:          %d\n", normal_stats->rx_pkt_byte_cnt_lo.rx_pkt_byte_cnt_l);
	printk("5.8 RX_PKT_BYTE_CNT_HI          0x%x\n", value[7]);
	printk("    rx_pkt_byte_cnt_h:          %d\n", normal_stats->rx_pkt_byte_cnt_hi.pkt_byte_cnt_h);
	printk("    rx_pkt_byte_cnt_clr:        %d\n", normal_stats->rx_pkt_byte_cnt_hi.pkt_byte_cnt_clr);
	printk("5.9 RX_PKT_NORMAL_CNT           0x%x\n", value[8]);
	printk("    rx_normal_pkt_cnt:          %d\n", normal_stats->rx_pkt_normal_cnt.rx_normal_pkt_cnt);
	printk("#######################################\n");
}

void cls_edma_stats_dump(struct cls_eth_priv *adapter, u8 option)
{
	printk("################EDMA STATISTICS################\n");

	if (option < EDMA_STATS_OPTION_MIN || option > EDMA_STATS_OPTION_MAX)
		option = EDMA_STATS_OPTION_ALL;

	if (option == EDMA_STATS_OPTION_MAINT_TEST || option == EDMA_STATS_OPTION_ALL) {
		cls_edma_update_maint_test(adapter, &edma_stats.maint_test);
		cls_edma_dump_maint_test(adapter, &edma_stats.maint_test);
	}

	if (option == EDMA_STATS_OPTION_ALARM_REPORT || option == EDMA_STATS_OPTION_ALL) {
		cls_edma_update_alarm_report(adapter, &edma_stats.alarm_report);
		cls_edma_dump_alarm_report(adapter, &edma_stats.alarm_report);
	}

	if (option == EDMA_STATS_OPTION_ABNORMAL_REPORT || option == EDMA_STATS_OPTION_ALL) {
		cls_edma_update_abnormal_report(adapter, &edma_stats.abnormal_report);
		cls_edma_dump_abnormal_report(adapter, &edma_stats.abnormal_report);
	}

	if (option == EDMA_STATS_OPTION_ABNORMAL_STATS || option == EDMA_STATS_OPTION_ALL) {
		cls_edma_update_abnormal_stats(adapter, &edma_stats.abnormal_stats);
		cls_edma_dump_abnormal_stats(adapter, &edma_stats.abnormal_stats);
	}

	if (option == EDMA_STATS_OPTION_NORMAL_STATS || option == EDMA_STATS_OPTION_ALL) {
		cls_edma_update_normal_stats(adapter, &edma_stats.normal_stats);
		cls_edma_dump_normal_stats(adapter, &edma_stats.normal_stats);
	}

	printk("###############################################\n");
}

