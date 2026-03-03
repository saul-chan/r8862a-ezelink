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
 *  Zetta SPI NAND Flash Driver.
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

#define SPINAND_MFR_ZETTA		0xBA      /* Zetta Manufacturer ID*/
#define ZETTA_STATUS_ECC_MASK	GENMASK(5, 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int zd35x1ga_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 4;
	region->length = 4;

	return 0;
}

static int zd35x1ga_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 8;

	return 0;
}

static int zd35x1ga_4_ecc_get_status(struct spinand_device *spinand,
				   u8 status)
{
	switch (status & ZETTA_STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case STATUS_ECC_HAS_BITFLIPS:
		return 4;
	default:
		break;
	}
	return 0;
}

static const struct mtd_ooblayout_ops zd35x1ga_ooblayout = {
	.ecc = zd35x1ga_ooblayout_ecc,
	.rfree = zd35x1ga_ooblayout_free,
};

static const struct spinand_info zetta_spinand_table[] = {
	/* 1v8 */
	SPINAND_INFO("ZD35M1GA", 0x21,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 2, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&zd35x1ga_ooblayout, zd35x1ga_4_ecc_get_status)),
	/* 3v3 */
	SPINAND_INFO("ZD35Q1GC", 0x71,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 2, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&zd35x1ga_ooblayout, zd35x1ga_4_ecc_get_status)),
};

static int zetta_spinand_detect (struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	/*
	 * Macronix SPI NAND read ID needs a dummy byte, so the first byte in
	 * raw_id is garbage.
	 */
	if (id[1] != SPINAND_MFR_ZETTA)
		return 0;

	ret = spinand_match_and_init(spinand, zetta_spinand_table,
				     ARRAY_SIZE(zetta_spinand_table),
				     id[2]);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops zetta_spinand_manuf_ops = {
	.detect = zetta_spinand_detect,
};

const struct spinand_manufacturer zetta_spinand_manufacturer = {
	.id = SPINAND_MFR_ZETTA,
	.name = "Zetta",
	.ops = &zetta_spinand_manuf_ops,
};

