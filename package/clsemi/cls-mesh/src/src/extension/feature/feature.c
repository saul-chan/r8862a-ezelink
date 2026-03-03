#include "platform.h"
#include "datamodel.h"
#include "platform_os.h"
#include "packet_tools.h"
#include "al_msg.h"
#include "feature.h"

struct feature_ctrl {
    void *evt_queue;
    dlist_head evt_handler[feat_evt_max];
};

struct feature_evt_handler {
    dlist_item l;
    int (*handler)(void *, uint8_t *, uint16_t);
    void *data;
};

static struct feature_ctrl _feat_ctrl = {0};


static void _featureEvtProcess(void *data, uint8_t *msg, uint32_t len)
{
    uint8_t evt = msg[0];
    struct feature_evt_handler *e;
    struct feature_ctrl *ctrl = (struct feature_ctrl *)data;

    dlist_for_each(e, ctrl->evt_handler[evt], l) {
        e->handler(e->data, msg+1, len-1);
    };
}

int featInit()
{
    int i;
    struct feature_ctrl *ctrl = &_feat_ctrl;

    for (i=0;i<feat_evt_max;i++)
        dlist_head_init(&ctrl->evt_handler[i]);

    msgRegisterFamily(msg_family_feat_evt, _featureEvtProcess, ctrl);
    return 0;
}


int featSuscribeEvent(uint8_t evt, int (*handler)(void *ctx, uint8_t *m, uint16_t l), void *data)
{
    struct feature_ctrl *ctrl = &_feat_ctrl;
    int ret = -1;

    if ((handler) && (evt<feat_evt_max)) {
        struct feature_evt_handler *h = calloc(1, sizeof(struct feature_evt_handler));

        h->handler = handler;
        h->data = data;
        dlist_add_tail(&ctrl->evt_handler[evt], &h->l);

        ret = 0;
    }
    return ret;
}

