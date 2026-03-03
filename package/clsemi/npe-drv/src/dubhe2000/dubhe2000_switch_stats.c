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
/* dubhe1000_switch_stats.c
 * Shared functions for accessing and configuring the Statistics of switch
 */

#include "dubhe2000_switch_stats.h"
#include "dubhe2000_switch_regs.h"

struct dubhe1000_switch_stats switch_stats;

static u64 dubhe1000_switch_get_addr(u64 based_addr, int index, int max, u8 addr_per_entry)
{
	u64 address;

	if (index >= max) {
		pr_info("[%s] index %d overstep in addr=0x%llx\n", __func__, index, based_addr);
		return 0;
	}

	address = based_addr + index * addr_per_entry;

	return address;
}

static u32 dubhe1000_switch_get_value_directly(u64 address)
{
	u32 value = 0;

	value = readl(switch_base_addr + address * 4);

	return value;
}

static u32 dubhe1000_switch_get_value(u64 based_addr, int index, int max, u8 addr_per_entry)
{
	u64 address;
	u32 value = 0;

	address = dubhe1000_switch_get_addr(based_addr, index, max, addr_per_entry);

	if (address)
		value = dubhe1000_switch_get_value_directly(address);

	return value;
}

static void dubhe1000_switch_clear_value_directly(u64 address)
{
	writel(0, switch_base_addr + address * 4);
}

static void dubhe1000_switch_clear_value(u64 based_addr, int index, int max, u8 addr_per_entry)
{
	u64 address;

	address = dubhe1000_switch_get_addr(based_addr, index, max, addr_per_entry);

	if (address)
		dubhe1000_switch_clear_value_directly(address);
}

#define dubhe1000_switch_get_addr_by_index(reg, index) \
	dubhe1000_switch_get_addr(reg, index, reg##_MAX, reg##_ADDR_PER_ENTRY)

#define dubhe1000_switch_get_value_by_index(reg, index) \
	dubhe1000_switch_get_value(reg, index, reg##_MAX, reg##_ADDR_PER_ENTRY)

#define dubhe1000_switch_clear_value_by_index(reg, index) \
	dubhe1000_switch_clear_value(reg, index, reg##_MAX, reg##_ADDR_PER_ENTRY)

static void dubhe1000_switch_update_ingress_stats(struct dubhe1000_adapter *adapter,
						  struct dubhe1000_switch_ingress_stats *ingress)
{
	u64 address;
	int i;

	/* MAC Interface Counters For RX */
	for (i = 0; i < MAC_INTERFACE_COUNTERS_FOR_RX_MAX; i++) {
		address = dubhe1000_switch_get_addr_by_index(MAC_INTERFACE_COUNTERS_FOR_RX, i);
		if (address) {
			ingress->rxIf_correct[i] = dubhe1000_switch_get_value_directly(address);
			ingress->rxIf_error[i] = dubhe1000_switch_get_value_directly(address + 1);
		}
	}

	/* MAC RX Broken Packets */
	for (i = 0; i < MAC_RX_BROKEN_PACKETS_MAX; i++)
		ingress->macBrokenPkt[i] = dubhe1000_switch_get_value_by_index(MAC_RX_BROKEN_PACKETS, i);

	/* MAC RX Short Packet Drop */
	for (i = 0; i < MAC_RX_SHORT_PACKET_DROP_MAX; i++)
		ingress->macRxMin[i] = dubhe1000_switch_get_value_by_index(MAC_RX_SHORT_PACKET_DROP, i);

	/* MAC RX Long Packet Drop */
	for (i = 0; i < MAC_RX_LONG_PACKET_DROP_MAX; i++)
		ingress->macRxMax[i] = dubhe1000_switch_get_value_by_index(MAC_RX_LONG_PACKET_DROP, i);

	/* SP Overflow Drop */
	for (i = 0; i < SP_OVERFLOW_DROP_MAX; i++)
		ingress->spOverflow[i] = dubhe1000_switch_get_value_by_index(SP_OVERFLOW_DROP, i);
}

static void dubhe1000_switch_clear_ingress_stats(struct dubhe1000_adapter *adapter)
{
	/* MAC Interface Counters For RX */
	// Read Only

	/* MAC RX Broken Packets */
	// Read Only

	/* MAC RX Short Packet Drop */
	// Read Only

	/* MAC RX Long Packet Drop */
	// Read Only

	/* SP Overflow Drop */
	// Read Only
}

static void dubhe1000_switch_dump_ingress_stats(struct dubhe1000_adapter *adapter,
						struct dubhe1000_switch_ingress_stats *ingress)
{
	pr_info("============= 1. SWITCH CORE INGRESS STATISTICS =============\n");

	/*  rxIf*/
	pr_info("RX_Correct_Protocol_Packets:     [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ingress->rxIf_correct[0], ingress->rxIf_correct[1], ingress->rxIf_correct[2],
		ingress->rxIf_correct[3], ingress->rxIf_correct[4], ingress->rxIf_correct[5]);

	pr_info("RX_Bus_Protocol_Errors:          [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ingress->rxIf_error[0], ingress->rxIf_error[1], ingress->rxIf_error[2],
		ingress->rxIf_error[3], ingress->rxIf_error[4], ingress->rxIf_error[5]);

	/* macBrokenPkt */
	pr_info("RX_Broken_Packets:               [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ingress->macBrokenPkt[0], ingress->macBrokenPkt[1], ingress->macBrokenPkt[2],
		ingress->macBrokenPkt[3], ingress->macBrokenPkt[4], ingress->macBrokenPkt[5]);

	/* macRxMin */
	pr_info("RX_Short_Packet_Drop:            [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ingress->macRxMin[0], ingress->macRxMin[1], ingress->macRxMin[2],
		ingress->macRxMin[3], ingress->macRxMin[4], ingress->macRxMin[5]);

	/* macRxMax */
	pr_info("RX_Long_Packet_Drop:             [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ingress->macRxMax[0], ingress->macRxMax[1], ingress->macRxMax[2],
		ingress->macRxMax[3], ingress->macRxMax[4], ingress->macRxMax[5]);

	/* spOverflow */
	pr_info("SP_FIFO_Overflow_Drop:           [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ingress->spOverflow[0], ingress->spOverflow[1], ingress->spOverflow[2],
		ingress->spOverflow[3], ingress->spOverflow[4], ingress->spOverflow[5]);

	pr_info("=============================================================\n");
}

static void dubhe1000_switch_update_ipp_stats(struct dubhe1000_adapter *adapter, struct dubhe1000_switch_ipp_stats *ipp)
{
	u64 address;
	int i;

	/* Unknown Ingress Drop */
	address = UNKNOWN_INGRESS_DROP;
	ipp->ippDrop.unkonwn_ingress = dubhe1000_switch_get_value_directly(address);

	/* Empty Mask Drop */
	address = EMPTY_MASK_DROP;
	ipp->ippDrop.empty_mask = dubhe1000_switch_get_value_directly(address);

	/* Ingress Spanning Tree Drop: Listen */
	address = INGRESS_SPANNING_TREE_DROP_LISTEN;
	ipp->ippDrop.span_listen = dubhe1000_switch_get_value_directly(address);

	/* Ingress Spanning Tree Drop: Learning */
	address = INGRESS_SPANNING_TREE_DROP_LEARNING;
	ipp->ippDrop.span_learning = dubhe1000_switch_get_value_directly(address);

	/* Ingress Spanning Tree Drop: Blocking */
	address = INGRESS_SPANNING_TREE_DROP_BLOCKING;
	ipp->ippDrop.span_blocking = dubhe1000_switch_get_value_directly(address);

	/* L2 Lookup Drop */
	address = L2_LOOKUP_DROP;
	ipp->ippDrop.l2_lookup = dubhe1000_switch_get_value_directly(address);

	/* Ingress Packet Filtering Drop */
	address = INGRESS_PACKET_FILTERING_DROP;
	ipp->ippDrop.ingress_filter = dubhe1000_switch_get_value_directly(address);

	/* Reserved MAC DA Drop */
	address = RESERVED_MAC_DA_DROP;
	ipp->ippDrop.reserved_dmac = dubhe1000_switch_get_value_directly(address);

	/* Reserved MAC SA Drop */
	address = RESERVED_MAC_SA_DROP;
	ipp->ippDrop.reserved_smac = dubhe1000_switch_get_value_directly(address);

	/* VLAN Member Drop */
	address = VLAN_MEMBER_DROP;
	ipp->ippDrop.vlan_member = dubhe1000_switch_get_value_directly(address);

	/* Minimum Allowed VLAN Drop */
	address = MINIMUM_ALLOWED_VLAN_DROP;
	ipp->ippDrop.min_vlan = dubhe1000_switch_get_value_directly(address);

	/* Maximum Allowed VLAN Drop */
	address = MAXIMUM_ALLOWED_VLAN_DROP;
	ipp->ippDrop.max_vlan = dubhe1000_switch_get_value_directly(address);

	/* Invalid Routing Protocol Drop */
	address = INVALID_ROUTING_PROTOCOL_DROP;
	ipp->ippDrop.invalid_routing = dubhe1000_switch_get_value_directly(address);

	/* Expired TTL Drop */
	address = EXPIRED_TTL_DROP;
	ipp->ippDrop.expired_ttl = dubhe1000_switch_get_value_directly(address);

	/* L3 Lookup Drop */
	address = L3_LOOKUP_DROP;
	ipp->ippDrop.l3_lookup = dubhe1000_switch_get_value_directly(address);

	/* IP Checksum Drop */
	address = IP_CHECKSUM_DROP;
	ipp->ippDrop.ip_checksum = dubhe1000_switch_get_value_directly(address);

	/* Second Tunnel Exit Drop */
	address = SECOND_TUNNEL_EXIT_DROP;
	ipp->ippDrop.second_tunnel_exit = dubhe1000_switch_get_value_directly(address);

	/* Tunnel Exit Miss Action Drop */
	address = TUNNEL_EXIT_MISS_ACTION_DROP;
	ipp->ippDrop.tunnel_exit_miss = dubhe1000_switch_get_value_directly(address);

	/* Tunnel Exit Too Small Packet Modification Drop */
	address = TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_DROP;
	ipp->ippDrop.tunnel_exit_small_mod = dubhe1000_switch_get_value_directly(address);

	/* Tunnel Exit Too Small to Decode Drop */
	address = TUNNEL_EXIT_TOO_SMALL_TO_DECODE_DROP;
	ipp->ippDrop.tunnel_exit_small_decode = dubhe1000_switch_get_value_directly(address);

	/* Learning Packet Drop */
	address = LEARNING_PACKET_DROP;
	ipp->ippDrop.learning_packet = dubhe1000_switch_get_value_directly(address);

	/* L2 Reserved Multicast Address Drop */
	address = L2_RESERVED_MULTICAST_ADDRESS_DROP;
	ipp->ippDrop.l2_reserved_multi_addr = dubhe1000_switch_get_value_directly(address);

	/* Ingress Configurable ACL Drop */
	address = INGRESS_CONFIGURABLE_ACL_DROP;
	ipp->ippDrop.ingress_acl = dubhe1000_switch_get_value_directly(address);

	/* Egress Configurable ACL Drop */
	address = EGRESS_CONFIGURABLE_ACL_DROP;
	ipp->ippDrop.egress_acl = dubhe1000_switch_get_value_directly(address);

	/* ARP Decoder Drop */
	address = ARP_DECODER_DROP;
	ipp->ippDrop.arp_decoder = dubhe1000_switch_get_value_directly(address);

	/* RARP Decoder Drop */
	address = RARP_DECODER_DROP;
	ipp->ippDrop.rarp_decoder = dubhe1000_switch_get_value_directly(address);

	/* L2 IEEE 1588 Decoder Drop */
	address = L2_IEEE_1588_DECODER_DROP;
	ipp->ippDrop.l2_1588_decoder = dubhe1000_switch_get_value_directly(address);

	/* L4 IEEE 1588 Decoder Drop */
	address = L4_IEEE_1588_DECODER_DROP;
	ipp->ippDrop.l4_1588_decoder = dubhe1000_switch_get_value_directly(address);

	/* IEEE 802.1X and EAPOL Decoder Drop */
	address = IEEE_802_1X_AND_EAPOL_DECODER_DROP;
	ipp->ippDrop.dotx_eapol_decoder = dubhe1000_switch_get_value_directly(address);

	/* SCTP Decoder Drop */
	address = SCTP_DECODER_DROP;
	ipp->ippDrop.sctp_decoder = dubhe1000_switch_get_value_directly(address);

	/* LACP Decoder Drop */
	address = LACP_DECODER_DROP;
	ipp->ippDrop.lacp_decoder = dubhe1000_switch_get_value_directly(address);

	/* AH Decoder Drop */
	address = AH_DECODER_DROP;
	ipp->ippDrop.ah_decoder = dubhe1000_switch_get_value_directly(address);

	/* ESP Decoder Drop */
	address = ESP_DECODER_DROP;
	ipp->ippDrop.esp_decoder = dubhe1000_switch_get_value_directly(address);

	/* DNS Decoder Drop */
	address = DNS_DECODER_DROP;
	ipp->ippDrop.dns_decoder = dubhe1000_switch_get_value_directly(address);

	/* BOOTP and DHCP Decoder Drop */
	address = BOOTP_AND_DHCP_DECODER_DROP;
	ipp->ippDrop.bootp_dhcp_decoder = dubhe1000_switch_get_value_directly(address);

	/* CAPWAP Decoder Drop */
	address = CAPWAP_DECODER_DROP;
	ipp->ippDrop.capwap_decoder = dubhe1000_switch_get_value_directly(address);

	/* IKE Decoder Drop */
	address = IKE_DECODER_DROP;
	ipp->ippDrop.ike_decoder = dubhe1000_switch_get_value_directly(address);

	/* GRE Decoder Drop */
	address = GRE_DECODER_DROP;
	ipp->ippDrop.gre_decoder = dubhe1000_switch_get_value_directly(address);

	/* NAT Action Table Drop */
	address = NAT_ACTION_TABLE_DROP;
	ipp->ippDrop.nat_action = dubhe1000_switch_get_value_directly(address);

	/* L2 Action Table Special Packet Type Drop */
	address = L2_ACTION_TABLE_SPECIAL_PACKET_TYPE_DROP;
	ipp->ippDrop.l2_action_special = dubhe1000_switch_get_value_directly(address);

	/* L2 Action Table Drop */
	address = L2_ACTION_TABLE_DROP;
	ipp->ippDrop.l2_action = dubhe1000_switch_get_value_directly(address);

	/* L2 Action Table Port Move Drop */
	address = L2_ACTION_TABLE_PORT_MOVE_DROP;
	ipp->ippDrop.l2_action_port_move = dubhe1000_switch_get_value_directly(address);

	/* Source Port Default ACL Action Drop */
	address = SOURCE_PORT_DEFAULT_ACL_ACTION_DROP;
	ipp->ippDrop.sport_default_acl_action = dubhe1000_switch_get_value_directly(address);

	/* smon */
	for (i = 0; i < SMON_SET_CONUTER_MAX; i++) {
		/* SMON Set 0/1/2/3 Packet Counter */
		address = SMON_SET_0_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set0_packet[i] = dubhe1000_switch_get_value_directly(address);

		address = SMON_SET_1_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set1_packet[i] = dubhe1000_switch_get_value_directly(address);

		address = SMON_SET_2_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set2_packet[i] = dubhe1000_switch_get_value_directly(address);

		address = SMON_SET_3_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set3_packet[i] = dubhe1000_switch_get_value_directly(address);

		/* SMON Set 0/1/2/3 Byte Counter */
		address = SMON_SET_0_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set0_byte[i] = dubhe1000_switch_get_value_directly(address);

		address = SMON_SET_1_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set1_byte[i] = dubhe1000_switch_get_value_directly(address);

		address = SMON_SET_2_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set2_byte[i] = dubhe1000_switch_get_value_directly(address);

		address = SMON_SET_3_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		ipp->smon.set3_byte[i] = dubhe1000_switch_get_value_directly(address);
	}

	/* Ingress Configurable ACL Match Counter */
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX; i++)
		ipp->ippAcl[i] = dubhe1000_switch_get_value_by_index(INGRESS_CONFIGURABLE_ACL_MATCH_COUNTER, i);

	/* Received Packets on Ingress VRF */
	for (i = 0; i < RECEIVED_PACKETS_ON_INGRESS_VRF_MAX; i++)
		ipp->vrfIn[i] = dubhe1000_switch_get_value_by_index(RECEIVED_PACKETS_ON_INGRESS_VRF, i);

	/* Next Hop Hit Status */
	for (i = 0; i < NEXT_HOP_HIT_STATUS_MAX; i++)
		ipp->nextHop[i] = dubhe1000_switch_get_value_by_index(NEXT_HOP_HIT_STATUS, i);

	/* Egress Configurable ACL Match Counter */
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX; i++)
		ipp->eppAcl[i] = dubhe1000_switch_get_value_by_index(EGRESS_CONFIGURABLE_ACL_MATCH_COUNTER, i);

	/* preEppDrop */
	/* Queue Off Drop */
	for (i = 0; i < QUEUE_OFF_DROP_MAX; i++)
		ipp->preEppDrop.queue_off_drop[i] = dubhe1000_switch_get_value_by_index(QUEUE_OFF_DROP, i);

	/* Egress Spanning Tree Drop */
	for (i = 0; i < EGRESS_SPANNING_TREE_DROP_MAX; i++)
		ipp->preEppDrop.egress_st_drop[i] = dubhe1000_switch_get_value_by_index(EGRESS_SPANNING_TREE_DROP, i);

	/* MBSC Drop */
	for (i = 0; i < MBSC_DROP_MAX; i++)
		ipp->preEppDrop.mbsc_drop[i] = dubhe1000_switch_get_value_by_index(MBSC_DROP, i);

	/* Ingress-Egress Packet Filtering Drop */
	for (i = 0; i < INGRESS_EGRESS_PACKET_FILTERING_DROP_MAX; i++)
		ipp->preEppDrop.filter_drop[i] =
			dubhe1000_switch_get_value_by_index(INGRESS_EGRESS_PACKET_FILTERING_DROP, i);

	/* L2 Action Table Per Port Drop */
	for (i = 0; i < L2_ACTION_TABLE_PER_PORT_DROP_MAX; i++)
		ipp->preEppDrop.l2_action_tbl_per_port_drop[i] =
			dubhe1000_switch_get_value_by_index(L2_ACTION_TABLE_PER_PORT_DROP, i);

	/* IP */
	for (i = 0; i < IP_COUNTER_MAX; i++) {
		/* IP Unicast Received Counter */
		address = IP_UNICAST_RECEIVED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		ipp->ip.unicast_received[i] = dubhe1000_switch_get_value_directly(address);

		/* IP Multicast Received Counter */
		address = IP_MULTICAST_RECEIVED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		ipp->ip.multi_received[i] = dubhe1000_switch_get_value_directly(address);

		/* IP Unicast Routed Counter */
		address = IP_UNICAST_ROUTED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		ipp->ip.unicast_routed[i] = dubhe1000_switch_get_value_directly(address);

		/* IP Multicast Routed Counter */
		address = IP_MULTICAST_ROUTED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		ipp->ip.multi_routed[i] = dubhe1000_switch_get_value_directly(address);

		/* IP Multicast ACL Drop Counter */
		address = IP_MULTICAST_ACL_DROP_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		ipp->ip.multicast_acl_drop[i] = dubhe1000_switch_get_value_directly(address);
	}

	/* Debug IPP Counter */
	for (i = 0; i < DEBUG_IPP_COUNTER_MAX; i++) {
		address = DEBUG_IPP_COUNTER + i * DEBUG_IPP_COUNTER_ADDR_PER_ENTRY;
		ipp->ippDebug[i] = dubhe1000_switch_get_value_directly(address);
	}

	/* Debug EPP Counter */
	for (i = 0; i < DEBUG_EPP_COUNTER_MAX; i++) {
		address = DEBUG_EPP_COUNTER + i * DEBUG_EPP_COUNTER_ADDR_PER_ENTRY;
		ipp->eppDebug[i] = dubhe1000_switch_get_value_directly(address);
	}

	/* IPP PM Drop */
	address = IPP_PM_DROP;
	ipp->ipmOverflow = dubhe1000_switch_get_value_directly(address);

	/* IPP Packet Head Counter */
	address = IPP_PACKET_HEAD_COUNTER;
	ipp->ippTxPkt.head = dubhe1000_switch_get_value_directly(address);

	/* IPP Packet Tail Counter */
	address = IPP_PACKET_TAIL_COUNTER;
	ipp->ippTxPkt.tail = dubhe1000_switch_get_value_directly(address);

	/* Ingress/Egress Admission Control Drop */
	address = INGRESS_EGRESS_ADMISSION_CONTROL_DROP;
	ipp->mmp = dubhe1000_switch_get_value_directly(address);
}

static void dubhe1000_switch_clear_ipp_stats(struct dubhe1000_adapter *adapter)
{
	u64 address;
	int i;

	/* Unknown Ingress Drop */
	address = UNKNOWN_INGRESS_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Empty Mask Drop */
	address = EMPTY_MASK_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress Spanning Tree Drop: Listen */
	address = INGRESS_SPANNING_TREE_DROP_LISTEN;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress Spanning Tree Drop: Learning */
	address = INGRESS_SPANNING_TREE_DROP_LEARNING;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress Spanning Tree Drop: Blocking */
	address = INGRESS_SPANNING_TREE_DROP_BLOCKING;
	dubhe1000_switch_clear_value_directly(address);

	/* L2 Lookup Drop */
	address = L2_LOOKUP_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress Packet Filtering Drop */
	address = INGRESS_PACKET_FILTERING_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Reserved MAC DA Drop */
	address = RESERVED_MAC_DA_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Reserved MAC SA Drop */
	address = RESERVED_MAC_SA_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* VLAN Member Drop */
	address = VLAN_MEMBER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Minimum Allowed VLAN Drop */
	address = MINIMUM_ALLOWED_VLAN_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Maximum Allowed VLAN Drop */
	address = MAXIMUM_ALLOWED_VLAN_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Invalid Routing Protocol Drop */
	address = INVALID_ROUTING_PROTOCOL_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Expired TTL Drop */
	address = EXPIRED_TTL_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L3 Lookup Drop */
	address = L3_LOOKUP_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* IP Checksum Drop */
	address = IP_CHECKSUM_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Second Tunnel Exit Drop */
	address = SECOND_TUNNEL_EXIT_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Tunnel Exit Miss Action Drop */
	address = TUNNEL_EXIT_MISS_ACTION_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Tunnel Exit Too Small Packet Modification Drop */
	address = TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Tunnel Exit Too Small to Decode Drop */
	address = TUNNEL_EXIT_TOO_SMALL_TO_DECODE_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Learning Packet Drop */
	address = LEARNING_PACKET_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L2 Reserved Multicast Address Drop */
	address = L2_RESERVED_MULTICAST_ADDRESS_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress Configurable ACL Drop */
	address = INGRESS_CONFIGURABLE_ACL_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Egress Configurable ACL Drop */
	address = EGRESS_CONFIGURABLE_ACL_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* ARP Decoder Drop */
	address = ARP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* RARP Decoder Drop */
	address = RARP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L2 IEEE 1588 Decoder Drop */
	address = L2_IEEE_1588_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L4 IEEE 1588 Decoder Drop */
	address = L4_IEEE_1588_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* IEEE 802.1X and EAPOL Decoder Drop */
	address = IEEE_802_1X_AND_EAPOL_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* SCTP Decoder Drop */
	address = SCTP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* LACP Decoder Drop */
	address = LACP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* AH Decoder Drop */
	address = AH_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* ESP Decoder Drop */
	address = ESP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* DNS Decoder Drop */
	address = DNS_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* BOOTP and DHCP Decoder Drop */
	address = BOOTP_AND_DHCP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* CAPWAP Decoder Drop */
	address = CAPWAP_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* IKE Decoder Drop */
	address = IKE_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* GRE Decoder Drop */
	address = GRE_DECODER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* NAT Action Table Drop */
	address = NAT_ACTION_TABLE_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L2 Action Table Special Packet Type Drop */
	address = L2_ACTION_TABLE_SPECIAL_PACKET_TYPE_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L2 Action Table Drop */
	address = L2_ACTION_TABLE_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* L2 Action Table Port Move Drop */
	address = L2_ACTION_TABLE_PORT_MOVE_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Source Port Default ACL Action Drop */
	address = SOURCE_PORT_DEFAULT_ACL_ACTION_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* smon */
	for (i = 0; i < SMON_SET_CONUTER_MAX; i++) {
		/* SMON Set 0/1/2/3 Packet Counter */
		address = SMON_SET_0_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		address = SMON_SET_1_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		address = SMON_SET_2_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		address = SMON_SET_3_PACKET_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		/* SMON Set 0/1/2/3 Byte Counter */
		address = SMON_SET_0_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		address = SMON_SET_1_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		address = SMON_SET_2_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		address = SMON_SET_3_BYTE_COUNTER + i * SMON_SET_CONUTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);
	}

	/* Ingress Configurable ACL Match Counter */
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX; i++)
		dubhe1000_switch_clear_value_by_index(INGRESS_CONFIGURABLE_ACL_MATCH_COUNTER, i);

	/* Received Packets on Ingress VRF */
	for (i = 0; i < RECEIVED_PACKETS_ON_INGRESS_VRF_MAX; i++)
		dubhe1000_switch_clear_value_by_index(RECEIVED_PACKETS_ON_INGRESS_VRF, i);

	/* Next Hop Hit Status */
	for (i = 0; i < NEXT_HOP_HIT_STATUS_MAX; i++)
		dubhe1000_switch_clear_value_by_index(NEXT_HOP_HIT_STATUS, i);

	/* Egress Configurable ACL Match Counter */
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX; i++)
		dubhe1000_switch_clear_value_by_index(EGRESS_CONFIGURABLE_ACL_MATCH_COUNTER, i);

	/* preEppDrop */
	/* Queue Off Drop */
	for (i = 0; i < QUEUE_OFF_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(QUEUE_OFF_DROP, i);

	/* Egress Spanning Tree Drop */
	for (i = 0; i < EGRESS_SPANNING_TREE_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(EGRESS_SPANNING_TREE_DROP, i);

	/* MBSC Drop */
	for (i = 0; i < MBSC_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(MBSC_DROP, i);

	/* Ingress-Egress Packet Filtering Drop */
	for (i = 0; i < INGRESS_EGRESS_PACKET_FILTERING_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(INGRESS_EGRESS_PACKET_FILTERING_DROP, i);

	/* L2 Action Table Per Port Drop */
	for (i = 0; i < L2_ACTION_TABLE_PER_PORT_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(L2_ACTION_TABLE_PER_PORT_DROP, i);

	/* IP */
	for (i = 0; i < IP_COUNTER_MAX; i++) {
		/* IP Unicast Received Counter */
		address = IP_UNICAST_RECEIVED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		/* IP Multicast Received Counter */
		address = IP_MULTICAST_RECEIVED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		/* IP Unicast Routed Counter */
		address = IP_UNICAST_ROUTED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		/* IP Multicast Routed Counter */
		address = IP_MULTICAST_ROUTED_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);

		/* IP Multicast ACL Drop Counter */
		address = IP_MULTICAST_ACL_DROP_COUNTER + i * IP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);
	}

	/* Debug IPP Counter */
	for (i = 0; i < DEBUG_IPP_COUNTER_MAX; i++) {
		address = DEBUG_IPP_COUNTER + i * DEBUG_IPP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);
	}

	/* Debug EPP Counter */
	for (i = 0; i < DEBUG_EPP_COUNTER_MAX; i++) {
		address = DEBUG_EPP_COUNTER + i * DEBUG_EPP_COUNTER_ADDR_PER_ENTRY;
		dubhe1000_switch_clear_value_directly(address);
	}

	/* IPP PM Drop */
	address = IPP_PM_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* IPP Packet Head Counter */
	address = IPP_PACKET_HEAD_COUNTER;
	dubhe1000_switch_clear_value_directly(address);

	/* IPP Packet Tail Counter */
	address = IPP_PACKET_TAIL_COUNTER;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress/Egress Admission Control Drop */
	address = INGRESS_EGRESS_ADMISSION_CONTROL_DROP;
	dubhe1000_switch_clear_value_directly(address);
}

static void dubhe1000_switch_dump_ipp_stats(struct dubhe1000_adapter *adapter, struct dubhe1000_switch_ipp_stats *ipp)
{
	int i, j;
	int size, index, no_empty;

	pr_info("=============== 2. SWITCH CORE IPP STATISTICS ===============\n");

	/* ipppDrop */
	pr_info("Unknown_Ingress_Drop:                  %10u\n", ipp->ippDrop.unkonwn_ingress);
	pr_info("Empty_Dest_Port_Mask_Drop:             %10u\n", ipp->ippDrop.empty_mask);
	pr_info("Ingress_Spanning_Tree_Drop_Listen:     %10u\n", ipp->ippDrop.span_listen);
	pr_info("Ingress_Spanning_Tree_Drop_Learning:   %10u\n", ipp->ippDrop.span_learning);
	pr_info("Ingress_Spanning_Tree_Drop_Blocking:   %10u\n", ipp->ippDrop.span_blocking);
	pr_info("L2_Lookup_Drop:                        %10u\n", ipp->ippDrop.l2_lookup);
	pr_info("Ingress_Packet_Filtering_Drop:         %10u\n", ipp->ippDrop.ingress_filter);
	pr_info("Reserved_MAC_DA_Drop:                  %10u\n", ipp->ippDrop.reserved_dmac);
	pr_info("Reserved_MAC_SA_Drop:                  %10u\n", ipp->ippDrop.reserved_smac);
	pr_info("VLAN_Member_Drop:                      %10u\n", ipp->ippDrop.vlan_member);
	pr_info("Minimum_Allowed_VLAN_Drop:             %10u\n", ipp->ippDrop.min_vlan);
	pr_info("Maximum_Allowed_VLAN_Drop:             %10u\n", ipp->ippDrop.max_vlan);
	pr_info("Invalid_Routing_Protocol_Drop:         %10u\n", ipp->ippDrop.invalid_routing);
	pr_info("Expired_TTL_Drop:                      %10u\n", ipp->ippDrop.expired_ttl);
	pr_info("L3_Lookup_Drop:                        %10u\n", ipp->ippDrop.l3_lookup);
	pr_info("IP_Checksum_Drop:                      %10u\n", ipp->ippDrop.ip_checksum);
	pr_info("Second_Tunnel_Exit_Drop:               %10u\n", ipp->ippDrop.second_tunnel_exit);
	pr_info("Tunnel_Exit_Miss_Action_Drop:          %10u\n", ipp->ippDrop.tunnel_exit_miss);
	pr_info("Tunnel_Exit_Too_Small_Packet_Mod_Drop: %10u\n", ipp->ippDrop.tunnel_exit_small_mod);
	pr_info("Tunnel_Exit_Too_Small_to_Decode_Drop:  %10u\n", ipp->ippDrop.tunnel_exit_small_decode);
	pr_info("Learning_Packet_Drop:                  %10u\n", ipp->ippDrop.learning_packet);
	pr_info("L2_Reserved_Multicast_Address_Drop:    %10u\n", ipp->ippDrop.l2_reserved_multi_addr);
	pr_info("Ingress_Configurable_ACL_Drop:         %10u\n", ipp->ippDrop.ingress_acl);
	pr_info("Egress_Configurable_ACL_Drop:          %10u\n", ipp->ippDrop.egress_acl);
	pr_info("ARP_Decoder_Drop:                      %10u\n", ipp->ippDrop.arp_decoder);
	pr_info("RARP_Decoder_Drop:                     %10u\n", ipp->ippDrop.rarp_decoder);
	pr_info("L2_IEEE_1588_Decoder_Drop:             %10u\n", ipp->ippDrop.l2_1588_decoder);
	pr_info("L4_IEEE_1588_Decoder_Drop:             %10u\n", ipp->ippDrop.l4_1588_decoder);
	pr_info("IEEE_802.1X_and_EAPOL_Decoder_Drop:    %10u\n", ipp->ippDrop.dotx_eapol_decoder);
	pr_info("SCTP_Decoder_Drop:                     %10u\n", ipp->ippDrop.sctp_decoder);
	pr_info("LACP_Decoder_Drop:                     %10u\n", ipp->ippDrop.lacp_decoder);
	pr_info("AH_Decoder_Drop:                       %10u\n", ipp->ippDrop.ah_decoder);
	pr_info("ESP_Decoder_Drop:                      %10u\n", ipp->ippDrop.esp_decoder);
	pr_info("DNS_Decoder_Drop:                      %10u\n", ipp->ippDrop.dns_decoder);
	pr_info("BOOTP_and_DHCP_Decoder_Drop:           %10u\n", ipp->ippDrop.bootp_dhcp_decoder);
	pr_info("CAPWAP_Decoder_Drop:                   %10u\n", ipp->ippDrop.capwap_decoder);
	pr_info("IKE_Decoder_Drop:                      %10u\n", ipp->ippDrop.ike_decoder);
	pr_info("GRE_Decoder_Drop:                      %10u\n", ipp->ippDrop.gre_decoder);
	pr_info("NAT_Action_Table_Drop:                 %10u\n", ipp->ippDrop.nat_action);
	pr_info("L2_Action_Table_Special_Pkt_Type_Drop: %10u\n", ipp->ippDrop.l2_action_special);
	pr_info("L2_Action_Table_Drop:                  %10u\n", ipp->ippDrop.l2_action);
	pr_info("L2_Action_Table_Port_Move_Drop:        %10u\n", ipp->ippDrop.l2_action_port_move);
	pr_info("Source_Port_Default_ACL_Action_Drop:   %10u\n", ipp->ippDrop.sport_default_acl_action);

	/* smon */
	pr_info("SMON_Set_0_Packet:  [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set0_packet[0], ipp->smon.set0_packet[1], ipp->smon.set0_packet[2], ipp->smon.set0_packet[3],
		ipp->smon.set0_packet[4], ipp->smon.set0_packet[5], ipp->smon.set0_packet[6], ipp->smon.set0_packet[7]);
	pr_info("SMON_Set_1_Packet:  [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set1_packet[0], ipp->smon.set1_packet[1], ipp->smon.set1_packet[2], ipp->smon.set1_packet[3],
		ipp->smon.set1_packet[4], ipp->smon.set1_packet[5], ipp->smon.set1_packet[6], ipp->smon.set1_packet[7]);
	pr_info("SMON_Set_2_Packet:  [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set2_packet[0], ipp->smon.set2_packet[1], ipp->smon.set2_packet[2], ipp->smon.set2_packet[3],
		ipp->smon.set2_packet[4], ipp->smon.set2_packet[5], ipp->smon.set2_packet[6], ipp->smon.set2_packet[7]);
	pr_info("SMON_Set_3_Packet:  [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set3_packet[0], ipp->smon.set3_packet[1], ipp->smon.set3_packet[2], ipp->smon.set3_packet[3],
		ipp->smon.set3_packet[4], ipp->smon.set3_packet[5], ipp->smon.set3_packet[6], ipp->smon.set3_packet[7]);
	pr_info("SMON_Set_0_Byte:    [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set0_byte[0], ipp->smon.set0_byte[1], ipp->smon.set0_byte[2], ipp->smon.set0_byte[3],
		ipp->smon.set0_byte[4], ipp->smon.set0_byte[5], ipp->smon.set0_byte[6], ipp->smon.set0_byte[7]);
	pr_info("SMON_Set_1_Byte:    [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set1_byte[0], ipp->smon.set1_byte[1], ipp->smon.set1_byte[2], ipp->smon.set1_byte[3],
		ipp->smon.set1_byte[4], ipp->smon.set1_byte[5], ipp->smon.set1_byte[6], ipp->smon.set1_byte[7]);
	pr_info("SMON_Set_2_Byte:    [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set2_byte[0], ipp->smon.set2_byte[1], ipp->smon.set2_byte[2], ipp->smon.set2_byte[3],
		ipp->smon.set2_byte[4], ipp->smon.set2_byte[5], ipp->smon.set2_byte[6], ipp->smon.set2_byte[7]);
	pr_info("SMON_Set_3_Byte:    [%10u, %10u, %10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->smon.set3_byte[0], ipp->smon.set3_byte[1], ipp->smon.set3_byte[2], ipp->smon.set3_byte[3],
		ipp->smon.set3_byte[4], ipp->smon.set3_byte[5], ipp->smon.set3_byte[6], ipp->smon.set3_byte[7]);

	/* ippAcl */
	size = 4;
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (ipp->ippAcl[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Ingress_ACL_Match_CNT[%d-%d]:       [%10u, %10u, %10u, %10u]\n",
				index, index + size - 1,
				ipp->ippAcl[index], ipp->ippAcl[index + 1],
				ipp->ippAcl[index + 2], ipp->ippAcl[index + 3]);
	}

	/* vrfIn */
	pr_info("Received_Packets_on_Ingress_VRF:      [%10u, %10u, %10u, %10u]\n",
		ipp->vrfIn[0], ipp->vrfIn[1], ipp->vrfIn[2], ipp->vrfIn[3]);

	/* nextHop */
	size = 16;
	for (i = 0; i < NEXT_HOP_HIT_STATUS_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (ipp->nextHop[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Next_Hop_Hit_Status[%d-%d]:     [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
				index, index + size - 1, ipp->nextHop[index], ipp->nextHop[index + 1],
				ipp->nextHop[index + 2], ipp->nextHop[index + 3], ipp->nextHop[index + 4],
				ipp->nextHop[index + 5], ipp->nextHop[index + 6], ipp->nextHop[index + 7],
				ipp->nextHop[index + 8], ipp->nextHop[index + 9], ipp->nextHop[index + 10],
				ipp->nextHop[index + 11], ipp->nextHop[index + 12], ipp->nextHop[index + 13],
				ipp->nextHop[index + 14], ipp->nextHop[index + 15]);
	}

	/* eppAcl */
	size = 4;
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_MATCH_COUNTER_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (ipp->eppAcl[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Egress_ACL_Match_CNT[%d-%d]:        [%10u, %10u, %10u, %10u]\n", index,
				index + size - 1, ipp->eppAcl[index], ipp->eppAcl[index + 1], ipp->eppAcl[index + 2],
				ipp->eppAcl[index + 3]);
	}

	/* preEppDrop */
	pr_info("Queue_Off_Drop:                       [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->preEppDrop.queue_off_drop[0], ipp->preEppDrop.queue_off_drop[1],
		ipp->preEppDrop.queue_off_drop[2], ipp->preEppDrop.queue_off_drop[3],
		ipp->preEppDrop.queue_off_drop[4], ipp->preEppDrop.queue_off_drop[5]);

	pr_info("Egress_Spanning_Tree_Drop:            [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->preEppDrop.egress_st_drop[0], ipp->preEppDrop.egress_st_drop[1],
		ipp->preEppDrop.egress_st_drop[2], ipp->preEppDrop.egress_st_drop[3],
		ipp->preEppDrop.egress_st_drop[4], ipp->preEppDrop.egress_st_drop[5]);

	pr_info("MBSC_Drop:                            [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->preEppDrop.mbsc_drop[0], ipp->preEppDrop.mbsc_drop[1], ipp->preEppDrop.mbsc_drop[2],
		ipp->preEppDrop.mbsc_drop[3], ipp->preEppDrop.mbsc_drop[4], ipp->preEppDrop.mbsc_drop[5]);

	pr_info("Ingress_Egress_Packet_Filtering_Drop: [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->preEppDrop.filter_drop[0], ipp->preEppDrop.filter_drop[1], ipp->preEppDrop.filter_drop[2],
		ipp->preEppDrop.filter_drop[3], ipp->preEppDrop.filter_drop[4], ipp->preEppDrop.filter_drop[5]);

	pr_info("L2_Action_Table_Per_Port_Drop:        [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->preEppDrop.l2_action_tbl_per_port_drop[0], ipp->preEppDrop.l2_action_tbl_per_port_drop[1],
		ipp->preEppDrop.l2_action_tbl_per_port_drop[2], ipp->preEppDrop.l2_action_tbl_per_port_drop[3],
		ipp->preEppDrop.l2_action_tbl_per_port_drop[4], ipp->preEppDrop.l2_action_tbl_per_port_drop[5]);

	/* ip */
	pr_info("IP_Unicast_Received_Counter:          [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->ip.unicast_received[0], ipp->ip.unicast_received[1], ipp->ip.unicast_received[2],
		ipp->ip.unicast_received[3], ipp->ip.unicast_received[4], ipp->ip.unicast_received[5]);

	pr_info("IP_Multicast_Received_Counter:        [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->ip.multi_received[0], ipp->ip.multi_received[1], ipp->ip.multi_received[2],
		ipp->ip.multi_received[3], ipp->ip.multi_received[4], ipp->ip.multi_received[5]);

	pr_info("IP_Unicast_Routed_Counter:            [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->ip.unicast_routed[0], ipp->ip.unicast_routed[1], ipp->ip.unicast_routed[2],
		ipp->ip.unicast_routed[3], ipp->ip.unicast_routed[4], ipp->ip.unicast_routed[5]);

	pr_info("IP_Multicast_Routed_Counter:          [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->ip.multi_routed[0], ipp->ip.multi_routed[1], ipp->ip.multi_routed[2],
		ipp->ip.multi_routed[3], ipp->ip.multi_routed[4], ipp->ip.multi_routed[5]);

	pr_info("IP_Multicast_ACL_Drop_Counter:        [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		ipp->ip.multicast_acl_drop[0], ipp->ip.multicast_acl_drop[1], ipp->ip.multicast_acl_drop[2],
		ipp->ip.multicast_acl_drop[3], ipp->ip.multicast_acl_drop[4], ipp->ip.multicast_acl_drop[5]);

	/* ippDebug */
	// 1. Debug IPP Counter
	size = 4;
	for (i = 0; i < DEBUG_IPP_COUNTER_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (ipp->ippDebug[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Debug_IPP_Counter[%d-%d]:           [%10u, %10u, %10u, %10u]\n",
				index, index + size - 1, ipp->ippDebug[index], ipp->ippDebug[index + 1],
				ipp->ippDebug[index + 2], ipp->ippDebug[index + 3]);
	}

	// 2. Debug EPP Counter
	size = 4;
	for (i = 0; i < DEBUG_EPP_COUNTER_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (ipp->eppDebug[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Debug_EPP_Counter[%d-%d]:           [%10u, %10u, %10u, %10u]\n",
				index, index + size - 1, ipp->eppDebug[index], ipp->eppDebug[index + 1],
				ipp->eppDebug[index + 2], ipp->eppDebug[index + 3]);
	}

	/* ipmOverflow */
	pr_info("IPP_PM_Drop:                           %10u\n", ipp->ipmOverflow);

	/* ippTxPkt */
	pr_info("IPP_Packet_Head_Counter:               %10u\n", ipp->ippTxPkt.head);
	pr_info("IPP_Packet_Tail_Counter:               %10u\n", ipp->ippTxPkt.tail);

	/* mmp */
	pr_info("Ingress_Egress_Admission_Ctrl_Drop:    %10u\n", ipp->mmp);

	pr_info("=============================================================\n");
}

static void dubhe1000_switch_update_shared_bm_stats(struct dubhe1000_adapter *adapter,
						    struct dubhe1000_switch_shared_bm_stats *shared_bm)
{
	u64 address;
	int i;

	/* Egress Resource Management Drop */
	for (i = 0; i < EGRESS_RESOURCE_MANAGER_DROP_MAX; i++)
		shared_bm->erm[i] = dubhe1000_switch_get_value_by_index(EGRESS_RESOURCE_MANAGER_DROP, i);

	/*  Buffer Overflow Drop */
	address = BUFFER_OVERFLOW_DROP;
	shared_bm->bmOverflow = dubhe1000_switch_get_value_directly(address);

	/* Ingress Resource Manager Drop */
	address = INGRESS_RESOURCE_MANAGER_DROP;
	shared_bm->irm = dubhe1000_switch_get_value_directly(address);

	/* PB Packet Head Counter */
	address = PB_PACKET_HEAD_COUNTER;
	shared_bm->pbTxPkt.head = dubhe1000_switch_get_value_directly(address);

	/* PB Packet Tail Counter */
	address = PB_PACKET_TAIL_COUNTER;
	shared_bm->pbTxPkt.tail = dubhe1000_switch_get_value_directly(address);
}

static void dubhe1000_switch_clear_shared_bm_stats(struct dubhe1000_adapter *adapter)
{
	u64 address;
	int i;

	/* Egress Resource Management Drop */
	for (i = 0; i < EGRESS_RESOURCE_MANAGER_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(EGRESS_RESOURCE_MANAGER_DROP, i);

	/*  Buffer Overflow Drop */
	address = BUFFER_OVERFLOW_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* Ingress Resource Manager Drop */
	address = INGRESS_RESOURCE_MANAGER_DROP;
	dubhe1000_switch_clear_value_directly(address);

	/* PB Packet Head Counter */
	address = PB_PACKET_HEAD_COUNTER;
	dubhe1000_switch_clear_value_directly(address);

	/* PB Packet Tail Counter */
	address = PB_PACKET_TAIL_COUNTER;
	dubhe1000_switch_clear_value_directly(address);
}

static void dubhe1000_switch_dump_shared_bm_stats(struct dubhe1000_adapter *adapter,
						  struct dubhe1000_switch_shared_bm_stats *shared_bm)
{
	pr_info("======= 3. SWITCH CORE Shared Buffer Memory STATISTICS ======\n");

	/* erm */
	pr_info("Egress_Resource_Management_Drop: [%10u, %10u, %10u, %10u, %10u, %10u]\n", shared_bm->erm[0],
		shared_bm->erm[1], shared_bm->erm[2], shared_bm->erm[3], shared_bm->erm[4], shared_bm->erm[5]);

	/* bmOverflow */
	pr_info("Buffer_Overflow_Drop:             %10u\n", shared_bm->bmOverflow);

	/* irm */
	pr_info("Ingress_Resource_Manager_Drop:    %10u\n", shared_bm->irm);

	/* pbTxPkt */
	pr_info("PB_Packet_Head:                   %10u\n", shared_bm->pbTxPkt.head);
	pr_info("PB_Packet_Tail:                   %10u\n", shared_bm->pbTxPkt.tail);

	pr_info("=============================================================\n");
}

static void dubhe1000_switch_update_epp_stats(struct dubhe1000_adapter *adapter, struct dubhe1000_switch_epp_stats *epp)
{
	int i;

	/* epppDrop */
	for (i = 0; i < UNKNOWN_EGRESS_DROP_MAX; i++)
		epp->epppDrop.unkonwn_egress[i] = dubhe1000_switch_get_value_by_index(UNKNOWN_EGRESS_DROP, i);

	for (i = 0; i < EGRESS_PORT_DISABLED_DROP_MAX; i++)
		epp->epppDrop.eport_disabled[i] = dubhe1000_switch_get_value_by_index(EGRESS_PORT_DISABLED_DROP, i);

	for (i = 0; i < EGRESS_PORT_FILTERING_DROP_MAX; i++)
		epp->epppDrop.eport_filtering[i] = dubhe1000_switch_get_value_by_index(EGRESS_PORT_FILTERING_DROP, i);

	for (i = 0; i < TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_TO_SMALL_DROP_MAX; i++)
		epp->epppDrop.small_to_small[i] =
			dubhe1000_switch_get_value_by_index(TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_TO_SMALL_DROP, i);

	/* vrfOut */
	for (i = 0; i < TRANSMITTED_PACKETS_ON_EGRESS_VRF_MAX; i++)
		epp->vrfOut[i] = dubhe1000_switch_get_value_by_index(TRANSMITTED_PACKETS_ON_EGRESS_VRF, i);

	/* nat */
	for (i = 0; i < INGRESS_NAT_HIT_STATUS_MAX; i++)
		epp->ingress_nat_hit[i] = !!dubhe1000_switch_get_value_by_index(INGRESS_NAT_HIT_STATUS, i);

	for (i = 0; i < EGRESS_NAT_HIT_STATUS_MAX; i++)
		epp->egress_nat_hit[i] = !!dubhe1000_switch_get_value_by_index(EGRESS_NAT_HIT_STATUS, i);

	/* drain */
	for (i = 0; i < DRAIN_PORT_DROP_MAX; i++)
		epp->drain[i] = dubhe1000_switch_get_value_by_index(DRAIN_PORT_DROP, i);

	/* epmOverflow */
	epp->epmOverflow = dubhe1000_switch_get_value_directly(EPP_PM_DROP);

	/* rqOverflow */
	epp->rqOverflow = dubhe1000_switch_get_value_directly(RE_QUEUE_OVERFLOW_DROP);

	/* eppTxPkt */
	epp->eppTxPkt.head = dubhe1000_switch_get_value_directly(EPP_PACKET_HEAD_COUNTER);

	epp->eppTxPkt.tail = dubhe1000_switch_get_value_directly(EPP_PACKET_TAIL_COUNTER);
}

static void dubhe1000_switch_clear_epp_stats(struct dubhe1000_adapter *adapter)
{
	int i;

	/* epppDrop */
	for (i = 0; i < UNKNOWN_EGRESS_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(UNKNOWN_EGRESS_DROP, i);

	for (i = 0; i < EGRESS_PORT_DISABLED_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(EGRESS_PORT_DISABLED_DROP, i);

	for (i = 0; i < EGRESS_PORT_FILTERING_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(EGRESS_PORT_FILTERING_DROP, i);

	for (i = 0; i < TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_TO_SMALL_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(TUNNEL_EXIT_TOO_SMALL_PACKET_MODIFICATION_TO_SMALL_DROP, i);

	/* vrfOut */
	for (i = 0; i < TRANSMITTED_PACKETS_ON_EGRESS_VRF_MAX; i++)
		dubhe1000_switch_clear_value_by_index(TRANSMITTED_PACKETS_ON_EGRESS_VRF, i);

	/* nat */
	for (i = 0; i < INGRESS_NAT_HIT_STATUS_MAX; i++)
		dubhe1000_switch_clear_value_by_index(INGRESS_NAT_HIT_STATUS, i);

	for (i = 0; i < EGRESS_NAT_HIT_STATUS_MAX; i++)
		dubhe1000_switch_clear_value_by_index(EGRESS_NAT_HIT_STATUS, i);

	/* drain */
	for (i = 0; i < DRAIN_PORT_DROP_MAX; i++)
		dubhe1000_switch_clear_value_by_index(DRAIN_PORT_DROP, i);

	/* epmOverflow */
	dubhe1000_switch_clear_value_directly(EPP_PM_DROP);

	/* rqOverflow */
	dubhe1000_switch_clear_value_directly(RE_QUEUE_OVERFLOW_DROP);

	/* eppTxPkt */
	dubhe1000_switch_clear_value_directly(EPP_PACKET_HEAD_COUNTER);

	dubhe1000_switch_clear_value_directly(EPP_PACKET_TAIL_COUNTER);
}

static void dubhe1000_switch_dump_epp_stats(struct dubhe1000_adapter *adapter, struct dubhe1000_switch_epp_stats *epp)
{
	int i, j;
	int size, index, no_empty;

	pr_info("=============== 4. SWITCH CORE EPP STATISTICS ===============\n");

	pr_info("Unknown_Egress_Drop:             [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		epp->epppDrop.unkonwn_egress[0], epp->epppDrop.unkonwn_egress[1], epp->epppDrop.unkonwn_egress[2],
		epp->epppDrop.unkonwn_egress[3], epp->epppDrop.unkonwn_egress[4], epp->epppDrop.unkonwn_egress[5]);

	pr_info("Egress_Port_Disabled_Drop:       [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		epp->epppDrop.eport_disabled[0], epp->epppDrop.eport_disabled[1], epp->epppDrop.eport_disabled[2],
		epp->epppDrop.eport_disabled[3], epp->epppDrop.eport_disabled[4], epp->epppDrop.eport_disabled[5]);

	pr_info("Egress_Port_Filtering_Drop:      [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		epp->epppDrop.eport_filtering[0], epp->epppDrop.eport_filtering[1], epp->epppDrop.eport_filtering[2],
		epp->epppDrop.eport_filtering[3], epp->epppDrop.eport_filtering[4], epp->epppDrop.eport_filtering[5]);

	pr_info("Tunnel_Exit_Mod_too_Small_Drop:  [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		epp->epppDrop.small_to_small[0], epp->epppDrop.small_to_small[1], epp->epppDrop.small_to_small[2],
		epp->epppDrop.small_to_small[3], epp->epppDrop.small_to_small[4], epp->epppDrop.small_to_small[5]);

	pr_info("Transmitted_Pkts_on_Egress_VRF:  [%10u, %10u, %10u, %10u]\n",
		epp->vrfOut[0], epp->vrfOut[1], epp->vrfOut[2], epp->vrfOut[3]);

	size = 16;
	for (i = 0; i < INGRESS_NAT_HIT_STATUS_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (epp->ingress_nat_hit[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Ingress_NAT_Hit_Status[%d-%d]:  [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
				index, index + size - 1,
				epp->ingress_nat_hit[index], epp->ingress_nat_hit[index + 1],
				epp->ingress_nat_hit[index + 2], epp->ingress_nat_hit[index + 3],
				epp->ingress_nat_hit[index + 4], epp->ingress_nat_hit[index + 5],
				epp->ingress_nat_hit[index + 6], epp->ingress_nat_hit[index + 7],
				epp->ingress_nat_hit[index + 8], epp->ingress_nat_hit[index + 9],
				epp->ingress_nat_hit[index + 10], epp->ingress_nat_hit[index + 11],
				epp->ingress_nat_hit[index + 12], epp->ingress_nat_hit[index + 13],
				epp->ingress_nat_hit[index + 14], epp->ingress_nat_hit[index + 15]);
	}

	size = 16;
	for (i = 0; i < EGRESS_NAT_HIT_STATUS_MAX / size; i++) {
		index = i * size; // based index
		no_empty = 0;

		for (j = 0; j < size; j++) {
			if (epp->egress_nat_hit[index + j]) {
				no_empty = 1;
				break;
			}
		}

		if (no_empty)
			pr_info("Egress_NAT_Hit_Status[%d-%d]:   [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
				index, index + size - 1,
				epp->egress_nat_hit[index], epp->egress_nat_hit[index + 1],
				epp->egress_nat_hit[index + 2], epp->egress_nat_hit[index + 3],
				epp->egress_nat_hit[index + 4], epp->egress_nat_hit[index + 5],
				epp->egress_nat_hit[index + 6], epp->egress_nat_hit[index + 7],
				epp->egress_nat_hit[index + 8], epp->egress_nat_hit[index + 9],
				epp->egress_nat_hit[index + 10], epp->egress_nat_hit[index + 11],
				epp->egress_nat_hit[index + 12], epp->egress_nat_hit[index + 13],
				epp->egress_nat_hit[index + 14], epp->egress_nat_hit[index + 15]);
	}

	pr_info("Drain_Port_Drop:                 [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		epp->drain[0], epp->drain[1], epp->drain[2], epp->drain[3], epp->drain[4], epp->drain[5]);

	pr_info("EPP_PM_Drop:                      %10u\n", epp->epmOverflow);
	pr_info("Re-queue_Overflow_Drop:           %10u\n", epp->rqOverflow);
	pr_info("EPP_Packet_Head:                  %10u\n", epp->eppTxPkt.head);
	pr_info("EPP_Packet_Tail:                  %10u\n", epp->eppTxPkt.tail);

	pr_info("=============================================================\n");
}

static void dubhe1000_switch_update_egress_stats(struct dubhe1000_adapter *adapter,
						 struct dubhe1000_switch_egress_stats *egress)
{
	u64 address;
	u32 value[2];
	int i;

	/* PS Packet Head/Tail Counter */
	egress->psTxPkt.head = dubhe1000_switch_get_value_directly(PS_PACKET_HEAD_COUNTER);
	egress->psTxPkt.tail = dubhe1000_switch_get_value_directly(PS_PACKET_TAIL_COUNTER);

	/* PS Error Counter */
	for (i = 0; i < PS_ERROR_COUNTER_MAX; i++) {
		address = dubhe1000_switch_get_addr_by_index(PS_ERROR_COUNTER, i);
		if (address) {
			value[0] = dubhe1000_switch_get_value_directly(address);
			value[1] = dubhe1000_switch_get_value_directly(address + 1);
			egress->psError_underrun[i] = value[0];
			egress->psError_overflow[i] = value[1];
		}
	}

	/* MAC Interface Counters For TX */
	for (i = 0; i < MAC_INTERFACE_COUNTERS_FOR_TX_MAX; i++) {
		address = dubhe1000_switch_get_addr_by_index(MAC_INTERFACE_COUNTERS_FOR_TX, i);
		if (address) {
			egress->txIf_packets[i] = dubhe1000_switch_get_value_directly(address);
			egress->txIf_error[i] = dubhe1000_switch_get_value_directly(address + 1);
			egress->txIf_halt[i] = dubhe1000_switch_get_value_directly(address + 2);
		}
	}
}

static void dubhe1000_switch_clear_egress_stats(struct dubhe1000_adapter *adapter)
{
	/* PS Packet Head/Tail Counter */
	dubhe1000_switch_clear_value_directly(PS_PACKET_HEAD_COUNTER);
	dubhe1000_switch_clear_value_directly(PS_PACKET_TAIL_COUNTER);

	/* PS Error Counter */
	// Read Only

	// Read Only
	/* MAC Interface Counters For TX */
}

static void dubhe1000_switch_dump_egress_stats(struct dubhe1000_adapter *adapter,
					       struct dubhe1000_switch_egress_stats *egress)
{
	pr_info("============= 5. SWITCH CORE EGRESS STATISTICS ==============\n");

	/* PS Error Counter */
	pr_info("PS_Underrun_Error:               [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		egress->psError_underrun[0], egress->psError_underrun[1], egress->psError_underrun[2],
		egress->psError_underrun[3], egress->psError_underrun[4], egress->psError_underrun[5]);

	pr_info("PS_overflow_Error:               [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		egress->psError_overflow[0], egress->psError_overflow[1], egress->psError_overflow[2],
		egress->psError_overflow[3], egress->psError_overflow[4], egress->psError_overflow[5]);

	/* PS Packet Head/Tail Counter */
	pr_info("PS_Packet_Head:                   %10u\n", egress->psTxPkt.head);
	pr_info("PS_Packet_Tail:                   %10u\n", egress->psTxPkt.tail);

	/* MAC Interface Counters For TX */
	pr_info("TX_Correct:                      [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		egress->txIf_packets[0], egress->txIf_packets[1], egress->txIf_packets[2],
		egress->txIf_packets[3], egress->txIf_packets[4], egress->txIf_packets[5]);

	pr_info("TX_Error:                        [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		egress->txIf_error[0], egress->txIf_error[1], egress->txIf_error[2],
		egress->txIf_error[3], egress->txIf_error[4], egress->txIf_error[5]);

	pr_info("TX_Halt:                         [%10u, %10u, %10u, %10u, %10u, %10u]\n",
		egress->txIf_halt[0], egress->txIf_halt[1], egress->txIf_halt[2],
		egress->txIf_halt[3], egress->txIf_halt[4], egress->txIf_halt[5]);

	pr_info("=============================================================\n");
}

void dubhe1000_switch_stats_dump(struct dubhe1000_adapter *adapter, u8 option)
{
	if (option < SWITCH_STATS_OPTION_MIN || option > SWITCH_STATS_OPTION_MAX)
		option = SWITCH_STATS_OPTION_ALL;

	pr_info("\n==================== SWITCH CORE STATISTICS =====================\n");

	if (option == SWITCH_STATS_OPTION_ALL || option == SWITCH_STATS_OPTION_INGRESS) {
		dubhe1000_switch_update_ingress_stats(adapter, &switch_stats.ingress_stats);
		dubhe1000_switch_dump_ingress_stats(adapter, &switch_stats.ingress_stats);
	}

	if (option == SWITCH_STATS_OPTION_ALL || option == SWITCH_STATS_OPTION_IPP) {
		dubhe1000_switch_update_ipp_stats(adapter, &switch_stats.ipp_stats);
		dubhe1000_switch_dump_ipp_stats(adapter, &switch_stats.ipp_stats);
	}

	if (option == SWITCH_STATS_OPTION_ALL || option == SWITCH_STATS_OPTION_SHARED_BM) {
		dubhe1000_switch_update_shared_bm_stats(adapter, &switch_stats.shared_bm_stats);
		dubhe1000_switch_dump_shared_bm_stats(adapter, &switch_stats.shared_bm_stats);
	}

	if (option == SWITCH_STATS_OPTION_ALL || option == SWITCH_STATS_OPTION_EPP) {
		dubhe1000_switch_update_epp_stats(adapter, &switch_stats.epp_stats);
		dubhe1000_switch_dump_epp_stats(adapter, &switch_stats.epp_stats);
	}

	if (option == SWITCH_STATS_OPTION_ALL || option == SWITCH_STATS_OPTION_EGRESS) {
		dubhe1000_switch_update_egress_stats(adapter, &switch_stats.egress_stats);
		dubhe1000_switch_dump_egress_stats(adapter, &switch_stats.egress_stats);
	}

	pr_info("=================================================================\n");
}

void dubhe1000_switch_clear_stats(struct dubhe1000_adapter *adapter)
{
	pr_info("\n================= CLEAR SWITCH CORE STATISTICS =================\n");

	dubhe1000_switch_clear_ingress_stats(adapter);

	dubhe1000_switch_clear_ipp_stats(adapter);

	dubhe1000_switch_clear_shared_bm_stats(adapter);

	dubhe1000_switch_clear_epp_stats(adapter);

	dubhe1000_switch_clear_egress_stats(adapter);

	pr_info("============================== Done =============================\n");
}
