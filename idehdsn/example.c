/*
 *	idehdsn.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include "idehdsn.h"


int main (int argc, char *argv[]) {
	char	*model, *serial;

	if (ide_ata_disk_sn (0, &model, &serial) == 0) return 1;

	fprintf (stderr, "Model: %s\nSerial: %s\n", model, serial);

	return 0;
}
