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

void dubhe1000_arp_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = ARP_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 23); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_ah_header_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = AH_HEADER_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 15); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_bootp_dhcp_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = BOOTP_DHCP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = readl(switch_base_addr + address);
	val |= (0x1F << 7); // port 0 ~ port 4 toCPU  bit 39
	writel(val, switch_base_addr + address);
}

void dubhe1000_capwap_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = CAPWAP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = readl(switch_base_addr + address);
	val |= (0x1F << 7); // port 0 ~ port 4 toCPU  bit 39
	writel(val, switch_base_addr + address);
}

void dubhe1000_dns_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = DNS_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 23); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_esp_header_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = ESP_HEADER_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 15); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_gre_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = GRE_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = readl(switch_base_addr + address);
	val |= (0x1F << 15); // port 0 ~ port 4 toCPU  bit 47

	writel(val, switch_base_addr + address);
}

void dubhe1000_1588_l2_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = L2_1588_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 23); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_1588_l4_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = L4_1588_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += (4 * 21);
	val = readl(switch_base_addr + address);

	val |= (0x1F << 7); // port 0 ~ port 4 toCPU  bit 679

	writel(val, switch_base_addr + address);
}

void dubhe1000_eapol_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = EAPOL_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 23); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_ike_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = IKE_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = readl(switch_base_addr + address);

	val |= (0x1F << 7); // port 0 ~ port 4 toCPU bit 39

	writel(val, switch_base_addr + address);
}

void dubhe1000_lacp_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = LACP_PACKET_DECODER_OPTIONS * 4;
	u32 val = 0;

	address += 4;
	val = readl(switch_base_addr + address);

	val |= (0x1F << 23); // port 0 ~ port 4 toCPU bit 55

	writel(val, switch_base_addr + address);
}

void dubhe1000_rarp_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = RARP_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 23); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_sctp_packet_decoder_options_set(struct dubhe1000_adapter *adapter)
{
	u64 address = SCTP_PACKET_DECODER_OPTIONS * 4;
	u32 val = readl(switch_base_addr + address);

	val |= (0x1F << 15); // port 0 ~ port 4 toCPU

	writel(val, switch_base_addr + address);
}

void dubhe1000_l2_packet_decoder_init(struct dubhe1000_adapter *adapter)
{
	dubhe1000_arp_packet_decoder_options_set(adapter);
	dubhe1000_ah_header_packet_decoder_options_set(adapter);
	dubhe1000_bootp_dhcp_packet_decoder_options_set(adapter);
	dubhe1000_capwap_packet_decoder_options_set(adapter);
	dubhe1000_dns_packet_decoder_options_set(adapter);
	dubhe1000_esp_header_packet_decoder_options_set(adapter);
	dubhe1000_gre_packet_decoder_options_set(adapter);
	dubhe1000_1588_l2_packet_decoder_options_set(adapter);
	dubhe1000_1588_l4_packet_decoder_options_set(adapter);
	dubhe1000_eapol_packet_decoder_options_set(adapter);

	dubhe1000_ike_packet_decoder_options_set(adapter);
	dubhe1000_lacp_packet_decoder_options_set(adapter);
	dubhe1000_rarp_packet_decoder_options_set(adapter);
	dubhe1000_sctp_packet_decoder_options_set(adapter);
}

void cls_npe_switch_config_delay_ms(void)
{
	/* msleep(0) may allow CPU time slots to other task holders */
	if (g_adapter->switch_cfg_delay)
		msleep(g_adapter->switch_cfg_delay);
}
EXPORT_SYMBOL(cls_npe_switch_config_delay_ms);

/**
 * dubhe1000_switch_init - Performs basic configuration of the switch.
 *
 */
s32 dubhe1000_switch_init(struct dubhe1000_adapter *adapter)
{
	int i;
	u32 val = 0;
	t_FloodingActionSendtoPort flooding_action;

	switch_base_addr = adapter->switch_regs;

	val = readl(switch_base_addr + 4 * CORE_VERSION);
	pr_info("Switch Core Version 0x%x\n", val);

	if (adapter->switch_pure_mod)
		return 0;

	// flooding -> send to CPU
	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		if (i == DUBHE2000_CPU_PORT)
			continue;

		memset(&flooding_action, 0, sizeof(flooding_action));
		rd_FloodingActionSendtoPort(adapter, i, &flooding_action);
		flooding_action.enable = 1;
		flooding_action.destPort = DUBHE2000_CPU_PORT;

		wr_FloodingActionSendtoPort(adapter, i, &flooding_action);
	}

	dubhe1000_l2_packet_decoder_init(adapter);
	dubhe2000_switch_l2_init(adapter);
	dubhe2000_switch_l3_init(adapter);

	dubhe2000_switch_ingress_aclN_init(adapter);
	dubhe2000_switch_nat_init(adapter);

	dubhe2000_tunnel_init_config(adapter);

	dubhe2000_switch_generic_init_config(adapter);

	dubhe2000_switch_pause_config(adapter);

	dubhe2000_switch_use_dwrr_scheduler(adapter);
	dubhe2000_switch_init_queue_weight(adapter);

	return 0;
}
