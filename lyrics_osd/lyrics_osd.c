/*
 *	lyrics_osd.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "lyrics_osd.h"
#include "xtimer.h"
#include "koklib.h"
#include "rmosdlib.h"
#include "rmimage256.h"
#include "stio_engine.h"

#define ROUND(x) ((int)((x) + 0.5f))

static const int	img_w = 608;
static const int	img_h = 126;

struct rmimgYUV_t {
	unsigned char	blend;
	unsigned char	Y;
	unsigned char	U;
	unsigned char	V;
};

// 5,214,5
#define RMIMGYUV_BLUE		{ 0xff, 0x28, 0xef, 0x6d }
#define RMIMGYUV_SKYBLUE	{ 0xff, 0xef, 0xef, 0 }
#define RMIMGYUV_ORANGE		{ 0xff, 0xef, 0x10, 0xef }
#define RMIMGYUV_GREEN		{ 0xff, 159,17,24 }
#define RMIMGYUV_LIGHTGREEN	{ 0xff, 0xef, 0x10, 0x10 }
#define RMIMGYUV_RED		{ 0xff, 128,10,243 }
#define RMIMGYUV_WHITE		{ 0xff, 234,128,128 }
#define RMIMGYUV_BLACK		{ 0xff, 3,128,128 }
#define RMIMGYUV_BLACK2		{ 0xff, 3,128,128 }
#define RMIMGYUV_PINK		{ 0xff, 0xef, 0xef, 0xef }

static struct rmimgYUV_t	my_palette[] = {
	{ 0x00,    0,    0,    0 },

	RMIMGYUV_WHITE,
	RMIMGYUV_SKYBLUE,
	RMIMGYUV_PINK,
	RMIMGYUV_LIGHTGREEN,

	RMIMGYUV_BLUE,
	RMIMGYUV_BLUE,
	RMIMGYUV_RED,
	RMIMGYUV_GREEN,

	RMIMGYUV_BLACK,
	RMIMGYUV_BLACK,
	RMIMGYUV_BLACK,
	RMIMGYUV_BLACK,

	RMIMGYUV_WHITE,
	RMIMGYUV_WHITE,
	RMIMGYUV_WHITE,
	RMIMGYUV_BLACK2,

	{ 0xff, 0xd5, 0x7f, 0x7f },	// White	(7)
	{ 0xff, 0x28, 0xef, 0x6d },	// Blue		(4)
	{ 0xff, 0x6a, 0xca, 0xdd },	// Purple	(5)
	{ 0xff, 0x90, 0x35, 0x22 },     // green1	(2)

	{ 0xff,   72,  241,   86 },	// Blue
	{ 0xff, 0xa9, 0xa5, 0x0f },	// green2	(3)
	{ 0xff, 0xea, 0x7f, 0x7f },	// White	(6)
	{ 0xff, 0x34, 0xa8, 0x8b },     // blue2	(20)

	{ 0xff, 0x10, 0x80, 0x80 },	// Black
	{ 0xff, 0xca, 0x14, 0x93 },     // yellow	(17)
	{ 0xff, 0xd5, 0x7f, 0x7f },	// White	(7)
	{ 0xff, 0x28, 0xef, 0x6d },	// Blue		(4)

	{ 0xff, 0xea, 0x7f, 0x7f },	// White	(6)
	{ 0xff, 0x34, 0xa8, 0x8b },     // blue2	(20)
	{ 0xff, 0x6a, 0xca, 0xdd },	// Purple	(5)
	{ 0xff, 0x90, 0x35, 0x22 },     // green1	(2)

	{ 0xff,   72,  241,   86 },	// Blue
	{ 0xff, 0xa9, 0xa5, 0x0f },	// green2	(3)
	{ 0xff, 0x34, 0xa8, 0x8b },     // blue2	(20)
	{ 0xff, 0xca, 0x14, 0x93 }      // yellow	(17)
};

static void lyr_set_my_palette (const int singer, const int channel,
				const int yy, const int uu, const int vv) {
	int	i;

	if ((singer  < 0) || (singer  >= 4)) return;
	if ((channel < 0) || (channel >= 4)) return;

	i = singer * 4 + channel + 1;

	my_palette[i].Y = yy % 256;
	my_palette[i].U = uu % 256;
	my_palette[i].V = vv % 256;

	fprintf (stderr, "PALETTE: [%d,%d,%d,%d,%d]\n", singer, channel,
			my_palette[i].Y,
			my_palette[i].U,
			my_palette[i].V);
}

static void * lyrsvc_main (void *selfptr) {
	struct lyrics_osd_t		*self = selfptr;
	struct lyrics_osd_pd_t		*pd   = &self->pd;
	struct koklib_t			*kok  = pd->kok;
	struct koklib_bitmap_t		*kbmp = pd->kbmp;
	struct rmosdlib_t		*osd  = pd->osd;
	struct xtimer_t			*xtm  = pd->xtm;
	struct rmimage256_t		*img  = pd->img;
	struct rmimage256_t		*banner = pd->banner;
	int				i, np;
	struct koklib_lyricspara_t	*lyr[2];
	int				kidx[2];
	int				state[2] = { 0, 0 };
	int				tm;
	int				update, clear, cls[2];
	//const int			xx[2] = { 10, 10 };
	// const int			yy[2] = { 10, 80 };
	const int			bw = 3;
	int				last_w[2] = { 0, 0 };
	int				last_h[2] = { 0, 0 };
	const int			org_x[2]   = {  4,  4 };
	int				start_x[2] = {  0,  0 };
	int				start_y[2] = {  3, 63 };
	int				singer[2] = { 0, 0 };

	const static int		fcolor[4][2] = {
							{    1,  5 },
							{    2,  6 },
							{    3,  7 },
							{    4,  8 }
					};
	const static int		bcolor[4][2] = {
							{    9, 13 },
							{   10, 14 },
							{   11, 15 },
							{   12, 16 }
					};
	const static int		convmap[2][4][2] = {
						{
							{  9,  1 },
							{ 10,  2 },
							{ 11,  3 },
							{ 12,  4 }
						},
						{
							{ 13,  5 },
							{ 14,  6 },
							{ 15,  7 },
							{ 16,  8 }
						}
					};

	int				from_x[2], to_x[2];
	int				division[2] = { 0 , 0 };
	int				ii[2] = { 0, 0 };
	struct koklib_hdr_ds_t		*hdr;
	int				fw;
	int				range[2][2] = { {0, 0}, {0, 0}};
	int				step[2] = { 0, 0 };
	int				tcnt = 0;
	int				lstop[2] = { 0, 0 };
	int				init_step = 20;
	const int			prepare_time = 280;
	// const int			prepare_time = 220;
	const int			old_lyr_clear_time = 20; // 2 sec
	void				*banner_location;
	int				banner_width;
	int				banner_height;
	int				banner_real_width;
	int				title_color = 4;
	const double			osd_constant = 3.1;
	// const double			osd_constant = 4.2;

	if (pd->use_default_rgb) {
		for (i = 0; i < sizeof my_palette / sizeof (
						struct rmimgYUV_t); i++) {
			img->setPaletteColor (img, i, (char *) &my_palette[i]);
		}
	} else {
		int	j, k, ixi, *rgb;
		int	bw_rgb[2];

		for (j = 0; j < 2; j++) {
			rgb = kok->get_RGB (kok, j);

			// common(b,w), boy, girl, mix
			for (i = ixi = 0; i < 4; i++, ixi += 2) {
				bw_rgb[0] = rgb[ixi];
				bw_rgb[1] = rgb[ixi + 1];

				for (k = 0; k < 2; k++) {
					img->setPaletteRGB (img,
						convmap[j][i][k],
						bw_rgb[k] & 0xff,
						(bw_rgb[k] >> 8) & 0xff,
						(bw_rgb[k] >> 16) & 0xff);
				}

				// fprintf (stderr, "[%06x,%06x]",
				//		bw_rgb[0], bw_rgb[1]);
			}
			// fprintf (stderr, "\n");
		}
	}

	fprintf (stderr, "Lyrics Main Thread STARTED\n");

	banner->rectangle (banner, 0, 0,
				KOK_BANNER_IMAGE_WIDTH  + 1,
				KOK_BANNER_IMAGE_HEIGHT + 1, 0, 1);

	banner_location = kok->get_banner (kok, &banner_width,
					&banner_height, &banner_real_width);

	banner->packedBitmap (banner, 1, 1, title_color, 0, 
				banner_location,
				banner_width,
				banner_height,
				banner_real_width);

	banner->border (banner, 1, 1, banner_real_width,
					banner_height, title_color, 7, bw);

	np = pd->np;

	hdr = kok->header (kok);
	fw = hdr->realsizes / 2;	// font width

	init_step = fw / 3;

	// fprintf (stderr, "Font width=%d\n", fw);

	// start_y[1] = hdr->realsizes + 4;

	kok->reset (kok);

	lyr[0] = kok->get (kok, 0, &kidx[0]);
	lyr[1] = kok->get (kok, 1, &kidx[1]);

	// fprintf (stderr, "%d,%d\n", kidx[0], kidx[1]);
	from_x[0] = from_x[1] = to_x[0] = to_x[1] = 0;

	img->rectangle (img, 0, 0, img_w, img_h, 0, 1);
	/*  Debug 框
	img->rectangle (img, 0, 0, img_w - 1, img_h - 1, 2, 0);
	*/

	/*
	 *	Show banner here ....
	 */

	pthread_mutex_lock   (&pd->mutex);
	while (pd->thrwait) pthread_cond_wait (&pd->condi, &pd->mutex);
	pthread_mutex_unlock (&pd->mutex);


	xtm->start (xtm);

	update = clear = cls[0] = cls[1] = 0;
	if (pd->have_fmp) osd->open (osd);
	// fprintf (stderr, "Display banner\n");
	if (pd->have_fmp) osd->displayBitmap (osd, banner->image (banner), 100);

	while (! pd->terminate) {
		tm = xtm->elapsed (xtm);

		for (i = 0; i < 2; i++) {
			switch (state[i]) {
			case 0: // 出現前
				if (lyr[i] == NULL) {
					if (! lstop[i]) {
						tcnt++;
						lstop[i] = 1;
					}

					if (tcnt == 2) {
						if (! clear) pd->terminate = 1;
					}
				} else if (lyr[i]->time[0] * 100 - 2500 <= tm) {
					if (cls[i]) {
						img->rectangle (img,
							start_x[i] - bw,
							start_y[i] - bw,
							last_w[i], last_h[i],
							0, 1);
						cls[i] = 0;
					}
					/*
					fprintf (stderr, "%d. %d,%d\n",
						i, lyr[i]->time[0] * 100, tm);
					*/
					state[i] = 1;
					update  = 1;

					start_x[i] = org_x[i] + lyr[i]->xoffset;
					division[i] = lyr[i]->division;
					ii[i] = 0;
					range[i][0] = range[i][1] = 0;

					/*
					fprintf (stderr, "(%d,%d)%d[%s]%d\n",
						kbmp[kidx[i]].width,
						kbmp[kidx[i]].height,
						lyr[i]->length,
						lyr[i]->sentence,
						lyr[i]->singer);
					*/

					singer[i] = lyr[i]->singer % 4;

					img->packedBitmap (img,
						start_x[i],
						start_y[i],
						fcolor[singer[i]][0], 0,
						kbmp[kidx[i]].bmp,
						kbmp[kidx[i]].width,
						kbmp[kidx[i]].height,
						kbmp[kidx[i]].real_width);

					last_w[i] = kbmp[kidx[i]].real_width
					       		+ bw * 2;
					last_h[i] = kbmp[kidx[i]].height
							+ bw * 2;
					img->border (img,
						start_x[i] - bw,
						start_y[i] - bw,
						last_w[i], last_h[i],
						fcolor[singer[i]][0],
						bcolor[singer[i]][0], bw);
					from_x[i] = to_x[i] = 0;
				}
				break;
			case 1: // 準備開始拉
				if (ii[i] >= division[i]) {
					/*
					img->rectangle (img,
						start_x[i] - bw,
						start_y[i] - bw,
						last_w[i], last_h[i],
						0, 1);
					*/
					cls[i] = 1;
					state[i] = 0;

					lyr[i] = kok->get (kok, i, &kidx[i]);
					clear = 1;

					break;
				} else if (lyr[i]->time[ii[i]] * 100 -
						prepare_time > tm) {
					// 時間未到
					// 假定 0.05 秒的準備時間
					break;
				}

				state[i] = 2;
				//range[i][0] = range[i][1];
				range[i][0] = lyr[i]->jump[ii[i]]   * fw;
				range[i][1] = lyr[i]->jump[ii[i]+1] * fw;

				// from_x[i] = to_x[i] = range[i][0] - bw;
				from_x[i] = to_x[i] = range[i][0];

				if (lyr[i]->time[ii[i]+1] >
						lyr[i]->time[ii[i]]) {
					step[i] = ROUND (((double) (
						range[i][1] - range[i][0])) / 
						((double) (
						lyr[i]->time[ii[i]+1] -
						 lyr[i]->time[ii[i]])) /
						osd_constant);

					if (step[i] < 1) step[i] = 1;

					if (step[i] < init_step) {
						to_x[i] += init_step;
					} else {
						to_x[i] += step[i];
					}
				} else {
					int	r, j = 2;

					while (ii[i] + j < division[i]) {
						if (lyr[i]->time[ii[i] + j] <=
							lyr[i]->time[ii[i]]) {
							j++;
						} else {
							break;
						}
					}
					--j;

					r = lyr[i]->jump[ii[i]+j] * fw;
					range[i][1] =
						lyr[i]->jump[ii[i]+j+1] * fw;

					ii[i] += j;
					step[i] = range[i][1] - range[i][0];

					step[i] = ROUND (((double) (
						range[i][1] - r)) / 
						((double) (
						lyr[i]->time[ii[i]+1] -
						 lyr[i]->time[ii[i]])) /
						osd_constant);

					if (step[i] < 1) step[i] = 1;

					if (step[i] < init_step) {
						to_x[i] += init_step;
					} else {
						to_x[i] += step[i];
					}

					to_x[i] += (r - range[i][0]);

					fprintf (stderr, "\nSTEPPING=%d\n", j);
				}
				// fprintf (stderr, "STEP=%d\n", step[i]);

			case 2: // 已經在拉了 ....

				if (to_x[i] > range[i][1]) {
					to_x[i] = range[i][1];
				}

				if (from_x[i] < range[i][1]) {
					img->atob (img, start_x[i] - bw
							+ from_x[i],
						start_y[i],
						to_x[i] - from_x[i],
						last_h[i],
						fcolor[singer[i]][0],
						fcolor[singer[i]][1]);
					img->atob (img, start_x[i] - bw
							+ from_x[i],
						start_y[i],
						to_x[i] - from_x[i],
						last_h[i],
						bcolor[singer[i]][0],
						bcolor[singer[i]][1]);
					update = 1;
				} else {
					ii[i]++;
					state[i] = 1;
				}

				from_x[i] = to_x[i];
				to_x[i] += step[i];

				break;
			}
		}

		if (update) {
			if (pd->have_fmp) {
				//osd->displayBitmap (osd, img->image (img), 100);
				osd->displayBitmap (osd, img->image (img), 1);
			}
			update = clear = 0;
		} else if (clear) {
			if (++clear >= old_lyr_clear_time) {
				clear = 0;
				update = 1;
				for (i = 0; i < 2; i++) {
					if (cls[i]) {
						img->rectangle (img,
							start_x[i] - bw,
							start_y[i] - bw,
							last_w[i], last_h[i],
							0, 1);
						cls[i] = 0;
					}
				}
			}
		}
		
		usleep (10000);
	}

	if (pd->have_fmp) {
		osd->osd_off (osd);
		osd->close (osd);
	}

	fprintf (stderr, "Lyrics Main Thread TERMINATE\n");
	pd->have_thr = pd->terminate = 0;
	pthread_exit (NULL);
}

static void lyr_start (struct lyrics_osd_t *self) {
	self->pd.thrwait = 0;
	pthread_cond_signal  (&self->pd.condi);
}

static void lyr_pause (struct lyrics_osd_t *self) {
	self->pd.thrwait = 1;
}

static int lyr_stop (struct lyrics_osd_t *self) {
	if (self->pd.have_thr) {
		// fprintf (stderr, "lyr_stop\n");

		self->pd.terminate = 1;
		self->start (self);
		self->pd.need_join = 0;
		pthread_join (self->pd.thr, NULL);

		return 1;
	} else if (self->pd.need_join) {
		self->pd.need_join = 0;
		pthread_join (self->pd.thr, NULL);

		// fprintf (stderr, "stop join for lyrics thread\n");

		return 2;
	}

	return 0;
}

static void lyr_cleanup (struct lyrics_osd_t *self) {
	struct koklib_bitmap_t	*kbmp = self->pd.kbmp;
	int			i;

	if (kbmp != NULL) {
		for (i = 0; i < self->pd.nk; i++) free (kbmp[i].bmp);

		free (kbmp);
		self->pd.nk = 0;

		self->pd.kbmp = NULL;
	}
}

static void lyr_dispose (struct lyrics_osd_t *self) {
	struct lyrics_osd_pd_t	*pd  = &self->pd;

	// fprintf (stderr, "lyr_dispose\n");

	self->stop (self);
	self->cleanup (self);

	if (pd->kok    != NULL) pd->kok->dispose (pd->kok);
	if (pd->osd    != NULL) pd->osd->dispose (pd->osd);
	if (pd->img    != NULL) pd->img->dispose (pd->img);
	if (pd->banner != NULL) pd->banner->dispose (pd->banner);
	if (pd->xtm    != NULL) pd->xtm->dispose (pd->xtm);

	free (self);
}

static int lyr_init (struct lyrics_osd_t *self, const char *fname) {
	struct lyrics_osd_pd_t	*pd  = &self->pd;
	struct koklib_t		*kok = pd->kok;
	struct koklib_bitmap_t	*kbmp = pd->kbmp;
	int			i, np;

	if ((np = kok->load (kok, fname)) < 0) return 0;

	self->stop (self);
	self->cleanup (self);

	if ((kbmp = calloc (np, sizeof (struct koklib_bitmap_t))) == NULL) {
		perror ("lyr_init:calloc");
		return -1;
	}

	pd->kbmp = kbmp;
	pd->nk = pd->np = np;

	for (i = 0; i < np; i++) {
		kok->read_para (kok);
		kok->read_bitmap (kok, &kbmp[i]);
	}

	kok->close (kok);

	self->pause (self);

	if (pd->need_join) {
		pd->need_join = 0;
		pthread_join (self->pd.thr, NULL);
		fprintf (stderr, "Join last lyrics thread\n");
	}

	pd->have_thr = 1;
	pd->terminate = 0;
	pthread_create (&pd->thr, NULL, lyrsvc_main, self);
	pd->need_join = 1;

	pd->can_restart = 1;

	return 1;
}

static int lyr_is_running (struct lyrics_osd_t *self) {
	return self->pd.have_thr ? 1 : 0;
}

static int lyr_reset (struct lyrics_osd_t *self) {
	self->stop (self);
	return self->reinit (self);
}

static int lyr_reinit (struct lyrics_osd_t *self) {
	if (self->pd.can_restart && ! self->pd.have_thr) {
		if (self->pd.need_join) {
			self->pd.need_join = 0;
			pthread_join (self->pd.thr, NULL);
			fprintf (stderr, "Join last lyrics thread\n");
		}

		self->pause (self);
		self->pd.have_thr = 1;
		self->pd.terminate = 0;
		pthread_create (&self->pd.thr, NULL, lyrsvc_main, self);
		return 1;
	}
	return 0;
}

static int lyr_restart (struct lyrics_osd_t *self) {
	if (self->reinit (self)) {
		self->start (self);
		return 1;
	}

	return 0;
}

static void lyr_abort (struct lyrics_osd_t *self) {
	struct stio_engine_t	*stio = self->stio_engine (self);

	// fprintf (stderr, "Lyrics abort\n");
	// self->pd.abort = 1;
	self->pd.terminate = 1;
	stio->abort (stio);
}

static struct stio_engine_t * lyr_stio_engine (struct lyrics_osd_t *self) {
	struct koklib_t		*kok = self->pd.kok;

	return kok->stio_engine (kok);
}

static struct rmimage256_t *  lyr_image (struct lyrics_osd_t *self) {
	return self->pd.img;
}

static int lyr_rgb (struct lyrics_osd_t *self, const int from) {
	int	rc = self->pd.use_default_rgb;

	if (from == 0) {
		self->pd.use_default_rgb = 0;
	} else if (from > 0) {
		self->pd.use_default_rgb = 1;
	}

	fprintf (stderr, "OSD RGB default = %d\n", self->pd.use_default_rgb);

	return rc;
}

struct lyrics_osd_t * new_lyrics_osd (const int have_fmp) {
	struct lyrics_osd_t	*self;
	struct lyrics_osd_pd_t	*pd;
	int			ok;

	while ((self = malloc (sizeof (struct lyrics_osd_t))) != NULL) {
		pd = &self->pd;

		self->dispose		= lyr_dispose;
		self->init		= lyr_init;
		self->reinit		= lyr_reinit;
		self->reset		= lyr_reset;
		self->restart		= lyr_restart;
		self->stop		= lyr_stop;
		self->start		= lyr_start;
		self->pause		= lyr_pause;
		self->is_running	= lyr_is_running;
		self->cleanup		= lyr_cleanup;
		self->abort		= lyr_abort;
		self->stio_engine	= lyr_stio_engine;
		self->image		= lyr_image;
		self->rgb		= lyr_rgb;
		self->set_palette	= lyr_set_my_palette;

		pd->kok = NULL;
		pd->osd = NULL;
		pd->img = NULL;

		pd->use_default_rgb	= 0;

		pd->have_fmp  = have_fmp;
		pd->have_thr  = 0;
		pd->terminate = 0;
		pd->thrwait   = 0;
		pd->can_restart = 0;
		pd->need_join = 0;

		pthread_mutex_init (&pd->mutex, NULL);
		pthread_cond_init  (&pd->condi, NULL);

		pd->np  =  pd->nk = 0;

		ok = 1;

		if ((pd->kok = new_koklib ()) == NULL) {
			ok = 0;
		} else if ((pd->img = new_rmimage256 (
					img_w + 1, img_h + 1)) == NULL) {
			ok = 0;
		} else if ((pd->banner = new_rmimage256 (
					KOK_BANNER_IMAGE_WIDTH + 2,
					KOK_BANNER_IMAGE_HEIGHT + 2)) == NULL) {
			ok = 0;
		} else if ((pd->osd = new_rmosdlib ()) == NULL) {
			ok = 0;
		} else if ((pd->xtm = new_xtimer ()) == NULL) {
			ok = 0;
		}

		if (! ok) {
			self->dispose (self);
			break;
		}

		// Draw a rectangle to show OSD boundary
		pd->img->rectangle (pd->img, 0, 0, img_w - 1, img_h - 1, 2, 0);


		break;
	}

	return self;
}
