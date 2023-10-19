/*
 *	streamio.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "stb_table.h"
#include "vodstb.h"
#include "vodsvr.h"
#include "udplib.h"

#ifndef SLOW_DOWN_ON_TRANSMIT
#define SLOW_DOWN_ON_TRANSMIT	1
#endif

extern struct udplib_t		*stbudp;

void * streamio_main (void *arg) {
	int				n, i, len, hlen;
#if SLOW_DOWN_ON_TRANSMIT == 1
	int				cnt;
	// struct timespec		req, rem;
	// struct timeval		now;
#endif
	char				buffer[2000];
	struct stb_vod_pdu_t		*pdu = (void*) buffer;
	// struct stb_vod_fetch_t	*fptr = &ptr->fetch;
	struct stb_vod_fetch_data_t	*ptr = (void *) &pdu->fetch;


	pdu->cmd = STBVOD_PDU_CMD_FETCH;

	hlen = ptr->buffer - buffer;

	// fprintf (stderr, "Header Length = %d\n", hlen);

	while ((n = stb->pop ()) >= 0) {
		/*
		fprintf (stderr, "Thread(%ld): wakeup, client id=%d, udp=%d\n",
						pthread_self (), n,
						stb->udp (n));
		fprintf (stderr ,"IP=%s, port=%d\n",
				stbudp->ip_addr (stbudp, stb->udp (n)),
				stbudp->udp_port (stbudp, stb->udp (n)));
		*/
		
		stb->streamming_lock (n);
		// check if disk fetching is required !!
		stb->read (n, ptr);

		if ((len = stb->fill (n, 1, ptr->buffer, &i)) > 0) {
#if SLOW_DOWN_ON_TRANSMIT == 1
			cnt = 0;
#endif

			do {
				ptr->len = len;

				len += hlen;
				ptr->bid = i;
				pdu->cksum = vodstb->cksum (buffer, len);
				stbudp->sendto (stbudp,
						stb->udp (n), buffer, len);

#if SLOW_DOWN_ON_TRANSMIT == 1
				if (++cnt % streamming_max_packet == 0) {
					/*
					gettimeofday (&now, NULL);
					req.tv_sec  = now.tv_sec;
					req.tv_nsec = now.tv_usec * 1000 + 10;

					nanosleep (&req, &rem);
					*/
					usleep (1);
				}
#endif
			} while ((len = stb->fill (n, 0, ptr->buffer, &i)) > 0);
		}

		stb->streamming_unlock (n);
	}

	// fprintf (stderr, "Streamming IO: terminated\n");
	pthread_exit (NULL);
}
