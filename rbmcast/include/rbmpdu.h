/*
 *	include/rbmpdu.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __RBM_PDU_H__
#define __RBM_PDU_H__

#define RBM_MAX_TRANSMIT_DATA   1400

struct rbmc_header_pdu_t {
	u_int16_t	cksum;
	int16_t		length;
	int32_t		magic_id;
	int32_t		seq;
	u_int16_t	server_id;
	int16_t		client_id;
	int16_t		cmd;
	int16_t		win;
} __attribute__ ((__packed__));

struct rbmc_pdu_t {
	struct rbmc_header_pdu_t;
	char		data[RBM_MAX_TRANSMIT_DATA];
} __attribute__ ((__packed__));

#endif
