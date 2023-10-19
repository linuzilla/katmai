/*
 *	holktv_pdu.h
 *
 *	Copyright (c) 2002-2003, Jiann-Ching Liu
 */

#ifndef __HOLKTV_PDU_H__
#define __HOLKTV_PDU_H__

// STB ¶Ç¦^ªº¿ù»~½X
#if 0

#define STBerr_NOTHING		(-1)
#define STBerr_NORMAL		0
#define STBerr_VODERR		1
#define STBerr_STBERR		2
#define STBerr_NEEDKEY		3
#define STBerr_NOFILE		4
#define STBerr_IOERR		5
#define STBerr_DECRYPTERR	6
#define STBerr_PLAYFAIL		7

#define STBstate_WAITING	'H'
// #define STBstate_PREPARE	'J'
#define STBstate_PLAYING	'K'
#define STBstate_PAUSE		'P'
#define STBsterr_ERROR		'N'

#define MASTERcmd_SEND_KEY	'E'
#define MASTERcmd_CLEAR_KEY	'X'
#define MASTERcmd_STB_STATE	'S'
#define MASTERcmd_PLAY		'K'
#define MASTERcmd_CUT		'T'
#define MASTERcmd_PAUSE		'P'
#define MASTERcmd_CONTINUE	'R'
#define MASTERcmd_AUTOREPEAT	'A'
#define MASTERcmd_NoAUTOREPEAT	'a'
#define MASTERcmd_SetPUBLIC	'B'
#define MASTERcmd_ClearPUBLIC	'b'
#define MASTERcmd_REBOOT	'O'
#define MASTERcmd_MOUNT_SERVER1	'M'
#define MASTERcmd_MOUNT_SERVER2 'm'
#define MASTERcmd_VOLUME	'v'
#define MASTERcmd_VOLUME_WB	'V'
#define MASTERcmd_CLEAR_BUFUDN	'C'
#define MASTERcmd_READ_BUFUDN	'c'
#define MASTERcmd_PLAY_VOCAL	'F'

#define MASTERcmd_STM_DELAY	'D'
#define MASTERcmd_UMOUNT_NFS	'u'
#define MASTERcmd_TERMINATE	'z'
#define MASTERcmd_SHUTDOWN	'!'
#define MASTERcmd_BUFFER_SIZE	'#'
#define MASTERcmd_BUF_LOW_LIM	'L'
#define MASTERcmd_STB_VERSION	'%'
#define MASTERcmd_BLOCK_TIME	':'

// ------------------------------------------

#define MASTERcmdVOD_STATE		'S'
#define MASTERcmdVOD_TERMINATE		'T'
#define MASTERcmdVOD_STOP_SERVICES	's'
#define MASTERcmdVOD_CONTINUE_SERVICES	'c'
#define MASTERcmdVOD_WRITE_REQUEST	'W'
#define MASTERcmdVOD_WRITE_FINISH	'O'
#define MASTERcmdVOD_EXEC_SHELL		'x'
#define MASTERcmdVOD_SYNC_VOD		'Y'
#define MASTERcmdVOD_SYNC_LOCAL		'L'
#define MASTERcmdVOD_SET_MAX_PACKETS	'M'
#define MASTERcmdVOD_EXECUTE		'X'
#define MASTERcmdQOS_THRESHOLD		'Q'
#define MASTERcmdVOD_IPMASKGATE		'I'
#define MASTERcmdVOD_SETTIME		't'
#define MASTERcmdVOD_VERSION		'%'

#endif

#define HKTV_CMD_RETURN_NOHANDLER	-1
#define HKTV_CMD_RETURN_NORMAL		0
#define HKTV_CMD_RETURN_BLOCKING	1
#define HKTV_CMD_RETURN_NON_BLOCKING	2
#define HKTV_CMD_RETURN_TERMINATE	3
#define HKTV_CMD_RETURN_REBOOT		4
#define HKTV_CMD_RETURN_SHUTDOWN	5



#define HOLIDAY_DEFAULT_SONG_LEN	6

struct holktv_cmd_pdu {
	char	lead;
	char	checksum[2];
	char	cmd;
	char	echo;
	char	len;
	char	song[HOLIDAY_DEFAULT_SONG_LEN];
	char	queue[HOLIDAY_DEFAULT_SONG_LEN];
} __attribute__ ((__packed__));

struct holktv_cmd_reply_pdu {
	char	lead;
	char	checksum[2];
	char	state;
	char	flag;
	char	retcode[2];
	char	len;
	char	song[HOLIDAY_DEFAULT_SONG_LEN];
	char	queue[HOLIDAY_DEFAULT_SONG_LEN];
} __attribute__ ((__packed__));

#endif
