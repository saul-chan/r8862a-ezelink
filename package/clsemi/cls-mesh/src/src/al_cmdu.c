
#include "platform.h"

#include "os_utils.h"
#include "al_send.h"
#include "1905_cmdus.h"
#include "platform_os.h"
#include "al_cmdu.h"

struct cmdu2_retry {
    dlist_item l;
    struct CMDU2 *cmdu;
    char *intf_name;
    void *timer;
    uint16_t resp_type;
    uint8_t dst[MACLEN];
    uint8_t times;
};

DEFINE_DLIST_HEAD(_cmdu_retry_list);

struct cmdu2_retry *_sendFindRetry(uint16_t type, uint8_t *dst)
{
    struct cmdu2_retry *e;

    dlist_for_each(e, _cmdu_retry_list, l) {
        if ((e->cmdu->type==type)
            && (!MACCMP(dst, e->dst)))
            return e;
    }
    return NULL;
}

struct cmdu2_retry *_receiveFindRetry(uint16_t type, uint8_t *dst, uint16_t mid)
{
    struct cmdu2_retry *e;

    dlist_for_each(e, _cmdu_retry_list, l) {
        if ((!MACCMP(dst, e->dst))
            && (mid==e->cmdu->id)
            && (type==e->resp_type))
            return e;
    }
    return NULL;
}

void _freeRetryData(struct cmdu2_retry * retry)
{
    cmdu2Free(retry->cmdu);
    free(retry->intf_name);

    platformCancelTimer(retry->timer);
}

void _retryHandler(void *data)
{
    struct cmdu2_retry *retry = (struct cmdu2_retry *)data;

    if (retry->timer) {
        retry->timer = NULL;
    }
 
    if (++retry->times<=3) {
        updateCMDU(retry->cmdu,  getNextMid());
        sendRawPacket2(retry->cmdu, retry->intf_name, retry->dst, NULL);
        if (!(retry->timer = platformAddTimer(2000, 0, _retryHandler, retry))) {
            goto bail;
        }
    } else {
        DEBUG_ERROR("retry exceed for %s to "MACFMT"\n", convert_1905_CMDU_type_to_string(retry->cmdu->type),
                    MACARG(retry->dst));
        goto bail;
    }
    return;
bail:
    dlist_remove(&retry->l);
    _freeRetryData(retry);
    free(retry);
}

uint8_t cmdu2AddRetry(struct CMDU2 *c, char *interface_name, uint8_t *dst)
{
    struct cmdu2_retry *retry;
    struct cmdu_msg_desc *desc = CMDU_type_to_desc(c->type);

    if (desc->resp_type==CMDU_TYPE_INVALID)
        return 0;

    if (!interface_name)
        return 0;

    //skip same dst and same type
    if ((retry = _sendFindRetry(c->type, dst))) {
        dlist_remove(&retry->l);
        _freeRetryData(retry);
    } else {
        retry = malloc(sizeof(struct cmdu2_retry));
        if (!retry)
            return 0;
    }
    retry->cmdu = cmdu2Ref(c);
    retry->times = 0;
    retry->resp_type = desc->resp_type;
    MACCPY(retry->dst, dst);
    retry->intf_name = strdup(interface_name);
    if (!(retry->timer = platformAddTimer(1000, 0, _retryHandler, retry))) {
        _freeRetryData(retry);
        free(retry);
        return 0;
    }
    dlist_add_tail(&_cmdu_retry_list, &retry->l);
    return 1;
}

void checkRetry(struct CMDU2 *c, uint8_t *src)
{
    struct cmdu_msg_desc *desc = CMDU_type_to_desc(c->type);
    struct cmdu2_retry *retry;

    if ((!desc) || (!(desc->flag & FLAG_RESPONSE_TYPE)))
        return;

    if ((retry=_receiveFindRetry(c->type, src, c->id))) {
        dlist_remove(&retry->l);
        _freeRetryData(retry);
        free(retry);
    }

}

int checkAck(struct CMDU2 *c, uint8_t *src)
{
    struct cmdu_msg_desc *desc = CMDU_type_to_desc(c->type);

    if ((!desc) || (desc->resp_type!=CMDU_TYPE_ACK))
        return 0;

    if ((desc->flag & FLAG_UNSOLICITED) && _receiveFindRetry(c->type, src, c->id))
        return 0;

    return 1;
}
