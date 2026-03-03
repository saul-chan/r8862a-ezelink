#include <netinet/ether.h>
#include "platform.h"
#include "feature/ubus/ubus_helper.h"

static const char *_failString[] = {
    [RESULT_OK] = "ok",
    [RESULT_INVALID_ARGUMENT] = "fail: invalid argument",
    [RESULT_NOT_FOUND] = "fail: not found",
    [RESULT_NOT_SUPPORT] = "fail: not support",
    [RESULT_UNKNOWN] = "fail: unknown",
};

void blobmsgAddMacStr(struct blob_buf *b, const char *name, uint8_t *mac)
{
    char buf[20];
    blobmsg_add_string(b, name, ether_ntoa_r((const struct ether_addr *)mac, buf));
}

void addResult(uint8_t rc)
{
    if (rc<RESULT_MAX)
        blobmsg_add_string(&b, "result", _failString[rc]);
}

inline void blobmsg_get_mac(struct blob_attr *attr, uint8_t *mac)
{
    ether_aton_r((char *)blobmsg_data(attr), (struct ether_addr *)mac);
}

void visit_attrs(struct blob_attr *attrs, void (*cb)(void *, void *, void *, int), void *ctx, void *param)
{
    struct blob_attr *attr;
    int rem;

    if (!attrs)
        return;

    blobmsg_for_each_attr(attr, attrs, rem)
    {
        if (cb)
            cb(ctx, param, blobmsg_data(attr), blobmsg_len(attr));
    }
}

extern struct ubus_object *getApiObj(void);
int sendUbusEvent(enum mesh_event event, struct mesh_ubus_event_request *req)
{
    int ret = UBUS_STATUS_OK;

    switch (event) {
        case EVENT_ROLE_CHANGE:
            char *old_role, *new_role;
            old_role = ROLE2STR(req->u.role_change.old_role);
            new_role = ROLE2STR(req->u.role_change.new_role);
            blob_buf_init(&b, 0);
            blobmsg_add_string(&b, "old_role", old_role);
            blobmsg_add_string(&b, "new_role", new_role);
            ret = ubus_notify(PLATFORM_GET_UBUS(), getApiObj(), "role.change", b.head, -1);
            DEBUG_INFO("send role.change ubus event. old_role: %s, new_role: %s\n", old_role, new_role);
            break;
        case EVENT_DETECT_OTHER_CONTROLLER:
            blob_buf_init(&b, 0);
            blobmsgAddMacStr(&b, "al_mac", req->u.other_controller.al_mac);
            ret = ubus_notify(PLATFORM_GET_UBUS(), getApiObj(), "role.other_controller", b.head, -1);
            DEBUG_INFO("send role.other_controller ubus event. other_controller: "MACFMT"\n",
                    MACARG(req->u.other_controller.al_mac));
            break;
        case EVENT_CONFIGURATION_SYNCHRONIZED:
            blob_buf_init(&b, 0);
            blobmsgAddMacStr(&b, "controller", req->u.configuration_synchronized.controller);
            ret = ubus_notify(PLATFORM_GET_UBUS(), getApiObj(), "configuration_synchronized", b.head, -1);
            DEBUG_INFO("send configuration_synchronized ubus event. controller: "MACFMT"\n",
                    MACARG(req->u.configuration_synchronized.controller));
            break;
        default:
            return -1;
    }

    if (ret != UBUS_STATUS_OK) {
        DEBUG_ERROR("send ubus event failed. ret: %u\n", ret);
        return -1;
    }

    return 0;
}

