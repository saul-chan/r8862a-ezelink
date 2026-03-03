/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _NET_UTIL_H
#define _NET_UTIL_H

#include "clsapi_common.h"

/*
 * \brief Convert firewall enum type chain to string type.
 * \details This function aims to validate firewall chain is legal or not, check by enum chain, if leagle and convert to string.
 * \param chain [in] Enum type structure to imply firewall chain.
 * \return string to imply firewall chain on success or others for errors.
 * */
static inline char *firewall_chain_enum2str(enum clsapi_firewall_chain chain)
{
	static string_32 str_chain = {0};

	if (chain == CLSAPI_FIREWALL_CHAIN_INPUT)
		strncpy(str_chain, "input", sizeof(str_chain));
	else if (chain == CLSAPI_FIREWALL_CHAIN_OUTPUT)
		strncpy(str_chain, "output", sizeof(str_chain));
	else if (chain == CLSAPI_FIREWALL_CHAIN_FORWARD)
		strncpy(str_chain, "forward", sizeof(str_chain));

	return str_chain;
}

/*
 * \brief Convert firewall string type chain to enum type.
 * \details This function aims to validate firewall chain is legal or not, check by string chain, if leagle and convert to enum type.
 * \param chain [in] String to imply firewall chain.
 * \return euum type to imply firewall chain or _CLSAPI_FIREWALL_CHAIN_MAX for errors.
 * */
static inline enum clsapi_firewall_chain firewall_chain_str2enum(const char *str_chain)
{
	enum clsapi_firewall_chain chain = _CLSAPI_FIREWALL_CHAIN_MAX;

	if (!str_chain)
		return chain;

	if (strcmp(str_chain, "input") == 0)
		chain = CLSAPI_FIREWALL_CHAIN_INPUT;
	else if (strcmp(str_chain, "output") == 0)
		chain = CLSAPI_FIREWALL_CHAIN_OUTPUT;
	else if (strcmp(str_chain, "forward") == 0)
		chain = CLSAPI_FIREWALL_CHAIN_FORWARD;

	return chain;
}

/*
 * \brief Convert firewall enum type policy to string type.
 * \details This function aims to validate firewall policy is legal or not, check by enum policy, if leagle and convert to string.
 * \param policy [in] Enum type structure to imply firewall policy.
 * \return string to imply firewall policy on success or others for errors.
 * */
static inline char *firewall_policy_enum2str(enum clsapi_firewall_policy policy)
{
	static string_32 str_policy = {0};

	if (policy == CLSAPI_FIREWALL_POLICY_ACCEPT)
		cls_strncpy(str_policy, "ACCEPT", sizeof(str_policy));
	else if (policy == CLSAPI_FIREWALL_POLICY_REJECT)
		cls_strncpy(str_policy, "REJECT", sizeof(str_policy));
	else if (policy == CLSAPI_FIREWALL_POLICY_DROP)
		cls_strncpy(str_policy, "DROP", sizeof(str_policy));

	return str_policy;
}

/*
 * \brief Convert firewall string type policy to enum type.
 * \details This function aims to validate firewall policy is legal or not, check by string policy, if leagle and convert to enum type.
 * \param policy [in] String to imply firewall policy.
 * \return euum type to imply firewall policy or _CLSAPI_FIREWALL_POLICY_MAX for errors.
 */
static inline enum clsapi_firewall_policy firewall_policy_str2enum(const char *str_policy)
{
	enum clsapi_firewall_policy policy = _CLSAPI_FIREWALL_POLICY_MAX;

	if (!str_policy)
		return policy;

	if (strcmp(str_policy, "ACCEPT") == 0)
		policy = CLSAPI_FIREWALL_POLICY_ACCEPT;
	else if (strcmp(str_policy, "REJECT") == 0)
		policy = CLSAPI_FIREWALL_POLICY_REJECT;
	else if (strcmp(str_policy, "DROP") == 0)
		policy = CLSAPI_FIREWALL_POLICY_DROP;

	return policy;
}

/*
 * \brief Convert network protocol enum type to string type.
 * \details This function aims to validate network protocol is legal or not, check by enum protocol, if leagle and convert to string.
 * \param proto [in] Enum type structure to imply network protocol.
 * \return string to imply network protocol on success or others for errors.
 * */
static inline char *network_proto_enum2str(enum clsapi_net_protocol proto)
{
	static string_32 str_proto = {0};

	if (proto == CLSAPI_NETWORK_PROTO_STATIC)
		cls_strncpy(str_proto, "static", sizeof(str_proto));
	else if (proto == CLSAPI_NETWORK_PROTO_DHCP)
		cls_strncpy(str_proto, "dhcp", sizeof(str_proto));
	else if (proto == CLSAPI_NETWORK_PROTO_DHCPV6)
		cls_strncpy(str_proto, "dhcpv6", sizeof(str_proto));
	else if (proto == CLSAPI_NETWORK_PROTO_PPPOE)
		cls_strncpy(str_proto, "pppoe", sizeof(str_proto));

	return str_proto;
}

/*
 * \brief Convert network string type protocol to enum type.
 * \details This function aims to validate network protocol is legal or not, check by string protocol, if leagle and convert to enum type.
 * \param proto [in] String to imply network protocol.
 * \return enum type to imply network protocol or _CLSAPI_NETWORK_PROTO_MAX for errors.
 */
static inline enum clsapi_net_protocol network_proto_str2enum(const char *str_proto)
{
	enum clsapi_net_protocol proto = _CLSAPI_NETWORK_PROTO_MAX;

	if (!str_proto)
		return proto;

	if (strcmp(str_proto, "static") == 0)
		proto = CLSAPI_NETWORK_PROTO_STATIC;
	else if (strcmp(str_proto, "dhcp") == 0)
		proto = CLSAPI_NETWORK_PROTO_DHCP;
	else if (strcmp(str_proto, "dhcpv6") == 0)
		proto = CLSAPI_NETWORK_PROTO_DHCPV6;
	else if (strcmp(str_proto, "pppoe") == 0)
		proto = CLSAPI_NETWORK_PROTO_PPPOE;
	else
		proto = _CLSAPI_NETWORK_PROTO_MAX;

	return proto;
}

/*
 * \brief Convert device opmode enum type to string type.
 * \details This function aims to validate device opmode is legal or not, check by enum opmode, if leagle and convert to string.
 * \param opmode [in] Enum type structure to imply device opmode.
 * \return string to imply device opmode on success or others for errors.
 * */
static inline char *network_opmode_enum2str(enum clsapi_net_opmode opmode)
{
	static string_32 str_opmode = {0};

	if (opmode == CLSAPI_NETWORK_OPMODE_ROUTER)
		cls_strncpy(str_opmode, "router", sizeof(str_opmode));
	else if (opmode == CLSAPI_NETWORK_OPMODE_BRIDGE)
		cls_strncpy(str_opmode, "bridge", sizeof(str_opmode));
	else if (opmode == CLSAPI_NETWORK_OPMODE_REPEATER)
		cls_strncpy(str_opmode, "repeater", sizeof(str_opmode));
	else if (opmode == CLSAPI_NETWORK_OPMODE_REPEATER_5G)
		cls_strncpy(str_opmode, "repeater_5g", sizeof(str_opmode));

	return str_opmode;
}

/*
 * \brief Convert device string type opmode to enum type.
 * \details This function aims to validate device opmode is legal or not, check by string opmode, if leagle and convert to enum type.
 * \param opmode [in] String to imply device opmode.
 * \return enum type to imply device opmode or _CLSAPI_NETWORK_OPMODE_MAX for errors.
 */
static inline enum clsapi_net_opmode network_opmode_str2enum(const char *str_opmode)
{
	enum clsapi_net_opmode opmode = _CLSAPI_NETWORK_OPMODE_MAX;

	if (!str_opmode)
		return opmode;

	if (strcmp(str_opmode, "router") == 0)
		opmode = CLSAPI_NETWORK_OPMODE_ROUTER;
	else if (strcmp(str_opmode, "bridge") == 0)
		opmode = CLSAPI_NETWORK_OPMODE_BRIDGE;
	else if (strcmp(str_opmode, "repeater") == 0)
		opmode = CLSAPI_NETWORK_OPMODE_REPEATER;
	else if (strcmp(str_opmode, "repeater_5g") == 0)
		opmode = CLSAPI_NETWORK_OPMODE_REPEATER_5G;

	return opmode;
}
#endif
