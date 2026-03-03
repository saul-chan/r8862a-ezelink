// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * reset controller for Dubhe1000 Top CRG glb_sft_rst
 *
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 */

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/reset-controller.h>

#include <asm/system_misc.h>

#define TOP_GLOBAL_SWRST_EN          0xA5A5A5A5
#if defined(CONFIG_RESET_CLS_DUBHE1000)
#define TOP_GLOBAL_SWRST_START       0x5A5A7676
#elif defined(CONFIG_RESET_CLS_DUBHE2000)
#define TOP_GLOBAL_SWRST_START       0x765A765A
#endif

static void __iomem *dubhe_glrstc_base;

static void dubhe_global_restart(enum reboot_mode mode, const char *cmd)
{
	writel(TOP_GLOBAL_SWRST_EN, dubhe_glrstc_base);
	writel(TOP_GLOBAL_SWRST_START, dubhe_glrstc_base);
}

static int dubhe_glrstc_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	dubhe_glrstc_base = of_iomap(np, 0);
	if (!dubhe_glrstc_base) {
		dev_err(&pdev->dev, "unable to map global reset registers\n");
		return -ENOMEM;
	}

	arm_pm_restart = dubhe_global_restart;
	return 0;
}

static const struct of_device_id dubhe_glrstc_ids[]  = {
	{ .compatible = "cls,global-rstc" },
	{},
};

static struct platform_driver dubhe_glrstc_driver = {
	.probe		= dubhe_glrstc_probe,
	.driver		= {
		.name	= "dubhe_global_rstc",
		.of_match_table = dubhe_glrstc_ids,
	},
};

static int __init dubhe_glrstc_init(void)
{
	return platform_driver_register(&dubhe_glrstc_driver);
}
subsys_initcall(dubhe_glrstc_init);
