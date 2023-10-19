/*
 *	ltrim.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <string.h>

int ltrim (char *buffer) {
	int	i, j, len;

	len = strlen (buffer);

	for (i = 0; i < len; i++) {
		if ((buffer[i] != ' ') && (buffer[i] != '\t')) {
			break;
		}
	}

	if (i > 0) {
		for (j = 0, len -= i; j <= len; i++, j++) {
			buffer[j] = buffer[i];
		}
	}

	return len;
}
