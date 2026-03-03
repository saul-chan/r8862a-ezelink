/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "clsapi_base.h"


#define CLSAPI_DEFER_MODE		"/tmp/.clsapi_defer"


/*****************************	Functions declarations	*************************/

static int clsapi_defer_mode = -1;

int clsapi_base_get_defer_mode(int *defer)
{
	FILE *f_defer = NULL;

	if (clsapi_defer_mode < 0) {
		f_defer = fopen(CLSAPI_DEFER_MODE, "r");
		if (f_defer) {
			clsapi_defer_mode = 1;
			fclose(f_defer);
		}
		else
			clsapi_defer_mode = 0;
	}

	*defer = clsapi_defer_mode;
	return CLSAPI_OK;
}

int clsapi_base_set_defer_mode(const int defer)
{
	int ret = CLSAPI_OK;
	FILE *f_defer = NULL;

	if (defer == 1) {
		f_defer = fopen(CLSAPI_DEFER_MODE, "wb");
		if (!f_defer)
			ret = -CLSAPI_ERR_SYSTEM_ERROR;
		else
			fclose(f_defer);
		clsapi_defer_mode = 1;
	}
	else {
		f_defer = fopen(CLSAPI_DEFER_MODE, "r");
		if (f_defer) {
			fclose(f_defer);
			unlink(CLSAPI_DEFER_MODE);
		}
		clsapi_defer_mode = 0;
	}

	return ret;
}

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
int clsapi_base_get_conf_param(const char *cfg, const char *section, const char *param, string_1024 value)
{
	return clsconf_get_param(cfg, section, param, value);
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
int clsapi_base_set_conf_param(const char *cfg, const char *section, const char *param, const char *value)
{
	return clsconf_set_param(cfg, section, param, value);
}


/* Make CLS-Conf configuration be applied. It's platform independent API.
 * Input options:
 *	cfg:		config file or module
 * Return value:
 *	0:	Success
 *	!0:	Error
 */
int clsapi_base_apply_conf_cfg(const char *cfg)
{
	return clsconf_apply_cfg(cfg);
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
int clsapi_base_set_apply_conf_param(const char *cfg, const char *section, const char *param, const char *value)
{
	return clsconf_set_apply_param(cfg, section, param, value);
}

