#include <stdlib.h>
#include "extension.h"
#include "extension_list.h"

struct extension {
    dlist_item  l;
    struct extension_ops *ops;
    void *data;
};

static DEFINE_DLIST_HEAD(extensions);

int registerExtension(struct extension_ops *ops)
{
    struct extension *drv;

    if (!ops)
        return -1;
    drv = calloc(1, sizeof(struct extension));
    if (drv) {
        drv->ops = ops;
        dlist_add_tail(&extensions, &drv->l);
        return 0;
    }
    return -1;
}


void emulateExtensions(void)
{
    struct extension *drv;
    dlist_for_each(drv, extensions, l) {
        if (drv->ops->init)
            drv->data = drv->ops->init();
    }
}

void startExtensions(void)
{
    struct extension *drv;
    dlist_for_each(drv, extensions, l) {
        if ((drv->ops->start) && ((!drv->ops->init) || (drv->data)))
            drv->ops->start(drv->data, NULL);
    }
}

void loadExtensions(void)
{
    struct _extension *e = g_exts;

    while (e->entry) {
        e->entry();
        e++;
    }
}
