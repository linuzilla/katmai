/*
 *	rbmcast.c	(Realiable Broadcast/Multicast over UDP Library)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rbmcast.h"
#include "thread_svc.h"
#include "include/rbmpdu.h"
#include "include/rbmbuffer.h"
#include "include/rbmcmd.h"
#include "common/rbmcommon.c"
#include "common/rbmbuffer.c"


static int rbm_open (struct rbmcast_lib_t *self,
					const char *mgrp, const char *intf) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	struct hostent		*hp;
	struct ip_mreq		mreq;
	unsigned char		loop = pd->loopback;
	unsigned char		ttl  = pd->ttl;


	if (pd->fd >= 0) return -1;

	// bzero ((char *) &pd->addr, sizeof pd->addr);
	// bzero ((char *) &pd->mcaddr, sizeof pd->mcaddr);

	memset (&pd->addr, 0, sizeof pd->addr);
	memset (&pd->mcaddr, 0, sizeof pd->mcaddr);

	pd->addr.sin_family   = AF_INET;
	pd->mcaddr.sin_family = AF_INET;

	do {
		if (mgrp != NULL) {
			if ((hp = gethostbyname (mgrp)) == NULL) {
				perror ("[gethostbyname]");
				break;
			} else {
				struct in_addr	*ip;

				ip = (struct in_addr *) hp->h_addr;
				pd->mcast_group = ip->s_addr;
			}

			pd->mcaddr.sin_addr.s_addr  = pd->mcast_group;
			//  pd->mcaddr.sin_port	    = htons (port);
			pd->use_broadcast = 0;
		} else {
			pd->mcast_group = 0;
			pd->use_broadcast = 1;
		}

		if ((pd->fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror ("[socket]");
			break;
		}

		// bzero ((char *) &pd->bcaddr, sizeof pd->bcaddr);
		memset (&pd->bcaddr, 0, sizeof pd->bcaddr);
		pd->bcaddr.sin_family      = AF_INET;
		pd->bcaddr.sin_addr.s_addr = htonl (INADDR_BROADCAST);

		if (intf == NULL) {
			pd->our_addr = get_local_address (pd->mcast_group);

			pd->addr.sin_addr.s_addr = INADDR_ANY;
		} else {
			struct ifreq		if_data;
			struct sockaddr_in	*addr;
			struct in_addr		inaddr;

			strncpy (if_data.ifr_name, intf, IFNAMSIZ);

			if (ioctl (pd->fd, SIOCGIFBRDADDR, &if_data) < 0) {
				perror (intf);
			} else {
				addr = (struct sockaddr_in *)
						&if_data.ifr_ifru.ifru_addr;
				pd->bcaddr.sin_addr = addr->sin_addr;
			}

			if (ioctl (pd->fd, SIOCGIFADDR, &if_data) < 0) {
				perror (intf);
				break;
			}

			addr = (struct sockaddr_in *)
						&if_data.ifr_ifru.ifru_addr;
			pd->our_addr = addr->sin_addr.s_addr;
			// printf ("Using %s\n", inet_ntoa(addr->sin_addr));

			pd->addr.sin_addr.s_addr = addr->sin_addr.s_addr;

			inaddr = addr->sin_addr;

			if (mgrp != NULL) {
				setsockopt (pd->fd, IPPROTO_IP,
						IP_MULTICAST_IF,
						&inaddr,
						sizeof(struct in_addr));
			}
		}

		if (mgrp != NULL) {
			mreq.imr_multiaddr.s_addr = pd->mcast_group;
			mreq.imr_interface.s_addr = pd->our_addr;

			setsockopt (pd->fd, IPPROTO_IP,
					IP_ADD_MEMBERSHIP, &mreq, sizeof mreq);

			if (setsockopt (pd->fd, IPPROTO_IP, IP_MULTICAST_LOOP,
						&loop, sizeof loop) < 0) {
				perror ("setsockopt (IP_MULTICAST_LOOP)");
			}

			if (setsockopt (pd->fd, IPPROTO_IP, IP_MULTICAST_TTL,
					       	&ttl,  sizeof ttl) < 0) {
				perror ("setsockopt (IP_MULTICAST_TTL)");
			}
		}

		// printf ("Using %s as Broadcast Address\n",
		//		inet_ntoa (pd->bcaddr.sin_addr));
	} while (0);

	if (pd->fd >= 0) {
		int	flag = 1;
		// int	loop_size;

		if (setsockopt (pd->fd, SOL_SOCKET, SO_REUSEADDR,
					(char *) &flag, sizeof flag) < 0) {
			perror ("setsockopt (SO_REUSEADDR)");
		}

		if (pd->use_broadcast) {
			if (setsockopt (pd->fd, SOL_SOCKET, SO_BROADCAST,
					(char *) &flag, sizeof flag) < 0) {
				perror ("setsockopt (SO_BROADCAST)");
			}
		}

		/*
		loop_size = sizeof loop;
		getsockopt (self->pd.fd, IPPROTO_IP,
					IP_MULTICAST_LOOP, &loop, &loop_size);

		fprintf (stderr, "Loopback=%d\n", loop);
		*/
	}

	return 1;
}

static int rbm_bind (struct rbmcast_lib_t *self,
					const int port, const int magic,
					const int is_server) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	int			i;

	pd->port	= port - (port % 2);
	pd->magic_id	= magic;


	if (is_server > 0) {
		srand (time (NULL));
		pd->server_id	     = rand () % 65530 + 1;
		pd->is_server        = 1;
		pd->addr.sin_port    = htons (pd->port);
		pd->bcaddr.sin_port  = htons (pd->port + 1);
		pd->mcaddr.sin_port  = htons (pd->port + 1);

		if ((pd->addrcnt = is_server) > 512) {
			pd->addrcnt = 512;
		}

		pd->addrlist = malloc (pd->addrcnt *
					sizeof (struct sockaddr_in));
		pd->addruse  = malloc (pd->addrcnt * sizeof (short));

		if ((pd->addrlist == NULL) || (pd->addruse == NULL)) {
			perror ("malloc");
			return -1;
		}

		for (i = 0; i < pd->addrcnt; i++) pd->addruse[i] = 0;

		pd->addruse_cnt = 0;
	} else {
		pd->server_id        = 0;
		pd->is_server        = 0;
		pd->addr.sin_port    = htons (pd->port + 1);
		pd->bcaddr.sin_port  = htons (pd->port);
		pd->mcaddr.sin_port  = htons (pd->port);
	}

	if (bind (pd->fd, (struct sockaddr *)
					&pd->addr, sizeof pd->addr) != 0) {
		perror ("bind");
		return -1;
	}

	return 1;
}

static int regist_client (struct rbmcast_lib_t *self,
						struct sockaddr_in *from) {
	struct rbmcast_lib_pd_t		*pd = &self->pd;
	int				i, f;

	for (i = 0, f = -1; i < pd->addrcnt; i++) {
		if (pd->addruse[i] == 0) {
			if (f < 0) f = i;
		} else if (memcmp (&from->sin_addr,
					&pd->addrlist[i].sin_addr,
					sizeof from->sin_addr) == 0) {
			// Already regist
			return i;
		}
	}

	if (f >= 0) {
		pd->addruse[f] = 1;
		memcpy (&pd->addrlist[f], from, sizeof (struct sockaddr_in));

		pd->addruse_cnt++;
	}

	return f;
}

static int listen_main (void *selfptr) {
	struct rbmcast_lib_t		*self = selfptr;
	struct rbmcast_lib_pd_t		*pd = &self->pd;
	char				buffer[2000];
	struct rbmc_header_pdu_t	*pdu = (void *) buffer;
	struct timeval			tv;
	struct timeval			*tptr = NULL;
	int				msec = 1000;
	fd_set				rfds;
	struct sockaddr_in		from;
	socklen_t			fromlen;
	int				i, len;
	struct rbm_win_buffer_t		*ptr;

	while (! pd->terminate) {
		if (msec > 0) {
			tv.tv_sec  = msec / 1000;
			tv.tv_usec = (msec % 1000) * 1000;
			tptr       = &tv;
		} else {
			tptr = NULL;
		}

		FD_ZERO (&rfds);
		FD_SET  (pd->fd, &rfds);

		// fprintf (stderr, "Ready to select ...\n");

		if (select (pd->fd + 1, &rfds, NULL, NULL, tptr) <= 0) {
			if (errno > 0) perror ("[select]");
			continue;
		} else if (! FD_ISSET (pd->fd, &rfds)) {
			// Not the one I want !!
			fprintf (stderr, "Not the one I want !!\n");
			continue;
		}

		fromlen = sizeof from;

		len = recvfrom (pd->fd, buffer, sizeof buffer,
					pd->recvflag,
					(struct sockaddr *) &from, &fromlen);


		if (len < sizeof (struct rbmc_header_pdu_t)) {
			// Short Packet !!
			fprintf (stderr, "Short packet from: %s\n",
						inet_ntoa (from.sin_addr));
			continue;
		}

		if (pdu->magic_id != pd->magic_id) {
			fprintf (stderr, "Incorrect magic id from: %s\n",
						inet_ntoa (from.sin_addr));
			continue; // Magic ID
		}

		if (pdu->cksum != rbm_cksum (pdu, len)) {
			fprintf (stderr, "Incorrect cksum from: %s\n",
						inet_ntoa (from.sin_addr));
			continue; // cksum
		}

		fprintf (stderr, "%s from: %s (CMD=%d)\n",
				pd->is_server ? "Receieve" : "Reply",
				inet_ntoa (from.sin_addr), pdu->cmd);

		switch (pdu->cmd) {
		case RBMCMD_CONNECT:
			pdu->server_id = pd->server_id;

			if (pd->stage == RBMST_LISTEN) {
				if ((i = regist_client (self, &from)) >= 0) {
					pdu->cmd = RBMCMD_CONN_OK;
					pdu->client_id = i;
				} else {
					pdu->cmd = RBMCMD_CONN_FULL;
				}
			} else {
				pdu->cmd = RBMCMD_NOT_ALLOW;
			}

			pdu->cksum = rbm_cksum (pdu, sizeof *pdu);

			sendto (pd->fd, pdu, sizeof *pdu, pd->sendflag,
				(struct sockaddr *) &from,
				sizeof from);
			break;
		case RBMCMD_CONN_OK:
			if (pd->stage != RBMST_CONNECT) break;

			fprintf (stderr,
				"Connect ok, Server ID=%u, Client ID=%d\n",
				pdu->server_id,
				pdu->client_id);

			pd->server_id = pdu->server_id;
			pd->client_id = pdu->client_id;

			pthread_mutex_lock (&pd->mutex);
			pd->stage = RBMST_WAIT_DATA;
			pthread_cond_signal (&pd->condi);
			pthread_mutex_unlock (&pd->mutex);

			break;
		case RBMCMD_CONN_FULL:
			break;
		case RBMCMD_NOT_ALLOW:
			break;
		case RBMCMD_DATA: // Client 收到 Server 送來的 data
			if (pd->stage != RBMST_WAIT_DATA) break;
			if (pdu->seq < pd->expect_seq) break;	// Don't care
			if (pdu->seq > pd->max_expect_seq) {
				break;
			}

			ptr = rbm_get_free_win_buf (self, pdu->seq);

			break;
		default:
			break;
		}
	}

	return 0;
}

static int rbm_listen (struct rbmcast_lib_t *self, const int noc) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	struct thread_svc_t	*thr;

	if (pd->thr != NULL) return 0;

	pd->terminate = 0;

	if (pd->is_server) {
		if (allocate_win_buffer (self)) {
			pd->stage = RBMST_LISTEN;

			pd->thr = thr = new_thread_svc (listen_main, self);
			thr->start (thr);
		} else {
			return 0;
		}
	}

	return 1;
}

static int rbm_connect (struct rbmcast_lib_t *self, const int msec) {
	struct rbmcast_lib_pd_t		*pd = &self->pd;
	struct rbmc_header_pdu_t	hdr;
	struct thread_svc_t		*thr;

	if (pd->thr == NULL) {
		if (allocate_win_buffer (self)) {
			pd->terminate = 0;
			pd->thr = thr = new_thread_svc (listen_main, self);
			thr->start (thr);
		} else {
			return 0;
		}
	}

	pd->stage = RBMST_CONNECT;

	hdr.magic_id  = pd->magic_id;
	hdr.cmd       = RBMCMD_CONNECT;
	hdr.server_id = 0;
	hdr.length    = 0;
	hdr.cksum     = rbm_cksum (&hdr, sizeof hdr);

	if (pd->use_broadcast) {
		fprintf (stderr, "Broadcast connect\n");
		sendto (pd->fd, &hdr, sizeof hdr, pd->sendflag,
				(struct sockaddr *) &pd->bcaddr,
				sizeof pd->bcaddr);
	} else {
		fprintf (stderr, "Multicast connect\n");
		sendto (pd->fd, &hdr, sizeof hdr, pd->sendflag,
				(struct sockaddr *) &pd->mcaddr,
				sizeof pd->mcaddr);
	}

	if (pd->stage == RBMST_CONNECT) {
		struct timeval		now;
		struct timespec		timeout;
		int			rc;

		gettimeofday (&now, NULL);
		timeout.tv_sec  = now.tv_sec + 1;
		timeout.tv_nsec = now.tv_usec * 1000;

		pthread_mutex_lock (&pd->mutex);
		rc = pthread_cond_timedwait (&pd->condi, &pd->mutex, &timeout);
		pthread_mutex_unlock (&pd->mutex);

		if ((rc != 0) && (errno == ETIMEDOUT)) {
			perror ("pthread_cond_timedwait");
		}
	}

	if (pd->stage == RBMST_WAIT_DATA) return 1;

	return 0;
}

static int rbm_loopback (struct rbmcast_lib_t *self, const int lp) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	int			rc = pd->loopback;

	if (lp >= 0) {
		self->pd.loopback = lp > 0 ? 1 : 0;

		if ((pd->fd >= 0) && (pd->mcast_group != 0)) {
			struct ip_mreq		mreq;
			unsigned char		loop = lp;

			mreq.imr_multiaddr.s_addr = pd->mcast_group;
			mreq.imr_interface.s_addr = pd->our_addr;

			setsockopt (pd->fd, IPPROTO_IP,
					IP_MULTICAST_LOOP, &loop, sizeof loop);
		}
	}

	return rc;
}

static int rbm_ttl (struct rbmcast_lib_t *self, const int ttl) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	int			rc = pd->ttl;

	if (ttl >= 0) {
		self->pd.ttl = ttl;

		if ((pd->fd >= 0) && (pd->mcast_group != 0)) {
			struct ip_mreq		mreq;
			unsigned char		myttl = ttl;

			mreq.imr_multiaddr.s_addr = pd->mcast_group;
			mreq.imr_interface.s_addr = pd->our_addr;

			setsockopt (pd->fd, IPPROTO_IP,
					IP_MULTICAST_TTL, &myttl, sizeof myttl);
		}
	}

	return rc;
}

static void rbm_close (struct rbmcast_lib_t *self) {
	if (self->pd.fd >= 0) {
		close (self->pd.fd);
		self->pd.fd = -1;
	}
}

static void rbm_stop (struct rbmcast_lib_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	if (thr != NULL) {
		self->pd.terminate = 1;
		thr->stop (thr);
	}
}

static void rbm_dispose (struct rbmcast_lib_t *self) {
	if (self->pd.thr != NULL) {
		struct thread_svc_t	*thr = self->pd.thr;

		self->stop (self);
		thr->dispose (thr);
	}

	self->close (self);
	free (self);
}

static int rbm_set_sendflag (struct rbmcast_lib_t *self, const int f) {
	int	rc = self->pd.sendflag;

	self->pd.sendflag = f;
	return rc;
}

static int rbm_set_recvflag (struct rbmcast_lib_t *self, const int f) {
	int	rc = self->pd.recvflag;

	self->pd.recvflag = f;
	return rc;
}

static int rbm_num_of_client (struct rbmcast_lib_t *self) {
	return self->pd.addruse_cnt;
}

static int rbm_mtu (struct rbmcast_lib_t *self) {
	return RBM_MAX_TRANSMIT_DATA;
}

static int rbm_send (struct rbmcast_lib_t *self,
					const void *buf, const int len) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	struct rbm_win_buffer_t	*ptr;
	struct rbmc_pdu_t	*pdu;
	int			restlen = len;
	int			i;

	if (pd->stage == RBMST_LISTEN) {
		pd->stage = RBMST_TRANSMIT;
	} else if (pd->stage != RBMST_TRANSMIT) {
		// Only Server can transmit via send
		return 0;
	}

	while (restlen > 0) {
		ptr = rbm_get_free_win_buf (self, 0);
		pdu = &ptr->pdu;

		pdu->magic_id	= pd->magic_id;
		pdu->server_id	= pd->server_id;
		pdu->client_id	= pd->client_id;
		pdu->cmd	= RBMCMD_DATA;
		pdu->seq	= ++pd->seq;
		pdu->win	= pd->winsize;

		if (restlen > RBM_MAX_TRANSMIT_DATA) {
			i = RBM_MAX_TRANSMIT_DATA;
			restlen -= RBM_MAX_TRANSMIT_DATA;
		} else {
			i = restlen;
			restlen = 0;
		}

		memcpy (pdu->data, buf, len);

		pthread_mutex_lock (&ptr->mutex);
		ptr->ready = 1;
		pthread_cond_signal (&ptr->condi);
		pthread_mutex_unlock (&ptr->mutex);

		pdu->length = i;
		ptr->len = (sizeof *pdu) - RBM_MAX_TRANSMIT_DATA + i;

		pdu->cksum = rbm_cksum (pdu, sizeof *pdu);

		if (pd->use_broadcast) {
			// fprintf (stderr, "Broadcast connect\n");
			sendto (pd->fd, pdu, ptr->len, pd->sendflag,
				(struct sockaddr *) &pd->bcaddr,
				sizeof pd->bcaddr);
		} else {
			// fprintf (stderr, "Multicast connect\n");
			sendto (pd->fd, pdu, ptr->len, pd->sendflag,
				(struct sockaddr *) &pd->mcaddr,
				sizeof pd->mcaddr);
		}
	}

	return 1;
}

static int rbm_recv (struct rbmcast_lib_t *self, void *buf, const int len) {
	int	rc;

	return rc;
}

struct rbmcast_lib_t *new_rbmcast (void) {
	struct rbmcast_lib_t	*self;
	struct rbmcast_lib_pd_t	*pd;


	if ((self = malloc (sizeof (struct rbmcast_lib_t))) != NULL) {
		pd = &self->pd;

		pd->magic_id		= 0;
		pd->port		= 0;
		pd->mcast_group		= 0;
		pd->ttl			= 1;
		pd->loopback		= 0;
		pd->fd			= -1;
		pd->use_broadcast	= 0;
		pd->sendflag		= 0;
		pd->recvflag		= 0;
		pd->is_server		= 0;
		pd->client_id		= -1;
		pd->server_id		= 0;
		pd->stage		= RBMST_INIT;
		pd->addrlist		= NULL;
		pd->addruse		= NULL;
		pd->addrcnt		= 0;
		pd->addruse_cnt		= 0;
		pd->seq			= 0;
		pd->errcode		= 0;

		pd->expect_seq		= 1;
		pd->max_expect_seq	= RBM_MAX_WIN_BUFFER;
		pd->max_recv_seq	= 0;

		pd->wbuf		= NULL;
		pd->wb_front		= 0;
		pd->wb_rear		= 0;
		pd->wb_ptr		= 0;
		pd->wb_cnt		= 0;
		pd->winsize		= 1;
		pd->thr			= NULL;
		pd->sender		= NULL;

		pthread_mutex_init (&pd->mutex, NULL);
		pthread_cond_init  (&pd->condi, NULL);

		pthread_mutex_init (&pd->wb_mutex, NULL);
		pthread_cond_init  (&pd->wb_condi, NULL);

		pthread_mutex_init (&pd->wb_ready_mutex, NULL);
		pthread_cond_init  (&pd->wb_ready_condi, NULL);

		self->open		= rbm_open;
		self->close		= rbm_close;
		self->stop		= rbm_stop;
		self->listen		= rbm_listen;
		self->connect		= rbm_connect;
		self->bind		= rbm_bind;
		self->send		= rbm_send;

		self->dispose		= rbm_dispose;

		self->ttl		= rbm_ttl;
		self->loopback		= rbm_loopback;

		self->set_sendflag	= rbm_set_sendflag;
		self->set_recvflag	= rbm_set_recvflag;

		self->num_of_client	= rbm_num_of_client;
		self->mtu		= rbm_mtu;
	}

	return self;
}
