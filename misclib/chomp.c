/*
 *	chomp.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <string.h>

int chomp (char *buffer) {
	int	i, len;

	len = strlen (buffer);

	for (i = len - 1; i >= 0; i--) {
		if (buffer[i] == '\r' || buffer[i] == '\n') {
			buffer[len = i] = '\0';
		} else {
			break;
		}
	}
	return len;
}
