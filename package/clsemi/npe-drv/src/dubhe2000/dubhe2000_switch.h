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

/* dubhe1000_switch.h
 * Structures, enums, and macros for the Switch
 */

#ifndef _DUBHE1000_SWITCH_H_
#define _DUBHE1000_SWITCH_H_

#include "dubhe2000_osdep.h"
#include "dubhe2000.h"
#include "dubhe2000_tc.h"
#include "dubhe2000_acl.h"
#include "dubhe2000_switch_conf.h"
#include <linux/netdevice.h>

extern void __iomem *switch_base_addr;

extern u8 default_gw_da_mac[8][ETH_ALEN];
extern u16 switch_l3_hash_counter[];
extern u64 switch_nexthop_info[];

/* L2 */
int dubhe1000_l2_cpu_da_lookup_del(u8 *mac_array, u16 gid);
int dubhe1000_l2_cpu_da_lookup_add(u8 *mac_array, u16 gid);
int dubhe1000_arp_set(struct net_device *dev, u32 *ip, u8 *mac_addr);
int dubhe1000_arp_del(struct net_device *dev, u32 *ip);

bool dubhe2000_l2_learning_dmac_get(struct dubhe1000_adapter *adapter, u64 *dmac);
void dubhe2000_l2_learning_dmac_config(struct dubhe1000_adapter *adapter, bool is_en, u8 *mac);

/* L3 */
void dubhe1000_switch_router_port_macAddr_table_add(struct dubhe1000_adapter *adapter, u8 *macAddr, u8 *macMask,
						    u8 vrf);

void dubhe1000_ingress_router_table_op(struct dubhe1000_adapter *adapter, u32 val, bool add);
void dubhe1000_hash_based_l3_routing_table_op(struct dubhe1000_adapter *adapter, u8 isIpv6, u8 vrf, u8 *destIP,
					      u8 nextHopPointer, bool add);
void dubhe1000_next_hop_da_mac_table_op(struct dubhe1000_adapter *adapter, u8 index, u8 *daMac, bool add);
void dubhe1000_next_hop_packet_modify_table_op(struct dubhe1000_adapter *adapter, u8 index, bool add);
void dubhe1000_next_hop_table_op(struct dubhe1000_adapter *adapter, u16 index, u8 nextHopPacketMod, u8 destPortNum,
				 bool isUc, bool drop, bool toCPU, bool add);
void dubhe1000_ingr_egr_port_pkt_type_filter_table_op(struct dubhe1000_adapter *adapter, u8 egress_port, u8 filter_port,
						      bool add);
void dubhe1000_l3_routing_default_table_op(struct dubhe1000_adapter *adapter, u8 nextHop, u8 drop, u8 toCPU, bool add);
void dubhe1000_router_port_macAddr_table_del(struct dubhe1000_adapter *adapter, u8 *macAddr, u8 *macMask, u8 vrf);
void dubhe1000_router_port_macAddr_table_add(struct dubhe1000_adapter *adapter, u8 *macAddr, u8 *macMask, u8 vrf);

void dubhe1000_router_add(struct dubhe1000_adapter *adapter, u8 isIpv6, u8 vrf, u8 *destIP, u8 nextHopPointer,
			  u8 destPortNum, u8 nextHopPacketMod, u8 *nextHopDAMac, bool add);

int dubhe1000_router_port_macAddr_table_get_free_index(struct dubhe1000_adapter *adapter);
int dubhe1000_hash_based_l3_routing_tbl_get_new_v4_index(struct dubhe1000_adapter *adapter, u32 vrf, u32 ip_addr);
int dubhe1000_hash_based_l3_routing_tbl_get_existed_v4_index(struct dubhe1000_adapter *adapter, u32 vrf, u32 ip_addr);
int dubhe1000_hash_based_l3_routing_tbl_get_new_v6_index(struct dubhe1000_adapter *adapter, u32 vrf, const u8 *ipv6);
int dubhe1000_hash_based_l3_routing_tbl_get_existed_v6_index(struct dubhe1000_adapter *adapter, u32 vrf,
							     const u8 *ipv6);

void dubhe2000_switch_l3_hash_counter_dump(void);
void cls_npe_router_port_macaddr_config(u8 *mac, bool is_wan);

int dubhe1000_change_in6_addr_to_u8(struct in6_addr ipv6, u8 *data);
int dubhe1000_next_hop_tbl_get_new_index(struct dubhe1000_adapter *adapter, u64 dmac);
void dubhe2000_l3_nexthop_counter_op(int index, u64 dmac, bool is_add);
void dubhe2000_switch_nexthop_info_dump(void);

int dubhe2000_ingress_nat_op_get_free_index(void);
void dubhe2000_ingress_nat_op_status_op(u16 index, bool is_add);
void dubhe2000_ingress_nat_add_op_config(const struct hw_nat_act *act, u16 index);
void dubhe2000_ingress_nat_del_op_config(u16 index);
void dubhe2000_nat_dump_ingress_op_status(void);
void dubhe2000_switch_nat_init(struct dubhe1000_adapter *adapter);
void dubhe2000_change_wan_port(struct dubhe1000_adapter *adapter);

void dubhe1000_ingress_acl0_ipv4_nat_hashkey_build(u8 *compareData, u32 src_addr, u32 dest_addr, u16 src_port,
						   u16 dest_port, u8 ip_proto, u8 source_port);
void dubhe2000_ingress_acl0_ipv6_router_hashkey_build(u8 *compareData, u32 *src_ipv6, u32 *dest_ipv6, u16 src_port,
						      u16 dest_port, u8 ip_proto, u8 source_port);

void dubhe2000_ingress_acl1_ip_hashkey_build(u8 *compareData, u32 sip, u32 dip, u16 src_port, u16 dst_port, u8 l4_proto,
					     u8 source_port);

void dubhe2000_ingress_acl1_nonip_hashkey_build(u8 *compareData, u8 *dmac, u8 *smac, bool has_vlans,
						bool outer_vlan_type, bool inner_vlan_type, u16 etype, u8 source_port);

void dubhe2000_ingress_acl2_ipv4_hashkey_build(u8 *compareData, u8 *dmac, u8 *smac, u32 *sip, u32 *dip, u8 tos,
					       u16 l4_sport, u16 l4_dport, u8 l4_proto, enum SWITCH_ACL_L4_TYPE l4_type,
					       u8 source_port);

void dubhe2000_ingress_acl2_ipv6_hashkey_build(u8 *compareData, u8 *dmac, u8 *smac, u32 *sip, u32 *dip, u8 tos,
					       u16 l4_sport, u16 l4_dport, u32 ipv6_flow_lable, u8 l4_proto,
					       enum SWITCH_ACL_L4_TYPE l4_type, u8 source_port);

void dubhe2000_ingress_acl3_hashkey_build(u8 *compareData, u8 l2_packet_flags, u8 ipv4_options, u16 tcp_flags,
					  u8 l4_proto, u16 etype, enum SWITCH_ACL_L3_TYPE l4_type,
					  enum SWITCH_ACL_L3_TYPE l3_type);

void dubhe1000_ingress_acl_entry_hashkey_build(u8 *compareData);

int dubhe1000_acl_hashval_cal(u8 aclN, u8 *compareData, bool is_large);

int dubhe2000_ingress_aclN_get_free_index(u8 aclN, u8 *hashkey, u8 *tbl_type);
int dubhe2000_ingress_aclN_get_existed_index(u8 aclN, u8 *hashkey, u8 *mask, u8 *tbl_type);
int dubhe1000_acl_check_index(u8 aclN, u8 tbl_type, u16 index);

void dubhe1000_ingress_acl0_config_nat(bool is_snat, u16 index, u8 tbl_type, u8 *hashkey, u16 nat_index,
				       const struct hw_nat_key *key, const struct hw_nat_act *act);
void dubhe2000_ingress_acl0_config_ipv6_router(u16 index, u8 tbl_type, u8 *hashkey, u16 nat_index,
					       const struct hw_ipv6_key *key, const struct hw_ipv6_act *act);

void dubhe1000_ingress_acl1_config(u16 index, u8 tbl_type, u8 *hashkey, const struct hw_acl_match *match,
				   const struct hw_acl_act *act);

void dubhe1000_ingress_acl2_config(u16 index, u8 tbl_type, u8 *hashkey, u8 *mask, const struct hw_acl_match *match,
				   const struct hw_acl_act *act);

void dubhe1000_ingress_acl3_config(u16 index, u8 tbl_type, u8 *hashkey, u8 *mask, const struct hw_acl_match *match,
				   const struct hw_acl_act *act);

void dubhe2000_acl0_init_config(void);
void dubhe2000_acl1_init_config(void);
void dubhe2000_acl2_init_config(void);
void dubhe2000_acl3_init_config(void);
void dubhe2000_switch_ingress_aclN_init(struct dubhe1000_adapter *adapter);

void cls_npe_switch_config_delay_ms(void);

void dubhe2000_switch_dump_dfx(struct dubhe1000_adapter *adapter, char *reg_name);

void dubhe2000_switch_l2_init(struct dubhe1000_adapter *adapter);
void dubhe2000_switch_l3_init(struct dubhe1000_adapter *adapter);

void dubhe2000_switch_pause_config(struct dubhe1000_adapter *adapter);
void dubhe2000_switch_use_dwrr_scheduler(struct dubhe1000_adapter *adapter);
void dubhe2000_switch_init_queue_weight(struct dubhe1000_adapter *adapter);
void dubhe2000_switch_set_queue_weight(struct dubhe1000_adapter *adapter);
int dubhe2000_max_pause_mod(void);

int dubhe2000_tunnel_exit_delete_session(struct dubhe1000_adapter *adapter, u32 index);
int dubhe2000_tunnel_exit_config_session(struct dubhe1000_adapter *adapter, u16 session_id, bool is_ipv4);
int dubhe2000_tunnel_entry_delete_session(struct dubhe1000_adapter *adapter, u32 index);
int dubhe2000_tunnel_entry_config_session(struct dubhe1000_adapter *adapter, u16 session_id, bool is_ipv4);
void dubhe2000_tunnel_init_config(struct dubhe1000_adapter *adapter);
void dubhe1000_switch_drain_port(struct dubhe1000_adapter *adapter, int port, int enable);

void dubhe2000_switch_generic_svp_config(struct dubhe1000_adapter *adapter, bool is_enable);
void dubhe2000_switch_generic_tcp_config(struct dubhe1000_adapter *adapter, bool is_enable);
void dubhe2000_switch_generic_ftp_config(struct dubhe1000_adapter *adapter, bool is_enable);
void dubhe2000_switch_generic_ip_frag_config(struct dubhe1000_adapter *adapter, bool is_enable);
void dubhe2000_switch_generic_nonip_config(struct dubhe1000_adapter *adapter, u8 if_mask);
void dubhe2000_switch_generic_iperf_config(struct dubhe1000_adapter *adapter, int mode);
void dubhe2000_switch_generic_init_config(struct dubhe1000_adapter *adapter);
s32 dubhe1000_switch_init(struct dubhe1000_adapter *adapter);

#define DUBHE2000_SWITCH_CFG_DELAY_DEFAULT 76 //ms
#endif
