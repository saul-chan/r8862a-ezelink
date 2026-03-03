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
#include <unistd.h>
#include <errno.h>
#include <libubox/uloop.h>
#include "log.h"
#include "ubus.h"
#include "backhaul_manager.h"

struct netmanager_conf netmng_conf;
int port_polling = 0;

static void usage(void)
{
	printf(
		"%s: clsnetmanager [OPTIONS]\n"
		"   cls net manager\n"
		"   options are:\n"
		"       -h   show this help\n"
		"       -d   show more debug messages (-ddd is the max level)\n"
		"       -s   log output to syslog instead of stdout\n"
		"       -f <file>	log output to debug file instead of stdout\n"
		"       -p <ports>	interface index list for state polling\n"
		"       -i <ifname>	interface name of state polling\n",
		__func__);
	exit(0);
}
void netmng_conf_init(void)
{
	netmng_conf.is_mesh = is_mesh_version();
	get_mesh_role(netmng_conf.mesh_role);
	get_onboarding_done(&netmng_conf.onboarding_done);
}

int main(int argc, char *argv[])
{
	int opt;
	char *port_index = NULL;
	char *port_ifname = NULL;

	for (;;) {
		opt = getopt(argc, argv, "hdsf:p:i:");
		if (opt < 0)
			break;
		switch (opt) {
		case 'h':
			usage();
			break;
		case 'd':
			debug_level--;
			break;
		case 'f':
			log_file = fopen(optarg, "a");
			break;
		case 's':
			debug_syslog = 1;
			break;
		case 'p':
			port_index = optarg;
			port_polling = 1;
			break;
		case 'i':
			port_ifname = optarg;
			break;
		default:
			usage();
			break;
		}
	}

	if (!log_file)
		log_file = fopen(DEFAULT_LOG_FILE, "a");

	log_debug("clsnetmanager start......");
	uloop_init();
	ubus_listen();
	netmng_conf_init();
	backhaul_manager_init(port_index, port_ifname);
	uloop_run();
	return 1;
}


