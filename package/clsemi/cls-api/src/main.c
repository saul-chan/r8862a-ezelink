/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "clsapi_util.h"
#include <rpcd/plugin.h>

/* location of clsapi plugins */
#ifndef CLSAPI_PLUGIN_DIR
#define CLSAPI_PLUGIN_DIR	"/usr/lib/clsapi/"
#endif

static LIST_HEAD(clsapi_plugins);

static int clsapi_plugin_register_library(struct ubus_context *ctx, const char *path)
{
	struct rpc_plugin *p;
	void *dlh;

	dlh = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
	if (!dlh) {
		DBG_ERROR("Failed to load plugin %s: %s\n", path, dlerror());
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
	p = dlsym(dlh, "rpc_plugin");
	if (!p)
		return UBUS_STATUS_NOT_FOUND;

	list_add(&p->list, &clsapi_plugins);

	return p->init(NULL, ctx);
}


int clsapi_plugin_init(struct ubus_context *ctx)
{
	DIR *d;
	int ret = 0;
	struct stat s;
	struct dirent *e;
	char path[PATH_MAX];

	if ((d = opendir(CLSAPI_PLUGIN_DIR)) != NULL) {
		while ((e = readdir(d)) != NULL) {
			snprintf(path, sizeof(path) - 1, CLSAPI_PLUGIN_DIR "/%s", e->d_name);
			if (stat(path, &s) || !S_ISREG(s.st_mode))
				continue;

			ret |= clsapi_plugin_register_library(ctx, path);
		}

		closedir(d);
	}

	return ret;
}

static struct ubus_context *ctx;

int main(int argc, char **argv)
{
	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		DBG_ERROR("Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	clsapi_plugin_init(ctx);

	uloop_run();
	ubus_free(ctx);
	uloop_done();

	return 0;
}
