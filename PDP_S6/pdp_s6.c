/*
 *	pdp_s6.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include "pdp_s6.h"
#include "../rs232/rs232.h"

static int pdps6_open (struct pdp_s6_t *self, const char *device) {
	struct rs232_t	*rs232 = self->pd.rs232;

	if (rs232->open (rs232, device) == 0) {
		perror ("rs232:open");
		return 0;
	}

	self->pd.fd = rs232->fd (rs232);

	rs232->nonblocking (rs232, 1);
	rs232->clear (rs232);

	return 1;
}

static void pdps6_close (struct pdp_s6_t *self) {
	struct rs232_t	*rs232 = self->pd.rs232;

	rs232->close (rs232);
}

struct pdp_s6_t * new_PDP_S6 (void) {
	struct pdp_s6_t *	self;

	if ((self = malloc (sizeof (struct pdp_s6_t))) != NULL) {
		self->open	= pdps6_open;
		self->close	= pdps6_close;

		if ((self->pd.rs232 = new_rs232 ()) == NULL) {
			free (self);
			self = NULL;
		}
	}

	return self;
}
