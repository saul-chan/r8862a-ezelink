/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <net/cfg80211.h>
#include "cls_wifi_fw_trace.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_msg_tx.h"
#include "ipc_host.h"

#define CLS_WIFI_FW_TRACE_HEADER_LEN 4
#define CLS_WIFI_FW_TRACE_HEADER_FMT "ts=%12u ID=%8d"
#define CLS_WIFI_FW_TRACE_HEADER_ASCII_LEN (3 + 12 + 4 + 8)
#define CLS_WIFI_FW_TRACE_PARAM_FMT ", %5d"
#define CLS_WIFI_FW_TRACE_PARAM_ASCII_LEN (7)

#define TRACE_READ16(_offset) (u16)((ipc_host_trace_data_get(cls_wifi_hw->ipc_env, _offset)) & 0xffff)

#define CLS_WIFI_FW_TRACE_IPC_NB_PARAM(a) ((a >> 8) & 0xff)
#define CLS_WIFI_FW_TRACE_IPC_ID(a0, a1) (uint32_t)(((a0 & 0xff) << 16) + a1)
#define CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(a) (CLS_WIFI_FW_TRACE_IPC_NB_PARAM(a) + \
							CLS_WIFI_FW_TRACE_HEADER_LEN)

#define CLS_WIFI_FW_TRACE_NB_PARAM(a) ((*a >> 8) & 0xff)
#define CLS_WIFI_FW_TRACE_ID(a) (uint32_t)(((a[0] & 0xff) << 16) + a[1])
#define CLS_WIFI_FW_TRACE_ENTRY_SIZE(a) (CLS_WIFI_FW_TRACE_NB_PARAM(a) + \
									 CLS_WIFI_FW_TRACE_HEADER_LEN)

#define CLS_WIFI_FW_TRACE_READY  0x1234
#define CLS_WIFI_FW_TRACE_LOCKED 0xdead
#define CLS_WIFI_FW_TRACE_LOCKED_HOST 0x0230
#define CLS_WIFI_FW_TRACE_LAST_ENTRY 0xffff

#define CLS_WIFI_FW_TRACE_RESET "*** RESET ***\n"
#define CLS_WIFI_FW_TRACE_RESET_SIZE sizeof(CLS_WIFI_FW_TRACE_RESET) - 1 // don't count '\0'

static int trace_last_reset=0;

static const int startup_max_to = 500;

#if defined(MERAK2000) && MERAK2000
static uint32_t *saved_filters = NULL;
static int saved_filters_cnt = 0;
#endif

#define CLS_WIFI_FW_TRACE_CHECK_INT_MS 1000

#define NETLINK_BASE 30

int netlink_radio[CLS_WIFI_MAX_RADIOS];

struct USER_PROCESS {
	uint32_t pid;
};

struct USER_PROCESS user_process[CLS_WIFI_MAX_RADIOS];
static int initial_send[CLS_WIFI_MAX_RADIOS] = { 0, 0, 0};

struct sock *netlink_sock[CLS_WIFI_MAX_RADIOS] = {NULL, NULL, NULL};

/**
 * cls_wifi_fw_trace_empty() - Check if shared buffer is empty
 *
 * @shared_buf: Pointer to shared buffer
 */
static inline bool cls_wifi_fw_trace_empty(struct cls_wifi_fw_trace_buf *shared_buf)
{
	return (ipc_host_trace_end_get(shared_buf->cls_wifi_hw->ipc_env) >= shared_buf->size);
}

int send_entry_to_user(int idx, int _pid, char *pbuf, uint32_t len)
{
	struct sk_buff *nl_skb;
	struct nlmsghdr *nlh;
	int ret;

	nl_skb = nlmsg_new(len, GFP_ATOMIC);
	if (!nl_skb) {
		pr_err("netlink alloc failure\n");
		return -1;
	}

	nlh = nlmsg_put(nl_skb, 0, 0, netlink_radio[idx], len, 0);
	if (nlh == NULL) {
		pr_err("nlmsg_put failaure\n");
		nlmsg_free(nl_skb);
		return -1;
	}

	memcpy(nlmsg_data(nlh), pbuf, len);
	ret = netlink_unicast(netlink_sock[idx], nl_skb, _pid, MSG_DONTWAIT);

	return ret;
}

/**
 * cls_wifi_fw_trace_work() - Work function to check for new traces
 *						process function for &struct cls_wifi_fw_trace.work
 *
 * @ws: work structure
 *
 * Check if new traces are available in the shared buffer, by comparing current
 * end index with end index in the last check. If so wake up pending threads,
 * otherwise re-schedule the work is there are still some pending readers.
 *
 * Note: If between two check firmware exactly write one buffer of trace then
 * those traces will be lost. Fortunately this is very unlikely to happen.
 *
 * Note: Even if wake_up doesn't actually wake up threads (because condition
 * failed), calling waitqueue_active just after will still return false.
 * Fortunately this should never happen (new trace should always trigger the
 * waiting condition) otherwise it may be needed to re-schedule the work after
 * wake_up.
 */
static void cls_wifi_fw_trace_work(struct work_struct *ws)
{
	struct delayed_work *dw = container_of(ws, struct delayed_work, work);
	struct cls_wifi_fw_trace *trace = container_of(dw, struct cls_wifi_fw_trace, work);
	u32 cur_trace_end;

	cur_trace_end = ipc_host_trace_end_get(trace->buf.cls_wifi_hw->ipc_env);
	if (trace->closing ||
		(!cls_wifi_fw_trace_empty(&trace->buf) &&
		trace->last_read_index != cur_trace_end)) {
		trace->last_read_index = cur_trace_end;
		wake_up_interruptible(&trace->queue);
		return;
	}

	if (waitqueue_active(&trace->queue) && !delayed_work_pending(dw)) {
		schedule_delayed_work(dw, msecs_to_jiffies(CLS_WIFI_FW_TRACE_CHECK_INT_MS));
	}
}

/**
 * check and send the added data entry to user process in work function
 */
static void cls_wifi_send_entry_work(struct work_struct *ws)
{
	struct delayed_work *dw = container_of(ws, struct delayed_work, work);
	struct cls_wifi_fw_trace *trace = container_of(dw, struct cls_wifi_fw_trace, send_work);
	u32 cur_trace_end;
	u32 ptr_limit;
	u32 ptr;
	u32 entry_size;
	uint8_t log_buf[1024];
	struct cls_wifi_hw *cls_wifi_hw = trace->buf.cls_wifi_hw;

	cur_trace_end = trace->buf.offset + ipc_host_trace_end_get(trace->buf.cls_wifi_hw->ipc_env) * 2;

	ptr = trace->last_send_index;
	ptr_limit = trace->buf.offset + ipc_host_trace_size_get(trace->buf.cls_wifi_hw->ipc_env) * 2;

	if (initial_send[cls_wifi_hw->radio_idx] && ptr != cur_trace_end) {
		if (ptr < cur_trace_end) {
			ptr += CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(TRACE_READ16(ptr)) * 2;
			while (ptr <= cur_trace_end) {
				entry_size = CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(TRACE_READ16(ptr)) * 2;
				ipc_host_log_read(trace->buf.cls_wifi_hw->ipc_env, ptr, (void *)log_buf, entry_size);
				send_entry_to_user(cls_wifi_hw->radio_idx, user_process[cls_wifi_hw->radio_idx].pid,
										log_buf, entry_size);
				if (ptr == cur_trace_end)
					break;
				ptr += entry_size;
			}
		} else if (ptr > cur_trace_end) {
			while (ptr < ptr_limit) {
				if (TRACE_READ16(ptr) == CLS_WIFI_FW_TRACE_LAST_ENTRY) {
					ptr = 0;
					break;
				}
				entry_size = CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(TRACE_READ16(ptr)) * 2;
				ipc_host_log_read(trace->buf.cls_wifi_hw->ipc_env, ptr, (void *)log_buf, entry_size);
				send_entry_to_user(cls_wifi_hw->radio_idx, user_process[cls_wifi_hw->radio_idx].pid,
										log_buf, entry_size);
				ptr += entry_size;
			}
			ptr = trace->buf.offset;
			while (ptr <= cur_trace_end) {
				entry_size = CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(TRACE_READ16(ptr)) * 2;
				ipc_host_log_read(trace->buf.cls_wifi_hw->ipc_env, ptr, (void *)log_buf, entry_size);
				send_entry_to_user(cls_wifi_hw->radio_idx, user_process[cls_wifi_hw->radio_idx].pid,
										log_buf, entry_size);
				if (ptr == cur_trace_end)
					break;
				ptr += entry_size;
			}
		}
		trace->last_send_index = ptr;
	}
	schedule_delayed_work(&trace->send_work, msecs_to_jiffies(100));
}

/**
 * cls_wifi_fw_trace_buf_lock() - Lock trace buffer for firmware
 *
 * @shared_buf: Pointer to shared buffer
 *
 * Very basic synchro mechanism so that fw do not update trace buffer while host
 * is reading it. Not safe to race condition if host and fw read lock value at
 * the "same" time.
 */
static void cls_wifi_fw_trace_buf_lock(struct cls_wifi_fw_trace_buf *shared_buf)
{
	u32 lock;
  wait:
	while((lock = ipc_host_trace_pattern_get(shared_buf->cls_wifi_hw->ipc_env)) == CLS_WIFI_FW_TRACE_LOCKED) {}
	lock &= CLS_WIFI_FW_TRACE_LOCKED_HOST;
	ipc_host_trace_pattern_set(shared_buf->cls_wifi_hw->ipc_env, lock);

	/* re-read to reduce race condition window */
	if ((lock = ipc_host_trace_pattern_get(shared_buf->cls_wifi_hw->ipc_env)) == CLS_WIFI_FW_TRACE_LOCKED)
		goto wait;
}

/**
 * cls_wifi_fw_trace_buf_unlock() - Unlock trace buffer for firmware
 *
 * @shared_buf: Pointer to shared buffer
 *
 */
static void cls_wifi_fw_trace_buf_unlock(struct cls_wifi_fw_trace_buf *shared_buf)
{
	ipc_host_trace_pattern_set(shared_buf->cls_wifi_hw->ipc_env, CLS_WIFI_FW_TRACE_READY);
}

/**
 * cls_wifi_fw_trace_buf_init() - Initialize cls_wifi_fw_trace_buf structure
 *
 * @shared_buf: Structure to initialize
 * @ipc: Pointer to IPC shard structure that contains trace buffer info
 *
 *
 * Return: 0 if initialization succeed, <0 otherwise. It can only fail if
 * trace feature is not enabled in the firmware (or buffer is corrupted).
 */
int cls_wifi_fw_trace_buf_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_fw_trace_buf *shared_buf)
{
	uint16_t lock_status = ipc_host_trace_pattern_get(cls_wifi_hw->ipc_env);

	if ((lock_status != CLS_WIFI_FW_TRACE_READY &&
		 lock_status != CLS_WIFI_FW_TRACE_LOCKED)) {
		shared_buf->offset = 0;
		return -ENOENT;
	}

	/* Buffer starts <offset> bytes from the location of ipc->offset */
	shared_buf->cls_wifi_hw = cls_wifi_hw;
	shared_buf->size = ipc_host_trace_size_get(cls_wifi_hw->ipc_env);
	shared_buf->offset = ipc_host_trace_offset_get(cls_wifi_hw->ipc_env);
	//shared_buf->lock = &ipc->pattern;
	//shared_buf->start = &ipc->start;
	//shared_buf->end = &ipc->end;
	//shared_buf->data = (uint16_t *)((uint8_t *)(&ipc->offset) + ipc->offset);
	shared_buf->reset_idx = ++trace_last_reset;

	wiphy_info(cls_wifi_hw->wiphy, "---trace size [%08x]\n", shared_buf->size);
	wiphy_info(cls_wifi_hw->wiphy, "---trace offset [%08x]\n", shared_buf->offset);

	/* backward compatibilty with firmware without trace activation */
	if ((ipc_host_trace_nb_compo_get(cls_wifi_hw->ipc_env) >> 16) == CLS_WIFI_FW_TRACE_READY) {
		shared_buf->nb_compo = ipc_host_trace_nb_compo_get(cls_wifi_hw->ipc_env) & 0xffff;
	} else {
		shared_buf->nb_compo = 0;
	}

	return 0;
}

/**
 * netlink sock receive msg for user process and get radio info
 */
void netlink_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	char *data = NULL;
	int idx = 0;

	nlh = nlmsg_hdr(skb);

	if (skb->len >= NLMSG_SPACE(0)) {
		data = NLMSG_DATA(nlh);
		if (data) {
			idx = data[0] - '0';
			user_process[idx].pid = nlh->nlmsg_pid;
			pr_err("---recv from user pid %d:%s\n", user_process[idx].pid, data);
			initial_send[idx] = 1;

		}
	} else
		pr_err("%s:err skb,length:%d\n", __func__, skb->len);
}

/**
 * creat netlink sock for the radio
 */
int init_fw_trace_netlink(int idx, struct sock **psock)
{
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv_msg,
		.groups = 0,
	};
	netlink_radio[idx] = NETLINK_BASE + idx;
	*psock = (struct sock *)netlink_kernel_create(&init_net, netlink_radio[idx], &cfg);
	if (*psock == NULL) {
		pr_err("can't create a netlink sock\n");
		return -1;
	}
	netlink_sock[idx] = *psock;
	return 0;
}

/**
 * cls_wifi_fw_trace_init() - Initialize cls_wifi_fw_trace structure
 *
 * @trace: Structure to initialize
 * @ipc: Pointer to IPC shard structure that contains trace buffer info
 *
 * Return: 0 if initialization succeed, <0 otherwise. It can only fail if
 * trace feature is not enabled in the firmware (or buffer is corrupted).
 */
int cls_wifi_fw_trace_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_fw_trace *trace)
{
	if (cls_wifi_fw_trace_buf_init(cls_wifi_hw, &trace->buf))
		return -ENOENT;

	INIT_DELAYED_WORK(&trace->work, cls_wifi_fw_trace_work);
	INIT_DELAYED_WORK(&trace->send_work, cls_wifi_send_entry_work);
	init_waitqueue_head(&trace->queue);
	mutex_init(&trace->mutex);
	trace->closing = false;
	if (init_fw_trace_netlink(cls_wifi_hw->radio_idx, &trace->nlfd))
		return -ENOENT;
	if (trace->nlfd != NULL)
		wiphy_info(cls_wifi_hw->wiphy, "%s:trace->nlfd = %p\n", __func__, trace->nlfd);
	trace->last_send_index = trace->buf.offset;
	schedule_delayed_work(&trace->send_work, msecs_to_jiffies(1000));
	return 0;
}

/**
 * cls_wifi_fw_trace_deinit() - De-initialization before releasing cls_wifi_fw_trace
 *
 * @trace: fw trace control structure
 */
void cls_wifi_fw_trace_deinit(struct cls_wifi_fw_trace *trace)
{
	if (trace->nlfd) {
		netlink_kernel_release(trace->nlfd);
		trace->nlfd = NULL;
	}
	trace->closing = true;
	if (trace->work.work.func)
		cancel_delayed_work_sync(&trace->work);
	if (trace->send_work.work.func)
		cancel_delayed_work_sync(&trace->send_work);
	trace->buf.offset = 0;
}

/**
 * cls_wifi_fw_trace_reset_local() - Reset local buffer pointer/status
 *
 * @local_buf: structure to reset
 */
static void cls_wifi_fw_trace_reset_local(struct cls_wifi_fw_trace_local_buf *local_buf)
{
	local_buf->read = local_buf->data;
	local_buf->write = local_buf->data;
	local_buf->nb_entries = 0;
	local_buf->free_space = local_buf->size;
	local_buf->last_read = 0;
	local_buf->reset_idx = 0;
	local_buf->show_reset = NULL;
}

/**
 * cls_wifi_fw_trace_alloc_local() - Allocate a local buffer and initialize
 * cls_wifi_fw_trace_local_buf structure
 *
 * @local_buf: structure to initialize
 * @size: Size of the buffer to allocate
 *
 * @local structure is initialized to use the allocated buffer.
 *
 * Return: 0 if allocation succeed and <0 otherwise.
 */
int cls_wifi_fw_trace_alloc_local(struct cls_wifi_fw_trace_local_buf *local_buf,
							  int size)
{
	local_buf->data = kmalloc(size * sizeof(uint16_t), GFP_KERNEL);
	if (!local_buf->data) {
		return -ENOMEM;
	}

	local_buf->data_end = local_buf->data + size;
	local_buf->size = size;
	cls_wifi_fw_trace_reset_local(local_buf);
	return 0;
}

/**
 * cls_wifi_fw_trace_free_local() - Free local buffer
 *
 * @local_buf: structure containing buffer pointer to free.
 */
void cls_wifi_fw_trace_free_local(struct cls_wifi_fw_trace_local_buf *local_buf)
{
	if (local_buf->data)
		kfree(local_buf->data);
	local_buf->data = NULL;
}

/**
 * cls_wifi_fw_trace_strlen() - Return buffer size needed convert a trace entry into
 * string
 *
 * @entry: Pointer on trace entry
 *
 */
static inline int cls_wifi_fw_trace_strlen(uint16_t *entry)
{
	return (CLS_WIFI_FW_TRACE_HEADER_ASCII_LEN +
			(CLS_WIFI_FW_TRACE_NB_PARAM(entry) * CLS_WIFI_FW_TRACE_PARAM_ASCII_LEN) +
			1); /* for \n */
}

/**
 * cls_wifi_fw_trace_to_str() - Convert one trace entry to a string
 *
 * @trace: Poitner to the trace entry
 * @buf: Buffer for the string
 * @size: Size of the string buffer, updated with the actual string size
 *
 * Return: pointer to the next tag entry.
 */
static uint16_t *cls_wifi_fw_trace_to_str(uint16_t *trace, char *buf, size_t *size)
{
	uint32_t ts, id;
	int nb_param;
	int res, buf_idx = 0, left = *size;

	id = CLS_WIFI_FW_TRACE_ID(trace);
	nb_param = CLS_WIFI_FW_TRACE_NB_PARAM(trace);

	trace +=2;
	ts = *trace++;
	ts <<= 16;
	ts += *trace++;

	res = scnprintf(&buf[buf_idx], left, CLS_WIFI_FW_TRACE_HEADER_FMT, ts, id);
	buf_idx += res;
	left	-= res;

	while (nb_param > 0) {
		res = scnprintf(&buf[buf_idx], left, CLS_WIFI_FW_TRACE_PARAM_FMT, *trace++);
		buf_idx += res;
		left	-= res;
		nb_param--;
	}

	res = scnprintf(&buf[buf_idx], left, "\n");
	left -= res;
	*size = (*size - left);

	return trace;
}

/**
 * cls_wifi_fw_trace_ipc_to_str() - Convert one trace entry to a string
 *
 * @trace: Poitner to the trace entry
 * @buf: Buffer for the string
 * @size: Size of the string buffer, updated with the actual string size
 *
 * Return: pointer to the next tag entry.
 */
static u32 cls_wifi_fw_trace_ipc_to_str(struct cls_wifi_hw *cls_wifi_hw, u32 trace, char *buf, size_t *size)
{
	uint32_t ts, id;
	int nb_param;
	int res, buf_idx = 0, left = *size;

	id = CLS_WIFI_FW_TRACE_IPC_ID(TRACE_READ16(trace), TRACE_READ16((trace + 2)));
	nb_param = CLS_WIFI_FW_TRACE_IPC_NB_PARAM(TRACE_READ16(trace));

	trace +=4;
	ts = TRACE_READ16(trace);
	trace +=2;
	ts <<= 16;
	ts += TRACE_READ16(trace);
	trace +=2;

	res = scnprintf(&buf[buf_idx], left, CLS_WIFI_FW_TRACE_HEADER_FMT, ts, id);
	buf_idx += res;
	left	-= res;

	while (nb_param > 0) {
		res = scnprintf(&buf[buf_idx], left, CLS_WIFI_FW_TRACE_PARAM_FMT, TRACE_READ16(trace));
		trace +=2;
		buf_idx += res;
		left	-= res;
		nb_param--;
	}

	res = scnprintf(&buf[buf_idx], left, "\n");
	left -= res;
	*size = (*size - left);

	return trace;
}

/**
 * cls_wifi_fw_trace_copy_entry() - Copy one trace entry in a local buffer
 *
 * @local_buf: Local buffer to copy trace into
 * @trace_entry: Pointer to the trace entry (in shared memory) to copy
 * @size: Size, in 16bits words, of the trace entry
 *
 * It is assumed that local has enough contiguous free-space available in
 * local buffer (i.e. from local_buf->write) to copy this trace.
 */
static void cls_wifi_fw_trace_copy_entry(struct cls_wifi_fw_trace_local_buf *local_buf,
									 struct cls_wifi_hw *cls_wifi_hw, u32 trace_entry, int size)
{
	uint16_t *write = local_buf->write;
	u32 read = trace_entry;
	int i;

	for (i = 0; i < size;) {
		*write++ = TRACE_READ16(read);
		read += 2;
		i += 2;
	}

	if (write >= local_buf->data_end)
		local_buf->write = local_buf->data;
	else
		local_buf->write = write;

	local_buf->free_space -= size;
	local_buf->last_read = trace_entry;
	local_buf->last_read_value = TRACE_READ16(trace_entry);
	local_buf->nb_entries++;
}

/**
 * cls_wifi_fw_trace_copy() - Copy trace entries from shared to local buffer
 *
 * @trace_buf: Pointer to shard buffer
 * @local_buf: Pointer to local buffer
 *
 * Copy has many trace entry as possible from shared buffer to local buffer
 * without overwriting traces in local buffer.
 *
 * Return: number of trace entries copied to local buffer
 */
static int cls_wifi_fw_trace_copy(struct cls_wifi_fw_trace *trace,
							  struct cls_wifi_fw_trace_local_buf *local_buf)
{
	struct cls_wifi_fw_trace_buf *trace_buf = &trace->buf;
	//treat as offset instead of pointer
	u32 ptr, ptr_end, ptr_limit;
	int entry_size, ret = 0;
	struct cls_wifi_hw *cls_wifi_hw = trace_buf->cls_wifi_hw;

	if (mutex_lock_interruptible(&trace->mutex))
		return 0;

	/* reset last_read ptr if shared buffer has been reset */
	if (local_buf->reset_idx != trace_buf->reset_idx) {
		local_buf->show_reset = local_buf->write;
		local_buf->reset_idx = trace_buf->reset_idx;
		local_buf->last_read = 0;
	}

	cls_wifi_fw_trace_buf_lock(trace_buf);

	ptr_end = trace_buf->offset + ipc_host_trace_end_get(cls_wifi_hw->ipc_env) * 2;
	if (cls_wifi_fw_trace_empty(trace_buf) || (ptr_end == local_buf->last_read))
		goto end;
	ptr_limit = trace_buf->offset + ipc_host_trace_size_get(cls_wifi_hw->ipc_env) * 2;

	if (local_buf->last_read &&
		(local_buf->last_read_value == TRACE_READ16(local_buf->last_read))) {
		ptr = local_buf->last_read;
		ptr += CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(TRACE_READ16(ptr)) * 2;
	} else {
		ptr = trace_buf->offset + ipc_host_trace_start_get(cls_wifi_hw->ipc_env) * 2;
	}

	while (1) {

		if ((ptr == ptr_limit) || (TRACE_READ16(ptr) == CLS_WIFI_FW_TRACE_LAST_ENTRY))
			 ptr = trace_buf->offset;

		entry_size = CLS_WIFI_FW_TRACE_IPC_ENTRY_SIZE(TRACE_READ16(ptr)) * 2;

		if ((ptr + entry_size) > ptr_limit) {
			pr_err("Corrupted trace buffer\n");
			_cls_wifi_fw_trace_reset(trace, false);
			break;
		} else if (entry_size > local_buf->size) {
			pr_err("FW_TRACE local buffer too small, trace skipped");
			goto next_entry;
		}

		if (local_buf->free_space >= entry_size) {
			int contiguous = local_buf->data_end - local_buf->write;

			if ((local_buf->write < local_buf->read) || contiguous >= entry_size) {
				/* enough contiguous memory from local_buf->write */
				cls_wifi_fw_trace_copy_entry(local_buf, cls_wifi_hw, ptr, entry_size);
				ret++;
			} else if ((local_buf->free_space - contiguous) >= entry_size) {
				/* not enough contiguous from local_buf->write but enough
				   from local_buf->data */
				*local_buf->write = CLS_WIFI_FW_TRACE_LAST_ENTRY;
				if (local_buf->show_reset == local_buf->write)
					local_buf->show_reset = local_buf->data;
				local_buf->write = local_buf->data;
				local_buf->free_space -= contiguous;
				cls_wifi_fw_trace_copy_entry(local_buf, cls_wifi_hw, ptr, entry_size);
				ret++;
			} else {
				/* not enough contiguous memory */
				goto end;
			}
		} else {
			goto end;
		}

		if (ptr == ptr_end)
			break;

	  next_entry:
		ptr += entry_size;
	}

  end:
	cls_wifi_fw_trace_buf_unlock(trace_buf);
	mutex_unlock(&trace->mutex);
	return ret;
}

/**
 * cls_wifi_fw_trace_read_local() - Read trace from local buffer and convert it to
 * string in a user buffer
 *
 * @local_buf: Pointer to local buffer
 * @user_buf: Pointer to user buffer
 * @size: Size of the user buffer
 *
 * Read traces from shared buffer to write them in the user buffer after string
 * conversion. Stop when no more space in user buffer or no more trace to read.
 *
 * Return: The size written in the user buffer.
 */
static size_t cls_wifi_fw_trace_read_local(struct cls_wifi_fw_trace_local_buf *local_buf,
									   char __user *user_buf, size_t size)
{
	uint16_t *ptr;
	char str[1824]; // worst case 255 params
	size_t str_size;
	int entry_size;
	size_t res = 0 , remain = size, not_cpy = 0;

	if (!local_buf->nb_entries)
		return res;

	ptr = local_buf->read;
	while(local_buf->nb_entries && !not_cpy) {

		if (local_buf->show_reset == ptr) {
			if (remain < CLS_WIFI_FW_TRACE_RESET_SIZE)
				break;

			local_buf->show_reset = NULL;
			not_cpy = copy_to_user(user_buf + res, CLS_WIFI_FW_TRACE_RESET,
								   CLS_WIFI_FW_TRACE_RESET_SIZE);
			res += (CLS_WIFI_FW_TRACE_RESET_SIZE - not_cpy);
			remain -= (CLS_WIFI_FW_TRACE_RESET_SIZE - not_cpy);
		}

		if (remain < cls_wifi_fw_trace_strlen(ptr))
			break;

		entry_size = CLS_WIFI_FW_TRACE_ENTRY_SIZE(ptr);
		str_size = sizeof(str);
		ptr = cls_wifi_fw_trace_to_str(ptr, str, &str_size);
		not_cpy = copy_to_user(user_buf + res, str, str_size);
		str_size -= not_cpy;
		res += str_size;
		remain -= str_size;

		local_buf->nb_entries--;
		local_buf->free_space += entry_size;
		if (ptr >= local_buf->data_end) {
			ptr = local_buf->data;
		} else if (*ptr == CLS_WIFI_FW_TRACE_LAST_ENTRY) {
			local_buf->free_space += local_buf->data_end - ptr;
			ptr = local_buf->data;
		}
		local_buf->read = ptr;
	}

	/* read all entries reset pointer */
	if ( !local_buf->nb_entries) {

		local_buf->write = local_buf->read = local_buf->data;
		local_buf->free_space = local_buf->size;
	}

	return res;
}

/**
 * cls_wifi_fw_trace_read() - Update local buffer from shared buffer and convert
 * local buffer to string in user buffer
 *
 * @trace: Fw trace control structure
 * @local_buf: Local buffer to update and read from
 * @dont_wait: Indicate whether function should wait or not for traces before
 * returning
 * @user_buf: Pointer to user buffer
 * @size: Size of the user buffer
 *
 * Read traces from shared buffer to write them in the user buffer after string
 * conversion. Stop when no more space in user buffer or no more trace to read.
 *
 * Return: The size written in the user buffer if > 0, -EAGAIN if there is no
 * new traces and dont_wait is set and -ERESTARTSYS if signal has been
 * received while waiting for new traces.
 */
size_t cls_wifi_fw_trace_read(struct cls_wifi_fw_trace *trace,
						  struct cls_wifi_fw_trace_local_buf *local_buf,
						  bool dont_wait, char __user *user_buf, size_t size)
{
	size_t res = 0;

	cls_wifi_fw_trace_copy(trace, local_buf);

	while(!local_buf->nb_entries) {
		int last_index;

		if (dont_wait)
			return -EAGAIN;

		/* no trace, schedule work to periodically check trace buffer */
		if (!delayed_work_pending(&trace->work)) {
			trace->last_read_index = trace->buf.offset +
										ipc_host_trace_end_get(trace->buf.cls_wifi_hw->ipc_env) * 2;
			schedule_delayed_work(&trace->work,
								  msecs_to_jiffies(CLS_WIFI_FW_TRACE_CHECK_INT_MS));
		}

		/* and wait for traces */
		last_index = trace->buf.offset + ipc_host_trace_end_get(trace->buf.cls_wifi_hw->ipc_env) * 2;
		if (wait_event_interruptible(trace->queue,
									 (trace->closing ||
									  (last_index != trace->buf.offset +
											ipc_host_trace_end_get(trace->buf.cls_wifi_hw->ipc_env) * 2)))) {
			return -ERESTARTSYS;
		}

		if (trace->closing)
			return 0;

		cls_wifi_fw_trace_copy(trace, local_buf);
	}

	/* copy as many traces as possible in user buffer */
	while (1) {
		size_t read;
		read = cls_wifi_fw_trace_read_local(local_buf, user_buf + res, size - res);
		res += read;
		cls_wifi_fw_trace_copy(trace, local_buf);
		if (!read)
			break;
	}

	return res;
}


/**
 * _cls_wifi_fw_trace_dump() - Dump shared trace buffer in kernel buffer
 *
 * @trace_buf: Pointer to shared trace buffer;
 *
 * Called when error is detected, output trace on dmesg directly read from
 * shared memory
 */
void _cls_wifi_fw_trace_dump(struct cls_wifi_fw_trace_buf *trace_buf)
{
	u32 ptr, ptr_end, ptr_limit, next_ptr;
	struct cls_wifi_hw *cls_wifi_hw = trace_buf->cls_wifi_hw;
	char buf[1824]; // worst case 255 params
	size_t size;

	if (!trace_buf->offset || cls_wifi_fw_trace_empty(trace_buf))
		return;

	cls_wifi_fw_trace_buf_lock(trace_buf);

	ptr = trace_buf->offset + ipc_host_trace_start_get(trace_buf->cls_wifi_hw->ipc_env);
	ptr_end = trace_buf->offset + ipc_host_trace_end_get(trace_buf->cls_wifi_hw->ipc_env);
	ptr_limit = trace_buf->offset + trace_buf->size;

	while (1) {
		size = sizeof(buf);
		next_ptr = cls_wifi_fw_trace_ipc_to_str(cls_wifi_hw, ptr, buf, &size);
		pr_info("%s", buf);

		if (ptr == ptr_end) {
			break;
		} else if ((next_ptr == ptr_limit) ||
				   (TRACE_READ16(next_ptr) == CLS_WIFI_FW_TRACE_LAST_ENTRY)) {
			ptr = trace_buf->offset;
		} else if (next_ptr > ptr_limit) {
			pr_err("Corrupted trace buffer\n");
			break;
		} else {
			ptr = next_ptr;
		}
	}

	cls_wifi_fw_trace_buf_unlock(trace_buf);
}

/**
 * _cls_wifi_fw_trace_reset() - Reset trace buffer at firmware level
 *
 * @trace: Pointer to shared trace buffer;
 * @bool: Indicate if mutex must be aquired before
 */
int _cls_wifi_fw_trace_reset(struct cls_wifi_fw_trace *trace, bool lock)
{
	struct cls_wifi_fw_trace_buf *trace_buf = &trace->buf;

	if (lock && mutex_lock_interruptible(&trace->mutex))
		return -ERESTARTSYS;

	if (trace->buf.offset) {
		cls_wifi_fw_trace_buf_lock(trace_buf);
		ipc_host_trace_start_set(trace_buf->cls_wifi_hw->ipc_env, 0);
		ipc_host_trace_end_set(trace_buf->cls_wifi_hw->ipc_env, trace_buf->size + 1);
		trace_buf->reset_idx = ++trace_last_reset;
		cls_wifi_fw_trace_buf_unlock(trace_buf);
	}

	if (lock)
		mutex_unlock(&trace->mutex);
	return 0;
}

/**
 * cls_wifi_fw_trace_get_trace_level() - Get trace level for a given component
 *
 * @trace: Pointer to shared trace buffer;
 * @compo_id: Index of the componetn in the table
 *
 * Return: The trace level set for the given component, 0 if component index
 * is invalid.
 */
#if defined(MERAK2000) && MERAK2000
static uint32_t cls_wifi_fw_trace_get_trace_level(struct cls_wifi_fw_trace_buf *trace_buf,
											  unsigned int compo_id)
{
	if (compo_id >= trace_buf->nb_compo)
		return 0;
	return ipc_host_trace_compo_get(trace_buf->cls_wifi_hw->ipc_env, compo_id);
}
#endif

/**
 * cls_wifi_fw_trace_set_trace_level() - Set trace level for a given component
 *
 * @trace_buf: Pointer to shared trace buffer;
 * @compo_id: Index of the componetn in the table
 * @level: Trace level to set
 *
 * Set all components if compo_id is equals to the number of component and
 * does nothing if it is greater.
 */
#if defined(MERAK2000) && MERAK2000
static void cls_wifi_fw_trace_set_trace_level(struct cls_wifi_fw_trace_buf *trace_buf,
										  unsigned int compo_id, uint32_t level)
{
	struct cls_wifi_hw *cls_wifi_hw;
	struct dbg_trace_loglevel_set_cfm cfm;

	if (compo_id > trace_buf->nb_compo)
		return;

	cls_wifi_hw =  trace_buf->cls_wifi_hw;
	if (compo_id == trace_buf->nb_compo) {
		int i;
		for (i = 0; i < trace_buf->nb_compo; i++) {
			//ipc_host_trace_compo_set(trace_buf->cls_wifi_hw->ipc_env, i, level);
			cls_wifi_send_set_trace_loglevel(cls_wifi_hw, cls_wifi_hw->radio_idx, compo_id, level, &cfm);
		}
	} else {
		cls_wifi_send_set_trace_loglevel(cls_wifi_hw, cls_wifi_hw->radio_idx, compo_id, level, &cfm);
	}
}
#endif

/**
 * cls_wifi_fw_trace_level_read() - Write current trace level in a user buffer
 *							  as a string
 *
 * @trace: Fw trace control structure
 * @user_buf: Pointer to user buffer
 * @len: Size of the user buffer
 * @ppos: position offset
 *
 * Return: Number of bytes written in user buffer if > 0, error otherwise
 */
 #if defined(MERAK2000) && MERAK2000
size_t cls_wifi_fw_trace_level_read(struct cls_wifi_fw_trace *trace,
								char __user *user_buf, size_t len, loff_t *ppos)
{
	struct cls_wifi_fw_trace_buf *trace_buf = &trace->buf;
	size_t res = 0;
	int i, size;
	char *buf;
	struct cls_wifi_hw *cls_wifi_hw = trace_buf->cls_wifi_hw;
	struct dbg_trace_loglevel_get_cfm cfm;

	size = trace_buf->nb_compo * 16;
	buf = kmalloc(size, GFP_KERNEL);
	if (buf == NULL)
		return 0;

	if (mutex_lock_interruptible(&trace->mutex)) {
		kfree(buf);
		return -ERESTARTSYS;
	}
	memset(&cfm, 0, sizeof(cfm));
	cls_wifi_send_get_trace_loglevel(cls_wifi_hw, cls_wifi_hw->radio_idx, &cfm);

	for (i = 0 ; i < trace_buf->nb_compo ; i ++) {
		res += scnprintf(&buf[res], size - res, "%3d:0x%08x\n", i, cfm.mod_level_list[i]);
	}
	mutex_unlock(&trace->mutex);

	res = simple_read_from_buffer(user_buf, len, ppos, buf, res);

	kfree(buf);
	return res;
}
#endif

/**
 * cls_wifi_fw_trace_level_write() - Read trace level from  a user buffer provided
 *							   as a string and apply them.
 *
 * @trace: Fw trace control structure
 * @user_buf: Pointer to user buffer
 * @len: Size of the user buffer
 *
 * trace level must be provided in the following form:
 * <compo_id>:<trace_level> where <compo_id> is in decimal notation and
 * <trace_level> in decical or hexadecimal notation.
 * Several trace level can be provided, separated by space,tab or new line.
 *
 * Return: Number of bytes read form user buffer if > 0, error otherwise
 */
#if defined(MERAK2000) && MERAK2000
size_t cls_wifi_fw_trace_level_write(struct cls_wifi_fw_trace *trace,
								 const char __user *user_buf, size_t len)
{
	struct cls_wifi_fw_trace_buf *trace_buf = &trace->buf;
	char *buf, *token, *next;

	buf = kmalloc(len + 1, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	if (copy_from_user(buf, user_buf, len)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[len] = '\0';

	if (mutex_lock_interruptible(&trace->mutex)) {
		kfree(buf);
		return -ERESTARTSYS;
	}

	next = buf;
	token = strsep(&next, " \t\n");
	while (token) {
		unsigned int compo, level;
		if ((sscanf(token, "%d:0x%x", &compo, &level) == 2)||
			(sscanf(token, "%d:%d", &compo, &level) == 2)) {
			cls_wifi_fw_trace_set_trace_level(trace_buf, compo, level);
		}

		token = strsep(&next, " \t");
	}
	mutex_unlock(&trace->mutex);

	kfree(buf);
	return len;
}
#endif
/**
 * cls_wifi_fw_trace_config_filters() - Update FW trace filters
 *
 * @trace_buf: Pointer to shared buffer
 * @ipc: Pointer to IPC shared structure that contains trace buffer info
 * @ftl: Firmware trace level
 *
 * Return: 0 if the trace filters are successfully updated, <0 otherwise.
 */
#if defined(MERAK2000) && MERAK2000
int cls_wifi_fw_trace_config_filters(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_fw_trace_buf *trace_buf, char *ftl)
{
	int to;
	char *next, *token;

	to = 0;
	while((ipc_host_trace_pattern_get(cls_wifi_hw->ipc_env) != CLS_WIFI_FW_TRACE_READY) && (to < startup_max_to))
	{
		msleep(50);
		to += 50;
	}

	if (cls_wifi_fw_trace_buf_init(cls_wifi_hw, trace_buf))
		return -ENOENT;

	next = ftl;
	token = strsep(&next, " ");
	while(token)
	{
		unsigned int compo, ret, id, level = 0;
		char action;

		if ((sscanf(token, "%d%c0x%x", &compo, &action, &id) == 3)||
			(sscanf(token, "%d%c%d", &compo, &action, &id) == 3))
		{
			if(action == '=')
			{
				level = id;
			}
			else
			{
				ret = cls_wifi_fw_trace_get_trace_level(trace_buf, compo);
				if(action == '+')
					level = (ret | id);
				else if (action == '-')
					level = (ret & ~id);
			}
			cls_wifi_fw_trace_set_trace_level(trace_buf, compo, level);
		}

		token = strsep(&next, " ");
	}

	return 0;
}
#endif

/**
 * cls_wifi_fw_trace_save_filters() - Save filters currently configured so that
 * they can be restored with cls_wifi_fw_trace_restore_filters()
 *
 * @trace: Fw trace control structure
 * @return 0 if filters have been saved and != 0 in case on error
 */
#if defined(MERAK2000) && MERAK2000
int cls_wifi_fw_trace_save_filters(struct cls_wifi_fw_trace *trace)
{
	int i;

	if (saved_filters)
		kfree(saved_filters);

	saved_filters_cnt = trace->buf.nb_compo;
	saved_filters = kmalloc(saved_filters_cnt * sizeof(uint32_t), GFP_KERNEL);
	if (!saved_filters)
		return -1;

	for (i = 0; i < saved_filters_cnt; i++) {
		saved_filters[i] = cls_wifi_fw_trace_get_trace_level(&trace->buf, i);
	}

	return 0;
}
#endif
/**
 * cls_wifi_fw_trace_restore_filters() - Restore filters previoulsy saved
 * by cls_wifi_fw_trace_save_filters()
 *
 * @trace: Fw trace control structure
 * @return 0 if filters have been restored and != 0 in case on error
 */
#if defined(MERAK2000) && MERAK2000
int cls_wifi_fw_trace_restore_filters(struct cls_wifi_fw_trace *trace)
{
	int i;

	if (!saved_filters || (trace->buf.offset == 0))
		return -1;

	if (saved_filters_cnt != trace->buf.nb_compo) {
		pr_warn("Number of trace components change between saved and restore\n");
		if (saved_filters_cnt > trace->buf.nb_compo) {
			saved_filters_cnt = trace->buf.nb_compo;
		}
	}

	for (i = 0; i < saved_filters_cnt; i++) {
		cls_wifi_fw_trace_set_trace_level(&trace->buf, i, saved_filters[i]);
	}

	kfree(saved_filters);
	saved_filters = NULL;
	saved_filters_cnt = 0;

	return 0;
}
#endif
