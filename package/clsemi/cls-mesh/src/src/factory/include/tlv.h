#ifndef _TLV_H_
#define _TLV_H_



#define TLV_DESC_MAX_FIELD    16
#define TLV_DESC_MAX_FIELD1   3
#define TLV_DESC_MAX_CHILD    TLIST_MAX_CHILD
#define TLV_TYPE_UNKNOWN      (0x7f)



enum e_fmt {
    fmt_skip,
    fmt_binary,
    fmt_unsigned,
    fmt_l1v,  //1 byte L with V
    fmt_l4bitsv, // 4bit len
    fmt_l0vv, //V, L in TLV
    fmt_l1vv, //1 byte L with malloced V
    fmt_l2vv, //2 bytes L with malloced V
    fmt_elem, // 1 byte ID, 1 byte len, value
};

#define FMT_L4BITSV_LEN_MASK  (0xf)

struct TLVFieldDesc {
    const char *name;
    size_t offset;
    size_t size; //field size
    int fmt;
};

struct TLVDesc {
    const char *name;
    struct TLVFieldDesc fields[TLV_DESC_MAX_FIELD];
    struct TLVDesc *childs[TLV_DESC_MAX_CHILD];
    struct TLVFieldDesc fields1[TLV_DESC_MAX_FIELD1];
    size_t size; //tlv structure size
    struct TLVOperator *ops;
    uint16_t count_type;
    uint16_t tag;
};

struct TLVStruct {
    tlist_item t;
    struct TLVDesc *desc;
};

struct TLV {
    struct TLVStruct s;
    uint8_t ref;
    union {
        uint8_t tlv_type;
        uint16_t tlv_subtype;
    };
};

struct TLVOperator {
    struct TLVStruct *(*decode)(struct TLVDesc *desc, dlist_head *parent, uint8_t **p, uint16_t *len);
    int (*encode)(struct TLVStruct *s, uint8_t **p, uint16_t *len);
    uint16_t (*length)(struct TLVStruct *s);
    void (*free)(struct TLVStruct *s);
};

uint8_t *requestBuf(struct TLVStruct *s, size_t len);
void TLVFree(struct TLV *tlv);
int encodeTLV(struct TLV *tlv, uint8_t **pp, uint16_t *plen);
struct TLV *decodeTLV(uint8_t *p);
struct TLV *TLVNew(dlist_head *parent, uint8_t type, uint32_t size);
struct TLV *TLVRef(struct TLV *tlv);
struct TLVStruct *TLVStructNew(struct TLVDesc *desc, dlist_head *parent, uint32_t size);
int decodeTLVArray(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *plen);
int decodeTLVField(struct TLVFieldDesc * desc, struct TLVStruct * s, uint8_t * * pp, uint16_t * plen);
int encodeTLVArray(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *plen);
int encodeTLVField(struct TLVFieldDesc * desc, struct TLVStruct * s, uint8_t * * pp, uint16_t * plen);
uint16_t getLenTLVOne(struct TLVStruct *s);
struct TLVStruct *decodeTLVOne(struct TLVDesc *desc, dlist_head *parent,
                                       uint8_t **pp, uint16_t *plen);
int encodeTLVOne(struct TLVStruct *s,uint8_t **pp, uint16_t *plen);
uint16_t getLenField(struct TLVStruct *s, struct TLVFieldDesc *f_desc);
#endif
