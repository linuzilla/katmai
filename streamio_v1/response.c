/*
 *	response.c
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
#include <errno.h> 
#include <pthread.h>
#include "vodsvr.h"
#include "vodstb.h"
#include "udplib.h"
#include "stb_table.h"

#define RECV_BUF_SIZE		1518

struct udplib_t		*stbudp = NULL;
static short		ready   = 0;

#if ENABLE_VERSION_1_STREAMMING_IO
static int check_regist (const int sn) {
	struct stb_vod_pdu_t	pdu;

	if (my_id == (sn >> 8)) return sn & 0xff;

	pdu.cmd = STBVOD_PDU_CMD_NEED_REGIST;
	pdu.cksum = vodstb->cksum (&pdu, sizeof pdu);
	stbudp->send (stbudp, &pdu, sizeof pdu);
	return -1;
}
#endif

void response_main (void) {
	char			buffer[2000];
	int			cksum, len;
	struct stb_vod_pdu_t    *ptr = (struct stb_vod_pdu_t *) buffer;
#if ENABLE_VERSION_1_STREAMMING_IO
	int			i, j, err;
	struct stb_vod_regist_t	*svrptr = &ptr->regist;
	struct stb_vod_fileio_t	*fioptr = &ptr->fileio;
	struct stb_vod_fetch_t	*fthptr = &ptr->fetch;
	struct stb_vod_fetch_request_t *ftreq = (void *) &ptr->fetch;
#endif

	while (! terminate) {
		if ((stbudp = new_udplib (256)) == NULL)   break;
		// if (stbudp->open (stbudp, VODSVR_PORT, "eth0") < 0) break;
		if (stbudp->open (stbudp, VODSVR_PORT, NULL, 0) < 0) break;

		stbudp->set_sendflag (stbudp, 0);
		stbudp->set_recvflag (stbudp, 0);

		fprintf (stderr, "Listen on %d [ for STB ]\n",
				VODSVR_PORT);

		ready = 1;

		while (! terminate) {
			if ((len = stbudp->recv (stbudp,
					buffer, sizeof buffer, 0)) < 0) {
				if (errno == EAGAIN) {
					usleep (20000);
				} else {
					perror (NULL);
				}
				continue;
			} else if (len == 0) {
				perror (NULL);
				continue;
			}

			/*
			fprintf (stderr, "Received from STB: %s:%d (%d)\n",
					stbudp->remote_addr (stbudp),
					stbudp->remote_port (stbudp),
					len);
					*/

			cksum = vodstb->cksum (buffer, len);

			if (cksum != ptr->cksum) {
				fprintf (stderr, "Checksum error!!\n");
				continue;
			} else if (stop_services) {
				fprintf (stderr, "Stop services\n");
				continue;
			}

			switch (ptr->cmd) {
			case STBVOD_PDU_CMD_FINDSERVER:
				// fprintf (stderr ,"find server\n");
				stbudp->send (stbudp, buffer, len);
				break;
#if ENABLE_VERSION_1_STREAMMING_IO
			case STBVOD_PDU_CMD_REGIST:
				j = stbudp->regist_rmaddr (stbudp);
				i = stb->regist (
					stbudp->get_remote (stbudp), j);

				if (i >= 0) {
					svrptr->sn = (my_id << 8) + i;
				} else {
					svrptr->sn = -1;
				}
				fprintf (stderr ,"Regist %d (%d,%d)\n",
						i, svrptr->sn, j);

				svrptr->threshold = stb_qos_threshold;
				svrptr->version_major = version_major;
				svrptr->version_minor = version_minor;
				ptr->cksum = vodstb->cksum (buffer, len);

				stbudp->send (stbudp, buffer, len);

				break;
			case STBVOD_PDU_CMD_OPENFILE:
				if ((i = check_regist (fioptr->sn)) < 0) break;

				fprintf (stderr,
					"STB[%d]: Playfile [%s], cnt=%u\n",
						i, fioptr->file,
						fioptr->filecnt);

				fioptr->filesize = stb->setfile (i,
						fioptr->file, fioptr->filecnt,
						fioptr->stbid, &err);

				fioptr->err = err;
				ptr->cksum = vodstb->cksum (buffer, len);
				stbudp->send (stbudp, buffer, len);

				stb->init (i, fioptr->sn);

				break;
			case STBVOD_PDU_CMD_FAST_FETCH:
				/*
				if ((i = check_regist (fthptr->sn)) < 0) break;
				fprintf (stderr, 
					"STB[%d]: fast fetch\n", i);
					*/
			case STBVOD_PDU_CMD_FETCH:
				if ((i = check_regist (fthptr->sn)) < 0) break;

				/*
				fprintf (stderr, "fetch, entry %d, cnt=%u\n",
						i, fthptr->filecnt);
				*/

				if (! stb->is_file_matched (i,
							fthptr->filecnt)) {
					ptr->cmd = STBVOD_PDU_CMD_NEED_REOPEN;
					ptr->cksum = vodstb->cksum (buffer,
								len);
					stbudp->send (stbudp, buffer, len);
					break;
				}

				ptr->cmd = STBVOD_PDU_CMD_FETCH_RESPONSE;
				ptr->cksum = vodstb->cksum (buffer, len);
				stbudp->send (stbudp, buffer, len);

				// 如果還在送 ...   等送完
				// 如果已經送完 ... 重送
				
				stb->setrequest (i, ftreq->h.block,
							ftreq->map, ftreq->qos);
				stb->push (i, ptr->cmd);

				// stb->start (i, fthptr->sn);
				// 丟進 Queue 中
				break;
			case STBVOD_PDU_CMD_CLOSEFILE:
				if ((i = check_regist (fthptr->sn)) < 0) break;

				fprintf (stderr, "closefile\n");
				break;
#endif
			default:
				break;
			}
		}
	}

	if (stbudp != NULL) stbudp->close (stbudp);
	terminate = 1;

	fprintf (stderr, "Response: terminated\n");
	pthread_exit (NULL);
}

void reply_fetch_main (void) {
	while ((! ready) && (! terminate)) sleep (1);

	while (! terminate) {
	}

	terminate = 1;
	pthread_exit (NULL);
}
