/*
 *	vod_table.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
// #include <pthread.h>
#include "vod_table.h"
#define MAX_VODSVR	5

static struct vodsvr_t		vodsvr;
static struct vodsvr_table_t	*vodtab = NULL;
static int			count = 0;
static int			inuse = 0;
// static pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

struct vodsvr_table_t {
	in_addr_t	addr;
	time_t		lastuse;
	short		inuse;
	int		regist;
	int		udp_entry;
};

static int vodsvr_regist_code (const int n) {
	if ((n >= 0) && (n < count)) {
		return vodtab[n].regist;
	}
	return -1;
}


static int vodsvr_udp_entry (const int n) {
	if ((n >= 0) && (n < count)) {
		return vodtab[n].udp_entry;
	}
	return -1;
}

static int vodsvr_add (in_addr_t addr) {
	int	i, f;

	// pthread_mutex_lock (&mutex);

	for (i = 0, f = -1; i < count; i++) {
		if (vodtab[i].inuse == 0) {
			if (f < 0) f = i;
		} else if (vodtab[i].addr == addr) {
			// vodtab[i].lastuse = time (NULL);

			// pthread_mutex_unlock (&mutex);
			return i;
		}
	}

	if (f >= 0) {
		// vodtab[f].lastuse   = time (NULL);
		vodtab[f].inuse     = 1;
		vodtab[f].addr      = addr;
		vodtab[f].regist    = 0;
		vodtab[f].udp_entry = -1;
		inuse++;
	}

	// pthread_mutex_unlock (&mutex);

	return f;	// table full
}

static int vodsvr_unset_registcode (const int n) {
	if ((n >= 0) && (n < count)) {
		vodtab[n].regist = 0;
		// vodtab[i].udp_entry = -1;
		return 1;
	}
	return 0;
}

static int vodsvr_set_registcode (in_addr_t addr, const int code, const int u) {
	int	i;

	if ((i = vodsvr_add (addr)) >= 0) {
		vodtab[i].regist = code;
		// if (u >= 0) vodtab[i].udp_entry = u;
		vodtab[i].udp_entry = u;
	}

	return i;
}

struct vodsvr_t * new_vodsvr (const int n) {
	int		i;

	if (count == 0) {
		vodtab = calloc (n, sizeof (struct vodsvr_table_t));
		count = n;
		inuse = 0;
		vodsvr.add		= vodsvr_add;
		vodsvr.regist_code	= vodsvr_regist_code;
		vodsvr.set_registcode	= vodsvr_set_registcode;
		vodsvr.udp_entry	= vodsvr_udp_entry;
		vodsvr.unset_registcode	= vodsvr_unset_registcode;

		// pthread_mutex_init (&vodsvr.mutex, NULL);

		for (i = 0; i < n; i++) vodtab[i].inuse = 0;
	}

	return &vodsvr;
}
