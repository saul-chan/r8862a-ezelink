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
 *  ShenZhen Longsys Electronics Foresee SPI NAND Flash Driver.
 *
 */

#ifndef __UBOOT__
#include <malloc.h>
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_FORESEE		0xCD      /* ShenZhen Longsys Electronics FORESEE Products Manufacturer ID*/
#define FORESEE_STATUS_ECC_MASK		GENMASK(5, 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int f35sqa001g_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int f35sqa001g_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM(0x800h byte is for Bad Block Marker). */
	region->offset = 1;
	/* Spare area 0x801H to 0x83FH is all available for user*/
	region->length = mtd->oobsize - 1;

	return 0;
}

static int f35sqa001g_1_ecc_get_status(struct spinand_device *spinand,
				   u8 status)
{
	switch (status & FORESEE_STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case STATUS_ECC_HAS_BITFLIPS:
		return 1;
	default:
		break;
	}
	return 0;
}
static const struct mtd_ooblayout_ops f35sqa001g_ooblayout = {
	.ecc = f35sqa001g_ooblayout_ecc,
	.rfree = f35sqa001g_ooblayout_free,
};

static const struct spinand_info longsys_foresee_spinand_table[] = {
	SPINAND_INFO("F35SQA001G", 0x71,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 2, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&f35sqa001g_ooblayout, f35sqa001g_1_ecc_get_status)),
};

static int longsys_foresee_spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	/*
	 * Fudan Micro SPI NAND read ID needs a dummy byte, so the first byte in
	 * raw_id is garbage.
	 */
	if (id[1] != SPINAND_MFR_FORESEE)
		return 0;

	ret = spinand_match_and_init(spinand, longsys_foresee_spinand_table,
				     ARRAY_SIZE(longsys_foresee_spinand_table),
				     id[2]);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops longsys_foresee_spinand_manuf_ops = {
	.detect = longsys_foresee_spinand_detect,
};

const struct spinand_manufacturer longsys_foresee_spinand_manufacturer = {
	.id = SPINAND_MFR_FORESEE,
	.name = "LongSys ForeSee",
	.ops = &longsys_foresee_spinand_manuf_ops,
};

