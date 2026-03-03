/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "clsapi_base.h"
#include "clsapi_net.h"
#include "iwinfo.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "net_common.h"
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#define IP_ADDRESS_BITS 32
#define MAX_LINE_LEN 256
#define IF_INET6_PATH "/proc/net/if_inet6"
const char *cls_wifi_ifname_fmt = "wlan%d-%d";

/* This function is just for fix following warnings which introduced in iwinfo.so.
 * This function should be deleted if the warnings went away.
 *   iwinfo.h:217:17: warning: 'vht_chan_width' defined but not used [-Wunused-variable]
 *   iwinfo.h:212:17: warning: 'ht_chan_width' defined but not used [-Wunused-variable]
 *   iwinfo.h:204:20: warning: 'ht_secondary_offset' defined but not used [-Wunused-variable]
 */
void __just_for_fix_warnings(void)
{
	int dummy = 0;
	const char *dummy_ptr = ht_secondary_offset[0];

	dummy = ht_chan_width[0];
	dummy += vht_chan_width[0];
	dummy += dummy_ptr[0];
}

/** Parse ifname and retrieve radio index and interface/bss index
 * Note: radio_idx and/or if_idx returns -1 means they are NOT scanfed.
 * Returns:
 *   >0: OK
 *   <0: Error
 */
static inline int parse_ifname(const char *ifname, int *radio_idx, int *if_idx)
{
	int scan_cnt;
	int local_phyidx = -1, local_ifidx = -1;

	if (!ifname)
		return -1;

	scan_cnt = sscanf(ifname, cls_wifi_ifname_fmt, &local_phyidx, &local_ifidx);
	if (scan_cnt <= 0)
		return -1;

	if (radio_idx)
		*radio_idx = local_phyidx;
	if (if_idx)
		*if_idx = local_ifidx;

	return scan_cnt;
}

/*
 * convert Wi-Fi ifname to section name of uci 'wireless'.
 * OpenWrt ifname generation rules:
 *   o OpenWrt default Wi-Fi ifname = 'wlan<radio_idx>[-if_idx]', omit [-if_idx] if if_idx=0
 *   o if 'ifname' option presented ==> ifname = value of 'ifname' option
 * Inputs:
 *   ifname: Wi-Fi interface name
 *   radio_sct:	buffer to carry the radio section for the ifname; if it's NULL, omit radio section
 *   rdo_sec_len: buffer len of radio_sct
 *   bss_sct: buffer to carry the interface/bss section for the ifname; if it's NULL, omit radio section
 *   bss_sec_len: buffer len of bss_sct
 * Returns:
 *   0:  Success and section name in 'radio_sct' and/or ''bss_sct' param
 *   !0: Errors
 */
static int ifname_to_uci_section(const char *ifname, char *radio_sct,
		const int rdo_sct_len, char *bss_sct, const int bss_sct_len)

{
	int if_idx = 0;
	int ret = CLSAPI_OK;
	char radio_name_fmt[] = "radio%d";
	char target_dev[32] = {0};
	char local_bss_sct[32] = {0};
	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	struct uci_element *e = NULL;
	int target_phy_idx = -1, target_if_idx = -1;

	if (parse_ifname(ifname, &target_phy_idx, &target_if_idx) <= 0)
		return -CLSAPI_ERR_INVALID_IFNAME;

	if (target_if_idx == -1)
		target_if_idx = 0;

	sprintf(target_dev, radio_name_fmt, target_phy_idx);
	ctx = uci_alloc_context();
	if (!ctx) {
		DBG_ERROR("UCI error--uci_alloc_context() fail.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	uci_load(ctx, CLSCONF_CFG_WIRELESS, &pkg);
	if (!pkg) {
		DBG_ERROR("UCI error--uci_load() fail.\n");
		ret = -1;
		goto out;
	}

	uci_foreach_element(&pkg->sections, e) {
		const char *opt_ifname, *opt_dev;
		struct uci_section *s = NULL;

		s = uci_to_section(e);

		if (strcmp(s->type, "wifi-iface") != 0)
			continue;

		opt_ifname = uci_lookup_option_string(ctx, s, "ifname");
		opt_dev = uci_lookup_option_string(ctx, s, "device");
		if (!opt_dev)  /* 'device' is mandatory option in wifi-iface */
			continue;

		/* set wifi-iface as OpenWrt default if it does NOT have ifname option
		 * handle OpenWrt default ifname based on phy index and ifname index
		 */
		if (!opt_ifname) {
			/* is target device */
			if (strcmp(opt_dev, target_dev) == 0) {
				if (if_idx == target_if_idx) {
					/* found it */
					cls_strncpy(local_bss_sct, s->e.name, sizeof(local_bss_sct));
					ret = CLSAPI_OK;
					goto out;
				} else
					if_idx++; /* go to next ifname in same device to handle the type wifiX-Y */
			}
		} else {
			/* handle the common ifname from UCI, such as wifiX-Y */
			if (strcmp(opt_ifname, ifname) == 0) {
				/* found it,  given 'ifname' in 'wireless' uci */
				cls_strncpy(local_bss_sct, s->e.name, sizeof(local_bss_sct));
				goto out;
			}
		}
	}

	DBG_ERROR("Invalid data--could not find any UCI section.\n");
	ret = -CLSAPI_ERR_NOT_FOUND;

out:
	if (radio_sct)
		cls_strncpy(radio_sct, target_dev, rdo_sct_len);
	if (bss_sct)
		cls_strncpy(bss_sct, local_bss_sct, bss_sct_len);
	if (pkg)
		uci_unload(ctx, pkg);
	if (ctx)
		uci_free_context(ctx);

	return ret;
}

/** Get section name in "wireless" from ifname
 * Inputs:
 *   ifname:      Wi-Fi interface name
 *   radio_sct:   buffer to carry the radio section for the ifname; if it's NULL, omit radio section
 *   rdo_sec_len: buffer len of radio_sct
 *   bss_sct:     buffer to carry the interface/bss section for the ifname; if it's NULL, omit radio section
 *   bss_sec_len: buffer len of bss_sct
 * Returns:
 *   0:  Success and section name in 'radio_sct' and/or ''bss_sct' param
 *   !0: Errors
 */
static int clsconf_ifname_to_section(const char *ifname, char *radio_sct, const int rdo_sct_len,
				char *bss_sct, const int bss_sct_len)

{
#ifdef CLSAPI_PLAT_OPENWRT
	return ifname_to_uci_section(ifname, radio_sct, rdo_sct_len, bss_sct, bss_sct_len);
#else
	#error No platform defined
#endif
}

/*****************************	Data type definitions	**************************/


/*****************************	Variables definitions	**************************/


/*************************	C-Call functions declarations	**********************/
/*
 * \brief Convert hex string IPv6 address to normal IPv6 address string.
 * \param hex [in] String refer to hex string IPv6 address.
 * \param ipv6 [in] String address to store normal IPv6 address.
 * \param prefix_len [in] Prefix length of IPv6 address.
 */
static inline void converthex2IPv6(const char *hex, char *ipv6, int prefix_len)
{
    for (int i = 0; i < prefix_len; i++) {
        if (i > 0 && i % 4 == 0) {
            *ipv6++ = ':';
        }
        *ipv6++ = hex[i];
    }
    *ipv6 = '\0';
}

/*
 * \breif Validate weekdays is legal or not.
 * \param weekdays [in] Array for weekdays passed in to check.
 * \param length [in] Length of weekdays array.
 * \return true on success or false on fail.
 * */
static inline bool validate_weekdays(string_32 weekdays[], int length)
{
	bool found = false;
	char *week[] = {"!", "Mon", "Tue", "Wed", "Thu", "Fri", "Sta", "Sun"};

	for (int i = 0; i < length; i++) {
		if (strcmp(weekdays[i], "\0") == 0)
			continue;
		for (int j = 0; j < ARRAY_SIZE(week); j++) {
			if (strcmp(weekdays[i], week[j]) == 0)
				found = true;
		}
		if (!found)
			return false;
	}

	for (int i = 0; i < length; i++) {
		for (int j = i + 1; j < length; j++) {
			if (strcmp(weekdays[i], "\0") == 0)
				continue;
			if (strcmp(weekdays[i], weekdays[j]) == 0)
			{
				return false;
			}
		}
	}

	return true;
}

/*
 * \breif Validate icmp type.
 * \param rule [in] The structures include icmp type.
 * \return 0 on success or others on errors.
 * */
static inline int validate_icmp_type(struct clsapi_net_firewall_rule *rule)
{
	bool found = false;
	char *icmp_type[] = {
		"address-mask-reply", "host-redirect", "pong",
		"time-exceeded", "address-mask-request", "host-unknown",
		"port-unreachable",	"timestamp-reply", "any", "host-unreachable",
		"precedence-cutoff", "timestamp-request", "communication-prohibited",
		"ip-header-bad", "protocol-unreachable", "TOS-host-redirect",
		"destination-unreachable", "network-prohibited", "redirect",
		"TOS-host-unreachable", "echo-reply",	"network-redirect",
		"required-option-missing", "TOS-network-redirect", "echo-request",
		"network-unknown", "router-advertisement",	"TOS-network-unreachable",
		"fragmentation-needed",	"source-route-failed", "ttl-zero-during-transit",
		"network-unreachable",	"router-solicitation",	"ttl-exceeded",
		"host-precedence-violation", "parameter-problem", "source-quench",
		"ttl-zero-during-reassembly", "host-prohibited", "ping",
	};

	for (int i = 0; i < ARRAY_SIZE(rule->icmp_type); i++) {
		for (int j = 0; j < ARRAY_SIZE(icmp_type); j++) {
			if (strcmp(rule->icmp_type[i], icmp_type[j]) == 0)
				found = true;
		}
		if (!found)
			return -CLSAPI_ERR_INVALID_PARAM;
	}

	return CLSAPI_OK;
}

/*
 * \brief Validate firewall rule protocol.
 * \param proto [in] Protocol for firewall rule.
 * \return 0 on success or others on errors.
 * * */
static inline int validate_firewall_rule_protocol(char *proto)
{
	FILE *fp = NULL;
	char line[512] = {0};
	char protocol_name[512] = {0};
	int protocol_number = 0;
	int num_proto = -1;

	if (strcmp(proto, "0") == 0 || strcmp(proto, "all") == 0)
		num_proto = 0;
	else if (atoi(proto) != 0)
		num_proto = atoi(proto);

	fp = fopen("/etc/protocols", "r");
	if (fp == NULL)
		return -CLSAPI_ERR_FILE_OPERATION;

	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '#' || line[0] == '\n')
			continue;

		sscanf(line, "%s %d", protocol_name, &protocol_number);
		if (strcmp(protocol_name, proto) == 0 || protocol_number == num_proto) {
			fclose(fp);
			return CLSAPI_OK;
		}
	}

	fclose(fp);

	return -CLSAPI_ERR_INVALID_PARAM;
}

/*
 * \brief Vaalidate conntrack helper.
 * \param conn_helper [in] Conntrack helper.
 * \return true on success or false on fail.
 * */
static inline bool validate_conntrack_helper(const char *conn_helper)
{
	bool found = false;
	char *helper[] = {
		"amanda", "ftp", "ras",	"Q.931","irc", "tftp",
		"netbios-ns", "pptp", "sane", "sip", "snmp","rtsp"
	};

	for (int i = 0; i < ARRAY_SIZE(helper); i++) {
		if (strcmp(helper[i], conn_helper) == 0)
			found = true;
	}

	return found;
}

/*
 * \brief Vaalidate DSCP classfication.
 * \param dscp_class [in] DSCP classfication.
 * \return true on success or false on fail.
 * */
static inline bool validate_dscp_class(char *dscp_class)
{
	bool found = false;
	char *dscp[] = {
		"CS0", "CS1", "CS2", "CS3", "CS4", "CS5", "CS6", "CS7",
		"AF11", "AF12", "AF13", "AF21", "AF22", "AF23", "AF31",
		"AF32", "AF33", "AF41", "AF42", "AF43", "BE", "EF"
	};

	for (int i = 0; i < ARRAY_SIZE(dscp); i++) {
		if (strcmp(dscp[i], dscp_class) == 0)
			found = true;
	}
	if (strtol(dscp_class, NULL, 16) > 0 ||
			strtol(dscp_class, NULL, 16) < 0xff)
		found = true;

	return found;
}

/*
 * \brief Get parameter number from entire string.
 * \param str [in] The entire string parameters.
 * \return numbers for independent parameters.
 * */
static inline int get_param_num(char *str)
{
	int count = 0;

	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] == ' ') {
			count++;
		}
	}

	return count + 1;
}

/*
 * \brief Split DNS server addresses in DHCP options.
 * \details Input address string in DHCP options, handle string into address array.
 * \param str_address [in] Address string in DHCP options.
 * \param address [out] Address array output.
 * \param addr_len [out] Length of address array, input as large as possible, output real length.
 * \return 0 on success, others on errors.
 * */
static inline int split_dns_address_in_dhcp_op(char *str_address, string_32 address[], int *addr_len)
{
	char *token = ",";
	string_1024 tmp_address = {0};
	char *tmp = NULL;
	char *tmp_end = NULL;
	int i = 0;

	cls_strncpy(tmp_address, str_address, sizeof(tmp_address));
	tmp = strtok(tmp_address, token);
	while (tmp != NULL) {
		if (i >= *addr_len)
			return -CLSAPI_ERR_INVALID_DATA;
		while (*tmp == ' ')
			tmp++;
		tmp_end = tmp + strlen(tmp) - 1;
		while (*tmp_end == ' ')
			tmp_end--;
		*(tmp_end + 1) = '\0';

		cls_strncpy(address[i], tmp, sizeof(string_32));
		tmp = strtok(NULL, token);
		i++;
	}
	*addr_len = i;

	return CLSAPI_OK;
}

/*
 * \brief Convert netmask to prefix length.
 * \param netmask [in] Netmask of string.
 * \return prefix length of netmask.
 **/
static inline int netmask_to_prefix_length(const char *netmask)
{
	int count = 0;
	uint32_t hex_netmask = 0;

	hex_netmask = inet_addr(netmask);
	while (hex_netmask) {
		count += hex_netmask & 1;
		hex_netmask >>= 1;
	}

	return count;
}

/* \brief Devide network address(IP address/netmask) into array
 * \param netaddr_string [in] Address inputed. Such as IP address.
 * \param addr [out] Address output by array.
 * \return 0 on success or others on error.
 **/
static inline int netaddr_segmentation(char *netaddr_string, uint8_t addr[4])
{
	char *token = ".";
	char *tmp = NULL;
	int i = 0;

	if (!netaddr_string || !addr)
		return -CLSAPI_ERR_INVALID_PARAM;

	tmp = strtok(netaddr_string, token);
	while(tmp != NULL) {
		addr[i] = atoi(tmp);
		tmp = strtok(NULL, token);
		i++;
	}

	return CLSAPI_OK;
}

/* \brief Check protocol is validated or not.
 * \param proto [in] The protocol string will be passed in to check.
 * \return 0 on success or others on error.
 */
static inline int protocol_validate(const char *proto)
{
	if (!proto)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strcmp(proto, "dhcp") != 0 &&
		strcmp(proto, "dhcpv6") != 0 &&
		strcmp(proto, "static") != 0 &&
		strcmp(proto, "pppoe") != 0)
		return -CLSAPI_ERR_INVALID_PARAM;
	else
		return CLSAPI_OK;
}

/*
 * \brief Check netmask is validated or not
 * \param str_netmask [in] The netmask string will be passed in to check.
 * \return 0 on success or others on error.
 * */
static inline int netmask_validate(const char *str_netmask)
{
	struct in_addr netmask;
	unsigned int tmp;

	if (inet_aton(str_netmask, &netmask) == 0)
			return -CLSAPI_ERR_INVALID_PARAM;
	tmp = ntohl(netmask.s_addr);
	tmp = ~tmp;
	if (tmp & (tmp + 1))
			return -CLSAPI_ERR_INVALID_PARAM;

	return CLSAPI_OK;
}

/*
 * \brief Validate network interface name.
 * \details Input name of network device interface, this API will check the name is existed or not. if return 0, represent success, or error on error code.
 * \param netdev_name the name of network device.
 * \return 0 on success or others on error.
 *
 * */
static inline int netdev_netif_name_validate(const char *netdev_name)
{
	struct ifaddrs *ifaddr, *ifa;
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 section_name = {0};

	// get all netdevice interface names
	if (!netdev_name) {
		DBG_ERROR("Network interface name error\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (getifaddrs(&ifaddr) == -1) {
		DBG_ERROR("Fail to getifaddrs\n");
		return -CLSAPI_ERR_INTERNAL_ERR;
	}

	//iterate all the interface to check the interface name if existed such as eth0/eth1/xxx
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_PACKET) {
			if (!strcmp(ifa->ifa_name, netdev_name)) {
				ret = CLSAPI_OK;
			}
		}
	}
	freeifaddrs(ifaddr);

	//check especially for bridge device such as br-lan/br-lan1
	if (ret != CLSAPI_OK) {
		ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_DEVICE,
				section_name, 2, CLS_NETWORK_PARAM_NAME, netdev_name);
		if (ret != CLSAPI_OK)
			ret = -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

static inline int get_netdev_info(const char *devname, struct netdev_info *info)
{
	int ret = CLSAPI_OK;
	const char *bridge_name = NULL;
	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	struct uci_element *e = NULL;

	ctx = uci_alloc_context();
	if (!ctx) {
		DBG_ERROR("UCI error--uci_alloc_context() fail.\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	uci_load(ctx, CLSCONF_CFG_NETWORK, &pkg);
	if (!pkg) {
		DBG_ERROR("UCI error--uci_load() fail.\n");
		ret = -1;
		goto out;
	}

	uci_foreach_element(&pkg->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (strcmp(s->type, "device") == 0)
			bridge_name = uci_lookup_option_string(ctx, s, "name");

		if (strcmp(s->type, "interface") != 0)
			continue;

		const char *s_name = s->e.name;
		const struct uci_option *ip6addr = uci_lookup_option(ctx, s, "ip6addr");
		const char *proto = uci_lookup_option_string(ctx, s, "proto");
		const char *device = uci_lookup_option_string(ctx, s, "device");
		if (!device || !bridge_name)
			continue;

		if (strcmp(device, devname) == 0 && strcmp(bridge_name, devname) != 0) {
			strcpy(info->zone, "wan");
			if (proto && !strcmp(proto, "dhcpv6")) {
				strcpy(info->section_name, s_name);
				goto out;
			} else if (proto && !strcmp(proto, "static") && ip6addr) {
				strcpy(info->section_name, s_name);
				goto out;
			}
		} else if (strcmp(bridge_name, devname) == 0) {
			strcpy(info->zone, "lan");
			strcpy(info->section_name, s_name);
			goto out;
		} else {
			struct uci_element *de;

			uci_foreach_element(&pkg->sections, de) {
				struct uci_section *ds = uci_to_section(de);
				if (strcmp(ds->type, "device") != 0)
					continue;

				const char *name = uci_lookup_option_string(ctx, ds, "name");
				if (!name || strcmp(name, bridge_name) != 0)
					continue;

				const struct uci_option *opt = uci_lookup_option(ctx, ds, "ports");
				if (!opt || opt->type != UCI_TYPE_LIST)
					return -CLSAPI_ERR_NOT_FOUND;

				struct uci_element *p;
				uci_foreach_element(&opt->v.list, p) {
					if (strcmp(p->name, devname) == 0) {
						strcpy(info->zone, "lan");
						strcpy(info->section_name, s_name);
						goto out;
					}
				}
			}
		}
	}
out:
	if (pkg)
		uci_unload(ctx, pkg);
	if (ctx)
		uci_free_context(ctx);

	return ret;
}

int clsapi_net_get_opmode(enum clsapi_net_opmode *opmode)
{
	int ret = CLSAPI_OK;
	string_1024 str_opmode = {0};
	string_1024 sta_phyname = {0};

	ret = clsconf_get_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, CLS_OPMODE_PARAM_MODE, str_opmode);
	if (strcmp(str_opmode, "repeater") == 0) {
		ret = clsconf_get_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, "sta_phyname", sta_phyname);
		if (strcmp(sta_phyname, "radio0") == 0)
			*opmode = CLSAPI_NETWORK_OPMODE_REPEATER;
		else if (strcmp(sta_phyname, "radio1") == 0)
			*opmode = CLSAPI_NETWORK_OPMODE_REPEATER_5G;
	} else
		*opmode = network_opmode_str2enum(str_opmode);

	return ret;
}


int clsapi_net_set_opmode(enum clsapi_net_opmode opmode)
{
	int ret = CLSAPI_OK;
	enum clsapi_net_opmode old_opmode;
	string_32 str_opmode = {0};

	ret = clsapi_net_get_opmode(&old_opmode);
	if (ret < 0)
		return ret;

	if (old_opmode == opmode)
		return CLSAPI_OK;

	switch (opmode) {
		case CLSAPI_NETWORK_OPMODE_ROUTER:
			strncpy(str_opmode, "router", sizeof(str_opmode));
			clsconf_defer_apply_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, CLS_OPMODE_PARAM_MODE, str_opmode);
			break;

		case CLSAPI_NETWORK_OPMODE_BRIDGE:
			strncpy(str_opmode, "bridge", sizeof(str_opmode));
			clsconf_defer_apply_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, CLS_OPMODE_PARAM_MODE, str_opmode);
			break;

		case CLSAPI_NETWORK_OPMODE_REPEATER:
			strncpy(str_opmode, "repeater", sizeof(str_opmode));
			clsconf_set_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, CLS_OPMODE_PARAM_MODE, str_opmode);
			clsconf_defer_apply_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, "sta_phyname", "radio0");
			break;

		case CLSAPI_NETWORK_OPMODE_REPEATER_5G:
			strncpy(str_opmode, "repeater", sizeof(str_opmode));
			clsconf_set_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, CLS_OPMODE_PARAM_MODE, str_opmode);
			clsconf_defer_apply_param(CLSCONF_CFG_CLS_OPMODE, CLS_OPMODE_SECT_GLOBALS, "sta_phyname", "radio1");
			break;
		case _CLSAPI_NETWORK_OPMODE_MAX:
		default:
			DBG_ERROR("Invalid parameter: opmode is not supported\n");
			return -CLSAPI_ERR_INVALID_PARAM;
	}

	return ret;
}

int clsapi_net_get_macaddr(const char *netif_name, uint8_t macaddr[ETH_ALEN])
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	int fd = 0;
	struct ifreq ifr;

	if (netdev_netif_name_validate(netif_name) != 0 || !macaddr)
		return -CLSAPI_ERR_INVALID_PARAM;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd == -1) {
		DBG_ERROR("Failed to create socket\n");
		return -CLSAPI_ERR_INTERNAL_ERR;
	}

	cls_strncpy(ifr.ifr_name, netif_name, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
		DBG_ERROR("Ioctl SIOCGIFHWADDR error\n");
		close(fd);
		return -CLSAPI_ERR_INTERNAL_ERR;
	}
	ret = CLSAPI_OK;
	close(fd);
	memcpy(macaddr, ifr.ifr_hwaddr.sa_data, sizeof(uint8_t) * ETH_ALEN);

	return ret;
}

int clsapi_net_get_speed(const char *netif_name, enum clsapi_net_speed_type *speed)
{
	char filename[64];
	FILE *file;
	int int_speed = 0;
	int ret = -CLSAPI_ERR_INVALID_PARAM;

	if (netdev_netif_name_validate(netif_name) != 0 || !speed)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(filename, sizeof(filename), "/sys/class/net/%s/speed", netif_name);

	file = fopen(filename, "r");
	if (file == NULL) {
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	// get integer speed value from the file
	ret = fscanf(file, "%d", &int_speed);
	if (ret != 1) {
		DBG_ERROR("Failed to read interface speed\n");
		fclose(file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	ret = CLSAPI_OK;
	fclose(file);

	switch (int_speed) {
	case CLSAPI_NET_SPEED_10M:
		*speed = CLSAPI_NET_SPEED_10M;
		break;
	case CLSAPI_NET_SPEED_100M:
		*speed = CLSAPI_NET_SPEED_100M;
		break;
	case CLSAPI_NET_SPEED_1000M:
		*speed = CLSAPI_NET_SPEED_1000M;
		break;
	case CLSAPI_NET_SPEED_2500M:
		*speed = CLSAPI_NET_SPEED_2500M;
		break;
	case CLSAPI_NET_SPEED_5000M:
		*speed = CLSAPI_NET_SPEED_5000M;
		break;
	case CLSAPI_NET_SPEED_10000M:
		*speed = CLSAPI_NET_SPEED_10000M;
		break;
	default:
		*speed = _CLSAPI_NET_SPEED_TYPE_MAX;
		break;
	}

	return ret;
}

int clsapi_net_get_stats(const char *netif_name, struct clsapi_net_statistics *netstat)
{
	int ret = CLSAPI_OK;
	char file_path[64] = {0};
	uint64_t *p_net = (uint64_t *)netstat;
	char detail_path[128] = {0};
	uint64_t value = 0;
	FILE *fp = NULL;
	char *member[] = {
		"collisions",
		"rx_frame_errors",
		"tx_compressed",
		"multicast",
		"rx_length_errors",
		"tx_dropped",
		"rx_bytes",
		"rx_missed_errors",
		"tx_errors",
		"rx_compressed",
		"rx_over_errors",
		"tx_fifo_errors",
		"rx_crc_errors",
		"rx_packets",
		"tx_heartbeat_errors",
		"rx_dropped",
		"tx_aborted_errors",
		"tx_packets",
		"rx_errors",
		"tx_bytes",
		"tx_window_errors",
		"rx_fifo_errors",
		"tx_carrier_errors"
	};

	if (netdev_netif_name_validate(netif_name) != 0 || !netstat)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(file_path, sizeof(file_path), "/sys/class/net/%s/statistics/", netif_name);
	for (int i = 0; i < ARRAY_SIZE(member); i++) {
		snprintf(detail_path, sizeof(detail_path), "%s%s", file_path, member[i]);
		fp = fopen(detail_path, "r");
		if (fp != NULL) {
			if (!fscanf(fp, "%lu", &value)) {
				DBG_ERROR("File operation failed\n");
				ret = -CLSAPI_ERR_FILE_OPERATION;
			} else
				p_net[i] = value;
		} else {
			DBG_ERROR("File operation failed\n");
			ret = -CLSAPI_ERR_FILE_OPERATION;
		}
		fclose(fp);
	}

	return ret;
}

int clsapi_net_get_proto(const char *interface_name, enum clsapi_net_protocol *proto)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	const char *section_name = NULL;
	string_1024 str_proto = {0};

	if (!interface_name || !proto)
		return -CLSAPI_ERR_INVALID_PARAM;

	section_name = interface_name;
	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name , NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "proto", str_proto);
	if (ret)
		return ret;

	*proto = network_proto_str2enum(str_proto);

	return ret;
}

int clsapi_net_set_proto(const char *interface_name, enum clsapi_net_protocol proto)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	const char *section_name = NULL;
	string_32 str_proto = {0};

	cls_strncpy(str_proto, network_proto_enum2str(proto), sizeof(str_proto));
	if (!interface_name || protocol_validate(str_proto) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	section_name = interface_name;
	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name , NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	if (strcmp(str_proto, "dhcp") == 0) {
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipaddr", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "netmask", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqaddress", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqprefix", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipv6", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "username", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "password", "");

	} else if (strcmp(str_proto, "dhcpv6") == 0) {
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipaddr", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "netmask", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipv6", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "username", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "password", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqaddress", "try");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqprefix", "auto");

	} else if (strcmp(str_proto, "pppoe") == 0) {
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipaddr", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "netmask", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipv6", "auto");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqaddress", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqprefix", "");

	} else if (strcmp(str_proto, "static") == 0) {
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "ipv6", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqaddress", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "reqprefix", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "username", "");
		clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "password", "");

	} else {
		DBG_ERROR("Protocol not matched\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "proto", str_proto);

	return ret;
}

int clsapi_net_get_netmask(const char *netif_name, struct in_addr *netmask)
{
	int sockfd;
	struct ifreq ifr;
	int ret = CLSAPI_OK;
	string_32 str_netmask = {0};
	enum clsapi_net_protocol proto;
	string_32 section_name = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !netmask)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE,
			section_name, 2, CLS_NETWORK_SECT_DEVICE, netif_name);

	if (ret != 0)
		return ret;

	ret = clsapi_net_get_proto(section_name, &proto);
	if (ret != 0)
		return ret;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		DBG_ERROR("Fail to create socket\n");
		return -CLSAPI_ERR_SOCKET;
	}

	if (proto == CLSAPI_NETWORK_PROTO_PPPOE) {
		string_64 fullname = {0};

		snprintf(fullname, sizeof(fullname), "pppoe-%s", section_name);
		cls_strncpy(ifr.ifr_name, fullname, sizeof(ifr.ifr_name));
	} else
		cls_strncpy(ifr.ifr_name, netif_name, sizeof(ifr.ifr_name));

	if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0) {
		DBG_ERROR("Ioctl SIOCGIFNETMASK error\n");
		close(sockfd);
		return -CLSAPI_ERR_IOCTL;
	}
	cls_strncpy(str_netmask, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr), sizeof(str_netmask));
	close(sockfd);

	inet_pton(AF_INET, str_netmask, netmask);

	return ret;
}

int clsapi_net_get_ipv6addr(const char *netif_name, struct in6_addr *ipaddr)
{
	int ret = CLSAPI_OK;
	struct ifaddrs *ifa, *p;
	string_64 interface_name = {0};
	string_32 section_name = {0};
	enum clsapi_net_protocol proto;

	if (netdev_netif_name_validate(netif_name) != 0 || !ipaddr)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE,
			section_name, 2, CLS_NETWORK_SECT_DEVICE, netif_name);
	if (ret != 0)
		return ret;

	ret = clsapi_net_get_proto(section_name, &proto);
	if (ret != 0)
		return ret;

	if (proto == CLSAPI_NETWORK_PROTO_PPPOE)
		snprintf(interface_name, sizeof(interface_name), "pppoe-%s6", section_name);
	else
		strncpy(interface_name, netif_name, sizeof(interface_name));

	//ipv6
	if (getifaddrs(&ifa) != 0)
		return -CLSAPI_ERR_INTERNAL_ERR;

	for (p = ifa; p != NULL; p = p->ifa_next) {
		if (p->ifa_name && !strcmp(interface_name, p->ifa_name)) {
			if (p->ifa_addr && p->ifa_addr->sa_family == AF_INET6) {
				memcpy(ipaddr, &((struct sockaddr_in6 *)(p->ifa_addr))->sin6_addr,
						sizeof(struct in6_addr));
				break;
			}
		}
	}

	freeifaddrs(ifa);

	return ret;
}

int clsapi_net_get_ipaddr(const char *netif_name, struct in_addr *ipaddr)
{
	int sockfd;
	struct ifreq ifr;
	int ret = CLSAPI_OK;
	string_256 address = {0};
	enum clsapi_net_protocol proto;
	string_32 section_name = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !ipaddr)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE,
			section_name, 2, CLS_NETWORK_SECT_DEVICE, netif_name);
	if (ret != 0)
		return ret;

	ret = clsapi_net_get_proto(section_name, &proto);
	if (ret != 0)
		return ret;

	if (proto == CLSAPI_NETWORK_PROTO_PPPOE) {
		string_64 fullname = {0};
		struct ifaddrs *ifaddr, *ifa;

		snprintf(fullname, sizeof(fullname), "pppoe-%s", section_name);
		if (getifaddrs(&ifaddr) == -1)
			return -CLSAPI_ERR_INTERNAL_ERR;

		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_name && strcmp(ifa->ifa_name, fullname) == 0) {
				if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
					if (!inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
								address, sizeof(address)))
						return -CLSAPI_ERR_INVALID_PARAM;
				}
			}
		}

		freeifaddrs(ifaddr);
	} else {
		//ipv4
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0) {
			DBG_ERROR("Fail to create socket\n");
			return -CLSAPI_ERR_SOCKET;
		}

		cls_strncpy(ifr.ifr_name, netif_name, sizeof(ifr.ifr_name));
		if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
			DBG_ERROR("Ioctl SIOCGIFADDR error\n");
			close(sockfd);
			return -CLSAPI_ERR_IOCTL;
		}

		cls_strncpy(address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), sizeof(string_32));
		close(sockfd);
	}

	inet_pton(AF_INET, address, ipaddr);

	return ret;
}

int clsapi_net_set_netmask(const char *netif_name, struct in_addr *netmask)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};
	string_32 section_name = {0};
	string_32 str_netmask = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !netmask)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!inet_ntop(AF_INET, netmask, str_netmask, sizeof(str_netmask)))
		return -CLSAPI_ERR_INVALID_PARAM;

	//check netmask
	ret = netmask_validate(str_netmask);
	if (ret < 0)
		return ret;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_name, 2, CLS_NETWORK_SECT_DEVICE, netif_name);
	if (ret != 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "proto", proto);
	if (ret < 0)
		return ret;

	if (!strcmp(proto, "dhcp") || !strcmp(proto, "pppoe")) {
		DBG_ERROR("IP not support to set because of protocol\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "netmask", str_netmask);

	return ret;
}

int clsapi_net_set_ipaddr(const char *netif_name, struct in_addr *ipaddr)
{
	int ret = CLSAPI_OK;
	string_32 str_ipaddr = {0};
	string_1024 proto = {0};
	string_32 section_name = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !ipaddr)
		return -CLSAPI_ERR_INVALID_PARAM;

	//check ip address
	if (!inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(str_ipaddr)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_name, 2, CLS_NETWORK_SECT_DEVICE, netif_name);
	if (ret != 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "proto", proto);
	if (ret == 0) {
		if (!strcmp(proto, "dhcp") || !strcmp(proto, "pppoe")) {
			DBG_ERROR("IP not support to set because of protocol\n");
			return -CLSAPI_ERR_NOT_SUPPORTED;
		}
	} else {
		DBG_ERROR("UCI read fault\n");
		return -CLSAPI_ERR_UCI;
	}

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "ipaddr", str_ipaddr);

	return ret;
}

int clsapi_net_add_bridge(const char *bridge_name)
{
	int ret = CLSAPI_OK;
	string_1024 section_name = {0};

	if (!bridge_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, "device", section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret == 0)
		return -CLSAPI_ERR_EXISTED;

	ret = clsconf_add_section(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_DEVICE, CLSCONF_SECNAME_ANON, section_name);
	if (ret < 0)
		return ret;

	clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "type", "bridge");
	clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "bridge_empty", "1");
	clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "mtu", "1500");
	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, CLS_NETWORK_PARAM_NAME, bridge_name);

	return ret;
}

int clsapi_net_del_bridge(const char *bridge_name)
{
	int ret = CLSAPI_OK;
	string_1024 section_name = {0};

	if (!bridge_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE,
			section_name, 2, CLS_NETWORK_PARAM_DEVICE, bridge_name);
	if (ret == CLSAPI_OK) {
		DBG_ERROR("Internal error: bridge device is in use, interface: %s\n", section_name);
		return -CLSAPI_ERR_INTERNAL_ERR;
	}

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, "device", section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret < 0)
		return ret;
	clsconf_defer_del_section(CLSCONF_CFG_NETWORK, section_name);

	return ret;
}

int clsapi_net_add_bridge_port(const char *bridge_name, const char *port)
{
	int ret = CLSAPI_OK;
	string_1024 section_name = {0};

	if (!bridge_name || !port || netdev_netif_name_validate(port) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, "device", section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret < 0)
		return ret;

	clsconf_defer_add_apply_list(CLSCONF_CFG_NETWORK, section_name, "ports", port);

	return ret;
}

int clsapi_net_del_bridge_port(const char *bridge_name, const char *port)
{
	int ret = CLSAPI_OK;
	string_1024 section_name = {0};

	if (!bridge_name || !port)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, "device", section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret < 0)
		return ret;

	clsconf_defer_del_apply_list(CLSCONF_CFG_NETWORK, section_name, "ports", port);

	return ret;
}

int clsapi_net_add_interface(const char *interface_name, const char *device_name, enum clsapi_net_protocol proto)
{
	int ret = CLSAPI_OK;
	string_32 tmp_ifname = {0};
	string_32 str_proto = {0};

	cls_strncpy(str_proto, network_proto_enum2str(proto), sizeof(str_proto));

	if (!interface_name || protocol_validate(str_proto) != 0 || netdev_netif_name_validate(device_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		cls_strncpy(tmp_ifname, interface_name, sizeof(string_32));
		ret = clsconf_add_section(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_INTERFACE, CLSCONF_SECNAME_SET, tmp_ifname);
		clsconf_set_param(CLSCONF_CFG_NETWORK, tmp_ifname, CLS_NETWORK_PARAM_DEVICE, device_name);
		clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, tmp_ifname, "proto", str_proto);
	}

	return ret;
}

int clsapi_net_del_interface(const char *interface_name)
{
	int ret = CLSAPI_OK;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret == -CLSAPI_ERR_EXISTED) {
		clsconf_defer_del_section(CLSCONF_CFG_NETWORK, interface_name);
		return CLSAPI_OK;
	}

	return ret;
}


int clsapi_net_get_dhcp_leasetime(const char *netif_name, uint32_t *lease_time)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_1024 leasetime_string = {0};
	string_32 section_ifname = {0};
	char *endptr = NULL;
	uint32_t time =  0;

	if (netdev_netif_name_validate(netif_name) != 0 || !lease_time)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_DHCP, section_ifname, "leasetime", leasetime_string);

	if (strcmp(leasetime_string, "infinite") == 0)
		*lease_time = 0;
	else {
		time = strtod(leasetime_string, &endptr);
		if (time && endptr && endptr[0]) {
			if (endptr[0] == 's')
				time *= 1;
			else if (endptr[0] == 'm')
				time *= 60;
			else if (endptr[0] == 'h')
				time *= 3600;
			else if (endptr[0] == 'd')
				time *= 24 * 3600;
			else if (endptr[0] == 'w')
				time *= 7 * 24 * 3600;
			else
				return -CLSAPI_ERR_INVALID_PARAM;
		}
		if (time < 120)
			time = 120;
		*lease_time = time;
	}

	return ret;
}

int clsapi_net_del_firewall_zone(const char *zone_name)
{
	int ret = CLSAPI_OK;
	string_32 section_zonename = {0};

	if (!zone_name)
		return -CLSAPI_ERR_INVALID_PARAM;
	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);
	if (ret < 0)
		return ret;

	clsconf_defer_del_section(CLSCONF_CFG_FIREWALL, section_zonename);

	return ret;
}

int clsapi_net_add_firewall_zone(const char *zone_name)
{
	int ret = CLSAPI_OK;
	string_32 section_zonename = {0};

	if (!zone_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);

	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = clsconf_add_section(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, CLSCONF_SECNAME_ANON, section_zonename);
		if (ret < 0)
			return ret;
		clsconf_defer_apply_param(CLSCONF_CFG_FIREWALL, section_zonename, CLS_NETWORK_PARAM_NAME, zone_name);
	} else if (ret == CLSAPI_OK)
		return -CLSAPI_ERR_EXISTED;
	else
		return ret;

	return ret;
}

int clsapi_net_set_dhcp_leasetime(const char *netif_name, const uint32_t lease_time)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 leasetime_string = {0};
	string_32 section_ifname = {0};

	if (netdev_netif_name_validate(netif_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (lease_time == 0)
		snprintf(leasetime_string, sizeof(string_32), "infinite");
	else if (lease_time >= 120)
		snprintf(leasetime_string, sizeof(string_32), "%us", lease_time);
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_DHCP, section_ifname, "leasetime", leasetime_string);

	return ret;
}

int clsapi_net_get_firewall_zone_network(const char *zone_name, clsapi_ifname **network_name)
{
	int ret = CLSAPI_OK;
	string_32 section_zonename = {0};
	string_1024 local_network_name[1024] = {0};
	int network_name_len = ARRAY_SIZE(local_network_name);

	if (!zone_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);
	if (ret < 0)
		return ret;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_FIREWALL, section_zonename, CLS_NETWORK_PARAM_NETWORK);
	if (ret == -CLSAPI_ERR_EXISTED) {
		ret = clsconf_get_list(CLSCONF_CFG_FIREWALL, section_zonename, CLS_NETWORK_PARAM_NETWORK, local_network_name, &network_name_len);
		if (ret)
			return ret;
	}
	*network_name = (clsapi_ifname *)calloc(network_name_len, sizeof(clsapi_ifname));
	if (!*network_name)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < network_name_len; i++)
		cls_strncpy((*network_name)[i], local_network_name[i], sizeof(clsapi_ifname));

	return network_name_len;
}

int clsapi_net_del_firewall_zone_network(const char *zone_name, const char *network_name)
{
	int ret = CLSAPI_OK;
	string_32 section_zonename = {0};

	if (!zone_name || !network_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);
	if (ret < 0)
		return ret;

	clsconf_defer_del_apply_list(CLSCONF_CFG_FIREWALL, section_zonename, CLS_NETWORK_PARAM_NETWORK, network_name);

	return ret;
}

int clsapi_net_add_firewall_zone_network(const char *zone_name, const char *network_name)
{
	int ret = CLSAPI_OK;
	string_32 section_zonename = {0};

	if (!zone_name || !network_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);
	if (ret < 0)
		return ret;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, network_name, NULL);
	if (ret == -CLSAPI_ERR_EXISTED)
		clsconf_defer_add_apply_list(CLSCONF_CFG_FIREWALL, section_zonename, CLS_NETWORK_PARAM_NETWORK, network_name);

	return ret;
}

int clsapi_net_set_firewall_zone_policy(const char *zone_name, enum clsapi_firewall_chain chain, enum clsapi_firewall_policy policy)
{
	int ret = CLSAPI_OK;
	string_32 section_zonename = {0};

	if (!zone_name ||
		policy >= _CLSAPI_FIREWALL_POLICY_MAX ||
		policy < CLSAPI_FIREWALL_POLICY_ACCEPT ||
		chain >= _CLSAPI_FIREWALL_CHAIN_MAX ||
		chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);
	if (ret < 0)
		return ret;
	clsconf_defer_apply_param(CLSCONF_CFG_FIREWALL, section_zonename, firewall_chain_enum2str(chain), firewall_policy_enum2str(policy));

	return ret;
}

int clsapi_net_get_firewall_zone_policy(const char *zone_name, enum clsapi_firewall_chain chain, enum clsapi_firewall_policy *policy)
{
	int ret = CLSAPI_OK;
	string_1024 str_policy = {0};
	string_32 section_zonename = {0};

	if (!zone_name || !policy)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zonename, 2, CLS_NETWORK_PARAM_NAME, zone_name);
	if (ret < 0)
		return ret;

	if (chain >= _CLSAPI_FIREWALL_CHAIN_MAX || chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_zonename, firewall_chain_enum2str(chain), str_policy);
	if (ret)
		return ret;

	*policy = firewall_policy_str2enum(str_policy);
	if (*policy >= _CLSAPI_FIREWALL_POLICY_MAX || *policy < CLSAPI_FIREWALL_POLICY_ACCEPT)
		return -CLSAPI_ERR_INVALID_PARAM;

	return ret;
}

int clsapi_net_get_dhcp_server_enabled(const char *netif_name, bool *enable)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	int ignore = -1;
	string_32 section_ifname = {0};
	string_1024 proto = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !enable)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_DHCP, section_ifname, NULL);
	if (ret == -CLSAPI_ERR_EXISTED) {
		ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_ifname, "proto", proto);
		if (strcmp(proto, "static") != 0) {
			*enable = 0;
			return CLSAPI_OK;
		} else {
			clsconf_get_int_param(CLSCONF_CFG_DHCP, section_ifname, "ignore", ignore);
			if (ignore == -1 || ignore == 0) {
				*enable = 1;
				return CLSAPI_OK;
			} else if (ignore == 1)
				*enable = 0;
			else
				return -CLSAPI_ERR_INVALID_DATA;

		}
	} else
		*enable = 0;

	return ret;
}

int clsapi_net_enable_dhcp_server(const char *netif_name, const bool enable)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	int ignore = -1;
	string_32 section_ifname = {0};
	string_1024 proto = {0};

	if (netdev_netif_name_validate(netif_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (enable == 0)
		ignore = 1;
	else if (enable == 1)
		ignore = 0;
	else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_ifname, "proto", proto);
	if (ret == CLSAPI_OK) {
		if (strcmp(proto, "static") == 0) {
			ret = clsconf_check_section_param_existed(CLSCONF_CFG_DHCP, section_ifname, NULL);
			if (ret == -CLSAPI_ERR_NOT_FOUND) {
				ret = clsconf_add_section(CLSCONF_CFG_DHCP, "dhcp", CLSCONF_SECNAME_SET, section_ifname);
				clsconf_set_param(CLSCONF_CFG_DHCP, section_ifname, "leasetime", "12h");
				clsconf_set_int_param(CLSCONF_CFG_DHCP, section_ifname, "start", 100);
				clsconf_set_int_param(CLSCONF_CFG_DHCP, section_ifname, "limit", 150);
				clsconf_defer_apply_int_param(CLSCONF_CFG_DHCP, section_ifname, "ignore", ignore);
			} else if (ret == -CLSAPI_ERR_EXISTED) {
				clsconf_defer_apply_int_param(CLSCONF_CFG_DHCP, section_ifname, "ignore", ignore);
			}
			return ret;
		} else {
			DBG_ERROR("DHCP server not support to set because of protocol\n");
			return -CLSAPI_ERR_NOT_SUPPORTED;
		}
	}

	return ret;
}

int clsapi_net_get_dhcp_addr_pool(const char *netif_name, uint32_t *start_offset, uint32_t *max_lease)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 section_ifname = {0};
	string_1024 proto = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !start_offset || !max_lease)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;
	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_ifname, "proto", proto);
	if (ret < 0)
		return ret;

	if (strcmp(proto, "static") == 0) {
		clsconf_get_int_param(CLSCONF_CFG_DHCP, section_ifname, "start", *start_offset);
		if (ret < 0)
			return ret;
		clsconf_get_int_param(CLSCONF_CFG_DHCP, section_ifname, "limit", *max_lease);
	} else
		return -CLSAPI_ERR_NOT_SUPPORTED;

	return ret;
}

int clsapi_net_set_dhcp_addr_pool(const char *netif_name, const uint32_t start_offset, const uint32_t max_lease)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 section_ifname = {0};
	string_32 str_netmask = {0};
	struct in_addr netmask = {0};
	string_32 ipaddr = {0};
	string_1024 proto = {0};
	uint8_t netmask_phase[4] = {0};
	uint8_t ipaddr_phase[4] = {0};
	uint32_t tmp = 0;
	uint32_t sum = 0;
	bool enabled = 0;
	struct in_addr s_ipaddr;


	if (netdev_netif_name_validate(netif_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (max_lease == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;

	ret = clsapi_net_get_netmask(netif_name, &netmask);
	if (ret < 0)
		return ret;

	inet_ntop(AF_INET, &netmask, str_netmask, sizeof(str_netmask));

	ret = clsapi_net_get_ipaddr(netif_name, &s_ipaddr);
	if (ret < 0)
		return ret;
	inet_ntop(AF_INET, &s_ipaddr, ipaddr, sizeof(ipaddr));

	ret = netaddr_segmentation(str_netmask, netmask_phase);
	if (ret < 0)
		return ret;

	ret = netaddr_segmentation(ipaddr, ipaddr_phase);
	if (ret < 0)
		return ret;

	for (int i = 0; i < 4; i++) {
		if (255 - netmask_phase[i] > ipaddr_phase[i]) {
			tmp = 255 - netmask_phase[i] - ipaddr_phase[i];
		} else
			continue;
		if (i == 0)
			sum = tmp * 256 * 256 * 254;
		else if (i == 1)
			sum += tmp * 256 * 254;
		else if (i == 2)
			sum += tmp * 254;
		else
			sum += tmp - 1;
	}

	if (max_lease > sum)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_get_dhcp_server_enabled(netif_name, &enabled);
	if (ret < 0)
		return ret;

	if (enabled == 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_ifname, "proto", proto);
	if (ret < 0)
		return ret;

	if (strcmp(proto, "static") == 0) {
		clsconf_set_int_param(CLSCONF_CFG_DHCP, section_ifname, "start", start_offset);
		clsconf_defer_apply_int_param(CLSCONF_CFG_DHCP, section_ifname, "limit", max_lease);
	} else
		return -CLSAPI_ERR_NOT_SUPPORTED;

	return ret;
}

int clsapi_net_set_firewall_default_policy(enum clsapi_firewall_chain chain, enum clsapi_firewall_policy policy)
{
	int ret = CLSAPI_OK;

	if (policy >= _CLSAPI_FIREWALL_POLICY_MAX ||
		policy < CLSAPI_FIREWALL_POLICY_ACCEPT ||
		chain >= _CLSAPI_FIREWALL_CHAIN_MAX ||
		chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_defer_apply_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_DEFAULT_POLICY, firewall_chain_enum2str(chain), firewall_policy_enum2str(policy));

	return ret;
}


int clsapi_net_add_static_route(const char *interface_name, struct in_addr *ipaddr, struct in_addr *netmask)
{
	int ret = CLSAPI_OK;
	string_32 section_route = {0};
	string_32 str_netmask = {0};
	string_32 str_ipaddr = {0};
	string_64 ipaddr_cidr = {0};
	uint8_t num_netmask = 0;

	if (!interface_name || !ipaddr || !netmask)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)) ||\
		!inet_ntop(AF_INET, netmask, str_netmask, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = netmask_validate(str_netmask);
	if (ret < 0)
		return ret;

	num_netmask = netmask_to_prefix_length(str_netmask);
	snprintf(ipaddr_cidr, sizeof(string_64), "%s/%d", str_ipaddr, num_netmask);

	//add new route
	ret = clsconf_add_section(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_ROUTE, CLSCONF_SECNAME_ANON, section_route);
	if (ret < 0)
		return ret;

	clsconf_set_param(CLSCONF_CFG_NETWORK, section_route, CLS_NETWORK_PARAM_INTERFACE, interface_name);
	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_route, "target", ipaddr_cidr);

	return ret;
}

int clsapi_net_del_static_route(const char *interface_name, struct in_addr *ipaddr, struct in_addr *netmask)
{
	int ret = CLSAPI_OK;
	string_32 section_route = {0};
	string_32 str_ipaddr = {0};
	string_64 ipaddr_cidr = {0};
	string_32 str_netmask = {0};
	uint8_t num_netmask = 0;

	if (!interface_name || !ipaddr || !netmask)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!inet_ntop(AF_INET, netmask, str_netmask, sizeof(string_32)) ||\
		!inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	num_netmask = netmask_to_prefix_length(str_netmask);
	snprintf(ipaddr_cidr, sizeof(string_64), "%s/%d", str_ipaddr, num_netmask);
	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_ROUTE, section_route, 4, CLS_NETWORK_PARAM_INTERFACE, interface_name, "target", ipaddr_cidr);
	if (ret < 0)
		return ret;
	//delete related info.
	clsconf_defer_del_section(CLSCONF_CFG_NETWORK, section_route);

	return ret;
}

int clsapi_net_get_static_route_gateway(const char *interface_name, struct in_addr *ipaddr,
		struct in_addr *netmask, struct in_addr *gateway)
{
	int ret = CLSAPI_OK;
	string_32 section_route = {0};
	string_32 str_ipaddr = {0};
	string_64 ipaddr_cidr = {0};
	string_32 str_netmask = {0};
	uint8_t num_netmask = 0;
	string_1024 str_gateway = {0};

	if (!interface_name || !ipaddr || !netmask || !gateway)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!inet_ntop(AF_INET, netmask, str_netmask, sizeof(string_32)) ||\
		!inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	num_netmask = netmask_to_prefix_length(str_netmask);
	snprintf(ipaddr_cidr, sizeof(string_64), "%s/%d", str_ipaddr, num_netmask);
	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_ROUTE, section_route, 4, CLS_NETWORK_PARAM_INTERFACE, interface_name, "target", ipaddr_cidr);
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_route, "gateway", str_gateway);
	if (ret < 0)
		return ret;

	inet_pton(AF_INET, str_gateway, gateway);

	return ret;
}

int clsapi_net_get_firewall_default_policy(enum clsapi_firewall_chain chain, enum clsapi_firewall_policy *policy)
{
	int ret = CLSAPI_OK;
	string_1024 str_policy = {0};

	if (!policy)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (chain >= _CLSAPI_FIREWALL_CHAIN_MAX || chain < CLSAPI_FIREWALL_CHAIN_INPUT)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_DEFAULT_POLICY, firewall_chain_enum2str(chain), str_policy);
	if (ret)
		return ret;

	*policy = firewall_policy_str2enum(str_policy);
	if (*policy >= _CLSAPI_FIREWALL_POLICY_MAX || *policy < CLSAPI_FIREWALL_POLICY_ACCEPT)
		return -CLSAPI_ERR_INVALID_PARAM;

	return ret;
}

int clsapi_net_set_static_route_gateway(const char *interface_name, struct in_addr *ipaddr,
		struct in_addr *netmask, struct in_addr *gateway)
{
	int ret = CLSAPI_OK;
	uint8_t num_netmask = 0;
	string_32 section_route = {0};
	string_32 str_ipaddr = {0};
	string_64 ipaddr_cidr = {0};
	string_32 str_netmask = {0};
	struct in_addr ori_netmask;
	string_32 str_gateway = {0};
	string_1024 netif_name = {0};
	struct in_addr s_ipaddr;

	if (!interface_name || !ipaddr || !netmask || !gateway)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!inet_ntop(AF_INET, netmask, str_netmask, sizeof(string_32)) ||\
		!inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)) ||\
		!inet_ntop(AF_INET, gateway, str_gateway, sizeof(string_1024)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	num_netmask = netmask_to_prefix_length(str_netmask);
	snprintf(ipaddr_cidr, sizeof(string_64), "%s/%d", str_ipaddr, num_netmask);
	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_ROUTE, section_route, 4, CLS_NETWORK_PARAM_INTERFACE, interface_name, "target", ipaddr_cidr);
	if (ret < 0)
		return ret;
	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret < 0)
		return ret;

	ret = clsapi_net_get_ipaddr(netif_name, &s_ipaddr);
	if (ret < 0)
		return ret;
	inet_ntop(AF_INET, &s_ipaddr, str_ipaddr, sizeof(str_ipaddr));

	ret = clsapi_net_get_netmask(netif_name, &ori_netmask);
	if (ret < 0)
		return ret;

	inet_ntop(AF_INET, &ori_netmask, str_netmask, sizeof(str_netmask));

	if ((inet_addr(str_ipaddr) & inet_addr(str_netmask)) != (inet_addr(str_gateway) & inet_addr(str_netmask)))
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_route, "gateway", str_gateway);

	return ret;
}

int clsapi_net_del_static_route_gateway(const char *interface_name, struct in_addr *ipaddr, struct in_addr *netmask)
{
	int ret = CLSAPI_OK;
	string_32 section_route = {0};
	string_32 str_ipaddr = {0};
	string_64 ipaddr_cidr = {0};
	string_32 str_netmask = {0};
	uint8_t num_netmask = 0;

	if (!interface_name || !ipaddr || !netmask)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (!inet_ntop(AF_INET, netmask, str_netmask, sizeof(string_32)) ||\
		!inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	num_netmask = netmask_to_prefix_length(str_netmask);
	snprintf(ipaddr_cidr, sizeof(string_64), "%s/%d", str_ipaddr, num_netmask);
	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_ROUTE, section_route, 4, CLS_NETWORK_PARAM_INTERFACE, interface_name, "target", ipaddr_cidr);
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_route, "gateway", "");

	return ret;
}

int clsapi_net_get_dns_domain_binding(const char *domain_name, struct in_addr *ipaddr)
{
	int ret = CLSAPI_OK;
	string_32 section_domain = {0};
	string_1024 str_ipaddr = {0};

	if (!ipaddr || !domain_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_DHCP, CLS_NETWORK_SECT_TYPE_DOMAIN, section_domain, 2, CLS_NETWORK_PARAM_NAME, domain_name);
	if (ret < 0)
		return ret;

	clsconf_get_param(CLSCONF_CFG_DHCP, section_domain, "ip", str_ipaddr);
	if (ret < 0)
		return ret;

	if(!inet_pton(AF_INET, str_ipaddr, ipaddr))
		return -CLSAPI_ERR_INVALID_DATA;

	return ret;
}

int clsapi_net_set_dns_domain_binding(const char *domain_name, struct in_addr *ipaddr)
{
	int ret = CLSAPI_OK;
	string_32 section_domain = {0};
	string_1024 str_ipaddr = {0};

	if (!domain_name || !inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_DHCP, CLS_NETWORK_SECT_TYPE_DOMAIN, section_domain, 2, CLS_NETWORK_PARAM_NAME, domain_name);
	if (ret < 0)
		return ret;

	clsconf_defer_apply_param(CLSCONF_CFG_DHCP, section_domain, "ip", str_ipaddr);

	return CLSAPI_OK;
}

int clsapi_net_add_dns_domain_binding(const char *domain_name, struct in_addr *ipaddr)
{
	int ret = CLSAPI_OK;
	string_32 str_ipaddr = {0};
	string_32 section_domain = {0};

	if (!domain_name || !inet_ntop(AF_INET, ipaddr, str_ipaddr, sizeof(string_32)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_DHCP, CLS_NETWORK_SECT_TYPE_DOMAIN, section_domain, 2, CLS_NETWORK_PARAM_NAME, domain_name);

	if (!ret)
		return -CLSAPI_ERR_EXISTED;
	else if (ret == -CLSAPI_ERR_NOT_FOUND) {
		clsconf_defer_add_section(CLSCONF_CFG_DHCP, CLS_NETWORK_SECT_TYPE_DOMAIN, CLSCONF_SECNAME_ANON, section_domain);
		clsconf_set_param(CLSCONF_CFG_DHCP, section_domain, CLS_NETWORK_PARAM_NAME, domain_name);
		clsconf_defer_apply_param(CLSCONF_CFG_DHCP, section_domain, "ip", str_ipaddr);
	} else
		return ret;

	return CLSAPI_OK;
}

int clsapi_net_del_dns_domain_binding(const char *domain_name)
{
	int ret = CLSAPI_OK;
	string_1024 section_domain= {0};

	if (!domain_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_DHCP, CLS_NETWORK_SECT_TYPE_DOMAIN, section_domain, 2, CLS_NETWORK_PARAM_NAME, domain_name);
	if (ret < 0)
		return ret;

	clsconf_defer_del_section(CLSCONF_CFG_DHCP, section_domain);

	return ret;
}

int clsapi_net_get_dns_domain_suffix(string_64 dns_suffix)
{
	int ret = CLSAPI_OK;
	string_1024 str_dns_suffix = {0};

	if (!dns_suffix)
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_get_param(CLSCONF_CFG_DHCP, "@dnsmasq[0]", "domain", str_dns_suffix);
	if (ret < 0)
		return ret;

	cls_strncpy(dns_suffix, str_dns_suffix, sizeof(string_64));

	return ret;
}

int clsapi_net_set_dns_domain_suffix(const char *dns_suffix)
{
	int ret = CLSAPI_OK;

	if (!dns_suffix)
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_defer_apply_param(CLSCONF_CFG_DHCP, "@dnsmasq[0]", "domain", dns_suffix);

	return ret;
}

int clsapi_net_get_dns_server_addr(const char *netif_name, struct in_addr **server_addr)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 section_ifname = {0};
	string_32 array_server_addr[256] = {0};
	string_1024 dhcp_options[128] =  {0};
	string_1024 str_server_addr =  {0};
	int dhcp_options_num = ARRAY_SIZE(dhcp_options);
	int server_addr_len = ARRAY_SIZE(array_server_addr);

	if (netdev_netif_name_validate(netif_name) != 0 || !server_addr)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret)
		return ret;

	ret = clsconf_get_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, dhcp_options, &dhcp_options_num);
	if (ret)
		return ret;

	for (int i = 0; i < dhcp_options_num; i++) {
		if (dhcp_options[i][0] == '6')
			cls_strncpy(str_server_addr, dhcp_options[i] + 2, sizeof(str_server_addr));
	}

	if (str_server_addr[0] == '\0')
		return -CLSAPI_ERR_NOT_FOUND;

	ret = split_dns_address_in_dhcp_op(str_server_addr, array_server_addr, &server_addr_len);
	if (ret)
		return ret;

	*server_addr = (struct in_addr *)calloc(server_addr_len, sizeof(struct in_addr));
	if (!(*server_addr))
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < server_addr_len; i++) {
		if (!inet_pton(AF_INET, array_server_addr[i], &(*server_addr)[i])) {
			free(*server_addr);
			return -CLSAPI_ERR_INVALID_DATA;
		}
	}

	return server_addr_len;
}

int clsapi_net_add_dns_server_addr(const char *netif_name, const struct in_addr *server_addr)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 section_ifname = {0};
	string_1024 proto = {0};
	string_32 str_server_addr = {0};
	string_512 str_all_server_addr =  {0};
	string_32 all_server_addr[64] = {0};
	int server_addr_len = ARRAY_SIZE(all_server_addr);
	string_1024 dhcp_options[64] =  {0};
	int dhcp_options_num = ARRAY_SIZE(dhcp_options);
	struct in_addr tmp_server_addr = {0};
	string_1024 tmp_option_string = {0};

	if (netdev_netif_name_validate(netif_name) != 0 || !server_addr)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_ifname, "proto", proto);
	if (ret)
		return ret;

	if (strcmp(proto, "static") != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	if (!inet_ntop(AF_INET, server_addr, str_server_addr, sizeof(str_server_addr)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, dhcp_options, &dhcp_options_num);
	if (ret != -CLSAPI_ERR_NOT_FOUND && ret)
		return ret;

	for (int i = 0; i < dhcp_options_num; i++) {
		if (dhcp_options[i][0] == '6')
			cls_strncpy(str_all_server_addr, dhcp_options[i], sizeof(str_all_server_addr));
	}
	if (str_all_server_addr[0] == '\0') {
		snprintf(str_all_server_addr, sizeof(str_all_server_addr), "6,%s", str_server_addr);
		clsconf_add_apply_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, str_all_server_addr);
		return CLSAPI_OK;
	}

	ret = split_dns_address_in_dhcp_op(str_all_server_addr + 2, all_server_addr, &server_addr_len);
	if (ret)
		return ret;
	for (int i = 0; i < server_addr_len; i++) {
		inet_pton(AF_INET, all_server_addr[i], &tmp_server_addr);
		if (tmp_server_addr.s_addr == server_addr->s_addr)
			return -CLSAPI_ERR_EXISTED;
	}
	clsconf_defer_del_apply_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, str_all_server_addr);
	snprintf(tmp_option_string, sizeof(tmp_option_string), "6,%s,%s", str_all_server_addr + 2, str_server_addr);
	clsconf_add_apply_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, tmp_option_string);

	return ret;
}

int clsapi_net_add_firewall_zone_forwarding(const char *src_zone, const char *dest_zone)
{
	int ret = CLSAPI_OK;
	string_32 section_forwarding = {0};
	string_32 section_zone = {0};

	if (!src_zone || !dest_zone || strcmp(src_zone, dest_zone) == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zone, 2, CLS_NETWORK_PARAM_NAME, src_zone) ||
		clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zone, 2, CLS_NETWORK_PARAM_NAME, dest_zone))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_FORWARD, section_forwarding, 4, "src", src_zone, "dest", dest_zone);
	if (ret == CLSAPI_OK)
		return -CLSAPI_ERR_EXISTED;

	else if (ret == -CLSAPI_ERR_NOT_FOUND) {
		ret = clsconf_add_section(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_FORWARD, CLSCONF_SECNAME_ANON, section_forwarding);
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_forwarding, "src", src_zone);
		clsconf_defer_apply_param(CLSCONF_CFG_FIREWALL, section_forwarding, "dest", dest_zone);
	}

	return ret;
}

int clsapi_net_del_firewall_zone_forwarding(const char *src_zone, const char *dest_zone)
{
	int ret = CLSAPI_OK;
	string_32 section_forwarding = {0};
	string_32 section_zone = {0};

	if (!src_zone || !dest_zone || strcmp(src_zone, dest_zone) == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zone, 2, CLS_NETWORK_PARAM_NAME, src_zone) ||
		clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, section_zone, 2, CLS_NETWORK_PARAM_NAME, dest_zone))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_FORWARD, section_forwarding, 4, "src", src_zone, "dest", dest_zone);
	if (ret)
		return ret;

	clsconf_defer_del_section(CLSCONF_CFG_FIREWALL, section_forwarding);

	return ret;
}

int clsapi_net_del_dns_server_addr(const char *netif_name, const struct in_addr *server_addr)
{
	int ret = -CLSAPI_ERR_INVALID_PARAM;
	string_32 section_ifname = {0};
	string_32 str_server_addr = {0};
	struct in_addr *all_server_addr = {0};
	int server_addr_len = 0;
	string_1024 str_all_server_addr =  {0};
	bool only_one_same_addr = false;
	int all_addr_len = 0;

	if (netdev_netif_name_validate(netif_name) != 0 || !server_addr)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_TYPE_INTERFACE, section_ifname, 2, CLS_NETWORK_PARAM_DEVICE, netif_name);
	if (ret)
		return ret;

	if (!inet_ntop(AF_INET, server_addr, str_server_addr, sizeof(str_server_addr)))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsapi_net_get_dns_server_addr(netif_name, &all_server_addr);
	if (ret < 0)
		return ret;
	else if( ret == 0)
		return -CLSAPI_ERR_NOT_FOUND;

	server_addr_len = ret;

	cls_strncpy(str_all_server_addr, "6", sizeof(str_all_server_addr));
	all_addr_len = strlen(str_all_server_addr);
	for (int i = 0; i < server_addr_len; i++) {
		if (all_server_addr[i].s_addr == server_addr->s_addr && server_addr_len == 1)
			only_one_same_addr = true;
		inet_ntop(AF_INET, &all_server_addr[i], str_server_addr, sizeof(str_server_addr));
		snprintf(str_all_server_addr + all_addr_len, sizeof(str_all_server_addr) - all_addr_len, ",%s", str_server_addr);
		// "1" for ","
		all_addr_len += (strlen(str_server_addr) + 1);
	}
	clsconf_defer_del_apply_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, str_all_server_addr);
	if (only_one_same_addr) {
		if (all_server_addr)
			free(all_server_addr);
		return ret;
	}

	cls_strncpy(str_all_server_addr, "6", sizeof(str_all_server_addr));
	all_addr_len = strlen(str_all_server_addr);
	for (int i = 0; i < server_addr_len; i++) {
		if (all_server_addr[i].s_addr == server_addr->s_addr)
			continue;
		inet_ntop(AF_INET, &all_server_addr[i], str_server_addr, sizeof(str_server_addr));
		snprintf(str_all_server_addr + all_addr_len, sizeof(str_all_server_addr) - all_addr_len, ",%s", str_server_addr);
		all_addr_len += (strlen(str_server_addr) + 1);
	}
	clsconf_defer_add_apply_list(CLSCONF_CFG_DHCP, section_ifname, CLS_NETWORK_PARAM_DHCP_OP, str_all_server_addr);
	if (all_server_addr)
		free(all_server_addr);

	return ret;
}

int clsapi_net_get_dhcp_lease_info(struct dhcp_lease_info info[], int *info_len)
{
	FILE *file;
	char line[MAX_LINE_LEN] = {0};
	int ret = CLSAPI_OK;
	string_32 str_ipaddr = {0};
	string_32 str_macaddr = {0};
	int i = 0;

	if (!info || !info_len)
		return -CLSAPI_ERR_INVALID_PARAM;

	file = fopen("/tmp/dhcp.leases", "r");
	if (file == NULL) {
		DBG_ERROR("fail to open file");
		return -CLSAPI_ERR_FILE_OPERATION;
	}
	while (fgets(line, sizeof(line), file)) {
		if (i >= *info_len) {
			fclose(file);
			return -CLSAPI_ERR_INVALID_PARAM;
		}

		if (sscanf(line, "%u %17s %31s %128s", &info[i].expires, str_macaddr, str_ipaddr, info[i].hostname) != 4) {
			DBG_ERROR("parsing line: %s", line);
			fclose(file);
			return -CLSAPI_ERR_FILE_OPERATION;
		}
		mac_aton(str_macaddr, info[i].macaddr);
		if (!inet_pton(AF_INET, str_ipaddr, &info[i].ipaddr.s_addr)) {
			fclose(file);
			return -CLSAPI_ERR_INVALID_DATA;
		}
		i++;
	}

	*info_len = i;
	fclose(file);

	return ret;
}

int clsapi_net_get_firewall_rule(struct clsapi_net_firewall_rule *rule)
{
	int ret = CLSAPI_OK;
	char *token = " ";
	char *tmp = NULL;
	int i = 0;
	string_32 section_rule = {0};
	string_1024 dummy = {0};

	if (!rule || !rule->name || rule->flag == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_RULE, section_rule, 2, CLS_NETWORK_PARAM_NAME, rule->name);
	if (ret)
		return ret;
	// get src
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "src", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_src(rule, dummy);

	//get dest
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "dest", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_dest(rule, dummy);

	// get mark
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "mark", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_mark(rule, dummy);

	//get start time
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "start_time", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_start_time(rule, dummy);
	//get start date
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "start_date", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_start_date(rule, dummy);
	//get stop time
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "stop_time", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_stop_time(rule, dummy);
	//get stop date
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "stop_date", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_stop_date(rule, dummy);
	//get weekdays
	string_32 weekdays[7] = {0};

	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "weekdays", dummy);
	if (ret == CLSAPI_OK) {
		tmp = strtok(dummy, token);
		i = 0;
		while (tmp != NULL) {
			if (i >= ARRAY_SIZE(weekdays))
				return -CLSAPI_ERR_INVALID_PARAM;
			cls_strncpy(weekdays[i], tmp, sizeof(weekdays[i]));
			tmp = strtok(NULL, token);
			i++;
		}
		set_fw_rule_weekdays(rule, weekdays);
	}
	//get monthdays
	string_32 monthdays[31] = {0};
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "monthdays", dummy);
	if (ret == CLSAPI_OK) {
		tmp = strtok(dummy, token);
		i = 0;
		while (tmp != NULL) {
			if (i >= ARRAY_SIZE(monthdays))
				return -CLSAPI_ERR_INVALID_PARAM;
			cls_strncpy(monthdays[i], tmp, sizeof(monthdays[i]));
			tmp = strtok(NULL, token);
			i++;
		}
		set_fw_rule_monthdays(rule, monthdays);
	}
	//get utc_time
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "utc_time", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_utc_time(rule, true);
	else
		set_fw_rule_utc_time(rule, false);
	//get target
	enum clsapi_firewall_rule_target target = _CLSAPI_FIREWALL_TARGET_MAX;

	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "target", dummy);

	if (ret == CLSAPI_OK) {
		if (strcmp(dummy, "ACCEPT") == 0)
			target = CLSAPI_FIREWALL_TARGET_ACCEPT;
		else if (strcmp(dummy, "REJECT") == 0)
			target = CLSAPI_FIREWALL_TARGET_REJECT;
		else if (strcmp(dummy, "DROP") == 0)
			target = CLSAPI_FIREWALL_TARGET_DROP;
		else if (strcmp(dummy, "NOTRACK") == 0)
			target = CLSAPI_FIREWALL_TARGET_NOTRACK;
		else if (strcmp(dummy, "MARK") == 0) {
			ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "set_xmark", dummy);
			if (ret == CLSAPI_OK) {
				target = CLSAPI_FIREWALL_TARGET_XMARK;
				rule->target_option.set_xmark = atoi(dummy);
			} else {
				ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "set_mark", dummy);
				if (ret == CLSAPI_OK) {
					target = CLSAPI_FIREWALL_TARGET_MARK;
					rule->target_option.set_mark = atoi(dummy);
				} else
					return -CLSAPI_ERR_INVALID_PARAM;
			}
		} else if (strcmp(dummy, "HELPER") == 0) {
			target = CLSAPI_FIREWALL_TARGET_HELPER;
			ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "set_helper", dummy);
			if (ret == CLSAPI_OK)
				cls_strncpy(rule->target_option.set_helper, dummy, sizeof(rule->target_option.set_helper));
		} else if (strcmp(dummy, "DSCP") == 0) {
			target = CLSAPI_FIREWALL_TARGET_DSCP;
			ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "set_dscp", dummy);
			if (ret == CLSAPI_OK)
				cls_strncpy(rule->target_option.set_dscp, dummy, sizeof(rule->target_option.set_dscp));
		} else
			target = _CLSAPI_FIREWALL_TARGET_MAX;

		set_fw_rule_target(rule, target);
	}
	//get family
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "family", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_family(rule, dummy);
	//get limit
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "limit", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_limit(rule, dummy);
	//get limit burst
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "limit_burst", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_limit_burst(rule, atoi(dummy));
	//get enabled
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "enabled", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_enabled(rule, false);
	else
		set_fw_rule_enabled(rule, true);
	//get device + get direction
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "device", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_device(rule, dummy);
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "direction", dummy);
	if (ret == CLSAPI_OK)
		cls_strncpy(rule->direction, dummy, sizeof(rule->direction));
	//get dscp
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "dscp", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_dscp(rule, dummy);
	//get helper
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "helper", dummy);
	if (ret == CLSAPI_OK)
		set_fw_rule_helper(rule, dummy);

	//get dest ipaddr
	string_1024 local_dest_ipaddr[1024] = {0};
	int dest_ipaddr_len = ARRAY_SIZE(local_dest_ipaddr);

	ret = clsconf_get_list(CLSCONF_CFG_FIREWALL, section_rule, "dest_ip", local_dest_ipaddr, &dest_ipaddr_len);
	if (ret == CLSAPI_OK) {
		rule->dest_ipaddr_num = dest_ipaddr_len;
		rule->dest_ipaddr = (struct in_addr *)calloc(sizeof(struct in_addr), dest_ipaddr_len);
		for (int i = 0; i < dest_ipaddr_len; i++)
			rule->dest_ipaddr[i].s_addr = inet_addr(local_dest_ipaddr[i]);

		rule->flag |= (1 << 8);
	}

	//get dest port
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "dest_port", dummy);
	if (ret == CLSAPI_OK) {
		rule->dest_port_num = get_param_num(dummy);
		rule->dest_port = (uint32_t *)calloc(sizeof(uint32_t), rule->dest_port_num);
		tmp = strtok(dummy, token);
		i = 0;
		while (tmp != NULL) {
			rule->dest_port[i] = atoi(tmp);
			tmp = strtok(NULL, token);
			i++;
		}
		rule->flag |= (1 << 9);\
	}
	//get src ipaddr
	string_1024 local_src_ipaddr[1024] = {0};
	int src_ipaddr_len = ARRAY_SIZE(local_src_ipaddr);

	ret = clsconf_get_list(CLSCONF_CFG_FIREWALL, section_rule, "src_ip", local_src_ipaddr, &src_ipaddr_len);
	if (ret == CLSAPI_OK) {
		rule->src_ipaddr_num = src_ipaddr_len;
		rule->src_ipaddr = (struct in_addr *)calloc(sizeof(struct in_addr), src_ipaddr_len);
		for (int i = 0; i < src_ipaddr_len; i++)
			rule->src_ipaddr[i].s_addr = inet_addr(local_src_ipaddr[i]);

		rule->flag |= (1 << 2);
	}

	//get src macaddr
	string_1024 local_src_macaddr[128] = {0};
	int macaddr_len = ARRAY_SIZE(local_src_ipaddr);

	ret = clsconf_get_list(CLSCONF_CFG_FIREWALL, section_rule, "src_mac", local_src_macaddr, &macaddr_len);
	if (ret == CLSAPI_OK) {
		rule->src_macaddr_num = macaddr_len;
		rule->src_macaddr = (uint8_t (*)[ETH_ALEN])calloc(sizeof(uint8_t [ETH_ALEN]), macaddr_len);
		for (int i = 0; i < macaddr_len; i++)
			ret = mac_aton(local_src_macaddr[i], rule->src_macaddr[i]);
		rule->flag |= (1 << 3);
	}

	//get src port
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "src_port", dummy);
	if (ret == CLSAPI_OK) {
		rule->src_port_num = get_param_num(dummy);
		rule->src_port = (uint32_t *)calloc(sizeof(uint32_t), rule->src_port_num);
		tmp = strtok(dummy, token);
		i = 0;
		while (tmp != NULL) {
			rule->src_port[i] = atoi(tmp);
			tmp = strtok(NULL, token);
			i++;
		}
		rule->flag |= (1 << 4);
	}
	//get protocol
	ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "proto", dummy);
	if (ret == CLSAPI_OK) {
		rule->proto_num = 1;
		rule->proto = (string_32 *)calloc(sizeof(string_32), 1);
		cls_strncpy(rule->proto[0], dummy, sizeof(string_32));
		rule->flag |= (1 << 5);
	} else {
		string_1024 str_proto[128] = {0};
		int proto_num = ARRAY_SIZE(str_proto);

		ret = clsconf_get_list(CLSCONF_CFG_FIREWALL, section_rule, "proto", str_proto, &proto_num);
		if (ret == CLSAPI_OK) {
			rule->proto_num = proto_num;
			rule->proto = (string_32 *)calloc(sizeof(string_32), rule->proto_num);
			for (int i = 0; i < rule->proto_num; i++)
				cls_strncpy(rule->proto[i], str_proto[i], sizeof(rule->proto[i]));
			rule->flag |= (1 << 5);
		}
	}
	//icmp type
	string_1024 icmp_type[128] = {0};
	int icmp_type_len = ARRAY_SIZE(icmp_type);

	ret = clsconf_get_list(CLSCONF_CFG_FIREWALL, section_rule, "icmp_type", icmp_type, &icmp_type_len);
	if (ret == CLSAPI_OK) {
		for (int i = 0; i < icmp_type_len; i++)
			cls_strncpy(rule->icmp_type[i], icmp_type[i], sizeof(rule->icmp_type[i]));
		rule->flag |= (1 << 6);
	}
	return CLSAPI_OK;
}

int clsapi_net_set_firewall_rule(struct clsapi_net_firewall_rule *rule)
{
	int ret = CLSAPI_OK;
	string_32 section_rule = {0};
	string_32 section_zone = {0};
	string_32 str_ipaddr = {0};
	string_32 str_macaddr = {0};
	string_1024 str_dest_zone = {0};

	if (!rule || !rule->name || rule->flag == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_RULE, section_rule, 2, CLS_NETWORK_PARAM_NAME, rule->name);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		// add new rule section
		ret = clsconf_add_section(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_RULE, CLSCONF_SECNAME_ANON, section_rule);
		if (ret)
			return ret;
		ret = clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "name", rule->name);
		if (ret)
			return ret;
	} else if (ret)
		return ret;

	if (rule->flag <= 0)
		return CLSAPI_OK;

	// src
	if (rule->flag & 0x2) {
		if (!rule->src)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (strcmp(rule->src, "*") == 0)
			clsconf_defer_apply_param(CLSCONF_CFG_FIREWALL, section_rule, "src", rule->src);
		else {
			//check rule->src existed or not.
			ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, "zone", section_zone, 2, CLS_NETWORK_PARAM_NAME, rule->src);
			if (ret)
				return ret;

			ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "dest", str_dest_zone);
			if (ret == -CLSAPI_ERR_NOT_FOUND && strcmp(rule->src, "") == 0)
				return -CLSAPI_ERR_INVALID_PARAM;
			else
				clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "src", rule->src);
		}
	}
	// src_ipaddr
	if (rule->flag & 0x4) {
		if (!rule->src_ipaddr)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < rule->src_ipaddr_num; i++) {
			if (!inet_ntop(AF_INET, &rule->src_ipaddr[i], str_ipaddr, sizeof(str_ipaddr)))
				return -CLSAPI_ERR_INVALID_PARAM;

			clsconf_add_list(CLSCONF_CFG_FIREWALL, section_rule, "src_ip", str_ipaddr);
		}
	}
	// src_macaddr
	if (rule->flag & 0x8) {
		if (!rule->src_macaddr)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < rule->src_macaddr_num; i++) {
			snprintf(str_macaddr, sizeof(str_macaddr), MACFMT, MACARG(rule->src_macaddr[i]));
			clsconf_add_list(CLSCONF_CFG_FIREWALL, section_rule, "src_mac", str_macaddr);
		}
	}
	//src port
	if (rule->flag & 0x10) {
		string_32 str_port = {0};
		int offset = 0;

		if (!rule->src_port)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < rule->src_port_num; i++) {
			offset += snprintf(str_port + offset, sizeof(str_port) - offset, "%d", rule->src_port[i]);
			if (i != rule->src_port_num - 1)
				offset += snprintf(str_port + offset, sizeof(str_port) - offset, " ");
		}
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "src_port", str_port);
	}
	// protocol
	if (rule->flag & 0x20) {
		if (!rule->proto)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < rule->proto_num; i++) {
			ret = validate_firewall_rule_protocol(rule->proto[i]);
			if (ret)
				return ret;
			clsconf_add_list(CLSCONF_CFG_FIREWALL, section_rule, "proto", rule->proto[i]);
		}
	}
	//icmp type
	if (rule->flag & 0x40) {
		if (!rule->icmp_type)
			return -CLSAPI_ERR_INVALID_PARAM;
		ret = validate_icmp_type(rule);
		if (ret)
			return ret;

		for (int i = 0; i < ARRAY_SIZE(rule->icmp_type); i++)
			if (strcmp(rule->icmp_type[i], "\0") != 0)
				clsconf_add_list(CLSCONF_CFG_FIREWALL, section_rule, "icmp_type", rule->icmp_type[i]);
	}
	//dest
	if (rule->flag & 0x80) {
		if (!rule->dest)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (strcmp(rule->dest, "*") == 0)
			clsconf_defer_apply_param(CLSCONF_CFG_FIREWALL, section_rule, "dest", rule->dest);
		else {
			//check rule->dest existed or not.
			ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, "zone", section_zone, 2, CLS_NETWORK_PARAM_NAME, rule->dest);
			if (ret)
				return ret;

			ret = clsconf_get_param(CLSCONF_CFG_FIREWALL, section_rule, "dest", str_dest_zone);
			if (ret == -CLSAPI_ERR_NOT_FOUND && strcmp(rule->dest, "") == 0)
				return -CLSAPI_ERR_INVALID_PARAM;
			else
				clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "dest", rule->dest);
		}
	}
	//dest ipaddr
	if (rule->flag & 0x100) {
		if (!rule->dest_ipaddr)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < rule->dest_ipaddr_num; i++) {
			if (!inet_ntop(AF_INET, &rule->dest_ipaddr[i], str_ipaddr, sizeof(str_ipaddr)))
				return -CLSAPI_ERR_INVALID_PARAM;

			clsconf_add_list(CLSCONF_CFG_FIREWALL, section_rule, "dest_ip", str_ipaddr);
		}
	}
	//dest_port
	if (rule->flag & 0x200) {
		string_32 str_port = {0};
		int offset = 0;

		if (!rule->dest_port)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < rule->dest_port_num; i++) {
			offset += snprintf(str_port + offset, sizeof(str_port) - offset, "%d", rule->dest_port[i]);
			if (i != rule->dest_port_num - 1)
				offset += snprintf(str_port + offset, sizeof(str_port) - offset, " ");
		}
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "dest_port", str_port);
	}
	//mark
	if (rule->flag & 0x400) {
		char *str_mark = rule->mark;

		if (!str_mark)
			return -CLSAPI_ERR_INVALID_PARAM;

		int len = strlen(str_mark);

		if (*str_mark != '!') {
			if ((len >= 2) && (str_mark[0] == '0') && ((str_mark[1] == 'x') || (str_mark[1] == 'X') ) &&  len <= 10) {
				str_mark += 2;
				len -= 2;
			} else
				return -CLSAPI_ERR_INVALID_PARAM;

		} else {
			if ((len >= 3) && (str_mark[1] == '0') && ((str_mark[2] == 'x') || (str_mark[2] == 'X')) && len <= 11) {
				str_mark += 3;
				len -= 3;
			} else
				return -CLSAPI_ERR_INVALID_PARAM;
		}

		for (int i = 0; i < len; i++) {
			if (!((str_mark[i] >= '0' && str_mark[i] <= '9') ||
						(str_mark[i] >= 'a' && str_mark[i] <= 'f') ||
						(str_mark[i] >= 'A' && str_mark[i] <= 'F'))) {
				return -CLSAPI_ERR_INVALID_PARAM;
			}
		}

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "mark", rule->mark);
	}
	//start date
	if (rule->flag & 0x800) {
		char *date = rule->start_date;

		if (!date || strlen(date) != 10)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (date[4] != '-' || date[7] != '-')
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < strlen(date); i++) {
			if (i == 4 || i == 7)
				continue;
			if (!isdigit(*date))
				return -CLSAPI_ERR_INVALID_PARAM;
		}

		int year = atoi(date);
		int month = atoi(date + 5);
		int day = atoi(date + 8);

		if (year < 2015 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31)
			return -CLSAPI_ERR_INVALID_PARAM;

		if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
			return -CLSAPI_ERR_INVALID_PARAM;
		else if (month == 2) {
			if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
				if (day > 29)
					return -CLSAPI_ERR_INVALID_PARAM;
			} else {
				if (day > 28)
					return -CLSAPI_ERR_INVALID_PARAM;
			}
		}

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "start_date", rule->start_date);
	}
	//start time
	if (rule->flag & 0x1000) {
		char *time = rule->start_time;

		if (!time || strlen(time) != 8) {
			return -CLSAPI_ERR_INVALID_PARAM;
		}

		if (time[2] != ':' || time[5] != ':') {
			return -CLSAPI_ERR_INVALID_PARAM;
		}
		for (int i = 0; i < strlen(time); i++) {
			if (i == 2 || i == 5)
				continue;
			if (!isdigit(*time))
				return -CLSAPI_ERR_INVALID_PARAM;
		}

		int hour = atoi(time);
		int minute = atoi(time + 3);
		int second = atoi(time + 6);

		if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59)
			return -CLSAPI_ERR_INVALID_PARAM;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "start_time", rule->start_time);
	}
	//stop date
	if (rule->flag & 0x2000) {
		char *date = rule->stop_date;

		if (!date || strlen(date) != 10)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (date[4] != '-' || date[7] != '-')
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < strlen(date); i++) {
			if (i == 2 || i == 5)
				continue;
			if (!isdigit(*date))
				return -CLSAPI_ERR_INVALID_PARAM;
		}

		int year = atoi(date);
		int month = atoi(date + 5);
		int day = atoi(date + 8);

		if (year < 2015 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31)
			return -CLSAPI_ERR_INVALID_PARAM;

		if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
			return -CLSAPI_ERR_INVALID_PARAM;
		else if (month == 2) {
			if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
				if (day > 29)
					return -CLSAPI_ERR_INVALID_PARAM;
			} else {
				if (day > 28)
					return -CLSAPI_ERR_INVALID_PARAM;
			}
		}
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "stop_date", rule->stop_date);
	}
	//stop time
	if (rule->flag & 0x4000) {
		char *time = rule->stop_time;

		if (!time || strlen(time) != 8) {
			return -CLSAPI_ERR_INVALID_PARAM;
		}

		if (time[2] != ':' || time[5] != ':') {
			return -CLSAPI_ERR_INVALID_PARAM;
		}
		for (int i = 0; i < strlen(time); i++) {
			if (i == 2 || i == 5)
				continue;
			if (!isdigit(*time))
				return -CLSAPI_ERR_INVALID_PARAM;
		}

		int hour = atoi(time);
		int minute = atoi(time + 3);
		int second = atoi(time + 6);

		if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59)
			return -CLSAPI_ERR_INVALID_PARAM;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "stop_time", rule->stop_time);
	}
	//weekdays
	if (rule->flag & 0x8000) {
		string_128 str_weekdays = {0};
		int offset = 0;

		if (!rule->weekdays)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (!validate_weekdays(rule->weekdays, ARRAY_SIZE(rule->weekdays)))
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < ARRAY_SIZE(rule->weekdays); i++) {
			if (strcmp(rule->weekdays[i], "\0") != 0) {
				offset += snprintf(str_weekdays + offset, sizeof(str_weekdays) - offset, "%s ", rule->weekdays[i]);
			}
		}
		str_weekdays[strlen(str_weekdays) - 1] = '\0';
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "weekdays", str_weekdays);
	}
	//monthdays
	if (rule->flag & 0x10000) {
		string_128 str_monthdays = {0};
		int offset = 0;

		if (!rule->monthdays)
			return -CLSAPI_ERR_INVALID_PARAM;

		for (int i = 0; i < ARRAY_SIZE(rule->monthdays); i++) {
			if (strcmp(rule->monthdays[i], "\0") == 0 || strcmp(rule->monthdays[i], "!") == 0)
				continue;
			if (atoi(rule->monthdays[i]) < 0 || atoi(rule->monthdays[i]) > 31)
				return -CLSAPI_ERR_INVALID_PARAM;
		}

		for (int i = 0; i < ARRAY_SIZE(rule->monthdays); i++) {
			if (strcmp(rule->monthdays[i], "\0") != 0)
				offset += snprintf(str_monthdays + offset, sizeof(str_monthdays) - offset, "%s ", rule->monthdays[i]);
		}
		str_monthdays[strlen(str_monthdays) - 1] = '\0';
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "monthdays", str_monthdays);
	}
	//utc time
	if (rule->flag & 0x20000) {

		if (rule->utc_time)
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "utc_time", "1");
		else
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "utc_time", "");
	}
	//target
	if (rule->flag & 0x40000) {
		string_32 target = {0};

		if (!rule->target)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (rule->target == CLSAPI_FIREWALL_TARGET_ACCEPT)
			cls_strncpy(target, "ACCEPT", sizeof(target));
		else if (rule->target == CLSAPI_FIREWALL_TARGET_REJECT)
			cls_strncpy(target, "REJECT", sizeof(target));
		else if (rule->target == CLSAPI_FIREWALL_TARGET_DROP)
			cls_strncpy(target, "DROP", sizeof(target));
		else if (rule->target == CLSAPI_FIREWALL_TARGET_NOTRACK)
			cls_strncpy(target, "NOTRACK", sizeof(target));
		else if (rule->target == CLSAPI_FIREWALL_TARGET_MARK) {
			// set mark
			string_32 str_mark = {0};

			if (rule->target_option.set_mark < 0 || rule->target_option.set_mark > 0xffffffff)
				return -CLSAPI_ERR_INVALID_PARAM;
			cls_strncpy(target, "MARK", sizeof(target));
			snprintf(str_mark, sizeof(str_mark),"%ld", rule->target_option.set_mark);
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_mark", str_mark);
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_helper", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_dscp", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_xmark", "");
		} else if (rule->target == CLSAPI_FIREWALL_TARGET_XMARK) {
			// set xmark
			string_32 str_mark = {0};

			if (rule->target_option.set_xmark < 0 || rule->target_option.set_xmark > 0xffffffff)
				return -CLSAPI_ERR_INVALID_PARAM;
			cls_strncpy(target, "MARK", sizeof(target));
			snprintf(str_mark, sizeof(str_mark),"%ld", rule->target_option.set_xmark);
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_helper", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_dscp", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_mark", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_xmark", str_mark);
		} else if (rule->target == CLSAPI_FIREWALL_TARGET_HELPER) {
			bool found = validate_conntrack_helper(rule->target_option.set_helper);

			if (!rule->target_option.set_helper || !found)
				return -CLSAPI_ERR_INVALID_PARAM;
			cls_strncpy(target, "HELPER", sizeof(target));
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_dscp", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_mark", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_xmark", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_helper", rule->target_option.set_helper);
		} else if (rule->target == CLSAPI_FIREWALL_TARGET_DSCP) {
			bool found = validate_dscp_class(rule->target_option.set_dscp);

			if (!rule->target_option.set_dscp || !found)
				return -CLSAPI_ERR_INVALID_PARAM;

			cls_strncpy(target, "DSCP", sizeof(target));
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_helper", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_mark", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_xmark", "");
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "set_dscp", rule->target_option.set_dscp);
		} else
			return -CLSAPI_ERR_INVALID_PARAM;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "target", target);
	}
	//family
	if (rule->flag & 0x80000) {
		if (!rule->family)
			return -CLSAPI_ERR_INVALID_PARAM;
		//ipv4, ipv6 or any)
		if (strcmp(rule->family, "ipv4") != 0 &&
				strcmp(rule->family, "ipv6") != 0 &&
				strcmp(rule->family, "any") != 0)
			return -CLSAPI_ERR_INVALID_PARAM;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "family", rule->family);
	}
	//limit
	if (rule->flag & 0x100000) {
		char *limit = rule->limit;
		char *endptr;

		strtod(limit, &endptr);
		if (!rule->limit)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (endptr == limit)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (strncmp(endptr, "/second", 7) == 0 ||
				strncmp(endptr, "/minute", 7) == 0 ||
				strncmp(endptr, "/hour", 5) == 0 ||
				strncmp(endptr, "/day", 4) == 0)
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "limit", rule->limit);
	}
	//limit burst
	if (rule->flag & 0x200000) {
		string_32 str_limit_burst = {0};

		if (!rule->limit_burst)
			return -CLSAPI_ERR_INVALID_PARAM;

		snprintf(str_limit_burst,sizeof(str_limit_burst), "%d", rule->limit_burst);
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "limit_burst", str_limit_burst);
	}
	//enabled
	if (rule->flag & 0x400000) {
		if (!rule->enabled)
			clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "enabled", "0");
	}
	//device + (direction)
	if (rule->flag & 0x800000) {
		char *device = rule->device;
		char *direction = rule->direction;

		if (!device || !direction)
			return -CLSAPI_ERR_INVALID_PARAM;

		if (strcmp(direction, "in") != 0 && strcmp(direction, "out") != 0)
			return -CLSAPI_ERR_INVALID_PARAM;

		ret = netdev_netif_name_validate(device);
		if (ret)
			return ret;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "direction", direction);
		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "device", device);
	}
	// dscp
	if (rule->flag & 0x1000000) {
		bool found = validate_dscp_class(rule->dscp);

		if (!rule->dscp || !found)
			return -CLSAPI_ERR_INVALID_PARAM;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "dscp", rule->dscp);
	}
	// helper
	if (rule->flag & 0x2000000) {
		bool found = validate_conntrack_helper(rule->helper);

		if (!rule->helper || !found)
			return -CLSAPI_ERR_INVALID_PARAM;

		clsconf_set_param(CLSCONF_CFG_FIREWALL, section_rule, "helper", rule->helper);
	}

	ret = CLSAPI_OK;
	clsconf_apply_cfg(CLSCONF_CFG_FIREWALL);

	return ret;
}

int clsapi_net_get_firewall_zone_names(string_128 zone_names[], int *zone_names_len)
{
	int ret = CLSAPI_OK;

	if (!zone_names || !zone_names_len || *zone_names_len <= 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_all_section_alias(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_ZONE, CLS_NETWORK_PARAM_NAME, zone_names, zone_names_len);

	return ret;
}

int clsapi_net_add_wan_dns_server(const char *interface_name, struct clsapi_net_ipaddr *dns_server)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};
	string_256 str_dns_server = {0};
	const char *section_name = interface_name;

	if (!interface_name || !dns_server)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	if (dns_server->in_family == AF_INET) {
		if (!inet_ntop(AF_INET, &dns_server->v4_addr, str_dns_server, sizeof(str_dns_server)))
			return -CLSAPI_ERR_INVALID_PARAM;
	} else if (dns_server->in_family == AF_INET6) {
		if (!inet_ntop(AF_INET6, &dns_server->v6_addr, str_dns_server, sizeof(str_dns_server)))
			return -CLSAPI_ERR_INVALID_PARAM;
	} else
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "proto", proto);
	if (ret)
		return ret;

	if (strcmp(proto, "static") != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	clsconf_defer_add_apply_list(CLSCONF_CFG_NETWORK, section_name, "dns", str_dns_server);

	return ret;
}

int clsapi_net_del_wan_dns_server(const char *interface_name, struct clsapi_net_ipaddr *dns_server)
{
	int ret = CLSAPI_OK;
	string_256 str_dns_server = {0};
	const char *section_name = interface_name;

	if (!interface_name || !dns_server)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	if (dns_server->in_family == AF_INET) {
		if (!inet_ntop(AF_INET, &dns_server->v4_addr, str_dns_server, sizeof(str_dns_server)))
			return -CLSAPI_ERR_INVALID_PARAM;
	} else if (dns_server->in_family == AF_INET6) {
		if (!inet_ntop(AF_INET6, &dns_server->v6_addr, str_dns_server, sizeof(str_dns_server)))
			return -CLSAPI_ERR_INVALID_PARAM;
	} else
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_defer_del_apply_list(CLSCONF_CFG_NETWORK, section_name, "dns", str_dns_server);

	return ret;
}

int clsapi_net_get_wan_dns_server(const char *interface_name, struct clsapi_net_ipaddr **dns_server)
{
	int ret = CLSAPI_OK;
	const char *section_name = interface_name;
	string_1024 str_dns_server[64] = {0};
	int server_num = ARRAY_SIZE(str_dns_server);

	if (!interface_name || !dns_server)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = clsconf_get_list(CLSCONF_CFG_NETWORK, section_name, "dns", str_dns_server, &server_num);
	if (ret)
		return ret;

	*dns_server = (struct clsapi_net_ipaddr *)calloc(server_num, sizeof(struct clsapi_net_ipaddr));
	if ((*dns_server) == NULL)
		return -CLSAPI_ERR_NO_MEM;

	for (int i = 0; i < server_num; i++) {
		if (inet_pton(AF_INET, str_dns_server[i], &((*dns_server)[i].v4_addr)))
			(*dns_server)[i].in_family = AF_INET;
		else if (inet_pton(AF_INET6, str_dns_server[i], &((*dns_server)[i].v4_addr)))
			(*dns_server)[i].in_family = AF_INET6;
		else {
			free(*dns_server);
			return -CLSAPI_ERR_INVALID_PARAM;
		}
	}

	return server_num;
}

int clsapi_net_get_pppoe_username(const char *interface_name, string_32 username)
{
	int ret = CLSAPI_OK;
	string_1024 p_username = {0};
	const char *section_name = interface_name;

	if (!interface_name || !username)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "username", p_username);
	if (ret)
		return ret;

	cls_strncpy(username, p_username, sizeof(string_32));

	return ret;
}

int clsapi_net_set_pppoe_username(const char *interface_name, const char *username)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};
	const char *section_name = interface_name;

	if (!interface_name || !username)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "proto", proto);
	if (ret)
		return ret;

	if (strcmp(proto, "pppoe") != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "username", username);

	return ret;
}

int clsapi_net_get_pppoe_passwd(const char *interface_name, string_32 passwd)
{
	int ret = CLSAPI_OK;
	string_1024 p_passwd = {0};
	const char *section_name = interface_name;

	if (!interface_name || !passwd)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "password", p_passwd);
	if (ret)
		return ret;

	cls_strncpy(passwd, p_passwd, sizeof(string_32));

	return ret;
}

int clsapi_net_set_pppoe_passwd(const char *interface_name, const char *passwd)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};
	const char *section_name = interface_name;

	if (!interface_name || !passwd)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, section_name, NULL);
	if (ret != -CLSAPI_ERR_EXISTED)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, section_name, "proto", proto);
	if (ret)
		return ret;

	if (strcmp(proto, "pppoe") != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "password", passwd);

	return ret;
}

int clsapi_net_get_uni_global_ipv6prefix(const char *netif_name, string_256 ipv6prefix, uint8_t *ipv6prefix_len)
{
	FILE *fp = NULL;
	string_1024 line_contents = {0};
	string_128 str_ipv6addr_hex = {0};
	uint32_t ifidx = 0;
	uint32_t scope = 0;
	uint32_t reserve = 0;
	string_32 if_name;
	bool found = false;

	if (!ipv6prefix || !ipv6prefix_len || netdev_netif_name_validate(netif_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	fp = fopen(IF_INET6_PATH, "r");
	if (!fp) {
		DBG_ERROR("Open file fail\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while (fgets(line_contents, sizeof(line_contents), fp) != NULL) {
		sscanf(line_contents, "%32s %x %hhx %x %x %s",
				str_ipv6addr_hex, &ifidx, ipv6prefix_len, &scope, &reserve, if_name);
		if (!strcmp(if_name, netif_name)) {
			//global address
			if (scope == 0 && reserve == 0) {
				found = true;
				converthex2IPv6(str_ipv6addr_hex, ipv6prefix, *ipv6prefix_len / 4);
				break;
			}
		}
	}

	fclose(fp);
	if (!found)
		return -CLSAPI_ERR_NOT_FOUND;

	return CLSAPI_OK;
}

int clsapi_net_get_ula_global_ipv6prefix(const char *netif_name, string_256 ipv6prefix, uint8_t *ipv6prefix_len)
{
	FILE *fp = NULL;
	string_1024 line_contents = {0};
	string_128 str_ipv6addr_hex = {0};
	uint32_t ifidx = 0;
	uint32_t scope = 0;
	uint32_t reserve = 0;
	string_32 if_name;
	bool found = false;

	if (!ipv6prefix || !ipv6prefix_len || netdev_netif_name_validate(netif_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	fp = fopen(IF_INET6_PATH, "r");
	if (!fp) {
		DBG_ERROR("Open file fail\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while (fgets(line_contents, sizeof(line_contents), fp) != NULL) {
		sscanf(line_contents, "%32s %x %hhx %x %x %s",
				str_ipv6addr_hex, &ifidx, ipv6prefix_len, &scope, &reserve, if_name);
		if (!strcmp(if_name, netif_name)) {
			//global address
			if (scope == 0 && reserve == 0x80) {
				found = true;
				converthex2IPv6(str_ipv6addr_hex, ipv6prefix, *ipv6prefix_len / 4);
				break;
			}
		}
	}

	fclose(fp);
	if (!found)
		return -CLSAPI_ERR_NOT_FOUND;

	return CLSAPI_OK;
}

int clsapi_net_get_all_ipv6addrs(const char *netif_name, struct clsapi_ipv6_info **ipv6_addrs)
{
	FILE *fp = NULL;
	string_32 if_name;
	uint32_t flag = 0;
	int addr_count = 0;
	uint32_t ifidx = 0;
	uint32_t scope = 0;
	uint8_t ipv6prefix_len = 0;
	string_128 str_ipv6addr = {0};
	string_1024 line_contents = {0};
	string_128 str_ipv6addr_hex = {0};
	struct clsapi_ipv6_info local_ipv6addrs[256];

	if (!ipv6_addrs || netdev_netif_name_validate(netif_name) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	fp = fopen(IF_INET6_PATH, "r");
	if (!fp) {
		DBG_ERROR("Open file fail\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while (fgets(line_contents, sizeof(line_contents), fp) != NULL) {
		sscanf(line_contents, "%32s %x %hhx %x %x %s",
				str_ipv6addr_hex, &ifidx, &ipv6prefix_len, &scope, &flag, if_name);
		if (!strcmp(if_name, netif_name)) {
			converthex2IPv6(str_ipv6addr_hex, str_ipv6addr, 32);
			inet_pton(AF_INET6, str_ipv6addr, &local_ipv6addrs[addr_count].addr) ;

			if (scope == 0  && flag == 0)
				local_ipv6addrs[addr_count].scope = CLSAPI_ADDR_GLOBAL;
			else if (scope == 0x20  && flag == 0x80)
				local_ipv6addrs[addr_count].scope = CLSAPI_ADDR_LINKLOCAL;
			else if (scope == 0 && flag == 0x80)
				local_ipv6addrs[addr_count].scope = CLSAPI_ADDR_ULA_LOCAL;
			else
				local_ipv6addrs[addr_count].scope = CLSAPI_ADDR_OTHERS;
			addr_count++;
		}
	}

	if (addr_count == 0) {
		fclose(fp);
		return -CLSAPI_ERR_NOT_FOUND;
	}

	*ipv6_addrs = (struct clsapi_ipv6_info *)calloc(addr_count, sizeof(struct clsapi_ipv6_info));
	if (!(*ipv6_addrs)) {
		fclose(fp);
		return -CLSAPI_ERR_NO_MEM;
	}

	memcpy(*ipv6_addrs, local_ipv6addrs, addr_count * sizeof(struct clsapi_ipv6_info));
	fclose(fp);

	return addr_count;
}

int clsapi_net_get_default_gateway(const char *netif_name, struct in_addr *ipaddr)
{
	FILE *fp;
	char *tmp;
	string_256 line = {0};
	string_128 cmd = {0};
	string_128 gateway = {0};
	bool found = false;

	if (netdev_netif_name_validate(netif_name) || !ipaddr)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(cmd, sizeof(cmd), "ip -4 route show dev %s", netif_name);

	fp = popen(cmd, "r");
	if (!fp) {
		DBG_ERROR("Failed to run command\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		if ((tmp = strstr(line, "via")) != NULL) {
			sscanf(tmp, "%*s %s %*s", gateway);
			found = true;
			inet_pton(AF_INET, gateway, ipaddr);
			break;
		}
	}
	// Close the command stream.
	pclose(fp);

	if (!found)
		return -CLSAPI_ERR_NOT_FOUND;

	return CLSAPI_OK;
}

int clsapi_net_get_ipv6_default_gateway(const char *netif_name, struct in6_addr *ipaddr)
{
	FILE *fp;
	char *tmp;
	string_256 line = {0};
	string_128 cmd = {0};
	string_128 gateway = {0};
	bool found = false;

	if (netdev_netif_name_validate(netif_name) || !ipaddr)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(cmd, sizeof(cmd), "ip -6 route show dev %s", netif_name);

	fp = popen(cmd, "r");
	if (!fp) {
		DBG_ERROR("Failed to run command\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		tmp = strstr(line, "via");
		if (tmp != NULL) {
			sscanf(tmp, "%*s %s %*s", gateway);
			found = true;
			inet_pton(AF_INET6, gateway, ipaddr);
			break;
		}
	}
	// Close the command stream.
	pclose(fp);

	if (!found)
		return -CLSAPI_ERR_NOT_FOUND;

	return CLSAPI_OK;
}

int clsapi_net_del_firewall_rule(struct clsapi_net_firewall_rule *rule)
{
	string_32 section_rule = {0};
	int ret = CLSAPI_OK;

	if (!rule || !rule->name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_FIREWALL, CLS_NETWORK_SECT_TYPE_RULE, section_rule, 2, CLS_NETWORK_PARAM_NAME, rule->name);
	if (ret)
		return ret;

	clsconf_defer_del_section(CLSCONF_CFG_FIREWALL, section_rule);

	return ret;
}

int clsapi_net_get_ipv6prefixDelegationEnabled(const char *interface_name, bool *enabled)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};
	string_1024 delegate = {0};

	if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL) != -CLSAPI_ERR_EXISTED || !enabled)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "proto", proto);
	if (ret < 0)
		return ret;

	if (strcmp(proto, "static") != 0) {
		DBG_ERROR("Not support to get because of protocol\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "delegate", delegate);
	if (ret < 0) {
		*enabled = true;
		ret = CLSAPI_OK;
	} else
		*enabled = false;

	return ret;
}

int clsapi_net_get_ipv6ConnStatus(const char *interface_name, bool *connected)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, ret = CLSAPI_OK;
	string_1024 proto = {0};
	string_1024 netif_name = {0};
	char host[NI_MAXHOST];
	int ipv6_connected = 0;

	if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL) != -CLSAPI_ERR_EXISTED || !connected)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (getifaddrs(&ifaddr) == -1) {
		DBG_ERROR("Getifaddrs\n");
		return -CLSAPI_ERR_INTERNAL_ERR;
	}

	//make sure protocol of this netif_name is not static
	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "proto", proto);
	if (ret < 0)
		return ret;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "device", netif_name);
	if (ret < 0)
		return ret;

	if (strcmp(proto, "static") == 0) {
		DBG_ERROR("Not support to get because of protocol\n");
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if (family == AF_INET6 && strcmp(ifa->ifa_name, netif_name) == 0) {
			if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6),
						host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
				if (strncmp(host, "fe80::", 6) != 0) {
					ipv6_connected = 1;
				}
			}
		}
	}

	freeifaddrs(ifaddr);

	if (ipv6_connected)
		*connected = true;
	else
		*connected = false;

	return CLSAPI_OK;
}

int clsapi_net_get_ipv6DSliteEnabled(const char *interface_name, bool *enabled)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};

	if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL) != -CLSAPI_ERR_EXISTED || !enabled)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "proto", proto);
	if (ret < 0)
		return ret;

	if (strcmp(proto, "dslite") == 0)
		*enabled = true;
	else
		*enabled = false;

	return CLSAPI_OK;
}

int clsapi_net_get_ipv6AddrAlloc(const char *interface_name, enum clsapi_ipv6addr_allocation *alloc_method)
{
	int ret = CLSAPI_OK;
	string_1024 proto = {0};
	string_1024 device = {0};
	struct in6_addr ipv6addr;
	string_128 str_ipv6addr;

	if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL) != -CLSAPI_ERR_EXISTED || !alloc_method)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "proto", proto);
	if (ret < 0)
		return ret;
	if (strcmp(proto, "static") == 0)
		*alloc_method = CLSAPI_IPV6ADDR_Manual_Assign;
	else {
		ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "device", device);
		if (ret < 0)
			return ret;

		ret = clsapi_net_get_ipv6addr(device, &ipv6addr);
		if (ret < 0)
			return ret;
		inet_ntop(AF_INET6, &ipv6addr, str_ipv6addr, sizeof(str_ipv6addr));
		uint8_t mac_bytes[6];
		char eui64[24] = {0};
		const char *interface_identifier = strchr(str_ipv6addr, ':');

		ret = clsapi_net_get_macaddr(device, mac_bytes);
		if (ret < 0)
			return ret;

		// Flip the 7th bit of the first byte
		mac_bytes[0] ^= 0x02;

		sprintf(eui64, "%02x:%02x:%02xff:fe%02x:%02x:%02x",
				mac_bytes[0], mac_bytes[1], mac_bytes[2], mac_bytes[3], mac_bytes[4], mac_bytes[5]);

		for (int i = 0; i < 3 && interface_identifier; ++i)
			interface_identifier = strchr(interface_identifier + 1, ':');

		if (!interface_identifier)
			return 0; // Invalid IPv6 address

		interface_identifier++; // Skip the colon

		// Compare the EUI-64 identifier with the interface identifier
		if (strcasecmp(interface_identifier, eui64) == 0)
			*alloc_method = CLSAPI_IPV6ADDR_SLAAC;
		else
			*alloc_method = CLSAPI_IPV6ADDR_DHCPv6;
	}

	return ret;
}

int clsapi_net_get_ipv6PrefixAlloc(const char *interface_name, enum clsapi_ipv6prefix_allocation *allocation)
{
	int ret = CLSAPI_OK;
	string_1024 delegate = {0};
	bool connected = false;
	string_1024 proto = {0};

	if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, interface_name, NULL) != -CLSAPI_ERR_EXISTED || !allocation)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "proto", proto);
	if (ret < 0)
		return ret;

	if (strcmp(proto, "static") != 0) {
		ret = clsapi_net_get_ipv6ConnStatus(interface_name, &connected);
		if (ret < 0)
			return ret;
		if (!connected)
			return -CLSAPI_ERR_NOT_SUPPORTED;
	}

	ret = clsconf_get_param(CLSCONF_CFG_NETWORK, interface_name, "delegate", delegate);
	if (ret == -CLSAPI_ERR_NOT_FOUND || ret == CLSAPI_OK) {
		ret = CLSAPI_OK;
		if (strcmp(delegate, "0") == 0)
			*allocation = CLSAPI_IPV6PRE_STATIC;
		else
			*allocation = CLSAPI_IPV6PRE_PD;
	}

	return ret;
}

int clsapi_net_add_bridge_vlan(const char *bridge_name, const uint16_t vlan_id)
{
	int ret = CLSAPI_OK;
	string_1024 section_name = {0};
	string_1024 section_brvlan_name = {0};
	string_32 str_vlan_id = {0};

	if (vlan_id <= 1 || vlan_id > 4094)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_DEVICE,
			section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret != CLSAPI_OK)
		return ret;

	snprintf(str_vlan_id, sizeof(str_vlan_id), "%d", vlan_id);
	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_BRIDGE_VLAN,
			section_brvlan_name, 4, CLS_NETWORK_PARAM_DEVICE, bridge_name, "vlan", str_vlan_id);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		clsconf_defer_add_section(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_BRIDGE_VLAN,
				CLSCONF_SECNAME_ANON, section_name);
		if (ret < 0)
			return ret;

		ret = clsconf_set_param(CLSCONF_CFG_NETWORK, section_name, "device", bridge_name);
		if (ret < 0)
			return ret;

		clsconf_defer_apply_int_param(CLSCONF_CFG_NETWORK, section_name, "vlan", vlan_id);
		if (ret < 0)
			return ret;
	}

	return ret;
}

int clsapi_net_del_bridge_vlan(const char *bridge_name, const uint16_t vlan_id)
{
	int ret = CLSAPI_OK;
	string_32 str_vlan_id = {0};
	string_1024 section_name = {0};
	string_1024 section_brvlan_name = {0};
	int section_len = sizeof(section_name);
	string_1024 str_ports[64] = {0};
	int ports_num = ARRAY_SIZE(str_ports);

	if (vlan_id <= 1 || vlan_id > 4094)
		return -CLSAPI_ERR_INVALID_PARAM;


	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_DEVICE,
			section_brvlan_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret != CLSAPI_OK)
		return ret;

	snprintf(str_vlan_id, sizeof(str_vlan_id), "%d", vlan_id);
	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_BRIDGE_VLAN,
			section_brvlan_name, 4, CLS_NETWORK_PARAM_DEVICE, bridge_name, "vlan", str_vlan_id);
	//search for br-vlan which ID is str_vlan_id
	if (ret == CLSAPI_OK) {
		//found br-vlan
		ret = clsconf_get_list(CLSCONF_CFG_NETWORK, section_brvlan_name, "ports", str_ports, &ports_num);
		//search for the port list which belongs to this br-vlan
		if (ret == CLSAPI_OK) {
			//found this br-vlan port list
			for (int i = 0; i < ports_num; i++) {
				char *colon_ptr = strchr(str_ports[i], ':');
				string_1024 str_wireless_ports[64] = {0};
				int wireless_ports_num = ARRAY_SIZE(str_wireless_ports);

				if (colon_ptr != NULL) {
					*colon_ptr = '\0';
				}

				ret = clsconf_ifname_to_section(str_ports[i], NULL, 0, section_name, section_len);
				if (ret == CLSAPI_OK) {
					ret = clsconf_get_list(CLSCONF_CFG_WIRELESS, section_name, "brvlan", str_wireless_ports, &wireless_ports_num);
					if (ret < CLSAPI_OK)
						return ret;

					for (int i = 0; i < wireless_ports_num; i++) {
						if (strstr(str_wireless_ports[i], str_vlan_id) != NULL) {
							clsconf_defer_del_apply_list(CLSCONF_CFG_WIRELESS, section_name,
									"brvlan", str_wireless_ports[i]);
							break;
						}
					}
				}
			}
		}

		clsconf_defer_del_section(CLSCONF_CFG_NETWORK, section_brvlan_name);
		if (ret < 0)
			return ret;
	} else {
		DBG_ERROR("Internal_error: br-vlan not found!.\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	return ret;
}

int clsapi_net_enable_bridge_vlan(const char *bridge_name, const bool enabled)
{
	int ret = CLSAPI_OK;
	string_1024 section_name = {0};

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_DEVICE,
			section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret != CLSAPI_OK)
		return ret;

	if (!enabled)
		clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "vlan_disabled", "1");
	else
		clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_name, "vlan_disabled", "");

	return ret;
}

int clsapi_net_add_bridge_vlan_port(const char *bridge_name, const uint16_t vlan_id,
		const char *port, const bool tagged, const bool pvid)
{
	string_32 section_name;
	int ret = CLSAPI_OK;
	int section_len = sizeof(section_name);
	bool found = false;
	string_32 policy = {0};
	string_32 str_vlan_id = {0};
	string_1024 str_ports[64] = {0};
	int ports_num = ARRAY_SIZE(str_ports);
	string_1024 section_brvlan_name = {0};

	if (vlan_id <= 1 || vlan_id > 4094)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (netdev_netif_name_validate(port) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_ifname_to_section(port, NULL, 0, section_name, section_len);

	if (ret != CLSAPI_OK) {
		//network
		string_1024 section_name = {0};

		ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK,
				CLS_NETWORK_SECT_DEVICE, section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
		if (ret != CLSAPI_OK)
			return ret;

		string_1024 str_netdev_ports[64] = {0};
		int netdev_ports_num = ARRAY_SIZE(str_netdev_ports);

		ret = clsconf_get_list(CLSCONF_CFG_NETWORK, section_name, "ports",
				str_netdev_ports, &netdev_ports_num);
		if (ret != CLSAPI_OK && ret != -CLSAPI_ERR_NOT_FOUND)
			return ret;

		for (int i = 0; i < netdev_ports_num; i++) {
			if (strncmp(str_netdev_ports[i], port, sizeof(str_netdev_ports[i])) == 0) {
				found = true;
				break;
			}
		}

		if (!found)
			return -CLSAPI_ERR_INVALID_PARAM;

		found = false;

		snprintf(str_vlan_id, sizeof(str_vlan_id), "%d", vlan_id);
		ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_BRIDGE_VLAN,
				section_brvlan_name, 4, CLS_NETWORK_PARAM_DEVICE, bridge_name, "vlan", str_vlan_id);
		if (ret == CLSAPI_OK) {
			if (tagged && pvid)
				snprintf(policy, sizeof(policy), "%s:t*", port);
			else if (tagged && !pvid)
				snprintf(policy, sizeof(policy), "%s:t", port);
			else if (!tagged && !pvid)
				snprintf(policy, sizeof(policy), "%s:u", port);
			else
				snprintf(policy, sizeof(policy), "%s:u*", port);

			ret = clsconf_get_list(CLSCONF_CFG_NETWORK, section_brvlan_name, "ports",
					str_ports, &ports_num);
			if (ret != CLSAPI_OK && ret != -CLSAPI_ERR_NOT_FOUND)
				return ret;

			for (int i = 0; i < ports_num; i++) {
				if (strncmp(str_ports[i], policy, sizeof(str_ports[i])) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				clsconf_defer_add_apply_list(CLSCONF_CFG_NETWORK, section_brvlan_name, "ports", policy);
				ret = CLSAPI_OK;

				return ret;
			} else {
				DBG_ERROR("Internal_error:invalid parameter, port existed.\n");
				return -CLSAPI_ERR_EXISTED;
			}
		}
	} else {
		//wireless
		memset(policy, 0, sizeof(policy));
		memset(str_ports, 0, sizeof(str_ports));

		ret = clsconf_check_section_param_existed(CLSCONF_CFG_WIRELESS, section_name, NULL);
		if (ret == -CLSAPI_ERR_EXISTED) {
			if (tagged && pvid)
				snprintf(policy, sizeof(policy), "%d:t*", vlan_id);
			else if (tagged && !pvid)
				snprintf(policy, sizeof(policy), "%d:t", vlan_id);
			else if (!tagged && !pvid)
				snprintf(policy, sizeof(policy), "%d:u", vlan_id);
			else
				snprintf(policy, sizeof(policy), "%d:u*", vlan_id);

			ret = clsconf_get_list(CLSCONF_CFG_WIRELESS, section_name, "brvlan", str_ports, &ports_num);
			if (ret != CLSAPI_OK && ret != -CLSAPI_ERR_NOT_FOUND)
				return ret;

			for (int i = 0; i < ports_num; i++) {
				if (strncmp(str_ports[i], policy, sizeof(str_ports[i])) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				clsconf_defer_add_apply_list(CLSCONF_CFG_WIRELESS, section_name, "brvlan", policy);
				ret = CLSAPI_OK;
				return ret;
			} else {
				DBG_ERROR("Internal_error:invalid parameter, port existed.\n");
				return -CLSAPI_ERR_EXISTED;
			}
		}
	}

	return ret;
}

int clsapi_net_del_bridge_vlan_port(const char *bridge_name, const uint16_t vlan_id, const char *port)
{
	int ret = CLSAPI_OK;
	string_32 policy = {0};
	string_32 str_vlan_id = {0};
	string_1024 section_name = {0};
	string_1024 str_ports[64] = {0};
	int ports_num = ARRAY_SIZE(str_ports);
	string_1024 section_brvlan_name = {0};
	int section_len = sizeof(section_name);

	if (vlan_id <= 1 || vlan_id > 4094)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (netdev_netif_name_validate(port) != 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_ifname_to_section(port, NULL, 0, section_name, section_len);

	if (ret != CLSAPI_OK) {
		//network
		ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK,
				CLS_NETWORK_SECT_DEVICE, section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
		if (ret != CLSAPI_OK)
			return ret;

		snprintf(str_vlan_id, sizeof(str_vlan_id), "%d", vlan_id);
		ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK,
				CLS_NETWORK_SECT_BRIDGE_VLAN, section_brvlan_name, 4,
				CLS_NETWORK_PARAM_DEVICE, bridge_name, "vlan", str_vlan_id);
		if (ret == CLSAPI_OK) {
			ret = clsconf_get_list(CLSCONF_CFG_NETWORK, section_brvlan_name,
					"ports", str_ports, &ports_num);
			if (ret != CLSAPI_OK && ret != -CLSAPI_ERR_NOT_FOUND)
				return ret;

			for (int i = 0; i < ports_num; i++) {
				if (strstr(str_ports[i], port) != NULL) {
					clsconf_defer_del_apply_list(CLSCONF_CFG_NETWORK, section_brvlan_name,
							"ports", str_ports[i]);
					continue;
				}
			}
			}

	} else {
		//wireless
		memset(str_ports, 0, sizeof(str_ports));

		clsconf_ifname_to_section(port, NULL, 0, section_name, section_len);
		ret = clsconf_check_section_param_existed(CLSCONF_CFG_WIRELESS, section_name, NULL);
		if (ret == -CLSAPI_ERR_EXISTED) {
			ret = clsconf_get_list(CLSCONF_CFG_WIRELESS, section_name, "brvlan", str_ports, &ports_num);
			if (ret != CLSAPI_OK && ret != -CLSAPI_ERR_NOT_FOUND)
				return ret;

			snprintf(policy, sizeof(policy), "%d", vlan_id);
			for (int i = 0; i < ports_num; i++) {
				if (strstr(str_ports[i], policy) != NULL) {
					clsconf_defer_del_apply_list(CLSCONF_CFG_WIRELESS, section_name, "brvlan", str_ports[i]);
					continue;
				}
			}
		}
	}

		return ret;
}

int clsapi_net_set_bridge_vlan_primary(const char *bridge_name, const uint16_t vlan_id, const bool primary)
{
	int ret = CLSAPI_OK;
	string_32 str_vlan_id = {0};
	string_1024 section_name = {0};
	string_1024 section_brvlan_name = {0};
	string_1024 pre_section_brvlan_name = {0};

	snprintf(str_vlan_id, sizeof(str_vlan_id), "%d", vlan_id);

	if (vlan_id <= 1 || vlan_id > 4094)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK, CLS_NETWORK_SECT_DEVICE,
			section_name, 2, CLS_NETWORK_PARAM_NAME, bridge_name);
	if (ret != CLSAPI_OK)
		return ret;

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK,
			CLS_NETWORK_SECT_BRIDGE_VLAN, pre_section_brvlan_name,
			4,
			CLS_NETWORK_PARAM_DEVICE, bridge_name,
			"primary", "1");

	ret = clsconf_find_section_by_param(CLSCONF_CFG_NETWORK,
			CLS_NETWORK_SECT_BRIDGE_VLAN, section_brvlan_name, 4,
			CLS_NETWORK_PARAM_DEVICE, bridge_name,
			"vlan", str_vlan_id);

	if (strcmp(pre_section_brvlan_name,"") != 0 && strcmp(pre_section_brvlan_name, section_brvlan_name) != 0) {
		clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, pre_section_brvlan_name, "primary", "");
	}

	if (ret == CLSAPI_OK) {
		if (primary)
			clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_brvlan_name, "primary", "1");
		else
			clsconf_defer_apply_param(CLSCONF_CFG_NETWORK, section_brvlan_name, "primary", "");
	}

	return ret;
}

int clsapi_net_get_ipv6PrefixOrigin(const char *netif_name, enum clsapi_ipv6_prefix_origin *prefix_origin)
{
	FILE *fp;
	int ret = CLSAPI_OK;
	uint8_t ipv6prefix_len = 0;
	string_256 ipv6prefix = {0};
	string_128 cmd = {0};
	string_512 line = {0};
	bool hasPrefix = false;
	struct netdev_info info = {0};
	enum clsapi_net_protocol proto = _CLSAPI_NETWORK_PROTO_MAX;

	if (!netif_name || !prefix_origin)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = get_netdev_info(netif_name, &info);
	if (ret)
		return ret;

	if (!strcmp(info.zone, "wan")) {
		snprintf(cmd, sizeof(cmd), "ip -6 route show dev %s", netif_name);

		fp = popen(cmd, "r");
		if (!fp) {
			DBG_ERROR("Failed to run command\n");
			return -CLSAPI_ERR_FILE_OPERATION;
		}

		while (fgets(line, sizeof(line), fp)) {
			if (strncmp(line, "default from", 12) == 0) {
				char *token;
				int count = 0;
				token = strtok(line, " \t\n");
				while (token != NULL) {
					count++;
					if (count == 3) {
						if (strchr(token, '/'))
							hasPrefix = true;
						break;
					}
					token = strtok(NULL, " \t\n");
				}
			}
		}

		pclose(fp);

		if (!hasPrefix) {
			if (clsapi_net_get_proto(info.section_name, &proto))
				*prefix_origin = CLSAPI_PREFIX_ORIGIN_UNKNOW;
			else {
				switch (proto) {
				case CLSAPI_NETWORK_PROTO_STATIC:
					if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, info.section_name, "ip6addr") != -CLSAPI_ERR_EXISTED)
						*prefix_origin = CLSAPI_PREFIX_ORIGIN_UNKNOW;
					else
						*prefix_origin = CLSAPI_PREFIX_ORIGIN_STATIC;
					break;
				default:
					break;
				}
			}
		} else
			*prefix_origin = CLSAPI_PREFIX_ORIGIN_PREFIX_DELEGATION;
	} else if (!strcmp(info.zone, "lan")) {
		if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, info.section_name, "ip6assign") != -CLSAPI_ERR_EXISTED)
			*prefix_origin = CLSAPI_PREFIX_ORIGIN_UNKNOW;


		if (clsapi_net_get_uni_global_ipv6prefix(netif_name, ipv6prefix, &ipv6prefix_len)) {
			if (clsapi_net_get_ula_global_ipv6prefix(netif_name, ipv6prefix, &ipv6prefix_len))
				*prefix_origin = CLSAPI_PREFIX_ORIGIN_UNKNOW;
			else
				*prefix_origin = CLSAPI_PREFIX_ORIGIN_STATIC;
		} else
			*prefix_origin = CLSAPI_PREFIX_ORIGIN_PREFIX_DELEGATION;
	}

	return ret;
}

int clsapi_net_set_ipv6PrefixOrigin(const char *netif_name, enum clsapi_ipv6_prefix_origin prefix_origin)
{
	int ret = CLSAPI_OK;
	struct netdev_info info = {0};

	if (!netif_name || !(prefix_origin != CLSAPI_PREFIX_ORIGIN_PREFIX_DELEGATION ||
		prefix_origin != CLSAPI_PREFIX_ORIGIN_STATIC))
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = get_netdev_info(netif_name, &info);
	if (ret)
		return ret;

	if (!strcmp(info.zone, "lan")) {
		if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, "lan", "ip6assign") != -CLSAPI_ERR_EXISTED)
			clsconf_set_param(CLSCONF_CFG_NETWORK, info.section_name, "ip6assign", IPV6_PREFIX_ASSIGN_BITS);

		if (clsconf_check_section_param_existed(CLSCONF_CFG_NETWORK, "globals", "ula_prefix") != -CLSAPI_ERR_EXISTED)
			clsconf_set_param(CLSCONF_CFG_NETWORK, "globals", "ula_prefix", DEFAULT_IPV6_ULA_ADDR);
	}

	switch (prefix_origin) {
	case CLSAPI_PREFIX_ORIGIN_PREFIX_DELEGATION:
		if (!strcmp(info.zone, "wan")) {
			clsconf_set_param(CLSCONF_CFG_NETWORK, info.section_name, "proto", "dhcpv6");
			clsconf_set_param(CLSCONF_CFG_NETWORK, info.section_name, "reqprefix", "auto");
		}
		break;
	case CLSAPI_PREFIX_ORIGIN_STATIC:
		if (!strcmp(info.zone, "wan")) {
			clsconf_set_param(CLSCONF_CFG_NETWORK, info.section_name, "proto", "static");
			clsconf_add_list(CLSCONF_CFG_NETWORK, info.section_name, "ip6addr", DEFAULT_IPV6_STATIC_ADDR);
			clsconf_set_param(CLSCONF_CFG_NETWORK, info.section_name, "ip6prefix", DEFAULT_IPV6_PREFIX);
		}
		break;
	default:
		break;
	}

	clsconf_apply_cfg(CLSCONF_CFG_NETWORK);

	return ret;
}
