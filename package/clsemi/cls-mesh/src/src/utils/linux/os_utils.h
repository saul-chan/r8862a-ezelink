#ifndef _OS_UTILS_H
#define _OS_UTILS_H

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define BCMAC       "\xff\xff\xff\xff\xff\xff"
#define ZEROMAC     "\x00\x00\x00\x00\x00\x00"

#define MACLEN  (6)
#define MACCPY(dst,src)  memcpy((dst), (src), MACLEN)
#define MACCMP(dst,src)  (memcmp((dst), (src), MACLEN))

#define OUILEN  (3)
#define OUICPY(dst,src)  memcpy((dst), (src), OUILEN)
#define OUICMP(dst,src)  (memcmp((dst), (src), OUILEN))


#define IS_ZERO_MAC(mac) ((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) == 0x00)
#define IS_WILDCARD_MAC(mac) ((mac[0] & mac[1] & mac[2] & mac[3] & mac[4] & mac[5]) == 0xff)

#define MACFMT  "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]

#define IS_ZERO_MAC(mac) ((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) == 0x00)
#define IS_WILDCARD_MAC(mac) ((mac[0] & mac[1] & mac[2] & mac[3] & mac[4] & mac[5]) == 0xff)

#define MIN(_a, _b) ((_a) > (_b) ? (_b) : (_a))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))
#endif

#define is_types_compatible(obj, other_obj) \
    ((long)"type is incompatible" ? (obj) : (other_obj))

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)is_types_compatible(ptr, &((type*)ptr)->member) - offsetof(type, member)))
#endif

#define REPLACE_STRUCT(_old, _new, _free_func) \
    do {\
        if ((_old)) {(_free_func)((_old));}\
        (_old) = (_new); \
    }while(0)

#define REPLACE_STR(_value, _newstr) REPLACE_STRUCT(_value, _newstr, free)

#define CHECK_DELETE(_value, _delete_func) \
    if ((_value)) (_delete_func)((_value))

#define CHECK_FREE(_value) \
    CHECK_DELETE(_value, free)

#define CMDUBUF_PUT_DATA(buffer, payload, len)\
    do {\
        memcpy((buffer)->end, (payload), (len));\
        cmduBufPut((buffer), (len));\
    }while(0)

#define BIT(_shift) (1<<(_shift))
#define BIT_IS_SET(_value, _shift)  ((_value) & (1 << (_shift)))
#define SET_BIT(_value, _shift)     ((_value) |= (1 << (_shift)))
#define CLR_BIT(_value, _shift)     ((_value) &= ~(1 << (_shift)))
#define GET_BITS(_value, _shift0, _shift1) ((((1<<((_shift1)+1))-1) & (_value)) >> (_shift0))
#define GET_BIT(_value, _shift)     GET_BITS((_value), (_shift), (_shift))

#define SET_BITS(_value, _v, _mask, _shift) \
    do { \
        (_value) &= (~((_mask) << (_shift))); \
        (_value) |= (((_v) & (_mask)) << (_shift)); \
    }while(0)


typedef uint8_t mac_address[MACLEN];


#define TLIST_MAX_CHILD  4

typedef struct dlist_head {
    struct dlist_head *next;
    struct dlist_head *prev;
} dlist_head;

typedef dlist_head dlist_item;

struct mark {
    uint8_t hit:1;
    uint8_t new:1;
    uint8_t change:1;
};

typedef struct dlist_item2 {
    dlist_item l;
    struct mark mark;
} dlist_item2;


struct tlist_internal_buffer {
    dlist_item l;
    uint8_t data[0];
};

typedef struct tlist_item {
    dlist_item l;
    struct dlist_head childs[TLIST_MAX_CHILD];
    struct dlist_head internal_bufs;
} tlist_item;

static inline void dlist_head_init(dlist_head *head)
{
    head->next = head;
    head->prev = head;
}

#define DEFINE_DLIST_HEAD(name) dlist_head name = {&name, &name}

static inline void dlist_add_head(dlist_head *list, dlist_item *item)
{
    item->next = list->next;
    item->prev = list;
    list->next->prev = item;
    list->next = item;
}

static inline void dlist_add_tail(dlist_head *list, dlist_item *item)
{
    dlist_add_head(list->prev, item);
}


static inline bool dlist_empty(const dlist_head *list)
{
    return list->next == list;
}


static inline dlist_item *dlist_get_first(const dlist_head *list)
{
    if (list->next == list) {
        return NULL;
    } else {
        return list->next;
    }
}

static inline dlist_item *dlist_get_last(const dlist_head *list)
{
    if (list->prev == list) {
        return NULL;
    } else {
        return list->prev;
    }
}
static inline dlist_item *dlist_get_next(dlist_head *list, dlist_item *item)
{
    return (item->next == list) ? NULL : item->next;
}

#define dlist_for_each(item, head, dlist_member) \
    for ((item) = container_of((head).next, typeof(*(item)), dlist_member); \
         (&(item)->dlist_member != &(head)) || (((item) = NULL), false); \
         (item) = container_of((item)->dlist_member.next, typeof(*(item)), dlist_member))

#define dlist_for_each_safe(item, n, head, dlist_member) \
    for ((item) = container_of((head).next, typeof(*(item)), dlist_member), \
         n = container_of((item)->dlist_member.next, typeof(*(item)), dlist_member); \
         (&(item)->dlist_member != &(head)) || (((item) = NULL), false); \
         (item) = n, n = container_of((n)->dlist_member.next, typeof(*(item)), dlist_member))

#define dlist_for_each_item(item, head) \
    for ((item) = (head).next; \
         ((item) != &(head)) || (((item) = NULL), false); \
         (item) = (item)->next)

#define dlist_for_each_item_safe(item, n, head) \
    for ((item) = (head).next, n = (item)->next; \
         ((item) != &(head)) || (((item) = NULL), false); \
         (item) = (n), (n) = (item)->next)


static inline unsigned dlist_count(const dlist_head *list)
{
    size_t count = 0;
    dlist_item *item;
    dlist_for_each_item(item, *list) {
        count++;
    }
    return count;
}

static inline void dlist_remove(dlist_item *item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    dlist_head_init(item);
}

#define dlist_free_items(head, type, dlist_member) do { \
    dlist_item *_item;  type *_value;\
    while (NULL != (_item = dlist_get_first(head))) {     \
        dlist_remove(_item);        \
        _value = container_of(_item, type, dlist_member); \
        free(_value);   \
    } } while (0)

#define tlist_for_each(item, head, type, tlist_member) \
    for ((item) = container_of((head).next, type, tlist_member.l); \
            &(item)->tlist_member.l != &(head); \
            (item) = container_of((item)->tlist_member.l.next, type, tlist_member.l))

struct tlist_item *tlist_alloc(size_t size, dlist_head *parent);
void tlist_delete(dlist_head *list);
void tlist_delete_item(tlist_item *item);

int string2Hex(char *str, uint8_t *hex, int len);
int hex2String(uint8_t *hex, char *str, int len);


struct cmdu_buf {
    dlist_item l;
    uint8_t *tail;
    uint8_t *data;
    uint8_t *end;
    uint8_t head[0];
};

struct cmdu_buf *cmduBufNew(int size, int reserve);
struct cmdu_buf *cmduReset(struct cmdu_buf *b);
void cmduBufFree(struct cmdu_buf *b);
void cmduBufPut(struct cmdu_buf *b, int size);
uint8_t *cmduBufPush(struct cmdu_buf *b, int size);
uint8_t *cmduBufPop(struct cmdu_buf *b, int size);
uint8_t *cmduBufReserve(struct cmdu_buf *b, int size);
int cmduBufTailRoom(struct cmdu_buf *b);
#define cmduBufLen(_buf) ((_buf)->end-(_buf)->data)
void printbuf(uint8_t *buf, int len);


#define MARK_CLEAR(_head) \
    do {\
        dlist_item *item;\
        dlist_for_each_item(item, *_head) {\
            dlist_item2 *item2 = (dlist_item2 *)item;\
            item2->mark.new = item2->mark.hit = 0;\
        }\
    }while(0)
#define MARK_NEW(item_) \
    do {\
        ((dlist_item2 *)(item_))->mark.new = 1;\
    }while(0)
#define MARK_HIT(item_) \
    do {\
        ((dlist_item2 *)(item_))->mark.hit = 1;\
    }while(0)
typedef int (*listCompareFunc)(dlist_item *, dlist_item *);
typedef dlist_item *(*listItemNewFunc)(dlist_item *);
typedef void (*listItemFreeFunc)(dlist_item *);
int listMerge(dlist_head *target, dlist_head *src, listCompareFunc compare,
                listItemNewFunc newl, listItemFreeFunc freel);

#endif

