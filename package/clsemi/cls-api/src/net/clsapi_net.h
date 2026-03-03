/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_NET_H
#define _CLSAPI_NET_H

#include "clsapi_common.h"
#include <arpa/inet.h>


/*******************************	Macro definitions	**************************/

/******************	Configurations	******************/
#define CLSCONF_CFG_APPFILTER		"appfilter"
#define CLSCONF_CFG_CLS_OPMODE		"cls-opmode"
#define CLSCONF_CFG_CLS_QOS			"cls-qos"
#define CLSCONF_CFG_DHCP			"dhcp"
#define CLSCONF_CFG_DROPBEAR		"dropbear"
#define CLSCONF_CFG_FIREWALL		"firewall"
#define CLSCONF_CFG_NETWORK			"network"
#define CLSCONF_CFG_UHTTPD			"uhttpd"
#define CLSCONF_CFG_WIRELESS		"wireless"

/* CLSCONF_CFG_CLS_OPMODE */
#define CLS_OPMODE_SECT_GLOBALS		"globals"
#define CLS_OPMODE_PARAM_MODE		"mode"

#define CLS_NETWORK_SECT_DEFAULT_POLICY		"@defaults[0]"
#define CLS_NETWORK_SECT_DEVICE		"device"
#define CLS_NETWORK_SECT_BRIDGE_VLAN	"bridge-vlan"
#define CLS_NETWORK_SECT_INTERFACE		"interface"
#define CLS_NETWORK_SECT_ROUTE		"route"

#define CLS_NETWORK_SECT_TYPE_INTERFACE		"interface"
#define CLS_NETWORK_SECT_TYPE_DEVICE		"device"
#define CLS_NETWORK_SECT_TYPE_ZONE		"zone"
#define CLS_NETWORK_SECT_TYPE_DOMAIN	"domain"
#define CLS_NETWORK_SECT_TYPE_FORWARD	"forwarding"
#define CLS_NETWORK_SECT_TYPE_RULE		"rule"

#define CLS_NETWORK_PARAM_DEVICE		"device"
#define CLS_NETWORK_PARAM_INTERFACE		"interface"
#define CLS_NETWORK_PARAM_NAME		"name"
#define CLS_NETWORK_PARAM_DHCP_OP	"dhcp_option"
#define CLS_NETWORK_PARAM_NETWORK	"network"

#define IPV6_PREFIX_ASSIGN_BITS "60"
#define DEFAULT_IPV6_STATIC_ADDR "fd00:abcd::1/60"
#define DEFAULT_IPV6_PREFIX "fd00:abcd::/60"
#define DEFAULT_IPV6_ULA_ADDR "fd12:abcd::1/60"

/** Stores the mapping between a physical network device and its corresponding OpenWrt logical interface configuration */
struct netdev_info {
	/** Firewall/network zone name (e.g., "lan", "wan") */
	string_32 zone;

	/** Logical interface section name in OpenWrt's /etc/config/network */
	string_32 section_name;
};

/** DHCP lease information */
struct dhcp_lease_info {
	/** Expires time for dhcp lease, in uint of second */
	uint32_t expires;

	/** MAC address for host leased */
	uint8_t macaddr[ETH_ALEN];

	/** IP address for host leased */
	struct in_addr ipaddr;

	/** Name for host */
	string_128 hostname;
};

/** IP address struct for clsapi */
struct clsapi_net_ipaddr {
	/** sa_family_t in_family: Specifies the address family (AF_INET for IPv4, AF_INET6 for IPv6) */
	sa_family_t in_family;

	union {
		/** The in_addr structure represents an IPv4 Internet address */
		struct in_addr ipaddr;

		/** The in6_addr structure represents an IPv6 Internet address */
		struct in6_addr ipv6addr;
	} addr;

#define v4_addr addr.ipaddr
#define v6_addr addr.ipv6addr
};

/** Save the assigned IPv6 address and its scope on the interface */
struct clsapi_ipv6_info {
	/** The in6_addr structure represents an IPv6 Internet address */
	struct in6_addr addr;

	/** Restore IPv6 address scope, such as 1 for unspecified... */
	int scope;
};

/** Record different types of network packet statistics or error statistics */
struct clsapi_net_statistics {
	/** Collision counting */
	uint64_t collisions;

	/** Error frame reception count */
	uint64_t rx_frame_errors;

	/** Number of compressed data packets sent */
	uint64_t tx_compressed;

	/** Number of received multicast packets */
	uint64_t multicast;

	/** Received packets of illegal length */
	uint64_t rx_length_errors;

	/** Number of dropped sent packets */
	uint64_t tx_dropped;

	/** Total number of bytes received */
	uint64_t rx_bytes;

	/** Data packets lost due to high load or full buffer */
	uint64_t rx_missed_errors;

	/** Total number of error packets sent */
	uint64_t tx_errors;

	/** Number of compressed data packets received */
	uint64_t rx_compressed;

	/** Buffer overflow Error */
	uint64_t rx_over_errors;

	/** FIFO encountered an error when sending */
	uint64_t tx_fifo_errors;

	/** Received packet with CRC check failure */
	uint64_t rx_crc_errors;

	/** Total number of data packets received */
	uint64_t rx_packets;

	/** Heartbeat error */
	uint64_t tx_heartbeat_errors;

	/** Number of dropped received packets */
	uint64_t rx_dropped;

	/** Number of interrupted packets sent */
	uint64_t tx_aborted_errors;

	/** Total number of data packets sent */
	uint64_t tx_packets;

	/** Total number of error packets received */
	uint64_t rx_errors;

	/** Total number of bytes sent */
	uint64_t tx_bytes;

	/** Sending window error, eg: TCP window overflow */
	uint64_t tx_window_errors;

	/** FIFO encountered an error when receiving */
	uint64_t rx_fifo_errors;

	/** Carrier signal error */
	uint64_t tx_carrier_errors;
};

enum clsapi_ipv6_prefix_origin {
	CLSAPI_PREFIX_ORIGIN_STATIC,
	CLSAPI_PREFIX_ORIGIN_PREFIX_DELEGATION,
	CLSAPI_PREFIX_ORIGIN_UNKNOW,
};

enum clsapi_net_protocol {
	CLSAPI_NETWORK_PROTO_STATIC,
	CLSAPI_NETWORK_PROTO_DHCP,
	CLSAPI_NETWORK_PROTO_PPPOE,
	CLSAPI_NETWORK_PROTO_DHCPV6,
	_CLSAPI_NETWORK_PROTO_MAX,
};

enum clsapi_firewall_chain {
	CLSAPI_FIREWALL_CHAIN_INPUT,
	CLSAPI_FIREWALL_CHAIN_OUTPUT,
	CLSAPI_FIREWALL_CHAIN_FORWARD,
	_CLSAPI_FIREWALL_CHAIN_MAX
};

enum clsapi_firewall_policy {
	CLSAPI_FIREWALL_POLICY_ACCEPT,
	CLSAPI_FIREWALL_POLICY_REJECT,
	CLSAPI_FIREWALL_POLICY_DROP,
	_CLSAPI_FIREWALL_POLICY_MAX
};

enum clsapi_net_opmode {
	CLSAPI_NETWORK_OPMODE_ROUTER,
	CLSAPI_NETWORK_OPMODE_REPEATER,
	CLSAPI_NETWORK_OPMODE_BRIDGE,
	CLSAPI_NETWORK_OPMODE_REPEATER_5G,
	_CLSAPI_NETWORK_OPMODE_MAX
};

enum clsapi_ipv6addr_classfication {
	CLSAPI_ADDR_GLOBAL = 0,
	CLSAPI_ADDR_LINKLOCAL,
	CLSAPI_ADDR_ULA_LOCAL,
	CLSAPI_ADDR_OTHERS,
	_CLSAPI_ADDR_MAX,
};

enum clsapi_net_speed_type {
	CLSAPI_NET_SPEED_10M = 10,
	CLSAPI_NET_SPEED_100M = 100,
	CLSAPI_NET_SPEED_1000M = 1000,
	CLSAPI_NET_SPEED_2500M = 2500,
	CLSAPI_NET_SPEED_5000M = 5000,
	CLSAPI_NET_SPEED_10000M = 10000,
	_CLSAPI_NET_SPEED_TYPE_MAX = -1,
};

enum clsapi_ipv6addr_allocation {
	CLSAPI_IPV6ADDR_Manual_Assign = 0,
	CLSAPI_IPV6ADDR_DHCPv6,
	CLSAPI_IPV6ADDR_SLAAC,
	_CLSAPI_IPV6ADDR_MAX,
};

enum clsapi_ipv6prefix_allocation {
	CLSAPI_IPV6PRE_PD= 0,
	CLSAPI_IPV6PRE_STATIC,
	_CLSAPI_IPV6PRE_MAX,
};

union clsapi_fw_rule_target_option {
	/*Zeroes out the bits given by mask and ORs value into the packet mark. If mask is omitted, 0xFFFFFFFF is assumed.*/
	uint64_t set_mark;

	/*Zeroes out the bits given by mask and XORs value into the packet mark. If mask is omitted, 0xFFFFFFFF is assumed.*/
	uint64_t set_xmark;

	/*Helper for netfilter such as manda,ftp...*/
	string_32 set_helper;

	/*DSCP classification*/
	string_32 set_dscp;
};

enum clsapi_firewall_rule_target {
	CLSAPI_FIREWALL_TARGET_ACCEPT,
	CLSAPI_FIREWALL_TARGET_REJECT,
	CLSAPI_FIREWALL_TARGET_DROP,
	CLSAPI_FIREWALL_TARGET_NOTRACK,
	CLSAPI_FIREWALL_TARGET_XMARK,
	CLSAPI_FIREWALL_TARGET_MARK,
	CLSAPI_FIREWALL_TARGET_HELPER,
	CLSAPI_FIREWALL_TARGET_DSCP,
	_CLSAPI_FIREWALL_TARGET_MAX,
};

#define set_fw_rule_name(rule, param_val)	\
	do {\
		cls_strncpy(rule->name, param_val, sizeof(rule->name));\
		rule->flag |= (1 << 0);\
	} while(0)\

#define set_fw_rule_src(rule, param_val)	\
	do {\
		cls_strncpy(rule->src, param_val, sizeof(rule->src));\
		rule->flag |= (1 << 1);\
	} while(0)\

#define set_fw_rule_src_ipaddr(rule, param_val)	\
	do {\
		memcpy(rule->src_ipaddr, param_val, sizeof(struct in_addr) * rule->src_ipaddr_num);\
		rule->flag |= (1 << 2);\
	} while(0)\

#define set_fw_rule_src_macaddr(rule, param_val)	\
	do {\
		memcpy(rule->src_macaddr, param_val, sizeof(uint8_t [ETH_ALEN]) * rule->src_macaddr_num);\
		rule->flag |= (1 << 3);\
	} while(0)\

#define set_fw_rule_src_port(rule, param_val)	\
	do {\
		memcpy(rule->src_port, param_val, sizeof(uint32_t) * rule->src_port_num);\
		rule->flag |= (1 << 4);\
	} while(0)\

#define set_fw_rule_proto(rule, param_val)	\
	do {\
		memcpy(rule->proto, param_val, sizeof(string_32) * rule->proto_num);\
		rule->flag |= (1 << 5);\
	} while(0)\

#define set_fw_rule_icmp_type(rule, param_val)	\
	do {\
		memcpy(rule->icmp_type, param_val, sizeof(rule->icmp_type));\
		rule->flag |= (1 << 6);\
	} while(0)\

#define set_fw_rule_dest(rule, param_val)	\
	do {\
		cls_strncpy(rule->dest, param_val, sizeof(rule->dest));\
		rule->flag |= (1 << 7);\
	} while(0)\

#define set_fw_rule_dest_ipaddr(rule, param_val)	\
	do {\
		memcpy(rule->dest_ipaddr, param_val, sizeof(struct in_addr) * rule->dest_ipaddr_num);\
		rule->flag |= (1 << 8);\
	} while(0)\

#define set_fw_rule_dest_port(rule, param_val)	\
	do {\
		memcpy(rule->dest_port, param_val, sizeof(uint32_t) * rule->dest_port_num);\
		rule->flag |= (1 << 9);\
	} while(0)\

#define set_fw_rule_mark(rule, param_val)	\
	do {\
		cls_strncpy(rule->mark, param_val, sizeof(rule->mark));\
		rule->flag |= (1 << 10);\
	} while(0)\

#define set_fw_rule_start_date(rule, param_val)	\
	do {\
		cls_strncpy(rule->start_date, param_val, sizeof(rule->start_date));\
		rule->flag |= (1 << 11);\
	} while(0)\

#define set_fw_rule_start_time(rule, param_val)	\
	do {\
		cls_strncpy(rule->start_time, param_val, sizeof(rule->start_time));\
		rule->flag |= (1 << 12);\
	} while(0)\

#define set_fw_rule_stop_date(rule, param_val)	\
	do {\
		cls_strncpy(rule->stop_date, param_val, sizeof(rule->stop_date));\
		rule->flag |= (1 << 13);\
	} while(0)\

#define set_fw_rule_stop_time(rule, param_val)	\
	do {\
		cls_strncpy(rule->stop_time, param_val, sizeof(rule->stop_time));\
		rule->flag |= (1 << 14);\
	} while(0)\

#define set_fw_rule_weekdays(rule, param_val)	\
	do {\
		memcpy(rule->weekdays, param_val, sizeof(rule->weekdays));\
		rule->flag |= (1 << 15);\
	} while(0)\

#define set_fw_rule_monthdays(rule, param_val)	\
	do {\
		memcpy(rule->monthdays, param_val, sizeof(rule->monthdays));\
		rule->flag |= (1 << 16);\
	} while(0)\

#define set_fw_rule_utc_time(rule, param_val)	\
	do {\
		rule->utc_time = param_val;\
		rule->flag |= (1 << 17);\
	} while(0)\

#define set_fw_rule_target(rule, param_val)	\
	do {\
		rule->target = param_val;\
		rule->flag |= (1 << 18);\
	} while(0)\

#define set_fw_rule_family(rule, param_val)	\
	do {\
		cls_strncpy(rule->family, param_val, sizeof(rule->family));\
		rule->flag |= (1 << 19);\
	} while(0)\

#define set_fw_rule_limit(rule, param_val)	\
	do {\
		cls_strncpy(rule->limit, param_val, sizeof(rule->limit));\
		rule->flag |= (1 << 20);\
	} while(0)\

#define set_fw_rule_limit_burst(rule, param_val)	\
	do {\
		rule->limit_burst = param_val;\
		rule->flag |= (1 << 21);\
	} while(0)\

#define set_fw_rule_enabled(rule, param_val)	\
	do {\
		rule->enabled = param_val;\
		rule->flag |= (1 << 22);\
	} while(0)\

#define set_fw_rule_device(rule, param_val)	\
	do {\
		cls_strncpy(rule->device, param_val, sizeof(rule->device));\
		rule->flag |= (1 << 23);\
	} while(0)\

#define set_fw_rule_dscp(rule, param_val)	\
	do {\
		cls_strncpy(rule->dscp, param_val, sizeof(rule->dscp));\
		rule->flag |= (1 << 24);\
	} while(0)\

#define set_fw_rule_helper(rule, param_val)	\
	do {\
		cls_strncpy(rule->helper, param_val, sizeof(rule->helper));\
		rule->flag |= (1 << 25);\
	} while(0)\

/** The rule of firewall */
struct clsapi_net_firewall_rule {
	/** Name of rule */
	string_32 name;

	/** Specifies the traffic source zone. Refers to one of the defined zone names, or * for any zone. If omitted, the rule applies to output traffic */
	string_32 src;

	/** Match incoming traffic from the specified source IP address */
	struct in_addr *src_ipaddr;

	/** Source ip address number */
	uint32_t src_ipaddr_num;

	/** Match incoming traffic from the specified MAC address */
	uint8_t (*src_macaddr)[ETH_ALEN];

	/** Source mac address number */
	uint32_t src_macaddr_num;

	/** Match incoming traffic from the specified source port or port range, if relevant proto is specified. Multiple ports can be specified like '80 443 465' */
	uint32_t *src_port;

	/** Source port number */
	uint32_t src_port_num;

	/** Match incoming traffic using the given protocol */
	string_32 *proto;

	/** Protocol number */
	uint32_t proto_num;

	/** For protocol icmp select specific ICMP types to match. Values can be either exact ICMP type numbers or type names */
	string_32 icmp_type[128];

	/** Specifies the traffic destination zone. Refers to one of the defined zone names, or * for any zone. If specified, the rule applies to forwarded traffic; otherwise, it is treated as input rule */
	string_32 dest;

	/** Match incoming traffic directed to the specified destination IP address. With no dest zone, this is treated as an input rule! */
	struct in_addr *dest_ipaddr;

	/** Target IP address number */
	uint32_t dest_ipaddr_num;

	/** Match incoming traffic directed at the given destination port or port range, if relevant proto is specified. Multiple ports can be specified like '80 443 465' */
	uint32_t *dest_port;

	/** Target port number */
	uint32_t dest_port_num;

	/** If specified, match traffic against the given firewall mark, e.g. 0xFF to match mark 255 or 0x0/0x1 to match any even mark value. The match can be inverted by prefixing the value with an exclamation mark, e.g. !0x10 to match all but mark #16 */
	string_32 mark;

	/** If specifed, only match traffic after the given date (inclusive). yyyy:mm:dd */
	string_32 start_date;

	/** If specified, only match traffic after the given time of day (inclusive). hh:mm:ss */
	string_32 start_time;

	/** If specified, only match traffic before the given date (inclusive). yyyy:mm:dd */
	string_32 stop_date;

	/** If specified, only match traffic before the given time of day (inclusive). hh:mm:ss */
	string_32 stop_time;

	/** If specified, only match traffic during the given week days, e.g. sun mon thu fri to only match on sundays, mondays, thursdays and Fridays. The list can be inverted by prefixing it with an exclamation mark, e.g. ! sat sun to always match but on Saturdays and sundays */
	string_32 weekdays[7];

	/** If specified, only match traffic during the given days of the month, e.g. 2 5 30 to only match on every 2nd, 5th and 30rd day of the month. The list can be inverted by prefixing it with an exclamation mark, e.g. ! 31 to always match but on the 31st of the month */
	string_32 monthdays[31];

	/** Treat all given time values as UTC time instead of local time */
	bool utc_time;

	/** Firewall action (ACCEPT, REJECT, DROP, MARK, NOTRACK) for matched traffic */
	enum clsapi_firewall_rule_target target;

	union clsapi_fw_rule_target_option target_option;

	/** Specifies the address family (ipv4, ipv6 or any) for which the rules are generated. If unspecified, matches the address family of other options in this section and defaults to any */
	string_32 family;

	/** Maximum average matching rate; specified as a number, with an optional /second, /minute, /hour or /day suffix. Examples: 3/minute, 3/min or 3/m */
	string_32 limit;

	/** Maximum initial number of packets to match, allowing a short-term average above limit */
	uint8_t limit_burst;

	/** Enable this firewall rule or not */
	bool enabled;

	/** Specify whether to tie this rule to a inbound or outbound network device */
	string_32 device;

	/** Specify the direction, inbound or outbound */
	string_32 direction;

	/** Specified DSCP class name such as CS0-CS7... */
	string_32 dscp;

	/** Specified netfilter conntrack helper, such as amanda,ftp... */
	string_32 helper;

	/** Flag to specify which member to be set */
	uint32_t flag;
};

/*****************************	Data type definitions	**************************/

/*!\addtogroup network
 *  @{
*/

/*************************	C-Call functions declarations	**********************/

/**
 * \brief Get the operation mode of the device.
 * \details Retrieve current operation mode of device.
 * \param opmode [Out] The operation mode of the device.
 \n	CLSAPI_NETWORK_OPMODE_ROUTER = 0
 \n	CLSAPI_NETWORK_OPMODE_REPEATER = 1
 \n	CLSAPI_NETWORK_OPMODE_BRIDGE = 2
 \n	CLSAPI_NETWORK_OPMODE_REPEATER_5G = 3
 * \return 0 on success or others on error.
 */
int clsapi_net_get_opmode(enum clsapi_net_opmode *opmode);

/**
 * \brief Set the operation mode of the device.
 * \details Set current operation mode of device.
 * \param opmode [In] The operation mode of the device.
 \n	CLSAPI_NETWORK_OPMODE_ROUTER = 0
 \n	CLSAPI_NETWORK_OPMODE_REPEATER = 1
 \n	CLSAPI_NETWORK_OPMODE_BRIDGE = 2
 \n	CLSAPI_NETWORK_OPMODE_REPEATER_5G = 3
 * \return 0 on success or others on error.
 */
int clsapi_net_set_opmode(enum clsapi_net_opmode opmode);

/**
 * \brief Get the MAC address of specified network interface.
 * \details Get MAC addresses of different network interfaces, such as br-lan.
 * \param netif_name [In] The name of the network interface.
 * \param macaddr [Out] The MAC address of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_macaddr(const char *netif_name, uint8_t macaddr[ETH_ALEN]);

/**
 * \brief Get the speed of specified network interface.
 * \details Get speed of different network interfaces, such as br-lan. If the driver is not supported, the value will be -1.
 * \param netif_name [In] The name of network interface.
 * \param speed [Out] The speed of network interface.
 \n	CLSAPI_NET_SPEED_10M = 10
 \n	CLSAPI_NET_SPEED_100M = 100
 \n	CLSAPI_NET_SPEED_1000M = 1000
 \n	CLSAPI_NET_SPEED_2500M = 2500
 \n	CLSAPI_NET_SPEED_5000M = 5000
 \n	CLSAPI_NET_SPEED_10000M = 10000
 * \return 0 on success or others on error.
 */
int clsapi_net_get_speed(const char *netif_name, enum clsapi_net_speed_type *speed);

/**
 * \brief Get all the statistics data of specific network interface.
 * \details Get all the statistics data of specific network interfaces, such as br-lan.
 * \param netif_name [In] The name of the network interface.
 * \param netstat [Out] The statistics of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_stats(const char *netif_name, struct clsapi_net_statistics *netstat);

/**
 * \brief Get the protocol of specified network interface.
 * \details Get the protocol of specified network interface, like "wan" or "lan".
 * \param interface_name [In] The name of the network interface, such as "wan" or "lan".
 * \param proto [Out] The protocol of the network interface.
 \n	CLSAPI_NETWORK_PROTO_STATIC = 0
 \n	CLSAPI_NETWORK_PROTO_DHCP = 1
 \n	CLSAPI_NETWORK_PROTO_PPPOE = 2
 \n	CLSAPI_NETWORK_PROTO_DHCPV6 = 3
 * \return 0 on success or others on error.
 */
int clsapi_net_get_proto(const char *interface_name, enum clsapi_net_protocol *proto);

/**
 * \brief Set the protocol of specified network interface.
 * \details Set the protocol according to different requirements: static/dhcp/dhcpv6/PPPoE are supported.
 * \param interface_name [In] The name of the network interface.
 * \param proto [In] The protocol of the network interface.
 \n	CLSAPI_NETWORK_PROTO_STATIC = 0
 \n	CLSAPI_NETWORK_PROTO_DHCP = 1
 \n	CLSAPI_NETWORK_PROTO_PPPOE = 2
 \n	CLSAPI_NETWORK_PROTO_DHCPV6 = 3
 * \return 0 on success or others on error.
 */
int clsapi_net_set_proto(const char *interface_name, enum clsapi_net_protocol proto);

/**
 * \brief Get the IPv4 address of specified network interface.
 * \details Get the IPv4 address according to different interfaces, such as "br-lan"/"eth0".
 * \param netif_name [In] The name of the network interface, such as "br-lan"/"eth0".
 * \param ipaddr [Out] The IPv4 address structure of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipaddr(const char *netif_name, struct in_addr *ipaddr);

/**
 * \brief Get the IPv6 address of specified network interface.
 * \details Get the IPv6 address according to different interfaces, such as "br-lan"/"eth0".
 * \param netif_name [In] The name of the network interface, such as "br-lan"/"eth0".
 * \param ipaddr [Out] The IPv6 address structure of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6addr(const char *netif_name, struct in6_addr *ipaddr);
/**
 * \brief Set the IPv4 address of a specified network interface.
 * \details Set the IPv4 address according to different interfaces, such as "br-lan"/"eth0".
 * \param netif_name [In] The name of the network interface. "br-lan" or "eth0".
 * \param ipaddr [In] The IPv4 address of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_ipaddr(const char *netif_name, struct in_addr *ipaddr);

/**
 * \brief Get the netmask of specified network interface.
 * \details Get the netmask according to different interfaces, such as "br-lan"/"eth0".
 * \param netif_name [In] The name of the network interface, such as "br-lan"/"eth0".
 * \param netmask [Out] The netmask of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_netmask(const char *netif_name, struct in_addr *netmask);

/**
 * \brief Set the netmask of specified network interface.
 * \details Set the netmask according to different interfaces, such as "br-lan"/"eth0".
 * \param netif_name [In] The name of the network interface, like "br-lan" or "eth0".
 * \param netmask [In] The netmask of the network interface.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_netmask(const char *netif_name, struct in_addr *netmask);

/**
 * \brief Delete a bridge.
 * \details Delete the "bridge_name" bridge in the netwrok.
 * \param bridge_name [In] The name of the network bridge, like "br-lan" or "br-lan1".
 * \return 0 on success or others on error.
 */
int clsapi_net_del_bridge(const char *bridge_name);

/**
 * \brief Add a bridge.
 * \details Add a specified bridge without ports. Ports should be added into this bridge later.
 * \param bridge_name [In] The name of the network bridge, like "br-lan" or "br-lan1".
 * \return 0 on success or others on error.
 */
int clsapi_net_add_bridge(const char *bridge_name);

/**
 * \brief Add ports to the bridge.
 * \details Add new ports to bridge in the network.
 * \param bridge_name [In] The bridge name which the port will be added to.
 * \param port [In] The port name such as "ifconfig" show.
 * \return 0 on success or others on error.
 * */
int clsapi_net_add_bridge_port(const char *bridge_name, const char *port);

/**
 * \brief Delete a port of bridge device.
 * \details Delete an existed port of bridge device in the network.
 * \param bridge_name [In] The name of bridge which will be deleted.
 * \param port [In] The port name will be deleted.
 * \return 0 on success or others on error.
 * */
int clsapi_net_del_bridge_port(const char *bridge_name, const char *port);

/**
 * \brief Add a network logical interface.
 * \details Add a specified logical interface named "interface_name" with device name, protocol.
 * \param interface_name [In] The name of the network interface. Such as "lan" or "wan".
 * \param device_name [In] The name of the device attached to interface. As "ifconfig" show.
 * \param proto [In] The protocol of the network interface. Detail information need to be filled.
 \n	CLSAPI_NETWORK_PROTO_STATIC = 0
 \n	CLSAPI_NETWORK_PROTO_DHCP = 1
 \n	CLSAPI_NETWORK_PROTO_PPPOE = 2
 \n	CLSAPI_NETWORK_PROTO_DHCPV6 = 3
 * \return 0 on success or others on error.
 */
int clsapi_net_add_interface(const char *interface_name, const char *device_name, enum clsapi_net_protocol proto);

/**
 * \brief Delete a logical network interface.
 * \details Delete "interface_name" logical interface in the netwrok.
 * \param interface_name [In] The name of the network interface. Such as "lan" or "wan".
 * \return 0 on success or others on error.
 */
int clsapi_net_del_interface(const char *interface_name);

/**
 * \brief Get the lease time of DHCP server in second.
 * \details Get the lease time of addresses for clients, in unit of second, like 12 * 3600s or 30 * 60s.
 * \param netif_name [In] The name of the network interface. "br-lan" or "eth0".
 * \param lease_time [Out] The lease time of the address which DHCP server allocates. "0" represents "infinite".
 * \return 0 on success or others on error.
 */
int clsapi_net_get_dhcp_leasetime(const char *netif_name, uint32_t *lease_time);

/**
 * \brief Set the lease time of DHCP server in second.
 * \details Set the lease time of addresses for clients, in unit of second, like 12 * 3600s or 30 * 60s.
 * \param netif_name [In] The name of the network interface. "br-lan" or "eth0".
 * \param lease_time [In] The lease time of the address which the DHCP server allocates,in unit of second, no less than 120 sec. "0" represents "infinite".
 * \return 0 on success or others on error.
 */
int clsapi_net_set_dhcp_leasetime(const char *netif_name, const uint32_t lease_time);

/**
 * \brief Enable or disable the DHCP server of network interface.
 * \details Set 1/0 to enable/disable the DHCP server of the specified network interface. Need to add start address/limit/lease time later.
 * \param netif_name [In] The name of netwrork interface.
 * \param enable [In] "Enable(1)" or "disable(0)" DHCP server of netif_name.
 * \return 0 on success or others on error.
 */
int clsapi_net_enable_dhcp_server(const char *netif_name, const bool enable);

/**
 * \brief Get the status of the DHCP server.
 * \details Get whether the DHCP server of "netif_name" is enabled or not.
 * \param netif_name [In] The name of netwrork interface .
 * \param enable [Out] "Enable(1)" or "disable(0)" the DHCP server of "netif_name".
 * \return 0 on success or others on error.
 */
int clsapi_net_get_dhcp_server_enabled(const char *netif_name, bool *enable);

/**
 * \brief Get the address pool of DHCP server.
 * \details This function retrieves the configured DHCP client address range from "start_offset", as well as the maximum number of leases allocated.
 * \param netif_name [In] The name of the network interface, such as "br-lan".
 * \param start_offset [Out] The offset from the beginning of the address which is assigned to the current interface.
 * \param max_lease [Out] The maximum leased numbers of the DHCP server allocates.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_dhcp_addr_pool(const char *netif_name, uint32_t *start_offset, uint32_t *max_lease);

/**
 * \brief Set the address pool of the DHCP server.
 * \details This function sets the IP address range from "start_offset", as well as the maximum number of leases allocated.
 * \param netif_name [In] The interface name of netwrork.
 * \param start_offset [In] The offset from the beginning of the address assigned to the current interface.
 * \param max_lease [In] The maximum lease numbers of the DHCP server allocates.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_dhcp_addr_pool(const char *netif_name, const uint32_t start_offset, const uint32_t max_lease);

/**
 * \brief Set the firewall default policy.
 * \details Set the firewall default policy for the network, which determines the default behavior of the firewall for incoming, outgoing and forward traffic.
 * \param direction [In] The direction of firewall default policy.
 \n	CLSAPI_FIREWALL_CHAIN_INPUT = 0
 \n	CLSAPI_FIREWALL_CHAIN_OUTPUT = 1
 \n	CLSAPI_FIREWALL_CHAIN_FORWARD = 2
 * \param policy [In] The policy of firewall default.
 \n	CLSAPI_FIREWALL_POLICY_ACCEPT = 0
 \n	CLSAPI_FIREWALL_POLICY_REJECT = 1
 \n	CLSAPI_FIREWALL_POLICY_DROP = 2
 * \return 0 on success or others on error.
 */
int clsapi_net_set_firewall_default_policy(enum clsapi_firewall_chain direction, enum clsapi_firewall_policy policy);

/**
 * \brief Get the firewall default policy.
 * \details Get the firewall default policy for the network, which determines the default behavior of the firewall for incoming, outgoing and forward traffic.
 * \param direction [In] The direction of firewall default policy.
 \n	CLSAPI_FIREWALL_CHAIN_INPUT = 0
 \n	CLSAPI_FIREWALL_CHAIN_OUTPUT = 1
 \n	CLSAPI_FIREWALL_CHAIN_FORWARD = 2
 * \param policy [Out] The policy of firewall default.
 \n	CLSAPI_FIREWALL_POLICY_ACCEPT = 0
 \n	CLSAPI_FIREWALL_POLICY_REJECT = 1
 \n	CLSAPI_FIREWALL_POLICY_DROP = 2
 * \return 0 on success or others on error.
 */
int clsapi_net_get_firewall_default_policy(enum clsapi_firewall_chain direction, enum clsapi_firewall_policy *policy);

/**
 * \brief Add a static route.
 * \details Define a static IPv4 route on a specific interface, gateway should be specified later.
 * \param interface_name [In] Specifies the name of the logical interface which this route belongs to; Must refer to one of the defined interface sections.
 * \param ipaddr [In] Network address.
 * \param netmask [In] Route netmask. If omitted, 255.255.255.255 is assumed which makes target a host address.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_static_route(const char *interface_name, struct in_addr *ipaddr, struct in_addr *netmask);

/**
 * \brief Delete a static route.
 * \details Undefine a static IPv4 route on a specific interface.
 * \param interface_name [In] Specifies the name of the logical interface which this route belongs to;. This interface must refer to one of the defined interface sections.
 * \param ipaddr [In] Network address.
 * \param netmask [In] Route netmask. If omitted, 255.255.255.255 is assumed which makes target a host address.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_static_route(const char *interface_name, struct in_addr *ipaddr, struct in_addr *netmask);

/**
 * \brief Get the gateway of a static route.
 * \details Get the gateway of a static IPv4 route.
 * \param interface_name [In] Specifies the name of the logical interface which this route belongs to;. This interface must refer to one of the defined interface sections.
 * \param ipaddr [In] Network address.
 * \param netmask [In] Route netmask. If omitted, 255.255.255.255 is assumed which makes target a host address.
 * \param gateway [Out] Gateway. Gateway for the route.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_static_route_gateway(const char *interface_name, struct in_addr *ipaddr,
		struct in_addr *netmask, struct in_addr *gateway);

/**
 * \brief Set the gateway of static route.
 * \details Set the gateway of a static IPv4 route on a specific interface.
 * \param interface_name [In] Specifies the name of the logical interface which this route belongs to;. This interface must refer to one of the defined interface sections.
 * \param ipaddr [In] Network address.
 * \param netmask [In] Route netmask. If omitted, 255.255.255.255 is assumed which makes target a host address.
 * \param gateway [In] Gateway. Gateway for the route.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_static_route_gateway(const char *interface_name, struct in_addr *ipaddr,
		struct in_addr *netmask, struct in_addr *gateway);

/**
 * \brief Delete a static route gateway.
 * \details Delete gateway of static IPv4 routes on specific interfaces.
 * \param interface_name [In] Specifies the name of the logical interface which this route belongs to;. This interface must refer to one of the defined interface sections.
 * \param ipaddr [In] Network address.
 * \param netmask [In] Route netmask. If omitted, 255.255.255.255 is assumed which makes target a host address.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_static_route_gateway(const char *interface_name, struct in_addr *ipaddr, struct in_addr *netmask);

/**
 * \brief Add a new firewall zone.
 * \details Add a new firewall zone to the network, basic settings(network, chain) should be added later.
 * \param zone_name [In] The name of firewall zone.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_firewall_zone(const char *zone_name);

/**
 * \brief Delete a firewall zone.
 * \details Delete a entire firewall zone include all internal settings.
 * \param zone_name [In] The name of firewall zone.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_firewall_zone(const char *zone_name);

/**
 * \brief Retrieve DNS specific suffix.
 * \details Retrieve DNS specific suffix.
 * \param dns_suffix [Out] Local domain suffix.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_dns_domain_suffix(string_64 dns_suffix);

/**
 * \brief Set DNS specific suffix.
 * \details Set DNS specific suffix.
 * \param dns_suffix [In] Local domain suffix.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_dns_domain_suffix(const char *dns_suffix);

/**
 * \brief Add a DNS domain binding information.
 * \details This function add detailed information typically include its name and IP address. Devices will be able to be visited by <domain_name>.<domain_suffix> instead of IP address after added in the network.
 * \param domain_name [In] DNS domain handed out to DHCP clients.
 * \param ipaddr [In] The IP address of DHCP clients.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_dns_domain_binding(const char *domain_name, struct in_addr *ipaddr);

/**
 * \brief Set a DNS domain binding information.
 * \details This function set detailed information about a domain based on its name. The domain information typically includes IP address.
 * \param domain_name [In] DNS domain handed out to DHCP clients.
 * \param ipaddr [In] The IP address of DHCP clients.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_dns_domain_binding(const char *domain_name, struct in_addr *ipaddr);

/**
 * \brief Retrieve a DNS domain binding information.
 * \details This function retrieves detailed information about a domain based on its name. The domain information typically includes IP address.
 * \param domain_name [In] DNS domain handed out to DHCP clients.
 * \param ipaddr [Out] The IP address of DHCP clients.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_dns_domain_binding(const char *domain_name, struct in_addr *ipaddr);

/**
 * \brief Delete a DNS domain binding information.
 * \details Delete a DNS domain binding information by its name.
 * \param domain_name [In] The DNS domain name for DHCP clients.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_dns_domain_binding(const char *domain_name);

/**
 * \brief Retrieve a DNS server address of the DHCP server.
 * \details This function retrieves the DNS server addresses configured for DHCP. It returns a list of IP addresses which are used by the DHCP server to provide DNS resolution services to clients.
 * \param netif_name [In] The name of the network interface.
 * \param server_addr [Out] The list of DNS servers to forward requests to, CLS-API malloc()ed array to carry out dns_server addresses, Caller MUST free() after using.
 * \return number of server address on success or negative error code on error.
 */
int clsapi_net_get_dns_server_addr(const char *netif_name, struct in_addr **server_addr);

/**
 * \brief Add DNS server address of DHCP server.
 * \details This function add the DNS server addresses to be used by the DHCP server for providing DNS resolution services to clients by using dhcp_option.
 * \param netif_name [In] The name of the network interface.
 * \param server_addr [In] DNS servers to forward requests to.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_dns_server_addr(const char *netif_name, const struct in_addr *server_addr);

/**
 * \brief Delete DNS server address of DHCP server.
 * \details This function deletes the DNS server addresses to be used by the DHCP server for providing DNS resolution services to clients by using dhcp_option.
 * \param netif_name [In] The name of the network interface.
 * \param server_addr [In] The DNS servers to forward requests to.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_dns_server_addr(const char *netif_name, const struct in_addr *server_addr);

/**
 * \brief Get DHCP lease information.
 * \details Retrieve the lease information of DHCP clients connected to the network. It provides details such as the MAC address, IP address, lease time, and hostname of each DHCP client. This information is essential for network management and monitoring purposes.
 * \param info [Out] The structure of DHCP lease info to output.
 * \param info_len [Out] Number of infos, input as large as possible and return filled number in return.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_dhcp_lease_info(struct dhcp_lease_info info[], int *info_len);

/**
 * \brief Get all firewall zone names.
 * \details This function gets all firewall zone names.
 * \param zone_names [Out] The array for all zone names.
 * \param zone_names_len [Out] Quantity of all zone names, input as large as possible and output real value.
 * \return 0 on success or others on error.
 * */
int clsapi_net_get_firewall_zone_names(string_128 zone_names[], int *zone_names_len);

/**
 * \brief Add a specified firewall forwarding.
 * \details This function add the specified forwarding based on src_zone and dest_zone. Only one direction is covered by a forwarding rule. To allow bidirectional traffic flows between two zones, two forwardings are required, with src and dest reversed in each.
 * \param src_zone [In] Specifies the traffic source zone. Refers to one of the defined zone names. For typical port forwards this usually is 'wan'.
 * \param dest_zone [In] Specifies the traffic destination zone. Refers to one of the defined zone names.
 * \return 0 on success or others on error.
 * */
int clsapi_net_add_firewall_zone_forwarding(const char *src_zone, const char *dest_zone);

/**
 * \brief Delete specified firewall forwarding.
 * \details This function delete the specified forwarding which based on src_zone and dest_zone.
 * \param src_zone [In] Specifies the traffic source zone. Refers to one of the defined zone names. For typical port forwards this usually is 'wan'.
 * \param dest_zone [In] Specifies the traffic destination zone. Refers to one of the defined zone names.
 * \return 0 on success or others on error.
 * */
int clsapi_net_del_firewall_zone_forwarding(const char *src_zone, const char *dest_zone);

/**
 * \brief Set policy of firewall zone.
 * \details Set the firewall policy for the network if existed, which determines the behavior of the firewall for incoming, outgoing and forward.
 * \param zone_name [In] The unique zone name.
 * \param chain [In] The chain of zone.
 \n	CLSAPI_FIREWALL_CHAIN_INPUT = 0
 \n	CLSAPI_FIREWALL_CHAIN_OUTPUT = 1
 \n	CLSAPI_FIREWALL_CHAIN_FORWARD = 2
 * \param policy [In] The policy of firewall zone.
 \n	CLSAPI_FIREWALL_POLICY_ACCEPT = 0
 \n	CLSAPI_FIREWALL_POLICY_REJECT = 1
 \n	CLSAPI_FIREWALL_POLICY_DROP = 2
 * \return 0 on success or others on error.
 */
int clsapi_net_set_firewall_zone_policy(const char *zone_name, enum clsapi_firewall_chain chain, enum clsapi_firewall_policy policy);

/**
 * \brief Get the policy of firewall zone.
 * \details Get the firewall policy for the network, which determines the behavior of the firewall for incoming, outgoing and forward.
 * \param zone_name [In] The unique zone name.
 * \param chain [In] The chain of zone.
 \n	CLSAPI_FIREWALL_CHAIN_INPUT = 0
 \n	CLSAPI_FIREWALL_CHAIN_OUTPUT = 1
 \n	CLSAPI_FIREWALL_CHAIN_FORWARD = 2
 * \param policy [Out] The policy of firewall zone.
 \n	CLSAPI_FIREWALL_POLICY_ACCEPT = 0
 \n	CLSAPI_FIREWALL_POLICY_REJECT = 1
 \n	CLSAPI_FIREWALL_POLICY_DROP = 2
 * \return 0 on success or others on error.
 */
int clsapi_net_get_firewall_zone_policy(const char *zone_name, enum clsapi_firewall_chain chain, enum clsapi_firewall_policy *policy);

/**
 * \brief Add network interface of firewall zone.
 * \details Add the firewall zone network, which determines the interface of the firewall zone, such as lan/wan.
 * \param zone_name [In] The unique zone name. 32 characters is the maximum working firewall zone name length.
 * \param network_name [In] Interfaces attached to this zone. If omitted and neither extra * options, subnets nor devices are given, the value of name is used by default.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_firewall_zone_network(const char *zone_name, const char *network_name);

/**
 * \brief Delete the network interface of a firewall zone.
 * \details Delete the network interface of a firewall zone.
 * \param zone_name [In] The unique zone name.
 * \param network_name [In] The interface name attached to this zone. If omitted and neither extra * options, subnets nor devices are given, the value of name is used by default.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_firewall_zone_network(const char *zone_name, const char *network_name);

/**
 * \brief Get network interface of firewall zone.
 * \details Get the firewall zone network.
 * \param zone_name [In] The unique zone name.
 * \param network_name [Out] List of interfaces attached to this zone. If omitted and neither extra * options, subnets nor devices are given, the value of name is used by default.
 * \return 0 on success and negative number for error, positive number for real length of network.
 */
int clsapi_net_get_firewall_zone_network(const char *zone_name, clsapi_ifname **network_name);

/**
 * \brief Add a DNS server address to WAN in the network.
 * \details Add a DNS server address(both IPv4 and IPv6 are supported) to WAN interface.
 * \param interface_name [In] The logical interface of network in Openwrt, such as wan/wan6...
 * \param dns_server [In] The address of DNS server.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_wan_dns_server(const char *interface_name, struct clsapi_net_ipaddr *dns_server);

/**
 * \brief Delete a DNS server address of WAN in the network.
 * \details Delete a DNS server address(both IPv4 and IPv6 are supported) from WAN interface.
 * \param interface_name [In] The logical interface of network in Openwrt, such as wan/wan6...
 * \param dns_server [In] The address of DNS server.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_wan_dns_server(const char *interface_name, struct clsapi_net_ipaddr *dns_server);

/**
 * \brief Get all DNS server addresses of WAN in the Network.
 * \details Get all DNS server addresses(both IPv4 and IPv6 are supported) from the WAN interface.
 * \param interface_name [In] The logical interface of network in Openwrt, such as wan/wan6...
 * \param dns_server [Out] CLS-API malloc()ed array to carry out dns_server addresses, Caller MUST free() after using.
 * \return number of server address on success or negative error code on error.
 */
int clsapi_net_get_wan_dns_server(const char *interface_name, struct clsapi_net_ipaddr **dns_server);

/**
 * \brief Get the PPPoE username of WAN.
 * \details Get the username of PPPoE account in the local netwrok, which usually works on WAN.
 * \param interface_name [In] The name of the logical network interface. Such as "wan" or "wan6".
 * \param username [Out] The username of PPPoE account.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_pppoe_username(const char *interface_name, string_32 username);

/**
 * \brief Set the PPPoE username of WAN.
 * \details Set the username of PPPoE account in the local netwrok, which usually works on WAN.
 * \param interface_name [In] The name of the logical network interface. Such as "wan" or "wan6".
 * \param username [In] The username of PPPoE account.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_pppoe_username(const char *interface_name, const char *username);

/**
 * \brief Get the PPPoE password of WAN.
 * \details Get the password of PPPoE account in the local netwrok, which usually works on WAN.
 * \param interface_name [In] The name of the logical network interface. Such as "wan" or "wan6".
 * \param passwd [Out] The password of PPPoE account.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_pppoe_passwd(const char *interface_name, string_32 passwd);

/**
 * \brief Set the PPPoE password of WAN.
 * \details Set the password of PPPoE account in the local netwrok, usually works on WAN.
 * \param interface_name [In] The name of the logical network interface. Such as "wan" or "wan6".
 * \param passwd [In] The password of PPPoE account.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_pppoe_passwd(const char *interface_name, const char *passwd);

/**
 * \brief Get the unicast global IPv6 prefix and prefix length.
 * \details Get the unicast global IPv6 prefix and its prefix length by its device interface.
 * \param netif_name [In] The name of the network interface, such as "br-lan", "eth0".
 * \param ipv6prefix [Out] IPv6 prefix to pass out;
 * \param ipv6prefix_len [Out] IPv6 prefix length to pass out;
 * \return 0 on success or others on error.
 */
int clsapi_net_get_uni_global_ipv6prefix(const char *netif_name, string_256 ipv6prefix, uint8_t *ipv6prefix_len);

/**
 * \brief Get ULA global IPv6 prefix and prefix length.
 * \details Get the ULA global IPv6 prefix and its prefix length by its interface.
 * \param netif_name [In] The name of the network device interface, such as "br-lan", "eth0".
 * \param ipv6prefix [Out] IPv6 prefix to pass out;
 * \param ipv6prefix_len [Out] IPv6 prefix length to pass out;
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ula_global_ipv6prefix(const char *netif_name, string_256 ipv6prefix, uint8_t *ipv6prefix_len);

/**
 * \brief Get all IPv6 local addresses.
 * \details Get all IPv6 local addresses by its interface. Sorted by GLOBAL address/ LINK address/Other address, all special IPv6 address refer to clsapi_ipv6addr_classfication.
 * \param netif_name [In] The name of the network interface, such as "br-lan", "eth0".
 * \param ipv6_addrs [Out] CLS-API malloc()ed array to carry out all IPv6 addresses, Caller MUST free() after using.
 \n	CLSAPI_ADDR_GLOBAL = 0
 \n	CLSAPI_ADDR_LINKLOCAL = 1
 \n	CLSAPI_ADDR_ULA_LOCAL = 2
 \n	CLSAPI_ADDR_OTHERS = 3
 * \return number of ipv6addresses on success or negative error code on error.
 */
int clsapi_net_get_all_ipv6addrs(const char *netif_name, struct clsapi_ipv6_info **ipv6_addrs);

/**
 * \brief Get default IPv4 gateway of specified interface.
 * \details This function retrieves the default gateway for IPv4 from the system routing table.
 * \param netif_name [In] The name of the network interface, such as "eth0(WAN)" or other non-static interface.
 * \param ipaddr [Out] Structures for default gateway address(v4) to pass out.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_default_gateway(const char *netif_name, struct in_addr *ipaddr);

/**
 * \brief Get default IPv6 gateway of specified interface.
 * \details This function retrieves the default gateway addresses for IPv6 from the system routing table.
 * \param netif_name [In] The name of the network device interface, such as "eth0(WAN)" or other non-static interface.
 * \param ipaddr [Out] Structures for default gateway address(v6) to pass out.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6_default_gateway(const char *netif_name, struct in6_addr *ipaddr);

/**
 * \brief Get the firewall rule by rule name.
 * \details Get the firewall rule by rule name.
 * \param rule [In] Rule name
 * \return 0 on success or others on error.
 */
int clsapi_net_get_firewall_rule(struct clsapi_net_firewall_rule *rule);

/**
 * \brief Set the firewall rule by rule name.
 * \details Set the firewall rule by rule name.
 * \param rule [In] Rule name
 * \return 0 on success or others on error.
 */
int clsapi_net_set_firewall_rule(struct clsapi_net_firewall_rule *rule);

/**
 * \brief Delete the firewall rule by rule name.
 * \details Delete the firewall rule by rule name.
 * \param rule [In] Rule name
 * \return 0 on success or others on error.
 */
int clsapi_net_del_firewall_rule(struct clsapi_net_firewall_rule *rule);

/**
 * \brief Get the IPv6 connect status of specified interface.
 * \details Get the IPv6 connect status of specified interface.
 * \param interface_name [In] The name of the network device interface, such as "eth0(WAN)" or other non-static interface.
 * \param connected [Out] IPv6 connected or not.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6ConnStatus(const char *interface_name, bool *connected);

/**
 * \brief Get the IPv6 prefix of the specified interface is delegated or not.
 * \details Get the IPv6 prefix of the specified interface is delegated or not.
 * \param interface_name [In] The name of the network device interface, such as "br-lan(LAN)" or other static interface.
 * \param enabled [Out] IPv6 prefix delegated or not.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6prefixDelegationEnabled(const char *interface_name, bool *enabled);

/**
 * \brief Get the D-Slite status of the specified interface.
 * \details Get the D-Slite status of the specified interface.
 * \param interface_name [In] The name of the network interface, such as "wan" or "lan".
 * \param enabled [Out] D-Slite enabled or not.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6DSliteEnabled(const char *interface_name, bool *enabled);

/**
 * \brief Get the IPv6 address allocation of a specified interface.
 * \details Get the IPv6 address allocation of a specified interface.
 * \param interface_name [In] The name of the network interface, such as "wan" or "lan".
 * \param alloc_method [Out] IPv6 address allocation.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6AddrAlloc(const char *interface_name, enum clsapi_ipv6addr_allocation *alloc_method);

/**
 * \brief Get the IPv6 prefix allocation of a specified interface.
 * \details Get the IPv6 prefix allocation of a specified interface.
 * \param interface_name [In] The name of the network interface, such as "wan" or "lan".
 * \param allocation [Out] IPv6 prefix allocation.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6PrefixAlloc(const char *interface_name, enum clsapi_ipv6prefix_allocation *allocation);

/**
 * \brief Add the bridge vlan to the network.
 * \details Add the bridge vlan to the netwrok with its specified device name and vlan_id.
 * \param bridge_name [In] The name of the bridge-vlan such as "br-lan".
 * \param vlan_id [In] Bridge-vlan ID.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_bridge_vlan(const char *bridge_name, const uint16_t vlan_id);

/**
 * \brief Delete the bridge vlan from the network.
 * \details Delete the bridge vlan from the netwrok with its specified device name and vlan_id.
 * \param bridge_name [In] The name of the bridge-vlan such as "br-lan".
 * \param vlan_id [In] Bridge-vlan ID.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_bridge_vlan(const char *bridge_name, const uint16_t vlan_id);

/**
 * \brief Enable the bridge vlan of the network.
 * \details Enable the bridge vlan of the netwrok with its specified device name and vlan_id.
 * \param bridge_name [In] The name of the bridge-vlan such as "br-lan".
 * \param enabled [In] Enable or disable the br-vlan.
 * \return 0 on success or others on error.
 */
int clsapi_net_enable_bridge_vlan(const char *bridge_name, const bool enabled);

/**
 * \brief Add the port to the bridge vlan of the network.
 * \details Add the port to the bridge vlan of the netwrok with its specified device name, vlan_id, tagged/pvid or not.
 * \param bridge_name [In] The name of the bridge-vlan such as "br-lan".
 * \param vlan_id [In] Bridge-vlan ID.
 * \param port [In] The device which will be added to the br-vlan.
 * \param tagged [In] The port egress is tagged or not.
 * \param pvid [In] The port is pvid or not.
 * \return 0 on success or others on error.
 */
int clsapi_net_add_bridge_vlan_port(const char *bridge_name, const uint16_t vlan_id,
		const char *port, const bool tagged, const bool pvid);

/**
 * \brief Delete the port from the bridge vlan of the network.
 * \details Delete the port from the bridge vlan of the netwrok with its specified device name and vlan_id.
 * \param bridge_name [In] The name of the bridge-vlan such as "br-lan".
 * \param vlan_id [In] Bridge-vlan ID.
 * \param port [In] The device which will be deleted from br-vlan.
 * \return 0 on success or others on error.
 */
int clsapi_net_del_bridge_vlan_port(const char *bridge_name, const uint16_t vlan_id, const char *port);

/**
 * \brief Set the bridge vlan is primary or not.
 * \details Set the bridge vlan is primary or not with its specified device name and vlan_id.
 * \param bridge_name [In] The name of the bridge-vlan such as "br-lan".
 * \param vlan_id [In] Bridge-vlan ID.
 * \param primary [In] The br-vlan is primary or not.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_bridge_vlan_primary(const char *bridge_name, const uint16_t vlan_id, const bool primary);

/**
 * \brief GET IPv6 prefix origin.
 * \details GET IPv6 prefix origin.
 * \param netif_name [In] The name of the network device interface, such as "eth0(WAN)", "br-lan(LAN).
 * \param prefix_origin [Out] The type of ipv6 prefix origin.
 * \return 0 on success or others on error.
 */
int clsapi_net_get_ipv6PrefixOrigin(const char *netif_name, enum clsapi_ipv6_prefix_origin *prefix_origin);

/**
 * \brief Set IPv6 prefix origin.
 * \details Set IPv6 prefix origin.
 * \param netif_name [In] The name of the network device interface, such as "eth0(WAN)", "br-lan(LAN)".
 * \param prefix_origin [In] The type of ipv6 prefix origin, Static: 0, Prefix Delegation: 1.
 * \return 0 on success or others on error.
 */
int clsapi_net_set_ipv6PrefixOrigin(const char *netif_name, enum clsapi_ipv6_prefix_origin prefix_origin);
/*!@} */

#endif /* _CLSAPI_NET_H */

