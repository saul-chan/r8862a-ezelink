/* SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0 */
/* Copyright (c) 2020 Clourneysemi Corporation. All rights reserved */

#ifndef __CLS_NPE_TAG_H_
#define __CLS_NPE_TAG_H_

#include <linux/types.h>
#include "cls_npe.h"

#define CLS_NPE_TO_CPU_TAG_HLEN		24

#define CLS_NPE_TO_CPU_ETHE_TYPE		0x9999

#define CLS_NPE_TO_CPU_TAG_W0_ETHTYPE		GENMASK(31, 16)
#define CLS_NPE_TO_CPU_TAG_W0_PKT_EN		GENMASK(15, 0)
#define CLS_NPE_TO_CPU_TAG_W1_PKT_TYPE	GENMASK(31, 26)
#define CLS_NPE_TO_CPU_TAG_W1_PKT_IPV4_TYPE	BIT(25)
#define CLS_NPE_TO_CPU_TAG_W1_PKT_IPV6_TYPE	BIT(24)
#define CLS_NPE_TO_CPU_TAG_W1_IP_HDR_OFFSET	GENMASK(23, 16)
#define CLS_NPE_TO_CPU_TAG_W1_IPV4_HDR_LEN	GENMASK(15, 12)
#define CLS_NPE_TO_CPU_TAG_W1_TCP_HDR_LEN	GENMASK(11, 8)
#define CLS_NPE_TO_CPU_TAG_W1_IP_FRAG_IND	BIT(7)
#define CLS_NPE_TO_CPU_TAG_W1_TRANSMIT_TYPE	GENMASK(6, 5)
#define CLS_NPE_TO_CPU_TAG_W1_VLAN_TYPE	GENMASK(4, 3)
#define CLS_NPE_TO_CPU_TAG_W1_PPPOE_TYPE	BIT(2)
#define CLS_NPE_TO_CPU_TAG_W1_PKT_MODIFY_IND	BIT(1)
#define CLS_NPE_TO_CPU_TAG_W1_RESERVED	BIT(0)

#define CLS_NPE_TO_CPU_TAG_W2_VLAN_ID		GENMASK(31, 20)
#define CLS_NPE_TO_CPU_TAG_W2_RESERVED	GENMASK(19, 16)
#define CLS_NPE_TO_CPU_TAG_W2_SOURCE_PORT	GENMASK(15, 8)
#define CLS_NPE_TO_CPU_TAG_W2_REASON_CODE_H	GENMASK(7, 0)

#define CLS_NPE_TO_CPU_TAG_W3_REASON_CODE_L	GENMASK(31, 24)
#define CLS_NPE_TO_CPU_TAG_W3_RESERVED	GENMASK(23, 8)
#define CLS_NPE_TO_CPU_TAG_W3_PTP_IND		GENMASK(7, 0)

/*
 * ethernet_type -- To CPU ethernet type(0x9999)
 * pakcet_length -- packet length in octet, cpu tag is not incuded if exists.
 *                  Refer to figure on the right.non-minus-one encoding
 * ip_header_offset -- IP header offset in octet,  cpu tag is not incuded if exists.
 *                     Refer to figure on the right. non-minus-one encoding
 * transmit_type --
 *                  0：unicast
 *                  1：multicat
 *                  2：broadcast
 *                  3：flooding
 * vlan_type --
 *                  0：untagged
 *                  1：one tag
 *                  2：two tag
 *                  3：More than two tags
 * packet_modification_indication --
 *                  0：Outgoing packet with normal packet modifications.
 *                  1：Original incoming packet.
 * vlan_id -- outer VLAN ID
 * source_port -- Bits [2:0] contains the source port where the packet entered the switch.
 * ptp_ind -- PTP bit, if bit 0 is set to one then the packet is a PTP packet and the Timestamp field is valid.
 *
 */
struct cls_to_cpu_tag {
	u16 ethernet_type;
	u16 pakcet_length;
	u8  packet_type:6;
	u8  ipv4_type:1;
	u8  ipv6_type:1;
	u8  ip_header_offset;
	u8  ipv4_header_len:4;
	u8  tcp_header_len:4;
	u8  ip_fragment_ind:1;
	u8  transmit_type:2;
	u8  vlan_type:2;

	u8  pppoe_type:1;
	u8  packet_modification_ind:1;
	u8  reserved0:1;
	u16 vlan_id;
	u8  source_port;
	u16 reason_code;

	u16 reserved1;
	u8  ptp_ind;
	u8  timestamps[8];
} __packed;

int cls_to_cpu_tag_parse(struct udevice *dev,
	      struct cls_to_cpu_tag *tag, const u8 *tag_buf);
#endif /* __CLS_NPE_TAG_H_ */
