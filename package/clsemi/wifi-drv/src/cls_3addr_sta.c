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

#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/jhash.h>
#include <net/sock.h>
#include "cls_wifi_defs.h"

struct arp_message {
	uint16_t hw_type;
	uint16_t pro_type;
	uint8_t hw_size;
	uint8_t pro_size;
	uint16_t opcode;
	uint8_t shost[ETH_ALEN];
	uint32_t sipaddr;
	uint8_t thost[ETH_ALEN];
	uint32_t tipaddr;
	uint8_t others[0];
}__attribute__ ((packed));

struct dhcp_message {
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
	u_int8_t sname[64];
	u_int8_t file[128];
	u_int32_t cookie;
	u_int8_t options[0];
}__attribute__ ((packed));

bool skb_is_multicast(struct sk_buff *skb)
{
	if (skb->protocol == htons(ETH_P_IP)) {
		return IN_MULTICAST(ntohl(ip_hdr(skb)->daddr));
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		return ipv6_addr_is_multicast(&ipv6_hdr(skb)->daddr);
	} else {
		return is_multicast_ether_addr(eth_hdr(skb)->h_dest);
	}
}

bool is_broadcast_packet(struct sk_buff *skb)
{
	int ret=0;
	ret = is_broadcast_ether_addr(eth_hdr(skb)->h_dest);
	return ret;
}

int skb_l4_proto(struct sk_buff *skb, bool *is_bcast, bool *is_mcast, uint8_t **l3_header)
{
	int protocol = 0;
	struct iphdr *ip = ip_hdr(skb);
	struct ethhdr *eh = (struct ethhdr *)eth_hdr(skb);
	uint16_t ether_type;
	uint8_t *data_start = cls_wifi_find_data_start(skb, eh, &ether_type);

	if (l3_header)
		*l3_header = data_start;

	if (is_broadcast_packet(skb)) {
		if(is_bcast)
			*is_bcast = 1;
	} else if (skb_is_multicast(skb)) {
		if(is_mcast)
			*is_mcast = 1;
	}

	if (skb->protocol == htons(ETH_P_ARP)) {
		struct ether_arp *arp = (void *)data_start;
		protocol |= ETH_P_ARP<<16;

		protocol |= htons(arp->ea_hdr.ar_op);

		return protocol;
	}

	if (skb->protocol == htons(ETH_P_IP)) {
		protocol |= ETH_P_IP << 16;
		protocol |= ip->protocol;

		return protocol;
	}

	if (skb->protocol == htons(ETH_P_IPV6)) {
		int nexthdr = ipv6_hdr(skb)->nexthdr;
		int offset = ETH_HLEN;
		int ret = 0;
		protocol |= ETH_P_IPV6 << 16;

		if (skb->data - (unsigned char *)eth_hdr(skb) > 0)
			offset = 0;

		ret = ipv6_find_hdr(skb, &offset, -1, NULL, NULL);
		if (ret < 0) {
			pr_err("skb head:%px data:%px mac_header:%d\n", skb->head, skb->data, skb->mac_header);
			print_hex_dump(KERN_INFO, "skb_l4_proto: ", DUMP_PREFIX_OFFSET, 16, 1,
					skb->data, 32, true);
			print_hex_dump(KERN_INFO, "raw skb_l4_proto: ", DUMP_PREFIX_OFFSET, 16, 1,
					skb->data - 14, 32, true);

			printk(KERN_ERR "Could not find ipv6 layer 4 header ret=%d\n", ret);
			return -1;
		}

		protocol |= nexthdr;

		return protocol;
	}

	return 0;
}

/*
 * This function returns:
 * 0 if DA remains untouched
 * 1 if DA is changed by the qdrv bridge
 */
int cls_wifi_rx_set_dest_mac(struct cls_wifi_vif *vif, const struct sk_buff *skb)
{
	bool is_bcast = 0, is_mcast = 0;

	skb_l4_proto((struct sk_buff *)skb, NULL, &is_mcast, NULL);
	if (is_mcast || is_bcast) {
		return 0;
	}
	if (vif->use_3addr_br) {
		return !cls_wifi_br_set_rx_dest_mac(&vif->bridge_table, skb);
	}

	return 0;
}

/*
 * Find the first IP address for a device.
 */
__be32 cls_wifi_dev_ipaddr_get(struct net_device *dev)
{
	struct in_device *in_dev;
	__be32 addr = 0;

	in_dev = in_dev_get(dev);
	if (!in_dev) {
		return 0;
	}

	rcu_read_lock();
	if (in_dev->ifa_list) {
		addr = in_dev->ifa_list->ifa_address;
	}
	rcu_read_unlock();

	in_dev_put(in_dev);

	return addr;
}

int cls_wifi_get_br_ipaddr(struct cls_wifi_vif *vif, __be32 *ipaddr)
{
	if (!vif->br_dev) {
		vif->br_dev = dev_get_by_name(&init_net, "br-lan");
	}

	if (vif->br_dev) {
		*ipaddr = cls_wifi_dev_ipaddr_get(vif->br_dev);
	} else {
		return -1;
	}

	return 0;
}

static int cls_wifi_is_bridge_ipaddr(struct cls_wifi_vif *vif, __be32 ipaddr) {
	__be32 br_ipaddr = 0;

	if (cls_wifi_get_br_ipaddr(vif, &br_ipaddr) >= 0) {
		if (br_ipaddr == ipaddr) {
			return 1;
		}
	}

	return 0;
}

/*
 * Special handling for ARP packets when in 3-address mode.
 * Returns 0 if OK, or 1 if the frame should be dropped.
 * The original skb may be copied and modified.
 */
int cls_wifi_tx_3addr_check_arp(struct cls_wifi_vif *vif, struct sk_buff *skb,
		uint8_t *data_start)
{
	struct ether_arp *arp = (struct ether_arp *)data_start;
	__be32 ipaddr = 0;

	if ((skb->len < (data_start - skb->data) + sizeof(*arp)) ||
			(!(arp->ea_hdr.ar_op == __constant_htons(ARPOP_REQUEST)) &&
			 !(arp->ea_hdr.ar_op == __constant_htons(ARPOP_REPLY)))) {
		return 0;
	}

	pr_debug(
			"ARP hrd=%04x pro=%04x ln=%02x/%02x op=%04x sha=%pM tha=%pM\n",
			arp->ea_hdr.ar_hrd, arp->ea_hdr.ar_pro, arp->ea_hdr.ar_hln,
			arp->ea_hdr.ar_pln, arp->ea_hdr.ar_op,
			arp->arp_sha, arp->arp_tha);
	pr_debug(
			"    sip=%pI4 tip=%pI4 new sha=%pM\n",
			&arp->arp_spa, &arp->arp_tpa, vif->ndev->dev_addr);

	if (vif->use_3addr_br) {
		/* update the cls_wifi bridge table if doing 3-address mode bridging */
		cls_wifi_br_uc_update_from_arp(&vif->bridge_table, arp);
	} else {
		/*
		 * In basic 3-address mode, don't send ARP requests for our own
		 * bridge IP address to the wireless network because the hack
		 * below will associate it with the wireless MAC, making the
		 * bridge IP address unreachable.  The bridge module will
		 * respond to the request.
		 */
		if (arp->ea_hdr.ar_op == __constant_htons(ARPOP_REQUEST)) {
			ipaddr = get_unaligned((uint32_t *)&arp->arp_tpa);
			if (cls_wifi_is_bridge_ipaddr(vif, ipaddr)) {
				pr_debug(
						"Not forwarding ARP request for bridge IP %pI4\n",
						&ipaddr);
				return 1;
			}
		}
	}

	/*
	 * ### Hack alert ###
	 * In 3-addr mode, the source host address in upstream ARP packets from
	 * the STA must be changed to the local wireless address.  Use a new
	 * skb to avoid modifying the bridge's copy of the frame.
	 */

	/* The offset of the arp structure in the new skb is the same as in the old skb */
	arp = (struct ether_arp *)(skb->data + ((unsigned char *)arp - skb->data));
	ether_addr_copy(&arp->arp_sha[0], vif->ndev->dev_addr);
	//pr_err("my mac:%pM\n", vif->ndev->dev_addr);

	return 0;
}


/*
 * Special handling for IP packets when in 3-address mode.
 * Returns 0 if OK, or 1 if the frame should be dropped.
 */
int cls_wifi_tx_3addr_check_ip(struct sk_buff *skb,
		struct cls_wifi_vif *vif, struct ethhdr *eh, uint8_t *data_start)
{
	struct iphdr *p_iphdr = (struct iphdr *)data_start;

	if (skb->len < (data_start - skb->data) + sizeof(*p_iphdr)) {
		return 0;
	}

	switch (p_iphdr->protocol) {
		case IPPROTO_UDP:
			cls_wifi_br_uc_update_from_dhcp(&vif->bridge_table, skb, p_iphdr);
			break;
		case IPPROTO_IGMP:
			if (cls_wifi_br_mc_update_from_igmp(&vif->bridge_table,
						skb, eh, p_iphdr) != 0) {
				pr_debug("Dropping IGMP packet - "
						"not last downstream client to unsubscribe\n");
				return 1;
			}

			break;
		default:
			break;
	}

	return 0;
}

/*
 * Replace Source MAC Address and BOOTP Client Address with Client Identifier
 */
static inline void cls_wifi_replace_dhcp_packets_header(struct sk_buff *skb)
{
	struct ethhdr *eh = (struct ethhdr *) skb->data;
	struct iphdr *iphdr_p = (struct iphdr *)(eh + 1);
	struct udphdr *uh = (struct udphdr*)(iphdr_p + 1);
	struct dhcp_message *dhcp_msg = (struct dhcp_message *)((u8 *)uh + sizeof(struct udphdr));

	u8 *frm;
	u8 *efrm;
	int chsum = 0;
	__wsum csum;


	if (dhcp_msg->op != BOOTREQUEST || dhcp_msg->htype != ARPHRD_ETHER)
		return;

	frm = (u8 *)(dhcp_msg->options);
	efrm = skb->data + skb->len;

	while (frm < efrm) {
		if (*frm == 0x3d && *(frm + 1) == 0x07 && *(frm + 2) == 0x01) {
			if (memcmp(dhcp_msg->chaddr, frm + 3, ETH_ALEN)) {
				memcpy(dhcp_msg->chaddr, frm + 3, ETH_ALEN);
			}
			if (memcmp(eh->h_source, frm + 3, ETH_ALEN)) {
				memcpy(eh->h_source, frm + 3, ETH_ALEN);
			}
			chsum = 1;
			break;
		}
		frm += *(frm+1) + 2;
	}

	/* Recalculate the UDP checksum */
	if (chsum && uh->check != 0) {
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

	return;
}

#if 0
#define CHECK_VENDOR	do {						\
	if (!vendor_checked) {					\
		vendor = cls_wifi_find_src_vendor(vif, skb);		\
		vendor_checked = 1;				\
	}							\
} while (0)
#endif
