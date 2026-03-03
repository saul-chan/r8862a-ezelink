/*
 * Copyright (c) 2021-2025, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <linux/kernel.h>
#include <linux/if_vlan.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_flow_table.h>
#include <net/ip.h>
#include "../bridge/br_private.h"

enum {
	FAST_FWD_AT_NULL = 0,
	FAST_FWD_AT_NF_NETDEV_INGRESS,
	FAST_FWD_AT_DRIVER_RX_PATH,
};

enum {
	MEASURE_PPS_AT_NULL = 0,
	MEASURE_PPS_BEFORE_FWD,
	MEASURE_PPS_BEFORE_TX,
};

enum {
	TX_BY_DEV_QUEUE_XMIT = 0,
	TX_BY_CLS_DIRECT_XMIT,
	TX_BY_CLS_FAST_XMIT,
	TX_BY_CLS_AFAP_XMIT,
};

static int g_fast_fwd_site = FAST_FWD_AT_DRIVER_RX_PATH;
static struct net_device *(*l3_find_out_dev)(struct sk_buff *skb);
static bool g_lan_mac_addr_specified;
static u8 g_lan_mac_addr[ETH_ALEN];
static u8 g_sw_fwd_vid[VLAN_N_VID];

bool is_dns_port(struct sk_buff *skb)
{
	__be16 proto = skb->protocol;
	const struct flow_ports *portsh;
	const struct vlan_ethhdr *veth;
	const struct iphdr *ip4h;
	const struct ipv6hdr *ip6h;
	u32 offset = 0, ip4h_len;

	if (proto == htons(ETH_P_8021Q)) {
		veth = (struct vlan_ethhdr *)skb_mac_header(skb);
		proto = veth->h_vlan_encapsulated_proto;
		offset = VLAN_HLEN;
	}

	if (proto == htons(ETH_P_IP)) {
		if (!pskb_may_pull(skb, skb_network_offset(skb) + offset + sizeof(*ip4h)))
			return false;

		ip4h = (struct iphdr *)(skb_network_header(skb) + offset);
		if (ip4h->protocol != IPPROTO_UDP && ip4h->protocol != IPPROTO_TCP)
			return false;

		ip4h_len = ip4h->ihl * 4;
		if (ip_is_fragment(ip4h) || unlikely(ip4h_len != sizeof(struct iphdr)))
			return false;

		if (!pskb_may_pull(skb, skb_network_offset(skb) + offset + ip4h_len + sizeof(*portsh)))
			return false;

		portsh = (struct flow_ports *)((uint8_t *)ip4h + ip4h_len);
		return portsh->dest == htons(53);
	} else if (proto == htons(ETH_P_IPV6)) {
		if (!pskb_may_pull(skb, skb_network_offset(skb) + offset + sizeof(*ip6h)))
			return false;

		ip6h = (struct ipv6hdr *)(skb_network_header(skb) + offset);
		if (ip6h->nexthdr != IPPROTO_UDP && ip6h->nexthdr == IPPROTO_TCP)
			return false;

		if (!pskb_may_pull(skb, skb_network_offset(skb) + offset + sizeof(*ip6h) + sizeof(*portsh)))
			return false;

		portsh = (struct flow_ports *)((uint8_t *)ip6h + sizeof(*ip6h));
		return portsh->dest == htons(53);
	}

	return false;
}

bool clsemi_fast_fwd_enabled(void)
{
	return g_fast_fwd_site != FAST_FWD_AT_NULL;
}
EXPORT_SYMBOL(clsemi_fast_fwd_enabled);

bool clsemi_vid_should_sw_fwd(u16 vid)
{
	return g_sw_fwd_vid[vid];
}
EXPORT_SYMBOL(clsemi_vid_should_sw_fwd);

static ssize_t sw_fwd_vid_show(struct class *cls,
			       struct class_attribute *attr,
			       char *buf)
{
	int i = 0, len = 0;

	for (; i < VLAN_N_VID; ++i)
		if (g_sw_fwd_vid[i])
			len = snprintf(buf + len, PAGE_SIZE - len, "%d\n", i);

	return len;
}

static ssize_t sw_fwd_vid_store(struct class *cls,
				   struct class_attribute *attr,
				   const char *buf, size_t len)
{
	int val, ret;

	ret = kstrtoint(buf, 0, &val);
	if (ret)
		return ret;

	if (val < 0 || val >= VLAN_N_VID)
		return -EINVAL;

	/* 0 means clean sw forward vid table */
	if (val == 0)
		memset(g_sw_fwd_vid, 0, sizeof(g_sw_fwd_vid));
	else
		g_sw_fwd_vid[val] = 1;

	return len;
}

static const struct class_attribute class_attr_sw_fwd_vid = {
	.attr = {
		.name = "sw_fwd_vid_list",
		.mode = 0644,
	},
	.show = sw_fwd_vid_show,
	.store = sw_fwd_vid_store,
};

static ssize_t lan_mac_addr_show(struct class *cls,
				 struct class_attribute *attr,
				 char *buf)
{
	return sysfs_format_mac(buf, g_lan_mac_addr, ETH_ALEN);
}

static ssize_t lan_mac_addr_store(struct class *cls,
				  struct class_attribute *attr,
				  const char *buf, size_t len)
{
	u8 mac_addr[ETH_ALEN];

	if (!mac_pton(buf, mac_addr))
		return -EINVAL;

	ether_addr_copy(g_lan_mac_addr, mac_addr);
	g_lan_mac_addr_specified = true;

	return len;
}

static const struct class_attribute class_attr_lan_mac_addr = {
	.attr = {
		.name = "lan_mac_addr",
		.mode = 0644,
	},
	.show = lan_mac_addr_show,
	.store = lan_mac_addr_store,
};

static ssize_t fast_fwd_site_show(struct class *cls,
				  struct class_attribute *attr,
				  char *buf)
{
	return sprintf(buf, "current fast_fwd_site: %d\n"
			    "available fast_fwd_site:\n"
			    "    %d: don't fast fwd at anywhere\n"
			    "    %d: fast fwd at NF_NETDEV_INGRESS\n"
			    "    %d: fast fwd at driver RX path\n",
			    g_fast_fwd_site,
			    FAST_FWD_AT_NULL,
			    FAST_FWD_AT_NF_NETDEV_INGRESS,
			    FAST_FWD_AT_DRIVER_RX_PATH);
}

static ssize_t fast_fwd_site_store(struct class *cls,
				   struct class_attribute *attr,
				   const char *buf, size_t len)
{
	int val, ret;

	ret = kstrtoint(buf, 0, &val);
	if (ret)
		return ret;

	if (val != FAST_FWD_AT_NULL &&
	    val != FAST_FWD_AT_NF_NETDEV_INGRESS &&
	    val != FAST_FWD_AT_DRIVER_RX_PATH)
		return -EINVAL;

	g_fast_fwd_site = val;

	return len;
}

static const struct class_attribute class_attr_fast_fwd_site = {
	.attr = {
		.name = "fast_fwd_site",
		.mode = 0644,
	},
	.show = fast_fwd_site_show,
	.store = fast_fwd_site_store,
};

#ifdef ENABLE_MEASURE_PPS
static int g_measure_pps_site = MEASURE_PPS_AT_NULL;

static ssize_t measure_pps_site_show(struct class *cls,
				     struct class_attribute *attr,
				     char *buf)
{
	return sprintf(buf, "current measure_pps_site: %d\n"
			    "available measure_pps_site:\n"
			    "    %d: don't measure pps at anywhere\n"
			    "    %d: measure pps before forward\n"
			    "    %d: measure pps before xmit\n",
			    g_measure_pps_site,
			    MEASURE_PPS_AT_NULL,
			    MEASURE_PPS_BEFORE_FWD,
			    MEASURE_PPS_BEFORE_TX);
}

static ssize_t measure_pps_site_store(struct class *cls,
				      struct class_attribute *attr,
				      const char *buf, size_t len)
{
	int val, ret;

	ret = kstrtoint(buf, 0, &val);

	if (val != MEASURE_PPS_AT_NULL &&
	    val != MEASURE_PPS_BEFORE_FWD &&
	    val != MEASURE_PPS_BEFORE_TX)
		return -EINVAL;

	g_measure_pps_site = val;

	return len;
}

static const struct class_attribute class_attr_measure_pps_site = {
	.attr = {
		.name = "measure_pps_site",
		.mode = 0644,
	},
	.show = measure_pps_site_show,
	.store = measure_pps_site_store,
};

static inline int measure_pps(struct sk_buff *skb, int site)
{
	static unsigned long pkts, timestamp;

	if (g_measure_pps_site == site &&
	    eth_hdr(skb)->h_proto == htons(ETH_P_IP) &&
	    ip_hdr(skb)->protocol == IPPROTO_UDP) {
		pkts++;
		if (time_after_eq(jiffies, timestamp + HZ)) {
			pr_info("UDP %s PPS %lu\n", site == MEASURE_PPS_BEFORE_FWD ? "RX" : "FWD", pkts);
			timestamp = jiffies;
			pkts = 0;
		}
		dev_kfree_skb(skb);
		return 1;
	}

	return 0;
}
#endif /* ENABLE_MEASURE_PPS */

#ifdef CONFIG_SYSFS
static inline int dev_isalive(struct net_device *dev)
{
	return dev->reg_state <= NETREG_REGISTERED;
}

static ssize_t fast_fwd_ops_store(struct device *dev, const char *buf, size_t len,
				  int (*set)(struct net_device *ndev, unsigned long))
{
	struct net_device *ndev = to_net_dev(dev);
	struct net *net = dev_net(ndev);
	unsigned long val;
	int ret;

	if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
		return -EPERM;

	ret = kstrtoul(buf, 0, &val);
	if (ret)
		return ret;

	if (!rtnl_trylock())
		return restart_syscall();

	if (dev_isalive(ndev)) {
		ret = (*set)(ndev, val);
		if (ret == 0)
			ret = len;
	}
	rtnl_unlock();

	return ret;
}

static ssize_t fast_fwd_tx_func_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct net_device *ndev = to_net_dev(dev);

	if (!dev_isalive(ndev))
		return -EINVAL;

	return sprintf(buf, "current tx function: %d\n"
			    "available tx function:\n"
			    "    %d: dev_queue_xmit\n"
			    "    %d: cls_direct_xmit\n"
			    "    %d: cls_fast_xmit, unsafe\n"
			    "    %d: cls_afap_xmit, unsafe\n",
			ndev->fast_fwd_tx_func,
			TX_BY_DEV_QUEUE_XMIT,
			TX_BY_CLS_DIRECT_XMIT,
			TX_BY_CLS_FAST_XMIT,
			TX_BY_CLS_AFAP_XMIT);
}

static int fast_fwd_tx_func_set(struct net_device *ndev, unsigned long val)
{
	if (val != TX_BY_DEV_QUEUE_XMIT &&
	    val != TX_BY_CLS_DIRECT_XMIT &&
	    val != TX_BY_CLS_FAST_XMIT &&
	    val != TX_BY_CLS_AFAP_XMIT)
		return -EINVAL;

	WRITE_ONCE(ndev->fast_fwd_tx_func, val);
	return 0;
}

static ssize_t fast_fwd_tx_func_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t len)
{
	return fast_fwd_ops_store(dev, buf, len, fast_fwd_tx_func_set);
}
static DEVICE_ATTR_RW(fast_fwd_tx_func);

static ssize_t fast_fwd_l2_pkts_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct net_device *ndev = to_net_dev(dev);

	if (!dev_isalive(ndev))
		return -EINVAL;

	return sprintf(buf, "%lu\n", ndev->fast_fwd_l2_pkts);
}

static int fast_fwd_l2_pkts_set(struct net_device *ndev, unsigned long val)
{
	WRITE_ONCE(ndev->fast_fwd_l2_pkts, val);
	return 0;
}

static ssize_t fast_fwd_l2_pkts_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t len)
{
	return fast_fwd_ops_store(dev, buf, len, fast_fwd_l2_pkts_set);
}
static DEVICE_ATTR_RW(fast_fwd_l2_pkts);

static ssize_t fast_fwd_l3_pkts_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct net_device *ndev = to_net_dev(dev);

	if (!dev_isalive(ndev))
		return -EINVAL;

	return sprintf(buf, "%lu\n", ndev->fast_fwd_l3_pkts);
}

static int fast_fwd_l3_pkts_set(struct net_device *ndev, unsigned long val)
{
	WRITE_ONCE(ndev->fast_fwd_l3_pkts, val);
	return 0;
}

static ssize_t fast_fwd_l3_pkts_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t len)
{
	return fast_fwd_ops_store(dev, buf, len, fast_fwd_l3_pkts_set);
}
static DEVICE_ATTR_RW(fast_fwd_l3_pkts);

static struct attribute *fast_fwd_attrs[] = {
	&dev_attr_fast_fwd_tx_func.attr,
	&dev_attr_fast_fwd_l2_pkts.attr,
	&dev_attr_fast_fwd_l3_pkts.attr,
	NULL
};

static const struct attribute_group fast_fwd_group = {
	.name = "fast_fwd",
	.attrs = fast_fwd_attrs,
};
#endif

static inline int skb_get_vlan_id(struct sk_buff *skb, u16 *vid)
{
	const struct ethhdr *ehdr = eth_hdr(skb);
	const struct vlan_ethhdr *vhdr;

	if (unlikely(skb_vlan_tag_present(skb))) {
		*vid = skb_vlan_tag_get_id(skb);
		return 0;
	}

	if (ehdr->h_proto == htons(ETH_P_8021Q)) {
		if (!pskb_may_pull(skb, VLAN_HLEN))
			return -EINVAL;

		vhdr = (struct vlan_ethhdr *)ehdr;
		*vid = ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK;
		return 0;
	}

	*vid = br_port_get_pvid(skb->dev);
	return 0;
}

static inline __be16 parse_ether_type(struct sk_buff *skb)
{
	/* skb forwarded by this module don't need to know the protocol precisely,
	 * return h_proto is enough
	 */
	return eth_hdr(skb)->h_proto;
}

static inline void l2_handle_vlan(struct sk_buff *skb, u16 vlan_id, struct net_device *outdev)
{
	struct net_bridge_vlan_group *vg;
	struct net_bridge_port *p;
	struct net_bridge_vlan *v;
	u8 *ehdr;

	p = br_port_get_check_rcu(outdev);
	if (!p)
		return;

	if (!br_opt_get(p->br, BROPT_VLAN_ENABLED))
		return;

	vg = nbp_vlan_group_rcu(p);
	v = br_vlan_find(vg, vlan_id);
	if (!v)
		return;

	if (v->flags & BRIDGE_VLAN_INFO_UNTAGGED) {
		__vlan_hwaccel_clear_tag(skb);
		if (skb->protocol == htons(ETH_P_8021Q)) {
			ehdr = skb_mac_header(skb);
			memmove(ehdr + VLAN_HLEN, ehdr, 2 * ETH_ALEN);
			__skb_pull(skb, VLAN_HLEN);
			skb->mac_header += VLAN_HLEN;
			skb->network_header += VLAN_HLEN;
			skb->protocol = parse_ether_type(skb);
		}
	} else {
		if (skb->protocol != htons(ETH_P_8021Q) && !skb_vlan_tag_present(skb))
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vlan_id);
	}
}

// only called by fast_fwd_from_non_npe
static inline struct net_device *l2_find_out_dev(struct sk_buff *skb, u32 *out_subport)
{
	struct ethhdr *ehdr = eth_hdr(skb);
	struct net_device *smac_outdev, *dmac_outdev;
	u32 s_subport;
	u16 smac_index, dmac_index;
	u16 vlan_id = 0;

	if (unlikely(is_multicast_ether_addr(ehdr->h_dest)))
		return NULL;

	if (unlikely(!g_get_fwt_entry_info_hook))
		return NULL;

	if (skb_get_vlan_id(skb, &vlan_id))
		return NULL;

	dmac_outdev = g_get_fwt_entry_info_hook(ehdr->h_dest, vlan_id, &dmac_index, out_subport);
	/* Between stations in a vif, packet may be resended on skb->dev.
	 * wifi-drv can ignore this case: dmac_outdev == skb->dev
	 */
	if (!dmac_outdev)
		return NULL;

	smac_outdev = g_get_fwt_entry_info_hook(ehdr->h_source, vlan_id, &smac_index, &s_subport);
	if (smac_outdev != skb->dev)
		return NULL;

	if (g_reset_fwt_ageing_hook)
		g_reset_fwt_ageing_hook(smac_index);

	l2_handle_vlan(skb, vlan_id, dmac_outdev);

	return dmac_outdev;
}

static inline u16 cls_set_queue(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	const struct net_device_ops *ops = dev->netdev_ops;
	u16 queue_id = 0;

	if (ops->ndo_select_queue)
		queue_id = ops->ndo_select_queue(dev, skb, NULL);
	skb_set_queue_mapping(skb, queue_id);

	return queue_id;
}

static inline void cls_direct_xmit(struct sk_buff *skb)
{
	u16 queue_id;

	queue_id = cls_set_queue(skb);
	dev_direct_xmit(skb, queue_id);
}

/* simplified from dev_direct_xmit, unsafe, test only */
static inline void cls_fast_xmit(struct sk_buff *skb)
{
	struct sk_buff *orig_skb = skb;
	struct net_device *dev = skb->dev;
	const struct net_device_ops *ops = dev->netdev_ops;
	bool again = false;
	int ret;

	if (unlikely(!netif_running(dev) ||
		     !netif_carrier_ok(dev)))
		goto drop;

	skb = validate_xmit_skb_list(skb, dev, &again);
	if (skb != orig_skb)
		goto drop;

	cls_set_queue(skb);

	local_bh_disable();
	ret = ops->ndo_start_xmit(skb, dev);
	local_bh_enable();

	if (!dev_xmit_complete(ret))
		kfree_skb(skb);

	return;
drop:
	atomic_long_inc(&dev->tx_dropped);
	kfree_skb_list(skb);
}

/* xmit as fast as possible, unsafe, test only */
static inline void cls_afap_xmit(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	const struct net_device_ops *ops = dev->netdev_ops;
	int ret;

	cls_set_queue(skb);

	ret = ops->ndo_start_xmit(skb, dev);
	if (!dev_xmit_complete(ret))
		kfree_skb_list(skb);
}

void register_l3_find_out_dev(struct net_device *(*func)(struct sk_buff *skb))
{
	l3_find_out_dev = func;
}
EXPORT_SYMBOL(register_l3_find_out_dev);

static bool l2_check_and_update_src_fdb(struct sk_buff *skb)
{
	struct ethhdr *ehdr = eth_hdr(skb);
	struct net_device *smac_outdev;
	u32 s_subport;
	u16 smac_index;
	u16 vlan_id = 0;

	if (skb_get_vlan_id(skb, &vlan_id))
		return false;

	smac_outdev = g_get_fwt_entry_info_hook(ehdr->h_source, vlan_id, &smac_index, &s_subport);
	if (smac_outdev != skb->dev)
		return false;

	if (g_reset_fwt_ageing_hook)
		g_reset_fwt_ageing_hook(smac_index);

	return true;
}

static int fast_fwd_from_non_npe(struct sk_buff *skb, struct net_device *in_dev, bool by_ingress_hook)
{
	struct net_device *out_dev = NULL;
	u32 out_subport = 0;

	if (ether_addr_equal_64bits(eth_hdr(skb)->h_dest, g_lan_mac_addr)) {
		if (!l3_find_out_dev)
			return 0;
		out_dev = l3_find_out_dev(skb);
		if (!out_dev || (netif_is_bridge_port(in_dev) && !l2_check_and_update_src_fdb(skb)))
			return 0;
		skb->bmu_flag |= CLS_BMU_FLAG_HW_ACCEL;
		in_dev->fast_fwd_l3_pkts++;
	} else if (netif_is_bridge_port(in_dev)) {
		if (unlikely(is_dns_port(skb)))
			return 0;
		out_dev = l2_find_out_dev(skb, &out_subport);
		if (!out_dev)
			return 0;
		in_dev->fast_fwd_l2_pkts++;
	} else {
		return 0;
	}

	if (by_ingress_hook)
		skb_push(skb, ETH_HLEN);

	skb->dev = out_dev;
	skb->dest_port = out_subport;

#ifdef ENABLE_MEASURE_PPS
	if (measure_pps(skb, MEASURE_PPS_BEFORE_TX))
		return 1;
#endif

	switch (out_dev->fast_fwd_tx_func) {
	case TX_BY_DEV_QUEUE_XMIT:
		dev_queue_xmit(skb);
		break;
	case TX_BY_CLS_DIRECT_XMIT:
		cls_direct_xmit(skb);
		break;
	case TX_BY_CLS_FAST_XMIT:
		cls_fast_xmit(skb);
		break;
	case TX_BY_CLS_AFAP_XMIT:
		cls_afap_xmit(skb);
		break;
	}

	return 1;
}

static int fast_fwd_from_npe(struct sk_buff *skb, struct net_device *in_dev, bool by_ingress_hook)
{
	struct ethhdr *ehdr = eth_hdr(skb);
	struct net_device *smac_outdev, *dmac_outdev;
	u32 s_subport, d_subport;
	u16 smac_index, dmac_index;
	u16 vlan_id = 0;
	const u16 *vlan_list;
	int i, vlan_num;

	if (unlikely(is_multicast_ether_addr(ehdr->h_dest) || !g_get_fwt_entry_info_hook))
		return 0;

	if (skb_get_vlan_id(skb, &vlan_id))
		return 0;

	if (ether_addr_equal_64bits(ehdr->h_source, g_lan_mac_addr)) {
		if (unlikely(netif_is_bridge_port(in_dev) &&
			     ether_addr_equal_64bits(ehdr->h_source, br_port_get_rcu(in_dev)->br->dev->dev_addr)))
			return 0;
		dmac_outdev = g_get_fwt_entry_info_hook(ehdr->h_dest, vlan_id, &dmac_index, &d_subport);
		if (!dmac_outdev && !vlan_id) {
			/* WAN->WLAN untagged packet, try all available vlan */
			vlan_list = br_get_vlan_list(&vlan_num);
			if (!vlan_list)
				return 0;
			for (i = 0; i < vlan_num; ++i) {
				dmac_outdev = g_get_fwt_entry_info_hook(ehdr->h_dest, vlan_list[i],
									&dmac_index, &d_subport);
				if (dmac_outdev)
					break;
			}
		}
		if (unlikely(!dmac_outdev || dmac_outdev == in_dev))
			return 0;
		in_dev->fast_fwd_l3_pkts++;
	} else if (netif_is_bridge_port(in_dev)) {
		dmac_outdev = g_get_fwt_entry_info_hook(ehdr->h_dest, vlan_id, &dmac_index, &d_subport);
		if (unlikely(!dmac_outdev || dmac_outdev == in_dev))
			return 0;

		smac_outdev = g_get_fwt_entry_info_hook(ehdr->h_source, vlan_id, &smac_index, &s_subport);
		if (unlikely(smac_outdev != in_dev))
			return 0;

		if (g_reset_fwt_ageing_hook)
			g_reset_fwt_ageing_hook(smac_index);

		l2_handle_vlan(skb, vlan_id, dmac_outdev);
		in_dev->fast_fwd_l2_pkts++;
	} else {
		return 0;
	}

	if (by_ingress_hook)
		skb_push(skb, ETH_HLEN);

	skb->dev = dmac_outdev;
	skb->dest_port = d_subport;

#ifdef ENABLE_MEASURE_PPS
	if (measure_pps(skb, MEASURE_PPS_BEFORE_TX))
		return 1;
#endif

	switch (dmac_outdev->fast_fwd_tx_func) {
	case TX_BY_DEV_QUEUE_XMIT:
		dev_queue_xmit(skb);
		break;
	case TX_BY_CLS_DIRECT_XMIT:
		cls_direct_xmit(skb);
		break;
	case TX_BY_CLS_FAST_XMIT:
		cls_fast_xmit(skb);
		break;
	case TX_BY_CLS_AFAP_XMIT:
		cls_afap_xmit(skb);
		break;
	}

	return 1;
}

int clsemi_fast_fwd(struct sk_buff *skb, struct net_device *in_dev, bool from_npe_eth_dev)
{
	if (g_fast_fwd_site != FAST_FWD_AT_DRIVER_RX_PATH)
		return 0;

	skb_reset_mac_header(skb);

	skb->dev = in_dev;
	skb->protocol = parse_ether_type(skb);
	skb_set_network_header(skb, ETH_HLEN);

#ifdef ENABLE_MEASURE_PPS
	if (measure_pps(skb, MEASURE_PPS_BEFORE_FWD))
		return 1;
#endif

	if (from_npe_eth_dev)
		return fast_fwd_from_npe(skb, in_dev, false);
	else
		return fast_fwd_from_non_npe(skb, in_dev, false);
}
EXPORT_SYMBOL(clsemi_fast_fwd);

static unsigned int clsemi_fast_fwd_hook(void *priv, struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
	int ret;
	struct net_device *in_dev = skb->dev;

	if (g_fast_fwd_site != FAST_FWD_AT_NF_NETDEV_INGRESS)
		return NF_ACCEPT;

#ifdef ENABLE_MEASURE_PPS
	if (measure_pps(skb, MEASURE_PPS_BEFORE_FWD))
		return 1;
#endif

	if (if_port_is_npe_eth_port(in_dev->if_port))
		ret = fast_fwd_from_npe(skb, in_dev, true);
	else
		ret = fast_fwd_from_non_npe(skb, in_dev, true);

	if (ret)
		return NF_STOLEN;

	return NF_ACCEPT;
}

static int __hotplug_dev_fast_fwd_hook(struct sk_buff *skb, const struct nf_hook_state *state)
{
	unsigned int verdict;
	int ret;

	/* refer from nf_hook_slow() */
	verdict = clsemi_fast_fwd_hook(NULL, skb, state);
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
	return 1;
}

static int dev_init_fast_fwd(struct net_device *dev)
{
	struct kobject *kobj = &dev->dev.kobj;
	struct nf_hook_ops *ops;
	int err = 0;

	if (dev->type != ARPHRD_ETHER || dev->flags & IFF_LOOPBACK ||
	    netif_is_bridge_master(dev))
		return -ENOTSUPP;

	if (dev->ieee80211_ptr) {
		pr_info("ready to accelerate packet from hotplug dev %s\n", dev->name);
		dev->hotplug_dev_flow_offload = true;
		goto done;
	}

	ops = kzalloc(sizeof(*ops), GFP_KERNEL);
	if (!ops)
		return -ENOMEM;

	ops->hook	= clsemi_fast_fwd_hook;
	ops->pf		= NFPROTO_NETDEV;
	ops->hooknum	= NF_NETDEV_INGRESS;
	ops->priority	= INT_MIN + 1;
	ops->dev	= dev;
	ops->priv	= NULL;

	err = nf_register_net_hook(&init_net, ops);
	if (err < 0) {
		kfree(ops);
		return err;
	}

	dev->fast_fwd_ingress_hook_ops = ops;
	pr_info("ready to accelerate packet from %s\n", dev->name);

done:
#ifdef CONFIG_SYSFS
	err = sysfs_create_group(kobj, &fast_fwd_group);
#endif
	return err;
}

static void dev_deinit_fast_fwd(struct net_device *dev)
{
	struct kobject *kobj = &dev->dev.kobj;

	if (dev->fast_fwd_ingress_hook_ops) {
		nf_unregister_net_hook(&init_net, dev->fast_fwd_ingress_hook_ops);
		kfree(dev->fast_fwd_ingress_hook_ops);
		dev->fast_fwd_ingress_hook_ops = NULL;
	}
#ifdef CONFIG_SYSFS
	if (dev->fast_fwd_ingress_hook_ops || dev->hotplug_dev_flow_offload)
		sysfs_remove_group(kobj, &fast_fwd_group);
#endif
}

static void guess_lan_mac_addr(struct net_device *dev)
{
	if (g_lan_mac_addr_specified)
		return;

	if (!netif_is_bridge_master(dev))
		return;

	ether_addr_copy(g_lan_mac_addr, dev->dev_addr);
}

static int fast_fwd_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);

	switch (event) {
	case NETDEV_REGISTER:
		dev_init_fast_fwd(dev);
		break;
	case NETDEV_UP:
		if (if_port_is_npe_eth_port(dev->if_port))
			dev->fast_fwd_tx_func = TX_BY_DEV_QUEUE_XMIT;
		else
			dev->fast_fwd_tx_func = TX_BY_CLS_DIRECT_XMIT;
		break;
	case NETDEV_UNREGISTER:
		dev_deinit_fast_fwd(dev);
		break;
	case NETDEV_CHANGEADDR:
		guess_lan_mac_addr(dev);
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block fast_fwd_notifier = {
	.notifier_call = fast_fwd_event,
};

static int __init clsemi_fast_fwd_init(void)
{
	register_netdevice_notifier(&fast_fwd_notifier);

	sysfs_attr_init(&class_attr_lan_mac_addr.attr);
	sysfs_attr_init(&class_attr_fast_fwd_site.attr);
	sysfs_attr_init(&class_attr_sw_fwd_vid.attr);
	netdev_class_create_file_ns(&class_attr_lan_mac_addr, &init_net);
	netdev_class_create_file_ns(&class_attr_fast_fwd_site, &init_net);
	netdev_class_create_file_ns(&class_attr_sw_fwd_vid, &init_net);
#ifdef ENABLE_MEASURE_PPS
	sysfs_attr_init(&class_attr_measure_pps_site.attr);
	netdev_class_create_file_ns(&class_attr_measure_pps_site, &init_net);
#endif
	hotplug_dev_fast_fwd_hook = __hotplug_dev_fast_fwd_hook;

	return 0;
}

static void __exit clsemi_fast_fwd_exit(void)
{
	hotplug_dev_fast_fwd_hook = NULL;
	unregister_netdevice_notifier(&fast_fwd_notifier);
	netdev_class_remove_file_ns(&class_attr_lan_mac_addr, &init_net);
	netdev_class_remove_file_ns(&class_attr_fast_fwd_site, &init_net);
#ifdef ENABLE_MEASURE_PPS
	netdev_class_remove_file_ns(&class_attr_measure_pps_site, &init_net);
#endif
}

module_init(clsemi_fast_fwd_init);
module_exit(clsemi_fast_fwd_exit);
