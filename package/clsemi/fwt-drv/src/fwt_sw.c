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

#include "fwt.h"

MODULE_DESCRIPTION("Forwarding Table");
MODULE_AUTHOR("Clourneysemi Corporation, <linux.nics@clourneysemi.com>");
MODULE_DESCRIPTION("Clourneysemi FWT Network Driver");
MODULE_LICENSE("GPL v2");

char cls_fwt_driver_name[] = "fwt";

/* FWT entries counter */
uint16_t g_fwt_ent_cnt;

spinlock_t cls_fwt_lock;

/* FWT cache entry function enable / disable */
#define CLS_FWT_CACHE_ENTRY_ENABLE	1

/* FWT cache entry */
int32_t g_fwt_cache[CLS_FWT_CACHE_ENTRY_SIZE];

/* FWT cache entry function active index */
//this index will be replaced by next entry which has been hit enough times
int g_fwt_cache_entry_index;

static struct timer_list fwt_timer;

static unsigned int fwt_timer_delay = 10000; //10s
module_param(fwt_timer_delay, uint, 0644);
MODULE_PARM_DESC(fwt_timer_delay, "timer: poll the switch_l2 eth entry and sync it");

unsigned int switch_mod = CLS_FWT_SWITCH_MODE_DIRECT;
module_param(switch_mod, uint, 0644);
MODULE_PARM_DESC(switch_mod, "switch l2 mode");

unsigned int mop_interval = 50;
module_param(mop_interval, uint, 0644);
MODULE_PARM_DESC(mop_interval, "fwt Mac oscillation protection interval");

struct net_bridge *fwt_get_br_by_dev(struct net_device *netdev)
{
	struct net_bridge_port *p;

	if (netdev) {
		p = br_port_get_rcu(netdev);
		if (p || p->dev == netdev)
			return p->br;
	}

	return NULL;
}

static void fwt_trigger_gc_work_by_dev(struct net_device *netdev)
{
	struct net_bridge *br;

	if (netdev) {
		br = fwt_get_br_by_dev(netdev);

		if (br)
			mod_delayed_work(system_long_wq, &br->gc_work, msecs_to_jiffies(10));
	}
}

static bool fwt_get_br_hold_time_by_dev(struct net_device *netdev, unsigned long *hold_time)
{
	struct net_bridge *br;

	if (netdev) {
		br = fwt_get_br_by_dev(netdev);

		if (br) {
			*hold_time = br->topology_change ? br->forward_delay : br->ageing_time;

			return 1;
		}
	}

	return 0;
}

void fwt_make_entry_aged_out(int index)
{
	struct net_device *netdev;
	fwt_db_entry *fwt_ent;
	unsigned long hold_time;

	fwt_ent = fwt_db_get_valid_entry(index);
	if (!fwt_ent) {
		pr_info("[%s] index %d invalid\n", __func__, index);
		return;
	}

	netdev = fwt_ent->out_dev;
	pr_info("[%s] outdev=%s flag=0x%x\n", __func__, netdev->name, netdev->priv_flags);

	fwt_ent->timestamp = 0;

	if (fwt_get_br_hold_time_by_dev(netdev, &hold_time)) {
		if (jiffies > (hold_time + msecs_to_jiffies(10))) {
			fwt_ent->timestamp = jiffies - hold_time - msecs_to_jiffies(10);
			pr_info("[%s] index %d timestamp %lld hold_time=%ld\n", __func__,
							index, fwt_ent->timestamp, hold_time);
		}
	}

	fwt_trigger_gc_work_by_dev(netdev);

	if (cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_LEARNING_PKT
					&& fwt_ent->node_type == CLS_FWT_ENTRY_TYPE_ETH)
		fwt_ent->aged_out = 1;
}

// check if invalid update
static int fwt_sw_check_should_update_local(uint16_t index, struct net_device *new_dev,
			uint32_t new_sub_port)
{
	fwt_db_entry *entry = fwt_db_get_table_entry(index);
	uint16_t new_node, old_node;
	uint8_t new_vif_idx, old_vif_idx;
	uint8_t old_use_4addr, new_use_4addr;
	uint8_t old_radio, new_radio;
	uint8_t old_type, new_type;
	uint8_t is_wireless = 0, old_is_wireless;
	struct net_device *old_dev;
	int ret = CLS_FWT_UPDATE_ACCEPT;
	static DEFINE_RATELIMIT_STATE(rl, 1 * HZ, 1);

	is_wireless = fwt_sw_parse_sub_port(new_sub_port,
						&new_node, &new_radio, &new_vif_idx,
						&new_type, &new_use_4addr);

	old_is_wireless = entry->wireless;
	old_dev = entry->out_dev;
	old_type = entry->node_type;
	old_node = entry->out_node;
	old_radio = entry->out_radio;
	old_vif_idx = entry->out_vif_idx;
	old_use_4addr = entry->portal;//unusable

	// nothing changed
	if ((old_is_wireless == is_wireless) && (old_dev == new_dev) &&
			(old_type == new_type) && (old_node == new_node) &&
			(old_radio == new_radio) && (old_vif_idx == new_vif_idx)) {
		ret = CLS_FWT_UPDATE_BYPASS;// no update. skip it
	
		/* kernel will bypass fdb update, just update timestamp in fwt */
		fwt_sw_reset_timestamp_by_index_local(index);

		if (cls_fwt_dbg_enable)
			pr_info("[%s]: [%pM.%u] subport 0x%x, cpuid %d: ignore no update\n", __func__,
				entry->mac_id, entry->vlan_id, new_sub_port, smp_processor_id());
	} else {
		if (FWT_IS_STATIC_ENTRY(old_type))
			ret = CLS_FWT_UPDATE_AFTER_DEL_TABLE;

		if (FWT_IS_STATIC_ENTRY(new_type)) {
			if (FWT_IS_STATIC_ENTRY(old_type))
				pr_info("[%s] static re-add witout del [%pM.%u] %s -> %s: cpuid %d\n",
					__func__, entry->mac_id, entry->vlan_id, entry->out_dev->name, new_dev->name,
					smp_processor_id());
			else {
				pr_info("[%s] dynamic to static! [%pM.%u] %s -> %s, cpuid %d\n",
					__func__, entry->mac_id, entry->vlan_id,
					entry->out_dev->name, new_dev->name, smp_processor_id());
			}
		} else if (FWT_IS_STATIC_ENTRY(old_type)) {
			pr_info("[%s] static to dynamic, loop or disconnect! [%pM.%u] %s -> %s, cpuid %d\n",
				__func__, entry->mac_id, entry->vlan_id,
				entry->out_dev->name, new_dev->name, smp_processor_id());
		} else {// wds portmove
			if (mop_interval && (jiffies_to_msecs(jiffies - entry->timestamp) < mop_interval)) {
				if (__ratelimit(&rl))
					pr_info("[%s] not update (MOP)! [%pM.%u] %s -> %s, interval %ldms, cpuid %d\n",
						__func__, entry->mac_id, entry->vlan_id, entry->out_dev->name,
						new_dev->name, jiffies_to_secs(jiffies - entry->timestamp),
						smp_processor_id());

				ret = CLS_FWT_UPDATE_BYPASS;
			} else
				pr_info("[%s] accept non-static update [%pM.%u] %s -> %s, cpuid %d\n",
					__func__, entry->mac_id, entry->vlan_id,
					entry->out_dev->name, new_dev->name, smp_processor_id());
		}
	}

	return ret;
}

/*
 * Note: The MAC address and VLAN ID fields in an existing entry must never be updated,
 * because they are used to derive the hash and index.
 */
static int fwt_sw_update_params(uint16_t index, uint32_t new_sub_port, struct net_device *new_dev)
{
	fwt_db_entry *entry = fwt_db_get_table_entry(index);
	uint16_t new_node, old_node;
	uint8_t new_vif_idx, old_vif_idx;
	uint8_t old_use_4addr, new_use_4addr;
	uint8_t old_radio, new_radio;
	uint8_t old_type, new_type;
	uint8_t is_wireless = 0, old_is_wireless;
	struct net_device *old_dev;
	static DEFINE_RATELIMIT_STATE(rl, 1 * HZ, 10);
	void (*switch_add_l2_entry)(u16 fwt_index, u8 *mac, u16 vid, u8 dport, u16 sta_idx);

	if (cls_fwt_dbg_enable)
		pr_info("[%s]: [%pM.%u] > %s, cpuid %d: enter\n", __func__,
			entry->mac_id, entry->vlan_id, new_dev->name, smp_processor_id());

	is_wireless = fwt_sw_parse_sub_port(new_sub_port,
						&new_node, &new_radio, &new_vif_idx,
						&new_type, &new_use_4addr);

	old_is_wireless = entry->wireless;
	old_dev = entry->out_dev;
	old_type = entry->node_type;
	old_node = entry->out_node;
	old_radio = entry->out_radio;
	old_vif_idx = entry->out_vif_idx;
	old_use_4addr = entry->portal;//unusable

	if ((old_is_wireless == is_wireless) && (old_dev == new_dev) &&
			(old_type == new_type) && (old_node == new_node) &&
			(old_radio == new_radio) && (old_vif_idx == new_vif_idx)) {
		if (cls_fwt_dbg_enable)
			pr_info("[%s]: [%pM.%u] > %s, cpuid %d: ignore\n", __func__,
				entry->mac_id, entry->vlan_id, new_dev->name, smp_processor_id());
		return 0;// no update. skip it
	}

	if (FWT_IS_STATIC_ENTRY(old_type)) // static entry should not be happend here, fdb will delete static entry first 
		pr_err("[%s]: [%pM.%u] cpuid %d: old static should not be happened\n", __func__,
			entry->mac_id, entry->vlan_id, smp_processor_id());
	else if (FWT_IS_WDS_ENTRY(old_type))
		fwt_db_delete_index_from_node_table(old_radio, old_node, index, old_dev);

	fwt_db_update_params(index, new_dev, new_sub_port);

	if (is_wireless)
		fwt_db_add_new_node(new_radio, new_node, index, new_dev);

	if (cls_fwt_switch_enable) {
		switch_add_l2_entry = symbol_get(cls_npe_l2_update_entry);
		if (switch_add_l2_entry) {
			switch_add_l2_entry(entry->fwt_index, entry->mac_id, entry->vlan_id,
					    entry->out_dev->if_port, entry->out_node);
			symbol_put(cls_npe_l2_update_entry);
		}
	}

	if (__ratelimit(&rl))
		pr_info("FWT: [%pM.%u] idx %u updated, dev:%s->%s node:%d->%d radio:%d->%d vif_idx:%d->%d type:%d->%d 4addr:%u->%u, cpuid %d\n",
				entry->mac_id, entry->vlan_id, index,
				old_dev->name, new_dev->name,
				old_node, new_node, old_radio, new_radio,
				old_vif_idx, new_vif_idx, old_type, new_type,
				old_use_4addr, new_use_4addr, smp_processor_id());

	return 1;
}

int fwt_sw_delete_device(const uint8_t *mac_be, const uint16_t vlan_id)
{
	int fwt_index = -EINVAL;
	fwt_db_entry *element;
	struct net_device *dev;
	uint8_t case_type = 0;
	uint8_t radio_idx;
	uint16_t node_idx;
	unsigned long flags;
	int cnt = 0;
	static DEFINE_RATELIMIT_STATE(rl, 1 * HZ, 10);

	if (mac_be == NULL)
		return -EINVAL;

	local_irq_save(flags);

	/* get the index from the FWT HW algorithm */
	fwt_index = fwt_sw_get_index_from_mac_vid(mac_be, vlan_id);
	element = fwt_db_get_valid_entry(fwt_index);
	if (!element) {
		local_irq_restore(flags);
		return -EINVAL;
	}

	dev = element->out_dev;
	node_idx = element->out_node;
	radio_idx = element->out_radio;

	/* there are 3 cases
	 * case0: eth entry: delete entry
	 * case1: device connected station: delete node ;delete entry
	 * case2: station: delete the entries in node list and node list
	 */

	if (!element->wireless) {
		//case0: no exist node entry
		fwt_db_delete_table_entry(fwt_index);
		cnt = 1;
		case_type = 0;
	} else if (!FWT_IS_STATIC_ENTRY(element->node_type)) {
		//case1: delete node; delete entry
		fwt_db_delete_index_from_node_table(radio_idx, node_idx, fwt_index, dev);

		fwt_db_delete_table_entry(fwt_index);
		case_type = 1;
		cnt = 1;
	} else {//case2: sta disassociation
		cnt = fwt_db_clear_node(element);
		case_type = 2;
	}

	g_fwt_ent_cnt = g_fwt_ent_cnt - cnt;
	if (__ratelimit(&rl))
		pr_info("FWT: [%pM.%u] del case%d entry, dev:%s node:%u idx:%d cnt:%d, cpuid %d\n",
				mac_be, vlan_id, case_type,
				dev->name, node_idx, fwt_index, cnt, smp_processor_id());

	local_irq_restore(flags);

	return fwt_index;
}

/* timestamp reset when
 * 1. add new entry
 * 2. when fwt work, SMAC's entry(not DMAC's)
 */
void fwt_sw_reset_timestamp_by_index_local(uint16_t fwt_index)
{
	fwt_db_entry *db_ent;

	db_ent = fwt_db_get_valid_entry(fwt_index);

	if (db_ent) {
		db_ent->timestamp = jiffies;
		db_ent->aged_out = 0;
	}
}

void fwt_sw_reset_timestamp_by_index(uint16_t fwt_index)
{
	unsigned long flags;

	local_irq_save(flags);

	fwt_sw_reset_timestamp_by_index_local(fwt_index);

	local_irq_restore(flags);
}

EXPORT_SYMBOL(fwt_sw_reset_timestamp_by_index);

int fwt_sw_parse_sub_port(uint32_t sub_port, uint16_t *node,
				uint8_t *radio, uint8_t *vif, uint8_t *node_type, uint8_t *is_4addr)
{
	uint16_t node_idx = 0;
	uint8_t radio_idx = 0;
	uint8_t vif_idx = 0;
	uint8_t is_wireless = 0;

	if (CLS_IEEE80211_NODE_IDX_VALID(sub_port) && CLS_IEEE80211_VIF_IDX_VALID(sub_port)) {
		node_idx = CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(sub_port);
		radio_idx = CLS_IEEE80211_RADIO_IDX_FROM_SUBPORT(sub_port);
		vif_idx = CLS_IEEE80211_VIF_IDX_FROM_SUBPORT(sub_port);

		if (node_idx < CLS_NCIDX_MAX_PER_RADIO) {
			*node = node_idx;
			*radio = radio_idx;
			*vif = vif_idx;
			*node_type = CLS_IEEE80211_NODE_TYPE_FROM_SUBPORT(sub_port);
			*is_4addr = CLS_IEEE80211_IS_4ADDR_SUPPORT(sub_port);
			is_wireless = 1;
		} else
			pr_err("[%s] invalid sta_idx\n", __func__);
	} else {
		*node = 0;
		*radio = 0;
		*vif = 0;
		*node_type = CLS_FWT_ENTRY_TYPE_ETH;
		*is_4addr = 0;
		is_wireless = 0;
	}

	return is_wireless;
}

/* @ sub_port: a station will be Unique Pointing by radio_idx and node_idx
 * |-BIT31:28-|-BIT27:24-|---BIT23---|--BIT22:16--|----BIT15----|--BIT14:12--|--BIT11:0--|
 * | is_4addr | node type| vif valid |   vif idx  | node valid  |  radio idx |  node idx |
 * Note:
 *  sub_port: a station will be Unique Pointing by radio_idx and node_idx
 */

uint32_t fwt_sw_get_entry_sub_port(fwt_db_entry *fwt_ent)
{
	uint32_t sub_port;
	uint16_t node_idx, vif_idx;

	if (!fwt_ent || !(fwt_ent->wireless))
		return 0;

	node_idx = CLS_IEEE80211_NODE_TO_IDXS(fwt_ent->out_radio, fwt_ent->out_node);
	node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);
	vif_idx = CLS_IEEE80211_VIF_IDX_MAP(fwt_ent->out_vif_idx);

	sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(node_idx, vif_idx, fwt_ent->node_type,
			fwt_ent->portal);

	return sub_port;
}

int fwt_sw_add_device(const uint8_t *mac_be, const uint16_t vlan_id, struct net_device *dev,
		uint32_t sub_port, const struct br_ip *group)
{
	fwt_db_entry new_device;
	int fwt_index;
	uint8_t use_4addr = 0;
	uint16_t node_idx = 0;
	uint8_t radio_idx = 0, node_type = 0;
	uint8_t vif_idx = 0;
	unsigned long flags;
	int ret = 0;
	uint8_t is_wireless = 0;
	static DEFINE_RATELIMIT_STATE(rl, 1 * HZ, 10);

	if (ETHER_IS_MULTICAST(mac_be) && (group == NULL))
		return -EINVAL;

	local_irq_save(flags);

	fwt_index = fwt_sw_get_index_from_mac_vid(mac_be, vlan_id);

	/* prevent duplication */
	if (fwt_db_get_valid_entry(fwt_index)) {
		/*
		 * Check for parameters change in the current entry, if so
		 * update an existing entry
		 */

		fwt_sw_reset_timestamp_by_index_local(fwt_index);
		fwt_sw_update_params(fwt_index, sub_port, dev);

		local_irq_restore(flags);
		return -EEXIST;
	}

	/* clear new device */
	fwt_db_init_entry(&new_device);

	/* Set MAC ID */
	memcpy(&new_device.mac_id, mac_be, ETH_ALEN);

	new_device.out_dev = dev;
	new_device.vlan_id = vlan_id;
	new_device.vlan_enabled = (vlan_id > 0);

	is_wireless = fwt_sw_parse_sub_port(sub_port,
					    &node_idx, &radio_idx, &vif_idx, &node_type, &use_4addr);
	new_device.wireless = !!is_wireless;

	if (new_device.wireless) {
		new_device.out_node = node_idx;
		new_device.out_radio = radio_idx;
		new_device.out_vif_idx = vif_idx;
		new_device.node_type = node_type;
		new_device.portal = !!use_4addr;
	} else {
		new_device.out_node = 0;
		new_device.out_radio = 0;
		new_device.out_vif_idx = 0;
		new_device.node_type = CLS_FWT_ENTRY_TYPE_ETH;
		new_device.portal = 0;
	}

	new_device.fwt_index = fwt_sw_new_index_from_mac_vid(mac_be, vlan_id);
	/* if no error and entry was accepted, insert new entry to fwt database */
	if (FWT_IF_SUCCESS(new_device.fwt_index)) {
		new_device.valid = true;
		fwt_db_table_insert(new_device.fwt_index, &new_device);
		fwt_sw_reset_timestamp_by_index_local(new_device.fwt_index);
		g_fwt_ent_cnt++;
		if (__ratelimit(&rl))
			pr_info("FWT: [%pM.%u] add, dev:%s port:%d node:%u/%u/%u index:%u is_wireless:%u type:%d 4addr:%u entries:%d\n",
				new_device.mac_id, new_device.vlan_id,
				dev->name, dev->if_port,
				new_device.out_node, new_device.out_radio, new_device.out_vif_idx,
				new_device.fwt_index, new_device.wireless, new_device.node_type,
				new_device.portal, g_fwt_ent_cnt);
		/* set node list */
		if (new_device.wireless)
			fwt_db_add_new_node(new_device.out_radio, new_device.out_node, new_device.fwt_index, dev);
	} else {
		ret = -ENOSPC;
	}

	local_irq_restore(flags);

	return ret;
}

unsigned long fwt_sw_get_timestamp(const uint8_t *mac_be, const uint16_t vlan_id)
{
	int fwt_index;
	fwt_db_entry *db_ent;
	bool is_static;
	unsigned long tmp;
	unsigned long flags;

	local_irq_save(flags);

	fwt_index = fwt_sw_get_index_from_mac_vid(mac_be, vlan_id);
	db_ent = fwt_db_get_valid_entry(fwt_index);

	if (db_ent) {
		is_static = FWT_IS_STATIC_ENTRY(db_ent->node_type) ||
			(cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_LEARNING_PKT
				&& db_ent->node_type == CLS_FWT_ENTRY_TYPE_ETH
				&& !db_ent->aged_out);

		tmp = is_static ? jiffies : db_ent->timestamp;

		local_irq_restore(flags);

		return tmp;
	} else {
		//Invalid entry: return a very old timestap and kernel fdb will make it aged out
		pr_info("[%s] invalid entry[%pM.%d]! Should make it aged out\n",
								__func__, mac_be, vlan_id);

		local_irq_restore(flags);

		return 0;
	}
}

/*
 * Move an entry to another interface.
 */
static int fwt_sw_check_should_update(const uint8_t *mac_be, const uint16_t vlan_id,
			      struct net_device *new_dev, uint32_t sub_port)
{
	int fwt_index;
	fwt_db_entry *db_ent;
	unsigned long flags;
	int ret = CLS_FWT_UPDATE_ACCEPT;

	local_irq_save(flags);

	fwt_index = fwt_sw_get_index_from_mac_vid(mac_be, vlan_id);
	db_ent = fwt_db_get_valid_entry(fwt_index);
	if (db_ent)
		ret = fwt_sw_check_should_update_local(fwt_index, new_dev, sub_port);

	local_irq_restore(flags);

	if (unlikely(!db_ent))
		pr_err("%s: [%pM.%u] entry not found\n", __func__, mac_be, vlan_id);

	return ret;
}

/*
 * 1. replace the non-updated entry directly
 * 2. fwt_db_entry.count is over FWT_CACHE_ENTRY_COUNT_THRESHOLD
 */
static void fwt_sw_cache_entry_update(int fwt_index)
{
#if CLS_FWT_CACHE_ENTRY_ENABLE
	int i;
	fwt_db_entry *db_ent = NULL;

	db_ent = fwt_db_get_valid_entry(fwt_index);
	if (!db_ent)
		return;

	// check cache entry
	for (i = 0; i < CLS_FWT_CACHE_ENTRY_SIZE; i++) {
		if (g_fwt_cache[i] == fwt_index) {// existed cache entry
			return;
		} else if (g_fwt_cache[i] == -1) {// empty cache entry
			g_fwt_cache[i] = fwt_index;

			goto UPDATE;
		}
	}

	if (db_ent->count < FWT_CACHE_ENTRY_COUNT_THRESHOLD) {
		db_ent->count++;

		return;
	}

	db_ent->count = 0;

	g_fwt_cache[g_fwt_cache_entry_index] = fwt_index;

UPDATE:
	if ((++g_fwt_cache_entry_index) == CLS_FWT_CACHE_ENTRY_SIZE)
		g_fwt_cache_entry_index = 0;
#endif
}

static int fwt_sw_get_index_from_cache(const uint8_t *mac_be, const uint16_t vlan_id)
{
#if CLS_FWT_CACHE_ENTRY_ENABLE
	int i;
	int index;
	fwt_db_entry *db_ent;

	for (i = 0; i < CLS_FWT_CACHE_ENTRY_SIZE; i++) {
		index = g_fwt_cache[i];

		db_ent = fwt_db_get_valid_entry(index);
		if (!db_ent)
			continue;

		if (fwt_sw_entry_match_with_mac_vid(db_ent, mac_be, vlan_id))
			return index;
	}
#endif
	return -1;
}

static int fwt_sw_get_index_quickly(const uint8_t *mac_be, const uint16_t vlan_id)
{
	int index;

	index = fwt_sw_get_index_from_cache(mac_be, vlan_id);

	if (unlikely(index < 0)) {
		index = fwt_sw_get_index_from_mac_vid(mac_be, vlan_id);
		if (!fwt_db_get_valid_entry(index))
			return -1;
	}

	return index;
}

struct net_device *fwt_sw_get_entry_info(const uint8_t *mac_be, uint16_t vlan_id,
						uint16_t *index, uint32_t *sub_port)
{
	fwt_db_entry *fwt_ent;
	int tmp_idx;
	struct net_device *outdev;

	unsigned long flags;
	unsigned long spin_lock_flags;

	local_irq_save(flags);

	tmp_idx = fwt_sw_get_index_quickly(mac_be, vlan_id);
	if (tmp_idx < 0) {
		local_irq_restore(flags);
		return NULL;
	}

	// update cache entry array.
	fwt_sw_cache_entry_update(tmp_idx);
	*index = tmp_idx;
	fwt_ent = fwt_db_get_table_entry(tmp_idx);

	spin_lock_irqsave(&cls_fwt_lock, spin_lock_flags);
	if (!fwt_ent->valid)  {
		spin_unlock_irqrestore(&cls_fwt_lock, spin_lock_flags);
		local_irq_restore(flags);
		return NULL;
	}
	*sub_port = fwt_sw_get_entry_sub_port(fwt_ent);
	outdev = fwt_ent->out_dev;
	spin_unlock_irqrestore(&cls_fwt_lock, spin_lock_flags);

	local_irq_restore(flags);

	return outdev;
}

/*
 * In learning mode, check switch dynamic l2 table whether does it still exist.
 * if not, maek it aged out in kernel fdb.
 */
static void fwt_timer_callback(struct timer_list *timer)
{
	int i;
	fwt_db_entry *entry;
	int (*switch_check_l2_entry)(u16 fwt_index, u8 *mac, u16 vid);
	unsigned long flags;
	static DEFINE_RATELIMIT_STATE(rl, 1 * HZ, 10);

	if (!cls_fwt_switch_enable)
		goto out;

	switch_check_l2_entry = symbol_get(cls_npe_l2_entry_lookup_is_valid);
	if (!switch_check_l2_entry)
		goto out;

	local_irq_save(flags);

	for (i = 0; i < CLS_FWT_TOTAL_ENTRIES; i++) {
		entry = fwt_db_get_valid_entry(i);
		if (entry && entry->node_type == CLS_FWT_ENTRY_TYPE_ETH && !entry->aged_out &&
				!switch_check_l2_entry(entry->fwt_index, entry->mac_id,
					entry->vlan_id)) {
			fwt_make_entry_aged_out(i);

			if (__ratelimit(&rl))
				pr_info("[%s] delete fwt_index %d: [%pM.%u]\n", __func__,
						entry->fwt_index, entry->mac_id, entry->vlan_id);
		}
	}

	local_irq_restore(flags);

	symbol_put(cls_npe_l2_entry_lookup_is_valid);

out:
	mod_timer(timer, jiffies + msecs_to_jiffies(fwt_timer_delay));
}

static int __init fwt_sw_init(void)
{
	pr_info("[%s] enter\n", __func__);

	spin_lock_init(&cls_fwt_lock);

	br_register_hooks_cbk_t(fwt_sw_add_device, fwt_sw_delete_device,
			fwt_sw_get_timestamp, fwt_sw_reset_timestamp_by_index,
			fwt_sw_check_should_update,
			fwt_sw_get_entry_info);

	fwt_db_init();

	cls_fwt_dbg_init();

	cls_fwt_g_enable = 1;

	cls_fwt_switch_enable = switch_mod;

	g_fwt_cache_entry_index = 0;

	timer_setup(&fwt_timer, fwt_timer_callback, 0);
	// unsupported learning pky in phase1
	if (cls_fwt_switch_enable == CLS_FWT_SWITCH_MODE_LEARNING_PKT)
		mod_timer(&fwt_timer, jiffies + msecs_to_jiffies(fwt_timer_delay));

	return 0;
}

static void __exit fwt_sw_exit(void)
{
	pr_info("[%s] enter\n", __func__);

	br_register_hooks_cbk_t(NULL, NULL, NULL, NULL, NULL, NULL);

	cls_fwt_g_enable = 0;

	cls_fwt_dbg_exit();

	del_timer(&fwt_timer);
}

module_init(fwt_sw_init);
module_exit(fwt_sw_exit);
