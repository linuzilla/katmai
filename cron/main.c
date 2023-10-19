/*
 *	main.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include "cron.h"

int main (int argc, char *argv[]) {
	struct cron_t	*cron;

	if ((cron = new_cron ()) == NULL) {
		perror ("new_cron");
		return 1;
	}

	return 0;
}
