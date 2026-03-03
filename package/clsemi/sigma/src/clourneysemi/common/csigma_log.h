/****************************************************************************
*
* Copyright (c) 2023  Clourney Semiconductor Co.,Ltd.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#ifndef CSIGMA_LOG_H
#define CSIGMA_LOG_H

#include <syslog.h>

#define cls_log(frm, args...)							\
	do {										\
		syslog(LOG_DEBUG, "%s:%d " frm, __FILE__, __LINE__, ##args);		\
	} while (0)

#define cls_error(frm, args...)							\
	do {										\
		syslog(LOG_ERR, "%s:%d " frm, __FILE__, __LINE__, ##args);		\
	} while (0)

#endif				/* CSIGMA_LOG_H */

/* debug print switch on(1)/off(0) */
#define ENABLE_PRINT 1
#ifdef ENABLE_PRINT
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif