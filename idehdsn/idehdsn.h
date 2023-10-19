/*
 *	idehdsn.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __IDEHDSN_H__
#define __IDEHDSN_H__

struct ide_hdsn_pd_t {
	char	model[41];
	char	serial[21];
};

int ide_ata_disk_sn (const int drvid, char **model, char **serial);

#endif
