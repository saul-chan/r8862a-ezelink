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

/* dubhe1000_switch.c
 * Shared functions for accessing and configuring the Switch
 */

#include "dubhe2000_switch_regs.h"
#include "dubhe2000_switch.h"
#include <net/neighbour.h>
#include <net/arp.h>
#define SWITCH_ADDR(offset) (switch_base_addr + ((offset) * 4))

struct dubhe2000_switch_learning_frame {
	u32 mac_part0;

	u32 mac_part1:16;
	u32 gid:16;

	u32 portOrPtr;

	u32 uc:8;
	u32 drop:8;
	u32 daDestAddr_part0:16;

	u32 daDestAddr_part1:16;
	u32 valid:8;
	u32 stat:8;

	u32 hit:8;
	u32 l2ActDa:8;
	u32 l2ActSa:8;
	u32 meta_part0:8;

	u32 meta_part1:8;
	u32 reserved:24;
};

struct dubhe2000_switch_meta_data {
	u32 sta_index:10;
	u32 dest_wpu:2;
	u32 magic_data:4;
	u32 reserved:16;
};

enum {
	METADATA_DESTWPU_MIN = 0,
	METADATA_DESTWPU_HOSTCPU = METADATA_DESTWPU_MIN,
	METADATA_DESTWPU_WIFI_2_4G,
	METADATA_DESTWPU_WIFI_5G,
	METADATA_DESTWPU_PCIE,
	METADATA_DESTWPU_MAX,

};

// change macaddr type from small-endian u64 to big-endian u8 *
// mac(00:11:2:33:44:55): 0x001122334455 -> {0x00, 0x11, 0x22, x033, 0x44, 0x55}
void dubhe2000_mac_u64_to_u8(u64 mac_u64, u8 *mac_u8)
{
	int i;

	if (!mac_u8) {
		pr_err("[%s] null mac_u8\n", __func__);
		return;
	}

	for (i = 0; i < ETH_ALEN; ++i)
		mac_u8[i] = (mac_u64 >> ((ETH_ALEN - i - 1) * 8)) & 0xFF;
}

// change macaddr type big-endian u8 * to small-endian u64
// mac(00:11:2:33:44:55): {0x00, 0x11, 0x22, x033, 0x44, 0x55} -> 0x001122334455
void dubhe2000_mac_u8_to_u64(u64 *mac_u64, u8 *mac_u8)
{
	int i;
	u64 tmp = 0;

	if (!mac_u8) {
		pr_err("[%s] null mac_u8\n", __func__);
		return;
	}

	for (i = 0; i < ETH_ALEN; i++)
		tmp = mac_u8[i] + (tmp << 8);

	*mac_u64 = tmp;
}

bool dubhe2000_l2_learning_dmac_get(struct dubhe1000_adapter *adapter, u64 *dmac)
{
	t_LearningDAMAC data;

	rd_LearningDAMAC(adapter, &data);

	if (data.enable)
		*dmac = data.mac;

	return !!data.enable;
}

//mac is big-endian
void dubhe2000_l2_learning_dmac_config(struct dubhe1000_adapter *adapter, bool is_en, u8 *mac)
{
	t_LearningDAMAC data;
	u64 tmp = 0;

	data.enable = !!is_en;

	if (data.enable)
		dubhe2000_mac_u8_to_u64(&tmp, mac);

	data.mac = tmp;

	wr_LearningDAMAC(adapter, &data);
}

int dubhe2000_translate_fwt_port(u8 fwt_port)
{
	if (fwt_port >= CLS_FWT_PORT_ETH_0 && fwt_port <= CLS_FWT_PORT_ETH_4)
		return (fwt_port - CLS_FWT_PORT_ETH_0);
	else if (fwt_port == CLS_FWT_PORT_WLAN0 || fwt_port == CLS_FWT_PORT_WLAN1)
		return DUBHE2000_CPU_PORT;
	/* For netdev like GRE tap interface not registered by CLS NPE/wifi driver */
	else if (fwt_port == CLS_FWT_PORT_UNSPEC)
		return DUBHE2000_CPU_PORT;
	else
		return -1;
}

static int dubhe2000_l2_learning_frame_trigger(struct dubhe1000_adapter *adapter,
					       struct dubhe2000_switch_learning_frame data)
{
	struct sk_buff *skb;
	struct net_device *xmit_dev = NULL;
	struct ethhdr *eth;
	u64 learning_dmac;
	u8 dmac[ETH_ALEN];
	int i, ret;

	if (!dubhe2000_l2_learning_dmac_get(adapter, &learning_dmac)) {
		pr_err("[%s] Learning Da MAC Disable.\n", __func__);
		// write switch l2 register directly

		return -1;
	}

	dubhe2000_mac_u64_to_u8(learning_dmac, dmac);

	// find valid net_device
	for (i = 0; i < DUBHE1000_MAC_COUNT; i++) {
		if (g_adapter->mac[i]->netdev) {
			xmit_dev = g_adapter->mac[i]->netdev;
			break;
		}
	}

	if (!xmit_dev) {
		pr_err("[%s] Failed to find active device.\n", __func__);
		return -ENODEV;
	}

	skb = dev_alloc_skb(ETH_FRAME_LEN);
	if (!skb) {
		pr_err("[%s] Failed to allocate learning skb.\n", __func__);
		return -ENOMEM;
	}

	//no heardroom tailroom
	skb_put(skb, ETH_FRAME_LEN);

	skb->protocol = htons(ETH_P_IP);
	skb->dev = xmit_dev;
	skb->pkt_type = PACKET_OTHERHOST;

	skb->bmu_flag |= CLS_BMU_FLAG_LEARNING;

	// ethernet header
	eth = (struct ethhdr *)skb->data;
#define DUBHE2000_LEARNING_FRAME_ETYPE 0xFFFF
	eth->h_proto = htons(DUBHE2000_LEARNING_FRAME_ETYPE);
	memcpy(eth->h_dest, &dmac, ETH_ALEN);
	memcpy(eth->h_source, xmit_dev->dev_addr, ETH_ALEN);

	// learning header
	memcpy(skb->data + ETH_HLEN, &data, sizeof(struct dubhe2000_switch_learning_frame));

	// xmit
	ret = xmit_dev->netdev_ops->ndo_start_xmit(skb, xmit_dev);
	if (ret) {
		//try it aagin
		ret = xmit_dev->netdev_ops->ndo_start_xmit(skb, xmit_dev);
		if (ret) {
			pr_err("[%s] Failed to send learning frame (%d)\n", __func__, ret);
			kfree_skb(skb);

			return -1;
		}
	}

	g_adapter->total_l2_learning++;

	return 0;
}

void dubhe2000_l2_lookup_config_directly(struct dubhe2000_switch_learning_frame data)
{
	t_L2DAHashLookupTable l2_da;
	t_L2DestinationTable l2_dest;
	t_L2AgingTable l2_aging;
	t_L2AgingStatusShadowTable l2_shadow;
	u64 tmp;
	u32 index;

	tmp = data.daDestAddr_part1;
	tmp = data.daDestAddr_part0 + (tmp << 16);
	index = tmp;

	//config L2 Destination Table
	memset(&l2_dest, 0, sizeof(l2_dest));
	l2_dest.uc = data.uc;
	l2_dest.destPortormcAddr = data.portOrPtr;
	l2_dest.pktDrop = data.drop;
	l2_dest.l2ActionTableDaStatus = data.l2ActDa;
	l2_dest.l2ActionTableSaStatus = data.l2ActSa;
	tmp = data.meta_part1;
	tmp = data.meta_part0 + (tmp << 8);
	l2_dest.metaData = tmp;

	memset(&l2_da, 0, sizeof(l2_da));
	tmp = data.mac_part1;
	tmp = data.mac_part0 + (tmp << 32);
	l2_da.macAddr = tmp;
	l2_da.gid = data.gid;

	//config L2 Aging Table
	memset(&l2_aging, 0, sizeof(l2_aging));
	l2_aging.valid = data.valid;
	l2_aging.stat = data.stat;
	l2_aging.hit = data.hit;

	//config L2 Aging Status Shadow Table (enable it end)
	memset(&l2_shadow, 0, sizeof(l2_shadow));
	l2_shadow.valid = data.valid;

	wr_L2DestinationTable(g_adapter, index, &l2_dest);

	if (index < L2DAHashLookupTable_nr_entries) {
		wr_L2DAHashLookupTable(g_adapter, index, (void *)(&l2_da));
		wr_L2AgingTable(g_adapter, index, (void *)(&l2_aging));
		wr_L2AgingStatusShadowTable(g_adapter, index, (void *)(&l2_shadow));
	} else {
		index = index - L2DAHashLookupTable_nr_entries;
		wr_L2LookupCollisionTable(g_adapter, index, (void *)(&l2_da));
		wr_L2AgingCollisionTable(g_adapter, index, (void *)(&l2_aging));
		wr_L2AgingCollisionShadowTable(g_adapter, index, (void *)(&l2_shadow));
	}

	g_adapter->total_l2_directly++;
}

void cls_npe_l2_update_entry(u16 fwt_index, u8 *mac, u16 vid, u8 dport, u16 sta_idx)
{
	struct dubhe2000_switch_learning_frame data;
	struct dubhe2000_switch_meta_data meta_data;
	u64 dmac = 0;
	u32 metadata;
	int switch_port;
	bool is_static;

	if (!mac) {
		pr_err("[%s] null mac\n", __func__);
		return;
	}

	switch_port = dubhe2000_translate_fwt_port(dport);
	if (switch_port < 0) {
		pr_err("[%s] invalid dport\n", __func__);

		return;
	}

	if (clsemi_vid_should_sw_fwd(vid))
		switch_port = DUBHE2000_CPU_PORT;

	// in dubhe2000 phase2, non-eth(wireless) entry will be static type(kernel control).
	is_static =
		!((cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_LEARNING_PKT) && (switch_port != DUBHE2000_CPU_PORT));

	memset(&data, 0, sizeof(struct dubhe2000_switch_learning_frame));
	memset(&meta_data, 0, sizeof(struct dubhe2000_switch_meta_data));

	// change mac type from u8 * to u64
	dubhe2000_mac_u8_to_u64(&dmac, mac);
	data.mac_part0 = dmac & 0xFFFFFFFF;
	data.mac_part1 = (dmac >> 32) & 0xFFFF;

	data.gid = vid;
	data.portOrPtr = switch_port;
	data.uc = 1;
	data.drop = 0;
	data.daDestAddr_part0 = fwt_index & 0xFFFF;
	data.daDestAddr_part1 = fwt_index >> 16;
	data.valid = 1;

	if (is_static)
		data.stat = 1;
	else
		data.stat = 0;

	data.hit = 1;
	data.l2ActDa = 0;
	data.l2ActSa = 0;

	/* MetaData:
	 * In dubhe2000 phase1, only edma's host-tx-fifo will be enabled.
	 * But sta_index can always work!
	 */

	if (dport == CLS_FWT_PORT_WLAN1) {
		if (g_adapter->txq_status_num[METADATA_DESTWPU_WIFI_5G])
			meta_data.dest_wpu = METADATA_DESTWPU_WIFI_5G;
		else
			meta_data.dest_wpu = METADATA_DESTWPU_HOSTCPU;
	} else if (dport == CLS_FWT_PORT_WLAN0) {
		if (g_adapter->txq_status_num[METADATA_DESTWPU_WIFI_2_4G])
			meta_data.dest_wpu = METADATA_DESTWPU_WIFI_2_4G;
		else
			meta_data.dest_wpu = METADATA_DESTWPU_HOSTCPU;
	} else
		meta_data.dest_wpu = METADATA_DESTWPU_HOSTCPU;

	meta_data.sta_index = sta_idx; //Note: use it when pkt should be sent by wifi if
	meta_data.magic_data = 0x0; //unused
	meta_data.reserved = 0;
	memcpy(&metadata, (void *)&meta_data, sizeof(struct dubhe2000_switch_meta_data));

	data.meta_part0 = metadata & 0xFF;
	data.meta_part1 = (metadata >> 8) & 0xFF;

	data.reserved = 0;

	// when learning mode and trigger success, config_directly can be ignored.
	if (!(cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_LEARNING_PKT &&
	      !dubhe2000_l2_learning_frame_trigger(g_adapter, data)))
		dubhe2000_l2_lookup_config_directly(data); // write switch l2 register directly
}
EXPORT_SYMBOL(cls_npe_l2_update_entry);

void cls_npe_l2_del_entry(u16 fwt_index)
{
	struct dubhe2000_switch_learning_frame data;

	memset(&data, 0, sizeof(data));
	data.daDestAddr_part0 = fwt_index & 0xFFFF;
	data.daDestAddr_part1 = fwt_index >> 16;

	// when learning mode and trigger success, config_directly can be ignored.
	if (!(cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_LEARNING_PKT &&
	      !dubhe2000_l2_learning_frame_trigger(g_adapter, data)))
		dubhe2000_l2_lookup_config_directly(data); // write switch l2 register directly
}
EXPORT_SYMBOL(cls_npe_l2_del_entry);

// check mac/vid???
int cls_npe_l2_entry_lookup_is_valid(u16 fwt_index, u8 *mac, u16 vid)
{
	t_L2DAHashLookupTable l2_da;
	t_L2LookupCollisionTable l2_coll;
	u64 dmac;

	dubhe2000_mac_u8_to_u64(&dmac, mac);

	if (fwt_index < L2DAHashLookupTable_nr_entries) {
		rd_L2DAHashLookupTable(g_adapter, fwt_index, &l2_da);
		if ((dmac != l2_da.macAddr) || (vid != l2_da.gid))
			return 0;
	} else if ((fwt_index - L2DAHashLookupTable_nr_entries) < L2LookupCollisionTable_nr_entries) {
		rd_L2LookupCollisionTable(g_adapter, fwt_index - L2DAHashLookupTable_nr_entries, &l2_coll);
		if ((dmac != l2_coll.macAddr) || (vid != l2_coll.gid))
			return 0;
	} else {
		pr_err("[%s] L2 Destination Table index\n", __func__);
	}

	return 1;
}
EXPORT_SYMBOL(cls_npe_l2_entry_lookup_is_valid);

static u64 switch_l2_hashkey(u64 macaddr, u16 gid)
{
	u64 key;

	key = gid;
	key = (key << 48) + (macaddr & 0xFFFFFFFFFFFF);

	return key;
}

int dubhe1000_l2_cpu_da_lookup_del(u8 *mac_array, u16 gid)
{
	u8 bucket;
	u32 value[L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY];
	u32 old_value[L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY] = { 0 };
	u16 hashval = 0, index = 0;
	u64 macaddr, hashkey, address;

	if (!is_valid_ether_addr(mac_array)) {
		printk(KERN_ERR "%s Err MAC  %02x:%02x:%02x:%02x:%02x:%02x", __func__,
		       mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5]);
		return -EINVAL;
	}

	macaddr = ether_addr_to_u64(mac_array);
	hashval = l2_hash(gid, macaddr);
	hashkey = switch_l2_hashkey(macaddr, gid);
	value[0] = hashkey & 0xFFFFFFFF;
	value[1] = hashkey >> 32;

	for (bucket = 0; bucket < 4; bucket++) {
		index = hashval + ((bucket & 0x3) << 10);
		address = L2_DA_HASH_LOOKUP_TBL + index * L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY;
		old_value[0] = readl(SWITCH_ADDR(address));
		old_value[1] = readl(SWITCH_ADDR(address + 1));
		if (memcmp(old_value, value, L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY * 4) == 0) {
			writel(0, SWITCH_ADDR(address));
			writel(0, SWITCH_ADDR(address + 1));
			break;
		}
	}

	if (bucket == 4)
		return 0;

	index = hashval + ((bucket & 0x3) << 10);

	//-uc=1 --destPort_or_mcAddr=5
	writel(0x0, SWITCH_ADDR(L2_DESTINATION_TBL + (index * L2_DESTINATION_TBL_ADDR_PER_ENTRY)));

	//--valid=1 --stat=1 --hit=1
	writel(0x0, SWITCH_ADDR(L2_AGING_TBL + index));

	//--valid=1
	writel(0x0, SWITCH_ADDR(L2_AGING_STATUS_SHADOW_TBL + index));

	return 0;
}

int dubhe1000_l2_cpu_da_lookup_add(u8 *mac_array, u16 gid)
{
	u8 bucket, empty_bucket = 4;
	u32 value[L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY];
	u32 old_value[L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY] = { 0 };
	u32 null_value[L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY] = { 0 };
	u16 hashval = 0, index = 0, need_set = 1;
	u64 address, macaddr, hashkey;

	if (!is_valid_ether_addr(mac_array)) {
		printk(KERN_ERR "%s Err MAC  %02x:%02x:%02x:%02x:%02x:%02x", __func__,
		       mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5]);
		return -EINVAL;
	}

	macaddr = ether_addr_to_u64(mac_array);
	hashval = l2_hash(gid, macaddr);
	hashkey = switch_l2_hashkey(macaddr, gid);
	value[0] = hashkey & 0xFFFFFFFF;
	value[1] = hashkey >> 32;

	for (bucket = 0; bucket < 4; bucket++) {
		index = hashval + ((bucket & 0x3) << 10);
		address = L2_DA_HASH_LOOKUP_TBL + index * L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY;
		old_value[0] = readl(SWITCH_ADDR(address));
		old_value[1] = readl(SWITCH_ADDR(address + 1));
		if (empty_bucket == 4 && (memcmp(old_value, null_value, L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY * 4) == 0)) {
			empty_bucket = bucket;
		} else if (memcmp(old_value, value, L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY * 4) == 0) {
			need_set = 0;
			empty_bucket = bucket;
			break;
		}
	}

	if (need_set) {
		if (empty_bucket < 4) {
			index = hashval + ((empty_bucket & 0x3) << 10);
			address = L2_DA_HASH_LOOKUP_TBL + index * L2_HASH_LOOKUP_TBL_ADDR_PER_ENTRY;
			writel(value[0], SWITCH_ADDR(address));
			writel(value[1], SWITCH_ADDR(address + 1));
		} else {
			printk(KERN_ERR "%s buckets are full, MAC %02x:%02x:%02x:%02x:%02x:%02x GID %d", __func__,
			       mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5], gid);
			return -ENOMEM;
		}
	}

	index = hashval + ((empty_bucket & 0x3) << 10);

	//-uc=1 --destPort_or_mcAddr=5
	writel(0xb, SWITCH_ADDR(L2_DESTINATION_TBL + (index * L2_DESTINATION_TBL_ADDR_PER_ENTRY)));

	//--valid=1 --stat=1 --hit=1
	writel(0x7, SWITCH_ADDR(L2_AGING_TBL + index));

	//--valid=1
	writel(0x1, SWITCH_ADDR(L2_AGING_STATUS_SHADOW_TBL + index));

	return 0;
}

int dubhe1000_arp_set(struct net_device *dev, u32 *ip, u8 *mac_addr)
{
	int err;
	struct neighbour *neigh;

	if (!ip || !mac_addr || !dev)
		return -EINVAL;

	rtnl_lock();

	neigh = __neigh_lookup_errno(&arp_tbl, ip, dev);
	err = PTR_ERR(neigh);
	if (!IS_ERR(neigh)) {
		err = neigh_update(neigh, mac_addr, NUD_PERMANENT, NEIGH_UPDATE_F_OVERRIDE | NEIGH_UPDATE_F_ADMIN, 0);
		neigh_release(neigh);
	}

	rtnl_unlock();

	return err;
}

int dubhe1000_arp_del(struct net_device *dev, u32 *ip)
{
	/* TODO */
	return 0;
}

void dubhe2000_switch_l2_hairpin_enable_init(struct dubhe1000_adapter *adapter)
{
	t_HairpinEnable hairpin_enable;
	int i;

	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		memset(&hairpin_enable, 0, sizeof(hairpin_enable));
		rd_HairpinEnable(adapter, i, &hairpin_enable);

		if (i == DUBHE2000_CPU_PORT)
			hairpin_enable.allowUc = 1;
		else
			hairpin_enable.allowUc = 0;

		wr_HairpinEnable(adapter, i, &hairpin_enable);
	}
}

/* In L2 switch, the packe of SA MISS should be sent to cpu */
void dubhe2000_switch_l2_action_table_source_port_init(struct dubhe1000_adapter *adapter)
{
	t_SourcePortTable source_port;
	t_L2ActionTableSourcePort l2_action_tbl_sport;
	int i;

	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		if (i == DUBHE2000_CPU_PORT)
			continue;

		memset(&source_port, 0, sizeof(source_port));
		rd_SourcePortTable(adapter, i, &source_port);

		// Do Lookup in the L2 ActionTable and L2 Action Table SourcePort
		source_port.enableL2ActionTable = 1;

		wr_SourcePortTable(adapter, i, &source_port);
	}

	for (i = 0; i < L2ActionTableSourcePort_nr_entries; i++) {
		if (i & 0x2) // sa hit
			continue;

		memset(&l2_action_tbl_sport, 0, sizeof(l2_action_tbl_sport));
		rd_L2ActionTableSourcePort(adapter, i, &l2_action_tbl_sport);

		l2_action_tbl_sport.sendToCpu = 1;
		l2_action_tbl_sport.forceSendToCpuOrigPkt = 1;

		wr_L2ActionTableSourcePort(adapter, i, &l2_action_tbl_sport);
	}
}

void dubhe2000_switch_vlan_table_init(struct dubhe1000_adapter *adapter)
{
	t_VLANTable vlan_table;
	int i;

	for (i = 0; i < VLANTable_nr_entries; i++) {
		memset(&vlan_table, 0, sizeof(vlan_table));
		rd_VLANTable(adapter, i, &vlan_table);

		vlan_table.vlanPortMask = 0x3f;
		vlan_table.gid = i;

		wr_VLANTable(adapter, i, &vlan_table);
	}
}

void dubhe2000_switch_l2_init(struct dubhe1000_adapter *adapter)
{
	t_SourcePortTable source_port;
	int i;

	if (cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_DIRECT) {
		for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
			memset(&source_port, 0, sizeof(source_port));
			rd_SourcePortTable(adapter, i, &source_port);

			// Disable DA MAC Auto Learning on CPU Port
			source_port.learningEn = 0;

			wr_SourcePortTable(adapter, i, &source_port);
		}
	}

	dubhe2000_switch_l2_hairpin_enable_init(adapter);
	dubhe2000_switch_l2_action_table_source_port_init(adapter);
	dubhe2000_switch_vlan_table_init(adapter);
}
