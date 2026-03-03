// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_l3.h"
#include <arpa/inet.h>

static u16 dubhe2000_l3_ipv4_hash_calc(u8 vrf, u32 ipAddr)
{
	u16 hashval = 0;
	u64 key = 0;// vrf << 32 + ipAddr
	u64 tmp = 0;

	/* vrf << 32 */
	tmp = vrf & 0x3;
	tmp = tmp << 32;
	key = tmp | ipAddr;

	/* hash function */
	hashval = key & 0x1FF;
	hashval = hashval ^ (key >> 9);
	hashval = hashval & 0x1FF;
	hashval = hashval ^ (key >> 18);
	hashval = hashval & 0x1FF;
	hashval = hashval ^ (key >> 27);
	hashval = hashval & 0x1FF;

	printf("[%s] ipAddr 0x%08x vrf %d: hashkey 0x%lx hashval 0x%x\n",
				__func__, ipAddr, vrf, key, hashval);

	return hashval;
}

// 2001:1234::1 --- u32 ipAddr[4] = {0x1, 0x0, 0x0, 0x20011234};
static u16 dubhe2000_l3_ipv6_hash_calc(u8 vrf, u32 *ipAddr)
{
	u32 tmp[5] = {0};
	u16 hashval = 0;
	u8 shift = 9;
	u32 base_value = get_one_bits(0, shift -1 );

	memcpy(tmp, ipAddr, sizeof(u32) * 4);
	tmp[4] = vrf;

	printf("[%s] hashkey = [0x%x%08x%08x%08x%08x]\n", __func__, tmp[4], tmp[3], tmp[2], tmp[1], tmp[0]);

	hashval = tmp[0] & base_value;

	for (int i = 1; i < 15; i++) {
		switch_u32_array_right_shift(tmp, ARRAY_SIZE(tmp), shift);
		hashval = hashval ^ tmp[0];
		hashval = hashval & base_value;
	}

	return hashval;
}

/***********************************************************************/

struct switch_field_def switch_router_port_mac_address[] = {
	{ROUTER_PORT_MAC_ARG_MACADDRESS, "macAddress", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{ROUTER_PORT_MAC_ARG_MACMASK, "macMask", 48, 48, SWITCH_FIELD_TYPE_MACADDR},
	{ROUTER_PORT_MAC_ARG_SELECTMACENTRYPORTMASK, "selectMacEntryPortMask", 6, 96},
	{ROUTER_PORT_MAC_ARG_ALTMACADDRESS, "altMacAddress", 48, 102, SWITCH_FIELD_TYPE_MACADDR},
	{ROUTER_PORT_MAC_ARG_VALID, "valid", 1, 150},
	{ROUTER_PORT_MAC_ARG_VRF, "vrf", 2, 151},
};

void switch_l3_port_mac_usage(void)
{
	printf("Usage: npecmd switch l3 port_mac <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", ROUTER_PORT_MAC_ADDRESS_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_router_port_mac_address); i++)
		printf(" %s", switch_router_port_mac_address[i].name);

	printf("\n");
}

int switch_l3_port_mac_config(int argc, char **argv)
{
	u64 address;
	u32 value[ROUTER_PORT_MAC_ADDRESS_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= ROUTER_PORT_MAC_ADDRESS_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = ROUTER_PORT_MAC_ADDRESS + index * ROUTER_PORT_MAC_ADDRESS_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < ROUTER_PORT_MAC_ADDRESS_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_router_port_mac_address, ARRAY_SIZE(switch_router_port_mac_address), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < ROUTER_PORT_MAC_ADDRESS_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x %08x %08x %08x]\n",
				__func__, index, address,
				value[7], value[6], value[5], value[4],
				value[3], value[2], value[1], value[0]);

	return 1;
ERR:
	switch_l3_port_mac_usage();
	return 0;
}

struct switch_field_def switch_ingress_router_table[] = {
	{INGRESS_ROUTER_TABLE_ARG_ACCEPTIPV4, "acceptIPv4", 1, 0},
	{INGRESS_ROUTER_TABLE_ARG_ACCEPTIPV6, "acceptIPv6", 1, 1},
	{INGRESS_ROUTER_TABLE_ARG_ACCEPTMPLS, "acceptMPLS", 1, 2},
	{INGRESS_ROUTER_TABLE_ARG_MINTTL, "minTTL", 8, 3},
	{INGRESS_ROUTER_TABLE_ARG_MINTTLTOCPU, "minTtlToCpu", 1, 11},
	{INGRESS_ROUTER_TABLE_ARG_IPV4HITUPDATES, "ipv4HitUpdates", 1, 12},
	{INGRESS_ROUTER_TABLE_ARG_IPV6HITUPDATES, "ipv6HitUpdates", 1, 13},
	{INGRESS_ROUTER_TABLE_ARG_MPLSHITUPDATES, "mplsHitUpdates", 1, 14},
	{INGRESS_ROUTER_TABLE_ARG_ECMPUSEIPDA, "ecmpUseIpDa", 1, 15},
	{INGRESS_ROUTER_TABLE_ARG_ECMPUSEIPSA, "ecmpUseIpSa", 1, 16},
	{INGRESS_ROUTER_TABLE_ARG_ECMPUSEIPTOS, "ecmpUseIpTos", 1, 17},
	{INGRESS_ROUTER_TABLE_ARG_ECMPUSEIPPROTO, "ecmpUseIpProto", 1, 18},
	{INGRESS_ROUTER_TABLE_ARG_ECMPUSEIPL4SP, "ecmpUseIpL4Sp", 1, 19},
	{INGRESS_ROUTER_TABLE_ARG_ECMPUSEIPL4DP, "ecmpUseIpL4Dp", 1, 20},
	{INGRESS_ROUTER_TABLE_ARG_MMPVALID, "mmpValid", 1, 21},
	{INGRESS_ROUTER_TABLE_ARG_MMPPTR, "mmpPtr", 5, 22},
	{INGRESS_ROUTER_TABLE_ARG_MMPORDER, "mmpOrder", 2, 27},
};

void switch_l3_ingress_router_usage(void)
{
	printf("Usage: npecmd switch l3 ingress_router <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", INGRESS_ROUTER_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_ingress_router_table); i++)
		printf(" %s", switch_ingress_router_table[i].name);

	printf("\n");
}

int switch_l3_ingress_router_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_ROUTER_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_ROUTER_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_ROUTER_TABLE + index * INGRESS_ROUTER_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_ROUTER_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_ingress_router_table, ARRAY_SIZE(switch_ingress_router_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_ROUTER_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n",
				__func__, index, address, value[0]);

	return 1;
ERR:
	switch_l3_ingress_router_usage();
	return 0;
}

struct switch_field_def switch_l3_lpm_result[] = {
	{L3_LPM_RESULT_ARG_USEECMP, "useECMP", 1, 0},
	{L3_LPM_RESULT_ARG_ECMPMASK, "ecmpMask", 6, 1},
	{L3_LPM_RESULT_ARG_ECMPSHIFT, "ecmpShift", 3, 7},
	{L3_LPM_RESULT_ARG_NEXTHOPPOINTER, "nextHopPointer", 10, 10},
};

void switch_l3_lpm_result_usage(void)
{
	printf("Usage: npecmd switch l3 lpm_result <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", L3_LPM_RESULT_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l3_lpm_result); i++)
		printf(" %s", switch_l3_lpm_result[i].name);

	printf("\n");
}

int switch_l3_lpm_result_config(int argc, char **argv)
{
	u64 address;
	u32 value[L3_LPM_RESULT_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L3_LPM_RESULT_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = L3_LPM_RESULT + index * L3_LPM_RESULT_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L3_LPM_RESULT_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l3_lpm_result, ARRAY_SIZE(switch_l3_lpm_result), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L3_LPM_RESULT_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n",
				__func__, index, address, value[0]);

	return 1;
ERR:
	switch_l3_lpm_result_usage();
	return 0;
}

void switch_l3_hash_cal_v4_usage(void)
{
	printf("Usage: npecmd switch l3 hash_cal_v4 <vrf> <IPv4> <bucket>\n");
	printf("vrf range: [0 - %d]\n", get_one_bits(0, IP_ROUTE_VRF_NUMBER - 1));
	printf("bucket range: [0 - %d]\n", get_one_bits(0, IP_TABLE_HASH_BUCKET_NUMBER - 1));
}

int switch_l3_hash_cal_v4_config(int argc, char **argv)
{
	u8 vrf;
	u16 index, bucket;
	u32 tmp, ipAddr;

	if (argc != 3) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[0], "%i", &tmp) != 1) || (tmp > get_one_bits(0, IP_ROUTE_VRF_NUMBER - 1))) {
		printf("[%s] Failed! Invalid vrf\n", __func__);
		goto ERR;
	}
	vrf = tmp & get_one_bits(0, IP_ROUTE_VRF_NUMBER - 1);

	if (!switch_ipv4_to_u32(argv[1], &ipAddr)) {
		printf("[%s] Failed! Invalid IPv4 Address\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[2], "%i", &tmp) != 1) || (tmp > get_one_bits(0, IP_TABLE_HASH_BUCKET_NUMBER - 1))) {
		printf("[%s] Failed! Invalid bucket\n", __func__);
		goto ERR;
	}

	bucket = tmp & get_one_bits(0, IP_TABLE_HASH_BUCKET_NUMBER - 1);
	index = dubhe2000_l3_ipv4_hash_calc(vrf, ipAddr) + (bucket << 9);

	printf("[%s] index=0x%x\n", __func__, index);

	return 1;
ERR:
	switch_l3_hash_cal_v4_usage();
	return 0;
}

void switch_l3_hash_cal_v6_usage(void)
{
	printf("Usage: npecmd switch l3 hash_cal_v6 <vrf> <IPv6> <bucket>\n");
	printf("vrf range: [0 - %d]\n", get_one_bits(0, IP_ROUTE_VRF_NUMBER - 1));
	printf("bucket range: [0 - %d]\n", get_one_bits(0, IP_TABLE_HASH_BUCKET_NUMBER - 1));
}

int switch_l3_hash_cal_v6_config(int argc, char **argv)
{
	u8 vrf;
	u16 index, bucket;
	u32 tmp, ipAddr[4];

	if (argc != 3) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[0], "%i", &tmp) != 1) || (tmp > get_one_bits(0, IP_ROUTE_VRF_NUMBER - 1))) {
		printf("[%s] Failed! Invalid vrf\n", __func__);
		goto ERR;
	}
	vrf = tmp & get_one_bits(0, IP_ROUTE_VRF_NUMBER - 1);

	if (!switch_ipv6_to_u32(argv[1], ipAddr)) {
		printf("[%s] Failed! Invalid IPv6 Address\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[2], "%i", &tmp) != 1) || (tmp > get_one_bits(0, IP_TABLE_HASH_BUCKET_NUMBER - 1))) {
		printf("[%s] Failed! Invalid bucket\n", __func__);
		goto ERR;
	}

	bucket = tmp & get_one_bits(0, IP_TABLE_HASH_BUCKET_NUMBER - 1);
	index = dubhe2000_l3_ipv6_hash_calc(vrf, ipAddr) + (bucket << 9);

	printf("[%s] index=0x%x\n", __func__, index);

	return 1;
ERR:
	switch_l3_hash_cal_v6_usage();
	return 0;
}

struct switch_field_def switch_hash_based_l3_routing_table[] = {
	{HASH_BASED_L3_ROUTING_TABLE_ARG_IPVERSION, "ipVersion", 1, 0},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_MPLS, "mpls", 1, 1},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_VRF, "vrf", 2, 2},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_DESTIPADDR, "destIPAddr", 128, 4},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_NEXTHOPPOINTER, "nextHopPointer", 10, 132},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_USEECMP, "useECMP", 1, 142},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_ECMPMASK, "ecmpMask", 6, 143},
	{HASH_BASED_L3_ROUTING_TABLE_ARG_ECMPSHIFT, "ecmpShift", 3, 149},
};

void switch_l3_hash_based_usage(void)
{
	printf("Usage: npecmd switch l3 hash_based <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", HASH_BASED_L3_ROUTING_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_hash_based_l3_routing_table); i++)
		printf(" %s", switch_hash_based_l3_routing_table[i].name);

	printf("\n");
}

int switch_l3_hash_based_config(int argc, char **argv)
{
	u64 address;
	u32 value[HASH_BASED_L3_ROUTING_TABLE_ADDR_PER_ENTRY] = {0}, index, tmp;
	int i, type;
	bool ipVersion, mpls;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= HASH_BASED_L3_ROUTING_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = HASH_BASED_L3_ROUTING_TABLE + index * HASH_BASED_L3_ROUTING_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < HASH_BASED_L3_ROUTING_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//get the type of "destIPAddr"
	//	1. old value
	ipVersion = value[0] & 0x1;
	mpls = value[0] & 0x2;

	//	2. argv value
	for (i = 1; i < argc; i++) {
		if (sscanf(argv[i], "--ipVersion=%i", &tmp) == 1)
			ipVersion = tmp & 0x1;

		if (sscanf(argv[i], "--mpls=%i", &tmp) == 1)
			mpls = tmp & 0x1;
	}

	//	3. update param type
	if (mpls)
		type = SWITCH_FIELD_TYPE_SHORT_VALUE;
	else if (ipVersion)
		type = SWITCH_FIELD_TYPE_IPV6;
	else
		type = SWITCH_FIELD_TYPE_IPV4;

	printf("[%s] destIPAddr.type=%d\n", __func__, type);
	switch_hash_based_l3_routing_table[HASH_BASED_L3_ROUTING_TABLE_ARG_DESTIPADDR].flag = type;

	//update
	if (!switch_field_setup(argc, argv, switch_hash_based_l3_routing_table, ARRAY_SIZE(switch_hash_based_l3_routing_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < HASH_BASED_L3_ROUTING_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x %08x %08x %08x]\n",
				__func__, index, address,
				value[7], value[6], value[5], value[4],
				value[3], value[2], value[1], value[0]);

	return 1;
ERR:
	switch_l3_hash_based_usage();
	return 0;
}

struct switch_field_def switch_l3_routing_default[] = {
	{L3_ROUTING_DEFAULT_ARG_NEXTHOP, "nextHop", 10, 0},
	{L3_ROUTING_DEFAULT_ARG_PKTDROP, "pktDrop", 1, 10},
	{L3_ROUTING_DEFAULT_ARG_SENDTOCPU, "sendToCpu", 1, 11},
};

void switch_l3_routing_default_usage(void)
{
	printf("Usage: npecmd switch l3 routing_default <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", L3_ROUTING_DEFAULT_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l3_routing_default); i++)
		printf(" %s", switch_l3_routing_default[i].name);

	printf("\n");
}

int switch_l3_routing_default_config(int argc, char **argv)
{
	u64 address;
	u32 value[L3_ROUTING_DEFAULT_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L3_ROUTING_DEFAULT_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = L3_ROUTING_DEFAULT + index * L3_ROUTING_DEFAULT_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L3_ROUTING_DEFAULT_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l3_routing_default, ARRAY_SIZE(switch_l3_routing_default), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L3_ROUTING_DEFAULT_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n",
				__func__, index, address, value[0]);

	return 1;
ERR:
	switch_l3_routing_default_usage();
	return 0;
}

struct switch_field_def switch_l3_routing_tcam[] = {
	{L3_ROUTING_TCAM_ARG_PROTO, "proto", 2, 0},
	{L3_ROUTING_TCAM_ARG_VRF, "vrf", 2, 2},
	{L3_ROUTING_TCAM_ARG_DESTIPADDR, "destIPAddr", 128, 4},
	{L3_ROUTING_TCAM_ARG_PROTOMASKN, "protoMaskN", 2, 132},
	{L3_ROUTING_TCAM_ARG_VRFMASKN, "vrfMaskN", 2, 134},
	{L3_ROUTING_TCAM_ARG_DESTIPADDRMASKN, "destIPAddrMaskN", 128, 136},
	{L3_ROUTING_TCAM_ARG_VALID, "valid", 1, 264},
};

void switch_l3_routing_tcam_usage(void)
{
	printf("Usage: npecmd switch l3 routing_tcam <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", L3_ROUTING_TCAM_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l3_routing_tcam); i++)
		printf(" %s", switch_l3_routing_tcam[i].name);

	printf("\n");
}

int switch_l3_routing_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[L3_ROUTING_TCAM_ADDR_PER_ENTRY] = {0}, index, tmp;
	int i, type;
	u8 proto;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L3_ROUTING_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = L3_ROUTING_TCAM + index * L3_ROUTING_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L3_ROUTING_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//get the type of "destIPAddr"
	//	1. old value
	proto = value[0] & 0x3;

	//	2. argv value
	for (i = 1; i < argc; i++) {
		if (sscanf(argv[i], "--proto=%i", &tmp) == 1) {
			proto = tmp & 0x3;
			break;
		}
	}

	//	3. update param type
	if (proto == 2)
		type = SWITCH_FIELD_TYPE_IPV4;
	else if (proto == 3)
		type = SWITCH_FIELD_TYPE_IPV6;
	else
		type = SWITCH_FIELD_TYPE_SHORT_VALUE;

	printf("[%s] destIPAddr.type=%d\n", __func__, type);
	switch_l3_routing_tcam[L3_ROUTING_TCAM_ARG_DESTIPADDR].flag = type;
	switch_l3_routing_tcam[L3_ROUTING_TCAM_ARG_DESTIPADDRMASKN].flag = type;

	//update
	if (!switch_field_setup(argc, argv, switch_l3_routing_tcam, ARRAY_SIZE(switch_l3_routing_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L3_ROUTING_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value(15-8) 0x[%08x %08x %08x %08x %08x %08x %08x %08x]\n",
				__func__, index, address,
				value[15], value[14], value[13], value[12],
				value[11], value[10], value[9], value[8]);

	printf("[%s] index %d addr 0x%lx value(7-0) 0x[%08x %08x %08x %08x %08x %08x %08x %08x]\n",
				__func__, index, address,
				value[7], value[6], value[5], value[4],
				value[3], value[2], value[1], value[0]);

	return 1;
ERR:
	switch_l3_routing_tcam_usage();
	return 0;
}

struct switch_field_def switch_next_hop_table[] = {
	{NEXT_HOP_TABLE_ARG_VALIDENTRY, "validEntry", 1, 0},
	{NEXT_HOP_TABLE_ARG_SRV6SID, "srv6Sid", 1, 1},
	{NEXT_HOP_TABLE_ARG_NEXTHOPPACKETMOD, "nextHopPacketMod", 10, 2},
	{NEXT_HOP_TABLE_ARG_L2UC, "l2Uc", 1, 12},
	{NEXT_HOP_TABLE_ARG_DESTPORT_OR_MCADDR, "destPort_or_mcAddr", 6, 13},
	{NEXT_HOP_TABLE_ARG_PKTDROP, "pktDrop", 1, 19},
	{NEXT_HOP_TABLE_ARG_SENDTOCPU, "sendToCpu", 1, 20},
	{NEXT_HOP_TABLE_ARG_TUNNELENTRY, "tunnelEntry", 1, 21},
	{NEXT_HOP_TABLE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 22},
	{NEXT_HOP_TABLE_ARG_TUNNELEXIT, "tunnelExit", 1, 28},
	{NEXT_HOP_TABLE_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 29},

};

void switch_l3_nexthop_table_usage(void)
{
	printf("Usage: npecmd switch l3 nexthop_table <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", NEXT_HOP_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_next_hop_table); i++)
		printf(" %s", switch_next_hop_table[i].name);

	printf("\n");
}

int switch_l3_nexthop_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[NEXT_HOP_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= NEXT_HOP_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = NEXT_HOP_TABLE + index * NEXT_HOP_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < NEXT_HOP_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_next_hop_table, ARRAY_SIZE(switch_next_hop_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < NEXT_HOP_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n",
				__func__, index, address, value[1], value[0]);

	return 1;
ERR:
	switch_l3_nexthop_table_usage();
	return 0;
}

struct switch_field_def switch_next_hop_pkt_mod[] = {
	{NEXT_HOP_PKT_MOD_ARG_VALID, "valid", 1, 0},
	{NEXT_HOP_PKT_MOD_ARG_OUTERVLANAPPEND, "outerVlanAppend", 1, 1},
	{NEXT_HOP_PKT_MOD_ARG_OUTERPCPSEL, "outerPcpSel", 2, 2},
	{NEXT_HOP_PKT_MOD_ARG_OUTERCFIDEISEL, "outerCfiDeiSel", 2, 4},
	{NEXT_HOP_PKT_MOD_ARG_OUTERETHTYPE, "outerEthType", 2, 6},
	{NEXT_HOP_PKT_MOD_ARG_OUTERVID, "outerVid", 12, 8},
	{NEXT_HOP_PKT_MOD_ARG_OUTERPCP, "outerPcp", 3, 20},
	{NEXT_HOP_PKT_MOD_ARG_OUTERCFIDEI, "outerCfiDei", 1, 23},
	{NEXT_HOP_PKT_MOD_ARG_INNERVLANAPPEND, "innerVlanAppend", 1, 24},
	{NEXT_HOP_PKT_MOD_ARG_INNERPCPSEL, "innerPcpSel", 2, 25},
	{NEXT_HOP_PKT_MOD_ARG_INNERCFIDEISEL, "innerCfiDeiSel", 2, 27},
	{NEXT_HOP_PKT_MOD_ARG_INNERETHTYPE, "innerEthType", 2, 29},
	{NEXT_HOP_PKT_MOD_ARG_INNERVID, "innerVid", 12, 31},
	{NEXT_HOP_PKT_MOD_ARG_INNERPCP, "innerPcp", 3, 43},
	{NEXT_HOP_PKT_MOD_ARG_INNERCFIDEI, "innerCfiDei", 1, 46},
	{NEXT_HOP_PKT_MOD_ARG_MSPTPTR, "msptPtr", 4, 47},
};

void switch_l3_nexthop_pkt_mod_usage(void)
{
	printf("Usage: npecmd switch l3 nexthop_pkt_mod <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", NEXT_HOP_PACKET_MODIFICATIONS_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_next_hop_pkt_mod); i++)
		printf(" %s", switch_next_hop_pkt_mod[i].name);

	printf("\n");
}

int switch_l3_nexthop_pkt_mod_config(int argc, char **argv)
{
	u64 address;
	u32 value[NEXT_HOP_PACKET_MODIFICATIONS_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= NEXT_HOP_PACKET_MODIFICATIONS_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = NEXT_HOP_PACKET_MODIFICATIONS + index * NEXT_HOP_PACKET_MODIFICATIONS_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < NEXT_HOP_PACKET_MODIFICATIONS_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_next_hop_pkt_mod, ARRAY_SIZE(switch_next_hop_pkt_mod), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < NEXT_HOP_PACKET_MODIFICATIONS_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n",
				__func__, index, address, value[1], value[0]);

	return 1;
ERR:
	switch_l3_nexthop_pkt_mod_usage();
	return 0;
}

struct switch_field_def switch_next_hop_da_mac[] = {
	{NEXT_HOP_DA_MAC_ARG_DAMAC, "daMac", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
};

void switch_l3_nexthop_da_usage(void)
{
	printf("Usage: npecmd switch l3 nexthop_da <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", NEXT_HOP_DA_MAC_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_next_hop_da_mac); i++)
		printf(" %s", switch_next_hop_da_mac[i].name);

	printf("\n");
}

int switch_l3_nexthop_da_config(int argc, char **argv)
{
	u64 address;
	u32 value[NEXT_HOP_DA_MAC_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= NEXT_HOP_DA_MAC_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = NEXT_HOP_DA_MAC + index * NEXT_HOP_DA_MAC_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < NEXT_HOP_DA_MAC_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_next_hop_da_mac, ARRAY_SIZE(switch_next_hop_da_mac), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < NEXT_HOP_DA_MAC_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n",
				__func__, index, address, value[1], value[0]);


	return 1;
ERR:
	switch_l3_nexthop_da_usage();
	return 0;
}

struct cmd_module switch_l3_cmd[] = {
	{"port_mac", switch_l3_port_mac_config, switch_l3_port_mac_usage},
	{"ingress_router", switch_l3_ingress_router_config, switch_l3_ingress_router_usage},
	{"hash_cal_v4", switch_l3_hash_cal_v4_config, switch_l3_hash_cal_v4_usage},
	{"hash_cal_v6", switch_l3_hash_cal_v6_config, switch_l3_hash_cal_v6_usage},
	{"hash_based", switch_l3_hash_based_config, switch_l3_hash_based_usage},
	{"routing_tcam", switch_l3_routing_tcam_config, switch_l3_routing_tcam_usage},
	{"lpm_result", switch_l3_lpm_result_config, switch_l3_lpm_result_usage},
	{"routing_default", switch_l3_routing_default_config, switch_l3_routing_default_usage},
	{"nexthop_table", switch_l3_nexthop_table_config, switch_l3_nexthop_table_usage},
	{"nexthop_pkt_mod", switch_l3_nexthop_pkt_mod_config, switch_l3_nexthop_pkt_mod_usage},
	{"nexthop_da", switch_l3_nexthop_da_config, switch_l3_nexthop_da_usage},
};

void switch_l3_usage(void)
{
	printf("Usage: npecmd switch l3 SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l3_cmd); i++)
		printf(" %s", switch_l3_cmd[i].name);

	printf("\n");
}

int switch_l3_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_l3_cmd); i++) {
		if (!strcasecmp(argv[0], switch_l3_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_l3_cmd[i].usage)
					switch_l3_cmd[i].usage();

				return 0;
			}

			if (switch_l3_cmd[i].func)
				return switch_l3_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_l3_usage();

	return 0;
}
