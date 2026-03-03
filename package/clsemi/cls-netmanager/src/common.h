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
#ifndef __COMMON_H__
#define __COMMON_H__

#include <arpa/inet.h>
#include <uloop.h>
#include "clsapi_base.h"
#include "clsapi_net.h"
#include "clsapi_wifi.h"
#include "clsapi_sys.h"
#include "list.h"

#define STR_LEN_16 16
#define STR_LEN_32 32
#define STR_LEN_64 64
#define STR_LEN_128 128
#define STR_LEN_256 256
#define STR_LEN_512 512
#define STR_LEN_1025 1025
#define NLMSG_LEN_MAX 4096

#define ETH_ALEN 6

#define NSEC_PER_MSEC		1000000L
#define NSEC_PER_SEC		1000000000L

#define CONTROLLER_WEIGHT_ETH_DEFAULT 70
#define CONTROLLER_WEIGHT_WIFI_DEFAULT 30

#define ETH_INTF_NAME "eth"
#define ETH_INTF_NAME_LEN 3
#define STA_INTF_NAME_DEFAULT "wlan1-sta"
#define BRIDGE_IFNAME "br-lan"
#define DEFAULT_BRIDGE_IPADDR "192.168.2.1"
#define BACKHAUL_STATUS_DISCONNECT "disconnect"
#define BACKHAUL_STATUS_CONNECT "connect"
#define PORT_LINK_UP "PORT_LINK_UP"
#define PORT_LINK_DOWN "PORT_LINK_DOWN"

/*UCI network defination*/
#define UCI_CFG_NETWORK "network"
#define UCI_SECTION_WAN "wan"
#define UCI_PARA_DEVICE "device"
#define UCI_PARA_IPADDR "ipaddr"
#define UCI_PARA_NETMASK "netmask"
#define UCI_PARA_DEVICE "device"

/*UCI cls-opmode defination*/
#define UCI_CFG_CLSOPMODE "cls-opmode"
#define UCI_SECTION_GLOBALS "globals"
#define UCI_PARA_STA_PHYNAME "sta_phyname"

/*UCI clsnetmanager defination*/
#define UCI_CFG_CLSNETMANAGER "cls-netmanager"
#define UCI_SECTION_CONFIG "config"
#define UCI_PARA_ONBOARDING_DONE "onboarding_done"
#define UCI_SECTION_ARPGW "arp_gw"
#define UCI_PARA_INTERVAL "interval"
#define UCI_PARA_TIMEOUT "timeout"
#define UCI_PARA_RETRY_COUNT "retry_count"
#define UCI_SECTION_NETCONN_CHECK "netconn_check"
#define UCI_SECTION_BACKHAUL "backhaul"
#define UCI_PARA_BACKHAUL "backhaul"

/*UCI dhcp defination*/
#define UCI_CFG_DHCP "dhcp"
#define UCI_SECTION_LAN "lan"

/*UCI cls-mesh defination*/
#define UCI_CFG_CLS_MESH "cls-mesh"
#define UCI_SECTION_DEFAULT "default"
#define UCI_PARA_MODE "mode"
#define UCI_MODE_CONTROLLER "controller"
#define UCI_MODE_AUTO "auto"

/*Service defination*/
#define HTTP_SERVICE_DAEMON "uhttpd"
#define UDHCP_SERVICE_DAEMON "udhcpd"
#define UDHCP_CONF_FILE "/var/etc/udhcpd.conf"
#define UDHCP_PID_FILE "/var/run/udhcpd.pid"
#define UDHCP_LEASE_FILE "/tmp/dhcp.leases"

enum ARPING_GW_STATE {
	ARPING_GW_INIT = 0,
	ARPING_GW_SENT,
	ARPING_GW_RECEIVED,
};

struct uloop_timer {
	struct uloop_timeout timer;
	struct uloop_timeout timeout;
	enum ARPING_GW_STATE state;
};

struct netmanager_conf {
	int is_mesh;
	char mesh_role[STR_LEN_32];
	int onboarding_done;
};

struct port_info {
	int index;
	int state;
};

struct port_link_info {
	char ifname[STR_LEN_32];
	int ifindex;
	struct port_info *ports;
	int port_num;
	int wan_index;
};

struct interface_info {
	char ifname[STR_LEN_32];
	int ifindex;                    /* interface index */
	unsigned int ipaddr;
	char ipstr[STR_LEN_32];
	unsigned int subnet;
	char hwaddr[ETH_ALEN];
};

struct backhaul_conf {
	int arping_gw_interval;
	int arping_gw_timeout;
	int arping_gw_count;
	int netconn_check_interval;
	int netconn_check_count;
};

enum opmode_type {
	UNDETERMINED_MODE = 0,
	ROUTE_MODE,
	BRIDGE_MODE,
	REPEATER_MODE,
};

enum backhaul_type {
	UNCONFIGURED_BACKHAUL = -1,
	NONE_BACKHAUL,
	WIFI_BACKHAUL,
	ETHERNET_BACKHAUL,
};

struct backhaul_info {
	enum opmode_type opmode;
	enum backhaul_type last_backhaul;
	enum backhaul_type cur_backhaul;
	struct interface_info last_bhintf;  /* last backhaul interface */
	struct interface_info cur_bhintf;   /* curr backhaul interface */
};

struct gateway_info {
	char ifname[STR_LEN_32];
	unsigned int ipaddr;
	char ipstr[STR_LEN_32];
	unsigned int subnet;
	char hwaddr[ETH_ALEN];
};

struct network_info {
	enum clsapi_net_opmode opmode;
	struct interface_info wan_intf; /* wan interface name */
};

typedef void (*timer_handler)(void *arg);
typedef void (*timer_free_arg)(void *arg);
struct backhaul_timer {
	struct dlist_head lh;
	struct timespec timeout;
	timer_handler handler;
	timer_free_arg free_arg;
	void *arg;
};

typedef void (*uloop_timer_handler)(struct uloop_timeout *t);

#define MACFMT  "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]
#define IS_ZERO_MAC(mac) ((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) == 0x00)

extern struct netmanager_conf netmng_conf;
extern struct backhaul_info bh_info;
extern struct port_link_info port_list;
extern int port_polling;

int register_timer(long sec, long nsec, timer_handler handler, void *arg, timer_free_arg free_arg);
void register_uloop_timer(struct uloop_timeout *timer, int sec, uloop_timer_handler handler);
void cancel_uloop_timer(struct uloop_timeout *timer);

int is_intf_exist(char *ifname);
int get_intfinfo(const char *ifname, struct interface_info *intf_info);
int get_intfip(const char *ifname, struct interface_info *intf_info);
int get_bhconf(struct backhaul_conf *conf);
int get_netinfo_conf(enum clsapi_net_opmode *opmode, char *wifname);
int get_netinfo(struct network_info *netinfo);
int get_opmode(enum clsapi_net_opmode *opmode);
int get_gwinfo(struct interface_info *gwinfo);
int network_check(char *ifname, char *gwip);
int get_bhintf(struct interface_info *gwinfo, struct interface_info *bhintf);
void get_bhtype_name(enum backhaul_type bhtype, char *bhtype_name);
int get_bhinfo(struct backhaul_info *bhinfo);
int set_opmode_by_bhtype(enum backhaul_type bhtype);
int set_netmanager_conf(char *section, char *param, char *value);
int get_sta_ifname(char *ifname);
int set_wifi_sta_intf(char *ifname, int enable);
int is_wan_intf(char *ifname);
int is_sta_connected(const char *ifname);
int is_interface_connected(const char *ifname);
int bridge_link_check(void);
void set_bridge_loop_detect_state(int state);
int set_intf_state(const char *ifname, int state);
int get_backhaul_ifname(const char *phyname, char *bh_ifname, enum backhaul_type bhtype);
bool is_mesh_version(void);
int config_interface(void);
void flush_ip_addr(const char *ifname);
int set_dhcp_service(int enable);
int get_mesh_role(char *role_name);
int set_mesh_role(char *role_name);
int get_onboarding_done(int *status);
void restart_clsmeshd(void);
int get_interface_index(char *ifname, int *ifindex);
void get_port_link_state(int index, int *state);
int get_port_list_wan_index(struct interface_info *gwinfo, int *wan_index);

#endif
