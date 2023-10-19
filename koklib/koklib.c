/*
 *	koklib.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include "koklib.h"
#include "stio_engine.h"

static void kok_dispose (struct koklib_t *self) {
	struct stio_engine_t	*stio = self->pd.stio;

	self->close (self);
	self->cleanup (self);
	if (self->pd.banner != NULL) free (self->pd.banner);
	stio->dispose (stio);
	free (self);
}

static void kok_cleanup (struct koklib_t *self) {
	struct koklib_pd_t	*pd = &self->pd;

	if (pd->lptr != NULL) {
		// fprintf (stderr, "KOK Cleanup\n");
		free (pd->lptr);
		pd->lptr = NULL;
	}
}

static void kok_close (struct koklib_t *self) {
	struct stio_engine_t	*stio = self->pd.stio;

	stio->close (stio);
}

static struct koklib_hdr_ds_t * kok_header (struct koklib_t *self) {
	return &self->pd.header;
}

static void kok_reset (struct koklib_t *self) {
	struct koklib_pd_t	*pd = &self->pd;

	pd->lidx = 0;
	pd->bidx = 0;
	pd->idx[0] = pd->idx[1] = 0;
}

static int kok_load (struct koklib_t *self, const char *fname) {
	struct koklib_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio = pd->stio;
	int			i;
	
	self->cleanup (self);
	self->close (self);

	if (stio->open (stio, fname) <= 0) {
		perror (fname);
		return -1;
	}

	pd->lidx = 0;
	pd->bidx = 0;
	pd->idx[0] = pd->idx[1] = 0;

	i = stio->read (stio, &pd->header, sizeof pd->header);

	//	fprintf (stderr, "KOKLIB: %d bytes read (expect: %d)\n",
	//				i, sizeof pd->header);

	pd->header.song[sizeof pd->header.song - 1] = '\0';
	pd->header.singer[sizeof pd->header.singer - 1] = '\0';

	// fprintf (stderr, "Song Title:[%s]\n", pd->header.song);
	// fprintf (stderr, "Singer    :[%s]\n", pd->header.singer);
	// fprintf (stderr, "Para      :[%d]\n", pd->header.lyricspara);
	// fprintf (stderr, "Version   :[%d]\n", pd->header.version);

	pd->lptr = calloc (pd->header.lyricspara,
			sizeof (struct koklib_lyricspara_t));

	if (pd->lptr == NULL) return -1;

	for (i = 0; i < pd->header.lyricspara; i++) {
		stio->read (stio, &pd->lptr[i],
				sizeof (struct koklib_lyricspara_t));
	}

	// fprintf (stderr, "Read banner for %d bytes\n", pd->banner_size);
	stio->read (stio, pd->banner, pd->banner_size);

	return pd->header.lyricspara;
}

static void *  kok_get_banner (struct koklib_t *self,
			int *width, int *height, int *rwidth) {
	*width  = self->pd.banner_width;
	*height = KOK_BANNER_IMAGE_HEIGHT;
	*rwidth = KOK_BANNER_IMAGE_WIDTH;

	return self->pd.banner;
}

static int kok_read_banner (struct koklib_t *self, char *bitmap) {
	struct koklib_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio = pd->stio;
	int			img_h = KOK_BANNER_IMAGE_HEIGHT;
	int			img_w = KOK_BANNER_IMAGE_WIDTH;
	int			aa, len, widthArray;

	aa = img_w / 8 * 200;

	if (aa % 800 == 0) {
		widthArray = img_w / 8;
	} else {
		widthArray = (((int)(img_w / 8))/4 + 1) * 4;
	}

	len = widthArray * img_h;

	if (bitmap != NULL) stio->read (stio, bitmap, len);

	self->pd.banner_width = widthArray;

	return len;
}

static int kok_read_bitmap (struct koklib_t *self, struct koklib_bitmap_t *p) {
	struct koklib_pd_t		*pd = &self->pd;
	struct stio_engine_t		*stio = pd->stio;
	int				i = pd->bidx;
	struct koklib_lyricspara_t	*lp = &pd->lptr[i];
	int				aa;
	int				widthArray;
	int				len;
	int				realsizes;
	int				length;

	if (p == NULL) return -1;
	if (pd->bidx >= pd->header.lyricspara) return -1;

	realsizes = pd->header.realsizes;
	length = lp->length;

	aa = 0.5 * length * realsizes / 8 * 200;

	if (aa % 800 == 0) {
		widthArray = 0.5 * length * realsizes / 8;
	} else {
		widthArray = (((int)(0.5 * length * realsizes / 8))/4 + 1) * 4;
	}

	len = widthArray * realsizes;

#if 0
	if ((p->size > 0) && (p->bmp != NULL)) {
		if (p->size < len) {
			p->size = 0;
			free (p->bmp);
		}
	} else {
		p->size = 0;
	}

	if (p->size == 0) {
		if ((p->bmp = malloc (len)) == NULL) return -1;
		p->size = len;
	}
#endif
	if ((p->bmp = malloc (len)) == NULL) return -1;
	p->size = len;

	stio->read (stio, p->bmp, len);

	p->real_width = length * realsizes / 2;
	p->width  = widthArray;
	p->height = realsizes;

	pd->bidx++;

	return 1;
}

static struct koklib_lyricspara_t * kok_get (struct koklib_t *self,
					const int n, int *pos) {
	struct koklib_pd_t	*pd = &self->pd;
	int			i, j;

	// fprintf (stderr, "kok_get %d %d\n", n, *pos);

	if (pd->lptr == NULL) return NULL;

	for (i = (n == 0) ? 0 : 1;
				pd->idx[i] < pd->header.lyricspara;
				pd->idx[i]++) {
		// fprintf (stderr, "%d\n", i);
		if (pd->lptr[pd->idx[i]].updown == i) {
			*pos = j = pd->idx[i]++;
			return &pd->lptr[j];
		}
	}
	return NULL;
}

static int kok_read_para (struct koklib_t *self) {
	struct koklib_pd_t		*pd = &self->pd;
	struct koklib_lyricspara_t	*lp;
	int				len;
	// int				i;

	if (pd->lptr == NULL) return -1;
	if (pd->lidx >= pd->header.lyricspara) return -1;

	lp = &pd->lptr[pd->lidx++];
	lp->sentence[sizeof lp->sentence - 1] = '\0';
	/*
	fprintf (stderr, "(l=%d,L=%2d,D=%d,A=%d,S=%d,O=%3d,u=%d) %s\n",
			lp->leadtime,
			lp->length,
			lp->division,
			lp->alignment,
			lp->singer,
			lp->xoffset,
			lp->updown,
			lp->sentence
		);
	for (i = 0; i <= lp->division; i++) {
		fprintf (stderr, "[%d,%3d]", lp->jump[i], lp->time[i]);
	}
	fprintf (stderr, "\n");
	*/

	return len;
}

static struct stio_engine_t * kok_stio_engine (struct koklib_t *self) {
	return self->pd.stio;
}

static int * kok_get_RGB (struct koklib_t *self, const int flag) {
	if (flag == 0) {
		return self->pd.header.orgb;
	} else {
		return self->pd.header.rgb;
	}
}

struct koklib_t * new_koklib (void) {
	struct koklib_t		*self;
	struct koklib_pd_t	*pd;

	while ((self = malloc (sizeof (struct koklib_t))) != NULL) {
		pd = &self->pd;

		if ((pd->stio = new_stio_engine ()) == NULL) {
			free (self);
			break;
		}

		pd->lidx = 0;
		pd->bidx = 0;
		pd->idx[0] = pd->idx[1] = 0;
		pd->lptr = NULL;
		pd->banner = NULL;
		pd->banner_size = 0;
		pd->banner_width = 0;

		self->dispose	= kok_dispose;
		self->header	= kok_header;
		self->reset	= kok_reset;
		self->load	= kok_load;
		self->close	= kok_close;
		self->get	= kok_get;
		self->get_RGB	= kok_get_RGB;
		self->read_para	= kok_read_para;
		self->read_banner = kok_read_banner;
		self->read_bitmap = kok_read_bitmap;
		self->get_banner = kok_get_banner;
		self->cleanup	= kok_cleanup;
		self->stio_engine = kok_stio_engine;

		pd->banner_size = self->read_banner (self, NULL);

		if ((pd->banner = malloc (pd->banner_size)) == NULL) {
			self->dispose (self);
			return NULL;
		}
		break;
	}

	return self;
}
