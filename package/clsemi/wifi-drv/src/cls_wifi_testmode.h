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

#ifndef _CLS_WIFI_TESTMODE_H_
#define _CLS_WIFI_TESTMODE_H_

#include <net/mac80211.h>
#include <net/netlink.h>

/* Commands from user space to kernel space(CLS_WIFI_TM_CMD_APP2DEV_XX) and
 * from and kernel space to user space(CLS_WIFI_TM_CMD_DEV2APP_XX).
 * The command ID is carried with CLS_WIFI_TM_ATTR_COMMAND.
 */
enum cls_wifi_tm_cmd_t {
	/* commands from user application to access register */
	CLS_WIFI_TM_CMD_APP2DEV_REG_READ = 1,
	CLS_WIFI_TM_CMD_APP2DEV_REG_WRITE,

	/* commands from user application to select the Debug levels */
	CLS_WIFI_TM_CMD_APP2DEV_SET_DBGMODFILTER,
	CLS_WIFI_TM_CMD_APP2DEV_SET_DBGSEVFILTER,

	/* commands to access registers without sending messages to LMAC layer,
	 * this must be used when LMAC FW is stuck. */
	CLS_WIFI_TM_CMD_APP2DEV_REG_READ_DBG,
	CLS_WIFI_TM_CMD_APP2DEV_REG_WRITE_DBG,

	CLS_WIFI_TM_CMD_MAX,
};

enum cls_wifi_tm_attr_t {
	CLS_WIFI_TM_ATTR_NOT_APPLICABLE = 0,

	CLS_WIFI_TM_ATTR_COMMAND,

	/* When CLS_WIFI_TM_ATTR_COMMAND is CLS_WIFI_TM_CMD_APP2DEV_REG_XXX,
	 * The mandatory fields are:
	 * CLS_WIFI_TM_ATTR_REG_OFFSET for the offset of the target register;
	 * CLS_WIFI_TM_ATTR_REG_VALUE32 for value */
	CLS_WIFI_TM_ATTR_REG_OFFSET,
	CLS_WIFI_TM_ATTR_REG_VALUE32,

	/* When CLS_WIFI_TM_ATTR_COMMAND is CLS_WIFI_TM_CMD_APP2DEV_SET_DBGXXXFILTER,
	 * The mandatory field is CLS_WIFI_TM_ATTR_REG_FILTER. */
	CLS_WIFI_TM_ATTR_REG_FILTER,

	CLS_WIFI_TM_ATTR_MAX,
};

/***********************************************************************/
int cls_wifi_testmode_reg(struct ieee80211_hw *hw, struct nlattr **tb);
int cls_wifi_testmode_dbg_filter(struct ieee80211_hw *hw, struct nlattr **tb);
int cls_wifi_testmode_reg_dbg(struct ieee80211_hw *hw, struct nlattr **tb);

#endif /* _CLS_WIFI_TESTMODE_H_ */
