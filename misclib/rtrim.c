/*
 *	rtrim.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <string.h>

int rtrim (char *buffer) {
	int	i, len;

	len = strlen (buffer);

	for (i = len - 1; i >= 0; i--) {
		if (buffer[i] == ' ' || buffer[i] == '\t') {
			buffer[len = i] = '\0';
		} else {
			break;
		}
	}

	return len;
}
