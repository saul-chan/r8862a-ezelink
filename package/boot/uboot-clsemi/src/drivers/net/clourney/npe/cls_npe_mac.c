// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2006 Intel Corporation. */

#include "cls_npe.h"
#include "cls_npe_bmu.h"
#include "cls_npe_fwd.h"
#include <phy.h>
#define XGMAC_TX_CONFIG			0x00000000
#define XGMAC_RX_CONFIG			0x00000004
#define XGMAC_FILTER_CONFIG		0x00000008
#define XGMAC_Q0_TX_FLOW_CONF   0x00000070
#define XGMAC_RX_FLOW_CONF      0x00000090
#define XGMAC_RXQ_CTL2_CONF     0x000000A8
#define XGMAC_RXQ_CTL3_CONF     0x000000AC
#define XGMAC_RX_CONFIG			0x00000004
#define XGMAC_FILTER_CONFIG		0x00000008
#define XGMAC_Q0_TX_FLOW_CONF   0x00000070
#define XGMAC_RX_FLOW_CONF      0x00000090
#define XGMAC_RXQ_CTL2_CONF     0x000000A8
#define XGMAC_RXQ_CTL3_CONF     0x000000AC

void  cls_xgmac_init(struct cls_xgmac_priv *port)
{
	void __iomem *ioaddr = port->ioaddr;
	struct mac_tx_configuration_reg_st tx;
	struct mac_rx_configuration_reg_st rx;
	static struct mac_packet_filter_reg_st  filter_conf = {
		.PR   = 1,                  	/*R/W Promiscuous Mode*/
		.DAIF = 1,              		/*R/W DA Inverse Filtering*/
		.PCF  = 1,            			/*R/W Pass Control Packets*/
		.SAIF = 1,                  	/*R/W SA Inverse Filtering*/
	};

	struct mac_q0_tx_flow_ctrl_reg_st  q0_tx_flow_conf = {
		.PT = 0xffff,
		.TFE = 1,
		.PLT = 1
	};

	struct mac_rx_flow_ctrl_st rx_flow_conf = {
		.RFE = 1
	};

	struct mac_rxq_ctrl2_reg_st rxq_ctl2_conf = {
		.PSRQ0 = 0xff,
		.PSRQ1 = 0xff,
		.PSRQ2 = 0xff,
		.PSRQ3 = 0xff
	};

	static struct mac_rxq_ctrl3_reg_st rxq_ctl3_conf = {
		.PSRQ4 = 0xff,
		.PSRQ5 = 0xff,
		.PSRQ6 = 0xff,
		.PSRQ7 = 0xff
	};

	tx.raw = readl(ioaddr + XGMAC_TX_CONFIG);
	rx.raw = readl(ioaddr + XGMAC_RX_CONFIG);

	if (g_debug_cls)
  	cls_eth_print("%s before tx[%#x] rx[%#x]\n",__func__, tx.raw, rx.raw);

	tx.SS= SS_1G_GMII;
	tx.TE = 0;
	tx.JD = 1;
	//rx.WD = 1;
	rx.RE = 0;
	rx.ACS = 1;
	rx.CST = 1;
	rx.IPC = 1;

	writel(tx.raw, ioaddr + XGMAC_TX_CONFIG);
	writel(rx.raw, ioaddr + XGMAC_RX_CONFIG);
	writel(filter_conf.raw,	ioaddr + XGMAC_FILTER_CONFIG);

	writel(q0_tx_flow_conf.raw,	ioaddr + XGMAC_Q0_TX_FLOW_CONF);
	writel(rx_flow_conf.raw,	ioaddr + XGMAC_RX_FLOW_CONF);
	writel(rxq_ctl2_conf.raw,	ioaddr + XGMAC_RXQ_CTL2_CONF);
	writel(rxq_ctl3_conf.raw,	ioaddr + XGMAC_RXQ_CTL3_CONF);

	if (g_debug_cls)
  	cls_eth_print("%s end tx[%#x]\nrx[%#x]\nfilter[%#x]\nq0_tx_flow[%#x]\nrx_flow[%#x]\nrxq_ctl2[%#x]\nrxq_ctl3[%#x]\n",__func__, tx.raw, rx.raw,filter_conf.raw, q0_tx_flow_conf.raw, rx_flow_conf.raw,
			rxq_ctl2_conf.raw, rxq_ctl3_conf.raw);
}

void cls_enable_xgmac(struct cls_xgmac_priv *port, bool enable)
{

	void __iomem *ioaddr = port->ioaddr;
	struct mac_tx_configuration_reg_st tx;
	struct mac_rx_configuration_reg_st rx;
	tx.raw = readl(ioaddr + XGMAC_TX_CONFIG);
	rx.raw = readl(ioaddr + XGMAC_RX_CONFIG);

	if (g_debug_cls)
		cls_eth_print("%s before tx[%#x] rx[%#x] enable[%d]\n",__func__, tx.raw, rx.raw, enable);

	if (enable) {
		tx.TE = 1;
		rx.RE = 1;
	} else {
		tx.TE = 0;
		rx.RE = 0;
	}

	writel(tx.raw, ioaddr + XGMAC_TX_CONFIG);
	writel(rx.raw, ioaddr + XGMAC_RX_CONFIG);

	if (g_debug_cls)
  	cls_eth_print("%s end tx[%#x] rx[%#x]\n",__func__, tx.raw, rx.raw);
}

void cls_xgmac_speed(struct cls_xgmac_priv *port, unsigned int speed)
{
	int speed_sel = SS_1G_GMII;
	void __iomem *ioaddr = port->ioaddr;
	struct mac_tx_configuration_reg_st tx;
	phy_interface_t  interface = port->phy_interface;
	tx.raw = readl(ioaddr + XGMAC_TX_CONFIG);

	cls_eth_print("Set XGMAC%d speed[%d]\n",port->id, speed);

	if (g_debug_cls)
		cls_eth_print("%s before tx[%#x] speed[%d]\n",__func__, tx.raw,speed);

	switch (speed) {
	case SPEED_10000:
		speed_sel = SS_10G_XGMII;
		break;
	case SPEED_2500:
		if (interface == PHY_INTERFACE_MODE_USXGMII)
			speed_sel = SS_2Dot5G_XGMII;
		else
			speed_sel = SS_1G_GMII;
		break;
	case 5000:
		speed_sel = SS_5G_XGMII;
		break;
	case SPEED_1000:
		speed_sel = SS_1G_GMII;
		break;
	case SPEED_100:
		speed_sel = SS_100M_GMII;
		break;
	case SPEED_10:
		speed_sel = SS_10M_MII;
		break;
	default:
		speed_sel = SS_1G_GMII;
		break;
	}

    tx.SS = speed_sel;

	writel(tx.raw, ioaddr + XGMAC_TX_CONFIG);

	if (port->interface == XGMAC_INTERFACE_MODE_RGMII) {

		switch (speed) {
		case SPEED_10:
			speed_sel = 7;
			break;
		case SPEED_100:
			speed_sel = 4;
			break;
		case SPEED_1000:
			speed_sel = 3;
			break;
		}

		writel(speed_sel, XGMAC_RGMII_IO_SPEED_ADDR(port->id));
	}

	if (g_debug_cls)
		cls_eth_print("%s end tx[%#x]\n",__func__, tx.raw);
}


s32 cls_gmac1_config(struct udevice *dev, u8 enable)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	void __iomem *gmac1_base_addr = priv_data->xgmac[2].ioaddr;

	if (enable) {
		writel(0x60000001, gmac1_base_addr + 0x0);
		writel(0x207, gmac1_base_addr + 0x4);
		writel(0x80000000, gmac1_base_addr + 0x8);
	} else {
		writel(0, gmac1_base_addr + 0x8);
		writel(0, gmac1_base_addr + 0x4);
		writel(0, gmac1_base_addr + 0x0);
	}

	val = readl(gmac1_base_addr);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x00 = %x\n", __func__, val);
#endif
	val = readl(gmac1_base_addr+4);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x04 = %x\n", __func__, val);
#endif
	val = readl(gmac1_base_addr + 0x8);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x08 = %x\n", __func__, val);
#endif
	return 0;
}

s32 cls_gmac2_config(struct udevice *dev, u8 enable)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	void __iomem *gmac2_base_addr = priv_data->xgmac[3].ioaddr;

	if (enable) {
		writel(0x60000001, gmac2_base_addr + 0x0);
		writel(0x207, gmac2_base_addr + 0x4);
		writel(0x80000000, gmac2_base_addr + 0x8);
	} else {
		writel(0, gmac2_base_addr + 0x8);
		writel(0, gmac2_base_addr + 0x4);
		writel(0, gmac2_base_addr + 0x0);
	}

	val = readl(gmac2_base_addr);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x00 = %x\n", __func__, val);
#endif
	val = readl(gmac2_base_addr + 0x4);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x04 = %x\n", __func__, val);
#endif
	val = readl(gmac2_base_addr + 0x8);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x08 = %x\n", __func__, val);
#endif

	return 0;
}

s32 cls_gmac3_config(struct udevice *dev, u8 enable)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	void __iomem *gmac3_base_addr = priv_data->xgmac[4].ioaddr;

	if (enable) {
		writel(0x60000001, gmac3_base_addr + 0x0);
		writel(0x207, gmac3_base_addr + 0x4);
		writel(0x80000000, gmac3_base_addr + 0x8);
	} else {
		writel(0, gmac3_base_addr + 0x8);
		writel(0, gmac3_base_addr + 0x4);
		writel(0, gmac3_base_addr + 0x0);
	}

	val = readl(gmac3_base_addr);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x00 = %x\n", __func__, val);
#endif
	val = readl(gmac3_base_addr + 0x4);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x04 = %x\n", __func__, val);
#endif
	val = readl(gmac3_base_addr + 0x8);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x08 = %x\n", __func__, val);
#endif
	return 0;
}

s32 cls_xgmac0_config(struct udevice *dev, u8 enable)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	void __iomem *xgmac0_base_addr = priv_data->xgmac[0].ioaddr;

	if (enable) {
		writel(0x80000000, xgmac0_base_addr + 0x8);
		writel(0x60000001, xgmac0_base_addr + 0x0);
		writel(0x207, xgmac0_base_addr + 0x4);
	} else {
		writel(0, xgmac0_base_addr + 0x8);
		writel(0, xgmac0_base_addr + 0x0);
		writel(0, xgmac0_base_addr + 0x4);
	}

	val = readl(xgmac0_base_addr);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x00 = %x\n", __func__, val);
#endif
	val = readl(xgmac0_base_addr+0x4);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x04 = %x\n", __func__, val);
#endif
	val = readl(xgmac0_base_addr+0x8);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x08 = %x\n", __func__, val);
#endif
	return 0;
}

s32 cls_xgmac1_config(struct udevice *dev, u8 enable)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	u32 val = 0;
	void __iomem *xgmac1_base_addr = priv_data->xgmac[1].ioaddr;

	if (enable) {
		writel(0x80000000, xgmac1_base_addr + 0x8);
		writel(0x1, xgmac1_base_addr + 0x0);
		writel(0x207, xgmac1_base_addr + 0x4);
	} else {
		writel(0, xgmac1_base_addr + 0x8);
		writel(0, xgmac1_base_addr + 0x0);
		writel(0, xgmac1_base_addr + 0x4);
	}

	val = readl(xgmac1_base_addr);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x00 = %x\n", __func__, val);
#endif
	val = readl(xgmac1_base_addr+0x4);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x04 = %x\n", __func__, val);
#endif
	val = readl(xgmac1_base_addr + 0x8);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("%s 0x08 = %x\n", __func__, val);
#endif
	return 0;
}

void cls_eth_mac_pcs_config(struct udevice *dev, u8 enable)
{
	cls_xgmac0_config(dev, enable);
	cls_xgmac1_config(dev, enable);
	cls_gmac1_config(dev, enable);
	cls_gmac2_config(dev, enable);
	cls_gmac3_config(dev, enable);
	udelay(200);

#ifdef CLS_NPE_DEBUG
	cls_eth_print("[%s] enable=%d done!\n", __func__, enable);
#endif
}
