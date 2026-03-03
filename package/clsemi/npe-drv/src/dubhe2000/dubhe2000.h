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

/* Linux DUBHE1000 Driver main header file */

#ifndef _DUBHE1000_H_
#define _DUBHE1000_H_

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inet.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/bitops.h>
#include <linux/atomic.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/capability.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/pkt_sched.h>
#include <linux/list.h>
#include <linux/reboot.h>
#include <net/checksum.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#ifdef CONFIG_DUBHE2000_PHYLINK
#include <linux/phylink.h>
#elif defined(CONFIG_DUBHE2000_DEVLINK)
#include <net/devlink.h>
#else
#error "Please choice a phy management method"
#endif

struct dubhe1000_adapter;

#include "dubhe2000_hw.h"
#include "dubhe2000_edma.h"
#include "dubhe2000_switch.h"
#include "dubhe2000_fwd.h"
#include "dubhe2000_aging.h"
#include "../net/bridge/br_private.h"

#include "switch_api/flexswitch.h"
#include "switch_api/hashes.h"
#include "switch_api/common.h"
#include "switch_api/device_read_write.h"

#define DUBHE2000_CPU_PORT	       5
#define DUBHE2000_NON_EXIST_PORT       0xff
#define DUBHE1000_MAC_MONITOR_DELAY_MS 1000
#define DUBHE1000_MAC_COUNT	       5
#define DUBHE1000_PCS_COUNT	       2
#define DUBHE1000_MDIO_COUNT	       3
#define EDMA_HW			       1

#define DUBHE2000_BRIDGE_VRF   0
#define DUBHE2000_WAN_VRF      1
#define DUBHE2000_VRF_USED_MAX 2

#define DUBHE1000_HEAD_BLOCK_SIZE 256
#define DUBHE1000_BODY_BLOCK_SIZE 2048

#define DUBHE1000_PAGE_NUM 64

// granularity: 16, ex. the FIFO size = 16*16=256
#define DUBHE1000_LHOST_DEFAULT_FIFO_DEPTH    16
#define DUBHE1000_WMAC_24G_DEFAULT_FIFO_DEPTH 16
#define DUBHE1000_WMAC_5G_DEFAULT_FIFO_DEPTH  16
#define DUBHE1000_PCIE_DEFAULT_FIFO_DEPTH     16

#define DUBHE1000_DEFAULT_HEAD_OFFSET 32
#define DUBHE1000_DEFAULT_BODY_OFFSET 64

#define DUBHE1000_HW_BMU_EN 0

#define DUBHE1000_SWITCH_EN 1
#define DUBHE2000_PORT_MAX  5

#define DUBHE1000_TXD_CMD_IFCS 0x02000000 /* Insert FCS (Ethernet CRC) */

#define DUBHE1000_HEADROOM (NET_SKB_PAD + NET_IP_ALIGN)

/* TX/RX defines */
#define DUBHE1000_DEFAULT_TXD (16 * DUBHE1000_LHOST_DEFAULT_FIFO_DEPTH)
#define DUBHE1000_MAX_TXD     1024
#define DUBHE1000_MIN_TXD     48

#define DUBHE1000_DEFAULT_RXD (16 * DUBHE1000_LHOST_DEFAULT_FIFO_DEPTH)
#define DUBHE1000_MAX_RXD     1024
#define DUBHE1000_MIN_RXD     48

/* this is the size past which hardware will drop packets when setting LPE=0 */
#define MAXIMUM_ETHERNET_VLAN_SIZE 2008 // 576   // 1512   //1522

/* Supported Rx Buffer Sizes */
#define DUBHE1000_RXBUFFER_128	 128 /* Used for packet split */
#define DUBHE1000_RXBUFFER_256	 256 /* Used for packet split */
#define DUBHE1000_RXBUFFER_512	 512
#define DUBHE1000_RXBUFFER_1024	 1024
#define DUBHE1000_RXBUFFER_2048	 2048
#define DUBHE1000_RXBUFFER_4096	 4096
#define DUBHE1000_RXBUFFER_8192	 8192
#define DUBHE1000_RXBUFFER_16384 16384

/* How many Rx Buffers do we bundle into one write to the hardware ? */
#define DUBHE1000_RX_BUFFER_WRITE 16 /* Must be power of 2 */

#define DUBHE1000_MIRROR_FRAME_MAX_LEN 1600

#define DUBHE1000_AWCACHE_EN  0xF // 4'b1111
#define DUBHE1000_AWCACHE_DIS 0x3 // 4'b0011

#define DUBHE1000_RXD_SPC_VLAN_MASK 0x0FFF /* VLAN ID is in lower 12 bits */

/* HW BMU buffer pool */
#define DUBHE1000_HW_BMU_HEAD_BUF_MAX	  0x200000 // 32 * 256 * 192
#define DUBHE1000_HW_BMU_BUF_MAX(adapter) adapter->hw_bmu_buf_size

/* wrapper around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer
 */
struct dubhe1000_tx_buffer {
	struct sk_buff *skb;
	dma_addr_t dma;
	unsigned long time_stamp;
	u16 length;
	u16 next_to_watch;
	bool mapped_as_page;
	unsigned short segs;
	unsigned int bytecount;

	unsigned int rx_description1;
};

struct dubhe1000_rx_buffer {
	union {
		struct page *page; /* jumbo: alloc_page */
		u8 *data;	   /* else, netdev_alloc_frag */
	} rxbuf;
	dma_addr_t dma;
};

struct dubhe1000_tx_ring {
	/* physical address of the descriptor ring */
	dma_addr_t dma;
	/* length of descriptor ring in bytes */
	unsigned int size;
	/* number of descriptors in the ring */
	unsigned int count;
	/* next descriptor to associate a buffer with */
	unsigned int next_to_use;
	/* next descriptor to check for DD status bit */
	unsigned int next_to_clean;
	/* array of buffer information structs */
	struct dubhe1000_tx_buffer *buffer_info;

	u16 tdh;
	u16 tdt;
	bool last_tx_tso;
};

struct dubhe1000_rx_ring {
	/* physical address of the descriptor ring */
	dma_addr_t dma;
	/* length of descriptor ring in bytes */
	unsigned int size;
	/* number of descriptors in the ring */
	unsigned int count;
	/* next descriptor to associate a buffer with */
	unsigned int next_to_use;
	/* next descriptor to check for DD status bit */
	unsigned int next_to_clean;
	/* array of buffer information structs */
	struct dubhe1000_rx_buffer *buffer_info;
	struct dubhe1000_rx_buffer *head_buffer_info;
	void *hw_head_addr;
	void *hw_body_addr;

	/* cpu for rx queue */
	int cpu;
};

#define DUBHE1000_DESC_UNUSED(R) ({                                                                                    \
	unsigned int clean = smp_load_acquire(&(R)->next_to_clean);                                                    \
	unsigned int use = READ_ONCE((R)->next_to_use);                                                                \
	(clean > use ? 0 : (R)->count) + clean - use - 1;                                                              \
})

enum {
	HSGMII_AUTONEG_IGNORE_EN,
	HSGMII_AUTONEG_OFF,
	HSGMII_AUTONEG_ON,
};

/* board specific private data structure */
struct dubhe1000_pcs {
	u32 id;
	u32 usxg_mode;
	int irq;
	u32 is_sfp;
	bool link;
	void __iomem *ioaddr;
#ifdef CONFIG_DUBHE2000_PHYLINK
	phy_interface_t phy_interface;
	u32 autoneg;
	u32 hsgmii_autoneg;
#endif
	struct dubhe1000_adapter *adapter;
};

struct dubhe1000_mac {
	u32 id;
	int maxmtu;
	bool fp_bypass_qdisc;
	u32 is_sfp;
	u32 rgmii_rx_delay;
	u32 rgmii_tx_delay;
#ifdef CONFIG_DUBHE2000_PHYLINK
	int speed;
	int duplex;
	int link;
	int phy_addr;
	int pcs_sel;
	int pcs_port;
	int mdio_sel;
	int max_speed;
	int mac_monitor;
	int interface;
	phy_interface_t phy_interface;
	int mac_port_sel_speed;
	unsigned int flow_ctrl;
	unsigned int pause;
	u32 rx_queues_to_use;
	u32 tx_queues_to_use;
#endif
	char mac_addr[ETH_ALEN];
	void __iomem *ioaddr;
	struct device *device;
	struct net_device *netdev;
	struct net_device *dev_fp;
#ifdef CONFIG_DUBHE2000_PHYLINK
	struct device_node *phy_node;
	struct device_node *phylink_node;
	struct phy_device *phydev;
	struct phylink_config phylink_config;
	struct phylink_pcs phylink_pcs;
	struct phylink *phylink;
#elif defined(CONFIG_DUBHE2000_DEVLINK)
	struct devlink_port dl_port;
#else
#error "Please choice a phy management method"
#endif
	struct dubhe1000_adapter *adapter;
};

struct dubhe1000_adapter {
	struct dubhe1000_mac *mac[DUBHE1000_MAC_COUNT + 1];
	struct dubhe1000_pcs *pcs[DUBHE1000_PCS_COUNT];
	struct dubhe1000_mdio *mdio[DUBHE1000_MDIO_COUNT];
	struct device_node *mdio_node[DUBHE1000_MDIO_COUNT];
	char base_mac[ETH_ALEN];
	u32 port_count;
	u32 rx_buffer_len;
	u32 wol;

	unsigned int total_tx_bytes;
	unsigned int total_tx_packets;
	unsigned int total_tx_ack_bytes;
	unsigned int total_tx_ack_packets;
	unsigned int total_rx_bytes;
	unsigned int total_rx_packets;
	unsigned int total_fwt_packets;
	unsigned int total_l2_learning;
	unsigned int total_l2_directly;
	uint32_t hw_bmu_buf_size;

	/* TX */
	struct dubhe1000_tx_ring *tx_ring; /* One per active queue */
	spinlock_t tx_lock;		   /* lock used by tx reclaim and xmit */

	/* RX */
	bool (*clean_rx)(struct dubhe1000_adapter *adapter,
			 int channel,
			 struct dubhe1000_rx_ring *rx_ring,
			 int *work_done, int work_to_do);
	void (*alloc_rx_buf)(struct dubhe1000_adapter *adapter,
			     struct dubhe1000_rx_ring *rx_ring,
			     int cleaned_count);
	struct dubhe1000_rx_ring *rx_ring; /* One per active queue */
	struct napi_struct tx_napi;

	int num_tx_queues;
	int num_rx_queues;

	u32 alloc_rx_buff_failed;
	u32 rx_int_delay;

	/* OS defined structs */
	struct net_device napi_dev;
	struct platform_device *pdev;
	struct device *dev;

	/* Clock for AXI bus */
	int switch_irq;
	int tx_irq;
	int xpcs0_irq;
	int xpcs1_irq;

	int xgmac_irq[DUBHE1000_MAC_COUNT];

	int status_err_irq;
	int lmx_irq;

	u32 max_frame_size;
	u32 min_frame_size;
	u32 txq_status_num[4];

	struct workqueue_struct *wq;
	struct delayed_work time_work;
	struct delayed_work mac_monitor_work;
	refcount_t mac_monitor_ref;

	bool tx_poll_start;

	dma_addr_t mrc_dma;

	dma_addr_t head_dma;
	dma_addr_t body_dma;
	dma_addr_t txq_status_dma;

	void *head_hw_data;
	void *hw_data;

	/* IO registers, dma functions and IRQs */
	void __iomem *nvm_addr;

	void __iomem *edma_regs;
	void __iomem *edma_dbg_regs;
	void __iomem *lmx_regs;
	void __iomem *aging0_regs;
	void __iomem *aging1_regs;
	void __iomem *switch_regs;
	void __iomem *mrc_regs;
	void __iomem *acp_shaper_regs;
	void __iomem *cci_regs;
	void __iomem *fwd_pll_regs;
	void __iomem *rst_para_regs;
	void __iomem *cg_para_regs;
	void __iomem *top_regs;
	void __iomem *io_dr_regs;
	void __iomem *io_left_regs;
	void __iomem *io_right_regs;
	struct resource xgmac_pll_regs;

	bool soft_bmu_en;
	bool token_fc_en;
	bool head_awcachable;
	bool body_awcachable;

	/* 0: no split, 1: tag split, 2: l2 header, 3: l3 header, 4: l4 header*/
	u8 split_mode;
	u8 loopback_mode;
	u16 page_num;
	u32 lhost_fifo_depth;
	u32 wmac_24g_fifo_depth;
	u32 wmac_5g_fifo_depth;
	u32 pcie_fifo_depth;

	u32 head_offset;
	u32 body_offset;

	u32 head_block_size;
	u32 body_block_size;

	int msg_enable;
	atomic_long_t edma_unack_rx[4];
	u32 edma_rx_limit[4];
	u32 rx_intr_list[4];
	struct napi_struct edma_rx_napi[4];

#if defined(__ENABLE_FWD_TEST__)
	struct net_device emda_napi_dev[4];
	struct napi_struct edma_test_napi[4];
	uint tx_intr_list[4];
#endif
	spinlock_t stats64_lock; /* protects fwd statistics counters */
	u8 enable_from_cpu_tag;
	u8 tc_debug;
	u16 switch_cfg_delay;
	__be16 egress_user_tpid;
	u8 wan_port;
	u8 switch_pause_mod;
	u8 switch_pure_mod; // only used for Chip Testing
	struct {
		u8	tid:3,
			queue:3,
			configured:1;
	} dscp_map[64];
	struct {
		u8	tid:3,
			queue:3,
			configured:1;
	} tc_map[256];
	u8 queue_weight[DUBHE2000_PORT_MAX + 1][8];
};

#undef pr_fmt
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define e_err(msglvl, format, arg...)	 netif_err(port->adapter, msglvl, port->netdev, format, ##arg)
#define e_info(msglvl, format, arg...)	 netif_info(port->adapter, msglvl, port->netdev, format, ##arg)
#define e_warn(msglvl, format, arg...)	 netif_warn(port->adapter, msglvl, port->netdev, format, ##arg)
#define e_notice(msglvl, format, arg...) netif_notice(port->adapter, msglvl, port->netdev, format, ##arg)
#define e_dev_info(format, arg...)	 dev_info(&adapter->pdev->dev, format, ##arg)
#define e_dev_warn(format, arg...)	 dev_warn(&adapter->pdev->dev, format, ##arg)
#define e_dev_err(format, arg...)	 dev_err(&adapter->pdev->dev, format, ##arg)

#define DUBHE1000_SWITCH_ACCUM_ADDR_ENABLE 1

static inline int dubhe1000_if_port_to_switch_port(enum cls_fwt_port_id if_port)
{
	switch (if_port) {
	case CLS_FWT_PORT_ETH_0:
		return 0;
	case CLS_FWT_PORT_ETH_1:
		return 1;
	case CLS_FWT_PORT_ETH_2:
		return 2;
	case CLS_FWT_PORT_ETH_3:
		return 3;
	case CLS_FWT_PORT_ETH_4:
		return 4;
	default:
		return DUBHE2000_CPU_PORT;
	}
	return DUBHE2000_CPU_PORT;
}

static inline enum cls_fwt_port_id dubhe1000_switch_port_to_if_port(int id)
{
	switch (id) {
	case 0:
		return CLS_FWT_PORT_ETH_0;
	case 1:
		return CLS_FWT_PORT_ETH_1;
	case 2:
		return CLS_FWT_PORT_ETH_2;
	case 3:
		return CLS_FWT_PORT_ETH_3;
	case 4:
		return CLS_FWT_PORT_ETH_4;
	default:
		return CLS_FWT_PORT_LHOST;
	}
	return CLS_FWT_PORT_LHOST;
}

extern char cls_npe_driver_name[];

extern void __iomem *conf_base_addr;

extern void __iomem *debug_base_addr;

extern struct dubhe1000_adapter *g_adapter;
extern unsigned int verbose;
extern int switch_pause_mod;
extern u32 g_rx_intr;
extern u32 g_tx_intr;
extern int (*cls_skb_free)(struct sk_buff *skb);
extern int (*cls_is_bmu_skb)(struct sk_buff *skb);

#ifdef CONFIG_ARM_CCI
extern void __iomem *cci_ctrl_base __ro_after_init;
#endif

int rtl8367_init(struct mii_bus *mii);
void rtksdk_phy_init(void);
void rtk_phy_register(void);
void rtk_phy_unregister(void);

int dubhe1000_clean(struct napi_struct *napi, int budget);
int dubhe1000_skb_free(struct sk_buff *skb);
int dubhe1000_is_bmu_skb(struct sk_buff *skb);
void dubhe1000_dump_regs(struct dubhe1000_adapter *adapter, void __iomem *address);
void *dubhe1000_get_head_data(void *body);
void dubhe1000_set_ethtool_ops(struct net_device *netdev);
int dubhe1000_clean_tx_irq_loopback(struct dubhe1000_adapter *adapter, u32 total_tx);

int dubhe1000_setup_all_rx_resources(struct dubhe1000_adapter *adapter);
int dubhe1000_setup_all_tx_resources(struct dubhe1000_adapter *adapter);
void dubhe1000_free_all_rx_resources(struct dubhe1000_adapter *adapter);
void dubhe1000_free_all_tx_resources(struct dubhe1000_adapter *adapter);

struct dubhe1000_mac *dubhe1000_find_port(struct dubhe1000_adapter *adapter, u32 id);
void dubhe1000_rx_poll(struct dubhe1000_adapter *adapter, u8 start);
void dubhe1000_msg_set(struct dubhe1000_adapter *adapter, int port_id, u32 level);
void dubhe1000_loopback_init(struct dubhe1000_adapter *adapter, u8 value);
unsigned int dubhe1000_set_tx_thd(unsigned int thd);
void npe_rxmpud_data_dump(void);
void npe_tx_data_dump(void);
/*Reserved mac index for default_gw_da_mac*/
#define LAN_MAC_INX	0
#define WAN_MAC_INX	1
/*ACL1 TCAM Rule index*/
#define ACL1_TCAM_INX_IPV6_FRAG	0
#define ACL1_TCAM_INX_NO_IP_LAN	1
#define ACL1_TCAM_INX_NO_IP_WAN	2
#define ACL1_TCAM_INX_FTP_DPORT	3
#define ACL1_TCAM_INX_FTP_SPORT	4
#define ACL1_TCAM_INX_IPERF_TCP	5
/*reuse ACL1_TCAM_INX_IPERF_TCP for ACL1_TCAM_INX_IPERF_SRC*/
#define ACL1_TCAM_INX_IPERF_SRC	ACL1_TCAM_INX_IPERF_TCP
#define ACL1_TCAM_INX_IPERF_DST	6
#endif /* _DUBHE1000_H_ */
