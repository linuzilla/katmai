/*
 *	rc4encrypt.c
 */

#include "rc4encrypt.h"
#include "md5.h"

/* RC4 as implemented from a posting from
 * Newsgroups: sci.crypt
 * From: sterndark@netcom.com (David Sterndark)
 * Subject: RC4 Algorithm revealed.
 * Message-ID: <sternCvKL4B.Hyy@netcom.com>
 * Date: Wed, 14 Sep 1994 06:35:31 GMT
 */

#define LOOP(in,out) \
		x=((x+1)&0xff); \
		tx=d[x]; \
		y=(tx+y)&0xff; \
		d[x]=ty=d[y]; \
		d[y]=tx; \
		(out) = d[(tx+ty)&0xff]^ (in);

#define RC4_LOOP(a,b,i)	LOOP(*((a)++),*((b)++))

#define SK_LOOP(n) { \
		tmp=d[(n)]; \
		id2 = (data[id1] + tmp + id2) & 0xff; \
		if (++id1 == len) id1=0; \
		d[(n)]=d[id2]; \
		d[id2]=tmp; }

void RC4 (RC4_KEY *key, unsigned long len,
			const unsigned char *indata, unsigned char *outdata) {
	register RC4_INT	*d;
	register RC4_INT	x,y,tx,ty;
	int			i;
        
	x = key->x;     
	y = key->y;     
	d = key->data; 


	i = (int)(len >> 3L);

	if (i) {
		while (1) {
			RC4_LOOP (indata, outdata, 0);
			RC4_LOOP (indata, outdata, 1);
			RC4_LOOP (indata, outdata, 2);
			RC4_LOOP (indata, outdata, 3);
			RC4_LOOP (indata, outdata, 4);
			RC4_LOOP (indata, outdata, 5);
			RC4_LOOP (indata, outdata, 6);
			RC4_LOOP (indata, outdata, 7);

			if (--i == 0) break;
		}
	}

	i = (int) len & 0x07;

	if (i) {
		while (1) {
			RC4_LOOP (indata, outdata, 0); if (--i == 0) break;
			RC4_LOOP (indata, outdata, 1); if (--i == 0) break;
			RC4_LOOP (indata, outdata, 2); if (--i == 0) break;
			RC4_LOOP (indata, outdata, 3); if (--i == 0) break;
			RC4_LOOP (indata, outdata, 4); if (--i == 0) break;
			RC4_LOOP (indata, outdata, 5); if (--i == 0) break;
			RC4_LOOP (indata, outdata, 6); if (--i == 0) break;
		}
	}               

	key->x = x;     
	key->y = y;
}

/*
const char *RC4_options(void) {
	if (sizeof(RC4_INT) == 1)
		return("rc4(ptr,char)");
	else
		return("rc4(ptr,int)");
	}
	*/


void RC4_set_key (RC4_KEY *key, int len, const unsigned char *data) {
        register RC4_INT	tmp;
        register RC4_INT	*d;
        register int		id1, id2;
        unsigned int		i;
        
        d= &(key->data[0]);

	for (i = 0; i < 256; i++) d[i]=i;

        key->x = 0;     
        key->y = 0;     
        id1 = id2 = 0;     

	for (i=0; i < 256; i+=4) {
		SK_LOOP (i + 0);
		SK_LOOP (i + 1);
		SK_LOOP (i + 2);
		SK_LOOP (i + 3);
	}
}
    
