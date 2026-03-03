/*
 *  Copyright (c) 2021-2025, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */
#ifndef __LOG_H__
#define __LOG_H__

#include "common.h"

extern int debug_level;
extern int debug_syslog;
extern FILE *log_file;

enum {
	LOG_LEVEL_DEBUG = 0, LOG_LEVEL_INFO, LOG_LEVEL_WARNING, LOG_LEVEL_ERROR
};

#define DEFAULT_LOG_FILE "/var/log/clsnetmanager.log"
/*Log file default size is 64k*/
#define DEFAULT_LOG_FILE_SIZ 0x100000

void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_dump(unsigned char *buf, size_t len);

#endif

