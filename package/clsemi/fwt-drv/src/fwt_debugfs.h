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

#ifndef _FWT_DEBUGFS_H_
#define _FWT_DEBUGFS_H_

typedef enum fwt_debugfs_cmd {
	FWT_DEBUGFS_CMD_CLEAR = 0,
	FWT_DEBUGFS_CMD_ON,
	FWT_DEBUGFS_CMD_OFF,
	FWT_DEBUGFS_CMD_PRINT,
	FWT_DEBUGFS_CMD_ADD_STATIC_MC,
	FWT_DEBUGFS_CMD_DEL_STATIC_MC,
	FWT_DEBUGFS_CMD_GET_MC_LIST,
	FWT_DEBUGFS_CMD_ADD,
	FWT_DEBUGFS_CMD_DELETE,
	FWT_DEBUGFS_CMD_AUTO,
	FWT_DEBUGFS_CMD_MANUAL,
	FWT_DEBUGFS_CMD_4ADDR,
	FWT_DEBUGFS_CMD_DEBUG,
	FWT_DEBUGFS_CMD_HELP,
	FWT_DEBUGFS_CMD_AGEING,
	FWT_DEBUGFS_CMD_QOS,
	FWT_DEBUGFS_MAX_CMD,
} fwt_debugfs_cmd;

#endif /* _FWT_BUGFS_H_ */
