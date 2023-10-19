/*
 *	xlist.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "xlist.h"

/*
#ifndef PLAYLIST_V2_SONGLEN
#define PLAYLIST_V2_SONGLEN	12
#endif

struct playlist_v2_list_t {
	char	video[PLAYLIST_V2_SONGLEN + 1];
	char	audio[PLAYLIST_V2_SONGLEN + 1];
	int	type;		// Type 0: MPEG
				// Type 1: MPEG + Music MP3
				// Type 2: Random MPEG + Music MP3
				// Type 3: Random MPEG + Music MP3 + Vocal MP3
};
*/

struct xlist_list_t {
	void			*ptr;
	struct xlist_list_t	*prev;
	struct xlist_list_t	*next;
};

static int xlv1_front (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	int			i;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		i = (pd->front + 1) % pd->count;
		if (data != NULL) memcpy (data, pd->list[i].ptr, pd->ssize);
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);

	return rc;
}

static int xlv1_rear (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	int			i;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		i = pd->rear;
		if (data != NULL) memcpy (data, pd->list[i].ptr, pd->ssize);
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);

	return rc;
}

static int xlv1_push (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	int			i;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt < pd->count) {
		i = pd->rear = (pd->rear + 1) % pd->count;
		memcpy (pd->list[i].ptr, data, pd->ssize);
		pd->usecnt++;
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);

	return rc;
}

static int xlv1_pop (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	int			i;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		i = pd->rear;
		pd->rear = (pd->rear + pd->count - 1) % pd->count;
		if (data != NULL) memcpy (data, pd->list[i].ptr, pd->ssize);
		pd->usecnt--;
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);

	return rc;
}

static int xlv1_unshift (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	int			i;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt < pd->count) {
		i = pd->front;
		pd->front = (pd->front + pd->count - 1) % pd->count;
		memcpy (pd->list[i].ptr, data, pd->ssize);
		pd->usecnt++;
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);

	return rc;
}

static int xlv1_shift (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	int			i;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		i = pd->front = (pd->front + 1) % pd->count;
		if (data != NULL) memcpy (data, pd->list[i].ptr, pd->ssize);
		pd->usecnt--;
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);

	return rc;
}

static struct xlist_list_t * xlv2_malloc (struct xlist_t *self) {
	struct xlist_list_t	*ptr;

	pthread_mutex_lock (&self->pd.mutex);
	if (self->pd.freelist == NULL) {
		if ((ptr = malloc (sizeof (struct xlist_t))) != NULL) {
			if ((ptr->ptr = malloc (self->pd.ssize)) == NULL) {
				free (ptr);
				ptr = NULL;
			}
		}
	} else {
		ptr = self->pd.freelist;
		self->pd.freelist = ptr->next;
	}
	pthread_mutex_unlock (&self->pd.mutex);

	return ptr;
}

static void xlv2_free (struct xlist_t *self, struct xlist_list_t *ptr) {
	pthread_mutex_lock (&self->pd.mutex);
	ptr->next = self->pd.freelist;
	self->pd.freelist = ptr->next;
	pthread_mutex_unlock (&self->pd.mutex);
}

static int xlv2_front (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	struct xlist_list_t	*ptr;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		ptr = pd->head;
		if (data != NULL) memcpy (data, ptr->ptr, pd->ssize);
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);
	return rc;
}

static int xlv2_rear (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	struct xlist_list_t	*ptr;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		ptr = pd->tail;
		if (data != NULL) memcpy (data, ptr->ptr, pd->ssize);
		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);
	return rc;
}

static int xlv2_push (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	struct xlist_list_t	*ptr;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if ((ptr = xlv2_malloc (self)) != NULL) {
		memcpy (ptr->ptr, data, pd->ssize);

		ptr->next = NULL;

		if (pd->usecnt++ == 0) {
			ptr->prev = NULL;
			pd->head = pd->tail = ptr;
		} else {
			ptr->prev = pd->tail;
			pd->tail = ptr;
		}

		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);
	return rc;
}

static int xlv2_pop (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	struct xlist_list_t	*ptr;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		ptr = pd->tail;

		if (--pd->usecnt == 0) {
			pd->head = pd->tail = NULL;
		} else {
			pd->tail = ptr->prev;
			pd->tail->next = NULL;
		}

		if (data != NULL) memcpy (data, ptr->ptr, pd->ssize);

		xlv2_free (self, ptr);

		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);
	return rc;
}

static int xlv2_unshift (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	struct xlist_list_t	*ptr;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if ((ptr = xlv2_malloc (self)) != NULL) {
		memcpy (ptr->ptr, data, pd->ssize);

		ptr->prev = NULL;

		if (pd->usecnt++ == 0) {
			ptr->next = NULL;
			pd->head  = pd->tail = ptr;
		} else {
			ptr->next = pd->head;
			pd->head  = ptr;
		}

		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);
	return rc;
}

static int xlv2_shift (struct xlist_t *self, void *data) {
	struct xlist_pd_t	*pd = &self->pd;
	struct xlist_list_t	*ptr;
	int			rc = 0;

	pthread_mutex_lock (&pd->mutex);

	if (pd->usecnt > 0) {
		ptr = pd->head;

		if (--pd->usecnt == 0) {
			pd->head = pd->tail = NULL;
		} else {
			pd->head = ptr->next;
			pd->head->prev = NULL;
		}

		if (data != NULL) memcpy (data, ptr->ptr, pd->ssize);

		xlv2_free (self, ptr);

		rc = 1;
	}

	pthread_mutex_unlock (&pd->mutex);
	return rc;
}


static int xl_count (struct xlist_t *self) {
	if (self->pd.count > 0) return self->pd.count;
	return 0;
}

static int xl_freebuf (struct xlist_t *self) {
	if (self->pd.count > 0) return self->pd.count - self->pd.usecnt;
	return 0;
}

static int xl_usedbuf (struct xlist_t *self) {
	return self->pd.usecnt;
}

static void xl_dispose (struct xlist_t *self) {
	int			i;
	struct xlist_list_t	*p, *q;

	if (self->pd.count > 0) {
		if (self->pd.list != NULL) {
			for (i = 0; i < self->pd.count; i++) {
				free (self->pd.list[i].ptr);
			}

			free (self->pd.list);
		}
	} else {
		for (p = self->pd.head; p != NULL; p = q) {
			q = p->next;

			if (p->ptr != NULL) free (p->ptr);
			free (p);
		}

		for (p = self->pd.freelist; p != NULL; p = q) {
			q = p->next;

			if (p->ptr != NULL) free (p->ptr);
			free (p);
		}
	}

	free (self);
}

struct xlist_t * new_xlist (const int n, const int ssize) {
	struct xlist_t		*self;
	struct xlist_pd_t	*pd;
	int			i;

	if ((self = malloc (sizeof (struct xlist_t))) != NULL) {
		pd = &self->pd;

		pd->head = pd->tail = pd->freelist = NULL;
		pd->usecnt = pd->front = pd->rear = 0;

		pthread_mutex_init (&pd->mutex, NULL);

		self->dispose		= xl_dispose;
		self->count		= xl_count;
		self->freebuf		= xl_freebuf;
		self->usedbuf		= xl_usedbuf;

		pd->ssize = ssize;

		if (n <= 0) {
			self->push	= xlv2_push;
			self->pop	= xlv2_pop;
			self->shift	= xlv2_shift;
			self->unshift	= xlv2_unshift;
			self->front	= xlv2_front;
			self->rear	= xlv2_rear;

			pd->count = 0;
		} else {
			self->push	= xlv1_push;
			self->pop	= xlv1_pop;
			self->shift	= xlv1_shift;
			self->unshift	= xlv1_unshift;
			self->front	= xlv1_front;
			self->rear	= xlv1_rear;

			pd->count = n;

			if ((pd->list = calloc (n, sizeof (struct
						xlist_list_t))) == NULL) {
				self->dispose (self);
				return NULL;
			}

			for (i = 0; i < n; i++) pd->list[i].ptr = NULL;

			for (i = 0; i < n; i++) {
				if ((pd->list[i].ptr = calloc
							(1, ssize)) == NULL) {
					self->dispose (self);
					return NULL;
				}
			}
		}
	}

	return self;
}
