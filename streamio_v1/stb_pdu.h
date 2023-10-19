/*
 *	stb_pdu.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

struct stb_pdu_t {
	int	checksum;
	int	mtv_id;
	int	block;		// 0 - n block (block size = 32k)
	int	section;	// 0 (all), 1-24
	int	priority;	// for QoS
};
