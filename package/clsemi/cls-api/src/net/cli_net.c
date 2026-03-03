/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#include "clsapi_net.h"
#include "clsapi_cli.h"
#include "autogened_cli_net.h"
#include "net_common.h"
#include <arpa/inet.h>
#include <time.h>

static int cli_get_net_ipv6PrefixAllocMethod(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	enum clsapi_ipv6prefix_allocation allocation = _CLSAPI_IPV6PRE_MAX;

	ret = clsapi_net_get_ipv6PrefixAlloc(interface_name, &allocation);
	switch (allocation) {
		case CLSAPI_IPV6PRE_PD:
			cli_print(output, "IPV6PRE_PD\n");
			break;
		case CLSAPI_IPV6PRE_STATIC:
			cli_print(output, "IPV6PRE_STATIC\n");
			break;
		default:
			cli_print(output, "IPV6PRE_ERROR\n");
	}

	return ret;
}

static int cli_get_net_ipv6AddrAllocMethod(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	enum clsapi_ipv6addr_allocation allocation = _CLSAPI_IPV6ADDR_MAX;

	ret = clsapi_net_get_ipv6AddrAlloc(interface_name, &allocation);
	switch (allocation) {
		case CLSAPI_IPV6ADDR_Manual_Assign:
			cli_print(output, "IPV6ADDR_Manual_Assign\n");
			break;
		case CLSAPI_IPV6ADDR_DHCPv6:
			cli_print(output, "IPV6ADDR_DHCPv6\n");
			break;
		case CLSAPI_IPV6ADDR_SLAAC:
			cli_print(output, "IPV6ADDR_SLAAC\n");
			break;
		default:
			cli_print(output, "IPV6ADDR_ERROR\n");

	}

	return ret;
}

static int cli_get_net_opmode(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	enum clsapi_net_opmode opmode = _CLSAPI_NETWORK_OPMODE_MAX;

	ret = clsapi_net_get_opmode(&opmode);
	if (ret)
		return ret;

	if (opmode >= _CLSAPI_NETWORK_OPMODE_MAX || opmode < CLSAPI_NETWORK_OPMODE_ROUTER)
		return -CLSAPI_ERR_INVALID_PARAM;

	cli_print(output, "%s\n", network_opmode_enum2str(opmode));

	return ret;
}

static int cli_set_net_opmode(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *str_opmode = argv[0];
	enum clsapi_net_opmode opmode = _CLSAPI_NETWORK_OPMODE_MAX;

	opmode = network_opmode_str2enum(str_opmode);
	if (opmode >= _CLSAPI_NETWORK_OPMODE_MAX || opmode < CLSAPI_NETWORK_OPMODE_ROUTER)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_opmode(opmode);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_net_interface(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *device = argv[1];
	const char *str_proto = argv[2];
	enum clsapi_net_protocol proto = _CLSAPI_NETWORK_PROTO_MAX;

	proto = network_proto_str2enum(str_proto);
	if (proto >= _CLSAPI_NETWORK_PROTO_MAX || proto < CLSAPI_NETWORK_PROTO_STATIC)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_add_interface(interface_name, device, proto);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_all_ipv6addrs(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	string_128 ipaddr = {0};
	const char *netif_name = argv[0];
	struct clsapi_ipv6_info *ipaddrs = NULL;

	ret = clsapi_net_get_all_ipv6addrs(netif_name, &ipaddrs);
	if (ret <= 0)
		return ret;

	for (int i = 0; i < ret; i++) {
		inet_ntop(AF_INET6, &ipaddrs[i].addr, ipaddr, sizeof(ipaddr));

		switch (ipaddrs[i].scope) {
			case CLSAPI_ADDR_GLOBAL:
				cli_print(output, "UNI GLOBAL: %s\n", ipaddr);
				break;
			case CLSAPI_ADDR_LINKLOCAL:
				cli_print(output, "LINKLOCAL: %s\n", ipaddr);
				break;
			case CLSAPI_ADDR_ULA_LOCAL:
				cli_print(output, "ULA_LOCAL: %s\n", ipaddr);
				break;
			case CLSAPI_ADDR_OTHERS:
				cli_print(output, "OTHERS: %s\n", ipaddr);
				break;
			default:
				cli_print(output, "ERROR\n");
				break;
		}
	}

	if (ipaddrs)
		free(ipaddrs);

	return CLSAPI_OK;
}

static int cli_get_net_ula_global_ipv6prefix(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t ipv6prefix_len = 0;
	string_256 ipv6prefix = {0};
	const char *netif_name = argv[0];

	ret = clsapi_net_get_ula_global_ipv6prefix(netif_name, ipv6prefix, &ipv6prefix_len);
	if (ret)
		return ret;

	cli_print(output, "prefix: %s, prefix len: %d\n", ipv6prefix, ipv6prefix_len);

	return CLSAPI_OK;
}

static int cli_get_net_uni_global_ipv6prefix(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint8_t ipv6prefix_len = 0;
	string_256 ipv6prefix = {0};
	const char *netif_name = argv[0];

	ret = clsapi_net_get_uni_global_ipv6prefix(netif_name, ipv6prefix, &ipv6prefix_len);
	if (ret)
		return ret;

	cli_print(output, "prefix: %s, prefix len: %d\n", ipv6prefix, ipv6prefix_len);

	return CLSAPI_OK;
}

static int cli_set_net_netmask(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	const char *str_netmask = argv[1];
	struct in_addr netmask;

	if (!inet_pton(AF_INET, str_netmask, &netmask))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_netmask(netif_name, &netmask);
	if (ret)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_netmask(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	struct in_addr netmask;
	string_32 str_netmask = {0};

	ret = clsapi_net_get_netmask(netif_name, &netmask);
	if (ret)
		return ret;

	inet_ntop(AF_INET, &netmask, str_netmask, sizeof(str_netmask));
	cli_print(output, "%s\n", str_netmask);

	return ret;
}

static int cli_get_net_proto(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	enum clsapi_net_protocol proto = _CLSAPI_NETWORK_PROTO_MAX;

	ret = clsapi_net_get_proto(interface_name, &proto);
	if (ret)
		return ret;

	cli_print(output, "%s\n", network_proto_enum2str(proto));

	return ret;
}

static int cli_set_net_proto(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_proto = argv[1];
	enum clsapi_net_protocol proto = _CLSAPI_NETWORK_PROTO_MAX;

	proto = network_proto_str2enum(str_proto);
	if (proto >= _CLSAPI_NETWORK_PROTO_MAX || proto < CLSAPI_NETWORK_PROTO_STATIC)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_proto(interface_name, proto);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_ipv6PrefixOrigin(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	enum clsapi_ipv6_prefix_origin prefix_origin = CLSAPI_PREFIX_ORIGIN_UNKNOW;

	ret = clsapi_net_get_ipv6PrefixOrigin(ifname, &prefix_origin);
	if (ret)
		return ret;

	switch (prefix_origin) {
	case CLSAPI_PREFIX_ORIGIN_UNKNOW:
		cli_print(output, "Unknow\n");
		break;
	case CLSAPI_PREFIX_ORIGIN_PREFIX_DELEGATION:
		cli_print(output, "Prefix Delegation\n");
		break;
	case CLSAPI_PREFIX_ORIGIN_STATIC:
		cli_print(output, "Static\n");
		break;
	}

	return ret;
}

static int cli_set_ipv6PrefixOrigin(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *ifname = argv[0];
	int char2num = atoi(argv[1]);
	enum clsapi_ipv6_prefix_origin prefix_origin = (enum clsapi_ipv6_prefix_origin )char2num;

	ret = clsapi_net_set_ipv6PrefixOrigin(ifname, prefix_origin);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_net_ipaddr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	const char *str_ipaddr = argv[1];
	struct in_addr ipaddr;

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_ipaddr(netif_name, &ipaddr);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_default_gateway(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	string_256 str_ipaddr = {0};
	const char *interface_name = argv[0];
	struct in_addr ipaddr;

	ret = clsapi_net_get_default_gateway(interface_name, &ipaddr);
	if (ret)
		return ret;
	inet_ntop(AF_INET, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
	cli_print(output, "%s\n", str_ipaddr);

	return ret;
}

static int cli_get_net_ipv6_default_gateway(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	string_256 str_ipaddr = {0};
	const char *interface_name = argv[0];
	struct in6_addr ipaddr;

	ret = clsapi_net_get_ipv6_default_gateway(interface_name, &ipaddr);
	if (ret)
		return ret;

	inet_ntop(AF_INET6, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
	cli_print(output, "%s\n", str_ipaddr);

	return ret;
}

static int cli_get_net_ipv6addr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	struct in6_addr ipaddr = {0};
	string_256 str_ipaddr = {0};

	ret = clsapi_net_get_ipv6addr(interface_name, &ipaddr);
	if (ret)
		return ret;

	if (inet_ntop(AF_INET6, &ipaddr, str_ipaddr, sizeof(str_ipaddr)))
		cli_print(output, "%s\n", str_ipaddr);
	else {
		DBG_ERROR("IPv6 address converts error\n");
		return clsapi_cli_report_complete(ret, output);
	}
	return ret;
}

static int cli_get_net_ipaddr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	struct in_addr ipaddr = {0};
	string_256 str_ipaddr = {0};

	ret = clsapi_net_get_ipaddr(interface_name, &ipaddr);
	if (ret)
		return ret;
	inet_ntop(AF_INET, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
	cli_print(output, "%s\n", str_ipaddr);

	return ret;
}

static int cli_get_net_speed(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	enum clsapi_net_speed_type speed = _CLSAPI_NET_SPEED_TYPE_MAX;

	ret = clsapi_net_get_speed(netif_name, &speed);
	if (ret)
		return ret;

	if (speed == _CLSAPI_NET_SPEED_TYPE_MAX)
		return -CLSAPI_ERR_INVALID_DATA;

	cli_print(output, "%d\n", speed);

	return ret;
}

static int cli_del_net_wan_dns_server(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_address = argv[1];
	struct clsapi_net_ipaddr dns_server = {0};

	if (inet_pton(AF_INET, str_address, &dns_server.addr.ipaddr))
		dns_server.in_family = AF_INET;
	else if (inet_pton(AF_INET6, str_address, &dns_server.addr.ipv6addr))
		dns_server.in_family = AF_INET6;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_del_wan_dns_server(interface_name, &dns_server);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_net_wan_dns_server(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_address = argv[1];
	struct clsapi_net_ipaddr dns_server = {0};

	if (inet_pton(AF_INET, str_address, &dns_server.addr.ipaddr))
		dns_server.in_family = AF_INET;
	else if (inet_pton(AF_INET6, str_address, &dns_server.addr.ipv6addr))
		dns_server.in_family = AF_INET6;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_add_wan_dns_server(interface_name, &dns_server);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_wan_dns_server(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int len = 0;
	int ret = CLSAPI_OK;
	string_256 str_address = {0};
	const char *interface_name = argv[0];
	struct clsapi_net_ipaddr *dns_server = NULL;

	ret = clsapi_net_get_wan_dns_server(interface_name, &dns_server);
	if (ret > 0)
		len = ret;
	else
		return ret;

	for (int i = 0; i < len; i++) {
		if (dns_server[i].in_family == AF_INET)
			inet_ntop(AF_INET, &(dns_server[i].addr.ipaddr), str_address, sizeof(str_address));
		else if (dns_server[i].in_family == AF_INET6)
			inet_ntop(AF_INET6, &(dns_server[i].addr.ipv6addr), str_address, sizeof(str_address));
		else {
			if (dns_server)
				free(dns_server);
			return -CLSAPI_ERR_INVALID_DATA;
		}

		cli_print(output, "%s\n", str_address);
	}

	if (dns_server)
		free(dns_server);

	return CLSAPI_OK;
}

static int cli_get_net_firewall_zone_names(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	string_128 zone_names[32] = {0};
	int zone_names_len = ARRAY_SIZE(zone_names);

	ret = clsapi_net_get_firewall_zone_names(zone_names, &zone_names_len);
	if (ret)
		return ret;

	for (int i = 0; i < zone_names_len; i++)
		cli_print(output, "%s\n", zone_names[i]);

	return ret;
}

static int cli_set_firewall_default_policy(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *str_chain = argv[0];
	const char *str_policy = argv[1];
	enum clsapi_firewall_chain chain = _CLSAPI_FIREWALL_CHAIN_MAX;
	enum clsapi_firewall_policy policy = _CLSAPI_FIREWALL_POLICY_MAX;

	chain = firewall_chain_str2enum(str_chain);
	if (chain >= _CLSAPI_FIREWALL_CHAIN_MAX || chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	policy = firewall_policy_str2enum(str_policy);
	if (policy >= _CLSAPI_FIREWALL_POLICY_MAX || policy < CLSAPI_FIREWALL_POLICY_ACCEPT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_firewall_default_policy(chain, policy);
	if (ret)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_firewall_default_policy(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *str_chain = argv[0];
	enum clsapi_firewall_chain chain = _CLSAPI_FIREWALL_CHAIN_MAX;
	enum clsapi_firewall_policy policy = _CLSAPI_FIREWALL_POLICY_MAX;

	chain = firewall_chain_str2enum(str_chain);
	if (chain >= _CLSAPI_FIREWALL_CHAIN_MAX || chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_get_firewall_default_policy(chain, &policy);
	if (ret)
		return ret;

	cli_print(output, "%s\n", firewall_policy_enum2str(policy));

	return ret;
}

static int cli_set_net_firewall_zone_policy(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *zone_name = argv[0];
	const char *str_chain = argv[1];
	const char *str_policy = argv[2];
	enum clsapi_firewall_chain chain = _CLSAPI_FIREWALL_CHAIN_MAX;
	enum clsapi_firewall_policy policy = _CLSAPI_FIREWALL_POLICY_MAX;

	chain = firewall_chain_str2enum(str_chain);
	if (chain >= _CLSAPI_FIREWALL_CHAIN_MAX || chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	policy = firewall_policy_str2enum(str_policy);
	if (policy >= _CLSAPI_FIREWALL_POLICY_MAX || policy < CLSAPI_FIREWALL_POLICY_ACCEPT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_firewall_zone_policy(zone_name, chain, policy);
	if (ret)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_firewall_zone_policy(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *zone_name = argv[0];
	const char *str_chain = argv[1];
	enum clsapi_firewall_chain chain = _CLSAPI_FIREWALL_CHAIN_MAX;
	enum clsapi_firewall_policy policy = _CLSAPI_FIREWALL_POLICY_MAX;

	chain = firewall_chain_str2enum(str_chain);
	if (chain >= _CLSAPI_FIREWALL_CHAIN_MAX || chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_get_firewall_zone_policy(zone_name, chain, &policy);
	if (ret)
		return ret;

	cli_print(output, "%s\n", firewall_policy_enum2str(policy));

	return ret;
}

static int cli_get_net_firewall_zone_network(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *zone_name = argv[0];
	clsapi_ifname *network_name = NULL;
	int network_name_len = 0;

	ret = clsapi_net_get_firewall_zone_network(zone_name, &network_name);
	if (ret > 0)
		network_name_len  = ret;
	else
		return ret;

	for (int i = 0; i < network_name_len; i++)
		cli_print(output, "%s\n", network_name[i]);

	if (network_name)
		free(network_name);

	return CLSAPI_OK;
}

static inline int get_param_number(char *str)
{
	int count = 0;

	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] == ',') {
			count++;
		}
	}

	return count + 1;
}

static int cli_del_net_firewall_rule(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *rule_name = argv[0];
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;

	set_fw_rule_name(prule, rule_name);

	ret = clsapi_net_del_firewall_rule(prule);

	return ret;
}

static int cli_get_net_firewall_rule(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *rule_name = argv[0];
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;

	set_fw_rule_name(prule, rule_name);

	ret = clsapi_net_get_firewall_rule(prule);

	//src
	if (prule->flag & 0x2) {
		cli_print(output, "src: %s\n", prule->src);
	}
	// src_ipaddr
	if (prule->flag & 0x4) {
		string_32 ipaddr = {0};

		for (int i = 0; i < prule->src_ipaddr_num; i++) {
			inet_ntop(AF_INET, &prule->src_ipaddr[i], ipaddr, sizeof(ipaddr));
			cli_print(output, "src ipaddr: %s\n", ipaddr);
		}
		free(prule->src_ipaddr);
	}
	// src_macaddr
	if (prule->flag & 0x8) {
		for (int i = 0; i < prule->src_macaddr_num; i++) {
			cli_print(output, "src macaddr:"MACFMT"\n", MACARG(prule->src_macaddr[i]));
		}
		free(prule->src_macaddr);
	}
	//src port
	if (prule->flag & 0x10) {
		string_32 port = {0};

		cli_print(output, "src port:");
		for (int i = 0; i < prule->src_port_num; i++) {
			snprintf(port, sizeof(port), "%d", prule->src_port[i]);
			if (i != prule->src_port_num - 1)
			cli_print(output, "%s,", port);
			else
			cli_print(output, "%s", port);
		}
		free(prule->src_port);
		cli_print(output, "\n");
	}
	// protocol
	if (prule->flag & 0x20) {

		cli_print(output, "protocol:");
		for (int i = 0; i < prule->proto_num; i++)
			if (i != prule->proto_num - 1)
				cli_print(output, "%s,", prule->proto[i]);
			else
				cli_print(output, "%s", prule->proto[i]);

		free(prule->proto);
		cli_print(output, "\n");
	}
	//icmp type
	if (prule->flag & 0x40) {
		for (int i = 0; i < ARRAY_SIZE(prule->icmp_type); i++) {
			if (strcmp(prule->icmp_type[i], "\0") != 0)
				cli_print(output, "icmp type: %s\n", prule->icmp_type[i]);
		}
	}
	//dest
	if (prule->flag & 0x80) {
		cli_print(output, "dest: %s\n", prule->dest);
	}
	//dest ipaddr
	if (prule->flag & 0x100) {
		string_32 ipaddr = {0};

		for (int i = 0; i < prule->dest_ipaddr_num; i++) {
			inet_ntop(AF_INET, &prule->dest_ipaddr[i], ipaddr, sizeof(ipaddr));
			cli_print(output, "dest ipaddr: %s\n", ipaddr);
		}
		free(prule->dest_ipaddr);
	}
	//dest_port
	if (prule->flag & 0x200) {
		string_32 port = {0};

		cli_print(output, "dest port:");
		for (int i = 0; i < prule->dest_port_num; i++) {
			snprintf(port, sizeof(port), "%d", prule->dest_port[i]);
			if (i != prule->dest_port_num - 1)
				cli_print(output, "%s,", port);
			else
				cli_print(output, "%s", port);
		}
		free(prule->dest_port);
		cli_print(output, "\n");
	}
	//mark
	if (prule->flag & 0x400) {
		cli_print(output, "mark: %s\n", prule->mark);
	}
	//start date
	if (prule->flag & 0x800) {
		cli_print(output, "start_date: %s\n", prule->start_date);
	}
	//start time
	if (prule->flag & 0x1000) {
		cli_print(output, "start_time: %s\n", prule->start_time);
	}
	//stop date
	if (prule->flag & 0x2000) {
		cli_print(output, "stop_date: %s\n", prule->stop_date);
	}
	//stop time
	if (prule->flag & 0x4000) {
		cli_print(output, "stop_time: %s\n", prule->stop_time);
	}
	//weekdays
	if (prule->flag & 0x8000) {
		cli_print(output, "weekdays:\n");
		for (int i = 0; i < ARRAY_SIZE(prule->weekdays); i++) {
			if (strcmp(prule->weekdays[i], "\0") != 0)
				cli_print(output, "%s\n", prule->weekdays[i]);
		}
		cli_print(output, "\n");
	}
	//monthdays
	if (prule->flag & 0x10000) {
				cli_print(output, "monthdays:");
		for (int i = 0; i < ARRAY_SIZE(prule->monthdays); i++) {
			if (strcmp(prule->monthdays[i], "\0") != 0)
				cli_print(output, "%s,", prule->monthdays[i]);
		}
		cli_print(output, "\n");
	}
	//utc time
	if (prule->flag & 0x20000) {
		if (prule->utc_time)
			cli_print(output, "utc_time: enabled\n");
		else
			cli_print(output, "utc_time: disabled\n");
	}
	//target
	if (prule->flag & 0x40000) {
		string_32 dummy = {0};

		switch (prule->target) {
			case CLSAPI_FIREWALL_TARGET_ACCEPT:
				cli_print(output, "target: ACCEPT\n");
				break;
			case CLSAPI_FIREWALL_TARGET_DROP:
				cli_print(output, "target: DROP\n");
				break;
			case CLSAPI_FIREWALL_TARGET_REJECT:
				cli_print(output, "target: REJECT\n");
				break;
			case CLSAPI_FIREWALL_TARGET_NOTRACK:
				cli_print(output, "target: NOTRACK\n");
				break;
			case CLSAPI_FIREWALL_TARGET_XMARK:
				cli_print(output, "target: XMARK\n");
				snprintf(dummy, sizeof(dummy), "%ld", prule->target_option.set_xmark);
				cli_print(output, "target option: %s\n", dummy);
				break;
			case CLSAPI_FIREWALL_TARGET_MARK:
				cli_print(output, "target: MARK\n");
				snprintf(dummy, sizeof(dummy), "%ld", prule->target_option.set_mark);
				cli_print(output, "target option: %s\n", dummy);
				break;
			case CLSAPI_FIREWALL_TARGET_HELPER:
				cli_print(output, "target: HELPER\n");
				cli_print(output, "target option: %s\n", prule->target_option.set_helper);
				break;
			case CLSAPI_FIREWALL_TARGET_DSCP:
				cli_print(output, "target: DSCP\n");
				cli_print(output, "target option: %s\n", prule->target_option.set_dscp);
				break;
			default:
				break;
		}
	}
	//family
	if (prule->flag & 0x80000) {
		cli_print(output, "family: %s\n", prule->family);
	}
	//limit
	if (prule->flag & 0x100000) {
		cli_print(output, "limit: %s\n", prule->limit);
	}
	//limit burst
	if (prule->flag & 0x200000) {
		string_32 dummy = {0};

		snprintf(dummy, sizeof(dummy), "%d", prule->limit_burst);
		cli_print(output, "limit: %s\n", dummy);
	}
	//enabled
	if (prule->flag & 0x400000) {
		if (prule->enabled)
			cli_print(output, "enabled\n");
		else
			cli_print(output, "disabled\n");
	}
	//device + (direction)
	if (prule->flag & 0x800000) {

		cli_print(output, "device: %s\n", prule->device);
		cli_print(output, "direction: %s\n", prule->direction);
	}
	// dscp
	if (prule->flag & 0x1000000) {
		cli_print(output, "dscp: %s\n", prule->dscp);
	}
	// helper
	if (prule->flag & 0x2000000) {
		cli_print(output, "helper: %s\n", prule->helper);
	}

	return ret;
}

static int cli_set_net_firewall_rule_timer(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;
	bool utc_time = true;
	string_32 weekdays[7] = {0};
	string_32 monthdays[31] = {0};
	char *token = ",";
	char *tmp = NULL;
	int i = 0;

	const char *rule_name = argv[0];
	char *start_date = argv[1];
	char *start_time = argv[2];
	char *stop_date = argv[3];
	char *stop_time = argv[4];
	char *str_weekdays = argv[5];
	char *str_monthdays = argv[6];
	char *str_utc_time = argv[7];

	set_fw_rule_name(prule, rule_name);

	tmp = strtok(str_weekdays, token);
	i = 0;
	while (tmp != NULL) {
		if (i >= ARRAY_SIZE(weekdays))
			return -CLSAPI_ERR_INVALID_PARAM;
		cls_strncpy(weekdays[i], tmp, sizeof(weekdays[i]));
		tmp = strtok(NULL, token);
		i++;
	}
	set_fw_rule_weekdays(prule, weekdays);

	tmp = strtok(str_monthdays, token);
	i = 0;
	while (tmp != NULL) {
		if (i >= ARRAY_SIZE(monthdays))
			return -CLSAPI_ERR_INVALID_PARAM;
		cls_strncpy(monthdays[i], tmp, sizeof(monthdays[i]));
		tmp = strtok(NULL, token);
		i++;
	}
	set_fw_rule_monthdays(prule, monthdays);

	if (strcmp(str_utc_time, "enable") == 0)
		utc_time = true;
	else if (strcmp(str_utc_time, "disable") == 0)
		utc_time = false;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	set_fw_rule_utc_time(prule, utc_time);

	set_fw_rule_start_date(prule, start_date);

	set_fw_rule_start_time(prule, start_time);

	set_fw_rule_stop_date(prule, stop_date);

	set_fw_rule_stop_time(prule, stop_time);

	ret = clsapi_net_set_firewall_rule(prule);

	return ret;
}

static int cli_set_net_firewall_rule_advanced(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;
	char *token = ",";
	char *tmp = NULL;
	int i = 0;
	string_32 icmp_type[128] = {0};

	const char *rule_name = argv[0];
	char *str_icmp_type = argv[1];
	char *family = argv[2];
	char *limit = argv[3];
	char *limit_burst = argv[4];
	char *device = argv[5];
	char *direction = argv[6];
	char *mark = argv[7];
	char *dscp = argv[8];
	char *helper = argv[9];

	set_fw_rule_name(prule, rule_name);

	tmp = strtok(str_icmp_type, token);
	i = 0;
	while (tmp != NULL) {
		if (i >= ARRAY_SIZE(icmp_type))
			return -CLSAPI_ERR_INVALID_PARAM;
		cls_strncpy(icmp_type[i], tmp, sizeof(icmp_type[i]));
		tmp = strtok(NULL, token);
		i++;
	}
	set_fw_rule_icmp_type(prule, icmp_type);

	set_fw_rule_family(prule, family);

	set_fw_rule_limit(prule, limit);

	if (atoi(limit_burst)) {
		if (!prule->limit)
			return -CLSAPI_ERR_INVALID_PARAM;
		set_fw_rule_limit_burst(prule, atoi(limit_burst));
	} else
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!direction)
		return -CLSAPI_ERR_INVALID_PARAM;

	set_fw_rule_device(prule, device);
	cls_strncpy(prule->direction, direction, sizeof(prule->direction));

	set_fw_rule_mark(prule, mark);

	set_fw_rule_dscp(prule, dscp);

	set_fw_rule_helper(prule, helper);

	ret = clsapi_net_set_firewall_rule(prule);

	return ret;
}

static int cli_set_net_firewall_rule(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;
	bool enabled = true;
	char *token = ",";
	char *tmp = NULL;
	int i = 0;

	const char *rule_name = argv[0];
	char *str_enabled = argv[1];
	const char *src = argv[2];
	char *str_src_ipaddr = argv[3];
	char *str_src_macaddr = argv[4];
	char *str_src_port = argv[5];
	char *str_proto = argv[6];
	char *dest = argv[7];
	char *str_dest_ipaddr = argv[8];
	char *str_dest_port = argv[9];
	char *target = argv[10];
	char *target_option = argv[11];

	set_fw_rule_name(prule, rule_name);

	if (strcmp(str_enabled, "enable") == 0)
		enabled = true;
	else if (strcmp(str_enabled, "disable") == 0)
		enabled = false;
	else
		return -CLSAPI_ERR_INVALID_PARAM;
	set_fw_rule_enabled(prule, enabled);

	set_fw_rule_src(prule, src);
	if (strcmp(target, "ACCEPT") == 0)
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_ACCEPT);
	else if (strcmp(target, "REJECT") == 0 )
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_REJECT);
	else if (strcmp(target, "DROP") == 0)
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_DROP);
	else if (strcmp(target, "NOTRACK") == 0)
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_NOTRACK);
	//need extra option
	else if (strcmp(target, "MARK") == 0) {
		uint64_t set_mark = 0;

		if (target_option[0] == '0' && (target_option[1] == 'x' || target_option[1] == 'X'))
			set_mark = strtol(target_option, NULL, 16);
		else
			set_mark = strtol(target_option, NULL, 10);
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_MARK);
		prule->target_option.set_mark = set_mark;
	} else if (strcmp(target, "XMARK") == 0) {
		uint64_t set_xmark = 0;

		if (target_option[0] == '0' && (target_option[1] == 'x' || target_option[1] == 'X'))
			set_xmark = strtol(target_option, NULL, 16);
		else
			set_xmark = strtol(target_option, NULL, 10);
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_XMARK);
		prule->target_option.set_xmark = set_xmark;
	} else if (strcmp(target, "HELPER") == 0) {
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_HELPER);
		cls_strncpy(prule->target_option.set_helper, target_option, sizeof(prule->target_option.set_helper));
	} else if (strcmp(target, "DSCP") == 0) {
		set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_DSCP);
		cls_strncpy(prule->target_option.set_dscp, target_option, sizeof(prule->target_option.set_dscp));
	} else
		return -CLSAPI_ERR_INVALID_PARAM;

	prule->src_ipaddr_num = get_param_number(str_src_ipaddr);
	prule->src_ipaddr = (struct in_addr *)calloc(sizeof(struct in_addr), prule->src_ipaddr_num);
	tmp = strtok(str_src_ipaddr, token);
	i = 0;
	while (tmp != NULL) {
		if(!inet_pton(AF_INET, tmp, &prule->src_ipaddr[i])) {
			ret = -CLSAPI_ERR_INVALID_PARAM;
			goto out;
		}
		tmp = strtok(NULL, token);
		i++;
	}
	prule->flag |= (1 << 2);

	prule->src_macaddr_num = get_param_number(str_src_macaddr);
	prule->src_macaddr = (uint8_t (*)[ETH_ALEN])calloc(sizeof(uint8_t [ETH_ALEN]), prule->src_macaddr_num);
	tmp = strtok(str_src_macaddr, token);
	i = 0;
	while (tmp != NULL) {
		mac_aton(tmp, prule->src_macaddr[i]);
		tmp = strtok(NULL, token);
		i++;
	}
	prule->flag |= (1 << 3);

	prule->src_port_num = get_param_number(str_src_port);
	prule->src_port = (uint32_t *)calloc(sizeof(uint32_t), prule->src_port_num);
	tmp = strtok(str_src_port, token);
	i = 0;
	while (tmp != NULL) {
		if ((prule->src_port[i] = atoi(tmp)) == 0) {
			ret = -CLSAPI_ERR_INVALID_PARAM;
			goto out;
		}
		tmp = strtok(NULL, token);
		i++;
	}
	prule->flag |= (1 << 4);

	prule->proto_num = get_param_number(str_proto);
	prule->proto = (string_32 *)calloc(sizeof(string_32), prule->proto_num);
	tmp = strtok(str_proto, token);
	i = 0;
	while (tmp != NULL) {
		cls_strncpy(prule->proto[i], tmp, sizeof(string_32));
		tmp = strtok(NULL, token);
		i++;
	}
	prule->flag |= (1 << 5);

	set_fw_rule_dest(prule, dest);

	prule->dest_port_num = get_param_number(str_dest_port);
	prule->dest_port = (uint32_t *)calloc(sizeof(uint32_t), prule->dest_port_num);
	tmp = strtok(str_dest_port, token);
	i = 0;
	while (tmp != NULL) {
		if ((prule->dest_port[i] = atoi(tmp)) == 0) {
			ret = -CLSAPI_ERR_INVALID_PARAM;
			goto out;
		}
		tmp = strtok(NULL, token);
		i++;
	}
	prule->flag |= (1 << 9);

	prule->dest_ipaddr_num = get_param_number(str_dest_ipaddr);
	prule->dest_ipaddr = (struct in_addr *)calloc(sizeof(struct in_addr), prule->dest_ipaddr_num);

	tmp = strtok(str_dest_ipaddr, token);
	i = 0;
	while (tmp != NULL) {
		if(!inet_pton(AF_INET, tmp, &prule->dest_ipaddr[i])) {
			ret = -CLSAPI_ERR_INVALID_PARAM;
			goto out;
		}
		tmp = strtok(NULL, token);
		i++;
	}
	prule->flag |= (1 << 8);

	ret = clsapi_net_set_firewall_rule(prule);

out:
	if (prule->src_ipaddr)
		free(prule->src_ipaddr);
	if (prule->src_macaddr)
		free(prule->src_macaddr);
	if (prule->src_port)
		free(prule->src_port);
	if (prule->proto)
		free(prule->proto);
	if (prule->dest_port)
		free(prule->dest_port);
	if (prule->dest_ipaddr)
		free(prule->dest_ipaddr);

	return ret;
}

static int cli_set_net_static_route_gateway(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_ipaddr = argv[1];
	struct in_addr ipaddr;
	struct in_addr netmask;
	struct in_addr gateway;
	const char *str_netmask = argv[2];
	const char *str_gateway = argv[3];

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr) || !inet_pton(AF_INET, str_netmask, &netmask) || !inet_pton(AF_INET, str_gateway, &gateway))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_static_route_gateway(interface_name, &ipaddr, &netmask, &gateway);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_static_route_gateway(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_ipaddr = argv[1];
	struct in_addr ipaddr;
	const char *str_netmask = argv[2];
	string_32 str_gateway = {0};
	struct in_addr gateway;
	struct in_addr netmask;

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr) || !inet_pton(AF_INET, str_netmask, &netmask))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_get_static_route_gateway(interface_name, &ipaddr, &netmask, &gateway);
	if (ret < 0)
		return ret;
	if (!inet_ntop(AF_INET,  &gateway, str_gateway, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	cli_print(output, "Gateway: %s\n", str_gateway);

	return ret;
}

static int cli_del_net_static_route_gateway(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_ipaddr = argv[1];
	const char *str_netmask = argv[2];
	struct in_addr ipaddr;
	struct in_addr netmask;

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr) || !inet_pton(AF_INET, str_netmask, &netmask))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_del_static_route_gateway(interface_name, &ipaddr, &netmask);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_net_static_route(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_ipaddr = argv[1];
	struct in_addr ipaddr;
	struct in_addr netmask;
	const char *str_netmask = argv[2];

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr) || !inet_pton(AF_INET, str_netmask, &netmask))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_del_static_route(interface_name, &ipaddr, &netmask);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_net_static_route(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = argv[0];
	const char *str_ipaddr = argv[1];
	struct in_addr ipaddr;
	struct in_addr netmask;
	const char *str_netmask = argv[2];

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr) || !inet_pton(AF_INET, str_netmask, &netmask))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_add_static_route(interface_name, &ipaddr, &netmask);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_dhcp_lease_info(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct dhcp_lease_info info[255];
	int number_lease = ARRAY_SIZE(info);
	time_t timestamp = 0;
	char *time_str = NULL;

	ret = clsapi_net_get_dhcp_lease_info(info, &number_lease);
	if (ret < 0)
		return ret;

	cli_print(output, "%32s\t%16s\t%17s\t%31s\n", "Hostname", "IP", "MAC", "Lease time");
	for (int i = 0; i < number_lease; i++) {
		cli_print(output, "%32s\t", info[i].hostname);
		cli_print(output, "%16s\t", inet_ntoa(info[i].ipaddr));
		cli_print(output, MACFMT"\t", MACARG(info[i].macaddr));
		timestamp = info[i].expires;
		time_str = ctime(&timestamp);
		cli_print(output, "%32s", time_str);
	}

	return ret;
}

static int cli_del_net_dns_server_addr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	const char *ipaddr = argv[1];
	struct in_addr addr;

	if (!inet_pton(AF_INET, ipaddr, &addr))
		return -CLSAPI_ERR_INVALID_PARAM;
	ret = clsapi_net_del_dns_server_addr(netif_name, &addr);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_net_dns_server_addr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	const char *ipaddr = argv[1];
	struct in_addr addr;

	if (!inet_pton(AF_INET, ipaddr, &addr))
		return -CLSAPI_ERR_INVALID_PARAM;
	ret = clsapi_net_add_dns_server_addr(netif_name, &addr);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_dns_server_addr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = argv[0];
	struct in_addr *server_addr = NULL;
	int length = 0;

	ret = clsapi_net_get_dns_server_addr(netif_name, &server_addr);
	if (ret < 0)
		return ret;
	else if (ret == 0)
		return -CLSAPI_ERR_NOT_FOUND;

	length = ret;

	for (int i = 0; i < length; i++)
		cli_print(output, "%s\n", inet_ntoa(server_addr[i]));

	if (server_addr)
		free(server_addr);

	return CLSAPI_OK;
}

static int cli_get_net_addr_pool(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char *netif_name = argv[0];
	uint32_t start = 0;
	uint32_t max_num = 0;

	ret = clsapi_net_get_dhcp_addr_pool(netif_name, &start, &max_num);
	if (ret < 0)
		return ret;

	cli_print(output, "start offset: %u\n", start);
	cli_print(output, "maximum numbers: %u\n", max_num);

	return ret;
}

static int cli_add_net_dns_domain_binding(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	const char *domain_name = argv[0];
	const char *str_ipaddr = argv[1];
	struct in_addr ipaddr;
	int ret = CLSAPI_OK;

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr))
		return -CLSAPI_ERR_INVALID_PARAM;
	ret = clsapi_net_add_dns_domain_binding(domain_name, &ipaddr);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_net_dns_domain_binding(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	const char *domain_name = argv[0];
	const char * str_ipaddr = argv[1];
	struct in_addr ipaddr;
	int ret = CLSAPI_OK;

	if (!inet_pton(AF_INET, str_ipaddr, &ipaddr))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_set_dns_domain_binding(domain_name, &ipaddr);
	if (ret < 0)
		return ret;

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_net_dns_domain_binding(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	char *domain_name = argv[0];
	int ret = CLSAPI_OK;
	string_32 str_ipaddr = {0};
	struct in_addr ipaddr;

	ret = clsapi_net_get_dns_domain_binding(domain_name, &ipaddr);
	if (ret < 0)
		return ret;

	if (!inet_ntop(AF_INET, &ipaddr, str_ipaddr, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_DATA;

	cli_print(output, "IP: %s\n", str_ipaddr);

	return ret;

}
static int cli_get_net_macaddr(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char *netif_name = argv[0];
	uint8_t macaddr[ETH_ALEN];

	ret = clsapi_net_get_macaddr(netif_name, macaddr);
	if (ret < 0)
		return ret;

	cli_print(output, MACFMT"\n", MACARG(macaddr));

	return ret;
}


static int cli_get_net_statistics(struct clsapi_cli_output *output,
		struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_net_statistics netstat;
	char *netif_name = argv[0];

	ret = clsapi_net_get_stats(netif_name, &netstat);
	if (ret < 0)
		return ret;

	cli_print(output, "interface :%s, statistics:\n", netif_name);
	cli_print(output, "\tcollisions: %lu\n", netstat.rx_frame_errors);
	cli_print(output, "\trx_frame_errors: %lu\n", netstat.tx_compressed);
	cli_print(output, "\ttx_compressed: %lu\n", netstat.multicast);
	cli_print(output, "\tmulticast: %lu\n", netstat.multicast);
	cli_print(output, "\trx_length_errors: %lu\n", netstat.rx_length_errors);
	cli_print(output, "\ttx_dropped: %lu\n", netstat.tx_dropped);
	cli_print(output, "\trx_bytes: %lu\n", netstat.rx_bytes);
	cli_print(output, "\trx_missed_errors: %lu\n", netstat.rx_missed_errors);
	cli_print(output, "\ttx_errors: %lu\n", netstat.tx_errors);
	cli_print(output, "\trx_compressed: %lu\n", netstat.rx_compressed);
	cli_print(output, "\trx_over_errors: %lu\n", netstat.rx_over_errors);
	cli_print(output, "\ttx_fifo_errors: %lu\n", netstat.tx_fifo_errors);
	cli_print(output, "\trx_crc_errors: %lu\n", netstat.rx_crc_errors);
	cli_print(output, "\trx_packets: %lu\n", netstat.rx_packets);
	cli_print(output, "\ttx_heartbeat_errors: %lu\n", netstat.tx_heartbeat_errors);
	cli_print(output, "\trx_dropped: %lu\n", netstat.rx_dropped);
	cli_print(output, "\ttx_aborted_errors: %lu\n", netstat.tx_aborted_errors);
	cli_print(output, "\ttx_packets: %lu\n", netstat.tx_packets);
	cli_print(output, "\trx_errors: %lu\n", netstat.rx_errors);
	cli_print(output, "\ttx_bytes: %lu\n", netstat.tx_bytes);
	cli_print(output, "\ttx_window_errors: %lu\n", netstat.tx_window_errors);
	cli_print(output, "\trx_fifo_errors: %lu\n", netstat.rx_fifo_errors);
	cli_print(output, "\ttx_carrier_errors: %lu\n", netstat.tx_carrier_errors);

	return ret;
}

static struct clsapi_cli_entry clsapi_cli_entry_net_manual[] = {
	{"get macaddr", 1, 1, "<netif_name>", cli_get_net_macaddr},
	{"get stats", 1, 1, "<netif_name>", cli_get_net_statistics},
	{"get dns_server_addr", 1, 1, "<netif_name>", cli_get_net_dns_server_addr},
	{"add dns_server_addr", 2, 2, "<netif_name> <server addr>", cli_add_net_dns_server_addr},
	{"del dns_server_addr", 2, 2, "<netif_name> <server addr>", cli_del_net_dns_server_addr},
	{"get dhcp_lease_info", 0, 0, "", cli_get_net_dhcp_lease_info},
	{"get dhcp_addr_pool", 1, 1, "<netif_name>", cli_get_net_addr_pool},
	{"add static_route", 3, 3, "<interface name> <ipaddr> <netmask>", cli_add_net_static_route},
	{"del static_route", 3, 3, "<interface name> <ipaddr> <netmask>", cli_del_net_static_route},
	{"del static_route_gateway", 3, 3, "<interface name> <ipaddr> <netmask>", cli_del_net_static_route_gateway},
	{"get static_route_gateway", 3, 3, "<interface name> <ipaddr> <netmask>", cli_get_net_static_route_gateway},
	{"set static_route_gateway", 4, 4, "<interface name> <ipaddr> <netmask> <gateway>",
		cli_set_net_static_route_gateway},
	{"get dns_domain_binding", 1, 1, "<domain_name>", cli_get_net_dns_domain_binding},
	{"set dns_domain_binding", 2, 2, "<domain_name> <ipaddr>", cli_set_net_dns_domain_binding},
	{"add dns_domain_binding", 2, 2, "<domain_name> <ipaddr>", cli_add_net_dns_domain_binding},
	{"get firewall_zone_names", 0, 0, "", cli_get_net_firewall_zone_names},
	{"get firewall_zone_network", 1, 1, "<zone_name>", cli_get_net_firewall_zone_network},
	{"set firewall_zone_policy", 3, 3, "<zone_name> <chain> <policy>", cli_set_net_firewall_zone_policy},
	{"get firewall_zone_policy", 2, 2, "<zone_name> <chain>", cli_get_net_firewall_zone_policy},
	{"get wan_dns_server", 1, 1, "<interface_name>", cli_get_net_wan_dns_server},
	{"add wan_dns_server", 2, 2, "<interface_name> <server_address>", cli_add_net_wan_dns_server},
	{"del wan_dns_server", 2, 2, "<interface_name> <server_address>", cli_del_net_wan_dns_server},
	{"get ipaddr", 1, 1, "<netif_name>", cli_get_net_ipaddr},
	{"get ipv6addr", 1, 1, "<netif_name>", cli_get_net_ipv6addr},
	{"get ula_global_ipv6prefix", 1, 1, "<netif_name>", cli_get_net_ula_global_ipv6prefix},
	{"get uni_global_ipv6prefix", 1, 1, "<netif_name>", cli_get_net_uni_global_ipv6prefix},
	{"get all_ipv6addrs", 1, 1, "<netif_name>", cli_get_net_all_ipv6addrs},
	{"get default_gateway", 1, 1, "<netif_name>", cli_get_net_default_gateway},
	{"get ipv6_default_gateway", 1, 1, "<netif_name>", cli_get_net_ipv6_default_gateway},
	{"get speed", 1, 1, "<netif_name>", cli_get_net_speed},
	{"set firewall_default_policy", 2, 2, "<chain> <policy>", cli_set_firewall_default_policy},
	{"get firewall_default_policy", 1, 1, "<chain>", cli_get_firewall_default_policy},
	{"set ipaddr", 2, 2, "<netif_name> <ipaddr>", cli_set_net_ipaddr},
	{"set proto", 2, 2, "<interface_name> <proto>", cli_set_net_proto},
	{"get proto", 1, 1, "<interface_name>", cli_get_net_proto},
	{"get netmask", 1, 1, "<netif_name>", cli_get_net_netmask},
	{"set netmask", 2, 2, "<netif_name> <netmask>", cli_set_net_netmask},
	{"set opmode", 1, 1, "<opmode: router/bridge/repeater/repeater_5g>", cli_set_net_opmode},
	{"get opmode", 0, 0, "", cli_get_net_opmode},
	{"add interface", 3, 3, "<interface_name> <device> <proto>", cli_add_net_interface},
	{"set firewall_rule", 11, 12, "<rule name> <enabled> <src> <src_ipaddr1,src_ipaddr2> <src_macaddr1,src_macaddr2> <src_port1,src_port2> <proto1,proto2> <dest> <dest_ipaddr1,dest_ipaddr2> <dest_port1,dest_port2> <target> <target_option>", cli_set_net_firewall_rule},
	{"set firewall_rule_advanced", 10, 10, "<rule name> <icmp_type1,icmp_type2> <family> <limit> <limit burst> <device> <direction> <mark> <dscp> <helper>", cli_set_net_firewall_rule_advanced},
	{"set firewall_rule_timer", 8, 8, "<rule name> <start date> <start time> <stop date> <stop time> <Sun,Mon...> <1,2,3...> <utc_time> ", cli_set_net_firewall_rule_timer},
	{"del firewall_rule", 1, 1, "<rule name>", cli_del_net_firewall_rule},
	{"get firewall_rule", 1, 1, "<rule name>", cli_get_net_firewall_rule},
	{"get ipv6AddrAlloc", 1, 1, "<interface name>", cli_get_net_ipv6AddrAllocMethod},
	{"get ipv6PrefixAlloc", 1, 1, "<interface name>", cli_get_net_ipv6PrefixAllocMethod},
	{"get ipv6PrefixOrigin", 1, 1, "<netif_name>", cli_get_ipv6PrefixOrigin},
	{"set ipv6PrefixOrigin", 2, 2, "<netif_name> <prefix origin: static: 0, prefix delegation: 1>", cli_set_ipv6PrefixOrigin},
	{}
};


int clsapi_cli_net_init(struct clsapi_cli_entry *cli_entry_tbl[])
{
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_NET_AUTO] = clsapi_cli_entry_net_auto;
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_NET_MANUAL] = clsapi_cli_entry_net_manual;

	return CLSAPI_OK;
}


struct clsapi_cli_plugin clsapi_cli_plugin = {
	.init = clsapi_cli_net_init
};
