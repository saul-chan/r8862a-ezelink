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

#ifndef _CLS_WIFI_HEARTBEAT_H_
#define _CLS_WIFI_HEARTBEAT_H_
#include "cls_wifi_platform.h"

#define HEARTBEAT_TIMER_INTERVAL		1
#define HEARTBEAT_TIMER_INTERVAL_JIFFIES	(HEARTBEAT_TIMER_INTERVAL * HZ)
#define CLS_WIFI2FW_ACK			0x4841434B
#define CLS_FW2WIFI_ACK			0x48454154
#define CLS_WIFI2FW_RDY			0x48524459
#define CLS_FW2WIFI_RDY			0x46524459
#define LAST_HEARTBEAT_BIT			0
#define CLS_WIFI2FW_URDY		0x55524400
#define CLS_WIFI_UART_0			"90008000"
#define CLS_WIFI_UART_1			"90009000"
#define CLS_WIFI_UART_2			"9000a000"

extern int cls_wifi_heartbeat_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
extern int cls_wifi_heartbeat_deinit(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
extern int cls_wifi_update_uart_info(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
extern u32 cls_wifi_get_heartbeat_uevent(struct cls_wifi_hw *cls_wifi_hw);
extern u32 ipc_host_heart_ack(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value);
extern u32 ipc_host_heart_rdy(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value);
extern u32 ipc_host_uart_info(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value);
#endif /* _CLS_WIFI_HEARTBEAT_H_ */
