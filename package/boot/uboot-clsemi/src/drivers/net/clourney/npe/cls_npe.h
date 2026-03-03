/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

/* UBOOT CLS Driver main header file */

#ifndef _CLS_NPE_H_
#define _CLS_NPE_H_

#include <log.h>
#include <net.h>
#include <config.h>
#include <console.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include <fdtdec.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <asm/types.h>
#include <bitfield.h>
#include "cls_npe_bmu.h"
#include "cls_npe_edma.h"
#include "cls_npe_mac.h"

#define CLS_NPE_PADDING_ENABLE

#define CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_ENABLE	1
#define CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_MAXTRY	5
#define CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_PERIOD	10

#define CLS_NPE_MAX_TXD_PWR		12
#define CLS_NPE_MAX_DATA_PER_TXD	(1 << CLS_NPE_MAX_TXD_PWR)

#ifdef CONFIG_CLOURNEY_ETH_DBG
#define CLS_NPE_DEBUG
#endif

#define cls_eth_print(args...)	printf(args)

#define cls_eth_r32(base_addr, reg)		(readl(base_addr + reg))
#define cls_eth_w32(base_addr, reg, value)	(writel((value), base_addr + reg))

#define CLS_NPE_GET_RX_QUEUE_SIZE	1
#define CLS_NPE_GET_TX_QUEUE_SIZE	0

#define CLS_NPE_ENABLE_EDMA_TX_RX	3
#define CLS_NPE_DISABLE_EDMA_TX_RX	0

#define ALL_QUEUE_STATUS		0
#define ONLY_QUEUE_SIZE			1


#define CLS_NPE_CRG_RST_PARA_BASE		0x90414000
#define CLS_NPE_SUB_RST_PARA_OFFSET	0x002C
#define CLS_NPE_SUB_RST_PARA		(CLS_NPE_CRG_RST_PARA_BASE + CLS_NPE_SUB_RST_PARA_OFFSET)
#define CLS_NPE_SUB_SOFT_RESET		0x0
#define CLS_NPE_SUB_SOFT_UNSET		0x3
#define CLS_NPE_SUB_RESET_HOLD_TIME	5	//us

#define  CLS_NPE_MAX_XPCS_NUM   2
#define  CLS_NPE_MAX_XGMAC_NUM           5
/* USE HW */
#define HW_ENABLE_BIT              	(1)

/* HAS INIT HW*/
#define HW_INITD_BIT                (2)

/*MDIO ALREADY REGISTERED */
#define FLAGS_MDIO_INITD_BIT        (3)

/* PHY IS ALREADY RST */
#define FLAGS_PHY_RST_BIT         (4)

/* PHY IS ALREADY */
#define FLAGS_PHY_INITD_BIT         (5)

/* PHY CONFIG COMPLETE*/
#define FLAGS_PHY_CONFIG_BIT         (6)
enum {
	HSGMII_AUTONEG_IGNORE_EN,
	HSGMII_AUTONEG_OFF,
	HSGMII_AUTONEG_ON,
};
struct cls_xpcs_priv {
	void __iomem *ioaddr;
	phy_interface_t phy_interface;
	int usxg_mode ;
	int flags;
	u64 regs_size;
	int id;
	int autoneg;
	int hsgmii_autoneg;
};

struct cls_xgmac_priv {
	struct cls_eth_priv * adapter;
	phy_interface_t phy_interface;
	u32 interface;
	u32 max_speed;
	int id;
	int phy_addr;
	int pcs_sel;
	int pcs_port;
	int flags;
	int clk_csr;
	int clk_csr_h;
    int mdio_flags;
	u32 first_pre;
	int reset;
	int mdio_sel;
	int has_mdio;
	u8 fixed_link;
	u8 rgmii_rx_delay;
	u8 rgmii_tx_delay;
	char * name;
	void __iomem *ioaddr;
	struct cls_xpcs_priv *xpcs;
	struct phy_device *phydev;
	ofnode phy_handle;
	ofnode mdio_handle;
	struct mii_dev *bus;
	u64 regs_size;
};

/* This struct is based on struct cls_adapter */
struct cls_eth_priv {
	bool initd;
	/* base address and size*/
	void __iomem *edma_regs;
	void __iomem *edma_dbg_regs;
	void __iomem *switch_regs;
	struct cls_xpcs_priv  xpcs[CLS_NPE_MAX_XPCS_NUM];
	struct cls_xgmac_priv xgmac[CLS_NPE_MAX_XGMAC_NUM];

	u64 edma_regs_size;
	u64 edma_dbg_regs_size;
	u64 switch_regs_size;

	bool soft_bmu_en;
	bool token_fc_en;
	u8 split_mode;
	u32 lhost_fifo_depth;

	uchar base_mac[ARP_HLEN];
	struct udevice *udev;
     
	/* TX */
	//only support one tx queue in uboot
	struct cls_tx_ring tx_ring;

	/* RX */
	int (*alloc_rx_buf)(struct udevice *dev,
			     struct cls_rx_ring *rx_ring,
			     int cleaned_count);

	//only support one rx queue in uboot
	struct cls_rx_ring rx_ring;
	u32 rx_buffer_len;

	// RX STATUS
	unsigned int total_rx_empty_queue_cnt;
	unsigned int total_rx_pkt_err_invalid;
	unsigned int total_rx_err_cpu_tag;
	unsigned int total_rx_bytes;
	unsigned int total_rx_packets;
	// RX BUFFER
	unsigned int total_alloc_map_rx_buff_cnt;
	unsigned int total_alloc_rx_buff_skip;
	unsigned int total_alloc_rx_buff_failed;
	unsigned int total_map_rx_buff_failed;
	unsigned int total_alloc_map_rx_buff_succ;
	//Free RX
	unsigned int total_rx_clean_packets;
	unsigned int total_rx_unmap_free_packets;
	unsigned int total_rx_unmap_non_dma_packets;
	unsigned int total_rx_unmap_free_cnt;

	// TX MAP
	unsigned int total_tx_map_fifo_err_cnt;
	unsigned int total_tx_bytes;
	unsigned int total_tx_packets;
	// TX QUEUE
	unsigned int total_tx_dma_map_err_cnt;
	unsigned int total_tx_queue_fifo_err_cnt;
	unsigned int total_tx_queue_packets;
	// Free TX
	unsigned int total_tx_ack_bytes;
	unsigned int total_tx_ack_packets;
	unsigned int total_tx_ack_err_invalid_packets;
	unsigned int total_tx_ack_non_dma_packets;
	unsigned int total_tx_unmap_cnt;
};
extern int g_debug_cls;
#endif
