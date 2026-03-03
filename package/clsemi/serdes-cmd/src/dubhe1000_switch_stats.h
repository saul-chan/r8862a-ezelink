/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _DUBHE1000_SWITCH_STATS_H_
#define _DUBHE1000_SWITCH_STATS_H_
#include <stdint.h>
#include <stdbool.h>
#define uint uint32_t
#define u8   uint8_t
#define u16   uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define DUBHE1000_SWITCH_PORT_MAX			6

#define SWITCH_STATS_OPTION_ALL				0
#define SWITCH_STATS_OPTION_MIN				SWITCH_STATS_OPTION_INGRESS
#define SWITCH_STATS_OPTION_INGRESS			1
#define SWITCH_STATS_OPTION_IPP				2
#define SWITCH_STATS_OPTION_SHARED_BM			3
#define SWITCH_STATS_OPTION_EPP				4
#define SWITCH_STATS_OPTION_EGRESS			5
#define SWITCH_STATS_OPTION_MAX				SWITCH_STATS_OPTION_EGRESS

/******** INGRESS ********/
//Note: The "INGRESS" is per port
/**** 1. rxIf ****/
#define MAC_INTERFACE_COUNTER_FOR_TX			0x30
#define MAC_INTERFACE_COUNTER_FOR_TX_MAX		6
#define MAC_INTERFACE_COUNTER_FOR_TX_ADDR_PER_ENTRY	2

/**** 2. macBrokenPkt ****/
#define MAC_RX_BROKEN_PACKETS				0x42
#define MAC_RX_BROKEN_PACKETS_MAX			6
#define MAC_RX_BROKEN_PACKETS_ADDR_PER_ENTRY		1

/**** 3. macRxMin ****/
#define MAC_RX_SHORT_PACKET_DROP			0x48
#define MAC_RX_SHORT_PACKET_DROP_MAX			6
#define MAC_RX_SHORT_PACKET_DROP_ADDR_PER_ENTRY		1

/**** 4. macRxMax ****/
#define MAC_RX_LONG_PACKET_DROP				0x4e
#define MAC_RX_LONG_PACKET_DROP_MAX			6
#define MAC_RX_LONG_PACKET_DROP_ADDR_PER_ENTRY		1

/**** 5. spOverflow ****/
#define SP_OVERFLOW_DROP				0x11E0
#define SP_OVERFLOW_DROP_MAX				6
#define SP_OVERFLOW_DROP_ADDR_PER_ENTRY			1

/******** INGRESS Packet Processing(ipp) ********/
/**** 10. ippBrokenPkt ****/
#define IPP_BROKEN_PACKETS				0x1210

/**** 11. ippDrop ****/
#define UNKNOWN_INGRESS_DROP				0x1212
#define EMPTY_MASK_DROP					0x1213
#define INGRESS_SPANNING_TREE_DROP_LISTEN		0x1214
#define INGRESS_SPANNING_TREE_DROP_LEARNING		0x1215
#define INGRESS_SPANNING_TREE_DROP_BLOCKING		0x1216
#define L2_LOOKUP_DROP					0x1217
#define INGRESS_PACKET_FILTERING_DROP			0x1218
#define RESERVED_MAC_DA_DROP				0x1219
#define RESERVED_MAC_SA_DROP				0x121a
#define VLAN_MEMBER_DROP				0x121b
#define MINIMUM_ALLOWED_VLAN_DROP			0x121c
#define MAXIMUM_ALLOWED_VLAN_DROP			0x121d
#define INVALID_ROUTING_PROTOCOL_DROP			0x121e
#define EXPIRED_TTL_DROP				0x121f
#define L3_LOOKUP_DROP					0x1220
#define IP_CHECKSUM_DROP				0x1221
#define SECOND_TUNNEL_EXIT_DROP				0x1222
#define TUNNEL_EXIT_MISS_ACTION_DROP			0x1223
#define TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_DROP	0x1224
#define TUNNEL_EXIT_TOO_SMALL_TO_DECODE_DROP		0x1225
#define L2_RESERVED_MULTICAST_ADDRESS_DROP		0x1226
#define INGRESS_CONFIGURABLE_ACL_DROP			0x1227
#define EGRESS_CONFIGURABLE_ACL_DROP			0x1228
#define ARP_DECODER_DROP				0x1229
#define RARP_DECODER_DROP				0x122a
#define L2_IEEE_1588_DECODER_DROP			0x122b
#define L4_IEEE_1588_DECODER_DROP			0x122c
#define IEEE_802_1X_AND_EAPOL_DECODER_DROP		0x122d
#define SCTP_DECODER_DROP				0x122e
#define LACP_DECODER_DROP				0x122f
#define AH_DECODER_DROP					0x1230
#define ESP_DECODER_DROP				0x1231
#define DNS_DECODER_DROP				0x1232
#define BOOTP_AND_DHCP_DECODER_DROP			0x1233
#define CAPWAP_DECODER_DROP				0x1234
#define IKE_DECODER_DROP				0x1235
#define GRE_DECODER_DROP				0x1236
#define NAT_ACTION_TABLE_DROP				0x1237

/**** 11. smon ****/
#define SMON_SET_CONUTER_MAX			8
#define SMON_SET_CONUTER_ADDR_PER_ENTRY		1

#define SMON_SET_0_PACKET_COUNTER		0x1EC65
#define SMON_SET_1_PACKET_COUNTER		0x1EC6D
#define SMON_SET_2_PACKET_COUNTER		0x1EC75
#define SMON_SET_3_PACKET_COUNTER		0x1EC7D
#define SMON_SET_0_BYTE_COUNTER			0x1EC85
#define SMON_SET_1_BYTE_COUNTER			0x1EC8D
#define SMON_SET_2_BYTE_COUNTER			0x1EC95
#define SMON_SET_3_BYTE_COUNTER			0x1EC9D

/**** 11. ippAcl ****/
#define INGRESS_CFG_ACL_MATCH_COUNTER			0x1ECA5
#define INGRESS_CFG_ACL_MATCH_COUNTER_MAX		64
#define INGRESS_CFG_ACL_MATCH_COUNTER_ADDR_PER_ENTRY	1

/**** 11. vrfln ****/
#define RECEIVED_PACKETS_ON_INGRESS_VRF			0x1ECE5
#define RECEIVED_PACKETS_ON_INGRESS_VRF_MAX		4
#define RECEIVED_PACKETS_ON_INGRESS_VRF_ADDR_PER_ENTRY	1

/**** 11. nextHop ****/
#define	NEXT_HOP_HIT_STATUS				0x1ECE9
#define	NEXT_HOP_HIT_STATUS_MAX				1024
#define	NEXT_HOP_HIT_STATUS_ADDR_PER_ENTRY		1

/**** 11. eppAcl ****/
#define EGRESS_CFG_ACL_MATCH_COUNTER			0x1F0E9
#define EGRESS_CFG_ACL_MATCH_COUNTER_MAX		64
#define EGRESS_CFG_ACL_MATCH_COUNTER_ADDR_PER_ENTRY	1

/**** 11. ip ****/
#define IP_COUNTER_MAX					6
#define IP_COUNTER_ADDR_PER_ENTRY			1

#define IP_UNICAST_RECEIVED_COUNTER			0x1F129
#define IP_MULTICAST_RECEIVED_COUNTER			0x1F12F
#define IP_UNICAST_ROUTED_COUNTER			0x1F135
#define IP_MULTICAST_ROUTED_COUNTER			0x1F13B
#define IP_MULTICAST_ACL_DROP_COUNTER			0x1F141

/**** 11. preEppDrop ****/
#define QUEUE_OFF_DROP					0x1F147
#define QUEUE_OFF_DROP_MAX				6
#define QUEUE_OFF_DROP_ADDR_PER_ENTRY			1

#define EGRESS_SPANNING_TREE_DROP			0x1F14D
#define EGRESS_SPANNING_TREE_DROP_MAX			6
#define EGRESS_SPANNING_TREE_DROP_ADDR_PER_ENTRY	1

#define MBSC_DROP					0x1F153
#define MBSC_DROP_MAX					6
#define MBSC_DROP_ADDR_PER_ENTRY			1

#define INGRESS_EGRESS_PACKET_FILTERING_DROP			0x1F159
#define INGRESS_EGRESS_PACKET_FILTERING_DROP_MAX		6
#define INGRESS_EGRESS_PACKET_FILTERING_DROP_ADDR_PER_ENTRY	1

/**** 11. ippDebug ****/
#define DEBUG_IPP_COUNTER				0x1F15F
#define DEBUG_IPP_COUNTER_MAX				25
#define DEBUG_IPP_COUNTER_ADDR_PER_ENTRY		1

#define DEBUG_EPP_COUNTER				0x2270D
#define DEBUG_EPP_COUNTER_MAX				15
#define DEBUG_EPP_COUNTER_ADDR_PER_ENTRY		1

/**** 12. ipmOverflow ****/
#define IPP_PM_DROP					0x1211
#define IPP_PM_DROP_MAX					1
#define IPP_PM_DROP_ADDR_PER_ENTRY			1

/**** 13. ippTxPkt ****/
#define IPP_PACKET_HEAD_COUNTER				0x1238

#define IPP_PACKET_TAIL_COUNTER				0x1239

/**** 14. mmp ****/
#define INGRESS_EGRESS_ADMISSION_CONTROL_DROP			0xc9

/******** Shared Buffer Memory ********/
/**** 15. erm ****/
#define EGRESS_RESOURCE_MANAGEMENT_DROP			0x1F1B6
#define EGRESS_RESOURCE_MANAGEMENT_DROP_MAX		6
#define EGRESS_RESOURCE_MANAGEMENT_DROP_ADDR_PER_ENTRY	1

/**** 16. bmOverflow ****/
#define BUFFER_OVERFLOW_DROP				0x1F1BD

/**** 16. irm ****/
#define INGRESS_RESOURCE_MANAGER_DROP			0x1F1BE

/**** 18. pbTxPkt ****/
#define PB_PACKET_HEAD_COUNTER				0x1F510

#define PB_PACKET_TAIL_COUNTER				0x1F511

/******** Egress Packet Processing ********/
/**** 19. epppDrop ****/
#define UNKNOWN_EGRESS_DROP				0x1F519
#define UNKNOWN_EGRESS_DROP_MAX				6
#define UNKNOWN_EGRESS_DROP_ADDR_PER_ENTRY		1

#define EGRESS_PORT_DISABLED_DROP			0x1F51F
#define EGRESS_PORT_DISABLED_DROP_MAX			6
#define EGRESS_PORT_DISABLED_DROP_ADDR_PER_ENTRY	1

#define EGRESS_PORT_FILTERING_DROP			0x1F525
#define EGRESS_PORT_FILTERING_DROP_MAX			6
#define EGRESS_PORT_FILTERING_DROP_ADDR_PER_ENTRY	1

#define TUNNEL_EXIT_TOO_SMALL_MOD_TO_SMALL_DROP			0x1F52B
#define TUNNEL_EXIT_TOO_SMALL_MOD_TO_SMALL_DROP_MAX		6
#define TUNNEL_EXIT_TOO_SMALL_MOD_TO_SMALL_DROP_ADDR_PER_ENTRY	1

/**** 19. vrfOut ****/
#define TRANSMITTED_PACKETS_ON_EGRESS_VRF			0x21F09
#define TRANSMITTED_PACKETS_ON_EGRESS_VRF_MAX			4
#define TRANSMITTED_PACKETS_ON_EGRESS_VRF_ADDR_PER_ENTRY	1

/**** 19. nat ****/
#define INGRESS_NAT_HIT_STATUS			0x21F0D
#define INGRESS_NAT_HIT_STATUS_MAX		1024
#define INGRESS_NAT_HIT_STATUS_ADDR_PER_ENTRY	1

#define EGRESS_NAT_HIT_STATUS			0x2230D
#define EGRESS_NAT_HIT_STATUS_MAX		1024
#define EGRESS_NAT_HIT_STATUS_ADDR_PER_ENTRY	1

/**** 21. drain ****/
#define DRAIN_PORT_DROP				0x1F513
#define DRAIN_PORT_DROP_MAX			6
#define DRAIN_PORT_DROP_ADDR_PER_ENTRY		1

/**** 22. epmOverflow ****/
#define EPP_PM_DROP				0x1F531

/**** 24. rqOverflow ****/
#define REQUEUE_OVERFLOW_DROP			0x1F1C5

/**** 24. eppTxPkt ****/
#define EPP_PACKET_HEAD_COUNTER			0x1F532

#define EPP_PACKET_TAIL_COUNTER			0x1F533

/******** EGRESS ********/
/**** 25. psError ****/
#define PS_ERROR_COUNTER				0x22740
#define PS_ERROR_COUNTER_MAX				6
#define PS_ERROR_COUNTER_ADDR_PER_ENTRY			2

/**** 25. psTxPkt ****/
#define PS_PACKET_HEAD_COUNTER				0x2274C
#define PS_PACKET_TAIL_COUNTER				0x2274D

/**** 28. txIf ****/
#define MAC_INTERFACE_COUNTERS_FOR_TX				0x54
#define MAC_INTERFACE_COUNTERS_FOR_TX_MAX			6
#define MAC_INTERFACE_COUNTERS_FOR_TX_ADDR_PER_ENTRY		4

struct dubhe1000_swicth_txpkt_table {
	u32 head;
	u32 tail;
};

struct dubhe1000_switch_ingress_stats {
	/* interface protocol checkers */
	u32 rxIf_correct[6]; // Correct packets completed
	u32 rxIf_error[6]; // Bus protocol errors

	/* packets with last=1 and valid bytes=0 */
	u32 macBrokenPkt[6];

	/* length below 60 bytes */
	u32 macRxMin[6]; // length below 60 bytes

	/* length above "MAC RX Maximum Packet Length" */
	u32 macRxMax[6];

	/* FIFO overflow in the SP-converter */
	u32 spOverflow[6];
};

struct dubhe1000_switch_ippdrop_stats {
	/*  unknown reasons */
	u32 unkonwn_ingress;

	/* empty destination port mask. */
	u32 empty_mask;

	/* a port’s ingress spanning tree protocol state was Listening/Learning/Blocking. */
	u32 span_listen;
	u32 span_learning;
	u32 span_blocking;

	/* dropped in the L2 destination port lookup process. */
	u32 l2_lookup;
	u32 ingress_filter;
	u32 reserved_dmac;
	u32 reserved_smac;

	/* source port notbeing part of the packets VLAN membership */
	u32 vlan_member;
	/* insufficient VLAN tags */
	u32 min_vlan;
	/* too many VLAN tags */
	u32 max_vlan;

	u32 invalid_routing;
	u32 expired_ttl;
	u32 l3_lookup;
	u32 ip_checksum;
	u32 second_tunnel_exit;
	u32 tunnel_exit_miss;
	u32 tunnel_exit_small_mod;
	u32 tunnel_exit_small_decode;
	u32 l2_reserved_multi_addr;
	u32 ingress_acl;
	u32 egress_acl;
	u32 arp_decoder;
	u32 rarp_decoder;
	u32 l2_1588_decoder;
	u32 l4_1588_decoder;
	u32 dotx_eapol_decoder;
	u32 sctp_decoder;
	u32 lacp_decoder;
	u32 ah_decoder;
	u32 esp_decoder;
	u32 dns_decoder;
	u32 bootp_dhcp_decoder;
	u32 capwap_decoder;
	u32 ike_decoder;
	u32 gre_decoder;
	u32 nat_action;
};

struct dubhe1000_switch_smon_stats {
	u32 set0_packet[8];
	u32 set1_packet[8];
	u32 set2_packet[8];
	u32 set3_packet[8];
	u32 set0_byte[8];
	u32 set1_byte[8];
	u32 set2_byte[8];
	u32 set3_byte[8];
};

struct dubhe1000_switch_ip_stats { // per Port
	u32 unicast_received[6];
	u32 multi_received[6];
	u32 unicast_routed[6];
	u32 multi_routed[6];
	u32 multicast_acl_drop[6];
};

struct dubhe1000_switch_preEppDrop_stats { //per Port
	u32 queue_off_drop[6];
	u32 egress_st_drop[6];//Spaining Tree
	u32 mbsc_drop[6];
	u32 filter_drop[6];
};

struct dubhe1000_switch_ipp_stats {
	/* packets with last=1 and valid bytes=0 */
	u32 ippBrokenPkt;
	struct dubhe1000_switch_ippdrop_stats ippDrop;
	struct dubhe1000_switch_smon_stats smon;
	u32 ippAcl[64];
	u32 vrfIn[4];
	u8 nextHop[1024];
	u32 eppAcl[64];
	struct dubhe1000_switch_ip_stats ip;
	struct dubhe1000_switch_preEppDrop_stats preEppDrop;

	/* Debug IPP/EPP counter */
	u16 ippDebug[25];
	u16 eppDebug[15];

	u32 ipmOverflow;
	struct dubhe1000_swicth_txpkt_table ippTxPkt;

	u32 mmp;
};

struct dubhe1000_switch_shared_bm_stats {
	u32 erm[6];
	u32 bmOverflow;
	u32 irm;
	struct dubhe1000_swicth_txpkt_table pbTxPkt;
};

struct dubhe1000_epppDrop_cnt {
	u32 unkonwn_egress[6];
	u32 eport_disabled[6];
	u32 eport_filtering[6];
	/* The packet modification after the tunnel exit resulted in packet size < 0. */
	u32 small_to_small[6];
};

struct dubhe1000_switch_epp_stats {
	/* epppDrop */
	struct dubhe1000_epppDrop_cnt epppDrop;
	u32 vrfOut[4];
	bool ingress_nat_hit[1024];
	bool egress_nat_hit[1024];
	u32 drain[6];
	u32 epmOverflow;
	u32 rqOverflow;
	struct dubhe1000_swicth_txpkt_table eppTxPkt;
};

struct dubhe1000_switch_egress_stats {
	u32 psError_underrun[6];
	u32 psError_overflow[6];
	struct dubhe1000_swicth_txpkt_table psTxPkt;
	u32 txIf_packets[6];
	u32 txIf_error[6];
	u32 txIf_halt[6];
};

struct dubhe1000_switch_stats {
	struct dubhe1000_switch_ingress_stats ingress_stats;
	struct dubhe1000_switch_ipp_stats ipp_stats;
	struct dubhe1000_switch_shared_bm_stats shared_bm_stats;
	struct dubhe1000_switch_epp_stats epp_stats;
	struct dubhe1000_switch_egress_stats egress_stats;
};

void dubhe1000_switch_stats_dump( u8 option);
void dubhe1000_switch_clear_stats();
#endif /* _DUBHE1000_SWITCH_STATS_H_ */
