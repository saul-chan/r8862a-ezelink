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

#include "common.h"

#define HW_OFFLOAD_MAX_ENTRIES 2000

int hw_acl_add(struct clsemi_flow_entry *flow, const struct hw_acl_act *act)
{
	/* TODO: save the npe_acl_index if successfully add entry to NPE */

	return -EOPNOTSUPP;
}

int hw_acl_del(const struct clsemi_flow_entry *flow)
{
	/* TODO: delete entry from NPE by npe_acl_index */

	return 0;
}

bool hw_acl_read_hit_and_reset(const struct clsemi_flow_entry *flow)
{
	/* TODO: read and reset hit from NPE by npe_acl_index */

	return true;
}

int hw_nat_add(struct clsemi_flow_entry *flow, const struct hw_nat_act *act)
{
	const struct hw_nat_key *key = &flow->nat_key;
	int ret;

	ret = cls_npe_tc_add_nat_entry(key, act);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %-15pI4:%-5hu -> %-15pI4:%-5hu\n", __func__, ret,
				    &key->src_addr, ntohs(key->src_port), &key->dst_addr, ntohs(key->dst_port));
		flow->npe_acl_index = -EINVAL;
		return -EINVAL;
	}

	flow->npe_acl_index = ret;

	return 0;
}

int hw_nat_del(const struct clsemi_flow_entry *flow)
{
	const struct hw_nat_key *key = &flow->nat_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return -EINVAL;

	ret = cls_npe_tc_del_nat_entry(flow, true, true);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %-15pI4:%-5hu -> %-15pI4:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_addr, ntohs(key->src_port), &key->dst_addr, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return -EINVAL;
	}

	return 0;
}

int hw_nat_del_step1(const struct clsemi_flow_entry *flow)
{
	const struct hw_nat_key *key = &flow->nat_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return -EINVAL;

	ret = cls_npe_tc_del_nat_entry(flow, true, false);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %-15pI4:%-5hu -> %-15pI4:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_addr, ntohs(key->src_port), &key->dst_addr, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return -EINVAL;
	}

	return 0;
}

int hw_nat_del_step2(const struct clsemi_flow_entry *flow)
{
	const struct hw_nat_key *key = &flow->nat_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return -EINVAL;

	ret = cls_npe_tc_del_nat_entry(flow, false, true);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %-15pI4:%-5hu -> %-15pI4:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_addr, ntohs(key->src_port), &key->dst_addr, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return -EINVAL;
	}

	return 0;
}

bool hw_nat_read_hit_and_reset(const struct clsemi_flow_entry *flow)
{
	const struct hw_nat_key *key = &flow->nat_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return false;

	ret = cls_npe_tc_get_nat_hit(flow->npe_acl_index);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %-15pI4:%-5hu -> %-15pI4:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_addr, ntohs(key->src_port), &key->dst_addr, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return false;
	}

	return ret;
}

int hw_ipv6_add(struct clsemi_flow_entry *flow, const struct hw_ipv6_act *act)
{
	const struct hw_ipv6_key *key = &flow->ipv6_key;
	int ret;

	ret = cls_npe_tc_add_ipv6_entry(key, act);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %pI6:%-5hu -> %pI6:%-5hu\n", __func__, ret,
				    &key->src_v6, ntohs(key->src_port), &key->dst_v6, ntohs(key->dst_port));
		flow->npe_acl_index = -EINVAL;
		return -EINVAL;
	}

	flow->npe_acl_index = ret;

	return 0;
}

int hw_ipv6_del(const struct clsemi_flow_entry *flow)
{
	const struct hw_ipv6_key *key = &flow->ipv6_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return -EINVAL;

	ret = cls_npe_tc_del_ipv6_entry(flow, true, true);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %pI6:%-5hu -> %pI6:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_v6, ntohs(key->src_port), &key->dst_v6, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return -EINVAL;
	}

	return 0;
}

int hw_ipv6_del_step1(const struct clsemi_flow_entry *flow)
{
	const struct hw_ipv6_key *key = &flow->ipv6_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return -EINVAL;

	ret = cls_npe_tc_del_ipv6_entry(flow, true, false);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %pI6:%-5hu -> %pI6:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_v6, ntohs(key->src_port), &key->dst_v6, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return -EINVAL;
	}

	return 0;
}

int hw_ipv6_del_step2(const struct clsemi_flow_entry *flow)
{
	const struct hw_ipv6_key *key = &flow->ipv6_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return -EINVAL;

	ret = cls_npe_tc_del_ipv6_entry(flow, false, true);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %pI6:%-5hu -> %pI6:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_v6, ntohs(key->src_port), &key->dst_v6, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return -EINVAL;
	}

	return 0;
}

bool hw_ipv6_read_hit_and_reset(const struct clsemi_flow_entry *flow)
{
	const struct hw_ipv6_key *key = &flow->ipv6_key;
	int ret;

	if (flow->npe_acl_index < 0)
		return false;

	ret = cls_npe_tc_get_ipv6_hit(flow->npe_acl_index);
	if (ret < 0) {
		pr_info_ratelimited("%s failed(%d): %s %pI6:%-5hu -> %pI6:%-5hu 0x%08x %u %u 0x%02x\n",
				    __func__, ret, key->ip_proto == IPPROTO_TCP ? "tcp" : "udp",
				    &key->src_v6, ntohs(key->src_port), &key->dst_v6, ntohs(key->dst_port),
				    flow->npe_acl_index, flow->npe_nexthop_index, flow->npe_nat_index, flow->flags);
		return false;
	}

	return ret;
}

void hw_wait_for_del(void)
{
	cls_npe_switch_config_delay_ms();
}

u32 hw_flow_offload_get_max_entries(void)
{
	return HW_OFFLOAD_MAX_ENTRIES;
}

int hw_flow_offload_init(void)
{
	return 0;
}
