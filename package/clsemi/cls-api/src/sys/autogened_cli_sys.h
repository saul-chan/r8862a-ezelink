/* Automatically generated file; DO NOT EDIT. */
/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _AUTOGENED_CLI_SYS_H
#define _AUTOGENED_CLI_SYS_H

#include <stdio.h>
#include <stdlib.h>

static int cli_get_led(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t led_gpio = atol(argv[0]);
	bool onoff = 0;

	ret = clsapi_sys_get_led(led_gpio, &onoff);

	return clsapi_cli_report_int_value(ret, output, onoff);
}

static int cli_set_led(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t led_gpio = atol(argv[0]);
	const bool onoff = atol(argv[1]);

	ret = clsapi_sys_set_led(led_gpio, onoff);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_restore_factory(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;

	ret = clsapi_sys_restore_factory();

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_ssh_iface(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	char interface[33] = {0};

	ret = clsapi_sys_get_ssh_iface(idx, interface);

	return clsapi_cli_report_str_value(ret, output, interface);
}

static int cli_set_ssh_iface(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	const char *interface = (argv[1]);

	ret = clsapi_sys_set_ssh_iface(idx, interface);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_ssh_port(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	uint16_t port = 0;

	ret = clsapi_sys_get_ssh_port(idx, &port);

	return clsapi_cli_report_uint_value(ret, output, port);
}

static int cli_set_ssh_port(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	const uint16_t port = atol(argv[1]);

	ret = clsapi_sys_set_ssh_port(idx, port);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_ssh_password_auth(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	bool onoff = 0;

	ret = clsapi_sys_get_ssh_password_auth(idx, &onoff);

	return clsapi_cli_report_int_value(ret, output, onoff);
}

static int cli_set_ssh_password_auth(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	const bool onoff = atol(argv[1]);

	ret = clsapi_sys_set_ssh_password_auth(idx, onoff);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_ssh_root_login(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	bool onoff = 0;

	ret = clsapi_sys_get_ssh_root_login(idx, &onoff);

	return clsapi_cli_report_int_value(ret, output, onoff);
}

static int cli_set_ssh_root_login(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	const uint8_t idx = atol(argv[0]);
	const bool onoff = atol(argv[1]);

	ret = clsapi_sys_set_ssh_root_login(idx, onoff);

	return clsapi_cli_report_complete(ret, output);
}

static int cli_get_sys_uptime(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;
	uint64_t uptime = 0;

	ret = clsapi_sys_get_sys_uptime(&uptime);

	return clsapi_cli_report_ulong_value(ret, output, uptime);
}

static int cli_trigger_system_reboot(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;

	ret = clsapi_sys_trigger_system_reboot();

	return clsapi_cli_report_complete(ret, output);
}


struct clsapi_cli_entry clsapi_cli_entry_sys_auto[] = {
	{"get led", 1, 1, "<led_gpio>", cli_get_led},
	{"set led", 2, 2, "<led_gpio> <onoff>", cli_set_led},
	{"get hostname", 0, 0, "", cli_generic_get, C_API(clsapi_get_out_char1024, clsapi_sys_get_hostname)},
	{"set hostname", 1, 1, "<hostname>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_set_hostname)},
	{"restore factory", 0, 0, "", cli_restore_factory},
	{"get lang", 0, 0, "", cli_generic_get, C_API(clsapi_get_out_char1024, clsapi_sys_get_lang)},
	{"set lang", 1, 1, "<lang>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_set_lang)},
	{"get ssh_iface", 1, 1, "<idx>", cli_get_ssh_iface},
	{"set ssh_iface", 2, 2, "<idx> <interface>", cli_set_ssh_iface},
	{"get ssh_port", 1, 1, "<idx>", cli_get_ssh_port},
	{"set ssh_port", 2, 2, "<idx> <port>", cli_set_ssh_port},
	{"get ssh_password_auth", 1, 1, "<idx>", cli_get_ssh_password_auth},
	{"set ssh_password_auth", 2, 2, "<idx> <onoff>", cli_set_ssh_password_auth},
	{"get ssh_root_login", 1, 1, "<idx>", cli_get_ssh_root_login},
	{"set ssh_root_login", 2, 2, "<idx> <onoff>", cli_set_ssh_root_login},
	{"get sys_uptime", 0, 0, "", cli_get_sys_uptime},
	{"add backup_file", 1, 1, "<filepath>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_add_backup_file)},
	{"del backup_file", 1, 1, "<filepath>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_del_backup_file)},
	{"backup conf", 1, 1, "<filepath>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_backup_conf)},
	{"restore backup_conf", 1, 1, "<filepath>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_restore_backup_conf)},
	{"upgrade firmware", 1, 1, "<filepath>", cli_generic_set, C_API(clsapi_set_charptr, clsapi_sys_upgrade_firmware)},
	{"trigger system_reboot", 0, 0, "", cli_trigger_system_reboot},

	{}
};

#endif /* _AUTOGENED_CLI_SYS_H */