#ifndef _EXTENSION_LIST_H
#define _EXTENSION_LIST_H

//add all extensions below
extern void clsDriverLoad();
extern void uciDriverLoad();
extern void topologyFeatLoad();
extern void ubusFeatLoad();
extern void rcpiSteerFeatLoad();
#ifdef CONFIG_VBSS
extern void vbssFeatLoad();
#endif
extern void vendorALoad();

struct _extension {
    void (*entry)(void);
};

static struct _extension g_exts[] = {
    {clsDriverLoad},
#ifdef USE_UCI
    {uciDriverLoad},
#endif
    {topologyFeatLoad},
    {ubusFeatLoad},
    {rcpiSteerFeatLoad},
#ifdef CONFIG_VBSS
    {vbssFeatLoad},
#endif
#ifdef VENDORA_CONFIG
    {vendorALoad},
#endif
    {NULL},
};

#endif
