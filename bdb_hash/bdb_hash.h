/*
 *	bdb_hash.h	( Berkeley DB hashing )
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __BDB_HASH_H__
#define __BDB_HASH_H__

struct bdb_hash_pd_t;

struct bdb_hash_t {
	struct bdb_hash_pd_t	*pd;

	char *		(*bdb_version)(int *major, int *minor, int *patch);
	char *		(*get)(struct bdb_hash_t *, char *key);
	int		(*put)(struct bdb_hash_t *, char *key, char *value);
	int		(*del)(struct bdb_hash_t *self, char *key);
	int		(*is_exist)(struct bdb_hash_t *, char *key);
	int		(*get_first)(struct bdb_hash_t *,
					char **key, char **value);
	int		(*get_next)(struct bdb_hash_t *,
					char **key, char **value);
	void		(*dispose)(struct bdb_hash_t *);
};

struct bdb_hash_t	*new_bdb_hash (const char *file);

#endif
