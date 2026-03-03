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

#include <linux/bitfield.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/kmod.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include "cls_wifi_platform.h"
#include "cls_wifi_mod_params.h"
#include "cls_wifi_heartbeat.h"
#include "cls_wifi_defs.h"
#include "ipc_shared.h"
#include "cls_wifi_core.h"
#include "cls_wifi_dubhe2000.h"
#include "cls_wifi_msg_tx.h"

#define LOST_HEARTBEAT_MASK(x)			GENMASK(x,  0)

static int cls_wifi_init_fw_cnt[CLS_WIFI_MAX_RADIOS];
static u32 cls_wifi_heartbeat_record[CLS_WIFI_MAX_RADIOS];
static u32 cls_wifi_heartbeat_cmdcrash[CLS_WIFI_MAX_RADIOS];
static bool cls_wifi_heartbeat_wpu_reset[CLS_WIFI_MAX_RADIOS];
static u32 cls_wifi_report_uevent;

static void cls_wifi_heartbeat_reset_work(struct work_struct *ws)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(ws, struct cls_wifi_hw, heartbeat_reset_work);
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct dht_force_reset_hw_req_op reset_req;
	u8 radio_idx = cls_wifi_hw->radio_idx;
	char *envp[2] = {"EVENT=WPU_CRASH", NULL};

	if (!cls_wifi_mod_params.heartbeat_en)
		return;

	if (cls_wifi_hw->heartbeat_cmdcrash_recover) {
		cls_wifi_hw->cmd_mgr.state = CLS_WIFI_CMD_MGR_STATE_INITED;
		reset_req.reset_type = DHTF_RST_HW_RST_B;
		reset_req.reset_sub_type = DHTF_WMAC_STATE_KEEP_RESET;
		reset_req.op = DHTF_SEND_MSG_OP;
		// send dht soft reset hw request msg
		cls_wifi_send_force_soft_reset_hw_req(cls_wifi_hw, &reset_req);
		cls_wifi_hw->heartbeat_cmdcrash_recover = 0;
	}

	if ((cls_wifi_hw->heartbeat_uevent) && (!cls_wifi_heartbeat_wpu_reset[radio_idx])) {
		cls_wifi_heartbeat_wpu_reset[radio_idx] = true;
		// clean cmd msg queue
		cls_wifi_hw->cmd_mgr.drain(&cls_wifi_hw->cmd_mgr);
		// clear wpu ready status
		ipc_host_heart_rdy(cls_wifi_plat, radio_idx, CLS_WIFI2FW_RDY);
		ipc_host_heart_ack(cls_wifi_plat, radio_idx, CLS_WIFI2FW_ACK);

		spin_lock(&cls_wifi_hw->heartbeat_reset_lock);
		// wifi drv lost wpu heartbeat 30 times
		if (!cls_wifi_report_uevent)
			kobject_uevent_env(&cls_wifi_plat->dev->kobj, KOBJ_CHANGE, envp);
		cls_wifi_report_uevent |= cls_wifi_plat->hw_params.band_cap[radio_idx];
		spin_unlock(&cls_wifi_hw->heartbeat_reset_lock);
	}
}

static void cls_wifi_heartbeat_handler(struct timer_list *timer)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(timer, struct cls_wifi_hw, heartbeat_timer);
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct device *dev = cls_wifi_plat->dev;
	u8 radio_idx = cls_wifi_hw->radio_idx;
	u32 current_heartbeat_record = (cls_wifi_heartbeat_record[radio_idx] &
			LOST_HEARTBEAT_MASK(cls_wifi_mod_params.heartbeat_cnt)) << 1;

	if (!cls_wifi_mod_params.heartbeat_en)
		goto handler_exit;

	if (ipc_host_heart_rdy(cls_wifi_plat, radio_idx, 0) == CLS_FW2WIFI_RDY) {
		// firsty cls_wifi_mod_params.heartbeat_skip seconds no need monitor heartbeat message
		if (cls_wifi_init_fw_cnt[radio_idx] < cls_wifi_mod_params.heartbeat_skip)
			cls_wifi_init_fw_cnt[radio_idx]++;
		else {
			if (ipc_host_heart_ack(cls_wifi_plat, radio_idx, 0) == CLS_FW2WIFI_ACK) {
				cls_wifi_heartbeat_record[radio_idx] =
					(current_heartbeat_record | (1 << LAST_HEARTBEAT_BIT)) &
					LOST_HEARTBEAT_MASK(cls_wifi_mod_params.heartbeat_cnt);
				// wifi drv give wpu heartbeat ack
				ipc_host_heart_ack(cls_wifi_plat, radio_idx, CLS_WIFI2FW_ACK);
				// clear uevent
				cls_wifi_hw->heartbeat_uevent = 0;
				cls_wifi_report_uevent &= ~(cls_wifi_plat->hw_params.band_cap[radio_idx]);
				// cmd queue is crash, but heartbeat is still alive
				if (cls_wifi_hw->cmd_mgr.state == CLS_WIFI_CMD_MGR_STATE_CRASHED) {
					if (cls_wifi_heartbeat_cmdcrash[radio_idx] >=
							cls_wifi_mod_params.heartbeat_recover) {
						//Triggle wpu crash uevent
						cls_wifi_hw->heartbeat_uevent = true;
						cls_wifi_hw->heartbeat_cmdcrash_recover = 0;
						cls_wifi_hw->cmd_mgr.drain(&cls_wifi_hw->cmd_mgr);
						dev_err(dev,
							"wpu for radio %d occur 'cmd queue crash' recover exceed %d times\n",
							radio_idx, cls_wifi_mod_params.heartbeat_recover);
					} else	//Triggle cmd crash recover
						cls_wifi_hw->heartbeat_cmdcrash_recover = true;
					cls_wifi_heartbeat_cmdcrash[radio_idx]++;
				}
			} else
				cls_wifi_heartbeat_record[radio_idx] =
					(current_heartbeat_record & (~LAST_HEARTBEAT_BIT)) &
					LOST_HEARTBEAT_MASK(cls_wifi_mod_params.heartbeat_cnt);

			if ((cls_wifi_heartbeat_record[radio_idx] == 0) && (!cls_wifi_hw->heartbeat_uevent)) {
				cls_wifi_hw->heartbeat_uevent = true;
				// clean cmd msg queue
				cls_wifi_hw->cmd_mgr.drain(&cls_wifi_hw->cmd_mgr);
				dev_err(dev,
					" missing wpu for radio %d heartbeat message exceed %d times\n",
					radio_idx, cls_wifi_mod_params.heartbeat_cnt);
				dev_err(dev,
					" current heartbeat ipc share msg heart_rdy:%08x heart_ack:%08x\n",
					ipc_host_heart_rdy(cls_wifi_plat, radio_idx, 0),
					ipc_host_heart_ack(cls_wifi_plat, radio_idx, 0));
			}
		}
	}

handler_exit:
	schedule_work(&cls_wifi_hw->heartbeat_reset_work);

	mod_timer(&cls_wifi_hw->heartbeat_timer, jiffies + HEARTBEAT_TIMER_INTERVAL_JIFFIES);
}

int cls_wifi_update_uart_info(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	struct device *dev = cls_wifi_plat->dev;
	struct device_node *np;
	const char *serial_str, *uart_reg;
	int uart_port = 0;

	for_each_node_by_name(np, "aliases") {
		of_property_read_string(np, "serial0", &serial_str);
		uart_reg = (strchr(serial_str, '@') + 1);
		if (!strcmp(uart_reg, CLS_WIFI_UART_0))
			uart_port = 0;
		else  if (!strcmp(uart_reg, CLS_WIFI_UART_1))
			uart_port = 1;
		else  if (!strcmp(uart_reg, CLS_WIFI_UART_2))
			uart_port = 2;
		else
			uart_port = 0;
		dev_info(dev,
			" wpu radio %d uart using duart%d@%s\n",
			radio_idx, uart_port, uart_reg);
	}

	ipc_host_uart_info(cls_wifi_plat, radio_idx, (uart_port | CLS_WIFI2FW_URDY));
	return 0;
}
EXPORT_SYMBOL(cls_wifi_update_uart_info);

int cls_wifi_heartbeat_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[radio_idx];
	struct device *dev = cls_wifi_plat->dev;

	if (!cls_wifi_mod_params.heartbeat_en) {
		dev_info(dev, "For SRRC case, heartbeat disable!!!!!\n");
		return 0;
	}

	// clear uevent
	cls_wifi_hw->heartbeat_uevent = 0;
	cls_wifi_report_uevent &= ~(cls_wifi_plat->hw_params.band_cap[radio_idx]);
	// clear heartbeat record
	cls_wifi_heartbeat_record[radio_idx] = 0xffffffff;
	// clear heartbeat skip counter
	cls_wifi_init_fw_cnt[radio_idx] = 0x0;
	/* CLS WiFi Host send H-ACK Message to WPU FW, when heartbeat_timer is ready */
	ipc_host_heart_rdy(cls_wifi_plat, radio_idx, CLS_WIFI2FW_RDY);
	ipc_host_heart_ack(cls_wifi_plat, radio_idx, CLS_WIFI2FW_ACK);

	timer_setup(&cls_wifi_hw->heartbeat_timer, cls_wifi_heartbeat_handler, 0);
	mod_timer(&cls_wifi_hw->heartbeat_timer, jiffies + HEARTBEAT_TIMER_INTERVAL_JIFFIES);

	spin_lock_init(&cls_wifi_hw->heartbeat_reset_lock);
	INIT_WORK(&cls_wifi_hw->heartbeat_reset_work, cls_wifi_heartbeat_reset_work);

	return 0;
}
EXPORT_SYMBOL(cls_wifi_heartbeat_init);

int cls_wifi_heartbeat_deinit(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[radio_idx];

	if (!cls_wifi_mod_params.heartbeat_en)
		return 0;

	if (cls_wifi_hw->heartbeat_reset_work.func)
		cancel_work_sync(&cls_wifi_hw->heartbeat_reset_work);
	del_timer_sync(&cls_wifi_hw->heartbeat_timer);

	// clear heartbeat record
	cls_wifi_heartbeat_record[radio_idx] = 0xffffffff;
	// clear heartbeat skip counter
	cls_wifi_init_fw_cnt[radio_idx] = 0x0;

	ipc_host_heart_rdy(cls_wifi_plat, radio_idx, 0xffffffff);
	ipc_host_heart_ack(cls_wifi_plat, radio_idx, 0xffffffff);
	return 0;
}
EXPORT_SYMBOL(cls_wifi_heartbeat_deinit);

u32 cls_wifi_get_heartbeat_uevent(struct cls_wifi_hw *cls_wifi_hw)
{
	return cls_wifi_report_uevent;
}
EXPORT_SYMBOL(cls_wifi_get_heartbeat_uevent);
