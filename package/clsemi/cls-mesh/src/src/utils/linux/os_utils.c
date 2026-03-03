#include "os_utils.h"

struct tlist_item *tlist_alloc(size_t size, dlist_head *parent)
{
    int i;
    tlist_item *t = calloc(1, size);

    if (t) {
        dlist_head_init(&t->l);

        for (i = 0; i < TLIST_MAX_CHILD; i++)
            dlist_head_init(&t->childs[i]);
        dlist_head_init(&t->internal_bufs);

        if (parent)
        {
            dlist_add_tail(parent, &t->l);
        }
    }
    return t;

}


void tlist_delete(dlist_head *list)
{
    tlist_item *item, *tmp_item;

    dlist_for_each_safe(item, tmp_item, *list, l) {
        tlist_delete_item(item);
    }
    dlist_head_init(list);
}

void tlist_delete_item(tlist_item *item)
{
    int i;

    for (i = 0; i < TLIST_MAX_CHILD; i++)
        tlist_delete(&item->childs[i]);
    dlist_free_items(&item->internal_bufs, struct tlist_internal_buffer, l);

    free(item);
}

struct cmdu_buf *cmduBufNew(int size, int reserve)
{
    struct cmdu_buf *b = NULL;
    if (size>0) {
        b = malloc(size+sizeof(struct cmdu_buf));
        if (b) {
            b->data = b->end = b->head;
            b->tail = b->head + size;
            cmduBufReserve(b, reserve);
        }
    }
    return b;
}

void cmduBufFree(struct cmdu_buf *b)
{
    if (b)
        free(b);
}

struct cmdu_buf *cmduReset(struct cmdu_buf *b)
{
    if (b)
        b->data = b->end = b->head;
    return b;
}

void cmduBufPut(struct cmdu_buf *b, int size)
{
    if (b)
        b->end += size;
}

uint8_t *cmduBufPush(struct cmdu_buf *b, int size)
{
    b->data -= size;
    return b->data;
}

uint8_t *cmduBufPop(struct cmdu_buf *b, int size)
{
    b->data += size;
    return b->data;
}

uint8_t *cmduBufReserve(struct cmdu_buf *b, int size)
{
    b->data+=size;
    b->end+=size;
    return b->data;
}

int cmduBufTailRoom(struct cmdu_buf *b)
{
    return (b->tail-b->end);
}


void printbuf(uint8_t *buf, int len)
{
    int i;
    for (i=0;i<len;i++) {
        printf(" %02x", buf[i]);
        if (((i+1) & 15)==0x0)
            printf("\n");
    }
    printf("\n");
}

static int _skipStr(char *str)
{
    if ((*str=='{') || (*str=='}') || (*str==' ') ||(*str==':'))
        return 1;
    if ((*str=='0') && (*(str+1)=='x'))
        return 2;

    if (*str=='\0')
        return -1;
    return 0;
}

static int _toHex(char s)
{
    if ((s>='0') && (s<='9'))
        return (int)s - 0x30;
    else if ((s>='a') && (s<='z'))
        return (int)s - 0x57;
    else if ((s>='A') && (s<='Z'))
        return (int)s - 0x37;
    else
        return -1;
}

int string2Hex(char *str, uint8_t *hex, int len)
{
    int ret = 0, low, high;
    char *p = str;

    if ((!len) || (!str) || (!hex))
        return ret;

    do {
        while ((low = _skipStr(p))>0) {
            p+=low;
        };
        if (low<0)
            break;

        if (((high=_toHex(*p++))>=0) && ((low=_toHex(*p++))>=0))
            hex[ret++]=(uint8_t)((high<<4)|low);
        else
            break;
    } while (ret<len);

    return ret;
}

static char hex2char(uint8_t hex)
{
    if (hex<10)
        return '0'+hex;
    else
        return 'a'-10+hex;
}

int hex2String(uint8_t *hex, char *str, int len)
{
    char *p = str;
    uint8_t value;

    while (len>0) {
        value = (((*hex) & 0xf0) >> 4);
        *(p++) = hex2char(value);
        value = ((*hex++) & 0xf);
        *(p++) = hex2char(value);
        len --;
    };
    return p-str;
}

int listMerge(dlist_head *target, dlist_head *src, listCompareFunc compare,
                listItemNewFunc newl, listItemFreeFunc freel)
{
    int result = 0;
    dlist_item *item_src, *item_target;

    MARK_CLEAR(target);
    dlist_for_each_item(item_src, *src) {
        dlist_for_each_item(item_target, *target) {
            if (!compare(item_target, item_src))
                break;
        }
        if (!item_target) {
            item_target = newl(item_src);
            MARK_NEW(item_target);
            MARK_HIT(item_target);
            dlist_add_tail(target, item_target);
            result = 1;
        } else {
            MARK_HIT(item_target);
        }
    }
    dlist_for_each_item_safe(item_target, item_src, *target) {
        if (!((dlist_item2 *)(item_target))->mark.hit) {
            result = 1;
            dlist_remove(item_target);
            if (freel)
                freel(item_target);
            else
                free(item_target);
        }
    }
    return result;
}
