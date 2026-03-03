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

#ifndef __CLS_BIMID_H__
#define __CLS_BIMID_H__

enum mft_hdr_subtype_e {
	MFT_HDR_MFTS		= 0x4D465453U,  /* 'M' 'F' 'T' 'S'  */

	MFT_HDR_CFG_MASK	= 0x43464700U,  /* 'C' 'F' 'G' 0x00 */
	MFT_HDR_CFG_DATA	= 0x43464701U,  /* 'C' 'F' 'G' 0x01 CONFIG_DATA_MFT */
	MFT_HDR_DDR_CFG		= 0x43464702U,  /* 'C' 'F' 'G' 0x02 DDR_CONFIG_MFT */
	MFT_HDR_PINMUX_CFG	= 0x43464703U,  /* 'C' 'F' 'G' 0x03 PINMUX_CONFIG_MFT */
	MFT_HDR_BOOT_FW_CFG	= 0x43464704U,  /* 'C' 'F' 'G' 0x04 TRUSTED_BOOT_FW_CONFIG_MFT */
	MFT_HDR_ATF_CFG		= 0x43464705U,  /* 'C' 'F' 'G' 0x05 SOC_FW_CONFIG_MFT */
	MFT_HDR_TOS_CFG		= 0x43464706U,  /* 'C' 'F' 'G' 0x06 TRUSTED_OS_FW_CONFIG_MFT */
	MFT_HDR_UBOOT_CFG	= 0x43464707U,  /* 'C' 'F' 'G' 0x07 NON_TRUSTED_FW_CONFIG_MFT */
	MFT_HDR_DDR_DIAG_CFG	= 0x43464708U,  /* 'C' 'F' 'G' 0x08 DDR_DIAG_CONFIG_MFT */

	MFT_HDR_IMG_MASK	= 0x494D4700U,  /* 'I' 'M' 'G' 0x00 */
	MFT_HDR_SBL_IMG		= 0x494D4701U,  /* 'I' 'M' 'G' 0x01 Trusted Boot Firmware BL2 */
	MFT_HDR_ATF_IMG		= 0x494D4702U,  /* 'I' 'M' 'G' 0x02 EL3 Runtime Firmware BL31 */
	MFT_HDR_TOS_IMG		= 0x494D4703U,  /* 'I' 'M' 'G' 0x03 Secure Payload BL32 (Trusted OS) */
	MFT_HDR_UBOOT_IMG	= 0x494D4704U,  /* 'I' 'M' 'G' 0x04 Non-Trusted Firmware BL33 */
	MFT_HDR_KERNEL_IMG	= 0x494D4705U,  /* 'I' 'M' 'G' 0x05 Non-Trusted OS Kernel */
};

#define MFT_HDR_MAGIC_ID		0x24435324U /* '$' 'C' 'S' '$'*/
#define ENC_MAX_IV_SIZE			16
#define ENC_MAX_TAG_SIZE		16
#define ENC_MAX_KEY_SIZE		32

/* Clourney's bootloader image header format */
struct cls_img_hdr_t {
	unsigned int header_type;
	unsigned int header_subtype;
	unsigned int length;
	unsigned int header_version;
	int nvctr_type;
	unsigned int secure_flag;
	unsigned int img_is_enc;
	unsigned int pukey_sig_is_enc;
	unsigned int vendor_id;
	unsigned int build_date;
	unsigned int magic_id;
	unsigned int reserved[4];
	unsigned int crc32c;
};

struct pubkey_sign_t {
	unsigned int   key_scheme;
	unsigned int   passwd_offset;
	unsigned int   passwd_len;
	unsigned int   pukey_offset;
	unsigned int   pukey_len;
	unsigned int   sign_offset;
	unsigned int   sign_len;
	unsigned int   sign_pad;
	unsigned int   reserved[8];
};

struct img_enc_info_t {
	unsigned int  enc_scheme;
	unsigned int  enc_hmac_keylen;
	unsigned int  enc_hmac_ntimes;
	unsigned int  enc_ivlen;
	unsigned int  enc_taglen;
	unsigned char enc_hmac_key[ENC_MAX_KEY_SIZE];
	unsigned char enc_iv[ENC_MAX_IV_SIZE];
	unsigned char enc_tag[ENC_MAX_TAG_SIZE];
	unsigned int  enc_total_len;
	unsigned int  enc_block_size;
	unsigned int  enc_split_deep;
	unsigned int  reserved[4];
};

struct img_body_info_t {
	unsigned int  img_body_offset;
	unsigned int  img_body_len;
	unsigned int  img_body_entry;
	unsigned int  reserved;
};

struct cls_manifest_hdr_t {
	struct cls_img_hdr_t cls_img_hdr;
	struct img_body_info_t img_info;
	struct img_enc_info_t  enc_info;
	struct pubkey_sign_t  rot_pubkey_sign;
	struct pubkey_sign_t  img_pubkey_sign;
	unsigned int   reserved[15];
	unsigned int   crc32c;
};

struct paras_header_t {
	uint8_t type;		//!< type of the structure
	uint8_t version;	//!< version of this structure
	uint16_t size;		//!< size of this structure in bytes
	uint32_t attr;		//!< attributes: unused bits SBZ
};

struct img_info_hdr_t {
	struct paras_header_t h;
	uint32_t image_id; //!< Contains image id for the image
	uint32_t image_base; // !< reserve
	uint32_t image_size; //!< bytes read from image file
	uint32_t total_size; //!< bytes read from image file
	uint32_t reserved[10];
};

struct manifest_rot_t {
	struct cls_img_hdr_t cls_img_hdr;
	unsigned int   passwd_offset; // sm2 id
	unsigned int   passwd_len;
	unsigned int   pukey_offset;
	unsigned int   pukey_len;
	unsigned int   pukey_scheme; //[rsa(0) and sm2(1)]
	unsigned int   body_sign_offset;
	unsigned int   body_sign_len;
	unsigned int   body_sign_pad; //signature type
	unsigned int   hash_tbl_num;
	unsigned int   reserved;
};

enum HASH_ALG_E {
	_SHA256 = 0x0,
	_SM3 = 0x1
};

#endif
