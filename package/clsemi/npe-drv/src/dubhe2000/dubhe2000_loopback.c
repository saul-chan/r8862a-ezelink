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

#include "dubhe2000.h"
#include "dubhe2000_tag.h"

int dubhe1000_clean_loopback(struct napi_struct *napi, int budget);
static bool dubhe1000_clean_rx_irq_loopback(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring,
					    int *work_done, int work_to_do);
static void dubhe1000_skb_dump_simplified(struct dubhe1000_adapter *adapter, dma_addr_t dma, bool tx, u32 offset);
static netdev_tx_t dubhe1000_xmit_frame_simplified(struct dubhe1000_adapter *adapter, dma_addr_t dma, u16 len,
						   u32 offset);

static void dubhe1000_skb_dump_simplified(struct dubhe1000_adapter *adapter, dma_addr_t dma, bool tx, u32 offset)
{
	int k2, k = 0;
	u8 hdr[32] = { 0 };
	u64 address2 = 0;
	u32 len = 64;

	k2 = len / 16;

	address2 = ((u8 *)adapter->hw_data + dma + offset);
	pr_info("%s, %s packet:\n", __func__, tx ? "tx" : "rx");
	snprintf(hdr, sizeof(hdr), "skb(data[k=%d]): ", k);
	print_hex_dump(KERN_INFO, hdr, DUMP_PREFIX_NONE, 16, 1, address2, 16, 0);

	for (k = 0; k < k2; k++) {
		if (k == 0)
			continue;
		address2 += 16;

		snprintf(hdr, sizeof(hdr), "skb(data[k=%d]): ", k);
		print_hex_dump(KERN_INFO, hdr, DUMP_PREFIX_NONE, 16, 1, address2, 16, 0);
	}

	address2 += 16;
	snprintf(hdr, sizeof(hdr), "skb(data[k=%d]): ", k);
	print_hex_dump(KERN_INFO, hdr, DUMP_PREFIX_NONE, 16, 1, address2, 16, 0);
}

void dubhe1000_free_status1_loopback(struct dubhe1000_adapter *adapter, dma_addr_t dma)
{
	u32 val = 0;
	u16 buf_pid = dma / (adapter->body_block_size * 256);
	u8 buf_bid = (dma - buf_pid * adapter->body_block_size * 256) / adapter->body_block_size;

	val = buf_bid;
	val += (buf_pid << FREE_TX_BUF_PID_BIT);
	val += (0 << FREE_TX_BLK_NUM_BIT);
	val += (1 << FREE_TX_BUF_FREE_EN_BIT);
	ew32(TX_BUF_FREE_INSTR, val);

	if (netif_msg_pktdata(adapter))
		pr_info("free bid %d pid %d val 0x%x\n", buf_bid, buf_pid, val);
}

int dubhe1000_skb_free_simplified(struct dubhe1000_adapter *adapter, dma_addr_t dma)
{
	u8 *data = NULL;

	if ((adapter == NULL) || (adapter->soft_bmu_en))
		return (-1);

	// dma point to start of ETH
	data = adapter->hw_data + (dma - adapter->body_dma);
	data -= DUBHE1000_TO_CPU_TAG_HLEN;

	if (netif_msg_pktdata(adapter))
		pr_info("%s, offset 0x%x, tag 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
			__func__, (u32)dma, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

	dubhe1000_free_status1_loopback(adapter, dma - adapter->body_dma);

	return 0;
}

static void dubhe1000_unmap_and_free_tx_resource_simplified(struct dubhe1000_adapter *adapter,
							    struct dubhe1000_tx_buffer *buffer_info)
{
	u8 is_to_cpu_tag_pkt = 0;

	if (!adapter->soft_bmu_en) {
		if ((buffer_info->dma >= adapter->body_dma) &&
		    (buffer_info->dma <= (adapter->body_dma + DUBHE1000_HW_BMU_BUF_MAX(adapter)))) {
			is_to_cpu_tag_pkt = 1;
		}
	}

	if (buffer_info->dma) {
		if (netif_msg_pktdata(adapter))
			pr_info("free tx-buff buffer_info->dma %lx body_dma %lx %lx is_to_cpu_tag_pkt %d\n",
				buffer_info->dma, adapter->body_dma,
				(adapter->body_dma + DUBHE1000_HW_BMU_BUF_MAX(adapter)), is_to_cpu_tag_pkt);
		dubhe1000_skb_free_simplified(adapter, buffer_info->dma);
		buffer_info->dma = 0;
	}

	if (buffer_info->skb)
		buffer_info->skb = NULL;

	buffer_info->time_stamp = 0;
}

/* 1: RX all completed; 0: RX has pkt waiting */
int dubhe1000_clean_tx_irq_loopback(struct dubhe1000_adapter *adapter, u32 total_tx)
{
	struct dubhe1000_tx_buffer *buffer_info;
	struct dubhe1000_tx_ring *tx_ring;
	int cnt = 0;
	u32 val = 0, val2 = 0;
	u32 rx_queue_qsize = 0, rx_pkt_id = 0;
	int qsize;
	u16 rx_pkt_err = 0;
	u16 rx_status_invalid = 0;
	int i;
	u8 max = g_max_txrx;

	rx_queue_qsize = er32(RX_QUEUE_STATUS);
	rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
	rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
	rx_status_invalid = ((rx_queue_qsize >> 31) & 0x1);
	qsize = rx_queue_qsize & 0xFFF;

	tx_ring = adapter->tx_ring;
	i = tx_ring->next_to_clean;
	while (qsize > 0 && cnt < max) {
		if (rx_pkt_err || rx_status_invalid)
			pr_info("%s, ERR: queue_size 0x%x pkt-err %d, inv %d\n",
				__func__, rx_queue_qsize, rx_pkt_err, rx_status_invalid);

		buffer_info = &tx_ring->buffer_info[i];
		cnt++;
		adapter->total_tx_ack_bytes += buffer_info->length;

		if (netif_msg_pktdata(adapter))
			pr_info("%s, rx_queue_qsize 0x%x cnt %d RAW 0x%x tx-ack(pkt_id %d len %d pkt-cnt %d byte-cnt %d) dma %llx\n",
				__func__, rx_queue_qsize, cnt, val, rx_pkt_id, buffer_info->length,
				adapter->total_tx_ack_packets, adapter->total_tx_ack_bytes, buffer_info->dma);

		if (buffer_info->dma)
			dubhe1000_unmap_and_free_tx_resource_simplified(adapter, buffer_info);
		else
			pr_info("%s, dma NULL, pos %d, next 0x%x\n", __func__, i, tx_ring->buffer_info[i + 1].dma);

		memset(buffer_info, 0, sizeof(struct dubhe1000_tx_buffer));
		if (unlikely(++i == tx_ring->count))
			i = 0;

		tx_ring->next_to_clean = i; //rx_pkt_id;
		qsize--;
	}

	if (cnt) {
		val2 = ((1 << RX_QUEUE_DEL_EN_BIT) | cnt);
		ew32(RX_QUEUE_DEL_INSTR, val2);
	}

	if ((er32(RX_QUEUE_STATUS) & 0xFFF) > 0)
		return 0;
	else
		return 1;
}

static int dubhe1000_tx_map_simplified(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring,
				       dma_addr_t dma, unsigned int first, unsigned int max_per_txd, u16 len,
				       u32 offset1)
{
	struct dubhe1000_tx_buffer *buffer_info;
	unsigned int offset = 0, size, count = 0, i;
	unsigned int f, bytecount, segs;
	static u16 pkt_id = 0;

#if (EDMA_HW)
	s32 val = 0;
	s32 rx_fifo_remain_size = 0;
#endif

	i = tx_ring->next_to_use;
	if (netif_msg_pktdata(adapter))
		pr_info("%s, len %d, body dma 0x%x, offset 0x%x/%d, next to use %d\n", __func__, len,
			(u32)adapter->body_dma, (u32)dma, offset1, i);

	while (len) {
		buffer_info = &tx_ring->buffer_info[i];
		size = min(len, max_per_txd);

		buffer_info->length = size;
		/* set time_stamp *before* dma to help avoid a possible race */
		buffer_info->time_stamp = jiffies;
		buffer_info->mapped_as_page = false;

#if (EDMA_HW)
		rx_fifo_remain_size = er32(RX_DESCR_FIFO_STATUS);

		if (rx_fifo_remain_size == (adapter->lhost_fifo_depth * DUBHE1000_FIFO_DEPTH_LEVEL)) {
			pr_info("%s, rx ring full\n", __func__);
			break;
		}

		buffer_info->dma = adapter->body_dma + dma + offset1;

		pkt_id++;
		if (pkt_id == 0x7FFF)
			pkt_id = 0;
		val = size;			  // packet-len
		val |= (pkt_id << RX_PKT_ID_BIT); // RX packet ID
		val |= (1 << RX_DESCR_CFG_EN);

		buffer_info->rx_description1 = val;

		adapter->total_tx_bytes += size;
		adapter->total_tx_packets += 1;
		if (netif_msg_pktdata(adapter))
			dubhe1000_skb_dump_simplified(adapter, dma, true, offset1);
#endif
		buffer_info->next_to_watch = i;

		len -= size;
		offset += size;
		count++;
		if (len) {
			if (netif_msg_pktdata(adapter))
				pr_info("%s, more buffer for one pkt, i changed to %d\n", __func__, i);
			i++;
			if (unlikely(i == tx_ring->count))
				i = 0;
		}
	}

	bytecount = len;
	if (netif_msg_pktdata(adapter))
		pr_info("%s, i changed to %d\n", __func__, i);

	tx_ring->buffer_info[i].skb = dma;
	tx_ring->buffer_info[i].segs = 0;
	tx_ring->buffer_info[i].bytecount = bytecount;
	tx_ring->buffer_info[first].next_to_watch = i;

	return count;

dma_error:
	pr_info("TX DMA map failed\n");
	buffer_info->dma = 0;
	if (count)
		count--;

	while (count--) {
		if (i == 0)
			i += tx_ring->count;
		i--;
		buffer_info = &tx_ring->buffer_info[i];
		dubhe1000_unmap_and_free_tx_resource_simplified(adapter, buffer_info);
	}

	return 0;
}

/* get the tx-buff, which include dma, buffer_addr, the bus will map the buf to the address
 */
static void dubhe1000_tx_queue_simplified(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring,
					  int count)
{
	struct dubhe1000_tx_buffer *buffer_info;
	unsigned int i;
	u64 descr;

#if (EDMA_HW)
	s32 rx_fifo_remain_size = 0;
#endif
	i = tx_ring->next_to_use;

	while (count--) {
		buffer_info = &tx_ring->buffer_info[i];
#if (EDMA_HW)
		descr = buffer_info->rx_description1;
		ew64(RX_DESCRIPTION0, ((descr << 32) | buffer_info->dma));
		if (netif_msg_pktdata(adapter))
			pr_info("%s, reaback descr0 0x%x, descr1 0x%x\n", __func__, er32(RX_DESCRIPTION0),
				er32(RX_DESCRIPTION1));

		if (netif_msg_pktdata(adapter))
			pr_info("tx packet-id %d len %d (desc1 %lx desc0 %llx) tx-cnt(%d ack %d) clean (ack %d) use (xmit %d)\n",
				i, buffer_info->length, buffer_info->rx_description1, buffer_info->dma,
				adapter->total_tx_packets, adapter->total_tx_ack_packets, tx_ring->next_to_clean,
				tx_ring->next_to_use);
#endif
		if (unlikely(++i == tx_ring->count))
			i = 0;
	}

	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64).
	 */
	dma_wmb();

	tx_ring->next_to_use = i;
}

static netdev_tx_t dubhe1000_xmit_frame_simplified(struct dubhe1000_adapter *adapter, dma_addr_t dma, u16 len,
						   u32 offset)
{
	struct dubhe1000_tx_ring *tx_ring;
	unsigned int first, max_per_txd = (1 << 12); //DUBHE1000_MAX_DATA_PER_TXD;
	int count = 0;

	tx_ring = adapter->tx_ring;
	first = tx_ring->next_to_use;
	count = dubhe1000_tx_map_simplified(adapter, tx_ring, dma, first, max_per_txd, len, offset);

	if (count) {
		dubhe1000_tx_queue_simplified(adapter, tx_ring, count);
	} else {
		pr_info("%s, count is 0\n", __func__);
		return NETDEV_TX_BUSY;
	}

	return NETDEV_TX_OK;
}

static bool dubhe1000_clean_hw_bmu_split_rx_irq_loopback(struct dubhe1000_adapter *adapter,
							 struct dubhe1000_rx_ring *rx_ring, int *work_done,
							 int work_to_do)
{
	int cleaned_count = 0;
	bool cleaned = false;
	u8 *data, *head_data;
	u32 qstatus1, qstatus0;
	u32 total_rx_bytes = 0, total_rx_packets = 0;
	u32 tx_queue_qsize = 0;
	u8 tx_pkt_err = 0, is_split = 0;
	u8 status_invalid = 0;
	u8 buf_bid = 0, blk_num = 0;
	u8 input_port;
	u16 buf_pid = 0, packet_tid = 0;
	dma_addr_t dma, head_dma;
	struct dubhe1000_to_cpu_tag to_cpu_tag2;
	int err;
	int pkt_id;
	int kk = 0;
	int k2 = 0;
	u64 qstatus;
	u32 dma_wait[32];
	u32 tx_queue_status[32];
	u32 total_tx = 0;

	qstatus = er64(TX_QUEUE_STATUS0);
	qstatus0 = qstatus & 0xFFFFFFFF;
	pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
	is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
	tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
	status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);

	tx_queue_qsize = (qstatus0 & 0xFFF);

	while (tx_queue_qsize > 0) {
		qstatus1 = (qstatus >> 32);

		buf_bid = (qstatus1 & 0xFF);
		buf_pid = ((qstatus1 >> TX_BUF_PID_BIT) & 0x1FF);
		blk_num = ((qstatus1 >> TX_BLK_NUM_BIT) & 0x3);
		packet_tid = ((qstatus1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF);
		dma = (buf_pid * 256 + buf_bid) * adapter->body_block_size;
		head_dma = (buf_pid * 256 + buf_bid) * adapter->head_block_size;

		if (netif_msg_pktdata(adapter))
			pr_info("%s, HW-BMU rx-intr tx_queue_qsize %d rx(pkt_id %d split %d pkt_err %d bid %d pid %d blk-num %d tid %d addr %llx) %d/%d\n",
				__func__, tx_queue_qsize, pkt_id, is_split, tx_pkt_err, buf_bid, buf_pid, blk_num,
				packet_tid, dma, *work_done, work_to_do);

		ew32(TX_QUEUE_HOLD_INSTR, 1);
		(*work_done)++;
		dma_rmb(); /* read rx_buffer_info after status DD */

		head_data = adapter->head_hw_data;
		head_data += head_dma;
		prefetch(head_data);

		if (netif_msg_pktdata(adapter)) {
			pr_info("rx head %px dma %llx %llx adapter->hw_data %px:\n", head_data, adapter->head_dma,
				head_dma, adapter->head_hw_data);

			k2 = 6;
			dubhe1000_dump_regs(adapter, adapter->head_hw_data + head_dma);
			for (kk = 0; kk < k2; kk++) {
				if (kk == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->head_hw_data + head_dma + 32 * kk);
			}
			dubhe1000_dump_regs(adapter, adapter->head_hw_data + head_dma + 32 * kk);
			pr_info("skb dump finished\n");
		}

		err = dubhe1000_to_cpu_tag_parse(adapter, &to_cpu_tag2,
						 head_data + DUBHE1000_HEADROOM + adapter->head_offset);
		cleaned = true;
		cleaned_count++;

		if (unlikely((to_cpu_tag2.source_port != 0) && (to_cpu_tag2.source_port != 1))) {
			pr_info("received pkt for non-existent port(%u)\n", to_cpu_tag2.source_port);
			ew32(TX_BUF_FREE_INSTR, qstatus1 | (1 << FREE_TX_BUF_FREE_EN_BIT));
			goto dma_map;
		}

		data = adapter->hw_data;
		data += dma;
		prefetch(data);

		if (netif_msg_pktdata(adapter)) {
			pr_info("rx body %px dma %llx %llx adapter->hw_data %px:\n", data, adapter->body_dma, dma,
				adapter->hw_data);
			k2 = 8;
			dubhe1000_dump_regs(adapter, adapter->hw_data + dma);
			for (kk = 0; kk < k2; kk++) {
				if (kk == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->hw_data + dma + 32 * kk);
			}
			dubhe1000_dump_regs(adapter, adapter->hw_data + dma + 32 * kk);
		}

		// replace DA to avoid loopback in switch
		*((u32 *)(data + 4)) = 0x0100f1c0;

		dma_wait[total_tx] = dma;
		tx_queue_status[total_tx++] = qstatus1;
		if (total_tx >= g_max_txrx)
			break;

dma_map:
		if (*work_done >= work_to_do)
			break;

		/* use prefetched values */
		qstatus = er64(TX_QUEUE_STATUS0);
		qstatus0 = qstatus & 0xFFFFFFFF;
		pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
		status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);

		tx_queue_qsize = (qstatus0 & 0xFFF);
	}

	/* check we need to process txdone(irq is disabled) */
	if (dubhe1000_clean_tx_irq_loopback(adapter, total_tx) == 0) {
		cleaned = 1;
		*work_done = work_to_do;
	}

	if (netif_msg_pktdata(adapter))
		pr_info("HW BMU, total tx %d, work done %d\n", total_tx, *work_done);
	if (total_tx) {
		int j = 0;

		for (j = 0; j < total_tx; j++) {
			if (NETDEV_TX_BUSY ==
			    dubhe1000_xmit_frame_simplified(adapter, dma_wait[j], to_cpu_tag2.pakcet_length, 0)) {
				pr_info("RX Busy, free buffer directly\n");
				ew32(TX_BUF_FREE_INSTR, tx_queue_status[j] | (1 << FREE_TX_BUF_FREE_EN_BIT));
			}
		}
	}

	// re-attach the rx-buf here
	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;
	return cleaned;
}

static bool dubhe1000_clean_hw_bmu_rx_irq_loopback(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring,
						   int *work_done, int work_to_do)
{
	u32 length = 0;
	int cleaned_count = 0;
	bool cleaned = false;
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;
	int tx_queue_qsize = 0;
	u8 tx_pkt_err = 0, is_split = 0, status_invalid = 0;
	u32 qstatus1 = 0, qstatus0;
	u8 buf_bid = 0, blk_num = 0;
	u16 buf_pid = 0, packet_tid = 0;
	dma_addr_t dma;
	u32 val = 0;
	u32 dma_wait[32];
	u16 packet_len[32];
	u32 tx_queue_status[32];
	u32 total_tx = 0;
	struct dubhe1000_to_cpu_tag to_cpu_tag2;
	u32 input_port;
	int err;
	int pkt_id;
	int kk = 0;

	u8 *data;
	u8 status = 0;
	u64 qstatus;

	if (adapter->split_mode)
		return dubhe1000_clean_hw_bmu_split_rx_irq_loopback(adapter, rx_ring, work_done, work_to_do);

	qstatus = er64(TX_QUEUE_STATUS0);
	qstatus0 = qstatus & 0xFFFFFFFF;
	pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
	is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
	tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
	status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);

	tx_queue_qsize = (qstatus0 & 0xFFF);

	if (netif_msg_pktdata(adapter))
		pr_info("%s, qsize %d rx(pkt_id %d split %d pkt_err %d, status invalid %d\n",
			__func__, tx_queue_qsize, pkt_id, is_split, tx_pkt_err, status_invalid);

	while (tx_queue_qsize > 0) {
		if (status_invalid) { /* status invalid, maybe HW is not ready, SW try later */
			pr_info("%s, tx status invalid, 0x%x\n", __func__, qstatus0);
			break;
		}

		qstatus1 = (qstatus >> 32);

		buf_bid = (qstatus1 & 0xFF);
		buf_pid = ((qstatus1 >> TX_BUF_PID_BIT) & 0x1FF);
		packet_tid = ((qstatus1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF);
		blk_num = ((qstatus1 >> TX_BLK_NUM_BIT) & 0x3);
		dma = (buf_pid * 256 + buf_bid) * adapter->body_block_size;

		if (netif_msg_pktdata(adapter))
			pr_info("%s bid %d pid %d blk-num %d tid %d addr %llx) %d/%d\n", __func__, buf_bid, buf_pid,
				blk_num, packet_tid, dma, *work_done, work_to_do);

		if (tx_pkt_err) { /* err pkt, SW move to next, free this one */
			pr_info("%s, tx status invalid, status0 0x%x, status1 0x%x\n", __func__, qstatus0, qstatus1);
			ew32(TX_QUEUE_DEL_INSTR, 1);
			goto dma_map;
		}

		(*work_done)++;

		dma_rmb(); /* read rx_buffer_info after status DD */

		data = adapter->hw_data;
		data += dma;

		prefetch(data);
		ew32(TX_QUEUE_HOLD_INSTR, 1);

		if (adapter->loopback_mode == 2)
			err = dubhe1000_to_cpu_tag_parse(adapter, &to_cpu_tag2,
							 data + DUBHE1000_HEADROOM + adapter->body_offset);

		if (adapter->loopback_mode == 1) {
			length = 1500;
			input_port = 0;
		} else {
			if (to_cpu_tag2.nr_of_vlans)
				status |= DUBHE1000_RXD_STAT_VLAN;

			length = to_cpu_tag2.pakcet_length;
			input_port = to_cpu_tag2.source_port;
		}

		cleaned = true;
		cleaned_count++;

		if (unlikely((input_port != 0) && (input_port != 1))) {
			pr_info("received pkt for non-existent port(%u)\n", input_port);
			pr_info("rx body %px dma %llx %llx adapter->hw_data %px:\n", data, adapter->body_dma, dma,
				adapter->hw_data);
			for (kk = 0; kk < length; kk += 32)
				dubhe1000_dump_regs(adapter, adapter->hw_data + dma + 32 * kk);
			ew32(TX_BUF_FREE_INSTR, qstatus1 | (1 << FREE_TX_BUF_FREE_EN_BIT));
			goto dma_map;
		}

		total_rx_bytes += length;
		total_rx_packets++;

		if (netif_msg_pktdata(adapter)) {
			pr_info("port %d HW-BMU received pkt packet-len %d vlan %d rx-pkt-cnt(%d/%d) tx-pkt-cnt(%d ack %d) len %d\n",
				input_port, length, to_cpu_tag2.vlan_id, total_rx_packets, adapter->total_rx_packets,
				adapter->total_tx_packets, adapter->total_tx_ack_packets, length);

			dubhe1000_skb_dump_simplified(adapter, dma, false,
						      DUBHE1000_HEADROOM + adapter->body_offset +
							      DUBHE1000_TO_CPU_TAG_HLEN);
		}

		*((u32 *)(data + 92)) = 0x0100f1c0;
		dma_wait[total_tx] = dma;
		packet_len[total_tx] = length;
		tx_queue_status[total_tx++] = qstatus1;
		if (total_tx >= g_max_txrx)
			break;
dma_map:
		if (*work_done >= work_to_do)
			break;
		/* use prefetched values */
		qstatus = er64(TX_QUEUE_STATUS0);
		qstatus0 = qstatus & 0xFFFFFFFF;
		pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
		status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);

		tx_queue_qsize = (qstatus0 & 0xFFF);
	}

	if (dubhe1000_clean_tx_irq_loopback(adapter, total_tx) == 0) {
		cleaned = 1;
		*work_done = work_to_do;
	}

	if (netif_msg_pktdata(adapter))
		pr_info("HW BMU, total tx %d, work done %d\n", total_tx, *work_done);
	if (likely(total_tx)) {
		int j = 0;

		for (j = 0; j < total_tx; j++) {
			if (NETDEV_TX_BUSY ==
			    dubhe1000_xmit_frame_simplified(adapter, dma_wait[j], packet_len[j],
							    DUBHE1000_HEADROOM + adapter->body_offset +
								    DUBHE1000_TO_CPU_TAG_HLEN)) {
				pr_info("RX Busy, free buffer directly\n");
				ew32(TX_BUF_FREE_INSTR, tx_queue_status[j] | (1 << FREE_TX_BUF_FREE_EN_BIT));
			}
		}
	}

	// re-attach the rx-buf here
	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;
	return cleaned;
}

static bool dubhe1000_clean_rx_irq_loopback(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring,
					    int *work_done, int work_to_do)
{
	bool ret = false;

	if (adapter->soft_bmu_en)
		pr_info("Dont support loopback testing for SW BMU\n");
	else
		ret = dubhe1000_clean_hw_bmu_rx_irq_loopback(adapter, rx_ring, work_done, work_to_do);

	return ret;
}

int dubhe1000_clean_loopback(struct napi_struct *napi, int budget)
{
	struct dubhe1000_adapter *adapter = container_of(napi, struct dubhe1000_adapter, napi);
	int work_done = 0;

	ew32(RX_INTERRUPT_EN, 0);
	dubhe1000_clean_rx_irq_loopback(adapter, &adapter->rx_ring[0], &work_done, budget);

	if (work_done == budget)
		return budget;

	/* Exit the polling mode, but don't re-enable interrupts if stack might
	 * poll us due to busy-polling
	 */
	if (likely(napi_complete_done(napi, work_done))) {
		ew32(TX_INTERRUPT_EN, 1);
		ew32(RX_INTERRUPT_EN, 1);
	}

	return work_done;
}

void dubhe1000_loopback_init(struct dubhe1000_adapter *adapter, u8 enable)
{
	//replace with new napi routine
	napi_disable(&adapter->napi);
	netif_napi_del(&adapter->napi);

	if (enable)
		netif_napi_add(&adapter->napi_dev, &adapter->napi, dubhe1000_clean_loopback, 32);
	/*TODO: exit loopback mode*/

	napi_enable(&adapter->napi);
}

/* dubhe1000_loopback.c */
