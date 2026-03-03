#ifndef _WIFI_H_
#define _WIFI_H_
#include <linux/nl80211.h>
#include "datamodel.h"
#define IEEE80211_FC0_VERSION_0         0x00

#define IEEE80211_FC0_TYPE_MASK         0x0c
#define IEEE80211_FC0_TYPE_MGT          0x00
#define IEEE80211_FC0_TYPE_CTL          0x04
#define IEEE80211_FC0_TYPE_DATA         0x08

#define IEEE80211_FC0_SUBTYPE_MASK          0xf0
#define IEEE80211_FC0_SUBTYPE_ASSOC_REQ     0x00
#define IEEE80211_FC0_SUBTYPE_ASSOC_RESP    0x10
#define IEEE80211_FC0_SUBTYPE_REASSOC_REQ   0x20
#define IEEE80211_FC0_SUBTYPE_REASSOC_RESP  0x30
#define IEEE80211_FC0_SUBTYPE_PROBE_REQ     0x40
#define IEEE80211_FC0_SUBTYPE_PROBE_RESP    0x50
#define IEEE80211_FC0_SUBTYPE_BEACON        0x80
#define IEEE80211_FC0_SUBTYPE_DISASSOC      0xa0
#define IEEE80211_FC0_SUBTYPE_AUTH          0xb0
#define IEEE80211_FC0_SUBTYPE_DEAUTH        0xc0
#define IEEE80211_FC0_SUBTYPE_ACTION        0xd0

#define IEEE80211_FC1_DIR_MASK          0x03
#define IEEE80211_FC1_DIR_NODS          0x00    /* STA->STA */
#define IEEE80211_FC1_DIR_TODS          0x01    /* STA->AP  */
#define IEEE80211_FC1_DIR_FROMDS        0x02    /* AP ->STA */
#define IEEE80211_FC1_DIR_DSTODS        0x03    /* AP ->AP  */

/* from ieee80211.h */
#define DBM_TO_MBM(gain) ((gain) * 100)
#define MBM_TO_DBM(gain) ((gain) / 100)


#define IEEE80211_FC0(_type, _sub) ((_type) | (_sub))
#define IEEE80211_FC0_GET_TYPE(_fc) ((_fc) | IEEE80211_FC0_TYPE_MASK)
#define IEEE80211_FC0_GET_STYPE(_fc) ((_fc) | IEEE80211_FC0_STYPE_MASK)


/*
* Management information element payloads.
*/
enum {
    IEEE80211_ELEMID_SSID                     = 0,
    IEEE80211_ELEMID_SUPPORTED_RATES          = 1,
    IEEE80211_ELEMID_REPORTING_DETAIL         = 2,
    IEEE80211_ELEMID_REQUEST_ELEM             = 10,
    IEEE80211_ELEMID_BSS_LOAD                 = 11,
    IEEE80211_ELEMID_MEASREQ                  = 38,
    IEEE80211_ELEMID_HTCAP                    = 45,
    IEEE80211_ELEMID_AP_CHANNEL_REPORT        = 51,
    IEEE80211_ELEMID_NEIGHBOR_REPORT          = 52,
    IEEE80211_ELEMID_HTINFO                   = 61,
    IEEE80211_ELEMID_RM_ENABLED               = 70,
    IEEE80211_ELEMID_ADVERTISEMENT            = 108,
    IEEE80211_ELEMID_MESHID                   = 114,
    IEEE80211_ELEMID_EXTCAP                   = 127,
    IEEE80211_ELEMID_VHTCAP                   = 191,
    IEEE80211_ELEMID_VHTOP                    = 192,
    IEEE80211_ELEMID_VENDOR                   = 221,  /* vendor private */
    IEEE80211_ELEMID_EXT                      = 255,  /* Elements using the Element ID Extension field */
};

/*
* Element ID Extension
*/
enum {
    IEEE80211_ELEMID_EXT_HECAP      = 35,
    IEEE80211_ELEMID_EXT_HEOP       = 36,
};

/* Status codes from ieee80211.h*/
enum ieee80211_statuscode {
    WLAN_STATUS_SUCCESS = 0,
    WLAN_STATUS_UNSPECIFIED_FAILURE = 1,
    WLAN_STATUS_CAPS_UNSUPPORTED = 10,
    WLAN_STATUS_REASSOC_NO_ASSOC = 11,
    WLAN_STATUS_ASSOC_DENIED_UNSPEC = 12,
    WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG = 13,
    WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION = 14,
    WLAN_STATUS_CHALLENGE_FAIL = 15,
    WLAN_STATUS_AUTH_TIMEOUT = 16,
    WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA = 17,
    WLAN_STATUS_ASSOC_DENIED_RATES = 18,
    /* 802.11b */
    WLAN_STATUS_ASSOC_DENIED_NOSHORTPREAMBLE = 19,
    WLAN_STATUS_ASSOC_DENIED_NOPBCC = 20,
    WLAN_STATUS_ASSOC_DENIED_NOAGILITY = 21,
    /* 802.11h */
    WLAN_STATUS_ASSOC_DENIED_NOSPECTRUM = 22,
    WLAN_STATUS_ASSOC_REJECTED_BAD_POWER = 23,
    WLAN_STATUS_ASSOC_REJECTED_BAD_SUPP_CHAN = 24,
    /* 802.11g */
    WLAN_STATUS_ASSOC_DENIED_NOSHORTTIME = 25,
    WLAN_STATUS_ASSOC_DENIED_NODSSSOFDM = 26,
    /* 802.11w */
    WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY = 30,
    WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION = 31,
    /* 802.11i */
    WLAN_STATUS_INVALID_IE = 40,
    WLAN_STATUS_INVALID_GROUP_CIPHER = 41,
    WLAN_STATUS_INVALID_PAIRWISE_CIPHER = 42,
    WLAN_STATUS_INVALID_AKMP = 43,
    WLAN_STATUS_UNSUPP_RSN_VERSION = 44,
    WLAN_STATUS_INVALID_RSN_IE_CAP = 45,
    WLAN_STATUS_CIPHER_SUITE_REJECTED = 46,
    /* 802.11e */
    WLAN_STATUS_UNSPECIFIED_QOS = 32,
    WLAN_STATUS_ASSOC_DENIED_NOBANDWIDTH = 33,
    WLAN_STATUS_ASSOC_DENIED_LOWACK = 34,
    WLAN_STATUS_ASSOC_DENIED_UNSUPP_QOS = 35,
    WLAN_STATUS_REQUEST_DECLINED = 37,
    WLAN_STATUS_INVALID_QOS_PARAM = 38,
    WLAN_STATUS_CHANGE_TSPEC = 39,
    WLAN_STATUS_WAIT_TS_DELAY = 47,
    WLAN_STATUS_NO_DIRECT_LINK = 48,
    WLAN_STATUS_STA_NOT_PRESENT = 49,
    WLAN_STATUS_STA_NOT_QSTA = 50,
    /* 802.11s */
    WLAN_STATUS_ANTI_CLOG_REQUIRED = 76,
    WLAN_STATUS_FCG_NOT_SUPP = 78,
    WLAN_STATUS_STA_NO_TBTT = 78,
    /* 802.11ad */
    WLAN_STATUS_REJECTED_WITH_SUGGESTED_CHANGES = 39,
    WLAN_STATUS_REJECTED_FOR_DELAY_PERIOD = 47,
    WLAN_STATUS_REJECT_WITH_SCHEDULE = 83,
    WLAN_STATUS_PENDING_ADMITTING_FST_SESSION = 86,
    WLAN_STATUS_PERFORMING_FST_NOW = 87,
    WLAN_STATUS_PENDING_GAP_IN_BA_WINDOW = 88,
    WLAN_STATUS_REJECT_U_PID_SETTING = 89,
    WLAN_STATUS_REJECT_DSE_BAND = 96,
    WLAN_STATUS_DENIED_WITH_SUGGESTED_BAND_AND_CHANNEL = 99,
    WLAN_STATUS_DENIED_DUE_TO_SPECTRUM_MANAGEMENT = 103,
    /* 802.11ai */
    WLAN_STATUS_FILS_AUTHENTICATION_FAILURE = 108,
    WLAN_STATUS_UNKNOWN_AUTHENTICATION_SERVER = 109,
    WLAN_STATUS_SAE_HASH_TO_ELEMENT = 126,
    WLAN_STATUS_SAE_PK = 127,
};

/* Reason codes from ieee80211.h */
enum ieee80211_reasoncode {
    WLAN_REASON_UNSPECIFIED = 1,
    WLAN_REASON_PREV_AUTH_NOT_VALID = 2,
    WLAN_REASON_DEAUTH_LEAVING = 3,
    WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY = 4,
    WLAN_REASON_DISASSOC_AP_BUSY = 5,
    WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA = 6,
    WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA = 7,
    WLAN_REASON_DISASSOC_STA_HAS_LEFT = 8,
    WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH = 9,
    /* 802.11h */
    WLAN_REASON_DISASSOC_BAD_POWER = 10,
    WLAN_REASON_DISASSOC_BAD_SUPP_CHAN = 11,
    /* 802.11i */
    WLAN_REASON_BSS_TRANSITION_DISASSOC = 12,  /* not exist in ieee80211.h defined in 802.11-2020 */
    WLAN_REASON_INVALID_IE = 13,
    WLAN_REASON_MIC_FAILURE = 14,
    WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT = 15,
    WLAN_REASON_GROUP_KEY_HANDSHAKE_TIMEOUT = 16,
    WLAN_REASON_IE_DIFFERENT = 17,
    WLAN_REASON_INVALID_GROUP_CIPHER = 18,
    WLAN_REASON_INVALID_PAIRWISE_CIPHER = 19,
    WLAN_REASON_INVALID_AKMP = 20,
    WLAN_REASON_UNSUPP_RSN_VERSION = 21,
    WLAN_REASON_INVALID_RSN_IE_CAP = 22,
    WLAN_REASON_IEEE8021X_FAILED = 23,
    WLAN_REASON_CIPHER_SUITE_REJECTED = 24,
    /* TDLS (802.11z) */
    WLAN_REASON_TDLS_TEARDOWN_UNREACHABLE = 25,
    WLAN_REASON_TDLS_TEARDOWN_UNSPECIFIED = 26,
    /* 802.11e */
    WLAN_REASON_DISASSOC_UNSPECIFIED_QOS = 32,
    WLAN_REASON_DISASSOC_QAP_NO_BANDWIDTH = 33,
    WLAN_REASON_DISASSOC_LOW_ACK = 34,
    WLAN_REASON_DISASSOC_QAP_EXCEED_TXOP = 35,
    WLAN_REASON_QSTA_LEAVE_QBSS = 36,
    WLAN_REASON_QSTA_NOT_USE = 37,
    WLAN_REASON_QSTA_REQUIRE_SETUP = 38,
    WLAN_REASON_QSTA_TIMEOUT = 39,
    WLAN_REASON_QSTA_CIPHER_NOT_SUPP = 45,
    /* 802.11s */
    WLAN_REASON_MESH_PEER_CANCELED = 52,
    WLAN_REASON_MESH_MAX_PEERS = 53,
    WLAN_REASON_MESH_CONFIG = 54,
    WLAN_REASON_MESH_CLOSE = 55,
    WLAN_REASON_MESH_MAX_RETRIES = 56,
    WLAN_REASON_MESH_CONFIRM_TIMEOUT = 57,
    WLAN_REASON_MESH_INVALID_GTK = 58,
    WLAN_REASON_MESH_INCONSISTENT_PARAM = 59,
    WLAN_REASON_MESH_INVALID_SECURITY = 60,
    WLAN_REASON_MESH_PATH_ERROR = 61,
    WLAN_REASON_MESH_PATH_NOFORWARD = 62,
    WLAN_REASON_MESH_PATH_DEST_UNREACHABLE = 63,
    WLAN_REASON_MAC_EXISTS_IN_MBSS = 64,
    WLAN_REASON_MESH_CHAN_REGULATORY = 65,
    WLAN_REASON_MESH_CHAN = 66,
};


enum ieee80211_action_category {
    WLAN_CATEGORY_RADIO_MEASUREMENT = 5,
    WLAN_CATEGORY_WNM = 10,   /* wireless network management */
};

enum ieee80211_wnm_actioncode {
    WLAN_ACTION_BTM_REQ = 7,
    WLAN_ACTION_BTM_RSP = 8
};

enum ieee80211_radio_measure_actioncode{
    WLAN_ACTION_RADIO_MEASURE_REQ = 0,
    WLAN_ACTION_RADIO_MEASURE_RSP = 1
};


struct opclass_channel_table {
    uint8_t opclass;
    enum nl80211_channel_type type;
    enum nl80211_chan_width channelWidth;
    uint8_t chan_set[MAX_CHANNEL_PER_OPCLASS];
};

#define IEEE80211_FRAME_TYPE(_f)        ((_f)->fc[0] & IEEE80211_FC0_TYPE_MASK)
#define IEEE80211_FRAME_SUBTYPE(_f)     ((_f)->fc[0] & IEEE80211_FC0_SUBTYPE_MASK)

#define MAX_LEN_MMPDU 2304

struct ieee80211_header {
    uint8_t fc[2];
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq;
};

struct ieee80211_mgmt {
    uint8_t fc[2];
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq;
    union {
        struct {
            uint8_t category;
            uint8_t action_code;
            union {
                struct {
                    uint8_t token;
                    uint16_t repetition;
                    uint8_t variable[0];
                } __attribute__ ((packed)) radio_meas_req;
                struct {
                    uint8_t token;
                    uint8_t variable[0];
                } __attribute__ ((packed)) radio_meas_report;
                struct {
                    uint8_t token;
                    uint8_t req_mode;
                    uint16_t disassoc_timer;
                    uint8_t validity_interval;
                    uint8_t variable[0];
                } __attribute__ ((packed)) btm_req;
                struct {
                    uint8_t token;
#define BTM_STATUS_CODE_SUCCESS (0)
                    uint8_t status;
                    uint8_t termination_delay;
                    uint8_t variable[0];
                } __attribute__ ((packed)) btm_resp;
            } u;
        } __attribute__ ((packed)) action;
        struct {
            uint16_t reason_code;
            uint8_t variable[0];
        }__attribute__ ((packed)) deauth;
    } u;

} __attribute__ ((packed));

#define FILL_80211_HEADER(_mgmt, _fc0, _da, _sa, _bssid) \
do {\
    (_mgmt)->fc[0] = (_fc0);\
    MACCPY((_mgmt)->da, (_da));\
    MACCPY((_mgmt)->sa, (_sa));\
    MACCPY((_mgmt)->bssid, (_bssid));\
}while(0)

#define FILL_ACTION_HEADER(_mgmt, _category, _action) \
do {\
    (_mgmt)->u.action.category = (_category);\
    (_mgmt)->u.action.action_code = (_action);\
}while(0)

#define MODE_CANDI_INCLUDE_SHIFT 0
#define MODE_ABRIDGED_SHIFT 1
#define MODE_DISASSOC_IMM_SHIFT  2
#define MODE_BSS_TERM_INCLUDE_SHIFT  3

#define BSSID_INFO_REACHABILITY_SHIFT 0
#define BSSID_INFO_REACHABILITY_MASK  0x02
#define BSSID_INFO_HT_SUPPORT_SHIFT 11
#define BSSID_INFO_HT_SUPPORT_MASK  0x01
#define BSSID_INFO_VHT_SUPPORT_SHIFT 12
#define BSSID_INFO_VHT_SUPPORT_MASK  0x01

enum ieee80211_measure_type {
    WLAN_MEASURE_TYPE_BEACON = 5,
};

enum ieee80211_beacon_request_subelementid {
    IEEE80211_BEACONREQ_SUBELEMID_SSID = 0,
    IEEE80211_BEACONREQ_SUBELEMID_DETAIL = 2,
    IEEE80211_BEACONREQ_SUBELEMID_REQUEST = 10,
    IEEE80211_BEACONREQ_SUBELEMID_AP_CHANNEL_RPT = 51,
};

struct ieee80211_measure_req_report_element {
    uint8_t eid;
    uint8_t len;
    uint8_t token;
    uint8_t mode;
    uint8_t type;
    union {
        struct {
            uint8_t opclass;
            uint8_t channel;
            uint16_t random_interval;
#define BEACON_REQUEST_DEFAULT_DURATION (10)
            uint16_t duration;
            uint8_t mode;
            uint8_t bssid[MACLEN];
            uint8_t subelements[0];
        } __attribute__ ((packed)) beacon_req;
        struct {
            uint8_t opclass;
            uint8_t channel;
            uint64_t start_time;
            uint16_t duration;
            uint8_t reported_frame_info;
            uint8_t rcpi;
            uint8_t rsni;
            uint8_t bssid[MACLEN];
            uint8_t antenna;
            uint32_t parent_tsf;
            uint8_t subelements[0];
        } __attribute__ ((packed)) beacon_report;
    }u;
} __attribute__ ((packed));

/* Neighbor Report element: BSSID Information Field */
#define NEI_REP_BSSID_INFO_AP_NOT_REACH BIT(0)
#define NEI_REP_BSSID_INFO_AP_UNKNOWN_REACH BIT(1)
#define NEI_REP_BSSID_INFO_AP_REACHABLE (BIT(0) | BIT(1))
#define NEI_REP_BSSID_INFO_SECURITY BIT(2)
#define NEI_REP_BSSID_INFO_KEY_SCOPE BIT(3)
#define NEI_REP_BSSID_INFO_SPECTRUM_MGMT BIT(4)
#define NEI_REP_BSSID_INFO_QOS BIT(5)
#define NEI_REP_BSSID_INFO_APSD BIT(6)
#define NEI_REP_BSSID_INFO_RM BIT(7)
#define NEI_REP_BSSID_INFO_DELAYED_BA BIT(8)
#define NEI_REP_BSSID_INFO_IMM_BA BIT(9)
#define NEI_REP_BSSID_INFO_MOBILITY_DOMAIN BIT(10)
#define NEI_REP_BSSID_INFO_HT BIT(11)
#define NEI_REP_BSSID_INFO_VHT BIT(12)
#define NEI_REP_BSSID_INFO_FTM BIT(13)
#define NEI_REP_BSSID_INFO_HE BIT(14)

/* IEEE Std 802.11 - dot11PHYType */
enum phy_type {
    PHY_TYPE_UNSPECIFIED = 0,
    PHY_TYPE_FHSS = 1,
    PHY_TYPE_DSSS = 2,
    PHY_TYPE_IRBASEBAND = 3,
    PHY_TYPE_OFDM = 4,
    PHY_TYPE_HRDSSS = 5,
    PHY_TYPE_ERP = 6,
    PHY_TYPE_HT = 7,
    PHY_TYPE_DMG = 8,
    PHY_TYPE_VHT = 9,
    PHY_TYPE_HE = 14,
};

struct ieee80211_neighbor_report_element {
    uint8_t eid;
    uint8_t len;
    uint8_t bssid[6];
    uint32_t bssid_info;
    uint8_t opclass;
    uint8_t channel;
    uint8_t phy_type;
    uint8_t subelements[0];
}__attribute__ ((packed));

#define SUBELEMENT_ID_BTM_CANDI_PREF 3

#define MEASURE_MODE_PASSIVE 0
#define MEASURE_MODE_ACTIVE 1
#define MEASURE_MODE_BEACON_TABLE 2


/*
* 802.11n HT Capability IE
*/
struct ieee80211_ie_htcap {
    uint8_t id;                  /* element ID */
    uint8_t len;                 /* length in bytes */
    uint8_t cap[2];              /* HT capabilities */
    uint8_t ampdu;               /* A-MPDU parameters */
#define IEEE80211_HTCAP_MCS_LEN 16
    uint8_t mcsset[IEEE80211_HTCAP_MCS_LEN];          /* supported MCS set */
    uint8_t extcap[2];           /* extended HT capabilities */
    uint8_t txbf[4];             /* txbf capabilities */
    uint8_t antenna;             /* antenna capabilities */
} __attribute__ ((packed));
#define IEEE80211_HTCAP_LEN        (sizeof(struct ieee80211_ie_htcap))

#define IEEE80211_HTCAP_C_CHWIDTH40 0x02
#define IEEE80211_HTCAP_C_SHORTGI20 0x20
#define IEEE80211_HTCAP_C_SHORTGI40 0x40

/* MCS Tx MCS Set Defined, Bit0 in set 12(Bit 96) */
#define IEEE80211_HTCAP_MCS_TX_DEFINED(ht_mcsset) ((ht_mcsset[12] & 0x1))
/* MCS Tx Rx MCS Set Not Equal, Bit1 in set 12(Bit 97) */
#define IEEE80211_HTCAP_MCS_TXRX_NOT_EQUAL(ht_mcsset) ((ht_mcsset[12] & 0x2) >> 1)
/* MCS maximum spatial streams, B2-B3 in set 12 (Bit 98-99) */
#define IEEE80211_HTCAP_MCS_STREAMS(ht_mcsset) ((ht_mcsset[12] & 0xC) >> 2)
/* MCS set value (all bits) */
#define IEEE80211_HTCAP_MCS_VALUE(ht_mcsset, _set) (ht_mcsset[_set])

enum {
    IEEE80211_HT_MCSSET_20_40_NSS1,     /* CBW = 20/40 MHz, Nss = 1, Nes = 1, EQM/ No EQM */
    IEEE80211_HT_MCSSET_20_40_NSS2,     /* CBW = 20/40 MHz, Nss = 2, Nes = 1, EQM */
    IEEE80211_HT_MCSSET_20_40_NSS3,     /* CBW = 20/40 MHz, Nss = 3, Nes = 1, EQM */
    IEEE80211_HT_MCSSET_20_40_NSS4,     /* CBW = 20/40 MHz, Nss = 4, Nes = 1, EQM */
    IEEE80211_HT_MCSSET_20_40_UEQM1,    /* MCS 32 and UEQM MCSs 33 - 39 */
    IEEE80211_HT_MCSSET_20_40_UEQM2,    /* UEQM MCSs 40 - 47 */
    IEEE80211_HT_MCSSET_20_40_UEQM3,    /* UEQM MCSs 48 - 55 */
    IEEE80211_HT_MCSSET_20_40_UEQM4,    /* UEQM MCSs 56 - 63 */
    IEEE80211_HT_MCSSET_20_40_UEQM5,    /* UEQM MCSs 64 - 71 */
    IEEE80211_HT_MCSSET_20_40_UEQM6,    /* UEQM MCSs 72 - 76 plus 3 reserved bits */
};

/*
* 802.11n HT Information IE
*/
struct ieee80211_ie_htinfo {
    uint8_t id;            /* element ID */
    uint8_t len;            /* length in bytes */
    uint8_t ctrlchannel;        /* control channel */
    uint8_t bytes[5];        /* ht ie 5 bytes */
    uint8_t basicmcsset[IEEE80211_HTCAP_MCS_LEN];    /* basic MCS set */
} __attribute__ ((packed));
#define IEEE80211_HTINFO_LEN        (sizeof(struct ieee80211_ie_htinfo))

#define IEEE80211_HTINFO_CHOFF_SCN            0
#define IEEE80211_HTINFO_CHOFF_SCA            1
#define IEEE80211_HTINFO_CHOFF_SCB            3

#define IEEE80211_HTINFO_B1_SEC_CHAN_OFFSET        0x03
#define IEEE80211_HTINFO_B1_REC_TXCHWIDTH_40        0x04
#define IEEE80211_HTINFO_B1_REC_TXCHWIDTH_40_SHIFT    2

/* B0-B1, byte 1 */
#define IEEE80211_HTINFO_EXT_CHOFFSET(htie) (((htie)->bytes[0] & 0x3))

/*
* 802.11ac VHT Capabilities element
*/
struct ieee80211_ie_vhtcap {
    uint8_t id;            /* element ID */
    uint8_t len;           /* length in bytes */
    uint8_t cap[4];        /* VHT capabilities info */
#define IEEE80211_VHTCAP_MCS_LEN    8
    uint8_t mcs_nss_set[IEEE80211_VHTCAP_MCS_LEN];    /* supported MSC and NSS set */
} __attribute__ ((packed));
#define IEEE80211_VHTCAP_LEN        (sizeof(struct ieee80211_ie_vhtcap))

#define IEEE80211_VHTCAP_C_CHWIDTH 0x0C

/* B2-3 Supported channel width */
#define IEEE80211_VHTCAP_GET_CHANWIDTH(vht_capa) ((vht_capa[0] & 0x0C) >> 2)

/* Supported Channel Width Set B2-3 */
enum ieee80211_vhtcap_chanwidth {
    IEEE80211_VHTCAP_CW_80M_ONLY = 0,
    IEEE80211_VHTCAP_CW_160M,
    IEEE80211_VHTCAP_CW_160_AND_80P80M,
    IEEE80211_VHTCAP_CW_RESERVED,
};

/* B5 Short GI for 80MHz support */
#define IEEE80211_VHTCAP_GET_SGI_80MHZ(vht_capa) \
    ((vht_capa[0] & 0x20) >> 5)

/* B6 Short GI for 160MHz support */
#define IEEE80211_VHTCAP_GET_SGI_160MHZ(vht_capa) \
    ((vht_capa[0] & 0x40) >> 6)

/* B11 SU Beamformer Capable */
#define IEEE80211_VHTCAP_GET_SU_BEAM_FORMER(vht_capa) \
    ((vht_capa[1] & 0x08) >> 3)

/* B19 MU Beamformer Capable */
#define IEEE80211_VHTCAP_GET_MU_BEAM_FORMER(vht_capa) \
    ((vht_capa[2] & 0x08) >> 3)

/* B30-B31 Extended NSS BW Support */
#define IEEE80211_VHTCAP_GET_EXTENDED_NSS_BW_SUPPORT(vht_capa)    \
    ((vht_capa[3] & 0xc0) >> 6)

/* B0-B15 RX VHT-MCS MAP for Spatial streams 1-8 */
#define IEEE80211_VHTCAP_GET_RX_MCS_NSS(mcs) \
    ((mcs[1] << 8) | (mcs[0]))

/* B32-B47 TX VHT-MCS MAP for Spatial streams 1-8 */
#define IEEE80211_VHTCAP_GET_TX_MCS_NSS(mcs) \
    ((mcs[5] << 8) | (mcs[4]))

#define IEEE80211_VHT_HE_MCSMAP_MASK    0xC000

/*
* 802.11ac VHT Operation element
*/
struct ieee80211_ie_vhtop {
    uint8_t id;        /* element ID */
    uint8_t len;       /* length in bytes */
    uint8_t info[3];   /* VHT Operation info */
    uint8_t bvhtmcs[2];/* basic VHT MSC and NSS set */
} __attribute__ ((packed));
#define IEEE80211_VHTOP_LEN        (sizeof(struct ieee80211_ie_vhtop))

/* VHT Operation Information subfields */
enum ieee80211_vhtop_chanwidth {
    IEEE80211_VHTOP_CHAN_WIDTH_20_40MHZ,
    IEEE80211_VHTOP_CHAN_WIDTH_80MHZ,
    IEEE80211_VHTOP_CHAN_WIDTH_160MHZ,
    IEEE80211_VHTOP_CHAN_WIDTH_80PLUS80MHZ,
};

/* Channel Center Frequency Segment 0 */
#define IEEE80211_VHTOP_GET_CENTERFREQ0(vhtop) \
    (vhtop)->info[1]

/* Channel Center Frequency Segment 1 */
#define IEEE80211_VHTOP_GET_CENTERFREQ1(vhtop) \
    (vhtop)->info[2]

/*
* 802.11ax HE Capabilities element
*/
struct ieee80211_ie_hecap {
    uint8_t id;            /* Element ID */
    uint8_t len;            /* Length in bytes */
    uint8_t id_ext;            /* Element ID extension */
    uint8_t mac_cap[6];        /* HE MAC capabilities info */
    uint8_t phy_cap[11];        /* HE PHY capabilities info */
    uint8_t mcs_map_le80[4];    /* TX RX HE-MCS Map <=80MHz */
//      uint8_t mcs_map_160[4];        /* TX RX HE-MCS Map 160MHz(optional) */
//      uint8_t mcs_map_80p80[4];    /* TX RX HE-MCS Map 80+80MHz(optional) */
//      uint8_t ppe_thres[0];        /* PPE Thresholds(optional)(variable) */
} __attribute__ ((packed));
#define IEEE80211_HECAP_MINLEN        (sizeof(struct ieee80211_ie_hecap))
#define IEEE80211_HECAP_MAXLEN        (sizeof(struct ieee80211_ie_hecap) + 4 + 4 + (7 + 4 * 6 * 8 + 7) / 8)

#define IEEE80211_HECAP_PHY_CHAN_WIDTH_2P4G_40MHZ       (1 << 0)
#define IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_40P80MHZ      (1 << 1)
#define IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_160MHZ        (1 << 2)
#define IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_80P80MHZ      (1 << 3)
#define IEEE80211_HECAP_PHY_CHAN_WIDTH_2P4G_RU242       (1 << 4)
#define IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_RU242         (1 << 5)

/* B2-B7 Channel Width Set */
#define IEEE80211_HECAP_GET_160M_SUPPORTED(phy_capa) \
    (((phy_capa[0]) >> 1) & 0x04)
#define IEEE80211_HECAP_GET_8080M_SUPPORTED(phy_capa) \
    (((phy_capa[0]) >> 1) & 0x08)

#define IEEE80211_HECAP_GET_RX_MCS_NSS_80M(mcs_set) \
    ((mcs_set[1] << 8) |  (mcs_set[0]))

#define IEEE80211_HECAP_GET_TX_MCS_NSS_80M(mcs_set) \
    ((mcs_set[3] << 8) | (mcs_set[2]))

/* B26 OFDMA RA Support */
#define IEEE80211_HECAP_GET_OFDMA_RA_SUPPORT(hecap) \
    (((hecap)->mac_cap[3] & 0x04) >> 2)

/* B31 SU Beamformer Capable */
#define IEEE80211_HECAP_GET_SU_BEAM_FORMER(phy_cap) \
    ((phy_cap[3] & 0x80) >> 7)

/* B33 MU Beamformer Capable */
#define IEEE80211_HECAP_GET_MU_BEAM_FORMER(phy_cap) \
    ((phy_cap[4] & 0x02) >> 1)

/* B22 Full Bandwidth UL MU-MIMO */
#define IEEE80211_HECAP_GET_FULL_BW_UL_MUMIMO(phy_cap) \
    ((phy_cap[2] & 0x40) >> 6)
/* B23 Partial Bandwidth UL MU-MIMO: OFDMA */
#define IEEE80211_HECAP_GET_PART_BW_UL_MUMIMO(phy_cap) \
    ((phy_cap[2] & 0x80) >> 7)
/* B54 Partial Bandwidth DL MU-MIMO */
#define IEEE80211_HECAP_GET_PART_BW_DL_MUMIMO(phy_cap) \
    ((phy_cap[6] & 0x40) >> 6)

/*
* 802.11ax HE Operation Element
*/
struct ieee80211_ie_heop {
    uint8_t id;            /* Element ID */
    uint8_t len;            /* length in bytes */
    uint8_t id_ext;            /* Element ID Extension */
    uint8_t params[3];        /* HE Operation Parameters */
    uint8_t bsscolor_info[1];    /* BSS Color Information */
    uint8_t basic_mcs_nss[2];    /* Basic HE MCS and NSS Set */
//    uint8_t vhtop_info[3];        /* VHT Operation Information(optional) */
//    uint8_t heop_maxbssid[1];    /* Max Co-Located BSSID Indicator(optional) */
} __attribute__ ((packed));
#define IEEE80211_HEOP_MINLEN        (sizeof(struct ieee80211_ie_heop))
#define IEEE80211_HEOP_MAXLEN        (sizeof(struct ieee80211_ie_heop) + 3 + 1)

#define IEEE80211_HEOP_GET_VHTOP_PRESENT(heop)  \
            (((heop)->params[1] & 0x80) >> 7)


struct ieee80211_ies {
    uint8_t *ssid;
    uint8_t *meshid;
    uint8_t *htcap;
    uint8_t *htop;
    uint8_t *vhtcap;
    uint8_t *vhtop;
    uint8_t *hecap;
    uint8_t *heop;
    uint8_t *bss_load;
};

static inline int checkHTCapabilitiesIE(uint8_t *ie)
{
    if(ie[0] == IEEE80211_ELEMID_HTCAP
            && ie[1] == IEEE80211_HTCAP_LEN - 2) {
        return 1;
    }
    return 0;
}

static inline int checkVHTCapabilitiesIE(uint8_t *ie)
{
    if(ie[0] == IEEE80211_ELEMID_VHTCAP
            && ie[1] == IEEE80211_VHTCAP_LEN - 2) {
        return 1;
    }
    return 0;
}

static inline int checkHECapabilitiesIE(uint8_t *ie)
{
    if(ie[0] == IEEE80211_ELEMID_EXT
            && ie[1] >= IEEE80211_HECAP_MINLEN - 2
            && ie[1] <= IEEE80211_HECAP_MAXLEN - 2
            && ie[2] == IEEE80211_ELEMID_EXT_HECAP)
        return 1;
    return 0;
}

/* 802.11 9.4.2.44 RM Enabled Capabilities*/
#define IEEE80211_RM_BEACON_PASSIVE_MEASURE_CAP          BIT(4)  /* B4 Beacon Passive Measurement Capability Enabled */
#define IEEE80211_RM_BEACON_ACTIVE_MEASURE_CAP           BIT(5)  /* B5 Beacon Active Measurement Capability Enabled */
#define IEEE80211_RM_BEACON_TABLE_MEASURE_CAP            BIT(6)  /* B6 Beacon Table Measurement Capability Enabled */
#define IEEE80211_RM_BEACON_MEASURE_REPORT_CONDITION_CAP BIT(7)  /* B7 Beacon Measurement Reporting COnditions Capability Enabled */
struct ieee80211_ie_rmcap {
    uint8_t id;
    uint8_t len;
    uint8_t cap[5];
} __attribute__ ((packed));
#define IEEE80211_RMCAP_LEN (sizeof(struct ieee80211_ie_rmcap))
static inline int checkRMCapabilitiesIE(uint8_t *ie)
{
    if(ie[0] == IEEE80211_ELEMID_RM_ENABLED
            && ie[1] == IEEE80211_RMCAP_LEN - 2)
        return 1;
    return 0;
}

#define IEEE80211_RMCAP_BEACON_PASSIVE_MEASURE(cap) (cap[0] & IEEE80211_RM_BEACON_PASSIVE_MEASURE_CAP)
#define IEEE80211_RMCAP_BEACON_ACTIVE_MEASURE(cap) (cap[0] & IEEE80211_RM_BEACON_ACTIVE_MEASURE_CAP)
#define IEEE80211_RMCAP_BEACON_TABLE_MEASURE(cap) (cap[0] & IEEE80211_RM_BEACON_TABLE_MEASURE_CAP)
#define IEEE80211_RMCAP_BEACON_MEASURE(cap) (cap[0] & IEEE80211_RM_BEACON_MEASURE_REPORT_CONDITION_CAP)

/* 802.11 9.4.2.26 Extended Capabilities*/
#define IEEE80211_EXTCAP_BTM_BIT 19
#define IEEE80211_BTM_CAP BIT(3)
struct ieee80211_ie_extcap {
    uint8_t id;
    uint8_t len;
    uint8_t ext_cap[8];
} __attribute__ ((packed));
#define IEEE80211_EXCAP_BSS_TRANSITION(cap) (cap[2] & IEEE80211_BTM_CAP)
static inline int checkExtCapabilitiesIE(uint8_t *ie)
{
    if(ie[0] == IEEE80211_ELEMID_EXTCAP
            && ie[1] >= IEEE80211_EXTCAP_BTM_BIT / 8 + 1)
        return 1;
    return 0;
}

/* Maximum number of supported rates (from both Supported Rates and Extended
 * Supported Rates IEs). */
#define WLAN_SUPP_RATES_MAX 32

uint8_t freq2Band(uint32_t freq);
uint8_t freq2BandIdx(uint32_t freq);
uint8_t opclass2BandIdx(uint8_t opclass);
uint8_t opclass2Band(uint8_t opclass);
uint8_t bandType2Band(uint16_t band_type);
uint8_t band2BandIdx(uint8_t band);
uint8_t bandIdx2Band(uint8_t bandidx);
int freq2Channel(uint32_t freq);
uint8_t primaryChannel2CenterChannel(uint8_t primary_channel, enum nl80211_chan_width chan_width);
int ieee80211Freq2ChannelExt(uint32_t freq, int sec_channel, int chanwidth,
                           uint8_t *opclass, uint8_t *channel);
int channel2Freq(uint8_t chan, uint8_t opclass);
uint8_t nlbw2Idx(uint32_t nlbw);
uint32_t getCf1(enum nl80211_chan_width channelWidth, uint32_t freq, enum nl80211_channel_type type);
uint8_t rssi2Rcpi(int rssi);
int rcpi2Rssi(uint8_t rcpi);
uint8_t transfer2htcapa(uint16_t ht_capa, uint8_t *ht_mcsset);
uint16_t transfer2vhtcapa(uint8_t *vht_capa, uint16_t vht_rx_mss, uint16_t vht_tx_mss);
uint16_t transfer2hecapa(uint8_t *mac_capa, uint8_t *phy_capa, uint8_t *mcs_set);
uint8_t *addNeighborReport(uint8_t *p, uint8_t *mac, uint8_t opclass, uint8_t channel, uint32_t bss_info,
                            uint8_t phy_type, uint8_t *subelement, int subelement_len);
uint8_t *addBeaconRequest(uint8_t *p, uint8_t token, uint8_t mode, uint8_t opclass, uint8_t chan_num, uint8_t *chan,
                            uint8_t *bssid, struct ssid *ssid, uint8_t detail, struct vvData *element_list);
bool parse80211IEs(uint8_t *frm, uint32_t len,
                    uint8_t offset, struct ieee80211_ies *ies);
bool parseSsidIE(uint8_t *ie, uint8_t *ssid, uint16_t *len);
bool parseMeshIdIE(uint8_t *ie, uint8_t *meshid, uint16_t *len);
bool parseBssLoadIE(uint8_t *ie, uint16_t *stas, uint8_t *util);
uint8_t parseHeopInfoBssColor(uint8_t *ie);
uint8_t parseBWFromIEs(bool is5G, struct ieee80211_ies *ies);
void parseHTCapaIE(struct band_capability *bands_capa, uint8_t *ie);
void parseVHTCapaIE(struct band_capability *bands_capa, uint8_t *ie);
void parseHECapaIE(struct band_capability *bands_capa, uint8_t *ie);

uint32_t opclass2nlBandwidth(uint32_t opclass, enum nl80211_channel_type *ptype);
int isOpclassChannelValid(uint8_t opclass, uint8_t channel);
struct opclass_channel_table *findOperatingChannelTable(uint32_t opclass);
uint8_t freq2SpecChannel(uint32_t freq, uint8_t opclass);
uint8_t chanSurveyFindOpclass(uint8_t chan);

#define WFA_OUI        "\x50\x6f\x9a"
#define WFA_OUITYPE_DPP       (0x1a)
#define WFA_OUITYPE_MAP       (0x1b)

enum {
    WIFI_MAP_BACKHAUL_STA       = (1 << 7),
    WIFI_MAP_BACKHAUL_BSS       = (1 << 6),
    WIFI_MAP_FRONTHAUL_BSS      = (1 << 5),
    WIFI_MAP_DISALLOW_R1BSTA    = (1 << 3),
    WIFI_MAP_DISALLOW_R2BSTA    = (1 << 2),
};

#endif
