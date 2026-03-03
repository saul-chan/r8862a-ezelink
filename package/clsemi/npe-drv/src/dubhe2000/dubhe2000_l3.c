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

/* dubhe1000_switch.c
 * Shared functions for accessing and configuring the Switch
 */

#include "dubhe2000_switch_regs.h"
#include "dubhe2000_switch.h"

void __iomem *switch_base_addr = NULL;

/* index is equal vrf. index0 is br-lan, index1 is for wan. others are reserved */
u8 default_gw_da_mac[8][6];

u16 switch_l3_hash_counter[HashBasedL3RoutingTable_nr_entries];
u64 switch_nexthop_info[NextHopTable_nr_entries];
#define L3_HASH_BIT_NUM 9

static u16 dubhe1000_l3_ipv4_hash_calc(u8 vrf, u8 *ipAddr)
{
	u16 hashval = 0;
	u32 *ip = (u32 *)ipAddr;
	u64 key = 0; // vrf << 32 + *ip
	u8 k = 0;
	u64 tmp = 0;

	for (k = 0; k < 4; k++) {
		key = (key << 8);
		key |= ipAddr[k];
	}

	/* vrf << 32 */
	tmp = vrf & 0x3;
	tmp = tmp << 32;
	key |= tmp;

	/* hash function */
	hashval = key & 0x1FF;
	hashval = hashval ^ (key >> 9);
	hashval = hashval & 0x1FF;
	hashval = hashval ^ (key >> 18);
	hashval = hashval & 0x1FF;
	hashval = hashval ^ (key >> 27);
	hashval = hashval & 0x1FF;

	pr_info("[%s] ipAddr %08x vrf %d: hashkey %llx hashval 0x%x\n", __func__, *ip, vrf, key, hashval);

	return hashval;
}

/* Router Port MAC Address: 16-Entry,  4 Number of Address per Entry
 *     macAddress:  0 ~ 47
 *     macMask:     48 ~ 95
 *     valid:       96
 *     vrf:         97 ~ 98
 */
static bool dubhe1000_switch_router_port_macAddr_table_get_free(struct dubhe1000_adapter *adapter, int *offset)
{
	bool ret = false;
	int k = 0, k2 = 0;
	u32 val[ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY] = { 0 };
	u8 used = 0;
	u64 address = ROUTER_PORT_MACADDR_TABLE;

	for (k = 0; k < ROUTER_PORT_MACADDR_TABLE_MAX; k++) {
		address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * k * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;
		used = 0;
		for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++) {
			val[k2] = readl(switch_base_addr + address + 4 * k2);
			if (val[k2] != 0) {
				used = 1;
				break;
			}
		}
		if (!used) {
			ret = true;
			*offset = k;
			break;
		}
	}

	return ret;
}

void dubhe1000_router_port_macAddr_table_add(struct dubhe1000_adapter *adapter, u8 *macAddr, u8 *macMask, u8 vrf)
{
	bool ret = false;
	int offset = 0;
	int k = 0;
	u8 macMask2[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	u64 address = ROUTER_PORT_MACADDR_TABLE;
	u8 data[32] = { 0 };
	u32 *val = NULL;
	u32 val4 = 0;

	ret = dubhe1000_switch_router_port_macAddr_table_get_free(adapter, &offset);
	if (!ret) {
		pr_info("ERROR: Router Port macAddr Add vrf %d macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x no free entry!!!\n",
			vrf, macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5], macMask2[0],
			macMask2[1], macMask2[2], macMask2[3], macMask2[4], macMask2[5]);
		return;
	}
	address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * offset * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;
	for (k = 0; k < 6; k++) {
		data[k] = macAddr[5 - k];
		data[k + 6] = macMask2[5 - k];
	}

	data[18] = ((vrf & 0x1) << 7) | (1 << 6);
	data[19] = ((vrf & 0x2) >> 1);

	for (k = 0; k < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k++) {
		val = (u32 *)&data[k * 4];
		writel(*val, (switch_base_addr + address + 4 * k));
		val4 = readl(switch_base_addr + address + 4 * k);
	}
	pr_info("Router Port macAddr Add vrf %d macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x\n",
		vrf, macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5], macMask2[0], macMask2[1],
		macMask2[2], macMask2[3], macMask2[4], macMask2[5]);
}

void dubhe1000_router_port_macAddr_table_del(struct dubhe1000_adapter *adapter, u8 *macAddr, u8 *macMask, u8 vrf)
{
	int k = 0, k2 = 0;
	u32 val[ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY] = { 0 };
	u32 *data32 = NULL;
	u8 data[32] = { 0 };
	u8 found = 1;
	u64 address = ROUTER_PORT_MACADDR_TABLE;
	u8 macMask2[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	for (k = 0; k < 6; k++) {
		data[k] = macAddr[5 - k];
		data[k + 6] = macMask2[5 - k];
	}
	data[18] = ((vrf & 0x1) << 7) | (1 << 6);
	data[19] = ((vrf & 0x2) >> 1);

	for (k = 0; k < ROUTER_PORT_MACADDR_TABLE_MAX; k++) {
		address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * k * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;
		found = 1;
		for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++) {
			data32 = (u32 *)&data[k2 * 4];
			val[k2] = readl(switch_base_addr + address + k2 * 4);
			if (val[k2] != *data32) {
				found = 0;
				break;
			}
		}
		if (found == 1) {
			for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++)
				writel(0, (switch_base_addr + address + 4 * k2));
			pr_info("Router Port MacAddr Del vrf %d macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x\n",
				vrf, macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
				macMask2[0], macMask2[1], macMask2[2], macMask2[3], macMask2[4], macMask2[5]);
			break;
		}
	}
}

bool dubhe1000_l3_routing_default_table_get_free(struct dubhe1000_adapter *adapter, int *offset)
{
	bool ret = false;
	int k = 0;
	u32 val = 0;
	u64 address = L3_ROUTING_DEFAULT_TBL;

	for (k = 0; k < L3_ROUTING_DEFAULT_TBL_MAX; k++) {
		address = L3_ROUTING_DEFAULT_TBL * 4 + k * 4;
		val = readl(switch_base_addr + address);
		if (val == 0) {
			ret = true;
			*offset = k;
			break;
		}
	}
	return ret;
}

void dubhe1000_l3_routing_default_table_op(struct dubhe1000_adapter *adapter, u8 nextHop, u8 drop, u8 toCPU, bool add)
{
	bool ret = false;
	int offset = 0;
	u64 address = L3_ROUTING_DEFAULT_TBL;
	u32 data = 0;
	u32 tmp = 0;

	if (add)
		data = (toCPU << 11) | (drop << 10) | nextHop;

	ret = dubhe1000_l3_routing_default_table_get_free(adapter, &offset);
	if (!ret) {
		pr_info("ERROR: l3_routing_default_table %s nextHop %d toCPU %d drop %d no free entry!!!\n",
			add ? "add" : "del", nextHop, toCPU, drop);
		return;
	}
	address = L3_ROUTING_DEFAULT_TBL * 4 + offset * 4;

	writel(data, (switch_base_addr + address));
	tmp = readl(switch_base_addr + address);

	pr_info("l3_routing_default_table(%llx) %s on %d nextHop %d toCPU %d drop %d read %08x data %08x\n", address,
		add ? "add" : "del", offset, nextHop, toCPU, drop, tmp, data);
}

void dubhe1000_ingress_router_table_op(struct dubhe1000_adapter *adapter, u32 val, bool add)
{
	bool ret = false;
	int offset = 0;
	u64 address = INGRESS_ROUTER_TBL;
	u32 data = 0x1D9001; // 0x3003;  // accept IPv4/6, IPv4/6 Hit Update
	u32 tmp = 0;

	if (add) {
		if (val != 0)
			data = val;
	} else {
		data = 0x1d8000;
	}

	ret = dubhe1000_l3_routing_default_table_get_free(adapter, &offset);
	if (!ret) {
		pr_info("ERROR: ingress_router_table %s val %x no free entry!!!\n", add ? "add" : "del", val);
		return;
	}
	address = INGRESS_ROUTER_TBL * 4 + 4 * offset;

	writel(data, (switch_base_addr + address));
	tmp = readl(switch_base_addr + address);

	pr_info("ingress_router_table %llx: %s on %d val %08x read %08x\n",
		address, add ? "add" : "del", offset, val, tmp);
}

void dubhe1000_ingr_egr_port_pkt_type_filter_table_op(struct dubhe1000_adapter *adapter, u8 egress_port, u8 filter_port,
						      bool add)
{
	u64 address = INGR_EGR_PORT_PKT_TYPE_FILTER_TBL;
	u32 data = 0;
	u32 tmp = 0;

	address = INGR_EGR_PORT_PKT_TYPE_FILTER_TBL * 4 + egress_port * 4;
	tmp = readl(switch_base_addr + address);

	pr_info("0 ingr_egr_port_pkt_type_filter(%llx) %s egress_port %d filter_port %d tmp %08x\n",
		address, add ? "add" : "del", egress_port, filter_port, tmp);
	if (add)
		tmp |= (filter_port << 19);

	pr_info("1 ingr_egr_port_pkt_type_filter(%llx) %s egress_port %d filter_port %d tmp %08x\n",
		address, add ? "add" : "del", egress_port, filter_port, tmp);

	writel(tmp, (switch_base_addr + address));
	data = readl(switch_base_addr + address);

	pr_info("1 ingr_egr_port_pkt_type_filter(%llx) %s egress_port %d filter_port %d tmp %08x data %08x\n",
		address, add ? "add" : "del", egress_port, filter_port, tmp, data);
}

void dubhe1000_next_hop_table_op(struct dubhe1000_adapter *adapter, u16 index, u8 nextHopPacketMod, u8 destPortNum,
				 bool isUc, bool drop, bool toCPU, bool add)
{
	u32 val1[NEXT_HOP_TBL_ADDR_PER_ENTRY] = { 0 };
	u64 address = NEXT_HOP_TBL * 4 + index * 4;
	int k = 0;

	pr_info("1 next_hop_table 0x%llx [%d]: %s destPortNum %d isUc %d drop %d toCPU %d\n",
		address, index, add ? "add" : "del", destPortNum, isUc, drop, toCPU);

	if (add) {
		val1[0] =
			(toCPU << 20) | (drop << 19) | (destPortNum << 13) | (isUc << 12) | (nextHopPacketMod << 2) | 1;

		pr_info("2 next_hop_table 0x%llx [%d]: %s destPortNum %d isUc %d drop %d toCPU %d val1 %08x\n",
			address, index, add ? "add" : "del", destPortNum, isUc, drop, toCPU, val1[0]);
	}
	for (k = 0; k < NEXT_HOP_TBL_ADDR_PER_ENTRY; k++)
		writel(val1[k], switch_base_addr + address + 4 * k);

	pr_info("3 next_hop_table 0x%llx [%d]: %s destPortNum %d isUc %d drop %d toCPU %d val1 %08x %08x\n",
		address, index, add ? "add" : "del", destPortNum, isUc, drop, toCPU, val1[0], val1[1]);
}

void dubhe1000_next_hop_packet_modify_table_op(struct dubhe1000_adapter *adapter, u8 index, bool add)
{
	u32 val = 0;
	u64 address = NEXT_HOP_PACKET_MODIFY_TBL * 4 + 4 * index * NEXT_HOP_PACKET_MODIFY_TBL_ADDR_PER_ENTRY;

	val = readl(switch_base_addr + address);
	if (add)
		val |= 1;
	else
		val &= (~(1 << 0));

	writel(val, switch_base_addr + address);

	pr_info("next_hop_packet_modify_table %s 0x%llx [index = %d]: val: %08x\n",
		add ? "add" : "del", address, index, val);
}

void dubhe1000_next_hop_da_mac_table_op(struct dubhe1000_adapter *adapter, u8 index, u8 *daMac, bool add)
{
	u32 *val = NULL;
	u64 address = NEXT_HOP_DA_MAC_TBL;
	u8 data[8] = { 0 };
	int k = 0;

	if (add) {
		for (k = 0; k < 6; k++)
			data[k] = daMac[5 - k];
	}

	for (k = 0; k < NEXT_HOP_DA_MAC_TBL_ADDR_PER_ENTRY; k++) {
		address = NEXT_HOP_DA_MAC_TBL * 4 + 4 * index * NEXT_HOP_DA_MAC_TBL_ADDR_PER_ENTRY + 4 * k;
		val = (u32 *)&data[4 * k];
		writel(*val, switch_base_addr + address);
	}

	pr_info("next_hop_da_mac_table %s [index = %d]: daMac: %02x %02x %02x %02x %02x %02x\n",
		add ? "add" : "del", index, daMac[0], daMac[1], daMac[2], daMac[3], daMac[4], daMac[5]);
}

void dubhe1000_hash_based_l3_routing_table_op(struct dubhe1000_adapter *adapter, u8 isIpv6, u8 vrf, u8 *destIP,
					      u8 nextHopPointer, bool add)
{
	u32 *val = NULL;
	u64 address = HASH_BASED_L3_ROUTING_TBL;
	u8 data[HASH_BASED_L3_ROUTING_TBL_ADDR_PER_ENTRY * 4] = { 0 };
	int k = 0;
	u8 tmp = (vrf << 2) | isIpv6;

	u16 hashval = dubhe1000_l3_ipv4_hash_calc(vrf, destIP);

	pr_info("1 hash_based_l3_routing_table %s vrf %d destIp: %02x %02x %02x %02x tmp %x isIpv6 %d nextHopPointer 0x%x hashval %x\n",
		add ? "add" : "del", vrf, destIP[0], destIP[1], destIP[2], destIP[3], tmp, isIpv6, nextHopPointer,
		hashval);

	if (add) {
		data[0] = (((destIP[3] & 0x0F) << 4) | tmp);
		data[1] = (((destIP[2] & 0x0F) << 4) | ((destIP[3] & 0xF0) >> 4));
		data[2] = (((destIP[1] & 0x0F) << 4) | ((destIP[2] & 0xF0) >> 4));
		data[3] = (((destIP[0] & 0x0F) << 4) | ((destIP[1] & 0xF0) >> 4));
		data[4] = ((destIP[0] & 0xF0) >> 4);

		data[16] = ((nextHopPointer & 0x0F) << 4);
		data[17] = ((nextHopPointer >> 4) & 0x3);
		data[17] |= (1 << 6); // not ECMP
	}

	for (k = 0; k < HASH_BASED_L3_ROUTING_TBL_ADDR_PER_ENTRY; k++) {
		address =
			HASH_BASED_L3_ROUTING_TBL * 4 + 4 * hashval * HASH_BASED_L3_ROUTING_TBL_ADDR_PER_ENTRY + 4 * k;
		val = (u32 *)&data[4 * k];
		writel(*val, switch_base_addr + address);
	}

	pr_info("2 hash_based_l3_routing_table %s [hashval = %d]: data: %02x %02x %02x %02x %02x nextHopPointer: %02x %02x\n",
		add ? "add" : "del", hashval, data[0], data[1], data[2], data[3], data[4], data[16], data[17]);
}

/* switch api*/
int dubhe2000_l3_nexthop_lookup_by_mac(u64 dmac)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_nexthop_info); i++) {
		if (dmac == (switch_nexthop_info[i] & 0xFFFFFFFFFFFF)) {
			return i;
		}
	}

	return -1;
}

// dmac is useful only when first addition
void dubhe2000_l3_nexthop_counter_op(int index, u64 dmac, bool is_add)
{
	u64 counter;

	if (index < 0 || index >= NextHopTable_nr_entries) {
		pr_err("[%s] invalid index\n", __func__);
		return;
	}

	counter = switch_nexthop_info[index] >> 48;

	if (is_add) {
		if (counter == 0)//first entry
			switch_nexthop_info[index] = dmac;

		counter++;
	} else {
		counter--;
		if (counter == 0)//empty entry
			switch_nexthop_info[index] = 0;
	}

	switch_nexthop_info[index] = (switch_nexthop_info[index] & 0xFFFFFFFFFFFF) + (counter << 48);
}

//ip_addr is little-endian
int dubhe1000_hash_based_l3_routing_tbl_get_new_v4_index(struct dubhe1000_adapter *adapter, u32 vrf, u32 ip_addr)
{
	u32 ipv4, hashval, print_addr;
	int i;
	t_HashBasedL3RoutingTable hashbased_l3_routing;

	hashval = l3_ipv4_hash(vrf, ip_addr);

	print_addr = htonl(ip_addr);

	for (i = 0; i < 4; i++) { //buckt number:0 -3
		rd_HashBasedL3RoutingTable(adapter, hashval + (i << 9), &hashbased_l3_routing);

		memcpy(&ipv4, hashbased_l3_routing.destIPAddr, sizeof(ipv4));

		if (!hashbased_l3_routing.ipVersion && (hashbased_l3_routing.vrf == 0) &&
		    (hashbased_l3_routing.vrf == 0) && (ipv4 == 0)) {
			if (adapter->tc_debug)
				pr_info("[%s] ip_addr=%pI4 vrf=%d hashval=0x%x bucket=%d\n", __func__,
					&print_addr, vrf, hashval, i);
			return (hashval + (i << 9)); //empty entry found
		}
	}

	return -ENOMEM;
}

int dubhe1000_hash_based_l3_routing_tbl_get_existed_v4_index(struct dubhe1000_adapter *adapter, u32 vrf, u32 ip_addr)
{
	u32 ipv4, hashval, print_addr;
	int i;
	t_HashBasedL3RoutingTable hashbased_l3_routing;

	hashval = l3_ipv4_hash(vrf, ip_addr);

	print_addr = htonl(ip_addr);

	for (i = 0; i < 4; i++) { //buckt number:0 -3
		rd_HashBasedL3RoutingTable(adapter, hashval + (i << 9), &hashbased_l3_routing);

		memcpy(&ipv4, hashbased_l3_routing.destIPAddr, sizeof(ipv4));

		if (!hashbased_l3_routing.ipVersion && (hashbased_l3_routing.vrf == vrf) && (ipv4 == ip_addr)) {
			if (adapter->tc_debug)
				pr_info("[%s] ip_addr=%pI4 vrf=%d hashval=0x%x bucket=%d\n", __func__,
					&print_addr, vrf, hashval, i);
			return (hashval + (i << 9)); //already existing
		}
	}

	return -EINVAL;
}

/* This function is Not a common address translation.
 * Only used for switch register field
 */
int dubhe1000_change_in6_addr_to_u8(struct in6_addr ipv6, u8 *data)
{
	u32 val[4];

	if (!data) {
		pr_err("[%s] null data\n", __func__);

		return 0;
	}

	memcpy(val, &(ipv6.s6_addr), sizeof(val));
	val[0] = htons(val[0] & 0xFFFF) + ((htons((val[0] >> 16) & 0xFFFF)) << 16);
	val[1] = htons(val[1] & 0xFFFF) + ((htons((val[1] >> 16) & 0xFFFF)) << 16);
	val[2] = htons(val[2] & 0xFFFF) + ((htons((val[2] >> 16) & 0xFFFF)) << 16);
	val[3] = htons(val[3] & 0xFFFF) + ((htons((val[3] >> 16) & 0xFFFF)) << 16);

	memcpy(data, val, sizeof(val));

	return 1;
}

//ip_addr is little-endian
int dubhe1000_hash_based_l3_routing_tbl_get_new_v6_index(struct dubhe1000_adapter *adapter, u32 vrf, const u8 *ipv6)
{
	u8 val[16];
	u32 ipv6_addr[4]; // 2001:1234::1 --- u32 ipAddr[4] = {0x1, 0x0, 0x0, 0x20011234};
	u32 hashval;
	t_HashBasedL3RoutingTable hashbased_l3_routing;
	int i;

	memcpy(ipv6_addr, ipv6, sizeof(ipv6_addr));
	hashval = l3_ipv6_hash(vrf, ipv6_addr);

	for (i = 0; i < 4; i++) { //buckt number:0 -3
		memset(&hashbased_l3_routing, 0, sizeof(hashbased_l3_routing));
		rd_HashBasedL3RoutingTable(adapter, hashval + (i << 9), &hashbased_l3_routing);

		memcpy(&val, hashbased_l3_routing.destIPAddr, sizeof(val));

		if (!hashbased_l3_routing.ipVersion && (hashbased_l3_routing.vrf == 0) &&
		    (dubhe1000_check_u8_array_is_zero(val, sizeof(val)))) {
			if (adapter->tc_debug)
				pr_info("[%s] ipv6_addr=%pI6 vrf=%d hashval=0x%x bucket=%d\n", __func__,
					(struct in6_addr *)ipv6, vrf, hashval, i);
			return (hashval + (i << 9)); //new
		}
	}

	return -ENOMEM;
}

int dubhe1000_hash_based_l3_routing_tbl_get_existed_v6_index(struct dubhe1000_adapter *adapter, u32 vrf, const u8 *ipv6)
{
	u8 val[16];
	u32 ipv6_addr[4]; // 2001:1234::1 --- u32 ipAddr[4] = {0x1, 0x0, 0x0, 0x20011234};
	u32 hashval;
	t_HashBasedL3RoutingTable hashbased_l3_routing;
	int i;

	memcpy(ipv6_addr, ipv6, sizeof(ipv6_addr));
	hashval = l3_ipv6_hash(vrf, ipv6_addr);

	for (i = 0; i < 4; i++) { //buckt number:0 -3
		memset(&hashbased_l3_routing, 0, sizeof(hashbased_l3_routing));
		rd_HashBasedL3RoutingTable(adapter, hashval + (i << 9), &hashbased_l3_routing);

		memcpy(&val, hashbased_l3_routing.destIPAddr, sizeof(val));

		if (hashbased_l3_routing.ipVersion && (hashbased_l3_routing.vrf == vrf) &&
		    (!memcmp(val, ipv6, sizeof(val)))) {
			if (adapter->tc_debug)
				pr_info("[%s] ipv6_addr=%pI6 vrf=%d hashval=0x%x bucket=%d\n", __func__,
					(struct in6_addr *)ipv6, vrf, hashval, i);

			return (hashval + (i << 9)); //existed
		}
	}

	return -ENOMEM;
}

int dubhe1000_next_hop_tbl_get_new_index(struct dubhe1000_adapter *adapter, u64 dmac)
{
	t_NextHopTable nexthop_tbl;
	int i, index;

	index = dubhe2000_l3_nexthop_lookup_by_mac(dmac);

	if (index >= 0) {//already existed
		return index;
	} else {// new index
		for (i = 0; i < ARRAY_SIZE(switch_nexthop_info); i++) {
			rd_NextHopTable(adapter, i, &nexthop_tbl);
	
			if (!nexthop_tbl.validEntry)
				return i;
		}
	}

	return -ENOMEM;
}

void dubhe2000_switch_l3_init(struct dubhe1000_adapter *adapter)
{
	t_IngressRouterTable in_router_tbl;
	t_L3RoutingDefault l3_default;

	/*
	 * vrf 0/1 will be updated during system init process.
	 * We should prepare these configurations in advance to ensure that
	 * the default routing packets can be sent to the CPU
	 */

	memset(&in_router_tbl, 0, sizeof(in_router_tbl));
	memset(&l3_default, 0, sizeof(l3_default));

	in_router_tbl.acceptIPv4 = 1;
	in_router_tbl.acceptIPv6 = 1;
	in_router_tbl.ipv4HitUpdates = 1;
	in_router_tbl.ipv6HitUpdates = 1;
	wr_IngressRouterTable(adapter, 0, &in_router_tbl);
	wr_IngressRouterTable(adapter, 1, &in_router_tbl);

	l3_default.nextHop = 0; //just reserved for defalut router.
	l3_default.sendToCpu = 1;
	wr_L3RoutingDefault(adapter, 0, &l3_default);
	wr_L3RoutingDefault(adapter, 1, &l3_default);
}

void dubhe2000_l3_init_config(struct dubhe1000_adapter *adapter)
{
	int i;
	t_IngressRouterTable ingress_router;
	t_L3RoutingDefault l3_default;

	memset(switch_l3_hash_counter, 0, sizeof(switch_l3_hash_counter));
	memset(switch_nexthop_info, 0, sizeof(switch_nexthop_info));

	// Ingress Router Table
	for (i = 0; i < DUBHE2000_VRF_USED_MAX; i++) {
		memset(&ingress_router, 0, sizeof(ingress_router));
		rd_IngressRouterTable(adapter, i, &ingress_router);

		ingress_router.acceptIPv4 = 1;
		ingress_router.acceptIPv6 = 1;
		ingress_router.ipv4HitUpdates = 1;
		ingress_router.ipv6HitUpdates = 1;
		wr_IngressRouterTable(adapter, i, &ingress_router);
	}

	// L3 Routing Default
	for (i = 0; i < DUBHE2000_VRF_USED_MAX; i++) {
		memset(&l3_default, 0, sizeof(l3_default));
		rd_L3RoutingDefault(adapter, i, &l3_default);

		l3_default.sendToCpu = 1;

		wr_L3RoutingDefault(adapter, i, &l3_default);
	}

	memset(default_gw_da_mac, 0, sizeof(default_gw_da_mac));
}

void dubhe2000_switch_l3_hash_counter_dump(void)
{
	int i;

	pr_info("[%s]:\n", __func__);

	for (i = 0; i < ARRAY_SIZE(switch_l3_hash_counter); i++) {
		if (switch_l3_hash_counter[i])
			pr_info("[%s] index=%d counter=%d\n", __func__, i, switch_l3_hash_counter[i]);
	}
}

void dubhe2000_switch_nexthop_info_dump(void)
{
	int i;

	pr_info("[%s]:\n", __func__);

	for (i = 0; i < ARRAY_SIZE(switch_nexthop_info); i++) {
		if (switch_nexthop_info[i])
			pr_info("[%s] index=%d mac=0x%llx counter=%lld\n", __func__, i,
					switch_nexthop_info[i] & 0xFFFFFFFFFFFF,
					switch_nexthop_info[i] >> 48);
	}
}


void cls_npe_router_port_macaddr_config(u8 *mac, bool is_wan)
{
	t_RouterPortMACAddress router_mac;
	u8 dmac[6];
	u64 tmp;
	int i, index, vrf;

	if (!mac) {
		pr_err("[%s] NULL mac\n", __func__);
		return;
	}

	/* Only two vrf will be used in dubhe1000,
	 * we will use the following cfg on a fixed basis:
	 *     Bridge: index 0 vrf 0
	 *     Wan : index 1 vrf 1
	 */
	if (is_wan) {
		index = 1;
		vrf = 1;
		memcpy(default_gw_da_mac[WAN_MAC_INX], mac, ETH_ALEN);
	} else {
		index = 0;
		vrf = 0;
		memcpy(default_gw_da_mac[LAN_MAC_INX], mac, ETH_ALEN);
	}

	memset(&router_mac, 0, sizeof(router_mac));

	if (!dubhe1000_check_u8_array_is_zero(mac, ETH_ALEN)) { // if not, delete it
		for (i = 0; i < ETH_ALEN; i++)
			dmac[i] = mac[ETH_ALEN - i - 1];

		memcpy(&tmp, dmac, ETH_ALEN);
		router_mac.macAddress = tmp;

		memset(&tmp, 0xff, ETH_ALEN);
		router_mac.macMask = tmp;

		router_mac.valid = 1;
		router_mac.vrf = vrf;
	}

	wr_RouterPortMACAddress(g_adapter, index, &router_mac);

	//Router Port Egress SA MAC Address will be config when add ipv4_nat/ipv6_router
}
EXPORT_SYMBOL(cls_npe_router_port_macaddr_config);
