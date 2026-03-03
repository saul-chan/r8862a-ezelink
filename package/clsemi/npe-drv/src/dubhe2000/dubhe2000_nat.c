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

/* dubhe2000_nat.c
 * Shared functions for accessing and configuring the Switch NAT
 */

#include "dubhe2000_switch_regs.h"
#include "dubhe2000_switch.h"

u32 ingressnatop_status[IngressNATOperation_nr_entries / 32];

/*
 * NOTE: ACL0 must pointer NAT_OPERATION but do nothing for ipv6 router.
 * Because we need ipv6 route hit counters in dubhe100/dubhe2000.
 * And LAN<->WAN without Ingress NAT_operation will redirect to CPUPORT in NAT_ACTION_TABLE
 * So a ipv6 router also needs a Ingres NAT OPERATION index, but not nat process.
 * the bits in ingressnatop_status will be indicated if Ingres NAT OPERATION is used
 */
int dubhe2000_ingress_nat_op_get_free_index(void)
{
	int i, m, n;

	for (i = 0; i < IngressNATOperation_nr_entries; i++) {
		m = i / 32;
		n = i % 32;
		if (!(ingressnatop_status[m] & (1U << n)))
			return i;
	}

	return -TC_ERR_NO_FREE_NAT;
}

void dubhe2000_ingress_nat_op_status_op(u16 index, bool is_add)
{
	int m = index / 32;
	int n = index % 32;
	u32 temp = 1U << n;

	if (is_add)
		ingressnatop_status[m] |= temp;
	else
		ingressnatop_status[m] &= (~temp);
}

void dubhe2000_nat_dump_ingress_op_status(void)
{
	int i;

	pr_info("[%s]:\n", __func__);

	for (i = 0; i < ARRAY_SIZE(ingressnatop_status); i++) {
		if (ingressnatop_status[i])
			pr_info("Ingress_NAT_Operation[%d - %d]: status 0x%x\n",
						32 * i, 32 * i + 31, ingressnatop_status[i]);
	}
}

// please check arg before call this function
void dubhe2000_ingress_nat_add_op_config(const struct hw_nat_act *act, u16 index)
{
	t_IngressNATOperation nat_op;

	if (index >= IngressNATOperation_nr_entries) { //never happened
		pr_err("[%s] invalid nat index\n", __func__);
		return;
	}

	memset(&nat_op, 0, sizeof(nat_op));
	nat_op.replaceSrc = act->replace_src;
	nat_op.replaceIP = act->replace_addr;
	nat_op.replaceL4Port = act->replace_port;
	nat_op.ipAddress = ntohl(act->addr);
	nat_op.port = ntohs(act->port);

	wr_IngressNATOperation(g_adapter, index, &nat_op);

	dubhe2000_ingress_nat_op_status_op(index, true);
}

void dubhe2000_ingress_nat_del_op_config(u16 index)
{
	t_IngressNATOperation nat_op;

	if (index >= IngressNATOperation_nr_entries) { //never happened
		pr_err("[%s] invalid nat index\n", __func__);
		return;
	}

	memset(&nat_op, 0, sizeof(nat_op));

	wr_IngressNATOperation(g_adapter, index, &nat_op);

	dubhe2000_ingress_nat_op_status_op(index, false);
}

void dubhe2000_nat_action_table_config_entry(int index, u8 value)
{
	t_NATActionTable nat_action;

	if (index >= NATActionTable_nr_entries)
		return;

	nat_action.action = value & 0x3;

	wr_NATActionTable(g_adapter, index, &nat_action);
}

void dubhe2000_nat_action_table_init_config(struct dubhe1000_adapter *adapter)
{
	u8 value;
	int i;
	t_NATActionTableForceOriginalPacket original_pkt;

	//NAT Action Table Force Original Packet
	original_pkt.reasonOne = 1;
	original_pkt.reasonTwo = 1;
	wr_NATActionTableForceOriginalPacket(adapter, &original_pkt);

	/* NatActionTable should ensure that l3 routing must work with IngressNAT */
	for (i = 0; i < NATActionTable_nr_entries; i++) {
		value = 0;
		//routed and non-ingress-nat should be forward to CpuPort
		if (((i & 0xc) == 0x4) && ((i & 0x10) == 0x0)) {
			if ((i & 0x3) == 0x2) // Lan->wan
				value = 1;
			else if ((i & 0x3) == 0x1) // Wan ->Lan
				value = 2;
		}

		dubhe2000_nat_action_table_config_entry(i, value);
	}
}

void dubhe2000_switch_nat_init(struct dubhe1000_adapter *adapter)
{
	t_NATAddEgressPortforNATCalculation nat_add_eport;
	t_EgressPortNATState eport_nat_state;
	t_SourcePortTable source_port;
	int i;

	//step1: init NAT ACTION TABLE
	dubhe2000_nat_action_table_init_config(adapter);

	//step2: init  NAT_Add_Egress_Port_for_NAT_Calculation
	memset(&nat_add_eport, 0, sizeof(nat_add_eport));
	nat_add_eport.dontAddIngress = 1;
	nat_add_eport.dontAddEgress = 1;
	wr_NATAddEgressPortforNATCalculation(adapter, &nat_add_eport);

	//step3: Egress Port NAT State
	memset(&eport_nat_state, 0, sizeof(eport_nat_state));
	rd_EgressPortNATState(adapter, &eport_nat_state);
	eport_nat_state.portState = 1 << adapter->wan_port;
	wr_EgressPortNATState(adapter, &eport_nat_state);

	//step4: Source Port NAT State
	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		memset(&source_port, 0, sizeof(source_port));
		rd_SourcePortTable(adapter, i, &source_port);

		// natPortState
		if (i == adapter->wan_port)
			source_port.natPortState = 1;
		else
			source_port.natPortState = 0;

		// natActionTableEnable
		source_port.natActionTableEnable = 1;

		wr_SourcePortTable(adapter, i, &source_port);
	}

	//step5: clear switch_ipv6_nat_used
	memset(ingressnatop_status, 0, sizeof(ingressnatop_status));
}
