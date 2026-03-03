/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

#include "fwt.h"

static struct dentry *cls_fwt_dbg_root;

/**************************************************************
 * command
 * The command entry in debugfs is for giving the driver commands
 * to be executed - these may be for changing the internal switch
 * setup, adding or removing filters, or other things.  Many of
 * these will be useful for some forms of unit testing.
 **************************************************************/
static char cls_fwt_dbg_command_buf[256] = "";

/**
 * cls_fwt_dbg_command_read - read for command datum
 * @filp: the opened file
 * @buffer: where to write the data for the user to read
 * @count: the size of the user's buffer
 * @ppos: file position offset
 **/
static ssize_t cls_fwt_dbg_command_read(struct file *filp, char __user *buffer,
					size_t count, loff_t *ppos)
{
	int bytes_not_copied;
	int buf_size = 256;
	char *buf;
	int len;

	/* don't allow partial reads */
	if (*ppos != 0)
		return 0;

	if (count < buf_size)
		return -ENOSPC;

	buf = kzalloc(buf_size, GFP_KERNEL);
	if (!buf)
		return -ENOSPC;

	len = snprintf(buf, buf_size, "Result: %s\n",
		       cls_fwt_dbg_command_buf);

	bytes_not_copied = copy_to_user(buffer, buf, len);
	kfree(buf);

	if (bytes_not_copied)
		return -EFAULT;

	*ppos = len;

	return len;
}

static void cls_fwt_dbg_command_usage(void)
{
	pr_info("Available FWT debugfs commands:\n");
	pr_info("        print\n");
	pr_info("        counter\n");
	pr_info("        cache\n");
	pr_info("        node\n");
	pr_info("        dbg\n");
	pr_info("        4addr\n");
	pr_info("        switch\n");
	pr_info("        lookup\n");
}

static ssize_t cls_fwt_dbg_command_write(struct file *filp,
					 const char __user *buffer,
					 size_t count, loff_t *ppos)
{
	char *cmd_buf, *cmd_buf_tmp;
	int bytes_not_copied;
	uint32_t index;
	fwt_db_node_element *ptr;
	fwt_db_node_element *tmp;
	int cnt;

	/* don't allow partial writes */
	if (*ppos != 0)
		return 0;

	cmd_buf = kzalloc(count + 1, GFP_KERNEL);
	if (!cmd_buf) {
		pr_info("[%s] alloc fialed!\n", __func__);

		return count;
	}

	bytes_not_copied = copy_from_user(cmd_buf, buffer, count);
	if (bytes_not_copied) {
		kfree(cmd_buf);
		return -EFAULT;
	}

	cmd_buf[count] = '\0';
	cmd_buf_tmp = strchr(cmd_buf, '\n');

	if (cmd_buf_tmp) {
		*cmd_buf_tmp = '\0';
		count = cmd_buf_tmp - cmd_buf + 1;
	}

	if (strncmp(cmd_buf, "print", 5) == 0) {
		fwt_db_print();
	} else if (strncmp(cmd_buf, "counter", 7) == 0) {
		pr_info("g_fwt_ent_cnt: %u\n", g_fwt_ent_cnt);
	} else if (strncmp(cmd_buf, "cache", 5) == 0) {
		pr_info("g_fwt_cache_entry_index %d: %d %d %d %d\n", g_fwt_cache_entry_index,
					g_fwt_cache[0], g_fwt_cache[1],
					g_fwt_cache[2], g_fwt_cache[3]);
	} else if (strncmp(cmd_buf, "node", 4) == 0) {
		if (sscanf(&cmd_buf[5], "%i", &index) == 1) {
			if (index >= CLS_NCIDX_MAX) {
				pr_info("[%s] invalid node index\n", __func__);
				goto command_write_done;
			}

			ptr = NULL;
			tmp = NULL;
			cnt = 0;

			STAILQ_FOREACH_SAFE(ptr, &g_fwt_db_node_table[index], next, tmp)
			{
				pr_info("##node_tbl_list[0x%x] fwt_idx=%d\n", index, ptr->index);
				cnt++;
			}
			pr_info("####node_tbl_list[0x%x]: cnt=%d\n", index, cnt);
		}
	} else if (strncmp(cmd_buf, "dbg", 3) == 0) {
		if (sscanf(&cmd_buf[4], "%i", &index) == 1)
			cls_fwt_dbg_enable = !!index;

		pr_info("fwt debug: %d\n", cls_fwt_dbg_enable);
	} else if (strncmp(cmd_buf, "4addr", 5) == 0) {
		if (sscanf(&cmd_buf[6], "%i", &index) == 1)
			cls_fwt_4addr_by_vif = !!index;

		if (cls_fwt_4addr_by_vif)
			pr_info("###4addr determined by vif");
		else
			pr_info("###4addr determined by fwt");
	} else if (strncmp(cmd_buf, "switch", 5) == 0) {
		if (sscanf(&cmd_buf[6], "%i", &index) == 1) {
			if (index < CLS_FWT_SWITCH_MODE_MIN || index > CLS_FWT_SWITCH_MODE_MAX)
				cls_fwt_switch_enable = 0;
			else
				cls_fwt_switch_enable = index;
		}

		pr_info("fwt switch table enable: %d\n", cls_fwt_switch_enable);
	} else if (strncmp(cmd_buf, "lookup", 6) == 0) {
		uint8_t cnt, mac_array[6];
		uint16_t vid, fwt_idx;
		uint32_t sub_port;

		struct net_device *outdev;

		memset(cls_fwt_dbg_command_buf, 0, sizeof(cls_fwt_dbg_command_buf));

		cnt = sscanf(&cmd_buf[7], "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx %hi",
			&mac_array[0], &mac_array[1], &mac_array[2], &mac_array[3], &mac_array[4], &mac_array[5], &vid);
		if (cnt != 7) {
		        pr_err("invalid params: lookup <macaddr> <vid>");
		} else {
			outdev = fwt_sw_get_entry_info(mac_array, vid, &fwt_idx, &sub_port);
			if (outdev && fwt_db_get_valid_entry(fwt_idx))
				snprintf(cls_fwt_dbg_command_buf, sizeof(cls_fwt_dbg_command_buf),
					"<%pM.%u> lookup success: index=%u sub_port=0x%x outdev=%s", mac_array, vid,
					fwt_idx, sub_port, outdev->name);
			else
				snprintf(cls_fwt_dbg_command_buf, sizeof(cls_fwt_dbg_command_buf),
					"<%pM.%u> lookup failed!", mac_array, vid);
		}
	} else {
		pr_info("unknown command '%s'\n", cmd_buf);
		cls_fwt_dbg_command_usage();
	}

command_write_done:
	kfree(cmd_buf);
	cmd_buf = NULL;

	return count;
}

static const struct file_operations cls_fwt_dbg_command_fops = {
	.owner = THIS_MODULE,
	.open =  simple_open,
	.read =  cls_fwt_dbg_command_read,
	.write = cls_fwt_dbg_command_write,
};

/**
 * cls_fwt_dbg_init - start up debugfs for the driver
 **/
void cls_fwt_dbg_init(void)
{
	cls_fwt_dbg_root = debugfs_create_dir(cls_fwt_driver_name, NULL);

	if (!cls_fwt_dbg_root) {
		pr_info("CLS FWT init of debugfs failed\n");

		return;
	}

	debugfs_create_file("command", 0600, cls_fwt_dbg_root, NULL,
			    &cls_fwt_dbg_command_fops);
}

/**
 * cls_fwt_dbg_exit - stop debugfs for the driver
 **/
void cls_fwt_dbg_exit(void)
{
	debugfs_remove_recursive(cls_fwt_dbg_root);
	cls_fwt_dbg_root = NULL;
}

