/*
 *	common/rbmcommon.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __RBMCOMMON_C_
#define __RBMCOMMON_C_

static uint16_t rbm_cksum (void *packet, const int pktlen) {
	struct rbmc_header_pdu_t	*pdu = packet;
	unsigned char			*ptr = packet;
	int				i;
	uint32_t			sum = 0;

	for (i = sizeof pdu->cksum; i < pktlen; i++) {
		sum += (uint32_t) ptr[i];
		sum %= 65536;
	}

	return sum % 65536;
}

static u_int32_t get_local_address (u_int32_t mcast_group) {
	// Find a good local address for us

	int                 sockfd;
	struct sockaddr_in  addr;
	int                 addrlen = sizeof addr;
	// struct in_addr      *mcast_ptr = (struct in_addr *) &mcast_group;

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = mcast_group;
	addr.sin_port        = 0;	// htons (2000);

	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0
	    || connect (sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0
	    || getsockname (sockfd, (struct sockaddr *) &addr, &addrlen) < 0) {
		perror ("Determining local address");
		return 0;
	}

	close (sockfd);

	return addr.sin_addr.s_addr;
}

#endif
