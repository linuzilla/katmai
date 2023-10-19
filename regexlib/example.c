#include <stdio.h>
#include "regexlib.h"

int main (int argc, char *argv[]) {
	struct regexlib_t	*rex;
	int			n;
       
	if ((rex = new_regex_lib ()) == NULL) return 1; 

	while ((n = rex->regex (rex, "([0-9]+)\\.([0-9]+)")) > 0) {
		fprintf (stderr, "%d\n", n);
		if (rex->exec (rex, " 140.115.1.254") > 0) {
			fprintf (stderr, "[%s][%s][%s]\n",
					rex->var(rex, 0),
					rex->var(rex, 1),
					rex->var(rex, 2));
		}
		break;
	}

	rex->dispose (rex);
	return 0;
}
