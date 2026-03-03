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

#include "dubhe2000.h"
#include <linux/platform_device.h>
#include <net/ip6_checksum.h>
#include <linux/io.h>
#include <linux/prefetch.h>
#include <linux/bitops.h>
#include <linux/if_vlan.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/phy.h>
#include <linux/version.h>
#ifdef CONFIG_DUBHE2000_PHYLINK
#include <linux/phylink.h>
#endif
#include <linux/mii.h>
#include <linux/ethtool.h>

#include "dubhe2000_tag.h"
#include "dubhe2000_switch.h"

#include "dubhe2000_debugfs.h"
#ifdef CONFIG_DUBHE2000_PHYLINK
#include "dubhe2000_mdio.h"
#include "dubhe2000_xpcs.h"
#endif
#include "dubhe2000_mac.h"
#include "dubhe2000_mac_stats.h"

#define CREATE_TRACE_POINTS
#include "dubhe2000_trace.h"

#include "asm/cacheflush.h"

#define ENABLE	1
#define DISABLE 0

#define DUBHE1000_SW_BMU_EN  0
#define DUBHE1000_TX_OPTI_EN 0

#define DUBHE1000_NAPI_POLL_WEIGHT    8
#define DUBHE1000_TX_NAPI_POLL_WEIGHT 32

#define DUBHE1000_POLL_INTERVAL 5 // 5  --> 10 s

#define DUBHE1000_MAX_TXD_PWR	   12
#define DUBHE1000_MAX_DATA_PER_TXD (1 << DUBHE1000_MAX_TXD_PWR)

#define INET_RECVMSG_FREE_RX_BUF 0

#ifdef CONFIG_DUBHE2000_PHYLINK
#define dubhe1000_pcs_to_port(pcs) container_of(pcs, struct dubhe1000_mac, phylink_pcs)
#endif
char cls_npe_driver_name[] = "cls_npe";
static char *edma_rx_name[4] = { "EDMA_RX_LHOST", "EDMA_RX_2_4G", "EDMA_RX_5G", "EDMA_RX_PCIE" };
int (*switch_init)(struct mii_bus *mii);
static int dubhe1000_setup_tx_resources(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *txdr);
static int dubhe1000_setup_rx_resources(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rxdr);
static void dubhe1000_free_tx_resources(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring);
static void dubhe1000_free_rx_resources(struct dubhe1000_adapter *adapter, int channel,
					struct dubhe1000_rx_ring *rx_ring);

static int dubhe2000_probe(struct platform_device *pdev);
static int dubhe2000_remove(struct platform_device *pdev);

static int dubhe1000_alloc_queues(struct dubhe1000_adapter *adapter);

static int dubhe1000_sw_init(struct dubhe1000_adapter *adapter);
static void dubhe1000_configure_rx(struct dubhe1000_adapter *adapter);
static void dubhe1000_clean_tx_ring(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring);
static void dubhe1000_clean_rx_ring(struct dubhe1000_adapter *adapter, int channel, struct dubhe1000_rx_ring *rx_ring);
static int dubhe1000_open(struct net_device *netdev);
static int dubhe1000_close(struct net_device *netdev);
static void dubhe1000_tx_timeout(struct net_device *dev, unsigned int txqueue);

static int dubhe1000_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
static netdev_tx_t dubhe1000_xmit_frame(struct sk_buff *skb, struct net_device *netdev);

static irqreturn_t dubhe1000_clean_tx_irq(int irq, void *data);
static irqreturn_t dubhe1000_intr(int irq, void *data);
#if (DUBHE1000_TX_OPTI_EN)
int dubhe1000_tx_clean(struct napi_struct *napi, int budget);
#endif
static bool dubhe1000_clean_rx_irq(struct dubhe1000_adapter *adapter, int channel, struct dubhe1000_rx_ring *rx_ring,
				   int *work_done, int work_to_do);
static void dubhe1000_alloc_rx_buffers(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring,
				       int cleaned_count);

#ifdef CONFIG_DUBHE2000_DEVLINK
static int dubhe1000_devlink_port_register(struct dubhe1000_mac *port);
#endif

static void dubhe1000_unmap_and_free_tx_resource(struct dubhe1000_adapter *adapter,
						 struct dubhe1000_tx_buffer *buffer_info);
#if (DUBHE1000_SW_BMU_EN)
static void *dubhe1000_alloc_frag(const struct dubhe1000_adapter *adapter);
#endif
struct dubhe1000_mac *dubhe1000_find_port(struct dubhe1000_adapter *adapter, u32 id);
static void dubhe1000_init_interfaces(struct dubhe1000_adapter *adapter);

#ifdef CONFIG_DUBHE2000_PHYLINK
static int dubhe1000_phylink_setup(struct dubhe1000_mac *priv);
static int dubhe1000_init_phy(struct net_device *dev);
#endif

#define COPYBREAK_DEFAULT 256
static unsigned int copybreak __read_mostly = COPYBREAK_DEFAULT;
module_param(copybreak, uint, 0644);
MODULE_PARM_DESC(copybreak, "Maximum size of packet that is copied to a new buffer on receive");

unsigned int verbose = 0;
module_param(verbose, uint, 0644);
MODULE_PARM_DESC(verbose, "be verbose(default is 0)");

/* Match table for of_platform binding */
static const struct of_device_id dubhe2000_of_match[] = {
	{ .compatible = "clourney,dubhe2000 npe" },
	{},
};
MODULE_DEVICE_TABLE(of, dubhe2000_of_match);
#define PAUSE_TIME 0xffff
/* Flow Control defines */
#define FLOW_OFF   0
#define FLOW_RX	   1
#define FLOW_TX	   2
#define FLOW_AUTO  (FLOW_TX | FLOW_RX)

static int flow_ctrl = FLOW_AUTO;
module_param(flow_ctrl, int, 0644);
MODULE_PARM_DESC(flow_ctrl, "Flow control ability [on/off]");

static int pause = PAUSE_TIME;
module_param(pause, int, 0644);
MODULE_PARM_DESC(pause, "Flow Control Pause Time");

static struct platform_driver dubhe1000_driver = {
	.probe    = dubhe2000_probe,
	.remove    = dubhe2000_remove,
	.driver = {
		 .name = "clourney",
		 .of_match_table = dubhe2000_of_match,
	},
};

#ifdef CONFIG_DUBHE1000_MODULE
static struct platform_driver *const dubhe1000_edma_driver[] = {
	&dubhe1000_driver,
};

static int __init dubhe1000_driver_init(void)
{
	platform_register_drivers(dubhe1000_edma_driver, ARRAY_SIZE(dubhe1000_edma_driver));
	return 0;
}
module_init(dubhe1000_driver_init);

static void __exit dubhe1000_driver_exit(void)
{
	platform_unregister_drivers(dubhe1000_edma_driver, ARRAY_SIZE(dubhe1000_edma_driver));
}
module_exit(dubhe1000_driver_exit);
#else
module_platform_driver(dubhe1000_driver);
#endif

MODULE_AUTHOR("Clourneysemi Corporation, <linux.nics@clourneysemi.com>");
MODULE_DESCRIPTION("Clourneysemi DUBHE2000 Network Driver");
MODULE_LICENSE("GPL v2");

#define DEFAULT_MSG_ENABLE (NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK)
static int debug = -1;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

u32 g_rx_intr = 0;
u32 g_tx_intr = 0;
int g_tx_index = 0;

int switch_pause_mod = -1;
module_param(switch_pause_mod, int, 0644);
MODULE_PARM_DESC(switch_pause_mod, "switch pause mode");

static int switch_pure_mod = 0;
module_param(switch_pure_mod, int, 0644);
MODULE_PARM_DESC(switch_pure_mod, "switch work without init process");

struct dubhe1000_adapter *g_adapter = NULL;
extern dma_addr_t dubhe1000_cls_bmu_dma_addr(void *addr);
extern dma_addr_t (*cls_bmu_dma_addr)(void *addr);

static inline void dubhe1000_save_status1(struct dubhe1000_adapter *adapter, uint32_t channel, u8 *tag_buf, int pkt_id,
					  u8 buf_bid, u16 buf_pid, u8 blk_num, u16 packet_tid, u32 status1)
{
	//magic
	((u16 *)tag_buf)[0] = 0x9999;
	// pkt-id
	((u16 *)tag_buf)[1] = pkt_id;

	// status1
	((u32 *)tag_buf)[1] = status1;

	((u32 *)tag_buf)[2] = channel;

	if (netif_msg_pktdata(g_adapter))
		pr_info("save pkt_id %d bid %d pid %d blk-num %d token-id %d status1 %x on ch %d\n",
			pkt_id, buf_bid, buf_pid, blk_num, packet_tid, status1, channel);
}

void dubhe1000_msg_set(struct dubhe1000_adapter *adapter, int port_id, u32 level)
{
	struct dubhe1000_mac *port;
	struct net_device *netdev = NULL;
	u32 old_msg_level = 0;

	port = dubhe1000_find_port(adapter, port_id);
	if (unlikely(!port)) {
		pr_info("msg level non-existent port(%u)\n", port_id);
		return;
	}
	netdev = port->netdev;
	old_msg_level = netdev->ethtool_ops->get_msglevel(netdev);

	netdev->ethtool_ops->set_msglevel(netdev, level);

	pr_info("%s msg level old msg_enable 0x%x new 0x%x\n", netdev->name, old_msg_level, adapter->msg_enable);
}

static void dubhe1000_skb_dump(struct dubhe1000_adapter *adapter, struct sk_buff *skb, bool tx)
{
	int k2, k = 0;
	int len = skb->len;
	u8 hdr[32] = { 0 };
	u64 address2 = 0;

	if (skb->len > 64)
		len = 64;

	k2 = len / 16;

	address2 = (u64)skb->data;
	pr_info("%s packet:\n", tx ? "tx" : "rx");
	snprintf(hdr, sizeof(hdr), "skb(data[k=%d]): ", k);
	print_hex_dump(KERN_INFO, hdr, DUMP_PREFIX_NONE, 16, 1, (const void *)address2, 16, 0);

	for (k = 0; k < k2; k++) {
		if (k == 0)
			continue;
		address2 += 16;

		snprintf(hdr, sizeof(hdr), "skb(data[k=%d]): ", k);
		print_hex_dump(KERN_INFO, hdr, DUMP_PREFIX_NONE, 16, 1, (const void *)address2, 16, 0);
	}

	address2 += 16;
	snprintf(hdr, sizeof(hdr), "skb(data[k=%d]): ", k);
	print_hex_dump(KERN_INFO, hdr, DUMP_PREFIX_NONE, 16, 1, (const void *)address2, 16, 0);
}

static void dubhe1000_poll_handler(struct work_struct *work)
{
	struct dubhe1000_adapter *adapter =
		(struct dubhe1000_adapter *)container_of(work, struct dubhe1000_adapter, time_work.work);
	u32 channel;
	u32 tx_queue_qsize = 0, tx_packet_id = 0;
	u8 tx_pkt_err = 0, is_split = 0;

	dubhe1000_clean_tx_irq(adapter->tx_irq, adapter);

	for (channel = 0; channel < adapter->num_rx_queues; channel++) {
		tx_queue_qsize = er32_x(channel, TX_QUEUE_STATUS0);
		tx_packet_id = ((tx_queue_qsize & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((tx_queue_qsize & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((tx_queue_qsize & 0x4000) >> TX_PKT_ERR_BIT);

		tx_queue_qsize = (tx_queue_qsize & 0xFFF);

		if (tx_queue_qsize > 0)
			dubhe1000_clean(&adapter->edma_rx_napi[channel], DUBHE1000_NAPI_POLL_WEIGHT);
	}

	queue_delayed_work(adapter->wq, &adapter->time_work, msecs_to_jiffies(DUBHE1000_POLL_INTERVAL));
}

static inline void dubhe1000_poll(struct dubhe1000_adapter *adapter)
{
	adapter->wq = alloc_ordered_workqueue("dubhe1000_intr_poll", WQ_MEM_RECLAIM);

	if (adapter->wq)
		INIT_DELAYED_WORK(&adapter->time_work, dubhe1000_poll_handler);
}

static inline void dubhe1000_free_irq(struct dubhe1000_adapter *adapter)
{
	int channel;

	free_irq(adapter->tx_irq, adapter);

	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		free_irq(adapter->rx_intr_list[channel], adapter);
}

static int dubhe1000_request_irq(struct dubhe1000_adapter *adapter)
{
	int err;
	int channel;
	const char *driver_name = cls_npe_driver_name;

	for (channel = 0; channel < adapter->num_rx_queues; channel++) {
		err = request_irq(adapter->rx_intr_list[channel], dubhe1000_intr, IRQF_SHARED, edma_rx_name[channel],
				  adapter);
		if (err) {
			e_dev_err("Unable to allocate rx interrupt Error: %d rx_irq %d\n",
				  err, adapter->rx_intr_list[channel]);
		}
	}

	err = request_irq(adapter->tx_irq, dubhe1000_clean_tx_irq, IRQF_SHARED, driver_name, adapter);
	if (err)
		e_dev_err("Unable to allocate tx interrupt Error: %d\n", err);

	return err;
}

static inline void dubhe1000_clear_tx_irq_raw(void)
{
	int val = 0;

	val = er32(RX_INTERRUPT_RAW_RPT);
	ew32(RX_INTERRUPT_RAW_RPT, val);
}

static inline void dubhe1000_clear_rx_irq_raw(int channel)
{
	s32 val = 0;

	val = er32_x(channel, TX_INTERRUPT_RAW_RPT);
	ew32_x(channel, TX_INTERRUPT_RAW_RPT, val);
}

static inline void dubhe1000_enable_tx_irq(struct dubhe1000_adapter *adapter, int enable)
{
#if EDMA_HW
	if (enable) {
		dubhe1000_clear_tx_irq_raw();
		// 1: Enable Interrupt, 0: Mask Interrupt
		ew32(RX_INTERRUPT_EN, 1); // bit0: rx_queue_int_mask
	} else
		ew32(RX_INTERRUPT_EN, 0); // bit0: rx_queue_int_mask

#endif
}

/**
 * dubhe1000_enable_rx_irq - Enable default interrupt generation settings
 * @adapter: board private structure
 *
 * Note:
 *    a): need to Write clear the MASK Status or RAW Status?
 *    b): clear interrupt need or not for interrupt enable operation?
 *    c): clear interrupt work fine before interrupt enable or need clear after enable ?
 *    d): rx/tx interrupt enable respectivly?
 */

static inline void dubhe1000_enable_rx_irq(struct dubhe1000_adapter *adapter, int channel, int enable)
{
#if EDMA_HW
	if (enable) {
		dubhe1000_clear_rx_irq_raw(channel);
		// 1: Enable Interrupt, 0: Mask Interrupt
		ew32_x(channel, TX_INTERRUPT_EN, 1); // bit0: rx_queue_int_mask
	} else
		ew32_x(channel, TX_INTERRUPT_EN, 0); // bit0: rx_queue_int_mask
#endif
}

static inline void dubhe1000_irq_enable_all(struct dubhe1000_adapter *adapter)
{
#if EDMA_HW
	int channel;

	// 1: Enable Interrupt, 0: Mask Interrupt
	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		dubhe1000_enable_rx_irq(adapter, channel, ENABLE);

	dubhe1000_enable_tx_irq(adapter, ENABLE);
#endif
}

/**
 * dubhe1000_irq_disable_all - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 *
 * Note:
 *    a): need to Write clear the MASK Status or RAW Status?
 *    b): clear interrupt should after packet done and the buffer free,
 *        so, clear intterupt can't following disable interrupt
 *    c): rx/tx interrupt disable respectivly?
 **/
static void dubhe1000_irq_disable_all(struct dubhe1000_adapter *adapter)
{
#if EDMA_HW
	int channel;
	// 1: Enable Interrupt, 0: Mask Interrupt
	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		dubhe1000_enable_rx_irq(adapter, channel, DISABLE);

	dubhe1000_enable_tx_irq(adapter, DISABLE); // bit0: rx_queue_int_mask
#endif

	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		synchronize_irq(adapter->rx_intr_list[channel]); // ???
}
/**
 * dubhe1000_configure - configure the hardware for RX and TX
 * @adapter: private board structure
 **/
static void dubhe1000_configure(struct dubhe1000_adapter *adapter)
{
	int i;
	struct dubhe1000_rx_ring *ring;

	dubhe1000_configure_rx(adapter);

	/* call DUBHE1000_DESC_UNUSED which always leaves
	 * at least 1 unused to make sure
	 * next_to_use != next_to_clean
	 */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		ring = &adapter->rx_ring[i];
		adapter->alloc_rx_buf(adapter, ring, DUBHE1000_DESC_UNUSED(ring));
	}
}

static int dubhe1000_mac_addr_set(struct net_device *netdev, void *p)
{
#ifndef DISABLE_TRANSMIT
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	/* Proceed further only if, User provided MAC is different
	 * from active MAC
	 */
	if (ether_addr_equal(addr->sa_data, netdev->dev_addr))
		return 0;

	ether_addr_copy(netdev->dev_addr, addr->sa_data);
	pr_info("%s MAC address changed to %pM\n", netdev->name, addr->sa_data);
#endif
	return 0;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/**
 * dubhe1000_netpoll - A Polling 'interrupt' handler
 * @netdev: network interface device structure
 *
 * This is used by netconsole to send skbs without having to re-enable
 * interrupts.  It's not called while the normal interrupt routine is executing.
 **/
static void dubhe1000_netpoll(struct net_device *netdev)
{
	int channel;
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;

	for (channel = 0; channel < adapter->num_rx_queues; channel++) {
		if (disable_hardirq(adapter->rx_intr_list[channel]))
			dubhe1000_intr(adapter->rx_intr_list[channel], adapter);
	}

	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		enable_irq(adapter->rx_intr_list[channel]);
}
#endif

void dubhe1000_get_stats64(struct net_device *netdev, struct rtnl_link_stats64 *storage)
{
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
	u8 xgmac_index = port->id;
	struct dubhe1000_xgmac_stats_param *xgmac_stats = NULL;

	spin_lock(&adapter->stats64_lock);

	// get xgmac XGMAC TX STATISTICS
	dubhe1000_xgmac_stats_update_option(adapter, xgmac_index, XGMAC_STATS_OPTION_TX, &xgmac_stats);
	storage->tx_packets = xgmac_stats[TX_PACKET_COUNT_GOOD_BAD].value;
	storage->tx_bytes = xgmac_stats[TX_OCTET_COUNT_GOOD_BAD].value;

	// Because xgmac's abort cnt includes carrier/window error, just return 0
	storage->tx_aborted_errors = 0;
	storage->tx_carrier_errors = xgmac_stats[TX_CARRIER_ERROR_PACKETS].value;
	storage->tx_fifo_errors = xgmac_stats[TX_UNDERFLOW_ERROR_PACKETS].value;
	storage->tx_heartbeat_errors = 0;
	storage->tx_window_errors = xgmac_stats[TX_LATE_COLLISION_PACKETS].value;

	storage->tx_errors = xgmac_stats[TX_EXCESSIVE_COLLISION_PACKETS].value +
			     xgmac_stats[TX_EXCESSIVE_DEFERRAL_ERROR].value + storage->tx_aborted_errors +
			     storage->tx_carrier_errors + storage->tx_fifo_errors + storage->tx_heartbeat_errors +
			     storage->tx_window_errors;

	// get xgmac XGMAC RX STATISTICS
	dubhe1000_xgmac_stats_update_option(adapter, xgmac_index, XGMAC_STATS_OPTION_RX, &xgmac_stats);
	storage->rx_packets = xgmac_stats[RX_PACKET_COUNT_GOOD_BAD].value;
	storage->rx_bytes = xgmac_stats[RX_OCTET_COUNT_GOOD_BAD].value;
	// only one rx drop reason
	storage->rx_dropped = xgmac_stats[RX_FIFOOVERFLOW_PACKETS].value;
	storage->multicast = xgmac_stats[RX_MULTICAST_PACKETS_GOOD].value;

	storage->rx_crc_errors = xgmac_stats[RX_CRC_ERROR_PACKETS].value;
	storage->rx_frame_errors = xgmac_stats[RX_ALIGNMENT_ERROR_PACKETS].value;
	storage->rx_length_errors = xgmac_stats[RX_LENGTH_ERROR_PACKETS].value;
	storage->rx_over_errors = 0;
	storage->rx_fifo_errors = 0; //software packet queue overflow or sequencing errors
	storage->rx_missed_errors = 0;

	/* This may not be very accurate: The total count of known error types  */
	storage->rx_errors = xgmac_stats[RX_WATCHDOG_ERROR_PACKETS].value + storage->rx_crc_errors +
			     storage->rx_frame_errors + storage->rx_length_errors + storage->rx_over_errors +
			     storage->rx_fifo_errors + storage->rx_missed_errors;

	spin_unlock(&adapter->stats64_lock);
}

static int dubhe1000_change_mtu(struct net_device *dev, int new_mtu)
{
	const int mtu = new_mtu;
	dev->mtu = mtu;

	netdev_update_features(dev);

	return 0;
}

static const struct net_device_ops dubhe1000_netdev_ops = {
	.ndo_open		= dubhe1000_open,
	.ndo_stop		= dubhe1000_close,
	.ndo_change_mtu		= dubhe1000_change_mtu,
	.ndo_start_xmit		= dubhe1000_xmit_frame,
	.ndo_set_mac_address	= dubhe1000_mac_addr_set,
	.ndo_tx_timeout		= dubhe1000_tx_timeout,
	.ndo_do_ioctl		= dubhe1000_ioctl,
	.ndo_get_stats64	= dubhe1000_get_stats64,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= dubhe1000_netpoll,
#endif
};

#ifdef CONFIG_DUBHE2000_PHYLINK
static inline void dubhe1000_init_mdio(struct dubhe1000_adapter *adapter, int id)
{
	int err;
	struct dubhe1000_mac *port = adapter->mac[id];

	/* MDIO bus Registration */
	if (id < DUBHE1000_MDIO_COUNT) {
		struct dubhe1000_mdio *mdio_obj = NULL;

		mdio_obj = dubhe1000_mdio_register(adapter, id);
		if (IS_ERR(mdio_obj) || !mdio_obj)
			pr_err("%s:%d MDIO bus (id: %d) registration failed", __func__, __LINE__, id);
	}

	if (port == NULL) {
		pr_info("Not found XGMAC%d object", id);
		return;
	}

	err = dubhe1000_mdio_register_by_phy(port->netdev);
	if (err < 0)
		pr_err("%s:%d MDIO bus (id: %d) registration failed", __func__, __LINE__, port->id);
}
#endif

static void dubhe1000_init_port(struct dubhe1000_adapter *adapter, int id)
{
	int err;
	struct dubhe1000_mac *port = adapter->mac[id];
	struct net_device *netdev;

	if (port == NULL) {
		pr_info("Not found XGMAC%d object", id);
		return;
	}

	netdev = port->netdev;

	/* don't block initialization here due to bad MAC address */
	memcpy(netdev->dev_addr, port->mac_addr, netdev->addr_len);

	netdev->dev_addr[netdev->addr_len - 1] += port->id;

	sprintf(netdev->name, "eth%d", port->id);

#ifdef CONFIG_DUBHE2000_PHYLINK
	err = dubhe1000_phylink_setup(port);
	if (err) {
		pr_info("Phy Setup Error ret[%d]\n", err);
	} else {
		if (verbose)
			pr_info("Phy Setup Successfully");
	}
#endif

	err = register_netdev(netdev);
	if (err) {
		pr_info("%s: ERROR %i registering the device\n", __func__, err);
		return;
	}

	netdev->if_port = dubhe1000_switch_port_to_if_port(id);
	pr_info("[%s] %s if_port=%d\n", __func__, netdev->name, netdev->if_port);

	dubhe1000_port_dbg_init(port);

	/* carrier off remacing is immacant to ethtool even BEFORE open */
	netif_carrier_off(netdev);

#ifdef CONFIG_DUBHE2000_DEVLINK
	devlink_port_type_eth_set(&port->dl_port, port->netdev);
#endif

	if (verbose)
		pr_info("%s mac %s (%d) HW-Addr %02x:%02x:%02x:%02x:%02x:%02x hw_features %llx\n",
			__func__, netdev->name, port->id,
			netdev->dev_addr[0], netdev->dev_addr[1], netdev->dev_addr[2],
			netdev->dev_addr[3], netdev->dev_addr[4], netdev->dev_addr[5],
			netdev->hw_features);

	if (!is_valid_ether_addr(netdev->dev_addr))
		e_err(probe, "Invalid MAC Address\n");
}

static void dubhe1000_poll_mac_monotor(struct work_struct *work)
{
	struct dubhe1000_adapter *adapter = container_of(work, struct dubhe1000_adapter, mac_monitor_work.work);
	struct dubhe1000_mac *port = NULL;
	int index = 0;
	int status = 0;

	for (index = 0; index < DUBHE1000_MAC_COUNT; index++) {
		port = adapter->mac[index];

		if (!port || !port->mac_monitor)
			continue;

		status = dubhe1000_xgmac_get_link_status(port);

		if (verbose)
			pr_err("Link Status: %d\n", status);

		if (netif_carrier_ok(port->netdev) == status)
			continue;

		if (status) {
			dubhe1000_switch_drain_port(adapter, port->id, false);
			dubhe1000_enable_xgmac(port, true);
			netif_carrier_on(port->netdev);
			printk(KERN_ERR "eth%d Link up\n", port->id);
		} else {
			dubhe1000_enable_xgmac(port, false);
			dubhe1000_switch_drain_port(adapter, port->id, true);
			netif_carrier_off(port->netdev);
			printk(KERN_ERR "eth%d Link Down\n", port->id);
		}
	}

	if (refcount_read(&adapter->mac_monitor_ref) > 1)
		schedule_delayed_work(&adapter->mac_monitor_work, msecs_to_jiffies(DUBHE1000_MAC_MONITOR_DELAY_MS));
}

static inline void dubhe1000_init_ports(struct dubhe1000_adapter *adapter)
{
	int index;

#ifdef CONFIG_DUBHE2000_PHYLINK

	refcount_set(&adapter->mac_monitor_ref, 1);
	INIT_DELAYED_WORK(&adapter->mac_monitor_work, dubhe1000_poll_mac_monotor);

	/* Must init modio first for multiple phylinks share one mdio */
	for (index = 0; index < DUBHE1000_MAC_COUNT; index++)
		dubhe1000_init_mdio(adapter, index);

	rtksdk_phy_init();
#endif

	for (index = 0; index < DUBHE1000_MAC_COUNT; index++)
		dubhe1000_init_port(adapter, index);
}

inline struct dubhe1000_mac *dubhe1000_find_port(struct dubhe1000_adapter *adapter, u32 id)
{
	return adapter->mac[id];
}

#ifdef CONFIG_DUBHE2000_PHYLINK
static int dubhe1000_pcs_create(struct dubhe1000_adapter *adapter, struct device_node *np)
{
	int err, id;
	phy_interface_t phy_mode;
	struct dubhe1000_pcs *pcs;
	resource_size_t size;
	const __be32 *_id = of_get_property(np, "id", NULL);

	if (!_id) {
		pr_info("missing mac id\n");
		return -EINVAL;
	}

	id = be32_to_cpup(_id);
	if (id >= DUBHE1000_PCS_COUNT) {
		pr_err("%d is not a valid xpcs id\n", id);
		return -EINVAL;
	}

	if (adapter->pcs[id]) {
		pr_err("duplicate pcs id found: %d\n", id);
		return -EINVAL;
	}

	pcs = devm_kzalloc(adapter->dev, sizeof(*pcs), GFP_KERNEL);
	if (!pcs) {
		err = -ENOMEM;
		goto out;
	}

	err = of_get_phy_mode(np, &phy_mode);
	if (err && err != -ENODEV) {
		pr_err("Can't get phy-mode\n");
		goto err;
	}

	pcs->phy_interface = phy_mode;

	if (phy_mode == PHY_INTERFACE_MODE_USXGMII) {
		err = of_property_read_u32(np, "usxg_mode", &pcs->usxg_mode);
		if (err)
			pcs->usxg_mode = 0;

		if (verbose)
			pr_info("Set usxg_mode[%d]\n", pcs->usxg_mode);
	}

	err = of_property_read_u32(np, "auto-negotiation", &pcs->autoneg);
	if (err)
		pcs->autoneg = 2; /*Default Enable*/

	if (verbose)
		pr_info("Set pcs autoneg [%s]\n", pcs->autoneg ? "true" : "false");

	pcs->ioaddr = devm_of_iomap(adapter->dev, np, 0, &size);
	if (IS_ERR(pcs->ioaddr)) {
		pr_info("Get PCS[%d] ioaddr form DTS err = 0x%px\n", id, pcs->ioaddr);
		goto err;
	}

	if (verbose)
		pr_info("Get XPCS[%d] ioaddr form DTS iomap[0x%px]\n", id, pcs->ioaddr);

	pcs->irq = platform_get_irq_byname(adapter->pdev, id ? "xpcs1" : "xpcs0");
	if (pcs->irq < 0) {
		pr_info("No DBERR IRQ resource irq[%s] %d\n", id ? "xpcs1" : "xpcs0", pcs->irq);
		err = -ENOMEM;
		goto err;
	}

	adapter->pcs[id] = pcs;
	pcs->adapter = adapter;
	pcs->id = id;

	return 0;
err:
	devm_kfree(adapter->dev, pcs);
out:
	return err;
}
#endif

static int dubhe1000_mac_create(struct dubhe1000_adapter *adapter, struct device_node *np)
{
	int err, id;
	const __be32 *_id = of_get_property(np, "id", NULL);
	struct net_device *netdev;
	struct dubhe1000_mac *port;
	struct device_node *mdio_node;

	if (!_id) {
		pr_info("missing mac id\n");
		return -EINVAL;
	}

	id = be32_to_cpup(_id);
	if (id >= DUBHE1000_MAC_COUNT) {
		pr_err("%d is not a valid mac id\n", id);
		return -EINVAL;
	}

	if (adapter->mac[id]) {
		pr_err("duplicate mac id found: %d\n", id);
		return -EINVAL;
	}

	netdev = alloc_etherdev_mqs(sizeof(struct dubhe1000_mac), adapter->num_tx_queues, adapter->num_rx_queues);
	if (!netdev) {
		err = -ENOMEM;
		goto err_netdev_malloc;
	}

	SET_NETDEV_DEV(netdev, adapter->dev);

	port = netdev_priv(netdev);
	port->netdev = netdev;
	port->id = id;
	port->adapter = adapter;
	port->device = adapter->dev;
	adapter->mac[id] = port;
	netdev->dev.of_node = np;

	port->ioaddr = devm_of_iomap(adapter->dev, np, 0, NULL);
	if (IS_ERR(port->ioaddr)) {
		err = PTR_ERR(port->ioaddr);
		pr_info("Get XGMAC[%d] ioaddr form DTS err = 0x%px\n", id, port->ioaddr);
		goto err_netdev_malloc;
	}

	if (verbose)
		pr_info("Get XGMAC[%d] ioaddr form DTS iomap[0x%px]\n", id, port->ioaddr);

	memset(port->mac_addr, 0, ETH_ALEN);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 146)
	const char *mac_addr;

	mac_addr = of_get_mac_address(np);
	if (IS_ERR(mac_addr)) {
		if (PTR_ERR(mac_addr) == -EPROBE_DEFER) {
			err = PTR_ERR(mac_addr);
			goto err_register_netdev;
		}
		mac_addr = NULL;
	} else {
		ether_addr_copy(port->mac_addr, mac_addr);
	}
#else
	of_get_mac_address(np, port->mac_addr);
#endif

	if (!is_valid_ether_addr(port->mac_addr)) {
		eth_random_addr(port->mac_addr);
		dev_info(adapter->dev, "using random base mac address\n");
	}

#ifdef CONFIG_DUBHE2000_PHYLINK
	port->interface = XGMAC_INTERFACE_MODE_NON_RGMII;

	if (id == 0 || id == 2 || id == 4) {
		const char *str;

		err = of_property_read_string(np, "mode", &str);
		if (err && err != -ENODEV) {
			pr_err("Can't get mac mode\n");
			err = -EINVAL;
			goto err_register_netdev;
		}

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

	if (port->interface == XGMAC_INTERFACE_MODE_RGMII) {
		err = of_property_read_u32(np, "rgmii-rx-delay", &port->rgmii_rx_delay);
		if (err)
			port->rgmii_rx_delay = 2;

		err = of_property_read_u32(np, "rgmii-tx-delay", &port->rgmii_tx_delay);
		if (err)
			port->rgmii_tx_delay = 0;

		pr_err("set rgmii rx_delay [%d] tx_delay [%d]\n", port->rgmii_rx_delay, port->rgmii_tx_delay);

		port->phy_interface = PHY_INTERFACE_MODE_RGMII;
	} else
		port->phy_interface = PHY_INTERFACE_MODE_NA;

	if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
		int pcs_sel = 0;

		switch (id) {
		case 1:
		case 2:
		case 3:
			err = of_property_read_u32(np, "pcs-sel", &pcs_sel);
			if (err) {
				pcs_sel = 1;
				pr_err("Can't get pcs_sel,Set pcs_sel [%d]", pcs_sel);
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

		if (verbose)
			pr_info("XGMAC%d set pcs_sel [%d]\n", id, port->pcs_sel);
	}

	/* MDIO SEL */
	port->mdio_sel = id;
	if (id < 3) {
		err = of_property_read_u32(np, "mdio-sel", &port->mdio_sel);
		if (err) {
			if (verbose)
				pr_err("Can't get mdio_sel,Set mdio_sel [%d]", port->mdio_sel);
		}

		if (port->mdio_sel > 2)
			port->mdio_sel = id;

		if (verbose)
			pr_info("XGMAC1 set mdio_sel [%d]", port->mdio_sel);
	}

	/* Get max speed of operation from device tree */
	if (of_property_read_u32(np, "max-speed", &port->max_speed)) {
		if (verbose)
			pr_info("Can't get max-speed, use 1000\n");
		port->max_speed = 1000;
	}

	mdio_node = of_get_child_by_name(np, "mdio");
	if (mdio_node) {
		do {
			if (!of_device_is_available(mdio_node))
				break;

			if (id >= DUBHE1000_MDIO_COUNT) {
				pr_err("%s:%d ERR MDIO id:%d >= %d!!", __func__, __LINE__, id, DUBHE1000_MDIO_COUNT);
				break;
			}

			if (adapter->mdio_node[id]) {
				pr_err("%s:%d ERR MDIO%d already exist!!", __func__, __LINE__, id);
				break;
			}

			adapter->mdio_node[id] = mdio_node;

			if (verbose)
				pr_err("Get mdio in xgmac%d!!!", id);

		} while (0);
	}

	port->phy_node = of_parse_phandle(np, "sfp", 0);
	if (port->phy_node != NULL) {
		port->is_sfp = 2;
	} else if (of_property_read_bool(np, "sfp")) {
		port->is_sfp = 1;
	} else {
		port->phy_node = of_parse_phandle(np, "phy-handle", 0);
		if (port->phy_node != NULL) {
			/* PHYLINK automatically parses the phy-handle property */
			port->phylink_node = np;

			/* Default to phy auto-detection */
			port->phy_addr = -1;

			if (!port->phy_node || of_property_read_u32(port->phy_node, "reg", &port->phy_addr))
				pr_err("Can't get phy address\n");
			else {
				if (verbose)
					pr_err("Get phy address [%d]", port->phy_addr);
			}
		}
	}
	/* PHYLINK automatically parses the phy-handle property */
	port->phylink_node = np;

	if (of_property_read_u32(np, "snps,ps-speed", &port->mac_port_sel_speed)) {
		if (verbose)
			pr_info("Can't get ps-speed, use -1\n");
		port->mac_port_sel_speed = -1;
	}

#endif /*CONFIG_DUBHE2000_PHYLINK*/

	port->netdev->max_mtu = 1540;

#ifdef CONFIG_DUBHE2000_DEVLINK
	err = dubhe1000_devlink_port_register(port);
	if (err)
		goto err_register_netdev;
#endif

	netdev->netdev_ops = &dubhe1000_netdev_ops;

	dubhe1000_set_ethtool_ops(netdev);

	netdev->watchdog_timeo = 5 * HZ;

	return 0;

err_register_netdev:
	adapter->mac[id] = NULL;
	free_netdev(netdev);

err_netdev_malloc:
	return err;
}

#ifdef CONFIG_DUBHE2000_PHYLINK
static inline int dubhe1000_interfaces_validate(struct dubhe1000_adapter *adapter)
{
	int index, pcs_index;
	struct dubhe1000_mac *port;
	struct dubhe1000_pcs *pcs;

	for (index = 0; index < DUBHE1000_MAC_COUNT; index++) {
		port = adapter->mac[index];

		if (!port)
			continue;

		pcs_index = port->pcs_sel;

		pcs = adapter->pcs[pcs_index];

		if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
			if (!pcs) {
				pr_err("XGMAC%d MUST config XPCS%d DTS!", index, pcs_index);
				return -EINVAL;
			}

			port->phy_interface = pcs->phy_interface;
			/* XGMAC0 USE XPCS0, XGMAC1 USE XPCS0/XPCS1, XGMAC2-4 USE XPCS1*/
			port->pcs_port = pcs_index ? port->id - 1 : port->id;
			pcs->is_sfp = port->is_sfp;

			if (pcs->autoneg == 2)
				if (port->is_sfp ||
						(port->phylink_node && of_phy_is_fixed_link(port->phylink_node)))
					pcs->autoneg = 0;

			if (verbose)
				pr_info("Set XGMAC%d PHY interface type [%d], Use PCS%d port%d:autoneg:%d",
					index, port->phy_interface, pcs_index, port->pcs_port, pcs->autoneg);
		} else if (index != 0 && index != 2 && index != 4) /* Only XGMAC0,2,4 can choice RGMII*/ {
			pr_err("XGMAC%d NOT supported RGMII mode!", index);
			return -EINVAL;
		}

		if (!port->max_speed) {
			switch (port->phy_interface) {
			case PHY_INTERFACE_MODE_2500BASEX:
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
#endif

static void dubhe1000_mac_destroy(struct dubhe1000_adapter *adapter, int port_id)
{
	struct net_device *netdev;
	struct dubhe1000_mac *port = adapter->mac[port_id];

	if (!port)
		return;

	netdev = port->netdev;
	port_id = port->id;

	netif_carrier_off(port->netdev);
	netdev_reset_queue(netdev);
	unregister_netdev(netdev);

#ifdef CONFIG_DUBHE2000_PHYLINK
	if (port->phylink)
		phylink_destroy(port->phylink);
#elif defined(CONFIG_DUBHE2000_DEVLINK)
#else
#error "Please choice a phy management method"
#endif

	free_netdev(netdev);
}

static inline void dubhe1000_destroy_ports(struct dubhe1000_adapter *adapter)
{
	int index;

	for (index = 0; index < DUBHE1000_MAC_COUNT; index++)
		dubhe1000_mac_destroy(adapter, index);

	for (index = 0; index < DUBHE1000_MDIO_COUNT; index++)
		dubhe1000_mdio_unregister(adapter, index);
}

#ifdef CONFIG_DUBHE2000_DEVLINK
static int dubhe1000_devlink_port_register(struct dubhe1000_mac *port)
{
	struct dubhe1000_adapter *adapter = port->adapter;
	struct devlink *dl = priv_to_devlink(adapter);
	struct devlink_port_attrs attrs = {};
	int err;
	u8 id = 0;

	attrs.flavour = DEVLINK_PORT_FLAVOUR_PHYSICAL;
	attrs.phys.port_number = id;
	attrs.switch_id.id_len = sizeof(id);
	memcpy(attrs.switch_id.id, &id, attrs.switch_id.id_len);

	devlink_port_attrs_set(&port->dl_port, &attrs);

	err = devlink_port_register(dl, &port->dl_port, port->id);

	return 0;
}

static int dubhe1000_dl_info_get(struct devlink *dl, struct devlink_info_req *req, struct netlink_ext_ack *extack)
{
	return 0;
}

static const struct devlink_ops dubhe1000_dl_ops = {
	.info_get = dubhe1000_dl_info_get,
};

struct dubhe1000_adapter *dubhe1000_devlink_alloc(void)
{
	struct devlink *dl;

	dl = devlink_alloc(&dubhe1000_dl_ops, sizeof(struct dubhe1000_adapter));
	return devlink_priv(dl);
}

void dubhe1000_devlink_unregister(struct dubhe1000_adapter *adapter)
{
	struct devlink *dl = priv_to_devlink(adapter);

	devlink_unregister(dl);
	devlink_free(dl);
}

int dubhe1000_devlink_register(struct dubhe1000_adapter *adapter)
{
	struct devlink *dl = priv_to_devlink(adapter);
	int err;

	err = devlink_register(dl, adapter->dev);
	if (err)
		dev_err(adapter->dev, "devlink_register failed: %d\n", err);

	return err;
}
#endif

void dubhe1000_enable_napi(struct dubhe1000_adapter *adapter)
{
	int channel;

	init_dummy_netdev(&adapter->napi_dev);
	for (channel = 0; channel < adapter->num_rx_queues; channel++) {
		netif_napi_add(&adapter->napi_dev, &adapter->edma_rx_napi[channel], dubhe1000_clean,
			       DUBHE1000_NAPI_POLL_WEIGHT);
		napi_enable(&adapter->edma_rx_napi[channel]);
	}

#if (DUBHE1000_TX_OPTI_EN)
	netif_napi_add(&adapter->napi_dev, &adapter->tx_napi, dubhe1000_tx_clean, DUBHE1000_TX_NAPI_POLL_WEIGHT);
	napi_enable(&adapter->tx_napi);
#endif
}

void dubhe1000_disable_napi(struct dubhe1000_adapter *adapter)
{
	int channel;

	for (channel = 0; channel < adapter->num_rx_queues; channel++) {
		napi_disable(&adapter->edma_rx_napi[channel]);
		netif_napi_del(&adapter->edma_rx_napi[channel]);
	}

#if (DUBHE1000_TX_OPTI_EN)
	napi_disable(&adapter->tx_napi);
	netif_napi_del(&adapter->tx_napi);
#endif
}

static int dubhe2000_remove(struct platform_device *pdev)
{
	struct dubhe1000_adapter *adapter = platform_get_drvdata(pdev);

	if (verbose)
		pr_info("[%s] enter\n", __func__);

	if (!adapter) {
		pr_info("[%s]: adapter is null pointer\n", __func__);
		return -1;
	}

	dubhe1000_dbg_exit(adapter);
	dubhe1000_irq_disable_all(adapter);
	dubhe1000_free_irq(adapter);
	dubhe1000_disable_napi(adapter);
	dubhe1000_destroy_ports(adapter);

#ifdef CONFIG_DUBHE2000_DEVLINK
	dubhe1000_mac_pcs_config(adapter, 0); // stop fwd's tx/rx
#endif

	// free alloced memory
	dubhe1000_free_all_rx_resources(adapter);
	dubhe1000_free_all_tx_resources(adapter);
	kfree(adapter->tx_ring);
	kfree(adapter->rx_ring);
	dubhe1000_edma_destroy();

	// soft reset fwd
	dubhe1000_soft_reset_fwd(adapter);

#ifdef CONFIG_DUBHE2000_DEVLINK
	dubhe1000_devlink_unregister(adapter);
#else
	rtk_phy_unregister();
#endif

	g_adapter = NULL;
	cls_skb_free = NULL;
	cls_is_bmu_skb = NULL;
	cls_bmu_dma_addr = NULL;

	pr_info("[%s] Leave\n", __func__);

	return 0;
}

void cls_npe_eth_tx_info_get(bool *soft_bmu_en, dma_addr_t *body_dma, int *vaddr_size, void **hw_data)
{
	if (g_adapter) {
		*soft_bmu_en = g_adapter->soft_bmu_en;
		*body_dma = g_adapter->body_dma;
		*vaddr_size = DUBHE1000_HW_BMU_BUF_MAX(g_adapter);
		*hw_data = g_adapter->hw_data;
	} else {
		*soft_bmu_en = true;
		*body_dma = 0;
		*vaddr_size = 0;
		*hw_data = NULL;
	}
}
EXPORT_SYMBOL(cls_npe_eth_tx_info_get);

#ifdef CONFIG_DUBHE2000_PHYLINK
static void dump_sfp_support(unsigned long *support)
{
	int i = 0;
	static const struct {
		int mode_bit;
		const char *desc;
	} mode_table[] = {
		{ ETHTOOL_LINK_MODE_10000baseSR_Full_BIT, "10GBase-SR" },
		{ ETHTOOL_LINK_MODE_10000baseLR_Full_BIT, "10GBase-LR" },
		{ ETHTOOL_LINK_MODE_10000baseLRM_Full_BIT, "10GBase-LRM" },
		{ ETHTOOL_LINK_MODE_10000baseER_Full_BIT, "10GBase-ER" },
		{ ETHTOOL_LINK_MODE_10000baseCR_Full_BIT, "10GBase-CR" },
		{ ETHTOOL_LINK_MODE_10000baseT_Full_BIT, "10GBase-T" },

		{ ETHTOOL_LINK_MODE_5000baseT_Full_BIT, "5GBase-T" },
		{ ETHTOOL_LINK_MODE_2500baseT_Full_BIT, "2.5GBase-T" },
		{ ETHTOOL_LINK_MODE_2500baseX_Full_BIT, "2.5GBase-X" },

		{ ETHTOOL_LINK_MODE_1000baseX_Full_BIT, "1000Base-X" },
		{ ETHTOOL_LINK_MODE_1000baseT_Full_BIT, "1000Base-T Full" },
		{ ETHTOOL_LINK_MODE_1000baseT_Half_BIT, "1000Base-T Half" },

		{ ETHTOOL_LINK_MODE_100000baseSR4_Full_BIT, "100GBase-SR4" },
		{ ETHTOOL_LINK_MODE_100000baseLR4_ER4_Full_BIT, "100GBase-LR4/ER4" },
		{ ETHTOOL_LINK_MODE_100000baseCR4_Full_BIT, "100GBase-CR4" },
		{ ETHTOOL_LINK_MODE_25000baseSR_Full_BIT, "25GBase-SR" },
		{ ETHTOOL_LINK_MODE_25000baseCR_Full_BIT, "25GBase-CR" },
		{ ETHTOOL_LINK_MODE_Autoneg_BIT, "Autoneg"},
		{ ETHTOOL_LINK_MODE_Pause_BIT, "Pause"},
		{ ETHTOOL_LINK_MODE_Asym_Pause_BIT, "Asym_Pause"}
	};

	for (i = 0; i < ARRAY_SIZE(mode_table); i++)
		if (test_bit(mode_table[i].mode_bit, support))
			pr_err("  %s", mode_table[i].desc);
}

static void dubhe1000_validate(struct phylink_config *config, unsigned long *supported,
			       struct phylink_link_state *state)
{
	struct dubhe1000_mac *port = netdev_priv(to_net_dev(config->dev));
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mac_supported) = { 0, };
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };
	int max_speed = port->max_speed;

	if (verbose) {
		pr_info("%s max_speed[%d]", __func__, max_speed);

		pr_err("support %*pb\n",
				__ETHTOOL_LINK_MODE_MASK_NBITS, supported);
		dump_sfp_support(supported);

		pr_err("support advertising %*pb\n",
				__ETHTOOL_LINK_MODE_MASK_NBITS, state->advertising);
		dump_sfp_support(state->advertising);
	}

	phylink_set(mac_supported, 10baseT_Half);
	phylink_set(mac_supported, 10baseT_Full);
	phylink_set(mac_supported, 100baseT_Half);
	phylink_set(mac_supported, 100baseT_Full);
	phylink_set(mac_supported, 1000baseT_Half);
	phylink_set(mac_supported, 1000baseT_Full);
	phylink_set(mac_supported, 1000baseKX_Full);
	phylink_set(mac_supported, 1000baseX_Full);
	if (!port->is_sfp)
		phylink_set(mac_supported, Autoneg);
	phylink_set(mac_supported, Pause);
	phylink_set(mac_supported, Asym_Pause);
	phylink_set_port_modes(mac_supported);

	/* Cut down 1G if asked to */
	if ((max_speed > 0) && (max_speed < 1000)) {
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseX_Full);
	} else {
		if (!max_speed || (max_speed >= 2500)) {
			phylink_set(mac_supported, 2500baseT_Full);
			phylink_set(mac_supported, 2500baseX_Full);
		}
		if (!max_speed || (max_speed >= 5000))
			phylink_set(mac_supported, 5000baseT_Full);
		if (!max_speed || (max_speed >= 10000)) {
			phylink_set(mac_supported, 10000baseSR_Full);
			phylink_set(mac_supported, 10000baseLR_Full);
			phylink_set(mac_supported, 10000baseER_Full);
			phylink_set(mac_supported, 10000baseLRM_Full);
			phylink_set(mac_supported, 10000baseT_Full);
			phylink_set(mac_supported, 10000baseKX4_Full);
			phylink_set(mac_supported, 10000baseKR_Full);
		}
		if (!max_speed || (max_speed >= 25000)) {
			phylink_set(mac_supported, 25000baseCR_Full);
			phylink_set(mac_supported, 25000baseKR_Full);
			phylink_set(mac_supported, 25000baseSR_Full);
		}

		if (!max_speed || (max_speed >= 50000)) {
			phylink_set(mac_supported, 50000baseCR2_Full);
			phylink_set(mac_supported, 50000baseKR2_Full);
			phylink_set(mac_supported, 50000baseSR2_Full);
			phylink_set(mac_supported, 50000baseKR_Full);
			phylink_set(mac_supported, 50000baseSR_Full);
			phylink_set(mac_supported, 50000baseCR_Full);
			phylink_set(mac_supported, 50000baseLR_ER_FR_Full);
			phylink_set(mac_supported, 50000baseDR_Full);
		}
		if (!max_speed || (max_speed >= 100000)) {
			phylink_set(mac_supported, 100000baseKR4_Full);
			phylink_set(mac_supported, 100000baseSR4_Full);
			phylink_set(mac_supported, 100000baseCR4_Full);
			phylink_set(mac_supported, 100000baseLR4_ER4_Full);
			phylink_set(mac_supported, 100000baseKR2_Full);
			phylink_set(mac_supported, 100000baseSR2_Full);
			phylink_set(mac_supported, 100000baseCR2_Full);
			phylink_set(mac_supported, 100000baseLR2_ER2_FR2_Full);
			phylink_set(mac_supported, 100000baseDR2_Full);
		}
	}

	linkmode_and(supported, supported, mac_supported);

	if (verbose) {
		pr_err("support1 %*pb\n",
				__ETHTOOL_LINK_MODE_MASK_NBITS, supported);
		dump_sfp_support(supported);
	}

	linkmode_andnot(supported, supported, mask);

	if (verbose) {
		pr_err("support2 %*pb\n",
				__ETHTOOL_LINK_MODE_MASK_NBITS, supported);
		dump_sfp_support(supported);
		}

	linkmode_and(state->advertising, state->advertising, mac_supported);
	linkmode_andnot(state->advertising, state->advertising, mask);

	if (verbose) {
		pr_err("state->advertising %*pb\n",
				__ETHTOOL_LINK_MODE_MASK_NBITS, state->advertising);
		dump_sfp_support(state->advertising);
		pr_err("%s MAC[%d] interface %s\n",
				__func__, port->id, phy_modes(state->interface));
	}
	/* If PCS is supported, check which modes it supports. */
}

static void dubhe1000_mac_pcs_get_state(struct phylink_config *config, struct phylink_link_state *state)
{
	pr_info("%s ", __func__);
}

static void dubhe1000_mac_link_up(struct phylink_config *config, struct phy_device *phy, unsigned int mode,
				  phy_interface_t interface, int speed, int duplex, bool tx_pause, bool rx_pause)
{
	struct dubhe1000_mac *port = netdev_priv(to_net_dev(config->dev));
	struct dubhe1000_adapter *adapter = port->adapter;

	printk(KERN_ERR "%s Is_phy: %s mac %d pcs %d pcs_port %d, speed %d interface %s",
			__func__, port->phydev ? "True" : "False",
			port->id, port->pcs_sel,
			port->pcs_port, speed, phy_modes(port->phy_interface));

	if (port->phydev || of_phy_is_fixed_link(port->phylink_node)) {
		if (port->phy_interface == PHY_INTERFACE_MODE_SGMII ||
				port->phy_interface == PHY_INTERFACE_MODE_1000BASEX ||
				port->phy_interface == PHY_INTERFACE_MODE_2500BASEX) {
			int pcs_index = 0, phy_interface = 0;

		pcs_index = port->pcs_sel;

		if (speed == SPEED_2500)
			phy_interface = PHY_INTERFACE_MODE_2500BASEX;
		else
			phy_interface = PHY_INTERFACE_MODE_SGMII;

		if (phy_interface != port->phy_interface) {
			adapter->pcs[pcs_index]->phy_interface = phy_interface;
			port->phy_interface = phy_interface;
			dubhe1000_set_pcs_mode(adapter->pcs[pcs_index]);
		}

		dubhe1000_xpcs_clear_an_interrupt(adapter->pcs[port->pcs_sel], port->pcs_port);

		dubhe1000_xpcs_speed(adapter->pcs[pcs_index], port->pcs_port, speed);

		} else if (port->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
				port->phy_interface == PHY_INTERFACE_MODE_10GBASER) {
			dubhe1000_xpcs_clear_an_interrupt(adapter->pcs[port->pcs_sel], port->pcs_port);
			dubhe1000_xpcs_speed(adapter->pcs[port->pcs_sel], port->pcs_port, speed);
			dubhe1000_usxgmii_reset_rate_adapter(adapter->pcs[port->pcs_sel], port->pcs_port);
		}
	}

	if (verbose)
		pr_info("%s mac_link_up speed is [%d] max_speed [%d]", __func__, speed, port->max_speed);

	if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
		struct phylink_link_state state;

		dubhe1000_xpcs_get_link_state(adapter->pcs[port->pcs_sel], port->pcs_port, &state);

		pr_info("Get PCS%d port%d state type:%d:%s (an_enabled:%d  an_complete:%d link:%d duplex:%d speed:%d)",
				port->pcs_sel, port->pcs_port, state.interface,
				phy_modes(state.interface), state.an_enabled,
				state.an_complete, state.link,
			state.duplex, state.speed);
	}

	dubhe1000_enable_xgmac(port, false);

	//set auto-ne
	dubhe1000_xgmac_speed(port, speed);

	/* Flow Control operation */
	if (tx_pause && rx_pause)
		dubhe1000_xgmac_flow_ctrl(port, duplex);

	dubhe1000_xgmac_duplex(port, duplex);

	dubhe1000_enable_xgmac(port, true);

	if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII)
		dubhe1000_xpcs_enable(adapter->pcs[port->pcs_sel], port->pcs_port, 1);

	dubhe1000_switch_drain_port(adapter, port->id, 0);
}

static void dubhe1000_mac_config(struct phylink_config *config, unsigned int mode,
				 const struct phylink_link_state *state)
{
	if (verbose)
		pr_info("%s: state: %d:%s", __func__, state->interface, phy_modes(state->interface));
}

static void dubhe1000_mac_an_restart(struct phylink_config *config)
{
	if (verbose)
		pr_info("%s  entry ", __func__);
	/* Not Supported */
}

static void dubhe1000_mac_link_down(struct phylink_config *config, unsigned int mode, phy_interface_t interface)
{
	struct dubhe1000_mac *port;
	struct dubhe1000_adapter *adapter;

	if (verbose)
		pr_info("%s  entry ", __func__);
	port = netdev_priv(to_net_dev(config->dev));
	adapter = port->adapter;

	dubhe1000_switch_drain_port(adapter, port->id, true);

	if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII)
		dubhe1000_xpcs_enable(adapter->pcs[port->pcs_sel], port->pcs_port, 1);

	dubhe1000_enable_xgmac(port, false);
}

void dubhe1000_pcs_get_state(struct phylink_pcs *pcs, struct phylink_link_state *state)
{
	int pcs_index;
	struct dubhe1000_mac *port = dubhe1000_pcs_to_port(pcs);
	struct dubhe1000_adapter *adapter = port->adapter;

	if (verbose)
		pr_info("%s entry", __func__);

	pcs_index = port->pcs_sel;

	dubhe1000_xpcs_get_link_state(adapter->pcs[pcs_index], port->pcs_port, state);

	if (adapter->pcs[pcs_index]->link != state->link) {
		dubhe1000_switch_drain_port(adapter, port->id, !state->link);
		adapter->pcs[pcs_index]->link = state->link;
	}

	if (verbose) {
		pr_info("Get PCS%d port%d state type:%d (an_enabled:%d  an_complete:%d link:%d duplex:%d speed:%d)",
			pcs_index, port->pcs_port, state->interface, state->an_enabled, state->an_complete, state->link,
			state->duplex, state->speed);
	}
}

int dubhe1000_pcs_config(struct phylink_pcs *pcs, unsigned int mode, phy_interface_t interface,
			 const unsigned long *advertising, bool permit_pause_to_mac)
{
	struct dubhe1000_mac *port = dubhe1000_pcs_to_port(pcs);
	struct dubhe1000_adapter *adapter = port->adapter;

	if (verbose)
		pr_info("%s  entry ", __func__);

	if (port->is_sfp == 2) {
		int speed = SPEED_1000;

		if (interface == PHY_INTERFACE_MODE_SGMII ||
				interface == PHY_INTERFACE_MODE_1000BASEX)
			speed = SPEED_1000;
		else if (interface == PHY_INTERFACE_MODE_2500BASEX)
			speed = SPEED_2500;
		else if (interface == PHY_INTERFACE_MODE_10GBASER)
			speed = 10000;

		adapter->pcs[port->pcs_port]->phy_interface = interface;
		port->phy_interface = interface;
		port->speed = speed;
		dubhe1000_set_pcs_mode(adapter->pcs[port->pcs_port]);
		dubhe1000_xpcs_speed(adapter->pcs[port->pcs_port], port->pcs_port, speed);
		printk(KERN_ERR "%s: MAC%d interface %s pcs %d pcs_port %d speed %d",
				__func__,
				port->id,
				phy_modes(interface),
				port->pcs_sel,
				port->pcs_port,
				speed);
	}
	return 0;
}

void dubhe1000_pcs_an_restart(struct phylink_pcs *pcs)
{
	int pcs_index;
	struct dubhe1000_mac *port = dubhe1000_pcs_to_port(pcs);
	struct dubhe1000_adapter *adapter = port->adapter;

	pr_info("%s  entry ", __func__);

	pcs_index = port->pcs_sel;

	dubhe1000_xpcs_an_restart(adapter->pcs[pcs_index], port->pcs_port);
}

static const struct phylink_pcs_ops dubhe1000_pcs_ops = {
	.pcs_get_state = dubhe1000_pcs_get_state,
	.pcs_config = dubhe1000_pcs_config,
	.pcs_an_restart = dubhe1000_pcs_an_restart,
};

int dubhe1000_mac_prepare(struct phylink_config *config, unsigned int mode, phy_interface_t iface)
{
	int pcs_index;
	struct dubhe1000_pcs *pcs;
	struct dubhe1000_mac *port = netdev_priv(to_net_dev(config->dev));
	struct dubhe1000_adapter *adapter = port->adapter;

	pcs_index = port->pcs_sel;

	pcs = adapter->pcs[pcs_index];

	if (port->interface == XGMAC_INTERFACE_MODE_RGMII)
		return 0;

	if ((port->pcs_port == 0) || (port->phy_interface == PHY_INTERFACE_MODE_QSGMII) ||
	    ((port->phy_interface == PHY_INTERFACE_MODE_USXGMII) &&
	     ((pcs->usxg_mode == USXG_MODE_10G_QXGMII) ||
	      ((pcs->usxg_mode == USXG_MODE_10G_DXGMII) ||
	       ((pcs->usxg_mode == USXG_MODE_5G_DXGMII) && (port->pcs_port < 2)))))) {
		port->phylink_pcs.ops = &dubhe1000_pcs_ops;
		phylink_set_pcs(port->phylink, &port->phylink_pcs);
	}

	return 0;
}

static const struct phylink_mac_ops dubhe1000_phylink_mac_ops = {
	.validate = dubhe1000_validate,
	.mac_prepare = dubhe1000_mac_prepare,
	.mac_pcs_get_state = dubhe1000_mac_pcs_get_state,
	.mac_config = dubhe1000_mac_config,
	.mac_an_restart = dubhe1000_mac_an_restart,
	.mac_link_down = dubhe1000_mac_link_down,
	.mac_link_up = dubhe1000_mac_link_up,
};

static void fix_rgmii_war(struct dubhe1000_adapter *adapter, int port)
{
	writel(1, adapter->top_regs + 0x200 + (port * 8));
}

/**
 * dubhe1000_init_phy - PHY initialization
 * @dev: net device structure
 * Description: it initializes the driver's PHY state, and attaches the PHY
 * to the mac driver.
 *  Return value:
 *  0 on success
 */
static int dubhe1000_init_phy(struct net_device *dev)
{
	struct dubhe1000_mac *port = netdev_priv(dev);
	struct dubhe1000_adapter *adapter = port->adapter;
	struct device_node *node;
	struct phy_device *phydev = NULL;
	int ret = 0;

	node = port->phylink_node;

	/*Get DTS phy config*/
	if (node) {
		ret = phylink_of_phy_connect(port->phylink, node, 0);
		if (!ret) {
			struct device_node *phy_node;

			phy_node = of_parse_phandle(node, "phy-handle", 0);
			if (phy_node) {
				phydev = of_phy_find_device(phy_node);
				port->phydev = phydev;
			}
		}
		if (verbose)
			pr_info("phylink_of_phy_connect ret[%d]\n", ret);
	}

	/* Some DT bindings do not set-up the PHY handle. Let's try to
	 * manually parse it
	 */
	if (adapter->mdio[port->id] && (!node || ret)) {
		int addr = port->phy_addr;

		phydev = mdiobus_get_phy(adapter->mdio[port->id]->bus, addr);
		if (!phydev) {
			pr_info("no phy at addr %d\n", addr);
			return -ENODEV;
		}

		ret = phylink_connect_phy(port->phylink, phydev);
	}

	if (phydev) {
		if (phydev->phy_id == 0x1cc916) { //RTL8211F
			printk(KERN_ERR "IS RGMII RTL8211F!");
			fix_rgmii_war(adapter, port->id);
		} else if (phydev->phy_id == 0x4f51e91b) { //YT8531
			phy_write(phydev, 0x1e, 0xa003);
			phy_write(phydev, 0x1f, 0xff);
			phy_write(phydev, 0x1e, 0x0);
			printk(KERN_ERR "IS RGMII YT8531!");
		} else if (phydev->phy_id == 0x001ccad0) { //RTL8224
			phy_write(phydev, 0x0, 0x8000);
		} else if (phydev->phy_id == 0x4f51ea19) { //YT8821
			if (adapter->pcs[port->pcs_sel])
				adapter->pcs[port->pcs_sel]->hsgmii_autoneg = HSGMII_AUTONEG_OFF;
		}
	}
	return ret;
}

static int dubhe1000_phylink_setup(struct dubhe1000_mac *port)
{
	struct fwnode_handle *fwnode = of_fwnode_handle(port->phylink_node);
	int mode = port->phy_interface;
	struct phylink *phylink;

	port->phylink_config.dev = &port->netdev->dev;
	port->phylink_config.type = PHYLINK_NETDEV;
	port->phylink_config.pcs_poll = true;

	if (!fwnode)
		fwnode = dev_fwnode(port->device);

	phylink = phylink_create(&port->phylink_config, fwnode, mode, &dubhe1000_phylink_mac_ops);
	if (IS_ERR(phylink))
		return PTR_ERR(phylink);

	port->phylink = phylink;
	return 0;
}
#endif /*CONFIG_DUBHE2000_PHYLINK*/

/**
 * dubhe2000_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in dubhe1000_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * dubhe1000_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
static int dubhe2000_probe(struct platform_device *pdev)
{
#define GET_RESOURCE_BYNAME(reg_name) ({                                                                               \
	reg_index = of_property_match_string(np, "reg-names", #reg_name);                                              \
	if (reg_index < 0) {                                                                                           \
		pr_info("Can't find DTS reg[%s]\n", #reg_name);                                                        \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	err = of_address_to_resource(np, reg_index, &res);                                                             \
	if (err < 0) {                                                                                                 \
		pr_info("of_address_to_resource reg_name[%s] err = %d\n", #reg_name, err);                             \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] Resources start[%#llx]\n", #reg_name, res.start);                                     \
	res;                                                                                                           \
})

#define IOREMAP_BYNAME(reg_name) ({                                                                                    \
	reg_index = of_property_match_string(np, "reg-names", #reg_name);                                              \
	if (reg_index < 0) {                                                                                           \
		pr_info("Can't find DTS reg[%s]\n", #reg_name);                                                        \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	err = of_address_to_resource(np, reg_index, &res);                                                             \
	if (err < 0) {                                                                                                 \
		pr_info("of_address_to_resource reg_name[%s] err = %d\n", #reg_name, err);                             \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] Resources start[%#llx]\n", #reg_name, res.start);                                     \
	ioaddr = devm_ioremap_resource(&pdev->dev, &res);                                                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] ioremap[0x%px]\n", #reg_name, ioaddr);                                                \
	if (IS_ERR(ioaddr)) {                                                                                          \
		pr_info("devm_ioremap_resource reg_name[%s] err = 0x%px\n", #reg_name, ioaddr);                        \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	ioaddr;                                                                                                        \
})

#define MEMREMAP_BYNAME(reg_name) ({                                                                                   \
	reg_index = of_property_match_string(np, "reg-names", #reg_name);                                              \
	if (reg_index < 0) {                                                                                           \
		pr_info("Can't find DTS reg[%s]\n", #reg_name);                                                        \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	err = of_address_to_resource(np, reg_index, &res);                                                             \
	if (err < 0) {                                                                                                 \
		pr_info("of_address_to_resource reg_name[%s] err = %d\n", #reg_name, err);                             \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] Resources start[%#llx]\n", #reg_name, res.start);                                     \
	memaddr = devm_memremap(&pdev->dev, res.start, resource_size(&res), MEMREMAP_WC);                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] memremap[0x%px]\n", #reg_name, memaddr);                                              \
	if (IS_ERR(memaddr)) {                                                                                         \
		pr_info("devm_memremap name[%s] err = 0x%px\n", #reg_name, memaddr);                                   \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	memaddr;                                                                                                       \
})

#define MEMREMAP_BYNAME_WB(reg_name) ({                                                                                \
	reg_index = of_property_match_string(np, "reg-names", #reg_name);                                              \
	if (reg_index < 0) {                                                                                           \
		pr_info("Can't find DTS reg[%s]\n", #reg_name);                                                        \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	err = of_address_to_resource(np, reg_index, &res);                                                             \
	if (err < 0) {                                                                                                 \
		pr_info("of_address_to_resource reg_name[%s] err = %d\n", #reg_name, err);                             \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] Resources start[%#llx]\n", #reg_name, res.start);                                     \
	memaddr = devm_memremap(&pdev->dev, res.start, resource_size(&res), MEMREMAP_WB);                              \
	if (verbose)                                                                                                   \
		pr_info("reg[%s] memremap[0x%px]\n", #reg_name, memaddr);                                              \
	if (IS_ERR(memaddr)) {                                                                                         \
		pr_info("devm_memremap name[%s] err = 0x%px\n", #reg_name, memaddr);                                   \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	memaddr;                                                                                                       \
})

#define GET_IRQ_BYNAME(name) ({                                                                                        \
	irq = platform_get_irq_byname(pdev, #name);                                                                    \
	if (irq < 0) {                                                                                                 \
		pr_info("No DBERR IRQ resource irq[%s] %d\n", #name, irq);                                             \
		err = -ENOMEM;                                                                                         \
		goto err_sw_init;                                                                                      \
	}                                                                                                              \
	irq;                                                                                                           \
})

	struct device_node *np = pdev->dev.of_node;
	struct dubhe1000_adapter *adapter = NULL;
	struct device_node *child_np;
	struct resource res;
	int addr_width = 32;
	u32 page_num = 0;
	void __iomem *ioaddr;
	void *memaddr;

	int err, using_dac, reg_index, irq;
	u32 soft_bmu = 5;
	u32 tmp[6] = { 0 };
	u32 split_mode = 0, qos = 0, pause_mod = 0;

	if (verbose)
		pr_info("[%s] enter\n", __func__);

		// 1. netdev config
#ifdef CONFIG_DUBHE2000_PHYLINK
	adapter = devm_kzalloc(&pdev->dev, sizeof(*adapter), GFP_KERNEL);
	if (!adapter)
		return -ENOMEM;
#elif defined(CONFIG_DUBHE2000_DEVLINK)
	adapter = dubhe1000_devlink_alloc();
	if (!adapter)
		return -ENOMEM;
#else
#error "Please choice a phy management method"
#endif

	g_adapter = adapter;

	if (of_property_read_u32(np, "max-ports", &adapter->port_count) != 0)
		adapter->port_count = DUBHE1000_MAC_COUNT;

	platform_set_drvdata(pdev, adapter);

	adapter->dev = &pdev->dev;

#ifdef CONFIG_DUBHE2000_DEVLINK
	dubhe1000_devlink_register(adapter);
#endif

	adapter->pdev = pdev;
#ifdef NPE_DEBUG_DEFAULT_ON
	adapter->msg_enable = netif_msg_init(debug, DEFAULT_MSG_ENABLE | NETIF_MSG_PKTDATA);
#else
	adapter->msg_enable = 0;
#endif

	adapter->max_frame_size = ETH_DATA_LEN + ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;
	adapter->min_frame_size = MINIMUM_ETHERNET_FRAME_SIZE;

	/* interrupts = <0 102 1>;
	 *  param@0:
	 *       0: not SPI (shared peripheral interrupt)
	 *       1: SPI interrupt
	 */
	adapter->switch_irq = GET_IRQ_BYNAME(switch);
	adapter->tx_irq = GET_IRQ_BYNAME(host_tx);

	adapter->rx_intr_list[0] = GET_IRQ_BYNAME(host_rx);
	adapter->rx_intr_list[1] = GET_IRQ_BYNAME(wifi_40M_rx);
	adapter->rx_intr_list[2] = GET_IRQ_BYNAME(wifi_160M_rx);
	adapter->rx_intr_list[3] = GET_IRQ_BYNAME(pcie_rx);

#if defined(__ENABLE_FWD_TEST__)
	adapter->tx_intr_list[0] = adapter->tx_irq;
	adapter->tx_intr_list[1] = GET_IRQ_BYNAME(wifi_40M_tx);
	adapter->tx_intr_list[2] = GET_IRQ_BYNAME(wifi_160M_tx);
	adapter->tx_intr_list[3] = GET_IRQ_BYNAME(pcie_tx);
#endif

	if ((adapter->rx_intr_list[0] <= 0) || (adapter->tx_irq <= 0)) {
		dev_err(&pdev->dev, "could not determine irqs (rx_irq %d tx_irq %d)\n", adapter->rx_intr_list[0],
			adapter->tx_irq);
		err = -ENOMEM;

		goto err_sw_init;
	}

	adapter->status_err_irq	= GET_IRQ_BYNAME(status_err);
	adapter->lmx_irq	= GET_IRQ_BYNAME(lmx);
	adapter->xpcs1_irq	= GET_IRQ_BYNAME(xpcs1);
	adapter->xpcs0_irq	= GET_IRQ_BYNAME(xpcs0);

	adapter->xgmac_irq[0]	= GET_IRQ_BYNAME(xgmac0);
	adapter->xgmac_irq[1]	= GET_IRQ_BYNAME(xgmac1);
	adapter->xgmac_irq[2]	= GET_IRQ_BYNAME(xgmac2);
	adapter->xgmac_irq[3]	= GET_IRQ_BYNAME(xgmac3);
	adapter->xgmac_irq[4]	= GET_IRQ_BYNAME(xgmac4);

	pr_info("switch_irq=%d tx_irq=%d rx_irq=%d xpcs1_irq=%d xpcs0_irq=%d status_err_irq=%d lmx_irq=%d\n",
		adapter->switch_irq, adapter->tx_irq, adapter->rx_intr_list[0], adapter->xpcs1_irq, adapter->xpcs0_irq,
		adapter->status_err_irq, adapter->lmx_irq);

	printk(KERN_ERR "rx_intr_list[%d %d %d %d]\n",
	       adapter->rx_intr_list[0], adapter->rx_intr_list[1], adapter->rx_intr_list[2], adapter->rx_intr_list[3]);

#if defined(__ENABLE_FWD_TEST__)
	printk(KERN_ERR "tx_intr_list[%d %d %d %d]\n",
	       adapter->tx_intr_list[0], adapter->tx_intr_list[1], adapter->tx_intr_list[2], adapter->tx_intr_list[3]);
#endif

	/* Get the address of the memory mapped registers */
	adapter->edma_regs	= IOREMAP_BYNAME(edma);
	adapter->edma_dbg_regs	= IOREMAP_BYNAME(edma_dbg);
	adapter->lmx_regs	= IOREMAP_BYNAME(lmx);
	adapter->aging0_regs	= IOREMAP_BYNAME(aging0);
	adapter->aging1_regs	= IOREMAP_BYNAME(aging1);
	adapter->switch_regs	= IOREMAP_BYNAME(switch);
	adapter->top_regs	= IOREMAP_BYNAME(top);
	adapter->io_dr_regs	= IOREMAP_BYNAME(io_dr);
	adapter->io_left_regs	= IOREMAP_BYNAME(io_left);
	adapter->io_right_regs	= IOREMAP_BYNAME(io_right);
	adapter->xgmac_pll_regs	= GET_RESOURCE_BYNAME(xgmac_pll);

	/* <0x00 0x50001000 0x00 0x1FFF>;    Memory Region Controller */
	adapter->mrc_regs = IOREMAP_BYNAME(mrc);
	adapter->mrc_dma = res.start;

	adapter->acp_shaper_regs = IOREMAP_BYNAME(acp_shaper);
	if (IS_ENABLED(CONFIG_ARM_CCI) && cci_ctrl_base) {
		if (verbose)
			pr_info("get cci_regs from arm-cci module\n");
		adapter->cci_regs = (void *)cci_ctrl_base;
	} else {
		adapter->cci_regs = IOREMAP_BYNAME(cci);
	}

	if (verbose)
		pr_info("mrc_regs 0x%px, acp_shaper_regs 0x%px, cci_regs 0x%px\n",
			adapter->mrc_regs, adapter->acp_shaper_regs, adapter->cci_regs);

	/* <0x00 0x01000000 0x00 0x200000>;    HW-BMU Head Reserved Address 2M */
	adapter->head_hw_data = MEMREMAP_BYNAME_WB(head_pool);
	adapter->head_dma = res.start;

	if (verbose)
		pr_info(" HW-BMU Head buff 0x%px dma %llx len %lld\n", adapter->head_hw_data, adapter->head_dma,
			res.end - adapter->head_dma);

	/* <0x00 0x01200000 0x00 0x1000000>;	HW-BMU Body Reserved Address  16M */
	adapter->hw_data = MEMREMAP_BYNAME_WB(body_pool);
	adapter->body_dma = res.start;

	if (verbose)
		pr_info(" HW-BMU Body buff 0x%px dma %llx len %lld\n", adapter->hw_data, adapter->body_dma,
			res.end - adapter->body_dma);

	adapter->num_tx_queues = 1;
	adapter->num_rx_queues = 4;

	/* <0x00 0x02200000 0x00 0x10000>;	HW-BMU TXQ status Reserved Address 64k (32 * 256 num) */
	GET_RESOURCE_BYNAME(bmu_txq);

	adapter->txq_status_dma = res.start;

	if (of_property_read_u32_array(np, "txq-allocate", tmp, 4) == 0) {
		int i;
		u32 total_nums = resource_size(&res) / 8;
		u32 actual_size = 0;

		for (i = 0; i < 4; i++) {
			if (tmp[i]) {
				adapter->txq_status_num[i] = total_nums * tmp[i] /
					(100 * DEFAULT_STATUS_PKT_NUM) * DEFAULT_STATUS_PKT_NUM;
				if (adapter->txq_status_num[i] < (DEFAULT_STATUS_PKT_NUM * 4))
					adapter->txq_status_num[i] = 4 * DEFAULT_STATUS_PKT_NUM;
			} else
				adapter->txq_status_num[i] = 0;

			actual_size += adapter->txq_status_num[i];
		}

		if (actual_size > total_nums) {
			printk(KERN_ERR"TXQ-DESCR num(%d) must <= %d\n", actual_size, total_nums);
			goto err_sw_init;
		}

		adapter->edma_rx_limit[0] = 1024;
		adapter->edma_rx_limit[1] = 1024;
		adapter->edma_rx_limit[2] = 2048;
		adapter->edma_rx_limit[3] = 1024;

	} else
		adapter->txq_status_num[0] = resource_size(&res) / 8;

	pr_info("hw-bmu txq_regs: 0x%llx 0x%llx txq num %d:%d:%d:%d\n",
		res.start, resource_size(&res),
		adapter->txq_status_num[0], adapter->txq_status_num[1],
		adapter->txq_status_num[2], adapter->txq_status_num[3]);

	adapter->fwd_pll_regs	= IOREMAP_BYNAME(fwd_pll);
	adapter->rst_para_regs	= IOREMAP_BYNAME(reset);
	adapter->cg_para_regs	= IOREMAP_BYNAME(CG);

	/* there is a workaround being applied below that limits
	 * 64-bit DMA addresses to 64-bit hardware.  There are some
	 * 32-bit adapters that Tx hang when given 64-bit DMA addresses
	 */
	using_dac = 0;
	if (!dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(addr_width))) {
		using_dac = 1;
	} else {
		err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(addr_width));
		if (err) {
			pr_err("No usable DMA config, aborting\n");
			goto err_dma;
		}
	}

	//foreach node
	for_each_child_of_node(np, child_np) {
		if (of_device_is_compatible(child_np, "clourney,dwmac")) {
			if (!of_device_is_available(child_np))
				continue;

			err = dubhe1000_mac_create(adapter, child_np);
			if (err) {
				of_node_put(child_np);
				goto err_port_init;
			}

		}
#ifdef CONFIG_DUBHE2000_PHYLINK
		else if (of_device_is_compatible(child_np, "clourney,xpcs")) {

			if (!of_device_is_available(child_np))
				continue;

			err = dubhe1000_pcs_create(adapter, child_np);
			if (err) {
				of_node_put(child_np);
				goto err_port_init;
			}
		} else if (of_device_is_compatible(child_np, "clourney,mdio")) {
			uint32_t mdio_id;

			if (!of_device_is_available(child_np))
				continue;

			if (of_property_read_u32(child_np, "id", &mdio_id)) {
				pr_err("%s:%d Not find MDIO id!", __func__, __LINE__);
				continue;
			}

			if (mdio_id >= DUBHE1000_MDIO_COUNT) {
				pr_err("%s:%d ERR MDIO id:%d >= %d!!", __func__, __LINE__, mdio_id,
				       DUBHE1000_MDIO_COUNT);
				continue;
			}

			if (adapter->mdio_node[mdio_id]) {
				pr_err("%s:%d ERR MDIO%d already exist!!", __func__, __LINE__, mdio_id);
				continue;
			}

			adapter->mdio_node[mdio_id] = child_np;

			if (verbose)
				pr_err("Get mdio%d in root dts!!!", mdio_id);
		}
#endif
	}

#ifdef CONFIG_DUBHE2000_PHYLINK
	err = dubhe1000_interfaces_validate(adapter);
	if (err)
		goto err_port_init;
#endif

	// enable napi to poll the rx handle function
	dubhe1000_enable_napi(adapter);

	// 5. init edma by params which are from the process of loading module
	if (of_property_read_u32(np, "soft-bmu", &soft_bmu) == 0)
		adapter->soft_bmu_en = (soft_bmu == 1) ? true : false;
	else
		adapter->soft_bmu_en = false;

	if (of_property_read_u32(np, "qos", &qos) == 0)
		adapter->token_fc_en = (qos == 1) ? true : false;
	else
		adapter->token_fc_en = false;

	adapter->head_awcachable = true;
	adapter->body_awcachable = false;
	/* 0: no split, 1: tag split, 2: l2 header, 3: l3 header, 4: l4 header*/
	if (of_property_read_u32(np, "split-mode", &split_mode) == 0)
		adapter->split_mode = split_mode;
	else
		adapter->split_mode = 0;

	if (of_property_read_u32(np, "page-num", &page_num) == 0)
		adapter->page_num = page_num;
	else
		adapter->page_num = DUBHE1000_PAGE_NUM;

	// granularity: 16, ex. the FIFO size = 16*16=256
	if (of_property_read_u32_array(np, "fifo-depth", tmp, 4) == 0) {
		adapter->lhost_fifo_depth = tmp[0];
		adapter->wmac_24g_fifo_depth = tmp[1];
		adapter->wmac_5g_fifo_depth = tmp[2];
		adapter->pcie_fifo_depth = tmp[3];
	} else {
		adapter->lhost_fifo_depth = DUBHE1000_LHOST_DEFAULT_FIFO_DEPTH;
		adapter->wmac_24g_fifo_depth = DUBHE1000_WMAC_24G_DEFAULT_FIFO_DEPTH;
		adapter->wmac_5g_fifo_depth = DUBHE1000_WMAC_5G_DEFAULT_FIFO_DEPTH;
		adapter->pcie_fifo_depth = DUBHE1000_PCIE_DEFAULT_FIFO_DEPTH;
	}

	if (of_property_read_u32_array(np, "offset", tmp, 2) == 0) {
		adapter->head_offset = tmp[0];
		adapter->body_offset = tmp[1];
	} else {
		adapter->head_offset = DUBHE1000_DEFAULT_HEAD_OFFSET;
		adapter->body_offset = DUBHE1000_DEFAULT_BODY_OFFSET;
	}
	if (of_property_read_u32_array(np, "block-size", tmp, 2) == 0) {
		adapter->head_block_size = tmp[0];
		adapter->body_block_size = tmp[1];
	} else {
		adapter->head_block_size = DUBHE1000_HEAD_BLOCK_SIZE;
		adapter->body_block_size = DUBHE1000_BODY_BLOCK_SIZE;
	}

	adapter->hw_bmu_buf_size = (adapter->page_num + 1) * 256 * adapter->body_block_size;

	if (switch_pause_mod >= 0 && switch_pause_mod <= dubhe2000_max_pause_mod())
		adapter->switch_pause_mod = switch_pause_mod;
	else if (of_property_read_u32(np, "switch-pause", &pause_mod) == 0 && pause_mod <= dubhe2000_max_pause_mod())
		adapter->switch_pause_mod = pause_mod;
	else
		adapter->switch_pause_mod = 0;

	adapter->switch_pure_mod = switch_pure_mod;

	if (verbose) {
		pr_info("port_count %d\n", adapter->port_count);
		pr_info("soft_bmu_en %d\n", adapter->soft_bmu_en);
		pr_info("token-en %d\n", adapter->token_fc_en);
		pr_info("split-mode %d\n", adapter->split_mode);
		pr_info("page-num %d\n", adapter->page_num);
		pr_info("default fifo depth %d %d %d %d\n", adapter->lhost_fifo_depth, adapter->wmac_24g_fifo_depth,
			adapter->wmac_5g_fifo_depth, adapter->pcie_fifo_depth);

		pr_info("offset (head %d body %d)\n", adapter->head_offset, adapter->body_offset);

		pr_info("block-size (head %d body %d)\n", adapter->head_block_size, adapter->body_block_size);

		pr_info("hw-bmu-size %d\n", adapter->hw_bmu_buf_size);

		pr_info("switch pause mode %d\n", adapter->switch_pause_mod);
		pr_info("switch pure mode %d\n", adapter->switch_pure_mod);
	}
#ifdef CONFIG_DUBHE2000_PHYLINK
	rtk_phy_register();
#endif

	// soft reset fwd_sub
	dubhe1000_init_interfaces(adapter);
#ifndef DISABLE_TRANSMIT
	dubhe1000_edma_init(adapter);
	// 6. rx_ring, tx_ring malloc
	err = dubhe1000_sw_init(adapter);
	if (err)
		goto err_sw_init;
#endif
	/* initialize the wol settings */
	adapter->wol = 0;
#ifndef DISABLE_TRANSMIT
	device_set_wakeup_enable(adapter->dev, adapter->wol);

	err = dubhe1000_setup_all_tx_resources(adapter);
	if (err)
		goto err_register;

	spin_lock_init(&adapter->tx_lock);
	spin_lock_init(&adapter->stats64_lock);

	err = dubhe1000_setup_all_rx_resources(adapter);
	if (err)
		goto err_setup_rx;

	dubhe1000_configure(adapter);

	cls_skb_free = dubhe1000_skb_free;
	cls_is_bmu_skb = dubhe1000_is_bmu_skb;
	cls_bmu_dma_addr = dubhe1000_cls_bmu_dma_addr;
	pr_err("cls_bmu_dma_addr=%px\n", cls_bmu_dma_addr);

	adapter->tx_poll_start = false;

	dubhe1000_poll(adapter);

	adapter->wan_port = 0; /* port 0 is WAN port by default */
	dubhe1000_switch_init(adapter);

#ifdef CONFIG_DUBHE2000_DEVLINK
	dubhe1000_mac_pcs_config(adapter, 1);
#endif

	err = dubhe1000_request_irq(adapter);
	if (err)
		goto err_req_irq;

	dubhe1000_irq_enable_all(adapter);

#endif /*DISABLE_TRANSMIT*/
	dubhe1000_dbg_init(adapter);

	adapter->loopback_mode = 0;

	adapter->enable_from_cpu_tag = 1;
	adapter->tc_debug = 0;
	adapter->switch_cfg_delay = DUBHE2000_SWITCH_CFG_DELAY_DEFAULT;

	dubhe1000_init_ports(adapter);

	return 0;

err_req_irq:
	dubhe1000_free_all_rx_resources(adapter);
err_setup_rx:
	dubhe1000_free_all_tx_resources(adapter);

err_register:
	kfree(adapter->tx_ring);
	kfree(adapter->rx_ring);
err_dma:
err_sw_init:
err_port_init:
#ifdef CONFIG_DUBHE2000_DEVLINK
	dubhe1000_devlink_unregister(adapter);
#else
	rtk_phy_unregister();
#endif

	dubhe1000_soft_reset_fwd(adapter);

	return err;
}

/**
 * dubhe1000_sw_init - Initialize general software structures (struct dubhe1000_adapter)
 * @adapter: board private structure to initialize
 *
 * dubhe1000_sw_init initializes the Adapter private data structure.
 **/
static int dubhe1000_sw_init(struct dubhe1000_adapter *adapter)
{
	adapter->rx_buffer_len = MAXIMUM_ETHERNET_VLAN_SIZE;

	if (dubhe1000_alloc_queues(adapter)) {
		e_dev_err("Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	/* Explicitly disable IRQ since the NIC can be in any state. */
	dubhe1000_irq_disable_all(adapter);
	return 0;
}

/**
 * dubhe1000_alloc_queues - Allocate memory for all rings
 * @adapter: board private structure to initialize
 *
 * We allocate one ring per queue at run-time since we don't know the
 * number of queues at compile-time.
 **/
static int dubhe1000_alloc_queues(struct dubhe1000_adapter *adapter)
{
	int channel;

	adapter->tx_ring = kcalloc(adapter->num_tx_queues, sizeof(struct dubhe1000_tx_ring), GFP_KERNEL);
	if (!adapter->tx_ring)
		return -ENOMEM;

	adapter->tx_ring[0].count = adapter->lhost_fifo_depth * DUBHE1000_FIFO_DEPTH_LEVEL;

	adapter->rx_ring = kcalloc(adapter->num_rx_queues, sizeof(struct dubhe1000_rx_ring), GFP_KERNEL);
	if (!adapter->rx_ring) {
		kfree(adapter->tx_ring);
		return -ENOMEM;
	}

	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		adapter->rx_ring[channel].count = adapter->lhost_fifo_depth * DUBHE1000_FIFO_DEPTH_LEVEL;

	return DUBHE1000_SUCCESS;
}

static int dubhe1000_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret = -EOPNOTSUPP;

	pr_info("%s dev %s cmd %x\n", __func__, dev->name, cmd);

	if (!netif_running(dev))
		return -EINVAL;
	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}
	return ret;
}

/**
 * dubhe1000_tx_timeout - Respond to a Tx Hang
 * @netdev: network interface device structure
 * @txqueue: number of the Tx queue that hung (unused)
 **/
static void dubhe1000_tx_timeout(struct net_device *netdev, unsigned int __always_unused txqueue)
{
	struct dubhe1000_adapter *adapter = NULL;
	struct dubhe1000_mac *port;

	port = netdev_priv(netdev);
	adapter = port->adapter;
	pr_info("%s dev %s port %d\n", __func__, netdev->name, port->id);
	/* Do the reset outside of interrupt context */
}

void dubhe1000_rx_poll(struct dubhe1000_adapter *adapter, u8 start)
{
	bool ret = false;

	switch (start) {
	case 1:
		if (!adapter->tx_poll_start) {
			dubhe1000_irq_disable_all(adapter);
			adapter->tx_poll_start = true;
			ret = queue_delayed_work(adapter->wq, &adapter->time_work,
						 msecs_to_jiffies(DUBHE1000_POLL_INTERVAL));
		}
		break;
	case 0:
		if (adapter->tx_poll_start) {
			dubhe1000_irq_enable_all(adapter);
			adapter->tx_poll_start = false;
			ret = cancel_delayed_work_sync(&adapter->time_work);
		}
		break;
	default:
		break;
	}
}

void dubhe1000_register_xgmac_link_monitor(struct dubhe1000_mac *port)
{
	struct dubhe1000_adapter *adapter = port->adapter;

	port->mac_monitor = 1;

	if (refcount_read(&adapter->mac_monitor_ref) == 1) {
		refcount_inc(&adapter->mac_monitor_ref);
		schedule_delayed_work(&adapter->mac_monitor_work, msecs_to_jiffies(DUBHE1000_MAC_MONITOR_DELAY_MS));
	} else
		refcount_inc(&adapter->mac_monitor_ref);
}

void dubhe1000_unregister_xgmac_link_monitor(struct dubhe1000_mac *port)
{
	struct dubhe1000_adapter *adapter = port->adapter;

	port->mac_monitor = 0;

	refcount_dec(&adapter->mac_monitor_ref);
}

/**
 * dubhe1000_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog task is started,
 * and the stack is notified that the interface is ready.
 **/
static int dubhe1000_open(struct net_device *netdev)
{
	int ret;

	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;

	if (verbose)
		pr_info("%s dev %s\n", __func__, netdev->name);

#ifdef CONFIG_DUBHE2000_PHYLINK
	if (port->is_sfp != 1) {
		ret = dubhe1000_init_phy(netdev);
		if (ret) {
			pr_err("%s: Cannot attach to PHY (error: %d)\n", __func__, ret);
			return ret;
		}
	}
#endif

	if (!adapter->tx_poll_start) {
		/* TODO:
		 * do something
		 */
	}

#ifdef CONFIG_DUBHE2000_PHYLINK
	/* Initialize the MAC Core */
	dubhe1000_xgmac_init(port, netdev);
	if (port->is_sfp != 1) {
		phylink_start(port->phylink);
		/* We may have called phylink_speed_down before */
		phylink_speed_up(port->phylink);
	} else {
		uint32_t is_link = 0;
		int speed = SPEED_1000;
		struct phylink_link_state state;

		if (port->phy_interface == PHY_INTERFACE_MODE_SGMII)
			speed = SPEED_1000;
		else if (port->phy_interface == PHY_INTERFACE_MODE_2500BASEX)
			speed = SPEED_2500;
		else
			speed = port->max_speed;

		printk(KERN_ERR "MAC%d pcs %d pcs_port %d speed %d", port->id, port->pcs_sel, port->pcs_port, speed);

		if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII)
			dubhe1000_xpcs_speed(adapter->pcs[port->pcs_port], port->pcs_port, speed);

		dubhe1000_xgmac_speed(port, speed);

		if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII)
			dubhe1000_xpcs_enable(adapter->pcs[port->pcs_sel], port->pcs_port, true);

		dubhe1000_register_xgmac_link_monitor(port);

		dubhe1000_enable_xgmac_interrupt(port, true);

		dubhe1000_enable_xgmac(port, false);

		usleep_range(1, 2);

		is_link = dubhe1000_xgmac_get_link_status(port);

		if (is_link) {
			dubhe1000_switch_drain_port(adapter, port->id, false);
			dubhe1000_enable_xgmac(port, true);
		}

		if (port->interface == XGMAC_INTERFACE_MODE_NON_RGMII) {
			dubhe1000_xpcs_get_link_state(adapter->pcs[port->pcs_sel], port->pcs_port, &state);

			pr_info("Get PCS%d port%d state type:%d (an_enabled:%d an_complete:%d link:%d duplex:%d speed:%d)",
					port->pcs_sel, port->pcs_port, state.interface,
					state.an_enabled, state.an_complete,
					state.link, state.duplex, state.speed);
		}

		if (is_link)
			netif_carrier_on(netdev);
	}
#elif defined(CONFIG_DUBHE2000_DEVLINK)
	netif_carrier_on(netdev);
#else
#error "Please choice a phy management method"
#endif

#ifndef DISABLE_TRANSMIT
	netif_start_queue(netdev);

#endif
	return DUBHE1000_SUCCESS;
}

/**
 * dubhe1000_close - Disables a network interface
 * @netdev: network interface device structure
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the drivers control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 **/
static int dubhe1000_close(struct net_device *netdev)
{
	struct dubhe1000_mac *port = netdev_priv(netdev);
	//struct dubhe1000_adapter *adapter = port->adapter;

	pr_info("%s dev %s\n", __func__, netdev->name);

#ifdef CONFIG_DUBHE2000_PHYLINK
	/* Stop and disconnect the PHY */
	if (port->phylink) {
		phylink_stop(port->phylink);
		phylink_disconnect_phy(port->phylink);
	}
#endif

#ifndef DISABLE_TRANSMIT
	netif_stop_queue(netdev);

#endif
	return 0;
}

#if (DUBHE1000_SW_BMU_EN)
/**
 * dubhe1000_check_64k_bound - check that memory doesn't cross 64kB boundary
 * @adapter: address of board private structure
 * @start: address of beginning of memory
 * @len: length of memory
 **/
static bool dubhe1000_check_64k_bound(struct dubhe1000_adapter *adapter, void *start, unsigned long len)
{
	unsigned long begin = (unsigned long)start;
	unsigned long end = begin + len;

	/* not allow any memory write location to cross 64k boundary due to errata 23 */
	return ((begin ^ (end - 1)) >> 16) == 0;
}
#endif
/**
 * dubhe1000_setup_tx_resources - allocate Tx resources
 * @adapter: board private structure
 * @txdr:
 *
 * Return 0 on success, negative on failure
 **/
static int dubhe1000_setup_tx_resources(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *txdr)
{
	int size;

	size = sizeof(struct dubhe1000_tx_buffer) * txdr->count;

	txdr->buffer_info = vzalloc(size);
	if (!txdr->buffer_info)
		return -ENOMEM;

	txdr->next_to_use = 0;
	txdr->next_to_clean = 0;

	return 0;
}

/**
 * dubhe1000_setup_all_tx_resources - wrapper to allocate Tx resources for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/
int dubhe1000_setup_all_tx_resources(struct dubhe1000_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		err = dubhe1000_setup_tx_resources(adapter, &adapter->tx_ring[i]);
		if (err) {
			e_dev_err("Allocation for Tx Queue %u failed\n", i);
			for (i--; i >= 0; i--)
				dubhe1000_free_tx_resources(adapter, &adapter->tx_ring[i]);
			break;
		}
	}

	return err;
}

/**
 * dubhe1000_setup_rx_resources - allocate Rx resources ()
 * @adapter: board private structure
 * @rxdr:
 *
 * Returns 0 on success, negative on failure
 **/
static int dubhe1000_setup_rx_resources(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rxdr)
{
	int size;

	size = sizeof(struct dubhe1000_rx_buffer) * rxdr->count;

	rxdr->buffer_info = vzalloc(size);
	if (!rxdr->buffer_info)
		return -ENOMEM;

	rxdr->next_to_clean = 0;
	rxdr->next_to_use = 0;

	return 0;
}

/**
 * dubhe1000_setup_all_rx_resources
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/
int dubhe1000_setup_all_rx_resources(struct dubhe1000_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_rx_queues; i++) {
		err = dubhe1000_setup_rx_resources(adapter, &adapter->rx_ring[i]);
		if (err) {
			e_dev_err("Allocation for Rx Queue %u failed\n", i);
			for (i--; i >= 0; i--)
				dubhe1000_free_rx_resources(adapter, i, &adapter->rx_ring[i]);
			break;
		}
	}

	return err;
}

/**
 * dubhe1000_configure_rx - Configure 8254x Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void dubhe1000_configure_rx(struct dubhe1000_adapter *adapter)
{
	adapter->clean_rx = dubhe1000_clean_rx_irq;
	adapter->alloc_rx_buf = dubhe1000_alloc_rx_buffers;
}

/**
 * dubhe1000_free_tx_resources - Free Tx Resources per Queue
 * @adapter: board private structure
 * @tx_ring:
 *
 * Free all transmit software resources
 **/
static void dubhe1000_free_tx_resources(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring)
{
	dubhe1000_clean_tx_ring(adapter, tx_ring);

	vfree(tx_ring->buffer_info);
	tx_ring->buffer_info = NULL;
}

/**
 * dubhe1000_free_all_tx_resources - Free Tx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all transmit software resources
 **/
void dubhe1000_free_all_tx_resources(struct dubhe1000_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		dubhe1000_free_tx_resources(adapter, &adapter->tx_ring[i]);
}

static void dubhe1000_unmap_and_free_tx_resource(struct dubhe1000_adapter *adapter,
						 struct dubhe1000_tx_buffer *buffer_info)
{
	u8 is_to_cpu_tag_pkt = 0;

	if (!adapter->soft_bmu_en) {
		if ((buffer_info->dma >= adapter->body_dma) &&
		    (buffer_info->dma <= (adapter->body_dma + DUBHE1000_HW_BMU_BUF_MAX(adapter)))) {
			is_to_cpu_tag_pkt = 1;
		}
	}

	if (buffer_info->dma) {
		if (netif_msg_pktdata(adapter))
			pr_info("free tx-buff buffer_info->dma %llx body_dma %llx %llx is_to_cpu_tag_pkt %d\n",
				buffer_info->dma, adapter->body_dma,
				(adapter->body_dma + DUBHE1000_HW_BMU_BUF_MAX(adapter)), is_to_cpu_tag_pkt);

		if (is_to_cpu_tag_pkt == 0) {
			if (buffer_info->mapped_as_page)
				dma_unmap_page(adapter->dev, buffer_info->dma, buffer_info->length, DMA_TO_DEVICE);
			else
				dma_unmap_single(adapter->dev, buffer_info->dma, buffer_info->length, DMA_TO_DEVICE);
		}
		buffer_info->dma = 0;
	}
	if (buffer_info->skb) {
		dev_kfree_skb_any(buffer_info->skb);
		buffer_info->skb = NULL;
	}
	buffer_info->time_stamp = 0;
	/* buffer_info must be completely set up in the transmit path */
}

/**
 * dubhe1000_clean_tx_ring - Free Tx Buffers
 * @adapter: board private structure
 * @tx_ring: ring to be cleaned
 **/
static void dubhe1000_clean_tx_ring(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring)
{
	struct dubhe1000_tx_buffer *buffer_info;
	unsigned long size;
	unsigned int i;

	/* Free all the Tx ring sk_buffs */
	for (i = 0; i < tx_ring->count; i++) {
		buffer_info = &tx_ring->buffer_info[i];
		dubhe1000_unmap_and_free_tx_resource(adapter, buffer_info);
	}

#ifdef CONFIG_DUBHE2000_DEVLINK
	dubhe1000_mac_pcs_config(adapter, 0); // stop fwd's tx/rx
#endif

	size = sizeof(struct dubhe1000_tx_buffer) * tx_ring->count;
	memset(tx_ring->buffer_info, 0, size);

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
	tx_ring->last_tx_tso = false;
}

/**
 * dubhe1000_free_rx_resources - Free Rx Resources
 * @adapter: board private structure
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 **/
static void dubhe1000_free_rx_resources(struct dubhe1000_adapter *adapter, int channel,
					struct dubhe1000_rx_ring *rx_ring)
{
	dubhe1000_clean_rx_ring(adapter, channel, rx_ring);
	vfree(rx_ring->buffer_info);
	rx_ring->buffer_info = NULL;
}

/**
 * dubhe1000_free_all_rx_resources - Free Rx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all receive software resources
 **/
void dubhe1000_free_all_rx_resources(struct dubhe1000_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		dubhe1000_free_rx_resources(adapter, i, &adapter->rx_ring[i]);
}

static unsigned int dubhe1000_frag_len(const struct dubhe1000_adapter *adapter)
{
	return SKB_DATA_ALIGN(adapter->rx_buffer_len + DUBHE1000_HEADROOM + DUBHE1000_TO_CPU_TAG_HLEN +
			      adapter->body_offset) +
	       SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
}

#if (DUBHE1000_SW_BMU_EN)
static void *dubhe1000_alloc_frag(const struct dubhe1000_adapter *adapter)
{
	unsigned int len = dubhe1000_frag_len(adapter);
	u8 *data = netdev_alloc_frag(len);

	return data;
}
#endif
/**
 * dubhe1000_clean_rx_ring - Free Rx Buffers per Queue
 * @adapter: board private structure
 * @rx_ring: ring to free buffers from
 **/
static void dubhe1000_clean_rx_ring(struct dubhe1000_adapter *adapter, int channel, struct dubhe1000_rx_ring *rx_ring)
{
	struct dubhe1000_rx_buffer *buffer_info;
	unsigned long size;
	unsigned int i;

	/* Free all the Rx netfrags */
	for (i = 0; i < rx_ring->count; i++) {
		buffer_info = &rx_ring->buffer_info[i];
		if (adapter->clean_rx) {
			if (buffer_info->dma)
				dma_unmap_single(adapter->dev, buffer_info->dma, adapter->rx_buffer_len,
						 DMA_FROM_DEVICE);
			if (buffer_info->rxbuf.data) {
				skb_free_frag(buffer_info->rxbuf.data);
				buffer_info->rxbuf.data = NULL;
			}
		}
		buffer_info->dma = 0;
	}

	/* there also may be some cached data from a chained receive */
	napi_free_frags(&adapter->edma_rx_napi[channel]);

	size = sizeof(struct dubhe1000_rx_buffer) * rx_ring->count;
	memset(rx_ring->buffer_info, 0, size);

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
}

u8 dubhe1000_hw_bmu_tx_buff_dma(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring, u8 *skb_data,
				struct dubhe1000_tx_buffer *buffer_info, u32 *bmu_id)
{
	u8 *data = NULL;
	u8 is_to_cpu_tag_pkt = 0;
	u32 dma_offset = 0;

	data = adapter->hw_data;      // VA
	dma_offset = skb_data - data; // VA-VA_basic

	if (netif_msg_pktdata(g_adapter))
		pr_info("%s hw_data:0x%px,dma_offset:%x,skb_data:0x%px,body_dma:%llx\n",
			__func__, data, dma_offset, skb_data, adapter->body_dma);

	if ((skb_data >= data) && (skb_data <= (data + DUBHE1000_HW_BMU_BUF_MAX(adapter)))) {
		is_to_cpu_tag_pkt = 1;
		buffer_info->dma = adapter->body_dma + dma_offset; ///adapter->body_dma

		*bmu_id = dma_offset / adapter->body_block_size;
	}

	return is_to_cpu_tag_pkt;
}

/*
 * Init the tx-buff
 */
static int dubhe1000_tx_map(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring, struct sk_buff *skb,
			    unsigned int first, unsigned int max_per_txd, unsigned int nr_frags, uint32_t cpu_tag)
{
	struct dubhe1000_tx_buffer *buffer_info;
	unsigned int len = skb_headlen(skb);
	unsigned int offset = 0, size, count = 0, i;
	unsigned int f, bytecount, segs;
	static u16 pkt_id = 0;
	u32 bmu_id = 0;
	u8 is_to_cpu_tag_pkt = 0;

#if (EDMA_HW)
	s32 val = 0;
	s32 rx_fifo_remain_size = 0;
	//s32 cpu_tag[4] = {0};
#endif

	i = tx_ring->next_to_use;

	while (len) {
		buffer_info = &tx_ring->buffer_info[i];
		size = min(len, max_per_txd);

		buffer_info->length = size;
		/* set time_stamp *before* dma to help avoid a possible race */
		buffer_info->time_stamp = jiffies;
		buffer_info->mapped_as_page = false;

#if (EDMA_HW)
		rx_fifo_remain_size = er32(RX_DESCR_FIFO_STATUS);

		if (rx_fifo_remain_size == (adapter->lhost_fifo_depth * DUBHE1000_FIFO_DEPTH_LEVEL))
			break;

		if (!adapter->soft_bmu_en) {
			is_to_cpu_tag_pkt = dubhe1000_hw_bmu_tx_buff_dma(adapter, tx_ring, skb->data + offset,
									 buffer_info, &bmu_id);
		}

		if (is_to_cpu_tag_pkt == 0) {
			buffer_info->dma = dma_map_single(adapter->dev, skb->data + offset, size, DMA_TO_DEVICE);

			if (dma_mapping_error(adapter->dev, buffer_info->dma))
				goto dma_error;
		} else {
			__flush_dcache_area(adapter->hw_data + (bmu_id * adapter->body_block_size), 2048);
			barrier();
		}

		pkt_id++;
		if (pkt_id == 0x7FFF)
			pkt_id = 0;
		val = size;			     // packet-len
		val |= ((!!cpu_tag) << TAG_IND_BIT); // without From CPU Tag
		val |= (pkt_id << RX_PKT_ID_BIT);    // RX packet ID
		val |= (1 << RX_DESCR_CFG_EN);

		buffer_info->rx_description1 = val;
		cpu_tag = 0;
		adapter->total_tx_bytes += size;
		adapter->total_tx_packets += 1;
		if (netif_msg_pktdata(adapter))
			dubhe1000_skb_dump(adapter, skb, true);
#endif
		buffer_info->next_to_watch = i;

		len -= size;
		offset += size;
		count++;
		if (len) {
			i++;
			if (unlikely(i == tx_ring->count))
				i = 0;
		}
	}

	for (f = 0; f < nr_frags; f++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[f];

		len = skb_frag_size(frag);
		offset = 0;

		while (len) {
			i++;
			if (unlikely(i == tx_ring->count))
				i = 0;

			buffer_info = &tx_ring->buffer_info[i];
			size = min(len, max_per_txd);

			buffer_info->length = size;
			buffer_info->time_stamp = jiffies;
			buffer_info->mapped_as_page = true;

#if (EDMA_HW)
			rx_fifo_remain_size = er32(RX_DESCR_FIFO_STATUS);
			if (rx_fifo_remain_size == (adapter->lhost_fifo_depth * DUBHE1000_FIFO_DEPTH_LEVEL))
				break;

			is_to_cpu_tag_pkt = 0;
			if (!adapter->soft_bmu_en) {
				is_to_cpu_tag_pkt = dubhe1000_hw_bmu_tx_buff_dma(adapter, tx_ring, skb->data + offset,
										 buffer_info, &bmu_id);
			}

			if (is_to_cpu_tag_pkt == 0) {
				buffer_info->dma = skb_frag_dma_map(adapter->dev, frag, offset, size, DMA_TO_DEVICE);

				if (dma_mapping_error(adapter->dev, buffer_info->dma))
					goto dma_error;
			}

			pkt_id++;
			if (pkt_id == 0x7FFF)
				pkt_id = 0;
			val = size;			  // packet-len
			val |= (0 << TAG_IND_BIT);	  // with From CPU Tag
			val |= (pkt_id << RX_PKT_ID_BIT); // RX packet ID
			val |= (1 << RX_DESCR_CFG_EN);

			buffer_info->rx_description1 = val;

			adapter->total_tx_bytes += size;
			adapter->total_tx_packets += 1;
#endif
			buffer_info->next_to_watch = i;

			len -= size;
			offset += size;
			count++;
		}
	}

	segs = skb_shinfo(skb)->gso_segs ?: 1;
	/* multiply data chunks by size of headers */
	bytecount = ((segs - 1) * skb_headlen(skb)) + skb->len;

	tx_ring->buffer_info[i].skb = skb;
	tx_ring->buffer_info[i].segs = segs;
	tx_ring->buffer_info[i].bytecount = bytecount;
	tx_ring->buffer_info[first].next_to_watch = i;

	return count;

dma_error:
	pr_info("TX DMA map failed\n");
	buffer_info->dma = 0;
	if (count)
		count--;

	while (count--) {
		if (i == 0)
			i += tx_ring->count;
		i--;
		buffer_info = &tx_ring->buffer_info[i];
		dubhe1000_unmap_and_free_tx_resource(adapter, buffer_info);
	}

	return 0;
}

/*
 * get the tx-buff, which include dma, buffer_addr, the bus will map the buf to the address
 */
static void dubhe1000_tx_queue(struct dubhe1000_adapter *adapter, struct dubhe1000_tx_ring *tx_ring,
			       struct net_device *netdev, int count)
{
	struct dubhe1000_tx_buffer *buffer_info;
	unsigned int i;
	u64 descr;
#if (EDMA_HW)
	u32 rx_fifo_remain_size;
#endif

	i = tx_ring->next_to_use;

	while (count--) {
		buffer_info = &tx_ring->buffer_info[i];
#if (EDMA_HW)
		rx_fifo_remain_size = er32(RX_DESCR_FIFO_STATUS);
		if (netif_msg_pktdata(adapter))
			pr_info("try to tx-packet rx_fifo_remain_size %d\n", rx_fifo_remain_size);

		if (rx_fifo_remain_size == (adapter->lhost_fifo_depth * DUBHE1000_FIFO_DEPTH_LEVEL))
			break;

		descr = buffer_info->rx_description1;
		ew64(RX_DESCRIPTION0, ((descr << 32) | buffer_info->dma));

		netdev->stats.tx_bytes += buffer_info->length;
		netdev->stats.tx_packets += 1;

		g_tx_index = i;
		if (netif_msg_pktdata(adapter))
			pr_info("tx packet-id %d len %d (rx_descrip1 %x RX_DESCRIPTION0 %llx) tx-cnt(%d ack %d) clean (ack %d) use (xmit %d)\n",
				i, buffer_info->length, buffer_info->rx_description1, buffer_info->dma,
				adapter->total_tx_packets, adapter->total_tx_ack_packets, tx_ring->next_to_clean,
				tx_ring->next_to_use);
#endif
		if (unlikely(++i == tx_ring->count))
			i = 0;
	}

	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64).
	 */
	dma_wmb();

	tx_ring->next_to_use = i;
}

static int __dubhe1000_maybe_stop_tx(struct net_device *netdev, int size)
{
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
	struct dubhe1000_tx_ring *tx_ring = adapter->tx_ring;

	netif_stop_queue(netdev);
	/* Herbert's original patch had:
	 *  smp_mb__after_netif_stop_queue();
	 * but since that doesn't exist yet, just open code it.
	 */
	smp_mb();

	/* We need to check again in a case another CPU has just
	 * made room available.
	 */
	if (likely(DUBHE1000_DESC_UNUSED(tx_ring) < size))
		return -EBUSY;

	/* A reprieve! */
	netif_start_queue(netdev);

	return 0;
}

static int dubhe1000_maybe_stop_tx(struct net_device *netdev, struct dubhe1000_tx_ring *tx_ring, int size)
{
	if (likely(DUBHE1000_DESC_UNUSED(tx_ring) >= size))
		return 0;

	return __dubhe1000_maybe_stop_tx(netdev, size);
}

uint64_t npe_statistics_tx_first_time = 0, npe_statistics_tx_last_time = 0;
uint64_t npe_statistics_tx_cnt = 0, npe_statistics_tx_byte = 0;

void npe_tx_data_statistics(uint32_t skb_len)
{
	npe_statistics_tx_cnt++;
	npe_statistics_tx_byte += skb_len;
	if (npe_statistics_tx_cnt == 1) {
		npe_statistics_tx_first_time = jiffies;
		npe_statistics_tx_last_time = npe_statistics_tx_first_time;
	} else {
		npe_statistics_tx_last_time = jiffies;
	}
}

void npe_tx_data_dump(void)
{
	pr_warn("[tx]first_time:%llu,last_time:%llu,delt_time:%lld,txcnt:%lld(%llu Bytes)\n",
		npe_statistics_tx_first_time, npe_statistics_tx_last_time,
		(npe_statistics_tx_last_time - npe_statistics_tx_first_time), npe_statistics_tx_cnt,
		npe_statistics_tx_byte);

	npe_statistics_tx_first_time = 0;
	npe_statistics_tx_last_time = 0;
	npe_statistics_tx_cnt = 0;
	npe_statistics_tx_byte = 0;
}

static inline int dubhe2000_skb_is_learning_frame(struct sk_buff *skb)
{
	return (skb->bmu_flag & CLS_BMU_FLAG_LEARNING);
}

static inline int dubhe2000_skb_is_hw_accel(struct sk_buff *skb)
{
	return (skb->bmu_flag & CLS_BMU_FLAG_HW_ACCEL);
}

/* mapping the tx-buff to DMA, and notifiy DMA to send packet
 */
#if !(DUBHE1000_TX_OPTI_EN)
static netdev_tx_t dubhe1000_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
#ifndef DISABLE_TRANSMIT
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
	struct dubhe1000_tx_ring *tx_ring;
	unsigned int first, max_per_txd = DUBHE1000_MAX_DATA_PER_TXD;
	unsigned int nr_frags = 0;
	int count = 0;
	struct dubhe1000_from_cpu_tag from_cpu_tag = {
		.ethernet_type = DUBHE1000_FROM_CPU_ETHE_TYPE,
		.port_mask = 1 << (port->id),
		.queue_ind = 0,
		.modified_ind = 1,
		.timestamps = { 0 }
	};
	int cloned = 0;
	u8 *data = NULL;
	struct sk_buff *skb2 = NULL;
	bool need_fromcputag;

	/* if packet size is less than ETH_ZLEN, pad all small packets manually.*/
	if (eth_skb_pad(skb))
		return NETDEV_TX_OK;

	need_fromcputag = adapter->enable_from_cpu_tag &&
			  !dubhe2000_skb_is_learning_frame(skb) &&
			  !dubhe2000_skb_is_hw_accel(skb);

	/* lock against tx reclaim */
	spin_lock(&adapter->tx_lock);

	if (need_fromcputag) {
		cloned = skb_cloned(skb);
		if (likely(!cloned)) {
			if (skb_headroom(skb) < DUBHE1000_FROM_CPU_TAG_HLEN) {
				skb2 = skb_realloc_headroom(skb, DUBHE1000_FROM_CPU_TAG_HLEN);
				if (skb2 == NULL) {
					printk(KERN_ERR "%s: no memory\n", __func__);
					kfree_skb(skb);
					return -ENOBUFS;
				}
				consume_skb(skb);
				skb = skb2;
			}
			skb_push(skb, DUBHE1000_FROM_CPU_TAG_HLEN);
			memcpy(skb->data, &from_cpu_tag, DUBHE1000_FROM_CPU_TAG_HLEN);
		} else {
			int length;

			length = SKB_DATA_ALIGN(DUBHE1000_HEADROOM + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) +
						skb->len + DUBHE1000_FROM_CPU_TAG_HLEN);

			data = netdev_alloc_frag(length);
			if (!data) {
				printk(KERN_ERR "%s: no memory\n", __func__);
				return -ENOBUFS;
			}
			skb2 = build_skb(data, length);
			if (!skb2)
				return 0;

			skb_reserve(skb2, DUBHE1000_HEADROOM);
			skb2->dev = skb->dev;
			memcpy(skb2->data, &from_cpu_tag, DUBHE1000_FROM_CPU_TAG_HLEN);
			memcpy(skb2->data + DUBHE1000_FROM_CPU_TAG_HLEN, skb->data, skb->len);
			skb_put(skb2, skb->len + DUBHE1000_FROM_CPU_TAG_HLEN);
			consume_skb(skb);
			skb = skb2;
		}
	}

	tx_ring = adapter->tx_ring;
	dubhe1000_trace(xmit_frame, skb, netdev, tx_ring);

	nr_frags = skb_shinfo(skb)->nr_frags;

	first = tx_ring->next_to_use;

	count = dubhe1000_tx_map(adapter, tx_ring, skb, first, max_per_txd, nr_frags, !!need_fromcputag);
	if (count) {
		int desc_needed = MAX_SKB_FRAGS + 7;

		netdev_sent_queue(netdev, skb->len);
		skb_tx_timestamp(skb);
		npe_tx_data_statistics(skb->len);

		dubhe1000_tx_queue(adapter, tx_ring, netdev, count);

		/* Make sure there is space in the ring for the next send. */
		dubhe1000_maybe_stop_tx(netdev, tx_ring, desc_needed);

		if (!netdev_xmit_more() || netif_xmit_stopped(netdev_get_tx_queue(netdev, 0))) {
			/* TODO:
			 * do something
			 */
		}
	} else {
		dev_kfree_skb_any(skb);
		tx_ring->buffer_info[first].time_stamp = 0;
		tx_ring->next_to_use = first;
	}

	spin_unlock(&adapter->tx_lock);
#endif
	return NETDEV_TX_OK;
}

/**
 * dubhe1000_clean_tx_irq - Reclaim resources after transmit completes
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t dubhe1000_clean_sw_bmu_tx_irq(struct dubhe1000_adapter *adapter, int irq)
{
#if (DUBHE1000_SW_BMU_EN)

	struct dubhe1000_tx_buffer *buffer_info;
	struct dubhe1000_tx_ring *tx_ring;
	int cnt = 0;
	u32 val2 = 0;
	u32 rx_queue_qsize = 0, rx_pkt_id = 0;
	u8 rx_pkt_err = 0;
	int i = 0;

	/* disable interrupts, without the synchronize_irq bit
	 *
	 * bit0: rx_queue_intr_en
	 *       1: Enable Interrupt, 0: Mask Interrupt
	 */
	dubhe1000_enable_tx_irq(adapter, DISABLE);

	/* RX_QUEUE_STATUS
	 *     bit0~11:  rx_queue_qsize
	 *     bit12:    rx_pkt_err
	 *                  1, bus_err
	 *                  2, len=0
	 *     bit16~30: rx_queue_pkt_id
	 */

	rx_queue_qsize = er32(RX_QUEUE_STATUS);
	rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
	rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
	rx_queue_qsize &= 0xFFF;

	g_tx_intr++;

	tx_ring = adapter->tx_ring;
	i = tx_ring->next_to_clean;
	while (rx_queue_qsize > 0) {
		// bit0~11: rx_queue_del_num, bit12 W1P: rx_queue_del_en
		val2 = ((1 << RX_QUEUE_DEL_EN_BIT) | 1);
		ew32(RX_QUEUE_DEL_INSTR, val2);
		if (rx_pkt_err)
			pr_info("ERR: tx-intr i %d queue_size %d pkt_id %d pkt-err %d clean (ack %d) use (xmit %d)\n",
				i, rx_queue_qsize, rx_pkt_id, rx_pkt_err, tx_ring->next_to_clean, tx_ring->next_to_use);

		buffer_info = &tx_ring->buffer_info[i];

		cnt++;

		netdev_completed_queue(buffer_info->skb->dev, 1, buffer_info->length);

		adapter->total_tx_ack_bytes += buffer_info->length;
		adapter->total_tx_ack_packets += 1;

		if (netif_msg_pktdata(adapter))
			pr_info("tx-complete-intr: tx-packet i %d ack rx_queue_qsize %d %s cnt %d tx-ack(pkt_id %d len %d dma %lx pkt-cnt %d) clean (ack %d) use (xmit %d)\n",
				i, rx_queue_qsize, buffer_info->skb->dev->name, cnt, rx_pkt_id, buffer_info->length,
				buffer_info->dma, adapter->total_tx_ack_packets, tx_ring->next_to_clean,
				tx_ring->next_to_use);

		if (buffer_info->dma)
			dubhe1000_unmap_and_free_tx_resource(adapter, buffer_info);

		memset(buffer_info, 0, sizeof(struct dubhe1000_tx_buffer));

		if (unlikely(++i == tx_ring->count))
			i = 0;

		tx_ring->next_to_clean = i; //rx_pkt_id;
		rx_queue_qsize = er32(RX_QUEUE_STATUS);

		rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7F);
		rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
		rx_queue_qsize &= 0xFFF;
	}

#if (EDMA_HW)
	dubhe1000_enable_tx_irq(adapter, ENABLE);
#endif

#endif
	return IRQ_HANDLED;
}

static irqreturn_t dubhe1000_clean_hw_bmu_tx_irq(struct dubhe1000_adapter *adapter, int irq)
{
	struct dubhe1000_tx_buffer *buffer_info;
	struct dubhe1000_tx_ring *tx_ring;
	int cnt = 0;
	u32 val2 = 0;
	u32 rx_queue_qsize = 0, rx_pkt_id = 0;
	u8 rx_pkt_err = 0;
	int i;

	/* disable interrupts, without the synchronize_irq bit
	 *
	 * bit0: rx_queue_intr_en
	 *       1: Enable Interrupt, 0: Mask Interrupt
	 */
	dubhe1000_enable_tx_irq(adapter, DISABLE);

	/* RX_QUEUE_STATUS
	 *     bit0~11:  rx_queue_qsize
	 *     bit12:    rx_pkt_err
	 *                  1, bus_err
	 *                  2, len=0
	 *     bit16~30: rx_queue_pkt_id
	 */
	rx_queue_qsize = er32(RX_QUEUE_STATUS);
	rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
	rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
	rx_queue_qsize &= 0xFFF;

	g_tx_intr++;

#if defined(__ENABLE_LOOPBACK__)
	if (adapter->loopback_mode) {
		dubhe1000_clean_tx_irq_loopback(adapter, 32);
		dubhe1000_enable_tx_irq(adapter, ENABLE);
		return IRQ_HANDLED;
	}
#endif
	tx_ring = adapter->tx_ring;
	i = tx_ring->next_to_clean;
	while (rx_queue_qsize > 0) {
		// bit0~11: rx_queue_del_num, bit12 W1P: rx_queue_del_en
		val2 = ((1 << RX_QUEUE_DEL_EN_BIT) | 1);
		ew32(RX_QUEUE_DEL_INSTR, val2);
		if (rx_pkt_err)
			pr_info("ERR: HW-BMU tx-complete-intr: queue_size %d pkt_id %d pkt-err %d\n", rx_queue_qsize,
				rx_pkt_id, rx_pkt_err);

		buffer_info = &tx_ring->buffer_info[i];

		cnt++;

		netdev_completed_queue(buffer_info->skb->dev, 1, buffer_info->length);

		adapter->total_tx_ack_bytes += buffer_info->length;
		adapter->total_tx_ack_packets += 1;

		if (netif_msg_pktdata(adapter))
			pr_info("HW-BMU tx-complete-intr: tx-packet ack rx_queue_qsize %d %s cnt %d tx-ack(pkt_id %d len %d pkt-cnt %d byte-cnt %d) dma %llx\n",
				rx_queue_qsize, buffer_info->skb->dev->name, cnt, rx_pkt_id, buffer_info->length,
				adapter->total_tx_ack_packets, adapter->total_tx_ack_bytes, buffer_info->dma);

		if (buffer_info->dma)
			dubhe1000_unmap_and_free_tx_resource(adapter, buffer_info);

		memset(buffer_info, 0, sizeof(struct dubhe1000_tx_buffer));

		if (unlikely(++i == tx_ring->count))
			i = 0;

		tx_ring->next_to_clean = i; //rx_pkt_id;

		rx_queue_qsize = er32(RX_QUEUE_STATUS);

		rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7F);
		rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
		rx_queue_qsize &= 0xFFF;
	}

#if (EDMA_HW)
	dubhe1000_enable_tx_irq(adapter, ENABLE);
#endif

	return IRQ_HANDLED;
}
#else
#define DH_XIMT_FRAME_IRQ_EN_THD 32
unsigned int g_dhxmit_cnt = 0, g_dhxmit_irq_cfm_cnt = 0;
unsigned int g_dhxmit_irq_en_thd = DH_XIMT_FRAME_IRQ_EN_THD;
// 0:close irq, 1:open irq, 0xFF:unknown
#define DHTXIRQ_EN		 1
#define DHTXIRQ_DIS		 0
unsigned int g_dh_tx_irq_en = 0xFF;
int npe_tx_wq = 0;

unsigned int dubhe1000_set_tx_thd(unsigned int thd)
{
	g_dhxmit_irq_en_thd = thd;
	return g_dhxmit_irq_en_thd;
}

static inline int dubhe1000_is_alloc_enable_tx_irq(void)
{
	unsigned int delt_thd = g_dhxmit_cnt - g_dhxmit_irq_cfm_cnt;

	if (delt_thd < 0)
		delt_thd = g_dhxmit_cnt + (0xFFFFFFFF - g_dhxmit_irq_cfm_cnt);

	return (((delt_thd % g_dhxmit_irq_en_thd) == 0) && (g_dh_tx_irq_en != DHTXIRQ_EN)) ? 1 : 0;
}

static netdev_tx_t dubhe1000_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
	struct dubhe1000_tx_ring *tx_ring;
	unsigned int first, max_per_txd = DUBHE1000_MAX_DATA_PER_TXD;
	unsigned int nr_frags = 0;
	int count = 0;
	bool need_fromcputag;

	/* if packet size is less than ETH_ZLEN, pad all small packets manually.*/
	if (eth_skb_pad(skb))
		return NETDEV_TX_OK;

	/* lock against tx reclaim */
	spin_lock(&adapter->tx_lock);

	tx_ring = adapter->tx_ring;
	dubhe1000_trace(xmit_frame, skb, netdev, tx_ring);

	nr_frags = skb_shinfo(skb)->nr_frags;

	first = tx_ring->next_to_use;

	need_fromcputag = adapter->enable_from_cpu_tag &&
			  !dubhe2000_skb_is_learning_frame(skb) &&
			  !dubhe2000_skb_is_hw_accel(skb);

	count = dubhe1000_tx_map(adapter, tx_ring, skb, first, max_per_txd, nr_frags, need_fromcputag ? 1 : 0);
	if (count) {
		npe_tx_data_statistics(skb->len);

		dubhe1000_tx_queue(adapter, tx_ring, netdev, count);
	} else {
		dev_kfree_skb_any(skb);
		tx_ring->buffer_info[first].time_stamp = 0;
		tx_ring->next_to_use = first;
	}

	spin_unlock(&adapter->tx_lock);
	g_dhxmit_cnt++;
	if (dubhe1000_is_alloc_enable_tx_irq()) {
		g_dh_tx_irq_en = DHTXIRQ_EN;
		dubhe1000_enable_tx_irq(adapter, ENABLE);
	}

	return NETDEV_TX_OK;
}

static void dubhe1000_clean_sw_bmu_tx_irq(struct dubhe1000_adapter *adapter, int budget, u32 *work_done)
{
#if (DUBHE1000_SW_BMU_EN)

	struct dubhe1000_tx_buffer *buffer_info;
	struct dubhe1000_tx_ring *tx_ring;
	int cnt = 0;
	u32 val2 = 0;
	u32 rx_queue_qsize = 0, rx_pkt_id = 0;
	u8 rx_pkt_err = 0;
	int i = 0;

	rx_queue_qsize = er32(RX_QUEUE_STATUS);
	rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
	rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
	rx_queue_qsize &= 0xFFF;

	tx_ring = adapter->tx_ring;
	i = tx_ring->next_to_clean;
	while (rx_queue_qsize > 0 && cnt < budget) {
		if (rx_pkt_err)
			pr_info("ERR: tx-intr i %d queue_size %d pkt_id %d pkt-err %d clean (ack %d) use (xmit %d)\n",
				i, rx_queue_qsize, rx_pkt_id, rx_pkt_err, tx_ring->next_to_clean, tx_ring->next_to_use);

		buffer_info = &tx_ring->buffer_info[i];

		cnt++;

		netdev_completed_queue(buffer_info->skb->dev, 1, buffer_info->length);

		adapter->total_tx_ack_bytes += buffer_info->length;
		adapter->total_tx_ack_packets += 1;

		if (netif_msg_pktdata(adapter)) {
			pr_info("tx-complete-intr: tx-packet i %d ack rx_queue_qsize %d %s cnt %d tx-ack(pkt_id %d len %d dma %lx pkt-cnt %d) clean (ack %d) use (xmit %d)\n",
				i, rx_queue_qsize, buffer_info->skb->dev->name, cnt, rx_pkt_id, buffer_info->length,
				buffer_info->dma, adapter->total_tx_ack_packets, tx_ring->next_to_clean,
				tx_ring->next_to_use);
		}

		if (buffer_info->dma)
			dubhe1000_unmap_and_free_tx_resource(adapter, buffer_info);

		memset(buffer_info, 0, sizeof(struct dubhe1000_tx_buffer));

		if (unlikely(++i == tx_ring->count))
			i = 0;

		tx_ring->next_to_clean = i; //rx_pkt_id;
		rx_queue_qsize = er32(RX_QUEUE_STATUS);

		rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7F);
		rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
		rx_queue_qsize &= 0xFFF;
	}

	*work_done = cnt;
	// bit0~11: rx_queue_del_num, bit12 W1P: rx_queue_del_en
	val2 = ((1 << RX_QUEUE_DEL_EN_BIT) | cnt);
	ew32(RX_QUEUE_DEL_INSTR, val2);
	g_dhxmit_irq_cfm_cnt += cnt;
#endif
}

uint32_t static_skb_dev_cnt = 0, static_adapter_skb_dev_cnt = 0;
static irqreturn_t dubhe1000_clean_hw_bmu_tx_irq(struct dubhe1000_adapter *adapter, int budget, u32 *work_done)
{
	struct dubhe1000_tx_buffer *buffer_info;
	struct dubhe1000_tx_ring *tx_ring;
	u32 tx_packet_cnt = 0, tx_packet_cnt_dev = 0;
	u32 tx_packet_len = 0, tx_packet_len_dev = 0;
	u32 val2 = 0;
	u32 rx_queue_qsize = 0, rx_pkt_id = 0;
	u8 rx_pkt_err = 0;
	int i;
	struct net_device *netdev = NULL;

	/* RX_QUEUE_STATUS
	 *     bit0~11:  rx_queue_qsize
	 *     bit12:    rx_pkt_err
	 *                  1, bus_err
	 *                  2, len=0
	 *     bit16~30: rx_queue_pkt_id
	 */
	rx_queue_qsize = er32(RX_QUEUE_STATUS);
	rx_pkt_id = ((rx_queue_qsize >> RX_QUEUE_PKT_BIT) & 0x7FFF);
	rx_pkt_err = ((rx_queue_qsize >> RX_PKT_ERR_BIT) & 0x1);
	rx_queue_qsize &= 0xFFF;

#if defined(__ENABLE_LOOPBACK__)
	if (adapter->loopback_mode) {
		dubhe1000_clean_tx_irq_loopback(adapter, 32);
		dubhe1000_enable_tx_irq(adapter, ENABLE);
		return IRQ_HANDLED;
	}
#endif
	spin_lock(&adapter->tx_lock);
	tx_ring = adapter->tx_ring;
	i = tx_ring->next_to_clean;
	while (rx_queue_qsize > 0 && tx_packet_cnt < budget) {
		if (rx_pkt_err)
			pr_info("ERR: HW-BMU tx-complete-intr: queue_size %d pkt_id %d pkt-err %d\n",
				rx_queue_qsize, rx_pkt_id, rx_pkt_err);

		buffer_info = &tx_ring->buffer_info[i];
		tx_packet_cnt++;

		adapter->total_tx_ack_bytes += buffer_info->length;
		adapter->total_tx_ack_packets += 1;

		if (netif_msg_pktdata(adapter)) {
			pr_info("HW-BMU tx-complete-intr: tx-packet ack rx_queue_qsize %d %s cnt %d tx-ack(pkt_id %d len %d pkt-cnt %d byte-cnt %d) dma %llx\n",
				rx_queue_qsize, buffer_info->skb->dev->name, tx_packet_cnt, rx_pkt_id,
				buffer_info->length, adapter->total_tx_ack_packets, adapter->total_tx_ack_bytes,
				buffer_info->dma);
		}

		if (buffer_info->dma)
			dubhe1000_unmap_and_free_tx_resource(adapter, buffer_info);

		memset(buffer_info, 0, sizeof(struct dubhe1000_tx_buffer));

		if (unlikely(++i == tx_ring->count))
			i = 0;

		tx_ring->next_to_clean = i; //rx_pkt_id;
		rx_queue_qsize--;
	}

	spin_unlock(&adapter->tx_lock);

	// bit0~11: rx_queue_del_num, bit12 W1P: rx_queue_del_en
	val2 = ((1 << RX_QUEUE_DEL_EN_BIT) | tx_packet_cnt);
	ew32(RX_QUEUE_DEL_INSTR, val2);

	*work_done = tx_packet_cnt;
	g_dhxmit_irq_cfm_cnt += tx_packet_cnt;

	return IRQ_HANDLED;
}
#endif

/**
 * dubhe1000_clean_tx_irq - Reclaim resources after transmit completes
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t dubhe1000_clean_tx_irq(int irq, void *data)
{
#if !(DUBHE1000_TX_OPTI_EN)
	irqreturn_t ret = IRQ_HANDLED;
	struct dubhe1000_adapter *adapter = (struct dubhe1000_adapter *)data;

	if (adapter->soft_bmu_en)
		ret = dubhe1000_clean_sw_bmu_tx_irq(adapter, irq);
	else
		ret = dubhe1000_clean_hw_bmu_tx_irq(adapter, irq);

	return ret;
#else
	struct dubhe1000_adapter *adapter = (struct dubhe1000_adapter *)data;
	s32 val = 0;

	g_dh_tx_irq_en = DHTXIRQ_DIS;

	dubhe1000_enable_tx_irq(adapter, DISABLE);

	g_tx_intr++;

	if (likely(napi_schedule_prep(&adapter->tx_napi)))
		__napi_schedule(&adapter->tx_napi);

	return IRQ_HANDLED;
#endif
}

/**
 * dubhe1000_intr - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/
static irqreturn_t dubhe1000_intr(int irq, void *data)
{
	int channel;
	struct dubhe1000_adapter *adapter = data;

	for (channel = 0; channel < adapter->num_rx_queues; channel++)
		if (adapter->rx_intr_list[channel] == irq)
			break;

	/* disable interrupts, without the synchronize_irq bit
	 * bit0: tx_queue_intr_en
	 * 0: Enable Interrupt, 1: Mask Interrupt
	 */
	dubhe1000_enable_rx_irq(adapter, channel, DISABLE);

	g_rx_intr++;

	if (likely(napi_schedule_prep(&adapter->edma_rx_napi[channel]))) {
		__napi_schedule(&adapter->edma_rx_napi[channel]);
	} else {
		/* this really should not happen! if it does it is basically a
		 * bug, but not a hard error, so enable ints and continue
		 */
		dubhe1000_enable_rx_irq(adapter, channel, ENABLE);
	}

	return IRQ_HANDLED;
}

/**
 * dubhe1000_clean - NAPI Rx polling callback
 * @napi: napi struct containing references to driver info
 * @budget: budget given to driver for receive packets
 **/
int dubhe1000_clean(struct napi_struct *napi, int budget)
{
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);
	struct dubhe1000_adapter *adapter = container_of(napi->dev, struct dubhe1000_adapter, napi_dev);
	int channel = ((u64)napi - (u64)adapter->edma_rx_napi) / sizeof(struct napi_struct);
	int work_done = 0;

	if (channel > 0 &&
			atomic_long_read(&adapter->edma_unack_rx[channel]) >
			adapter->edma_rx_limit[channel]) {
		if (verbose && __ratelimit(&rl))
			printk(KERN_ERR "===>EDMA Limit unack_rx[%d] = %ld\n",
					channel, atomic_long_read(&adapter->edma_unack_rx[channel]));
		return budget;
	}

	adapter->clean_rx(adapter, channel, &adapter->rx_ring[channel], &work_done, budget);

	if (work_done == budget)
		return budget;

	/* Exit the polling mode, but don't re-enable interrupts if stack might
	 * poll us due to busy-polling
	 */
	if (likely(napi_complete_done(napi, work_done)))
		dubhe1000_enable_rx_irq(adapter, channel, ENABLE);

	return work_done;
}

#if DUBHE1000_TX_OPTI_EN
/**
 * dubhe1000_clean - NAPI Rx polling callback
 * @napi: napi struct containing references to driver info
 * @budget: budget given to driver for receive packets
 **/
int dubhe1000_tx_clean(struct napi_struct *tx_napi, int budget)
{
	struct dubhe1000_adapter *adapter = container_of(tx_napi, struct dubhe1000_adapter, tx_napi);
	int work_done = 0;

	if (adapter->soft_bmu_en)
		dubhe1000_clean_sw_bmu_tx_irq(adapter, budget, &work_done);
	else
		dubhe1000_clean_hw_bmu_tx_irq(adapter, budget, &work_done);

	if (work_done >= budget)
		return budget;

	/* Exit the polling mode, but don't re-enable interrupts if stack might
	 * poll us due to busy-polling
	 */
	if (likely(napi_complete_done(tx_napi, work_done))) {
		/* TODO:
		 * do something
		 */
	}
	return work_done;
}
#endif

uint32_t eth_process_logs = 0x2;
module_param(eth_process_logs, uint, 0644);

// this marco is copied from CLS_STA_NDEV_IDX in wifi_drv
#define CLS_TID_PER_STA		       8
#define CLS_STA_NDEV_IDX(tid, sta_idx) ((tid) + (sta_idx)*CLS_TID_PER_STA)

static int dev_xmit_bypass_qdisc(struct sk_buff *skb, struct dubhe1000_to_cpu_tag *to_cpu_tag2, int sta_idx, u8 tid)
{
	struct net_device *dev = skb->dev;
	int queue_idx = -1;
	const struct net_device_ops *ops = dev->netdev_ops;
	u16 netdev_queue;
	struct ethhdr *eth_header = (struct ethhdr *)skb->data;

	if (to_cpu_tag2 || (sta_idx >= 0)) {
		//To wifi interface, queue and sta_idx can be get directly without .ndo_select_queue
		skb->priority = tid;
		//in dubhe1000 phase1, only destWPU = HOSTCPU will be enable
		if (to_cpu_tag2)
			netdev_queue = CLS_STA_NDEV_IDX(tid, to_cpu_tag2->meta_data.sta_index_or_flow_id);
		else
			netdev_queue = CLS_STA_NDEV_IDX(tid, sta_idx);

		skb_reset_mac_header(skb);
		skb_set_queue_mapping(skb, netdev_queue);
		skb->bmu_flag |= CLS_BMU_FLAG_NON_TXQ;
	} else if (ops->ndo_select_queue) {
		// some info in skb should be updated before calling .ndo_start_xmit():
		skb_reset_mac_header(skb);
		skb->protocol = eth_header->h_proto;

		if ((skb->protocol == htons(ETH_P_IP)) || (skb->protocol == htons(ETH_P_IPV6)))
			skb_set_network_header(skb, ETH_HLEN);

		queue_idx = ops->ndo_select_queue(dev, skb, NULL);

		skb_set_queue_mapping(skb, queue_idx);
	}

	if (!ops->ndo_start_xmit || ops->ndo_start_xmit(skb, dev) != NETDEV_TX_OK)
		return NET_XMIT_DROP;

	return NET_XMIT_SUCCESS;
}

uint64_t npe_statistics_rx_first_time = 0, npe_statistics_rx_last_time = 0;
uint64_t npe_statistics_rx_cnt = 0, npe_statistics_rx_byte = 0;

void npe_rxmpdu_data_statistics(uint32_t skb_len)
{
	npe_statistics_rx_cnt++;
	npe_statistics_rx_byte += skb_len;
	if (npe_statistics_rx_cnt == 1) {
		npe_statistics_rx_first_time = jiffies;
		npe_statistics_rx_last_time = npe_statistics_rx_first_time;
	} else {
		npe_statistics_rx_last_time = jiffies;
	}
}

void npe_rxmpud_data_dump(void)
{
	pr_warn("[rx]first_time:%llu,last_time:%llu,delt_time:%lld,txcnt:%llu(%llu Bytes)\n",
		npe_statistics_rx_first_time, npe_statistics_rx_last_time,
		(npe_statistics_rx_last_time - npe_statistics_rx_first_time), npe_statistics_rx_cnt,
		npe_statistics_rx_byte);

	npe_statistics_rx_first_time = 0;
	npe_statistics_rx_last_time = 0;
	npe_statistics_rx_cnt = 0;
}


static inline bool dubhe2000_check_reason_code_bypass_fwt(struct dubhe1000_to_cpu_tag *to_cpu_tag2)
{
	u16 reason_code = to_cpu_tag2->reason_code;
	bool ret;

	switch (reason_code) {
	case TOCPU_DNS:
		ret = true;
		break;
	default:
		ret = false;
	}

	return ret;
}


//#define SKB_SW_DUP
#ifdef SKB_SW_DUP
struct sk_buff *dup_sw_skb(struct sk_buff *skb1)
{
	struct sk_buff *skb2 = NULL;

	skb2 = skb_copy(skb1, GFP_KERNEL);
	if (!skb2){
		pr_err("skb copy to sw fail, return bmu skb\n");
		return skb1;
	}

	consume_skb(skb1);

	printk_once("use sw skb\n");
	return skb2;
}
#endif

/**
 * dubhe1000_receive_skb - helper function to handle rx indications
 * @adapter: board private structure
 * @status: descriptor status field as written by hardware
 * @vlan: descriptor vlan field as written by hardware (no le/be conversion)
 * @skb: pointer to sk_buff to be indicated to stack
 */
static void dubhe1000_receive_skb(struct net_device *netdev, struct dubhe1000_to_cpu_tag *to_cpu_tag2,
				  struct sk_buff *skb, u8 tid)
{
	struct dubhe1000_mac *port = netdev_priv(netdev);
	u16 vid = 0;
	int res;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);
	u16 smac_index, dmac_index;
	struct net_device *smac_outdev = NULL;
	struct net_device *dmac_outdev = NULL;
	u32 s_subport, d_subport;
	struct ethhdr *eth_hdr = (struct ethhdr *)skb->data;
	bool is_fwd = false, is_bypass_fwt = false;

	npe_rxmpdu_data_statistics(skb->len);

	if (to_cpu_tag2->nr_of_vlans)
		vid = le16_to_cpu(to_cpu_tag2->vlan_id) & DUBHE1000_RXD_SPC_VLAN_MASK;

	if (port->dev_fp) {
		skb->dev = port->dev_fp;
		if (port->fp_bypass_qdisc)
			res = dev_xmit_bypass_qdisc(skb, 0, -1, 0);
		else
			res = dev_queue_xmit(skb);

		if (res == NET_XMIT_SUCCESS) {
			if (eth_process_logs & 0x1)
				pr_info("Eth succeed to fp skb, res %d, dev 0x%px\n", res, port->dev_fp);
		} else {
			if (eth_process_logs & 0x2)
				pr_warn("Eth failed to fp skb, res %d, dev 0x%px\n", res, port->dev_fp);
		}
	} else {
		skb->src_port = 0;

		is_bypass_fwt = dubhe2000_check_reason_code_bypass_fwt(to_cpu_tag2);
		if (!is_bypass_fwt && clsemi_fast_fwd_enabled()) {
			skb->npe_meta_data.valid = 1;
			skb->npe_meta_data.tid = tid;
			skb->npe_meta_data.raw = to_cpu_tag2->meta_data.raw;
			if (clsemi_fast_fwd(skb, netdev, true)) {
				g_adapter->total_fwt_packets++;
			} else {
				skb->protocol = eth_type_trans(skb, netdev);
				netif_receive_skb(skb);
			}
			return;
		}

		/*
		 * 1. fwt only support unicast
		 * 2. when da/sa exist in fwt
		 * 3. smac's outdev UNCHANGE,
		 * 4. dmac's outdev is not smac's
		 * The packets that meet the above conditions can bypass network stack.
		 */
		if (!vid)
			vid = br_port_get_pvid(netdev);
		if (!is_bypass_fwt && !is_multicast_ether_addr(eth_hdr->h_dest) && g_get_fwt_entry_info_hook) {
			dmac_outdev = g_get_fwt_entry_info_hook(eth_hdr->h_dest, vid, &dmac_index, &d_subport);
			if (dmac_outdev && dmac_outdev != netdev) {
				smac_outdev =
					g_get_fwt_entry_info_hook(eth_hdr->h_source, vid, &smac_index, &s_subport);
				if (smac_outdev && smac_outdev == netdev)
					is_fwd = true;
			}
		}

		if (is_fwd) {
			skb->dev = dmac_outdev;
			skb->dest_port = d_subport;

			// smac is active, reset smac's timestamp. dmac not
			if (g_reset_fwt_ageing_hook)
				g_reset_fwt_ageing_hook(smac_index);

			if (cls_fwt_dbg_enable)
				pr_info("Forward Packet([%pM] (%s) -> [%pM]) to %s\n",
					eth_hdr->h_source, netdev->name,
					eth_hdr->h_dest, dmac_outdev->name);

			/* If FWT will forward it to wifi netdev, tid/sta_idx can be used directly*/
			if (CLS_IEEE80211_VIF_IDX_VALID(d_subport))
				if (to_cpu_tag2->meta_data.destWPU == 0)//sta_idx is invalid(metada has been reassigned, like iperf)
					res = dev_xmit_bypass_qdisc(skb, 0, CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(d_subport), tid);
				else
					res = dev_xmit_bypass_qdisc(skb, to_cpu_tag2, -1, tid);
			else
				res = dev_xmit_bypass_qdisc(skb, 0, -1, 0);
			if (res != NET_XMIT_SUCCESS) {
				if (__ratelimit(&rl))
					pr_err("[%s] FWT FAILED(%d): forward [mac=%pM vid=%d] to %s\n",
					       __func__, res, eth_hdr->h_dest, vid, dmac_outdev->name);

				consume_skb(skb);
			} else {
				g_adapter->total_fwt_packets++;
			}
		} else { // not hit or sa should to be learned
			skb->protocol = eth_type_trans(skb, netdev);
#ifdef SKB_SW_DUP
			netif_receive_skb(dup_sw_skb(skb));
#else
			netif_receive_skb(skb);
#endif
		}
	}
}

/**
 * dubhe1000_clean_rx_irq - Send received data up the network stack; legacy
 * @adapter: board private structure
 * @rx_ring: ring to clean
 * @work_done: amount of napi work completed this call
 * @work_to_do: max amount of work allowed for this call to do
 */
static bool dubhe1000_clean_sw_bmu_rx_irq(struct dubhe1000_adapter *adapter, int channel,
					  struct dubhe1000_rx_ring *rx_ring, int *work_done, int work_to_do)
{
	bool cleaned = false;

#if (DUBHE1000_SW_BMU_EN)
	struct net_device *netdev = NULL;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	struct dubhe1000_rx_buffer *buffer_info, *next_buffer;
	u32 length = 0;
	unsigned int i;
	int cleaned_count = 0;
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;

	u32 tx_queue_qsize = 0, qstatus1 = 0;
	u8 tx_pkt_err = 0, is_split = 0, tag_qos_tid = 0;
	unsigned int frag_len = 0;

	const struct dubhe1000_mac *port;
	struct dubhe1000_to_cpu_tag to_cpu_tag2;
	u32 input_port;
	int err;
	int pkt_id;
	u64 qstatus;

	i = rx_ring->next_to_clean;
	buffer_info = &rx_ring->buffer_info[i];
	/*  TX_QUEUE_STATUS0
	 *     bit0-11: tx_queue_qsize
	 *     bit14:   tx_pkt_err
	 *     bit15:   is_split
	 *     bit16-31: Packet id
	 */
	qstatus = er64_x(channel, TX_QUEUE_STATUS0);

	tx_queue_qsize = qstatus & 0xFFFFFFFF;

	pkt_id = ((tx_queue_qsize & 0xFFFF0000) >> TX_PACKET_ID);
	is_split = ((tx_queue_qsize & 0x8000) >> IS_SPLIT_BIT);
	tx_pkt_err = ((tx_queue_qsize & 0x4000) >> TX_PKT_ERR_BIT);

	tx_queue_qsize = (tx_queue_qsize & 0xFFF);

	while (tx_queue_qsize > 0) {
		struct sk_buff *skb;
		u8 *data;

		qstatus1 = (qstatus >> 32);
		tag_qos_tid = ((qstatus1 >> TX_TAG_QOS_TID_BIT) & 0x7); //QoS TID

		if (netif_msg_pktdata(adapter))
			pr_info("rx-intr i %d tx_queue_qsize %d rx(pkt_id %d split %d pkt_err %d qos-tid %d) %d/%d clean(rx %d) use(malloc %d)\n",
				i, tx_queue_qsize, pkt_id, is_split, tx_pkt_err, tag_qos_tid, *work_done, work_to_do,
				rx_ring->next_to_clean, rx_ring->next_to_use);

		if (*work_done >= work_to_do)
			break;

		(*work_done)++;
		dma_rmb(); /* read rx_buffer_info after status DD */

		data = buffer_info->rxbuf.data;
		prefetch(data);

		frag_len = dubhe1000_frag_len(adapter);
		skb = build_skb(data, frag_len);
		if (!skb) {
			adapter->alloc_rx_buff_failed++;
			break;
		}
		// get the packet to data here
		dma_unmap_single(adapter->dev, buffer_info->dma, (adapter->rx_buffer_len + DUBHE1000_TO_CPU_TAG_HLEN),
				 DMA_FROM_DEVICE);
		buffer_info->dma = 0;
		buffer_info->rxbuf.data = NULL;
		// TX_QUEUE_DEL_INSTR
		ew32_x(channel, TX_QUEUE_DEL_INSTR, 1);

		// move the skb->data to DMAC, which following the 24-byte To CPU Tag
		skb_reserve(skb, DUBHE1000_HEADROOM + DUBHE1000_TO_CPU_TAG_HLEN + adapter->body_offset);

		/* the to CPU Tag header before the DMAC */
		err = dubhe1000_to_cpu_tag_parse(adapter, &to_cpu_tag2, skb->data - DUBHE1000_TO_CPU_TAG_HLEN);

		if (err) { // without to cpu tag, mirror packet
			// free skb here
			dev_kfree_skb(skb);
			return err;
		}

		length = to_cpu_tag2.pakcet_length;
		input_port = to_cpu_tag2.source_port;

		if (++i == rx_ring->count)
			i = 0;

		next_buffer = &rx_ring->buffer_info[i];

		cleaned = true;
		cleaned_count++;

		port = dubhe1000_find_port(adapter, input_port);
		if (unlikely(!port)) {
			if (__ratelimit(&rl)) {
				pr_info("received pkt for non-existent port(%u): reason %d\n",
						input_port, to_cpu_tag2.reason_code);
				dubhe1000_skb_dump(adapter, skb, false);
			}

			// free skb here
			goto dma_map;
		}
		netdev = port->netdev;

		total_rx_bytes += length;
		total_rx_packets++;

		if (buffer_info->rxbuf.data == NULL) {
			skb_put(skb, length);
		} else { /* copybreak skb */
			skb_put(skb, length);
		}
		if (netif_msg_pktdata(adapter)) {
			pr_info("%s received pkt %d packet-len %d vlan %d rx-pkt-cnt(%d/%d) tx-pkt-cnt(%d ack %d)\n",
				netdev->name, i, skb->len, to_cpu_tag2.vlan_id, total_rx_packets,
				adapter->total_rx_packets, adapter->total_tx_packets, adapter->total_tx_ack_packets);

			dubhe1000_skb_dump(adapter, skb, false);
		}

		skb_set_hash(skb, packet_tid, PKT_HASH_TYPE_L2);
		skb_record_rx_queue(skb, channel);

		dubhe1000_receive_skb(netdev, &to_cpu_tag2, skb, tag_qos_tid);

		netdev->stats.rx_bytes += length;
		netdev->stats.rx_packets += 1;
dma_map:

		/* use prefetched values */
		buffer_info = next_buffer;
		tx_queue_qsize = er32_x(channel, TX_QUEUE_STATUS0);
		pkt_id = ((tx_queue_qsize & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((tx_queue_qsize & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((tx_queue_qsize & 0x4000) >> TX_PKT_ERR_BIT);
		tx_queue_qsize = (tx_queue_qsize & 0xFFF);
	}

	rx_ring->next_to_clean = i;
	cleaned_count = DUBHE1000_DESC_UNUSED(rx_ring);

	// re-attach the rx-buf here
	if (cleaned_count)
		adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);
	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;
#endif
	return cleaned;
}

static bool dubhe1000_clean_hw_bmu_split_rx_irq(struct dubhe1000_adapter *adapter, int channel,
						struct dubhe1000_rx_ring *rx_ring, int *work_done, int work_to_do)
{
	bool cleaned = false;
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;

	u32 tx_queue_qsize = 0;
	u8 tx_pkt_err = 0, is_split = 0;
	u32 qstatus1, qstatus0;
	u8 buf_bid = 0, blk_num = 0, tag_qos_tid = 0;
	u16 buf_pid = 0, packet_tid = 0;
	dma_addr_t dma, head_dma;

	unsigned int frag_len = 0;

	int pkt_id;
	int kk = 0;
	u8 *data, *head_data;
	u64 qstatus;
	int k2 = 0;
	u32 val = 0;
	u8 status_invalid;

	/*  TX_QUEUE_STATUS0
	 *     bit0-11: tx_queue_qsize
	 *     bit14:   tx_pkt_err
	 *     bit15:   is_split
	 *     bit16-31: Packet id
	 */
	qstatus = er64_x(channel, TX_QUEUE_STATUS0);

	qstatus0 = qstatus & 0xFFFFFFFF;
	pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
	is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
	tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
	status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);
	tx_queue_qsize = (qstatus0 & 0xFFF);

	while (tx_queue_qsize > 0) {
		qstatus1 = (qstatus >> 32);

		buf_bid = (qstatus1 & 0xFF);
		buf_pid = ((qstatus1 >> TX_BUF_PID_BIT) & 0x1FF);
		blk_num = ((qstatus1 >> TX_BLK_NUM_BIT) & 0x7);
		packet_tid = ((qstatus1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF); //Packet token id
		tag_qos_tid = ((qstatus1 >> TX_TAG_QOS_TID_BIT) & 0x7);	     //QoS TID
		dma = (buf_pid * 256 + buf_bid) * adapter->body_block_size;
		head_dma = (buf_pid * 256 + buf_bid) * adapter->head_block_size;

		if (netif_msg_pktdata(adapter))
			pr_info("%s, qsize %d pkt_id %d split %d err %d bid %d pid %d blk-num %d token-tid %d qos-tid %d addr %llx) %d/%d\n",
				__func__, tx_queue_qsize, pkt_id, is_split, tx_pkt_err, buf_bid, buf_pid, blk_num,
				packet_tid, tag_qos_tid, dma, *work_done, work_to_do);

		if (*work_done >= work_to_do)
			break;

		ew32_x(channel, TX_QUEUE_HOLD_INSTR, 1);

		(*work_done)++;

		dma_rmb(); /* read rx_buffer_info after status DD */

		head_data = adapter->head_hw_data;
		head_data += head_dma;
		prefetch(head_data);

		if (netif_msg_pktdata(adapter)) {
			pr_info("rx head 0x%px dma %llx %llx adapter->hw_data 0x%px:\n",
				head_data, adapter->head_dma, head_dma, adapter->head_hw_data);

			k2 = 6;
			dubhe1000_dump_regs(adapter, adapter->head_hw_data + head_dma);
			for (kk = 0; kk < k2; kk++) {
				if (kk == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->head_hw_data + head_dma + 32 * kk);
			}
			dubhe1000_dump_regs(adapter, adapter->head_hw_data + head_dma + 32 * kk);
			pr_info("skb dump finished\n");
		}

		data = adapter->hw_data;
		data += dma;
		prefetch(data);

		frag_len = dubhe1000_frag_len(adapter);

		if (netif_msg_pktdata(adapter)) {
			pr_info("rx body 0x%px dma %llx %llx adapter->hw_data 0x%px:\n", data, adapter->body_dma, dma,
				adapter->hw_data);
			k2 = 8;
			dubhe1000_dump_regs(adapter, adapter->hw_data + dma);
			for (kk = 0; kk < k2; kk++) {
				if (kk == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->hw_data + dma + 32 * kk);
			}
			dubhe1000_dump_regs(adapter, adapter->hw_data + dma + 32 * kk);
		}

		val = buf_bid;
		val += (buf_pid << FREE_TX_BUF_PID_BIT);
		val += (packet_tid << FREE_TX_BUF_TOKEN_ID_BIT);
		val += (blk_num << FREE_TX_BLK_NUM_BIT);
		val += (1 << FREE_TX_BUF_FREE_EN_BIT);
		ew32_x(channel, TX_BUF_FREE_INSTR, val);

		/* use prefetched values */
		qstatus = er64_x(channel, TX_QUEUE_STATUS0);

		qstatus0 = qstatus & 0xFFFFFFFF;
		pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
		status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);
		tx_queue_qsize = (qstatus0 & 0xFFF);
	}

	// re-attach the rx-buf here
	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;

	return cleaned;
}

void *dubhe1000_copy_skb_ethhdr(struct sk_buff *skb)
{
	void *head = dubhe1000_get_head_data(skb->head);

	memcpy(head, skb->data - DUBHE1000_TO_CPU_TAG_HLEN, DUBHE1000_TO_CPU_TAG_HLEN + sizeof(struct ethhdr));

	skb->bmu_ethhdr = head + DUBHE1000_TO_CPU_TAG_HLEN;

	skb->bmu_flag |= CLS_BMU_FLAG_ETH;

	return head;
}

static bool dubhe1000_clean_hw_bmu_rx_irq(struct dubhe1000_adapter *adapter, int channel,
					  struct dubhe1000_rx_ring *rx_ring, int *work_done, int work_to_do)
{
	struct net_device *netdev = NULL;
	static DEFINE_RATELIMIT_STATE(rl, 3 * HZ, 1);

	u32 length = 0;
	int cleaned_count = 0;
	bool cleaned = false;
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;

	u32 tx_queue_qsize = 0;
	u8 tx_pkt_err = 0, is_split = 0, status_invalid = 0;
	u32 qstatus1, qstatus0;
	u8 buf_bid = 0, blk_num = 0, tag_qos_tid = 0;
	u16 buf_pid = 0, packet_tid = 0;
	dma_addr_t dma;

	unsigned int frag_len = 0;

	const struct dubhe1000_mac *port;
	struct dubhe1000_to_cpu_tag to_cpu_tag2;
	u32 input_port;
	int err;
	int pkt_id;
	int kk = 0;
	struct sk_buff *skb;
	u8 *data;
	u64 qstatus;
	u8 *head;

	if (adapter->split_mode)
		return dubhe1000_clean_hw_bmu_split_rx_irq(adapter, channel, rx_ring, work_done, work_to_do);

	/*  TX_QUEUE_STATUS0
	 *     bit0-11: tx_queue_qsize
	 *     bit13:   status_invalid
	 *     bit14:   tx_pkt_err
	 *     bit15:   is_split
	 *     bit16-31: Packet id
	 */
	qstatus = er64_x(channel, TX_QUEUE_STATUS0);

	qstatus0 = qstatus & 0xFFFFFFFF;
	pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
	is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
	tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
	status_invalid = ((qstatus0 & 0x2000) >> TX_STATUS_INV_BIT);
	tx_queue_qsize = (qstatus0 & 0xFFF);

	while (tx_queue_qsize > 0) {
		if (status_invalid) { /* status invalid, maybe HW is not ready, SW try later */
			pr_info("%s, tx status invalid, 0x%x\n", __func__, qstatus0);
			break;
		}

		qstatus1 = (qstatus >> 32);

		buf_bid = (qstatus1 & 0xFF);
		buf_pid = ((qstatus1 >> TX_BUF_PID_BIT) & 0x1FF);
		packet_tid = ((qstatus1 >> TX_PACKET_TOKEN_ID_BIT) & 0x1FF); //Packet token id
		blk_num = ((qstatus1 >> TX_BLK_NUM_BIT) & 0x7);
		tag_qos_tid = ((qstatus1 >> TX_TAG_QOS_TID_BIT) & 0x7); //QoS TID
		dma = (buf_pid * 256 + buf_bid) * adapter->body_block_size;

		if (netif_msg_pktdata(adapter))
			pr_info("%s, qsize %d rx(pkt_id %d split %d err %d bid %d pid %d blk-num %d token-tid %d qos-tid %d addr %llx) %d/%d\n",
				__func__, tx_queue_qsize, pkt_id, is_split, tx_pkt_err, buf_bid, buf_pid, blk_num,
				packet_tid, tag_qos_tid, dma, *work_done, work_to_do);

		if (*work_done >= work_to_do)
			break;

		if (tx_pkt_err) { /* err pkt, SW move to next, free this one */
			pr_info("%s, tx status invalid, status0 0x%x, status1 0x%x\n", __func__, qstatus0, qstatus1);
			ew32_x(channel, TX_QUEUE_DEL_INSTR, 1);
			goto dma_map;
		}

		(*work_done)++;

		dma_rmb(); /* read rx_buffer_info after status DD */

		data = adapter->hw_data;
		data += dma;

		__inval_dcache_area(data, 2048);
		ew32_x(channel, TX_QUEUE_HOLD_INSTR, 1);

		frag_len = dubhe1000_frag_len(adapter);
		//skb = build_skb(data, frag_len);
		skb = __build_skb(data, frag_len);
		if (!skb) {
			adapter->alloc_rx_buff_failed++;
			break;
		}

		// move the skb->data to DMAC, which following the 24-byte To CPU Tag
		skb_reserve(skb, DUBHE1000_HEADROOM + DUBHE1000_TO_CPU_TAG_HLEN + adapter->body_offset);

		head = dubhe1000_copy_skb_ethhdr(skb);
		/* the to CPU Tag header before the DMAC */
		err = dubhe1000_to_cpu_tag_parse(adapter, &to_cpu_tag2, head);

		if (err) { // without to cpu tag, mirror packet
			// handle mirror pkt or free skb
			if (netif_msg_link(adapter)) {
				pr_info("err: rx skb->data:\n");
				kk = 0;
				pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					skb->data[kk],      skb->data[kk + 1],  skb->data[kk + 2],  skb->data[kk + 3],
					skb->data[kk + 4],  skb->data[kk + 5],  skb->data[kk + 6],  skb->data[kk + 7],
					skb->data[kk + 8],  skb->data[kk + 9],  skb->data[kk + 10], skb->data[kk + 11],
					skb->data[kk + 12], skb->data[kk + 13], skb->data[kk + 14], skb->data[kk + 15]);

				kk = 16;
				pr_info(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					skb->data[kk],      skb->data[kk + 1],  skb->data[kk + 2],  skb->data[kk + 3],
					skb->data[kk + 4],  skb->data[kk + 5],  skb->data[kk + 6],  skb->data[kk + 7],
					skb->data[kk + 8],  skb->data[kk + 9],  skb->data[kk + 10], skb->data[kk + 11],
					skb->data[kk + 12], skb->data[kk + 13], skb->data[kk + 14], skb->data[kk + 15]);
			}
			dev_kfree_skb(skb);
			ew32_x(channel, TX_BUF_FREE_INSTR, qstatus1 | (1 << FREE_TX_BUF_FREE_EN_BIT));

			return err;
		}

		dubhe1000_save_status1(adapter, channel, head, pkt_id, buf_bid, buf_pid, blk_num, packet_tid, qstatus1);

		length = to_cpu_tag2.pakcet_length;
		input_port = to_cpu_tag2.source_port;

		cleaned = true;
		cleaned_count++;

		port = dubhe1000_find_port(adapter, input_port);
		if (unlikely(!port)) {
			if (__ratelimit(&rl)) {
				pr_info("received pkt for non-existent port(%u): reason %d\n",
						input_port, to_cpu_tag2.reason_code);
				dubhe1000_skb_dump(adapter, skb, false);
			}

			dev_kfree_skb(skb);
			goto dma_map;
		}
		netdev = port->netdev;

		total_rx_bytes += length;
		total_rx_packets++;

		atomic_long_inc(&adapter->edma_unack_rx[channel]);

		skb_put(skb, length);
		if (netif_msg_pktdata(adapter)) {
			pr_info("%s HW-BMU received pkt ch %d packet-len %d vlan %d rx-pkt-cnt(%d/%d) tx-pkt-cnt(%d ack %d) len %d\n",
				netdev->name, channel, skb->len, to_cpu_tag2.vlan_id, total_rx_packets,
				adapter->total_rx_packets, adapter->total_tx_packets, adapter->total_tx_ack_packets,
				length);

			dubhe1000_skb_dump(adapter, skb, false);
		}

		skb_set_hash(skb, packet_tid, PKT_HASH_TYPE_L2);
		skb_record_rx_queue(skb, channel);

		//TODO: 'tag_qos_tid' will work after fwt module ported
		dubhe1000_receive_skb(netdev, &to_cpu_tag2, skb, tag_qos_tid);

		netdev->stats.rx_bytes += length;
		netdev->stats.rx_packets += 1;
dma_map:
		/* use prefetched values */
		qstatus = er64_x(channel, TX_QUEUE_STATUS0);

		qstatus0 = qstatus & 0xFFFFFFFF;
		pkt_id = ((qstatus0 & 0xFFFF0000) >> TX_PACKET_ID);
		is_split = ((qstatus0 & 0x8000) >> IS_SPLIT_BIT);
		tx_pkt_err = ((qstatus0 & 0x4000) >> TX_PKT_ERR_BIT);
		tx_queue_qsize = (qstatus0 & 0xFFF);
	}

	// re-attach the rx-buf here
	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;

	return cleaned;
}

static bool dubhe1000_clean_rx_irq(struct dubhe1000_adapter *adapter, int channel, struct dubhe1000_rx_ring *rx_ring,
				   int *work_done, int work_to_do)
{
	bool ret = false;

	if (adapter->soft_bmu_en)
		ret = dubhe1000_clean_sw_bmu_rx_irq(adapter, channel, rx_ring, work_done, work_to_do);
	else
		ret = dubhe1000_clean_hw_bmu_rx_irq(adapter, channel, rx_ring, work_done, work_to_do);

	return ret;
}

/**
 * dubhe1000_alloc_rx_buffers - Replace used receive buffers; legacy & extended
 * @adapter: address of board private structure
 * @rx_ring: pointer to ring struct
 * @cleaned_count: number of new Rx buffers to try to allocate
 **/
static void dubhe1000_sw_bmu_alloc_rx_buffers(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring,
					      int cleaned_count)
{
#if (DUBHE1000_SW_BMU_EN)
	struct dubhe1000_rx_buffer *buffer_info;
	unsigned int i;
	unsigned int bufsz = adapter->rx_buffer_len;
	static u16 packet_id = 0;

	i = rx_ring->next_to_use;
	buffer_info = &rx_ring->buffer_info[i];

	if (netif_msg_pktdata(adapter))
		pr_info("sw_bmu_alloc_rx_buffer cleaned_count %d\n", cleaned_count);

	while (cleaned_count--) {
		void *data;

		if (buffer_info->rxbuf.data)
			goto skip;

		data = dubhe1000_alloc_frag(adapter);
		if (!data) {
			/* Better luck next round */
			adapter->alloc_rx_buff_failed++;
			break;
		}

		/* Fix for errata 23, can't cross 64kB boundary */
		if (!dubhe1000_check_64k_bound(adapter, data, bufsz)) {
			void *olddata = data;

			e_dev_err("skb align check failed: %u bytes at %llx\n", bufsz, data);
			/* Try again, without freeing the previous */
			data = dubhe1000_alloc_frag(adapter);
			/* Failed allocation, critical failure */
			if (!data) {
				skb_free_frag(olddata);
				adapter->alloc_rx_buff_failed++;
				break;
			}

			if (!dubhe1000_check_64k_bound(adapter, data, bufsz)) {
				/* give up */
				skb_free_frag(data);
				skb_free_frag(olddata);
				adapter->alloc_rx_buff_failed++;
				break;
			}

			/* Use new allocation */
			skb_free_frag(olddata);
		}
		buffer_info->dma = dma_map_single(adapter->dev,
						  data,
						  (adapter->rx_buffer_len + DUBHE1000_TO_CPU_TAG_HLEN),
						  DMA_FROM_DEVICE);
		if (dma_mapping_error(adapter->dev, buffer_info->dma)) {
			skb_free_frag(data);
			buffer_info->dma = 0;
			adapter->alloc_rx_buff_failed++;
			break;
		}
		/* XXX if it was allocated cleanly it will never map to a
		 * boundary crossing
		 */

		/* Fix for errata 23, can't cross 64kB boundary */
		if (!dubhe1000_check_64k_bound(adapter, (void *)(unsigned long)buffer_info->dma,
					       adapter->rx_buffer_len)) {
			e_dev_err("dma align check failed: %u bytes at %llx\n",
				  adapter->rx_buffer_len, (void *)(unsigned long)buffer_info->dma);

			dma_unmap_single(adapter->dev, buffer_info->dma,
					 (adapter->rx_buffer_len + DUBHE1000_TO_CPU_TAG_HLEN), DMA_FROM_DEVICE);

			skb_free_frag(data);
			buffer_info->rxbuf.data = NULL;
			buffer_info->dma = 0;

			adapter->alloc_rx_buff_failed++;
			break;
		}
		buffer_info->rxbuf.data = data;
		if (!adapter->split_mode) {
			packet_id++;
			dubhe1000_edma_bmu_config(adapter, packet_id, 0, buffer_info->dma, adapter->txq_status_dma,
						  adapter->txq_status_num);
		}
skip:
		if (unlikely(++i == rx_ring->count))
			i = 0;
		buffer_info = &rx_ring->buffer_info[i];
	}

	if (likely(rx_ring->next_to_use != i)) {
		rx_ring->next_to_use = i;
		if (unlikely(i-- == 0))
			i = (rx_ring->count - 1);

		/* Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.  (Only
		 * applicable for weak-ordered memory model archs,
		 * such as IA-64).
		 */
		dma_wmb();
	}
#endif
}

/**
 * dubhe1000_hw_bmu_alloc_rx_buffers - Replace used receive buffers; legacy & extended
 * @adapter: address of board private structure
 * @rx_ring: pointer to ring struct
 **/
static void dubhe1000_hw_bmu_alloc_rx_buffers(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring)
{
	unsigned int i = 0;
	unsigned int bufsz = adapter->rx_buffer_len;
	unsigned int len = dubhe1000_frag_len(adapter);
	u32 value = 0;

	pr_info(" body(dma %llx data 0x%px)\n", adapter->body_dma, adapter->hw_data);

	if (adapter->split_mode)
		pr_info("HW-BMU alloc rx head_hw_data 0x%px head_dma %llx\n", adapter->head_hw_data, adapter->head_dma);

	if (!adapter->split_mode) {
		dubhe1000_edma_bmu_config(adapter, i, 0, adapter->body_dma, adapter->txq_status_dma,
					  adapter->txq_status_num);
	} else {
		dubhe1000_edma_bmu_config(adapter, i, adapter->head_dma, adapter->body_dma, adapter->txq_status_dma,
					  adapter->txq_status_num);

		value = readl(adapter->mrc_regs + 0x100);
		pr_info(" mrc_regs 0x%px %llx reg[0x100] = %x\n", adapter->mrc_regs, adapter->mrc_dma, value);

		/* Region head config, start/end -> 0x200/0x300, region_en/acp_en->0x100 */
		writel(adapter->head_dma, adapter->mrc_regs + 0x200);
		writel(adapter->head_dma + DUBHE1000_HW_BMU_HEAD_BUF_MAX - 1, adapter->mrc_regs + 0x300);
		writel(value | 0x1, adapter->mrc_regs + 0x100);
	}

	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64).
	 */
	dma_wmb();
	pr_info("hw_bmu_alloc_rx_buffer len %d bufsz %d dma %llx adapter->edma_regs 0x%px hw_data 0x%px buff-size %x\n",
		len, bufsz, adapter->body_dma, adapter->edma_regs, adapter->hw_data, DUBHE1000_HW_BMU_BUF_MAX(adapter));
}

static void dubhe1000_alloc_rx_buffers(struct dubhe1000_adapter *adapter, struct dubhe1000_rx_ring *rx_ring,
				       int cleaned_count)
{
	if (adapter->soft_bmu_en)
		dubhe1000_sw_bmu_alloc_rx_buffers(adapter, rx_ring, cleaned_count);
	else
		dubhe1000_hw_bmu_alloc_rx_buffers(adapter, rx_ring);
}

static void dubhe1000_init_interfaces(struct dubhe1000_adapter *adapter)
{
	int index, status;
	struct dubhe1000_mac *port;
	struct dubhe1000_pcs *pcs;

	if (readl_poll_timeout(adapter->fwd_pll_regs + FWD_TOP_PLL_PARA3_REG_OFFSET,
			       status, (status & BIT(0)), 10, 10000)) {
		pr_err("Line %d: wait PLL Lock status[%d] timeout\n", __LINE__, status);
		return;
	}

	/* Enable FWD TOP CLK SEL PARA */
	writel(FWD_CLK_SEL_VAL, adapter->fwd_pll_regs + FWD_CLK_PARA_REG_OFFSET);
	udelay(FWD_HOLD_TIME);

	/* RGMII TX NEED */
	writel(DIV_DEASSERT_VAL, adapter->rst_para_regs + DIV_RST_PARA_REG_OFFSET);
	udelay(FWD_HOLD_TIME);

	/*Enable FWD SUB CLK */
	writel(FWD_SUB_CLK_EN_VAL, adapter->cg_para_regs + FWD_SUB_CG_PARA_REG_OFFSET);
	udelay(FWD_HOLD_TIME);

	/* REST FWD */
	writel(FWD_ASSERT_VAL, adapter->rst_para_regs + FWD_APB_SUB_RST_REG_OFFSET);
	udelay(FWD_HOLD_TIME);

	/*De-ASSERT FWD APB*/
	writel(FWD_DEASSERT_CFG_VAL, adapter->rst_para_regs + FWD_APB_SUB_RST_REG_OFFSET);
	udelay(FWD_HOLD_TIME);

#ifdef CONFIG_DUBHE2000_PHYLINK
	for (index = 0; index < DUBHE1000_MAC_COUNT; index++) {
		port = adapter->mac[index];
		if (!port)
			continue;

		if (port->interface == XGMAC_INTERFACE_MODE_RGMII) {
			if (verbose)
				pr_info("Set XGMAC[%d] interface [RGMII]\n", port->id);
			writel(0x1, NPE_DUBHE1000_ETH_INTERFACE_OFFSET(adapter, port->id));

			writel(port->rgmii_rx_delay, XGMAC_PHY_RGMII_RX_DELAY(adapter, port->id));
			writel(port->rgmii_tx_delay, XGMAC_PHY_RGMII_TX_DELAY(adapter, port->id));

			writel(0x1, XGMAC_PHY_INTF_EN_REG_OFFSET(adapter, port->id));
		} else {
			if (verbose)
				pr_info("Set XGMAC[%d] interface [NON-RGMII]\n", port->id);
			writel(0x0, NPE_DUBHE1000_ETH_INTERFACE_OFFSET(adapter, port->id));
			if (port->id == 1) {
				if (verbose)
					pr_info("XGMAC[%d] switch to XPCS%d\n", port->id, port->pcs_sel);

				writel(!port->pcs_sel, adapter->top_regs + TEN_G_MODE_SEL);
			}
		}
	}
#endif

	writel(FWD_DEASSERT_FULL_VAL, adapter->rst_para_regs + FWD_APB_SUB_RST_REG_OFFSET);
	udelay(FWD_HOLD_TIME);

#ifdef CONFIG_DUBHE2000_PHYLINK
	/* init xpcs */
	for (index = 0; index < DUBHE1000_PCS_COUNT; index++) {
		pcs = adapter->pcs[index];
		if (!pcs)
			continue;

		if (verbose)
			pr_info("Set xpcs%d!", index);

		if (pcs->is_sfp != 2) {
			dubhe1000_serdes_load_and_modify(pcs);
			dubhe1000_set_pcs_mode(pcs);
		}
	}
#endif
}

/* Normally, caller can determine whether dev is NPE Ethernet dev by dev->if_port,
 * but during dev initialization, dev->if_port may not be determined, caller should
 * use this API to determine whether dev is NPE Ethernet dev
 */
bool netif_is_npe_eth_port(const struct net_device *dev)
{
	int i;
	struct dubhe1000_mac *port = netdev_priv(dev);

	for (i = 0; i < DUBHE1000_MAC_COUNT; i++) {
		if (port == g_adapter->mac[i])
			return true;
	}

	return false;
}
EXPORT_SYMBOL(netif_is_npe_eth_port);
/* dubhe1000_main.c */
