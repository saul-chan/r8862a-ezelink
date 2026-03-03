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

#include <linux/firmware.h>
#include <linux/if_ether.h>
#include <linux/fs.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_afe.h"
#include "cls_wifi_msg_tx.h"

/**
 *
 */
static const char *cls_wifi_find_tag(const u8 *file_data, unsigned int file_size,
								 const char *tag_name, unsigned int tag_len)
{
	unsigned int curr, line_start = 0, line_size;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Walk through all the lines of the configuration file */
	while (line_start < file_size) {
		/* Search the end of the current line (or the end of the file) */
		for (curr = line_start; curr < file_size; curr++)
			if (file_data[curr] == '\n')
				break;

		/* Compute the line size */
		line_size = curr - line_start;

		/* Check if this line contains the expected tag */
		if ((line_size == (strlen(tag_name) + tag_len)) &&
			(!strncmp(&file_data[line_start], tag_name, strlen(tag_name))))
			return (&file_data[line_start + strlen(tag_name)]);

		/* Move to next line */
		line_start = curr + 1;
	}

	/* Tag not found */
	return NULL;
}

/**
 * Parse the Config file used at init time
 */
int cls_wifi_parse_configfile(struct cls_wifi_hw *cls_wifi_hw, const char *filename,
						  struct cls_wifi_conf_file *config)
{
	const struct firmware *config_fw;
	u8 dflt_mac[ETH_ALEN] = { 0, 111, 111, 111, 111, 0 };
	int ret;
	const u8 *tag_ptr = NULL;
	char tag_name[12];
	int mac_flag = 0;
	char full_name[FS_PATH_MAX_LEN];

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if ((strlen(cls_wifi_hw->plat->hw_params.firmware_path) + strlen(filename)) >
			FS_PATH_MAX_LEN)
	{
		pr_warn("file name or file path overlong\n");
		return -1;
	}

	strcpy(full_name, cls_wifi_hw->plat->hw_params.firmware_path);
	strcat(full_name, filename);
	pr_warn("open target file : %s\n", full_name);

	ret = request_firmware(&config_fw, full_name, cls_wifi_hw->dev);

	if (ret) {
		pr_warn("%s: Failed to get %s (%d)\n", __func__, filename, ret);
		return ret;
	}

	if (!cls_wifi_hw->radio_idx)
		strcpy(tag_name, "MAC_ADDR=");
	else
		sprintf(tag_name, "MAC_ADDR%d=", cls_wifi_hw->radio_idx);

	tag_ptr = cls_wifi_find_tag(config_fw->data, config_fw->size, tag_name, strlen("00:00:00:00:00:00"));

	if (tag_ptr != NULL) {
		u8 *addr = config->mac_addr;

		if (sscanf(tag_ptr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				addr + 0, addr + 1, addr + 2,
				addr + 3, addr + 4, addr + 5) != ETH_ALEN)
			memcpy(config->mac_addr, dflt_mac, ETH_ALEN);
		else
			mac_flag = 1;
	} else
		memcpy(config->mac_addr, dflt_mac, ETH_ALEN);

	if (!mac_flag && !cls_wifi_hw->radio_idx) {
		if (config->mac_addr[5] & 0x1)
			config->mac_addr[5] = config->mac_addr[5] & 0xFE;
		else
			config->mac_addr[5] |= 1;
	}

	CLS_WIFI_DBG("MAC Address of radio%d is: %pM\n", cls_wifi_hw->radio_idx, config->mac_addr);

	/* Release the configuration file */
	release_firmware(config_fw);

	return 0;
}

int cls_wifi_load_irf_configfile(struct cls_wifi_hw *cls_wifi_hw, bool full_path, char *src_fname, u32 offset,
						u32 len, u32 mem_typ, bool cfr_data_flag)
{
	struct file *filp = NULL;
	char tmp_str[9] = {0};
	u32 i;
	char delimiter;
	unsigned long buffer;
	u32 *buf;
	ssize_t read_size;
	char full_name[FS_PATH_MAX_LEN];

	if (full_path) {
		filp = filp_open(src_fname, O_RDONLY, 0);
	} else {
		if ((strlen(cls_wifi_hw->plat->path_info.irf_path) + strlen(src_fname)) >
				FS_PATH_MAX_LEN)
		{
			pr_warn("file name or file path overlong\n");
			return -1;
		}
		strcpy(full_name, cls_wifi_hw->plat->path_info.irf_path);
		strcat(full_name, src_fname);
		pr_info("open target file : %s\n", full_name);
		filp = filp_open(full_name, O_RDONLY, 0);
	}

	if (IS_ERR(filp)) {
		pr_warn("open target file fail: %s\n", full_path? src_fname : full_name);
		return -1;
	}

	buf = kzalloc(len * sizeof(u32), GFP_KERNEL);
	if (!buf) {
		filp_close(filp, current->files);
		return -1;
	}

	for (i = 0; i < len; i++) {
		read_size = kernel_read(filp, tmp_str, sizeof(tmp_str) - 1, &filp->f_pos);
		if(read_size <= 0)
			break;
		if (kstrtoul(tmp_str, 16, &buffer))
			break;
		buf[i] = buffer;
		kernel_read(filp, &delimiter, 1, &filp->f_pos);
		if (delimiter == '\r')
			filp->f_pos += 1;
	}

	pr_info("*** loading file %s size: %zu/%u\n", src_fname, i*sizeof(u32), len);

	switch (cls_wifi_hw->plat->hw_params.hw_rev) {
	case CLS_WIFI_HW_DUBHE2000:
		if (mem_typ == IRF_RESV_MEM)
			cls_wifi_hw->ipc_env->ops->irf_writen(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, offset, buf, i * sizeof(u32));
		else
			cls_wifi_hw->ipc_env->ops->irf_snd_smp_writen(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, offset, buf, i * sizeof(u32));
		break;
	case CLS_WIFI_HW_MERAK2000:
	case CLS_WIFI_HW_MERAK3000:
		if (cfr_data_flag) {
			cls_wifi_hw->plat->ep_ops->irf_table_writen(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, offset, buf, i * sizeof(u32));
		} else {
			if (mem_typ == IRF_RESV_MEM)
				cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_WRITE, MM_MEM_REGION_IRAM,
						cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
						cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, offset),
						buf, i * sizeof(u32));

#if defined(CFG_M3K_FPGA)
			else if (mem_typ == IRF_SND_SMP_MEM_FPGA)
				cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_WRITE, MM_MEM_REGION_IRAM, offset,
								 buf, i * sizeof(u32));
#endif
			else
				cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_WRITE, MM_MEM_REGION_IRAM,
						cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
						cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP_PHY, offset),
						buf, i * sizeof(u32));
		}
		break;
	default:
		pr_warn("%s %d hw %d %s\n", __func__, __LINE__,
				cls_wifi_hw->plat->hw_params.hw_rev,
				cls_wifi_hw->plat->hw_params.name);
		break;
	}

	kfree(buf);
	filp_close(filp, current->files);
	return (i * sizeof(u32));
}

int cls_wifi_load_irf_cfr_data(struct cls_wifi_hw *cls_wifi_hw, bool full_path, int path_type,
		char *src_fname, u32 offset, u32 len, u32 mem_typ, bool cfr_data_flag)
{
	struct file *filp = NULL;
	char tmp_str[9] = {0};
	u32 i;
	char delimiter;
	unsigned long buffer;
	u32 *buf = NULL;
	ssize_t read_size;
	char full_name[FS_PATH_MAX_LEN];

	if (cfr_data_buf == NULL) {
		if (full_path) {
			filp = filp_open(src_fname, O_RDONLY, 0);
		} else {
			if ((strlen(cls_wifi_hw->plat->path_info.irf_path) + strlen(src_fname)) >
					FS_PATH_MAX_LEN) {
				pr_warn("file name or file path overlong\n");
				return -1;
			}
			strcpy(full_name, cls_wifi_hw->plat->path_info.irf_path);
			strcat(full_name, src_fname);
			pr_info("open target file : %s\n", full_name);
			filp = filp_open(full_name, O_RDONLY, 0);
		}

		if (IS_ERR(filp)) {
			pr_warn("open target file fail: %s\n", full_path ? src_fname : full_name);
			return -1;
		}

		buf = kzalloc(len * sizeof(u32), GFP_KERNEL);
		if (!buf) {
			filp_close(filp, current->files);
			return -1;
		}

		for (i = 0; i < len; i++) {
			read_size = kernel_read(filp, tmp_str, sizeof(tmp_str) - 1, &filp->f_pos);
			if (read_size <= 0)
				break;
			if (kstrtoul(tmp_str, 16, &buffer))
				break;
			buf[i] = buffer;
			kernel_read(filp, &delimiter, 1, &filp->f_pos);
			if (delimiter == '\r')
				filp->f_pos += 1;
		}
		filp_close(filp, current->files);
		cfr_data_buf = buf;
		pr_info("*** loading file %s size: %zu/%u\n", src_fname, i*sizeof(u32), len);
	}

	switch (cls_wifi_hw->plat->hw_params.hw_rev) {
	case CLS_WIFI_HW_DUBHE2000:
		if (mem_typ == IRF_RESV_MEM)
			cls_wifi_hw->ipc_env->ops->irf_writen(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, offset, cfr_data_buf, len * sizeof(u32));
		else
			cls_wifi_hw->ipc_env->ops->irf_snd_smp_writen(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, offset, cfr_data_buf, len * sizeof(u32));
		break;
	case CLS_WIFI_HW_MERAK2000:
		if (cfr_data_flag) {
			cls_wifi_hw->plat->ep_ops->irf_table_writen(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, offset, cfr_data_buf, len * sizeof(u32));
		} else {
			if (mem_typ == IRF_RESV_MEM)
				cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_WRITE, MM_MEM_REGION_IRAM,
						cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
						cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, offset),
						cfr_data_buf, len * sizeof(u32));
			else
				cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_WRITE, MM_MEM_REGION_IRAM,
						cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
						cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP_PHY, offset),
						cfr_data_buf, len * sizeof(u32));
		}
		break;
	default:
		pr_warn("%s %d hw %d %s\n", __func__, __LINE__,
				cls_wifi_hw->plat->hw_params.hw_rev,
				cls_wifi_hw->plat->hw_params.name);
		break;
	}
	return (len * sizeof(u32));
}


void cls_wifi_save_irf_configfile(struct cls_wifi_hw *cls_wifi_hw, u32 offset,
		u32 len, u32 data_mux, u32 mem_typ)
{
	struct file *filp = NULL;
	char dst_fname[32];
	char tmp_str[11] = {0};
	u32 i;
	u32 *buf;

	sprintf(dst_fname, "/tmp/irf_%d.dat", data_mux);

	if (NULL == current) {
		pr_err("current is NULL, file not saved!\n");
		return;
	}
	if (NULL == current->fs) {
		pr_err("save file in %s process, please check!\n", current->comm);
		return;
	}

	filp = filp_open(dst_fname, O_RDWR | O_CREAT | O_TRUNC, 0);
	if (IS_ERR(filp)) {
		pr_warn("open target file fail: %s\n", dst_fname);
		return;
	}

	buf = kzalloc(len * sizeof(u32), GFP_KERNEL);
	if (!buf) {
		filp_close(filp, current->files);
		return;
	}

	switch (cls_wifi_hw->plat->hw_params.hw_rev) {
	case CLS_WIFI_HW_DUBHE2000:
		if (mem_typ == IRF_RESV_MEM)
			cls_wifi_hw->ipc_env->ops->irf_readn(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, offset, buf,
					len * sizeof(u32));
		else
			cls_wifi_hw->ipc_env->ops->irf_snd_smp_readn(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, offset, buf,
					len * sizeof(u32));
		break;
	case CLS_WIFI_HW_MERAK2000:
	case CLS_WIFI_HW_MERAK3000:
		if (mem_typ == IRF_RESV_MEM)
			cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_READ, MM_MEM_REGION_IRAM,
					cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, offset),
					buf, len * sizeof(u32));
#if defined(CFG_M3K_FPGA)
		else if (mem_typ == IRF_SND_SMP_MEM_FPGA)
			cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_READ, MM_MEM_REGION_IRAM,
							 offset, buf, len * sizeof(u32));
#endif
		else
			cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_READ, MM_MEM_REGION_IRAM,
					cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP_PHY, offset),
					buf, len * sizeof(u32));
		break;
	default:
		pr_warn("%s %d hw %d %s\n", __func__, __LINE__,
				cls_wifi_hw->plat->hw_params.hw_rev,
				cls_wifi_hw->plat->hw_params.name);
		break;
	}

	for (i = 0; i < len; i++) {
		sprintf(tmp_str, "%08x\r\n", buf[i]);
		kernel_write(filp, tmp_str, strlen(tmp_str), &filp->f_pos);
	}

	pr_warn("*** target file %s pos: %llx\n", dst_fname, filp->f_pos);

	kfree(buf);
	filp_close(filp, current->files);
}

void cls_wifi_save_irf_multi_phase_configfile(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_smp_start_ind *ind)
{
	struct file *filp = NULL;
	char dst_fname[32];
	u8 i;
	u32 len, base_offset;
	u32 offset[IRF_MAX_SMP_DESC_NUM] = {0};
	char tmp_str[11] = {0};
	u32 *buf[IRF_MAX_SMP_DESC_NUM] = {NULL};

	if (ind->sel_bitmap == 0) {
		pr_warn("irf smp select bitmap is null!\n");
		return;
	}

	for (i = 0; i < IRF_MAX_SMP_DESC_NUM; i++) {
		if (CO_BIT(i) & ind->sel_bitmap) {
			sprintf(dst_fname, "/tmp/irf_%d.dat", ind->smp_addr_desc[i].node);
			len = ind->smp_addr_desc[i].len;
			break;
		}
	}

	filp = filp_open(dst_fname, O_RDWR | O_CREAT | O_TRUNC, 0);
	if (IS_ERR(filp)) {
		pr_warn("open target file fail: %s\n", dst_fname);
		return;
	}

	for (i = 0; i < IRF_MAX_SMP_DESC_NUM; i++) {
		if (CO_BIT(i) & ind->sel_bitmap) {
			buf[i] = kzalloc(len * sizeof(u32), GFP_KERNEL);
			if (!buf[i])
				goto err_buf;
			base_offset = ind->smp_addr_desc[i].irf_smp_buf_addr -
					cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
			cls_wifi_hw->ipc_env->ops->irf_readn(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx, base_offset, buf,
					len * sizeof(u32));
		}
	}

	while (len / sizeof(u32)) {
		for (i = 0; i < IRF_MAX_SMP_DESC_NUM; i++) {
			if (CO_BIT(i) & ind->sel_bitmap) {
				sprintf(tmp_str, "%08x\r\n", buf[i][offset[i]]);
				kernel_write(filp, tmp_str, strlen(tmp_str), &filp->f_pos);
				len -= sizeof(u32);
				offset[i]++;
			}
		}
	}

	pr_warn("*** target file %s pos: %llx\n", dst_fname, filp->f_pos);
err_buf:
	for (i = 0; i < IRF_MAX_SMP_DESC_NUM; i++) {
		if (buf[i])
			kfree(buf[i]);
	}
	filp_close(filp, current->files);
}

int cls_wifi_save_irf_binfile(char *name,u32 *buf, u32 len)
{
	struct file *filp = NULL;

	filp = filp_open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (IS_ERR(filp)) {
		printk("open target file fail: %s\n", name);
		return -1;
	}

	kernel_write(filp, buf, len, &filp->f_pos);

	printk("*** target file %s pos: %llx\n", name, filp->f_pos);

	filp_close(filp, current->files);
	return 0;
}

int cls_wifi_save_irf_binfile_mem(struct cls_wifi_hw *cls_wifi_hw, char *name, u32 offset, u32 len)
{
	struct file *filp = NULL;
	u32 *buf;
#ifdef CFG_PCIE_SHM
	pcie_shm_pool_st *shm_obj = cls_wifi_hw->ipc_env->plat->pcie_shm_pools[0];
#endif
	if (NULL == current) {
		pr_err("current is NULL, file not saved!\n");
		return -1;
	}

	if (NULL == current->fs) {
		pr_err("save file in %s process, please check!\n", current->comm);
		return -1;
	}

	filp = filp_open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (IS_ERR(filp)) {
		printk("open target file fail: %s\n", name);
		return -1;
	}

	buf = kzalloc(len, GFP_KERNEL);
	if (!buf) {
		filp_close(filp, NULL);
		return -1;
	}
#ifdef CFG_PCIE_SHM
	pcie_shmem_read_sync(shm_obj, cls_wifi_hw->ipc_env->plat->irf_tbl_virt_addr + offset, len);
	memcpy(buf, cls_wifi_hw->ipc_env->plat->irf_tbl_virt_addr + offset, len);
#else
	cls_wifi_hw->ipc_env->ops->irf_table_readn(cls_wifi_hw->ipc_env->plat,
			cls_wifi_hw->ipc_env->radio_idx, offset, buf,
			len);
#endif
	kernel_write(filp, buf, len, &filp->f_pos);

	printk("*** target file %s pos: %llx\n", name, filp->f_pos);

	kfree(buf);
	filp_close(filp, NULL);
	return 0;
}

int cls_wifi_load_irf_binfile(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		char *name, u32 offset, u32 len, u8 *version, int msg_flag)
{
	struct file *filp = NULL;
	u32 file_size;
	u32 *buf;
	filp = filp_open(name, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		if (msg_flag)
			pr_err("open target file fail: %s\n", name);
		return 0;
	}
	file_size = filp->f_inode->i_size;
	if (file_size > len || file_size < sizeof(struct irf_tbl_head)) {
		if (msg_flag)
			pr_err("*** target file %s size 0x%x invalid\n", name, file_size);
		filp_close(filp, current->files);
		return 0;
	}

	buf = kzalloc(file_size * sizeof(u32), GFP_KERNEL);
	if (!buf) {
		filp_close(filp, current->files);
		return 0;
	}

	kernel_read(filp, buf, file_size, &filp->f_pos);

	if (msg_flag)
		pr_info("*** target file %s pos: %llx\n", name, filp->f_pos);

#ifdef CFG_PCIE_SHM
	memcpy(cls_wifi_plat->irf_tbl_virt_addr + offset, buf, file_size);
#else
	cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, offset, buf, file_size);
#endif
	*version = ((struct irf_comm_tbl *)buf)->head.ver;

	kfree(buf);
	filp_close(filp, current->files);
	return file_size;
}


char *cls_wifi_fgets(char *buf, int buf_size, struct file *filp)
{
	int i;
	ssize_t read_size;

	read_size = kernel_read(filp, buf, buf_size, &filp->f_pos);
	if (read_size < 1)
		return NULL;

	for (i = 0; i < read_size; i++) {
		if (buf[i] == '\n') {
			buf[i++] = '\0';
			break;
		}
	}

	filp->f_pos += (i - read_size);

	return buf;
}

int cls_wifi_get_cal_chip_id(const char *name, u32 id[5])
{
	struct file *filp = NULL;
	char tmp_str[128] = {0};
	int ret = -1;
	char *key_word;
	int line;

	filp = filp_open(name, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		//pr_err("open target file fail: %s\n", name);
		return -1;
	}
	for (line = 0; line < 10; line++) {
		memset(tmp_str, 0, sizeof(tmp_str));
		if (!cls_wifi_fgets(tmp_str, sizeof(tmp_str) - 1, filp)) {
			//pr_err("read %s end, No Chip ID info\n", name);
			break;
		}
		key_word = strstr(tmp_str, "Chip ID:");
		if (key_word) {
			key_word += strlen("Chip ID:");
			if (sscanf(key_word, "%x-%x-%x-%x-%x", &id[0], &id[1], &id[2], &id[3], &id[4]) > 0)
				ret = 0;
			break;
		}
	}
	filp_close(filp, current->files);

	return ret;
}


