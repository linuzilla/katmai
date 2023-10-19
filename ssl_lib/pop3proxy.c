#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ssl_lib.h"


static struct SSL_lib_t	*ssl;
static volatile short	terminate = 0;

static void interrupt (const int signo) {
	terminate = 1;
}

static int do_service (const int fd, const int use_ssl) {
	fd_set			rfds;
	int			maxfd;
	int			cfd;
	int			i, j, rc, len;
	int			bytes;
	char			user[12];
	char			buf[65536];
	short			have_user = 0;
	char			pop3hello[] = "+OK pop3proxy ready\r\n";
	char			pop3err[] = "-ERR\r\n";

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

			fprintf (stderr, "[USER=%s]\n", user);
		} else {
			if (use_ssl) {
				ssl->server_write (ssl, pop3err,
						strlen (pop3err));
			} else {
				write (fd, pop3err, strlen (pop3err));
			}
		}
	}

	if (! ssl->connect (ssl, "dove.cc.ncu.edu.tw", 995)) {
		fprintf (stderr, "%s\n", ssl->errstr (ssl));
		return 1;
	}

	rc = ssl->client_read (ssl, buf, sizeof buf);
	sprintf (buf, "USER %s\r\n", user);
	ssl->client_write (ssl, buf, strlen (buf));

	cfd = ssl->client_fd (ssl);

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

			fwrite (buf, 1, bytes, stderr);

			if (ssl->client_write (ssl, buf, bytes) != bytes) {
				break;
			}
		}

		if (FD_ISSET (cfd, &rfds)) {
			if ((bytes = ssl->client_read (
					ssl, buf, sizeof buf)) <= 0) break;
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

	ssl->dispose (ssl);

	return 0;
}

int main (int argc, char *argv[]) {
	int			sockfd, sslfd, maxfd;
	fd_set			rfds;
	struct sockaddr_in	sa;
	int			fd, rc, len;
	int			port = 2110;
	int			sslport = 1995;


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
		ssl->set_CAfile   (ssl, "rootcert.pem");
		ssl->set_CERTfile (ssl, "server.pem");
	}

	if (ssl->listen (ssl, sslport) == 0) {
		close (sockfd);
		ssl->dispose (ssl);
		return 3;
	}

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
				fprintf (stderr, "[fd=%d]\n", fd);
				if (fork () == 0) {
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
