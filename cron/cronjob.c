/*
 *	cronjob.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cronjob.h"

#define DBGP2(x,y)
// #define DBGP(x,y)	fprintf (stderr, x, y)
#define DBGP3(x,y,z)


static int parse_cron_string (struct cronjob_table_data_t *tbl,
							const char *str) {
	int		i, j, n = 0, f, len;
	short		state = 0;
	short		newfield = 0;
	u_int64_t	m64 = 0;
	u_int32_t	m32 = 0;
	u_int64_t	i64 = 0;
	u_int32_t	i32 = 0;
	u_int64_t	one = 1;

	len = strlen (str);

	for (i = f = 0; i < len; i++) {
		// fprintf (stderr, "Read [%c]\n", str[i]);
		switch (state) {
		case 0:
			if (newfield) {
				newfield = 0;

				switch (++f) {
				case 1:	// min
					DBGP2 ("MIN  MASK=%llx\n", i64);
					tbl->min = i64;
					break;
				case 2: // hour
					DBGP2 ("HOUR MASK=%x\n", i32);
					tbl->hour = i32;
					break;
				case 3: // day
					DBGP2 ("DAY  MASK=%x\n", i32);
					tbl->day = i32;
					break;
				case 4: // mon
					DBGP2 ("MON  MASK=%x\n", i32);
					tbl->mon = i32;
					break;
				case 5: // wday
					DBGP2 ("WDAY MASK=%x\n", i32);
					tbl->wday = i32;
					state = 6;
					break;
				}
				if (state == 6) {
					i--;
					break;
				}
			}

			i32 = 0;
			i64 = 0;

			if (str[i] == ' ' || str[i] == '\t') {
			} else if (str[i] == '*') {
				// fprintf (stderr, "Get STAR\n");
				state = 1;
			} else if (str[i] >= '0' && str[i] <= '9') {
				n = (int) (str[i] - '0');
				state = 2;
			} else {
				state = -1;
			}
			break;
		case 1:	//	*
			if (str[i] == ' ' || str[i] == '\t') {
				i32 = 0xffffffff;
				i64 = 0x7fffffffffffffff;

				// fprintf (stderr, "PreMatch\n");
				newfield = 1;
				state = 0;
			} else if (str[i] == '/') {
				n = 0;
				state = 3;
			} else {
				state = -1;
			}
			break;
		case 2: //	0-9
			if (n == 0) {
				m32 = 1;
				m64 = one;
			} else if (n < 32) {
				m32 = 1 << n;
				m64 = one << n;
			} else if (n < 63) {
				m32 = 0;
				m64 = one << n;
			} else {
				m32 = 0;
				m64 = 0;
			}

			if (str[i] == ' ' || str[i] == '\t') {
				// fprintf (stderr, "[N=%d]\n", n);
				i64 |= m64;
				i32 |= m32;

				newfield = 1;
				state = 0;
			} else if (str[i] >= '0' && str[i] <= '9') {
				n = n * 10 + (int) (str[i] - '0');
			} else if (str[i] == ',') {
				i64 |= m64;
				i32 |= m32;

				state = 4;
			}
			break;
		case 3: //	*/
			if (str[i] >= '0' && str[i] <= '9') {
				n = (int) (str[i] - '0');
				state = 5;
			} else {
				state = -1;
			}
			break;
		case 4: //	n,
			if (str[i] >= '0' && str[i] <= '9') {
				n = (int) (str[i] - '0');
				state = 2;
			} else {
				state = -1;
			}
			break;
		case 5: //	*/n
			if (str[i] == ' ' || str[i] == '\t') {
				m64 = 1;
				m32 = 1;

				for (j = 0; j < 63; j++) {
					if ((j % n) == 0) {
						if (i < 32) i32 |= m32;
						i64 |= m64;
					}
					m64 <<= 1;
					m32 <<= 1;
				}
				state = 0;
				newfield = 1;
			} else if (str[i] >= '0' && str[i] <= '9') {
				n = n * 10 + (int) (str[i] - '0');
			} else {
				state = -1;
			}
			break;
		case 6:	// ok
			if (str[i] == ' ' || str[i] == '\t') {
			} else {
				tbl->cmd = strdup (&str[i]);
				// fprintf (stderr, "[%s]\n", tbl->cmd);
				return 1;
			}
			break;
		default:
		case -1:
			return 0;
		}
	}

	return 0;
}

static void cronjob_list (struct cronjob_t *self) {
	struct cronjob_table_data_t	*ptr = self->pd.tbl;

	while (ptr != NULL) {
		fprintf (stderr, "[%s]\n", ptr->cmd);
		ptr = ptr->next;
	}
}

static int cronjob_add (struct cronjob_t *self, const char *str) {
	struct cronjob_table_data_t	tbl, *ptr;
	int				rc = 0;

	if (parse_cron_string (&tbl, str) != 0) {
		if ((ptr = malloc (sizeof tbl)) != NULL) {
			memcpy (ptr, &tbl, sizeof tbl);
			ptr->next = self->pd.tbl;
			self->pd.tbl = ptr;
			rc = 1;
			// fprintf (stderr, "%d [%s]\n", sizeof tbl, tbl.cmd);
		} else {
			perror ("malloc");
		}
	} else {
		fprintf (stderr, "CRON: [%s] PARSING ERROR\n", str);
	}

	return rc;
}

static char * cronjob_check_next (struct cronjob_t *self) {
	struct cronjob_table_data_t	*tbl = &self->pd.cktbl;
	struct cronjob_table_data_t	*ptr;

	while ((ptr = self->pd.findptr) != NULL) {
		self->pd.findptr = ptr->next;

		DBGP2 ("CHECK RUN for [%s]\n", ptr->cmd);

		if ((tbl->wday & ptr->wday) == 0) {
			DBGP3 ("Week day %x v.s. %x\n", tbl->wday,  ptr->wday);
			continue;
		}
		if ((tbl->mon  & ptr->mon)  == 0) {
			DBGP3 ("Mon %x v.s. %x\n", tbl->mon, ptr->mon);
			continue;
		}
		if ((tbl->day  & ptr->day)  == 0) {
			DBGP3 ("Day %x v.s. %x\n", tbl->day , ptr->day);
			continue;
		}
		if ((tbl->hour & ptr->hour) == 0) {
			DBGP3 ("Hour %x v.s. %x\n", tbl->hour, ptr->hour);
			continue;
		}
		if ((tbl->min  & ptr->min)  == 0) {
			DBGP3 ("Min %llx v.s. %llx\n", tbl->min, ptr->min);
			continue;
		}

		return ptr->cmd;
	}

	return NULL;
}

static char * cronjob_check_first (struct cronjob_t *self, struct tm *tmptr) {
	struct cronjob_table_data_t	*tbl = &self->pd.cktbl;
	u_int64_t			one = 1;

	tbl->min  = one << tmptr->tm_min;
	tbl->hour = 1 << tmptr->tm_hour;
	tbl->day  = 1 << tmptr->tm_mday;
	tbl->mon  = 1 << (tmptr->tm_mon + 1);
	tbl->wday = 1 << tmptr->tm_wday;

	self->pd.findptr = self->pd.tbl;

	return cronjob_check_next (self);
}

struct cronjob_t * new_cronjob (void) {
	struct cronjob_t	*self;

	if ((self = malloc (sizeof (struct cronjob_t))) != NULL) {
		self->add		= cronjob_add;
		self->list		= cronjob_list;
		self->check_first	= cronjob_check_first;
		self->check_next	= cronjob_check_next;
		self->pd.tbl	= NULL;
	}

	return self;
}
