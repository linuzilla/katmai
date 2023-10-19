/*
 *	udpcmdrcv.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "udpcmdrcv.h"
#include "udplib.h"
#include "genpdu.h"

static void * listen_main (void *ptr) {
	struct udpcmdrcv_t			*self = ptr;
	struct udpcmdrcv_private_data_t		*pd = &self->pd;
	struct udplib_t				*udp = pd->udp;
	struct genpdu_t				*pdu = pd->pdu;
	struct generic_pdu_callback_data_t	cbd;
	char					buffer[2000];
	int					len = 0;
	int					fd;
	fd_set					rfds;
	struct timeval				tv;
	int					rc, cmd;

	// fprintf (stderr, "listen thread started (%ld)\n", pthread_self ());

	fd = udp->getfd (udp);

	cbd.data = buffer;
	cbd.self = self;
	cbd.udp  = udp;


	while (! pd->terminate) {
		FD_ZERO (&rfds);
		FD_SET  (fd, &rfds);
		tv.tv_sec  = 5;
		tv.tv_usec = 0;

		if (select (fd+1, &rfds, NULL, NULL, &tv) < 0) {
			perror ("select");
			continue;
		}

		if (FD_ISSET (fd, &rfds)) {
			if ((len = udp->receive (udp,
					buffer, sizeof buffer)) <= 0) {
				perror ("receive");
				continue;
			}
			cbd.len  = len;

			rc = pdu->do_cmd (pdu, buffer, len, &cbd, &cmd);

			if (self->pd.callback != NULL) {
				self->pd.callback (rc, cmd);
			}
		}
	}

	// fprintf (stderr, "listen thread ended (%ld)\n", pthread_self ());

	udp->close (udp);
	self->pd.have_udp = 0;
	pd->services = 0;
	pthread_exit (NULL);
}

static int ucr_listen (struct udpcmdrcv_t *self, const int port,
					const char *intf, const int isbind) {
	struct udplib_t		*udp = self->pd.udp;

	if (self->pd.have_udp) udp->close (udp);
	if (udp->open (udp, port, intf, isbind) < 0) return -1;

	udp->set_sendflag (udp, 0);
	udp->set_recvflag (udp, 0);

	self->pd.terminate = 0;
	self->pd.services  = 1;

	// fprintf (stderr, "listen\n");

	pthread_create (&self->pd.thr, NULL, listen_main, self);

	return 1;
}

static struct udplib_t * ucr_udp (struct udpcmdrcv_t *self) {
	return self->pd.udp;
}

static struct genpdu_t * ucr_pdu (struct udpcmdrcv_t *self) {
	return self->pd.pdu;
}

static int ucr_stop (struct udpcmdrcv_t *self) {
	int	rc = 0;

	// fprintf (stderr, "stop listen services\n");
	if (self->pd.services) {
		self->pd.terminate = 1;
		pthread_join (self->pd.thr, NULL);
		rc = 1;
	}
	return rc;
}

static int ucr_addcmd (struct udpcmdrcv_t *self, struct gp_cmd_handler_t *lst) {
	struct genpdu_t	*pdu = self->pd.pdu;
	return pdu->addlist (pdu, lst);
}

static int ucr_bsend (struct udpcmdrcv_t *self, const int port, const int cmd) {
	struct udplib_t		*udp = self->pd.udp;
	struct genpdu_t		*pdu = self->pd.pdu;
	struct generic_pdu_t	gpdu;

	// fprintf (stderr, "BROADCAST SEND (%d)\n", cmd);

	pdu->pack (pdu, &gpdu, cmd, sizeof (struct generic_pdu_t));
	udp->bsend (udp, port, &gpdu, sizeof (struct generic_pdu_t));

	return 1;
}

static int ucr_bcast (struct udpcmdrcv_t *self, const int port,
					const int cmd, void *data, int len) {
	struct udplib_t		*udp = self->pd.udp;
	struct genpdu_t		*pdu = self->pd.pdu;

	// fprintf (stderr, "BROADCAST SEND (%d)\n", cmd);

	pdu->pack (pdu, data, cmd, len);
	udp->bsend (udp, port, data, len);

	return 1;
}

static int ucr_reply (struct udpcmdrcv_t *self, const int cmd,
						void *data, int len) {
	struct udplib_t		*udp = self->pd.udp;
	struct genpdu_t		*pdu = self->pd.pdu;
	struct generic_pdu_t	*gpdu = data;

	pdu->pack (pdu, gpdu, cmd, len);
	udp->send (udp, data, len);
	return 1;
}

static int ucr_regist_ftp (struct udpcmdrcv_t *self, const int cmd) {
	struct genpdu_t		*pdu = self->pd.pdu;

	if (self->pd.ftp != NULL) return 0;
	if ((self->pd.ftp =
			new_udpcmdrcv_sendfile (self, cmd)) == NULL) return 0;

	pdu->add (pdu, cmd, self->pd.ftp->handler);
	return 1;
}

static int ucr_sendfile (struct udpcmdrcv_t *self,
				const int n, const char *file) {
	struct udpcmdrcv_sendfile_t	*ftp;

	if ((ftp = self->pd.ftp) == NULL) return 0;

	return ftp->send (ftp, n, file);
}

static int ucr_recvfile (struct udpcmdrcv_t *self,
				const int n, const char *file) {
	struct udpcmdrcv_sendfile_t	*ftp;

	if ((ftp = self->pd.ftp) == NULL) return 0;

	return ftp->recv (ftp, n, file);
}

static int ucr_regist_addr (struct udpcmdrcv_t *self,
					const char *ip, const int port) {
	struct udplib_t		*udp = self->pd.udp;

	return udp->regist_addr (udp, ip, port);
}

static void ucr_set_callback (struct udpcmdrcv_t *self,
					void (*cbk)(const int, const int)) {
	self->pd.callback = cbk;
}

static void ucr_set_flag_callback (struct udpcmdrcv_t *self,
			unsigned int (*cbk)(struct genpdu_t *)) {
	struct genpdu_t	*pdu = self->pd.pdu;

	pdu->set_flag_callback (pdu, cbk);
}


struct udpcmdrcv_t * new_udpcmdrcv (const int nudp, const int ncmd) {
	struct udpcmdrcv_t		*self = NULL;
	struct udpcmdrcv_private_data_t	*pd;
	int				ok = 0;

	while ((self = malloc (sizeof (struct udpcmdrcv_t))) != NULL) {
		pd = &self->pd;
		if ((pd->udp = new_udplib (nudp)) == NULL) break;
		if ((pd->pdu = new_genpdu (ncmd)) == NULL) break;

		pd->terminate  = 0;
		pd->have_udp   = 0;
		pd->services   = 0;
		pd->ftp        = NULL;
		pd->callback   = NULL;

		self->udp		= ucr_udp;
		self->pdu		= ucr_pdu;
		self->listen		= ucr_listen;
		self->stop		= ucr_stop;
		self->addcmd		= ucr_addcmd;
		self->bsend		= ucr_bsend;
		self->bcast		= ucr_bcast;
		self->reply		= ucr_reply;
		self->regist_ftp	= ucr_regist_ftp;
		self->sendfile		= ucr_sendfile;
		self->recvfile		= ucr_recvfile;
		self->regist_addr	= ucr_regist_addr;
		self->set_callback	= ucr_set_callback;
		self->set_flag_callback	= ucr_set_flag_callback;

		ok = 1;
		break;
	}

	if ((self != NULL) && (! ok)) {
		free (self);
		self = NULL;
	}

	return self;
}
