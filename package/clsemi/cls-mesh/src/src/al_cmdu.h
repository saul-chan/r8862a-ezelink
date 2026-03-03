#ifndef _AL_CMDU_H_
#define _AL_CMDU_H_

uint8_t cmdu2AddRetry(struct CMDU2 *c, char *interface_name, uint8_t *dst);
void checkRetry(struct CMDU2 *c, uint8_t *src);
int checkAck(struct CMDU2 *c, uint8_t *src);

#endif
