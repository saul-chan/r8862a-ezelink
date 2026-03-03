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
#include<linux/if_ether.h>
#include "common.h"
#include "ubus.h"
#include "log.h"
#include "backhaul_manager.h"

struct ubus_context *ubus_ctx = NULL;
struct ubus_notification notifications;
struct ubus_event_handler objadd_handler;
int clmesh_api_monitor;

static int
backhaul_handle_change(struct ubus_context *ctx, struct ubus_object *obj,
		 struct ubus_request_data *req, const char *method,
		 struct blob_attr *msg)
{
	return UBUS_STATUS_OK;
}


static struct ubus_method backhaul_object_methods[] = {
	{ .name = UBUS_OBJECT_BACKHAUL, .handler = backhaul_handle_change },
};

static struct ubus_object_type backhaul_object_type =
	UBUS_OBJECT_TYPE(UBUS_OBJECT_BACKHAUL, backhaul_object_methods);


static struct ubus_object backhaul_object = {
	.name = UBUS_OBJECT_BACKHAUL_ID,
	.type = &backhaul_object_type,
	.methods = backhaul_object_methods,
	.n_methods = ARRAY_SIZE(backhaul_object_methods),
};

int ubus_backhaul_notify(char *type, char *status)
{
	static struct blob_buf b;
	void *backhaul;
	int ret;

	blob_buf_init(&b, 0);
	backhaul = blobmsg_open_table(&b, UBUS_OBJECT_BACKHAUL);
	blobmsg_add_string(&b, UBUS_OBJECT_BACKHAUL_TYPE, type);
	blobmsg_add_string(&b, UBUS_OBJECT_BACKHAUL_STATUS, status);
	blobmsg_close_table(&b, backhaul);
	ret = ubus_notify(ubus_ctx, &backhaul_object, UBUS_OBJECT_BACKHAUL, b.head, -1);
	if (ret != UBUS_STATUS_OK)
		return -1;
	return 0;
}

enum {
	SERVICE_ATTR_CONFIG,
	NUM_SERVICE_ATTRS,
};

static const struct blobmsg_policy config_change_policy[] = {
	[SERVICE_ATTR_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
};

enum {
	MESH_ROLE_CHANGE_ATTR_OLD_ROLE,
	MESH_ROLE_CHANGE_ATTR_NEW_ROLE,
	NUM_MESH_ROLE_CHANGE_ATTRS,
};

static const struct blobmsg_policy mesh_role_change_policy[] = {
	[MESH_ROLE_CHANGE_ATTR_OLD_ROLE] = { .name = "old_role", .type = BLOBMSG_TYPE_STRING },
	[MESH_ROLE_CHANGE_ATTR_NEW_ROLE] = { .name = "new_role", .type = BLOBMSG_TYPE_STRING },
};

enum {
	OBJECT_UPDATE_ATTR_PATH,
	NUM_OBJECT_UPDATE_ATTRS,
};

static const struct blobmsg_policy object_update_policy[] = {
	[OBJECT_UPDATE_ATTR_PATH] = { .name = "path", .type = BLOBMSG_TYPE_STRING },
};

int service_notify_handler(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_SERVICE_ATTRS];
	const char *config;

	if (!strcmp(method, UBUS_OBJECT_SERVICE_CONFIG_CHANGE)) {
		blobmsg_parse(config_change_policy, NUM_SERVICE_ATTRS, tb, blob_data(msg), blob_len(msg));
		if (!tb[SERVICE_ATTR_CONFIG]) {
			log_error("Parameter interface should be included\n");
			return -1;
		}
		config = blobmsg_get_string(tb[SERVICE_ATTR_CONFIG]);
		if (!strcmp(config, UBUS_OBJECT_SERVICE_NETWORK)) {
			log_debug("Network changed\n");
			backhaul_manager_reload();
		} else if (!strcmp(config, UBUS_OBJECT_SERVICE_NETMANAGER)) {
			log_debug("cls-netmanager changed\n");
			get_onboarding_done(&netmng_conf.onboarding_done);
		}
	}
	return 0;
}

static int meshapi_notify_handler(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_MESH_ROLE_CHANGE_ATTRS];
	char *role = NULL;

	log_debug("%s: Enter: method:%s\n", __func__, method);
	if (!strcmp(method, UBUS_OBJECT_MESHAPI_MESH_ROLE_CHANGE)) {
		blobmsg_parse(mesh_role_change_policy, NUM_MESH_ROLE_CHANGE_ATTRS, tb, blob_data(msg), blob_len(msg));
		if (!tb[MESH_ROLE_CHANGE_ATTR_OLD_ROLE] || !tb[MESH_ROLE_CHANGE_ATTR_NEW_ROLE]) {
			log_error("Parameter ol role and new role should be included\n");
			return -1;
		}
		role = blobmsg_get_string(tb[MESH_ROLE_CHANGE_ATTR_NEW_ROLE]);
		log_debug("Runtime mesh role is %s\n", role);
	} else if (!strcmp(method, UBUS_OBJECT_MESHAPI_MESH_CONF_SYNCED)) {
		clsapi_base_set_apply_conf_param(UCI_CFG_CLSNETMANAGER,
			UCI_SECTION_CONFIG, UCI_PARA_ONBOARDING_DONE, "1");
	}
	log_debug("%s: Exit\n", __func__);
	return 0;
}

int add_notify_listener(struct ubus_subscriber_ctx *notification, char *object)
{
	if (ubus_lookup_id(ubus_ctx, object, &notification->id)) {
		log_error("Look up id failed on %s!\n", object);
		return -1;
	}
	if (ubus_register_subscriber(ubus_ctx, &notification->suscriber)) {
		log_error("Register subscriber failed on %s!\n", object);
		return -1;
	}
	if (ubus_subscribe(ubus_ctx, &notification->suscriber, notification->id)) {
		log_error("ubus subscribe failed\n");
		return -1;
	}
	return 0;
}

void objupdate_event_handler(struct ubus_context *ctx, struct ubus_event_handler *ev_handler,
	const char *type, struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_SERVICE_ATTRS];
	const char *path;
	int weight = 0;

	if (strcmp(type, UBUS_OBJECT_ADD))
		return;

	blobmsg_parse(object_update_policy, NUM_OBJECT_UPDATE_ATTRS, tb, blob_data(msg), blob_len(msg));
	if (!tb[OBJECT_UPDATE_ATTR_PATH]) {
		log_error("Parameter path should be included\n");
		return;
	}
	path = blobmsg_get_string(tb[OBJECT_UPDATE_ATTR_PATH]);
	if (strcmp(path, UBUS_OBJECT_MESHAPI) == 0) {
		if (clmesh_api_monitor == 1) {
			log_debug("Unregister clmesh.api object notification");
			ubus_unsubscribe(ubus_ctx, &notifications.mesh_notification.suscriber, 0);
			ubus_unregister_subscriber(ubus_ctx, &notifications.mesh_notification.suscriber);
			clmesh_api_monitor = 0;
		}
		log_debug("Register clmesh.api object notification");
		if (add_notify_listener(&notifications.mesh_notification, UBUS_OBJECT_MESHAPI))
			return;
		notifications.mesh_notification.suscriber.cb = meshapi_notify_handler;
		clmesh_api_monitor = 1;
		if (bh_info.cur_backhaul == ETHERNET_BACKHAUL)
			weight = CONTROLLER_WEIGHT_ETH_DEFAULT;
		else if (bh_info.cur_backhaul == WIFI_BACKHAUL)
			weight = CONTROLLER_WEIGHT_WIFI_DEFAULT;
		else
			return;
		send_controller_weight_notify(weight);
	}
}

int ubus_event_listen(void)
{
	objadd_handler.cb = objupdate_event_handler;
	ubus_register_event_handler(ubus_ctx, &objadd_handler, UBUS_OBJECT_ADD);
	return 0;
}

int ubus_notify_listen(void)
{
	unsigned int id;

	log_debug("%s: Enter\n", __func__);

	if (add_notify_listener(&notifications.service_notification, UBUS_OBJECT_SERVICE))
		return -1;
	notifications.service_notification.suscriber.cb = service_notify_handler;

	if (ubus_lookup_id(ubus_ctx, UBUS_OBJECT_MESHAPI, &id) == 0) {
		if (add_notify_listener(&notifications.mesh_notification, UBUS_OBJECT_MESHAPI))
			return -1;
		notifications.mesh_notification.suscriber.cb = meshapi_notify_handler;
		clmesh_api_monitor = 1;
	}
	log_debug("%s: Exit\n", __func__);
	return 0;
}

int ubus_listen(void)
{
	log_debug("%s: Enter\n", __func__);
	clmesh_api_monitor = 0;
	if (ubus_ctx) {
		log_debug("Ubus is already connected\n");
		return 0;
	}

	ubus_ctx = ubus_connect(NULL);
	if (!ubus_ctx) {
		log_error("Connect to ubus failed\n");
		return -1;
	}
	ubus_add_uloop(ubus_ctx);

	if (ubus_notify_listen() < 0) {
		log_debug("Add ubus notification listen failed");
		goto fail;
	}
	if (ubus_event_listen() < 0) {
		log_debug("Add ubus event listen failed");
		goto fail;
	}
	if (ubus_add_object(ubus_ctx, &backhaul_object) < 0) {
		log_debug("Add backhaul_object to ubus failed");
		goto fail;
	}

	log_debug("%s: Exit\n", __func__);
	return 0;
fail:
	ubus_free(ubus_ctx);
	ubus_ctx = NULL;
	return -1;
}

static int send_wps_event(struct ubus_context *ubus_ctx, struct blob_buf b)
{
	int ret = -1;

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "action", "start");

	ret = ubus_send_event(ubus_ctx, "wps_status", b.head);

	return ret;
}

int handle_wps_onboarding(enum backhaul_type bhtype)
{
	int ret = 0;
	static struct blob_buf b;
	uint32_t id;
	char ifname[STR_LEN_32] = { 0 };

	if (ubus_lookup_id(ubus_ctx, "clmesh.api", &id)) {
		log_error("%s: Ubus object not found\n", __func__);
		return -1;
	}

	switch (bhtype) {
	case NONE_BACKHAUL:
		ret = get_backhaul_ifname(CLS_WIFI_RADIO1, ifname, NONE_BACKHAUL);
		if (ret < 0)
			return ret;
		break;
	case WIFI_BACKHAUL:
	case ETHERNET_BACKHAUL:
		ret = get_backhaul_ifname(CLS_WIFI_RADIO1, ifname, WIFI_BACKHAUL);
		if (ret < 0)
			return ret;
		break;
	default:
		return -1;
	}

	ret = send_wps_event(ubus_ctx, b);
	if (ret) {
		log_error("Error sending wps event: %s\n", ubus_strerror(ret));
		return ret;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "ifname", ifname);

	ret = ubus_invoke(ubus_ctx, id, "start_wps", b.head, NULL, NULL, 3000);
	if (ret) {
		log_error("Ubus invoke failed: %s\n", ubus_strerror(ret));
		return ret;
	}

	return 0;
}

int send_controller_weight_notify(int weight)
{
	static struct blob_buf b;
	uint32_t id;
	int ret = 0;

	log_debug("%s: Enter\n", __func__);
	if (ubus_lookup_id(ubus_ctx, UBUS_OBJECT_MESHAPI, &id)) {
		log_error("%s: Ubus object %s not found\n", __func__, UBUS_OBJECT_MESHAPI);
		return -1;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "controller_weight", weight);
	ret = ubus_invoke(ubus_ctx, id, "set", b.head, NULL, NULL, 3000);
	if (ret) {
		log_error("Ubus invoke failed: %s\n", ubus_strerror(ret));
		return ret;
	}
	blob_buf_free(&b);
	log_debug("%s: Exit\n", __func__);
	return 0;
}

