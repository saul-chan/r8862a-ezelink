#ifndef SM3_H
#define SM3_H

#define SM3_DIGEST_LENGTH	32
#define SM3_BLOCK_SIZE		64

#include <stdint.h>
#include <string.h>

#define cpu_to_be16(v) (((v)<<8) | ((v)>>8))
#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))
#define be16_to_cpu(v) cpu_to_be16(v)
#define be32_to_cpu(v) cpu_to_be32(v)

struct sm3_ctx_t {
	uint32_t digest[8];
	int nblocks;
	unsigned char block[64];
	int num;
};

void sm3_init(struct sm3_ctx_t *ctx);
void sm3_update(struct sm3_ctx_t *ctx, const unsigned char *data, size_t data_len);
void sm3_final(struct sm3_ctx_t *ctx, unsigned char *digest);
void sm3_compress(uint32_t *digest, const unsigned char *block);
unsigned char *SM3(const unsigned char *data, size_t datalen, unsigned char *digest);

#endif

