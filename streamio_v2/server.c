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
#include "streamio_v2.h"
#include "streamio_v2_pdu.h"
#include "thread_svc.h"
#include "client_entry.h"
// ------------------------------------------
#include "common/cksum.c"

static int dummy_fopen (const int i, const char *fname) {
	// char buffer[256];
	// sprintf (buffer, "/home/cc/liujc/MPEG/80/12/%s", fname);
	// sprintf (buffer, "/home/cc/liujc/FoveonX3/download");

	return open (fname, O_RDONLY);
}

static int streamio_v2_main (void *selfptr) {
	struct streamming_io_v2_server_t	*self = selfptr;
	struct streamming_io_v2_server_pd_t	*pd = &self->pd;
	struct udplib_t				*udp = pd->udp;
	int					i, port, len, cksum;
	char					buffer[2000];
	struct streamming_io_v2_pdu_t		*ptr =
				(struct streamming_io_v2_pdu_t *) buffer;

	fprintf (stderr, "STREAM IO (V2) MAIN THREAD\n");

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
		case STIOV2_CMD_FINDSERVER:
			if (ptr->protocol_version == STIOV2_PROTOCOL_VERSION) {
				fprintf (stderr, "Find server request (%d)\n",
						ptr->protocol_version);
				udp->send (udp, buffer, len);
			} else {
				fprintf (stderr, "Find server (protocol "
						"version %d != %d)\n",
						ptr->protocol_version,
						STIOV2_PROTOCOL_VERSION);
			}
			break;
		case STIOV2_CMD_REGIST:
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

	fprintf (stderr, "STREAM IO (V2) MAIN THREAD Terminated !!\n");

	return 0;
}

static int stiov2_start (struct streamming_io_v2_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.already_started = 1;
	self->pd.terminate = 0;
	thr->conti (thr);

	return thr->start (thr);
}

static int stiov2_stop (struct streamming_io_v2_server_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	self->pd.terminate = 1;
	return thr->stop (thr);
}

static int stiov2_pause (struct streamming_io_v2_server_t *self, const int p) {
	int	rc = self->pd.stop_services;

	if (p >= 0) self->pd.stop_services = p == 0 ? 0 : 1;

	return rc;
}

static int stiov2_regist (struct streamming_io_v2_server_t *self,
			const in_addr_t addr, const int n, const int sn) {
	struct client_entry_t	*cli = self->pd.cli[n];
	
	if ((n < 0) || (n >= self->pd.maxclient)) return -1;

	fprintf (stderr, "Regist on entry %d\n", n);

	return cli->regist (cli, addr, sn);
}

static void stiov2_set_fopen (struct streamming_io_v2_server_t *self,
					int (*f)(const int, const char *)) {
	self->pd.fopen = (f == NULL) ? dummy_fopen : f;
}

static int stiov2_set_parameter (struct streamming_io_v2_server_t *self,
				const int channel,
				const int diskbuf, const int netbuf) {
	if (self->pd.already_started) {
		fprintf (stderr, "Can not set parameter !!\n");
		return 0;
	}

	if ((channel < 0) || (channel > MAXIMUM_STIOV2_SERVICES)) {
		fprintf (stderr, "Channel out of range\n");
		return 0;
	}

	if ((diskbuf < 4096) || (diskbuf > DEFAULT_DISKIO_BUFFER_FAVIOR)) {
		fprintf (stderr, "Diskbuf size %d out of range\n", diskbuf);
		return 0;
	}

	if ((netbuf < 4096) || (netbuf > STIOV2_MAX_TRANS_BUFSIZE)) {
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
	self->pd.packet_map[channel]   = netbuf / STIOV2_CARRY_DATASIZE +
			(netbuf % STIOV2_CARRY_DATASIZE > 0 ? 1 : 0);

	fprintf (stderr, "[*] Channel %d streamming parameter [%d,%d,%d,%d]\n",
			channel,
			self->pd.disk_bufsize[channel],
			self->pd.net_bufsize[channel],
			self->pd.disknet_fact[channel],
			self->pd.packet_map[channel]);

	return 1;
}

static void stiov2_dispose (struct streamming_io_v2_server_t *self) {
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

struct streamming_io_v2_server_t * new_streamming_io_v2_server (
					const int maxclient, 
					const int port) {
	struct streamming_io_v2_server_t	*self;
	struct streamming_io_v2_server_pd_t	*pd;
	struct udplib_t				*udp;
	struct client_entry_t			**cli;
	int					i, initok = 1;

	while ((self = malloc (sizeof
				(struct streamming_io_v2_server_t))) != NULL) {
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

		self->start		= stiov2_start;
		self->stop		= stiov2_stop;
		self->dispose		= stiov2_dispose;
		self->cksum		= stiov2_cs_cksum;
		self->regist		= stiov2_regist;
		self->pause		= stiov2_pause;
		self->set_parameter	= stiov2_set_parameter;
		self->set_fopen		= stiov2_set_fopen;

		if ((pd->thr = new_thread_svc (
					streamio_v2_main, self)) == NULL) {
			initok = 0;
			break;
		}

		if ((pd->cli = calloc (maxclient,
				sizeof (struct client_entry_t *))) == NULL) {
			initok = 0;
			break;
		} else {
			cli = pd->cli;

			for (i = 0; i < maxclient; i++) {
				if ((cli[i] = new_client_entry (self))== NULL) {
					initok = 0;
					break;
				}
			}
		}

		for (i = 0; i < MAXIMUM_STIOV2_SERVICES; i++) {
			pd->disk_bufsize[i] = DEFAULT_DISKIO_BUFFER_FAVIOR;
			pd->net_bufsize[i]  = DEFAULT_STIOV2_TRANS_BUFSIZE;
			pd->disknet_fact[i] = DEFAULT_STIOV2_DISKNET_FACTOR;
			pd->packet_map[i]   = DEFAULT_STIOV2_PACKET_MAP; 
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
