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

/* clsimg.h
 * header file for check amd mk img tools
 */


#ifndef __CLSIMG_H__
#define CLS_IMG_MAGIC 0xbfde3210
#define Mbytes (1024*1024)
#define Kbytes (1024)

#define MACRO_STR(x) {x, #x}

#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

typedef struct {
	uint32_t id;
	char *name;
} MACRO_STR_T;

typedef struct {
	int sbl:1;
	int atf:1;
	int uboot:1;
	int uboot_env:1;
	int firmware:1;
	int dtb:1;
	int cal:1;
	int script:1;
} __attribute__((aligned(32))) cls_content_t;

typedef enum {
	sbl=0,
	atf,
	uboot,
	uboot_env,
	firmware,
	dtb,
	cal,
	info_gz,
	script_gz,
	end=9,
	/*new element must put below end*/
} img_type;

MACRO_STR_T image_type_str[] = {
	MACRO_STR(sbl),
	MACRO_STR(atf),
	MACRO_STR(uboot),
	MACRO_STR(uboot_env),
	MACRO_STR(firmware),
	MACRO_STR(dtb),
	MACRO_STR(cal),
	MACRO_STR(info_gz),
	MACRO_STR(script_gz),
	MACRO_STR(end),
	/*new item must put after end, keep same order as img_type*/

};

#define TLV_MAGIC 0x43218765

/*every img add tlv*/
typedef struct {
	uint32_t magic;
	uint16_t type;
	uint16_t is_gzip;
	uint32_t len;
	uint32_t crc;
} __attribute__((aligned(4))) img_tlv;

#define IMG_TLV_LEN(a) (a.len + sizeof(img_tlv))

typedef struct {
	uint32_t magic;

	struct {
		uint8_t ver_1:8;
		uint8_t ver_2:8;
		uint8_t ver_3:8;
		uint8_t ver_4:8;
	} ver;

	uint32_t hdr_len;
	uint32_t hdr_crc;
	uint32_t total_len;
	uint32_t total_crc;
	/* flash block_size size, padding to block_size */
	uint32_t block_size;
} __attribute__((aligned(16))) cls_hdr;

#endif /*__CLSIMG_H__*/
