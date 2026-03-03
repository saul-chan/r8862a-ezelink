/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <stdio.h>
#include <stdarg.h>
#ifdef DBG_SYSLOG
#include <syslog.h>
#endif

#include "clsapi_util.h"

int clsapi_debug_level = DBG_LVL_DEFAULT;

/**
 * clsapi_printf - conditional printf
 * @level: priority level (DBG_LVL_*) of the message
 * @fmt: printf format string, followed by optional arguments
 */
void clsapi_printf(int level, const char *fmt, ...)
{
	va_list arglist;

	if (level <= clsapi_debug_level) {
		va_start(arglist, fmt);
		vprintf(fmt, arglist);
#ifdef DBG_SYSLOG
		vsyslog(level, fmt, arglist);
#endif
		va_end(arglist);
	}

	return;
}

