/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_CLI_H
#define _CLSAPI_CLII_H

#define CLSAPI_CLI_INDENT	"    "

struct clsapi_cli_output {
	char *buf;
	int (*print)(const char *format, ...);
};


enum clsapi_api_type {
	clsapi_undefined,
	clsapi_manual = clsapi_undefined,
	/* clsapi get operations (have return values) */
	clsapi_get_out_char1024,	//int (*ptr_clsapi_get_out_char1024)(char[1024]);
	clsapi_get_in_charptr_out_char1024,	//int (*ptr_clsapi_get_in_charptr_out_char1024)(const char *, char[1024]);
	clsapi_get_in_charptr_out_u32ptr,	//int (*ptr_clsapi_get_in_charptr_out_u32ptr)(const char *, uint32_t *);
	clsapi_get_in_charptr_out_u8ptr,	//int (*ptr_clsapi_get_in_charptr_out_u8ptr)(const char *, uint8_t *);
	clsapi_get_in_charptr_out_s32ptr,	//int (*ptr_clsapi_get_in_charptr_out_s32ptr)(const char *, int *);
	clsapi_get_in_charptr_s32_out_u32ptr,	//int (*ptr_clsapi_get_in_charptr_s32_out_u32ptr)(const char *, const int, uint32_t *);
	clsapi_get_in_charptr_out_u8ptr_arrlen,	//int (*ptr_clsapi_get_in_charptr_out_u8ptr_arrlen)(const char *, uint8_t *, int *);
	clsapi_get_in_charptr_out_u32ptr_buflen,	//int (*ptr_clsapi_get_in_charptr_out_u32ptr_buflen)(const char *, uint32_t *, int *);

	/* clsapi set operations (do NOT have return value) */
	clsapi_set_charptr = 100,	//int (*ptr_clsapi_set_charptr)(const char *);
	clsapi_set_charptr_charptr,	//int (*ptr_clsapi_set_charptr_charptr)(const char *, const char *);
	clsapi_set_charptr_s8,	//int (*ptr_clsapi_set_charptr_s8)(const char *, const int8_t);
	clsapi_set_charptr_s32,	//int (*ptr_clsapi_set_charptr_s32)(const char *, const int);
	clsapi_set_charptr_u8,	//int (*ptr_clsapi_set_charptr_u32)(const char *, const uint8_t);
	clsapi_set_charptr_u32,	//int (*ptr_clsapi_set_charptr_u32)(const char *, const uint32_t);
	clsapi_set_charptr_s32_int,	//int (*ptr_clsapi_set_charptr_s32_int)(const char *, const int, const int);
};

#define C_API(api_type, apt_ptr) api_type, .c_api_ptr.ptr_ ## api_type = apt_ptr

struct clsapi_cli_entry {
	char cmd_name[64];
	int min_arg, max_arg;
	char usage[256];
	int (*cli_handler)(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv);
	int c_api_type;
	union {
		/* clsapi get operations (have return values) */
		int (*ptr_clsapi_get_out_char1024)(string_1024);
		int (*ptr_clsapi_get_in_charptr_out_char1024)(const char *, string_1024);
		int (*ptr_clsapi_get_in_charptr_out_u32ptr)(const char *, uint32_t *);
		int (*ptr_clsapi_get_in_charptr_out_u8ptr)(const char *, uint8_t *);
		int (*ptr_clsapi_get_in_charptr_out_s32ptr)(const char *, int *);
		int (*ptr_clsapi_get_in_charptr_s32_out_u32ptr)(const char *, const int, uint32_t *);
		int (*ptr_clsapi_get_in_charptr_out_u8ptr_arrlen)(const char *, uint8_t *, int *);
		int (*ptr_clsapi_get_in_charptr_out_u32ptr_buflen)(const char *, uint32_t *, int *);

		/* clsapi set operations (do NOT have return value) */
		int (*ptr_clsapi_set_charptr)(const char *);
		int (*ptr_clsapi_set_charptr_charptr)(const char *, const char *);
		int (*ptr_clsapi_set_charptr_s8)(const char *, const int8_t);
		int (*ptr_clsapi_set_charptr_s32)(const char *, const int);
		int (*ptr_clsapi_set_charptr_u8)(const char *, const uint8_t);
		int (*ptr_clsapi_set_charptr_u32)(const char *, const uint32_t);
		int (*ptr_clsapi_set_charptr_s32_int)(const char *, const int, const int);
	} c_api_ptr;
};

enum clsapi_cli_entry_table_index {
	CLSAPI_CLI_ENTRY_TABLE_BASE_AUTO,
	CLSAPI_CLI_ENTRY_TABLE_BASE_MANUAL,
	CLSAPI_CLI_ENTRY_TABLE_WIFI_AUTO,
	CLSAPI_CLI_ENTRY_TABLE_WIFI_MANUAL,
	CLSAPI_CLI_ENTRY_TABLE_NET_AUTO,
	CLSAPI_CLI_ENTRY_TABLE_NET_MANUAL,
	CLSAPI_CLI_ENTRY_TABLE_SYS_AUTO,
	CLSAPI_CLI_ENTRY_TABLE_SYS_MANUAL,
	CLSAPI_CLI_ENTRY_TABLE_MAX
};

extern struct clsapi_cli_entry *g_clsapi_cli_entry[CLSAPI_CLI_ENTRY_TABLE_MAX];

struct clsapi_cli_plugin {
	int (*init)(struct clsapi_cli_entry *cli_entry_tbl[]);
};


#define cli_print(output, fmt, ...) (output->print(fmt, ## __VA_ARGS__))
int clsapi_cli_report_error(const int ret, struct clsapi_cli_output *output);
int clsapi_cli_report_complete(const int ret, struct clsapi_cli_output *output);
int clsapi_cli_report_str_value(const int ret, struct clsapi_cli_output *output, const char *str_value);
int clsapi_cli_report_uint_value(const int ret, struct clsapi_cli_output *output, const unsigned int uint_value);
int clsapi_cli_report_ulong_value(const int ret, struct clsapi_cli_output *output, const unsigned long ulong_value);
int clsapi_cli_report_int_value(const int ret, struct clsapi_cli_output *output, const int int_value);
int clsapi_cli_report_long_value(const int ret, struct clsapi_cli_output *output, const long long_value);
int clsapi_cli_report_hexint_value(const int ret, struct clsapi_cli_output *output, const int int_value);
int clsapi_cli_report_hexlong_value(const int ret, struct clsapi_cli_output *output, const long int_value);
int clsapi_cli_report_u8_buf(const int ret, struct clsapi_cli_output *output, const unsigned char *u8_buf,
	const int len);
int clsapi_cli_report_u32_buf(const int ret, struct clsapi_cli_output *output, const unsigned int *u32_buf,
	const int len);
int cli_generic_get(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv);
int cli_generic_set(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv);

#endif /* _CLSAPI_CLI_H */

