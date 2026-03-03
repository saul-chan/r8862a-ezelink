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
#include <pvt_sensor.h>

struct clourney_pvt_dev *pvt_dev = NULL;
unsigned int debug_pr_info = 0;
unsigned int debug_pr_info_m = 0;

module_param(debug_pr_info_m, uint, 0644);

static int debugfs_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;

	if (IS_ERR(pvt_dev)) {
		return PTR_ERR(pvt_dev);
	}

	pvt_ops.init(pvt_dev);

	return 0;
}

static int debugfs_close(struct inode *inode, struct file *filp)
{
	if (IS_ERR(pvt_dev)) {
		return PTR_ERR(pvt_dev);
	}

	pvt_ops.deinit(pvt_dev);

	return 0;
}

static ssize_t debugfs_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	if (IS_ERR(pvt_dev)) {
		return PTR_ERR(pvt_dev);
	}

	pvt_ops.read_process(pvt_dev);

	return 0;
}

static ssize_t debugfs_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	int    argc = 0;
	char * cmd = NULL;
	char   buf[PVT_CMD_MAX_LEN] = {0};
	bool   is_para_left = true;
	char *argv[PVT_CMD_MAX_LEN] = {0};

	if (IS_ERR(pvt_dev)) {
		return PTR_ERR(pvt_dev);
	}

	if (*ppos != 0)
		return 0;

	if ((*ppos + count) > sizeof(buf)) {
		count = sizeof(buf) - *ppos;
	}

	if (copy_from_user(buf, buffer, count)) {
		return -EFAULT;
	}

	printk(KERN_ERR "cmd[%s]\n", buf);

	cmd = strchr(buf, '\n');
	if (cmd) {
		*cmd = '\0';
		count = cmd - buf + 1;
	}

	cmd = buf;

	while ( '\0' != *cmd && argc < PVT_CMD_MAX_LEN) {
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

			if(strstr(argv[1], "printk")) {
				debug_pr_info = simple_strtol(argv[2], NULL, 16);
				printk(KERN_ERR "PVT cmd type[%s], debug_pr_info = %x\n", argv[1], debug_pr_info);
				if((debug_pr_info < 3))
					printk(KERN_ERR "pvt cmd value:[%d] invalid\n", debug_pr_info);
				else
					printk(KERN_ERR "pvt cmd value:[%d]\n", debug_pr_info);
			} else if (strstr(argv[1], "temp_rise_alarm1_L")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->temp_rise_alarm1_H)
					goto ERR;
				pvt_dev->temp_rise_alarm1_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_rise_alarm1_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->temp_rise_alarm1_L)
					goto ERR;
				pvt_dev->temp_rise_alarm1_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_rise_alarm2_L")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->temp_rise_alarm2_H)
					goto ERR;
				pvt_dev->temp_rise_alarm2_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_rise_alarm2_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->temp_rise_alarm2_L)
					goto ERR;
				pvt_dev->temp_rise_alarm2_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_rise_alarm3_L")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->temp_rise_alarm3_H)
					goto ERR;
				pvt_dev->temp_rise_alarm3_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_rise_alarm3_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->temp_rise_alarm3_L)
					goto ERR;
				pvt_dev->temp_rise_alarm3_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_fall_alarm1_L")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->temp_fall_alarm1_H)
					goto ERR;
				pvt_dev->temp_fall_alarm1_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_fall_alarm1_H")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->temp_fall_alarm1_L)
					goto ERR;
				pvt_dev->temp_fall_alarm1_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_fall_alarm2_L")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->temp_fall_alarm2_H)
					goto ERR;
				pvt_dev->temp_fall_alarm2_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_fall_alarm2_H")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->temp_fall_alarm2_L)
					goto ERR;
				pvt_dev->temp_fall_alarm2_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_fall_alarm3_L")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->temp_fall_alarm3_H)
					goto ERR;
				pvt_dev->temp_fall_alarm3_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "temp_fall_alarm3_H")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->temp_fall_alarm3_L)
					goto ERR;
				pvt_dev->temp_fall_alarm3_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_rise_alarm1_L")) {	
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->volt_rise_alarm1_H)
					goto ERR;
				pvt_dev->volt_rise_alarm1_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_rise_alarm1_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->volt_rise_alarm1_L)
					goto ERR;
				pvt_dev->volt_rise_alarm1_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_rise_alarm2_L")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->volt_rise_alarm2_H)
					goto ERR;
				pvt_dev->volt_rise_alarm2_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_rise_alarm2_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->volt_rise_alarm2_L)
					goto ERR;
				pvt_dev->volt_rise_alarm2_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_fall_alarm1_L")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->volt_fall_alarm1_H)
					goto ERR;
				pvt_dev->volt_fall_alarm1_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_fall_alarm1_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->volt_fall_alarm1_L)
					goto ERR;
				pvt_dev->volt_fall_alarm1_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_fall_alarm2_L")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->volt_fall_alarm2_H)
					goto ERR;
				pvt_dev->volt_fall_alarm2_L = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "volt_fall_alarm2_H")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->volt_fall_alarm2_L)
					goto ERR;
				pvt_dev->volt_fall_alarm2_H = simple_strtol(argv[2], NULL, 10);
			} else if (strstr(argv[1], "process_alarmA")) {
				if (simple_strtol(argv[2], NULL, 10) > pvt_dev->process_alarmB)
					goto ERR;
				pvt_dev->process_alarmA = simple_strtol(argv[2], NULL, 16);
			} else if (strstr(argv[1], "process_alarmB")) {
				if (simple_strtol(argv[2], NULL, 10) < pvt_dev->process_alarmA)
					goto ERR;
				pvt_dev->process_alarmB = simple_strtol(argv[2], NULL, 16);
			} else {
				printk(KERN_ERR "echo \"w cmd value\" > pvt_cmd not found\n");
				return EFAULT;
			}

			printk(KERN_INFO "################ upate date list ##############\n");
			printk(KERN_INFO "pvt_dev->temp_rise_alarm1_L = %d\n", pvt_dev->temp_rise_alarm1_L);
			printk(KERN_INFO "pvt_dev->temp_rise_alarm1_H = %d\n", pvt_dev->temp_rise_alarm1_H);
			printk(KERN_INFO "pvt_dev->temp_rise_alarm2_L = %d\n", pvt_dev->temp_rise_alarm2_L);
			printk(KERN_INFO "pvt_dev->temp_rise_alarm2_H = %d\n", pvt_dev->temp_rise_alarm2_H);
			printk(KERN_INFO "pvt_dev->temp_rise_alarm3_L = %d\n", pvt_dev->temp_rise_alarm3_L);
			printk(KERN_INFO "pvt_dev->temp_rise_alarm3_H = %d\n", pvt_dev->temp_rise_alarm3_H);
			printk(KERN_INFO "pvt_dev->temp_fall_alarm1_L = %d\n", pvt_dev->temp_fall_alarm1_L);
			printk(KERN_INFO "pvt_dev->temp_fall_alarm1_H = %d\n", pvt_dev->temp_fall_alarm1_H);
			printk(KERN_INFO "pvt_dev->temp_fall_alarm2_L = %d\n", pvt_dev->temp_fall_alarm2_L);
			printk(KERN_INFO "pvt_dev->temp_fall_alarm2_H = %d\n", pvt_dev->temp_fall_alarm2_H);
			printk(KERN_INFO "pvt_dev->temp_fall_alarm3_L = %d\n", pvt_dev->temp_fall_alarm3_L);
			printk(KERN_INFO "pvt_dev->temp_fall_alarm3_H = %d\n", pvt_dev->temp_fall_alarm3_H);

			printk(KERN_INFO "pvt_dev->process_alarmA = 0x%x\n", pvt_dev->process_alarmA);
			printk(KERN_INFO "pvt_dev->process_alarmB = 0x%x\n", pvt_dev->process_alarmB);

			printk(KERN_INFO "pvt_dev->volt_rise_alarm1_L = %d\n", pvt_dev->volt_rise_alarm1_L);
			printk(KERN_INFO "pvt_dev->volt_rise_alarm1_H = %d\n", pvt_dev->volt_rise_alarm1_H);
			printk(KERN_INFO "pvt_dev->volt_rise_alarm2_L = %d\n", pvt_dev->volt_rise_alarm2_L);
			printk(KERN_INFO "pvt_dev->volt_rise_alarm2_H = %d\n", pvt_dev->volt_rise_alarm2_H);
			printk(KERN_INFO "pvt_dev->volt_fall_alarm1_L = %d\n", pvt_dev->volt_fall_alarm1_L);
			printk(KERN_INFO "pvt_dev->volt_fall_alarm1_H = %d\n", pvt_dev->volt_fall_alarm1_H);
			printk(KERN_INFO "pvt_dev->volt_fall_alarm2_L = %d\n", pvt_dev->volt_fall_alarm2_L);
			printk(KERN_INFO "pvt_dev->volt_fall_alarm2_H = %d\n", pvt_dev->volt_fall_alarm2_H);
			printk(KERN_INFO "##############################################\n");

			mutex_lock(&pvt_dev->my_mutex);
			pvt_dev->temp_rise_alarm1 = (temperature_code_value_conversion(pvt_dev->temp_rise_alarm1_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_rise_alarm1_L);
			pvt_dev->temp_rise_alarm2 = (temperature_code_value_conversion(pvt_dev->temp_rise_alarm2_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_rise_alarm2_L);
			pvt_dev->temp_rise_alarm3 = (temperature_code_value_conversion(pvt_dev->temp_rise_alarm3_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_rise_alarm3_L);
			pvt_dev->temp_fall_alarm1 = (temperature_code_value_conversion(pvt_dev->temp_fall_alarm1_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_fall_alarm1_L);
			pvt_dev->temp_fall_alarm2 = (temperature_code_value_conversion(pvt_dev->temp_fall_alarm2_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_fall_alarm2_L);
			pvt_dev->temp_fall_alarm3 = (temperature_code_value_conversion(pvt_dev->temp_fall_alarm3_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_fall_alarm3_L);

			pvt_dev->volt_rise_alarm1 = (voltage_code_value_conversion(pvt_dev->volt_rise_alarm1_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_rise_alarm1_L);
			pvt_dev->volt_rise_alarm2 = (voltage_code_value_conversion(pvt_dev->volt_rise_alarm2_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_rise_alarm2_L);
			pvt_dev->volt_fall_alarm1 = (voltage_code_value_conversion(pvt_dev->volt_fall_alarm1_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_fall_alarm1_L);
			pvt_dev->volt_fall_alarm2 = (voltage_code_value_conversion(pvt_dev->volt_fall_alarm2_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_fall_alarm2_L);
			mutex_unlock(&pvt_dev->my_mutex);

			pvt_sensor_parameter_update(pvt_dev);
		}
		break;
	default:
		printk(KERN_ERR "PVT unknown cmd type[%d]", *argv[0]);
		break;
	}
	return count;
ERR:
	pr_info("ERROR: The input parameter sizes do not match!\n");
	return EFAULT;
}

struct file_operations debugfs_fops = {
	.owner = THIS_MODULE,
	.open  = debugfs_open,
	.release = debugfs_close,
	.read  = debugfs_read,
	.write = debugfs_write,
};

static struct dentry *dir = NULL;
static struct dentry *junk = NULL;

static int clourney_pvt_dbgfs_init(struct clourney_pvt_dev *clourney_pvt)
{
	dir = debugfs_create_dir("pvt", 0);
	if (!dir) {
		printk(KERN_ALERT "debugfs failed to create /sys/kernel/debug/pvt\n");
		return -1;
	}

	junk = debugfs_create_file(
			"pvt_cmd",
			0666,
			dir,
			NULL,
			&debugfs_fops);
	if (!junk) {
		printk(KERN_ALERT "debugfs_pvt: failed to create /sys/kernel/debug/pvt\n");
		return -1;
	}

	return 0;
}

static int clourney_pvt_get_resource(struct platform_device *pdev, struct clourney_pvt_dev *pvt_dev, struct resource **ret_irq)
{
	struct resource *mem, *irq;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	const char *dts_str;
	int err;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pvt_dev->reg_base = devm_ioremap_resource(dev, mem);
	if (IS_ERR(pvt_dev->reg_base)) {
		return PTR_ERR(pvt_dev->reg_base);
	}

	irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (!irq) {
		dev_err(&pdev->dev, "irq resource for pvt\n");
		return -ENXIO;
    }

	err = of_property_read_string(np, "temp_rise_alarm1_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm1\n");
		return -EINVAL;
	}
	pvt_dev->temp_rise_alarm1_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "temp_rise_alarm1_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm1 H\n");
		return -EINVAL;
	}
	pvt_dev->temp_rise_alarm1_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->temp_rise_alarm1 = (temperature_code_value_conversion(pvt_dev->temp_rise_alarm1_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_rise_alarm1_L);

	err = of_property_read_string(np, "temp_rise_alarm2_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm2 L\n");
		return -EINVAL;
	}
	pvt_dev->temp_rise_alarm2_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "temp_rise_alarm2_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_rise_alarm2_H\n");
		return -EINVAL;
	}
	pvt_dev->temp_rise_alarm2_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->temp_rise_alarm2 = (temperature_code_value_conversion(pvt_dev->temp_rise_alarm2_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_rise_alarm2_L);

	err = of_property_read_string(np, "temp_rise_alarm3_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm3 L\n");
		return -EINVAL;
	}
	pvt_dev->temp_rise_alarm3_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "temp_rise_alarm3_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp rise alarm3 H\n");
		return -EINVAL;
	}
	pvt_dev->temp_rise_alarm3_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->temp_rise_alarm3 = (temperature_code_value_conversion(pvt_dev->temp_rise_alarm3_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_rise_alarm3_L);

	err = of_property_read_string(np, "temp_fall_alarm1_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm1 L\n");
		return -EINVAL;
	}
	pvt_dev->temp_fall_alarm1_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "temp_fall_alarm1_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm1 H\n");
		return -EINVAL;
	}
	pvt_dev->temp_fall_alarm1_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->temp_fall_alarm1 = (temperature_code_value_conversion(pvt_dev->temp_fall_alarm1_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_fall_alarm1_L);

	err = of_property_read_string(np, "temp_fall_alarm2_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm2 L\n");
		return -EINVAL;
	}
	pvt_dev->temp_fall_alarm2_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "temp_fall_alarm2_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm2 H\n");
		return -EINVAL;
	}
	pvt_dev->temp_fall_alarm2_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->temp_fall_alarm2 = (temperature_code_value_conversion(pvt_dev->temp_fall_alarm2_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_fall_alarm2_L);

	err = of_property_read_string(np, "temp_fall_alarm3_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm3 L\n");
		return -EINVAL;
	}
	pvt_dev->temp_fall_alarm3_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "temp_fall_alarm3_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read temp_fall_alarm3 H\n");
		return -EINVAL;
	}
	pvt_dev->temp_fall_alarm3_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->temp_fall_alarm3 = (temperature_code_value_conversion(pvt_dev->temp_fall_alarm3_H) << 16) | temperature_code_value_conversion(pvt_dev->temp_fall_alarm3_L);

	err = of_property_read_string(np, "volt_rise_alarm1_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt rise alarm1_L\n");
		return -EINVAL;
	}
	pvt_dev->volt_rise_alarm1_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "volt_rise_alarm1_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt rise alarm1_H\n");
		return -EINVAL;
	}
	pvt_dev->volt_rise_alarm1_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->volt_rise_alarm1 = (voltage_code_value_conversion(pvt_dev->volt_rise_alarm1_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_rise_alarm1_L);

	err = of_property_read_string(np, "volt_rise_alarm2_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt rise alarm2_L\n");
		return -EINVAL;
	}
	pvt_dev->volt_rise_alarm2_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "volt_rise_alarm2_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt rise alarm2_H\n");
		return -EINVAL;
	}
	pvt_dev->volt_rise_alarm2_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->volt_rise_alarm2 = (voltage_code_value_conversion(pvt_dev->volt_rise_alarm2_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_rise_alarm2_L);

	err = of_property_read_string(np, "volt_rise_alarm1_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt rise alarm1_L\n");
		return -EINVAL;
	}
	pvt_dev->volt_rise_alarm1_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "volt_fall_alarm1_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt fall alarm1_H\n");
		return -EINVAL;
	}
	pvt_dev->volt_fall_alarm1_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->volt_fall_alarm1 = (voltage_code_value_conversion(pvt_dev->volt_fall_alarm1_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_fall_alarm1_L);

	err = of_property_read_string(np, "volt_rise_alarm2_L", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt rise alarm2_L\n");
		return -EINVAL;
	}
	pvt_dev->volt_rise_alarm2_L = simple_strtol(dts_str, NULL, 10);

	err = of_property_read_string(np, "volt_fall_alarm2_H", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read volt fall alarm2_H\n");
		return -EINVAL;
	}
	pvt_dev->volt_fall_alarm2_H = simple_strtol(dts_str, NULL, 10);
	pvt_dev->volt_fall_alarm2 = (voltage_code_value_conversion(pvt_dev->volt_fall_alarm2_H) << 16) | voltage_code_value_conversion(pvt_dev->volt_fall_alarm2_L);

	err = of_property_read_string(np, "process_alarmA", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read process alarmA\n");
		return -EINVAL;
	}
	pvt_dev->process_alarmA = simple_strtol(dts_str, NULL, 16);

	err = of_property_read_string(np, "process_alarmB", &dts_str);
	if (err < 0) {
		dev_err(dev, "Failed to read process alarmB\n");
		return -EINVAL;
	}
	pvt_dev->process_alarmB = simple_strtol(dts_str, NULL, 16);
	*ret_irq = irq;
	return 0;
}

static int pvt_sensor_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct resource *ret_irq;
	struct device *dev = &pdev->dev;
	struct clourney_pvt_priv *priv;

	pvt_dev = devm_kzalloc(dev, sizeof(struct clourney_pvt_dev), GFP_KERNEL);
	if (IS_ERR(pvt_dev)) {
		return PTR_ERR(pvt_dev);
	}

	ret = clourney_pvt_get_resource(pdev, pvt_dev, &ret_irq);
	if (ret < 0) {
		pr_info("failed to get pvt resource, err = %d\n", ret);
		return ret;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof *priv, GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "no memory for pvt private data\n");
		return -ENOMEM;
	}

	mutex_init(&pvt_dev->my_mutex);

	priv->pvt_dev = pvt_dev;
	spin_lock_init(&priv->hw_lock);
	tasklet_init(&priv->pop_jobs, pvt_pop_jobs, (unsigned long)priv);
	platform_set_drvdata(pdev, priv);

	if (devm_request_irq(&pdev->dev, ret_irq->start, pvt_irq_handler, IRQF_SHARED, dev_name(&pdev->dev), &pdev->dev)) {
	   dev_err(&pdev->dev, "failed to request IRQ\n");
	   return -EBUSY;
	}

	pvt_sensor_interrupt_init(pvt_dev);

	clourney_pvt_dbgfs_init(pvt_dev);

	return 0;
}

static int pvt_sensor_remove(struct platform_device *pdev)
{
	struct clourney_pvt_dev *pvt_dev = platform_get_drvdata(pdev);

	pvt_dev->reg_base = NULL;
	debugfs_remove(junk);
	debugfs_remove(dir);
	return 0;
}

static const struct of_device_id of_pvt_sensor_match[] = {
	{ .compatible = "clourney,dubhe-pvt-sensor", },
	{},
};

MODULE_DEVICE_TABLE(of, of_pvt_sensor_match);

static struct platform_driver pvt_sensor_driver = {
	.probe		= pvt_sensor_probe,
	.remove     = pvt_sensor_remove,
	.driver		= {
		.name	= "clourney,dubhe-pvt-sensor",
		.of_match_table = of_pvt_sensor_match,
	},
};

module_platform_driver(pvt_sensor_driver);

MODULE_AUTHOR("clourney");
MODULE_DESCRIPTION("PVT SENSOR driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pvt-sensor");

