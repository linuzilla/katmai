/*
 *	genpdu.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include "genpdu.h"

static unsigned int genpdu_cksum (const void *pdu, const int rlen) {
	const struct generic_pdu_t	*p = pdu;
	const unsigned char		*q = pdu;
	int				i;
	unsigned int			sum = 0;
	int				len;


	len = p->len;

	if (rlen >= sizeof (struct generic_pdu_t)) {
		if (rlen != len) return p->cksum + 1;
	}

	for (i = sizeof p->cksum; i < len; i++) {
		sum += q[i];
		sum &= 0xfffffff;
	}

	return sum;
}

static void genpdu_mkcksum (void *pdu) {
	struct generic_pdu_t	*p = pdu;

	p->cksum = genpdu_cksum (pdu, 0);
}

static int genpdu_add (struct genpdu_t *self, const int cmd,
		int (*func)(struct generic_pdu_callback_data_t *ptr)) {
	int			i;
	struct gp_cmd_handler_t	*hdr = self->pd.handler;

	for (i = 0; i < self->pd.real_num; i++) {
		if (hdr[i].cmd == cmd) {
			fprintf (stderr, "[%c] duplicate command\n", cmd);
			return -1;
		}
	}

	if ((i = self->pd.real_num) >= self->pd.num) {
		fprintf (stderr, "Command table full\n");
		return -1;
	}

	++self->pd.real_num;

	hdr[i].cmd  = cmd;
	hdr[i].func = func;

	return i;
}

static int genpdu_addlist (struct genpdu_t *self,
					struct gp_cmd_handler_t *list) {
	int		i;
	int		rc = 0;

	for (i = 0; list[i].func != NULL; i++) {
		if (self->add (self, list[i].cmd, list[i].func) < 0){
			break;
		}
		rc++;
	}

	return rc;
}

static void genpdu_clear (struct genpdu_t *self) {
	self->pd.real_num = 0;
}

static int genpdu_do_cmd (struct genpdu_t *self, void *ptr,
					const int len, void *parm, int *cmd) {
	struct generic_pdu_t	*p = ptr;
	struct gp_cmd_handler_t	*hdr = self->pd.handler;
	int			i;
	unsigned int		cksum;

	cksum = genpdu_cksum (ptr, len);

	if (cksum != p->cksum) {
		fprintf (stderr, "Checksum error !!\n");
		return -1;
	}

	for (i = 0; i < self->pd.real_num; i++) {
		if (hdr[i].cmd == p->cmd) {
			if (cmd != NULL) *cmd = p->cmd;
			return hdr[i].func (parm);
		}
	}

	return -1;
}

static void genpdu_pack (struct genpdu_t *self, void *pdu,
					const int cmd, const int len) {
	struct generic_pdu_t	*p = pdu;

	if (self->pd.flag_callback != NULL) {
		p->flag = self->pd.flag_callback (self);
	}

	p->cmd = cmd;
	p->len = len;

	genpdu_mkcksum (pdu);
}

static void genpdu_set_flag_callback (struct genpdu_t *self,
				unsigned int (*cbk)(struct genpdu_t *)) {
	self->pd.flag_callback = cbk;
}

struct genpdu_t * new_genpdu (const int num) {
	struct genpdu_t			*self;
	struct genpdu_private_data_t	*pd;

	if ((self = malloc (sizeof (struct genpdu_t))) != NULL) {
		self->cksum		= genpdu_cksum;
		self->mkcksum		= genpdu_mkcksum;
		self->add		= genpdu_add;
		self->addlist		= genpdu_addlist;
		self->clear		= genpdu_clear;
		self->do_cmd		= genpdu_do_cmd;
		self->pack		= genpdu_pack;
		self->set_flag_callback	= genpdu_set_flag_callback;

		pd = &self->pd;

		if ((pd->handler = calloc (num,
				sizeof (struct gp_cmd_handler_t))) == NULL) {
			free (self);
			self = NULL;
		} else {
			pd->num			= num;
			pd->real_num		= 0;
			pd->flag_callback	= NULL;
		}
	}

	return self;
}
