/*
 *	vodstb.h
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#ifndef __VOD_STB_H__
#define __VOD_STB_H__

#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif

#if (GCC_VERSION < 2096)
// typedef u_int32_t	in_addr_t;
#endif

#define DEFAULT_FS_MCAST_ADDR		"225.1.1.1"
#define DEFAULT_FS_MCAST_PORT		6150

int local_addr (const int sock, const char *intf, struct in_addr *myaddr);
int fork_and_exec (void (*func)(void));


// -----------------------------------------------------------------------
//
/*
#define MCAST_PDU_CS_FTFS	1
#define MCAST_PDU_CS_STB	2
#define MCAST_PDU_CS_MONITOR	3

struct stb_fsfs_mcast_pdu {
	u_int8_t	cs;
	u_int8_t	cmd;
};
*/

#ifndef MAX_MPEG_FILENAME_LEN
#define MAX_MPEG_FILENAME_LEN		20	
#endif

#define STBVOD_PDU_CMD_FINDSERVER	0
#define STBVOD_PDU_CMD_REGIST		1
#define STBVOD_PDU_CMD_OPENFILE		2
#define STBVOD_PDU_CMD_FETCH		3
#define STBVOD_PDU_CMD_CLOSEFILE	4
#define STBVOD_PDU_CMD_VODSTOP		5
#define STBVOD_PDU_CMD_NEED_REGIST	6
#define STBVOD_PDU_CMD_FETCH_RESPONSE	7
#define STBVOD_PDU_CMD_NEED_REOPEN	8
#define STBVOD_PDU_CMD_FAST_FETCH	9

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
	off_t		filesize;
	int		stbid;
	char		file[MAX_MPEG_FILENAME_LEN + 1];
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

struct stb_upgrade_pdu_t {
	int	version;
	off_t	file_size;
};

#endif
