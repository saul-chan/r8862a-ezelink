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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include "log.h"

int debug_level = LOG_LEVEL_ERROR;
int debug_syslog = 0;
FILE *log_file = NULL;

static inline int _syslog_priority(int level)
{
	switch (level) {
	case LOG_LEVEL_DEBUG:
		return LOG_DEBUG;
	case LOG_LEVEL_INFO:
		return LOG_NOTICE;
	case LOG_LEVEL_WARNING:
		return LOG_WARNING;
	case LOG_LEVEL_ERROR:
		return LOG_ERR;
	}

	return LOG_INFO;
}

void log_print_timestamp(void)
{
	struct timeval tv;
	struct tm *tm;
	char tmstr[STR_LEN_32] = {0};

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	strftime(tmstr, sizeof(tmstr), "%Y-%m-%d %H:%M:%S", tm);
	if (log_file)
		fprintf(log_file, "%s.%03ld: ", tmstr, tv.tv_usec);
	if (!log_file && !debug_syslog)
		printf("%s.%06ld: ", tmstr, tv.tv_usec);
}

void log_printf(int level, const char *fmt, va_list args)
{
	unsigned long filesize = -1;
	char logfile_old[STR_LEN_64] = {0};

	if (debug_syslog)
		vsyslog(_syslog_priority(level), fmt, args);

	log_print_timestamp();
	if (log_file) {
		filesize = ftell(log_file);
		if (filesize >= DEFAULT_LOG_FILE_SIZ) {
			fclose(log_file);
			snprintf(logfile_old, sizeof(logfile_old), "%s.old", DEFAULT_LOG_FILE);
			rename(DEFAULT_LOG_FILE, logfile_old);
			log_file = fopen(DEFAULT_LOG_FILE, "w+");
			if (!log_file)
				log_file = stdout;
		}
		vfprintf(log_file, fmt, args);
		fprintf(log_file, "\n");
		fflush(log_file);
	}
	if (!debug_syslog && !log_file) {
		vprintf(fmt, args);
		printf("\n");
	}
}

void log_debug(const char *fmt, ...)
{
	va_list args;

	if (debug_level > LOG_LEVEL_DEBUG)
		return;

	va_start(args, fmt);
	log_printf(LOG_LEVEL_DEBUG, fmt, args);
	va_end(args);
}

void log_info(const char *fmt, ...)
{
	va_list args;

	if (debug_level > LOG_LEVEL_INFO)
		return;

	va_start(args, fmt);
	log_printf(LOG_LEVEL_INFO, fmt, args);
	va_end(args);
}

void log_warning(const char *fmt, ...)
{
	va_list args;

	if (debug_level > LOG_LEVEL_WARNING)
		return;

	va_start(args, fmt);
	log_printf(LOG_LEVEL_WARNING, fmt, args);
	va_end(args);
}

void log_error(const char *fmt, ...)
{
	va_list args;

	if (debug_level > LOG_LEVEL_ERROR)
		return;

	va_start(args, fmt);
	log_printf(LOG_LEVEL_ERROR, fmt, args);
	va_end(args);
}

void log_dump(unsigned char *buf, size_t len)
{
	char logStr[STR_LEN_1025] = {0};
	int hlen = 0;
	int i = 0;

	if (debug_level > LOG_LEVEL_DEBUG)
		return;
	if (!len || !buf)
		return;
	log_print_timestamp();

	sprintf(logStr, "hexdump(len=%lu)\n", (unsigned long) len);
	if (log_file) {
		hlen = strlen(logStr);
		for (i = 0; i < len; i++)
			snprintf(&logStr[hlen + i * 3], 4, " %02x", buf[i]);
		fwrite(logStr, strlen(logStr), 1, log_file);
		sprintf(logStr, "\n");
		fwrite(logStr, strlen(logStr), 1, log_file);
		fflush(log_file);
	}
}

