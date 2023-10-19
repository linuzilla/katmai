#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int fork_and_exec (void (*func)(void)) {
	pid_t	pid;
	int	status;

	if ((pid = fork ()) == 0) { // child process
		(*func)();
		exit (0);
	} else if (pid > 0) {
		// waitpid (pid, &status, int options);
		wait (&status);
		return WEXITSTATUS (status);
	}
	perror ("fork");
	return -1;
}
