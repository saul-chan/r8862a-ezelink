#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dubhe1000_xpcs_serdes.h"
#include "dubhe1000_mac_stats.h"
#include "sys_hal.h"
#include "dubhe1000_mac.h"
#include "serdes_fw.h"
#define DEFAULT_TIMEOUT_US    (1000 * 9000)
#define REG_PRINT(base, addr, field)  printf("%40s %25s:     %#x\n", #addr, #field, REG_R(base + addr, field))
extern int g_serdes_fw;
extern int g_debug;
enum {
	USXGMII_EM = 0,
	MP_USXGMII_EM,
	QSGMII_EM,
	HSGMII_EM,
	SGMII_EM
};

enum {
	USXGMII_10G_SXGMII_EM = 0,
	USXGMII_5G_SXGMII_EM,
	USXGMII_2_5G_SXGMII_EM,
	USXGMII_10G_DXGMII_EM,
	USXGMII_5G_DXGMII_EM,
	USXGMII_10G_QXGMII_EM
};

void msleep(int msec)
{
	usleep(msec * 1000);
}

void LOAD_SRAM(int xpcs_num)
{
	int status = 0, val = 0;

	//********************SRAM加载*************************
	LOG("Wait for XPCS0 SRAM initialization to complete ...\n");

	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
				status, status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	status = 0;
	while(status == 0) {											//等待SRAM初始化完成
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
	}
#endif
	//wait 80ns；								//该等待期间可修改加载到SRAM的firmware
	sleep_nano(80);

	if (g_serdes_fw) {
		for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ )
			ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成

	sleep(1);

	if (g_debug) {
		int val = ETH_serdes_reg_read(xpcs_num, 0x1b);
		LOG("==>serdes reg read 0x1b val[%#x]\n", ETH_serdes_reg_read(xpcs_num, 0x1b));

		LOG("before==>serdes reg read 0xe val[%#x]\n", ETH_serdes_reg_read(xpcs_num, 0xe));
	}

	ETH_serdes_reg_write(xpcs_num, 0xe, 0x18);

	if (g_debug)
	LOG("==>serdes reg read 0xe val[%#x]\n", ETH_serdes_reg_read(xpcs_num, 0xe));

	sleep_nano(80);

	do {
		val = ETH_serdes_reg_read(xpcs_num, 0x1b);
		if (g_debug)
			LOG("--==>serdes reg read 0x1b val[%#x]<===\n", (ETH_serdes_reg_read(xpcs_num, 0x1b) & BIT(10)) >> 10);
		sleep(1);
	} while(val & BIT(10));

	ETH_serdes_reg_write(xpcs_num, 0xe, 0x10);

#if 0
	LOG("before==>serdes reg read 0x3003 val[%#x]\n", ETH_serdes_reg_read(xpcs_num, 0x3003));
	ETH_serdes_reg_write(xpcs_num, 0x3003, 0x4);
	LOG("==>serdes reg read 0x3003 val[%#x]\n", ETH_serdes_reg_read(xpcs_num, 0x3003));
	ETH_serdes_reg_write(xpcs_num, 0x300e, 0x2);
#endif
	//********************SRAM加载*************************
	LOG("等待软复位释放，0表示释放\n");
	status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
				status, status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}

#if 0
	status = 1;
	while(status == 1) {											//等待软复位释放，0表示释放
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
	}
#endif

}

void VR_RST(int xpcs_num)
{
	int status;
	if (xpcs_num != 0 && xpcs_num != 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);

	LOAD_SRAM(xpcs_num);
}

//4.1.	Serdes启动
//XPCS0/Serdes0支持协议：
//USXGMII、MP-USXGMII(2port): 10.3125Gbps
//SGMII+: 3.125Gbps
//SGMII: 1.25Gbps
//XPCS1/Serdes1支持协议：
//USXGMII: 10.3125Gbps (实际应用不支持，测试需求);
//QSGMII: 5Gbps
//SGMII+: 3.125Gbps (实际应用不支持，测试需求);
//SGMII: 1.25Gbps
//RGMII2---MDIO1
//RGMII0/Serdes0/Serdes1---MDIO0/MDIO2
//4.1.1.	Serdes启动前准备
void Serdes_init_pre(void)
{
	int status, xgmac_index;

#include "chip_top.h"

	for ( xgmac_index = 1; xgmac_index < 5 ; xgmac_index ++) {
		writel(0x0, 0x53E0000C + (xgmac_index * 16));
	}

	//Serdes启动前依赖FWD SUB解复位：
	REG_W(SERDES0_SRAM_BYPASS, serdes0_sram_bypass, 0);		//加载serdes FW使能
    LOAD_SRAM(0);

	REG_W(SERDES1_SRAM_BYPASS, serdes1_sram_bypass, 0);
    LOAD_SRAM(1);

}

//4.1.2.	Serdes启动程序
//xcps_num=: 0---xpcs0/serdes0; 1--- xcps1/serdes1;
//serdes_mode: 0---USXGMII; 1---MP-USXGMII(2port); 2---QSGMII; 3---SGMII+; 4---SGMII;
//usxg_mode: 0---10G-SXGMII; 1---5G-SXGMII; 2---2.5G-SXGMII; 3---10G-DXGMII; 4---5G-DXGMII; 5---10G-QXGMII
void Serdes_init_proc(int xpcs_num,  int serdes_mode, int usxg_mode, int rxadpt_en)
{
	int status;
	int data_rate;
	int link_speed, duplex_mode, link_status, eee, eee_clk;
	switch (serdes_mode) {
	case USXGMII_EM:
	case MP_USXGMII_EM:
	{
		#include "usxgmii.h"
	}
	break;
	case QSGMII_EM:
	{
		#include "qsgmii.h"
	}
	break;
	case SGMII_EM:
	case HSGMII_EM:
	{
		#include "sgmii.h"
	}
	break;
	}
}

//4.2.	Serdes寄存器配置
//Refer to xpcs databook 7.11
//4.2.1.	Wirte
void ETH_serdes_reg_write(int xpcs_num,  int reg_addr, int reg_val)
{
	int start_busy_status;
	if (xpcs_num != 0 && xpcs_num != 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	LOG("等待CR Port空闲，0代表空闲\n");
	start_busy_status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY,
				start_busy_status, start_busy_status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}
#if 0
	while (start_busy_status == 1 ) {						//等待CR Port空闲，0代表空闲
		start_busy_status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY);
	}
#endif
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_ADDR, ADDRESS, reg_addr);		//要写入的PHY寄存器地址
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_DATA, DATA, reg_val);	    	//要写入PHY寄存器的数据
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, WR_RDN, 1); 	    	//Write执行
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY, 1);			//CR Port设置为busy

	LOG("等待CR Port空闲，0代表空闲\n");
	start_busy_status=1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY,
				start_busy_status, start_busy_status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}
#if 0
	while (start_busy_status == 1 ) {						//等待CR Port空闲，0代表空闲
		start_busy_status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY);
	}
#endif
}

//4.2.2.	Read
unsigned int PCIE_serdes_reg_read(int reg_addr)
{
	int tmp;
	int cnt;
	int reg_val;
	writel(reg_addr, 0x900c1004);

	writel(0x2, 0x900c1010);
	tmp=0;
	cnt=0;
	while(tmp!=0)
	{
		tmp=readl(0x900c1000);
		cnt=cnt+1;
		if(cnt==50)
		{
			printf("read timeout. \n");
			break;
		}
	}
	reg_val=readl(0x900c100c);
	return reg_val;
}

unsigned PCIE_serdes_reg_write(int reg_addr,int reg_val)
{
	int tmp;
	int cnt;

	writel(reg_addr, 0x900c1004);
	writel(reg_val, 0x900c1008);
	writel(0x1, 0x900c1010);
	tmp=0;
	cnt=0;
	while(tmp!=0)
	{
		tmp=readl(0x900c1000);
		cnt=cnt+1;
		if(cnt==50)
		{
			printf("read timeout. \n");
			break;
		}
	}
	return 1;

}

int ETH_serdes_reg_read(int xpcs_num,  int reg_addr)
{
	int reg_val = 0;
	int start_busy_status;
	if (xpcs_num == 0 || xpcs_num == 1) {

		LOG("等待CR Port空闲，0代表空闲\n");
		start_busy_status = 1;
		if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY,
					start_busy_status, start_busy_status != 1, DEFAULT_TIMEOUT_US)) {
			printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
			return 0;
		}
#if 0
		while (start_busy_status == 1 ) {						//等待CR Port空闲，0代表空闲
			start_busy_status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY);
		}
#endif
		REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_ADDR, ADDRESS, reg_addr);		//要写入的PHY寄存器地址
		REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, WR_RDN, 0); 	    	//Read执行
		REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY, 1);			//CR Port设置为busy

		LOG("等待CR Port空闲，0代表空闲\n");
		start_busy_status = 1;
		if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY,
					start_busy_status, start_busy_status != 1, DEFAULT_TIMEOUT_US)) {
			printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
			return 0;
		}

#if 0
		while (start_busy_status == 1 ) {						//等待CR Port空闲，0代表空闲
			start_busy_status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_CTRL, START_BUSY);
		}
#endif
		reg_val = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_SNPS_CR_DATA, DATA);			//读出寄存器值
		//printf("Serdes0 REG %x = %x. \n ", reg_addr, reg_val);				//讲读取的寄存器名及值打印出来
	}
	else if(xpcs_num==3) {
		reg_val = PCIE_serdes_reg_read(reg_addr);
		//printf("pcie serdes REG %x = %x. \n", reg_addr, reg_val);
	}

	else
		printf("Invalid xpcs numble. \n");

	return reg_val;
}

//4.3.	TX FFE Config  //!!! 10/16 进制
void TX_FFE_Config(int xpcs_num,  int tx_eq_main, int tx_eq_pre, int tx_eq_post)
{

	if (xpcs_num != 0 || xpcs_num !=1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, tx_eq_main);		//TX FFE Main
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, tx_eq_pre);		//TX FFE Pre
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_2, tx_eq_post);		//TX FFE Post
}

//4.4.	RX Adaption
//4.4.1.	RX Adaptation Enable
//reg_dm: 0---通过PCS寄存器配置；1---通过CR接口直接配置Serdes寄存器
void RX_Adapt_enable(int xpcs_num,  bool reg_dm)
{
	int status;
	if (reg_dm != 0) {
		printf("Not Support!!!!\n");
		return;
	}

	if (xpcs_num != 0 || xpcs_num !=1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL1, RX_RST_0, 1);		//复位RX

	LOG("等待RX配置完成\n");
	status = 0;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_STS, RX_ACK_0,
				status, status != 0, DEFAULT_TIMEOUT_US)) {										//change by long
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 1) {				//等待RX配置完成
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_STS, RX_ACK_0);
	}
#endif
	//Transition the PHY lane receiver into P0 state.
	LOG("RX链路上有信号\n");
	status = 0;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_RX_LSTS, SIG_DET_0,
				status, status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 0) {				//RX链路上有信号
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_RX_LSTS, SIG_DET_0);
	}
#endif
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_6G_RXGENCTRL0, RX_DT_EN_0, 1);		//enable RX Path

	LOG("Wait for assertion of pipe_rxX_valid from the PHY\n");
	status = 0;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + SR_PMA_STATUS1, RLU, status,
				status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 0) {				//Wait for assertion of pipe_rxX_valid from the PHY.
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + SR_PMA_STATUS1, RLU);
	}
#endif
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 1);
	LOG("RX adapt完成\n");
	status = 0;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, RX_ADAPT_ACK, status, status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 0) {				//RX adapt完成
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, RX_ADAPT_ACK);
	}
#endif
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 0);
	//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 1);
}

//4.5.1.	TX PRBS Enable/Disable
//xpcs_num: 0---serdes0; 1---serdes1;
//mode: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS23-2, 4---PRBS16, 5---PRBS15, 6---PRBS11, 7---PRBS9, 8---PRBS7
//9---PAT0, 10---(PAT0,~PAT0), 11---(000,PAT0,3ff,~PAT0);
// pat0: user defined pattern
void TX_PRBS_Config(int xpcs_num,  int mode, int pat0)
{
	int reg_val;
	if (xpcs_num > 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	if (mode < 12) {
		reg_val = ETH_serdes_reg_read(xpcs_num,  0x102b);
		printf("%s ===>xpcs%d reg_val[%#x]\n",__func__,xpcs_num, reg_val);
		//reg_val[3:0] = mode;
		reg_val &= ~GENMASK(3, 0);
		reg_val |= mode;
		//reg_val[14:5] = pat0;
		reg_val &= ~GENMASK(14, 5);
		reg_val |= (pat0 << 5);
		printf("%s ===> reg write [%#x]\n",__func__,xpcs_num, reg_val);
		ETH_serdes_reg_write(xpcs_num,  0x102b, reg_val);
	}
	else
		printf("Invalid TX PRBS MODE. \n");

}
//4.5.2.	TX PRBS Error Insert
void TX_err_Insert(int xpcs_num)
{
	int reg_val;

	if (xpcs_num > 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	reg_val = ETH_serdes_reg_read(xpcs_num,  0x102b);
	reg_val |= 1 << 4;
	ETH_serdes_reg_write(xpcs_num,  0x102b, reg_val);			//insert an error
	reg_val &= ~(1 << 4);
	ETH_serdes_reg_write(xpcs_num,  0x102b, reg_val);
}
//4.5.3.	RX PRBS Enable/Disable
//重新配置一遍可清空RX Error Counter
//prbs_num: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS23-2, 4---PRBS16, 5---PRBS15, 6---PRBS11, 7---PRBS9, 8---PRBS7
//9---d[n]=d[n-10], 10---d[n]=!d[n-10], 11---d[n]=!d[n-20]
void RX_PRBS_Config(int xpcs_num,  int mode)
{
	int reg_val;
	if (xpcs_num > 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	if (mode < 12) {
		reg_val = ETH_serdes_reg_read(xpcs_num,  0x1051);
		reg_val &= ~GENMASK(3,0);
		reg_val |= mode;		//prbs mode
		reg_val |= (1 << 4);				//sync pattern matcher
		ETH_serdes_reg_write(xpcs_num,  0x1051, reg_val);
	}
	else
		printf("Invalid RX PRBS MODE. \n");

}

//4.5.4.	Read RX Error Count
int RX_err_count(int xpcs_num)
{
	int reg_val = -1;
	if (xpcs_num > 1) {
		printf("Invalid xpcs numble. \n");
		return reg_val;
	}

	reg_val = ETH_serdes_reg_read(xpcs_num,  0x1052);
	return reg_val;
}
//4.6.	Loopback(serial/parallel);
//Refer to XCPS databook 2.17
//Refer to PHY databook 6.3.1
//mode: 0---disable loopback, 1---enable tx-rx serial loopback, 2---enable rx-tx parallel loopback
//reg_dm: 0---通过 xpcs reg配置, 1---直接通过 Serdes reg配置
void Loopback_config(int xpcs_num,  int mode, bool reg_dm)
{
	int reg_val;
	if (xpcs_num > 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	if (reg_dm == 0) {							//config xpcs reg
		if (mode == 0) {						//disable serial/parallel loopback
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0, TX2RX_LB_EN_0, 0);	//disable TX2RX serial loopback
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0, RX2TX_LB_EN_0, 0);	//disable RX2TX Parallel loopback
		}
		else if (mode == 1) {					//enable serial loopback
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0, TX2RX_LB_EN_0, 1);	//enable TX2RX serial loopback
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0, RX2TX_LB_EN_0, 0);	//disable RX2TX Parallel loopback

		}
		else if (mode == 2) {					//enable parallel loopback
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0, TX2RX_LB_EN_0, 0);	//disable TX2RX serial loopback
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0, RX2TX_LB_EN_0, 1);	//enable RX2TX Parallel loopback

		}
		else
			printf("Invalid Loopback Mode. \n");
	} else {							//config serdes reg
		reg_val = 0;
		if (mode == 0) {					//disable serial/parallel loopback
			reg_val = 4;
			//reg_val[2] = 1;				//enable override values
			//reg_val[1] = 0;				//diable RX2TX parallel loopback
			//reg_val[0] = 0;				//diable TX2RX serial loopback
			ETH_serdes_reg_write(xpcs_num,  0x1000, reg_val);
		} else if (mode == 1) {				//enable serial loopback
			reg_val = 5;
			//reg_val[2] = 1;				//enable override values
			//reg_val[1] = 0;				//diable RX2TX parallel loopback
			//reg_val[0] = 1;				//enable TX2RX serial loopback
			ETH_serdes_reg_write(xpcs_num,  0x1000, reg_val);
		} else if (mode == 2) {				//enable parallel loopback
			reg_val = 6;
			//reg_val[2] = 1;				//enable override values
			//reg_val[1] = 1;				//enable RX2TX parallel loopback
			//reg_val[0] = 0;				//diable TX2RX serial loopback
			ETH_serdes_reg_write(xpcs_num,  0x1000, reg_val);
		} else
			printf("Invalid Loopback Mode. \n");
	}

}

//4.8.	REF Repeat CLK output Enable/Disable
//只有Serdes1支持
void Repeat_CLK_Config(int xpcs_num, int mode)
{
	if (mode == 1) {				//使能repeat clk输出
		REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 1);
	} else {
		REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 0);
	}
	printf("REF Repeat CLK output enable. \n");
}

void tx_req(int xpcs_num)
{
	int status;

	if (xpcs_num != 0 && xpcs_num != 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX_REQ_0, 1);

	LOG("等待tx_ack指示操作完成\n");
	status = 0;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_STS, TX_ACK_0,
				status, status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	status = 0;
	while(status == 0) {											//等待tx_ack指示操作完成
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_STS, TX_ACK_0);
	}
#endif
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX_REQ_0, 0);
}

void rx_req(int xpcs_num)
{
	int status;

	if (xpcs_num != 0 && xpcs_num != 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX_REQ_0, 1);

	LOG("等待rx_ack指示操作完成\n");
	status = 0;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_STS, RX_ACK_0,
				status, status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}

#if 0
	status = 0;
	while(status == 1) {											//等待rx_ack指示操作完成
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_STS, RX_ACK_0);
	}
#endif
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX_REQ_0, 0);
}

void rx_afe_overwride(int xpcs_num, int eq_att, int eq_gain1, int eq_gain2, int eq_pole, int eq_boost, int eq_tap1)
{
	int status;
	if(xpcs_num == 0 || xpcs_num == 1) {
		status = 0;
		status = (eq_boost << 9) + (eq_gain1 << 6) + (eq_gain2 << 3) + (eq_att);
		ETH_serdes_reg_write(xpcs_num, 0x1009, status);
		status = 0;
		status = 0x400 + (eq_tap1 << 2) + eq_pole;
		ETH_serdes_reg_write(xpcs_num, 0x100a, status);
		//rx req
		ETH_serdes_reg_write(xpcs_num, 0x3006, 0xc);
		status = 0;
		//整理等待ack=1，等50个循环吧
		//status = ETH_serdes_reg_read(0, 0x300f);	//bit[0]:ack
		if (eth_reg_read_poll_timeout(xpcs_num, 0x300f,
					status, BIT(0) & status, DEFAULT_TIMEOUT_US)) {
			printf("Line %d: wait status[%d] timeout", __LINE__, status);
		}

		ETH_serdes_reg_write(xpcs_num, 0x3006, 0x4);

	} else if(xpcs_num == 2) {
		status = 0;
		status = (eq_boost << 9) + (eq_gain1 << 6) + (eq_gain2 << 3) + (eq_att);
		PCIE_serdes_reg_write(0x1009, status);
		status = 0;
		status = 0x400 + (eq_tap1 << 2) + eq_pole;
		PCIE_serdes_reg_write(0x100a, status);
		//rx req
		PCIE_serdes_reg_write(0x3006, 0xc);
		status = 0;
		//整理等待ack=1，等50个循环吧
		//status = PCIE_serdes_reg_read(0x300f);	//bit[0]:ack
		if (pcie_reg_read_poll_timeout(0x300f,
					status, BIT(0) & status, DEFAULT_TIMEOUT_US)) {
			printf("Line %d: wait status[%d] timeout", __LINE__, status);
		}

		PCIE_serdes_reg_write(0x3006, 0x4);
	}
}

void Serdes_dump(int option)
{
	for (int i = 1; i < 3; i++) {
		//Serdes0
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_MPLL_CMN_CTRL, MPLLB_SEL_0);	//0---MPLLA, 1---MPLLB
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_CLK_DIV2);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_MPLLA_DIV2);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL1, RX_DIV16P5_CLK_EN_0);
		//REG_PRINT(XPCS_BASE_ADDR(0), VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0);

		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_RX_LSTS, RX_VALID_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL0, TX_DT_EN_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_6G_RXGENCTRL0, RX_DT_EN_CTL);

		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_LVL);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_BOOST_CTRL, TX0_IBOOST);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_2);

		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_RX_ATTN_CTRL, RX0_EQ_ATT_LVL);		//3’b000---(-2dB), 3’b111---(-6dB, REG_PRINT);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0);
		REG_PRINT(XPCS_BASE_ADDR((option & i)), VR_XS_PMA_MP_12G_16G_25G_DFE_TAP_CTRL0, DFE_TAP1_0);
	}
}
//tx_vboost_en: 0---diable boost function; 1---enable boost function
void TX_Swing_Config(int xpcs_num, int tx_vboost_en, int tx_vboost_lvl, int tx_iboost_lvl)
{
	if(xpcs_num != 0 && xpcs_num != 1) {
		printf("Invalid xpcs numble. \n");
		return;
	}

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, tx_vboost_en);
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_LVL, tx_vboost_lvl);
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_BOOST_CTRL, TX0_IBOOST, tx_iboost_lvl);
}

void ETH_PLL_Config(int refdiv, int fbdiv, int frac, int postdiv1, int postdiv2)
{
	int lck_status;

	REG_W(FWD_TOP_CLK_SEL_PARA, CLK_SEL_PARA, 0);			//切到refclk
	REG_W(FWD_TOP_PLL_PARA1, PLLEN, 0);					//power down PLL

	REG_W(FWD_TOP_PLL_PARA1, REFDIV, refdiv);
	REG_W(FWD_TOP_PLL_PARA1, FBDIV, fbdiv);
	REG_W(FWD_TOP_PLL_PARA2, FRAC, frac);
	REG_W(FWD_TOP_PLL_PARA1, POSTDIV1, postdiv1);
	REG_W(FWD_TOP_PLL_PARA1, POSTDIV2, postdiv2);

	if(frac ==0)
		REG_W(FWD_TOP_PLL_PARA1, DSMEN, 0);
	else
		REG_W(FWD_TOP_PLL_PARA1, DSMEN, 1);

	REG_W(FWD_TOP_PLL_PARA1, PLLEN, 1);					//power up PLL

	printf("判断PLL是否lock，1代表lock\n");
	lck_status = 0;
	if (reg_poll_timeout(FWD_TOP_PLL_PARA3, PLL_PARA3,
				lck_status , lck_status  != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, lck_status);
		return;
	}

#if 0
	while(lck_status == 0) {						//判断PLL是否lock，1代表lock
		lck_status = REG_R(FWD_TOP_PLL_PARA3, PLL_PARA3);
	}
#endif
	REG_W(FWD_TOP_CLK_SEL_PARA, CLK_SEL_PARA, 1);			//切回PLL输出

}

//xpcs_num: 0---serdes0; 1---serdes1;
//mode: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS23-2, 4---PRBS16, 5---PRBS15, 6---PRBS11, 7---PRBS9, 8---PRBS7
//9---PAT0, 10---(PAT0,~PAT0), 11---(000,PAT0,3ff,~PAT0);
// pat0: user defined pattern
void PCIE_TX_PRBS_Config(int mode, int pat0)
{
	int reg_val;
	if(mode < 12) {
		reg_val = PCIE_serdes_reg_read(0x102b);
		reg_val&=~(GENMASK(3, 0) | GENMASK(14,5));
		reg_val |= mode | pat0 << 5;

		//reg_val[3:0] = mode;
		//reg_val[14:5] = pat0;
		PCIE_serdes_reg_write(0x102b, reg_val);
	}
	else
		printf("Invalid TX PRBS MODE. \n");

}

void PCIE_TX_err_Insert()
{
	int reg_val;

	reg_val = PCIE_serdes_reg_read(0x102b);
	reg_val |= BIT(4);
	PCIE_serdes_reg_write(0x102b, reg_val);			//insert an error
	reg_val &= ~BIT(4);
	PCIE_serdes_reg_write(0x102b, reg_val);
}

//重新配置一遍可清空RX Error Counter
//prbs_num: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS23-2, 4---PRBS16, 5---PRBS15, 6---PRBS11, 7---PRBS9, 8---PRBS7
//9---d[n]=d[n-10], 10---d[n]=!d[n-10], 11---d[n]=!d[n-20]
void PCIE_RX_PRBS_Config(int mode)
{
	int reg_val;

	if(mode < 12) {
		reg_val = PCIE_serdes_reg_read(0x1051);
		//reg_val[3:0] = mode;		//prbs mode
		reg_val&=~GENMASK(3, 0);
		reg_val |=mode & GENMASK(3,0);
		reg_val |= BIT(4);				//sync pattern matcher
		PCIE_serdes_reg_write(0x1051, reg_val);
	}
	else
		printf("Invalid RX PRBS MODE. \n");

}

void PCIE_RX_err_count()		//2023.7.24修改
{
	int reg_val;
	reg_val = PCIE_serdes_reg_read(0x1052);
	return reg_val;
}

void PCIE_TX_FFE_Config(int tx_eq_main, int tx_eq_pre, int tx_eq_post)
{
	int reg_val;
	reg_val = PCIE_serdes_reg_read(0x1002);
	reg_val&=~(GENMASK(14,9));
	reg_val |= tx_eq_main << 9;
	reg_val |= BIT(15);
	PCIE_serdes_reg_write(0x1002, reg_val);

	reg_val = PCIE_serdes_reg_read(0x1003);
	reg_val&=~(GENMASK(12,7));
	reg_val |= tx_eq_post << 7;
	reg_val |= BIT(13);
	PCIE_serdes_reg_write(0x1003, reg_val);

	reg_val = PCIE_serdes_reg_read(0x1002);
	reg_val&=~(GENMASK(14,9));
	reg_val |= tx_eq_pre << 9;
	reg_val |= BIT(15);
	PCIE_serdes_reg_write(0x1002, reg_val);
}

void ETH_RX_ADAPT(int xpcs_num)
{
	int reg_val;
	//ATE adapt req
	//overriding rx_adapt_req bit[8] bit[5] bit[4]=1
	ETH_serdes_reg_write(xpcs_num, 0x3008, 0x130);
	//PCIE_serdes_reg_read(0x3008);
	//adapt req ADAPT_REQ_OVRD_EN bit[14]=1, ADAPT_REQ bit[13]=1
	//PCIE_serdes_reg_read 0x3019
	//PCIE_serdes_reg_write(0x3019, 0x6000);
	//wait 10ms
	usleep(10000);
	//reading RX REQ at PMA level(0x2408) bit[3]=1   ->0x2404
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d\n", reg_val);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=1, DATA_EN bit[2]=1
	ETH_serdes_reg_write(xpcs_num, 0x1005, 0x240C);
	//wait 10ms
	usleep(10000);
	//expecting RX ACK to be asserted(0xf)  bit[0]=1   ->0xd
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//wait 10ms
	//expecting RX REQ to be de-asserted(0x2400) bit[3]=0   ->0x2404
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d (1:RX_REQ asserted, 0:RX_REQ de-asserted)\n", reg_val);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=0, DATA_EN bit[2]=1
	ETH_serdes_reg_write(xpcs_num, 0x1005, 0x2404);
	//wait 10ms
	usleep(10000);
	//expecting RX ACK to be de-asserted(0x6) ACK bit[0]=0
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//overriding rx_ovrd_eq_in_en EQ_OVRD_EN bit[10]=0 ????
	//reg_val = PCIE_serdes_reg_read(0x100a);
	//PCIE_serdes_reg_write(0x100a, 0x28);
	//wait 900ms
	sleep(1);	//sleep 1s
	//expecting RX REQ to be asserted(0x2408) REQ bit[3]=1   ->0x2404
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d (1:RX_REQ asserted, 0:RX_REQ de-asserted)\n", reg_val);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=0, DATA_EN bit[2]=1
	ETH_serdes_reg_write(xpcs_num, 0x1005, 0x2404);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=1, DATA_EN bit[2]=1
	ETH_serdes_reg_write(xpcs_num, 0x1005, 0x240C);
	//waite 10ms
	usleep(10000);
	//expecting RX ACK to be asserted(0xf) ACK bit[0]=1   ->0xf->0xd change
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//wait 10ms
	usleep(10000);
	//expecting RX REQ to be de-asserted(0x2400)   ->0x2404
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d (1:RX_REQ asserted, 0:RX_REQ de-asserted)\n", reg_val);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=0, DATA_EN bit[2]=1
	ETH_serdes_reg_write(xpcs_num, 0x1005, 0x2404);
	//wait 10ms
	usleep(10000);
	//expecting RX ACK to be de-asserted(0xe)   ->0xc
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//wait 100ms
	usleep(100000);
	//expecting RX ADAPT ACK to be asserted(0x1)  ->0x1
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x3010);
	ETH_serdes_reg_write(xpcs_num, 0x3008, 0x120);
	//wait 100ms
	usleep(100000);
	//expecting RX ADAPT ACK to be-asserted(0x0)   ->0x0
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x3010);
	//enable RX cont adapt
	ETH_serdes_reg_write(xpcs_num, 0x3008, 0x1e0);
	//rx cont adapt status
	reg_val = ETH_serdes_reg_read(xpcs_num, 0x30C6);
	reg_val &= BIT(0);
	printf("RX ADAPT CONT = %d (1:cont adapt enable, 0:cont adapt disable)\n", reg_val);
}

void PCIE_RX_ADAPT()
{
	int reg_val;
	//ATE adapt req
	//overriding rx_adapt_req bit[8] bit[5] bit[4]=1
	PCIE_serdes_reg_write(0x3008, 0x130);
	//PCIE_serdes_reg_read(0x3008);
	//adapt req ADAPT_REQ_OVRD_EN bit[14]=1, ADAPT_REQ bit[13]=1
	//PCIE_serdes_reg_read 0x3019
	//PCIE_serdes_reg_write(0x3019, 0x6000);
	//wait 10ms
	usleep(10000);
	//reading RX REQ at PMA level(0x2408) bit[3]=1   ->0x2404
	reg_val = PCIE_serdes_reg_read(0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d\n", reg_val);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=1, DATA_EN bit[2]=1
	PCIE_serdes_reg_write(0x1005, 0x240C);
	//wait 10ms
	usleep(10000);
	//expecting RX ACK to be asserted(0xf)  bit[0]=1   ->0xd
	reg_val = PCIE_serdes_reg_read(0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//wait 10ms
	//expecting RX REQ to be de-asserted(0x2400) bit[3]=0   ->0x2404
	reg_val = PCIE_serdes_reg_read(0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d (1:RX_REQ asserted, 0:RX_REQ de-asserted)\n", reg_val);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=0, DATA_EN bit[2]=1
	PCIE_serdes_reg_write(0x1005, 0x2404);
	//wait 10ms
	usleep(10000);
	//expecting RX ACK to be de-asserted(0x6) ACK bit[0]=0
	reg_val = PCIE_serdes_reg_read(0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//overriding rx_ovrd_eq_in_en EQ_OVRD_EN bit[10]=0 ????
	//reg_val = PCIE_serdes_reg_read(0x100a);
	//PCIE_serdes_reg_write(0x100a, 0x28);
	//wait 900ms
	sleep(1);	//sleep 1s
	//expecting RX REQ to be asserted(0x2408) REQ bit[3]=1   ->0x2404
	reg_val = PCIE_serdes_reg_read(0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d (1:RX_REQ asserted, 0:RX_REQ de-asserted)\n", reg_val);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=0, DATA_EN bit[2]=1
	PCIE_serdes_reg_write(0x1005, 0x2404);
	//wait 1ms
	usleep(1000);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=1, DATA_EN bit[2]=1
	PCIE_serdes_reg_write(0x1005, 0x240C);
	//waite 10ms
	usleep(10000);
	//expecting RX ACK to be asserted(0xf) ACK bit[0]=1   ->0xf->0xd change
	reg_val = PCIE_serdes_reg_read(0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//wait 10ms
	usleep(10000);
	//expecting RX REQ to be de-asserted(0x2400)   ->0x2404
	reg_val = PCIE_serdes_reg_read(0x1011);
	reg_val &= BIT(3);
	reg_val = reg_val >> 3;
	printf("RX_REQ = %d (1:RX_REQ asserted, 0:RX_REQ de-asserted)\n", reg_val);
	//EN bit[13]=1, width bit[10:9]=2, RATE=0 bit[8:7], PSTATE bit[6:5]=0, REQ bit[3]=0, DATA_EN bit[2]=1
	PCIE_serdes_reg_write(0x1005, 0x2404);
	//wait 10ms
	usleep(10000);
	//expecting RX ACK to be de-asserted(0xe)   ->0xc
	reg_val = PCIE_serdes_reg_read(0x1017);
	reg_val &= BIT(0);
	printf("RX_ACK = %d (1:ack, 0:not ack)\n", reg_val);
	//wait 100ms
	usleep(100000);
	//expecting RX ADAPT ACK to be asserted(0x1)  ->0x1
	reg_val = PCIE_serdes_reg_read(0x3010);
	PCIE_serdes_reg_write(0x3008, 0x120);
	//wait 100ms
	usleep(100000);
	//expecting RX ADAPT ACK to be-asserted(0x0)   ->0x0
	reg_val = PCIE_serdes_reg_read(0x3010);
	//enable RX cont adapt
	PCIE_serdes_reg_write(0x3008, 0x1e0);
	//rx cont adapt status
	reg_val = PCIE_serdes_reg_read(0x30C6);
	reg_val &= BIT(0);
	printf("RX ADAPT CONT = %d (1:cont adapt enable, 0:cont adapt disable)\n", reg_val);
}

void PCIE_PLL_Config(int refdiv, int fbdiv, int frac, int postdiv1, int postdiv2)
{
	int lck_status;
#if 1
	REG_W(PCIE_TOP_CLK_SEL_PARA, CLK_SEL_PARA, 0);			//切到refclk
	REG_W(PCIE_TOP_PLL_PARA1, PLLEN, 0);					//power down PLL

	REG_W(PCIE_TOP_PLL_PARA1, REFDIV, refdiv);
	REG_W(PCIE_TOP_PLL_PARA1, FBDIV, fbdiv);
	REG_W(PCIE_TOP_PLL_PARA2, FRAC, frac);
	REG_W(PCIE_TOP_PLL_PARA1, POSTDIV1, postdiv1);
	REG_W(PCIE_TOP_PLL_PARA1, POSTDIV2, postdiv2);

	if(frac ==0)
		REG_W(PCIE_TOP_PLL_PARA1, DSMEN, 0);
	else
		REG_W(PCIE_TOP_PLL_PARA1, DSMEN, 1);

	REG_W(PCIE_TOP_PLL_PARA1, PLLEN, 1);					//power up PLL

	printf("判断PLL是否lock，1代表lock\n");
	lck_status = 0;
	if (reg_poll_timeout(PCIE_TOP_PLL_PARA3, PLL_PARA3,
				lck_status , lck_status  != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, lck_status);
		return;
	}

	REG_W(PCIE_TOP_CLK_SEL_PARA, CLK_SEL_PARA, 1);			//切回PLL输出
#endif
}

void PCIE_init(int gen, int tx_main, int tx_pre, int tx_post, int rx_adapt_en)
{
	int status = 0;
	int tx_main_reg = 0;
	int tx_post_pre_reg = 0;
	//clkout_sel 1000M
	writel(0x1, 0x90412a0c);
	//sys_aux_pwr_det
	writel(0x1, 0x900c0008);

	writel(0xffffe5, 0x90414008);
	//sram_bypass=0
	writel(0x0, 0x900c074c);
	//PCIE_SFT_RST_CTRL
	writel(0x0, 0x90000084);
	//sft_buttom_rst_n
	writel(0x1, 0x90000084);
	//sft_rst_pcie_n
	writel(0xffffe7, 0x90414008);

	usleep(100);
	//sft_perst_n
	writel(0x3, 0x90000084);

	//判断SRAM是否加载完成
	//status = PCIE_serdes_reg_read(0x203e);
	status = 0;
	if (pcie_reg_read_poll_timeout(0x203e,
				status, status != 0, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}

	sleep_nano(80);

	if (g_serdes_fw)
		for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
			PCIE_serdes_reg_write(SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
		}

	//SRAM init in
	writel(1, 0x900c0750);

	//target_link_speed gen4
	if(gen == 4)
		writel(0x2010044, 0xc00000a0);
	else if(gen == 3)
		writel(0x2010043, 0xc00000a0);
	else if(gen == 2)
		writel(0x2010042, 0xc00000a0);
	else if(gen == 1)
		writel(0x2010041, 0xc00000a0);
	else {
		printf("Wrong Gen number\n");
		return;
	}
	//loopback_enable
	writel(0x10124, 0xc0000710);
	//app_ltssm_enale
	writel(0x1, 0x900c0010);
	//10ms
	sleep(1);
	status = readl(0x900c00c0);
	if(status == 0x3)
		printf("PCIE Gen%d link ok\n", gen);
	else
		printf("PCIE link down\n");

	//TX Main=30
	tx_main_reg = (PCIE_serdes_reg_read(0x1002) & ~GENMASK(14, 9)) + (tx_main << 9) + 0x8000;
	tx_post_pre_reg = (tx_post << 7) + tx_pre + 0x2040;
	PCIE_serdes_reg_write(0x1002, tx_main_reg);
	//TX Post=40
	PCIE_serdes_reg_write(0x1003, tx_post_pre_reg);

	//tx prbs config
	PCIE_serdes_reg_write(0x102b, 0x1);
	if (rx_adapt_en) {
		//RX adapt
		PCIE_RX_ADAPT();
		msleep(100);
		PCIE_RX_ADAPT();
		msleep(100);
	}
	//rx prbs
	PCIE_serdes_reg_write(0x1051, 0x11);
	//read error
	status = PCIE_serdes_reg_read(0x1052);
	status = PCIE_serdes_reg_read(0x1052);
	// add err
	PCIE_serdes_reg_write(0x102b, 0x11);
	PCIE_serdes_reg_write(0x102b, 0x1);
	msleep(100);
	//read error
	status = PCIE_serdes_reg_read(0x1052);
	msleep(100);
	status = PCIE_serdes_reg_read(0x1052);
	printf("PCIE RX Error Counters %d\n", status);
}
typedef struct {
	int serdes0;
	int serdes1;
	int pcie;
} err_stats_st;

//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define  ERR_STATS_PATH  "/tmp/err_stats"

static void save_err_stats(err_stats_st * stats)
{
	int fd = open(ERR_STATS_PATH, O_CREAT | O_RDWR, 0777);
	write(fd, stats, sizeof(*stats));
	close(fd);
}

static void load_err_stats(err_stats_st * stats)
{
	if (0 == access(ERR_STATS_PATH, F_OK)) {
		int fd = open(ERR_STATS_PATH, O_RDONLY, 0777);
		write(fd, stats, sizeof(*stats));
		read(fd, stats, sizeof(*stats));
		close(fd);
	}
}
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define SEDES_PID_PATH  	"/tmp/serdes.pid"
#define SEDES_CONFIG_PATH  	"/tmp/serdes.config"
typedef struct {
	int unecho;
	int title;
	int sleep;
	int exit;
	int debug;
} serdes_config_st;

serdes_config_st  g_serdes_config = {.sleep = 30};


void reload_conf()
{
	if (0 == access(SEDES_CONFIG_PATH, F_OK)) {
		int fd = open(SEDES_CONFIG_PATH, O_RDONLY, 0777);
		read(fd, &g_serdes_config, sizeof(g_serdes_config));
		close(fd);;
	}

	g_debug = g_serdes_config.debug;

}

int load_pid_from_file()
{
	int pid = 0;
	if (0 == access(SEDES_PID_PATH, F_OK)) {
		int fd = open(SEDES_PID_PATH, O_RDONLY, 0777);
		read(fd, &pid, sizeof(pid));
		close(fd);
	}

	return pid;
}

int save_pid_to_file()
{
	int pid = getpid();
	int fd = open(SEDES_PID_PATH, O_CREAT | O_RDWR, 0777);
	write(fd, &pid, sizeof(pid));
	close(fd);
	return pid;
}

void save_config()
{
	int	fd = open(SEDES_CONFIG_PATH, O_CREAT | O_RDWR, 0777);
	write(fd, &g_serdes_config, sizeof(g_serdes_config));
	close(fd);
}

void log_config(int argc, char *argv[])
{
	//printf("Get %s val[%#x]\n", #val, val);
#define CONVERT_TO_UINT(val) ({val = strtoul(argv[++i], NULL, 0); \
		})
	int pid;
	int fd;
	int i;
	int val;
	char * name;
	char pid_path[256] = {0};

	fd = open(SEDES_CONFIG_PATH, O_CREAT | O_RDWR, 0777);
	if (0 == access(SEDES_CONFIG_PATH, F_OK)) {
		read(fd, &g_serdes_config, sizeof(g_serdes_config));
	}

	for (i = 0; i < argc; i++) {
		name = argv[i];
		if (0 == strcasecmp("title", name)) {
			CONVERT_TO_UINT(val);
			g_serdes_config.title = val;
		} else if (0 == strcasecmp("sleep", name)) {
			CONVERT_TO_UINT(val);
			g_serdes_config.sleep = val ? val : 1;
		} else if (0 == strcasecmp("unecho", name)) {
			CONVERT_TO_UINT(val);
			g_serdes_config.unecho = val;
		} else if (0 == strcasecmp("exit", name)) {
			CONVERT_TO_UINT(val);
			g_serdes_config.exit = val;
		} else if (0 == strcasecmp("debug", name)) {
			CONVERT_TO_UINT(val);
			g_serdes_config.debug = val;
		}
	}

	lseek(fd, SEEK_SET, 0);
	write(fd, &g_serdes_config, sizeof(g_serdes_config));
	close(fd);

	pid = load_pid_from_file();
	if (pid) {
		struct stat st={0};
		sprintf(pid_path, "/proc/%d", pid);
		if (stat(pid_path, &st) == 0 && S_ISDIR(st.st_mode)) {
			kill(pid, SIGUSR1);
		}
	}
}
#include <stdio.h>
#include <stdbool.h>
#define PARA_MAX_NUM  10
int cmd_get(char * cmd, char * buf, int buf_size, char * argv[])
{
	FILE * fp = NULL;
	char * str_ptr = buf;
	bool is_para_left = true;
	int size = 0, argc = 0;

	memset(buf, 0, sizeof(buf_size));
	fp = popen(cmd, "r");

	size = fread(buf, sizeof(buf_size), 100, fp);

	while ( '\0' != *str_ptr && argc < PARA_MAX_NUM) {
		if ('\t' == *str_ptr || ' ' == *str_ptr || '\n' == *str_ptr) {
			*str_ptr = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = str_ptr;
		}
		str_ptr++;
	}

	pclose(fp);

	return argc;
}

float tempcal(uint32_t datain, uint32_t modecalc)
{
	float DataCal;
	uint32_t Fclkm = 6;

	if (modecalc == 1)
		DataCal = 42.74 + (220.5 * (((float)datain / 4094) - 0.5)) - (0.16 * Fclkm);
	else if (modecalc == 2)
		DataCal = 59.1 + (202.8 * (((float)datain / 4094) - 0.5)) - (0.16 * Fclkm);
	//printf("datain %08x modecalc %08x Temperature[%f C]", datain, modecalc, DataCal);
	return DataCal;
}

float volcal(uint32_t datain, uint32_t resolution)
{
	float voltage = 0;

	voltage = (1.20 * ((float)(6 * datain) / (1 << 14) -  ((float)3 / (1 << resolution)) - 1)) / 5;
	return voltage;
}

void log_printf(int title)
{
#define TEMP_CMD "dmesg -c > /dev/null && echo w printk 0x1 >/sys/kernel/debug/pvt/pvt_cmd && dmesg -c | grep Temperature | sed -s \"s/.*Temperature\\[//g\" |  sed -s \"s/C\\].*//g\""
#define V_CMD  	 "dmesg -c > /dev/null && echo w printk 0x1 >/sys/kernel/debug/pvt/pvt_cmd && dmesg -c | grep VDDQ | sed  -s \"s/.*: \\[//g\" | sed -s \"s/V\\].*//g\""
#define DATE_CMD "date  +\"%Y/%m/%d %H:%M:%S\""

	int serdes_num = 0;
	int err_status = 0;
	int err_count = 0;
	int tx_eq_main = 0;
	int tx_eq_pre = 0;
	int tx_eq_post = 0;
	int vboost_en = 0;
	int vboost_lvl = 0;
	int tx_iboost = 0;
	int tx_rate = 0;
	int rx_rate = 0;
	int tx_witdh = 0;
	int rx_width = 0;
	int cont_adpat = 0;
	int rx_eq_att_lvl = 0;
	int ctle_pole = 0;
	int vga1_gain = 0;
	int vga2_gain = 0;
	int ctle_boost = 0;
	int dfe_tap1 = 0;
	int xpcs_num = 0;
	int status = 0;
	int rx_adapt_done = 0;
	int rx_cont_adapt = 0;
	int rx_dfe_bypass = 0;
	int rx_att_adapt_val = 0;
	int rx_vga_adapt_val = 0;
	int rx_ctle_pole_adapt_val = 0;
	int rx_ctle_boost_adapt_val = 0;
	int dfe_status;
	int dfe_tap1_adapt_val = 0;
	int dfe_tap2_adapt_val = 0;
	int dfe_tap3_adapt_val = 0;
	int dfe_tap4_adapt_val = 0;
	int dfe_tap5_adapt_val = 0;

	err_stats_st s_err_stats = {0};

	log_config(0, NULL);

	int pid = load_pid_from_file();
	if (pid) {
		char pid_path[256] = {0};
		struct stat st={0};
		sprintf(pid_path, "/proc/%d", pid);
		if (stat(pid_path, &st) == 0 && S_ISDIR(st.st_mode))
			return;
	}

	daemon(0, 1);
	save_config();

	signal(SIGUSR1, reload_conf);
	save_pid_to_file();

	load_err_stats(&s_err_stats);
	system("insmod /lib/firmware/dubhe1000_pvt.ko");
	system("mount -t debugfs none /sys/kernel/debug");
	while(true) {

		char str[128] = {0};
		int offset = 0;
		int v_offset = 0;
		int unecho = g_serdes_config.unecho;
		int sleep_sec = g_serdes_config.sleep ? g_serdes_config.sleep : 30;
		char sensor_buf[256] = {0};
		char sensor_v_buf[256] = {0};
		char temp_buf[100] = {0};
		char * temp_para[PARA_MAX_NUM] = {0};
		char v_buf[100] = {0};
		char * v_para[PARA_MAX_NUM] = {0};
		char date_buf[100] = {0};
		char * date_para[PARA_MAX_NUM] = {0};
		int dig_rx_dpll_freq = 0;

		if (g_serdes_config.exit) {
			printf("exit !!!!!!!!!!!\n");
			break;
		}

		if (unecho) {
			sleep(1);
			continue;
		}
#if 1
#define T0_SENSOR_BASE_ADDR     0x90500000
#define T_SENSOR_NUM            3
#define V_SENSOR_POINT_NUM      7
#define TS_COMMON_OFFSET        0x80
#define VM_COMMON_OFFSET        0x400
#define MACRO_OFFSET            0x40
#define TS_MACRO_OFFSET         (TS_COMMON_OFFSET + MACRO_OFFSET)
#define VM_MACRO_OFFSET         (VM_COMMON_OFFSET + 0x200)
#define SDIF_DATA_REG           0x18
#define VM_SDIF_DATA_REG        0x40

		system("echo w printk 0x3 >/sys/kernel/debug/pvt/pvt_cmd");
		cmd_get(DATE_CMD, date_buf, sizeof(date_buf), date_para);
		offset += snprintf(sensor_buf + offset, sizeof(sensor_buf) - offset,  "%20s",
				date_para[0]);

		for (int cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
			uint32_t ts_rdata = readl(T0_SENSOR_BASE_ADDR + TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
			offset += snprintf(sensor_buf + offset, sizeof(sensor_buf) - offset,  ",%03.2f",
					tempcal(ts_rdata, 1));
			v_offset += snprintf(sensor_v_buf + v_offset, sizeof(sensor_v_buf) - v_offset,  ",%#X",
					ts_rdata);
		}

		uint32_t vm_radta = readl(T0_SENSOR_BASE_ADDR + VM_MACRO_OFFSET + VM_SDIF_DATA_REG);
		offset += snprintf(sensor_buf+ offset, sizeof(sensor_buf) - offset,  ",%02.3f",
				volcal(vm_radta, 14));

		v_offset += snprintf(sensor_v_buf + v_offset, sizeof(sensor_v_buf) - v_offset,  ",%#X",
				vm_radta);
		printf("==>sensor val %s\n", sensor_v_buf);
#endif
#if 0
		cmd_get(TEMP_CMD, temp_buf, sizeof(temp_buf), temp_para);
		cmd_get(V_CMD, v_buf, sizeof(v_buf), v_para);
		cmd_get(DATE_CMD, date_buf, sizeof(date_buf), date_para);
		sprintf(sensor_buf, "%12s,%10s,%10s,%10s,%10s",
				date_para[0],
				temp_para[0],
				temp_para[1],
				temp_para[2],
				v_para[0]);
#endif

		if (title)
			printf("%15s,date, t0, t1, t2, voltage,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s,%15s\n",
					"serdes_num",
					"err_status",
					"err_count",
					"tx_eq_main",
					"tx_eq_pre",
					"tx_eq_post",
					"vboost_en",
					"vboost_lvl",
					"tx_iboost",
					"tx_rate",
					"rx_rate",
					"tx_witdh",
					"rx_width",
					"cont_adpat",
					"rx_eq_att_lvl",
					"ctle_pole",
					"vga1_gain",
					"vga2_gain",
					"ctle_boost",
					"dfe_tap1",
					"rx_adapt_done",
					"rx_dfe_bypass",
					"rx_cont_adapt",
					"rx_att_adapt_val",
					"rx_vga_adapt_val",
					"rx_ctle_pole_adapt_val",
					"rx_ctle_boost_adapt_val",
					"dfe_tap1_adapt_val",
					"dfe_tap2_adapt_val",
					"dfe_tap3_adapt_val",
					"dfe_tap4_adapt_val",
					"dfe_tap5_adapt_val",
					"dig_rx_dpll_freq");

		for (xpcs_num = 0 ; xpcs_num < 2; xpcs_num++)
		{
			status = 0;
			err_count = RX_err_count(xpcs_num);
			if (err_count > s_err_stats.serdes0) {
				status = 1;
				if(xpcs_num)
					s_err_stats.serdes1 = err_count;
				else
					s_err_stats.serdes0 = err_count;
			}

			if ((ETH_serdes_reg_read(xpcs_num, 0x1002) & BIT(15)))
				tx_eq_main = (ETH_serdes_reg_read(xpcs_num, 0x1002)&GENMASK(14, 9)) >> 9;
			else
				tx_eq_main = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN);
			if ((ETH_serdes_reg_read(xpcs_num, 0x1003)& BIT(6)))
				tx_eq_pre = (ETH_serdes_reg_read(xpcs_num, 0x1003)&GENMASK(5, 0)) >> 0;
			else
				tx_eq_pre = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE);
			if ((ETH_serdes_reg_read(xpcs_num, 0x1003) & BIT(13)))
				tx_eq_post = (ETH_serdes_reg_read(xpcs_num, 0x1003)&GENMASK(12, 7)) >> 7;
			else
				tx_eq_post = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_1);
			vboost_en = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0);
			vboost_lvl = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_LVL);
			tx_iboost = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_BOOST_CTRL, TX0_IBOOST);
			tx_rate = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE);
			rx_rate = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_2_0);
			tx_witdh = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH);
			rx_width = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH);

			cont_adpat = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0);

			if (ETH_serdes_reg_read(xpcs_num, 0x100a) & BIT(10))
				rx_eq_att_lvl =((ETH_serdes_reg_read(xpcs_num, 0x1009) & GENMASK(2, 0)) >> 0);
			else
				rx_eq_att_lvl = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_ATTN_CTRL, RX0_EQ_ATT_LVL);
			if (ETH_serdes_reg_read(xpcs_num, 0x100a) & BIT(10))
				ctle_pole = ((ETH_serdes_reg_read(xpcs_num, 0x100a) & GENMASK(1, 0)) >> 0);
			else
				ctle_pole = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0);
			if (ETH_serdes_reg_read(xpcs_num, 0x100a) & BIT(10))
				vga1_gain = ((ETH_serdes_reg_read(xpcs_num, 0x1009) & GENMASK(5, 3)) >> 3);
			else
				vga1_gain = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0);
			if (ETH_serdes_reg_read(xpcs_num, 0x100a) & BIT(10))
				vga2_gain =((ETH_serdes_reg_read(xpcs_num, 0x1009) & GENMASK(8, 6)) >> 6);
			else
				vga2_gain = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0);
			if (ETH_serdes_reg_read(xpcs_num, 0x100a) & BIT(10))
				ctle_boost =((ETH_serdes_reg_read(xpcs_num, 0x1009) & GENMASK(13, 9)) >> 9);
			else
				ctle_boost = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0);
			if (ETH_serdes_reg_read(xpcs_num, 0x100a) & BIT(10))
				dfe_tap1 =((ETH_serdes_reg_read(xpcs_num, 0x100a) & GENMASK(9, 2)) >> 2);
			else
				dfe_tap1 = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_DFE_TAP_CTRL0, DFE_TAP1_0);
			//add by long
			rx_adapt_done = ((ETH_serdes_reg_read(xpcs_num, 0x305e) & GENMASK(1, 0)) >> 0);
			rx_dfe_bypass = ((ETH_serdes_reg_read(xpcs_num, 0x3009) & BIT(10)) >> 10);
			rx_cont_adapt = ETH_serdes_reg_read(xpcs_num, 0x30c6) & BIT(0);
			rx_att_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3054) & GENMASK(7, 0)) >> 5);
			rx_vga_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3055) & GENMASK(8, 0)) >> 6);
			rx_ctle_pole_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3056) & GENMASK(11, 10)) >> 10);
			rx_ctle_boost_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3056) & GENMASK(9, 0)) >> 5);
			dfe_status = ((ETH_serdes_reg_read(xpcs_num, 0x3057) & BIT(12)) >> 12);
			if(dfe_status)
				dfe_tap1_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3057) & GENMASK(12, 0)) >> 5) - 256;
			else
				dfe_tap1_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3057) & GENMASK(12, 0)) >> 5);
			dfe_tap2_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3058) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap3_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x3059) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap4_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x305a) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap5_adapt_val = ((ETH_serdes_reg_read(xpcs_num, 0x305b) & GENMASK(11, 0)) >> 5) - 64;

			dig_rx_dpll_freq =(ETH_serdes_reg_read(xpcs_num, 0x1059) & GENMASK(13, 0));

			printf("serdes%d%8s,%s,%15s,%5d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%d;\n",
					xpcs_num,
					"",
					sensor_buf,
					status ? "1" : "0",
					err_count,
					tx_eq_main,
					tx_eq_pre,
					tx_eq_post,
					vboost_en,
					vboost_lvl,
					tx_iboost,
					tx_rate,
					rx_rate,
					tx_witdh,
					rx_width,
					cont_adpat,
					rx_eq_att_lvl,
					ctle_pole,
					vga1_gain,
					vga2_gain,
					ctle_boost,
					dfe_tap1,
					rx_adapt_done,
					rx_dfe_bypass,
					rx_cont_adapt,
					rx_att_adapt_val,
					rx_vga_adapt_val,
					rx_ctle_pole_adapt_val,
					rx_ctle_boost_adapt_val,
					dfe_tap1_adapt_val,
					dfe_tap2_adapt_val,
					dfe_tap3_adapt_val,
					dfe_tap4_adapt_val,
					dfe_tap5_adapt_val,
					dig_rx_dpll_freq
						);
		}
		int gen_num = (readl(0xc00000a0) & GENMASK(3, 0)) >> 0;
		err_count  = 0;
		err_count = PCIE_serdes_reg_read(0x1052);
		if (err_count > s_err_stats.pcie) {
			status = 1;
			s_err_stats.pcie = err_count;
		}
		else
			status = 0;

		if ((PCIE_serdes_reg_read(0x1002) & BIT(15)))
			tx_eq_main = (PCIE_serdes_reg_read(0x1002)&GENMASK(14, 9)) >> 9;
		else
			tx_eq_main = (PCIE_serdes_reg_read(0x100e)&GENMASK(11, 6)) >> 6;
		if ((PCIE_serdes_reg_read(0x1003)& BIT(6)))
			tx_eq_pre = (PCIE_serdes_reg_read(0x1003)&GENMASK(5, 0)) >> 0;
		else
			tx_eq_pre = (PCIE_serdes_reg_read(0x100f)&GENMASK(5, 0)) >> 0;
		if ((PCIE_serdes_reg_read(0x1003) & BIT(13)))
			tx_eq_post = (PCIE_serdes_reg_read(0x1003)&GENMASK(12, 7)) >> 7;
		else
			tx_eq_post = (PCIE_serdes_reg_read(0x100f)&GENMASK(12, 7)) >> 7;
		if (PCIE_serdes_reg_read(0x1002) & BIT(8))
			vboost_en = (PCIE_serdes_reg_read(0x1002) & BIT(7)) >> 7;
		else
			vboost_en = (PCIE_serdes_reg_read(0x100e) & BIT(5)) >> 5;
		if (PCIE_serdes_reg_read(0x12) & BIT(9))
			vboost_lvl = (PCIE_serdes_reg_read(0x12)&GENMASK(8, 6)) >> 6;
		else
			vboost_lvl = (PCIE_serdes_reg_read(0x1c)&GENMASK(7, 5)) >> 5;
		if (PCIE_serdes_reg_read(0x1002) & BIT(8))
			tx_iboost = (PCIE_serdes_reg_read(0x1002)&GENMASK(6, 3)) >> 3;
		else
			tx_iboost = (PCIE_serdes_reg_read(0x100e)&GENMASK(4, 1)) >> 1;

		if (PCIE_serdes_reg_read(0x1001) & BIT(15))
			tx_rate =((PCIE_serdes_reg_read(0x1001) & GENMASK(10, 8)) >> 8);
		else
			tx_rate =((PCIE_serdes_reg_read(0x100d) & GENMASK(10, 8)) >> 8);

		if (PCIE_serdes_reg_read(0x1005) & BIT(13))
			rx_rate = ((PCIE_serdes_reg_read(0x1005) & GENMASK(10, 9)) >> 9);
		else
			rx_rate = ((PCIE_serdes_reg_read(0x1011) & GENMASK(8, 7)) >> 7);

		if (PCIE_serdes_reg_read(0x1001) & BIT(15))
			tx_witdh = ((PCIE_serdes_reg_read(0x1001) & GENMASK(12, 11)) >> 11);
		else
			tx_witdh = ((PCIE_serdes_reg_read(0x100d) & GENMASK(12, 11)) >> 11);

		if (PCIE_serdes_reg_read(0x1005) & BIT(13))
			rx_width = ((PCIE_serdes_reg_read(0x1005) & GENMASK(10, 9)) >> 9);
		else
			rx_width = ((PCIE_serdes_reg_read(0x1011) & GENMASK(10, 9)) >> 9);

		if (PCIE_serdes_reg_read(0x3008) & BIT(8))	//确认下PCIE GEN2/3/4该值
			cont_adpat = (PCIE_serdes_reg_read(0x3008) & BIT(6)) >> 6;
		else
			cont_adpat = (PCIE_serdes_reg_read(0x3009) & BIT(12)) >> 12;

		if (PCIE_serdes_reg_read(0x100a) & BIT(10))
			rx_eq_att_lvl =((PCIE_serdes_reg_read(0x1009) & GENMASK(2, 0)) >> 0);
		else
			rx_eq_att_lvl =((PCIE_serdes_reg_read(0x1013) & GENMASK(2, 0)) >> 0);

		if (PCIE_serdes_reg_read(0x100a) & BIT(10))
			ctle_pole = ((PCIE_serdes_reg_read(0x100a) & GENMASK(1, 0)) >> 0);
		else
			ctle_pole = ((PCIE_serdes_reg_read(0x1014) & GENMASK(1, 0)) >> 0);

		if (PCIE_serdes_reg_read(0x100a) & BIT(10))
			vga1_gain =   ((PCIE_serdes_reg_read(0x1009) & GENMASK(5, 3)) >> 3);
		else
			vga1_gain =   ((PCIE_serdes_reg_read(0x1013) & GENMASK(5, 3)) >> 3);

		if (PCIE_serdes_reg_read(0x100a) & BIT(10))
			vga2_gain =((PCIE_serdes_reg_read(0x1009) & GENMASK(8, 6)) >> 6);
		else
			vga2_gain =((PCIE_serdes_reg_read(0x1013) & GENMASK(8, 6)) >> 6);

		if (PCIE_serdes_reg_read(0x100a) & BIT(10))
			ctle_boost =((PCIE_serdes_reg_read(0x1009) & GENMASK(13, 9)) >> 9);
		else
			ctle_boost =((PCIE_serdes_reg_read(0x1013) & GENMASK(13, 9)) >> 9);

		if (PCIE_serdes_reg_read(0x100a) & BIT(10))
			dfe_tap1 =((PCIE_serdes_reg_read(0x100a) & GENMASK(9, 2)) >> 2);
		else
			dfe_tap1 =((PCIE_serdes_reg_read(0x1014) & GENMASK(9, 2)) >> 2);
		if(gen_num == 4)
			rx_adapt_done = ((PCIE_serdes_reg_read(0x3069) & GENMASK(1, 0)) >> 0);
		else
			rx_adapt_done = ((PCIE_serdes_reg_read(0x305e) & GENMASK(1, 0)) >> 0);

		rx_dfe_bypass = ((PCIE_serdes_reg_read(0x1011) & BIT(12)) >> 12);
		rx_cont_adapt = PCIE_serdes_reg_read(0x30c6) & BIT(0);

		rx_att_adapt_val = ((PCIE_serdes_reg_read(0x106b) & GENMASK(7, 0)) >> 5);
		rx_vga_adapt_val = ((PCIE_serdes_reg_read(0x106c) & GENMASK(8, 0)) >> 6);
		rx_ctle_pole_adapt_val = ((PCIE_serdes_reg_read(0x106d) & GENMASK(11, 10)) >> 10);
		rx_ctle_boost_adapt_val = ((PCIE_serdes_reg_read(0x106d) & GENMASK(9, 0)) >> 5);
		dfe_status = ((PCIE_serdes_reg_read(0x106e) & BIT(12)) >> 12);
		if(dfe_status)
			dfe_tap1_adapt_val = ((PCIE_serdes_reg_read(0x106e) & GENMASK(12, 0)) >> 5) - 256;
		else
			dfe_tap1_adapt_val = ((PCIE_serdes_reg_read(0x106e) & GENMASK(12, 0)) >> 5);
		dfe_tap2_adapt_val = ((PCIE_serdes_reg_read(0x106f) & GENMASK(11, 0)) >> 5) - 64;
		dfe_tap3_adapt_val = ((PCIE_serdes_reg_read(0x1070) & GENMASK(11, 0)) >> 5) - 64;
		dfe_tap4_adapt_val = ((PCIE_serdes_reg_read(0x1071) & GENMASK(11, 0)) >> 5) - 64;
		dfe_tap5_adapt_val = ((PCIE_serdes_reg_read(0x1072) & GENMASK(11, 0)) >> 5) - 64;
		dig_rx_dpll_freq =(PCIE_serdes_reg_read(0x1059) & GENMASK(13, 0));

		printf("%14s%d,%s,%15s,%5d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%d;\n",
				"pcie_asic_GEN",
				gen_num,
				sensor_buf,
				status ? "1" : "0",
				err_count,
				tx_eq_main,
				tx_eq_pre,
				tx_eq_post,
				vboost_en,
				vboost_lvl,
				tx_iboost,
				tx_rate,
				rx_rate,
				tx_witdh,
				rx_width,
				cont_adpat,
				rx_eq_att_lvl,
				ctle_pole,
				vga1_gain,
				vga2_gain,
				ctle_boost,
				dfe_tap1,
				rx_adapt_done,
				rx_dfe_bypass,
				rx_cont_adapt,
				rx_att_adapt_val,
				rx_vga_adapt_val,
				rx_ctle_pole_adapt_val,
				rx_ctle_boost_adapt_val,
				dfe_tap1_adapt_val,
				dfe_tap2_adapt_val,
				dfe_tap3_adapt_val,
				dfe_tap4_adapt_val,
				dfe_tap5_adapt_val,
				dig_rx_dpll_freq);

		//	offset += sprintf(str + offset, "\x1b[1F\x1b[2K");

		if (PCIE_serdes_reg_read(0x301c) & BIT(9))
			rx_eq_att_lvl =((PCIE_serdes_reg_read(0x301c) & GENMASK(8, 6)) >> 6);
		else
			rx_eq_att_lvl =((PCIE_serdes_reg_read(0x300c) & GENMASK(2, 0)) >> 0);

		if (PCIE_serdes_reg_read(0x301c) & BIT(9))
			ctle_pole =((PCIE_serdes_reg_read(0x301f) & GENMASK(14, 13)) >> 13);
		else
			ctle_pole =((PCIE_serdes_reg_read(0x300d) & GENMASK(1, 0)) >> 0);

		if (PCIE_serdes_reg_read(0x301c) & BIT(9))
			vga1_gain = ((PCIE_serdes_reg_read(0x301c) & GENMASK(2, 0)) >> 0);
		else
			vga1_gain = ((PCIE_serdes_reg_read(0x300c) & GENMASK(5, 3)) >> 3);

		if (PCIE_serdes_reg_read(0x301c) & BIT(9))
			vga2_gain =((PCIE_serdes_reg_read(0x301c) & GENMASK(5, 3)) >> 3);
		else
			vga2_gain =((PCIE_serdes_reg_read(0x300c) & GENMASK(8, 6)) >> 6);

		if (PCIE_serdes_reg_read(0x301c) & BIT(9))
			ctle_boost =((PCIE_serdes_reg_read(0x301f) & GENMASK(12, 8)) >> 8);
		else
			ctle_boost =((PCIE_serdes_reg_read(0x300c) & GENMASK(13, 9)) >> 9);

		if (PCIE_serdes_reg_read(0x3000) & BIT(12))
			tx_rate =((PCIE_serdes_reg_read(0x3000) & GENMASK(7, 5)) >> 5);
		else
			tx_rate =((PCIE_serdes_reg_read(0x3002) & GENMASK(9, 7)) >> 7);

		if (PCIE_serdes_reg_read(0x3005) & BIT(7))
			rx_rate = ((PCIE_serdes_reg_read(0x3005) & GENMASK(1, 0)) >> 0);
		else
			rx_rate = ((PCIE_serdes_reg_read(0x3009) & GENMASK(2, 1)) >> 1);

		if (PCIE_serdes_reg_read(0x3000) & BIT(12))
			tx_witdh = ((PCIE_serdes_reg_read(0x3000) & GENMASK(4, 3)) >> 3);
		else
			tx_witdh = ((PCIE_serdes_reg_read(0x3002) & GENMASK(6, 5)) >> 5);

		if (PCIE_serdes_reg_read(0x3005) & BIT(7))
			rx_width = ((PCIE_serdes_reg_read(0x3005) & GENMASK(3, 2)) >> 2);
		else
			rx_width = ((PCIE_serdes_reg_read(0x3009) & GENMASK(4, 3)) >> 3);

		if (PCIE_serdes_reg_read(0x301c) & BIT(9))
			dfe_tap1 =((PCIE_serdes_reg_read(0x301f) & GENMASK(7, 0)) >> 0);
		else
			dfe_tap1 =((PCIE_serdes_reg_read(0x300d) & GENMASK(9, 2)) >> 2);
		if(gen_num == 4)
			rx_adapt_done = ((PCIE_serdes_reg_read(0x3069) & GENMASK(1, 0)) >> 0);
		else
			rx_adapt_done = ((PCIE_serdes_reg_read(0x305e) & GENMASK(1, 0)) >> 0);
		rx_dfe_bypass = ((PCIE_serdes_reg_read(0x3009) & BIT(10)) >> 10);
		rx_cont_adapt = PCIE_serdes_reg_read(0x30c6) & BIT(0);
		if(gen_num == 4)
		{
			rx_att_adapt_val = ((PCIE_serdes_reg_read(0x305f) & GENMASK(7, 0)) >> 5);
			rx_vga_adapt_val = ((PCIE_serdes_reg_read(0x3060) & GENMASK(8, 0)) >> 6);
			rx_ctle_pole_adapt_val = ((PCIE_serdes_reg_read(0x3061) & GENMASK(11, 10)) >> 10);
			rx_ctle_boost_adapt_val = ((PCIE_serdes_reg_read(0x3061) & GENMASK(9, 0)) >> 5);
			dfe_status = ((PCIE_serdes_reg_read(0x3062) & BIT(12)) >> 12);
			if(dfe_status)
				dfe_tap1_adapt_val = ((PCIE_serdes_reg_read(0x3062) & GENMASK(12, 0)) >> 5) - 256;
			else
				dfe_tap1_adapt_val = ((PCIE_serdes_reg_read(0x3062) & GENMASK(12, 0)) >> 5);
			dfe_tap2_adapt_val = ((PCIE_serdes_reg_read(0x3063) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap3_adapt_val = ((PCIE_serdes_reg_read(0x3064) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap4_adapt_val = ((PCIE_serdes_reg_read(0x3065) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap5_adapt_val = ((PCIE_serdes_reg_read(0x3066) & GENMASK(11, 0)) >> 5) - 64;

		}
		else
		{
			rx_att_adapt_val = ((PCIE_serdes_reg_read(0x3054) & GENMASK(7, 0)) >> 5);
			rx_vga_adapt_val = ((PCIE_serdes_reg_read(0x3055) & GENMASK(8, 0)) >> 6);
			rx_ctle_pole_adapt_val = ((PCIE_serdes_reg_read(0x3056) & GENMASK(11, 10)) >> 10);
			rx_ctle_boost_adapt_val = ((PCIE_serdes_reg_read(0x3056) & GENMASK(9, 0)) >> 5);
			dfe_status = ((PCIE_serdes_reg_read(0x3057) & BIT(12)) >> 12);
			if(dfe_status)
				dfe_tap1_adapt_val = ((PCIE_serdes_reg_read(0x3057) & GENMASK(12, 0)) >> 5) - 256;
			else
				dfe_tap1_adapt_val = ((PCIE_serdes_reg_read(0x3057) & GENMASK(12, 0)) >> 5);
			dfe_tap2_adapt_val = ((PCIE_serdes_reg_read(0x3058) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap3_adapt_val = ((PCIE_serdes_reg_read(0x3059) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap4_adapt_val = ((PCIE_serdes_reg_read(0x305a) & GENMASK(11, 0)) >> 5) - 64;
			dfe_tap5_adapt_val = ((PCIE_serdes_reg_read(0x305b) & GENMASK(11, 0)) >> 5) - 64;

		}

		dig_rx_dpll_freq =(PCIE_serdes_reg_read(0x1059) & GENMASK(13, 0));
		printf("%14s%d,%s,%15s,%5d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%2d,%d;\n",
				"pcie_pcs_GEN",
				gen_num,
				sensor_buf,
				status ? "1" : "0",
				err_count,
				tx_eq_main,
				tx_eq_pre,
				tx_eq_post,
				vboost_en,
				vboost_lvl,
				tx_iboost,
				tx_rate,
				rx_rate,
				tx_witdh,
				rx_width,
				cont_adpat,
				rx_eq_att_lvl,
				ctle_pole,
				vga1_gain,
				vga2_gain,
				ctle_boost,
				dfe_tap1,
				rx_adapt_done,
				rx_dfe_bypass,
				rx_cont_adapt,
				rx_att_adapt_val,
				rx_vga_adapt_val,
				rx_ctle_pole_adapt_val,
				rx_ctle_boost_adapt_val,
				dfe_tap1_adapt_val,
				dfe_tap2_adapt_val,
				dfe_tap3_adapt_val,
				dfe_tap4_adapt_val,
				dfe_tap5_adapt_val,
				dig_rx_dpll_freq);

		//	offset += sprintf(str + offset, "\x1b[1F\x1b[2K");

		sleep(sleep_sec);

		//printf("%s", str);

		title = g_serdes_config.title;
	}

	save_err_stats(&s_err_stats);
}

//4.1.2.	Serdes启动程序, 可靠性测试用
//xcps_num=: 0---xpcs0/serdes0; 1--- xcps1/serdes1;
//serdes_mode: 0---USXGMII; 1---MP-USXGMII(2port); 2---QSGMII; 3---SGMII+; 4---SGMII;
//usxg_mode: 0---10G-SXGMII; 1---5G-SXGMII; 2---2.5G-SXGMII; 3---10G-DXGMII; 4---5G-DXGMII; 5---10G-QXGMII
void Serdes_init_proc_test(
		int xpcs_num,
		int serdes_mode,
		int usxg_mode,
		int rxadpt_en,
		int tx_eq_main_in,
		int  tx_eq_pre_in,
		int tx_eq_post_in,
		int vga1_gain_0_val,
		int vga2_gain_0_val,
		int ctle_pole_0_val,
		int ctle_boost_0_val)
{
	int status;
	int data_rate;
	int link_speed, duplex_mode, link_status, eee, eee_clk;
	int tx_main_reg, tx_post_pre_reg;
	if (xpcs_num == 0) {
		//********************SRAM加载*************************
		//status = 0;
		//while (status == 0) {											//等待SRAM初始化完成
		//	status0 = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
		//}
		////wait 80ns;							//该等待期间可修改加载到SRAM的firmware
		//sleep_nano(80);
		//	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, //EXT_LD_DN, 1);		//指示SRAM外部加载完成
		//********************SRAM加载*************************
		//status = 1;
		//while (status == 1) {											//等待软复位释放，0为解复位
		//	status = REG_R(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL1, RST);
		//}

		if ((serdes_mode == 0) || (serdes_mode == 1)) {				//USXGMII || MP-USXGMII
			if ((usxg_mode == 0) || (usxg_mode == 3) || (usxg_mode == 5))
				data_rate = 0;
			else if ((usxg_mode == 1) || (usxg_mode == 4))
				data_rate = 1;
			else if (usxg_mode == 2)
				data_rate = 2;
			else
				printf("Invalid USXGMII mode. \n");

			if (serdes_mode == 0)
				REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 0);
			else if (serdes_mode == 1) {
				REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 0);
				REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 0);
				//REG_W(XGMAC2_SFT_RST_N, xgmac2_sft_rst_n, 0);		//Dubhe1000不支持，Dubhe2000支持
				//REG_W(XGMAC3_SFT_RST_N, xgmac3_sft_rst_n, 0);		//Dubhe1000不支持，Dubhe2000支持
			}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 0);			//Select 10GBASE-R PCS Type
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USXG_EN, 1);				//Enable USXGMII mode
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_KR_CTRL, USXG_2PT5G_GMII, 0);			//Use XGMII interface
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_KR_CTRL, USXG_MODE, usxg_mode);		//USXGMII mode
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 33);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1353);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 41);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, data_rate);		//确认下data_rate值
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_2_0, data_rate);		//确认下data_rate值
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 1);	//使能DIV16.5
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);		//使能DIV10
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);		//关闭DIV8
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);	//databook流程里面没有
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, vga1_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, vga2_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, ctle_pole_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, ctle_boost_0_val);

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 18);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, RX0_DELTA_IQ, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);	//databook有，仿真代码没

			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USXG_EN, 1);								//已配过，是否冗余？
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);
			//********************SRAM加载*************************

			LOG("Wait for XPCS0 SRAM initialization to complete ...\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 0) {											//等待SRAM初始化完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
			}
#endif
			//wait 80ns;							//该等待期间可修改加载到SRAM的firmware
			sleep_nano(80);

			if (g_serdes_fw)
				for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
					ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
				}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成
			//********************SRAM加载*************************


			LOG("等待软复位释放，0表示释放\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}

			while (status == 1) {											//等待软复位释放，0表示释放
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
			}


			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DEBUG_CTRL, //RX_DT_EN_CTL, 1);			//不支持10Base-(K)R mode不配置
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DEBUG_CTRL, //SUPRESS_LOS_DET, 1);		//不支持10Base-(K)R mode不配置

			LOG("等待DPLL Lock，1表示lock\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_RX_LSTS, RX_VALID_0,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}

			while (status == 0) {											//等待DPLL Lock，1表示lock
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_RX_LSTS, RX_VALID_0);
			}

#if 0
			if (rxadpt_en) {
				//TX_PRBS_Config(0, 1, 0);	//TX PRBS31 enable
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 1);	//发起RX adaptation

				LOG("等待RX adapt响应，1表示响应\n");
				status = 0;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, RX_ADAPT_ACK,
							status, status != 0, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}

				while (status == 0) {											//等待RX adapt响应，1表示响应
					status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, RX_ADAPT_ACK);
				}
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 0);	//关闭RX adaptation
			}

#endif

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);				//4-bit MII
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, SGMII_LINK_STS, 1);		//Link Up, PHY Side时配置
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, TX_CONFIG, 0);			//MAC Side
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_AN_INTR_EN, 1);		//CL37 AN Complete Interrupt Enable
			//!!! BIT 9  AUTOSW
			//(Option)Duration=CL37_LINK_TIME*6.4ns
			//#define   CL37_LINK_TIME_VAL        0
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_LINK_TIMER_CTRL, CL37_LINK_TIME, CL37_LINK_TIME_VAL);
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, CL37_TMR_OVR_RIDE, 1);
			/*
			//enable auto-negotiation时不配置
			//当不支持auto-negotiation时，需要获取对端设配的速率配置
			if (usxg_mode == 0) {		//10G-SXGMII
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS13, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS6, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS3, 0);
			}
			else if (usxg_mode == 1) {		//5G-SXGMII
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS13, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS6, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS3, 1);
			}
			else if (usxg_mode == 2) {		//2.5G-SXGMII
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS13, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS6, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS3, 1);
			}
			...
			*/

#if 0
			//配置auto-negotiation
			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, AN_ENABLE, 1);

			LOG("CL37 AN Complete Interrupt, 1代表完成\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
			while (status == 0) {						//CL37 AN Complete Interrupt, 1代表完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR);
			}
#endif
			REG_W(XPCS_BASE_ADDR(xpcs_num) +  VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR, 0);

			//Read REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS);
			link_status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_14);		//0---Link is Down; 1---Link is Up
			duplex_mode = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_13);		//0---Half Duplex; 1---Full Duplex
			link_speed = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_12_10);		//000:10M;001:100M;010:1000M;011: 10G;100:2.5G;101:5G
			eee = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_9);				//0:EEE not support; 1:EEE support
			eee_clk = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_8);			//0:EEE clock stop not support; 1:EEE clock-stop support

			if (link_status == 1)
				printf("USXGMII Link Up. \n");
			else
				printf("USXGMII Link Down. \n");
			if (duplex_mode == 1)
				printf("Full Duplex. \n");
			else
				printf("Half Duplex. \n");
			if (link_speed == 0)
				printf("USXGMII Link Speed: 10Mbps. \n");
			else if (link_speed == 1)
				printf("USXGMII Link Speed: 100Mbps. \n");
			else if (link_speed == 2)
				printf("USXGMII Link Speed: 1000Mbps. \n");
			else if (link_speed == 3)
				printf("USXGMII Link Speed: 10Gbps. \n");
			else if (link_speed == 4)
				printf("USXGMII Link Speed: 2.5Gbps. \n");
			else if (link_speed == 5)
				printf("USXGMII Link Speed: 5Gbps. \n");
			else
				printf("USXGMII Link Speed: Invalid. \n");
			if (eee == 1)
				printf("EEE support. \n");
			else
				printf("EEE not support. \n");
			if (eee_clk == 1)
				printf("EEE clock-stop support. \n");
			else
				printf("EEE clock-stop not support. \n");

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR, 0);		//Clear interrupt
			//wait 1us;												//等待XGMII clock0稳定
#if 0
			sleep_nano(1 * 1000);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USRA_RST_1, 1);				//port0
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USXG_EN, 1);					//仿真上冗余

			LOG("Wait for USRA_RST_1 Set 0\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USRA_RST_1,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
			while (status == 1) {						//
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USRA_RST_1);
			}
#endif
			if ((usxg_mode == 3) || (usxg_mode == 4)) {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, MAC_AUTO_SW, 1);			//databook没有
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2, 1);

				LOG("Wait for USRA_RST_2 Set 0\n");
				status = 1;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2,
							status, status != 1, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}
#if 0
				while (status == 1) {						//
					status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2);
				}
#endif
			}

			if (usxg_mode == 5) {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, MAC_AUTO_SW, 1);			//databook没有
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2, 1);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_2_DIG_CTRL1, USRA_RST_2, 1);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_3_DIG_CTRL1, USRA_RST_2, 1);

				LOG("Wait for Port1 USRA_RST_2 Set 0\n");
				status = 1;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2,
							status, status != 1, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}
#if 0
				while (status == 1) {						//
					status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2);
				}
#endif

				LOG("Wait for Port2 USRA_RST_2 Set 0\n");
				status = 1;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_2_DIG_CTRL1, USRA_RST_2,
							status, status != 1, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}
#if 0
				while (status == 1) {						//
					status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_2_DIG_CTRL1, USRA_RST_2);
				}
#endif

				LOG("Wait for Port3 USRA_RST_2 Set 0\n");
				status = 1;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_3_DIG_CTRL1, USRA_RST_2, status, status != 1, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}
#if 0
				while (status == 1) {						//
					status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_3_DIG_CTRL1, USRA_RST_2);
				}
#endif
			}

			if (serdes_mode == 0)
				REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 1);
			else if (serdes_mode == 1) {
				REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 1);
				REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 1);
				//REG_W(XGMAC2_SFT_RST_N, xgmac2_sft_rst_n, 1);		//Dubhe1000不支持，Dubhe2000支持
				//REG_W(XGMAC3_SFT_RST_N, xgmac3_sft_rst_n, 1);		//Dubhe1000不支持，Dubhe2000支持
			}

			//TX Main=30
			tx_main_reg = (ETH_serdes_reg_read(0,  0x1002) & ~GENMASK(14, 9)) + (tx_eq_main_in << 9) + 0x8000;
			tx_post_pre_reg = (tx_eq_post_in << 7) + tx_eq_pre_in + 0x2040;
			ETH_serdes_reg_write(0, 0x1002, tx_main_reg);
			//TX Post=40
			ETH_serdes_reg_write(0, 0x1003, tx_post_pre_reg);
			//TX_FFE_Config(0, tx_eq_main_in, tx_eq_pre_in, tx_eq_post_in);
			TX_PRBS_Config(0, 1, 0);	//TX PRBS31 enable
			if(rxadpt_en) {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 1);	//发起RX adaptation

				LOG("等待RX adapt响应，1表示响应\n");
				status = 0;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, RX_ADAPT_ACK,
							status, status != 0, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 0);	//关闭RX adaptation
			}
			msleep(100);
			RX_PRBS_Config(0, 1);	//RX PRBS31 check enable
			status = RX_err_count(0);
			TX_err_Insert(0);
			status = RX_err_count(0);
			status = RX_err_count(0);
			if(status == 1)
				printf("Serdes0 tx/rx prbs ok\n");
		}
		else if ((serdes_mode == 3) ||(serdes_mode == 4)) {			//SGMII+ || SGMII
			REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 0);		//

			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, TX_CONFIG, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, PCS_MODE, 0);
			if (serdes_mode==4) {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL1, SS13, 0);
			}
			//配置PCS工作模式
			//TO_DO
			if (serdes_mode==4) { //2.5G SGMII+
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 40);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 40983);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1360);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 34);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 2);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 2);
			}
			else { //1G SGMII
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 32);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1344);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 42);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 3);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 3);
			}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, ctle_boost_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, vga1_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, vga2_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, ctle_pole_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);

			if (serdes_mode==4) { //2.5G SGMII+
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 10);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 23);
			}
			else {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 22);
			}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 1);

			//phy soft reset
			if (serdes_mode == 4) {	//2.5G mode
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, EN_2_5G_MODE, 1);
			}
			else {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, EN_2_5G_MODE, 0);
			}
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);
			//********************SRAM加载*************************

			LOG("等待SRAM初始化完成\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 0) {											//等待SRAM初始化完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
			}
#endif
			//wait 80ns;							//该等待期间可修改加载到SRAM的firmware
			sleep_nano(80);

			if (g_serdes_fw)
				for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
					ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
				}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成
			//********************SRAM加载*************************

			LOG("等待软复位释放，0表示释放\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 1) {											//等待软复位释放，0表示释放
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
			}
#endif
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 40);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_1, 0);

			REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 1);
		}else if (serdes_mode == 5) {			//SGMII+ || SGMII
			REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 0);		//

			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, TX_CONFIG, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, PCS_MODE, 1);
			//配置PCS工作模式
			//TO_DO
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 32);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1344);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 42);   //42--1.25G config, 34--3.125g config
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 2);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 2);

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, ctle_boost_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, vga1_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, vga2_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, ctle_pole_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);


			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 22);

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 1);

			//phy soft reset
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, EN_2_5G_MODE, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);
			//********************SRAM加载*************************

			LOG("等待SRAM初始化完成\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 0) {											//等待SRAM初始化完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
			}
#endif
			//wait 80ns;							//该等待期间可修改加载到SRAM的firmware
			sleep_nano(80);

			if (g_serdes_fw)
				for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
					ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
				}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成
			//********************SRAM加载*************************

			LOG("等待软复位释放，0表示释放\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 1) {											//等待软复位释放，0表示释放
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
			}
#endif
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 40);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_1, 0);

			REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 1);
		}
		else {
			printf("Invalid serdes_mode, xpcs0 not support. \n");
			return;
		}

	}

	if (xpcs_num == 1) {
		//********************SRAM加载*************************
		//status = 0;
		//while (status == 0) {										//等待SRAM初始化完成，1表示完成
		//	status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
		//}
		////wait 80ns;							//该等待期间可修改加载到SRAM的firmware
		//sleep_nano(80);
		//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, //EXT_LD_DN, 1);		//指示SRAM外部加载完成
		//********************SRAM加载*************************
		//status = 1;
		//while (status == 1) {											//等待软复位释放，0为解复位
		//	status = RST(SR_MII_CTRL[15])；
		//}

		if (serdes_mode == 2) {									//QSGMII Serdes启动流程
			//复位XGMAC
			REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 0);			//QSGMII连接XGMAC1/2/3/4
			REG_W(XGMAC2_SFT_RST_N, xgmac2_sft_rst_n, 0);			//使用上可只用2个或3个XGMAC
			REG_W(XGMAC3_SFT_RST_N, xgmac3_sft_rst_n, 0);
			REG_W(XGMAC4_SFT_RST_N, xgmac4_sft_rst_n, 0);

			//REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 1);		//KR/10GBASE-R support
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL1, SS13, 0);					//Dubhe1000不存在，Dubhe2000存在
			//!!! 加其他3个
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, PCS_MODE, 3);				//配置为QSGMII mode
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);					//4-bit MII
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_AN_INTR_EN, 1);			//CL37 AN Complete Interrupt Enable
			//!!!!BIT 9

			//MPLL配置
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 32);		//PLL FB divider
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41013);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1344);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 42);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0);		//关闭连续自适应
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 1);			//baud/2
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 1);			//baud/2
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3);				//QSGMII---20bit
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3);				//QSGMII---20bit
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0);	//关闭DIV16.5
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);		//使能DIV10
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);		//关闭DIV8
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0);			//关闭TX Boost
			//Serdes RX配置
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, ctle_boost_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, vga1_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, vga2_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, ctle_pole_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 20);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);		//指示TX input clock Ready

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);							//软复位XPCS和PHY

			//********************SRAM加载*************************

			LOG("等待SRAM初始化完成，1表示完成\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 0) {										//等待SRAM初始化完成，1表示完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
			}
#endif
			//wait 80ns;								//该等待期间可修改加载到SRAM的firmware
			sleep_nano(80);

			if (g_serdes_fw)
				for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
					ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
				}

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成
			//********************SRAM加载*************************

			LOG("等待软复位释放，0表示释放\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}

#if 0
			while (status == 1) {	//！！加打印										//等待软复位释放，0表示释放
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
			}
#endif
			//配置TX FFE
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 36);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_2, 0);
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_OVR_RIDE, 1);		//不支持CL72，不需要配置

			printf("===>解复位XGMAC\n");
#if 1
			REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 1);			//和复位XGMAC对应
			REG_W(XGMAC2_SFT_RST_N, xgmac2_sft_rst_n, 1);
			REG_W(XGMAC3_SFT_RST_N, xgmac3_sft_rst_n, 1);
			REG_W(XGMAC4_SFT_RST_N, xgmac4_sft_rst_n, 1);
#endif

			TX_PRBS_Config(1, 8, 0);	//TX PRBS7 enable
			RX_PRBS_Config(1, 8);		//RX PRBS7 check enable
			status = RX_err_count(1);
			TX_err_Insert(1);
			status = RX_err_count(1);
			status = RX_err_count(1);
			if(status == 1)
				printf("Serdes1 tx/rx prbs ok\n");
			else
				printf("Serdes1 tx/rx prbs fail\n");
		}
		else if ((serdes_mode == 3) ||(serdes_mode == 4)) {			//SGMII+只Serdes测试用，业务不支持
			REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 0);			//SGMII连接XGMAC1，XGMAC1复位

			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, TX_CONFIG, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, PCS_MODE, 1);
			if (serdes_mode==4) {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL1, SS13, 0);
			}
			//配置PCS工作模式
			//TO_DO
			if (serdes_mode==4) { //2.5G SGMII+
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 40);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 40983);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1360);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 34);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 2);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 2);
			}
			else { //1G SGMII
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 32);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1344);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 42);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 3);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 3);
			}
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, ctle_boost_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, vga1_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, vga2_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, ctle_pole_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);
			if (serdes_mode==4) { //2.5G SGMII+
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 2);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 23);
			}
			else {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 22);
			}
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 1);
			//phy soft reset
			if(serdes_mode==4){	//2.5G SGMII+
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, EN_2_5G_MODE, 1);
			}
			else {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, EN_2_5G_MODE, 0);
			}
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);
			//********************SRAM加载*************************
			LOG("等待SRAM初始化完成\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 0) {											//等待SRAM初始化完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
			}
#endif
			//wait 80ns;							//该等待期间可修改加载到SRAM的firmware
			sleep_nano(80);
			if (g_serdes_fw)
				for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
					ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
				}
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成
			//********************SRAM加载*************************

			LOG("等待软复位释放，0表示释放\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			while (status == 1) {											//等待软复位释放，0表示释放
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
			}
#endif

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 40);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_1, 0);
			REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 1);			//XGMAC1解复位
		} else if (serdes_mode == 0) {								//USXGMII只Serdes测试用，业务不支持
			//REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 0);			//SGMII连接XGMAC1
			printf("Only for Serdes Test!!! \n");
			if (usxg_mode == 0)
				data_rate = 0;
			else if (usxg_mode == 1)
				data_rate = 1;
			else if (usxg_mode == 2)
				data_rate = 2;
			else
				printf("Invalid Serdes1 USXGMII mode.\n");

			REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 0);			//Select 10GBASE-R PCS Type
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USXG_EN, 1);				//Enable USXGMII mode，xpcs1不支持
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_KR_CTRL, USXG_2PT5G_GMII, 0);			//Use XGMII interface，xpcs1不支持
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_KR_CTRL, USXG_MODE, usxg_mode);		//USXGMII mode，xpcs1不支持
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 33);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1353);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 41);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 1);	//RX连续自适应打开
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, data_rate);		//确认下data_rate值
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, data_rate);		//确认下data_rate值
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 1);	//使能DIV16.5
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);		//使能DIV10
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);		//关闭DIV8
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);	//databook流程里面没有
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, vga1_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, vga2_gain_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, ctle_pole_0_val);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, ctle_boost_0_val);

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 18);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 3);
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);

			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST, 1);
			//********************SRAM加载*************************
			LOG("等待SRAM初始化完成，1表示完成\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			status = 0;
			while(status == 0) {											//等待SRAM初始化完成
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, INIT_DN);
			}
#endif
			//wait 80ns；								//该等待期间可修改加载到SRAM的firmware
			sleep_nano(80);
			if (g_serdes_fw)
				for (int reg_index = 0; reg_index < sizeof(serdes_fw_data_list)/sizeof(serdes_fw_data_t); reg_index++ ) {
					ETH_serdes_reg_write(xpcs_num, SERDES_RAM_BASE + serdes_fw_data_list[reg_index].reg, serdes_fw_data_list[reg_index].data);
				}
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_SRAM, EXT_LD_DN, 1);		//指示SRAM外部加载完成
			//********************SRAM加载*************************
			LOG("等待软复位释放，0表示释放\n");
			status = 1;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST,
						status, status != 1, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}

#if 0
			status = 1;
			while(status == 1) {											//等待软复位释放，0表示释放
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, VR_RST);
			}
#endif
			REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);

			LOG("RX链路上有信号\n");
			status = 0;
			if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_RX_LSTS, SIG_DET_0,
						status, status != 0, DEFAULT_TIMEOUT_US)) {
				printf("Line %d: wait status[%d] timeout", __LINE__, status);
				return;
			}
#if 0
			status = 0;
			while(status == 0) {											//等待DPLL Lock，1表示lock
				status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_RX_LSTS, RX_VALID_0);

			}
#endif
			if (rxadpt_en) {
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 1);	//发起RX adaptation

				LOG("等待RX adapt响应，1表示响应\n");
				status = 0;
				if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS,
							RX_ADAPT_ACK, status, status != 0, DEFAULT_TIMEOUT_US)) {
					printf("Line %d: wait status[%d] timeout", __LINE__, status);
					return;
				}
#if 0
				status = 0;
				while(status == 0) {											//等待RX adapt响应，1表示响应
					status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, RX_ADAPT_ACK);
				}
#endif
				REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 0);	//关闭RX adaptation
			}
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);			//4-bit MII
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, SGMII_LINK_STS, 1);		//Link Up, PHY Side时配置
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, TX_CONFIG, 0);			//MAC Side
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_AN_INTR_EN, 1);		//CL37 AN Complete Interrupt Enable
			//REG_W(XPCS_BASE_ADDR(xpcs_num) + XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 1);	//测试Sderdes UXGMII时，MAC1保持复位
		}

	} else {
		printf("Invalid serdes_mode, xpcs1 not support. \n");
		return;
	}
}

typedef struct {
	union {
		u32  raw;
		struct {
			u32  CL37_ANCMP_INTR:1,
				 CL37_ANSGM_STS_DUPLEX:1,
				 CL37_ANSGM_STS_SPEED:2,
				 CL37_ANSGM_STS_LINK:1,
				 LP_EEE_CAP:1,
				 LP_CK_STP:1,
				 Reserved_7:1,
				 USXG_AN_STS_EEE:2,
				 USXG_AN_STS_SPEED:3,
				 USXG_AN_STS_DUPLEX:1,
				 USXG_AN_STS_LINK:1,
				 Reserved_15:1;
		};
	};
} vr_mii_an_intr_sts_t;

typedef struct {
	union {
		u32  raw;
		struct {
			uint32_t reserved_4_0:5,
					 ss5:1,
					 ss6:1,
					 reserved_7:1,
					 duplex_mode:1,
					 restart_an:1,
					 reserved_10:1,
					 lpm:1,
					 an_enable:1,
					 ss13:1,
					 lbe:1,
					 rst:1;
		};
	};
} sr_mii_ctr_reg_st;

#define XPCS_SPEED_10				0
#define XPCS_SPEED_100				1
#define XPCS_SPEED_1000	    		2
#define XPCS_SPEED_10000			3
#define XPCS_SPEED_2500				4
#define XPCS_SPEED_5000				5

typedef enum {
	PHY_INTERFACE_MODE_MII,
	PHY_INTERFACE_MODE_GMII,
	PHY_INTERFACE_MODE_SGMII,
	PHY_INTERFACE_MODE_SGMII_2500,
	PHY_INTERFACE_MODE_QSGMII,
	PHY_INTERFACE_MODE_TBI,
	PHY_INTERFACE_MODE_RMII,
	PHY_INTERFACE_MODE_RGMII,
	PHY_INTERFACE_MODE_RGMII_ID,
	PHY_INTERFACE_MODE_RGMII_RXID,
	PHY_INTERFACE_MODE_RGMII_TXID,
	PHY_INTERFACE_MODE_RTBI,
	PHY_INTERFACE_MODE_1000BASEX,
	PHY_INTERFACE_MODE_2500BASEX,
	PHY_INTERFACE_MODE_XGMII,
	PHY_INTERFACE_MODE_XAUI,
	PHY_INTERFACE_MODE_RXAUI,
	PHY_INTERFACE_MODE_SFI,
	PHY_INTERFACE_MODE_INTERNAL,
	PHY_INTERFACE_MODE_25G_AUI,
	PHY_INTERFACE_MODE_XLAUI,
	PHY_INTERFACE_MODE_CAUI2,
	PHY_INTERFACE_MODE_CAUI4,
	PHY_INTERFACE_MODE_NCSI,
	PHY_INTERFACE_MODE_10GBASER,
	PHY_INTERFACE_MODE_USXGMII,
	PHY_INTERFACE_MODE_NONE,	/* Must be last */

	PHY_INTERFACE_MODE_COUNT,
} phy_interface_t;


static const char * const phy_interface_strings[] = {
	[PHY_INTERFACE_MODE_MII]		= "mii",
	[PHY_INTERFACE_MODE_GMII]		= "gmii",
	[PHY_INTERFACE_MODE_SGMII]		= "sgmii",
	[PHY_INTERFACE_MODE_SGMII_2500]		= "sgmii-2500",
	[PHY_INTERFACE_MODE_QSGMII]		= "qsgmii",
	[PHY_INTERFACE_MODE_TBI]		= "tbi",
	[PHY_INTERFACE_MODE_RMII]		= "rmii",
	[PHY_INTERFACE_MODE_RGMII]		= "rgmii",
	[PHY_INTERFACE_MODE_RGMII_ID]		= "rgmii-id",
	[PHY_INTERFACE_MODE_RGMII_RXID]		= "rgmii-rxid",
	[PHY_INTERFACE_MODE_RGMII_TXID]		= "rgmii-txid",
	[PHY_INTERFACE_MODE_RTBI]		= "rtbi",
	[PHY_INTERFACE_MODE_1000BASEX]		= "1000base-x",
	[PHY_INTERFACE_MODE_2500BASEX]		= "2500base-x",
	[PHY_INTERFACE_MODE_XGMII]		= "xgmii",
	[PHY_INTERFACE_MODE_XAUI]		= "xaui",
	[PHY_INTERFACE_MODE_RXAUI]		= "rxaui",
	[PHY_INTERFACE_MODE_SFI]		= "sfi",
	[PHY_INTERFACE_MODE_INTERNAL]		= "internal",
	[PHY_INTERFACE_MODE_25G_AUI]		= "25g-aui",
	[PHY_INTERFACE_MODE_XLAUI]		= "xlaui4",
	[PHY_INTERFACE_MODE_CAUI2]		= "caui2",
	[PHY_INTERFACE_MODE_CAUI4]		= "caui4",
	[PHY_INTERFACE_MODE_NCSI]		= "NC-SI",
	[PHY_INTERFACE_MODE_10GBASER]		= "10gbase-r",
	[PHY_INTERFACE_MODE_USXGMII]		= "usxgmii",
	[PHY_INTERFACE_MODE_NONE]		= "",
};

void XPCS_an_enable(int xpcs_num,int port, int enable)
{
	if(enable)
		REG_W(XPCS_BASE_ADDR(xpcs_num) +  SR_MII_OFFSET(port) + SR_MII_CTRL, AN_ENABLE, 1);
	else
		REG_W(XPCS_BASE_ADDR(xpcs_num) +  SR_MII_OFFSET(port) + SR_MII_CTRL, AN_ENABLE, 0);
}

void XPCS_mac_auto_sw(int xpcs_num,int port, int enable)
{
	if(enable)
		REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_OFFSET(port) + VR_MII_DIG_CTRL1, MAC_AUTO_SW, 1);
	else
		REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_OFFSET(port) + VR_MII_DIG_CTRL1, MAC_AUTO_SW, 0);
}

void XPCS_speed(int xpcs_num, int phy_interface, int port, unsigned int speed)
{
	sr_mii_ctr_reg_st sr_mii_ctr_reg = {0};
	uint32_t  dig_reg = readl(XPCS_BASE_ADDR(xpcs_num) + SR_MII_OFFSET(port) + VR_MII_DIG_CTRL1);
	sr_mii_ctr_reg.raw = readl(XPCS_BASE_ADDR(xpcs_num) + SR_MII_OFFSET(port) +  SR_MII_CTRL);

	if (sr_mii_ctr_reg.an_enable && (dig_reg & BIT(MAC_AUTO_SW_BEGIN))) {
			printf("%s has already enable MAC_AUTO_SW, so no need to set speed[%d]\n",__FUNCTION__,  speed);
		return;
	}

	switch (speed) {
	case 5000:
		sr_mii_ctr_reg.ss5 = 1;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_2500:
		if (phy_interface == PHY_INTERFACE_MODE_USXGMII) {
			sr_mii_ctr_reg.ss5 = 1;
			sr_mii_ctr_reg.ss6 = 0;
			sr_mii_ctr_reg.ss13 = 0;
		} else {
			sr_mii_ctr_reg.ss5 = 0;
			sr_mii_ctr_reg.ss6 = 1;
			sr_mii_ctr_reg.ss13 = 0;
		}
		break;
	case SPEED_10000:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 1;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_1000:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 1;
		sr_mii_ctr_reg.ss13 = 0;
		break;
	case SPEED_100:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_10:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 0;
		break;
	default:
		break;
	}

	writel(XPCS_BASE_ADDR(xpcs_num) +
			SR_MII_OFFSET(port) + SR_MII_CTRL,
			sr_mii_ctr_reg.raw);

	return;
}

void XPCS_clear_an_interrupt(int xpcs_num, int port)
{
	u32 val = 0;
	vr_mii_an_intr_sts_t sts = {0};

	val = readl(XPCS_BASE_ADDR(xpcs_num) +
			SR_MII_OFFSET(port) + SR_MII_CTRL);

	if (!(val & BIT(AN_ENABLE_BEGIN)))
		return;

	sts.raw = readl(XPCS_BASE_ADDR(xpcs_num) +
			SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS);

	sts.CL37_ANCMP_INTR = 0;

	writel(XPCS_BASE_ADDR(xpcs_num) +  SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS , sts.raw);
}

int phy_get_interface_by_name(const char *str)
{
	int i;

	for (i = 0; i < PHY_INTERFACE_MODE_COUNT; i++) {
		if (!strcmp(str, phy_interface_strings[i]))
			return i;
	}

	return -1;
}

void XPCS_get_link_state(int xpcs_num, int phy_interface, int port)
{
	int speed = 0, link, state, duplex;
	vr_mii_an_intr_sts_t sts = {0};
	sr_mii_ctr_reg_st sr_mii_ctr_reg = {0};
	sr_mii_ctr_reg.raw = readl(XPCS_BASE_ADDR(xpcs_num) +
			SR_MII_OFFSET(port) + SR_MII_CTRL);

	printf("interface: %d\n" , phy_interface);
	if (!sr_mii_ctr_reg.an_enable) {
		printf("an_enabled: 0\n");

		if (phy_interface == PHY_INTERFACE_MODE_USXGMII)
			speed = sr_mii_ctr_reg.ss5 | sr_mii_ctr_reg.ss6 << 1 | sr_mii_ctr_reg.ss13 << 2;
		else
			speed = sr_mii_ctr_reg.ss6 << 1 | sr_mii_ctr_reg.ss13 << 2;

		switch (speed) {
		case  5:
			speed = 5000;
			break;
		case 1:
			if (phy_interface == PHY_INTERFACE_MODE_USXGMII)
				speed = SPEED_2500;
			break;
		case 6:
			speed = SPEED_10000;
			break;
		case 2:
			if (phy_interface == PHY_INTERFACE_MODE_2500BASEX)
				speed = SPEED_2500;
			else
				speed = SPEED_1000;
			break;
		case  4:
			speed = SPEED_100;
			break;
		case  0:
			speed = SPEED_10;
			break;
		default:
			printf("Unkown the speed %d interface_type[%d]",
					speed, phy_interface);
			break;
		}

        printf("speed: %d\n", speed);
        printf("duplex: %d\n", sr_mii_ctr_reg.duplex_mode);
		printf("link: 1\n");

	} else {
			printf("an_enabled: 1\n");
			sts.raw = readl(XPCS_BASE_ADDR(xpcs_num) +
					SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS);

			switch(phy_interface) {
			case PHY_INTERFACE_MODE_SGMII:
			case PHY_INTERFACE_MODE_2500BASEX:
			case PHY_INTERFACE_MODE_QSGMII:
				link = sts.CL37_ANSGM_STS_LINK;
				duplex = sts.CL37_ANSGM_STS_DUPLEX;
				speed = sts.CL37_ANSGM_STS_SPEED;
				break;
			case PHY_INTERFACE_MODE_USXGMII:
				link = sts.USXG_AN_STS_LINK;
				duplex = sts.USXG_AN_STS_DUPLEX;
				speed = sts.USXG_AN_STS_SPEED;
				break;
			default:
				printf("Unkown the interface[%d]", phy_interface);
			}

			switch(speed)  {
			case XPCS_SPEED_10:
				speed = SPEED_10;
				break;
			case XPCS_SPEED_100:
				speed = SPEED_100;
				break;
			case XPCS_SPEED_1000:
				speed = SPEED_1000;
				break;
			case XPCS_SPEED_10000:
				speed = SPEED_10000;
				break;
			case XPCS_SPEED_2500:
				speed = SPEED_2500;
				break;
			case XPCS_SPEED_5000:
				speed = SPEED_5000;
				break;
			}

			printf("speed: %d\n", speed);
			printf("duplex: %d\n", duplex);
			printf("link: %d\n", link);
		}
}
