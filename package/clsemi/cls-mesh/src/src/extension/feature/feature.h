#ifndef _FEATURE_H_
#define _FEATURE_H_

int featInit();
int featSuscribeEvent(uint8_t evt, int (*handler)(void *ctx, uint8_t *m, uint16_t l), void *data);


#endif
