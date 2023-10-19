/*
 *	bdb_hash.c	( Berkeley DB hashing )
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <db.h>

#include "bdb_hash.h"

#define MEMORY_TEMP_BUFFER_NUMBER	(20)

struct membuf_t {
	char	*ptr;
	int	len;
};

struct bdb_hash_pd_t {
	DB			*dbp;
	DBC			*cursorp;
	int			mbuf_idx;
	struct membuf_t		mbuf[MEMORY_TEMP_BUFFER_NUMBER];
};

static int get_membuf (struct bdb_hash_t *self) {
	struct bdb_hash_pd_t	*pd = self->pd;
	int			rc = pd->mbuf_idx;

	if (pd->mbuf[rc].ptr != NULL) {
		free (pd->mbuf[rc].ptr);
		pd->mbuf[rc].ptr = NULL;
	}

	++pd->mbuf_idx;

	return rc;
}

static void internal_clean_dbt (DBT *ptr) {
	memset (ptr, 0, sizeof (DBT));
	ptr->flags   = DB_DBT_USERMEM;
}

static int internel_set_dbt (struct bdb_hash_t *self,
						DBT *ptr, char * string) {
	int	rc = -1;

	memset (ptr, 0, sizeof (DBT));

	if (string == NULL) {
		ptr->flags = DB_DBT_MALLOC;
		rc = get_membuf (self);
	} else {
		ptr->flags = DB_DBT_USERMEM;
		ptr->data  = string;
		ptr->size  = ptr->ulen = strlen (string) + 1;
	}

	return rc;
}

static int bdb_is_exist (struct bdb_hash_t *self, char *keystr) {
	struct bdb_hash_pd_t	*pd = self->pd;
	DB			*dbp = pd->dbp;
	DBT			key, value;

	if (keystr == NULL) return 0;

	internel_set_dbt (self, &key, keystr);
	internal_clean_dbt (&value);
	value.ulen = 0;
	value.data = NULL;

	if ((dbp->get (dbp, NULL, &key, &value, 0)) == 0) return 1;

	return 0;
}

static char * bdb_get (struct bdb_hash_t *self, char *keystr) {
	struct bdb_hash_pd_t	*pd = self->pd;
	DB			*dbp = pd->dbp;
	DBT			key, value;
	int			rc;

	if (keystr == NULL) return NULL;

	internel_set_dbt (self, &key, keystr);

	rc = internel_set_dbt (self, &value, NULL);

	if ((dbp->get (dbp, NULL, &key, &value, 0)) == 0) {
		pd->mbuf[rc].ptr = value.data;
		pd->mbuf[rc].len = value.size;

		return value.data;
	}

	return NULL;
}

static int bdb_del (struct bdb_hash_t *self, char *keystr) {
	struct bdb_hash_pd_t	*pd = self->pd;
	DB			*dbp = pd->dbp;
	DBT			key;

	if (keystr == NULL) return 0;

	internel_set_dbt (self, &key, keystr);

	if ((dbp->del (dbp, NULL, &key, 0)) == 0) return 1;

	return 0;
}

static int bdb_put (struct bdb_hash_t *self, char *keystr, char *value) {
	DB		*dbp = self->pd->dbp;
	DBT		key, val;

	if ((keystr == NULL) || (value == NULL)) return 0;

	internel_set_dbt (self, &key, keystr);
	internel_set_dbt (self, &val, value);

	if ((dbp->put (dbp, NULL, &key, &val, 0)) == 0) return 1;

	return 0;
}


static int bdb_get_first (struct bdb_hash_t *self,
				char **keystr, char **value) {
	DBC	*cursorp = self->pd->cursorp;
	DBT	key, val;

	internel_set_dbt (self, &key, NULL);
	internel_set_dbt (self, &val, NULL);

	if (cursorp->c_get (cursorp, &key, &val, DB_FIRST) == 0) {
		*keystr = key.data;
		*value  = val.data;
		return 1;
	}

	return 0;
}

static int bdb_get_next (struct bdb_hash_t *self,
				char **keystr, char **value) {
	DBC	*cursorp = self->pd->cursorp;
	DBT	key, val;

	internel_set_dbt (self, &key, NULL);
	internel_set_dbt (self, &val, NULL);

	if (cursorp->c_get (cursorp, &key, &val, DB_NEXT) == 0) {
		*keystr = key.data;
		*value  = val.data;
		return 1;
	}

	return 0;
}

static void bdb_dispose (struct bdb_hash_t *self) {
	int	i;

	for (i = 0; i < MEMORY_TEMP_BUFFER_NUMBER; i++) {
		if (self->pd->mbuf[i].ptr != NULL) {
			free (self->pd->mbuf[i].ptr);

			self->pd->mbuf[i].ptr = NULL;
			self->pd->mbuf[i].len = 0;
		}
	}

	free (self->pd);
	free (self);
}

static char *bdb_version (int *major, int *minor, int *patch) {
	return db_version (major, minor, patch);
}

struct bdb_hash_t * new_bdb_hash (const char *file) {
	struct bdb_hash_t	*self;
	struct bdb_hash_pd_t	*pd;
	DB			*dbp;
	DBC			*cursorp;
	int			i;

	// fprintf (stderr, "%s\n", bdb_version (NULL, NULL, NULL));

	if (db_create (&dbp, NULL, 0) != 0) {
		perror ("DB_create");
		return NULL;
	} else if (dbp->open (dbp, file, NULL, DB_HASH, 0, 0666) != 0) {
		perror ("DB->open");
		dbp->close (dbp, 0);
		return NULL;
	}

	if (dbp->cursor (dbp, NULL, &cursorp, 0) != 0) {
		dbp->close (dbp, 0);
		return NULL;
	}


	while ((self = malloc (sizeof (struct bdb_hash_t))) != NULL) {
		if ((pd = malloc (sizeof (struct bdb_hash_pd_t))) == NULL) {
			free (self);
			self = NULL;
		} else {
			self->pd = pd;
		}

		self->get		= bdb_get;
		self->put		= bdb_put;
		self->del		= bdb_del;
		self->is_exist		= bdb_is_exist;
		self->get_first		= bdb_get_first;
		self->get_next		= bdb_get_next;
		self->dispose		= bdb_dispose;
		self->bdb_version	= bdb_version;

		pd->dbp		= dbp;
		pd->cursorp	= cursorp;
		pd->mbuf_idx	= 0;


		for (i = 0; i < MEMORY_TEMP_BUFFER_NUMBER; i++) {
			pd->mbuf[i].ptr = NULL;
			pd->mbuf[i].len = 0;
		}

		break;
	}

	return self;
}
