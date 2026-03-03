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

/* e1000_mac.c
 * Shared functions for accessing and configuring the MAC + PCS
 */

#include "dubhe2000_mac.h"
#include "dubhe2000_xpcs.h"
#define XGMAC_TX_CONFIG	       0x00000000
#define XGMAC_RX_CONFIG	       0x00000004
#define XGMAC_FILTER_CONFIG    0x00000008
#define XGMAC_Q0_TX_FLOW_CONF  0x00000070
#define XGMAC_RX_FLOW_CONF     0x00000090
#define XGMAC_RXQ_CTL2_CONF    0x000000A8
#define XGMAC_RXQ_CTL3_CONF    0x000000AC
#define XGMAC_INTERRUPT_ENABLE 0x000000B4
#define XGMAC_INTERRUPT_STATUS 0x000000B0

#define FLOW_OFF  0
#define FLOW_RX	  1
#define FLOW_TX	  2
#define FLOW_AUTO (FLOW_TX | FLOW_RX)

#define XGMAC_RX_FLOW_CTRL	     0x00000090
#define XGMAC_Qx_TX_FLOW_CTRL(x)     (0x00000070 + (x)*4)
#define XGMAC_EXTENDED_CONFIGURATION 0x140

struct mac_extended_configuration_reg_st {
	union {
		u32 raw;
		struct {
			u32	EIPG:7,
				DDS:1,
				VPRE:1,
				TPRE:1,
				Reserved_15_10:6,
				SBDIOEN:1,
				DC:1,
				BL:2,
				DR:1,
				DCRS:1,
				DO:1,
				ECRSFD:1,
				HD:1,
				Reserved_31_25:7;
		};
	};
} __packed;

struct mac_tx_configuration_reg_st {
	union {
		u32 raw;
		struct {
			u32	TE:1,		  /*R/W Transmitter Enable*/
				DDIC:1,		  /*R/W Disable DIC Algorithm*/
				Reserved_2:1,	  /*R*/
				ISM:1,		  /*R/W IFG Stretch Ratio*/
				ISR:4,		  /*R/W IFG Stretch Mode*/
				IPG:3,		  /*R/W Inter-Packet Gap*/
				IFP:1,		  /*R/W IPG Control*/
				TC:1,		  /*R/W Transmit Configuration in RGMII*/
				LUD:1,		  /*R/W Link Up or Down*/
				Reserved_15_14:2, /*R*/
				JD:1,		  /*R/W Jabber Disable*/
				Reserved_17:1,	  /*R*/
				PCHM:1,		  /*R/W PCHM Mode*/
				PEN:1,		  /*R/W PCH Enable*/
				SARC:3,		  /*R/W Source Address Insertion or Replacement Control*/
				Reserved_23:1,	  /*R */
				VNE:1,		  /*R/W VxLAN-NVGRE Mode*/
				VNM:1,		  /*R/W VxLAN-NVGRE Mode*/
				Reserved_26:1,	  /*R*/
				GT9WH:1,	  /*R/W G9991 Without Ethernet Header encapsulation enable*/
				G9991EN:1,	  /*R/W G999_1 Mode enable*/
				SS:3;		  /*Speed Selection*/
		};
	};
} __packed;

struct mac_rx_configuration_reg_st {
	union {
		u32 raw;
		struct {
			u32	RE:1,	  /*R/W Receiver Enable*/
				ACS:1,	  /*R/W Automatic Pad or CRC Stripping*/
				CST:1,	  /*R/W CRC Stripping for Type packets*/
				DCRCC:1,  /*R/W Disable CRC Checking for Received Packets*/
				SPEN:1,	  /*R/W Slow Protocol Detection Enable*/
				USP:1,	  /*R/W Unicast Slow Protocol Packet Detect*/
				GPSLCE:1, /*R/W Giant Packet Size Limit Control Enable*/
				WD:1,	  /*R/W Watchdog Disable*/
				JE:1,	  /*R/W Jumbo Packet Enable*/
				IPC:1,	  /*R/W Checksum Offload*/
				LM:1,	  /*R/W Loopback Mode*/
				S2KP:1,	  /*R/W IEEE 802.3as Support for 2k Packets*/
				HDSMS:3,  /*R/W Maximum Size for Splitting the Header Data*/
				PRXM:1,	  /*R/W PCH Rx Mode*/
				GPSL:14,  /*R/W Giant Packet Size Limit*/
				ELEN:1,	  /*R/W External lookup enable*/
				ARPEN:1;  /*R/W ARP enable*/
		};
	};
} __packed;

struct mac_packet_filter_reg_st {
	union {
		u32 raw;
		struct {
			u32	PR:1,			  /*R/W Promiscuous Mode*/
				HUC:1,			  /*R/W Hash Unicast*/
				HMC:1,			  /*R/W Hash Multicast*/
				DAIF:1,			  /*R/W DA Inverse Filtering*/
				PM:1,			  /*R/W Pass All Multicast*/
				DBF:1,			  /*R/W Disable Broadcast Packets*/
				PCF:2,			  /*R/W Pass Control Packets*/
				SAIF:1,			  /*R/W SA Inverse Filtering*/
				SAF:1,			  /*R/W Source Address Filter Enable*/
				HPF:1,			  /*R/W Hash or Perfect Filter*/
				DHLFRS:2,		  /*R/W DA Hash Index or L3/L4 Filter Number in Receive Status*/
				Reserved_15_13:3, VTFE:1, /*R/W VLAN Tag Filter Enable*/
				Reserved_19_17:3,	  /*R*/
				IPFE:1,			  /*R/W Layer 3 and Layer 4 Filter Enable*/
				DNTU:1,			  /*R/W Drop Non-TCP/UDP over IP Packets*/
				VUCC:1,			  /*R/W Vxlan UDP/IPv6 Checksum Control*/
				Reserved_30_23:8,	  /*R*/
				RA:1;			  /*R/W Receive ALL*/
		};
	};
} __packed;

struct mac_q0_tx_flow_ctrl_reg_st {
	union {
		u32 raw;
		struct {
			u32	PCB:1,		 /*R/W Flow Control Busy or Backpressure Activate*/
				TFE:1,		 /*R/W Transmit Flow Control Enable*/
				Reserved_3_2:2,	 /*R*/
				PLT:3,		 /*R/W Pause Low Threshold*/
				DZPQ:1,		 /*R/W Disable Zero-Quanta Pause*/
				Reserved_15_8:8, /*R*/
				PT:16;		 /*R/W Pause Time*/
		};
	};
} __packed;

struct mac_rx_flow_ctrl_st {
	union {
		u32 raw;
		struct {
			u32	RFE:1,		  /*R/W Receive Flow Control Enable*/
				UP:1,		  /*R/W Unicast Pause Packet Detect*/
				Reserved_7_2:6,	  /*R*/
				PFCE:1,		  /*R/W Priority Based Flow Control Enable*/
				Reserved_31_9:23; /*R*/
		};
	};
} __packed;

struct mac_rxq_ctrl2_reg_st {
	union {
		u32 raw;
		struct {
			u32	PSRQ0:8, /*R/W Priorities Selected in the Receive Queue 0*/
				PSRQ1:8, /*R/W Priorities Selected in the Receive Queue 1*/
				PSRQ2:8, /*R/W Priorities Selected in the Receive Queue 2*/
				PSRQ3:8; /*R/W Priorities Selected in the Receive Queue 3*/
		};
	};
} __packed;

struct mac_rxq_ctrl3_reg_st {
	union {
		u32 raw;
		struct {
			u32	PSRQ4:8, /*R/W Priorities Selected in the Receive Queue 4*/
				PSRQ5:8, /*R/W Priorities Selected in the Receive Queue 5*/
				PSRQ6:8, /*R/W Priorities Selected in the Receive Queue 6*/
				PSRQ7:8; /*R/W Priorities Selected in the Receive Queue 7*/
		};
	};
} __packed;

#ifdef CONFIG_DUBHE2000_PHYLINK
void dubhe1000_xgmac_init(struct dubhe1000_mac *port, struct net_device *dev)
{
	void __iomem *ioaddr = port->ioaddr;
	struct mac_tx_configuration_reg_st tx;
	struct mac_rx_configuration_reg_st rx;
	static struct mac_packet_filter_reg_st filter_conf = { 0 };

	struct mac_q0_tx_flow_ctrl_reg_st q0_tx_flow_conf = {
		.PT = 0xffff,
		.TFE = 1,
		.PLT = 1,
	};

	struct mac_rx_flow_ctrl_st rx_flow_conf = {
		.RFE = 1,
	};

	struct mac_rxq_ctrl2_reg_st rxq_ctl2_conf = {
		.PSRQ0 = 0xff,
		.PSRQ1 = 0xff,
		.PSRQ2 = 0xff,
		.PSRQ3 = 0xff,
	};

	static struct mac_rxq_ctrl3_reg_st rxq_ctl3_conf = {
		.PSRQ4 = 0xff,
		.PSRQ5 = 0xff,
		.PSRQ6 = 0xff,
		.PSRQ7 = 0xff,
	};

	tx.raw = readl(ioaddr + XGMAC_TX_CONFIG);
	rx.raw = readl(ioaddr + XGMAC_RX_CONFIG);

	e_info(hw, "%s before tx[%#x] rx[%#x] mac_sel_speed[%d]", __func__, tx.raw, rx.raw, port->mac_port_sel_speed);

	if (port->mac_port_sel_speed != -1) {
		switch (port->mac_port_sel_speed) {
		case SS_10G_XGMII:
		case SS_2Dot5G_GMII:
		case SS_1G_GMII:
		case SS_100M_GMII:
		case SS_5G_XGMII:
		case SS_2Dot5G_XGMII:
		case SS_10M_MII:
			tx.SS = port->mac_port_sel_speed;
			break;
		default:
			tx.SS = SS_1G_GMII;
			break;
		}
	}

	tx.TE = 0;
	tx.JD = 1;
	//rx.WD = 1;
	rx.RE = 0;
	/* enable GPSLCE=1, GPSL=1540
	 * pkt < GPSL(1540) is considered as normal(not jambo) pkt
	 * only let pkt <= 1540 pkt in
	 * GPSL = 1518 (1500 +14(eth header) +4(fcs)) by defaut
	 * in current npe driver don't support pkt len > 2000
	 */
	//rx.JE = 1;
	rx.GPSLCE = 1;
	rx.GPSL = 1540;
	rx.ACS = 1;
	rx.CST = 1;
	rx.IPC = 1;
	rx.LM = 1;

	writel(tx.raw,			ioaddr + XGMAC_TX_CONFIG);
	writel(rx.raw,			ioaddr + XGMAC_RX_CONFIG);
	writel(filter_conf.raw,		ioaddr + XGMAC_FILTER_CONFIG);
	writel(q0_tx_flow_conf.raw,	ioaddr + XGMAC_Q0_TX_FLOW_CONF);
	writel(rx_flow_conf.raw,	ioaddr + XGMAC_RX_FLOW_CONF);
	writel(rxq_ctl2_conf.raw,	ioaddr + XGMAC_RXQ_CTL2_CONF);
	writel(rxq_ctl3_conf.raw,	ioaddr + XGMAC_RXQ_CTL3_CONF);
	writel(0x0, ioaddr + XGMAC_INTERRUPT_ENABLE);

	e_info(hw,
	       "%s end tx[%#x]\nrx[%#x]\nfilter[%#x]\nq0_tx_flow[%#x]\nrx_flow[%#x]\nrxq_ctl2[%#x]\nrxq_ctl3[%#x]\nmac_sel_speed[%d]",
	       __func__, tx.raw, rx.raw, filter_conf.raw, q0_tx_flow_conf.raw, rx_flow_conf.raw, rxq_ctl2_conf.raw,
	       rxq_ctl3_conf.raw, port->mac_port_sel_speed);
}

void dubhe1000_enable_xgmac_interrupt(struct dubhe1000_mac *port, bool enable)
{
	void __iomem *ioaddr = port->ioaddr;

	if (enable)
		writel(0xFFFFFFFF, ioaddr + XGMAC_INTERRUPT_ENABLE);
	else
		writel(0x0, ioaddr + XGMAC_INTERRUPT_ENABLE);
}

int dubhe1000_xgmac_get_link_status(struct dubhe1000_mac *port)
{
	uint32_t val = 0;
	int status = 0;

	if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
		val = dubhe1000_xpcs_get_sts2(port->adapter->pcs[port->pcs_sel]);
		status = !(BIT(10) & val);
	} else {
		status = 1;
	}

	return status;
}

void dubhe1000_xgmac_duplex(struct dubhe1000_mac *port, int duplex)
{
	struct mac_extended_configuration_reg_st reg;
	void __iomem *ioaddr = port->ioaddr;

	port->duplex = duplex;

	reg.raw = readl(ioaddr + XGMAC_EXTENDED_CONFIGURATION);

	if (duplex == DUPLEX_HALF)
		reg.HD = 1;
	else
		reg.HD = 0;

	writel(reg.raw, ioaddr + XGMAC_EXTENDED_CONFIGURATION);
}

void dubhe1000_enable_xgmac(struct dubhe1000_mac *port, bool enable)
{
	void __iomem *ioaddr = port->ioaddr;
	struct mac_tx_configuration_reg_st tx;
	struct mac_rx_configuration_reg_st rx;
	static struct mac_packet_filter_reg_st filter_conf = { 0 };

	tx.raw = readl(ioaddr + XGMAC_TX_CONFIG);
	rx.raw = readl(ioaddr + XGMAC_RX_CONFIG);

	e_info(hw, "%s before tx[%#x] rx[%#x] enable[%d] ", __func__, tx.raw, rx.raw, enable);

	if (enable) {
		tx.TE = 1;
		rx.RE = 1;
		rx.LM = 0;
		filter_conf.PR = 1;   /*R/W Promiscuous Mode*/
		filter_conf.DAIF = 1; /*R/W DA Inverse Filtering*/
		filter_conf.PCF = 1;  /*R/W Pass Control Packets*/
		filter_conf.SAIF = 1; /*R/W SA Inverse Filtering*/
		port->link = 1;
	} else {
		rx.LM = 1;
		writel(rx.raw, ioaddr + XGMAC_RX_CONFIG);
		mdelay(1);
		tx.TE = 0;
		rx.RE = 0;
		port->link = 0;
	}

	writel(filter_conf.raw, ioaddr + XGMAC_FILTER_CONFIG);
	writel(tx.raw, ioaddr + XGMAC_TX_CONFIG);
	writel(rx.raw, ioaddr + XGMAC_RX_CONFIG);

	e_info(hw, "%s end tx[%#x] rx[%#x]", __func__, tx.raw, rx.raw);
}

void dubhe1000_xgmac_flow_ctrl(struct dubhe1000_mac *port, u32 duplex)
{
	void __iomem *ioaddr = port->ioaddr;
	unsigned int fc = port->flow_ctrl;
	u32 tx_cnt = port->tx_queues_to_use;
	unsigned int pause_time = port->pause;
	struct mac_rx_flow_ctrl_st rx_flow = { .raw = 0 };
	struct mac_q0_tx_flow_ctrl_reg_st tx_flow = { .raw = 0 };
	u32 i;

	if (fc & FLOW_RX) {
		rx_flow.raw = readl(ioaddr + XGMAC_RX_FLOW_CTRL);
		rx_flow.RFE = 1;
		writel(rx_flow.raw, ioaddr + XGMAC_RX_FLOW_CTRL);
	}

	if (fc & FLOW_TX) {
		for (i = 0; i < tx_cnt; i++) {
			tx_flow.raw = readl(ioaddr + XGMAC_Qx_TX_FLOW_CTRL(i));
			tx_flow.TFE = 1;
			if (duplex)
				tx_flow.PT = pause_time;
			writel(tx_flow.raw, ioaddr + XGMAC_Qx_TX_FLOW_CTRL(i));
		}
	}

	e_info(hw, "%s end fc[%#x] tx_flow[%#x] rx_flow[%#x]", __func__, fc, tx_flow.raw, rx_flow.raw);
}

void dubhe1000_xgmac_speed(struct dubhe1000_mac *port, unsigned int speed)
{
	unsigned int speed_sel = SS_1G_GMII;
	void __iomem *ioaddr = port->ioaddr;
	struct mac_tx_configuration_reg_st tx;

	port->speed = speed;
	switch (speed) {
	case SPEED_10000:
		speed_sel = SS_10G_XGMII;
		break;
	case SPEED_2500:
		if (port->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
			port->phy_interface == PHY_INTERFACE_MODE_10GBASER)
			speed_sel = SS_2Dot5G_XGMII;
		else
			speed_sel = SS_1G_GMII;
		break;
	case SPEED_5000:
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
		pr_info("%s unknown speed %d", __func__, speed);
		speed_sel = SS_1G_GMII;
		break;
	}

	tx.raw = readl(ioaddr + XGMAC_TX_CONFIG);

	e_info(hw, "%s before tx[%#x] speed[%d]", __func__, tx.raw, speed);

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

		writel(speed_sel, XGMAC_RGMII_IO_SPEED_ADDR(port->adapter, port->id));
	}

	e_info(hw, "%s end tx[%#x]", __func__, tx.raw);
}
#elif defined(CONFIG_DUBHE2000_DEVLINK)
static struct mac_tx_configuration_reg_st s_tx_conf = {
	.TE = 1,
	.SS = SS_10G_XGMII,
};

static struct mac_rx_configuration_reg_st s_rx_conf = {
	.RE = 1,
	.ACS = 1,
	.CST = 1,
	.IPC = 1,
};

static struct mac_packet_filter_reg_st s_filter_conf = {
	.PR = 1,   /*R/W Promiscuous Mode*/
	.DAIF = 1, /*R/W DA Inverse Filtering*/
	.PCF = 1,  /*R/W Pass Control Packets*/
	.SAIF = 1, /*R/W SA Inverse Filtering*/
};

static struct mac_q0_tx_flow_ctrl_reg_st s_q0_tx_flow_conf = {
	.PT = 0xffff,
	.TFE = 1,
	.PLT = 1,
};

static struct mac_rx_flow_ctrl_st s_mac_rx_flow_conf = {
	.RFE = 1,
};

static struct mac_rxq_ctrl2_reg_st s_mac_rxq_ctl2_conf = {
	.PSRQ0 = 0xff,
	.PSRQ1 = 0xff,
	.PSRQ2 = 0xff,
	.PSRQ3 = 0xff,
};

static struct mac_rxq_ctrl3_reg_st s_mac_rxq_ctl3_conf = {
	.PSRQ4 = 0xff,
	.PSRQ5 = 0xff,
	.PSRQ6 = 0xff,
	.PSRQ7 = 0xff,
};

void __iomem *xpcs0_base_addr = NULL;
void __iomem *xpcs1_base_addr = NULL;

/**
 * dubhe1000_xcps0_init - Performs basic configuration of the XCPS0.
 *
 */
s32 dubhe1000_xcps0_init(struct dubhe1000_adapter *adapter)
{
	u32 val = 0;

	if (!adapter->pcs[0])
		return 0;

	xpcs0_base_addr = adapter->pcs[0]->ioaddr;

	val = readl(xpcs0_base_addr);
	pr_info("%s %x\n", __func__, val);

	return 0;
}

/**
 * dubhe1000_xcps1_init - Performs basic configuration of the XCPS1.
 *
 */
s32 dubhe1000_xcps1_init(struct dubhe1000_adapter *adapter)
{
	u32 val = 0;

	if (!adapter->pcs[1])
		return 0;

	xpcs1_base_addr = adapter->pcs[1];

	val = readl(xpcs1_base_addr);
	pr_info("%s %x\n", __func__, val);

	return 0;
}

/**
 * dubhe1000_xgmac_config - Performs basic configuration of the XGMACx.
 *
 */
s32 dubhe1000_xgmac_config(u8 port_id, struct dubhe1000_adapter *adapter, u8 enable)
{
	u32 val = 0;
	void __iomem *xgmac_base_addr;

	if (!adapter->mac[port_id])
		return 0;

	xgmac_base_addr = adapter->mac[port_id]->ioaddr;

	if (enable) {
		writel(s_tx_conf.raw,		xgmac_base_addr + 0x0);
		writel(s_rx_conf.raw,		xgmac_base_addr + 0x4);
		writel(s_filter_conf.raw,	xgmac_base_addr + 0x8);
		writel(s_q0_tx_flow_conf.raw,	xgmac_base_addr + 0x70);
		writel(s_mac_rx_flow_conf.raw,	xgmac_base_addr + 0x90);
		writel(s_mac_rxq_ctl2_conf.raw,	xgmac_base_addr + 0xa8);
		writel(s_mac_rxq_ctl3_conf.raw,	xgmac_base_addr + 0xac);
	} else {
		writel(0, xgmac_base_addr + 0x0);
		writel(0, xgmac_base_addr + 0x4);
		writel(0, xgmac_base_addr + 0x8);
		writel(0, xgmac_base_addr + 0x70);
		writel(0, xgmac_base_addr + 0x90);
		writel(0, xgmac_base_addr + 0xa8);
		writel(0, xgmac_base_addr + 0xac);
	}

	pr_info("%s port_id = %d\n", __func__, port_id);

	val = readl(xgmac_base_addr);
	pr_info("%s 0x00 = %#x old is 0x1\n", __func__, val);

	val = readl(xgmac_base_addr + 0x4);
	pr_info("%s 0x04 = %#x old is 0x207\n", __func__, val);

	val = readl(xgmac_base_addr + 0x8);
	pr_info("%s 0x08 = %#x old is 0x80000000\n", __func__, val);

	val = readl(xgmac_base_addr + 0x70);
	pr_info("%s 0x70 = %#x old is 0x00000000\n", __func__, val);

	val = readl(xgmac_base_addr + 0x90);
	pr_info("%s 0x90 = %#x old is 0x00000000\n", __func__, val);

	val = readl(xgmac_base_addr + 0xa8);
	pr_info("%s 0xa8 = %#x old is 0x00000000\n", __func__, val);

	val = readl(xgmac_base_addr + 0xac);
	pr_info("%s 0xac = %#x old is 0x00000000\n", __func__, val);

	return 0;
}

void dubhe1000_mac_pcs_config(struct dubhe1000_adapter *adapter, u8 enable)
{
	u8 port_id = 0;

	for (; port_id < ARRAY_SIZE(adapter->mac); port_id++)
		dubhe1000_xgmac_config(port_id, adapter, enable);
}
#else
#error "Please choice a phy management method"
#endif
