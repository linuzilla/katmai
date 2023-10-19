/*
 *	tcplib.c
 *
 *	Copyright (c) 2002, Jainn-Ching Liu
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "tcplib.h"

static void sigchild_handler (const int signo) {
	int	status;
	while (waitpid (0, &status, WNOHANG) >= 0);
}

static void * listen_main (void *ptr) {
	struct tcplib_t			*self = ptr;
	struct tcplib_private_data_t	*pd = &self->pd;
	fd_set				fds;
	struct timeval			timeout;
	int				fd;
	struct sockaddr_in		client_addr;
	int				client_len = sizeof client_addr;
	void				(*oldhandler)(const int);
	int				rc;

	listen (pd->fd, 5);


	oldhandler = signal (SIGCHLD, sigchild_handler);

	while (! pd->terminate) {
		FD_ZERO (&fds);
		FD_SET  (pd->fd, &fds);
		timeout.tv_sec = 2;
		timeout.tv_usec = 500000;

		if ((rc = select (pd->fd + 1,
					&fds, NULL, NULL, &timeout)) < 0) {
			if (errno != EINTR) perror ("select");
			continue;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET (pd->fd, &fds)) {
			fd = accept (pd->fd, (struct sockaddr *) &client_addr,
					&client_len);

			if (fd > 0) {
				if (fork () == 0) {
					close (pd->fd);
					/*
					fprintf (stderr, "Connect from: %s\n",
					      inet_ntoa (client_addr.sin_addr));
					*/
					pd->callback (fd, pd->param);
					exit (1);
				}

				close (fd);

				if (! pd->multiconn) pd->terminate = 1;
			}
		}
	}

	// fprintf (stderr, "TCP Listen stopped\n");
	signal (SIGCHLD, oldhandler);

	self->pd.terminate = 0;
	pthread_exit (NULL);
}

static int tcl_prepare_socket (struct tcplib_t *self, const int port) {

	struct tcplib_private_data_t	*pd = &self->pd;
	char				on = 1;
	int				portno;

	if ((pd->fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
		return -1;
	}

	setsockopt (pd->fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof on);
	setsockopt (pd->fd, SOL_SOCKET, SO_KEEPALIVE, (char*) &on, sizeof on);

	bzero (&pd->myaddr, sizeof pd->myaddr);
	pd->myaddr.sin_family      = AF_INET;
	pd->myaddr.sin_addr.s_addr = INADDR_ANY;
	pd->myaddr.sin_port        = htons (port);

	if (bind (pd->fd, (struct sockaddr *) &pd->myaddr,
				sizeof (struct sockaddr)) < 0) {
		perror ("bind");
		close (pd->fd);
		pd->fd = -1;
		return -1;
	} else {
		int	len = sizeof pd->myaddr;

		if (getsockname (pd->fd, (struct sockaddr *) &pd->myaddr,
								&len) < 0) {
			perror ("getsockname");
			close (pd->fd);
			pd->fd = -1;
			return -1;
		}
	}
	portno = ntohs (pd->myaddr.sin_port);

	/*
	if (portno != port) {
		fprintf (stderr, "local port number = %d\n", portno);
	}
	*/

	return portno;
}

static int tcl_listen (struct tcplib_t *self,
			void (*cbk)(const int, void *),
			const int port, void *param) {
	struct tcplib_private_data_t	*pd = &self->pd;
	int				rc;

	if ((rc = tcl_prepare_socket (self, port)) < 0 ) return -1;

	pd->callback = cbk;
	pd->param = param;
	self->pd.terminate = 0;

	self->pd.have_thr = 1;
	// fprintf (stderr, "TCP listen started\n");
	pthread_create (&pd->thr, NULL, listen_main, self);

	return rc;
}

static void tcl_stop (struct tcplib_t *self) {
	if (self->pd.have_thr) {
		self->pd.terminate = 1;
		pthread_join (self->pd.thr, NULL);
		self->pd.have_thr = 0;
	}
}

static int tcl_connect (struct tcplib_t *self, const char *ip, const int port) {
	struct tcplib_private_data_t	*pd = &self->pd;
	struct hostent			*hp;
	int				len;
	int				portno;

	// fprintf (stderr, "TCP connect started\n");

	if ((portno = tcl_prepare_socket (self, 0)) < 0 ) return -1;

	memset (&pd->rmaddr, 0, sizeof (pd->rmaddr));

	if (inet_aton (ip, &pd->rmaddr.sin_addr) != 0) {
		self->pd.rmaddr.sin_family = AF_INET;
	} else if ((hp = gethostbyname (ip)) != NULL) {
		pd->rmaddr.sin_family = hp->h_addrtype;
		len = hp->h_length;

		if (len > sizeof pd->rmaddr.sin_addr) {
			len = sizeof pd->rmaddr.sin_addr;
		}

		memcpy (&pd->rmaddr.sin_addr, hp->h_addr_list[0], len);
	} else {
		perror (ip);
		return -1;
	}

	pd->rmaddr.sin_port = htons (port);

	if (connect (pd->fd, (struct sockaddr *) &pd->rmaddr,
				sizeof pd->rmaddr) < 0) {
		perror ("connect");
		close (pd->fd);
		return -1;
	}

	return pd->fd;
}

static void tcl_close (struct tcplib_t *self) {
	if (self->pd.fd >= 0) {
		shutdown (self->pd.fd, 0);
		close (self->pd.fd);
		self->pd.fd = -1;
	}
}

static void tcl_dispose (struct tcplib_t *self) {
	self->close (self);
	free (self);
}

static int tcl_multiconn (struct tcplib_t *self, const int n) {
	int	rc = self->pd.multiconn;
	
	if (n >= 0) self->pd.multiconn = (n > 0) ? 1 : 0;
	return rc;
}

struct tcplib_t	* new_tcplib (void) {
	struct tcplib_t		*self;

	if ((self = malloc (sizeof (struct tcplib_t))) != NULL) {
		self->pd.terminate	= 0;
		self->pd.have_thr	= 0;
		self->pd.callback	= NULL;
		self->pd.multiconn	= 0;
		self->pd.param		= NULL;

		self->listen		= tcl_listen;
		self->multiconn		= tcl_multiconn;
		self->stop		= tcl_stop;
		self->connect		= tcl_connect;
		self->close		= tcl_close;
		self->dispose		= tcl_dispose;
	}

	return self;
}
