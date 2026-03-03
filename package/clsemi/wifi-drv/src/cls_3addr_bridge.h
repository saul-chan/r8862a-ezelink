#ifndef CLS_3ADDR_BR_H
#define CLS_3ADDR_BR_H
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/in6.h>
#include "cls_wifi_defs.h"

#define ETH_ALEN	6
#define CLS_WIFI_BR_ENT_MAX			512	/* unicast clients */
#define CLS_WIFI_BR_MCAST_MAX		128	/* multicast addresses & clients */
#define CLS_WIFI_BR_MAC_HASH_SIZE		256
#define CLS_WIFI_BR_IP_HASH_SIZE		256

#define ETHER_TYPE_UNKNOWN 0XFFFF

#define BOOTREQUEST     1
#define DHCPREQUEST     3
#define ARPHRD_ETHER    1
#define DHCP_BROADCAST_FLAG     0x8000

struct ether_arp {
	struct	arphdr ea_hdr;		/* fixed-size header */
	u_int8_t arp_sha[ETH_ALEN];	/* sender hardware address */
	u_int8_t arp_spa[4];		/* sender protocol address */
	u_int8_t arp_tha[ETH_ALEN];	/* target hardware address */
	u_int8_t arp_tpa[4];		/* target protocol address */
};

extern int skb_l4_proto(struct sk_buff *skb, bool *is_bcast, bool *is_mcast, uint8_t **data_start);

extern int cls_wifi_rx_set_dest_mac(struct cls_wifi_vif *vif, const struct sk_buff *skb);
extern int cls_wifi_tx_3addr_check_arp(struct cls_wifi_vif *vif, struct sk_buff *skb, uint8_t *data_start);
extern int cls_wifi_tx_3addr_check_ip(struct sk_buff *skb, struct cls_wifi_vif *vif, struct ethhdr *eh, uint8_t *data_start);

typedef int (*skb_rx_handler_t)(struct cls_wifi_vif *vif, struct sk_buff *skb);
typedef int (*skb_tx_handler_t)(struct cls_wifi_vif *vif, struct sk_buff *skb);


extern skb_rx_handler_t cls_3addr_br_rx_callback;
extern skb_tx_handler_t cls_3addr_br_tx_callback;

/*
 * Each bridge table (one per vap) contains:
 * - a list of downstream (unicast) clients, keyed by MAC address and IP address hash
 * - a list of multicast addresses, keyed by IP address, containing a list of
 *   subscribed downstream client MAC addresses
 * All keys are implemented as hash tables.
 */
struct cls_wifi_br_uc {
	__be32				ip_addr;
	unsigned char			mac_addr[ETH_ALEN];
	struct hlist_node		mac_hlist;
	struct hlist_node		ip_hlist;
	unsigned long			timestamp;
	struct rcu_head			rcu;
};

struct cls_wifi_br_mc {
	__be32				mc_ip_addr;
	atomic_t			mc_client_tot;
	struct hlist_node		mc_hlist;
	struct hlist_head		mc_client_hash[CLS_WIFI_BR_IP_HASH_SIZE];
	struct rcu_head			rcu;
};

struct cls_wifi_br_mc_client {
	unsigned char			mac_addr[ETH_ALEN];
	struct hlist_node		mc_client_hlist;
	unsigned long			timestamp;
	struct rcu_head			rcu;
};

struct cls_wifi_br {
	spinlock_t			uc_lock;
	spinlock_t			mc_lock;
	unsigned long		uc_lock_flags;
	unsigned long		mc_lock_flags;
	atomic_t			uc_tot;
	atomic_t			mc_tot;	// Total multicast and client entries
	struct hlist_head		uc_mac_hash[CLS_WIFI_BR_MAC_HASH_SIZE];
	struct hlist_head		uc_ip_hash[CLS_WIFI_BR_IP_HASH_SIZE];
	struct hlist_head		mc_ip_hash[CLS_WIFI_BR_IP_HASH_SIZE];
};

void cls_wifi_br_create(void);
void cls_wifi_br_exit(void);
void cls_wifi_br_delete(struct cls_wifi_br *br);
void cls_wifi_br_show(struct cls_wifi_br *br);
void cls_wifi_br_clear(struct cls_wifi_br *br);
void cls_wifi_br_uc_update_from_dhcp(struct cls_wifi_br *br, struct sk_buff *skb,
		struct iphdr *iphdr_p);
void cls_wifi_br_uc_update_from_arp(struct cls_wifi_br *br, struct ether_arp *arp);
int cls_wifi_br_mc_update_from_igmp(struct cls_wifi_br *br, struct sk_buff *skb,
		struct ethhdr *eh, struct iphdr *iphdr_p);
int cls_wifi_br_set_rx_dest_mac(struct cls_wifi_br *br, const struct sk_buff *skb);

#define CLS_WIFI_FLAG_3ADDR_BRIDGE_ENABLED() (vif->use_3addr_br)

typedef enum {
	REPLACE_IP_MAC = 0,
	SAVE_IP_MAC = 1
} ip_mac_flag;

struct ip_mac_mapping {
	struct          ip_mac_mapping *next;
	__be32          ip_addr;
	u_int8_t        mac[ETH_ALEN];
};

struct qdrv_br {
	spinlock_t                      uc_lock;
	spinlock_t                      mc_lock;
	unsigned long                   uc_lock_flags;
	unsigned long                   mc_lock_flags;
	atomic_t                        uc_tot;
	atomic_t                        mc_tot; // Total multicast and client entries
	struct hlist_head               uc_mac_hash[CLS_WIFI_BR_MAC_HASH_SIZE];
	struct hlist_head               uc_ip_hash[CLS_WIFI_BR_IP_HASH_SIZE];
	struct hlist_head               mc_ip_hash[CLS_WIFI_BR_IP_HASH_SIZE];
	unsigned long                   flags;
	uint8_t                         ic_addr[ETH_ALEN];
	uint8_t                         dhcp_chaddr[ETH_ALEN];
};

/*
 * IEEE 802.2 Link Level Control headers, for use in conjunction with
 * 802.{3,4,5} media access control methods.
 *
 * Headers here do not use bit fields due to shortcommings in many
 * compilers.
 */

struct llc {
	u_int8_t llc_dsap;
	u_int8_t llc_ssap;
	union {
		struct {
			u_int8_t control;
			u_int8_t format_id;
			u_int8_t class;
			u_int8_t window_x2;
		} __packed type_u;
		struct {
			u_int8_t num_snd_x2;
			u_int8_t num_rcv_x2;
		} __packed type_i;
		struct {
			u_int8_t control;
			u_int8_t num_rcv_x2;
		} __packed type_s;
		struct {
			u_int8_t control;
			/*
			 * We cannot put the following fields in a structure because
			 * the structure rounding might cause padding.
			 */
			u_int8_t frmr_rej_pdu0;
			u_int8_t frmr_rej_pdu1;
			u_int8_t frmr_control;
			u_int8_t frmr_control_ext;
			u_int8_t frmr_cause;
		} __packed type_frmr;
		struct {
			u_int8_t  control;
			u_int8_t  org_code[3];
			u_int16_t ether_type;
		} __packed type_snap;
		struct {
			u_int8_t control;
			u_int8_t control_ext;
		} __packed type_raw;
	} llc_un /* XXX __packed ??? */;
} __packed;


#define PEER_VENDOR_NONE        0x00
#define PEER_VENDOR_QTN         0x01
#define PEER_VENDOR_BRCM        0x02
#define PEER_VENDOR_ATH         0x04
#define PEER_VENDOR_RLNK        0x08
#define PEER_VENDOR_RTK         0x10
#define PEER_VENDOR_INTEL       0x20

/* Compatibility fix bitmap for various vendor peer */
#define VENDOR_FIX_BRCM_DHCP                    0x01
#define VENDOR_FIX_BRCM_REPLACE_IGMP_SRCMAC     0x02
#define VENDOR_FIX_BRCM_REPLACE_IP_SRCMAC       0x04
#define VENDOR_FIX_BRCM_DROP_STA_IGMPQUERY      0x08
#define VENDOR_FIX_BRCM_AP_GEN_IGMPQUERY        0x10

#define ETHER_MAX_LEN		1518
#define LLC_SNAPFRAMELEN	8
#define LLC_SNAP_LSAP       0xaa

/*
 * Skip over L2 headers in a buffer
 * Returns Ethernet type and a pointer to the payload
 */
static inline void * cls_wifi_find_data_start(struct sk_buff *skb,
		struct ethhdr *eh, u16 *ether_type)
{
	struct llc *llc_p;
	struct vlan_ethhdr *vlan_ethhdr_p;

	if (ntohs(eh->h_proto) < ETHER_MAX_LEN) {
		llc_p = (struct llc *)(eh + 1);
		if ((skb->len >= LLC_SNAPFRAMELEN) &&
				(llc_p->llc_dsap == LLC_SNAP_LSAP) &&
				(llc_p->llc_ssap == LLC_SNAP_LSAP)) {
			*ether_type = llc_p->llc_un.type_snap.ether_type;
			return (void *)((char *)(eh + 1) - sizeof(ether_type) + LLC_SNAPFRAMELEN);
		} else {
			*ether_type = ETHER_TYPE_UNKNOWN;
			return (void *)(eh + 1);
		}
	} else if (ntohs(eh->h_proto) == ETH_P_8021Q) {
		vlan_ethhdr_p = (struct vlan_ethhdr *)eh;
		*ether_type = vlan_ethhdr_p->h_vlan_encapsulated_proto;
		skb->vlan_tci = ntohs(get_unaligned((__be16 *)(&vlan_ethhdr_p->h_vlan_TCI)));
		return (void *)(vlan_ethhdr_p + 1);
	} else {
		*ether_type = eh->h_proto;
		return (void *)(eh + 1);
	}
}

static inline void ipv6_addr_copy(struct in6_addr *a1, const struct in6_addr *a2)
{
	*a1 = *a2;
}
#endif
