/*
 *	holktv_stb_cmd.c
 *
 *	Copyright (c) 2003, written by Jiann-Ching Liu
 */

#ifndef MAXIMUM_HKTV_STB_CMD
#define MAXIMUM_HKTV_STB_CMD		50
#endif

#ifndef QUIT_CNT
#define QUIT_CNT			10
#endif

#ifndef ENABLE_CONSOLE_IO
#define ENABLE_CONSOLE_IO		0
#endif

struct hktv_stbcmd_list_t {
	char		cmd;
	int		(*function)(struct holktv_regcmd_parm_t *);
};

static int		(*pre_function)(struct holktv_regcmd_parm_t *) = NULL;
static int		(*post_function)(struct holktv_regcmd_parm_t *) = NULL;
static int		(*idle_function)(struct holktv_regcmd_parm_t *) = NULL;

static struct hktv_stbcmd_list_t	cmdlist[MAXIMUM_HKTV_STB_CMD];
static int				cmdlist_idx = 0;

static int MasterCMD_check (const struct holktv_cmd_pdu *cpdu) {
	//	檢查 Master 的 Command 是否合法
	int	i, sum;
	int	len = cpdu->len - '0';
	char	checksum[10];

	if (cpdu->lead != '^') return 0;

	sum = (int) cpdu->cmd + (int) cpdu->echo + (int) cpdu->len;

	for (i = 0; i < len; i++) {
		sum += (int) cpdu->song[i];
	}

	sum &= 0xff;
	sprintf (checksum, "%02x", sum);

	if ((checksum[0] == cpdu->checksum[0]) &&
				(checksum[1] == cpdu->checksum[1])) {
		return 1;
	}

	/*
	fprintf (stderr, "[%c][%c]==[%c][%c]\n",
			checksum[0], checksum[1],
			cpdu->checksum[0],
			cpdu->checksum[1]);
	*/

	return 0;
}

static int hktvcmd_regist (const char cmd,
				int (*func)(struct holktv_regcmd_parm_t *)) {
	int	i;

	switch (cmd) {
	case '\r':
		if (pre_function != NULL) {
			fprintf (stderr, "Pre-function override\n");
		}

		pre_function = func;
		return 1;
	case '\n':
		if (post_function != NULL) {
			fprintf (stderr, "Post-function override\n");
		}

		post_function = func;
		return 1;
	case '\b':
		if (idle_function != NULL) {
			fprintf (stderr, "Idle function override\n");
		}

		idle_function = func;
		return 1;
	}

	for (i = 0; i < cmdlist_idx; i++) {
		if (cmdlist[i].cmd == cmd) {
			fprintf (stderr, "Duplicate command [%c]\n", cmd);
			return 0;
		}
	}

	if ((i = cmdlist_idx) < MAXIMUM_HKTV_STB_CMD) {
		cmdlist[i].cmd = cmd;
		cmdlist[i].function = func;
		cmdlist_idx++;

		return 1;
	}

	fprintf (stderr, "Command table full (%d entry)!!\n",
						MAXIMUM_HKTV_STB_CMD);
	return 0;
}

static int hktvcmd_mkcksum (struct holktv_cmd_reply_pdu *rpdu) {
	u_int		cksum;
	char		cksum_buf[5];
	int		i, len;

	cksum = (u_int) rpdu->state +
		(u_int) rpdu->flag +
		(u_int) rpdu->retcode[0] +
		(u_int) rpdu->retcode[1] +
		(u_int) rpdu->len;

	len = (u_int) (rpdu->len - '0');

	for (i = 0; i < len; i++) cksum += (u_int) rpdu->song[i];

	cksum &= 0xff;
	sprintf (cksum_buf, "%02x", cksum);
	rpdu->checksum[0] = cksum_buf[0];
	rpdu->checksum[1] = cksum_buf[1];

	return len;
}

static int hktvcmd_main (const int port) {
#if ENABLE_CONSOLE_IO == 1
	struct consoleIO_t	*conio = NULL;
	int			qstage = 0;
#endif
	struct udplib_t		*udp = NULL;
	int			terminate = 0;
	char			buffer[2000];
	char			reply_buf[256];
	int			i, len;
	char			waitmsg[] = "/|\\-";
	int			bidx = 0;
	int			rc;
	int			msec = 50;
	short			show_bar = 1;

	struct holktv_cmd_pdu		*pdu   = (void *) buffer;
	struct holktv_cmd_reply_pdu	*rpdu  = (void *) reply_buf;

	struct holktv_regcmd_parm_t	param = { NULL, pdu, rpdu };

#if ENABLE_CONSOLE_IO == 1
	conio = new_consoleIO ();

	conio->open ();
#endif


	do {
		if ((udp = new_udplib (1)) == NULL) {
			perror ("new_udplib");
			break;
		}

		if (udp->open (udp, port, NULL, 0) < 0) break;

		param.udp = udp;

		
		udp->set_sendflag (udp, 0);
		udp->set_recvflag (udp, 0);
		udp->nonblocking  (udp, 0);

		while (! terminate) {
			// if ((len = udp->receive (udp,
			//		buffer, sizeof buffer)) < 0) {
			if ((len = udp->recvms (udp,
					buffer, sizeof buffer, msec)) < 0) {
				if (errno == EAGAIN) {
					// udp->nonblocking  (udp, 0);
					// usleep (50000);
				} else {
					perror ("recvms");
				}
				continue;
			} else if (len == 0) {
				// fprintf (stderr, "wait\n");
				// sleep (1);
#if ENABLE_CONSOLE_IO == 1
				if (conio->kbhit ()) {
					switch (conio->read ()) {
					case 'q':
						qstage = 1;
						break;
					case 'u':
						qstage = qstage == 1 ?  2 : 0;
						break;
					case 'i':
						qstage = qstage == 2 ?  3 : 0;
						break;
					case 't':
						if (qstage == 3) {
							terminate= 1;
						} else {
							qstage = 0;
						}
						break;
					default:
						qstage = 0;
						break;
					}
				}
#endif

				if (idle_function != NULL) {
					if (idle_function (&param)) {
						len = hktvcmd_mkcksum (rpdu);

						udp->sendto (udp, 0,
							reply_buf,
							sizeof (
							struct
							holktv_cmd_reply_pdu) -
							HOLIDAY_DEFAULT_SONG_LEN
						       	-
							HOLIDAY_DEFAULT_SONG_LEN
						       	+ len
						    );
						msec = 1000; // 1 sec
					}
				} else if (show_bar) {
					bidx = (bidx+1) %
						(sizeof waitmsg - 1);
					fprintf (stderr,
							"%c\b", waitmsg[bidx]);
				}

				continue;
			}

			/*
			fprintf (stderr, "\rCommand from: %s (%d)[%c]:",
						udp->remote_addr (udp),
						udp->remote_port (udp),
						pdu->echo);
			*/

			if ((pdu->lead != '^') || (! MasterCMD_check (pdu))) {
				fprintf (stderr, "[%s] checksum error!\n",
						buffer);
				continue;
			}

			len = (int) (pdu->len - '0');
			if (len > 0) pdu->song[len] = '\0';

			rpdu->lead  = '$';
			// 假定命令是對的
			rpdu->retcode[0] = 'A';
			rpdu->retcode[1] = pdu->cmd;
			rpdu->len        = '0';
			rpdu->song[0]    = '\0';

			if (pre_function != NULL) pre_function (&param);

			for (i = rc = -1; i < cmdlist_idx; i++) {
				if (cmdlist[i].cmd == pdu->cmd) {
					rc = cmdlist[i].function (&param);
					break;
				}
			}

			if (rc == -1) {
				fprintf (stderr, "[-] Command %c not catch\n",
						pdu->cmd);
			}

			if (pdu->echo == 'Y') {
				if (post_function != NULL) {
					post_function (&param);
				}

				len = hktvcmd_mkcksum (rpdu);

				udp->send (udp, reply_buf,
					sizeof (struct holktv_cmd_reply_pdu) -
					HOLIDAY_DEFAULT_SONG_LEN -
					HOLIDAY_DEFAULT_SONG_LEN + len
				);
			}

			switch (rc) {
			case HKTV_CMD_RETURN_NOHANDLER:
				break;
			case HKTV_CMD_RETURN_NORMAL:
				break;
			case HKTV_CMD_RETURN_BLOCKING:
				show_bar = 0;
				msec = 1000;
				break;
			case HKTV_CMD_RETURN_NON_BLOCKING:
				show_bar = 1;
				msec = 50;
				break;
			case HKTV_CMD_RETURN_TERMINATE:
				terminate = 1;
				break;
			case HKTV_CMD_RETURN_REBOOT:
				terminate = 1;
				reboot (RB_AUTOBOOT);
				break;
			case HKTV_CMD_RETURN_SHUTDOWN:
				terminate = 1;
				reboot (RB_POWER_OFF);
				break;
			default:
				break;
			}
		 }
	} while (0);

#if ENABLE_CONSOLE_IO == 1
	conio->close ();
#endif

	if (udp != NULL) udp->close (udp);

	return 1;
}
