/*****************************************************************************/

/*
 *      handlemedia.h -- REALmagic simple windowing application in GTK
 *
 *      Copyright (C) 2001 Sigma Designs
 *                    written by Pascal Cannenterre <pascal_cannenterre@sdesigns.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#ifndef __HANDLEMEDIA_H__
#define __HANDLEMEDIA_H__

#include "cregset.h"
#include "osdsupport.h"

//Defines for the domains 
#define STOP_STATE	0x01
#define VMGM_DOM	0x03
#define VTSM_DOM	0x04

#define HDTV_PAR_SIZE 9

typedef struct tagINFO {
	int	symbol;
	BYTE	test;
	BYTE	maddie;
	int	playing;
        BYTE	bDiscType;
        BYTE    bVCDsubtype;
        /***LOCALFILE stuff***/
        char * sFilename;
	THREADHANDLE pFileplayer;
        DWORD dwBufferSize;
        DWORD dwNBuffer;
        DWORD dwStreamtype;
        DWORD dwBitrate;
        DWORD dwNFrames;
        DWORD dwRatio;
        BYTE loop_is_over;
        /***end of LOCALFILE stuff***/
        int b_stop;
        int b_resume;
        int b_vga;
        /***streaming stuff***/
        char * sUrl;
        char bserver;
        WORD iRTPPort;
        DWORD dwCurrentPos;
        BYTE bFFSpeed;
        BYTE bFRSpeed;
        /***end of streaming stuff***/
        /***CDDA stuff***/
        DWORD *CDDATrackDuration;
        DWORD previous_trackTime;
        BYTE CDDAtracknumber;
        BYTE CDDAcurrenttrack;
        /***end of CDDA stuff***/
        // MESSAGE_QUEUE_OBJECT osdMsgQ;
        // MESSAGE_QUEUE_OBJECT guiMsgQ
	BYTE bABRepeat;
	long bRepeat;
/*
	WORD	wFSpeed;
	WORD	wBSpeed;
*/
        BYTE   bTitleTotal;
        BYTE   titleAngle[MAX_TT];
        WORD   titleChapter[MAX_TT];
	
        BYTE    bAudioStreamTotal;
        WORD    AudioStreamLangage[MAX_AST];
        WORD    AudioStreamLangageExt[MAX_AST];
        BYTE    AudioStreamCodingMode[MAX_AST];
        BYTE    AudioStreamChannel[MAX_AST];
        BYTE    AudioStreamType[MAX_AST];
        BOOL    AudioStreamAvailable[MAX_AST];
  
        BYTE    bSPStreamTotal;
        WORD    SPStreamLangage[MAX_SP];
        WORD    SPStreamLangageExt[MAX_SP];
        BOOL    SPStreamAvailable[MAX_SP];

        BYTE    bTitle;
        WORD    bChapter;
        BYTE	bAudioStream;
        BYTE	bAngleNumber;
	BYTE	bSPStream;
        BOOL    SPStreamOn;
        DWORD   tv_format;
        /*** remote control ***/
        THREADHANDLE    pRemoteListener;
	PORTHANDLE	hPort;
        /*** end of remote control ***/
        BYTE	bFullScreen;
	// Position
	DWORD	dwX;
	DWORD	dwY;
	DWORD	dwcX;
	DWORD   dwcY;
	BYTE	bStep;
	BYTE	bPosAction;	// PosAction
	IRma  *pIRma;
	IOsd  *pIOsd;
	DWORD totalTime;
        DWORD previous_totalTime;
	DWORD dwTime;
	IHdtv *pIHdtv;
	int   IHdtv_c;
//	BOOL  IHdtv_AR;
	BOOL  osdFlag;
	BOOL  blk;
	BOOL  bMode;
	DWORD dwSearch;
	BOOL  bSearch;
	BOOL  bFast;
//variables for the timer
//	DWORD stopTime;
//	DWORD currTime;
} INFO;

// timing stuff for the usleep func 
#define TICK 10000.0
#define LIBC_ERROR 1
#define GDB_ERROR 2

#define POS_NO_ACTION	0
#define POS_RESIZE		1
#define POS_MOVE		2
#define POS_MAX			3

#define CLOSED		       -1
#define STOPPED			0
#define STOP_FOR_RESUME	        1
#define PAUSED			2
#define PLAYING			3
#define TRICK_MODE		4
#define UNDETERMINED            5
#define TRICK_MODE_PAUSED       6
#define VCD_WAITING             7

#ifndef DRIVE_NUMBER
#define DRIVE_NUMBER            2
#endif

#define LOCALFILE               0xAB
#define RTSPURL                 0xBC

#define ROUND(x) ((DWORD)((x) + 0.5f))

DWORD MPEGon(INFO *);
void handle_Stop_Close(INFO *);
void handle_FForward(INFO *);
void handle_FRewind(INFO *);
void handle_Prev_Track(INFO *);
void handle_Next_Track(INFO *);
void handle_Play_Pause(INFO *);
void handle_Time_Search(INFO *);
void retrieveInfo(INFO *);
void MPEGoff(INFO *);

extern DWORD g_Pass;
extern DWORD g_OutputMode;
extern HDTV_PARAMS Hdtv_Params[];
extern HDTV_PARAMS Hdtv_Params_Init;
extern HDTV_PARAMS Hdtv_Params1[];


#endif /* HANDLEMEDIA_H */
