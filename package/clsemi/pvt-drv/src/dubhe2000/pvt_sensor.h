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
 *  PVT-SONSOR driver for PVT
 *
 */

#ifndef PVT_SENSOR_H_
#define PVT_SENSOR_H_
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/irqreturn.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#define T0_SENSOR_BASE_ADDR     0x90500000
#define T_SENSOR_NUM            3
#define P_SENSOR_NUM            3
#define V_SENSOR_NUM            1
#define V_SENSOR_POINT_NUM      7
#define P_SENSOR_POINT_NUM      6

#define PVTS_HW_SW_LOCK_MASK    0x3

#define TS_COMMON_OFFSET        0x80
#define PD_COMMON_OFFSET        0x180
#define VM_COMMON_OFFSET        0x400
#define MACRO_OFFSET            0x40
#define TS_MACRO_OFFSET         (TS_COMMON_OFFSET + MACRO_OFFSET)
#define PD_MACRO_OFFSET         (PD_COMMON_OFFSET + MACRO_OFFSET)
#define VM_MACRO_OFFSET         (VM_COMMON_OFFSET + 0x200)

#define COMMON_CLK_SYNTH        0x00
#define COMMON_SDIF_DISABLE     0x04
#define COMMON_SDIF_STATUS      0x08
#define COMMON_SDIF             0x0C
#define COMMON_SDIF_HALT        0x10
#define COMMON_SMPL_CTRL        0x20
#define COMMON_SMPL_CNT         0x28

#define PVT_SDIF_LOCK_MASK	0x02
#define PVT_SDIF_BUSY_MASK	0x01
#define TS_ADC_BITMASK		0xFFF
#define PD_ADC_BITMASK          0xFFFF
#define VM_ADC_BITMASK          0x3FFF

#define IRQ_ENABLE_REG          0x00
#define IRQ_STATUS_REG          0x04
#define IRQ_CLEAR_REG           0x08
#define IRQ_TEST_REG            0x0C
#define SDIF_RDATA_REG          0x10
#define SDIF_DONE_REG           0x14
#define SDIF_DATA_REG           0x18
#define ARLARMA_CFG_REG         0x20
#define ARLARMB_CFG_REG         0x24
#define SMPL_HILO_REG           0x28
#define HILO_RESET_REG          0x2C

#define VM_CHANNEL_OFFSET       0x10
#define VM_ARLARMA_ENABLE_REG   0x10
#define VM_ARLARMB_ENABLE_REG   0x20
#define VM_SDIF_RDATA_REG       0x30
#define VM_SDIF_DONE_REG        0x34
#define VM_SDIF_DATA_REG        0x40
#define VM_ARLARMA_OFFSET_REG   0x80
#define VM_ARLARMB_OFFSET_REG   0x84
#define VM_SMPL_HILO_REG        0x88
#define VM_HILO_RESET_REG       0x8C

#define IRQ_GLB_EN_REG    0x40
#define PD_IRQ_ENABLE_BIT 3
#define VM_IRQ_ENABLE_BIT 2
#define TS_IRQ_ENABLE_BIT 1

#define IRQ_TS_MASK_REG   0x54
#define IRQ_VM_MASK_REG   0x58
#define IRQ_PD_MASK_REG   0x5c

#define IRQ_TS_STATUS_REG 0x64
#define IRQ_VM_STATUS_REG 0x68
#define IRQ_PD_STATUS_REG 0x6c

#define VM_IRQ_STATUS 0x04
#define VM_IRQ_CLEAR  0x08
#define VM_IRQ_ALARMA_STATUS 0x14
#define VM_IRQ_ALARMB_STATUS 0x24
#define VM_IRQ_ALARMA_CLEAR  0x18
#define VM_IRQ_ALARMB_CLEAR  0x28

#define PVT_CMD_MAX_LEN         128
#define PVT_WRITE_CMD  ('w')
#define PVT_READ_CMD   ('r')
#define MAP(v)  ((v) == 'c' ? BIT(0) : ((v) == 'r' ? BIT(1) : ((v) == 'w' ? BIT(2): 0)))

#define temperature_code_value_conversion(x) \
	(68310 * 4096 + x * 4094 ) / 220500

#define voltage_code_value_conversion(x) \
	(x * 16384 * 5 + 16387 * 1200) / (1200 * 6)


extern unsigned int debug_pr_info;
extern struct clourney_pvt_ops pvt_ops;
extern struct clourney_pvt_dev *pvt_dev;
extern void pvt_pop_jobs (unsigned long data);
extern irqreturn_t pvt_irq_handler(int irq, void *dev);
extern void pvt_sensor_interrupt_init(struct clourney_pvt_dev *clourney_pvt);
extern void pvt_sensor_parameter_update(struct clourney_pvt_dev *pvt_dev);

struct clourney_pvt_dev {
	struct mutex my_mutex;
	void __iomem *reg_base;
	uint32_t temp_rise_alarm1;
	int temp_rise_alarm1_L;
	int temp_rise_alarm1_H;
	uint32_t temp_rise_alarm2;
	int temp_rise_alarm2_L;
	int temp_rise_alarm2_H;
	uint32_t temp_rise_alarm3;
	int temp_rise_alarm3_L;
	int temp_rise_alarm3_H;
	uint32_t temp_fall_alarm1;
	int temp_fall_alarm1_L;
	int temp_fall_alarm1_H;
	uint32_t temp_fall_alarm2;
	int temp_fall_alarm2_L;
	int temp_fall_alarm2_H;
	uint32_t temp_fall_alarm3;
	int temp_fall_alarm3_L;
	int temp_fall_alarm3_H;
	uint32_t volt_rise_alarm1;
	int volt_rise_alarm1_L;
	int volt_rise_alarm1_H;
	uint32_t volt_rise_alarm2;
	int volt_rise_alarm2_L;
	int volt_rise_alarm2_H;
	uint32_t volt_fall_alarm1;
	int volt_fall_alarm1_L;
	int volt_fall_alarm1_H;
	uint32_t volt_fall_alarm2;
	int volt_fall_alarm2_L;
	int volt_fall_alarm2_H ;
	uint32_t process_alarmA;
	uint32_t process_alarmB;
};

struct clourney_pvt_priv {
	spinlock_t hw_lock;
	struct tasklet_struct pop_jobs;
	struct clourney_pvt_dev *pvt_dev;
};

struct clourney_pvt_ops {
	int (*init)(struct clourney_pvt_dev *pvt_dev);
	int (*deinit)(struct clourney_pvt_dev *pvt_dev);
	int (*start)(struct clourney_pvt_dev *pvt_dev);
	int (*stop)(struct clourney_pvt_dev *pvt_dev);
	int (*read_process)(struct clourney_pvt_dev *pvt_dev);
	int (*read_vol)(struct clourney_pvt_dev *pvt_dev);
	int (*read_temp)(struct clourney_pvt_dev *pvt_dev);
};

#endif

