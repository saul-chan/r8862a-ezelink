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

#ifndef _DUBHE1000_SWITCH_STATS_H_
#define _DUBHE1000_SWITCH_STATS_H_

#include "dubhe2000.h"
#include "dubhe2000_switch_conf.h"

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
#define INGRESS_MAX					7 // port6 - encrypt-decrypt
/**** 1. rxIf ****/

/**** 2. macBrokenPkt ****/

/**** 3. macRxMin ****/

/**** 4. macRxMax ****/

/**** 5. spOverflow ****/

/******** INGRESS Packet Processing(ipp) ********/

/**** 11. ippDrop ****/

/**** 11. smon ****/
#define SMON_SET_CONUTER_MAX			SMON_SET_0_PACKET_COUNTER_MAX
#define SMON_SET_CONUTER_ADDR_PER_ENTRY		SMON_SET_0_PACKET_COUNTER_ADDR_PER_ENTRY

/**** 11. ippAcl ****/

/**** 11. vrfln ****/

/**** 11. nextHop ****/

/**** 11. eppAcl ****/

/**** 11. preEppDrop ****/

/**** 11. ip ****/
#define IP_COUNTER_MAX					IP_UNICAST_RECEIVED_COUNTER_MAX
#define IP_COUNTER_ADDR_PER_ENTRY			IP_UNICAST_RECEIVED_COUNTER_ADDR_PER_ENTRY

/**** 11. ippDebug ****/

/**** 12. ipmOverflow ****/

/**** 13. ippTxPkt ****/

/**** 14. mmp ****/

/******** Shared Buffer Memory ********/
/**** 15. erm ****/

/**** 16. bmOverflow ****/

/**** 16. irm ****/

/**** 18. pbTxPkt ****/

/******** Egress Packet Processing ********/
/**** 19. epppDrop ****/

/**** 19. vrfOut ****/

/**** 19. nat ****/

/**** 21. drain ****/

/**** 22. epmOverflow ****/

/**** 24. rqOverflow ****/

/**** 24. eppTxPkt ****/

/******** EGRESS ********/
/**** 25. psError ****/

/**** 25. psTxPkt ****/

/**** 28. txIf ****/

struct dubhe1000_swicth_txpkt_table {
	u32 head;
	u32 tail;
};

struct dubhe1000_switch_ingress_stats {
	/* interface protocol checkers */
	u32 rxIf_correct[MAC_INTERFACE_COUNTERS_FOR_RX_MAX]; // Correct packets completed
	u32 rxIf_error[MAC_INTERFACE_COUNTERS_FOR_RX_MAX];   // Bus protocol errors

	/* packets with last=1 and valid bytes=0 */
	u32 macBrokenPkt[MAC_RX_BROKEN_PACKETS_MAX];

	/* length below 60 bytes */
	u32 macRxMin[MAC_RX_SHORT_PACKET_DROP_MAX]; // length below 60 bytes

	/* length above "MAC RX Maximum Packet Length" */
	u32 macRxMax[MAC_RX_LONG_PACKET_DROP_MAX];

	/* FIFO overflow in the SP-converter */
	u32 spOverflow[SP_OVERFLOW_DROP_MAX];
};

struct dubhe1000_switch_ippdrop_stats {
	/*  unknown reasons */
	u32 unkonwn_ingress;

	/* empty destination port mask. */
	u32 empty_mask;

	/* a port's ingress spanning tree protocol state was Listening/Learning/Blocking. */
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
	u32 learning_packet;
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
	u32 l2_action_special;
	u32 l2_action;
	u32 l2_action_port_move;
	u32 sport_default_acl_action;
};

struct dubhe1000_switch_smon_stats {
	u32 set0_packet[SMON_SET_CONUTER_MAX];
	u32 set1_packet[SMON_SET_CONUTER_MAX];
	u32 set2_packet[SMON_SET_CONUTER_MAX];
	u32 set3_packet[SMON_SET_CONUTER_MAX];
	u32 set0_byte[SMON_SET_CONUTER_MAX];
	u32 set1_byte[SMON_SET_CONUTER_MAX];
	u32 set2_byte[SMON_SET_CONUTER_MAX];
	u32 set3_byte[SMON_SET_CONUTER_MAX];
};

struct dubhe1000_switch_ip_stats { // per Port
	u32 unicast_received[IP_COUNTER_MAX];
	u32 multi_received[IP_COUNTER_MAX];
	u32 unicast_routed[IP_COUNTER_MAX];
	u32 multi_routed[IP_COUNTER_MAX];
	u32 multicast_acl_drop[IP_COUNTER_MAX];
};

struct dubhe1000_switch_preEppDrop_stats { //per Port
	u32 queue_off_drop[QUEUE_OFF_DROP_MAX];
	u32 egress_st_drop[EGRESS_SPANNING_TREE_DROP_MAX]; //Spaining Tree
	u32 mbsc_drop[MBSC_DROP_MAX];
	u32 filter_drop[INGRESS_EGRESS_PACKET_FILTERING_DROP_MAX];
	u32 l2_action_tbl_per_port_drop[L2_ACTION_TABLE_PER_PORT_DROP_MAX];
};

struct dubhe1000_switch_ipp_stats {
	struct dubhe1000_switch_ippdrop_stats ippDrop;
	struct dubhe1000_switch_smon_stats smon;
	u32 ippAcl[INGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX];
	u32 vrfIn[RECEIVED_PACKETS_ON_INGRESS_VRF_MAX];
	u8 nextHop[NEXT_HOP_HIT_STATUS_MAX];
	u32 eppAcl[EGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX];
	struct dubhe1000_switch_preEppDrop_stats preEppDrop;
	struct dubhe1000_switch_ip_stats ip;

	/* Debug IPP/EPP counter */
	u16 ippDebug[DEBUG_IPP_COUNTER_MAX];
	u16 eppDebug[DEBUG_EPP_COUNTER_MAX];

	u32 ipmOverflow;
	struct dubhe1000_swicth_txpkt_table ippTxPkt;

	u32 mmp;
};

struct dubhe1000_switch_shared_bm_stats {
	u32 erm[EGRESS_RESOURCE_MANAGER_DROP_MAX];
	u32 bmOverflow;
	u32 irm;
	struct dubhe1000_swicth_txpkt_table pbTxPkt;
};

struct dubhe1000_epppDrop_cnt {
	u32 unkonwn_egress[UNKNOWN_EGRESS_DROP_MAX];
	u32 eport_disabled[EGRESS_PORT_DISABLED_DROP_MAX];
	u32 eport_filtering[EGRESS_PORT_FILTERING_DROP_MAX];
	/* The packet modification after the tunnel exit resulted in packet size < 0. */
	u32 small_to_small[TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_TO_SMALL_DROP_MAX];
};

struct dubhe1000_switch_epp_stats {
	/* epppDrop */
	struct dubhe1000_epppDrop_cnt epppDrop;
	u32 vrfOut[TRANSMITTED_PACKETS_ON_EGRESS_VRF_MAX];
	bool ingress_nat_hit[INGRESS_NAT_HIT_STATUS_MAX];
	bool egress_nat_hit[EGRESS_NAT_HIT_STATUS_MAX];
	u32 drain[DRAIN_PORT_DROP_MAX];
	u32 epmOverflow;
	u32 rqOverflow;
	struct dubhe1000_swicth_txpkt_table eppTxPkt;
};

struct dubhe1000_switch_egress_stats {
	u32 psError_underrun[PS_ERROR_COUNTER_MAX];
	u32 psError_overflow[PS_ERROR_COUNTER_MAX];
	struct dubhe1000_swicth_txpkt_table psTxPkt;
	u32 txIf_packets[MAC_INTERFACE_COUNTERS_FOR_TX_MAX];
	u32 txIf_error[MAC_INTERFACE_COUNTERS_FOR_TX_MAX];
	u32 txIf_halt[MAC_INTERFACE_COUNTERS_FOR_TX_MAX];
};

struct dubhe1000_switch_stats {
	struct dubhe1000_switch_ingress_stats ingress_stats;
	struct dubhe1000_switch_ipp_stats ipp_stats;
	struct dubhe1000_switch_shared_bm_stats shared_bm_stats;
	struct dubhe1000_switch_epp_stats epp_stats;
	struct dubhe1000_switch_egress_stats egress_stats;
};

void dubhe1000_switch_stats_dump(struct dubhe1000_adapter *adapter, u8 option);
void dubhe1000_switch_clear_stats(struct dubhe1000_adapter *adapter);
#endif /* _DUBHE1000_SWITCH_STATS_H_ */
