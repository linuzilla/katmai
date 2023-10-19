/*
 *	response.c (streamio_v3 for client)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

static int streamio_v3_response (void *selfptr) {
	struct streamming_io_v3_client_t	*self = selfptr;
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	struct udplib_t				*udp = pd->udp;
	struct vodsvr_t				*vodsvr = pd->vodsvr;
	char					buffer[2000];
#if ENABLE_PREFER_VOD_SERVER != 1
	char					*ipptr;
#endif
	int					i, vi, len, channel, cksum;
	struct stio_v3_pdu_t			*ptr = (void *) buffer;
	struct stiov3_regist_t			*svrptr = &ptr->regist;
	struct stiov3_fileio_t			*fioptr = &ptr->fileio;
	//struct stb_vod_fetch_request_t	*frqptr = (void *) &ptr->fileio;
	struct stiov3_fetch_data_t		*fdtptr = (void *) &ptr->fileio;


	while (! pd->terminate) {
		if ((len = udp->receive (udp, buffer, sizeof buffer)) < 0) {
			if (errno == EAGAIN) {
				usleep (20000);
			} else {
				perror (NULL);
			}
			continue;
		} else if (len == 0) {
			continue;
		}


		cksum = self->cksum (buffer, len);

		if (cksum != ptr->cksum) {
#if DEBUG_STIOV3 > 1
			fprintf (stderr, "Checksum error!!\n");
#endif
			continue;
		}

#if DEBUG_STIOV3 > 3
		fprintf (stderr, "Received from VODSVR: %s:%d (%d)(cmd=%d)\n",
				udp->remote_addr (udp),
				udp->remote_port (udp),
				len, ptr->cmd);
#endif

		switch (ptr->cmd) {
		case STIOV3_PDU_CMD_NEED_REGIST:
#if DEBUG_STIOV3 > 3
			fprintf (stderr, "CLIENT RESPONSE: Need Regist\n");
#endif
			vi = vodsvr->add (udp->get_remote (udp));
			if (pd->current_VODserver == vi) {
				vodsvr->unset_registcode (vi);
				pd->current_VODserver = -1;
				pd->have_VODserver    = 0;
#if DEBUG_STIOV3 > 3
				fprintf (stderr, "Lost VODSVR\n");
#endif
			}

			break;
		case STIOV3_PDU_CMD_FINDSERVER:
#if DEBUG_STIOV3 > 3
			fprintf (stderr, "CLIENT RESPONSE: FineServer\n");
#endif
			if ((vi = vodsvr->add (udp->get_remote (udp))) < 0) {
				fprintf (stderr, "No room for vod server\n");
				break;
			}

#if DEBUG_STIOV3 > 1
			fprintf (stderr, "Find a VOD server (%d) [%s]\n",
					vi, udp->remote_addr (udp));
#endif

			if (pd->current_VODserver < 0) {
				ptr->cmd = STIOV3_PDU_CMD_REGIST;
				ptr->cksum = self->cksum (buffer, len);
				udp->send (udp, buffer, len);
				/*
				if (vodsvr->regist_code (i) == 0) {
					ptr->cmd = STBVOD_PDU_CMD_REGIST;
					ptr->cksum = vodstb->cksum (
								buffer, len);
					udp->send (udp, buffer, len);
				} else {
					current_VODserver = i;
					have_VODserver = 1;
				}
				*/
#if ENABLE_PREFER_VOD_SERVER == 1
			} else if (pd->curr_svr_id != pd->prefsvr_id) {
				// regist for prefer server only !!
				if (pd->curr_svr_id ==
						ntohl (udp->get_remote (udp)) %
						pd->prefsvr_divider) {
					fprintf (stderr,
						"Find a better server "
						"(%d) [%s]\n",
						vi, udp->remote_addr (udp));

					ptr->cmd = STIOV3_PDU_CMD_REGIST;
					ptr->cksum = self->cksum (buffer, len);
					udp->send (udp, buffer, len);
				}
#endif
			}
			/*
				if (vodsvr->regist_code (i) == 0){
					ptr->cmd = STBVOD_PDU_CMD_REGIST;
					ptr->cksum = vodstb->cksum (
							buffer, len);
					udp->send (udp, buffer, len);
				} else if ((i >= 0)&&(current_VODserver < 0)) {
					pthread_mutex_lock (&vod_mutex);
					current_VODserver = i;
					have_VODserver    = 1;
					pthread_cond_signal (&vod_condi);
					pthread_mutex_unlock (&vod_mutex);
				}
			*/

			break;
		case STIOV3_PDU_CMD_REGIST:
			i  = udp->regist_rmaddr (udp);
			vi = vodsvr->set_registcode (
						udp->get_remote (udp),
						svrptr->sn, i);
#if DEBUG_STIOV3 > 2
			fprintf (stderr, "My ID=%d [%s](%d,%d)\n",
						svrptr->sn,
						udp->ip_addr (udp, i),
						i, vi);
#endif

			if (pd->current_VODserver < 0) {
				pthread_mutex_lock (&pd->vod_mutex);
				pd->current_VODserver = vi;
				pd->have_VODserver = 1;
				pd->qos_threshold = svrptr->threshold;

				pd->server_port = udp->udp_port (udp, i);
#if ENABLE_PREFER_VOD_SERVER == 1
				pd->file_server = pd->curr_svr_id =
					ntohl (udp->get_remote (udp)) %
					pd->prefsvr_divider;

				fprintf (stderr,
						"QOS Threshold=%d, "
						"Server Version=%d.%d,"
						"(%d v.s. %d)\n",
						pd->qos_threshold,
						svrptr->version_major,
						svrptr->version_minor,
						pd->curr_svr_id,
						pd->prefsvr_id);
#else
				ipptr = udp->ip_addr (udp, i);

				strcpy (pd->server_ip, ipptr);

#  if DEBUG_STIOV3 > 1
				fprintf (stderr,
						"QOS Threshold=%d, "
						"Server Version=%d.%d,"
						"[%s](%d)\n",
						pd->qos_threshold,
						svrptr->version_major,
						svrptr->version_minor,
						ipptr, pd->current_VODserver);
#  endif

				// for STATE querry usage only
				i = ipptr[strlen(ipptr) - 1] - '0';
				if ((i % 2) == 0) {
					pd->file_server = 0;
				} else {
					pd->file_server = 1;
				}
#endif

				pthread_cond_signal (&pd->vod_condi);
				pthread_mutex_unlock (&pd->vod_mutex);
#if ENABLE_PREFER_VOD_SERVER == 1
			} else if (pd->curr_svr_id != pd->prefsvr_id) {
				// regist for prefer server only !!
				if (pd->curr_svr_id ==
						ntohl (udp->get_remote (udp)) %
						pd->prefsvr_divider) {
					fprintf (stderr,
						"Regist to a better server "
						"(%d) [%s]\n",
						vi, udp->remote_addr (udp));

					pthread_mutex_lock (&pd->vod_mutex);
					pd->current_VODserver = vi;
					pd->have_VODserver = 1;
					pd->qos_threshold = svrptr->threshold;

					pd->server_port =
							udp->udp_port (udp, i);
					pd->file_server = pd->curr_svr_id =
						ntohl (udp->get_remote (udp)) %
						pd->prefsvr_divider;

					pthread_cond_signal (&pd->vod_condi);
					pthread_mutex_unlock (&pd->vod_mutex);
				}
#endif
			}
			break;
		case STIOV3_PDU_CMD_OPENFILE:
			channel = fioptr->channel;

#if DEBUG_STIOV3 > 3
			fprintf (stderr, "OPENFILE on channel %d\n", channel);
#endif
			if (pd->need_filesize[channel]) {
				if ((pd->filesize[channel] =
						fioptr->filesize)== 0) {
					pd->filesize[channel] = -1;
				}
				pd->need_filesize[channel] = 0;
				pd->need_reopen[channel]   = 0;

				if (pd->may_need_fix_filename[channel]) {
					pd->may_need_fix_filename[channel] = 0;

					if (fioptr->file[0] != '\0') {
						strcpy (
						    pd->reopen_pdu[channel]->
							fileio.file,
							fioptr->file);
						fprintf (stderr,
							"Magic file [%s]\n",
							fioptr->file);
					}
				}

				pthread_mutex_lock   (&pd->mutex[channel]);
				pthread_cond_signal  (&pd->condi[channel]);
				pthread_mutex_unlock (&pd->mutex[channel]);
			}
			pd->open_errno[channel] = fioptr->err;
			break;
		case STIOV3_PDU_CMD_FETCH_RESPONSE:
			channel = fioptr->channel;

			if ((fdtptr->h.block == pd->blockwant[channel]) && 
			       (fdtptr->h.filecnt == pd->filecnt[channel])) {
				pd->vodsvr_response[channel] = 1;
			}
			break;
		case STIOV3_PDU_CMD_NEED_REOPEN:
			channel = fioptr->channel;

			if ((fdtptr->h.block == pd->blockwant[channel]) && 
			       (fdtptr->h.filecnt == pd->filecnt[channel])) {
#if DEBUG_STIOV3 > 1
				fprintf (stderr, "Need reopen on channel %d\n",
						channel);
#endif
				// streamming_init_mpeg_file ();
				pd->need_reopen[channel] = 1;

				pthread_mutex_lock   (&pd->mutex[channel]);
				pthread_cond_signal  (&pd->condi[channel]);
				pthread_mutex_unlock (&pd->mutex[channel]);
			} else {
#if DEBUG_STIOV3 > 1
				fprintf (stderr,
					"reopen ignore on channel %d\n",
					channel);
#endif
			}
			break;
		case STIOV3_PDU_CMD_FAST_FETCH:
		case STIOV3_PDU_CMD_FETCH:
			channel = fioptr->channel;

#if DEBUG_STIOV3 > 3
			fprintf (stderr, "Fetch data on channal %d comming "
					"(block=%d, bkwant=%d)\n",
					channel,
					fdtptr->h.block,
					pd->blockwant[channel]);
#endif

			/*
				fprintf (stderr,
					"Fetch data comming ... (%d,%d)",
					fdtptr->h.block, blockwant);
					*/
			if ((fdtptr->h.block == pd->blockwant[channel]) && 
				(fdtptr->h.filecnt == pd->filecnt[channel])) {

				pd->vodsvr_response[channel] = 1;

				pthread_mutex_lock (&pd->mutex[channel]);

				i = fdtptr->bid;
					//fprintf (stderr, "(%d,%d)", i,
					 //		fdtptr->len);
				if (pd->bufok[channel][i] == 1) {
					pd->bufok[channel][i] = 0;
					pd->stmreq[channel]->map[i] = 0;
					memcpy (pd->bbuf[channel][i],
							fdtptr->buffer,
							fdtptr->len);

					if (--(pd->brcvcnt[channel]) == 0) {
						pthread_cond_signal (
							&pd->condi[channel]);
					}
					// fprintf (stderr, "Yes\n");
				} else {
					// fprintf (stderr, "ok\n");
				}

				pthread_mutex_unlock (&pd->mutex[channel]);
			} else {
#if DEBUG_STIOV3 > 2
				fprintf (stderr,
						"Unexpected fetch data [%d]"
						"(%d/%d)(%d/%d)\n",
						channel,
						fdtptr->h.block,
						pd->blockwant[channel],
						fdtptr->h.filecnt,
						pd->filecnt[channel]);
#endif
			}
			break;
		case STIOV3_PDU_CMD_VODSTOP:
			break;
		default:
			break;
		}
	}

	return 0;
}
