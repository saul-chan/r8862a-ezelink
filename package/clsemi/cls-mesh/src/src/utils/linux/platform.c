/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *  
 *  Copyright (c) 2017, Broadband Forum
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *  
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *  
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *  
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *  
 *  DISCLAIMER
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include "platform.h"
#include <time.h>
#include <stdlib.h>      // free(), malloc(), ...
#include <string.h>      // memcpy(), memcmp(), ...
#include <stdio.h>       // printf(), ...
#include <stdarg.h>      // va_list
#include <sys/time.h>    // gettimeofday()
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include "extension.h"

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
#    include <pthread.h> // mutexes, pthread_self()
#endif
#include <libubus.h>
#include <syslog.h>

////////////////////////////////////////////////////////////////////////////////
// Private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// *********** libc stuff ******************************************************

// We will use this variable to save the instant when "PLATFORM_INIT()" was
// called. This way we sill be able to get relative timestamps later when
// someone calls "PLATFORM_GET_TIMESTAMP()"
//
static struct timeval tv_begin;

static struct ubus_context *platform_ubus = NULL;

// The following variable is used to set which "DEBUG_*()"
// functions should be ignored:
//
//   0 => Only print ERROR messages
//   1 => Print ERROR and WARNING messages
//   2 => Print ERROR, WARNING and INFO messages
//   3 => Print ERROR, WARNING, INFO and DETAIL messages
//
static int8_t log_level = DEBUG_LVL_DEFAULT;
static int8_t log_syslog = 0;
static int8_t log_flush = 5;
static char *log_file = NULL;
static FILE *log_file_fp = NULL;
// Mutex to avoid STDOUT "overlaping" due to different threads writing at the
// same time.
//
#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// Color codes to print messages from different threads in different colors
//
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define ENABLE_COLOR (1)


////////////////////////////////////////////////////////////////////////////////
// Platform API: libc stuff
////////////////////////////////////////////////////////////////////////////////

void *PLATFORM_MALLOC(uint32_t size)
{
    void *p;

    p = malloc(size);

    if (NULL == p)
    {
        printf("ERROR: Out of memory!\n");
        exit(1);
    }

    return p;
}


void PLATFORM_FREE(void *ptr)
{
    return free(ptr);
}


void *PLATFORM_REALLOC(void *ptr, uint32_t size)
{
    void *p;

    p = realloc(ptr, size);

    if (NULL == p)
    {
        printf("ERROR: Out of memory!\n");
        exit(1);
    }

    return p;
}

void *PLATFORM_MEMSET(void *dest, uint8_t c, uint32_t n)
{
    return memset(dest, c, n);
}

void *PLATFORM_MEMCPY(void *dest, const void *src, uint32_t n)
{
    return memcpy(dest, src, n);
}

uint8_t PLATFORM_MEMCMP(const void *s1, const void *s2, uint32_t n)
{
    int aux;

    aux = memcmp(s1, s2, n);

    if (0 == aux)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint32_t PLATFORM_STRLEN(const char *s)
{
    return strlen(s);
}

char *PLATFORM_STRDUP(const char *s)
{
    return strdup(s);
}

char *PLATFORM_STRNCAT(char *dest, const char *src, uint32_t n)
{
    return strncat(dest, src, n);
}

void PLATFORM_SNPRINTF(char *dest, uint32_t n, const char *format, ...)
{
    va_list arglist;

    va_start( arglist, format );
    vsnprintf( dest, n, format, arglist );
    va_end( arglist );

    return;
}

void PLATFORM_VSNPRINTF(char *dest, uint32_t n, const char *format, va_list ap)
{
    vsnprintf( dest, n, format, ap);

    return;
}

void PLATFORM_PRINTF(const char *format, ...)
{
    va_list arglist;

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

int PLATFORM_RANDOM(int val1, int val2)
{
    static int seeded = 0;
    int min = val1 > val2 ? val2 : val1;
    int max = val1 > val2 ? val1 : val2;

    if (val1 == val2)
        return val1;

    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }

    return (rand() % (max-min+1) + min);
}

void DEBUG_SET(enum debug_params param, int value)
{
    switch (param) {
        case debug_param_level:
            log_level = value;
            break;
        case debug_param_syslog:
            if (value)
                log_syslog = 1;
            break;
        case debug_param_flush:
            log_flush = value;
            break;
        default:
            break;
    }
}

void DEBUG_SET_LOGFILE(char *file)
{
    log_file = file;
}

static const char * _dbg_lvl_name[] = {
    "AAA",
    "ERR",
    "WAR",
    "IFO",
    "DTL",
};

static inline int _syslog_priority(int level)
{
    switch (level) {
    case DEBUG_LVL_DETAIL:
        return LOG_DEBUG;
    case DEBUG_LVL_INFO:
        return LOG_NOTICE;
    case DEBUG_LVL_WARN:
        return LOG_WARNING;
    case DEBUG_LVL_ERROR:
        return LOG_ERR;
    }

    return LOG_INFO;
}


static void get_current_time(char* time_buf, size_t buf_size) {
    time_t now;
    struct tm tm_info;

    time(&now);
    localtime_r(&now, &tm_info);
    strftime(time_buf, buf_size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

void debug_msg(uint8_t level, const char *func, int line, const char *format, ...)
{
    va_list arglist;
    char time_buf[64];
    char log_buf[1024];
    int log_len = 0;
    static int8_t log_num;

    if (level > log_level) {
        return;
    }
    get_current_time(time_buf, sizeof(time_buf));

    log_len = snprintf(log_buf, sizeof(log_buf), "[%s][%s][%s:%d] ",
                      time_buf, _dbg_lvl_name[level], func, line);
    if (log_len < 0 || log_len >= (int)sizeof(log_buf)) {
        return;
    }

    va_start(arglist, format);
    vsnprintf(log_buf + log_len, sizeof(log_buf) - log_len, format, arglist);
    va_end(arglist);

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_lock(&printf_mutex);
#endif

    if (log_file) {
        if (!log_file_fp)
            log_file_fp = fopen(log_file, "a");
        if (log_file_fp) {
            fputs(log_buf, log_file_fp);
            if ((++log_num) >= log_flush) {
                fflush(log_file_fp);
                log_num = 0;
            }
        } else {
            fprintf(stderr, "Failed to open log file: %s\n", log_file);
            fputs(log_buf, stderr);
        }
    } else if (log_syslog) {
        syslog(_syslog_priority(level), "%s", log_buf);
    } else
        printf("%s", log_buf);

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
    pthread_mutex_unlock(&printf_mutex);
#endif

    return;
}

void DEBUG_CLOSE()
{
    if (log_file_fp)
        fclose(log_file_fp);
    log_file_fp = NULL;
}

uint32_t PLATFORM_GET_TIMESTAMP(uint32_t age)
{
    struct timeval tv_end;
    uint32_t diff;

    gettimeofday(&tv_end, NULL);

    diff = (tv_end.tv_usec - tv_begin.tv_usec) / 1000 + (tv_end.tv_sec - tv_begin.tv_sec) * 1000;
    diff -= age;

    return diff;
}

char *PLATFORM_GET_TIMESTAMP_STR(struct timeval *ts)
{
    struct tm time;
    struct timeval now;
    static char timestamp_buf[64];

    if (!ts) {
        ts = &now;
        gettimeofday(ts, NULL);
    }

    if (localtime_r(&(ts->tv_sec), &time)) {
        sprintf(timestamp_buf, "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ",
                time.tm_year+1900, time.tm_mon + 1, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec,
                (unsigned int)(ts->tv_usec/1000));
        return timestamp_buf;
    } else
        return NULL;
}

char *PLATFORM_TIMESTAMP_TO_STR(uint32_t ts)
{
    struct tm time;
    struct timeval tv;
    static char timestamp_buf[64];

    tv.tv_sec = tv_begin.tv_sec + ts/1000;

    if (localtime_r(&(tv.tv_sec), &time)) {
        sprintf(timestamp_buf, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
                time.tm_year+1900, time.tm_mon + 1, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec, (unsigned int)(ts%1000));
        return timestamp_buf;
    } else
        return NULL;
}


void PLATFORM_GET_TIMESTAMP_TIMEVAL(char *str, struct timeval *tv)
{
    struct tm tm_time;
    time_t time;
    char *dot = NULL;
    double ms = 0;

    if (str) {
        strptime(str, "%Y-%m-YdT%H:%M:%S%z", &tm_time);
        time = mktime(&tm_time);
        tv->tv_sec = time;
        dot = strchr(str, '.');
        if (dot != NULL){
            ms = atof(dot);
            tv->tv_usec = (int)(ms*1000000);
        }
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: Initialization functions
////////////////////////////////////////////////////////////////////////////////
int PLATFORM_INIT(void)
{

    // Call "_timeval_print()" for the first time so that the initialization
    // time is saved for future reference.
    //
    gettimeofday(&tv_begin, NULL);

    uloop_init();
    platform_ubus = ubus_connect(NULL);

    if (!platform_ubus) {
        DEBUG_ERROR("ubus is not avaliable!\n");
        return -1;
    }

    ubus_add_uloop(platform_ubus);

    loadExtensions();
    emulateExtensions();

    return 0;
}


void PLATFORM_DEINIT(void)
{
    uloop_done();
}

struct ubus_context *PLATFORM_GET_UBUS()
{
    return platform_ubus;
}

int getInterfaceIndex(const char *name)
{
    int                 s;
    struct ifreq        ifr;

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (-1 == s) {
        DEBUG_ERROR("socket(%s) faied, errno=%d(%s)\n",
                name, errno, strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) == -1) {
        DEBUG_ERROR("SIOCGIFINDEX(%s) failed, errno=%d(%s)\n",
                ifr.ifr_name, errno, strerror(errno));
        close(s);
        return -1;
    }

    close(s);
    return ifr.ifr_ifindex;
}


