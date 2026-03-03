/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 *  PVT T-SONSOR driver for AP SoC
 *
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

#define T_SENSOR_NUM            3
#define TSEN_ALARM_RISE		1
#define TSEN_ALARM_FALL		0

struct cls_tsens_dev {
	struct device *dev;
	struct mutex my_mutex;
	void __iomem *reg_base;
	uint32_t temp_rise_alarm0;
	int temp_rise_alarm0_L;
	int temp_rise_alarm0_H;
	uint32_t temp_rise_alarm1;
	int temp_rise_alarm1_L;
	int temp_rise_alarm1_H;
	uint32_t temp_rise_alarm2;
	int temp_rise_alarm2_L;
	int temp_rise_alarm2_H;
	uint32_t temp_fall_alarm0;
	int temp_fall_alarm0_L;
	int temp_fall_alarm0_H;
	uint32_t temp_fall_alarm1;
	int temp_fall_alarm1_L;
	int temp_fall_alarm1_H;
	uint32_t temp_fall_alarm2;
	int temp_fall_alarm2_L;
	int temp_fall_alarm2_H;
	int tsens_data[T_SENSOR_NUM];
	uint8_t alarma_flag;	// bit0:T0 bit1:T1 bit2:T2
	uint8_t alarmb_flag;	// bit0:T0 bit1:T1 bit2:T2
	spinlock_t alarm_lock;
	uint32_t alarm_work_enable;
	struct work_struct tsens_work;
	int (*handler_hooks)(struct cls_tsens_dev *tsens_dev);
};

