/*
 *  Copyright (c) 2024, Clourneysemi. All rights reserved.
 *  This software and/or documentation is licensed by Clourneysemi under
 *  limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without
 *  modification, for use solely in conjunction with a Clourneysemi chipset, is
 *  permitted in condition which must retain the above copyright notice,
 *
 *  By using this software and/or documentation, you agree to the limited terms and
 *  conditions.
 */

#include "1905_tlvs.h"
#include "packet_tools.h"
#include "al_recv.h"
#include "extension.h"
#define VENDORA_PROTOCOL (0xcc)


enum {
    TLV_TYPE_WIFI_POWER = 0x61,
    TLV_TYPE_WPS_CONTROL = 0x62,
    TLV_TYPE_REBOOT = 0x63,
    TLV_TYPE_LED_CONTROL = 0x64,
    TLV_TYPE_VENDORA_MAX,
};

enum {
    TX_POWER_100P = 0x0,
    TX_POWER_60P = 0x1,
    TX_POWER_40P = 0x2,
};

#define TLV_TYPE_IDX(_type) (_type-TLV_TYPE_WIFI_POWER)

struct radioCtlTLV {
    struct TLV tlv;
    mac_address rid;
    uint8_t control;
};

static struct TLVDesc _vdescs[] = {
    [TLV_TYPE_IDX(TLV_TYPE_WIFI_POWER)] =
            TLV_DESC_2FIELDS(radioCtlTLV, 0, rid, fmt_binary, control, fmt_unsigned, NULL),
    [TLV_TYPE_IDX(TLV_TYPE_WPS_CONTROL)] =
            TLV_DESC_2FIELDS(radioCtlTLV, 0, rid, fmt_binary, control, fmt_unsigned, NULL),
    [TLV_TYPE_IDX(TLV_TYPE_REBOOT)] =
            TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_unsigned, NULL),
    [TLV_TYPE_IDX(TLV_TYPE_LED_CONTROL)] =
            TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_unsigned, NULL),

};

struct TLVDesc *_getVDesc(uint16_t type)
{
    return getDesc(_vdescs, TLV_TYPE_IDX(TLV_TYPE_VENDORA_MAX), type-TLV_TYPE_WIFI_POWER);
}


static int process1(struct al_device *d, uint32_t recv_idx, uint8_t *payload, uint16_t payload_len)
{
    struct TLVDesc *desc;
    struct TLVStruct *s;
    uint8_t *p = payload;
    uint8_t type;
    uint16_t length;

    if (!p)
        return -1;


    if ((isRegistrar()) || (d!=registrar))
        return -1;

    while (payload_len>3) {
        p = payload;
        _E1B(&p, &type);
        _E2B(&p, &length);
        payload += length+3;
        payload_len -= length+3;

        if ((!(desc = _getVDesc(type))) ||
                (!(s = decodeTLVOne(desc, NULL, &p, &length))))
            continue;

        switch (type) {
            case TLV_TYPE_WIFI_POWER:
            {
                struct radioCtlTLV *ctrl = (struct radioCtlTLV *)s;
                struct radio *r = radioFind(local_device, ctrl->rid);
                uint8_t power=0;

                if (r) {
                    switch (ctrl->control) {
                        case TX_POWER_100P:
                            power = 100;
                            break;
                        case TX_POWER_60P:
                            power = 60;
                            break;
                        case TX_POWER_40P:
                            power = 40;
                            break;
                        default:
                            break;
                    };
                    //TODO: Setting power to driver
                    DEBUG_INFO("set radio["MACFMT"] power to %d%%\n", MACARG(ctrl->rid), power);

                }
                break;
            }
            case TLV_TYPE_WPS_CONTROL:
            {
                struct radioCtlTLV *ctrl = (struct radioCtlTLV *)s;
                struct radio *r = radioFind(local_device, ctrl->rid);

                if (r) {
                    //TODO: Set wps to driver
                    DEBUG_INFO("set radio["MACFMT"] WPS %s\n", MACARG(ctrl->rid), ctrl->control ? "on":"off");

                }
                break;
            }
            case TLV_TYPE_REBOOT:
            {
                struct u8TLV *reboot = (struct u8TLV *)s;
                if (reboot->v1 == 1) {
                    DEBUG_INFO("rebooting triggered by controller\n");
                    system("reboot");
                }
                break;
            }
            case TLV_TYPE_LED_CONTROL:
            {
                struct u8TLV *led = (struct u8TLV *)s;
                DEBUG_INFO("set LED %s\n", led->v1 ? "on":"off");
                break;
            }
            default:
                break;
        };

        tlist_delete_item(&s->t);
    };

    return 0;
}

static int _vendor_start(void *p, char *cmdline)
{
    registerHigherLayerDataProtocol(VENDORA_PROTOCOL, process1);
    return 0;
}

static struct extension_ops _vendor_ops = {
    .init = NULL,
    .start = _vendor_start,
    .stop = NULL,
    .deinit = NULL,
};


void vendorALoad()
{
    registerExtension(&_vendor_ops);
}
