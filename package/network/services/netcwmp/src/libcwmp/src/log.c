/************************************************************************
 * Id: log.c                                                            *
 *                                                                      *
 * TR069 Project:  A TR069 library in C                                 *
 * Copyright (C) 2013-2014  netcwmp.netcwmp group                                *
 *                                                                      *
 *                                                                      *
 * Email: netcwmp ( & ) gmail dot com                                *
 *                                                                      *
 ***********************************************************************/
 
 
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "cwmp/log.h"

struct cwmp_log_t
{
    FILE * file;
    int fsize;
    int level;
    char * name;
};

static cwmp_log_t 	    g_cwmp_log_file;
static cwmp_log_t 		*	g_ot_log_file_ptr;

//log init
void cwmp_log_level_set(int level){
    g_cwmp_log_file.level = level;
}

void cwmp_log_fsize_set(int fsize){
    g_cwmp_log_file.fsize = fsize * CWMP_1K;
}

int cwmp_log_init(const char * filename, int level)
{

    g_cwmp_log_file.file = NULL;
    g_cwmp_log_file.name = NULL;
    if (filename)
    {
        g_cwmp_log_file.file = fopen(filename,"a+");
        g_cwmp_log_file.name = strdup(filename);
    }

    if (g_cwmp_log_file.file == NULL)
    {
        g_cwmp_log_file.file = stdout;
    }

    g_cwmp_log_file.level = level;

    g_cwmp_log_file.fsize = CWMP_LOG_FILE_SIZE;

    g_ot_log_file_ptr = &g_cwmp_log_file;

    return 0;
}

void cwmp_log_fini()
{
    if (g_cwmp_log_file.name)
    {
        free(g_cwmp_log_file.name);
    }

    if ((g_cwmp_log_file.file != stdout) && (g_cwmp_log_file.file != NULL))
    {
        fclose(g_cwmp_log_file.file);
    }

}

void cwmp_log_write(int level, cwmp_log_t * log, const char * fmt, va_list ap)
{
    cwmp_log_t * logger;
    unsigned long filesize = -1;
    char oldfilepath[64] = {0};

    if (log)
    {
        if (log->level < level )
        {
            return;
        }
        logger = log;
    }
    else
    {
        if (g_cwmp_log_file.level < level)
        {
            return;
        }
        logger = &g_cwmp_log_file;
    }

    logger = g_ot_log_file_ptr;

    filesize = ftell(logger->file);
    if((logger->file != stdout) && (filesize >= g_cwmp_log_file.fsize)){
        fclose(logger->file);
        snprintf(oldfilepath, sizeof(oldfilepath), "%s.old", logger->name);
        rename(logger->name, oldfilepath);
        logger->file = fopen(logger->name, "w+");
        if (logger->file == NULL){
            logger->file = stdout;
        }
    }

    vfprintf(logger->file, fmt, ap);
    fprintf(logger->file, "\n");

    fflush(logger->file);
}

void cwmp_log_tracer(int level, cwmp_log_t * log,const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    cwmp_log_write(level, log, fmt, ap);
    va_end(ap);
}

void cwmp_log_debug(const char * fmt, ...)
{
    va_list ap;
    if (g_ot_log_file_ptr->level < CWMP_LOG_DEBUG)
        return;


    va_start(ap, fmt);
    cwmp_log_write(CWMP_LOG_DEBUG, g_ot_log_file_ptr, fmt, ap);
    va_end(ap);
}

void cwmp_log_info(const char * fmt, ...)
{
    va_list ap;
    if (g_ot_log_file_ptr->level < CWMP_LOG_INFO)
        return;


    va_start(ap, fmt);
    cwmp_log_write(CWMP_LOG_INFO, g_ot_log_file_ptr, fmt, ap);
    va_end(ap);
}

void cwmp_log_error(const char * fmt, ...)
{
    va_list ap;
    if (g_ot_log_file_ptr->level < CWMP_LOG_ERROR)
        return;


    va_start(ap, fmt);
    cwmp_log_write(CWMP_LOG_ERROR, g_ot_log_file_ptr, fmt, ap);
    va_end(ap);
}

void cwmp_log_alert(const char * fmt, ...)
{
    va_list ap;
    if (g_ot_log_file_ptr->level < CWMP_LOG_ALERT)
        return;


    va_start(ap, fmt);
    cwmp_log_write(CWMP_LOG_ALERT, g_ot_log_file_ptr, fmt, ap);
    va_end(ap);
}

void cwmp_log_critical(const char * fmt, ...)
{
    va_list ap;
    if (g_ot_log_file_ptr->level < CWMP_LOG_CRIT)
        return;


    va_start(ap, fmt);
    cwmp_log_write(CWMP_LOG_CRIT, g_ot_log_file_ptr, fmt, ap);
    va_end(ap);
}

