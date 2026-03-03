// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023 Clourney Semiconductor - All Rights Reserved.
 * Clourney Semiconductor TOP CRG Watchdog timer driver.
 */

#include <linux/bitops.h>
#include <linux/limits.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/watchdog.h>
#include <linux/debugfs.h>


#define CRG_WDOG_CFG0_REG	0x00
#define CRG_WDOG_MODE_MASK	GENMASK(1,  0)
#define CRG_WDOG_INT_WIDTH	GENMASK(15, 8)
#define CRG_WDOG_INT_THRESHOLD	GENMASK(23, 16)
#define CRG_WDOG_TIMEOUT_REG	0x04
#define CRG_WDOG_START_REG	0x08
#define CRG_WDOG_CLR_REG	0x0C
#define CRG_WDOG_FEED_DOG	0xA5A55A5A
#define CRG_WDOG_RPT		0x0100
#define CRG_WDOG_RPT_CLR	0x0104

#define CLS_CRG_WDT_CNT_CYCLE    (2)
#define WDOG_INT_MAX_THRESHOLD   (256)
#define WDOG_PRETIME_THRESHOLD   (2)
#define WDOG_DEFAULT_SECONDS     (120)
#define WDOG_RST_MAX_SECONDS     (150)
#define WDOG_IRQ_MIN_SECONDS     (WDOG_PRETIME_THRESHOLD * WDOG_RST_MAX_SECONDS)
#define WDOG_IRQ_MAX_SECONDS     (WDOG_INT_MAX_THRESHOLD * WDOG_RST_MAX_SECONDS)

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started "
		"(default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

enum cls_crg_wdt_rmod {
	CLS_CRG_WDT_RMOD_RESET = 0,
	CLS_CRG_WDT_RMOD_IRQ = 1,
	CLS_CRG_WDT_RMOD_BOTH = 2
};

struct cls_crg_wdt_timeout {
	u32 top_val;
	unsigned int sec;
	unsigned int msec;
};

struct cls_crg_wdt {
	void __iomem		*regs;
	struct clk              *clk;
	enum cls_crg_wdt_rmod	rmod;
	struct watchdog_device	wdd;
	u32			pretimeout;
	u32			timeout;
	unsigned long           rate;
#ifdef CONFIG_DEBUG_FS
	struct dentry		*dbgfs_dir;
#endif
};

#define to_cls_crg_wdt(wdd)	container_of(wdd, struct cls_crg_wdt, wdd)

static void cls_crg_wdt_update_mode(struct cls_crg_wdt *cls_crg_wdt, enum cls_crg_wdt_rmod rmod)
{
	u32 val,wdt_int_threshold;

	val = readl(cls_crg_wdt->regs + CRG_WDOG_CFG0_REG);
	val &=  ~(CRG_WDOG_MODE_MASK|CRG_WDOG_INT_THRESHOLD);
	val |=  FIELD_PREP(CRG_WDOG_MODE_MASK, rmod);

	if (rmod == CLS_CRG_WDT_RMOD_IRQ) {
		if (cls_crg_wdt->timeout <= WDOG_IRQ_MIN_SECONDS) {
			val |=  FIELD_PREP(CRG_WDOG_INT_THRESHOLD, (WDOG_PRETIME_THRESHOLD - 1));
		}
		else if ((cls_crg_wdt->timeout > WDOG_IRQ_MIN_SECONDS) && (cls_crg_wdt->timeout <= WDOG_IRQ_MAX_SECONDS)) {

			if ( cls_crg_wdt->timeout%WDOG_RST_MAX_SECONDS == 0 )
				wdt_int_threshold = (cls_crg_wdt->timeout/WDOG_RST_MAX_SECONDS);
			else
				wdt_int_threshold = (cls_crg_wdt->timeout/WDOG_RST_MAX_SECONDS) + 1;

			val |=  FIELD_PREP(CRG_WDOG_INT_THRESHOLD, (wdt_int_threshold - 1));
		}
	}
	else
		val |=  FIELD_PREP(CRG_WDOG_INT_THRESHOLD, 0);

	writel(val, cls_crg_wdt->regs + CRG_WDOG_CFG0_REG);
	cls_crg_wdt->rmod = rmod;
}

static int cls_crg_wdt_ping(struct watchdog_device *wdd)
{
	struct cls_crg_wdt *cls_crg_wdt = to_cls_crg_wdt(wdd);

	writel(CRG_WDOG_FEED_DOG, cls_crg_wdt->regs +
	       CRG_WDOG_CLR_REG);

	return 0;
}

static int cls_crg_wdt_set_timeout(struct watchdog_device *wdd, unsigned int top_s)
{
	struct cls_crg_wdt *cls_crg_wdt = to_cls_crg_wdt(wdd);
	struct device *dev = cls_crg_wdt->wdd.parent;
	u32 val,wdt_int_threshold;

	cls_crg_wdt->timeout = top_s;

        if ( cls_crg_wdt->rmod == CLS_CRG_WDT_RMOD_IRQ ) {
                if ( cls_crg_wdt->timeout > WDOG_IRQ_MAX_SECONDS ) {
                        dev_err(dev, "timeout-sec: exceed the max setting %d seconds\n", WDOG_IRQ_MAX_SECONDS);
                        cls_crg_wdt->timeout = WDOG_IRQ_MAX_SECONDS;
                }
        }
        else if ( cls_crg_wdt->rmod == CLS_CRG_WDT_RMOD_RESET ) {
                if ( cls_crg_wdt->timeout > WDOG_RST_MAX_SECONDS ) {
                        dev_err(dev, "timeout-sec: exceed the max setting %d seconds\n", WDOG_RST_MAX_SECONDS);
                        cls_crg_wdt->timeout = WDOG_RST_MAX_SECONDS;
                }
        }

	if (cls_crg_wdt->rmod == CLS_CRG_WDT_RMOD_IRQ) {

		if (cls_crg_wdt->timeout <= WDOG_IRQ_MIN_SECONDS) {
			wdt_int_threshold = WDOG_PRETIME_THRESHOLD;
		}
		else if ((cls_crg_wdt->timeout > WDOG_IRQ_MIN_SECONDS) && (cls_crg_wdt->timeout <= WDOG_IRQ_MAX_SECONDS)) {

			if ( cls_crg_wdt->timeout%WDOG_RST_MAX_SECONDS == 0 )
				wdt_int_threshold = (cls_crg_wdt->timeout/WDOG_RST_MAX_SECONDS);
			else
				wdt_int_threshold = (cls_crg_wdt->timeout/WDOG_RST_MAX_SECONDS) + 1;
		}

		wdd->pretimeout = cls_crg_wdt->timeout;
		wdd->timeout = cls_crg_wdt->timeout * wdt_int_threshold;
	}
	else {
		wdd->pretimeout = 0;
		wdd->timeout = cls_crg_wdt->timeout;
	}
	val = (wdd->timeout * (cls_crg_wdt->rate / CLS_CRG_WDT_CNT_CYCLE));

	/* Kick new TOP value into the watchdog counter if activated. */
	if (watchdog_active(wdd))
		cls_crg_wdt_ping(wdd); 

	writel(val, cls_crg_wdt->regs + CRG_WDOG_TIMEOUT_REG);

	return 0;
}

static int cls_crg_wdt_set_pretimeout(struct watchdog_device *wdd, unsigned int req)
{
	struct cls_crg_wdt *cls_crg_wdt = to_cls_crg_wdt(wdd);

	/*
	 * We ignore actual value of the timeout passed from user-space
	 * using it as a flag whether the pretimeout functionality is intended
	 * to be activated.
	 */
	cls_crg_wdt_update_mode(cls_crg_wdt, req ? CLS_CRG_WDT_RMOD_IRQ : CLS_CRG_WDT_RMOD_RESET);
	cls_crg_wdt_set_timeout(wdd, wdd->timeout);

	return 0;
}

static int cls_crg_wdt_start(struct watchdog_device *wdd)
{
	struct cls_crg_wdt *cls_crg_wdt = to_cls_crg_wdt(wdd);

	cls_crg_wdt_set_timeout(wdd, wdd->timeout);
	cls_crg_wdt_ping(&cls_crg_wdt->wdd);
	writel(1, cls_crg_wdt->regs + CRG_WDOG_START_REG);
	return 0;
}

static int cls_crg_wdt_stop(struct watchdog_device *wdd)
{
	struct cls_crg_wdt *cls_crg_wdt = to_cls_crg_wdt(wdd);
	writel(0, cls_crg_wdt->regs + CRG_WDOG_START_REG);
	return 0;
}

static int cls_crg_wdt_restart(struct watchdog_device *wdd,
			  unsigned long action, void *data)
{
	struct cls_crg_wdt *cls_crg_wdt = to_cls_crg_wdt(wdd);

	/* kick watchdog, watchdog counter start from zero */
	writel(CRG_WDOG_FEED_DOG, cls_crg_wdt->regs + CRG_WDOG_CLR_REG);
	/* Set 500ms Reset Later */
	writel((cls_crg_wdt->rate / (CLS_CRG_WDT_CNT_CYCLE * 2)), cls_crg_wdt->regs + CRG_WDOG_TIMEOUT_REG);
	cls_crg_wdt_update_mode(cls_crg_wdt, CLS_CRG_WDT_RMOD_RESET);
        writel(CRG_WDOG_FEED_DOG, cls_crg_wdt->regs + CRG_WDOG_CLR_REG);

	/* Start Watchdog again*/
	/* wait for reset to assert... */
	mdelay(500);

	return 0;
}

static const struct watchdog_info cls_crg_wdt_ident = {
	.options	= WDIOF_KEEPALIVEPING | WDIOF_SETTIMEOUT |
			  WDIOF_MAGICCLOSE,
	.identity	= "Clourney TopCRG Watchdog",
};

static const struct watchdog_info cls_crg_wdt_pt_ident = {
	.options	= WDIOF_KEEPALIVEPING | WDIOF_SETTIMEOUT |
			  WDIOF_PRETIMEOUT | WDIOF_MAGICCLOSE,
	.identity	= "Clourney TopCRG Watchdog",
};

static const struct watchdog_ops cls_crg_wdt_ops = {
	.owner		= THIS_MODULE,
	.start		= cls_crg_wdt_start,
	.stop		= cls_crg_wdt_stop,
	.ping		= cls_crg_wdt_ping,
	.set_timeout	= cls_crg_wdt_set_timeout,
	.set_pretimeout	= cls_crg_wdt_set_pretimeout,
	.restart	= cls_crg_wdt_restart,
};

static irqreturn_t cls_crg_wdt_irq(int irq, void *devid)
{
	struct cls_crg_wdt *cls_crg_wdt = devid;
	/*
	 * We don't clear the IRQ status. It's supposed to be done by the
	 * following ping operations.
	 */
	cls_crg_wdt_ping(&cls_crg_wdt->wdd);

	//watchdog_notify_pretimeout(&cls_crg_wdt->wdd);

	return IRQ_HANDLED;
}

#ifdef CONFIG_DEBUG_FS

#define CLS_CRG_WDT_DBGFS_REG(_name, _off) \
{				      \
	.name = _name,		      \
	.offset = _off		      \
}

static const struct debugfs_reg32 cls_crg_wdt_dbgfs_regs[] = {
	CLS_CRG_WDT_DBGFS_REG("cfg0", CRG_WDOG_CFG0_REG),
	CLS_CRG_WDT_DBGFS_REG("cfg1", CRG_WDOG_TIMEOUT_REG),
	CLS_CRG_WDT_DBGFS_REG("cfg2", CRG_WDOG_START_REG),
	CLS_CRG_WDT_DBGFS_REG("cfg3", CRG_WDOG_CLR_REG),
	CLS_CRG_WDT_DBGFS_REG("rpt", CRG_WDOG_RPT),
	CLS_CRG_WDT_DBGFS_REG("rpt_clr", CRG_WDOG_RPT_CLR)
};

static void cls_crg_wdt_dbgfs_init(struct cls_crg_wdt *cls_crg_wdt)
{
	struct device *dev = cls_crg_wdt->wdd.parent;
	struct debugfs_regset32 *regset;

	regset = devm_kzalloc(dev, sizeof(*regset), GFP_KERNEL);
	if (!regset)
		return;

	regset->regs = cls_crg_wdt_dbgfs_regs;
	regset->nregs = ARRAY_SIZE(cls_crg_wdt_dbgfs_regs);
	regset->base = cls_crg_wdt->regs;

	cls_crg_wdt->dbgfs_dir = debugfs_create_dir(dev_name(dev), NULL);

	debugfs_create_regset32("registers", 0444, cls_crg_wdt->dbgfs_dir, regset);
}

static void cls_crg_wdt_dbgfs_clear(struct cls_crg_wdt *cls_crg_wdt)
{
	debugfs_remove_recursive(cls_crg_wdt->dbgfs_dir);
}

#else /* !CONFIG_DEBUG_FS */

static void cls_crg_wdt_dbgfs_init(struct cls_crg_wdt *cls_crg_wdt) {}
static void cls_crg_wdt_dbgfs_clear(struct cls_crg_wdt *cls_crg_wdt) {}

#endif /* !CONFIG_DEBUG_FS */

#ifdef CONFIG_OF
static const struct of_device_id cls_crg_wdt_dt_ids[] = {
        { .compatible = "clourney,crg-wdt", },
        { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, cls_crg_wdt_dt_ids);
#endif

static enum cls_crg_wdt_rmod cls_crg_wdt_parse_feed_mode(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	const char *mode_str;
	int ret;

	ret = of_property_read_string(np, "feed-mode", &mode_str);
	if (ret < 0)
		return CLS_CRG_WDT_RMOD_IRQ;

	if (strcmp(mode_str, "irq") == 0)
		return CLS_CRG_WDT_RMOD_IRQ;
	else if (strcmp(mode_str, "manual") == 0)
		return CLS_CRG_WDT_RMOD_RESET;

	dev_warn(&pdev->dev, "Invalid feed-mode: %s, using default\n", mode_str);
	return CLS_CRG_WDT_RMOD_IRQ;
}

static int cls_crg_wdt_drv_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct watchdog_device *wdd;
	struct cls_crg_wdt *cls_crg_wdt;
	const struct of_device_id *of_id;
	int ret, irq;

	of_id = of_match_device(cls_crg_wdt_dt_ids, &pdev->dev);
	if (of_id) {
		pdev->id_entry = of_id->data;
	} else {
		pr_err("Failed to find the right device id.\n");
		return -ENXIO;
	}

	cls_crg_wdt = devm_kzalloc(dev, sizeof(*cls_crg_wdt), GFP_KERNEL);
	if (!cls_crg_wdt)
		return -ENOMEM;

	cls_crg_wdt->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(cls_crg_wdt->regs))
		return PTR_ERR(cls_crg_wdt->regs);

        cls_crg_wdt->clk = devm_clk_get(dev, "osc_ref");
        if (IS_ERR(cls_crg_wdt->clk)) {
                cls_crg_wdt->clk = devm_clk_get(dev, NULL);
                if (IS_ERR(cls_crg_wdt->clk))
                        return PTR_ERR(cls_crg_wdt->clk);
        }

        cls_crg_wdt->rate = clk_get_rate(cls_crg_wdt->clk);
        if (cls_crg_wdt->rate == 0) {
		dev_err(dev, "cls_crg_wdt->rate get fail\n");
                return -EINVAL;;
        }
	dev_dbg(dev, "cls_crg_wdt->rate = %ld\n", cls_crg_wdt->rate);

	cls_crg_wdt->rmod = cls_crg_wdt_parse_feed_mode(pdev);
	cls_crg_wdt->wdd.info = &cls_crg_wdt_ident;

	/*
	 * Pre-timeout IRQ is optional, since some hardware may lack support
	 * of it. Note we must request rising-edge IRQ, since the lane is left
	 * pending either until the next watchdog kick event or up to the
	 * system reset.
	 */
	irq = platform_get_irq_optional(pdev, 0);
	if (irq > 0) {
		ret = devm_request_irq(dev, irq, cls_crg_wdt_irq,
				       IRQF_SHARED | IRQF_TRIGGER_RISING,
				       pdev->name, cls_crg_wdt);
		if (ret) {
			dev_err(dev, "request IRQ: nr %d\n", irq);
			goto probe_exit;
		}
		cls_crg_wdt->wdd.info = &cls_crg_wdt_pt_ident;
	}

	wdd = &cls_crg_wdt->wdd;
	wdd->ops = &cls_crg_wdt_ops;
	wdd->timeout = WDOG_DEFAULT_SECONDS;
	wdd->min_timeout = 1;
	wdd->max_hw_heartbeat_ms = ( cls_crg_wdt->rmod == CLS_CRG_WDT_RMOD_IRQ ) ?  ( WDOG_IRQ_MAX_SECONDS * 1000 ) : ( WDOG_RST_MAX_SECONDS * 1000 );
	wdd->parent = dev;

	watchdog_set_drvdata(wdd, cls_crg_wdt);
	watchdog_set_nowayout(wdd, nowayout);
	watchdog_init_timeout(wdd, 0, dev);
	cls_crg_wdt->timeout = wdd->timeout;

	/* TOP CRG Watchdog alreay start when SOC bootup.
	   Need feed watchdog first, then update work mode
	   and timeout setting. */
	cls_crg_wdt_ping(wdd);
	cls_crg_wdt_update_mode(cls_crg_wdt, cls_crg_wdt->rmod);
	cls_crg_wdt_set_timeout(wdd,wdd->timeout);

	platform_set_drvdata(pdev, cls_crg_wdt);

	watchdog_set_restart_priority(wdd, 128);

	ret = watchdog_register_device(wdd);
	if (ret) {
		dev_err(dev, "watchdog_register_device: err %d\n", ret);
		goto probe_exit;
	}
	cls_crg_wdt_dbgfs_init(cls_crg_wdt);

	dev_info(dev, "TOP CRG Watchdog work in %s mode @%dHz, timeout-sec %ds\n",((cls_crg_wdt->rmod == CLS_CRG_WDT_RMOD_IRQ) ? "irq" : "reset") , cls_crg_wdt->rate, wdd->timeout);

	return 0;

probe_exit:

	return ret;
}

static int cls_crg_wdt_drv_remove(struct platform_device *pdev)
{
	struct cls_crg_wdt *cls_crg_wdt = platform_get_drvdata(pdev);

	cls_crg_wdt_dbgfs_clear(cls_crg_wdt);

	watchdog_unregister_device(&cls_crg_wdt->wdd);

	return 0;
}

static struct platform_driver cls_crg_wdt_driver = {
	.probe		= cls_crg_wdt_drv_probe,
	.remove		= cls_crg_wdt_drv_remove,
	.driver		= {
		.name	= "cls_crg_wdt",
		.of_match_table = cls_crg_wdt_dt_ids,
	},
};

module_platform_driver(cls_crg_wdt_driver);

MODULE_AUTHOR("Weston Zhu");
MODULE_DESCRIPTION("ClourneySemi TOPCRG Watchdog Driver");
MODULE_LICENSE("GPL");
