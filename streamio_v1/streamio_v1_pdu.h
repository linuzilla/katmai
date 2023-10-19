/*
 *	streamming io (version 2) pdu
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAMIO_V1_PDU_H__
#define __STREAMIO_V1_PDU_H__

#define STIOV1_CMD_ERROR	0
#define STIOV1_CMD_FINDSERVER	1
#define STIOV1_CMD_REGIST	2
#define STIOV1_CMD_OPENFILE	3
#define STIOV1_CMD_FETCH	4

#define STIOV1_ERR_REOPEN	1

#ifndef MAXIMUM_STIOV1_FILENAME_LEN
#define MAXIMUM_STIOV1_FILENAME_LEN	63
#endif

/*
 *	以下的數值相互之間有關係, 不宜亂定
 */

#   define DEFAULT_DISKIO_BUFFER_FAVIOR	1048576	/* 1M, 是 STIOV1_TRANS_BUFSIZE
						   的整數倍 */
#   define DEFAULT_STIOV1_TRANS_BUFSIZE	  32768	/* 假定 32K */
#   define DEFAULT_STIOV1_DISKNET_FACTOR     32	/* STIOV1_TRANS_BUFSIZE 除以
						   STIOV1_TRANS_BUFSIZE 的值 */
#   define STIOV1_CARRY_DATASIZE	   1439	/* 1425 ~ 1458 */
#   define DEFAULT_STIOV1_PACKET_MAP	     23	/* STIOV1_TRANS_BUFSIZE
						   每次 STIOV1_CARRY_DATASIZE
						   共 STIOV1_PACKET_MAP 次 */
#   define STIOV1_MAX_PACKET_MAP	23
#   define STIOV1_MAX_TRANS_BUFSIZE	32768


#define STBVOD_PDU_CMD_FINDSERVER       0
#define STBVOD_PDU_CMD_REGIST           1
#define STBVOD_PDU_CMD_OPENFILE         2
#define STBVOD_PDU_CMD_FETCH            3
#define STBVOD_PDU_CMD_CLOSEFILE        4
#define STBVOD_PDU_CMD_VODSTOP          5
#define STBVOD_PDU_CMD_NEED_REGIST      6
#define STBVOD_PDU_CMD_FETCH_RESPONSE   7
#define STBVOD_PDU_CMD_NEED_REOPEN      8
#define STBVOD_PDU_CMD_FAST_FETCH       9

struct streamming_io_v1_generic_pdu_t {
	int	cksum;
	int	sn;
	char	cmd;
	char	channel;
} __attribute__ ((__packed__));

struct streamming_io_v1_pdu_t {
	struct streamming_io_v1_generic_pdu_t;

	int	port;
	int	protocol_version;
	int	threshold;
	int	version_major;
	int	version_minor;
} __attribute__ ((__packed__));

struct stio_v1_ft_open_pdu_t {
	struct streamming_io_v1_generic_pdu_t;

	int	filecnt;
	char	filename[MAXIMUM_STIOV1_FILENAME_LEN + 1];
} __attribute__ ((__packed__));

struct stio_v1_ft_ok_pdu_t {
	struct streamming_io_v1_generic_pdu_t;

	int	filecnt;
	off_t	filesize;
	int	disk_bufsize;
	int	net_bufsize;
	int	disknet_fact;
	int	packet_map;
} __attribute__ ((__packed__));

struct stio_v1_ft_req_pdu_t {
	struct streamming_io_v1_generic_pdu_t;

	int	filecnt;
	int	ndcnt;
	int	len;
	char	map[STIOV1_MAX_PACKET_MAP];
} __attribute__ ((__packed__));

struct stio_V1_ft_data_pdu_t {
	struct streamming_io_v1_generic_pdu_t;

	int	filecnt;
	int	ndcnt;
	char	map;
	char	data[STIOV1_CARRY_DATASIZE];
} __attribute__ ((__packed__));

struct stio_v1_error_pdu_t {
	struct streamming_io_v1_generic_pdu_t;
	char	orgcmd;
	int	errcode;
	int	lerrno;
} __attribute__ ((__packed__));

/*
struct stio_V1_file_t {
	off_t	size;
};
*/

// #include <sys/types.h>
// #include <netdb.h>
// #include <netinet/in.h>

// #ifndef GCC_VERSION
// #define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
// #endif

// #if (GCC_VERSION < 2096)
// // typedef u_int32_t	in_addr_t;
// #endif

#ifndef HOLIDAY_SONG_TITLE_LEN
#define HOLIDAY_SONG_TITLE_LEN	6
#endif

// -----------------------------------------------------------------------
//

struct stb_vod_server_info_t {
};

struct stb_vod_regist_t {
	int	sn;
	int	threshold;
	int	version_major;
	int	version_minor;
};

struct stb_vod_fetch_t {
	int		sn;
	int		err;
	unsigned int	filecnt;
	int		block;
};

struct stb_vod_fetch_request_t {
	struct stb_vod_fetch_t	h;
	int			qos;
	char			map[23];
};

struct stb_vod_fetch_data_t {
	struct stb_vod_fetch_t	h;
	int			bid;
	int			len;
	char			buffer[1425];
};

struct stb_vod_fileio_t {
	int		sn;
	int		err;
	unsigned int	filecnt;
	char		file[HOLIDAY_SONG_TITLE_LEN + 1];
	off_t		filesize;
	int		stbid;
};

struct stb_vod_pdu_t {
	int		cksum;
	int		cmd;
	union {
		struct stb_vod_server_info_t	vodsvr;
		struct stb_vod_regist_t		regist;
		struct stb_vod_fetch_t		fetch;
		struct stb_vod_fileio_t		fileio;
	};
};

///////////////////////////////////////////////////////////////////////////

struct vodstb_t {
	int	(*cksum)(const void *ptr, const int len);
};

struct vodstb_t	* new_vodstb (void);

///////////////////////////////////////////////////////////////////////////

#define VODSVR_PORT		6509
#define VODSTB_PORT		6508

///////////////////////////////////////////////////////////////////////////

struct stb_upgrade_pdu_t {
	int	version;
	off_t	file_size;
};

#endif
