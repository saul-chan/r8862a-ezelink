/*
 * Motorcomm DSA Tag support
 * Copyright (c), 2022, Motorcomm Electronic Technology Co.,Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/bitfield.h>
#include <linux/etherdevice.h>
#include "dsa_priv.h"
#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif

#define ETH_P_MOTORCOMM             0x9988
#define YT_HDR_LEN                  8
#define MOTORCOMM_PKT_TYPE          GENMASK(15, 14)
#define MOTORCOMM_PKT_FROM_CPU      0x1
#define MOTORCOMM_SRC_PORT          GENMASK(14, 11)
#define MOTORCOMM_DST_PORTMASK      GENMASK(10, 0)
#define MOTORCOMM_FORCE_DST         BIT(15)
#define MOTORCOMM_FORCE_DST_EN      0x1

#if ((KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 20, 0) > LINUX_VERSION_CODE)  || (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE))
static struct sk_buff *yt_tag_xmit(struct sk_buff *skb,
                    struct net_device *dev)
{
    u8 *tag;
    struct dsa_port *dp = dsa_slave_to_port(dev);
    __be16 tag16[YT_HDR_LEN / 2];

    if (skb_cow_head(skb, YT_HDR_LEN) < 0)
        return NULL;

    skb_push(skb, YT_HDR_LEN);
    memmove(skb->data, skb->data + YT_HDR_LEN, 2 * ETH_ALEN);

    tag = skb->data + 2 * ETH_ALEN;

    /* Set Motorcomm EtherType */
    tag16[0] = htons(ETH_P_MOTORCOMM);

    /* Set Type */
    tag16[1] = htons(FIELD_PREP(MOTORCOMM_PKT_TYPE, MOTORCOMM_PKT_FROM_CPU));

    /* Zero */
    tag16[2] = 0x0;

    /* set RX (CPU->switch) forwarding port mask */
    tag16[3] = htons(FIELD_PREP(MOTORCOMM_FORCE_DST, MOTORCOMM_FORCE_DST_EN) | FIELD_PREP(MOTORCOMM_DST_PORTMASK, BIT(dp->index)));

    memcpy(tag, tag16, YT_HDR_LEN);

    return skb;
}

static struct sk_buff *yt_tag_rcv(struct sk_buff *skb, struct net_device *dev,
                   struct packet_type *pt)
{
    u8 *tag;
    __be16 tag16[YT_HDR_LEN / 2];
    u16 etype;
    u8 port;

    if (unlikely(!pskb_may_pull(skb, YT_HDR_LEN)))
        return NULL;

    /* The YT header is added by the switch between src addr
     * and ethertype at this point, skb->data points to 2 bytes
     * after src addr so header should be 2 bytes right before.
     */
    tag = (void *)skb->data - 2;
    memcpy(tag16, tag, YT_HDR_LEN);

    /* Parse Realtek EtherType */
    etype = ntohs(tag16[0]);
    if (unlikely(etype != ETH_P_MOTORCOMM)) {
        dev_warn_ratelimited(&dev->dev,
                     "invalid ethertype 0x%04x\n", etype);
        return NULL;
    }

    /* Parse Source Port (switch->CPU) */
    port = FIELD_GET(MOTORCOMM_SRC_PORT, ntohs(tag16[2]));
    skb->dev = dsa_master_find_slave(dev, 0, port);
    if (!skb->dev) {
        dev_warn_ratelimited(&dev->dev,
                     "could not find slave for port %d\n",
                     port);

        return NULL;
    }

    /* Remove tag and recalculate checksum */
    skb_pull_rcsum(skb, YT_HDR_LEN);

    memmove(skb->data - ETH_HLEN,
        skb->data - ETH_HLEN - YT_HDR_LEN,
        2 * ETH_ALEN);

    return skb;
}
#elif (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
static struct sk_buff *yt_tag_xmit(struct sk_buff *skb,
                    struct net_device *dev)
{
    u8 *tag;
    struct dsa_port *dp = dsa_slave_to_port(dev);
    __be16 tag16[YT_HDR_LEN / 2];

    if (skb_cow_head(skb, YT_HDR_LEN) < 0)
        return NULL;

    skb_push(skb, YT_HDR_LEN);
    memmove(skb->data, skb->data + YT_HDR_LEN, 2 * ETH_ALEN);

    tag = skb->data + 2 * ETH_ALEN;

    /* Set Motorcomm EtherType */
    tag16[0] = htons(ETH_P_MOTORCOMM);

    /* Set Type */
    tag16[1] = htons(FIELD_PREP(MOTORCOMM_PKT_TYPE, MOTORCOMM_PKT_FROM_CPU));

    /* Zero */
    tag16[2] = 0x0;

    /* set RX (CPU->switch) forwarding port mask */
    tag16[3] = htons(FIELD_PREP(MOTORCOMM_FORCE_DST, MOTORCOMM_FORCE_DST_EN) | FIELD_PREP(MOTORCOMM_DST_PORTMASK, BIT(dp->index)));

    memcpy(tag, tag16, YT_HDR_LEN);

    return skb;
}

static struct int *yt_tag_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt)
{
    u8 *tag;
    __be16 tag16[YT_HDR_LEN / 2];
    u16 etype;
    u8 port;

    if (unlikely(!pskb_may_pull(skb, YT_HDR_LEN)))
        return NULL;

    /* The YT header is added by the switch between src addr
     * and ethertype at this point, skb->data points to 2 bytes
     * after src addr so header should be 2 bytes right before.
     */
    tag = (void *)skb->data - 2;
    memcpy(tag16, tag, YT_HDR_LEN);

    /* Parse Realtek EtherType */
    etype = ntohs(tag16[0]);
    if (unlikely(etype != ETH_P_MOTORCOMM)) {
        dev_warn_ratelimited(&dev->dev,
                     "invalid ethertype 0x%04x\n", etype);
        return NULL;
    }

    /* Parse Source Port (switch->CPU) */
    port = FIELD_GET(MOTORCOMM_SRC_PORT, ntohs(tag16[2]));

    /* Remove tag and recalculate checksum */
    skb_pull_rcsum(skb, YT_HDR_LEN);

    /* Move the Ethernet DA and SA */
    memmove(skb->data - ETH_HLEN,
        skb->data - ETH_HLEN - YT_HDR_LEN,
        2 * ETH_ALEN);

    skb_push(skb, ETH_HLEN);
    skb->pkt_type = PACKET_HOST;
    skb->dev = ds->ports[port].netdev;
    skb->protocol = eth_type_trans(skb, skb->dev);

    skb->dev->stats.rx_packets++;
    skb->dev->stats.rx_bytes += skb->len;

    netif_receive_skb(skb);
    return 0;
}

#endif // 4.90 kernel


#if (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 20, 0) > LINUX_VERSION_CODE)
const struct dsa_device_ops yt_netdev_ops = {
    .xmit       = yt_tag_xmit,
    .rcv        = yt_tag_rcv,
};
#elif (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE)
static const struct dsa_device_ops yt_netdev_ops = {
	.name	= "motorcomm",
	.proto	= DSA_TAG_PROTO_MOTORCOMM,
	.xmit	= yt_tag_xmit,
	.rcv	= yt_tag_rcv,
	.overhead = YT_HDR_LEN,
};

//DSA_TAG_DRIVER(yt_netdev_ops);
module_dsa_tag_driver(yt_netdev_ops);
MODULE_ALIAS_DSA_TAG_DRIVER(DSA_TAG_PROTO_MOTORCOMM);
#elif (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
const struct dsa_device_ops yt_netdev_ops = {
    .xmit       = yt_tag_xmit,
    .rcv        = yt_tag_rcv,
};
#endif
