/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "autogened_ubus_sys.h"
#include <rpcd/plugin.h>

/****************	System ubus object	*****************/

enum {
	SYS_SET_SSH_PORT_IDX,
	SYS_SET_SSH_PORT_PORT,
	__SYS_SET_SSH_PORT_MAX,
};

static const struct blobmsg_policy sys_set_ssh_port_policy[__SYS_SET_SSH_PORT_MAX] = {
	[SYS_SET_SSH_PORT_IDX] = { .name = "idx", .type = BLOBMSG_TYPE_INT32 },
	[SYS_SET_SSH_PORT_PORT] = { .name = "port", .type = BLOBMSG_TYPE_INT32 },
};

static int ubus_sys_set_ssh_port(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req, const char *method,
					struct blob_attr *msg)
{
	int ret = CLSAPI_OK;
	struct blob_attr *tb[__SYS_SET_SSH_PORT_MAX];

	blobmsg_parse(sys_set_ssh_port_policy, __SYS_SET_SSH_PORT_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[SYS_SET_SSH_PORT_IDX] || !tb[SYS_SET_SSH_PORT_PORT])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_sys_set_ssh_port(blobmsg_get_u32(tb[SYS_SET_SSH_PORT_IDX]), blobmsg_get_u32(tb[SYS_SET_SSH_PORT_PORT]));

	return -ret;
}

/****************	Generate automatically, DO NOT CHANGE	***************/
struct ubus_method clsapi_sys_methods[] = {
};

/****************	System ubus object	*****************/

static struct ubus_object_type clsapi_sys_type =
	UBUS_OBJECT_TYPE("clsapi", clsapi_sys_methods);

static struct ubus_object clsapi_sys_obj = {
	.name = "clsapi.sys",
	.type = &clsapi_sys_type,
	.methods = clsapi_sys_methods,
	.n_methods = ARRAY_SIZE(clsapi_sys_methods),
};

int clsapi_sys_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	return ubus_add_object(ctx, &clsapi_sys_obj);
}

struct rpc_plugin rpc_plugin = {
	.init = clsapi_sys_init
};
