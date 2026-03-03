/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

/*
 * pwm driver for pwm_drv.h
 */

#ifndef __PWM_DRV_H_
#define __PWM_DRV_H_
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/kernel.h>

/*
 *  0% duty cycle – Continuous Low and no high
 *  TimerNLoadCount = Do not care
 *  TimerNLoadCount2 = 0
 *  100% duty cycle – No low period and continuous high
 *  TimerNLoadCount = 0
 *  TimerNLoadCount2 = Do not care
 *  Other duty cycle – When 0% and 100% duty cycle mode is enabled (with timer PWM mode and the
 *  user-defined count mode is enabled), the definition of the toggle high and low period changes as
 *  follows for a duty cycle other than 0% or 100%:
 *  Width of timer_N_toggle HIGH period = TimerNLoadCount2 * timer_N_clk clock period
 *  Width of timer_N_toggle LOW period = TimerNLoadCount * timer_N_clk clock period
 */

/*
 * Duty Cycle (%) TimerNLoadCount TimerNLoadCount2
 * 0                    X                  0
 * 1                    99                 1
 * 2                    98                 2
 * 3                    97                 3
 * 4                    96                 4
 * .... .... ...
 */

#define COUNTER_REG_BASE    0x0

/*
 * TimerNControlReg [3] (PWM enable bit)
 * TimerNControlReg [1] (Timer mode bit)
 * TimerNControlReg [4] (0% and 100% duty cycle mode bit)
 */

#define TIMER_N_CONTROL_REG         0x8
#define TIMER_ENABLE_BIT            0
#define TIMER_MODE_BIT              1
#define TIMER_INTERRUPT_MASK_BIT    2
#define TIMER_PWM_BIT               3

#define COUNTER_2_REG_BASE  0xb0

struct dubhe_pwm_instance {
	struct mutex pwm_lock;
	struct device *dev;
	void __iomem *reg_base;
	struct pwm_chip chip;
	struct clk *pwm_clk;
	uint32_t dev_channel_num;
};

struct dubhe_pwm_channel {
	uint32_t period_ns;
	uint32_t duty_ns;
	uint32_t tin_ns;
};

#endif
