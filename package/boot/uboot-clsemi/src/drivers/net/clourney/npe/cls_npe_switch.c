// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2006 Intel Corporation. */

#include "cls_npe_switch.h"
#include "cls_npe.h"

void cls_router_port_macAddr_table_add(struct udevice *dev, u8 *macAddr);

void cls_arp_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = ARP_PACKET_DECODER_OPTIONS * 4;

	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_ah_header_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = AH_HEADER_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_bootp_dhcp_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = BOOTP_DHCP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU  bit 39
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_capwap_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = CAPWAP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU  bit 39
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_dns_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = DNS_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_esp_header_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = ESP_HEADER_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_gre_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = GRE_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU  bit 47
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_1588_l2_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = L2_1588_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_1588_l4_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = L4_1588_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += (4 * 21);
	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU  bit 679
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_eapol_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = EAPOL_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_ike_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = IKE_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 7);   // port 0 ~ port 4 toCPU bit 39
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_lacp_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = LACP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU bit 55
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_rarp_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = RARP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 23);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_sctp_packet_decoder_options_set(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = SCTP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	val = cls_eth_r32(priv_data->switch_regs, address);
	val |= (0x1F << 15);   // port 0 ~ port 4 toCPU
	cls_eth_w32(priv_data->switch_regs, address, val);
}

void cls_l2_protocol_init(struct udevice *dev)
{
	cls_arp_packet_decoder_options_set(dev);
	cls_ah_header_packet_decoder_options_set(dev);
	cls_bootp_dhcp_packet_decoder_options_set(dev);
	cls_capwap_packet_decoder_options_set(dev);
	cls_dns_packet_decoder_options_set(dev);
	cls_esp_header_packet_decoder_options_set(dev);
	cls_gre_packet_decoder_options_set(dev);
	cls_1588_l2_packet_decoder_options_set(dev);
	cls_1588_l4_packet_decoder_options_set(dev);
	cls_eapol_packet_decoder_options_set(dev);
	cls_ike_packet_decoder_options_set(dev);
	cls_lacp_packet_decoder_options_set(dev);
	cls_rarp_packet_decoder_options_set(dev);
	cls_sctp_packet_decoder_options_set(dev);
}

void cls_source_port_mac_learning_set(struct udevice *dev, u8 port_num, bool mac_learning)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	u64 address;

	address = SOURCE_PORT_TBL * 4 + 4 * port_num * SOURCE_PORT_NUM_ADDR_PER_ENTRY;
	val = cls_eth_r32(priv_data->switch_regs, address);

	// bit0 -  learningEn
	if (mac_learning)
		val |= 1;
	else
		val &= 0xFFFFFFFE;

	// The unknown sMAC address from this port will (NOT) be learned.
	cls_eth_w32(priv_data->switch_regs, address, val);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("source_port_mac_learning_set port %d mac_learning %d\n",
			port_num, mac_learning);
#endif
}

bool cls_l3_routing_default_table_get_free(struct udevice *dev, int *offset)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	bool ret = false;
	int k = 0;
	u32 val = 0;
	u64 address;

	for (k = 0; k < L3_ROUTING_DEFAULT_TBL_MAX; k++) {
		address = L3_ROUTING_DEFAULT_TBL * 4 + k * 4;
		val = cls_eth_r32(priv_data->switch_regs, address);

		if (val == 0) {
			ret = true;
			*offset = k;
			break;
		}
	}
	return ret;
}

void cls_ingress_router_table_op(struct udevice *dev, u32 val, bool add)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
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

	ret = cls_l3_routing_default_table_get_free(dev, &offset);
	if (!ret) {
		cls_eth_print("ERROR: ingress_router_table %s val %x no free entry!!!\n",
			 add?"add":"del",
			 val);
		return;
	}
	address = INGRESS_ROUTER_TBL * 4 + 4 * offset;

	cls_eth_w32(priv_data->switch_regs, address, data);
	tmp = cls_eth_r32(priv_data->switch_regs, address);

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("ingress_router_table %llx: %s on %d val %08x read %08x\n",
			 address,
			 add?"add":"del",
			 offset,
			 val, tmp);
#endif
}

void cls_l3_routing_default_table_op(struct udevice *dev, u8 nextHop, u8 drop, u8 toCPU, bool add)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	bool ret = false;
	int offset = 0;
	u64 address = L3_ROUTING_DEFAULT_TBL;
	u32 data = 0;
	u32 tmp = 0;

	if (add)
		data = (toCPU << 11) | (drop << 10) | nextHop;

	ret = cls_l3_routing_default_table_get_free(dev, &offset);
	if (!ret) {
		cls_eth_print("ERROR: l3_routing_default_table %s nextHop %d toCPU %d drop %d no free entry!!!\n",
			 add?"add":"del",
			 nextHop,
			 toCPU, drop);
		return;
	}
	address = L3_ROUTING_DEFAULT_TBL * 4 + offset * 4;

	cls_eth_w32(priv_data->switch_regs, address, data);
	tmp = cls_eth_r32(priv_data->switch_regs, address);

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("l3_routing_default_table(%llx) %s on %d nextHop %d toCPU %d drop %d read %08x data %08x\n",
			address,
			add ? "add" : "del",
			offset,
			nextHop,
			toCPU, drop, tmp, data);
#endif
}

/**
 * cls_switch_init - Performs basic configuration of the switch.
 *
 */
void cls_eth_switch_init(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	uchar env_macaddr[6];

	cls_l2_protocol_init(dev);

	// Disable HW unknown SMAC Learning from CPU Port
	// The unknown sMAC address from this port will NOT be learned.
	cls_source_port_mac_learning_set(dev, CLS_NPE_CPU_PORT, false);

	/* The ingress router table controls which packets are allowed to get
	 * access to this router.
	 * Config a Ingress Router Table: Accept IPv4
	 */
	cls_ingress_router_table_op(dev, 0x1D9001, true);

	/* The default router to be used if the dest lookup in L3 tables fails,
	 * i.e does not match either the LPM or the hash tables.
	 * Config a L3 route_default_table: nextHop=0, pktDrop=0, sendToCpu=1
	 */
	cls_l3_routing_default_table_op(dev, 0, 0, 1, true);

	if (eth_env_get_enetaddr("ethaddr", env_macaddr))
		cls_router_port_macAddr_table_add(dev, env_macaddr);
	else
		cls_router_port_macAddr_table_add(dev, priv_data->base_mac);

	val = cls_eth_r32(priv_data->switch_regs, 0);
	cls_eth_print("Switch Core Version 0x%x\n", val);
}

/* Router Port MAC Address: 16-Entry,  4 Number of Address per Entry
 *     macAddress:  0 ~ 47
 *     macMask:     48 ~ 95
 *     valid:       96
 *     vrf:         97 ~ 98
 */
static bool cls_switch_router_port_macAddr_table_get_free(struct udevice *dev, int *offset)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	bool ret = false;
	int k = 0, k2 = 0;
	u32 val[ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY] = {0};
	u8 used = 0;
	u64 address;

	for (k = 0; k < ROUTER_PORT_MACADDR_TABLE_MAX; k++) {
		address = ROUTER_PORT_MACADDR_TABLE * 4 + 4 * k * ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY;
		used = 0;
		for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++) {
			val[k2] = cls_eth_r32(priv_data->switch_regs, address + 4 * k2);
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

void cls_router_port_macAddr_table_add(struct udevice *dev, u8 *macAddr)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	bool ret = false;
	int offset = 0;
	int k = 0;
	u8 macMask2[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u64 address;
	u8 data[32] = {0};
	u32 *val = NULL;

	ret = cls_switch_router_port_macAddr_table_get_free(dev, &offset);
	if (!ret) {
		cls_eth_print("ERROR: Router Port macAddr Add non-vrf macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x no free entry!!!\n",
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
		cls_eth_w32(priv_data->switch_regs, address + 4 * k, *val);
	}

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("Router Port macAddr Add offset=%d non-vrf macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x\n",
				offset,
				macAddr[0], macAddr[1], macAddr[2], macAddr[3],
				macAddr[4], macAddr[5],
				macMask2[0], macMask2[1], macMask2[2], macMask2[3],
				macMask2[4], macMask2[5]);
#endif
}

void cls_router_port_macAddr_table_del(struct udevice *dev, u8 *macAddr, u8 *macMask)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
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
			val[k2] = cls_eth_r32(priv_data->switch_regs, address + k2 * 4);
			if (val[k2] != *data32) {
				found = 0;
				break;
			}
		}

		if (found == 1) {
			for (k2 = 0; k2 < ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY; k2++)
				cls_eth_w32(priv_data->switch_regs, address + 4 * k2, 0);

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
			cls_eth_print("Router Port MacAddr Del offset=%d non-vrf macAddr: %02x %02x %02x %02x %02x %02x macMask: %02x %02x %02x %02x %02x %02x\n",
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
void cls_ingress_port_packet_type_filter(struct udevice *dev, bool accept)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u64 address = INGRESS_PORT_PACKET_TYPE_FILTER * 4;
	u32 val, val_read;
	u8 port;

	if (accept)
		val = 0;// Default Value
	else
		val = 0x1FFFF;

	for (port = 0; port < INGRESS_PORT_PACKET_TYPE_FILTER_MAX - 1; port ++) {
		cls_eth_w32(priv_data->switch_regs, address + 4 * port, val);
		val_read = cls_eth_r32(priv_data->switch_regs, address + 4 * port);

		if (val != val_read)
			cls_eth_print("[%s] ingress port=%d op=%s val_read=0x%x\n",
						__func__,
						port,
						accept ? "accept" : "drop",
						val_read);
	}

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] all ingress ports op=%s val=0x%x\n",
						__func__,
						accept ? "accept" : "drop",
						val);
#endif
}
