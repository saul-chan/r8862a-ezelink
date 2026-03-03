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
 * pwm driver for pwm_drv.c
 */

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/pwm.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include "pwm_drv.h"

uint32_t pwm_period;

static DEFINE_SPINLOCK(dubeh_pwm_lock);

static inline struct dubhe_pwm_instance *to_dubhe_pwmchip(struct pwm_chip *cp)
{
	return container_of(cp, struct dubhe_pwm_instance, chip);
}

static int drv_pwm_period_set(struct dubhe_pwm_instance *pwm_instance, uint32_t period)
{
	pwm_period = period;
	return 0;
}

static int drv_pwm_duty_set(struct dubhe_pwm_instance *pwm_instance, uint32_t duty)
{
	unsigned long flags;
	uint32_t new_duty = 0;
	uint32_t val = 0;
	uint32_t rate = clk_get_rate(pwm_instance->pwm_clk);

	spin_lock_irqsave(&dubeh_pwm_lock, flags);

	/* Disable the timer enable bit in the TimerNControlReg register */
	val &= ~(1 << TIMER_ENABLE_BIT);
	writel(val, pwm_instance->reg_base + TIMER_N_CONTROL_REG);

	if (duty == 0)
		writel(0, pwm_instance->reg_base + COUNTER_2_REG_BASE);
	else if (duty >= pwm_period)
		writel(0, pwm_instance->reg_base + COUNTER_REG_BASE);
	else {
		writel(DIV_ROUND_CLOSEST(pwm_period, NSEC_PER_SEC / rate) -
				DIV_ROUND_CLOSEST(duty, NSEC_PER_SEC / rate),
				pwm_instance->reg_base + COUNTER_REG_BASE);

		writel(DIV_ROUND_CLOSEST(duty, NSEC_PER_SEC / rate), pwm_instance->reg_base + COUNTER_2_REG_BASE);
	}
	spin_unlock_irqrestore(&dubeh_pwm_lock, flags);

	return 0;
}

static int drv_pwm_enable(struct pwm_chip *chip)
{
	uint32_t val;
	unsigned long flags;
	struct dubhe_pwm_instance *pwm_instance = to_dubhe_pwmchip(chip);

	spin_lock_irqsave(&dubeh_pwm_lock, flags);

	val = readl(pwm_instance->reg_base + TIMER_N_CONTROL_REG);
	val |= (1 << TIMER_ENABLE_BIT) | (1 << TIMER_MODE_BIT) | (1 << TIMER_PWM_BIT);

	writel(val, pwm_instance->reg_base + TIMER_N_CONTROL_REG);
	spin_unlock_irqrestore(&dubeh_pwm_lock, flags);

	return 0;
}

static int drv_pwm_disable(struct pwm_chip *chip)
{
	uint32_t val;
	unsigned long flags;

	struct dubhe_pwm_instance *pwm_instance = to_dubhe_pwmchip(chip);

	spin_lock_irqsave(&dubeh_pwm_lock, flags);

	val = readl(pwm_instance->reg_base + TIMER_N_CONTROL_REG);
	val &= ~(1 << TIMER_PWM_BIT);
	val &= ~(1 << TIMER_ENABLE_BIT);
	val &= ~(1 << TIMER_MODE_BIT);

	writel(val, pwm_instance->reg_base + TIMER_N_CONTROL_REG);
	spin_unlock_irqrestore(&dubeh_pwm_lock, flags);

	return 0;
}

/**
 * sysfs echo * > period. 500000:50%
 * sysfs echo * > duty_cycle. 1000000:1hz
 * sysfs echo * > enable
 */
static int dubhe_pwm_apply(struct pwm_chip *chip, struct pwm_device *pwm,
						const struct pwm_state *state)
{
	struct dubhe_pwm_instance *pwm_instance = to_dubhe_pwmchip(chip);

	drv_pwm_period_set(pwm_instance, state->period);

	drv_pwm_duty_set(pwm_instance, state->duty_cycle);

	if (state->enabled)
		drv_pwm_enable(&pwm_instance->chip);
	else
		drv_pwm_disable(&pwm_instance->chip);

	return 0;
}

/**
 * sysfs echo * > request
 */
static int dubhe_pwm_request(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct dubhe_pwm_instance *pwm_instance = to_dubhe_pwmchip(chip);
	struct device *dev = pwm_instance->dev;
	struct dubhe_pwm_channel *pwm_chan;

	pwm_chan = devm_kzalloc(dev, sizeof(struct dubhe_pwm_channel), GFP_KERNEL);
	if (!pwm_chan)
		return -ENOMEM;

	pwm_set_chip_data(pwm, pwm_chan);

	return 0;
}

static const struct pwm_ops dubhe_pwm_ops = {
	.apply      = dubhe_pwm_apply,
	.request    = dubhe_pwm_request,
	.owner = THIS_MODULE,
};

static int clourney_add_pwmchip(struct platform_device *pdev, struct dubhe_pwm_instance *pwm_instance)
{
	int ret = -1;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int pwm_id = of_alias_get_id(np, "pwm");

	if (pwm_id < 0) {
		/* assigned by the linux kernel */
		pwm_id = -1;
		dev_err(dev, "pwm instatnce get id error!\n");
	}

	pwm_instance->chip.dev = dev;
	pwm_instance->chip.ops = &dubhe_pwm_ops;
	pwm_instance->chip.base = pwm_id;
	pwm_instance->chip.npwm = pwm_instance->dev_channel_num; /* the channel numbers of a pwm controller */
	pwm_instance->chip.of_xlate = of_pwm_xlate_with_flags;
	/* Channel, frequency, duty cycle, usually used in some driver phander pwm */
	pwm_instance->chip.of_pwm_n_cells = 3;

	ret = pwmchip_add(&pwm_instance->chip);
	if (ret < 0) {
		dev_err(dev, "pwmchip_add error!\n");
		return ret;
	}

	writel(0xffffffff, pwm_instance->reg_base + COUNTER_REG_BASE);

	return 0;
}

static int clourney_get_pwm_resource(struct platform_device *pdev, struct dubhe_pwm_instance *pwm_instance)
{
	int ret = -1;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	pwm_instance->dev = dev;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "get pwm clock error\n");
		return ret;
	}

	pwm_instance->reg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pwm_instance->reg_base)) {
		dev_err(dev, "remap pwm reg_base error\n");
		return PTR_ERR(pwm_instance->reg_base);
	}

	pwm_instance->pwm_clk = devm_clk_get(dev, "pwm_clk");
	if (IS_ERR(pwm_instance->pwm_clk))
		dev_err(dev, "get pwm clock error\n");

	ret = of_property_read_u32(np, "pwm-chan-num", &pwm_instance->dev_channel_num);
	if (ret < 0)
		dev_err(dev, "No pwm-chan-num optinos configed in dts\n");

	return 0;
}

static int dubhe_pwm_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct device *dev = &pdev->dev;
	struct dubhe_pwm_instance *pwm_instance;

	pwm_instance = devm_kzalloc(dev, sizeof(struct dubhe_pwm_instance), GFP_KERNEL);
	if (IS_ERR(pwm_instance))
		return PTR_ERR(pwm_instance);

	ret = clourney_get_pwm_resource(pdev, pwm_instance);
	if (ret < 0)
		return ret;

	ret = clourney_add_pwmchip(pdev, pwm_instance);
	if (ret < 0)
		return ret;

	platform_set_drvdata(pdev, pwm_instance);


	return 0;
}

static int dubhe_pwm_remove(struct platform_device *pdev)
{
	struct dubhe_pwm_instance *pwm_instance = platform_get_drvdata(pdev);

	drv_pwm_disable(&pwm_instance->chip);

	pwmchip_remove(&pwm_instance->chip);

	return 0;
}

static const struct of_device_id of_dubhe_pwm_match[] = {
	{ .compatible = "clourney,dubhe-pwm", },
	{},
};

MODULE_DEVICE_TABLE(of, of_dubhe_pwm_match);

static struct platform_driver clourney_pwm_driver = {
	.probe		= dubhe_pwm_probe,
	.remove		= dubhe_pwm_remove,
	.driver		= {
		.name		= "clourney,dubhe-pwm",
		.of_match_table	= of_dubhe_pwm_match,
	},
};

module_platform_driver(clourney_pwm_driver);

MODULE_AUTHOR("clourney");
MODULE_DESCRIPTION("PWM driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm");
