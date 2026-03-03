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
#include <cwmp/cwmp.h>
#include <cwmp/event.h>
#include <cwmp/cfg.h>
#include "cwmp_ubus.h"
#include "cwmp_type.h"
#include "cwmp_thread.h"
#include "cwmp/log.h"
#include "modules/data_model_util.h"

struct ubus_context *cwmp_ubus_ctx = NULL;
static pthread_t ubus_listen_thread = 0;
struct ubus_notification notifications;
static cwmp_t *ubus_cwmp = NULL;

extern struct wifi_interface_list ap_list[WIFI_BAND_MAX][WIFI_MAX_VAP];
extern int ap_list_update;

enum {
	NETWORK_INTERFACE_ATTR_INTERFACE,
	NUM_NETWORK_INTERFACE_ATTRS,
};

static const struct blobmsg_policy interface_up_policy[] = {
	[NETWORK_INTERFACE_ATTR_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
};

enum {
	SERVICE_ATTR_CONFIG,
	NUM_SERVICE_ATTRS,
};

static const struct blobmsg_policy service_policy[] = {
	[SERVICE_ATTR_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
};

enum {
	HOSTAPD_ASSOC_ATTR_ADDRESS,
	HOSTAPD_ASSOC_ATTR_TARGET,
	HOSTAPD_ASSOC_ATTR_SIGNAL,
	HOSTAPD_ASSOC_ATTR_FREQ,
	NUM_HOSTAPD_ASSOC_ATTRS,
};

static const struct blobmsg_policy hostapd_assoc_policy[] = {
	[HOSTAPD_ASSOC_ATTR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
	[HOSTAPD_ASSOC_ATTR_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
	[HOSTAPD_ASSOC_ATTR_SIGNAL] = { .name = "signal", .type = BLOBMSG_TYPE_INT32 },
	[HOSTAPD_ASSOC_ATTR_FREQ] = { .name = "freq", .type = BLOBMSG_TYPE_INT32 },
};

enum {
	HOSTAPD_DISASSOC_ATTR_ADDRESS,
	NUM_HOSTAPD_DISASSOC_ATTRS,
};

static const struct blobmsg_policy hostapd_disassoc_policy[] = {
	[HOSTAPD_DISASSOC_ATTR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
};

enum {
	OBJECT_UPDATE_ATTR_PATH,
	NUM_OBJECT_UPDATE_ATTRS,
};

static const struct blobmsg_policy object_update_policy[] = {
	[OBJECT_UPDATE_ATTR_PATH] = { .name = "path", .type = BLOBMSG_TYPE_STRING },
};

static int interface_update_handler(struct blob_attr *msg, int state)
{
	struct blob_attr *tb[NUM_NETWORK_INTERFACE_ATTRS];
	char *intf = NULL;

	blobmsg_parse(interface_up_policy, NUM_NETWORK_INTERFACE_ATTRS, tb, blob_data(msg), blob_len(msg));
	if(!tb[NETWORK_INTERFACE_ATTR_INTERFACE]) {
		cwmp_log_error("Parameter interface should be included\n");
		return -1;
	}
	intf = blobmsg_get_string(tb[NETWORK_INTERFACE_ATTR_INTERFACE]);
	if(!strcmp(intf, UBUS_OBJECT_NETWORK_INTF_WAN)) {
		ubus_cwmp->conn_state = state;
		cwmp_event_set_value(ubus_cwmp, INFORM_VALUECHANGE, EVENT_REF_UNTIL_REBOOTED, NULL, 0, 0, 0);
		ubus_cwmp->new_request = CWMP_YES;
	} else {
		cwmp_log_debug("Needn't handle, ignore\n");
	}
	return 0;
}

static int cwmp_event_service_handler(struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_SERVICE_ATTRS];
	const char *config;
	char value[CWMP_STR_LEN_1025];
	char *ptr;
	int changeflag = 0;

	blobmsg_parse(service_policy, NUM_SERVICE_ATTRS, tb, blob_data(msg), blob_len(msg));
	if(!tb[SERVICE_ATTR_CONFIG]){
		cwmp_log_error("Parameter interface should be included\n");
		return -1;
	}
	config = blobmsg_get_string(tb[SERVICE_ATTR_CONFIG]);
	if(!strcmp(config, UBUS_OBJECT_SERVICE_NETCWMP)) {
		if(clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_PC, value)) {
			cwmp_log_error("Get %s failed\n", UCI_PARA_CPE_PC);
		} else {
			ptr = cwmp_conf_pool_get(ubus_cwmp->pool, "cwmp:cpe_pc");
			if (strcmp(ptr, value))
				changeflag = 1;
		}
		if(ubus_cwmp->cpe_auth) {
			if(clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_USERNAME, value)) {
				cwmp_log_error("Get %s failed\n", UCI_PARA_CPE_USERNAME);
			} else {
				ptr = cwmp_conf_pool_get(ubus_cwmp->pool, "cwmp:cpe_username");
				if (strcmp(ptr, value))
					changeflag = 1;
			}
			if(clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_CPE_PASSWORD, value)) {
				cwmp_log_error("Get %s failed\n", UCI_PARA_CPE_PASSWORD);
			} else {
				ptr = cwmp_conf_pool_get(ubus_cwmp->pool, "cwmp:cpe_password");
				if (strcmp(ptr, value))
					changeflag = 1;
			}
		}
		if(ubus_cwmp->acs_auth) {
			if(clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ACS_USERNAME, value)) {
				cwmp_log_error("Get %s failed\n", UCI_PARA_ACS_USERNAME);
			} else {
				ptr = cwmp_conf_pool_get(ubus_cwmp->pool, "cwmp:acs_username");
				if (strcmp(ptr, value))
					changeflag = 1;
			}
			if(clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ACS_PASSWORD, value)) {
				cwmp_log_error("Get %s failed\n", UCI_PARA_ACS_PASSWORD);
			} else {
				ptr = cwmp_conf_pool_get(ubus_cwmp->pool, "cwmp:acs_password");
				if (strcmp(ptr, value))
					changeflag = 1;
			}
		}
		if(clsapi_base_get_conf_param(UCI_CFG_NETCWMP, UCI_SECTION_CWMP, UCI_PARA_ACS_URL, value)) {
			cwmp_log_error("Get %s failed\n", UCI_PARA_ACS_URL);
		} else {
			ptr = cwmp_conf_pool_get(ubus_cwmp->pool, "cwmp:acs_url");
			if (strcmp(ptr, value))
				changeflag = 1;
		}
		if(changeflag == 1) {
			cwmp_event_set_value(ubus_cwmp, INFORM_VALUECHANGE, EVENT_REF_UNTIL_REBOOTED, NULL, 0, 0, 0);
			ubus_cwmp->new_request = CWMP_YES;
		}
	} else {
		cwmp_log_debug("Needn't handle, ignore\n");
	}
	return 0;
}

static int cwmp_event_hostapd_handler(const char *method, struct blob_attr *msg)
{
	struct blob_attr *assoc_tb[NUM_HOSTAPD_ASSOC_ATTRS];
	struct blob_attr *disassoc_tb[NUM_HOSTAPD_DISASSOC_ATTRS];
	char *mac;
	unsigned char hexmac[ETH_ALEN] = {0};
	char ifname[CWMP_IFNAME_LEN] = {0};
	int i = 0;
	int j = 0;

	if(!strcmp(method, UBUS_OBJECT_HOSTAPD_ASSOC)) {
		blobmsg_parse(hostapd_assoc_policy, NUM_HOSTAPD_ASSOC_ATTRS, assoc_tb, blob_data(msg), blob_len(msg));
		if(!assoc_tb[HOSTAPD_ASSOC_ATTR_ADDRESS] || !assoc_tb[HOSTAPD_ASSOC_ATTR_TARGET]){
			cwmp_log_error("Parameter address and target should be included\n");
			return -1;
		}
		mac = blobmsg_get_string(assoc_tb[HOSTAPD_ASSOC_ATTR_TARGET]);
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if (!strcmp(ap_list[i][j].macaddr, mac)) {
					if(ap_list[i][j].assoc_sta.stalist) {
						free(ap_list[i][j].assoc_sta.stalist);
						ap_list[i][j].assoc_sta.stalist = NULL;
					}
					ap_list[i][j].assoc_sta.stanum = clsapi_wifi_get_assoc_list(ap_list[i][j].ifname, &ap_list[i][j].assoc_sta.stalist);
					if(ap_list[i][j].assoc_sta.stalist == NULL)
						ap_list[i][j].assoc_sta.stanum = 0;
					return 0;
				}
			}
		}
	} else if (!strcmp(method, UBUS_OBJECT_HOSTAPD_DISASSOC)) {
		blobmsg_parse(hostapd_disassoc_policy, NUM_HOSTAPD_DISASSOC_ATTRS, disassoc_tb, blob_data(msg), blob_len(msg));
		if(!disassoc_tb[HOSTAPD_DISASSOC_ATTR_ADDRESS]){
			cwmp_log_error("Parameter address should be included\n");
			return -1;
		}
		mac = blobmsg_get_string(disassoc_tb[HOSTAPD_DISASSOC_ATTR_ADDRESS]);
		if(macstrtohex(mac, hexmac))
			return FAULT_CODE_9003;
		if(clsapi_wifi_get_assoc_ap_by_station(hexmac, ifname))
			return FAULT_CODE_9003;
		for(i = 0; i < WIFI_BAND_MAX; i++) {
			for(j = 0; j < WIFI_MAX_VAP; j++) {
				if (!strcmp(ap_list[i][j].ifname, ifname)) {
					if(ap_list[i][j].assoc_sta.stalist) {
						free(ap_list[i][j].assoc_sta.stalist);
						ap_list[i][j].assoc_sta.stalist = NULL;
					}
					ap_list[i][j].assoc_sta.stanum = clsapi_wifi_get_assoc_list(ap_list[i][j].ifname, &ap_list[i][j].assoc_sta.stalist);
					if(ap_list[i][j].assoc_sta.stalist == NULL)
						ap_list[i][j].assoc_sta.stanum = 0;
					return 0;
				}
			}
		}
	}
	return 0;
}

void cwmp_event_objupdate_handler(struct ubus_context *ctx, struct ubus_event_handler *ev_handler,
	const char *type, struct blob_attr *msg)
{
	struct blob_attr *tb[NUM_SERVICE_ATTRS];
	const char *path;

	if(strcmp(type, UBUS_OBJECT_ADD) && strcmp(type, UBUS_OBJECT_REMOVE))
		return;
	blobmsg_parse(object_update_policy, NUM_OBJECT_UPDATE_ATTRS, tb, blob_data(msg), blob_len(msg));
	if(!tb[OBJECT_UPDATE_ATTR_PATH]){
		cwmp_log_error("Parameter path should be included\n");
		return;
	}
	path = blobmsg_get_string(tb[OBJECT_UPDATE_ATTR_PATH]);
	if(strncmp(path, UBUS_OBJECT_HOSTAPD_PATH, strlen(UBUS_OBJECT_HOSTAPD_PATH)) == 0) {
		ap_list_update = 1;
	}
	return;
}

int cwmp_event_handler(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	if(!strcmp(method, UBUS_OBJECT_NETWORK_METHOD_UPDATE))
		interface_update_handler(msg, 1);
	else if(!strcmp(method, UBUS_OBJECT_NETWORK_METHOD_DOWN))
		interface_update_handler(msg, 0);
	else if(!strcmp(method, UBUS_OBJECT_SERVICE_CONFIG_CHANGE))
		cwmp_event_service_handler(msg);
	else if(!strcmp(method, UBUS_OBJECT_HOSTAPD_ASSOC) || !strcmp(method, UBUS_OBJECT_HOSTAPD_DISASSOC))
		cwmp_event_hostapd_handler(method, msg);
	else
		cwmp_log_debug("Not registed, ignore\n");
	return 0;
}

void cwmp_event_listen_clear() {
	if(cwmp_ubus_ctx) {
		ubus_free(cwmp_ubus_ctx);
		cwmp_ubus_ctx = NULL;
		uloop_done();
	}

	if(ubus_listen_thread == 0) {
		cwmp_log_debug("No ubus listen thread");
		return;
	}
	if(pthread_cancel(ubus_listen_thread) != 0) {
		cwmp_log_error("Cancel ubus listen thread(%d) failed", ubus_listen_thread);
		return;
	}
	if(pthread_join(ubus_listen_thread, NULL) != 0) {
		cwmp_log_error("Join ubus listen thread(%d) failed", ubus_listen_thread);
		return;
	}
	cwmp_log_debug("Ubus listen thread has finided");
	ubus_listen_thread = 0;
	return;
}

int add_event_listener(struct ubus_subscriber_ctx *notification, char *object)
{
	notification->suscriber.cb = cwmp_event_handler;
	if(ubus_lookup_id(cwmp_ubus_ctx, object, &notification->id)) {
		cwmp_log_error("Look up id failed on %s!\n", object);
		return -1;
	}
	if(ubus_register_subscriber(cwmp_ubus_ctx, &notification->suscriber)) {
		cwmp_log_error("Register subscriber failed on %s!\n", object);
		return -1;
	}
	if(ubus_subscribe(cwmp_ubus_ctx, &notification->suscriber, notification->id)) {
		cwmp_log_error("ubus subscribe failed\n");
		return -1;
	}
	return 0;
}

int add_hostapd_listener()
{
	char ifname[CWMP_IFNAME_LEN] = {0};
	char ubus_name[CWMP_STR_LEN_64] = {0};
	enum clsapi_wifi_iftype wifi_iftype;
	int i = 0;
	int j = 0;

	wifi_interface_list_load();
	for(i = 0; i < WIFI_BAND_MAX; i++) {
		for(j = 0; j < WIFI_MAX_VAP; j++) {
			if(j == 0)
				snprintf(ifname, sizeof(ifname), "wlan%d", i);
			else
				snprintf(ifname, sizeof(ifname), "wlan%d-%d", i, j);
			if(clsapi_wifi_get_iftype(ifname, &wifi_iftype) != CLSAPI_OK) {
				continue;
			}
			if(wifi_iftype == CLSAPI_WIFI_IFTYPE_AP) {
				snprintf(ubus_name, sizeof(ubus_name), "%s.%s", UBUS_OBJECT_HOSTAPD, ifname);
				add_event_listener(&notifications.hostapd_notification, ubus_name);
			}
		}
	}
	return 0;
}

void *cwmp_event_listen_hander(cwmp_t *cwmp)
{
	struct ubus_event_handler objadd_handler;

	uloop_init();
	cwmp_ubus_ctx = ubus_connect(NULL);
	if(!cwmp_ubus_ctx) {
		cwmp_log_error("Connect to ubus failed\n");
		return NULL;
	}
	ubus_add_uloop(cwmp_ubus_ctx);
	ubus_cwmp = cwmp;

	if(add_event_listener(&notifications.network_notification, UBUS_OBJECT_NETWORK_INTERFACE))
		goto fail;
	if(add_event_listener(&notifications.service_notification, UBUS_OBJECT_SERVICE))
		goto fail;
	if(add_hostapd_listener())
		goto fail;

	objadd_handler.cb = cwmp_event_objupdate_handler;
	ubus_register_event_handler(cwmp_ubus_ctx, &objadd_handler, UBUS_OBJECT_ADD);
	ubus_register_event_handler(cwmp_ubus_ctx, &objadd_handler, UBUS_OBJECT_REMOVE);

	uloop_run();
	return NULL;
fail:
	ubus_free(cwmp_ubus_ctx);
	cwmp_ubus_ctx = NULL;
	return NULL;
}

int cwmp_event_listen(cwmp_t *cwmp)
{
	if(cwmp_ubus_ctx) {
		cwmp_log_debug("Ubus is already connected\n");
		return 0;
	}

	if(pthread_create(&ubus_listen_thread, NULL, (void *)cwmp_event_listen_hander, cwmp)) {
		cwmp_log_error("Create thread failed for cwmp network ubus listen\n");
		return -1;
	}
	return 0;
}

