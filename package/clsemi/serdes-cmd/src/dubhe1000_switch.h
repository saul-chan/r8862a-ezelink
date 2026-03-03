/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

#ifndef _DUBHE1000_ETH_SWITCH_H_
#define _DUBHE1000_ETH_SWITCH_H_
#include <stdint.h>
#include <stdbool.h>
#define uint uint32_t
#define u8   uint8_t
#define u16   uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define uchar uint8_t
#define DUBHE1000_CPU_PORT				5

/* Global Register */
#define SOURCE_PORT_TBL					0x1E164
#define SOURCE_PORT_TBL_MAX				6
#define SOURCE_PORT_NUM_ADDR_PER_ENTRY			8

/* L2 Protocol */
#define ARP_PACKET_DECODER_OPTIONS			0x1E129
#define AH_HEADER_PACKET_DECODER_OPTIONS		0x1E12E
#define BOOTP_DHCP_PACKET_DECODER_OPTIONS		0x1E446
#define CAPWAP_PACKET_DECODER_OPTIONS			0x1E448
#define DNS_PACKET_DECODER_OPTIONS			0x1E130
#define ESP_HEADER_PACKET_DECODER_OPTIONS		0x1E12F
#define GRE_PACKET_DECODER_OPTIONS			0x1E442
#define L2_1588_PACKET_DECODER_OPTIONS			0x1E12B
#define L4_1588_PACKET_DECODER_OPTIONS			0x1E540
#define EAPOL_PACKET_DECODER_OPTIONS			0x1E12C
#define IKE_PACKET_DECODER_OPTIONS			0x1E44A
#define LACP_PACKET_DECODER_OPTIONS			0x1E444
#define RARP_PACKET_DECODER_OPTIONS			0x1E12A
#define SCTP_PACKET_DECODER_OPTIONS			0x1E12D

/* L3 Table/Register */
#define ROUTER_PORT_MACADDR_TABLE			0x1E194
#define ROUTER_PORT_MACADDR_TABLE_MAX			16
#define ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY	8

#define L3_ROUTING_DEFAULT_TBL				0x1DE7A
#define L3_ROUTING_DEFAULT_TBL_MAX			4

#define INGRESS_ROUTER_TBL				0xE10A
#define INGRESS_ROUTER_TBL_MAX				4

#define INGRESS_PORT_PACKET_TYPE_FILTER			0x1E016
#define INGRESS_PORT_PACKET_TYPE_FILTER_MAX		6


void dubhe1000_eth_switch_init(uint32_t base_addr, u8 * macAddr);
void dubhe1000_router_port_macAddr_table_del(uint32_t base_addr,
		u8 *macAddr, u8 *macMask);
void dubhe1000_ingress_port_packet_type_filter(uint32_t base_addr, bool accept);
#endif
