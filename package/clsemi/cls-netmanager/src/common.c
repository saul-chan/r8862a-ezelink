/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <time.h>
#include <libubox/uloop.h>
#include "common.h"
#include "log.h"

extern struct dlist_head timer_list;

int macstr_to_array(char *mac_str, unsigned char *mac)
{
	if (!mac_str || !mac)
		return -1;
	if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != ETH_ALEN)
		return -1;
	return 0;
}

void ipuint_to_str(unsigned int addr, char *str)
{
	unsigned char bytes[4];

	bytes[0] = (addr >> 24) & 0xFF;
	bytes[1] = (addr >> 16) & 0xFF;
	bytes[2] = (addr >> 8)  & 0xFF;
	bytes[3] = addr & 0xFF;
	sprintf(str, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

unsigned int ipstr_to_uint(char *str)
{
	struct in_addr addr;

	if (inet_pton(AF_INET, str, &addr) != 1)
		return 0;
	return ntohl(addr.s_addr);
}

void trim_str(char *str, char *trimstr)
{
	size_t len;
	size_t start;
	size_t end;
	size_t new_len;

	if (str == NULL)
		return;

	len = strlen(str);
	if (len == 0)
		return;

	start = 0;
	while (start < len && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r'))
		start++;

	end = len - 1;
	while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\n' || str[end] == '\r'))
		end--;

	if (start == 0)
		new_len = end + 1;
	else
		new_len = (end >= start) ? (end - start + 1) : 0;

	if (start >= 0 || new_len <= len) {
		strncpy(trimstr, str + start, new_len);
		trimstr[new_len] = '\0';
	}
}

int time_before(struct timespec *a, struct timespec *b)
{
	return (a->tv_sec < b->tv_sec) ||
	       (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec);
}

void time_sub(struct timespec *a, struct timespec *b, struct timespec *res)
{
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (res->tv_nsec < 0) {
		res->tv_sec--;
		res->tv_nsec += NSEC_PER_SEC;
	}
}

void remove_timer(struct backhaul_timer *timer)
{
	dlist_del(&timer->lh);
	if (timer->free_arg && timer->arg)
		timer->free_arg(timer->arg);
	free(timer);
}

int register_timer(long sec, long nsec, timer_handler handler, void *arg, timer_free_arg free_arg)
{
	struct dlist_head *lh;
	struct backhaul_timer *timer, *tmp;
	struct timespec now;

	if (!handler)
		return -EINVAL;

	timer = calloc(1, sizeof(struct backhaul_timer));
	if (!timer)
		return -ENOMEM;

	timer->handler = handler;
	timer->arg = arg;
	timer->free_arg = free_arg;
	clock_gettime(CLOCK_MONOTONIC, &now);
	timer->timeout.tv_nsec = now.tv_nsec + nsec;
	timer->timeout.tv_sec = now.tv_sec + sec;
	if (timer->timeout.tv_nsec > NSEC_PER_SEC) {
		timer->timeout.tv_nsec -= NSEC_PER_SEC;
		timer->timeout.tv_sec++;
	}

	tmp = dlist_first_entry(&timer_list, struct backhaul_timer, lh);
	if (!tmp) {
		dlist_add_tail(&timer->lh, &timer_list);
		return 0;
	} else if (time_before(&timer->timeout, &tmp->timeout)) {
		__dlist_add(&timer->lh, tmp->lh.prev, &tmp->lh);
		return 0;
	}

	lh = &timer_list;
	dlist_for_each_entry(tmp, lh, lh) {
		if (time_before(&timer->timeout, &tmp->timeout)) {
			__dlist_add(&timer->lh, tmp->lh.prev, &tmp->lh);
			return 0;
		}
	}

	dlist_add_tail(&timer->lh, &timer_list);

	return 0;
}

void cancel_timer(timer_handler handler, void *arg)
{
	struct dlist_head *lh = &timer_list;
	struct backhaul_timer *timer, *prev;

	dlist_for_each_entry_safe(timer, prev, lh, lh) {
		if (timer->handler == handler && timer->arg == arg)
			remove_timer(timer);
	}
}

void clean_timer_list(void)
{
	struct backhaul_timer *timer, *prev;
	struct dlist_head *lh = &timer_list;

	dlist_for_each_entry_safe(timer, prev, lh, lh)
		remove_timer(timer);
}

void register_uloop_timer(struct uloop_timeout *timer, int sec, uloop_timer_handler handler)
{
	int msec = 0;

	log_debug("%s: Enter\n", __func__);
	timer->cb = handler;
	msec = sec*1000;
	uloop_timeout_set(timer, msec);
	log_debug("%s: Exit\n", __func__);
}

void cancel_uloop_timer(struct uloop_timeout *timer)
{
	log_debug("%s: Enter\n", __func__);
	uloop_timeout_cancel(timer);
	log_debug("%s: Exit\n", __func__);
}

int is_intf_exist(char *ifname)
{
	int sockfd = -1;
	struct ifconf ifc;
	struct ifreq *ifr;
	char buf[STR_LEN_1025];
	int found = 0;
	int num = 0;
	int i = 0;

	log_debug("%s: Enter\n", __func__);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
		log_debug("ioctl error");
		close(sockfd);
		return -1;
	}

	ifr = ifc.ifc_req;
	num = ifc.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < num; i++) {
		if (strcmp(ifr[i].ifr_name, ifname) == 0) {
			found = 1;
			break;
		}
	}

	close(sockfd);
	log_debug("interface %s exist: %d\n", ifname, found);
	log_debug("%s: Exit\n", __func__);
	return found;
}

int get_intfinfo(const char *ifname, struct interface_info *intf_info)
{
	struct ifreq ifr;
	int sock = -1;
	struct in_addr addr;

	log_debug("%s: Enter\n", __func__);
	strncpy(intf_info->ifname, ifname, sizeof(intf_info->ifname) - 1);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
		close(sock);
		return -1;
	}
	intf_info->ifindex = ifr.ifr_ifindex;

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		close(sock);
		return -1;
	}
	memcpy(intf_info->hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		close(sock);
		memset(intf_info->ipstr, 0, sizeof(intf_info->ipstr));
		return -1;
	}
	memcpy(&intf_info->ipaddr, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, sizeof(struct in_addr));
	sprintf(intf_info->ipstr, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0) {
		close(sock);
		return -1;
	}
	memcpy(&intf_info->subnet, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, sizeof(struct in_addr));
	close(sock);
	addr.s_addr = intf_info->subnet;
	log_debug("Get interface info for %s: index=%d, ipstr=%s, subnet=%s "MACFMT"\n", intf_info->ifname,
		intf_info->ifindex, intf_info->ipstr, inet_ntoa(addr), MACARG(intf_info->hwaddr));
	log_debug("%s: Exit\n", __func__);
	return 0;
}

int get_intfip(const char *ifname, struct interface_info *intf_info)
{
	struct ifreq ifr;
	int sock = -1;

	log_debug("%s: Enter\n", __func__);
	strncpy(intf_info->ifname, ifname, sizeof(intf_info->ifname) - 1);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		close(sock);
		memset(intf_info->ipstr, 0, sizeof(intf_info->ipstr));
		return -1;
	}
	memcpy(&intf_info->ipaddr, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, sizeof(struct in_addr));
	sprintf(intf_info->ipstr, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0) {
		close(sock);
		return -1;
	}
	memcpy(&intf_info->subnet, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, sizeof(struct in_addr));
	close(sock);
	log_debug("Get interface ip for %s: ip=%s\n", intf_info->ifname, intf_info->ipstr);
	log_debug("%s: Exit\n", __func__);
	return 0;
}

int get_bhconf(struct backhaul_conf *bhconf)
{
	int ret = CLSAPI_OK;
	char value[STR_LEN_1025] = {0};

	log_debug("%s: Enter\n", __func__);
	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_ARPGW, UCI_PARA_INTERVAL, value);
	if (ret)
		return ret;
	bhconf->arping_gw_interval = atoi(value);

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_ARPGW, UCI_PARA_TIMEOUT, value);
	if (ret)
		return ret;
	bhconf->arping_gw_timeout = atoi(value);

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_ARPGW, UCI_PARA_RETRY_COUNT, value);
	if (ret)
		return ret;
	bhconf->arping_gw_count = atoi(value);

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_NETCONN_CHECK, UCI_PARA_INTERVAL, value);
	if (ret)
		return ret;
	bhconf->netconn_check_interval = atoi(value);

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_NETCONN_CHECK, UCI_PARA_RETRY_COUNT, value);
	if (ret)
		return ret;
	bhconf->netconn_check_count = atoi(value);

	log_debug("bh_conf info: arping_gw_interval=%d, arping_gw_timeout=%d, arping_gw_count=%d\n",
		bhconf->arping_gw_interval, bhconf->arping_gw_timeout, bhconf->arping_gw_count);
	log_debug("netconn_check_interval=%d, netconn_check_count=%d\n",
		bhconf->netconn_check_interval, bhconf->netconn_check_count);
	log_debug("%s: Exit\n", __func__);
	return ret;
}

int get_netinfo_conf(enum clsapi_net_opmode *opmode, char *wifname)
{
	int ret = CLSAPI_OK;
	char ifname[STR_LEN_1025] = {0};

	log_debug("%s: Enter\n", __func__);
	ret = clsapi_net_get_opmode(opmode);
	if (ret)
		return ret;

	if (*opmode >= _CLSAPI_NETWORK_OPMODE_MAX || *opmode < CLSAPI_NETWORK_OPMODE_ROUTER)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (*opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
		ret = clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_WAN, UCI_PARA_DEVICE, ifname);
		if (ret)
			return ret;
		strcpy(wifname, ifname);
	} else {
		strcpy(wifname, BRIDGE_IFNAME);
	}
	log_debug("%s: Exit\n", __func__);
	return ret;
}

int get_netinfo(struct network_info *netinfo)
{
	int ret = CLSAPI_OK;
	char ifname[STR_LEN_1025] = {0};

	log_debug("%s: Enter\n", __func__);
	ret = clsapi_net_get_opmode(&netinfo->opmode);
	if (ret)
		return ret;

	if (netinfo->opmode >= _CLSAPI_NETWORK_OPMODE_MAX || netinfo->opmode < CLSAPI_NETWORK_OPMODE_ROUTER)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (netinfo->opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
		ret = clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_WAN, UCI_PARA_DEVICE, ifname);
		if (ret)
			return ret;
		strcpy(netinfo->wan_intf.ifname, ifname);
	} else {
		strcpy(netinfo->wan_intf.ifname, BRIDGE_IFNAME);
	}
	ret = get_intfinfo(netinfo->wan_intf.ifname, &netinfo->wan_intf);
	log_debug("net_info: opmode=%d, ifname=%s, ipstr=%s\n", netinfo->opmode, netinfo->wan_intf.ifname,
		netinfo->wan_intf.ipstr);
	log_debug("%s: Exit\n", __func__);
	return ret;
}

int get_opmode(enum clsapi_net_opmode *opmode)
{
	int ret = CLSAPI_OK;

	ret = clsapi_net_get_opmode(opmode);
	return ret;
}

static int netlink_request(int sock, int type)
{
	struct {
		struct nlmsghdr nh;
		struct rtgenmsg gen;
	} req;

	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	req.nh.nlmsg_type = type;
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.gen.rtgen_family = AF_INET;

	return send(sock, &req, req.nh.nlmsg_len, 0);
}

static void parse_route(struct nlmsghdr *nlh, struct interface_info *gwinfo)
{
	struct rtmsg *rt = NLMSG_DATA(nlh);
	struct rtattr *attr = RTM_RTA(rt);
	int len = RTM_PAYLOAD(nlh);
	unsigned int mask;
	char ipaddr[STR_LEN_32];
	//struct in_addr addr;
	struct in_addr gateway;
	struct in_addr maskaddr;
	unsigned int network;
	unsigned int gwip;
	unsigned int target;

	for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
		if (attr->rta_type == RTA_OIF) {
			if (rt->rtm_family == AF_INET && rt->rtm_dst_len == 0)
				if_indextoname(*(int *)RTA_DATA(attr), gwinfo->ifname);
		}
		if (attr->rta_type == RTA_GATEWAY) {
			if (rt->rtm_family == AF_INET && rt->rtm_dst_len == 0) {
				inet_ntop(AF_INET, RTA_DATA(attr), gwinfo->ipstr, INET_ADDRSTRLEN);
				memcpy(&gateway, RTA_DATA(attr), sizeof(gateway));
				memcpy(&gwinfo->ipaddr, &gateway, sizeof(struct in_addr));
			}
		}
		if (attr->rta_type == RTA_DST) {
			inet_ntop(AF_INET, RTA_DATA(attr), ipaddr, INET_ADDRSTRLEN);
			mask = (0xFFFFFFFFU << (32 - rt->rtm_dst_len)) & 0xFFFFFFFFU;
			gwip = ipstr_to_uint(gwinfo->ipstr);
			network = gwip & mask;
			target = ipstr_to_uint(ipaddr);
			if (network == target) {
				maskaddr.s_addr = htonl(mask);
				memcpy(&gwinfo->subnet, &maskaddr, sizeof(struct in_addr));
			}
		}
	}
}

static void parse_neigh(struct nlmsghdr *nlh, struct interface_info *gwinfo)
{
	struct ndmsg *ndm;
	struct rtattr *attr;
	int len;
	struct rtattr *lladdr;
	char ipstr[STR_LEN_32];

	ndm = NLMSG_DATA(nlh);
	attr = RTM_RTA(ndm);
	len = RTM_PAYLOAD(nlh);

	if (ndm->ndm_family != AF_INET)
		return;

	for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
		if (attr->rta_type == NDA_DST) {
			inet_ntop(AF_INET, RTA_DATA(attr), ipstr, INET_ADDRSTRLEN);
			if (strcmp(ipstr, gwinfo->ipstr) == 0) {
				lladdr = attr;
				while (RTA_OK(lladdr, len)) {
					if (lladdr->rta_type == NDA_LLADDR && RTA_PAYLOAD(lladdr) == 6) {
						memcpy(gwinfo->hwaddr, RTA_DATA(lladdr), ETH_ALEN);
						return;
					}
					lladdr = RTA_NEXT(lladdr, len);
				}
			}
		}
	}
}

int get_gwinfo(struct interface_info *gwinfo)
{
	int sock = -1;
	struct sockaddr_nl sa;
	char buf[NLMSG_LEN_MAX] = {0};
	int recv_len = 0;
	int len = 0;
	struct nlmsghdr *nlh;
	struct in_addr subnet;
	int ret = -1;

	log_debug("%s: Enter\n", __func__);
	sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock < 0)
		return -1;

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	ret = bind(sock, (struct sockaddr *)&sa, sizeof(sa));
	if (ret < 0) {
		log_error("Binding socket failed");
		close(sock);
		return -1;
	}

	if (netlink_request(sock, RTM_GETROUTE) < 0) {
		log_error("send netlink requset for RTM_GETROUTE failed");
		close(sock);
		return -1;
	}

	while (1) {
		recv_len = recv(sock, buf+len, sizeof(buf)-len, 0);
		if (recv_len <= 0)
			break;
		nlh = (struct nlmsghdr *)(buf + len);
		if (nlh->nlmsg_type == NLMSG_DONE)
			break;
		len += recv_len;
	}
	nlh = (struct nlmsghdr *)buf;
	for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
		if (nlh->nlmsg_type == RTM_NEWROUTE)
			parse_route(nlh, gwinfo);
	}

	if (netlink_request(sock, RTM_GETNEIGH) < 0) {
		log_debug("send netlink requset for RTM_GETNEIGH failed");
		close(sock);
		return -1;
	}

	len = 0;
	recv_len = 0;
	memset(buf, 0, NLMSG_LEN_MAX);
	while (1) {
		recv_len = recv(sock, buf+len, sizeof(buf)-len, 0);
		if (recv_len <= 0)
			break;
		nlh = (struct nlmsghdr *)(buf + len);
		if (nlh->nlmsg_type == NLMSG_DONE)
			break;
		len += recv_len;
	}
	nlh = (struct nlmsghdr *)buf;
	for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
		if (nlh->nlmsg_type == RTM_NEWNEIGH)
			parse_neigh(nlh, gwinfo);
	}

	subnet.s_addr = gwinfo->subnet;
	log_debug("Get gateway info: ifname=%s ipaddr=%s, subnet=%s hwmac="MACFMT"\n",
		gwinfo->ifname, gwinfo->ipstr, inet_ntoa(subnet), MACARG(gwinfo->hwaddr));

	close(sock);
	log_debug("%s: Exit\n", __func__);
	return 0;
}

int network_check(char *ifname, char *gwip)
{
	char buf[STR_LEN_256] = {0};
	char result[STR_LEN_256] = {0};
	FILE *pipe = NULL;

	log_debug("%s: Enter ifname: %s\n", __func__, ifname);
	snprintf(buf, sizeof(buf) - 1, "udhcpc -i %s -t 2 -T 2 -n -q | tail -n 1", ifname);
	pipe = popen(buf, "r");
	if (!pipe)
		return -1;

	fread(result, 1, sizeof(result) - 1, pipe);
	pclose(pipe);
	if (strlen(result) <= 0) {
		log_debug("udhcp check network failed\n");
		config_interface();
		return -1;
	}
	log_debug("udhcp check network result: %s", result);
	if (sscanf(result, "%*[^:]:%*[^:]:%s", gwip) == 1)
		log_debug("gateway ip: %s", gwip);

	log_debug("%s: Exit\n", __func__);
	return 0;
}

int get_bhintf(struct interface_info *gwinfo, struct interface_info *bhintf)
{
	char buf[STR_LEN_256] = {0};
	char result[STR_LEN_256] = {0};
	FILE *pipe = NULL;

	log_debug("%s: Enter\n", __func__);
	snprintf(buf, sizeof(buf) - 1, "echo lookup "MACFMT" 0 > /sys/kernel/debug/fwt/command;"
		"cat /sys/kernel/debug/fwt/command | awk -F'outdev=' '{print $2}'",
		MACARG(gwinfo->hwaddr));
	log_debug("%s: command:%s\n", __func__, buf);

	pipe = popen(buf, "r");
	if (!pipe)
		return -1;

	fread(result, 1, sizeof(result) - 1, pipe);
	pclose(pipe);
	if (strlen(result) > 0) {
		log_debug("interface query result: %s\n", result);
		trim_str(result, bhintf->ifname);
		log_debug("bhintf->ifname: %s\n", bhintf->ifname);
		get_intfinfo(bhintf->ifname, bhintf);
		if (port_polling) {
			if (strcmp(bhintf->ifname, port_list.ifname) == 0)
				get_port_list_wan_index(gwinfo, &port_list.wan_index);
			else
				port_list.wan_index = -1;
		}
	}

	log_debug("%s: Exit\n", __func__);
	return 0;
}

void get_bhtype_name(enum backhaul_type bhtype, char *bhtype_name)
{
	if (bhtype == NONE_BACKHAUL)
		strcpy(bhtype_name, "None");
	else if (bhtype == UNCONFIGURED_BACKHAUL)
		strcpy(bhtype_name, "Unconfigured");
	else if (bhtype == WIFI_BACKHAUL)
		strcpy(bhtype_name, "WiFi");
	else if (bhtype == ETHERNET_BACKHAUL)
		strcpy(bhtype_name, "Ethernet");
	else
		strcpy(bhtype_name, "None");
}

void get_bhtype(enum backhaul_type *bhtype, char *bhtype_name)
{
	if (strcmp(bhtype_name, "None") == 0)
		*bhtype = NONE_BACKHAUL;
	else if (strcmp(bhtype_name, "Unconfigured") == 0)
		*bhtype = UNCONFIGURED_BACKHAUL;
	else if (strcmp(bhtype_name, "WiFi") == 0)
		*bhtype = WIFI_BACKHAUL;
	else if (strcmp(bhtype_name, "Ethernet") == 0)
		*bhtype = ETHERNET_BACKHAUL;
	else
		*bhtype = NONE_BACKHAUL;
}

int get_bhinfo(struct backhaul_info *bhinfo)
{
	int ret = CLSAPI_OK;
	enum clsapi_net_opmode opmode;
	char backhaul[STR_LEN_1025] = {0};

	log_debug("%s: Enter\n", __func__);
	ret = clsapi_net_get_opmode(&opmode);
	if (ret)
		return ret;

	if (opmode >= _CLSAPI_NETWORK_OPMODE_MAX || opmode < CLSAPI_NETWORK_OPMODE_ROUTER)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (opmode == CLSAPI_NETWORK_OPMODE_ROUTER)
		bhinfo->opmode = ROUTE_MODE;
	else
		bhinfo->opmode = UNDETERMINED_MODE;

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_BACKHAUL, UCI_PARA_BACKHAUL, backhaul);
	if (ret) {
		bhinfo->last_backhaul = NONE_BACKHAUL;
		return ret;
	}
	get_bhtype(&bhinfo->last_backhaul, backhaul);
	bhinfo->cur_backhaul = UNCONFIGURED_BACKHAUL;
	log_debug("%s: Exit\n", __func__);
	return ret;
}

int set_opmode_by_bhtype(enum backhaul_type bhtype)
{
	int ret = CLSAPI_OK;
	enum clsapi_net_opmode opmode;

	if (bhtype == ETHERNET_BACKHAUL)
		opmode = CLSAPI_NETWORK_OPMODE_BRIDGE;
	else if (bhtype == WIFI_BACKHAUL)
		opmode = CLSAPI_NETWORK_OPMODE_REPEATER_5G;
	else
		return -1;

	ret = clsapi_net_set_opmode(opmode);
	return ret;
}

int set_netmanager_conf(char *section, char *param, char *value)
{
	int ret = CLSAPI_OK;

	ret = clsapi_base_set_apply_conf_param(UCI_CFG_CLSNETMANAGER, section, param, value);
	return ret;
}

int get_sta_ifname(char *ifname)
{
	char value[STR_LEN_1025] = {0};
	int radio_idx = 0;
	int ret = CLSAPI_OK;

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSOPMODE, UCI_SECTION_GLOBALS, UCI_PARA_STA_PHYNAME, value);
	if (ret) {
		strcpy(ifname, STA_INTF_NAME_DEFAULT);
	} else {
		if (sscanf(value, "radio%d", &radio_idx) < 0)
			radio_idx = 1;
		sprintf(ifname, "wlan%d-sta", radio_idx);
	}
	return CLSAPI_OK;
}

int set_wifi_sta_intf(char *ifname, int enable)
{
	int ret = CLSAPI_OK;

	ret = clsapi_wifi_enable_bss(ifname, enable);
	return ret;
}

int is_wan_intf(char *ifname)
{
	if (strncmp(ifname, ETH_INTF_NAME, ETH_INTF_NAME_LEN) == 0
		|| strcmp(ifname, BRIDGE_IFNAME) == 0)
		return 1;
	else
		return 0;
}

int is_sta_connected(const char *ifname)
{
	char cmd[STR_LEN_64] = {0};
	char line[STR_LEN_64] = {0};
	FILE *pipe = NULL;
	int connected = 0;

	snprintf(cmd, sizeof(cmd), "wpa_cli -i %s status", ifname);
	pipe = popen(cmd, "r");
	if (!pipe)
		return -1;

	while (fgets(line, sizeof(line), pipe)) {
		if (strstr(line, "wpa_state=COMPLETED") != NULL) {
			connected = 1;
			break;
		}
	}
	pclose(pipe);
	return connected;
}

int is_interface_connected(const char *ifname)
{
	char path[STR_LEN_256];
	int fd;
	char val;

	snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", ifname);
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		/* isn't existed or isn't enabled*/
		log_error("Failed to open carrier file: %s", path);
		return -1;
	}
	if (read(fd, &val, 1) != 1) {
		log_error("Failed to read carrier status: %s", path);
		close(fd);
		return -1;
	}
	close(fd);
	return (val == '1') ? 1 : 0;
}

int bridge_link_check(void)
{
	char path[STR_LEN_256];
	DIR *dir;
	struct dirent *entry;
	char *ifname;
	int status = 0;

	snprintf(path, sizeof(path), "/sys/class/net/%s/brif/", BRIDGE_IFNAME);
	dir = opendir(path);
	if (!dir)
		return status;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_LNK)
			continue;
		ifname = entry->d_name;
		if (strncmp(ifname, ETH_INTF_NAME, ETH_INTF_NAME_LEN) == 0) {
			status = is_interface_connected(ifname);
			if (status == 1) {
				log_debug("Ethernet link exist");
				break;
			}
		}
	}
	closedir(dir);
	return status;
}

void set_bridge_loop_detect_state(int state)
{
	FILE *fp;
	char path[STR_LEN_64] = {0};

	snprintf(path, sizeof(path), "/sys/class/net/%s/bridge/loop_detect_state", BRIDGE_IFNAME);
	fp = fopen(path, "w");
	if (!fp)
		return;

	fprintf(fp, "%d", state);
	fflush(fp);
	fclose(fp);
}

int set_intf_state(const char *ifname, int state)
{
	char cmd[STR_LEN_64] = {0};
	FILE *pipe = NULL;

	if (state == 0)
		snprintf(cmd, sizeof(cmd), "ifconfig %s down", ifname);
	else
		snprintf(cmd, sizeof(cmd), "ifconfig %s up", ifname);

	pipe = popen(cmd, "r");
	if (!pipe)
		return -1;
	return 0;
}

int get_backhaul_ifname(const char *phyname, char *bh_ifname, enum backhaul_type bhtype)
{
	struct mesh_bss_info info;
	enum clsapi_net_opmode opmode;

	if (!phyname || !bh_ifname)
		return -1;

	if (get_opmode(&opmode) < 0)
		return -1;

	memset(bh_ifname, 0, STR_LEN_32);

	if (clsapi_wifi_get_mesh_bss_ifname(phyname, &info) < 0)
		return -1;

	switch (bhtype) {
	case UNCONFIGURED_BACKHAUL:
		return -1;
	case NONE_BACKHAUL:
		if (opmode == CLSAPI_NETWORK_OPMODE_ROUTER)
			strcpy(bh_ifname, info.fh_ifname);
		else
			strcpy(bh_ifname, info.bh_sta_ifname);
		break;
	case WIFI_BACKHAUL:
	case ETHERNET_BACKHAUL:
		strcpy(bh_ifname, info.fh_ifname);
		break;
	}

	return 0;
}

bool is_mesh_version(void)
{
	const char *keyword = "Mesh";
	struct clsapi_board_info board_info;

	memset(&board_info, 0, sizeof(board_info));

	if (clsapi_sys_get_board_info(&board_info) < 0)
		return -1;

	if (strstr(board_info.release_version, keyword))
		return true;

	return false;
}

int exec_cmd(char *command)
{
	FILE *pipe = NULL;

	pipe = popen(command, "r");
	if (!pipe)
		return -1;

	return 0;
}
int check_file_size(char *fname)
{
	struct stat st;

	if (stat(fname, &st) != 0)
		return 0;
	return (st.st_size == 0) ? 0 : 1;
}

int check_service_running(char *srv_name)
{
	char buf[STR_LEN_256] = {0};
	int is_run = 0;

	log_debug("%s: Enter\n", __func__);
	snprintf(buf, sizeof(buf) - 1, "pgrep %s > %s", srv_name, UDHCP_PID_FILE);
	system(buf);

	if (check_file_size(UDHCP_PID_FILE))
		is_run = 1;
	else
		is_run = 0;
	log_debug("%s: Exit\n", __func__);
	return is_run;
}

void calculate_network_prefix(char *ip, char *mask, char *prefix)
{
	struct in_addr ip_addr, mask_addr, net_addr;
	int i = 0;
	int len = 0;

	if (inet_pton(AF_INET, ip, &ip_addr) != 1) {
		log_error("Invalid ip: %s", ip);
		return;
	}
	if (inet_pton(AF_INET, mask, &mask_addr) != 1) {
		log_error("Invalid subnet mask: %s", ip);
		return;
	}
	net_addr.s_addr = ip_addr.s_addr & mask_addr.s_addr;
	inet_ntop(AF_INET, &net_addr, prefix, INET_ADDRSTRLEN);
	len = strlen(prefix);
	for (i = len - 1; i >= 0; i--) {
		if (prefix[i] == '.') {
			prefix[i] = '\0';
			break;
		}
	}
}

int create_udhcp_conf(void)
{
	FILE *fp = NULL;
	char value[STR_LEN_1025] = {0};
	int ret = CLSAPI_OK;
	char line[STR_LEN_128] = {0};
	char content[STR_LEN_512] = {0};
	char ipaddr[STR_LEN_32] = {0};
	char mask[STR_LEN_32] = {0};
	char prefix[INET_ADDRSTRLEN] = {0};

	fp = fopen(UDHCP_CONF_FILE, "w");
	if (fp == NULL) {
		log_error("Open file %s failed", UDHCP_CONF_FILE);
		return -1;
	}

	sprintf(line, "interface			%s\n", BRIDGE_IFNAME);
	strcpy(content, line);

	ret = clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_LAN, UCI_PARA_IPADDR, value);
	if (ret) {
		fclose(fp);
		return ret;
	}
	strcpy(ipaddr, value);
	ret = clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_LAN, UCI_PARA_NETMASK, value);
	if (ret) {
		fclose(fp);
		return ret;
	}
	strcpy(mask, value);
	calculate_network_prefix(ipaddr, mask, prefix);

	sprintf(line, "start			%s.100\n", prefix);
	strcat(content, line);
	sprintf(line, "end			%s.249\n", prefix);
	strcat(content, line);
	sprintf(line, "max_leases		100\n");
	strcat(content, line);
	sprintf(line, "lease_file		%s\n", UDHCP_LEASE_FILE);
	strcat(content, line);
	sprintf(line, "option		lease		120\n");
	strcat(content, line);
	sprintf(line, "option		subnet		%s\n", mask);
	strcat(content, line);
	sprintf(line, "option		router		%s\n", ipaddr);
	strcat(content, line);
	sprintf(line, "option		dns		%s\n", ipaddr);
	strcat(content, line);
	fprintf(fp, "%s", content);
	fclose(fp);
	return ret;
}

int config_interface(void)
{
	int ret = CLSAPI_OK;
	char cmd[STR_LEN_64] = {0};
	char value[STR_LEN_1025] = {0};
	char ipaddr[STR_LEN_32] = {0};
	enum clsapi_net_opmode opmode;

	ret = clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_LAN, UCI_PARA_IPADDR, value);
	if (ret)
		return ret;
	strcpy(ipaddr, value);

	if (get_opmode(&opmode) < 0)
		return -1;

	if (opmode == CLSAPI_NETWORK_OPMODE_ROUTER)
		sprintf(cmd, "ifconfig %s %s", BRIDGE_IFNAME, ipaddr);
	else
		sprintf(cmd, "ifconfig %s %s", BRIDGE_IFNAME, DEFAULT_BRIDGE_IPADDR);
	exec_cmd(cmd);
	return 0;
}

void flush_ip_addr(const char *ifname)
{
	char cmd[STR_LEN_64] = {0};

	sprintf(cmd, "ip addr flush dev %s", ifname);
	exec_cmd(cmd);
}

int set_dhcp_service(int enable)
{
/*Don't start dhcp server for customer*/
#if 0
	int ret = 0;
	char cmd[STR_LEN_64] = {0};
	int is_run = 0;

	log_debug("%s: Enter\n", __func__);
	if (enable) {
		is_run = check_service_running(UDHCP_SERVICE_DAEMON);
		if (is_run)
			return ret;
		if (create_udhcp_conf())
			return -1;
		if (config_interface())
			return -1;
		sprintf(cmd, "%s -S %s", UDHCP_SERVICE_DAEMON, UDHCP_CONF_FILE);
		exec_cmd(cmd);
	} else {
		sprintf(cmd, "killall -q %s", UDHCP_SERVICE_DAEMON);
		exec_cmd(cmd);
	}
	log_debug("%s: Exit\n", __func__);
	return ret;
#else
	if (enable)
		config_interface();
	return 0;
#endif
}

int get_mesh_role(char *role_name)
{
	int ret = CLSAPI_OK;
	char value[STR_LEN_1025] = {0};

	ret = clsapi_base_get_conf_param(UCI_CFG_CLS_MESH, UCI_SECTION_DEFAULT, UCI_PARA_MODE, value);
	if (ret == CLSAPI_OK)
		strcpy(role_name, value);
	return ret;
}

int set_mesh_role(char *role_name)
{
	int ret = CLSAPI_OK;
	char value[STR_LEN_1025] = {0};

	strcpy(value, role_name);
	ret = clsapi_base_set_apply_conf_param(UCI_CFG_CLS_MESH, UCI_SECTION_DEFAULT, UCI_PARA_MODE, value);
	return ret;
}

int get_onboarding_done(int *status)
{
	int ret = CLSAPI_OK;
	char value[STR_LEN_1025] = {0};

	ret = clsapi_base_get_conf_param(UCI_CFG_CLSNETMANAGER, UCI_SECTION_CONFIG, UCI_PARA_ONBOARDING_DONE, value);
	if (ret == CLSAPI_OK)
		*status = atoi(value);
	else
		*status = 0;
	return ret;
}

void set_http_server(int enable)
{
	int is_run = 0;

	is_run = check_service_running(HTTP_SERVICE_DAEMON);
	log_debug("http server is running: %d", is_run);
	if (enable == 0 && is_run == 1)
		exec_cmd("/etc/init.d/uhttpd stop");
	else if (enable == 1 && is_run == 0)
		exec_cmd("/etc/init.d/uhttpd start");
}

void restart_clsmeshd(void)
{
	if (!netmng_conf.is_mesh)
		return;
	exec_cmd("/etc/init.d/cls-mesh restart");
}

int get_interface_index(char *ifname, int *ifindex)
{
	struct ifreq ifr;
	int sock = -1;

	log_debug("%s: Enter\n", __func__);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
		close(sock);
		return -1;
	}
	*ifindex = ifr.ifr_ifindex;
	close(sock);
	return 0;
}

void get_port_link_state(int index, int *state)
{
	char buf[STR_LEN_256] = {0};
	char result[STR_LEN_128] = {0};
	char state_str[STR_LEN_128] = {0};
	FILE *pipe = NULL;

	snprintf(buf, sizeof(buf) - 1,
		"esw_cli yt_port_link_status_get 0 %d | awk -F'pLinkStatus:' '{print $2}'", index);

	pipe = popen(buf, "r");
	if (!pipe)
		return;

	fread(result, 1, sizeof(result) - 1, pipe);
	pclose(pipe);
	if (strlen(result) > 0) {
		trim_str(result, state_str);
		if (strcmp(state_str, PORT_LINK_UP) == 0)
			*state = 1;
		else
			*state = 0;
	}
}

int get_port_list_wan_index(struct interface_info *gwinfo, int *wan_index)
{
	char buf[STR_LEN_256] = {0};
	char result[STR_LEN_128] = {0};
	char port_str[STR_LEN_128] = {0};
	FILE *pipe = NULL;

	log_debug("%s: Enter\n", __func__);
	snprintf(buf, sizeof(buf) - 1, "esw_cli yt_l2_fdb_uc_withMacAndVid_get 0 1 addr "MACFMT
		"| awk -F'port:' '{print $2}'", MACARG(gwinfo->hwaddr));
	log_debug("%s: command:%s\n", __func__, buf);

	pipe = popen(buf, "r");
	if (!pipe)
		return -1;

	fread(result, 1, sizeof(result) - 1, pipe);
	pclose(pipe);
	if (strlen(result) > 0) {
		trim_str(result, port_str);
		*wan_index = atoi(port_str);
		log_debug("wan port index: %d\n", *wan_index);
	}

	log_debug("%s: Exit\n", __func__);
	return 0;

}

