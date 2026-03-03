#ifndef _NARRAY_H_
#define _NARRAY_H_

#include <stdbool.h>


#define NARRAY_DEFAULT_LEN (2)

#define NARRAY(type) \
    struct { \
        uint16_t len; \
        uint16_t t_len; \
        type *p; \
    }

#define NARRAY_ADD(narray, item) \
    do { \
        uint16_t index; \
        for (index = 0; index < (narray).len; index++) { \
            if ((narray).p[index] == (item)) \
                break; \
        }\
        if (index<((narray).len)) { \
            break;\
        }\
        if ((narray).len>=(narray).t_len) { \
            if (!((narray).t_len)) (narray).t_len = NARRAY_DEFAULT_LEN; \
            (narray).t_len <<=1; \
            (narray).p = realloc((narray).p, ((narray).t_len) * sizeof(*(narray).p)); \
        } \
        (narray).p[(narray).len] = item; \
        (narray).len++; \
    } while (0)


#define NARRAY_DELETE(narray, item) \
    do { \
        uint16_t index; \
        for (index = 0; index < (narray).len; index++) { \
            if ((narray).p[index] == (item)) \
            break; \
        } \
        if (index<((narray).len)) { \
            (narray).len--;\
            memmove((narray).p + index, (narray).p + (index) + 1, \
                    ((narray).len - (index)) * sizeof(*(narray).p)); \
        } \
    } while (0)


#define NARRAY_CLEAR(narray) \
    do { \
        if ((narray).p) \
        free((narray).p); \
        (narray).p = NULL; \
        (narray).len = (narray).t_len = 0; \
    } while(0)

#endif
