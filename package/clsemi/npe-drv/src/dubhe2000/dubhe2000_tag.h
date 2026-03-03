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

#ifndef __DUBHE1000_TAG_H_
#define __DUBHE1000_TAG_H_

#include <linux/types.h>

#define DUBHE1000_TO_CPU_TAG_HLEN	24
#define DUBHE1000_FROM_CPU_TAG_HLEN	16

#define DUBHE1000_TO_CPU_ETHE_TYPE	0x9999
#if defined(__BIG_ENDIAN)
#define DUBHE1000_FROM_CPU_ETHE_TYPE	0x9998
#elif defined(__LITTLE_ENDIAN)
#define DUBHE1000_FROM_CPU_ETHE_TYPE	0x9899
#endif

#define GET_DESTWPU_STR(value)                                                                                         \
	(value == META_HOST ? "HostCPU" :                                                                              \
			      (value == META_WIFI_5G ? "WIFI 5G" : (value == META_WIFI_2_4G ? "WIFI 2.4G" : "PCIE")))

enum meta_destwpu_em {
	META_HOST,
	META_WIFI_5G,
	META_WIFI_2_4G,
	META_PCIE
};


enum TOCPU_REASON_CODE {
	/* The MAC table, L2 MC table, ACL send to port action, MPLS table, the from-CPU-TAG
	 * contained the CPU port or routing tables sent the packet to the CPU port.
	 */
	TOCPU_NORMAL = 0,
	// The packet header decoder requires more than one cell.
	TOCPU_NEED_MORE = 1,
	// This is a BPDU / RSTP frame.
	TOCPU_BPDU_RSTP = 2,
	// The Unique MAC address to the CPU was hit.
	TOCPU_UNIQUE_MACADDR = 3,
	//The Source MAC range sent the packet to the CPU..Index to rule.
	TOCPU_SMAC_RANGE_START = 4,
	TOCPU_SMAC_RANGE_END = 7,
	//The Destination MAC range sent the packet to the CPU..Index to rule.
	TOCPU_DMAC_RANGE_START = 8,
	TOCPU_DMAC_RANGE_END = 11,
	/* The source port default ACL action sent the packet to the CPU..Index to source port
	 * which sent the packet in.
	 */
	TOCPU_SPORT_DEFAUT_ACL_START = 12,
	TOCPU_SPORT_DEFAUT_ACL_END = 17,
	 /* ACL */
	//This is an L2 1588 frame.
	TOCPU_L2_1588 = 3834,
	TOCPU_L4_1588 = 3835,
	TOCPU_ARP = 3836,
	TOCPU_RARP = 3837,
	TOCPU_LLDP = 3838,
	TOCPU_802_1X_EAPOL = 3839,
	TOCPU_GRE = 3840,
	TOCPU_SCTP = 3841,
	TOCPU_LCAP = 3842,
	TOCPU_AH = 3843,
	TOCPU_ESP = 3844,
	TOCPU_DNS = 3845,
	TOCPU_BOOTP_DHCP = 3846,
	TOCPU_CAPWAP = 3847,
	TOCPU_IKE = 3848,
	TOCPU_TTL_EXPIRED = 3849,
	TOCPU_DEFAULT_ROUTE = 3850,
	TOCPU_MTU_EXCEEDED = 3851,
	TOCPU_INVALID_NEXT_HOP_TABLE = 3852,
	TOCPU_NEXT_HOP_PKT_MOD = 3853,
	TOCPU_NEXT_HOP_TABLE = 3854,
	TOCPU_ERROR_IPV4_HEADER_SIZE = 3855,
	TOCPU_MULTICAST_REDIRECTED = 3856,
	TOCPU_IPV6_ERR_ROUTING_TYPE = 3857,
	TOCPU_IPV6_SEG_ERR_ROUTING_HEADER_LEN = 3858,
	TOCPU_IPV6_SEG_WITH_TLV = 3859,
	TOCPU_MAX_MPLS_TAGS = 3860,
	TOCPU_L2_MULTI_RESERVED = 3861,
	TOCPU_TWO_TUNNEL_EXITS = 3862,
	TOCPU_FIRST_TUNNEL_EXIT_WITHOUT_SECOND = 3863,
	TOCPU_TUNNEL_EXIT_RESULT = 3864,
	TOCPU_NAT_ACTION_0 = 3865,
	TOCPU_NAT_ACTION_1 = 3866,
	TOCPU_L2_ACTION_TABLE = 3867,
	TOCPU_SNAP_LLC = 3868,
};

/*
 * ethernet_type -- To CPU ethernet type(0x9999)
 * pakcet_length -- packet length in octet, cpu tag is not incuded if exists.
 *                  Refer to figure on the right.non-minus-one encoding
 * ip_header_offset -- IP header offset in octet,  cpu tag is not incuded if exists.
 *                     Refer to figure on the right. non-minus-one encoding
 * transmit_type --
 *                  0: unicast
 *                  1: multicat
 *                  2: broadcast
 *                  3: flooding
 * vlan_type --
 *                  0: untagged
 *                  1: one tag
 *                  2: two tag
 *                  3: More than two tags
 * packet_modification_indication --
 *                  0: Outgoing packet with normal packet modifications.
 *                  1: Original incoming packet.
 * vlan_id -- outer VLAN ID
 * source_port -- Bits [2:0] contains the source port where the packet entered the switch.
 * ptp_ind -- PTP bit, if bit 0 is set to one then the packet is a PTP packet and the Timestamp field is valid.
 *
 */
struct dubhe1000_to_cpu_tag {
	u16 ethernet_type;
	u16 pakcet_length;

	u8	packet_type:6,
		ipv4_type:1,
		ipv6_type:1;
	u8 ip_header_offset;
	u8	ipv4_header_len:4,
		tcp_header_len:4;

	u8	ip_fragment_ind:1,
		transmit_type:2,
		nr_of_vlans:2,
		is_pppoe:1,
		packet_modification_ind:1,
		reserved_63:1;

	u16	vlan_id:12,
		l4type:4;

	u8 source_port;

	u16 reason_code;

	union {
		u16 raw;
		struct {
			u16	sta_index_or_flow_id:10,
				destWPU:2,
				magic_data:4;
		};
	} meta_data;

	u8	Reserved_ptp:6,
		valid_ts:1,
		ptp_ind:1;

	u8 timestamps[8];
} __packed;

/*
 * ethernet_type -- To CPU ethernet type(0x9999)
 * port_mask -- port bit mask. Bit 0 is port number 0, bit 1 is port number 1 etc.
 *                  Port 0 is located in bit 0 of byte number 0.
 * ip_header_offset -- IP header offset in octet,  cpu tag is not incuded if exists.
 *                     Refer to figure on the right. non-minus-one encoding
 * queue_ind --  Bits [2:0] specifies which egress queue the packet shall use.
 *
 * ptp_ind -- may be added other information
 *       Bit [0] will set the upd ts signal on the transmit MAC interface when the packet is transmitted.
 *       Bit [1] will set the upd cf signal on the transmit MAC interface when the packet is transmitted.
 *       Bit [2] will set the ts to sw signal on the transmit MAC interface when the packet is transmitted.
 *
 * ptp_ind -- PTP bit, if bit 0 is set to one then the packet is a PTP packet and the Timestamp field is valid.
 *
 */
struct dubhe1000_from_cpu_tag {
	u16 ethernet_type;
	u8 port_mask;
	u8	queue_ind:3,
		modified_ind:1,
		reserved:4;
	u8 signal;
	u8 timestamps[8];
	u8 unused[3];
} __packed;
int dubhe1000_to_cpu_tag_parse(struct dubhe1000_adapter *adapter, struct dubhe1000_to_cpu_tag *tag, const u8 *tag_buf);
struct sk_buff *dubhe1000_from_cpu_tag_build(struct dubhe1000_from_cpu_tag *from_cpu_tag, struct sk_buff *skb);

#endif /* __DUBHE1000_TAG_H_ */
