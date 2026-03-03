#include <pthread.h>
#include "os_utils.h"
#include "datamodel.h"
#include "al_msg.h"
#include "packet_tools.h"
#include "platform_os.h"

#define BUF_SIZE (8192)

enum {
    queue_id_family_driver = 0,
    queue_id_family_feat,
    queue_id_max
};

struct msg_family_handler {
    void (*handler)(void *, uint8_t *, uint32_t);
    void *data;
};

#define SOCKET_PREFIX "/tmp/"
static void *_msg_queue[queue_id_max]  = {NULL};
static const char *_msg_queue_name[queue_id_max] = {SOCKET_PREFIX"meshq0", SOCKET_PREFIX"meshq1"};
static struct msg_family_handler _msg_familys[msg_family_max] = {0};

static void *_family2Queue(uint8_t family)
{
    switch (family) {
        case msg_family_driver_evt:
        case msg_family_driver_cmd:
            return _msg_queue[queue_id_family_driver];
            break;
        case msg_family_feat_evt:
        case msg_family_feat_cmd:
            return _msg_queue[queue_id_family_feat];
            break;
        default:
            return NULL;
    }
}
static pthread_mutex_t _buf_lock;
static uint8_t *_b;

uint32_t msgGetMaxBufSize(void)
{
    return BUF_SIZE;
}

uint8_t * msgGetBuf(int size)
{
    pthread_mutex_lock(&_buf_lock);
    return _b;
}

void msgPutBuf(uint8_t *buf)
{
    pthread_mutex_unlock(&_buf_lock);
}



int msgaParseOne(struct msg_attr *attrs, uint8_t **pbuf, uint16_t *plen)
{
    uint16_t type;
    if (attrs && (*plen>=4)) {
        _E2B(pbuf, &type);
        _E2B(pbuf, &attrs->len);
        attrs->data = *pbuf;
        *pbuf += attrs->len;
        *plen -= (attrs->len+4);
        return (int)type;
    }

    return attr_none;
}

int msgaParse(struct msg_attr *attrs, uint16_t max, uint8_t *buf, uint16_t len)
{
    uint8_t *p = buf, *pe = buf+len;
    uint16_t t, l;

    if (!attrs)
        return -1;

    while (p<=pe-4) {
        _E2B(&p, &t);
        _E2B(&p, &l);
        if (t>=max) return -2;
        if ((pe-p)<l) return -3;
        attrs[t].len = l;
        attrs[t].data = p;
        p+=l;
    }
    return 0;
}

uint8_t *msgaPutAttrHeader(uint8_t *p, uint16_t type, uint16_t len)
{
    uint8_t *pp = p;
    _I2B(&type, &pp);
    _I2B(&len, &pp);
    return pp;
}

uint16_t msgaGetLen(struct msg_attr *attr)
{
    return attr->len;
}

uint8_t msgaGetU8(struct msg_attr *attr)
{
    uint8_t ret;
    uint8_t *p = attr->data;
    _E1B(&p, &ret);
    return ret;
}

uint8_t *msgaPutU8(uint8_t *p, uint16_t type, uint8_t value)
{
    uint8_t *pp = p;
    uint16_t l = 1;
    _I2B(&type, &pp);
    _I2B(&l, &pp);
    _I1B(&value, &pp);
    return pp;
}

uint16_t msgaGetU16(struct msg_attr *attr)
{
    uint16_t ret;
    uint8_t *p = attr->data;
    _E2B(&p, &ret);
    return ret;
}

uint8_t *msgaPutU16(uint8_t *p, uint16_t type, uint16_t value)
{
    uint8_t *pp = p;
    uint16_t l = 2;
    _I2B(&type, &pp);
    _I2B(&l, &pp);
    _I2B(&value, &pp);
    return pp;
}


uint32_t msgaGetU32(struct msg_attr *attr)
{
    uint32_t ret;
    uint8_t *p = attr->data;
    _E4B(&p, &ret);
    return ret;
}

uint8_t *msgaPutU32(uint8_t *p, uint16_t type, uint32_t value)
{
    uint8_t *pp = p;
    uint16_t l = 4;
    _I2B(&type, &pp);
    _I2B(&l, &pp);
    _I4B(&value, &pp);
    return pp;
}

uint32_t msgaGetU64(struct msg_attr *attr)
{
    uint64_t ret;
    uint8_t *p = attr->data;
    _E8B(&p, &ret);
    return ret;
}

uint8_t *msgaPutU64(uint8_t *p, uint16_t type, uint64_t value)
{
    uint8_t *pp = p;
    uint16_t l = 8;
    _I2B(&type, &pp);
    _I2B(&l, &pp);
    _I8B(&value, &pp);
    return pp;
}


uint8_t *msgaGetBin(struct msg_attr *attr)
{
    return attr->data;
}

uint8_t *msgaPutBin(uint8_t *p, uint16_t type, uint8_t *buf, uint16_t len)
{
    uint8_t *pp = p;
    _I2B(&type, &pp);
    _I2B(&len, &pp);
    _InB(buf, &pp, len);
    return pp;
}

char *msgaGetStr(struct msg_attr *attr)
{
    return (char *)attr->data;
}

uint8_t *msgaPutStr(uint8_t *p, uint16_t type, char *str)
{
    uint8_t *pp = p;
    uint16_t len = (uint16_t)strlen(str)+1;

    _I2B(&type, &pp);
    _I2B(&len, &pp);
    _InB(str, &pp, len-1);
    *pp++ = 0;
    return pp;
}

uint8_t *msgaPutFlag(uint8_t *p, uint16_t type)
{
    uint8_t *pp = p;
    uint16_t len = 0;

    _I2B(&type, &pp);
    _I2B(&len, &pp);
    return pp;
}

static void _processEvt(void *data, uint8_t *msg, uint32_t len)
{
    uint8_t family = msg[0];

    if ((family<msg_family_max) && (_msg_familys[family].handler)) {
        _msg_familys[family].handler(_msg_familys[family].data, msg+1, len-1);
    }
}

int msgInit()
{
    int i;

    for (i=0;i<queue_id_max;i++) {
        if (!(_msg_queue[i] = platformCreateQueue(_msg_queue_name[i], 0, _processEvt, NULL))){
            DEBUG_ERROR("can not create queue for msg\n");
            return -1;
        }
        platformStartQueue(_msg_queue[i]);
    }
    _b = malloc(BUF_SIZE);
    pthread_mutex_init(&_buf_lock, NULL);

    return 0;
}

int msgRegisterFamily(uint8_t family, void (*handler)(void *, uint8_t *, uint32_t), void *data)
{
    if ((family < msg_family_max) && (handler)) {
        _msg_familys[family].handler = handler;
        _msg_familys[family].data = data;
        return 0;
    }
    return -1;
}

int msgSend(uint8_t *msg, uint16_t len)
{
    uint8_t fml = *msg;
    void *q = _family2Queue(fml);
    if (q)
        return platformQueueSend(q, msg, len);

    return -1;
}
