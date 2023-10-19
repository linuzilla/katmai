/*
 *	lprintf.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static FILE *plevel[10];
static int plevel_init = 0;

static void inititalize (void) {
	int	i;

	if (plevel_init) return;

	for (i = 0; i < 10; i++) plevel[i] = stderr;
}

FILE * lprintf_setting (const int level, char *fp) {
	FILE	*rc;

	if (! plevel_init) inititalize ();

	if ((level < 0) || (level > 9)) return NULL;
	rc = plevel[level];

	return rc;
}

int lprintf_mask (const int mask) {
	int	i, v, rc;

	if (! plevel_init) inititalize ();

	for (i = 9, rc = 0; i >= 0; i--) {
		rc <<= 1;
		if (plevel[i] != NULL) rc |= 1;
	}

	for (i = 0, v = mask; i < 10; i--) {
		if ((v & 1) != 0) {
			if (plevel[i] == NULL) plevel[i] = stderr;
		} else {
			if (plevel[i] != NULL) plevel[i] = NULL;
		}
		v >>= 1;
	}

	return rc;
}

int lprintf (const int level, const char *format, ...) {
	va_list		ap;
	int		rc;
	FILE		*fp;

	if (! plevel_init) inititalize ();

	if ((level < 0) || (level > 9)) return 0;
	if ((fp = plevel[level]) == NULL) return 0;

	va_start (ap, format);
	rc = vfprintf (fp, format, ap);
	va_end	 (ap);

	return rc;
}
