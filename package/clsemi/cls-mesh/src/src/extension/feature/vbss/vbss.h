#ifndef _VBSS_H_
#define _VBSS_H_

#ifdef DUBHE1000
    #define CLSAPI_SET_VBSS_ENABLE clsapi_set_wifi_vbss_enabled
    #define CLSAPI_GET_VBSS_VAP clsapi_get_wifi_vbss_vap
    #define CLSAPI_ADD_VBSS_VAP clsapi_add_wifi_vbss_vap
    #define CLSAPI_DEL_VBSS_VAP clsapi_del_wifi_vbss_vap
    #define CLSAPI_GET_VBSS_STA clsapi_get_wifi_vbss_sta
    #define CLSAPI_ADD_VBSS_STA clsapi_add_wifi_vbss_sta
    #define CLSAPI_DEL_VBSS_STA clsapi_del_wifi_vbss_sta
    #define CLSAPI_TRIGGER_SWITCH clsapi_trigger_wifi_vbss_switch
    #define CLSAPI_SWITCH_DONE clsapi_set_wifi_vbss_switch_done
#else
    #define CLSAPI_SET_VBSS_ENABLE clsapi_wifi_set_vbss_enabled
    #define CLSAPI_GET_VBSS_VAP clsapi_wifi_get_vbss_vap
    #define CLSAPI_ADD_VBSS_VAP clsapi_wifi_add_vbss_vap
    #define CLSAPI_DEL_VBSS_VAP clsapi_wifi_del_vbss_vap
    #define CLSAPI_GET_VBSS_STA clsapi_wifi_get_vbss_sta
    #define CLSAPI_ADD_VBSS_STA clsapi_wifi_add_vbss_sta
    #define CLSAPI_DEL_VBSS_STA clsapi_wifi_del_vbss_sta
    #define CLSAPI_TRIGGER_SWITCH clsapi_wifi_trigger_vbss_switch
    #define CLSAPI_SWITCH_DONE clsapi_wifi_set_vbss_switch_done
#endif

#define VBSS_STATE_SWITCH(e, s) (e->state = s)

enum vbss_state_e {
    VBSS_STATE_START = 0,
    VBSS_STATE_PREPARE,
    VBSS_STATE_CREATION_VAP,
    VBSS_STATE_TRIGGER_CSA,
    VBSS_STATE_CANCEL,
    VBSS_STATE_DESTRUCTION_VAP,
    VBSS_STATE_DONE,
};

struct vbss_switch_entry {
    dlist_item l;
    mac_address client_mac;
    mac_address bssid;
    mac_address src_al_mac;
    mac_address dst_al_mac;
    mac_address src_ruid;
    mac_address dst_ruid;
    enum vbss_state_e state;
};


#endif
