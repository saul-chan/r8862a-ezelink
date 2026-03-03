#include "platform.h"
#include "platform_os.h"

extern struct global_steer_info g_steer;

#define STEER_TYPE_TO_STRING(t) \
    ((t) == STEER_TYPE_BAND ? "band_steer" : ((t) == STEER_TYPE_RCPI ? "rcpi_steer" : "unkown"))
#define STEER_METHOD_TO_STRING(m) \
    ((m) == STEER_METHOD_BTM ? "btm" : ((m) == STEER_METHOD_DEAUTH ? "deauth" : "send_client_steer_request"))

#define STEER_RECORD_PRINT(r) \
    do { \
        DEBUG_INFO("steering record detail: \n"); \
        DEBUG_INFO("\t\t create_ts   : %s\n", r->create_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(r->create_ts)); \
        DEBUG_INFO("\t\t steer type  : %s\n", STEER_TYPE_TO_STRING(r->type)); \
        DEBUG_INFO("\t\t method      : %s\n", STEER_METHOD_TO_STRING(r->method)); \
        DEBUG_INFO("\t\t sta mac     : "MACFMT"\n", MACARG(r->c->mac)); \
        DEBUG_INFO("\t\t src bss     : "MACFMT"\n", MACARG(r->src_bss)); \
        if (r->type == STEER_TYPE_RCPI) { \
            DEBUG_INFO("\t\t src_rcpi    : %u\n", r->src_rcpi); \
            DEBUG_INFO("\t\t target_rcpi : %u\n", r->target_rcpi); \
        }\
        DEBUG_INFO("\t\t target  bss : "MACFMT"\n", MACARG(r->target_bss)); \
        DEBUG_INFO("\t\t final   bss : "MACFMT"\n", MACARG(r->final_bss)); \
        DEBUG_INFO("\t\t btm_status  : %u\n", r->btm_status_code); \
        DEBUG_INFO("\t\t status      : %s\n", r->status == 1?"success":"fail"); \
        DEBUG_INFO("\t\t btm_rsp_ts  : %s\n", r->btm_rsp_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(r->btm_rsp_ts)); \
        DEBUG_INFO("\t\t finish_ts   : %s\n", r->finish_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(r->finish_ts)); \
        DEBUG_INFO("\t\t deauth_ts   : %s\n", r->deauth_ts == 0?"":PLATFORM_TIMESTAMP_TO_STR(r->deauth_ts)); \
    } while(0);

enum steer_type {
    STEER_TYPE_BAND = 0,
    STEER_TYPE_RCPI,
    STEER_TYPE_UNKOWN = 9,
};

enum steer_method {
    STEER_METHOD_BTM = 0,
    STEER_METHOD_DEAUTH,
    STEER_METHOD_SEND_CLIENT_STEER_REQUEST
};

struct steer_record {
    dlist_item l;
    struct steer_client *c;
    uint32_t create_ts; // ms
    enum steer_type type;
    uint8_t src_rcpi;     // only valid for rcpi-based steering
    uint8_t target_rcpi;  // only valid for rcpi-based steering
    uint8_t src_bss[MACLEN];
    uint8_t target_bss[MACLEN];
    uint8_t final_bss[MACLEN];
    enum steer_method method;
    uint8_t btm_status_code; // 0xff: not received, 0: Accept
    uint8_t status;  // 1: success
    uint32_t btm_rsp_ts; // ms
    uint32_t finish_ts; // ms
    uint32_t deauth_ts; // ms
};

struct steer_client {
    dlist_item l;
    uint8_t mac[MACLEN];
    uint32_t create_ts;      // ms
    uint32_t deny_acl_ts;    // ms the time add to deny acl list
    uint8_t record_num;      // default max 128

    dlist_head records;
};

struct nac_channel_item {
    dlist_item l;
    uint8_t channel;
    void *nac_timer;    // nac timer to disable nac
};

struct global_steer_info {
    uint8_t max_stick_times;   // max stick times default 3
    uint16_t stick_check_interval; // default: (max_stick_times + 1) * cooldown
    uint8_t max_record_num;    // max record num per client default 128
    uint32_t old_age_time;     // default 86400s
    uint8_t deny_acl_period;   // default 120s

    dlist_head steer_clients;
    dlist_head nac_channels;

    void *timer;               // check records timer
};

void steerInit(void);
int isStickSta(struct steer_client *steer_c, uint8_t *bssid);
struct steer_record *steerRecordFindTypeLatest(struct steer_client *steer_c, enum steer_type stype);
struct steer_record *steerRecordFindLatest(struct steer_client *steer_c);
struct steer_record *steerRecordAdd(enum steer_type type, uint8_t method, uint8_t *mac,
        uint8_t *current_bss, uint8_t *target_bss, uint8_t src_rcpi, uint8_t target_rcpi);
int steerRecordDel(struct steer_record *record);
struct steer_client *steerClientFind(uint8_t *mac);
struct steer_client *steerClientAdd(uint8_t *mac);
int steerClientDel(struct steer_client *steer_c);
int setNacEnable(uint8_t channel);


