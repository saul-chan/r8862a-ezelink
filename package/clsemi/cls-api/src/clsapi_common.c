/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <uci.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <stdarg.h>

#include "clsapi_common.h"


/*****************************	Data type definitions	**************************/


/*****************************	Variables definitions	**************************/


/*************************	C-Call functions declarations	**********************/

#ifdef CLSAPI_PLAT_OPENWRT

/* Get uci list value by (cfg, section, option). It's wrapper of uci APIs.
 * Input options:
 *	cfg:		uci cfg
 *	section:	uci section
 *	list_name:	uci list name
 *	length:		default length of list
 * Output options:
 *	value:		buff of the uci list passed out
 *	length:		real length of list passed out
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
static int uci_get_list(const char *cfg, const char *section, const char *list_name, string_1024 *value, int *length)
{
	int ret = CLSAPI_OK;
	struct uci_context *ctx;
	struct uci_ptr ptr = {0};
	string_1024 uci_path;
	struct uci_element *e;
	int count = 0;

	if (!cfg || !section || !list_name || !value)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	ret = snprintf(uci_path, sizeof(string_1024), "%s.%s.%s", cfg, section, list_name);
	if (ret > 1024) {
		uci_free_context(ctx);
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	if (uci_lookup_ptr(ctx, &ptr, uci_path, true) != UCI_OK) {
		DBG_DEBUG("uci_lookup_ptr() error!\n");
		uci_free_context(ctx);
		return -CLSAPI_ERR_NOT_FOUND;
	}

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE) || !ptr.o ||
			(ptr.o->type != UCI_TYPE_LIST) || !&ptr.o->v.list) {
		uci_free_context(ctx);
		return -CLSAPI_ERR_NOT_FOUND;
	}

	uci_foreach_element(&ptr.o->v.list, e) {
		if (count < *length) {
			cls_strncpy(value[count], e->name, sizeof(value[count]));
			count++;
		} else {
			DBG_ERROR("Input length less than real length\n");
			uci_free_context(ctx);
			return -CLSAPI_ERR_INVALID_PARAM;
		}
	}

	*length = count;
	uci_free_context(ctx);

	return CLSAPI_OK;
}

/* Get uci option value by (cfg, section, option). It's wrapper of uci APIs.
 * The option should be simple string value, not list.
 * Input options:
 *	cfg:		uci cfg
 *	section:	uci section
 *	option:		uci option
 * Output options:
 *	value:		value of the uci option
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
static int uci_get_option(const char *cfg, const char *section, const char *option, string_1024 value)
{
	int ret = CLSAPI_OK;
	struct uci_context *ctx;
	struct uci_ptr ptr = {0};
	string_1024 uci_path;

	if (!cfg || !section || !option || !value)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	ret = snprintf(uci_path, sizeof(string_1024), "%s.%s.%s", cfg, section, option);
	if (ret > 1024) {
		ret = -CLSAPI_ERR_INVALID_PARAM;
		goto out;
	}

	if (uci_lookup_ptr(ctx, &ptr, uci_path, true) != UCI_OK) {
		DBG_DEBUG("uci_lookup_ptr() error!\n");
		ret = -CLSAPI_ERR_NOT_FOUND;
		goto out;
	}

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE) || !ptr.o ||
		(ptr.o->type != UCI_TYPE_STRING) || !ptr.o->v.string) {
		ret = -CLSAPI_ERR_NOT_FOUND;
		goto out;
	}

	ret = cls_strncpy(value, ptr.o->v.string, sizeof(string_1024));
	if (ret == sizeof(string_1024)) {
		ret = -CLSAPI_ERR_TOO_LARGE;
		goto out;
	}

	ret = CLSAPI_OK;

out:
	uci_free_context(ctx);
	return ret;
}


/* Set uci option value of (cfg, section, option) or add the option if it's absent.
 * It's wrapper of uci APIs. Just save configuration to file, does NOT take effect.
 * The option should be simple string value, not list.
 * Input options:
 *	cfg:		uci cfg
 *	section:	uci section
 *	option:		uci option
 *	value:		value of the option
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
static int uci_set_option(const char *cfg, const char *section, const char *option, const char *value, bool list)
{
	int ret = CLSAPI_OK;
	struct uci_context *ctx;
	struct uci_ptr ptr = {0};
	string_1024 uci_path;

	if (!cfg || !section || !option || !value)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	ret = snprintf(uci_path, sizeof(string_1024), "%s.%s.%s=%s", cfg, section, option, value);
	if (ret > 1024)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (uci_lookup_ptr(ctx, &ptr, uci_path, true) != UCI_OK) {
		DBG_ERROR("uci_lookup_ptr() error!\n");
		ret = -CLSAPI_ERR_NOT_FOUND;
		goto out;
	}

	if (list) {
		if (uci_add_list(ctx, &ptr) != UCI_OK || uci_commit(ctx, &ptr.p, false) != UCI_OK) {
			DBG_ERROR("uci_add_list()/uci_commit() error for %s.%s.%s=%s\n", cfg, section, option, value);
			ret = -CLSAPI_ERR_UCI;
			goto out;
		}

	} else {
		if (uci_set(ctx, &ptr) != UCI_OK || uci_commit(ctx, &ptr.p, false) != UCI_OK) {
			DBG_ERROR("uci_set()/uci_commit() error for %s.%s.%s=%s\n", cfg, section, option, value);
			ret = -CLSAPI_ERR_UCI;
			goto out;
		}
	}

	ret = CLSAPI_OK;

out:
	uci_free_context(ctx);
	return ret;
}


/* Make UCI cfg applied.
 * It's wrapper of uci APIs.
 * Input options:
 *	cfg:		uci cfg
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
static int uci_apply_cfg(const char *cfg)
{
	int ret = CLSAPI_OK;
	uint32_t id;
	struct ubus_context *ctx;
	static struct blob_buf buf;
	void *p_data;

	ctx = ubus_connect(NULL);
	if (!ctx) {
		DBG_ERROR("Failed to connect to ubus\n");
		return -CLSAPI_ERR_UBUS;
	}

	ret = ubus_lookup_id(ctx, "service", &id);
	if (ret) {
		DBG_ERROR("Error: ubus_lookup_id() returns %d\n", ret);
		ret = -CLSAPI_ERR_UBUS;
		goto out;
	}

	blob_buf_init(&buf, 0);
	blobmsg_add_string(&buf, "type", "config.change");
	p_data = blobmsg_open_table(&buf, "data");
	blobmsg_add_string(&buf, "package", cfg);
	blobmsg_close_table(&buf, p_data);

	ret = ubus_invoke(ctx, id, "event", buf.head, NULL, NULL, 1000);
	if (ret) {
		DBG_ERROR("%s() Error: ubus_invoke() returns %d for service event(config.change, package:%s)\n",
				__func__, ret, cfg);
		ret = -CLSAPI_ERR_UBUS;
		goto out;
	}

out:
	if (ctx)
		ubus_free(ctx);
	blob_buf_free(&buf);

	return ret;
}


/* Set uci option value of (cfg, section, option) and make it applied.
 * It's wrapper of ubus uci APIs. Call ubus uci set, and then uci commit.
 * Then uci track will make it take effect.
 * The option should be simple string value, not list.
 * Input options:
 *	cfg:		uci cfg
 *	section:	uci section
 *	option:		uci option
 *	value:		value of the option
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
static int uci_set_apply_option(const char *cfg, const char *section, const char *option, const char *value, bool list)
{
	int ret = CLSAPI_OK;

	ret = uci_set_option(cfg, section, option, value, list);
	if (ret < 0)
		return ret;

	return uci_apply_cfg(cfg);
}

/* Add a new uci section to cfg and return created section name/id.
 * If section_name is provided, add a named section, and return the section name.
 * If section_name is not provided, add an anonymous section, and return the section id.
 * Input:
 *	cfg:			config file or module
 *	section_type:	section type
 *	section_name:	name of the section. If strlen > 0, add named section; otherwise, add anonymous section.
 * Output:
 *	section_name:	buffer to carry created section name/id.
 * Return value:
 *	0:	Success and return created section name in section_name
 *	!0:	Error
 */
static int uci_add_new_section(const char *cfg, const char *section_type, const enum clsconf_secname_type secname_type,
	string_32 section_name)
{
	int ret = CLSAPI_OK;
	struct uci_context *ctx;
	struct uci_package *p = NULL;
	struct uci_section *s = NULL;
	struct uci_ptr ptr = {0};

	if (!cfg || !section_type || !section_name || secname_type >= __CLSCONF_SECNAME_MAX)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	ret = uci_load(ctx, cfg, &p);
	if (ret != UCI_OK)
		goto out;

	ret = uci_add_section(ctx, p, section_type, &s);
	if (ret != UCI_OK)
		goto out;

	if (secname_type == CLSCONF_SECNAME_SET || secname_type == CLSCONF_SECNAME_USE_ID) {
		ptr.p = p;
		ptr.s = s;
		if (secname_type == CLSCONF_SECNAME_SET) {
			if (strlen(section_name) < 1) {
				ret = -CLSAPI_ERR_INVALID_PARAM;
				goto out;
			}
			ptr.value = section_name;
		} else {
			ptr.value = s->e.name;
			cls_strncpy(section_name, s->e.name, sizeof(string_32)); // section name = section id
		}
		ret = uci_rename(ctx, &ptr);
		if (ret != UCI_OK)
			goto out;
	} else
		cls_strncpy(section_name, s->e.name, sizeof(string_32)); // section name = section id

	ret = uci_save(ctx, p);
	if (ret == UCI_OK)
		ret = uci_commit(ctx, &p, false);

out:
	uci_unload(ctx, p);
	uci_free_context(ctx);
	if (ret != UCI_OK)
		return -CLSAPI_ERR_UCI;

	return CLSAPI_OK;
}

/* Delete value of list.
 * The section_name could be name of named section or id of anonymous section (@xxx[n] or cfgxxx).
 * Input:
 *	cfg:			config file or module
 *	section_name:	name of the section. name of named section or id of anonymous section.
 * Return value:
 *	0:	Success
 *	!0: Error
 */
static int uci_del_list_value(const char *cfg, const char *section_name, const char *list_name, const char *value)
{
	int ret = CLSAPI_OK;
	struct uci_context *ctx;
	string_1024 uci_path;
	struct uci_ptr ptr = {0};

	if (!cfg || !section_name || !value || !list_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	snprintf(uci_path, sizeof(uci_path), "%s.%s.%s=%s", cfg, section_name, list_name, value);
	if (uci_lookup_ptr(ctx, &ptr, uci_path, true) != UCI_OK) {
		DBG_DEBUG("uci_lookup_ptr() error!\n");
		ret = -CLSAPI_ERR_NOT_FOUND;
		goto out;
	}

	ret = uci_del_list(ctx, &ptr);
	if (ret == UCI_OK)
		ret = uci_commit(ctx, &(ptr.p), false);

out:
	uci_free_context(ctx);
	if (ret != UCI_OK)
		return -CLSAPI_ERR_UCI;

	return CLSAPI_OK;
}

/* Delete section of given name.
 * The section_name could be name of named section or id of anonymous section (@xxx[n] or cfgxxx).
 * Input:
 *	cfg:			config file or module
 *	section_name:	name of the section. name of named section or id of anonymous section.
 * Return value:
 *	0:	Success
 *	!0: Error
 */
static int uci_del_section(const char *cfg, const char *section_name)
{
	int ret = CLSAPI_OK;
	struct uci_context *ctx;
	string_1024 uci_path;
	struct uci_ptr ptr = {0};

	if (!cfg || !section_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	snprintf(uci_path, sizeof(uci_path), "%s.%s", cfg, section_name);
	if (uci_lookup_ptr(ctx, &ptr, uci_path, true) != UCI_OK) {
		DBG_DEBUG("uci_lookup_ptr() error!\n");
		ret = -CLSAPI_ERR_NOT_FOUND;
		goto out;
	}

	ret = uci_delete(ctx, &ptr);
	if (ret == UCI_OK)
		ret = uci_commit(ctx, &(ptr.p), false);

out:
	uci_free_context(ctx);
	if (ret != UCI_OK)
		return -CLSAPI_ERR_UCI;

	return CLSAPI_OK;
}

static int uci_find_section_by_option(const char *cfg, const char *section_type, string_32 section_id, uint32_t option_num, va_list args)
{
	struct uci_context *ctx = uci_alloc_context();
	struct uci_package *pkg = NULL;
	struct uci_element *e_section;
	struct uci_element *e_option;
	int ret = CLSAPI_OK;

	if (!cfg || !section_type || !section_id || option_num % 2 != 0 || option_num == 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	option_num /= 2;
	string_32 *option_value = (string_32 *)calloc(option_num, sizeof(string_32));
	string_32 *option_name = (string_32 *)calloc(option_num, sizeof(string_32));

	for (int i = 0; i < option_num; i++) {
		cls_strncpy(option_name[i], va_arg(args, char *), sizeof(string_32));
		cls_strncpy(option_value[i], va_arg(args, char *), sizeof(string_32));
	}

	ret = uci_load(ctx, cfg, &pkg);
	if (UCI_OK  != ret) {
		free(option_value);
		free(option_name);
		uci_free_context(ctx);

		return -CLSAPI_ERR_UCI;
	}

	uci_foreach_element(&pkg->sections, e_section)
	{
		struct uci_section *s = uci_to_section(e_section);
		int found = 0;

		if (strcmp(section_type, s->type) == 0) {
			for (int i = 0; i < option_num; i++) {
				uci_foreach_element(&s->options, e_option)
				{
					struct uci_option *o = uci_to_option(e_option);

					if (strcmp(option_name[i], o->e.name) == 0 && strcmp(option_value[i], o->v.string) == 0)
						found++;
				}
			}
			if (found == option_num) {
				cls_strncpy(section_id, s->e.name, sizeof(string_32));
				free(option_value);
				free(option_name);
				uci_unload(ctx, pkg);
				uci_free_context(ctx);

				return CLSAPI_OK;
			}
		} else
			continue;
	}

	free(option_value);
	free(option_name);
	uci_unload(ctx, pkg);
	uci_free_context(ctx);

	return -CLSAPI_ERR_NOT_FOUND;
}

int uci_check_section_option_existed(const char *cfg, const char *section_name, const char *option)
{
	struct uci_context *ctx;
	struct uci_ptr ptr = {0};
	string_1024 uci_path;
	int ret = CLSAPI_OK;

	if (!cfg || !section_name)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctx = uci_alloc_context();
	if (!ctx)
		return -CLSAPI_ERR_NO_MEM;

	if (!option) {
		ret = snprintf(uci_path, sizeof(string_1024), "%s.%s", cfg, section_name);
		if (ret > 1024) {
			uci_free_context(ctx);
			return -CLSAPI_ERR_INVALID_PARAM;
		}
	} else {
		ret = snprintf(uci_path, sizeof(string_1024), "%s.%s.%s", cfg, section_name, option);
		if (ret > 1024) {
			uci_free_context(ctx);
			return -CLSAPI_ERR_INVALID_PARAM;
		}
	}

	if (uci_lookup_ptr(ctx, &ptr, uci_path, true) != UCI_OK) {
		uci_free_context(ctx);
		return -CLSAPI_ERR_NOT_FOUND;
	}

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		uci_free_context(ctx);
		return -CLSAPI_ERR_NOT_FOUND;
	}

	uci_free_context(ctx);
	return -CLSAPI_ERR_EXISTED;
}

/*
 * \brief Get section alias names (if existed) or specified option value(which represent the section) from UCI.
 * \param cfg [in] Config name of UCI.
 * \param section_type [in] Type of sections.
 * \param option [in] Option name to identify section, such as "name", if set "NULL" return section name.
 * \param name [out] Section alias name of array to output all section names.
 * \param name_len [out] Length of names, input as large as possible, output real length.
 */
static int uci_get_all_section_alias(const char *cfg, const char *section_type, const char *option, string_128 name[], int *name_len)
{
	struct uci_context *ctx = uci_alloc_context();
	struct uci_package *pkg = NULL;
	struct uci_element *e_section;
	struct uci_element *e_option;
	int i = 0;

	if (UCI_OK != uci_load(ctx, cfg, &pkg)) {
		uci_free_context(ctx);
		return -CLSAPI_ERR_UCI;
	}

	uci_foreach_element(&pkg->sections, e_section)
	{
		struct uci_section *s = uci_to_section(e_section);

		if (strcmp(section_type, s->type) == 0) {
			//return option name and value
			uci_foreach_element(&s->options, e_option)
			{
				struct uci_option *o = uci_to_option(e_option);
				if (option) {
					if (strcmp(option, o->e.name) == 0)
						cls_strncpy(name[i], o->v.string, sizeof(string_128));
				} else
					cls_strncpy(name[i], s->e.name, sizeof(string_128));
			}
			i++;
		}
	}

	*name_len = i;
	uci_unload(ctx, pkg);
	uci_free_context(ctx);

	return CLSAPI_OK;
}
#endif /* CLSAPI_PLAT_OPENWRT */


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
int clsconf_get_param(const char *cfg, const char *section, const char *param, string_1024 value)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_get_option(cfg, section, param, value);
#else
	#error No platform defined
#endif
}

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
int clsconf_get_list(const char *cfg, const char *section, const char *param, string_1024 *value, int *len)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_get_list(cfg, section, param, value, len);
#else
	#error No platform defined
#endif
}


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
int clsconf_set_param(const char *cfg, const char *section, const char *param, const char *value)
{
#ifdef CLSAPI_PLAT_OPENWRT
	bool list = false;

	return uci_set_option(cfg, section, param, value, list);
#else
	#error No platform defined
#endif
}

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
int clsconf_add_list(const char *cfg, const char *section, const char *param, const char *value)
{
#ifdef CLSAPI_PLAT_OPENWRT
	bool list = true;

	return uci_set_option(cfg, section, param, value, list);
#else
	#error No platform defined
#endif
}

/* Make CLS-Conf configuration be applied. It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsconf_apply_cfg(const char *cfg)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_apply_cfg(cfg);
#else
	#error No platform defined
#endif
}


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
int clsconf_set_apply_param(const char *cfg, const char *section, const char *param, const char *value)
{
#ifdef CLSAPI_PLAT_OPENWRT
	bool list = false;

	return uci_set_apply_option(cfg, section, param, value, list);
#else
	#error No platform defined
#endif
}

/* Set CLS-Conf list value of (cfg, section, param) and make it applied. It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 *	section:	section @ cfg
 *	param:		list @ section
 *	value:		value of the list
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsconf_add_apply_list(const char *cfg, const char *section, const char *param, const char *value)
{
#ifdef CLSAPI_PLAT_OPENWRT
	bool list = true;

	return uci_set_apply_option(cfg, section, param, value, list);
#else
	#error No platform defined
#endif
}

int clsconf_add_section(const char *cfg, const char *section_type, const enum clsconf_secname_type secname_type, string_32 section_name)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_add_new_section(cfg, section_type, secname_type, section_name);
#else
	#error No platform defined
#endif
}

int clsconf_del_section(const char *cfg, const char *section_name)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_del_section(cfg, section_name);
#else
	#error No platform defined
#endif
}

int clsconf_del_list(const char *cfg, const char *section_name, const char *list_name, const char *value)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_del_list_value(cfg, section_name, list_name, value);
#else
	#error No platform defined
#endif
}

/*
 * \brief Find specified section by k-v pairs of param name and param value.
 * \details Assume the anonymous section with params named "param_name", the "param_name" will be found by iterating all params in this type section. When match the param and its value, section name will be outputed.
 * \param cfg [in] the config of the section.
 * \param section_type [in] the type of the section searched.
 * \param section_name [out] the name of the section which will be returned.
 * \param ... [in] Additional pairs of parameter names and values.
 * \return 0 on success or others on error.
 * */
int clsconf_find_section_by_param(const char *cfg, const char *section_type, string_32 section_name, uint32_t param_num, ...)
{
#ifdef CLSAPI_PLAT_OPENWRT
	int ret = 0;
	va_list args = {0};

	va_start(args, param_num);
	ret = uci_find_section_by_option(cfg, section_type, section_name, param_num, args);
	va_end(args);

	return ret;
#else
	#error No platform defined
#endif
}

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
int clsconf_check_section_param_existed(const char *cfg, const char *section_name, const char *param)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_check_section_option_existed(cfg, section_name, param);
#else
	#error No platform defined
#endif

}

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
int clsconf_get_all_section_alias(const char *cfg, const char *section_type, const char *param, string_128 alias_name[], int *alias_name_len)
{
#ifdef CLSAPI_PLAT_OPENWRT
	return uci_get_all_section_alias(cfg, section_type, param, alias_name, alias_name_len);
#else
	#error No platform defined
#endif
}
