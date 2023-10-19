/*
 */

#ifndef __WORDS_ASSIST_H__
#define __WORDS_ASSIST_H__

static char * dupstr (const char *s) {
	char	*r;

	// r = xmalloc (strlen (s) + 1);
	r = malloc (strlen (s) + 1);
	strcpy (r, s);
	return r;
}

static char *nthwordptr (char *cmd, const int n) {
	int		len;
	int		i, j, m;

	len = strlen (cmd);

	for (j = 1, i = m = 0; i < len; i++) {
		if (whitespace (cmd[i])) {
			if (m > 0) {
				m = 0;
				if (j++ == n) break;
			}
		} else {
			m++;
			if (j == n) return &cmd[i];
		}
	}
	return NULL;
}

static char *nthword (const char *cmd, const int n) {
	static char	word[5][MAX_CMD_WORD_LEN];
	static int	idx = 0;
	int		i, j, k, m, len;

	len = strlen (cmd);
	idx = (idx + 1) % 5;

	for (j = 1, i = k = m = 0; i < len; i++) {
		if (whitespace (cmd[i])) {
			if (m > 0) {
				m = 0;
				if (j++ == n) break;
			}
		} else {
			m++;
			if (j == n) {
				if (k < MAX_CMD_WORD_LEN-1) {
					word[idx][k++] = cmd[i];
				}
			}
		}
	}

	if (k == 0) return NULL;

	word[idx][k] = '\0';

	return word[idx];
}

static char * stripwhite (char *string) {
	char	*s, *t;

	for (s = string; whitespace (*s); s++) ;
	if (*s == 0) return s;
	t = s + strlen (s) - 1;
	while (t > s && whitespace (*t)) t--;
	*++t = '\0';
	return s;
}

static int nofwords (const char *cmd) {
	int	i, j, m, len;

	len = strlen (cmd);

	for (i = j = m = 0; i < len; i++) {
		if (whitespace (cmd[i])) {
			if (m > 0) {
				m = 0;
				j++;
			}
		} else {
			m++;
		}
	}

	if (m > 0) j++;
	return j;
} 

#endif
