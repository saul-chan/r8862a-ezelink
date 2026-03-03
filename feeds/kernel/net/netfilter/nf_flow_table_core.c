// SPDX-License-Identifier: GPL-2.0-only
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/rhashtable.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <net/ip.h>
#include <net/ip6_route.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_flow_table.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include "../bridge/br_private.h"

static DEFINE_MUTEX(flowtable_lock);
static LIST_HEAD(flowtables);
static u32 ft_hw_del_cnt;

u32 ft_hw_offload_cap;
u32 ft_hw_offload_max;
u32 ft_hw_idle_timeout	= HZ * 10;

#ifdef CONFIG_SYSCTL
static struct ctl_table_header *ft_sysctl_header;

static int proc_do_flowtable_hw_offload_max(struct ctl_table *ctl, int write,
					    void *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table lctl = *ctl;

	lctl.extra1 = SYSCTL_ONE;
	lctl.extra2 = &ft_hw_offload_cap;

	return proc_douintvec_minmax(&lctl, write, buffer, lenp, ppos);
}

enum flowtable_sysctl_index {
	NF_SYSCTL_FT_HW_OFFLOAD_MAX,
	NF_SYSCTL_FT_HW_IDLE_TIMEOUT,
};

static struct ctl_table flowtable_sysctl_table[] = {
	[NF_SYSCTL_FT_HW_OFFLOAD_MAX] = {
		.procname	= "hw_offload_max",
		.maxlen		= sizeof(u32),
		.mode		= 0644,
		.proc_handler	= proc_do_flowtable_hw_offload_max,
	},
	[NF_SYSCTL_FT_HW_IDLE_TIMEOUT] = {
		.procname	= "hw_idle_timeout",
		.maxlen		= sizeof(u32),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},

	{ }
};

static int nf_flowtable_init_sysctl(void)
{
	struct ctl_table *table;

	table = kmemdup(flowtable_sysctl_table, sizeof(flowtable_sysctl_table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	table[NF_SYSCTL_FT_HW_OFFLOAD_MAX].data		= &ft_hw_offload_max;
	table[NF_SYSCTL_FT_HW_IDLE_TIMEOUT].data	= &ft_hw_idle_timeout;

	ft_sysctl_header = register_net_sysctl(&init_net, "net/flowtable", table);
	if (!ft_sysctl_header) {
		kfree(table);
		return -ENOMEM;
	}

	return 0;
}

static void nf_flowtable_fini_sysctl(void)
{
	struct ctl_table *table;

	if (!ft_sysctl_header)
		return;

	table = ft_sysctl_header->ctl_table_arg;
	unregister_net_sysctl_table(ft_sysctl_header);
	kfree(table);
}
#else
static int nf_flowtable_init_sysctl(void)
{
	return 0;
}

static void nf_flowtable_fini_sysctl(void)
{
}
#endif

#ifdef CONFIG_PROC_FS
struct flowtable_seq_iter {
	struct seq_net_private p;
	struct rhashtable_iter hti;
};

static void *__flowtable_seq_next(struct seq_file *seq)
{
	struct flowtable_seq_iter *iter = seq->private;
	struct flow_offload_tuple_rhash *tuplehash;

	while ((tuplehash = rhashtable_walk_next(&iter->hti))) {
		if (IS_ERR(tuplehash)) {
			if (PTR_ERR(tuplehash) != -EAGAIN)
				break;
			continue;
		}
		if (tuplehash->tuple.dir)
			continue;
		if (tuplehash)
			break;
	}

	return tuplehash;
}

static void *flowtable_seq_start(struct seq_file *seq, loff_t *posp)
{
	struct flowtable_seq_iter *iter = seq->private;
	struct nf_flowtable *flowtable = PDE_DATA(file_inode(seq->file));
	void *obj = SEQ_START_TOKEN;
	loff_t pos;

	rhashtable_walk_enter(&flowtable->rhashtable, &iter->hti);
	rhashtable_walk_start(&iter->hti);

	for (pos = *posp; pos && obj && !IS_ERR(obj); pos--)
		obj = __flowtable_seq_next(seq);

	return obj;
}

static void *flowtable_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;
	return __flowtable_seq_next(seq);
}

static void flowtable_seq_stop(struct seq_file *seq, void *v)
{
	struct flowtable_seq_iter *iter = seq->private;

	rhashtable_walk_stop(&iter->hti);
	rhashtable_walk_exit(&iter->hti);
}

/* l4proto_name() is copied from net/netfilter/nf_conntrack_standalone.c */
static const char *l4proto_name(u8 proto)
{
	switch (proto) {
	case IPPROTO_ICMP: return "icmp";
	case IPPROTO_TCP: return "tcp";
	case IPPROTO_UDP: return "udp";
	case IPPROTO_DCCP: return "dccp";
	case IPPROTO_GRE: return "gre";
	case IPPROTO_SCTP: return "sctp";
	case IPPROTO_UDPLITE: return "udplite";
	case IPPROTO_ICMPV6: return "icmpv6";
	}

	return "unknown";
}

static int flowtable_seq_show(struct seq_file *seq, void *v)
{
	struct nf_flowtable *flowtable = PDE_DATA(file_inode(seq->file));
	struct flow_offload_tuple_rhash *tuplehash = v;
	struct flow_offload *flow;
	struct flow_offload_tuple *tuple;

	if (v == SEQ_START_TOKEN) {
		seq_printf(seq, "hw_offload_cap=%u hw_offload_max=%u hw_offload_cnt=%d recall_cnt=%d/%d\n",
			   ft_hw_offload_cap,
			   ft_hw_offload_max,
			   atomic_read(&flowtable->hw_offload_cnt),
			   atomic_read(&flowtable->tbd_recall_cnt),
			   atomic_read(&flowtable->gc_recall_cnt));
		return 0;
	}

	flow = container_of(tuplehash, struct flow_offload, tuplehash[0]);
	tuple = &tuplehash[IP_CT_DIR_ORIGINAL].tuple;

	if (tuple->l3proto == NFPROTO_IPV4)
		seq_printf(seq, "ipv4 %s %-15pI4:%-5hu <-> %-15pI4:%-5hu ",
			   l4proto_name(tuple->l4proto),
			   &tuple->src_v4.s_addr, ntohs(tuple->src_port),
			   &tuple->dst_v4.s_addr, ntohs(tuple->dst_port));
	else if (tuple->l3proto == NFPROTO_IPV6)
		seq_printf(seq, "ipv6 %s %pI6:%-5hu <-> %pI6:%-5hu ",
			   l4proto_name(tuple->l4proto),
			   &tuple->src_v6, ntohs(tuple->src_port),
			   &tuple->dst_v6, ntohs(tuple->dst_port));

	seq_printf(seq, "flags=0x%03lx ", flow->flags);

	seq_printf(seq, "sw_lifetime=%-2d ", nf_flow_timeout_delta(READ_ONCE(flow->timeout)) / HZ);

	if (test_bit(IPS_HW_OFFLOAD_BIT, &flow->ct->status)) {
		if (hw_flow_table_is_full(flowtable))
			seq_printf(seq, "hw_lifetime=%-3d ",
				   nf_flow_timeout_delta(READ_ONCE(flow->hw_lastused) + ft_hw_idle_timeout) / HZ);
		else
			seq_printf(seq, "hw_lifetime=%-3d ", nf_flow_timeout_delta(READ_ONCE(flow->timeout)) / HZ);
	} else
		seq_puts(seq, "hw_lifetime=NA  ");

	if (test_bit(IPS_HW_OFFLOAD_BIT, &flow->ct->status))
		seq_puts(seq, "HW_OFFLOAD ");
	else
		seq_puts(seq, "           ");

	if (flow->ct->cls_is_vip)
		seq_puts(seq, "VIP ");

	seq_puts(seq, "\n");

	return 0;
}

static const struct seq_operations flowtable_seq_ops = {
	.start = flowtable_seq_start,
	.next  = flowtable_seq_next,
	.stop  = flowtable_seq_stop,
	.show  = flowtable_seq_show,
};

static int nf_flowtable_init_proc(struct nf_flowtable *flowtable)
{
	struct net *net = read_pnet(&flowtable->net);
	struct nft_flowtable *nft_flowtable = container_of(flowtable, struct nft_flowtable, data);
	char path[256];

	if (flowtable->proc_ent)
		return 0;

	snprintf(path, sizeof(path), "flowtable_%s_%s", nft_flowtable->table->name, nft_flowtable->name);
	flowtable->proc_ent = proc_create_net_data(path, 0444, net->proc_net, &flowtable_seq_ops,
						   sizeof(struct flowtable_seq_iter), flowtable);
	if (!flowtable->proc_ent)
		return -ENOMEM;

	return 0;
}

static void nf_flowtable_fini_proc(struct nf_flowtable *flowtable)
{
	if (!flowtable->proc_ent)
		return;

	proc_remove(flowtable->proc_ent);
}
#else
static int nf_flowtable_init_proc(struct nf_flowtable *flowtable)
{
	return 0;
}

static void nf_flowtable_fini_proc(struct nf_flowtable *flowtable)
{
}
#endif

static void
flow_offload_fill_dir(struct flow_offload *flow,
		      enum flow_offload_tuple_dir dir)
{
	struct flow_offload_tuple *ft = &flow->tuplehash[dir].tuple;
	struct nf_conntrack_tuple *ctt = &flow->ct->tuplehash[dir].tuple;

	ft->dir = dir;

	switch (ctt->src.l3num) {
	case NFPROTO_IPV4:
		ft->src_v4 = ctt->src.u3.in;
		ft->dst_v4 = ctt->dst.u3.in;
		break;
	case NFPROTO_IPV6:
		ft->src_v6 = ctt->src.u3.in6;
		ft->dst_v6 = ctt->dst.u3.in6;
		break;
	}

	ft->l3proto = ctt->src.l3num;
	ft->l4proto = ctt->dst.protonum;
	ft->src_port = ctt->src.u.tcp.port;
	ft->dst_port = ctt->dst.u.tcp.port;
}

struct flow_offload *flow_offload_alloc(struct nf_conn *ct)
{
	struct flow_offload *flow;

	if (unlikely(nf_ct_is_dying(ct) ||
	    !atomic_inc_not_zero(&ct->ct_general.use)))
		return NULL;

	flow = kzalloc(sizeof(*flow), GFP_ATOMIC);
	if (!flow)
		goto err_ct_refcnt;

	flow->ct = ct;

	flow_offload_fill_dir(flow, FLOW_OFFLOAD_DIR_ORIGINAL);
	flow_offload_fill_dir(flow, FLOW_OFFLOAD_DIR_REPLY);

	if (ct->status & IPS_SRC_NAT)
		__set_bit(NF_FLOW_SNAT, &flow->flags);
	if (ct->status & IPS_DST_NAT)
		__set_bit(NF_FLOW_DNAT, &flow->flags);

	return flow;

err_ct_refcnt:
	nf_ct_put(ct);

	return NULL;
}
EXPORT_SYMBOL_GPL(flow_offload_alloc);

static u32 flow_offload_dst_cookie(struct flow_offload_tuple *flow_tuple)
{
	const struct rt6_info *rt;

	if (flow_tuple->l3proto == NFPROTO_IPV6) {
		rt = (const struct rt6_info *)flow_tuple->dst_cache;
		return rt6_get_cookie(rt);
	}

	return 0;
}

static int flow_offload_fill_route(struct flow_offload *flow,
				   const struct nf_flow_route *route,
				   enum flow_offload_tuple_dir dir)
{
	struct flow_offload_tuple *flow_tuple = &flow->tuplehash[dir].tuple;
	struct dst_entry *dst = route->tuple[dir].dst;
	int i, j = 0;

	switch (flow_tuple->l3proto) {
	case NFPROTO_IPV4:
		flow_tuple->mtu = ip_dst_mtu_maybe_forward(dst, true);
		break;
	case NFPROTO_IPV6:
		flow_tuple->mtu = ip6_dst_mtu_forward(dst);
		break;
	}

	flow_tuple->iifidx = route->tuple[dir].in.ifindex;
	for (i = route->tuple[dir].in.num_encaps - 1; i >= 0; i--) {
		flow_tuple->encap[j].id = route->tuple[dir].in.encap[i].id;
		flow_tuple->encap[j].proto = route->tuple[dir].in.encap[i].proto;
		if (route->tuple[dir].in.ingress_vlans & BIT(i))
			flow_tuple->in_vlan_ingress |= BIT(j);
		j++;
	}
	flow_tuple->encap_num = route->tuple[dir].in.num_encaps;

	switch (route->tuple[dir].xmit_type) {
	case FLOW_OFFLOAD_XMIT_DIRECT:
		memcpy(flow_tuple->out.h_dest, route->tuple[dir].out.h_dest,
		       ETH_ALEN);
		memcpy(flow_tuple->out.h_source, route->tuple[dir].out.h_source,
		       ETH_ALEN);
		flow_tuple->out.ifidx = route->tuple[dir].out.ifindex;
		flow_tuple->out.hw_ifidx = route->tuple[dir].out.hw_ifindex;
		break;
	case FLOW_OFFLOAD_XMIT_XFRM:
	case FLOW_OFFLOAD_XMIT_NEIGH:
		if (!dst_hold_safe(route->tuple[dir].dst))
			return -1;

		flow_tuple->dst_cache = dst;
		flow_tuple->dst_cookie = flow_offload_dst_cookie(flow_tuple);
		break;
	default:
		WARN_ON_ONCE(1);
		break;
	}
	flow_tuple->xmit_type = route->tuple[dir].xmit_type;

	return 0;
}

static void nft_flow_dst_release(struct flow_offload *flow,
				 enum flow_offload_tuple_dir dir)
{
	if (flow->tuplehash[dir].tuple.xmit_type == FLOW_OFFLOAD_XMIT_NEIGH ||
	    flow->tuplehash[dir].tuple.xmit_type == FLOW_OFFLOAD_XMIT_XFRM)
		dst_release(flow->tuplehash[dir].tuple.dst_cache);
}

int flow_offload_route_init(struct flow_offload *flow,
			    const struct nf_flow_route *route)
{
	int err;

	err = flow_offload_fill_route(flow, route, FLOW_OFFLOAD_DIR_ORIGINAL);
	if (err < 0)
		return err;

	err = flow_offload_fill_route(flow, route, FLOW_OFFLOAD_DIR_REPLY);
	if (err < 0)
		goto err_route_reply;

	flow->type = NF_FLOW_OFFLOAD_ROUTE;

	return 0;

err_route_reply:
	nft_flow_dst_release(flow, FLOW_OFFLOAD_DIR_ORIGINAL);

	return err;
}
EXPORT_SYMBOL_GPL(flow_offload_route_init);

static void flow_offload_fixup_tcp(struct ip_ct_tcp *tcp)
{
	tcp->seen[0].td_maxwin = 0;
	tcp->seen[1].td_maxwin = 0;
}

static void flow_offload_fixup_ct(struct nf_conn *ct)
{
	struct net *net = nf_ct_net(ct);
	int l4num = nf_ct_protonum(ct);
	s32 timeout;

	if (l4num == IPPROTO_TCP) {
		struct nf_tcp_net *tn = nf_tcp_pernet(net);

		flow_offload_fixup_tcp(&ct->proto.tcp);

		timeout = tn->timeouts[ct->proto.tcp.state];
		timeout -= tn->offload_timeout;
	} else if (l4num == IPPROTO_UDP) {
		struct nf_udp_net *tn = nf_udp_pernet(net);

		timeout = tn->timeouts[UDP_CT_REPLIED];
		timeout -= tn->offload_timeout;
	} else {
		return;
	}

	if (timeout < 0)
		timeout = 0;

	if (nf_flow_timeout_delta(READ_ONCE(ct->timeout)) > (__s32)timeout)
		WRITE_ONCE(ct->timeout, nfct_time_stamp + timeout);
}

static void flow_offload_route_release(struct flow_offload *flow)
{
	nft_flow_dst_release(flow, FLOW_OFFLOAD_DIR_ORIGINAL);
	nft_flow_dst_release(flow, FLOW_OFFLOAD_DIR_REPLY);
}

void flow_offload_free(struct flow_offload *flow)
{
	switch (flow->type) {
	case NF_FLOW_OFFLOAD_ROUTE:
		flow_offload_route_release(flow);
		break;
	default:
		break;
	}
	nf_ct_put(flow->ct);
	kfree_rcu(flow, rcu_head);
}
EXPORT_SYMBOL_GPL(flow_offload_free);

static u32 flow_offload_hash(const void *data, u32 len, u32 seed)
{
	const struct flow_offload_tuple *tuple = data;

	return jhash(tuple, offsetof(struct flow_offload_tuple, __hash), seed);
}

static u32 flow_offload_hash_obj(const void *data, u32 len, u32 seed)
{
	const struct flow_offload_tuple_rhash *tuplehash = data;

	return jhash(&tuplehash->tuple, offsetof(struct flow_offload_tuple, __hash), seed);
}

static int flow_offload_hash_cmp(struct rhashtable_compare_arg *arg,
					const void *ptr)
{
	const struct flow_offload_tuple *tuple = arg->key;
	const struct flow_offload_tuple_rhash *x = ptr;

	if (memcmp(&x->tuple, tuple, offsetof(struct flow_offload_tuple, __hash)))
		return 1;

	return 0;
}

static const struct rhashtable_params nf_flow_offload_rhash_params = {
	.head_offset		= offsetof(struct flow_offload_tuple_rhash, node),
	.hashfn			= flow_offload_hash,
	.obj_hashfn		= flow_offload_hash_obj,
	.obj_cmpfn		= flow_offload_hash_cmp,
	.automatic_shrinking	= true,
};

unsigned long flow_offload_get_timeout(struct flow_offload *flow)
{
	unsigned long timeout = NF_FLOW_TIMEOUT;
	struct net *net = nf_ct_net(flow->ct);
	int l4num = nf_ct_protonum(flow->ct);

	if (l4num == IPPROTO_TCP) {
		struct nf_tcp_net *tn = nf_tcp_pernet(net);

		timeout = tn->offload_timeout;
	} else if (l4num == IPPROTO_UDP) {
		struct nf_udp_net *tn = nf_udp_pernet(net);

		timeout = tn->offload_timeout;
	}

	return timeout;
}
EXPORT_SYMBOL_GPL(flow_offload_get_timeout);

static bool __netif_is_npe_eth_port(struct net *net, int ifindex)
{
	struct net_device *dev;
	bool ret;

	dev = dev_get_by_index(net, ifindex);
	if (!dev)
		return false;

	ret = if_port_is_npe_eth_port(dev->if_port);
	dev_put(dev);

	return ret;
}

static inline bool flow_can_hw_offload(struct nf_flowtable *flowtable, struct flow_offload *flow)
{
	struct net *net = read_pnet(&flowtable->net);

	if (clsemi_fast_fwd_enabled())
		return true;

	if (!__netif_is_npe_eth_port(net, flow->tuplehash[FLOW_OFFLOAD_DIR_ORIGINAL].tuple.iifidx) ||
	    !__netif_is_npe_eth_port(net, flow->tuplehash[FLOW_OFFLOAD_DIR_REPLY].tuple.iifidx))
		return false;

	return true;
}

int flow_offload_add(struct nf_flowtable *flow_table, struct flow_offload *flow)
{
	int err;

	WRITE_ONCE(flow->timeout, nf_flowtable_time_stamp + flow_offload_get_timeout(flow));

	err = rhashtable_insert_fast(&flow_table->rhashtable,
				     &flow->tuplehash[0].node,
				     nf_flow_offload_rhash_params);
	if (err < 0)
		return err;

	err = rhashtable_insert_fast(&flow_table->rhashtable,
				     &flow->tuplehash[1].node,
				     nf_flow_offload_rhash_params);
	if (err < 0) {
		rhashtable_remove_fast(&flow_table->rhashtable,
				       &flow->tuplehash[0].node,
				       nf_flow_offload_rhash_params);
		return err;
	}

	nf_ct_offload_timeout(flow->ct);

	if (nf_flowtable_hw_offload(flow_table) && flow_can_hw_offload(flow_table, flow)) {
		__set_bit(NF_FLOW_HW, &flow->flags);
		nf_flow_offload_add(flow_table, flow);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(flow_offload_add);

void flow_offload_refresh(struct nf_flowtable *flow_table,
			  struct flow_offload *flow)
{
	u32 timeout;

	timeout = nf_flowtable_time_stamp + flow_offload_get_timeout(flow);
	if (time_after32(timeout, READ_ONCE(flow->timeout) + HZ))
		WRITE_ONCE(flow->timeout, timeout);
	else
		return;

	if (likely(!nf_flowtable_hw_offload(flow_table) || !flow_can_hw_offload(flow_table, flow)))
		return;

	nf_flow_offload_add(flow_table, flow);
}
EXPORT_SYMBOL_GPL(flow_offload_refresh);

static inline bool nf_flow_has_expired(const struct flow_offload *flow)
{
	return nf_flow_timeout_delta(READ_ONCE(flow->timeout)) <= 0;
}

static void flow_offload_del(struct nf_flowtable *flow_table,
			     struct flow_offload *flow)
{
	rhashtable_remove_fast(&flow_table->rhashtable,
			       &flow->tuplehash[FLOW_OFFLOAD_DIR_ORIGINAL].node,
			       nf_flow_offload_rhash_params);
	rhashtable_remove_fast(&flow_table->rhashtable,
			       &flow->tuplehash[FLOW_OFFLOAD_DIR_REPLY].node,
			       nf_flow_offload_rhash_params);
	flow_offload_free(flow);
}

void flow_offload_teardown(struct flow_offload *flow)
{
	clear_bit(IPS_OFFLOAD_BIT, &flow->ct->status);
	set_bit(NF_FLOW_TEARDOWN, &flow->flags);
	flow_offload_fixup_ct(flow->ct);
}
EXPORT_SYMBOL_GPL(flow_offload_teardown);

struct flow_offload_tuple_rhash *
flow_offload_lookup(struct nf_flowtable *flow_table,
		    struct flow_offload_tuple *tuple)
{
	struct flow_offload_tuple_rhash *tuplehash;
	struct flow_offload *flow;
	int dir;

	tuplehash = rhashtable_lookup(&flow_table->rhashtable, tuple,
				      nf_flow_offload_rhash_params);
	if (!tuplehash)
		return NULL;

	dir = tuplehash->tuple.dir;
	flow = container_of(tuplehash, struct flow_offload, tuplehash[dir]);
	if (test_bit(NF_FLOW_TEARDOWN, &flow->flags))
		return NULL;

	if (unlikely(nf_ct_is_dying(flow->ct)))
		return NULL;

	return tuplehash;
}
EXPORT_SYMBOL_GPL(flow_offload_lookup);

int nf_flow_table_iterate(struct nf_flowtable *flow_table,
		      void (*iter)(struct flow_offload *flow, void *data),
		      void *data)
{
	struct flow_offload_tuple_rhash *tuplehash;
	struct rhashtable_iter hti;
	struct flow_offload *flow;
	int err = 0;

	rhashtable_walk_enter(&flow_table->rhashtable, &hti);
	rhashtable_walk_start(&hti);

	while ((tuplehash = rhashtable_walk_next(&hti))) {
		if (IS_ERR(tuplehash)) {
			if (PTR_ERR(tuplehash) != -EAGAIN) {
				err = PTR_ERR(tuplehash);
				break;
			}
			continue;
		}
		if (tuplehash->tuple.dir)
			continue;

		flow = container_of(tuplehash, struct flow_offload, tuplehash[0]);

		iter(flow, data);
	}
	rhashtable_walk_stop(&hti);
	rhashtable_walk_exit(&hti);

	return err;
}
EXPORT_SYMBOL_GPL(nf_flow_table_iterate);

static inline bool flow_should_recall_from_hw(struct nf_flowtable *flowtable, struct flow_offload *flow)
{
	if (!test_bit(IPS_HW_OFFLOAD_BIT, &flow->ct->status) ||
	    test_bit(NF_FLOW_HW_DYING, &flow->flags) ||
	    test_bit(NF_FLOW_HW_PENDING, &flow->flags))
		return false;

	if (flow->ct->cls_need_recall) {
		flow->ct->cls_need_recall = 0;
		return true;
	}

	if (flowtable->full &&
	    !flow->ct->cls_is_vip &&
	    nf_flow_timeout_delta(READ_ONCE(flow->hw_lastused) + ft_hw_idle_timeout) <= 0)
		return true;

	return false;
}

static void nf_flow_offload_cleanup(struct nf_flowtable *flowtable)
{
	struct flow_cls_offload cls_flow = {};
	struct list_head *block_cb_list = &flowtable->flow_block.cb_list;
	struct flow_block_cb *block_cb;

	cls_flow.command = FLOW_CLS_CLEANUP;

	down_read(&flowtable->flow_block_lock);
	list_for_each_entry(block_cb, block_cb_list, list) {
		block_cb->cb(TC_SETUP_CLSFLOWER, &cls_flow,
			     block_cb->cb_priv);
	}
	up_read(&flowtable->flow_block_lock);
}

static void nf_flow_offload_gc_step(struct flow_offload *flow, void *data)
{
	struct nf_flowtable *flow_table = data;

	if (nf_flow_has_expired(flow) ||
	    nf_ct_is_dying(flow->ct))
		flow_offload_teardown(flow);

	if (test_bit(NF_FLOW_TEARDOWN, &flow->flags)) {
		if (test_bit(NF_FLOW_HW, &flow->flags)) {
			if (!test_bit(NF_FLOW_HW_DYING, &flow->flags)) {
				nf_flow_offload_del(flow_table, flow);
				ft_hw_del_cnt++;
			} else if (test_bit(NF_FLOW_HW_DEAD, &flow->flags)) {
				flow_offload_del(flow_table, flow);
			}
		} else {
			flow_offload_del(flow_table, flow);
		}
	} else if (test_bit(NF_FLOW_HW, &flow->flags)) {
		if (flow_should_recall_from_hw(flow_table, flow)) {
			nf_flow_offload_recall(flow_table, flow);
			ft_hw_del_cnt++;
		}
	}
}

static void nf_flow_offload_work_gc(struct work_struct *work)
{
	struct nf_flowtable *flow_table;

	flow_table = container_of(work, struct nf_flowtable, gc_work.work);
	nf_flowtable_init_proc(flow_table);
	flow_table->full = hw_flow_table_is_full(flow_table);
	ft_hw_del_cnt = 0;
	nf_flow_table_iterate(flow_table, nf_flow_offload_gc_step, flow_table);
	if (ft_hw_del_cnt > 1)
		nf_flow_offload_cleanup(flow_table);
	queue_delayed_work_on(0, system_power_efficient_wq, &flow_table->gc_work, HZ);
}

static void nf_flow_nat_port_tcp(struct sk_buff *skb, unsigned int thoff,
				 __be16 port, __be16 new_port)
{
	struct tcphdr *tcph;

	tcph = (void *)(skb_network_header(skb) + thoff);
	inet_proto_csum_replace2(&tcph->check, skb, port, new_port, false);
}

static void nf_flow_nat_port_udp(struct sk_buff *skb, unsigned int thoff,
				 __be16 port, __be16 new_port)
{
	struct udphdr *udph;

	udph = (void *)(skb_network_header(skb) + thoff);
	if (udph->check || skb->ip_summed == CHECKSUM_PARTIAL) {
		inet_proto_csum_replace2(&udph->check, skb, port,
					 new_port, false);
		if (!udph->check)
			udph->check = CSUM_MANGLED_0;
	}
}

static void nf_flow_nat_port(struct sk_buff *skb, unsigned int thoff,
			     u8 protocol, __be16 port, __be16 new_port)
{
	switch (protocol) {
	case IPPROTO_TCP:
		nf_flow_nat_port_tcp(skb, thoff, port, new_port);
		break;
	case IPPROTO_UDP:
		nf_flow_nat_port_udp(skb, thoff, port, new_port);
		break;
	}
}

void nf_flow_snat_port(const struct flow_offload *flow,
		       struct sk_buff *skb, unsigned int thoff,
		       u8 protocol, enum flow_offload_tuple_dir dir)
{
	struct flow_ports *hdr;
	__be16 port, new_port;

	hdr = (void *)(skb_network_header(skb) + thoff);

	switch (dir) {
	case FLOW_OFFLOAD_DIR_ORIGINAL:
		port = hdr->source;
		new_port = flow->tuplehash[FLOW_OFFLOAD_DIR_REPLY].tuple.dst_port;
		hdr->source = new_port;
		break;
	case FLOW_OFFLOAD_DIR_REPLY:
		port = hdr->dest;
		new_port = flow->tuplehash[FLOW_OFFLOAD_DIR_ORIGINAL].tuple.src_port;
		hdr->dest = new_port;
		break;
	}

	nf_flow_nat_port(skb, thoff, protocol, port, new_port);
}
EXPORT_SYMBOL_GPL(nf_flow_snat_port);

void nf_flow_dnat_port(const struct flow_offload *flow, struct sk_buff *skb,
		       unsigned int thoff, u8 protocol,
		       enum flow_offload_tuple_dir dir)
{
	struct flow_ports *hdr;
	__be16 port, new_port;

	hdr = (void *)(skb_network_header(skb) + thoff);

	switch (dir) {
	case FLOW_OFFLOAD_DIR_ORIGINAL:
		port = hdr->dest;
		new_port = flow->tuplehash[FLOW_OFFLOAD_DIR_REPLY].tuple.src_port;
		hdr->dest = new_port;
		break;
	case FLOW_OFFLOAD_DIR_REPLY:
		port = hdr->source;
		new_port = flow->tuplehash[FLOW_OFFLOAD_DIR_ORIGINAL].tuple.dst_port;
		hdr->source = new_port;
		break;
	}

	nf_flow_nat_port(skb, thoff, protocol, port, new_port);
}
EXPORT_SYMBOL_GPL(nf_flow_dnat_port);

int nf_flow_table_init(struct nf_flowtable *flowtable)
{
	int err;

	INIT_DELAYED_WORK(&flowtable->gc_work, nf_flow_offload_work_gc);
	flow_block_init(&flowtable->flow_block);
	init_rwsem(&flowtable->flow_block_lock);

	err = rhashtable_init(&flowtable->rhashtable,
			      &nf_flow_offload_rhash_params);
	if (err < 0)
		return err;

	queue_delayed_work_on(0, system_power_efficient_wq, &flowtable->gc_work, HZ);

	mutex_lock(&flowtable_lock);
	list_add(&flowtable->list, &flowtables);
	mutex_unlock(&flowtable_lock);

	return 0;
}
EXPORT_SYMBOL_GPL(nf_flow_table_init);

static void nf_flow_table_do_cleanup(struct flow_offload *flow, void *data)
{
	struct net_device *dev = data;

	if (!dev) {
		flow_offload_teardown(flow);
		return;
	}

	if (net_eq(nf_ct_net(flow->ct), dev_net(dev)) &&
	    (flow->tuplehash[0].tuple.iifidx == dev->ifindex ||
	     flow->tuplehash[1].tuple.iifidx == dev->ifindex))
		flow_offload_teardown(flow);
}

void nf_flow_table_gc_cleanup(struct nf_flowtable *flowtable,
			      struct net_device *dev)
{
	nf_flow_table_iterate(flowtable, nf_flow_table_do_cleanup, dev);
	flush_delayed_work(&flowtable->gc_work);
	nf_flow_table_offload_flush(flowtable);
}

void nf_flow_table_cleanup(struct net_device *dev)
{
	struct nf_flowtable *flowtable;

	mutex_lock(&flowtable_lock);
	list_for_each_entry(flowtable, &flowtables, list)
		nf_flow_table_gc_cleanup(flowtable, dev);
	mutex_unlock(&flowtable_lock);
}
EXPORT_SYMBOL_GPL(nf_flow_table_cleanup);

void nf_flow_table_free(struct nf_flowtable *flow_table)
{
	WRITE_ONCE(flow_table->dying, true);

	nf_flowtable_fini_proc(flow_table);
	mutex_lock(&flowtable_lock);
	list_del(&flow_table->list);
	mutex_unlock(&flowtable_lock);

	cancel_delayed_work_sync(&flow_table->gc_work);
	nf_flow_table_iterate(flow_table, nf_flow_table_do_cleanup, NULL);
	ft_hw_del_cnt = 0;
	nf_flow_table_iterate(flow_table, nf_flow_offload_gc_step, flow_table);
	if (ft_hw_del_cnt > 1)
		nf_flow_offload_cleanup(flow_table);
	nf_flow_table_offload_flush(flow_table);
	if (nf_flowtable_hw_offload(flow_table))
		nf_flow_table_iterate(flow_table, nf_flow_offload_gc_step,
				      flow_table);
	rhashtable_destroy(&flow_table->rhashtable);
}
EXPORT_SYMBOL_GPL(nf_flow_table_free);

static int nf_flow_table_netdev_event(struct notifier_block *this,
				      unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);

	if (event != NETDEV_DOWN)
		return NOTIFY_DONE;

	nf_flow_table_cleanup(dev);

	return NOTIFY_DONE;
}

static struct notifier_block flow_offload_netdev_notifier = {
	.notifier_call	= nf_flow_table_netdev_event,
};

static int __hotplug_dev_flowtable_hook(struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct nf_flowtable *flowtable;
	unsigned int verdict;
	int ret;

	/* refer from nf_hook_slow() */
	list_for_each_entry(flowtable, &flowtables, list) {
		verdict = flowtable->type->hook(flowtable, skb, state);
		switch (verdict & NF_VERDICT_MASK) {
		case NF_ACCEPT:
			break;
		case NF_DROP:
			kfree_skb(skb);
			ret = NF_DROP_GETERR(verdict);
			if (ret == 0)
				ret = -EPERM;
			return ret;
		case NF_QUEUE:
			WARN_ON_ONCE(1);
			return 0;
		default:
			return 0;
		}
	}
	return 1;
}

static int __init nf_flow_table_module_init(void)
{
	int ret;

	ret = nf_flow_table_offload_init();
	if (ret)
		return ret;

	ret = register_netdevice_notifier(&flow_offload_netdev_notifier);
	if (ret)
		nf_flow_table_offload_exit();

	nf_flowtable_init_sysctl();

	hotplug_dev_flowtable_hook = __hotplug_dev_flowtable_hook;

	return ret;
}

static void __exit nf_flow_table_module_exit(void)
{
	hotplug_dev_flowtable_hook = NULL;
	nf_flowtable_fini_sysctl();
	unregister_netdevice_notifier(&flow_offload_netdev_notifier);
	nf_flow_table_offload_exit();
}

module_init(nf_flow_table_module_init);
module_exit(nf_flow_table_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pablo Neira Ayuso <pablo@netfilter.org>");
MODULE_DESCRIPTION("Netfilter flow table module");
