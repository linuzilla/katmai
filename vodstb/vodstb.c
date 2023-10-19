/*
 *	vodstb.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdlib.h>
#include "vodstb.h"

static int vodstb_cksum (const void *ptr, const int len) {
	const struct stb_vod_pdu_t	*p = ptr;
	const unsigned char		*q = ptr;
	int				i;
	int				sum = 0;

	for (i = sizeof p->cksum; i < len; i++) {
		sum += q[i];
		sum &= 0xfffffff;
	}

	return sum;
}

struct vodstb_t * new_vodstb (void) {
	struct vodstb_t		*self;

	if ((self = malloc (sizeof (struct vodstb_t))) != NULL) {
		self->cksum	= vodstb_cksum;
	}

	return self;
}
