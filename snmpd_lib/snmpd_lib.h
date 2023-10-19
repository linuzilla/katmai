/*
 *	snmpd_lib.h
 *
 *	Copyright (c) 2004, Jiann-Ching Liu
 */

#ifndef __SNMPD_LIB_H__
#define __SNMPD_LIB_H__


struct snmpd_lib_t {
	int		(*start)(const int port);
	int		(*stop)(void);
	int		(*set_enterprises_oid)(const int e);
};

struct snmpd_lib_t	*init_snmpd_lib (const char *str);

#endif
