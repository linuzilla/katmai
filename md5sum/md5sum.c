/*
 *	md5sum.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5sum.h"
#include "md5.h"

static void md5sum_init (struct md5sum_t *self) {
	md5_init_ctx (self->pd.ctx);
	self->pd.finished = 0;
}

static void md5sum_process_bytes (struct md5sum_t *self,
				const void *buf, const unsigned long len) {
	md5_process_bytes (buf, len, self->pd.ctx);
}

static unsigned char * md5sum_finish (struct md5sum_t *self,
						unsigned char *sum) {
	unsigned char	*ptr;

	ptr = &(self->pd.md[0]);

	md5_finish_ctx (self->pd.ctx, ptr);

	if (sum != NULL) {
		memcpy (sum, self->pd.md, sizeof self->pd.md);
		ptr = sum;
	}

	self->pd.finished = 1;

	return ptr;
}

static int md5sum_length (void) { return MD5_DIGEST_LENGTH; }

static char * md5sum_md5 (struct md5sum_t *self, char *str) {
	int	i, j, k;
	char	*ptr = self->pd.msg;

	if (! self->pd.finished) self->finish (self, NULL);

	for (i = j = 0; i < MD5_DIGEST_LENGTH; i++) {
		k = sprintf (&self->pd.msg[j], "%02x", self->pd.md[i]);
		j += k;
	}

	if (str != NULL) {
		strcpy (str, self->pd.msg);
		ptr = str;
	}

	return ptr;
}

static void md5sum_dispose (struct md5sum_t *self) {
	free (self->pd.ctx);
	free (self);
}

struct md5sum_t * new_md5sum (void) {
	struct md5sum_t	*self;

	if ((self = malloc (sizeof (struct md5sum_t))) != NULL) {
		self->init	= md5sum_init;
		self->process	= md5sum_process_bytes;
		self->finish	= md5sum_finish;
		self->md5	= md5sum_md5;
		self->dispose	= md5sum_dispose;
		self->length	= md5sum_length;

		// fprintf (stderr, "Process(%d)\n", self->process);

		if ((self->pd.ctx = malloc (sizeof (struct md5_ctx))) == NULL) {
			free (self);
			self = NULL;
		} else {
			self->init (self);
		}
	}

	return self;
}
