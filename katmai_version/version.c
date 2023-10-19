/*
 *	version.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include "katmai.h"

int katmai_version (void) {
	char	*vstr = VERSION;
	int	major, minor;

	sscanf (vstr, "%d.%d", &major, &minor);

	return major * 100 + minor;
}
