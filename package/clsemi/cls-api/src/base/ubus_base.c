/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "autogened_ubus_base.h"
#include <rpcd/plugin.h>

/****************	Generate automatically, DO NOT CHANGE	***************/
struct ubus_method clsapi_base_methods[] = {
};

/****************	Base ubus object	*****************/

static struct ubus_object_type clsapi_base_type =
	UBUS_OBJECT_TYPE("clsapi", clsapi_base_methods);

static struct ubus_object clsapi_base_obj = {
	.name = "clsapi.base",
	.type = &clsapi_base_type,
	.methods = clsapi_base_methods,
	.n_methods = ARRAY_SIZE(clsapi_base_methods),
};

int clsapi_base_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	return ubus_add_object(ctx, &clsapi_base_obj);
}

struct rpc_plugin rpc_plugin = {
	.init = clsapi_base_init
};
