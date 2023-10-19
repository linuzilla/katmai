/*
 *	stio_engine.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "stio_engine.h"
#include "thread_svc.h"
#include "buffer_cache.h"

#include "prefetch.c"

static ssize_t fio_read (struct stio_engine_t *self, void *buffer, size_t len) {
	char	*ptr = buffer;
	int	fd;
	int	rlen, explen, tlen;

	// fprintf (stderr, "FD=[%d]\n", self->pd.fd);

	if ((fd = self->pd.fd) < 0) return -1;

	explen = len;
	tlen   = 0;

	while (explen > 0) {
		if ((rlen = read (fd, &ptr[tlen], explen)) <= 0) {
			break;
		}
		tlen   += rlen;
		explen -= rlen;
	}
	// fprintf (stderr, "[%d] read\n", tlen);

	return tlen;
}

static int fio_open_next (struct stio_engine_t *self, const char *fname) {
	return 1;
}

static int fio_open (struct stio_engine_t *self, const char *fname) {
	int		fd;
	struct stat	stbuf;

	self->close (self);

	// fprintf (stderr, "fio open\n");

	if ((fd = open (fname, O_RDONLY)) < 0) {
		perror (fname);
		return -1;
	}

	if (fstat (fd, &stbuf) != 0) {
		perror (fname);
		close (fd);
		return -1;
	}

	// fprintf (stderr, "[%s] Opened\n", fname);

	self->pd.fsize = stbuf.st_size;
	self->pd.fd = fd;

	return fd;
}

static int fio_reset (struct stio_engine_t *self) {
	if (self->pd.fd >= 0) {
		lseek (self->pd.fd, 0, SEEK_SET);
		return 1;
	}
	return 0;
}

static int fio_seek (struct stio_engine_t *self, const off_t pos) {
	if (self->pd.fd >= 0) {
		lseek (self->pd.fd, pos, SEEK_SET);
		return 1;
	}
	return 0;
}

static int pub_set_repeat (struct stio_engine_t *self, const int r) {
	int	rc = self->pd.repeat_cnt;

	/*
	fprintf (stderr, "Set repeat=%d for %s\n",
			r, self->pd.name ? self->pd.name : "(:null:)");
	*/
	self->pd.repeat_cnt = r;

	return rc;
}

static void fio_close (struct stio_engine_t *self) {
	if (self->pd.fd >= 0) {
		close (self->pd.fd);
		self->pd.fd = -1;
		self->pd.fsize = 0;
	}
}

static off_t fio_filesize (struct stio_engine_t *self) {
	return self->pd.fsize;
}

static void fio_dispose (struct stio_engine_t *self) {
}

static ssize_t pub_read (struct stio_engine_t *self,
					void *buffer, size_t len) {
	int	rc;

	rc = self->pd.read (self, buffer, len);

	if (rc <= 0) {
		if (rc >= -1) {
			if (self->pd.repeat_cnt != 0) {
				if (self->pd.repeat_cnt > 0) {
					self->pd.repeat_cnt--;
				}
				self->reset (self);
				self->pd.fp_pos = 0;
				rc = self->pd.read (self, buffer, len);
			}
		}
	} else {
		self->pd.fp_pos += rc;
	}

	if (self->pd.read_plugin != NULL) {
		rc = self->pd.read_plugin (buffer, rc,
						self->pd.param_for_plugin);
	}

	return rc;
}

static int pub_open_next (struct stio_engine_t *self, const char *fname) {
	self->pd.have_next_file = 1;
	// self->pd.have_next_precache = 0;
	strncpy (self->pd.next_file, fname, sizeof self->pd.next_file - 1);
	self->pd.next_file[sizeof self->pd.next_file - 1] = '\0';
	// fprintf (stderr, "open next [%s]\n", self->pd.next_file);

	return self->pd.open_next (self, fname);
}

static int pub_open (struct stio_engine_t *self, const char *fname) {
	int	rc;
	
	// fprintf (stderr, "[%s]\n", fname);

	strncpy (self->pd.current_file, fname, sizeof self->pd.current_file -1);
	self->pd.current_file[sizeof self->pd.current_file -1] = '\0';

	self->pd.have_next_file = 0;

	rc = self->pd.open (self, fname);

	// fprintf (stderr, "pub open: %d\n", rc);

	self->pd.fp_pos = 0;

	if (rc >= 0) {
		self->pd.end_of_prefetch = 0;
		
		self->pd.repeat_cnt = 0;

		if (self->pd.read_plugin != NULL) {
			self->pd.read_plugin (NULL, 0,
						self->pd.param_for_plugin);
		}
	} else {
		self->pd.end_of_prefetch = 1;
	}

	return rc;
}

static int pub_reset (struct stio_engine_t *self) {
	int	rc;

	if ((rc = self->pd.reset (self)) > 0) {
		if (self->pd.read_plugin != NULL) {
			self->pd.read_plugin (NULL, 0,
						self->pd.param_for_plugin);
		}
	}

	return rc;
}

static off_t pub_read_pos (struct stio_engine_t *self) {
	return self->pd.fp_pos;
}

static int pub_seek (struct stio_engine_t *self, const off_t pos) {
	off_t	rc;

	if ((rc = self->pd.seek (self, pos)) >= 0) self->pd.fp_pos = rc;

	return rc;
}

static void pub_close (struct stio_engine_t *self) {
	return self->pd.close (self);
}

static off_t pub_filesize (struct stio_engine_t *self) {
	return self->pd.filesize (self);
}

static void pub_dispose (struct stio_engine_t *self) {
	self->close (self);
	self->pd.dispose (self);

	pbuf_dispose (self);
	free (self);
}

static void pub_set_name (struct stio_engine_t *self, char *name) {
	self->pd.name = name;
}

/*
static int pub_prefetch (struct stio_engine_t *self,
					const int nbuf, const int blen) {
	if (self->pd.pch != NULL) return 0;
	return 1;
}
*/

static int pub_end_of_prefetch (struct stio_engine_t *self) {
	return (self->pd.end_of_prefetch) ? 1 : 0;
}

static void * pub_set_read_plugin (struct stio_engine_t *self,
				int (*plugin)(void *, int, void *),
				void *param) {
	void	*rc = self->pd.read_plugin;

	self->pd.read_plugin      = plugin;
	self->pd.param_for_plugin = param;

	// fprintf (stderr, "OK .. Plugin is setting ok!\n");

	return rc;
}

static int stio_set_eop_callback (struct stio_engine_t *self,
						int (*func)(void *),
						void *parm) {
	self->pd.endof_prefetch_callback = func;
	self->pd.endof_prefetch_param = parm;
	return 1;
}

struct stio_engine_t * new_stio_engine (void) {
	struct stio_engine_t	*self;
	struct stio_engine_pd_t	*pd;

	if ((self = malloc (sizeof (struct stio_engine_t))) != NULL) {
		pd = &self->pd;

		pd->fd		= -1;
		pd->fsize	= 0;
		pd->myself	= NULL;
		pd->pch		= NULL;
		pd->abort_flag	= 0;
		pd->in_read	= 0;
		pd->repeat_cnt	= 0;
		pd->fp_pos	= 0;
		pd->end_of_prefetch	= 1;
		pd->have_next_file	= 0;
		pd->have_next_precache	= 0;
		pd->in_pq_prefetch	= 0;

		pd->read_plugin		= NULL;
		pd->param_for_plugin	= NULL;

		pd->endof_prefetch_callback	= NULL;
		pd->endof_prefetch_param	= NULL;

		self->open	= pub_open;
		self->open_next	= pub_open_next;
		self->reset	= pub_reset;
		self->seek	= pub_seek;
		self->read	= pub_read;
		self->close	= pub_close;
		self->abort	= pub_close;	// same as close
		self->filesize	= pub_filesize;
		self->set_repeat= pub_set_repeat;
		self->dispose	= pub_dispose;
		self->set_name  = pub_set_name;
		self->read_pos	= pub_read_pos;
		self->end_of_prefetch = pub_end_of_prefetch;
		self->set_read_plugin = pub_set_read_plugin;
		self->prefetch_is_running = pub_prefetch_is_running;

		self->set_buffer          = pub_set_buffer;
		self->set_keep_buffer     = pub_set_keep_buffer;
		self->set_prefetch_buffer = pub_set_prefetch_buffer;
		self->prefetch	= pub_prefetch;
		self->freebuf	= pub_free_buffer;

		self->set_eop_callback	= stio_set_eop_callback;

		self->start_pq_prefetch	= pub_start_pq_prefetch;
		self->stop_pq_prefetch	= pub_stop_pq_prefetch;

		pd->open	= fio_open;
		pd->open_next	= fio_open_next;
		pd->reset	= fio_reset;
		pd->seek	= fio_seek;
		pd->read	= fio_read;
		pd->close	= fio_close;
		pd->abort	= fio_close;	// same as close
		pd->filesize	= fio_filesize;
		pd->dispose	= fio_dispose;
		pd->read_plugin	= NULL;
		pd->name	= NULL;
	}

	return self;
}
