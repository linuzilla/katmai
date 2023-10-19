/*
 *	stream_mp3.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "stream_mp3.h"
#include "mp3lib.h"
#include "stio_engine.h"

static void * playMP3_main (void *selfptr) {
	struct stream_mp3_t	*self = selfptr;
	struct stream_mp3_pd_t	*pd = &self->pd;
	struct mp3lib_t		*mp3[2];
	struct stio_engine_t    *stio[2];
	char			buffer[2048];
	int			len;
	int			i;
	int			readycnt = 0;
	int			channel = 0;

	pd->reset_mp3	   = 0;

	for (i = 0; i < 2; i++) {
		mp3[i] = pd->mp3[i];
		stio[i] = pd->stio[i];

		if (pd->ready[i]) {
			mp3[i]->init (mp3[i]);
			readycnt++;

			if (i == pd->selected) {
				channel = i;
			} else {
				mp3[i]->mute (mp3[i], 1);
			}
		}
	}
	pd->channel_change = 0;

	fprintf (stderr, "Stream MP3 Main Thread STARTED\n");

	pthread_mutex_lock   (&pd->mutex);
	while (pd->thrwait) pthread_cond_wait (&pd->condi, &pd->mutex);
	pthread_mutex_unlock (&pd->mutex);

	while (! pd->terminate) {
		for (i = 0; i < 2; i++) {
			if (! pd->ready[i]) {
			} else if ((len = stio[i]->read (stio[i],
					buffer, sizeof buffer)) <= 0) {
				pd->ready[i] = 0;
				readycnt--;
			} else {
				mp3[i]->push (mp3[i], buffer, len);

				/*
				fprintf (stderr, "Frame[%d]: %d\n",
					i, mp3[i]->frameNumber (mp3[i]));
				*/
			}
		}
		if (readycnt <= 0) break;

		if (pd->channel_change) {
			mp3[channel]->mute (mp3[channel], 1);
			channel = pd->selected;
			mp3[channel]->mute (mp3[channel], 0);
			pd->channel_change = 0;
		}

		if (pd->reset_mp3) {
			// fprintf (stderr, "Stream MP3: RESET\n");
			pd->reset_mp3 = 0;
			for (i = 0; i < 2; i++) {
				if (pd->ready[i]) {
					mp3[i]->finish (mp3[i]);
					stio[i]->reset (stio[i]);
					mp3[i]->init   (mp3[i]);
				}
			}
			// self->pause (self);
			usleep (100);

			if (pd->thrwait) {
				struct timespec		timeout; 
				struct timeval		now;


				pthread_mutex_lock   (&pd->mutex);

				while (pd->thrwait) {
					gettimeofday (&now, NULL);

					timeout.tv_sec  = now.tv_sec;
					timeout.tv_nsec = now.tv_usec * 1000 +
							100000;

					if (timeout.tv_nsec >= 1000000000) {
						++timeout.tv_sec;
						timeout.tv_nsec %= 1000000000;
					}

					pthread_cond_timedwait (&pd->condi,
							&pd->mutex, &timeout);
				}
				pthread_mutex_unlock (&pd->mutex);
			}

			// fprintf (stderr,
			// 		"Reset after Fast Replay for MP3\n");
		}
	}

	for (i = 0; i < 2; i++) {
		// stio[i]->abort (stio[i]);
		stio[i]->close (stio[i]);
		mp3[i]->finish (mp3[i]);
		pd->ready[i] = 0;
	}

	fprintf (stderr, "Stream MP3 Main Thread TERMINATE\n");
	pd->have_thr = pd->terminate = 0;
	pthread_exit (NULL);
}

static void stmp3_start (struct stream_mp3_t *self) {
	self->pd.thrwait = 0;
	pthread_cond_signal  (&self->pd.condi);
}

static void stmp3_pause (struct stream_mp3_t *self) {
	self->pd.thrwait = 1;
}

static void stmp3_abort (struct stream_mp3_t *self) {
	struct stio_engine_t	*stio0 = self->pd.stio[0];
	struct stio_engine_t	*stio1 = self->pd.stio[1];

	// self->pd.abort = 1;
	self->pd.terminate = 1;
	stio0->abort (stio0);
	stio1->abort (stio1);
	// fprintf (stderr, "Abort stream mp3\n");
}

static int stmp3_stop (struct stream_mp3_t *self) {
	self->abort (self);

	if (self->pd.have_thr) {
		self->pd.terminate = 1;
		self->start (self);
		self->pd.need_join = 0;
		pthread_join (self->pd.thr, NULL);

		return 1;
	} else if (self->pd.need_join) {
		self->pd.need_join = 0;
		pthread_join (self->pd.thr, NULL);
		// fprintf (stderr, "Stop Join for mp3 streamming\n");

		return 2;
	}

	return 0;
}

static int stmp3_is_running (struct stream_mp3_t *self) {
	return self->pd.have_thr ? 1 : 0;
}

static void stmp3_dispose (struct stream_mp3_t *self) {
	int	i;

	struct stream_mp3_pd_t	*pd = &self->pd;

	self->stop (self);

	for (i = 0; i < 2; i++) {
		if (pd->stio[i] != NULL) pd->stio[i]->dispose (pd->stio[i]);
		if (pd->mp3[i]  != NULL) pd->mp3[i]->dispose (pd->mp3[i]);
	}

	free (self);
}

static void stmp3_reset (struct stream_mp3_t *self) {
	self->pd.reset_mp3 = 1;
}

static int stmp3_init (struct stream_mp3_t *self,
				const int channel,
				const char *fname) {
	struct stream_mp3_pd_t	*pd = &self->pd;
	struct stio_engine_t    *stio;
	int			i;

	i = (channel == 0) ? 0 : 1;

	stio = pd->stio[i];

	stio->close (stio);
	pd->ready[i] = 0;

	if (stio->open (stio, fname) <= 0) {
		perror (fname);
		return 0;
	}

	pd->ready[i] = 1;
	self->pd.terminate = 0;

	return 1;
}

static int stmp3_run (struct stream_mp3_t *self) {
	struct stream_mp3_pd_t	*pd = &self->pd;
	int			n;

	n = (pd->selected + 1) % 2;

	if (! pd->ready[pd->selected]) {
		if (pd->ready[n]) {
			pd->selected = n;
		} else {
			return 0;
		}
	}

	self->pause (self);

	if (pd->need_join) {
		self->pd.need_join = 0;
		pthread_join (self->pd.thr, NULL);
		fprintf (stderr, "Join last stream mp3 thread\n");
	}

	pd->have_thr = 1;
	pthread_create (&pd->thr, NULL, playMP3_main, self);
	self->pd.need_join = 1;

	return 1;
}

static int stmp3_select (struct stream_mp3_t *self, const int sel) {
	int	rc = self->pd.selected;
	int	i;

	if (sel >= 0) {
		i = (sel == 0) ? 0 : 1;

		if ((i != rc) && (self->pd.ready[i])) {
			self->pd.selected = i;
			// fprintf (stderr, "SELECT %d channel\n", i);
			self->pd.channel_change = 1;
		}
	}

	return rc;
}

static struct stio_engine_t * stmp3_stio_engine (struct stream_mp3_t *self,
		const int channel) {
	if (channel == 1) return self->pd.stio[1];

	return self->pd.stio[0];
}

struct stream_mp3_t * new_stream_mp3 (void) {
	struct stream_mp3_t	*self;
	struct stream_mp3_pd_t	*pd;
	int			i, ok;
	static char		*name[2] = {
					"Music MP3",
					"Vocal MP3"
				};

	while ((self = malloc (sizeof (struct stream_mp3_t))) != NULL) {
		pd = &self->pd;

		pd->stio[0] = NULL;
		pd->stio[1] = NULL;
		pd->mp3[0]  = NULL;
		pd->mp3[1]  = NULL;

		pd->reset_mp3 = 0;

		self->dispose	= stmp3_dispose;
		self->start	= stmp3_start;
		self->pause	= stmp3_pause;
		self->stop	= stmp3_stop;
		self->init	= stmp3_init;
		self->reset	= stmp3_reset;
		self->is_running= stmp3_is_running;
		self->select	= stmp3_select;
		self->run	= stmp3_run;
		self->abort	= stmp3_abort;
		self->stio_engine = stmp3_stio_engine;

		pthread_mutex_init (&pd->mutex, NULL);
		pthread_cond_init  (&pd->condi, NULL);


		ok = 1;

		for (i = 0; i < 2; i++) {
                	if ((pd->stio[i] = new_stio_engine ()) == NULL) {
				ok = 0;
				break;
			}

			pd->stio[i]->set_name (pd->stio[i], name[i]);

			if ((pd->mp3[i] = new_mp3lib ()) == NULL) {
				ok = 0;
				break;
			} else {
				// pd->mp3[i]->set_verbose (pd->mp3[i], 0);
				pd->mp3[i]->set_verbose (pd->mp3[i], 1);
			}
		}

		if (! ok) {
			self->dispose (self);
			self = NULL;
			break;
		}

		pd->have_thr = 0;
		pd->terminate = 0;
		pd->thrwait   = 0;
		pd->can_restart = 0;
		pd->selected = 0;
		pd->channel_change = 0;
		pd->need_join = 0;


		break;
	}

	return self;
}
