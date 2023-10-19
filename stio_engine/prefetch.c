/*
 *	prefetch.c
 */

#ifndef __STIOENG_PREFETCH_C__
#define __STIOENG_PREFETCH_C__

struct stio_engine_prefetch_t {
	struct buffer_cache_t		*bc;
	struct buffer_cache_t		*kbc;
	struct buffer_cache_t		*pbc;
	int				nbuf;
	int				knbuf;
	int				pnbuf;
	short				enable_kbuf;
	short				enable_pbuf;
	off_t				kbuf_pos;
	int				bsize;
	struct thread_svc_t		*thrd;
	struct thread_svc_t		*pref_thrd;
	char				*tmpbuff;
	int				tmpbsize;
	int				tmpbidx;
	short				in_running;
	volatile short			already_pq_prefetch;
	volatile short			stop_pq_prefetch;
	volatile short			need_reopen_old_file_if_reset;

	pthread_mutex_t			mutex;
	pthread_cond_t			condi;
};

static int pbuf_dispose (struct stio_engine_t *self) {
	struct stio_engine_prefetch_t	*pch = self->pd.pch;

	if (pch != NULL) {
		if (pch->thrd != NULL) pch->thrd->dispose (pch->thrd);
		if (pch->pref_thrd != NULL) {
			pch->pref_thrd->dispose (pch->pref_thrd);
		}
		if (pch->tmpbuff != NULL) free (pch->tmpbuff);
		if (pch->kbc != NULL) pch->kbc->dispose (pch->kbc);
		if (pch->pbc != NULL) pch->pbc->dispose (pch->pbc);
		if (pch->bc != NULL) {
			pch->bc->dispose (pch->bc);
		}
		free (self->pd.pch);
		self->pd.pch = NULL;
	}

	return 0;
}

static int prefetch_queue_main (void *selfptr) {
	struct stio_engine_t		*self = selfptr;
	struct stio_engine_pd_t		*pd   = &self->pd;
	struct stio_engine_prefetch_t	*pch  = pd->pch;
	struct buffer_cache_t		*pbc  = pch->pbc;
	struct disk_buffer_t		*dkptr;
	int				len, rc, rlen;
	int				cnt;

	pch->stop_pq_prefetch = 0;
	len = pch->bsize;

	// 狦璶 preload 郎? How?
	pthread_mutex_lock (&pch->mutex);

	pch->already_pq_prefetch = 0;
	pd->in_pq_prefetch = 1;
	rc = pd->have_next_file;

	pthread_mutex_unlock (&pch->mutex);

	if (rc) {
		pd->have_next_precache = 0;
		pch->need_reopen_old_file_if_reset = 1;

		fprintf (stderr, "Have Next File [%s]\n", pd->next_file);

		if ((rc = pd->open (self, pd->next_file)) >= 0) {
			// fprintf (stderr, "File opened\n");

			pbc->cleanup (pbc);
			cnt = 0;

			while ((dkptr = pbc->p_tryreq (pbc)) != NULL) {
				rlen = pd->read (self, dkptr->buffer, len);

				pd->have_next_precache = 1;
				if (rlen == len) {
					dkptr->rc = rlen;
					//fprintf (stderr, "Read (%d)\n", rlen);
					cnt++;
				} else {
					dkptr->rc = 0;
					break;
				}
				pbc->p_ready (pbc);

				if (pch->stop_pq_prefetch) break;
			}

			fprintf (stderr,
				"PQ prefetch ok [%d buffer(s)]\n", cnt);
		} else {
			fprintf (stderr, "File open failed\n");
		}
	}
	pd->in_pq_prefetch = 0;

	if (! pch->stop_pq_prefetch) pch->already_pq_prefetch = 1;
	pch->stop_pq_prefetch = 0;

	pthread_cond_signal (&pch->condi);

	return 1;	// terminate
}

static int prefetch_main (void *selfptr) {
	struct stio_engine_t		*self = selfptr;
	struct stio_engine_pd_t		*pd   = &self->pd;
	struct stio_engine_prefetch_t	*pch  = pd->pch;
	struct buffer_cache_t		*bc   = pch->bc;
	struct buffer_cache_t		*kbc  = pch->kbc;
	struct buffer_cache_t		*pbc  = pch->pbc;
	int				len, rc, rlen;
	struct disk_buffer_t		*dkptr;
	struct disk_buffer_t		*k_dkptr;
	struct disk_buffer_t		*p_dkptr;
	int				use_prefetch_data;
	short				use_prev_data = 0;
	short				kbc_state = 0;
	off_t				fp_off = 0;
	off_t				pbc_off = 0;

	len = pch->bsize;
	pch->in_running	= 1;

	pd->end_of_prefetch = 0;

	if (pch->already_pq_prefetch) {
		use_prev_data = 1;
		pch->already_pq_prefetch = 0;
	}

	fprintf (stderr,
		"\n[%p] %s STREAMMING I/O prefetch thread started! (rep=%d)\n",
		self, pd->name ? pd->name : "",
		pd->repeat_cnt);

	if (pch->enable_kbuf == 1) {
		kbc->t_reset (kbc);
		kbc_state = 1;	// Need store data ...
	} else if (pch->enable_kbuf == 2) {
		kbc->t_reset (kbc);
		kbc_state = 3;	// Output data ---

		// rlen = pd->read (self, dkptr->buffer, len);

		pd->seek (self, pch->kbuf_pos);

		fprintf (stderr, "[%s] Seek disk data to %ld\n",
				pd->name ? pd->name : "",
				pch->kbuf_pos);
		fp_off = pch->kbuf_pos;
	}

	do {
		// fprintf (stderr, "Request buffer ... ");
		dkptr = bc->p_request (bc);
		// fprintf (stderr, "ok\n");
		dkptr->dbflag = 0;
		use_prefetch_data = 0;

		if (kbc_state == 3) {
			if ((k_dkptr = kbc->t_tryreq (kbc)) != NULL) {
				memcpy (dkptr->buffer,
						k_dkptr->buffer, k_dkptr->rc);
				dkptr->rc = rc = rlen = k_dkptr->rc;
				// fprintf (stderr, "********* Output data\n");
				use_prefetch_data = 1;
			} else {
				// fprintf (stderr, "STOP OUTPUT DATA\n");
				kbc_state = 2;
				// stop;
				rc = 0;
			}
		}

		if (use_prefetch_data) {
		} else if (pd->abort_flag) {
			// fprintf (stderr, "abort\n");
			rc = dkptr->rc = -1;
		} else {
			if (use_prev_data) {
				rlen = 0;

				if ((p_dkptr = pbc->c_tryreq (pbc)) != NULL) {
					if ((rlen = p_dkptr->rc) > 0) {
						memcpy (dkptr->buffer,
							p_dkptr->buffer,
							rlen);
						pbc_off += rlen;
						// fprintf (stderr, "From\n");
					}
				}

				if (rlen == 0) {
					use_prev_data = 0;

					pd->seek (self, pbc_off);
					rlen = pd->read (self,
							dkptr->buffer, len);
					fprintf (stderr, "Fin, seek to %ld\n",
							pbc_off);
				}
			} else {
				rlen = pd->read (self, dkptr->buffer, len);
			}

			rc = dkptr->rc = rlen;

			if (rlen == 0 || rlen == -1) {	// End of file
				if (pd->repeat_cnt != 0) {
					fprintf (stderr, "Auto repeat\n");
					if (pd->repeat_cnt > 0) {
						pd->repeat_cnt--;
					}
					// pd->reset (self);
					//

					self->pd.reset (self);

					rlen = pd->read (self,
							dkptr->buffer, len);
					rc = dkptr->rc = rlen;

					dkptr->dbflag = 1;

					// fprintf (stderr, "Auto: %d\n", rc);
				}
			} else {
				fp_off += rc;

				if (kbc_state == 1) {
					if ((k_dkptr = kbc->c_tryreq (kbc))
								!= NULL) {
						memcpy (k_dkptr->buffer,
							dkptr->buffer, rlen);
						k_dkptr->rc = rlen;

						kbc->p_ready (kbc);

						pch->kbuf_pos = fp_off;
						//fprintf (stderr,
						//	"Store data\n");
						kbc->c_release (kbc);
					} else {
						kbc_state = 2;
						fprintf (stderr,
							"[%s] Store data full "
							"%ld[%ld]\n",
							pd->name ? pd->name :
									"",
							pch->kbuf_pos,
							pch->kbuf_pos % 32768);
					}
				}
			}
		}

		bc->p_ready (bc);
	} while (rc > 0);


	fprintf (stderr,
		"\n[%p] %s STREAMMING I/O prefetch thread terminated!\n",
		self, pd->name ? pd->name : "");

	if (pd->endof_prefetch_callback != NULL) {
		// fprintf (stderr, "Call back to %p with %p\n",
		//		pd->endof_prefetch_callback,
		//		pd->endof_prefetch_param);
		pd->endof_prefetch_callback (pd->endof_prefetch_param);
	}


	pd->end_of_prefetch = 1;
	pch->in_running	= 0;

	// return self->pd.read (self, buffer, len);
	return 1;	// terminate
}

static ssize_t pre_read (struct stio_engine_t *self, void *bufptr, size_t len) {
	struct stio_engine_prefetch_t	*pch = self->pd.pch;
	struct buffer_cache_t		*bc  = pch->bc;
	struct disk_buffer_t		*dkptr;
	int				i = 0;
	int				rlen = len;
	char				*buffer = bufptr;

	if (self->pd.abort_flag) return -1;

	// fprintf (stderr, "Pre read (%d,%d)\n", len, pch->tmpbsize);
	if (pch->tmpbsize < 0) return -1;

	if (pch->tmpbsize > 0) {
		if (pch->tmpbsize > len) {
			memcpy (buffer, &pch->tmpbuff[pch->tmpbidx], len);
			pch->tmpbidx  += len;
			pch->tmpbsize -= len;

			return len;
		} else {
			memcpy (buffer, &pch->tmpbuff[pch->tmpbidx],
								pch->tmpbsize);
			i    =  pch->tmpbsize;
			rlen -= i;
			pch->tmpbsize = 0;
			pch->tmpbidx  = 0;
		}
	}

	self->pd.in_read = 1;

	while (rlen > 0) {
		// fprintf (stderr, "Looping rlen = %d\n", rlen);
		dkptr = bc->c_request (bc);

		if (dkptr->rc <= 0) {	// eof
			pch->tmpbsize = -1;
			rlen = 0;
		} else {
			if (self->pd.read_plugin != NULL) {
				if (dkptr->dbflag == 1) {
					self->pd.read_plugin (NULL, 0,
						self->pd.param_for_plugin);
				}

				dkptr->rc = self->pd.read_plugin (
						dkptr->buffer,
						dkptr->rc,
						self->pd.param_for_plugin);
			}
			
			if (dkptr->rc <= 0) {
				pch->tmpbsize = -1;
				rlen = 0;
			} else if (dkptr->rc > rlen) {
				memcpy (&buffer[i], dkptr->buffer, rlen);
				i += rlen;
				pch->tmpbidx  = 0;
				pch->tmpbsize = dkptr->rc - rlen;
				memcpy (pch->tmpbuff, &dkptr->buffer[rlen],
						pch->tmpbsize);
				rlen = 0;
			} else {
				memcpy (&buffer[i], dkptr->buffer, dkptr->rc);
				i    += dkptr->rc;
				rlen -= dkptr->rc;
			}
		}

		bc->c_release (bc);
	}

	self->pd.in_read = 0;

	return i;
	// return self->pd.read (self, buffer, len);
}

static void pre_open_step1 (struct stio_engine_t *self) {
	struct thread_svc_t		*thr;
	struct buffer_cache_t		*bc  = self->pd.pch->bc;

	thr = self->pd.pch->thrd;
	self->pd.pch->tmpbsize = 0;
	self->pd.abort_flag = 0;
	self->pd.repeat_cnt = 0;
	self->pd.pch->in_running = 1;
	bc->cleanup (bc);
	thr->start (thr);
}

static int pre_open (struct stio_engine_t *self, const char *fname) {
	struct stio_engine_pd_t		*pd  = &self->pd;
	struct stio_engine_prefetch_t	*pch = pd->pch;
	int				rc;
	int				have_nf = 0;


	// fprintf (stderr, "prefetch open [%s]\n", fname);

	pch->need_reopen_old_file_if_reset = 0;

	if (self->pd.have_next_file) {
		if (strcmp (fname, self->pd.next_file) == 0) {
			have_nf = 1;
			fprintf (stderr, "Open [%s]: ok\n", fname);
		} else {
			self->pd.have_next_precache = 0;
			pch->stop_pq_prefetch = 1;
		}

		pthread_mutex_lock (&pch->mutex);

		if (self->pd.have_next_file) {
			while (pd->in_pq_prefetch) {
				// 箇更い ... ﹟ゼ更Ч ?
				// 单更挡 ?
				pthread_cond_wait (&pch->condi, &pch->mutex);
			}
			
			if (pch->already_pq_prefetch) {
			}
		}

		self->pd.have_next_file = 0;

		pthread_mutex_unlock (&pch->mutex);
	}

	if ((rc = self->pd.open (self, fname)) >= 0) {
		if (self->pd.pch->enable_kbuf) {
			self->pd.pch->enable_kbuf = 1;
		}

		if (self->pd.read_plugin != NULL) {
			self->pd.read_plugin (NULL, 0,
					self->pd.param_for_plugin);
		}

		pre_open_step1 (self);
	} else {
		fprintf (stderr, "pre open fail for %s (rc = %d)\n",
				fname, rc);
	}

	return rc;
}


static void pre_close_step1 (struct stio_engine_t *self) {
	struct thread_svc_t		*thr = self->pd.pch->thrd;
	struct buffer_cache_t		*bc  = self->pd.pch->bc;
	struct disk_buffer_t		*dkptr;

	self->pd.abort_flag = 1;

	// [NOTE] Some Problem here

	usleep (1);

	if (self->pd.pch->in_running) {
		bc->flush (bc);
	}

	// fprintf (stderr, "Pre close 1 (%p, %p)\n", thr, thr->stop);
	thr->stop (thr);
	// fprintf (stderr, "Pre close 2\n");

	if (self->pd.in_read) {
		dkptr = bc->p_request (bc);
		dkptr->rc = -1;
		bc->p_ready (bc);
	}

	while (self->pd.in_read) usleep (1);


	// bc->flush (bc);
	// fprintf (stderr, "\n[%p] %s To Join thread!\n",
	//			self, self->pd.name ? self->pd.name : "");
	while (self->pd.pch->in_running) usleep (1);
	// fprintf (stderr, "\n[%p] %s thread stop!\n",
	//			self, self->pd.name ? self->pd.name : "");

	bc->flush (bc);
	bc->cleanup (bc);

	self->pd.pch->tmpbsize = -1;
	self->pd.close (self);
}

static void pre_close (struct stio_engine_t *self) {
	pre_close_step1 (self);
	self->pd.close (self);
}

static int pre_reset (struct stio_engine_t *self) {
	struct stio_engine_pd_t		*pd   = &self->pd;
	int				rc;

	pre_close_step1 (self);

	self->stop_pq_prefetch (self);

	if (pd->pch->need_reopen_old_file_if_reset) {
		pd->pch->need_reopen_old_file_if_reset = 0;

		fprintf (stderr, "Re-open [%s]\n", pd->current_file);
		pd->open (self, pd->current_file);
	}

	if ((rc = pd->reset (self)) > 0) {
		if (pd->read_plugin != NULL) {
			pd->read_plugin (NULL, 0, pd->param_for_plugin);
		}
	}

	if (pd->pch->enable_kbuf) {
		// fprintf (stderr, "Prefetch output enable\n");
		pd->pch->enable_kbuf = 2;
	} else {
		// fprintf (stderr, "Nothing with Prefetch output enable\n");
	}

	pre_open_step1 (self);

	return rc;
}

static void pre_abort (struct stio_engine_t *self) {
	self->pd.abort_flag = 1;
	self->pd.abort (self);
	return self->pd.close (self);
}

static void pre_dispose (struct stio_engine_t *self) {
}

static int pub_free_buffer (struct stio_engine_t *self) {
	if (self->pd.pch == NULL) return 0;

	return self->pd.pch->bc->buf_free;
}

static int pub_prefetch_is_running (struct stio_engine_t *self) {
	if (self->pd.pch == NULL) return 0;

	return self->pd.pch->in_running;
}

static int pub_set_buffer (struct stio_engine_t *self, const int nbuf) {
	struct buffer_cache_t		*bc;

	if (self->pd.pch == NULL) return 0;
	if (self->prefetch_is_running (self)) {
		fprintf (stderr,
			"Can not set buffer while prefetch is running\n");
		return 0;
	}

	bc = self->pd.pch->bc;

	return bc->setbuf (bc, nbuf);
}

static int pub_set_prefetch_buffer (struct stio_engine_t *self,
							const int nbuf) {
	struct stio_engine_prefetch_t	*pch = self->pd.pch;

	if (pch == NULL) return 0;

	if (pch->pbc != NULL) {
		if (nbuf < 0) {
			fprintf (stderr, "prefetch buffer clean up\n");
			pch->pbc->cleanup (pch->pbc);
		} else {
			pch->enable_pbuf = nbuf > 0 ? 1 : 0;
			//fprintf (stderr, "Enable prefetch buffer = %d\n",
			//				pch->enable_pbuf);
		}
		return 1;
	} else if (nbuf < 3) {
		// fprintf (stderr, "Buffer too small %d\n", nbuf);
		return 0;
	}

	if (self->prefetch_is_running (self)) {
		fprintf (stderr,
		"Can not set prefetch buffer while prefetch is running\n");
		return 0;
	}

	if ((pch->pbc = new_buffer_cache (nbuf, pch->bsize)) == NULL) {
		return 0;
	}

	pch->pnbuf       = nbuf;
	pch->enable_pbuf = 0;

	// return bc->setbuf (bc, nbuf);
	return 1;
}

static int pub_set_keep_buffer (struct stio_engine_t *self, const int nbuf) {
	struct stio_engine_prefetch_t	*pch = self->pd.pch;

	if (pch == NULL) return 0;

	if (pch->kbc != NULL) {
		if (nbuf < 0) {
			// fprintf (stderr, "Keep buffer clean up\n");
			pch->kbc->cleanup (pch->kbc);
		} else {
			pch->enable_kbuf = nbuf > 0 ? 1 : 0;
			// fprintf (stderr, "Enable Keep buffer = %d\n",
			//				pch->enable_kbuf);
		}
		return 1;
	} else if (nbuf < 3) {
		// fprintf (stderr, "Buffer too small %d\n", nbuf);
		return 0;
	}

	if (self->prefetch_is_running (self)) {
		fprintf (stderr,
			"Can not set keep buffer while prefetch is running\n");
		return 0;
	}

	if ((pch->kbc = new_buffer_cache (nbuf, pch->bsize)) == NULL) {
		return 0;
	}

	pch->knbuf       = nbuf;
	pch->enable_kbuf = 0;

	// return bc->setbuf (bc, nbuf);
	return 1;
}

static int pub_stop_pq_prefetch (struct stio_engine_t *self) {
	struct stio_engine_prefetch_t	*pch = self->pd.pch;
	struct thread_svc_t		*thr = pch->pref_thrd;

	pch->stop_pq_prefetch = 1;
	thr->stop (thr);

	return 1;
}

static int pub_start_pq_prefetch (struct stio_engine_t *self) {
	struct stio_engine_prefetch_t	*pch = self->pd.pch;
	struct thread_svc_t		*thr = pch->pref_thrd;

	self->stop_pq_prefetch (self);
	thr->start (thr);

	return 1;
}

static int pub_prefetch (struct stio_engine_t *self,
					const int nbuf, const int bsize) {
	struct stio_engine_prefetch_t	*pch;

	if (self->pd.pch != NULL) return 0;

	if ((self->pd.pch = malloc (sizeof (struct
			stio_engine_prefetch_t))) == NULL) return 0;

	pch = self->pd.pch;
	pch->nbuf = 0;
	pch->thrd = NULL;
	pch->tmpbuff	= NULL;

	pch->pref_thrd = NULL;

	if ((pch->bc = new_buffer_cache (nbuf, bsize)) == NULL) {
		return pbuf_dispose (self);
	}

	if ((pch->tmpbuff = malloc (bsize)) == NULL) {
		return pbuf_dispose (self);
	}

	pthread_mutex_init (&pch->mutex, NULL);
	pthread_cond_init  (&pch->condi, NULL);

	pch->already_pq_prefetch = 0;
	pch->stop_pq_prefetch = 0;

	pch->kbc   = NULL;
	pch->knbuf = 0;
	pch->enable_kbuf = 0;
	pch->kbuf_pos	 = 0;

	pch->pbc   = NULL;
	pch->pnbuf = 0;
	pch->enable_pbuf = 0;

	pch->nbuf  = nbuf;
	pch->bsize = bsize;
	pch->tmpbsize = 0;
	pch->tmpbidx  = 0;
	pch->in_running = 0;

	if ((pch->thrd = new_thread_svc (prefetch_main, self)) == NULL) {
		return pbuf_dispose (self);
	}

	if ((pch->pref_thrd = new_thread_svc (
					prefetch_queue_main, self)) == NULL) {
		return pbuf_dispose (self);
	}

	// fprintf (stderr, "Thread [%p]\n", pch->thrd);

	self->open		= pre_open;
	self->reset		= pre_reset;
	self->read		= pre_read;
	self->close		= pre_close;
	self->abort		= pre_abort;    // same as close
	self->dispose		= pre_dispose;

	// fprintf (stderr, "STIO prefetch ready !\n");

	return 1;
}

#endif
