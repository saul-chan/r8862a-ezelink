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

/* dubhe2000_tc.h
 * Structures, enums, and macros for the Traffic Control
 */

#ifndef _DUBHE2000_TC_H_
#define _DUBHE2000_TC_H_

enum {
	SWITCH_ACL_TBL_TYPE_LARGE = 0,
	SWITCH_ACL_TBL_TYPE_SMALL,
	SWITCH_ACL_TBL_TYPE_TCAM,
};

enum {
	TC_ERR_EXISTE_ACL = 1024,
	TC_ERR_NO_FREE_ACL,
	TC_ERR_NO_FREE_NAT,
	TC_ERR_EXISTE_L3,
	TC_ERR_NO_FREE_L3,
	TC_ERR_NO_FREE_NEXTHOP_TBL,
	TC_ERR_NO_FREE_NEXTHOP_DMAC,
	TC_ERR_NO_FREE_NEXTHOP_PKT_MOD,
	TC_ERR_NO_FREE_TUNNEL_ENTRY,
	TC_ERR_NO_FREE_TUNNEL_EXIT,

	TC_ERR_INVALID_ARG,
	TC_ERR_UNSUPPORT,
	TC_ERR_OTHER,
};

enum offload_type {
	OFFLOAD_TYPE_UNSUPPORT = 0,
	OFFLOAD_TYPE_IPV4_NAT,	 /* offloaded by flowtable */
	OFFLOAD_TYPE_IPV6_ROUTE, /* offloaded by flowtable */
	OFFLOAD_TYPE_ACL,	 /* offloaded by nftable or tc flower */
};

struct push_vlan_info {
	__be16	tpid;
	union {
		struct {
			u16	pcp:3,
				dei:1,
				vid:12;
		};
		__be16	tci;
	};
};

struct hw_nat_act {
	struct net_device *out_dev;
	u8 dst_mac[ETH_ALEN]; //big-endian
	u8 src_mac[ETH_ALEN]; //big-endian
	__be32 addr;
	__be16 port;
	__be16 pppoe_sid;
	u16	replace_dscp:1,
		dscp_value:6,
		opt_queue:2,
		queue_value:3,
		push_pppoe:1,
		replace_src:1,	/* driver also can determine the dir according this val, 0: DL; 1: UL */
		replace_addr:1,
		replace_port:1;
	struct push_vlan_info inner_vlan;
	struct push_vlan_info outer_vlan;
};

struct hw_nat_key {
	int in_ifindex;
	__be32 src_addr;
	__be32 dst_addr;
	__be16 src_port;
	__be16 dst_port;
	u8 ip_proto;
};

struct hw_ipv6_key {
	int in_ifindex;
	struct in6_addr src_v6;
	struct in6_addr dst_v6;
	__be16 src_port;
	__be16 dst_port;
	u8 ip_proto;
};

struct hw_ipv6_act {
	struct net_device *out_dev;
	u8 dst_mac[ETH_ALEN];
	u8 src_mac[ETH_ALEN];
	__be16 pppoe_sid;
	u16	replace_dscp:1,
		dscp_value:6,
		opt_queue:2,
		queue_value:3,
		push_pppoe:1;
	struct push_vlan_info inner_vlan;
	struct push_vlan_info outer_vlan;
};

union acl_ip {
	__be32 ip4;
	struct in6_addr ip6;
};

struct hw_acl_match {
	int in_ifindex;
	struct {
		u8 dmac[ETH_ALEN];
		u8 smac[ETH_ALEN];
		union acl_ip sip;
		union acl_ip dip;
		__be16 sport;
		__be16 dport;
		__be16 h_proto; //ethernet type
		u8 ip_proto;	//l4 protocol
	} key, mask;
};

struct hw_acl_act {
	struct {
		u32 min_bytes_ps;
		u32 max_bytes_ps;
	} rate;			   /* data for act_rate_ctl */
	u8 out_if_port;		   /* data for act_redirect */
	u8 dscp;		   /* data for act_mangle_dscp */
	u8 priority;		   /* data for act_priority */
	u8	act_drop:1,
		act_redirect:1,
		act_mangle_dscp:1,
		act_priority:1,
		act_rate_ctl:1;
};

struct hw_acl2_match {
	int in_ifindex;
	struct {
		u8 dmac[ETH_ALEN];
		u8 smac[ETH_ALEN];
		struct in6_addr sip;
		struct in6_addr dip;
		__be16 sport;
		__be16 dport;
		__be16 h_proto; //ethernet type
		u8 ip_proto;	//l4 protocol
	} key, mask;
};

struct hw_acl3_match {
	int in_ifindex;
	struct {
		u8 dmac[ETH_ALEN];
		u8 smac[ETH_ALEN];
		union acl_ip sip;
		union acl_ip dip;
		__be16 sport;
		__be16 dport;
		__be16 h_proto; //ethernet type
		u8 ip_proto;	//l4 protocol
	} key, mask;
};

struct clsemi_flow_entry {
	struct rhash_head node;
	unsigned long cookie;
	enum offload_type offload_type;
	u64 lastused;
	union {
		struct hw_nat_key nat_key;
		struct hw_ipv6_key ipv6_key;
		struct hw_acl_match acl_match;
	};
	struct {
		int npe_acl_index;
		uint32_t npe_nexthop_index:10,
			 npe_nat_index:11;
	};
	u32 flags;
	struct rcu_head rcu;
};

#define SWITCH_DYNAMIC_CONFIG_DELAY_DEFAULT 16 //ms

#endif /* _DUBHE2000_TC_H_ */
