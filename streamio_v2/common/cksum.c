
#ifndef __STIO_V2_CS_CKSUM__C__
#define __STIO_V2_CS_CKSUM__C__

static int stiov2_cs_cksum (const void *ptr, const int len) {
	const struct streamming_io_v2_pdu_t	*p = ptr;
	const unsigned char			*q = ptr;
	int					i;
	int					sum = 0;

	for (i = sizeof p->cksum; i < len; i++) {
		sum += q[i];
		sum &= 0xfffffff;
	}

	return sum;
}

#endif
