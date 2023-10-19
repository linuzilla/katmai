/*
 *	mcast.c
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// #include "vodstb.h"
#include "mcast.h"

#define DEFAULT_TTL	4


static u_int32_t get_local_address (u_int32_t mcast_group) {
	// Find a good local address for us

	int                 sockfd;
	struct sockaddr_in  addr;
	int                 addrlen = sizeof(addr);
	// struct in_addr      *mcast_ptr = (struct in_addr *) &mcast_group;

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = mcast_group;
	addr.sin_port        = 0;	// htons (2000);

	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0
	    || connect (sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0
	    || getsockname (sockfd, (struct sockaddr *) &addr, &addrlen) < 0) {
		perror ("Determining local address");
		exit (1);
	}

	close (sockfd);

	return addr.sin_addr.s_addr;
}

static void mclib_close (struct mcast_lib_t *self) {
	if (self->pd.fd >= 0) close (self->pd.fd);

	self->pd.fd = -1;
}

static int mclib_bind (struct mcast_lib_t *self,
				const in_addr_t addr, const int port) {
	bzero((char *) &self->pd.addr, sizeof self->pd.addr);
	self->pd.addr.sin_family         = AF_INET;
	self->pd.addr.sin_addr.s_addr    = addr;
	self->pd.addr.sin_port           = htons (port);

	bzero ((char *) &self->pd.mcaddr, sizeof self->pd.mcaddr);
	self->pd.mcaddr.sin_family       = AF_INET;
	self->pd.mcaddr.sin_addr.s_addr  = self->pd.mcast_group;
	self->pd.mcaddr.sin_port         = htons (port);

	return bind (self->pd.fd, (struct sockaddr *) &self->pd.addr,
						sizeof self->pd.addr);
}

static int mclib_receive (struct mcast_lib_t *self, void *buf, size_t len) {
	int	rmaddrlen = sizeof (self->pd.rmaddr);

	return recvfrom (self->pd.fd, buf, len, self->pd.recvflag,
			(struct sockaddr *) &self->pd.rmaddr, &rmaddrlen);
}

static int mclib_recvfrom (struct mcast_lib_t *self,
				void *buf, size_t len, struct sockaddr *from,
				socklen_t *fromlen) {
	return recvfrom (self->pd.fd, buf, len, self->pd.recvflag,
						from, fromlen);
}

static int mclib_recv (struct mcast_lib_t *self,
					void *buf, size_t len, const int sec) {
	fd_set		rfds;
	struct timeval	tv;
	struct timeval	*ptr = NULL;
	int		rc;

	if (sec > 0) {
		tv.tv_sec  = sec;
		tv.tv_usec = 0;
		ptr        = &tv;
	}

	FD_ZERO (&rfds);
	FD_SET  (self->pd.fd, &rfds);

	if ((rc = select (self->pd.fd + 1, &rfds, NULL, NULL, ptr)) <= 0) {
		return rc;
	}

	if (FD_ISSET (self->pd.fd, &rfds)) {
		return self->receive (self, buf, len);
	}

	return -2;
}

static int mclib_recvms (struct mcast_lib_t *self,
					void *buf, size_t len, const int msec) {
	fd_set		rfds;
	struct timeval	tv;
	struct timeval	*ptr = NULL;
	int		rc;

	if (msec > 0) {
		tv.tv_sec  = msec / 1000;
		tv.tv_usec = (msec % 1000) * 1000;
		ptr        = &tv;
	}

	FD_ZERO (&rfds);
	FD_SET  (self->pd.fd, &rfds);

	if ((rc = select (self->pd.fd + 1, &rfds, NULL, NULL, ptr)) <= 0) {
		return rc;
	}

	if (FD_ISSET (self->pd.fd, &rfds)) {
		return self->receive (self, buf, len);
	}

	return -2;
}

static int mclib_send (struct mcast_lib_t *self, void *buf, size_t len) {
	return sendto (self->pd.fd, buf, len, self->pd.sendflag,
			(struct sockaddr *) &self->pd.mcaddr,
			sizeof self->pd.mcaddr);
}

static int mclib_sendback (struct mcast_lib_t *self, void *buf, size_t len) {
	return sendto (self->pd.fd, buf, len, self->pd.sendflag,
			(struct sockaddr *) &self->pd.rmaddr,
			sizeof self->pd.rmaddr);
}

static int mclib_sendto (struct mcast_lib_t *self, const int n,
						void *buf, size_t len) {
	if ((n >= 0) && (n < self->pd.listcnt)) {
		if (self->pd.addruse[n] == 0) return -1;
	} else {
		return -1;
	}

	/*
	 *	fprintf (stderr, "sendto %s:%d\n",
	 *			inet_ntoa (self->addrlist[n].sin_addr),
	 *			ntohs (self->addrlist[n].sin_port));
	 */

	return sendto (self->pd.fd, buf, len, self->pd.sendflag,
			(struct sockaddr *) &self->pd.addrlist[n],
			sizeof self->pd.addrlist[n]);
}

static int mclib_bsend (struct mcast_lib_t *self, const int port, void *buf,
						size_t len) {
	self->pd.bcaddr.sin_port = htons (port);

	return sendto (self->pd.fd, buf, len, self->pd.sendflag,
			(struct sockaddr *) &self->pd.bcaddr,
			sizeof self->pd.bcaddr);
}

static int mclib_nonblocking (struct mcast_lib_t *self, const int n) {
	if (n == 0) {
		if (self->pd.nonblocking_flag == 1) {
			fcntl (self->pd.fd, F_SETFL, self->pd.ioflag);
			self->pd.nonblocking_flag = 0;
		}
	} else if (n == 1) {
		if (self->pd.nonblocking_flag == 0) {
			fcntl (self->pd.fd, F_SETFL,
					self->pd.ioflag | O_NDELAY);
			// fcntl (sockfd, F_SETFL, FNDELAY);    // BSD
			// fcntl (sockfd, F_SETFL, O_NDELAY);   // System V
			self->pd.nonblocking_flag = 1;
		}
	}

	return self->pd.nonblocking_flag;
}

static int mclib_open (struct mcast_lib_t *self,
					const char *group,
					const char *interface,
					const int is_loopback) {
	struct hostent		*hp;
	int			retval = -1;
	unsigned char		loop = is_loopback;  // 0 to disable loopback
	int			loop_size;
	struct ip_mreq		mreq;
	unsigned char		ttl = DEFAULT_TTL; 

	do {
		if ((hp = gethostbyname (group)) == NULL) {
			break;
		} else {
			struct in_addr	*ip;

			ip = (struct in_addr *) hp->h_addr;
			self->pd.mcast_group = ip->s_addr;
		}

		if ((self->pd.fd = socket (AF_INET,
						SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror ("[socket]");
			break;
		}

		bzero ((char *) &self->pd.bcaddr, sizeof (self->pd.bcaddr));
		self->pd.bcaddr.sin_family	= AF_INET;
		self->pd.bcaddr.sin_addr.s_addr	= htonl (INADDR_BROADCAST);

		if (interface != NULL) {
			struct ifreq        if_data;
			struct sockaddr_in  *addr;
			struct in_addr      inaddr;

			strncpy (if_data.ifr_name, interface, IFNAMSIZ);

			if (ioctl (self->pd.fd, SIOCGIFBRDADDR, &if_data) < 0) {
				perror (interface);
			} else {
				addr = (struct sockaddr_in *)
						&if_data.ifr_ifru.ifru_addr;

				// fprintf (stderr, "Broadcast Address: %s\n",
				//		inet_ntoa(addr->sin_addr));

				self->pd.bcaddr.sin_addr = addr->sin_addr;
			}

			if (ioctl (self->pd.fd, SIOCGIFADDR, &if_data) < 0) {
				perror (interface);
				break;
			}
			addr = (struct sockaddr_in *)
						&if_data.ifr_ifru.ifru_addr;

			// printf ("Using %s\n", inet_ntoa(addr->sin_addr));
			self->pd.our_addr = addr->sin_addr.s_addr;
			inaddr = addr->sin_addr;

			setsockopt (self->pd.fd, IPPROTO_IP, IP_MULTICAST_IF,
					&inaddr, sizeof(struct in_addr));
		} else {
			self->pd.our_addr =
				get_local_address (self->pd.mcast_group);
		}

		if (group != NULL) {
			mreq.imr_multiaddr.s_addr = self->pd.mcast_group;
			mreq.imr_interface.s_addr = self->pd.our_addr;

			setsockopt (self->pd.fd, IPPROTO_IP,
				IP_ADD_MEMBERSHIP, &mreq, sizeof mreq);
			setsockopt (self->pd.fd, IPPROTO_IP,
				IP_MULTICAST_LOOP, &loop, sizeof loop);
			setsockopt (self->pd.fd, IPPROTO_IP,
				IP_MULTICAST_TTL,  &ttl,  sizeof ttl);

			loop_size = sizeof loop;
			getsockopt (self->pd.fd, IPPROTO_IP,
				IP_MULTICAST_LOOP, &loop, &loop_size);

			if (loop) {
				fprintf (stderr, "(Enable loopback)\n");
			}
		} else {
			int	flag = 1;

			if (setsockopt (self->pd.fd, SOL_SOCKET, SO_REUSEADDR,
					(char *) &flag, sizeof flag) < 0) {
				fprintf (stderr,
					"Can't set SO_REUSEADDR option\n");
			}

			if (setsockopt (self->pd.fd, SOL_SOCKET, SO_BROADCAST,
					(char *) &flag, sizeof flag) < 0) {
				fprintf (stderr,
					"Can't set SO_BROADCAST option\n");
			}
		}

		self->pd.ioflag = fcntl (self->pd.fd, F_GETFL);

		retval = self->pd.fd;
	} while (retval < 0);

	return retval;
}

static char * mclib_remote_addr (struct mcast_lib_t *self) {
	return inet_ntoa (self->pd.rmaddr.sin_addr);
} 

static int mclib_remote_port (struct mcast_lib_t *self) {
	return ntohs (self->pd.rmaddr.sin_port);
}

static int internal_find_free (struct mcast_lib_t *self) {
	int	i;

	for (i = 0; i < self->pd.listcnt; i++) {
		if (self->pd.addruse[i] == 0) {
			self->pd.addruse[i] = 1;
			return i;
		}
	}

	return -1;
}

static int mclib_num_of_registed (struct mcast_lib_t *self) {
	int	i, rc;

	for (i = rc = 0; i < self->pd.listcnt; i++) {
		if (self->pd.addruse[i] != 0) rc++;
	}

	return rc;
}

static int mclib_regist_addr (struct mcast_lib_t *self,
					const char *ip, const int port) {
	int		i, len;
	struct hostent	*hp;

	if ((i= internal_find_free (self)) < 0) return -1;

	bzero ((char *) &self->pd.addrlist[i], sizeof (struct sockaddr_in));

	if (inet_aton (ip, &self->pd.addrlist[i].sin_addr) != 0) {
		self->pd.addrlist[i].sin_family = AF_INET;
	} else if ((hp = gethostbyname (ip)) != NULL) {
		self->pd.addrlist[i].sin_family = hp->h_addrtype;
		len = hp->h_length;

		if (len > sizeof self->pd.addrlist[i].sin_addr) {
			len = sizeof self->pd.addrlist[i].sin_addr;
		}

		memcpy (&self->pd.addrlist[i].sin_addr,
					hp->h_addr_list[0], len);
	} else {
		self->pd.addruse[i] = 0;
		perror (ip);
		return -1;
	}

	self->pd.addrlist[i].sin_port	= htons (port);

	return i;
}

static int mclib_regist_rmaddr (struct mcast_lib_t *self) {
	struct mcast_lib_pd_t	*pd = &self->pd;
	int			i, f = -1;

	for (i = 0; i < pd->listcnt; i++) {
		if (pd->addruse[i] == 0) {
			if (f < 0) f = i;
		} else if (memcmp (&pd->addrlist[i], &pd->rmaddr,
					sizeof (struct sockaddr_in)) == 0) {
			// self->addruse[i] = 1;
			f = i;
			break;
		}
	}

	if (f >= 0) {
		if (pd->addruse[f] == 0) {
			memcpy (&pd->addrlist[f], &pd->rmaddr,
						sizeof (struct sockaddr_in));

			pd->addruse[f] = 1;
		}
	}

	return f;
}

static int mclib_set_sendflag (struct mcast_lib_t *self, const int f) {
	int	rc = self->pd.sendflag;

	self->pd.sendflag = f;
	return rc;
}

static int mclib_set_recvflag (struct mcast_lib_t *self, const int f) {
	int	rc = self->pd.recvflag;

	self->pd.recvflag = f;
	return rc;
}

static void mclib_unregist (struct mcast_lib_t *self, const int entry) {
	if ((entry >= 0) && (entry < self->pd.listcnt)) {
		self->pd.addruse[entry] = 0;
	}
}

static void mclib_dispose (struct mcast_lib_t *self) {
	self->close (self);
	free (self);
}

struct mcast_lib_t * new_multicast (const int n) {
	struct mcast_lib_t	*mcptr;
	struct mcast_lib_pd_t	*pd;
	int			i;

	if ((mcptr = malloc (sizeof (struct mcast_lib_t))) == NULL) {
		return NULL;
	}

	pd = &mcptr->pd;

	if (n > 0) {
		pd->addrlist = calloc (n, sizeof (struct sockaddr_in));
		pd->addruse  = calloc (n, sizeof (short));
		pd->listcnt  = n;

		for (i = 0; i < n; i++) pd->addruse[i] = 0;
	} else {
		pd->listcnt  = 0;
		pd->addrlist = NULL;
		pd->addruse = NULL;
	}

	pd->fd			= -1;
	pd->nonblocking_flag	= 0;
	pd->ioflag		= 0;
	pd->sendflag		= 0;
	pd->recvflag		= 0;

	mcptr->open		= mclib_open;
	mcptr->close		= mclib_close;
	mcptr->bind		= mclib_bind;
	mcptr->receive		= mclib_receive;
	mcptr->recvfrom		= mclib_recvfrom;
	mcptr->recv		= mclib_recv;
	mcptr->recvms		= mclib_recvms;
	mcptr->send		= mclib_send;
	mcptr->bsend		= mclib_bsend;
	mcptr->sendback		= mclib_sendback;
	mcptr->sendto		= mclib_sendto;
	mcptr->remote_addr	= mclib_remote_addr;
	mcptr->remote_port	= mclib_remote_port;
	mcptr->regist_rmaddr	= mclib_regist_rmaddr;
	mcptr->regist_addr	= mclib_regist_addr;
	mcptr->dispose		= mclib_dispose;
	mcptr->nonblocking	= mclib_nonblocking;
	mcptr->set_sendflag	= mclib_set_sendflag;
	mcptr->set_recvflag	= mclib_set_recvflag;
	mcptr->unregist		= mclib_unregist;
	mcptr->num_of_registed	= mclib_num_of_registed;

	return mcptr;
}
