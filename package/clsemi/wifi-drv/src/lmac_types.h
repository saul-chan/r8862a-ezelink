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

#ifndef _LMAC_TYPES_H_
#define _LMAC_TYPES_H_


/**
 ****************************************************************************************
 * @addtogroup CO_INT
 * @ingroup COMMON
 * @brief Common integer standard types (removes use of stdint)
 *
 * @{
 ****************************************************************************************
 */


/*
 * DEFINES
 ****************************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <linux/bits.h>
#else
#include <linux/bitops.h>
#endif

typedef uint8_t u8_l;
typedef int8_t s8_l;
typedef uint16_t u16_l;
typedef int16_t s16_l;
typedef uint32_t u32_l;
typedef int32_t s32_l;
typedef uint64_t u64_l;

/**
 * BF_FIELD - Define mask and offset value for a given field of a bitfield structure
 *
 * @bf: Name of the bitfield
 * @field: Name of the field
 * @lsb: Bit index at which field starts
 * @bit_len: Length, in bits, of the field
 *
 * This defines the values `<bf>_<field>_MSK` and `<bf>_<field>_OFT`.\n
 * This macro is intended to be used inside an enum, so that all fields of a given
 * bitfield are defined at the same place.\n
 * Here is an example, for a bitfield REG that is composed of 3 fields (F1, F2 and F3):
 * enum REG_BF
 * {
 *	 BF_FIELD(REG, F1, 0, 12),
 *	 BF_FIELD(REG, F2, 12, 12),
 *	 BF_FIELD(REG, F3, 24, 8).
 * };
 *
 * Once defined using BF_FIELD, a bitfield structure can be manipulated using BF_GET,
 * BF_SET and BF_VAL macros.\n
 */
#define BF_FIELD(bf, field, lsb, bit_len)			 \
	bf##_##field##_MSK = ((1 << bit_len) - 1) << lsb, \
		bf##_##field##_OFT = lsb

/**
 * BF_GET -  Extract field value from a bitfield structure
 *
 * @bf: Name of the bitfield (same as the one used in @ref BF_FIELD)
 * @field: Name of the field (same as the one used in @ref BF_FIELD)
 * @bf_val: Bitfield global value from which to extract field value
 *
 * @return The right shifted value of the requested field
 */
#define BF_GET(bf, field, bf_val) ((bf##_##field##_MSK & (bf_val)) >> bf##_##field##_OFT)

/**
 * BF_SET - Test whether a field value from a bitfield structure is null or not
 *
 * @bf: Name of the bitfield (same as the one used in @ref BF_FIELD)
 * @field: Name of the field (same as the one used in @ref BF_FIELD)
 * @bf_val: Bitfield global value from which to extract field value
 * @return A Boolean indicating whether the requested field value is 0 or not
 *
 * This is mainly intended for field that are defined on a single bit, to avoid the
 * unnecessary right shift of @ref BF_GET.
 */
#define BF_IS_SET(bf, field, bf_val) ((bf##_##field##_MSK & (bf_val)) != 0)

/**
 * BF_SET - Set field of a bitfield structure to a given value
 *
 * @bf: Name of the bitfield (same as the one used in @ref BF_FIELD)
 * @field: Name of the field (same as the one used in @ref BF_FIELD)
 * @bf_var: Variable name that contains the bitfield global value to be updated
 * @field_val: Value to set for the field
 *
 * This macro is intended to be used for field defined by @ref BF_FIELD.
 * @note: If value is too large for the field it is truncated.
 */
#define BF_SET(bf, field, bf_var, field_val)	 \
	(bf_var = (((bf_var) & ~bf##_##field##_MSK) | \
			  (bf##_##field##_MSK & ((field_val) << bf##_##field##_OFT))))

/**
 * BF_VAL - Return the 'left shifted' value for a field of a bitfield structure
 *
 * @bf: Name of the bitfield (same as the one used in @ref BF_FIELD)
 * @field: Name of the field (same as the one used in @ref BF_FIELD)
 * @field_val: Value to set for the field
 * @return Value of the bitfield structure with only field `<field>` set to `field_val`
 *
 * This macro is intended to be used for field defined by @ref BF_FIELD.
 * This can be used instead of @ref BF_SET when there is no need to clear the field first.
 * Note: If value is too large for the field it is truncated.
 */
#define BF_VAL(bf, field, field_val)							\
	(bf##_##field##_MSK & ((field_val) << bf##_##field##_OFT))



/// @} CO_INT
#endif // _LMAC_TYPES_H_
