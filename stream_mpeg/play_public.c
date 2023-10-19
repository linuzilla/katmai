/*
 *	play_public.c
 *
 *	Copyright (c) 2004, Jiann-Ching Liu
 */

#define PUB_PREFETCH_BUFFER	250
#define PMPPS_NULL		0
#define PMPPS_IDLE		1
#define PMPPS_START		2
#define PMPPS_STOP		3
#define PMPPS_PLAY		4

static enum fmp_stream_t       pub_streamtype = FMP_SYSTEM_STREAM;

static void initial_public_mpeg (struct stream_mpeg_t *self) {
	// struct stream_mpeg_pd_t	*pd   = &self->pd;
}

static int stm_open_public (struct stream_mpeg_t *self, const char *fname) {
	struct stream_mpeg_pd_t	*pd   = &self->pd;
	struct stio_engine_t	*stio = pd->pub_stio;

	if (pd->have_public) {
		stio->abort (stio);
		pd->public_stop_playing = 1;

		pthread_mutex_lock   (&pd->pub_mutex);
		while (pd->public_state != PMPPS_IDLE) {
			if (pd->public_state == PMPPS_NULL) break;
			pthread_cond_wait (&pd->pub_condi, &pd->pub_mutex);
		}
		pthread_mutex_unlock (&pd->pub_mutex);

		stio->close (stio);
		pd->have_public = 0;
	}

	if (stio->open (stio, fname) > 0) {
		fprintf (stderr, "Open Public MPEG [%s] ok\n", fname);
		pd->have_public = 1;
		stio->set_repeat (stio, -1);
		pd->public_state = PMPPS_IDLE;
		return 1;
	}

	return -1;
}

static int stm_set_public (struct stream_mpeg_t *self, const int onoff) {
	struct stream_mpeg_pd_t	*pd  = &self->pd;
	struct thread_svc_t	*thr = self->pd.pub_thr; 

	if (onoff < 0) return pd->enable_public;
	if (onoff == 0) {
		pd->enable_public = 0;
	} else if (pd->have_public) {
		pd->enable_public = 1;
	} else {
		pd->enable_public = 0;
	}

	if (pd->enable_public) {
		if (! thr->is_running (thr)) thr->start (thr);
		if (! self->is_playing (self)) {
			pd->public_state = PMPPS_START;
		}
	}

	return pd->enable_public;
}

static int playMPEG_public (void *selfptr) {
	struct stream_mpeg_t    *self = selfptr;
	struct stream_mpeg_pd_t *pd   = &self->pd;
	struct fmplib_t		*fmp  = pd->fmp;
	struct stio_engine_t	*stio = pd->pub_stio;
	int			len, blen, bidx = 0;
	char			buffer[FMP_BUFFER_SIZE];
	// int			last_state = -1;
	int			bnidx = 0;
	char			banner[] = "\\|/-";

	fprintf (stderr, "[*] Public MPEG thread starting\n");
	initial_public_mpeg (self);

	while (! pd->terminate_public) {
		/*
		if (pd->public_state != last_state) {
			fprintf (stderr, "PubState=%d\n", pd->public_state);
			last_state = pd->public_state;
		}
		*/

		switch (pd->public_state) {
		case PMPPS_START:
			if (pd->have_public && pd->enable_public) {
				pd->public_stop_playing = 0;


				if (pd->set_volume_callback != NULL) {
					pd->set_volume_callback (
							pd->volcbk_parm);
				}

				fprintf (stderr, "Start Public MPEG\n");
				fmp->open (pub_streamtype);
				fmp->set_volume (pd->volume_level);

				fmp->play ();

				pd->public_state = PMPPS_PLAY;
			} else {
				pd->public_state = PMPPS_IDLE;
			}
			break;

		case PMPPS_STOP:
			fprintf (stderr, "Stop Public MPEG\n");

			fmp->stop  ();
			fmp->close ();

			pthread_mutex_lock   (&pd->pub_mutex);
			pd->public_state = PMPPS_IDLE;
			pthread_cond_signal  (&pd->pub_condi);
			pthread_mutex_unlock (&pd->pub_mutex);
			break;

		case PMPPS_PLAY:
			if ((len = stio->read (stio,
					buffer, FMP_BUFFER_SIZE)) <= 0) {
				pd->public_state = PMPPS_STOP;
			}

			while (len > 0) {
				if (fmp->get_buffer () == 0) {
					perror ("get_buffer");
					break;
				}

				if ((blen = fmp->buffer_size ()) < len) {
					memcpy (fmp->buffer_ptr (),
							&buffer[bidx], blen);
					len  -= blen;
					bidx += blen;
				} else {
					memcpy (fmp->buffer_ptr (),
							&buffer[bidx], len);
					blen = len;
					len = bidx = 0;
				}

				fmp->set_data_size (blen);
				fmp->push ();
				bnidx = (bnidx + 1) % 4;
				fprintf (stderr, "\r%3d %c",
						stio->freebuf (stio),
						banner[bnidx]);
			}

			if (pd->public_stop_playing || ! pd->enable_public) {
				pd->public_state = PMPPS_STOP;
				pd->public_stop_playing = 0;
			}
			break;


		case PMPPS_IDLE:
			usleep (100000);
			break;

		case PMPPS_NULL:
		default:
			sleep (1);
			break;
		}
	}

	return 1;
}
