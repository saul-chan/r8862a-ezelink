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

#ifndef _CLS_WIFI_FW_TRACE_H_
#define _CLS_WIFI_FW_TRACE_H_

#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

/**
 * struct cls_wifi_fw_trace_desc - Trace buffer info as provided by fw in ipc
 *
 * @pattern: Synchro pattern
 * @start: Index of first entry in trace buffer
 * @end: Index of last entry in trace buffer
 * @size: Size, in 16bits words, od the trace buffer
 * @offset: Offset, in bytest, to the start of the buffer from the address of
 * this field.
 * @nb_compo: Number of filterable component (16LSB) synchro pattern (16 MSB)
 * @offset_compo: Offset, in bytest, to the start of the component activation
 * table from the address of this field.
 */
struct cls_wifi_fw_trace_ipc_desc {
	volatile uint32_t pattern;
	volatile uint32_t start;
	volatile uint32_t end;
	volatile uint32_t size;
	volatile uint32_t offset;
	volatile uint32_t nb_compo;
	volatile uint32_t offset_compo;
};

/**
 * struct cls_wifi_fw_trace_buf - Info for trace buffer in shared memory
 *
 * @lock: Address of the synchro word
 * @data: Address of the trace buffer
 * @size: Size, in 16bits words, of the trace buffer
 * @start: Address on the current index (in 16 bits words) of the first trace
 * entry.
 * @end: Address on the current index (in 16 bits words) of the last trace
 * entry. If *end > size, it means that the trace buffer contains no traces.
 * @reset_idx: Increased each time the trace buffer is reset
 * (by calling _cls_wifi_fw_trace_reset())
 * @nb_compo: Size of the compo_table
 * @compo_table: Table containing component filter status.
 */
struct cls_wifi_fw_trace_buf {
	volatile uint32_t *lock;
	uint16_t *data;
	uint32_t offset;
	uint32_t size;
	volatile uint32_t *start;
	volatile uint32_t *end;
	int reset_idx;
	unsigned int nb_compo;
	uint32_t *compo_table;
	struct cls_wifi_hw *cls_wifi_hw;
};

/**
 * struct cls_wifi_fw_trace_local_buf - Info for a local trace buffer
 *
 * @data: Address of the local buffer
 * @data_end: Address after the end of the local buffer
 * @size: Size, in 16bits words, oth the local buffer
 * @read: Pointer to the next trace entry to read
 * @write: Pointer to the next entry to write
 * @nb_entries: Number of trace entries ready to be read
 * @free_space: Free space, in 16bits words, in the buffer.
 * @last_read: Address of the last entry read in the shared buffer
 * @last_read_value: First word of the last trace entry read.
 * @reset_idx: Reset index. If it doesn't match share buffer index then it
 * means that share buffer has been resetted since last read.
 */
struct cls_wifi_fw_trace_local_buf {
	uint16_t *data;
	uint16_t *data_end;
	uint32_t size;
	uint16_t *read;
	uint16_t *write;
	uint16_t nb_entries;
	uint32_t free_space;
	//treat as offset instead of pointer
	uint32_t last_read;
	uint16_t last_read_value;
	int reset_idx;
	uint16_t * show_reset;
};


/**
 * struct cls_wifi_fw_trace - info to handle several reader of the shared buffer
 *
 * @buf: shared trace buffer.
 * @mutex: mutex, used to prevent several reader updating shared buffer at the
 * same time.
 * @queue: queue, used to delay reader.
 * @work: work used to periodically check for new traces in shared buffer.
 * @last_read_index: Last read index from shared buffer
 * @closing: Indicate whether is driver is being removed, meaning that reader
 * should no longer wait no new traces
 */
struct cls_wifi_fw_trace {
	struct cls_wifi_fw_trace_buf buf;
	struct mutex mutex;
	wait_queue_head_t queue;
	struct delayed_work work;
	struct delayed_work send_work;
	int last_read_index;
	int last_send_index;
	bool closing;
	struct sock *nlfd;
};

int cls_wifi_fw_trace_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_fw_trace *trace);
void cls_wifi_fw_trace_deinit(struct cls_wifi_fw_trace *trace);

int cls_wifi_fw_trace_buf_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_fw_trace_buf *shared_buf);

int _cls_wifi_fw_trace_reset(struct cls_wifi_fw_trace *trace, bool lock);
void _cls_wifi_fw_trace_dump(struct cls_wifi_fw_trace_buf *trace);

int cls_wifi_fw_trace_alloc_local(struct cls_wifi_fw_trace_local_buf *local,
							  int size);
void cls_wifi_fw_trace_free_local(struct cls_wifi_fw_trace_local_buf *local);


size_t cls_wifi_fw_trace_read(struct cls_wifi_fw_trace *trace,
						  struct cls_wifi_fw_trace_local_buf *local_buf,
						  bool dont_wait, char __user *user_buf, size_t size);


size_t cls_wifi_fw_trace_level_read(struct cls_wifi_fw_trace *trace,
								char __user *user_buf, size_t len, loff_t *ppos);
size_t cls_wifi_fw_trace_level_write(struct cls_wifi_fw_trace *trace,
								 const char __user *user_buf, size_t len);


int cls_wifi_fw_trace_config_filters(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_fw_trace_buf *trace_buf, char *ftl);

int cls_wifi_fw_trace_save_filters(struct cls_wifi_fw_trace *trace);
int cls_wifi_fw_trace_restore_filters(struct cls_wifi_fw_trace *trace);
#endif /* _CLS_WIFI_FW_TRACE_H_ */
