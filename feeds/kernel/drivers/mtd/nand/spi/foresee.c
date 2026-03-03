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
 *  ShenZhen Longsys Electronics Foresee SPI NAND Flash Driver .
 *
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_FORESEE		0xCD	/* ShenZhen Longsys Electronics FORESEE Products Manufacturer ID */

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

	region->offset = 1;
	region->length = mtd->oobsize - 1;

	return 0;
}

static int f35sqa001g_1_ecc_get_status(struct spinand_device *spinand,
				u8 status)
{
	switch (status & STATUS_ECC_MASK) {
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
	.free = f35sqa001g_ooblayout_free,
};

static const struct spinand_info longsys_foresee_spinand_table[] = {
	SPINAND_INFO("F35SQA001G",
		    SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x71),
		    NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		    NAND_ECCREQ(1, 512),
		    SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					     &write_cache_variants,
					     &update_cache_variants),
		    SPINAND_HAS_QE_BIT,
		    SPINAND_ECCINFO(&f35sqa001g_ooblayout,
				    f35sqa001g_1_ecc_get_status)),
};

static const struct spinand_manufacturer_ops longsys_foresee_spinand_manuf_ops = {
};

const struct spinand_manufacturer longsys_foresee_spinand_manufacturer = {
	.id = SPINAND_MFR_FORESEE,
	.name = "LongSys ForeSee",
	.chips = longsys_foresee_spinand_table,
	.nchips = ARRAY_SIZE(longsys_foresee_spinand_table),
	.ops = &longsys_foresee_spinand_manuf_ops,
};

