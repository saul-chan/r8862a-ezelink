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
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter.h>
#include <net/ip.h>
#include <net/ipv6.h>

#include "common.h"

struct sw_nat_entry {
	struct rhash_head node;
	struct hw_nat_key key;
	struct net_device *out_dev;
	struct rcu_head rcu;
};

struct sw_ipv6_entry {
	struct rhash_head node;
	struct hw_ipv6_key key;
	struct net_device *out_dev;
	struct rcu_head rcu;
};

struct tcpudphdr {
	__be16 src_port;
	__be16 dst_port;
};

static struct rhashtable sw_nat_table;
static struct rhashtable sw_ipv6_table;

static const struct rhashtable_params sw_nat_rhash_params = {
	.head_offset = offsetof(struct sw_nat_entry, node),
	.key_offset = offsetof(struct sw_nat_entry, key),
	.key_len = sizeof(struct hw_nat_key),
	.automatic_shrinking = true,
};

static const struct rhashtable_params sw_ipv6_rhash_params = {
	.head_offset = offsetof(struct sw_ipv6_entry, node),
	.key_offset = offsetof(struct sw_ipv6_entry, key),
	.key_len = sizeof(struct hw_ipv6_key),
	.automatic_shrinking = true,
};

int sw_nat_add(const struct hw_nat_key *key, struct net_device *out_dev)
{
	struct sw_nat_entry *entry = NULL;
	int err = 0;

	if (__netif_is_npe_eth_port(key->in_ifindex))
		return 0;

	entry = rhashtable_lookup(&sw_nat_table, key, sw_nat_rhash_params);
	if (entry)
		return -EEXIST;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	memcpy(&entry->key, key, sizeof(*key));
	entry->out_dev = out_dev;

	err = rhashtable_insert_fast(&sw_nat_table, &entry->node, sw_nat_rhash_params);
	if (err) {
		kfree_rcu(entry, rcu);
		return err;
	}

	return 0;
}

int sw_nat_del(const struct hw_nat_key *key)
{
	struct sw_nat_entry *entry = NULL;

	entry = rhashtable_lookup(&sw_nat_table, key, sw_nat_rhash_params);
	if (!entry)
		return -ENOENT;

	rhashtable_remove_fast(&sw_nat_table, &entry->node, sw_nat_rhash_params);
	kfree_rcu(entry, rcu);

	return 0;
}

int sw_ipv6_add(const struct hw_ipv6_key *key, struct net_device *out_dev)
{
	struct sw_ipv6_entry *entry = NULL;
	int err = 0;

	if (__netif_is_npe_eth_port(key->in_ifindex))
		return 0;

	entry = rhashtable_lookup(&sw_ipv6_table, key, sw_ipv6_rhash_params);
	if (entry)
		return -EEXIST;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	memcpy(&entry->key, key, sizeof(*key));
	entry->out_dev = out_dev;

	err = rhashtable_insert_fast(&sw_ipv6_table, &entry->node, sw_ipv6_rhash_params);
	if (err) {
		kfree_rcu(entry, rcu);
		return err;
	}

	return 0;
}

int sw_ipv6_del(const struct hw_ipv6_key *key)
{
	struct sw_ipv6_entry *entry = NULL;

	entry = rhashtable_lookup(&sw_ipv6_table, key, sw_ipv6_rhash_params);
	if (!entry)
		return -ENOENT;

	rhashtable_remove_fast(&sw_ipv6_table, &entry->node, sw_ipv6_rhash_params);
	kfree_rcu(entry, rcu);

	return 0;
}

static int flow_state_check(int proto, struct sk_buff *skb, unsigned int thoff)
{
	struct tcphdr *tcph;

	if (proto != IPPROTO_TCP)
		return 0;

	tcph = (void *)(skb_network_header(skb) + thoff);
	if (unlikely(tcph->fin || tcph->rst))
		return -1;

	return 0;
}

static bool ipv4_has_options(u32 thoff)
{
	return thoff != sizeof(struct iphdr);
}

static int ipv4_flow_key(struct sk_buff *skb, struct hw_nat_key *key, u32 offset)
{
	struct tcpudphdr *ports;
	u32 thoff, hdrsize;
	struct iphdr *iph;

	if (!pskb_may_pull(skb, sizeof(*iph) + offset))
		return -1;

	iph = (struct iphdr *)(skb_network_header(skb) + offset);
	thoff = (iph->ihl * 4);

	if (ip_is_fragment(iph) ||
	    unlikely(ipv4_has_options(thoff)))
		return -1;

	thoff += offset;

	switch (iph->protocol) {
	case IPPROTO_TCP:
		hdrsize = sizeof(struct tcphdr);
		break;
	case IPPROTO_UDP:
		hdrsize = sizeof(struct udphdr);
		break;
	default:
		return -1;
	}

	if (iph->ttl <= 1)
		return -1;

	if (!pskb_may_pull(skb, thoff + hdrsize))
		return -1;

	iph = (struct iphdr *)(skb_network_header(skb) + offset);
	ports = (struct tcpudphdr *)(skb_network_header(skb) + thoff);

	key->src_addr	= iph->saddr;
	key->dst_addr	= iph->daddr;
	key->src_port	= ports->src_port;
	key->dst_port	= ports->dst_port;
	key->ip_proto	= iph->protocol;
	key->in_ifindex = skb->dev->ifindex;

	return 0;
}

static int ipv6_flow_key(struct sk_buff *skb, struct hw_ipv6_key *key, u32 offset)
{
	struct tcpudphdr *ports;
	struct ipv6hdr *ip6h;
	u32 thoff, hdrsize;
	u8 nexthdr;

	thoff = sizeof(*ip6h) + offset;
	if (!pskb_may_pull(skb, thoff))
		return -1;

	ip6h = (struct ipv6hdr *)(skb_network_header(skb) + offset);

	nexthdr = ip6h->nexthdr;
	switch (nexthdr) {
	case IPPROTO_TCP:
		hdrsize = sizeof(struct tcphdr);
		break;
	case IPPROTO_UDP:
		hdrsize = sizeof(struct udphdr);
		break;
	default:
		return -1;
	}

	if (ip6h->hop_limit <= 1)
		return -1;

	if (!pskb_may_pull(skb, thoff + hdrsize))
		return -1;

	ip6h = (struct ipv6hdr *)(skb_network_header(skb) + offset);
	ports = (struct tcpudphdr *)(skb_network_header(skb) + thoff);

	key->src_v6	= ip6h->saddr;
	key->dst_v6	= ip6h->daddr;
	key->src_port	= ports->src_port;
	key->dst_port	= ports->dst_port;
	key->ip_proto	= nexthdr;
	key->in_ifindex = skb->dev->ifindex;

	return 0;
}

static struct net_device *ipv4_flow_find_out_dev(struct sk_buff *skb, u32 offset)
{
	struct sw_nat_entry *entry = NULL;
	unsigned int thoff;
	struct hw_nat_key key = {};
	struct iphdr *iph;

	if (ipv4_flow_key(skb, &key, offset) < 0)
		return NULL;

	iph = (struct iphdr *)(skb_network_header(skb) + offset);
	thoff = (iph->ihl * 4) + offset;
	if (flow_state_check(iph->protocol, skb, thoff))
		return NULL;

	entry = rhashtable_lookup(&sw_nat_table, &key, sw_nat_rhash_params);
	if (!entry)
		return NULL;

	/* both 0x0000 and 0xffff are valid (and equivalent) for IP header checksum,
	 * but NPE drops the packet if the checksum is 0xffff, replace with 0x0000 here
	 */
	if (iph->check == 0xffff)
		iph->check = 0;

	return entry->out_dev;
}

static struct net_device *ipv6_flow_find_out_dev(struct sk_buff *skb, u32 offset)
{
	struct sw_ipv6_entry *entry = NULL;
	struct hw_ipv6_key key = {};
	unsigned int thoff;
	struct ipv6hdr *ip6h;

	if (ipv6_flow_key(skb, &key, offset) < 0)
		return NULL;

	ip6h = (struct ipv6hdr *)(skb_network_header(skb) + offset);
	thoff = sizeof(*ip6h) + offset;
	if (flow_state_check(ip6h->nexthdr, skb, thoff))
		return NULL;

	entry = rhashtable_lookup(&sw_ipv6_table, &key, sw_ipv6_rhash_params);
	if (!entry)
		return NULL;

	return entry->out_dev;
}

static struct net_device *flow_find_out_dev(struct sk_buff *skb)
{
	struct ethhdr *ehdr = eth_hdr(skb);
	struct vlan_ethhdr *vhdr;
	__be16 proto = ehdr->h_proto;
	u32 offset = 0;

	if (proto == htons(ETH_P_8021Q)) {
		if (!pskb_may_pull(skb, VLAN_HLEN))
			return NULL;
		vhdr = (struct vlan_ethhdr *)ehdr;
		proto = vhdr->h_vlan_encapsulated_proto;
		offset = VLAN_HLEN;
	}

	switch (proto) {
	case htons(ETH_P_IP):
		return ipv4_flow_find_out_dev(skb, offset);
	case htons(ETH_P_IPV6):
		return ipv6_flow_find_out_dev(skb, offset);
	}

	return NULL;
}

void non_npe_dev_l3_fwt_init(void)
{
	rhashtable_init(&sw_nat_table, &sw_nat_rhash_params);
	rhashtable_init(&sw_ipv6_table, &sw_ipv6_rhash_params);

	register_l3_find_out_dev(flow_find_out_dev);
}
