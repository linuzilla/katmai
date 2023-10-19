/*
 *	md5sum.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __MD5SUM_H__
#define __MD5SUM_H__

#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH	16
#endif

struct md5sum_private_data_t {
	struct md5_ctx		*ctx;
	unsigned char		md[MD5_DIGEST_LENGTH];
	char			msg[MD5_DIGEST_LENGTH * 2 + 1];
	int			finished;
};

struct md5sum_t {
	struct md5sum_private_data_t	pd;
	void		(*init)(struct md5sum_t *);
	void		(*process)(struct md5sum_t *self,
				const void *buf, const unsigned long len);
	unsigned char *	(*finish)(struct md5sum_t *,
				unsigned char *md5);
	char *		(*md5)(struct md5sum_t *, char *str);
	void		(*dispose)(struct md5sum_t *self);
	int		(*length)(void);
};

struct md5sum_t * new_md5sum (void);

#endif
