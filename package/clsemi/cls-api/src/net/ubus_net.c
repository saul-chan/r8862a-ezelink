/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "autogened_ubus_net.h"
#include <rpcd/plugin.h>

enum {
	STRUCT_CLSAPI_NET_FIREWALL_RULE_NAME,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_IPADDR,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_MACADDR,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_PORT,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_PROTO,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_ICMP_TYPE,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_IPADDR,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_PORT,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_MARK,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_START_DATE,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_START_TIME,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_DATE,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_TIME,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_WEEKDAYS,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_MONTHDAYS,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_UTC_TIME,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_CLSAPI_FIREWALL_RULE_TARGET,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_FAMILY,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT_BURST,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_ENABLED,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_DEVICE,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_DIRECTION,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_DSCP,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_HELPER,
	STRUCT_CLSAPI_NET_FIREWALL_RULE_FLAG,
	__STRUCT_CLSAPI_NET_FIREWALL_RULE_MAX
};

static const struct blobmsg_policy struct_clsapi_net_firewall_rule_policy[__STRUCT_CLSAPI_NET_FIREWALL_RULE_MAX] = {
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC] = { .name = "src", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_IPADDR] = { .name = "src_ipaddr", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_MACADDR] = { .name = "src_macaddr", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_PORT] = { .name = "src_port", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_PROTO] = { .name = "proto", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_ICMP_TYPE] = { .name = "icmp_type", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST] = { .name = "dest", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_IPADDR] = { .name = "dest_ipaddr", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_PORT] = { .name = "dest_port", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_MARK] = { .name = "mark", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_START_DATE] = { .name = "start_date", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_START_TIME] = { .name = "start_time", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_DATE] = { .name = "stop_date", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_TIME] = { .name = "stop_time", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_WEEKDAYS] = { .name = "weekdays", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_MONTHDAYS] = { .name = "monthdays", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_UTC_TIME] = { .name = "utc_time", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_CLSAPI_FIREWALL_RULE_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION] = { .name = "target_option", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_FAMILY] = { .name = "family", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT] = { .name = "limit", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT_BURST] = { .name = "limit_burst", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_ENABLED] = { .name = "enabled", .type = BLOBMSG_TYPE_STRING},
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEVICE] = { .name = "device", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_DIRECTION] = { .name = "direction", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_DSCP] = { .name = "dscp", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_HELPER] = { .name = "helper", .type = BLOBMSG_TYPE_STRING },
	[STRUCT_CLSAPI_NET_FIREWALL_RULE_FLAG] = { .name = "flag", .type = BLOBMSG_TYPE_INT32 },
};

static inline int get_param_number(char *str)
{
	int count = 0;

	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] == ',') {
			count++;
		}
	}

	return count + 1;
}

enum {
	NET_SET_FIREWALL_RULE_RULE,
	__NET_SET_FIREWALL_RULE_MAX,
};

static const struct blobmsg_policy net_set_firewall_rule_policy[__NET_SET_FIREWALL_RULE_MAX] = {
	[NET_SET_FIREWALL_RULE_RULE] = { .name = "rule", .type = BLOBMSG_TYPE_TABLE },
};

static int ubus_net_set_firewall_rule(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_SET_FIREWALL_RULE_MAX];
	struct blob_attr *tb_struct_rule[__STRUCT_CLSAPI_NET_FIREWALL_RULE_MAX];
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;

	bool enabled = true;
	char *token = ",";
	char *tmp = NULL;
	int i = 0;
	string_128 rule_name = {0};
	string_128 str_enabled = {0};
	string_128 src = {0};
	string_128 str_src_ipaddr = {0};
	string_128 str_src_macaddr = {0};
	string_128 str_src_port = {0};
	string_128 str_proto = {0};
	string_128 dest = {0};
	string_128 str_dest_ipaddr = {0};
	string_128 str_dest_port = {0};
	string_128 target = {0};
	string_128 target_option = {0};

	bool utc_time = true;
	string_32 weekdays[7] = {0};
	string_32 monthdays[31] = {0};
	string_128 start_date = {0};
	string_128 start_time = {0};
	string_128 stop_date = {0};
	string_128 stop_time = {0};
	string_128 str_weekdays = {0};
	string_128 str_monthdays = {0};
	string_128 str_utc_time = {0};

	string_32 icmp_type[128] = {0};
	string_128 str_icmp_type = {0};
	string_128 family = {0};
	string_128 limit = {0};
	string_128 limit_burst = {0};
	string_128 device = {0};
	string_128 direction = {0};
	string_128 mark = {0};
	string_128 dscp = {0};
	string_128 helper = {0};

	blobmsg_parse(net_set_firewall_rule_policy, __NET_SET_FIREWALL_RULE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_SET_FIREWALL_RULE_RULE]) {
		ret = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	blobmsg_parse(struct_clsapi_net_firewall_rule_policy, __STRUCT_CLSAPI_NET_FIREWALL_RULE_MAX, tb_struct_rule, blobmsg_data(tb[NET_SET_FIREWALL_RULE_RULE]), blobmsg_len(tb[NET_SET_FIREWALL_RULE_RULE]));


	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_NAME]) {
		cls_strncpy(rule_name, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_NAME]), sizeof(rule_name));
		set_fw_rule_name(prule, rule_name);
	} else {
		ret = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_ENABLED]) {
		cls_strncpy(str_enabled, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_ENABLED]), sizeof(str_enabled));
		if (strcmp(str_enabled, "enable") == 0)
			enabled = true;
		else if (strcmp(str_enabled, "disable") == 0)
			enabled = false;
		else {
			ret = UBUS_STATUS_INVALID_ARGUMENT;
			goto out;
		}
		set_fw_rule_enabled(prule, enabled);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC]) {
		cls_strncpy(src, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC]), sizeof(src));
		set_fw_rule_src(prule, src);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_CLSAPI_FIREWALL_RULE_TARGET]) {
		cls_strncpy(target, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_CLSAPI_FIREWALL_RULE_TARGET]), sizeof(target));
		if (strcmp(target, "ACCEPT") == 0)
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_ACCEPT);
		else if (strcmp(target, "REJECT") == 0)
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_REJECT);
		else if (strcmp(target, "DROP") == 0)
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_DROP);
		else if (strcmp(target, "NOTRACK") == 0)
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_NOTRACK);
		//need extra option
		else if (strcmp(target, "MARK") == 0) {
			uint64_t set_mark = 0;

			if (!tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION])	{
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(target_option, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]), sizeof(target_option));
			if (target_option[0] == '0' && (target_option[1] == 'x' || target_option[1] == 'X'))
				set_mark = strtol(target_option, NULL, 16);
			else
				set_mark = strtol(target_option, NULL, 10);
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_MARK);
			prule->target_option.set_mark = set_mark;
		} else if (strcmp(target, "XMARK") == 0) {
			uint64_t set_xmark = 0;

			if (!tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(target_option, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]), sizeof(target_option));
			if (target_option[0] == '0' && (target_option[1] == 'x' || target_option[1] == 'X'))
				set_xmark = strtol(target_option, NULL, 16);
			else
				set_xmark = strtol(target_option, NULL, 10);
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_XMARK);
			prule->target_option.set_xmark = set_xmark;
		} else if (strcmp(target, "HELPER") == 0) {
			if (!tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(target_option, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]), sizeof(target_option));
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_HELPER);
			cls_strncpy(prule->target_option.set_helper, target_option, sizeof(prule->target_option.set_helper));
		} else if (strcmp(target, "DSCP") == 0) {
			if (!tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(target_option, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_TARGET_OPTION]), sizeof(target_option));
			set_fw_rule_target(prule, CLSAPI_FIREWALL_TARGET_DSCP);
			cls_strncpy(prule->target_option.set_dscp, target_option, sizeof(prule->target_option.set_dscp));
		} else {
			ret = UBUS_STATUS_INVALID_ARGUMENT;
			goto out;
		}
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_IPADDR]) {
		cls_strncpy(str_src_ipaddr, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_IPADDR]), sizeof(str_src_ipaddr));
		prule->src_ipaddr_num = get_param_number(str_src_ipaddr);
		prule->src_ipaddr = (struct in_addr *)calloc(sizeof(struct in_addr), prule->src_ipaddr_num);
		tmp = strtok(str_src_ipaddr, token);
		i = 0;
		while (tmp != NULL) {
			if(!inet_pton(AF_INET, tmp, &prule->src_ipaddr[i])) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			tmp = strtok(NULL, token);
			i++;
		}
		prule->flag |= (1 << 2);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_MACADDR]) {
		cls_strncpy(str_src_macaddr, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_MACADDR]), sizeof(str_src_macaddr));
		prule->src_macaddr_num = get_param_number(str_src_macaddr);
		prule->src_macaddr = (uint8_t (*)[ETH_ALEN])calloc(sizeof(uint8_t [ETH_ALEN]), prule->src_macaddr_num);
		tmp = strtok(str_src_macaddr, token);
		i = 0;
		while (tmp != NULL) {
			mac_aton(tmp, prule->src_macaddr[i]);
			tmp = strtok(NULL, token);
			i++;
		}
		prule->flag |= (1 << 3);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_PORT]) {
		cls_strncpy(str_src_port, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_SRC_PORT]), sizeof(str_src_port));
		prule->src_port_num = get_param_number(str_src_port);
		prule->src_port = (uint32_t *)calloc(sizeof(uint32_t), prule->src_port_num);
		tmp = strtok(str_src_port, token);
		i = 0;
		while (tmp != NULL) {
			if ((prule->src_port[i] = atoi(tmp)) == 0) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			tmp = strtok(NULL, token);
			i++;
		}
		prule->flag |= (1 << 4);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_PROTO]) {
		cls_strncpy(str_proto, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_PROTO]), sizeof(str_proto));
		prule->proto_num = get_param_number(str_proto);
		prule->proto = (string_32 *)calloc(sizeof(string_32), prule->proto_num);
		tmp = strtok(str_proto, token);
		i = 0;
		while (tmp != NULL) {
			cls_strncpy(prule->proto[i], tmp, sizeof(string_32));
			tmp = strtok(NULL, token);
			i++;
		}
		prule->flag |= (1 << 5);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST]) {
		cls_strncpy(dest, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST]), sizeof(dest));
		set_fw_rule_dest(prule, dest);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_PORT]) {
		cls_strncpy(str_dest_port, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_PORT]), sizeof(str_dest_port));
		prule->dest_port_num = get_param_number(str_dest_port);
		prule->dest_port = (uint32_t *)calloc(sizeof(uint32_t), prule->dest_port_num);
		tmp = strtok(str_dest_port, token);
		i = 0;
		while (tmp != NULL) {
			if ((prule->dest_port[i] = atoi(tmp)) == 0) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			tmp = strtok(NULL, token);
			i++;
		}
		prule->flag |= (1 << 9);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_IPADDR]) {
		cls_strncpy(str_dest_ipaddr, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEST_IPADDR]), sizeof(str_dest_ipaddr));
		prule->dest_ipaddr_num = get_param_number(str_dest_ipaddr);
		prule->dest_ipaddr = (struct in_addr *)calloc(sizeof(struct in_addr), prule->dest_ipaddr_num);

		tmp = strtok(str_dest_ipaddr, token);
		i = 0;
		while (tmp != NULL) {
			if(!inet_pton(AF_INET, tmp, &prule->dest_ipaddr[i])) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			tmp = strtok(NULL, token);
			i++;
		}
		prule->flag |= (1 << 8);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_WEEKDAYS]) {
		cls_strncpy(str_weekdays, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_WEEKDAYS]), sizeof(str_weekdays));
		tmp = strtok(str_weekdays, token);
		i = 0;
		while (tmp != NULL) {
			if (i >= ARRAY_SIZE(weekdays)) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(weekdays[i], tmp, sizeof(weekdays[i]));
			tmp = strtok(NULL, token);
			i++;
		}
		set_fw_rule_weekdays(prule, weekdays);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_MONTHDAYS]) {
		cls_strncpy(str_monthdays, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_MONTHDAYS]), sizeof(str_monthdays));
		tmp = strtok(str_monthdays, token);
		i = 0;
		while (tmp != NULL) {
			if (i >= ARRAY_SIZE(monthdays)) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(monthdays[i], tmp, sizeof(monthdays[i]));
			tmp = strtok(NULL, token);
			i++;
		}
		set_fw_rule_monthdays(prule, monthdays);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_UTC_TIME]) {
		cls_strncpy(str_utc_time, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_UTC_TIME]), sizeof(str_utc_time));
		if (strcmp(str_utc_time, "enable") == 0)
			utc_time = true;
		else if (strcmp(str_utc_time, "disable") == 0)
			utc_time = false;
		else {
			ret = UBUS_STATUS_INVALID_ARGUMENT;
			goto out;
		}

		set_fw_rule_utc_time(prule, utc_time);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_START_DATE]) {
		cls_strncpy(start_date, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_START_DATE]), sizeof(start_date));
		set_fw_rule_start_date(prule, start_date);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_START_TIME]) {
		cls_strncpy(start_time, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_START_TIME]), sizeof(start_time));
		set_fw_rule_start_time(prule, start_time);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_DATE]) {
		cls_strncpy(stop_date, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_DATE]), sizeof(stop_date));
		set_fw_rule_stop_date(prule, stop_date);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_TIME]) {
		cls_strncpy(stop_time, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_STOP_TIME]), sizeof(stop_time));
		set_fw_rule_stop_time(prule, stop_time);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_ICMP_TYPE]) {
		cls_strncpy(str_icmp_type, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_ICMP_TYPE]), sizeof(str_icmp_type));
		tmp = strtok(str_icmp_type, token);
		i = 0;
		while (tmp != NULL) {
			if (i >= ARRAY_SIZE(icmp_type)) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			cls_strncpy(icmp_type[i], tmp, sizeof(icmp_type[i]));
			tmp = strtok(NULL, token);
			i++;
		}
		set_fw_rule_icmp_type(prule, icmp_type);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_FAMILY]) {
		cls_strncpy(family, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_FAMILY]), sizeof(family));
		set_fw_rule_family(prule, family);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT]) {
		cls_strncpy(limit, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT]), sizeof(limit));
		set_fw_rule_limit(prule, limit);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT_BURST]) {
		cls_strncpy(limit_burst, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_LIMIT_BURST]), sizeof(limit_burst));
		if (atoi(limit_burst)) {
			if (!prule->limit) {
				ret = UBUS_STATUS_INVALID_ARGUMENT;
				goto out;
			}
			set_fw_rule_limit_burst(prule, atoi(limit_burst));
		} else {
			ret = UBUS_STATUS_INVALID_ARGUMENT;
			goto out;
		}
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DIRECTION]) {
		cls_strncpy(direction, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DIRECTION]), sizeof(direction));
		if (strcmp(direction, "\0") == 0) {
			ret = UBUS_STATUS_INVALID_ARGUMENT;
			goto out;
		}
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEVICE]) {
		cls_strncpy(device, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DEVICE]), sizeof(device));
		set_fw_rule_device(prule, device);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_MARK]) {
		cls_strncpy(mark, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_MARK]), sizeof(mark));
		set_fw_rule_mark(prule, mark);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DSCP]) {
		cls_strncpy(dscp, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_DSCP]), sizeof(dscp));
		set_fw_rule_dscp(prule, dscp);
	}

	if (tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_HELPER]) {
		cls_strncpy(helper, blobmsg_get_string(tb_struct_rule[STRUCT_CLSAPI_NET_FIREWALL_RULE_HELPER]), sizeof(helper));
		set_fw_rule_helper(prule, helper);
	}

	ret = clsapi_net_set_firewall_rule(prule);

out:
	if (prule->src_ipaddr)
		free(prule->src_ipaddr);
	if (prule->src_macaddr)
		free(prule->src_macaddr);
	if (prule->src_port)
		free(prule->src_port);
	if (prule->proto)
		free(prule->proto);
	if (prule->dest_port)
		free(prule->dest_port);
	if (prule->dest_ipaddr)
		free(prule->dest_ipaddr);

	return ret;
}

enum {
	NET_DEL_FIREWALL_RULE_RULE,
	__NET_DEL_FIREWALL_RULE_MAX,
};

static const struct blobmsg_policy net_del_firewall_rule_policy[__NET_DEL_FIREWALL_RULE_MAX] = {
	[NET_DEL_FIREWALL_RULE_RULE] = { .name = "rule_name", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_del_firewall_rule(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_DEL_FIREWALL_RULE_MAX];
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;

	blobmsg_parse(net_del_firewall_rule_policy, __NET_DEL_FIREWALL_RULE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_FIREWALL_RULE_RULE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	set_fw_rule_name(prule, blobmsg_get_string(tb[NET_DEL_FIREWALL_RULE_RULE]));
	ret = clsapi_net_del_firewall_rule(prule);

	return -ret;
}

enum {
	NET_GET_FIREWALL_RULE_RULE,
	__NET_GET_FIREWALL_RULE_MAX,
};

static const struct blobmsg_policy net_get_firewall_rule_policy[__NET_GET_FIREWALL_RULE_MAX] = {
	[NET_GET_FIREWALL_RULE_RULE] = { .name = "rule_name", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_get_firewall_rule(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_GET_FIREWALL_RULE_MAX];
	struct clsapi_net_firewall_rule rule = {0};
	struct clsapi_net_firewall_rule *prule = &rule;
	string_32 str_macaddr = {0};

	blobmsg_parse(net_get_firewall_rule_policy, __NET_GET_FIREWALL_RULE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_FIREWALL_RULE_RULE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	set_fw_rule_name(prule, blobmsg_get_string(tb[NET_GET_FIREWALL_RULE_RULE]));

	ret = clsapi_net_get_firewall_rule(prule);

	blob_buf_init(&buf, 0);
	//src
	if (prule->flag & 0x2)
	{
		blobmsg_add_string(&buf, "src", prule->src);
	}

	// src_ipaddr
	if (prule->flag & 0x4) {
		string_32 ipaddr = {0};

		for (int i = 0; i < prule->src_ipaddr_num; i++) {
			inet_ntop(AF_INET, &prule->src_ipaddr[i], ipaddr, sizeof(ipaddr));
			blobmsg_add_string(&buf, "src ipaddr", ipaddr);
		}
		free(prule->src_ipaddr);
	}

	// src_macaddr
	if (prule->flag & 0x8) {
		void *ptr_mac_list = blobmsg_open_array(&buf, "MAC list");

		for (int i = 0; i < prule->src_macaddr_num; i++) {
			snprintf(str_macaddr, sizeof(str_macaddr), MACFMT, MACARG(prule->src_macaddr[i]));
			blobmsg_add_string(&buf, "MAC", str_macaddr);
		}
		blobmsg_close_array(&buf, ptr_mac_list);
		free(prule->src_macaddr);
	}

	//src port
	if (prule->flag & 0x10) {
		string_32 port = {0};

		void *ptr_src_port_list = blobmsg_open_array(&buf, "src port");
		for (int i = 0; i < prule->src_port_num; i++) {
			snprintf(port, sizeof(port), "%d", prule->src_port[i]);
			blobmsg_add_string(&buf, "", port);
		}
		blobmsg_close_array(&buf, ptr_src_port_list);
		free(prule->src_port);
	}

	// protocol
	if (prule->flag & 0x20) {

		void *ptr_protocol_list = blobmsg_open_array(&buf, "protocol");
		for (int i = 0; i < prule->proto_num; i++)
			blobmsg_add_string(&buf, "", prule->proto[i]);

		blobmsg_close_array(&buf, ptr_protocol_list);
		free(prule->proto);
	}

	//icmp type
	if (prule->flag & 0x40) {
		void *ptr_icmp_list = blobmsg_open_array(&buf, "icmp");

		for (int i = 0; i < ARRAY_SIZE(prule->icmp_type); i++) {
			if (strcmp(prule->icmp_type[i], "\0") != 0)
			{
				blobmsg_add_string(&buf, "icmp type", prule->icmp_type[i]);
			}
		}
		blobmsg_close_array(&buf, ptr_icmp_list);
	}

	//dest
	if (prule->flag & 0x80)
	{
		blobmsg_add_string(&buf, "dest", prule->dest);
	}

	//dest ipaddr
	if (prule->flag & 0x100) {
		string_32 ipaddr = {0};

		void *ptr_dest_ipaddr_list = blobmsg_open_array(&buf, "dest_ip");
		for (int i = 0; i < prule->dest_ipaddr_num; i++) {
			inet_ntop(AF_INET, &prule->dest_ipaddr[i], ipaddr, sizeof(ipaddr));
			blobmsg_add_string(&buf, "dest ipaddr", ipaddr);
		}
		blobmsg_close_array(&buf, ptr_dest_ipaddr_list);
		free(prule->dest_ipaddr);
	}
	//dest_port
	if (prule->flag & 0x200) {
		string_32 port = {0};

		void *ptr_dest_port_list = blobmsg_open_array(&buf, "dest_port");
		for (int i = 0; i < prule->dest_port_num; i++) {
			snprintf(port, sizeof(port), "%d", prule->dest_port[i]);
			blobmsg_add_string(&buf, "dest port", port);
		}
		blobmsg_close_array(&buf, ptr_dest_port_list);
		free(prule->dest_port);
	}

	//mark
	if (prule->flag & 0x400)
	{
		blobmsg_add_string(&buf, "mark", prule->mark);
	}

	//start date
	if (prule->flag & 0x800)
	{
		blobmsg_add_string(&buf, "start_date", prule->start_date);
	}

	//start time
	if (prule->flag & 0x1000)
	{
		blobmsg_add_string(&buf, "start_time", prule->start_time);
	}

	//stop date
	if (prule->flag & 0x2000)
	{
		blobmsg_add_string(&buf, "stop_date", prule->stop_date);
	}

	//stop time
	if (prule->flag & 0x4000)
	{
		blobmsg_add_string(&buf, "stop_time", prule->stop_time);
	}

	//weekdays
	if (prule->flag & 0x8000) {
		void *ptr_weekdays_list = blobmsg_open_array(&buf, "weekdays");

		for (int i = 0; i < ARRAY_SIZE(prule->weekdays); i++) {
			if (strcmp(prule->weekdays[i], "\0") != 0)
				blobmsg_add_string(&buf, "", prule->weekdays[i]);
		}
		blobmsg_close_array(&buf, ptr_weekdays_list);
	}
	//monthdays
	if (prule->flag & 0x10000) {
		void *ptr_monthdays_list = blobmsg_open_array(&buf, "monthdays");
		for (int i = 0; i < ARRAY_SIZE(prule->monthdays); i++) {
			if (strcmp(prule->monthdays[i], "\0") != 0)
				blobmsg_add_string(&buf, "", prule->monthdays[i]);
		}
		blobmsg_close_array(&buf, ptr_monthdays_list);
	}

	//utc time
	if (prule->flag & 0x20000) {
		if (prule->utc_time)
			blobmsg_add_string(&buf, "utc_time", "enabled");
		else
			blobmsg_add_string(&buf, "utc_time", "disabled");
	}

	//target
	if (prule->flag & 0x40000) {
		string_512 dummy = {0};

		switch (prule->target) {
			case CLSAPI_FIREWALL_TARGET_ACCEPT:
				strcpy(dummy, "target: ACCEPT\n");
				break;
			case CLSAPI_FIREWALL_TARGET_DROP:
				strcpy(dummy, "target: DROP\n");
				break;
			case CLSAPI_FIREWALL_TARGET_REJECT:
				strcpy(dummy, "target: REJECT\n");
				break;
			case CLSAPI_FIREWALL_TARGET_NOTRACK:
				strcpy(dummy, "target: NOTRACK\n");
				break;
			case CLSAPI_FIREWALL_TARGET_XMARK:
				snprintf(dummy, sizeof(dummy), "target XMARK: %ld", prule->target_option.set_xmark);
				break;
			case CLSAPI_FIREWALL_TARGET_MARK:
				snprintf(dummy, sizeof(dummy), "target MARK: %ld", prule->target_option.set_mark);
				break;
			case CLSAPI_FIREWALL_TARGET_HELPER:
				snprintf(dummy, sizeof(dummy), "target HELPER: %s", prule->target_option.set_helper);
				break;
			case CLSAPI_FIREWALL_TARGET_DSCP:
				snprintf(dummy, sizeof(dummy), "target DSCP: %s\n", prule->target_option.set_dscp);
				break;
			default:
				break;
		}
		blobmsg_add_string(&buf, "target option", dummy);
	}

	//family
	if (prule->flag & 0x80000)
	{
		blobmsg_add_string(&buf, "family", prule->family);
	}

	//limit
	if (prule->flag & 0x100000)
	{
		blobmsg_add_string(&buf, "limit", prule->limit);
	}

	//limit burst
	if (prule->flag & 0x200000) {
		string_32 dummy = {0};

		snprintf(dummy, sizeof(dummy), "%d", prule->limit_burst);
		blobmsg_add_string(&buf, "limit", dummy);
	}

	//enabled
	if (prule->flag & 0x400000) {
		if (prule->enabled)
			blobmsg_add_string(&buf, "", "enabled");
		else
			blobmsg_add_string(&buf, "", "disabled");
	}

	//device + (direction)
	if (prule->flag & 0x800000) {
		blobmsg_add_string(&buf, "device", prule->device);
		blobmsg_add_string(&buf, "direction", prule->direction);
	}

	// dscp
	if (prule->flag & 0x1000000)
	{
		blobmsg_add_string(&buf, "dscp", prule->dscp);
	}

	// helper
	if (prule->flag & 0x2000000)
	{
		blobmsg_add_string(&buf, "helper", prule->helper);
	}

	ret = ubus_send_reply(ctx, req, buf.head);

	return ret;
}

enum {
	NET_GET_NETMASK_NETIF_NAME,
	__NET_GET_NETMASK_MAX,
};

static const struct blobmsg_policy net_get_netmask_policy[__NET_GET_NETMASK_MAX] = {
	[NET_GET_NETMASK_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_netmask(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_GET_NETMASK_MAX];
	string_32 str_netmask;
	struct in_addr netmask;

	blobmsg_parse(net_get_netmask_policy, __NET_GET_NETMASK_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_NETMASK_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_netmask(blobmsg_get_string(tb[NET_GET_NETMASK_NETIF_NAME]), &netmask);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		inet_ntop(AF_INET, &netmask, str_netmask, sizeof(str_netmask));
		blobmsg_add_string(&buf, "netmask", str_netmask);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_SET_NETMASK_NETIF_NAME,
	NET_SET_NETMASK_NETMASK,
	__NET_SET_NETMASK_MAX,
};

static const struct blobmsg_policy net_set_netmask_policy[__NET_SET_NETMASK_MAX] = {
	[NET_SET_NETMASK_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
	[NET_SET_NETMASK_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_set_netmask(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_SET_NETMASK_MAX];
	struct in_addr netmask;

	blobmsg_parse(net_set_netmask_policy, __NET_SET_NETMASK_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_SET_NETMASK_NETIF_NAME] || !tb[NET_SET_NETMASK_NETMASK])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_SET_NETMASK_NETMASK]), &netmask);
	ret = clsapi_net_set_netmask(blobmsg_get_string(tb[NET_SET_NETMASK_NETIF_NAME]), &netmask);

	return -ret;
}

enum {
	NET_SET_IPADDR_NETIF_NAME,
	NET_SET_IPADDR_IPADDR,
	__NET_SET_IPADDR_MAX,
};

static const struct blobmsg_policy net_set_ipaddr_policy[__NET_SET_IPADDR_MAX] = {
	[NET_SET_IPADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
	[NET_SET_IPADDR_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_set_ipaddr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_SET_IPADDR_MAX];
	struct in_addr ipaddr;

	blobmsg_parse(net_set_ipaddr_policy, __NET_SET_IPADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_SET_IPADDR_NETIF_NAME] || !tb[NET_SET_IPADDR_IPADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (!inet_pton(AF_INET, blobmsg_get_string(tb[NET_SET_IPADDR_IPADDR]), &ipaddr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_set_ipaddr(blobmsg_get_string(tb[NET_SET_IPADDR_NETIF_NAME]), &ipaddr);

	return -ret;
}

enum {
	NET_GET_MACADDR_NETIF_NAME,
	__NET_GET_MACADDR_MAX,
};

static const struct blobmsg_policy net_get_macaddr_policy[__NET_GET_MACADDR_MAX] = {
	[NET_GET_MACADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_macaddr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	uint8_t macaddr[ETH_ALEN];
	string_32 str_macaddr = {0};
	struct blob_attr *tb[__NET_GET_MACADDR_MAX];

	blobmsg_parse(net_get_macaddr_policy, __NET_GET_MACADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_MACADDR_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_macaddr(blobmsg_get_string(tb[NET_GET_MACADDR_NETIF_NAME]), macaddr);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		snprintf(str_macaddr, sizeof(str_macaddr), MACFMT, MACARG(macaddr));
		blobmsg_add_string(&buf, "MAC", str_macaddr);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_GET_IPV6_DEFAULT_GATEWAY_NETIF_NAME,
	__NET_GET_IPV6_DEFAULT_GATEWAY_MAX,
};

static const struct blobmsg_policy net_get_ipv6_default_gateway_policy[__NET_GET_IPV6_DEFAULT_GATEWAY_MAX] = {
	[NET_GET_IPV6_DEFAULT_GATEWAY_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_ipv6_default_gateway(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in6_addr ipaddr;
	string_128 str_ipv6addr = {0};
	struct blob_attr *tb[__NET_GET_IPV6_DEFAULT_GATEWAY_MAX];

	blobmsg_parse(net_get_ipv6_default_gateway_policy, __NET_GET_IPV6_DEFAULT_GATEWAY_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_IPV6_DEFAULT_GATEWAY_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_ipv6_default_gateway(blobmsg_get_string(tb[NET_GET_IPV6_DEFAULT_GATEWAY_NETIF_NAME]), &ipaddr);
	if (ret != 0)
		return UBUS_STATUS_INVALID_COMMAND;

	inet_ntop(AF_INET6, &ipaddr, str_ipv6addr, sizeof(str_ipv6addr));
	blob_buf_init(&buf, 0);
	blobmsg_add_string(&buf, "IPv6", str_ipv6addr);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_GET_UNI_GLOBAL_IPV6PREFIX_NETIF_NAME,
	__NET_GET_UNI_GLOBAL_IPV6PREFIX_MAX,
};

static const struct blobmsg_policy net_get_uni_global_ipv6prefix_policy[__NET_GET_UNI_GLOBAL_IPV6PREFIX_MAX] = {
	[NET_GET_UNI_GLOBAL_IPV6PREFIX_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_uni_global_ipv6prefix(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	string_256 ipv6prefix = {0};
	uint8_t ipv6prefix_len = 128;
	struct blob_attr *tb[__NET_GET_UNI_GLOBAL_IPV6PREFIX_MAX];

	blobmsg_parse(net_get_uni_global_ipv6prefix_policy, __NET_GET_UNI_GLOBAL_IPV6PREFIX_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_UNI_GLOBAL_IPV6PREFIX_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_uni_global_ipv6prefix(blobmsg_get_string(tb[NET_GET_UNI_GLOBAL_IPV6PREFIX_NETIF_NAME]), ipv6prefix, &ipv6prefix_len);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		blobmsg_add_string(&buf, "prefix", ipv6prefix);
		blobmsg_add_u32(&buf, "prefix length", ipv6prefix_len);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_GET_ULA_GLOBAL_IPV6PREFIX_NETIF_NAME,
	__NET_GET_ULA_GLOBAL_IPV6PREFIX_MAX,
};

static const struct blobmsg_policy net_get_ula_global_ipv6prefix_policy[__NET_GET_ULA_GLOBAL_IPV6PREFIX_MAX] = {
	[NET_GET_ULA_GLOBAL_IPV6PREFIX_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_ula_global_ipv6prefix(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	string_256 ipv6prefix = {0};
	uint8_t ipv6prefix_len = 128;
	struct blob_attr *tb[__NET_GET_ULA_GLOBAL_IPV6PREFIX_MAX];

	blobmsg_parse(net_get_ula_global_ipv6prefix_policy, __NET_GET_ULA_GLOBAL_IPV6PREFIX_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_ULA_GLOBAL_IPV6PREFIX_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_ula_global_ipv6prefix(blobmsg_get_string(tb[NET_GET_ULA_GLOBAL_IPV6PREFIX_NETIF_NAME]), ipv6prefix, &ipv6prefix_len);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		blobmsg_add_string(&buf, "prefix", ipv6prefix);
		blobmsg_add_u32(&buf, "prefix length", ipv6prefix_len);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_SET_DNS_DOMAIN_BINDING_DOMAIN_NAME,
	NET_SET_DNS_DOMAIN_BINDING_IPADDR,
	__NET_SET_DNS_DOMAIN_BINDING_MAX,
};

static const struct blobmsg_policy net_set_dns_domain_binding_policy[__NET_SET_DNS_DOMAIN_BINDING_MAX] = {
	[NET_SET_DNS_DOMAIN_BINDING_DOMAIN_NAME] = { .name = "domain_name", .type = BLOBMSG_TYPE_STRING },
	[NET_SET_DNS_DOMAIN_BINDING_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_set_dns_domain_binding(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_SET_DNS_DOMAIN_BINDING_MAX];
	struct in_addr ipaddr;

	blobmsg_parse(net_set_dns_domain_binding_policy, __NET_SET_DNS_DOMAIN_BINDING_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_SET_DNS_DOMAIN_BINDING_DOMAIN_NAME] || !tb[NET_SET_DNS_DOMAIN_BINDING_IPADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_SET_DNS_DOMAIN_BINDING_IPADDR]), &ipaddr);
	ret = clsapi_net_set_dns_domain_binding(blobmsg_get_string(tb[NET_SET_DNS_DOMAIN_BINDING_DOMAIN_NAME]), &ipaddr);

	return -ret;
}

enum {
	NET_GET_DNS_DOMAIN_BINDING_DOMAIN_NAME,
	__NET_GET_DNS_DOMAIN_BINDING_MAX,
};

static const struct blobmsg_policy net_get_dns_domain_binding_policy[__NET_GET_DNS_DOMAIN_BINDING_MAX] = {
	[NET_GET_DNS_DOMAIN_BINDING_DOMAIN_NAME] = { .name = "domain_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_dns_domain_binding(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr ipaddr;
	string_128 str_ipaddr = {0};
	struct blob_attr *tb[__NET_GET_DNS_DOMAIN_BINDING_MAX];

	blobmsg_parse(net_get_dns_domain_binding_policy, __NET_GET_DNS_DOMAIN_BINDING_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_DNS_DOMAIN_BINDING_DOMAIN_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_dns_domain_binding(blobmsg_get_string(tb[NET_GET_DNS_DOMAIN_BINDING_DOMAIN_NAME]), &ipaddr);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		inet_ntop(AF_INET, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
		blobmsg_add_string(&buf, "ipaddr", str_ipaddr);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_DEL_DNS_DOMAIN_BINDING_DOMAIN_NAME,
	__NET_DEL_DNS_DOMAIN_BINDING_MAX,
};

static const struct blobmsg_policy net_del_dns_domain_binding_policy[__NET_DEL_DNS_DOMAIN_BINDING_MAX] = {
	[NET_DEL_DNS_DOMAIN_BINDING_DOMAIN_NAME] = { .name = "domain_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_del_dns_domain_binding(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_DEL_DNS_DOMAIN_BINDING_MAX];

	blobmsg_parse(net_del_dns_domain_binding_policy, __NET_DEL_DNS_DOMAIN_BINDING_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_DNS_DOMAIN_BINDING_DOMAIN_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_del_dns_domain_binding(blobmsg_get_string(tb[NET_DEL_DNS_DOMAIN_BINDING_DOMAIN_NAME]));

	return -ret;
}

enum {
	NET_ADD_DNS_DOMAIN_BINDING_DOMAIN_NAME,
	NET_ADD_DNS_DOMAIN_BINDING_IPADDR,
	__NET_ADD_DNS_DOMAIN_BINDING_MAX,
};

static const struct blobmsg_policy net_add_dns_domain_binding_policy[__NET_ADD_DNS_DOMAIN_BINDING_MAX] = {
	[NET_ADD_DNS_DOMAIN_BINDING_DOMAIN_NAME] = { .name = "domain_name", .type = BLOBMSG_TYPE_STRING },
	[NET_ADD_DNS_DOMAIN_BINDING_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_add_dns_domain_binding(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr ipaddr;
	struct blob_attr *tb[__NET_ADD_DNS_DOMAIN_BINDING_MAX];

	blobmsg_parse(net_add_dns_domain_binding_policy, __NET_ADD_DNS_DOMAIN_BINDING_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_ADD_DNS_DOMAIN_BINDING_DOMAIN_NAME] || !tb[NET_ADD_DNS_DOMAIN_BINDING_IPADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_ADD_DNS_DOMAIN_BINDING_IPADDR]), &ipaddr);
	ret = clsapi_net_add_dns_domain_binding(blobmsg_get_string(tb[NET_ADD_DNS_DOMAIN_BINDING_DOMAIN_NAME]), &ipaddr);

	return -ret;
}

enum {
	NET_ADD_STATIC_ROUTE_INTERFACE_NAME,
	NET_ADD_STATIC_ROUTE_IPADDR,
	NET_ADD_STATIC_ROUTE_NETMASK,
	__NET_ADD_STATIC_ROUTE_MAX,
};

static const struct blobmsg_policy net_add_static_route_policy[__NET_ADD_STATIC_ROUTE_MAX] = {
	[NET_ADD_STATIC_ROUTE_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_ADD_STATIC_ROUTE_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
	[NET_ADD_STATIC_ROUTE_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_add_static_route(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr ipaddr;
	struct in_addr netmask;
	struct blob_attr *tb[__NET_ADD_STATIC_ROUTE_MAX];

	blobmsg_parse(net_add_static_route_policy, __NET_ADD_STATIC_ROUTE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_ADD_STATIC_ROUTE_INTERFACE_NAME] || !tb[NET_ADD_STATIC_ROUTE_IPADDR] || !tb[NET_ADD_STATIC_ROUTE_NETMASK])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_ADD_STATIC_ROUTE_IPADDR]), &ipaddr);
	inet_pton(AF_INET, blobmsg_get_string(tb[NET_ADD_STATIC_ROUTE_NETMASK]), &netmask);

	ret = clsapi_net_add_static_route(blobmsg_get_string(tb[NET_ADD_STATIC_ROUTE_INTERFACE_NAME]), &ipaddr, &netmask);

	return -ret;
}

enum {
	NET_DEL_STATIC_ROUTE_INTERFACE_NAME,
	NET_DEL_STATIC_ROUTE_IPADDR,
	NET_DEL_STATIC_ROUTE_NETMASK,
	__NET_DEL_STATIC_ROUTE_MAX,
};

static const struct blobmsg_policy net_del_static_route_policy[__NET_DEL_STATIC_ROUTE_MAX] = {
	[NET_DEL_STATIC_ROUTE_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_STATIC_ROUTE_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_STATIC_ROUTE_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_del_static_route(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_DEL_STATIC_ROUTE_MAX];
	struct in_addr ipaddr;
	struct in_addr netmask;

	blobmsg_parse(net_del_static_route_policy, __NET_DEL_STATIC_ROUTE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_STATIC_ROUTE_INTERFACE_NAME] || !tb[NET_DEL_STATIC_ROUTE_IPADDR] || !tb[NET_DEL_STATIC_ROUTE_NETMASK])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_DEL_STATIC_ROUTE_IPADDR]), &ipaddr);
	inet_pton(AF_INET, blobmsg_get_string(tb[NET_DEL_STATIC_ROUTE_NETMASK]), &netmask);

	ret = clsapi_net_del_static_route(blobmsg_get_string(tb[NET_DEL_STATIC_ROUTE_INTERFACE_NAME]), &ipaddr, &netmask);

	return -ret;
}

enum {
	NET_GET_STATIC_GATEWAY_INTERFACE_NAME,
	NET_GET_STATIC_GATEWAY_IPADDR,
	NET_GET_STATIC_GATEWAY_NETMASK,
	__NET_GET_STATIC_GATEWAY_MAX,
};

static const struct blobmsg_policy net_get_static_route_gateway_policy[__NET_GET_STATIC_GATEWAY_MAX] = {
	[NET_GET_STATIC_GATEWAY_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_GET_STATIC_GATEWAY_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
	[NET_GET_STATIC_GATEWAY_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_get_static_route_gateway(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_GET_STATIC_GATEWAY_MAX];
	string_32 str_gateway = {0};
	struct in_addr ipaddr;
	struct in_addr netmask;
	struct in_addr gateway;

	blobmsg_parse(net_get_static_route_gateway_policy, __NET_GET_STATIC_GATEWAY_MAX,
			tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_STATIC_GATEWAY_INTERFACE_NAME] || !tb[NET_GET_STATIC_GATEWAY_IPADDR] ||
			!tb[NET_GET_STATIC_GATEWAY_NETMASK])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_GET_STATIC_GATEWAY_IPADDR]), &ipaddr);
	inet_pton(AF_INET, blobmsg_get_string(tb[NET_GET_STATIC_GATEWAY_NETMASK]), &netmask);

	ret = clsapi_net_get_static_route_gateway(blobmsg_get_string(tb[NET_GET_STATIC_GATEWAY_INTERFACE_NAME]),
			&ipaddr, &netmask, &gateway);
	if (ret == 0) {
		inet_ntop(AF_INET,  &gateway, str_gateway, sizeof(string_32));
		blob_buf_init(&buf, 0);
		blobmsg_add_string(&buf, "gateway", str_gateway);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_SET_STATIC_GATEWAY_INTERFACE_NAME,
	NET_SET_STATIC_GATEWAY_IPADDR,
	NET_SET_STATIC_GATEWAY_NETMASK,
	NET_SET_STATIC_GATEWAY_GATEWAY,
	__NET_SET_STATIC_GATEWAY_MAX,
};

static const struct blobmsg_policy net_set_static_route_gateway_policy[__NET_SET_STATIC_GATEWAY_MAX] = {
	[NET_SET_STATIC_GATEWAY_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_SET_STATIC_GATEWAY_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
	[NET_SET_STATIC_GATEWAY_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING},
	[NET_SET_STATIC_GATEWAY_GATEWAY] = { .name = "gateway", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_set_static_route_gateway(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr ipaddr;
	struct in_addr gateway;
	struct in_addr netmask;
	struct blob_attr *tb[__NET_SET_STATIC_GATEWAY_MAX];

	blobmsg_parse(net_set_static_route_gateway_policy, __NET_SET_STATIC_GATEWAY_MAX,
			tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_SET_STATIC_GATEWAY_INTERFACE_NAME] || !tb[NET_SET_STATIC_GATEWAY_IPADDR] ||
			!tb[NET_SET_STATIC_GATEWAY_NETMASK] || !tb[NET_SET_STATIC_GATEWAY_GATEWAY])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_SET_STATIC_GATEWAY_IPADDR]), &ipaddr);
	inet_pton(AF_INET, blobmsg_get_string(tb[NET_SET_STATIC_GATEWAY_NETMASK]), &netmask);
	inet_pton(AF_INET, blobmsg_get_string(tb[NET_SET_STATIC_GATEWAY_GATEWAY]), &gateway);

	ret = clsapi_net_set_static_route_gateway(blobmsg_get_string(tb[NET_SET_STATIC_GATEWAY_INTERFACE_NAME]),
			&ipaddr, &netmask, &gateway);

	return -ret;
}

enum {
	NET_DEL_STATIC_GATEWAY_INTERFACE_NAME,
	NET_DEL_STATIC_GATEWAY_IPADDR,
	NET_DEL_STATIC_GATEWAY_NETMASK,
	__NET_DEL_STATIC_GATEWAY_MAX,
};

static const struct blobmsg_policy net_del_static_route_gateway_policy[__NET_DEL_STATIC_GATEWAY_MAX] = {
	[NET_DEL_STATIC_GATEWAY_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_STATIC_GATEWAY_IPADDR] = { .name = "ipaddr", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_STATIC_GATEWAY_NETMASK] = { .name = "netmask", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_del_static_route_gateway(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_DEL_STATIC_GATEWAY_MAX];
	struct in_addr ipaddr;
	struct in_addr netmask;

	blobmsg_parse(net_del_static_route_gateway_policy, __NET_DEL_STATIC_GATEWAY_MAX, tb,
			blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_STATIC_GATEWAY_INTERFACE_NAME] || !tb[NET_DEL_STATIC_GATEWAY_IPADDR] ||
			!tb[NET_DEL_STATIC_GATEWAY_NETMASK])
		return UBUS_STATUS_INVALID_ARGUMENT;

	inet_pton(AF_INET, blobmsg_get_string(tb[NET_DEL_STATIC_GATEWAY_IPADDR]), &ipaddr);
	inet_pton(AF_INET, blobmsg_get_string(tb[NET_DEL_STATIC_GATEWAY_NETMASK]), &netmask);

	ret = clsapi_net_del_static_route_gateway(blobmsg_get_string(tb[NET_DEL_STATIC_GATEWAY_INTERFACE_NAME]),
			&ipaddr, &netmask);

	return -ret;
}

enum {
	NET_GET_IPADDR_NETIF_NAME,
	__NET_GET_IPADDR_MAX,
};

static const struct blobmsg_policy net_get_ipaddr_policy[__NET_GET_IPADDR_MAX] = {
	[NET_GET_IPADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_ipaddr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_GET_IPADDR_MAX];
	struct in_addr ipaddr;
	string_128 str_ipaddr = {0};

	blobmsg_parse(net_get_ipaddr_policy, __NET_GET_IPADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_IPADDR_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_ipaddr(blobmsg_get_string(tb[NET_GET_IPADDR_NETIF_NAME]), &ipaddr);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		inet_ntop(AF_INET, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
		blobmsg_add_string(&buf, "ipaddr", str_ipaddr);
		ret = ubus_send_reply(ctx, req, buf.head);
	}

	return -ret;
}

enum {
	NET_GET_DEFAULT_GATEWAY_NETIF_NAME,
	__NET_GET_DEFAULT_GATEWAY_MAX,
};

static const struct blobmsg_policy net_get_default_gateway_policy[__NET_GET_DEFAULT_GATEWAY_MAX] = {
	[NET_GET_DEFAULT_GATEWAY_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_default_gateway(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr ipaddr;
	string_256 str_ipaddr = {0};
	struct blob_attr *tb[__NET_GET_DEFAULT_GATEWAY_MAX];

	blobmsg_parse(net_get_default_gateway_policy, __NET_GET_DEFAULT_GATEWAY_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_DEFAULT_GATEWAY_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_default_gateway(blobmsg_get_string(tb[NET_GET_DEFAULT_GATEWAY_NETIF_NAME]), &ipaddr);
	if (ret != 0)
		return UBUS_STATUS_INVALID_COMMAND;

	inet_ntop(AF_INET, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
	blob_buf_init(&buf, 0);
	blobmsg_add_string(&buf, "IPv4", str_ipaddr);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_GET_ALL_IPV6ADDRS_NETIF_NAME,
	__NET_GET_ALL_IPV6ADDRS_MAX,
};

static const struct blobmsg_policy net_get_all_ipv6addrs_policy[__NET_GET_ALL_IPV6ADDRS_MAX] = {
	[NET_GET_ALL_IPV6ADDRS_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_all_ipv6addrs(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int addr_len = 0;
	int ret = CLSAPI_OK;
	string_128 str_ipv6_addr = {0};
	struct clsapi_ipv6_info *ipv6_addrs;
	struct blob_attr *tb[__NET_GET_ALL_IPV6ADDRS_MAX];

	blobmsg_parse(net_get_all_ipv6addrs_policy, __NET_GET_ALL_IPV6ADDRS_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_ALL_IPV6ADDRS_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_all_ipv6addrs(blobmsg_get_string(tb[NET_GET_ALL_IPV6ADDRS_NETIF_NAME]), &ipv6_addrs);
	if (ret > 0)
		addr_len = ret;

	blob_buf_init(&buf, 0);
	void *ptr_v6addrs_array = blobmsg_open_array(&buf, "IPv6 addr");

	for (int i = 0; i < addr_len; i++) {
		inet_ntop(AF_INET6, &ipv6_addrs[i], str_ipv6_addr, sizeof(str_ipv6_addr));
		blobmsg_add_string(&buf, "Addr", str_ipv6_addr);
	}

	blobmsg_close_array(&buf, ptr_v6addrs_array);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_GET_IPV6ADDR_NETIF_NAME,
	__NET_GET_IPV6ADDR_MAX,
};

static const struct blobmsg_policy net_get_ipv6addr_policy[__NET_GET_IPV6ADDR_MAX] = {
	[NET_GET_IPV6ADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_ipv6addr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in6_addr ipaddr;
	string_128 str_ipaddr = {0};
	struct blob_attr *tb[__NET_GET_IPV6ADDR_MAX];

	blobmsg_parse(net_get_ipv6addr_policy, __NET_GET_IPV6ADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_IPV6ADDR_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_ipv6addr(blobmsg_get_string(tb[NET_GET_IPV6ADDR_NETIF_NAME]), &ipaddr);
	if (ret == 0) {
		blob_buf_init(&buf, 0);
		inet_ntop(AF_INET6, &ipaddr, str_ipaddr, sizeof(str_ipaddr));
		blobmsg_add_string(&buf, "result", str_ipaddr);
	}

	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

static int ubus_net_get_firewall_zone_names(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	string_128 zone_names[128];
	int zone_names_len = 128;

	ret = clsapi_net_get_firewall_zone_names(zone_names, &zone_names_len);
	if (ret)
		return -ret;

	blob_buf_init(&buf, 0);
	void *ptr_zone_name = blobmsg_open_array(&buf, "zone_name");

	for (int i = 0; i < zone_names_len; i++)
		blobmsg_add_string(&buf, "zone", zone_names[i]);

	blobmsg_close_array(&buf, ptr_zone_name);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_DEL_DNS_SERVER_ADDR_NETIF_NAME,
	NET_DEL_DNS_SERVER_ADDR_SERVER_ADDR,
	__NET_DEL_DNS_SERVER_ADDR_MAX,
};

static const struct blobmsg_policy net_del_dns_server_addr_policy[__NET_DEL_DNS_SERVER_ADDR_MAX] = {
	[NET_DEL_DNS_SERVER_ADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_DNS_SERVER_ADDR_SERVER_ADDR] = { .name = "server_addr", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_del_dns_server_addr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr addr;
	struct blob_attr *tb[__NET_DEL_DNS_SERVER_ADDR_MAX];

	blobmsg_parse(net_del_dns_server_addr_policy, __NET_DEL_DNS_SERVER_ADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_DNS_SERVER_ADDR_NETIF_NAME] || !tb[NET_DEL_DNS_SERVER_ADDR_SERVER_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (!inet_pton(AF_INET, blobmsg_get_string(tb[NET_DEL_DNS_SERVER_ADDR_SERVER_ADDR]), &addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_del_dns_server_addr(blobmsg_get_string(tb[NET_DEL_DNS_SERVER_ADDR_NETIF_NAME]), &addr);

	return -ret;
}

enum {
	NET_GET_DNS_SERVER_ADDR_NETIF_NAME,
	__NET_GET_DNS_SERVER_ADDR_MAX,
};

static const struct blobmsg_policy net_get_dns_server_addr_policy[__NET_GET_DNS_SERVER_ADDR_MAX] = {
	[NET_GET_DNS_SERVER_ADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_dns_server_addr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	int server_addr_len = 128;
	struct in_addr *server_addr;
	struct blob_attr *tb[__NET_GET_DNS_SERVER_ADDR_MAX];

	blobmsg_parse(net_get_dns_server_addr_policy, __NET_GET_DNS_SERVER_ADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_DNS_SERVER_ADDR_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_dns_server_addr(blobmsg_get_string(tb[NET_GET_DNS_SERVER_ADDR_NETIF_NAME]), &server_addr);
	if (ret > 0)
		server_addr_len = ret;
	else
		return UBUS_STATUS_INVALID_COMMAND;

	blob_buf_init(&buf, 0);
	void *ptr_server_addrs = blobmsg_open_array(&buf, "addrs");

	for (int i = 0; i < server_addr_len; i++)
		blobmsg_add_string(&buf, "server addr", inet_ntoa(server_addr[i]));

	blobmsg_close_array(&buf, ptr_server_addrs);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_ADD_DNS_SERVER_ADDR_NETIF_NAME,
	NET_ADD_DNS_SERVER_ADDR_SERVER_ADDR,
	__NET_ADD_DNS_SERVER_ADDR_MAX,
};

static const struct blobmsg_policy net_add_dns_server_addr_policy[__NET_ADD_DNS_SERVER_ADDR_MAX] = {
	[NET_ADD_DNS_SERVER_ADDR_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
	[NET_ADD_DNS_SERVER_ADDR_SERVER_ADDR] = { .name = "server_addr", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_add_dns_server_addr(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct in_addr addr;
	struct blob_attr *tb[__NET_ADD_DNS_SERVER_ADDR_MAX];

	blobmsg_parse(net_add_dns_server_addr_policy, __NET_ADD_DNS_SERVER_ADDR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_ADD_DNS_SERVER_ADDR_NETIF_NAME] || !tb[NET_ADD_DNS_SERVER_ADDR_SERVER_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (!inet_pton(AF_INET, blobmsg_get_string(tb[NET_ADD_DNS_SERVER_ADDR_SERVER_ADDR]), &addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_add_dns_server_addr(blobmsg_get_string(tb[NET_ADD_DNS_SERVER_ADDR_NETIF_NAME]), &addr);

	return -ret;
}

enum {
	NET_GET_DHCP_ADDR_POOL_NETIF_NAME,
	__NET_GET_DHCP_ADDR_POOL_MAX,
};

static const struct blobmsg_policy net_get_dhcp_addr_pool_policy[__NET_GET_DHCP_ADDR_POOL_MAX] = {
	[NET_GET_DHCP_ADDR_POOL_NETIF_NAME] = { .name = "netif_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_dhcp_addr_pool(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	uint32_t max_leases = 0;
	uint32_t start_offset = 0;
	struct blob_attr *tb[__NET_GET_DHCP_ADDR_POOL_MAX];

	blobmsg_parse(net_get_dhcp_addr_pool_policy, __NET_GET_DHCP_ADDR_POOL_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_DHCP_ADDR_POOL_NETIF_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_dhcp_addr_pool(blobmsg_get_string(tb[NET_GET_DHCP_ADDR_POOL_NETIF_NAME]), &start_offset, &max_leases);
	if (ret)
		return -ret;

	blob_buf_init(&buf, 0);
	blobmsg_add_u32(&buf, "start offset", start_offset);
	blobmsg_add_u32(&buf, "max leases", max_leases);

	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_GET_WAN_DNS_SERVER_INTERFACE_NAME,
	__NET_GET_WAN_DNS_SERVER_MAX,
};

static const struct blobmsg_policy net_get_wan_dns_server_policy[__NET_GET_WAN_DNS_SERVER_MAX] = {
	[NET_GET_WAN_DNS_SERVER_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_wan_dns_server(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int len = 0;
	int ret = CLSAPI_OK;
	string_256 str_address = {0};
	struct clsapi_net_ipaddr *dns_server;
	struct blob_attr *tb[__NET_GET_WAN_DNS_SERVER_MAX];

	blobmsg_parse(net_get_wan_dns_server_policy, __NET_GET_WAN_DNS_SERVER_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_WAN_DNS_SERVER_INTERFACE_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_wan_dns_server(blobmsg_get_string(tb[NET_GET_WAN_DNS_SERVER_INTERFACE_NAME]), &dns_server);

	if (ret > 0)
		len = ret;
	else
		return -ret;

	blob_buf_init(&buf, 0);
	void *ptr_dns_server = blobmsg_open_table(&buf, "dns_server");

	for (int i = 0; i < len; i++) {
		if (dns_server[i].in_family == AF_INET)
			inet_ntop(AF_INET, &(dns_server[i].addr.ipaddr), str_address, sizeof(str_address));
		else if (dns_server[i].in_family == AF_INET6)
			inet_ntop(AF_INET6, &(dns_server[i].addr.ipv6addr), str_address, sizeof(str_address));
		else {
			if (dns_server)
				free(dns_server);
			return UBUS_STATUS_NO_DATA;
		}

		blobmsg_add_string(&buf, "IP address", str_address);
	}

	blobmsg_close_table(&buf, ptr_dns_server);

	if (dns_server)
		free(dns_server);

	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}


enum {
	NET_DEL_WAN_DNS_SERVER_INTERFACE_NAME,
	NET_DEL_WAN_DNS_SERVER_DNS_SERVER,
	__NET_DEL_WAN_DNS_SERVER_MAX,
};

static const struct blobmsg_policy net_del_wan_dns_server_policy[__NET_DEL_WAN_DNS_SERVER_MAX] = {
	[NET_DEL_WAN_DNS_SERVER_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_WAN_DNS_SERVER_DNS_SERVER] = { .name = "dns_server", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_del_wan_dns_server(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	string_128 str_ipaddr = {0};
	struct clsapi_net_ipaddr dns_server;
	struct blob_attr *tb[__NET_DEL_WAN_DNS_SERVER_MAX];

	blobmsg_parse(net_del_wan_dns_server_policy, __NET_DEL_WAN_DNS_SERVER_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_WAN_DNS_SERVER_INTERFACE_NAME] || !tb[NET_DEL_WAN_DNS_SERVER_DNS_SERVER])
		return UBUS_STATUS_INVALID_ARGUMENT;

	cls_strncpy(str_ipaddr, blobmsg_get_string(tb[NET_DEL_WAN_DNS_SERVER_DNS_SERVER]), sizeof(str_ipaddr));

	if (inet_pton(AF_INET, str_ipaddr, &dns_server.v4_addr))
		dns_server.in_family =  AF_INET;
	else if (inet_pton(AF_INET6, str_ipaddr, &dns_server.v6_addr))
		dns_server.in_family =  AF_INET6;
	else
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_del_wan_dns_server(blobmsg_get_string(tb[NET_DEL_WAN_DNS_SERVER_INTERFACE_NAME]), &dns_server);

	return -ret;
}

enum {
	NET_ADD_WAN_DNS_SERVER_INTERFACE_NAME,
	NET_ADD_WAN_DNS_SERVER_DNS_SERVER,
	__NET_ADD_WAN_DNS_SERVER_MAX,
};

static const struct blobmsg_policy net_add_wan_dns_server_policy[__NET_ADD_WAN_DNS_SERVER_MAX] = {
	[NET_ADD_WAN_DNS_SERVER_INTERFACE_NAME] = { .name = "interface_name", .type = BLOBMSG_TYPE_STRING },
	[NET_ADD_WAN_DNS_SERVER_DNS_SERVER] = { .name = "dns_server", .type = BLOBMSG_TYPE_STRING},
};

static int ubus_net_add_wan_dns_server(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_DEL_WAN_DNS_SERVER_MAX];
	string_128 str_ipaddr = {0};

	struct clsapi_net_ipaddr dns_server;

	blobmsg_parse(net_del_wan_dns_server_policy, __NET_DEL_WAN_DNS_SERVER_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_WAN_DNS_SERVER_INTERFACE_NAME] || !tb[NET_ADD_WAN_DNS_SERVER_DNS_SERVER])
		return UBUS_STATUS_INVALID_ARGUMENT;

	cls_strncpy(str_ipaddr, blobmsg_get_string(tb[NET_ADD_WAN_DNS_SERVER_DNS_SERVER]), sizeof(str_ipaddr));

	if (inet_pton(AF_INET, str_ipaddr, &dns_server.v4_addr))
			dns_server.in_family =  AF_INET;
	else if (inet_pton(AF_INET6, str_ipaddr, &dns_server.v6_addr))
			dns_server.in_family =  AF_INET6;
	else
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_add_wan_dns_server(blobmsg_get_string(tb[NET_DEL_WAN_DNS_SERVER_INTERFACE_NAME]), &dns_server);

	return -ret;
}

enum {
	NET_GET_FIREWALL_ZONE_NETWORK_ZONE_NAME,
	__NET_GET_FIREWALL_ZONE_NETWORK_MAX,
};

static const struct blobmsg_policy net_get_firewall_zone_network_policy[__NET_GET_FIREWALL_ZONE_NETWORK_MAX] = {
	[NET_GET_FIREWALL_ZONE_NETWORK_ZONE_NAME] = { .name = "zone_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_get_firewall_zone_network(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	int network_name_len = 0;
	clsapi_ifname *network_name = NULL;
	struct blob_attr *tb[__NET_GET_FIREWALL_ZONE_NETWORK_MAX];

	blobmsg_parse(net_get_firewall_zone_network_policy, __NET_GET_FIREWALL_ZONE_NETWORK_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_GET_FIREWALL_ZONE_NETWORK_ZONE_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_get_firewall_zone_network(blobmsg_get_string(tb[NET_GET_FIREWALL_ZONE_NETWORK_ZONE_NAME]), &network_name);
	if (ret > 0)
		network_name_len = ret;
	else
		return -ret;

	blob_buf_init(&buf, 0);
	void *ptr_network = blobmsg_open_array(&buf, "network_name");

	for (int i = 0; i < network_name_len; i++)
		blobmsg_add_string(&buf, NULL, network_name[i]);

	blobmsg_close_array(&buf, ptr_network);
	free(network_name);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}


static int ubus_net_get_dhcp_lease_info(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct dhcp_lease_info info[256];
	int info_len = 128;
	time_t timestamp = 0;
	string_32 str_macaddr = {0};

	ret = clsapi_net_get_dhcp_lease_info(info, &info_len);

	blob_buf_init(&buf, 0);
	void *ptr_info_array = blobmsg_open_array(&buf, "lease info");

	for (int i = 0; i < info_len; i++) {
		void *ptr_info_tab = blobmsg_open_table(&buf, "");

		blobmsg_add_string(&buf, "Hostname", info[i].hostname);
		blobmsg_add_string(&buf, "IP", inet_ntoa(info[i].ipaddr));
		snprintf(str_macaddr, sizeof(str_macaddr), MACFMT, MACARG(info[i].macaddr));
		blobmsg_add_string(&buf, "MAC", str_macaddr);
		timestamp = info[i].expires;
		blobmsg_add_string(&buf, "Expired", ctime(&timestamp));
		blobmsg_close_table(&buf, ptr_info_tab);
	}
	blobmsg_close_array(&buf, ptr_info_array);
	ret = ubus_send_reply(ctx, req, buf.head);

	return -ret;
}

enum {
	NET_DEL_FIREWALL_ZONE_NETWORK_ZONE_NAME,
	NET_DEL_FIREWALL_ZONE_NETWORK_NETWORK_NAME,
	__NET_DEL_FIREWALL_ZONE_NETWORK_MAX,
};

static const struct blobmsg_policy net_del_firewall_zone_network_policy[__NET_DEL_FIREWALL_ZONE_NETWORK_MAX] = {
	[NET_DEL_FIREWALL_ZONE_NETWORK_ZONE_NAME] = { .name = "zone_name", .type = BLOBMSG_TYPE_STRING },
	[NET_DEL_FIREWALL_ZONE_NETWORK_NETWORK_NAME] = { .name = "network_name", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_net_del_firewall_zone_network(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__NET_DEL_FIREWALL_ZONE_NETWORK_MAX];

	blobmsg_parse(net_del_firewall_zone_network_policy, __NET_DEL_FIREWALL_ZONE_NETWORK_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NET_DEL_FIREWALL_ZONE_NETWORK_ZONE_NAME] || !tb[NET_DEL_FIREWALL_ZONE_NETWORK_NETWORK_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_net_del_firewall_zone_network(blobmsg_get_string(tb[NET_DEL_FIREWALL_ZONE_NETWORK_ZONE_NAME]), blobmsg_get_string(tb[NET_DEL_FIREWALL_ZONE_NETWORK_NETWORK_NAME]));

	return -ret;
}

/****************	Generate automatically, DO NOT CHANGE	***************/
struct ubus_method clsapi_net_methods[] = {
	UBUS_METHOD_NOARG("get_opmode",	ubus_net_get_opmode),
	UBUS_METHOD("set_opmode",	ubus_net_set_opmode,	net_set_opmode_policy),
	UBUS_METHOD("get_macaddr",	ubus_net_get_macaddr,	net_get_macaddr_policy),
	UBUS_METHOD("get_speed",	ubus_net_get_speed,	net_get_speed_policy),
	UBUS_METHOD("get_stats",	ubus_net_get_stats,	net_get_stats_policy),
	UBUS_METHOD("get_proto",	ubus_net_get_proto,	net_get_proto_policy),
	UBUS_METHOD("set_proto",	ubus_net_set_proto,	net_set_proto_policy),
	UBUS_METHOD("get_ipaddr",	ubus_net_get_ipaddr,	net_get_ipaddr_policy),
	UBUS_METHOD("get_ipv6addr",	ubus_net_get_ipv6addr,	net_get_ipv6addr_policy),
	UBUS_METHOD("set_ipaddr",	ubus_net_set_ipaddr,	net_set_ipaddr_policy),
	UBUS_METHOD("get_netmask",	ubus_net_get_netmask,	net_get_netmask_policy),
	UBUS_METHOD("set_netmask",	ubus_net_set_netmask,	net_set_netmask_policy),
	UBUS_METHOD("del_bridge",	ubus_net_del_bridge,	net_del_bridge_policy),
	UBUS_METHOD("add_bridge",	ubus_net_add_bridge,	net_add_bridge_policy),
	UBUS_METHOD("add_bridge_port",	ubus_net_add_bridge_port,	net_add_bridge_port_policy),
	UBUS_METHOD("del_bridge_port",	ubus_net_del_bridge_port,	net_del_bridge_port_policy),
	UBUS_METHOD("add_interface",	ubus_net_add_interface,	net_add_interface_policy),
	UBUS_METHOD("del_interface",	ubus_net_del_interface,	net_del_interface_policy),
	UBUS_METHOD("get_dhcp_leasetime",	ubus_net_get_dhcp_leasetime,	net_get_dhcp_leasetime_policy),
	UBUS_METHOD("set_dhcp_leasetime",	ubus_net_set_dhcp_leasetime,	net_set_dhcp_leasetime_policy),
	UBUS_METHOD("enable_dhcp_server",	ubus_net_enable_dhcp_server,	net_enable_dhcp_server_policy),
	UBUS_METHOD("get_dhcp_server_enabled",	ubus_net_get_dhcp_server_enabled,	net_get_dhcp_server_enabled_policy),
	UBUS_METHOD("get_dhcp_addr_pool",	ubus_net_get_dhcp_addr_pool,	net_get_dhcp_addr_pool_policy),
	UBUS_METHOD("set_dhcp_addr_pool",	ubus_net_set_dhcp_addr_pool,	net_set_dhcp_addr_pool_policy),
	UBUS_METHOD("set_firewall_default_policy",	ubus_net_set_firewall_default_policy,	net_set_firewall_default_policy_policy),
	UBUS_METHOD("get_firewall_default_policy",	ubus_net_get_firewall_default_policy,	net_get_firewall_default_policy_policy),
	UBUS_METHOD("add_static_route",	ubus_net_add_static_route,	net_add_static_route_policy),
	UBUS_METHOD("del_static_route",	ubus_net_del_static_route,	net_del_static_route_policy),
	UBUS_METHOD("get_static_route_gateway",	ubus_net_get_static_route_gateway,
			net_get_static_route_gateway_policy),
	UBUS_METHOD("set_static_route_gateway",	ubus_net_set_static_route_gateway,
			net_set_static_route_gateway_policy),
	UBUS_METHOD("del_static_route_gateway",	ubus_net_del_static_route_gateway,
			net_del_static_route_gateway_policy),
	UBUS_METHOD("add_firewall_zone",	ubus_net_add_firewall_zone,	net_add_firewall_zone_policy),
	UBUS_METHOD("del_firewall_zone",	ubus_net_del_firewall_zone,	net_del_firewall_zone_policy),
	UBUS_METHOD_NOARG("get_dns_domain_suffix",	ubus_net_get_dns_domain_suffix),
	UBUS_METHOD("set_dns_domain_suffix",	ubus_net_set_dns_domain_suffix,	net_set_dns_domain_suffix_policy),
	UBUS_METHOD("add_dns_domain_binding",	ubus_net_add_dns_domain_binding,	net_add_dns_domain_binding_policy),
	UBUS_METHOD("set_dns_domain_binding",	ubus_net_set_dns_domain_binding,	net_set_dns_domain_binding_policy),
	UBUS_METHOD("get_dns_domain_binding",	ubus_net_get_dns_domain_binding,	net_get_dns_domain_binding_policy),
	UBUS_METHOD("del_dns_domain_binding",	ubus_net_del_dns_domain_binding,	net_del_dns_domain_binding_policy),
	UBUS_METHOD("get_dns_server_addr",	ubus_net_get_dns_server_addr,	net_get_dns_server_addr_policy),
	UBUS_METHOD("add_dns_server_addr",	ubus_net_add_dns_server_addr,	net_add_dns_server_addr_policy),
	UBUS_METHOD("del_dns_server_addr",	ubus_net_del_dns_server_addr,	net_del_dns_server_addr_policy),
	UBUS_METHOD_NOARG("get_dhcp_lease_info",	ubus_net_get_dhcp_lease_info),
	UBUS_METHOD_NOARG("get_firewall_zone_names",	ubus_net_get_firewall_zone_names),
	UBUS_METHOD("add_firewall_zone_forwarding",	ubus_net_add_firewall_zone_forwarding,	net_add_firewall_zone_forwarding_policy),
	UBUS_METHOD("del_firewall_zone_forwarding",	ubus_net_del_firewall_zone_forwarding,	net_del_firewall_zone_forwarding_policy),
	UBUS_METHOD("set_firewall_zone_policy",	ubus_net_set_firewall_zone_policy,	net_set_firewall_zone_policy_policy),
	UBUS_METHOD("get_firewall_zone_policy",	ubus_net_get_firewall_zone_policy,	net_get_firewall_zone_policy_policy),
	UBUS_METHOD("add_firewall_zone_network",	ubus_net_add_firewall_zone_network,	net_add_firewall_zone_network_policy),
	UBUS_METHOD("del_firewall_zone_network",	ubus_net_del_firewall_zone_network,	net_del_firewall_zone_network_policy),
	UBUS_METHOD("get_firewall_zone_network",	ubus_net_get_firewall_zone_network,	net_get_firewall_zone_network_policy),
	UBUS_METHOD("add_wan_dns_server",	ubus_net_add_wan_dns_server,	net_add_wan_dns_server_policy),
	UBUS_METHOD("del_wan_dns_server",	ubus_net_del_wan_dns_server,	net_del_wan_dns_server_policy),
	UBUS_METHOD("get_wan_dns_server",	ubus_net_get_wan_dns_server,	net_get_wan_dns_server_policy),
	UBUS_METHOD("get_pppoe_username",	ubus_net_get_pppoe_username,	net_get_pppoe_username_policy),
	UBUS_METHOD("set_pppoe_username",	ubus_net_set_pppoe_username,	net_set_pppoe_username_policy),
	UBUS_METHOD("get_pppoe_passwd",	ubus_net_get_pppoe_passwd,	net_get_pppoe_passwd_policy),
	UBUS_METHOD("set_pppoe_passwd",	ubus_net_set_pppoe_passwd,	net_set_pppoe_passwd_policy),
	UBUS_METHOD("get_uni_global_ipv6prefix",	ubus_net_get_uni_global_ipv6prefix,	net_get_uni_global_ipv6prefix_policy),
	UBUS_METHOD("get_ula_global_ipv6prefix",	ubus_net_get_ula_global_ipv6prefix,	net_get_ula_global_ipv6prefix_policy),
	UBUS_METHOD("get_all_ipv6addrs",	ubus_net_get_all_ipv6addrs,	net_get_all_ipv6addrs_policy),
	UBUS_METHOD("get_default_gateway",	ubus_net_get_default_gateway,	net_get_default_gateway_policy),
	UBUS_METHOD("get_ipv6_default_gateway",	ubus_net_get_ipv6_default_gateway,	net_get_ipv6_default_gateway_policy),
	UBUS_METHOD("get_firewall_rule",	ubus_net_get_firewall_rule,	net_get_firewall_rule_policy),
	UBUS_METHOD("set_firewall_rule",	ubus_net_set_firewall_rule,	net_set_firewall_rule_policy),
	UBUS_METHOD("del_firewall_rule",	ubus_net_del_firewall_rule,	net_del_firewall_rule_policy),
	UBUS_METHOD("get_ipv6ConnStatus",	ubus_net_get_ipv6ConnStatus,	net_get_ipv6ConnStatus_policy),
	UBUS_METHOD("get_ipv6prefixDelegationEnabled",	ubus_net_get_ipv6prefixDelegationEnabled,	net_get_ipv6prefixDelegationEnabled_policy),
	UBUS_METHOD("get_ipv6DSliteEnabled",	ubus_net_get_ipv6DSliteEnabled,	net_get_ipv6DSliteEnabled_policy),
	UBUS_METHOD("get_ipv6AddrAllocMethod",	ubus_net_get_ipv6AddrAllocMethod,	net_get_ipv6AddrAllocMethod_policy),
};

/****************	Network ubus object	*****************/

static struct ubus_object_type clsapi_net_type =
	UBUS_OBJECT_TYPE("clsapi", clsapi_net_methods);

static struct ubus_object clsapi_net_obj = {
	.name = "clsapi.net",
	.type = &clsapi_net_type,
	.methods = clsapi_net_methods,
	.n_methods = ARRAY_SIZE(clsapi_net_methods),
};

int clsapi_net_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	return ubus_add_object(ctx, &clsapi_net_obj);
}

struct rpc_plugin rpc_plugin = {
	.init = clsapi_net_init
};
