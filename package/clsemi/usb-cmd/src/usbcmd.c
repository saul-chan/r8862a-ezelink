#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define USB_PHY_APB2CRP_BASE   0x90080000
#define USB_LOOK_BACK_REG      0x1000
#define USB_TX_PRBS_REG        0x1015
#define USB_RX_PRBS_REG        0x1016
#define USB_RX_ERR_REG         0x1017

#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))

#define _writel(a, v) (*REG32(a) = (v))
#define _readl(a) (*REG32(a))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~((((uint32_t)1 << (width)) - 1) << (startbit))) | ((val) << (startbit))

typedef enum {
	READ = 0,
	WRITE = 1,
} RW_MODE_E;

static int usb_register_rw_cmd(RW_MODE_E type, uint32_t reg_addr, uint32_t writeval, uint32_t *readval)
{
	void *map_base = NULL;
	void *virt_addr = NULL;
	uint32_t page_size, mapped_size, offset_in_page;
	int fd;

	fd = open("/dev/mem", (O_RDWR | O_SYNC), 0666);
	if (fd < 0) {
		perror("open(/dev/mem) failed.");
		exit(-1);
	}

	mapped_size = page_size = getpagesize();
	offset_in_page = (unsigned)reg_addr & (page_size - 1);
	map_base = mmap(NULL,
			mapped_size,
			(PROT_READ | PROT_WRITE),
			MAP_SHARED,
			fd,
			reg_addr & ~(uint32_t)(page_size - 1));

	if (map_base == MAP_FAILED) {
		perror("usb register mmap\n");
		goto ERROR;
	}

	virt_addr = (char*)map_base + offset_in_page;
	if (type == READ) {
		if (readval == NULL) {
			perror("Invalid usb readval point\n");
			return -1;
		}
		*readval = _readl(virt_addr);
	} else if (type == WRITE) {
		_writel(virt_addr, writeval);
	} else {
		perror("Invalid usb RW instruction\n");
		goto ERROR;
	}

	if (munmap(map_base, mapped_size) == -1) {
		perror("usb register munmap fail\n");
		exit(-1);
	}

	close(fd);
	return 0;

ERROR:
	if (fd >= 0) {
		close(fd);
	}

	if (map_base != NULL) {
		if (munmap(map_base, mapped_size) == -1) {
			perror("usb register munmap fail\n");
			exit(-1);
		}
	}

	return -1;
}

static int usb_phy_reg_write(uint32_t reg_addr, uint32_t reg_val)
{
	int ret = -1;
	uint32_t start_busy_status = 1;

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x14, 0xffff, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x14);
	}

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x4, reg_addr, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x04);
	}

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x8, reg_val, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x08);
	}

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x10, 1, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x10);
	}

	do {
		ret = usb_register_rw_cmd(READ, USB_PHY_APB2CRP_BASE, 0, &start_busy_status);
		if (ret < 0) {
			printf("usb register read register error, register is 0x%x\n", 0x0);
		}
	} while (!(start_busy_status & (1 << 0)));

	return 0;
}

static uint32_t usb_phy_reg_read(uint32_t reg_addr)
{
	int ret = -1;
	uint32_t reg_val;
	uint32_t start_busy_status = 0;

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x14, 0xffff, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x14);
	}

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x4, reg_addr, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x04);
	}

	ret = usb_register_rw_cmd(WRITE, USB_PHY_APB2CRP_BASE + 0x10, 0x2, NULL);
	if (ret < 0) {
		printf("usb register write register error, register is 0x%x\n", 0x10);
	}

	do {
		ret = usb_register_rw_cmd(READ, USB_PHY_APB2CRP_BASE, 0, &start_busy_status);
		if (ret < 0) {
			printf("usb register read register error, register is 0x%x\n", 0x0);
		}
	} while (!(start_busy_status & (1 << 1)));
	ret = usb_register_rw_cmd(READ, USB_PHY_APB2CRP_BASE + 0xC, 0, &reg_val);
	if (ret < 0) {
		printf("usb register read register error, register is 0x%x\n", 0xC);
	}
	return reg_val;
}

/* xpcs_num: 0---serdes0; 1---serdes1;
 * mode: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS15, 4---PRBS7, 5---PAT0, 6---(PAT0,~PAT0), 7---(000,PAT0,3ff,~PAT0), 8--USB3.0 TSEQ, 9-15--RESERVED;
 * pat0: user defined pattern
 */
static int usb_tx_prbs_enable_config(uint32_t mode, uint32_t pat0)
{
	uint32_t reg_val;
	int ret = -1;

	if(mode < 9) {
		usb_phy_reg_read(USB_TX_PRBS_REG);
		RMWREG32(&reg_val, 0, 4, mode);
		RMWREG32(&reg_val, 5, 10, pat0);
		usb_phy_reg_write(USB_TX_PRBS_REG, reg_val);
	} else {
		perror("Invalid TX PRBS MODE\n");
		return -1;
	}

	return 0;
}

static void usb_tx_err_insert(void)
{
	uint32_t reg_val;

	reg_val = usb_phy_reg_read(USB_TX_PRBS_REG);
	RMWREG32(&reg_val, 4, 1, 1);
	usb_phy_reg_write(USB_TX_PRBS_REG, reg_val);
	RMWREG32(&reg_val, 4, 1, 0);
	usb_phy_reg_write(USB_TX_PRBS_REG, reg_val);
}

/*
 * prbs_num: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS15-2, 4---PRBS7, 5---d[n]=d[n-10], 6---d[n]=!d[n-10], 7---d[n]=!d[n-20], 8--USB3.0 TESQ
 */
static int usb_rx_prbs_enable_config(uint32_t mode)
{
	uint32_t reg_val;

	if(mode < 9) {
		reg_val = usb_phy_reg_read(USB_RX_PRBS_REG);
		RMWREG32(&reg_val, 0, 4, mode);
		RMWREG32(&reg_val, 4, 1, 1);
		usb_phy_reg_write(USB_RX_PRBS_REG, reg_val);
	} else {
		perror("Invalid RX PRBS MODE\n");
		return -1;
	}

	return 0;
}

static void usb_look_back_en(uint32_t mode)
{
	uint32_t reg_val;

	reg_val = usb_phy_reg_read(USB_LOOK_BACK_REG);
	if (mode == 0) {
		RMWREG32(&reg_val, 0, 2, 0);
	} else if (mode == 1) {
		RMWREG32(&reg_val, 0, 2, 3);
	} else {
		printf("Invalid usb look back mode parameters, mode = %x\n", mode);
	}
	usb_phy_reg_write(USB_LOOK_BACK_REG, reg_val);
	printf("reg = 0x%x, data = 0x%x\n", USB_LOOK_BACK_REG, usb_phy_reg_read(USB_LOOK_BACK_REG));
}

static int usb_rx_err_count(void)
{
	return usb_phy_reg_read(USB_RX_ERR_REG);
}

static void usb_hw_init(void)
{
	usb_phy_reg_write(0x1018, 0x0aa8);
	printf("reg = 0x%x, data = 0x%x, EXPECTED DATA=0x0aa8\n", 0x1018, usb_phy_reg_read(0x1018));
	usb_phy_reg_write(0x1018, 0x0550);
	printf("reg = 0x%x, data = 0x%x, EXPECTED DATA=0x0550\n", 0x1018, usb_phy_reg_read(0x1018));
	usb_phy_reg_write(0x0011, 0x004c);
	usb_phy_reg_write(0x0011, 0x084c);
	usb_phy_reg_write(0x1001, 0x0040);
	usb_phy_reg_write(0x1001, 0x00c0);
	usb_phy_reg_write(0x1006, 0x1000);
	usb_phy_reg_write(0x1006, 0x3000);
	usb_phy_reg_write(0x0011, 0x084e);
	usb_phy_reg_write(0x1000, 0x0200);
	usb_phy_reg_write(0x1000, 0x0280);
	usb_phy_reg_write(0x1005, 0x0008);
	usb_phy_reg_write(0x1005, 0x0028);
	usb_phy_reg_write(0x1000, 0x02a0);
	usb_phy_reg_write(0x0010, 0x0001);
	usb_phy_reg_write(0x0010, 0x0003);
	usb_phy_reg_write(0x0010, 0x0007);
	usb_phy_reg_write(0x0015, 0x0000);
	usb_phy_reg_write(0x0015, 0x1000);
	usb_phy_reg_write(0x0012, 0x0140);
	usb_phy_reg_write(0x0012, 0x0340);
	usb_phy_reg_write(0x0011, 0x0002);
	usb_phy_reg_write(0x0011, 0x0a02);
	usb_phy_reg_write(0x0013, 0x0100);
	usb_phy_reg_write(0x0013, 0x0100);
	usb_phy_reg_write(0x0013, 0x2100);
	usb_phy_reg_write(0x1001, 0x00c0);
	usb_phy_reg_write(0x1001, 0x00c4);
	usb_phy_reg_write(0x1006, 0x3000);
	usb_phy_reg_write(0x1006, 0x3004);
	usb_phy_reg_write(0x0012, 0x8200);
	usb_phy_reg_write(0x0015, 0xb000);
	usb_phy_reg_write(0x0015, 0xb009);
	usb_phy_reg_write(0x0015, 0xb029);
	usb_phy_reg_write(0x0015, 0xb429);
	usb_phy_reg_write(0x1000, 0x02a0);
	usb_phy_reg_write(0x1000, 0x02a2);
	usb_phy_reg_write(0x1002, 0x007f);
	usb_phy_reg_write(0x1002, 0x11ff);
	usb_phy_reg_write(0x1002, 0x51ff);
	usb_phy_reg_write(0x1003, 0x0000);
	usb_phy_reg_write(0x1003, 0x0020);
	usb_phy_reg_write(0x0003, 0x0010);
	usb_phy_reg_write(0x0003, 0x0012);
	usb_phy_reg_write(0x0003, 0x0010);
	usb_phy_reg_write(0x1001, 0x01c4);
	usb_phy_reg_write(0x1001, 0x03c4);
	usb_phy_reg_write(0x1000, 0x02a2);
	usb_phy_reg_write(0x1000, 0x0aa2);
	usb_phy_reg_write(0x1000, 0x0aa2);
	usb_phy_reg_write(0x1000, 0x2aa2);
	usb_phy_reg_write(0x1000, 0x2aa2);
	usb_phy_reg_write(0x1000, 0x2aaa);
	usb_phy_reg_write(0x1006, 0x3004);
	usb_phy_reg_write(0x1006, 0x3084);
	usb_phy_reg_write(0x1006, 0x3484);
	usb_phy_reg_write(0x1006, 0x3c84);
	usb_phy_reg_write(0x1006, 0x3c84);
	usb_phy_reg_write(0x1006, 0x3ca4);
	usb_phy_reg_write(0x1005, 0x0028);
	usb_phy_reg_write(0x1005, 0x0228);
	usb_phy_reg_write(0x1005, 0x0228);
	usb_phy_reg_write(0x1005, 0x022a);
	usb_phy_reg_write(0x1005, 0x122a);
	usb_phy_reg_write(0x1005, 0x322a);
	usb_phy_reg_write(0x1005, 0x326a);
	usb_phy_reg_write(0x1005, 0x32ea);
	usb_phy_reg_write(0x1005, 0x36ea);
	usb_phy_reg_write(0x1005, 0x3eea);
	usb_phy_reg_write(0x0012, 0x8600);
	usb_phy_reg_write(0x0012, 0x8200);
	usb_phy_reg_write(0x0011, 0x0ba3);
	usleep(30);
	usb_phy_reg_write(0x1015, 0x03e0);
	usb_phy_reg_write(0x1015, 0x03e6);
	usb_phy_reg_write(0x1001, 0x0384);
	usb_phy_reg_write(0x1006, 0x2ca4);
	usb_phy_reg_write(0x1000, 0x2baa);
	usleep(330);
	usb_phy_reg_write(0x1000, 0x2bea);
	usb_phy_reg_write(0x1005, 0x3eee);
	usleep(10);
	usb_phy_reg_write(0x1005, 0x3efe);
	usleep(10);
	usb_phy_reg_write(0x1000, 0x2bfa);
	printf("reg = 0x%x, data = 0x%x, EXPECTED DATA=0x020b\n", 0x101c, usb_phy_reg_read(0x101c));
	usb_phy_reg_write(0x1005, 0x3ebe);
	usb_phy_reg_write(0x1015, 0x25c6);
	usb_phy_reg_write(0x1015, 0x25c4);
	usb_phy_reg_write(0x1016, 0x0004);
	usb_phy_reg_write(0x1016, 0x0004);
	//inset errors
	usb_phy_reg_write(0x1016, 0x0014);
	usb_phy_reg_write(0x1016, 0x0004);
	usb_phy_reg_write(0x1016, 0x0014);
	usb_phy_reg_write(0x1016, 0x0004);
	usleep(600);
	printf("data = %x, EXPECTED DATA=0x0000\n", usb_phy_reg_read(0x1017));
	usb_phy_reg_write(0x1015, 0x25d4);
	usb_phy_reg_write(0x1015, 0x25c4);
	printf("data = %x, EXPECTED DATA=0x0001\n", usb_phy_reg_read(0x1017));
	usleep(600);
	usleep(1000);
}

int main(int argc, char **argv, char * envp[])
{
	char *cmd = NULL;
	uint32_t reg_addr, reg_val;

	cmd = strrchr(argv[0], '/');
	cmd = cmd == NULL ? argv[0] : &cmd[1];

	if (!strcasecmp("USB_RD_Register", cmd)) {
		reg_addr = strtol(argv[1], NULL, 16);
		printf("phy offset address:0x%x, value = 0x%x\n", reg_addr, usb_phy_reg_read(reg_addr));
	}
	else if (!strcasecmp("USB_HW_Config", cmd)) {
		printf("just into configure phy, nothing todo!!\n");
		usb_hw_init();
	} else if (!strcasecmp("USB_TX_PRBS_Config", cmd)) {
		reg_addr = strtol(argv[1], NULL, 16);
		reg_val = strtol(argv[2], NULL, 16);
		usb_tx_prbs_enable_config(reg_addr, reg_val);
	} else if (!strcasecmp("USB_TX_err_Insert", cmd)) {
		usb_tx_err_insert();
	} else if (!strcasecmp("USB_RX_PRBS_Config", cmd)) {
		reg_addr = strtol(argv[1], NULL, 16);
		usb_rx_prbs_enable_config(reg_addr);
	} else if (!strcasecmp("USB_RX_err_count", cmd)) {
		printf("usb rx error count number is 0x%x\n", usb_rx_err_count());
	} else if (!strcasecmp("USB_LOOPBACK_EN", cmd)) {
		reg_val = strtol(argv[1], NULL, 16);
		usb_look_back_en(reg_val);
	} else {
		perror("please check the command argument\n");
	}

	return 0;
}
