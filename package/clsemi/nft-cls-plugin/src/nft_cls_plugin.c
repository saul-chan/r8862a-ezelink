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
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_tables_core.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nft_meta.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/ip.h>
#include <net/tcp.h>

#define CLSMARK_BIT_FOR_QOS_BR_RULE0_IDX	48
#define CLSMARK_BIT_FOR_QOS_BR_RULE0_DIR	56

enum nft_meta_cls_plugin_keys {
	NFT_META_URL = NFT_META_CLS_PLUGIN_BASE,
	NFT_META_DNS,
	NFT_META_CLSMARK,
	NFT_META_OFLD_QUEUE,
	NFT_META_OFLD_PCP,
	NFT_META_OFLD_DSCP,
	NFT_META_QOS_BR_RULE0,
	NFT_META_QOS_BR_RULE1,
	NFT_META_QOS_BR_RULE2,
	NFT_META_QOS_BR_RULE3,
	NFT_META_QOS_BR_RULE4,
	NFT_META_QOS_BR_RULE5,
	NFT_META_QOS_BR_RULE6,
	NFT_META_QOS_BR_RULE7,
};

/* according rfc5246 and wireshark */
struct tls_client_hello {
	__u8	content_type;
	__u8	major_version;
	__u8	minor_version;
	__be16	payload_len;
	__u8	handshake_type;
	__u8	handshake_len[3];
	__be16	client_version;
	__u8	random[32];
	__be16	sid_len;
} __packed;

/* according rfc6066 and wireshark */
struct ext_srv_name {
	__u8	ext_type[2];
	__u8	ext_len[2];
	__be16	name_list_len;
	__u8	name_type;
	__be16	name_len;
	__u8	name[];
} __packed;

#ifdef DEBUG
#define DPRINTK(fmt, args...) pr_info(fmt, ##args)
#else
#define DPRINTK(fmt, args...)
#endif

#define DNS_QTYPE_A	0x0001
#define DNS_QTYPE_AAAA	0x001c
#define DNS_QCLASS_IN	0x0001

/* according rfc1035 and wireshark */
struct dnshdr {
	uint16_t id;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint16_t rd : 1,
		 tc : 1,
		 aa : 1,
		 op : 4,
		 qr : 1,
		 rc : 4,
		 cd : 1,
		 ad : 1,
		 z  : 1,
		 ra : 1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	uint16_t qr : 1,
		 op : 4,
		 aa : 1,
		 tc : 1,
		 rd : 1,
		 ra : 1,
		 z  : 1,
		 ad : 1,
		 cd : 1,
		 rc : 4;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
};

static int https_extract_url(u32 *dest, struct sk_buff *skb, int offset, int len)
{
	char *p;
	int l;
	struct tls_client_hello *tls;
	struct ext_srv_name *ext;
	int16_t name_len;

	if (len < sizeof(struct tls_client_hello) + sizeof(struct ext_srv_name))
		return 0;

	if (!pskb_may_pull(skb, offset + sizeof(struct tls_client_hello)))
		return 0;

	tls = (struct tls_client_hello *)(skb->data + offset);
	if (tls->content_type	!= 0x16 ||
	    tls->major_version	!= 0x03 ||
	    tls->minor_version	!= 0x01 ||
	    tls->handshake_type	!= 0x01)
		return 0;

	if (!pskb_may_pull(skb, offset + len))
		return 0;

	p = skb->data + offset + sizeof(struct tls_client_hello);
	l = len - sizeof(struct tls_client_hello);
	for (; l >= sizeof(struct ext_srv_name); p++, l--) {
		ext = (struct ext_srv_name *)p;
		if (ext->ext_type[0] == 0 &&
		    ext->ext_type[1] == 0 &&
		    ext->ext_len[0] == 0 &&
		    ext->name_type == 0) {
			memcpy(&name_len, &ext->name_len, sizeof(name_len));
			name_len = ntohs(name_len);
			if (name_len < 1 || name_len + sizeof(struct ext_srv_name) > l)
				continue;
			name_len = name_len > URL_SIZE ? URL_SIZE : name_len;
			memcpy(dest, ext->name, name_len);
			return name_len;
		}
	}

	return 0;
}

/* according rfc2616 */
static const struct {
	char *method;
	int len;
} http_req_methods[] = {
	{"GET ",	4},
	{"PUT ",	4},
	{"POST ",	5},
	{"HEAD ",	5},
	{"PATCH ",	6},
	{"TRACE ",	6},
	{"DELETE ",	7},
	{"CONNECT ",	8},
	{"OPTIONS ",	8},
};

static int http_extract_url(u32 *dest, struct sk_buff *skb, int offset, int len)
{
#define HTTP_MIN_LEN		25	/* strlen("GET / HTTP/1.1\r\nHost: x\r\n") */
#define HTTP_METHOD_MAX_LEN	8	/* see http_req_methods */
#define HOST_FIELD_MIN_LEN	11	/* strlen("Host: x\r\n\r\n")*/
#define HOST_FIELD_HDR_LEN	6	/* strlen("Host: ") */
	char *p, *host;
	int i, l, host_len;
	bool is_http_req = false;

	if (len < HTTP_MIN_LEN)
		return 0;

	if (!pskb_may_pull(skb, offset + HTTP_METHOD_MAX_LEN))
		return 0;

	p = skb->data + offset;
	for (i = 0; i < ARRAY_SIZE(http_req_methods); ++i) {
		if (strncmp(p, http_req_methods[i].method, http_req_methods[i].len) == 0) {
			is_http_req = true;
			break;
		}
	}

	if (!is_http_req)
		return 0;

	if (!pskb_may_pull(skb, offset + len))
		return 0;

	p = skb->data + offset + http_req_methods[i].len;
	l = len - http_req_methods[i].len;
	for (; l > HOST_FIELD_MIN_LEN; p++, l--) {
		if (p[0] != '\r' || p[1] != '\n')
			continue;
		p += 2;
		l -= 2;
		if (strncmp(p, "Host: ", HOST_FIELD_HDR_LEN) == 0) {
			p += HOST_FIELD_HDR_LEN;
			l -= HOST_FIELD_HDR_LEN;
			host = p;
			for (; l > 0; p++, l--) {
				if (*p == '\r')
					break;
			}
			if (l == 0)
				break;
			host_len = p - host > URL_SIZE ? URL_SIZE : p - host;
			memcpy(dest, host, host_len);
			return host_len;
		} else if (p[0] == '\r' && p[1] == '\n') {
			break;
		}
	}

	return 0;
}

static int ipv4_extract_url(u32 *dest, struct sk_buff *skb)
{
	const struct iphdr *iph = ip_hdr(skb);
	const struct tcphdr *tcph;
	unsigned int iph_len = iph->ihl * 4;
	int offset, len;
	int (*func)(u32 *dest, struct sk_buff *skb, int offset, int len);

	if (ip_is_fragment(iph) || unlikely(iph_len != sizeof(struct iphdr)))
		return 0;

	if (iph->protocol != IPPROTO_TCP)
		return 0;

	offset = skb_network_offset(skb) + iph_len + sizeof(*tcph);
	if (!pskb_may_pull(skb, offset))
		return 0;

	tcph = (struct tcphdr *)((uint8_t *)iph + iph_len);
	if (tcph->dest == htons(80))
		func = http_extract_url;
	else if (tcph->dest == htons(443))
		func = https_extract_url;
	else
		return 0;

	offset = skb_network_offset(skb) + iph_len + tcph->doff * 4;
	len = ntohs(iph->tot_len) - iph_len - tcph->doff * 4;
	return func(dest, skb, offset, len);
}

static int ipv6_extract_url(u32 *dest, struct sk_buff *skb)
{
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);
	const struct tcphdr *tcph;
	int offset, len;
	int (*func)(u32 *dest, struct sk_buff *skb, int offset, int len);

	if (ip6h->nexthdr != IPPROTO_TCP)
		return 0;

	offset = skb_network_offset(skb) + sizeof(*ip6h) + sizeof(*tcph);
	if (!pskb_may_pull(skb, offset))
		return 0;

	tcph = (struct tcphdr *)((uint8_t *)ip6h + sizeof(*ip6h));
	if (tcph->dest == htons(80))
		func = http_extract_url;
	else if (tcph->dest == htons(443))
		func = https_extract_url;
	else
		return 0;

	offset = skb_network_offset(skb) + sizeof(*ip6h) + tcph->doff * 4;
	len = ntohs(ip6h->payload_len) - tcph->doff * 4;
	return func(dest, skb, offset, len);
}

static int nft_meta_get_eval_url(u32 *dest, struct sk_buff *skb)
{
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		return ipv4_extract_url(dest, skb);
	case htons(ETH_P_IPV6):
		return ipv6_extract_url(dest, skb);
	}

	return 0;
}

static int extract_dns(u32 *dest, struct sk_buff *skb, int offset, int len)
{
#define LABEL_NAME_MAX_SIZE	63
	struct dnshdr *dnsh;
	char _qname[DNS_SIZE] = {0};
	char *qname = _qname;
	uint16_t qlen = 0;
	uint8_t llen;
	uint16_t qtype, qclass;

	if (len < sizeof(*dnsh) + 6)
		return 0;

	if (!pskb_may_pull(skb, offset + sizeof(*dnsh)))
		return 0;

	dnsh = (struct dnshdr *)(skb->data + offset);
	if (dnsh->qr != 0) {
		DPRINTK("ignore dns pkt due to not dns query\n");
		return 0;
	}

	if (dnsh->qdcount != htons(1)) {
		DPRINTK("ignore due to qdcount not equal to 1: %d\n", ntohs(dnsh->qdcount));
		return 0;
	}

	offset += sizeof(*dnsh);

	while (qlen < DNS_SIZE) {
		if (skb_copy_bits(skb, offset, &llen, 1) < 0)
			return 0;
		if (llen > LABEL_NAME_MAX_SIZE || llen > DNS_SIZE) {
			DPRINTK("ignore due to label len overflow: %u\n", llen);
			return 0;
		}
		offset += 1;
		if (llen == 0) {
			break;
		} else if (qlen != 0) {
			qname[qlen] = '.';
			qlen += 1;
		}
		if (qlen + llen > DNS_SIZE) {
			DPRINTK("ignore due to dns name > DNS_SIZE\n");
			return 0;
		}
		if (skb_copy_bits(skb, offset, qname + qlen, llen) < 0)
			return 0;
		offset += llen;
		qlen += llen;
	}

	if (skb_copy_bits(skb, offset, &qtype, 2) < 0)
		return 0;
	if (qtype != htons(DNS_QTYPE_A) && qtype != htons(DNS_QTYPE_AAAA)) {
		DPRINTK("ignore due to unsupported qtype\n");
		return 0;
	}

	offset += 2;
	if (skb_copy_bits(skb, offset, &qclass, 2) < 0)
		return 0;
	if (qclass != htons(DNS_QCLASS_IN)) {
		DPRINTK("ignore due to unsupported qclass\n");
		return 0;
	}

	memcpy(dest, qname, qlen);

	return qlen;
}

static int ipv4_extract_dns(u32 *dest, struct sk_buff *skb)
{
	const struct iphdr *iph = ip_hdr(skb);
	const struct udphdr *udph;
	const struct tcphdr *tcph;
	unsigned int iph_len = iph->ihl * 4;
	int offset, len;

	if (ip_is_fragment(iph) || unlikely(iph_len != sizeof(struct iphdr)))
		return 0;

	if (iph->protocol == IPPROTO_UDP) {
		offset = skb_network_offset(skb) + iph_len + sizeof(*udph);
		if (!pskb_may_pull(skb, offset))
			return 0;

		udph = (struct udphdr *)((uint8_t *)iph + iph_len);
		if (udph->dest == htons(53)) {
			len = ntohs(iph->tot_len) - iph_len - sizeof(*udph);
			return extract_dns(dest, skb, offset, len);
		}
	} else if (iph->protocol == IPPROTO_TCP) {
		offset = skb_network_offset(skb) + iph_len + sizeof(*tcph);
		if (!pskb_may_pull(skb, offset))
			return 0;

		tcph = (struct tcphdr *)((uint8_t *)iph + iph_len);
		if (tcph->dest == htons(53)) {
			offset = skb_network_offset(skb) + iph_len + tcph->doff * 4;
			len = ntohs(iph->tot_len) - iph_len - tcph->doff * 4;
			return extract_dns(dest, skb, offset + 2, len - 2);
		}
	}

	return 0;
}

static int ipv6_extract_dns(u32 *dest, struct sk_buff *skb)
{
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);
	const struct udphdr *udph;
	const struct tcphdr *tcph;
	int offset, len;

	if (ip6h->nexthdr == IPPROTO_UDP) {
		offset = skb_network_offset(skb) + sizeof(*ip6h) + sizeof(*udph);
		if (!pskb_may_pull(skb, offset))
			return 0;

		udph = (struct udphdr *)((uint8_t *)ip6h + sizeof(*ip6h));
		if (udph->dest == htons(53)) {
			len = ntohs(ip6h->payload_len) - sizeof(*udph);
			return extract_dns(dest, skb, offset, len);
		}
	} else if (ip6h->nexthdr == IPPROTO_TCP) {
		offset = skb_network_offset(skb) + sizeof(*ip6h) + sizeof(*tcph);
		if (!pskb_may_pull(skb, offset))
			return 0;

		tcph = (struct tcphdr *)((uint8_t *)ip6h + sizeof(*ip6h));
		if (tcph->dest == htons(53)) {
			offset = skb_network_offset(skb) + sizeof(*ip6h) + tcph->doff * 4;
			len = ntohs(ip6h->payload_len) - tcph->doff * 4;
			return extract_dns(dest, skb, offset + 2, len - 2);
		}
	}

	return 0;
}

static int nft_meta_get_eval_dns(u32 *dest, struct sk_buff *skb)
{
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		return ipv4_extract_dns(dest, skb);
	case htons(ETH_P_IPV6):
		return ipv6_extract_dns(dest, skb);
	}

	return 0;
}

static int br_chain_get_nf_ct(const struct nft_ctx *ctx)
{
	int ret = -EOPNOTSUPP;
	struct nft_base_chain *basechain = nft_base_chain(ctx->chain);

	if (nft_is_base_chain(ctx->chain)) {
		switch (basechain->ops.hooknum) {
		case NF_BR_PRE_ROUTING:
			if (basechain->ops.priority <= NF_IP_PRI_CONNTRACK)
				ret = -EOPNOTSUPP;
			else
				ret = nf_ct_netns_get(ctx->net, NFPROTO_BRIDGE);
			break;
		case NF_BR_LOCAL_IN:
		case NF_BR_FORWARD:
		case NF_BR_POST_ROUTING:
			ret = nf_ct_netns_get(ctx->net, NFPROTO_BRIDGE);
			break;
		case NF_BR_LOCAL_OUT:
			ret = 1;
			break;
		}
	}

	return ret;
}

static void br_chain_put_nf_ct(const struct nft_ctx *ctx)
{
	struct nft_base_chain *basechain = nft_base_chain(ctx->chain);

	if (nft_is_base_chain(ctx->chain)) {
		switch (basechain->ops.hooknum) {
		case NF_BR_PRE_ROUTING:
		case NF_BR_LOCAL_IN:
		case NF_BR_FORWARD:
		case NF_BR_POST_ROUTING:
			nf_ct_netns_put(ctx->net, NFPROTO_BRIDGE);
			break;
		}
	}
}

static int nft_meta_get_eval_clsmark(u32 *dest, struct sk_buff *skb)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	memcpy(dest, &ct->clsmark, sizeof(u64));

	return 1;
}

static int nft_meta_get_eval_qos_br_rule(u32 *dest, struct sk_buff *skb, int br_rule_id)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	u64 mask, mark;
	u64 idx = BIT(CLSMARK_BIT_FOR_QOS_BR_RULE0_IDX + br_rule_id);
	u64 dir = BIT(CLSMARK_BIT_FOR_QOS_BR_RULE0_DIR + br_rule_id);
	u8 val = 0;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	mask = idx | dir;

	mark = idx;
	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL)
		mark |= dir;

	if ((ct->clsmark & mask) == mark)
		val = 1;

	nft_reg_store8(dest, val);

	return 1;
}

static int nft_meta_set_eval_qos_br_rule(struct sk_buff *skb, u8 val, int br_rule_id)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	bool changed = false;
	u64 mask, mark;
	u64 idx = BIT(CLSMARK_BIT_FOR_QOS_BR_RULE0_IDX + br_rule_id);
	u64 dir = BIT(CLSMARK_BIT_FOR_QOS_BR_RULE0_DIR + br_rule_id);

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	mask = idx | dir;

	if (val) {
		mark = idx;
		if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL)
			mark |= dir;
	} else
		mark = 0;

	if ((ct->clsmark & mask) != mark) {
		ct->clsmark = (ct->clsmark & ~mask) | mark;
		changed = true;
	}

	if (changed)
		nf_conntrack_event_cache(IPCT_CLSMARK, ct);

	return 1;
}

static int nft_meta_cls_plugin_get_eval(u32 *dest, int key, const struct nft_pktinfo *pkt)
{
	struct sk_buff *skb = pkt->skb;

	switch (key) {
	case NFT_META_URL:
		return nft_meta_get_eval_url(dest, skb);
	case NFT_META_DNS:
		return nft_meta_get_eval_dns(dest, skb);
	case NFT_META_CLSMARK:
		return nft_meta_get_eval_clsmark(dest, skb);
	case NFT_META_QOS_BR_RULE0:
	case NFT_META_QOS_BR_RULE1:
	case NFT_META_QOS_BR_RULE2:
	case NFT_META_QOS_BR_RULE3:
	case NFT_META_QOS_BR_RULE4:
	case NFT_META_QOS_BR_RULE5:
	case NFT_META_QOS_BR_RULE6:
	case NFT_META_QOS_BR_RULE7:
		return nft_meta_get_eval_qos_br_rule(dest, skb, key - NFT_META_QOS_BR_RULE0);
	}

	return -EOPNOTSUPP;
}

static int nft_meta_cls_plugin_get_init(const struct nft_ctx *ctx, unsigned int *len, int key)
{
	int ret = -EOPNOTSUPP;
	bool depend_ct = false;

	switch (key) {
	case NFT_META_URL:
		*len = URL_SIZE;
		depend_ct = true;
		ret = 1;
		break;
	case NFT_META_DNS:
		*len = DNS_SIZE;
		depend_ct = true;
		ret = 1;
		break;
	case NFT_META_CLSMARK:
		*len = sizeof(u64);
		depend_ct = true;
		ret = 1;
		break;
	case NFT_META_QOS_BR_RULE0:
	case NFT_META_QOS_BR_RULE1:
	case NFT_META_QOS_BR_RULE2:
	case NFT_META_QOS_BR_RULE3:
	case NFT_META_QOS_BR_RULE4:
	case NFT_META_QOS_BR_RULE5:
	case NFT_META_QOS_BR_RULE6:
	case NFT_META_QOS_BR_RULE7:
		*len = sizeof(u8);
		depend_ct = true;
		ret = 1;
		break;
	}

	if (depend_ct && ctx->family == NFPROTO_BRIDGE)
		ret = br_chain_get_nf_ct(ctx);

	return ret;
}

static void nft_meta_cls_plugin_get_destroy(const struct nft_ctx *ctx, int key)
{
	switch (key) {
	case NFT_META_CLSMARK:
	case NFT_META_OFLD_QUEUE:
	case NFT_META_OFLD_PCP:
	case NFT_META_OFLD_DSCP:
	case NFT_META_QOS_BR_RULE0:
	case NFT_META_QOS_BR_RULE1:
	case NFT_META_QOS_BR_RULE2:
	case NFT_META_QOS_BR_RULE3:
	case NFT_META_QOS_BR_RULE4:
	case NFT_META_QOS_BR_RULE5:
	case NFT_META_QOS_BR_RULE6:
	case NFT_META_QOS_BR_RULE7:
		if (ctx->family == NFPROTO_BRIDGE)
			br_chain_put_nf_ct(ctx);
		break;
	}
}

static int nft_meta_set_eval_clsmark(struct sk_buff *skb, u32 *sreg)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	if (!memcmp(&ct->clsmark, sreg, sizeof(u64)))
		return 0;

	memcpy(&ct->clsmark, sreg, sizeof(u64));
	nf_conntrack_event_cache(IPCT_CLSMARK, ct);

	return 1;
}

static int nft_meta_set_eval_ofld_queue(struct sk_buff *skb, u8 val)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	bool changed = false;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL) {
		if (ct->cls_ori_set_queue != 1) {
			ct->cls_ori_set_queue = 1;
			changed = true;
		}
		if (ct->cls_ori_queue != val) {
			ct->cls_ori_queue = val;
			changed = true;
		}
	} else {
		if (ct->cls_rep_set_queue != 1) {
			ct->cls_rep_set_queue = 1;
			changed = true;
		}
		if (ct->cls_rep_queue != val) {
			ct->cls_rep_queue = val;
			changed = true;
		}
	}

	if (changed)
		nf_conntrack_event_cache(IPCT_CLSMARK, ct);

	return 1;
}

static int nft_meta_set_eval_ofld_pcp(struct sk_buff *skb, u8 val)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	bool changed = false;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL) {
		if (ct->cls_ori_set_pcp != 1) {
			ct->cls_ori_set_pcp = 1;
			changed = true;
		}
		if (ct->cls_ori_pcp != val) {
			ct->cls_ori_pcp = val;
			changed = true;
		}
	} else {
		if (ct->cls_rep_set_pcp != 1) {
			ct->cls_rep_set_pcp = 1;
			changed = true;
		}
		if (ct->cls_rep_pcp != val) {
			ct->cls_rep_pcp = val;
			changed = true;
		}
	}

	if (changed)
		nf_conntrack_event_cache(IPCT_CLSMARK, ct);

	return 1;
}

static int nft_meta_set_eval_ofld_dscp(struct sk_buff *skb, u8 val)
{
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	bool changed = false;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return 0;

	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL) {
		if (ct->cls_ori_set_dscp != 1) {
			ct->cls_ori_set_dscp = 1;
			changed = true;
		}
		if (ct->cls_ori_dscp != val) {
			ct->cls_ori_dscp = val;
			changed = true;
		}
	} else {
		if (ct->cls_rep_set_dscp != 1) {
			ct->cls_rep_set_dscp = 1;
			changed = true;
		}
		if (ct->cls_rep_dscp != val) {
			ct->cls_rep_dscp = val;
			changed = true;
		}
	}

	if (changed)
		nf_conntrack_event_cache(IPCT_CLSMARK, ct);

	return 1;
}

static int nft_meta_cls_plugin_set_eval(u32 *sreg, int key, const struct nft_pktinfo *pkt)
{
	struct sk_buff *skb = pkt->skb;

	switch (key) {
	case NFT_META_CLSMARK:
		return nft_meta_set_eval_clsmark(skb, sreg);
	case NFT_META_OFLD_QUEUE:
		return nft_meta_set_eval_ofld_queue(skb, nft_reg_load8(sreg));
	case NFT_META_OFLD_PCP:
		return nft_meta_set_eval_ofld_pcp(skb, nft_reg_load8(sreg));
	case NFT_META_OFLD_DSCP:
		return nft_meta_set_eval_ofld_dscp(skb, nft_reg_load8(sreg));
	case NFT_META_QOS_BR_RULE0:
	case NFT_META_QOS_BR_RULE1:
	case NFT_META_QOS_BR_RULE2:
	case NFT_META_QOS_BR_RULE3:
	case NFT_META_QOS_BR_RULE4:
	case NFT_META_QOS_BR_RULE5:
	case NFT_META_QOS_BR_RULE6:
	case NFT_META_QOS_BR_RULE7:
		return nft_meta_set_eval_qos_br_rule(skb, nft_reg_load8(sreg), key - NFT_META_QOS_BR_RULE0);
	}

	return -EOPNOTSUPP;
}

static int nft_meta_cls_plugin_set_init(const struct nft_ctx *ctx, unsigned int *len, int key)
{
	int ret = -EOPNOTSUPP;
	bool depend_ct = false;

	switch (key) {
	case NFT_META_CLSMARK:
		*len = sizeof(u64);
		depend_ct = true;
		ret = 1;
		break;
	case NFT_META_OFLD_QUEUE:
	case NFT_META_OFLD_PCP:
	case NFT_META_OFLD_DSCP:
		*len = sizeof(u8);
		depend_ct = true;
		ret = 1;
		break;
	case NFT_META_QOS_BR_RULE0:
	case NFT_META_QOS_BR_RULE1:
	case NFT_META_QOS_BR_RULE2:
	case NFT_META_QOS_BR_RULE3:
	case NFT_META_QOS_BR_RULE4:
	case NFT_META_QOS_BR_RULE5:
	case NFT_META_QOS_BR_RULE6:
	case NFT_META_QOS_BR_RULE7:
		*len = sizeof(u8);
		depend_ct = true;
		ret = 1;
		break;
	}

	if (depend_ct && ctx->family == NFPROTO_BRIDGE)
		ret = br_chain_get_nf_ct(ctx);

	return ret;
}

static void nft_meta_cls_plugin_set_destroy(const struct nft_ctx *ctx, int key)
{
	switch (key) {
	case NFT_META_CLSMARK:
	case NFT_META_OFLD_QUEUE:
	case NFT_META_OFLD_PCP:
	case NFT_META_OFLD_DSCP:
	case NFT_META_QOS_BR_RULE0:
	case NFT_META_QOS_BR_RULE1:
	case NFT_META_QOS_BR_RULE2:
	case NFT_META_QOS_BR_RULE3:
	case NFT_META_QOS_BR_RULE4:
	case NFT_META_QOS_BR_RULE5:
	case NFT_META_QOS_BR_RULE6:
	case NFT_META_QOS_BR_RULE7:
		if (ctx->family == NFPROTO_BRIDGE)
			br_chain_put_nf_ct(ctx);
		break;
	}
}

static bool nft_url_match(const void *url, const void *pattern)
{
	const char *s1 = url;
	const char *s2 = pattern;
	size_t l1, l2;

	if (!strcmp(s2, ".."))
		return true;

	l1 = strlen(s1);
	l2 = strlen(s2);
	while (l1 >= l2) {
		l1--;
		if (!strncasecmp(s1, s2, l2))
			return true;
		s1++;
	}

	return false;
}

static bool nft_dns_match(const void *dns, const void *pattern)
{
	const char *s1 = dns;
	const char *s2 = pattern;
	size_t l1, l2;

	if (!strcmp(s2, ".."))
		return true;

	l1 = strlen(s1);
	l2 = strlen(s2);

	if (strcasecmp(s1, s2) == 0)
		return true;

	if (l1 > 4 && strncasecmp(s1, "www.", 4) == 0 && strcasecmp(s1 + 4, s2) == 0)
		return true;

	return false;
}

static bool nft_cls_match(const void *key, const void *pattern, u8 klen)
{
	switch (klen) {
	case URL_SIZE:
		return nft_url_match(key, pattern);
	case DNS_SIZE:
		return nft_dns_match(key, pattern);
	}

	return false;
}

union nft_url_data {
	char		url[URL_SIZE];
	struct nft_data val;
};

struct nft_cmp_url_expr {
	union nft_url_data	data;
	u8			sreg;
	u8			len;
	enum nft_cmp_ops	op:8;
};

static void nft_cmp_url_eval(const struct nft_expr *expr, struct nft_regs *regs,
			     const struct nft_pktinfo *pkt)
{
	const struct nft_cmp_url_expr *priv = nft_expr_priv(expr);

	if (nft_url_match(&regs->data[priv->sreg], &priv->data)) {
		if (priv->op == NFT_CMP_EQ)
			return;
	} else {
		if (priv->op == NFT_CMP_NEQ)
			return;
	}

	regs->verdict.code = NFT_BREAK;
}

static int nft_cmp_url_init(const struct nft_ctx *ctx, const struct nft_expr *expr,
			    const struct nlattr * const tb[])
{
	struct nft_cmp_url_expr *priv = nft_expr_priv(expr);
	struct nft_data_desc desc = {
		.type	= NFT_DATA_VALUE,
		.size	= sizeof(priv->data),
	};
	int err;

	err = nft_data_init(NULL, &priv->data.val, &desc, tb[NFTA_CMP_DATA]);
	if (err < 0)
		return err;

	err = nft_parse_register_load(tb[NFTA_CMP_SREG], &priv->sreg, desc.len);
	if (err < 0)
		return err;

	priv->len = desc.len;
	priv->op  = ntohl(nla_get_be32(tb[NFTA_CMP_OP]));

	if (priv->op != NFT_CMP_EQ && priv->op != NFT_CMP_NEQ)
		return -EINVAL;

	return 0;
}

static int nft_cmp_url_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_cmp_url_expr *priv = nft_expr_priv(expr);

	if (nft_dump_register(skb, NFTA_CMP_SREG, priv->sreg))
		goto nla_put_failure;
	if (nla_put_be32(skb, NFTA_CMP_OP, htonl(priv->op)))
		goto nla_put_failure;

	if (nft_data_dump(skb, NFTA_CMP_DATA, &priv->data.val,
			      NFT_DATA_VALUE, priv->len) < 0)
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static int nft_cmp_url_offload(struct nft_offload_ctx *ctx, struct nft_flow_rule *flow,
			       const struct nft_expr *expr)
{
	return -EOPNOTSUPP;
}

static const struct nft_expr_ops nft_cmp_url_ops = {
	.type		= &nft_cmp_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_cmp_url_expr)),
	.eval		= nft_cmp_url_eval,
	.init		= nft_cmp_url_init,
	.dump		= nft_cmp_url_dump,
	.offload	= nft_cmp_url_offload,
};

union nft_dns_data {
	char		dns[DNS_SIZE];
	struct nft_data val;
};

struct nft_cmp_dns_expr {
	union nft_dns_data	data;
	u8			sreg;
	u8			len;
	enum nft_cmp_ops	op:8;
};

static void nft_cmp_dns_eval(const struct nft_expr *expr, struct nft_regs *regs,
			     const struct nft_pktinfo *pkt)
{
	const struct nft_cmp_dns_expr *priv = nft_expr_priv(expr);

	if (nft_dns_match(&regs->data[priv->sreg], &priv->data)) {
		if (priv->op == NFT_CMP_EQ)
			return;
	} else {
		if (priv->op == NFT_CMP_NEQ)
			return;
	}

	regs->verdict.code = NFT_BREAK;
}

static int nft_cmp_dns_init(const struct nft_ctx *ctx, const struct nft_expr *expr,
			    const struct nlattr * const tb[])
{
	struct nft_cmp_dns_expr *priv = nft_expr_priv(expr);
	struct nft_data_desc desc = {
		.type	= NFT_DATA_VALUE,
		.size	= sizeof(priv->data),
	};
	int err;

	err = nft_data_init(NULL, &priv->data.val, &desc, tb[NFTA_CMP_DATA]);
	if (err < 0)
		return err;

	err = nft_parse_register_load(tb[NFTA_CMP_SREG], &priv->sreg, desc.len);
	if (err < 0)
		return err;

	priv->len = desc.len;
	priv->op  = ntohl(nla_get_be32(tb[NFTA_CMP_OP]));

	if (priv->op != NFT_CMP_EQ && priv->op != NFT_CMP_NEQ)
		return -EINVAL;

	return 0;
}

static int nft_cmp_dns_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_cmp_dns_expr *priv = nft_expr_priv(expr);

	if (nft_dump_register(skb, NFTA_CMP_SREG, priv->sreg))
		goto nla_put_failure;
	if (nla_put_be32(skb, NFTA_CMP_OP, htonl(priv->op)))
		goto nla_put_failure;

	if (nft_data_dump(skb, NFTA_CMP_DATA, &priv->data.val,
			      NFT_DATA_VALUE, priv->len) < 0)
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static int nft_cmp_dns_offload(struct nft_offload_ctx *ctx, struct nft_flow_rule *flow,
			       const struct nft_expr *expr)
{
	return -EOPNOTSUPP;
}

static const struct nft_expr_ops nft_cmp_dns_ops = {
	.type		= &nft_cmp_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_cmp_dns_expr)),
	.eval		= nft_cmp_dns_eval,
	.init		= nft_cmp_dns_init,
	.dump		= nft_cmp_dns_dump,
	.offload	= nft_cmp_dns_offload,
};

static const struct nft_expr_ops *
(*nft_cmp_orig_select_ops)(const struct nft_ctx *, const struct nlattr * const tb[]);

static const struct nft_expr_ops *
nft_cmp_cls_ext_select_ops(const struct nft_ctx *ctx, const struct nlattr * const tb[])
{
	int err;
	enum nft_cmp_ops op;
	union {
		char raw[NFT_DATA_VALUE_MAXLEN];
		struct nft_data val;
	} data;
	struct nft_data_desc desc = {
		.type	= NFT_DATA_VALUE,
		.size	= sizeof(data),
	};

	if (tb[NFTA_CMP_SREG] == NULL ||
	    tb[NFTA_CMP_OP] == NULL ||
	    tb[NFTA_CMP_DATA] == NULL)
		return ERR_PTR(-EINVAL);

	err = nft_data_init(NULL, &data.val, &desc, tb[NFTA_CMP_DATA]);
	if (err < 0)
		return ERR_PTR(err);

	if (desc.len == URL_SIZE) {
		op = ntohl(nla_get_be32(tb[NFTA_CMP_OP]));
		if (op != NFT_CMP_EQ && op != NFT_CMP_NEQ)
			return ERR_PTR(-EINVAL);
		return &nft_cmp_url_ops;
	}

	if (desc.len == DNS_SIZE) {
		op = ntohl(nla_get_be32(tb[NFTA_CMP_OP]));
		if (op != NFT_CMP_EQ && op != NFT_CMP_NEQ)
			return ERR_PTR(-EINVAL);
		return &nft_cmp_dns_ops;
	}

	return nft_cmp_orig_select_ops(ctx, tb);
}

static void hijack_nft_cmp_select_ops(void)
{
	nft_cmp_orig_select_ops = nft_cmp_type.select_ops;
	nft_cmp_type.select_ops = nft_cmp_cls_ext_select_ops;
}

static void restore_nft_cmp_select_ops(void)
{
	nft_cmp_type.select_ops = nft_cmp_orig_select_ops;
}

static int __init nft_cls_plugin_init(void)
{
	nft_register_meta_cls_plugin(nft_meta_cls_plugin_get_init,
				     nft_meta_cls_plugin_set_init,
				     nft_meta_cls_plugin_get_eval,
				     nft_meta_cls_plugin_set_eval,
				     nft_meta_cls_plugin_get_destroy,
				     nft_meta_cls_plugin_set_destroy);
	nft_register_cls_match(nft_cls_match);
	hijack_nft_cmp_select_ops();
	return 0;
}

static void __exit nft_cls_plugin_exit(void)
{
	nft_register_meta_cls_plugin(NULL, NULL, NULL, NULL, NULL, NULL);
	nft_register_cls_match(NULL);
	restore_nft_cmp_select_ops();
}

module_init(nft_cls_plugin_init);
module_exit(nft_cls_plugin_exit);

MODULE_DESCRIPTION("Clourneysemi plugin for netfilter");
MODULE_LICENSE("GPL");
