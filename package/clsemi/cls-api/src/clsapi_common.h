/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_COMMON_H
#define _CLSAPI_COMMON_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#include "cls_common.h"
#include "clsapi_util.h"


/*******************************	MACROs definitions	**************************/

#define CLSAPI_STR_RESULT		"result"


/*****************************	Data type definitions	**************************/

typedef char	string_8[9];
typedef char	string_16[17];
typedef char	string_32[33];
typedef char	string_64[65];
typedef char	string_128[129];
typedef char	string_256[257];
typedef char	string_512[513];
typedef char	string_1024[1025];
typedef char	string_2048[2049];
typedef char	string_4096[4097];
typedef char	string_8192[8193];

typedef char	clsapi_ifname[33];

enum clsapi_errno {
	CLSAPI_OK = 0,
	/**
	 * Mapping to enum ubus_msg_status
	 */
	CLSAPI_ERR_INVALID_COMMAND,
	CLSAPI_ERR_INVALID_PARAM,
	CLSAPI_ERR_METHOD_NOT_FOUND,
	CLSAPI_ERR_NOT_FOUND,
	CLSAPI_ERR_NO_DATA,
	CLSAPI_ERR_PERMISSION_DENIED,
	CLSAPI_ERR_TIMEOUT,
	CLSAPI_ERR_NOT_SUPPORTED,
	CLSAPI_ERR_UNKNOWN_ERROR,
	CLSAPI_ERR_CONNECTION_FAILED,
	CLSAPI_ERR_NO_MEM,
	CLSAPI_ERR_PARSE_ERROR,
	CLSAPI_ERR_SYSTEM_ERROR,
	CLSAPI_ERR_PIN_CHECKSUM,

	CLSAPI_ERR_BASE = 1000,
	CLSAPI_ERR_INTERNAL_ERR,
	CLSAPI_ERR_NL80211,
	CLSAPI_ERR_WPA,
	CLSAPI_ERR_HOSTAPD,
	CLSAPI_ERR_WPA_SUPPLICANT,
	CLSAPI_ERR_UCI,
	CLSAPI_ERR_TOO_LARGE,
	CLSAPI_ERR_UBUS,
	CLSAPI_ERR_INVALID_IFNAME,
	CLSAPI_ERR_INVALID_PHYNAME,
	CLSAPI_ERR_NON_PRIMARY_IFNAME,
	CLSAPI_ERR_INVALID_CHANNEL,
	CLSAPI_ERR_INVALID_HWMODE,
	CLSAPI_ERR_INVALID_BW,
	CLSAPI_ERR_FILE_OPERATION,
	CLSAPI_ERR_ONLY_ON_AP,
	CLSAPI_ERR_ONLY_ON_STA,
	CLSAPI_ERR_NEED_AP_MODE,
	CLSAPI_ERR_SOCKET,
	CLSAPI_ERR_IOCTL,
	CLSAPI_ERR_EXISTED,
	CLSAPI_ERR_INVALID_DATA,
	CLSAPI_ERR_INVALID_PARAM_LEN,
	CLSAPI_ERR_SCAN_BUSY,
	CLSAPI_ERR_SCAN_NOT_SUPPORT,
	CLSAPI_ERR_SCAN_NOT_START,
	CLSAPI_ERR_SCAN_SCANNING,
	CLSAPI_ERR_SCAN_ABORTED,
	CLSAPI_ERR_SCAN_NO_RESULT,
	CLSAPI_ERR_SCAN_UNKNOWN,
	CLSAPI_ERR_NULL_POINTER,
	CLSAPI_ERR_STA_NOT_FOUND,
	CLSAPI_ERR_NO_PASSPHRASE,

};

/*****************************	Variables definitions	**************************/



/*****************************	Functions declarations	*************************/

/*****************************	Abstract layer of platform independent	*************************/

/*************************************************************************************/
/**************	CLS-Conf: ClourneySemi Configuration Abstraction Layer	**************/
/*************************************************************************************/
/* Different platform has different config system, e.g. OpenWrt uses UCI, Buildroot uses original files.
 * CLS-Conf is designed to making the platform config system is transparent to caller of CLS-API.
 * This is done by adding an abstraction configuration layer. In this layer, platform independent config
 * scheme (cfg, section, param) is designed. (directly mapping to OpenWrt UCI)
 * 	o cfg:		config file or module
 *	o section:	section @ cfg
 *	o param:	param @ section
 * Every config param is represented by (cfg, section, param). E.g. channel number of Wi-Fi 5G radio is
 * (wireless, radio1, channel).
 * In specific platform, it's configuration system is an instance of CLS-Conf.E.g. for OpenWrt, 
 * UCI (package, section, option) is the instance. Instance CLS-Conf in specific platform is needed.
 * Instance CLS-Conf:
 *	o OpenWrt:	CLS-Conf = UCI
 *	o Others:	TODO (maybe porting UCI)
 */

enum clsconf_secname_type {
	CLSCONF_SECNAME_SET,	// section name is caller specified
	CLSCONF_SECNAME_USE_ID,	// use section id (e.g. cfg073579) as section name
	CLSCONF_SECNAME_ANON,	// anonymous section, don't set section name
	__CLSCONF_SECNAME_MAX,
};

/* Get CLS-Conf param value by (cfg, section, param). It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		param @ section
 * Output options:
 *	value:		value of the param
 * Return value:
 *	0:	Success
 *	!0: Error
 */
int clsconf_get_param(const char *cfg, const char *section, const char *param, string_1024 value);

/* Get CLS-Conf param value by (cfg, section, param). It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		param @ section
 *	len:		defalut length of "value"
 * Output options:
 *	value:		buff of the param passed out
 *	len:		real length of list passed out
 * Return value:
 *	0:	Success
 *	!0: Error
 */

int clsconf_get_list(const char *cfg, const char *section, const char *param, string_1024 *value, int *len);

/* Add CLS-Conf list value of (cfg, section, param). It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		param @ section
 *	value:		value of the param
 * Return value:
 *	0:	Success
 *	!0: Error
 */
int clsconf_add_list(const char *cfg, const char *section, const char *param, const char *value);

/* Delete CLS-Conf param value by (cfg, section, list_name). It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section_name:	section @ cfg
 *	list_name:	list @ section
 *	value:		"value" in list
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsconf_del_list(const char *cfg, const char *section_name, const char *list_name, const char *value);

/* Set CLS-Conf param value of (cfg, section, param). It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		param @ section
 *	value:		value of the param
 * Return value:
 *	0:	Success
 *	!0: Error
 */
int clsconf_set_param(const char *cfg, const char *section, const char *param, const char *value);

/* Make CLS-Conf configuration be applied. It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsconf_apply_cfg(const char *cfg);

/* Add CLS-Conf list value of (cfg, section, param) and make it applied. It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		list @ section
 *	value:		value of the list
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsconf_add_apply_list(const char *cfg, const char *section, const char *param, const char *value);

/* Set CLS-Conf param value of (cfg, section, param) and make it applied. It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		param @ section
 *	value:		value of the param
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsconf_set_apply_param(const char *cfg, const char *section, const char *param, const char *value);

/* Add new section with specific type to config and return added section.
 * If section_name is provided, add a named section, and return the section name.
 * If section_name is not provided, add an anonymous section, and return the section id.
 * Input:
 *	cfg:			config file or module
 *	section_type:	section type
 *  section_name:	name of the section. If strlen > 0, add named section; otherwise, add anonymous section.
 * Output:
 *	section_name:	buffer to carry created section name/id.
 * Return value:
 *	0:	Success and return created section name in section_name
 *	!0:	Error
 */
int clsconf_add_section(const char *cfg, const char *section_type, const enum clsconf_secname_type secname_type,
	string_32 section_name);

/* Delete section of given name.
 * The section_name could be name of named section or id of anonymous section (@xxx[n] or cfgxxx).
 * Input:
 *	cfg:			config file or module
 *	section_name:	name of the section. name of named section or id of anonymous section.
 * Return value:
 *	0:	Success
 *	!0: Error
 */
int clsconf_del_section(const char *cfg, const char *section_name);


#ifndef clsconf_get_int_param
#define clsconf_get_int_param(cfg, section, param, int_value) \
	do { \
		string_1024 str_value; \
		ret = clsconf_get_param((cfg), (section), (param), str_value); \
		if (ret == CLSAPI_OK) \
			(int_value) = strtol(str_value, NULL, 10); \
	} while (0);
#endif /* clsconf_get_int_param */

#ifndef clsconf_set_int_param
#define clsconf_set_int_param(cfg, section, param, int_value) \
	do { \
		string_32 str_value; \
		sprintf(str_value, "%d", (int_value)); \
		ret = clsconf_set_param((cfg), (section), (param), str_value); \
	} while (0)
#endif /* clsconf_set_int_param */

#ifndef clsconf_set_uint_param
#define clsconf_set_uint_param(cfg, section, param, uint_value) \
	do { \
		string_32 str_value; \
		sprintf(str_value, "%u", (uint_value)); \
		ret = clsconf_set_param((cfg), (section), (param), str_value); \
	} while (0)
#endif /* clsconf_set_uint_param */

#ifndef clsconf_set_long_param
#define clsconf_set_long_param(cfg, section, param, long_value) \
	do { \
		string_32 str_value; \
		sprintf(str_value, "%l", (long_value)); \
		ret = clsconf_set_param((cfg), (section), (param), str_value); \
	} while (0)
#endif /* clsconf_set_long_param */

#ifndef clsconf_set_ulong_param
#define clsconf_set_ulong_param(cfg, section, param, ulong_value) \
	do { \
		string_32 str_value; \
		sprintf(str_value, "%lu", (ulong_value)); \
		ret = clsconf_set_param((cfg), (section), (param), str_value); \
	} while (0)
#endif /* clsconf_set_ulong_param */

#ifndef clsconf_defer_apply_param
#define clsconf_defer_apply_param(cfg, section, param, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		ret = clsconf_set_param(cfg, section, param, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_apply_param */

#ifndef clsconf_defer_apply_int_param
#define clsconf_defer_apply_int_param(cfg, section, param, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		clsconf_set_int_param(cfg, section, param, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_apply_int_param */

#ifndef clsconf_defer_apply_uint_param
#define clsconf_defer_apply_uint_param(cfg, section, param, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		clsconf_set_uint_param(cfg, section, param, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_apply_uint_param */

#ifndef clsconf_defer_apply_long_param
#define clsconf_defer_apply_long_param(cfg, section, param, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		clsconf_set_long_param(cfg, section, param, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_apply_long_param */

#ifndef clsconf_defer_apply_ulong_param
#define clsconf_defer_apply_ulong_param(cfg, section, param, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		clsconf_set_ulong_param(cfg, section, param, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_apply_ulong_param */

#ifndef clsconf_defer_add_section
#define clsconf_defer_add_section(cfg, sectiont_type, secname_type, section_name) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		ret = clsconf_add_section(cfg, sectiont_type, secname_type, section_name); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_add_section */

#ifndef clsconf_defer_del_section
#define clsconf_defer_del_section(cfg, section_name) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		ret = clsconf_del_section(cfg, section_name); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_del_section */

#ifndef clsconf_defer_add_apply_list
#define clsconf_defer_add_apply_list(cfg, section_name, list_name, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		ret = clsconf_add_list(cfg, section_name, list_name, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_add_apply_list */

#ifndef clsconf_defer_del_apply_list
#define clsconf_defer_del_apply_list(cfg, section_name, list_name, value) \
	do { \
		int defer_mode = 0; \
		clsapi_base_get_defer_mode(&defer_mode); \
		ret = clsconf_del_list(cfg, section_name, list_name, value); \
		if (ret == CLSAPI_OK && defer_mode == 0) \
			ret = clsconf_apply_cfg(cfg); \
	} while (0)
#endif /* clsconf_defer_del_apply_list */

/*
 * \brief Find specified section by k-v pairs of param name and param value.
 * \details Assume the anonymous section with params named "param_name", the "param_name" will be found by iterating all params in this type section. When match the param and its value, section name will be outputed.
 * \param cfg [in] the config of the section.
 * \param section_type [in] the type of the section searched.
 * \param section_name [out] the name of the section which will be returned.
 * \param ... [in] Additional pairs of parameter names and values.
 * \return 0 on success or others on error.
 * */
int clsconf_find_section_by_param(const char *cfg, const char *section_type, string_32 section_name, uint32_t param_num, ...);

/*
 * \brief Check whether the cfg.section_name/cfg.section_name.param existed or not.
 * \details When add or delete some configure, need to check whether the section or param existed or not.
 * \param cfg [in] The config of the section, must be inputed.
 * \param section_name [in] The name of the section, must be inputed.
 * \param param [in] the name of param in the section [optional].
 * \return	CLSAPI_ERR_NOT_FOUND on not existed,
 *	CLSAPI_ERR_EXISTED on existed,
 *	or others on error.
 **/
int clsconf_check_section_param_existed(const char *cfg, const char *section_name, const char *param);

/** Get section alias names (if existed) or specified param value(which represent the section).
 * Inputs:
 *   cfg: Config name.
 *   section_type: Type of section.
 *   param: Name of parameter in sections, such as "name",if set "NULL" will return name of section.
 *   alias_name: Alias name array of sections which outputed.
 *   alias_name_len: Length of section names input as large as possible, output real length.
 *
 * Returns:
 *   0:  Success and section name output in name array.
 *   !0: Errors
*/
int clsconf_get_all_section_alias(const char *cfg, const char *section_type, const char *param, string_128 alias_name[], int *alias_name_len);
#endif /* _CLSAPI_COMMON_H */
