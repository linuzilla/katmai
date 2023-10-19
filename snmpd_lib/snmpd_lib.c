/*
 *	snmpd_lib.c
 *
 *	Copyright (c) 2004, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
// #include <net-snmp/agent/mib_module_config.h>
// #include <net-snmp/agent/mib_modules.h>
// #include <net-snmp/agent/snmp_vars.h>
// #include <net-snmp/agent/agent_trap.h>
// #include <net-snmp/agent/table.h>
// #include <net-snmp/agent/table_iterator.h>
#include <net-snmp/version.h>
#include <pthread.h>
#include "snmpd_lib.h"

#define TIMETICK		500000L

extern void init_system_mib (void);

struct variable2_list_t {
	struct variable2	var2;
	struct variable2_list_t	*next;
};

static pthread_t		recv_thr;
struct snmpd_lib_t		snmpd_lib_instance;
struct snmpd_lib_t		*snmptr = NULL;
static int			port = 161;
static char			*snmp_prog = NULL;
static volatile int		running = 1;

static oid			snmpd_var_oid[] = { SNMP_OID_ENTERPRISES, 0 };
static struct variable2		*var2 = NULL;
static struct variable2_list_t	*list = NULL;

#if 0
static void print_oid (oid *name, size_t length) {
	int	i = 0;

	for (i = 0; i < length; i++) {
		printf (".%lu", name[i]);
	}
	printf ("\n");
}
#endif

static int set_enterprises_oid (const int e) {
	int	i = sizeof snmpd_var_oid / sizeof (oid);

	snmpd_var_oid[i - 1] = e;
	// print_oid (snmpd_var_oid, 7);

	return 1;
}

static int init_local_mibs (const char *ptr) {
	REGISTER_MIB (ptr, var2, variable2, snmpd_var_oid);

	return 1;
}

static int initialize_snmpd (const char *prog) {
	if (snmp_prog == NULL) {
		snmp_prog = strdup (prog);
	}

	return 1;
}

static void * receieve (void *args) {
	int		numfds;
	fd_set		readfds, writefds, exceptfds;
	struct timeval	now, *nvp = &now;
	struct timeval	timeout, *tvp = &timeout;
	struct timeval	sched, *svp = &sched;
	int		block, count;

	gettimeofday (nvp, (struct timezone *) NULL);

	gettimeofday(nvp, (struct timezone *) NULL);
	svp->tv_usec = nvp->tv_usec + TIMETICK;
	svp->tv_sec = nvp->tv_sec;

	while (svp->tv_usec >= ONE_SEC) {
		svp->tv_usec -= ONE_SEC;
		svp->tv_sec++;
	}

	// set_an_alarm ();
	
	while (running) {
		tvp = &timeout;
		tvp->tv_sec = 1;
		tvp->tv_usec = TIMETICK;

		numfds = 0;

		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_ZERO (&exceptfds);

		block = 0;
		snmp_select_info (&numfds, &readfds, tvp, &block);


		if (block == 1) {
			// fprintf (stderr, "block without timeout\n");
			// tvp = NULL;         /* block without timeout */
		}

		if ((count = select (numfds, &readfds,
					&writefds, &exceptfds, tvp)) > 0) {
			// snmp_log (LOG_INFO, "Received SNMP packet(s)\n");
			snmp_read (&readfds);
		} else {
			switch (count) {
			case 0:
				// snmp_log (LOG_INFO, "Timeout\n");
				snmp_timeout();
				break;
			case -1:
				if (errno == EINTR) {
					continue;
				} else {
					snmp_log_perror ("select");
				}
				running = 0;
				break;
			default:
				// snmp_log (LOG_ERR,
				//	"select returned %d\n", count);
				running = 0;
				break;
			}
		}

		run_alarms ();
	}

	pthread_exit (NULL);
}

static int snmpd_start (const int bind_port) {
	int	quit, ret;
	int	stderr_log = 0;
	char	*c;

	port = bind_port;

	// snmp_enable_stderrlog ();

	/*
	snmp_log (LOG_INFO,
			"NET-SNMP %s, listen on UDP port %d (Thread: %ld)\n",
			NetSnmpVersionInfo, port, pthread_self ());
	*/

	if ((c = netsnmp_ds_get_string (NETSNMP_DS_APPLICATION_ID,
						NETSNMP_DS_AGENT_PORTS))) {
		char	*astring;

		astring = malloc (strlen (c) + 2 + 16);

		if (astring == NULL) {
			exit(1);
		}

		sprintf (astring, "%s,%d", c, port);
		netsnmp_ds_set_string (NETSNMP_DS_APPLICATION_ID,
			NETSNMP_DS_AGENT_PORTS, astring);
		// fprintf (stderr, "[%s]\n", astring);
		SNMP_FREE(astring);
	} else {
		char	portstr[16];

		// fprintf (stderr, "ok\n");
		sprintf (portstr, "%d", port);
		netsnmp_ds_set_string (NETSNMP_DS_APPLICATION_ID,
					NETSNMP_DS_AGENT_PORTS, portstr);
	}

	netsnmp_ds_set_boolean (NETSNMP_DS_APPLICATION_ID,
					NETSNMP_DS_AGENT_AGENTX_MASTER, 0);
	netsnmp_ds_set_int (NETSNMP_DS_APPLICATION_ID,
					NETSNMP_DS_AGENT_AGENTX_TIMEOUT, -1);
	netsnmp_ds_set_int (NETSNMP_DS_APPLICATION_ID,
					NETSNMP_DS_AGENT_AGENTX_RETRIES, -1);


	// netsnmp_ds_set_boolean (NETSNMP_DS_APPLICATION_ID,
	//				NETSNMP_DS_AGENT_ROLE, MASTER_AGENT);
	
	if (0) {
		quit = ! netsnmp_ds_get_boolean (NETSNMP_DS_APPLICATION_ID,
				NETSNMP_DS_AGENT_QUIT_IMMEDIATELY);

		ret = netsnmp_daemonize (quit, stderr_log);
	}

	init_agent (snmp_prog);
	init_system_mib ();
	init_local_mibs ("");

	// init_vacm_vars  ();
	// SNMPv3 View-based Access Control Model
	//
	// init_mib_modules ();
	//
	init_snmp (snmp_prog);

	if (init_master_agent() != 0) {
		/*
		 * Some error opening one of the specified agent transports.
		*/
		fprintf (stderr, "fail to init_master_agent\n");
		return 0;
	}

	// Store persistent data immediately in case we crash later.
	snmp_store (snmp_prog);

	send_easy_trap(0, 0);
	// snmp_log(LOG_INFO, "NET-SNMP version %s\n", netsnmp_get_version());

	netsnmp_addrcache_initialise ();

	pthread_create (&recv_thr, NULL, receieve, NULL);

	return 1;
}

static int snmpd_stop (void) {
	running = 0;

	switch (pthread_join (recv_thr, NULL)) {
	case ESRCH:
		fprintf (stderr, "No thread could be found corresponding "
			"to that specified by th.\n");
		break;
	case EINVAL:
		fprintf (stderr, "The th thread has been detached.\n");
	//	break;
	// case EINVAL:
		fprintf (stderr, "Another thread is already waiting on "
			"termination of th.\n");
		break;
	case EDEADLK:
		fprintf (stderr, "The th argument refers to the "
			"calling thread.\n");
		break;
	default:
		// fprintf (stderr, "SNMP terminate!\n");
		break;
	}

	snmp_shutdown (snmp_prog);
	shutdown_master_agent ();
	shutdown_agent ();

	return 1;
}

struct snmpd_lib_t * init_snmpd_lib (const char *name) {
	if (snmptr == NULL) {
		snmptr	= &snmpd_lib_instance;

		snmptr->start			= snmpd_start;
		snmptr->stop			= snmpd_stop;

		snmptr->set_enterprises_oid	= set_enterprises_oid;
		// snmptr->finalized_mib	= finalized_mib;

		initialize_snmpd (name);
	}

	// printf ("[%s]\n", NetSnmpVersionInfo);
	// printf ("[%s]\n", netsnmp_get_version ());

	return snmptr;
}
