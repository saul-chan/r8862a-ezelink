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

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/string.h>

#include "dubhe2000.h"
#include <net/ip6_checksum.h>
#include <linux/io.h>
#include <linux/prefetch.h>
#include <linux/bitops.h>
#include <linux/if_vlan.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/phy.h>
#include <linux/mii.h>
#include <linux/ethtool.h>

#include "dubhe2000_tag.h"

#define DUBHE1000_TO_CPU_TAG_W0_ETHTYPE			GENMASK(31, 16)
#define DUBHE1000_TO_CPU_TAG_W0_PKT_EN			GENMASK(15, 0)

#define DUBHE1000_TO_CPU_TAG_W1_PKT_TYPE		GENMASK(31, 26)
#define DUBHE1000_TO_CPU_TAG_W1_PKT_IPV4_TYPE		BIT(25)
#define DUBHE1000_TO_CPU_TAG_W1_PKT_IPV6_TYPE		BIT(24)
#define DUBHE1000_TO_CPU_TAG_W1_IP_HDR_OFFSET		GENMASK(23, 16)
#define DUBHE1000_TO_CPU_TAG_W1_IPV4_HDR_LEN		GENMASK(15, 12)
#define DUBHE1000_TO_CPU_TAG_W1_TCP_HDR_LEN		GENMASK(11, 8)
#define DUBHE1000_TO_CPU_TAG_W1_IP_FRAG_IND		BIT(7)
#define DUBHE1000_TO_CPU_TAG_W1_TRANSMIT_TYPE		GENMASK(6, 5)
#define DUBHE1000_TO_CPU_TAG_W1_NR_OF_VLANS_TYPE	GENMASK(4, 3)
#define DUBHE1000_TO_CPU_TAG_W1_PPPOE_TYPE		BIT(2)
#define DUBHE1000_TO_CPU_TAG_W1_PKT_MODIFY_IND		BIT(1)

#define DUBHE1000_TO_CPU_TAG_W2_VLAN_ID			GENMASK(31, 20)
#define DUBHE1000_TO_CPU_TAG_W2_L4TYPE			GENMASK(19, 16)
#define DUBHE1000_TO_CPU_TAG_W2_SOURCE_PORT		GENMASK(15, 8)

#define DUBHE1000_TO_CPU_TAG_W2_REASON_CODE_H		GENMASK(7, 0)

#define DUBHE1000_TO_CPU_TAG_W3_REASON_CODE_L		GENMASK(31, 24)
#define DUBHE1000_TO_CPU_TAG_W3_META_DATA		GENMASK(23, 8)
#define DUBHE1000_TO_CPU_TAG_W3_PTP_IND			BIT(0)
#define DUBHE1000_TO_CPU_TAG_W3_VALID_TS		BIT(1)
#define DUBHE1000_TO_CPU_TAG_W3_RESERVED_PTP		GENMASK(7, 2)

#define DUBHE1000_FROM_CPU_TAG_W0_ETHTYPE		GENMASK(31, 16)
#define DUBHE1000_FROM_CPU_TAG_W0_PORT_MASK		GENMASK(15, 8)
#define DUBHE1000_FROM_CPU_TAG_W0_QUEUE_IND		GENMASK(7, 0)

#define DUBHE1000_FROM_CPU_TAG_W1_RESERVED		GENMASK(31, 8)
#define DUBHE1000_FROM_CPU_TAG_W1_PTP_INFO		GENMASK(7, 0)

int dubhe1000_to_cpu_tag_parse(struct dubhe1000_adapter *adapter, struct dubhe1000_to_cpu_tag *tag, const u8 *tag_buf)
{
	u8 reason_code_h, reason_code_l;
	u32 words[DUBHE1000_TO_CPU_TAG_HLEN / 4] = { 0 };
	u64 tag_mem[3];

	int k = 0;
	int kk = 0;

	if (netif_msg_pktdata(adapter)) {
		pr_info("to_cpu_tag:\n");

		pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			tag_buf[kk],      tag_buf[kk + 1],  tag_buf[kk + 2],  tag_buf[kk + 3],
			tag_buf[kk + 4],  tag_buf[kk + 5],  tag_buf[kk + 6],  tag_buf[kk + 7],
			tag_buf[kk + 8],  tag_buf[kk + 9],  tag_buf[kk + 10], tag_buf[kk + 11],
			tag_buf[kk + 12], tag_buf[kk + 13], tag_buf[kk + 14], tag_buf[kk + 15]);

		kk = 16;
		pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x\n",
			tag_buf[kk],      tag_buf[kk + 1],  tag_buf[kk + 2],  tag_buf[kk + 3],
			tag_buf[kk + 4],  tag_buf[kk + 5],  tag_buf[kk + 6],  tag_buf[kk + 7]);
	}

	/* FIXME: currenlty only read back first 128bits */
	for (k = 0; k < 3; k++)
		tag_mem[k] = ((u64 *)tag_buf)[k];

	for (k = 0; k < 3; k++) {
		words[k * 2] = ntohl(tag_mem[k] & 0xFFFFFFFF);
		words[k * 2 + 1] = ntohl(tag_mem[k] >> 32);
	}

	/* set the common parameters */
	tag->ethernet_type = FIELD_GET(DUBHE1000_TO_CPU_TAG_W0_ETHTYPE, words[0]);
	if (tag->ethernet_type != DUBHE1000_TO_CPU_ETHE_TYPE)
		return 1;

	tag->pakcet_length	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W0_PKT_EN, words[0]);
	tag->packet_type	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_PKT_TYPE, words[1]);
	tag->ipv4_type		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_PKT_IPV4_TYPE, words[1]);
	tag->ipv6_type		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_PKT_IPV6_TYPE, words[1]);
	tag->ip_header_offset	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_IP_HDR_OFFSET, words[1]);
	tag->ipv4_header_len	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_IPV4_HDR_LEN, words[1]);
	tag->tcp_header_len	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_TCP_HDR_LEN, words[1]);
	tag->ip_fragment_ind	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_IP_FRAG_IND, words[1]);
	tag->transmit_type	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_TRANSMIT_TYPE, words[1]);
	tag->nr_of_vlans	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_NR_OF_VLANS_TYPE, words[1]);
	tag->is_pppoe		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_PPPOE_TYPE, words[1]);
	tag->packet_modification_ind = FIELD_GET(DUBHE1000_TO_CPU_TAG_W1_PKT_MODIFY_IND, words[1]);

	tag->vlan_id		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W2_VLAN_ID, words[2]);
	tag->l4type		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W2_L4TYPE, words[2]);
	tag->source_port	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W2_SOURCE_PORT, words[2]);
	reason_code_h		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W2_REASON_CODE_H, words[2]);

	reason_code_l		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W3_REASON_CODE_L, words[3]);
	tag->meta_data.raw	= FIELD_GET(DUBHE1000_TO_CPU_TAG_W3_META_DATA, words[3]);
	tag->ptp_ind		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W3_PTP_IND, words[3]);
	tag->valid_ts		= FIELD_GET(DUBHE1000_TO_CPU_TAG_W3_VALID_TS, words[3]);

	*((uint64_t *)tag->timestamps) = *(((uint64_t *)&words[4]));

	tag->reason_code = (reason_code_h << 8) | reason_code_l;

	if (netif_msg_pktdata(adapter)) {
		pr_info("pkt-len=%d, pkt-type=%d, ipv4=%d", tag->pakcet_length, tag->packet_type, tag->ipv4_type);

		pr_info("ipv6=%d, hdr-offset=%d, hdr-len=%d", tag->ipv6_type, tag->ip_header_offset,
			tag->ipv4_header_len);

		pr_info("tcp-hdr-len=%d, ip_fragment_ind=%d, transmit_type=%d", tag->tcp_header_len,
			tag->ip_fragment_ind, tag->transmit_type);

		pr_info("nr_of_vlans=0x%x, is_pppoe=%d, packet_modification_ind=%d", tag->nr_of_vlans, tag->is_pppoe,
			tag->packet_modification_ind);

		pr_info("vlan-id=%d, l4type=%d, sport=%d, reason=%d", tag->vlan_id, tag->l4type, tag->source_port,
			tag->reason_code);

		pr_info("Magic_data=%#x, DestWPU=%d, flow_id/sta_index=%d", tag->meta_data.magic_data,
			tag->meta_data.destWPU, tag->meta_data.sta_index_or_flow_id);

		pr_info("ptp_ind=%d, valid_ts=%d, Timestamp=%#llx\n", tag->ptp_ind, tag->valid_ts,
			*((uint64_t *)tag->timestamps));
	}

	return 0;
}

struct sk_buff *dubhe1000_from_cpu_tag_build(struct dubhe1000_from_cpu_tag *from_cpu_tag, struct sk_buff *skb)
{
	if (skb_headroom(skb) < DUBHE1000_FROM_CPU_TAG_HLEN) {
		struct sk_buff *skb2 = skb_realloc_headroom(skb, DUBHE1000_FROM_CPU_TAG_HLEN);

		if (skb2 == NULL) {
			printk(KERN_ERR "%s: no memory\n", __func__);
			return skb;
		}

		consume_skb(skb);
		skb = skb2;
	}

	skb_push(skb, DUBHE1000_FROM_CPU_TAG_HLEN);

	memcpy(skb->data, from_cpu_tag, DUBHE1000_FROM_CPU_TAG_HLEN);

	return skb;
}
