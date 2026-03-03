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

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "common/csigma_log.h"
#include "cls_timer.h"

int timer_thread_recv_fd = -1, dut_thread_send_fd = -1;
struct sockaddr_un recv_addr, send_addr;

uint32_t gb_timer_id = 0, expired_ticks = 1;

struct cls_timers_list dut_timer_list;
pthread_mutex_t timer_lock = PTHREAD_MUTEX_INITIALIZER;


static uint32_t gen_timer_id(void)
{
	uint32_t tmp = gb_timer_id + 1;

	if (tmp < gb_timer_id)
		gb_timer_id = 0;
	else
		gb_timer_id = tmp;
	return gb_timer_id;
}

int dut_register_timer(uint32_t sec, char *handler)
{
	char buf[128] = {0};
	uint8_t len_cmd;
	uint32_t timer_id = htonl(gen_timer_id());
	uint32_t value = htonl(sec);
	socklen_t len_addr = sizeof(recv_addr);

	len_cmd = strlen(handler) + 1;
	if (len_cmd > MAX_LEN_CMD) {
		cls_error("timer handler cmd is too long\n");
		return -1;
	}

	buf[0] = CLS_TIMER_ADD; /* timer action */
	memcpy((buf + 1), &timer_id, 4); /* timer ID */
	memcpy((buf + 5), &value, 4); /* timer value */
	sprintf((buf + 9), "%s", handler); /* handler cmd */

	if (sendto(dut_thread_send_fd, buf, 128, 0, (struct sockaddr *)&recv_addr, len_addr) == -1) {
		cls_error("add timer=%d sec, fail\n", sec);
		return -1;
	}
	cls_log("ADD timer[%d]=%d second, handler=%s\n", gb_timer_id, sec, handler);
	return gb_timer_id;
}

void dut_cancel_timer(uint32_t timer_id)
{
	uint8_t buf[5] = {0};
	uint32_t id = htonl(timer_id);
	socklen_t len_addr = sizeof(recv_addr);

	buf[0] = CLS_TIMER_DEL; /* timer action */
	memcpy((buf + 1), &id, 4);  /* timer ID */

	cls_log("CANCEL timer[%d]\n", timer_id);
	if (sendto(dut_thread_send_fd, buf, 5, 0, (struct sockaddr *)&recv_addr, len_addr) == -1)
		cls_error("cancel timer_id=%d fail\n", timer_id);
	return;
}

static int add_handler(struct cls_timer_item *cur, char *cmd_str, uint32_t timer_id)
{
	struct handler_item *handler = (struct handler_item *)malloc(sizeof(struct handler_item));

	memset(handler, 0, sizeof(struct handler_item));
	handler->cmd = (char *)malloc(strlen(cmd_str) + 1);
	if (!handler->cmd) {
		cls_error("NO space for new timer's cmd!\n");
		return -1;
	}
	handler->timer_id = timer_id;
	sprintf(handler->cmd, "%s", cmd_str);
	list_add(&handler->l, &cur->handler_list);
	return 0;
}

static struct cls_timer_item *add_timer(uint32_t id, uint32_t sec, char *cmd_str)
{
	struct cls_timer_item *timer = (struct cls_timer_item *)malloc(sizeof(struct cls_timer_item));

	if (!timer) {
		cls_error("NO space for new timer!\n");
		return NULL;
	}

	memset(timer, 0, sizeof(struct cls_timer_item));
	timer->id = id;
	timer->countdown = sec;
	INIT_LIST_HEAD(&timer->handler_list);

	add_handler(timer, cmd_str, id);
	return timer;
}

static void cancel_timer(uint32_t id, struct cls_timers_list *list)
{
	struct cls_timer_item *timer = NULL;
	struct handler_item *handler;

	list_for_each_entry(timer, &list->timers, l) {
		if (timer->id == id) {
			break;
		}
	}
	if (!timer) {
		cls_error("can NOT find timer[%d]\n", id);
		return;
	}
	CLS_TIMER_LOCK(&timer_lock);
	list_del(&timer->l);
	CLS_TIMER_UNLOCK(&timer_lock);
	list_free_items(&timer->handler_list, struct handler_item, l);
	free(timer);
}

/* We set the accurancy of timer to second, due to sigmal's requirement. */
/* The accurancy can be adjusted to lower than second, but the impact to */
/* CPU utilization must be considered */
static void set_periodical_timer(void)
{
	struct itimerval value;

	/* sigalarm triggered per one second */
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 1;
	value.it_interval.tv_sec = 1;
	value.it_interval.tv_usec = 0; /* interval = 1s */

	setitimer(ITIMER_REAL, &value, NULL);
	return;
}

static void timer_action(struct cls_timers_list *list, uint8_t action, uint32_t id, uint32_t sec, char *str_handler)
{
	struct cls_timer_item *new_timer;
	struct cls_timer_item *existed_timer;
	char *tmp_handler = NULL;

	if (action != CLS_TIMER_ADD && action != CLS_TIMER_DEL) {
		cls_error("wrong timer action=%d\n", action);
		return;
	}

	if ((CLS_TIMER_ADD == action) && list_empty(&list->timers)) {
		new_timer = add_timer(id, sec, str_handler);
		if (!new_timer) {
			cls_error("add new timer[id=%d] fail\n", id);
			return;
		}
		insert_behind(&new_timer->l, &dut_timer_list.timers);
		set_periodical_timer(); /* the first timer registration will trigger the periodcal timer */
		return;
	}

	if (CLS_TIMER_ADD == action) {
		list_for_each_entry(existed_timer, &list->timers, l) {
			if (existed_timer->countdown == sec) {
				CLS_TIMER_LOCK(&timer_lock);
				add_handler(existed_timer, str_handler, id);
				CLS_TIMER_UNLOCK(&timer_lock);
				return;
			}
			else {
				new_timer = add_timer(id, sec, str_handler);
				if (!new_timer) {
					cls_error("add new timer[id=%d] fail\n", id);
					return;
				}

				CLS_TIMER_LOCK(&timer_lock);
				if (existed_timer->countdown < sec)
					insert_behind(&new_timer->l, &existed_timer->l);
				else
					insert_before(&new_timer->l, &existed_timer->l);
				CLS_TIMER_UNLOCK(&timer_lock);
			}
		}
	}
	else
		cancel_timer(id, list);
}

static int timer_service_recv(struct cls_timers_list *list)
{
	uint8_t buf[128] = {0};
	uint32_t sec = 0, id = 0, tmp = 0;
	uint8_t action;
	socklen_t len_addr = sizeof(send_addr);
	char *handler = NULL;

	if (recvfrom(timer_thread_recv_fd, buf, 128, 0, (struct sockaddr *)&send_addr, &len_addr) > 0) {
		action = buf[0];
		memcpy(&tmp, buf + 1, 4);
		id = ntohl(tmp);
		if (action == CLS_TIMER_ADD) {
			memcpy(&tmp, buf + 5, 4);
			sec = ntohl(tmp);
			handler = buf + 9;
		}
		cls_log("recv timer registration: action=%s, id=%d, value=%d, handler=%s\n",
					action ? "DEL" : "ADD", id, sec, handler ? handler : "NONE");
		timer_action(list, action, id, sec, handler);
		return 0;
	}
	else {
		cls_error("timer registration receive failed\n");
		return -1;
	}
}

static void alarm_signal_block(void)
{
	sigset_t set;
	int ret;

	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	ret = sigprocmask(SIG_BLOCK, &set, NULL);
	cls_log("block SIGALARM %s\n", ret == 0 ? "success" : "fail");
}

static void *timer_service_loop(void *arg)
{
	fd_set read_fds;
	struct cls_timers_list *list = (struct cls_timers_list *)arg;

	FD_ZERO(&read_fds);
	FD_SET(timer_thread_recv_fd, &read_fds);
	alarm_signal_block();

	while (1) {
		select(timer_thread_recv_fd + 1, &read_fds, NULL, NULL, NULL);

		if (FD_ISSET(timer_thread_recv_fd, &read_fds)) {
			timer_service_recv(list);
		}
	}
}

static void do_handler(struct cls_timer_item *cur)
{
	struct handler_item *handler;

	/* NO need to lock handler's lock, because the timer is removed from dut timer list */
	list_for_each_entry(handler, &cur->handler_list, l) {
		cls_log("do timer handler: %s\n", handler->cmd);
		if (handler->cmd) {
			system(handler->cmd);
			free(handler->cmd); /* free the space of handler after execution */
		}
	}
}

static void handle_timeout(int signum)
{
	uint32_t tmp_value = 0;
	struct cls_timer_item *timer;
	int ret;

	if (list_empty(&dut_timer_list.timers)) {
		cls_log("there is NO timer now, close SIGALRM\n");
		alarm(0);
		return;
	}

	ret = CLS_TIMER_TRYLOCK(&timer_lock);
	if (0 != ret) { /* lock failed, record the tick, minus in the next success locking */
		expired_ticks++;
		return;
	}
	list_for_each_entry(timer, &dut_timer_list.timers, l) {
		tmp_value = timer->countdown - expired_ticks;
		if (0 == ret)
			expired_ticks = 1; /* reset the expired ticks */
		if (tmp_value > timer->countdown) /* if something make countdown value overflow */
			timer->countdown = 0;
		else
			timer->countdown = tmp_value;

		if (timer->countdown == 0) {
			list_del(&timer->l);
			CLS_TIMER_UNLOCK(&timer_lock);
			do_handler(timer);
			list_free_items(&timer->handler_list, struct handler_item, l);
			free(timer);
			return;
		}
	}
	CLS_TIMER_UNLOCK(&timer_lock);
	return;
}

static int timer_service_sock_init(void)
{
	if ((timer_thread_recv_fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
		cls_error("timer thread create read sock faild\n");
		return -1;
	}

	if (access(CLS_TIMER_THREAD_RECV_PATH, F_OK)) {
		if (NULL == fopen(CLS_TIMER_THREAD_RECV_PATH, "w")) {
			cls_error("can NOT create %s file for timer, error=%d(%s)\n", CLS_TIMER_THREAD_RECV_PATH, errno, strerror(errno));
			return -1;
		}
	}

	if ((dut_thread_send_fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
		cls_error("dut thread create send sock faild\n");
		return -1;
	}

	if (access(CLS_DUT_THREAD_SEND_PATH, F_OK)) {
		if (NULL == fopen(CLS_DUT_THREAD_SEND_PATH, "w")) {
			cls_error("can NOT create % file for timer, error=%d(%s)\n", CLS_DUT_THREAD_SEND_PATH, errno, strerror(errno));
			return -1;
		}
	}

	memset(&recv_addr, 0, sizeof(recv_addr));
	recv_addr.sun_family = AF_UNIX;
	strncpy(recv_addr.sun_path, CLS_TIMER_THREAD_RECV_PATH, sizeof(recv_addr.sun_path) - 1);
	unlink(recv_addr.sun_path);

	memset(&send_addr, 0, sizeof(send_addr));
	send_addr.sun_family = AF_UNIX;
	strncpy(send_addr.sun_path, CLS_DUT_THREAD_SEND_PATH, sizeof(send_addr.sun_path) - 1);
	unlink(send_addr.sun_path);

	if (bind(timer_thread_recv_fd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) == -1) {
		cls_error("timer thread recv socket bind failed errno=%d(%s)\n", errno, strerror(errno));
		close(timer_thread_recv_fd);
		timer_thread_recv_fd = -1;
		return -1;
	}

	if (bind(dut_thread_send_fd, (struct sockaddr *)&send_addr, sizeof(send_addr)) == -1) {
		cls_error("DUT thread send socket bind failed errno=%d(%s)\n", errno, strerror(errno));
		close(dut_thread_send_fd);
		dut_thread_send_fd = -1;
		return -1;
	}
	return 0;
}

static void * timer_service(void *arg)
{
	struct cls_timers_list *list = (struct cls_timers_list *)arg;
	pthread_t tid;
	int ret;

	INIT_LIST_HEAD(&list->timers);
	ret = timer_service_sock_init();

	if (ret < 0) {
		cls_error("DUT timer service init socket failed.\n");
		pthread_exit(NULL);
	}
	cls_log("timer service start\n");
	signal(SIGALRM, handle_timeout);
	pthread_create(&tid, NULL, timer_service_loop, list);
	pthread_join(tid, NULL);
}

int timer_service_init(void)
{
	pthread_t tid;

	if (pthread_create(&tid, NULL, timer_service, &dut_timer_list)) {
		cls_error("create timer thread fail for (%s)", strerror(errno));
		return -1;
	}
	pthread_detach(tid);
	return 0;
}

