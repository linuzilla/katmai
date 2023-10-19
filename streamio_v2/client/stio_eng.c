/*
 *	stio_eng.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __CLIENT_STIO_ENG__C__
#define __CLIENT_STIO_ENG__C__

static ssize_t steng_read (struct stio_engine_t *self,
					void *buffer, size_t len) {
	struct stio_v2_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v2_client_t	*stiov2 = cl->parent;
	char					*ptr = buffer;
	int					rlen, explen, tlen;

	// return stiov2->read (stiov2, cl->channel, buffer, len);

	// if ((fd = self->pd.fd) < 0) return -1;

	explen = len;
	tlen   = 0;

	while (explen > 0) {
		if ((rlen = stiov2->read (stiov2, cl->channel,
						&ptr[tlen], explen)) <= 0) {
			break;
		}
		tlen   += rlen;
		explen -= rlen;
	}

	return tlen;
}

static int steng_open (struct stio_engine_t *self, const char *fname) {
	struct stio_v2_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v2_client_t	*stiov2 = cl->parent;

	return stiov2->openfile (stiov2, cl->channel, fname);
}

static int steng_reset (struct stio_engine_t *self) {
	struct stio_v2_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v2_client_t	*stiov2 = cl->parent;

	// fprintf (stderr, "steng_reset\n");

	return stiov2->reopen (stiov2, cl->channel);
}

static void steng_close (struct stio_engine_t *self) {
	struct stio_v2_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v2_client_t	*stiov2 = cl->parent;

	stiov2->closefile (stiov2, cl->channel);
}

static void steng_abort (struct stio_engine_t *self) {
	struct stio_v2_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v2_client_t	*stiov2 = cl->parent;

	// fprintf (stderr, "steng_abort [%p]\n", self);

	stiov2->abort (stiov2);
	self->close (self);
}

static off_t steng_filesize (struct stio_engine_t *self) {
	struct stio_v2_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v2_client_t	*stiov2 = cl->parent;

	return stiov2->filesize (stiov2, cl->channel);
}

static void steng_dispose (struct stio_engine_t *self) {
	self->close (self);
	// free (self);
}

static int cli_regist_stio (struct streamming_io_v2_client_t *self,
				const int channel,
				struct stio_engine_t *stio) {
	struct stio_v2_client_local_t	*cl;

	if ((channel < 0) || (channel >= MAXIMUM_STIOV2_SERVICES)) return -1;

	cl = &self->pd.cl[channel];

	memcpy (&cl->save_stio, stio, sizeof (struct stio_engine_t));

	stio->close (stio);

	stio->pd.myself	= cl;

	stio->pd.open		= steng_open;
	stio->pd.read		= steng_read;
	stio->pd.close		= steng_close;
	stio->pd.abort		= steng_abort;
	stio->pd.filesize	= steng_filesize;
	stio->pd.dispose	= steng_dispose;
	stio->pd.reset		= steng_reset;

	return 1;
}

#endif
