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

#include <linux/list.h>

#include "cls_wifi_cmds.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_strs.h"
#define CREATE_TRACE_POINTS
#include "cls_wifi_events.h"

/**
 *
 */
static void cmd_mgr_crashdump(const struct cls_wifi_cmd *cmd)
{
	pr_err("tkn[%d]  flags:%04x  result:%3d  cmd:%4d-%-24s - reqcfm(%4d-%-s)\n",
		   cmd->tkn, cmd->flags, cmd->result, cmd->id, CLS_WIFI_ID2STR(cmd->id),
		   cmd->reqid, cmd->reqid != (lmac_msg_id_t)-1 ? CLS_WIFI_ID2STR(cmd->reqid) : "none");
}

/**
 *
 */
static void cmd_complete(struct cls_wifi_cmd_mgr *cmd_mgr, struct cls_wifi_cmd *cmd)
{
	lockdep_assert_held(&cmd_mgr->lock);

	list_del(&cmd->list);
	cmd_mgr->queue_sz--;

	cmd->flags |= CLS_WIFI_CMD_FLAG_DONE;
	if (cmd->flags & CLS_WIFI_CMD_FLAG_NONBLOCK) {
		kfree(cmd);
	} else {
		if (CLS_WIFI_CMD_WAIT_COMPLETE(cmd->flags)) {
			cmd->result = 0;
			complete(&cmd->complete);
		}
	}
}

/**
 *
 */
static int cmd_mgr_queue(struct cls_wifi_cmd_mgr *cmd_mgr, struct cls_wifi_cmd *cmd)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(cmd_mgr, struct cls_wifi_hw, cmd_mgr);
	bool defer_push = false;
	int cmd_flags;

	trace_msg_send(cmd->id);

	spin_lock_bh(&cmd_mgr->lock);

	if (!list_empty(&cmd_mgr->cmds)) {
		struct cls_wifi_cmd *last;

		if (cmd_mgr->queue_sz == cmd_mgr->max_queue_sz) {
			cmd->result = -ENOMEM;
			spin_unlock_bh(&cmd_mgr->lock);
			return -ENOMEM;
		}
		last = list_entry(cmd_mgr->cmds.prev, struct cls_wifi_cmd, list);
		if (last->flags & (CLS_WIFI_CMD_FLAG_WAIT_ACK | CLS_WIFI_CMD_FLAG_WAIT_PUSH)) {
			cmd->flags |= CLS_WIFI_CMD_FLAG_WAIT_PUSH;
			defer_push = true;
		}
	}

	cmd->flags |= CLS_WIFI_CMD_FLAG_WAIT_ACK;
	if (cmd->flags & CLS_WIFI_CMD_FLAG_REQ_CFM)
		cmd->flags |= CLS_WIFI_CMD_FLAG_WAIT_CFM;

	cmd->tkn	= cmd_mgr->next_tkn++;
	cmd->result	= -EINTR;

	if (!(cmd->flags & CLS_WIFI_CMD_FLAG_NONBLOCK))
		init_completion(&cmd->complete);

	list_add_tail(&cmd->list, &cmd_mgr->cmds);
	cmd_mgr->queue_sz++;
	cmd_flags = cmd->flags;
	spin_unlock_bh(&cmd_mgr->lock);

	if (!defer_push) {
		cls_wifi_ipc_msg_push(cls_wifi_hw, cmd, CLS_WIFI_CMD_A2EMSG_LEN(cmd->a2e_msg));
		kfree(cmd->a2e_msg);
	}

	if (!(cmd_flags & CLS_WIFI_CMD_FLAG_NONBLOCK)) {
		unsigned long tout = msecs_to_jiffies(CLS_WIFI_80211_CMD_TIMEOUT_MS * cmd_mgr->queue_sz);
		if (!wait_for_completion_killable_timeout(&cmd->complete, tout)) {
			spin_lock_bh(&cmd_mgr->lock);
			cmd_mgr->state = CLS_WIFI_CMD_MGR_STATE_CRASHED;
			if (!(cmd->flags & CLS_WIFI_CMD_FLAG_DONE)) {
				cmd->result = -ETIMEDOUT;
				cmd_complete(cmd_mgr, cmd);
			}
			spin_unlock_bh(&cmd_mgr->lock);
		}
	} else {
		cmd->result = 0;
	}

	return 0;
}

/**
 *
 */
static int cmd_mgr_llind(struct cls_wifi_cmd_mgr *cmd_mgr, struct cls_wifi_cmd *cmd)
{
	struct cls_wifi_cmd *cur, *acked = NULL, *next = NULL;

	spin_lock(&cmd_mgr->lock);
	list_for_each_entry(cur, &cmd_mgr->cmds, list) {
		if (!acked) {
			if (cur->tkn == cmd->tkn) {
				if (WARN_ON_ONCE(cur != cmd))
					cmd_mgr_crashdump(cmd);
				acked = cur;
				continue;
			}
		}
		if (cur->flags & CLS_WIFI_CMD_FLAG_WAIT_PUSH) {
			next = cur;
			break;
		}
	}
	if (!acked) {
		pr_err("Error: acked cmd not found\n");
	} else {
		cmd->flags &= ~CLS_WIFI_CMD_FLAG_WAIT_ACK;
		if (CLS_WIFI_CMD_WAIT_COMPLETE(cmd->flags))
			cmd_complete(cmd_mgr, cmd);
	}
	if (next) {
		struct cls_wifi_hw *cls_wifi_hw = container_of(cmd_mgr, struct cls_wifi_hw, cmd_mgr);

		next->flags &= ~CLS_WIFI_CMD_FLAG_WAIT_PUSH;
		cls_wifi_ipc_msg_push(cls_wifi_hw, next, CLS_WIFI_CMD_A2EMSG_LEN(next->a2e_msg));
		kfree(next->a2e_msg);
	}
	spin_unlock(&cmd_mgr->lock);

	return 0;
}



static int cmd_mgr_run_callback(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_cmd *cmd,
								struct cls_wifi_cmd_e2amsg *msg, msg_cb_fct cb)
{
	int res;

	if (!cb)
		return 0;

	spin_lock(&cls_wifi_hw->cb_lock);
	res = cb(cls_wifi_hw, cmd, msg);
	spin_unlock(&cls_wifi_hw->cb_lock);

	return res;
}

/**
 *

 */
static int cmd_mgr_msgind(struct cls_wifi_cmd_mgr *cmd_mgr, struct cls_wifi_cmd_e2amsg *msg,
						  msg_cb_fct cb)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(cmd_mgr, struct cls_wifi_hw, cmd_mgr);
	struct cls_wifi_cmd *cmd;
	bool found = false;

	trace_msg_recv(msg->id);

	spin_lock(&cmd_mgr->lock);
	list_for_each_entry(cmd, &cmd_mgr->cmds, list) {
		if (cmd->reqid == msg->id &&
			(cmd->flags & CLS_WIFI_CMD_FLAG_WAIT_CFM)) {

			if (!cmd_mgr_run_callback(cls_wifi_hw, cmd, msg, cb)) {
				found = true;
				cmd->flags &= ~CLS_WIFI_CMD_FLAG_WAIT_CFM;

				if (WARN((msg->param_len > CLS_WIFI_CMD_E2AMSG_LEN_MAX),
						 "Unexpect E2A msg len %d > %d\n", msg->param_len,
						 CLS_WIFI_CMD_E2AMSG_LEN_MAX)) {
					msg->param_len = CLS_WIFI_CMD_E2AMSG_LEN_MAX;
				}

				if (cmd->e2a_msg && msg->param_len)
					memcpy(cmd->e2a_msg, &msg->param, msg->param_len);

				if (CLS_WIFI_CMD_WAIT_COMPLETE(cmd->flags))
					cmd_complete(cmd_mgr, cmd);

				break;
			}
		}
	}
	spin_unlock(&cmd_mgr->lock);

	if (!found)
		cmd_mgr_run_callback(cls_wifi_hw, NULL, msg, cb);

	return 0;
}

/**
 *
 */
static void cmd_mgr_print(struct cls_wifi_cmd_mgr *cmd_mgr)
{
	struct cls_wifi_cmd *cur;

	spin_lock_bh(&cmd_mgr->lock);
	CLS_WIFI_DBG("q_sz/max: %2d / %2d - next tkn: %d\n",
			 cmd_mgr->queue_sz, cmd_mgr->max_queue_sz,
			 cmd_mgr->next_tkn);
	list_for_each_entry(cur, &cmd_mgr->cmds, list) {
		cmd_mgr_crashdump(cur);
	}
	spin_unlock_bh(&cmd_mgr->lock);
}

/**
 *
 */
static void cmd_mgr_drain(struct cls_wifi_cmd_mgr *cmd_mgr)
{
	struct cls_wifi_cmd *cur, *nxt;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	spin_lock_bh(&cmd_mgr->lock);
	list_for_each_entry_safe(cur, nxt, &cmd_mgr->cmds, list) {
		list_del(&cur->list);
		cmd_mgr->queue_sz--;
		if (!(cur->flags & CLS_WIFI_CMD_FLAG_NONBLOCK))
			complete(&cur->complete);
	}
	spin_unlock_bh(&cmd_mgr->lock);
}

/**
 *
 */
void cls_wifi_cmd_mgr_init(struct cls_wifi_cmd_mgr *cmd_mgr)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	INIT_LIST_HEAD(&cmd_mgr->cmds);
	spin_lock_init(&cmd_mgr->lock);
	cmd_mgr->max_queue_sz = CLS_WIFI_CMD_MAX_QUEUED;
	cmd_mgr->queue  = &cmd_mgr_queue;
	cmd_mgr->print  = &cmd_mgr_print;
	cmd_mgr->drain  = &cmd_mgr_drain;
	cmd_mgr->llind  = &cmd_mgr_llind;
	cmd_mgr->msgind = &cmd_mgr_msgind;
	cmd_mgr->crashdump = &cmd_mgr_crashdump;
}

/**
 *
 */
void cls_wifi_cmd_mgr_deinit(struct cls_wifi_cmd_mgr *cmd_mgr)
{
	cmd_mgr->print(cmd_mgr);
	cmd_mgr->drain(cmd_mgr);
	cmd_mgr->print(cmd_mgr);
	memset(cmd_mgr, 0, sizeof(*cmd_mgr));
}
