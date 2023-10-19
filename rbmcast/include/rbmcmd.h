/*
 *	include/rbmcmd.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __RBM_CMD_H__
#define __RBM_CMD_H__

#define RBMCMD_CONNECT		1
#define RBMCMD_NOT_ALLOW	2
#define RBMCMD_CONNECT_ACK	3
#define RBMCMD_CONN_FULL	4
#define RBMCMD_CONN_OK		5
#define RBMCMD_DATA		6

#define RBMST_INIT		0
#define RBMST_LISTEN		1
#define RBMST_TRANSMIT		2
#define RBMST_FINISH		3
#define RBMST_CONNECT		4
#define RBMST_WAIT_DATA		5

#define RBMERR_OK		0
#define RBMERR_BUFFER_FULL	1
#define RBMERR_BUFFER_NOT_READY	2
#define RBMERR_DUPLICATE_SEQ	3


#endif
