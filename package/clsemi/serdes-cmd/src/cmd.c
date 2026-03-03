#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "dubhe1000_xpcs_serdes.h"
#include "dubhe1000_mac_stats.h"
#include "dubhe1000_switch_stats.h"
#include "dubhe1000_mac.h"
#include <strings.h>

#include "sys_hal.h"
int g_debug = 0;
int g_serdes_fw = 0;
void PCIE_RX_ADAPT(void);
void Serdes_init_pre(void);
//void Serdes_init_pre(int i);
//xcps_num=0---xpcs0/serdes0; 1--- xcps1/serdes1;
//serdes_mode: 0---USXGMII; 1---MP-USXGMII(2port); 2---QSGMII; 3---SGMII+; 4---SGMII;
//usxg_mode: 0---10G-SXGMII; 1---5G-SXGMII; 2---2.5G-SXGMII; 3---10G-DXGMII; 4---5G-DXGMII; 5---10G-QXGMII
void Serdes_init_proc(int xpcs_num, int serdes_mode, int usxg_mode, int rxadpt_en);
void Serdes_init_proc_test(
int xpcs_num,
int serdes_mode,
int usxg_mode,
int rxadpt_en,
int tx_eq_main_in,
int  tx_eq_req_in,
int tx_eq_post_in,
int vga1_gain_0_val,
int vga2_gain_0_val,
int ctle_pole_0_val,
int ctle_boost_0_val);

void rx_afe_overwride(int xpcs_num, int eq_att, int eq_gain1, int eq_gain2, int eq_pole, int eq_boost, int eq_tap1);
void ETH_serdes_reg_write(int xpcs_num, int reg_addr, int reg_val);
void ETH_RX_ADAPT(int xpcs_num);
int ETH_serdes_reg_read(int xpcs_num, int reg_addr);
void TX_FFE_Config(int xpcs_num, int tx_eq_main, int tx_eq_pre, int tx_eq_post);
void RX_Adapt_enable(int xpcs_num, bool reg_dm);
//xpcs_num: 0---serdes0; 1---serdes1;
//mode: 0---Disable, 1---PRBS31, 2---PRBS23, 3---PRBS23-2, 4---PRBS16, 5---PRBS15, 6---PRBS11, 7---PRBS9, 8---PRBS7
//9---PAT0, 10---(PAT0,~PAT0), 11---(000,PAT0,3ff,~PAT0);
// pat0: user defined pattern
void TX_PRBS_Config(int xpcs_num, int mode, int pat0);
void TX_err_Insert(int xpcs_num);
void RX_PRBS_Config(int xpcs_num, int mode);
int RX_err_count(int xpcs_num);
void Loopback_config(int xpcs_num, int mode, bool reg_dm);
void Repeat_CLK_Config(int xpcs_num, int mode);
void VR_RST(int xpcs_num);
void tx_req(int xpcs_num);
void rx_req(int xpcs_num);
void Serdes_dump(int option);
void TX_Swing_Config(int xpcs_num, int tx_vboost_en, int tx_vboost_lvl, int tx_iboost_lvl);
void ETH_PLL_Config(int refdiv, int fbdiv, int frac, int postdiv1, int postdiv2);
void PCIE_init(int gen, int tx_main, int tx_pre, int tx_post, int rx_adapt_en);

unsigned PCIE_serdes_reg_read(int reg_addr);

unsigned PCIE_serdes_reg_write(int reg_addr, int);

void log_printf(int title);

void reload_conf();
//printf("Get %s val[%#x]\n", #val, val);
#define CONVERT_TO_UINT(val) ({val = strtoul(argv[offset++], NULL, 0); \
	 						})

#define CONVERT_TO_STRING(val) ({val = argv[offset++]; \
	 						})

void show_xgmac_stats(int xgmac_index, char * option)
{
	int value = XGMAC_STATS_OPTION_ALL; 
    uint32_t base_addr = XGMAC_BASE_ADDR(xgmac_index);
	if (strcasecmp(option, "all") == 0) {
		value = XGMAC_STATS_OPTION_ALL;
	} else if (strcasecmp(option, "tx") == 0) {
		value = XGMAC_STATS_OPTION_TX;
	} else if (strcasecmp(option, "rx") == 0) {
		value = XGMAC_STATS_OPTION_RX;
	} else if (strcasecmp(option, "clear") == 0) {
		value = -1;
	}

	printf("GET xgmac%d stats option[%d]\n", xgmac_index, value);

	if (value == -1) {
		writel(1, base_addr + 0x800);
	}
	else
		dubhe1000_xgmac_stats_dump_per_index(base_addr, xgmac_index, value);
}

void show_switch_stats(char * option) 
{
	int value = SWITCH_STATS_OPTION_ALL; 

	if (strcasecmp(option, "all") == 0) {
		value = SWITCH_STATS_OPTION_ALL;
	} else if (strcasecmp(option, "ingress") == 0) {
		value = SWITCH_STATS_OPTION_INGRESS;
	} else if (strcasecmp(option, "ipp") == 0) {
		value =SWITCH_STATS_OPTION_IPP ;
	}else if (strcasecmp(option, "bm") == 0) {
		value = SWITCH_STATS_OPTION_SHARED_BM;
	}else if (strcasecmp(option, "epp") == 0) {
		value = SWITCH_STATS_OPTION_EPP;
	}else if (strcasecmp(option, "egress") == 0) {
		value = SWITCH_STATS_OPTION_EGRESS;
	}

	dubhe1000_switch_stats_dump(value);

}

void switch_read(uint32_t addr)
{
    uint32_t val = readl(SWITCH_BASE_ADDR + (addr *4));
	printf("%10d: %#x\n", addr, val);
}

void switch_write(uint32_t addr, uint32_t val)
{
	writel(val, SWITCH_BASE_ADDR + (addr*4));
}

void xgmac_read(uint32_t id , uint32_t addr)
{
    uint32_t val = readl(XGMAC_BASE_ADDR(id) + addr);
	printf("%08x:%08x: %#x\n",XGMAC_BASE_ADDR(id) + addr, addr, val);
}

void xgmac_write(uint32_t id, uint32_t addr, uint32_t val)
{
	writel(val, XGMAC_BASE_ADDR(id) + addr );
}

void xpcs_read(uint32_t id , uint32_t addr)
{
    uint32_t val = readl(XPCS_BASE_ADDR(id) + (addr * 4));
	printf("%08x:%08x: %#x\n",XPCS_BASE_ADDR(id) + (4 * addr), addr, val);
}

void xpcs_write(uint32_t id, uint32_t addr, uint32_t val)
{
	writel(val, XPCS_BASE_ADDR(id) + (addr * 4)) ;
}

void switch_init(uint8_t* mac)
{
	dubhe1000_eth_switch_init(SWITCH_BASE_ADDR, mac);
}
void npe_top_dump()
{
	uint32_t addr = 0x53e00000;
	for (;addr <= 0x53e002a4; addr +=4) {
		uint32_t val = readl( addr);
		printf("%08x: %#x\n", addr, val);
	}
}

void xgmac_reset_all(int enable) 
{
	REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, enable);
	REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, enable);
	REG_W(XGMAC2_SFT_RST_N, xgmac1_sft_rst_n, enable);
	REG_W(XGMAC3_SFT_RST_N, xgmac1_sft_rst_n, enable);
	REG_W(XGMAC4_SFT_RST_N, xgmac1_sft_rst_n, enable);
}

void xgmac_set(int xgmac_index, char * speed )
{
	uint32_t base_addr = XGMAC_BASE_ADDR(xgmac_index);

		struct mac_tx_configuration_reg_st s_tx_conf = {
		.TE = 1,
	};

#if 0
	if (argc - pos < 1) { 
		printf("please enter (10G-XGMII/2_5G-GMII/1G-GMII/100M-GMII/5G-XGMII/2_5G-XGMII/10M-MII)");
		return CMD_RET_FAILURE;
	}
#endif

	if (strcasecmp(speed, "10G-XGMII") == 0) { 
		s_tx_conf.SS = SS_10G_XGMII;
	} else if  (strcasecmp(speed, "2_5G-GMII") == 0) {
		s_tx_conf.SS = SS_2Dot5G_GMII;
	} else if  (strcasecmp(speed, "1G-GMII") == 0) {
		s_tx_conf.SS = SS_1G_GMII;
	} else if  (strcasecmp(speed, "100M-GMII") == 0) {
		s_tx_conf.SS = SS_100M_GMII;
	} else if  (strcasecmp(speed, "5G-XGMII") == 0) {
		s_tx_conf.SS = SS_5G_XGMII;
	} else if  (strcasecmp(speed, "2_5G-XGMII") == 0) {
		s_tx_conf.SS = SS_2Dot5G_XGMII;
	} else if  (strcasecmp(speed, "10M-MII") == 0) {
		s_tx_conf.SS = SS_10M_MII;
	}else {
		printf("unknown speed setting[%s], please enter (10G-XGMII/2_5G-GMII/1G-GMII/100M-GMII/5G-XGMII/2_5G-XGMII/10M-MII)\n", speed);
		return ;
	}

	printf("init xgmac[%d]=) addr[%#x],value[0x149]\n",
			xgmac_index,
			base_addr  + 0x8);
	writel(0x149 ,base_addr  + 0x8);

	printf("init xgmac[%d]=)addr[%#x], value[%#x]\n",
			xgmac_index,
			base_addr  + 0x0,
			s_tx_conf.raw);
	writel(s_tx_conf.raw, base_addr  + 0x0);

	printf("init xgmac[%d]=)addr[%#x], value[0x207]\n",
			xgmac_index,
			base_addr  + 0x4);
	writel(0x207, base_addr  + 0x4);

	printf("init xgmac[%d]=)addr[%#x], value[0xffff0012]\n",
			xgmac_index,
			base_addr  + 0x70);
	writel(0xffff0012, base_addr  + 0x70);

	printf("init xgmac[%d]=)addr[%#x], value[0x1]\n",
			xgmac_index,
			base_addr  + 0x90);
	writel(0x1, base_addr  + 0x90);

	printf("init xgmac[%d]=)addr[%#x], value[0xffffffff]\n",
			xgmac_index,
			base_addr  + 0xa8);
	writel(0xffffffff, base_addr  + 0xa8);

	printf("init xgmac[%d]=)addr[%#x], value[0xffffffff]\n",
			xgmac_index,
			base_addr  + 0xac);
	writel(0xffffffff, base_addr  + 0xac);
}

int macStringToBinary(const char* mac_str, unsigned char* mac_binary) {

    unsigned char temp_mac[6];
    if (mac_str == NULL || mac_binary == NULL) {
        return -1; 
    }


    if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &temp_mac[0], &temp_mac[1], &temp_mac[2],
               &temp_mac[3], &temp_mac[4], &temp_mac[5]) != 6) {
        return -1;
    }

    memcpy(mac_binary, temp_mac, 6);

    return 0;
}

void xpcs_mii_dig_sts(int i) 
{
	uint32_t  value;
	value = readl(XPCS_BASE_ADDR(i) + VR_MII_DIG_STS);
	printf("VR_MII_DIG_STS:%#x:%#x\n", VR_MII_DIG_STS, value);
	value = readl(XPCS_BASE_ADDR(i) + VR_MII_1_DIG_STS);
	printf("VR_MII_1_DIG_STS:%#x:%#x\n", VR_MII_1_DIG_STS, value);
	value = readl(XPCS_BASE_ADDR(i) + VR_MII_2_DIG_STS);
	printf("VR_MII_2_DIG_STS:%#x:%#x\n", VR_MII_2_DIG_STS, value);
	value = readl(XPCS_BASE_ADDR(i) + VR_MII_3_DIG_STS);
	printf("VR_MII_3_DIG_STS:%#x:%#x\n", VR_MII_3_DIG_STS, value);
}

void xpcs_mii_ctrl(int i) 
{
	uint32_t  value;
	value = readl(XPCS_BASE_ADDR(i) + SR_MII_CTRL);
	printf("SR_MII_CTRL:%#x:%#x\n", SR_MII_CTRL, value);
	value = readl(XPCS_BASE_ADDR(i) + SR_MII_1_CTRL);
	printf("SR_MII_1_CTRL:%#x:%#x\n", SR_MII_1_CTRL, value);
	value = readl(XPCS_BASE_ADDR(i) + SR_MII_2_CTRL);
	printf("SR_MII_2_CTRL:%#x:%#x\n", SR_MII_2_CTRL, value);
	value = readl(XPCS_BASE_ADDR(i) + SR_MII_3_CTRL);
	printf("SR_MII_3_CTRL:%#x:%#x\n", SR_MII_3_CTRL, value);
}

void xpcs_an_sts(int i) 
{
	uint32_t  value;
	value = readl(XPCS_BASE_ADDR(i) + VR_MII_AN_INTR_STS);
	printf("VR_MII_AN_INTR_STS :%#x:%#x\n", VR_MII_AN_INTR_STS, value);
	value = readl(XPCS_BASE_ADDR(i) + VR_MII_1_AN_INTR_STS);
	printf("VR_MII_1_AN_INTR_STS :%#x:%#x\n", VR_MII_1_AN_INTR_STS, value);

	value = readl(XPCS_BASE_ADDR(i) + VR_MII_2_AN_INTR_STS);
	printf("VR_MII_2_AN_INTR_STS :%#x:%#x\n", VR_MII_2_AN_INTR_STS, value);

	value = readl(XPCS_BASE_ADDR(i) + VR_MII_3_AN_INTR_STS);
	printf("VR_MII_3_AN_INTR_STS :%#x:%#x\n", VR_MII_3_AN_INTR_STS, value);
}

#include <signal.h>

int main(int argc, char *argv[], char * envp[]) 
{
	int offset = 1, ret = 0;
	char *  cmd = NULL;
	int title = 0;
	int i;


	for (i=0; envp[i] != NULL; i ++) {
		if (0 == strcasecmp("debug=1", envp[i]))
			g_debug = 1;
		else if (0 == strcasecmp("title=1", envp[i])) {
			title = 1;
		} else if (0 == strcasecmp("serdes_fw=1", envp[i])) {
			g_serdes_fw = 1;
		}
	}

	cmd = strrchr(argv[0], '/'); 
	cmd = cmd == NULL ? argv[0] : &cmd[1];


	if (0 == strcasecmp("Serdes_init_pre", cmd)) {
		//printf("Is Serdes_init_pre Func\n");
		int id;

		if (argc != 1) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		//CONVERT_TO_UINT(id);

		Serdes_init_pre();
	} else if (0 == strcasecmp("Serdes_init_proc", cmd)) {
		int xpcs_num;
		int serdes_mode;
		int usxg_mode;
	 	int rxadpt_en;

		//printf("Is Serdes_init_proc Func\n");
		if (argc != 5) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(serdes_mode);
		CONVERT_TO_UINT(usxg_mode);
		CONVERT_TO_UINT(rxadpt_en);

		Serdes_init_proc(xpcs_num, serdes_mode, usxg_mode, rxadpt_en);

	} else if (0 == strcasecmp("ETH_serdes_reg_write", cmd)) {
		int xpcs_num;
		int reg_addr;
		int reg_val;

		//printf("Is ETH_serdes_reg_write Func\n");

		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(reg_addr);
		CONVERT_TO_UINT(reg_val);

		ETH_serdes_reg_write(xpcs_num, reg_addr, reg_val);
	} else if (0 == strcasecmp("ETH_serdes_reg_read", cmd)) {
		int xpcs_num;
		int reg_addr;
		uint32_t val;

		//printf("Is ETH_serdes_reg_read Func\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(reg_addr);
		//printf("Serdes%d REG %x = %x. \n ",xpcs_num, reg_addr, ETH_serdes_reg_read(xpcs_num, reg_addr));				//讲读取的寄存器名及值打印出来
        val = ETH_serdes_reg_read(xpcs_num, reg_addr);
		printf("%08X:%#X\n", reg_addr, val);
	} else if (0 == strcasecmp("TX_FFE_Config", cmd)) {
		int xpcs_num;
		int tx_eq_main;
		int tx_eq_pre;
		int tx_eq_post;

		//printf("Is TX_FFE_Config Func\n");

		if (argc != 5) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(tx_eq_main);
		CONVERT_TO_UINT(tx_eq_pre);
		CONVERT_TO_UINT(tx_eq_post);

		TX_FFE_Config(xpcs_num,tx_eq_main, tx_eq_pre, tx_eq_post);

	} else if (0 == strcasecmp("PCIE_TX_FFE_Config", cmd)) {
		int tx_eq_main;
		int tx_eq_pre;
		int tx_eq_post;

		//printf("Is PCIE_TX_FFE_Config Func\n");

		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(tx_eq_main);
		CONVERT_TO_UINT(tx_eq_pre);
		CONVERT_TO_UINT(tx_eq_post);

		PCIE_TX_FFE_Config(tx_eq_main, tx_eq_pre, tx_eq_post);
	} 
	else if (0 == strcasecmp("RX_Adapt_enable", cmd)) {
		int xpcs_num;
		int reg_dm;

		//printf("Is RX_Adapt_enable Func\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(reg_dm);

		RX_Adapt_enable( xpcs_num, reg_dm);
	} else if (0 == strcasecmp("TX_PRBS_Config", cmd)) {
		int xpcs_num;
		int mode;
		int pat0;

		//printf("Is TX_PRBS_Config Func\n");
		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(mode);
		CONVERT_TO_UINT(pat0);

		TX_PRBS_Config(xpcs_num, mode, pat0);
	} else if (0 == strcasecmp("TX_err_Insert", cmd)) {
		int xpcs_num;

		//printf("Is TX_err_Insert Func\n");

		CONVERT_TO_UINT(xpcs_num);

		TX_err_Insert(xpcs_num);
	} else if (0 == strcasecmp("RX_PRBS_Config", cmd)) {
		int xpcs_num;
		int mode;

		//printf("Is RX_PRBS_Config Func\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(mode);

		RX_PRBS_Config(xpcs_num, mode);
	} else if (0 == strcasecmp("RX_err_count", cmd)) {
		int xpcs_num;
		int ret = 0;

		//printf("Is RX_err_count Func\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		ret = RX_err_count(xpcs_num);

		printf("RX_err_count ret[%d]\n", ret);
	} else if (0 == strcasecmp("Loopback_config", cmd)) {
		//printf("Is Loopback_config Func\n");
		int xpcs_num;
		int mode;
		int reg_dm;
		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(mode);
		CONVERT_TO_UINT(reg_dm);

		Loopback_config(xpcs_num, mode, reg_dm);

	} else if (0 == strcasecmp("Repeat_CLK_Config", cmd)) {
		int xpcs_num;
		int mode;

		//printf("Is Repeat_CLK_Config Func\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(mode);

		Repeat_CLK_Config(xpcs_num, mode);
	} else if (0 == strcasecmp("VR_RST", cmd)) {
		int xpcs_num;

		//printf("Is VR_RST Func\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);

		VR_RST(xpcs_num);
	} else if (0 == strcasecmp("tx_req", cmd)) {
		int xpcs_num;
		//printf("Is tx_req Func\n");
		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}
		CONVERT_TO_UINT(xpcs_num);
		tx_req(xpcs_num);
	} else if (0 == strcasecmp("rx_req", cmd)) {
		int xpcs_num;
		//printf("Is rx_req Func\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}
		CONVERT_TO_UINT(xpcs_num);
		rx_req(xpcs_num);
	} else if (0 == strcasecmp("Serdes_dump", cmd)) {
		int option;

		//printf("Is Serdes_dump Func\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}
		CONVERT_TO_UINT(option);
		Serdes_dump(option);
	}  else if (0 == strcasecmp("TX_Swing_Config", cmd)) {
		int xpcs_num, tx_vboost_en, tx_vboost_lvl, tx_iboost_lvl;
		//printf("Is TX_Swing_Config\n");

		if (argc != 5) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(tx_vboost_en);
		CONVERT_TO_UINT(tx_vboost_lvl);
		CONVERT_TO_UINT(tx_iboost_lvl);

		TX_Swing_Config(xpcs_num, tx_vboost_en, tx_vboost_lvl, tx_iboost_lvl);
	} else if (0 == strcasecmp("show_xgmac_stats", cmd)) {
		int xgmac_index;  char * option = "all";
		//printf("Is show_xgmac_stats\n");

		if (argc == 1) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xgmac_index);

		if (argc > 2) {
			option = argv[2];
		}

		show_xgmac_stats( xgmac_index,option);
	}
	else if (0 == strcasecmp("show_switch_stats", cmd)) {
		char * option = "all";
		//printf("Is show_switch_stats\n");

		if (argc > 1) {
			option = argv[1];
		}

		show_switch_stats(option); 
	}
	else if (0 == strcasecmp("switch_init", cmd)) {
		u8 mac[6] = {0,1,0};
		//printf("Is switch_init\n");

		if (argc > 1) {
			macStringToBinary(argv[1], mac);
		}

		switch_init(mac);
	}
	else if (0 == strcasecmp("xgmac_set", cmd)) {
		int xgmac_index;
		//printf("Is xgmac_set\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xgmac_index);


		xgmac_set(xgmac_index, argv[2]);
	}
	else if (0 == strcasecmp("soc_read", cmd)) {
		uint32_t addr, value;
		//printf("Is soc_read\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(addr);
		value = readl(addr);
		printf("%08X:%#X\n", addr, value);

	}
	else if (0 == strcasecmp("soc_write", cmd)) {
		uint32_t addr, value;
		//printf("Is soc_write\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(addr);
		CONVERT_TO_UINT(value);
		writel(value, addr);
	}
	else if (0 == strcasecmp("ETH_PLL_Config", cmd)) {
		int refdiv;
		int fbdiv;
		int frac;
		int postdiv1;
		int postdiv2;
		//printf("Is ETH_PLL_Config\n");

		if (argc != 6) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(refdiv);
		CONVERT_TO_UINT(fbdiv);
		CONVERT_TO_UINT(frac);
		CONVERT_TO_UINT(postdiv1);
		CONVERT_TO_UINT(postdiv2);

		ETH_PLL_Config(refdiv, fbdiv,frac, postdiv1, postdiv2);
	} else if (0 == strcasecmp("PCIE_PLL_Config", cmd)) {
		int refdiv;
		int fbdiv;
		int frac;
		int postdiv1;
		int postdiv2;
		//printf("Is PCIE_PLL_Config\n");

		if (argc != 6) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(refdiv);
		CONVERT_TO_UINT(fbdiv);
		CONVERT_TO_UINT(frac);
		CONVERT_TO_UINT(postdiv1);
		CONVERT_TO_UINT(postdiv2);

		PCIE_PLL_Config(refdiv, fbdiv,frac, postdiv1, postdiv2);
	}
	else if (0 == strcasecmp("xgmac_reset_all", cmd)) {
		int enable;
		//printf("Is xgmac_reset_all\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(enable);
		xgmac_reset_all(enable);
	}
	else if (0 == strcasecmp("switch_read", cmd)) {
		uint32_t addr ;
		////printf("Is switch_read\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(addr);
		switch_read(addr);
	}
	else if (0 == strcasecmp("switch_write", cmd)) {
		uint32_t addr, value;
		//printf("Is switch_read\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(addr);
		CONVERT_TO_UINT(value);
		switch_write(addr,value);
	}
	else if (0 == strcasecmp("xpcs_read", cmd)) {
		uint32_t id;
		uint32_t addr;
		////printf("Is switch_read\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(id);
		CONVERT_TO_UINT(addr);
		xpcs_read(id, addr);
	}
	else if (0 == strcasecmp("xpcs_write", cmd)) {
		uint32_t id,addr, value;
		//printf("Is xpcs_write\n");

		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(id);
		CONVERT_TO_UINT(addr);
		CONVERT_TO_UINT(value);
		xpcs_write(id, addr,value);
	}
	else if (0 == strcasecmp("xgmac_read", cmd)) {
		uint32_t id;
		uint32_t addr;
		////printf("Is switch_read\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(id);
		CONVERT_TO_UINT(addr);
		xgmac_read(id, addr);
	}
	else if (0 == strcasecmp("xgmac_write", cmd)) {
		uint32_t id,addr, value;
		//printf("Is xgmac_write\n");

		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(id);
		CONVERT_TO_UINT(addr);
		CONVERT_TO_UINT(value);
		xgmac_write(id, addr,value);
	}
	else if (0 == strcasecmp("xpcs_mii_dig_sts", cmd)) {
		printf("xpcs_mii_dig_sts\n");
		uint32_t i;
		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(i);
		xpcs_mii_dig_sts(i);
	}	else if (0 == strcasecmp("xpcs_mii_ctrl", cmd)) {
		printf("xpcs_mii_ctrl\n");
		uint32_t i;
		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(i);
		xpcs_mii_ctrl(i);
	}
	else if (0 == strcasecmp("npe_top_dump", cmd)) {
		printf("npe_top_dump\n");
		if (argc != 1) {
			printf("Err ARG num!\n");
			goto ERR;
		}
		npe_top_dump();
	}	else if (0 == strcasecmp("xpcs_an_sts", cmd)) {
		printf("xpcs_an_sts\n");
		uint32_t i;
		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}
		CONVERT_TO_UINT(i);
		xpcs_an_sts(i);
	} else if (0 == strncasecmp("mdio_cmd", cmd, 8)) {
		printf("mdio cmd\n");
		if (argc <= 1) {
			printf("Err ARG num!\n");
			printf("mdio_cmd <id> read/write <flags> <addr> <reg> <value> \n");
			goto ERR;
		}
		do_phy(--argc, &argv[1]);	
	} else if (0 == strcasecmp("PCIE_serdes_reg_read", cmd)) {

		int reg_addr;
		uint32_t val;
		//printf("Is PCIE_serdes_reg_read\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(reg_addr);

		val = PCIE_serdes_reg_read(reg_addr);

		printf("%08X:%#X\n", reg_addr, val);

	} else if (0 == strcasecmp("PCIE_serdes_reg_write", cmd)) {

		int reg_addr;
		uint32_t val;
		//printf("Is PCIE_serdes_reg_write\n");

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(reg_addr);
		CONVERT_TO_UINT(val);

		PCIE_serdes_reg_write(reg_addr, val);
	} else if (0 == strcasecmp("PCIE_TX_PRBS_Config", cmd)) {
		int mode;
		int pat0;

		//printf("Is PCIE_TX_PRBS_Config Func\n");
		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(mode);
		CONVERT_TO_UINT(pat0);

		PCIE_TX_PRBS_Config(mode, pat0);
	} else if (0 == strcasecmp("PCIE_TX_err_Insert", cmd)) {

		//printf("Is PCIE_TX_err_Insert Func\n");

		PCIE_TX_err_Insert();
	} else if (0 == strcasecmp("PCIE_RX_PRBS_Config", cmd)) {
		int xpcs_num;
		int mode;

		//printf("Is PCIE_RX_PRBS_Config Func\n");

		if (argc != 2) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(mode);

		PCIE_RX_PRBS_Config(mode);
	} else if (0 == strcasecmp("PCIE_RX_err_count", cmd)) {
		int ret = 0;

		//printf("Is PCIE_RX_err_count Func\n");

		if (argc != 1) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		ret = PCIE_RX_err_count();

		printf("PCIE_RX_err_count ret[%d]\n", ret);
	} else if (0 == strcasecmp("PCIE_init", cmd)) {
		int gen = 0;
		int tx_main;
		int tx_pre;
		int tx_post;
		int rx_adapt_en;
		//printf("Is PCIE_init Func\n");

		if (argc != 6) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(gen);
		CONVERT_TO_UINT(tx_main);
		CONVERT_TO_UINT(tx_pre);
		CONVERT_TO_UINT(tx_post);
		CONVERT_TO_UINT(rx_adapt_en);

		PCIE_init(gen, tx_main, tx_pre, tx_post, rx_adapt_en);	
	} else if (0 == strcasecmp("log_printf", cmd)) {
		int gen = 0;

		//printf("Is PCIE_init Func\n");

		if (argc != 1) {
			printf("Err ARG num!\n");
			goto ERR;
		}

        log_printf(1); 
	} else if (0 == strcasecmp("log_config", cmd)) {
        log_config(argc - 1, argv + 1); 
	}
	 else if (0 == strcasecmp("PCIE_RX_ADAPT", cmd)) {
		 PCIE_RX_ADAPT();
	}
 else if (0 == strcasecmp("Serdes_init_proc_test", cmd)) {
	 int xpcs_num;
	 int serdes_mode;
	 int usxg_mode;
	 int rxadpt_en;
	 int tx_eq_main_in;
	 int tx_eq_req_in;
	 int tx_eq_post_in;
	 int vga1_gain_0_val;
	 int vga2_gain_0_val;
	 int ctle_pole_0_val;
	 int ctle_boost_0_val;

			 //printf("Is Serdes_init_proc Func\n");
	 if (argc != 12) {
		 printf("Err ARG num!\n");
		 goto ERR;
	 }

	 CONVERT_TO_UINT(xpcs_num);
	 CONVERT_TO_UINT(serdes_mode);
	 CONVERT_TO_UINT(usxg_mode);
	 CONVERT_TO_UINT(rxadpt_en);
	 CONVERT_TO_UINT(tx_eq_main_in);
	 CONVERT_TO_UINT(tx_eq_req_in);
	 CONVERT_TO_UINT(tx_eq_post_in);
	 CONVERT_TO_UINT(vga1_gain_0_val);
	 CONVERT_TO_UINT(vga2_gain_0_val);
	 CONVERT_TO_UINT(ctle_pole_0_val);
	 CONVERT_TO_UINT(ctle_boost_0_val);


	 Serdes_init_proc_test(xpcs_num,
			 serdes_mode,
			 usxg_mode,
			 rxadpt_en,
			tx_eq_main_in,
	 		tx_eq_req_in,
	 		tx_eq_post_in,
   			vga1_gain_0_val,
			vga2_gain_0_val,
			ctle_pole_0_val,
	   		ctle_boost_0_val);
	}
	else if (0 == strcasecmp("ETH_RX_ADAPT", cmd)) {
	 int xpcs_num;
	 if (argc != 2) {
		 printf("Err ARG num!\n");
		 goto ERR;
	 }

	 CONVERT_TO_UINT(xpcs_num);

	 ETH_RX_ADAPT(xpcs_num);
	}
	else if (0 == strcasecmp("RX_AFE_OVERWRIDE", cmd)) {
		int xpcs_num;
		int eq_att;
		int eq_gain1;
		int eq_gain2;
		int eq_pole;
		int eq_boost;
		int eq_tap1;
		if (argc != 8) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(eq_att);
		CONVERT_TO_UINT(eq_gain1);
		CONVERT_TO_UINT(eq_gain2);
		CONVERT_TO_UINT(eq_pole);
		CONVERT_TO_UINT(eq_boost);
		CONVERT_TO_UINT(eq_tap1);

		rx_afe_overwride(xpcs_num, eq_att, eq_gain1, eq_gain2, eq_pole, eq_boost, eq_tap1);
	}
	else if (0 == strcasecmp("XPCS_speed", cmd)) {
		int xpcs_num;
		int phy_interface;
		int port;
		unsigned int speed;
		char * str;

		if (argc != 5) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_STRING(str);
		CONVERT_TO_UINT(port);
		CONVERT_TO_UINT(speed);

		phy_interface = phy_get_interface_by_name(str);
		if (phy_interface == -1) 
			printf("Err phy_interface type\n");

		XPCS_speed(xpcs_num, phy_interface,  port, speed); 
	}
	else if (0 == strcasecmp("XPCS_get_link_state", cmd)) {
		int xpcs_num;
		int phy_interface;
		int port;
		char * str;

		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_STRING(str);
		CONVERT_TO_UINT(port);

		phy_interface = phy_get_interface_by_name(str);
		if (phy_interface == -1) 
			printf("Err phy_interface type\n");

		XPCS_get_link_state( xpcs_num,  phy_interface,  port);
	}
	else if (0 == strcasecmp("XPCS_clear_an_interrupt", cmd)) {
		int xpcs_num;
		int port;
		char * str;

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(port);

		XPCS_clear_an_interrupt(xpcs_num,  port);
	} else if (0 == strcasecmp("XPCS_an_enable", cmd)) {
		int xpcs_num;
		int port;
		int enable;
		char * str;

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(port);
		CONVERT_TO_UINT(enable);

		XPCS_an_enable(xpcs_num, port, enable);
	} else if (0 == strcasecmp("XPCS_mac_auto_sw", cmd)) {
		int xpcs_num;
		int port;
		int enable;
		char * str;

		if (argc != 3) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(xpcs_num);
		CONVERT_TO_UINT(port);
		CONVERT_TO_UINT(enable);

		XPCS_mac_auto_sw(xpcs_num, port, enable);
	}
	else if (0 == strcasecmp("dump_serdes", cmd)) {
		int index;
		int begin;
		int end;
		int size;
		int addr;
		uint32_t val;

		if (argc != 4) {
			printf("Err ARG num!\n");
			goto ERR;
		}

		CONVERT_TO_UINT(index);
		CONVERT_TO_UINT(begin);
		CONVERT_TO_UINT(size);

        end = begin + size;

		if (index == 0 || index == 1) {
			for (addr = begin; addr < end; addr++) {
				val = ETH_serdes_reg_read(index, addr);
				printf("%08X:%#X\n", addr, val);
			}
		} else if (index == 2) {
			for (addr = begin; addr < end; addr++) {
				val = PCIE_serdes_reg_read(addr);
				printf("%08X:%#X\n", addr, val);
			}
		}
	}
ERR: 

	return 0;

}
