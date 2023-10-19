/*
 *	server.c (streamio_v3)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "thread_svc.h"
#include "udplib.h"
#include "streamio_v3.h"
#include "streamio_v3_pdu.h"
#include "stb_table.h"
#include "stb_entry.h"

#include "common/cksum.c"

// static int dummy_fopen (const int i, const char *fname) {
//	return open (fname, O_RDONLY);
// }

static char * dummy_file_pathname (const int i, const char *fname) {
	static char	pathname[MAX_STIOV3_CHANNEL][256];

	fprintf (stderr, "Channel=%d, file pathname:%s\n", i, fname);

	sprintf (pathname[i], "%s", fname);
	return pathname[i];
}

static char * dummy_real_filename (const int i) {
	return NULL;
}

extern void * streamio_v3_service (void *arg);

static int check_regist (struct streamming_io_v3_server_t *self, const int sn) {
	struct stio_v3_pdu_t	pdu;
	struct udplib_t		*udp = self->pd.udp;

	if (self->pd.my_id == (sn >> 8)) return sn & 0xff;

	pdu.cmd = STIOV3_PDU_CMD_NEED_REGIST;
	pdu.cksum = self->cksum (&pdu, sizeof pdu);
	udp->send (udp, &pdu, sizeof pdu);
	return -1;
}

static int streamio_v3_main (void *selfptr) {
	struct streamming_io_v3_server_t	*self = selfptr;
	struct streamming_io_v3_server_pd_t	*pd = &self->pd;
	struct udplib_t				*udp = pd->udp;
	struct stbtable_t			*stb = pd->stb;
	int					i, j, err, len;
	int					cksum;
	char					buffer[2000];
	struct stio_v3_pdu_t			*ptr = (void *) buffer;
	struct stiov3_regist_t			*svrptr = &ptr->regist;
	struct stiov3_fileio_t			*fioptr = &ptr->fileio;
	struct stiov3_fetch_t			*fthptr = &ptr->fetch;
	struct stiov3_fetch_request_t		*ftreq = (void *) &ptr->fetch;
	char					*real_filename;

#if DEBUG_STIOV3 > 0
	fprintf (stderr, "Streamming I/O Version 3 (started)\n");
#endif

	while (! pd->terminate) {
		if ((len = udp->receive (udp, buffer, sizeof buffer)) < 0) {
			if (errno == EAGAIN) {
				usleep (20000);
			} else {
				perror ("Streamming I/O Version 3");
			}
			continue;
		} else if (len == 0) {
			perror (NULL);
			continue;
		}

		if ((cksum = self->cksum (buffer, len)) != ptr->cksum) {
			fprintf (stderr, "Checksum error!!\n");
			continue;
		} else if (pd->stop_services) {
			fprintf (stderr, "Stop services\n");
			continue;
		}

#if DEBUG_STIOV3 > 4
		fprintf (stderr, "Connect from: %s:%d [CMD=%d]\n",
				udp->remote_addr (udp),
				udp->remote_port (udp),
				ptr->cmd);
#endif

		switch (ptr->cmd) {
		case STIOV3_PDU_CMD_FINDSERVER:
#if DEBUG_STIOV3 > 2
			fprintf (stderr, "Reply find server\n");
#endif
			udp->send (udp, buffer, len);
			// udp->bsend (udp, udp->remote_port (udp),
			// 			buffer, len);
			break;
		case STIOV3_PDU_CMD_REGIST:
			j = udp->regist_rmaddr (udp);
			i = stb->regist (udp->get_remote (udp), j);

			if (i >= 0) {
				svrptr->sn = (pd->my_id << 8) + i;
			} else {
				svrptr->sn = -1;
			}

			fprintf (stderr ,"Regist %d (%d,%d)\n",
					i, svrptr->sn, j);
			svrptr->threshold = pd->stb_qos_threshold;
			svrptr->version_major = pd->version_major;
			svrptr->version_minor = pd->version_minor;
			len = sizeof (struct stio_v3_pdu_header_t) +
						sizeof (struct stiov3_regist_t);
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);
			break;
		case STIOV3_PDU_CMD_OPENFILE:
			if ((i = check_regist (self, fioptr->sn)) < 0) break;

			fprintf (stderr,
				"STB[%d]: Playfile [%s], Channel=%d, cnt=%u\n",
					i, fioptr->file, fioptr->channel,
					fioptr->filecnt);

			real_filename = NULL;

			fioptr->filesize = stb->setfile (
					i,
					fioptr->channel,
					fioptr->file, fioptr->filecnt,
					fioptr->stbid,
					&real_filename, &err);

			if (real_filename != NULL) {
				strncpy (fioptr->file, real_filename,
						sizeof fioptr->file);
			} else {
				fioptr->file[0] = '\0';
			}

			fioptr->err = err;
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);
			stb->init (i, fioptr->channel, fioptr->sn);
			break;
		case STIOV3_PDU_CMD_FAST_FETCH:
			break;
		case STIOV3_PDU_CMD_FETCH:
			if ((i = check_regist (self, fioptr->sn)) < 0) break;

			if (! stb->is_file_matched (i,
						fioptr->channel,
						fthptr->filecnt)) {
#if DEBUG_STIOV3 > 2
				fprintf (stderr, "fetch %d, but need reopen\n",
						fioptr->channel);
#endif
				ptr->cmd = STIOV3_PDU_CMD_NEED_REOPEN;
				ptr->cksum = self->cksum (buffer, len);
				udp->send (udp, buffer, len);
				break;
			}

#if DEBUG_STIOV3 > 2
			fprintf (stderr, "fetch on channel %d\n",
					fioptr->channel);
#endif

			ptr->cmd = STIOV3_PDU_CMD_FETCH_RESPONSE;
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);

			stb->setrequest (i, fioptr->channel,
					ftreq->h.block,
					ftreq->map, ftreq->qos);
			stb->push (i, fioptr->channel, ptr->cmd);
			break;
		case STIOV3_PDU_CMD_CLOSEFILE:
			if ((i = check_regist (self, fioptr->sn)) < 0) break;
#if DEBUG_STIOV3 > 2
			fprintf (stderr, "closefile\n");
#endif
			break;
		}
	}
#if DEBUG_STIOV3 >= 0
#endif
	fprintf (stderr, "Streamming I/O Version 3 (ended)\n");

	return 0;
}

static int stiov3_start_svc (struct streamming_io_v3_server_t *self) {
	static struct stio_v3_svc_param_t	parm[MAX_STIOV3_CHANNEL];
	int					i, j, k;
	int					nos[] = {
							30, 5, 5, 3, 20, 3, 3
						};

	for (i = 0; i < MAX_STIOV3_CHANNEL; i++) {
		parm[i].stiov3svr = self;
		parm[i].channel = i;
	}

	for (i = k = 0; i < MAX_STIOV3_CHANNEL; i++) {
		for (j = 0; j < nos[i]; j++) {
			pthread_create (&self->pd.svcthr[k++], NULL,
					streamio_v3_service, &parm[i]);
		}
	}

	return 1;
}

static void stiov3_close_all (struct streamming_io_v3_server_t *self) {
	struct stbtable_t	*stb = self->pd.stb;
	int			svcst;

	svcst = self->pd.stop_services;

	stb->close_all ();

	self->pd.stop_services = svcst;
}

static int stiov3_start (struct streamming_io_v3_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.already_started = 1;
	self->pd.terminate = 0;
	thr->conti (thr);

	return thr->start (thr);
}

static int stiov3_stop (struct streamming_io_v3_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.terminate = 1;
	return thr->stop (thr);
}

static void stiov3_dispose (struct streamming_io_v3_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;
	struct udplib_t		*udp = self->pd.udp;

	// fprintf (stderr, "stiov3_dispose\n");
	self->stop (self);
	// thr->kill (thr, SIGINT);
	// thr->kill (thr, SIGTERM);

	if (thr != NULL) thr->dispose (thr);
	if (udp != NULL) udp->close   (udp);
	if (udp != NULL) udp->dispose (udp);
	// free (self->pd.cli);
	free (self);
}

static int stiov3_set_parameter (struct streamming_io_v3_server_t *self,
				const int channel,
				const int diskbuf, const int netbuf) {
	// sorry! nothing
	return 1;
}

static void stiov3_set_fopen (struct streamming_io_v3_server_t *self,
				int (*f)(const int, const char *)) {
	// self->pd.fopen = (f == NULL) ? dummy_fopen : f;
}

static void stiov3_set_file_pathname (struct streamming_io_v3_server_t *self,
				char *(*f)(const int, const char *)) {
	self->pd.file_pathname = (f == NULL) ? dummy_file_pathname : f;
}

static void stiov3_set_real_filename (struct streamming_io_v3_server_t *self,
						char *(*f)(const int)) {
	self->pd.real_filename = (f == NULL) ? dummy_real_filename : f;
}

static int stiov3_pause (struct streamming_io_v3_server_t *self, const int p) {
	int	rc = self->pd.stop_services;

	if (p >= 0) self->pd.stop_services = p == 0 ? 0 : 1;

	return rc;
}

struct streamming_io_v3_server_t *
			new_streamming_io_v3_server (const int maxclient,
					const int port, const int enable_qos) {
	struct streamming_io_v3_server_t	*self;
	struct streamming_io_v3_server_pd_t	*pd;
	struct udplib_t				*udp;
	int					initok = 1;

	srand (time (NULL));

	while ((self = malloc (sizeof (struct streamming_io_v3_server_t)))
						!= NULL) {
		pd = &self->pd;

		pd->my_id = rand () % 8388608;	// 2 ^ 23
		
		pd->thr			= NULL;
		pd->udp			= NULL;
		pd->stb			= NULL;
		pd->terminate		= 0;
		pd->already_started	= 0;
		pd->stop_services	= 0;
		// pd->fopen		= dummy_fopen;
		pd->file_pathname	= dummy_file_pathname;
		pd->real_filename	= dummy_real_filename;
		pd->stb_qos_threshold	= 60;

		self->start		= stiov3_start;
		self->stop		= stiov3_stop;
		self->dispose		= stiov3_dispose;
		self->cksum		= stiov3_cs_cksum;
		self->pause		= stiov3_pause;
		self->start_svc		= stiov3_start_svc;
		self->set_parameter	= stiov3_set_parameter;
		self->set_fopen		= stiov3_set_fopen;
		self->set_file_pathname	= stiov3_set_file_pathname;
		self->set_real_filename = stiov3_set_real_filename;
		self->close_all		= stiov3_close_all;

		if ((pd->thr = new_thread_svc (
					streamio_v3_main, self)) == NULL) {
			initok = 0;
			break;
		}

		pd->stb = new_stbtable_v3 (self, maxclient, enable_qos);

		do {
			initok = 0;

			if ((udp = new_udplib (maxclient)) == NULL) break;
			// if (udp->open (udp, port, NULL, 0) < 0) break;
			if (udp->open (udp, port, "eth0", 0) < 0) break;

			initok = 1;
			pd->udp = udp;
			udp->set_sendflag (udp, 0);
			udp->set_recvflag (udp, 0);
		} while (0);

		break;
	}

	if (! initok) {
		 self->dispose (self);
		 return NULL;
	}

	return self;
}
