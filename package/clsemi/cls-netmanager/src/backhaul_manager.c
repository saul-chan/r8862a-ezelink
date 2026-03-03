/*
 *  Copyright (c) 2021-2025, Clourney Semiconductor. All rights reserved.
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
#include <stdint.h>
#include <errno.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <libubox/uloop.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include "common.h"
#include "log.h"
#include "ubus.h"
#include "backhaul_manager.h"

struct uloop_fd arp_fd;
struct uloop_fd nl_rt_fd;
struct uloop_fd nl_uevt_fd;

struct interface_info wifi_bh_intf;
struct interface_info gw_info;
struct backhaul_info bh_info;
struct backhaul_conf bh_conf;
struct network_info net_info;
struct port_link_info port_list;

int arping_gw_count = 0;
int netconn_check_count = 0;
int sta_state = 0;

struct dlist_head timer_list;
struct uloop_timer arp_timer;
struct uloop_timeout netconn_check_timer;
struct uloop_timeout port_polling_timer;

#define ETHERTYPE_ARP 0x0806

enum NL_UEVT_ACTION {
	NL_UEVT_ACTION_ADD = 10,
	NL_UEVT_ACTION_BIND,
	NL_UEVT_ACTION_REMOVE,
	NL_UEVT_ACTION_PRESSED,
	NL_UEVT_ACTION_RELEASED
};

struct nl_uevt_info {
	char button[STR_LEN_32];
	char subsystem[STR_LEN_32];
	enum NL_UEVT_ACTION action;
};

/* ARP Header struct */
struct arp_header {
	unsigned short htype;     /* Hardware type */
	unsigned short ptype;     /* Protocol type */
	unsigned char hlen;       /* Hardware address length */
	unsigned char plen;       /* Protocol address length */
	unsigned short oper;      /* Operation code (ARP request is 1)*/
	unsigned char srcmac[ETH_ALEN];     /* Source mac */
	unsigned int srcip;       /* Source IP */
	unsigned char dstmac[ETH_ALEN];     /* Destination mac */
	unsigned int dstip;       /* Destination IP */
} __packed;

#define get_unaligned(ptr)				\
({										\
	struct __packed {	\
		typeof(*(ptr)) __v;				\
	} *__p = (void *) (ptr);			\
	__p->__v;							\
})

#define put_unaligned(val, ptr)			\
do {									\
	struct __packed {	\
		typeof(*(ptr)) __v;				\
	} *__p = (void *) (ptr);			\
	__p->__v = (val);					\
} while (0)

static void parse_nl_uevt_message(const char *buf, struct nl_uevt_info *info, int len)
{
	int i = 0, rem;
	void *index;
	static struct blob_buf b;
	struct blob_attr *table;
	struct blob_attr *cur;

	blob_buf_init(&b, 0);
	index = blobmsg_open_table(&b, NULL);
	while (i < len) {
		int l = strlen(buf + i) + 1;
		char *e = strstr(&buf[i], "=");

		if (e) {
			*e = '\0';
			if (e[1] == '\0')
				break;
			blobmsg_add_string(&b, &buf[i], &e[1]);
		}
		i += l;
	}

	blobmsg_close_table(&b, index);
	table = blob_data(b.head);

	blobmsg_for_each_attr(cur, table, rem) {
		const char *name = blobmsg_name(cur);
		char *val;

		if (!strcmp(name, "ACTION")) {
			val = blobmsg_get_string(cur);
			if (!strcmp(val, "pressed"))
				info->action = NL_UEVT_ACTION_PRESSED;
			else if (!strcmp(val, "released"))
				info->action = NL_UEVT_ACTION_RELEASED;
		} else if (!strcmp(name, "BUTTON")) {
			val = blobmsg_get_string(cur);
			strncpy(info->button, val, STR_LEN_32 - 1);
			info->button[STR_LEN_32 - 1] = '\0';
		} else if (!strcmp(name, "SUBSYSTEM")) {
			val = blobmsg_get_string(cur);
			strncpy(info->subsystem, val, STR_LEN_32 - 1);
			info->subsystem[STR_LEN_32 - 1] = '\0';
		} else {
			//Todo...
		}
	}
}

int send_backhaul_event(int last_backhaul, int cur_backhaul)
{
	char type[STR_LEN_16] = {0};
	char *status;
	int ret = -1;

	log_debug("%s: Enter\n", __func__);
	if (cur_backhaul == NONE_BACKHAUL || cur_backhaul == UNCONFIGURED_BACKHAUL) {
		get_bhtype_name(bh_info.last_backhaul, type);
		status = BACKHAUL_STATUS_DISCONNECT;
	} else {
		get_bhtype_name(bh_info.cur_backhaul, type);
		status = BACKHAUL_STATUS_CONNECT;
	}
	ret = ubus_backhaul_notify(type, status);
	if (ret)
		log_debug("Send backhaul ubus notification failed\n");
	log_debug("%s: Exit\n", __func__);
	return ret;
}

void set_backhaul(enum backhaul_type bhtype, struct interface_info *bhintf)
{
	char new_bh[STR_LEN_16] = {0};
	char cur_bh[STR_LEN_16] = {0};

	if (bh_info.cur_backhaul == bhtype
		&& (!bhintf || strcmp(bhintf->ifname, bh_info.cur_bhintf.ifname) == 0)) {
		if (bh_info.opmode != ROUTE_MODE && bhtype == NONE_BACKHAUL) {
			if (netmng_conf.is_mesh)
				set_dhcp_service(1);
		}
		return;
	}

	get_bhtype_name(bh_info.cur_backhaul, cur_bh);
	get_bhtype_name(bhtype, new_bh);
	log_debug("Change backhual from %s to %s", cur_bh, new_bh);
	bh_info.last_backhaul = bh_info.cur_backhaul;
	memcpy(&bh_info.last_bhintf, &bh_info.cur_bhintf, sizeof(struct interface_info));
	bh_info.cur_backhaul = bhtype;
	if (!bhintf)
		memset(&bh_info.cur_bhintf, 0, sizeof(struct interface_info));
	else
		memcpy(&bh_info.cur_bhintf, bhintf, sizeof(struct interface_info));

	arping_gateway_stop();

	if (bhtype == ETHERNET_BACKHAUL || bhtype == WIFI_BACKHAUL) {
		cancel_uloop_timer(&netconn_check_timer);
		netconn_check_count = 0;
		get_intfinfo(net_info.wan_intf.ifname, &net_info.wan_intf);
		arp_socket_init(&net_info.wan_intf);
		arp_timer.state = ARPING_GW_INIT;
		register_uloop_timer(&arp_timer.timer, bh_conf.arping_gw_interval, arping_gateway);
	}

	set_netmanager_conf(UCI_SECTION_BACKHAUL, UCI_PARA_BACKHAUL, new_bh);
	send_backhaul_event(bh_info.last_backhaul, bh_info.cur_backhaul);

	if (bh_info.opmode == ROUTE_MODE)
		return;

	if (bhtype == NONE_BACKHAUL) {
		if (netmng_conf.is_mesh)
			set_dhcp_service(1);
	} else if (bhtype == ETHERNET_BACKHAUL) {
		set_opmode_by_bhtype(bhtype);
		net_info.opmode = CLSAPI_NETWORK_OPMODE_BRIDGE;
		sta_state = 0;
		if (netmng_conf.is_mesh) {
			set_dhcp_service(0);
			send_controller_weight_notify(CONTROLLER_WEIGHT_ETH_DEFAULT);
		}
	} else if (bhtype == WIFI_BACKHAUL) {
		sta_state = 0;
		set_opmode_by_bhtype(bhtype);
		net_info.opmode = CLSAPI_NETWORK_OPMODE_REPEATER_5G;
		if (netmng_conf.is_mesh) {
			set_dhcp_service(0);
			send_controller_weight_notify(CONTROLLER_WEIGHT_WIFI_DEFAULT);
		}
	}
}

void backhaul_change(void)
{
	int ret;
	int i = 0;
	int try_count = 0;
	int intflen = 0;
	struct interface_info bhintf;
	char gwip[STR_LEN_32] = {0};

	log_debug("%s: Enter\n", __func__);
	ret = network_check(net_info.wan_intf.ifname, gwip);
	if (ret < 0) {
		set_backhaul(NONE_BACKHAUL, NULL);
		if (bh_info.last_backhaul == ETHERNET_BACKHAUL) {
			sta_state = 0;
			/*try WiFi backhaul*/
			set_opmode_by_bhtype(WIFI_BACKHAUL);
		}
	} else {
		update_gateway_arp_table(gwip);
		try_count = bh_conf.netconn_check_count * bh_conf.netconn_check_interval;
		for (i = 0; i < try_count; i++) {
			get_gwinfo(&gw_info);
			memset(&bhintf, 0, sizeof(bhintf));
			get_bhintf(&gw_info, &bhintf);
			intflen = strlen(bhintf.ifname);
			if (intflen == 0) {
				sleep(1);
			} else {
				if (strcmp(bhintf.ifname, wifi_bh_intf.ifname) == 0)
					set_backhaul(WIFI_BACKHAUL, &bhintf);
				else if (strncmp(bhintf.ifname, ETH_INTF_NAME, intflen-1) == 0)
					set_backhaul(ETHERNET_BACKHAUL, &bhintf);
				else
					set_backhaul(NONE_BACKHAUL, NULL);
				break;
			}
		}
		if (intflen == 0)
			set_backhaul(NONE_BACKHAUL, NULL);
	}
	log_debug("%s: Exit\n", __func__);
}

void gateway_recovery_handler(void)
{
	log_debug("%s: Enter\n", __func__);
	if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
		set_backhaul(ETHERNET_BACKHAUL, NULL);
		return;
	}
	backhaul_change();
	if (bh_info.cur_backhaul == NONE_BACKHAUL) {
		cancel_uloop_timer(&arp_timer.timer);
		arp_timer.state = ARPING_GW_INIT;
		register_uloop_timer(&arp_timer.timer, bh_conf.arping_gw_interval, arping_gateway);
	}
	log_debug("%s: Exit\n", __func__);
}

void gateway_unreachable_handler(void)
{
	log_debug("%s: Enter\n", __func__);
	if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
		set_backhaul(NONE_BACKHAUL, NULL);
		cancel_uloop_timer(&arp_timer.timer);
		arp_timer.state = ARPING_GW_INIT;
		register_uloop_timer(&arp_timer.timer, bh_conf.arping_gw_interval, arping_gateway);
		return;
	}
	backhaul_change();
	if (bh_info.cur_backhaul == NONE_BACKHAUL) {
		cancel_uloop_timer(&arp_timer.timer);
		arp_timer.state = ARPING_GW_INIT;
		register_uloop_timer(&arp_timer.timer, bh_conf.arping_gw_interval, arping_gateway);
	}
	log_debug("%s: Exit\n", __func__);
}

void send_arp_request_frame(char *lhwaddr, char *srcip, char *dstip, int ifindex, int sockfd)
{
	unsigned char frame[STR_LEN_128];
	struct ether_header *eth_hdr = (struct ether_header *)frame;
	struct arp_header *arp_hdr;
	struct sockaddr_ll dest;

	log_debug("%s: Enter\n", __func__);
	memset(eth_hdr->ether_dhost, 0xFF, ETH_ALEN);
	memcpy(eth_hdr->ether_shost, lhwaddr, ETH_ALEN);
	eth_hdr->ether_type = htons(ETHERTYPE_ARP);

	arp_hdr = (struct arp_header *)(frame + sizeof(struct ether_header));
	arp_hdr->htype = htons(ARPHRD_ETHER);
	arp_hdr->ptype = htons(ETH_P_IP);
	arp_hdr->hlen = ETH_ALEN;
	arp_hdr->plen = 4;
	arp_hdr->oper = htons(ARPOP_REQUEST);

	memcpy(arp_hdr->srcmac, lhwaddr, ETH_ALEN);
	arp_hdr->srcip = inet_addr(srcip);
	memset(arp_hdr->dstmac, 0, ETH_ALEN);
	arp_hdr->dstip = inet_addr(dstip);

	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_ifindex = ifindex;
	dest.sll_protocol = htons(ETH_P_ARP);
	dest.sll_halen = ETH_ALEN;
	memset(dest.sll_addr, 0xff, ETH_ALEN);

	/* Send ARP request */
	if (sendto(sockfd, frame, sizeof(struct ether_header) + sizeof(struct arp_header),
		0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
		log_debug("Send ARP request error\n");
	log_dump(frame, sizeof(struct ether_header) + sizeof(struct arp_header));
	log_debug("%s: Exit\n", __func__);
}

int send_arp_request(char *target_ip)
{
	log_debug("%s: Enter\n", __func__);
	if (strlen(net_info.wan_intf.ipstr) == 0) {
		if (get_intfip(net_info.wan_intf.ifname, &net_info.wan_intf) < 0) {
			log_error("%s: cannot get WAN interface info\n", __func__);
			return -1;
		}
	}
	if (!target_ip)
		target_ip = net_info.wan_intf.ipstr;

	send_arp_request_frame(net_info.wan_intf.hwaddr, net_info.wan_intf.ipstr,
		target_ip, net_info.wan_intf.ifindex, arp_fd.fd);
	arp_timer.state = ARPING_GW_SENT;

	log_debug("%s: Exit\n", __func__);
	return 0;
}

int is_same_subnet(unsigned int ip, unsigned int netmask, unsigned int host)
{
	unsigned int minhost, maxhost;

	log_debug("%s: Enter\n", __func__);
	minhost = ntohl(ip & netmask) + 1;
	maxhost = ntohl((ip & netmask) | ~netmask) - 1;

	host = ntohl(host);
	log_debug("%s: Exit\n", __func__);
	return (host >= minhost && host <= maxhost);
}

int is_gateway_reachable(void)
{
	log_debug("%s: Enter\n", __func__);
	if (strlen(net_info.wan_intf.ipstr) == 0) {
		if (get_intfip(net_info.wan_intf.ifname, &net_info.wan_intf) < 0) {
			log_error("%s: cannot get WAN interface info\n", __func__);
			return -1;
		}
	}
	if (!is_same_subnet(gw_info.ipaddr, gw_info.subnet, net_info.wan_intf.ipaddr)) {
		log_error("%s: gateway isn't in same subnet\n", __func__);
		return -1;
	}
	if (!is_same_subnet(net_info.wan_intf.ipaddr, net_info.wan_intf.subnet, gw_info.ipaddr)) {
		log_error("%s: gateway isn't in same subnet\n", __func__);
		return -1;
	}
	log_debug("%s: Exit\n", __func__);
	return 0;
}

void arping_gateway_timeout(struct uloop_timeout *t)
{
	log_debug("%s: Enter\n", __func__);

	if (arping_gw_count++ < bh_conf.arping_gw_count) {
		send_arp_request(gw_info.ipstr);
		cancel_uloop_timer(&arp_timer.timeout);
		register_uloop_timer(&arp_timer.timeout, bh_conf.arping_gw_timeout, arping_gateway_timeout);
		log_debug("%s: Exit\n", __func__);
		return;
	}

	log_debug("Gateway is unreachable\n");
	arp_timer.state = ARPING_GW_INIT;
	gateway_unreachable_handler();
	log_debug("%s: Exit\n", __func__);
}

void arping_gateway(struct uloop_timeout *t)
{
	arping_gw_count = 0;

	log_debug("%s: Enter\n", __func__);
	if (is_gateway_reachable() == 0) {
		send_arp_request(gw_info.ipstr);
		register_uloop_timer(&arp_timer.timeout, bh_conf.arping_gw_timeout, arping_gateway_timeout);
	}
	log_debug("%s: Exit\n", __func__);
}

void arping_gateway_stop(void)
{
	log_debug("%s: Enter\n", __func__);
	cancel_uloop_timer(&arp_timer.timer);
	cancel_uloop_timer(&arp_timer.timeout);
	if (arp_fd.fd > 0) {
		uloop_fd_delete(&arp_fd);
		close(arp_fd.fd);
		arp_fd.fd = -1;
	}
	log_debug("%s: Exit\n", __func__);
}

void arp_handler(struct uloop_fd *u, unsigned int events)
{
	char buf[STR_LEN_128] = {0};
	struct sockaddr_ll sll;
	socklen_t sll_len = sizeof(sll);
	struct ether_header *eth_hdr = (struct ether_header *)buf;
	struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));
	int recv_len;

	if (arp_fd.fd < 0)
		return;
	recv_len = recvfrom(arp_fd.fd, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *)&sll, &sll_len);
	if (recv_len < 0) {
		log_error("Receive from arp socket failed: recv_len=%d\n", __func__, recv_len);
		return;
	}
	if (arp_timer.state != ARPING_GW_SENT)
		return;

	log_debug("%s: Receive from arp socket\n", __func__);
	if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP && ntohs(arp_hdr->oper) == ARPOP_REPLY) {
		log_debug("%s: Receive ARP reply\n", __func__);
		if (arp_hdr->srcip == gw_info.ipaddr && arp_hdr->dstip != 0) {
			log_dump((unsigned char *)buf, recv_len);
			memcpy(gw_info.hwaddr, arp_hdr->srcmac, ETH_ALEN);
			log_debug("gateway MAC:"MACFMT, MACARG(gw_info.hwaddr));
			cancel_uloop_timer(&arp_timer.timeout);
			arp_timer.state = ARPING_GW_RECEIVED;
			register_uloop_timer(&arp_timer.timer, bh_conf.arping_gw_interval, arping_gateway);
			if (bh_info.cur_backhaul == NONE_BACKHAUL)
				gateway_recovery_handler();
		}
	}
	log_debug("%s: Exit\n", __func__);
}

int arp_socket_create(struct interface_info *intf_info)
{
	struct sockaddr_ll local;
	int sock = -1;

	log_debug("%s: Enter\n", __func__);
	log_debug("arp interface info: ifname=%s, ifindex=%d, ip=%s, mac="MACFMT,
		intf_info->ifname, intf_info->ifindex, intf_info->ipstr, MACARG(intf_info->hwaddr));
	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (sock < 0)
		return -errno;
	memset(&local, 0, sizeof(local));
	local.sll_family = AF_PACKET;
	local.sll_ifindex = intf_info->ifindex;
	local.sll_protocol = htons(ETH_P_ARP);
	local.sll_halen = ETH_ALEN;
	if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
		close(sock);
		return -1;
	}
	log_debug("%s: Exit\n", __func__);
	return sock;
}

int arp_socket_init(struct interface_info *intf_info)
{
	int ret = -1;

	log_debug("%s: Enter\n", __func__);
	if (arp_fd.fd > 0) {
		uloop_fd_delete(&arp_fd);
		close(arp_fd.fd);
		log_debug("arp socket already created");
	}
	arp_fd.fd = arp_socket_create(intf_info);
	if (arp_fd.fd < 0) {
		log_error("Cannot open arp socket: %s\n", strerror(errno));
		return ret;
	}
	arp_fd.cb = arp_handler;
	uloop_fd_add(&arp_fd, ULOOP_READ);
	log_debug("%s: Exit\n", __func__);
	return 0;
}

void update_gateway_arp_table(char *target_ip)
{
	unsigned char buf[STR_LEN_128] = {0};
	int raw_sock = -1;
	int len = 0;
	struct timeval tv;
	struct ether_header *eth_hdr = NULL;
	struct arp_header *arp_hdr = NULL;

	log_debug("%s: Enter\n", __func__);
	raw_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (raw_sock < 0) {
		log_error("%s::Cannot open raw socket for APR: %s\n", __func__, strerror(errno));
		return;
	}
	tv.tv_sec = bh_conf.arping_gw_timeout;
	tv.tv_usec = 0;
	if (setsockopt(raw_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		log_error("%s: Set raw socket timeout option error: %s\n", __func__, strerror(errno));
		close(raw_sock);
		return;
	}

	if (IS_ZERO_MAC(net_info.wan_intf.hwaddr))
		get_intfinfo(net_info.wan_intf.ifname, &net_info.wan_intf);
	send_arp_request_frame(net_info.wan_intf.hwaddr, net_info.wan_intf.ipstr,
		target_ip, net_info.wan_intf.ifindex, raw_sock);

	while (1) {
		len = recvfrom(raw_sock, buf, sizeof(buf), 0, NULL, NULL);
		if (len < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				log_debug("ARP response timeout after %d seconds\n", bh_conf.arping_gw_timeout);
				break;
			}
			log_debug("Receive ARP response eroor\n");
			break;
		}
		if (len >= (ETH_HLEN + sizeof(struct ether_arp))) {
			eth_hdr = (struct ether_header *)buf;
			arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));
			if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP && ntohs(arp_hdr->oper) == ARPOP_REPLY) {
				if (arp_hdr->srcip == inet_addr(target_ip) && arp_hdr->dstip != 0) {
					log_debug("Receive arp repsone");
					log_dump((unsigned char *)buf, len);
					break;
				}
			}
		}
	}
	close(raw_sock);
	log_debug("%s: Exit\n", __func__);
}

int netlink_rt_socket_create(void)
{
	struct sockaddr_nl local;
	int sock;

	log_debug("%s: Enter\n", __func__);
	sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock < 0)
		return -errno;

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;

	if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
		close(sock);
		return -errno;
	}
	log_debug("%s: Exit\n", __func__);
	return sock;
}

int netlink_uevt_socket_create(void)
{
	int sock;
	struct sockaddr_nl local;

	log_debug("%s: Enter\n", __func__);
	sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock < 0)
		return -errno;

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;

	if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
		close(sock);
		return -errno;
	}
	log_debug("%s: Exit\n", __func__);
	return sock;
}

void backhaul_netconn_check(struct uloop_timeout *t)
{
	struct interface_info bhintf;
	char gwip[STR_LEN_32] = {0};
	int intflen = 0;
	int ret = -1;
	int i = 0;
	int try_count = 0;

	log_debug("%s: Enter\n", __func__);
	ret = network_check(net_info.wan_intf.ifname, gwip);
	if (ret < 0) {
		set_backhaul(NONE_BACKHAUL, NULL);
		if (netconn_check_count < bh_conf.netconn_check_count)
			register_uloop_timer(&netconn_check_timer, bh_conf.netconn_check_interval,
				backhaul_netconn_check);
		netconn_check_count++;
	} else {
		update_gateway_arp_table(gwip);

		if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
			get_gwinfo(&gw_info);
			set_backhaul(ETHERNET_BACKHAUL, &net_info.wan_intf);
		} else {
			memset(&gw_info, 0, sizeof(gw_info));
			memset(&bhintf, 0, sizeof(bhintf));
			try_count = bh_conf.netconn_check_count * bh_conf.netconn_check_interval;
			for (i = 0; i < try_count; i++) {
				get_gwinfo(&gw_info);
				get_bhintf(&gw_info, &bhintf);
				intflen = strlen(bhintf.ifname);
				if (intflen == 0) {
					sleep(1);
				} else {
					if (strcmp(bhintf.ifname, wifi_bh_intf.ifname) == 0)
						set_backhaul(WIFI_BACKHAUL, &bhintf);
					else if (strncmp(bhintf.ifname, ETH_INTF_NAME, intflen-1) == 0)
						set_backhaul(ETHERNET_BACKHAUL, &bhintf);
					else
						set_backhaul(NONE_BACKHAUL, NULL);
					break;
				}
			}
			if (intflen == 0)
				set_backhaul(NONE_BACKHAUL, NULL);
		}
	}
	log_debug("%s: Exit\n", __func__);
}

static void intf_newlink_handler(int ifindex, int state)
{
	char ifname[STR_LEN_32] = {0};

	log_debug("%s: Enter\n", __func__);
	if_indextoname(ifindex, ifname);
	log_debug("ifname=%s, state=%d", ifname, state);
	if (state == 1) {
		if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
			if (ifindex == net_info.wan_intf.ifindex) {
				cancel_uloop_timer(&netconn_check_timer);
				netconn_check_count = 0;
				register_uloop_timer(&netconn_check_timer, bh_conf.netconn_check_interval,
					backhaul_netconn_check);
			}
		} else {
			if (strcmp(ifname, wifi_bh_intf.ifname) == 0) {
				if (is_interface_connected(ifname) == 1) {
					sta_state = 1;
					if (netmng_conf.is_mesh) {
						set_dhcp_service(0);
						flush_ip_addr(BRIDGE_IFNAME);
					}
					cancel_uloop_timer(&netconn_check_timer);
					netconn_check_count = 0;
					log_debug("set backhaul_netconn_check timer for %s", ifname);
					register_uloop_timer(&netconn_check_timer, bh_conf.netconn_check_interval,
						backhaul_netconn_check);
				} else
					sta_state = 0;
				log_debug("ifname=%s, sta_state=%d", ifname, sta_state);
				return;
			}
			if (is_wan_intf(ifname)) {
				if (strcmp(ifname, BRIDGE_IFNAME) == 0 && sta_state == 0)
					return;
				if (strncmp(ifname, ETH_INTF_NAME, ETH_INTF_NAME_LEN) == 0
					&& bh_info.cur_backhaul == WIFI_BACKHAUL)
					return;
				if (bh_info.cur_backhaul == ETHERNET_BACKHAUL
					&& strcmp(ifname, bh_info.cur_bhintf.ifname))
					return;
				cancel_uloop_timer(&netconn_check_timer);
				netconn_check_count = 0;
				log_debug("set backhaul_netconn_check timer for %s", ifname);
				register_uloop_timer(&netconn_check_timer, bh_conf.netconn_check_interval,
					backhaul_netconn_check);
			} else {
				log_debug("Needn't handle up event for %s", ifname);
				return;
			}
		}
	} else {
		if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
			if (ifindex == bh_info.cur_bhintf.ifindex)
				set_backhaul(NONE_BACKHAUL, NULL);
		} else {
			if (strcmp(ifname, wifi_bh_intf.ifname) == 0)
				sta_state = 0;
			if (ifindex == bh_info.cur_bhintf.ifindex) {
				log_debug("current backhaul index, ifname=%s", bh_info.cur_bhintf.ifname);
				set_backhaul(NONE_BACKHAUL, NULL);
				if (ifindex == wifi_bh_intf.ifindex && bh_info.last_backhaul == WIFI_BACKHAUL) {
					/* try ethernet backhaul */
					log_debug("try ethernet backhaul");
					if (bridge_link_check()) {
						cancel_uloop_timer(&netconn_check_timer);
						netconn_check_count = 0;
						register_uloop_timer(&netconn_check_timer,
							bh_conf.netconn_check_interval, backhaul_netconn_check);
					}
				} else if (bh_info.last_backhaul == ETHERNET_BACKHAUL) {
					log_debug("try wifi backhaul");
					sta_state = 0;
					set_opmode_by_bhtype(WIFI_BACKHAUL);
				}
			}
		}
	}
	log_debug("%s: Exit\n", __func__);
}

static void newlink_msg_handler(struct ifinfomsg *ifi, uint8_t *buf, size_t len)
{
	int attrlen, rta_len;
	struct rtattr *attr;

	attrlen = len;
	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	attr = (struct rtattr *) buf;

	while (RTA_OK(attr, attrlen)) {
		switch (get_unaligned((uint16_t *)&(attr->rta_type))) {
		case IFLA_LINKMODE:
			log_debug("recv IFLA_LINKMODE event, rta_len=%d\n", rta_len);
			intf_newlink_handler(ifi->ifi_index, ifi->ifi_flags & IFF_RUNNING ? 1 : 0);
			break;
		default:
			break;
		}
		attr = RTA_NEXT(attr, attrlen);
	}
}

static void cls_event_handler(struct clsevt *evt, int len)
{
	char ifname[STR_LEN_32] = {0};
	int wifi_conn = 0;
	int eth_conn = 0;

	log_debug("%s: Enter:evt->type=%d, len=%d, evt->ifindex=%d\n", __func__, evt->type, len, evt->ifindex);
	switch (evt->type) {
	case CLSEVT_TOPOLOGY_CHANGE:
		log_debug("recv Topology Changed event\n");
		/*TODO: */
		break;
	case CLSEVT_NETWORK_LOOP:
		if_indextoname(evt->ifindex, ifname);
		log_debug("loop detected on interface %s!!!, try to rescure...\n", ifname);
		wifi_conn = is_sta_connected(wifi_bh_intf.ifname);
		eth_conn = bridge_link_check();
		if (wifi_conn && eth_conn) {
			sta_state = 0;
			set_wifi_sta_intf(wifi_bh_intf.ifname, 0);
			/*Change to Ethernet backhaul*/
			set_opmode_by_bhtype(ETHERNET_BACKHAUL);
		}
		cancel_uloop_timer(&netconn_check_timer);
		netconn_check_count = 0;
		register_uloop_timer(&netconn_check_timer, bh_conf.netconn_check_interval, backhaul_netconn_check);
		break;
	default:
		break;
	}
	log_debug("%s: Exit\n", __func__);
}

void netlink_rt_handler(struct uloop_fd *u, unsigned int events)
{
	char *buf;
	int len;
	struct nlmsghdr *h;
	uint16_t type = 0;

	buf = malloc(NLMSG_LEN_MAX);
	if (!buf) {
		log_error("Memory allocation failed for event buffer");
		return;
	}

	len = recvfrom(nl_rt_fd.fd, buf, NLMSG_LEN_MAX, MSG_DONTWAIT, NULL, NULL);
	if (len < 0) {
		if (errno != EINTR && errno != EAGAIN)
			log_error("%s:recvfrom failed: %s", __func__, strerror(errno));
		free(buf);
		return;
	}

	h = (struct nlmsghdr *) buf;
	type = get_unaligned((uint16_t *)&(h->nlmsg_type));
	while (NLMSG_OK(h, len)) {
		switch (type) {
		case RTM_NEWLINK:
			if (NLMSG_PAYLOAD(h, 0) < sizeof(struct ifinfomsg))
				break;
			newlink_msg_handler(NLMSG_DATA(h),
				(uint8_t *) NLMSG_DATA(h) + NLMSG_ALIGN(sizeof(struct ifinfomsg)),
				NLMSG_PAYLOAD(h, sizeof(struct ifinfomsg)));
			break;
		case RTM_CLSEVENT:
			if (NLMSG_PAYLOAD(h, 0) < sizeof(struct clsevt))
				break;
			cls_event_handler(NLMSG_DATA(h), NLMSG_PAYLOAD(h, sizeof(struct clsevt)));
			break;
		default:
			break;
		}
		h = NLMSG_NEXT(h, len);
	}

	if (len > 0)
		log_error("%d extra bytes in the end of netlink message", len);

	free(buf);
}

void netlink_uevt_handler(struct uloop_fd *u, unsigned int events)
{
	int len;
	char buf[NLMSG_LEN_MAX] = { 0 };
	struct nl_uevt_info info = { 0 };

	len = recvfrom(nl_uevt_fd.fd, buf, NLMSG_LEN_MAX, MSG_DONTWAIT, NULL, NULL);
	if (len < 0) {
		if (errno != EINTR && errno != EAGAIN)
			log_error("%s:recvfrom failed: %s", __func__, strerror(errno));
		return;
	}

	parse_nl_uevt_message(buf, &info, len);

	switch (info.action) {
	case NL_UEVT_ACTION_RELEASED:
		if (!strcmp(info.button, "wps")) {
			if (netmng_conf.is_mesh) {
				if (handle_wps_onboarding(bh_info.cur_backhaul))
					log_error("%s: Handle wps onboarding fail\n", __func__);
			}
		}
		break;
	default:
		break;
	}

	log_debug("%s: Exit\n", __func__);
}

int netlink_socket_init(void)
{
	log_debug("%s: Enter\n", __func__);
	nl_rt_fd.fd = -1;
	nl_uevt_fd.fd = -1;

	nl_rt_fd.fd = netlink_rt_socket_create();
	if (nl_rt_fd.fd < 0) {
		log_error("Cannot open netlink route socket: %s\n", strerror(errno));
		return -1;
	}
	nl_rt_fd.cb = netlink_rt_handler;

	nl_uevt_fd.fd = netlink_uevt_socket_create();
	if (nl_uevt_fd.fd < 0) {
		close(nl_rt_fd.fd);
		log_error("Cannot open netlink uevent socket: %s\n", strerror(errno));
		return -1;
	}
	nl_uevt_fd.cb = netlink_uevt_handler;

	uloop_fd_add(&nl_rt_fd, ULOOP_READ);
	uloop_fd_add(&nl_uevt_fd, ULOOP_READ);
	log_debug("%s: Exit\n", __func__);
	return 0;
}

void global_init(void)
{
	char ifname[STR_LEN_32] = {0};
	char role[STR_LEN_32] = {0};

	log_debug("%s: Enter\n", __func__);
	arp_fd.fd = -1;

	memset(&bh_conf, 0, sizeof(struct backhaul_conf));
	get_bhconf(&bh_conf);

	memset(&net_info, 0, sizeof(struct network_info));
	get_netinfo(&net_info);

	memset(&gw_info, 0, sizeof(struct interface_info));
	get_gwinfo(&gw_info);

	memset(&bh_info, 0, sizeof(struct backhaul_info));
	get_bhinfo(&bh_info);

	memset(&wifi_bh_intf, 0, sizeof(struct interface_info));
	get_sta_ifname(ifname);
	get_intfinfo(ifname, &wifi_bh_intf);

	set_bridge_loop_detect_state(1);
	system("echo 1 > /proc/sys/net/ipv4/conf/all/arp_accept");
	if (netmng_conf.is_mesh) {
		if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER)
			strcpy(role, UCI_MODE_CONTROLLER);
		else
			strcpy(role, UCI_MODE_AUTO);
		if (strcmp(role, netmng_conf.mesh_role)) {
			set_mesh_role(role);
			strcpy(netmng_conf.mesh_role, role);
		}
	}
	log_debug("%s: Exit\n", __func__);
}

void backhaul_type_init(void)
{
	struct interface_info bhintf;
	char gwip[STR_LEN_32] = {0};
	int intflen = 0;
	int ret = -1;

	if (strlen(net_info.wan_intf.ipstr)) {
		if (net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER) {
			set_backhaul(ETHERNET_BACKHAUL, &net_info.wan_intf);
		} else {
			memset(&bhintf, 0, sizeof(bhintf));
			ret = network_check(net_info.wan_intf.ifname, gwip);
			if (ret < 0) {
				set_backhaul(NONE_BACKHAUL, NULL);
			} else {
				get_bhintf(&gw_info, &bhintf);
				intflen = strlen(bhintf.ifname);
				if (intflen == 0) {
					set_backhaul(NONE_BACKHAUL, NULL);
				} else {
					if (strcmp(bhintf.ifname, wifi_bh_intf.ifname) == 0)
						set_backhaul(WIFI_BACKHAUL, &bhintf);
					else if (strncmp(bhintf.ifname, ETH_INTF_NAME, ETH_INTF_NAME_LEN) == 0)
						set_backhaul(ETHERNET_BACKHAUL, &bhintf);
					else
						set_backhaul(NONE_BACKHAUL, NULL);
				}
			}
		}
	} else {
		set_backhaul(NONE_BACKHAUL, NULL);
	}
	if (bh_info.cur_backhaul == NONE_BACKHAUL && net_info.opmode == CLSAPI_NETWORK_OPMODE_BRIDGE) {
		/*try WiFi backhaul*/
		log_debug("None backhaul, try WiFi backhaul\n");
		set_opmode_by_bhtype(WIFI_BACKHAUL);
	}
}

void backhaul_manager_reload(void)
{
	enum clsapi_net_opmode opmode;
	char wifname[STR_LEN_32] = {0};
	int opmode_change = 0;

	log_debug("%s: Enter\n", __func__);
	get_netinfo_conf(&opmode, wifname);
	if ((opmode == CLSAPI_NETWORK_OPMODE_ROUTER && net_info.opmode != CLSAPI_NETWORK_OPMODE_ROUTER)
		|| (opmode != CLSAPI_NETWORK_OPMODE_ROUTER && net_info.opmode == CLSAPI_NETWORK_OPMODE_ROUTER)) {
		restart_clsmeshd();
		opmode_change = 1;
	}
	if (opmode_change == 1 || strcmp(wifname, net_info.wan_intf.ifname)) {
		log_debug("%s: reload backhaul manager\n", __func__);
		set_backhaul(NONE_BACKHAUL, NULL);
		if (opmode == CLSAPI_NETWORK_OPMODE_ROUTER && netmng_conf.is_mesh)
			set_dhcp_service(0);
		backhaul_manager_deinit();
		global_init();
	}
	log_debug("%s: Exit\n", __func__);
}

void port_link_polling(struct uloop_timeout *t)
{
	int last_state = 0;
	int i = 0;

	log_debug("%s: Enter\n", __func__);
	for (i = 0; i < port_list.port_num; i++) {
		last_state = port_list.ports[i].state;
		get_port_link_state(port_list.ports[i].index, &port_list.ports[i].state);
		if (last_state != port_list.ports[i].state) {
			if (port_list.ports[i].state) {
				log_debug("port %d: Down to up\n", port_list.ports[i].index);
				if (bh_info.cur_backhaul == NONE_BACKHAUL)
					intf_newlink_handler(port_list.ifindex, port_list.ports[i].state);
			} else {
				log_debug("port %d: UP to down\n", port_list.ports[i].index);
				if (port_list.wan_index == port_list.ports[i].index)
					intf_newlink_handler(port_list.ifindex, port_list.ports[i].state);
			}
		}
	}
	register_uloop_timer(&port_polling_timer, 1, port_link_polling);
	log_debug("%s: Exit\n", __func__);
}

void port_link_polling_init(char *port_index, char *port_ifname)
{
	char input[STR_LEN_32] = {0};
	char *ptr;
	char *token;
	char *saveptr;
	int i = 0;

	log_debug("%s: Enter\n", __func__);
	memset(&port_list, 0, sizeof(struct port_link_info));
	port_list.wan_index = -1;

	strcpy(input, port_index);
	port_list.port_num = 1;
	for (ptr = input; *ptr; ptr++) {
		if (*ptr == ',')
			port_list.port_num++;
	}
	port_list.ports = malloc(port_list.port_num * sizeof(struct port_info));
	token = strtok_r(input, ",", &saveptr);
	while (token != NULL) {
		port_list.ports[i].index = atoi(token);
		get_port_link_state(port_list.ports[i].index, &port_list.ports[i].state);
		i++;
		token = strtok_r(NULL, ",", &saveptr);
	}
	strcpy(port_list.ifname, port_ifname);
	get_interface_index(port_list.ifname, &port_list.ifindex);
	for (i = 0; i < port_list.port_num; i++) {
		log_debug("port.index=%d, port_list.state=%d",
			port_list.ports[i].index, port_list.ports[i].state);
	}
	log_debug("port_list.ifname=%s, port_list.ifindex=%d, port_list.port_num=%d",
		port_list.ifname, port_list.ifindex, port_list.port_num);

	register_uloop_timer(&port_polling_timer, 1, port_link_polling);
	log_debug("%s: Exit\n", __func__);
}

int backhaul_manager_init(char *port_index, char *port_ifname)
{
	log_debug("%s: Enter\n", __func__);
	global_init();
	netlink_socket_init();
	if (port_polling)
		port_link_polling_init(port_index, port_ifname);
	backhaul_type_init();
	log_debug("%s: Exit\n", __func__);
	return 0;
}

void backhaul_manager_deinit(void)
{
	if (arp_fd.fd > 0)
		close(arp_fd.fd);
	uloop_fd_delete(&arp_fd);
	cancel_uloop_timer(&arp_timer.timer);
	cancel_uloop_timer(&arp_timer.timeout);
	cancel_uloop_timer(&netconn_check_timer);
}

