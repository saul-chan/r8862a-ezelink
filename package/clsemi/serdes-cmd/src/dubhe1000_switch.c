// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2006 Intel Corporation. */

#include "dubhe1000_switch.h"
#include <strings.h>
#include <stddef.h>
#include "sys_hal.h"
void dubhe1000_router_port_macAddr_table_add(uint32_t base_addr, u8 *macAddr);

void dubhe1000_arp_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = ARP_PACKET_DECODER_OPTIONS * 4;

	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_ah_header_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = AH_HEADER_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_bootp_dhcp_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = BOOTP_DHCP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU  bit 39
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_capwap_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = CAPWAP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU  bit 39
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_dns_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = DNS_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_esp_header_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = ESP_HEADER_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_gre_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = GRE_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU  bit 47
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_1588_l2_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = L2_1588_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_1588_l4_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = L4_1588_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += (4 * 21);
	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU  bit 679
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_eapol_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = EAPOL_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_ike_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = IKE_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU bit 39
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_lacp_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = LACP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU bit 55
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_rarp_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = RARP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_sctp_packet_decoder_options_set(uint32_t base_addr)
{
	u64 address = SCTP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = dubhe1000_eth_r32(base_addr, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU
	dubhe1000_eth_w32(base_addr, address, val);
}

void dubhe1000_l2_protocol_init(uint32_t base_addr)
{
	dubhe1000_arp_packet_decoder_options_set(base_addr);
	dubhe1000_ah_header_packet_decoder_options_set(base_addr);
	dubhe1000_bootp_dhcp_packet_decoder_options_set(base_addr);
	dubhe1000_capwap_packet_decoder_options_set(base_addr);
	dubhe1000_dns_packet_decoder_options_set(base_addr);
	dubhe1000_esp_header_packet_decoder_options_set(base_addr);
	dubhe1000_gre_packet_decoder_options_set(base_addr);
	dubhe1000_1588_l2_packet_decoder_options_set(base_addr);
	dubhe1000_1588_l4_packet_decoder_options_set(base_addr);
	dubhe1000_eapol_packet_decoder_options_set(base_addr);
	dubhe1000_ike_packet_decoder_options_set(base_addr);
	dubhe1000_lacp_packet_decoder_options_set(base_addr);
	dubhe1000_rarp_packet_decoder_options_set(base_addr);
	dubhe1000_sctp_packet_decoder_options_set(base_addr);
}

void dubhe1000_source_port_mac_learning_set(uint32_t base_addr, u8 port_num, bool mac_learning)
{
	u32 val = 0;
	u64 address;

	address = SOURCE_PORT_TBL * 4 + 4 * port_num * SOURCE_PORT_NUM_ADDR_PER_ENTRY;
	val = dubhe1000_eth_r32(base_addr, address);

	// bit0 -  learningEn
	if (mac_learning)
		val |= 1;
	else
		val &= 0xFFFFFFFE;

	// The unknown sMAC address from this port will (NOT) be learned.
	dubhe1000_eth_w32(base_addr, address, val);
#ifdef DUBHE1000_ETH_DEBUG
	dubhe1000_eth_print("source_port_mac_learning_set port %d mac_learning %d\n",
			port_num, mac_learning);
#endif
}

bool dubhe1000_l3_routing_default_table_get_free(uint32_t base_addr, int *offset)
{
	bool ret = false;
	int k = 0;
	u32 val = 0;
	u64 address;

	for (k = 0; k < L3_ROUTING_DEFAULT_TBL_MAX; k++) {
		address = L3_ROUTING_DEFAULT_TBL * 4 + k * 4;
		val = dubhe1000_eth_r32(base_addr, address);

		if (val == 0) {
			ret = true;
			*offset = k;
			break;
		}
	}
	return ret;
}

void dubhe1000_ingress_router_table_op(uint32_t base_addr, u32 val, bool add)
{
	bool ret = false;
	int offset = 0;
	u64 address = INGRESS_ROUTER_TBL;
	u32 data = 0x1D9001;  // 0x3003;  // accept IPv4/6, IPv4/6 Hit Update
	u32 tmp = 0;

	if (add) {
		if (val != 0)
			data = val;
	} else {
		data = 0x1d8000;
	}

	ret = dubhe1000_l3_routing_default_table_get_free(base_addr, &offset);
	if (!ret) {
		dubhe1000_eth_print("ERROR: ingress_router_table %s val %x no free entry!!!\n",
			 add?"add":"del",
			 val);
		return;
	}
	address = INGRESS_ROUTER_TBL * 4 + 4 * offset;

	dubhe1000_eth_w32(base_addr, address, data);
	tmp = dubhe1000_eth_r32(base_addr, address);

#ifdef DUBHE1000_ETH_DEBUG
	dubhe1000_eth_print("ingress_router_table %llx: %s on %d val %08x read %08x\n",
			 address,
			 add?"add":"del",
			 offset,
			 val, tmp);
#endif
}

void dubhe1000_l3_routing_default_table_op(uint32_t base_addr, u8 nextHop, u8 drop, u8 toCPU, bool add)
{
	bool ret = false;
	int offset = 0;
	u64 address = L3_ROUTING_DEFAULT_TBL;
	u32 data = 0;
	u32 tmp = 0;

	if (add)
		data = (toCPU << 11) | (drop << 10) | nextHop;

	ret = dubhe1000_l3_routing_default_table_get_free(base_addr, &offset);
	if (!ret) {
		dubhe1000_eth_print("ERROR: l3_routing_default_table %s nextHop %d toCPU %d drop %d no free entry!!!\n",
			 add?"add":"del",
			 nextHop,
			 toCPU, drop);
		return;
	}
	address = L3_ROUTING_DEFAULT_TBL * 4 + offset * 4;

	dubhe1000_eth_w32(base_addr, address, data);
	tmp = dubhe1000_eth_r32(base_addr, address);

#ifdef DUBHE1000_ETH_DEBUG
	dubhe1000_eth_print("l3_routing_default_table(%llx) %s on %d nextHop %d toCPU %d drop %d read %08x data %08x\n",
			address,
			add ? "add" : "del",
			offset,
			nextHop,
			toCPU, drop, tmp, data);
#endif
}

/**
 * dubhe1000_switch_init - Performs basic configuration of the switch.
 *
 */
void dubhe1000_eth_switch_init(uint32_t base_addr, uint8_t * mac)
{
	u32 val = 0;
	//uchar env_macaddr[6];

	dubhe1000_l2_protocol_init(base_addr);

	// Disable HW unknown SMAC Learning from CPU Port
	// The unknown sMAC address from this port will NOT be learned.
	dubhe1000_source_port_mac_learning_set(base_addr, DUBHE1000_CPU_PORT, false);

	/* The ingress router table controls which packets are allowed to get
	 * access to this router.
	 * Config a Ingress Router Table: Accept IPv4
	 */
	dubhe1000_ingress_router_table_op(base_addr, 0x1D9001, true);

	/* The default router to be used if the dest lookup in L3 tables fails,
	 * i.e does not match either the LPM or the hash tables.
	 * Config a L3 route_default_table: nextHop=0, pktDrop=0, sendToCpu=1
	 */
	dubhe1000_l3_routing_default_table_op(base_addr, 0, 0, 1, true);

	if (mac)
		dubhe1000_router_port_macAddr_table_add(base_addr, mac);
	else
	val = dubhe1000_eth_r32(base_addr, 0);
	dubhe1000_eth_print("Switch Core Version 0x%x\n", val);
}

/* Router Port MAC Address: 16-Entry,  4 Number of Address per Entry
 *     macAddress:  0 ~ 47
 *     macMask:     48 ~ 95
 *     valid:       96
 *     vrf:         97 ~ 98
 */
static bool dubhe1000_switch_router_port_macAddr_table_get_free(uint32_t base_addr, int *offset)
{
	bool ret = false;
	int k = 0, k2 = 0;
	u32 val[ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY] = {0};
	u8 used = 0;
	u64 address;

	for (k = 0; k < ROUTER_PORT_MACADDR_TABLE_MAX; k++) {
		address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * k * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;
		used = 0;
		for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++) {
			val[k2] = dubhe1000_eth_r32(base_addr, address + 4 * k2);
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

void dubhe1000_router_port_macAddr_table_add(uint32_t base_addr, u8 *macAddr)
{
	bool ret = false;
	int offset = 0;
	int k = 0;
	u8 macMask2[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u64 address;
	u8 data[32] = {0};
	u32 *val = NULL;

	ret = dubhe1000_switch_router_port_macAddr_table_get_free(base_addr, &offset);
	if (!ret) {
		dubhe1000_eth_print("ERROR: Router Port macAddr Add non-vrf macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x no free entry!!!\n",
				macAddr[0], macAddr[1], macAddr[2], macAddr[3],
				macAddr[4], macAddr[5],
				macMask2[0], macMask2[1], macMask2[2], macMask2[3],
				macMask2[4], macMask2[5]);

		return;
	}
	address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * offset * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;

	for (k = 0; k < 6; k++) {
		data[k] = macAddr[5 - k];
		data[k + 6] = macMask2[5 - k];
	}
	// bit150(valid) = 1
	data[18] = 1 << 6;

	for (k = 0; k <  ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k++) {
		val = (u32 *)&data[k * 4];
		dubhe1000_eth_w32(base_addr, address + 4 * k, *val);
	}

#ifdef DUBHE1000_ETH_DEBUG
	dubhe1000_eth_print("Router Port macAddr Add offset=%d non-vrf macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x\n",
				offset,
				macAddr[0], macAddr[1], macAddr[2], macAddr[3],
				macAddr[4], macAddr[5],
				macMask2[0], macMask2[1], macMask2[2], macMask2[3],
				macMask2[4], macMask2[5]);
#endif
}

void dubhe1000_router_port_macAddr_table_del(uint32_t base_addr, u8 *macAddr, u8 *macMask)
{
	int k = 0, k2 = 0;
	u32 val[ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY] = {0};
	u32 *data32 = NULL;
	u8 data[32] = {0};
	u8 found = 1;
	u64 address = ROUTER_PORT_MACADDR_TABLE;
	u8 macMask2[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	// data shoud be same in the *add()
	for (k = 0; k < 6; k++) {
		data[k] = macAddr[5 - k];
		data[k + 6] = macMask2[5 - k];
	}
	data[18] = 1 << 6;

	for (k = 0; k < ROUTER_PORT_MACADDR_TABLE_MAX; k++) {
		address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * k * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;
		found = 1;
		for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++) {
			data32 = (u32 *)&data[k2 * 4];
			val[k2] = dubhe1000_eth_r32(base_addr, address + k2 * 4);
			if (val[k2] != *data32) {
				found = 0;
				break;
			}
		}

		if (found == 1) {
			for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++)
				dubhe1000_eth_w32(base_addr, address + 4 * k2, 0);

#ifdef DUBHE1000_ETH_DEBUG
			dubhe1000_eth_print("Router Port MacAddr Del offset=%d non-vrf macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x\n",
					k,
					macAddr[0], macAddr[1], macAddr[2], macAddr[3],
					macAddr[4], macAddr[5],
					macMask2[0], macMask2[1], macMask2[2], macMask2[3],
					macMask2[4], macMask2[5]);
#endif
			break;
		}
	}
}

/* This function is used to configure which packet types that are to be dropped or allowed on each source port. */
void dubhe1000_ingress_port_packet_type_filter(uint32_t base_addr, bool accept)
{
	u64 address = INGRESS_PORT_PACKET_TYPE_FILTER * 4;
	u32 val, val_read;
	u8 port;
	
	if (accept)
		val = 0;// Default Value
	else
		val = 0x1FFFF;

	for (port = 0; port < INGRESS_PORT_PACKET_TYPE_FILTER_MAX - 1; port ++) {
		dubhe1000_eth_w32(base_addr, address + 4 * port, val);
		val_read = dubhe1000_eth_r32(base_addr, address + 4 * port);

		if (val != val_read)
			dubhe1000_eth_print("[%s] ingress port=%d op=%s val_read=0x%x\n",
						__func__,
						port,
						accept ? "accept" : "drop",
						val_read);
	}

#ifdef DUBHE1000_ETH_DEBUG
	dubhe1000_eth_print("[%s] all ingress ports op=%s val=0x%x\n",
						__func__,
						accept ? "accept" : "drop",
						val);
#endif
}
