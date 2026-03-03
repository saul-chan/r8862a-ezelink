/*SH0
 *******************************************************************************
 **                                                                           **
 **         Copyright (c) 2025 Clouneysemi Communications Inc                 **
 **                            All Rights Reserved                            **
 **                                                                           **
 **  Author      : Clourysemi Communications Inc                              **
 **  File        : cls_wifi_bridge.c                                          **
 **  Description : 3-address mode bridging                                    **
 **                                                                           **
 **  This module maintains two tables.                                        **
 **                                                                           **
 **  1. A Unicast table containing the MAC address and IP address of each     **
 **     downstream client. This is used to determine the destination Ethernet **
 **     MAC address for dowstream frames.                                     **
 **                                                                           **
 **  2. A Multicast table for keeping track of multicast group registrations  **
 **     made by downstream clients.                                           **
 **     Each multicast table entry contains an multicast IP address and a     **
 **     list of one or more downstream clients that have joined the           **
 **     multicast group.                                                      **
 **     Downstream clients are not visible to the AP in 3-address mode, so    **
 **     the AP 'sees' only that the station is joined to the multicast group. **
 **     This table is used to ensure that an upstream  multicast LEAVE        **
 **     message is only sent when the last downstream client leaves a         **
 **     group.                                                                **
 **                                                                           **
 *******************************************************************************/
/**
  Copyright (c) 2025 Clouneysemi Communications Inc
  All Rights Reserved

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include <linux/ip.h>
#include <linux/in.h>
#include <linux/spinlock.h>
#include <linux/jhash.h>
#include <linux/if_arp.h>
#include "linux/udp.h"
#include "linux/igmp.h"
#include <net/ipv6.h>
#include <linux/bug.h>
#include <linux/icmpv6.h>
#include <linux/types.h>
#include <linux/hashtable.h>
#include <linux/jiffies.h>
#include <linux/in6.h>
#include <linux/etherdevice.h>
#include <linux/slab.h>
#include <linux/inet.h>
#include "cls_wifi_defs.h"

/* These definitions are not present in any public header files */
struct dhcpMessage {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t other_fields[0];
}__attribute__ ((packed));

struct udp_dhcp_packet {
	struct iphdr iphdr_p;
	struct udphdr udphdr_p;
	struct dhcpMessage dhcp_msg;
}__attribute__ ((packed));

#define BOOTREQUEST	1
#define DHCPSERVER_PORT	67
#define DHCPREQUEST	3
#define ARPHRD_ETHER	1
#define DHCP_BROADCAST_FLAG	0x8000

#define CLS_WIFI_IGMP_OP_NONE	0x00
#define CLS_WIFI_IGMP_OP_JOIN	0x01
#define CLS_WIFI_IGMP_OP_LEAVE	0x02
#define CLS_WIFI_IP_MCAST_PREF	0xE

#define CLS_WIFI_BR_PRINT_BUF_SIZE 2048
#define CLS_WIFI_BR_PRINT_LINE_MAX 48

static struct kmem_cache *br_uc_cache __read_mostly = NULL;
static struct kmem_cache *br_mc_cache __read_mostly = NULL;
static struct kmem_cache *br_mc_client_cache __read_mostly = NULL;

#ifdef CONFIG_IPV6
#define IPV6_MAC_TABLE_SIZE 1024
#define ENTRY_EXPIRE_TIME   (1800 * HZ)  // 1800 sec expire

struct ipv6_mac_entry {
	struct in6_addr ip;
	unsigned char mac_addr[ETH_ALEN];
	unsigned long timestamp;
	struct hlist_node hnode;
};

static DEFINE_HASHTABLE(ipv6_mac_table, ilog2(IPV6_MAC_TABLE_SIZE));
static DEFINE_SPINLOCK(ipv6_mac_table_lock);
static struct kmem_cache *entry_cache __read_mostly;
static struct timer_list cleanup_timer;

void cleanup_ipv6_old_entries(void)
{
	struct ipv6_mac_entry *entry;
	struct hlist_node *tmp;
	unsigned long now = jiffies;
	unsigned long bkt;

	spin_lock_bh(&ipv6_mac_table_lock);

	hash_for_each_safe(ipv6_mac_table, bkt, tmp, entry, hnode) {
		if (time_after(now, entry->timestamp + ENTRY_EXPIRE_TIME)) {
			hash_del(&entry->hnode);
			kmem_cache_free(entry_cache, entry);
		}
	}

	spin_unlock_bh(&ipv6_mac_table_lock);
}

#if 0
/*TODO*/
void cleanup_ipv4_old_entries(struct cls_wifi_br *br)
{
	struct hlist_head *head = &br->uc_ip_hash[cls_wifi_br_ip_hash(ip_addr)];
	struct cls_wifi_br_uc *br_uc;

	hlist_for_each_entry_safe_rcu(br_uc, head, ip_hlist) {
		if (time_after(now, br_uc->timestamp + ENTRY_EXPIRE_TIME)) {
			cls_wifi_br_uc_delete(br, br_uc);
		}
	}

}
#endif

static void timer_callback(struct timer_list *unused)
{
	cleanup_ipv6_old_entries();
	mod_timer(&cleanup_timer, jiffies + 60 * HZ);
}

static void start_cleanup_timer(void)
{
	timer_setup(&cleanup_timer, timer_callback, 0);
	mod_timer(&cleanup_timer, jiffies + 60 * HZ);
}

static int ipv6_mac_table_init(void)
{
	entry_cache = kmem_cache_create("ipv6_mac_entries",
			sizeof(struct ipv6_mac_entry),
			0,
			SLAB_HWCACHE_ALIGN | SLAB_PANIC,
			NULL);
	if (!entry_cache)
		return -ENOMEM;

	start_cleanup_timer();

	return 0;
}

static void ipv6_mac_table_exit(void)
{
	struct ipv6_mac_entry *entry;
	struct hlist_node *tmp;
	unsigned long bkt;

	spin_lock_bh(&ipv6_mac_table_lock);

	//clean hash list
	hash_for_each_safe(ipv6_mac_table, bkt, tmp, entry, hnode) {
		hash_del(&entry->hnode);
		kmem_cache_free(entry_cache, entry);
	}

	spin_unlock_bh(&ipv6_mac_table_lock);

	kmem_cache_destroy(entry_cache);

	del_timer_sync(&cleanup_timer);
}

static u32 ipv6_mac_hash(const struct in6_addr *src_ip)
{
	return jhash2((const u32 *)src_ip,
			sizeof(struct in6_addr) / sizeof(u32),
			0);
}

void update_ipv6_mac_entry(const struct in6_addr *src_ip, const unsigned char *mac)
{
	struct ipv6_mac_entry *entry;
	u32 hash = ipv6_mac_hash(src_ip);

	spin_lock_bh(&ipv6_mac_table_lock);

	// find exist entry
	hash_for_each_possible(ipv6_mac_table, entry, hnode, hash) {
		if (ipv6_addr_equal(&entry->ip, src_ip)) {
			//update old entry
			memcpy(entry->mac_addr, mac, ETH_ALEN);
			entry->timestamp = jiffies;
			spin_unlock_bh(&ipv6_mac_table_lock);
			return;
		}
	}

	entry = kmem_cache_alloc(entry_cache, GFP_ATOMIC);
	if (!entry) {
		spin_unlock_bh(&ipv6_mac_table_lock);
		pr_err("alloc ipv6 3addr entry fail\n");
		return;
	}

	ipv6_addr_copy(&entry->ip, src_ip);
	memcpy(entry->mac_addr, mac, ETH_ALEN);
	entry->timestamp = jiffies;

	hash_add(ipv6_mac_table, &entry->hnode, hash);

	spin_unlock_bh(&ipv6_mac_table_lock);
}

void icmpv6_change_mac(uint16_t *old_sum, uint8_t *lladdr_pos, const u8 *new_mac)
{
	uint8_t old_mac[ETH_ALEN], *old_mac_ptr = lladdr_pos;
	uint32_t old_4 = *(uint32_t *)old_mac_ptr, new_4 = *(uint32_t *)new_mac;
	uint16_t old_2 = *(uint16_t *)(old_mac_ptr+4), new_2 = *(uint16_t *)(new_mac+4);
#if 0
	pr_err("old_4:%08x old_2:%04x, new_4:%08x new_2:%04x\n", old_4, old_2, new_4, new_2);
#endif
	memcpy(old_mac, old_mac_ptr, ETH_ALEN);
	memcpy(old_mac_ptr, new_mac, ETH_ALEN);
	/* update new checksum*/
	csum_replace4(old_sum, old_4, new_4);
	csum_replace2(old_sum, old_2, new_2);
#if 0
	pr_err("old mac:%pM, new mac:%pM, old sum =%02x new sum=%02x\n", old_mac, new_mac, old_sum, *old_sum);
#endif

}

int find_ipv6_subtype_header(struct sk_buff *skb, int target_subtype, void **header_addr)
{
	struct ipv6hdr *ip6h;
	u8 nexthdr;
	unsigned int offset;
	__be16 frag_off = 0;

	if (!skb || !header_addr)
		return -EINVAL;

	if (!pskb_may_pull(skb, sizeof(struct ipv6hdr)))
		return -ENODATA;

	ip6h = ipv6_hdr(skb);
	nexthdr = ip6h->nexthdr;
	offset = sizeof(struct ipv6hdr);

	offset = ipv6_skip_exthdr(skb, skb_network_offset(skb) + offset,
			&nexthdr, &frag_off);

	if (offset < 0)
		return offset;

	if (nexthdr == target_subtype) {
		if (offset <= skb->len) {
			*header_addr = (void *)(skb->data + offset);
			skb->transport_header = *header_addr - (void *)skb->head;
			return 0;
		}
		return -ENODATA;
	}

	return -ENOENT;
}

int skb_get_icmpv6_lladdr(struct sk_buff *skb, u8 **lladdr, uint8_t *type, const u8 *new_mac)
{
	struct icmp6hdr *icmp6h;
	u8 *opt_ptr;
	int opt_len;
	int offset;
	int ret;

	/* check SKB has ICMPv6 header */
	if (!pskb_may_pull(skb, sizeof(struct icmp6hdr)))
		return -EINVAL;

	ret = find_ipv6_subtype_header(skb, IPPROTO_ICMPV6, (void **)&icmp6h);

	/*not found icmpv6 header*/
	if (ret)
		return ret;

	/* only handle (NS) and (NA) */
	if (icmp6h->icmp6_type != NDISC_NEIGHBOUR_SOLICITATION &&
			icmp6h->icmp6_type != NDISC_NEIGHBOUR_ADVERTISEMENT)
		return -ENOENT;

	opt_len = skb->len - skb_transport_offset(skb) - sizeof(struct icmp6hdr);

	/* get opt pointer */
	opt_ptr = skb_transport_header(skb) + sizeof(struct icmp6hdr);
#if 0
	print_hex_dump(KERN_ERR, "Dump: ", DUMP_PREFIX_OFFSET, 16, 1, opt_ptr, opt_len, true);
#endif

	/*change opt offset according to ICMPv6 type*/
	if (icmp6h->icmp6_type == NDISC_NEIGHBOUR_ADVERTISEMENT ||
			icmp6h->icmp6_type == NDISC_NEIGHBOUR_SOLICITATION) {
		offset = sizeof(struct in6_addr);
		if (opt_len < offset)
			return -EINVAL;
		opt_ptr += offset;
		opt_len -= offset;
	}
#if 0
	print_hex_dump(KERN_ERR, "Dump: ", DUMP_PREFIX_OFFSET, 16, 1, opt_ptr, opt_len, true);
#endif

	/* check all opts */
	while (opt_len >= 2) {
		u8 opt_type = *opt_ptr;
		u8 opt_len_bytes = *(opt_ptr + 1) << 3;  /* convert to bytes */

		if (opt_len_bytes < 2 || opt_len_bytes > opt_len)
			break;
#if 0
		pr_err("opt_type=%d\n", opt_type);
		pr_err("opt_len_bytes=%d\n", opt_len_bytes);
#endif
		if (opt_type == ND_OPT_SOURCE_LL_ADDR
				|| opt_type == ND_OPT_TARGET_LL_ADDR) {
			int addr_len = opt_len_bytes - 2;

			if (addr_len != ETH_ALEN) {
				break;
			}

			*lladdr = opt_ptr + 2;
			*type = opt_type;
			icmpv6_change_mac(&icmp6h->icmp6_cksum, *lladdr, new_mac);

			return 0;
		} else {
			pr_debug("skip handle ndisc type:%d\n", opt_type);
		}

		/* check next opt */
		opt_len -= opt_len_bytes;
		opt_ptr += opt_len_bytes;
	}

	return -ENOENT;
}

int replace_src_mac(struct sk_buff *skb)
{
	struct ethhdr *eth;
	struct net_device *dev;

	if (skb->protocol != htons(ETH_P_IPV6))
		return -3;

	if (!skb || !skb->dev)
		return -1;

	dev = skb->dev;

	eth = eth_hdr(skb);
	if (!eth)
		return -2;

	memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);

	return 0;

}

const unsigned char *find_rx_mac_by_ipv6_safe(const struct in6_addr *ip)
{
	struct ipv6_mac_entry *entry;
	u32 hash = ipv6_mac_hash(ip);
	const unsigned char *found_mac = NULL;

	rcu_read_lock();

	hash_for_each_possible_rcu(ipv6_mac_table, entry, hnode, hash) {
		if (ipv6_addr_equal(&entry->ip, ip)) {
			found_mac = entry->mac_addr;
			break;
		}
	}

	rcu_read_unlock();
	return found_mac;
}

int cls_wifi_process_tx_ipv6_packet(struct sk_buff *skb, int type)
{
	struct ipv6hdr *ip6h = ipv6_hdr(skb);
	struct ethhdr *eth = eth_hdr(skb);
	uint8_t icmpv6_op_type = 0, *orig_mac = NULL;
	int ret = 0;

	if (!is_multicast_ether_addr(eth->h_source) &&
			!is_broadcast_ether_addr(eth->h_source)) {
#if 0
		pr_debug("update ipv6_mac_entry");
#endif
		update_ipv6_mac_entry(&ip6h->saddr, eth->h_source);
	}

	if(replace_src_mac(skb))
		return -1;

	if (type == IPPROTO_ICMPV6 && skb->dev) {
		ret = skb_get_icmpv6_lladdr(skb, &orig_mac, &icmpv6_op_type, skb->dev->dev_addr);
		pr_debug("ret = %d icmpv6_op_type=%d\n",ret, icmpv6_op_type);
		if(orig_mac) {
			pr_debug("change icmp ll addr to %pM\n", orig_mac);
		}else {
			pr_debug("not found icmpv6 op=%d\n", icmpv6_op_type);
		}
	}

	return 0;
}

int cls_wifi_process_rx_ipv6_packet(struct sk_buff *skb)
{
	struct ipv6hdr *ip6h;
	struct ethhdr *eth;
	const unsigned char *real_dst_mac;
	unsigned char orig_dst_mac[ETH_ALEN];

	eth = eth_hdr(skb);
	if (!eth)
		return -1;

	if (is_multicast_ether_addr(eth->h_dest) ||
			is_broadcast_ether_addr(eth->h_dest)) {
		return 0;
	}

	if (skb->protocol != htons(ETH_P_IPV6))
		return -3;

	ip6h = ipv6_hdr(skb);
	if (!ip6h)
		return -4;

	memcpy(orig_dst_mac, eth->h_dest, ETH_ALEN);

	real_dst_mac = find_rx_mac_by_ipv6_safe(&ip6h->daddr);

	if (real_dst_mac) {
#undef DEBUG
#ifdef DEBUG
		char src_ip[INET6_ADDRSTRLEN], dst_ip[INET6_ADDRSTRLEN];
#endif
		memcpy(eth->h_dest, real_dst_mac, ETH_ALEN);
#ifdef DEBUG
		snprintf(src_ip, INET6_ADDRSTRLEN, "%pI6", &ip6h->saddr);
		snprintf(dst_ip, INET6_ADDRSTRLEN, "%pI6", &ip6h->daddr);
		printk(KERN_ERR "Modified dest MAC: %pM -> %pM for dst IP %s\n",
				eth->h_dest, real_dst_mac, dst_ip);
#endif
	} else {
#ifdef DEBUG
		char dst_ip[INET6_ADDRSTRLEN];
		snprintf(dst_ip, INET6_ADDRSTRLEN, "%pI6", &ip6h->daddr);
		printk(KERN_ERR "No MAC found for dst IP: %s\n", dst_ip);
#endif
	}

	return 0;
}
#endif

/*
 * Create a hash of a MAC address
 */
static int
cls_wifi_br_mac_hash(unsigned char *mac_addr)
{
	return jhash(mac_addr, ETH_ALEN, 0) & (CLS_WIFI_BR_MAC_HASH_SIZE - 1);
}

/*
 * Create a hash of an IP address
 */
static int
cls_wifi_br_ip_hash(__be32 ip_addr)
{
	return jhash_1word(ip_addr, 0) & (CLS_WIFI_BR_IP_HASH_SIZE - 1);
}

/*
 * Lock the unicast table for write access
 */
static void
cls_wifi_br_uc_lock(struct cls_wifi_br *br)
{
	spin_lock_irqsave(&br->uc_lock, br->uc_lock_flags);
}

/*
 * Unlock the unicast table
 */
static void
cls_wifi_br_uc_unlock(struct cls_wifi_br *br)
{
	spin_unlock_irqrestore(&br->uc_lock, br->uc_lock_flags);
}

/*
 * Lock the multicast table for write access
 */
static void
cls_wifi_br_lock_mc(struct cls_wifi_br *br)
{
	spin_lock_irqsave(&br->mc_lock, br->mc_lock_flags);
}

/*
 * Unlock the multicast table
 */
static void
cls_wifi_br_unlock_mc(struct cls_wifi_br *br)
{
	spin_unlock_irqrestore(&br->mc_lock, br->mc_lock_flags);
}

static int
cls_wifi_br_ip_is_unicast(u32 ip_addr)
{
	if ((ip_addr != INADDR_ANY) &&
			(ip_addr != INADDR_BROADCAST) &&
			(!IN_MULTICAST(ntohl(ip_addr)))) {
		return 1;
	}

	return 0;
}

/*
 * Free a unicast entry
 */
static void
cls_wifi_br_uc_free(struct rcu_head *head)
{
	struct cls_wifi_br_uc *br_uc;

	br_uc = container_of(head, struct cls_wifi_br_uc, rcu);

	kmem_cache_free(br_uc_cache, br_uc);
}

/*
 * Free a multicast entry
 */
static void
cls_wifi_br_mc_free(struct rcu_head *head)
{
	struct cls_wifi_br_mc *br_mc;

	br_mc = container_of(head, struct cls_wifi_br_mc, rcu);

	kmem_cache_free(br_mc_cache, br_mc);
}

/*
 * Free a multicast client entry
 */
static void
cls_wifi_br_mc_client_free(struct rcu_head *head)
{
	struct cls_wifi_br_mc_client *br_mc_client;

	br_mc_client = container_of(head, struct cls_wifi_br_mc_client, rcu);

	kmem_cache_free(br_mc_client_cache, br_mc_client);
}

/*
 * Remove a multicast client entry from a multicast entry
 * Assumes the multicast table is locked for write.
 */
static void
cls_wifi_br_mc_client_delete(struct cls_wifi_br *br, struct cls_wifi_br_mc *br_mc,
		struct cls_wifi_br_mc_client *br_mc_client)
{
	hlist_del_rcu(&br_mc_client->mc_client_hlist);
	call_rcu(&br_mc_client->rcu, cls_wifi_br_mc_client_free);
	atomic_dec(&br_mc->mc_client_tot);
	atomic_dec(&br->mc_tot);
}

/*
 * Remove a multicast address entry
 * Assumes the multicast table is locked for write.
 */
static void
cls_wifi_br_mc_delete(struct cls_wifi_br *br, struct cls_wifi_br_mc *br_mc)
{
	hlist_del_rcu(&br_mc->mc_hlist);
	call_rcu(&br_mc->rcu, cls_wifi_br_mc_free);
	atomic_dec(&br->mc_tot);
}

/*
 * Remove a unicast entry
 * Assumes the unicast table is locked for write.
 */
static void
cls_wifi_br_uc_delete(struct cls_wifi_br *br, struct cls_wifi_br_uc *br_uc)
{
	hlist_del_rcu(&br_uc->mac_hlist);
	hlist_del_rcu(&br_uc->ip_hlist);
	call_rcu(&br_uc->rcu, cls_wifi_br_uc_free);
	atomic_dec(&br->uc_tot);
	pr_debug("delete br uc entry %pM %pI4\n", br_uc->mac_addr, &(br_uc->ip_addr));

	/*TODO*/
	//fwt_sw_remove_uc_ipmac(br_uc->ip_addr);
}

/*
 * Find a multicast client entry
 * Assumes the multicast table is locked for read or write.
 */
static struct cls_wifi_br_mc_client *
cls_wifi_br_mc_client_find(struct cls_wifi_br_mc *br_mc, unsigned char *mac_addr)
{
	struct hlist_head *head = &br_mc->mc_client_hash[cls_wifi_br_mac_hash(mac_addr)];
	struct cls_wifi_br_mc_client *br_mc_client;

	hlist_for_each_entry_rcu(br_mc_client, head, mc_client_hlist) {
		if (ether_addr_equal(br_mc_client->mac_addr, mac_addr)) {
			return br_mc_client;
		}
	}

	return NULL;
}

/*
 * Find a multicast entry
 * Assumes the multicast table is locked for read or write.
 */
static struct cls_wifi_br_mc *
cls_wifi_br_mc_find(struct cls_wifi_br *br, __be32 mc_ip_addr)
{
	struct hlist_head *head = &br->mc_ip_hash[cls_wifi_br_ip_hash(mc_ip_addr)];
	struct cls_wifi_br_mc *br_mc;

	hlist_for_each_entry_rcu(br_mc, head, mc_hlist) {
		if (br_mc->mc_ip_addr == mc_ip_addr) {
			return br_mc;
		}
	}

	return NULL;
}

/*
 * Find a unicast entry by IP address
 * Assumes the unicast table is locked for read or write.
 */
static struct cls_wifi_br_uc *
cls_wifi_br_uc_find_by_ip(struct cls_wifi_br *br, u32 ip_addr)
{
	struct hlist_head *head = &br->uc_ip_hash[cls_wifi_br_ip_hash(ip_addr)];
	struct cls_wifi_br_uc *br_uc;

	hlist_for_each_entry_rcu(br_uc, head, ip_hlist) {
		if (br_uc->ip_addr == ip_addr) {
			return br_uc;
		}
	}

	return NULL;
}

/*
 * Find a unicast entry by MAC address
 * Assumes the unicast table is locked for read or write.
 */
static struct cls_wifi_br_uc *
cls_wifi_br_uc_find_by_mac(struct cls_wifi_br *br, unsigned char *mac_addr)
{
	struct hlist_head *head = &br->uc_mac_hash[cls_wifi_br_mac_hash(mac_addr)];
	struct cls_wifi_br_uc *br_uc;

	hlist_for_each_entry_rcu(br_uc, head, mac_hlist) {
		if (ether_addr_equal(br_uc->mac_addr, mac_addr)) {
			//pr_err("found  uc mac ip\n");
			return br_uc;
		}
	}

	pr_err("no found  uc mac ip\n");
	return NULL;
}

/*
 * Add a multicast client entry to a multicast entry
 * Assumes the multicast table is locked for write.
 */
static void
cls_wifi_br_mc_client_add(struct cls_wifi_br *br, struct cls_wifi_br_mc *br_mc,
		unsigned char *mac_addr)
{
	struct cls_wifi_br_mc_client *br_mc_client;

	if (atomic_read(&br->mc_tot) >= CLS_WIFI_BR_MCAST_MAX) {
		pr_err("multicast table is full, can't add %pM\n",
				mac_addr);
		return;
	}

	br_mc_client = kmem_cache_alloc(br_mc_client_cache, GFP_ATOMIC);
	if (br_mc_client == NULL) {
		pr_err("failed to allocate multicast client entry %pM\n",
				mac_addr);
		return;
	}

	memset(br_mc_client, 0, sizeof(*br_mc_client));
	atomic_inc(&br->mc_tot);
	atomic_inc(&br_mc->mc_client_tot);
	ether_addr_copy(br_mc_client->mac_addr, mac_addr);
	hlist_add_head_rcu(&br_mc_client->mc_client_hlist,
			&br_mc->mc_client_hash[cls_wifi_br_mac_hash(mac_addr)]);
}

/*
 * Add multicast entry
 * Assumes the multicast table is locked for write.
 */
static struct cls_wifi_br_mc *
cls_wifi_br_mc_add(struct cls_wifi_br *br, __be32 mc_ip_addr)
{
	struct cls_wifi_br_mc *br_mc;

	if (atomic_read(&br->mc_tot) >= CLS_WIFI_BR_MCAST_MAX) {
		pr_err("multicast table is full, cant add %pI4\n", &mc_ip_addr);
		return NULL;
	}

	br_mc = kmem_cache_alloc(br_mc_cache, GFP_ATOMIC);
	if (br_mc == NULL) {
		pr_err("failed to allocate multicast entry %pI4\n", &mc_ip_addr);
		return NULL;
	}

	memset(br_mc, 0, sizeof(*br_mc));

	atomic_inc(&br->mc_tot);
	br_mc->mc_ip_addr = mc_ip_addr;
	hlist_add_head_rcu(&br_mc->mc_hlist, &br->mc_ip_hash[cls_wifi_br_ip_hash(mc_ip_addr)]);

	return br_mc;
}

/*
 * Add a unicast entry
 * Assumes the unicast table is locked for write.
 */
static void cls_wifi_br_uc_add(struct cls_wifi_br *br, unsigned char *mac_addr, __be32 ip_addr)
{
	struct cls_wifi_br_uc *br_uc;

	/* IP address must be unique.  Remove any stale entry. */
	br_uc = cls_wifi_br_uc_find_by_ip(br, ip_addr);
	if (br_uc != NULL) {
		pr_debug("delete old entry mac=%pM ip=%pI4\n",
				br_uc->mac_addr, &ip_addr);
		cls_wifi_br_uc_delete(br, br_uc);
	} else {
		if (atomic_read(&br->uc_tot) >= CLS_WIFI_BR_ENT_MAX) {
			pr_err("unicast table is full, can't add %pM\n",
					mac_addr);
			return;
		}
	}

	br_uc = kmem_cache_alloc(br_uc_cache, GFP_ATOMIC);
	if (br_uc == NULL) {
		pr_err("failed to allocate unicast entry %pM\n",
				mac_addr);
		return;
	}

	memset(br_uc, 0, sizeof(*br_uc));

	atomic_inc(&br->uc_tot);
	ether_addr_copy(br_uc->mac_addr, mac_addr);
	br_uc->ip_addr = ip_addr;
	br_uc->timestamp = jiffies;
	hlist_add_head_rcu(&br_uc->mac_hlist, &br->uc_mac_hash[cls_wifi_br_mac_hash(mac_addr)]);

	hlist_add_head_rcu(&br_uc->ip_hlist, &br->uc_ip_hash[cls_wifi_br_ip_hash(ip_addr)]);
}

/*
 * Update a multicast entry from an upstream IGMP packet
 *
 * For an IGMP JOIN, create an entry for the multicast address if it is not
 * already present, then add the client to the multicast address entry.
 *
 * For an IGMP LEAVE, delete the client from the multicast entry.  If no clients
 * are left under the multicast entry, delete the multicast entry.  If other clients
 * remain under the multicast entry, notify the caller so that the LEAVE message
 * is dropped.
 *
 * Returns 1 if leaving and other clients are registered with the multicast address,
 * otherwise 0.
 */
static int cls_wifi_br_mc_update(struct cls_wifi_br *br, int op, __be32 mc_ip_addr, unsigned char *client_addr)
{
	struct cls_wifi_br_mc *br_mc;
	struct cls_wifi_br_mc_client *br_mc_client;
	int rc = 0;

	pr_debug("ip=%pI4 client=%pM op=%s\n",
			&mc_ip_addr, client_addr,
			(op == CLS_WIFI_IGMP_OP_JOIN) ? "join" : "leave");

	cls_wifi_br_lock_mc(br);

	br_mc = cls_wifi_br_mc_find(br, mc_ip_addr);
	if ((br_mc == NULL) &&
			(op == CLS_WIFI_IGMP_OP_JOIN)) {
		br_mc = cls_wifi_br_mc_add(br, mc_ip_addr);
	}
	if (br_mc == NULL) {
		/* Either malloc failed or leaving and there is no mc entry to leave */
		if (op == CLS_WIFI_IGMP_OP_LEAVE) {
			pr_debug("client=%pM leaving mc %pI4 but mc not in table\n",
					client_addr, &mc_ip_addr);
		}
		cls_wifi_br_unlock_mc(br);
		return 0;
	}

	br_mc_client = cls_wifi_br_mc_client_find(br_mc, client_addr);
	switch (op) {
		case CLS_WIFI_IGMP_OP_JOIN:
			if (br_mc_client == NULL) {
				cls_wifi_br_mc_client_add(br, br_mc, client_addr);
			}
			break;
		case CLS_WIFI_IGMP_OP_LEAVE:
			if (br_mc_client == NULL) {
				/* This can happen, for example, if the STA rebooted after the JOIN */
				pr_debug("client=%pM leaving mc %pI4 but not in table\n",
						client_addr, &mc_ip_addr);
				if (br_mc != NULL) {
					rc = 1;
				}
			} else {
				cls_wifi_br_mc_client_delete(br, br_mc, br_mc_client);
				if (atomic_read(&br_mc->mc_client_tot) < 1) {
					cls_wifi_br_mc_delete(br, br_mc);
				} else {
					rc = 1;
				}
			}
	}

	cls_wifi_br_unlock_mc(br);

	return rc;
}

/*
 * Create or update a unicast entry
 */
static void cls_wifi_br_uc_update(struct cls_wifi_br *br, __be32 ip_addr, unsigned char *mac_addr)
{
	struct cls_wifi_br_uc *br_uc;

	cls_wifi_br_uc_lock(br);

	br_uc = cls_wifi_br_uc_find_by_mac(br, mac_addr);
	if (br_uc == NULL) {
		cls_wifi_br_uc_add(br, mac_addr, ip_addr);
	} else {
		/* Update the entry if its IP address has changed */
		if (br_uc->ip_addr != ip_addr) {
			hlist_del_rcu(&br_uc->ip_hlist);
			br_uc->ip_addr = ip_addr;
			hlist_add_head_rcu(&br_uc->ip_hlist,
					&br->uc_ip_hash[cls_wifi_br_ip_hash(ip_addr)]);
			br_uc->timestamp = jiffies;
		}
	}

	/*TODO*/
	//fwt_sw_update_uc_ipmac(mac_addr, ip_addr);

	cls_wifi_br_uc_unlock(br);
}

/*
 * Update a unicast entry from an upstream ARP packet
 */
void cls_wifi_br_uc_update_from_arp(struct cls_wifi_br *br, struct ether_arp *arp)
{
	__be32 ip_addr;

	ip_addr = get_unaligned((u32 *)&arp->arp_spa);

	if (!cls_wifi_br_ip_is_unicast(ip_addr)) {
		return;
	}

	pr_debug("mac=%pM ip=%pI4\n",
			arp->arp_sha, &ip_addr);

	cls_wifi_br_uc_update(br, ip_addr, arp->arp_sha);
}

/*
 * Update a multicast entry from an upstream IGMP packet
 *
 * Returns 0 if OK, or 1 if the packet should be dropped.
 *
 * A client station in 3-address mode forwards multicast subscriptions to the
 * AP with source address set to the station's Wifi MAC address, so the AP sees
 * only one subscription even if when multiple clients subscribe.  This
 * function's sole purpose is to keep track of the downstream clients that
 * subscribe to a given multicast address, and to only forward a delete
 * request when the last client leaves.
 *
 * Note: A combination of join and leave requests in a single message (probably
 * never done in practice?) may not be handled correctly, since the decision to
 * forward or drop the message can only be made once.
 */
int cls_wifi_br_mc_update_from_igmp(struct cls_wifi_br *br, struct sk_buff *skb,
		struct ethhdr *eh, struct iphdr *iphdr_p)
{
	const struct igmphdr *igmp_p = (struct igmphdr *)
		((unsigned int *) iphdr_p + iphdr_p->ihl);
	const struct igmpv3_report *igmpv3_p = (struct igmpv3_report *)
		((unsigned int *) iphdr_p + iphdr_p->ihl);

	__be32 mc_ip_addr = 0;
	int num = -1;
	int n = 0;
	int op;
	int rc = 0;

	if ((skb->data + skb->len + 1) < (unsigned char *)(igmp_p + 1)) {
		pr_debug("IGMP packet is too small (%p/%p)\n",
				skb->data + skb->len, igmp_p + 1);
		return 0;
	}

	do {
		op = CLS_WIFI_IGMP_OP_NONE;

		switch(igmp_p->type) {
			case IGMP_HOST_MEMBERSHIP_REPORT:
				op = CLS_WIFI_IGMP_OP_JOIN;
				mc_ip_addr = get_unaligned((u32 *)&igmp_p->group);
				break;
			case IGMPV2_HOST_MEMBERSHIP_REPORT:
				op = CLS_WIFI_IGMP_OP_JOIN;
				mc_ip_addr = get_unaligned((u32 *)&igmp_p->group);
				break;
			case IGMP_HOST_LEAVE_MESSAGE:
				op = CLS_WIFI_IGMP_OP_LEAVE;
				mc_ip_addr = get_unaligned((u32 *)&igmp_p->group);
				break;
			case IGMPV3_HOST_MEMBERSHIP_REPORT:
				mc_ip_addr = get_unaligned((u32 *)&igmpv3_p->grec[n].grec_mca);
				if (num < 0) {
					num = ntohs(igmpv3_p->ngrec);
				}
				if ((igmpv3_p->grec[n].grec_type ==
							IGMPV3_CHANGE_TO_EXCLUDE) ||
						(igmpv3_p->grec[n].grec_type ==
						 IGMPV3_MODE_IS_EXCLUDE)) {
					op = CLS_WIFI_IGMP_OP_JOIN;
				} else if ((igmpv3_p->grec[n].grec_type ==
							IGMPV3_CHANGE_TO_INCLUDE) ||
						(igmpv3_p->grec[n].grec_type ==
						 IGMPV3_MODE_IS_INCLUDE)) {
					op = CLS_WIFI_IGMP_OP_LEAVE;
				}
				n++;
				break;
			default:
				break;
		}

		if (op > CLS_WIFI_IGMP_OP_NONE) {
			/* rc will be 1 if leaving and the multicast entry still has clients */
			rc = cls_wifi_br_mc_update(br, op, mc_ip_addr, eh->h_source);
		}
	} while (--num > 0);

	/* Last operation in packet determines whether it will be dropped */
	return rc;
}

/*
 * Update a unicast entry from an upstream DHCP packet
 */
void cls_wifi_br_uc_update_from_dhcp(struct cls_wifi_br *br, struct sk_buff *skb, struct iphdr *iphdr_p)
{
	struct udp_dhcp_packet *dhcpmsg = (struct udp_dhcp_packet *)iphdr_p;
	struct udphdr *uh = &dhcpmsg->udphdr_p;
	__be32 ip_addr;
	__wsum csum;

	/* Is this a DHCP packet? */
	if ((skb->len < ((u8 *)iphdr_p - skb->data) + sizeof(*dhcpmsg)) ||
			(uh->dest != __constant_htons(DHCPSERVER_PORT)) ||
			(dhcpmsg->dhcp_msg.op != BOOTREQUEST) ||
			(dhcpmsg->dhcp_msg.htype != ARPHRD_ETHER)) {
		return;
	}

	/*
	 * 3rd party APs may not forward unicast DHCP responses to us, so set the
	 * broadcast flag and recompute the UDP checksum.
	 */
	if (!(dhcpmsg->dhcp_msg.flags & __constant_htons(DHCP_BROADCAST_FLAG))) {

		dhcpmsg->dhcp_msg.flags |= __constant_htons(DHCP_BROADCAST_FLAG);

		/* Recalculate the UDP checksum */
		if (uh->check != 0) {
			uh->check = 0;
			csum = csum_partial(uh, ntohs(uh->len), 0);

			/* Add psuedo IP header checksum */
			uh->check = csum_tcpudp_magic(iphdr_p->saddr, iphdr_p->daddr,
					ntohs(uh->len), iphdr_p->protocol, csum);

			/* 0 is converted to -1 */
			if (uh->check == 0) {
				uh->check = CSUM_MANGLED_0;
			}
		}
	}

	/*
	 * Assume that any record containing a valid client IP address in the bootp structure
	 * is valid.  Ideally we should parse the DHCP structure that follows
	 * the BOOTP structure for message type 3, but this should suffice.
	 */
	ip_addr = get_unaligned((u32 *)&dhcpmsg->dhcp_msg.ciaddr);
	if (cls_wifi_br_ip_is_unicast(ip_addr)) {
		pr_debug("source=%d dest=%d op=%02x ip=%pI4 mac=%pM\n",
				dhcpmsg->udphdr_p.source, dhcpmsg->udphdr_p.dest,
				dhcpmsg->dhcp_msg.op, &ip_addr,
				dhcpmsg->dhcp_msg.chaddr);
		cls_wifi_br_uc_update(br, ip_addr, dhcpmsg->dhcp_msg.chaddr);
	}
}

/*
 * Replace the destination MAC address in a downstream packet
 *
 * For multicast IP (224.0.0.0 to 239.0.0.0), the MAC address may have been
 * changed to the station's unicast MAC address by the AP.  Convert it back to
 * an IPv4 Ethernet MAC address as per RFC 1112, section 6.4: place the
 * low-order 23-bits of the IP address into the low-order 23 bits of the
 * Ethernet multicast address 01-00-5E-00-00-00.

 * For unicast IP, use the cls_wifi bridge table.
 *
 * Returns 0 if OK, or 1 if the MAC was not updated.
 */
int cls_wifi_br_set_rx_dest_mac(struct cls_wifi_br *br, const struct sk_buff *skb)
{
	struct cls_wifi_br_uc *br_uc;
	struct iphdr *iphdr_p;
	struct ether_arp *arp_p = NULL;
	__be32 ip_addr = INADDR_ANY;
	unsigned char *ip_addr_p = (unsigned char *)&ip_addr;
	char mc_pref[] = {0x01, 0x00, 0x5e};
	struct ethhdr *eh = (struct ethhdr *)eth_hdr(skb);
#ifdef CONFIG_IPV6
	struct ipv6hdr *ip6hdr_p;
	struct in6_addr *ip6addr_p;
	char mc6_pref[] = {0x33, 0x33};
#endif
#if 0
	pr_err("handle rx callback\n");
	print_hex_dump(KERN_INFO, "skb_data: ", DUMP_PREFIX_OFFSET, 16, 1,
			eh, 32, true);
#endif

	if (eh->h_proto == __constant_htons(ETH_P_IP)) {
		iphdr_p = (struct iphdr *)(eh + 1);
		if ((skb->data + skb->len) < (unsigned char *)(iphdr_p + 1)) {
			pr_debug("IP packet is too small (%p/%p)\n",
					skb->data + skb->len, iphdr_p + 1);
			return 1;
		}
		ip_addr = get_unaligned((u32 *)&iphdr_p->daddr);
		pr_debug("ip proto=%u smac=%pM sip=%pI4 dip=%pI4\n",
				iphdr_p->protocol, eh->h_source,
				&iphdr_p->saddr, &ip_addr);

		if ((ip_addr_p[0] >> 4) == CLS_WIFI_IP_MCAST_PREF) {
			eh->h_dest[0] = mc_pref[0];
			eh->h_dest[1] = mc_pref[1];
			eh->h_dest[2] = mc_pref[2];
			eh->h_dest[3] = ip_addr_p[1] & 0x7F;
			eh->h_dest[4] = ip_addr_p[2];
			eh->h_dest[5] = ip_addr_p[3];
			return 0;
		}
#if defined(CONFIG_IPV6)
	} else if (eh->h_proto == __constant_htons(ETH_P_IPV6)) {
		ip6hdr_p = (struct ipv6hdr *)(eh + 1);
		if ((skb->data + skb->len) < (unsigned char *)(ip6hdr_p + 1)) {
			pr_debug("IP packet is too small (%p/%p)\n",
					skb->data + skb->len, ip6hdr_p + 1);
			return 1;
		}

		/*
		 * IPv6 address map to MAC address
		 * First two octets are the value 0x3333 and
		 * last four octets are the last four octets of ip.
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |0 0 1 1 0 0 1 1|0 0 1 1 0 0 1 1|
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |    IP6[12]    |    IP6[13]    |
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |    IP6[14]    |    IP6[15]    |
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */
		ip6addr_p = &(ip6hdr_p->daddr);
		if (ip6addr_p->s6_addr[0] == 0xFF) {
			eh->h_dest[0] = mc6_pref[0];
			eh->h_dest[1] = mc6_pref[1];
			eh->h_dest[2] = ip6addr_p->s6_addr[12];
			eh->h_dest[3] = ip6addr_p->s6_addr[13];
			eh->h_dest[4] = ip6addr_p->s6_addr[14];
			eh->h_dest[5] = ip6addr_p->s6_addr[15];
			return 0;
		}

		//return qdrv_br_ipv6_set_dest_mac(br, eh, ip6addr_p);
#endif
	} else if (eh->h_proto == __constant_htons(ETH_P_ARP)) {
#if 0
		pr_err("handle rx arp\n");
#endif
		arp_p = (struct ether_arp *)(eh + 1);
		if ((skb->data + skb->len + 1) < (unsigned char *)(arp_p + 1)) {
			pr_debug("ARP packet is too small (%p/%p)\n",
					skb->data + skb->len, arp_p + 1);
			return 1;
		}
		ip_addr = get_unaligned((u32 *)&arp_p->arp_tpa);
		pr_debug("ARP proto=%04x op=%04x sha=%pM tha=%pM "
				"sip=%pI4 dip=%pI4\n",
				arp_p->ea_hdr.ar_pro, arp_p->ea_hdr.ar_op,
				arp_p->arp_sha, arp_p->arp_tha,
				&arp_p->arp_spa, &ip_addr);
	} else {
		pr_debug("Ethertype 0x%04x not supported\n", ntohs(eh->h_proto));
		return 1;
	}

	if (!cls_wifi_br_ip_is_unicast(ip_addr)) {
		pr_debug("IP address is not unicast\n");
		return 1;
	}

	rcu_read_lock();

	br_uc = cls_wifi_br_uc_find_by_ip(br, ip_addr);
	if (br_uc == NULL) {
		pr_debug("IP address not found in bridge table\n");
		rcu_read_unlock();
		return 1;
	}

	if (arp_p) {
		pr_debug("ARP ip=%pI4 dhost=%pM tha=%pM -> %pM\n",
				&ip_addr, eh->h_dest, arp_p->arp_tha, br_uc->mac_addr);
		ether_addr_copy(arp_p->arp_tha, br_uc->mac_addr);
	} else {
		pr_debug("type=0x%04x ip=%pI4 dhost=%pM -> %pM\n",
				eh->h_proto, &ip_addr, eh->h_dest, br_uc->mac_addr);
	}
	ether_addr_copy(eh->h_dest, br_uc->mac_addr);

	rcu_read_unlock();

	return 0;
}

/*
 * Display all entries in a vap's bridge table
 */
void cls_wifi_br_show(struct cls_wifi_br *br)
{
	struct cls_wifi_br_uc *br_uc;
	struct cls_wifi_br_mc *br_mc;
	struct cls_wifi_br_mc_client *br_mc_client;
	int i;
	int j;

	printk("Client MAC          IP Address\n");

	rcu_read_lock();
	for (i = 0; i < CLS_WIFI_BR_MAC_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(br_uc,  &br->uc_mac_hash[i], mac_hlist) {
			printk("%pM %pI4\n",
					br_uc->mac_addr, &br_uc->ip_addr);
		}
	}
	rcu_read_unlock();

	printk("\n");
	printk("Multicast IP        Client MAC\n");

	rcu_read_lock();
	for (i = 0; i < CLS_WIFI_BR_IP_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(br_mc, &br->mc_ip_hash[i], mc_hlist) {

			printk("%pI4\n", &br_mc->mc_ip_addr);

			for (j = 0; j < CLS_WIFI_BR_MAC_HASH_SIZE; j++) {
				hlist_for_each_entry_rcu(br_mc_client,
						&br_mc->mc_client_hash[j],
						mc_client_hlist) {
					printk("                    %pM\n",
							br_mc_client->mac_addr);

				}
			}
		}
	}
	rcu_read_unlock();

	printk("\n");
}

/*
 * Clear the cls_wifi bridge table for a vap
 */
void cls_wifi_br_clear(struct cls_wifi_br *br)
{
	struct cls_wifi_br_uc *br_uc;
	struct cls_wifi_br_mc *br_mc;
	struct cls_wifi_br_mc_client *br_mc_client;
	struct hlist_node *h, *h1;
	int i;
	int j;

	cls_wifi_br_uc_lock(br);

	for (i = 0; i < CLS_WIFI_BR_MAC_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(br_uc, h, &br->uc_mac_hash[i], mac_hlist) {
			cls_wifi_br_uc_delete(br, br_uc);
		}
	}
	atomic_set(&br->uc_tot, 0);

	cls_wifi_br_uc_unlock(br);

	cls_wifi_br_lock_mc(br);

	for (i = 0; i < CLS_WIFI_BR_IP_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(br_mc, h, &br->mc_ip_hash[i], mc_hlist) {
			for (j = 0; j < CLS_WIFI_BR_MAC_HASH_SIZE; j++) {
				hlist_for_each_entry_safe(br_mc_client, h1,
						&br_mc->mc_client_hash[j], mc_client_hlist) {
					cls_wifi_br_mc_client_delete(br, br_mc, br_mc_client);
				}
			}
			cls_wifi_br_mc_delete(br, br_mc);
		}
	}
	atomic_set(&br->mc_tot, 0);

	cls_wifi_br_unlock_mc(br);
}

/*
 * Delete the bridge table for a vap
 */
void cls_wifi_br_delete(struct cls_wifi_br *br)
{
	cls_wifi_br_clear(br);
}

/*
 * Create the bridge table for a vap
 */
void cls_wifi_br_create(void)
{
#if 0
	spin_lock_init(&br->uc_lock);
	spin_lock_init(&br->mc_lock);

	/* Cache tables are global and are never deleted */
	if (br_uc_cache != NULL) {
		return;
	}
#endif

	br_uc_cache = kmem_cache_create("cls_wifi_br_uc_cache",
			sizeof(struct cls_wifi_br_uc),
			0, 0, NULL);
	br_mc_cache = kmem_cache_create("cls_wifi_br_mc_cache",
			sizeof(struct cls_wifi_br_mc),
			0, 0, NULL);
	br_mc_client_cache = kmem_cache_create("cls_wifi_br_mc_client_cache",
			sizeof(struct cls_wifi_br_mc_client),
			0, 0, NULL);
	pr_err("%px %px %px\n", br_uc_cache, br_mc_cache, br_mc_client_cache);
	BUG_ON(((br_uc_cache == NULL) || (br_mc_cache == NULL) ||
				(br_mc_client_cache == NULL)));
}

void cls_wifi_br_exit(void)
{
	kmem_cache_destroy(br_uc_cache);
	kmem_cache_destroy(br_mc_cache);
	kmem_cache_destroy(br_mc_client_cache);
}

int handle_skb_data_tx(struct cls_wifi_vif *vif, struct sk_buff *skb)
{
	bool is_bcast = 0, is_mcast = 0;
	uint16_t l2_type, l3_type;
	struct ethhdr *eh = (struct ethhdr *) skb->data;
	uint8_t *data_start = NULL;
	int value = skb_l4_proto(skb, &is_bcast, &is_mcast, &data_start);

	if (!skb || !skb->data || value == 0) {
#if 0
		pr_err("quit handle tx value:%d\n", value);
		print_hex_dump(KERN_INFO, "skb_data: ", DUMP_PREFIX_OFFSET, 16, 1,
				skb->data, 16, true);
		pr_err("\n");
#endif
		return -1;
	}

	l2_type = (value >> 16) & 0xFFFF;
	l3_type = value & 0xFFFF;
#if 0
	pr_err("l2_type:%02x, l3_type:%02x\n", l2_type , l3_type);
	pr_err("is_bcast:%d, is_mcast:%d\n", is_bcast, is_mcast);

	if (l2_type == ETH_P_IPV6) {
		print_hex_dump(KERN_INFO, "skb_data_tx: ", DUMP_PREFIX_OFFSET, 16, 1,
				skb->data, 48, true);
		pr_err("\n");
	}
#endif

	switch(l2_type) {
		case ETH_P_ARP:
			if (cls_wifi_tx_3addr_check_arp(vif, skb, data_start) != 0) {
				return NET_XMIT_DROP;
			}

			break;
#if 1
		case ETH_P_IP:
			if (cls_wifi_tx_3addr_check_ip(skb, vif, eh, data_start) != 0) {
				return NET_XMIT_DROP;
			}

			break;

		case ETH_P_IPV6:
#if 0
			pr_err("tx ipv6\n");
			pr_err("l2_type:%02x, l3_type:%02x\n", l2_type , l3_type);
			pr_err("is_bcast:%d, is_mcast:%d\n", is_bcast, is_mcast);
#endif
			if (cls_wifi_process_tx_ipv6_packet(skb, l3_type) != 0){
				return NET_XMIT_DROP;
			}
			break;

#endif
		default:
			return -1;
	}

	ether_addr_copy(eh->h_source, vif->ndev->dev_addr);

#if 0
	if (l2_type == ETH_P_IPV6)
		print_hex_dump(KERN_INFO, "after change skb_data: ", DUMP_PREFIX_OFFSET, 16, 1,
				eth_hdr(skb), 48, true);
#endif
	return 0;
}


#if 0
int check_src_mac_bounce(struct sk_buff *skb)
{
	return ether_addr_equal(eth_hdr(skb)->h_source,
			skb->dev->dev_addr);
}
#endif

int dump_skb_data_rx(struct sk_buff *skb, struct cls_wifi_vif *vif, char *info)
{
	bool is_bcast=0, is_mcast=0;
	int protocol = skb_l4_proto(skb, &is_bcast, &is_mcast, NULL);

	if (!skb || !skb->data || !skb->dev)
		return -1;

#if 0
	pr_err("dump_%s protocol:%x\n", info, protocol);
	if(check_src_mac_bounce(skb))
		return -2;
#endif

	//if (protocol != 0) {
	if (protocol == 0x86dd003a) {
		pr_err("%s:l4 type:%02x\n", info, protocol);
		pr_err("skb head:%px data:%px mac_header:%d\n", skb->head, skb->data, skb->mac_header);

		print_hex_dump(KERN_INFO, info, DUMP_PREFIX_OFFSET, 16, 1,
				eth_hdr(skb), 48, true);
	}
	return 0;
}



int dump_skb_data_tx(struct sk_buff *skb, struct cls_wifi_vif *vif, char *info)
{
	bool is_bcast=0, is_mcast=0;
	int protocol = skb_l4_proto(skb, &is_bcast, &is_mcast, NULL);
	if (!skb || !skb->data)
		return -1;

	if (protocol != 0) {
		pr_err("%s:l4 type:%02x\n", info, protocol);

		print_hex_dump(KERN_INFO, "skb_data: ", DUMP_PREFIX_OFFSET, 16, 1,
				skb->data, 16, true);
	}
	return 0;
}

int handle_skb_data_rx(struct cls_wifi_vif *vif, struct sk_buff *skb)
{
	int rc;

	if (skb->protocol == htons(ETH_P_IPV6) ) {
		//dump_skb_data_rx(skb, vif, "rx0");
		rc = cls_wifi_process_rx_ipv6_packet(skb);
		if (rc)
			pr_err("handle rx ipv6 err ret=%d\n", rc);
		//dump_skb_data_rx(skb, vif, "rx1");
	} else {
		rc = cls_wifi_rx_set_dest_mac(vif, skb);
	}

	//dump_skb_data_rx(skb, vif, "rx");

	return rc;
}

static void cls_wifi_br_module_exit(void)
{
	pr_info("clear rx/tx call back\n");
	cls_wifi_br_exit();
	ipv6_mac_table_exit();
	cls_3addr_br_rx_callback = NULL;
	cls_3addr_br_tx_callback = NULL;
}

static int cls_wifi_br_module_init(void)
{
	pr_info("set rx/tx call back\n");
	cls_wifi_br_create();
	ipv6_mac_table_init();
	cls_3addr_br_rx_callback = handle_skb_data_rx;
	cls_3addr_br_tx_callback = handle_skb_data_tx;
	return 0;
}

module_init(cls_wifi_br_module_init);
module_exit(cls_wifi_br_module_exit);
MODULE_LICENSE("GPL");

