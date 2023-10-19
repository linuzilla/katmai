/*
 *	idehdsn.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include "idehdsn.h"


int ide_ata_disk_sn (const int drvid, char **m, char **s) {
	static char	model[41];
	static char	serial[21];
	char		file[128];
	int		i, j, identify[256];
	int		v[8];
	const char	*fmt = "/proc/ide/ide%d/hd%c/identify";
	FILE		*fp;

	switch (drvid) {
	case 0:
		sprintf (file, fmt, 0, 'a');
		break;
	case 1:
		sprintf (file, fmt, 0, 'b');
		break;
	case 2:
		sprintf (file, fmt, 1, 'c');
		break;
	case 3:
		sprintf (file, fmt, 1, 'd');
		break;
	default:
		return 0;
	}

	if ((fp = fopen (file, "r")) == NULL) return 0;

	j = 0;
	while (fgets (file, sizeof file, fp) != NULL) {
		sscanf (file, "%x %x %x %x %x %x %x %x",
					&v[0], &v[1], &v[2], &v[3],
					&v[4], &v[5], &v[6], &v[7]);
		for (i = 0; i < 8; i++) identify[j++] = v[i];
		if (j >= 256) break;
	}
	fclose (fp);


	for (j = 0, i = 10; i <= 19; i++) {
		serial[j++] = (char)((identify[i] >> 8) & 0xff);
		serial[j++] = (char)(identify[i] & 0xff);
	}
	serial[j] = '\0';

	for (j = 0, i = 27; i <= 46; i++) {
		model[j++] = (char)((identify[i] >> 8) & 0xff);
		model[j++] = (char)(identify[i] & 0xff);
	}
	model[j] = '\0';

	*m = model;
	*s = serial;

	return 1;
}
