/*
 *	include/rbmbuffer.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __RBM_BUFFER_H__
#define __RBM_BUFFER_H__

#define RBM_MAX_WIN_BUFFER	256

struct rbm_win_buffer_t {
	struct rbmc_pdu_t	pdu;
	volatile short		ready;
	pthread_mutex_t		mutex;
	pthread_cond_t		condi;
	int			len;
};

#endif
