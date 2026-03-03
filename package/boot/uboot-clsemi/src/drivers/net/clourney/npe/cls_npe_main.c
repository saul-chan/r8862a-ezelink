// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <config.h>
#include <console.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/io.h>
#include <phy.h>
#include <miiphy.h>
#include <fdtdec.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/iopoll.h>
#include "cls_npe.h"
#include "cls_npe_mac.h"
#include "cls_npe_edma.h"
#include "cls_npe_bmu.h"
#include "cls_npe_tag.h"
#include "cls_npe_switch.h"
#include "cls_npe_fwd.h"
#define  MDIO_SEL_OFFSET(index)  (0x284 + ((index) * 4))
extern int g_skip_check_phy;
int g_skip_switch = 0;
int g_skip_extswitch = 0;
#define	LINK_TIMEOUT_US	(8000 * 1000)
DECLARE_GLOBAL_DATA_PTR;

int g_debug_cls;
static int cls_eth_free_rx_pkt(struct udevice *dev, uchar *packet, int length);
static void cls_clean_sw_bmu_tx_irq(struct udevice *dev);
static void cls_eth_stop(struct udevice *dev);

/**
 * ofnode_get_phy_mode - Get phy mode for given device_node
 * @np:	Pointer to the given ofnode
 * @interface: Pointer to the result
 *
 * The function gets phy interface string from property 'phy-mode' or
 * 'phy-connection-type'. The index in phy_modes table is set in
 * interface and 0 returned. In case of error interface is set to
 * PHY_INTERFACE_MODE_NONE and an errno is returned, e.g. -ENODEV.
 */
static int ofnode_get_phy_mode(ofnode np, phy_interface_t *interface)
{
	const char *pm;
	int  i;

	*interface = PHY_INTERFACE_MODE_NONE;

	pm = ofnode_read_string(np, "phy-mode");
	if (!pm)
		pm = ofnode_read_string(np, "phy-connection-type");
	if (!pm)
		return -ENODEV;
	if (g_debug_cls)
		printf("%s phy-mode %s\n", __func__, pm);
	for (i = 0; i < PHY_INTERFACE_MODE_COUNT; i++) {
		if (0 == strcasecmp(pm,
					phy_interface_strings[i])) {
			*interface = i;
		return 0;
		}
	}

	return -ENODEV;
}
// Some counters are used to debug TX/RX during once net_loop().
// Reset those counters before the end of this net_loop()
static void cls_eth_reset_counter(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);

	//RX STATUS
	priv_data->total_rx_empty_queue_cnt = 0;
	priv_data->total_rx_pkt_err_invalid = 0;
	priv_data->total_rx_err_cpu_tag = 0;
	priv_data->total_rx_bytes = 0;
	priv_data->total_rx_packets = 0;
	//FREE RX
	priv_data->total_rx_unmap_free_packets = 0;
	priv_data->total_rx_unmap_non_dma_packets = 0;
	priv_data->total_rx_clean_packets = 0;

	priv_data->total_alloc_map_rx_buff_cnt = 0;
	priv_data->total_alloc_map_rx_buff_succ = 0;

	// TX MAP
	priv_data->total_tx_map_fifo_err_cnt = 0;
	priv_data->total_tx_dma_map_err_cnt = 0;
	priv_data->total_tx_bytes = 0;
	priv_data->total_tx_packets = 0;
	// TX QUEUE
	priv_data->total_tx_queue_fifo_err_cnt = 0;
	priv_data->total_tx_queue_packets = 0;
	// Free TX
	priv_data->total_tx_ack_bytes = 0;
	priv_data->total_tx_ack_packets = 0;
	priv_data->total_tx_ack_err_invalid_packets = 0;
	priv_data->total_tx_ack_non_dma_packets = 0;
	priv_data->total_tx_unmap_cnt = 0;
}

static void  cls_eth_dump_counter(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);

	cls_eth_print("[%s] TX/RX statistics:\n", dev->name);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls) {
	cls_eth_print("RING BUFFER TX(next_use=%d next_clean=%d) RX(next_use=%d next_clean=%d)\n",
					priv_data->tx_ring.next_to_use, priv_data->tx_ring.next_to_clean,
					priv_data->rx_ring.next_to_use, priv_data->rx_ring.next_to_clean);

	cls_eth_print("RX STATS: empty_queue(%d) err_cpu_tag(%d) pkterr_invalid(%d) packets(%d) bytes(%d)\n",
					priv_data->total_rx_empty_queue_cnt, priv_data->total_rx_err_cpu_tag,
					priv_data->total_rx_pkt_err_invalid,
					priv_data->total_rx_packets, priv_data->total_rx_bytes);

	cls_eth_print("RX BUFFER: alloc_map_cnt(%d) alloc_skip(%d) alloc_failed(%d) map_failed(%d) succ(%d)\n",
					priv_data->total_alloc_map_rx_buff_cnt, priv_data->total_alloc_rx_buff_skip,
					priv_data->total_alloc_rx_buff_failed, priv_data->total_map_rx_buff_failed,
					priv_data->total_alloc_map_rx_buff_succ);

	cls_eth_print("FREE RX: unmap_free_packets(%d) non-dma(%d) clean(%d) total(%d)\n",
					priv_data->total_rx_unmap_free_packets,
					priv_data->total_rx_unmap_non_dma_packets,
					priv_data->total_rx_clean_packets,
					priv_data->total_rx_unmap_free_cnt);

	cls_eth_print("TX MAP: fifo_err(%d) dma_map_err(%d) packets(%d) bytes(%d)\n",
					priv_data->total_tx_map_fifo_err_cnt, priv_data->total_tx_dma_map_err_cnt,
					priv_data->total_tx_packets, priv_data->total_tx_bytes);

	cls_eth_print("TX QUEUE: fifo_err(%d) packets(%d)\n",
					priv_data->total_tx_queue_fifo_err_cnt, priv_data->total_tx_queue_packets);

	cls_eth_print("FREE TX: ack_err_invalid(%d) non-dma(%d) ack_bytes(%d) ack_packets(%d) unmap(%d)\n",
					priv_data->total_tx_ack_err_invalid_packets, priv_data->total_tx_ack_non_dma_packets,
					priv_data->total_tx_ack_bytes,
					priv_data->total_tx_ack_packets, priv_data->total_tx_unmap_cnt);
	}
#else
	cls_eth_print("TX - Packet: pkts(%d) err(map=%d queue=%d)\n",
					priv_data->total_tx_queue_packets,
					priv_data->total_tx_map_fifo_err_cnt,
					priv_data->total_tx_queue_fifo_err_cnt);
	cls_eth_print("TX - Buffer: free(all=%d err=(invalid=%d non_dma=%d) unmap=%d)\n",
					priv_data->total_tx_ack_packets,
					priv_data->total_tx_ack_err_invalid_packets,
					priv_data->total_tx_ack_non_dma_packets,
					priv_data->total_tx_unmap_cnt);

	cls_eth_print("RX - Packet: bytes(%d) pkts(%d) err(tag=%d invalid=%d)\n",
					priv_data->total_rx_bytes,
					priv_data->total_rx_packets,
					priv_data->total_rx_err_cpu_tag,
					priv_data->total_rx_pkt_err_invalid);
	cls_eth_print("RX - Buffer: alloc(all=%d succ=%d) free(all=%d err=%d)\n",
					priv_data->total_alloc_map_rx_buff_cnt,
					priv_data->total_alloc_map_rx_buff_succ,
					priv_data->total_rx_unmap_free_packets,
					priv_data->total_rx_unmap_non_dma_packets);
#endif
}

static void cls_eth_dump_packet(char *info, unsigned char *buf, int length)
{
	int i, remainder, lines;

	cls_eth_print("[%s]: Print packet's first %d bytes\n", info, length);

	lines = length / 16;
	remainder = length % 16;

	for (i = 0; i < lines; i++) {
		int cur;

		for (cur = 0; cur < 8; cur++) {
			unsigned char a, b;

			a = *(buf++);
			b = *(buf++);
			cls_eth_print("%02x%02x ", a, b);
		}
		cls_eth_print("\n");
	}
	for (i = 0; i < remainder / 2; i++) {
		unsigned char a, b;

		a = *(buf++);
		b = *(buf++);
		cls_eth_print("%02x%02x ", a, b);
	}
	cls_eth_print("\n");
}

/*
 * 1.The uboot's rx_queue is based on EDMA's regs - CLS_NPE_TX_QUEUE_STATUS0.
 * 2.The uboot's tx_queue is based on EDMA's regs - CLS_NPE_RX_QUEUE_STATUS.
 */
static u32 cls_eth_get_queue_size(struct udevice *dev, int queue, int ops)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	u32 queue_qsize = 0;

	if (queue == CLS_NPE_GET_RX_QUEUE_SIZE) {
		/* TX_QUEUE_STATUS0
		 *     bit0-11: tx_queue_qsize
		 *     bit13:   tx_status_invalid
		 *     bit14:   tx_pkt_err
		 *     bit15:   is_split
		 *     bit16-31: Packet id
		 */
		queue_qsize = cls_eth_r32(base_addr, CLS_NPE_TX_QUEUE_STATUS0);
	} else {
		/* RX_QUEUE_STATUS
		 *     bit0~11:  rx_queue_qsize
		 *     bit12:    rx_pkt_err
		 *                  1、bus_err
		 *                  2、len=0
		 *     bit16~30: rx_queue_pkt_id
		 *     bit31:    rx_status_invalid
		 */
		queue_qsize = cls_eth_r32(base_addr, CLS_NPE_RX_QUEUE_STATUS);
	}

	return ((ops == ONLY_QUEUE_SIZE) ? (queue_qsize & 0xFFF) : queue_qsize);
}


static void cls_eth_reset_fwd(void)
{
	writel(CLS_NPE_SUB_SOFT_RESET, CLS_NPE_SUB_RST_PARA);
	//Note: This Soft Reset will be completed in 1 us.
	udelay(CLS_NPE_SUB_RESET_HOLD_TIME);
	writel(CLS_NPE_SUB_SOFT_UNSET, CLS_NPE_SUB_RST_PARA);
	udelay(CLS_NPE_SUB_RESET_HOLD_TIME);

	cls_eth_print("Soft Reset FWD: DONE!!!!\n");
}

static void cls_serdes_war(struct cls_eth_priv  *priv_data)
{
	int index;
	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index ++) {
		if (!(priv_data->xgmac[index].flags &
					BIT(HW_ENABLE_BIT)))
			continue;

		cls_xgmac_speed(&priv_data->xgmac[index], SPEED_1000);
	}
}


static void cls_init_npe_hw(struct cls_eth_priv  *priv_data)
{
	int status = 0;
	int xgmac_index = 0;

	printk("Waiting FWD PLL Lock\n");
	status = 0;
	if (readl_poll_timeout(FWD_TOP_PLL_PARA3,
				status, (status & BIT(0)), 10000)) {
		printf("Line %d: wait PLL Lock status[%d] timeout\n", __LINE__, status);
		return;
	}

	/*ADJUST THE CLOCK, Jitter Best*/
#if 0
	writel(0,FWD_TOP_CLK_SEL_PARA);
	writel(0x25010417, FWD_TOP_PLL_PARA1);
	writel(0x1AAAAB, FWD_TOP_PLL_PARA2);
	writel(0x2501041F, FWD_TOP_PLL_PARA1);
	mdelay(1);
#endif

	/*Enable FWD TOP CLK SEL PARA*/
	if (g_debug_cls)
		cls_eth_print("Enable TOP CLK SEL\n");
	writel(0x1, FWD_CLK_PARA_REG);
	mdelay(1);

	writel(0x7, DIV_RST_PARA_REG);

	/*Enable FWD SUB CLK switch */
	if (g_debug_cls)
		cls_eth_print("Enable FWD SUB clock\n");
	writel(0xf, FWD_SUB_CG_PARA_REG);
	mdelay(1);

	/*REST FWD*/
	if (g_debug_cls)
		cls_eth_print("Disable FWD sub\n");
	writel(0x0, FWD_APB_SUB_RST_REG);
	mdelay(1);

	/*De-ASSERT FWD APB*/
	if (g_debug_cls)
		cls_eth_print("Enable FWD apb\n");
	writel(0xA, FWD_APB_SUB_RST_REG);
	mdelay(1);

	//XGMAC1_PHY_INTF_SEL_I
	//0x0 non-RGMII interface(XGMII/GMII/MII)
	//0x1 RGMII interface
	//0x2 RMII  interface
	for (xgmac_index = 0; xgmac_index < 5; xgmac_index++) {

		struct cls_xgmac_priv * port = &priv_data->xgmac[xgmac_index];
		if(!(port->flags & BIT(HW_ENABLE_BIT)))
			continue;

		if (port->interface == XGMAC_INTERFACE_MODE_RGMII) {
			if (g_debug_cls)
				cls_eth_print("Init xgmac[%d] RGMII address=[%#x] en[%#x]\n",
						xgmac_index,
						XGMAC_PHY_INTF_SEL_REG + (xgmac_index * 16),
						XGMAC_PHY_INTF_EN_REG + (xgmac_index * 4)
						);

			writel(0x1, XGMAC_PHY_INTF_SEL_REG + (xgmac_index * 16));
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

			writel(port->rgmii_rx_delay, XGMAC_PHY_RGMII_RX_DELAY(port->id));
			writel(port->rgmii_tx_delay, XGMAC_PHY_RGMII_TX_DELAY(port->id));

		} else {
			if (g_debug_cls)
				cls_eth_print("Init xgmac[%d] non-RGMII address [%#x] interface[%s]\n",
						xgmac_index,
						XGMAC_PHY_INTF_SEL_REG + (xgmac_index * 16),
						phy_string_for_interface(port->phy_interface));

			writel(0x0, XGMAC_PHY_INTF_SEL_REG + (xgmac_index * 16));
			mdelay(1);

			if (xgmac_index == 1) {
				pr_info("XGMAC[%d] switch to XPCS%d\n", port->pcs_sel);
				writel(!port->pcs_sel, XGMAC_TOP_REG_REG + TEN_G_MODE_SEL);
				mdelay(1);
			}
		}
#if 0
		/* Select MDIO*/
		if(xgmac_index < 3 && port->has_mdio &&
				(port->mdio_sel != xgmac_index)) {
			writel(port->mdio_sel,
					XGMAC_TOP_REG_REG + MDIO_SEL_OFFSET(xgmac_index));
   			mdelay(1);
			pr_info("XGMAC[%d] switch to mdio%d\n",
					xgmac_index,
					port->mdio_sel);
		}
#endif
	}

#if 1
	/*ENABLE FWD*/
 if (g_debug_cls)
	cls_eth_print("Enable FWD SUB\n");
	writel(0xF, FWD_APB_SUB_RST_REG);
   	mdelay(1);
#endif
    /*WAR serdes*/
#if 0
	// cls_serdes_war(priv_data);
	printf("Loading Serdes Firmware.....\n");
	/*Set Serdes firmware Loading params*/
	if ((priv_data->xpcs[0].flags &
				BIT(HW_ENABLE_BIT)))
		cls_serdes_load_and_modify(&priv_data->xpcs[0]);

	if ((priv_data->xpcs[1].flags &
				BIT(HW_ENABLE_BIT)))
		cls_serdes_load_and_modify(&priv_data->xpcs[1]);
#endif
}

static inline int cls_interfaces_validate(struct cls_eth_priv *adapter)
{
	int index,pcs_index;
	struct cls_xgmac_priv * port;
	struct cls_xpcs_priv * pcs;

	for (index = 0; index < CLS_NPE_MAX_XGMAC_NUM; index ++) {

		port = &adapter->xgmac[index];

		if(!(port->flags & BIT(HW_ENABLE_BIT)))
			continue;

		pcs_index = port->pcs_sel;

		pcs = &adapter->xpcs[pcs_index];

		if (g_debug_cls)
			printf("%s pcs[%d] xgmac[%d] interface=%d pcs interface=%d\n",
					__func__,
					pcs_index,
				index,
				port->interface,
				pcs->phy_interface
				);

		if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
			if (!(pcs->flags & BIT(HW_ENABLE_BIT))) {
				pr_err("XGMAC%d MUST config XPCS%d DTS!", index, pcs_index);
				return -EINVAL;
			}
			port->phy_interface = pcs->phy_interface;
			/* XGMAC0 USE XPCS0, XGMAC1-3 USE XPCS0/XPCS1, XGMAC4 USE XPCS1*/
			port->pcs_port = pcs_index ? port->id - 1 : port->id;

			if (port->fixed_link) {
				if (pcs->autoneg == 2)
					pcs->autoneg = 0;
			}

			pr_info("Set XGMAC%d PHY interface type [%d], Use PCS%d port%d:autoneg:%d",
					index, port->phy_interface, pcs_index, port->pcs_port, pcs->autoneg);
		} else if (index != 0 && index != 2 && index != 4) /* Only XGMAC0,2,4 can choice RGMII*/ {
			pr_err("XGMAC%d NOT supported RGMII mode!!", index);
			return -EINVAL;
		}

		if (!port->max_speed) {
			switch(port->phy_interface) {
			case PHY_INTERFACE_MODE_2500BASEX:
			case PHY_INTERFACE_MODE_SGMII_2500:
				port->max_speed = SPEED_2500;
				break;
			case PHY_INTERFACE_MODE_10GBASER:
			case PHY_INTERFACE_MODE_USXGMII:
				port->max_speed = SPEED_10000;
				break;
			case PHY_INTERFACE_MODE_SGMII:
			case PHY_INTERFACE_MODE_RMII:
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_RGMII_ID:
			case PHY_INTERFACE_MODE_RGMII_RXID:
			case PHY_INTERFACE_MODE_RGMII_TXID:
			case PHY_INTERFACE_MODE_QSGMII:
			case PHY_INTERFACE_MODE_1000BASEX:
			default:
				port->max_speed = SPEED_1000;
			}
		}
	}

	return 0;
}

#ifdef CONFIG_CLS_PHYLINK
int cls_get_phy_link_map(struct cls_eth_priv *priv_data)
{
	int index, pcs_sel = 0, bitmap = 0, pcs_index = 0;
	struct cls_xgmac_priv * port = NULL;
	struct cls_xpcs_priv * pcs = NULL;

	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index ++) {

		port = &priv_data->xgmac[index];

		if (!(port->flags &
					BIT(HW_ENABLE_BIT)))
			continue;

		if (!(port->flags &
					BIT(FLAGS_PHY_INITD_BIT)))
			continue;

		{
			unsigned int mii_reg;

			if (!port->phydev)
				continue;

			mii_reg = phy_read(port->phydev, MDIO_DEVAD_NONE, MII_BMSR);
			if (mii_reg & BMSR_LSTATUS)
				bitmap |= BIT(index);
		}
	}

	return bitmap;
}

int	cls_wait_local_linkup(struct cls_eth_priv *priv_data, uint32_t timeout_us)
{
	char * tmp;
	int index = 0, is_up = 0, check_bitmap = 0, link_bitmap = 0;

	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index ++) {
		if (!(priv_data->xgmac[index].flags &
					BIT(HW_ENABLE_BIT)))
			continue;

		if (!(priv_data->xgmac[index].flags &
					BIT(FLAGS_PHY_INITD_BIT)))
			continue;

		if (!priv_data->xgmac[index].fixed_link &&
				!priv_data->xgmac[index].phydev)
			continue;

		check_bitmap |= BIT(index);
	}

	tmp = env_get("portmask");
	if(tmp) {
		int portmask = simple_strtoul(tmp , NULL, 16);
		if(portmask) check_bitmap &= portmask;
	}

	readx_poll_timeout(cls_get_phy_link_map,
			priv_data,
			link_bitmap,
			(check_bitmap == (check_bitmap & link_bitmap)),
			timeout_us);

	return check_bitmap & link_bitmap;
}
#endif

void fix_rgmii_war(int index)
{
	writel(1, XGMAC_TOP_REG_REG + 0x200 + (index * 8));
}

void set_rgmii_speed100(struct phy_device *phydev)
{
#if 0
#define msleep(a)			udelay(a * 1000)
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 2);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x15, 0x2030);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x4, 0xde0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x9, 0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0, 0x9100);
	msleep(500);
#else
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0xd08);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x15, 0x8);
#endif
}

#ifdef CONFIG_CLS_YTSWITCH
int yt_switch_init_1st = -1;
#endif

static int cls_eth_start(struct udevice *dev)
{
	int index = 0, ret, link_map;
	struct cls_eth_priv *priv_data = dev_get_priv(dev);

	ret = cls_interfaces_validate(priv_data);
	if (ret) {
		printf("please check xgmac xpcs config in DTS\n");
		return ret;
	}

	/* INIT TOP REG */
	if (!priv_data->initd) {
		if (g_debug_cls)
			printf("===>cls_init_npe_hw !!!\n");
		cls_init_npe_hw(priv_data);
#if 0
		priv_data->initd = true;
#endif
	}


	if (g_debug_cls)
		printf("INIT PHY !!!\n");

	/* foreach init phy	*/
	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index ++)  {

		if (!(priv_data->xgmac[index].flags &
					BIT(HW_ENABLE_BIT)))
			continue;

		if (priv_data->xgmac[index].flags &
				BIT(FLAGS_PHY_INITD_BIT))
			continue;

		if (g_debug_cls)
			printf("cls_phy_init xgmac%d phy_addr[%#x] type[%s] fixed_link[%d] !!!\n",
				index,
				priv_data->xgmac[index].phy_addr,
				phy_string_for_interface(priv_data->xgmac[index].phy_interface),
				priv_data->xgmac[index].fixed_link);

#ifdef CONFIG_CLS_PHYLINK
		if (!priv_data->xgmac[index].phy_handle.np) {
			priv_data->xgmac[index].flags |= BIT(FLAGS_PHY_INITD_BIT);
			continue;
		}

		ret = cls_phy_init(&priv_data->xgmac[index], dev);
		if (!ret) {
#if 0
			/* Just For Test in FPGA and EVB-A RGMII */
			if (priv_data->xgmac[index].interface == XGMAC_INTERFACE_MODE_RGMII) {
				printf("XGMAC%d set phy SPEED to 100M\n");
				set_rgmii_speed100(priv_data->xgmac[index].phydev);
			}
#endif
			priv_data->xgmac[index].flags |= BIT(FLAGS_PHY_INITD_BIT);
		}
#endif
	}

	/* foreach init xpcs */
	for (index = 0;
			index < CLS_NPE_MAX_XPCS_NUM;
			index ++) {
		if (!(priv_data->xpcs[index].flags &
					BIT(HW_ENABLE_BIT)))
			continue;

		if ((priv_data->xpcs[index].flags &
					BIT(HW_INITD_BIT)))
			continue;

		if (g_debug_cls)
			printf("cls_xpcs_config xpcs%d !!!\n", index);

		ret = cls_xpcs_config(&priv_data->xpcs[index]);
#if 0
		if (!ret) {
			priv_data->xpcs[index].flags |= BIT(HW_INITD_BIT);
		}
#endif
	}

#ifdef CONFIG_CLS_PHYLINK
#ifdef CONFIG_CLS_YTSWITCH
	/*only init yt switch once*/
	if (!g_skip_extswitch && yt_switch_init_1st) {
		printf("only init yt switch once\n");
		yt_switch_init_1st = cls_yt_switch_init();
	}
#endif
#ifdef CONFIG_CLS_RTKSDK
	/* config rtk phy by sdk */
	rtlsdk_phy_init();
#endif
	/* wait phy link up */
	link_map = cls_wait_local_linkup(priv_data, LINK_TIMEOUT_US);
	if(!link_map && !g_skip_check_phy) {
		printf("no found any phy link up !!!");
		return -1;
	}

	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index++) {

		if (!(BIT(index) & link_map))
			continue;

		phy_startup(priv_data->xgmac[index].phydev);
		printf("MAC%d phy has link up !!!\n", index);

		if (priv_data->xgmac[index].interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
			int speed;
			int pcs_index = 0, phy_interface = 0;

			pcs_index = priv_data->xgmac[index].pcs_sel;

			speed = priv_data->xgmac[index].phydev->speed;
			printf(">>>>>Set XGMAC%d speed %d<<<<<<\n", index, speed);

			if (priv_data->xgmac[index].phydev &&
					(priv_data->xgmac[index].phy_interface == PHY_INTERFACE_MODE_SGMII_2500 ||
						priv_data->xgmac[index].phy_interface == PHY_INTERFACE_MODE_SGMII)
			   ) {

				if (priv_data->xgmac[index].phydev->speed == SPEED_2500) {
					phy_interface = PHY_INTERFACE_MODE_SGMII_2500;
				} else {
					phy_interface = PHY_INTERFACE_MODE_SGMII;
				}

				if (phy_interface != priv_data->xpcs[pcs_index].phy_interface) {
					priv_data->xpcs[pcs_index].phy_interface = phy_interface;
					priv_data->xgmac[index].phy_interface = phy_interface;
					cls_xpcs_config(&priv_data->xpcs[pcs_index]);
				}
			}

			cls_xpcs_speed(&priv_data->xpcs[pcs_index], speed);
		}
	}
#endif

	cls_eth_edma_init(dev);
	if (cls_eth_soft_bmu_init(dev)) {
		cls_eth_print("[%s] soft_bmu_init failed!\n", __func__);
		cls_eth_stop(dev);
		return -1;
	}

	if (!g_skip_switch)
		cls_eth_switch_init(dev);
	//cls_eth_mac_pcs_config(dev, 1);

	//set mac rate
	//foreach xgmac
		for (index = 0;
				index < CLS_NPE_MAX_XGMAC_NUM;
				index ++) {
			if (!(priv_data->xgmac[index].flags &
						BIT(HW_ENABLE_BIT)))
				continue;

			if (!(priv_data->xgmac[index].flags &
						BIT(FLAGS_PHY_INITD_BIT)))
				continue;

			if (priv_data->xgmac[index].phydev->link) {

				if (g_debug_cls)
					printf("===>cls_set_mac_up xgmac%d up!!!\n", index);
#if 0
				if (priv_data->xgmac[index].phydev &&
						priv_data->xgmac[index].phydev->phy_id == 0x1cc916) { //RTL8211F
					fix_rgmii_war(index);
				}
#endif

				cls_xgmac_init(&priv_data->xgmac[index]);
				cls_xgmac_speed(&priv_data->xgmac[index],
						priv_data->xgmac[index].phydev->speed);
				cls_enable_xgmac(&priv_data->xgmac[index], true);
			}
		}

	return 0;
}

static void cls_eth_clean(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_tx_ring *tx_ring = &priv_data->tx_ring;
	struct cls_rx_ring *rx_ring = &priv_data->rx_ring;

	/* clean all tx_queue */
	if (cls_eth_get_queue_size(dev, CLS_NPE_GET_TX_QUEUE_SIZE, ONLY_QUEUE_SIZE) > 0)
		cls_clean_sw_bmu_tx_irq(dev);

	if (tx_ring->next_to_use != tx_ring->next_to_clean)
		cls_eth_print("[%s] ERR tx ring buffer: next_use=%d next_clean=%d\n", __func__,
				tx_ring->next_to_use, tx_ring->next_to_clean);

	/* clean all rx_queue */
	if (cls_eth_get_queue_size(dev, CLS_NPE_GET_RX_QUEUE_SIZE, ONLY_QUEUE_SIZE) > 0) {
		while (cls_eth_free_rx_pkt(dev, 0, 0) == 0)
			priv_data->total_rx_clean_packets += 1;
	}

	if (CLS_NPE_DESC_UNUSED(rx_ring))
		cls_eth_print("[%s] ERR rx ring buffer: next_use=%d next_clean=%d\n", __func__,
				rx_ring->next_to_use, rx_ring->next_to_clean);
}

static void cls_eth_stop(struct udevice *dev)
{
	int index;
	//cls_eth_mac_pcs_config(dev, 0);
	struct cls_eth_priv *priv_data = dev_get_priv(dev);

	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index ++) {
		if (!(priv_data->xgmac[index].flags
					& BIT(HW_ENABLE_BIT)))
			continue;

		cls_enable_xgmac(&priv_data->xgmac[index], false);
	}

	cls_eth_clean(dev);

	cls_eth_free_bmu(dev);
#if 0
	cls_eth_reset_fwd();
#endif

	cls_eth_dump_counter(dev);
	cls_eth_reset_counter(dev);
}

/* Configure the tx ring buffer
 * 1. mapping the tx-buff to DMA
 * 2. configure rx_description1
 * Return the number of buffer_info which have been mapped,
 * Note: for the most part, this function will return 1
 */
static int cls_tx_map(struct udevice *dev,
			struct cls_tx_ring *tx_ring,
			void *packet, unsigned int first,
			unsigned int max_per_txd, unsigned int len)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	struct cls_tx_buffer *buffer_info;
	unsigned int offset = 0, size, count = 0, i;
	static u16 pkt_id;
	s32 val = 0;
	s32 rx_fifo_remain_size = 0;

	i = tx_ring->next_to_use;

	while (len) {
		buffer_info = &tx_ring->buffer_info[i];
		size = min(len, max_per_txd);
		buffer_info->length = size;

		rx_fifo_remain_size = cls_eth_r32(base_addr, CLS_NPE_RX_DESCR_FIFO_STATUS);
		if (rx_fifo_remain_size == (priv_data->lhost_fifo_depth * CLS_NPE_FIFO_DEPTH_LEVEL)) {
			priv_data->total_tx_map_fifo_err_cnt += 1;
			cls_eth_print("[%s] invalid tx_fifo_size: depth=%d, count=%d i=%d next_use=%d\n",
					__func__, priv_data->lhost_fifo_depth, count, i, tx_ring->next_to_use);
			break;
		}

		/* Note: The content of this dma will not include cpu_tag field */
		buffer_info->dma = dma_map_single(packet + offset, size, DMA_TO_DEVICE);
		if (dma_mapping_error(packet, buffer_info->dma)) {
			priv_data->total_tx_dma_map_err_cnt += 1;
			goto dma_error;
		}

		/* pkt_id is used to debug the throughtput. */
		if (pkt_id == 0x7FFF)
			pkt_id = 0;

		val = size;// packet-len
		//val |= (0 << TAG_IND_BIT);// without From CPU Tag
		val |= (pkt_id << RX_PKT_ID_BIT);// RX packet ID
		val |= (1 << RX_DESCR_CFG_EN);

		buffer_info->rx_description1 = val;

		priv_data->total_tx_bytes += size;
		priv_data->total_tx_packets += 1;

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] size=%d pkt_id=%d len=%d count=%d offset=%d i=%d\n",
					__func__, size, pkt_id, len, count, offset, i);
#endif
		pkt_id++;
		len -= size;
		offset += size;
		count++;
		if (len) {
			i++;
			if (unlikely(i == tx_ring->count))
				i = 0;
		}
	}

	return count;
dma_error:
	/* dma failed! we should recover it */
	cls_eth_print("TX DMA map failed\n");
	buffer_info->length = 0;
	buffer_info->dma = 0;

	while (count--) {
		if (i == 0)
			i = tx_ring->count;
		i--;
		buffer_info = &tx_ring->buffer_info[i];
		cls_unmap_tx_resource(dev, buffer_info);
		memset(buffer_info, 0, sizeof(struct cls_tx_buffer));
	}

	return 0;
}

// write dma and rx_description1 to edma RX_DESCRIPTION0/RX_DESCRIPTION1
// Note: For the most part, the 'count'is 1.
static int cls_tx_queue(struct udevice *dev,
			   struct cls_tx_ring *tx_ring, int count)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	struct cls_tx_buffer *buffer_info;
	unsigned int i;
	s32 rx_fifo_remain_size = 0;
	int enqueue = 0;

	i = tx_ring->next_to_use;

	while (count--) {
		buffer_info = &tx_ring->buffer_info[i];

		rx_fifo_remain_size = cls_eth_r32(base_addr, CLS_NPE_RX_DESCR_FIFO_STATUS);
		if (rx_fifo_remain_size == (priv_data->lhost_fifo_depth * CLS_NPE_FIFO_DEPTH_LEVEL)) {
			priv_data->total_tx_queue_fifo_err_cnt += 1;
			cls_eth_print("[%s] invalid tx_fifo_size: depth=%d, count=%d i=%d next_use=%d\n",
					__func__, priv_data->lhost_fifo_depth, count, i, tx_ring->next_to_use);
			break;
		}

		cls_eth_w32(base_addr, CLS_NPE_RX_DESCRIPTION0, buffer_info->dma);
		cls_eth_w32(base_addr, CLS_NPE_RX_DESCRIPTION1, buffer_info->rx_description1);
		enqueue += 1;

		priv_data->total_tx_queue_packets += 1;

		if (unlikely(++i == tx_ring->count))
			i = 0;

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] Send packet to queue: count=%d i=%d\n",
				__func__, count, i);
#endif
	}

	dmb();

	tx_ring->next_to_use = i;

	return enqueue;
}

/* This function is based on cls_xmit_frame() */
static void cls_xmit_frame(struct udevice *dev, void *packet, int length)
{

	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_tx_ring *tx_ring;
	unsigned int first;
	int enqueue, count = 0;

	/* pad packets smaller than ETH_ZLEN */
	if (length < ETH_ZLEN) {
#ifdef CLS_NPE_PADDING_ENABLE
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] packet length have been padded from %d to %d\n",
					__func__, length, ETH_ZLEN);
#endif
		memset(packet + length, 0, ETH_ZLEN - length);
		length = ETH_ZLEN;
#else
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] packet length(%d) is less than %d\n",
					__func__, length, ETH_ZLEN);
#endif
#endif
	}

	tx_ring = &priv_data->tx_ring;
	first = tx_ring->next_to_use;
	count = cls_tx_map(dev, tx_ring, packet, first, CLS_NPE_MAX_DATA_PER_TXD, length);

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("[%s] after tx_map: first=%d length=%d count=%d\n",
				__func__, first, length, count);
#endif

	if (count) {
		enqueue= cls_tx_queue(dev, tx_ring, count);
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		if (enqueue)
			cls_eth_dump_packet("Transmited packet",
						packet, length > ETH_ZLEN ? ETH_ZLEN : length);
#endif
		if (count != enqueue)
			cls_eth_print("[%s] MISS Packet: packet %d, enqueue %d\n",
						__func__, count, enqueue);
	} else {
		cls_eth_print("[%s] No packet Send\n", __func__);
		tx_ring->next_to_use = first;
	}
}

/* This function is based on cls_clean_sw_bmu_tx_irq() */
// clean tx queue
static void cls_clean_sw_bmu_tx_irq(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	struct cls_tx_buffer *buffer_info;
	struct cls_tx_ring *tx_ring;
	u32 val = 0;
	u32 rx_queue_qsize = 0, rx_pkt_id = 0;
	u8 rx_pkt_err = 0, rx_status_invalid = 0;
	int i = 0, maxtry = CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_MAXTRY;

	do {
		rx_queue_qsize = cls_eth_get_queue_size(dev, CLS_NPE_GET_TX_QUEUE_SIZE, ALL_QUEUE_STATUS);
		rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
		rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
		rx_status_invalid = ((rx_queue_qsize >> RX_STATUS_INVALID) & 0x1);
		rx_queue_qsize &= 0xFFF;
#ifdef CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_ENABLE
		udelay(CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_PERIOD);
		maxtry--;
#else
		maxtry = 0;
#endif
	} while (rx_queue_qsize <= 0 && maxtry);

	if (rx_queue_qsize <= 0) {
		cls_eth_print("[%s] Empty tx_queue to free!\n", __func__);
		return;
	}

	tx_ring = &priv_data->tx_ring;
	i = tx_ring->next_to_clean;
	while (rx_queue_qsize > 0) {
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] will clean tx ring_buffer[%d], queue_size=%d pktid=%d\n",
				__func__, i, rx_queue_qsize, rx_pkt_id);
#endif
		/* RX_QUEUE_DEL_INSTR
		 * bit0~11: rx_queue_del_num, bit12 W1P: rx_queue_del_en
		 */
		val = ((1 << 12) | 1);
		cls_eth_w32(base_addr, CLS_NPE_RX_QUEUE_DEL_INSTR, val);

		if (rx_pkt_err || rx_status_invalid) {
			priv_data->total_tx_ack_err_invalid_packets += 1;
			cls_eth_print("[%s] ERR! rx_pkt_err: i=%d queue_size=%d pktid=%d pkterr=%d invalid=%d next_clean=%d next_use=%d\n",
					__func__,
					i, rx_queue_qsize, rx_pkt_id, rx_pkt_err, rx_status_invalid,
					tx_ring->next_to_clean, tx_ring->next_to_use);
		}

		buffer_info = &tx_ring->buffer_info[i];
		priv_data->total_tx_ack_bytes += buffer_info->length;

		if (buffer_info->dma)
			cls_unmap_tx_resource(dev, buffer_info);
		else {
			buffer_info->length = 0;
			buffer_info->rx_description1 = 0;
			priv_data->total_tx_ack_non_dma_packets += 1;
			cls_eth_print("[%s] ERR! cannot clean unmap dma\n", __func__);
		}

		memset(buffer_info, 0, sizeof(struct cls_tx_buffer));

		if (unlikely(++i == tx_ring->count))
			i = 0;

		tx_ring->next_to_clean = i;

#ifdef CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_ENABLE
		udelay(CLS_NPE_CLEAN_TX_RING_BUFFER_DELAY_PERIOD);
#endif

		rx_queue_qsize = cls_eth_get_queue_size(dev, CLS_NPE_GET_TX_QUEUE_SIZE, ALL_QUEUE_STATUS);
		rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
		rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
		rx_status_invalid = ((rx_queue_qsize >> RX_STATUS_INVALID) & 0x1);
		rx_queue_qsize &= 0xFFF;

		priv_data->total_tx_ack_packets += 1;
	}
}

// This function will be called by net_loop(). Only a packet will by handled once.
static int cls_eth_send(struct udevice *dev, void *packet, int length)
{
	cls_xmit_frame(dev, packet, length);
	cls_clean_sw_bmu_tx_irq(dev);

	return 0;
}

// This function will be called to free a packet(rx buffer).
// 1. unmap and free a rx buffer_info->data
// 2. alloc new one and map it
static int cls_eth_free_rx_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	void __iomem *base_addr = priv_data->edma_regs;
	struct cls_rx_ring *rx_ring = &priv_data->rx_ring;
	struct cls_rx_buffer *buffer_info;
	unsigned int i;
	int cleaned_count = 0;
	u32 tx_queue_qsize = 0;

	tx_queue_qsize = cls_eth_get_queue_size(dev, CLS_NPE_GET_RX_QUEUE_SIZE, ONLY_QUEUE_SIZE);
	if (tx_queue_qsize <= 0) {
#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("Empty rx_queue to free!\n");
#endif
		return -1;
	}

	i = rx_ring->next_to_clean;
	buffer_info = &rx_ring->buffer_info[i];

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("[%s]: queue_size=%d next_to_clean=%d\n", __func__, tx_queue_qsize, i);
#endif

	if (buffer_info->dma)
		dma_unmap_single(buffer_info->dma,
					 ALIGN(priv_data->rx_buffer_len + CLS_NPE_TO_CPU_TAG_HLEN, 64),
					 DMA_FROM_DEVICE);
	else {
		cls_eth_print("[%s] ERR! cannot clean unmap dma\n", __func__);
		priv_data->total_rx_unmap_non_dma_packets += 1;
	}

	buffer_info->dma = 0;
	free(buffer_info->data);
	buffer_info->data = NULL;
	priv_data->total_rx_unmap_free_packets += 1;

	cls_eth_w32(base_addr, CLS_NPE_TX_QUEUE_DEL_INSTR, 1);

	if (++i == rx_ring->count)
		i = 0;

	rx_ring->next_to_clean = i;

	// re-attach the rx-buf here
	cleaned_count = CLS_NPE_DESC_UNUSED(rx_ring);
	if (cleaned_count)
		priv_data->alloc_rx_buf(dev, rx_ring, cleaned_count);

	priv_data->total_rx_unmap_free_cnt += 1;

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("[%s]: cleaned=%d  unused=%d next_clean=%d\n",
					__func__, cleaned_count, CLS_NPE_DESC_UNUSED(rx_ring),
					rx_ring->next_to_clean);
#else
	if (cleaned_count != 1 || CLS_NPE_DESC_UNUSED(rx_ring))
		cls_eth_print("[%s]: ERR! ring buffer may crash! cleaned=%d unused=%d\n",
					__func__, cleaned_count, CLS_NPE_DESC_UNUSED(rx_ring));
#endif

	return 0;
}

// This function will be called by eth_rx(). Only a packet will by handled once.
// 1. get the packet from the dma (TX_QUEUE_STATUS0)
// 2. parse it and get the packet and length
/* This function is based on cls_clean_sw_bmu_rx_irq() */
static int cls_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_rx_ring *rx_ring = &priv_data->rx_ring;
	struct cls_rx_buffer *buffer_info;
	u32 length = 0;
	unsigned int i;
	u32 tx_queue_qsize = 0;
	struct cls_to_cpu_tag to_cpu_tag;
	int pkt_id;
	u8 tx_status_invalid = 0, tx_pkt_err = 0, is_split = 0;

	tx_queue_qsize = cls_eth_get_queue_size(dev, CLS_NPE_GET_RX_QUEUE_SIZE, ALL_QUEUE_STATUS);
	pkt_id = ((tx_queue_qsize & 0xFFFF0000) >> TX_PACKET_ID);
	is_split = ((tx_queue_qsize & 0x8000) >> IS_SPLIT_BIT);
	tx_pkt_err = ((tx_queue_qsize & 0x4000) >> TX_PKT_ERR_BIT);
	tx_status_invalid = ((tx_queue_qsize & 0x2000) >> TX_STATUS_INVALID_BIT);
	tx_queue_qsize = (tx_queue_qsize & 0xFFF);

	if (tx_queue_qsize <= 0) {
		priv_data->total_rx_empty_queue_cnt += 1;
		return -1;
	}

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_print("[%s] queue_size=%d pkt_id=%d is_split=%d next_clean=%d\n",
				__func__, tx_queue_qsize, pkt_id, is_split, rx_ring->next_to_clean);
#endif

	i = rx_ring->next_to_clean;
	buffer_info = &rx_ring->buffer_info[i];

	if (tx_pkt_err || tx_status_invalid || is_split) {
		priv_data->total_rx_pkt_err_invalid += 1;
		cls_eth_print("[%s]: PKT_ER: queue_qsize(%d) (pktid=%d) is_split(%d) err(%d) invalid(%d) next_clean(%d) next_use(%d)\n",
					__func__, tx_queue_qsize, pkt_id, is_split, tx_pkt_err, tx_status_invalid,
					rx_ring->next_to_clean, rx_ring->next_to_use);
	}

	dmb();

	if (cls_to_cpu_tag_parse(dev, &to_cpu_tag, buffer_info->data)) {
		cls_eth_print("[%s] invalid cpu_tag，pktid=%d data=0x%p\n", __func__, pkt_id, buffer_info->data);
		cls_eth_dump_packet("Received invalid tocpu_tag", buffer_info->data, CLS_NPE_TO_CPU_TAG_HLEN + ETH_ZLEN);
		// free invalid rx buffer_info
		cls_eth_free_rx_pkt(dev, 0, 0);
		priv_data->total_rx_err_cpu_tag += 1;

		return -1;
	}

	length = to_cpu_tag.pakcet_length;
	// packetp point to DMAC, should move the 24-byte(CPU Tag)
	*packetp = buffer_info->data + CLS_NPE_TO_CPU_TAG_HLEN;

	priv_data->total_rx_packets += 1;
	priv_data->total_rx_bytes += length;

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
	cls_eth_dump_packet("Received packet", buffer_info->data,
					CLS_NPE_TO_CPU_TAG_HLEN + (length > ETH_ZLEN ? ETH_ZLEN : length));
#endif

	return length;
}

void __iomem *conf_base_addr = NULL;
void __iomem *debug_base_addr = NULL;

static int cls_mac_create(struct cls_eth_priv *adapter, ofnode np)
{
	int err, id;
	ofnode child_np;
	const char *str;
	struct ofnode_phandle_args phandle;
	const __be32 *_id = ofnode_get_property(np, "id", NULL);
	struct cls_xgmac_priv *port = NULL;

	if (!_id) {
		cls_eth_print("missing mac id\n");
		return -EINVAL;
	}

	id = be32_to_cpup(_id);
	if (id >= CLS_NPE_MAX_XGMAC_NUM) {
		cls_eth_print("%d is not a valid mac id\n", id);
		return -EINVAL;
	}

	if (adapter->xgmac[id].flags & BIT(HW_ENABLE_BIT)) {
		cls_eth_print("duplicate mac id found: %d\n", id);
		return 0;
	}

	port = &adapter->xgmac[id];
    port->id = id;

	/*Default*/
	port->interface = XGMAC_INTERFACE_MODE_NON_RGMII;

	str = ofnode_read_string(np, "mode");
	if (str) {
		if (strcasecmp("RGMII", str) == 0)
			port->interface = XGMAC_INTERFACE_MODE_RGMII;
		else if (strcasecmp("NON-RGMII", str) == 0)
			port->interface = XGMAC_INTERFACE_MODE_NON_RGMII;
		else {
			pr_err("Can't get mac interface mode\n");
			err = -EINVAL;
			goto err_register_netdev;
		}
	}

	pr_info("XGMAC%d mode [%s]\n", id,
			port->interface == XGMAC_INTERFACE_MODE_RGMII ? "RGMII" : "NON-RGMII");

	if (port->interface == XGMAC_INTERFACE_MODE_RGMII) {
		int rx_delay = 0;
		int tx_delay = 0;

		err = ofnode_read_u32(np, "rgmii-rx-delay", &port->rgmii_rx_delay);
		if(err)
			port->rgmii_rx_delay = 2;

		err = ofnode_read_u32(np, "rgmii-tx-delay", &port->rgmii_tx_delay);
		if(err)
			port->rgmii_tx_delay = 2;

		pr_err("set rgmii rx_delay [%d] tx_delay [%d]\n",
				port->rgmii_rx_delay,
				port->rgmii_tx_delay);
#if 0
		if (tx_delay && rx_delay)
			port->phy_interface = PHY_INTERFACE_MODE_RGMII_ID;
		else if (tx_delay)
			port->phy_interface = PHY_INTERFACE_MODE_RGMII_TXID;
		else if (rx_delay)
			port->phy_interface = PHY_INTERFACE_MODE_RGMII_RXID;
		else
			port->phy_interface = PHY_INTERFACE_MODE_RGMII;
#endif

	}
	else
		port->phy_interface = PHY_INTERFACE_MODE_NONE;

	if ( port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
		int pcs_sel = 0;
		switch (id) {
		case 1:
		case 2:
		case 3:
			err = ofnode_read_u32(np, "pcs-sel", &pcs_sel);
			if(err) {
				pcs_sel = 1;
				if (g_debug_cls)
					pr_err("Can't get pcs_sel,Set pcs_sel [%d]\n", pcs_sel);
			}
			break;
		case 0:
			pcs_sel = 0;
			break;
		case 4:
			pcs_sel = 1;
			break;
		}

		port->pcs_sel = pcs_sel;
		port->xpcs = &adapter->xpcs[port->pcs_sel];
		pr_info("XGMAC%d set pcs_sel [%d]\n", id, port->pcs_sel);
	}

	/* MDIO SEL */
	port->mdio_sel = id;
	if(id < 3) {
		err = ofnode_read_u32(np, "mdio-sel", &port->mdio_sel);
		if(err) {
			if (g_debug_cls)
				cls_eth_print("Can't get mdio_sel,Set mdio_sel [%d]\n", port->mdio_sel);
		}
	}

	cls_eth_print("XGMAC%d set mdio_sel [%d]\n",id,  port->mdio_sel);

	port->max_speed = 0;
	ofnode_read_u32(np, "max-speed", &port->max_speed);

	err = ofnode_parse_phandle_with_args(np, "phy-handle", NULL, 0, 0,
			&phandle);
	if (!err) {
		/*Get phy reset level*/
		port->reset = -1;
		if(ofnode_read_bool(np, "reset-active-low")) {
			cls_eth_print( "is reset-active-low\n");
			port->reset = 0;
		}

		if(ofnode_read_bool(np, "reset-active-high")) {
			cls_eth_print("is reset-active-high\n");
			port->reset = 1;
		}

		/* Get phy address on mdio bus */
		err = ofnode_read_u32(phandle.node, "reg", &port->phy_addr);
		if (err) {
			printf("No found xgmac%d phy_addr\n", id);
			return -ENOMEM;
		}

		/* Get mdio_flags */
		port->mdio_flags = 0;
		ofnode_read_u32(phandle.node, "mdio-flags", &port->mdio_flags);

		cls_eth_print("mdio_flags = %d\n", port->mdio_flags);

		if (g_debug_cls)
			printf("Found xgmac%d phy_addr=%d\n", id, port->phy_addr);

		port->phy_handle = phandle.node;
	} else if (!g_skip_extswitch) {
		port->fixed_link = ofnode_phy_is_fixed_link(np, NULL);
		printf("XGMAC%d is fixed_link[%d] \n",id, port->fixed_link);
		if (port->fixed_link)
			port->phy_handle = np;
	}

	port->flags = BIT(HW_ENABLE_BIT);

	cls_eth_print("[%s] XGMAC%d create Successfully\n", __func__, id);

err_register_netdev:
	return;
}

static int cls_pcs_create(struct cls_eth_priv *adapter, ofnode np)
{
	int err, id;
	phy_interface_t phy_mode;
	struct cls_xpcs_priv * pcs = NULL;
	const __be32 *_id = ofnode_get_property(np, "id", NULL);

	if (!_id) {
		cls_eth_print( "missing mac id\n");
		return -EINVAL;
	}

	id = be32_to_cpup(_id);
	if (id >= CLS_NPE_MAX_XPCS_NUM) {
		cls_eth_print("%d is not a valid xpcs id\n", id);
		return -EINVAL;
	}

	if (adapter->xpcs[id].flags & BIT(HW_ENABLE_BIT)) {
		cls_eth_print("duplicate pcs id found: %d\n", id);
		return 0;
	}
    pcs = &adapter->xpcs[id];

	err = ofnode_get_phy_mode(np, &phy_mode);
	if (err && err != -ENODEV) {
		cls_eth_print("Can't get phy-mode\n");
		err = ERR_PTR(phy_mode);
		goto err;
	}

	pcs->phy_interface = phy_mode;

	if (g_debug_cls)
		printf("PCS[%d] get phy_interface = %d\n",id, phy_mode);

	if (phy_mode == PHY_INTERFACE_MODE_USXGMII) {
		err = ofnode_read_u32(np, "usxg_mode", &pcs->usxg_mode);
		if(err) {
			pcs->usxg_mode = 0;
		}

		cls_eth_print("Set usxg_mode[%d]\n", pcs->usxg_mode);
	}

	err = ofnode_read_u32(np, "auto-negotiation", &pcs->autoneg);
	if(err)
		pcs->autoneg = 2; /*Default Enable*/

	cls_eth_print("Set pcs autoneg [%s]\n", pcs->autoneg ? "true": "false");

	cls_eth_print("Get PCS[%d] ioaddr form DTS ioaddr  %#llx\n",id, pcs->ioaddr);

    pcs->flags = BIT(HW_ENABLE_BIT);

	return 0;

err:
out:
	return err;
}

static uint32_t try_get_hw_version()
{
	return readl(BOARD_HW_VER_ADDR);
}

ofnode try_get_device_ofnode_by_version(struct udevice *dev, u32 hw_version)
{
	ofnode child_np;

    char version[16] = {0};

	snprintf(version, sizeof(version), "hw@%x", hw_version);

	ofnode_for_each_subnode(child_np, dev_ofnode(dev)) {
		const char * name = ofnode_get_name(child_np);

		if (g_debug_cls)
			cls_eth_print("Node name [%s], version [%s]\n", name ? name: "", version);

		if (!name || strcasecmp(name, version))
			continue;

		return child_np;
	}

	return dev_ofnode(dev);
}

static int cls_eth_probe(struct udevice *dev)
{
	int index,err;
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	ofnode child_np, dev_np;
	uint32_t  hw_version;
	phys_addr_t iobase;
	const char *version_str;

	for (index = 0;
			index < CLS_NPE_MAX_XGMAC_NUM;
			index ++) {
		priv_data->xgmac[index].adapter = priv_data;
	}

	/* Those params are based on the clourneysemi-cls-rdb.dts */
	iobase = dev_read_addr_size_index(dev, 0, &priv_data->edma_regs_size);
	priv_data->edma_regs = ioremap(iobase, priv_data->edma_regs_size);

	iobase = dev_read_addr_size_index(dev, 1, &priv_data->edma_dbg_regs_size);
	priv_data->edma_dbg_regs = ioremap(iobase, priv_data->edma_dbg_regs_size);

	iobase = dev_read_addr_size_index(dev, 2, &priv_data->xpcs[0].regs_size);
	priv_data->xpcs[0].ioaddr = ioremap(iobase, priv_data->xpcs[0].regs_size);

	iobase = dev_read_addr_size_index(dev, 3, &priv_data->xpcs[1].regs_size);
	priv_data->xpcs[1].ioaddr = ioremap(iobase, priv_data->xpcs[1].regs_size);

	iobase = dev_read_addr_size_index(dev, 4, &priv_data->switch_regs_size);
	priv_data->switch_regs = ioremap(iobase, priv_data->switch_regs_size);

	iobase = dev_read_addr_size_index(dev, 5, &priv_data->xgmac[2].regs_size);
	priv_data->xgmac[2].ioaddr = ioremap(iobase, priv_data->xgmac[2].regs_size);

	iobase = dev_read_addr_size_index(dev, 6, &priv_data->xgmac[3].regs_size);
	priv_data->xgmac[3].ioaddr = ioremap(iobase, priv_data->xgmac[3].regs_size);

	iobase = dev_read_addr_size_index(dev, 7, &priv_data->xgmac[4].regs_size);
	priv_data->xgmac[4].ioaddr = ioremap(iobase, priv_data->xgmac[4].regs_size);

	iobase = dev_read_addr_size_index(dev, 8, &priv_data->xgmac[0].regs_size);
	priv_data->xgmac[0].ioaddr = ioremap(iobase, priv_data->xgmac[0].regs_size);

	iobase = dev_read_addr_size_index(dev, 9, &priv_data->xgmac[1].regs_size);
	priv_data->xgmac[1].ioaddr = ioremap(iobase, priv_data->xgmac[1].regs_size);

	priv_data->xgmac[0].xpcs = &priv_data->xpcs[0];
	priv_data->xgmac[1].xpcs = &priv_data->xpcs[1];
	priv_data->xgmac[2].xpcs = &priv_data->xpcs[1];
	priv_data->xgmac[3].xpcs = &priv_data->xpcs[1];
	priv_data->xgmac[4].xpcs = &priv_data->xpcs[1];

	priv_data->xpcs[0].id = 0;
	priv_data->xpcs[1].id = 1;

	priv_data->soft_bmu_en = dev_read_u32_default(dev, "soft-bmu", 1);
	priv_data->token_fc_en = dev_read_u32_default(dev, "qos", 0);
	priv_data->split_mode = dev_read_u32_default(dev, "split-mode", 0);
	priv_data->lhost_fifo_depth = dev_read_u32_index_default(dev, "fifo-depth", 0, 8);


    debug_base_addr = priv_data->edma_dbg_regs;
    conf_base_addr = priv_data->edma_regs;

	memcpy(priv_data->base_mac, pdata->enetaddr, ETH_ALEN);
	priv_data->udev = dev;

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls) {
	cls_eth_print("[%s] edma_regs=(0x%llx), size=(0x%llx)\n", __func__,
				(phys_addr_t)priv_data->edma_regs, priv_data->edma_regs_size);
	cls_eth_print("[%s] switch_regs=(0x%llx),size =(0x%llx)\n", __func__,
				(phys_addr_t)priv_data->switch_regs, priv_data->switch_regs_size);
	cls_eth_print("[%s] soft_bmu_en %d\n", __func__, priv_data->soft_bmu_en);
	cls_eth_print("[%s] token-en %d\n", __func__, priv_data->token_fc_en);
	cls_eth_print("[%s] split-mode %d\n", __func__, priv_data->split_mode);
	cls_eth_print("[%s] lhost fifo depth (%d * 16)\n", __func__,
				priv_data->lhost_fifo_depth);
	cls_eth_print("[%s] base_mac %pM\n", __func__, priv_data->base_mac);

	cls_eth_print("[%s] done!\n", __func__);
	}
#endif
	if(ofnode_read_bool(dev_ofnode(dev), "qspi_to_rgmii2")) {
		/*Pin Switch FOR EVB-A*/
		printf("Pin Switch FOR EVB-A FROM QSPI to RGMII2\n");
		writel(0x4008, 0x90420030);
		writel(0x4008, 0x90420034);
		writel(0x14008, 0x90420038);
		writel(0x4008, 0x9042003c);
		writel(0x4008, 0x90420040);
		writel(0x4008, 0x90420044);
		writel(0x2400c, 0x90420048);
		writel(0x400c, 0x9042004c);
		writel(0x400c, 0x90420050);
		writel(0x1400c, 0x90420054);
		writel(0x1400c, 0x90420058);
		writel(0x400c, 0x9042005c);
	}

	/* try get hw-version */
	hw_version = try_get_hw_version();

	dev_np = try_get_device_ofnode_by_version(dev, hw_version);

	version_str = ofnode_read_string(dev_np, "hw-version");
	if (version_str)
		cls_eth_print("hw version:%s\n", version_str);
	else
		cls_eth_print("hw version:%#x\n", hw_version);

	//foreach node
	ofnode_for_each_subnode(child_np, dev_np) {
		if (ofnode_device_is_compatible(child_np,
					"clourney,dwmac")) {

			if (!ofnode_is_available(child_np))
				continue;

			err = cls_mac_create(priv_data, child_np);
			if(err) {
				printf("cls_mac_create failed !!! err = %d\n", err);
				goto err_port_init;
			}
		} else if (ofnode_device_is_compatible(child_np,
					"clourney,xpcs")) {

			if (!ofnode_is_available(child_np))
				continue;

			err = cls_pcs_create(priv_data, child_np);
			if(err) {
				printf("cls_pcs_create failed !!! err = %d\n", err);
				goto err_port_init;
			}
#ifdef CONFIG_CLS_YTSWITCH && defined(CONFIG_CLS_PHYLINK)
		} else if (ofnode_device_is_compatible(child_np,
					"clourney,yt_switch")) {

			if (g_skip_extswitch || !ofnode_is_available(child_np))
				continue;

			err = cls_yt_switch_create(priv_data, child_np);
			if(err) {
				printf("cls_yt_switch_create failed !!! err = %d\n", err);
				goto err_port_init;
			}
#endif
		}
	}

#if 0
	/*Pin Switch FOR EVB-A*/
	printf("Pin Switch FOR EVB-A\n");
	writel(0x4008, 0x90420030);
	writel(0x4008, 0x90420034);
	writel(0x14008, 0x90420038);
	writel(0x4008, 0x9042003c);
	writel(0x4008, 0x90420040);
	writel(0x4008, 0x90420044);
	writel(0x2400c, 0x90420048);
	writel(0x400c, 0x9042004c);
	writel(0x400c, 0x90420050);
	writel(0x1400c, 0x90420054);
	writel(0x1400c, 0x90420058);
	writel(0x400c, 0x9042005c);
#endif
	/* Register PHY Driver */
#ifdef CONFIG_CLS_PHYLINK
#ifdef CONFIG_CLS_RTKSDK
	rtlsdk_phy_register();
#endif
	yt_phy_register();
#endif
err_port_init:
		return 0;
}

/* Init plat_data */
static int cls_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	const u8 *mac;

	pdata->iobase = dev_read_addr_index(dev, 0);
	pdata->phy_interface = -1;
	pdata->max_speed = -1;

	mac = dev_read_u8_array_ptr(dev, "local-mac-address", ETH_ALEN);
	if (!mac || !is_valid_ethaddr(mac)) {
		cls_eth_print("[%s] invalid macaddr in DT\n", __func__);
		return -EINVAL;
	}

	memcpy(pdata->enetaddr, mac, ETH_ALEN);

	return 0;
}

static const struct udevice_id cls_eth_ids[] = {
	{ .compatible = "clourney,cls edma" },
	{ }
};

static const struct eth_ops cls_eth_ops = {
	.start = cls_eth_start,
	.send = cls_eth_send,
	.recv = cls_eth_recv,
	.stop = cls_eth_stop,
	.free_pkt = cls_eth_free_rx_pkt,
};

U_BOOT_DRIVER(cls_eth) = {
	.name   = "cls_eth",
	.id     = UCLASS_ETH,
	.of_match = cls_eth_ids,
	.ops    = &cls_eth_ops,
	.probe  = cls_eth_probe,
	.of_to_plat = cls_eth_of_to_plat,
	.priv_auto	= sizeof(struct cls_eth_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
