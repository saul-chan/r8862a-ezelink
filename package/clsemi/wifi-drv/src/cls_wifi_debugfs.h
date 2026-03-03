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

#ifndef _CLS_WIFI_DEBUGFS_H_
#define _CLS_WIFI_DEBUGFS_H_

#include <linux/workqueue.h>
#include <linux/if_ether.h>
#include "cls_wifi_fw_trace.h"

struct cls_wifi_hw;
struct cls_wifi_sta;

// To ease debug let everyone access debug entries
// Mode for read only entries
#define DBGFS_RO (S_IRUGO)
// Mode for write entries
#define DBGFS_WR (S_IWUGO)
// Mode for read/write entries
#define DBGFS_RW (S_IWUGO | S_IRUGO)

#define DEBUGFS_ADD_FILE_WITH_OPS(ops, name, parent, mode) do {	\
		struct dentry *__tmp;									  \
		__tmp = debugfs_create_file(#name, mode, parent, cls_wifi_hw,  \
									&cls_wifi_dbgfs_##ops##_ops);	  \
		if (IS_ERR_OR_NULL(__tmp))								 \
			goto err;											  \
	} while (0)

#define DEBUGFS_ADD_FILE(name, parent, mode) DEBUGFS_ADD_FILE_WITH_OPS(name, name, parent, mode)

#define DEBUGFS_ADD_BOOL(name, parent, ptr, mode) do {			 \
		struct dentry *__tmp;									  \
		__tmp = debugfs_create_bool(#name, mode, parent, ptr);	 \
		if (IS_ERR_OR_NULL(__tmp))								 \
			goto err;											  \
	} while (0)

#define DEBUGFS_ADD_X64(name, parent, ptr, mode) do {			  \
		debugfs_create_x64(#name, mode, parent, ptr);			  \
	} while (0)

#define DEBUGFS_ADD_U64(name, parent, ptr, mode) do {			  \
		debugfs_create_u64(#name, mode, parent, ptr);			  \
	} while (0)

#define DEBUGFS_ADD_X32(name, parent, ptr) do {					\
		debugfs_create_x32(#name, mode, parent, ptr);			  \
	} while (0)

#define DEBUGFS_ADD_U32(name, parent, ptr, mode) do {			  \
		debugfs_create_u32(#name, mode, parent, ptr);			  \
	} while (0)


/* file operation */
#define DEBUGFS_READ_FUNC(name)										 \
	static ssize_t cls_wifi_dbgfs_##name##_read(struct file *file,		  \
											char __user *user_buf,	  \
											size_t count, loff_t *ppos);

#define DEBUGFS_WRITE_FUNC(name)										 \
	static ssize_t cls_wifi_dbgfs_##name##_write(struct file *file,		  \
											 const char __user *user_buf,\
											 size_t count, loff_t *ppos);

#define DEBUGFS_OPEN_FUNC(name)							  \
	static int cls_wifi_dbgfs_##name##_open(struct inode *inode, \
										struct file *file);

#define DEBUGFS_RELEASE_FUNC(name)							  \
	static int cls_wifi_dbgfs_##name##_release(struct inode *inode, \
										   struct file *file);

#define DEBUGFS_READ_FILE_OPS(name)							 \
	DEBUGFS_READ_FUNC(name);									\
static const struct file_operations cls_wifi_dbgfs_##name##_ops = { \
	.read   = cls_wifi_dbgfs_##name##_read,						 \
	.open   = simple_open,									  \
	.llseek = generic_file_llseek,							  \
};

#define DEBUGFS_WRITE_FILE_OPS(name)							\
	DEBUGFS_WRITE_FUNC(name);								   \
static const struct file_operations cls_wifi_dbgfs_##name##_ops = { \
	.write  = cls_wifi_dbgfs_##name##_write,						\
	.open   = simple_open,									  \
	.llseek = generic_file_llseek,							  \
};

#define DEBUGFS_READ_WRITE_FILE_OPS(name)					   \
	DEBUGFS_READ_FUNC(name);									\
	DEBUGFS_WRITE_FUNC(name);								   \
static const struct file_operations cls_wifi_dbgfs_##name##_ops = { \
	.write  = cls_wifi_dbgfs_##name##_write,						\
	.read   = cls_wifi_dbgfs_##name##_read,						 \
	.open   = simple_open,									  \
	.llseek = generic_file_llseek,							  \
};

#define DEBUGFS_READ_WRITE_OPEN_RELEASE_FILE_OPS(name)			  \
	DEBUGFS_READ_FUNC(name);										\
	DEBUGFS_WRITE_FUNC(name);									   \
	DEBUGFS_OPEN_FUNC(name);										\
	DEBUGFS_RELEASE_FUNC(name);									 \
static const struct file_operations cls_wifi_dbgfs_##name##_ops = {	 \
	.write   = cls_wifi_dbgfs_##name##_write,						   \
	.read	= cls_wifi_dbgfs_##name##_read,							\
	.open	= cls_wifi_dbgfs_##name##_open,							\
	.release = cls_wifi_dbgfs_##name##_release,						 \
	.llseek  = generic_file_llseek,								 \
};


#ifdef CONFIG_CLS_WIFI_DEBUGFS

struct cls_sta_snr_info {
	struct cls_sta_snr_info *next;
	u8 format;
	u8 mcs;
	u8 nss;
	u8 gi;
	u8 bw;
	u16 snr[16];
};

#define STA_DEBUGFS_MAX   64
struct cls_wifi_debugfs {
	struct dentry *dir;
	struct dentry *dir_link;
	struct dentry *dir_stas;
	bool trace_prst;
	bool la_phy;

	char helper_cmd[64];
	struct work_struct helper_work;
	bool helper_scheduled;
	spinlock_t umh_lock;
	bool unregistering;

	struct cls_wifi_fw_trace fw_trace;

	struct work_struct sta_work;

	struct dentry **dir_sta;
	u16 sta_idx[STA_DEBUGFS_MAX];
	u16 sta_idx_rd;
	u16 sta_idx_wr;
	struct dentry **dir_rc;
	struct dentry **dir_rc_sta;
	u32 *rc_config;
	struct list_head rc_config_save;
	struct dentry *dir_twt;
	struct dentry **dir_twt_sta;
};

// Max duration in msecs to save rate config for a sta after disconnection
#define RC_CONFIG_DUR 600000

struct cls_wifi_rc_config_save {
	struct list_head list;
	unsigned long timestamp;
	u32 rate;
	u8 mac_addr[ETH_ALEN];
};

#ifdef CONFIG_CLS_MSGQ_TEST
void cls_wifi_msgq_dbgfs_register(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_msgq_dbgfs_unregister(void);
#endif
int cls_wifi_dbgfs_register(struct cls_wifi_hw *cls_wifi_hw, const char *name);
void cls_wifi_dbgfs_unregister(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_um_helper(struct cls_wifi_debugfs *cls_wifi_debugfs, const char *cmd);
int cls_wifi_trigger_um_helper(struct cls_wifi_debugfs *cls_wifi_debugfs);
void cls_wifi_wait_um_helper(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_dbgfs_register_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta);
void cls_wifi_dbgfs_unregister_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta);

int cls_wifi_dbgfs_register_fw_dump(struct cls_wifi_hw *cls_wifi_hw,
								struct dentry *dir_drv,
								struct dentry *dir_diags);
void cls_wifi_dbgfs_trigger_fw_dump(struct cls_wifi_hw *cls_wifi_hw, char *reason);
void cls_wifi_dbgfs_config_fw_dump(struct cls_wifi_hw *cls_wifi_hw, bool la_phy);

void cls_wifi_fw_trace_dump(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_fw_trace_reset(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_dbgfs_rate_idx(int format, int bw_or_ru, int mcs, int gi, int nss, int preamble);
int cls_wifi_dbgdir_register(void);
void cls_wifi_dbgdir_unregister(void);
int cls_wifi_dbgfs_prepare(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_dbgfs_free(struct cls_wifi_hw *cls_wifi_hw);
#else

struct cls_wifi_debugfs {
};

static inline int cls_wifi_dbgfs_register(struct cls_wifi_hw *cls_wifi_hw, const char *name) { return 0; }
static inline void cls_wifi_dbgfs_unregister(struct cls_wifi_hw *cls_wifi_hw) {}
static inline int cls_wifi_um_helper(struct cls_wifi_debugfs *cls_wifi_debugfs, const char *cmd) { return 0; }
static inline int cls_wifi_trigger_um_helper(struct cls_wifi_debugfs *cls_wifi_debugfs) {return 0;}
static inline void cls_wifi_wait_um_helper(struct cls_wifi_hw *cls_wifi_hw) {}
static inline void cls_wifi_dbgfs_register_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)  {}
static inline void cls_wifi_dbgfs_unregister_sta(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta)  {}

void cls_wifi_fw_trace_dump(struct cls_wifi_hw *cls_wifi_hw) {}
void cls_wifi_fw_trace_reset(struct cls_wifi_hw *cls_wifi_hw) {}
void cls_wifi_dbgfs_config_fw_dump(struct cls_wifi_hw *cls_wifi_hw, bool la_phy) {}
static inline int cls_wifi_dbgdir_register(void) { return 0; }
static inline void cls_wifi_dbgdir_unregister(void) {}
int cls_wifi_dbgfs_prepare(struct cls_wifi_hw *cls_wifi_hw) { return 0;};
void cls_wifi_dbgfs_free(struct cls_wifi_hw *cls_wifi_hw) { return 0; };
#endif /* CONFIG_CLS_WIFI_DEBUGFS */


#endif /* _CLS_WIFI_DEBUGFS_H_ */
