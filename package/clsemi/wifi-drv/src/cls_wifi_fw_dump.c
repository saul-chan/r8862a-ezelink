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

#include <linux/kmod.h>
#include <linux/debugfs.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_debugfs.h"

static ssize_t cls_wifi_dbgfs_rhd_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->rhd_mem,
								   dump->dbg_info.rhd_len);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(rhd);

static ssize_t cls_wifi_dbgfs_rbd_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->rbd_mem,
								   dump->dbg_info.rbd_len);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(rbd);

static ssize_t cls_wifi_dbgfs_thdx_read(struct file *file, char __user *user_buf,
									size_t count, loff_t *ppos, int idx)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   &dump->thd_mem[idx],
								   dump->dbg_info.thd_len[idx]);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

static ssize_t cls_wifi_dbgfs_thd0_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_thdx_read(file, user_buf, count, ppos, 0);
}
DEBUGFS_READ_FILE_OPS(thd0);

static ssize_t cls_wifi_dbgfs_thd1_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_thdx_read(file, user_buf, count, ppos, 1);
}
DEBUGFS_READ_FILE_OPS(thd1);

static ssize_t cls_wifi_dbgfs_thd2_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_thdx_read(file, user_buf, count, ppos, 2);
}
DEBUGFS_READ_FILE_OPS(thd2);

static ssize_t cls_wifi_dbgfs_thd3_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_thdx_read(file, user_buf, count, ppos, 3);
}
DEBUGFS_READ_FILE_OPS(thd3);

#if (CLS_TXQ_CNT == 5)
static ssize_t cls_wifi_dbgfs_thd4_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_thdx_read(file, user_buf, count, ppos, 4);
}
DEBUGFS_READ_FILE_OPS(thd4);
#endif

static ssize_t cls_wifi_dbgfs_mactrace_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		char msg[64];
		mutex_unlock(&priv->dbgdump.mutex);
		scnprintf(msg, sizeof(msg), "Force trigger\n");
		cls_wifi_dbgfs_trigger_fw_dump(priv, msg);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								  dump->la_mem[0],
								  dump->dbg_info.la_conf[0].trace_len);

	mutex_unlock(&priv->dbgdump.mutex);

	return read;
}
DEBUGFS_READ_FILE_OPS(mactrace);

static ssize_t cls_wifi_dbgfs_phytrace_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								  dump->la_mem[1],
								  dump->dbg_info.la_conf[1].trace_len);

	mutex_unlock(&priv->dbgdump.mutex);

	return read;
}
DEBUGFS_READ_FILE_OPS(phytrace);

static ssize_t cls_wifi_dbgfs_macdiags_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->dbg_info.diags_mac,
								   DBG_DIAGS_MAC_MAX * 2);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(macdiags);

static ssize_t cls_wifi_dbgfs_phydiags_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->dbg_info.diags_phy,
								   DBG_DIAGS_PHY_MAX * 2);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(phydiags);

static ssize_t cls_wifi_dbgfs_hwdiags_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	char buf[16];
	int ret;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%08X\n", dump->dbg_info.hw_diag);

	mutex_unlock(&priv->dbgdump.mutex);
	return simple_read_from_buffer(user_buf, count, ppos, buf, ret);
}

DEBUGFS_READ_FILE_OPS(hwdiags);

static ssize_t cls_wifi_dbgfs_plfdiags_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	char buf[16];
	int ret;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%08X\n", dump->dbg_info.la_conf[0].diag_conf);

	mutex_unlock(&priv->dbgdump.mutex);
	return simple_read_from_buffer(user_buf, count, ppos, buf, ret);
}

DEBUGFS_READ_FILE_OPS(plfdiags);

static ssize_t cls_wifi_dbgfs_swdiags_read(struct file *file,
									  char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   &dump->dbg_info.sw_diag,
								   dump->dbg_info.sw_diag_len);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(swdiags);

static ssize_t cls_wifi_dbgfs_error_read(struct file *file,
									 char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->dbg_info.error,
								   strlen((char *)dump->dbg_info.error));

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(error);

static ssize_t cls_wifi_dbgfs_rxdesc_read(struct file *file,
									  char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	char buf[32];
	int ret;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%08X\n%08X\n", dump->dbg_info.rhd,
					dump->dbg_info.rbd);
	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(rxdesc);

static ssize_t cls_wifi_dbgfs_txdesc_read(struct file *file,
									  char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	char buf[64];
	int len = 0;
	int i;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	for (i = 0; i < CLS_TXQ_CNT; i++) {
		len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) - len - 1, count),
						 "%08X\n", dump->dbg_info.thd[i]);
	}

	mutex_unlock(&priv->dbgdump.mutex);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

DEBUGFS_READ_FILE_OPS(txdesc);

static ssize_t cls_wifi_dbgfs_macrxptr_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   &dump->dbg_info.rhd_hw_ptr,
								   2 * sizeof(dump->dbg_info.rhd_hw_ptr));

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}

DEBUGFS_READ_FILE_OPS(macrxptr);

static ssize_t cls_wifi_dbgfs_lamacconf_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->dbg_info.la_conf[0].conf,
								   LA_CONF_LEN * 4);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}
DEBUGFS_READ_FILE_OPS(lamacconf);

static ssize_t cls_wifi_dbgfs_laphyconf_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	ssize_t read;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	read = simple_read_from_buffer(user_buf, count, ppos,
								   dump->dbg_info.la_conf[1].conf,
								   LA_CONF_LEN * 4);

	mutex_unlock(&priv->dbgdump.mutex);
	return read;
}
DEBUGFS_READ_FILE_OPS(laphyconf);

static ssize_t cls_wifi_dbgfs_chaninfo_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_debug_dump_tag *dump = priv->dbgdump.buf.addr;
	char buf[4 * 32];
	int ret;

	mutex_lock(&priv->dbgdump.mutex);
	if (!priv->debugfs.trace_prst) {
		mutex_unlock(&priv->dbgdump.mutex);
		return 0;
	}

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"type:		  %d\n"
					"prim20_freq:   %d MHz\n"
					"center1_freq:  %d MHz\n"
					"center2_freq:  %d MHz\n",
					(dump->dbg_info.chan_info.info1 >> 8)  & 0xFF,
					(dump->dbg_info.chan_info.info1 >> 16) & 0xFFFF,
					(dump->dbg_info.chan_info.info2 >> 0)  & 0xFFFF,
					(dump->dbg_info.chan_info.info2 >> 16) & 0xFFFF);

	mutex_unlock(&priv->dbgdump.mutex);
	return simple_read_from_buffer(user_buf, count, ppos, buf, ret);
}

DEBUGFS_READ_FILE_OPS(chaninfo);

static ssize_t cls_wifi_dbgfs_um_helper_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[sizeof(priv->debugfs.helper_cmd)];
	int ret;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%s", priv->debugfs.helper_cmd);

	return simple_read_from_buffer(user_buf, count, ppos, buf, ret);
}

static ssize_t cls_wifi_dbgfs_um_helper_write(struct file *file,
										  const char __user *user_buf,
										  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	int eobuf = min_t(size_t, sizeof(priv->debugfs.helper_cmd) - 1, count);

	priv->debugfs.helper_cmd[eobuf] = '\0';
	if (copy_from_user(priv->debugfs.helper_cmd, user_buf, eobuf))
		return -EFAULT;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(um_helper);

/*
 * Calls a userspace pgm
 */
int cls_wifi_um_helper(struct cls_wifi_debugfs *cls_wifi_debugfs, const char *cmd)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(cls_wifi_debugfs, struct cls_wifi_hw,
										   debugfs);
	char *envp[] = { "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	char **argv;
	int argc, ret;

	if (!cls_wifi_debugfs->dir ||
		!strlen((cmd = cmd ? cmd : cls_wifi_debugfs->helper_cmd)))
		return 0;
	argv = argv_split(in_interrupt() ? GFP_ATOMIC : GFP_KERNEL, cmd, &argc);
	if (!argc)
		return PTR_ERR(argv);

	if ((ret = call_usermodehelper(argv[0], argv, envp,
								   UMH_WAIT_PROC | UMH_KILLABLE)))
		dev_err(cls_wifi_hw->dev, "Failed to call %s (%s returned %d)\n",
				argv[0], cmd, ret);
	argv_free(argv);

	return ret;
}

static void cls_wifi_um_helper_work(struct work_struct *ws)
{
	struct cls_wifi_debugfs *cls_wifi_debugfs = container_of(ws, struct cls_wifi_debugfs,
													 helper_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(cls_wifi_debugfs, struct cls_wifi_hw,
										   debugfs);
	cls_wifi_um_helper(cls_wifi_debugfs, NULL);
	if (!cls_wifi_debugfs->unregistering)
		cls_wifi_umh_done(cls_wifi_hw);
	cls_wifi_debugfs->helper_scheduled = false;
}

int cls_wifi_trigger_um_helper(struct cls_wifi_debugfs *cls_wifi_debugfs)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(cls_wifi_debugfs, struct cls_wifi_hw,
										   debugfs);

	if (cls_wifi_debugfs->helper_scheduled == true) {
		dev_err(cls_wifi_hw->dev, "%s: Already scheduled\n", __func__);
		return -EBUSY;
	}

	spin_lock_bh(&cls_wifi_debugfs->umh_lock);
	if (cls_wifi_debugfs->unregistering) {
		spin_unlock_bh(&cls_wifi_debugfs->umh_lock);
		dev_err(cls_wifi_hw->dev, "%s: unregistering\n", __func__);
		return -ENOENT;
	}
	cls_wifi_debugfs->helper_scheduled = true;
	schedule_work(&cls_wifi_debugfs->helper_work);
	spin_unlock_bh(&cls_wifi_debugfs->umh_lock);

	return 0;
}

void cls_wifi_wait_um_helper(struct cls_wifi_hw *cls_wifi_hw)
{
	cancel_work_sync(&cls_wifi_hw->debugfs.helper_work);
}

int cls_wifi_dbgfs_register_fw_dump(struct cls_wifi_hw *cls_wifi_hw,
								struct dentry *dir_drv,
								struct dentry *dir_diags)
{

	struct cls_wifi_debugfs *cls_wifi_debugfs = &cls_wifi_hw->debugfs;

	BUILD_BUG_ON(sizeof(CONFIG_CLS_WIFI_UM_HELPER_DFLT) >=
				 sizeof(cls_wifi_debugfs->helper_cmd));
	strncpy(cls_wifi_debugfs->helper_cmd,
			CONFIG_CLS_WIFI_UM_HELPER_DFLT, sizeof(cls_wifi_debugfs->helper_cmd));
	INIT_WORK(&cls_wifi_debugfs->helper_work, cls_wifi_um_helper_work);
	DEBUGFS_ADD_FILE(um_helper, dir_drv, S_IWUSR | S_IRUSR);

	cls_wifi_debugfs->trace_prst = cls_wifi_debugfs->helper_scheduled = false;
	spin_lock_init(&cls_wifi_debugfs->umh_lock);
	DEBUGFS_ADD_FILE(rhd,	   dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(rbd,	   dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(thd0,	  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(thd1,	  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(thd2,	  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(thd3,	  dir_diags, S_IRUSR);
#if (CLS_TXQ_CNT == 5)
	DEBUGFS_ADD_FILE(thd4,	  dir_diags, S_IRUSR);
#endif
	DEBUGFS_ADD_FILE(mactrace,  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(macdiags,  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(phydiags,  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(plfdiags,  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(hwdiags,   dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(swdiags,   dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(error,	 dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(rxdesc,	dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(txdesc,	dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(macrxptr,  dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(lamacconf, dir_diags, S_IRUSR);
	DEBUGFS_ADD_FILE(chaninfo,  dir_diags, S_IRUSR);

	if (cls_wifi_debugfs->la_phy) {
		DEBUGFS_ADD_FILE(phytrace,  dir_diags, S_IRUSR);
		DEBUGFS_ADD_FILE(laphyconf, dir_diags, S_IRUSR);
	}
	return 0;

  err:
	return -1;
}

void cls_wifi_dbgfs_config_fw_dump(struct cls_wifi_hw *cls_wifi_hw, bool la_phy)
{
	struct cls_wifi_debugfs *cls_wifi_debugfs = &cls_wifi_hw->debugfs;
	cls_wifi_debugfs->la_phy = la_phy;
}
