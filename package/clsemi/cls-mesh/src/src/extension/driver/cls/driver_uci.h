#ifndef _DRIVER_UCI_H
#define _DRIVER_UCI_H


#define UCI_ADD_SECTION(_ctx, _ptr, _name, _value) \
    do { \
        (_ptr).s = NULL; \
        (_ptr).section = (_name); \
        (_ptr).value = (_value); \
        ret = uci_set((_ctx), &(_ptr)); \
    }while(0)

#define UCI_ADD_OPTION(_ctx, _ptr, _name, _value)\
    do {\
        (_ptr).o = NULL;\
        (_ptr).option = (_name);\
        (_ptr).value = (_value);\
        ret = uci_set((_ctx), &(_ptr));\
    }while(0)

#define UCI_START_OPTION_LIST(_ptr, _name)\
    do {\
        (_ptr).o = NULL;\
        (_ptr).option = (_name);\
    }while(0)
#define UCI_ADD_OPTION_LIST(_ctx, _ptr, _value) \
    do {\
        (_ptr).value = (_value);\
        ret = uci_add_list((_ctx), &(_ptr));\
    }while(0)

const char *get_uci_encryption(uint8_t auth, uint8_t encrypt);

#endif
