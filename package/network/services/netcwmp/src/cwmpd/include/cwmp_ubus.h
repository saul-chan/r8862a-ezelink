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
#ifndef __CWMP_UBUS_H__
#define __CWMP_UBUS_H__

#include <libubus.h>

#define UBUS_OBJECT_NETWORK_INTERFACE "network.interface"
#define UBUS_OBJECT_NETWORK_METHOD_UPDATE "interface.update"
#define UBUS_OBJECT_NETWORK_METHOD_DOWN "interface.down"
#define UBUS_OBJECT_NETWORK_INTF_WAN "wan"
#define UBUS_OBJECT_SERVICE "service"
#define UBUS_OBJECT_SERVICE_CONFIG_CHANGE "config.change"
#define UBUS_OBJECT_SERVICE_NETCWMP "netcwmp"
#define UBUS_OBJECT_HOSTAPD "hostapd"
#define UBUS_OBJECT_HOSTAPD_ASSOC "assoc"
#define UBUS_OBJECT_HOSTAPD_DISASSOC "disassoc"
#define UBUS_OBJECT_ADD "ubus.object.add"
#define UBUS_OBJECT_REMOVE "ubus.object.remove"
#define UBUS_OBJECT_HOSTAPD_PATH "hostapd.wlan"


struct ubus_subscriber_ctx {
	struct ubus_subscriber suscriber;
	unsigned int id;
};

struct ubus_notification {
	struct ubus_subscriber_ctx network_notification;
	struct ubus_subscriber_ctx service_notification;
	struct ubus_subscriber_ctx hostapd_notification;
};

int cwmp_event_listen(cwmp_t *cwmp);

#endif
