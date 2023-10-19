/*
 *	mactable.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <regex.h>
#include "mactable.h"
#include "../misclib/misclib.h"

struct ipmac_table_t {
	struct in_addr	ip;
	unsigned char	mac[6];
	short		inuse;
	short		verified;
	int		idx;
};

static char *mac_address (const unsigned char *mac) {
	static char	macbuf[2][18];
	static int	idx = 0;

	idx = (idx + 1) % 2;

	sprintf (macbuf[idx], "%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0], mac[1], mac[2],
			mac[3], mac[4], mac[5]);

	return macbuf[idx];
}


static unsigned char * macstr_pack (const char *str, unsigned char *macx) {
	static unsigned char	mbuf[2][6];
	static int		idx = 0;
	unsigned char		*mac;
	int			i, j, k, c;
	unsigned char		macv[6];
	unsigned char		v1, v2;

	// fprintf (stderr, "Internal:[%s][%s]\n", mac_address (mbuf[0]),
	//					mac_address (mbuf[1]));

	if ((mac = macx) == NULL) {
		idx = (idx + 1) % 2;
		mac = mbuf[idx];
	}

	for (i = j = k = 0, v1 = v2 = 0; i < 6; j++) {
		c = str[j];
		// fprintf (stderr, "Read:[%c] %d,%d,%d\n", c, i, j, k);
		if ((c >= '0') && (c <= '9')) {
			v1 = (c - '0');
		} else if ((c >= 'a') && (c <= 'f')) {
			v1 = (c - 'a' + 10);
		} else if ((c >= 'A') && (c <= 'F')) {
			v1 = (c - 'A' + 10);
		} else if ((c == ':') || (c == '-')) {
			if (k == 1) {
				k = 2;
			} else {
				continue;
			}
		} else if ((i == 5) && (k == 1)) {
			j--;
			k = 2;
		} else {
			fprintf (stderr, "error\n");
			return NULL;
		}

		k++;

		switch (k) {
		case 1:
			v2 = v1;
			break;
		case 3:
			v1 = v2;
			v2 = 0;
		case 2:
			macv[i++] = (v2 << 4) + v1;
			v1 = v2 = 0;
			k = 0;
			break;
		}
	}

	memcpy (mac, macv, 6);
	// fprintf (stderr, "MAC ADDRESS:[%s]\n", mac_address (mac));
	return mac;
}

static int mctbl_wakeup_all (struct mac_table_t *self) {
	struct ipmac_table_t 	*ipmac = self->pd.ipmac;
	int			i, rc = 0;

	for (i = 0; i < self->pd.num; i++) {
		if (ipmac[i].inuse == 0) continue;
		if (ipmac[i].verified == 0) {
			self->wakeup (self, i);
			rc++;
		}
	}

	return rc;
}

static int mctbl_wakeup (struct mac_table_t *self, const int n) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t 		*ipmac = pd->ipmac;
	char				cmdstr[512];

	if ((n >= 0) && (n < pd->num)) {
		if (ipmac[n].inuse == 0) return 0;

		sprintf (cmdstr, "ether-wake %s", mac_address (ipmac[n].mac));
		system (cmdstr);
	}

	return 1;
}

static int mctbl_unset_verify_all (struct mac_table_t *self) {
	struct ipmac_table_t	*ipmac = self->pd.ipmac;
	int			i, rc = 0;

	for (i = 0; i < self->pd.num; i++) {
		if (ipmac[i].inuse == 0) continue;
		if (ipmac[i].verified != 0) {
			ipmac[i].verified = 0;
			rc++;
		}
	}

	return rc;
}

static int mctbl_verify_all (struct mac_table_t *self) {
	struct ipmac_table_t 	*ipmac = self->pd.ipmac;
	int			i, rc = 0;

	for (i = 0; i < self->pd.num; i++) {
		if (ipmac[i].inuse == 0) continue;
		if (ipmac[i].verified == 0) {
			self->verify (self, i);
			if (ipmac[i].verified != 0) rc++;
		}
	}

	return rc;
}

static int mctbl_is_verify (struct mac_table_t *self, const int n) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t 		*ipmac = pd->ipmac;
	int				rc = -1;

	if ((n >= 0) && (n < pd->num)) {
		if (ipmac[n].inuse != 0) {
			rc = ipmac[n].verified ? 1 : 0;
		}
	}

	return rc;
}

static int mctbl_wakeup_unverify (struct mac_table_t *self) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t		*ipmac = pd->ipmac;
	int				i;
	int				rc = 0;

	for (i = 0; i < pd->num; i++) {
		if (ipmac[i].inuse == 0) continue;

		if (ipmac[i].verified == 0) {
			rc++;
			self->wakeup (self, i);
		}
	}

	return rc;
}

static int mctbl_verified_cnt (struct mac_table_t *self) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t		*ipmac = pd->ipmac;
	int				i, rc = 0;

	for (i = 0; i < pd->num; i++) {
		if (ipmac[i].inuse == 0) continue;
		if (ipmac[i].verified) rc++;
	}

	return rc;
}


static int mctbl_verify (struct mac_table_t *self, const int n) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t 		*ipmac = pd->ipmac;
	int				rc = 0;
	int		len, nsub, code;
	FILE		*fp;
	char		cmdstr[512];
	regex_t		preg;
	const char	*regex = "\\(([\\.0-9]+)\\) +at +([:a-fA-F0-9]+) +";
	regmatch_t	*pmatch;
	char		macstr[18];
	unsigned char	*mptr;

	if (regcomp (&preg, regex, REG_EXTENDED|REG_NEWLINE) == 0) {
		nsub = preg.re_nsub + 1;

		if ((pmatch = alloca (sizeof (regmatch_t) * nsub)) == NULL) {
			perror ("alloca");
			return 0;
		}

		while ((n >= 0) && (n < pd->num)) {

			if (ipmac[n].inuse == 0) break;

			sprintf (cmdstr, "arping -c 1 %s",
						inet_ntoa (ipmac[n].ip));

			fprintf (stderr, "MacTable[%s]\n", cmdstr);

			if ((fp = popen (cmdstr, "r")) != NULL) {
				while (fgets (cmdstr,
						sizeof cmdstr, fp) != NULL) {
					// fprintf (stderr, "PING: %s", cmdstr);
				}
				code = pclose (fp);
				code = WEXITSTATUS (code);

				// fprintf (stderr, "Return Code=%d\n", code);
				// if (code != 0) break;
			}

			// system (cmdstr);

			sprintf (cmdstr, "arp -an %s", inet_ntoa (ipmac[n].ip));
			fprintf (stderr, "MacTable[%s]\n", cmdstr);

			if ((fp = popen (cmdstr, "r")) != NULL) {
				while (fgets (cmdstr,
						sizeof cmdstr, fp) != NULL) {

					chomp (cmdstr);
					// fprintf (stderr, "[%s]\n", cmdstr);
					if (regexec (&preg, cmdstr,
							nsub, pmatch, 0) == 0) {
						len = pmatch[2].rm_eo -
								pmatch[2].rm_so;

						if (len >= sizeof macstr) 
							continue;

						strncpy (macstr,
							cmdstr +
							pmatch[2].rm_so,
							len);
						macstr[len] = '\0';

						mptr = macstr_pack (
								macstr, NULL);

						if (memcmp (mptr, ipmac[n].mac,
								6) != 0) {
							memcpy (ipmac[n].mac,
								mptr, 6);
							pd->have_changed = 1;
						}
						ipmac[n].verified = 1;
						break;
					}
				}
				pclose (fp);
			} else {
				perror (cmdstr);
			}

			break;
		}
		regfree (&preg);
	}

	return rc;
}

static int mctbl_load_table (struct mac_table_t *self, const char *file) {
	// struct mac_table_private_data_t	*pd = &self->pd;
	FILE		*fp;
	char		buffer[512];
	int		nsub;
	regex_t		preg;
	const char	*regex = "^([\\.0-9]+)[ \t]+([:a-fA-F0-9]+)";
	regmatch_t	*pmatch;
	int		len;
	char		ip[16];
	char		mac[18];

	if (regcomp (&preg, regex, REG_EXTENDED|REG_NEWLINE) != 0) {
		perror ("regcomp");
		return 0;
	}

	nsub = preg.re_nsub + 1;

	// fprintf (stderr, "RE: NSUB=%d\n", preg.re_nsub);

	if ((pmatch = alloca (sizeof (regmatch_t) * nsub)) == NULL) {
		perror ("alloca");
		regfree (&preg);
		return 0;
	}

	if ((fp = fopen (file, "r")) == NULL) {
		perror (file);
		regfree (&preg);
		return 0;
	}

	self->clear_all (self);
	self->pd.have_changed = 0;

	while (fgets (buffer, sizeof buffer, fp) != NULL) {
		chomp (buffer);
		rtrim (buffer);

		if (buffer[0] == '\0') continue;
		if ((buffer[0] == '#') || (buffer[0] == ';')) continue;

		// fprintf (stderr, "[%s]%d\n", buffer, nsub);
		if (regexec (&preg, buffer, nsub, pmatch, 0) == 0) {
			if ((pmatch[1].rm_so < 0) || (pmatch[2].rm_so < 0)) {
				continue;
			}

			len = pmatch[1].rm_eo - pmatch[1].rm_so;
			if (len >= sizeof ip) continue;

			strncpy (ip, buffer + pmatch[1].rm_so, len);
			ip[len] = '\0';

			len = pmatch[2].rm_eo - pmatch[2].rm_so;
			if (len >= sizeof mac) continue;
			strncpy (mac, buffer + pmatch[2].rm_so, len);
			mac[len] = '\0';

			// fprintf (stderr, "[%s][%s]\n", ip, mac);

			self->add (self, ip, macstr_pack (mac, NULL));
		} else {
			fprintf (stderr, "No match\n");
		}
	}

	fclose (fp);
	return 1;
}

static int mctbl_add (struct mac_table_t *self,
				const char *ip, const unsigned char *mac) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t 		*ipmac = pd->ipmac;
	int				i, f, found;
	struct in_addr			inp;
	int				rc = 0;

	if ((ip == NULL) || (mac == NULL)) {
		fprintf (stderr, "Input string is NULL\n");
		return 0;
	}

	// fprintf (stderr, "Add [%s][%s]\n", ip, mac_address (mac));

	if (inet_aton (ip, &inp) == 0) {
		fprintf (stderr, "Address not valid: %s\n", ip);
		return 0;
	}

	for (i = found = 0, f = -1; i < pd->num; i++) {
		if (ipmac[i].inuse == 0) {
			if (f < 0) f = i;
			continue;
		}

		if (ipmac[i].ip.s_addr == inp.s_addr) {
			if (memcmp (ipmac[i].mac, mac, 6) == 0) {
				fprintf (stderr,
					"IP/MAC address duplicate !!\n");
				found = 1;
				break;
			} else {
				fprintf (stderr,
					"IP address duplicate !!\n");
			}
		} else if (memcmp (ipmac[i].mac, mac, 6) == 0) {
			fprintf (stderr, "MAC address duplicate !!\n");
		}
	}

	if (! found) {
		if (f >= 0) {
			ipmac[f].inuse     = 1;
			ipmac[f].verified  = 0;
			ipmac[f].idx       = -1;
			ipmac[f].ip.s_addr = inp.s_addr;
			memcpy (ipmac[f].mac, mac, 6);
			rc = 1;
			pd->usecnt++;
		} else {
			fprintf (stderr, "Table full !!\n");
		}
	}

	return rc;
}

static int mctbl_save_table (struct mac_table_t *self, const char *file) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t 		*ipmac = pd->ipmac;
	int				i;
	int				cnt;
	FILE				*fp;

	self->verify_all (self);

	for (i = cnt = 0; i < pd->num; i++) if (ipmac[i].inuse != 0) cnt++;
	if (cnt == 0) return 0;

	if ((fp = fopen (file, "w")) == NULL) {
		perror (file);
		return 0;
	}

	for (i = 0; i < pd->num; i++) {
		if (ipmac[i].inuse == 0) continue;

		fprintf (fp, "%-18s %s\n",
				inet_ntoa (ipmac[i].ip),
				mac_address (ipmac[i].mac));
	}

	fclose (fp);

	return 1;
}

static void mctbl_clear_all (struct mac_table_t *self) {
	struct mac_table_private_data_t	*pd = &self->pd;
	int				i;

	pd->usecnt = 0;

	for (i = 0; i < pd->num; i++) {
		pd->ipmac[i].verified = 0;
		pd->ipmac[i].inuse    = 0;
		pd->ipmac[i].idx      = -1;
	}
}

static int mctbl_is_changed (struct mac_table_t *self) {
	return self->pd.have_changed ? 1 : 0;
}

static void mctbl_clear_change (struct mac_table_t *self) {
	self->pd.have_changed = 0;
}

static void mctbl_external_call (struct mac_table_t *self,
			void *caller,
			void (*callback)(void *, const int, const char *,
					const char *, int *)) {
	struct mac_table_private_data_t	*pd = &self->pd;
	struct ipmac_table_t 		*ipmac = pd->ipmac;
	int				i;

	for (i = 0; i < pd->num; i++) {
		if (ipmac[i].inuse == 0) continue;

		callback (caller, i, inet_ntoa (ipmac[i].ip),
				mac_address (ipmac[i].mac), &ipmac[i].idx);
	}
}

struct mac_table_t * new_mac_table (const int n) {
	struct mac_table_t		*self;
	struct mac_table_private_data_t	*pd;

	if ((self = malloc (sizeof (struct mac_table_t))) != NULL) {
		pd = &self->pd;

		self->clear_all		= mctbl_clear_all;
		self->load_table	= mctbl_load_table;
		self->save_table	= mctbl_save_table;
		self->verify_all	= mctbl_verify_all;
		self->verify		= mctbl_verify;
		self->is_verify		= mctbl_is_verify;
		self->unset_verify_all	= mctbl_unset_verify_all;
		self->verified_cnt	= mctbl_verified_cnt;
		self->wakeup_unverify	= mctbl_wakeup_unverify;
		self->add		= mctbl_add;
		self->wakeup		= mctbl_wakeup;
		self->wakeup_all	= mctbl_wakeup_all;
		self->is_changed	= mctbl_is_changed;
		self->clear_change	= mctbl_clear_change;
		self->external_call	= mctbl_external_call;

		if ((pd->ipmac = calloc (n,
				sizeof (struct ipmac_table_t))) == NULL) {
			free (self);
			self = NULL;
		} else {
			pd->num    = n;
			self->clear_all (self);
		}
	}

	return self;
}
