#if defined(__ENABLE_FWD_TEST__)
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h> //kernel threads
#include <linux/sched.h>   //task_struct
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "dubhe2000_fwd_test.h"
#include "dubhe2000_tag.h"
#include "dubhe2000_debugfs.h"
#include "dubhe2000_switch.h"
#include "dubhe2000_edma_stats.h"
#define FWD_PARA_MAX_NUM     10
#define MAX_CPU_NUM	     4
#define MAX_HASH_NUM	     48
#define FREE_PKT_LEVEL	     2
#define CPU_HOST	     0
#define CPU_WIFI_2_4	     1
#define CPU_WIFI_5	     2
#define CPU_PCIE	     3
#define NEED_READ_RX(cpu_id) (er32_x(cpu_id, TX_INTERRUPT_RAW_RPT))
#define RING_BUF_SIZE	     512 //TX_FIFO_SIZE

static uint32_t interrupt_in_count[MAX_CPU_NUM];
static uint32_t interrupt_out_count[MAX_CPU_NUM];
static uint32_t in_deel_times[MAX_CPU_NUM];
static uint32_t out_deel_times[MAX_CPU_NUM];
static char *edma_rx_name[4] = { "EDMA_RX_LHOST", "EDMA_RX_2_4G", "EDMA_RX_5G", "EDMA_RX_PCIE" };
struct dubhe1000_from_cpu_tag g_from_cpu_tag[DUBHE1000_MAC_COUNT] = { { 0 } };
int g_from_cpuTag_bitmap = 0;
struct ring_buffer_st {
	spinlock_t write_lock; /* lock used by tx reclaim and xmit */
	spinlock_t read_lock;  /* lock used by tx reclaim and xmit */
	int write_index;
	int read_index;
	dma_addr_t buffer[RING_BUF_SIZE];
};

#define RING_GET_LEFT(r)                                                                                               \
	((r)->write_index == (r)->read_index ? RING_BUF_SIZE :                                                         \
					       (((r)->read_index - (r)->write_index + RING_BUF_SIZE) % RING_BUF_SIZE))
#define IS_RING_FULL(r)	 (RING_GET_LEFT(r) == 1)
#define IS_RING_EMPTY(r) (RING_GET_LEFT(r) == RING_BUF_SIZE)

static struct ring_buffer_st s_tx_ring[MAX_CPU_NUM] = { 0 };
static u16 s_pkt_id[MAX_CPU_NUM] = { 0 };
static uint32_t s_body_offset[MAX_CPU_NUM] = { 0 };
static unsigned int total_tx_packets[MAX_CPU_NUM];
static unsigned int total_tx_ack_packets[MAX_CPU_NUM];
static unsigned int total_rx_ack_packets[MAX_CPU_NUM];
static unsigned int total_rx_packets[MAX_CPU_NUM];

const char *route_dir_name[] = { "Host", "WIFI_40M", "WIFI_60M", "PCIE" };
static const char *cpu_name_list[MAX_CPU_NUM] = { "CPU_LHOST", "CPU_2DOT4G", "CPU_5G", "CPU_PCIE" };
bool g_is_init = false;
static int s_token_enable = 0;
static struct fwd_cmd_def_obj cmd_name_list[] = {
	{
		FWD_START,
		"fwd start",
		"fwd b",
		0,
	},
	{
		FWD_STOP,
		"fwd stop",
		"fwd e",
		0,
	},
	{
		ADD_ROUTE,
		"fwd add route",
		"fwd r a <mac>(xx:xx:xx:xx:xx:xx) <dir>(0:Host,1:WIFI_40M,2:WIFI_60M,3:PCIE)",
		4,
	},
	{
		DEL_ROUTE,
		"fwd del route",
		"fwd r d <mac>(xx:xx:xx:xx:xx:xx) <dir>(0:Host,1:WIFI_40M,2:WIFI_60M,3:PCIE)",
		4,
	},
	{
		QUERY_ROUTE,
		"fwd query route",
		"fwd r q <mac>(xx:xx:xx:xx:xx:xx)",
		3,
	},
	{
		READALL_ROUTE,
		"fwd readall route",
		"fwd r r",
		0,
	},
	{
		PUT_TOKEN,
		"fwd add token num",
		"fwd t p <cpu_id> (0:Host,1:WIFI_40M,2:WIFI_60M,3:PCIE) <id>(< 512) <num>(<0xfff)",
		5,
	},
	{
		INIT_TOKEN,
		"fwd init token",
		"fwd t i <cpu_id> (0:Host,1:WIFI_40M,2:WIFI_60M,3:PCIE) <num>(<0xfff)",
		4,
	},
	{
		FREE_TOKEN,
		"fwd sub token num",
		"fwd t f <cpu_id> (0:Host,1:WIFI_40M,2:WIFI_60M,3:PCIE) <token_id>( < 512) <num>(<0xfff)",
		5,
	},
	{
		GET_TOKEN,
		"fwd get token",
		"fwd t g <id>(< 512)",
		3,
	},
	{
		EN_TOKEN,
		"fwd enable token",
		"fwd t e <0/1>",
		3,
	},
	{
		RECV_DATA,
		"EDMA HAS RECV DATA",
		NULL,
		0,
	},
	{
		FWD_PRINT,
		"PRINT EDMA INFO",
		NULL,
		0,
	},
	{
		FREE_PKT,
		"FREE FIFO",
		NULL,
		4,
	},
};

static int ring_push(struct ring_buffer_st *ring, dma_addr_t data)
{
	if (IS_RING_FULL(ring)) {
		printk(KERN_ERR "ring buff is full w%d r%d left%d\n", ring->write_index, ring->read_index,
		       RING_GET_LEFT(ring));
		return -1;
	}

	spin_lock(&ring->write_lock);
	ring->buffer[ring->write_index] = data;
	ring->write_index = (ring->write_index + 1) % RING_BUF_SIZE;
	spin_unlock(&ring->write_lock);
	return 0;
}

static dma_addr_t ring_pull(struct ring_buffer_st *ring)
{
	dma_addr_t data = 0;

	if (IS_RING_EMPTY(ring)) {
		printk(KERN_ERR "ring buff is EMPTY\n");
		return -1;
	}

	spin_lock(&ring->read_lock);
	data = ring->buffer[ring->read_index];
	ring->read_index = (ring->read_index + 1) % RING_BUF_SIZE;
	spin_unlock(&ring->read_lock);
	return data;
}
static struct fwd_cmd_def_obj *get_fwdCmd_defObj(uint16_t type)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(cmd_name_list); i++) {
		if (cmd_name_list[i].type == type)
			return &cmd_name_list[i];
	}
	return NULL;
}

char *get_fwdCmd_name(struct fwd_cmd_def_obj *cmd_def, uint16_t type)
{
	if (!cmd_def)
		cmd_def = get_fwdCmd_defObj(type);

	if (cmd_def)
		return cmd_def->name;

	return UNKNOWN_CMD;
}

char *get_fwdCmd_help(struct fwd_cmd_def_obj *cmd_def, uint16_t type)
{
	if (!cmd_def)
		cmd_def = get_fwdCmd_defObj(type);

	if (cmd_def)
		return cmd_def->help = NULL ? "" : cmd_def->help;

	return "";
}

static void __dubhe1000_free_status1(struct dubhe1000_adapter *adapter, u8 *tag_buf)
{
	u8 ret = 0;
	int k = 0;
	u32 val = 0;
	int pkt_id;
	u8 buf_bid;
	u16 buf_pid;
	u8 blk_num;
	u16 packet_tid;
	u32 status1;

	// pkt-id
	pkt_id = (tag_buf[2] << 8) + tag_buf[3];
	status1 = (tag_buf[4] << 24) + (tag_buf[5] << 16) + (tag_buf[6] << 8) + tag_buf[7];
	// status1
	buf_bid = (status1 & 0xFF);
	buf_pid = ((status1 >> TX_BUF_PID_BIT) & 0x1FF);
	packet_tid = ((status1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF);
	blk_num = ((status1 >> TX_BLK_NUM_BIT) & 0x7);

	val = buf_bid;
	val += (buf_pid << FREE_TX_BUF_PID_BIT);
	val += (packet_tid << FREE_TX_BUF_TOKEN_ID_BIT);
	val += (blk_num << FREE_TX_BLK_NUM_BIT);
	val += (1 << FREE_TX_BUF_FREE_EN_BIT);

	printk(KERN_ERR "Please free [%#x:%d] by yourself\n", val, val);

	if (netif_msg_pktdata(g_adapter))
		pr_info("free pkt_id %d bid %d pid %d blk-num %d tokenid %d status1 %x val %x\n",
			pkt_id, buf_bid, buf_pid, blk_num, packet_tid, status1, val);
}

static int __dubhe1000_fwd_skb_free(struct sk_buff *skb)
{
	u8 *data = NULL;
	int ret = 1;
	struct dubhe1000_to_cpu_tag to_cpu_tag2;
	u32 offset = 0;
	u8 hdr[32] = { 0 };
	u64 address = 0, address2 = 0;
	u32 max_off = 0;
	int len = skb->len;

	if ((g_adapter == NULL) || (g_adapter->soft_bmu_en))
		return ret;

	max_off = DUBHE1000_HEADROOM + DUBHE1000_TO_CPU_TAG_HLEN + g_adapter->body_offset;
	if (likely(skb->head) && (likely(skb->data))) {
		if (skb->data > skb->head)
			offset = skb->data - skb->head;

		if (offset < max_off)
			return ret;

		data = skb->head;
		data += (DUBHE1000_HEADROOM + g_adapter->body_offset);

		if (data[0] == 0x99 && data[1] == 0x99) {
			__dubhe1000_free_status1(g_adapter, data);
			ret = 0;
		}
	}

	return ret;
}

static void fwd_test_print_packet_stats(void)
{
	int cpu_id;

	for (cpu_id = 0; cpu_id < MAX_CPU_NUM; cpu_id++) {
		printk(KERN_ERR
		       "[%s] ring_size:%d rx packets[%lu] tx packets[%lu] unack_tx[%u] unack_rx[%u] int_in[%u] int_out[%d] in_deel[%d], out_deel[%d]",
		       cpu_name_list[cpu_id], RING_BUF_SIZE, total_rx_packets[cpu_id], total_tx_packets[cpu_id],
		       total_tx_packets[cpu_id] - total_tx_ack_packets[cpu_id],
		       total_rx_packets[cpu_id] - total_rx_ack_packets[cpu_id],
		       interrupt_in_count[cpu_id], interrupt_out_count[cpu_id],
		       in_deel_times[cpu_id], out_deel_times[cpu_id]);
	}
}

static inline int fwd_test_free_tx_rx(struct dubhe1000_adapter *adapter, int cpu_id)
{
	u32 rx_queue_qsize = 0, rx_pkt_id = 0, rx_pkt_err = 0, cnt = 0;

	rx_queue_qsize = er32_x(cpu_id, RX_QUEUE_STATUS);
	rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
	rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
	rx_queue_qsize &= 0xFFF;
	if (netif_msg_pktdata(adapter))
		printk(KERN_ERR "--->%s,[%s] tx unack_num %d", __func__, cpu_name_list[cpu_id],
		       RING_GET_LEFT(&s_tx_ring[cpu_id]));

	if (rx_queue_qsize == 0)
		return 0;

	do {
		u32 val = 0;
		u16 buf_pid = 0, buf_bid = 0;
		dma_addr_t dma = 0;

		if (IS_RING_EMPTY(&s_tx_ring[cpu_id]))
			break;

		dma = ring_pull(&s_tx_ring[cpu_id]);
		if (netif_msg_pktdata(adapter))
			pr_info("---->%s, offset 0x%x\n", __func__, (u32)dma);

		buf_pid = dma / (adapter->body_block_size * 256);
		buf_bid = (dma - buf_pid * adapter->body_block_size * 256) / adapter->body_block_size;

		val = buf_bid;
		val += (buf_pid << FREE_TX_BUF_PID_BIT);
		val += (0 << FREE_TX_BLK_NUM_BIT);
		val += (1 << FREE_TX_BUF_FREE_EN_BIT);

		ew32_x(cpu_id, TX_BUF_FREE_INSTR, val);

		total_rx_ack_packets[cpu_id]++;

		if (netif_msg_pktdata(adapter))
			printk(KERN_ERR "---->[%s]free bid %d pid %d val 0x%x\n",
			       cpu_name_list[cpu_id], buf_bid, buf_pid, val);
		cnt++;

	} while (--rx_queue_qsize);

	if (cnt) {
		uint32_t val2 = ((1 << RX_QUEUE_DEL_EN_BIT) | cnt);

		total_tx_ack_packets[cpu_id] += cnt;
		ew32_x(cpu_id, RX_QUEUE_DEL_INSTR, val2);
	}

	return cnt;
}

static inline int fwd_test_tx_simplified(struct dubhe1000_adapter *adapter, int cpu_id, dma_addr_t dma,
					 unsigned int max_per_txd, u16 len, u32 offset1, int tag_ind)
{
	unsigned int offset = 0, size, count = 0;
	dma_addr_t data_dma;
	u16 unack_num = 0;

#if (EDMA_HW)
	s32 val = 0;
#endif

	//pkt_id = s_pkt_id[cpu_id];

	while (len) {
		size = min(len, max_per_txd);
#if (EDMA_HW)

		if (s_pkt_id[cpu_id] == 0x7FFF)
			s_pkt_id[cpu_id] = 0;

		data_dma = adapter->body_dma + dma + offset1 + offset;

		if (netif_msg_pktdata(adapter)) {
			int kk = 0;
			char *data1 = adapter->hw_data + dma + offset1 + offset;

			pr_info("%s,[%s] len %d, body dma 0x%x, offset 0x%x/%d, pkt_id %d tag_ind %d\n", __func__,
				cpu_name_list[cpu_id], len, (u32)adapter->body_dma, (u32)dma, offset1, s_pkt_id[cpu_id],
				tag_ind);

			pr_info("TX DATA:\n");
			kk = 0;
			pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				data1[kk],      data1[kk + 1],  data1[kk + 2],  data1[kk + 3],
				data1[kk + 4],  data1[kk + 5],  data1[kk + 6],  data1[kk + 7],
				data1[kk + 8],  data1[kk + 9],  data1[kk + 10], data1[kk + 11],
				data1[kk + 12], data1[kk + 13], data1[kk + 14], data1[kk + 15]);

			kk = 16;
			pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				data1[kk],      data1[kk + 1],  data1[kk + 2],  data1[kk + 3],
				data1[kk + 4],  data1[kk + 5],  data1[kk + 6],  data1[kk + 7],
				data1[kk + 8],  data1[kk + 9],  data1[kk + 10], data1[kk + 11],
				data1[kk + 12], data1[kk + 13], data1[kk + 14], data1[kk + 15]);
		}

		ring_push(&s_tx_ring[cpu_id], dma);

		val = size;				    // packet-len
		val |= ((!!tag_ind) << TAG_IND_BIT);	    // without From CPU Tag
		val |= (s_pkt_id[cpu_id] << RX_PKT_ID_BIT); // RX packet ID
		val |= (1 << RX_DESCR_CFG_EN);

		ew32_x(cpu_id, RX_DESCRIPTION0, data_dma);
		ew32_x(cpu_id, RX_DESCRIPTION1, val);
		if (netif_msg_pktdata(adapter))
			printk(KERN_ERR "Out data_dma[%#x] val[%#x]", data_dma, val);
		total_tx_packets[cpu_id]++;
#endif
		len -= size;
		offset += size;
		count++;
		s_pkt_id[cpu_id]++;
		if (len) {
			if (netif_msg_pktdata(adapter))
				pr_info("%s, more buffer for one pkt, i changed to %d\n", __func__, s_pkt_id[cpu_id]);
		}
	}

	return count;
}

static inline int fwd_test_deel_rx_packets(struct dubhe1000_adapter *adapter, uint32_t cpu_id, int free_num)
{
	uint32_t ret = 0;
	char *data;
	u32 length = 0, port = 0, rel_offset = 0;
	struct dubhe1000_to_cpu_tag to_cpu_tag2;
	u32 tx_queue_qsize = 0;
	u32 tx_queue_status1 = 0;
	u8 tx_pkt_err = 0, is_split = 0, tx_status_inv = 0;
	int pkt_id;
	int err, kk;
	u32 token_id;
	u32 token_left;
	u8 buf_bid = 0, blk_num = 0;
	u16 buf_pid = 0, packet_tid = 0;
	dma_addr_t dma, head_dma;
	int tag_ind = 0, tag_tid = 0;

	do {
		tx_queue_qsize = er32_x(cpu_id, TX_QUEUE_STATUS0);
		tx_queue_status1 = er32_x(cpu_id, TX_QUEUE_STATUS1);

		pkt_id = ((tx_queue_qsize & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((tx_queue_qsize & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((tx_queue_qsize & 0x4000) >> TX_PKT_ERR_BIT);
		tx_status_inv = ((tx_queue_qsize & 0x2000) >> TX_STATUS_INV_BIT);
		tx_queue_qsize = (tx_queue_qsize & 0xFFF);

		if (tx_queue_qsize == 0)
			return 0;

		if (tx_status_inv == 1) {
			printk(KERN_ERR "---->ERROR tx_status_inv cpu_id %d\n", cpu_id);
			continue;
		}

		token_id = (tx_queue_status1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF;
		token_left = DUBHE1000_READ_REG_ARRAY(TX_TOKEN_COUNTER, 0x4 * token_id);
		buf_bid = (tx_queue_status1 & 0xFF);
		buf_pid = ((tx_queue_status1 >> TX_BUF_PID_BIT) & 0x1FF);
		blk_num = ((tx_queue_status1 >> TX_BLK_NUM_BIT) & 0x7);
		tag_tid = ((tx_queue_status1 >> TX_TAG_QOS_TID_BIT) & 0x7);
		dma = (buf_pid * 256 + buf_bid) * adapter->body_block_size;
	} while (0);

	if (netif_msg_pktdata(adapter))
		pr_info("[FWD] %s Recv DATA{qsize %d pkt_id %d tag_tid %d split %d err %d bid %d pid %d blk-num %d tokenid %d addr %llx, free_num[%d]}\n",
			cpu_name_list[cpu_id], tx_queue_qsize, pkt_id, tag_tid, is_split, tx_pkt_err, buf_bid, buf_pid,
			blk_num, token_id, dma, free_num);
	dma_rmb(); /* read rx_buffer_info after status DD */

	data = adapter->hw_data;
	data += dma;

	prefetch(data);

	if (adapter->loopback_mode)
		ew32_x(cpu_id, TX_QUEUE_HOLD_INSTR, 1);

	if (adapter->loopback_mode == 1) {
		length = 1500;
	} else {
		err = dubhe1000_to_cpu_tag_parse(adapter, &to_cpu_tag2, data + s_body_offset[cpu_id]);
		length = to_cpu_tag2.pakcet_length;
		port = to_cpu_tag2.source_port;
		if (err == 1) {
			char *data1 = data;

			printk(KERN_ERR "!!!ERROR TO_CPU_TAG cpu[%d] offset[%d]\n", cpu_id, s_body_offset[cpu_id]);
			printk(KERN_ERR
			       "[FWD] %s Recv DATA{qsize %d pkt_id %d split %d err %d bid %d pid %d blk-num %d tid %d addr %llx}\n",
			       cpu_name_list[cpu_id], tx_queue_qsize, pkt_id, is_split, tx_pkt_err, buf_bid, buf_pid,
			       blk_num, token_id, dma, free_num);
			fwd_test_print_packet_stats();

			pr_info("!!!error Rx DATA:\n");
			for (kk = 0; kk < 128; kk += 16)
				pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					data1[kk],      data1[kk + 1],  data1[kk + 2],  data1[kk + 3],
					data1[kk + 4],  data1[kk + 5],  data1[kk + 6],  data1[kk + 7],
					data1[kk + 8],  data1[kk + 9],  data1[kk + 10], data1[kk + 11],
					data1[kk + 12], data1[kk + 13], data1[kk + 14], data1[kk + 15]);
		}
	}

	total_rx_packets[cpu_id]++;

	if (netif_msg_pktdata(adapter)) {
		char *data1 = data + DUBHE1000_TO_CPU_TAG_HLEN + s_body_offset[cpu_id];

		pr_info("Rx DATA:\n");
		kk = 0;
		pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data1[kk],      data1[kk + 1],  data1[kk + 2],  data1[kk + 3],
			data1[kk + 4],  data1[kk + 5],  data1[kk + 6],  data1[kk + 7],
			data1[kk + 8],  data1[kk + 9],  data1[kk + 10], data1[kk + 11],
			data1[kk + 12], data1[kk + 13], data1[kk + 14], data1[kk + 15]);

		kk = 16;
		pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data1[kk],      data1[kk + 1],  data1[kk + 2],  data1[kk + 3],
			data1[kk + 4],  data1[kk + 5],  data1[kk + 6],  data1[kk + 7],
			data1[kk + 8],  data1[kk + 9],  data1[kk + 10], data1[kk + 11],
			data1[kk + 12], data1[kk + 13], data1[kk + 14], data1[kk + 15]);
	}

	if (adapter->loopback_mode) {
		rel_offset = DUBHE1000_TO_CPU_TAG_HLEN + s_body_offset[cpu_id];
		*((u32 *)(data + rel_offset + 4)) = 0x0100f1c0;
		tag_ind = g_from_cpuTag_bitmap & BIT(port);
		if (tag_ind && rel_offset >= DUBHE1000_FROM_CPU_TAG_HLEN) {
			rel_offset -= DUBHE1000_FROM_CPU_TAG_HLEN;
			memcpy(data + rel_offset, &g_from_cpu_tag[port], DUBHE1000_FROM_CPU_TAG_HLEN);
		}

		fwd_test_tx_simplified(adapter, cpu_id, dma, (1 << 12),
				       length + (tag_ind ? DUBHE1000_FROM_CPU_TAG_HLEN : 0), rel_offset, tag_ind);
	} else {
		if (s_token_enable) {
			s32 val = buf_bid;

			val += (buf_pid << FREE_TX_BUF_PID_BIT);
			val += (token_id << FREE_TX_BUF_TOKEN_ID_BIT);
			val += (blk_num << FREE_TX_BLK_NUM_BIT);
			val += (1 << FREE_TX_BUF_FREE_EN_BIT);
			printk(KERN_ERR "Please free [%#x:%d] by yourself\n", val, val);
		} else {
			ew32_x(cpu_id, TX_QUEUE_DEL_INSTR, 1);
		}
	}

	return tx_queue_qsize--;
}

inline void fwd_test_enable_interrupt(int cpu_id, int enable)
{
	unsigned long flags;
	s32 val = 0;

	local_irq_save(flags);

	if (enable) {
		val = er32_x(cpu_id, TX_INTERRUPT_RAW_RPT);
		ew32_x(cpu_id, TX_INTERRUPT_RAW_RPT, val);

		val = er32_x(cpu_id, RX_INTERRUPT_RAW_RPT);
		ew32_x(cpu_id, RX_INTERRUPT_RAW_RPT, val);

		interrupt_out_count[cpu_id]++;
	} else {
		interrupt_in_count[cpu_id]++;
	}

	ew32_x(cpu_id, TX_INTERRUPT_EN, !!enable);
	if (!enable)
		ew32_x(cpu_id, RX_INTERRUPT_EN, !!enable);

	local_irq_restore(flags);
}

inline void fwd_test_enable_all_interrupt(int enable)
{
	unsigned long flags;
	int cpu_id;
	s32 val;

	local_irq_save(flags);
	for (cpu_id = 0; cpu_id < MAX_CPU_NUM; cpu_id++) {
		if (enable) {
			val = er32_x(cpu_id, TX_INTERRUPT_RAW_RPT);
			ew32_x(cpu_id, TX_INTERRUPT_RAW_RPT, val);

			val = er32_x(cpu_id, RX_INTERRUPT_RAW_RPT);
			ew32_x(cpu_id, RX_INTERRUPT_RAW_RPT, val);
		}

		ew32_x(cpu_id, TX_INTERRUPT_EN, !!enable);
		if (!enable)
			ew32_x(cpu_id, RX_INTERRUPT_EN, !!enable);
	}

	local_irq_restore(flags);
}

static int fwd_test_dowork(struct dubhe1000_adapter *adapter, int cpu_id, int *work_done, int work_to_do)
{
	uint32_t qsize = 0;
	uint32_t left = 0;

	if (adapter->loopback_mode) {
		left = RING_GET_LEFT(&s_tx_ring[cpu_id]) - 1;
		if (!left)
			goto END;
	} else
		left = work_to_do;

	if (netif_msg_pktdata(adapter))
		printk(KERN_ERR "%s cpu_id:%d ring left:%d", __func__, cpu_id, left);

	while (left--) {
		qsize = fwd_test_deel_rx_packets(adapter, cpu_id, left);

		if ((*work_done)++ >= work_to_do)
			goto END;

		if (!qsize)
			goto END;
	}

END:
	fwd_test_free_tx_rx(adapter, cpu_id);

	if (netif_msg_pktdata(adapter))
		printk(KERN_ERR " --->%s packet end, deel num [%d]", cpu_name_list[cpu_id], work_done);

	return 0;
}

int dubhe1000_deel_pkg(struct napi_struct *napi, int budget)
{
	struct dubhe1000_adapter *adapter = g_adapter;
	int cpu_id = ((u64)napi - (u64)adapter->edma_test_napi) / sizeof(struct napi_struct);
	int work_done = 0;

	in_deel_times[cpu_id]++;

	if (netif_msg_pktdata(adapter))
		printk(KERN_ERR
		       "%s cpu_id:%d budget:%d napi_addr[%llx] edma_test_napi[%llx] napi_struct:%d offset:%d, cpu:%d",
		       __func__, cpu_id, budget, napi, adapter->edma_test_napi, sizeof(struct napi_struct),
		       ((u64)napi - (u64)adapter->edma_test_napi),
		       ((u64)napi - (u64)adapter->edma_test_napi) / sizeof(struct napi_struct));

	fwd_test_dowork(adapter, cpu_id, &work_done, budget);

	if (netif_msg_pktdata(adapter))
		printk(KERN_ERR "%s cpu_id:%d budget:%d do:%d", __func__, cpu_id, budget, work_done);

	if (work_done >= budget) {
		out_deel_times[cpu_id]++;
		return budget;
	}

	if (!work_done)
		work_done = budget;

	/* Exit the polling mode, but don't re-enable interrupts if stack might
	 * poll us due to busy-polling
	 */
	if (likely(napi_complete_done(napi, work_done)))
		fwd_test_enable_interrupt(cpu_id, 1);

	out_deel_times[cpu_id]++;

	return work_done;
}

static irqreturn_t dubhe1000_test_intr(int irq, void *data)
{
	struct dubhe1000_adapter *adapter = data;
	int cpu_id = 0;

	for (; cpu_id < MAX_CPU_NUM; cpu_id++)
		if (adapter->rx_intr_list[cpu_id] == irq)
			break;

	if (netif_msg_pktdata(adapter))
		printk(KERN_ERR "%s cpu_id:%d irq:%d RX Interrupt", __func__, cpu_id, irq);

	fwd_test_enable_interrupt(cpu_id, 0);
	if (likely(napi_schedule_prep(&adapter->edma_test_napi[cpu_id]))) {
		__napi_schedule(&adapter->edma_test_napi[cpu_id]);
	} else {
		/* this really should not happen! if it does it is basically a
		 * bug, but not a hard error, so enable ints and continue
		 */
		BUG();
	}

	return IRQ_HANDLED;
}

static void fwd_test_init(struct dubhe1000_adapter *adapter, u32 cpu_id)
{
	s32 desc_fifo_size = 0;
	s32 pktid_reg_value = 0;
	dma_addr_t dma;
	u8 *data = NULL;
	s32 ret = 0;
	u32 val = 0;

	spin_lock_init(&s_tx_ring[cpu_id].read_lock);
	spin_lock_init(&s_tx_ring[cpu_id].write_lock);

	// 1: Clear Interrupt, need to Write clear the MASK Status or no?????
	val = er32_x(cpu_id, RX_INTERRUPT_RAW_RPT);
	ew32_x(cpu_id, RX_INTERRUPT_RAW_RPT, val);

	//enable cpu_id soft bmu
	if (adapter->soft_bmu_en)
		printk(KERN_ERR "!!!!! IT IS NOT SUPPORT IN SOFT_BMU MODE!!!");

	val = er32_x(cpu_id, TX_BODY_BLOCK_REG);

	s_body_offset[cpu_id] = val & GENMASK(11, 0);

	//register interrupt
	ret = request_irq(adapter->rx_intr_list[cpu_id], dubhe1000_test_intr, IRQF_SHARED, edma_rx_name[cpu_id],
			  adapter);
	if (ret) {
		dev_info(adapter->dev, "[FWD]Unable to allocate rx interrupt Error: %d rx_irq %d\n", ret,
			 adapter->rx_intr_list[cpu_id]);
		return;
	}
}

void fwd_test_cmd(struct dubhe1000_adapter *adapter, char *cmd_buf)
{
	int argc = 0;
	bool is_para_left = true;
	char *argv[FWD_PARA_MAX_NUM] = { 0 };
	char *str_ptr = cmd_buf + sizeof(FWD_CMD_STR);
	uint16_t cmd_type = 0;

	while ('\0' != *str_ptr && argc < FWD_PARA_MAX_NUM) {
		if ('\t' == *str_ptr || ' ' == *str_ptr || '\n' == *str_ptr) {
			*str_ptr = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = str_ptr;
		}
		str_ptr++;
	}

	if (argc < 1) {
		FWD_USAGE_HELP();
		return;
	}

	switch (*argv[0]) {
	case FWD_ROUTE:
	case FWD_TOKEN:
		if (argc < 2) {
			dev_info(adapter->dev, "[FWD] ARG too long!!");
			return;
		}

		cmd_type = (*argv[0] | (*argv[1] << 8));
		break;
	default:
		cmd_type = *argv[0];
		break;
	}

	struct fwd_cmd_def_obj *cmd_def = get_fwdCmd_defObj(cmd_type);

	if (!cmd_def) {
		FWD_USAGE_HELP();
		return;
	}

	dev_info(adapter->dev, "[FWD]cmd[%s]\n", get_fwdCmd_name(cmd_def, cmd_type));

	if (argc < cmd_def->min_argc) {
		dev_info(adapter->dev, "Please use command: %s\n", get_fwdCmd_help(cmd_def, cmd_type));
		return;
	}

	switch (cmd_type) {
	case FWD_START: {
		uint32_t cpu_id = 0;

		if (!g_is_init) {
			g_is_init = true;
		} else {
			dev_info(adapter->dev, "already begin!\n");
			return;
		}

		fwd_test_enable_all_interrupt(0);

		napi_disable(&adapter->napi);
		netif_napi_del(&adapter->napi);

		free_irq(adapter->rx_irq, adapter);
		free_irq(adapter->tx_irq, adapter);

		for (cpu_id = 0; cpu_id < MAX_CPU_NUM; cpu_id++) {
			dev_info(adapter->dev, "[FWD] Init EDMA for [%s]\n", cpu_name_list[cpu_id]);
			init_dummy_netdev(&adapter->emda_napi_dev[cpu_id]);
			netif_napi_add(&adapter->emda_napi_dev[cpu_id], &adapter->edma_test_napi[cpu_id],
				       dubhe1000_deel_pkg, 8);
			napi_enable(&adapter->edma_test_napi[cpu_id]);
			fwd_test_init(adapter, cpu_id);
		}

		fwd_test_enable_all_interrupt(1);
	} break;
	case FWD_STOP: {
		//g_is_init = false;
		dev_info(adapter->dev, "Stop kthread!!!\n");
	} break;
	case ADD_ROUTE: {
		uint8_t macOut[8] = { 0 };
		uint8_t data[6] = { 0 };
		uint32_t *mac_ptr = (uint32_t *)macOut;
		u32 route_type = 0;
		int k;
		s32 ret;

		if (!g_is_init) {
			dev_info(adapter->dev, "Please use command: [%s] frist\n", get_fwdCmd_help(cmd_def, FWD_START));
			return;
		}

		route_type = *argv[3] - '0';
		if (route_type > ARRAY_SIZE(route_dir_name)) {
			dev_info(adapter->dev, "Unknown cfg_route_type[%d]!", route_type);
			return;
		}

		ret = sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			     macOut + 5, macOut + 4, macOut + 3, macOut + 2, macOut + 1, macOut);
		if (ret != 6) {
			pr_warn("Invalid mac %s\n", argv[2]);
			break;
		}

		ret = dubhe1000_route_config(mac_ptr[0], mac_ptr[1], route_type, 0);

		dev_info(adapter->dev, "[FWD]: ADD Route MAC [%02x:%02x:%02x:%02x:%02x:%02x] To [%s], Ret[%d]\n",
			 macOut[5], macOut[4], macOut[3], macOut[2], macOut[1], macOut[0], route_dir_name[route_type],
			 ret);

		for (k = 0; k < 6; k++)
			data[k] = macOut[5 - k];

		dubhe1000_router_port_macAddr_table_add(adapter, data, NULL, 0);
	} break;
	case DEL_ROUTE: {
		uint8_t data[6] = { 0 };
		uint8_t macOut[8] = { 0 };
		uint32_t *mac_ptr = (uint32_t *)macOut;
		u32 route_type = 0;
		int k;
		s32 ret;

		if (!g_is_init) {
			dev_info(adapter->dev, "Please use command: [%s] frist\n", get_fwdCmd_help(cmd_def, FWD_START));
			return;
		}

		route_type = *argv[3] - '0';
		if (route_type > ARRAY_SIZE(route_dir_name)) {
			dev_info(adapter->dev, "Unknown cfg_route_type[%d]!", route_type);
			break;
		}

		ret = sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			     macOut + 5, macOut + 4, macOut + 3, macOut + 2, macOut + 1, macOut);
		if (ret != 6) {
			pr_warn("Invalid mac %s\n", argv[2]);
			break;
		}

		ret = dubhe1000_route_config(mac_ptr[0], mac_ptr[1], route_type, 1);

		dev_info(adapter->dev, "[FWD]: DEL Route MAC [%02x:%02x:%02x:%02x:%02x:%02x] To [%s],Ret[%d]\n",
			 macOut[5], macOut[4], macOut[3], macOut[2], macOut[1], macOut[0], route_dir_name[route_type],
			 ret);

		for (k = 0; k < 6; k++)
			data[k] = macOut[5 - k];

		dubhe1000_router_port_macAddr_table_del(adapter, data, NULL, 0);
	} break;
	case QUERY_ROUTE: {
		uint8_t macOut[8] = { 0 };
		uint32_t mac[2] = { 0 };
		uint8_t *mac_ptr = (uint8_t *)mac;
		int index = 0;

		ret = sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			     macOut + 5, macOut + 4, macOut + 3, macOut + 2, macOut + 1, macOut);
		if (ret != 6) {
			pr_warn("Invalid mac %s\n", argv[2]);
			break;
		}

		for (index = 0; index < 16; index++) {
			int value = DUBHE1000_READ_REG_ARRAY(TX_ROUTE_BITMAP + (debug_base_addr - conf_base_addr),
							     0x4 * index);
			printk(KERN_ERR "---->Bitmap[%d] %#x", index, value);
		}

		for (index = 0; index < 512; index++) {
			mac[0] = DUBHE1000_READ_REG_ARRAY(TX_ROUTE_CAM0 + (debug_base_addr - conf_base_addr),
							  0x8 * index);
			mac[1] = DUBHE1000_READ_REG_ARRAY(TX_ROUTE_CAM1 + (debug_base_addr - conf_base_addr),
							  0x8 * index);
			if (memcmp(macOut, mac, 6) == 0)
				break;
		}

		if (index < 512)
			dev_info(adapter->dev, "[FWD]: Found MAC [%02x:%02x:%02x:%02x:%02x:%02x] index[%d] Dir [%s]\n",
				 macOut[5], macOut[4], macOut[3], macOut[2], macOut[1], macOut[0], index,
				 route_dir_name[mac_ptr[6]]);
		else
			dev_info(adapter->dev, "[FWD]: Route MAC [%02x:%02x:%02x:%02x:%02x:%02x] Not Found]\n",
				 macOut[5], macOut[4], macOut[3], macOut[2], macOut[1], macOut[0]);

	} break;
	case READALL_ROUTE: {
		int index = 0;

		for (index = 0; index < 512; index++) {
			uint32_t mac[2] = { 0 };
			uint8_t *macOut = (uint8_t *)mac;

			mac[0] = DUBHE1000_READ_REG_ARRAY(TX_ROUTE_CAM0 + (debug_base_addr - conf_base_addr),
							  0x8 * index);
			mac[1] = DUBHE1000_READ_REG_ARRAY(TX_ROUTE_CAM1 + (debug_base_addr - conf_base_addr),
							  0x8 * index);

			dev_info(adapter->dev, "[FWD]: Route[%d] MAC [%02x:%02x:%02x:%02x:%02x:%02x] Dir [%s]\n", index,
				 macOut[5], macOut[4], macOut[3], macOut[2], macOut[1], macOut[0],
				 route_dir_name[macOut[6]]);
		}
	} break;
	case EN_TOKEN: {
		s32 val = 0;

		s_token_enable = *argv[2] - '0';
		s_token_enable = !!s_token_enable;

		if (s_token_enable) {
			if (cls_skb_free != __dubhe1000_fwd_skb_free)
				cls_skb_free = __dubhe1000_fwd_skb_free;
		} else {
			cls_skb_free = dubhe1000_skb_free;
		}

		val = er32(TX_BASE_REG0);
		val |= s_token_enable << TOKEN_FLOW_CTRL_EN_BIT;
		ew32(TX_BASE_REG0, val);

		dev_info(adapter->dev, "[FWD] token %s\n", s_token_enable ? "enable" : "disable");
	} break;
	case INIT_TOKEN: {
		int cpu_id = 0;
		int num = 0;
		s32 val = 0;
		u32 token_id;

		cpu_id = *argv[2] - '0';
		if (cpu_id > ARRAY_SIZE(cpu_name_list)) {
			printk(KERN_ERR "Unknown CPU_ID[%d]\n", cpu_id);
			break;
		}

		num = simple_strtol(argv[3], NULL, 10);
		if (num > 0xfff) {
			printk(KERN_ERR "token NUM must < %d\n", num);
			break;
		}

		for (token_id = 0; token_id < 512; token_id++) {
			val = (num << TOKEN_INIT_NUM_BIT);
			val |= (token_id << TOKEN_INIT_TID_BIT);
			val |= (1 << TOKEN_INIT_EN_BIT);
			ew32_x(cpu_id, TX_TOKEN_INIT, val);
		}
	} break;
	case FWD_PRINT:
		fwd_test_print_packet_stats();
		break;
	case PUT_TOKEN: {
		int cpu_id = 0;
		int num = 0;
		s32 val = 0;
		u32 token_id;

		cpu_id = *argv[2] - '0';
		if (cpu_id > ARRAY_SIZE(cpu_name_list)) {
			printk(KERN_ERR "Unknown CPU_ID[%d]\n", cpu_id);
			break;
		}

		token_id = simple_strtol(argv[3], NULL, 10);
		if (token_id >= 512) {
			printk(KERN_ERR "token token id must < %d\n", token_id);
			break;
		}

		num = simple_strtol(argv[4], NULL, 10);
		if (num > 0xfff) {
			printk(KERN_ERR "token num must < %d\n", num);
			break;
		}

		val = (num << TOKEN_INIT_NUM_BIT);
		val |= (token_id << TOKEN_INIT_TID_BIT);
		val |= (1 << TOKEN_INIT_EN_BIT);
		ew32_x(cpu_id, TX_TOKEN_SUPPLEMENT, val);

		val = er32_x(cpu_id, TX_TOKEN_STATUS);

		dev_info(adapter->dev, "[FWD] %s token[%d] add num=%d, ret=%d!", cpu_name_list[cpu_id], token_id, num,
			 val);
	} break;
	case GET_TOKEN: {
		int num = 0;
		s32 val = 0;
		u32 token_id;

		token_id = simple_strtol(argv[2], NULL, 10);
		if (token_id >= 512) {
			printk(KERN_ERR "token token id must < %d\n", token_id);
			break;
		}

		num = DUBHE1000_READ_REG_ARRAY(TX_TOKEN_COUNTER, 0x4 * token_id);

		dev_info(adapter->dev, "[FWD] Get token[%d] num=%d!", token_id, num);
	} break;
	case FREE_TOKEN: {
		int cpu_id = 0;
		int num = 0;
		s32 val = 0;
		u32 token_id;

		cpu_id = *argv[2] - '0';
		if (cpu_id > ARRAY_SIZE(cpu_name_list)) {
			printk(KERN_ERR "Unknown CPU_ID[%d]\n", cpu_id);
			break;
		}

		token_id = simple_strtol(argv[3], NULL, 10);
		if (token_id >= 512) {
			printk(KERN_ERR "token token id must < %d\n", token_id);
			break;
		}

		num = simple_strtol(argv[4], NULL, 10);
		if (num > 0xfff) {
			printk(KERN_ERR "token num must < %d\n", num);
			break;
		}

		val = (num << TOKEN_INIT_NUM_BIT);
		val |= (token_id << TOKEN_INIT_TID_BIT);
		val |= (1 << TOKEN_INIT_EN_BIT);
		ew32_x(cpu_id, TX_TOKEN_FREE, val);

		val = er32_x(cpu_id, TX_TOKEN_STATUS);

		dev_info(adapter->dev, "[FWD] %s token[%d] del num[%d]!", cpu_name_list[cpu_id], token_id, num, val);
	} break;
	case FREE_PKT: {
		int cpu_id = 0;
		int num = 0;
		s32 val = 0;

		cpu_id = *argv[1] - '0';
		if (cpu_id > ARRAY_SIZE(cpu_name_list)) {
			printk(KERN_ERR "Unknown CPU_ID[%d]\n", cpu_id);
			break;
		}

		val = simple_strtol(argv[2], NULL, 16);
		printk(KERN_ERR "free %#x\n", val);

		ew32_x(cpu_id, TX_BUF_FREE_INSTR, val);
	} break;
	default:
		dev_info(adapter->dev, "unknown cmd_type[%c%c]\n", cmd_type, ((char *)&cmd_type)[1]);
		return;
	}
}
#endif
