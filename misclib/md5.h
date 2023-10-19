/*
 *	md5.h
 */

#ifndef __MESSAGE_DIGEST_5__H__
#define __MESSAGE_DIGEST_5__H__

#define MD5_DIGEST_LENGTH       16

typedef u_int32_t md5_uint32;

struct md5_ctx {
	md5_uint32	A;
	md5_uint32	B;
	md5_uint32	C;
	md5_uint32	D;

	md5_uint32	total[2];
	md5_uint32	buflen;
	char		buffer[128];
};

void * md5sum (const char *buffer, size_t len, void *resblock);

void md5_process_block (const void *buffer, size_t len, struct md5_ctx *ctx);
void md5_process_bytes (const void *buffer, size_t len, struct md5_ctx *ctx);
void * md5_finish_ctx (struct md5_ctx *ctx, void *resbuf);
void * md5_read_ctx (const struct md5_ctx *ctx, void *resbuf);
void md5_init_ctx (struct md5_ctx *ctx);

#endif
