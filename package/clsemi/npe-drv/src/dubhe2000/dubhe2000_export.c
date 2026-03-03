#include "dubhe2000.h"
#include "dubhe2000_tag.h"
#include <linux/skbuff.h>
#include "asm/cacheflush.h"

static void dubhe1000_free_status1(struct dubhe1000_adapter *adapter, u8 *tag_buf)
{
	u32 val = 0;
	int pkt_id;
	u8 buf_bid;
	u16 buf_pid;
	u8 blk_num;
	u16 packet_tid;
	u32 status1;
	dma_addr_t dma;
	int channel;

	// pkt-id
	pkt_id = ((u16 *)tag_buf)[1];
	status1 = ((u32 *)tag_buf)[1];
	channel = ((u32 *)tag_buf)[2];

	// status1
	buf_bid = (status1 & 0xFF);
	buf_pid = ((status1 >> TX_BUF_PID_BIT) & 0x1FF);
	packet_tid = ((status1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF);
	blk_num = ((status1 >> TX_BLK_NUM_BIT) & 0x3);
	dma = (buf_pid * 256 + buf_bid) * adapter->body_block_size;

	val = buf_bid;
	val += (buf_pid << FREE_TX_BUF_PID_BIT);
	val += (packet_tid << FREE_TX_BUF_TOKEN_ID_BIT);
	val += (blk_num << FREE_TX_BLK_NUM_BIT);
	val += (1 << FREE_TX_BUF_FREE_EN_BIT);

	__inval_dcache_area(adapter->hw_data + dma, 2048);
	barrier();
	ew32_x(channel, TX_BUF_FREE_INSTR, val);

	atomic_long_dec(&adapter->edma_unack_rx[channel]);

	if (netif_msg_pktdata(g_adapter))
		pr_info("free pkt_id %d bid %d pid %d blk-num %d tid %d status1 %x val %x\n",
			pkt_id, buf_bid, buf_pid, blk_num, packet_tid, status1, val);
}

int dubhe1000_is_bmu_skb(struct sk_buff *skb)
{
	if ((g_adapter == NULL) || (g_adapter->soft_bmu_en))
		return 0;

	if (((u64)skb->data >= (u64)g_adapter->hw_data) &&
	    ((u64)skb->data <= (((u64)g_adapter->hw_data) + DUBHE1000_HW_BMU_BUF_MAX(g_adapter))))
		return 1;
	else
		return 0;
}

dma_addr_t dubhe1000_cls_bmu_dma_addr(void *addr)
{
	if((g_adapter == NULL) || (g_adapter->soft_bmu_en))
		return 0;

	if(((u64)addr >= (u64)g_adapter->hw_data) &&
		((u64)addr <= (((u64)g_adapter->hw_data) + DUBHE1000_HW_BMU_BUF_MAX(g_adapter))))
		return (u64)addr - (u64)g_adapter->hw_data + (u64)g_adapter->body_dma;
	else
		return 0;
}


void *dubhe1000_get_head_data(void *body)
{
	int offset = ((u64)body - (u64)g_adapter->hw_data) / g_adapter->body_block_size;

	return (void *)(g_adapter->head_hw_data + (offset * g_adapter->head_block_size));
}

int dubhe1000_skb_free(struct sk_buff *skb)
{
	u8 *head = NULL;
	int ret = 1;

	if ((g_adapter == NULL) || (g_adapter->soft_bmu_en))
		return ret;

	if (((u64)skb->data >= (u64)g_adapter->hw_data) &&
	    ((u64)skb->data <= (((u64)g_adapter->hw_data) + DUBHE1000_HW_BMU_BUF_MAX(g_adapter)))) {
		if (unlikely(!skb->head))
			BUG();

		head = dubhe1000_get_head_data(skb->head);

		if (*((u16 *)head) != 0x9999) {
			pr_err("===>Saved data is dirty!!!");
			return 0;
		}

		dubhe1000_free_status1(g_adapter, head);

		ret = 0;
	}

	return ret;
}
