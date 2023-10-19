/*
 *	stio_eng.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __CLIENT_STIO_ENG__C__
#define __CLIENT_STIO_ENG__C__

static ssize_t steng_read (struct stio_engine_t *self,
					void *buffer, size_t len) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;
	char					*ptr = buffer;
	int					rlen, explen, tlen;

	// return stiov2->read (stiov2, cl->channel, buffer, len);

	// if ((fd = self->pd.fd) < 0) return -1;

	explen = len;
	tlen   = 0;

	while (explen > 0) {
		if ((rlen = stiov3->read (stiov3, cl->channel,
						&ptr[tlen], explen)) <= 0) {
			break;
		}
		tlen   += rlen;
		explen -= rlen;
	}

	return tlen;
}

static int steng_open (struct stio_engine_t *self, const char *fname) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;

	return stiov3->openfile (stiov3, cl->channel, fname);
}

static int steng_reset (struct stio_engine_t *self) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;

	// fprintf (stderr, "steng_reset\n");

	return stiov3->reopen (stiov3, cl->channel);
}

static int steng_seek (struct stio_engine_t *self, const off_t pos) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;

	// fprintf (stderr, "steng_reset\n");

	return stiov3->seek (stiov3, cl->channel, pos);
}

static void steng_close (struct stio_engine_t *self) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;

	stiov3->closefile (stiov3, cl->channel);
}

static void steng_abort (struct stio_engine_t *self) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;

	// fprintf (stderr, "steng_abort [%p]\n", self);

	stiov3->abort (stiov3);
	self->close (self);
}

static off_t steng_filesize (struct stio_engine_t *self) {
	struct stio_v3_client_local_t		*cl = self->pd.myself;
	struct streamming_io_v3_client_t	*stiov3 = cl->parent;

	return stiov3->filesize (stiov3, cl->channel);
}

static void steng_dispose (struct stio_engine_t *self) {
	self->close (self);
	// free (self);
}

static int cli_regist_stio (struct streamming_io_v3_client_t *self,
				const int channel,
				struct stio_engine_t *stio) {
	struct stio_v3_client_local_t	*cl;

	if ((channel < 0) || (channel >= MAX_STIOV3_CHANNEL)) return -1;

	cl = &self->pd.stio[channel];

	// memcpy (cl->save_stio, stio, sizeof (struct stio_engine_t));

	cl->save_stio = stio;

	stio->close (stio);

	stio->pd.myself	= cl;

	stio->pd.open		= steng_open;
	stio->pd.read		= steng_read;
	stio->pd.close		= steng_close;
	stio->pd.abort		= steng_abort;
	stio->pd.filesize	= steng_filesize;
	stio->pd.dispose	= steng_dispose;
	stio->pd.reset		= steng_reset;
	stio->pd.seek		= steng_seek;

	return 1;
}

#endif
