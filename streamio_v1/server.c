/*
 *	streamming io (server)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "udplib.h"
#include "streamio_v1.h"
#include "streamio_v1_pdu.h"
#include "thread_svc.h"
#include "client_entry.h"
#include "stb_table.h"
// ------------------------------------------
#include "common/cksum.c"

static int dummy_fopen (const int i, const char *fname) {
	// char buffer[256];
	// sprintf (buffer, "/home/cc/liujc/MPEG/80/12/%s", fname);
	// sprintf (buffer, "/home/cc/liujc/FoveonX3/download");

	return open (fname, O_RDONLY);
}

static int check_regist (struct streamming_io_v1_server_t *self, const int sn) {
	struct stb_vod_pdu_t    pdu;
	struct udplib_t		*udp = self->pd.udp;

	if (self->pd.my_id == (sn >> 8)) return sn & 0xff;

	pdu.cmd = STBVOD_PDU_CMD_NEED_REGIST;
	pdu.cksum = self->cksum (&pdu, sizeof pdu);
	udp->send (udp, &pdu, sizeof pdu);

	return -1;
}

static int streamio_v1_main (void *selfptr) {
	struct streamming_io_v1_server_t	*self = selfptr;
	struct streamming_io_v1_server_pd_t	*pd = &self->pd;
	struct udplib_t				*udp = pd->udp;
	int					i, len, cksum;
	char					buffer[2000];
	//struct streamming_io_v1_pdu_t		*ptr =
	//			(struct streamming_io_v1_pdu_t *) buffer;

	struct stbtable_t       *stb = pd->stb;
	struct stb_vod_pdu_t    *ptr = (struct stb_vod_pdu_t *) buffer;
	int                     j, err;
	struct stb_vod_regist_t *svrptr = &ptr->regist;
	struct stb_vod_fileio_t *fioptr = &ptr->fileio;
	struct stb_vod_fetch_t  *fthptr = &ptr->fetch;
	struct stb_vod_fetch_request_t *ftreq = (void *) &ptr->fetch;

	fprintf (stderr, "STREAM IO (v1) MAIN THREAD\n");


	while (! pd->terminate) {
		if ((len = udp->recv (udp, buffer, sizeof buffer, 0)) < 0) {
			if (errno == EAGAIN) {
				usleep (20000);
			} else {
				perror (NULL);
			}
			continue;
		} else if (len == 0) {
			perror (NULL);
			continue;
		}

		cksum = self->cksum (buffer, len);

		if (cksum != ptr->cksum) {
			fprintf (stderr, "Checksum error!!\n");
			break;
		} else if (self->pd.stop_services) {
			fprintf (stderr, "Stop services\n");
			break;
		}

		switch (ptr->cmd) {
		case STBVOD_PDU_CMD_FINDSERVER:
			// fprintf (stderr ,"find server\n");
			udp->send (udp, buffer, len);
			break;
		case STBVOD_PDU_CMD_REGIST:
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
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);
			break;
		case STBVOD_PDU_CMD_OPENFILE:
			if ((i = check_regist (self, fioptr->sn)) < 0) break;

			fprintf (stderr,
				"STB[%d]: Playfile [%s], cnt=%u\n",
				i, fioptr->file,
				fioptr->filecnt);
			fioptr->filesize = stb->setfile (i,
					fioptr->file, fioptr->filecnt,
					fioptr->stbid, &err);

			fioptr->err = err;
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);
			stb->init (i, fioptr->sn);
			break;

		case STBVOD_PDU_CMD_FAST_FETCH:
		case STBVOD_PDU_CMD_FETCH:
			if ((i = check_regist (self, fthptr->sn)) < 0) break;

			if (! stb->is_file_matched (i, fthptr->filecnt)) {
				ptr->cmd = STBVOD_PDU_CMD_NEED_REOPEN;
				ptr->cksum = self->cksum (buffer, len);
				udp->send (udp, buffer, len);
				break;
			}

			ptr->cmd = STBVOD_PDU_CMD_FETCH_RESPONSE;
			ptr->cksum = self->cksum (buffer, len);
			udp->send (udp, buffer, len);

			// 如果還在送 ...   等送完
			// 如果已經送完 ... 重送

			stb->setrequest (i, ftreq->h.block,
						ftreq->map, ftreq->qos);
			stb->push (i, ptr->cmd);
			break;

		case STBVOD_PDU_CMD_CLOSEFILE:
			if ((i = check_regist (self, fthptr->sn)) < 0) break;
			fprintf (stderr, "closefile\n");
			break;
		}
	}
#if 0
	while (! pd->terminate) {
		if ((len = udp->recv (udp, buffer, sizeof buffer, 5)) < 0) {
			if (errno == EAGAIN) {
				usleep (50000);
			} else if (errno == EINTR) {
				usleep (50000);
			} else if (errno == 0) {
				usleep (50000);
			} else {
				perror ("stream io (thread)");
			}
			continue;
		} else if (len == 0) {
			// perror (NULL);
			continue;
		}

		// -- ** -- ** -- ** --

		cksum = self->cksum (buffer, len);

		if (cksum != ptr->cksum) {
			fprintf (stderr, "Checksum error!!\n");
			break;
		} else if (self->pd.stop_services) {
			fprintf (stderr, "Stop services\n");
			break;
		}

		// -- ** -- ** -- ** --
		
		switch (ptr->cmd) {
		case STIOv1_CMD_FINDSERVER:
			if (ptr->protocol_version == STIOv1_PROTOCOL_VERSION) {
				fprintf (stderr, "Find server request (%d)\n",
						ptr->protocol_version);
				udp->send (udp, buffer, len);
			} else {
				fprintf (stderr, "Find server (protocol "
						"version %d != %d)\n",
						ptr->protocol_version,
						STIOv1_PROTOCOL_VERSION);
			}
			break;
		case STIOv1_CMD_REGIST:
			if ((i = udp->regist_ipaddr (udp)) < 0) {
				fprintf (stderr, "Table full\n");
				break;
			}

			if ((port = self->regist (self,
					udp->get_remote (udp), i,
					ptr->sn)) < 0) {
				fprintf (stderr, "port error\n");
				break;
			}

			fprintf (stderr, "Regist on %d\n", port);
			ptr->port = port;
			ptr->cksum = self->cksum (ptr, len);
			udp->send (udp, buffer, len);
			break;
		default:
			fprintf (stderr, "cmd[%d]: unknow\n", ptr->cmd);
			break;
		}
	}
#endif

	fprintf (stderr, "STREAM IO (v1) MAIN THREAD Terminated !!\n");

	return 0;
}

static int stiov1_start (struct streamming_io_v1_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.already_started = 1;
	self->pd.terminate = 0;
	thr->conti (thr);

	return thr->start (thr);
}

static int stiov1_stop (struct streamming_io_v1_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.terminate = 1;
	return thr->stop (thr);
}

static int stiov1_pause (struct streamming_io_v1_server_t *self, const int p) {
	int	rc = self->pd.stop_services;

	if (p >= 0) self->pd.stop_services = p == 0 ? 0 : 1;

	return rc;
}

static int stiov1_regist (struct streamming_io_v1_server_t *self,
			const in_addr_t addr, const int n, const int sn) {
	struct client_entry_t	*cli = self->pd.cli[n];
	
	if ((n < 0) || (n >= self->pd.maxclient)) return -1;

	fprintf (stderr, "Regist on entry %d\n", n);

	return cli->regist (cli, addr, sn);
}

static void stiov1_set_fopen (struct streamming_io_v1_server_t *self,
					int (*f)(const int, const char *)) {
	self->pd.fopen = (f == NULL) ? dummy_fopen : f;
}

static int stiov1_set_parameter (struct streamming_io_v1_server_t *self,
				const int channel,
				const int diskbuf, const int netbuf) {
	if (self->pd.already_started) {
		fprintf (stderr, "Can not set parameter !!\n");
		return 0;
	}

	if ((channel < 0) || (channel > MAXIMUM_STIOV1_SERVICES)) {
		fprintf (stderr, "Channel out of range\n");
		return 0;
	}

	if ((diskbuf < 4096) || (diskbuf > DEFAULT_DISKIO_BUFFER_FAVIOR)) {
		fprintf (stderr, "Diskbuf size %d out of range\n", diskbuf);
		return 0;
	}

	if ((netbuf < 4096) || (netbuf > STIOV1_MAX_TRANS_BUFSIZE)) {
		fprintf (stderr, "Netbuf size %d out of range\n", netbuf);
		return 0;
	}

	if (netbuf % 1024 != 0) {
		fprintf (stderr, "Netbuf size %d not ok\n", netbuf);
		return 0;
	}

	if (diskbuf % netbuf != 0) {
		fprintf (stderr, "Diskbuf v.s. Netbuf not ok\n");
		return 0;
	}

	self->pd.disk_bufsize[channel] = diskbuf;
	self->pd.net_bufsize[channel]  = netbuf;
	self->pd.disknet_fact[channel] = diskbuf / netbuf;
	self->pd.packet_map[channel]   = netbuf / STIOV1_CARRY_DATASIZE +
			(netbuf % STIOV1_CARRY_DATASIZE > 0 ? 1 : 0);

	fprintf (stderr, "[*] Channel %d streamming parameter [%d,%d,%d,%d]\n",
			channel,
			self->pd.disk_bufsize[channel],
			self->pd.net_bufsize[channel],
			self->pd.disknet_fact[channel],
			self->pd.packet_map[channel]);

	return 1;
}

static void stiov1_dispose (struct streamming_io_v1_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;
	struct udplib_t		*udp = self->pd.udp;
	struct client_entry_t	**cli = self->pd.cli;
	int			i;

	self->stop (self);

	if (cli != NULL) {
		for (i = 0; i < self->pd.maxclient; i++) {
			// fprintf (stderr, "Dispose %d ... ", i);
			cli[i]->dispose (cli[i]);
			// fprintf (stderr, "ok\n");
		}
	}

	if (thr != NULL) thr->dispose (thr);
	if (udp != NULL) udp->close   (udp); 
	if (udp != NULL) udp->dispose (udp);

	free (self->pd.cli);
	free (self);
}

struct streamming_io_v1_server_t * new_streamming_io_v1_server (
					const int maxclient, 
					const int port) {
	struct streamming_io_v1_server_t	*self;
	struct streamming_io_v1_server_pd_t	*pd;
	struct udplib_t				*udp;
	struct client_entry_t			**cli;
	int					i, initok = 1;

	srand (time (NULL));

	while ((self = malloc (sizeof
				(struct streamming_io_v1_server_t))) != NULL) {
		pd = &self->pd;

		pd->terminate		= 0;
		pd->maxclient		= maxclient;
		pd->port		= port;
		pd->udp			= NULL;
		pd->cli			= NULL;
		pd->thr			= NULL;
		pd->stop_services	= 0;
		pd->already_started	= 0;
		pd->fopen		= dummy_fopen;
		pd->my_id		= rand () % 8388608;      // 2 ^ 23
		pd->stb_qos_threshold	= 100;

		self->start		= stiov1_start;
		self->stop		= stiov1_stop;
		self->dispose		= stiov1_dispose;
		self->cksum		= stiov1_cs_cksum;
		self->regist		= stiov1_regist;
		self->pause		= stiov1_pause;
		self->set_parameter	= stiov1_set_parameter;
		self->set_fopen		= stiov1_set_fopen;

		if ((pd->thr = new_thread_svc (
					streamio_v1_main, self)) == NULL) {
			initok = 0;
			break;
		}

		if ((pd->cli = calloc (maxclient,
				sizeof (struct client_entry_t *))) == NULL) {
			initok = 0;
			break;
		} else {
			cli = pd->cli;

			/*
			for (i = 0; i < maxclient; i++) {
				if ((cli[i] = new_client_entry (self))== NULL) {
					initok = 0;
					break;
				}
			}
			*/
		}

		for (i = 0; i < MAXIMUM_STIOV1_SERVICES; i++) {
			pd->disk_bufsize[i] = DEFAULT_DISKIO_BUFFER_FAVIOR;
			pd->net_bufsize[i]  = DEFAULT_STIOV1_TRANS_BUFSIZE;
			pd->disknet_fact[i] = DEFAULT_STIOV1_DISKNET_FACTOR;
			pd->packet_map[i]   = DEFAULT_STIOV1_PACKET_MAP; 
		}


		do {
			initok = 0;

			if ((udp = new_udplib (maxclient)) == NULL) break;
			if (udp->open (udp, port, NULL, 0) < 0) break;

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
