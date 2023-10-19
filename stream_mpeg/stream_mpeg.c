/*
 *	stream_mpeg.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "stream_mpeg.h"
#include "stio_engine.h"
#include "thread_svc.h"
#include "usec_timer.h"
#include "fmplib.h"

#define ENABLE_AUDIO_MODE	1
#define ENABLE_AUDIO_CHANNEL	1
#define ENABLE_PUBLIC_MPEG	1

#ifndef ENABLE_OSD_TESTING
#define ENABLE_OSD_TESTING	1
#endif

#if ENABLE_OSD_TESTING == 1
#include "rmimage256.h"
#include "rmosdlib.h"
#endif

#ifndef	FMP_BUFFER_SIZE
#define FMP_BUFFER_SIZE		32768
#endif

#ifndef DELAY_LOAD_MPEG_DRIVER
#define DELAY_LOAD_MPEG_DRIVER	1
#endif

#if ENABLE_PUBLIC_MPEG == 1
#include "play_public.c"
#endif

static void starting_play_mpeg (struct stream_mpeg_t *self,
						enum fmp_stream_t streamtype) {
	struct stream_mpeg_pd_t	*pd   = &self->pd;
	struct fmplib_t		*fmp  = pd->fmp;

#if ENABLE_PUBLIC_MPEG == 1
	pthread_mutex_lock   (&pd->pub_mutex);
	pd->public_stop_playing = 1;
	while (pd->public_state != PMPPS_IDLE) {
		if (pd->public_state == PMPPS_NULL) break;

		pthread_cond_wait (&pd->pub_condi, &pd->pub_mutex);
	}
	pthread_mutex_unlock (&pd->pub_mutex);
#endif

	fmp->stop  ();
	fmp->close ();

	fmp->open (streamtype);
	fmp->set_volume (pd->volume_level);

	// fmp->set_audio_mode (FMP_AM_STEREO);

	self->audio_mode (self, pd->current_audio_mode);

	if (pd->current_audio_channel != 0) {
		self->audio_channel (self, pd->current_audio_channel);
	}

	fmp->play ();
}

static int playMPEG_main (void *selfptr) {
	struct stream_mpeg_t	*self = selfptr;
	struct stream_mpeg_pd_t	*pd   = &self->pd;
	struct stio_engine_t	*stio = pd->stio;
	struct fmplib_t		*fmp  = pd->fmp;
	struct usec_timer_t	*timer = pd->timer;
	char			buffer[FMP_BUFFER_SIZE];
	unsigned int 		*sttype = (unsigned int *) buffer;
	unsigned int		total_len;
	int			len, blen, bidx;
	int			mpeg_afmt;
	enum fmp_stream_t	streamtype;
	const char		bar[] = "\\|/-";
	int			baridx = 0;
	int			cache_buffer;
	int			bpcnt;
	int			mpeg_fmt = 0;
	double			bitrate = 0.0;
	double			gbitrate;
	double			elapse;
	short			dummy_play = pd->dummy_play;
#if ENABLE_OSD_TESTING == 1
	struct rmimage256_t	*osdimg = pd->osdimg;
	struct rmosdlib_t	*osdptr = pd->osdptr;
#endif

	pd->dummy_play = 0;

	timer->reset (timer);

	fprintf (stderr, "MPEG PLAYER is running ...\n");

	if (dummy_play) {
		mpeg_fmt = 1;
	} else {
		if ((len = stio->read (stio, buffer, FMP_BUFFER_SIZE)) <= 0) {
			pthread_mutex_lock     (&pd->mutex);
			pd->playing_error = 1;
			pthread_cond_broadcast (&pd->condi);
			pthread_mutex_unlock   (&pd->mutex);

			fprintf (stderr, "MPEG PLAYER terminated\n");
			return 1;
		}
		bidx  = 0;
		bpcnt = 0;

		if (pd->user_streamtype != NULL) {
			mpeg_fmt = pd->user_streamtype (pd->param);
		} else {
			mpeg_fmt = 0;
		}

		switch (*sttype) {
		case 0x46464952:	// RIFF
			mpeg_afmt = 1;
			break;
		case 0xba010000:        // PROGRAM stream
			mpeg_afmt = 1;
			break;
		case 0x11004047:        // TRANSPORT stream
			mpeg_afmt = 2;
			break;
		case 0xb3010000:        // VIDEO stream
			mpeg_afmt = 3;
			break;
		default:
			mpeg_afmt = -1;
			fprintf (stderr,
				"Stream type error, MPEG PLAYER terminated\n");
			pthread_mutex_lock     (&pd->mutex);
			pd->playing_error = 1;
			pthread_cond_broadcast (&pd->condi);
			pthread_mutex_unlock   (&pd->mutex);
			return 1;
		}

		if (mpeg_fmt == 0) {
			fprintf (stderr, "Auto Detected: ");
			mpeg_fmt = mpeg_afmt;
		} else if (mpeg_fmt != mpeg_afmt) {
			fprintf (stderr, "UNKNOW stream %d != %d\n",
							mpeg_fmt, mpeg_afmt);

			pthread_mutex_lock     (&pd->mutex);
			pd->playing_error = 1;
			pthread_cond_broadcast (&pd->condi);
			pthread_mutex_unlock   (&pd->mutex);
			return 1;
		} else {
			fprintf (stderr, "StreamType Match: ");
		}
	}

	switch (mpeg_fmt) {
	case 1:
		fprintf (stderr, "SYSTEM (PROGRAM) stream\n");
		// streamtype = FMP_PROGRAM_STREAM;
		streamtype = FMP_SYSTEM_STREAM;
		break;
	case 2:
		streamtype = FMP_TRANSPORT_STREAM;
		fprintf (stderr, "TRANSPORT stream\n");
		break;
	case 3:
		fprintf (stderr, "VIDEO stream\n");
		streamtype = FMP_VIDEO_STREAM;
		break;
	default:
		fprintf (stderr, "UNKNOW stream\n");

		pthread_mutex_lock     (&pd->mutex);
		pd->playing_error = 1;
		pthread_cond_broadcast (&pd->condi);
		pthread_mutex_unlock   (&pd->mutex);
		return 1;
	}

	if (! dummy_play) {
		// FIXME 如果有 pre-queue, 則段的時間可以節省 ?!

		fprintf (stderr,
				"Wait until minimal buffer (%d) ready "
				"..........", pd->start_play_buffer);

		while ((cache_buffer = pd->rbuf - stio->freebuf (stio))
						< pd->start_play_buffer) {
			fprintf (stderr, "\b\b\b\b%4d", cache_buffer);
			usleep (100000);

			if (pd->abort) break;
			if (stio->end_of_prefetch (stio)) break;
		}

		fprintf (stderr, "\b\b\b\b.. ok.\n");
	}

	if (pd->set_mixer_callback != NULL) {
		pd->set_mixer_callback (pd->mixcbk_parm);
	}

	/*
	mix->close_lock (1);
	mix->setvolume (pd->channel0, pd->vol0);
	mix->setvolume (pd->channel1, pd->vol1);
	mix->close_lock (0);
	mix->close ();
	*/

	starting_play_mpeg (self, streamtype);

#if ENABLE_OSD_TESTING == 1
	if (dummy_play) {
		fprintf (stderr, "Open OSD\n");
		osdptr->open (osdptr);
		osdptr->displayBitmap (osdptr, osdimg->image (osdimg), 0);
	}
#endif
	pthread_mutex_lock     (&pd->mutex);
	pd->playing = 1;
	pthread_cond_broadcast (&pd->condi);
	pthread_mutex_unlock   (&pd->mutex);

	timer->start (timer);
	timer->start (timer);
	total_len = 0;

	pd->reset_mpeg = 0;
	pd->in_replay_state = 0;
	pd->into_replay_state = 0;

	while (! pd->terminate) {
		if (dummy_play) {
			usleep (10000);

			if (pd->verbose > 0) {
				baridx = (baridx + 1) % 4;
				fprintf (stderr, "\r%c", bar[baridx]);
			}

			if (pd->change_plattle) {
				fprintf (stderr, "Change image\n");
				pd->change_plattle = 0;
				osdptr->displayBitmap (osdptr,
						osdimg->image (osdimg), 0);
			}
			continue;
		}

		while (len > 0) {
			if (fmp->get_buffer () == 0) {
				perror ("get_buffer");
				return 1;
			}
			// blen = fmp->buffer_size ();

			if ((blen = fmp->buffer_size ()) < len) {
				memcpy (fmp->buffer_ptr (),
						&buffer[bidx], blen);
				len  -= blen;
				bidx += blen;
			} else {
				memcpy (fmp->buffer_ptr (), &buffer[bidx], len);
				blen = len;
				len = bidx = 0;
			}


			fmp->set_data_size (blen);
			fmp->push ();

			total_len += blen;

			elapse = timer->ended (timer);
			if ((blen >= FMP_BUFFER_SIZE) && (elapse > 0.0)) {
				bitrate = (double) blen /
						elapse / 128.0 / 1024.0;

				if (bitrate < pd->danger_bitrate) {
					if (++bpcnt == 3) {
						pd->bitrate_problem++;
						bpcnt = 0;
					}
				} else {
					bpcnt = 0;
				}
			}

			if ((elapse = timer->elapsed (timer)) > 2.0) {
				gbitrate = (double)  total_len / elapse 
							/ 128.0 / 1024.0;
			}

			if ((cache_buffer = pd->rbuf -
						stio->freebuf (stio)) <= 1) {
				if (pd->in_replay_state) {
					pd->into_replay_state = 1;
				} else if (stio->prefetch_is_running (stio)) {
					if (! self->pd.abort) {
						pd->under_run = 1;
					}
				}
			} else if (pd->into_replay_state) {
				if (cache_buffer > 20) {
					pd->in_replay_state = 0;
					pd->into_replay_state = 0;
				}
			}

			if (pd->verbose > 0) {
				baridx = (baridx + 1) % 4;
				fprintf (stderr, "\r%5d/%d %c%c"
					"[%7.2fMbps/%5.2fMbps] %c",
					cache_buffer, pd->rbuf,
					pd->under_run ? 'u' : '-',
					pd->bitrate_problem ? 'b' : '-',
					bitrate, gbitrate, bar[baridx]);
			}

			if (pd->reset_mpeg) {
				// FIXME: 如果已經 prefetch next ?
				// fprintf (stderr, "Stream MPEG: RESET\n");
				pd->reset_mpeg = 0;
				stio->reset (stio);

				starting_play_mpeg (self, streamtype);
			}
		}

		timer->start (timer);

		// fprintf (stderr, "Reading ...\n");
		if ((len = stio->read (stio, buffer, FMP_BUFFER_SIZE)) <= 0)
			break;
	}

#if ENABLE_OSD_TESTING == 1
	if (dummy_play) {
		fprintf (stderr, "Close OSD\n");
		osdptr->osd_off (osdptr);
		osdptr->close (osdptr);
	}
#endif

	fmp->stop ();
	fmp->close ();

	stio->abort (stio);
	stio->close (stio);

	fprintf (stderr, "MPEG PLAYER terminated\n");

	// Reset Audio Mode and Audio Channel

	pd->current_audio_mode	= 'S';
	pd->current_audio_channel = 0;

#if ENABLE_PUBLIC_MPEG == 1
	if (pd->public_state == PMPPS_IDLE) {
		pd->public_state = PMPPS_START;
	}
#endif

	return 1;
}

static int stm_audio_channel (struct stream_mpeg_t *self, const int channel) {
#if ENABLE_AUDIO_CHANNEL == 1
	struct fmplib_t		*fmp  = self->pd.fmp;

	if ((channel >= 0) && (channel < 10)) {
		fmp->set_audio_channel (channel);
		return 1;
	}
#endif
	fprintf (stderr, "[+] Audio channel: %d\n", channel);

	return 0;
}

static int stm_audio_mode (struct stream_mpeg_t *self, const char mode) {
	struct fmplib_t		*fmp  = self->pd.fmp;
	enum fmp_audio_mode_t	amode;

	switch (mode) {
	case 'S':
	case 's':
		amode = FMP_AM_STEREO;
		break;
	case 'L':
	case 'l':
		amode = FMP_AM_LEFT;
		break;
	case 'R':
	case 'r':
		amode = FMP_AM_RIGHT;
		break;
	case 'M':
	case 'm':
		amode = FMP_AM_MIX;
		break;
	default:
		return 0;
	}

#if ENABLE_AUDIO_MODE == 1
	fmp->set_audio_mode (amode);
#endif
	fprintf (stderr, "[+] Audio Mode: %c\n", mode);

	return 1;
}

static void stm_preset_audio_channel (struct stream_mpeg_t *self,
						const int channel) {
	self->pd.current_audio_channel = channel;
}

static void stm_preset_audio_mode (struct stream_mpeg_t *self,
						const char mode) {
	self->pd.current_audio_mode	= mode;
}

static int stm_open (struct stream_mpeg_t *self, const char *fname) {
	struct stio_engine_t	*stio = self->pd.stio;

	self->close (self);

	if (stio->open (stio, fname) > 0) {
		self->pd.playing = 0;
		self->pd.bitrate_problem = 0;
		self->pd.under_run = 0;
		self->pd.repeat_cnt = 0;
		// fprintf (stderr, "stm_open: ok\n");
		return 1;
	}

	return -1;
}

static int stm_wait_for_playing (struct stream_mpeg_t *self) {
	struct stream_mpeg_pd_t	*pd = &self->pd;

	if ((! pd->playing && ! pd->playing_error)) {
		pthread_mutex_lock (&pd->mutex);
		while ((! pd->playing) && (! pd->playing_error)) {
			pthread_cond_wait (&pd->condi, &pd->mutex);
		}
		pthread_mutex_unlock (&pd->mutex);
	}

	if (pd->playing_error) return 0;
	return 1;
}

static int stm_start (struct stream_mpeg_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	// self->stop (self);
	self->pd.playing_error = 0;
	self->pd.on_play = 1;
	self->pd.terminate = 0;
	self->pd.abort = 0;
	thr->start (thr);
	return 1;
}

static int stm_reset (struct stream_mpeg_t *self) {
	self->pd.reset_mpeg = 1;
	self->pd.in_replay_state = 1;
	/*
	struct thread_svc_t	*thr = self->pd.thr;
	struct stio_engine_t	*stio = self->pd.stio;

	self->pd.terminate = 1;
	stio->abort (stio);
	// fprintf (stderr, "MPEG stop, abort STIO ok\n");
	thr->stop (thr);
	self->pd.on_play = 0;
	stio->close (stio);
	self->pd.dummy_play = 0;
	*/

	return 1;
}

static int stm_dummy_start (struct stream_mpeg_t *self) {
	self->pd.dummy_play = 1;
	return self->start (self);
}

static void stm_abort (struct stream_mpeg_t *self) {
	struct stio_engine_t	*stio = self->pd.stio;

	self->pd.abort = 1;
	self->pd.terminate = 1;
	stio->abort (stio);
}

static int stm_set_repeat (struct stream_mpeg_t *self, const int r) {
	return 0;
}

static int stm_set_buffer (struct stream_mpeg_t *self, const int r) {
	struct stio_engine_t	*stio = self->pd.stio;
	int			rc = stio->set_buffer (stio, r);

	if (rc > 0) self->pd.rbuf = rc;

	return rc;
}

static int stm_set_volume (struct stream_mpeg_t *self, const int v) {
	int	rc = self->pd.volume_level;

	if ((v >= 0) && (v < 100)) self->pd.volume_level = v;

	return rc;
}

static int stm_is_playing (struct stream_mpeg_t *self) {
	if (self->pd.on_play) {
		struct thread_svc_t	*thr = self->pd.thr;

		return thr->is_running (thr);
	}
	return 0;
}

static void stm_stop (struct stream_mpeg_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;
	struct stio_engine_t	*stio = self->pd.stio;

	self->pd.terminate = 1;
	stio->abort (stio);
	// fprintf (stderr, "MPEG stop, abort STIO ok\n");
	thr->stop (thr);
	self->pd.on_play = 0;
	stio->close (stio);
	self->pd.dummy_play = 0;
}

static void stm_close (struct stream_mpeg_t *self) {
	self->stop (self);
}

static int stm_in_fmp (struct stream_mpeg_t *self) {
	struct fmplib_t		*fmp = self->pd.fmp;
	struct timeval		tv;

	if (fmp->is_infmp (&tv)) return tv.tv_sec;
	return 0;
}

static int stm_verbose (struct stream_mpeg_t *self, const int level) {
	int	rc = self->pd.verbose;

	if (level >= 0) self->pd.verbose = level;

	return rc;
}

static int stm_set_minplay (struct stream_mpeg_t *self, const int level) {
	int	rc = self->pd.start_play_buffer;

	if (level >= 0) {
		if (level < self->pd.rbuf) {
			self->pd.start_play_buffer = level;
		} else {
			self->pd.start_play_buffer = self->pd.rbuf;
		}
	}

	return rc;
}

static void stm_dispose (struct stream_mpeg_t *self) {
	struct stream_mpeg_pd_t	*pd = &self->pd;

	if (pd->stio    != NULL) pd->stio->dispose (pd->stio);
	if (pd->thr     != NULL) pd->thr->dispose (pd->thr);
	if (pd->pub_thr != NULL) pd->pub_thr->dispose (pd->pub_thr);

	// pd->fmp->close ();
	pd->fmp->driver_unload ();
	fprintf (stderr, "FMP driver unload ok !\n");

	if (pd->timer != NULL) pd->timer->dispose (pd->timer);

	free (self);
}

static struct stio_engine_t * stm_stio_engine (struct stream_mpeg_t *self) {
	return self->pd.stio;
}

static struct stio_engine_t * stm_pub_stio_engine (struct stream_mpeg_t *self) {
	return self->pd.pub_stio;
}

static int stm_bitrate_problem (struct stream_mpeg_t *self) {
	return self->pd.bitrate_problem;
}

static int stm_clear_bitrate_problem (struct stream_mpeg_t *self) {
	int	rc = self->pd.bitrate_problem;

	self->pd.bitrate_problem = 0;

	return rc;
}

static double stm_set_danger_bitrate (struct stream_mpeg_t *self,
						const double dbr) {
	double	rc = self->pd.danger_bitrate;

	if (dbr > 0.0) self->pd.danger_bitrate = dbr;

	return rc;
}

static int stm_clear_under_run (struct stream_mpeg_t *self) {
	int	rc = self->pd.under_run;
	self->pd.under_run = 0;
	return rc;
}

static int stm_is_under_run (struct stream_mpeg_t *self) {
	return self->pd.under_run;
}

static void stm_set_user_streamtype (struct stream_mpeg_t *self,
					int (*st)(void *), void *param) {
	self->pd.param = param;
	self->pd.user_streamtype = st;
}

static int stm_chip_id (struct stream_mpeg_t *self) {
	struct fmplib_t		*fmp = self->pd.fmp;

	return fmp->em84xx_id ();
}	

static void stm_change_palette (struct stream_mpeg_t *self,
				const int sty, const int stu, const int stv,
				const int yy, const int uu, const int vv) {
#if ENABLE_OSD_TESTING == 1
	struct rmimage256_t	*osdimg = self->pd.osdimg;
	int			i, j, k, y, u, v;
	int			cidx = 0;

	for (i = 0; i < yy; i++) {
		y = i * (256 - sty)  / (yy + 1) + sty;

		for (j = 0; j < uu; j++) {
			u = j * (256 - stu) / (uu + 1) + stu;

			for (k = 0; k < vv; k++) {
				v = k * (256 - stv) / (vv + 1) + stv;

				osdimg->setPaletteYUV (
					osdimg, cidx++, y, u, v);
			}
		}
	}

	self->pd.change_plattle = 1;
#endif
}

static void stm_set_vol_cbk (struct stream_mpeg_t *self,
				void (*fcn)(void *), void *parm) {
	self->pd.set_volume_callback	= fcn;
	self->pd.volcbk_parm		= parm;
}

static void stm_set_mix_cbk (struct stream_mpeg_t *self,
				void (*fcn)(void *), void *parm) {
	self->pd.set_mixer_callback	= fcn;
	self->pd.mixcbk_parm		= parm;
}

static int stm_pq_prefetch (struct stream_mpeg_t *self, const char *str) {
	struct stream_mpeg_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio = pd->stio;

	if (stio->open_next (stio, str) >= 0) {
		stio->start_pq_prefetch (stio);
	}

	return 1;
}

struct stream_mpeg_t * new_stream_mpeg (const int nbuf) {
	struct stream_mpeg_t	*self;
	struct stream_mpeg_pd_t	*pd;
	// static char		*name = "MPEG";

	if ((self = malloc (sizeof (struct stream_mpeg_t))) != NULL) {
		pd = &self->pd;

		pd->stio		= NULL;
		pd->pub_stio		= NULL;
		pd->thr			= NULL;
		pd->pub_thr		= NULL;
		pd->timer		= NULL;
		pd->fmp			= new_fmplib ();
		// pd->fmp->disable ();
		pd->volume_level	= 70;
		pd->have_load_driver	= 0;
		pd->on_play		= 0;
		pd->terminate		= 0;
		pd->verbose		= 1;
		pd->nbuf		= 0;
		pd->rbuf		= 0;
		pd->abort		= 0;
		pd->start_play_buffer	= 0;
		pd->danger_bitrate	= 0.5;
		pd->under_run		= 0;
		pd->bitrate_problem	= 0;
		pd->repeat_cnt		= 0;
		pd->dummy_play		= 0;
		pd->user_streamtype	= NULL;

		pd->current_audio_mode	= 'S';
		pd->current_audio_channel = 0;

		pd->terminate_public	= 0;
#if ENABLE_PUBLIC_MPEG == 1
		pd->public_state	= PMPPS_NULL;
#endif
		pd->public_stop_playing	= 0;
		pd->enable_public	= 0;
		pd->have_public		= 0;

		pd->set_volume_callback	= NULL;
		pd->volcbk_parm		= NULL;

		pd->set_mixer_callback	= NULL;
		pd->mixcbk_parm		= NULL;

#if ENABLE_OSD_TESTING == 1
		pd->osdimg_x		= 610;
		pd->osdimg_y		= 420;
		pd->change_plattle	= 0;

		if ((pd->osdimg = new_rmimage256 (pd->osdimg_x + 1,
						pd->osdimg_y + 1)) == NULL) {
			return NULL;
		} else {
			int	i, j, x1, x2, y1, y2, xx = 16, yy = 16;
			int	cidx = 0;

			pd->osdimg->rectangle (pd->osdimg, 0, 0,
				pd->osdimg_x - 1, pd->osdimg_y - 1, 0, 1);

			// stm_change_palette (self, 0, 0, 0, 4, 8, 8);

			for (i = cidx = 0; i < xx; i++) {
				x1 = pd->osdimg_x * i / xx;
				x2 = pd->osdimg_x * (i + 1) / xx;

				for (j = 0; j < yy; j++) {
					y1 = pd->osdimg_y * j / yy;
					y2 = pd->osdimg_y * (j + 1) / yy;

					pd->osdimg->rectangle (pd->osdimg,
							x1, y1,
							x2 - x1, y2 - y1,
							cidx % 256, 1);
					cidx++;
				}
			}
		}

		if ((pd->osdptr = new_rmosdlib ()) == NULL) {
			perror ("new_rosdlib");
			return NULL;
		}
#endif

		pthread_mutex_init (&pd->mutex, NULL);
		pthread_cond_init  (&pd->condi, NULL);

		pthread_mutex_init (&pd->pub_mutex, NULL);
		pthread_cond_init  (&pd->pub_condi, NULL);

		self->dispose		= stm_dispose;
		self->start		= stm_start;
		self->reset		= stm_reset;
		self->dummy_start	= stm_dummy_start;
		self->stop		= stm_stop;
		self->open		= stm_open;
		self->close		= stm_close;
		self->verbose		= stm_verbose;
		self->abort		= stm_abort;
		self->is_playing	= stm_is_playing;
		self->stio_engine	= stm_stio_engine;
		self->pub_stio_engine	= stm_pub_stio_engine;
		self->set_minplay	= stm_set_minplay;
		self->set_volume	= stm_set_volume;
		self->bitrate_problem	= stm_bitrate_problem;
		self->clear_bitrate_problem = stm_clear_bitrate_problem;
		self->set_danger_bitrate = stm_set_danger_bitrate;
		self->clear_under_run	= stm_clear_under_run;
		self->is_under_run	= stm_is_under_run;
		self->wait_for_playing	= stm_wait_for_playing;
		self->set_repeat	= stm_set_repeat;
		self->set_buffer	= stm_set_buffer;
		self->set_user_streamtype = stm_set_user_streamtype;
		self->in_fmp		= stm_in_fmp;
		self->chip_id		= stm_chip_id;
		self->audio_mode	= stm_audio_mode;
		self->audio_channel	= stm_audio_channel;
		self->preset_audio_channel	= stm_preset_audio_channel;
		self->preset_audio_mode	= stm_preset_audio_mode;
		self->change_palette	= stm_change_palette;
#if ENABLE_PUBLIC_MPEG == 1
		self->set_public	= stm_set_public;
		self->open_public	= stm_open_public;
#endif
		self->pq_prefetch	= stm_pq_prefetch;
		self->set_vol_cbk	= stm_set_vol_cbk;
		self->set_mix_cbk	= stm_set_mix_cbk;

		if ((pd->timer = new_usec_timer ()) == NULL) {
			self->dispose (self);
			return NULL;
		}

		if ((pd->stio = new_stio_engine ()) == NULL) {
			self->dispose (self);
			return NULL;
		}

		pd->stio->set_name (pd->stio, "MPEG");

#if ENABLE_PUBLIC_MPEG == 1
		if ((pd->pub_stio = new_stio_engine ()) == NULL) {
			self->dispose (self);
			return NULL;
		}

		pd->pub_stio->set_name (pd->pub_stio, "Public MPEG");
		pd->pub_stio->prefetch (pd->pub_stio,
				PUB_PREFETCH_BUFFER, FMP_BUFFER_SIZE);
#endif

		if ((pd->thr = new_thread_svc (playMPEG_main, self)) == NULL) {
			self->dispose (self);
			return NULL;
		}

#if ENABLE_PUBLIC_MPEG == 1
		if ((pd->pub_thr = new_thread_svc (
					playMPEG_public, self)) == NULL) {
			self->dispose (self);
			return NULL;
		}
#endif

		usleep (50000);
		if (! pd->fmp->driver_entry ()) {
			self->dispose (self);
			return NULL;
		} else {
			usleep (100000);
			pd->fmp->driver_unload ();
#if DELAY_LOAD_MPEG_DRIVER == 0
			usleep (100000);
			
			if (! pd->fmp->driver_entry ()) {
				self->dispose (self);
				return NULL;
			}
#endif
		}

		if (nbuf > 1) {
			pd->stio->prefetch (pd->stio, nbuf, FMP_BUFFER_SIZE);
			pd->rbuf = pd->nbuf = nbuf;
		}
	}

	return self;
}
