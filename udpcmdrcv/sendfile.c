/*
 *	sendfile.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include "tcplib.h"
#include "genpdu.h"
#include "udplib.h"
#include "udpcmdrcv.h"

#define SendfilePDUcmd_HELLO	1
#define SendfilePDUcmd_SENDREQ	2
#define SendfilePDUcmd_SENDACK	3

struct sendfile_pdu_t {
	struct generic_pdu_t	h;
	int			cmd;
	int			id;
	int			port;
};

static int entering_ftpmode (struct udpcmdrcv_sendfile_t *self) {
	int	rc = 0;

	pthread_mutex_lock (&self->pd.mutex);
	if (self->pd.inftp == 0) {
		self->pd.inftp = 1;
		rc = 1;
	}
	pthread_mutex_unlock (&self->pd.mutex);
	return rc;
}

static void leaving_ftpmode (struct udpcmdrcv_sendfile_t *self) {
	pthread_mutex_lock (&self->pd.mutex);
	self->pd.inftp = 0;
	pthread_mutex_unlock (&self->pd.mutex);
}

static void callback (const int fd, void *param) {
	int		len;
	char		buffer[2048];

	fprintf (stderr, "Calling comming up from: %d\n", fd);

	while ((len = read (fd, buffer, sizeof buffer)) > 0) {
		fprintf (stderr, "Callback data\n");
		write (STDOUT_FILENO, buffer, len);
	}
	fprintf (stderr, "Callback ended\n");
	close (fd);
}

static int rcvfile_prepare (void) {
	static struct tcplib_t	*tcp = NULL;
	int			port;

	if (tcp != NULL) tcp->dispose (tcp);

	if ((tcp = new_tcplib ()) == NULL) return -1;

	tcp->multiconn (tcp, 0);

	if ((port = tcp->listen (tcp, callback, 0, NULL)) < 0) {
		tcp->dispose (tcp);
		return -1;
	}

	return port;
}

static int cmd_handler (struct generic_pdu_callback_data_t *ptr) {
	// struct udplib_t			*udp    = ptr->udp;
	struct sendfile_pdu_t		*sfpdu  = ptr->data;
	struct udpcmdrcv_t		*cmdrcv = ptr->self;
	struct udpcmdrcv_sendfile_t	*self   = cmdrcv->pd.ftp;
	int				port;
	struct sendfile_pdu_t		sfrpdu;
	struct udplib_t			*udp = self->pd.udp;

	/*
	fprintf (stderr, "[%d] SELF Pointer (%d)\n", getpid (), self);
	*/

	if (ptr->len < sizeof (struct sendfile_pdu_t)) {
		fprintf (stderr, "Short packet\n");
		return 1;
	}

	fprintf (stderr, "SENDFILE command handler [%s] CMD:%d\n",
				udp->remote_addr (udp), sfpdu->cmd);

	switch (sfpdu->cmd) {
	case SendfilePDUcmd_HELLO:
		fprintf (stderr, "HELLO\n");
		break;
	case SendfilePDUcmd_SENDREQ:
		fprintf (stderr, "SENDFILE REQ\n");

		if (entering_ftpmode (self)) {
			if ((port = rcvfile_prepare ()) >= 0) {
				self->pd.rmid = sfpdu->id;

				sfrpdu.id   = self->pd.id;
				sfrpdu.cmd  = SendfilePDUcmd_SENDACK;
				sfrpdu.port = port;

				self->pd.pdu->pack (
					self->pd.pdu, &sfrpdu, self->pd.cmd,
					sizeof (struct sendfile_pdu_t));

				udp->send (udp, &sfrpdu,
						sizeof (struct sendfile_pdu_t));
			}
		} else if (self->pd.rmid == sfpdu->id) {
		} else {
			fprintf (stderr, "lock busy!\n");
		}
		break;
	case SendfilePDUcmd_SENDACK:
		fprintf (stderr, "SENDFILE ACK on port %d\n",
				sfpdu->port);
		self->pd.tcpport = sfpdu->port;
		break;
	default:
		break;
	}

	return 1;
}

/*
static void * ftprecv_main (void *ptr) {
	pthread_exit (NULL);
}
*/

static void * ftpsend_main (void *ptr) {
	struct udpcmdrcv_sendfile_t	*self = ptr;
	char				buffer[2000];
	struct sendfile_pdu_t		*sfpdu =
					    (struct sendfile_pdu_t *) buffer;
	struct udplib_t			*udp = self->pd.udp;
	struct tcplib_t			*tcp;
	int				fd;

	if ((tcp = new_tcplib ()) != NULL) {
		sfpdu->id  = self->pd.id;
		sfpdu->cmd = SendfilePDUcmd_SENDREQ;

		self->pd.pdu->pack (self->pd.pdu, sfpdu, self->pd.cmd,
					sizeof (struct sendfile_pdu_t));

		self->pd.tcpport = -1;

		udp->sendto (udp, self->pd.udpslot, sfpdu,
					sizeof (struct sendfile_pdu_t));

		fprintf (stderr, "Sendfile started\n");

		while (self->pd.tcpport < 0) { sleep (1); }

		fprintf (stderr, "ok, remote port is %d\n",
				self->pd.tcpport);

		fd = tcp->connect (tcp, udp->ip_addr (udp, self->pd.udpslot),
				self->pd.tcpport);

		if (fd > 0) {
			write (fd, "test\n", 5);
		}

		tcp->dispose (tcp);
	}

	close (self->pd.fd);
	self->pd.transfer_completed = 1;
	leaving_ftpmode (self);
	pthread_exit (NULL);
}

static int ftp_recv (struct udpcmdrcv_sendfile_t *self,
					const int n, const char *file) {
	return 1;
}

static int ftp_send (struct udpcmdrcv_sendfile_t *self,
					const int n, const char *file) {
	int		need_leaving = 0;
	int		fd;
	struct stat	stbuf;

	while (entering_ftpmode (self)) {
		need_leaving = 1;

		if ((fd = open (file, O_RDONLY)) < 0) {
			perror (file);
			break;
		}

		if (fstat (fd, &stbuf) < 0) {
			close (fd);
			fd = -1;
			break;
		}

		self->pd.udpslot		= n;
		self->pd.filesize		= stbuf.st_size;
		self->pd.fd			= fd;
		self->pd.transfer_completed 	= 0;

		pthread_create (&self->pd.thr, NULL, ftpsend_main, self);

		return 1;
	}

	if (need_leaving) leaving_ftpmode (self);

	return 0;
}

struct udpcmdrcv_sendfile_t * new_udpcmdrcv_sendfile (
			struct udpcmdrcv_t *parent, const int cmd) {

	struct udpcmdrcv_sendfile_t	*self;

	if ((self = malloc (sizeof (struct udpcmdrcv_sendfile_t))) != NULL) {
		self->pd.cmd			= cmd;
		self->pd.inftp			= 0;
		self->pd.transfer_completed	= 1;
		self->pd.rmid			= -1;
		self->pd.udp			= parent->pd.udp;
		self->pd.pdu			= parent->pd.pdu;

		self->handler	= cmd_handler;
		self->send	= ftp_send;
		self->recv	= ftp_recv;


		pthread_mutex_init (&self->pd.mutex, NULL);

		srand (time (NULL));
		self->pd.id	= rand () % 8388608;

		// fprintf (stderr, "[%d] SELF Pointer (%d)\n",
		// 					getpid (), self);
	}

	return self;
}
