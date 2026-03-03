/*
 * ClourneySemi TOPCRG HRST Driver
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/kobject.h>
#include <linux/input.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>


#define HRST_LENGTH_RPT_REG	0x0
#define HRST_RSP_OT_TH_REG	0x4
#define BH_SKB_SIZE	2048

#define DRV_NAME	"cls-hrst-button"
#define PFX	DRV_NAME ": "

struct cls_crg_hrst {
	void __iomem		*regs;
	unsigned long           rate;
};

struct bh_event {
	const char		*name;
	unsigned int		type;
	char			*action;
	unsigned long		seen;

	struct sk_buff		*skb;
	struct work_struct	work;
};

extern u64 uevent_next_seqnum(void);

static __printf(3, 4)
int bh_event_add_var(struct bh_event *event, int argv, const char *format, ...)
{
	char buf[128];
	char *s;
	va_list args;
	int len;

	if (argv)
		return 0;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	if (len >= sizeof(buf)) {
		WARN(1, "buffer size too small");
		return -ENOMEM;
	}

	s = skb_put(event->skb, len + 1);
	strcpy(s, buf);

	pr_debug(PFX "added variable '%s'\n", s);

	return 0;
}

static int button_hotplug_fill_event(struct bh_event *event)
{
	int ret;

	ret = bh_event_add_var(event, 0, "HOME=%s", "/");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "PATH=%s",
					"/sbin:/bin:/usr/sbin:/usr/bin");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "SUBSYSTEM=%s", "button");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "ACTION=%s", event->action);
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "BUTTON=%s", event->name);
	if (ret)
		return ret;

	if (event->type == EV_SW) {
		ret = bh_event_add_var(event, 0, "TYPE=%s", "switch");
		if (ret)
			return ret;
	}

	ret = bh_event_add_var(event, 0, "SEEN=%ld", event->seen);
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "SEQNUM=%llu", uevent_next_seqnum());

	return ret;
}

static void cls_hrst_button_hotplug_work(struct work_struct *work)
{
	struct bh_event *event = container_of(work, struct bh_event, work);
	int ret = 0;

	event->skb = alloc_skb(BH_SKB_SIZE, GFP_KERNEL);
	if (!event->skb)
		goto out_free_event;

	ret = bh_event_add_var(event, 0, "%s@", event->action);
	if (ret)
		goto out_free_skb;

	ret = button_hotplug_fill_event(event);
	if (ret)
		goto out_free_skb;

	NETLINK_CB(event->skb).dst_group = 1;
	broadcast_uevent(event->skb, 0, 1, GFP_KERNEL);

 out_free_skb:
	if (ret) {
		pr_err(PFX "work error %d\n", ret);
		kfree_skb(event->skb);
	}
 out_free_event:
	kfree(event);
}

static int cls_hrst_button_hotplug_create_event(const char *name, unsigned int type,
		unsigned long seen, int pressed)
{
	struct bh_event *event;

	pr_debug(PFX "create event, name=%s, seen=%lu, pressed=%d\n",
		 name, seen, pressed);

	event = kzalloc(sizeof(*event), GFP_ATOMIC);
	if (!event)
		return -ENOMEM;

	event->name = name;
	event->type = type;
	event->seen = seen;
	/*Actully, only "released" action is detected */
	event->action = pressed ? "pressed" : "released";

	INIT_WORK(&event->work, (void *)(void *)cls_hrst_button_hotplug_work);
	schedule_work(&event->work);

	return 0;
}

static irqreturn_t cls_hrst_button_interrupt(int irq, void *dev_id)
{
	struct cls_crg_hrst *cls_crg_hrst = dev_id;
	unsigned long seen;

	seen = readl(cls_crg_hrst->regs + HRST_LENGTH_RPT_REG);
	cls_hrst_button_hotplug_create_event("reset", EV_KEY, seen / cls_crg_hrst->rate, 0);
	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static const struct of_device_id cls_crg_hrst_dt_ids[] = {
	{ .compatible = "clourney,crg-hrst", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, cls_crg_hrst_dt_ids);
#endif

static int cls_crg_hrst_drv_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cls_crg_hrst *cls_crg_hrst;
	struct clk              *clk;
	const struct of_device_id *of_id;
	int ret, irq;
	unsigned int val;

	of_id = of_match_device(cls_crg_hrst_dt_ids, &pdev->dev);
	if (of_id) {
		pdev->id_entry = of_id->data;
	} else {
		pr_err(PFX "Failed to find the right device id.\n");
		return -ENXIO;
	}

	cls_crg_hrst = devm_kzalloc(dev, sizeof(*cls_crg_hrst), GFP_KERNEL);
	if (!cls_crg_hrst)
		return -ENOMEM;

	cls_crg_hrst->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(cls_crg_hrst->regs))
		return PTR_ERR(cls_crg_hrst->regs);

	clk = devm_clk_get(dev, "osc_ref");
	if (IS_ERR(clk)) {
		clk = devm_clk_get(dev, NULL);
		if (IS_ERR(clk))
			return PTR_ERR(clk);
	}
	cls_crg_hrst->rate = clk_get_rate(clk);
	if (cls_crg_hrst->rate == 0) {
		dev_err(dev, "fail to get cls_crg_hrst->rate\n");
		return -EINVAL;
	}

	/* According to my test,set timeout to 0x00ffffff to let software have more
	 * time to deal with interrupt.It's 3 bytes.
	 */
	val = readl(cls_crg_hrst->regs + HRST_RSP_OT_TH_REG);
	writel(val | 0x00ffffff, cls_crg_hrst->regs + HRST_RSP_OT_TH_REG);

	irq = platform_get_irq_optional(pdev, 0);
	if (irq > 0) {
		ret = devm_request_irq(dev, irq, cls_hrst_button_interrupt,
				       IRQF_SHARED | IRQF_TRIGGER_RISING,
				       pdev->name, cls_crg_hrst);
		if (ret) {
			dev_err(dev, "request IRQ: nr %d\n", irq);
			goto probe_exit;
		}
	} else {
		if (irq == -EPROBE_DEFER) {
			dev_err(dev, "platform_get_irq_optional: err %d\n", ret);
			goto probe_exit;
		}
	}

	dev_info(dev, "TOP CRG HRST driver probe successfully, irq number is %d, clock rate is %luHz\n",
			irq, cls_crg_hrst->rate);

	return 0;

probe_exit:
	return ret;
}

static struct platform_driver cls_crg_hrst_driver = {
	.probe		= cls_crg_hrst_drv_probe,
	.driver		= {
		.name	= "cls_crg_hrst",
		.of_match_table = cls_crg_hrst_dt_ids,
	},
};

module_platform_driver(cls_crg_hrst_driver);

MODULE_AUTHOR("Tony He");
MODULE_DESCRIPTION("ClourneySemi TOPCRG HRST Driver");
MODULE_LICENSE("GPL");
