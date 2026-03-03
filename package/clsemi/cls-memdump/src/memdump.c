#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "memdump.h"

static int fd_sys;

int is_xdigital(char *str)
{
	int i;

	if (memcmp(str, "0x", 2) && memcmp(str, "0X", 2))
		return 0;

	for(i = 2; i < strlen(str); i++) {
		if (!isxdigit(str[i]))
			return 0;
	}

	return 1;
}

int is_digital(char *str)
{
	int i;

	for(i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i]))
			return 0;
	}

	return 1;
}

void help(void)
{
	printf("Usage:\n");
	printf("memdump [addr] [len] [format] [endian] [print_addr] [file_name]\n");
	printf("addr: the address to dump\n");
	printf("len: the sample number to dump\n");
	printf("format: hex/bin/hex16 in string\n");
	printf("endian: 0 -- little endian 1 -- big endian\n");
	printf("print_addr: 0 -- not print addr 1 -- print addr\n");
	printf("file_name: it's optional, if set, save contents to this file\n");
}

int main(int argc,char *argv[])
{
	uint32_t addr, len, input_addr;
	uint32_t endian = 0;
	void *map_addr = NULL;
	void *map_addr_orig = NULL;
	char *format = NULL;
	char *file_name = NULL;
	uint32_t i;
	uint32_t format_index;
	uint32_t step;
	char file_option[FILE_OPTION_SIZE];
	FILE *fp = NULL;
	uint32_t tmp;
	uint32_t offset;
	uint32_t print_addr = 0;

	if (argc < 6 || argc > 7) {
		help();
		return 0;
	}

	if (!is_xdigital(argv[1]) && !is_digital(argv[1])) {
		printf("Wrong input for addr!\n");
		help();
		return 0;
	}

	if (!is_digital(argv[2])) {
		printf("Wrong input for len!\n");
		help();
		return 0;
	}

	if (!is_digital(argv[5])) {
		printf("Wrong input for print_addr!\n");
		help();
		return 0;
	}

	addr = strtoul(argv[1], NULL, 0);
	len  = strtoul(argv[2], NULL, 0);
	format = argv[3];
	endian = strtoul(argv[4], NULL, 0);
	print_addr = strtoul(argv[5], NULL, 0);

	if (BIG_ENDIAN_MODE != endian && LITTLE_ENDIAN_MODE != endian) {
		printf("endian not support!\n");
		return 0;
	}

	if (!strcasecmp(format, "hex")) {
		format_index = FORMAT_HEX32;
		step = sizeof(uint32_t);
		strcpy(file_option, "w+");
	} else if (!strcasecmp(format, "bin")) {
		format_index = FORMAT_BIN32;
		step = sizeof(uint32_t);
		strcpy(file_option, "wb+");
	} else if (!strcasecmp(format, "hex16")) {
		format_index = FORMAT_HEX16;
		step = sizeof(uint16_t);
		strcpy(file_option, "w+");
	} else {
		printf("format not support!\n");
		return 0;
	}

	len *= step;

	if (argc == 7) {
		file_name = argv[6];
		fp = fopen(file_name, file_option);
		if (fp == NULL) {
			printf("open %s failed.\n", file_name);
			printf("%s\n", strerror(errno));
			return 0;
		}
	}

	fd_sys = open("/dev/mem", O_RDWR | O_NDELAY);

	if (fd_sys < 0) {
		printf("open /dev/mem failed.\n");
		printf("%s\n", strerror(errno));
		return 0;
	}

	input_addr = addr;
	offset = addr % MMAP_SIZE;
	addr -= offset;

	map_addr = mmap(NULL, len + offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd_sys, addr);
	if (map_addr == MAP_FAILED) {
		printf("mmap failed: %s\n", strerror(errno));
		if(fd_sys)
			close(fd_sys);
		if (fp)
			fclose(fp);
		return 0;
	}

	map_addr_orig = map_addr;
	map_addr += offset;

	if (file_name) {
		for(i = 0; i < len; i = i + step) {
			if (FORMAT_HEX32 == format_index) {
				if (BIG_ENDIAN_MODE == endian) {
					if (PRINT_ADDR == print_addr)
						fprintf(fp, "%08x %08x\n", input_addr + i, htonl(*(volatile uint32_t *)(map_addr + i)));
					else
						fprintf(fp, "%08x\n", htonl(*(volatile uint32_t *)(map_addr + i)));
				} else {
					if (PRINT_ADDR == print_addr)
						fprintf(fp, "%08x %08x\n", input_addr + i, *(volatile uint32_t *)(map_addr + i));
					else
						fprintf(fp, "%08x\n", *(volatile uint32_t *)(map_addr + i));
				}
			} else if (FORMAT_BIN32 == format_index) {
				if (BIG_ENDIAN_MODE == endian)
					tmp = htonl(*(volatile uint32_t *)(map_addr + i));
				else
					tmp = *(volatile uint32_t *)(map_addr + i);
				fwrite(&tmp, sizeof(uint32_t), 1, fp);
			} else {
				if (BIG_ENDIAN_MODE == endian) {
					/* This is a specific usage for AGC state dump */
					if (i % 4 == 0)
						fprintf(fp, "%04x\n", *(volatile uint16_t *)(map_addr + i + sizeof(uint16_t)));
					else
						fprintf(fp, "%04x\n", *(volatile uint16_t *)(map_addr + i - sizeof(uint16_t)));
				} else {
					fprintf(fp, "%04x\n", *(volatile uint16_t *)(map_addr + i));
				}
			}
		}
	} else {
		for(i = 0; i < len; i = i + step) {
			if (FORMAT_HEX32 == format_index || FORMAT_BIN32 == format_index) {
				if (BIG_ENDIAN_MODE == endian) {
					if (PRINT_ADDR == print_addr)
						printf("%08x %08x\n", input_addr + i, htonl(*(volatile uint32_t *)(map_addr + i)));
					else
						printf("%08x\n", htonl(*(volatile uint32_t *)(map_addr + i)));
				} else {
					if (PRINT_ADDR == print_addr)
						printf("%08x %08x\n", input_addr + i, *(volatile uint32_t *)(map_addr + i));
					else
						printf("%08x\n", *(volatile uint32_t *)(map_addr + i));
				}
			} else {
				if (BIG_ENDIAN_MODE == endian) {
					/* This is a specific usage for AGC state dump */
					if (i % 4 == 0)
						printf("%04x\n", *(volatile uint16_t *)(map_addr + i + sizeof(uint16_t)));
					else
						printf("%04x\n", *(volatile uint16_t *)(map_addr + i - sizeof(uint16_t)));
				} else {
					printf("%04x\n", *(volatile uint16_t *)(map_addr + i));
				}
			}
		}
	}

	if(fd_sys)
		close(fd_sys);

	if (fp)
		fclose(fp);

	munmap(map_addr_orig, len + offset);
	return 0;
}
