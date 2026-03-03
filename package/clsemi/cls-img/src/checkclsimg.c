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

/* checkclsimg.c
 * Check and upgrade cls image
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <zlib.h>
#include "clsimg.h"

char dirname[] = "split_img_dir";
char *global_img_name;
int run_script = 0;

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
		crc = crc32(crc, buffer, len); // update crc

	fclose(fp);

	return crc;
}

unsigned long crc32_file(const char *file)
{
	return crc32_file_with_seed(file, 0);
}

int print_cls_hdr_info(cls_hdr *hdr)
{
	printf("magic=%x\n", hdr->magic);
	printf("ver_1=%d\n", hdr->ver.ver_1);
	printf("ver_2=%d\n", hdr->ver.ver_2);
	printf("ver_3=%d\n", hdr->ver.ver_3);
	printf("ver_4=%d\n", hdr->ver.ver_4);
	printf("hdr_len=%d\n", hdr->hdr_len);
	printf("hdr_crc=%x\n", hdr->hdr_crc);
	printf("total_len=%d\n", hdr->total_len);
	printf("total_crc=%x\n", hdr->total_crc);
}

int check_cls_hdr_valid(cls_hdr *hdr)
{
	uint32_t crc = hdr->hdr_crc;
	uint32_t new_crc;

	if (hdr->magic != CLS_IMG_MAGIC) {
		printf("error hdr magic=%x, expect=%x\n", hdr->magic, CLS_IMG_MAGIC);
		return -1;
	}
	printf("hdr magic=%x pass!!!\n", hdr->magic);

	hdr->hdr_crc = 0;
	new_crc = crc32(0, (void *)hdr, sizeof(cls_hdr));
	if (crc != new_crc) {
		printf("wrong hdr crc=%x, expect=%x\n", new_crc, crc);
		return -2;
	}
	printf("hdr crc=%x pass!!!\n", new_crc);

	return 0;
}

int check_cls_img_valid(FILE *fp, uint32_t hdr_crc)
{
	int len = 0;
	char buffer[1024];
	int i = 0;
	uint32_t crc, total_crc;
	int32_t total_read = 0, total_len, last_block;
	cls_hdr hdr;

	fseek(fp, 0, SEEK_SET);
	len = fread(&hdr, 1, sizeof(cls_hdr), fp);
	total_crc = hdr.total_crc;
	total_len = hdr.total_len;

	if (len == sizeof(cls_hdr)) {
		/*set to 0 for crc check*/
		hdr.hdr_crc = 0;
		hdr.total_crc = 0;
		hdr.total_len = 0;
	}

	crc = crc32(0, (void *)&hdr, sizeof(cls_hdr)); // clean hdr crc=0
	/*remove cls_hdr size*/
	total_len -= sizeof(cls_hdr);
	while ((len = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {
		total_read += len;
		if (total_read > total_len) {
			last_block = total_len - i*sizeof(buffer);
			printf("last block=%d\n", last_block);
			/*just over a block*/
			if (last_block != sizeof(buffer)) {
				printf("check last crc=%d\n", last_block);
				crc = crc32(crc, buffer, last_block);
			}
			break;
		}
		if (total_read == total_len)
			printf("just end of file, last block = %d\n", len);
		crc = crc32(crc, buffer, len); // update crc
		i++;
	}

	if (crc == total_crc)
		printf(GREEN "total crc=%x, check pass\n" NONE, crc);
	else {
		printf(RED "total_crc=%x !=%x check fail\n" NONE, crc, total_crc);
		return -1;
	}

	return 0;
}

int extract_subfile(FILE *input_file, int target_type_id)
{
	img_tlv tlv_hdr;
	int crc = 0;

	while (fread(&tlv_hdr, sizeof(img_tlv), 1, input_file) == 1) {
		//check subimg magic first
		if (tlv_hdr.magic != TLV_MAGIC) {
			printf("bad tlv_hdr magic =%x not match %x\n", tlv_hdr.type, TLV_MAGIC);
			return -1;
		}
		if (tlv_hdr.type == target_type_id) {
			char buffer[1024];
			size_t remaining_bytes = tlv_hdr.len;
			char sub_name[128];
			FILE *output_file;

			snprintf(sub_name, sizeof(sub_name), "/tmp/split_img_dir/%s",
					image_type_str[target_type_id].name);
			output_file = fopen(sub_name, "wb");

			if (output_file == NULL) {
				perror("Error creating output file");
				exit(EXIT_FAILURE);
			}

			while (remaining_bytes > 0) {
				size_t bytes_to_read = (remaining_bytes < sizeof(buffer)) ? remaining_bytes : sizeof(buffer);
				fread(buffer, 1, bytes_to_read, input_file);
				fwrite(buffer, 1, bytes_to_read, output_file);
				remaining_bytes -= bytes_to_read;
			}

			fclose(output_file);

			crc = crc32_file(sub_name);
			if (crc == tlv_hdr.crc) {
				printf(GREEN "split %s crc pass=%x\n" NONE, sub_name, crc);
			} else {
				printf(RED "split %s crc error=%x, remove it\n" NONE, sub_name, crc);
				remove(sub_name);
				return -2;
			}

			printf("subfile %s extract done\n", image_type_str[target_type_id].name);
			return 0;
		} else {
			//skip none matched subimg
			fseek(input_file, tlv_hdr.len, SEEK_CUR);
		}
	}

	printf("no script or info found\n");
	return -3;
}

int remove_img_hdr(char *img_name, int size)
{
	FILE *new_file, *img;
	char *tmp = "tmp_file";
	unsigned char buffer[1024];
	int len;

	new_file = fopen(tmp, "wb");
	if (!new_file)
		return -1;

	img = fopen(img_name, "rr");
	if (!img)
		return -2;

	fseek(img, size, SEEK_SET);

	// read 2nd file and write to 1st file
	while ((len = fread(buffer, sizeof(char), sizeof(buffer), img)) > 0)
		fwrite(buffer, sizeof(char), len, new_file);

	fclose(img);
	fclose(new_file);

	remove(img_name);
	rename(tmp, img_name);
}

int split_each_single_img(char *img_name)
{
	img_tlv tlv_hdr;
	char buffer[1024];
	FILE *img;
	FILE *new_file;
	uint32_t file_len, total_read = 0, total_len, more_copy, last_copy, len, crc;
	char *new_file_name;
	char full_path[128] = {0};
	int i = 0;

	img = fopen(img_name, "r");
	if (!img) {
		printf("image_name:%s not found\n", img_name);
		return -1;
	}

	fread(&tlv_hdr, 1, sizeof(img_tlv), img);

	if (tlv_hdr.magic != TLV_MAGIC) {
		printf(RED "TLV magic error=%x\n" NONE, tlv_hdr.magic);
		return -1;
	}

	if (tlv_hdr.type == end) {
		printf("split to the end!! quit\n");
		/*end of img*/
		return 0;
	}

	new_file_name = image_type_str[tlv_hdr.type].name;

	/*generate full path of img*/
	strcat(full_path, dirname);
	strcat(full_path, "/");
	strcat(full_path, new_file_name);

	new_file = fopen(full_path, "w");
	if (!new_file)
		printf(RED "new file %s open error\n" NONE, image_type_str[tlv_hdr.type].name);

	file_len = tlv_hdr.len;
#if DEBUG
	printf(GREEN "new file len=%d old crc=%x\n" NONE, file_len, tlv_hdr.crc);
#endif

	// read 2nd file and write to 1st file
	while ((len = fread(buffer, sizeof(char), sizeof(buffer), img)) > 0) {
		total_read += len;
		if (total_read > file_len) {
			more_copy = total_read - file_len;
			/*whole buffer more, just break*/
			if (more_copy == sizeof(buffer))
				break;
			/*most case: 1 buffer include image*/
			if (len == sizeof(buffer)) {
				last_copy = sizeof(buffer) - more_copy;
			} else {
				/*endof file*/
				last_copy = file_len - sizeof(buffer)*i;
			}
#if DEBUG
			printf(RED"last write=%d total_read=%d\n" NONE, last_copy, total_read);
			printf(RED"file_len=%d last_whole_pg=%ld\n" NONE, file_len, sizeof(buffer)*i);
#endif
			fwrite(buffer, sizeof(char), last_copy, new_file);
			break;
		}

		i++;
		fwrite(buffer, sizeof(char), len, new_file);
	}

	total_len = (sizeof(img_tlv) + file_len);

	fclose(new_file);
	fclose(img);

	crc = crc32_file(full_path);
	if (crc == tlv_hdr.crc) {
		printf(GREEN "split %s crc pass=%x\n" NONE, full_path, crc);
	} else {
		printf(RED "split %s crc error=%x, remove it\n" NONE, full_path, crc);
		remove(full_path);
		return -2;
	}

	remove_img_hdr(global_img_name, total_len);

	if (tlv_hdr.type == script_gz) {
		run_script = 1;
		system("mv split_img_dir/script_gz split_img_dir/script.sh.gz;"
		       "gunzip split_img_dir/script.sh.gz;"
		       "chmod +x split_img_dir/script.sh");
	} else if (tlv_hdr.type == info_gz) {
		system("mv split_img_dir/info_gz split_img_dir/image-info.gz;"
		       "gunzip split_img_dir/image-info.gz;");
	}

	return total_len;
}

int print_help(void)
{
	fprintf(stderr, "Usage: checkclsimg:\n"
                        "\t\t[-c] filename checkimage sanity\n"
                        "\t\t[-x] extract info and script\n"
                        "\t\t[-h] print help\n");
}

int main(int argc, char *argv[])
{
	FILE *file;
	cls_hdr hdr;
	char *img_name;
	unsigned char buffer[32];
	int len, total_len = 0;
	int opt, last_opt=0;
	int option_index = 0;
	char *string = "c:x:h";
	int only_do_check = 0, extract_script_info = 0;

	static struct option long_options[] = {
		{"check", required_argument, NULL, 'c'},
		{"extract", required_argument, NULL, 'x'},
		{"help", required_argument, NULL, 'h'},
		{NULL, 0, NULL, 0},
	};

	if (argc == 1) {
		print_help();
		exit(0);
	}
	/*get image name first*/
	while ((opt = getopt_long(argc, argv, string, long_options, &option_index)) != -1) {
		switch (opt) {
		case 'c':
			only_do_check = 1;
			printf("only do img check\n");
			break;
		case 'x':
			extract_script_info = 1;
			/*keep img not change, inly extract one file = extract_file_name*/
			printf("extract_script_info = 1\n");
			break;
		case 'h':
		default:
			print_help();
			exit(EXIT_FAILURE);
			break;
		}
	}

	if(only_do_check || extract_script_info)
		last_opt = optind-1;
	else
		last_opt = optind;

	img_name = argv[last_opt];

	/*change work dir to /tmp */
	chdir("/tmp");

	system("rm split_img_dir -rf");
	if (mkdir(dirname, 0755)) {
		printf("Failed to create directory: %s\n", strerror(errno));
		exit(0);
	}
	printf("Directory created successfully.\n");

	global_img_name = img_name;
	printf("image name=%s\n", img_name);

	file = fopen(img_name, "rb");
	if (file == NULL) {
		printf("cannot open '%s': %s\n", img_name, strerror(errno));
		return 1;
	}

	fread(&hdr, sizeof(cls_hdr), 1, file);

	if (check_cls_hdr_valid(&hdr) < 0) {
		printf("error cls hdr\n");
		return -1;
	}

	if (check_cls_img_valid(file, hdr.hdr_crc) < 0) {
		printf("cls img format errror\n");
		return -2;
	}

	print_cls_hdr_info(&hdr);

	/*return 0 after check img header and internal crc*/
	if (only_do_check) {
		printf("cls img format check pass!\n");
		return 0;
	}

	if (extract_script_info) {
		fseek(file, sizeof(cls_hdr), SEEK_SET);
		if(!extract_subfile(file, script_gz))
			system("mv /tmp/split_img_dir/script_gz /tmp/split_img_dir/script.sh.gz;"
				"gunzip /tmp/split_img_dir/script.sh.gz;"
				"chmod +x /tmp/split_img_dir/script.sh");

		fseek(file, sizeof(cls_hdr), SEEK_SET);
		if(!extract_subfile(file, info_gz))
			system("mv /tmp/split_img_dir/info_gz /tmp/split_img_dir/image-info.gz;"
				"gunzip /tmp/split_img_dir/image-info.gz;");

		fclose(file);
		return 0;
	}

	/*skip first cl hdr*/
	remove_img_hdr(img_name, sizeof(cls_hdr));

	do {
		len = split_each_single_img(img_name);
		if (len < 0) {
			printf("split error\n");
			goto bail;
		}

		if (len == 0)
			break;

		total_len += len;
		if (total_len > hdr.total_len)
			goto bail;

	} while (1);

	fclose(file);
	remove(img_name);

	if (run_script) {
		printf(YELLOW "run image upgrade script!!\n" NONE);
		printf(NONE "\n" NONE);
		system("sh split_img_dir/script.sh");
	}

	return total_len;

bail:
	return -1;
}
