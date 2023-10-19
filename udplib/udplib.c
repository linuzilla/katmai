/*
 *	udplib.c
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
#include "udplib.h"


static void udplib_close (struct udplib_t *self) {
	if (self->fd >= 0) close (self->fd);

	self->fd = -1;
}

static void udplib_dispose (struct udplib_t *self) {
	self->close (self);
	free (self);
}

static int udplib_recvfrom (struct udplib_t *self,
				void *buf, size_t len,
				struct sockaddr *from,
				socklen_t *fromlen) {
	return recvfrom (self->fd, buf, len, self->recvflag, from, fromlen);
}

static int udplib_receive (struct udplib_t *self,
				void *buf, size_t len) {
	int	rc, fromlen;

	fromlen = sizeof (self->rmaddr);

	rc = recvfrom (self->fd, buf, len, self->recvflag, 
			(struct sockaddr *) &self->rmaddr, &fromlen);

	return rc;
}

static int udplib_recv (struct udplib_t *self,
				void *buf, size_t len, const int sec) {
	fd_set		rfds;
	struct timeval	tv;
	struct timeval	*ptr = NULL;
	int		rc;

	if (sec > 0) {
		tv.tv_sec  = sec;
		tv.tv_usec = 0;
		ptr	   = &tv;
	}

	FD_ZERO (&rfds);
	FD_SET  (self->fd, &rfds);

	if ((rc = select (self->fd + 1, &rfds, NULL, NULL, ptr)) <= 0) {
		return rc;
	}

	if (FD_ISSET (self->fd, &rfds)) {
		return self->receive (self, buf, len);
	}

	return -2;
}

static int udplib_recvms (struct udplib_t *self,
				void *buf, size_t len, const int msec) {
	fd_set		rfds;
	struct timeval	tv;
	struct timeval	*ptr = NULL;
	int		rc;

	if (msec > 0) {
		tv.tv_sec  = msec / 1000;
		tv.tv_usec = (msec % 1000) * 1000;
		ptr	   = &tv;
	}

	FD_ZERO (&rfds);
	FD_SET  (self->fd, &rfds);

	if ((rc = select (self->fd + 1, &rfds, NULL, NULL, ptr)) <= 0) {
		return rc;
	}

	if (FD_ISSET (self->fd, &rfds)) {
		return self->receive (self, buf, len);
	}

	return -2;
}


static int udplib_send (struct udplib_t *self, void *buf,
						size_t len) {
	return sendto (self->fd, buf, len, self->sendflag,
			(struct sockaddr *) &self->rmaddr,
			sizeof self->rmaddr);
}

static int udplib_sendto (struct udplib_t *self, const int n,
				void *buf, size_t len) {
	if ((n >= 0) && (n < self->listcnt)) {
		if (self->addruse[n] == 0) return -1;
	} else {
		return -1;
	}

	/*
	fprintf (stderr, "sendto %s:%d\n",
			inet_ntoa (self->addrlist[n].sin_addr),
			ntohs (self->addrlist[n].sin_port));
	*/

	return sendto (self->fd, buf, len, self->sendflag,
			(struct sockaddr *) &self->addrlist[n],
			sizeof self->addrlist[n]);
}

static int udplib_bsend (struct udplib_t *self, const int port, void *buf,
						size_t len) {
	self->bcaddr.sin_port = htons (port);

	return sendto (self->fd, buf, len, self->sendflag,
			(struct sockaddr *) &self->bcaddr,
			sizeof self->bcaddr);
}

static int udplib_set_sendflag (struct udplib_t *self, const int f) {
	int	rc = self->sendflag;

	self->sendflag = f;
	return rc;
}

static int udplib_set_recvflag (struct udplib_t *self, const int f) {
	int	rc = self->recvflag;

	self->recvflag = f;
	return rc;
}

static int udplib_open (struct udplib_t *self, const int port,
					const char *intf, const int isbind) {
	struct ifreq		if_data;
	struct sockaddr_in	*addr = NULL;
	int			flag;
	int			setlocal = 0;
	int			length;
	struct sockaddr_in	name;
	// char			*intf = "eth0";

	if ((self->fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror ("socket");
		return -1;
	}
	flag = 1;

	if (setsockopt (self->fd, SOL_SOCKET, SO_REUSEADDR,
					(char *)&flag, sizeof flag) < 0) {
		fprintf (stderr, "Can't set SO_REUSEADDR option\n");
	}

	if (setsockopt (self->fd, SOL_SOCKET, SO_BROADCAST,
					(char *)&flag, sizeof flag) < 0) {
		fprintf (stderr, "Can't set SO_BROADCAST option\n");
	}

	bzero ((char *) &self->myaddr, sizeof self->myaddr);
	self->myaddr.sin_family		= AF_INET;
	self->myaddr.sin_addr.s_addr	= INADDR_ANY;
	self->myaddr.sin_port		= htons (port);


	if (intf != NULL) {
		strncpy (if_data.ifr_name, intf, IFNAMSIZ);

		if (ioctl (self->fd, SIOCGIFBRDADDR, &if_data) < 0) {
			perror (intf);
		} else {
			addr = (struct sockaddr_in *)
						&if_data.ifr_ifru.ifru_addr;

			// fprintf (stderr, "Broadcast Address: %s\n",
			// 			inet_ntoa(addr->sin_addr));
			self->bcaddr.sin_addr = addr->sin_addr;
		}

		if (ioctl (self->fd, SIOCGIFADDR, &if_data) < 0) {
			perror (intf);
		} else {
			addr = (struct sockaddr_in *)
						&if_data.ifr_ifru.ifru_addr;

			// fprintf (stderr, "My Address: %s\n",
			// 			inet_ntoa(addr->sin_addr));

			if (isbind) {
				self->myaddr.sin_addr = addr->sin_addr;
			} else {
				setlocal = 1;
			}
		}
	}

	if (bind (self->fd, (struct sockaddr *) &self->myaddr,
						sizeof self->myaddr) < 0) {
		perror ("bind");
		close (self->fd);
		self->fd = -1;
	}

	if (setlocal) {
		self->myaddr.sin_addr = addr->sin_addr;
	}

	length = sizeof (name);

	if (getsockname (self->fd, (struct sockaddr *) &name, &length) < 0) {
		perror("getting socket name");
	}

	self->myport = ntohs (name.sin_port);

	self->ioflag = fcntl (self->fd, F_GETFL);

	return self->fd;
}

static int udplib_num_of_registed (struct udplib_t *self) {
	int	i, rc;

	for (i = rc = 0; i < self->listcnt; i++) {
		if (self->addruse[i] != 0) rc++;
	}

	return rc;
}

static int udplib_nonblocking (struct udplib_t *self, const int n) {
	if (n == 0) {
		if (self->nonblocking_flag == 1) {
			fcntl (self->fd, F_SETFL, self->ioflag);
			self->nonblocking_flag = 0;
		}
	} else if (n == 1) {
		if (self->nonblocking_flag == 0) {
			fcntl (self->fd, F_SETFL, self->ioflag | O_NDELAY);
			// fcntl (sockfd, F_SETFL, FNDELAY);    // BSD
			// fcntl (sockfd, F_SETFL, O_NDELAY);   // System V
			self->nonblocking_flag = 1;
		}
	}

	return self->nonblocking_flag;
}

static char * udplib_remote_addr (struct udplib_t *self) {
	return inet_ntoa (self->rmaddr.sin_addr);
}

static int udplib_remote_port (struct udplib_t *self) {
	return ntohs (self->rmaddr.sin_port);
}

static in_addr_t udplib_get_remote (struct udplib_t *self) {
	return self->rmaddr.sin_addr.s_addr;
}

static in_addr_t udplib_get_myself (struct udplib_t *self) {
	return self->myaddr.sin_addr.s_addr;
}

static int udplib_is_local (struct udplib_t *self) {
	if (self->rmaddr.sin_addr.s_addr == self->myaddr.sin_addr.s_addr) {
		return 1;
	}
	return 0;
}

static int udplib_ip_cmp (struct udplib_t *self) {
	if (self->rmaddr.sin_addr.s_addr == self->myaddr.sin_addr.s_addr) {
		return 0;
	} else if (self->rmaddr.sin_addr.s_addr >
					self->myaddr.sin_addr.s_addr) {
		return -1;
	}
	return 1;
}

static char * udplib_my_ip (struct udplib_t *self) {
	return inet_ntoa (self->myaddr.sin_addr);
}

static char * udplib_ip_addr (struct udplib_t *self, const int n) {
	if ((n >= 0) && (n < self->listcnt)) {
		return inet_ntoa (self->addrlist[n].sin_addr);
	}
	return NULL;
}

static int udplib_udp_port (struct udplib_t *self, const int n) {
	if ((n >= 0) && (n < self->listcnt)) {
		return ntohs (self->addrlist[n].sin_port);
	}
	return 0;
}

static int udplib_set_port (struct udplib_t *self, const int n,
					const int port) {
	if ((n >= 0) && (n < self->listcnt)) {
		self->addrlist[n].sin_port = htons (port);
		return 1;
	}
	return 0;
}

static int internal_find_free (struct udplib_t *self) {
	int	i;

	for (i = 0; i < self->listcnt; i++) {
		if (self->addruse[i] == 0) {
			self->addruse[i] = 1;
			return i;
		}
	}

	return -1;
}

static int udplib_regist_rmaddr (struct udplib_t *self) {
	int	i, f = -1;

	for (i = 0; i < self->listcnt; i++) {
		if (self->addruse[i] == 0) {
			if (f < 0) f = i;
		} else if (memcmp (&self->addrlist[i], &self->rmaddr,
					sizeof (struct sockaddr_in)) == 0) {
			// self->addruse[i] = 1;
			f = i;
			break;
		}
	}

	if (f >= 0) {
		if (self->addruse[f] == 0) {
			memcpy (&self->addrlist[f], &self->rmaddr,
					sizeof (struct sockaddr_in));
			/*
			fprintf (stderr, "Address [%s] registed on %d\n",
				inet_ntoa (self->addrlist[f].sin_addr), f);
				*/
			self->addruse[f] = 1;
		}
	}

	return f;
}

static int udplib_regist_ipaddr (struct udplib_t *self) {
	int	i, f = -1;

	for (i = 0; i < self->listcnt; i++) {
		if (self->addruse[i] == 0) {
			if (f < 0) f = i;
		} else if (self->addrlist[i].sin_addr.s_addr ==
					self->rmaddr.sin_addr.s_addr) {
			// self->addruse[i] = 1;
			f = i;
			break;
		}
	}

	if (f >= 0) {
		if (self->addruse[f] == 0) {
			memcpy (&self->addrlist[f], &self->rmaddr,
					sizeof (struct sockaddr_in));
			/*
			fprintf (stderr, "Address [%s] registed on %d\n",
				inet_ntoa (self->addrlist[f].sin_addr), f);
				*/
			self->addruse[f] = 1;
		}
	}

	return f;
}

static int udplib_regist_addr (struct udplib_t *self,
					const char *ip, const int port) {
	int		i, len;
	struct hostent	*hp;

	if ((i= internal_find_free (self)) < 0) return -1;

	bzero ((char *) &self->addrlist[i], sizeof (struct sockaddr_in));
	//self->addrlist[i].sin_family		= AF_INET;
	//self->addrlist[i].sin_addr.s_addr	= htonl (INADDR_BROADCAST);

	if (inet_aton (ip, &self->addrlist[i].sin_addr) != 0) {
		self->addrlist[i].sin_family = AF_INET;
	} else if ((hp = gethostbyname (ip)) != NULL) {
		self->addrlist[i].sin_family = hp->h_addrtype;
		len = hp->h_length;

		if (len > sizeof self->addrlist[i].sin_addr) {
			len = sizeof self->addrlist[i].sin_addr;
		}

		memcpy (&self->addrlist[i].sin_addr,
				hp->h_addr_list[0], len);
	} else {
		self->addruse[i] = 0;
		perror (ip);
		return -1;
	}

	self->addrlist[i].sin_port		= htons (port);

	return i;
}

static int udplib_regist_sockaddr (struct udplib_t *self,
					struct sockaddr_in *addr) {
	int	i;

	if ((i = internal_find_free (self)) < 0) return -1;
	memcpy (&self->addrlist[i], addr, sizeof (struct sockaddr_in));

	return i;
}

static void udplib_unregist (struct udplib_t *self, const int entry) {
	if ((entry >= 0) && (entry < self->listcnt)) self->addruse[entry] = 0;
}

static void udplib_external_call (struct udplib_t *self, void *caller,
				void (*callback)(void *, const int)) {
	int	i;

	for (i = 0; i < self->listcnt; i++) {
		if (self->addruse[i] != 0) callback (caller, i);
	}
}

static int udplib_getfd (struct udplib_t *self) { return self->fd; }

static int udplib_local_port (struct udplib_t *self) { return self->myport; }

struct udplib_t * new_udplib (const int n) {
	int			i;
	struct udplib_t		*ptr;

	if ((ptr = malloc (sizeof (struct udplib_t))) == NULL) {
		return NULL;
	}

	if (n > 0) {
		ptr->addrlist = calloc (n, sizeof (struct sockaddr_in));
		ptr->addruse  = calloc (n, sizeof (short));
		ptr->listcnt  = n;

		for (i = 0; i < n; i++) ptr->addruse[i] = 0;
	} else {
		ptr->listcnt  = 0;
		ptr->addrlist = NULL;
	}

	bzero ((char *) &ptr->bcaddr, sizeof (ptr->bcaddr));
	ptr->bcaddr.sin_family		= AF_INET;
	ptr->bcaddr.sin_addr.s_addr	= htonl (INADDR_BROADCAST);
	// ptr->bcaddr.sin_port		= htons (STB_LISTEN_PORT);

	ptr->fd			= -1;
	ptr->nonblocking_flag	= 0;
	ptr->sendflag		= 0;
	ptr->recvflag		= 0;
	ptr->open		= udplib_open;
	ptr->close		= udplib_close;
	ptr->dispose		= udplib_dispose;
	ptr->receive		= udplib_receive;
	ptr->recv		= udplib_recv;
	ptr->recvms		= udplib_recvms;
	ptr->recvfrom		= udplib_recvfrom;
	ptr->bsend		= udplib_bsend;
	ptr->send		= udplib_send;
	ptr->sendto		= udplib_sendto;
	ptr->regist_addr	= udplib_regist_addr;
	ptr->regist_sockaddr	= udplib_regist_sockaddr;
	ptr->regist_rmaddr	= udplib_regist_rmaddr;
	ptr->regist_ipaddr	= udplib_regist_ipaddr;
	ptr->unregist		= udplib_unregist;
	ptr->remote_addr	= udplib_remote_addr;
	ptr->remote_port	= udplib_remote_port;
	ptr->get_remote		= udplib_get_remote;
	ptr->get_myself		= udplib_get_myself;
	ptr->nonblocking	= udplib_nonblocking;
	ptr->set_sendflag	= udplib_set_sendflag;
	ptr->set_recvflag	= udplib_set_recvflag;
	ptr->ip_cmp		= udplib_ip_cmp;
	ptr->ip_addr		= udplib_ip_addr;
	ptr->my_ip		= udplib_my_ip;
	ptr->udp_port		= udplib_udp_port;
	ptr->set_port		= udplib_set_port;
	ptr->is_local		= udplib_is_local;
	ptr->getfd		= udplib_getfd;
	ptr->local_port		= udplib_local_port;
	ptr->external_call	= udplib_external_call;
	ptr->num_of_registed	= udplib_num_of_registed;

	return ptr;
}
