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

#ifndef _HW_OFFLOAD_COMMON_H
#define _HW_OFFLOAD_COMMON_H

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/kernel.h>
#include <net/clsemi_hw_flow.h>
#include "../net/bridge/br_private.h"

#ifdef DEBUG
#define cls_dbg(fmt, ...) \
	pr_info(fmt, ##__VA_ARGS__)
#else
#define cls_dbg(fmt, ...) \
	do { } while (0)
#endif

#define MATCH_ALL_IFINDEX	(~0)

enum offload_type {
	OFFLOAD_TYPE_UNSUPPORT = 0,
	OFFLOAD_TYPE_IPV4_NAT,		/* offloaded by flowtable */
	OFFLOAD_TYPE_IPV6_ROUTE,	/* offloaded by flowtable */
	OFFLOAD_TYPE_ACL,		/* offloaded by nftable or tc flower */
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

struct hw_nat_key {
	int in_ifindex;
	__be32 src_addr;
	__be32 dst_addr;
	__be16 src_port;
	__be16 dst_port;
	u8 ip_proto;
};

struct hw_nat_act {
	struct net_device *out_dev;
	u8 dst_mac[ETH_ALEN];
	u8 src_mac[ETH_ALEN];
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

struct hw_acl_match {
	int in_ifindex;
	struct {
		u8 dmac[ETH_ALEN];
		u8 smac[ETH_ALEN];
		union {
			__be32		sip4;
			struct in6_addr sip6;
		};
		union {
			__be32		dip4;
			struct in6_addr dip6;
		};
		__be16 sport;
		__be16 dport;
		__be16 h_proto;
		u8 ip_proto;
	} key, mask;
};

struct hw_acl_act {
	struct {
		u32 min_bytes_ps;
		u32 max_bytes_ps;
	} rate;				/* data for act_rate_ctl */
	u8 out_if_port;			/* data for act_redirect */
	u8 dscp;			/* data for act_mangle_dscp */
	u8 priority;			/* data for act_priority */
	u8	act_drop:1,
		act_redirect:1,
		act_mangle_dscp:1,
		act_priority:1,
		act_rate_ctl:1;
};

#define CLS_FLOW_HW_DEL_STEP1_DONE	0x0001

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

static inline bool netif_is_wlan(struct net_device *dev)
{
	return dev->ieee80211_ptr != NULL;
}

static inline bool __netif_is_npe_eth_port(int ifindex)
{
	struct net_device *dev;
	bool ret;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev)
		return false;

	ret = if_port_is_npe_eth_port(dev->if_port);
	dev_put(dev);

	return ret;
}

int hw_acl_add(struct clsemi_flow_entry *flow, const struct hw_acl_act *act);
int hw_acl_del(const struct clsemi_flow_entry *flow);
bool hw_acl_read_hit_and_reset(const struct clsemi_flow_entry *flow);

int hw_nat_add(struct clsemi_flow_entry *flow, const struct hw_nat_act *act);
int hw_nat_del(const struct clsemi_flow_entry *flow);
int hw_nat_del_step1(const struct clsemi_flow_entry *flow);
int hw_nat_del_step2(const struct clsemi_flow_entry *flow);
bool hw_nat_read_hit_and_reset(const struct clsemi_flow_entry *flow);
int cls_npe_tc_add_nat_entry(const struct hw_nat_key *key, const struct hw_nat_act *act);
int cls_npe_tc_del_nat_entry(const struct clsemi_flow_entry *flow, bool step1, bool step2);
int cls_npe_tc_get_nat_hit(int npe_acl_index);

int hw_ipv6_add(struct clsemi_flow_entry *flow, const struct hw_ipv6_act *act);
int hw_ipv6_del(const struct clsemi_flow_entry *flow);
int hw_ipv6_del_step1(const struct clsemi_flow_entry *flow);
int hw_ipv6_del_step2(const struct clsemi_flow_entry *flow);
bool hw_ipv6_read_hit_and_reset(const struct clsemi_flow_entry *flow);
int cls_npe_tc_add_ipv6_entry(const struct hw_ipv6_key *key, const struct hw_ipv6_act *act);
int cls_npe_tc_del_ipv6_entry(const struct clsemi_flow_entry *flow, bool step1, bool step2);
int cls_npe_tc_get_ipv6_hit(int npe_acl_index);

void hw_wait_for_del(void);
void cls_npe_switch_config_delay_ms(void);

u32 hw_flow_offload_get_max_entries(void);
int hw_flow_offload_init(void);

void non_npe_dev_l3_fwt_init(void);
int sw_nat_add(const struct hw_nat_key *key, struct net_device *out_dev);
int sw_nat_del(const struct hw_nat_key *key);
int sw_ipv6_add(const struct hw_ipv6_key *key, struct net_device *out_dev);
int sw_ipv6_del(const struct hw_ipv6_key *key);

#endif
