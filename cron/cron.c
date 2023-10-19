/*
 *	cron.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "cron.h"
#include "../misclib/misclib.h"


struct ccbk_t {
	void	(*func)(const char *str);
	char	*cmd;
};

static void interrupt (const int signo) {
	fprintf (stderr, "SIGNAL %d received\n", signo);
}

static void cron_list (struct cron_t *self) {
	struct cronjob_t	*cj = self->pd.cj;

	cj->list (cj);
}

static int cron_add (struct cron_t *self, const char *str) {
	struct cronjob_t	*cj = self->pd.cj;

	return cj->add (cj, str);
}

static int cron_load_crontable (struct cron_t *self, const char *file) {
	FILE			*fp;
	char			buffer[4096];
	int			len;
	struct cronjob_t	*cj = self->pd.cj;

	if ((fp = fopen (file, "r")) != NULL) {
		while (fgets (buffer, sizeof buffer - 1, fp) != NULL) {
			chomp (buffer);
			rtrim (buffer);

			len = strlen (buffer);

			if (len == 0) continue;
			if (buffer[0] == '#' || buffer[0] == ';') continue;

			cj->add (cj, buffer);

			// fprintf (stderr, "[%s]\n", buffer);
		}

		fclose (fp);
		return 1;
	} else {
		perror (file);
	}

	return 0;
}


static void * default_callback (void *func) {
	struct ccbk_t	*ptr = func;
	int		len;

	len = strlen (ptr->cmd);

	if ((len > 3) && (ptr->cmd[0] == '[') && (ptr->cmd[len-1] == ']')) {
		if (ptr->func != NULL) {
			ptr->cmd[len - 1] = '\0';
			ptr->func (&(ptr->cmd[1]));
		}
	} else {
		system (ptr->cmd);
	}

	free (ptr->cmd);
	free (ptr);

	pthread_exit (NULL);
}

static void * cron_main (void *ptr) {
	struct cron_t			*self = ptr;
	struct cron_private_data_t	*pd = &self->pd;
	struct cronjob_t		*cj = pd->cj;
	struct tm			*tmptr;
	time_t				now;
	int				stime;
	char				*cmd;
	pthread_t			thr;
	struct ccbk_t			*cptr;

	signal (SIGUSR2, interrupt);

	while (! pd->terminate) {
		now = time (NULL);
		tmptr = localtime (&now);

		if (tmptr->tm_min % pd->timeslot == 0) {
			if ((cmd = cj->check_first (cj, tmptr)) != NULL) {
				do {
					cptr = malloc (sizeof (struct ccbk_t));
					cptr->func = pd->callback;
					cptr->cmd  = strdup (cmd);

					pthread_create (&thr, NULL,
							default_callback, cptr);

					pthread_detach (thr);
				} while ((cmd = cj->check_next (cj)) != NULL);
			}
			// self->list (self);
		}

		now = time (NULL);
		tmptr = localtime (&now);
		stime = ((59 - tmptr->tm_min) % pd->timeslot) * 60
			+ (60 - tmptr->tm_sec);
		// fprintf (stderr, "Sleep for %d seconds\n", stime);

		while ((stime = sleep (stime)) != 0) {
			if (pd->terminate) break;
			sleep (stime);
		}
	}

	pd->on_the_run = 0;
	pthread_exit (NULL);
}

static int cron_set_timeslot (struct cron_t *self, const int min) {
	int	rc = self->pd.timeslot;

	if (min > 60) return -1;
	if (min > 0) self->pd.timeslot = min;

	return rc;
}

static int cron_run (struct cron_t *self) {
	if (self->pd.on_the_run) return 0;

	pthread_create (&self->pd.cron_thread, NULL, cron_main, self);
	self->pd.on_the_run = 1;

	return 1;
}

static int cron_stop (struct cron_t *self) {
	if (! self->pd.on_the_run) return 0;

	// fprintf (stderr, "Wait until next scheduling time coming up ...\n");
	self->pd.terminate = 1;
	pthread_kill (self->pd.cron_thread, SIGUSR2);
	pthread_join (self->pd.cron_thread, NULL);
	fprintf (stderr, "\ncron thread terminated!!\n");
	self->pd.on_the_run = 0;

	return 1;
}

void *  cron_set_callback (struct cron_t * self, void (*func)(const char *)) {
	void (*ptr)(const char *) = self->pd.callback;

	// fprintf (stderr, "Relocate callback funcion at %d\n", func);
	self->pd.callback = func;

	return ptr;
}

struct cron_t * new_cron (void) {
	struct cron_t	*self;

	if ((self = malloc (sizeof (struct cron_t))) != NULL) {
		self->load_crontable	= cron_load_crontable;
		self->add		= cron_add;
		self->set_timeslot	= cron_set_timeslot;
		self->list		= cron_list;
		self->run		= cron_run;
		self->stop		= cron_stop;
		self->set_callback	= cron_set_callback;
		self->pd.callback	= NULL;
		self->pd.on_the_run     = 0;
		self->pd.terminate	= 0;
		self->pd.timeslot	= 1;

		if ((self->pd.cj = new_cronjob ()) == NULL) {
			free (self);
			self = NULL;
		}
	}

	return self;
}

#if 0

static void local_execute (const char *cmd) {
	fprintf (stderr, "Callback function receive [%s]\n", cmd);
}

int main (int argc, char *argv[]) {
	struct cron_t	*cron;

	if ((cron = new_cron ()) == NULL) {
		perror ("new_cron");
		return 1;
	}

	cron->load_crontable (cron, "crontab");
	cron->add (cron, "* 21 * * * testing");

	cron->set_timeslot (cron, 1);
	// cron->list (cron);
	cron->run  (cron);
	sleep (10);
	cron->set_callback (cron, local_execute);
	sleep (120);
	cron->stop (cron);

	return 0;
}
#endif
