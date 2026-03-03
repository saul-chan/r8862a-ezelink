#include "packet_tools.h"
#include "1905_tlvs.h"
#include "tlv.h"


int decodeTLVArray(struct TLVDesc *desc, dlist_head *parent,
                    uint8_t **pp, uint16_t *plen)
{
    uint32_t n = 0, i;

    if (desc->count_type) {
        switch (desc->count_type) {
            case 1:
            {
                uint8_t nn;
                _E1B(pp, &nn);
                n = nn;
                break;
            }
            case 2:
            {
                uint16_t nn;
                _E2B(pp, &nn);
                n = nn;
                break;
            }
            case 4:
            {
                _E4B(pp, &n);
                break;
            }
            default:
                return -1;
                break;
        }
        *plen -= desc->count_type;

        for (i=0;i<n;i++) {
            struct TLVStruct *child = decodeTLVOne(desc, parent, pp, plen);
            if (!child)
                return -1;
        }
    } else {
        while (*plen>0) {
            struct TLVStruct *child = decodeTLVOne(desc, parent, pp, plen);
            if (!child)
                return -1;
            n++;
        }
    }

    return n;
}

uint8_t *requestBuf(struct TLVStruct *s, size_t len)
{
    uint8_t *ret = NULL;
    struct tlist_internal_buffer *tbuf;

    if ((!s) || (!len))
        return ret;

    tbuf = malloc(sizeof(struct tlist_internal_buffer)+len);
    if (tbuf) {
        ret = tbuf->data;
        dlist_add_tail(&s->t.internal_bufs, &tbuf->l);
    }
    return ret;
}

int decodeTLVField(struct TLVFieldDesc *desc, struct TLVStruct *s,
                    uint8_t **pp, uint16_t *plen)
{
    uint8_t *dst = ((uint8_t *)s) + desc->offset;
    size_t l = 0;
    int ret = -1;

    if ((desc->fmt==fmt_binary) && ((*plen)<desc->size))
        return -1;

    switch (desc->fmt) {
        case fmt_unsigned:
            if (desc->size==1)
                l = _E1B(pp, dst);
            else if (desc->size==2)
                l = _E2B(pp, (uint16_t *)dst);
            else if (desc->size==4)
                l = _E4B(pp, (uint32_t *)dst);
            else if (desc->size==8)
                l = _E8B(pp, (uint64_t *)dst);
            else
                goto bail;
            break;
        case fmt_binary:
            l =  _EnB(pp, dst, desc->size);
            break;
        case fmt_l1v:
        case fmt_l4bitsv:
            _E1B(pp, dst);
            l = (size_t)(*(dst++));
            if (desc->fmt==fmt_l4bitsv)
                l &= FMT_L4BITSV_LEN_MASK;
            if (l<desc->size)
                _EnB(pp, dst, l);
            else
                goto bail;
            l+=1;
            break;
        case fmt_l0vv:
        {
            struct vvData *data = (struct vvData *)dst;
            data->len = *plen;
            if (data->len) {
                if ((dst = requestBuf(s, data->len))) {
                    data->datap = dst;
                    _EnB(pp, dst, data->len);
                } else
                    goto bail;
            }
            l = data->len;
            break;
        }
        case fmt_l1vv:
        {
            struct vvData *data = (struct vvData *)dst;
            uint8_t len;
            _E1B(pp, &len);
            data->len = len;
            if (data->len) {
                if ((dst = requestBuf(s, data->len))) {
                    data->datap = dst;
                    _EnB(pp, dst, data->len);
                } else
                    goto bail;
            }
            l = data->len+1;
            break;
        }
        case fmt_l2vv:
        {
            struct vvData *data = (struct vvData *)dst;
            uint16_t len;
            _E2B(pp, &len);
            data->len = len;
            if (data->len) {
                if ((dst = requestBuf(s, data->len))) {
                    data->datap = dst;
                    _EnB(pp, dst, data->len);
                } else
                    goto bail;
            }
            l = data->len+2;
            break;
        }
        case fmt_elem:
        {
            struct vvData *data = (struct vvData *)dst;
            uint8_t id;
            uint8_t len;
            _E1B(pp, &id);
            _E1B(pp, &len);
            data->len = len+2;
            if ((dst = requestBuf(s, data->len))) {
                data->datap = dst;
                dst[0] = id;
                dst[1] = len;
                _EnB(pp, dst+2, len);
            } else
                goto bail;
            l = len+2;
            break;
        }
        case fmt_skip:
        {
            l = desc->size;
            *pp += l;
            break;
        }

        default:
            DEBUG_ERROR("unknown field fmt %d\n", desc->fmt);
            break;
    }
    *plen -= l;
    ret = 0;
bail:
    if (ret)
        DEBUG_ERROR("decodeTLVfield err=%d\n", ret);
    return ret;
}

struct TLVStruct *TLVStructNew(struct TLVDesc *desc, dlist_head *parent, uint32_t size)
{
    struct TLVStruct *s = NULL;

    if (!size) {
        if (desc)
            size = desc->size;
        else
            size = sizeof(struct TLV);
    }
    s = container_of(tlist_alloc(size, (parent)), struct TLVStruct, t);

    if (s) {
        s->desc = desc;
    }

    return s;
}

struct TLVStruct *decodeTLVOne(struct TLVDesc *desc, dlist_head *parent,
                                       uint8_t **pp, uint16_t *plen)
{
    int i;
    struct TLVStruct *s;

    if ((desc->ops) && (desc->ops->decode))
        return desc->ops->decode(desc, parent, pp, plen);

    s = TLVStructNew(desc, parent, 0);

    for (i=0; (i<ARRAY_SIZE(desc->fields)) && (desc->fields[i].name); i++) {
        if (decodeTLVField(&desc->fields[i], s, pp, plen)) {
            DEBUG_WARNING("decode field %s failed\n", desc->fields[i].name);
            goto fail;
        }
    }

    for (i=0; (i<ARRAY_SIZE(desc->childs)) && (desc->childs[i]); i++) {
        if (decodeTLVArray(desc->childs[i], &s->t.childs[i], pp, plen)<0) {
            DEBUG_WARNING("decode child %s failed\n", desc->childs[i]->name);
            goto fail;
        }
    }

    for (i=0; (i<ARRAY_SIZE(desc->fields1)) && (desc->fields1[i].name); i++) {
        if (decodeTLVField(&desc->fields1[i], s, pp, plen)) {
            DEBUG_WARNING("decode field %s failed\n", desc->fields1[i].name);
            goto fail;
        }
    }
    return s;
fail:
    if (parent)
        dlist_remove(&s->t.l);
    tlist_delete_item(&s->t);
    return NULL;
}

struct TLV *decodeTLV(uint8_t *p)
{
    struct TLVDesc *desc;
    uint8_t tlv_type;
    uint16_t tlv_len;
    struct TLVStruct *s;
    struct TLV *tlv = NULL;

    _E1B(&p, &tlv_type);
    _E2B(&p, &tlv_len);

    desc = getTLVDesc(tlv_type);
    if (desc) {
        s = decodeTLVOne(desc, NULL, &p, &tlv_len);
    } else {
        s = TLVStructNew(NULL, NULL, 0);
        tlv_type = TLV_TYPE_UNKNOWN;
    }
    if (s) {
        tlv = container_of(s, struct TLV, s);
        tlv->tlv_type = tlv_type;
    }
    return tlv;
}

int encodeTLVArray(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *plen)
{
    struct TLVStruct *child;
    uint32_t n = dlist_count(parent);

    switch (desc->count_type) {
        case 0:
            break;
        case 1:
        {
            uint8_t nn = n;
            _I1B(&nn, pp);
            break;
        }
        case 2:
        {
            uint16_t nn = n;
            _I2B(&nn, pp);
            break;
        }
        case 4:
            _I4B(&n, pp);
            break;
        default:
            return -1;
            break;
    }
    *plen -= desc->count_type;

    tlist_for_each(child, *parent, struct TLVStruct, t) {
        encodeTLVOne(child, pp, plen);
    }

    return 0;
}

int encodeTLVField(struct TLVFieldDesc *desc, struct TLVStruct *s, uint8_t **pp, uint16_t *plen)
{
    uint8_t *src = ((uint8_t *)s) + desc->offset;
    int l = 0;
    int ret = -1;

    switch (desc->fmt) {
        case fmt_unsigned:
            if (desc->size==1) {
                _I1B(src, pp);
                l = 1;
            } else if (desc->size==2) {
                _I2B((uint16_t *)src, pp);
                l = 2;
            } else if (desc->size==4) {
                _I4B((uint32_t *)src, pp);
                l = 4;
            } else if (desc->size==8) {
                _I8B((uint64_t *)src, pp);
                l = 8;
            } else {
                ret = -2;
                goto bail;
            }
            break;
        case fmt_binary:
            _InB(src, pp, desc->size);
            l = desc->size;
            break;
        case fmt_l1v:
        case fmt_l4bitsv:
        {
            int len = *(src);
            if (desc->fmt==fmt_l4bitsv)
                len &= FMT_L4BITSV_LEN_MASK;
            if (len<desc->size) {
                _I1B(src++, pp);
                _InB(src, pp, len);
                l = len+1;
            }else{
                ret = -3;
                goto bail;
            }
            break;
        }
        case fmt_l0vv:
        {
            struct vvData *data = (struct vvData *)src;
            _InB(data->datap, pp, data->len);
            l = data->len;
            break;
        }
        case fmt_l1vv:
        {
            struct vvData *data = (struct vvData *)src;
            uint8_t len = data->len;
            _I1B(&len, pp);
            _InB(data->datap, pp, data->len);
            l = data->len + 1;
            break;
        }
        case fmt_l2vv:
        {
            struct vvData *data = (struct vvData *)src;
            uint16_t len = data->len;
            _I2B(&len, pp);
            _InB(data->datap, pp, data->len);
            l = data->len + 2;
            break;
        }
        case fmt_elem:
        {
            struct vvData *data = (struct vvData *)src;
            _InB(data->datap, pp, data->len);
            l = data->len;
            break;
        }
        case fmt_skip:
        {
            l = desc->size;
            *pp += l;
            break;
        }
        default:
            goto bail;
            break;
    }
    *plen-=l;

    ret = 0;
bail:
    if (ret)
        DEBUG_ERROR("encodeTLVfield %s failed, err=%d\n", desc->name ? desc->name: "unknown", ret);
    return ret;
}



int encodeTLVOne(struct TLVStruct *s,uint8_t **pp, uint16_t *plen)
{
    int i;

    if ((s->desc->ops) && (s->desc->ops->encode))
        return s->desc->ops->encode(s, pp, plen);

    for (i=0; i<ARRAY_SIZE(s->desc->fields) && (s->desc->fields[i].name); i++) {
        encodeTLVField(&s->desc->fields[i], s, pp, plen);
    }

    for (i=0; i<ARRAY_SIZE(s->desc->childs) && (s->desc->childs[i]); i++) {
        encodeTLVArray(s->desc->childs[i], &s->t.childs[i], pp, plen);
    }

    for (i=0; i<ARRAY_SIZE(s->desc->fields1) && (s->desc->fields1[i].name); i++) {
        encodeTLVField(&s->desc->fields1[i], s, pp, plen);
    }

    return 0;
}

static uint16_t getLenTLVArray(struct TLVDesc *desc, dlist_head *parent)
{
    uint16_t len = desc->count_type;
    struct TLVStruct *child;

    tlist_for_each(child, *parent, struct TLVStruct, t) {
        len += getLenTLVOne(child);
    }
    return len;
}

uint16_t getLenField(struct TLVStruct *s, struct TLVFieldDesc *f_desc)
{
    uint16_t l = 0;

    switch (f_desc->fmt) {
        case fmt_unsigned:
        case fmt_binary:
        case fmt_skip:
            l = f_desc->size;
            break;
        case fmt_l1v:
        case fmt_l4bitsv:
        {
            uint8_t *src = ((uint8_t *)s) + f_desc->offset;
            if (f_desc->fmt==fmt_l4bitsv)
                l = 1+((*src) & FMT_L4BITSV_LEN_MASK);
            else
                l = 1+(*src);
            break;
        }
        case fmt_l1vv:
        {
            struct vvData *data = (struct vvData *)(((uint8_t *)s) + f_desc->offset);
            l = data->len+1;
            break;
        }
        case fmt_l2vv:
        {
            struct vvData *data = (struct vvData *)(((uint8_t *)s) + f_desc->offset);
            l = data->len+2;
            break;
        }
        case fmt_l0vv:
        case fmt_elem:
        {
            struct vvData *data = (struct vvData *)(((uint8_t *)s) + f_desc->offset);
            l = data->len;
            break;
        }
        default:
            break;
    }
    return l;
}

uint16_t getLenTLVOne(struct TLVStruct *s)
{
    int i;
    uint16_t len = 0;

    if ((s->desc->ops) && (s->desc->ops->length))
        return s->desc->ops->length(s);

    for (i=0; i<ARRAY_SIZE(s->desc->fields) && (s->desc->fields[i].name); i++) {
        len += getLenField(s, &(s->desc->fields[i]));
    }

    for (i=0; i<ARRAY_SIZE(s->desc->childs) && (s->desc->childs[i]); i++) {
        len += getLenTLVArray(s->desc->childs[i], &s->t.childs[i]);
    }

    for (i=0; i<ARRAY_SIZE(s->desc->fields1) && (s->desc->fields1[i].name); i++) {
        len += getLenField(s, &(s->desc->fields1[i]));
    }

    return len;
}

int encodeTLV(struct TLV *tlv, uint8_t **pp, uint16_t *plen)
{
    uint16_t len = 0, l;
    uint8_t *buf, *p;

    len += getLenTLVOne(&tlv->s);
    l = len;

    if (*pp) {
        buf = *pp;
    } else {
        buf = malloc(len+3);
    }

    if ((p = buf)) {
        //write type only, and calculate the len after encode
        _I1B(&tlv->tlv_type, &p);
        p+=2;l-=2;//skip len

        encodeTLVOne(&tlv->s, &p, &l);
        len = p-buf-3;
        p = buf+1;
        _I2B(&len, &p);
        *pp = buf;
        *plen = len+3;
    } else {
        *pp = NULL;
        *plen = 0;
    }

    return 0;
}

//coverity[+free : arg-0]
void TLVFree(struct TLV *tlv)
{
    tlist_item *t = &tlv->s.t;
    dlist_remove(&t->l);
    if (tlv->ref--) return;

    if ((tlv->s.desc) && (tlv->s.desc->ops) && (tlv->s.desc->ops->free))
        tlv->s.desc->ops->free(&tlv->s);
    tlist_delete_item(t);
}

struct TLV *TLVNew(dlist_head *parent, uint8_t type, uint32_t size)
{
    struct  TLVDesc *desc = getTLVDesc(type);
    struct TLV *tlv = (struct TLV *)TLVStructNew(desc, parent, size);
    if (tlv) {
        tlv->tlv_type = type;
    }
    return tlv;
}

struct TLV *TLVRef(struct TLV *tlv)
{
    if (tlv)
        tlv->ref++;
    return tlv;
}
