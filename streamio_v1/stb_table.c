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
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "streamio_v1_pdu.h"
#include "stb_table.h"
#include "stb_entry.h"
/*
#include "vodsvr.h"
#include "vodstb.h"
#include "pqueue.h"
#include "mqueue.h"
*/

#define USE_PRIORITY_QUEUE	0

unsigned int             stb_regist_cnt     = 0;
pthread_mutex_t          stb_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int             stb_opend_file     = 0;
unsigned int             stb_opend_file_all = 0;


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
static struct priority_queue_t	*netq  = NULL;
#else
static struct multiple_queue_t	*netq  = NULL;
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
	int	i, f;

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
		stbent[f].fd        = -1;
		stbent[f].filesize  = 0;
		stbent[f].filecount = 0;
		stbent[f].udp       = u;
	}

	return f;
}

static off_t stbtbl_setfile (const int n, const char *file,
				const int cnt, const int stbid, int *err) {
	char		filename[256];
	struct stat	stbuf;

	if ((n >= 0) && (n < count)) {
		if (stbent[n].stbid == stbid) {
			if (stbent[n].filecount == cnt) {
				fprintf (stderr,
					"STB[%d]: open on same file count\n",
					n);
				*err = stbent[n].err;
				return stbent[n].filesize;
			} else if (stbent[n].filecount > cnt) {
				fprintf (stderr, "Old packet !!\n");
				return -1;
			}
		} else {
			fprintf (stderr, "STB[%d]: stbid=%d\n", n, stbid);
			stbent[n].stbid = stbid;
		}

		pthread_mutex_lock (&stbent[n].mutex);

		if (stbent[n].fd >= 0) {

			if (stbent[n].filestatus != 2) {
				fprintf (stderr,
					"STB[%d]: File closed on new file\n",
					n);
				pthread_mutex_lock (&stb_mutex);
				stb_opend_file--;
				pthread_mutex_unlock (&stb_mutex);
			}

			close (stbent[n].fd);
			stbent[n].fd = -1;
		}

		stbent[n].filecount = cnt;

		sprintf (filename, "%s/%c%c/%c%c/%s",
					MPEG_BASEDIR,
					file[0], file[1],
					file[2], file[3],
					file);

		fprintf (stderr, "STB[%d]: File=[%s] ", n, filename);

		if (stat (filename, &stbuf) == -1) {
			perror (filename);
			stbent[n].filesize = -1;
		} else if ((stbent[n].fd = open (filename, O_RDONLY)) < 0) {
			perror (NULL);
			stbent[n].filesize = -1;
		} else {
			stbent[n].filestatus = 1;

			pthread_mutex_lock (&stb_mutex);
			stb_opend_file++;
			stb_opend_file_all++;
			pthread_mutex_unlock (&stb_mutex);

			fprintf (stderr, "ok [%d/%d]\n",
						stb_opend_file,
						stb_opend_file_all);

			stbent[n].filesize = stbuf.st_size;
			stbent[n].blockcnt = stbent[n].filesize / 32768;

			if (stbent[n].filesize % 32768 != 0) {
				stbent[n].blockcnt++;
			}
		}

		*err = stbent[n].err = errno;

		stbent[n].current_block = -1;
		stbent[n].cdkio_block   = -1;

		pthread_mutex_unlock (&stbent[n].mutex);

		return stbent[n].filesize;
	} else {
		return 0;
	}

	fprintf (stderr, "Out of range!!\n");
	return -1;
}

static int stbtbl_streamming_lock (const int n) {
	if ((n >= 0) && (n < count)) {
		pthread_mutex_lock (&stbent[n].mutex);
		// stbent[n].streamio_lock = 1;
		return 1;
	}
	return 0;
}

static int stbtbl_streamming_unlock (const int n) {
	if ((n >= 0) && (n < count)) {
		pthread_mutex_unlock (&stbent[n].mutex);
		// stbent[n].streamio_lock = 0;
		return 1;
	}
	return 0;
}

static int stbtbl_is_file_matched (const int n, const int fc) {
	if ((n >= 0) && (n < count)) {
		if (stbent[n].filecount == fc) return 1;

		fprintf (stderr, "STB[%d]: File mis-matched (%d v.s %d)\n",
				n, stbent[n].filecount, fc);
	}
	return 0;
}

static int stbtbl_fill (const int n, const int mapflag,
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

		bptr = (void *) stbent[n].buffer;
		bptr = bptr + (32768 * stbent[n].netio_block);
		ptr = (void *) bptr;

		// pthread_mutex_lock (&stbent[n].mutex);

		i = (mapflag == 1) ? -1 : stbent[n].mapi;

		for (++i; i < 23; i++) {
			if (stbent[n].map[i] != 0) {
				k = stbent[n].buflen / 1425;
				if (i < k) {
					rc = 1425;
				} else {
					rc = stbent[n].buflen % 1425;
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
		stbent[n].mapi = i;

		// pthread_mutex_unlock (&stbent[n].mutex);
		return rc;
	}

	return -1;
}

static int stbtbl_read (const int n, struct stb_vod_fetch_data_t *fptr) {
	char		*ptr;
	int		len, rlen;

	if ((n >= 0) && (n < count)) {
		// pthread_mutex_lock (&stbent[n].mutex);
		// fprintf (stderr, "read from fd=%d\n", stbent[n].fd);

		// stbent[n].request_block = block;
		// stbent[n].rdkio_block   = block % netio_block_per_diskio;

		if (stbent[n].rdkio_block != stbent[n].cdkio_block) {
			if (stbent[n].rdkio_block !=
						stbent[n].cdkio_block + 1) {
				fprintf (stderr,
					"STB[%d]: lseek (req:%d v.s. cur:%d)\n",
						n,
						stbent[n].rdkio_block,
						stbent[n].cdkio_block);
				rlen = stbent[n].rdkio_block * 
						best_diskio_buffer_size;
				lseek (stbent[n].fd, rlen, SEEK_SET);
				// lseek ();
			// } else {
				/*
				fprintf (stderr, "Normal read %d/%d\n",
						stbent[n].rdkio_block,
						stbent[n].cdkio_block);
				*/
			}
			stbent[n].cdkio_block = stbent[n].rdkio_block;
			// stbent[n].current_block = stbent[n].request_block;

			ptr  = stbent[n].buffer;
			rlen = 0;

			while ((len = read (stbent[n].fd, &ptr[rlen],
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
				fprintf (stderr, "STB[%d]: File closed\n", n);

				stbent[n].filestatus = 2;
				// close (stbent[n].fd);
				// stbent[n].fd = -1;

				pthread_mutex_lock (&stb_mutex);
				stb_opend_file--;
				pthread_mutex_unlock (&stb_mutex);
			}

			stbent[n].buflen = rlen;

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
		fptr->h.filecnt = stbent[n].filecount;
		// fptr->h.block   = stbent[n].cdkio_block;
		fptr->h.block   = stbent[n].request_block;

		// pthread_mutex_unlock (&stbent[n].mutex);
		return 1;
	}

	return 0;
}

static int stbtbl_push (const int n, const int cmd) {
	// STBVOD_PDU_CMD_FAST_FETCH
	// STBVOD_PDU_CMD_FETCH

	if ((n >= 0) && (n < count)) {
#if USE_PRIORITY_QUEUE == 1
		netq->enqueue (netq, n);
#else
		netq->enqueue (netq, n,
				cmd == STBVOD_PDU_CMD_FAST_FETCH ? 0 : 1);
#endif
		return 1;
	}
	return 0;
}

static int stbtbl_pop (void) {
	return netq->dequeue (netq, 1);
}

static int stbtbl_init (const int n, const int sn) {
	if ((n >= 0) && (n < count)) {
		pthread_mutex_lock (&stbent[n].mutex);

		stbent[n].stb_sn = sn;

		if (stbent[n].buffer == NULL) {
			if ((stbent[n].buffer = malloc (
				best_diskio_buffer_size)) == NULL) {
				perror ("malloc");
			}
		}

		if (stbent[n].buffer == NULL) {
			fprintf (stderr, "Memory allocation error!\n");
			return 0;
		}

		pthread_mutex_unlock (&stbent[n].mutex);
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

static int stbtbl_stop (const int n) {
	if ((n >= 0) && (n < count)) {
		/*
		if (stbent[n].running) {
			pthread_kill (stbent[n].thread, SIGTERM);
			stbent[n].running = 0;
		}
		*/
		if (stbent[n].fd >= 0) {
			fprintf (stderr,
				"STB[%d]: File closed on stop !!\n", n);

			close (stbent[n].fd);
			stbent[n].fd = -1;

			if (stbent[n].filestatus != 2) {
				pthread_mutex_lock (&stb_mutex);
				stb_opend_file--;
				pthread_mutex_unlock (&stb_mutex);
			}

			stbent[n].filestatus = 0;
		}
	} else {
		return 0;
	}
	return 1;
}

static int stbtbl_setrequest (const int n, const int block,
					const char *map, const int qos) {
	int	j;

	if ((n >= 0) && (n < count)) {
		pthread_mutex_lock (&stbent[n].mutex);

		// i = (stbent[n].mapidx + 1) % 2;

		for (j = 0; j < 23; j++) {
			stbent[n].map[j] = map[j];
		}

		// stbent[n].mapidx = i;
		stbent[n].qos		= qos;
		stbent[n].request_block = block;
		stbent[n].rdkio_block   = block / netio_block_per_diskio;
		stbent[n].netio_block	= block % netio_block_per_diskio;
		/*
		fprintf (stderr, "Request block=%d/%d/%d\n", block,
				stbent[n].rdkio_block,
				stbent[n].netio_block);
		*/

		pthread_mutex_unlock (&stbent[n].mutex);
		return 1;
	}

	return 0;
}

static int stbtbl_udp (const int n) {
	if ((n >= 0) && (n < count)) return stbent[n].udp;
	return -1;
}

static int stbtbl_close_all (void) {
	int	i;
	int	svc_flag = stop_services;

	stop_services = 1;	// make sure services is stopped !!
	sleep (1);

	while (! netq->is_empty (netq)) {
		fprintf (stderr, "Wait for queue is empty !\n");
		sleep (1);
	}

	for (i = 0; i < count; i++) {
		pthread_mutex_lock   (&stbent[i].mutex);

		if (stbent[i].fd >= 0) {
			close (stbent[i].fd);
			stbent[i].fd = -1;
			stbent[i].filecount = 0;

			if (stbent[i].filestatus != 2) stb_opend_file--;

			stbent[i].filestatus = 0;
		}

		pthread_mutex_unlock (&stbent[i].mutex);
	}

	stop_services = svc_flag;

	return 1;
}

struct stbtable_t * new_stbtable (const int n, const int enable_qos) {
	int		i;

	if (count == 0) {
		stbent = calloc (n, sizeof (struct stb_entry_t));
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

#if USE_PRIORITY_QUEUE == 1
		if (enable_qos) {
			netq  = new_priority_queue (n, qos_compare);
		} else {
			netq  = new_priority_queue (n, NULL);
		}
#else
		netq = new_multiple_queue (n, 2, 256);
#endif

		// vodsvr.add		= vodsvr_add;
		// vodsvr.regist_code	= vodsvr_regist_code;
		for (i = 0; i < n; i++) {
			stbent[i].fd		= -1;
			stbent[i].filestatus	= 0;
			stbent[i].n		= i;
			stbent[i].inuse		= 0;
			// stbent[i].running	= 0;
			stbent[i].ready		= 0;
			stbent[i].buffer	= NULL;
			stbent[i].mapidx	= 0;
			stbent[i].udp		= -1;
			stbent[i].stbid		= -1;

			pthread_cond_init  (&stbent[i].condi, NULL);
			pthread_mutex_init (&stbent[i].mutex, NULL);

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
