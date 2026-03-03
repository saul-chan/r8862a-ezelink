/* Automatically generated file; DO NOT EDIT. */
/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _AUTOGENED_CLI_NET_H
#define _AUTOGENED_CLI_NET_H

#include <stdio.h>
#include <stdlib.h>

static int cli_enable_dhcp_server(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = (argv[0]);
	const bool enable = atol(argv[1]);

	ret = clsapi_net_enable_dhcp_server(netif_name, enable);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_dhcp_server_enabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = (argv[0]);
	bool enable = 0;

	ret = clsapi_net_get_dhcp_server_enabled(netif_name, &enable);

	return clsapi_cli_report_int_value(ret, output, enable);
}

static int cli_set_dhcp_addr_pool(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *netif_name = (argv[0]);
	const uint32_t start_offset = atol(argv[1]);
	const uint32_t max_lease = atol(argv[2]);

	ret = clsapi_net_set_dhcp_addr_pool(netif_name, start_offset, max_lease);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_dns_domain_suffix(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	char dns_suffix[65] = {0};

	ret = clsapi_net_get_dns_domain_suffix(dns_suffix);

	return clsapi_cli_report_str_value(ret, output, dns_suffix);
}

static int cli_get_pppoe_username(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = (argv[0]);
	char username[33] = {0};

	ret = clsapi_net_get_pppoe_username(interface_name, username);

	return clsapi_cli_report_str_value(ret, output, username);
}

static int cli_get_pppoe_passwd(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = (argv[0]);
	char passwd[33] = {0};

	ret = clsapi_net_get_pppoe_passwd(interface_name, passwd);

	return clsapi_cli_report_str_value(ret, output, passwd);
}

static int cli_get_ipv6ConnStatus(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = (argv[0]);
	bool connected = 0;

	ret = clsapi_net_get_ipv6ConnStatus(interface_name, &connected);

	return clsapi_cli_report_int_value(ret, output, connected);
}

static int cli_get_ipv6prefixDelegationEnabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = (argv[0]);
	bool enabled = 0;

	ret = clsapi_net_get_ipv6prefixDelegationEnabled(interface_name, &enabled);

	return clsapi_cli_report_int_value(ret, output, enabled);
}

static int cli_get_ipv6DSliteEnabled(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *interface_name = (argv[0]);
	bool enabled = 0;

	ret = clsapi_net_get_ipv6DSliteEnabled(interface_name, &enabled);

	return clsapi_cli_report_int_value(ret, output, enabled);
}

static int cli_add_bridge_vlan(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = (argv[0]);
	const uint16_t vlan_id = atol(argv[1]);

	ret = clsapi_net_add_bridge_vlan(bridge_name, vlan_id);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_bridge_vlan(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = (argv[0]);
	const uint16_t vlan_id = atol(argv[1]);

	ret = clsapi_net_del_bridge_vlan(bridge_name, vlan_id);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_enable_bridge_vlan(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = (argv[0]);
	const bool enabled = atol(argv[1]);

	ret = clsapi_net_enable_bridge_vlan(bridge_name, enabled);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_add_bridge_vlan_port(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = (argv[0]);
	const uint16_t vlan_id = atol(argv[1]);
	const char *port = (argv[2]);
	const bool tagged = atol(argv[3]);
	const bool pvid = atol(argv[4]);

	ret = clsapi_net_add_bridge_vlan_port(bridge_name, vlan_id, port, tagged, pvid);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_del_bridge_vlan_port(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = (argv[0]);
	const uint16_t vlan_id = atol(argv[1]);
	const char *port = (argv[2]);

	ret = clsapi_net_del_bridge_vlan_port(bridge_name, vlan_id, port);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_bridge_vlan_primary(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = (argv[0]);
	const uint16_t vlan_id = atol(argv[1]);
	const bool primary = atol(argv[2]);

	ret = clsapi_net_set_bridge_vlan_primary(bridge_name, vlan_id, primary);

	return clsapi_cli_report_complete(ret, output);
}


struct clsapi_cli_entry clsapi_cli_entry_net_auto[] = {
	{"del bridge", 1, 1, "<bridge_name>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_del_bridge)},
	{"add bridge", 1, 1, "<bridge_name>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_add_bridge)},
	{"add bridge_port", 2, 2, "<bridge_name> <port>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_add_bridge_port)},
	{"del bridge_port", 2, 2, "<bridge_name> <port>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_del_bridge_port)},
	{"del interface", 1, 1, "<interface_name>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_del_interface)},
	{"get dhcp_leasetime", 1, 1, "<netif_name>", cli_generic_get, C_API(clsapi_get_in_charptr_out_u32ptr, clsapi_net_get_dhcp_leasetime)},
	{"set dhcp_leasetime", 2, 2, "<netif_name> <lease_time>", cli_generic_set, C_API(clsapi_set_charptr_u32, clsapi_net_set_dhcp_leasetime)},
	{"enable dhcp_server", 2, 2, "<netif_name> <enable>", cli_enable_dhcp_server},
	{"get dhcp_server_enabled", 1, 1, "<netif_name>", cli_get_dhcp_server_enabled},
	{"set dhcp_addr_pool", 3, 3, "<netif_name> <start_offset> <max_lease>", cli_set_dhcp_addr_pool},
	{"add firewall_zone", 1, 1, "<zone_name>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_add_firewall_zone)},
	{"del firewall_zone", 1, 1, "<zone_name>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_del_firewall_zone)},
	{"get dns_domain_suffix", 0, 0, "", cli_get_dns_domain_suffix},
	{"set dns_domain_suffix", 1, 1, "<dns_suffix>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_set_dns_domain_suffix)},
	{"del dns_domain_binding", 1, 1, "<domain_name>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_net_del_dns_domain_binding)},
	{"add firewall_zone_forwarding", 2, 2, "<src_zone> <dest_zone>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_add_firewall_zone_forwarding)},
	{"del firewall_zone_forwarding", 2, 2, "<src_zone> <dest_zone>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_del_firewall_zone_forwarding)},
	{"add firewall_zone_network", 2, 2, "<zone_name> <network_name>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_add_firewall_zone_network)},
	{"del firewall_zone_network", 2, 2, "<zone_name> <network_name>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_del_firewall_zone_network)},
	{"get pppoe_username", 1, 1, "<interface_name>", cli_get_pppoe_username},
	{"set pppoe_username", 2, 2, "<interface_name> <username>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_set_pppoe_username)},
	{"get pppoe_passwd", 1, 1, "<interface_name>", cli_get_pppoe_passwd},
	{"set pppoe_passwd", 2, 2, "<interface_name> <passwd>", cli_generic_set, C_API(clsapi_set_charptr_charptr, clsapi_net_set_pppoe_passwd)},
	{"get ipv6ConnStatus", 1, 1, "<interface_name>", cli_get_ipv6ConnStatus},
	{"get ipv6prefixDelegationEnabled", 1, 1, "<interface_name>", cli_get_ipv6prefixDelegationEnabled},
	{"get ipv6DSliteEnabled", 1, 1, "<interface_name>", cli_get_ipv6DSliteEnabled},
	{"add bridge_vlan", 2, 2, "<bridge_name> <vlan_id>", cli_add_bridge_vlan},
	{"del bridge_vlan", 2, 2, "<bridge_name> <vlan_id>", cli_del_bridge_vlan},
	{"enable bridge_vlan", 2, 2, "<bridge_name> <enabled>", cli_enable_bridge_vlan},
	{"add bridge_vlan_port", 5, 5, "<bridge_name> <vlan_id> <port> <tagged> <pvid>", cli_add_bridge_vlan_port},
	{"del bridge_vlan_port", 3, 3, "<bridge_name> <vlan_id> <port>", cli_del_bridge_vlan_port},
	{"set bridge_vlan_primary", 3, 3, "<bridge_name> <vlan_id> <primary>", cli_set_bridge_vlan_primary},

	{}
};

#endif /* _AUTOGENED_CLI_NET_H */