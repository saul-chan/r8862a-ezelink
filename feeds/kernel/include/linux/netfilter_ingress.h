/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _NETFILTER_INGRESS_H_
#define _NETFILTER_INGRESS_H_

#include <linux/netfilter.h>
#include <linux/netdevice.h>

#ifdef CONFIG_NETFILTER_INGRESS
static inline bool nf_hook_ingress_active(const struct sk_buff *skb)
{
	if (skb->dev->hotplug_dev_flow_offload)
		return true;

#ifdef CONFIG_JUMP_LABEL
	if (!static_key_false(&nf_hooks_needed[NFPROTO_NETDEV][NF_NETDEV_INGRESS]))
		return false;
#endif
	return rcu_access_pointer(skb->dev->nf_hooks_ingress);
}

/* caller must hold rcu_read_lock */
static inline int nf_hook_ingress(struct sk_buff *skb)
{
	struct nf_hook_entries *e = rcu_dereference(skb->dev->nf_hooks_ingress);
	struct nf_hook_state state;
	int ret = 1;

	/* Must recheck the ingress hook head, in the event it became NULL
	 * after the check in nf_hook_ingress_active evaluated to true.
	 */
	if (unlikely(!e && !skb->dev->hotplug_dev_flow_offload))
		return 0;

	nf_hook_state_init(&state, NF_NETDEV_INGRESS,
			   NFPROTO_NETDEV, skb->dev, NULL, NULL,
			   dev_net(skb->dev), NULL);

	if (skb->dev->hotplug_dev_flow_offload && hotplug_dev_fast_fwd_hook)
		ret = hotplug_dev_fast_fwd_hook(skb, &state);

	if (ret == 1 && e)
		ret = nf_hook_slow(skb, &state, e, 0);

	if (ret == 1 && skb->dev->hotplug_dev_flow_offload && hotplug_dev_flowtable_hook)
		ret = hotplug_dev_flowtable_hook(skb, &state);

	if (ret == 0)
		return -1;

	return ret;
}

static inline void nf_hook_ingress_init(struct net_device *dev)
{
	RCU_INIT_POINTER(dev->nf_hooks_ingress, NULL);
}
#else /* CONFIG_NETFILTER_INGRESS */
static inline int nf_hook_ingress_active(struct sk_buff *skb)
{
	return 0;
}

static inline int nf_hook_ingress(struct sk_buff *skb)
{
	return 0;
}

static inline void nf_hook_ingress_init(struct net_device *dev) {}
#endif /* CONFIG_NETFILTER_INGRESS */
#endif /* _NETFILTER_INGRESS_H_ */
