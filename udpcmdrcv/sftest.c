#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "udpcmdrcv.h"

static struct udpcmdrcv_t	*cmdrcv = NULL;

void server (void) {
	int	id;

	id = cmdrcv->regist_addr (cmdrcv, "140.115.11.124", 6502);

	cmdrcv->listen     (cmdrcv, 6500, "eth0", 0);
	cmdrcv->regist_ftp (cmdrcv, 1);
	sleep (1);
	cmdrcv->bsend	   (cmdrcv, 6502, 1);
	cmdrcv->sendfile   (cmdrcv, id, "/etc/issue");
}

void client (void) {
	cmdrcv->listen     (cmdrcv, 6502, "eth0", 0);
	cmdrcv->regist_ftp (cmdrcv, 1);
}

int main (int argc, char *argv[]) {
	cmdrcv = new_udpcmdrcv (1, 2);

	if (fork () == 0) {
		server ();
	} else {
		client ();
	}
	sleep (100);
	cmdrcv->stop (cmdrcv);
	return 1;
}
