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

#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <malloc.h>
#include <memalign.h>
#include <mtd.h>
#include <search.h>

#if CONFIG_SYSINFO_SIZE_MTD_REDUND < CONFIG_SYSINFO_SIZE
#undef CONFIG_SYSINFO_SIZE_MTD_REDUND
#define CONFIG_SYSINFO_SIZE_MTD_REDUND CONFIG_SYSINFO_SIZE
#endif

DECLARE_GLOBAL_DATA_PTR;

static int sysinfo_mtd_init(void)
{
	gd->sysinfo_addr	= (ulong)&default_sysinfo[0];
	gd->sysinfo_valid	= ENV_VALID;
	return 0;
}

static struct mtd_info *sysinfo_mtd_get_dev(void)
{
	struct mtd_info *mtd;

	mtd_probe_devices();

	mtd = get_mtd_device_nm(CONFIG_SYSINFO_MTD_NAME);
	if (IS_ERR(mtd) || !mtd) {
		printf("MTD device '%s' not found\n", CONFIG_SYSINFO_MTD_NAME);
		return NULL;
	}

	return mtd;
}

static inline bool mtd_addr_is_block_aligned(struct mtd_info *mtd, u64 addr)
{
	return (addr & mtd->erasesize_mask) == 0;
}

static int mtd_io_skip_bad(struct mtd_info *mtd, bool read, loff_t offset,
			   size_t length, size_t redund, u8 *buffer)
{
	struct mtd_oob_ops io_op = {};
	size_t remaining = length;
	loff_t off, end;
	int ret;

	io_op.mode = MTD_OPS_PLACE_OOB;
	io_op.len = mtd->writesize;
	io_op.datbuf = (void *)buffer;

	/* Search for the first good block after the given offset */
	off = offset;
	end = (off + redund) | (mtd->erasesize - 1);
	while (mtd_block_isbad(mtd, off) && off < end)
		off += mtd->erasesize;

	/* Reached end position */
	if (off >= end)
		return -EIO;

	/* Loop over the pages to do the actual read/write */
	while (remaining) {
		/* Skip the block if it is bad */
		if (mtd_addr_is_block_aligned(mtd, off) &&
		    mtd_block_isbad(mtd, off)) {
			off += mtd->erasesize;
			continue;
		}

		if (read)
			ret = mtd_read_oob(mtd, off, &io_op);
		else
			ret = mtd_write_oob(mtd, off, &io_op);

		if (ret) {
			printf("Failure while %s at offset 0x%llx\n",
			       read ? "reading" : "writing", off);
			break;
		}

		off += io_op.retlen;
		remaining -= io_op.retlen;
		io_op.datbuf += io_op.retlen;
		io_op.oobbuf += io_op.oobretlen;

		/* Reached end position */
		if (off >= end)
			return -EIO;
	}

	return 0;
}

#ifdef CONFIG_CMD_SAVESYSINFO
static int mtd_erase_skip_bad(struct mtd_info *mtd, loff_t offset,
			      size_t length, size_t redund)
{
	struct erase_info erase_op = {};
	loff_t end = (offset + redund) | (mtd->erasesize - 1);
	int ret;

	erase_op.mtd = mtd;
	erase_op.addr = offset;
	erase_op.len = length;

	while (erase_op.len) {
		ret = mtd_erase(mtd, &erase_op);

		/* Abort if its not a bad block error */
		if (ret != -EIO)
			return ret;

		printf("Skipping bad block at 0x%08llx\n", erase_op.fail_addr);

		/* Skip bad block and continue behind it */
		erase_op.len -= erase_op.fail_addr - erase_op.addr;
		erase_op.len -= mtd->erasesize;
		erase_op.addr = erase_op.fail_addr + mtd->erasesize;

		/* Reached end position */
		if (erase_op.addr >= end)
			return -EIO;
	}

	return 0;
}

static int sysinfo_mtd_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	struct mtd_info *mtd;
	int ret = 0;

	ret = sysinfo_env_export(env_new);
	if (ret)
		return ret;

	mtd = sysinfo_mtd_get_dev();
	if (!mtd)
		return 1;

	printf("Erasing on MTD device '%s'... ", mtd->name);

	ret = mtd_erase_skip_bad(mtd, CONFIG_SYSINFO_OFFSET, CONFIG_SYSINFO_SIZE,
				 CONFIG_SYSINFO_SIZE_MTD_REDUND);

	puts(ret ? "FAILED\n" : "OK\n");

	if (ret) {
		put_mtd_device(mtd);
		return 1;
	}

	printf("Writing to MTD device '%s'... ", mtd->name);

	ret = mtd_io_skip_bad(mtd, false, CONFIG_SYSINFO_OFFSET, CONFIG_SYSINFO_SIZE,
			      CONFIG_SYSINFO_SIZE_MTD_REDUND, (u8 *)env_new);

	puts(ret ? "FAILED\n" : "OK\n");

	put_mtd_device(mtd);

	return !!ret;
}
#endif /* CONFIG_CMD_SAVESYSINFO */

static int readsysinfo(size_t offset, u_char *buf)
{
	struct mtd_info *mtd;
	int ret;

	mtd = sysinfo_mtd_get_dev();
	if (!mtd)
		return 1;

	ret = mtd_io_skip_bad(mtd, true, offset, CONFIG_SYSINFO_SIZE,
			      CONFIG_SYSINFO_SIZE_MTD_REDUND, buf);

	put_mtd_device(mtd);

	return !!ret;
}

static int sysinfo_mtd_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_SYSINFO_SIZE);
	int ret;

	ret = readsysinfo(CONFIG_SYSINFO_OFFSET, (u_char *)buf);
	if (ret) {
		sysinfo_set_default("readsysinfo() failed", 0);
		return -EIO;
	}

	return sysinfo_env_import(buf, 1, H_EXTERNAL);
}

U_BOOT_SYSINFO_LOCATION(mtd) = {
	.location	= ENVL_MTD,
	SYSINFO_NAME("MTD")
	.load		= sysinfo_mtd_load,
#if defined(CONFIG_CMD_SAVESYSINFO)
	.save		= sysinfo_save_ptr(sysinfo_mtd_save),
#endif
	.init		= sysinfo_mtd_init,
};
