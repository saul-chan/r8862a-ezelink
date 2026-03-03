#ifndef _UBUS_HELPER_H
#define _UBUS_HELPER_H

#include <libubus.h>
#include <libubox/blobmsg.h>
#include <netinet/ether.h>

extern struct blob_buf b;

#define DECLARE_FUNC_UBUS_METHOD(_method) \
static int _method(struct ubus_context *ctx, \
        struct ubus_object *obj, struct ubus_request_data *req, \
        const char *method, struct blob_attr *msg)
#define DECLARE_UBUS_OBJ(_name, _obj_name, _type, _methods) \
        static struct ubus_object _name = { \
            .name = _obj_name, \
            .type = _type, \
            .methods = _methods, \
            .n_methods = ARRAY_SIZE(_methods), \
        }

#define POLICY_ATTR(_index, _name, _type)\
    [_index] = {.name = _name, .type = _type}

enum {
    RESULT_OK = 0,
    RESULT_INVALID_ARGUMENT,
    RESULT_NOT_FOUND,
    RESULT_NOT_SUPPORT,
    RESULT_UNKNOWN,
    RESULT_MAX,
};

enum mesh_event {
    EVENT_ROLE_CHANGE,
    EVENT_DETECT_OTHER_CONTROLLER,
    EVENT_CONFIGURATION_SYNCHRONIZED,
    EVENT_MAX,
};

struct mesh_ubus_event_request {
#define ROLE2STR(role) (role == 0 ? "auto" : (role == 1 ? "controller" : "agent"))
    union {
        struct {
            uint8_t old_role;   // 0: auto, 1: controller, 2: agent
            uint8_t new_role;   // 0: auto, 1: controller, 2: agent
        }role_change;
        struct {
            uint8_t al_mac[6];
        }other_controller;
        struct {
            uint8_t controller[6];
        }configuration_synchronized;
    }u;
};


void blobmsgAddMacStr(struct blob_buf *b, const char *name, uint8_t *mac);
void addResult(uint8_t rc);
void blobmsg_get_mac(struct blob_attr * attr, uint8_t * mac);
void visit_attrs(struct blob_attr *attrs, void (*cb)(void *, void *, void *, int), void *ctx, void *param);
int sendUbusEvent(enum mesh_event event, struct mesh_ubus_event_request *request);

#endif
