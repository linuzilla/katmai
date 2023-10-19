/*
 *	strip_dos_eof.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <string.h>

int strip_dos_eof (char *buffer) {
	int	len;

	if ((len = strlen (buffer)) > 0) {
		if (buffer[len-1] == '\x1a') buffer[--len] = '\0';
	}

	return len;
}
