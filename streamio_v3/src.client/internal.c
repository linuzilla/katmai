/*
 *	internal.c (streamio_v3 for client)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#define CMP_GT		1
#define CMP_GE		2
#define CMP_LT		3
#define CMP_LE		4
#define CMP_EQ		5
#define CMP_NE		6

#define SENDWAIT_WAIT_FIRST	1
#define SENDWAIT_SEND_FIRST	0

#define PUBLIC_CHANNEL		4

static int internal_reopen_file (struct streamming_io_v3_client_t *self,
							const int channel);

static void unset_VODserver (struct streamming_io_v3_client_t *self) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	struct vodsvr_t				*vodsvr = pd->vodsvr;

	pthread_mutex_lock (&pd->vod_mutex);

	if (pd->current_VODserver >= 0) {
#if DEBUG_STIOV3 > 1
		fprintf (stderr, "Unset VOD server %d\n",
				pd->current_VODserver);
#endif
		vodsvr->unset_registcode (pd->current_VODserver);
		pd->current_VODserver = -1;
		pd->have_VODserver = 0;
#if ENABLE_PREFER_VOD_SERVER == 1
		pd->curr_svr_id = -1;
#endif
	}
	pthread_mutex_unlock (&pd->vod_mutex);
}

static void set_compare (struct streamming_io_v3_client_t *self,
			const int channel,
			volatile int *variable, const int op, const int exp) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;

	pd->compare_variable[channel] = variable;
	pd->compare_operator[channel] = op;
	pd->compare_expect[channel]   = exp;
}

static int do_compare (struct streamming_io_v3_client_t *self,
							const int channel) {

	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	volatile int	*compare_variable = pd->compare_variable[channel];
	int		compare_expect = pd->compare_expect[channel];

	switch (pd->compare_operator[channel]) {
	case CMP_GT:
		if (*compare_variable > compare_expect) return 1;
		break;
	case CMP_GE:
		if (*compare_variable >= compare_expect) return 1;
		break;
	case CMP_LT:
		if (*compare_variable < compare_expect) return 1;
		break;
	case CMP_LE:
		if (*compare_variable <= compare_expect) return 1;
		break;
	case CMP_EQ:
		if (*compare_variable == compare_expect) return 1;
		break;
	case CMP_NE:
		if (*compare_variable != compare_expect) return 1;
		break;
	}
	return 0;
}

static int sendto_vod (struct streamming_io_v3_client_t *self,
					const int channel,
					void *buffer, const int len) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	struct vodsvr_t				*vodsvr = pd->vodsvr;
	struct udplib_t				*udp = pd->udp;
	struct stio_v3_pdu_t			*ptr = buffer;
	int					i, rc;
	struct timeval				now;
	struct timespec				timeout;

	static int				wait_channel = -1;
	static pthread_mutex_t		mutex = PTHREAD_MUTEX_INITIALIZER; 


#if DEBUG_STIOV3 > 2
	fprintf (stderr, "sendto_vod [%d]\n", ptr->cmd);
#endif
	while ((i = pd->current_VODserver) < 0) {

		if (pd->cut && channel != PUBLIC_CHANNEL) {
			pthread_mutex_lock (&mutex);
			if (wait_channel == channel) wait_channel = -1;
			pthread_mutex_unlock (&mutex);

			return -1;
		} else if (channel != wait_channel) {
			pthread_mutex_lock (&mutex);
			if (wait_channel < 0) wait_channel = channel;
			pthread_mutex_unlock (&mutex);

			if (channel != wait_channel) {
				usleep (1000);
				continue;
			}
		}

		self->find_server (self);

		gettimeofday (&now, NULL);
		timeout.tv_sec  = now.tv_sec + 1;
		timeout.tv_nsec = now.tv_usec * 1000;

		pthread_mutex_lock (&pd->vod_mutex);

		if (pd->current_VODserver < 0) {
			rc = pthread_cond_timedwait (&pd->vod_condi,
						&pd->vod_mutex, &timeout);
		}

		pthread_mutex_unlock (&pd->vod_mutex);
	}

	pthread_mutex_lock (&mutex);
	if (wait_channel == channel) wait_channel = -1;
	pthread_mutex_unlock (&mutex);


	ptr->regist.sn = vodsvr->regist_code (i);

	pthread_mutex_lock (&pd->mutex[channel]);
	ptr->cksum     = self->cksum (buffer, len);

	/*
	if (ptr->cmd == STBVOD_PDU_CMD_FETCH) {
		fprintf (stderr, "Block Request=%d\n", stmreq->h.block);
	}
	*/

#if DEBUG_STIOV3 > 2
	fprintf (stderr, "sendto_vod (%d)\n", i);
#endif
	udp->sendto (udp, vodsvr->udp_entry (i) , buffer, len);
	pthread_mutex_unlock (&pd->mutex[channel]);

	return i;
}

static int send_and_wait (struct streamming_io_v3_client_t *self,
			const int channel,
			void *pdu, const int len,
			volatile int *variable,
			const int op, const int unexpect, const int wf,
			const int wait_useconds, const int retry) {

	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	struct timeval		now;
	struct timespec		timeout;
	int			count = 0;
	int			rc;
	int			retval = 0;
	long			usec;

	set_compare (self, channel, variable, op, unexpect);

#if DEBUG_STIOV3 > 3
	fprintf (stderr, "Send and wait (pass one)\n");
#endif
	if (! wf) {
		sendto_vod (self, channel, pdu, len);
		usleep (1);
	}
	// pthread_mutex_lock (&mutex);
#if DEBUG_STIOV3 > 3
	fprintf (stderr, "Send and wait (pass two)\n");
#endif
	while (do_compare (self, channel)) {
		rc = 0;
		pthread_mutex_lock (&pd->mutex[channel]);
		if (do_compare (self, channel)) {
			gettimeofday (&now, NULL);

			if ((usec = now.tv_usec + wait_useconds) < 1000000) {
				timeout.tv_sec  = now.tv_sec;
				timeout.tv_nsec = usec * 1000;
			} else {
				timeout.tv_sec  = now.tv_sec + (usec / 1000000);
				timeout.tv_nsec = (usec % 1000000) * 1000;
			}
			rc = pthread_cond_timedwait (
					&pd->condi[channel],
					&pd->mutex[channel], &timeout);
		}
		pthread_mutex_unlock (&pd->mutex[channel]);

		if (do_compare (self, channel)) {
			if (pd->need_reopen[channel]) {
#if DEBUG_STIOV3 > 1
				fprintf (stderr,
					"Re-OPEN is required on channel %d!\n",
					channel);
#endif
				// reopen_remote_mpeg_file ();
				pd->need_reopen[channel] = 0;
				retval = 1;
				break;
			} else if (rc == ETIMEDOUT) {
				if (retry == 0) {
					retval = 2;
					break;
				}

#if DEBUG_STIOV3 > 2
				fprintf (stderr,
					"Channel %d re-send (timeout %d usec, "
					"VAR=%d)\n",
					channel,
					wait_useconds, *variable);
#endif

#if ENABLE_PREFER_VOD_SERVER == 1
				if (pd->curr_svr_id == pd->prefsvr_id) {
					if (++count == retry + 5) {
						unset_VODserver (self);
						count = 0;
					}
				} else {
					if (++count == retry) {
						unset_VODserver (self);
						count = 0;
					}
				}
#else
				if (++count == retry) {
					// if (channel == 0) {
					//	unset_VODserver (self);
					// }
					unset_VODserver (self);
					count = 0;
				}
#endif

				if (sendto_vod (self, channel, pdu, len) < 0) {
#if DEBUG_STIOV3 > 1
					fprintf (stderr, "CUT\n");
#endif
					retval = -1;
					break;
				}

				usleep (1);
			} else {
#if DEBUG_STIOV3 > 2
				fprintf (stderr, "Check condition (signal)\n");
#endif
			}
		}
	}

	// pthread_mutex_unlock (&mutex);

	if (retval == 1) internal_reopen_file (self, channel);

	return retval;
}

static int internal_reopen_file (struct streamming_io_v3_client_t *self,
							const int channel) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;

	pd->filesize[channel] = 0;
	pd->need_filesize[channel] = 1;

	pd->reopen_pdu[channel]->fileio.stbid = pd->stbid;

#if DEBUG_STIOV3 > 3
	fprintf (stderr, "Internal openfile (1,%d)\n",
			pd->reopen_pdu[channel]->cmd);
#endif

	while (send_and_wait (self, channel,
			pd->reopen_pdu[channel],
			pd->reopen_pdu_len[channel],
			&pd->need_filesize[channel], CMP_EQ, 1,
			SENDWAIT_SEND_FIRST,
			pd->timeout_openfile, pd->retry_openfile) != 0) {
		if (pd->cut && channel != PUBLIC_CHANNEL) return 0;
	}

#if DEBUG_STIOV3 > 3
	fprintf (stderr, "Internal openfile (2)\n");
#endif

	if (pd->filesize[channel] < 0) {
#if DEBUG_STIOV3 > 2
		fprintf (stderr, "Internal reopen (%s)\n",
				strerror (pd->open_errno[channel]));
#endif
		return -1;
	}

	pd->blockcnt[channel] = pd->filesize[channel] / 32768;

	if (pd->filesize[channel] % 32768 != 0) pd->blockcnt[channel]++;

#if DEBUG_STIOV3 > 1
	fprintf (stderr, "File size is %ld (%d blocks) on channel %d\n",
			pd->filesize[channel], pd->blockcnt[channel],
			channel);
#endif

	// wait until response ....
	
	return 1;
}

// int streamming_read_mpeg_file  (struct disk_buffer_t *ptr) {
static int internal_read (struct streamming_io_v3_client_t *self,
				const int channel,
				char *buffer) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	int	i;
	int	len;
	int	expect_len;
	int	delay;
	int	retry;
	int	lastval;
	int	rc;
	struct stiov3_fetch_request_t	*stmreq = pd->stmreq[channel];

	if (pd->blockwant[channel] >= pd->blockcnt[channel]) {
		/*
		fprintf (stderr, "[-] black want %d >= %d\n",
				pd->blockwant[channel],
				pd->blockcnt[channel]);
		*/
		// ptr->rc = -1;
		return -1;
	}

	pd->stmpdu[channel]->cmd     = STIOV3_PDU_CMD_FETCH;

	stmreq->h.block = pd->blockwant[channel];

	if (pd->blockwant[channel] < pd->blockcnt[channel] - 1) {
		expect_len = 32768;
	} else {
		if ((expect_len = (pd->filesize[channel] % 32768)) == 0) {
			expect_len = 32768;
		}
	}

	pd->brcvcnt[channel] = 23;
	for (i = 0; i < 23; i++) {
		pd->bufok[channel][i]       = 1;
		stmreq->map[i] = 1;
	}

	len = ((int) &stmreq->map[24]) - ((int) pd->stmpdu[channel]);

	pd->vodsvr_response[channel] = 0;

	/*
		fprintf (stderr, "Send FETCH request, len = %d/block=%d\n",
				       	len, stmreq->h.block);
	*/

	// if ((stmreq->qos = bcptr->num - bcptr->buf_free) < qos_threshold) {
	//	stmpdu->cmd = STBVOD_PDU_CMD_FAST_FETCH;
	// }

	while (send_and_wait (self, channel,
				pd->stmpdu[channel], len,
				&pd->vodsvr_response[channel], CMP_EQ, 0,
				SENDWAIT_SEND_FIRST,
				pd->timeout_response,
				pd->retry_response) != 0) {
		if (pd->cut && channel != PUBLIC_CHANNEL) {
			// fprintf (stderr, "[+] End of file due to cut (1)\n");
			return -1;
		}
	}

	lastval = 23;

	while (pd->brcvcnt[channel] != 0) {
		// stmreq->qos = bcptr->num - bcptr->buf_free;
		stmreq->qos = 90;

		if (pd->brcvcnt[channel] < lastval) {
			// Server 有送一些
			delay = 100;
			retry = 0;
			lastval = pd->brcvcnt[channel];
		} else {
			// Server 完全沒送
			if (stmreq->qos < pd->qos_threshold) {
				delay = pd->streamming_delay;
				pd->stmpdu[channel]->cmd =
						STIOV3_PDU_CMD_FAST_FETCH;
			} else {
				delay = pd->streamming_delay << 3;
			}

			retry = 2;
		}

		if (pd->brcvcnt[channel] != 0) {
			send_and_wait (self, channel,
					pd->stmpdu[channel], len,
					&pd->brcvcnt[channel], CMP_GT, 0,
					SENDWAIT_WAIT_FIRST,
					delay, retry);
		}

		if (pd->cut && channel != PUBLIC_CHANNEL) {
			// fprintf (stderr, "[+] End of file due to cut (2)\n");
			return -1;
		}
	}


	memcpy (buffer, pd->bbuf[channel], expect_len);
	rc = expect_len;

#if ENABLE_MD5SUM_FOR_STREAMMING_FILE == 1
	md5_process_bytes (pd->bbuf[channel],
			(unsigned long) expect_len, &pd->md5ctx[channel]); 
#endif

	pd->blockwant[channel]++;

	return rc;
}

// static int internal_closefile (struct disk_buffer_t *ptr) {
static int internal_closefile (struct streamming_io_v3_client_t *self,
						const int channel) {
#if ENABLE_MD5SUM_FOR_STREAMMING_FILE == 1
	int		i;
	unsigned char	md[MD5_DIGEST_LENGTH];

	if (self->pd.have_md5sum[channel]) {
		fprintf (stderr, "\rStreamming IO (%d) closed: MD5=", channel);

		md5_finish_ctx (&self->pd.md5ctx[channel], &(md[0]));

		for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
			fprintf(stderr, "%02x",md[i]);
		}

		fprintf(stderr, "\n");
	}
	self->pd.have_md5sum[channel] = 0;
#else
	// fprintf (stderr, "\rStreamming IO closed\n");
#endif

	return 0;
}
