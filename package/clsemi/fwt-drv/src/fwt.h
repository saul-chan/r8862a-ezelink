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

#ifndef __FWT_H__
#define __FWT_H__

#include <linux/platform_device.h>
#include <net/ip6_checksum.h>
#include <linux/io.h>
#include <linux/prefetch.h>
#include <linux/bitops.h>
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
#include "../net/bridge/br_private.h"

/*
 * Singly-linked Tail queue declarations.
 */
#define	STAILQ_HEAD(name, type)						\
struct name {								\
	struct type *stqh_first;/* first element */			\
	struct type **stqh_last;/* addr of last next element */		\
}

#define	STAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).stqh_first }

#define	STAILQ_ENTRY(type)						\
struct {								\
	struct type *stqe_next;	/* next element */			\
}

/*
 * Singly-linked Tail queue functions.
 */
#define	STAILQ_EMPTY(head)	((head)->stqh_first == NULL)

#define	STAILQ_FIRST(head)	((head)->stqh_first)

#define	STAILQ_INIT(head) do {						\
	STAILQ_FIRST((head)) = NULL;					\
	(head)->stqh_last = &STAILQ_FIRST((head));			\
} while (0)

#define	STAILQ_FOREACH(var, head, field)				\
	for ((var) = STAILQ_FIRST((head));				\
		(var);							\
		(var) = STAILQ_NEXT((var), field))

#define STAILQ_FOREACH_SAFE(var, head, field, tvar)           \
	for ((var) = STAILQ_FIRST((head));                     \
		(var) && ((tvar) = STAILQ_NEXT((var), field), 1); \
		(var) = (tvar))

#define	STAILQ_INSERT_AFTER(head, tqelm, elm, field) do {		\
	if ((STAILQ_NEXT((elm), field) = STAILQ_NEXT((tqelm), field)) == NULL)\
		(head)->stqh_last = &STAILQ_NEXT((elm), field);		\
	STAILQ_NEXT((tqelm), field) = (elm);				\
} while (0)

#define	STAILQ_INSERT_HEAD(head, elm, field) do {			\
	if ((STAILQ_NEXT((elm), field) = STAILQ_FIRST((head))) == NULL)	\
		(head)->stqh_last = &STAILQ_NEXT((elm), field);		\
	STAILQ_FIRST((head)) = (elm);					\
} while (0)

#define	STAILQ_INSERT_TAIL(head, elm, field) do {			\
	STAILQ_NEXT((elm), field) = NULL;				\
	STAILQ_LAST((head)) = (elm);					\
	(head)->stqh_last = &STAILQ_NEXT((elm), field);			\
} while (0)

#define	STAILQ_LAST(head)	(*(head)->stqh_last)

#define	STAILQ_NEXT(elm, field)	((elm)->field.stqe_next)

#define	STAILQ_REMOVE(head, elm, type, field) do {			\
	if (STAILQ_FIRST((head)) == (elm)) {				\
		STAILQ_REMOVE_HEAD(head, field);			\
	}								\
	else {								\
		struct type *curelm = STAILQ_FIRST((head));		\
		while (STAILQ_NEXT(curelm, field) != (elm))		\
			curelm = STAILQ_NEXT(curelm, field);		\
		if ((STAILQ_NEXT(curelm, field) =			\
		     STAILQ_NEXT(STAILQ_NEXT(curelm, field), field)) == NULL)\
			(head)->stqh_last = &STAILQ_NEXT((curelm), field);\
	}								\
} while (0)

#define	STAILQ_REMOVE_HEAD(head, field) do {				\
	if ((STAILQ_FIRST((head)) =					\
	     STAILQ_NEXT(STAILQ_FIRST((head)), field)) == NULL)		\
		(head)->stqh_last = &STAILQ_FIRST((head));		\
} while (0)

#define	STAILQ_REMOVE_HEAD_UNTIL(head, elm, field) do {			\
	if ((STAILQ_FIRST((head)) = STAILQ_NEXT((elm), field)) == NULL)	\
		(head)->stqh_last = &STAILQ_FIRST((head));		\
} while (0)

/* Singly-linked Tail queue declarations end.*/

#define jiffies_to_secs(_x)	(jiffies_to_msecs(_x) / MSEC_PER_SEC)

/*
 * LHost FWT entry copy.
 * Sufficient for unicast; multicast with multiple ports/node is unsupported.
 * !!!node index is unmapped
 */
typedef struct fwt_db_entry {
	uint8_t mac_id[ETH_ALEN];
	uint16_t vlan_id;
	uint16_t vlan_enabled;
	uint16_t out_node;// Indentify the Devices(PC/phone..) connected with the same station
	uint8_t out_vif_idx; //vap index
	uint8_t out_radio; //radio
	struct net_device *out_dev;
	uint64_t timestamp;// jiffies
	int16_t fwt_index;
	uint32_t count; //cache
	uint16_t portal	:1,//use 4addr
		valid	:1,
		mcast	:1,
		wireless:1,
		/*
		 * static entry should be aged out in learning mode,
		 * it means that return aged timestamp (not jiffies) to kernel fdb.
		 */
		aged_out:1,
		node_type:4;// Will not be modified after fwt_sw_add_device()
} fwt_db_entry;

/* node list indexed by the fwt */
typedef struct fwt_db_node_element {
	struct net_device *dev;
	uint16_t index;

	STAILQ_ENTRY(fwt_db_node_element) next;
} fwt_db_node_element;

#define CLS_FWT_CACHE_ENTRY_SIZE	4

#define FWT_CACHE_ENTRY_COUNT_THRESHOLD	1024

typedef STAILQ_HEAD(, fwt_db_node_element) fwt_db_node_element_head;

/* Set success return status */
#define FWT_DB_STATUS_SUCCESS (1)

/* Set Invalid node number */
#define FWT_DB_INVALID_NODE (0xFF)

/* Set Invalid IPV4 value */
#define FWT_DB_INVALID_IPV4 (0xFF)

#define CLS_FWT_HASH_ENTRIES		4096
#define CLS_FWT_COLLISION_ENTRIES	0//32
#define CLS_FWT_TOTAL_ENTRIES		(CLS_FWT_HASH_ENTRIES + CLS_FWT_COLLISION_ENTRIES)
#define CLS_FWT_BUCKET_NUM	4
#define CLS_FWT_BUCKET_SHIFT	10

#define VLAN_ID_MAX	4096

#define CLS_NCIDX_MAX_PER_RADIO	128// CFG_STA_MAX
#define CLS_NCIDX_RADIO_NUM	2// 2.4G and 5G
#define CLS_NCIDX_MAX	(CLS_NCIDX_MAX_PER_RADIO * CLS_NCIDX_RADIO_NUM)

/* Success definition in FWT Interface is return positive value */
#define FWT_IF_SUCCESS(x)	((x) >= 0)
/* Error definition in FWT Interface is return negative value */
#define FWT_IF_ERROR(x)		(!(FWT_IF_SUCCESS(x)))

#define	ETHER_IS_MULTICAST(addr) (*(addr) & 0x01) /* is address mcast/bcast? */

#define FWT_IS_STATIC_ENTRY(t) \
		((t == CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE) || \
				(t == CLS_FWT_ENTRY_TYPE_AP_IN_STA_MODE))

#define FWT_IS_WDS_ENTRY(t) \
		((t == CLS_FWT_ENTRY_TYPE_WDS_IN_AP_MODE) || \
				(t == CLS_FWT_ENTRY_TYPE_WDS_IN_STA_MODE))

fwt_db_entry *fwt_db_get_valid_entry(int fwt_idx);
fwt_db_entry *fwt_db_get_table_entry(uint16_t fwt_idx);

int fwt_db_init_entry(struct fwt_db_entry *entry);

int fwt_db_add_new_node(uint8_t radio_idx, uint16_t node_idx, uint16_t fwt_idx, struct net_device *dev);
int fwt_db_clear_node(struct fwt_db_entry *entry);
void fwt_db_delete_index_from_node_table(uint8_t radio_idx, uint16_t node_idx, uint16_t fwt_idx, struct net_device *dev);

bool fwt_db_devide_is_head_in_node_list(fwt_db_entry *entry);

int fwt_db_table_insert(uint16_t fwt_idx, fwt_db_entry *element);
void fwt_db_delete_table_entry(uint16_t fwt_idx);
int fwt_db_update_params(uint16_t fwt_idx, struct net_device *dev, uint32_t sub_port);

void fwt_sw_reset_timestamp_by_index_local(uint16_t fwt_index);
int fwt_sw_entry_match_with_mac_vid(fwt_db_entry *fwt_ent,
			const uint8_t *mac_be, const uint16_t vid);
int fwt_sw_get_index_from_mac_vid(const uint8_t *mac_be, const uint16_t vid);
int fwt_sw_new_index_from_mac_vid(const uint8_t *mac_be, const uint16_t vid);

struct net_device *fwt_sw_get_entry_info(const uint8_t *mac_be, uint16_t vlan_id,
						uint16_t *index, uint32_t *sub_port);
void fwt_db_init(void);

void cls_fwt_dbg_init(void);
void cls_fwt_dbg_exit(void);

int fwt_db_print(void);

int fwt_sw_parse_sub_port(uint32_t sub_port, uint16_t *node, uint8_t *radio,
			  uint8_t *vif_idx, uint8_t *node_type, uint8_t *is_4addr);

extern char cls_fwt_driver_name[];
extern uint16_t g_fwt_ent_cnt;
extern int32_t g_fwt_cache[CLS_FWT_CACHE_ENTRY_SIZE];

extern int g_fwt_cache_entry_index;

extern fwt_db_node_element_head g_fwt_db_node_table[CLS_NCIDX_MAX];

extern void cls_npe_l2_update_entry(u16 fwt_index, u8 *mac, u16 vid, u8 dport, u16 sta_idx);
extern void cls_npe_l2_del_entry(u16 fwt_index);
extern int cls_npe_l2_entry_lookup_is_valid(u16 fwt_index, u8 *mac, u16 vid);

extern spinlock_t cls_fwt_lock;
#endif /* __FWT_H__ */
