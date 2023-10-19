/*
 *	simconf.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "misclib.h"
#include "simconf.h"

enum vartype_t {
	VARTYPE_INT = 1,
	VARTYPE_STRING = 2,
	VARTYPE_IP = 3
};


struct simconf_data_t {
	char			*str;
	enum vartype_t		type;
	void			*value;
	struct simconf_data_t	*next;
};

struct vartype_regex_t {
	char		*regex;
	enum vartype_t	vartype;
};

static struct vartype_regex_t	vartype[] = {
	{ "^[0-9]+$"				, VARTYPE_INT    },
	{ "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$"	, VARTYPE_IP	 },
	{ NULL					, VARTYPE_STRING }
};

static int strip_comments (char *buffer) {
	int	i, len;

	len = strlen (buffer);

	for (i = 0; i < len; i++) {
		if ((buffer[i] == '#') || (buffer[i] == ';')) {
			buffer[i] = '\0';
			return i;
		}
	}

	return len;
}

static int sc_load (struct simconf_t *self, const char *file) {
	FILE	*fp;
	int	len, rc = 0;
	char	buffer[2048];
	char	*ptr;

	if ((fp = fopen (file, "r")) == NULL) {
		perror (file);
		return -1;
	}

	while (fgets (buffer, sizeof buffer, fp) != NULL) {
		chomp		(buffer);
		strip_comments	(buffer);
		strip_dos_eof	(buffer);
		ltrim		(buffer);
		len = rtrim	(buffer);
		if (len == 0) continue;

		if ((ptr = strchr (buffer, '=')) == NULL) {
			fprintf (stderr, "Unknow [%s]\n", buffer);
			continue;
		}

		*ptr = '\0';
		ptr++;
		rtrim (buffer);
		ltrim (ptr);

		if (strlen (buffer) == 0) {
			fprintf (stderr, "Null variable\n");
			continue;
		}

		self->add (self, buffer, ptr);
	}

	fclose (fp);
	return rc;
}

static struct simconf_data_t * sc_search_var (
				struct simconf_t *self, const char *var) {
	struct simconf_data_t	*ptr = self->pd.data;

	while (ptr != NULL) {
		if (strcasecmp (ptr->str, var) == 0) break;
		ptr = ptr->next;
	}

	return ptr;
}

static int sc_variable_type (struct simconf_t *self, const char *var) {
	struct simconf_data_t	*ptr;

	if ((ptr = self->search_var (self, var)) == NULL) return -1;

	return ptr->type;
}

static struct simconf_data_t * sc_new_variables (const char *var) {
	struct simconf_data_t	*ptr;
	
	if ((ptr = malloc (sizeof (struct simconf_data_t))) == NULL) {
		perror ("malloc");
		return NULL;
	}

	if ((ptr->str = strdup (var)) == NULL) {
		perror ("strdup");
		free (ptr);
		return NULL;
	}

	return ptr;
}

static int sc_getint (struct simconf_t *self, const char *var) {
	struct simconf_data_t	*ptr;
	int			*iptr;

	if ((ptr = self->search_var (self, var)) != NULL) {
		// fprintf (stderr, "Found variable %s\n", var);
		if (ptr->type == VARTYPE_INT) {
			iptr = ptr->value;

			//fprintf (stderr, "Value = %d\n", *iptr);

			return *iptr;
		}
	}
	return -1;
}

static char * sc_getstr (struct simconf_t *self, const char *var) {
	struct simconf_data_t	*ptr;

	if ((ptr = self->search_var (self, var)) != NULL) {
		if (ptr->type != VARTYPE_INT) {
			return ptr->value;
		}
	}
	return NULL;
}

static char * sc_getip (struct simconf_t *self, const char *var) {
	struct simconf_data_t	*ptr;

	if ((ptr = self->search_var (self, var)) != NULL) {
		if (ptr->type == VARTYPE_IP) {
			return ptr->value;
		}
	}
	return NULL;
}

static int sc_add (struct simconf_t *self, const char *var, const char *val) {
	struct simconf_data_t	*ptr;
	int			i;
	int			*iptr;
	regex_t			preg;
	regmatch_t		pmatch[3];
	int			matched;

	if (sc_variable_type (self, var) != -1) {
		fprintf (stderr, "Variable [%s] already exists\n", var);
		return 0;
	}

	for (i = 0; vartype[i].regex != NULL; i++) {
		if (regcomp (&preg, vartype[i].regex,
					REG_EXTENDED|REG_NEWLINE) != 0) {
			continue;
		}

		matched = 0;

		if (regexec (&preg, val, 1, pmatch, 0) == 0) {
			// matched
			matched = 1;
		}

		regfree (&preg);

		if (matched) break;
	}

	if ((ptr = sc_new_variables (var)) == NULL) return 0;

	ptr->type = vartype[i].vartype;

	switch (vartype[i].vartype) {
	case VARTYPE_INT:
		iptr  = ptr->value = malloc (sizeof (int));
		*iptr = atoi (val);
		// fprintf (stderr, "AddInt (%s)(%d)\n", var, *iptr);
		break;
	case VARTYPE_IP:
		ptr->value = strdup (val);
		// fprintf (stderr, "AddIP (%s)(%s)\n", var, val);
		break;
	case VARTYPE_STRING:
		ptr->value = strdup (val);
		// fprintf (stderr, "AddStr (%s)(%s)\n", var, val);
		break;
	}

	ptr->next     = self->pd.data;
	self->pd.data = ptr;

	return 1;
}

static void sc_clean (struct simconf_t *self) {
	struct simconf_data_t	*p, *q;

	p = self->pd.data;

	self->pd.data = NULL;

	while (p != NULL) {
		q = p->next;
		free (p->str);
		free (p->value);
		free (p);
		p = q;
	}
}

static void sc_dispose (struct simconf_t *self) {
	self->clean (self);
	free (self);
}

struct simconf_t * new_simconf (void) {
	struct simconf_t	*self;

	if ((self = malloc (sizeof (struct simconf_t))) != NULL) {
		self->load		= sc_load;
		self->add		= sc_add;
		self->dispose		= sc_dispose;
		self->search_var	= sc_search_var;
		self->getstr		= sc_getstr;
		self->getint		= sc_getint;
		self->getip		= sc_getip;
		self->clean		= sc_clean;
		self->variable_type	= sc_variable_type;

		self->pd.data	= NULL;
	}

	return self;
}
