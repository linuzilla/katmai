/*
 *	playlist.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misclib.h"
#include "playlist.h"

struct strlist_t {
	char			*str;
	struct strlist_t	*next;
};

// time_t mktime(struct tm *timeptr);

static time_t format_time (const char *str) {
	struct tm	tmrec;
	int		i, j, k, m, v, len;
	int		split[] = { 4, 2, 2, 1, 2, 2, 0 };

	for (i = j = 0; split[i] != 0; i++) j += split[i];
	if ((len = strlen (str)) != j) return -1;

	for (i = j = 0; split[i] != 0; i++) {
		for (k = v = 0, m = split[i]; k < m; k++, j++) {
			if (m == 1) {
				if (str[j] != '-') {
					fprintf (stderr, "- expected\n");
					return -1;
				}
				continue;
			}
			
			if ((str[j] > '9') || (str[j] < '0')) {
				fprintf (stderr, "[%c] Out of range\n", str[j]);
				return -1;
			}

			v = (v * 10) + (int) (str[j] - '0');
		}

		switch (i) {
		case 0:	// Year
			if (v < 1900) {
				fprintf (stderr, "Year error (%d)\n", v);
				return -1;
			}
			tmrec.tm_year = v - 1900;
			break;
		case 1: // Month
			if ((v < 1) || (v > 12)) {
				fprintf (stderr, "Month error (%d)\n", v);
				return -1;
			}
			tmrec.tm_mon = v - 1;
			break;
		case 2: // MDay
			if ((v < 1) || (v > 31)) {
				fprintf (stderr, "Month day error (%d)\n", v);
				return -1;
			}
			tmrec.tm_mday = v;
			break;
		case 3: // -
			break;
		case 4: // Hour
			if ((v < 0) || (v > 23)) {
				fprintf (stderr, "Hour error (%d)\n", v);
				return -1;
			}
			tmrec.tm_hour = v;
			break;
		case 5: // Min
			if ((v < 0) || (v > 59)) {
				fprintf (stderr, "Min error (%d)\n", v);
				return -1;
			}
			tmrec.tm_min = v;
			break;
		}
	}

	tmrec.tm_sec   = 0;
	tmrec.tm_isdst = 0;

	return mktime (&tmrec);
}

static int pl_set_starttime (struct playlist_t *self, const char *str) {
	if ((self->pd.start = format_time (str)) == (time_t) -1) {
		return 0;
	}
	return 1;
}

static int pl_set_stoptime (struct playlist_t *self, const char *str) {
	if ((self->pd.stop = format_time (str)) == (time_t) -1) {
		return 0;
	}

	return 1;
}


static time_t pl_start_time (struct playlist_t *self) {
	return self->pd.start;
}

static time_t pl_stop_time (struct playlist_t *self) {
	return self->pd.stop;
}

static int pl_add (struct playlist_t *self, const char *fname) {
	struct strlist_t	*ptr, **pptr;
	int			rc = 1;

	if ((ptr = malloc (sizeof (struct strlist_t))) == NULL) {
		perror ("malloc");
		return -1;
	} else if ((ptr->str = strdup (fname)) == NULL) {
		free (ptr);
		perror ("strdup");
		return -1;
	}
	ptr->next = NULL;

	pptr = &self->pd.list;

	while (*pptr != NULL) {
		pptr = &((*pptr)->next);
		rc++;
	}
	*pptr = ptr;

	return rc;
}

static void pl_clean (struct playlist_t *self) {
	struct strlist_t	*p, *ptr = self->pd.list;

	self->pd.list    = NULL;
	self->pd.pointer = NULL;

	while (ptr != NULL) {
		p = ptr->next;

		if (ptr->str != NULL) free (ptr->str);
		free (ptr);
		ptr = p;
	}
}

static void pl_first (struct playlist_t *self) {
	self->pd.pointer = NULL;
}

static char *pl_next (struct playlist_t *self) {
	if (self->pd.list == NULL) return NULL;

	if (self->pd.pointer == NULL) {
		self->pd.pointer = self->pd.list;
	} else {
		if ((self->pd.pointer = self->pd.pointer->next) == NULL) {
			if (self->pd.loop_mode) {
				self->pd.pointer = self->pd.list;
			} else {
				return NULL;
			}
		}
	}

	return self->pd.pointer->str;
}

static char *pl_current (struct playlist_t *self) {
	if (self->pd.pointer == NULL) return NULL;
	return self->pd.pointer->str;
}

static int pl_looping (struct playlist_t *self, const int mode) {
	int	rc = self->pd.loop_mode;
	if (mode >= 0) self->pd.loop_mode = (mode != 0) ? 1 : 0;
	return rc;
}

static int pl_load_playlist (struct playlist_t *self, const char *file) {
	FILE	*fp;
	int	rc = 0;
	int	line, len;
	char	buffer[2048];

	if ((fp = fopen (file, "r")) != NULL) {
		self->clean (self);

		line = 0;
		while (fgets (buffer, sizeof buffer, fp) != NULL) {
			chomp (buffer);
			len = rtrim (buffer);

			if (len == 0) continue;
			if ((len == 1) && (buffer[0] == '\x1a')) continue;

			switch (++line) {
			case 1:	// begin time
				fprintf (stderr, "Begin: [%s] (%d)\n",
					buffer,
					self->set_starttime (self, buffer));
				break;
			case 2: // end time
				fprintf (stderr, "End  : [%s] (%d)\n",
					buffer,
					self->set_stoptime (self, buffer));
				break;
			default:
				fprintf (stderr, "Insert at %d [%s]\n",
					self->add (self, buffer),
					buffer);
				break;
			}
		}

		fclose (fp);
	}

	return rc;
}

static int pl_auto_playlist (struct playlist_t *self, const int id) {
	return self->load_playlist (self, self->playlist_file (self, id));
}

static char * pl_playlist_file (struct playlist_t *self, const int id) {
	char		file[2048];
	time_t		now;
	struct tm	*t;

	// fprintf (stderr, "STBID=%d\n", id);

	now = time (NULL);
	t = localtime (&now);

	sprintf (file, "%04d%02d%2d-%02d.lst",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			id % 100);

	if (self->pd.plfile != NULL) free (self->pd.plfile);
	self->pd.plfile = strdup (file);

	return self->pd.plfile;
}

static char * pl_playrecord_file (struct playlist_t *self, const int id) {
	char		file[2048];
	time_t		now;
	struct tm	*t;

	// fprintf (stderr, "STBID=%d\n", id);

	now = time (NULL);
	t = localtime (&now);

	sprintf (file, "%04d%02d%2d-%02d.rec",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			id % 100);

	if (self->pd.plrecfile != NULL) free (self->pd.plrecfile);
	self->pd.plrecfile = strdup (file);

	return self->pd.plrecfile;
}

static void pl_dispose (struct playlist_t *self) {
	self->clean (self);
	if (self->pd.plfile    != NULL) free (self->pd.plfile);
	if (self->pd.plrecfile != NULL) free (self->pd.plrecfile);
	free (self);
}

struct playlist_t * new_playlist (void) {
	struct playlist_t	*self;

	if ((self = malloc (sizeof (struct playlist_t))) != NULL) {
		self->set_starttime	= pl_set_starttime;
		self->set_stoptime	= pl_set_stoptime;
		self->start_time	= pl_start_time;
		self->stop_time		= pl_stop_time;
		self->load_playlist	= pl_load_playlist;
		self->auto_playlist	= pl_auto_playlist;
		self->playlist_file	= pl_playlist_file;
		self->playrecord_file	= pl_playrecord_file;
		self->clean		= pl_clean;
		self->add		= pl_add;
		self->looping		= pl_looping;
		self->first		= pl_first;
		self->next		= pl_next;
		self->current		= pl_current;
		self->dispose		= pl_dispose;

		self->pd.list		= NULL;
		self->pd.cnt		= 0;
		self->pd.loop_mode	= 0;
		self->pd.pointer	= NULL;
		self->pd.plfile		= NULL;
		self->pd.plrecfile	= NULL;
	}

	return self;
}
