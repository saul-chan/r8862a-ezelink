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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdarg.h>   // va_list
#include <libubox/uloop.h>
#include <errno.h>
#include <unistd.h>
#include "os_utils.h"

// NOTE: This is part of the C standard, thus *all* platforms should have it
  // available... and that's why this include can exist in this "platform
  // independent" file

////////////////////////////////////////////////////////////////////////////////
// Hardware stuff
////////////////////////////////////////////////////////////////////////////////

// The following preprocessor macros must be defined to a platform-dependent
// value:
//
//   _HOST_IS_LITTLE_ENDIAN_        |--> Set one (and only one!) of these macros
//   _HOST_IS_BIG_ENDIAN_ENDIAN_    |    to "1" to indicate your CPU endianness
//
//
//   MAX_NETWORK_SEGMENT_SIZE  --------> This is the maximum packet size that
//                                       is allowed in your platform. It is
//                                       used to 'fragment' CMDUs.  Note that
//                                       even if your platform supports packets
//                                       bigger than 1500 bytes, this macro
//                                       must never be bigger than that.  This
//                                       macro is only present in this file for
//                                       those special cases where, for some
//                                       platform related reason, packets must
//                                       be smaller than 1500.
//
//  uint8_t  |                             These types must be adjusted to that
//  uint16_t |---------------------------> they represent 1, 2 and 4 bytes
//  uint32_t |                             unsigned integers respectively.
//
//  INT8S  |                             These types must be adjusted to that
//  INT16S |---------------------------> they represent 1, 2 and 4 bytes signed
//  int32_t |                             integers respectively.
//
//
// In the next few lines we are just going to check that these are defined,
// nothing else.
// In order to actually define them use the "root" Makefile where these MACROS
// are sent to the compiler using the "-D flag" (open the "root" Makefile and
// search for "CCFLAGS" to understand how to do this)

#if !defined(_HOST_IS_LITTLE_ENDIAN_) && !defined(_HOST_IS_BIG_ENDIAN_ENDIAN_)
#  error  "You must define either '_HOST_IS_LITTLE_ENDIAN_' or '_HOST_IS_BIG_ENDIAN_'"
#elif defined(_HOST_IS_LITTLE_ENDIAN_) && defined(_HOST_IS_BIG_ENDIAN_ENDIAN_)
#  error  "You cannot define both '_HOST_IS_LITTLE_ENDIAN_' and '_HOST_IS_BIG_ENDIAN_' at the same time"
#endif

#ifndef  MAX_NETWORK_SEGMENT_SIZE
#  error  "You must define 'MAX_NETWORK_SEGMENT_SIZE' to some value (for example, '1500')"
#endif

#define DEBUG_LVL_ALWAYS (0)
#define DEBUG_LVL_ERROR (1)
#define DEBUG_LVL_WARN (2)
#define DEBUG_LVL_INFO (3)
#define DEBUG_LVL_DETAIL (4)
#define DEBUG_LVL_DEFAULT DEBUG_LVL_ERROR


////////////////////////////////////////////////////////////////////////////////
// Typical libc stuff
////////////////////////////////////////////////////////////////////////////////

// Allocate a chunk of 'n' bytes and return a pointer to it.
//
// If no memory can be allocated, this function must *not* return (instead of
// returning a NULL pointer), and the program must be exited immediately.
//
void *PLATFORM_MALLOC(uint32_t size);

// Free a memory area previously obtained with "PLATFORM_MALLOC()"
//
void PLATFORM_FREE(void *ptr);

// Redimendion a memory area previously obtained  with "PLATFORM_MALLOC()"
//
// If no memory can be allocated, this function must *not* return (instead of
// returning a NULL pointer), and the program must be exited immediately.
//
void *PLATFORM_REALLOC(void *ptr, uint32_t size);

// Sets 'n' bytes (with value 'c') to 'dest'
//
void *PLATFORM_MEMSET(void *dest, uint8_t c, uint32_t n);

// Copies 'n' bytes from 'src' to 'dest'
//
void *PLATFORM_MEMCPY(void *dest, const void *src, uint32_t n);

// Return '0' only if the first 'n' bytes of 's1' and 's2' match, '1' otherwise
// [PLATFORM PORTING NOTE]
//   Note that this is different from the standard libc "memcmp()" behaviour
//
uint8_t PLATFORM_MEMCMP(const void *s1, const void *s2, uint32_t n);

// Return the length of string 's' not including the terminating NULL character
//
uint32_t PLATFORM_STRLEN(const char *s);

// Return ca copy of the provided string 's', that can later be freed with
// "PLATFORM_FREE()" 
//
char *PLATFORM_STRDUP(const char *s);

// Append up to 'n' characters (in addition to the terminating NULL character)
// from 'src' into 'dest'
//
char *PLATFORM_STRNCAT(char *dest, const char *src, uint32_t n);

// Output to a string (see 'man 3 snprintf' on any Linux box)
//
void PLATFORM_SNPRINTF(char *dest, uint32_t n, const char *format, ...);

// Output to a string ("va" version, see 'man 3 vsnprintf' on any Linux box)
//
void PLATFORM_VSNPRINTF(char *dest, uint32_t n, const char *format, va_list ap);

// Output the provided format string (see 'man 3 printf' on any Linux box)
//
void PLATFORM_PRINTF(const char *format, ...);

int PLATFORM_RANDOM(int min, int max);

// Same as 'PLATFORM_PRINTF', but the message will only be processed if the
// platform has the pertaining debug level enabled
//

void debug_msg(uint8_t level, const char *func, int line, const char *format, ...);

#define DEBUG_ALWAYS(...) debug_msg(DEBUG_LVL_ALWAYS, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_ERROR(...) debug_msg(DEBUG_LVL_ERROR, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_WARNING(...) debug_msg(DEBUG_LVL_WARN, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_INFO(...) debug_msg(DEBUG_LVL_INFO, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_DETAIL(...) debug_msg(DEBUG_LVL_DETAIL, __func__, __LINE__, __VA_ARGS__)

// Used to set the verbosity of the previous functions:
//
//   0 => Only print ERROR messages
//   1 => Print ERROR and WARNING messages
//   2 => Print ERROR, WARNING and INFO messages
//   3 => Print ERROR, WARNING, INFO and DETAIL messages
//
enum debug_params {
    debug_param_level = 0,
    debug_param_syslog,
    debug_param_flush
};
void DEBUG_SET(enum debug_params param, int value);
void DEBUG_SET_LOGFILE(char *file);
void DEBUG_CLOSE();
uint32_t PLATFORM_GET_TIMESTAMP(uint32_t age);
char *PLATFORM_GET_TIMESTAMP_STR(struct timeval *ts);
char *PLATFORM_TIMESTAMP_TO_STR(uint32_t ts);
void PLATFORM_GET_TIMESTAMP_TIMEVAL(char *str, struct timeval *tv);
#define PLATFORM_GET_AGE(_ts) (PLATFORM_GET_TIMESTAMP(0)-(_ts))

////////////////////////////////////////////////////////////////////////////////
// Misc stuff
////////////////////////////////////////////////////////////////////////////////

// [PLATFORM PORTING NOTE]
//   Depending on what other platform headers you have included up until this
//   point, 'NULL' might or might not be defined. If so, define it here
//
#ifndef NULL
#  define NULL (0x0)
#endif


////////////////////////////////////////////////////////////////////////////////
// Initialization functions
////////////////////////////////////////////////////////////////////////////////

// This function *must* be called before any other "PLATFORM_*()" API function
//
// Returns "0" if there was a problem. "1" otherwise.
//
// [PLATFORM PORTING NOTE]
//   Use this function to reserve memory, initialize semaphores, etc...
//   
int PLATFORM_INIT(void);
void PLATFORM_DEINIT(void);
#define UBUS_INVOKE_DEFAULT_TIMEOUT (3000)
struct ubus_context *PLATFORM_GET_UBUS();

int getInterfaceIndex(const char *name);

#endif
