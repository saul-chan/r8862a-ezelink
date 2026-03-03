#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

void sleep_nano(uint32_t nsec)  
{
	struct timespec  time = {0}, rem = {0};
	time.tv_nsec = nsec; 
	nanosleep(&time, &rem);
}

uint64_t  timer_get_us(void)
{
    uint64_t us = 0;  
	struct timeval tv;
    gettimeofday(&tv, NULL);
    us = tv.tv_sec * 1000000 + tv.tv_usec;
    return us;
}
uint32_t hal_soc_read(off_t target)
{
	void *map_base, *virt_addr;
	uint64_t read_result;
	unsigned page_size, mapped_size, offset_in_page;
	int fd;
	unsigned width = 8 * sizeof(int);

	fd = open("/dev/mem", (O_RDONLY | O_SYNC), 0666);

	mapped_size = page_size = getpagesize();
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + width > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size *= 2;
	}
	map_base = mmap(NULL,
			mapped_size,
			PROT_READ,
			MAP_SHARED,
			fd,
			target & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	virt_addr = (char*)map_base + offset_in_page;

	switch (width) {
	case 8:
		read_result = *(volatile uint8_t*)virt_addr;
		break;
	case 16:
		read_result = *(volatile uint16_t*)virt_addr;
		break;
	case 32:
		read_result = *(volatile uint32_t*)virt_addr;
		break;
	case 64:
		read_result = *(volatile uint64_t*)virt_addr;
		break;
	default:
		printf("bad width\n");
		break;
	}
	//printf("0x%0*llX\n", (width >> 2), (unsigned long long)read_result);
	if (munmap(map_base, mapped_size) == -1)
		perror("munmap");

	close(fd);

    usleep(500);
	return read_result;
}

int hal_soc_write(off_t target, uint32_t writeval)
{
	void *map_base, *virt_addr;
	uint64_t read_result;
	unsigned page_size, mapped_size, offset_in_page;
	int fd;
	unsigned width = 8 * sizeof(int);

	fd = open("/dev/mem", (O_RDWR | O_SYNC), 0666);
	mapped_size = page_size = getpagesize();
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + width > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size *= 2;
	}
	map_base = mmap(NULL,
			mapped_size,
			(PROT_READ | PROT_WRITE),
			MAP_SHARED,
			fd,
			target & ~(off_t)(page_size - 1));

	if (map_base == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	//	printf("Memory mapped at address %p.\n", map_base);

	virt_addr = (char*)map_base + offset_in_page;

	switch (width) {
	case 8:
		*(volatile uint8_t*)virt_addr = writeval;
		//			read_result = *(volatile uint8_t*)virt_addr;
		break;
	case 16:
		*(volatile uint16_t*)virt_addr = writeval;
		//			read_result = *(volatile uint16_t*)virt_addr;
		break;
	case 32:
		*(volatile uint32_t*)virt_addr = writeval;
		//			read_result = *(volatile uint32_t*)virt_addr;
		break;
	case 64:
		*(volatile uint64_t*)virt_addr = writeval;
		//			read_result = *(volatile uint64_t*)virt_addr;
		break;
	default:
		printf("bad width\n");
	}

	if (munmap(map_base, mapped_size) == -1)
		perror("munmap");

	close(fd);

    usleep(500);
	return 0;
}

