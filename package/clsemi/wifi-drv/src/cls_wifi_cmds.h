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

#ifndef _CLS_WIFI_CMDS_H_
#define _CLS_WIFI_CMDS_H_

#ifdef __KERNEL__
#include <linux/spinlock.h>
#include <linux/completion.h>
#include "lmac_msg.h"
#include "ipc_shared.h"

#if defined(CFG_MERAK3000)
#if defined(CFG_M3K_FPGA)
#define CLS_WIFI_80211_CMD_TIMEOUT_MS	(800 * 1000)
#else
#define CLS_WIFI_80211_CMD_TIMEOUT_MS	(140 * 1000)
#endif
#else
#define CLS_WIFI_80211_CMD_TIMEOUT_MS	30000
#endif

#define CLS_WIFI_CMD_FLAG_NONBLOCK	  BIT(0)
#define CLS_WIFI_CMD_FLAG_REQ_CFM	   BIT(1)
#define CLS_WIFI_CMD_FLAG_WAIT_PUSH	 BIT(2)
#define CLS_WIFI_CMD_FLAG_WAIT_ACK	  BIT(3)
#define CLS_WIFI_CMD_FLAG_WAIT_CFM	  BIT(4)
#define CLS_WIFI_CMD_FLAG_DONE		  BIT(5)

#define CLS_WIFI_CMD_WAIT_COMPLETE(flags) \
	(!(flags & (CLS_WIFI_CMD_FLAG_WAIT_ACK | CLS_WIFI_CMD_FLAG_WAIT_CFM)))

#define CLS_WIFI_CMD_MAX_QUEUED		 1024

#define cls_wifi_cmd_e2amsg ipc_e2a_msg
#define cls_wifi_cmd_a2emsg lmac_msg
#define CLS_WIFI_CMD_A2EMSG_LEN(m) (sizeof(struct lmac_msg) + m->param_len)
#define CLS_WIFI_CMD_E2AMSG_LEN_MAX (IPC_E2A_MSG_PARAM_SIZE * 4)

struct cls_wifi_hw;
struct cls_wifi_cmd;
#endif

typedef int (*msg_cb_fct)(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
						  struct cls_wifi_cmd_e2amsg *msg);

#ifdef __KERNEL__
enum cls_wifi_cmd_mgr_state {
	CLS_WIFI_CMD_MGR_STATE_DEINIT,
	CLS_WIFI_CMD_MGR_STATE_INITED,
	CLS_WIFI_CMD_MGR_STATE_CRASHED,
};

struct cls_wifi_cmd {
	struct list_head list;
	lmac_msg_id_t id;
	lmac_msg_id_t reqid;
	struct cls_wifi_cmd_a2emsg *a2e_msg;
	char *e2a_msg;
	u32 tkn;
	u16 flags;

	struct completion complete;
	u32 result;
};

struct cls_wifi_cmd_mgr {
	enum cls_wifi_cmd_mgr_state state;
	spinlock_t lock;
	u32 next_tkn;
	u32 queue_sz;
	u32 max_queue_sz;

	struct list_head cmds;

	int  (*queue)(struct cls_wifi_cmd_mgr *mgr, struct cls_wifi_cmd *cmd);
	int  (*llind)(struct cls_wifi_cmd_mgr *mgr, struct cls_wifi_cmd *cmd);
	int  (*msgind)(struct cls_wifi_cmd_mgr *mgr, struct cls_wifi_cmd_e2amsg *msg, msg_cb_fct cb);
	void (*print)(struct cls_wifi_cmd_mgr *mgr);
	void (*drain)(struct cls_wifi_cmd_mgr *mgr);
	void (*crashdump)(const struct cls_wifi_cmd *cmd);
};

void cls_wifi_cmd_mgr_init(struct cls_wifi_cmd_mgr *cmd_mgr);
void cls_wifi_cmd_mgr_deinit(struct cls_wifi_cmd_mgr *cmd_mgr);
#endif
#endif /* _CLS_WIFI_CMDS_H_ */
