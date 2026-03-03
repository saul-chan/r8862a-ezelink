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
#include "memload.h"

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

void copy_bin_file_to_mem(FILE *fp, uint32_t *dst_addr, uint32_t cnt, uint32_t endian)
{
	uint32_t i;
	uint32_t data;
	cnt /= sizeof(uint32_t);

	fseek(fp, 0, SEEK_SET);

	for (i = 0; i < cnt; i++) {
		fread(&data, sizeof(data), 1, fp);
		if (BIG_ENDIAN_MODE == endian)
			dst_addr[i] = ntohl(data);
		else
			dst_addr[i] = data;
	}
}

void copy_hex_file_to_mem(FILE *fp, uint32_t *dst_addr, uint32_t endian)
{
	char *line = NULL;
	uint32_t data;
	size_t len = 0;
	uint32_t i = 0;

	fseek(fp, 0, SEEK_SET);

	while(getline(&line, &len, fp) != -1) {
		data = strtoul(line, NULL, 16);
		if (BIG_ENDIAN_MODE == endian)
			dst_addr[i++] = ntohl(data);
		else
			dst_addr[i++] = data;
	}

	if (line)
		free(line);
}

uint32_t get_len_from_bin_file(FILE *fp)
{
	uint32_t cnt = 0;
	char ch;

	while(!feof(fp)) {
		ch = getc(fp);
		cnt++;
	}

	cnt -= (cnt % sizeof(uint32_t));
	return cnt;
}

uint32_t get_len_from_hex_file(FILE *fp)
{
	uint32_t cnt = 0;
	size_t len = 0;
	char *line = NULL;

	while(getline(&line, &len, fp) != -1)
		cnt++;

	if (line)
		free(line);

	cnt *= sizeof(uint32_t);
	return cnt;
}

void help(void)
{
	printf("Usage:\n");
	printf("memload [addr] [format] [file_name] [endian]\n");
	printf("addr: the address to dump\n");
	printf("format: hex/bin in string\n");
	printf("file_name: the file name to load\n");
	printf("endian: it's optional, 0 -- little endian 1 -- big endian, default 0\n");
}

int main(int argc,char *argv[])
{
	uint32_t addr;
	uint32_t endian = 0;
	void *map_addr = NULL;
	void *map_addr_orig = NULL;
	char *format = NULL;
	char *file_name = NULL;
	uint32_t i;
	uint32_t format_index;
	char file_option[FILE_OPTION_SIZE];
	FILE *fp = NULL;
	uint32_t len;
	uint32_t offset;

	if (argc < 4 || argc > 5) {
		help();
		return 0;
	}

	if (!is_xdigital(argv[1]) && !is_digital(argv[1])) {
		printf("Wrong input for addr!\n");
		help();
		return 0;
	}

	addr = strtoul(argv[1], NULL, 0);
	format = argv[2];
	if (argc == 5)
		endian = strtoul(argv[4], NULL, 0);

	if (BIG_ENDIAN_MODE != endian && LITTLE_ENDIAN_MODE != endian) {
		printf("endian not support!\n");
		return 0;
	}

	if (!strcasecmp(format, "hex")) {
		format_index = FORMAT_HEX32;
		strcpy(file_option, "r+");
	} else if (!strcasecmp(format, "bin")) {
		format_index = FORMAT_BIN32;
		strcpy(file_option, "rb+");
	} else {
		printf("format not support!\n");
		return 0;
	}

	file_name = argv[3];
	fp = fopen(file_name, file_option);
	if (fp == NULL) {
		printf("open %s failed.\n", file_name);
		printf("%s\n", strerror(errno));
		return 0;
	}

	if (format_index == FORMAT_HEX32)
		len = get_len_from_hex_file(fp);
	else
		len = get_len_from_bin_file(fp);

	fd_sys = open("/dev/mem", O_RDWR | O_NDELAY);

	if (fd_sys < 0) {
		printf("open /dev/mem failed.\n");
		printf("%s\n", strerror(errno));
		return 0;
	}

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
		if (format_index == FORMAT_HEX32)
			copy_hex_file_to_mem(fp, map_addr, endian);
		else
			copy_bin_file_to_mem(fp, map_addr, len, endian);
	}

	if(fd_sys)
		close(fd_sys);

	if (fp)
		fclose(fp);

	munmap(map_addr_orig, len + offset);
	return 0;
}
