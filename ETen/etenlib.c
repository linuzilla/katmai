#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "etenlib.h"

#ifndef INCLUDE_FONT15
#define INCLUDE_FONT15		0
#endif

// file handle for ET font files
static unsigned char	pat [72];
static int		ffont24 = -1, hfont24 = -1, fspc24 = -1, fsfs24 = -1;
static int		ffont15 = -1, hfont15 = -1, fspc15 = -1, fsfs15 = -1;
static int		f24fd[4] = { -1, -1, -1, -1 };
static int		f15fd[4] = { -1, -1, -1, -1 };
static const char	*fontfname[] = {
				"stdfont", "ascfont", "spcfont", "spcfsupp"
			};

static int close_ETen_fonts (void) {
	int	i;

	for (i = 0; i < 4; i++) {
		if (f24fd[i] > 0) close (f24fd[i]);
		if (f15fd[i] > 0) close (f15fd[i]);
		f24fd[i] = f15fd[i] = -1;
	}
	return 0;
}

static int init_ETen_fonts (const char *path) {
	char	filename [256];
	int	i;

	if (strlen (path) > 200) return 0;

	for (i = 0; i < 4; i++) {
		sprintf (filename, "%s/%s.%d", path, fontfname[i], 24);

		if ((f24fd[i] = open (filename, O_RDONLY)) < 0)
			return close_ETen_fonts ();

#if INCLUDE_FONT15
		sprintf (filename, "%s/%s.%d", path, fontfname[i], 15);

		if ((f15fd[i] = open (filename, O_RDONLY)) < 0)
			return close_ETen_fonts ();
#endif
	}

	ffont24 = f24fd[0];	ffont15 = f15fd[0];
	hfont24 = f24fd[1];	hfont15 = f15fd[1];
	fspc24  = f24fd[2];	fspc15  = f15fd[2];
	fsfs24  = f24fd[3];	fsfs15  = f15fd[3];

	return 1;
}


char *ET_half_font24 (unsigned char bp) {
	lseek (hfont24, 48L * (long) bp, SEEK_SET);
	read  (hfont24, pat, 48);
	return pat;
}

char *ET_half_font15 (unsigned char bp) {
#if INCLUDE_FONT15
	lseek (hfont15, 15L * (long) bp, SEEK_SET);
	read  (hfont15, pat, 15);
#endif
	return pat;
}

static int isbig5 (const int hi, const int lo) {
	return  ((hi >= 0x81 && hi <= 0xFE) &&
		((lo >= 0x40 && lo <= 0x7e) || (lo >= 0xA1 && lo <= 0xFE)))
		? 1 : 0;
}

static int chcode (unsigned char hbp, unsigned char lbp) {
	int   codep;
	int   lt, ht;

	lt = (int)(lbp);
	ht = (int)(hbp);

	codep = (int) hbp * 256 + (int) lbp;

	if (ht < 129) {
		codep = ht;
	} else {
		if (ht < 164) {
			if (lt >= 161) lt -= 34;
			codep = 157 * (ht - 161) + lt - 64;
		} else if (codep >= 0xC6A1 && codep <= 0xC8FE) {
			if (lt >= 161) lt -= 34;
			codep = 157 * (ht - 0xC6) + lt - 64 - 161;
		} else {
			if (lt >= 161) lt -= 34;
			codep = 157 * (ht - 164) + lt - 64;
			// if (codep >= 5809) codep -= 408;
		}
	}

	return codep;
}

static char *ET_full_font24 (unsigned char hbp, unsigned char lbp) {
	int   cnum = hbp * 256 + lbp;

	if (cnum >= 0xA140 && cnum <= 0xA3FE) {        /* spcfont  */
		lseek (fspc24, 72L * (long) (chcode(hbp, lbp)), SEEK_SET);
		read  (fspc24, pat, 72);
	} else if (cnum >= 0xC6A1 && cnum <= 0xC8FE) { /* spcfsupp */
		lseek (fsfs24, 72L * (long) (chcode(hbp, lbp)), SEEK_SET);
		read  (fsfs24, pat, 72);
	} else {                                       /* stdfont  */
		lseek (ffont24, 72L * (long) (chcode(hbp, lbp)), SEEK_SET);
		read  (ffont24, pat, 72);
	}
	return pat;
}

static char *ET_full_font15 (unsigned char hbp, unsigned char lbp) {
#if INCLUDE_FONT15
	int   cnum = hbp * 256 + lbp;

	if (cnum >= 0xA140 && cnum <= 0xA3FE) {        /* spcfont  */
		lseek (fspc15, 30L * (long) (chcode(hbp, lbp)), SEEK_SET);
		read  (fspc15, pat, 30);
	} else if (cnum >= 0xC6A1 && cnum <= 0xC8FE) { /* spcfsupp */
		lseek (fsfs15, 30L * (long) (chcode(hbp, lbp)), SEEK_SET);
		read  (fsfs15, pat, 30);
	} else {                                       /* stdfont  */
		lseek (ffont15, 30L * (long) (chcode(hbp, lbp)), SEEK_SET);
		read  (ffont15, pat, 30);
	}
#endif
	return pat;
}

struct ETlib_t *new_etenlib (void) {
	struct ETlib_t	*self;

	if ((self = malloc (sizeof (struct ETlib_t))) != NULL) {
		self->open	= init_ETen_fonts;
		self->close	= close_ETen_fonts;
		self->isbig5	= isbig5;
		self->ffont24	= ET_full_font24;
		self->hfont24	= ET_half_font24;
		self->ffont15	= ET_full_font15;
		self->hfont15	= ET_half_font15;
	}

	return self;
}
