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

#include <unistd.h>
#include <uci.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>


#define UBUS_OBJ_UCI			"uci"
#define UBUS_UCI_METHOD_SET		"set"
#define UBUS_UCI_METHOD_GET		"get"
#define UBUS_UCI_METHOD_COMMIT		"commit"

#define UCI_CFG_WIRELESS		"wireless"


/*
 * call ubus uci interface to set and commit uci option.
 * Note: ubus uci commit will trigger ucitrack and apply the changed config.
 * Returns:
 *   0:  Success
 *   !0: Errors
 * */
static inline int cls_ubus_uci_set_and_commit(const char *cfg, const char *section, const char *opt, const char *value)
{
	uint32_t id;
	int ret;
	struct ubus_context *ctx;
	static struct blob_buf buf;
	void *table;

	ctx = ubus_connect(NULL);
	if (!ctx) {
		printf("Failed to connect to ubus\n");
		return -1;
	}

	ret = ubus_lookup_id(ctx, UBUS_OBJ_UCI, &id);
	if (ret) {
		printf("Error: ubus_lookup_id() returns %d\n", ret);
		goto out;
	}

	blob_buf_init(&buf, 0);
	blobmsg_add_string(&buf, "config", cfg);
	blobmsg_add_string(&buf, "section", section);
	table = blobmsg_open_table(&buf, "values");
	blobmsg_add_string(&buf, opt, value);
	blobmsg_close_table(&buf, table);

	ret = ubus_invoke(ctx, id, UBUS_UCI_METHOD_SET, buf.head, NULL, NULL, 1000);
	if (ret) {
		printf("Error: ubus_invoke(%s %s) returns %d\n", UBUS_OBJ_UCI, UBUS_UCI_METHOD_SET, ret);
		goto out;
	}

	blob_buf_init(&buf, 0);
	blobmsg_add_string(&buf, "config", cfg);
	ret = ubus_invoke(ctx, id, UBUS_UCI_METHOD_COMMIT, buf.head, NULL, NULL, 1000);
	if (ret) {
		printf("Error: ubus_invoke(%s %s) returns %d\n", UBUS_OBJ_UCI, UBUS_UCI_METHOD_COMMIT, ret);
		goto out;
	}

out:
	if (ctx)
		ubus_free(ctx);
	blob_buf_free(&buf);

	return ret;
}


#ifdef USE_UBUS_UCI_GET
char *uci_ret_buf;
unsigned int uci_buf_len = 0;

enum {
	UCI_RETURN_VALUE,
	UCI_RETURN_VALUES,
	__UCI_RETURN_MAX,
};

static const struct blobmsg_policy ubus_uci_get_policy[__UCI_RETURN_MAX] = {
	[UCI_RETURN_VALUE]	= { .name = "value",	.type = BLOBMSG_TYPE_STRING }, /* return value of option */
	[UCI_RETURN_VALUES]	= { .name = "values",	.type = BLOBMSG_TYPE_ARRAY },  /* return value of section or package */
};

void cls_ubus_uci_get_cb(struct ubus_request *req, int type, struct blob_attr *msg);
void cls_ubus_uci_get_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	int *ret = (int *)req->priv;;
	struct blob_attr *tb[__UCI_RETURN_MAX];

	*ret = blobmsg_parse(ubus_uci_get_policy, __UCI_RETURN_MAX, tb, blob_data(msg), blob_len(msg));
	if (*ret < 0) {
		printf("error in blobmsg_parse(), ret=%d\n", *ret);
		return;
	}

	if (!tb[UCI_RETURN_VALUE]) {
		printf("No value returned in uci get\n");
		*ret = -1;
		return;
	}

	strncpy(uci_ret_buf, blobmsg_get_string(tb[UCI_RETURN_VALUE]), uci_buf_len);

	return;
}

/*
 * call ubus uci interface to get uci option.
 * Returns:
 *   0:  Success
 *   !0: Errors
 */

static inline int cls_ubus_uci_get(const char *cfg, const char *section, const char *opt, char *ret_value, const int len)
{
	uint32_t id;
	int ret = 0, invoke_cb_ret = 0;
	struct ubus_context *ctx;
	static struct blob_buf buf;

	ctx = ubus_connect(NULL);
	if (!ctx) {
		printf("Failed to connect to ubus\n");
		return -1;
	}

	ret = ubus_lookup_id(ctx, UBUS_OBJ_UCI, &id);
	if (ret) {
		printf("Error: ubus_lookup_id() returns %d\n", ret);
		goto out;
	}

	blob_buf_init(&buf, 0);
	blobmsg_add_string(&buf, "config", cfg);
	blobmsg_add_string(&buf, "section", section);
	blobmsg_add_string(&buf, "option", opt);

	uci_ret_buf = ret_value;
	uci_buf_len = len;
	ret = ubus_invoke(ctx, id, UBUS_UCI_METHOD_GET, buf.head, cls_ubus_uci_get_cb, &invoke_cb_ret, 1000);
	if (ret) {
		printf("Error: ubus_invoke(%s %s) returns %d\n", UBUS_OBJ_UCI, UBUS_UCI_METHOD_GET, ret);
		goto out;
	}
	if (invoke_cb_ret < 0)
		ret = invoke_cb_ret;

out:
	if (ctx)
		ubus_free(ctx);
	blob_buf_free(&buf);

	return ret;
}
#endif /* USE_UBUS_UCI_GET */


/*
 * call ubus uci interface to get uci option.
 * Returns:
 *   0:  Success
 *   !0: Errors
 */

static inline int cls_uci_get(const char *cfg, const char *section, const char *opt, char *ret_value, const int len)
{
	int ret = 0;
	struct uci_context *ctx = NULL;
	struct uci_ptr ptr = {
		.package = cfg,
		.section = section,
		.option  = opt,
	};

	if (!cfg || !section || !opt || !ret_value)
		return -1;

	ctx = uci_alloc_context();
	if (!ctx) {
		printf("error in uci_alloc_context()\n");
		return -1;
	}


	if (uci_lookup_ptr(ctx, &ptr, NULL, false) || ptr.o == NULL) {
		ret = -1;
		goto out;
	}
	strncpy(ret_value, ptr.o->v.string, len);

out:
	if (ctx)
		uci_free_context(ctx);

	return ret;
}


/*
 * convert Wi-Fi ifname to section name of uci 'wireless'.
 * OpenWrt ifname generation rules:
 *   o OpenWrt default Wi-Fi ifname = 'wlan<radio_idx>[-if_idx]', omit [-if_idx] if if_idx=0
 *   o if 'ifname' option presented ==> ifname = value of 'ifname' option
 * Returns:
 *   0:  Success and section name in 'section' param
 *   !0: Errors
 */
int cls_wifi_ifname_to_uci_section(const char *ifname, char *section, const int len);
int cls_wifi_ifname_to_uci_section(const char *ifname, char *section, const int len)
{
	int ret = 0;

	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	struct uci_element *e;
	int tgt_phy_idx = -1, tgt_if_idx = -1, is_def_openwrt_ifname = 0;
	char tgt_dev_name[32];
	int if_idx = 0;

	tgt_dev_name[0] = 0;

	ret = sscanf(ifname, "wlan%d-%d", &tgt_phy_idx, &tgt_if_idx);
	if (ret > 0) {
		is_def_openwrt_ifname = 1;
		snprintf(tgt_dev_name, 32, "radio%d", tgt_phy_idx);
		if (tgt_if_idx == 0) {
			printf("'wlan<idx>-0' is invalid ifname, input wlan<idx> instead.\n");
			ret = -1;
			goto out;
		} else if (tgt_if_idx == -1)
			tgt_if_idx = 0;
	}

	ctx = uci_alloc_context();
	if (!ctx) {
		printf("error in uci_alloc_context()\n");
		return -1;
	}

	uci_load(ctx, UCI_CFG_WIRELESS, &pkg);
	if (!pkg) {
		printf("error in uci_load()\n");
		ret = -1;
		goto out;
	}
	uci_foreach_element(&pkg->sections, e) {
		const char *opt_ifname, *opt_dev;
		struct uci_section *s   = uci_to_section(e);
		if (strcmp(s->type, "wifi-iface") != 0)
			continue;
		opt_ifname = uci_lookup_option_string(ctx, s, "ifname");

		/* convertion pricinple 2: OpenWrt default ifname */
		if (is_def_openwrt_ifname) {

			/* preparing the comparision */
			opt_dev = uci_lookup_option_string(ctx, s, "device");
			if (!opt_dev) { /* 'device' is mandatory option in wifi-iface */
				ret = -1;
				goto out;
			}
			if (strcmp(opt_dev, tgt_dev_name) == 0) {
				if (if_idx == tgt_if_idx) {
					/* matching OpenWrt default ifname */
					if (opt_ifname) {
						printf("Conflict naming: customized ifname '%s' conflict with OpenWrt default naming rules!\n", opt_ifname);
						ret = -1;
						goto out;
					}
					strncpy(section, s->e.name, len);
					ret = 0;
					goto out;
				}
				if_idx++;
			}
		}
		else {
			/* convertion pricinple 1: search 'ifname' in uci */

			if (opt_ifname) {
				if (strcmp(opt_ifname, ifname) == 0) {
					/* found given 'ifname' in 'wireless' uci */
					strncpy(section, s->e.name, len);
					goto out;
				}
				else {
					continue;
				}
			}
		}
	}
	ret = -1;

out:
	if (pkg)
		uci_unload(ctx, pkg);
	if (ctx)
		uci_free_context(ctx);

	return ret;
}


int cls_wifi_get_ap_ssid(const char *ifname, char *ret_value, const int ret_len);
int cls_wifi_set_ap_ssid(const char *ifname, const char *ssid);


/*
 * set SSID of AP
 * Returns:
 *   0:  Success
 *   !0: Errors
 */
int cls_wifi_set_ap_ssid(const char *ifname, const char *ssid)
{
	int ret = 0;

	char section[32];
	section[0] = 0;

	/* tr181 path to uci section & option */
	ret = cls_wifi_ifname_to_uci_section(ifname, section, 31);
	if (!ret)
		ret = cls_ubus_uci_set_and_commit(UCI_CFG_WIRELESS, section, "ssid", ssid);

	return ret;
}


/*
 * get SSID of AP
 * Returns:
 *   0:  Success
 *   !0: Errors
 */
int cls_wifi_get_ap_ssid(const char *ifname, char *ret_value, const int ret_len)
{
	int ret = 0;

	char section[32];
	section[0] = 0;

	ret = cls_wifi_ifname_to_uci_section(ifname, section, 31);
	if (!ret)
#ifdef USE_UBUS_UCI_GET
		ret = cls_ubus_uci_get(UCI_CFG_WIRELESS, section, "ssid", ret_value, ret_len);
#else
		ret = cls_uci_get(UCI_CFG_WIRELESS, section, "ssid", ret_value, ret_len);
#endif

	return ret;
}


#define CLS_SAMPLE_WIFI_SSID	"ssid"

#define CLS_SAMPLE_SSID_GET	"get"
#define CLS_SAMPLE_SSID_SET	"set"

void usage(void);
void usage(void)
{
	printf("Usage: demo_ubus_uci_wifi_cfg <ssid> <get|set> <ifname> [value]\n\n");
}


/*
* argv[0] = "cls_wifi_sample"
* argv[1] = "<ssid>"
* argv[2] = "get|set"
* argv[3] = "<interface>"
* argv[4] = "[value]"
*/
int main(int argc, char **argv)
{
	int ret = 0;
	char *ifname, *param, *ops, *value;
#define BUF_LEN		128
	char buf [BUF_LEN];

	if (argc < 4) {
		usage();
		return -1;
	}

	param  = argv[1];
	ops    = argv[2];
	ifname = argv[3];
	value  = argv[4];

	buf[0] = 0;

	if (strncmp(param, CLS_SAMPLE_WIFI_SSID, strlen(CLS_SAMPLE_WIFI_SSID)) == 0) {
		if (strncmp(ops, CLS_SAMPLE_SSID_SET, strlen(CLS_SAMPLE_SSID_SET)) == 0) {
			if (argc < 5) {
				usage();
				return -1;
			}
			ret = cls_wifi_set_ap_ssid(ifname, value);
		}
		else if (strncmp(ops, CLS_SAMPLE_SSID_GET, strlen(CLS_SAMPLE_SSID_GET)) == 0) {
			ret = cls_wifi_get_ap_ssid(ifname, buf, BUF_LEN);
			if (ret >= 0)
				printf("%s\n", buf);
		}
	} else {
		printf("Un-supported param!");
		return -1;
	}

	if (!ret)
		printf("Success!\n\n");
	else
		printf("Error: %d\n\n", ret);

	return ret;
}

