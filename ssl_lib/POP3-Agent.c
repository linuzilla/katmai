/*
 *	POP3-Agent.c
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <getopt.h>
#include <syslog.h>
#include "ssl_lib.h"

struct server_list_t {
	char			*user;
	regex_t			preg;
	char			*server;
	struct server_list_t	*next;
	short			use_ssl;
};


static struct SSL_lib_t		*ssl;
static struct server_list_t	*svlist = NULL;
static volatile short		terminate = 0;
static char			*config_file = "POP3-Agent.conf";



static int do_service (const int fd, const int use_ssl) {
	fd_set			rfds;
	int			maxfd;
	int			cfd;
	int			i, j, rc, len;
	int			bytes;
	char			user[18];
	char			buf[65536];
	short			have_user = 0;
	char			pop3hello[] = "+OK POP3-Agent ready, "
					"Written by Jiann-Ching Liu\r\n";
	char			pop3err[] = "-ERR\r\n";
	char			pop3bye[] = "+OK see you later.\r\n";
	struct server_list_t	*p;
	char			*popserver = "dove.cc.ncu.edu.tw";
	struct sockaddr_in	sa;
	int			salen;

	salen = sizeof sa;

	if (getpeername (fd, (struct sockaddr *) &sa, &salen) != 0) {
		perror ("getpeername");
		close (fd);
		ssl->dispose (ssl);
		exit (1);
	}


	len = strlen (pop3hello);

	if (use_ssl) {
		rc = ssl->server_write (ssl, pop3hello, len);
	} else {
		rc = write (fd, pop3hello, len);
	}

	if (rc <= 0) exit (0);

	while (! have_user) {
		if (use_ssl) {
			rc = ssl->server_read (ssl, buf, sizeof buf);
		} else {
			rc = read (fd, buf, sizeof buf);
		}
		if (rc <= 0) exit (0);

		if (strncasecmp (buf, "user ", 5) == 0) {
			have_user = 1;

			len = strlen (buf);

			for (j = 4; j < len; j++) if (buf[j] != ' ') break;

			for (i = 0; i < sizeof user - 1; i++, j++) {
				switch (buf[j]) {
				case '\r':
				case '\n':
				case ' ':
				case '@':
					user[i] = '\0';
					i = sizeof user;
					break;
				default:
					user[i] = buf[j];
					break;
				}
			}

			user[sizeof user - 1] = '\0';

			// fprintf (stderr, "[USER=%s]\n", user);
		} else if (strncasecmp (buf, "quit", 4) == 0) {
			if (use_ssl) {
				ssl->server_write (ssl, pop3bye,
						strlen (pop3bye));
				ssl->clear_server (ssl);
				ssl->shutdown_server (ssl);
				ssl->free_server (ssl);
				shutdown (fd, 0);
				close (fd);
				ssl->dispose (ssl);
				exit (0);
			} else {
				write (fd, pop3bye, strlen (pop3bye));
				shutdown (fd, 0);
				close (fd);
				exit (0);
			}
		} else {
			if (use_ssl) {
				ssl->server_write (ssl, pop3err,
						strlen (pop3err));
			} else {
				write (fd, pop3err, strlen (pop3err));
			}
		}
	}
	
	p = svlist;

	while (p != NULL) {
		if (regexec (&p->preg, user, 0, NULL, 0) == 0) {
			// fprintf (stderr, "Use:[%s][%s][%s]\n",
			//		user, p->user, p->server);
			break;
		}

		p = p->next;
	}

	if (p != NULL) {
		popserver = p->server;

	}

	if (p->use_ssl) {
		if (! ssl->connect (ssl, popserver, 995)) {
			fprintf (stderr, "%s\n", ssl->errstr (ssl));
			return 1;
		}
	} else {
		struct hostent		*hp;
		struct sockaddr_in	sca;

		if ((cfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
			perror ("socket");
			return 1;
		}

		sca.sin_family = AF_INET;

		if ((hp = gethostbyname (popserver)) == NULL) {
			fprintf(stderr, "%s: unknow host\n", popserver);
			return 1;
		}

		bcopy ((char*) hp->h_addr, (char*) &sca.sin_addr, hp->h_length);
		sca.sin_port = htons (110);

		if (connect (cfd, (struct sockaddr*)&sca, sizeof sca) < 0) {
			perror ("connecting");
			return 1;
		}
	}

	openlog ("pop3-agent", LOG_PID, LOG_LOCAL0);
	syslog (LOG_WARNING, "%s%s@%s:%s %s",
			use_ssl ? "SSL:" : "",
			user, popserver,
			p->use_ssl ? "pop3s" : "pop3",
			inet_ntoa (sa.sin_addr));
	closelog ();


	if (p->use_ssl) {
		rc = ssl->client_read (ssl, buf, sizeof buf);
	} else {
		rc = read (cfd, buf, sizeof buf);
	}

	sprintf (buf, "USER %s\r\n", user);

	if (p->use_ssl) {
		ssl->client_write (ssl, buf, strlen (buf));

		cfd = ssl->client_fd (ssl);
	} else {
		write (cfd, buf, strlen (buf));
	}

	maxfd = cfd > fd ? cfd : fd;

	while (! terminate) {
		struct timeval	tv;

		FD_ZERO (&rfds);
		FD_SET  (fd, &rfds);
		FD_SET  (cfd,  &rfds);

		tv.tv_sec  = 60;
		tv.tv_usec = 0;

		if ((rc = select (maxfd + 1, &rfds, NULL, NULL, &tv)) < 0) {
			if (errno != EINTR) perror ("select");
			continue;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET (fd, &rfds)) {
			if (use_ssl) {
				if ((bytes = ssl->server_read (
					ssl, buf, sizeof buf)) <= 0) break;

			} else {
				if ((bytes = read (fd,
						buf, sizeof buf)) <= 0) {
					break;
				}
			}

			// fwrite (buf, 1, bytes, stderr);

			if (p->use_ssl) {
				if (ssl->client_write (ssl,
						buf, bytes) != bytes) {
					break;
				}
			} else {
				if (write (cfd, buf, bytes) != bytes) {
					break;
				}
			}
		}

		if (FD_ISSET (cfd, &rfds)) {
			if (p->use_ssl) {
				if ((bytes = ssl->client_read (
					ssl, buf, sizeof buf)) <= 0) break;
			} else {
				if ((bytes = read (
					cfd, buf, sizeof buf)) <= 0) break;
			}

			// fwrite (buffer, 1, rc, stdout);

			if (use_ssl) {
				if (ssl->server_write (ssl, buf,
							bytes) != bytes) {
					break;
				}
			} else {
				if (write (fd, buf, bytes) != bytes) break;
			}
		}
	}

	if (! use_ssl) {
		shutdown (fd, 0);
		close (fd);
	}

	if (! p->use_ssl) {
		shutdown (cfd, 0);
		close (cfd);
	}

	ssl->dispose (ssl);

	return 0;
}

static void trim (char *buf) {
	int	i, j, len;

	len = strlen (buf);

	for (i = len - 1; i >= 0; i--) {
		if (buf[i] == ' ' || buf[i] == '\t') {
			len = i;
		} else {
			break;
		}
	}
	buf[len] = '\0';

	for (i = j = 0; i < len; i++) {
		if (buf[i] == ' ' || buf[i] == '\t') {
		} else {
			j = i;
			break;
		}
	}

	if (j != 0) {
		for (i = 0; j <= len; i++, j++) buf[i] = buf[j];
	}
}

static void chomp (char *buf) {
	int	i, len;

	len = strlen (buf);

	for (i = len - 1; i >= 0; i--) {
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = '\0';
		} else {
			break;
		}
	}
}

static int is_blank (char ch) {
	if (ch == ' ' || ch == '\t') return 1;
	return 0;
}

static int split_2 (char *buf, char **p1, char **p2) {
	int	i, len;
	int	stage = 0;

	len = strlen (buf);

	for (i = 0; i < len; i++) {
		switch (stage) {
		case 0:
			if (! is_blank (buf[i])) {
				*p1 = &buf[i];
				stage = 1;
			}
			break;
		case 1:
			if (is_blank (buf[i])) {
				buf[i] = '\0';
				stage = 2;
			}
			break;
		case 2:
			if (! is_blank (buf[i])) {
				*p2 = &buf[i];
				stage = 3;
			}
			break;

		case 3:
			if (is_blank (buf[i])) {
				buf[i] = '\0';
			}
			break;
		}
	}

	return (stage == 3) ? 1 : 0;
}

static void read_config (const char *file) {
	FILE			*fp;
	char			buf[512];
	char			*p1, *p2;
	char			*pp;
	struct server_list_t	*p, *q, **ptr;
	short			use_ssl;

	p = svlist;
	while (p != NULL) {
		free (p->user);
		free (p->server);
		regfree (&p->preg);
		q = p;
		p = p->next;
		free (q);
	}
	svlist = NULL;

	ptr = &svlist;

	if ((fp = fopen (file, "r")) != NULL) {
		while (fgets (buf, sizeof buf - 1, fp)) {
			chomp (buf);
			trim (buf);

			if (buf[0] == '#' || buf[0] == ';' || buf[0] == '\0') {
				continue;
			}

			if (! split_2 (buf, &p1, &p2)) continue;

			use_ssl = 1;

			if ((pp = strchr (p2, ':')) != NULL) {
				*pp = '\0';
				++pp;

				if (*pp == '\0') {
				} else if (strcasecmp (pp, "pop3") == 0) {
					use_ssl = 0;
				} else if (strcasecmp (pp, "pop3s") == 0) {
					use_ssl = 1;
				} else {
					fprintf (stderr,
						"%s: unknow protocol\n",
						pp);
					continue;
				}
			}

			// fprintf (stderr, "[%s][%s]\n", p1, p2);
			if ((p = malloc (sizeof *p)) == NULL) {
				perror ("malloc");
				exit (1);
			}

			if ((p->user = strdup (p1)) == NULL) {
				perror ("strdup");
				exit (1);
			}

			if ((p->server = strdup (p2)) == NULL) {
				perror ("strdup");
				exit (1);
			}
			p->next = NULL;
			p->use_ssl = use_ssl;

			if (regcomp (&p->preg, p->user,
					REG_EXTENDED|REG_NEWLINE) != 0) {
				fprintf (stderr, "%s: regcomp compiling error",
						p->user);
				exit (1);
			}

			*ptr = p;
			ptr = &p->next;
		}

		fclose (fp);
	}

	/*
	p = svlist;

	while (p != NULL) {
		fprintf (stderr, "[%s]-[%s]\n", p->user, p->server);
		p = p->next;
	}

	exit (1);
	*/
}

static void interrupt (int signo) {
	// signal (signo, interrupt);

	if (signo == SIGHUP) {
		read_config (config_file);
	} else {
		terminate = 1;
	}
}

int main (int argc, char *argv[]) {
	int			sockfd, sslfd, maxfd;
	fd_set			rfds;
	struct sockaddr_in	sa;
	int			fd, rc, len;
	int			port    = 110;
	int			sslport = 995;
	int			c, errflag = 0;
	char			*ca_file   = NULL; // "rootcert.pem";
	char			*cert_file = "server.pem";
	short			debug_flag = 0;
	int			option_index = 0;
	struct option		long_options[] = {
					{ "debug"    , 0, 0, 'd' },
					{ "port"     , 1, 0, 'p' },
					{ "ssl-port" , 1, 0, 's' },
					{ "cfg"      , 1, 0, 'f' },
					{ "ca"       , 1, 0, 'a' },
					{ "cert"     , 1, 0, 't' }
				};

	while (( c = getopt_long (argc, argv, "a:df:hp:s:t:",
				long_options, &option_index)) != EOF) {
		switch (c) {
		case 'd':
			debug_flag = 1;
			break;
		case 'p':
			port = atoi (optarg);
			break;
		case 's':
			sslport = atoi (optarg);
			break;
		case 'a':
			ca_file = optarg;
			break;
		case 'f':
			config_file = optarg;
			break;
		case 't':
			cert_file = optarg;
			break;
		case 'h':
		default:
			errflag++;
			break;
		}
	}

	if (errflag) {
		fprintf (stderr, "%s [options]\n\n"
				" --debug\n"
				" --port=110\n"
				" --ssl-port=995\n"
				" --ca=rootca.pem\n"
				" --cert=server.pem\n"
				" --cfg=POP3-Agent.conf\n"
				"\n", argv[0]);
		return 0;
	}

	read_config (config_file);

	if (! debug_flag) {
		if (fork () != 0) exit (0);
		setsid ();
		close (0);
		close (1);
		close (2);
	}


	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
		return 1;
	}

	memset (&sa, 0, sizeof sa);
	sa.sin_family		= AF_INET;
	sa.sin_addr.s_addr	= INADDR_ANY;
	sa.sin_port		= htons (port);

	if (bind (sockfd, (struct sockaddr *) &sa, sizeof sa) < 0) {
		perror("bind");
		close (sockfd);
		return 1;
	}

	len = sizeof sa;

	if (getsockname (sockfd, (struct sockaddr *) &sa,  &len) < 0) {
		perror ("getsockname");
		close (sockfd);
		return 1;
	}

	if ((ssl = new_SSL_lib ()) == NULL) {
		close (sockfd);
		return 2;
	} else {
		if (ca_file != NULL)   ssl->set_CAfile   (ssl, ca_file);
		if (cert_file != NULL) ssl->set_CERTfile (ssl, cert_file);
	}

	if (ssl->listen (ssl, sslport) == 0) {
		close (sockfd);
		ssl->dispose (ssl);
		return 3;
	}

	signal (SIGCHLD, SIG_IGN);
	signal (SIGHUP,  interrupt);
	signal (SIGINT,  interrupt);
	signal (SIGTERM, interrupt);

	sslfd = ssl->server_fd (ssl);

	maxfd = sslfd > sockfd ? sslfd : sockfd;

	listen (sockfd, 5);


	while (! terminate) {
		struct timeval	tv;

		FD_ZERO (&rfds);
		FD_SET  (sockfd, &rfds);
		FD_SET  (sslfd,  &rfds);

		tv.tv_sec  = 1800;
		tv.tv_usec = 0;

		if ((rc = select (maxfd + 1, &rfds, NULL, NULL, &tv)) < 0) {
			if (errno != EINTR) perror ("select");
			continue;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET (sockfd, &rfds)) {
			struct sockaddr_in	caddr;
			int			calen;

			calen = sizeof caddr;

			if ((fd = accept (sockfd,
					(struct sockaddr *) &caddr,
					&calen)) < 0) {
				perror ("accept");
			} else {
				if (fork () == 0) {
					signal (SIGHUP, SIG_IGN);
					close (sockfd);
					do_service (fd, 0);
					exit (0);
				}
				close (fd);
			}
		}

		if (FD_ISSET (sslfd, &rfds)) {
			if ((fd = ssl->accept (ssl)) < 0) {
			} else {
				// fprintf (stderr, "[fd=%d]\n", fd);
				if (fork () == 0) {
					signal (SIGHUP, SIG_IGN);
					close (sockfd);
					do_service (fd, 1);
					exit (0);
				}
				ssl->clear_server (ssl);
				// ssl->shutdown_server (ssl);
				ssl->free_server (ssl);
				close (fd);
			}
		}
	}

	close (sockfd);
	
	ssl->disconnect (ssl);
	ssl->dispose (ssl);


	return 0;
}
