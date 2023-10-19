/*
 *	stiov3_pdu.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STIO_V3_PDU_H__
#define __STIO_V3_PDU_H__

#define STIOV3_DEFAULT_FILENAME_LEN	6

#define STIOV3_PDU_CMD_FINDSERVER	0
#define STIOV3_PDU_CMD_REGIST		1
#define STIOV3_PDU_CMD_OPENFILE		2
#define STIOV3_PDU_CMD_FETCH		3
#define STIOV3_PDU_CMD_FAST_FETCH	4
#define STIOV3_PDU_CMD_CLOSEFILE	5
#define STIOV3_PDU_CMD_VODSTOP		6
#define STIOV3_PDU_CMD_NEED_REGIST	7
#define STIOV3_PDU_CMD_FETCH_RESPONSE	8
#define STIOV3_PDU_CMD_NEED_REOPEN	9

struct stiov3_server_info_t {
} __attribute__ ((__packed__));

struct stiov3_regist_t {
	int	sn;
	int	threshold;
	int	version_major;
	int	version_minor;
} __attribute__ ((__packed__));

struct stiov3_fetch_t {
	int		sn;
	char		channel;
	int		err;
	unsigned int	filecnt;
	int		block;
} __attribute__ ((__packed__));

struct stiov3_fetch_request_t {
	struct stiov3_fetch_t	h;
	int			qos;
	char			map[23];
} __attribute__ ((__packed__));

struct stiov3_fetch_data_t {
	struct stiov3_fetch_t	h;
	int			bid;
	int			len;
	char			buffer[1425];
} __attribute__ ((__packed__));

struct stiov3_fileio_t {
	int		sn;
	char		channel;
	int		err;
	unsigned int	filecnt;
	off_t		filesize;
	int		stbid;
	char		fnlen;
	char		file[STIOV3_DEFAULT_FILENAME_LEN + 1];
} __attribute__ ((__packed__));

struct stio_v3_pdu_header_t {
	int	cksum;
	int	cmd;
} __attribute__ ((__packed__));

struct stio_v3_pdu_t {
	struct stio_v3_pdu_header_t;
	union {
		struct stiov3_server_info_t	vodsvr;
		struct stiov3_regist_t		regist;
		struct stiov3_fetch_t		fetch;
		struct stiov3_fileio_t		fileio;
	};
} __attribute__ ((__packed__));

#endif
