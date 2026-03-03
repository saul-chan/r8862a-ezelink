#include <libubus.h>
#include "platform.h"
#include "extension.h"
#include "feature/feature.h"

extern struct ubus_object *getCliObj(void);
extern struct ubus_object *getApiObj(void);
extern struct ubus_object *getDeObj(void);
extern struct ubus_object *getSigmaObj(void);

struct blob_buf b;


static int _ubusStart(void *p, char *cmdline)
{
    struct ubus_context *ubus;

    if (!(ubus = PLATFORM_GET_UBUS())) {
        DEBUG_ERROR("Can not connect to ubus\n");
        return -1;
    }

    ubus_add_object(ubus, getCliObj());
    ubus_add_object(ubus, getApiObj());
    ubus_add_object(ubus, getDeObj());
    ubus_add_object(ubus, getSigmaObj());

    return 0;
}



static struct extension_ops _ubus_ops = {
    .init = NULL,
    .start = _ubusStart,
    .stop = NULL,
    .deinit = NULL,
};


void ubusFeatLoad()
{
    registerExtension(&_ubus_ops);
}
