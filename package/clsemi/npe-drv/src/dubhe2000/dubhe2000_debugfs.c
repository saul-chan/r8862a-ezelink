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

/*
 * Driver for Clourney DUBHE1000 network controller.
 *
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

#include "dubhe2000_debugfs.h"
#include "dubhe2000_switch.h"
#include "dubhe2000_switch_regs.h"
#include "dubhe2000_switch_stats.h"
#include "dubhe2000_edma_stats.h"
#include "dubhe2000_mac_stats.h"
#include "dubhe2000_aging.h"
#include "dubhe2000_tag.h"
#if defined(__ENABLE_FWD_TEST__)
#include "dubhe2000_fwd_test.h"
#endif

#include <net/neighbour.h>
#include <linux/netdevice.h>

static struct dentry *dubhe1000_dbg_root;

enum ring_type {
	RING_TYPE_RX,
	RING_TYPE_TX,
};

u32 g_max_txrx = 32;

/**************************************************************
 * command
 * The command entry in debugfs is for giving the driver commands
 * to be executed - these may be for changing the internal switch
 * setup, adding or removing filters, or other things.  Many of
 * these will be useful for some forms of unit testing.
 **************************************************************/
static char dubhe1000_dbg_command_buf[256] = "";

/**
 * dubhe1000_dbg_command_read - read for command datum
 * @filp: the opened file
 * @buffer: where to write the data for the user to read
 * @count: the size of the user's buffer
 * @ppos: file position offset
 **/
static ssize_t dubhe1000_dbg_command_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	int bytes_not_copied;
	int buf_size = 256;
	char *buf;
	int len;

	/* don't allow partial reads */
	if (*ppos != 0)
		return 0;

	if (count < buf_size)
		return -ENOSPC;

	buf = kzalloc(buf_size, GFP_KERNEL);
	if (!buf)
		return -ENOSPC;

	len = snprintf(buf, buf_size, "Result: %s\n", dubhe1000_dbg_command_buf);

	bytes_not_copied = copy_to_user(buffer, buf, len);
	kfree(buf);

	if (bytes_not_copied)
		return -EFAULT;

	*ppos = len;
	return len;
}
static int __parse_args(char *cmd_buf, char **argv, int max)
{
	int argc = 0;
	char *str_ptr = cmd_buf;
	bool is_para_left = true;

	while ('\0' != *str_ptr && argc < max) {
		if ('\t' == *str_ptr || ' ' == *str_ptr || '\n' == *str_ptr) {
			*str_ptr = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = str_ptr;
		}
		str_ptr++;
	}

	return argc;
}
/**
 * dubhe1000_dbg_dump_switch - handles dump port port_id write into command datum
 **/
static void dubhe1000_dbg_dump_switch(struct dubhe1000_adapter *adapter)
{
	int i;
	struct dubhe1000_rx_ring *rx_ring = NULL;
	struct dubhe1000_tx_ring *tx_ring = NULL;

	pr_info(" rx_intr %d tx_intr %d\n", g_rx_intr, g_tx_intr);

	dev_info(
		adapter->dev,
		"    net_stats: rx_packets = %u, rx_bytes = %u, tx_packets = %u, tx_bytes = %u, fwt_packets = %u, l2_learning = %u, l2_directly = %u\n",
		adapter->total_rx_packets, adapter->total_rx_bytes, adapter->total_tx_packets, adapter->total_tx_bytes,
		adapter->total_fwt_packets, adapter->total_l2_learning, adapter->total_l2_directly);

	rcu_read_lock();
	for (i = 0; i < adapter->num_rx_queues; i++) {
		rx_ring = &adapter->rx_ring[i];

		dev_info(adapter->dev, "    rx_rings[%i]: next_to_use = %d, next_to_clean = %d, count = %i size=%i\n",
			 i, rx_ring->next_to_use, rx_ring->next_to_clean, rx_ring->count, rx_ring->size);
	}

	for (i = 0; i < adapter->num_tx_queues; i++) {
		tx_ring = &adapter->tx_ring[i];

		dev_info(adapter->dev, "    tx_rings[%i]: next_to_use = %d, next_to_clean = %d, count = %i size=%i\n",
			 i, tx_ring->next_to_use, tx_ring->next_to_clean, tx_ring->count, tx_ring->size);
	}
	rcu_read_unlock();
}

/**
 * dubhe1000_dbg_dump_port - handles dump port port_id write into command datum
 * @port_id: the port_id the user put in
 **/
static void dubhe1000_dbg_dump_port(struct dubhe1000_adapter *adapter, int port_id)
{
	struct dubhe1000_mac *port;
	struct net_device *netdev = NULL;

	port = dubhe1000_find_port(adapter, port_id);

	dev_info(adapter->dev, "dump %d: port %px\n", port_id, port);
	if (!port) {
		dev_info(adapter->dev, "dump %d: port not found\n", port_id);
		return;
	}

	if (!port->netdev) {
		dev_info(adapter->dev, "dump %d: netdev not found\n", port_id);
		return;
	}

	netdev = port->netdev;

	dev_info(adapter->dev, "    netdev: name = %s, state = %lu, flags = 0x%08x mtu = %d\n",
		 netdev->name, netdev->state, netdev->flags, netdev->mtu);
	dev_info(adapter->dev, "        features      = 0x%08lx\n", (unsigned long)netdev->features);
	dev_info(adapter->dev, "        hw_features   = 0x%08lx\n", (unsigned long)netdev->hw_features);
	dev_info(adapter->dev, "        vlan_features = 0x%08lx\n", (unsigned long)netdev->vlan_features);

	// statistics
	dev_info(adapter->dev,
		 "	  net_stats: rx_packets = %lu, rx_bytes = %lu, rx_errors = %lu, rx_dropped = %lu\n",
		 netdev->stats.rx_packets, netdev->stats.rx_bytes, netdev->stats.rx_errors, netdev->stats.rx_dropped);

	dev_info(adapter->dev,
		 "	  net_stats: tx_packets = %lu, tx_bytes = %lu, tx_errors = %lu, tx_dropped = %lu\n",
		 netdev->stats.tx_packets, netdev->stats.tx_bytes, netdev->stats.tx_errors, netdev->stats.tx_dropped);
}

/**
 * dubhe1000_dbg_dump_port_all - handles dump adapter write into command datum
 * @adapter:
 **/
static void dubhe1000_dbg_dump_port_all(struct dubhe1000_adapter *adapter)
{
	int id;

	for (id = 0; id < DUBHE1000_MAC_COUNT; id++)
		dubhe1000_dbg_dump_port(adapter, id);
}

/**
 * dubhe1000_mac_reset_stats - handles dump port port_id write into command datum
 * @port_id: the port_id the user put in
 **/
static void dubhe1000_mac_reset_stats(struct dubhe1000_adapter *adapter, int port_id)
{
	struct dubhe1000_mac *port;
	struct net_device *netdev = NULL;

	if (!(port_id < DUBHE1000_MAC_COUNT))
		return;

	port = adapter->mac[port_id];
	if (!port) {
		dev_info(adapter->dev, "reset stats %d: port not found\n", port_id);
		return;
	}
	netdev = port->netdev;
	dev_info(adapter->dev, "reset stats port %d\n", port_id);

	// statistics
	netdev->stats.rx_packets = 0;
	netdev->stats.rx_bytes = 0;
	netdev->stats.rx_errors = 0;
	netdev->stats.rx_dropped = 0;
	netdev->stats.tx_packets = 0;
	netdev->stats.tx_bytes = 0;
	netdev->stats.tx_errors = 0;
	netdev->stats.tx_dropped = 0;

	writel(1, port->ioaddr + 0x800);
}

/**
 * dubhe1000_mac_reset_stats_all - handles dump adapter write into command datum
 * @adapter:
 **/
static void dubhe1000_mac_reset_stats_all(struct dubhe1000_adapter *adapter)
{
	int id;

	for (id = 0; id < DUBHE1000_MAC_COUNT; id++)
		dubhe1000_mac_reset_stats(adapter, id);
}

void dubhe1000_edma_split_config(struct dubhe1000_adapter *adapter)
{
	u32 val = 0;

	ew32(TX_SPLIT_REG, adapter->split_mode); // disable split

	if (adapter->split_mode) {
		if (adapter->head_awcachable)
			val = (DUBHE1000_AWCACHE_EN << HEAD_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << HEAD_AWCACHE_BIT);

		val |= (adapter->head_offset + DUBHE1000_HEADROOM);
		ew32(TX_HEAD_BLOCK_REG, val);

		if (adapter->body_awcachable)
			val = (DUBHE1000_AWCACHE_EN << BODY_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << BODY_AWCACHE_BIT);

		val |= (adapter->body_offset << BODY_BLOCK_OFFSET_BIT);
		ew32(TX_BODY_BLOCK_REG, val);

		// MRC region enable
		val = readl(adapter->mrc_regs + 0x100);
		pr_info(" mrc_regs %px %llx reg[0x100] = %x\n", adapter->mrc_regs, adapter->mrc_dma, val);

		writel(adapter->head_dma, adapter->mrc_regs + 0x200);
		// DUBHE1000_HW_BMU_HEAD_BUF_MAX
		writel(adapter->head_dma + 0x200000 - 1, adapter->mrc_regs + 0x300);
		writel(val | 0x1, adapter->mrc_regs + 0x100);
	} else {
		if (adapter->head_awcachable)
			val = (DUBHE1000_AWCACHE_EN << HEAD_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << HEAD_AWCACHE_BIT);

		ew32(TX_HEAD_BLOCK_REG, val);

		if (adapter->body_awcachable)
			val = (DUBHE1000_AWCACHE_EN << BODY_AWCACHE_BIT);
		else
			val = (DUBHE1000_AWCACHE_DIS << BODY_AWCACHE_BIT);

		val |= (adapter->body_offset + DUBHE1000_HEADROOM);

		ew32(TX_BODY_BLOCK_REG, val);

		// MRC region disable
		val = readl(adapter->mrc_regs + 0x100);
		pr_info(" mrc_regs %px %llx reg[0x100] = %x\n", adapter->mrc_regs, adapter->mrc_dma, val);
		writel(val & (~0x1), adapter->mrc_regs + 0x100);
	}

	val = (adapter->body_block_size << BODY_BLOCK_SIZE_BIT);
	ew32(TX_BASE_REG3, adapter->body_dma);

	/* 0: no split, 1: tag split, 2: l2 header, 3: l3 header, 4: l4 header*/
	if (adapter->split_mode) {
		ew32(TX_BASE_REG2, adapter->head_dma);
		val |= (adapter->head_block_size << HEAD_BLOCK_SIZE_BIT);
	}
	ew32(TX_BASE_REG1, val);
}

void dubhe1000_free_buffer_page(int page)
{
	int b = 0;
	u32 val;

	pr_info("buffer free page %d\n", page);
	for (b = 0; b < 256; b++) {
		val = b;
		val += (page << FREE_TX_BUF_PID_BIT);
		val += (1 << FREE_TX_BUF_FREE_EN_BIT);
		ew32(TX_BUF_FREE_INSTR, val);
	}
}

void dubhe1000_dump_regs(struct dubhe1000_adapter *adapter, void __iomem *address)
{
	u32 data[8] = { 0 };

	data[0] = readl(address);
	data[1] = readl(address + 4 * 1);
	data[2] = readl(address + 4 * 2);
	data[3] = readl(address + 4 * 3);
	data[4] = readl(address + 4 * 4);
	data[5] = readl(address + 4 * 5);
	data[6] = readl(address + 4 * 6);
	data[7] = readl(address + 4 * 7);
	dev_info(adapter->dev, "address 0x%px: %08x %08x %08x %08x %08x %08x %08x %08x\n", (u64 *)address,
		 data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
}

#ifdef CONFIG_DUBHE2000_PHYLINK
static int extract_range(char *input, int *plo, int *phi)
{
	char *end;
	*plo = simple_strtol(input, &end, 16);
	if (end == input)
		return -1;

	if ((*end == '-') && *(++end))
		*phi = simple_strtol(end, NULL, 16);
	else if (*end == '\0')
		*phi = *plo;
	else
		return -1;

	return 0;
}

/* The register will be in the form [a[-b].]x[-y] */
static int extract_reg_range(char *input, int *devadlo, int *devadhi, int *reglo, int *reghi)
{
	char *regstr;

	/* use strrchr to find the last string after a '.' */
	regstr = strrchr(input, '.');

	/* If it exists, extract the devad(s) */
	if (regstr) {
		char devadstr[32];

		strncpy(devadstr, input, regstr - input);
		devadstr[regstr - input] = '\0';

		if (extract_range(devadstr, devadlo, devadhi))
			return -1;

		regstr++;
	} else {
		/* Otherwise, we have no devad, and we just got regs */
		*devadlo = *devadhi = MDIO_DEVAD_NONE;

		regstr = input;
	}

	return extract_range(regstr, reglo, reghi);
}

uint32_t hextoul(const char *cp, char **endp)
{
	return simple_strtoul(cp, endp, 16);
}

static int do_phy(struct dubhe1000_adapter *adapter, char *cmd)
{
#define PARA_MAX_NUM 10
	char op[2];
	int reglo, reghi, devadlo, devadhi;
	unsigned short data;
	int pos;
	uint32_t xgmac_index = 0;
	struct phy_device *phydev;
	int argc = 0;
	bool is_para_left = true;
	char *argv[PARA_MAX_NUM] = { 0 };
	char *str_ptr = cmd;

	while ('\0' != *str_ptr && argc < PARA_MAX_NUM) {
		if ('\t' == *str_ptr || ' ' == *str_ptr || '\n' == *str_ptr) {
			*str_ptr = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = str_ptr;
		}
		str_ptr++;
	}

	if (argc < 2) {
		printk(KERN_ERR "%s ARG error", __func__);
		return -1;
	}

	pos = argc - 1;

	op[0] = argv[1][0];

	xgmac_index = argv[0][3] - '0';
	if (xgmac_index >= DUBHE1000_MAC_COUNT || !adapter->mac[xgmac_index]) {
		printk(KERN_ERR "%s err  phy index %s", __func__, argv[0]);
		return -1;
	}

	switch (op[0]) {
	case 'w':
		if (pos > 1)
			data = hextoul(argv[pos--], NULL);
		/* Intentional fall-through - Get reg for read and write */
	case 'r':
		if (pos > 1)
			if (extract_reg_range(argv[pos--], &devadlo, &devadhi, &reglo, &reghi))
				return -1;
		/* Intentional fall-through - Get phy for all commands */
	default:

		break;
	}

	phydev = adapter->mac[xgmac_index]->phydev;
	if (!phydev) {
		printk(KERN_ERR "No MDIO bus found\n");
		return -1;
	}

	switch (op[0]) {
	case 'w':
		if (devadlo == MDIO_DEVAD_NONE)
			phy_write(phydev, reglo, data);
		else
			phy_write(phydev, mdiobus_c45_addr(devadlo, reglo), data);
		break;

	case 'r':
		if (devadlo == MDIO_DEVAD_NONE) {
			data = phy_read(phydev, reglo);
			printk(KERN_ERR "%#x: %#x\n", reglo, data);
		} else {
			data = phy_read(phydev, mdiobus_c45_addr(devadlo, reglo));
			printk(KERN_ERR "%#x.%#x: %#x\n", devadlo, reglo, data);
		}
		break;
	}

	return 0;
}
#endif

/**
 * dubhe1000_dbg_command_write - write into command datum
 **/
static ssize_t dubhe1000_dbg_command_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;

	int port_id = 0;
	char *cmd_buf, *cmd_buf_tmp;
	int bytes_not_copied;
	u32 address = 0, value = 0;
	int cnt;
	int len = 0, k, k2;
	u8 data2[16] = { 0 };
	int index = 0;
	u16 vrf = 0, session_id = 0;
	u8 option = 0;
	char module_name[16];

	/* don't allow partial writes */
	if (*ppos != 0)
		return 0;

	cmd_buf = kzalloc(count + 1, GFP_KERNEL);
	if (!cmd_buf)
		return count;
	bytes_not_copied = copy_from_user(cmd_buf, buffer, count);
	if (bytes_not_copied) {
		kfree(cmd_buf);
		return -EFAULT;
	}
	cmd_buf[count] = '\0';

	cmd_buf_tmp = strchr(cmd_buf, '\n');
	if (cmd_buf_tmp) {
		*cmd_buf_tmp = '\0';
		count = cmd_buf_tmp - cmd_buf + 1;
	}

	if (strncmp(cmd_buf, "msg level", 9) == 0) {
		cnt = sscanf(&cmd_buf[10], "%i %x", &port_id, &value);
		if (cnt != 2) {
			pr_info("msg level <msg-level> cnt %d\n", cnt);
			goto command_write_done;
		} else {
			dubhe1000_msg_set(adapter, port_id, value);
		}
	} else if (strncmp(cmd_buf, "fp", 2) == 0) {
		char dev_src[32], dev_dst[32];
		int port_id;
		struct dubhe1000_mac *port;
		u32 found = 0;
#if defined(BOC_THROUGH_PUT_OPTIMIZE) && BOC_THROUGH_PUT_OPTIMIZE
		bool bypass_qdisc = true;
#else
		bool bypass_qdisc = false;
#endif

		memset(dev_src, 0, sizeof(dev_src));
		memset(dev_dst, 0, sizeof(dev_dst));
		cnt = sscanf(&cmd_buf[3], "%s %s", dev_src, dev_dst);
		if (cnt != 2) {
			pr_err("invalid dev_src or dev_dst\n");
			goto command_write_done;
		}

		if (strstr(&cmd_buf[3], "bypass_qdisc_en"))
			bypass_qdisc = true;
		else if (strstr(&cmd_buf[3], "bypass_qdisc_dis"))
			bypass_qdisc = false;

		for (port_id = 0; port_id < DUBHE1000_MAC_COUNT; port_id++) {
			port = adapter->mac[port_id];
			if (!port) {
				dev_info(adapter->dev, " %d: port not found\n", port_id);
				continue;
			}

			if (!strcmp(dev_src, port->netdev->name)) {
				port->fp_bypass_qdisc = bypass_qdisc;
				port->dev_fp = dev_get_by_name(&init_net, dev_dst);
				if (port->dev_fp) {
					dev_put(port->dev_fp);
					pr_warn("%s %d, fp from [%s] to [%s].\n", __func__, __LINE__, dev_src, dev_dst);
				} else {
					pr_warn("%s %d, des dev [%s] not found.\n", __func__, __LINE__, dev_dst);
				}
				found = 1;
				break;
			}
		}

		if (!found)
			pr_warn("%s %d, src dev [%s] not found.\n", __func__, __LINE__, dev_src);
	} else if (strncmp(cmd_buf, "dump", 4) == 0) {
		if (strncmp(&cmd_buf[5], "switch", 6) == 0) {
			dubhe1000_dbg_dump_switch(adapter);
		} else if (strncmp(&cmd_buf[5], "port", 4) == 0) {
			cnt = sscanf(&cmd_buf[10], "%i", &port_id);
			if (cnt > 0)
				dubhe1000_dbg_dump_port(adapter, port_id);
			else
				dubhe1000_dbg_dump_port_all(adapter);
		} else {
			dev_info(adapter->dev, "dump switch\n");
			dev_info(adapter->dev, "dump port [port_id]\n");
		}
	} else if (strncmp(cmd_buf, "clear_stats", 11) == 0) {
		if (strncmp(&cmd_buf[12], "port", 4) == 0) {
			cnt = sscanf(&cmd_buf[17], "%i", &port_id);
			if (cnt == 0) {
				dubhe1000_mac_reset_stats_all(adapter);
				dev_info(adapter->dev, "clear stats called for all port's\n");
			} else if (cnt == 1) {
				if (!(port_id < DUBHE1000_MAC_COUNT)) {
					dev_info(adapter->dev, "clear_stats port: bad port %d\n", port_id);
					goto command_write_done;
				}
				dubhe1000_mac_reset_stats(adapter, port_id);
				dev_info(adapter->dev, "port clear stats called for port %d\n", port_id);
			} else {
				dev_info(adapter->dev, "clear_stats port [port_id]\n");
			}
		} else {
			dev_info(adapter->dev, "clear_stats port [port_id]\n");
		}
	} else if (strncmp(cmd_buf, "nvm read", 8) == 0) {
		cnt = sscanf(&cmd_buf[8], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "nvm read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "nvm read: 0x%08x len: %d\n", address, value);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->hw_data + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->hw_data + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->hw_data + address + 32 * k);
		} else {
			value = readl(adapter->hw_data + address);
			dev_info(adapter->dev, "nvm read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "nvm head read", 13) == 0) {
		cnt = sscanf(&cmd_buf[13], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "nvm body read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "nvm body read: 0x%08x len: %d\n", address, value);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->head_hw_data + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->head_hw_data + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->head_hw_data + address + 32 * k);
		} else {
			value = readl(adapter->head_hw_data + address);
			dev_info(adapter->dev, "nvm body read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "mrc read", 8) == 0) {
		cnt = sscanf(&cmd_buf[8], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "mrc read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "mrc read: 0x%08x len: %d\n", address, len);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->mrc_regs + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->mrc_regs + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->mrc_regs + address + 32 * k);
		} else {
			value = readl(adapter->mrc_regs + address);
			dev_info(adapter->dev, "mrc read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "mrc write", 9) == 0) {
		cnt = sscanf(&cmd_buf[9], "%i %i", &address, &value);
		if (cnt != 2) {
			dev_info(adapter->dev, "mrc write <reg_addr> <value>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "mrc write: 0x%08x = 0x%08x\n", address, value);

		writel(value, adapter->mrc_regs + address);
		value = readl(adapter->mrc_regs + address);

		dev_info(adapter->dev, "mrc read: 0x%08x = 0x%08x\n", address, value);
	} else if (strncmp(cmd_buf, "edma read", 9) == 0) {
		cnt = sscanf(&cmd_buf[9], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "edma read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "edma read: 0x%08x len: %d\n", address, len);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->edma_regs + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->edma_regs + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->edma_regs + address + 32 * k);
		} else {
			value = readl(adapter->edma_regs + address);
			dev_info(adapter->dev, "edma read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "tid ", 4) == 0) {
#define CONVERT_TO_UINT(val) ({                                                                                        \
	if (i + 1 >= argc) {                                                                                           \
		printk(KERN_ERR "err arg num!");                                                                       \
		break;                                                                                                 \
	}                                                                                                              \
	val = simple_strtoul(argv[++i], NULL, 0);                                                                      \
})

#define CONVERT_TO_U64(val) ({                                                                                         \
	if (i + 1 >= argc) {                                                                                           \
		printk(KERN_ERR "err arg num!");                                                                       \
		break;                                                                                                 \
	}                                                                                                              \
	val = simple_strtoull(argv[++i], NULL, 0);                                                                     \
})
		char *_argv[10] = { 0 };
		int argc = 0, i = 0;
#ifdef TID_TEST_EN
		int bit;
#endif
		uint32_t val = 0, map = 0, group = 0;
		uint64_t map64 = 0;
		char *name = NULL;
		char **argv = _argv;

		argc = __parse_args(cmd_buf, argv, 10);
		argc--;
		argv++;
		for (i = 0; i < argc; i++) {
			name = argv[i];
			if (strcasecmp("BYPASS", name) == 0) {
				CONVERT_TO_UINT(val);
				ew32(TID_BYPASS, val);
			} else if (strcasecmp("DEF", name) == 0) {
				CONVERT_TO_UINT(val);
				ew32(DEF_TID, val);
			} else if (strcasecmp("QOS_SEL", name) == 0) {
				CONVERT_TO_UINT(val);
				ew32(QOS_SEL, val);
			} else {
				if (strcasecmp("DOTIP", name) == 0) {
					CONVERT_TO_UINT(map);
					CONVERT_TO_UINT(val);
#ifdef TID_TEST_EN
					for (bit = 0; bit < 8; bit++) {
						if (!((1 << bit) & map))
							continue;
						DUBHE1000_WRITE_REG_ARRAY(DOT1P_TID, bit * 0x4, val);
					}
#else
					DUBHE1000_WRITE_REG_ARRAY(DOT1P_TID, map * 0x4, val);
#endif
				} else if (strcasecmp("DSCP", name) == 0) {
					CONVERT_TO_U64(map64);
					CONVERT_TO_UINT(val);
#ifdef TID_TEST_EN
					for (bit = 0; bit < 64; bit++) {
						if (!((1 << bit) & map64))
							continue;
						DUBHE1000_WRITE_REG_ARRAY(DSCP_TID, bit * 0x4, val);
					}
#else
					DUBHE1000_WRITE_REG_ARRAY(DSCP_TID, map64 * 0x4, val);
#endif
				} else if (strcasecmp("TC", name) == 0) {
					CONVERT_TO_U64(group);
#ifdef TID_TEST_EN
					CONVERT_TO_U64(map64);
					CONVERT_TO_UINT(val);
					for (bit = 0; bit < 64; bit++) {
						if (!((1 << bit) & map64))
							continue;
						DUBHE1000_WRITE_REG_ARRAY(TC_TID, ((group * 64) + bit) * 0x4, val);
					}
#else
					CONVERT_TO_UINT(val);
					DUBHE1000_WRITE_REG_ARRAY(TC_TID, group * 0x4, val);
#endif
				}
			}
		}
#if defined(__ENABLE_FWD_TEST__)
	} else if (strncmp(cmd_buf, "from_cpu_tag", 12) == 0) {
		char *_argv[20] = { 0 };
		int argc = 0, i = 0, mac_id = 0;
		char *name = NULL, *value = NULL;
		char **argv = _argv;

		argc = __parse_args(cmd_buf, argv, 20);
		argc--;
		argv++;
		if (argc && (strcasecmp(argv[0], "print") == 0)) {
			for (i = 0; i < DUBHE1000_MAC_COUNT; i++) {
				printk(KERN_ERR "Port: %d", i);
				printk(KERN_ERR "%20s: %s", "enable", g_from_cpuTag_bitmap & BIT(i) ? "True" : "False");
				printk(KERN_ERR "%20s: %#x", "ethernet_type", g_from_cpu_tag[i].ethernet_type);
				printk(KERN_ERR "%20s: %#x", "port_mask", g_from_cpu_tag[i].port_mask);
				printk(KERN_ERR "%20s: %#x", "queue_ind", g_from_cpu_tag[i].queue_ind);
				printk(KERN_ERR "%20s: %#x", "modified_ind", g_from_cpu_tag[i].modified_ind);
				printk(KERN_ERR "%20s: %#x", "signal", g_from_cpu_tag[i].signal);
				printk(KERN_ERR "%20s: %#llx\n", "timestamps",
				       *((uint64_t *)g_from_cpu_tag[i].timestamps));
			}
			goto command_write_done;
		}

		if (argc >= 2) {
			if (strcasecmp(argv[0], "sport")) {
				printk(KERN_ERR "sport Not Found\n");
				goto command_write_done;
			}

			mac_id = simple_strtol(argv[1], NULL, 0);
			if (mac_id >= DUBHE1000_MAC_COUNT)
				printk(KERN_ERR "ERR ARG0\n");

			argc -= 2;
			argv += 2;
		}

		if (argc % 2) {
			printk(KERN_ERR "ERR ARG\n");
			goto command_write_done;
		}

		for (i = 0; i < argc; i += 2) {
			name = argv[i];
			value = argv[i + 1];
			if (strcasecmp(name, "enable") == 0) {
				if (simple_strtol(value, NULL, 0))
					g_from_cpuTag_bitmap |= BIT(mac_id);
				else
					g_from_cpuTag_bitmap &= ~BIT(mac_id);
				g_from_cpu_tag[mac_id].ethernet_type = DUBHE1000_FROM_CPU_ETHE_TYPE;
			} else if (strcasecmp(name, "portMask") == 0) {
				g_from_cpu_tag[mac_id].port_mask = simple_strtol(value, NULL, 0);
			} else if (strcasecmp(name, "queue") == 0) {
				g_from_cpu_tag[mac_id].queue_ind = simple_strtol(value, NULL, 0);
			} else if (strcasecmp(name, "modify") == 0) {
				g_from_cpu_tag[mac_id].modified_ind = simple_strtol(value, NULL, 0);
			} else if (strcasecmp(name, "signal") == 0) {
				g_from_cpu_tag[mac_id].signal = simple_strtol(value, NULL, 0);
			} else if (strcasecmp(name, "timestamps") == 0) {
				*((uint64_t *)g_from_cpu_tag[mac_id].timestamps) = simple_strtoull(value, NULL, 0);
			}
		}
#endif
	} else if (strncmp(cmd_buf, "from_cpu_tag", 12) == 0) {
		cnt = sscanf(&cmd_buf[12], "%i", &value);
		if (cnt != 1)
			dev_info(adapter->dev, "from_cpu_tag <0/1>\n");
		else {
			adapter->enable_from_cpu_tag = !!value;
			dev_info(adapter->dev, "from_cpu_tag %s\n",
				 adapter->enable_from_cpu_tag ? "enable" : "disable");
		}
	} else if (strncmp(cmd_buf, "edma write", 10) == 0) {
		cnt = sscanf(&cmd_buf[10], "%i %i", &address, &value);
		if (cnt != 2) {
			dev_info(adapter->dev, "edma write <reg_addr> <value>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "edma write: 0x%08x = 0x%08x\n", address, value);

		writel(value, adapter->edma_regs + address);
		value = readl(adapter->edma_regs + address);

		dev_info(adapter->dev, "edma read: 0x%08x = 0x%08x\n", address, value);
	} else if (strncmp(cmd_buf, "edma clear_stats", 16) == 0) {
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50E4);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50E8);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50EC);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50F0);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50F4);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50F8);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x50FC);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5100);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5104);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5108);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5110);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5114);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5118);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x511c);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5120);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5124);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5128);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5130);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5134);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5138);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x513c);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5140);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5144);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x517c);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5180);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5184);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5188);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5154);
		writel(0xffffffff, adapter->edma_dbg_regs + 0x5158);
		dev_info(adapter->dev, "edma  clear_stats Success!!\n");
	} else if (strncmp(cmd_buf, "edma_dbg read", 13) == 0) {
		cnt = sscanf(&cmd_buf[13], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "edma_dbg read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			k2 = len / 16;
			dev_info(adapter->dev, "edma_dbg read: 0x%08x len: %d k2 %d\n", address, len, k2);
			dubhe1000_dump_regs(adapter, adapter->edma_dbg_regs + address);
			for (k = 0; k < k2; k++) {
				if (k != 0)
					dubhe1000_dump_regs(adapter, adapter->edma_dbg_regs + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->edma_dbg_regs + address + 32 * k);
		} else {
			value = readl(adapter->edma_dbg_regs + address);
			dev_info(adapter->dev, "edma_dbg read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "edma_dbg write", 14) == 0) {
		cnt = sscanf(&cmd_buf[14], "%i %i", &address, &value);
		if (cnt != 2) {
			dev_info(adapter->dev, "edma_dbg write <reg_addr> <value>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "edma_dbg write: 0x%08x = 0x%08x\n", address, value);

		writel(value, adapter->edma_dbg_regs + address);
		value = readl(adapter->edma_dbg_regs + address);

		dev_info(adapter->dev, "edma_dbg read: 0x%08x = 0x%08x\n", address, value);
	} else if (strncmp(cmd_buf, "edma_dbg stats", 14) == 0) {
		cnt = sscanf(&cmd_buf[14], "%hhi", &option);
		if (cnt != 1)
			option = EDMA_STATS_OPTION_ALL;

		dubhe1000_edma_stats_dump(adapter, option);
	} else if (strncmp(cmd_buf, "edma split", 10) == 0) {
		cnt = sscanf(&cmd_buf[10], "%i", &value);
		if (cnt != 1) {
			dev_info(adapter->dev, "edma split <val>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "edma split: %d\n", value);
		adapter->split_mode = value;
		dubhe1000_edma_split_config(adapter);
	} else if (strncmp(cmd_buf, "switch read", 11) == 0) {
		cnt = sscanf(&cmd_buf[11], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "switch read <reg_addr>\n");
			goto command_write_done;
		}
		if (cnt == 2) {
			dev_info(adapter->dev, "switch read: 0x%08x len: %d\n", address, value);
			k2 = len / 8;
			address = 4 * address;
			dubhe1000_dump_regs(adapter, adapter->switch_regs + address);
			for (k = 0; k < k2; k++) {
				if (k != 0)
					dubhe1000_dump_regs(adapter, adapter->switch_regs + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->switch_regs + address + 32 * k);
		} else {
			value = readl(adapter->switch_regs + 4 * address);
			dev_info(adapter->dev, "switch read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "switch write", 12) == 0) {
		cnt = sscanf(&cmd_buf[12], "%i %i", &address, &value);
		if (cnt != 2) {
			dev_info(adapter->dev, "switch write <reg_addr> <value>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "switch write: 0x%08x = 0x%08x\n", address, value);
		address = 4 * address;
		writel(value, adapter->switch_regs + address);
		value = readl(adapter->switch_regs + address);

		dev_info(adapter->dev, "switch read: 0x%08x = 0x%08x\n", address, value);
	} else if (strncmp(cmd_buf, "switch stats", 12) == 0) {
		cnt = sscanf(&cmd_buf[12], "%hhi", &option);
		if (cnt != 1)
			option = SWITCH_STATS_OPTION_ALL;

		dubhe1000_switch_stats_dump(adapter, option);
	} else if (strncmp(cmd_buf, "switch clear_stats", 12) == 0) {
		dubhe1000_switch_clear_stats(adapter);
	} else if (strncmp(cmd_buf, "router port mac", 15) == 0) {
		if (strncmp(&cmd_buf[16], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[20], "%hhx: %hhx: %hhx: %hhx: %hhx: %hhx %hi",
				     &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5], &vrf);
			if (cnt != 7) {
				dev_info(adapter->dev, "router port mac add <mac> <vrf>\n");
				goto command_write_done;
			} else {
				dubhe1000_router_port_macAddr_table_add(adapter, data2, NULL, vrf);
			}
		} else if (strncmp(&cmd_buf[16], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[20], "%hhx: %hhx: %hhx: %hhx: %hhx: %hhx %hi",
				     &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5], &vrf);
			if (cnt != 7) {
				dev_info(adapter->dev, "router port mac del <mac> <vrf>\n");
				goto command_write_done;
			} else {
				dubhe1000_router_port_macAddr_table_del(adapter, data2, NULL, vrf);
			}
		}
	} else if (strncmp(cmd_buf, "l3 route default", 16) == 0) {
		if (strncmp(&cmd_buf[17], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[21], "%hhi %hhi %hhi", &data2[0], &data2[1], &data2[2]);
			if (cnt != 3) {
				dev_info(adapter->dev, "l3 route default add <nextHop> <pktDrop> <sendToCPU>\n");
				goto command_write_done;
			} else {
				dubhe1000_l3_routing_default_table_op(adapter, data2[0], data2[1], data2[2], true);
			}
		} else if (strncmp(&cmd_buf[17], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[21], "%hhi %hhi %hhi", &data2[0], &data2[1], &data2[2]);
			if (cnt != 3) {
				dev_info(adapter->dev, "l3 route default del <nextHop> <pktDrop> <sendToCPU>\n");
				goto command_write_done;
			} else {
				dubhe1000_l3_routing_default_table_op(adapter, data2[0], data2[1], data2[2], false);
			}
		}
	} else if (strncmp(cmd_buf, "ingr egr pkt type filter", 24) == 0) {
		if (strncmp(&cmd_buf[25], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[29], "%hhi %hhi", &data2[0], &data2[1]);
			if (cnt != 2) {
				dev_info(adapter->dev, "ingr egr pkt type filter add <egr_port> <filter_port>\n");
				goto command_write_done;
			} else {
				dubhe1000_ingr_egr_port_pkt_type_filter_table_op(adapter, data2[0], data2[1], true);
			}
		} else if (strncmp(&cmd_buf[25], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[29], "%hhi %hhi", &data2[0], &data2[1]);
			if (cnt != 2) {
				dev_info(adapter->dev, "ingr egr pkt type filter  del <egr_port> <filter_port>\n");
				goto command_write_done;
			} else {
				dubhe1000_ingr_egr_port_pkt_type_filter_table_op(adapter, data2[0], data2[1], false);
			}
		}
	} else if (strncmp(cmd_buf, "next hop", 8) == 0) {
		if (strncmp(&cmd_buf[9], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[13], "%hhi %hhi %hhi %hhi %hhi %hhi",
				     &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5]);

			if (cnt != 6) {
				dev_info(
					adapter->dev,
					"next hop add <index> <nextHopPacketMod> <destPortNum> <isUc> <drop> <toCPU>\n");
				goto command_write_done;
			} else {
				dubhe1000_next_hop_table_op(adapter, data2[0], data2[1], data2[2], data2[3], data2[4],
							    data2[5], true);
			}
		} else if (strncmp(&cmd_buf[9], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[13], "%hhi", &data2[0]);
			if (cnt != 1) {
				dev_info(adapter->dev, "next hop del <index>\n");
				goto command_write_done;
			} else {
				dubhe1000_next_hop_table_op(adapter, data2[0], 0, 0, 0, 0, 0, false);
			}
		}
	} else if (strncmp(cmd_buf, "pkt modify", 10) == 0) {
		if (strncmp(&cmd_buf[11], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[15], "%hhi", &data2[0]);
			if (cnt != 1) {
				dev_info(adapter->dev, "pkt modify add <index>\n");
				goto command_write_done;
			} else {
				dubhe1000_next_hop_packet_modify_table_op(adapter, data2[0], true);
			}
		} else if (strncmp(&cmd_buf[11], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[15], "%hhi", &data2[0]);
			if (cnt != 1) {
				dev_info(adapter->dev, "pkt modify del <index>\n");
				goto command_write_done;
			} else {
				dubhe1000_next_hop_packet_modify_table_op(adapter, data2[0], false);
			}
		}
	} else if (strncmp(cmd_buf, "nxt hop da mac", 14) == 0) {
		if (strncmp(&cmd_buf[15], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[19], "%i %hhx: %hhx: %hhx: %hhx: %hhx: %hhx",
				     &index, &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5]);
			if (cnt != 7) {
				dev_info(adapter->dev, "nxt hop da mac add <index> <daMac>\n");
				goto command_write_done;
			} else {
				dubhe1000_next_hop_da_mac_table_op(adapter, index, data2, true);
			}
		} else if (strncmp(&cmd_buf[15], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[19], "%i %hhx: %hhx: %hhx: %hhx: %hhx: %hhx",
				     &index, &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5]);
			if (cnt != 7) {
				dev_info(adapter->dev, "nxt hop da mac del <index> <daMac>\n");
				goto command_write_done;
			} else {
				dubhe1000_next_hop_da_mac_table_op(adapter, index, data2, false);
			}
		}
	} else if (strncmp(cmd_buf, "ipv4 router", 11) == 0) {
		if (strncmp(&cmd_buf[12], "add", 3) == 0) {
			cnt = sscanf(&cmd_buf[16], "%hi %hhu. %hhu. %hhu. %hhu %hhi",
				     &vrf, &data2[0], &data2[1], &data2[2], &data2[3], &data2[4]);

			if (cnt != 6) {
				dev_info(adapter->dev, "ipv4 router add <vrf> <destIp> <nextHopPointer>\n");
				goto command_write_done;
			} else {
				dubhe1000_hash_based_l3_routing_table_op(adapter, false, vrf, &data2[0], data2[4],
									 true);
			}
		} else if (strncmp(&cmd_buf[12], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[16], "%hi %hhu. %hhu. %hhu. %hhu %hhi",
				     &vrf, &data2[0], &data2[1], &data2[2], &data2[3], &data2[4]);
			if (cnt != 6) {
				dev_info(adapter->dev, "ipv4 router del <vrf> <destIp> <nextHopPointer>\n");
				goto command_write_done;
			} else {
				dubhe1000_hash_based_l3_routing_table_op(adapter, false, vrf, &data2[0], data2[4],
									 false);
			}
		}
	} else if (strncmp(cmd_buf, "ingr router", 11) == 0) {
		if (strncmp(&cmd_buf[12], "add", 3) == 0)
			dubhe1000_ingress_router_table_op(adapter, 0, true);
		else if (strncmp(&cmd_buf[12], "del", 3) == 0)
			dubhe1000_ingress_router_table_op(adapter, 0, false);
	} else if (strncmp(cmd_buf, "xgmac0 read", 11) == 0 || strncmp(cmd_buf, "xgmac1 read", 11) == 0 ||
		   strncmp(cmd_buf, "xgmac2 read", 11) == 0 || strncmp(cmd_buf, "xgmac3 read", 11) == 0 ||
		   strncmp(cmd_buf, "xgmac4 read", 11) == 0) {
		u8 xgmac_index = cmd_buf[5] - '0';

		cnt = sscanf(&cmd_buf[11], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "xgmac%d read <reg_addr>\n", xgmac_index);
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "xgmac%d read: 0x%08x len: %d\n", xgmac_index, address, value);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->mac[xgmac_index]->ioaddr + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;
				dubhe1000_dump_regs(adapter, adapter->mac[xgmac_index]->ioaddr + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->mac[xgmac_index]->ioaddr + address + 32 * k);
		} else {
			value = readl(adapter->mac[xgmac_index]->ioaddr + address);
			dev_info(adapter->dev, "xgmac%d read: 0x%08x = 0x%08x\n", xgmac_index, address, value);
		}
	} else if (strncmp(cmd_buf, "xgmac0 write", 12) == 0 || strncmp(cmd_buf, "xgmac1 write", 12) == 0 ||
		   strncmp(cmd_buf, "xgmac2 write", 12) == 0 || strncmp(cmd_buf, "xgmac3 write", 12) == 0 ||
		   strncmp(cmd_buf, "xgmac4 write", 12) == 0) {
		u8 xgmac_index = cmd_buf[5] - '0';

		cnt = sscanf(&cmd_buf[12], "%i %i", &address, &value);
		if (cnt != 2 || !adapter->mac[xgmac_index]) {
			dev_info(adapter->dev, "xgmac%d write <reg_addr> <value>\n", xgmac_index);
			goto command_write_done;
		}

		dev_info(adapter->dev, "xgmac%d write: 0x%08x = 0x%08x\n", xgmac_index, address, value);

		writel(value, adapter->mac[xgmac_index]->ioaddr + address);
		value = readl(adapter->mac[xgmac_index]->ioaddr + address);

		dev_info(adapter->dev, "xgmac%d read: 0x%08x = 0x%08x\n", xgmac_index, address, value);
	} else if (strncmp(cmd_buf, "xgmac0 stats", 12) == 0 || strncmp(cmd_buf, "xgmac1 stats", 12) == 0 ||
		   strncmp(cmd_buf, "xgmac2 stats", 12) == 0 || strncmp(cmd_buf, "xgmac3 stats", 12) == 0 ||
		   strncmp(cmd_buf, "xgmac4 stats", 12) == 0) {
		u8 xgmac_index = cmd_buf[5] - '0';

		cnt = sscanf(&cmd_buf[12], "%hhi", &option);
		if (cnt != 1)
			option = XGMAC_STATS_OPTION_ALL;

		dubhe1000_xgmac_stats_dump_per_index(adapter, xgmac_index, option);
	} else if (strncmp(cmd_buf, "rx poll", 7) == 0) {
		cnt = sscanf(&cmd_buf[7], "%i ", &value);
		if (cnt != 1) {
			dev_info(adapter->dev, "rx poll <val>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "rx poll: val = %d\n", value);
		dubhe1000_rx_poll(adapter, value);
	} else if (strncmp(cmd_buf, "cci read", 8) == 0) {
		cnt = sscanf(&cmd_buf[8], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "cci read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "cci read: 0x%08x len: %d\n", address, len);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->cci_regs + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->cci_regs + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->cci_regs + address + 32 * k);
		} else {
			value = readl(adapter->cci_regs + address);
			dev_info(adapter->dev, "cci read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "cci write", 9) == 0) {
		cnt = sscanf(&cmd_buf[9], "%i %i", &address, &value);
		if (cnt != 2) {
			dev_info(adapter->dev, "cci write <reg_addr> <value>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "cci write: 0x%08x = 0x%08x\n", address, value);

		writel(value, adapter->cci_regs + address);
		value = readl(adapter->cci_regs + address);

		dev_info(adapter->dev, "cci read: 0x%08x = 0x%08x\n", address, value);
	} else if (strncmp(cmd_buf, "acp read", 8) == 0) {
		cnt = sscanf(&cmd_buf[8], "%i %i", &address, &len);
		if (cnt < 1) {
			dev_info(adapter->dev, "acp read <reg_addr>\n");
			goto command_write_done;
		}

		if (cnt == 2) {
			dev_info(adapter->dev, "acp read: 0x%08x len: %d\n", address, len);
			k2 = len / 8;
			dubhe1000_dump_regs(adapter, adapter->acp_shaper_regs + address);
			for (k = 0; k < k2; k++) {
				if (k == 0)
					continue;

				dubhe1000_dump_regs(adapter, adapter->acp_shaper_regs + address + 32 * k);
			}
			dubhe1000_dump_regs(adapter, adapter->acp_shaper_regs + address + 32 * k);
		} else {
			value = readl(adapter->acp_shaper_regs + address);
			dev_info(adapter->dev, "acp read: 0x%08x = 0x%08x\n", address, value);
		}
	} else if (strncmp(cmd_buf, "acp write", 9) == 0) {
		cnt = sscanf(&cmd_buf[9], "%i %i", &address, &value);
		if (cnt != 2) {
			dev_info(adapter->dev, "acp write <reg_addr> <value>\n");
			goto command_write_done;
		}

		dev_info(adapter->dev, "acp write: 0x%08x = 0x%08x\n", address, value);

		writel(value, adapter->acp_shaper_regs + address);
		value = readl(adapter->acp_shaper_regs + address);

		dev_info(adapter->dev, "acp read: 0x%08x = 0x%08x\n", address, value);

#if defined(__ENABLE_FWD_TEST__)
	} else if (strncmp(cmd_buf, FWD_CMD_STR, sizeof(FWD_CMD_STR) - 1) == 0) {
		fwd_test_cmd(adapter, cmd_buf);
#endif
#ifdef CONFIG_DUBHE2000_PHYLINK
	} else if (strncmp(cmd_buf, "phy", 3) == 0) {
		do_phy(adapter, cmd_buf);
#endif
	} else if (strncmp(cmd_buf, "free buffer", 11) == 0) {
		cnt = sscanf(&cmd_buf[11], "%i", &value);
		dubhe1000_free_buffer_page(value);
#if defined(__ENABLE_LOOPBACK__) || defined(__ENABLE_FWD_TEST__)
	} else if (strncmp(cmd_buf, "loopback", 8) == 0) {
		cnt = sscanf(&cmd_buf[8], "%i", &value);
		if (cnt != 1)
			dev_info(adapter->dev, "loopback <0/1>\n");

		if (value < 3) {
			dev_info(adapter->dev, "loopback flag %i -> %i\n", adapter->loopback_mode, value);
			adapter->loopback_mode = value;
#if defined(__ENABLE_LOOPBACK__)
			dubhe1000_loopback_init(adapter, value);
#endif
		} else if (value <= 32) {
			dev_info(adapter->dev, "budget %i -> %i\n", g_max_txrx, value);
			g_max_txrx = value;
		} else {
		}
#endif
	} else if (strncmp(cmd_buf, "cpu_da_add", 10) == 0) {
		u8 cnt, mac_array[6];
		u16 gid;
		int ret;

		cnt = sscanf(&cmd_buf[10], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx %hi",
			     &mac_array[0], &mac_array[1], &mac_array[2], &mac_array[3], &mac_array[4], &mac_array[5],
			     &gid);
		if (cnt != 7) {
			printk(KERN_ERR "cpu_da_add params too many");
			goto command_write_done;
		}

		ret = dubhe1000_l2_cpu_da_lookup_add(mac_array, gid);

		printk(KERN_ERR "cpu_da_add: MAC %02x:%02x:%02x:%02x:%02x:%02x GID %d ret=%d",
		       mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5], gid, ret);
	} else if (strncmp(cmd_buf, "cpu_da_del", 10) == 0) {
		u8 cnt, mac_array[6];
		u16 gid;
		int ret;

		cnt = sscanf(&cmd_buf[10], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx %hi",
			     &mac_array[0], &mac_array[1], &mac_array[2], &mac_array[3], &mac_array[4], &mac_array[5],
			     &gid);
		if (cnt != 7) {
			printk(KERN_ERR "cpu_da_del params too many");
			goto command_write_done;
		}

		ret = dubhe1000_l2_cpu_da_lookup_del(mac_array, gid);

		printk(KERN_ERR "cpu_da_del: MAC %02x:%02x:%02x:%02x:%02x:%02x GID %d ret=%d",
		       mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5], gid, ret);
	} else if (strncmp(cmd_buf, "arp_add", 7) == 0) {
		__be32 ip;
		unsigned char *in = (unsigned char *)&ip;
		u8 cnt, mac_array[6], devname[64];
		struct net_device *dev = NULL;
		int ret;

		cnt = sscanf(&cmd_buf[7], "%hhd.%hhd.%hhd.%hhd %hhx:%hhx:%hhx:%hhx:%hhx:%hhx %s",
			     &in[0], &in[1], &in[2], &in[3],
			     &mac_array[0], &mac_array[1], &mac_array[2], &mac_array[3], &mac_array[4], &mac_array[5],
			     devname);
		if (cnt != 11) {
			printk(KERN_ERR "arp_add params too many");
			goto command_write_done;
		}

		dev = __dev_get_by_name(&init_net, devname);
		if (!dev) {
			pr_err("NO found the dev[%s]\n", devname);
			goto command_write_done;
		}

		ret = dubhe1000_arp_set(dev, &ip, mac_array);
		printk(KERN_ERR "arp_add:IP %d.%d.%d.%d MAC %02x:%02x:%02x:%02x:%02x:%02x dev %s ret=%d",
		       in[0], in[1], in[2], in[3],
		       mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5],
		       devname, ret);
	} else if (strncmp(cmd_buf, "arp_del", 7) == 0) {
		__be32 ip;
		unsigned char *in = (unsigned char *)&ip;
		u8 cnt, devname[64];
		struct net_device *dev = NULL;
		int ret;

		cnt = sscanf(&cmd_buf[7], "%hhd.%hhd.%hhd.%hhd %s", &in[0], &in[1], &in[2], &in[3], devname);
		if (cnt != 5) {
			printk(KERN_ERR "arp_del params too many");
			goto command_write_done;
		}

		dev = __dev_get_by_name(&init_net, devname);
		if (!dev) {
			pr_err("NO found the dev[%s]\n", devname);
			goto command_write_done;
		}

		ret = dubhe1000_arp_del(dev, &ip);
		printk(KERN_ERR "arp_del:IP %d.%d.%d.%d dev %s ret=%d", in[0], in[1], in[2], in[3], devname, ret);

	} else if (strncmp(cmd_buf, "edma_unack_rx", 13) == 0) {
		char *_argv[10] = { 0 };
		int argc = 0;
		char **argv = _argv;

		argc = __parse_args(cmd_buf, argv, 10);
		if (argc == 1) {
			printk(KERN_ERR"\nedma_unack_rx[0]=%ld\n edma_unack_rx[1]=%ld\n"
					"edma_unack_rx[2]=%ld\nedma_unack_rx[3]=%ld\n",
					atomic_long_read(&adapter->edma_unack_rx[0]),
					atomic_long_read(&adapter->edma_unack_rx[1]),
					atomic_long_read(&adapter->edma_unack_rx[2]),
					atomic_long_read(&adapter->edma_unack_rx[3]));
			goto command_write_done;
		} else if (argc >= 3) {
			int index = simple_strtol(argv[1], NULL, 0);
			int value = simple_strtol(argv[2], NULL, 0);

			atomic_long_set(&adapter->edma_unack_rx[index], value);

			printk(KERN_ERR "\nedma_unack_rx[%d] = %ld\n", index,
					atomic_long_read(&adapter->edma_unack_rx[index]));
		}
	}  else if (strncmp(cmd_buf, "npecmd_set", 10) == 0) {
		cnt = sscanf(&cmd_buf[11], "%s %i %i", module_name, &address, &value);
		if (cnt != 3) {
			dev_info(adapter->dev, "npecmd_set <module> <address> <value>\n");
			goto command_write_done;
		}

		if (strncmp(&cmd_buf[11], "edma_dbg", 8) == 0) {
		} else if (strncmp(&cmd_buf[11], "edma", 4) == 0) {
		} else if (strncmp(&cmd_buf[11], "switch", 5) == 0) {
			writel(value, adapter->switch_regs + address * 4);
		} else if (strncmp(&cmd_buf[11], "mac", 3) == 0) {
		} else {
			dev_info(adapter->dev, "npecmd module: edma edma_dbg switch mac\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "edma_rx_limit", 13) == 0) {
		char *_argv[10] = { 0 };
		int argc = 0;
		char **argv = _argv;

		argc = __parse_args(cmd_buf, argv, 10);
		if (argc == 1) {
			printk(KERN_ERR"\nedma_rx_limit[0]=%u\n edma_rx_limit[1]=%u\n"
					"edma_rx_limit[2]=%u\nedma_rx_limit[3]=%u\n",
					adapter->edma_rx_limit[0],
					adapter->edma_rx_limit[1],
					adapter->edma_rx_limit[2],
					adapter->edma_rx_limit[3]);
			goto command_write_done;
		} else if (argc >= 3) {
			int index = simple_strtol(argv[1], NULL, 0);
			int value = simple_strtol(argv[2], NULL, 0);

			adapter->edma_rx_limit[index] = value;

			printk(KERN_ERR "\nedma_rx_limit[%d] = %u\n", index,
					adapter->edma_rx_limit[index]);
		}
	} else if (strncmp(cmd_buf, "npecmd_get", 10) == 0) {
		memset(dubhe1000_dbg_command_buf, 0, sizeof(dubhe1000_dbg_command_buf));

		cnt = sscanf(&cmd_buf[11], "%s %i", module_name, &address);
		if (cnt != 2) {
			dev_info(adapter->dev, "npecmd_get <module> <address>\n");
			goto command_write_done;
		}

		if (strncmp(&cmd_buf[11], "edma_dbg", 8) == 0) {
		} else if (strncmp(&cmd_buf[11], "edma", 4) == 0) {
		} else if (strncmp(&cmd_buf[11], "switch", 5) == 0) {
			value = readl(adapter->switch_regs + 4 * address);
			snprintf(dubhe1000_dbg_command_buf, sizeof(dubhe1000_dbg_command_buf), "0x%x", value);
		} else if (strncmp(&cmd_buf[11], "mac", 3) == 0) {
		} else {
			dev_info(adapter->dev, "npecmd module: edma edma_dbg switch mac\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "aging", 5) == 0) {
		if (strncmp(&cmd_buf[6], "status_tbl", 10) == 0) {
			if (strncmp(&cmd_buf[17], "dump", 4) == 0) {
				if (sscanf(&cmd_buf[22], "%i", &value) == 1) {
					dubhe2000_aging_status_tbl_dump(adapter, value);
				} else {
					dev_info(adapter->dev, "aging status_tbl dump <index>\n");
					goto command_write_done;
				}
			} else if (sscanf(&cmd_buf[17], "%hhi %i", &option, &value) == 2) {
				dubhe2000_aging_status_tbl_config(adapter, !!option, value);
			} else {
				dev_info(adapter->dev, "aging status_tbl <index> <is_add>\n");
				dev_info(adapter->dev, "aging status_tbl dump <index>\n");
				goto command_write_done;
			}
		} else if (strncmp(&cmd_buf[6], "cfg_1ms_timer", 13) == 0) {
			if (strncmp(&cmd_buf[20], "dump", 4) == 0) {
				dubhe2000_aging_cfg_1ms_timer_dump(adapter);
			} else if (sscanf(&cmd_buf[20], "%i", &value) == 1) {
				dubhe2000_aging_cfg_1ms_timer_config(adapter, value);
			} else {
				dev_info(adapter->dev, "aging cfg_1ms_timer <value>\n");
				dev_info(adapter->dev, "aging cfg_1ms_timer dump\n");
				goto command_write_done;
			}
		} else if (strncmp(&cmd_buf[6], "cfg_aging_timer", 15) == 0) {
			if (strncmp(&cmd_buf[22], "dump", 4) == 0) {
				dubhe2000_aging_cfg_aging_timer_dump(adapter);
			} else if (sscanf(&cmd_buf[22], "%i", &value) == 1) {
				dubhe2000_aging_cfg_aging_timer_config(adapter, value);
			} else {
				dev_info(adapter->dev, "aging cfg_aging_timer <value>\n");
				dev_info(adapter->dev, "aging cfg_aging_timer dump\n");
				goto command_write_done;
			}
		} else if (strncmp(&cmd_buf[6], "cfg_ctrl", 8) == 0) {
			if (strncmp(&cmd_buf[15], "dump", 4) == 0) {
				dubhe2000_aging_cfg_ctrl_dump(adapter);
			} else if (sscanf(&cmd_buf[15], "%i", &value) == 1) {
				dubhe2000_aging_cfg_ctrl_config(adapter, value);
			} else {
				dev_info(adapter->dev, "aging cfg_ctrl <value>\n");
				dev_info(adapter->dev, "aging cfg_ctrl dump\n");
				goto command_write_done;
			}
		} else if (strncmp(&cmd_buf[6], "read", 4) == 0) {
			cnt = sscanf(&cmd_buf[11], "%i", &address);
			if (cnt != 1) {
				dev_info(adapter->dev, "switch aging <reg_addr>\n");
				goto command_write_done;
			}

			value = aging_r32(address);
			dev_info(adapter->dev, "aging read: 0x%08x = 0x%08x\n", address, value);
		} else if (strncmp(&cmd_buf[6], "write", 5) == 0) {
			cnt = sscanf(&cmd_buf[12], "%i %i", &address, &value);
			if (cnt != 2) {
				dev_info(adapter->dev, "aging write <reg_addr> <value>\n");
				goto command_write_done;
			}

			dev_info(adapter->dev, "aging write: 0x%08x = 0x%08x\n", address, value);

			aging_w32(address, value);
			value = aging_r32(address);

			dev_info(adapter->dev, "aging read: 0x%08x = 0x%08x\n", address, value);
		} else {
			dev_info(adapter->dev, "aging status_tbl <index> <is_add>\n");
			dev_info(adapter->dev, "aging status_tbl dump <index>\n");
			dev_info(adapter->dev, "aging cfg_1ms_timer <value>\n");
			dev_info(adapter->dev, "aging cfg_1ms_timer dump\n");
			dev_info(adapter->dev, "aging cfg_aging_timer <value>\n");
			dev_info(adapter->dev, "aging cfg_aging_timer dump\n");
			dev_info(adapter->dev, "aging cfg_ctrl <value>\n");
			dev_info(adapter->dev, "aging cfg_ctrl dump\n");
			dev_info(adapter->dev, "aging read <reg_addr>\n");
			dev_info(adapter->dev, "aging write <reg_addr> <value>\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "statistics", 10) == 0) {
		npe_rxmpud_data_dump();
		npe_tx_data_dump();
	} else if (strncmp(cmd_buf, "switch_delay", 12) == 0) {
		pr_info("[%s] old switch_cfg_delay = %d ms\n", __func__, adapter->switch_cfg_delay);
		if (sscanf(&cmd_buf[13], "%i", &value) == 1)
			adapter->switch_cfg_delay = value;

		pr_info("[%s] new nat switch_cfg_delay = %d ms\n", __func__, adapter->switch_cfg_delay);
	} else if (strncmp(cmd_buf, "tc_debug", 8) == 0) {
		if (sscanf(&cmd_buf[9], "%i", &value) == 1)
			adapter->tc_debug = !!value;
		else {
			dubhe2000_switch_l3_hash_counter_dump();
			dubhe2000_switch_nexthop_info_dump();
			dubhe2000_nat_dump_ingress_op_status();
		}
	} else if (strncmp(cmd_buf, "acl_svp", 7) == 0) {
		if (strncmp(&cmd_buf[8], "enable", 6) == 0) {
			dubhe2000_switch_generic_svp_config(adapter, 1);
		} else if (strncmp(&cmd_buf[8], "disable", 7) == 0) {
			dubhe2000_switch_generic_svp_config(adapter, 0);
		} else {
			dev_info(adapter->dev, "acl_svp [enable | disable]\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "acl_ftp", 7) == 0) {
		if (strncmp(&cmd_buf[8], "enable", 6) == 0) {
			dubhe2000_switch_generic_ftp_config(adapter, 1);
		} else if (strncmp(&cmd_buf[8], "disable", 7) == 0) {
			dubhe2000_switch_generic_ftp_config(adapter, 0);
		} else {
			dev_info(adapter->dev, "acl_ftp [enable | disable]\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "acl_tcp", 7) == 0) {
		if (strncmp(&cmd_buf[8], "enable", 6) == 0) {
			dubhe2000_switch_generic_tcp_config(adapter, 1);
		} else if (strncmp(&cmd_buf[8], "disable", 7) == 0) {
			dubhe2000_switch_generic_tcp_config(adapter, 0);
		} else {
			dev_info(adapter->dev, "acl_tcp [enable | disable]\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "acl_nonip", 9) == 0) {
		if (strncmp(&cmd_buf[10], "enable", 6) == 0) {
			/*only set lan port mac for no ip for generic platform
			 *dsa mode use "acl_nonip lanwan" to enable wan mac to send acl_nonip enable lanwan
			 */
			dubhe2000_switch_generic_nonip_config(adapter, BIT(LAN_MAC_INX));
		} else if (strncmp(&cmd_buf[10], "disable", 7) == 0) {
			dubhe2000_switch_generic_nonip_config(adapter, 0);
		} else if (strncmp(&cmd_buf[10], "lanwan", 6) == 0) {
			dubhe2000_switch_generic_nonip_config(adapter, BIT(LAN_MAC_INX) | BIT(WAN_MAC_INX));
		} else {
			dev_info(adapter->dev, "acl_nonip [enable | disable | lanwan]\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "acl_ipfrag", 10) == 0) {
		if (strncmp(&cmd_buf[11], "enable", 6) == 0) {
			dubhe2000_switch_generic_ip_frag_config(adapter, 1);
		} else if (strncmp(&cmd_buf[11], "disable", 7) == 0) {
			dubhe2000_switch_generic_ip_frag_config(adapter, 0);
		} else {
			dev_info(adapter->dev, "acl_ipfrag [enable | disable]\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "acl_iperf", 9) == 0) {
		int iperf_mode = -1;

		if (sscanf(&cmd_buf[10], "%d", &iperf_mode) == 1) {
			dubhe2000_switch_generic_iperf_config(adapter, iperf_mode);
		} else {
			dev_info(adapter->dev, "acl_iperf [mode]\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "switch_dfx", 10) == 0) {
		dubhe2000_switch_dump_dfx(adapter, &cmd_buf[11]);
	} else if (strncmp(cmd_buf, "learning_dmac", 13) == 0) {
		if (strncmp(&cmd_buf[14], "enable", 6) == 0) {
			cnt = sscanf(&cmd_buf[21], "%hhx: %hhx: %hhx: %hhx: %hhx: %hhx",
				     &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5]);
			if (cnt != 6) {
				dev_info(adapter->dev, "learning_dmac enable <macaddr>\n");
				goto command_write_done;
			} else {
				dubhe2000_l2_learning_dmac_config(adapter, 1, data2);
			}
		} else if (strncmp(&cmd_buf[14], "disable", 7) == 0) {
			dubhe2000_l2_learning_dmac_config(adapter, 0, NULL);
		} else if (strncmp(&cmd_buf[14], "show", 4) == 0) {
			u64 dmac;

			if (dubhe2000_l2_learning_dmac_get(adapter, &dmac))
				pr_info("Learning Da MAC: 0x%llx\n", dmac);
			else
				pr_info("Learning Da MAC: disable\n");
		} else {
			dev_info(adapter->dev, "learning_dmac enable <macaddr>\n");
			dev_info(adapter->dev, "learning_dmac disable\n");
			dev_info(adapter->dev, "learning_dmac show\n");
			goto command_write_done;
		}
	} else if (strncmp(cmd_buf, "port_mac", 8) == 0) {
		cnt = sscanf(&cmd_buf[9], "%hi %hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &vrf,
			     &data2[0], &data2[1], &data2[2], &data2[3], &data2[4], &data2[5]);

		if (cnt == 7)
			cls_npe_router_port_macaddr_config(data2, !!vrf);
		else
			dev_info(adapter->dev, "port_mac <is_wan> <Mac>\n");
	} else if (strncmp(cmd_buf, "tunnel_del", 10) == 0) {
		/* TODO */
	} else if (strncmp(cmd_buf, "tunnel_entry", 12) == 0) {
		if (strncmp(&cmd_buf[13], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[17], "%i", &value);
			if (cnt == 1)
				dubhe2000_tunnel_entry_delete_session(adapter, value);
			else
				dev_info(adapter->dev, "tunnel_entry del <index>\n");
		} else {
			cnt = sscanf(&cmd_buf[13], "%hi %hhi", &session_id, &option);
			if (cnt == 2)
				dubhe2000_tunnel_entry_config_session(adapter, session_id, !!option);
			else
				dev_info(adapter->dev, "tunnel_entry <session_id> <is_ipv4>\n");
		}
	} else if (strncmp(cmd_buf, "tunnel_exit", 11) == 0) {
		if (strncmp(&cmd_buf[12], "del", 3) == 0) {
			cnt = sscanf(&cmd_buf[16], "%i", &value);
			if (cnt == 1)
				dubhe2000_tunnel_exit_delete_session(adapter, value);
			else
				dev_info(adapter->dev, "tunnel_exit del <index>\n");
		} else {
			cnt = sscanf(&cmd_buf[12], "%hi %hhi", &session_id, &option);
			if (cnt == 2)
				dubhe2000_tunnel_exit_config_session(adapter, session_id, !!option);
			else
				dev_info(adapter->dev, "tunnel_exit <session_id> <is_ipv4>\n");
		}
	} else if (strncmp(cmd_buf, "token", 5) == 0) {
		char *_argv[10] = { 0 };
		int argc = 0;
		char *name = NULL;
		char **argv = _argv;

		argc = __parse_args(cmd_buf, argv, 10);
		if (argc < 2) {
			printk(KERN_ERR "Error parameters Number\n");
			goto command_write_done;
		}
		name = argv[1];
		argc -= 2;
		argv += 2;

		if (strcasecmp("enable", name) == 0) {
			s32 val = 0;
			s32 token_enable = 0;

			if (argc < 1) {
				printk(KERN_ERR "Error parameters Number\n");
				goto command_write_done;
			}

			token_enable = simple_strtol(argv[0], NULL, 0);

			val = er32(TX_BASE_REG0);
			val |= (!!token_enable) << TOKEN_FLOW_CTRL_EN_BIT;
			ew32(TX_BASE_REG0, val);

			dev_info(adapter->dev, "token %s\n", token_enable ? "enable" : "disable");
		} else if (strcasecmp("init", name) == 0) {
			s32 val = 0;
			int num = 0;
			u32 token_id = 0;

			if (argc < 2) {
				printk(KERN_ERR "Error parameters Number\n");
				goto command_write_done;
			}

			num = simple_strtol(argv[0], NULL, 0);

			if (num > 0xfff) {
				printk(KERN_ERR "token num[%d] must < 0xfff\n", num);
				goto command_write_done;
			}

			token_id = simple_strtol(argv[1], NULL, 0);

			if (token_id >= 512) {
				for (token_id = 0; token_id < 512; token_id++) {
					val = (num << TOKEN_INIT_NUM_BIT);
					val |= (token_id << TOKEN_INIT_TID_BIT);
					val |= (1 << TOKEN_INIT_EN_BIT);
					ew32(TX_TOKEN_INIT, val);
				}

				dev_info(adapter->dev, "init all token_id num=%d!",  num);
			} else {
				val = (num << TOKEN_INIT_NUM_BIT);
				val |= (token_id << TOKEN_INIT_TID_BIT);
				val |= (1 << TOKEN_INIT_EN_BIT);
				ew32(TX_TOKEN_INIT, val);

				dev_info(adapter->dev, "init token[%d] num=%d!", token_id, num);
			}
		} else if (strcasecmp("put", name) == 0) {
			int num = 0;
			s32 val = 0;
			u32 token_id;

			if (argc < 2) {
				printk(KERN_ERR "Error parameters Number\n");
				goto command_write_done;
			}

			token_id = simple_strtol(argv[0], NULL, 0);
			if (token_id >= 512) {
				printk(KERN_ERR "token token id must < %d\n", token_id);
				goto command_write_done;
			}

			num = simple_strtol(argv[1], NULL, 0);
			if (num > 0xfff) {
				printk(KERN_ERR "token num[%d] must < 0xfff\n", num);
				goto command_write_done;
			}

			val = (num << TOKEN_INIT_NUM_BIT);
			val |= (token_id << TOKEN_INIT_TID_BIT);
			val |= (1 << TOKEN_INIT_EN_BIT);
			ew32(TX_TOKEN_SUPPLEMENT, val);

			val = er32(TX_TOKEN_STATUS);

			dev_info(adapter->dev, "token[%d] add num=%d, ret=%d!", token_id, num, val);
		} else if (strcasecmp("get", name) == 0) {
			int num = 0;
			u32 token_id;

			if (argc < 1) {
				printk(KERN_ERR "Error parameters Number\n");
				goto command_write_done;
			}

			token_id = simple_strtol(argv[0], NULL, 0);
			if (token_id >= 512) {
				printk(KERN_ERR "token token id must < %d\n", token_id);
				goto command_write_done;
			}

			num = DUBHE1000_READ_REG_ARRAY(TX_TOKEN_COUNTER, 0x4 * token_id);

			dev_info(adapter->dev, "Get token[%d] num=%d", token_id, num);
		} else if (strcasecmp("free", name) == 0) {
			int num = 0;
			s32 val = 0;
			u32 token_id;

			if (argc < 2) {
				printk(KERN_ERR "Error parameters Number\n");
				goto command_write_done;
			}

			token_id = simple_strtol(argv[0], NULL, 0);
			if (token_id >= 512) {
				printk(KERN_ERR "token token id must < %d\n", token_id);
				goto command_write_done;
			}

			num = simple_strtol(argv[1], NULL, 0);
			if (num > 0xfff) {
				printk(KERN_ERR "token num[%d] must < 0xfff\n", num);
				goto command_write_done;
			}

			val = (num << TOKEN_INIT_NUM_BIT);
			val |= (token_id << TOKEN_INIT_TID_BIT);
			val |= (1 << TOKEN_INIT_EN_BIT);
			ew32(TX_TOKEN_FREE, val);

			val = er32(TX_TOKEN_STATUS);

			dev_info(adapter->dev, "token[%d] del num[%d] ret[%d]", token_id, num, val);
		}
	} else {
		dev_info(adapter->dev, "unknown command '%s'\n", cmd_buf);
		dev_info(adapter->dev, "available commands\n");

		dev_info(adapter->dev, "  edma read <reg_addr>\n");
		dev_info(adapter->dev, "  edma write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  edma_dbg read <reg_addr>\n");
		dev_info(adapter->dev, "  edma_dbg write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  edma_dbg stats [option]\n");
		dev_info(adapter->dev, "  switch read <reg_addr>\n");
		dev_info(adapter->dev, "  switch write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  switch stats [option]\n");
		dev_info(adapter->dev, "  switch clear_stats\n");
		dev_info(adapter->dev, "  xgmac0 read <reg_addr>\n");
		dev_info(adapter->dev, "  xgmac0 write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  xgmac0 stats [option]\n");
		dev_info(adapter->dev, "  xgmac1 read <reg_addr>\n");
		dev_info(adapter->dev, "  xgmac1 write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  xgmac1 stats [option]\n");
		dev_info(adapter->dev, "  xgmac2 read <reg_addr>\n");
		dev_info(adapter->dev, "  xgmac2 write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  xgmac2 stats [option]\n");
		dev_info(adapter->dev, "  xgmac3 read <reg_addr>\n");
		dev_info(adapter->dev, "  xgmac3 write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  xgmac3 stats [option]\n");
		dev_info(adapter->dev, "  xgmac4 read <reg_addr>\n");
		dev_info(adapter->dev, "  xgmac4 write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  xgmac4 stats [option]\n");
		dev_info(adapter->dev, "  aging read <reg_addr>\n");
		dev_info(adapter->dev, "  aging write <reg_addr> <value>\n");
		dev_info(adapter->dev, "  fp <dev_src> <dev_dst>\n");
#ifdef CONFIG_DUBHE2000_PHYLINK
		dev_info(adapter->dev, "  phy<id> <read/write> <reg> <data>\n");
#endif
#if defined(__ENABLE_FWD_TEST__)
		FWD_USAGE_HELP();
#endif
	}

command_write_done:
	kfree(cmd_buf);
	cmd_buf = NULL;
	return count;
}

static const struct file_operations dubhe1000_dbg_command_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_command_read,
	.write = dubhe1000_dbg_command_write,
};

/**************************************************************
 * netdev_ops
 * The netdev_ops entry in debugfs is for giving the driver commands
 * to be executed from the netdev operations.
 **************************************************************/
static char dubhe1000_dbg_netdev_ops_buf[256] = "";

/**
 * dubhe1000_dbg_netdev_ops_read - read for netdev_ops datum
 * @filp: the opened file
 * @buffer: where to write the data for the user to read
 * @count: the size of the user's buffer
 * @ppos: file position offset
 **/
static ssize_t dubhe1000_dbg_netdev_ops_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	struct net_device *netdev = NULL;
	int bytes_not_copied;
	int buf_size = 256;
	char *buf;
	int len;

	/* don't allow partal reads */
	if (*ppos != 0)
		return 0;
	if (count < buf_size)
		return -ENOSPC;

	buf = kzalloc(buf_size, GFP_KERNEL);
	if (!buf)
		return -ENOSPC;

	len = snprintf(buf, buf_size, "%s: %s\n", netdev->name, dubhe1000_dbg_netdev_ops_buf);

	bytes_not_copied = copy_to_user(buffer, buf, len);
	kfree(buf);

	if (bytes_not_copied)
		return -EFAULT;

	*ppos = len;
	return len;
}

/**
 * dubhe1000_dbg_netdev_ops_write - write into netdev_ops datum
 * @filp: the opened file
 * @buffer: where to find the user's data
 * @count: the length of the user's data
 * @ppos: file position offset
 **/
static ssize_t dubhe1000_dbg_netdev_ops_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	int bytes_not_copied;
	struct dubhe1000_mac *port = NULL;
	char *buf_tmp;
	int portid;
	int cnt;

	/* don't allow partial writes */
	if (*ppos != 0)
		return 0;
	if (count >= sizeof(dubhe1000_dbg_netdev_ops_buf))
		return -ENOSPC;

	memset(dubhe1000_dbg_netdev_ops_buf, 0, sizeof(dubhe1000_dbg_netdev_ops_buf));
	bytes_not_copied = copy_from_user(dubhe1000_dbg_netdev_ops_buf, buffer, count);
	if (bytes_not_copied)
		return -EFAULT;
	dubhe1000_dbg_netdev_ops_buf[count] = '\0';

	buf_tmp = strchr(dubhe1000_dbg_netdev_ops_buf, '\n');
	if (buf_tmp) {
		*buf_tmp = '\0';
		count = buf_tmp - dubhe1000_dbg_netdev_ops_buf + 1;
	}

	if (strncmp(dubhe1000_dbg_netdev_ops_buf, "netpoll", 7) == 0) {
		cnt = sscanf(&dubhe1000_dbg_netdev_ops_buf[8], "%i", &portid);
		if (cnt != 1) {
			dev_info(adapter->dev, "netpoll <portid>\n");
			goto netdev_ops_write_done;
		}
		port = dubhe1000_find_port(adapter, portid);
		if (!port) {
			dev_info(adapter->dev, "netpoll: port %d not found\n", portid);
		} else if (!port->netdev) {
			dev_info(adapter->dev, "netpoll: no netdev for port %d\n", portid);
		} else {
			dev_info(adapter->dev, "netpoll port %d\n", portid);
#ifdef CONFIG_NET_POLL_CONTROLLER
			netdev->netdev_ops->ndo_poll_controller(port->netdev);
			dev_info(adapter->dev, "netpoll called\n");
#endif
		}

	} else if (strncmp(dubhe1000_dbg_netdev_ops_buf, "napi", 4) == 0) {
		int channel;

		cnt = sscanf(&dubhe1000_dbg_netdev_ops_buf[4], "%i %i", &channel, &portid);
		if (cnt != 2) {
			dev_info(adapter->dev, "napi <channel> <portid>\n");
			goto netdev_ops_write_done;
		}
		port = dubhe1000_find_port(adapter, portid);
		if (!port) {
			dev_info(adapter->dev, "napi: port %d not found\n", portid);
		} else if (!port->netdev) {
			dev_info(adapter->dev, "napi: no netdev for port %d\n", portid);
		} else {
			dev_info(adapter->dev, "napi port %d\n", portid);
			napi_schedule(&adapter->edma_rx_napi[channel]);
			dev_info(adapter->dev, "napi called\n");
		}
	} else {
		dev_info(adapter->dev, "unknown command '%s'\n", dubhe1000_dbg_netdev_ops_buf);
		dev_info(adapter->dev, "available commands\n");
		dev_info(adapter->dev, "  netpoll <portid>\n");
		dev_info(adapter->dev, "  napi <portid>\n");
	}
netdev_ops_write_done:
	return count;
}

static const struct file_operations dubhe1000_dbg_netdev_ops_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_netdev_ops_read,
	.write = dubhe1000_dbg_netdev_ops_write,
};

static int dubhe1000_set_wan_port(struct dubhe1000_adapter *adapter, const char *ifname)
{
	int i;
	struct net_device *dev;
	struct dubhe1000_mac *port;

	if (!strcmp(ifname, "none")) {
		if (adapter->wan_port != DUBHE2000_NON_EXIST_PORT) {
			adapter->wan_port = DUBHE2000_NON_EXIST_PORT;
			dubhe2000_change_wan_port(adapter);
		}
		return 0;
	}

	dev = __dev_get_by_name(&init_net, ifname);
	if (!dev) {
		pr_warn("%s: %s don't exist\n", __func__, ifname);
		return -EINVAL;
	}

	port = netdev_priv(dev);
	for (i = 0; i < DUBHE1000_MAC_COUNT; i++) {
		if (port == adapter->mac[i]) {
			if (adapter->wan_port != port->id) {
				adapter->wan_port = port->id;
				dubhe2000_change_wan_port(adapter);
			}
			return 0;
		}
	}

	pr_warn("%s: %s is not a NPE Ethernet device\n", __func__, ifname);
	return -EINVAL;
}

static ssize_t dubhe1000_dbg_wan_port_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char buf[IFNAMSIZ] = { 0 };
	int len;

	if (adapter->wan_port == DUBHE2000_NON_EXIST_PORT)
		len = sprintf(buf, "%s\n", "none");
	else if (adapter->wan_port >= DUBHE1000_MAC_COUNT || !adapter->mac[adapter->wan_port])
		len = sprintf(buf, "%s\n", "invalid");
	else
		len = sprintf(buf, "%s\n", adapter->mac[adapter->wan_port]->netdev->name);

	return simple_read_from_buffer(userbuf, count, ppos, buf, len);
}

static ssize_t dubhe1000_dbg_wan_port_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char buf[IFNAMSIZ] = { 0 };
	char *newline;
	int ret;

	ret = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, userbuf, count);
	if (ret < 0)
		return ret;

	newline = strchr(buf, '\n');
	if (newline)
		*newline = '\0';

	ret = dubhe1000_set_wan_port(adapter, buf);
	if (ret < 0)
		return ret;

	return count;
}

static const struct file_operations dubhe1000_dbg_wan_port_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_wan_port_read,
	.write = dubhe1000_dbg_wan_port_write,
};

static ssize_t dubhe1000_dbg_dscp_map_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char *buf;
	int i, len;
	ssize_t ret;

	buf = kzalloc(64 * 10 + 16, GFP_KERNEL);
	if (!buf)
		return count;

	len = sprintf(buf, "DS  TID Queue\n");
	for (i = 0; i < 64; ++i)
		len += sprintf(buf + len, "%-3d %-3u %u\n", i, adapter->dscp_map[i].tid, adapter->dscp_map[i].queue);

	ret = simple_read_from_buffer(userbuf, count, ppos, buf, len);
	kfree(buf);

	return ret;
}

static ssize_t dubhe1000_dbg_dscp_map_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char *buf, *tmp, *line;
	int dscp, queue, tid, ret, i;

	buf = kzalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return count;

	ret = simple_write_to_buffer(buf, count, ppos, userbuf, count);
	if (ret < 0) {
		kfree(buf);
		return count;
	}

	if (!strncmp(buf, "reset", 5)) {
		dubhe1000_init_dscp_map(adapter);
		kfree(buf);
		return count;
	} else if (!strncmp(buf, "apply", 5)) {
		for (i = 0; i < 64; i++)
			DUBHE1000_WRITE_REG_ARRAY(DSCP_TID, 0x4 * i, adapter->dscp_map[i].tid);
		kfree(buf);
		return count;
	}

	tmp = buf;
	while ((line = strsep(&tmp, "\n")) && *line) {
		dscp = tid = queue = INT_MIN;
		if (sscanf(line, "%d %d %d", &dscp, &tid, &queue) != 3)
			continue;

		if (dscp != -1 && (dscp < 0 || dscp > 63))
			continue;

		if (tid < 0 || tid > 7)
			continue;

		if (queue < 0 || queue > 7)
			continue;

		if (dscp == -1) {
			/* tid and queue of the dscp that is not configured */
			for (i = 0; i < 64; ++i) {
				if (adapter->dscp_map[i].configured)
					continue;
				adapter->dscp_map[i].tid = tid;
				adapter->dscp_map[i].queue = queue;
			}
		} else {
			adapter->dscp_map[dscp].tid = tid;
			adapter->dscp_map[dscp].queue = queue;
			adapter->dscp_map[dscp].configured = 1;
		}
	}

	kfree(buf);
	return count;
}

static const struct file_operations dubhe1000_dbg_dscp_map_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_dscp_map_read,
	.write = dubhe1000_dbg_dscp_map_write,
};

static ssize_t dubhe1000_dbg_tc_map_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char *buf;
	int i, len;
	ssize_t ret;

	buf = kzalloc(256 * 10 + 16, GFP_KERNEL);
	if (!buf)
		return count;

	len = sprintf(buf, "TC  TID Queue\n");
	for (i = 0; i < 256; ++i)
		len += sprintf(buf + len, "%-3d %-3u %u\n", i, adapter->tc_map[i].tid, adapter->tc_map[i].queue);

	ret = simple_read_from_buffer(userbuf, count, ppos, buf, len);
	kfree(buf);

	return ret;
}

static ssize_t dubhe1000_dbg_tc_map_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char *buf, *tmp, *line;
	int tc, queue, tid, ret, i;

	buf = kzalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return count;

	ret = simple_write_to_buffer(buf, count, ppos, userbuf, count);
	if (ret < 0) {
		kfree(buf);
		return count;
	}

	if (!strncmp(buf, "reset", 5)) {
		dubhe1000_init_tc_map(adapter);
		kfree(buf);
		return count;
	} else if (!strncmp(buf, "apply", 5)) {
		for (i = 0; i < 256; i++)
			DUBHE1000_WRITE_REG_ARRAY(TC_TID, i * 0x4, adapter->tc_map[i].tid);
		kfree(buf);
		return count;
	}

	tmp = buf;
	while ((line = strsep(&tmp, "\n")) && *line) {
		tc = tid = queue = INT_MIN;
		if (sscanf(line, "%d %d %d", &tc, &tid, &queue) != 3)
			continue;

		if (tc != -1 && (tc < 0 || tc > 255))
			continue;

		if (tid < 0 || tid > 7)
			continue;

		if (queue < 0 || queue > 7)
			continue;

		if (tc == -1) {
			/* tid and queue of the tc that is not configured */
			for (i = 0; i < 256; ++i) {
				if (adapter->tc_map[i].configured)
					continue;
				adapter->tc_map[i].tid = tid;
				adapter->tc_map[i].queue = queue;
			}
		} else {
			adapter->tc_map[tc].tid = tid;
			adapter->tc_map[tc].queue = queue;
			adapter->tc_map[tc].configured = 1;
		}
	}

	kfree(buf);
	return count;
}

static const struct file_operations dubhe1000_dbg_tc_map_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_tc_map_read,
	.write = dubhe1000_dbg_tc_map_write,
};

static ssize_t dubhe1000_dbg_queue_weight_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char *buf;
	int port, queue, len;
	ssize_t ret;

	buf = kzalloc(48 * 15 + 20, GFP_KERNEL);
	if (!buf)
		return count;

	len = sprintf(buf, "Port Queue Weight\n");
	for (port = 0; port < 6; ++port) {
		for (queue = 0; queue < 8; ++queue)
			len += sprintf(buf + len, "%-4d %-5d %u\n", port, queue, adapter->queue_weight[port][queue]);
	}

	ret = simple_read_from_buffer(userbuf, count, ppos, buf, len);
	kfree(buf);

	return ret;
}

static ssize_t dubhe1000_dbg_queue_weight_write(struct file *filp, const char __user *userbuf, size_t count,
						loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char *buf, *tmp, *line;
	int port_start, port_end, port, queue, weight, ret;

	buf = kzalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return count;

	ret = simple_write_to_buffer(buf, count, ppos, userbuf, count);
	if (ret < 0) {
		kfree(buf);
		return count;
	}

	if (!strncmp(buf, "reset", 5)) {
		dubhe2000_switch_init_queue_weight(adapter);
		kfree(buf);
		return count;
	} else if (!strncmp(buf, "apply", 5)) {
		dubhe2000_switch_set_queue_weight(adapter);
		kfree(buf);
		return count;
	}

	tmp = buf;
	while ((line = strsep(&tmp, "\n")) && *line) {
		port_start = port_end = queue = weight = INT_MIN;
		if (sscanf(line, "%d~%d %d %d", &port_start, &port_end, &queue, &weight) == 4) {
			if (port_start < 0 || port_start > 5 || port_end < 0 || port_end > 5 || port_start > port_end)
				continue;
		} else if (sscanf(line, "%d %d %d", &port_start, &queue, &weight) == 3) {
			if (port_start < 0 || port_start > 5)
				continue;
			port_end = port_start;
		} else
			continue;

		if (queue < 0 || queue > 7)
			continue;

		if (weight < 1 || weight > 255)
			continue;

		for (port = port_start; port <= port_end; ++port)
			adapter->queue_weight[port][queue] = weight;
	}

	kfree(buf);
	return count;
}

static const struct file_operations dubhe1000_dbg_queue_weight_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_queue_weight_read,
	.write = dubhe1000_dbg_queue_weight_write,
};

static ssize_t dubhe1000_dbg_pcp_to_queue_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char buf[64];
	int pcp, len;
	ssize_t ret;
	t_VLANPCPToQueueMappingTable data;

	len = sprintf(buf, "PCP Queue\n");
	for (pcp = 0; pcp < 8; ++pcp) {
		rd_VLANPCPToQueueMappingTable(adapter, pcp, &data);
		len += sprintf(buf + len, "%u %u\n", pcp, data.pQueue);
	}

	ret = simple_read_from_buffer(userbuf, count, ppos, buf, len);

	return ret;
}

static ssize_t dubhe1000_dbg_pcp_to_queue_write(struct file *filp, const char __user *userbuf, size_t count,
						loff_t *ppos)
{
	struct dubhe1000_adapter *adapter = filp->private_data;
	char buf[64], *tmp, *line;
	int pcp, queue, ret;
	t_VLANPCPToQueueMappingTable data;

	ret = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, userbuf, count);
	if (ret < 0)
		return ret;

	tmp = buf;
	while ((line = strsep(&tmp, "\n")) && *line) {
		pcp = queue = INT_MIN;
		if (sscanf(line, "%d %d", &pcp, &queue) != 2)
			continue;
		if (pcp < 0 || pcp > 7)
			continue;
		if (queue < 0 || queue > 7)
			continue;

		data.pQueue = queue;
		wr_VLANPCPToQueueMappingTable(adapter, pcp, &data);
	}

	return count;
}

static const struct file_operations dubhe1000_dbg_pcp_to_queue_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_pcp_to_queue_read,
	.write = dubhe1000_dbg_pcp_to_queue_write,
};

static ssize_t dubhe1000_dbg_dft_vid_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_mac *port = filp->private_data;
	t_SourcePortTable source_port;
	char buf[8];
	int len;

	rd_SourcePortTable(port->adapter, port->id, &source_port);
	len = scnprintf(buf, sizeof(buf), "%u\n", source_port.defaultVid);

	return simple_read_from_buffer(userbuf, count, ppos, buf, len);
}

static ssize_t dubhe1000_dbg_dft_vid_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	struct dubhe1000_mac *port = filp->private_data;
	t_SourcePortTable source_port;
	u16 vid;

	if (kstrtou16_from_user(userbuf, count, 0, &vid))
		return -EINVAL;

	if (vid > 4095)
		return -EINVAL;

	rd_SourcePortTable(port->adapter, port->id, &source_port);
	source_port.vidSel = 1;
	source_port.defaultVid = vid;
	wr_SourcePortTable(port->adapter, port->id, &source_port);

	return count;
}

static const struct file_operations dubhe1000_dbg_dft_vid_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = dubhe1000_dbg_dft_vid_read,
	.write = dubhe1000_dbg_dft_vid_write,
};

void dubhe1000_port_dbg_init(struct dubhe1000_mac *port)
{
	struct dentry *dir_port;

	if (!port->netdev || !dubhe1000_dbg_root)
		return;

	dir_port = debugfs_create_dir(port->netdev->name, dubhe1000_dbg_root);
	if (!dir_port) {
		pr_info("%s: failed to create debugfs for %s\n", __func__, port->netdev->name);
		return;
	}

	debugfs_create_file("default_vid", 0600, dir_port, port, &dubhe1000_dbg_dft_vid_fops);
}

/**
 * dubhe1000_dbg_init - start up debugfs for the driver
 **/
void dubhe1000_dbg_init(struct dubhe1000_adapter *adapter)
{
	dubhe1000_dbg_root = debugfs_create_dir(cls_npe_driver_name, NULL);
	if (!dubhe1000_dbg_root) {
		pr_info("DUBHE1000 init of debugfs failed\n");
		return;
	}

	debugfs_create_file("command", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_command_fops);

	debugfs_create_file("netdev_ops", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_netdev_ops_fops);

	debugfs_create_file("wan_port", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_wan_port_fops);

	debugfs_create_file("dscp_map", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_dscp_map_fops);

	debugfs_create_file("tc_map", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_tc_map_fops);

	debugfs_create_file("queue_weight", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_queue_weight_fops);

	debugfs_create_file("pcp_to_queue", 0600, dubhe1000_dbg_root, adapter, &dubhe1000_dbg_pcp_to_queue_fops);
}

/**
 * dubhe1000_dbg_exit - stop debugfs for the driver
 **/
void dubhe1000_dbg_exit(struct dubhe1000_adapter *adapter)
{
	debugfs_remove_recursive(dubhe1000_dbg_root);
	dubhe1000_dbg_root = NULL;
}
