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
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv6.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_core.h>

#define APP_FEATURE_PATH	"/etc/cls-dpi/app_feature"
#define MIN_HTTP_DATA_LEN	16
#define MIN_FEATURE_STR_LEN	16
#define MAX_FEATURE_STR_LEN	128
#define MAX_HOST_URL_LEN	128
#define MAX_REQUEST_URL_LEN	128
#define MAX_POS_PER_FEATURE	16
#define MAX_FEATURE_LINE_LEN	256
#define MIN_FEATURE_LINE_LEN	16
#define MAX_URL_MATCH_LEN	64
#define MAX_PORT_RANGE_NUM	5

#define MAX_DPI_PKT_NUM		64

#define HTTPS_URL_OFFSET	9
#define HTTPS_LEN_OFFSET	7

enum CLS_FEATURE_PARAM_INDEX {
	CLS_PROTO_PARAM_INDEX,
	CLS_SRC_PORT_PARAM_INDEX,
	CLS_DST_PORT_PARAM_INDEX,
	CLS_HOST_URL_PARAM_INDEX,
	CLS_REQUEST_URL_PARAM_INDEX,
	CLS_DICT_PARAM_INDEX,
};

enum e_http_method {
	HTTP_METHOD_GET = 1,
	HTTP_METHOD_POST,
};

typedef struct http_proto {
	int match;
	int method;
	char *url_pos;
	int url_len;
	char *host_pos;
	int host_len;
	char *data_pos;
	int data_len;
} http_proto_t;

typedef struct https_proto {
	int match;
	char *url_pos;
	int url_len;
} https_proto_t;

typedef struct flow_info {
	struct sk_buff *skb;
	int proto;
	uint16_t sport;
	uint16_t dport;
	int l4_offset;
	int l4_len;
	http_proto_t http;
	https_proto_t https;
	uint8_t app_id;
} flow_info_t;

typedef struct pos_info {
	int pos;
	unsigned char value;
} pos_info_t;

typedef struct range_value {
	int not;
	uint16_t start;
	uint16_t end;
} range_value_t;

typedef struct port_info {
	int num;
	range_value_t range_list[MAX_PORT_RANGE_NUM];
} port_info_t;

typedef struct feature_node {
	struct list_head head;
	uint8_t app_id;
	uint8_t proto;
	uint16_t sport;
	port_info_t dport_info;
	char host_url[MAX_HOST_URL_LEN];
	char request_url[MAX_REQUEST_URL_LEN];
	int pos_num;
	pos_info_t pos_info[MAX_POS_PER_FEATURE];
} feature_node_t;

static void swap_odd_even_positions(char *str)
{
	s32 i = 0;
	s32 length = strlen(str);

	for (i = 0; i < length - 1; i += 2) {
		char temp = str[i];

		str[i] = str[i + 1];
		str[i + 1] = temp;
	}
}

static const unsigned char base64_suffix_map[256] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 255,
	255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
	52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
	255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
	7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
	19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
	255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
	37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255 };

static int base64_decode(const char *indata, int inlen, char *outdata, int *outlen)
{
	int t = 0, x = 0, y = 0, i = 0;
	unsigned char c = 0;
	int g = 3;

	if (indata == NULL || inlen <= 0 || outdata == NULL || outlen == NULL)
		return -1;

	if (inlen % 4 != 0)
		return -2;

	while (x < inlen && indata[x] != 0) {
		c = base64_suffix_map[indata[x++]];
		if (c == 255)
			return -1;
		if (c == 253)
			continue;
		if (c == 254) {
			c = 0;
			g--;
		}
		t = (t<<6) | c;
		if (++y == 4) {
			outdata[i++] = (unsigned char)((t>>16)&0xff);
			if (g > 1)
				outdata[i++] = (unsigned char)((t>>8)&0xff);
			if (g > 2)
				outdata[i++] = (unsigned char)(t&0xff);
			y = t = 0;
		}
	}

	*outlen = i;

	return 0;
}

struct list_head feature_head = LIST_HEAD_INIT(feature_head);
DEFINE_RWLOCK(feature_lock);

#define feature_list_read_lock()	read_lock_bh(&feature_lock)
#define feature_list_read_unlock()	read_unlock_bh(&feature_lock)
#define feature_list_write_lock()	write_lock_bh(&feature_lock)
#define feature_list_write_unlock()	write_unlock_bh(&feature_lock)

int regexp_match(char *reg, char *text);

#ifdef DEBUG
static void __dump_app_feature(feature_node_t *node)
{
	int i;
	port_info_t *info = &node->dport_info;

	pr_info("app %u's feature:\n", node->app_id);
	pr_info("proto:%u\n", node->proto);
	pr_info("sport:%u\n", node->sport);
	pr_info("host url:%s\n", node->host_url);
	pr_info("rqst url:%s\n", node->request_url);
	pr_cont("dport:");
	for (i = 0; i < info->num; i++) {
		pr_cont("%s%u-%u ", info->range_list[i].not ? "!" : "",
				info->range_list[i].start, info->range_list[i].end);
	}
	pr_cont("\n");
	pr_cont("pos:");
	for (i = 0; i < node->pos_num; i++)
		pr_cont("%d:%02x ", node->pos_info[i].pos, node->pos_info[i].value);
	pr_cont("\n");
}

static void dump_app_feature(void)
{
	feature_node_t *node;

	list_for_each_entry(node, &feature_head, head) {
		__dump_app_feature(node);
	}
}
#endif

static int validate_range_value(char *range_str)
{
	char *p = range_str;

	if (!range_str)
		return 0;

	while (*p) {
		if (*p == ' ' || *p == '!' || *p == '-' ||
		((*p >= '0') && (*p <= '9'))) {
			p++;
			continue;
		} else {
			pr_warn("error, invalid char %x\n", *p);
			return 0;
		}
	}
	return 1;
}

static int parse_range_value(char *range_str, range_value_t *range)
{
	int start, end;
	char pure_range[128] = {0};

	if (!validate_range_value(range_str)) {
		pr_warn("validate range str failed, value = %s\n", range_str);
		return -1;
	}
	strim(range_str);
	if (range_str[0] == '!') {
		range->not = 1;
		strcpy(pure_range, range_str + 1);
	} else {
		range->not = 0;
		strcpy(pure_range, range_str);
	}
	strim(pure_range);
	if (strstr(pure_range, "-")) {
		if (sscanf(pure_range, "%d-%d", &start, &end) != 2)
			return -1;
	} else {
		if (sscanf(pure_range, "%d", &start) != 1)
			return -1;
		end = start;
	}
	range->start = start;
	range->end = end;
	return 0;
}

static int parse_port_info(char *port_str, port_info_t *info)
{
	char *p = port_str;
	char *begin = port_str;
	char one_port_buf[128] = {0};

	strim(port_str);
	if (strlen(port_str) == 0)
		return -1;

	while (*p++) {
		if (*p != '|')
			continue;
		memset(one_port_buf, 0x0, sizeof(one_port_buf));
		strncpy(one_port_buf, begin, p - begin);
		if (parse_range_value(one_port_buf, &info->range_list[info->num]) == 0)
			info->num++;
		begin = p + 1;
	}
	memset(one_port_buf, 0x0, sizeof(one_port_buf));
	strncpy(one_port_buf, begin, p - begin);
	if (parse_range_value(one_port_buf, &info->range_list[info->num]) == 0)
		info->num++;

	return 0;
}

static int __add_app_feature(int appid, int proto, int src_port,
			     char *dst_port_str, char *host_url, char *request_url, char *dict)
{
	feature_node_t *node = NULL;
	char *p = dict;
	char *begin = dict;
	char pos[32] = {0};
	int index = 0;
	int value = 0;

	node = kzalloc(sizeof(feature_node_t), GFP_KERNEL);
	if (node == NULL)
		return -1;

	node->app_id = appid;
	node->proto = proto;
	node->sport = src_port;
	parse_port_info(dst_port_str, &node->dport_info);
	strcpy(node->host_url, host_url);
	strcpy(node->request_url, request_url);
	p = dict;
	begin = dict;
	index = 0;
	value = 0;
	while (*p++) {
		if (*p == '|') {
			strncpy(pos, begin, p - begin);
			sscanf(pos, "%d:%x", &index, &value);
			memset(pos, 0x0, sizeof(pos));
			begin = p + 1;
			node->pos_info[node->pos_num].pos = index;
			node->pos_info[node->pos_num].value = value;
			node->pos_num++;
		}
	}

	if (begin != dict)
		strncpy(pos, begin, p - begin);
	else
		strcpy(pos, dict);

	if (pos[0]) {
		sscanf(pos, "%d:%x", &index, &value);
		node->pos_info[node->pos_num].pos = index;
		node->pos_info[node->pos_num].value = value;
		node->pos_num++;
	}

	feature_list_write_lock();
	list_add(&(node->head), &feature_head);
	feature_list_write_unlock();

	return 0;
}

static int add_app_feature(int appid, char *feature)
{
	char proto_str[16] = {0};
	char src_port_str[16] = {0};
	char dst_port_str[16] = {0};
	char host_url[32] = {0};
	char request_url[128] = {0};
	char dict[128] = {0};
	int proto = IPPROTO_TCP;
	char *p = feature;
	char *begin = feature;
	int param_num = 0;
	int src_port = 0;

	if (!feature)
		return -1;

	while (*p++) {
		if (*p != ';')
			continue;

		switch (param_num) {
		case CLS_PROTO_PARAM_INDEX:
			strncpy(proto_str, begin, p - begin);
			break;
		case CLS_SRC_PORT_PARAM_INDEX:
			strncpy(src_port_str, begin, p - begin);
			break;
		case CLS_DST_PORT_PARAM_INDEX:
			strncpy(dst_port_str, begin, p - begin);
			break;
		case CLS_HOST_URL_PARAM_INDEX:
			strncpy(host_url, begin, p - begin);
			break;
		case CLS_REQUEST_URL_PARAM_INDEX:
			strncpy(request_url, begin, p - begin);
			break;
		}
		param_num++;
		begin = p + 1;
	}

	if (param_num != CLS_DICT_PARAM_INDEX && strlen(feature) > MIN_FEATURE_STR_LEN)
		return -1;

	strncpy(dict, begin, p - begin);

	if (strcmp(proto_str, "tcp") == 0)
		proto = IPPROTO_TCP;
	else if (strcmp(proto_str, "udp") == 0)
		proto = IPPROTO_UDP;
	else
		return -1;
	sscanf(src_port_str, "%d", &src_port);

	__add_app_feature(appid, proto, src_port, dst_port_str, host_url, request_url, dict);
	return 0;
}

static void parse_app_feature(char *feature_str)
{
	int app_id;
	char feature_buf[MAX_FEATURE_LINE_LEN] = {0};
	char *p = feature_str;
	char *pos = NULL;
	int len = 0;
	char *begin = NULL;
	char feature[MAX_FEATURE_STR_LEN] = {0};

	if (strstr(feature_str, "#"))
		return;

	sscanf(feature_str, "%d %*s", &app_id);
	while (*p++) {
		if (*p == '[') {
			pos = p + 1;
			continue;
		}

		if (*p == ']' && pos != NULL)
			len = p - pos;
	}

	if (pos && len)
		strncpy(feature_buf, pos, len);

	p = feature_buf;
	begin = feature_buf;

	while (*p++) {
		if (*p == ',') {
			memset(feature, 0x0, sizeof(feature));
			strncpy((char *)feature, begin, p - begin);

			add_app_feature(app_id, feature);
			begin = p + 1;
		}
	}

	if (p != begin) {
		memset(feature, 0x0, sizeof(feature));
		strncpy((char *)feature, begin, p - begin);
		add_app_feature(app_id, feature);
	}
}

static int parse_tcp_flow(struct sk_buff *skb, flow_info_t *flow, unsigned int thoff, int payload_len)
{
	struct tcphdr *tcph;
	unsigned int doff;

	tcph = tcp_hdr(skb);
	doff = tcph->doff * 4;

	flow->l4_offset = skb_transport_offset(skb) + doff;
	flow->l4_len = payload_len - doff;
	flow->dport = ntohs(tcph->dest);
	flow->sport = ntohs(tcph->source);

	return 0;
}

static int parse_udp_flow(struct sk_buff *skb, flow_info_t *flow, unsigned int thoff, int payload_len)
{
	struct udphdr *udph;

	udph = udp_hdr(skb);

	flow->l4_offset = skb_transport_offset(skb) + sizeof(*udph);
	flow->l4_len = payload_len - sizeof(*udph);
	flow->dport = ntohs(udph->dest);
	flow->sport = ntohs(udph->source);

	return 0;
}

static int parse_ipv4_flow(struct sk_buff *skb, flow_info_t *flow)
{
	struct iphdr *iph;
	u32 thoff;

	iph = ip_hdr(skb);
	thoff = iph->ihl * 4;

	if (ip_is_fragment(iph) || thoff != sizeof(struct iphdr))
		return -1;

	flow->proto = iph->protocol;

	switch (iph->protocol) {
	case IPPROTO_TCP:
		return parse_tcp_flow(skb, flow, thoff, ntohs(iph->tot_len) - thoff);
	case IPPROTO_UDP:
		return parse_udp_flow(skb, flow, thoff, ntohs(iph->tot_len) - thoff);
	}

	return -1;
}

static int parse_ipv6_flow(struct sk_buff *skb, flow_info_t *flow)
{
	struct ipv6hdr *ip6h;
	u32 thoff;

	ip6h = ipv6_hdr(skb);
	thoff = sizeof(*ip6h);

	flow->proto = ip6h->nexthdr;

	switch (ip6h->nexthdr) {
	case IPPROTO_TCP:
		return parse_tcp_flow(skb, flow, thoff, ntohs(ip6h->payload_len) - thoff);
	case IPPROTO_UDP:
		return parse_udp_flow(skb, flow, thoff, ntohs(ip6h->payload_len) - thoff);
	}

	return -1;
}

static int parse_basic_info(struct sk_buff *skb, flow_info_t *flow)
{
	if (!skb)
		return -1;

	flow->skb = skb;
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		return parse_ipv4_flow(skb, flow);
	case htons(ETH_P_IPV6):
		return parse_ipv6_flow(skb, flow);
	}
	return -1;
}

static int parse_https_flow(flow_info_t *flow)
{
	int i;
	short url_len = 0;
	char *p = NULL;
	int data_len = flow->l4_len;

	if (data_len == 0)
		return -1;

	if (!pskb_may_pull(flow->skb, flow->l4_offset + 3))
		return -1;

	p = flow->skb->data + flow->l4_offset;
	if (!(p[0] == 0x16 && p[1] == 0x03 && p[2] == 0x01))
		return -1;

	if (!pskb_may_pull(flow->skb, flow->l4_offset + data_len))
		return -1;

	p = flow->skb->data + flow->l4_offset;
	for (i = 0; i < data_len; i++) {
		if (i + HTTPS_URL_OFFSET >= data_len)
			return -1;

		if (p[i] == 0x0 && p[i + 1] == 0x0 && p[i + 2] == 0x0 && p[i + 3] != 0x0) {
			memcpy(&url_len, p + i + HTTPS_LEN_OFFSET, 2);
			if (ntohs(url_len) <= 0 || ntohs(url_len) > data_len)
				continue;
			if (i + HTTPS_URL_OFFSET + ntohs(url_len) < data_len) {
				flow->https.match = true;
				flow->https.url_pos = p + i + HTTPS_URL_OFFSET;
				flow->https.url_len = ntohs(url_len);
				return 0;
			}
		}
	}
	return -1;
}

static void parse_http_flow(flow_info_t *flow)
{
	int i = 0;
	int start = 0;
	char *data = NULL;
	int data_len = 0;

	data_len = flow->l4_len;
	if (data_len < MIN_HTTP_DATA_LEN)
		return;

	if (!pskb_may_pull(flow->skb, flow->l4_offset + data_len))
		return;

	data = flow->skb->data + flow->l4_offset;
	for (i = 0; i < data_len; i++) {
		if (data[i] == 0x0d && data[i + 1] == 0x0a) {
			if (memcmp(&data[start], "POST ", 5) == 0) {
				flow->http.match = true;
				flow->http.method = HTTP_METHOD_POST;
				flow->http.url_pos = data + start + 5;
				flow->http.url_len = i - start - 5;
			} else if (memcmp(&data[start], "GET ", 4) == 0) {
				flow->http.match = true;
				flow->http.method = HTTP_METHOD_GET;
				flow->http.url_pos = data + start + 4;
				flow->http.url_len = i - start - 4;
			} else if (memcmp(&data[start], "Host:", 5) == 0) {
				flow->http.host_pos = data + start + 6;
				flow->http.host_len = i - start - 6;
			}

			if (data[i + 2] == 0x0d && data[i + 3] == 0x0a) {
				flow->http.data_pos = data + i + 4;
				flow->http.data_len = data_len - i - 4;
				break;
			}

			start = i + 2;
		}
	}
}

static int match_port(port_info_t *info, int port)
{
	int i;
	int with_not = 0;

	if (info->num == 0)
		return 1;

	for (i = 0; i < info->num; i++) {
		if (info->range_list[i].not) {
			with_not = 1;
			break;
		}
	}

	for (i = 0; i < info->num; i++) {
		if (with_not) {
			if (info->range_list[i].not &&
			    port >= info->range_list[i].start &&
			    port <= info->range_list[i].end) {
				return 0;
			}
		} else {
			if (port >= info->range_list[i].start &&
			    port <= info->range_list[i].end) {
				return 1;
			}
		}
	}

	return with_not;
}

static int match_by_pos(flow_info_t *flow, feature_node_t *node)
{
	int i, pos;
	unsigned char byte, *value;

	for (i = 0; i < node->pos_num; i++) {
		if (node->pos_info[i].pos < 0)
			pos = flow->l4_len + node->pos_info[i].pos;
		else
			pos = node->pos_info[i].pos;

		if (pos >= flow->l4_len)
			return false;

		value = skb_header_pointer(flow->skb, flow->l4_offset + pos, sizeof(byte), &byte);
		if (!value || *value != node->pos_info[i].value)
			return false;
	}
	return true;
}

static int match_by_url(flow_info_t *flow, feature_node_t *node)
{
	char reg_url_buf[MAX_URL_MATCH_LEN] = {0};

	if (!flow || !node)
		return false;

	if (flow->https.match == true && flow->https.url_pos) {
		if (flow->https.url_len >= MAX_URL_MATCH_LEN)
			strncpy(reg_url_buf, flow->https.url_pos, MAX_URL_MATCH_LEN - 1);
		else
			strncpy(reg_url_buf, flow->https.url_pos, flow->https.url_len);
	} else if (flow->http.match == true && flow->http.host_pos) {
		if (flow->http.host_len >= MAX_URL_MATCH_LEN)
			strncpy(reg_url_buf, flow->http.host_pos, MAX_URL_MATCH_LEN - 1);
		else
			strncpy(reg_url_buf, flow->http.host_pos, flow->http.host_len);
	}

	if (strlen(reg_url_buf) > 0 && strlen(node->host_url) > 0 && regexp_match(node->host_url, reg_url_buf))
		return true;

	if (flow->http.match == true && flow->http.url_pos) {
		memset(reg_url_buf, 0x0, sizeof(reg_url_buf));
		if (flow->http.url_len >= MAX_URL_MATCH_LEN)
			strncpy(reg_url_buf, flow->http.url_pos, MAX_URL_MATCH_LEN - 1);
		else
			strncpy(reg_url_buf, flow->http.url_pos, flow->http.url_len);
		if (strlen(reg_url_buf) > 0 && strlen(node->request_url) > 0 &&
		    regexp_match(node->request_url, reg_url_buf))
			return true;
	}
	return false;
}

static int match_one(flow_info_t *flow, feature_node_t *node)
{
	if (node->proto > 0 && flow->proto != node->proto)
		return false;

	if (flow->l4_len == 0)
		return false;

	if (node->sport != 0 && flow->sport != node->sport)
		return false;

	if (!match_port(&node->dport_info, flow->dport))
		return false;

	if (strlen(node->request_url) > 0 || strlen(node->host_url) > 0)
		return match_by_url(flow, node);
	else if (node->pos_num > 0)
		return match_by_pos(flow, node);

	return true;
}

uint8_t do_dpi(struct sk_buff *skb)
{
	flow_info_t _flow = {}, *flow = &_flow;
	feature_node_t *node;

	if (parse_basic_info(skb, flow) < 0)
		return 0;

	if (flow->proto == IPPROTO_TCP) {
		if (flow->sport == 80 || flow->dport == 80)
			parse_http_flow(flow);
		if (flow->dport == 443 || flow->dport == 8443)
			parse_https_flow(flow);
	}

	feature_list_read_lock();
	list_for_each_entry(node, &feature_head, head) {
		if (match_one(flow, node)) {
			flow->app_id = node->app_id;
			break;
		}
	}
	feature_list_read_unlock();

	return flow->app_id;
}

static unsigned int cls_dpi_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	unsigned long long total_packets = 0;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct = NULL;
	struct nf_conn_acct *acct;
	int app_id;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL || !nf_ct_is_confirmed(ct))
		return NF_ACCEPT;

	if (ct->cls_dpi_done)
		return NF_ACCEPT;

	acct = nf_conn_acct_find(ct);
	if (!acct)
		return NF_ACCEPT;

	total_packets = (unsigned long long)atomic64_read(&acct->counter[IP_CT_DIR_ORIGINAL].packets) +
			(unsigned long long)atomic64_read(&acct->counter[IP_CT_DIR_REPLY].packets);
	if (total_packets > MAX_DPI_PKT_NUM) {
		ct->cls_dpi_done = 1;
		ct->cls_app_id = 0;
		return NF_ACCEPT;
	}

	app_id = do_dpi(skb);
	if (app_id != 0) {
		ct->cls_dpi_done = 1;
		ct->cls_app_id = app_id;
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops cls_dpi_hooks[] __read_mostly = {
	{
		.hook		= cls_dpi_hook,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FILTER + 1,
	},
	{
		.hook		= cls_dpi_hook,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FILTER + 1,
	},
};

static int load_app_feature(void)
{
	void *cipher_data = NULL, *plain_data = NULL;
	int len, rc, cipher_data_len, plain_data_len;
	char *p, *begin;

	rc = kernel_read_file_from_path(APP_FEATURE_PATH, 0, &cipher_data, INT_MAX, NULL,
					READING_FIRMWARE);
	if (rc < 0) {
		pr_err("Unable to open file: %s (%d)", APP_FEATURE_PATH, rc);
		return rc;
	}

	cipher_data_len = rc;
	/* base64 decode plain_data is less than cipher_data */
	plain_data = vmalloc(cipher_data_len);

	if (!plain_data) {
		vfree(cipher_data);
		return -ENOMEM;
	}

	swap_odd_even_positions(cipher_data);

	rc = base64_decode(cipher_data, cipher_data_len, plain_data, &plain_data_len);
	if (rc < 0) {
		pr_info("failed to decode rc = %d\n", rc);
		vfree(plain_data);
		vfree(cipher_data);
		return -EINVAL;
	}

	p = begin = plain_data;
	for (len = plain_data_len; len > 0; --len, ++p) {
		if (*p == '\n' || len == 1) {
			if (*begin == '#' ||
			    p - begin < MIN_FEATURE_LINE_LEN ||
			    p - begin > MAX_FEATURE_LINE_LEN) {
				begin = p + 1;
				continue;
			}
			if (*p == '\n')
				*p = 0;
			parse_app_feature(begin);
			begin = p + 1;
		}
	}
	vfree(plain_data);
	vfree(cipher_data);
#ifdef DEBUG
	dump_app_feature();
#endif
	return 0;
}

static void free_app_feature(void)
{
	feature_node_t *node;

	feature_list_write_lock();
	while (!list_empty(&feature_head)) {
		node = list_first_entry(&feature_head, feature_node_t, head);
		list_del(&(node->head));
		kfree(node);
	}
	feature_list_write_unlock();
}

static int __init cls_dpi_init(void)
{
	int err;

	err = load_app_feature();
	if (err)
		return err;

	err =  nf_register_net_hooks(&init_net, cls_dpi_hooks, ARRAY_SIZE(cls_dpi_hooks));
	if (err) {
		free_app_feature();
		return err;
	}

	nf_conntrack_offload_thresh = MAX_DPI_PKT_NUM;
	pr_info("nf_conntrack_offload_thresh is set to %d by cls_dpi\n", nf_conntrack_offload_thresh);

	return err;
}

static void __exit cls_dpi_fini(void)
{
	nf_unregister_net_hooks(&init_net, cls_dpi_hooks, ARRAY_SIZE(cls_dpi_hooks));
	free_app_feature();

	nf_conntrack_offload_thresh = 0;
	pr_info("nf_conntrack_offload_thresh is set to 0 by cls_dpi\n");
}

module_init(cls_dpi_init);
module_exit(cls_dpi_fini);

MODULE_DESCRIPTION("Clourneysemi DPI");
MODULE_LICENSE("GPL");
