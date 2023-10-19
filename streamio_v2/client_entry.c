/*
 *	client_entry.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "udplib.h"
#include "thread_svc.h"
#include "client_entry.h"
#include "streamio_v2.h"
#include "streamio_v2_pdu.h"

#include "common/cksum.c"

#define STREAMMING_MAX_PACKET	13

struct stio_v2_server_local_t {
	int		fd;
	int		filecnt;
	off_t		filesize;
	off_t		last_fptr;
	off_t		curr_fptr;
	char		*buffer;
	int		blkcnt;
	int		bufsize;
	int		buflen;
};

/*
static int dummy_fopen (const int i, const char *fname) {
	// char	buffer[256];

	// sprintf (buffer, "/home/cc/liujc/MPEG/80/12/%s", fname);
	// sprintf (buffer, "/home/cc/liujc/FoveonX3/download");

	return open (fname, O_RDONLY);
}
*/

static int fullread (const int fd, char *buffer, const int bufsiz,
					const int cptr, const int fsize) {
	int	explen = bufsiz;
	int	rc = 0;
	int	rlen;

	if ((rlen = fsize - cptr) < explen) explen = rlen;
	while (explen > 0) {
		if ((rlen = read (fd, &buffer[rc], explen)) > 0) {
			explen -= rlen;
			rc += rlen;
		} else {
			break;
		}
	}
	return rc;
}

static void remote_openfile (struct client_entry_t *self,
				struct stio_v2_server_local_t *sl,
				const int channel,
				void *buffer, const int len) {
	struct client_entry_pd_t	*pd = &self->pd;
	struct udplib_t			*udp = self->pd.udp;
	struct stio_v2_error_pdu_t	*errpdu = buffer;
	struct stio_v2_ft_open_pdu_t	*ftpdu =  buffer;
	struct stio_v2_ft_ok_pdu_t	*okpdu =  buffer;
	int				fd, errtmp;
	struct stat			stbuf;
	struct streamming_io_v2_server_pd_t	*svr = &pd->parent->pd;

	if (sl->fd >= 0) {
		fprintf (stderr, "close (%d)\n", sl->fd);
		close (sl->fd);
		sl->fd = -1;
	}

	fprintf (stderr, "Open File [%s] on channel %d\n",
					ftpdu->filename, ftpdu->channel);

	while (sl->fd < 0) {
		if (sl->buffer == NULL) {
			if ((sl->buffer = malloc (
					svr->disk_bufsize[channel]))==NULL) {
				errtmp = errno;
				perror (NULL);
				errpdu->lerrno = errtmp;
				break;
			}

			sl->bufsize = svr->disk_bufsize[channel];
		}

		if ((fd = pd->parent->pd.fopen (channel,
						ftpdu->filename)) < 0) {
			errtmp = errno;
			perror (ftpdu->filename);
			errpdu->lerrno = errtmp;
			break;
		}

		if (fstat (fd, &stbuf) != 0) {
			errtmp = errno;
			perror ("fstat");
			errpdu->lerrno = errtmp;
			break;
		}

		sl->fd		= fd;
		sl->filesize	= stbuf.st_size;
		sl->curr_fptr	= 0;
		sl->last_fptr	= 0;
		sl->blkcnt	= -1;

		pd->lastuse	= time (NULL);

		fprintf (stderr, "FD=%d, Filesize = %ld\n",
						sl->fd, sl->filesize);

		okpdu->filesize = sl->filesize;

		okpdu->disk_bufsize = svr->disk_bufsize[channel];
		okpdu->net_bufsize  = svr->net_bufsize[channel];
		okpdu->disknet_fact = svr->disknet_fact[channel];
		okpdu->packet_map   = svr->packet_map[channel];

		okpdu->cksum = self->cksum (okpdu,
					sizeof (struct stio_v2_ft_ok_pdu_t));

		udp->send (udp, okpdu, sizeof (struct stio_v2_ft_ok_pdu_t));
	}

	if (sl->fd < 0) {
		errpdu->orgcmd = errpdu->cmd;
		errpdu->cmd    = STIOV2_CMD_ERROR;
		errpdu->cksum = self->cksum (errpdu,
				sizeof (struct stio_v2_error_pdu_t));

		udp->send (udp, errpdu, sizeof (struct stio_v2_error_pdu_t));
	}
}

static void remote_fetch_file (struct client_entry_t *self,
				struct stio_v2_server_local_t *sl,
				const int channel,
				void *buffer, const int len) {
	//struct client_entry_pd_t	*pd = &self->pd;
	struct udplib_t			*udp = self->pd.udp;
	struct stio_v2_error_pdu_t	*errpdu = buffer;
	struct stio_v2_ft_req_pdu_t	*req    = buffer;
	struct stio_v2_ft_data_pdu_t	*ptr	= buffer;
	int				blkcnt;
	int				blkent;
	int				i, j, idx, clen;
	char				map[STIOV2_MAX_PACKET_MAP];
	struct streamming_io_v2_server_pd_t	*svr = &self->pd.parent->pd;

	if (sl->fd < 0) {
		fprintf (stderr, "File not opened\n");
		errpdu->orgcmd  = errpdu->cmd;
		errpdu->cmd     = STIOV2_CMD_ERROR;
		errpdu->lerrno  = EBADF;
		errpdu->errcode = STIOV2_ERR_REOPEN;
		errpdu->cksum = self->cksum (errpdu,
					sizeof (struct stio_v2_error_pdu_t));

		udp->send (udp, errpdu, sizeof (struct stio_v2_error_pdu_t));
		return;
	}

	memcpy (map, req->map, sizeof map);

	blkcnt = req->ndcnt / svr->disknet_fact[channel];
	blkent = req->ndcnt % svr->disknet_fact[channel];

	if (blkcnt != sl->blkcnt) {
		if (blkcnt != sl->blkcnt + 1) {
			lseek (sl->fd, blkcnt * svr->disk_bufsize[channel],
							SEEK_SET);
			// fprintf (stderr, "Seeking and reading\n");
		} else {
			// fprintf (stderr, "Reading\n");
		}

		sl->buflen = fullread (sl->fd,
					sl->buffer, sl->bufsize,
					sl->curr_fptr, sl->filesize);
		sl->blkcnt = blkcnt;
	}

	/*
	fprintf (stderr, "Fetch file on channel %d(%d,%d,%d)\n",
			channel, req->filecnt, req->ndcnt, req->len);
			*/

	for (i = j = 0; i < svr->packet_map[channel]; i++) {
		if (map[i] == 0) continue;

		idx = blkent * svr->net_bufsize[channel] +
						i * STIOV2_CARRY_DATASIZE;

		if (i == svr->packet_map[channel] - 1) {
			clen = svr->net_bufsize[channel] -
						i * STIOV2_CARRY_DATASIZE;
		} else {
			clen = STIOV2_CARRY_DATASIZE;
		}

		ptr->map = i;
		memcpy (ptr->data, &sl->buffer[idx], clen);

		ptr->cksum = self->cksum (ptr,
					sizeof (struct stio_v2_ft_data_pdu_t));

#ifdef STREAMMING_MAX_PACKET
// #error MAX packet
		if (++j % STREAMMING_MAX_PACKET == 0) usleep (1);
#endif

		udp->send (udp, ptr, sizeof (struct stio_v2_ft_data_pdu_t));
	}
}

static int ce_svc_main (void *selfptr) {
	struct client_entry_t		*self = selfptr;
	struct client_entry_pd_t	*pd = &self->pd;
	struct udplib_t			*udp = self->pd.udp;
	int				channel, len;
	char				buffer[2000];
	struct stio_v2_server_local_t	*sl;
	struct stio_v2_ft_open_pdu_t	*ftpdu = (void *) buffer;
	struct streamming_io_v2_server_pd_t *svr = &pd->parent->pd;

	fprintf (stderr, "Service started for %d\n", self->pd.sn);

	self->pd.in_services = 1;

	// sleep (1);

	while (! pd->terminate) {
		if ((len = udp->recv (udp, buffer, sizeof buffer, 5)) <= 0) {
			continue;
		} else if (svr->stop_services) {
			fprintf (stderr, "Services is stopping\n");
			continue;
		} else if (pd->addr != udp->get_remote (udp)) {
			fprintf (stderr, "Un-wanted client %s\n",
					udp->remote_addr (udp));
			continue;
		} else if (ftpdu->cksum != self->cksum (buffer, len)) {
			fprintf (stderr, "Cksum error !!\n");
			continue;
		} else if ((ftpdu->channel < 0) ||
				(ftpdu->channel >= MAXIMUM_STIOV2_SERVICES)) {
			fprintf (stderr, "Channel %d out of range\n",
					ftpdu->channel);
			continue;
		} else if (ftpdu->sn != pd->sn) {
			fprintf (stderr, "S/N: %d != %d\n", ftpdu->sn, pd->sn);
			continue;
		// } else if (rand () % 20 == 0) {
		//	fprintf (stderr, "packet drop!\n");
		//	continue;
		}

		sl = &pd->sl[channel = (int) ftpdu->channel];

		switch (ftpdu->cmd) {
		case STIOV2_CMD_OPENFILE:
			remote_openfile (self, sl, channel, buffer, len);
			break;
		case STIOV2_CMD_FETCH:
			remote_fetch_file (self, sl, channel, buffer, len);
			break;
		default:
			fprintf (stderr, "Unknow Command: CMD=%d, Channel=%d\n",
					ftpdu->cmd, ftpdu->channel);
			break;
		}
	}

	self->pd.in_services = 0;

	return 1;
}

static void ce_dispose (struct client_entry_t *self) {
	struct client_entry_pd_t	*pd = &self->pd;

	pd->terminate = 1;
	if (pd->thr != NULL) pd->thr->dispose (pd->thr);
	if (pd->udp != NULL) pd->udp->dispose (pd->udp);
	if (pd->sl  != NULL) {
		if (pd->sl->buffer != NULL) free (pd->sl->buffer);
		free (pd->sl);
	}

	free (self);
}

static int ce_start (struct client_entry_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	return thr->start (thr);
}

static int ce_stop (struct client_entry_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	return thr->stop (thr);
}

static int ce_regist (struct client_entry_t *self,
				const in_addr_t addr, const int sn) {
	struct client_entry_pd_t	*pd = &self->pd;
	struct udplib_t			*udp = pd->udp;
	struct thread_svc_t		*thr = pd->thr;
	int				rc;

	if (pd->inuse) {
		if (pd->addr == addr) {
			if (pd->sn != sn) {
				// Not the same client as before !!
				fprintf (stderr, "Client update\n");
				pd->sn = sn;
			}
			rc = pd->port;
		} else {
			self->unuse (self);
		}
	}

	if (! pd->inuse) {
		pd->inuse = 1;
		pd->addr  = addr;
		pd->sn    = sn;

		if (udp->open (udp, 0, NULL, 0) >= 0) {
			rc = pd->port = udp->local_port (udp);

			thr->start (thr);

			while (! pd->in_services) usleep (10);
		}
	}

	return rc;
}

static int ce_is_inuse (struct client_entry_t *self) {
	return self->pd.inuse;
}

static int ce_unuse (struct client_entry_t *self) {
	struct udplib_t		*udp = self->pd.udp;

	fprintf (stderr, "Unuse entry\n");
	self->pd.inuse = 0;
	udp->close (udp);
	return 1;
}

/*
static void ce_set_fopen (struct client_entry_t *self,
				int (*f)(const int, const char *)) {

	self->pd.fopen = (f == NULL) ? dummy_fopen : f;
}
*/

struct client_entry_t * new_client_entry (struct streamming_io_v2_server_t *p) {
	struct client_entry_t		*self;
	struct client_entry_pd_t	*pd;
	int				i, initok = 1;

	while ((self = malloc (sizeof (struct client_entry_t))) != NULL) {
		pd = &self->pd;

		self->dispose	= ce_dispose;
		self->start	= ce_start;
		self->stop	= ce_stop;
		self->unuse	= ce_unuse;
		self->regist	= ce_regist;
		self->is_inuse	= ce_is_inuse;
		self->cksum	= stiov2_cs_cksum;
		// self->set_fopen	= ce_set_fopen;

		pd->parent	= p;
		pd->thr		= NULL;
		pd->udp		= NULL;
		pd->sl		= NULL;
		pd->terminate	= 0;
		pd->in_services	= 0;
		// pd->fopen	= dummy_fopen;

		if ((pd->sl = calloc (MAXIMUM_STIOV2_SERVICES, sizeof (
				struct stio_v2_server_local_t))) == NULL) {
			initok = 0;
			break;
		}

		for (i = 0; i < MAXIMUM_STIOV2_SERVICES; i++) {
			pd->sl[i].fd	  = -1;
			pd->sl[i].filecnt = -1;
			pd->sl[i].buffer  = NULL;
			pd->sl[i].bufsize = 0;
		}

		if ((pd->thr = new_thread_svc (ce_svc_main, self)) == NULL) {
			initok = 0;
			break;
		}


		if ((pd->udp = new_udplib (0)) == NULL) {
			initok = 0;
			break;
		}
		

		break;
	}

	if (! initok) {
		self->dispose (self);
		return NULL;
	}

	return self;
}
