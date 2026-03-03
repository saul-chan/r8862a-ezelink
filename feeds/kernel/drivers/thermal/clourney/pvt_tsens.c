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
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include "pvt_tsens.h"

unsigned int debug_pr_info_m;

module_param(debug_pr_info_m, uint, 0644);

static int debugfs_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;

	if (IS_ERR(tsens_dev))
		return PTR_ERR(tsens_dev);

	tsens_ops.init(tsens_dev);

	return 0;
}

static int debugfs_close(struct inode *inode, struct file *filp)
{
	if (IS_ERR(tsens_dev))
		return PTR_ERR(tsens_dev);

	tsens_ops.deinit(tsens_dev);

	return 0;
}

static ssize_t debugfs_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	if (IS_ERR(tsens_dev))
		return PTR_ERR(tsens_dev);

	tsens_ops.read_process(tsens_dev);

	return 0;
}

static void cls_tsens_alarm_work(struct work_struct *ws)
{
	int ret = 0;

	printk(KERN_WARNING  "Pre-process AlarmA=%x AlarmB=%x",
		tsens_dev->alarma_flag, tsens_dev->alarmb_flag);

	printk(KERN_WARNING  "TS0(%d) TS1(%d) TS2(%d)",
		tsens_dev->tsens_data[0],
		tsens_dev->tsens_data[1],
		tsens_dev->tsens_data[2]);

	if (tsens_dev->handler_hooks) {
		ret = tsens_dev->handler_hooks(tsens_dev);
		if (ret) {
			dev_err(tsens_dev->dev, "tsens_dev->handler_hooks err (%d)\n", ret);
			return;
		}
	} else
		dev_warn(tsens_dev->dev, "tsens_dev->handler_hooks is null\n");

	spin_lock(&tsens_dev->alarm_lock);
	tsens_dev->alarma_flag = 0;
	tsens_dev->alarmb_flag = 0;
	spin_unlock(&tsens_dev->alarm_lock);
	printk(KERN_WARNING  "Post-process AlarmA=%x AlarmB=%x\n",
			tsens_dev->alarma_flag, tsens_dev->alarmb_flag);
}

static ssize_t debugfs_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	int    argc = 0, temp_val = 0, err = 0;
	char   *cmd = NULL;
	char   buf[PVT_CMD_MAX_LEN] = {0};
	bool   is_para_left = true;
	char   *argv[PVT_CMD_MAX_LEN] = {0};

	if (IS_ERR(tsens_dev))
		return PTR_ERR(tsens_dev);

	if (*ppos != 0)
		return 0;

	if ((*ppos + count) > sizeof(buf))
		count = sizeof(buf) - *ppos;

	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	printk(KERN_ERR "cmd %s", buf);

	cmd = strchr(buf, '\n');
	if (cmd) {
		*cmd = '\0';
		count = cmd - buf + 1;
	}

	cmd = buf;

	while ('\0' != *cmd && argc < PVT_CMD_MAX_LEN) {
		if ('\t' == *cmd || ' ' == *cmd || '\n' == *cmd) {
			*cmd = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = cmd;
		}
		cmd++;
	}

	if (argc < 2) {
		printk(KERN_ERR " need ARG >= 2");
		return -EFAULT;
	}

	switch (*argv[0])  {
	case PVT_WRITE_CMD:
		{
			if (argc != 3) {
				printk(KERN_ERR "Err ARG num[%d], Please use cmd:\" w <address> <value>\"\n", argc);
				return -EFAULT;
			}

			if (strstr(argv[1], "printk")) {
				err = kstrtol(argv[2], 16, (long *)&debug_pr_info);
				if (err)
					goto ERR;
				printk(KERN_ERR "t-sensor cmd type[%s], debug_pr_info = %x\n", argv[1], debug_pr_info);
				if ((debug_pr_info < 3))
					printk(KERN_ERR "t-sensor cmd value:[%d] invalid\n", debug_pr_info);
				else
					printk(KERN_ERR "t-sensor cmd value:[%d]\n", debug_pr_info);
			} else  if (strstr(argv[1], "alarm_enable")) {
				err = kstrtol(argv[2], 16, (long *)&tsens_dev->alarm_work_enable);
				if (err)
					goto ERR;
				printk(KERN_ERR "t-sensor cmd type[%s], alarm_enable = %x\n",
					argv[1], tsens_dev->alarm_work_enable);
			} else if (strstr(argv[1], "temp_rise_alarm0_L")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val > tsens_dev->temp_rise_alarm0_H)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_rise_alarm0_L);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_rise_alarm0_H")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val < tsens_dev->temp_rise_alarm0_L)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_rise_alarm0_H);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_rise_alarm1_L")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val > tsens_dev->temp_rise_alarm1_H)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_rise_alarm1_L);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_rise_alarm1_H")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val < tsens_dev->temp_rise_alarm1_L)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_rise_alarm1_H);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_rise_alarm2_L")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val > tsens_dev->temp_rise_alarm2_H)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_rise_alarm2_L);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_rise_alarm2_H")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val < tsens_dev->temp_rise_alarm2_L)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_rise_alarm2_H);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_fall_alarm0_H")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val < tsens_dev->temp_fall_alarm0_L)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_fall_alarm0_H);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_fall_alarm0_L")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val > tsens_dev->temp_fall_alarm0_H)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_fall_alarm0_L);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_fall_alarm1_H")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val < tsens_dev->temp_fall_alarm1_L)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_fall_alarm1_H);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_fall_alarm1_L")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val > tsens_dev->temp_fall_alarm1_H)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_fall_alarm1_L);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_fall_alarm2_H")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val < tsens_dev->temp_fall_alarm2_L)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_fall_alarm2_H);
				if (err)
					goto ERR;
			} else if (strstr(argv[1], "temp_fall_alarm2_L")) {
				err = kstrtol(argv[2], 10, (long *)&temp_val);
				if (err)
					goto ERR;
				if (temp_val > tsens_dev->temp_fall_alarm2_H)
					goto ERR;
				err = kstrtol(argv[2], 10, (long *)&tsens_dev->temp_fall_alarm2_L);
				if (err)
					goto ERR;
			} else {
				printk(KERN_ERR "echo \"w cmd value\" > pvt_cmd not found\n");
				return -EFAULT;
			}

			printk(KERN_INFO "################ update date list ##############\n");
			printk(KERN_INFO "tsens_dev->temp_rise_alarm0_L = %d\n", tsens_dev->temp_rise_alarm0_L);
			printk(KERN_INFO "tsens_dev->temp_rise_alarm0_H = %d\n", tsens_dev->temp_rise_alarm0_H);
			printk(KERN_INFO "tsens_dev->temp_rise_alarm1_L = %d\n", tsens_dev->temp_rise_alarm1_L);
			printk(KERN_INFO "tsens_dev->temp_rise_alarm1_H = %d\n", tsens_dev->temp_rise_alarm1_H);
			printk(KERN_INFO "tsens_dev->temp_rise_alarm2_L = %d\n", tsens_dev->temp_rise_alarm2_L);
			printk(KERN_INFO "tsens_dev->temp_rise_alarm2_H = %d\n", tsens_dev->temp_rise_alarm2_H);
			printk(KERN_INFO "tsens_dev->temp_fall_alarm0_L = %d\n", tsens_dev->temp_fall_alarm0_L);
			printk(KERN_INFO "tsens_dev->temp_fall_alarm0_H = %d\n", tsens_dev->temp_fall_alarm0_H);
			printk(KERN_INFO "tsens_dev->temp_fall_alarm1_L = %d\n", tsens_dev->temp_fall_alarm1_L);
			printk(KERN_INFO "tsens_dev->temp_fall_alarm1_H = %d\n", tsens_dev->temp_fall_alarm1_H);
			printk(KERN_INFO "tsens_dev->temp_fall_alarm2_L = %d\n", tsens_dev->temp_fall_alarm2_L);
			printk(KERN_INFO "tsens_dev->temp_fall_alarm2_H = %d\n", tsens_dev->temp_fall_alarm2_H);
			printk(KERN_INFO "##############################################\n");

			tsens_parameter_update(tsens_dev);
		}
		break;
	default:
		printk(KERN_ERR "T-Sensor unknown cmd type[%d]", *argv[0]);
		break;
	}
	return count;
ERR:
	pr_info("ERROR: The input parameter sizes do not match!\n");
	return -EFAULT;
}

const struct file_operations debugfs_fops = {
	.owner = THIS_MODULE,
	.open  = debugfs_open,
	.release = debugfs_close,
	.read  = debugfs_read,
	.write = debugfs_write,
};

static struct dentry *dir;
static struct dentry *junk;

static int cls_tsens_dbgfs_init(struct cls_tsens_dev *tsens_dev)
{
	dir = debugfs_create_dir("cls_tsens", 0);
	if (!dir) {
		printk(KERN_ALERT "debugfs failed to create /sys/kernel/debug/cls_tsens\n");
		return -1;
	}

	junk = debugfs_create_file(
			"tsens_cmd",
			0666,
			dir,
			NULL,
			&debugfs_fops);
	if (!junk) {
		printk(KERN_ALERT "debugfs_tsens: failed to create /sys/kernel/debug/cls_tsens\n");
		return -1;
	}

	return 0;
}

static int cls_tsens_get_resource(struct platform_device *pdev,
		struct cls_tsens_dev *tsens_dev, struct resource **ret_irq)
{
	struct resource *mem, *irq;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	const char *dts_str;
	int err;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	tsens_dev->reg_base = devm_ioremap_resource(dev, mem);

	if (IS_ERR(tsens_dev->reg_base))
		return PTR_ERR(tsens_dev->reg_base);

	irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

	if (!irq) {
		dev_err(&pdev->dev, "irq resource for pvt\n");
		return -ENXIO;
	}

	err = of_property_read_string(np, "temp_rise_alarm0_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm0\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_rise_alarm0_L);
	if (err < 0)
		return -EINVAL;

	err = of_property_read_string(np, "temp_rise_alarm0_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm0 H\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_rise_alarm0_H);
	if (err < 0)
		return -EINVAL;

	tsens_dev->temp_rise_alarm0 =
		(temperature_code_value_conversion(tsens_dev->temp_rise_alarm0_H) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_rise_alarm0_L);

	err = of_property_read_string(np, "temp_rise_alarm1_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm1 L\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_rise_alarm1_L);
	if (err < 0)
		return -EINVAL;

	err = of_property_read_string(np, "temp_rise_alarm1_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_rise_alarm1_H\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_rise_alarm1_H);
	if (err < 0)
		return -EINVAL;

	tsens_dev->temp_rise_alarm1 =
		(temperature_code_value_conversion(tsens_dev->temp_rise_alarm1_H) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_rise_alarm1_L);

	err = of_property_read_string(np, "temp_rise_alarm2_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm2 L\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_rise_alarm2_L);
	if (err < 0)
		return -EINVAL;

	err = of_property_read_string(np, "temp_rise_alarm2_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm2 H\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_rise_alarm2_H);
	if (err < 0)
		return -EINVAL;

	tsens_dev->temp_rise_alarm2 =
		(temperature_code_value_conversion(tsens_dev->temp_rise_alarm2_H) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_rise_alarm2_L);

	err = of_property_read_string(np, "temp_fall_alarm0_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm0 L\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_fall_alarm0_L);
	if (err < 0)
		return -EINVAL;

	err = of_property_read_string(np, "temp_fall_alarm0_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm0 H\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_fall_alarm0_H);
	if (err < 0)
		return -EINVAL;

	tsens_dev->temp_fall_alarm0 =
		(temperature_code_value_conversion(tsens_dev->temp_fall_alarm0_L) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_fall_alarm0_H);

	err = of_property_read_string(np, "temp_fall_alarm1_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm1 L\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_fall_alarm1_L);
	if (err < 0)
		return -EINVAL;

	err = of_property_read_string(np, "temp_fall_alarm1_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm1 H\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_fall_alarm1_H);
	if (err < 0)
		return -EINVAL;

	tsens_dev->temp_fall_alarm1 =
		(temperature_code_value_conversion(tsens_dev->temp_fall_alarm1_L) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_fall_alarm1_H);

	err = of_property_read_string(np, "temp_fall_alarm2_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm2 L\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_fall_alarm2_L);
	if (err < 0)
		return -EINVAL;

	err = of_property_read_string(np, "temp_fall_alarm2_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm2 H\n");
		return -EINVAL;
	}

	err = kstrtol(dts_str, 10, (long *)&tsens_dev->temp_fall_alarm2_H);
	if (err < 0)
		return -EINVAL;

	tsens_dev->temp_fall_alarm2 =
		(temperature_code_value_conversion(tsens_dev->temp_fall_alarm2_L) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_fall_alarm2_H);

	*ret_irq = irq;
	return 0;
}

static int cls_tsens_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct resource *ret_irq;
	struct device *dev = &pdev->dev;
	struct cls_tsens_priv *priv;

	tsens_dev = devm_kzalloc(dev, sizeof(struct cls_tsens_dev), GFP_KERNEL);

	if (IS_ERR(tsens_dev))
		return PTR_ERR(tsens_dev);

	ret = cls_tsens_get_resource(pdev, tsens_dev, &ret_irq);
	if (ret < 0) {
		pr_info("failed to get pvt resource, err = %d\n", ret);
		return ret;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);

	if (!priv)
		return -ENOMEM;

	priv->tsens_dev = tsens_dev;
	tsens_dev->dev = dev;

	mutex_init(&tsens_dev->my_mutex);
	spin_lock_init(&priv->hw_lock);
	spin_lock_init(&tsens_dev->alarm_lock);
	tasklet_init(&priv->pop_jobs, tsens_pop_jobs, (unsigned long)priv);
	platform_set_drvdata(pdev, priv);

	if (devm_request_irq(&pdev->dev, ret_irq->start,
			tsens_irq_handler, IRQF_SHARED, dev_name(&pdev->dev), &pdev->dev)) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		return -EBUSY;
	}

	INIT_WORK(&tsens_dev->tsens_work, cls_tsens_alarm_work);
	tsens_interrupt_init(tsens_dev);
	cls_tsens_dbgfs_init(tsens_dev);

	return 0;
}

static int cls_tsens_remove(struct platform_device *pdev)
{
	struct cls_tsens_dev *tsens_dev = platform_get_drvdata(pdev);

	tsens_dev->reg_base = NULL;
	debugfs_remove(junk);
	debugfs_remove(dir);
	return 0;
}

static const struct of_device_id of_tsensor_match[] = {
	{ .compatible = "clourney,t-sensor", },
	{},
};

MODULE_DEVICE_TABLE(of, of_tsensor_match);

static struct platform_driver tsensor_driver = {
	.probe		= cls_tsens_probe,
	.remove     = cls_tsens_remove,
	.driver		= {
		.name	= "clourney,t-sensor",
		.of_match_table = of_tsensor_match,
	},
};

module_platform_driver(tsensor_driver);

MODULE_AUTHOR("clourney");
MODULE_DESCRIPTION("T-SENSOR driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:t-sensor");

