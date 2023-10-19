#include <stdio.h>
#include "x_object.h"

int main (int argc, char *argv[]) {
	struct x_object_interface_t	*objintf;
	struct x_object_t		*obj;
	char				*str;

	objintf = init_x_object_interface ();

	if ((obj = objintf->newobj ()) == NULL) {
		perror ("objintf->newobj");
		return 1;
	}

	objintf->str_push (obj, "1sdf");
	objintf->str_push (obj, "2sdfj");
	objintf->str_push (obj, "3sdfj");
	objintf->str_pop  (obj);
	objintf->str_push (obj, "4jk");

	while ((str = objintf->str_pop (obj)) != NULL) {
		printf ("POP[%s]\n", str);
	}

	objintf->empty (obj);
	if (objintf->put (obj, "abc", "def") == 0) {
		fprintf (stderr, "Put error\n");
	}
	objintf->put (obj, "vjs", "dfief");

	printf ("[%s]\n", objintf->get (obj, "vjs"));

	objintf = objintf->cleanup ();

	return 0;
}
