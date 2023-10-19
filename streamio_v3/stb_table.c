/*
 *	stb_table.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
// #include "vodsvr.h"
// #include "vodstb.h"
#include "streamio_v3.h"
#include "streamio_v3_pdu.h"
#include "stb_table.h"
#include "stb_entry.h"
#include "pqueue.h"
#include "mqueue.h"
#include "udplib.h"

#define USE_PRIORITY_QUEUE	0


static pthread_mutex_t		stb_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned int		stb_regist_cnt = 0;
static unsigned int		stb_opend_file = 0;
static unsigned int		stb_opend_file_all = 0;
static int			best_diskio_buffer_size = 0;
static int			netio_block_per_diskio = 0;
static short			stop_services;
static int			streamming_max_packet = 12;

static struct streamming_io_v3_server_t	*stio = NULL;


static struct stbtable_t	stbtable;
static struct stb_entry_t	*stbent  = NULL;
static int			count = 0;
static int			inuse = 0;
// static int			*diskio_queue = NULL;
static int			*netio_queue  = NULL;
// static pthread_mutex_t	diskio_mutex  = PTHREAD_MUTEX_INITIALIZER;
// static pthread_mutex_t	netio_mutex   = PTHREAD_MUTEX_INITIALIZER;

// static struct priority_queue_t	*diskq = NULL;

#if USE_PRIORITY_QUEUE == 1
static struct priority_queue_t	*netq[MAX_STIOV3_CHANNEL];
#else
static struct multiple_queue_t	*netq[MAX_STIOV3_CHANNEL];
#endif

#if USE_PRIORITY_QUEUE == 1
static int qos_compare (const int *i, const int *j) {
	if (stbent[*i].qos > stbent[*j].qos) {
		return -1;
	} else if (stbent[*i].qos < stbent[*j].qos) {
		return 1;
	}
	return 0;
}
#endif

static int stbtbl_regist (const in_addr_t addr, const int u) {
	int	i, j, f;

	for (i = 0, f = -1; i < count; i++) {
		if (stbent[i].inuse == 0) {
			if (f < 0) f = i;
		} else if (stbent[i].addr == addr) {
			fprintf (stderr,
				"STB[%d]: Already regist: (%d) "
				"[%d STB registed]\n",
				i, u, stb_regist_cnt);
			// 註解掉下列數行, 意即重新註冊時
			// 並不重新開新檔 ...
			// 請注意, 這是有風險的 ... 但發生的機率很低
			//
			// if (stbent[i].fd >= 0) {
			//	fprintf (stderr, "File closed on regist\n");
			//	close (stbent[i].fd);
			// }
			// stbent[i].fd        = -1;
			// stbent[i].filesize  = 0;
			// stbent[i].filecount = 0;
			stbent[i].udp       = u;
			return i;
		}
	}

	if (f >= 0) {
		pthread_mutex_lock (&stb_mutex);
		stb_regist_cnt++;
		pthread_mutex_unlock (&stb_mutex);

		fprintf (stderr, "STB[%d]: Regist New Entry: (%d) "
				"[%d STB registed]\n",
				f, u, stb_regist_cnt);

		stbent[f].inuse     = 1;
		stbent[f].addr      = addr;
		stbent[f].lastuse   = time (NULL);

		for (j = 0; j < MAX_STIOV3_CHANNEL; j++) {
			stbent[f].fd[j]        = -1;
			stbent[f].filesize[j]  = 0;
			stbent[f].filecount[j] = 0;
		}
		stbent[f].udp       = u;
	}

	return f;
}

static off_t stbtbl_setfile (const int n, const int channel,
				const char *file,
				const int cnt, const int stbid,
				char **realfile, int *err) {
	char		filename[256];
	struct stat	stbuf;

#if DEBUG_STIOV3 > 3
	fprintf (stderr, "Set file (%d, channel %d, %s)\n", n, channel, file);
#endif

	if ((n >= 0) && (n < count)) {
		if (stbent[n].stbid[channel] == stbid) {
			if (stbent[n].filecount[channel] == cnt) {
				fprintf (stderr,
					"STB[%d/%d]: open on same file count\n",
					n, channel);
				*err = stbent[n].err[channel];
				return stbent[n].filesize[channel];
			} else if (stbent[n].filecount[channel] > cnt) {
				fprintf (stderr, "STB[%d/%d] Old packet !!"
						" (%d > %d)\n",
						n, channel,
						stbent[n].filecount[channel],
						cnt);
				return -1;
			}
		} else {
			fprintf (stderr, "STB[%d/%d]: stbid=%d\n",
						n, channel, stbid);
			stbent[n].stbid[channel] = stbid;
		}

		pthread_mutex_lock (&stbent[n].mutex[channel]);

		if (stbent[n].fd[channel] >= 0) {

			if (stbent[n].filestatus[channel] != 2) {
				fprintf (stderr,
					"STB[%d/%d]: File closed on new file\n",
					n, channel);
				pthread_mutex_lock (&stb_mutex);
				stb_opend_file--;
				pthread_mutex_unlock (&stb_mutex);
			}

			close (stbent[n].fd[channel]);
			stbent[n].fd[channel] = -1;
		}

		stbent[n].filecount[channel] = cnt;

		/*
		sprintf (filename, "%s/%c%c/%c%c/%s",
					MPEG_BASEDIR,
					file[0], file[1],
					file[2], file[3],
					file);
					*/
#if DEBUG_STIOV3 > 3
		// fprintf (stderr, "Mutex 2 %p\n", stio);
#endif
		sprintf (filename, "%s",
				stio->pd.file_pathname (channel, file));

		*realfile = stio->pd.real_filename (channel);

		/*
		if (*realfile != NULL) {
			fprintf (stderr, "Realfile=[%s]\n", *realfile);
		} else {
			fprintf (stderr, "Realfile is same as origional\n");
		}
		*/

		fprintf (stderr, "STB[%d/%d]: File=[%s] ",
					n, channel, filename);

		if (stat (filename, &stbuf) == -1) {
			perror (filename);
			stbent[n].filesize[channel] = -1;
		} else if ((stbent[n].fd[channel] =
					open (filename, O_RDONLY)) < 0) {
			perror (NULL);
			stbent[n].filesize[channel] = -1;
		} else {
			stbent[n].filestatus[channel] = 1;

			pthread_mutex_lock (&stb_mutex);
			stb_opend_file++;
			stb_opend_file_all++;
			pthread_mutex_unlock (&stb_mutex);

			fprintf (stderr, "ok [%d/%d]\n",
						stb_opend_file,
						stb_opend_file_all);

			stbent[n].filesize[channel] = stbuf.st_size;
			stbent[n].blockcnt[channel] =
					stbent[n].filesize[channel] / 32768;

			if (stbent[n].filesize[channel] % 32768 != 0) {
				stbent[n].blockcnt[channel]++;
			}
		}

		*err = stbent[n].err[channel] = errno;

		stbent[n].current_block[channel] = -1;
		stbent[n].cdkio_block[channel]   = -1;

		pthread_mutex_unlock (&stbent[n].mutex[channel]);

		return stbent[n].filesize[channel];
	} else {
		return 0;
	}

	fprintf (stderr, "Out of range!!\n");
	return -1;
}

static int stbtbl_streamming_lock (const int n, const int channel) {
	if ((n >= 0) && (n < count)) {
		pthread_mutex_lock (&stbent[n].mutex[channel]);
		// stbent[n].streamio_lock = 1;
		return 1;
	}
	return 0;
}

static int stbtbl_streamming_unlock (const int n, const int channel) {
	if ((n >= 0) && (n < count)) {
		pthread_mutex_unlock (&stbent[n].mutex[channel]);
		// stbent[n].streamio_lock = 0;
		return 1;
	}
	return 0;
}

static int stbtbl_is_file_matched (const int n,
					const int channel, const int fc) {
	if ((n >= 0) && (n < count)) {
		if (stbent[n].filecount[channel] == fc) return 1;

		fprintf (stderr, "STB[%d/%d]: File mis-matched (%d v.s %d)\n",
				n, channel, stbent[n].filecount[channel], fc);
	}
	return 0;
}

static int stbtbl_fill (const int n, const int channel, const int mapflag,
						char *buffer, int *bid) {
	int			i, k, rc = -1;
	char			*bptr;
	struct netio_buffer_t	*ptr;

	if ((n >= 0) && (n < count)) {
		/*
		fprintf (stderr, "Request Buffer = (%d,%d)\n",
				stbent[n].netio_block,
				stbent[n].rdkio_block);
				*/

		bptr = (void *) stbent[n].buffer[channel];
		bptr = bptr + (32768 * stbent[n].netio_block[channel]);
		ptr = (void *) bptr;

		// pthread_mutex_lock (&stbent[n].mutex);

		i = (mapflag == 1) ? -1 : stbent[n].mapi[channel];

		for (++i; i < 23; i++) {
			if (stbent[n].map[channel][i] != 0) {
				k = stbent[n].buflen[channel] / 1425;
				if (i < k) {
					rc = 1425;
				} else {
					rc = stbent[n].buflen[channel]  % 1425;
				}


				// stbent[n].mapi = i;
				if (bid != NULL) *bid = i;

				if (rc == 0) {
					rc = -1;
				} else {
					memcpy (buffer, ptr->b[i], rc);
				}
				break;
			}
		}
		stbent[n].mapi[channel] = i;

		// pthread_mutex_unlock (&stbent[n].mutex);
		return rc;
	}

	return -1;
}

static int stbtbl_read (const int n, const int channel,
					struct stiov3_fetch_data_t *fptr) {
	char		*ptr;
	int		len, rlen;

	if ((n >= 0) && (n < count)) {
		// pthread_mutex_lock (&stbent[n].mutex);
		// fprintf (stderr, "read from fd=%d\n", stbent[n].fd);

		// stbent[n].request_block = block;
		// stbent[n].rdkio_block   = block % netio_block_per_diskio;

		if (stbent[n].rdkio_block[channel]
					!= stbent[n].cdkio_block[channel]) {
			if (stbent[n].rdkio_block[channel] !=
					stbent[n].cdkio_block[channel] + 1) {
				rlen = stbent[n].rdkio_block[channel] * 
						best_diskio_buffer_size;
				fprintf (stderr,
					"STB[%d/%d]: "
					"lseek (req:%d v.s. cur:%d) %d\n",
						n, channel,
						stbent[n].rdkio_block[channel],
						stbent[n].cdkio_block[channel],
						rlen);
				lseek (stbent[n].fd[channel], rlen, SEEK_SET);
				// lseek ();
			// } else {
				/*
				fprintf (stderr, "Normal read %d/%d\n",
						stbent[n].rdkio_block,
						stbent[n].cdkio_block);
				*/
			}

			stbent[n].cdkio_block[channel] =
					stbent[n].rdkio_block[channel];
			// stbent[n].current_block = stbent[n].request_block;

			ptr  = stbent[n].buffer[channel];
			rlen = 0;

			while ((len = read (stbent[n].fd[channel], &ptr[rlen],
					best_diskio_buffer_size - rlen)) > 0) {
				if ((rlen = rlen + len) ==
						best_diskio_buffer_size) break;
				/*
				fprintf (stderr,
				 	"WARNING: File not read in once !!\n");
				*/
			}

			if (len < 0) {
				perror ("read");
			} else if (len == 0) {
				fprintf (stderr, "STB[%d/%d]: File closed\n",
						n, channel);

				stbent[n].filestatus[channel] = 2;
				// close (stbent[n].fd);
				// stbent[n].fd = -1;

				pthread_mutex_lock (&stb_mutex);
				stb_opend_file--;
				pthread_mutex_unlock (&stb_mutex);
			}

			stbent[n].buflen[channel] = rlen;

			/*
			fprintf (stderr, "STB[%d]: "
				"Reading %u/%u QOS=%d (fs=%ld, len=%d)\n",
					n,
					stbent[n].request_block,
					stbent[n].blockcnt,
					stbent[n].qos,
					stbent[n].filesize,
					rlen);
			*/
			// read ();
		// } else {
			// Already in buffer
			// fprintf (stderr, "Already in buffer\n");
		}

		fptr->h.sn	= stbent[n].stb_sn;
		fptr->h.filecnt = stbent[n].filecount[channel];
		// fptr->h.block   = stbent[n].cdkio_block;
		fptr->h.block   = stbent[n].request_block[channel];

		// pthread_mutex_unlock (&stbent[n].mutex);

		return 1;
	}

	return 0;
}

static int stbtbl_push (const int n, const int channel, const int cmd) {
	// STBVOD_PDU_CMD_FAST_FETCH
	// STBVOD_PDU_CMD_FETCH

	if ((n >= 0) && (n < count)) {
#if USE_PRIORITY_QUEUE == 1
		netq[channel]->enqueue (netq[channel], n);
#else
		netq[channel]->enqueue (netq[channel], n,
				cmd == STIOV3_PDU_CMD_FAST_FETCH ? 0 : 1);
#endif
		return 1;
	}
	return 0;
}

static int stbtbl_pop (const int channel) {
	return netq[channel]->dequeue (netq[channel], 1);
}

static int stbtbl_init (const int n, const int channel, const int sn) {
	if ((n >= 0) && (n < count)) {
		pthread_mutex_lock (&stbent[n].mutex[channel]);

		stbent[n].stb_sn = sn;

		if (stbent[n].buffer[channel] == NULL) {
			if ((stbent[n].buffer[channel] = malloc (
				best_diskio_buffer_size)) == NULL) {
				perror ("malloc");
			}
		}

		if (stbent[n].buffer[channel] == NULL) {
			fprintf (stderr, "Memory allocation error!\n");
			return 0;
		}

		pthread_mutex_unlock (&stbent[n].mutex[channel]);
		return 1;
	}

	return 0;
}

static int stbtbl_start (const int n, const int sn) {
	/*
	if ((n >= 0) && (n < count)) {
		if (! stbent[n].running) stbtbl_init (n, sn);

		// while (stbent[n].ready == 0) usleep (1000);

		// if (stbent[n].ready == 1) {
		//	pthread_mutex_lock   (&stbent[n].mutex);
		//	pthread_cond_signal  (&stbent[n].condi);
		//	pthread_mutex_unlock (&stbent[n].mutex);
		// }

		// 把 stb_main 叫起來做事
	} else {
		return 0;
	}

	return 1;
	*/
	return 1;
}

static int stbtbl_stop (const int n, const int channel) {
	if ((n >= 0) && (n < count)) {
		/*
		if (stbent[n].running) {
			pthread_kill (stbent[n].thread, SIGTERM);
			stbent[n].running = 0;
		}
		*/
		if (stbent[n].fd[channel] >= 0) {
			fprintf (stderr,
				"STB[%d/%d]: File closed on stop !!\n",
				n, channel);

			close (stbent[n].fd[channel]);
			stbent[n].fd[channel] = -1;

			if (stbent[n].filestatus[channel] != 2) {
				pthread_mutex_lock (&stb_mutex);
				stb_opend_file--;
				pthread_mutex_unlock (&stb_mutex);
			}

			stbent[n].filestatus[channel] = 0;
		}
	} else {
		return 0;
	}

	return 1;
}

static int stbtbl_setrequest (const int n, const int channel, const int block,
					const char *map, const int qos) {
	int	j;

	if ((n >= 0) && (n < count)) {
		pthread_mutex_lock (&stbent[n].mutex[channel]);

		// i = (stbent[n].mapidx + 1) % 2;

		for (j = 0; j < 23; j++) {
			stbent[n].map[channel][j] = map[j];
		}

		// stbent[n].mapidx = i;
		stbent[n].qos[channel]		= qos;
		stbent[n].request_block[channel] = block;
		stbent[n].rdkio_block[channel] = block / netio_block_per_diskio;
		stbent[n].netio_block[channel] = block % netio_block_per_diskio;

		/*
		fprintf (stderr, "Request block=%d/%d/%d\n", block,
				stbent[n].rdkio_block[channel],
				stbent[n].netio_block[channel]);
		*/

		pthread_mutex_unlock (&stbent[n].mutex[channel]);
		return 1;
	}

	return 0;
}

static int stbtbl_udp (const int n) {
	if ((n >= 0) && (n < count)) return stbent[n].udp;
	return -1;
}

static int stbtbl_close_all (void) {
	int	i, j;
	int	svc_flag = stop_services;

	stop_services = 1;	// make sure services is stopped !!
	sleep (1);

	for (j = 0; j < MAX_STIOV3_CHANNEL; j++) {
		while (! netq[j]->is_empty (netq[j])) {
			fprintf (stderr, "Wait for queue is empty !\n");
			sleep (1);
		}
	}

	for (i = 0; i < count; i++) {
		for (j = 0; j < MAX_STIOV3_CHANNEL; j++) {
			pthread_mutex_lock   (&stbent[i].mutex[j]);

			if (stbent[i].fd[j] >= 0) {
				close (stbent[i].fd[j]);
				stbent[i].fd[j] = -1;
				stbent[i].filecount[j] = 0;

				if (stbent[i].filestatus[j] != 2) {
					stb_opend_file--;
				}

				stbent[i].filestatus[j] = 0;
			}

			pthread_mutex_unlock (&stbent[i].mutex[j]);
		}
	}

	stop_services = svc_flag;

	return 1;
}

struct stbtable_t * new_stbtable_v3 (struct streamming_io_v3_server_t *stiov3,
					const int n, const int enable_qos) {
	int		i, j;

	best_diskio_buffer_size = 1024 * 1024;
	netio_block_per_diskio = best_diskio_buffer_size / 32768;
	stb_regist_cnt = 0;
	stb_opend_file = 0;
	stb_opend_file_all = 0;

	if (count == 0) {
		stbent = calloc (n, sizeof (struct stb_entry_t));

		stio			= stiov3;

		count = n;
		inuse = 0;
		stbtable.regist		= stbtbl_regist;
		stbtable.setfile	= stbtbl_setfile;
		stbtable.setrequest	= stbtbl_setrequest;
		stbtable.push		= stbtbl_push;
		stbtable.pop		= stbtbl_pop;
		stbtable.init		= stbtbl_init;
		stbtable.start		= stbtbl_start;
		stbtable.stop		= stbtbl_stop;
		stbtable.read		= stbtbl_read;
		stbtable.fill		= stbtbl_fill;
		stbtable.udp		= stbtbl_udp;
		stbtable.close_all	= stbtbl_close_all;

		stbtable.is_file_matched   = stbtbl_is_file_matched;
		stbtable.streamming_lock   = stbtbl_streamming_lock;
		stbtable.streamming_unlock = stbtbl_streamming_unlock;

		// diskio_queue = calloc (n, sizeof (int));
		netio_queue  = calloc (n, sizeof (int));

		// diskq = new_priority_queue (n, NULL);

		for (i = 0; i < MAX_STIOV3_CHANNEL; i++) {
#if USE_PRIORITY_QUEUE == 1
			if (enable_qos) {
				netq[i]  = new_priority_queue (n, qos_compare);
			} else {
				netq[i]  = new_priority_queue (n, NULL);
			}
#else
			netq[i] = new_multiple_queue (n, 2, 256);
#endif
		}

		// vodsvr.add		= vodsvr_add;
		// vodsvr.regist_code	= vodsvr_regist_code;
		for (i = 0; i < n; i++) {
			stbent[i].n		= i;
			stbent[i].inuse		= 0;
			// stbent[i].running	= 0;
			stbent[i].ready		= 0;
			stbent[i].udp		= -1;

			for (j = 0; j < MAX_STIOV3_CHANNEL; j++) {
				stbent[i].stbid[j]	= -1;
				stbent[i].fd[j]		= -1;
				stbent[i].filestatus[j]	= 0;
				stbent[i].buffer[j]	= NULL;
				stbent[i].mapidx[j]	= 0;

				pthread_cond_init  (&stbent[i].condi[j], NULL);
				pthread_mutex_init (&stbent[i].mutex[j], NULL);
			}

			/*
			if (pthread_create (&stbent[i].thread, NULL,
					(void *) stb_main, &stbent[i]) == 0) {
				stbent[i].running = 1;
			} else {
				perror ("pthread_create");
			}
			*/
		}
	}

	return &stbtable;
}


////////////////////////////////////////////////////////////////////////


/*
 *	streamio.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef SLOW_DOWN_ON_TRANSMIT
#define SLOW_DOWN_ON_TRANSMIT	1
#endif

// extern struct udplib_t		*stbudp;

void * streamio_v3_service (void *arg) {
	struct stio_v3_svc_param_t	*parm = arg;

	struct streamming_io_v3_server_t	*self = parm->stiov3svr;
	int					channel = parm->channel;
	struct udplib_t			*udp = self->pd.udp;
	struct stbtable_t		*stb = self->pd.stb;
	int				n, i, len, hlen;
#if SLOW_DOWN_ON_TRANSMIT == 1
	int				cnt;
	// struct timespec		req, rem;
	// struct timeval		now;
#endif
	char				buffer[2000];
	struct stio_v3_pdu_t		*pdu = (void*) buffer;
	// struct stb_vod_fetch_t	*fptr = &ptr->fetch;
	struct stiov3_fetch_data_t	*ptr = (void *) &pdu->fetch;


	pdu->cmd = STIOV3_PDU_CMD_FETCH;

	hlen = ptr->buffer - buffer;

	// fprintf (stderr, "Header Length = %d\n", hlen);

#if DEBUG_STIOV3 > 3
	fprintf (stderr, "Service for channel %d\n", channel);
#endif
	ptr->h.channel = channel;

	while ((n = stb->pop (channel)) >= 0) {
		/*
		fprintf (stderr, "Thread(%ld): wakeup, client id=%d, udp=%d\n",
						pthread_self (), n,
						stb->udp (n));
		fprintf (stderr ,"IP=%s, port=%d\n",
				stbudp->ip_addr (stbudp, stb->udp (n)),
				stbudp->udp_port (stbudp, stb->udp (n)));
		*/
		
		stb->streamming_lock (n, channel);
		// check if disk fetching is required !!
		stb->read (n, channel, ptr);

		if ((len = stb->fill (n, channel, 1, ptr->buffer, &i)) > 0) {
#if SLOW_DOWN_ON_TRANSMIT == 1
			cnt = 0;
#endif

			do {
				ptr->len = len;

				len += hlen;
				ptr->bid = i;
				pdu->cksum = self->cksum (buffer, len);
				udp->sendto (udp, stb->udp (n), buffer, len);

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
			} while ((len = stb->fill (n, channel,
						0, ptr->buffer, &i)) > 0);
		}

		stb->streamming_unlock (n, channel);
	}

	// fprintf (stderr, "Streamming IO: terminated\n");
	pthread_exit (NULL);
}
