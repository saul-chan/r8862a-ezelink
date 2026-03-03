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

/* Mirror table to the kernel fdb entry */
static fwt_db_entry g_fwt_db[CLS_FWT_TOTAL_ENTRIES];

/* Node table */
#define CLS_FWT_DB_RADIO_OFFSET	0x80
fwt_db_node_element_head g_fwt_db_node_table[CLS_NCIDX_MAX];

uint16_t fwt_get_db_table_index_by_radio(uint8_t radio, uint16_t node_idx)
{
	if (radio)
		return CLS_FWT_DB_RADIO_OFFSET + node_idx;
	else
		return node_idx;
}

//without check index
fwt_db_entry *fwt_db_get_table_entry(uint16_t fwt_idx)
{
	return &g_fwt_db[fwt_idx];
}

fwt_db_entry *fwt_db_get_valid_entry(int fwt_idx)
{
	fwt_db_entry *fwt_ent;

	if (fwt_idx < 0 || fwt_idx >= CLS_FWT_TOTAL_ENTRIES)
		return NULL;

	fwt_ent = fwt_db_get_table_entry(fwt_idx);
	if (!fwt_ent->valid || (fwt_idx != fwt_ent->fwt_index))
		return NULL;

	return fwt_ent;
}

/*
 * Allocate memory for new node element.
 * @return node element entry
 */
static fwt_db_node_element *fwt_db_create_node_element(void)
{

	/* allocate mem for new element */
	fwt_db_node_element *new_element = kzalloc(sizeof(fwt_db_node_element), GFP_ATOMIC);

	if (!new_element) {
		pr_err("%s: Not enough memory\n", __func__);

		return NULL;
	}

	return new_element;
}

int fwt_db_init_entry(struct fwt_db_entry *entry)
{
	unsigned long spin_lock_flags;

	if (entry == NULL)
		return -EINVAL;

	spin_lock_irqsave(&cls_fwt_lock, spin_lock_flags);
	entry->valid = false;
	spin_unlock_irqrestore(&cls_fwt_lock, spin_lock_flags);

	memset(entry, 0, sizeof(struct fwt_db_entry));

	entry->valid = false;
	entry->fwt_index = -1;
	entry->count = 0;
	entry->timestamp = jiffies;

	return 0;
}

static bool fwt_db_is_node_exists_list(uint16_t node_tbl_idx, uint16_t fwt_idx, struct net_device *dev)
{
	fwt_db_node_element *ptr = NULL;

	STAILQ_FOREACH(ptr, &g_fwt_db_node_table[node_tbl_idx], next) {
		if (ptr->index == fwt_idx && ptr->dev == dev)
			return true;
	}
	return false;
}

/* once association, the sta_in_ap/ap_in_sta will be the first in node list */
int fwt_db_add_new_node(uint8_t radio_idx, uint16_t node_idx, uint16_t fwt_idx, struct net_device *dev)
{
	fwt_db_node_element *new_element;
	uint16_t node_tbl_idx = fwt_get_db_table_index_by_radio(radio_idx, node_idx);

	if (fwt_db_is_node_exists_list(node_tbl_idx, fwt_idx, dev) == false) {
		new_element = fwt_db_create_node_element();
		if (new_element) {
			new_element->index = fwt_idx;
			new_element->dev = dev;

			pr_info("[%s] list[0x%x] add [fwt_idx %d out_dev %s]\n",
					__func__, node_tbl_idx, new_element->index, dev->name);

			STAILQ_INSERT_TAIL(&g_fwt_db_node_table[node_tbl_idx], new_element, next);
		}

		return 1;
	}
	return 0;
}

void fwt_db_delete_index_from_node_table(uint8_t radio_idx, uint16_t node_idx, uint16_t fwt_idx, struct net_device *dev)
{
	fwt_db_node_element *ptr = NULL;
	fwt_db_node_element *tmp = NULL;
	uint16_t node_tbl_idx = fwt_get_db_table_index_by_radio(radio_idx, node_idx);

	STAILQ_FOREACH_SAFE(ptr, &g_fwt_db_node_table[node_tbl_idx], next, tmp)
	{
		/* find the specific index */
		if ((ptr->index == fwt_idx) && (ptr->dev == dev)) {
			pr_info("[%s] list[0x%x] del [fwt_idx %d]\n",
							__func__, node_tbl_idx, ptr->index);

			STAILQ_REMOVE(&g_fwt_db_node_table[node_tbl_idx], ptr,
							fwt_db_node_element, next);
			kfree(ptr);
			ptr = NULL;
		}
	}
}

bool fwt_db_devide_is_head_in_node_list(fwt_db_entry *entry)
{
	uint16_t node_tbl_idx;
	fwt_db_node_element *element;

	if (!entry)
		return false;

	node_tbl_idx = fwt_get_db_table_index_by_radio(entry->out_radio, entry->out_node);

	element = STAILQ_FIRST(&g_fwt_db_node_table[node_tbl_idx]);

	if (element && (element->index == entry->fwt_index))
		return true;
	else
		return false;
}

int fwt_db_clear_node(fwt_db_entry *entry)
{
	uint16_t count = 0;
	uint16_t node_tbl_idx;
	fwt_db_node_element *ptr = NULL;
	fwt_db_node_element *tmp = NULL;

	if (!entry)
		return 0;

	node_tbl_idx = fwt_get_db_table_index_by_radio(entry->out_radio, entry->out_node);

	pr_info("[%s] list[0x%x] dev %s [%pM.%d]\n",
			__func__, node_tbl_idx, entry->out_dev->name, entry->mac_id, entry->vlan_id);

	STAILQ_FOREACH_SAFE(ptr, &g_fwt_db_node_table[node_tbl_idx], next, tmp)
	{

		pr_info("[%s] list[0x%x] del [fwt_idx %d]\n", __func__, node_tbl_idx, ptr->index);

		fwt_db_delete_table_entry(ptr->index);

		STAILQ_REMOVE(&g_fwt_db_node_table[node_tbl_idx], ptr, fwt_db_node_element, next);
		kfree(ptr);
		ptr = NULL;
		count++;
	}

	return count;
}

int fwt_db_table_insert(uint16_t fwt_idx, fwt_db_entry *element)
{
	unsigned long spin_lock_flags;

	void (*switch_add_l2_entry)(u16 fwt_index, u8 *mac, u16 vid, u8 dport, u16 sta_idx);

	if (element == NULL)
		return -EINVAL;

	spin_lock_irqsave(&cls_fwt_lock, spin_lock_flags);
	memcpy(&g_fwt_db[fwt_idx], element, sizeof(fwt_db_entry));
	spin_unlock_irqrestore(&cls_fwt_lock, spin_lock_flags);

	if (cls_fwt_switch_enable) {
		switch_add_l2_entry = symbol_get(cls_npe_l2_update_entry);
		if (switch_add_l2_entry) {
			switch_add_l2_entry(element->fwt_index, element->mac_id, element->vlan_id,
					    element->out_dev->if_port, element->out_node);
			symbol_put(cls_npe_l2_update_entry);
		}
	}

	return FWT_DB_STATUS_SUCCESS;
}

// check index before call this function
int fwt_db_update_params(uint16_t fwt_idx, struct net_device *dev, uint32_t sub_port)
{
	uint8_t node_type = 0;
	uint8_t is_4addr = 0;
	uint8_t is_wireless = 0;
	unsigned long spin_lock_flags;
	static DEFINE_RATELIMIT_STATE(rl, 1 * HZ, 10);

	spin_lock_irqsave(&cls_fwt_lock, spin_lock_flags);
	if (g_fwt_db[fwt_idx].valid == false) {
		spin_unlock_irqrestore(&cls_fwt_lock, spin_lock_flags);
		return -ENOENT;
	}

	g_fwt_db[fwt_idx].out_dev = dev;

	is_wireless = fwt_sw_parse_sub_port(sub_port, &g_fwt_db[fwt_idx].out_node, &g_fwt_db[fwt_idx].out_radio,
			&g_fwt_db[fwt_idx].out_vif_idx, &node_type, &is_4addr);

	g_fwt_db[fwt_idx].portal = !!is_4addr;
	g_fwt_db[fwt_idx].wireless = !!is_wireless;
	g_fwt_db[fwt_idx].node_type = node_type;

	spin_unlock_irqrestore(&cls_fwt_lock, spin_lock_flags);

	if (__ratelimit(&rl))
		pr_info("[%s] index=%d dev=%s sub_port=0x%x(node=%d radio=%d vif_idx=%d type=%d)\n",
			__func__, fwt_idx, g_fwt_db[fwt_idx].out_dev->name, sub_port,
			g_fwt_db[fwt_idx].out_node, g_fwt_db[fwt_idx].out_radio,
			g_fwt_db[fwt_idx].out_vif_idx, g_fwt_db[fwt_idx].node_type);

	return 1;
}

uint16_t fwt_db_calculate_aging_sec(uint16_t fwt_idx)
{
	fwt_db_entry *fwt_ent = NULL;

	fwt_ent = fwt_db_get_valid_entry(fwt_idx);

	if (fwt_ent)
		return (jiffies_to_secs(jiffies - fwt_ent->timestamp));
	else
		return 0;
}

void fwt_db_delete_table_entry(uint16_t fwt_idx)
{
	struct fwt_db_entry *db_ent;
	void (*switch_del_l2_entry)(u16 index);

	db_ent = fwt_db_get_valid_entry(fwt_idx);
	if (!db_ent)
		return;

	if (cls_fwt_switch_enable) {
		switch_del_l2_entry = symbol_get(cls_npe_l2_del_entry);
		if (switch_del_l2_entry) {
			switch_del_l2_entry(db_ent->fwt_index);
			symbol_put(cls_npe_l2_del_entry);
		}
	}

	fwt_db_init_entry(db_ent);
}

int fwt_db_print_unicast_entry(uint32_t index)
{
	char *is_4addr[] = {"no", "yes"};
	int ret = 0;

	if (index < CLS_FWT_TOTAL_ENTRIES) {
		if (g_fwt_db[index].valid && !ETHER_IS_MULTICAST(g_fwt_db[index].mac_id)) {
			ret = 1;
			pr_info("%-5u %pM %4u %5s %6u %7s %4u %4u %3u %4u %5u\n",
					index,
					g_fwt_db[index].mac_id,
					g_fwt_db[index].vlan_id,
					is_4addr[g_fwt_db[index].portal],
					fwt_db_calculate_aging_sec(index),
					g_fwt_db[index].out_dev->name,
					g_fwt_db[index].out_dev->if_port,
					g_fwt_db[index].out_node,
					g_fwt_db[index].out_vif_idx,
					g_fwt_db[index].node_type,
					g_fwt_db[index].count);
		}
	}

	return ret;
}

int fwt_db_print(void)
{
	int count = 0;
	int i;

	pr_info("Unicast Table\n");
	pr_info("Index        MAC        VLAN 4addr Ageing DevName Port Node VIF Type Count\n");

	for (i = 0; i < CLS_FWT_TOTAL_ENTRIES; i++) {
		if (fwt_db_print_unicast_entry(i))
			count++;
	}

	pr_info("Unicast Table Count: %d\n", count);

	return count;
}

void fwt_db_init(void)
{
	int i;

	for (i = 0; i < CLS_FWT_TOTAL_ENTRIES; i++)
		fwt_db_init_entry(&g_fwt_db[i]);

	for (i = 0; i < CLS_NCIDX_MAX; i++)
		STAILQ_INIT(&g_fwt_db_node_table[i]);

	for (i = 0; i < CLS_FWT_CACHE_ENTRY_SIZE; i++)
		g_fwt_cache[i] = -1;

}

/********************** Hash Calculation ********************************/
//return (gid << 48 + macaddr)
static uint64_t fwt_build_hashkey(const uint8_t *mac_be, uint16_t vid)
{
	uint64_t key, tmp;

	tmp = vid;
	tmp = tmp << 48;

	key = mac_be[0];
	key = (key << 8) + mac_be[1];
	key = (key << 8) + mac_be[2];
	key = (key << 8) + mac_be[3];
	key = (key << 8) + mac_be[4];
	key = (key << 8) + mac_be[5];

	key = key + tmp;

	return key;
}

static uint16_t switch_l2_hash_calc(const uint8_t *mac_be, uint16_t vid)
{
	uint16_t hashval = 0;
	uint64_t key = 0;

	key = fwt_build_hashkey(mac_be, vid);

	hashval = key & 0x3FF;
	hashval = hashval ^ (key >> 10);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 20);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 30);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 40);
	hashval = hashval & 0x3FF;
	hashval = hashval ^ (key >> 50);
	hashval = hashval & 0x3FF;

	return hashval;
}

//match: return 1;
int fwt_sw_entry_match_with_mac_vid(fwt_db_entry *fwt_ent,
					const uint8_t *mac_be, const uint16_t vid)
{
	if (fwt_ent && (vid == fwt_ent->vlan_id)
	    && ether_addr_equal_64bits(fwt_ent->mac_id, mac_be))
		return 1;
	else
		return 0;
}

int fwt_sw_get_index_from_mac_vid(const uint8_t *mac_be, const uint16_t vid)
{
	uint16_t bucket;
	int hashkey, index;
	fwt_db_entry *fwt_ent;

	if (mac_be == NULL || vid >= VLAN_ID_MAX)
		return -EINVAL;

	//hash
	hashkey = switch_l2_hash_calc(mac_be, vid);

	for (bucket = 0; bucket < CLS_FWT_BUCKET_NUM; bucket++) {
		index = hashkey + (bucket << CLS_FWT_BUCKET_SHIFT);
		fwt_ent = fwt_db_get_table_entry(index);

		if (fwt_sw_entry_match_with_mac_vid(fwt_ent, mac_be, vid))
			return index;
	}

	//collision
	for (index = CLS_FWT_HASH_ENTRIES; index < CLS_FWT_TOTAL_ENTRIES; index++) {
		fwt_ent = fwt_db_get_table_entry(index);

		if (fwt_sw_entry_match_with_mac_vid(fwt_ent, mac_be, vid))
			return index;
	}

	return -EINVAL;
}

int fwt_sw_new_index_from_mac_vid(const uint8_t *mac_be, const uint16_t vid)
{
	uint16_t bucket;
	int hashkey, index;

	if (mac_be == NULL || vid >= VLAN_ID_MAX)
		return -EINVAL;

	//hash
	hashkey = switch_l2_hash_calc(mac_be, vid);

	for (bucket = 0; bucket < CLS_FWT_BUCKET_NUM; bucket++) {
		index = hashkey + (bucket << CLS_FWT_BUCKET_SHIFT);
		if (!fwt_db_get_valid_entry(index))
			return index;
	}

	//collision
	for (index = CLS_FWT_HASH_ENTRIES; index < CLS_FWT_TOTAL_ENTRIES; index++) {
		if (!fwt_db_get_valid_entry(index))
			return index;
	}

	return -EINVAL;
}
/**************************************************************************/
