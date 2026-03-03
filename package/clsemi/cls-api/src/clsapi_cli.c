/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <sys/prctl.h>
#include "clsapi_common.h"
#include "clsapi_cli.h"


#define CLSAPI_CLI_PROG_NAME			"clsapi"
#define CLSAPI_CLI_PROG_ALIAS_PREFIX	"cls_"

static char *clsapi_error_code2string(const int error_code)
{
	switch (-error_code) {
	case CLSAPI_ERR_INVALID_COMMAND: return "INVALID_COMMAND";
	case CLSAPI_ERR_INVALID_PARAM: return "INVALID_PARAM";
	case CLSAPI_ERR_METHOD_NOT_FOUND: return "METHOD_NOT_FOUND";
	case CLSAPI_ERR_NOT_FOUND: return "RESOURCE_NOT_FOUND";
	case CLSAPI_ERR_NO_DATA: return "NO_DATA_FOUND";
	case CLSAPI_ERR_PERMISSION_DENIED: return "PERMISSION_DENIED";
	case CLSAPI_ERR_TIMEOUT: return "OPERATION_TIMEOUT";
	case CLSAPI_ERR_NOT_SUPPORTED: return "OPERATION_NOT_SUPPORTED";
	case CLSAPI_ERR_UNKNOWN_ERROR: return "UNKNOWN_ERROR";
	case CLSAPI_ERR_CONNECTION_FAILED: return "CONNECTION_FAILED";
	case CLSAPI_ERR_NO_MEM: return "NO_MEMORY";
	case CLSAPI_ERR_PARSE_ERROR: return "PARSE_ERROR";
	case CLSAPI_ERR_SYSTEM_ERROR: return "SYSTEM_ERROR";
	case CLSAPI_ERR_PIN_CHECKSUM: return "PIN_CHECKSUM_ERROR";
	case CLSAPI_ERR_INTERNAL_ERR: return "INTERNAL_ERR";
	case CLSAPI_ERR_NL80211: return "NL80211_ERROR";
	case CLSAPI_ERR_WPA: return "WPA_ERROR";
	case CLSAPI_ERR_HOSTAPD: return "HOSTAPD_ERROR";
	case CLSAPI_ERR_WPA_SUPPLICANT: return "WPA_SUPPLICANT_ERROR";
	case CLSAPI_ERR_UCI: return "UCI_ERROR";
	case CLSAPI_ERR_TOO_LARGE: return "INVALID_PARAM_OVERSIZE";
	case CLSAPI_ERR_UBUS: return "UBUS_ERROR";
	case CLSAPI_ERR_INVALID_IFNAME: return "INVALID_PARAM_IFNAME_ERROR";
	case CLSAPI_ERR_INVALID_PHYNAME: return "INVALID_PARAM_PHYNAME_ERROR";
	case CLSAPI_ERR_NON_PRIMARY_IFNAME: return "INVALID_PARAM_NON_PRIMARY_IFNAME";
	case CLSAPI_ERR_INVALID_CHANNEL: return "INVALID_PARAM_CHANNEL_ERROR";
	case CLSAPI_ERR_INVALID_HWMODE: return "INVALID_PARAM_HWMODE_ERROR";
	case CLSAPI_ERR_INVALID_BW: return "INVALID_PARAM_BW_ERROR";
	case CLSAPI_ERR_FILE_OPERATION: return "FILE_OPERATION_ERROR";
	case CLSAPI_ERR_ONLY_ON_AP: return "OPERATION_ONLY_ON_AP_MODE";
	case CLSAPI_ERR_ONLY_ON_STA: return "OPERATION_ONLY_ON_STA_MODE";
	case CLSAPI_ERR_NEED_AP_MODE: return "OPERATION_NEED_AP_MODE";
	case CLSAPI_ERR_SOCKET: return "SOCKET_ERROR";
	case CLSAPI_ERR_IOCTL: return "IOCTL_ERROR";
	case CLSAPI_ERR_EXISTED: return "RESOURCE_ALREADY_EXISTED";
	case CLSAPI_ERR_INVALID_DATA: return "INVALID_DATA";
	case CLSAPI_ERR_INVALID_PARAM_LEN: return "INVALID_PARAMETER_LENGTH";
	case CLSAPI_ERR_SCAN_BUSY: return "SCAN_BUSY";
	case CLSAPI_ERR_SCAN_NOT_SUPPORT: return "SCAN_NOT_SUPPORT";
	case CLSAPI_ERR_SCAN_NOT_START: return "SCAN_NOT_START";
	case CLSAPI_ERR_SCAN_SCANNING: return "SCANNING";
	case CLSAPI_ERR_SCAN_ABORTED: return "SCAN_ABORTED";
	case CLSAPI_ERR_SCAN_NO_RESULT: return "SCAN_NO_RESULT";
	case CLSAPI_ERR_SCAN_UNKNOWN: return "SCAN_UNKNOWN_ERROR";
	case CLSAPI_ERR_NULL_POINTER: return "NULL_POINTER";
	case CLSAPI_ERR_STA_NOT_FOUND: return "STATION_NOT_FOUND";
	case CLSAPI_ERR_NO_PASSPHRASE: return "PLEASE SET PASSPHRASE FIRST";
	default: return "Error Code Invalid";
	}
}

struct clsapi_cli_entry *g_clsapi_cli_entry[CLSAPI_CLI_ENTRY_TABLE_MAX] = {0};

static inline int clsapi_cli_local_print(const char *fmt, ...)
{
	int ret;
	va_list arglist;

	va_start(arglist, fmt);
	ret = vprintf(fmt, arglist);
	va_end(arglist);

	if (ret < 0)
		return -CLSAPI_ERR_INTERNAL_ERR;
	return CLSAPI_OK;
}


static inline int clsapi_cli_local_print_output(struct clsapi_cli_output *output)
{
	if (output->buf)
		output->print("%s\n", output->buf);

	printf("\n");
	return CLSAPI_OK;
}


inline int clsapi_cli_report_error(const int ret, struct clsapi_cli_output *output)
{
	return cli_print(output, "error message: %s\n", clsapi_error_code2string(ret));
}

inline int clsapi_cli_report_complete(const int ret, struct clsapi_cli_output *output)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "complete\n");
}


inline int clsapi_cli_report_str_value(const int ret, struct clsapi_cli_output *output, const char *str_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "%s\n", str_value);
}


inline int clsapi_cli_report_uint_value(const int ret, struct clsapi_cli_output *output, const unsigned int uint_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "%u\n", uint_value);
}


inline int clsapi_cli_report_ulong_value(const int ret, struct clsapi_cli_output *output,
	const unsigned long ulong_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "%lu\n", ulong_value);
}


inline int clsapi_cli_report_int_value(const int ret, struct clsapi_cli_output *output, const int int_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "%d\n", int_value);
}


inline int clsapi_cli_report_long_value(const int ret, struct clsapi_cli_output *output, const long long_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "%l\n", long_value);
}


inline int clsapi_cli_report_hexint_value(const int ret, struct clsapi_cli_output *output, const int int_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "0x%08x\n", int_value);
}


inline int clsapi_cli_report_hexlong_value(const int ret, struct clsapi_cli_output *output, const long long_value)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "0x%016lx\n", long_value);
}


inline int clsapi_cli_report_u8_buf(const int ret, struct clsapi_cli_output *output, const unsigned char *u8_buf,
	const int len)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "clsapi_cli_report_u8_buf() support later\n\n");
}

inline int clsapi_cli_report_u32_buf(const int ret, struct clsapi_cli_output *output, const unsigned int *u32_buf,
	const int len)
{
	if (ret < 0)
		return ret;

	return cli_print(output, "clsapi_cli_report_u32_buf() support later\n\n");
}


/**********************	Generic cli handlers for CLS-API with same args	**********************/
/* cli generic handler for get APIs with same args */

int cli_generic_get(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;

	switch (cli_entry->c_api_type) {
	case clsapi_get_out_char1024: {
		// int (*ptr_clsapi_get_out_char1024)(char[1024]);
		string_1024 str_value;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_out_char1024(str_value);
		return clsapi_cli_report_str_value(ret, output, str_value);
	}

	case clsapi_get_in_charptr_out_char1024: {
		// int (*ptr_clsapi_get_in_charptr_out_char1024)(const char *, char[1024]);
		const char *str_arg = argv[0];
		string_1024 str_value;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_out_char1024(str_arg, str_value);
		return clsapi_cli_report_str_value(ret, output, str_value);
	}

	case clsapi_get_in_charptr_out_u32ptr: {
		// int (*ptr_clsapi_get_in_charptr_out_u32ptr)(const char *, uint32_t *);
		const char *str_arg = argv[0];
		uint32_t u32_value;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_out_u32ptr(str_arg, &u32_value);
		return clsapi_cli_report_uint_value(ret, output, u32_value);
	}

	case clsapi_get_in_charptr_out_u8ptr: {
		// int (*ptr_clsapi_get_in_charptr_out_u8ptr)(const char *, uint8_t *);
		const char *str_arg = argv[0];
		uint8_t u8_value;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_out_u8ptr(str_arg, &u8_value);
		return clsapi_cli_report_uint_value(ret, output, u8_value);
	}

	case clsapi_get_in_charptr_out_s32ptr: {
		// int (*ptr_clsapi_get_in_charptr_out_s32ptr)(const char *, int *);
		const char *str_arg = argv[0];
		int int_value;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_out_s32ptr(str_arg, &int_value);
		return clsapi_cli_report_int_value(ret, output, int_value);
	}

	case clsapi_get_in_charptr_s32_out_u32ptr: {
		// int (*ptr_clsapi_get_in_charptr_s32_out_u32ptr)(const char *, const int, uint32_t *);
		const char *str_arg = argv[0];
		const int int_arg = atoi(argv[1]);
		uint32_t u32_value;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_s32_out_u32ptr(str_arg, int_arg, &u32_value);
		return clsapi_cli_report_uint_value(ret, output, u32_value);
	}

	case clsapi_get_in_charptr_out_u8ptr_arrlen: {
		// int (*ptr_clsapi_get_in_charptr_out_u8ptr_arrlen)(const char *, uint8_t *, int *);
		const char *str_arg = argv[0];
		uint8_t buf_arg[1024];
		int buf_len = sizeof(buf_arg);

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_out_u8ptr_arrlen(str_arg, buf_arg, &buf_len);
		return clsapi_cli_report_u8_buf(ret, output, buf_arg, buf_len);
	}

	case clsapi_get_in_charptr_out_u32ptr_buflen: {
		// int (*ptr_clsapi_get_in_charptr_out_u32ptr_buflen)(const char *, uint32_t *, int *);
		const char *str_arg = argv[0];
		uint32_t buf_arg[1024];
		int buf_len = 1024;

		ret = cli_entry->c_api_ptr.ptr_clsapi_get_in_charptr_out_u32ptr_buflen(str_arg, buf_arg, &buf_len);
		return clsapi_cli_report_u32_buf(ret, output, buf_arg, buf_len);
	}

	default:
		cli_print(output, "Error: unsupported CLSAPI type %d\n", cli_entry->c_api_type);
		return -CLSAPI_ERR_NOT_SUPPORTED;
	}
}


/* cli generic handler for set APIs with same args */
int cli_generic_set(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
	int ret = CLSAPI_OK;

	switch (cli_entry->c_api_type) {
	case clsapi_set_charptr: {
		// int (*ptr_clsapi_set_charptr)(const char *);
		const char *str_arg0 = argv[0];

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr(str_arg0);
		break;
	}

	case clsapi_set_charptr_charptr: {
		// int (*ptr_clsapi_set_charptr_charptr)(const char *, const char *);
		const char *str_arg0 = argv[0];
		const char *str_arg1 = argv[1];

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr_charptr(str_arg0, str_arg1);
		break;
	}

	case clsapi_set_charptr_s8: {
		// int (*ptr_clsapi_set_charptr_s8)(const char *, const int8_t);
		const char *str_arg0 = argv[0];
		const int8_t s8_arg1 = atoi(argv[1]);

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr_s8(str_arg0, s8_arg1);
		break;
	}

	case clsapi_set_charptr_s32: {
		// int (*ptr_clsapi_set_charptr_s32)(const char *, const int);
		const char *str_arg0	= argv[0];
		const int int_arg1   = atoi(argv[1]);

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr_s32(str_arg0, int_arg1);
		break;
	}

	case clsapi_set_charptr_u8: {
		// int (*ptr_clsapi_set_charptr_u32)(const char *, const uint8_t);
		const char *str_arg0  = argv[0];
		const uint8_t u8_arg1 = atoi(argv[1]);

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr_u8(str_arg0, u8_arg1);
		break;
	}

	case clsapi_set_charptr_u32: {
		// int (*ptr_clsapi_set_charptr_u32)(const char *, const uint32_t);
		const char *str_arg0    = argv[0];
		const uint32_t u32_arg1 = atoi(argv[1]);

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr_u32(str_arg0, u32_arg1);
		break;
	}

	case clsapi_set_charptr_s32_int: {
		// int (*ptr_clsapi_set_charptr_s32_int)(const char *, const int, const int);
		const char *str_arg0 = argv[0];
		const int int_arg1   = atoi(argv[1]);
		const int int_arg2   = atoi(argv[2]);

		ret = cli_entry->c_api_ptr.ptr_clsapi_set_charptr_s32_int(str_arg0, int_arg1, int_arg2);
		break;
	}

	default:
		cli_print(output, "Error: unsupported CLSAPI type %d\n", cli_entry->c_api_type);
		ret = -CLSAPI_ERR_NOT_SUPPORTED;
		break;
	}

	if (ret == CLSAPI_OK)
		return clsapi_cli_report_complete(ret, output);
	return ret;
}



#define CLSAPI_CLI_LIB_DIR	"/usr/lib/cls/clsapi_cli"

/** @brief Load symbol and call init() to register auto/manual cli entry
 * \param path [in] Path to cli entry plugin
 * Return value:
 *	0: OK
 * !0: Error
 */
static int clsapi_cli_plugin_register_library(const char *path)
{
	void *dlh;
	struct clsapi_cli_plugin *p;

	dlh = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
	if (!dlh) {
		DBG_ERROR("Failed to load plugin %s: %s\n", path, dlerror());
		return -CLSAPI_ERR_NOT_FOUND;
	}

	p = dlsym(dlh, "clsapi_cli_plugin");
	if (!p) {
		DBG_ERROR("Symbol 'clsapi_cli_plugin' is not found!\n\n");
		return -CLSAPI_ERR_METHOD_NOT_FOUND;
	}

	return p->init(g_clsapi_cli_entry);
}


static int cli_entry_table_init(void)
{
	DIR *d;
	int ret = CLSAPI_OK;
	struct stat s;
	struct dirent *e;
	char path[300] = {0};

	if ((d = opendir(CLSAPI_CLI_LIB_DIR)) != NULL) {
		while ((e = readdir(d)) != NULL) {
			snprintf(path, sizeof(path) - 1, CLSAPI_CLI_LIB_DIR "/%s", e->d_name);
			if (stat(path, &s) || !S_ISREG(s.st_mode))
				continue;

			ret = clsapi_cli_plugin_register_library(path);
			if (ret < 0)
				break;
		}
	}

	closedir(d);

	return ret;
}


static void print_usage_options(void)
{
	printf("\nOptions:\n");
	printf("  -h        Show this help.\n");
	printf("  --h       Show this help.\n");
	printf("  -a        List all commands.\n");
}


void print_usage(void)
{
	printf("Usage:\n");
	printf("  %s [<option>...] <action> <param> [<arg>...]\n", CLSAPI_CLI_PROG_NAME);

	print_usage_options();

	// TODO: collect supported actions in array when cli entry loaded,
	// and show collected actions here.
	printf("\nActions:\n");
	printf("  get\n");
	printf("  set\n");
	printf("  add\n");
	printf("  del\n");
	printf("  list\n");

	printf("\nAlias commands:\n");
	printf("  cls_get [<option>...] <param> [<arg>...]\n");
	printf("  cls_set [<option>...] <param> [<arg>...]\n");
	printf("  cls_add [<option>...] <param> [<arg>...]\n");
	printf("  cls_del [<option>...] <param> [<arg>...]\n");
	printf("Note:\n");
	printf("  'cls_<action> [<option>...] <param> [<arg>...]' stands for\n");
	printf("  'clsapi [<option>...] <action> <param> [<arg>...]'\n");
}


void print_alias_usage(const char *action)
{
	printf("Usage:\n");
	printf("  %s%s [<option>...] <param> [<arg>...]\n", CLSAPI_CLI_PROG_ALIAS_PREFIX, action);

	print_usage_options();
}


// Print usage of cmd
void print_usage_cmd(struct clsapi_cli_entry *cli_entry)
{
	printf("Usage:\n");
	printf("  %s %s %s\n", CLSAPI_CLI_PROG_NAME, cli_entry->cmd_name, cli_entry->usage);
}


// List all cmds
void list_all_cmds(void)
{
	int i;
	struct clsapi_cli_entry *cli_entry;

	for (i = 0; i < CLSAPI_CLI_ENTRY_TABLE_MAX; i++)
		for (cli_entry = g_clsapi_cli_entry[i]; cli_entry && cli_entry->cmd_name[0]; cli_entry++)
			printf("  %s %s %s\n", CLSAPI_CLI_PROG_NAME, cli_entry->cmd_name, cli_entry->usage);
}


// List cmds with <action>
void list_action_cmds(const char *action)
{
	int i, action_len, found_action = 0;
	struct clsapi_cli_entry *cli_entry;

	for (i = 0; i < CLSAPI_CLI_ENTRY_TABLE_MAX; i++) {
		for (cli_entry = g_clsapi_cli_entry[i]; cli_entry && cli_entry->cmd_name[0]; cli_entry++) {
			action_len = strlen(action);
			if (memcmp(action, cli_entry->cmd_name, action_len) == 0) {
				printf("  %s %s\n", cli_entry->cmd_name + action_len + 1, cli_entry->usage);
				found_action++;
			}
		}
	}
	if (found_action == 0)
		printf("Unknown action '%s'!\n", action);
}


// List cmds which contains <param>
static int list_possible_cmds(const char *action, const char *param)
{
	int i, found = 0;
	struct clsapi_cli_entry *cli_entry;

	for (i = 0; i < CLSAPI_CLI_ENTRY_TABLE_MAX; i++) {
		for (cli_entry = g_clsapi_cli_entry[i]; cli_entry && cli_entry->cmd_name[0]; cli_entry++) {
			if (strstr(cli_entry->cmd_name, param)) {
				if (found == 0) {
					found = 1;
					printf("\nDid you mean:\n");
				}
				printf("  %s %s %s\n", CLSAPI_CLI_PROG_NAME, cli_entry->cmd_name, cli_entry->usage);
			}
		}
	}

	return found;
}


/** Process cmd options
 * \param argc [in] Num of input args, including options and following others args
 * \param argv [in] Input args in format:
 *                    [option]... <action> <param> [<arg>...] or
 *                    [option]... <param> [<arg>...]
 * Return value:
 *	< 0: Errors
 *  i(>=0): num of options processed
 */

static int process_options(int argc, char **argv, const char *action)
{
	int i = 0;
	int opt_len;
	char option;

	while (i < argc && argv[i][0] == '-') {
		opt_len = strlen(argv[i]);
		if (opt_len == 1) {
			DBG_ERROR("No option found!\n");
			return -CLSAPI_ERR_NOT_SUPPORTED;
		}

		option = argv[i][1];
		switch (option) {
		case '-': // "--h": Show usage
			if (argv[i][2] != 'h')
				return -CLSAPI_ERR_NOT_SUPPORTED;
		case 'h': // Show usage
			if (action[0] == 0) // clsapi
				print_usage();
			else
				print_alias_usage(action);
			return -CLSAPI_ERR_NO_DATA;

		case 'a': // list all cmds
			if (action[0] == 0) // clsapi
				list_all_cmds();
			else
				list_action_cmds(action);
			return -CLSAPI_ERR_NO_DATA;

		default:
			DBG_ERROR("Invalid option '%c'!\n", option);
			return -CLSAPI_ERR_NOT_SUPPORTED;
		}
		i++;
	}

	return i;
}


/** process inputs, return num of args processed
 * \param argc [in] Num of input args, including program name
 * \param argv [in] Input args in format:
 *                    clsapi [<option>...] <action> <param> [<arg>...] or
 *                    cls_<action> [<option>...] <param> [<arg>...]
 * \param action [out] parsed action
 * \param param [out] parsed param
 * Return value:
 *	< 0: Errors
 *  arg_processed(>=0): num of args processed
 */
static int process_input(int argc, char **argv, char action[64], char param[64])
{
	int ret, arg_processed = 0;
	char proc_name[32];

	ret = prctl(PR_GET_NAME, proc_name, 0, 0, 0);
	if (ret < 0)
		return ret;

	if (memcmp(proc_name, CLSAPI_CLI_PROG_ALIAS_PREFIX, sizeof(CLSAPI_CLI_PROG_ALIAS_PREFIX) - 1) == 0) {
		// it's alias cmd, "cls_<action> ..."
		cls_strncpy(action, proc_name + sizeof(CLSAPI_CLI_PROG_ALIAS_PREFIX) - 1, 63);
	}
	if (argc == 1 && action[0] == 0) {
		print_usage();
		return -CLSAPI_ERR_NO_DATA;
	}
	arg_processed++; // program name has been processed

	ret = process_options(argc - arg_processed, argv + arg_processed, action);
	if (ret < 0)
		return -CLSAPI_ERR_INVALID_PARAM;
	arg_processed += ret; // Options have been processed

	// expecting <action> for 'clsapi', <param> for 'cls_<action>'
	if (arg_processed >= argc) { // <action> or <param> is missed
		if (action[0] == 0) // <action> is missed for "clsapi"
			print_usage();
		else // <param> is missed for 'cls_<action>'
			list_action_cmds(action);
		return -CLSAPI_ERR_NO_DATA;
	}
	if (action[0] == 0) // "clsapi ..." case, get action
		cls_strncpy(action, argv[arg_processed++], 63);

	// <action> have processed, expecting <param>
	if (arg_processed >= argc) { // <param> is missed, list params for this action
		list_action_cmds(action);
		return -CLSAPI_ERR_NO_DATA;
	}
	cls_strncpy(param, argv[arg_processed++], 63);

	return arg_processed;
}


/* Usage:
 * clsapi <action> <param> [<arg1> <arg2> ...]
 * or
 * cls_<action> <param> [<arg1> <arg2> ...]
 */
int main(int argc, char **argv)
{
	int arg_processed;
	int i, ret = CLSAPI_OK;
	char action[64] = {0}, param[64] = {0}, cmd_name[129] = {0};
	struct clsapi_cli_entry *cli_entry;
	struct clsapi_cli_output cli_output = {0};

	cli_output.print = clsapi_cli_local_print;
	// load module's cli entry table: auto and manual per module
	if (cli_entry_table_init()) {
		DBG_ERROR("CLSAPI cli entry table init failed\n");
		return -1;
	}

	arg_processed = process_input(argc, argv, action, param);
	if (arg_processed < 0) {
		ret = CLSAPI_OK;
		goto RETURN;
	}
	argc -= arg_processed;
	argv += arg_processed;

	snprintf(cmd_name, sizeof(cmd_name) - 1, "%s %s", action, param);
	for (i = 0; i < CLSAPI_CLI_ENTRY_TABLE_MAX; i++) {
		for (cli_entry = g_clsapi_cli_entry[i]; cli_entry && cli_entry->cmd_name[0]; cli_entry++) {
			if (strcmp(cmd_name, cli_entry->cmd_name) == 0) {
				if (argc < cli_entry->min_arg || argc > cli_entry->max_arg) {
					print_usage_cmd(cli_entry);
					goto RETURN;
				}
				ret = cli_entry->cli_handler(&cli_output, cli_entry, argc, argv);
				goto RETURN;
			}
		}
	}
	printf("Command '%s %s %s' not found!\n", CLSAPI_CLI_PROG_NAME, action, param);

	// not matched cmds, show possible cmds
	ret = list_possible_cmds(action, param);
	if (ret == 0) // no possible cmds found, totally wrong, show usage
		print_usage();

	return CLSAPI_OK;

RETURN:
	if (ret < 0)
		clsapi_cli_report_error(ret, &cli_output);
	return clsapi_cli_local_print_output(&cli_output);
}


