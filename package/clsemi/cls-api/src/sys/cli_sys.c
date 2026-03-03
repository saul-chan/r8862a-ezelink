/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#include "clsapi_sys.h"
#include "clsapi_cli.h"
#include "autogened_cli_sys.h"

static int cli_get_meminfo(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_meminfo info = {0};

	ret = clsapi_sys_get_meminfo(&info);
	if (ret < 0)
		return ret;

	cli_print(output, "total: %lu kB\n", info.total);
	cli_print(output, "free: %lu kB\n", info.free);
	cli_print(output, "shared: %lu kB\n", info.shared);
	cli_print(output, "buffered: %lu kB\n", info.buffered);
	cli_print(output, "available: %lu kB\n", info.available);
	cli_print(output, "cached: %lu kB\n", info.cached);

	return ret;
}

static int cli_get_storage(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_storage_info info = {0};

	ret = clsapi_sys_get_storage(&info);
	if (ret < 0)
		return ret;

	cli_print(output, "total: %lu kB\n", info.total);
	cli_print(output, "free: %lu kB\n", info.free);
	cli_print(output, "used: %lu kB\n", info.used);
	cli_print(output, "avail: %lu kB\n", info.avail);

	return ret;
}

static int cli_get_board_info(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	struct clsapi_board_info info = {0};

	ret = clsapi_sys_get_board_info(&info);
	if (ret < 0)
		return ret;

	cli_print(output, "kernel: %s\n", info.kernel);
	cli_print(output, "hostname: %s\n", info.hostname);
	cli_print(output, "system: %s\n", info.system);
	cli_print(output, "model: %s\n", info.model);
	cli_print(output, "board_name: %s\n", info.board_name);
	cli_print(output, "rootfs_type: %s\n", info.rootfs_type);
	cli_print(output, "release_distribution: %s\n", info.release_distribution);
	cli_print(output, "release_version: %s\n", info.release_version);
	cli_print(output, "release_revision: %s\n", info.release_revision);
	cli_print(output, "release_target: %s\n", info.release_target);
	cli_print(output, "release_description: %s\n", info.release_description);

	return ret;
}

static struct clsapi_cli_entry clsapi_cli_entry_sys_manual[] = {
	{"get meminfo",             0, 0, "", cli_get_meminfo},
	{"get storage",             0, 0, "", cli_get_storage},
	{"get board_info",          0, 0, "", cli_get_board_info},
	{}
};

int clsapi_cli_sys_init(struct clsapi_cli_entry *cli_entry_tbl[])
{
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_SYS_AUTO] = clsapi_cli_entry_sys_auto;
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_SYS_MANUAL] = clsapi_cli_entry_sys_manual;

	return CLSAPI_OK;
}


struct clsapi_cli_plugin clsapi_cli_plugin = {
	.init = clsapi_cli_sys_init
};

