/*
 *	regexlib.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexlib.h"

static void rl_cleanup (struct regexlib_t *self) {
	if (self->pd.mstr != NULL) {
		int	i;

		for (i = 0; i < self->pd.mstrlen; i++) {
			if (self->pd.mstr[i] != NULL) free (self->pd.mstr[i]);
		}

		self->pd.mstrlen = 0;
		free (self->pd.mstr);
	}

	if (self->pd.regex != NULL) {
		free (self->pd.regex);
		self->pd.regex = NULL;
	}

	if (self->pd.pmatch != NULL) {
		free (self->pd.pmatch);
		self->pd.pmatch = NULL;
	}

	if (self->pd.have_preg) {
		regfree (&self->pd.preg);
		self->pd.have_preg = 0;
	}
}

static void rl_dispose (struct regexlib_t *self) {
	self->cleanup (self);
	free (self);
}

static int rl_regex (struct regexlib_t *self, const char *str) {
	int		i, n;

	self->cleanup (self);

	if (regcomp (&self->pd.preg, str, REG_EXTENDED|REG_NEWLINE) != 0) {
		fprintf (stderr, "regcomp: compiling error");
		return -1;
	}

	n = self->pd.preg.re_nsub + 1;

	if ((self->pd.pmatch = malloc (sizeof (regmatch_t) * n)) == NULL) {
		self->cleanup (self);
		perror ("malloc");
		return -1;
	}

	if ((self->pd.mstr = calloc (n, sizeof (char *))) == NULL) {
		self->cleanup (self);
		perror ("calloc");
		return -1;
	}

	self->pd.mstrlen = n;

	for (i = 0; i < self->pd.mstrlen; i++) {
		self->pd.mstr[i] = NULL;
	}

	self->pd.regex = strdup (str);

	self->pd.have_preg = 1;

	return n;
}

static int rl_exec (struct regexlib_t *self, const char *str) {
	int		i, n, len;
	regmatch_t	*pmatch = self->pd.pmatch;

	if (! self->pd.have_preg) return 0;

	n = self->pd.preg.re_nsub + 1;
	// fprintf (stderr, "n=%d\n", n);

	if (regexec (&self->pd.preg, str, n, pmatch, 0) != 0) return 0;


	for (i = 0; i < n; i++) {
		if ((pmatch[i].rm_so < 0) || (pmatch[i].rm_so < 0)) continue;

		len = pmatch[i].rm_eo - pmatch[i].rm_so;

		if (self->pd.mstr[i] == NULL) {
			free (self->pd.mstr[i]);
			self->pd.mstr[i] = NULL;
		}

		if ((self->pd.mstr[i] = malloc (len + 1)) == NULL) continue;

		// --

		memcpy (self->pd.mstr[i], &str[pmatch[i].rm_so], len);
		self->pd.mstr[i][len] = '\0';
	}

	return n;
}

static char * rl_var (struct regexlib_t *self, const int n) {
	if (n < 0 || n >= self->pd.mstrlen) return NULL;
	if (self->pd.mstr == NULL) return NULL;

	return self->pd.mstr[n];
}

struct regexlib_t * new_regex_lib (void) {
	struct regexlib_t	*self;

	if ((self = malloc (sizeof (struct regexlib_t))) != NULL) {
		self->pd.have_preg	= 0;
		self->pd.regex		= NULL;
		self->pd.pmatch		= NULL;
		self->pd.mstr		= NULL;
		self->pd.mstrlen	= 0;

		self->regex	= rl_regex;
		self->exec	= rl_exec;
		self->var	= rl_var;
		self->cleanup	= rl_cleanup;
		self->dispose	= rl_dispose;
	}

	return self;
}
