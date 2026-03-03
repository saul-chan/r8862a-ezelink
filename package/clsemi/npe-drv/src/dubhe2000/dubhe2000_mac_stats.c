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
/* dubhe1000_mac_stats.c
 * Shared functions for accessing and configuring the Statistics of mac(xgmac)
 */

#include "dubhe2000.h"
#include "dubhe2000_mac_stats.h"

/* TX */
struct dubhe1000_xgmac_stats_param xgmac_tx_stats[] = {
	{ "Tx_Octet_Count_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x814 },
	{ "Tx_Packet_Count_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x81c },
	{ "Tx_Broadcast_Packets_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x824 },
	{ "Tx_Multicast_Packets_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x82c },
	{ "Tx_64Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x834 },
	{ "Tx_65To127Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x83c },
	{ "Tx_128To255Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x844 },
	{ "Tx_256To511Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x84c },
	{ "Tx_512To1023Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x854 },
	{ "Tx_1024ToMaxOctets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x85c },
	{ "Tx_Unicast_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x864 },
	{ "Tx_Multicast_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x86c },
	{ "Tx_Broadcast_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x874 },
	{ "Tx_Underflow_Error_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x87c },
	{ "Tx_Octet_Count_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x884 },
	{ "Tx_Packet_Count_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x88c },
	{ "Tx_Pause_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x894 },
	{ "Tx_VLAN_Packets_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x89c },
	{ "Priority_Interrupt_Status", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x8cc },
	{ "Tx_Per_Priority_Status", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x8d0 },
	{ "Tx_Per_Priority_Pkt_Good_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x8d4 },
	{ "Tx_Per_Priority_PFC_Pkt_Good_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x8d8 },
	{ "Tx_Per_Priority_GPFC_Pkt_Good_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x8dc },
	{ "Tx_Per_Priority_Octet_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x8e0 },
	{ "Tx_Late_Collision_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0xa4c },
	{ "Tx_Excessive_Collision_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0xa50 },
	{ "Tx_Carrier_Error_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0xa54 },
	{ "Tx_Excessive_Deferral_Error", XGMAC_STATS_PARAM_TYPE_32_BIT, 0xa58 },
};

/* RX */
struct dubhe1000_xgmac_stats_param xgmac_rx_stats[] = {
	{ "Rx_Packet_Count_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x900 },
	{ "Rx_Octet_Count_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x908 },
	{ "Rx_Octet_Count_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x910 },
	{ "Rx_Broadcast_Packets_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x918 },
	{ "Rx_Multicast_Packets_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x920 },
	{ "Rx_CRC_Error_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x928 },
	{ "Rx_Runt_Error_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x930 },
	{ "Rx_Jabber_Error_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x934 },
	{ "Rx_Undersize_Packets_Good", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x938 },
	{ "Rx_Oversize_Packets_Good", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x93c },
	{ "Rx_64Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x940 },
	{ "Rx_65To127Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x948 },
	{ "Rx_128To255Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x950 },
	{ "Rx_256To511Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x958 },
	{ "Rx_512To1023Octets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x960 },
	{ "Rx_1024ToMaxOctets_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x968 },
	{ "Rx_Unicast_Packets_Good", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x970 },
	{ "Rx_Length_Error_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x978 },
	{ "Rx_OutofRange_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x980 },
	{ "Rx_Pause_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x988 },
	{ "Rx_FIFOOverflow_Packets", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x990 },
	{ "Rx_VLAN_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x998 },
	{ "Rx_Watchdog_Error_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9a0 },
	{ "Rx_Discard_Packets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x9ac },
	{ "Rx_Discard_Octets_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x9b4 },
	{ "Rx_Alignment_Error_Packets", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9bc },
	{ "Rx_Per_Priority_Status", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9d0 },
	{ "Rx_Per_Priority_Pkt_Good_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9d4 },
	{ "Rx_Per_Priority_Pkt_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9d8 },
	{ "Rx_Per_Priority_PFC_Pkt_Good_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9dc },
	{ "Rx_Per_Priority_Octet_Good_Bad", XGMAC_STATS_PARAM_TYPE_64_BIT, 0x9e0 },
	{ "Rx_Per_Priority_Discard_Good_Bad", XGMAC_STATS_PARAM_TYPE_32_BIT, 0x9e8 }
};

/* Please validate xgmac_index/option before call this function */
void dubhe1000_xgmac_stats_update_option(struct dubhe1000_adapter *adapter, u8 xgmac_index, u8 option,
					 struct dubhe1000_xgmac_stats_param **xgmac_stats)
{
	int i, size;
	void __iomem *address;

	if (option == XGMAC_STATS_OPTION_TX) {
		size = sizeof(xgmac_tx_stats) / sizeof(struct dubhe1000_xgmac_stats_param);
		*xgmac_stats = xgmac_tx_stats;
	} else {
		size = sizeof(xgmac_rx_stats) / sizeof(struct dubhe1000_xgmac_stats_param);
		*xgmac_stats = xgmac_rx_stats;
	}

	address = adapter->mac[xgmac_index]->ioaddr;

	for (i = 0; i < size; i++) {
		if ((*xgmac_stats)[i].type == XGMAC_STATS_PARAM_TYPE_32_BIT) {
			(*xgmac_stats)[i].value = readl(address + (*xgmac_stats)[i].addr);
		} else if ((*xgmac_stats)[i].type == XGMAC_STATS_PARAM_TYPE_64_BIT) {
			(*xgmac_stats)[i].value = readl(address + (*xgmac_stats)[i].addr + 4);
			(*xgmac_stats)[i].value = (*xgmac_stats)[i].value << 32;
			(*xgmac_stats)[i].value |= readl(address + (*xgmac_stats)[i].addr);
		}
	}
}

/* Please validate xgmac_index/option before call this function */
void dubhe1000_xgmac_stats_dump_option(struct dubhe1000_adapter *adapter, u8 xgmac_index, u8 option)
{
	int i, size;
	char fieldname[64] = { 0 };
	struct dubhe1000_xgmac_stats_param *xgmac_stats = NULL;

	if (option == XGMAC_STATS_OPTION_TX) {
		size = sizeof(xgmac_tx_stats) / sizeof(struct dubhe1000_xgmac_stats_param);
		pr_info("=========== Port%d XGMAC TX STATISTICS ===========\n", xgmac_index);
	} else {
		size = sizeof(xgmac_rx_stats) / sizeof(struct dubhe1000_xgmac_stats_param);
		pr_info("=========== Port%d XGMAC RX STATISTICS ===========\n", xgmac_index);
	}

	spin_lock(&adapter->stats64_lock);

	dubhe1000_xgmac_stats_update_option(adapter, xgmac_index, option, &xgmac_stats);

	for (i = 0; i < size; i++) {
		memset(fieldname, 0, sizeof(fieldname));
		snprintf(fieldname, sizeof(fieldname) - 1, "%s:", xgmac_stats[i].name);
		pr_info("%-48s    0x%llx(%llu)\n", fieldname, xgmac_stats[i].value, xgmac_stats[i].value);
	}

	spin_unlock(&adapter->stats64_lock);

	pr_info("=================================================\n");
}

void dubhe1000_xgmac_stats_dump_per_index(struct dubhe1000_adapter *adapter, u8 xgmac_index, u8 option)
{
	if (xgmac_index >= DUBHE1000_MAC_COUNT) {
		pr_info("[%s] invalid xgmac_index\n", __func__);
		return;
	}

	if (!adapter->mac[xgmac_index]) {
		pr_info("[%s] invalid xgmac base address\n", __func__);
		return;
	}

	pr_info("\n=============== Port%d XGMAC STATISTICS ===============\n", xgmac_index);

	if (option == XGMAC_STATS_OPTION_TX || option == XGMAC_STATS_OPTION_RX) {
		dubhe1000_xgmac_stats_dump_option(adapter, xgmac_index, option);
	} else {
		dubhe1000_xgmac_stats_dump_option(adapter, xgmac_index, XGMAC_STATS_OPTION_TX);
		dubhe1000_xgmac_stats_dump_option(adapter, xgmac_index, XGMAC_STATS_OPTION_RX);
	}

	pr_info("=======================================================\n");
}
