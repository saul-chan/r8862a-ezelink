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

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/firmware.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_main.h"
#include "cls_wifi_core.h"
#include "ipc_host.h"
#include "cls_wifi_prof.h"
#ifdef CONFIG_CLS_MSGQ_TEST
#include "cls_wifi_debugfs.h"
#endif
#include "cls_wifi_irf.h"
#include "cls_wifi_pci.h"
#include "cls_wifi_mod_params.h"
#include "cls_wifi_heartbeat.h"
#ifdef CFG_PCIE_SHM
#include "cls_wifi_pci_shm.h"
#endif

/* [0]:word_1 [1]:word_2 [2]:word_3 [3]:word_4 [4]:chip_version */
uint32_t efuse_word[5] = {0xff, 0xff, 0xff, 0xff, 0xff};

static const struct pci_device_id cls_wifi_pci_id_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_CLS, PCI_DEVICE_ID_MERAK2000_EP0) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CLS, PCI_DEVICE_ID_MERAK2000_EP1) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CLS, PCI_DEVICE_ID_MERAK3000) },
	{0}
};

uint32_t is_first_boot;
module_param(is_first_boot, uint, 0644);

/* valid values: 0 - legacy mode; 1 - use 1 irq; 32 - use 32 irqs */
uint32_t msi_irq = 32;
module_param(msi_irq, uint, 0644);

char *fw_path = NULL;
module_param(fw_path, charp, 0644);

struct cls_wifi_pci_vaddr {
	void __iomem *cls_wifi_pci_sys_mem_vaddr;
	void __iomem *cls_wifi_pci_msgq_vaddr;
	void __iomem *cls_wifi_pci_shared_mem_vaddr;
	void __iomem *cls_wifi_pci_ipc_vaddr;
};

struct cls_wifi_pci_ops {
	int (*wakeup)(struct cls_wifi_plat *cls_wifi_plat);
	void (*release)(struct cls_wifi_plat *cls_wifi_plat);
	int (*get_msi_irq)(struct cls_wifi_plat *cls_wifi_plat, unsigned int vector);
	void (*window_write32)(struct cls_wifi_plat *cls_wifi_plat, u32 offset, u32 value);
	u32 (*window_read32)(struct cls_wifi_plat *cls_wifi_plat, u32 offset);
};

struct cls_wifi_pci {
	struct pci_dev *pci_dev;
	struct cls_wifi_pci_vaddr pci_vaddr;
	const struct cls_wifi_pci_ops *pci_ops;
	u16 dev_id;
	u32 msi_irq;
	u8 *legacy_out_address;
};

static struct cls_wifi_pci *cls_wifi_pci_priv(struct cls_wifi_plat *cls_wifi_plat)
{
	return (struct cls_wifi_pci *)cls_wifi_plat->priv;
}

static void cls_wifi_pci_cpu_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		void *dst, u32 length)
{
	memcpy_fromio(dst, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_CPU, offset), length);
}

static void cls_wifi_pci_cpu_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx,
		u32 offset, void *src, u32 length)
{
	if (src)
		memcpy_toio(cls_wifi_plat->if_ops->get_address(cls_wifi_plat,
				radio_idx, CLS_WIFI_ADDR_CPU, offset), src, length);
	else
		memset_io(cls_wifi_plat->if_ops->get_address(cls_wifi_plat,
				radio_idx, CLS_WIFI_ADDR_CPU, offset), 0, length);
}

#define CLS_WIFI_CRC32C_INIT		0
#define CLS_WIFI_CRC_TABLE_SIZE		256
#define CLS_WIFI_CRC_POLY_REV		0xEDB88320

static uint32_t crc32c_table[CLS_WIFI_CRC_TABLE_SIZE];

static void cls_wifi_crc32_table_gen(uint32_t crc_table[], uint32_t poly)
{
	uint32_t x;
	size_t i;
	size_t j;

	for (i = 0; i < CLS_WIFI_CRC_TABLE_SIZE; i++) {
		x = i;

		for (j = 0; j < 8; j++)
			x = (x >> 1) ^ (poly & (-(int32_t)(x & 1)));

		crc_table[i] = x;
	}
}

uint32_t cls_wifi_crc32c(uint32_t crc, const uint8_t *buf, size_t len)
{
	const uint32_t crc32c_poly_rev = CLS_WIFI_CRC_POLY_REV;

	if (unlikely(!crc32c_table[1]))
		cls_wifi_crc32_table_gen(crc32c_table, crc32c_poly_rev);

	while (len--)
		crc = (crc << 8) ^ crc32c_table[(crc >> 24) ^ *buf++];

	return crc;
}

#define CLS_WIFI_LOCAL_SYS_ENABLE_OFFSET	0x4

#define CLS_WIFI_LOCAL_SYS_EP_STATE_OFFSET	0x8
#define CLS_WIFI_LOCAL_SYS_EP_STATE_EP0		1
#define CLS_WIFI_LOCAL_SYS_EP_STATE_EP1		2
#define CLS_WIFI_LOCAL_SYS_EP_STATE_DONE	4

#define CLS_WIFI_LOCAL_SYS_TSENSOR_OFFSET	0xc

#define CLS_WIFI_LOCAL_SYS_RESET_OFFSET		0x14
#define CLS_WIFI_LOCAL_SYS_RESET_VALUE_ENABLE	0x1
#define CLS_WIFI_LOCAL_SYS_RESET_VALUE_DISABLE	0x0

static u32 cls_wifi_pci_get_mem_size(struct cls_wifi_plat *cls_wifi_plat)
{
	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		return CLS_WIFI_MERAK2000_MEM_SIZE_C;
	else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		return CLS_WIFI_MERAK3000_MEM_SIZE_C;

	return 0;
}

static u32 cls_wifi_pci_get_irf_tb_phy_off(struct cls_wifi_plat *cls_wifi_plat)
{
	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		return CLS_WIFI_MERAK2000_IRF_TB_PHY_OFFSET;
	else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		return CLS_WIFI_MERAK3000_IRF_TB_PHY_OFFSET;

	return 0;
}

static u32 cls_wifi_pci_local_sys_read32(struct cls_wifi_plat *cls_wifi_plat,
		u32 offset)
{
	return ioread32(cls_wifi_plat->if_ops->get_address(cls_wifi_plat,
			CLS_WIFI_INDEX_C, CLS_WIFI_ADDR_LSYS, offset));
}

static void cls_wifi_pci_local_sys_write32(struct cls_wifi_plat *cls_wifi_plat,
		u32 offset, u32 value)
{
	iowrite32(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat,
			CLS_WIFI_INDEX_C, CLS_WIFI_ADDR_LSYS, offset));
}

static int cls_wifi_pci_tsensor_get(struct cls_wifi_plat *cls_wifi_plat)
{
	return (int)cls_wifi_pci_local_sys_read32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_TSENSOR_OFFSET);
}

static void cls_wifi_pci_firmware_sleep(void)
{
	msleep(20);
}

#define CLS_WIFI_PCI_FIRMWARE_RETRY 60
static bool cls_wifi_pci_firmware_wait_init(struct cls_wifi_plat *cls_wifi_plat)
{
	struct cls_wifi_fw_param fw_param;
	struct device *dev = cls_wifi_plat->dev;
	int retry = CLS_WIFI_PCI_FIRMWARE_RETRY;

	while (retry) {
		cls_wifi_plat->ep_ops->readn(cls_wifi_plat, CLS_WIFI_INDEX_C,
				CLS_WIFI_MERAK2000_MEM_SIZE_C - sizeof(struct cls_wifi_fw_param),
				&fw_param, sizeof(struct cls_wifi_fw_param));

		if (fw_param.state == CLS_WIFI_FW_STATE_INIT)
			return true;

		dev_err(dev, "%s:%d state 0x%x retry %d.\n", __func__, __LINE__, fw_param.state, retry);
		cls_wifi_pci_firmware_sleep();
		retry--;
	}
	dev_err(dev, "%s:%d timeout.\n", __func__, __LINE__);
	return false;
}

static uint32_t cls_wifi_pci_firmware_wait_success(struct cls_wifi_plat *cls_wifi_plat)
{
	struct cls_wifi_fw_param fw_param;
	struct device *dev = cls_wifi_plat->dev;
	int retry = CLS_WIFI_PCI_FIRMWARE_RETRY;
	u32 mem_size = cls_wifi_pci_get_mem_size(cls_wifi_plat);

	while (retry) {
		cls_wifi_plat->ep_ops->readn(cls_wifi_plat, CLS_WIFI_INDEX_C,
				mem_size - sizeof(struct cls_wifi_fw_param),
				&fw_param, sizeof(struct cls_wifi_fw_param));

		switch (fw_param.state) {
		case CLS_WIFI_FW_STATE_FAILED:
		case CLS_WIFI_FW_STATE_FAILED_HEADER:
		case CLS_WIFI_FW_STATE_INIT:
			dev_err(dev, "%s:%d state 0x%x.\n", __func__, __LINE__, fw_param.state);
			fallthrough;
		case CLS_WIFI_FW_STATE_SUCCESS:
			return fw_param.state;
		default:
			break;
		}

		dev_err(dev, "%s:%d state 0x%x retry %d.\n", __func__, __LINE__, fw_param.state, retry);
		cls_wifi_pci_firmware_sleep();
		retry--;
	}
	dev_err(dev, "%s:%d timeout.\n", __func__, __LINE__);
	return fw_param.state;
}

static void cls_wifi_pci_firmware_param_set(struct cls_wifi_plat *cls_wifi_plat,
		struct cls_wifi_fw_param *fw_param)
{
	u32 mem_size = cls_wifi_pci_get_mem_size(cls_wifi_plat);

	// use the last part of shared memory
	cls_wifi_plat->ep_ops->writen(cls_wifi_plat, CLS_WIFI_INDEX_C,
			mem_size - sizeof(struct cls_wifi_fw_param),
			fw_param, sizeof(struct cls_wifi_fw_param));
}

static uint32_t cls_wifi_pci_efuse_word_wait_success(struct cls_wifi_plat *cls_wifi_plat)
{
	struct cls_wifi_efuse_word_read efuse_param;
	struct device *dev = cls_wifi_plat->dev;
	int retry = CLS_WIFI_PCI_FIRMWARE_RETRY;

	while (retry) {
		cls_wifi_plat->ep_ops->readn(cls_wifi_plat, CLS_WIFI_INDEX_C,
				CLS_WIFI_MERAK2000_MEM_SIZE_C - sizeof(struct cls_wifi_efuse_word_read) -
				sizeof(struct cls_wifi_fw_param),
				&efuse_param, sizeof(struct cls_wifi_efuse_word_read));

		switch (efuse_param.state) {
		case CLS_EFUSE_WORD_UNREADY:
			break;
		case CLS_EFUSE_WORD_READY:
			efuse_word[0] = efuse_param.word.word_1;
			efuse_word[1] = efuse_param.word.word_2;
			efuse_word[2] = efuse_param.word.word_3;
			efuse_word[3] = efuse_param.word.word_4;
			efuse_word[4] = efuse_param.word.chip_version;

			return efuse_param.state;
		default:
			break;
		}

		dev_err(dev, "%s:%d state 0x%x retry %d.\n", __func__, __LINE__, efuse_param.state, retry);
		cls_wifi_pci_firmware_sleep();
		retry--;
	}
	dev_err(dev, "%s:%d timeout.\n", __func__, __LINE__);
	return efuse_param.state;
}

static void cls_wifi_pci_efuse_param_set(struct cls_wifi_plat *cls_wifi_plat,
		struct cls_wifi_efuse_word_read *efuse_param)
{
	// use the last part of shared memory
	cls_wifi_plat->ep_ops->writen(cls_wifi_plat, CLS_WIFI_INDEX_C,
			CLS_WIFI_MERAK2000_MEM_SIZE_C - sizeof(struct cls_wifi_efuse_word_read) -
			sizeof(struct cls_wifi_fw_param),
			efuse_param, sizeof(struct cls_wifi_efuse_word_read));
}

static int request_custom_firmware(struct firmware **fw, const char *path)
{
	struct file *fp;
	loff_t size;
	char *buf;
	int ret;

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp))
		return PTR_ERR(fp);

	size = i_size_read(file_inode(fp));
	buf = vmalloc(size);
	if (!buf) {
		ret = -ENOMEM;
		goto out;
	}

	ret = kernel_read(fp, buf, size, &fp->f_pos);
	if (ret != size) {
		ret = -EIO;
		goto free_buf;
	}

	*fw = kmalloc(sizeof(**fw), GFP_KERNEL);
	if (*fw == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	(*fw)->size = size;
	(*fw)->data = buf;

	ret = 0;
	goto out;

free_buf:
	vfree(buf);
out:
	filp_close(fp, NULL);
	return ret;
}

static int cls_wifi_pci_firemware_load(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		const char *filename, bool final)
{
	struct cls_wifi_pci *cls_pci = cls_wifi_pci_priv(cls_wifi_plat);
	struct device *dev = cls_wifi_plat->dev;
	const struct firmware *fw;
	struct firmware *c_fw;
	struct cls_wifi_fw_header *fw_header;
	struct cls_wifi_fw_param fw_param;
	resource_size_t pci_res_len;
	uint32_t crc;
	int err;
	char custom_file[256];

	if (fw_path == NULL) {
		dev_err(dev, "Load firmware %s for radio 0x%x\n", filename, radio_index);
		err = request_firmware(&fw, filename, dev);
		if (err) {
			dev_err(dev, "Error %d, %s:%d\n", err, __func__, __LINE__);
			return err;
		}
	} else {
		if (strlen(fw_path) + strlen(filename) > 256) {
			dev_err(dev, "Custom firmware path %s for radio 0x%x too long\n", fw_path, radio_index);
			return -1;
		}

		sprintf(custom_file, "%s/%s", fw_path, filename);
		dev_err(dev, "Load Custom firmware %s for radio 0x%x\n", custom_file, radio_index);
		err = request_custom_firmware(&c_fw, custom_file);
		if (err) {
			dev_err(dev, "Error %d, %s:%d\n", err, __func__, __LINE__);
			return err;
		}

		fw = c_fw;
	}

	pci_res_len = pci_resource_len(cls_pci->pci_dev, CLS_WIFI_BAR_IDX_SYS);
	if (fw->size > pci_res_len) {
		dev_err(dev, "File %s length %zu is larger then pci bar %d size %pa.\n",
				filename, fw->size, CLS_WIFI_BAR_IDX_SYS,
				&pci_res_len);
		goto return_release_firmware;
	}

	fw_header = (struct cls_wifi_fw_header *)fw->data;
	if (fw_header->magic != CLS_WIFI_FW_MAGIC) {
		dev_err(dev, "Error %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

	if (fw->size != fw_header->fw_len + sizeof(struct cls_wifi_fw_header)) {
		dev_err(dev, "Error %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

	if (radio_index != fw_header->radio) {
		dev_err(dev, "Error %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

	crc = cls_wifi_crc32c(CLS_WIFI_CRC32C_INIT, (uint8_t *)fw_header,
			offsetof(struct cls_wifi_fw_header, hdr_crc));
	if (crc != fw_header->hdr_crc) {
		dev_err(dev, "CRC error, crc 0x%x header crc 0x%x.\n", crc, fw_header->hdr_crc);
		goto return_release_firmware;
	}

	crc = cls_wifi_crc32c(CLS_WIFI_CRC32C_INIT, (uint8_t *)fw->data + sizeof(struct cls_wifi_fw_header),
			fw_header->fw_len);
	if (crc != fw_header->crc) {
		dev_err(dev, "CRC error, crc 0x%x file crc 0x%x.\n", crc, fw_header->crc);
		goto return_release_firmware;
	}

	if (!cls_wifi_pci_firmware_wait_init(cls_wifi_plat)) {
		dev_err(dev, "Timeout %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

retry:
	memcpy(&fw_param.fw_header, fw_header, sizeof(struct cls_wifi_fw_header));
	cls_wifi_pci_cpu_writen(cls_wifi_plat, radio_index, 0,
			(u8 *)fw->data + sizeof(struct cls_wifi_fw_header), fw_header->fw_len);

	//todo: dma addr
	fw_param.read_addr = 0;
	fw_param.state = CLS_WIFI_FW_STATE_DONE;
	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);

	switch (cls_wifi_pci_firmware_wait_success(cls_wifi_plat)) {
	case CLS_WIFI_FW_STATE_INIT:
		dev_err(dev, "Init state again, retry, %s:%d\n", __func__, __LINE__);
		goto retry;
	case CLS_WIFI_FW_STATE_SUCCESS:
		break;
	case CLS_WIFI_FW_STATE_FAILED:
	case CLS_WIFI_FW_STATE_FAILED_HEADER:
	default:
		goto return_release_firmware;
	}

	if (final)
		fw_param.state = CLS_WIFI_FW_STATE_DONE_FINAL;
	else
		fw_param.state = CLS_WIFI_FW_STATE_SUCCESS_ACK;
	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);

	if (fw_path != NULL) {

		dev_err(dev, "Load Custom firmware %s for radio 0x%x succeed.\n", custom_file, radio_index);
		kfree(fw);
	} else {
		dev_err(dev, "Load firmware %s for radio 0x%x succeed.\n", filename, radio_index);
		release_firmware(fw);
	}
	return 0;

return_release_firmware:
	if (fw_path != NULL) {
		dev_err(dev, "Load Custom firmware %s for radio 0x%x failed.\n", custom_file, radio_index);
		kfree(fw);
	} else {
		dev_err(dev, "Load firmware %s for radio 0x%x failed.\n", filename, radio_index);
		release_firmware(fw);
	}
	return -1;
}

static int cls_wifi_pci_firemware_load_block(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		const char *filename, bool final)
{
	struct device *dev = cls_wifi_plat->dev;
	const struct firmware *fw;
	struct firmware *c_fw;
	struct cls_wifi_fw_header *fw_header;
	struct cls_wifi_fw_param fw_param;
	struct cls_wifi_fw_section fw_section;
	uint32_t crc;
	int err;
	uint32_t section_idx;
	uint32_t section_offset;
	uint32_t block_size_left;
	uint32_t i;
	char custom_file[256];

	if (fw_path == NULL) {
		dev_err(dev, "Load firmware in block mode: %s for radio 0x%x\n", filename, radio_index);
		err = request_firmware(&fw, filename, dev);
		if (err) {
			dev_err(dev, "Error %d, %s:%d\n", err, __func__, __LINE__);
			return err;
		}
	} else {
		if (strlen(fw_path) + strlen(filename) > 256) {
			dev_err(dev, "Custom firmware path %s for radio 0x%x too long\n", fw_path, radio_index);
			return -1;
		}

		sprintf(custom_file, "%s/%s", fw_path, filename);
		dev_err(dev, "Load Custom firmware %s for radio 0x%x\n", custom_file, radio_index);
		err = request_custom_firmware(&c_fw, custom_file);
		if (err) {
			dev_err(dev, "Error %d, %s:%d\n", err, __func__, __LINE__);
			return err;
		}
		fw = c_fw;
	}

	fw_header = (struct cls_wifi_fw_header *)fw->data;
	if (fw_header->magic != CLS_WIFI_FW_MAGIC) {
		dev_err(dev, "Error %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

	if (fw->size != fw_header->fw_len + sizeof(struct cls_wifi_fw_header) +
			fw_header->section_nr * sizeof(struct cls_wifi_fw_section)) {
		dev_err(dev, "Error %s:%d 0x%zx 0x%x 0x%zx %d 0x%zx\n", __func__, __LINE__,
				fw->size, fw_header->fw_len, sizeof(struct cls_wifi_fw_header),
				fw_header->section_nr, sizeof(struct cls_wifi_fw_section));
		goto return_release_firmware;
	}

	if (radio_index != fw_header->radio) {
		dev_err(dev, "Error %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

	crc = cls_wifi_crc32c(CLS_WIFI_CRC32C_INIT, (uint8_t *)fw_header,
			offsetof(struct cls_wifi_fw_header, hdr_crc));
	if (crc != fw_header->hdr_crc) {
		dev_err(dev, "CRC error, crc 0x%x header crc 0x%x.\n", crc, fw_header->hdr_crc);
		goto return_release_firmware;
	}

	crc = cls_wifi_crc32c(CLS_WIFI_CRC32C_INIT, (uint8_t *)fw->data +
			sizeof(struct cls_wifi_fw_header) +
			fw_header->section_nr * sizeof(struct cls_wifi_fw_section),
			fw_header->fw_len);
	if (crc != fw_header->crc) {
		dev_err(dev, "CRC error, crc 0x%x file crc 0x%x.\n", crc, fw_header->crc);
		goto return_release_firmware;
	}

	if (!cls_wifi_pci_firmware_wait_init(cls_wifi_plat)) {
		dev_err(dev, "Timeout %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

retry:
	memcpy(&fw_param.fw_header, fw_header, sizeof(struct cls_wifi_fw_header));

	for (section_idx = 0; section_idx < fw_header->section_nr; section_idx++) {
		memcpy(&fw_section, (uint8_t *)fw->data + sizeof(struct cls_wifi_fw_header) +
				section_idx * sizeof(fw_section), sizeof(fw_section));
		if (fw_section.size & 3) {
			dev_err(dev, "%s:%d section %d size 0x%x is not 4 aligned.\n",
				__func__, __LINE__, section_idx, fw_section.size);
			goto return_release_firmware;
		}
		fw_param.block_addr = fw_section.addr;
		section_offset = fw_section.offset;
		block_size_left = fw_section.size;
		while (block_size_left) {
			fw_param.read_addr = 0;
			if (block_size_left > CLS_WIFI_FW_BLOCK_SIZE) {
				fw_param.block_size = CLS_WIFI_FW_BLOCK_SIZE;
				fw_param.state = CLS_WIFI_FW_STATE_DONE_BLOCK;
			} else {
				fw_param.block_size = block_size_left;
				if (section_idx == fw_header->section_nr - 1)
					fw_param.state = CLS_WIFI_FW_STATE_DONE_FILE;
				else
					fw_param.state = CLS_WIFI_FW_STATE_DONE_SECTION;
			}

			cls_wifi_plat->ep_ops->writen(cls_wifi_plat, CLS_WIFI_INDEX_C,
					0, (uint8_t *)fw->data + section_offset, fw_param.block_size);

			cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);
			switch (cls_wifi_pci_firmware_wait_success(cls_wifi_plat)) {
			case CLS_WIFI_FW_STATE_INIT:
				dev_err(dev, "Init state again, retry, %s:%d\n", __func__, __LINE__);
				goto retry;
			case CLS_WIFI_FW_STATE_SUCCESS:
				fw_param.block_addr += fw_param.block_size;
				section_offset += fw_param.block_size;
				block_size_left -= fw_param.block_size;
				break;
			case CLS_WIFI_FW_STATE_FAILED:
			case CLS_WIFI_FW_STATE_FAILED_HEADER:
			default:
				goto return_release_firmware;
			}
		}
	}

	if (final) {
		fw_param.state = CLS_WIFI_FW_STATE_DONE_FINAL;
		// shared memory was used in firmware loading, so set pattern at the end
		for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
			if (!(bands_enable & cls_wifi_plat->hw_params.band_cap[i]))
				continue;
			ipc_host_ipc_pattern_set(cls_wifi_plat, i, IPC_PATTERN_INIT_MAGIC);
			ipc_host_wpu_ipc_pattern_set(cls_wifi_plat, i, 0);
		}
	} else
		fw_param.state = CLS_WIFI_FW_STATE_SUCCESS_ACK;
	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);

	if (fw_path != NULL) {
		dev_err(dev, "Load Custom firmware %s for radio 0x%x succeed.\n", custom_file, radio_index);
		kfree(fw);
	} else {
		dev_err(dev, "Load firmware %s for radio 0x%x succeed.\n", filename, radio_index);
		release_firmware(fw);
	}
	return 0;

return_release_firmware:
	if (fw_path != NULL) {
		dev_err(dev, "Load Custom firmware %s for radio 0x%x failed.\n", custom_file, radio_index);
		kfree(fw);
	} else {
		dev_err(dev, "Load firmware %s for radio 0x%x failed.\n", filename, radio_index);
		release_firmware(fw);
	}
	return -1;
}

#ifdef CFG_PCIE_SHM
static int cls_wifi_pci_cmn_trans_msg(
		struct cls_wifi_plat *cls_wifi_plat,
		int type,
		void *msg,
		uint32_t msg_size)
{
	struct cls_wifi_pci *cls_pci = cls_wifi_pci_priv(cls_wifi_plat);
	struct device *dev = cls_wifi_plat->dev;
	struct cls_wifi_fw_param fw_param;

	dev_err(dev, "Trans MSG to CMN WPU: type(%d) size(%d)\n", type, msg_size);

	if (msg_size > pci_resource_len(cls_pci->pci_dev, CLS_WIFI_BAR_IDX_MEM)) {
		dev_err(dev, "msg length %u is larger then pci bar %d size %llu.\n",
				msg_size, CLS_WIFI_BAR_IDX_MEM,
				pci_resource_len(cls_pci->pci_dev, CLS_WIFI_BAR_IDX_MEM));
		goto return_release_firmware;
	}

	if (!cls_wifi_pci_firmware_wait_init(cls_wifi_plat)) {
		dev_err(dev, "Timeout %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

retry:

	fw_param.fw_header.magic = CLS_WIFI_FW_MAGIC;
	fw_param.fw_header.fw_len = msg_size;
	fw_param.fw_header.radio =  0;
	fw_param.read_addr = 0;
	fw_param.state = type;
	dev_err(dev, "%s %d type 0x%x size 0x%u\n", __func__, __LINE__, type, msg_size);

	cls_wifi_plat->ep_ops->writen(cls_wifi_plat, CLS_WIFI_INDEX_C,
			0, (u8 *)msg, msg_size);

	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);
	switch (cls_wifi_pci_firmware_wait_success(cls_wifi_plat)) {
	case CLS_WIFI_FW_STATE_INIT:
		dev_err(dev, "Init state again, retry, %s:%d\n", __func__, __LINE__);
		goto retry;
	case CLS_WIFI_FW_STATE_SUCCESS:
		break;
	case CLS_WIFI_FW_STATE_FAILED:
	case CLS_WIFI_FW_STATE_FAILED_HEADER:
	default:
		goto return_release_firmware;
	}

	fw_param.state = CLS_WIFI_FW_STATE_SUCCESS_ACK;

	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);

	dev_err(dev, "Trans MSG to CMN WPU: type(%d) size(%d) succeed.\n", type, msg_size);

	return 0;

return_release_firmware:
	return -1;
}

static int cls_wifi_pci_shm_pools_create(struct cls_wifi_plat *cls_wifi_plat)
{
	int ret = 0;
	pcie_shm_pool_idx_em pool_type;
	pcie_shm_pool_st *shm_obj = NULL;
	struct device *dev = cls_wifi_plat->dev;
	uint64_t phys_addr;
	uint32_t size;

	for (pool_type = 0; pool_type < SHM_POOL_IDX_MAX; pool_type ++ ) {

		struct pcie_shm_msg_create msg;

		shm_obj = pcie_shm_pool_create(cls_wifi_plat, pool_type);
		if (IS_ERR(shm_obj)) {
			dev_err(dev, "Error %ld, %s:%d\n", PTR_ERR(shm_obj), __func__, __LINE__);
			return PTR_ERR(shm_obj);
		}

		phys_addr = pcie_shm_get_pool_phyinfos(shm_obj, &size);
		msg = (struct pcie_shm_msg_create) {
			.type = pool_type,
			.host_addr = phys_addr,
			.size = size
		};

		ret = cls_wifi_pci_cmn_trans_msg(cls_wifi_plat,
				CLS_WIFI_FW_STATE_CREATE_PCIE_SHM,
				&msg,
				sizeof(msg));
		if (ret) {
			dev_err(dev, "Error %d, %s:%d\n", ret, __func__, __LINE__);
			return ret;
		}
	}

	return ret;
}
#endif

static int cls_wifi_pci_agc_load(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		const char *filename, bool final)
{
	struct cls_wifi_pci *cls_pci = cls_wifi_pci_priv(cls_wifi_plat);
	struct device *dev = cls_wifi_plat->dev;
	const struct firmware *fw;
	resource_size_t pci_res_len;
	struct cls_wifi_fw_param fw_param;
	int err;

	dev_err(dev, "Load agc: %s for radio 0x%x\n", filename, radio_index);
	err = request_firmware(&fw, filename, dev);
	if (err) {
		dev_err(dev, "Error %d, %s:%d\n", err, __func__, __LINE__);
		return err;
	}

	pci_res_len = pci_resource_len(cls_pci->pci_dev, CLS_WIFI_BAR_IDX_MEM);
	if (fw->size > pci_res_len) {
		dev_err(dev, "File %s length %zu is larger then pci bar %d size %pa.\n",
				filename, fw->size, CLS_WIFI_BAR_IDX_MEM,
				&pci_res_len);
		goto return_release_firmware;
	}

	if (!cls_wifi_pci_firmware_wait_init(cls_wifi_plat)) {
		dev_err(dev, "Timeout %s:%d\n", __func__, __LINE__);
		goto return_release_firmware;
	}

retry:
	//todo: dma addr
	fw_param.fw_header.magic = CLS_WIFI_FW_MAGIC;
	fw_param.fw_header.fw_len = fw->size;
	fw_param.fw_header.radio = radio_index;
	fw_param.read_addr = 0;
	fw_param.state = CLS_WIFI_FW_STATE_DONE_AGC;
	dev_err(dev, "%s %d radio 0x%x size 0x%zx\n", __func__, __LINE__, radio_index, fw->size);

	cls_wifi_plat->ep_ops->writen(cls_wifi_plat, CLS_WIFI_INDEX_C,
			0, (u8 *)fw->data, fw->size);

	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);
	switch (cls_wifi_pci_firmware_wait_success(cls_wifi_plat)) {
	case CLS_WIFI_FW_STATE_INIT:
		dev_err(dev, "Init state again, retry, %s:%d\n", __func__, __LINE__);
		goto retry;
	case CLS_WIFI_FW_STATE_SUCCESS:
		break;
	case CLS_WIFI_FW_STATE_FAILED:
	case CLS_WIFI_FW_STATE_FAILED_HEADER:
	default:
		goto return_release_firmware;
	}

	if (final)
		fw_param.state = CLS_WIFI_FW_STATE_DONE_FINAL;
	else
		fw_param.state = CLS_WIFI_FW_STATE_SUCCESS_ACK;
	cls_wifi_pci_firmware_param_set(cls_wifi_plat, &fw_param);
	dev_err(dev, "Load firmware %s for radio 0x%x succeed.\n", filename, radio_index);
	release_firmware(fw);
	return 0;

return_release_firmware:
	release_firmware(fw);
	return -1;
}

/**
 * cls_wifi_pci_firmware_on() - Load FW code and on
 *
 * @cls_wifi_plat: platform data
 * @firmware: Name of the fw.
 */
static int cls_wifi_pci_firmware_on(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
				    bool final)
{
	struct device *dev = cls_wifi_plat->dev;
	const char *firmware;
	int ret = 0;
	int debug_mode = cls_wifi_mod_params.debug_mode;

	// warm reset
	if (!is_first_boot && radio_index == CLS_WIFI_INDEX_C) {
		dev_err(dev, "Reset cmn cpu\n");
		cls_wifi_pci_local_sys_write32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_ENABLE_OFFSET,
				CLS_WIFI_LOCAL_SYS_ENABLE_VALUE_ENABLE);
		cls_wifi_pci_local_sys_write32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_RESET_OFFSET,
				CLS_WIFI_LOCAL_SYS_RESET_VALUE_DISABLE);
		cls_wifi_pci_local_sys_read32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_RESET_OFFSET);
		cls_wifi_pci_local_sys_write32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_RESET_OFFSET,
				CLS_WIFI_LOCAL_SYS_RESET_VALUE_ENABLE);
	}

	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000 &&
	    radio_index == CLS_WIFI_INDEX_C) {
		if (debug_mode)
			firmware = CLS_WIFI_MERAK2000_CALI_FW_PATH CLS_WIFI_MERAK2000_FWNAME_C;
		else
			firmware = CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_MERAK2000_FWNAME_C;
		ret = cls_wifi_pci_firemware_load(cls_wifi_plat, radio_index, firmware, final);
	} else if (cls_wifi_plat->hw_rev ==  CLS_WIFI_HW_MERAK3000 &&
		   radio_index == CLS_WIFI_INDEX_C) {
		if (debug_mode)
			firmware = CLS_WIFI_MERAK3000_CALI_FW_PATH CLS_WIFI_MERAK3000_FWNAME_C;
		else
			firmware = CLS_WIFI_MERAK3000_FIRMARE_PATH CLS_WIFI_MERAK3000_FWNAME_C;
		ret = cls_wifi_pci_firemware_load(cls_wifi_plat, radio_index, firmware, final);
#ifdef CFG_PCIE_SHM
		if (ret) {
			dev_err(dev, "Error %d, %s:%d\n", ret, __func__, __LINE__);
			return ret;
		}

		ret = cls_wifi_pci_shm_pools_create(cls_wifi_plat);
		if (ret)
			return ret;
#endif
	} else {
		firmware = cls_wifi_plat->hw_params.agc_file_name[radio_index];
		ret = cls_wifi_pci_agc_load(cls_wifi_plat, radio_index, firmware, 0);
		if (ret) {
			dev_err(dev, "Error %d, %s:%d\n", ret, __func__, __LINE__);
			return ret;
		}

		if (debug_mode)
			firmware = cls_wifi_plat->hw_params.cali_firmware_file_name[radio_index];
		else
			firmware = cls_wifi_plat->hw_params.firmware_file_name[radio_index];
		ret = cls_wifi_pci_firemware_load_block(cls_wifi_plat, radio_index,
							firmware, final);
	}
	if (ret) {
		dev_err(dev, "Error %d, %s:%d\n", ret, __func__, __LINE__);
		return ret;
	}
	return ret;
}

void (*g_task_cb[CLS_WIFI_MAX_IRQS])(struct cls_wifi_hw *cls_wifi_hw) = {
	ipc_host_dbg_handler,
	ipc_host_msg_handler,
	ipc_host_msgack_handler,
	ipc_host_rxdesc_handler,
	ipc_host_radar_handler,
	ipc_host_unsup_rx_vec_handler,
	ipc_host_he_mu_map_handler,
	ipc_host_tx_cfm_handler,
	ipc_host_tx_cfm_handler,
	ipc_host_tx_cfm_handler,
	ipc_host_tx_cfm_handler,
	ipc_host_tx_cfm_handler,
	ipc_host_csi_handler,
	ipc_host_atf_stats_handler,
};

/**
 * cls_wifi_pci_msi_irq_hdlr - IRQ handler
 */
irqreturn_t cls_wifi_pci_msi_irq_hdlr(int irq, void *arg)
{
	struct cls_wifi_irq_task *irq_task = (struct cls_wifi_irq_task *)arg;

	disable_irq_nosync(irq);
	tasklet_schedule(&irq_task->task);
	return IRQ_HANDLED;
}

/**
 * cls_wifi_pci_msi_task - Bottom half for IRQ handler
 */
void cls_wifi_pci_msi_task(unsigned long data)
{
	struct cls_wifi_irq_task *irq_task = (struct cls_wifi_irq_task *)data;
	struct cls_wifi_hw *cls_wifi_hw = irq_task->cls_wifi_hw;

	if (irq_task->cb)
		irq_task->cb(cls_wifi_hw);

	spin_lock(&cls_wifi_hw->tx_lock);
	barrier();
	cls_wifi_hwq_process_all(cls_wifi_hw);
	barrier();
	spin_unlock(&cls_wifi_hw->tx_lock);
	//pr_warn("%s %d irq %d\n", __func__, __LINE__, irq_task->irq);
	enable_irq(irq_task->irq);
}

inline bool cls_wifi_is_single_irq(struct cls_wifi_plat *cls_wifi_plat)
{
	return (cls_wifi_pci_priv(cls_wifi_plat)->msi_irq <= 1);
}

#define CLS_WIFI_PCI_LEGACY_OUT_VALUE	0
/**
 * cls_wifi_pci_legacy_irq_hdlr - IRQ handler
 */
irqreturn_t cls_wifi_pci_legacy_irq_hdlr(int irq, void *arg)
{
	struct cls_wifi_irq_task *irq_task = (struct cls_wifi_irq_task *)arg;
	struct cls_wifi_plat *cls_wifi_plat = irq_task->cls_wifi_hw->plat;
	struct cls_wifi_pci *cls_pci = cls_wifi_pci_priv(cls_wifi_plat);

	if (!cls_pci->msi_irq)
		iowrite32(CLS_WIFI_PCI_LEGACY_OUT_VALUE, cls_pci->legacy_out_address);
	disable_irq_nosync(irq);
	tasklet_schedule(&irq_task->task);
	return IRQ_HANDLED;
}

/**
 * cls_wifi_pci_legacy_task - Bottom half for IRQ handler
 */
void cls_wifi_pci_legacy_task(unsigned long data)
{
	struct cls_wifi_irq_task *irq_task = (struct cls_wifi_irq_task *)data;
	struct cls_wifi_hw *cls_wifi_hw = irq_task->cls_wifi_hw;
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	u32 status;

	REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_CLS_WIFI_IPC_IRQ_HDLR);

	/* Ack unconditionnally in case ipc_host_get_status does not see the irq */
	cls_wifi_plat->if_ops->ack_irq(cls_wifi_plat);

	while ((status = ipc_host_get_status(cls_wifi_hw->ipc_env))) {
		barrier();
		ipc_host_irq(cls_wifi_hw, status);
		barrier();
		cls_wifi_plat->if_ops->ack_irq(cls_wifi_plat);
	}

	spin_lock(&cls_wifi_hw->tx_lock);
	barrier();
	cls_wifi_hwq_process_all(cls_wifi_hw);
	barrier();
	spin_unlock(&cls_wifi_hw->tx_lock);

	enable_irq(irq_task->irq);

	REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_CLS_WIFI_IPC_IRQ_HDLR);
}

static int cls_wifi_pci_plat_enable(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct cls_wifi_pci *cls_pci = cls_wifi_pci_priv(cls_wifi_plat);
	struct cls_wifi_irq_task *irq_task;
	int ret;
	int i;

	if (cls_wifi_is_single_irq(cls_wifi_plat)) {
		irq_task = &cls_wifi_plat->irq_task[cls_wifi_hw->radio_idx][0];
		irq_task->cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[cls_wifi_hw->radio_idx];
		irq_task->irq = pci_irq_vector(cls_pci->pci_dev, 0);
		cls_wifi_plat->hw_params.irq_start[cls_wifi_hw->radio_idx] = 0;
		cls_wifi_plat->hw_params.irq_end[cls_wifi_hw->radio_idx] = 1;

		tasklet_init(&irq_task->task, cls_wifi_pci_legacy_task, (unsigned long)irq_task);
		ret = request_irq(irq_task->irq, cls_wifi_pci_legacy_irq_hdlr, IRQF_SHARED,
				"cls_wifi_pci", irq_task);
		if (ret)
			pr_warn("%s %d ret %d\n", __func__, __LINE__, ret);
		return ret;
	}

	for (i = cls_wifi_plat->hw_params.irq_start[cls_wifi_hw->radio_idx];
			i < cls_wifi_plat->hw_params.irq_end[cls_wifi_hw->radio_idx]; i++) {
		irq_task = &cls_wifi_plat->irq_task[cls_wifi_hw->radio_idx][i];
		irq_task->cls_wifi_hw = cls_wifi_hw;
		irq_task->irq = pci_irq_vector(cls_pci->pci_dev, i);
		irq_task->cb = g_task_cb[i -
				cls_wifi_plat->hw_params.irq_start[cls_wifi_hw->radio_idx]];

		tasklet_init(&irq_task->task, cls_wifi_pci_msi_task, (unsigned long)irq_task);
		ret = request_irq(irq_task->irq, cls_wifi_pci_msi_irq_hdlr, 0,
				"cls_wifi_pci", irq_task);
		if (ret) {
			pr_warn("%s %d ret %d\n", __func__, __LINE__, ret);
			return ret;
		}
	}

	return 0;
}

static int cls_wifi_pci_plat_disable(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct cls_wifi_irq_task *irq_task;
	int i;

	for (i = cls_wifi_plat->hw_params.irq_start[cls_wifi_hw->radio_idx];
			i < cls_wifi_plat->hw_params.irq_end[cls_wifi_hw->radio_idx]; i++) {
		irq_task = &cls_wifi_plat->irq_task[cls_wifi_hw->radio_idx][i];
		free_irq(irq_task->irq, irq_task);
		tasklet_kill(&irq_task->task);
	}

	// warm reset
	cls_wifi_pci_local_sys_write32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_ENABLE_OFFSET,
			CLS_WIFI_LOCAL_SYS_ENABLE_VALUE_DISABLE);
	cls_wifi_plat->ep_ops->irq_trigger(cls_wifi_plat, cls_wifi_hw->radio_idx, IPC_IRQ_A2E_DBG);
	return 0;
}

static u8 *cls_wifi_pci_plat_get_address(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
			int addr_name, unsigned int offset)
{
	struct cls_wifi_pci *cls_wifi_pci = cls_wifi_pci_priv(cls_wifi_plat);

	if (WARN(addr_name >= CLS_WIFI_ADDR_MAX, "Invalid address %d", addr_name))
		return NULL;

	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000 && radio_index == CLS_WIFI_INDEX_C) {
		switch (addr_name) {
		case CLS_WIFI_ADDR_CPU:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr
				+ CLS_WIFI_MERAK2000_SYS_OFFSET_C + offset;
		case CLS_WIFI_ADDR_SHARED:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr
				+ CLS_WIFI_MERAK2000_MEM_OFFSET_C + offset;
		case CLS_WIFI_ADDR_IPC_OUT:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK2000_IPCO_OFFSET_C + offset;
		case CLS_WIFI_ADDR_LSYS:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK2000_LSYS_OFFSET_C + offset;
		default:
			dev_err(cls_wifi_plat->dev, "Invalid address %d for radio index 0x%x\n",
				addr_name, radio_index);
			return NULL;
		}
	} else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000 &&
		   radio_index == CLS_WIFI_INDEX_C) {
		switch (addr_name) {
		case CLS_WIFI_ADDR_CPU:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr
				+ CLS_WIFI_MERAK3000_SYS_OFFSET_C + offset;
		case CLS_WIFI_ADDR_SHARED:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr
				+ CLS_WIFI_MERAK3000_MEM_OFFSET_C + offset;
		case CLS_WIFI_ADDR_IPC_OUT:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK3000_IPCO_OFFSET_C + offset;
		case CLS_WIFI_ADDR_LSYS:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK3000_LSYS_OFFSET_C + offset;
		default:
			dev_err(cls_wifi_plat->dev, "Invalid address %d for radio index 0x%x\n",
				addr_name, radio_index);
			return NULL;
		}
	} else {
		switch (addr_name) {
		case CLS_WIFI_ADDR_CPU:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr
				+ CLS_WIFI_MERAK2000_SYS_OFFSET_C + offset;
		case CLS_WIFI_ADDR_SHARED:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr
				+ cls_wifi_plat->hw_params.shared_mem_offset[radio_index] + offset;
		case CLS_WIFI_ADDR_MSGQ:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr
				+ cls_wifi_plat->hw_params.msgq_reg_offset[radio_index] + offset;
		case CLS_WIFI_ADDR_IRF_TBL:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr
				+ cls_wifi_plat->hw_params.irf_table_mem_offset[radio_index] + offset;
		case CLS_WIFI_ADDR_IPC_IN:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ cls_wifi_plat->hw_params.ipc_in_offset[radio_index] + offset;
		case CLS_WIFI_ADDR_IPC_OUT:
			return cls_wifi_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ cls_wifi_plat->hw_params.ipc_out_offset[radio_index] + offset;
		default:
			dev_err(cls_wifi_plat->dev, "Invalid address %d for radio index 0x%x\n",
				addr_name, radio_index);
			return NULL;
		}
	}
}

static u32 cls_wifi_pci_plat_get_phy_address(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
			int addr_name, unsigned int offset)
{
	u32 irf_tb_phy_offset = cls_wifi_pci_get_irf_tb_phy_off(cls_wifi_plat);

	if (WARN(addr_name >= CLS_WIFI_ADDR_MAX, "Invalid address %d", addr_name))
		return 0;

	switch (addr_name) {
	case CLS_WIFI_ADDR_IRF_PHY:
		return cls_wifi_plat->hw_params.irf_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF_SND_SMP_PHY:
		return cls_wifi_plat->hw_params.irf_snd_smp_mem_offset[radio_index] + offset;
	case CLS_WIFI_ADDR_IRF_TBL_PHY:
		return cls_wifi_plat->hw_params.irf_table_mem_offset[radio_index] +
				irf_tb_phy_offset + offset;
	default:
		dev_err(cls_wifi_plat->dev, "Invalid address %d for radio index 0x%x\n",
				addr_name, radio_index);
		return 0;
	}
}

static void cls_wifi_pci_plat_ack_irq(struct cls_wifi_plat *cls_wifi_plat)
{
}

static int cls_wifi_pci_plat_get_config_reg(struct cls_wifi_plat *cls_wifi_plat, const u32 **list)
{
	return 0;
}

static const struct cls_wifi_if_ops cls_wifi_if_ops_pci = {
	.firmware_on = cls_wifi_pci_firmware_on,
	.enable = cls_wifi_pci_plat_enable,
	.disable = cls_wifi_pci_plat_disable,
	.get_address = cls_wifi_pci_plat_get_address,
	.get_phy_address = cls_wifi_pci_plat_get_phy_address,
	.ack_irq = cls_wifi_pci_plat_ack_irq,
	.get_config_reg = cls_wifi_pci_plat_get_config_reg,
};

int cls_wifi_pci_bus_wake_up(struct cls_wifi_plat *cls_wifi_plat)
{
	return 0;
}
void cls_wifi_pci_bus_release(struct cls_wifi_plat *cls_wifi_plat)
{
}
void cls_wifi_pci_window_write32(struct cls_wifi_plat *cls_wifi_plat, u32 offset, u32 value)
{
}
u32 cls_wifi_pci_window_read32(struct cls_wifi_plat *cls_wifi_plat, u32 offset)
{
	return 0;
}

static const struct cls_wifi_pci_ops cls_wifi_pci_ops_merak = {
	.wakeup = cls_wifi_pci_bus_wake_up,
	.release = cls_wifi_pci_bus_release,
	.window_write32 = cls_wifi_pci_window_write32,
	.window_read32 = cls_wifi_pci_window_read32,
};

static u32 cls_wifi_pci_read32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset)
{
	u32 value_read;

	value_read = ioread32(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_SHARED, offset));
	if (value_read == 0xFFFFFFFF && cls_wifi_plat->cls_wifi_hw[radio_idx] &&
			cls_wifi_plat->cls_wifi_hw[radio_idx]->radio_params &&
			cls_wifi_plat->cls_wifi_hw[radio_idx]->radio_params->debug_pci)
		WARN(1, "%s %d offset 0x%x rd 0x%x\n", __func__, __LINE__, offset, value_read);
	return value_read;
}
static void cls_wifi_pci_write32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		u32 value)
{
	u32 value_read;

	iowrite32(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_SHARED, offset));
	if (cls_wifi_plat->cls_wifi_hw[radio_idx] &&
			cls_wifi_plat->cls_wifi_hw[radio_idx]->radio_params &&
			cls_wifi_plat->cls_wifi_hw[radio_idx]->radio_params->debug_pci) {
		value_read = ioread32(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_SHARED, offset));
		if (value_read == 0xFFFFFFFF)
			WARN(1, "%s %d wr 0x%x rd 0x%x\n", __func__, __LINE__, value, value_read);
	}
}
static void cls_wifi_pci_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		void *dst, u32 length)
{
	memcpy_fromio(dst, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_SHARED, offset), length);
}
static void cls_wifi_pci_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		void *src, u32 length)
{
	if (src)
		memcpy_toio(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
				CLS_WIFI_ADDR_SHARED, offset), src, length);
	else
		memset_io(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
				CLS_WIFI_ADDR_SHARED, offset), 0, length);
	wmb();
}
static u32 cls_wifi_pci_sys_read32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset)
{
	return ioread32(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_SYSTEM, offset));
}
static void cls_wifi_pci_sys_write32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		u32 value)
{
	iowrite32(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_SYSTEM, offset));
}
static void cls_wifi_pci_irf_table_readn(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx,
		u32 offset, void *dst, u32 length)
{
	memcpy_fromio(dst, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_IRF_TBL, offset), length);
}
static void cls_wifi_pci_irf_table_writen(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx,
		u32 offset, void *src, u32 length)
{
	if (src)
		memcpy_toio(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
				CLS_WIFI_ADDR_IRF_TBL, offset), src, length);
	else
		memset_io(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
				CLS_WIFI_ADDR_IRF_TBL, offset), 0, length);
	wmb();
}
static u32 cls_wifi_pci_msgq_read32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset)
{
	return ioread32(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_MSGQ, offset));
}

static void cls_wifi_pci_msgq_write32(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		u32 value)
{
	iowrite32(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_MSGQ, offset));
}

#if defined(__aarch64__) || defined(__x86_64__)
static void cls_wifi_pci_msgq_write64(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 offset,
		u64 value)
{
	writeq(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_MSGQ, offset));
}
#endif

#define IPC_EMB2APP_UNMASK_CLEAR_INDEX  0x000
#define IPC_APP2EMB_TRIGGER_INDEX	   0x100
#define IPC_EMB2APP_RAWSTATUS_INDEX	 0x100
#define IPC_EMB2APP_STATUS_INDEX		0x200
#define IPC_EMB2APP_ACK_INDEX		   0x300
#define IPC_EMB2APP_UNMASK_SET_INDEX	0x400

static void cls_wifi_pci_irq_enable(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	pr_warn("%s\n", __func__);
}
static void cls_wifi_pci_irq_disable(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	pr_warn("%s\n", __func__);
}
static u32 cls_wifi_pci_irq_get_status(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	if (!cls_wifi_is_single_irq(cls_wifi_plat))
		return 0;
	return ioread32(cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_IPC_IN, IPC_EMB2APP_RAWSTATUS_INDEX));
}
static u32 cls_wifi_pci_irq_get_raw_status(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	pr_warn("%s\n", __func__);
	return 0;
}
static void cls_wifi_pci_irq_ack(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	if (!cls_wifi_is_single_irq(cls_wifi_plat))
		return;
	iowrite32(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_IPC_IN, IPC_EMB2APP_ACK_INDEX));
}
static void cls_wifi_pci_irq_trigger(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx, u32 value)
{
	iowrite32(value, cls_wifi_plat->if_ops->get_address(cls_wifi_plat, radio_idx,
				CLS_WIFI_ADDR_IPC_OUT, IPC_APP2EMB_TRIGGER_INDEX));
}

static const struct cls_wifi_ep_ops cls_wifi_ep_ops_pci = {
	.read32 = cls_wifi_pci_read32,
	.write32 = cls_wifi_pci_write32,
	.readn = cls_wifi_pci_readn,
	.writen = cls_wifi_pci_writen,
	.cpu_readn = cls_wifi_pci_cpu_readn,
	.cpu_writen = cls_wifi_pci_cpu_writen,
	.sys_read32 = cls_wifi_pci_sys_read32,
	.sys_write32 = cls_wifi_pci_sys_write32,
	.irf_table_readn = cls_wifi_pci_irf_table_readn,
	.irf_table_writen = cls_wifi_pci_irf_table_writen,
	.msgq_read32 = cls_wifi_pci_msgq_read32,
	.msgq_write32 = cls_wifi_pci_msgq_write32,
#if defined(__aarch64__) || defined(__x86_64__)
	.msgq_write64 = cls_wifi_pci_msgq_write64,
#endif
	.irq_enable = cls_wifi_pci_irq_enable,
	.irq_disable = cls_wifi_pci_irq_disable,
	.irq_get_status = cls_wifi_pci_irq_get_status,
	.irq_get_raw_status = cls_wifi_pci_irq_get_raw_status,
	.irq_ack = cls_wifi_pci_irq_ack,
	.irq_trigger = cls_wifi_pci_irq_trigger,
	.tsensor_get = cls_wifi_pci_tsensor_get,
};


static bool cls_wifi_pci_is_2ep(struct cls_wifi_plat *cls_wifi_plat, struct pci_dev *pci_dev)
{
	int timeout = 10;
	uint32_t value;

	value = cls_wifi_pci_local_sys_read32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_EP_STATE_OFFSET);
	while ((timeout > 0) && (!(value & CLS_WIFI_LOCAL_SYS_EP_STATE_DONE))) {
		dev_err(&pci_dev->dev, "Waiting for 2 EP state, state 0x%x, timeout %d.\n", value, timeout);
#ifdef CONFIG_CLS_EMU_ADAPTER
		msleep(10);
#else
		msleep(1000);
#endif
		timeout--;
		value = cls_wifi_pci_local_sys_read32(cls_wifi_plat, CLS_WIFI_LOCAL_SYS_EP_STATE_OFFSET);
	}

	if ((value & CLS_WIFI_LOCAL_SYS_EP_STATE_EP0) && (value & CLS_WIFI_LOCAL_SYS_EP_STATE_EP1))
		return true;
	else
		return false;
}

static int cls_wifi_pci_probe(struct pci_dev *pci_dev,
				const struct pci_device_id *pci_dev_id)
{
	struct cls_wifi_plat *cls_wifi_plat;
	struct cls_wifi_pci *cls_pci;
	struct cls_wifi_hw *cls_wifi_hw;
	struct cls_wifi_efuse_word_read efuse_param;
	int ret = 0, i;
	static uint32_t bands_enabled;
	bool is_2ep;
	u8 radio_index;
	u8 index_c = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_plat = kzalloc(sizeof(struct cls_wifi_plat) + sizeof(struct cls_wifi_pci),
				GFP_KERNEL);
	if (!cls_wifi_plat)
		return -ENOMEM;

	pci_set_drvdata(pci_dev, (void *)cls_wifi_plat);
	cls_wifi_plat->dev = &(pci_dev->dev);
	cls_wifi_plat->ep_ops = &cls_wifi_ep_ops_pci;
	cls_wifi_plat->if_ops = &cls_wifi_if_ops_pci;
	cls_wifi_plat->bus_type = CLS_WIFI_BUS_TYPE_PCI;
	cls_pci = cls_wifi_pci_priv(cls_wifi_plat);
	cls_pci->dev_id = pci_dev_id->device;
	cls_pci->pci_dev = pci_dev;
	cls_pci->pci_ops = &cls_wifi_pci_ops_merak;

	ret = pci_enable_device(pci_dev);
	if (ret) {
		dev_err(&(pci_dev->dev), "pci_enable_device failed\n");
		goto out_kfree;
	}
	pci_set_master(pci_dev);

	ret = pci_request_regions(pci_dev, KBUILD_MODNAME);
	if (ret) {
		dev_err(&(pci_dev->dev), "pci_request_regions failed\n");
		goto out_master;
	}

	dev_err(&(pci_dev->dev), "pci probe %04x:%04x %04x:%04x\n",
		pci_dev->vendor, pci_dev->device,
		pci_dev->subsystem_vendor, pci_dev->subsystem_device);

	cls_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr = pci_ioremap_bar(pci_dev,
			CLS_WIFI_BAR_IDX_SYS);
	if (!cls_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr) {
		dev_err(&(pci_dev->dev), "pci_ioremap_bar(%d) failed\n", CLS_WIFI_BAR_IDX_SYS);
		ret = -ENOMEM;
		goto out_regions;
	}

	cls_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr = pci_ioremap_bar(pci_dev,
			CLS_WIFI_BAR_IDX_MSGQ);
	if (!cls_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr) {
		dev_err(&(pci_dev->dev), "pci_ioremap_bar(%d) failed\n", CLS_WIFI_BAR_IDX_MSGQ);
		ret = -ENOMEM;
		goto out_bar;
	}

	cls_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr = pci_ioremap_bar(pci_dev,
			CLS_WIFI_BAR_IDX_MEM);
	if (!cls_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr) {
		dev_err(&(pci_dev->dev), "pci_ioremap_bar(%d) failed\n", CLS_WIFI_BAR_IDX_MEM);
		ret = -ENOMEM;
		goto out_bar;
	}

	cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr = pci_ioremap_bar(pci_dev,
			CLS_WIFI_BAR_IDX_REG);
	if (!cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr) {
		dev_err(&(pci_dev->dev), "pci_ioremap_bar(%d) failed\n", CLS_WIFI_BAR_IDX_REG);
		ret = -ENOMEM;
		goto out_bar;
	}

	switch (cls_pci->dev_id) {
	case PCI_DEVICE_ID_MERAK2000_EP0:
		radio_index = 0;
		dev_err(&pci_dev->dev, "PCI device 0x%x, index %d\n", cls_pci->dev_id, radio_index);
		cls_wifi_plat->hw_rev = CLS_WIFI_HW_MERAK2000;
		cls_pci->legacy_out_address = cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK2000_LGCO_OFFSET_EP0;
		cls_wifi_plat->path_info.cal_path = cls_wifi_mod_params.cal_path_cs8662;
		cls_wifi_plat->path_info.irf_path = cls_wifi_mod_params.irf_path_cs8662;
		index_c = CLS_WIFI_INDEX_C;
		break;
	case PCI_DEVICE_ID_MERAK2000_EP1:
		radio_index = 1;
		dev_err(&pci_dev->dev, "PCI device 0x%x, index %d\n", cls_pci->dev_id, radio_index);
		cls_wifi_plat->hw_rev = CLS_WIFI_HW_MERAK2000;
		cls_pci->legacy_out_address = cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK2000_LGCO_OFFSET_EP1;
		cls_wifi_plat->path_info.cal_path = cls_wifi_mod_params.cal_path_cs8662;
		cls_wifi_plat->path_info.irf_path = cls_wifi_mod_params.irf_path_cs8662;
		index_c = CLS_WIFI_INDEX_C;
		break;
	case PCI_DEVICE_ID_MERAK3000:
		dev_err(&pci_dev->dev, "PCI device 0x%x, index %d\n", cls_pci->dev_id, radio_index);
		cls_wifi_plat->hw_rev = CLS_WIFI_HW_MERAK3000;
		cls_pci->legacy_out_address = cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr
				+ CLS_WIFI_MERAK3000_LGCO_OFFSET_EP0;
		cls_wifi_plat->path_info.cal_path = cls_wifi_mod_params.cal_path_m3k;
		cls_wifi_plat->path_info.irf_path = cls_wifi_mod_params.irf_path_m3k;
		index_c = CLS_WIFI_INDEX_C;
		break;
	default:
		cls_wifi_plat->hw_rev = CLS_WIFI_HW_MAX_INVALID;
		dev_err(&pci_dev->dev, "Unknown PCI device 0x%x\n", cls_pci->dev_id);
		ret = -EOPNOTSUPP;
		goto out_bar;
	}

	is_2ep = cls_wifi_pci_is_2ep(cls_wifi_plat, pci_dev);

	ret = cls_wifi_init_hw_params(cls_wifi_plat);
	if (ret)
		goto out_bar;

#ifdef CONFIG_CLS_MSGQ_TEST
	cls_wifi_hw = kzalloc(sizeof(struct cls_wifi_hw), GFP_KERNEL);
	cls_wifi_hw->plat = cls_wifi_plat;
	cls_wifi_hw->dev = cls_wifi_plat->dev;
	cls_wifi_hw->radio_idx = 1;
	cls_wifi_hw->band_cap = cls_wifi_plat->hw_params.band_cap[1];
	cls_wifi_plat->cls_wifi_hw[1] = cls_wifi_hw;

	cls_wifi_msgq_dbgfs_register(cls_wifi_hw);
	return 0;
#endif

	switch (msi_irq) {
	case 0:
		ret = pci_alloc_irq_vectors(pci_dev, 1, 1, PCI_IRQ_LEGACY);
		if (ret < 0) {
			dev_err(&(pci_dev->dev), "Failed to allocate legacy irq, ret %d.", ret);
			goto out_bar;
		}
		dev_err(&(pci_dev->dev), "Use legacy irq.");
		break;
	case 1:
	case 32:
		ret = pci_alloc_irq_vectors(pci_dev, msi_irq, msi_irq, PCI_IRQ_MSI);
		if (ret < 0) {
			dev_err(&(pci_dev->dev), "Failed to allocate %d msi irqs, ret %d.",
					msi_irq, ret);
			goto out_bar;
		}
		dev_err(&(pci_dev->dev), "Use %d msi irqs.", msi_irq);
		break;
	default:
		dev_err(&pci_dev->dev, "Unsupported msi_irq %d.\n", msi_irq);
		ret = -EOPNOTSUPP;
		goto out_bar;
	}
	cls_pci->msi_irq = msi_irq;

#ifdef CLS_WIFI_DUBHE_ETH
	cls_wifi_init_eth_info(cls_wifi_plat);
#endif

	cls_wifi_plat->chip_id = efuse_word;

	if (bands_enabled == bands_enable) {
		dev_err(&(pci_dev->dev), "Skip loading firmware for the second EP.\n");
	} else {
		efuse_param.state = CLS_EFUSE_WORD_UNREADY;
		cls_wifi_pci_efuse_param_set(cls_wifi_plat, &efuse_param);
		ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat, index_c, false);
		if (ret) {
			dev_err(&(pci_dev->dev), "firmware_on [0x%x] failed(%d)\n",
				index_c, ret);
			goto out_msi;
		}

		ret = cls_wifi_pci_efuse_word_wait_success(cls_wifi_plat);
		if (ret == CLS_EFUSE_WORD_UNREADY) {
			dev_err(&(pci_dev->dev), "efuse [0x%x] failed(%d)\n",
				index_c, ret);
		}

		for (i = cls_wifi_plat->hw_params.radio_base;
				i < cls_wifi_plat->hw_params.radio_max; i++) {
			// load afe table for all radios before firmware_on
			if (cls_wifi_irf_init(cls_wifi_plat, i))
				dev_err(&(pci_dev->dev), "Failed to init irf.\n");
		}

#ifdef CFG_PCIE_SHM
		if (cls_wifi_irf_outbound_init(cls_wifi_plat))
			dev_err(&(pci_dev->dev), "Failed to init outbound irf tbl!\n");
#endif
		if (bands_reverse) {
			for (i = cls_wifi_plat->hw_params.radio_max - 1;
					i >= cls_wifi_plat->hw_params.radio_base; i--) {
				if (!(bands_enable & cls_wifi_plat->hw_params.band_cap[i])) {
					dev_err(&(pci_dev->dev), "radio %d is disabled\n", i);
					continue;
				}
				bands_enabled |= cls_wifi_plat->hw_params.band_cap[i];
				if (bands_enabled == bands_enable)
					ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat, i,
							true);
				else
					ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat, i,
							false);
				if (ret) {
					dev_err(&(pci_dev->dev), "radio %d fw fail(%d)\n", i, ret);
					goto out_msi;
				}
			}
		} else {
			for (i = cls_wifi_plat->hw_params.radio_base;
					i < cls_wifi_plat->hw_params.radio_max; i++) {
				if (!(bands_enable & cls_wifi_plat->hw_params.band_cap[i])) {
					dev_err(&(pci_dev->dev), "radio %d is disabled\n", i);
					continue;
				}
				bands_enabled |= cls_wifi_plat->hw_params.band_cap[i];
				if (bands_enabled == bands_enable)
					ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat, i,
							true);
				else
					ret = cls_wifi_plat->if_ops->firmware_on(cls_wifi_plat, i,
							false);
				if (ret) {
					dev_err(&(pci_dev->dev), "radio %d fw fail(%d)\n", i, ret);
					goto out_msi;
				}
			}
		}
	}

	for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
		if (!(bands_enable & cls_wifi_plat->hw_params.band_cap[i])) {
			dev_err(&(pci_dev->dev), "radio %d is disabled by ko params\n", i);
			continue;
		}
		if (is_2ep && (radio_index != i)) {
			dev_err(&(pci_dev->dev), "Skip init radio %d for ep %d in dual ep mode.\n",
					i, radio_index);
			continue;
		}
		ret = cls_wifi_platform_init(cls_wifi_plat, (void **)&cls_wifi_hw, i);
		if (ret) {
			dev_err(&(pci_dev->dev), "cls_wifi_platform_init[%d] failed(%d)\n", i, ret);
			goto out_platform;
		}
		cls_wifi_plat->bands_enabled |= (1 << i);
		cls_wifi_dif_sm_init(cls_wifi_plat, i);
		cls_wifi_heartbeat_init(cls_wifi_plat, i);
		cls_wifi_irf_smp_send_ram_init(cls_wifi_plat, i);
	}
	cls_wifi_dif_online_schedule_init(cls_wifi_plat);
	cls_wifi_tsensor_timer_init(cls_wifi_plat);

	return 0;

out_platform:
	for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
		if (!is_band_enabled(cls_wifi_plat, i)) {
			dev_err(&(pci_dev->dev), "radio %d is disabled by ko params\n", i);
			continue;
		}
		cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[i];
		cls_wifi_heartbeat_deinit(cls_wifi_plat, i);
		cls_wifi_platform_deinit(cls_wifi_hw);
	}
out_msi:
	cls_wifi_tsensor_timer_deinit(cls_wifi_plat);
	pci_free_irq_vectors(pci_dev);
out_bar:
	if (cls_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr);
	if (cls_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr);
	if (cls_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr);
	if (cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr);
out_regions:
	pci_release_regions(pci_dev);
out_master:
	pci_clear_master(pci_dev);
	pci_disable_device(pci_dev);
out_kfree:
	pci_set_drvdata(pci_dev, NULL);
	kfree(cls_wifi_plat);
	return ret;
}

static void cls_wifi_pci_remove(struct pci_dev *pci_dev)
{
	struct cls_wifi_plat *cls_wifi_plat;
	struct cls_wifi_pci *cls_pci;
#ifndef CONFIG_CLS_MSGQ_TEST
	struct cls_wifi_hw *cls_wifi_hw;
	int i;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_plat = pci_get_drvdata(pci_dev);
	cls_pci = cls_wifi_pci_priv(cls_wifi_plat);
	cls_wifi_dif_online_schedule_deinit(cls_wifi_plat);
#ifndef CONFIG_CLS_MSGQ_TEST
	for (i = cls_wifi_plat->hw_params.radio_base; i < cls_wifi_plat->hw_params.radio_max; i++) {
		if (!is_band_enabled(cls_wifi_plat, i)) {
			dev_err(&(pci_dev->dev), "radio %d is disabled by ko params\n", i);
			continue;
		}
		cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[i];
		cls_wifi_heartbeat_deinit(cls_wifi_plat, i);
		cls_wifi_platform_deinit(cls_wifi_hw);
	}
	cls_wifi_tsensor_timer_deinit(cls_wifi_plat);
	pci_free_irq_vectors(cls_pci->pci_dev);
#else
	cls_wifi_msgq_dbgfs_unregister();
#endif
	if (cls_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_sys_mem_vaddr);
	if (cls_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_msgq_vaddr);
	if (cls_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_shared_mem_vaddr);
	if (cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr)
		iounmap(cls_pci->pci_vaddr.cls_wifi_pci_ipc_vaddr);
	pci_release_regions(cls_pci->pci_dev);
	pci_clear_master(cls_pci->pci_dev);
	pci_disable_device(cls_pci->pci_dev);
	pci_set_drvdata(pci_dev, NULL);
	kfree(cls_wifi_plat);
}

static struct pci_driver cls_wifi_pci_driver = {
	.name = "cls_wifi_pci",
	.id_table = cls_wifi_pci_id_table,
	.probe = cls_wifi_pci_probe,
	.remove = cls_wifi_pci_remove,
};

static int cls_wifi_pci_init(void)
{
	int ret;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	ret = pci_register_driver(&cls_wifi_pci_driver);
	if (ret)
		pr_err("failed to register cls_wifi pci driver: %d\n",
			   ret);

	return ret;
}

static void cls_wifi_pci_exit(void)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	pci_unregister_driver(&cls_wifi_pci_driver);
}

MODULE_FIRMWARE(CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_CONFIG_FW_NAME);
MODULE_FIRMWARE(CLS_WIFI_MERAK2000_FIRMARE_PATH CLS_WIFI_AGC_FW_NAME);
module_init(cls_wifi_pci_init);
module_exit(cls_wifi_pci_exit);
MODULE_LICENSE("GPL");
