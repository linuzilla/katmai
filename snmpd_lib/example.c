#include <stdio.h>
#include <stdlib.h>
#include "snmp_lib.h"
#include "snmpd_lib.h"

int main (int argc, char *argv[]) {
	struct snmp_lib_t	*snmp;
	struct snmpd_lib_t	*snmpd;
	char			buffer[12];
	int			client = 1;
	char			*ptr;

	setenv ("LD_LIBRARY_PATH", "/usr/local/misc/net-snmp-5.1/lib", 1);

	if (client) {
		if ((snmp = new_snmp_lib ()) == NULL) {
			perror ("new_snmp_lib");
			return 1;
		}

		ptr = snmp->snmpget (snmp, "140.115.1.149", 161, "public",
					"SNMPv2-MIB::sysName.0");

		if (ptr != NULL) fprintf (stderr, "[%s](%s)\n", ptr,
				snmp->string (ptr));
		
		ptr = snmp->snmpget (snmp, "140.115.1.149", 161, "public",
					"sysName.0");
		if (ptr != NULL) fprintf (stderr, "[%s]\n", ptr);
		
		ptr = snmp->snmpget (snmp, "140.115.1.149", 161, "public",
					".1.3.6.1.2.1.43.10.2.1.4.1.1");
		if (ptr != NULL) fprintf (stderr, "[%s](%d)\n", ptr,
				snmp->int32 (ptr));

		snmp->dispose (snmp);
	} else {
		snmpd = init_snmpd_lib ("test");

		snmpd->set_enterprises_oid (90125);

		snmpd->start (3161);

		fgets (buffer, sizeof buffer, stdin);

		snmpd->stop ();
	}

	return 0;
}
