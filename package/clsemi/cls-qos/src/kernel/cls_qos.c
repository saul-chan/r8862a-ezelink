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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv6.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_tables.h>

#define APP_QOS_POLICY_PATH	"/etc/cls-qos/app_qos_policy"
#define MAX_POLICY_LINE_LEN	128
#define MIN_POLICY_LINE_LEN	8
#define MAX_APP_ID		31

enum flow_dir {
	FLOW_DIR_US,
	FLOW_DIR_DS,
	FLOW_DIR_MAX
};

struct flow_qos_policy {
	uint8_t dscp;
	/* maybe add other qos policy in future */
};

struct app_qos_policy {
	uint8_t valid;
	struct flow_qos_policy policy[FLOW_DIR_MAX];
};

static struct app_qos_policy app_qos_policy[MAX_APP_ID + 1];

static struct ctl_table_header *cls_qos_table_hdr;
static unsigned int vip_qos_policy_first;

static struct ctl_table cls_qos_table[] = {
	{
		.procname	= "vip_qos_policy_first",
		.data		= &vip_qos_policy_first,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{ }
};

static struct ctl_table cls_qos_dir_table[] = {
	{
		.procname	= "qos",
		.mode		= 0555,
		.child		= cls_qos_table,
	},
	{ }
};

static struct ctl_table cls_qos_root_table[] = {
	{
		.procname	= "cls",
		.mode		= 0555,
		.child		= cls_qos_dir_table,
	},
	{ }
};

static inline int get_classic_qos_queue(struct nf_conn *ct, int dir)
{
	switch (dir) {
	case IP_CT_DIR_ORIGINAL:
		if (ct->cls_ori_set_queue)
			return ct->cls_ori_queue;
		break;
	case IP_CT_DIR_REPLY:
		if (ct->cls_rep_set_queue)
			return ct->cls_rep_queue;
		break;
	}

	return -1;
}

static inline int get_classic_qos_dscp(struct nf_conn *ct, int dir)
{
	switch (dir) {
	case IP_CT_DIR_ORIGINAL:
		if (ct->cls_ori_set_dscp)
			return ct->cls_ori_dscp;
		break;
	case IP_CT_DIR_REPLY:
		if (ct->cls_rep_set_dscp)
			return ct->cls_rep_dscp;
		break;
	}

	return -1;
}

static int get_vip_qos_dscp(struct nf_conn *ct, int dir)
{
	struct flow_qos_policy *policy;
	int app_id = ct->cls_app_id;

	if (!ct->cls_dpi_done || !ct->cls_is_vip)
		return -1;

	if (app_id > MAX_APP_ID || !app_qos_policy[app_id].valid)
		return -1;

	if (dir == ct->cls_is_reverse)
		policy = &app_qos_policy[ct->cls_app_id].policy[FLOW_DIR_US];
	else
		policy = &app_qos_policy[ct->cls_app_id].policy[FLOW_DIR_DS];

	return policy->dscp;
}

static int get_cls_qos_dscp(struct nf_conn *ct, int dir)
{
	int dscp = -1;

	if (vip_qos_policy_first) {
		dscp = get_vip_qos_dscp(ct, dir);
		if (dscp >= 0)
			return dscp;
		dscp = get_classic_qos_dscp(ct, dir);
	} else {
		dscp = get_classic_qos_dscp(ct, dir);
		if (dscp >= 0)
			return dscp;
		dscp = get_vip_qos_dscp(ct, dir);
	}

	return dscp;
}

static void __flow_set_dscp_ipv4(struct nf_conn *ct, int dir, struct iphdr *iph)
{
	int dscp;
	u8 dsfield;

	dscp = get_cls_qos_dscp(ct, dir);
	if (dscp < 0)
		return;

	dsfield = dscp << 2;

	if ((iph->tos & ~INET_ECN_MASK) != dsfield)
		ipv4_change_dsfield(iph, INET_ECN_MASK, dsfield);
}

static void __flow_set_dscp_ipv6(struct nf_conn *ct, int dir, struct ipv6hdr *ip6h)
{
	int dscp;
	u8 dsfield;

	dscp = get_cls_qos_dscp(ct, dir);
	if (dscp < 0)
		return;

	dsfield = dscp << 2;

	if ((ipv6_get_dsfield(ip6h) & ~INET_ECN_MASK) != dsfield)
		ipv6_change_dsfield(ip6h, INET_ECN_MASK, dsfield);
}

static void flow_set_dscp_ipv4(struct nf_conn *ct, int dir, struct sk_buff *skb)
{
	struct iphdr *iph;
	u32 thoff;

	iph = ip_hdr(skb);
	thoff = iph->ihl * 4;

	if (ip_is_fragment(iph) || thoff != sizeof(struct iphdr))
		return;

	if (skb_ensure_writable(skb, skb_transport_offset(skb)))
		return;

	__flow_set_dscp_ipv4(ct, dir, iph);
}

static void flow_set_dscp_ipv6(struct nf_conn *ct, int dir, struct sk_buff *skb)
{
	struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);

	if (skb_ensure_writable(skb, skb_transport_offset(skb)))
		return;

	__flow_set_dscp_ipv6(ct, dir, ip6h);
}

static void __flow_offload_cls_meta(struct nf_conn *ct, int dir, struct flow_action_entry *entry)
{
	int dscp = -1, queue = -1;

	entry->id = FLOW_ACTION_CLS_META;
	memset(&entry->cls_meta, 0, sizeof(entry->cls_meta));
	if (vip_qos_policy_first) {
		dscp = get_vip_qos_dscp(ct, dir);
		if (dscp >= 0) {
			entry->cls_meta.set_dscp = 1;
			entry->cls_meta.dscp = dscp;
			entry->cls_meta.opt_queue = 2;
			return;
		}

		dscp = get_classic_qos_dscp(ct, dir);
		if (dscp >= 0) {
			entry->cls_meta.set_dscp = 1;
			entry->cls_meta.dscp = dscp;
		}

		queue = get_classic_qos_queue(ct, dir);
		if (queue >= 0) {
			entry->cls_meta.opt_queue = 1;
			entry->cls_meta.queue = queue;
		}
	} else {
		dscp = get_classic_qos_dscp(ct, dir);
		if (dscp >= 0) {
			entry->cls_meta.set_dscp = 1;
			entry->cls_meta.dscp = dscp;
		}

		queue = get_classic_qos_queue(ct, dir);
		if (queue >= 0) {
			entry->cls_meta.opt_queue = 1;
			entry->cls_meta.queue = queue;
		}

		dscp = get_vip_qos_dscp(ct, dir);
		if (dscp >= 0) {
			if (!entry->cls_meta.set_dscp) {
				entry->cls_meta.set_dscp = 1;
				entry->cls_meta.dscp = dscp;
			}
			if (!entry->cls_meta.opt_queue)
				entry->cls_meta.opt_queue = 2;
		}
	}
}

static unsigned int cls_qos_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct = NULL;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL || !nf_ct_is_confirmed(ct))
		return NF_ACCEPT;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		flow_set_dscp_ipv4(ct, CTINFO2DIR(ctinfo), skb);
		break;
	case htons(ETH_P_IPV6):
		flow_set_dscp_ipv6(ct, CTINFO2DIR(ctinfo), skb);
		break;
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops cls_qos_hooks[] __read_mostly = {
	{
		.hook		= cls_qos_hook,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FILTER + 3,
	},
	{
		.hook		= cls_qos_hook,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FILTER + 3,
	},
};

static void parse_app_qos_policy(char *policy_str)
{
	int app_id, us_dscp, ds_dscp;

	if (sscanf(policy_str, "%d %*s %d %d", &app_id, &us_dscp, &ds_dscp) != 3)
		return;

	if (app_id > MAX_APP_ID)
		return;

	app_qos_policy[app_id].valid = 1;
	app_qos_policy[app_id].policy[FLOW_DIR_US].dscp = us_dscp;
	app_qos_policy[app_id].policy[FLOW_DIR_DS].dscp = ds_dscp;
}

static int load_app_policy(void)
{
	void *data = NULL;
	int rc, len;
	char *p, *begin;

	memset(app_qos_policy, 0, sizeof(app_qos_policy));
	rc = kernel_read_file_from_path(APP_QOS_POLICY_PATH, 0, &data, INT_MAX, NULL,
					READING_FIRMWARE);
	if (rc < 0) {
		pr_err("Unable to open file: %s (%d)", APP_QOS_POLICY_PATH, rc);
		return rc;
	}

	p = begin = data;
	for (len = rc; len > 0; --len, ++p) {
		if (*p == '\n' || len == 1) {
			if (*begin == '#' ||
			    p - begin < MIN_POLICY_LINE_LEN ||
			    p - begin > MAX_POLICY_LINE_LEN) {
				begin = p + 1;
				continue;
			}
			if (*p == '\n')
				*p = 0;
			parse_app_qos_policy(begin);
			begin = p + 1;
		}
	}
	vfree(data);

	return 0;
}

static int __init cls_qos_init(void)
{
	int err;

	err = load_app_policy();
	if (err)
		return err;

	err =  nf_register_net_hooks(&init_net, cls_qos_hooks, ARRAY_SIZE(cls_qos_hooks));
	if (err)
		return err;

	cls_qos_table_hdr = register_sysctl_table(cls_qos_root_table);

	register_flow_set_dscp_ipv4(__flow_set_dscp_ipv4);
	register_flow_set_dscp_ipv6(__flow_set_dscp_ipv6);
	register_flow_offload_cls_meta(__flow_offload_cls_meta);

	return err;
}

static void __exit cls_qos_fini(void)
{
	unregister_sysctl_table(cls_qos_table_hdr);
	nf_unregister_net_hooks(&init_net, cls_qos_hooks, ARRAY_SIZE(cls_qos_hooks));
	register_flow_set_dscp_ipv4(NULL);
	register_flow_set_dscp_ipv6(NULL);
	register_flow_offload_cls_meta(NULL);
}

module_init(cls_qos_init);
module_exit(cls_qos_fini);

MODULE_DESCRIPTION("Clourneysemi QoS");
MODULE_LICENSE("GPL");
