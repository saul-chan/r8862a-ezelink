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
#ifndef __UBUS_H__
#define __UBUS_H__

#include <libubus.h>

#define UBUS_OBJECT_NETWORK_INTERFACE "network.interface"
#define UBUS_OBJECT_NETWORK_METHOD_UP "interface.update"
#define UBUS_OBJECT_SERVICE_NETWORK "network"
#define UBUS_OBJECT_SERVICE_NETMANAGER "cls-netmanager"

#define UBUS_OBJECT_HOSTAPD "hostapd"
#define UBUS_OBJECT_ASSOC "assoc"
#define UBUS_OBJECT_DISASSOC "disassoc"
#define UBUS_OBJECT_WPASUPPLICANT "wpa_supplicant"
/* config change monitor */
#define UBUS_OBJECT_SERVICE "service"
#define UBUS_OBJECT_SERVICE_CONFIG_CHANGE "config.change"
/* mesh role change monitor */
#define UBUS_OBJECT_MESHAPI "clmesh.api"
#define UBUS_OBJECT_MESHAPI_MESH_ROLE_CHANGE "role.change"
#define UBUS_OBJECT_MESHAPI_MESH_CONF_SYNCED "configuration_synchronized"

/* backhaul notification */
#define UBUS_OBJECT_BACKHAUL_ID "network.clsnetmanager"
#define UBUS_OBJECT_BACKHAUL "clsnetmanager.backhaul"
#define UBUS_OBJECT_BACKHAUL_TYPE "type"
#define UBUS_OBJECT_BACKHAUL_STATUS "status"

/* ubus object add event */
#define UBUS_OBJECT_ADD "ubus.object.add"

struct ubus_subscriber_ctx {
	struct ubus_subscriber suscriber;
	unsigned int id;
};

struct ubus_notification {
	struct ubus_subscriber_ctx wifi_notification;
	struct ubus_subscriber_ctx service_notification;
	struct ubus_subscriber_ctx mesh_notification;
};

int ubus_backhaul_notify(char *type, char *status);
int ubus_listen(void);
int ubus_backhaul_notify(char *type, char *status);
int handle_wps_onboarding(enum backhaul_type bhtype);
int send_controller_weight_notify(int weight);

#endif

