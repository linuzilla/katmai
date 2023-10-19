#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

int local_addr (const int sock, const char *intf, struct in_addr *myaddr) {
	if (intf != NULL) {
		struct ifreq		if_data;
		struct sockaddr_in	*addr;
		int			sockfd;
		// struct in_addr	inaddr;

		if (sock < 0) {
			if ((sockfd = socket (AF_INET, SOCK_DGRAM,
							IPPROTO_UDP)) < 0) {
				perror ("socket");
				return 0;
			}
		} else {
			sockfd = sock;
		}

		// fprintf (stderr, "sock(%d), interface %s\n", sockfd, intf);

		strncpy (if_data.ifr_name, intf, IFNAMSIZ);

		if (ioctl (sockfd, SIOCGIFADDR, &if_data) < 0) {
			perror (intf);
			return 0;
		}

		addr = (struct sockaddr_in *) &if_data.ifr_ifru.ifru_addr;

		// fprintf (stderr, "Using %s\n", inet_ntoa(addr->sin_addr));
		*myaddr = addr->sin_addr;

		if (sock < 0) close (sockfd);
	} else {
		return 0;
	}

	return 1;
}
