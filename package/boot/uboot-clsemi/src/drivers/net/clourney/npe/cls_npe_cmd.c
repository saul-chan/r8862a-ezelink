#include <common.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include "cls_npe.h"
#if 1
#include "cls_npe_edma_stats.h"
#endif
#include "cls_npe_mac_stats.h"
#include "cls_npe_switch_stats.h"
#include "cls_npe_fwd.h"

#define  CMD_MDIO     0x6f69646d //mdio
#define  CMD_PHY      0x796870 //phy
#define  CMD_INIT     0x74696e69 //init
#define  CMD_ENABLE   0x6E65  //en
#define  CMD_SKIP     0x70696b73 //skip
#define  CMD_SKIP_SW  0x7773 //sw
#define  CMD_SKIP_EXTSW  0x777365 //esw
#define  CMD_READ     0x72       //'r'
#define  CMD_WRITE    0x77       //'w'
#define  CMD_STAS     0x73617473 //stas
int  g_skip_check_phy = 0;
int g_skip_qsgmii = 0;
extern int g_skip_switch;
extern int g_skip_extswitch;
int rtk_disable = 0;
extern struct mii_dev *mii_bus_list[CLS_NPE_MAX_XGMAC_NUM];
#define SET_BITMAP(map, index)     ((map) |= (1 << (index)))
#define IS_BITMAP_SET(map, index)  ((map) & (1 << (index)))
enum {
	USXG_MODE_10G_SXGMII 	=   (0),
	USXG_MODE_5G_SXGMII 	=   (1),
	USXG_MODE_2_5G_SXGMII 	=   (2),
	USXG_MODE_10G_DXGMII  	=   (3),
	USXG_MODE_5G_DXGMII  	=   (4),
	USXG_MODE_10G_QXGMII 	=   (5),
	USXG_MODE_MAX_NUM     	=   (6)
};

#define REG_W(addr, field, value) 		({ uint32_t __val =  readl(addr); \
		__val &= ~GENMASK(field##_END, field##_BEGIN); \
		__val |= (value << (field##_BEGIN)) & GENMASK(field##_END, field##_BEGIN); \
		writel(__val, addr); \
		printf("WR [%s][%#x] [%s][%d_%d] __val[%#x] val[%#x]\n", \
#addr, addr, #field,field##_BEGIN,field##_END,__val, value); \
		})

static int  xgmac_int_bitmap;
static void cls_dump_regs(void __iomem *address)
{
	u32 data[8] = {0};
	data[0] = readl(address);
	data[1] = readl(address + 4*1);
	data[2] = readl(address + 4*2);
	data[3] = readl(address + 4*3);
	data[4] = readl(address + 4*4);
	data[5] = readl(address + 4*5);
	data[6] = readl(address + 4*6);
	data[7] = readl(address + 4*7);
	printf("address 0x%08px: %08x %08x %08x %08x %08x %08x %08x %08x\n",
			address,
			data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);
}

static void cls_dump_regs_w(void __iomem *address)
{
	u16 data[8] = {0};
	data[0] = readl(address);
	data[1] = readl(address + 4*1);
	data[2] = readl(address + 4*2);
	data[3] = readl(address + 4*3);
	data[4] = readl(address + 4*4);
	data[5] = readl(address + 4*5);
	data[6] = readl(address + 4*6);
	data[7] = readl(address + 4*7);
	printf("address 0x%08px: %04x %04x %04x %04x %04x %04x %04x %04x\n",
			address,
			data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);
}
#ifdef CONFIG_CLS_PHYLINK
static int do_xgmac(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cls_eth_priv *priv_data = NULL;
	struct udevice *dev = eth_get_dev();
	int len, interface, pos = 0, xgmac_index;
	unsigned int sub_cmd = 0;

	if(!dev) {
		printf("Can't find udevice_eth device!\n");
		return CMD_RET_FAILURE;
	}

	//printf("eth_device name[%s] priv[%p]\n",dev->name, dev_get_priv(dev));

	if (argc < 2)
		return CMD_RET_USAGE;

	priv_data = dev_get_priv(dev);

	xgmac_index = *(argv[pos] + strlen("xgmac")) - '0';
	if (xgmac_index > 4) {
		printf("xgmac must be (0-4), but this is (%d)\n", xgmac_index);
		return CMD_RET_FAILURE;
	}

	if (!IS_BITMAP_SET(xgmac_int_bitmap, xgmac_index) && !priv_data->initd) {
         printf("Please use interface command first!\n");
		 return CMD_RET_FAILURE;
	}

	printf("xgmac index %d\n", xgmac_index);

	pos++;
	len = strlen(argv[pos]);
	if (len > 4)
		return CMD_RET_USAGE;

	memcpy(&sub_cmd, argv[pos], len);

	pos++;
	switch (sub_cmd) {
	case CMD_ENABLE:
		{
			if (argc - pos < 1) {
				printf("please enter <enable>");
				return CMD_RET_FAILURE;
			}
			int enable = simple_strtoul(argv[pos], NULL, 10);
			if (enable)
				priv_data->xgmac[xgmac_index].flags |= BIT(HW_ENABLE_BIT);
			else
				priv_data->xgmac[xgmac_index].flags &= ~BIT(HW_ENABLE_BIT);
			printf("xgmac %d has %s\n", xgmac_index, enable ? "enable" : "disable");
		}
		break;
	case CMD_MDIO:
		{
			char * name = NULL;
			int clk_csr = 0;

			printf("is mdio command!\n");

			if (argc - pos < 1) {
				printf("please enter <name>");
				return CMD_RET_FAILURE;
			}

			name = argv[pos];

			if (argc - pos > 1) {
				clk_csr = *argv[pos + 1] - '0';
			}

			printf("clk_csr = %d\n", clk_csr);

			cls_eth_print("[%s] init xgmac%d phy\n", __func__, xgmac_index);

			priv_data->xgmac[xgmac_index].clk_csr = clk_csr;/*100-150M*/
#ifdef CONFIG_CLS_PHYLINK
			cls_mdio_init(name, 0, &priv_data->xgmac[xgmac_index]);
#endif
			//SET_BITMAP(xgmac_int_bitmap, 5 * xgmac_index);

			//pos += 2;
		}
		break;
	case CMD_PHY:
		{
			struct phy_device *phydev = NULL;
			int reset = -1, mdio_flags = -1, phy_addr = -1, mask = -1;
			struct cls_xgmac_priv *priv = &priv_data->xgmac[xgmac_index];

			//1 phyaddr 2.pre 3.reset
			if (argc >= pos)
				phy_addr = simple_strtoul(argv[pos], NULL, 16);

			if (argc - pos > 0)
				mdio_flags = *argv[pos + 1] - '0';

			if (argc - pos > 1)
				reset = *argv[pos + 2] - '0';

		     printf("phy_addr = %d\n mdio_flags = %d reset = %d", phy_addr, mdio_flags, reset);

			 if (reset != -1) {
				 printf("==) phy reset [%s]\n", !reset ? "high" : "low");
				// writel(!reset, 0x90414820);
				 //mdelay(600);
				 //writel(reset, 0x90414820);
				 //mdelay(600);
				 //printf("==) phy reset end [%s]\n",reset ? "high" : "low");
			 }

			 if (mdio_flags == 1) {
				 priv->first_pre |= BIT(phy_addr);
				 printf("Need disable PSE on first Read\n");
			 }

			 if (phy_addr >= 0 && phy_addr <= 31) {
				 mask = BIT(phy_addr);
			 }

			printf("phy_find_by_mask XGMAC%d bus addr %#x\n", priv->id, mii_bus_list[priv->id]);
			phydev = phy_find_by_mask(mii_bus_list[priv->id], mask, priv->interface);
			if (!phydev) {
				printf("Can't find the phy\n");
				return -ENODEV;
			}

			printf("phy_connect_dev ==)\n");
			phy_connect_dev(phydev, dev);
			phydev->supported &= PHY_GBIT_FEATURES;
			phy_set_supported(phydev, 1000);
			phydev->advertising = phydev->supported;
			phy_config(phydev);
			printf("cls_phy_init  END\n");
		}
		break;
	case CMD_INIT:
		{
			char * speed = argv[pos];
			int speed_sel = 3;

			printf("is init command!\n");

			struct mac_tx_configuration_reg_st s_tx_conf = {
				.TE = 1,
			};

			if (argc - pos < 1) {
				printf("please enter (10G-XGMII/2_5G-GMII/1G-GMII/100M-GMII/5G-XGMII/2_5G-XGMII/10M-MII)");
				return CMD_RET_FAILURE;
			}

			if (strcasecmp(speed, "10G-XGMII") == 0) {
				s_tx_conf.SS = SS_10G_XGMII;
			} else if  (strcasecmp(speed, "2_5G-GMII") == 0) {
				s_tx_conf.SS = SS_2Dot5G_GMII;
			} else if  (strcasecmp(speed, "1G-GMII") == 0) {
				speed_sel = 3;
				s_tx_conf.SS = SS_1G_GMII;
			} else if  (strcasecmp(speed, "100M-GMII") == 0) {
				speed_sel = 4;
				s_tx_conf.SS = SS_100M_GMII;
			} else if  (strcasecmp(speed, "5G-XGMII") == 0) {
				s_tx_conf.SS = SS_5G_XGMII;
			} else if  (strcasecmp(speed, "2_5G-XGMII") == 0) {
				s_tx_conf.SS = SS_2Dot5G_XGMII;
			} else if  (strcasecmp(speed, "10M-MII") == 0) {
				speed_sel = 7;
				s_tx_conf.SS = SS_10M_MII;
			}else {
				printf("unknown speed setting[%s], please enter (10G-XGMII/2_5G-GMII/1G-GMII/100M-GMII/5G-XGMII/2_5G-XGMII/10M-MII)\n", speed);
				return CMD_RET_FAILURE;
			}

			printf("init xgmac[%d]=) addr[%#x],value[0x149]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0x8);
			writel(0x149 ,priv_data->xgmac[xgmac_index].ioaddr  + 0x8);

			printf("init xgmac[%d]=)addr[%#x], value[%#x]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0x0,
					s_tx_conf.raw);
			writel(s_tx_conf.raw, priv_data->xgmac[xgmac_index].ioaddr  + 0x0);

			printf("init xgmac[%d]=)addr[%#x], value[0x207]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0x4);
			writel(0x207, priv_data->xgmac[xgmac_index].ioaddr  + 0x4);

			printf("init xgmac[%d]=)addr[%#x], value[0xffff0012]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0x70);
			writel(0xffff0012, priv_data->xgmac[xgmac_index].ioaddr  + 0x70);

			printf("init xgmac[%d]=)addr[%#x], value[0x1]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0x90);
			writel(0x1, priv_data->xgmac[xgmac_index].ioaddr  + 0x90);

			printf("init xgmac[%d]=)addr[%#x], value[0xffffffff]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0xa8);
			writel(0xffffffff, priv_data->xgmac[xgmac_index].ioaddr  + 0xa8);

			printf("init xgmac[%d]=)addr[%#x], value[0xffffffff]\n",
					xgmac_index,
					priv_data->xgmac[xgmac_index].ioaddr  + 0xac);
			writel(0xffffffff, priv_data->xgmac[xgmac_index].ioaddr + 0xac);

			if (priv_data->xgmac[xgmac_index].interface == XGMAC_INTERFACE_MODE_RGMII) {
				printf("Set RMII%d speed_sel %d\n", speed_sel, xgmac_index);
				writel(speed_sel, XGMAC_RGMII_IO_SPEED_ADDR(xgmac_index));
			}

			pos++;
		}
		break;
	case CMD_READ:
		{
			unsigned int addr,len,k,k2;

			printf("is read command !\n");

			if (argc - pos != 2)
				return CMD_RET_USAGE;

			addr = simple_strtoul(argv[pos], NULL, 16);
			len  = simple_strtoul(argv[pos + 1], NULL, 10);

			printf("xgmac%d read: 0x%08x len: %d\n", xgmac_index, addr, len);

			k2 = len /8;

			cls_dump_regs(priv_data->xgmac[xgmac_index].ioaddr + addr);

			for(k = 0; k < k2; k++) {
				if(k == 0)
					continue;
				cls_dump_regs(priv_data->xgmac[xgmac_index].ioaddr + addr + 32*k);
			}

			cls_dump_regs(priv_data->xgmac[xgmac_index].ioaddr + addr + 32*k);

			pos += 2;
		}
		break;
	case CMD_WRITE:
		{
			unsigned int addr,value;

			if (argc - pos != 2)
				return CMD_RET_USAGE;

			addr = simple_strtoul(argv[pos], NULL, 16);

			value  = simple_strtoul(argv[pos + 1], NULL, 16);

			//printf("is write command [%#x] [%#x]!!\n", addr, value);
			printf("xgmac%d write: 0x%08x = 0x%08x\n",xgmac_index, addr, value);

			writel(value, priv_data->xgmac[xgmac_index].ioaddr + addr);
			value = readl(priv_data->xgmac[xgmac_index].ioaddr + addr);

			printf("xgmac%d read: 0x%08x = 0x%08x\n",xgmac_index, addr, value);

			pos += 2;
		}
		break;
	case CMD_STAS:
		{
			int value = XGMAC_STATS_OPTION_ALL;

			if (argc - pos == 1) {
				char * option = argv[pos];
				if (strcasecmp(option, "all") == 0) {
					value = XGMAC_STATS_OPTION_ALL;
				} else if (strcasecmp(option, "tx") == 0) {
					value = XGMAC_STATS_OPTION_TX;
				} else if (strcasecmp(option, "rx") == 0) {
					value = XGMAC_STATS_OPTION_RX;
				}
				else if (strcasecmp(option, "clear") == 0) {
					value = -1;
				}
			}

			printf("GET xgmac%d stats option[%d]\n", xgmac_index, value);

			if (value == -1) {
				writel(1, priv_data->xgmac[xgmac_index].ioaddr + 0x800);
			}
			else
				cls_xgmac_stats_dump_per_index(priv_data, xgmac_index, value);
		}
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}
#endif

static int do_switch(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cls_eth_priv *priv_data = NULL;
	struct udevice *dev = eth_get_dev();
	int len, interface, pos = 0, xpcs_index, ret = CMD_RET_SUCCESS;
	unsigned int sub_cmd = 0;

	if(!dev) {
		printf("Can't find udevice_eth device!\n");
		return CMD_RET_FAILURE;
	}

	//printf("eth_device name[%s] priv[%p]\n",dev->name, dev_get_priv(dev));

	if (argc < 2)
		return CMD_RET_USAGE;

	priv_data = dev_get_priv(dev);

	pos++;
	len = strlen(argv[pos]);
	if (len > 4)
		return CMD_RET_USAGE;

	memcpy(&sub_cmd, argv[pos], len);

	pos++;
	switch (sub_cmd) {

	case CMD_INIT:
		{
			printf("=)init switch\n");
			cls_eth_switch_init(dev);
			pos ++;
		}
		break;
	case CMD_STAS:
		{
#if 1
			int value = SWITCH_STATS_OPTION_ALL;

			if (argc - pos == 1) {

				char * option = argv[pos];

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
			}

			cls_switch_stats_dump(priv_data, value);
#endif
		}
		break;
	default:
		return CMD_RET_USAGE;
	}


	return CMD_RET_SUCCESS;

}

static int do_edma(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cls_eth_priv *priv_data = NULL;
	struct udevice *dev = eth_get_dev();
	int len, interface, pos = 0, xpcs_index, ret = CMD_RET_SUCCESS;
	unsigned int sub_cmd = 0;

	if(!dev) {
		printf("Can't find udevice_eth device!\n");
		return CMD_RET_FAILURE;
	}

	//printf("eth_device name[%s] priv[%p]\n",dev->name, dev_get_priv(dev));

	if (argc < 2)
		return CMD_RET_USAGE;

	priv_data = dev_get_priv(dev);

	pos++;
	len = strlen(argv[pos]);
	if (len > 4)
		return CMD_RET_USAGE;

	memcpy(&sub_cmd, argv[pos], len);

	pos++;
	switch (sub_cmd) {

	case CMD_INIT:
		{
			printf("=)init edma ==)\n ERR:function not define!!!!!\n");
			pos ++;
		}
		break;
	case CMD_STAS:
		{
#if 1
			int value = EDMA_STATS_OPTION_ALL;

			if (argc - pos == 1) {

				char * option = argv[pos];

				if (strcasecmp(option, "all") == 0) {
					value = EDMA_STATS_OPTION_ALL;
				} else if (strcasecmp(option, "test") == 0) {
					value = EDMA_STATS_OPTION_MAINT_TEST;
				} else if (strcasecmp(option, "alarm") == 0) {
					value = EDMA_STATS_OPTION_ALARM_REPORT;
				}else if (strcasecmp(option, "a_report") == 0) {
					value = EDMA_STATS_OPTION_ABNORMAL_REPORT;
				}else if (strcasecmp(option, "a_stats") == 0) {
					value = EDMA_STATS_OPTION_ABNORMAL_STATS;
				}else if (strcasecmp(option, "stats") == 0) {
					value = EDMA_STATS_OPTION_NORMAL_STATS;
				}
			}
			cls_edma_stats_dump(priv_data, value);
#endif
		}
		break;
	default:
		return CMD_RET_USAGE;
	}


	return CMD_RET_SUCCESS;

}
static int do_xpcs(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cls_eth_priv *priv_data = NULL;
	struct udevice *dev = eth_get_dev();
	int len, interface, pos = 0, xpcs_index, ret = CMD_RET_SUCCESS;
	unsigned int sub_cmd = 0;

	if(!dev) {
		printf("Can't find udevice_eth device!\n");
		return CMD_RET_FAILURE;
	}

	//printf("eth_device name[%s] priv[%p]\n",dev->name, dev_get_priv(dev));

	if (argc < 2)
		return CMD_RET_USAGE;

	priv_data = dev_get_priv(dev);

	xpcs_index = *(argv[pos] + strlen("xpcs")) - '0';
	if (xpcs_index > 1)
		return CMD_RET_USAGE;

	printf("xpcs index %d\n", xpcs_index);

	pos++;
	len = strlen(argv[pos]);
	if (len > 4)
		return CMD_RET_USAGE;

	memcpy(&sub_cmd, argv[pos], len);

	pos++;
	switch (sub_cmd) {
	case CMD_ENABLE:
		{
			int enable = simple_strtoul(argv[pos], NULL, 10);
			if (enable)
				priv_data->xpcs[xpcs_index].flags |= BIT(HW_ENABLE_BIT);
			else
				priv_data->xpcs[xpcs_index].flags &= ~BIT(HW_ENABLE_BIT);
			printf("xpcs %d has %s\n", xpcs_index, enable ? "enable" : "disable");
		}
		break;
	case CMD_SKIP:
		{
			int enable = simple_strtoul(argv[pos], NULL, 10);
			if (enable)
				priv_data->xpcs[xpcs_index].flags |= BIT(HW_INITD_BIT);
			else
				priv_data->xpcs[xpcs_index].flags &= ~BIT(HW_INITD_BIT);
			printf("xpcs %d has %s\n", xpcs_index, enable ? "skip init" : "need init");
		}
		break;
	case CMD_SKIP_SW:
		g_skip_switch = simple_strtoul(argv[pos], NULL, 10);
		printf("skip switch init is %s\n", g_skip_switch ? "true" : "false");
		break;
	case CMD_INIT:
		{
			int interface = 0;
			uint8_t usxg_mode = 0;

	        printf("====) xpcs index %d, interface = %d\n", xpcs_index, priv_data->xpcs[xpcs_index].phy_interface);
			if(xpcs_index == 0) {
				int i = 0;
				for (i = 0; i < 2; i++) {
                   if (/*IS_BITMAP_SET(xgmac_int_bitmap, 5 * i) && */ (PHY_INTERFACE_MODE_RGMII != priv_data->xgmac[i].phy_interface)) {
					   interface = priv_data->xgmac[i].phy_interface;
                            break;
				   }
				   if (i == 2) {
					   printf ("please use command mdio first\n");
					   return CMD_RET_FAILURE;
				   }
				}

			} else {

				int i = 0;
				for (i = 2; i < 5; i++) {
					if (/* IS_BITMAP_SET(xgmac_int_bitmap, 5 * i) && */(PHY_INTERFACE_MODE_RGMII != priv_data->xgmac[i].phy_interface)) {
						interface = priv_data->xgmac[i].phy_interface;
						printf("xxxxx=) get interface  xgmac[%d] interface=%d\n", i, interface);
						break;
					}
				}
				if (i == 5) {
					printf ("please use command mdio first\n");
					return CMD_RET_FAILURE;
				}
			}

			 if (interface == PHY_INTERFACE_MODE_USXGMII) {

				if (argc - pos != 1)  {
					printf("please enter usxg_mode\n");
					return CMD_RET_USAGE;
				}

				usxg_mode = simple_strtoul(argv[pos], NULL, 10);
				if (usxg_mode > USXG_MODE_MAX_NUM) {
					printf("Unkown usxg_mode[%d]\n", usxg_mode);
					return CMD_RET_FAILURE;
				}
				pos++;
			}

            priv_data->xpcs[xpcs_index].phy_interface = interface;
			priv_data->xpcs[xpcs_index].usxg_mode = usxg_mode;

			ret = cls_xpcs_config(&priv_data->xpcs[xpcs_index]);

			printf("set xpcs(%d) interface=(%s),usxg_mode=%d,ret=%d\n",xpcs_index, phy_string_for_interface(interface), usxg_mode,ret);
			pos ++;
		}
		break;
	case CMD_READ:
		{
			unsigned int addr,len,k,k2;

			if (argc - pos != 2)
				return CMD_RET_USAGE;

			addr = simple_strtoul(argv[pos], NULL, 16);
			len  = simple_strtoul(argv[pos + 1], NULL, 10);

			//printf("is read command [%#x] [%d]!\n", addr, len);
			printf("xpcs%d read: 0x%08x len: %d\n", xpcs_index, addr, len);

			k2 = len /8;

			cls_dump_regs_w(priv_data->xpcs[xpcs_index].ioaddr + addr);

			for(k = 0; k < k2; k++) {
				if(k == 0)
					continue;
				cls_dump_regs_w(priv_data->xpcs[xpcs_index].ioaddr + addr + 32*k);
			}

			cls_dump_regs_w(priv_data->xpcs[xpcs_index].ioaddr + addr + 32*k);

			pos += 2;
		}
		break;
	case CMD_WRITE:
		{
			unsigned int addr,value;

			if (argc - pos != 2)
				return CMD_RET_USAGE;

			addr = simple_strtoul(argv[pos], NULL, 16);

			value  = simple_strtoul(argv[pos + 1], NULL, 16);
			if (value > 0xffff) {
				printf("faild: the value overflow [%#x] [%#x]!!\n", addr, value);
				return;
			}
			printf("xpcs%d write: 0x%08x = 0x%08x\n",xpcs_index, addr, value);

			writew(value & 0xffff, priv_data->xpcs[xpcs_index].ioaddr + addr);
			value = readl(priv_data->xpcs[xpcs_index].ioaddr + addr);

			printf("xpcs%d read: 0x%04x = 0x%04x\n",xpcs_index, addr, value);

			pos += 2;
		}
		break;
	default:
		return CMD_RET_USAGE;
	}

	return ret;
}

static int get_phy_interface_by_name(char * name)
{
	int interface;

	for (interface = 0; interface < PHY_INTERFACE_MODE_COUNT; interface++) {
		if (strcasecmp(phy_string_for_interface(interface), name) == 0) {
			printf("interface=%d,[%s]\n", interface, name);
			break;
		}
	}

	return interface;
}
extern int g_clk_csr_h;
static int do_cls(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cls_eth_priv *priv_data = NULL;
	struct udevice *dev = eth_get_dev();
	int len, interface, pos = 0, xgmac_index, ret = CMD_RET_SUCCESS;
	char  * cmd;

	if(!dev) {
		printf("Can't find udevice_eth device!\n");
		return CMD_RET_FAILURE;
	}

	//printf("eth_device name[%s] priv[%p]\n",dev->name, dev_get_priv(dev));

	priv_data = dev_get_priv(dev);

	pos++;
	cmd = argv[pos];
	if (strcasecmp(cmd, "debug") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0')
				g_debug_cls = argv[pos][0] - '0';
			else
				g_debug_cls = 0;
		}
		printf("==) the debug mode = %d\n", g_debug_cls);
	} else if (strcasecmp(cmd, "initd") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0') {
				priv_data->xpcs[0].flags &= ~BIT(HW_INITD_BIT);
				priv_data->xpcs[1].flags &= ~BIT(HW_INITD_BIT);
				priv_data->initd = argv[pos][0] - '0';
			}
			else
				priv_data->initd = 0;
		}
		printf("==) npe hw initd = %d\n", priv_data->initd);
	} else if (strcasecmp(cmd, "rtk_print") == 0) {
    // hwp_parsedInfo_show(0);
	} else if (strcasecmp(cmd, "rtk") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0') {
		     rtk_disable = 0;
			}
			else
		     rtk_disable = 1;
		}
		printf("==) rtk Disable = %d\n", rtk_disable);
	} else if (strcasecmp(cmd, "ext_sw") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0') {
				g_skip_extswitch = 0;
			}
			else
				g_skip_extswitch = 1;
		}

		printf("skip extern switch init is %s\n", g_skip_extswitch ? "true" : "false");
	} else if (strcasecmp(cmd, "QSGMII") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0')
				g_skip_qsgmii = argv[pos][0] - '0';
			else
				g_skip_qsgmii = 0;
		}
		printf("==)skip rtk QSGMII init = %d\n", g_skip_check_phy);
	}
	else if (strcasecmp(cmd, "skip") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0')
				g_skip_check_phy = argv[pos][0] - '0';
			else
				g_skip_check_phy = 0;
		}
		printf("==)skip check phy exist = %d\n", g_skip_check_phy);
	}	else if (strcasecmp(cmd, "csr") == 0) {
		pos++;
		if(argc - pos == 1) {
			if (0  < argv[pos][0] - '0')
			 g_clk_csr_h = 1;
			else
				g_clk_csr_h = 0;
		}
		printf("==) set clk_csr %d\n", g_clk_csr_h);
	}
	else if (strcasecmp(cmd, "pll") == 0) {
		pos++;
		int lck_status;
		int refdiv, fbdiv, frac, postdiv1, postdiv2;

		if(argc - pos != 5) {
			printf("err argc num!!!!\n");
			return CMD_RET_FAILURE;
		}

		refdiv = simple_strtoul(argv[pos], NULL, 10);
		fbdiv = simple_strtoul(argv[pos + 1], NULL, 10);
		frac = simple_strtoul(argv[pos + 2 ], NULL, 10);
		postdiv1 = simple_strtoul(argv[pos + 3], NULL, 10);
		postdiv2 = simple_strtoul(argv[pos + 4], NULL, 10);
		printf("refdiv %d, fbdiv %d, frac %d, postdiv1 %d, postdiv2 %d \n", refdiv, fbdiv, frac, postdiv1, postdiv2);
		REG_W(FWD_TOP_CLK_SEL_PARA, CLK_SEL_PARA, 0);			//?D¦Ì?refclk
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

		printf("??PLL??lock,1??lock\n");
		lck_status = 0;
		if (readl_poll_timeout(FWD_TOP_PLL_PARA3,
					lck_status , (lck_status&BIT(0)), 10000)) {
			printf("Line %d: wait status[%d] timeout", __LINE__, lck_status);
			return;

#if 0
			while(lck_status == 0) {						//?D??PLL¨º?¡¤?lock¡ê?1¡ä¨²¡À¨ªlock
				lck_status = REG_R(FWD_TOP_PLL_PARA3, PLL_PARA3);
			}
#endif
			REG_W(FWD_TOP_CLK_SEL_PARA, CLK_SEL_PARA, 1);			//?D??PLL¨º?3?
		}
	}
	else if (strcasecmp(cmd, "interface") == 0) {
		int  mac_bitmap = 0, xpcs0_set = -1, xpcs1_set = -1, i;
		int * xpcs_set_ptr = NULL;
		unsigned int xgmac_index = 0;

		pos++;

		if(argc - pos < 1) {
			printf("interface num must > 0\n");
			return CMD_RET_FAILURE;
		}

		for (i = 0 ; i < 5 && i + pos < argc; i++) {

			if (strlen(argv[pos + i]) <= 2) {
				printf("FAIlD: format is <num>-<string>\n");
				return CMD_RET_FAILURE;
			}

			xgmac_index = *argv[pos + i] - '0';
			if (xgmac_index > 4) {
				printf("MAC must 0-4\n");
				return CMD_RET_FAILURE;
			}

			interface = get_phy_interface_by_name(argv[pos + i] + 2);

			if (interface == PHY_INTERFACE_MODE_COUNT) {
				printf("FAIlD: unknown interface[%s] \n", argv[pos + i] + 2);
				return CMD_RET_USAGE;
			}

			xpcs_set_ptr = xgmac_index > 0 ? &xpcs1_set: &xpcs0_set;

			if (IS_BITMAP_SET(mac_bitmap, xgmac_index)) {
				printf("The MAC(%d) has alreasy set\n", xgmac_index);
			} else {

				if (PHY_INTERFACE_MODE_RGMII != interface) {
					if ( *xpcs_set_ptr > 0 && (*xpcs_set_ptr != interface)) {
						printf("xpcs[%d] config conflicts!!!\n", xgmac_index > 1 ? 1 : 0);
						return CMD_RET_FAILURE;
					}
				}
			}

			if (interface == PHY_INTERFACE_MODE_RGMII)
				 priv_data->xgmac[xgmac_index].interface = XGMAC_INTERFACE_MODE_RGMII;
			 else
				 priv_data->xgmac[xgmac_index].interface = XGMAC_INTERFACE_MODE_NON_RGMII;

			priv_data->xgmac[xgmac_index].phy_interface = interface;
			printf("set xgmac[%d] interface = %d\n", xgmac_index, interface);
			priv_data->xgmac[xgmac_index].clk_csr = 1;/*100-150M*/

			SET_BITMAP(mac_bitmap, xgmac_index);

			*xpcs_set_ptr = priv_data->xgmac[xgmac_index].phy_interface;

			printf("[%s] set xgmac%d phy[%d]\n", __func__, xgmac_index, interface);
		}

		/*Enable FWD TOP CLK SEL PARA*/
		cls_eth_print("Enable TOP CLK SEL\n");
		writel(0x1, FWD_CLK_PARA_REG);
		mdelay(1);

		writel(0x7, DIV_RST_PARA_REG);

		/*Enable FWD SUB CLK switch */
		cls_eth_print("Enable FWD SUB clock\n");
		writel(0xf, FWD_SUB_CG_PARA_REG);
		mdelay(1);

		/*REST FWD*/
		cls_eth_print("Disable FWD sub\n");
		writel(0x0, FWD_APB_SUB_RST_REG);
		mdelay(1);

		/*De-ASSERT FWD APB*/
		cls_eth_print("Enable FWD apb\n");
		writel(0xA, FWD_APB_SUB_RST_REG);
		mdelay(1);

		//XGMAC1_PHY_INTF_SEL_I
		//0x0 non-RGMII interface (XGMII/GMII/MII)
		//0x1 RGMII interface
		//0x2 RMII  interface
		for (xgmac_index = 0; xgmac_index < 5; xgmac_index++) {
			if (IS_BITMAP_SET(mac_bitmap, xgmac_index)) {
				if (priv_data->xgmac[xgmac_index].phy_interface == PHY_INTERFACE_MODE_RGMII) {
					priv_data->xgmac[xgmac_index].interface = XGMAC_INTERFACE_MODE_RGMII;
					printf("Init xgmac[%d] RGMII address=[%#x]\n", xgmac_index, 0x53E0000C + (xgmac_index * 16));
					writel(0x1, 0x53E0000C + (xgmac_index * 16));
					switch(xgmac_index) {
					case 0:
						writel(0x1, XGMAC_PHY_INTF_EN_REG + (0 * 4));
						break;
					case 4:
						writel(0x1, XGMAC_PHY_INTF_EN_REG + (1 * 4));
						break;
					case 2:
						writel(0x1, XGMAC_PHY_INTF_EN_REG + (2 * 4));
						break;
					}
				} else {
					priv_data->xgmac[xgmac_index].interface = XGMAC_INTERFACE_MODE_NON_RGMII;
					printf("Init xgmac[%d] non-RGMII address [%#x] interface[%s]\n", xgmac_index, 0x53E0000C + (xgmac_index * 16), phy_string_for_interface(priv_data->xgmac[xgmac_index].phy_interface));
					writel(0x0, 0x53E0000C + (xgmac_index * 16));
					switch(xgmac_index) {
					case 0:
						writel(0x0, XGMAC_PHY_INTF_EN_REG + (0 * 4));
						break;
					case 4:
						writel(0x0, XGMAC_PHY_INTF_EN_REG + (1 * 4));
						break;
					case 2:
						writel(0x0, XGMAC_PHY_INTF_EN_REG + (2 * 4));
						break;
					}
				}
			}
		}

		printf("Enable FWD SUB addr[%#x], value[0x3]\n",FWD_APB_SUB_RST_REG);
		writel(0xF, FWD_APB_SUB_RST_REG);
   		mdelay(1);

		xgmac_int_bitmap  = mac_bitmap;
		printf("xgmac init map[%#x]!!\n", xgmac_int_bitmap);

	} else if (strcasecmp(cmd, "config") == 0) {
		int  mac_bitmap = 0, xpcs0_set = -1, xpcs1_set = -1, i;
		int * xpcs_set_ptr = NULL;
		unsigned int xgmac_index = 0;

		pos++;

		if(argc - pos < 1) {
			printf("interface num must > 0\n");
			return CMD_RET_FAILURE;
		}

		for (i = 0 ; i < 5 && i + pos < argc; i++) {

			if (strlen(argv[pos + i]) <= 2) {
				printf("FAIlD: format is <num>-<string>\n");
				return CMD_RET_FAILURE;
			}

			xgmac_index = *argv[pos + i] - '0';
			if (xgmac_index > 4) {
				printf("MAC must 0-4\n");
				return CMD_RET_FAILURE;
			}

			interface = get_phy_interface_by_name(argv[pos + i] + 2);

			if (interface == PHY_INTERFACE_MODE_COUNT) {
				printf("FAIlD: unknown interface[%s] \n", argv[pos + i] + 2);
				return CMD_RET_USAGE;
			}

			xpcs_set_ptr = xgmac_index > 0 ? &xpcs1_set: &xpcs0_set;

			if (IS_BITMAP_SET(mac_bitmap, xgmac_index)) {
				printf("The MAC(%d) has alreasy set\n", xgmac_index);
			} else {

				if (PHY_INTERFACE_MODE_RGMII != interface) {
					if ( *xpcs_set_ptr > 0 && (*xpcs_set_ptr != interface)) {
						printf("xpcs[%d] config conflicts!!!\n", xgmac_index > 1 ? 1 : 0);
						return CMD_RET_FAILURE;
					}
				}
			}

			if (interface == PHY_INTERFACE_MODE_RGMII)
				 priv_data->xgmac[xgmac_index].interface = XGMAC_INTERFACE_MODE_RGMII;
			 else
				 priv_data->xgmac[xgmac_index].interface = XGMAC_INTERFACE_MODE_NON_RGMII;

			priv_data->xgmac[xgmac_index].phy_interface = interface;
			printf("set xgmac[%d] interface = %d\n", xgmac_index, interface);
			priv_data->xgmac[xgmac_index].clk_csr = 1;/*100-150M*/

			//SET_BITMAP(mac_bitmap, xgmac_index);

			*xpcs_set_ptr = priv_data->xgmac[xgmac_index].phy_interface;

			printf("[%s] set xgmac%d phy[%d]\n", __func__, xgmac_index, interface);
		}

		//xgmac_int_bitmap = mac_bitmap;
		if (xpcs0_set != -1)
			priv_data->xpcs[0].phy_interface = xpcs0_set;

		if (xpcs1_set != -1)
			priv_data->xpcs[1].phy_interface = xpcs1_set;

	//	printf("xgmac init map[%#x]!!\n", xgmac_int_bitmap);
	} else if (strncasecmp(cmd,"xgmac", 5) == 0)	{
		if (xgmac_int_bitmap == 0 && !priv_data->initd) {
			printf("please  init interface first!!!!\n");
			return CMD_RET_FAILURE;
		}
#ifdef CONFIG_CLS_PHYLINK
		ret	= do_xgmac(cmdtp, flag,  argc - pos, &argv[pos]);
#endif
	} else if (strncasecmp(cmd,"xpcs", 4) == 0) {
		if (xgmac_int_bitmap == 0 && !priv_data->initd) {
			printf("please  init interface first!!!!\n");
			return CMD_RET_FAILURE;
		}
		ret = do_xpcs(cmdtp, flag,  argc - pos, &argv[pos]);
	} else if (strncasecmp(cmd,"switch", 7) == 0) {
		ret = do_switch(cmdtp, flag,  argc - pos, &argv[pos]);
	} else if (strncasecmp(cmd,"edma", 4) == 0) {
		ret = do_edma(cmdtp, flag,  argc - pos, &argv[pos]);
	} else {
		return CMD_RET_USAGE;
	}

	return ret;
}

U_BOOT_CMD(
		cls,	10,	1,	do_cls,
		"clourney xgmac xpcs utility commands",
		"interface [<mac>-<interface>](5)		- init interfce,mac (0-4), interface (SGMII/SGMII-2500/"
		"QSGMII/RMII/RGMII/USXGMII/NONE]\n"
		"cls xgmac(0-4) mdio <name> <clk_csr> <reset>- init mdio\n"
		"cls xgmac(0-4) phy <phy_addr> <pre> <reset>- init phy\n"
		"xgmac(0-4) init (10G-XGMII/2_5G-GMII/1G-GMII/100M-GMII/5G-XGMII/2_5G-XGMII/10M-MII)- "
		"init xgmac\n"
		"cls xgmac(0-4) r <reg> <num> - "
		"read xgmac register \n"
		"cls xgmac(0-4) w <reg> <0x> - "
		"write xgmac register \n"
		"cls xgmac(0-4) stas (ALL/TX/RX/Clear) - "
		"print xgmacX stats default ALL\n"
		"cls xpcs(0-1) init <usxgmii_mode> -"
		"init xpcs, usxgmii_mode in (0-10G_SXGMII,1-5G_SXGMII 2-2.5G_SXGMII,3-10G_DXGMII,4-5G_DXGMII,5-10G_QXGMII)\n"
		"cls xpcs(0-1) r <reg> <num> - "
		"read xpcs register \n"
		"cls xpcs(0-1) w <reg> <0x> - "
		"write xpcs register \n"
		"cls switch init - "
		"init switch\n"
		"cls switch stas - "
		"print switch stats (all/ingress/ipp/bm/epp/egress)\n"
		"cls edma stas (all/test/alarm/a_report/a_stats/stats)- "
		"print edma stats\n"
		"cls to_rgmii -"
		"switch qspi to XGMAC2's RGMII\n"
);

#include <net.h>
#include <net/udp.h>

#define IPERF_PACKET_LEN 1460
static u8 udp_remote_ethaddr[6]={0};
static int iperf_our_port;
static int iperf_remote_port;
static struct in_addr net_iperf_remote;
static int tx_total_len, rx_total_len, tx_total_pkt, rx_total_pkt;
struct iperf_pkt_t  {
  char buf[IPERF_PACKET_LEN];
};

int iperf_prereq(void *data)
{
	if (0 == iperf_our_port)
		iperf_our_port = 10000 + (get_timer(0) % 4096);

	if (0 == iperf_remote_port)
		iperf_remote_port = 10000 + (get_timer(0) % 4096);

	if (net_iperf_remote.s_addr == 0) {
		puts("*** ERROR: iperf remote address not given\n");
		return 1;
	}

	return 0;
}

static struct iperf_pkt_t send_pkt;
int send_pktlen = IPERF_PACKET_LEN;
static int waiting_loop_times = 0;

static void iperf_delay_restart(void)
{
	//printf("Timeout times=%d\n", waiting_loop_times);
	waiting_loop_times ++;
	if (waiting_loop_times > 2) {
		net_set_state(NETLOOP_RESTART);
	} else {
		net_set_timeout_handler(1, iperf_delay_restart);
	}
	return;
}

static void iperf_timeout_handler(void)
{
    //puts("Timeout\n");

	net_set_state(NETLOOP_RESTART);
	return;
}

static void  iperf_send(void)
{

	//	debug("%s\n", __func__);
	memcpy((char *)net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE,
			(char *)&send_pkt, send_pktlen);

	net_send_udp_packet(udp_remote_ethaddr, net_iperf_remote, iperf_remote_port,
			iperf_our_port, send_pktlen);

	tx_total_pkt ++;
	tx_total_len += send_pktlen;
	waiting_loop_times = 0;
	net_set_timeout_handler(1, iperf_delay_restart);
}

static int max_once_recv_times = 15;
static int once_recv_times = 0;

static void iperf_handler(uchar *pkt, unsigned dest, struct in_addr sip,
		unsigned src, unsigned len)
{
   // printf("%s enter==>\n", __func__);
	rx_total_pkt ++;
	rx_total_len += len;
	once_recv_times++;
	if (once_recv_times > max_once_recv_times) {
		once_recv_times = 0;
		printf("=)has recv max times\n");
		net_set_state(NETLOOP_RESTART);
	} else {
		net_set_state(NETLOOP_CONTINUE);
		net_set_timeout_handler(1, iperf_timeout_handler);
	}
}

#if 0
#endif

int iperf_start(void *data)
{
	//debug("%s\n", __func__);

	memset(&send_pkt, 0xA5, sizeof(send_pkt));
    once_recv_times = 0;
	net_set_udp_handler(iperf_handler);
	//memset(udp_remote_ethaddr, 0, sizeof(udp_remote_ethaddr));

	iperf_send();

	return 0;
}

static struct udp_ops iperf_ops = {
	.prereq = iperf_prereq,
	.start = iperf_start,
	.data = NULL,
};
int do_iperf(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{

	if (argc < 2) {
		net_iperf_remote = env_get_ip("udpremoteip");
		if (net_iperf_remote.s_addr == 0) {
			printf("udpremoteip not set\n");
			return CMD_RET_FAILURE;
		}
	} else {
		net_iperf_remote = string_to_ip(argv[1]);
		if (net_iperf_remote.s_addr == 0) {
			printf("Bad iperf remote IP address\n");
			return CMD_RET_FAILURE;
		}
		if (argc >= 3) {
			send_pktlen = simple_strtoul(argv[2], NULL, 10);
			if (send_pktlen > IPERF_PACKET_LEN)
				send_pktlen = IPERF_PACKET_LEN;
		}
		if (argc >= 4) {
			max_once_recv_times = simple_strtoul(argv[3], NULL, 10);
		}
		if (argc >= 5) {
			printf("Skip arp request\n");
			udp_remote_ethaddr[0] = 0x0;
			udp_remote_ethaddr[1] = 0x1;
			udp_remote_ethaddr[2] = 0x2;
			udp_remote_ethaddr[3] = 0x3;
			udp_remote_ethaddr[4] = 0x4;
			udp_remote_ethaddr[5] = 0x0;
		}
		else {
			printf("Open arp request\n");
			udp_remote_ethaddr[0] = 0x0;
			udp_remote_ethaddr[1] = 0x0;
			udp_remote_ethaddr[2] = 0x0;
			udp_remote_ethaddr[3] = 0x0;
			udp_remote_ethaddr[4] = 0x0;
			udp_remote_ethaddr[5] = 0x0;
		}
	}

	printf ("send udp payload size %d, once recv loop max times %d\n", send_pktlen, max_once_recv_times);

	rx_total_len = 0;
	rx_total_pkt = 0;
	tx_total_len = 0;
	tx_total_pkt = 0;

	if (udp_loop(&iperf_ops) < 0) {
		printf("IPERF failed: host %pI4 not responding\n",
				&net_iperf_remote);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

int do_iperf_info(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int i = 0;
	printf ("iperf =)\n");
	printf ("tx_pkt [%d]\n", tx_total_pkt);
	printf ("tx_len [%d]\n", tx_total_len);
	printf ("rx_pkt [%d]\n", rx_total_pkt);
	printf ("rx_len [%d]\n", rx_total_len);
	for (i = 0; i < send_pktlen; i++) {
		printf ("%02X", send_pkt.buf[i]);
		if (i%16 == 15)
			printf("\n");
	}
	printf("\n");

	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_CLS_PHYLINK
U_BOOT_CMD(
	iperf,	5,	1,	do_iperf,
	"udp send 1460 playload to remote",
	"[remote IP] [pkt_size] [once recv loop max times] [Skip arp]\n"
);

U_BOOT_CMD(
	iperf_info,	1,	1,	do_iperf_info,
	"print iperf run info",
	"\n"
);
#endif
