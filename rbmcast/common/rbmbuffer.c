/*
 *	common/rbmbuffer.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __RBMBUFFER_C_
#define __RBMBUFFER_C_

static int allocate_win_buffer (struct rbmcast_lib_t *self) {
	struct rbmcast_lib_pd_t		*pd = &self->pd;
	struct rbm_win_buffer_t		*ptr;
	int				i;

	if (pd->wbuf != NULL) return 0;

	if ((pd->wbuf = malloc (RBM_MAX_WIN_BUFFER *
				sizeof (struct rbm_win_buffer_t))) == NULL) {
		perror ("malloc");
		return 0;
	}

	for (i = 0; i < RBM_MAX_WIN_BUFFER; i++) {
		ptr = &pd->wbuf[i];
		ptr->ready = 0;
		pthread_mutex_init (&ptr->mutex, NULL);
		pthread_cond_init  (&ptr->condi, NULL);
	}

	pd->wb_front	= 0;
	pd->wb_rear	= 0;
	pd->wb_cnt	= 0;

	return 1;
}

static struct rbm_win_buffer_t * rbm_get_free_win_buf (
					struct rbmcast_lib_t *self,
					const u_int32_t seq) {
	struct rbmcast_lib_pd_t	*pd = &self->pd;
	struct rbm_win_buffer_t	*wbuf = pd->wbuf;
	struct rbm_win_buffer_t	*ptr = NULL;
	int			i, j;

	// ------------------------------------------------------

	pthread_mutex_lock (&pd->wb_mutex);

	if (seq != 0) {
		if (seq == pd->expect_seq) {
			if (pd->wb_cnt < RBM_MAX_WIN_BUFFER) {
				// Buffer full !!
				pd->errcode = RBMERR_BUFFER_FULL;
			} else {
				ptr = &wbuf[pd->wb_rear];
				ptr->ready = 0;
				pd->wb_rear = (pd->wb_rear + 1) %
							RBM_MAX_WIN_BUFFER;
				pd->wb_ptr = pd->wb_rear;
				pd->wb_cnt++;
			}
		} else if (seq > pd->max_expect_seq) {
			pd->errcode = RBMERR_BUFFER_NOT_READY;
		} else if (seq < pd->expect_seq) {
			pd->errcode = RBMERR_DUPLICATE_SEQ;
		} else {
			i = seq - pd->expect_seq;
			j = (pd->wb_rear + i) % RBM_MAX_WIN_BUFFER;
			ptr = &wbuf[j];

			if (ptr->dataok) {
				pd->errcode = RBMERR_DUPLICATE_SEQ;
				ptr = NULL;
			} else {
				// ptr->w
			}
		}
	} else {
		while (pd->wb_cnt < RBM_MAX_WIN_BUFFER) {
			pthread_cond_wait (&pd->wb_condi, &pd->wb_mutex);
		}

		ptr = &wbuf[pd->wb_rear];
		ptr->ready = 0;
		pd->wb_rear = (pd->wb_rear + 1) % RBM_MAX_WIN_BUFFER;

		pd->wb_ptr = pd->wb_rear;

		pd->wb_cnt++;
	}

	pthread_mutex_unlock (&pd->wb_mutex);

	// ------------------------------------------------------

	return ptr;
}

#endif
