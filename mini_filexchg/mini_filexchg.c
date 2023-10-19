/*
 *	mini_filexchg.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "tcplib.h"
#include "mini_filexchg.h"
#include "md5sum.h"

#define BEST_BUFFER_SIZE	1280

static void for_download (struct mini_filexchg_t *self, const int fd) {
	char			buffer[BEST_BUFFER_SIZE];
	int			*intptr = (int *) buffer;
	int			filename_len;
	int			len, rdlen, explen;
	int			total_len;
	int			ffd = -1;
	struct stat		stbuf;
	char			*filename;
	struct md5sum_t		*md5;

	fprintf (stderr, "For Download\n");

	if ((md5 = new_md5sum ()) == NULL) return;

	do {
		if ((len = read (fd, buffer, sizeof (int))) <= 0) {
			perror ("read");
			break;
		}

		if ((filename_len = *intptr) > 1000) {
			fprintf (stderr, "File name too long (%d)\n",
					filename_len);
			break;
		}

		fprintf (stderr, "File name length = %d\n", filename_len);

		explen = filename_len;
		rdlen  = 0;

		while (explen > 0) {
			if ((len = read (fd, &buffer[rdlen], explen)) <= 0) {
				perror ("read");
				break;
			}
			explen -= len;
			rdlen  += len;
		}

		if (explen > 0) break;

		buffer[filename_len] = '\0';

		fprintf (stderr, "File name is: [%s]\n",
				buffer);

		if (strstr (buffer, "..") != NULL) {
			*intptr = -1;
			write (fd, buffer, sizeof (int));
			break;
		}

		if (self->pd.exportdir == NULL) {
			filename = buffer;
		} else {
			if ((filename = alloca (filename_len + 2 +
					strlen (self->pd.exportdir))) == NULL) {
				perror ("alloca");
			}

			sprintf (filename, "%s/%s",
					self->pd.exportdir, buffer);
		}

		if ((ffd = open (filename, O_RDONLY)) < 0) {
			fprintf (stderr, "Read file [%s]\n", filename);
			perror (filename);
			*intptr = -1;
			write (fd, buffer, sizeof (int));
			break;
		}

		if (fstat (ffd, &stbuf) < 0) {
			*intptr = -1;
			write (fd, buffer, sizeof (int));
			break;
		}

		*intptr = stbuf.st_size;
		write (fd, buffer, sizeof (int));

		if (stbuf.st_size <= 0) break;

		fprintf (stderr, "File size is %d.\n", *intptr);
		total_len = *intptr;

		while ((len = read (ffd, buffer, sizeof buffer)) > 0) {
			md5->process (md5, buffer, len);
			explen = len;
			rdlen  = 0;
			total_len -= len;

			while (explen > 0) {
				len = write (fd, &buffer[rdlen], explen);
				if (len <= 0) {
					perror ("write");
					break;
				}

				rdlen  += len;
				explen -= len;
			}

			if (explen > 0) break;
		}

		if (total_len != 0) {
			perror ("read");
			break;
		}

		explen = md5->length ();
		rdlen  = 0;

		while (explen > 0) {
			if ((len = read (fd, &buffer[rdlen], explen)) <= 0) {
				perror ("read");
				break;
			}

			rdlen  += len;
			explen -= len;
		}
		if (explen > 0) break;

		if (memcmp (md5->finish (md5, NULL), buffer,
					md5->length ()) == 0) {
			*intptr = 1;
			fprintf (stderr, "File completed !! SUM=%s\n",
				md5->md5 (md5, NULL));
		} else {
			*intptr = 0;
			fprintf (stderr, "Check sum error !! SUM=%s\n",
				md5->md5 (md5, NULL));

			fprintf (stderr, "Received Checksum = ");
			for (len = 0 ; len < md5->length (); len++) {
				fprintf (stderr, "%02x",
						(unsigned char) buffer[len]);
			}
			fprintf (stderr, "\n");
		}

		write (fd, buffer, sizeof (int));
	} while (0);

	md5->dispose (md5);
	if (ffd >= 0) close (ffd);
}

static void for_upload (struct mini_filexchg_t *self, const int fd) {
	char			buffer[BEST_BUFFER_SIZE];
	int			*intptr = (int *) buffer;
	int			filename_len;
	int			len, rdlen, explen;
	int			total_len;
	int			ffd = -1;
	char			*filename;
	struct md5sum_t		*md5;
	int			filesize;

	fprintf (stderr, "For Upload\n");

	if ((md5 = new_md5sum ()) == NULL) return;

	do {
		if ((len = read (fd, buffer, sizeof (int))) <= 0) {
			perror ("read");
			break;
		}

		if ((filename_len = *intptr) > 1000) {
			fprintf (stderr, "File name too long (%d)\n",
					filename_len);
			break;
		}

		fprintf (stderr, "File name length = %d\n", filename_len);

		explen = filename_len;
		rdlen  = 0;

		while (explen > 0) {
			if ((len = read (fd, &buffer[rdlen], explen)) <= 0) {
				perror ("read");
				break;
			}
			explen -= len;
			rdlen  += len;
		}

		if (explen > 0) break;

		buffer[filename_len] = '\0';

		fprintf (stderr, "File name is: [%s]\n",
				buffer);

		if (strstr (buffer, "..") != NULL) {
			*intptr = -1;
			write (fd, buffer, sizeof (int));
			break;
		}

		if (self->pd.uploaddir == NULL) {
			break;
		} else {
			if ((filename = alloca (filename_len + 2 +
					strlen (self->pd.uploaddir))) == NULL) {
				perror ("alloca");
			}

			sprintf (filename, "%s/%s", self->pd.uploaddir, buffer);
		}

		if ((ffd = open (filename,
					O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
			perror (filename);
			*intptr = errno;
			write (fd, buffer, sizeof (int));
			break;
		} else {
			*intptr = 0;
			write (fd, buffer, sizeof (int));
		}

		read (fd, &filesize, sizeof filesize);

		if (filesize <= 0) break;

		fprintf (stderr, "File size is %d.\n", filesize);
		total_len = filesize;

		while (total_len > 0) {
			if (total_len > sizeof buffer) {
				len = sizeof buffer;
			} else {
				len = total_len;
			}

			if ((len = read (fd, buffer, len)) <= 0) break;

			md5->process (md5, buffer, len);

			total_len -= len;

			rdlen  = 0;
			explen = len;

			while (explen > 0) {
				len = write (ffd, &buffer[rdlen], explen);
				if (len <= 0) {
					perror ("write");
					break;
				}

				rdlen  += len;
				explen -= len;
			}

			if (explen > 0) break;
		}

		if (total_len != 0) {
			perror ("write");
			break;
		}

		write (fd, md5->finish (md5, NULL), md5->length ());

		read (fd, buffer, sizeof (int));

		if (*intptr == 1) {
			fprintf (stderr, "File completed !! SUM=%s\n",
				md5->md5 (md5, NULL));
		} else {
			fprintf (stderr, "Check sum error !! SUM=%s\n",
				md5->md5 (md5, NULL));
		}
	} while (0);

	md5->dispose (md5);
	if (ffd >= 0) close (ffd);
}

static void for_gettimeofday (struct mini_filexchg_t *self, const int fd) {
	struct timeval	tv;
	struct timezone tz;
	struct md5sum_t	*md5;

	if ((md5 = new_md5sum ()) == NULL) return;

	if (gettimeofday (&tv, &tz) < 0) {
		perror ("gettimeofday");
	} else {
		fprintf (stderr, "for Gettimeofday\n");

		write (fd, &tv, sizeof (struct timeval));
		write (fd, &tz, sizeof (struct timezone));

		md5->process (md5, (void *) &tv, sizeof (struct timeval));
		md5->process (md5, (void *) &tz, sizeof (struct timezone));

		write (fd, md5->finish (md5, NULL), md5->length ());
	}

	md5->dispose (md5);
}

static void fchg_server (const int fd, void *param) {
	struct mini_filexchg_t	*self = param;
	int			sel, len, explen, rdlen;
	char			*buffer;

	do {
		if (self->pd.request != NULL) {
			len = strlen (self->pd.request);

			if ((buffer = alloca (len)) == NULL) {
				perror ("alloca");
				break;
			}

			explen = len;
			rdlen  = 0;

			while (explen > 0) {
				if ((len = read (fd, &buffer[rdlen],
								explen)) <= 0) {
					perror ("read");
					break;
				}

				explen -= len;
				rdlen  += len;
			}

			if (explen != 0) break;

			if (memcmp (self->pd.request, buffer,
					strlen (self->pd.request)) != 0) {
				fprintf (stderr, "Error in signature !!\n");
				break;
			}
		}

		if (self->pd.response != NULL) {
			write (fd, self->pd.response,
					strlen (self->pd.response));
		}

		if (read (fd, &sel, sizeof sel) <= 0) {
			perror ("read");
		} else {
			switch (sel) {
			case 0:	// for ftpget (download)
				for_download (self, fd);
				break;
			case 1: // for ftpput (upload)
				sel = (self->pd.uploaddir == NULL) ? 0 : 1;
				write (fd, &sel, sizeof sel);

				if (sel) for_upload (self, fd);
				break;
			case 2: // for gettimeofday
				for_gettimeofday (self, fd);
				break;
			}
		}
	} while (0);

	close (fd);
	exit (0);
}

static int mfc_start (struct mini_filexchg_t *self, const int port) {
	struct tcplib_t		*tcp = self->pd.tcp;
	int			rc;

	tcp->multiconn (tcp, 1);
	if ((rc = tcp->listen (tcp, fchg_server, port, self)) < 0) return -1;

	return rc;
}

static void mfc_stop (struct mini_filexchg_t *self) {
	struct tcplib_t		*tcp = self->pd.tcp;

	tcp->stop (tcp);
}

static void mfc_uploads (struct mini_filexchg_t *self, const char *path) {
	if (self->pd.uploaddir != NULL) free (self->pd.uploaddir);
	if (path != NULL) self->pd.uploaddir = strdup (path);
}

static void mfc_exports (struct mini_filexchg_t *self, const char *path) {
	if (self->pd.exportdir != NULL) free (self->pd.exportdir);
	if (path != NULL) self->pd.exportdir = strdup (path);
}

static int mfc_ftpget (struct mini_filexchg_t *self,
				const char *ip, const int port,
				const char *rmfile, const char *lcfile) {
	struct tcplib_t		*tcp = self->pd.tcp;
	int			rc = 0;
	int			nfd, lfd = -1;
	char			buffer[BEST_BUFFER_SIZE];
	int			len, *intptr = (int *) buffer;
	int			filelen;
	struct md5sum_t		*md5;
	int			explen, rdlen;
	int			fnlen;

	while (rmfile != NULL) {
		if ((fnlen = strlen (rmfile)) == 0) break;

		if ((lfd = open (lcfile, O_CREAT|O_WRONLY|O_TRUNC, 0644)) < 0) {
			perror (lcfile);
			break;
		}

		if ((nfd = tcp->connect (tcp, ip, port)) < 0) {
			perror ("tcp->connect");
			break;
		}

		if (self->pd.request != NULL) {
			write (nfd, self->pd.request,
					strlen (self->pd.request));
		}

		if (self->pd.response != NULL) {
			explen = strlen (self->pd.response);
			rdlen  = 0;

			while (explen > 0) {
				if ((len = read (nfd, &buffer[rdlen],
							explen)) <= 0) {
					perror ("read");
					break;
				}
				explen -= len;
				rdlen  += len;
			}
			if (explen != 0) break;

			if (memcmp (self->pd.response, buffer,
					strlen (self->pd.response)) == 0) {
				// fprintf (stderr, "Good signature !\n");
			} else {
				fprintf (stderr, "error response !\n");
				break;
			}
		}

		*intptr = 0;
		write (nfd, buffer, sizeof (int));

		write (nfd, &fnlen, sizeof (int));
		write (nfd, rmfile, strlen (rmfile));

		if ((len = read (nfd, buffer, sizeof (int))) <= 0) {
			perror ("read");
			break;
		}

		filelen = *intptr;
		fprintf (stderr, "File length = %d\n", filelen);
		if (filelen <= 0) break;

		if ((md5 = new_md5sum ()) == NULL) break;

		while (filelen > 0) {
			if ((len = read (nfd, buffer, sizeof buffer)) <= 0) {
				perror ("read");
				break;
			}

			md5->process (md5, buffer, len);

			write (lfd, buffer, len);
			filelen -= len;
		}

		if (filelen > 0) {
			fprintf (stderr, "Short write\n");
			break;
		}

		write (nfd, md5->finish (md5, NULL), md5->length ());

		if ((len = read (nfd, buffer, sizeof (int))) <= 0) {
			perror ("read");
			break;
		}

		if (*intptr == 1) {
			fprintf (stderr, "Copy completed! SUM=%s\n",
					md5->md5 (md5, NULL));
		} else {
			fprintf (stderr, "Check sum error ! SUM=%s\n",
					md5->md5 (md5, NULL));
			break;
		}

		rc = 1;

		break;
	}

	tcp->close (tcp);
	if (lfd > 0) close (lfd);

	return rc;
}

static int mfc_ftpput (struct mini_filexchg_t *self,
				const char *ip, const int port,
				const char *rmfile, const char *lcfile) {
	struct tcplib_t		*tcp = self->pd.tcp;
	int			rc = 0;
	int			nfd, lfd = -1;
	char			buffer[BEST_BUFFER_SIZE];
	int			len, *intptr = (int *) buffer;
	int			filelen;
	int			rdlen, explen;
	struct md5sum_t		*md5;
	struct stat		stbuf;

	while (rmfile != NULL) {
		if ((len = strlen (rmfile)) == 0) break;

		if ((lfd = open (lcfile, O_RDONLY)) < 0) {
			perror (lcfile);
			break;
		}

		if (fstat (lfd, &stbuf) != 0) {
			perror ("fstat");
			break;
		}

		if (stbuf.st_size <= 0) {
			fprintf (stderr, "File size error !!\n");
			break;
		}

		if ((nfd = tcp->connect (tcp, ip, port)) < 0) {
			perror ("tcp->connect");
			break;
		}

		if (self->pd.request != NULL) {
			write (nfd, self->pd.request,
					strlen (self->pd.request));
		}

		if (self->pd.response != NULL) {
			explen = strlen (self->pd.response);
			rdlen  = 0;

			while (explen > 0) {
				if ((len = read (nfd, &buffer[rdlen],
							explen)) <= 0) {
					perror ("read");
					break;
				}
				explen -= len;
				rdlen  += len;
			}
			if (explen != 0) break;

			if (memcmp (self->pd.response, buffer,
					strlen (self->pd.response)) == 0) {
				// fprintf (stderr, "Good signature !\n");
			} else {
				fprintf (stderr, "error response !\n");
				break;
			}
		}

		// say I wanna upload
		*intptr = 1;
		write (nfd, buffer, sizeof (int));

		if ((len = read (nfd, buffer, sizeof (int))) <= 0) {
			perror ("read");
			break;
		}

		if (*intptr == 0) {
			fprintf (stderr, "Remote do not accept upload\n");
			break;
		}

		// say file name length
		*intptr = len;
		write (nfd, buffer, sizeof (int));
		// say upload file
		write (nfd, rmfile, strlen (rmfile));

		if ((len = read (nfd, buffer, sizeof (int))) <= 0) {
			perror ("read");
			break;
		}

		if (*intptr != 0) {
			fprintf (stderr, "Remote error: %s\n",
					strerror (*intptr));
			break;
		}

		// send file size
		filelen = *intptr = stbuf.st_size;
		write (nfd, buffer, sizeof (int));

		fprintf (stderr, "File length = %d\n", filelen);

		if ((md5 = new_md5sum ()) == NULL) break;

		while ((len = read (lfd, buffer, sizeof buffer)) > 0) {
			md5->process (md5, buffer, len);

			explen = len;
			rdlen  = 0;

			while (explen > 0) {
				len = write (nfd, &buffer[rdlen], explen);
				explen -= len;
				rdlen += len;
			}
		}

		read (nfd, buffer, md5->length ());

		if (memcmp (md5->finish (md5, NULL), buffer,
							md5->length ()) == 0) {
			*intptr = 1;
			write (nfd, buffer, sizeof (int));
			rc = 1;

			fprintf (stderr, "Copy completed! SUM=%s\n",
					md5->md5 (md5, NULL));
		} else {
			*intptr = 0;
			write (nfd, buffer, sizeof (int));

			fprintf (stderr, "Check sum error ! SUM=%s\n",
					md5->md5 (md5, NULL));
		}
		break;
	}

	tcp->close (tcp);
	if (lfd > 0) close (lfd);

	return rc;
}

static int mfc_gettimeofday (struct mini_filexchg_t *self,
			const char *ip, const int port,
			struct timeval *tv, struct timezone *tz) {
	struct tcplib_t		*tcp = self->pd.tcp;
	int			rc = 0;
	int			fd = -1;
	int			explen, rdlen;
	char			buffer[BEST_BUFFER_SIZE];
	int			len, *intptr = (int *) buffer;
	struct md5sum_t		*md5;

	do {
		if ((md5 = new_md5sum ()) == NULL) break;

		if ((fd = tcp->connect (tcp, ip, port)) < 0) {
			perror ("tcp->connect");
			break;
		}

		if (self->pd.request != NULL) {
			write (fd, self->pd.request,
					strlen (self->pd.request));
		}

		if (self->pd.response != NULL) {
			explen = strlen (self->pd.response);
			rdlen  = 0;

			while (explen > 0) {
				if ((len = read (fd, &buffer[rdlen],
							explen)) <= 0) {
					perror ("read");
					break;
				}
				explen -= len;
				rdlen  += len;
			}
			if (explen != 0) break;

			if (memcmp (self->pd.response, buffer,
					strlen (self->pd.response)) == 0) {
				// fprintf (stderr, "Good signature !\n");
			} else {
				fprintf (stderr, "error response !\n");
				break;
			}
		}

		// say I wanna gettimeofday
		*intptr = 2;
		write (fd, buffer, sizeof (int));

		explen = sizeof (struct timeval) +
			 sizeof (struct timezone) +
			 md5->length ();
		rdlen  = 0;

		while (explen > 0) {
			if ((len = read (fd, &buffer[rdlen], explen)) <= 0) {
				perror ("read");
				break;
			}
			explen -= len;
			rdlen  += len;
		}

		if (explen != 0) break;

		len   = sizeof (struct timeval);
		rdlen = 0;
		memcpy (tv, &buffer[rdlen], len);
		rdlen += len;
		len   = sizeof (struct timezone);
		memcpy (tz, &buffer[rdlen], len);
		rdlen += len;

		md5->process (md5, tv, sizeof (struct timeval));
		md5->process (md5, tz, sizeof (struct timezone));

		if (memcmp (&buffer[rdlen], md5->finish (md5, NULL),
					md5->length ()) == 0) {
			rc = 1;
			fprintf (stderr, "gettimeofday ok\n");
		} else {
			fprintf (stderr, "Checksum error !!\n");
		}
	} while (0);

	tcp->close (tcp);

	return rc;

}

static void mfc_set_request (struct mini_filexchg_t *self,
						const char *request) {
	if (self->pd.request != NULL) free (self->pd.request);
	self->pd.request = NULL;

	if (request != NULL) self->pd.request = strdup (request);
}

static void mfc_set_response (struct mini_filexchg_t *self,
						const char *response) {
	if (self->pd.response != NULL) free (self->pd.response);
	self->pd.response = NULL;

	if (response != NULL) self->pd.response = strdup (response);
}

static int mfc_rdate (struct mini_filexchg_t *self,
				const char *ip, const int port) {
	struct timeval	tv;
	struct timezone	tz;
	int		rc = 0;

	if (self->gettimeofday (self, ip, port, &tv, &tz)) {
		if (settimeofday (&tv, &tz) < 0) {
			perror ("settimeofday");
		} else {
			rc = 1;
		}
	}

	return rc;
}


static void mfc_dispose (struct mini_filexchg_t *self) {
	struct tcplib_t		*tcp = self->pd.tcp;

	self->set_request  (self, NULL);
	self->set_response (self, NULL);

	tcp->dispose (tcp);

	if (self->pd.exportdir != NULL) free (self->pd.exportdir);

	free (self);
}

struct mini_filexchg_t * new_mini_filexchg (void) {
	struct mini_filexchg_t	*self;

	if ((self = malloc (sizeof (struct mini_filexchg_t))) != NULL) {
		if ((self->pd.tcp = new_tcplib ()) == NULL) {
			free (self);
			self = NULL;
		} else {
			self->pd.exportdir	= NULL;
			self->pd.uploaddir	= NULL;
			self->pd.request	= NULL;
			self->pd.response	= NULL;
			self->start		= mfc_start;
			self->stop		= mfc_stop;
			self->ftpget		= mfc_ftpget;
			self->ftpput		= mfc_ftpput;
			self->exports		= mfc_exports;
			self->uploads		= mfc_uploads;
			self->set_request	= mfc_set_request;
			self->set_response	= mfc_set_response;
			self->gettimeofday	= mfc_gettimeofday;
			self->rdate		= mfc_rdate;
			self->dispose		= mfc_dispose;
		}
	}

	return self;
}
