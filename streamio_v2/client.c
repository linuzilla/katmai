/*
 *	client.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "stio_engine.h"
#include "udplib.h"
#include "thread_svc.h"
#include "streamio_v2.h"
#include "streamio_v2_pdu.h"

struct stio_v2_client_local_t {
	struct streamming_io_v2_client_t	*parent;
	int					channel;
	off_t					fsize;
	int					filecnt;
	struct stio_v2_ft_open_pdu_t		fopen_pdu;
	struct stio_v2_ft_req_pdu_t		fread_pdu;
	int					need_filesize;
	volatile int				*compare_variable;
	int					compare_operator;
	int					compare_expect;
	short					have_file;
	volatile short				need_reopen;
	pthread_mutex_t				mutex;
	pthread_cond_t				condi;
	char					buffer[
						     STIOV2_MAX_TRANS_BUFSIZE];
	int					bidx;
	int					blen;
	int					request_map;
	int					ndtotal;
	int					ndcnt;
	struct stio_engine_t			save_stio;
	int					errcode;
	int					disk_bufsize;
	int					net_bufsize;
	int					disknet_fact;
	int					packet_map;
};

static struct streamming_io_v2_pdu_t	find_server_pdu;
static int				find_server_pdu_len = 0;

#include "common/cksum.c"
#include "client/compare.c"
#include "client/internal.c"
#include "client/stio_eng.c"

static void cs_generic (struct streamming_io_v2_client_t *self,
					void *buffer, int len) {
	struct streamming_io_v2_pdu_t	*ptr  = buffer;
	struct udplib_t			*udp  = self->pd.sudp;
	struct udplib_t			*rudp = self->pd.rudp;
	int				cksum;

	cksum = self->cksum (buffer, len);

	if (ptr->cksum != cksum) {
		fprintf (stderr, "Checksum error!!\n");
		return;
	} else if (ptr->sn != self->pd.sn) {
		fprintf (stderr,
			"Serial number not the one I want (%d != %d)\n",
			ptr->sn, self->pd.sn);
		return;
	}

	if (ptr->cmd == STIOV2_CMD_FINDSERVER) {
		if (! self->pd.have_server) {
			ptr->cmd   = STIOV2_CMD_REGIST;
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);
		}
	} else if (ptr->cmd == STIOV2_CMD_REGIST) {
		if (! self->pd.have_server) {
			pthread_mutex_lock (&self->pd.svr_mutex);
			rudp->unregist (rudp, 0);
			rudp->regist_addr (rudp,
					udp->remote_addr (udp), ptr->port);

			self->pd.server_port = ptr->port;
			strcpy (self->pd.server_ip, udp->remote_addr (udp));

			self->pd.server_id = (self->pd.server_ip[
				strlen (self->pd.server_ip) - 1] - '0') % 2;

			self->pd.have_server = 1;
			pthread_cond_signal (&self->pd.svr_condi);
			pthread_mutex_unlock (&self->pd.svr_mutex);

			fprintf (stderr, "Found server %s on port %d\n",
					udp->remote_addr (udp),
					ptr->port);
		}
	}
}


static void response (struct streamming_io_v2_client_t *self,
					const int cmd, const int channel,
					void *buffer, const int len) {
	struct stio_v2_ft_ok_pdu_t	*okpdu= buffer;
	struct stio_v2_ft_data_pdu_t	*dtpdu= buffer;
	struct stio_v2_error_pdu_t	*epdu = buffer;
	struct stio_v2_client_local_t	*cl = &self->pd.cl[channel];
	int				i, j, k;

	switch (cmd) {
	case STIOV2_CMD_ERROR:
		switch (epdu->orgcmd) {
		case STIOV2_CMD_OPENFILE:
			fprintf (stderr, "Open file: [%d] (%s)\n",
					channel,
					strerror (epdu->lerrno));
			if (cl->need_filesize) {
				pthread_mutex_lock (&cl->mutex);

				cl->need_filesize = 0;
				cl->fsize = 0;
				cl->errcode = epdu->lerrno;

				pthread_cond_signal (&cl->condi);
				pthread_mutex_unlock (&cl->mutex);
			}
			break;
		case STIOV2_CMD_FETCH:
			if (epdu->errcode == STIOV2_ERR_REOPEN) {
				pthread_mutex_lock (&cl->mutex);
				cl->need_reopen = 1;
				pthread_cond_signal (&cl->condi);
				pthread_mutex_unlock (&cl->mutex);

				fprintf (stderr, "Need reopen\n");
			}

			break;
		default:
			fprintf (stderr, "Remote error (%s)\n",
					strerror (epdu->lerrno));
			break;
		}
		break;
	case STIOV2_CMD_OPENFILE:
		// fprintf (stderr, "Open file (%ld)\n", okpdu->filesize);

		if (cl->need_filesize) {
			pthread_mutex_lock (&cl->mutex);

			cl->need_filesize = 0;
			cl->fsize = okpdu->filesize;
			cl->need_reopen = 0;

			cl->disk_bufsize = okpdu->disk_bufsize;
			cl->net_bufsize  = okpdu->net_bufsize;
			cl->disknet_fact = okpdu->disknet_fact;
			cl->packet_map   = okpdu->packet_map;

			pthread_cond_signal (&cl->condi);
			pthread_mutex_unlock (&cl->mutex);

			fprintf (stderr,
				"Streamming Parameter: [%d,%d,%d,%d]\n",
				okpdu->disk_bufsize,
				okpdu->net_bufsize,
				okpdu->disknet_fact,
				okpdu->packet_map);
		}

		break;
	case STIOV2_CMD_FETCH:
		if (cl->request_map > 0) {
			i = dtpdu->map;
			if (cl->fread_pdu.map[i]) {
				j = STIOV2_CARRY_DATASIZE;
				k = i * STIOV2_CARRY_DATASIZE;

				if (i == cl->packet_map - 1) {
					j = cl->net_bufsize - k;
				}

				// fprintf (stderr, "Receive %d\n", i);
				cl->fread_pdu.map[i] = 0;
				--cl->request_map;
				memcpy (&cl->buffer[k], dtpdu->data, j);

				pthread_mutex_lock (&cl->mutex);
				pthread_cond_signal (&cl->condi);
				pthread_mutex_unlock (&cl->mutex);
			}
		}
		break;
	default:
		fprintf (stderr, "Unknow CMD(%d)\n", okpdu->cmd);
		break;
	}
}

#if CAPTURE_USR2_SIGNAL
static void usr2_handler (const int signo) {
	fprintf (stderr, "Singal %d capture for thread %ld\n",
			signo, pthread_self ());
}
#endif

static int client_main (void *selfptr) {
	struct streamming_io_v2_client_t	*self = selfptr;
	struct streamming_io_v2_client_pd_t	*pd = &self->pd;
	struct udplib_t				*udp[2];
	int					fd[2];
	int					mfd;
	fd_set					rfds;
	struct timeval				tv;
	int					i, len;
	char					buffer[2000];
	struct streamming_io_v2_generic_pdu_t	*pdu = (void *) buffer;
	// sigset_t				old_mask, new_mask;

	// SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK

	// pthread_sigmask (SIG_BLOCK, NULL, &new_mask);
	// sigaddset (&new_mask, SIGUSR2);
	// pthread_sigmask (SIG_BLOCK, &new_mask, &old_mask);

#if CAPTURE_USR2_SIGNAL
	signal (SIGUSR2, usr2_handler);
#endif

	udp[0] = pd->sudp;
	udp[1] = pd->rudp;

	fd[0] = udp[0]->getfd (udp[0]);
	fd[1] = udp[1]->getfd (udp[1]);

	mfd = (fd[0] > fd[1] ? fd[0] : fd[1]) + 1;


	fprintf (stderr, "[*] Stream I/O Client Daemon Starting\n");

	while (! pd->terminate) {
		FD_ZERO (&rfds);
		FD_SET  (fd[0], &rfds);
		FD_SET  (fd[1], &rfds);
		tv.tv_sec  = 5;
		tv.tv_usec = 0;

		if (select (mfd, &rfds, NULL, NULL, &tv) < 0) {
			perror ("select");
			continue;
		}

		for (i = 0; i < 2; i++) {
			if (FD_ISSET (fd[i], &rfds)) {
				if ((len = udp[i]->receive (udp[i],
						buffer, sizeof buffer)) <= 0) {
					perror ("receive");
				} else if (i == 0) {
					cs_generic (self, buffer, len);
				} else if (pdu->cksum !=
						self->cksum (pdu, len)) {
					fprintf (stderr,
						"Cksum error !!\n");
				} else if ((pdu->channel < 0) ||
						(pdu->channel >=
						 MAXIMUM_STIOV2_SERVICES)) {
					fprintf (stderr,
						"Channel error %d\n",
						pdu->channel);
				} else {
					response (self, pdu->cmd,
							pdu->channel,
							buffer, len);
				}
			}
		}
	}

	fprintf (stderr, "[*] Stream I/O Client Daemon Terminated\n");

	return 1;
}

static int cli_find_server (struct streamming_io_v2_client_t *self) {
	struct udplib_t		*udp = self->pd.sudp;

	if (self->pd.have_server) return 1;

	udp->bsend (udp, self->pd.port, &find_server_pdu, find_server_pdu_len);
	usleep (100);

	return self->pd.have_server;
}


static int cli_openfile (struct streamming_io_v2_client_t *self,
				const int channel, const char *fname) {
	struct stio_v2_client_local_t	*cl;
	int				rc;

	if ((channel < 0) || (channel >= MAXIMUM_STIOV2_SERVICES)) return -1;

	cl = &self->pd.cl[channel];

	self->pd.abort		= 0;

	++cl->filecnt;

	cl->fopen_pdu.cmd	= STIOV2_CMD_OPENFILE;
	cl->fopen_pdu.channel   = channel;
	cl->fopen_pdu.sn	= self->pd.sn;
	cl->fopen_pdu.filecnt	= cl->filecnt;

	cl->ndcnt	= 0;
	cl->ndtotal	= 0;
	cl->bidx	= 0;
	cl->blen	= 0;

	strncpy (cl->fopen_pdu.filename, fname, MAXIMUM_STIOV2_FILENAME_LEN);
	cl->fopen_pdu.filename[MAXIMUM_STIOV2_FILENAME_LEN] = '\0';

	rc = internal_reopen (self, channel);

	if (rc > 0) {
		cl->have_file = 1;
		cl->ndtotal = cl->fsize / cl->net_bufsize;

		if (cl->fsize % cl->net_bufsize != 0) cl->ndtotal++;
	} else {
		errno = cl->errcode;
	}

	return rc;
	/*
	self->send (self, &cl->fopen_pdu, sizeof cl->fopen_pdu);
	
	return 0;
	*/
}

static int cli_reopen (struct streamming_io_v2_client_t *self,
							const int channel) {
	struct stio_v2_client_local_t	*cl;

	if ((channel < 0) || (channel >= MAXIMUM_STIOV2_SERVICES)) return -1;

	cl = &self->pd.cl[channel];

	cl->ndcnt	= 0;
	cl->bidx	= 0;
	cl->blen	= 0;

	// fprintf (stderr, "cli_reopen\n");

	return 1;
}

static int cli_closefile (struct streamming_io_v2_client_t *self,
							const int channel) {
	struct stio_v2_client_local_t	*cl;

	if ((channel < 0) || (channel >= MAXIMUM_STIOV2_SERVICES)) return -1;

	cl = &self->pd.cl[channel];
	cl->have_file = 1;

	return 0;
}

static off_t cli_filesize (struct streamming_io_v2_client_t *self,
							const int channel) {
	struct stio_v2_client_local_t	*cl;

	if ((channel < 0) || (channel >= MAXIMUM_STIOV2_SERVICES)) return -1;

	cl = &self->pd.cl[channel];

	return cl->fsize;
}	

static void cli_dispose (struct streamming_io_v2_client_t *self) {
	struct streamming_io_v2_client_pd_t	*pd = &self->pd;

	pd->terminate = 1;

	if (pd->thr  != NULL) {
		pd->terminate = 1;
		pd->thr->stop     (pd->thr);
		pd->thr->dispose  (pd->thr);
	}
	if (pd->sudp != NULL) pd->sudp->dispose (pd->sudp);
	if (pd->rudp != NULL) pd->rudp->dispose (pd->rudp);
	if (pd->cl   != NULL) free (pd->cl);

	free (self);
}

static int cli_read (struct streamming_io_v2_client_t *self,
			const int channel, char *buffer, const int len) {
	struct stio_v2_client_local_t	*cl;
	int				rc = -1;

	if ((channel < 0) || (channel >= MAXIMUM_STIOV2_SERVICES)) return -1;

	cl = &self->pd.cl[channel];

	if (! cl->have_file) return -1;

	if (cl->blen <= 0) {
		if (cl->ndcnt >= cl->ndtotal) return -1;
		// to read data from remote
		/*
		fprintf (stderr, "Internal Read: %d, %d\n",
					cl->ndcnt, cl->ndtotal);
		*/
		cl->bidx = 0;
		cl->blen = internal_read (cl);

		cl->ndcnt++;
	}

	if (cl->blen > 0) {
		rc = (cl->blen >= len) ? len : cl->blen;
		// fprintf (stderr, "Reading from %d, size %d\n", cl->bidx, rc);
		memcpy (buffer, &cl->buffer[cl->bidx], rc);

		cl->blen -= rc;
		cl->bidx += rc;
	}

	return rc;
}

static void cli_abort (struct streamming_io_v2_client_t *self) {
	// fprintf (stderr, "Streamming io client - Abort [%p]\n", self);
	self->pd.abort = 1;
}

static int cli_get_server (struct streamming_io_v2_client_t *self,
						char **server, int **port) {
	*server = self->pd.server_ip;
	*port = &self->pd.server_port;

	return self->pd.have_server;
}

static int cli_get_server_id (struct streamming_io_v2_client_t *self) {
	return self->pd.server_id;
}

struct streamming_io_v2_client_t *
			new_streamming_io_v2_client (const int port) {
	struct streamming_io_v2_client_t	*self;
	struct streamming_io_v2_client_pd_t	*pd;
	struct stio_v2_client_local_t		*cl;
	int					initok = 1;
	int					i;

	while ((self = malloc (
			sizeof (struct streamming_io_v2_client_t))) != NULL) {
		pd = &self->pd;

		initok = 0;

		pd->port	= port;
		pd->svcport	= -1;
		pd->thr 	= NULL;
		pd->sudp	= NULL;
		pd->rudp	= NULL;
		pd->terminate	= 0;
		pd->have_server	= 0;
		pd->abort	= 0;
		pd->sn		= time (NULL);
		pd->cl		= NULL;
		pd->server_id	= 0;
		pd->server_port	= 0;
		pd->server_ip[0] = pd->server_ip[sizeof pd->server_ip-1] = '\0';

		// pthread_mutex_init (&pd->mutex, NULL);
		// pthread_cond_init  (&pd->condi, NULL);

		pthread_mutex_init (&pd->svr_mutex, NULL);
		pthread_cond_init  (&pd->svr_condi, NULL);


		self->dispose		= cli_dispose;
		self->cksum		= stiov2_cs_cksum;
		self->find_server	= cli_find_server;
		self->openfile		= cli_openfile;
		self->closefile		= cli_closefile;
		self->filesize		= cli_filesize;
		self->read		= cli_read;
		self->abort		= cli_abort;
		self->regist_stio	= cli_regist_stio;
		self->reopen		= cli_reopen;
		self->get_server	= cli_get_server;
		self->get_server_id	= cli_get_server_id;

		if ((cl = calloc (MAXIMUM_STIOV2_SERVICES, sizeof (
				struct stio_v2_client_local_t))) == NULL) {
			break;
		} else {
			pd->cl = cl;

			for (i = 0; i < MAXIMUM_STIOV2_SERVICES; i++) {
				// fprintf (stderr, "%d. %p\n", i, &cl[i]);
				cl[i].parent	= self;
				cl[i].channel	= i;
				cl[i].fsize	= -1;
				cl[i].filecnt	= 0;
				cl[i].have_file	= 0;
				cl[i].bidx	= 0;
				cl[i].blen	= 0;
				cl[i].request_map	= 0;
				cl[i].fopen_pdu.channel = i;

				pthread_mutex_init (&cl[i].mutex, NULL);
				pthread_cond_init  (&cl[i].condi, NULL);
			}
		}

		pd->thr = new_thread_svc (client_main, self);

		if ((pd->sudp = new_udplib (0)) == NULL) break;
		if ((pd->rudp = new_udplib (1)) == NULL) break;

		if (pd->sudp->open (pd->sudp, 0, "eth0", 1) < 0) break;
		if (pd->rudp->open (pd->rudp, 0, "eth0", 1) < 0) break;


		initok = 1;

		if (find_server_pdu_len == 0) {
			find_server_pdu.cmd	= STIOV2_CMD_FINDSERVER;
			find_server_pdu.sn	= pd->sn;
			find_server_pdu.protocol_version = 
						  STIOV2_PROTOCOL_VERSION;
			find_server_pdu_len	= sizeof find_server_pdu;
			find_server_pdu.cksum	=
				self->cksum (&find_server_pdu,
						find_server_pdu_len);
		}

		pd->thr->start (pd->thr);

		break;
	}

	if (! initok) {
		self->dispose (self);
		return NULL;
	}

	return self;
}
