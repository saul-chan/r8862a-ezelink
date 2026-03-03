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

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <ctype.h>
#include <stdarg.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>

#include <uci.h>
#include <libubox/list.h>
#include <libubox/ulog.h>
#include "clsqos.h"

#ifndef BIT
#define BIT(x)	(1UL << (x))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define rule_has_ops(rule, ops) (rule->options & (ops))

#define rule_test_ops(rule, mask, value) ((rule->options & (mask)) == (value))

#define CLSQOS_OPT(name, parse, member) \
	{ name, parse, offsetof(struct clsqos_rule, member), 0 }

#define CLSQOS_LIST(name, parse, member) \
	{ name, parse, offsetof(struct clsqos_rule, member), 1 }

#define NFT_FILE_INET_FWD	"/tmp/cls-qos/inet-fwd.nft"
#define NFT_FILE_BR_INPUT	"/tmp/cls-qos/br-input.nft"
#define NFT_FILE_BR_OUTPUT	"/tmp/cls-qos/br-output.nft"

#define QOS_BR_RULE_MAX_NUM	8
#define CLSMARK_BIT_FOR_QOS_BR_RULE0_IDX	48

enum ip_family {
	FAMILY_ANY	= 0,
	FAMILY_V4	= 4,
	FAMILY_V6	= 6,
};

enum rule_option {
	OPS_SRC_DEV	= BIT(0),
	OPS_DST_DEV	= BIT(1),
	OPS_SRC_MAC	= BIT(2),
	OPS_DST_MAC	= BIT(3),
	OPS_SRC_IPV4	= BIT(4),
	OPS_DST_IPV4	= BIT(5),
	OPS_SRC_IPV6	= BIT(6),
	OPS_DST_IPV6	= BIT(7),
	OPS_DSCP	= BIT(8),
};

#define OPS_IPV4_ADDR	(OPS_SRC_IPV4 | OPS_DST_IPV4)
#define OPS_IPV6_ADDR	(OPS_SRC_IPV6 | OPS_DST_IPV6)
#define OPS_IP_ADDR	(OPS_IPV4_ADDR | OPS_IPV6_ADDR)

enum {
	CMD_START	= BIT(0),
	CMD_STOP	= BIT(1),
};

struct clsqos_dev {
	struct list_head list;

	bool inv;
	const char *name;
};

struct clsqos_mac {
	struct list_head list;

	bool inv;
	struct ether_addr mac;
};

struct clsqos_address {
	struct list_head list;

	bool range;
	bool inv;
	enum ip_family family;
	union {
		struct in_addr v4;
		struct in6_addr v6;
	} address;
	union {
		struct in_addr v4;
		struct in6_addr v6;
	} mask;
};

struct clsqos_protocol {
	struct list_head list;

	bool any;
	bool inv;
	uint32_t protocol;
};

struct clsqos_port {
	struct list_head list;

	bool inv;
	uint16_t port_min;
	uint16_t port_max;
};

struct clsqos_dscp {
	struct list_head list;

	bool set;
	bool inv;
	uint8_t dscp;
};

struct clsqos_priority {
	bool set;
	uint8_t priority;
};

struct clsqos_rate {
	bool set;
	u64 kbit_per_sec;
};

struct clsqos_rule {
	struct list_head list;

	bool enabled;
	const char *name;
	int id;
	uint32_t options;
	struct uci_element *uci_elm;

	/* match */
	enum ip_family family;
	struct list_head proto;
	struct list_head src_dev;
	struct list_head dst_dev;
	struct list_head src_mac;
	struct list_head dst_mac;
	struct list_head src_ip;
	struct list_head dst_ip;
	struct list_head src_port;
	struct list_head dst_port;
	struct list_head dscp;

	/* action */
	struct clsqos_priority set_vlan8021p;
	struct clsqos_priority set_priority;
	struct clsqos_dscp set_dscp;
	struct clsqos_rate rate_limit;
};

struct clsqos_option {
	const char *name;
	bool (*parse)(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list);
	size_t offset;
	bool is_list;
};

static const struct {
	const char *name;
	uint8_t dscp;
} dscp_classes[] = {
	{ "BE",   0x00 },
	{ "DF",   0x00 },
	{ "CS0",  0x00 },
	{ "CS1",  0x08 },
	{ "CS2",  0x10 },
	{ "CS3",  0x18 },
	{ "CS4",  0x20 },
	{ "CS5",  0x28 },
	{ "CS6",  0x30 },
	{ "CS7",  0x38 },
	{ "LE",   0x01 },
	{ "AF11", 0x0a },
	{ "AF12", 0x0c },
	{ "AF13", 0x0e },
	{ "AF21", 0x12 },
	{ "AF22", 0x14 },
	{ "AF23", 0x16 },
	{ "AF31", 0x1a },
	{ "AF32", 0x1c },
	{ "AF33", 0x1e },
	{ "AF41", 0x22 },
	{ "AF42", 0x24 },
	{ "AF43", 0x26 },
	{ "EF",   0x2e }
};

static const struct clsqos_match {
	bool inv;
	const char *operator;
} clsqos_matches[] = {
	{ false, "" },
	{ true,  "!= " },
	{ }
};

static const char *rule_name(struct clsqos_rule *rule)
{
	static char name[16];

	if (rule->name)
		return rule->name;

	sprintf(name, "@rule[%d]", rule->id);

	return name;
}

static int get_section_name(char *buf, struct uci_section *s, bool find_name)
{
	int i = 0, n = 0;
	struct uci_option *o;
	struct uci_element *tmp;

	if (s->anonymous) {
		uci_foreach_element(&s->package->sections, tmp) {
			if (strcmp(uci_to_section(tmp)->type, s->type))
				continue;

			if (&s->e == tmp)
				break;

			i++;
		}

		n = sprintf(buf, "@%s[%d]", s->type, i);

		if (find_name) {
			uci_foreach_element(&s->options, tmp) {
				o = uci_to_option(tmp);

				if (!strcmp(tmp->name, "name") && (o->type == UCI_TYPE_STRING)) {
					n += sprintf(buf + n, " (%s)", o->v.string);
					break;
				}
			}
		}
	} else {
		n = sprintf(buf, "'%s'", s->e.name);
	}

	return n;
}

static void warn_elem(struct uci_element *e, const char *format, ...)
{
	va_list argptr;
	char buf[512];
	int offset = 0;

	if (e->type == UCI_TYPE_SECTION) {
		offset = sprintf(buf, "Warning: Section ");
		offset += get_section_name(buf + offset, uci_to_section(e), true);
		offset += sprintf(buf + offset, " ");
	} else if (e->type == UCI_TYPE_OPTION) {
		offset = sprintf(buf, "Warning: Option ");
		offset += get_section_name(buf + offset, uci_to_option(e)->section, false);
		offset += sprintf(buf + offset, ".%s ", e->name);
	}

	va_start(argptr, format);
	vsnprintf(buf + offset, sizeof(buf) - offset, format, argptr);
	va_end(argptr);

	ULOG_WARN("%s\n", buf);
}

static void error(const char *format, ...)
{
	va_list argptr;

	va_start(argptr, format);
	ULOG_ERR(format, argptr);
	va_end(argptr);

	exit(1);
}

static int netmask2bitlen(int family, void *mask)
{
	int bits;
	struct in_addr *v4;
	struct in6_addr *v6;

	if (family == FAMILY_V6)
		for (bits = 0, v6 = mask; bits < 128 && (v6->s6_addr[bits / 8] << (bits % 8)) & 128; bits++)
			;
	else
		for (bits = 0, v4 = mask; bits < 32 && (ntohl(v4->s_addr) << bits) & 0x80000000; bits++)
			;

	return bits;
}

static bool bitlen2netmask(int family, int bits, void *mask)
{
	int i;
	uint8_t rem, b;
	struct in_addr *v4;
	struct in6_addr *v6;

	if (family == FAMILY_V6) {
		if (bits < -128 || bits > 128)
			return false;

		v6 = mask;
		rem = abs(bits);

		for (i = 0; i < sizeof(v6->s6_addr); i++) {
			b = (rem > 8) ? 8 : rem;
			v6->s6_addr[i] = (uint8_t) (0xFF << (8 - b));
			rem -= b;
		}

		if (bits < 0)
			for (i = 0; i < sizeof(v6->s6_addr); i++)
				v6->s6_addr[i] = ~v6->s6_addr[i];
	} else {
		if (bits < -32 || bits > 32)
			return false;

		v4 = mask;
		v4->s_addr = bits ? htonl(~((1 << (32 - abs(bits))) - 1)) : 0;

		if (bits < 0)
			v4->s_addr = ~v4->s_addr;
	}

	return true;
}

static bool put_value(void *ptr, void *val, int elem_size, bool is_list)
{
	void *copy;

	if (is_list) {
		copy = malloc(elem_size);

		if (!copy)
			return false;

		memcpy(copy, val, elem_size);
		list_add_tail((struct list_head *)copy, (struct list_head *)ptr);
		return true;
	}

	memcpy(ptr, val, elem_size);
	return false;
}

static bool parse_bool(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	if (!strcmp(val, "true") || !strcmp(val, "yes") || !strcmp(val, "1"))
		*((bool *)((char *)rule + offset)) = true;
	else
		*((bool *)((char *)rule + offset)) = false;

	return true;
}

static bool parse_string(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	*((const char **)((char *)rule + offset)) = val;
	return true;
}

static bool parse_device(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	struct clsqos_dev dev = { };

	if (*val == '!') {
		dev.inv = true;
		while (isspace(*++val))
			;
	}

	dev.name = val;
	put_value((char *)rule + offset, &dev, sizeof(dev), is_list);

	if (offset == offsetof(struct clsqos_rule, src_dev))
		rule->options |= OPS_SRC_DEV;
	else if (offset == offsetof(struct clsqos_rule, dst_dev))
		rule->options |= OPS_DST_DEV;

	return true;
}

static bool parse_mac(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	struct clsqos_mac addr = { };

	if (*val == '!') {
		addr.inv = true;
		while (isspace(*++val))
			;
	}

	if (!ether_aton_r(val, &addr.mac))
		return false;

	put_value((char *)rule + offset, &addr, sizeof(addr), is_list);

	if (offset == offsetof(struct clsqos_rule, src_mac))
		rule->options |= OPS_SRC_MAC;
	else if (offset == offsetof(struct clsqos_rule, dst_mac))
		rule->options |= OPS_DST_MAC;

	return true;
}

static bool parse_address(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	struct clsqos_address addr = { };
	struct in_addr v4;
	struct in6_addr v6;
	char *p = NULL, *m = NULL, *s, *e;
	int bits = -1;

	if (*val == '!') {
		addr.inv = true;
		while (isspace(*++val))
			;
	}

	s = strdup(val);
	if (!s)
		return false;

	m = strchr(s, '/');
	if (m != NULL)
		*m++ = 0;
	else {
		p = strchr(s, '-');
		if (p != NULL)
			*p++ = 0;
	}

	if (inet_pton(AF_INET6, s, &v6)) {
		addr.family = FAMILY_V6;
		addr.address.v6 = v6;

		if (m) {
			if (!inet_pton(AF_INET6, m, &v6)) {
				bits = strtol(m, &e, 10);

				if ((*e != 0) || !bitlen2netmask(addr.family, bits, &v6))
					goto fail;
			}

			addr.mask.v6 = v6;
		} else if (p) {
			if (!inet_pton(AF_INET6, p, &addr.mask.v6))
				goto fail;

			addr.range = true;
		} else {
			memset(addr.mask.v6.s6_addr, 0xFF, 16);
		}
	} else if (inet_pton(AF_INET, s, &v4)) {
		addr.family = FAMILY_V4;
		addr.address.v4 = v4;

		if (m) {
			if (!inet_pton(AF_INET, m, &v4)) {
				bits = strtol(m, &e, 10);

				if ((*e != 0) || !bitlen2netmask(addr.family, bits, &v4))
					goto fail;
			}

			addr.mask.v4 = v4;
		} else if (p) {
			if (!inet_pton(AF_INET, p, &addr.mask.v4))
				goto fail;

			addr.range = true;
		} else {
			addr.mask.v4.s_addr = 0xFFFFFFFF;
		}
	} else {
		goto fail;
	}

	free(s);
	put_value((char *)rule + offset, &addr, sizeof(addr), is_list);

	if (offset == offsetof(struct clsqos_rule, src_ip)) {
		if (addr.family == FAMILY_V4)
			rule->options |= OPS_SRC_IPV4;
		else
			rule->options |= OPS_SRC_IPV6;
	} else if (offset == offsetof(struct clsqos_rule, dst_ip)) {
		if (addr.family == FAMILY_V4)
			rule->options |= OPS_DST_IPV4;
		else
			rule->options |= OPS_DST_IPV6;
	}

	return true;

fail:
	free(s);
	return false;
}

static bool parse_port(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	struct clsqos_port range = { };
	uint16_t n;
	uint16_t m;
	char *p, *e;

	if (*val == '!') {
		range.inv = true;
		while (isspace(*++val))
			;
	}

	n = strtoul(val, &e, 10);
	if (n < 0 || n > 65535)
		return false;

	if (e == val || (*e && *e != '-'))
		return false;

	if (*e) {
		p = e + 1;
		m = strtoul(p, &e, 10);
		if (m < 0 || m > 65535)
			return false;

		if (e == p || *e || m < n)
			return false;

		range.port_min = n;
		range.port_max = m;
	} else {
		range.port_min = n;
		range.port_max = n;
	}

	put_value((char *)rule + offset, &range, sizeof(range), is_list);

	return true;
}

static bool parse_family(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	enum ip_family *family = (enum ip_family *)((char *)rule + offset);

	if (!strcmp(val, "all") || !strcmp(val, "any") || !strcmp(val, "*"))
		*family = FAMILY_ANY;
	else if (!strcmp(val, "inet") || strrchr(val, '4'))
		*family = FAMILY_V4;
	else if (!strcmp(val, "inet6") || strrchr(val, '6'))
		*family = FAMILY_V6;
	else
		return false;

	return true;
}

static bool parse_protocol(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	struct clsqos_protocol proto = { };
	struct protoent *ent;
	void *ptr = (char *)rule + offset;
	char *e;

	if (*val == '!') {
		proto.inv = true;
		while (isspace(*++val))
			;
	}

	if (!strcmp(val, "all") || !strcmp(val, "any") || !strcmp(val, "*")) {
		if (proto.inv)
			return false;

		proto.any = true;
		put_value(ptr, &proto, sizeof(proto), is_list);
		return true;
	} else if (!strcmp(val, "tcpudp")) {
		proto.protocol = 6;
		if (put_value(ptr, &proto, sizeof(proto), is_list)) {
			proto.protocol = 17;
			put_value(ptr, &proto, sizeof(proto), is_list);
		}

		return true;
	}

	ent = getprotobyname(val);

	if (ent) {
		proto.protocol = ent->p_proto;
		put_value(ptr, &proto, sizeof(proto), is_list);
		return true;
	}

	proto.protocol = strtoul(val, &e, 10);

	if (e == val || *e)
		return false;

	put_value(ptr, &proto, sizeof(proto), is_list);

	return true;
}

static bool parse_dscp(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	struct clsqos_dscp dscp = { };
	void *ptr = (char *)rule + offset;
	uint32_t n;
	char *e;

	if (*val == '!') {
		dscp.inv = true;
		while (isspace(*++val))
			;
	}

	for (n = 0; n < ARRAY_SIZE(dscp_classes); n++) {
		if (strcasecmp(dscp_classes[n].name, val))
			continue;

		dscp.set = true;
		dscp.dscp = dscp_classes[n].dscp;
		put_value(ptr, &dscp, sizeof(dscp), is_list);
		if (offset == offsetof(struct clsqos_rule, dscp))
			rule->options |= OPS_DSCP;
		return true;
	}

	n = strtoul(val, &e, 0);
	if (e == val || *e || n > 0x3F)
		return false;

	dscp.set = true;
	dscp.dscp = n;
	put_value(ptr, &dscp, sizeof(dscp), is_list);
	if (offset == offsetof(struct clsqos_rule, dscp))
		rule->options |= OPS_DSCP;

	return true;
}

static bool parse_priority(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	char *e;
	int n = strtol(val, &e, 0);
	struct clsqos_priority *p = (struct clsqos_priority *)((char *)rule + offset);

	if (e == val || *e || n < 0 || n > 7)
		return false;

	p->set = true;
	p->priority = n;

	return true;
}

static bool parse_rate_mbps(struct clsqos_rule *rule, size_t offset, const char *val, bool is_list)
{
	char *e;
	double mbit_per_sec = strtod(val, &e);
	struct clsqos_rate *rate = (struct clsqos_rate *)((char *)rule + offset);

	if (e == val || *e || mbit_per_sec < 0.5)
		return false;

	rate->set = true;
	rate->kbit_per_sec = mbit_per_sec * 1024;

	return true;
}

static const struct clsqos_option clsqos_opts[] = {
	CLSQOS_OPT("enabled",		parse_bool,		enabled),
	CLSQOS_OPT("name",		parse_string,		name),

	/* match */
	CLSQOS_OPT("family",		parse_family,		family),
	CLSQOS_LIST("proto",		parse_protocol,		proto),
	CLSQOS_LIST("src_ip",		parse_address,		src_ip),
	CLSQOS_LIST("src_mac",		parse_mac,		src_mac),
	CLSQOS_LIST("src_port",		parse_port,		src_port),
	CLSQOS_LIST("src_device",	parse_device,		src_dev),
	CLSQOS_LIST("dest_ip",		parse_address,		dst_ip),
	CLSQOS_LIST("dest_mac",		parse_mac,		dst_mac),
	CLSQOS_LIST("dest_port",	parse_port,		dst_port),
	CLSQOS_LIST("dest_device",	parse_device,		dst_dev),
	CLSQOS_LIST("dscp",		parse_dscp,		dscp),

	/* action */
	CLSQOS_OPT("set_dscp",		parse_dscp,		set_dscp),
	CLSQOS_OPT("set_priority",	parse_priority,		set_priority),
	CLSQOS_OPT("set_vlan8021p",	parse_priority,		set_vlan8021p),
	CLSQOS_OPT("rate_limit_mbps",	parse_rate_mbps,	rate_limit),

	{ }
};

static struct clsqos_rule *alloc_rule(struct list_head *rule_list)
{
	const struct clsqos_option *opt;
	struct list_head *list;
	struct clsqos_rule *rule;

	rule = calloc(1, sizeof(*rule));
	if (!rule)
		return NULL;

	for (opt = clsqos_opts; opt->name; opt++) {
		if (!opt->is_list)
			continue;

		list = (struct list_head *)((char *)rule + opt->offset);
		INIT_LIST_HEAD(list);
	}

	list_add_tail(&rule->list, rule_list);
	rule->enabled = true;

	return rule;
}

static bool parse_rule(struct clsqos_rule *rule, struct uci_section *section)
{
	char *p, *v;
	bool known, inv;
	struct uci_element *e, *l;
	struct uci_option *o;
	const struct clsqos_option *opt;
	bool valid = true;

	uci_foreach_element(&section->options, e) {
		o = uci_to_option(e);
		known = false;

		for (opt = clsqos_opts; opt->name; opt++) {
			if (!opt->parse)
				continue;

			if (strcmp(opt->name, e->name))
				continue;

			if (o->type == UCI_TYPE_LIST) {
				if (!opt->is_list) {
					warn_elem(e, "must not be a list");
					valid = false;
				} else {

					uci_foreach_element(&o->v.list, l) {
						if (!l->name)
							continue;

						if (!opt->parse(rule, opt->offset, l->name, true)) {
							warn_elem(e, "has invalid value '%s'", l->name);
							valid = false;
							continue;
						}
					}
				}
			} else {
				v = o->v.string;

				if (!v)
					continue;

				if (!opt->is_list) {
					if (!opt->parse(rule, opt->offset, o->v.string, false)) {
						warn_elem(e, "has invalid value '%s'", o->v.string);
						valid = false;
					}
				} else {
					inv = false;
					for (p = strtok(v, " \t"); p != NULL; p = strtok(NULL, " \t")) {
						/* If we encounter a sole "!" token, assume that it
						 * is meant to be part of the next token, so silently
						 * skip it and remember the state...
						 */
						if (!strcmp(p, "!")) {
							inv = true;
							continue;
						}

						/* The previous token was a sole "!", rewind pointer
						 * back by one byte to precede the value with an
						 * exclamation mark which effectively turns
						 * ("!", "foo") into ("!foo")
						 */
						if (inv) {
							*--p = '!';
							inv = false;
						}

						if (!opt->parse(rule, opt->offset, p, true)) {
							warn_elem(e, "has invalid value '%s'", p);
							valid = false;
							continue;
						}
					}

					/* The last token was a sole "!" without any subsequent
					 * text, so pass it to the option parser as-is.
					 */
					if (inv && !opt->parse(rule, opt->offset, "!", true)) {
						warn_elem(e, "has invalid value '%s'", v);
						valid = false;
					}
				}
			}

			known = true;
			break;
		}

		if (!known)
			warn_elem(e, "is unknown");
	}

	return valid;
}

static bool check_rule(struct clsqos_rule *rule, struct uci_element *e)
{
	/* rule does not specify a protocol, assuming TCP+UDP */
	if (list_empty(&rule->proto))
		parse_protocol(rule, offsetof(struct clsqos_rule, proto), "tcpudp", true);

	if (!rule->set_dscp.set && !rule->set_vlan8021p.set && !rule->set_priority.set &&
	    !rule->rate_limit.set) {
		warn_elem(e, "must have one of 'set_dscp', 'set_vlan8021p', 'set_priority' "
			     "'rate_limit' option");
		return false;
	}

	if (rule->set_dscp.inv) {
		warn_elem(e, "must not have inverted 'set_dscp'");
		return false;
	}

	if (rule_has_ops(rule, OPS_IPV4_ADDR) &&
	    !rule_has_ops(rule, OPS_IPV6_ADDR)) {
		if (rule->family == FAMILY_V6) {
			warn_elem(e, "family is ipv6 but only has ipv4 address");
			return false;
		}
		if (rule->family == FAMILY_ANY)
			rule->family = FAMILY_V4;
	}

	if (rule_has_ops(rule, OPS_IPV6_ADDR) &&
	    !rule_has_ops(rule, OPS_IPV4_ADDR)) {
		if (rule->family == FAMILY_V4) {
			warn_elem(e, "family is ipv4 but only has ipv6 address");
			return false;
		}
		if (rule->family == FAMILY_ANY)
			rule->family = FAMILY_V6;
	}

	if (rule_test_ops(rule, OPS_SRC_IPV4 | OPS_SRC_IPV6, OPS_SRC_IPV4) &&
	    rule_test_ops(rule, OPS_DST_IPV4 | OPS_DST_IPV6, OPS_DST_IPV6)) {
		warn_elem(e, "src_ip is ipv4 but dest_ip is ipv6");
		return false;
	}

	if (rule_test_ops(rule, OPS_SRC_IPV4 | OPS_SRC_IPV6, OPS_SRC_IPV6) &&
	    rule_test_ops(rule, OPS_DST_IPV4 | OPS_DST_IPV6, OPS_DST_IPV4)) {
		warn_elem(e, "src_ip is ipv6 but dest_ip is ipv4");
		return false;
	}

	if (rule_has_ops(rule, OPS_SRC_DEV | OPS_SRC_MAC) &&
	    rule_has_ops(rule, OPS_DST_DEV | OPS_DST_MAC)) {
		warn_elem(e, "src_device/src_mac can not work together with dest_device/dest_mac");
		return false;
	}

	return true;
}

static void free_rule(struct clsqos_rule *rule)
{
	const struct clsqos_option *opt;
	struct list_head *list, *cur, *tmp;

	for (opt = clsqos_opts; opt->name; opt++) {
		if (!opt->is_list)
			continue;

		list = (struct list_head *)((char *)rule + opt->offset);
		list_for_each_safe(cur, tmp, list) {
			list_del(cur);
			free(cur);
		}
	}

	list_del(&rule->list);
	free(rule);
}

static void load_rules(struct list_head *rule_list, struct uci_package *p)
{
	struct uci_section *s;
	struct uci_element *e;
	struct clsqos_rule *rule;
	int id = -1;

	uci_foreach_element(&p->sections, e) {
		s = uci_to_section(e);

		if (strcmp(s->type, "rule"))
			continue;

		id++;
		rule = alloc_rule(rule_list);
		if (!rule)
			continue;

		if (!parse_rule(rule, s)) {
			warn_elem(e, "skipped due to invalid options");
			free_rule(rule);
			continue;
		}

		if (!rule->enabled) {
			free_rule(rule);
			continue;
		}

		if (!check_rule(rule, e)) {
			warn_elem(e, "skipped due to invalid options");
			free_rule(rule);
			continue;
		}

		rule->uci_elm = e;
		rule->id = id;
	}
}

static char *print_dev(struct clsqos_rule *rule, struct list_head *list, char *buf, int len, bool inv)
{
	struct clsqos_dev *e;
	int offset = 0;
	int num = 0;

	list_for_each_entry(e, list, list) {
		if (e->inv != inv)
			continue;
		if (len - offset <= 0) {
			error("%s: buf is not big enough\n", __func__);
			break;
		}
		offset += snprintf(buf + offset, len - offset, "%s%s", num ? ", " : "", e->name);
		num++;
	}

	if (num == 1) {
		return buf;
	} else if (num > 1) {
		snprintf(buf + offset, len - offset, " }");
		return buf - 2;
	}

	return NULL;
}

static char *print_mac(struct clsqos_rule *rule, struct list_head *list, char *buf, int len, bool inv)
{
	struct clsqos_mac *e;
	uint8_t *addr;
	int offset = 0;
	int num = 0;

	list_for_each_entry(e, list, list) {
		if (e->inv != inv)
			continue;
		if (len - offset <= 0) {
			error("%s: buf is not big enough\n", __func__);
			break;
		}
		addr = e->mac.ether_addr_octet;
		offset += snprintf(buf + offset, len - offset, "%s%02x:%02x:%02x:%02x:%02x:%02x", num ? ", " : "",
				   addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		num++;
	}

	if (num == 1) {
		return buf;
	} else if (num > 1) {
		snprintf(buf + offset, len - offset, " }");
		return buf - 2;
	}

	return NULL;
}

static char *print_family(struct clsqos_rule *rule, char *buf, int len, int family)
{
	if (family == FAMILY_V4) {
		snprintf(buf, len, "ipv4");
		return buf;
	} else if (family == FAMILY_V6) {
		snprintf(buf, len, "ipv6");
		return buf;
	}

	return NULL;
}

static char *print_proto(struct clsqos_rule *rule, struct list_head *list, char *buf, int len, bool inv)
{
	struct clsqos_protocol *e;
	struct protoent *pe;
	int offset = 0;
	int num = 0;

	list_for_each_entry(e, list, list) {
		if (e->any)
			return NULL;
		if (e->inv != inv)
			continue;
		pe = getprotobynumber(e->protocol);
		if (!pe)
			continue;
		if (len - offset <= 0) {
			error("%s: buf is not big enough\n", __func__);
			break;
		}
		offset += snprintf(buf + offset, len - offset, "%s%s", num ? ", " : "", pe->p_name);
		num++;
	}

	if (num == 1) {
		return buf;
	} else if (num > 1) {
		snprintf(buf + offset, len - offset, " }");
		return buf - 2;
	}

	return NULL;
}

static const char *__print_address(struct clsqos_address *address)
{
	char *p, ip[INET6_ADDRSTRLEN];
	static char buf[INET6_ADDRSTRLEN * 2 + 2];
	size_t rem = sizeof(buf);
	int len;
	int netmask_len;

	p = buf;

	inet_ntop(address->family == FAMILY_V4 ? AF_INET : AF_INET6, &address->address.v4, ip, sizeof(ip));

	len = snprintf(p, rem, "%s", ip);

	if (len < 0 || len >= rem)
		return buf;

	rem -= len;
	p += len;

	if (address->range) {
		inet_ntop(address->family == FAMILY_V4 ? AF_INET : AF_INET6, &address->mask.v4, ip, sizeof(ip));

		snprintf(p, rem, "-%s", ip);
	} else {
		netmask_len = netmask2bitlen(address->family, &address->mask.v6);
		if ((address->family == FAMILY_V4 && netmask_len != 32) ||
		    (address->family == FAMILY_V6 && netmask_len != 128))
			snprintf(p, rem, "/%u", netmask_len);
	}

	return buf;
}

static char *print_address(struct clsqos_rule *rule, struct list_head *list, char *buf, int len, int family, bool inv)
{
	struct clsqos_address *e;
	int offset = 0;
	int num = 0;

	list_for_each_entry(e, list, list) {
		if (e->family != family || e->inv != inv)
			continue;
		if (len - offset <= 0) {
			error("%s: buf is not big enough\n", __func__);
			break;
		}
		offset += snprintf(buf + offset, len - offset, "%s%s", num ? ", " : "", __print_address(e));
		num++;
	}

	if (num == 1) {
		return buf;
	} else if (num > 1) {
		snprintf(buf + offset, len - offset, " }");
		return buf - 2;
	}

	return NULL;
}

static char *print_dscp(struct clsqos_rule *rule, struct list_head *list, char *buf, int len, bool inv)
{
	struct clsqos_dscp *e;
	int offset = 0;
	int num = 0;

	list_for_each_entry(e, list, list) {
		if (e->inv != inv)
			continue;
		if (len - offset <= 0) {
			error("%s: buf is not big enough\n", __func__);
			break;
		}
		offset += snprintf(buf + offset, len - offset, "%s0x%02x", num ? ", " : "", e->dscp);
		num++;
	}

	if (num == 1) {
		return buf;
	} else if (num > 1) {
		snprintf(buf + offset, len - offset, " }");
		return buf - 2;
	}

	return NULL;
}

static char *print_port(struct clsqos_rule *rule, struct list_head *list, char *buf, int len, bool inv)
{
	struct clsqos_port *e;
	int offset = 0;
	int num = 0;

	list_for_each_entry(e, list, list) {
		if (e->inv != inv)
			continue;
		if (len - offset <= 0) {
			error("%s: buf is not big enough\n", __func__);
			break;
		}
		if (e->port_min == e->port_max)
			offset += snprintf(buf + offset, len - offset, "%s%u", num ? ", " : "", e->port_min);
		else
			offset += snprintf(buf + offset, len - offset, "%s%u-%u", num ? ", " : "",
					   e->port_min, e->port_max);
		num++;
	}

	if (num == 1) {
		return buf;
	} else if (num > 1) {
		snprintf(buf + offset, len - offset, " }");
		return buf - 2;
	}

	return NULL;
}

static void print_br_input_rule(FILE *fp, struct clsqos_rule *rule, int br_rule_id)
{
	char _buf[1024] = { "{ " };
	char *buf = &_buf[2];
	int len = sizeof(_buf) - 2;
	const struct clsqos_match *match;
	char *str;
	u64 clsmark = BIT(CLSMARK_BIT_FOR_QOS_BR_RULE0_IDX + br_rule_id);

	fprintf(fp, "        clsmark and 0x%016llx != 0x%016llx ", clsmark, clsmark);

	/* src dev */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_dev(rule, &rule->src_dev, buf, len, match->inv);
		if (str)
			fprintf(fp, "iifname %s%s ", match->operator, str);
	}

	fprintf(fp, "qos-br-rule%d set 1 ", br_rule_id);

	fprintf(fp, "comment \"%s\"\n", rule_name(rule));
}

static void print_br_output_rule(FILE *fp, struct clsqos_rule *rule, int br_rule_id)
{
	char _buf[1024] = { "{ " };
	char *buf = &_buf[2];
	int len = sizeof(_buf) - 2;
	const struct clsqos_match *match;
	char *str;
	u64 clsmark = BIT(CLSMARK_BIT_FOR_QOS_BR_RULE0_IDX + br_rule_id);

	fprintf(fp, "        clsmark and 0x%016llx != 0x%016llx ", clsmark, clsmark);

	/* dest dev */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_dev(rule, &rule->dst_dev, buf, len, match->inv);
		if (str)
			fprintf(fp, "oifname %s%s ", match->operator, str);
	}

	/* dest mac */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_mac(rule, &rule->dst_mac, buf, len, match->inv);
		if (str)
			fprintf(fp, "ether daddr %s%s ", match->operator, str);
	}

	fprintf(fp, "qos-br-rule%d set 1 ", br_rule_id);

	fprintf(fp, "comment \"%s\"\n", rule_name(rule));
}

static void print_inet_fwd_rule(FILE *fp,  struct clsqos_rule *rule, int family, int br_rule_id)
{
	char _buf[1024] = { "{ " };
	char *buf = &_buf[2];
	int len = sizeof(_buf) - 2;
	char *ip_family = "";
	const struct clsqos_match *match;
	char *str;
	union clsmark no_accel = { };

	no_accel.cls_no_accel_for_qos = 1;

	if (family == FAMILY_V4)
		ip_family = "ip";
	else if (family == FAMILY_V6)
		ip_family = "ip6";

	if (rule_has_ops(rule, OPS_SRC_DEV | OPS_DST_DEV | OPS_DST_MAC))
		fprintf(fp, "        qos-br-rule%d 1 ", br_rule_id);
	else
		fprintf(fp, "        ");

	/* src mac */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_mac(rule, &rule->src_mac, buf, len, match->inv);
		if (str)
			fprintf(fp, "ether saddr %s%s ", match->operator, str);
	}

	if (rule_has_ops(rule, OPS_IP_ADDR | OPS_DSCP)) {
		/* src ip */
		for (match = clsqos_matches; match->operator; match++) {
			str = print_address(rule, &rule->src_ip, buf, len, family, match->inv);
			if (str)
				fprintf(fp, "%s saddr %s%s ", ip_family, match->operator, str);
		}

		/* dest ip */
		for (match = clsqos_matches; match->operator; match++) {
			str = print_address(rule, &rule->dst_ip, buf, len, family, match->inv);
			if (str)
				fprintf(fp, "%s daddr %s%s ", ip_family, match->operator, str);
		}

		/* dscp */
		for (match = clsqos_matches; match->operator; match++) {
			str = print_dscp(rule, &rule->dscp, buf, len, match->inv);
			if (str)
				fprintf(fp, "%s dscp %s%s ", ip_family, match->operator, str);
		}
	} else {
		/* family */
		str = print_family(rule, buf, len, family);
		if (str)
			fprintf(fp, "meta nfproto %s ", str);
	}

	/* proto */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_proto(rule, &rule->proto, buf, len, match->inv);
		if (str)
			fprintf(fp, "meta l4proto %s%s ", match->operator, str);
	}

	/* src port */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_port(rule, &rule->src_port, buf, len, match->inv);
		if (str)
			fprintf(fp, "th sport %s%s ", match->operator, str);
	}

	/* dest port */
	for (match = clsqos_matches; match->operator; match++) {
		str = print_port(rule, &rule->dst_port, buf, len, match->inv);
		if (str)
			fprintf(fp, "th dport %s%s ", match->operator, str);
	}

	if (rule->set_vlan8021p.set)
		fprintf(fp, "ofld-pcp set %u ", rule->set_vlan8021p.priority);

	if (rule->set_priority.set)
		fprintf(fp, "ofld-queue set %u ", rule->set_priority.priority);

	if (rule->set_dscp.set)
		fprintf(fp, "ofld-dscp set %u ", rule->set_dscp.dscp);

	if (rule->rate_limit.set) {
		fprintf(fp, "clsmark set clsmark | 0x%016llx ", no_accel.clsmark);
		fprintf(fp, "limit rate over %llu kbytes/second drop ", rule->rate_limit.kbit_per_sec / 8);
	}

	fprintf(fp, "comment \"%s\"\n", rule_name(rule));
}

static void apply_rules(struct list_head *rule_list)
{
	struct clsqos_rule *rule;
	FILE *inet_fwd_nft = NULL, *br_input_nft = NULL, *br_output_nft = NULL;
	static int br_rule_id = -1;
	int nft_rule_num = 0;

	inet_fwd_nft = fopen(NFT_FILE_INET_FWD, "w");
	if (!inet_fwd_nft)
		goto fail;

	br_input_nft = fopen(NFT_FILE_BR_INPUT, "w");
	if (!br_input_nft)
		goto fail;

	br_output_nft = fopen(NFT_FILE_BR_OUTPUT, "w");
	if (!br_output_nft)
		goto fail;

	fprintf(inet_fwd_nft,
		"table inet cls_qos {\n"
		"    chain forward {\n"
		"        type filter hook forward priority filter; policy accept;\n");

	fprintf(br_input_nft,
		"table bridge cls_qos {\n"
		"    chain input {\n"
		"        type filter hook input priority filter; policy accept;\n");

	fprintf(br_output_nft,
		"table bridge cls_qos {\n"
		"    chain output {\n"
		"        type filter hook output priority filter; policy accept;\n");

	list_for_each_entry(rule, rule_list, list) {
		if (rule_has_ops(rule, OPS_SRC_DEV | OPS_DST_DEV | OPS_DST_MAC)) {
			if (++br_rule_id >= QOS_BR_RULE_MAX_NUM) {
				warn_elem(rule->uci_elm, "skipped due to only support %d rules with "
					  "src_device, dest_device or dest_mac option",
					  QOS_BR_RULE_MAX_NUM);
				continue;
			}
		}

		if (rule_has_ops(rule, OPS_SRC_DEV))
			print_br_input_rule(br_input_nft, rule, br_rule_id);
		else if (rule_has_ops(rule, OPS_DST_DEV | OPS_DST_MAC))
			print_br_output_rule(br_output_nft, rule, br_rule_id);

		if (rule->family == FAMILY_ANY) {
			if (rule_has_ops(rule, OPS_SRC_IPV4 | OPS_DST_IPV4))
				print_inet_fwd_rule(inet_fwd_nft, rule, FAMILY_V4, br_rule_id);
			if (rule_has_ops(rule, OPS_SRC_IPV6 | OPS_DST_IPV6))
				print_inet_fwd_rule(inet_fwd_nft, rule, FAMILY_V6, br_rule_id);
			if (!rule_has_ops(rule, OPS_IP_ADDR)) {
				if (rule_has_ops(rule, OPS_DSCP)) {
					print_inet_fwd_rule(inet_fwd_nft, rule, FAMILY_V4, br_rule_id);
					print_inet_fwd_rule(inet_fwd_nft, rule, FAMILY_V6, br_rule_id);
				} else
					print_inet_fwd_rule(inet_fwd_nft, rule, FAMILY_ANY, br_rule_id);
			}
		} else {
			print_inet_fwd_rule(inet_fwd_nft, rule, rule->family, br_rule_id);
		}

		nft_rule_num++;
	}

	fprintf(inet_fwd_nft,
		"    }\n"
		"}");

	fprintf(br_input_nft,
		"    }\n"
		"}");

	fprintf(br_output_nft,
		"    }\n"
		"}");

fail:
	if (inet_fwd_nft)
		fclose(inet_fwd_nft);
	if (br_input_nft)
		fclose(br_input_nft);
	if (br_output_nft)
		fclose(br_output_nft);

	if (nft_rule_num > 0) {
		system("nft -f "NFT_FILE_INET_FWD);
		system("nft -f "NFT_FILE_BR_INPUT);
		system("nft -f "NFT_FILE_BR_OUTPUT);
	} else {
		unlink(NFT_FILE_INET_FWD);
		unlink(NFT_FILE_BR_INPUT);
		unlink(NFT_FILE_BR_OUTPUT);
	}
}

static void usage_and_exit(const char *prog)
{
	fprintf(stderr, "%s [-q] [-k] {start|stop|restart}\n", prog);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct uci_context *uci;
	struct uci_package *uci_clsqos = NULL;
	struct list_head rule_list;
	struct clsqos_rule *rule, *tmp;
	bool quiet = false;
	int opt;
	int cmds = 0;
	const char *cmd = NULL;
	const char *prog = argv[0];
	static struct option long_options[] = {
		{"quiet",	no_argument,		NULL, 'q'},
		{"help",	no_argument,		NULL, 'h'},
		{NULL,		0,			NULL,	0},
	};

	while (1) {
		opt = getopt_long(argc, argv, "qh", long_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'q':
			quiet = true;
			break;

		case 'h':
			usage_and_exit(prog);
			break;
		}
	}

	cmd = argv[optind];

	if (!cmd)
		usage_and_exit(prog);
	else if (!strcmp(cmd, "stop"))
		cmds = CMD_STOP;
	else if (!strcmp(cmd, "start"))
		cmds = CMD_START;
	else if (!strcmp(cmd, "restart"))
		cmds = CMD_STOP | CMD_START;
	else
		usage_and_exit(prog);

	mkdir("/tmp/cls-qos", 0755);
	if (cmds & CMD_STOP) {
		system("nft delete table bridge cls_qos 2>/dev/null");
		system("nft delete table inet cls_qos 2>/dev/null");
		unlink(NFT_FILE_INET_FWD);
		unlink(NFT_FILE_BR_INPUT);
		unlink(NFT_FILE_BR_OUTPUT);
	}

	if (!(cmds & CMD_START))
		return 0;

	if (!access(NFT_FILE_INET_FWD, F_OK)) {
		error("%s has been started, please stop it first\n", argv[0]);
		return 1;
	}

	if (quiet)
		ulog_open(ULOG_SYSLOG, LOG_USER, "cls-qos");
	else
		ulog_open(ULOG_SYSLOG | ULOG_STDIO, LOG_USER, "cls-qos");

	INIT_LIST_HEAD(&rule_list);

	uci = uci_alloc_context();
	if (!uci) {
		error("Out of memory\n");
		return -1;
	}

	if (uci_load(uci, "cls-qos", &uci_clsqos) != UCI_OK) {
		error("uci failed to load cls-qos\n");
		uci_free_context(uci);
		return 0;
	}

	load_rules(&rule_list, uci_clsqos);
	apply_rules(&rule_list);

	list_for_each_entry_safe(rule, tmp, &rule_list, list)
		free_rule(rule);

	uci_free_context(uci);

	return 0;
}
