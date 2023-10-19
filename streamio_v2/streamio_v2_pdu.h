/*
 *	streamming io (version 2) pdu
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAMIO_V2_PDU_H__
#define __STREAMIO_V2_PDU_H__

#define STIOV2_CMD_ERROR	0
#define STIOV2_CMD_FINDSERVER	1
#define STIOV2_CMD_REGIST	2
#define STIOV2_CMD_OPENFILE	3
#define STIOV2_CMD_FETCH	4

#define STIOV2_ERR_REOPEN	1

#ifndef MAXIMUM_STIOV2_FILENAME_LEN
#define MAXIMUM_STIOV2_FILENAME_LEN	63
#endif

/*
 *	以下的數值相互之間有關係, 不宜亂定
 */

#   define DEFAULT_DISKIO_BUFFER_FAVIOR	1048576	/* 1M, 是 STIOV2_TRANS_BUFSIZE
						   的整數倍 */
#   define DEFAULT_STIOV2_TRANS_BUFSIZE	  32768	/* 假定 32K */
#   define DEFAULT_STIOV2_DISKNET_FACTOR     32	/* STIOV2_TRANS_BUFSIZE 除以
						   STIOV2_TRANS_BUFSIZE 的值 */
#   define STIOV2_CARRY_DATASIZE	   1439	/* 1425 ~ 1458 */
#   define DEFAULT_STIOV2_PACKET_MAP	     23	/* STIOV2_TRANS_BUFSIZE
						   每次 STIOV2_CARRY_DATASIZE
						   共 STIOV2_PACKET_MAP 次 */
#   define STIOV2_MAX_PACKET_MAP	23
#   define STIOV2_MAX_TRANS_BUFSIZE	32768


struct streamming_io_v2_generic_pdu_t {
	int	cksum;
	int	sn;
	char	cmd;
	char	channel;
} __attribute__ ((__packed__));

struct streamming_io_v2_pdu_t {
	struct streamming_io_v2_generic_pdu_t;

	int	port;
	int	protocol_version;
	int	threshold;
	int	version_major;
	int	version_minor;
} __attribute__ ((__packed__));

struct stio_v2_ft_open_pdu_t {
	struct streamming_io_v2_generic_pdu_t;

	int	filecnt;
	char	filename[MAXIMUM_STIOV2_FILENAME_LEN + 1];
} __attribute__ ((__packed__));

struct stio_v2_ft_ok_pdu_t {
	struct streamming_io_v2_generic_pdu_t;

	int	filecnt;
	off_t	filesize;
	int	disk_bufsize;
	int	net_bufsize;
	int	disknet_fact;
	int	packet_map;
} __attribute__ ((__packed__));

struct stio_v2_ft_req_pdu_t {
	struct streamming_io_v2_generic_pdu_t;

	int	filecnt;
	int	ndcnt;
	int	len;
	char	map[STIOV2_MAX_PACKET_MAP];
} __attribute__ ((__packed__));

struct stio_v2_ft_data_pdu_t {
	struct streamming_io_v2_generic_pdu_t;

	int	filecnt;
	int	ndcnt;
	char	map;
	char	data[STIOV2_CARRY_DATASIZE];
} __attribute__ ((__packed__));

struct stio_v2_error_pdu_t {
	struct streamming_io_v2_generic_pdu_t;
	char	orgcmd;
	int	errcode;
	int	lerrno;
} __attribute__ ((__packed__));

/*
struct stio_v2_file_t {
	off_t	size;
};
*/

#endif
