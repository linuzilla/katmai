/*
 *	ftpclient.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <arpa/ftp.h>
#include <arpa/telnet.h>
#include "ftpclient.h"

static int getreply (struct ftp_client_t *self, int expecteof) {
	char	*cp;
	int	c, n, dig, code;
	int	pflag = 0;
	size_t	px = 0;
	size_t	psize = sizeof self->pd.pasv;
	int	continuation = 0;
	int	originalcode = 0;

	while (1) {
		dig = n = code = 0;
		cp = self->pd.reply_string;

		while ((c = getc (self->pd.cin)) != '\n') {
			if (c == IAC) {
				switch (c = getc (self->pd.cin)) {
				case WILL:
				case WONT:
					c = getc (self->pd.cin);
					fprintf (self->pd.cout, "%c%c%c",
								IAC, DONT, c);
					fflush (self->pd.cout);
				case DO:
                                case DONT:
					c = getc (self->pd.cin);
					fprintf (self->pd.cout, "%c%c%c",
								IAC, WONT, c);
					fflush (self->pd.cout);
					break;
				}
				continue;
			}
			dig++;

			if (c == EOF) {
				if (expecteof) {
				}
			}

			if (self->pd.verbose > 1) {
				if (c != '\r') putchar (c);
			}

			if (dig < 4 && isdigit(c)) {
				code = code * 10 + (c - '0');
			}

			if (!pflag && code == 227) pflag = 1;

			if (dig > 4 && pflag == 1 && isdigit(c)) pflag = 2;

			if (pflag == 2) {
				if (c != '\r' && c != ')') {
					if (px < psize-1) {
						self->pd.pasv[px++] = c;
					}
				} else {
					self->pd.pasv[px] = '\0';
					pflag = 3;
				}
			}

			if (dig == 4 && c == '-') {
				if (continuation) code = 0;
				continuation++;
			}

			if (n == 0) n = c;

			if (cp < &self->pd.reply_string[
					sizeof(self->pd.reply_string) - 1]) {
				*cp++ = c;
			}
		}

		if (self->pd.verbose > 1) {
			putchar (c);
			fflush (stdout);
		}

		if (continuation && (code != originalcode)) {
			if (originalcode == 0) originalcode = code;
			continue;
		}

		*cp = '\0';

		self->pd.code = code;
		return n - '0';
	}

	return 1;
}

static int command (struct ftp_client_t *self, const char *fmt, ...) {
	va_list	ap;
	int	rc;

	if (self->pd.debug) {
		printf ("---> ");
		va_start (ap, fmt);
		if (strncmp ("PASS ", fmt, 5) == 0) {
			printf ("PASS XXXX");
		} else {
			vfprintf (stdout, fmt, ap);
		}
		va_end (ap);

		printf ("\n");
		fflush (stdout);
	}

	if (self->pd.cout == NULL) {
		perror ("No control connection for command");
		self->pd.code = -1;
		return 0;
	}

	va_start (ap, fmt);
	vfprintf (self->pd.cout, fmt, ap);
	va_end (ap);
	fprintf (self->pd.cout, "\r\n");
	fflush (self->pd.cout);

	rc = getreply (self, !strcmp (fmt, "QUIT"));
	return rc;
}

static int initconn (struct ftp_client_t *self) {
	while (self->pd.passive) {
		u_long	a1, a2, a3, a4, p1, p2;

		self->pd.datafd = socket(AF_INET, SOCK_STREAM, 0);

		if (self->pd.datafd < 0) {
			perror ("socket");
			return 0;
		}

		if (command (self, "PASV") != COMPLETE) {
			printf ("Passive mode refused.\n");
			self->pd.passive = 0;
			break;
		} else if (sscanf (self->pd.pasv, "%ld,%ld,%ld,%ld,%ld,%ld",
					&a1,&a2,&a3,&a4,&p1,&p2) != 6) {
			printf ("Passive mode address scan failure. "
					"Shouldn't happen!\n");
			self->pd.passive = 0;
			break;
		}

		self->pd.data_addr.sin_family = AF_INET;
		self->pd.data_addr.sin_addr.s_addr =
				htonl((a1 << 24) | (a2 << 16) | (a3 << 8) | a4);
		self->pd.data_addr.sin_port = htons((p1 << 8) | p2);

		if (connect (self->pd.datafd,
				(struct sockaddr *) &(self->pd.data_addr),
				sizeof (self->pd.data_addr)) <0 ) {
			perror ("ftp: connect");
			return 0;
		}

		return 1;
	}

	/*
	self->pd.data_addr = self->pd.myctladdr;
	self->pd.data_addr.sin_port = 0; // let system pick one

	if (self->pd.datafd >= 0) close (self->pd.datafd);

	if ((self->pd.datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
		return 0;
	}

	if (bind (self->pd.datafd,
			(struct sockaddr *) &self->pd.data_addr,
			sizeof self->pd.data_addr) < 0) {
		perror ("ftp: bind");
		close (self->pd.datafd);
		self->pd.datafd = -1;
		return 0;
	}

	return 1;
	*/
	return 0;
}

static FILE *dataconn (struct ftp_client_t *self, const char *lmode) {
	// struct sockaddr_in	from;
	// int			s, tos;
	// socklen_t		fromlen = sizeof from;

	if (self->pd.passive) {
		return fdopen (self->pd.datafd, lmode);
	}

	/*
	 */
	return NULL;
}

static int sendrequest (struct ftp_client_t *self, const char *cmd,
				char *local, char *remote, int printnames) {
	FILE *volatile		fin;
	FILE *volatile		dout;
	char			buf[BUFSIZ], *bufp;
	int			c, d;
	volatile long		bytes = 0;

	if ((fin = fopen (local, "r")) == NULL) {
		perror (local);
		return 0;
	}

	if (! initconn (self)) {
		self->pd.code = -1;
		fclose (fin);
		return 0;
	}

	if (remote != NULL) {
		if (command (self, "%s %s", cmd, remote) != PRELIM) {
			fclose (fin);
			return 0;
		}
	} else {
		if (command (self, "%s", cmd) != PRELIM) {
			fclose (fin);
			return 0;
		}
	}

	dout = dataconn (self, "w");

	if (dout == NULL) {
		fprintf (stderr, "Data connection fail\n");
		fclose (fin);
		return 0;
	}

	d = 0;

	while ((c = read (fileno (fin), buf, sizeof (buf))) > 0) {
		bytes += c;
		for (bufp = buf; c > 0; c -= d, bufp += d) {
			if ((d = write (fileno (dout), bufp, c)) <= 0) break;
		}
	}

	fclose (fin);
	fclose (dout);

	close (self->pd.datafd);
	self->pd.datafd = -1;

	(void) getreply(self, 0);

	return 1;
}

static int recvrequest (struct ftp_client_t *self, const char *cmd,
				char *volatile local, char *remote,
				const char *lmode, int printnames) {
	FILE *volatile	din;
	FILE *volatile	fout;
	volatile int	is_retr;
	int		c, d;
	struct stat	st;
	static unsigned	bufsize = 0;
	static char	*buf = NULL;

	is_retr = (strcmp (cmd, "RETR") == 0);

	if (! initconn (self)) {
		self->pd.code = -1;
		return 0;
	}

	if (is_retr && (self->pd.restart_point != 0)) {
		if (command (self, "REST %ld",
				(long) self->pd.restart_point) != CONTINUE) {
			fprintf (stderr, "Can not RSET\n");
			return 0;
		}
	}

	if (remote != NULL) {
		if (command (self, "%s %s", cmd, remote) != PRELIM) {
			return 0;
		}
	} else {
		if (command (self, "%s", cmd) != PRELIM) {
			return 0;
		}
	}

	if ((local == NULL) || (strcmp (local, "-") == 0)) {
		fout = stdout;
	} else if (*local != '|') {
		/*
		if (access (local, W_OK) < 0) {
			perror (local);
			return 0;
		}
		*/
		fout = fopen (local, lmode);
	}

	din = dataconn (self, "r");

	if (fstat (fileno (fout), &st) < 0 || st.st_blksize == 0) {
		st.st_blksize = BUFSIZ;
	}

	if (st.st_blksize > bufsize) {
		if (buf != NULL) free (buf);
		buf = malloc ((unsigned) st.st_blksize);
		bufsize = st.st_blksize;
	}

	while ((c = read (fileno (din), buf, bufsize)) > 0) {
		if ((d = write (fileno (fout), buf, c)) != c) break;
	}

	fclose (din);
	close (self->pd.datafd);

	if (fout != stdout) fclose (fout);

	self->pd.datafd = -1;

	getreply (self, 0);
	
	return 0;
}

static int ftpclient_setpeer (struct ftp_client_t *self,
					const char *host, const int port) {
	struct hostent	*hp;
	int		len;

	memset (&self->pd.servaddr, 0, sizeof (self->pd.servaddr));

	if (inet_aton (host, &self->pd.servaddr.sin_addr) != 0) {
		self->pd.servaddr.sin_family = AF_INET;
	} else if ((hp = gethostbyname (host)) != NULL) {
		self->pd.servaddr.sin_family = hp->h_addrtype;
		len = hp->h_length;

		if (len > sizeof self->pd.servaddr.sin_addr) {
			len = sizeof self->pd.servaddr.sin_addr;
		}

		memcpy (&self->pd.servaddr.sin_addr,
				hp->h_addr_list[0], len);
	} else {
		perror (host);
		return 0;
	}

	self->pd.servaddr.sin_port = htons (port);
	self->pd.have_peer = 1;

	return 1;
}

static int ftpclient_settype (struct ftp_client_t *self, const int binary) {
	char	*stp[] = { "A", "I" };
	int	comret;
	int	flag;

	flag = binary ? 1 : 0;
	comret = command (self, "TYPE %s", stp[flag]);

	if (comret == COMPLETE) self->pd.mode = flag;

	return self->pd.mode;
}

static int ftpclient_quit (struct ftp_client_t *self) {
	command (self, "QUIT");
	return 1;
}

static int ftpclient_cdup (struct ftp_client_t *self) {
	command (self, "CDUP");
	return 1;
}

static int ftpclient_cwd (struct ftp_client_t *self, const char *path) {
	command (self, "CWD %s", path);
	return 1;
}

static int ftpclient_dir (struct ftp_client_t *self, char *path) {
	recvrequest (self, "LIST", "-", path, "w", 0);
	return 1;
}

static int getit (struct ftp_client_t *self, const int restartit,
			const char *modstr, char *rm, char *lc) {
	
	self->pd.restart_point = 0;

	if (restartit) {
		struct stat	stbuf;
		int		ret;

		ret = stat (lc, &stbuf);

		if (ret == 0) {
			self->pd.restart_point = stbuf.st_size;
		}
	}

	recvrequest (self, "RETR", lc, rm, modstr, 0);

	return 1;
}

static int ftpclient_reget (struct ftp_client_t *self, char *rm, char *lc) {
	return getit (self, 1, "a", rm, lc);
}

static int ftpclient_get (struct ftp_client_t *self, char *rm, char *lc) {
	return getit (self, 0, "w", rm, lc);
}

static int ftpclient_put (struct ftp_client_t *self, char *rm, char *lc) {
	sendrequest (self, "STOR", lc, rm, 0);
	return 1;
}

static int ftpclient_del (struct ftp_client_t *self, const char *path) {
	command (self, "DELE %s", path);
	return 1;
}

static int ftpclient_login (struct ftp_client_t *self,
					const char *user, const char *pass) {
	int	c, n;
	char	*cp;
	/*
	strncpy (self->pd.user, user, sizeof self->pd.user - 1);
	strncpy (self->pd.pass, pass, sizeof self->pd.pass - 1);
	*/

	// fprintf (stderr, "[%s][%s]\n", self->pd.user, self->pd.pass);
	n = command (self, "USER %s", user);

	if (n == CONTINUE) {
		n = command (self, "PASS %s", pass);
	}

	/*
	if (n == CONTINUE) {
		n = command (self, "ACCT %s", acct);
	}
	 */

	if (n != COMPLETE) {
		fprintf (stderr, "Login failed.\n");
		return 0;
	}

	if (command (self, "SYST") == COMPLETE) {
		 cp = index (self->pd.reply_string + 4, ' ');

		 if (cp == NULL) {
			 cp = index (self->pd.reply_string + 4, '\r');
		 } else {
			 if (cp[-1] == '.') cp--;
			 c = *cp;
			 *cp = '\0';

			if (self->pd.verbose > 0) {
				printf ("Remote system type is [%s].\n",
					  self->pd.reply_string + 4);
			}
		 }
	}

	self->pd.login_ok = 1;


	return 1;
}

static int ftpclient_close (struct ftp_client_t *self) {
	if (self->pd.sockfd >= 0) {
		shutdown (self->pd.sockfd, 2);
		close (self->pd.sockfd);

		self->pd.sockfd = -1;
	}

	return 1;
}

static int ftpclient_connect (struct ftp_client_t *self) {
	int	len;

	if (! self->pd.have_peer) return 0;

	if (self->pd.sockfd < 0) {
		self->pd.sockfd = socket (self->pd.servaddr.sin_family,
						SOCK_STREAM, 0);

		if (self->pd.sockfd < 0) {
			perror ("socket");
			return 0;
		}
	}

	if (self->pd.connected) return 0;

	while (connect (self->pd.sockfd,
				(struct sockaddr *) &self->pd.servaddr,
				sizeof self->pd.servaddr) < 0) {
		perror ("connect");
		return 0;
	}

	len = sizeof self->pd.myctladdr;

	if (getsockname (self->pd.sockfd,
			(struct sockaddr *) &self->pd.myctladdr, &len) < 0) {
                perror("ftp: getsockname");
		ftpclient_close (self);
		return 0;
	}

	self->pd.cin  = fdopen (self->pd.sockfd, "r");
	self->pd.cout = fdopen (self->pd.sockfd, "w");

	if ((self->pd.cin == NULL) || (self->pd.cout == NULL)) {
		if (self->pd.cin  != NULL) fclose (self->pd.cin);
		if (self->pd.cout != NULL) fclose (self->pd.cout);
		fprintf (stderr, "fdopen failed.\n");
		ftpclient_close (self);
		return 0;
	}

	// printf ("Connected to %s\n", inet_ntoa (self->pd.servaddr.sin_addr));

	if (getreply (self, 0) > 2) {
		if (self->pd.cin  != NULL) fclose (self->pd.cin);
		if (self->pd.cout != NULL) fclose (self->pd.cout);
		ftpclient_close (self);
		self->pd.code = -1;

		return 0;
	}

	/*
	fprintf (stderr, "%d,%d\n", getreply (self, 0), self->pd.code);
	fprintf (stderr, "code = %d\n", self->pd.code);
	*/

	self->pd.connected = 1;

	return 1;
}

static int ftpclient_debug (struct ftp_client_t *self, const int db) {
	int	rc = self->pd.debug;

	self->pd.debug = db;
	return rc;
}

static int ftpclient_verbose (struct ftp_client_t *self, const int vb) {
	int	rc = self->pd.verbose;

	self->pd.verbose = vb;
	return rc;
}

static int ftpclient_passive (struct ftp_client_t *self, const int ps) {
	int	rc = self->pd.passive;

	self->pd.passive = ps;
	return rc;
}

static void clear_variable (struct ftp_client_private_date_t *ptr) {
	ptr->connected = 0;
	ptr->have_peer = 0;
	ptr->passive   = 0;
	ptr->verbose   = 0;
	ptr->debug     = 0;
	ptr->login_ok  = 0;
	ptr->sockfd    = -1;
	ptr->datafd    = -1;
}

struct ftp_client_t *new_ftpclient (void) {
	struct ftp_client_t	*self;

	if ((self = malloc (sizeof (struct ftp_client_t))) != NULL) {
		self->setpeer	= ftpclient_setpeer;
		self->login	= ftpclient_login;
		self->connect	= ftpclient_connect;
		self->close	= ftpclient_close;
		self->passive	= ftpclient_passive;
		self->verbose	= ftpclient_verbose;
		self->debug	= ftpclient_debug;
		self->settype	= ftpclient_settype;
		self->quit	= ftpclient_quit;
		self->cwd	= ftpclient_cwd;
		self->cdup	= ftpclient_cdup;
		self->dir	= ftpclient_dir;
		self->get	= ftpclient_get;
		self->reget	= ftpclient_reget;
		self->put	= ftpclient_put;
		self->del	= ftpclient_del;

		clear_variable (&self->pd);
	}

	return self;
}

#if 0
int main (int argc, char *argv[]) {
	struct ftp_client_t	*ftp;

	if ((ftp = new_ftpclient ()) == NULL) {
		perror ("new_ftpclient");
		exit (1);
	}

	ftp->setpeer (ftp, "140.115.11.122", 21);
	ftp->verbose (ftp, 2);
	ftp->debug   (ftp, 1);
	ftp->passive (ftp, 1);

	if (ftp->connect (ftp)) {
		if (ftp->login (ftp, "ftp", "anonymous")) {
			ftp->settype (ftp, 1);
			ftp->dir   (ftp, NULL);
			ftp->cwd   (ftp, "APACHE");
			ftp->dir   (ftp, NULL);
			ftp->get   (ftp, "mod_auth_mysql.c.gz", "test");
			ftp->reget (ftp, "mod_auth_mysql.c.gz", "test2");
			ftp->cdup  (ftp);
			ftp->dir   (ftp, "Linux");
			ftp->cwd   (ftp, "incoming");
			ftp->put   (ftp, "test", "/etc/issue");
			ftp->get   (ftp, "test", "issue");
			ftp->del   (ftp, "test");
			ftp->cdup  (ftp);
			ftp->cwd   (ftp, "GNU");
			ftp->reget (ftp, "emacs-21.2.tar.gz", "emacs.tar.gz");
			ftp->dir   (ftp, NULL);
		}
		ftp->quit (ftp);
		ftp->close (ftp);
	}

	return 0;
}
#endif
