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

#include <linux/rhashtable.h>
#include <linux/debugfs.h>
#include <net/pkt_cls.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/inet_ecn.h>
#include <net/netfilter/nf_flow_table.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_conntrack.h>

#include "common.h"

static DEFINE_MUTEX(clsemi_flow_offload_mutex);
static struct rhashtable clsemi_flow_table;
static struct workqueue_struct *clsemi_flow_wq;
static struct delayed_work flow_stats_work;
static unsigned long flow_stats_ts;

enum match_flag {
	MATCH_IN_IFINDEX	= BIT(0),
	MATCH_ETH_SADDR		= BIT(1),
	MATCH_ETH_DADDR		= BIT(2),
	MATCH_NF_PROTO		= BIT(3),
	MATCH_IP_PROTO		= BIT(4),
	MATCH_IPV4_SADDR	= BIT(5),
	MATCH_IPV4_DADDR	= BIT(6),
	MATCH_IPV6_SADDR	= BIT(7),
	MATCH_IPV6_DADDR	= BIT(8),
	MATCH_TH_SPORT		= BIT(9),
	MATCH_TH_DPORT		= BIT(10),
};

enum action_flag {
	ACTION_DROP		= BIT(0),
	ACTION_REDIRECT		= BIT(1),
	ACTION_CSUM		= BIT(2),
	ACTION_MANGLE_ETH	= BIT(3),
	ACTION_MANGLE_IPV4_ADDR	= BIT(4),
	ACTION_MANGLE_DSCP	= BIT(5),
	ACTION_MANGLE_TH_PORT	= BIT(6),
	ACTION_PRIORITY		= BIT(7),
	ACTION_RATE_CTL		= BIT(8),
};

struct clsemi_tc_cb_priv {
	union {
		struct net_device *dev;
		struct nf_flowtable *flowtable;
	};
	bool frontend_is_flowtable;
};

struct clsemi_flow_data {
	int in_ifindex;
	struct ethhdr eth;
	union {
		struct {
			__be32 src_addr;
			__be32 dst_addr;
		} v4;

		struct {
			struct in6_addr src_addr;
			struct in6_addr dst_addr;
		} v6;
	};
	struct {
		u16 vid;
		__be32 proto;
		u8 prio;
	} vlan;
	struct {
		__be16 sid;
		u8 encap;
	} pppoe;
	struct net_device *redirect_dev;
	struct hw_acl_act acl_act;
	u32 action_flags;
	__be16 src_port;
	__be16 dst_port;
	u8 ip_proto;
};

struct orig_ndev_ops {
	struct list_head list;
	const struct net_device *dev;
	const struct net_device_ops *orig_ops;
};

static struct list_head orig_ndev_ops_list = LIST_HEAD_INIT(orig_ndev_ops_list);

static const struct rhashtable_params clsemi_rhash_params = {
	.head_offset = offsetof(struct clsemi_flow_entry, node),
	.key_offset = offsetof(struct clsemi_flow_entry, cookie),
	.key_len = sizeof(unsigned long),
	.automatic_shrinking = true,
};

static inline const char *get_frontend_name(const struct clsemi_tc_cb_priv *cb_priv)
{
	struct nft_flowtable *nft_flowtable;

	if (cb_priv->frontend_is_flowtable) {
		nft_flowtable = container_of(cb_priv->flowtable, struct nft_flowtable, data);
		return nft_flowtable->name;
	}

	return cb_priv->dev->name;
}

static int parse_mangle_eth(const struct flow_action_entry *act, void *eth)
{
	void *dest = eth + act->mangle.offset;
	const void *src = &act->mangle.val;

	if (act->mangle.offset > 8)
		return -EINVAL;

	if (act->mangle.mask == 0xffff) {
		src += 2;
		dest += 2;
	}

	memcpy(dest, src, act->mangle.mask ? 2 : 4);
	return 0;
}

static int parse_mangle_ports(const struct flow_action_entry *act, struct clsemi_flow_data *data)
{
	u32 val = ntohl(act->mangle.val);

	switch (act->mangle.offset) {
	case 0:
		if (act->mangle.mask == ~htonl(0xffff))
			data->dst_port = cpu_to_be16(val);
		else
			data->src_port = cpu_to_be16(val >> 16);
		break;
	case 2:
		data->dst_port = cpu_to_be16(val);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int parse_mangle_ipv4(const struct flow_action_entry *act, struct clsemi_flow_data *data)
{
	struct iphdr iph;

	switch (act->mangle.offset) {
	case 0:
		/* cmd: tc filter add dev eth0 ingress flower skip_sw action pedit ex munge ip tos set $((DSCP << 2)) */
		if (act->mangle.mask != htonl(0xff00ffff))
			return -EOPNOTSUPP;
		memcpy(&iph, &act->mangle.val, sizeof(act->mangle.val));
		data->action_flags |= ACTION_MANGLE_DSCP;
		data->acl_act.act_mangle_dscp = 1;
		data->acl_act.dscp = (ipv4_get_dsfield(&iph) & ~INET_ECN_MASK) >> 2;
		break;
	case offsetof(struct iphdr, saddr):
		data->action_flags |= ACTION_MANGLE_IPV4_ADDR;
		memcpy(&data->v4.src_addr, &act->mangle.val, sizeof(u32));
		break;
	case offsetof(struct iphdr, daddr):
		data->action_flags |= ACTION_MANGLE_IPV4_ADDR;
		memcpy(&data->v4.dst_addr, &act->mangle.val, sizeof(u32));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int parse_mangle_ipv6(const struct flow_action_entry *act, struct clsemi_flow_data *data)
{
	struct ipv6hdr ip6h;

	switch (act->mangle.offset) {
	case 0:
		/* cmd: tc filter add dev eth0 ingress flower skip_sw action pedit ex munge ip6 traffic_class set $((DSCP << 2)) */
		if (act->mangle.mask != htonl(0xf00fffff))
			return -EOPNOTSUPP;
		memcpy(&ip6h, &act->mangle.val, sizeof(act->mangle.val));
		data->action_flags |= ACTION_MANGLE_DSCP;
		data->acl_act.act_mangle_dscp = 1;
		data->acl_act.dscp = (ipv6_get_dsfield(&ip6h) & ~INET_ECN_MASK) >> 2;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int parse_mangle(const struct flow_action_entry *act, struct clsemi_flow_data *data, __be16 *n_proto)
{
	int err = 0;

	switch (act->mangle.htype) {
	case FLOW_ACT_MANGLE_HDR_TYPE_TCP:
	case FLOW_ACT_MANGLE_HDR_TYPE_UDP:
		err = parse_mangle_ports(act, data);
		if (!err)
			data->action_flags |= ACTION_MANGLE_TH_PORT;
		break;
	case FLOW_ACT_MANGLE_HDR_TYPE_IP4:
		if (*n_proto == 0)
			*n_proto = htons(ETH_P_IP);
		else if (*n_proto != htons(ETH_P_IP))
			return -EINVAL;
		err = parse_mangle_ipv4(act, data);
		break;
	case FLOW_ACT_MANGLE_HDR_TYPE_IP6:
		if (*n_proto == 0)
			*n_proto = htons(ETH_P_IPV6);
		else if (*n_proto != htons(ETH_P_IPV6))
			return -EINVAL;
		err = parse_mangle_ipv6(act, data);
		break;
	case FLOW_ACT_MANGLE_HDR_TYPE_ETH:
		err = parse_mangle_eth(act, &data->eth);
		if (!err)
			data->action_flags |= ACTION_MANGLE_ETH;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return err;
}

static int determine_offload_type(u32 match_flags, u32 action_flags)
{
#define IPV4_NAT_MATCH_FLAGS (MATCH_IN_IFINDEX	| \
			      MATCH_NF_PROTO	| \
			      MATCH_IP_PROTO	| \
			      MATCH_IPV4_SADDR	| \
			      MATCH_IPV4_DADDR	| \
			      MATCH_TH_SPORT	| \
			      MATCH_TH_DPORT)

#define IPV6_RUT_MATCH_FLAGS (MATCH_IN_IFINDEX	| \
			      MATCH_NF_PROTO	| \
			      MATCH_IP_PROTO	| \
			      MATCH_IPV6_SADDR	| \
			      MATCH_IPV6_DADDR	| \
			      MATCH_TH_SPORT	| \
			      MATCH_TH_DPORT)

#define IPV4_NAT_ACTION_FLAGS (ACTION_REDIRECT		| \
			       ACTION_MANGLE_ETH	| \
			       ACTION_MANGLE_IPV4_ADDR	| \
			       ACTION_MANGLE_TH_PORT	| \
			       ACTION_CSUM)

#define IPV6_RUT_ACTION_FLAGS (ACTION_REDIRECT		| \
			       ACTION_MANGLE_ETH)

#define ACL_SUPPORT_ACTION_FLAGS (ACTION_DROP		| \
				  ACTION_REDIRECT	| \
				  ACTION_RATE_CTL	| \
				  ACTION_PRIORITY	| \
				  ACTION_MANGLE_DSCP)

	if (match_flags == IPV4_NAT_MATCH_FLAGS && action_flags == IPV4_NAT_ACTION_FLAGS)
		return OFFLOAD_TYPE_IPV4_NAT;

	if (match_flags == IPV6_RUT_MATCH_FLAGS && action_flags == IPV6_RUT_ACTION_FLAGS)
		return OFFLOAD_TYPE_IPV6_ROUTE;

	if (!(action_flags & ~ACL_SUPPORT_ACTION_FLAGS))
		return OFFLOAD_TYPE_ACL;

	return OFFLOAD_TYPE_UNSUPPORT;
}

static int offload_nat(struct clsemi_flow_entry *flow,
		       const struct clsemi_flow_data *data_key,
		       const struct clsemi_flow_data *data_act,
		       const struct flow_action_entry *act_cls_meta)
{
	struct hw_nat_key *nat_key = &flow->nat_key;
	struct hw_nat_act nat_act = {};
	struct net_device *dev;
	int err;

	nat_key->in_ifindex = data_key->in_ifindex;
	dev = dev_get_by_index(&init_net, nat_key->in_ifindex);
	if (!dev)
		return -ENODEV;

	if (dev->type == ARPHRD_NONE || dev->type == ARPHRD_IPGRE || dev->type == ARPHRD_PPP) {
		dev_put(dev);
		return -EOPNOTSUPP;
	}
	dev_put(dev);

	nat_key->src_addr = data_key->v4.src_addr;
	nat_key->dst_addr = data_key->v4.dst_addr;
	nat_key->src_port = data_key->src_port;
	nat_key->dst_port = data_key->dst_port;
	nat_key->ip_proto = data_key->ip_proto;

	nat_act.out_dev = data_act->redirect_dev;
	if (nat_act.out_dev->type == ARPHRD_NONE || nat_act.out_dev->type == ARPHRD_IPGRE
			|| nat_act.out_dev->type == ARPHRD_PPP)
		return -EOPNOTSUPP;

	ether_addr_copy(nat_act.dst_mac, data_act->eth.h_dest);
	ether_addr_copy(nat_act.src_mac, data_act->eth.h_source);

	if (data_act->pppoe.encap) {
		nat_act.push_pppoe = 1;
		nat_act.pppoe_sid = data_act->pppoe.sid;
	}

	if (data_act->vlan.vid) {
		nat_act.inner_vlan.tpid = data_act->vlan.proto;
		nat_act.inner_vlan.vid = data_act->vlan.vid;
		nat_act.inner_vlan.pcp = data_act->vlan.prio;
		nat_act.inner_vlan.dei = 0;
	}

	/* TODO: set outer_vlan if out_dev is DSA interface and use VLAN tag as DSA tag */

	if (data_act->v4.src_addr != 0 && data_act->v4.dst_addr == 0) {
		nat_act.replace_src = 1;
		nat_act.replace_addr = 1;
		nat_act.addr = data_act->v4.src_addr;
	} else if (data_act->v4.src_addr == 0 && data_act->v4.dst_addr != 0) {
		nat_act.replace_src = 0;
		nat_act.replace_addr = 1;
		nat_act.addr = data_act->v4.dst_addr;
	} else
		return -EOPNOTSUPP;

	if (data_act->src_port != 0 && data_act->dst_port == 0) {
		if (!nat_act.replace_src)
			return -EOPNOTSUPP;
		if (data_key->src_port != data_act->src_port) {
			nat_act.replace_port = 1;
			nat_act.port = data_act->src_port;
		}
	} else if (data_act->src_port == 0 && data_act->dst_port != 0) {
		if (nat_act.replace_src)
			return -EOPNOTSUPP;
		if (data_key->dst_port != data_act->dst_port) {
			nat_act.replace_port = 1;
			nat_act.port = data_act->dst_port;
		}
	} else
		return -EOPNOTSUPP;

	if (act_cls_meta) {
		nat_act.replace_dscp	= act_cls_meta->cls_meta.set_dscp;
		nat_act.dscp_value	= act_cls_meta->cls_meta.dscp;
		nat_act.opt_queue	= act_cls_meta->cls_meta.opt_queue;
		nat_act.queue_value	= act_cls_meta->cls_meta.queue;
	}

	err = hw_nat_add(flow, &nat_act);
	if (err == -EEXIST) {
		hw_nat_del(flow);
		err = hw_nat_add(flow, &nat_act);
	}

	if (!err)
		sw_nat_add(nat_key, nat_act.out_dev);

	return err;
}

static int offload_ipv6(struct clsemi_flow_entry *flow,
			const struct clsemi_flow_data *data_key,
			const struct clsemi_flow_data *data_act,
			const struct flow_action_entry *act_cls_meta)
{
	struct hw_ipv6_key *ipv6_key = &flow->ipv6_key;
	struct hw_ipv6_act ipv6_act = {};
	int err;

	ipv6_key->in_ifindex = data_key->in_ifindex;
	memcpy(&ipv6_key->src_v6, &data_key->v6.src_addr, sizeof(data_key->v6.src_addr));
	memcpy(&ipv6_key->dst_v6, &data_key->v6.dst_addr, sizeof(data_key->v6.dst_addr));
	ipv6_key->src_port = data_key->src_port;
	ipv6_key->dst_port = data_key->dst_port;
	ipv6_key->ip_proto = data_key->ip_proto;

	ipv6_act.out_dev = data_act->redirect_dev;
	ether_addr_copy(ipv6_act.dst_mac, data_act->eth.h_dest);
	ether_addr_copy(ipv6_act.src_mac, data_act->eth.h_source);

	if (data_act->pppoe.encap) {
		ipv6_act.push_pppoe = 1;
		ipv6_act.pppoe_sid = data_act->pppoe.sid;
	}

	if (data_act->vlan.vid) {
		ipv6_act.inner_vlan.tpid = data_act->vlan.proto;
		ipv6_act.inner_vlan.vid = data_act->vlan.vid;
		ipv6_act.inner_vlan.pcp = data_act->vlan.prio;
		ipv6_act.inner_vlan.dei = 0;
	}

	if (act_cls_meta) {
		ipv6_act.replace_dscp	= act_cls_meta->cls_meta.set_dscp;
		ipv6_act.dscp_value	= act_cls_meta->cls_meta.dscp;
		ipv6_act.opt_queue	= act_cls_meta->cls_meta.opt_queue;
		ipv6_act.queue_value	= act_cls_meta->cls_meta.queue;
	}

	err = hw_ipv6_add(flow, &ipv6_act);
	if (err == -EEXIST) {
		hw_ipv6_del(flow);
		err = hw_ipv6_add(flow, &ipv6_act);
	}

	if (!err)
		sw_ipv6_add(ipv6_key, ipv6_act.out_dev);

	return err;
}

static int offload_acl(struct clsemi_flow_entry *flow,
		       const struct clsemi_flow_data *data_key,
		       const struct clsemi_flow_data *data_mask,
		       const struct clsemi_flow_data *data_act)
{
	struct hw_acl_match *match = &flow->acl_match;

	match->in_ifindex = data_key->in_ifindex;

	ether_addr_copy(match->key.dmac, data_key->eth.h_dest);
	ether_addr_copy(match->key.smac, data_key->eth.h_source);
	ether_addr_copy(match->mask.dmac, data_mask->eth.h_dest);
	ether_addr_copy(match->mask.smac, data_mask->eth.h_source);
	match->key.h_proto = data_key->eth.h_proto;
	match->mask.h_proto = data_mask->eth.h_proto;

	switch (match->key.h_proto) {
	case htons(ETH_P_IP):
		match->key.sip4 = data_key->v4.src_addr;
		match->key.dip4 = data_key->v4.dst_addr;
		match->mask.sip4 = data_mask->v4.src_addr;
		match->mask.dip4 = data_mask->v4.dst_addr;
		break;
	case htons(ETH_P_IPV6):
		match->key.sip6 = data_key->v6.src_addr;
		match->key.dip6 = data_key->v6.dst_addr;
		match->mask.sip6 = data_mask->v6.src_addr;
		match->mask.dip6 = data_mask->v6.dst_addr;
		break;
	}

	match->key.ip_proto	= data_key->ip_proto;
	match->key.sport	= data_key->src_port;
	match->key.dport	= data_key->dst_port;
	match->mask.ip_proto	= data_mask->ip_proto;
	match->mask.sport	= data_mask->src_port;
	match->mask.dport	= data_mask->dst_port;

	return hw_acl_add(flow, &data_act->acl_act);
}

#define MATCH_IS_MASKED(member) memchr_inv(&match.mask->member, 0, sizeof(match.mask->member))

static int clsemi_flow_offload_replace(const struct clsemi_tc_cb_priv *cb_priv, struct flow_cls_offload *f)
{
	const struct flow_rule *rule = flow_cls_offload_flow_rule(f);
	const struct flow_action_entry *act, *act_cls_meta = NULL;
	struct clsemi_flow_data data_key = {};
	struct clsemi_flow_data data_mask = {};
	struct clsemi_flow_data data_act = {};
	struct clsemi_flow_entry *entry = NULL;
	enum offload_type offload_type;
	u32 match_flags = 0;
	u16 addr_type = 0;
	int err = 0;
	int i;
	__be16 n_proto = 0;
	union {
		u64 rate_bytes_ps;
		struct {
			u64     min_bytes_ps:30,
				max_bytes_ps:30,
				unused:4;
		};
	} rate;

	if (rhashtable_lookup(&clsemi_flow_table, &f->cookie, clsemi_rhash_params))
		return -EEXIST;

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_META)) {
		struct flow_match_meta match;

		flow_rule_match_meta(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_META\n"
			"mask {\n"
			"    ingress_ifindex = 0x%08x\n"
			"    ingress_iftype  = 0x%08x\n"
			"} key {\n"
			"    ingress_ifindex = 0x%08x\n"
			"    ingress_iftype  = 0x%08x\n"
			"}\n",
			match.mask->ingress_ifindex,
			match.mask->ingress_iftype,
			match.key->ingress_ifindex,
			match.key->ingress_iftype);
		data_key.in_ifindex = match.key->ingress_ifindex;
		data_mask.in_ifindex = match.mask->ingress_ifindex;
		if (MATCH_IS_MASKED(ingress_ifindex))
			match_flags |= MATCH_IN_IFINDEX;
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_CONTROL)) {
		struct flow_match_control match;

		flow_rule_match_control(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_CONTROL\n"
			"mask {\n"
			"    thoff     = 0x%04x\n"
			"    addr_type = 0x%04x\n"
			"    flags     = 0x%08x\n"
			"} key {\n"
			"    thoff     = 0x%04x\n"
			"    addr_type = 0x%04x\n"
			"    flags     = 0x%08x\n"
			"}\n",
			match.mask->thoff,
			match.mask->addr_type,
			match.mask->flags,
			match.key->thoff,
			match.key->addr_type,
			match.key->flags);
		addr_type = match.key->addr_type;
	} else if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_IPV4_ADDRS)) {
		addr_type = FLOW_DISSECTOR_KEY_IPV4_ADDRS;
	} else if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_IPV6_ADDRS)) {
		addr_type = FLOW_DISSECTOR_KEY_IPV6_ADDRS;
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_BASIC)) {
		struct flow_match_basic match;

		flow_rule_match_basic(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_BASIC\n"
			"mask {\n"
			"    n_proto  = 0x%04x\n"
			"    ip_proto = 0x%02x\n"
			"} key {\n"
			"    n_proto  = 0x%04x\n"
			"    ip_proto = %u\n"
			"}\n",
			match.mask->n_proto,
			match.mask->ip_proto,
			ntohs(match.key->n_proto),
			match.key->ip_proto);

		n_proto = match.key->n_proto;
		data_key.ip_proto = match.key->ip_proto;
		data_key.eth.h_proto = match.key->n_proto;
		data_mask.ip_proto = match.mask->ip_proto;
		data_mask.eth.h_proto = match.mask->n_proto;

		if (MATCH_IS_MASKED(n_proto))
			match_flags |= MATCH_NF_PROTO;
		if (MATCH_IS_MASKED(ip_proto))
			match_flags |= MATCH_IP_PROTO;
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_ETH_ADDRS)) {
		struct flow_match_eth_addrs match;

		flow_rule_match_eth_addrs(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_ETH_ADDRS\n"
			"mask {\n"
			"    src = %pM\n"
			"    dst = %pM\n"
			"} key {\n"
			"    src = %pM\n"
			"    dst = %pM\n"
			"}\n",
			match.mask->src,
			match.mask->dst,
			match.key->src,
			match.key->dst);
		ether_addr_copy(data_key.eth.h_dest, match.key->dst);
		ether_addr_copy(data_key.eth.h_source, match.key->src);
		ether_addr_copy(data_mask.eth.h_dest, match.mask->dst);
		ether_addr_copy(data_mask.eth.h_source, match.mask->src);
		if (MATCH_IS_MASKED(src))
			match_flags |= MATCH_ETH_SADDR;
		if (MATCH_IS_MASKED(dst))
			match_flags |= MATCH_ETH_DADDR;
	}

	if (addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS) {
		struct flow_match_ipv4_addrs match;

		flow_rule_match_ipv4_addrs(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_IPV4_ADDRS\n"
			"mask {\n"
			"    src = %pI4\n"
			"    dst = %pI4\n"
			"} key {\n"
			"    src = %pI4\n"
			"    dst = %pI4\n"
			"}\n",
			&match.mask->src,
			&match.mask->dst,
			&match.key->src,
			&match.key->dst);
		data_key.v4.src_addr = match.key->src;
		data_key.v4.dst_addr = match.key->dst;
		data_mask.v4.src_addr = match.mask->src;
		data_mask.v4.dst_addr = match.mask->dst;
		if (MATCH_IS_MASKED(src))
			match_flags |= MATCH_IPV4_SADDR;
		if (MATCH_IS_MASKED(dst))
			match_flags |= MATCH_IPV4_DADDR;
	} else if (addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS) {
		struct flow_match_ipv6_addrs match;

		flow_rule_match_ipv6_addrs(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_IPV6_ADDRS\n"
			"mask {\n"
			"    src = %pI6\n"
			"    dst = %pI6\n"
			"} key {\n"
			"    src = %pI6\n"
			"    dst = %pI6\n"
			"}\n",
			&match.mask->src,
			&match.mask->dst,
			&match.key->src,
			&match.key->dst);
		data_key.v6.src_addr = match.key->src;
		data_key.v6.dst_addr = match.key->dst;
		data_mask.v6.src_addr = match.mask->src;
		data_mask.v6.dst_addr = match.mask->dst;
		if (MATCH_IS_MASKED(src))
			match_flags |= MATCH_IPV6_SADDR;
		if (MATCH_IS_MASKED(dst))
			match_flags |= MATCH_IPV6_DADDR;
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_PORTS)) {
		struct flow_match_ports match;

		flow_rule_match_ports(rule, &match);
		cls_dbg("FLOW_DISSECTOR_KEY_PORTS\n"
			"mask {\n"
			"    src = 0x%04x\n"
			"    dst = 0x%04x\n"
			"} key {\n"
			"    src = %u\n"
			"    dst = %u\n"
			"}\n",
			match.mask->src,
			match.mask->dst,
			ntohs(match.key->src),
			ntohs(match.key->dst));
		data_key.src_port = match.key->src;
		data_key.dst_port = match.key->dst;
		data_mask.src_port = match.mask->src;
		data_mask.dst_port = match.mask->dst;
		if (MATCH_IS_MASKED(src))
			match_flags |= MATCH_TH_SPORT;
		if (MATCH_IS_MASKED(dst))
			match_flags |= MATCH_TH_DPORT;
	}

	flow_action_for_each(i, act, &rule->action) {
		switch (act->id) {
		case FLOW_ACTION_ADD:
		case FLOW_ACTION_MANGLE:
			cls_dbg("%s {\n"
				"    htpye  = %d\n"
				"    offset = %u\n"
				"    mask   = 0x%08x\n"
				"    val    = 0x%08x\n"
				"}\n",
				act->id == FLOW_ACTION_ADD ? "FLOW_ACTION_ADD" : "FLOW_ACTION_MANGLE",
				act->mangle.htype,
				act->mangle.offset,
				act->mangle.mask,
				act->mangle.val);
			err = parse_mangle(act, &data_act, &n_proto);
			break;
		case FLOW_ACTION_REDIRECT:
			cls_dbg("FLOW_ACTION_REDIRECT {\n"
				"    name    = %s\n"
				"    ifindex = %d\n"
				"}\n",
				act->dev->name,
				act->dev->ifindex);
			if (data_key.in_ifindex == act->dev->ifindex) {
				cls_dbg("%s: don't allow redirect packet to source port\n", __func__);
				return -EINVAL;
			}
			data_act.redirect_dev = act->dev;
			data_act.action_flags |= ACTION_REDIRECT;
			data_act.acl_act.act_redirect = 1;
			data_act.acl_act.out_if_port = act->dev->if_port;
			break;
		case FLOW_ACTION_VLAN_PUSH:
			cls_dbg("FLOW_ACTION_VLAN_PUSH {\n"
				"    proto = %04x\n"
				"    vid   = %u\n"
				"    prio  = %u\n"
				"}\n",
				act->vlan.proto,
				act->vlan.vid,
				act->vlan.prio);
			data_act.vlan.proto = act->vlan.proto,
			data_act.vlan.vid = act->vlan.vid;
			data_act.vlan.prio = act->vlan.prio;
			break;
		case FLOW_ACTION_PPPOE_PUSH:
			cls_dbg("FLOW_ACTION_PPPOE_PUSH {\n"
				"    sid = %u\n"
				"}\n",
				act->pppoe.sid);
			if (data_act.pppoe.encap >= 1)
				return -EOPNOTSUPP;
			data_act.pppoe.sid = htons(act->pppoe.sid);
			data_act.pppoe.encap++;
			break;
		case FLOW_ACTION_CSUM:
			cls_dbg("FLOW_ACTION_CSUM {\n"
				"    csum_flags = 0x%08x\n"
				"}\n",
				act->csum_flags);
			data_act.action_flags |= ACTION_CSUM;
			break;
		case FLOW_ACTION_DROP:
			cls_dbg("FLOW_ACTION_DROP\n");
			data_act.action_flags |= ACTION_DROP;
			data_act.acl_act.act_drop = 1;
			break;
		case FLOW_ACTION_CLS_META:
			cls_dbg("FLOW_ACTION_CLS_META {\n"
				"    opt_queue = %u\n"
				"    queue     = %u\n"
				"    set_dscp  = %u\n"
				"    dscp      = %u\n"
				"}\n",
				act->cls_meta.opt_queue,
				act->cls_meta.queue,
				act->cls_meta.set_dscp,
				act->cls_meta.dscp);
			act_cls_meta = act;
			break;
		case FLOW_ACTION_POLICE:
			rate.rate_bytes_ps = act->police.rate_bytes_ps;
			cls_dbg("FLOW_ACTION_POLICE {\n"
				"    index         = %u\n"
				"    burst         = %u\n"
				"    rate_bytes_ps = %llu\n"
				"    min_bytes_ps  = %u\n"
				"    max_bytes_ps  = %u\n"
				"    mtu           = %u\n"
				"}\n",
				act->police.index,
				act->police.burst,
				act->police.rate_bytes_ps,
				rate.min_bytes_ps,
				rate.max_bytes_ps,
				act->police.mtu);
			data_act.action_flags |= ACTION_RATE_CTL;
			data_act.acl_act.act_rate_ctl = 1;
			data_act.acl_act.rate.min_bytes_ps = rate.min_bytes_ps;
			data_act.acl_act.rate.max_bytes_ps = rate.max_bytes_ps;
			break;
		case FLOW_ACTION_PRIORITY:
			cls_dbg("FLOW_ACTION_PRIORITY {\n"
				"    priority = %u\n"
				"}\n",
				act->priority);
			if (act->priority > 7) {
				cls_dbg("%s: invalid priority, must be 0~7\n", __func__);
				return -EINVAL;
			}
			data_act.action_flags |= ACTION_PRIORITY;
			data_act.acl_act.act_priority = 1;
			data_act.acl_act.priority = act->priority;
			break;
		default:
			cls_dbg("%s: don't support action id %d\n", __func__, act->id);
			return -EOPNOTSUPP;
		}

		if (err)
			return err;
	}

	if (data_key.eth.h_proto != n_proto) {
		data_key.eth.h_proto = n_proto;
		data_mask.eth.h_proto = ~0;
	}

	if (!(match_flags & MATCH_IN_IFINDEX)) {
		data_key.in_ifindex = MATCH_ALL_IFINDEX;
		data_mask.in_ifindex = ~0;
	}

	offload_type = determine_offload_type(match_flags, data_act.action_flags);
	if (offload_type == OFFLOAD_TYPE_UNSUPPORT)
		return -EOPNOTSUPP;

	cls_dbg("%s: match_flags=0x%08x action_flags=0x%08x offload_type=%d\n",
		__func__, match_flags, data_act.action_flags, offload_type);

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	entry->cookie = f->cookie;
	entry->offload_type = offload_type;

	err = rhashtable_insert_fast(&clsemi_flow_table, &entry->node, clsemi_rhash_params);
	if (err)
		goto error;

	if (offload_type == OFFLOAD_TYPE_IPV4_NAT)
		err = offload_nat(entry, &data_key, &data_act, act_cls_meta);
	else if (offload_type == OFFLOAD_TYPE_IPV6_ROUTE)
		err = offload_ipv6(entry, &data_key, &data_act, act_cls_meta);
	else if (offload_type == OFFLOAD_TYPE_ACL)
		err = offload_acl(entry, &data_key, &data_mask, &data_act);

	if (err) {
		rhashtable_remove_fast(&clsemi_flow_table, &entry->node, clsemi_rhash_params);
		goto error;
	}

	WRITE_ONCE(entry->lastused, jiffies);

	return 0;

error:
	kfree_rcu(entry, rcu);
	return err;
}

static int clsemi_flow_offload_destroy(const struct clsemi_tc_cb_priv *cb_priv, struct flow_cls_offload *f)
{
	struct clsemi_flow_entry *entry = NULL;

	entry = rhashtable_lookup(&clsemi_flow_table, &f->cookie, clsemi_rhash_params);
	if (!entry)
		return -ENOENT;

	if (entry->offload_type == OFFLOAD_TYPE_IPV4_NAT) {
		sw_nat_del(&entry->nat_key);
		hw_nat_del(entry);
	} else if (entry->offload_type == OFFLOAD_TYPE_IPV6_ROUTE) {
		sw_ipv6_del(&entry->ipv6_key);
		hw_ipv6_del(entry);
	} else if (entry->offload_type == OFFLOAD_TYPE_ACL)
		hw_acl_del(entry);

	rhashtable_remove_fast(&clsemi_flow_table, &entry->node, clsemi_rhash_params);
	kfree_rcu(entry, rcu);

	return 0;
}

static int clsemi_flow_offload_stats(const struct clsemi_tc_cb_priv *cb_priv, struct flow_cls_offload *f)
{
	struct clsemi_flow_entry *entry = NULL;

	entry = rhashtable_lookup(&clsemi_flow_table, &f->cookie, clsemi_rhash_params);
	if (!entry)
		return -ENOENT;

	f->stats.lastused = READ_ONCE(entry->lastused);

	return 0;
}

static void clsemi_flow_offload_find_tbd_flow(struct flow_cls_offload *f)
{
	struct rhashtable_iter iter;
	struct clsemi_flow_entry *entry;
	struct flow_offload *flow, *tbd_flow = NULL;

	rhashtable_walk_enter(&clsemi_flow_table, &iter);
	rhashtable_walk_start(&iter);

	while ((entry = rhashtable_walk_next(&iter)) != NULL) {
		if (IS_ERR(entry))
			continue;

		if (entry->offload_type != OFFLOAD_TYPE_IPV4_NAT &&
		    entry->offload_type != OFFLOAD_TYPE_IPV6_ROUTE)
			continue;

		flow = cookie_to_flow(entry->cookie);
		if (flow->ct->cls_is_vip)
			continue;

		if (!tbd_flow || time_before32(READ_ONCE(flow->hw_lastused), READ_ONCE(tbd_flow->hw_lastused)))
			tbd_flow = flow;
	}

	rhashtable_walk_stop(&iter);
	rhashtable_walk_exit(&iter);

	*((struct flow_offload **)f->common.extack->cookie) = tbd_flow;
}

static void clsemi_flow_stats(void)
{
	struct rhashtable_iter iter;
	struct clsemi_flow_entry *entry;
	bool hit;
	struct flow_offload *flow;

	if (time_before(jiffies, READ_ONCE(flow_stats_ts) + HZ))
		return;

	WRITE_ONCE(flow_stats_ts, jiffies);

	rhashtable_walk_enter(&clsemi_flow_table, &iter);
	rhashtable_walk_start(&iter);

	while ((entry = rhashtable_walk_next(&iter)) != NULL) {
		if (IS_ERR(entry))
			continue;

		if (entry->offload_type == OFFLOAD_TYPE_IPV4_NAT)
			hit = hw_nat_read_hit_and_reset(entry);
		else if (entry->offload_type == OFFLOAD_TYPE_IPV6_ROUTE)
			hit = hw_ipv6_read_hit_and_reset(entry);
		else if (entry->offload_type == OFFLOAD_TYPE_ACL)
			hit = hw_acl_read_hit_and_reset(entry);

		if (hit) {
			WRITE_ONCE(entry->lastused, jiffies);
			if (entry->offload_type == OFFLOAD_TYPE_IPV4_NAT ||
			    entry->offload_type == OFFLOAD_TYPE_IPV6_ROUTE) {
				flow = cookie_to_flow(entry->cookie);
				WRITE_ONCE(flow->hw_lastused, nf_flowtable_time_stamp);
				WRITE_ONCE(flow->timeout, nf_flowtable_time_stamp + flow_offload_get_timeout(flow));
			}
		}
	}

	rhashtable_walk_stop(&iter);
	rhashtable_walk_exit(&iter);
}

static void clsemi_flow_offload_cleanup(void)
{
	struct rhashtable_iter iter;
	struct clsemi_flow_entry *entry;
	struct flow_offload *flow;
	int count = 0;

	rhashtable_walk_enter(&clsemi_flow_table, &iter);
	rhashtable_walk_start(&iter);

	while ((entry = rhashtable_walk_next(&iter)) != NULL) {
		if (IS_ERR(entry))
			continue;
		if (entry->offload_type == OFFLOAD_TYPE_IPV4_NAT) {
			flow = cookie_to_flow(entry->cookie);
			if (test_bit(NF_FLOW_HW_DYING, &flow->flags)) {
				sw_nat_del(&entry->nat_key);
				hw_nat_del_step1(entry);
				entry->flags |= CLS_FLOW_HW_DEL_STEP1_DONE;
				count++;
			}
		} else if (entry->offload_type == OFFLOAD_TYPE_IPV6_ROUTE) {
			flow = cookie_to_flow(entry->cookie);
			if (test_bit(NF_FLOW_HW_DYING, &flow->flags)) {
				sw_ipv6_del(&entry->ipv6_key);
				hw_ipv6_del_step1(entry);
				entry->flags |= CLS_FLOW_HW_DEL_STEP1_DONE;
				count++;
			}
		}
	}

	rhashtable_walk_stop(&iter);
	rhashtable_walk_exit(&iter);

	if (!count)
		return;

	hw_wait_for_del();

	rhashtable_walk_enter(&clsemi_flow_table, &iter);
	rhashtable_walk_start(&iter);

	while ((entry = rhashtable_walk_next(&iter)) != NULL) {
		if (IS_ERR(entry))
			continue;
		if (entry->offload_type == OFFLOAD_TYPE_IPV4_NAT) {
			flow = cookie_to_flow(entry->cookie);
			if (entry->flags & CLS_FLOW_HW_DEL_STEP1_DONE) {
				hw_nat_del_step2(entry);
				rhashtable_remove_fast(&clsemi_flow_table, &entry->node, clsemi_rhash_params);
				kfree_rcu(entry, rcu);
			}
		} else if (entry->offload_type == OFFLOAD_TYPE_IPV6_ROUTE) {
			flow = cookie_to_flow(entry->cookie);
			if (entry->flags & CLS_FLOW_HW_DEL_STEP1_DONE) {
				hw_ipv6_del_step2(entry);
				rhashtable_remove_fast(&clsemi_flow_table, &entry->node, clsemi_rhash_params);
				kfree_rcu(entry, rcu);
			}
		}
	}

	rhashtable_walk_stop(&iter);
	rhashtable_walk_exit(&iter);
}

static int clsemi_tc_setup_flower(const struct clsemi_tc_cb_priv *cb_priv, struct flow_cls_offload *f)
{
	int ret;
	cls_dbg("%s: frontend=%s command=%d cookie=0x%lx\n",
		__func__, get_frontend_name(cb_priv), f->command, f->cookie);

	switch (f->command) {
	case FLOW_CLS_REPLACE:
		mutex_lock(&clsemi_flow_offload_mutex);
		ret = clsemi_flow_offload_replace(cb_priv, f);
		clsemi_flow_stats();
		mutex_unlock(&clsemi_flow_offload_mutex);
		return ret;
	case FLOW_CLS_DESTROY:
		mutex_lock(&clsemi_flow_offload_mutex);
		ret = clsemi_flow_offload_destroy(cb_priv, f);
		clsemi_flow_stats();
		mutex_unlock(&clsemi_flow_offload_mutex);
		return ret;
	case FLOW_CLS_CLEANUP:
		mutex_lock(&clsemi_flow_offload_mutex);
		clsemi_flow_offload_cleanup();
		mutex_unlock(&clsemi_flow_offload_mutex);
		return 0;
	case FLOW_CLS_FIND_TDB_FLOW:
		mutex_lock(&clsemi_flow_offload_mutex);
		clsemi_flow_offload_find_tbd_flow(f);
		mutex_unlock(&clsemi_flow_offload_mutex);
		return 0;
	case FLOW_CLS_STATS:
		return clsemi_flow_offload_stats(cb_priv, f);
	default:
		return -EOPNOTSUPP;
	}
}

static int clsemi_setup_tc_block_cb(enum tc_setup_type type, void *type_data, void *cb_priv)
{
	int err;

	if (type != TC_SETUP_CLSFLOWER)
		return -EOPNOTSUPP;

	switch (type) {
	case TC_SETUP_CLSFLOWER:
		err = clsemi_tc_setup_flower(cb_priv, type_data);
		break;
	default:
		err = -EOPNOTSUPP;
		break;
	}

	return err;
}

static void clsemi_eth_setup_hw_fwd_cap(struct net_device *dev, struct flow_block_offload *f)
{
	struct hw_flowtable_cap hw_cap = {};

	if (!f->extack)
		return;

	hw_cap.magic = HW_FT_CAP_MAGIC;
	/* 1 flow use 2 HW entries */
	hw_cap.hw_ft_size = hw_flow_offload_get_max_entries() / 2;

	memcpy(f->extack->cookie, &hw_cap, sizeof(hw_cap));
	f->extack->cookie_len = sizeof(hw_cap);
}

static void clsemi_tc_block_release(void *cb_priv)
{
	kfree(cb_priv);
}

static int clsemi_setup_tc_block(struct net_device *dev, enum tc_setup_type type, struct flow_block_offload *f)
{
	static LIST_HEAD(block_cb_list);
	struct flow_block_cb *block_cb;
	struct clsemi_tc_cb_priv *cb_priv;
	flow_setup_cb_t *cb = clsemi_setup_tc_block_cb;
	/* ACL is shared by all port in NPE, only one block_cb is necessary in cb_list */
	void *cb_ident = clsemi_setup_tc_block_cb;

	if (f->binder_type != FLOW_BLOCK_BINDER_TYPE_CLSACT_INGRESS)
		return -EOPNOTSUPP;

	f->driver_block_list = &block_cb_list;

	switch (f->command) {
	case FLOW_BLOCK_BIND:
		block_cb = flow_block_cb_lookup(f->block, cb, cb_ident);
		if (block_cb) {
			flow_block_cb_incref(block_cb);
			return 0;
		}

		cb_priv = kzalloc(sizeof(*cb_priv), GFP_KERNEL);
		if (IS_ERR(cb_priv))
			return PTR_ERR(cb_priv);

		if (type == TC_SETUP_FT) {
			cb_priv->frontend_is_flowtable = true;
			cb_priv->flowtable = container_of(f->block, struct nf_flowtable, flow_block);
		} else {
			cb_priv->frontend_is_flowtable = false;
			cb_priv->dev = dev;
		}

		block_cb = flow_block_cb_alloc(cb, cb_ident, cb_priv, clsemi_tc_block_release);
		if (IS_ERR(block_cb))
			return PTR_ERR(block_cb);

		flow_block_cb_incref(block_cb);
		flow_block_cb_add(block_cb, f);
		list_add_tail(&block_cb->driver_list, &block_cb_list);
		return 0;
	case FLOW_BLOCK_UNBIND:
		block_cb = flow_block_cb_lookup(f->block, cb, cb_ident);
		if (!block_cb)
			return -ENOENT;

		if (!flow_block_cb_decref(block_cb)) {
			flow_block_cb_remove(block_cb, f);
			list_del(&block_cb->driver_list);
		}
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int clsemi_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data)
{
	switch (type) {
	case TC_SETUP_BLOCK:
		return clsemi_setup_tc_block(dev, type, type_data);
	case TC_SETUP_FT:
		clsemi_eth_setup_hw_fwd_cap(dev, type_data);
		return clsemi_setup_tc_block(dev, type, type_data);
	default:
		return -EOPNOTSUPP;
	}
}

static int dev_enable_hw_tc(struct net_device *dev)
{
	struct net_device_ops *ops;
	struct orig_ndev_ops *node;

	if (dev->type != ARPHRD_ETHER || dev->flags & IFF_LOOPBACK ||
	    netif_is_bridge_master(dev))
		return -ENOTSUPP;

	if (dev->ieee80211_ptr) {
		pr_info("enable flowtable support on hotplug dev %s\n", dev->name);
		dev->hotplug_dev_flow_offload = true;
		return 0;
	}

	if (dev->netdev_ops->ndo_setup_tc == clsemi_setup_tc)
		return 0;

	ops = kzalloc(sizeof(*ops), GFP_KERNEL);
	if (!ops)
		return -ENOMEM;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		kfree(ops);
		return -ENOMEM;
	}

	node->dev = dev;
	node->orig_ops = dev->netdev_ops;
	list_add(&node->list, &orig_ndev_ops_list);

	memcpy(ops, dev->netdev_ops, sizeof(*ops));
	ops->ndo_setup_tc = clsemi_setup_tc;

	dev->netdev_ops = ops;
	dev->hw_features |= NETIF_F_HW_TC;
	dev->wanted_features |= NETIF_F_HW_TC;
	netdev_update_features(dev);
	pr_info("enable NETIF_F_HW_TC on %s\n", dev->name);

	return 0;
}

static int tc_flow_offload_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct orig_ndev_ops *node, *next;

	if (event == NETDEV_REGISTER) {
		dev_enable_hw_tc(dev);
	} else if (event == NETDEV_UNREGISTER) {
		list_for_each_entry_safe(node, next, &orig_ndev_ops_list, list) {
			if (node->dev == dev) {
				kfree(dev->netdev_ops);
				dev->netdev_ops = node->orig_ops;
				list_del(&node->list);
				kfree(node);
				break;
			}
		}
	}

	return NOTIFY_DONE;
}

static struct notifier_block tc_flow_offload_notifier = {
	.notifier_call = tc_flow_offload_event,
};

static int clsemi_flow_table_show(struct seq_file *f, void *offset)
{
	struct rhashtable_iter iter;
	struct clsemi_flow_entry *entry = NULL;
	const struct hw_nat_key *key4;
	const struct hw_ipv6_key *key6;

	seq_printf(f, "stats at %u ms ago\n", jiffies_to_msecs(jiffies - READ_ONCE(flow_stats_ts)));
	rhashtable_walk_enter(&clsemi_flow_table, &iter);
	rhashtable_walk_start(&iter);

	while ((entry = rhashtable_walk_next(&iter)) != NULL) {
		if (IS_ERR(entry))
			continue;

		if (entry->offload_type == OFFLOAD_TYPE_IPV4_NAT) {
			key4 = &entry->nat_key;
			seq_printf(f, "ipv4 %s %-15pI4:%-5hu -> %-15pI4:%-5hu 0x%02x 0x%08x %-4u %-4u %u\n",
				   key4->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				   &key4->src_addr, ntohs(key4->src_port),
				   &key4->dst_addr, ntohs(key4->dst_port),
				   entry->flags,
				   entry->npe_acl_index,
				   entry->npe_nexthop_index,
				   entry->npe_nat_index,
				   jiffies_to_msecs(jiffies - READ_ONCE(entry->lastused)));
		} else if (entry->offload_type == OFFLOAD_TYPE_IPV6_ROUTE) {
			key6 = &entry->ipv6_key;
			seq_printf(f, "ipv6 %s %pI6:%-5hu -> %pI6:%-5hu 0x%02x 0x%08x %-4u %-4u %u\n",
				   key6->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				   &key6->src_v6, ntohs(key6->src_port),
				   &key6->dst_v6, ntohs(key6->dst_port),
				   entry->flags,
				   entry->npe_acl_index,
				   entry->npe_nexthop_index,
				   entry->npe_nat_index,
				   jiffies_to_msecs(jiffies - READ_ONCE(entry->lastused)));
		}
	}
	rhashtable_walk_stop(&iter);
	rhashtable_walk_exit(&iter);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(clsemi_flow_table);

static void clsemi_flow_stats_work(struct work_struct *work)
{
	unsigned long interval = HZ;

	if (time_before(jiffies, READ_ONCE(flow_stats_ts) + HZ)) {
		queue_delayed_work_on(0, clsemi_flow_wq, &flow_stats_work, HZ);
		return;
	}

	if (mutex_trylock(&clsemi_flow_offload_mutex)) {
		clsemi_flow_stats();
		mutex_unlock(&clsemi_flow_offload_mutex);
	} else {
		interval = msecs_to_jiffies(10);
	}

	queue_delayed_work_on(0, clsemi_flow_wq, &flow_stats_work, interval);
}

static __init int tc_flow_offload_init(void)
{
	struct net_device *dev;

	hw_flow_offload_init();
	non_npe_dev_l3_fwt_init();
	rhashtable_init(&clsemi_flow_table, &clsemi_rhash_params);
	register_netdevice_notifier(&tc_flow_offload_notifier);
	debugfs_create_file("clsemi_flow_table", 0600, NULL, NULL, &clsemi_flow_table_fops);
	INIT_DELAYED_WORK(&flow_stats_work, clsemi_flow_stats_work);
	clsemi_flow_wq = create_singlethread_workqueue("clsemi_flow_wq");
	queue_delayed_work_on(0, clsemi_flow_wq, &flow_stats_work, HZ);

	for_each_netdev(&init_net, dev) {
		rtnl_lock();
		dev_enable_hw_tc(dev);
		rtnl_unlock();
	}

	return 0;
}
module_init(tc_flow_offload_init);

/* don't implement exit function to make this module permanent */

MODULE_DESCRIPTION("Clourneysemi tc hw flow offload driver");
MODULE_LICENSE("GPL");
