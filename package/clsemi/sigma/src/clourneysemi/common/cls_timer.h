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


#include <pthread.h>
#include "list.h"

#define CLS_TIMER_ADD 0
#define CLS_TIMER_DEL 1

#define CLS_TIMER_THREAD_RECV_PATH "/var/run/cls_timer_recv"
#define CLS_DUT_THREAD_SEND_PATH "/var/run/cls_dut_send"

#define CLS_TIMER_LOCK(p_lock)  pthread_mutex_lock(p_lock)
#define CLS_TIMER_TRYLOCK(p_lock)  pthread_mutex_trylock(p_lock)
#define CLS_TIMER_UNLOCK(p_lock) pthread_mutex_unlock(p_lock)

#define MAX_LEN_CMD   64 /* the max length of handler cmd, such as cls_api, ubus cmd ,... etc */
struct handler_item {
	struct list_head l;
	uint32_t timer_id;
	char *cmd;
};

struct cls_timer_item {
	struct list_head l;
	uint32_t id;
	uint32_t countdown; /* the countdown value from timer starting, the init value is the timer value */
	struct list_head handler_list; /* all handler list */
};

struct cls_timers_list {
	struct list_head timers; /* all handler list */
};

int dut_register_timer(uint32_t sec, char *handler);
void dut_cancel_timer(uint32_t timer_id);
int timer_service_init(void);
