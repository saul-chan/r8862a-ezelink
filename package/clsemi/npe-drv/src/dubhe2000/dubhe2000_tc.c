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

/* dubhe1000_tc.c
 * Shared functions for Traffic Control
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#include "dubhe2000.h"

#define TC_ERR(fmt, ...) do { if (__ratelimit(&rl)) pr_err(fmt, ##__VA_ARGS__); } while (0)

/* fixed config:
 *     router_port_macaddr index 0/vrf 0 is for bridge's mac(default is br-lan)
 *     router_port_macaddr index 1/vrf 1 is for wan's mac(default is eth0)
 */
u8 dubhe2000_tc_get_vrf_by_nat_type(bool is_snat)
{
	if (is_snat)
		return 0;
	else
		return 1;
}

u8 dubhe2000_tc_get_vrf_by_inport(u8 inport)
{
	if (inport != g_adapter->wan_port)
		return 0;
	else
		return 1;
}

int dubhe2000_tc_get_eth_port_by_index(int index)
{
	struct net_device *netdev;
	int port;

	netdev = __dev_get_by_index(&init_net, index);
	if (!netdev)
		return -EINVAL;

	port = dubhe1000_if_port_to_switch_port(netdev->if_port);

	return port;
}

/* npe acl index
 * bit30 - bit24 | bit23 - bit20 | bit19 - bit16 | bit15 - bit0 |
 *      reserved |  acl engine   |   acl type    |  table index |
 */

#define DUBHE1000_TC_ACL_INDEX_SHIFT	0
#define DUBHE1000_TC_ACL_INDEX_MASK	0xFFFF
#define DUBHE1000_TC_ACL_TYPE_SHIFT	16
#define DUBHE1000_TC_ACL_TYPE_MASK	0xF
#define DUBHE1000_TC_ACL_ENGINE_SHIFT	20
#define DUBHE1000_TC_ACL_ENGINE_MASK	0xF

static int dubhe2000_tc_get_npe_acl_index(u8 aclN, u8 tbl_type, u16 index)
{
	int npe_acl_index = 0;

	npe_acl_index = (index & DUBHE1000_TC_ACL_INDEX_MASK) << DUBHE1000_TC_ACL_INDEX_SHIFT;
	npe_acl_index += (tbl_type & DUBHE1000_TC_ACL_TYPE_MASK) << DUBHE1000_TC_ACL_TYPE_SHIFT;
	npe_acl_index += (aclN & DUBHE1000_TC_ACL_ENGINE_MASK) << DUBHE1000_TC_ACL_ENGINE_SHIFT;

	return npe_acl_index;
}

static void dubhe2000_tc_get_acl_hash_type(int npe_index, u8 *aclN, u8 *tbl_type, u16 *index)
{
	*aclN = (npe_index >> DUBHE1000_TC_ACL_ENGINE_SHIFT) & DUBHE1000_TC_ACL_ENGINE_MASK;
	*tbl_type = (npe_index >> DUBHE1000_TC_ACL_TYPE_SHIFT) & DUBHE1000_TC_ACL_TYPE_MASK;
	*index = (npe_index >> DUBHE1000_TC_ACL_INDEX_SHIFT) & DUBHE1000_TC_ACL_INDEX_MASK;
}

static void dubhe2000_conf_egress_user_tpid(__be16 tpid)
{
	t_EgressEthernetTypeforVLANtag data;

	data.typeValue = tpid;
	wr_EgressEthernetTypeforVLANtag(g_adapter, &data);
}

static void dubhe2000_conf_inner_vlan(t_NextHopPacketModifications *nexthop_pkt_mod, const struct push_vlan_info *vlan)
{
	if (!vlan->vid)
		return;

	nexthop_pkt_mod->innerVlanAppend = 1;
	nexthop_pkt_mod->innerPcpSel = 1;
	nexthop_pkt_mod->innerCfiDeiSel = 1;
	nexthop_pkt_mod->innerVid = vlan->vid;
	nexthop_pkt_mod->innerPcp = vlan->pcp;
	nexthop_pkt_mod->innerCfiDei = vlan->dei;
	if (vlan->tpid == htons(ETH_P_8021Q))
		nexthop_pkt_mod->innerEthType = 0;
	else if (vlan->tpid == htons(ETH_P_8021AD))
		nexthop_pkt_mod->innerEthType = 1;
	else {
		nexthop_pkt_mod->innerEthType = 2;
		if (g_adapter->egress_user_tpid && g_adapter->egress_user_tpid != vlan->tpid)
			pr_warn("egress user tpid changed: %04x -> %04x\n", g_adapter->egress_user_tpid, vlan->tpid);
		dubhe2000_conf_egress_user_tpid(vlan->tpid);
	}
}

static void dubhe2000_conf_outer_vlan(t_NextHopPacketModifications *nexthop_pkt_mod, const struct push_vlan_info *vlan)
{
	if (!vlan->vid)
		return;

	nexthop_pkt_mod->outerVlanAppend = 1;
	nexthop_pkt_mod->outerPcpSel = 1;
	nexthop_pkt_mod->outerCfiDeiSel = 1;
	nexthop_pkt_mod->outerVid = vlan->vid;
	nexthop_pkt_mod->outerPcp = vlan->pcp;
	nexthop_pkt_mod->outerCfiDei = vlan->dei;
	if (vlan->tpid == htons(ETH_P_8021Q))
		nexthop_pkt_mod->outerEthType = 0;
	else if (vlan->tpid == htons(ETH_P_8021AD))
		nexthop_pkt_mod->outerEthType = 1;
	else {
		nexthop_pkt_mod->outerEthType = 2;
		if (g_adapter->egress_user_tpid && g_adapter->egress_user_tpid != vlan->tpid)
			pr_warn("egress user tpid changed: %04x -> %04x\n", g_adapter->egress_user_tpid, vlan->tpid);
		dubhe2000_conf_egress_user_tpid(vlan->tpid);
	}
}

static struct {
	u16	refcnt;
	__be16	pppoe_sid;
	u8	is_ipv4;
} tunnel_entry_tbl[TunnelEntryInstructionTable_nr_entries];

static struct {
	u16	refcnt;
	__be16	pppoe_sid;
	u8	is_ipv4;
} tunnel_exit_tbl[SecondTunnelExitLookupTCAM_nr_entries];

static struct {
	int8_t	tunnel_entry_idx;
	int8_t	tunnel_exit_idx;
} nexthop_tunnel_map[NextHopTable_nr_entries];

static int dubhe2000_nexthop_map_tunnel(int nexthop_idx, __be16 pppoe_sid, bool is_ipv4)
{
	int tunnel_idx;

	for (tunnel_idx = 0; tunnel_idx < TunnelEntryInstructionTable_nr_entries; ++tunnel_idx) {
		if (tunnel_entry_tbl[tunnel_idx].pppoe_sid == pppoe_sid &&
		    tunnel_entry_tbl[tunnel_idx].is_ipv4 == is_ipv4) {
			nexthop_tunnel_map[nexthop_idx].tunnel_entry_idx = tunnel_idx;
			tunnel_entry_tbl[tunnel_idx].refcnt++;
			break;
		}
	}

	for (tunnel_idx = 0; tunnel_idx < SecondTunnelExitLookupTCAM_nr_entries; ++tunnel_idx) {
		if (tunnel_exit_tbl[tunnel_idx].pppoe_sid == pppoe_sid &&
		    tunnel_exit_tbl[tunnel_idx].is_ipv4 == is_ipv4) {
			nexthop_tunnel_map[nexthop_idx].tunnel_exit_idx = tunnel_idx;
			tunnel_exit_tbl[tunnel_idx].refcnt++;
			break;
		}
	}

	if (nexthop_tunnel_map[nexthop_idx].tunnel_entry_idx < 0) {
		tunnel_idx = dubhe2000_tunnel_entry_config_session(g_adapter, ntohs(pppoe_sid), is_ipv4);
		if (tunnel_idx < 0)
			return -TC_ERR_NO_FREE_TUNNEL_ENTRY;
		tunnel_entry_tbl[tunnel_idx].pppoe_sid = pppoe_sid;
		tunnel_entry_tbl[tunnel_idx].is_ipv4 = is_ipv4;
		tunnel_entry_tbl[tunnel_idx].refcnt = 1;
		nexthop_tunnel_map[nexthop_idx].tunnel_entry_idx = tunnel_idx;
	}

	if (nexthop_tunnel_map[nexthop_idx].tunnel_exit_idx < 0) {
		tunnel_idx = dubhe2000_tunnel_exit_config_session(g_adapter, ntohs(pppoe_sid), is_ipv4);
		if (tunnel_idx < 0)
			return -TC_ERR_NO_FREE_TUNNEL_EXIT;
		tunnel_exit_tbl[tunnel_idx].pppoe_sid = pppoe_sid;
		tunnel_exit_tbl[tunnel_idx].is_ipv4 = is_ipv4;
		tunnel_exit_tbl[tunnel_idx].refcnt = 1;
		nexthop_tunnel_map[nexthop_idx].tunnel_exit_idx = tunnel_idx;
	}

	return 0;
}

static void dubhe2000_nexthop_unmap_tunnel(int nexthop_idx)
{
	int tunnel_idx;

	if (nexthop_tunnel_map[nexthop_idx].tunnel_entry_idx >= 0) {
		tunnel_idx = nexthop_tunnel_map[nexthop_idx].tunnel_entry_idx;
		if (tunnel_entry_tbl[tunnel_idx].refcnt) {
			tunnel_entry_tbl[tunnel_idx].refcnt--;
			if (tunnel_entry_tbl[tunnel_idx].refcnt == 0) {
				dubhe2000_tunnel_entry_delete_session(g_adapter, tunnel_idx);
				memset(&tunnel_entry_tbl[tunnel_idx], 0, sizeof(tunnel_entry_tbl[tunnel_idx]));
			}
		}
	}

	if (nexthop_tunnel_map[nexthop_idx].tunnel_exit_idx >= 0) {
		tunnel_idx = nexthop_tunnel_map[nexthop_idx].tunnel_exit_idx;
		if (tunnel_exit_tbl[tunnel_idx].refcnt) {
			tunnel_exit_tbl[tunnel_idx].refcnt--;
			if (tunnel_exit_tbl[tunnel_idx].refcnt == 0) {
				dubhe2000_tunnel_exit_delete_session(g_adapter, tunnel_idx);
				memset(&tunnel_exit_tbl[tunnel_idx], 0, sizeof(tunnel_exit_tbl[tunnel_idx]));
			}
		}
	}
}

int dubhe2000_tc_nat_add_router_config(bool is_snat, const struct hw_nat_key *key, const struct hw_nat_act *act,
				       u8 inport, u8 outport)
{
	struct clsemi_flow_entry *flow = container_of(key, struct clsemi_flow_entry, nat_key);
	/* switch routing configuration process
	 * 1. Router Port MAC Address / Ingress Router Table
	 * 2. Hash Based L3 Routing Table
	 * 3. Next Hop Table
	 * 4. Next Hop Packet Modifications
	 * 5. Next Hop DA MAC
	 * 6. Router Port Egress SA MAC Address
	 * Note:
	 *	(1) In step1, br0's mac/wan's mac has been already updated with vrf 0/1 when if up;
	 *	    And Ingress Router Table has been already updated with acceptIPv4/acceptIPv6
	 *	(2) In step2, L3 Routing TCAM/L3 LPM Result are Not used
	 *	(3) In step3/4/5, three tables are the same index
	 *	(4) In step6, sa should be bind with nat_type.(wan's mac in snat, br's mac in dnat)
	 */
	t_HashBasedL3RoutingTable hashbased_l3_routing;
	t_NextHopTable nexthop_tbl;
	t_NextHopPacketModifications nexthop_pkt_mod;
	t_NextHopDAMAC nexthop_damac;
	t_RouterPortEgressSAMACAddress router_eport_smac;
	int ret, i, l3_index, nextHop_index;
	u8 vrf, dmac[ETH_ALEN], smac[ETH_ALEN];
	u16 nextHop_counter;
	u32 dest_ipaddr;
	u64 dest_mac, src_mac;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	for (i = 0; i < ETH_ALEN; i++)
		dmac[i] = act->dst_mac[ETH_ALEN - i - 1];

	memcpy(&dest_mac, dmac, ETH_ALEN);

	if (is_snat) // SNAT: Ingress_ACL(enableUpdateIp:0) -> ROUTER(origin DIP) vrf1 -> INGRESS_NAT
		dest_ipaddr = ntohl(key->dst_addr);
	else // DNAT: INGRESS_ACL(enableUpdateIp:1) -> ROUTER(newIpValue) vrf0 -> INGRESS_NAT
		dest_ipaddr = ntohl(act->addr);

	vrf = dubhe2000_tc_get_vrf_by_nat_type(is_snat);

	l3_index = dubhe1000_hash_based_l3_routing_tbl_get_existed_v4_index(g_adapter, vrf, dest_ipaddr);
	if (l3_index < 0) {
		l3_index = dubhe1000_hash_based_l3_routing_tbl_get_new_v4_index(g_adapter, vrf, dest_ipaddr);
		if (l3_index < 0) {
			TC_ERR("[%s] no free l3 index\n", __func__);

			return -TC_ERR_NO_FREE_L3;
		}

		nextHop_index = dubhe1000_next_hop_tbl_get_new_index(g_adapter, dest_mac);
		if (nextHop_index < 0) {
			TC_ERR("[%s] no free nextHop_index\n", __func__);

			return -TC_ERR_NO_FREE_NEXTHOP_TBL;
		}
	} else {
		switch_l3_hash_counter[l3_index] = switch_l3_hash_counter[l3_index] + 1;

		rd_HashBasedL3RoutingTable(g_adapter, l3_index, &hashbased_l3_routing);
		nextHop_index = hashbased_l3_routing.nextHopPointer;
		dubhe2000_l3_nexthop_counter_op(nextHop_index, dest_mac, 1);

		flow->npe_nexthop_index = nextHop_index;
		if (g_adapter->tc_debug)
			pr_info("[%s] l3_index 0x%x exised, nexthop_index 0x%x\n",
				__func__, l3_index, nextHop_index);

		return 0;
	}

	flow->npe_nexthop_index = nextHop_index;
	if (g_adapter->tc_debug)
		pr_info("[%s] ipaddr=0x%x vrf=%d l3_index=0x%x nexthop_index=0x%x inport=%d outport=%d\n",
			__func__, dest_ipaddr, vrf, l3_index, nextHop_index, inport, outport);

	dubhe2000_l3_nexthop_counter_op(nextHop_index, dest_mac, 1);
	nextHop_counter = switch_nexthop_info[nextHop_index] >> 48;
	if (nextHop_counter == 1) {
		//config Next_Hop_Table
		memset(&nexthop_tbl, 0, sizeof(nexthop_tbl));
		nexthop_tbl.validEntry = 1;
		nexthop_tbl.nextHopPacketMod = nextHop_index;
		nexthop_tbl.l2Uc = 1;
		nexthop_tbl.destPortormcAddr = outport;
		nexthop_tunnel_map[nextHop_index].tunnel_entry_idx = -1;
		nexthop_tunnel_map[nextHop_index].tunnel_exit_idx = -1;
		if (act->push_pppoe) {
			ret = dubhe2000_nexthop_map_tunnel(nextHop_index, act->pppoe_sid, true);
			if (ret < 0) {
				TC_ERR("%s: failed to bind tunnel\n", __func__);
				return ret;
			}
			nexthop_tbl.tunnelEntry = 1;
			nexthop_tbl.tunnelEntryPtr = nexthop_tunnel_map[nextHop_index].tunnel_entry_idx;
		}
		wr_NextHopTable(g_adapter, nextHop_index, &nexthop_tbl);

		//Next Hop DA MAC
		memset(&nexthop_damac, 0, sizeof(nexthop_damac));
		nexthop_damac.daMac = dest_mac;
		wr_NextHopDAMAC(g_adapter, nextHop_index, &nexthop_damac);

		//Config Next_Hop_Packet_Modifications
		memset(&nexthop_pkt_mod, 0, sizeof(nexthop_pkt_mod));
		nexthop_pkt_mod.valid = 1;
		dubhe2000_conf_inner_vlan(&nexthop_pkt_mod, &act->inner_vlan);
		dubhe2000_conf_outer_vlan(&nexthop_pkt_mod, &act->outer_vlan);
		wr_NextHopPacketModifications(g_adapter, nextHop_index, &nexthop_pkt_mod);
	} else {//check nexthop reuse
		memset(&nexthop_tbl, 0, sizeof(nexthop_tbl));
		rd_NextHopTable(g_adapter, nextHop_index, &nexthop_tbl);

		memset(&nexthop_damac, 0, sizeof(nexthop_damac));
		rd_NextHopDAMAC(g_adapter, nextHop_index, &nexthop_damac);

		memset(&nexthop_pkt_mod, 0, sizeof(nexthop_pkt_mod));
		rd_NextHopPacketModifications(g_adapter, nextHop_index, &nexthop_pkt_mod);

		if ((nexthop_damac.daMac != dest_mac) || (nexthop_tbl.destPortormcAddr != outport) ||
			(nexthop_tbl.validEntry != 1) || (nexthop_pkt_mod.valid != 1))
			TC_ERR("[%s] nexthop reuse ERROR\n", __func__);
	}

	//config Hash_Based_L3_Routing_Table
	memset(&hashbased_l3_routing, 0, sizeof(hashbased_l3_routing));
	hashbased_l3_routing.ipVersion = 0; //IPV4
	hashbased_l3_routing.mpls = 0;
	hashbased_l3_routing.vrf = vrf;
	memcpy(hashbased_l3_routing.destIPAddr, &dest_ipaddr, sizeof(dest_ipaddr));
	hashbased_l3_routing.nextHopPointer = nextHop_index;
	wr_HashBasedL3RoutingTable(g_adapter, l3_index, &hashbased_l3_routing);
	switch_l3_hash_counter[l3_index] = switch_l3_hash_counter[l3_index] + 1;

	/*
	 * Note: this register is per vrf and can be config when
	 *	a. when br/wan up, br0's vrf's sa is wan's mac, Another case is the opposite
	 *	b. when add_nat_api called. 'struct hw_nat_act' contains smac (selected)
	 */
	if (!dubhe1000_check_u8_array_is_zero(act->src_mac, ETH_ALEN)) {
		rd_RouterPortEgressSAMACAddress(g_adapter, vrf, &router_eport_smac);

		router_eport_smac.selectMacEntryPortMask |= (1 << outport);

		for (i = 0; i < ETH_ALEN; i++)
			smac[i] = act->src_mac[ETH_ALEN - i - 1];

		memcpy(&src_mac, smac, ETH_ALEN);
		router_eport_smac.altMacAddress = src_mac;
		wr_RouterPortEgressSAMACAddress(g_adapter, vrf, &router_eport_smac);
	}

	return 0;
}

int dubhe2000_tc_nat_entry_parse_and_check(const struct hw_nat_key *key, const struct hw_nat_act *act, u8 *inport,
					   u8 *outport, u8 *tbl_type, u16 *acl_idx, u16 *nat_idx, u8 *acl_compareData)
{
	int tmp;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	tmp = dubhe2000_tc_get_eth_port_by_index(key->in_ifindex);
	if (tmp < 0) {
		TC_ERR("[%s] invalid in_ifindex\n", __func__);

		return -TC_ERR_INVALID_ARG;
	}
	*inport = tmp;

	*outport = dubhe1000_if_port_to_switch_port(act->out_dev->if_port);

	// hashkey calculate
	/* ipv4 nat rulessetup: sip/dip/l4_protocol/sport/dport */
	dubhe1000_ingress_acl0_ipv4_nat_hashkey_build(acl_compareData, ntohl(key->src_addr), ntohl(key->dst_addr),
						      ntohs(key->src_port), ntohs(key->dst_port), key->ip_proto,
						      *inport);

	// check if already existing entry
	tmp = dubhe2000_ingress_aclN_get_existed_index(0, acl_compareData, 0, tbl_type);
	if (tmp >= 0) {
		*acl_idx = tmp;

		return -TC_ERR_EXISTE_ACL; // we should delete it and add it again
	}
	// get free acl tbl_type and acl_idx
	tmp = dubhe2000_ingress_aclN_get_free_index(0, acl_compareData, tbl_type);
	if (tmp < 0)
		return -TC_ERR_NO_FREE_ACL;
	*acl_idx = tmp;

	// get free ingress nat op idx
	tmp = dubhe2000_ingress_nat_op_get_free_index();
	if (tmp < 0)
		return -TC_ERR_NO_FREE_NAT;
	*nat_idx = tmp;

	return 0;
}

//ingress acl0 config nat
int dubhe2000_tc_ingress_acl0_config_nat(bool is_snat, const struct hw_nat_key *key, const struct hw_nat_act *act,
					 u8 tbl_type, u16 acl_index, u16 nat_index, u8 *acl_compareData)
{
	struct clsemi_flow_entry *flow = container_of(key, struct clsemi_flow_entry, nat_key);
	/*
	 * ingress acl0 config dnat process
	 * 1. useAclN/rules pointer/rules setup/acl_mask have been configed in dubhe1000_switch_init()
	 * 2. build hashkey and calculate hashvalue
	 * 3. get free ingress acl0 index and ingress nat op index
	 * 4. config nat and acl
	 */

	flow->npe_nat_index = nat_index;
	if (g_adapter->tc_debug) {
		pr_info("[%s] %s acl_index=0x%x tbl_type=%d nat_index=%d\n", __func__, is_snat ? "snat" : "dnat",
			acl_index, tbl_type, nat_index);

		pr_info("[%s] compareData[63 - 0]: 0x%02x%02x%02x%02x 0x%02x%02x%02x%02x\n", __func__,
			acl_compareData[7], acl_compareData[6], acl_compareData[5], acl_compareData[4],
			acl_compareData[3], acl_compareData[2], acl_compareData[1], acl_compareData[0]);
		pr_info("[%s] compareData[127 - 64]: 0x%02x%02x%02x%02x %02x%02x%02x%02x\n", __func__,
			acl_compareData[15], acl_compareData[14], acl_compareData[13], acl_compareData[12],
			acl_compareData[11], acl_compareData[10], acl_compareData[9], acl_compareData[8]);
		pr_info("[%s] compareData[153 - 128]: 0x%02x%02x%02x%02x]\n", __func__,
			acl_compareData[19], acl_compareData[18], acl_compareData[17], acl_compareData[16]);
	}

	dubhe2000_ingress_nat_add_op_config(act, nat_index);

	dubhe1000_ingress_acl0_config_nat(is_snat, acl_index, tbl_type, acl_compareData, nat_index, key, act);

	return dubhe2000_tc_get_npe_acl_index(0, tbl_type, acl_index);
}

int cls_npe_tc_add_nat_entry(const struct hw_nat_key *key, const struct hw_nat_act *act)
{
	u8 inport, outport, tbl_type;
	u8 acl_compareData[44]; // 330bits
	u16 acl_idx, nat_idx;
	bool is_snat;
	int ret;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	if (!key || !act) {
		TC_ERR("%s: invalid key or act\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	is_snat = !!(act->replace_src);

	if (g_adapter->tc_debug) {
		pr_info("[%s] dump key:\n", __func__);
		pr_info("#### in_ifindex=%d; src_addr=%pI4 dst_addr=%pI4 src_port=0x%x dst_port=0x%x ip_proto=0x%x\n",
			key->in_ifindex, &key->src_addr, &key->dst_addr, key->src_port, key->dst_port, key->ip_proto);

		pr_info("[%s] dump act:\n", __func__);
		pr_info("#### out_dev=%s dst_mac=[%pM] src_mac=[%pM] addr=0x%x port=0x%x replace[src=%d addr=%d port=%d]\n",
			act->out_dev->name, act->dst_mac, act->src_mac, act->addr, act->port, is_snat,
			act->replace_addr, act->replace_port);

		pr_info("#### pppoe_sid=0x%04x replace_dscp=%d dscp_value=%d opt_queue=%d queue_value=%d push_pppoe=%d\n",
			act->pppoe_sid, act->replace_dscp, act->dscp_value, act->opt_queue, act->queue_value,
			act->push_pppoe);

		pr_info("#### inner_vlan: tpid=0x%04x pcp=%u dei=%u vid=%u\n",
			act->inner_vlan.tpid, act->inner_vlan.pcp, act->inner_vlan.dei, act->inner_vlan.vid);

		pr_info("#### outer_vlan: tpid=0x%04x pcp=%u dei=%u vid=%u\n",
			act->outer_vlan.tpid, act->outer_vlan.pcp, act->outer_vlan.dei, act->outer_vlan.vid);
	}

	/*
	 * To maintain atomicity:
	 * (1)check ingress_acl0 and ingress_nat_op here.
	 * (2)dubhe2000_tc_nat_add_router_config() can check l3.
	 *      if failed, We can also maintain atomicity
	 */
	memset(acl_compareData, 0, sizeof(acl_compareData));
	ret = dubhe2000_tc_nat_entry_parse_and_check(key, act, &inport, &outport, &tbl_type, &acl_idx, &nat_idx,
						     acl_compareData);
	if (ret == -TC_ERR_EXISTE_ACL)
		TC_ERR("[%s] existed acl0 nat entry! Please delete entry(tbl=%d idx=%d) first\n",
		       __func__, tbl_type, acl_idx);
	else if (ret == -TC_ERR_NO_FREE_ACL)
		TC_ERR("[%s] no free acl0 entry! Please wait other entry aged out\n", __func__);
	else if (ret == -TC_ERR_NO_FREE_NAT)
		TC_ERR("[%s] no free nat op entry! Please wait other entry aged out\n", __func__);

	if (g_adapter->tc_debug) {
		pr_info("[%s] analysis results result: inport=%d ouport=%d acl(type=%d idx=%d) nat_idx=%d\n",
			__func__, inport, outport, tbl_type, acl_idx, nat_idx);
	}

	if (ret >= 0)
		ret = dubhe2000_tc_nat_add_router_config(is_snat, key, act, inport, outport);

	if (ret >= 0)
		ret = dubhe2000_tc_ingress_acl0_config_nat(is_snat, key, act, tbl_type, acl_idx, nat_idx,
							   acl_compareData);

	return ret;
}
EXPORT_SYMBOL(cls_npe_tc_add_nat_entry);

int cls_npe_tc_del_nat_entry(const struct clsemi_flow_entry *flow, bool step1, bool step2)
{
	int npe_acl_index = flow->npe_acl_index;
	int l3_index, nextHop_index;
	u8 tbl_type, vrf, inport, aclN;
	bool is_snat, entryValid;
	u16 natOpPtr, acl_index, nextHop_counter;
	u32 valid_dip, new_dip, key_tmp[11];
	t_IngressConfigurableACL0LargeTable large_tbl;
	t_IngressConfigurableACL0SmallTable small_tbl;
	t_IngressConfigurableACL0TCAM tcam_tbl;
	t_IngressConfigurableACL0TCAMAnswer tcam_answer;
	t_IngressNATOperation nat_op;
	t_HashBasedL3RoutingTable l3_tbl;
	t_NextHopTable nexthop_tbl;
	t_NextHopPacketModifications nexthop_pkt_mod;
	t_NextHopDAMAC nexthop_dmac;
	bool natOpValid, enableUpdateIp, updateSaOrDa;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	dubhe2000_tc_get_acl_hash_type(npe_acl_index, &aclN, &tbl_type, &acl_index);

	if (aclN != 0 || (!dubhe1000_acl_check_index(aclN, tbl_type, acl_index))) {
		TC_ERR("[%s] invalid index\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	if (!step1) {
		nextHop_index = flow->npe_nexthop_index;
		natOpPtr = flow->npe_nat_index;
		goto second_step;
	}

	/*
	 * 1. get ingress nat option index form acl_tbl
	 * 2. get l3 index form acl_tbl
	 * 3. get nexthop index from l3 table
	 * 4. clean l3 dip
	 * 5. clean nat
	 * 6. clean acl
	 */
	memset(key_tmp, 0, sizeof(key_tmp));

	//get acl base param
	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		if (acl_index >= IngressConfigurableACL0LargeTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -TC_ERR_INVALID_ARG;
		}

		rd_IngressConfigurableACL0LargeTable(g_adapter, acl_index, &large_tbl);

		entryValid = !!large_tbl.valid;
		natOpValid = !!large_tbl.natOpValid;
		natOpPtr = large_tbl.natOpPtr;
		enableUpdateIp = !!large_tbl.enableUpdateIp;
		updateSaOrDa = !!large_tbl.updateSaOrDa;
		new_dip = large_tbl.newIpValue;
		memcpy(key_tmp, large_tbl.compareData, sizeof(large_tbl.compareData));
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		if (acl_index >= IngressConfigurableACL0SmallTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -TC_ERR_INVALID_ARG;
		}

		rd_IngressConfigurableACL0SmallTable(g_adapter, acl_index, &small_tbl);

		entryValid = !!small_tbl.valid;
		natOpValid = !!small_tbl.natOpValid;
		natOpPtr = small_tbl.natOpPtr;
		enableUpdateIp = !!small_tbl.enableUpdateIp;
		updateSaOrDa = !!small_tbl.updateSaOrDa;
		new_dip = small_tbl.newIpValue;
		memcpy(key_tmp, small_tbl.compareData, sizeof(small_tbl.compareData));
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		if (acl_index >= IngressConfigurableACL0TCAMAnswer_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -TC_ERR_INVALID_ARG;
		}

		rd_IngressConfigurableACL0TCAM(g_adapter, acl_index, &tcam_tbl);
		rd_IngressConfigurableACL0TCAMAnswer(g_adapter, acl_index, &tcam_answer);

		entryValid = !!tcam_tbl.valid;
		natOpValid = !!tcam_answer.natOpValid;
		natOpPtr = tcam_answer.natOpPtr;
		enableUpdateIp = !!tcam_answer.enableUpdateIp;
		updateSaOrDa = !!tcam_answer.updateSaOrDa;
		new_dip = tcam_answer.newIpValue;
		memcpy(key_tmp, tcam_tbl.compareData, sizeof(tcam_tbl.compareData));
	} else {
		TC_ERR("[%s] invalid acl tbl_type\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	// check and update params
	if (!natOpValid) {
		TC_ERR("[%s] acl=0x%x type=%d: acl_entry disable or not nat entry\n", __func__, acl_index, tbl_type);

		return -TC_ERR_OTHER;
	}

	if (enableUpdateIp) { // only dnat will config it, get valid_from is new_ip
		if (!updateSaOrDa) {
			TC_ERR("[%s] acl=0x%x type=%d: invalid UpdateIp config\n", __func__, acl_index, tbl_type);
			return -TC_ERR_OTHER;
		}

		is_snat = false;
		valid_dip = new_dip;
	} else { //snat: get valid_dip from compareDtata->dest_addr
		is_snat = true;
		valid_dip = ((key_tmp[1] & BITS_32(7, 31)) >> 7) + ((key_tmp[2] & BITS_32(0, 6)) << 25);
	}

	inport = (key_tmp[3] & BITS_32(15, 17)) >> 15;
	vrf = dubhe2000_tc_get_vrf_by_inport(inport);

	rd_IngressNATOperation(g_adapter, natOpPtr, &nat_op);

	if (is_snat != nat_op.replaceSrc) {
		TC_ERR("[%s] acl=0x%x type=%d nat=0x%x: invalid nat type(snat: acl=%d nat=%d)\n",
		       __func__, acl_index, tbl_type, natOpPtr, is_snat, nat_op.replaceSrc);

		return -TC_ERR_OTHER;
	}

	l3_index = dubhe1000_hash_based_l3_routing_tbl_get_existed_v4_index(g_adapter, vrf, valid_dip);
	if (l3_index < 0) {
		TC_ERR("[%s] acl=0x%x type=%d: l3 index non-existed(ip=0x%x vrf=%d)\n",
		       __func__, acl_index, tbl_type, valid_dip, vrf);
		return -TC_ERR_OTHER;
	}

	rd_HashBasedL3RoutingTable(g_adapter, l3_index, &l3_tbl);
	nextHop_index = l3_tbl.nextHopPointer;

	if ((switch_nexthop_info[nextHop_index] >> 48) == 0) {
		TC_ERR("[%s] acl=0x%x type=%d l3_index 0x%x: nextHop_info 0x%llx non-existed\n",
		       __func__, acl_index, tbl_type, l3_index, switch_nexthop_info[nextHop_index]);
		return -TC_ERR_OTHER;
	}

	// start clearing
	/* clear acl entry */
	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		memset(&large_tbl, 0, sizeof(large_tbl));
		wr_IngressConfigurableACL0LargeTable(g_adapter, acl_index, &large_tbl);

	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		memset(&small_tbl, 0, sizeof(small_tbl));
		wr_IngressConfigurableACL0SmallTable(g_adapter, acl_index, &small_tbl);

	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		memset(&tcam_tbl, 0, sizeof(tcam_tbl));
		memset(tcam_tbl.mask, 0xFF, sizeof(tcam_tbl.mask));
		wr_IngressConfigurableACL0TCAM(g_adapter, acl_index, &tcam_tbl);

		memset(&tcam_answer, 0, sizeof(tcam_answer));
		wr_IngressConfigurableACL0TCAMAnswer(g_adapter, acl_index, &tcam_answer);
	}

	switch_l3_hash_counter[l3_index] = switch_l3_hash_counter[l3_index] - 1;
	// check l3_index counter
	if (!switch_l3_hash_counter[l3_index]) {
		// clear l3 table
		memset(&l3_tbl, 0, sizeof(l3_tbl));
		wr_HashBasedL3RoutingTable(g_adapter, l3_index, &l3_tbl);

	}

	if (!step2)
		return 0;

	cls_npe_switch_config_delay_ms();

second_step:

	//check nexthop counter
	dubhe2000_l3_nexthop_counter_op(nextHop_index, 0, 0);
	nextHop_counter = switch_nexthop_info[nextHop_index] >> 48;
	if (nextHop_counter == 0) {
		// clear next hop tbl
		memset(&nexthop_tbl, 0, sizeof(nexthop_tbl));
		wr_NextHopTable(g_adapter, nextHop_index, &nexthop_tbl);
		dubhe2000_nexthop_unmap_tunnel(nextHop_index);

		// clear next hop dmac
		memset(&nexthop_dmac, 0, sizeof(nexthop_dmac));
		wr_NextHopDAMAC(g_adapter, nextHop_index, &nexthop_dmac);

		// clear next hop packet modification
		memset(&nexthop_pkt_mod, 0, sizeof(nexthop_pkt_mod));
		wr_NextHopPacketModifications(g_adapter, nextHop_index, &nexthop_pkt_mod);
	}

	// clear nat entry
	dubhe2000_ingress_nat_del_op_config(natOpPtr);

	if (g_adapter->tc_debug)
		pr_info("[%s] del acl0 acl_index=0x%x(%d) nat_index=0x%x l3_index=0x%x nexthop_index=0x%x inport=%d vrf=%d\n",
			__func__, acl_index, tbl_type, natOpPtr, l3_index, nextHop_index, inport, vrf);

	return 0;
}
EXPORT_SYMBOL(cls_npe_tc_del_nat_entry);

int cls_npe_tc_get_nat_hit(int npe_acl_index)
{
	bool entryValid, natOpValid, hit;
	u8 tbl_type, aclN;
	u16 natOpPtr, acl_index;
	t_IngressConfigurableACL0LargeTable large_tbl;
	t_IngressConfigurableACL0SmallTable small_tbl;
	t_IngressConfigurableACL0TCAM tcam_tbl;
	t_IngressConfigurableACL0TCAMAnswer tcam_answer;
	t_IngressNATHitStatus nat_hit;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	dubhe2000_tc_get_acl_hash_type(npe_acl_index, &aclN, &tbl_type, &acl_index);

	if (aclN != 0 || (!dubhe1000_acl_check_index(aclN, tbl_type, acl_index))) {
		TC_ERR("[%s] invalid index\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	/*
	 * 1. get ingreess nat option index form acl_tbl
	 * 2. read nat hit
	 * 3. clear nat hit
	 */

	//get acl base param
	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		if (acl_index >= IngressConfigurableACL0LargeTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -TC_ERR_INVALID_ARG;
		}

		rd_IngressConfigurableACL0LargeTable(g_adapter, acl_index, &large_tbl);

		entryValid = !!large_tbl.valid;
		natOpValid = !!large_tbl.natOpValid;
		natOpPtr = large_tbl.natOpPtr;
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		if (acl_index >= IngressConfigurableACL0SmallTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -TC_ERR_INVALID_ARG;
		}

		rd_IngressConfigurableACL0SmallTable(g_adapter, acl_index, &small_tbl);

		entryValid = !!small_tbl.valid;
		natOpValid = !!small_tbl.natOpValid;
		natOpPtr = small_tbl.natOpPtr;
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		if (acl_index >= IngressConfigurableACL0TCAMAnswer_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -TC_ERR_INVALID_ARG;
		}

		rd_IngressConfigurableACL0TCAM(g_adapter, acl_index, &tcam_tbl);
		rd_IngressConfigurableACL0TCAMAnswer(g_adapter, acl_index, &tcam_answer);

		entryValid = !!tcam_tbl.valid;
		natOpValid = !!tcam_answer.natOpValid;
		natOpPtr = tcam_answer.natOpPtr;
	} else {
		TC_ERR("[%s] invalid acl tbl_type\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	// check params
	if (!entryValid || !natOpValid) {
		TC_ERR("[%s] acl=0x%x type=%d: acl_entry disable or not nat entry\n", __func__, acl_index, tbl_type);
		return -TC_ERR_OTHER;
	}

	//read hit status
	rd_IngressNATHitStatus(g_adapter, natOpPtr, &nat_hit);
	hit = nat_hit.hit;

	if (g_adapter->tc_debug)
		pr_info("[%s] acl0=0x%x type=%d nat=0x%x: hit=%d\n", __func__, acl_index, tbl_type, natOpPtr, hit);

	//reset hit status
	memset(&nat_hit, 0, sizeof(nat_hit));
	wr_IngressNATHitStatus(g_adapter, natOpPtr, &nat_hit);

	return hit;
}
EXPORT_SYMBOL(cls_npe_tc_get_nat_hit);

int dubhe2000_tc_ipv6_add_router_config(const struct hw_ipv6_key *key, const struct hw_ipv6_act *act, u8 inport,
					u8 outport, u8 *dest_ipv6)
{
	struct clsemi_flow_entry *flow = container_of(key, struct clsemi_flow_entry, ipv6_key);
	t_HashBasedL3RoutingTable hashbased_l3_routing;
	t_NextHopTable nexthop_tbl;
	t_NextHopPacketModifications nexthop_pkt_mod;
	t_NextHopDAMAC nexthop_damac;
	t_RouterPortEgressSAMACAddress router_eport_smac;

	int ret, i, l3_index, nextHop_index;
	u8 vrf, dmac[ETH_ALEN], smac[ETH_ALEN];
	u16 nextHop_counter;
	u64 dest_mac, src_mac;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	for (i = 0; i < ETH_ALEN; i++)
		dmac[i] = act->dst_mac[ETH_ALEN - i - 1];

	memcpy(&dest_mac, dmac, ETH_ALEN);

	vrf = dubhe2000_tc_get_vrf_by_inport(inport);

	l3_index = dubhe1000_hash_based_l3_routing_tbl_get_existed_v6_index(g_adapter, vrf, dest_ipv6);
	if (l3_index < 0) {
		l3_index = dubhe1000_hash_based_l3_routing_tbl_get_new_v6_index(g_adapter, vrf, dest_ipv6);
		if (l3_index < 0) {
			TC_ERR("[%s] invalid l3 index\n", __func__);

			return -TC_ERR_NO_FREE_L3;
		}

		nextHop_index = dubhe1000_next_hop_tbl_get_new_index(g_adapter, dest_mac);
		if (nextHop_index < 0) {
			TC_ERR("[%s] invalid nextHop_index\n", __func__);

			return -TC_ERR_NO_FREE_NEXTHOP_TBL;
		}
	} else {
		switch_l3_hash_counter[l3_index] = switch_l3_hash_counter[l3_index] + 1;

		rd_HashBasedL3RoutingTable(g_adapter, l3_index, &hashbased_l3_routing);
		nextHop_index = hashbased_l3_routing.nextHopPointer;
		dubhe2000_l3_nexthop_counter_op(nextHop_index, dest_mac, 1);

		flow->npe_nexthop_index = nextHop_index;
		if (g_adapter->tc_debug)
			pr_info("[%s] l3_index 0x%x exised, nexthop_index 0x%x\n", __func__, l3_index, nextHop_index);

		return 0;
	}

	flow->npe_nexthop_index = nextHop_index;
	if (g_adapter->tc_debug)
		pr_info("[%s] ipaddr=%pI6 vrf=%d l3_index=0x%x nexthop_index=0x%x inport=%d outport=%d\n",
			__func__, &key->dst_v6, vrf, l3_index, nextHop_index, inport, outport);

	dubhe2000_l3_nexthop_counter_op(nextHop_index, dest_mac, 1);
	nextHop_counter = switch_nexthop_info[nextHop_index] >> 48;
	if (nextHop_counter == 1) {
		//config Next_Hop_Table
		memset(&nexthop_tbl, 0, sizeof(nexthop_tbl));
		nexthop_tbl.validEntry = 1;
		nexthop_tbl.nextHopPacketMod = nextHop_index;
		nexthop_tbl.l2Uc = 1;
		nexthop_tbl.destPortormcAddr = outport;
		nexthop_tunnel_map[nextHop_index].tunnel_entry_idx = -1;
		nexthop_tunnel_map[nextHop_index].tunnel_exit_idx = -1;
		if (act->push_pppoe) {
			ret = dubhe2000_nexthop_map_tunnel(nextHop_index, act->pppoe_sid, false);
			if (ret < 0) {
				TC_ERR("%s: failed to bind tunnel\n", __func__);
				return ret;
			}
			nexthop_tbl.tunnelEntry = 1;
			nexthop_tbl.tunnelEntryPtr = nexthop_tunnel_map[nextHop_index].tunnel_entry_idx;
		}
		wr_NextHopTable(g_adapter, nextHop_index, &nexthop_tbl);

		//Next Hop DA MAC
		memset(&nexthop_damac, 0, sizeof(nexthop_damac));
		nexthop_damac.daMac = dest_mac;
		wr_NextHopDAMAC(g_adapter, nextHop_index, &nexthop_damac);

		//Config Next_Hop_Packet_Modifications
		memset(&nexthop_pkt_mod, 0, sizeof(nexthop_pkt_mod));
		nexthop_pkt_mod.valid = 1;
		dubhe2000_conf_inner_vlan(&nexthop_pkt_mod, &act->inner_vlan);
		dubhe2000_conf_outer_vlan(&nexthop_pkt_mod, &act->outer_vlan);
		wr_NextHopPacketModifications(g_adapter, nextHop_index, &nexthop_pkt_mod);
	} else {//check nexthop reuse
		memset(&nexthop_tbl, 0, sizeof(nexthop_tbl));
		rd_NextHopTable(g_adapter, nextHop_index, &nexthop_tbl);

		memset(&nexthop_damac, 0, sizeof(nexthop_damac));
		rd_NextHopDAMAC(g_adapter, nextHop_index, &nexthop_damac);

		memset(&nexthop_pkt_mod, 0, sizeof(nexthop_pkt_mod));
		rd_NextHopPacketModifications(g_adapter, nextHop_index, &nexthop_pkt_mod);

		if ((nexthop_damac.daMac != dest_mac) || (nexthop_tbl.destPortormcAddr != outport) ||
			(nexthop_tbl.validEntry != 1) || (nexthop_pkt_mod.valid != 1))
			TC_ERR("[%s] nexthop reuse ERROR\n", __func__);
	}

	//config Hash_Based_L3_Routing_Table
	memset(&hashbased_l3_routing, 0, sizeof(hashbased_l3_routing));
	hashbased_l3_routing.ipVersion = 1; //IPV6
	hashbased_l3_routing.mpls = 0;
	hashbased_l3_routing.vrf = vrf;
	memcpy(hashbased_l3_routing.destIPAddr, dest_ipv6, sizeof(hashbased_l3_routing.destIPAddr));
	hashbased_l3_routing.nextHopPointer = nextHop_index;
	wr_HashBasedL3RoutingTable(g_adapter, l3_index, &hashbased_l3_routing);
	switch_l3_hash_counter[l3_index] = switch_l3_hash_counter[l3_index] + 1;

	if (!dubhe1000_check_u8_array_is_zero(act->src_mac, ETH_ALEN)) {
		rd_RouterPortEgressSAMACAddress(g_adapter, vrf, &router_eport_smac);

		router_eport_smac.selectMacEntryPortMask |= (1 << outport);

		for (i = 0; i < ETH_ALEN; i++)
			smac[i] = act->src_mac[ETH_ALEN - i - 1];

		memcpy(&src_mac, smac, ETH_ALEN);
		router_eport_smac.altMacAddress = src_mac;
		wr_RouterPortEgressSAMACAddress(g_adapter, vrf, &router_eport_smac);
	}

	return l3_index;
}

int dubhe2000_tc_ipv6_parse_and_check(const struct hw_ipv6_key *key, const struct hw_ipv6_act *act, u8 *inport,
				      u8 *outport, u32 *src_ipv6, u32 *dest_ipv6, u8 *tbl_type, u16 *acl_idx,
				      u16 *nat_idx, u8 *acl_compareData)
{
	int tmp;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	tmp = dubhe2000_tc_get_eth_port_by_index(key->in_ifindex);
	if (tmp < 0) {
		TC_ERR("[%s] invalid in_ifindex\n", __func__);

		return -TC_ERR_INVALID_ARG;
	}
	*inport = tmp;

	*outport = dubhe1000_if_port_to_switch_port(act->out_dev->if_port);

	/* ipv6 route rules setup:
	 *	hashkey struct : src_ipv6/dest_ipv6/sport/dport/l4_protocol/source_port
	 */
	dubhe2000_ingress_acl0_ipv6_router_hashkey_build(acl_compareData, src_ipv6, dest_ipv6, ntohs(key->src_port),
							 ntohs(key->dst_port), key->ip_proto, *inport);

	// check if already existing entry
	tmp = dubhe2000_ingress_aclN_get_existed_index(0, acl_compareData, 0, tbl_type);
	if (tmp >= 0) {
		*acl_idx = tmp;
		return -TC_ERR_EXISTE_ACL; // we should delete it and add it again
	}

	// get free acl tbl_type and acl_idx
	tmp = dubhe2000_ingress_aclN_get_free_index(0, acl_compareData, tbl_type);
	if (tmp < 0)
		return -TC_ERR_NO_FREE_ACL;
	*acl_idx = tmp;

	tmp = dubhe2000_ingress_nat_op_get_free_index();
	if (tmp < 0)
		return -TC_ERR_NO_FREE_NAT;
	*nat_idx = tmp;

	return 0;
}

//ingress acl0 config ipv6
int dubhe2000_tc_ingress_acl0_config_ipv6(const struct hw_ipv6_key *key, const struct hw_ipv6_act *act, u8 tbl_type,
					  u16 acl_index, u16 nat_index, u8 *acl_compareData)
{
	struct clsemi_flow_entry *flow = container_of(key, struct clsemi_flow_entry, ipv6_key);
	struct hw_nat_act nat_act;
	/*
	 * ingress acl0(Lan<->Wan) config ipv6 process
	 * 1. useAclN/rules pointer/rules setup/acl_mask have been configed in dubhe1000_switch_init()
	 * 2. build hashkey and calculate hashvalue
	 * 3. get free ingress acl0 index and ingress nat op index
	 * 4. config acl, nat(no operation)
	 */

	flow->npe_nat_index = nat_index;
	if (g_adapter->tc_debug) {
		pr_info("[%s] acl0  acl_index=0x%x tbl_type=%d nat_index=%d\n", __func__,
			acl_index, tbl_type, nat_index);

		pr_info("[%s] compareData[63 - 0]: 0x%02x%02x%02x%02x 0x%02x%02x%02x%02x\n", __func__,
			acl_compareData[7], acl_compareData[6], acl_compareData[5], acl_compareData[4],
			acl_compareData[3], acl_compareData[2], acl_compareData[1], acl_compareData[0]);
		pr_info("[%s] compareData[127 - 64]: 0x%02x%02x%02x%02x %02x%02x%02x%02x\n", __func__,
			acl_compareData[15], acl_compareData[14], acl_compareData[13], acl_compareData[12],
			acl_compareData[11], acl_compareData[10], acl_compareData[9], acl_compareData[8]);
		pr_info("[%s] compareData[191 - 128]: 0x%02x%02x%02x%02x %02x%02x%02x%02x\n", __func__,
			acl_compareData[23], acl_compareData[22], acl_compareData[21], acl_compareData[20],
			acl_compareData[19], acl_compareData[18], acl_compareData[17], acl_compareData[16]);
	}

	memset(&nat_act, 0, sizeof(nat_act));
	dubhe2000_ingress_nat_add_op_config(&nat_act, nat_index);

	dubhe2000_ingress_acl0_config_ipv6_router(acl_index, tbl_type, acl_compareData, nat_index, key, act);

	return dubhe2000_tc_get_npe_acl_index(0, tbl_type, acl_index);
}

int cls_npe_tc_add_ipv6_entry(const struct hw_ipv6_key *key, const struct hw_ipv6_act *act)
{
	u8 inport, outport, tbl_type;
	u8 acl_compareData[44]; // compareData is 330bits and it will exchange u32*
	u8 src_ipv6[16], dest_ipv6[16];
	u16 acl_idx, nat_idx;
	int ret;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	if (!key || !act) {
		TC_ERR("%s: NULL key or NULL act\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	if (!dubhe1000_change_in6_addr_to_u8(key->dst_v6, dest_ipv6) ||
	    (!dubhe1000_change_in6_addr_to_u8(key->src_v6, src_ipv6))) {
		TC_ERR("[%s] fail to parse ipv6 addr\n", __func__);

		return -TC_ERR_INVALID_ARG;
	}

	if (g_adapter->tc_debug) {
		pr_info("[%s] dump key:\n", __func__);
		pr_info("#### in_ifindex=%d; src_v6=%pI6 dst_v6=%pI6 src_port=0x%x dst_port=0x%x ip_proto=0x%x\n",
			key->in_ifindex, &key->src_v6, &key->dst_v6, key->src_port, key->dst_port, key->ip_proto);

		pr_info("[%s] dump act:\n", __func__);
		pr_info("#### out_dev=%s dst_mac=[%pM] src_mac=[%pM]\n", act->out_dev->name, act->dst_mac,
			act->src_mac);

		pr_info("#### pppoe_sid=0x%x replace_dscp=%d dscp_value=%d opt_queue=%d queue_value=%d push_pppoe=%d\n",
			act->pppoe_sid, act->replace_dscp, act->dscp_value, act->opt_queue, act->queue_value,
			act->push_pppoe);
	}

	memset(acl_compareData, 0, sizeof(acl_compareData));
	ret = dubhe2000_tc_ipv6_parse_and_check(key, act, &inport, &outport, (u32 *)src_ipv6, (u32 *)dest_ipv6,
						&tbl_type, &acl_idx, &nat_idx, acl_compareData);
	if (ret == -TC_ERR_EXISTE_ACL)
		TC_ERR("[%s] existed acl0 entry! Please delete existed entry first!\n", __func__);
	else if (ret == -TC_ERR_NO_FREE_ACL)
		TC_ERR("[%s] no free acl0 entry! Please delete other entry first!\n", __func__);
	else if (ret == -TC_ERR_NO_FREE_NAT)
		TC_ERR("[%s] no free ipv6 nat entry! Please delete other entry first!\n", __func__);
	else if (ret == -TC_ERR_INVALID_ARG)
		TC_ERR("[%s] invalid args! Please check!\n", __func__);

	if (g_adapter->tc_debug)
		pr_info("[%s] analysis results result: inport=%d ouport=%d acl(type=%d idx=%d) nat_idx=%d\n",
			__func__, inport, outport, tbl_type, acl_idx, nat_idx);

	if (ret >= 0)
		ret = dubhe2000_tc_ipv6_add_router_config(key, act, inport, outport, dest_ipv6);

	if (ret >= 0)
		ret = dubhe2000_tc_ingress_acl0_config_ipv6(key, act, tbl_type, acl_idx, nat_idx, acl_compareData);

	return ret;
}
EXPORT_SYMBOL(cls_npe_tc_add_ipv6_entry);

int cls_npe_tc_del_ipv6_entry(const struct clsemi_flow_entry *flow, bool step1, bool step2)
{
	int npe_acl_index = flow->npe_acl_index;
	int l3_index, nextHop_index;
	u8 tbl_type, inport, aclN;
	bool entryValid, natOpValid;
	u8 vrf;
	u16 natOpPtr, acl_index, nextHop_counter;
	u32 key_tmp[11], key_dip[4];
	t_IngressConfigurableACL0LargeTable large_tbl;
	t_IngressConfigurableACL0SmallTable small_tbl;
	t_IngressConfigurableACL0TCAM tcam_tbl;
	t_IngressConfigurableACL0TCAMAnswer tcam_answer;
	t_HashBasedL3RoutingTable l3_tbl;
	t_NextHopTable nexthop_tbl;
	t_NextHopPacketModifications nexthop_pkt_mod;
	t_NextHopDAMAC nexthop_dmac;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	dubhe2000_tc_get_acl_hash_type(npe_acl_index, &aclN, &tbl_type, &acl_index);

	if (aclN != 0 || (!dubhe1000_acl_check_index(aclN, tbl_type, acl_index))) {
		TC_ERR("[%s] invalid index\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	if (!step1) {
		nextHop_index = flow->npe_nexthop_index;
		natOpPtr = flow->npe_nat_index;
		goto second_step;
	}

	/*
	 * 1. get ingreess nat option index form acl_tbl
	 * 2. get l3 index form acl_tbl (parse compareData to sourceport and dipv6)
	 * 3. get nexthop index from l3 table
	 * 4. clean l3 dip
	 * 6. clean acl (nat unused)
	 */

	//get acl base param
	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		if (acl_index >= IngressConfigurableACL0LargeTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -EINVAL;
		}

		rd_IngressConfigurableACL0LargeTable(g_adapter, acl_index, &large_tbl);

		entryValid = !!large_tbl.valid;
		natOpValid = !!large_tbl.natOpValid;
		natOpPtr = large_tbl.natOpPtr;
		memcpy(key_tmp, large_tbl.compareData, sizeof(large_tbl.compareData));
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		if (acl_index >= IngressConfigurableACL0SmallTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -EINVAL;
		}

		rd_IngressConfigurableACL0SmallTable(g_adapter, acl_index, &small_tbl);

		entryValid = !!small_tbl.valid;
		natOpValid = !!small_tbl.natOpValid;
		natOpPtr = small_tbl.natOpPtr;
		memcpy(key_tmp, small_tbl.compareData, sizeof(small_tbl.compareData));
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		if (acl_index >= IngressConfigurableACL0TCAMAnswer_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -EINVAL;
		}

		rd_IngressConfigurableACL0TCAM(g_adapter, acl_index, &tcam_tbl);
		rd_IngressConfigurableACL0TCAMAnswer(g_adapter, acl_index, &tcam_answer);

		entryValid = !!tcam_tbl.valid;
		natOpValid = !!tcam_answer.natOpValid;
		natOpPtr = tcam_answer.natOpPtr;
		memcpy(key_tmp, tcam_tbl.compareData, sizeof(tcam_tbl.compareData));
	} else {
		TC_ERR("[%s] invalid acl tbl_type\n", __func__);
		return -EINVAL;
	}

	// check and update params
	if (!entryValid || !natOpValid) {
		TC_ERR("[%s] acl=0x%x type=%d: acl_entry disable or not nat entry\n", __func__, acl_index, tbl_type);

		return -EINVAL;
	}

	inport = (key_tmp[9] & BITS_32(15, 17)) >> 15;
	vrf = dubhe2000_tc_get_vrf_by_inport(inport);

	key_dip[0] = ((key_tmp[4] & BITS_32(7, 31)) >> 7) + ((key_tmp[5] & BITS_32(0, 6)) << 25);
	key_dip[1] = ((key_tmp[5] & BITS_32(7, 31)) >> 7) + ((key_tmp[6] & BITS_32(0, 6)) << 25);
	key_dip[2] = ((key_tmp[6] & BITS_32(7, 31)) >> 7) + ((key_tmp[7] & BITS_32(0, 6)) << 25);
	key_dip[3] = ((key_tmp[7] & BITS_32(7, 31)) >> 7) + ((key_tmp[8] & BITS_32(0, 6)) << 25);

	l3_index = dubhe1000_hash_based_l3_routing_tbl_get_existed_v6_index(g_adapter, vrf, (u8 *)key_dip);
	if (l3_index < 0) {
		TC_ERR("[%s] acl=0x%x type=%d: l3 index non-existed(ip=0x%pI6 vrf=%d)\n",
		       __func__, acl_index, tbl_type, (struct in6_addr *)key_dip, vrf);
		return -EINVAL;
	}

	rd_HashBasedL3RoutingTable(g_adapter, l3_index, &l3_tbl);
	nextHop_index = l3_tbl.nextHopPointer;

	if ((switch_nexthop_info[nextHop_index] >> 48) == 0) {
		TC_ERR("[%s] acl=0x%x type=%d l3_index 0x%x: nextHop_info 0x%llx non-existed\n",
		       __func__, acl_index, tbl_type, l3_index, switch_nexthop_info[nextHop_index]);
		return -TC_ERR_OTHER;
	}

	/* clean acl entry */
	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		memset(&large_tbl, 0, sizeof(large_tbl));
		wr_IngressConfigurableACL0LargeTable(g_adapter, acl_index, &large_tbl);

	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		memset(&small_tbl, 0, sizeof(small_tbl));
		wr_IngressConfigurableACL0SmallTable(g_adapter, acl_index, &small_tbl);

	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		memset(&tcam_tbl, 0, sizeof(tcam_tbl));
		memset(tcam_tbl.mask, 0xFF, sizeof(tcam_tbl.mask));
		wr_IngressConfigurableACL0TCAM(g_adapter, acl_index, &tcam_tbl);

		memset(&tcam_answer, 0, sizeof(tcam_answer));
		wr_IngressConfigurableACL0TCAMAnswer(g_adapter, acl_index, &tcam_answer);
	}

	switch_l3_hash_counter[l3_index] = switch_l3_hash_counter[l3_index] - 1;
	// check l3_index counter
	if (!switch_l3_hash_counter[l3_index]) {
		// clear l3 table
		memset(&l3_tbl, 0, sizeof(l3_tbl));
		wr_HashBasedL3RoutingTable(g_adapter, l3_index, &l3_tbl);
	}

	if (!step2)
		return 0;

	cls_npe_switch_config_delay_ms();

second_step:

	//check nexthop counter
	dubhe2000_l3_nexthop_counter_op(nextHop_index, 0, 0);
	nextHop_counter = switch_nexthop_info[nextHop_index] >> 48;
	if (nextHop_counter == 0) {
		// clear next hop tbl
		memset(&nexthop_tbl, 0, sizeof(nexthop_tbl));
		wr_NextHopTable(g_adapter, nextHop_index, &nexthop_tbl);
		dubhe2000_nexthop_unmap_tunnel(nextHop_index);

		// clear next hop dmac
		memset(&nexthop_dmac, 0, sizeof(nexthop_dmac));
		wr_NextHopDAMAC(g_adapter, nextHop_index, &nexthop_dmac);

		// clear next hop packet modification
		memset(&nexthop_pkt_mod, 0, sizeof(nexthop_pkt_mod));
		wr_NextHopPacketModifications(g_adapter, nextHop_index, &nexthop_pkt_mod);
	}

	dubhe2000_ingress_nat_del_op_config(natOpPtr);

	if (g_adapter->tc_debug)
		pr_info("[%s] del acl0 acl_index=0x%x(%d) nat_index=0x%x l3_index=0x%x nexthop_index=%d inport=%d vrf=%d\n",
			__func__, acl_index, tbl_type, natOpPtr, l3_index, nextHop_index, inport, vrf);

	return 0;
}
EXPORT_SYMBOL(cls_npe_tc_del_ipv6_entry);

int cls_npe_tc_get_ipv6_hit(int npe_acl_index)
{
	bool entryValid, natOpValid, hit;
	u8 tbl_type, aclN;
	u16 natOpPtr, acl_index;
	t_IngressConfigurableACL0LargeTable large_tbl;
	t_IngressConfigurableACL0SmallTable small_tbl;
	t_IngressConfigurableACL0TCAM tcam_tbl;
	t_IngressConfigurableACL0TCAMAnswer tcam_answer;
	t_IngressNATHitStatus nat_hit;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	dubhe2000_tc_get_acl_hash_type(npe_acl_index, &aclN, &tbl_type, &acl_index);

	if (aclN != 0 || (!dubhe1000_acl_check_index(aclN, tbl_type, acl_index))) {
		TC_ERR("[%s] invalid index\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	/*
	 * 1. get ingreess nat option index form acl_tbl
	 * 2. read nat hit
	 * 3. clean nat hit
	 */

	//get acl base param
	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		if (acl_index >= IngressConfigurableACL0LargeTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -EINVAL;
		}

		rd_IngressConfigurableACL0LargeTable(g_adapter, acl_index, &large_tbl);

		entryValid = !!large_tbl.valid;
		natOpValid = !!large_tbl.natOpValid;
		natOpPtr = large_tbl.natOpPtr;
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		if (acl_index >= IngressConfigurableACL0SmallTable_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -EINVAL;
		}

		rd_IngressConfigurableACL0SmallTable(g_adapter, acl_index, &small_tbl);

		entryValid = !!small_tbl.valid;
		natOpValid = !!small_tbl.natOpValid;
		natOpPtr = small_tbl.natOpPtr;
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		if (acl_index >= IngressConfigurableACL0TCAMAnswer_nr_entries) {
			TC_ERR("[%s] invalid acl_index\n", __func__);
			return -EINVAL;
		}

		rd_IngressConfigurableACL0TCAM(g_adapter, acl_index, &tcam_tbl);
		rd_IngressConfigurableACL0TCAMAnswer(g_adapter, acl_index, &tcam_answer);

		entryValid = !!tcam_tbl.valid;
		natOpValid = !!tcam_answer.natOpValid;
		natOpPtr = tcam_answer.natOpPtr;
	} else {
		TC_ERR("[%s] invalid acl tbl_type\n", __func__);
		return -EINVAL;
	}

	// check params
	if (!entryValid || !natOpValid) {
		TC_ERR("[%s] acl=0x%x type=%d: acl_entry disable or not nat entry\n", __func__, acl_index, tbl_type);

		return -EINVAL;
	}

	//read hit status
	rd_IngressNATHitStatus(g_adapter, natOpPtr, &nat_hit);
	hit = nat_hit.hit;

	if (g_adapter->tc_debug)
		pr_info("[%s] acl0=0x%x type=%d nat=0x%x: hit=%d\n", __func__, acl_index, tbl_type, natOpPtr, hit);

	//reset hit status
	memset(&nat_hit, 0, sizeof(nat_hit));
	wr_IngressNATHitStatus(g_adapter, natOpPtr, &nat_hit);

	return hit;
}
EXPORT_SYMBOL(cls_npe_tc_get_ipv6_hit);

/*
 * ACL0: IPV4 NAT/ IPV6 ROUTING: unused here
 * ACL1: IPV4 case (non nat) withou macaddr
 * ACL2: IPV6 case (non routing), and ipv4 (non nat) with macaddr
 * ACL3: special packet(not include ipaddr)
 */
int dubhe2000_tc_select_acl_engine(const struct hw_acl_match *match, const struct hw_acl_act *act, u8 *hashkey,
				   u8 *mask)
{
	int aclN = -1;

	/*
	 * when ipv6 address is masked, select acl2
	 * when mac address is masked, select acl2
	 * when ipv4 address is masked: if macaddr is masked, select acl2. if unmasked, select acl1;
	 * when ipv4/ipv6/mac address and l4_port are unmasked, select acl3
	 * Note: aclN Fields should cover all masked entry, if not, return UNSUPPORT
	 */

	if (aclN == 1) {
		// TODO
		// build hashkey in struct dubhe2000_tc_acl1_hashkey
	} else if (aclN == 2) {
		// TODO
		// build hashkey/mask in struct dubhe2000_tc_acl2_ipv4_hashkey/dubhe2000_tc_acl2_ipv6_hashkey/
	} else if (aclN == 3) {
		// TODO
		// build hashkey/mask in struct dubhe2000_tc_acl3_hashkey
	}

	return aclN;
}

// ingress acl entry
int cls_npe_tc_add_acl_entry(const struct hw_acl_match *match, const struct hw_acl_act *act)
{
	int aclN, acl_idx;
	u8 tbl_type;
	u8 acl_compareData[68], mask[68]; // can cover all acl compareData
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	if (!match || !act) {
		TC_ERR("%s: invalid match or act\n", __func__);
		return -EINVAL;
	}

	if (g_adapter->tc_debug) {
		pr_info("[%s] dump match:\n", __func__);
		pr_info("#### in_ifindex=%d; dmac=%pM smac=%pM src_port=0x%x dst_port=0x%x h_proto=0x%xip_proto=0x%x\n",
			match->in_ifindex, match->key.dmac, match->key.smac, match->key.sport, match->key.dport,
			match->key.h_proto, match->key.ip_proto);

		if (match->key.h_proto == htons(ETH_P_IPV6))
			pr_info("#### IPV6: SIP=%pI6 DIP=%pI6\n", &(match->key.sip.ip6), &(match->key.dip.ip6));
		else if (match->key.h_proto == htons(ETH_P_IP))
			pr_info("#### IPV4: SIP=%pI4 DIP=%pI4\n", &(match->key.sip.ip4), &(match->key.dip.ip4));

		pr_info("[%s] dump act:\n", __func__);
		pr_info("#### rate(min=0x%x max=0x%x) out_if_port=0x%x dscp=0x%x\n priority=0x%x\n",
			act->rate.min_bytes_ps, act->rate.max_bytes_ps, act->out_if_port, act->dscp, act->priority);
		pr_info("#### act_drop=%d act_redirect=%d act_mangle_dscp=%d act_priority=%d act_rate_ctl=%d\n",
			act->act_drop, act->act_redirect, act->act_mangle_dscp, act->act_priority, act->act_rate_ctl);
	}

	memset(acl_compareData, 0, sizeof(acl_compareData));
	memset(mask, 0, sizeof(mask)); // only used in acl2/acl3 tcam
	/* selected acl engine and build compareData/mask */
	aclN = dubhe2000_tc_select_acl_engine(match, act, acl_compareData, mask);
	if (aclN < 0)
		return -TC_ERR_UNSUPPORT;

	if (dubhe2000_ingress_aclN_get_existed_index(aclN, acl_compareData, mask, &tbl_type))
		return -TC_ERR_EXISTE_ACL;

	// get free acl tbl_type and acl_idx
	acl_idx = dubhe2000_ingress_aclN_get_free_index(aclN, acl_compareData, &tbl_type);
	if (acl_idx < 0)
		return -TC_ERR_NO_FREE_ACL;

	if (aclN == 1)
		dubhe1000_ingress_acl1_config(acl_idx, tbl_type, acl_compareData, match, act); //tcam mask all
	else if (aclN == 2)
		dubhe1000_ingress_acl2_config(acl_idx, tbl_type, acl_compareData, mask, match, act);
	else if (aclN == 3)
		dubhe1000_ingress_acl3_config(acl_idx, tbl_type, acl_compareData, mask, match, act);

	return 0;
}
EXPORT_SYMBOL(cls_npe_tc_add_acl_entry);

int cls_npe_tc_del_acl_entry(int npe_acl_index)
{
	u8 aclN, tbl_type;
	u16 acl_index;
	t_IngressConfigurableACL1LargeTable large1_tbl;
	t_IngressConfigurableACL1SmallTable small1_tbl;
	t_IngressConfigurableACL1TCAM tcam1_tbl;
	t_IngressConfigurableACL1TCAMAnswer tcam1_answer;
	t_IngressConfigurableACL2TCAM tcam2_tbl;
	t_IngressConfigurableACL2TCAMAnswer tcam2_answer;
	t_IngressConfigurableACL3TCAM tcam3_tbl;
	t_IngressConfigurableACL3TCAMAnswer tcam3_answer;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	dubhe2000_tc_get_acl_hash_type(npe_acl_index, &aclN, &tbl_type, &acl_index);

	if (aclN == 0 || (!dubhe1000_acl_check_index(aclN, tbl_type, acl_index))) {
		TC_ERR("[%s] invalid index\n", __func__);
		return -TC_ERR_INVALID_ARG;
	}

	if (aclN == 1) {
		if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
			memset(&large1_tbl, 0, sizeof(large1_tbl));

			wr_IngressConfigurableACL1LargeTable(g_adapter, acl_index, &large1_tbl);
		} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
			memset(&small1_tbl, 0, sizeof(small1_tbl));

			wr_IngressConfigurableACL1SmallTable(g_adapter, acl_index, &small1_tbl);
		} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
			memset(&tcam1_tbl, 0, sizeof(tcam1_tbl));
			memset(&tcam1_answer, 0, sizeof(tcam1_answer));

			wr_IngressConfigurableACL1TCAM(g_adapter, acl_index, &tcam1_tbl);
			wr_IngressConfigurableACL1TCAMAnswer(g_adapter, acl_index, &tcam1_answer);
		}

	} else if (aclN == 2) {
		memset(&tcam2_tbl, 0, sizeof(tcam2_tbl));
		memset(&tcam2_answer, 0, sizeof(tcam2_answer));

		wr_IngressConfigurableACL2TCAM(g_adapter, acl_index, &tcam2_tbl);
		wr_IngressConfigurableACL2TCAMAnswer(g_adapter, acl_index, &tcam2_answer);
	} else if (aclN == 3) {
		memset(&tcam3_tbl, 0, sizeof(tcam3_tbl));
		memset(&tcam3_answer, 0, sizeof(tcam3_answer));

		wr_IngressConfigurableACL3TCAM(g_adapter, acl_index, &tcam3_tbl);
		wr_IngressConfigurableACL3TCAMAnswer(g_adapter, acl_index, &tcam3_answer);
	}

	return 0;
}
EXPORT_SYMBOL(cls_npe_tc_del_acl_entry);
