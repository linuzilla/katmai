/*
 *	client.c (streamio_v3)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#include "streamio_v3.h"
#include "streamio_v3_pdu.h"
#include "udplib.h"
#include "thread_svc.h"
#include "vod_table.h"
#include "stio_engine.h"

#include "common/cksum.c"
#include "src.client/response.c"
#include "src.client/internal.c"
#include "src.client/stio_eng.c"

#define DEFALUT_DELAY_OPENFILE	500000
#define DEFALUT_DELAY_RESPONSE	 20000
#define DEFALUT_DELAY_SENDDATA	 50000

static int find_fileserver (struct streamming_io_v3_client_t *self) {
	// struct stb_fsfs_mcast_pdu	pdu;
	struct stio_v3_pdu_t	pdu;
	struct udplib_t		*udp = self->pd.udp;

	// 看看關於 Multicasting 的準備工作是不是做好了 ...
	//if (! self->pd.ready) return 0;

	pdu.cmd = STIOV3_PDU_CMD_FINDSERVER;
	// 準備好了, 就送一個 Broadcast 的封包去問網路上的 Server
	//
	// 要注意的是, 在此並不檢查 Server 是否 response
	// 而是在 response_main 中檢查

#if DEBUG_STIOV3 > 1
	fprintf (stderr, "Send packet to find VOD Server\n");
#endif
	pdu.cksum = self->cksum (&pdu, sizeof pdu);
	udp->bsend (udp, self->pd.port, &pdu, sizeof pdu);

	return 1;
}

static int stiov3_start (struct streamming_io_v3_client_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.terminate = 0;
	thr->conti (thr);

	return thr->start (thr);
}

static int stiov3_stop (struct streamming_io_v3_client_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.terminate = 1;
	return thr->stop (thr);
}

static void stiov3_dispose (struct streamming_io_v3_client_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;
	struct udplib_t		*udp = self->pd.udp;

	self->stop (self);

	if (thr != NULL) thr->dispose (thr);
	if (udp != NULL) udp->close   (udp);
	if (udp != NULL) udp->dispose (udp);

	free (self);
}

static int stiov3_reopen (struct streamming_io_v3_client_t *self,
					const int channel) {
	self->pd.blockwant[channel] = 0;
	self->pd.rdbufidx[channel]  = 0;
	self->pd.rdbuflen[channel]  = 0;

#if ENABLE_MD5SUM_FOR_STREAMMING_FILE == 1
	self->pd.have_md5sum[channel] = 1;
	md5_init_ctx (&self->pd.md5ctx[channel]);
#endif
	return 1;
}

static int stiov3_seek (struct streamming_io_v3_client_t *self,
					const int channel, const off_t pos) {
	// int	old = self->pd.blockwant[channel];

	self->pd.blockwant[channel] = pos / 32768;
	self->pd.rdbufidx[channel]  = 0;
	self->pd.rdbuflen[channel]  = 0;

	/*
	fprintf (stderr, "stiov3_seek: want = %d -> %d\n",
					old,
					self->pd.blockwant[channel]);
	*/

	return self->pd.blockwant[channel] * 32768;
}

static int stiov3_openfile (struct streamming_io_v3_client_t *self,
					const int channel, const char *fname) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	int					len;

	if ((len = strlen (fname)) > 100) {
		fprintf (stderr, "filename too long ...[%s]\n", fname);
		return -1;
	}

	pd->stmpdu[channel]->cmd = STIOV3_PDU_CMD_OPENFILE;
	pd->stmptr[channel]->channel = channel;
	strcpy (pd->stmptr[channel]->file, fname);
	pd->stmptr[channel]->fnlen = len;
	pd->stmptr[channel]->filecnt = ++pd->filecnt[channel];

	if (channel == 0) {
		pd->may_need_fix_filename[channel] = 1;
	} else {
		pd->may_need_fix_filename[channel] = 0;
	}

	pd->reopen_pdu_len[channel] = sizeof (struct stio_v3_pdu_t);

	if (len > STIOV3_DEFAULT_FILENAME_LEN) {
		pd->reopen_pdu_len[channel] = pd->reopen_pdu_len[channel] -
				STIOV3_DEFAULT_FILENAME_LEN + len;
	}

	memcpy (pd->reopen_pdu[channel], pd->stmpdu[channel],
						pd->reopen_pdu_len[channel]);

#if DEBUG_STIOV3 > 1
	fprintf (stderr, "CLIENT: Open file [%s] on channel %d, cmd=%d\n",
			fname, channel,
			pd->reopen_pdu[channel]->cmd);
#endif

	self->pd.cut = 0;

	self->reopen (self, channel);

	return internal_reopen_file (self, channel);
}


static off_t stiov3_filesize (struct streamming_io_v3_client_t *self,
						const int channel) {
	return self->pd.filesize[channel];
}

static int stiov3_partial_read (struct streamming_io_v3_client_t *self,
					const int channel,
					char *buffer, const int len) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	int					rc = -1;

	if (len <= 0) return len;

	if (pd->rdbuflen[channel] > 0) {
		if ((rc = pd->rdbuflen[channel]) <= len) {
			memcpy (buffer,
				&pd->readbuffer[channel][pd->rdbufidx[channel]],
				rc);
			pd->rdbuflen[channel] = 0;
			pd->rdbufidx[channel] = 0;
		} else {
			memcpy (buffer,
				&pd->readbuffer[channel][pd->rdbufidx[channel]],
				len);
			pd->rdbuflen[channel] -= len;
			pd->rdbufidx[channel] += len;
			rc = len;
		}
	} else if (len < 32768) {
		rc = internal_read (
				self, channel,
				pd->readbuffer[channel]);

		if (rc > 0) {
			if (rc <= len) {
				memcpy (buffer, pd->readbuffer[channel], rc);
			} else {
				memcpy (buffer, pd->readbuffer[channel], len);

				pd->rdbuflen[channel] = rc - len;
				pd->rdbufidx[channel] = len;

				rc = len;
			}
		}
	} else {
		rc = internal_read (self, channel, buffer);
	}

	return rc;
}

static int stiov3_read (struct streamming_io_v3_client_t *self,
					const int channel,
					char *buffer, const int len) {
	int	explen = len;
	int	rl, bs, rc = 0;

	for (bs = rl = 0, explen = len; explen > 0; explen -= rl) {
		rl = stiov3_partial_read (self, channel, &buffer[bs], explen);

		if (rl <= 0) {
			// fprintf (stderr, "[+] stiov3_read (end of file)\n");
			break;
		}

		rc += rl;
		bs += rl;
	}

	return rc;
}

static int stiov3_closefile (struct streamming_io_v3_client_t *self,
							const int channel) {
	return internal_closefile (self, channel);
}

static void stiov3_abort (struct streamming_io_v3_client_t *self) {
	// fprintf (stderr, "[+] CUT signal received\n");
	self->pd.cut = 1;
}

static int stiov3_get_server (struct streamming_io_v3_client_t *self,
						char **server, int **port) {
	*server = self->pd.server_ip;
	*port = &self->pd.server_port;

	return self->pd.have_VODserver;
}

static int stiov3_get_server_id (struct streamming_io_v3_client_t *self) {
	return self->pd.file_server;
}


static int stiov3_set_delay (struct streamming_io_v3_client_t *self,
					const int which, const int usec) {
	struct streamming_io_v3_client_pd_t	*pd = &self->pd;
	int					rc = 0;

	switch (which) {
	case 1:	// File Open
		rc = pd->timeout_openfile;
		
		if (usec > 0) {
			pd->timeout_openfile = usec;
		} else if (usec < 0){
			pd->timeout_openfile = DEFALUT_DELAY_OPENFILE;
		}
		break;
	case 2: // Response
		rc = pd->timeout_response;

		if (usec > 0) {
			pd->timeout_response = usec;
		} else if (usec < 0){
			pd->timeout_response = DEFALUT_DELAY_RESPONSE;
		}
		break;
	case 3: // Send data
		rc = pd->streamming_delay;

		if (usec > 0) {
			pd->streamming_delay = usec;
		} else if (usec < 0){
			pd->streamming_delay = DEFALUT_DELAY_SENDDATA;
		}
		break;
	}

	return rc;
}

#if ENABLE_PREFER_VOD_SERVER == 1
static int stiov3_set_prefer_server (struct streamming_io_v3_client_t *self,
			const int id, const int divider) {
	int	rc = self->pd.prefsvr_id;

	if ((id >= 0) && (divider > id)) {
		self->pd.prefsvr_id = id;
		self->pd.prefsvr_divider = divider;
	} else if ((id == divider) && (id > 0)) {
		struct udplib_t			*udp = self->pd.udp;

		self->pd.prefsvr_id = ntohl (udp->get_myself (udp)) % divider;
		self->pd.prefsvr_divider = divider;
	}

	return rc;
}
#endif

struct streamming_io_v3_client_t *
			new_streamming_io_v3_client (const int port,
							const int cliport) {
	struct streamming_io_v3_client_t	*self;
	struct streamming_io_v3_client_pd_t	*pd;
	int					i, initok = 1;
	struct udplib_t				*udp;

	srand (time (NULL));

	while ((self = malloc (sizeof (struct streamming_io_v3_client_t)))
						!= NULL) {
		pd = &self->pd;

		// pd->my_id = rand () % 8388608;	// 2 ^ 23
		pd->stbid = rand () % 8388608;
		pd->udp			= NULL;
		pd->thr			= NULL;
		pd->cut			= 0;
		pd->port		= port;
		pd->terminate		= 0;
		pd->current_VODserver	= -1;
		pd->have_VODserver	= 0;
		pd->file_server		= 0;

		// pd->timeout_openfile	= 500000;	// 0.5  sec
		// pd->timeout_response	=  20000;	// 0.02 sec
		// pd->streamming_delay	=  50000;	// 0.05 sec
		pd->timeout_openfile	= DEFALUT_DELAY_OPENFILE;
		pd->timeout_response	= DEFALUT_DELAY_RESPONSE;
		pd->streamming_delay	= DEFALUT_DELAY_SENDDATA;

		pd->retry_openfile	= 3;
		pd->retry_response	= 3;

#if ENABLE_PREFER_VOD_SERVER == 1
		pd->curr_svr_id		= -1;
#endif

		pthread_mutex_init (&pd->vod_mutex, NULL);
		pthread_cond_init  (&pd->vod_condi, NULL);

		for (i = 0; i < MAX_STIOV3_CHANNEL; i++) {
			pthread_mutex_init (&pd->mutex[i], NULL);
			pthread_cond_init  (&pd->condi[i], NULL);

			pd->vodsvr_response[i]	= 0;

			pd->need_filesize[i] = 0;
			pd->need_reopen[i]   = 0;
			pd->filesize[i]	     = 0;
			pd->open_errno[i]    = 0;

			pd->blockwant[i]     = 0;
			pd->blockcnt[i]      = 0;
			pd->filecnt[i]       = 0;

			pd->brcvcnt[i]       = 0;

			pd->rdbufidx[i]      = 0;
			pd->rdbuflen[i]      = 0;


			pd->stmpdu[i] = (void *) pd->stm_buffer[i];
			pd->stmptr[i] = (void *) &pd->stmpdu[i]->fileio;
			pd->stmreq[i] = (void *) &pd->stmpdu[i]->fileio;

			/*
			pd->reopen_pdu[i] = malloc (
						sizeof (struct stio_v3_pdu_t));
				*/
			pd->reopen_pdu[i] = malloc (256);
			pd->reopen_pdu_len[i] = sizeof (struct stio_v3_pdu_t);

			pd->stio[i].parent  = self;
			pd->stio[i].channel = i;
			pd->stio[i].save_stio = malloc (
						sizeof (struct stio_engine_t));
		}

		// -------------------------------------------------

		self->dispose		= stiov3_dispose;
		self->start		= stiov3_start;
		self->stop		= stiov3_stop;
		self->find_server	= find_fileserver;

		self->openfile		= stiov3_openfile;
		self->seek		= stiov3_seek;
		self->reopen		= stiov3_reopen;
		self->filesize		= stiov3_filesize;
		self->read		= stiov3_read;
		self->closefile		= stiov3_closefile;

		self->abort		= stiov3_abort;
		self->get_server_id	= stiov3_get_server_id;
		self->get_server	= stiov3_get_server;

		self->regist_stio	= cli_regist_stio;

		self->cksum		= stiov3_cs_cksum;

		self->set_delay		= stiov3_set_delay;

#if ENABLE_PREFER_VOD_SERVER == 1
		self->set_prefer_server	= stiov3_set_prefer_server;
		self->set_prefer_server (self, 0, 1);
#endif

		pd->vodsvr = new_vodsvr (16);

		if ((pd->thr = new_thread_svc (
					streamio_v3_response, self)) == NULL) {
			initok = 0;
			break;
		}

		do {
			initok = 0;

			if ((udp = new_udplib (16)) == NULL) break;
			// if (udp->open (udp, port, NULL, 0) < 0) break;
			if (udp->open (udp, cliport, "eth0", 1) < 0) break;

			initok = 1;
			pd->udp = udp;
			udp->set_sendflag (udp, 0);
			udp->set_recvflag (udp, 0);

#if ENABLE_PREFER_VOD_SERVER == 1
			self->set_prefer_server (self, 2, 2);
#endif
		} while (0);
		
		break;
	}

	if (! initok) {
		 self->dispose (self);
		 return NULL;
	}

	return self;
}
