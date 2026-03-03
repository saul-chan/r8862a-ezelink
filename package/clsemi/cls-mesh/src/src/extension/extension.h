#ifndef _EXTENSION_H_
#define _EXTENSION_H
#include "os_utils.h"
#include "datamodel.h"

/** @brief extension ops */
struct extension_ops {
    void *(*init)(void);
    int (*start)(void *, char *);
    void (*stop)(void *);
    void (*deinit)(void *);
};



int registerExtension(struct extension_ops *ops);
void emulateExtensions();
void startExtensions();
void loadExtensions();
#endif

