#ifndef _DATAMODEL_H_
#define _DATAMODEL_H_

#include <stdint.h>
#include <net/if.h>
#include <sys/time.h>
#include "narray.h"

#define NONCE_LEN   16
#define TIMESTAMP_MAX_LEN 64
#define SCAN_RETRY_MAX_TIMES 8

#define DM_UPDATE_DEVICE(_dev, _ridx)\
do {\
    if (((_dev)->recv_intf_idx) && ((_dev)->recv_intf_idx!=(_ridx))) {\
        DEBUG_WARNING("device["MACFMT"] move from %s to %s!", MACARG((_dev)->al_mac),\
                idx2InterfaceName((_dev)->recv_intf_idx), idx2InterfaceName((_ridx)));\
        (_dev)->recv_intf_idx = (_ridx);\
    }\
    (_dev)->ts_alive = PLATFORM_GET_TIMESTAMP(0);\
}while(0)

#define DM_CHECK_DEVICE(_dev, _mac, _ridx)\
do {\
    if (!((_dev) = alDeviceFind((_mac)))) {\
        (_dev) = alDeviceNew((_mac));\
        (_dev)->recv_intf_idx = (_ridx);\
        (_dev)->ts_alive = PLATFORM_GET_TIMESTAMP(0);\
        alDeviceUpdateEvt(_mac);\
    }\
}while(0)

#define BSS_SSID_IS_SAME(bss1, bss2) (0 == memcmp((bss1)->bssInfo.ssid.ssid, (bss2)->bssInfo.ssid.ssid, MAX_SSID_LEN))
#define BSS_AUTH_IS_SAME(bss1, bss2) (((bss1)->bssInfo.auth == (bss2)->bssInfo.auth))
#define BSS_ENCRYPT_IS_SAME(bss1, bss2) (((bss1)->bssInfo.encrypt == (bss2)->bssInfo.encrypt))
#define BSS_KEY_IS_SAME(bss1, bss2) (0 == memcmp((bss1)->bssInfo.key.key, (bss2)->bssInfo.key.key, MAX_KEY_LEN + 2))
#define BSS_IS_SAME(bss1, bss2) (BSS_SSID_IS_SAME(bss1, bss2) &&\
                                 BSS_AUTH_IS_SAME(bss1, bss2) &&\
                                 BSS_ENCRYPT_IS_SAME(bss1, bss2) &&\
                                 BSS_KEY_IS_SAME(bss1, bss2))

#define CLEAR_DLIST_MARK (_head, _type, _member, _mark) \
    do {\
        _type *tmp;\
        dlist_for_each(tmp, _head, _member) {\
            tmp->_mark.new = 0;\
            tmp->_mark.change = 0;\
        }\
    } while(0)


enum e_profile {
    profile_unknown = 0,
    profile_1 = 1,
    profile_2 = 2,
    profile_3 = 3,
};

enum e_wifi_role {
    role_ap = 0,
    role_sta = 0x4,

    role_unknown = 0xf,
};

enum e_wifi_band_idx {
    band_unknown = -1,
    band_2g_idx = 0,
    band_5g_idx,
    band_6g_idx,
    band_max_idx,
};

enum e_wifi_band {
    band_none = 0,
    band_2g = 0x1,
    band_5g = 0x2,
    band_6g = 0x4,
};

enum e_wifi_band_index {
    bw_idx_20MHz = 0,
    bw_idx_40MHz,
    bw_idx_80MHz,
    bw_idx_160MHz,
    bw_idx_80P80MHz,
    bw_idx_320Mhz,
    bw_idx_unknown,
    bw_idx_max,
};

enum e_interface_type {
    interface_type_unknown =  -1,       /**< Initiated type */
    interface_type_ethernet = 0,        /**< Ethernet interface */
    interface_type_wifi =     1,        /**< Wi-Fi interface. */
    interface_type_other =    255,      /**< Other interfaces types, not supported by this data model. */
};

enum e_power_state {
    power_state_on = 0,
    power_state_save = 1,
    power_state_off = 2,
};

enum e_auth {
    auth_open = 0x1,
    auth_wpapsk = 0x2,
    auth_shared = 0x4,
    auth_wpa = 0x8,
    auth_wpa2 = 0x10,
    auth_wpa2psk = 0x20,
    auth_sae = 0x40,
};

enum e_encrypt {
    encrypt_none = 0x1,
    encrypt_wep = 0x2,
    encrypt_tkip = 0x4,
    encrypt_aes = 0x8,
};

enum e_count_units {
    count_bytes = 0,
    count_kib = 1,
    count_mib = 2,
};

enum e_cac_methods {
    CAC_METHOD_CONTINUOUS_CAC = 0x0,
    CAC_METHOD_CONTINUOUS_CAC_DEDICATE_RADIO = 0x01,
    CAC_METHOD_MIMO_REDUCED_CAC = 0x02,
    CAC_METHOD_TIME_SLICED_CAC = 0x03,
    CAC_METHOD_MAX = 0x04
};

struct band_width{
    enum e_wifi_band_index bw;
    char *str;
};

struct l1vData {
    uint8_t len;
    uint8_t data[255];
};

struct vvData {
    uint16_t len;
    uint8_t *datap;
};

#define REPLACE_VVDATA(_vv, _new, _len) \
    do {\
        if ((_vv).datap) {\
            free((_vv).datap);\
            (_vv).datap = NULL;\
            (_vv).len = 0;\
        }\
        if ((_len) && (_new)) {\
            if (((_vv).datap = malloc(_len))) {\
                memcpy((_vv).datap, (_new), (_len));\
                (_vv).len = (_len);\
            }\
        }\
    }while(0)

#define MAX_SSID_LEN 32
struct ssid {
    uint8_t len;
    uint8_t ssid[MAX_SSID_LEN+2];
};

struct ipv4 {
    uint32_t ip;
};

struct ipv6 {
    uint32_t ip[4];
};

#define IP_PROTO_DHCP (3)
#define IP_PROTO_STATIC (2)
#define IP_PROTO_AUTO (1)

#define IPV6_IS_VALID(_ipv6) \
    ((_ipv6)[0] || (_ipv6)[1] || (_ipv6)[2] || (_ipv6)[3])

struct ipv4_item {
    dlist_item l;
    uint8_t proto;
    struct ipv4 ip;
    struct ipv4 dhcp_server;
};

struct ipv6_item {
    dlist_item l;
    uint8_t proto;
    struct ipv6 ip;
    struct ipv6 ip_origin;
};

#define MAX_HT_MCS_SIZE 16
struct ht_capability {
    uint8_t capa;                       /**< @brief HT Capabilities. */
};

struct vht_capability {
    uint16_t tx_mcs;                    /**< @brief Supported VHT Tx MCS. */
    uint16_t rx_mcs;                    /**< @brief Supported VHT Rx MCS. */
    uint16_t capa;                      /**< @brief VHT Capabilities. */
};

#define MAX_HE_MCS_SIZE 12
struct he_capability {
    //mcs[0]=len, mcs[1]..=value
    uint8_t mcs[1+MAX_HE_MCS_SIZE];                  /**< @brief Supported HE MCS.*/
    uint16_t capa;                      /**< @brief HE Capabilities. */
};

struct cac_capability {
    int32_t duration;     /**< @brief required cac period to complete CAC. */
};

#define MAX_CHANNEL_PER_OPCLASS 64
#define MAX_OPCLASS             32

struct chscan_req_item {
    dlist_item l;
    struct chscan_req *req;
    uint8_t opclass;                     /**< @brief Operation class. */
    uint8_t ch_num;                      /**< @brief Channle number. */
    uint8_t chans[MAX_CHANNEL_PER_OPCLASS]; /**< @brief Channel list. */
    uint8_t retries;                     /**< @brief Scan retry times. */
#define CHANNEL_SCAN_STATUS_NOT_START        0
#define CHANNEL_SCAN_STATUS_TRIGGERRED       1
#define CHANNEL_SCAN_STATUS_GETTING_RESULTS  2
#define CHANNEL_SCAN_STATUS_RESULTS_FINISHED 3
#define CHANNEL_SCAN_STATUS_ABORTED          4
    uint8_t status;                      /**< @brief result of performing scan on current channel. */
};

struct chscan_req {
    dlist_head h;
    struct radio *r;            /**< @brief Channel scan request radio. */
    mac_address src;            /**< @brief Reachable from local interface. */
    uint32_t ifindex;           /**< @brief Receiving interface index. */
    uint32_t ts;                /**< @brief Timestamp for request received. */
    uint32_t timeout;           /**< @brief Timerout for chan scan. */
    void *timer;                /**< @brief Timer for chan scan. */
    uint8_t status;             /**< @brief Status for this request. */
};

struct chan_info {
    uint8_t id;                       /**< @brief Channel ID */
#define MOST_PREF_SCORE (15)
    uint8_t pref;                     /**< @brief The preference value of the channel */
#define REASON_NON_SPECIFIC (0)
    uint8_t reason;                   /**< The reason of the preference */
    uint8_t freq_separation;          /**< Minimum frequency separation (in multiples of 10 MHz) that this radio would require when operating on this channel */
    uint8_t cac_method_mask;   /**< @brief CAC method mask */
    uint32_t ts_cacstat_changed;      /**< @brief timestamp for cac status changed. */
    uint8_t cac_status;               /**< @brief cac status --- ongoing, nonoccupancy, available. */
    uint8_t radar_detected;           /**< @brief radar detected for this channel. */
    struct timeval start_scan_ts;               /**< @brief Timestamp of channel scan start time */
    uint8_t scan_status;                        /**< @brief Channel scan status. */
    uint8_t utilization;                        /**< @brief Current channel utilization. */
    uint8_t avg_noise;                          /**< @brief Average radio noise. */
    uint32_t chscan_dur;                        /**< @brief Total time spent performing the scan of this channel in milliseconds*/
    uint8_t chscan_type;                        /**< @brief Scan type, 1= active scan; 0=passive scan*/
    uint8_t chscan_mode;                        /**< @brief Chscan mode = impact mode, it's bit mask */
    dlist_head neighbor_list;                   /**< @brief Neighbor list */
    uint8_t disable;
};

#define INIT_CHANNEL_INFO(_info, _channel, _disable) \
do {\
    (_info)->id = (_channel);\
    (_info)->pref = MOST_PREF_SCORE;\
    (_info)->reason = 0;\
    (_info)->freq_separation = 0;\
    (_info)->disable = (_disable);\
    (_info)->cac_method_mask = 0;\
    (_info)->scan_status = SCAN_STATUS_INIT;\
    dlist_head_init(&(_info)->neighbor_list);\
}while(0)

struct opc_chan {
    uint8_t opclass_id;
    uint8_t bw;
    uint8_t chans[MAX_CHANNEL_PER_OPCLASS];
};

struct most_pref_pair {
    dlist_item l;
    uint8_t opclass;
    uint8_t channel;
};

struct operating_class {
    uint8_t op_class;
    uint8_t max_tx_power;
    uint8_t num_support_chan;
    struct chan_info channels[MAX_CHANNEL_PER_OPCLASS];
};

struct wsc_context {
    struct TLV *m1;
    uint8_t *mac;
    uint8_t *nonce;
    uint8_t *priv_key;
    uint16_t priv_key_len;
    uint16_t auth;
    uint16_t encrypt;
};

struct band_capability {
    uint32_t ht_capa_valid:1;           /**< @brief if HT capability is valid */
    uint32_t vht_capa_valid:1;          /**< @brief if VHT capability is valid */
    uint32_t he_capa_valid:1;           /**< @brief if HE capability is valid */
    struct ht_capability  ht_capa;      /**< @brief HT capability */
    struct vht_capability vht_capa;     /**< @brief VHT capability */
    struct he_capability  he_capa;      /**< @brief HE capability */
};

struct policy_param_steer {
    uint8_t agt_steer_mode;
    uint8_t chan_util;
    uint8_t rcpi_thresh;
};

struct policy_param_metrics_rpt {
    uint8_t sta_rcpi_thresh;
    uint8_t sta_rcpi_margin;
    uint8_t ap_chutil_thresh;
#define REPORT_STA_TRAFFIC_STATS BIT(7)
#define REPORT_STA_LINK_METRICS BIT(6)
#define REPORT_STA_WIFI6_STATUS BIT(5)
    uint8_t assoc_sta_inclusion_mode;
};

struct radio_metrics {
    uint8_t noise;
    uint8_t transmit;
    uint8_t receive_self;
    uint8_t receive_other;
};

////////////////////////////////////////////////////////////////////////////////
// Link metrics
////////////////////////////////////////////////////////////////////////////////
struct tx_linkMetrics {
    uint32_t tx_packet_errors;              /**< @brief Estimated number of packets with errors transmitted from "local interface" to "neighbor interface"
                                              * in the last 'measures_window' seconds. */
    uint32_t tx_packet_ok;                  /**< @brief Estimated number of transmitted packets from "local interface" to "neighbor interface"
                                              * in the last 'measures_window' seconds. */
    uint16_t tx_max_xput;                   /**< @brief Extimated maximum MAC throughput from "local interface" to "B" in Mbits/s. */
    uint16_t tx_link_availability;          /**< @brief Estimated average percentage of time that the link is available to transmit data
                                              * from "local interface" to "neighbor interface" in the last 'measures_window' seconds. */ 
    uint16_t tx_phy_rate;                   /**< @brief Extimated PHY rate from "A" to "B" in Mbits/s. */
};

struct rx_linkMetrics {
    uint32_t rx_packet_errors;              /**< @brief Estimated number of packets with errors transmitted from "neighbor interface" to "local interface"
                                              * in the last 'measures_window' seconds. */
    uint32_t rx_packet_ok;                  /**< @brief Estimated number of transmitted packets from "neighbor interface" to "local interface"
                                              * in the last 'measures_window' seconds. */
    uint8_t rx_rssi;                        /**< @brief Estimated RSSI when receiving data from "neighbor interface" to "local interface" in dB. */
};

struct linkMetrics
{
    uint32_t measures_window;               /**< @brief Time in seconds representing how far back in time statistics have been being recorded for this interface. */
                                            /* For example, if this value is set to "5" and 'tx_packet_ok' is set to "7", it means that in the last 5 seconds 7 packets
                                             * have been transmitted OK between "local interface" and "neighbor interface".
                                             * [PLATFORM PORTING NOTE]
                                             *     This is typically the amount of time ellapsed since the interface was brought up. */
    struct tx_linkMetrics tx_link_metrics;
    struct rx_linkMetrics rx_link_metrics;
};

struct chan_scan_capability {
    uint8_t scan_bootonly;       /**< @brief scan only on boot or scan upon request.*/
    uint8_t impact_mode;         /**< @brief different impact to performance: 0=no impact, 1=spatial streams reduced, 2=time slicing impairment, 3=radio unavailable.*/
    uint32_t min_scan_interval;
};

struct vbss_capability {
    uint8_t max_vbss;             /**< @brief Maximun number of VBSSs supported by this radio.>*/
    uint8_t vbss_subtract:1;      /**< @brief 0 = Each active VBSS subtracts from the maximum number of VBSSs supported by the radio and
                                    * is independent of the maximum number of BSSs supported by the radio as specified in the AP Radio Basic
                                    * Capabilities TLV. 1 = Each active VBSS subtracts from both the maximum number of VBSSs and the maximum
                                    * number of BSSs supported by the radio as specified in the AP Radio Basic Capabilities TLV.>*/
    uint8_t vbssid_restrictions:1;/**< @brief 0 = No Match and Mask VBSSID restrictions apply
                                    * 1 = Match and Mask VBSSID restrictions apply to all non-fixed value bits (i.e., VBSSIDs must be
                                    * orthogonal to other BSSIDs/VBSSIDs under bit-wise operations).>*/
    uint8_t match_and_mask_restrictions:1; /**< @brief 0 = No Match and Mask VBSSID restrictions apply; 1 = Match and Mask VBSSID restrictions
                                             * apply to all non-fixed value bits (i.e., VBSSIDs must be orthogonal to other BSSIDs/VBSSIDs
                                             under bit-wise operations).*/
    uint8_t fixed_bits_restrictions:1; /**< @brief 0 = No fixed bits restrictions apply for the VBSSID; 1 = Fixed bits restrictions apply for
                                         * the VBSSID. Refer to Fixed Bits Mask and Fixed Bits Value fields.>*/
    mac_address fixed_bits_mask;   /**< @brief Mask of bits that must be fixed in the VBSSID that the radio can support. Note that the two least
                                    * significant bits of the first octet of this mask shall be 0s, to exclude the unicast/multicast bit and the
                                    * globally unique (OUI enforced)/locally administered bit.>*/
    mac_address fixed_bits_value; /** @brief Value of the VBSSID that must be fixed, when masked with the Fixed Bits Mask, that the radio can support.*/
};

#define CAC_STATUS_REQUIRED_CAC 0
#define CAC_STATUS_AVAIL        1
#define CAC_STATUS_NON_OCCUP    2
#define CAC_STATUS_ONGOING      3
#define CAC_STATUS_FAIL         4
#define CAC_STATUS_TERM         5
#define CAC_STATUS_NON_DFS      6
#define CAC_PERIODIC_INTERVAL 70

struct cac_request {
    mac_address ruid;
    uint8_t opclass;
    uint8_t channel;
    uint8_t method;
    uint8_t cmpl_action;
};

struct cac_termination {
    mac_address ruid;
    uint8_t opclass;
    uint8_t channel;
};

struct cur_cac_scan {
    struct cac_request cur_scan;
    uint8_t cur_status;
    uint32_t timestamp; /* mil-seconds since mesh init:
                         *  for avail-chan: fill the timestamp of finishing;
                         *  for non-occu-chan: fill the timestamp of radar detected;
                         *  for ongoing-chan: fill the timestamp of beginning. */
};

struct akm_suite_cap {
    uint8_t oui[3];
    uint8_t akm_suite_type;
};

struct radio {
    dlist_item  l;                      /**< @brief Membership of ::al_device.radios */

    uint8_t     uid[MACLEN];            /**< @brief Radio Unique Identifier for this radio. */
    char        *name;                  /**< @brief Radio's name (eg phy0) */
    uint32_t    index;                  /**< @brief Radio's index (PHY) */
    struct al_device  *d;               /**< @brief Radio belong to which device > */

    struct      operating_class opclasses[MAX_OPCLASS]; /**< @brief supported operation class */

    uint8_t     num_opc_support;        /**< @brief num of supported opclasses by this radio */
#define DEFAULT_MAX_BSS (4)
    uint8_t     max_bss;                /**< @brief Maximum number of BSSes */
    uint8_t     opclass;             	/**< @brief Current operating class  */
    uint8_t     channel;                /**< @brief Current operating channel */
    uint8_t     bw;                     /**< @brief Current operating bandwidth. 0: 20_NOHT, 1: 20, 2: 40, 3: 80, 4: 80P80, 5: 160*/
    uint8_t     current_opclass[bw_idx_max];
    uint8_t     current_channel[bw_idx_max];
    uint8_t     informed_bw;            /**< @brief Operating bw has been informed to registrar */
    uint8_t     informed_chan;          /**< @brief Operating channel has been informed to registrar */
    uint8_t     bands;        			/**< @brief Band ID, see IEEE80211_FREQUENCY_BAND_ */
    uint8_t     tx_power;               /**< @brief Tx power */
    uint8_t     ch_util;                /**< @brief Channel Utilization as measured by the radio */
    uint8_t     channel_selection_code;
#define OPERATING_CHANNEL_CHANGE_MASK BIT(0)
    uint8_t     change_mask;
    struct      dlist_head ctrl_most_pref_chs; /**< @brief opclass/channel pairs which pref=15 in chan-sel-req msg*/
    bool        chpref_refresh_needed;   /**< @brief flag to indicate the channel preference value need to be refresh */
    enum e_wifi_band_idx current_band_idx;	/**< @brief current working band: 0: 2g, 1: 5g, 2: 6g, -1: unknown> */
    struct band_capability bands_capa[band_max_idx]; /**< @brief bands capability. current_band as the array index> */
    struct chan_scan_capability scan_capa;  /** SCAN capability */
    struct vbss_capability vbss_capa;   /** VBSS capability */
    uint32_t current_vbss_num;
    uint32_t configured:1;
    uint32_t sae_capa_valid:1;
    uint32_t change_power:1;
    uint32_t change_channel:1;
    uint32_t ch_util_crossed:1;

    NARRAY(struct wifi_interface *) configured_bsses; /**< @brief BSSes configured */
    dlist_head  unassoc_stas;
    struct policy_param_steer steer_policy;         /**< @brief steering policy */
    struct policy_param_metrics_rpt metrics_rpt_policy;   /**< @brief metrics report policy */

    struct radio_metrics metrics;
    struct wsc_context *wsc;
    //struct radio_ops *ops;
    /** @brief cac scan relevant */
    struct cur_cac_scan cur_cac;      /**< @brief current cac scan, available for agent */
    void *cacscan_operate_timer;      /**< timer for cac scan action  */

    struct chscan_req *chscan_req;          /**< @brief Ongoing channel scan request. */
    struct chscan_req *chscan_req_pend;     /**< @brief Pending new scan request if another scan request is ongoing */
    struct timeval chscan_result_ts; /**<@brief Channel scan result timestamp update for radio*/
    struct cac_capability cac_capa[CAC_METHOD_MAX];
};

#define RESET_CURRENT_CHANNEL(_opclass, _channel) \
do {\
    int i;\
    for (i=0;i<bw_idx_max;i++) {\
        (_opclass)[i] = 0;\
        (_channel)[i] = 0;\
    }\
}while(0)

struct neighbor {
    dlist_item l;
    uint8_t al_mac[MACLEN];
    uint8_t intf_mac[MACLEN];
    uint8_t is_1905:1;
    uint32_t last_lldp_discovery;
    uint32_t last_1905_discovery;
    struct linkMetrics link_metric;
};

struct neighbor_bss {
    dlist_item  l;
    mac_address bssid;               /**< @brief Neighbor bssid*/
    struct ssid ssid;                /**< @brief Neighbor ssid*/
    uint8_t signal_stength;          /**< @brief RSSI of beacon or probe-response from this neighbor*/
    enum e_wifi_band_index bw;            /**< @brief bandwidth */
    uint8_t bss_load_element_present;          /**< @brief bit7=1: BSS load element present*/
    uint8_t bss_color;               /**< @brief bit0-5: BSS color*/
    uint8_t chan_utilize;            /**< @brief Channel utilization of this neighbor bss*/
    uint16_t sta_cnt;                /**< @brief Count of STAs associated with this neighbor bss*/
    uint32_t last_seen;
};

struct interface {
    dlist_item l;                       /**< @brief Membership of the al_device::interfaces */
    char *name;                   /**< @brief Radio's name (eg phy0) */
    int index;
    uint8_t     mac[MACLEN];

    enum e_interface_type type;
    enum e_power_state power_state;

    struct al_device *owner;
    struct _linux_interface_info *interface_info; /**< @brief Some additional information for receive 1905 packages. */

    uint32_t last_topology_discovery_ts;
    uint32_t last_lldp_discovery_ts;

    dlist_head neighbors;
    struct ipv6 local_ipv6;
    dlist_head ipv4s;
    dlist_head ipv6s;

    void *ops;
};

#define SSIDCPY(dst, src) \
    do {\
        (dst).len = (src).len;\
        memcpy((dst).ssid, (src).ssid, (dst).len);\
        (dst).ssid[(dst).len] = 0;\
    }while(0)

#define SSIDCMP(dst, src) (((dst).len!=(src).len ) || (memcmp((dst).ssid, (src).ssid, (dst).len)))

#define MAX_KEY_LEN 64
struct key {
    uint8_t key[MAX_KEY_LEN+2];
    uint16_t len;
};

struct vbss_client_context_info {
    mac_address client_mac;
    struct vvData password;
    uint64_t tx_packet_number;
    uint64_t group_tx_packet_number;
    struct vvData ptk;
    struct vvData gtk;
};


#define VLAN_MAX  (16)
#define VLAN_ID_INVALID (0)
struct bss_info {
    uint8_t bssid[MACLEN];              /**< BSSID (MAC address) of the BSS configured by this WSC exchange. */
    uint8_t auth;                       /**< Authentication mode. */
    uint8_t encrypt;                    /**< Encryption mode. */
    uint16_t beacon_interval;           /**< Beacon Interval */
    uint16_t fronthaul:1;               /**< Is fronthaul AP */
    uint16_t backhaul:1;                /**< Is backhaul AP */
    uint16_t hidden:1;                  /**< Is hidden SSID */
    uint16_t backhaul_sta:1;            /**< Is backhaul STA */
    struct ssid ssid;                   /**< SSID used on this BSS. */
    struct key key;                     /**< Key used on this BSS, valid only for WPA2*/
    uint16_t vlan_map[VLAN_MAX];
    struct vbss_info *vbss;             /**< TODO: VBSS info if bss is vbss>*/
    uint8_t role;
};

struct wifi_config {
    dlist_item l;
    struct bss_info bss;
    uint8_t bands;
};

struct ap_ext_metrics {
    uint32_t uc_tx;
    uint32_t uc_rx;
    uint32_t mc_tx;
    uint32_t mc_rx;
    uint32_t bc_tx;
    uint32_t bc_rx;
};

struct espi_conf {
    uint8_t enabled;
    uint8_t data_fmt;
    uint8_t ba_win;
    uint8_t airtime_fraction;
    uint8_t ppdu_duration;
};

struct bss_metrics {
    uint8_t    ch_util;
    struct     espi_conf espi[4];  /**< @brief member[0] for be, [1] for bk, [2] for vo, [3] for vi */
};

struct wifi_interface {
    struct interface i;

    enum e_wifi_role role;
    struct bss_info bssInfo;
    struct radio *radio;

    dlist_head clients;

    void *steering_timer;
    mac_address last_steering_target;

    bool is_vbss;
    struct vbss_client_context_info *vbss_client_context; /** < @brief store vbss client context info> */

    struct ap_ext_metrics ext_metrics;
    struct bss_metrics metrics;
    uint8_t assoc_allowance;
    void *ops;
    uint8_t neighbor_set:1;
    struct mark mark;
    char priv[0];                       /**< @brief pointer to private data */
};

struct chan_scan_result_item {
    dlist_item l;
    uint8_t bssid[MACLEN];
    uint8_t channel;
    uint8_t signal;
    uint32_t last_seen_ms;
    uint8_t *ies;
    uint32_t ies_len;
    uint8_t *beacon_ies;
    uint32_t beacon_ies_len;
};

struct associated_sta_link_metrics {
    uint32_t mac_rate_dl;
    uint32_t mac_rate_ul;
    uint8_t rcpi_ul;
};

struct unassociated_sta_link_metrics {
    uint8_t rcpi_ul;
};

struct associated_sta_ext_link_metrics {
    uint32_t last_data_rate_dl;
    uint32_t last_data_rate_ul;
    uint32_t util_rx;
    uint32_t util_tx;
};

struct associated_sta_traffic_stats {
    uint32_t bytes_tx;
    uint32_t bytes_rx;
    uint32_t packets_tx;
    uint32_t packets_rx;
    uint32_t packets_err_tx;
    uint32_t packets_err_rx;
    uint32_t retransmission;
};

struct sta_seen {
    dlist_item l;             /**< Membership of the seen clients */
    uint8_t rid[MACLEN];      /**< mac addr of radio the sta seen */
    uint8_t opclass;          /**< the opclass where the station is seen */
    uint8_t channel;          /**< the channel where the station is seen */
    uint8_t   rcpi_ul;
    uint32_t  rcpi_ul_ts;
};

struct btm_context {
    uint8_t token;                /**< token for indentity a pair of btm req/rsp */
    uint8_t target[MACLEN];
    uint32_t send_btm_ts;    /**< ts(ms) of last btm request if can not receive btm response in 6s then deauth the client */
    void *check_timer;  /**< if btm response status code is 0. start this timer to check if client really steer successfully >*/
    void *wait_rsp_timer;  /**< wait btm response timer */
};

struct requested_chrpt_item {
    dlist_item l;
    uint8_t opclass;
    uint8_t num_chs;
    uint8_t ch_list[MAX_CHANNEL_PER_OPCLASS];
};

struct measure_elem_item {
    dlist_item l;
    uint8_t id;
    uint8_t len;
    uint8_t *elem;
};

struct client_ies {
    uint8_t *rm_enabled;
    uint8_t *extcap;
    uint8_t *ht_cap;
    uint8_t *vht_cap;
    uint8_t *he_cap;
    uint8_t *supported_rates;
};

struct beacon_context {
    uint8_t token;     /**< Token used in beacon request frame, identity a pair of dialogs */
//  struct _platform_timer *beacon_meas_timer;
};

struct vbss_context {
    mac_address source_agent;
    mac_address target_ruid;
};

enum client_steer_status {
    CLIENT_STEER_STATUS_INIT = 0,
    CLIENT_STEER_STATUS_WAIT_SEEN = 1,
    CLIENT_STEER_STATUS_WAIT_BTM = 2,
    CLIENT_STEER_STATUS_FINISHED = 9,
};

#define client_steer_status_update(_client, _state) do { \
    DEBUG_INFO("client("MACFMT") steer status: %u --> %u\n", MACARG((_client)->mac), \
        (_client)->steer_state, _state); \
    (_client)->steer_state = _state; \
} while(0)

struct client {
    dlist_item  l;                      /**< Membership of the wifi_interface.clients */
    struct wifi_interface *wif;         /**< belong to which wifi interface >*/
    uint8_t mac[MACLEN];                /**< MAC address of the STA. */
    uint32_t    bsta:1;                 /**< Is this backhaul STA. */

    uint32_t    last_assoc_ts;          /**< Last associated timestamps */
    struct band_capability bands_capa;
    struct vvData last_assoc;           /**< The most recently received (Re)Association Request frame from this client */
    uint8_t last_capa_rpt_result;       /**< err code for the last client capability report, 0 means sucess */
    struct client_ies ies;              /**< The parsed IEs from the lasted assoc */
    struct associated_sta_link_metrics link_metrics;
    uint32_t link_metrics_ts;
    struct unassociated_sta_link_metrics unasso_link_metrics; /** < nac info > */
#define MIN_STA_TRAFFIC_STATUS_REPORT_INTERVAL_MS (10*1000)
    uint32_t last_traffic_status_report_ts;
    struct associated_sta_ext_link_metrics ext_link_metrics;
    struct associated_sta_traffic_stats traffic_stats;
    enum client_steer_status steer_state;
#define CLIENT_MAX_FAIL (6)
    uint8_t fail_cnt;  /**< continuous get stats failed times if > 6 remove the station >*/
    dlist_head seens;
    void *wait_seen_timer;             /**< wait other device metrics timer. >*/
    struct btm_context btm_ctx;
//  dlist_head measure_elem_list;
    struct beacon_context beacon_ctx;
    struct vbss_context vbss_ctx;
    uint8_t token;
};

struct p2_ec_item {
    dlist_item l;
    uint8_t reason_code;
    uint8_t mac[MACLEN];
    uint32_t rule_id;
    uint16_t qmid;
};

struct cls_device_capability {
    uint8_t vip_max;
};

struct device_info {
    char *friendly_name;
    char *manufacturer;
    char *model_name;
    int32_t start_time;
};

enum vip_conf_type {
    VIP_CONF_TYPE_QUEUE = 0,
    VIP_CONF_TYPE_DSCP,
    VIP_CONF_TYPE_TC,

    VIP_CONF_TYPE_MAX,
};

#define VIP_QUEUE_PARAMS_NUM 3
#define VIP_DSCP_PARAMS_NUM 3
#define VIP_TC_PARAMS_NUM 3
#define VIP_QOS_KEYWORDS_NUM 6
#define VIP_QUEUE_CONF_PATH "/etc/cls-qos/egress_queue.conf"
#define VIP_DSCP_CONF_PATH "/etc/cls-qos/dscp_mapping.conf"
#define VIP_TC_CONF_PATH    "/etc/cls-qos/tc_mapping.conf"
#define DEBUGFS_QUEUE_CONF_PATH "/sys/kernel/debug/cls_npe/queue_weight"
#define DEBUGFS_DSCP_CONF_PATH  "/sys/kernel/debug/cls_npe/dscp_map"
#define DEBUGFS_TC_CONF_PATH    "/sys/kernel/debug/cls_npe/tc_map"


struct al_device {
    dlist_item l;                       /**< @brief Membership of ::local_network */

    uint8_t al_mac[MACLEN];             /**< @brief 1905.1 AL MAC address */

    enum e_profile profile;             /**< @brief MAP profile, 0 for non-easymesh device */
    dlist_head interfaces;              /**< @brief The interfaces belonging to this device. */
    dlist_head radios;                  /**< @brief The radios belonging to this device. */

    uint8_t controller_weight;          /**< @brief default 50. set from cls-netmanager. 255: no weight TLV*/
    uint32_t is_agent:1;                /**< @brief true if this device is a Multi-AP Agent. */
    uint32_t is_controller:1;           /**< @brief true if this device is a Multi-AP Controller. */
    uint32_t configured:1;              /**< @brief if device configured by controller */
    uint32_t ap_basic_capaed:1;
    uint32_t inited_map_policy:1;       /**<>@brief if send inited map policy to agent>*/

#define APCAPA_NONASSOCIATE_STA_METRICS  (BIT(7))
#define APCAPA_NONASSOCIATE_STA_METRICS_DIFF_CHANNEL (BIT(6))
    uint8_t  ap_capability;

    uint32_t last_topo_resp_ts;         /**< @brief Last received topology response timestamp */
    //uint8_t recv_intf_addr[MACLEN];     /**< @brief Reachable from local interface */
    uint32_t recv_intf_idx;	            /**< @brief Receiving interface index */
    //char recv_intf_name[IFNAMSIZ];      /**< @brief Receiving interface name */

#define STATUS_CONFIGURED (1)
#define STATUS_UNCONFIGURED  (0)
    uint8_t status;
    uint32_t set_unconfigured_ts;

    uint8_t country_code[2];            /**< @brief country code, 2 characters */
    uint8_t max_vid;
    uint8_t count_units;

    uint8_t metrics_rpt_interval;       /**< @brief ap metrics reporting interval(uint is second) */
#define DISALLOW_LIST_RESET(_list) \
    do {\
        dlist_free_items((_list), struct mac_item, l);\
    } while (0)
    dlist_head local_disallow_list;     /**< @brief the sta list which local steering is disallowed */
    dlist_head btm_disallow_list;       /**< @brief the sta list which BTM steering is disallowed */

    uint32_t ts_alive;
    uint32_t ts_topo_resp;
#define VIP_CONF_CHANGED_STA_SHIFT      0
#define VIP_CONF_CHANGED_MAPPING_SHIFT  2
#define VIP_CONF_CHANGED_QUEUE_SHIFT    1
#define VIP_CONF_CHANGED_TC_SHIFT       3

    uint32_t vip_conf_changed;
    void *metrics_rpt_timer;
    struct device_info device_info;

    struct cls_device_capability cls_cap;

    void *autorole_timer;
};

struct device_vendor_info {
	char *manufacturer;
	char *model_name;
	char *model_num;
	char *serial_no;
    char *device_name;
#define UUID_LEN 16
    uint8_t uuid[UUID_LEN];
};

struct device_wifi_config {
    struct dlist_head bsses;            /**<@brief list of bss_info */
};

struct map_config {
    uint8_t auto_role:1;                    /**<@brief 1: support auto role */
    uint8_t agent:1;                   /**<@brief roles has agent */
    uint8_t controller:1;              /**<@brief roles has controller */
    uint8_t lldp:1;                    /**<@brief if send lldp discovery */
    uint8_t relay:1;
    uint8_t listen_specific_protocol:1;
    uint8_t wfa_mode:1;
    uint8_t sync_sta:1;
    uint8_t autoconf_timeout;
    uint8_t topology_timeout;
    uint8_t age_timeout;
    uint8_t collection_interval;

    enum e_profile profile;             /**<@brief Easymesh profile level */

    char *ubus_prefix;                  /**<@brief prefix name of ubus objects and unix socket to support multiple instances */

    struct device_vendor_info device_info; /**<@brief device vendor information for m2 */
    struct device_wifi_config wifi_config; /**<@brief wifi configuration information for m2 */
};

struct link_pair_item {
    dlist_item l;
    uint8_t local_intf_mac[MACLEN];
    uint8_t neighbor_intf_mac[MACLEN];
};

struct link_item {
    dlist_item l;
    uint8_t local_al_mac[MACLEN];
    uint8_t neighbor_al_mac[MACLEN];
    struct dlist_head links;
};

struct mac_item {
    dlist_item l;
    uint8_t mac[MACLEN];
};

struct steer_policy_item {
    dlist_item l;
    uint8_t rid[MACLEN];
    struct policy_param_steer params;
};

struct client_assoc_ctrl_item {
    dlist_item l;
    uint8_t req_bssid[MACLEN];
    uint8_t assoc_ctrl;
    uint16_t period;
    struct dlist_head sta_list;
};

struct backhaul_bss_conf_item {
    dlist_item l;
    uint8_t bssid[MACLEN];
    uint8_t config;
};

struct unassoc_sta_metrics_query_per_chan_item {
    dlist_item l;     /**< Membership of unassoc sta link metrics in one channel */
    uint8_t chan;     /**< channel No. */
    dlist_head sta_list; /**< list of sta's mac */
};

struct deny_sta_info {
    uint8_t al_mac[MACLEN];
    uint8_t bssid[MACLEN];
    uint8_t mac[MACLEN];
};

struct roam_policy {
    uint8_t rcpi_steer;  /* default 0 */
    uint8_t band_steer;  /* default 1 */
    int32_t rssi_low;    /* When RSSI is less than this threshold, steer module will steer sta to better connection */
    int32_t rssi_high;   /* When RSSI is greater than this threshold and band is 2G, steer module will steer sta to 5G at same device */
    uint8_t rcpi_thresh; /* when UL Rcpi < rcpi_thresh, steer module will try to steer sta to better connection */
    uint8_t rssi_gain_of_5g; /* 5g bss rssi of nac add this param */
    uint8_t rssi_gain_thresh; /* target bss rssi - current bss rssi > rssi_gain_thresh will be selected as target */
    uint32_t cooldown;   /* if switched then at least keep this time period */
};

struct metrics_rpt_policy {
    enum e_wifi_band_idx band_idx;
    struct policy_param_metrics_rpt rpt_policy;
};

#define DSCP2UP_SIZE    64

struct DSCP_mapping_item {
    dlist_item l;
    uint8_t dscp_value;  /**< @brief dscp value */
    uint8_t queue_id;    /**< @brief sw queue id */
    uint8_t tid;         /**< @brief wmm tid */
};

struct DSCP_mapping_conf {
    uint8_t dft_tid;     /**< @brief default DSCP mapping TID */
    uint8_t dft_qid;     /**< @brief default DSCP mapping queue ID */
    dlist_head dscp_list;  /**< @brief DSCP mapping list */
};

struct queue_conf_item {
    dlist_item l;
    uint8_t port_id;      /**< @brief sw queue port id */
    uint8_t queue_id;     /**< @brief sw queue queue id */
    uint8_t weight;       /**< @brief sw queue weight */
};

struct tc_mapping_item {
    dlist_item l;
    uint8_t tc_value;     /**< @brief tc value, -1 means default config */
    uint8_t tid;  /**< @brief wmm tid */
    uint8_t queue_id;    /**< @brief egress queue id */
};

struct tc_mapping_conf {
    uint8_t dft_tid;     /**< @brief default TC mapping TID */
    uint8_t dft_qid;     /**< @brief default TC mapping queue ID */
    dlist_head mapping_list;  /**< @brief mapping list */
};

struct vbss_config {
    uint8_t enable;       /** < @ if enable vbss on controller > */
    struct ssid vbss_ssid;
    uint8_t auth;
    uint8_t encrypt;
    struct key k;
};

struct vlan_config_item {
    dlist_item2 l2;
    struct ssid ssid;
    uint16_t vlan;
};

struct map_policy {
    struct roam_policy roaming_policy;
    struct metrics_rpt_policy metrics_rpt[band_max_idx];
    struct dlist_head steer;
    //struct dlist_head metrics_rpt;
    struct dlist_head stalist_local_steer_disallow;
    struct dlist_head stalist_btm_steer_disallow;
    struct dlist_head vlans;
    uint8_t metrics_rpt_intvl;
    uint16_t def_vlan;
    uint16_t def_pcp;
    uint32_t dscp2up_set:1;
    uint8_t dscp2up_table[DSCP2UP_SIZE];
    struct dlist_head backhaul_bss_configs;
    dlist_head vips;
#define APPLY_PORT_NUM 6
    dlist_head queue_conf;   /* brief sw egress queue conf for Ultra QoS */
    struct DSCP_mapping_conf dscp_conf; /* brief DSCP mapping conf for Ultra QoS */
    struct tc_mapping_conf tc_conf;    /* brief tc conf for Ultra QoS */
    struct vbss_config vbss_conf;
};

struct interface_ip_item {
    dlist_item l;
    uint8_t intf_mac[MACLEN];
    struct ipv6 local_ipv6;
    dlist_head ipv4s;
    dlist_head ipv6s;
};

struct configurator_ops {
    int (*getWifiConfig)(void *data, dlist_head *bsses);
    int (*getPolicy) (void *data, struct map_policy *policy);
    int (*getConfig) (void *data, struct map_config *config);
    int (*getDeviceInfo)(void *data, struct device_info * device_info);
    int (*getIpInfo)(void *data, dlist_head *ips);
    int (*getVlan) (void *data, struct map_policy *policy);
};

struct configurator {
    struct configurator_ops *ops;
    void *data;
};

struct frame_filter {
};

struct wifi_interface_ops {
    void (*startWPS)(struct wifi_interface *ifw);
    void (*sendFrame)(struct wifi_interface *ifw, uint8_t *frame, uint32_t frame_len);
    void (*deauthClient)(struct wifi_interface *ifw, uint8_t *sta, uint16_t reason);
    void (*registerMgmtFrame)(struct wifi_interface *ifw, struct frame_filter *filter, uint8_t dir);
    void (*addAPPIEs)(struct wifi_interface *ifw, uint32_t mask, uint8_t *ies, uint32_t len);
    void (*delAPPIEs)(struct wifi_interface *ifw, uint32_t mask, uint8_t *ies, uint32_t len);
    void (*confExtCapa)(struct wifi_interface *ifw, uint8_t *extcap, uint8_t *extcap_mask, uint32_t len);
};

struct radio_ops {
    int (*addAP)(struct radio *r, struct bss_info *bss);
    int (*removeAP)(struct radio *r, struct wifi_interface *ifw);
    int (*tearDown)(struct radio *r);
    int (*commit)(struct radio *r);

    int (*setChannelExt)(struct radio *r, uint32_t opclass, uint32_t chan, uint32_t txpower);
};

#define RADIO_ADD_AP(_ret, _radio, _bss) \
    do { \
        if ((_radio) && (_radio->ops) && (_radio->ops->addAP)) \
            (_ret) = _radio->ops->addAP(_radio, bss); \
    } while(0)

#define RADIO_COMMIT(_ret, _radio) \
    do { \
        if ((_radio) && (_radio->ops) && (_radio->ops->commit)) \
            (_ret) = _radio->ops->commit(_radio); \
    } while(0)

#define RADIO_TEARDOWN(_ret, _radio) \
    do { \
        if ((_radio) && (_radio->ops) && (_radio->ops->tearDown)) \
            (_ret) = _radio->ops->tearDown(_radio); \
    } while(0)

extern struct map_config local_config;
extern struct map_policy local_policy;
extern struct al_device *local_device;
extern struct al_device *registrar;
extern dlist_head local_network;

extern struct configurator local_configurator;

void printBSSConfig(struct bss_info *bss);
void datamodelInit(void);
void policyReset(struct map_policy *p);
void  DMalMacSet(uint8_t *al_mac_address);
uint8_t *DMalMacGet();
void updateLocalWifiConfig(struct radio *r, dlist_head *bsses);


#define IS_1905 (1)
#define IS_LLDP (0)
uint8_t dmUpdateNeighbor(struct interface *intf, uint8_t *al_mac, uint8_t *intf_mac, uint8_t is_1905);
uint8_t dmDeviceNeedUpdate(struct al_device *d);
void dmUpdateIPInfo();
struct al_device *alDeviceNew(uint8_t *mac);
void alDeviceDelete(struct al_device *d);
struct al_device *alDeviceAdd(uint8_t *mac);
void alDeviceSetConfigured(struct al_device *d, uint8_t configured);
bool staInLocalDisallowedList(uint8_t *mac);
bool staInBTMDisallowedList(uint8_t *mac);
int chanScanItemStatusTransfer(struct chscan_req_item *item);
struct chscan_req_item *chscanReqItemFind(struct radio *r, uint8_t opclass);
struct chscan_req_item *chscanReqItemNew(struct chscan_req *req, uint8_t opclass, uint8_t chnum, uint8_t *chans);
struct chscan_req *chscanReqNew(struct radio *r, uint32_t ifindex, uint8_t *from);
void chscanReqDelete(struct chscan_req *req);
struct radio *radioFind(struct al_device *d, uint8_t *mac);
struct radio *radioFindById(struct al_device *d, uint32_t id);
struct radio *radioFindByName(struct al_device *d, const char *name);
struct wifi_interface *radioFindInterface(struct radio *r, uint8_t *mac);
struct radio *radioNew(struct al_device *d, uint8_t *mac);
struct radio *radioAdd(struct al_device *d, uint8_t *mac);
void radioDelete(struct radio *r) ;
void alDeviceAddInterface(struct al_device *d, struct interface *intf);
struct interface *interfaceNew(struct al_device *d, uint8_t *mac, size_t size);
void interfaceDelete(struct interface *intf);
struct wifi_interface *wifiInterfaceNew(struct al_device *d, uint8_t *mac);
struct vbss_client_context_info *wifiInterfaceAddVbssClientContext(struct wifi_interface *wif,
        uint8_t *sta_mac, uint64_t tx_packet_number, uint64_t group_tx_packet_number,
        uint8_t *ptk, uint16_t ptk_len, uint8_t *gtk, uint16_t gtk_len);
struct wifi_interface *wifiInterfaceAdd(struct al_device *d, struct radio *r,  uint8_t *mac);
struct wifi_interface *peerInterfaceFind(uint8_t *mac);
struct interface *interfaceFind(struct al_device *d, uint8_t *mac,
                                enum e_interface_type type);
#define interfaceFindMAC(_dev, _mac) interfaceFind((_dev), (_mac), interface_type_unknown)
struct interface *interfaceFindName(struct al_device *d, char *name);
#define wifiInterfaceDelete(_ifw) interfaceDelete((struct interface *)(_ifw))
struct client *clientFind(struct al_device *d, struct wifi_interface *ifw, uint8_t *mac);
struct client *clientAdd(struct wifi_interface *ifw, uint8_t *mac);
struct client *clientNew(struct wifi_interface *ifw, uint8_t *mac);
int clientMove(struct client *c, struct wifi_interface *dst);
void clientDelete(struct client *c);
struct sta_seen *seenFind(struct client *c, uint8_t *rid);
struct sta_seen *seenNew(struct client *c, uint8_t *rid);
struct sta_seen *seenAdd(struct client *c, uint8_t *rid);
struct unassoc_sta_metrics_query_per_chan_item *unassocStaChanItemAdd(struct dlist_head *head,
        uint8_t channel);
void unassocStaChanItemDelete(struct unassoc_sta_metrics_query_per_chan_item *item);
struct al_device *alDeviceFindBySta(struct client *sta);
struct al_device *alDeviceFind(uint8_t *mac);
struct al_device *alDeviceFindAny(uint8_t *mac);
struct interface *interfaceFindIdx(int idx);
struct neighbor *neighborAdd(struct interface *i, uint8_t *al_mac, uint8_t *mac);
struct neighbor *neighborFind(struct interface *i, uint8_t *al_mac, uint8_t *mac);
void neighborDelete(struct neighbor *n);
int neighborDeleteRelated(uint8_t *remote, uint8_t *neighbor);
int configuratorAdd(struct configurator_ops *ops, void *data);
int configuratorGetWifiConfig(dlist_head *bsses);
int configuratorGetPolicy(struct map_policy *policy);
int configuratorGetConfig(struct map_config *config);
int configuratorGetVlan(struct map_policy *policy);
int configuratorGetDeviceInfo(struct device_info *info);
int configuratorGetIpInfo(dlist_head *ips);
int isRegistrar();
int setRegistrar(struct al_device *d);
void radioApplyBsses(struct radio *r, dlist_head *pbsses, struct bss_info *backhaul_bss, uint8_t sync_sta);
char *idx2InterfaceName(uint32_t idx);
uint8_t *interfaceName2MAC(char *name);
struct operating_class *opclassFind(struct radio *r, uint8_t opclass);
struct operating_class *opclassAdd(struct radio *r, uint8_t opclass);
struct chan_info *channelFind(struct operating_class *opclass, uint8_t channel);
enum e_wifi_band_index bandwidthStrToIndex(char *bwstr);
char *bandwidth2String(uint8_t bw);
int bandwidthToIndex(uint8_t band);
int initOperatingClass(struct operating_class *opclass, uint8_t disable);
int resetOperatingClass(struct operating_class *opclass, uint8_t pref, uint8_t reason);
int validateOperatingChannel(struct radio *r, uint8_t opclass, uint8_t channel);
int dmSweep(void);
struct client * check_beacon_request_params(uint8_t *bssid, uint8_t *sta);
//uint16_t check_and_fill_meas_elem(uint8_t *elem, struct client *sta);
int parseChanScanResults(struct radio *r, struct chscan_req_item *req_item,
            dlist_head *head, bool first_flag);
uint8_t parseAssocFrame(struct client *client, uint8_t offset);
uint8_t checkBeaconReportSupported(uint8_t *rm_enabled);
uint8_t checkBtmSupported(uint8_t *extcap);
int updateRadioMetrics(struct radio *r, uint8_t cur_util);
struct wifi_interface *findLocalOtherRadioBss(struct client *c);
struct wifi_interface *findSpecificBandBss(struct client *c, enum e_wifi_band_idx band_idx);
uint8_t checkChanSelRequestValid(struct operating_class *opc, uint8_t *chans);
int selectBestChannel(struct radio *r, uint8_t *opclass, uint8_t *channel);
int getPeriodForChannel(uint8_t channel, uint8_t opclass, struct radio *r, uint32_t *period_cac, uint32_t *unoccup);
int updateOperatingChannelReport(struct radio *r);
int generateNewBssid(struct al_device *d, struct radio *r, uint8_t *new_bssid);
void neighborAgeHandle(struct al_device *d);
void denyStaTimerHandle(void *data);
struct policy_param_metrics_rpt *findReportPolicy(uint8_t band);
void controllerWeightReset(uint8_t weight);


#define UP_CROSS    (1)
#define DOWN_CROSS  (2)
#define DEFAULT_RPT_RCPI_HYSTERSIS 3

#define CROSSED_THRESHOLD(_threshold, _history, _current) \
((((_history) <= (_threshold)) && ((_current) > (_threshold))) ? UP_CROSS : \
    ((((_history) >= (_threshold)) && ((_current) < (_threshold))) ? DOWN_CROSS : 0))

#define CROSSED_DOWN_THRESHOLD_MARGIN(_threshold, _hysteresis, _history, _current) \
    (((_history >= _threshold+_hysteresis) && (_current <= _threshold-_hysteresis))) || \
    ((_current <= _threshold-_hysteresis) && (_history <= _threshold-_hysteresis))

#define BSS_IS_BACKHAUL(bssInfo)   (bssInfo.backhaul == 1 || bssInfo.backhaul_sta == 1)
#define STA_IS_BACKHAUL(sta)       (sta->wif && BSS_IS_BACKHAUL(sta->wif->bssInfo))

#endif
