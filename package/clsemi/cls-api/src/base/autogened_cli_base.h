/* Automatically generated file; DO NOT EDIT. */
/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _AUTOGENED_CLI_BASE_H
#define _AUTOGENED_CLI_BASE_H

#include <stdio.h>
#include <stdlib.h>

static int cli_get_defer_mode(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	int defer = 0;

	ret = clsapi_base_get_defer_mode(&defer);

	return clsapi_cli_report_int_value(ret, output, defer);
}

static int cli_set_defer_mode(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const int defer = atol(argv[0]);

	ret = clsapi_base_set_defer_mode(defer);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_conf_param(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *cfg = (argv[0]);
	const char *section = (argv[1]);
	const char *param = (argv[2]);
	char value[1025] = {0};

	ret = clsapi_base_get_conf_param(cfg, section, param, value);

	return clsapi_cli_report_str_value(ret, output, value);
}

static int cli_set_conf_param(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *cfg = (argv[0]);
	const char *section = (argv[1]);
	const char *param = (argv[2]);
	const char *value = (argv[3]);

	ret = clsapi_base_set_conf_param(cfg, section, param, value);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_set_apply_conf_param(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const char *cfg = (argv[0]);
	const char *section = (argv[1]);
	const char *param = (argv[2]);
	const char *value = (argv[3]);

	ret = clsapi_base_set_apply_conf_param(cfg, section, param, value);

	return clsapi_cli_report_complete(ret, output);
}


struct clsapi_cli_entry clsapi_cli_entry_base_auto[] = {
	{"get defer_mode", 0, 0, "", cli_get_defer_mode},
	{"set defer_mode", 1, 1, "<defer>", cli_set_defer_mode},
	{"get conf_param", 3, 3, "<cfg> <section> <param>", cli_get_conf_param},
	{"set conf_param", 4, 4, "<cfg> <section> <param> <value>", cli_set_conf_param},
	{"apply conf_cfg", 1, 1, "<cfg>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_base_apply_conf_cfg)},
	{"set apply_conf_param", 4, 4, "<cfg> <section> <param> <value>", cli_set_apply_conf_param},

	{}
};

#endif /* _AUTOGENED_CLI_BASE_H */