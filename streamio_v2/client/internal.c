/*
 *	internal.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __CLIENT_INTERNAL__C__
#define __CLIENT_INTERNAL__C__

static int internal_reopen (struct streamming_io_v2_client_t *, const int);

static void internal_unset_server (struct streamming_io_v2_client_t *self) {
	struct streamming_io_v2_client_pd_t	*pd = &self->pd;

	pthread_mutex_lock (&pd->svr_mutex);

	if (pd->have_server) {
		pd->have_server = 0;
		fprintf (stderr, "unset server\n");
	}

	pthread_mutex_unlock (&pd->svr_mutex);
}

static int internal_send (struct streamming_io_v2_client_t *self,
						void *packet, const int len) {
	struct streamming_io_v2_client_pd_t	*pd = &self->pd;
	struct udplib_t				*udp = self->pd.rudp;
	struct streamming_io_v2_generic_pdu_t	*pdu = packet;
	struct timespec				timeout;
	struct timeval				now;

	while (! self->pd.have_server) {
		fprintf (stderr, "Trying to find server [%p]\n", self);
		self->find_server (self);

		gettimeofday (&now, NULL);
		timeout.tv_sec  = now.tv_sec  + 1;
		timeout.tv_nsec = now.tv_usec * 1000;

		pthread_mutex_lock (&pd->svr_mutex);

		if (! self->pd.have_server) {
			pthread_cond_timedwait (&pd->svr_condi,
					&pd->svr_mutex, &timeout);
		}

		pthread_mutex_unlock (&pd->svr_mutex);

		if (self->pd.abort) {
			fprintf (stderr, "Abort find server [%p]!\n", self);
			return -1;
		}
	}

	pdu->cksum = self->cksum (packet, len);
	udp->sendto (udp, 0, packet, len);

	return 1;
}


static int internal_sendwait (struct stio_v2_client_local_t *cl,
				void *packet, const int len,
				volatile int *variable, const int op,
				const int unexpect, const int wf,
				const int wait_useconds, const int retry) {
	//struct streamming_io_v2_client_pd_t	*pd = &self->pd;
	struct timeval				now;
	struct timespec				timeout;
	int					count = 0;
	int					rc;
	int					retval = 0;
	long					usec;

	set_compare (cl, variable, op, unexpect);

	if (! wf) {	// wait first
		if (internal_send (cl->parent, packet, len) < 0) return -1;
		usleep (1);
	}

	while (do_compare (cl)) {
		rc = 0;

		pthread_mutex_lock (&cl->mutex);

		if (do_compare (cl)) {
			gettimeofday (&now, NULL);

			if ((usec = now.tv_usec + wait_useconds) < 1000000) {
				timeout.tv_sec  = now.tv_sec;
				timeout.tv_nsec = usec * 1000;
			} else {
				timeout.tv_sec  = now.tv_sec + (usec / 1000000);
				timeout.tv_nsec = (usec % 1000000) * 1000;
			}

			rc = pthread_cond_timedwait (&cl->condi,
							&cl->mutex, &timeout);
		}

		pthread_mutex_unlock (&cl->mutex);

		if (do_compare (cl)) {
			if (cl->need_reopen) {
				fprintf (stderr, "Re-OPEN is required!\n");
				cl->need_reopen = 0;
				retval = 1;
				break;
			} else if (rc == ETIMEDOUT) {
				if (retry == 0) {
					retval = 2;
					break;
				}

				fprintf (stderr,
					"Re-send (timeout %d usec, VAR=%d)\n",
					wait_useconds, *variable);

				if (++count >= retry) {
					internal_unset_server (cl->parent);
					count = 0;
				}

				if (internal_send (cl->parent,
							packet, len) <0) {
					// fprintf (stderr, "CUT\n");
					retval = -1;
					break;
				}

				usleep (1);
			}
		}
	}

	if (retval == 1) {
		internal_reopen (cl->parent, cl->channel);
	}

	return retval;
}

static int internal_reopen (struct streamming_io_v2_client_t *self,
						const int channel) {
	struct stio_v2_client_local_t	*cl;

	cl = &self->pd.cl[channel];

	cl->fsize = 0;
	cl->need_filesize = 1;

	fprintf (stderr, "STIOv2(Internal Reopen)\n");

	while (internal_sendwait (cl, &cl->fopen_pdu, sizeof cl->fopen_pdu,
					&cl->need_filesize, CMP_EQ, 1,
					0, 500000, 3) != 0) {
		// if (about) break;
		if (self->pd.abort) return -1;
	}

	return cl->fsize > 0 ? cl->fsize : -1;
}

static int internal_read (struct stio_v2_client_local_t	*cl) {
	struct streamming_io_v2_client_t	*self = cl->parent;
	int					i, n, len;
	int					rc = 0;

	cl->fread_pdu.cmd	= STIOV2_CMD_FETCH;
	cl->fread_pdu.channel	= cl->channel;
	cl->fread_pdu.sn	= self->pd.sn;
	cl->fread_pdu.filecnt	= cl->filecnt;
	cl->fread_pdu.ndcnt	= cl->ndcnt;

	if ((len = cl->fsize - cl->ndcnt * cl->net_bufsize)
						>= cl->net_bufsize) {
		cl->fread_pdu.len = cl->net_bufsize;
		n		  = cl->packet_map;
		rc = cl->net_bufsize;
	} else {
		cl->fread_pdu.len = len;
		n = len / STIOV2_CARRY_DATASIZE;
		if (len % STIOV2_CARRY_DATASIZE != 0) n++;
		rc = len;
	}

	cl->request_map = n;

	for (i = 0; i < n; i++) cl->fread_pdu.map[i] = 1;
	for (; i < cl->packet_map; i++) cl->fread_pdu.map[i] = 0;

	while (internal_sendwait (cl, &cl->fread_pdu, sizeof cl->fread_pdu,
					&cl->request_map, CMP_NE, 0,
					0, 500000, 3) != 0) {
		if (self->pd.abort) return -1;
	}

	return rc;
}

#endif
