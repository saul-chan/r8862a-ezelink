/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <netinet/ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <glob.h>
#include <cls/cls_nl80211_vendor.h>

#include <libubus.h>
#include <libubox/avl.h>
#include <libubox/usock.h>
#include <libubox/uloop.h>
#include <uci.h>
#include <rpcd/plugin.h>
#include "clsqos.h"

#define NFT_RULE_FILE		"/tmp/vip_qos.nft"
#define NFT_RULE_INDENT		"        "
#define APP_POLICY_FILE		"/etc/cls-qos/app_qos_policy"
#define MAX_APP_ID		32
#define APP_NAME_LEN		15

/* TODO: include clsapi_common.h and replace UBUS_STATUS_UCI_ERROR with CLSAPI_RET_ERR_UCI */
#define UBUS_STATUS_UCI_ERROR	1009

static struct blob_buf blob;

enum {
	RPC_MAC_ADDR,
	__RPC_MAC_ADDR_MAX,
};

static const struct blobmsg_policy rpc_mac_addr_policy[__RPC_MAC_ADDR_MAX] = {
	[RPC_MAC_ADDR] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
};

enum {
	RPC_APP_NAME,
	__RPC_APP_NAME_MAX,
};

static const struct blobmsg_policy rpc_app_name_policy[__RPC_APP_NAME_MAX] = {
	[RPC_APP_NAME] = { .name = "app", .type = BLOBMSG_TYPE_STRING },
};

enum {
	RPC_CLSMARK_VALUE,
	__RPC_CLSMARK_MAX,
};

static const struct blobmsg_policy rpc_clsmark_policy[__RPC_CLSMARK_MAX] = {
	[RPC_CLSMARK_VALUE] = { .name = "value", .type = BLOBMSG_TYPE_STRING },
};

struct app_attr {
	char name[APP_NAME_LEN + 1];
	uint8_t valid;
	uint8_t id;
};

static struct app_attr app_attr_list[MAX_APP_ID];

static int
add_vip_qos_section(struct uci_context *uci_ctx)
{
	int ret;
	struct uci_ptr ptr = {
		.package = UCI_CLSQOS_PACKAGE,
		.section = UCI_VIPQOS_SECTION,
		.option = NULL,
		.value = UCI_VIPQOS_SECTION_TYPE
	};

	ret = uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ret != UCI_OK)
		return ret;

	ret = uci_set(uci_ctx, &ptr);
	if (ret != UCI_OK)
		return ret;

	return uci_save(uci_ctx, ptr.p);
}

enum {
	UCI_ADD_LIST,
	UCI_DEL_LIST
};

static int
uci_config(int cmd, const char *option, const char *value)
{
	int ret = UCI_OK;
	struct uci_context *uci_ctx = NULL;
	struct uci_package *p = NULL;
	struct uci_ptr ptr = {
		.package = UCI_CLSQOS_PACKAGE,
		.section = UCI_VIPQOS_SECTION,
		.option = option,
		.value = value
	};

	uci_ctx = uci_alloc_context();
	if (!uci_ctx)
		return UCI_ERR_MEM;

	ret = uci_load(uci_ctx, UCI_CLSQOS_PACKAGE, &p);
	if (ret != UCI_OK)
		goto out;

	if (uci_lookup_section(uci_ctx, p, UCI_VIPQOS_SECTION) == NULL) {
		ret = add_vip_qos_section(uci_ctx);
		if (ret != UCI_OK)
			goto out;
	}

	uci_unload(uci_ctx, p);

	ret = uci_lookup_ptr(uci_ctx, &ptr, NULL, false);
	if (ret != UCI_OK)
		goto out;

	if (cmd == UCI_ADD_LIST)
		ret = uci_add_list(uci_ctx, &ptr);
	else if (cmd == UCI_DEL_LIST)
		ret = uci_del_list(uci_ctx, &ptr);
	else
		ret = UCI_ERR_INVAL;

	if (ret != UCI_OK)
		goto out;

	ret = uci_save(uci_ctx, ptr.p);
	if (ret != UCI_OK)
		goto out;

	ret = uci_commit(uci_ctx, &ptr.p, false);

out:
	uci_free_context(uci_ctx);
	return ret;
}

static void
load_app_attr(void)
{
	FILE *fp;
	struct app_attr app;
	int i;

	memset(app_attr_list, 0, sizeof(app_attr_list));

	fp = fopen(APP_POLICY_FILE, "r");
	if (!fp)
		return;

	/* skip first line */
	fscanf(fp, "%*[^\n]\n");
	while (1) {
		memset(&app, 0, sizeof(app));
		if (fscanf(fp, "%hhu %15s %*[^\n]\n", &app.id, app.name) != 2)
			break;
		if (app.id >= MAX_APP_ID)
			continue;
		app.valid = 1;
		memcpy(&app_attr_list[app.id], &app, sizeof(app));
	}

	fclose(fp);

	for (i = 0; i < MAX_APP_ID; ++i) {
		if (!app_attr_list[i].valid)
			snprintf(app_attr_list[i].name, APP_NAME_LEN, "undefined(%u)", i);
	}
}

static const char *
get_app_name(uint32_t id)
{
	static char buf[32];

	if (id >= MAX_APP_ID) {
		sprintf(buf, "invalid(%u)", id);
		return buf;
	}

	return app_attr_list[id].name;
}

static const struct app_attr *
find_app_by_name(const char *app_name)
{
	int i;

	for (i = 0; i < MAX_APP_ID; ++i) {
		if (app_attr_list[i].valid && strcmp(app_name, app_attr_list[i].name) == 0)
			return &app_attr_list[i];
	}

	return NULL;
}

static void
gen_rule_to_mark_one_vip_sta(void *data, const char *mac)
{
	FILE *fp = data;
	union clsmark match_mask, match_conn, update_value;

	match_mask.clsmark	= 0;
	match_mask.cls_is_vip	= 1;

	match_conn.clsmark	= 0;

	update_value.clsmark	= 0;
	update_value.cls_is_vip	= 1;

	fprintf(fp, NFT_RULE_INDENT);
	fprintf(fp,
		"ether saddr %s clsmark & 0x%016llx == 0x%016llx clsmark set clsmark | 0x%016llx comment \"mark %s's conn to vip\"\n",
		mac, match_mask.clsmark, match_conn.clsmark, update_value.clsmark, mac);
}

static void
gen_rule_to_mark_one_vip_app(void *data, const char *app_name)
{
	FILE *fp = data;
	union clsmark match_mask, match_conn, update_value;
	const struct app_attr *app;

	app = find_app_by_name(app_name);
	if (!app)
		return;

	match_mask.clsmark	= 0;
	match_mask.cls_is_vip	= 1;
	match_mask.cls_dpi_done	= 1;
	match_mask.cls_app_id	= ~0;

	match_conn.clsmark	= 0;
	match_conn.cls_dpi_done	= 1;
	match_conn.cls_app_id	= app->id;

	update_value.clsmark	= 0;
	update_value.cls_is_vip	= 1;

	fprintf(fp, NFT_RULE_INDENT);
	fprintf(fp,
		"clsmark & 0x%016llx == 0x%016llx clsmark set clsmark | 0x%016llx comment \"mark %s's conn to vip\"\n",
		match_mask.clsmark, match_conn.clsmark, update_value.clsmark, app->name);
}

static void
uci_list_foreach_val(const char *list, void *data, void(*func)(void *, const char *))
{
	struct uci_context *uci_ctx = NULL;
	struct uci_ptr ptr = {
		.package = UCI_CLSQOS_PACKAGE,
		.section = UCI_VIPQOS_SECTION,
		.option = list
	};
	struct uci_element *e;

	uci_ctx = uci_alloc_context();
	if (!uci_ctx)
		return;

	if (uci_lookup_ptr(uci_ctx, &ptr, NULL, false) != UCI_OK ||
	    !(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		uci_free_context(uci_ctx);
		return;
	}

	uci_foreach_element(&ptr.o->v.list, e) {
		func(data, e->name);
	}

	uci_free_context(uci_ctx);
}

static int
gen_rule_to_mark_all_vip_sta(void)
{
	FILE *fp;

	fp = fopen(NFT_RULE_FILE, "w");
	if (!fp)
		return -1;

	fprintf(fp, "table inet vip_qos {\n"
		    "    chain vip_sta {\n"
		    "        type filter hook forward priority filter + 2; policy accept;\n");
	uci_list_foreach_val("vip_sta", fp, gen_rule_to_mark_one_vip_sta);
	fprintf(fp, "    }\n"
		    "}");

	fclose(fp);

	system("nft flush chain inet vip_qos vip_sta 2>/dev/null");
	system("nft -f "NFT_RULE_FILE);
	unlink(NFT_RULE_FILE);

	return 0;
}

static int
gen_rule_to_mark_all_vip_app(void)
{
	FILE *fp;

	fp = fopen(NFT_RULE_FILE, "w");
	if (!fp)
		return -1;

	fprintf(fp, "table inet vip_qos {\n"
		    "    chain vip_app {\n"
		    "        type filter hook forward priority filter + 2; policy accept;\n");
	uci_list_foreach_val("vip_app", fp, gen_rule_to_mark_one_vip_app);
	fprintf(fp, "    }\n"
		    "}");

	fclose(fp);

	system("nft flush chain inet vip_qos vip_app 2>/dev/null");
	system("nft -f "NFT_RULE_FILE);
	unlink(NFT_RULE_FILE);

	return 0;
}

static int get_available_wlan(char *ifname, int len)
{
	glob_t globbuf;
	int i, ret = 0;
	char *p;

	glob("/sys/class/ieee80211/phy0/device/net/*", 0, NULL, &globbuf);

	for (i = 0; i < globbuf.gl_pathc; i++) {
		p = strrchr(globbuf.gl_pathv[i], '/');
		if (!p)
			continue;

		strncpy(ifname, p + 1, len);
		ret = 1;
		break;
	}

	globfree(&globbuf);
	return ret;
}

static void
iw_add_vip_sta(void *data, const char *mac)
{
	char *wlan = data;
	char cmd[256];
	uint8_t addr[ETH_ALEN];

	if (!ether_aton_r(mac, (struct ether_addr *)addr))
		return;

	snprintf(cmd, sizeof(cmd),
		 "iw dev %s vendor send 0x%06x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
		 wlan, CLSEMI_OUI, CLS_NL80211_CMD_ADD_VIP_QOS_MAC,
		 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	system(cmd);
}

static void
iw_del_vip_sta(char *wlan, const char *mac)
{
	char cmd[256];
	uint8_t addr[ETH_ALEN];

	if (!ether_aton_r(mac, (struct ether_addr *)addr))
		return;

	snprintf(cmd, sizeof(cmd),
		 "iw dev %s vendor send 0x%06x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
		 wlan, CLSEMI_OUI, CLS_NL80211_CMD_DEL_VIP_QOS_MAC,
		 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	system(cmd);
}

static void
iw_clr_vip_sta(void)
{
	char wlan[IFNAMSIZ + 1];
	char cmd[256];

	if (!get_available_wlan(wlan, IFNAMSIZ))
		return;

	snprintf(cmd, sizeof(cmd),
		 "iw dev %s vendor send 0x%06x 0x%02x 0x00",
		 wlan, CLSEMI_OUI, CLS_NL80211_CMD_CLR_VIP_QOS_MAC);

	system(cmd);
}

static uint32_t
get_online_app(const char *mac)
{
#define CONNTRACK_OUTPUT_FILE "/tmp/.conntrack_output"
	FILE *fp;
	char cmd[256];
	uint8_t id;
	union clsmark match_mask, match_conn;
	uint32_t app_bits = 0;

	match_mask.clsmark	= 0;
	match_mask.cls_dpi_done	= 1;

	match_conn.clsmark	= 0;
	match_conn.cls_dpi_done	= 1;

	if (mac)
		snprintf(cmd, sizeof(cmd),
			 "conntrack -L -H %s -k 0x%016llx/0x%016llx | awk -F 'clsmark=' '{print $2}' > %s 2>/dev/null",
			 mac, match_conn.clsmark, match_mask.clsmark, CONNTRACK_OUTPUT_FILE);
	else
		snprintf(cmd, sizeof(cmd),
			 "conntrack -L -k 0x%016llx/0x%016llx | awk -F 'clsmark=' '{print $2}' > %s 2>/dev/null",
			 match_conn.clsmark, match_mask.clsmark, CONNTRACK_OUTPUT_FILE);
	/* popen with conntrack command is unstable, don't use popen here */
	system(cmd);
	fp = fopen(CONNTRACK_OUTPUT_FILE, "r");
	if (!fp)
		return 0;
	while (1) {
		if (fscanf(fp, "%llx %*s", &match_conn.clsmark) != 1)
			break;
		id = match_conn.cls_app_id;
		app_bits |= 1 << id;
	}
	fclose(fp);
	unlink(CONNTRACK_OUTPUT_FILE);

	return app_bits;
}

static uint8_t
uci_list_lookup(const char *list, const char *value, int (*cmp)(const char *s1, const char *s2))
{
	struct uci_context *uci_ctx = NULL;
	struct uci_ptr ptr = {
		.package = UCI_CLSQOS_PACKAGE,
		.section = UCI_VIPQOS_SECTION,
		.option = list
	};
	struct uci_element *e;
	uint8_t found = 0;

	uci_ctx = uci_alloc_context();
	if (!uci_ctx)
		return 0;

	if (uci_lookup_ptr(uci_ctx, &ptr, NULL, false) != UCI_OK ||
	    !(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		uci_free_context(uci_ctx);
		return 0;
	}

	uci_foreach_element(&ptr.o->v.list, e) {
		if (cmp(value, e->name) == 0) {
			found = 1;
			break;
		}
	}

	uci_free_context(uci_ctx);

	return found;
}

static int
ether_cmp(const char *s1, const char *s2)
{
	struct ether_addr e1, e2;

	if (!ether_aton_r(s1, &e1) || !ether_aton_r(s2, &e2))
		return -1;

	return memcmp(&e1, &e2, sizeof(struct ether_addr));
}

static inline uint8_t
is_vip_sta(const char *mac)
{
	return uci_list_lookup("vip_sta", mac, ether_cmp);
}

static inline uint8_t
is_vip_app(const char *app_name)
{
	return uci_list_lookup("vip_app", app_name, strcmp);
}

static int
get_sta_status(struct ubus_context *ctx, struct ubus_object *obj,
	       struct ubus_request_data *req, const char *method,
	       struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_MAC_ADDR_MAX];
	const char *macarg;
	char macstr[18];
	char addr[ETH_ALEN];
	uint32_t app_bits, i;
	void *a;

	blobmsg_parse(rpc_mac_addr_policy, __RPC_MAC_ADDR_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[RPC_MAC_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	macarg = blobmsg_get_string(tb[RPC_MAC_ADDR]);
	if (!ether_aton_r(macarg, (struct ether_addr *)addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	sprintf(macstr, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	app_bits = get_online_app(macstr);

	blob_buf_init(&blob, 0);
	blobmsg_add_u8(&blob, "vip", is_vip_sta(macstr));
	a = blobmsg_open_array(&blob, "apps");
	for (i = 1; i < MAX_APP_ID; ++i) {
		if (app_bits & (1 << i))
			blobmsg_add_string(&blob, NULL, get_app_name(i));
	}
	blobmsg_close_array(&blob, a);

	ubus_send_reply(ctx, req, blob.head);
	blob_buf_free(&blob);

	return UBUS_STATUS_OK;
}

static void
runtime_set_vip_app(const struct app_attr *app)
{
	union clsmark match_mask, match_conn, and_val, or_val;
	char cmd[256];

	match_mask.clsmark	= 0;
	match_mask.cls_is_vip	= 1;
	match_mask.cls_dpi_done	= 1;
	match_mask.cls_app_id	= ~0;

	match_conn.clsmark	= 0;
	match_conn.cls_dpi_done	= 1;
	match_conn.cls_app_id	= app->id;

	and_val.clsmark		= ~0;

	or_val.clsmark		= 0;
	or_val.cls_is_vip	= 1;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u !HW_OFFLOAD -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);

	or_val.cls_need_recall	= 1;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u HW_OFFLOAD -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);
}

static void
runtime_set_vip_sta(const char *mac)
{
	union clsmark match_mask, match_conn, and_val, or_val;
	char cmd[256];

	match_mask.clsmark	= 0;
	match_mask.cls_is_vip	= 1;

	match_conn.clsmark	= 0;

	and_val.clsmark		= ~0;

	or_val.clsmark		= 0;
	or_val.cls_is_vip	= 1;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u !HW_OFFLOAD -H %s -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 mac, match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);

	or_val.cls_need_recall	= 1;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u HW_OFFLOAD -H %s -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 mac, match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);
}

static void
runtime_unset_vip_app(const struct app_attr *app, const char *mac)
{
	union clsmark match_mask, match_conn, and_val, or_val;
	char cmd[256];
	char macarg[32] = "";

	if (mac)
		snprintf(macarg, sizeof(macarg), "-H %s", mac);

	match_mask.clsmark	= 0;
	match_mask.cls_is_vip	= 1;
	match_mask.cls_dpi_done	= 1;
	match_mask.cls_app_id	= ~0;

	match_conn.clsmark	= 0;
	match_conn.cls_is_vip	= 1;
	match_conn.cls_dpi_done	= 1;
	match_conn.cls_app_id	= app->id;

	and_val.clsmark		= ~0;
	and_val.cls_is_vip	= 0;

	or_val.clsmark		= 0;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u !HW_OFFLOAD %s -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 macarg, match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);

	or_val.cls_need_recall	= 1;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u HW_OFFLOAD %s -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 macarg, match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);
}

static void
runtime_unset_vip_sta(const char *mac)
{
	int i;
	union clsmark match_mask, match_conn, and_val, or_val;
	char cmd[256];

	for (i = 0; i < MAX_APP_ID; ++i) {
		if (!app_attr_list[i].valid)
			continue;
		if (is_vip_app(app_attr_list[i].name))
			continue;
		runtime_unset_vip_app(&app_attr_list[i], mac);
	}

	match_mask.clsmark	= 0;
	match_mask.cls_dpi_done	= 1;
	match_mask.cls_is_vip	= 1;

	match_conn.clsmark	= 0;
	match_conn.cls_is_vip	= 1;

	and_val.clsmark		= ~0;
	and_val.cls_is_vip	= 0;

	or_val.clsmark		= 0;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -H %s -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 mac, match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);

	system(cmd);
}

static void
runtime_unset_all_vip_conn(void)
{
	union clsmark match_mask, match_conn, and_val, or_val;
	char cmd[256];

	match_mask.clsmark	= 0;
	match_mask.cls_is_vip	= 1;

	match_conn.clsmark	= 0;
	match_conn.cls_is_vip	= 1;

	and_val.clsmark		= ~0;
	and_val.cls_is_vip	= 0;

	or_val.clsmark		= 0;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u !HW_OFFLOAD -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);

	or_val.cls_need_recall	= 1;

	snprintf(cmd, sizeof(cmd),
		 "conntrack -U -u HW_OFFLOAD -k 0x%016llx/0x%016llx -K 0x%016llx/0x%016llx > /dev/null 2>&1",
		 match_conn.clsmark, match_mask.clsmark, or_val.clsmark, and_val.clsmark);
	system(cmd);
}

static int
add_vip_sta(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_MAC_ADDR_MAX];
	const char *macarg;
	char macstr[18];
	char addr[ETH_ALEN];
	char wlan[IFNAMSIZ + 1];

	blobmsg_parse(rpc_mac_addr_policy, __RPC_MAC_ADDR_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[RPC_MAC_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	macarg = blobmsg_get_string(tb[RPC_MAC_ADDR]);
	if (!ether_aton_r(macarg, (struct ether_addr *)addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	sprintf(macstr, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (is_vip_sta(macstr))
		return UBUS_STATUS_OK;

	if (uci_config(UCI_ADD_LIST, "vip_sta", macstr) != UCI_OK)
		return UBUS_STATUS_UCI_ERROR;

	gen_rule_to_mark_all_vip_sta();
	runtime_set_vip_sta(macstr);

	if (get_available_wlan(wlan, IFNAMSIZ))
		iw_add_vip_sta(wlan, macstr);
	system("ubus call service event '{\"type\":\"config.change\",\"data\":{\"package\":\"cls-qos\"}}'");

	return UBUS_STATUS_OK;
}

static void
uci_del_vip_sta(void *data, const char *mac)
{
	if (ether_cmp(data, mac) == 0)
		uci_config(UCI_DEL_LIST, "vip_sta", mac);
}

static int
del_vip_sta(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_MAC_ADDR_MAX];
	const char *macarg;
	char macstr[18];
	char addr[ETH_ALEN];
	char wlan[IFNAMSIZ + 1];

	blobmsg_parse(rpc_mac_addr_policy, __RPC_MAC_ADDR_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[RPC_MAC_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	macarg = blobmsg_get_string(tb[RPC_MAC_ADDR]);
	if (!ether_aton_r(macarg, (struct ether_addr *)addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	sprintf(macstr, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (!is_vip_sta(macstr))
		return UBUS_STATUS_OK;

	uci_list_foreach_val("vip_sta", macstr, uci_del_vip_sta);

	gen_rule_to_mark_all_vip_sta();
	runtime_unset_vip_sta(macstr);

	if (get_available_wlan(wlan, IFNAMSIZ))
		iw_del_vip_sta(wlan, macstr);
	system("ubus call service event '{\"type\":\"config.change\",\"data\":{\"package\":\"cls-qos\"}}'");

	return UBUS_STATUS_OK;
}

static int
add_vip_app(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_APP_NAME_MAX];
	const char *app_name;
	const struct app_attr *app;

	blobmsg_parse(rpc_app_name_policy, __RPC_APP_NAME_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[RPC_APP_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	app_name = blobmsg_get_string(tb[RPC_APP_NAME]);
	app = find_app_by_name(app_name);
	if (!app)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (is_vip_app(app_name))
		return UBUS_STATUS_OK;

	if (uci_config(UCI_ADD_LIST, "vip_app", app_name) != UCI_OK)
		return UBUS_STATUS_UCI_ERROR;

	gen_rule_to_mark_all_vip_app();
	runtime_set_vip_app(app);

	return UBUS_STATUS_OK;
}

static int
del_vip_app(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_APP_NAME_MAX];
	const char *app_name;
	const struct app_attr *app;

	blobmsg_parse(rpc_app_name_policy, __RPC_APP_NAME_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[RPC_APP_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	app_name = blobmsg_get_string(tb[RPC_APP_NAME]);
	app = find_app_by_name(app_name);
	if (!app)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (!is_vip_app(app_name))
		return UBUS_STATUS_OK;

	if (uci_config(UCI_DEL_LIST, "vip_app", app_name) != UCI_OK)
		return UBUS_STATUS_UCI_ERROR;

	gen_rule_to_mark_all_vip_app();
	runtime_unset_vip_app(app, NULL);

	return UBUS_STATUS_OK;
}

static int
start_vip_qos(struct ubus_context *ctx, struct ubus_object *obj,
	      struct ubus_request_data *req, const char *method,
	      struct blob_attr *msg)
{
	char wlan[IFNAMSIZ + 1];

	gen_rule_to_mark_all_vip_sta();
	gen_rule_to_mark_all_vip_app();

	if (get_available_wlan(wlan, IFNAMSIZ))
		uci_list_foreach_val("vip_sta", wlan, iw_add_vip_sta);

	return UBUS_STATUS_OK;
}

static int
stop_vip_qos(struct ubus_context *ctx, struct ubus_object *obj,
	     struct ubus_request_data *req, const char *method,
	     struct blob_attr *msg)
{
	system("nft flush table inet vip_qos 2>/dev/null");
	runtime_unset_all_vip_conn();
	iw_clr_vip_sta();

	return UBUS_STATUS_OK;
}

static int
get_app_list(struct ubus_context *ctx, struct ubus_object *obj,
	     struct ubus_request_data *req, const char *method,
	     struct blob_attr *msg)
{
	uint32_t app_bits, i;
	const char *app_name;
	void *a, *t;

	app_bits = get_online_app(NULL);

	blob_buf_init(&blob, 0);
	a = blobmsg_open_array(&blob, "apps");
	for (i = 1; i < MAX_APP_ID; ++i) {
		if (app_bits & (1 << i)) {
			app_name = get_app_name(i);
			t = blobmsg_open_table(&blob, NULL);
			blobmsg_add_string(&blob, "name", app_name);
			blobmsg_add_u8(&blob, "vip", is_vip_app(app_name));
			blobmsg_close_table(&blob, t);
		}
	}
	blobmsg_close_array(&blob, a);

	ubus_send_reply(ctx, req, blob.head);
	blob_buf_free(&blob);

	return UBUS_STATUS_OK;
}

static int
parse_clsmark(struct ubus_context *ctx, struct ubus_object *obj,
	      struct ubus_request_data *req, const char *method,
	      struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_CLSMARK_MAX];
	const char *value;
	char *endp;
	union clsmark clsmark;

	blobmsg_parse(rpc_clsmark_policy, __RPC_CLSMARK_MAX, tb,
		      blob_data(msg), blob_len(msg));

	if (!tb[RPC_CLSMARK_VALUE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	value = blobmsg_get_string(tb[RPC_CLSMARK_VALUE]);

	clsmark.clsmark = strtoull(value, &endp, 0);
	if (endp == value || *endp != '\0')
		return UBUS_STATUS_INVALID_ARGUMENT;

	blob_buf_init(&blob, 0);

	blobmsg_add_u16(&blob, "cls_ori_set_queue",	clsmark.cls_ori_set_queue);
	blobmsg_add_u16(&blob, "cls_ori_queue",		clsmark.cls_ori_queue);
	blobmsg_add_u16(&blob, "cls_ori_set_pcp",	clsmark.cls_ori_set_pcp);
	blobmsg_add_u16(&blob, "cls_ori_pcp",		clsmark.cls_ori_pcp);
	blobmsg_add_u16(&blob, "cls_ori_set_dscp",	clsmark.cls_ori_set_dscp);
	blobmsg_add_u16(&blob, "cls_ori_dscp",		clsmark.cls_ori_dscp);
	blobmsg_add_u16(&blob, "cls_rep_set_queue",	clsmark.cls_rep_set_queue);
	blobmsg_add_u16(&blob, "cls_rep_queue",		clsmark.cls_rep_queue);
	blobmsg_add_u16(&blob, "cls_rep_set_pcp",	clsmark.cls_rep_set_pcp);
	blobmsg_add_u16(&blob, "cls_rep_pcp",		clsmark.cls_rep_pcp);
	blobmsg_add_u16(&blob, "cls_rep_set_dscp",	clsmark.cls_rep_set_dscp);
	blobmsg_add_u16(&blob, "cls_rep_dscp",		clsmark.cls_rep_dscp);
	blobmsg_add_u16(&blob, "cls_is_vip",		clsmark.cls_is_vip);
	blobmsg_add_u16(&blob, "cls_need_recall",	clsmark.cls_need_recall);
	blobmsg_add_u16(&blob, "cls_is_reverse",	clsmark.cls_is_reverse);
	blobmsg_add_u16(&blob, "cls_no_accel_for_uft",	clsmark.cls_no_accel_for_uft);
	blobmsg_add_u16(&blob, "cls_uft_done",		clsmark.cls_uft_done);
	blobmsg_add_u16(&blob, "cls_no_accel_for_qos",	clsmark.cls_no_accel_for_qos);
	blobmsg_add_u16(&blob, "cls_no_accel_for_ctl",	clsmark.cls_no_accel_for_ctl);
	blobmsg_add_u16(&blob, "cls_dpi_done",		clsmark.cls_dpi_done);
	blobmsg_add_u16(&blob, "cls_app_id",		clsmark.cls_app_id);

	ubus_send_reply(ctx, req, blob.head);
	blob_buf_free(&blob);

	return UBUS_STATUS_OK;
}

static int
rpc_clsqos_api_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	static const struct ubus_method clsqos_methods[] = {
		UBUS_METHOD("get_sta_status", get_sta_status, rpc_mac_addr_policy),
		UBUS_METHOD("add_vip_sta", add_vip_sta, rpc_mac_addr_policy),
		UBUS_METHOD("del_vip_sta", del_vip_sta, rpc_mac_addr_policy),
		UBUS_METHOD("add_vip_app", add_vip_app, rpc_app_name_policy),
		UBUS_METHOD("del_vip_app", del_vip_app, rpc_app_name_policy),
		UBUS_METHOD("parse_clsmark", parse_clsmark, rpc_clsmark_policy),
		UBUS_METHOD_NOARG("get_app_list", get_app_list),
		UBUS_METHOD_NOARG("start", start_vip_qos),
		UBUS_METHOD_NOARG("stop", stop_vip_qos),
	};

	static struct ubus_object_type clsqos_type =
		UBUS_OBJECT_TYPE("rpcd-clsqos", clsqos_methods);

	static struct ubus_object obj = {
		.name = "clsqos",
		.type = &clsqos_type,
		.methods = clsqos_methods,
		.n_methods = ARRAY_SIZE(clsqos_methods),
	};

	load_app_attr();

	return ubus_add_object(ctx, &obj);
}

struct rpc_plugin rpc_plugin = {
	.init = rpc_clsqos_api_init
};
