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

/* mkclsimg.c
 * generate cls image
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <zlib.h>
#include "clsimg.h"

char *image_dir = NULL;

long int getFileSize(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) != 0) {
		fprintf(stderr, "Cannot open '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	return st.st_size;
}

int update_img_hdr(const char *filename, cls_hdr *hdr)
{
	FILE *fp;

	fp = fopen(filename, "r+");
	if (!fp) {
		printf("open %s error\n", filename);
		return -1;
	}

	fseek(fp, 0, SEEK_SET);
	fwrite(hdr, sizeof(*hdr), 1, fp);
	fclose(fp);

	return 0;
}

unsigned long crc32_file_with_seed(const char *file, unsigned long seed)
{
	FILE *fp;
	size_t len;
	unsigned char buffer[1024];
	unsigned long crc = seed;

	fp = fopen(file, "rb");

	if (fp == NULL) {
		printf("Cannot open file:%s.\n", file);
		return -1;
	}

	while ((len = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0)
		crc = crc32(crc, buffer, len);

	fclose(fp);

	return crc;
}

unsigned long crc32_file(const char *file)
{
	return crc32_file_with_seed(file, 0);
}

int append_img(const char *name, FILE *first_file, int type)
{
	FILE *img;
	img_tlv i_tlv;
	struct stat statbuf;
	int size;
	int ret = 0, len = 0;
	char buffer[1024];
	char img_name[512];
	char tempFileName[] = "/tmp/clstempfile_XXXXXX";
	int tmp_fd;

	memset(&i_tlv, 0, sizeof(img_tlv));

	if (image_dir)
		snprintf(img_name, sizeof(img_name), "%s/%s", image_dir, name);
	else
		strncpy(img_name, name, sizeof(img_name));

	if (type == script_gz || type == info_gz) {
		i_tlv.is_gzip = 1;

		tmp_fd = mkstemp(tempFileName);
		if (tmp_fd != -1) {
			printf("Temporary file name: %s\n", tempFileName);
			snprintf(buffer, sizeof(buffer), "gzip -k -f -9 %s -c > %s ", img_name, tempFileName);
			printf("%s\n", buffer);
			system(buffer);
			strncpy(img_name, tempFileName, sizeof(tempFileName));
		} else {
			perror("Error creating temporary file");
			return -1;
		}
	}

#ifdef DEBUG
	printf("image_name:%s\n", img_name);
#endif

	if (!first_file) {
		printf(RED "file error\n" NONE);
		return -1;
	}

	/*get img file len*/
	len = getFileSize(img_name);
	if (len <= 0) {
		printf(RED "%s open error\n" NONE, img_name);
		if (i_tlv.is_gzip) {
			printf(RED "remove tmp file:%s\n" NONE, img_name);
			unlink(tempFileName);
		}

		exit(-2);
	}

	printf(GREEN "%s = %s size=%d\n" NONE, image_type_str[type].name, img_name, len);
	i_tlv.magic = TLV_MAGIC;
	i_tlv.type = image_type_str[type].id;
	i_tlv.len = len;
	/*TODO file open fail?*/
	i_tlv.crc = crc32_file(img_name);
	printf(GREEN "%s crc32=%x\n" NONE, image_type_str[type].name, i_tlv.crc);

	/*set pointer to end*/
	fseek(first_file, 0, SEEK_END);
	fwrite(&i_tlv, sizeof(img_tlv), 1, first_file);
	/*move pointer to end*/

	img = fopen(img_name, "r");
	if (!img) {
		printf(RED "img:%s open fail!!!\n" NONE, img_name);
		exit(-1);
	}

	//read 2nd file and write to 1st file
	while ((len = fread(buffer, sizeof(char), sizeof(buffer), img)) > 0)
		fwrite(buffer, sizeof(char), len, first_file);

	fclose(img);

	if (i_tlv.is_gzip)
		unlink(img_name);
}

int append_tal_img(FILE *img)
{
	img_tlv i_tlv;

	fseek(img, 0, SEEK_END);

	if (!img)
		return -1;

	i_tlv.magic = TLV_MAGIC;
	i_tlv.type = image_type_str[end].id;
	i_tlv.len = 0;
	/*TODO file open fail?*/
	i_tlv.crc = 0;

	fwrite(&i_tlv, sizeof(char), sizeof(img_tlv), img);
}

int print_help(void)
{
	printf("\t--sbl(-S) for sbl\n"
			"\t--aft(-a) for atf\n"
			"\t--uboot(-u) for uboot\n"
			"\t--uboot_env(-e) for uboot_env\n"
			"\t--firmware(-f), for firmware\n"
			"\t--dtb(-d), for dtb\n"
			"\t--cal(-c), for calibration data\n"
			"\t--script(-s), for upgrade script\n"
			"\t--info(-i), for image information\n"
			"\t--path(-p), for sub-image path\n"
			"\t--out(-o), out put image name\n"
			"\t--help(-h), print help\n"
		  );
}

int main(int argc, char *argv[])
{
	FILE *file;
	cls_hdr hdr;
	char *img_name = "test-all-img.img";
	int hdr_len = sizeof(hdr);
	int whole_crc = 0;
	int opt;
	int option_index = 0;
	char *string = "u:e:f:d:c:s:n:p:i:o:hS:a:";

	static struct option long_options[] = {
		{"sbl", required_argument, NULL, 'S'},
		{"atf", required_argument, NULL, 'a'},
		{"uboot", required_argument, NULL, 'u'},
		{"uboot-env", required_argument, NULL, 'e'},
		{"firmware", required_argument, NULL, 'f'},
		{"dtb", required_argument, NULL, 'd'},
		{"cal", required_argument, NULL, 'c'},
		{"script", required_argument, NULL, 's'},
		{"info", required_argument, NULL, 'i'},
		{"path", required_argument, NULL, 'p'},
		{"out", required_argument, NULL, 'o'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0},
	};

	if (argc == 1) {
		print_help();
		exit(0);
	}

	memset(&hdr, 0, hdr_len);
	/*set memory to 0*/
	hdr.magic = CLS_IMG_MAGIC;
	hdr.ver.ver_1 = 192;
	hdr.ver.ver_2 = 168;
	hdr.ver.ver_3 = 100;
	hdr.ver.ver_4 = 32;
	hdr.hdr_len = hdr_len;
	/*64k*/
	hdr.block_size = 64*Kbytes;

	/*get image name first*/
	while ((opt = getopt_long(argc, argv, string, long_options, &option_index)) != -1) {
		switch (opt) {
		case 'h':
			print_help();
			exit(0);
			break;
		case 'o':
			img_name = optarg;
			printf("out_put name=%s\n", optarg);
			break;
		default:
			break;
		}
	}

	file = fopen(img_name, "w+");
	if (file == NULL) {
		printf(RED "can't open file %s\n" NONE, img_name);
		return 1;
	}

	/*hold the place for header*/
	fwrite(&hdr, hdr_len, 1, file);

	/*rerun getopt_long, global optind must set to 0*/
	optind = 0;
	while ((opt = getopt_long(argc, argv, string, long_options, &option_index)) != -1) {
		switch (opt) {
		case 'S':
			append_img(optarg, file, sbl);
			break;
		case 'a':
			append_img(optarg, file, atf);
			break;
		case 'u':
			append_img(optarg, file, uboot);
			break;
		case 'e':
			append_img(optarg, file, uboot_env);
			break;
		case 'f':
			append_img(optarg, file, firmware);
			break;
		case 'd':
			append_img(optarg, file, dtb);
			break;
		case 'c':
			append_img(optarg, file, cal);
			break;
		case 's':
			append_img(optarg, file, script_gz);
			break;
		case 'i':
			append_img(optarg, file, info_gz);
			break;
		case 'p':
			image_dir = optarg;
			printf("image_dir=%s\n", optarg);
			break;
		default:
			break;
		}
	}

	append_tal_img(file);
	fclose(file);

	/*must placed in end*/
	hdr.total_len = getFileSize(img_name);
	hdr.total_crc = crc32_file(img_name);
	whole_crc = crc32_file(img_name);
	printf(LIGHT_CYAN "total len=%d total crc=%x\n" NONE, hdr.total_len, whole_crc);

	hdr.hdr_crc = crc32(0, (void *)&hdr, sizeof(cls_hdr));
	printf(LIGHT_CYAN "hdr len=%d hdr crc=%x\n" NONE, hdr.hdr_len, hdr.hdr_crc);

	update_img_hdr(img_name, &hdr);

	return 0;
}
