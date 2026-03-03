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

#ifndef PVT_TSENS_H_
#define PVT_TSENS_H_
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/irqreturn.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/thermal/pvt.h>

#define T0_SENSOR_BASE_ADDR     0x90500000

#define PVTS_HW_SW_LOCK_MASK    0x3

#define TS_COMMON_OFFSET        0x80
#define MACRO_OFFSET            0x40
#define TS_MACRO_OFFSET         (TS_COMMON_OFFSET + MACRO_OFFSET)

#define COMMON_CLK_SYNTH        0x00
#define COMMON_SDIF_DISABLE     0x04
#define COMMON_SDIF_STATUS      0x08
#define COMMON_SDIF             0x0C
#define COMMON_SDIF_HALT        0x10
#define COMMON_SMPL_CTRL        0x20
#define COMMON_SMPL_CNT         0x28

#define TS_SDIF_LOCK_MASK       0x02
#define TS_SDIF_BUSY_MASK	0x01
#define TS_ADC_BITMASK		0xFFF

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

#define IRQ_GLB_EN_REG    0x40
#define TS_IRQ_ENABLE_BIT 1
#define IRQ_TS_MASK_REG   0x54
#define IRQ_TS_STATUS_REG 0x64

#define IRQ_STATUS_ALARMA 0x08	// For Reset Handler
#define IRQ_STATUS_ALARMB 0x10	// For Recovery Handler

#define PVT_CMD_MAX_LEN         128
#define PVT_WRITE_CMD  ('w')
#define PVT_READ_CMD   ('r')
#define MAP(v)  ((v) == 'c' ? BIT(0) : ((v) == 'r' ? BIT(1):((v) == 'w' ? BIT(2):0)))

#define temperature_code_value_conversion(x) ((68310 * 4096 + x * 4094) / 220500)

#define voltage_code_value_conversion(x) ((x * 16384 * 5 + 16387 * 1200) / (1200 * 6))


extern unsigned int debug_pr_info;
extern struct cls_tsens_ops tsens_ops;
extern struct cls_tsens_dev *tsens_dev;
extern void tsens_pop_jobs(unsigned long data);
extern irqreturn_t tsens_irq_handler(int irq, void *dev);
extern void tsens_interrupt_init(struct cls_tsens_dev *tsens_dev);
extern void tsens_parameter_update(struct cls_tsens_dev *tsens_dev);

struct cls_tsens_priv {
	spinlock_t hw_lock;
	struct tasklet_struct pop_jobs;
	struct cls_tsens_dev *tsens_dev;
};

struct cls_tsens_ops {
	int (*init)(struct cls_tsens_dev *tsens_dev);
	int (*deinit)(struct cls_tsens_dev *tsens_dev);
	int (*start)(struct cls_tsens_dev *tsens_dev);
	int (*stop)(struct cls_tsens_dev *tsens_dev);
	int (*read_process)(struct cls_tsens_dev *tsens_dev);
	int (*read_vol)(struct cls_tsens_dev *tsens_dev);
	int (*read_temp)(struct cls_tsens_dev *tsens_dev);
};

#endif

