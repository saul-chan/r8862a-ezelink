/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_UTIL_H
#define _CLSAPI_UTIL_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <dirent.h>

/* mapping debug level to syslog level */
enum {
	DBG_LVL_ERROR = 3,
	DBG_LVL_WARN = 4,
	DBG_LVL_INFO = 6,
	DBG_LVL_DEBUG = 7
};

#ifndef DBG_LVL_DEFAULT
#define DBG_LVL_DEFAULT	DBG_LVL_WARN
#endif

#define DBG_DEBUG(fmt, ...)	clsapi_printf(DBG_LVL_DEBUG,	"clsapi.debug [%s():%d] "fmt, __func__, __LINE__, ## __VA_ARGS__)
#define DBG_INFO(fmt, ...)	clsapi_printf(DBG_LVL_INFO,		"clsapi.info [%s():%d] "fmt, __func__, __LINE__, ## __VA_ARGS__)
#define DBG_WARN(fmt, ...)	clsapi_printf(DBG_LVL_WARN,		"clsapi.warn [%s():%d] "fmt, __func__, __LINE__, ## __VA_ARGS__)
#define DBG_ERROR(fmt, ...)	clsapi_printf(DBG_LVL_ERROR,	"clsapi.error [%s():%d] "fmt, __func__, __LINE__, ## __VA_ARGS__)

#ifdef CLSAPI_DEBUG
extern void clsapi_printf(int level, const char *fmt, ...);
#else
#define clsapi_printf(args...)	do{ } while(0)
#endif


#define MACFMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef offsetof
#define offsetof(type, member) ((long) &((type *) 0)->member)
#endif

/*
 * convert MAC address in string format "11:22:33:dd:ee:ff" to 6 byte hex.
 */
static inline int mac_aton(const char *str_mac, unsigned char hex_mac[ETH_ALEN])
{
#define STR_MAC_FORMAT	"aa:bb:cc:dd:ee:ff"
	int ret;

	if (strlen(str_mac) != sizeof(STR_MAC_FORMAT) -1)
		return -1;

	ret = sscanf(str_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hex_mac[0], &hex_mac[1], &hex_mac[2], 
			&hex_mac[3], &hex_mac[4], &hex_mac[5]);
	if (ret == ETH_ALEN)
		return 0;
	else
		return -1;
}

/* check if mac address is unicast: not all 0 or all 0xFF
 * \return 0 on success or others on error.
 */
static inline int validate_unicast_macaddr(const unsigned char hex_mac[ETH_ALEN])
{
	if (!hex_mac)
		return -1;

	if (hex_mac[0] + hex_mac[1] + hex_mac[2] + hex_mac[3] + hex_mac[4] + hex_mac[5] == 0)
		return -1;

	if (hex_mac[0] + hex_mac[1] + hex_mac[2] + hex_mac[3] + hex_mac[4] + hex_mac[5] == 0xFF * 6)
		return -1;

	return 0;
}

/** Get long value with give base by key from text. Match key at beginning of new line.
 * The text is multiple lines which in "<key>=<value>" format, and value is integer/long.
 * Returns:
 *   OK: 0
 *   Errors: -1
 */
static inline int get_long_value_by_key(const char *text, const char *key, long *value, int base)
{
	char key_equal[128];
	const char *pos, *str;

	if (base != 0 && base != 10 && base != 8 && base != 16)
		return -1;

	if (snprintf(key_equal, sizeof(key_equal), "%s=", key) >= sizeof(key_equal))
		return -1;

	str = text;
	while ((pos = strstr(str, key_equal)) != NULL) {
		if ((pos == text || *(pos - 1) == '\n')) { // match from begining of line
			pos += strlen(key_equal);
			*value = strtol(pos, NULL, base);
			return 0;
		}
		str++;
	}

	return -1;
}

/** Get string value by key from text. Match key at beginning of new line. The text is multiple
 * lines which in "<key>=<value>" format, and value is string. Copy string to 'value' until
 * end of line ('\n') or end of string ('\0'). E.g.
 * state=ENABLED
 * phy=wlan1
 */
static inline int get_str_value_by_key(const char *text, const char *key, char *value, const int val_len)
{
	int i = 0;
	char key_equal[128];
	const char *pos, *str;

	if (snprintf(key_equal, sizeof(key_equal), "%s=", key) >= sizeof(key_equal))
		return -1;

	str = text;
	while ((pos = strstr(str, key_equal)) != NULL) {
		if ((pos == text || *(pos - 1) == '\n')) { // match from begining of line
			pos += strlen(key_equal);
			for (i = 0; i < val_len && *pos != '\0' && *pos != '\n'; i++)
				value[i] = *pos++;
			if (i == val_len) // trucated
				return -1;
			 value[i] = '\0';
			return 0;
		}
		str++;
	}

	return -1;
}

/** Copy at max n chars from src to dest util (including) the '\0'.
 * If src longer than dest, string was truncated, set dest[n-1] = '\0' and returns n.
 * Returns:
 *   >0: the number of chars copied (excluding the '\0'). A return value of n means
 *       dest was truncated.
 *   <=0: Errors
 */
static inline int cls_strncpy(char *dest, const char *src, size_t n)
{
	size_t i;

	if (!dest || !src)
		return -1;

	for (i = 0; i < n - 1 && *src != '\0'; i++)
		*dest++ = *src++;
	*dest = '\0';

	if (*src != '\0')
		i++; // src is longer than dest, trucated, return n

	return i;
}

static inline int ignore_dots_dir_filter(const struct dirent *d)
{
	// skip ./ and ../
	if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
		return 0;

	// accept this
	return 1;
}

/* Get first line from file and strip the '\n' to '\0'
 * Returns:
 *   Success: string buffer
 *   Errors:  NULL
 */
static inline char *get_one_line_from_file(const char *file_path)
{
	FILE *fp;
	static char one_line_buf[256];
	fp = fopen(file_path, "r");

	if (fp == NULL)
		return NULL;

	if (fgets(one_line_buf, sizeof(one_line_buf), fp) == NULL) {
		fclose(fp);
		return NULL;
	}

	for (int i = 0; i < sizeof(one_line_buf) && one_line_buf[i]; i ++)
		if (one_line_buf[i] == '\n')
			one_line_buf[i] = '\0';

	fclose(fp);
	return one_line_buf;
}

#endif /* _CLSAPI_UTIL_H */

