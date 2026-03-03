// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/* Copyright (c) 2020 Clourneysemi Corporation. All rights reserved */
#include <linux/bitfield.h>
#include "cls_npe_tag.h"

int cls_to_cpu_tag_parse(struct udevice *dev,
			struct cls_to_cpu_tag *tag, const u8 *tag_buf)
{
	__be32 *tag_words = (__be32 *)tag_buf;
	u8 reason_code_h, reason_code_l;

	u32 words[CLS_NPE_TO_CPU_TAG_HLEN] = {0};

	int k = 0;

	for (k = 0; k < CLS_NPE_TO_CPU_TAG_HLEN; k++)
		words[k] = ntohl(tag_words[k]);

	/* set the common parameters */
	tag->ethernet_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W0_ETHTYPE, words[0]);
	if (tag->ethernet_type != CLS_NPE_TO_CPU_ETHE_TYPE)
		return -1;

	tag->pakcet_length = FIELD_GET(CLS_NPE_TO_CPU_TAG_W0_PKT_EN,  words[0]);
	tag->packet_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_PKT_TYPE,       words[1]);
	tag->ipv4_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_PKT_IPV4_TYPE,  words[1]);
	tag->ipv6_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_PKT_IPV6_TYPE,  words[1]);
	tag->ip_header_offset = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_IP_HDR_OFFSET,  words[1]);
	tag->ipv4_header_len = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_IPV4_HDR_LEN,   words[1]);
	tag->tcp_header_len = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_TCP_HDR_LEN,    words[1]);
	tag->ip_fragment_ind = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_IP_FRAG_IND,    words[1]);
	tag->transmit_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_TRANSMIT_TYPE,  words[1]);
	tag->vlan_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_VLAN_TYPE,      words[1]);
	tag->pppoe_type = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_PPPOE_TYPE,     words[1]);
	tag->packet_modification_ind = FIELD_GET(CLS_NPE_TO_CPU_TAG_W1_PKT_MODIFY_IND, words[1]);
	tag->vlan_id = FIELD_GET(CLS_NPE_TO_CPU_TAG_W2_VLAN_ID,        words[2]);
	tag->source_port = FIELD_GET(CLS_NPE_TO_CPU_TAG_W2_SOURCE_PORT,    words[2]);
	reason_code_h = FIELD_GET(CLS_NPE_TO_CPU_TAG_W2_REASON_CODE_H,  words[2]);
	reason_code_l = FIELD_GET(CLS_NPE_TO_CPU_TAG_W3_REASON_CODE_L,  words[3]);
	tag->ptp_ind = FIELD_GET(CLS_NPE_TO_CPU_TAG_W3_PTP_IND,        words[3]);
	tag->reason_code = (reason_code_h << 8) | reason_code_l;

	return 0;
}
