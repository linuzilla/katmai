/*****************************************************************************
*  FMP.h : Type Definitions for FMP implementation
*  REALmagic FMP RTOS Driver
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 10/29/98
*  Description:
*  $Archive: /ANTILLES/include/fmp.h $
*  $Date: 3/18/02 4:38p $
*****************************************************************************/

//@doc
#ifndef __FMP_h__
#define __FMP_h__

#ifdef __cplusplus
extern "C"{
#endif 

/*+-------------------------------------------------------------------------
*
*   Sigma Designs
*   Copyright 1998 Sigma Designs, Inc.
*
*--------------------------------------------------------------------------*/

#define _MAX_STRING_	256

#ifdef _DEFINE_FMP_TYPES_
typedef unsigned long	DWORD;
typedef long 		LONG;
typedef unsigned short	WORD;
typedef unsigned char	BYTE;
typedef int		BOOL;

#ifndef TRUE
#define TRUE 1
#endif // TRUE

#ifndef FALSE
#define FALSE 0
#endif // FALSE

#ifdef _UNICODE_
typedef unsigned short			TCHAR;
#else
typedef char			TCHAR;
#endif // _UNICODE_

#ifdef _BARBADOS_
typedef unsigned long long ULONGLONG;
#endif

#endif // _DEFINE_FMP_TYPES_

// Callback definition
typedef DWORD (*PFMPCALLBACK) (DWORD dwContext, DWORD dwMsg, DWORD dwValue);

typedef struct tagFMP_BUFFER{
	BYTE  *pBuffer;         // Address of the buffer		
	DWORD  dwBufferSize;    // Size of the buffer
	DWORD  dwDataSize;      // Size of valid data in the buffer
	DWORD  dwFlags;         // Flags
	DWORD  dwFlagsEx;		// Extended flags
	DWORD  pReserved[8];    // Reserved
} FMP_BUFFER, *PFMP_BUFFER;

//@struct FMP_OPENSTRUCT | Parameter sent to the Graph Manager upon opening
typedef struct tagFMP_OPENSTRUCT{
	DWORD			dwStructSize;	// @field Size of the structure
	DWORD			dwFlags;		// @field Open Flag
									// @flag FMPF_TRANSPORT | Transport stream demux
									// @flag  FMPF_SYSTEM   | System stream demux 
									// @flag FMPF_PROGRAM     | Program stream demux 
									// @flag FMPF_VIDEO       | Video stream (MPEG1/MPEG2) 
									// @flag FMPF_MPEG_AUDIO  | MPEG Audio stream 
									// @flag FMPF_AC3         | AC3 Audio stream 
									// @flag FMPF_DVD         | DVD demux 
									// @flag FMPF_SVCD        | VCD/SVCD demux 
									// @flag FMPF_DISCPLAYBACK  0xFFFFFFFF | 

	DWORD			dwSize;			// @field Size of buffers
	DWORD			dwCount;		// @field Number of buffers
	PFMPCALLBACK	pCallback;		// @field FMP Callback
	DWORD			dwContext;		// @field Callback context
	BYTE			bDriverNumber;	// @field Driver Number
	TCHAR*			sFileName;		// @field File Name
	BYTE			bFileSystem;	// @field disk data access method 
									// @flag SYS_ACCESS | using the OS function
									// @flag UDF_ACCESS | using UDF
									// @flag ISO_ACCESS | using ISO9660
	BYTE			bDiscContent;	// @field Disc Content (returned by the driver)
									// @flag FMPC_DVD_VIDEO	| DVD Video Disc 
									// @flag FMPC_VCD		| Video CD Disc 
									// @flag FMPC_SVCD		| Super Video CD Disc 

} FMP_OPENSTRUCT, *PFMP_OPENSTRUCT;

// FMP Message Structure
typedef struct tagFMPmsg{
	DWORD	dwCommand;
	DWORD	dwArg1;
	DWORD	dwArg2;
	DWORD	dwArg3;
	DWORD	dwArg4;
	DWORD	dwArg5;
	BYTE	pBuffer[sizeof (FMP_OPENSTRUCT) + _MAX_STRING_];
} FMP_MSG, *PFMP_MSG;

// Open flags 
#define FMPF_TRANSPORT		0 /* Transport stream demux. */ 
#define FMPF_SYSTEM			1 /* System stream demux */
#define FMPF_PROGRAM		2 /* Program stream demux */
#define FMPF_VIDEO			3 /* Video stream (MPEG1/MPEG2) */
#define FMPF_MPEG_AUDIO		4 /* MPEG Audio stream */
#define FMPF_AC3			5 /* AC3 Audio stream */
#define FMPF_DVD			6 /* DVD demux */
#define FMPF_SVCD			7 /* SVCD/VCD demux */
#define FMPF_CDDA			8 /* CDDA demux */
#define FMPF_DRIVEPOLLING	9 /* minimum graphmanager for Drive polling only */
#define FMPF_PES			10 /* pes stream support */
#define FMPF_AAC_CELP		11 /* AAC CELP stream support */
#define FMPF_PES_MPEG4		12 /* MPEG4 PES stream support */

#define FMPF_DISCPLAYBACK 0xFFFFFFFF 

// Disc Content flags
#define FMPC_DVD_VIDEO		0 /* DVD Video Disc */
#define FMPC_VCD		1 /* Video CD Disc */
#define FMPC_SVCD		2 /* Super Video CD Disc */
#define FMPC_CDDA		4 /* CD audio */
#define FMPC_UNSUPPORTED	0xFE 	/* namely, CDI and HQVCD */
#define FMPC_UNKNOWN		0xFF 	/* for error management purpose */

// Tray management constant
#define FMPT_TRAYCHECK	10
#define FMPT_TRAYCHANGE	11
#define FMPT_TRAYCLOSE	1
#define FMPT_TRAYOPEN	0


// Used to specify the method to acces the files
#define SYS_ACCESS 0x00	
#define UDF_ACCESS 0x01
#define ISO_ACCESS 0x02
#define DONTCARE_ACCESS 0xFF

// Buffer flags
enum tagFMP_PROPERTY_FLAGS
    {	
	FMP_DATADISCONTINUITY   = 0x1,		/* Data discontinuity : data loss */
	FMP_TIMEDISCONTINUITY   = 0x2,		/* PTS discontinuity */
	FMP_TRICKMODE_START     = 0x4,		/* Trick Mode Starts */
	FMP_TYPECHANGED         = 0x8,		/* Type of data has changed */
	FMP_TIMEVALID           = 0x10,		/* PTS is valid */
	FMP_ENDOFSTREAM         = 0x20,		/* End of stream */
	FMP_TRICKMODE_END       = 0x40,		/* Trick Mode Ends */
	FMP_TRICKMODE_SAMPLE	= 0x80,		/* A Trick Mode Sample */
 	FMP_SVCD_TM_2X			= 0x100,	/* SVCD trick mode speed 2x */
	FMP_SVCD_TM_3X			= 0x200,	/* SVCD trick mode speed 3x */
	FMP_SVCD_TM_4X			= 0x400,	/* SVCD trick mode speed 4x */
//	FMP_SVCD_FF				= 0x800,
	FMP_SVCD_FR				= 0x1000,
	FMP_SVCD_AUTOPAUSE		= 0x2000,
	FMP_SVCD_SEQ_HDR		= 0x4000,
	FMP_SVCD_UPDATE_PTS_REF	= 0x8000,

	FMP_TRICKMODE_DVD		= 0x10000,
	FMP_SEEKINGOP_DVD		= 0x20000,
	FMP_SVCD_CDDA_TRACK	= 0x40000 /* CDDA track on the VCD disk */

   };

// flags for the dwFlagsEx field in the FMP_BUFFER structure
enum tagFMP_PROPERTY_FLAGS_EX
{	
	FMP_VIDEO_PES		= 0x1,
	FMP_AUDIO_PES		= 0x2,
	FMP_DVD_AUDIO_PES	= 0x4
};

#define	FMP_SVCD_FLAGS		(0x0000FF80)
#define FMP_DVD_FLAGS		(0x00010000)

// DriverNumber : 
#define PRIMARY_MASTER		0x00	/* Drive is primary master */
#define SECONDARY_MASTER	0x02	/* Drive is secondary master */
#define NO_DRIVE			0xFF	/* No DVD-ROM drive */

DWORD MPEGPowerOn (BYTE bDriverNumber);
DWORD MPEGPowerOff (void);

// Init MPEG driver
DWORD MPEGDriverEntry (BYTE bDriverNumber);

// Prepare to Unload the driver
DWORD MPEGDriverUnload (void);

// Open the MPEG driver (Push Mode)
DWORD FMPOpen (DWORD dwflags, DWORD dwSize, DWORD dwCount, PFMPCALLBACK pCallback, DWORD dwContext);

// Open the MPEG driver (Disc Playback)
DWORD FMPOpenDiscPlayback (PFMP_OPENSTRUCT pFMPOpenStruct);

// Close the MPEG driver
DWORD FMPClose (void);

// Give version and copyright information
DWORD FMPAbout( TCHAR* sVersion, TCHAR* sDate, TCHAR *sCopyright );

// Close a MPEG file and get ready to play another one
DWORD FMPCloseFile(void);

// Stop playing a MPEG file and get ready to restart playing it at its beginning
DWORD FMPStopFile(void);

// Really play a MPEG file
DWORD FMPPlayFile(char *);

// Start MPEG playback
DWORD FMPPlay (void);

// Stop MPEG playback
DWORD FMPStop (void);

// Pause MPEG playback
DWORD FMPPause (void);

// Flush internal FIFOs of the driver
DWORD FMPFlush (BOOL bGDF);

// Get a buffer from the memory manager
DWORD FMPGetBuffer (PFMP_BUFFER pBuffer, BOOL bBlockingCall);

// Push buffer into driver
DWORD FMPPush (PFMP_BUFFER pBuffer);

// Get & Set
DWORD FMPGet (DWORD dwIndex);
DWORD FMPSet (DWORD dwIndex, DWORD dwValue);

// set,get properties for Hwlib using rm84cmn.h
#include "rm84cmn.h"
DWORD FMPProperty(DWORD dwFlags, DWORD dwPropSet, DWORD dwPropId, DWORD dwPropFlags, void* pPropData, DWORD dwPropSizeIn, DWORD* pdwPropSizeOut);

// change the video port output dimensions
DWORD FMPSetVideoPortDimensions (DWORD Width, DWORD Height);

// Queries for a specific interface
// this interface is valid between FMPOpen and FMPClose
DWORD FMPQueryInterface (DWORD InterfaceId, void **ppv);
#include "ifmp.h"

// Picture Placement
DWORD FMPSetSource (DWORD dwX, DWORD dwY, DWORD dwcX, DWORD dwcY);
DWORD FMPSetVisibleSource (DWORD dwX, DWORD dwY, DWORD dwcX, DWORD dwcY);
DWORD FMPSetDestination (DWORD dwX, DWORD dwY, DWORD dwcX, DWORD dwcY);
DWORD FMPEnableOverlay (DWORD dwEnable);
DWORD FMPEnableFullScreen (DWORD dwEnable);

// Bitmap operation
DWORD FMPEnableOSD (DWORD dwEnable);
DWORD FMPShowBitmap (DWORD dwBitMap, DWORD dwAlphaBlendingFactor, DWORD dwX, DWORD dwY, DWORD dwcX, DWORD dwcY);

// Disc operations
DWORD FMPEjectDisc (void);
DWORD FMPLoadDisc (void);
DWORD FMPIsDiscLoaded(void);
DWORD FMPTrayOperation( DWORD dwFMPTAction);

// Get Last Error
const char* FMPGetLastError (DWORD dwError);

/*******************************************************************************
* DVD Functions
*******************************************************************************/

//Play a Title by Title number
DWORD TitlePlay (DWORD dwTitleNumber);

//Play from the beginning of a chapter specified by the Title number and the chapter number
DWORD ChapterPlay(BYTE bTitleNumber, WORD wChapterNumber);

//Play from the specified position of the Title by the Title number and Time
DWORD TimePlay(BYTE bTitleNumber, TCHAR* sTime);

//This command stops the current presentation and starts presentation from the specified position 
//of the title by Time within the same title.
DWORD TimeSearch(TCHAR* sTime);

//Stop the current presentation and start the presentation from the beginning of PTT (Part_of_Title) 
//specified by PTT numbers within the same Title.
DWORD ChapterSearch(WORD wChapterNumber);

//Stop the execution of the current Program Chain and play the GoUp Program Chain 
//(specified in the current Program Chain)
DWORD GoUp(void);

//Stop the current presentation and start the presentation from the beginning 
//of previous Program within the same Program Chain (PGC).
DWORD PrevPGSearch(void);

//Stop the current presentation and start the presentation from the beginning 
//of the current Program within the same PGC.
DWORD TopPGSearch (void);

//Stop the current presentation and start the presentation from the beginning 
//of the next Program within the same PGC.
DWORD NextPGSearch (void);

//The operation to scan play at the specified Speed. 
//This user function includes scan play and slow playback for forward navigation 
//at any speed (expect normal speed).
DWORD ForwardScan (WORD wSpeed, BOOL bFast);

//The operation to scan play at the specified Speed. 
//This user function includes scan play and slow playback for backward navigation 
//at any speed (expect normal speed).
DWORD BackwardScan (WORD wSpeed, BOOL bFast);

//Call the Menu in the Program Chain (PGC).
DWORD MenuCall (BYTE bMenuID);

//This operation returns from the Menu-space
DWORD Resume(void);

//This operation stops play for later resume of play
DWORD StopForResume(void);

//DVD Query attributes
DWORD DVDQueryAttribute(DWORD dwFlag, DWORD dwArg);


//Select the Buttons on the Menu Screen.
DWORD UpperButtonSelect(void);
DWORD LowerButtonSelect(void);
DWORD LeftButtonSelect(void);
DWORD RightButtonSelect(void);

//Activate the current Highlighted Button.
DWORD ButtonActivate(void);

//Activate the specified Highlighted Button.
DWORD ButtonSelectAndActivate(BYTE bButton);

//Operation to release Still.
DWORD StillOff(void);

//Pause the presentation.
DWORD PauseOn(void);

//Set the stream number of the Sub-Picture and whether the sub-picture is displayed or not.
DWORD SubPictureStreamChange (BYTE bStreamNumber, BOOL bDisplayFlag);

//Set the Stream number of the Audio.
DWORD AudioStreamChange (BYTE bStreamNumber);

//Select the language for System Menu according to the Language Code
DWORD MenuLanguageSelect (WORD dwCode);

//In Angle Block, change the Angle
DWORD AngleChange (BYTE bAngleNumber);

//Set the Parental Level
DWORD ParentalLevelSelect (BYTE bParentalLevel);

//Set the Country Code for Parental Management
DWORD ParentalCountrySelect (DWORD dwCountryCode);

//Change the mode of Audio Mixing mode for Karaoke.
DWORD KaraokeAudioPresentationMode (DWORD dwMode);

//Start or stop title repeat
DWORD RepeatTitle(void);

//Start of stop chapter repeat
DWORD RepeatChapter(void);

//Start AB repeat
DWORD RepeatAB(void);

//Clear Repeat AB;
DWORD ClearRepeatAB(void);

//Store bookmark
DWORD StoreBookmark(void *pBookmark);

//Resume playback from bookmark
DWORD ResumeBookmark(void *pBookmark);

//Stop the current presentation and start the presentation from the beginning 
//of the return play item.
DWORD ReturnPGSearch (void);

//Stop the current presentation and start the presentation from the beginning 
//of the default play item.
DWORD DefaultPGSearch (void);

// For VCD button selection
DWORD NumericSelections (BYTE sel);

// Fastforward (VCD and DVD)
DWORD FastForward (BYTE bSpeed);

// Rewind (VCD and DVD)
DWORD Rewind (BYTE bSpeed);

// Step and pause MPEG playback
DWORD FMPStep (DWORD nFrames);

// Region Code Control
DWORD RegionCodeControl (BOOL bInit, DWORD dwArg1, DWORD dwArg2);

/*******************************************************************************
* CD-Audio Commands
*******************************************************************************/
DWORD	CDDANextTrack( void );
DWORD	CDDAPrevTrack( void );
DWORD	CDDAFastForward( DWORD dwSpeed );
DWORD	CDDARewind( DWORD dwSpeed );
#define CDDA_MODE_REPEAT_NONE		0
#define CDDA_MODE_REPEAT_TRACK		1
#define CDDA_MODE_REPEAT_DISC		2
DWORD	CDDARepeat( DWORD dwMode );
DWORD	CDDAGetNumberOfTracks( BYTE* pbTrackNumber );
DWORD	CDDAGetTrackDuration( BYTE bTrackNumber, DWORD *pbSeconds );
DWORD	CDDAGetStatus( DWORD *pdwMode, BYTE *pbTrack, DWORD *pdwSeconds );
DWORD	CDDAPlayTrack( DWORD dwTrack, DWORD dwMinutes, DWORD dwSeconds);

/*******************************************************************************
* Local Storage of Settings Commands
*******************************************************************************/
DWORD FMPWriteProfileWord(	DWORD dwKeyIndex /* key index */, DWORD wValue /* value */);
DWORD FMPGetProfileWord  (	DWORD dwKeyIndex /* key index */);

// DVD Index Keys
#define RMREG_LANGUAGECODE            0x00
#define RMREG_MENU_LANGUAGECODE       0x01
#define RMREG_AUDIO_LANGUAGECODE      0x02
#define RMREG_SUBPICTURE_LANGUAGECODE 0x03
#define RMREG_SUBPICTURE_DISPLAY_MODE 0x04
#define RMREG_USER_ASPECT_RATIO		  0x05
#define RMREG_USER_VIDEO_OUTPUT_MODE  0x06
#define RMREG_COUNTRYCODE             0x07
#define RMREG_PARENTALCONTROL         0x08
#define RMREG_PASSW_LOW_WORD          0x09
#define RMREG_PASSW_HIGH_WORD         0x0A
#define RMREG_PASSW_STATE             0x0B
#define RMREG_AUTOPLAY                0x0C
#define RMREG_PBC_OFF				  0x0D
#define RMREG_MACROVISION             0x0E
#define RMREG_REGION_CODE             0x0F

#define RMREG_AUDIO_TYPE              0x10
#define RMREG_OUTPUTMODE              0x11
#define RMREG_HDTV_RES                0x12

#define RMREG_MAX					  0x13


/*******************************************************************************
* FMP Commands
*******************************************************************************/
#define FMP_DRIVERUNLOAD                0xDD   /* Unload the MPEG driver */
#define FMP_CLOSEFILE                   0xCC   /* Close the file and get ready to play another one */
#define FMP_STOPFILE                    0xBB   /* Stop Playing a File and get ready to replay at the beginning */
#define FMP_PLAYFILE                    0xAA   /* Really Play a File */
#define FMP_OPEN                        0x01   /* Open New Stream */
#define FMP_CLOSE                       0x02   /* Close Stream */
#define FMP_PLAY                        0x03   /* Play Stream */
#define FMP_PAUSE                       0x04   /* Pause Stream */
#define FMP_STOP                        0x05   /* Stop Stream */
#define FMP_SEEK                        0x06   /* Seek inside the stream */
#define FMP_STEP                        0x07   /* Step a number of frames inside the stream */
#define FMP_SET                         0x09   /* Driver Settings : set */
#define FMP_GET                         0x0A   /* Driver Settings : get */
#define FMP_PUSH                        0x0B   /* Push model : push one buffer*/
#define FMP_FLUSH                       0x0C   /* Flush all the data inside the driver */
#define FMP_GETBUFFER                   0x0D   /* Get a buffer from the driver */
#define FMP_EJECT_DISC					0x0E   /* issue an eject command to the dvdrom */
#define FMP_LOAD_DISC					0x0F   /* issue a load command to the dvdrom */
#define FMP_SHOW_BITMAP					0x10   /* show a bitmap on the screen */
#define FMP_ENABLE_OSD					0x11   /* enable osd functionality */
#define FMP_SET_SOURCE					0x12   /* set the source capture dimensions */
#define FMP_SET_VISIBLE_SOURCE			0x13   /* set the dimensions in source to be zoomed in destination*/
#define FMP_SET_DESTINATION				0x14   /* set the destination rectangle */
#define FMP_ENABLE_OVERLAY				0x15   /* enable overlay functionality */
#define FMP_ENABLE_FULLSCREEN			0x16   /* enable full screen functionality */
#define FMP_IS_DISC_LOADED				0x17   /* Test whether a Disc is present in the drive */
#define FMP_QUERY_INTERFACE             0x18   /* Query Specific interfaces*/
#define FMP_REGISTER_INTERFACE          0x19   /* Register external interface */
#define FMP_SET_VIDEOPORT_DIMENSIONS	0x1a   /* set dimensions of the output video port */
#define FMP_PROPERTY_GET				0x1b   /* get properties */
#define FMP_PROPERTY_SET				0x1c   /* get properties */
#define FMP_PROPERTY                    0x1d   /* reference to the FMPProperty() function, I need that flag. (Pascal) */
#define FMP_REGIONCONTROL               0x1e   /* Region Code Control */

#define FMP_NAV_EOS                     0x0100 /* Simulate EOS (internal function) */
#define FMP_NAV_CHECK_EOS               0x0101 /* Check if EOS is needed (internal function) */
#define FMP_NAV_AUTOPAUSE				0x0102 /* auto pause sector (SVCD) has finished */
#define FMP_NAV_COMMAND					0x0103 /* general navigation command */

#define FMP_TITLEPLAY				        0x0200 /* DVD Title Play */
#define FMP_TIME_PLAY                       0x0205 /* Time Play */
#define FMP_TIME_SEARCH                     0x0208 /* Time Search */
#define FMP_RESUME                          0x0210 /* Resume */
#define FMP_PREV_PG_SEARCH                  0x020A /* Previous Program Search */
#define FMP_NEXT_PG_SEARCH                  0x020C /* Next Program Search */
#define FMP_AST_STREAM_CHANGE               0x021B /* Audio Stream Change */
#define FMP_QUERY_ATTRIBUTE                 0x0223
#define FMP_FORWARD_SCAN                    0x020D /* Forward Scan */
#define FMP_BACKWARD_SCAN                   0x020E /* Backward Scan */
#define FMP_REPEAT_TITLE                    0x0224 /* Repeat Title */
#define FMP_REPEAT_CHAPTER                  0x0225 /* Repeat Chapter */
#define FMP_REPEAT_AB						0x0226 /* AB Repeat */
#define FMP_CLEAR_REPEAT_AB					0x0227 /* AB Repeat */
#define FMP_STORE_BOOKMARK                  0x0228 /* Store bookmark */
#define FMP_RESUME_BOOKMARK                  0x0229 /* Restore bookmark */


#define FMP_DVD_PTT_PLAY                        0x0201 /* Chapter Play */
#define FMP_DVD_STOP                            0x0206   /* Stop */    
#define FMP_DVD_GO_UP                           0x0207 /* Go Up */
#define FMP_DVD_PTT_SEARCH                      0x0209 /* Chapter Search */
#define FMP_DVD_TOP_PG_SEARCH                   0x020B /* Top Program Search */
#define FMP_DVD_MENU_CALL                       0x020F /* Menu Call */
#define FMP_DVD_UPPER_BUTTON_SELECT             0x0211 /* Upper Button Select */
#define FMP_DVD_LOWER_BUTTON_SELECT             0x0212 /* Lower Button Select */
#define FMP_DVD_LEFT_BUTTON_SELECT              0x0213 /* Left Button Select */
#define FMP_DVD_RIGHT_BUTTON_SELECT             0x0214 /* Right Button Select */
#define FMP_DVD_BUTTON_ACTIVATE                 0x0215 /* Button Activate */
#define FMP_DVD_BUTTON_SELECT_AND_ACTIVATE      0x0216 /* Button Select and Activate */
#define FMP_DVD_STILL_OFF                       0x0217 /* Still Off */
#define FMP_DVD_MENU_LANGUAGE_SELECT            0x021A /* Menu Language Select */
#define FMP_DVD_SPST_STREAM_CHANGE              0x021C /* Subpicture Stream Change */
#define FMP_DVD_ANGLE_CHANGE                    0x021D /* Angle Change */
#define FMP_DVD_PARENTAL_LEVEL_SELECT           0x021F /* Parental Level Change */
#define FMP_DVD_PARENTAL_COUNTRY_SELECT         0x0220 /* Parental Country Change*/
#define FMP_DVD_KARAOKE_AUDIO_PRESENTATION      0x0221 /* Karaoke Audio Presentation change */
#define FMP_DVD_VIDEO_PRESENTATION_MODE_CHANGE  0x0222 /* Video Presentation Mode Change */

#define FMP_VCD_STOP_FOR_RESUME					0x0300
#define FMP_VCD_RETURN_PG_SEARCH                0x0306 /* VCD Return Play Item Search */
#define FMP_VCD_DEFAULT_PG_SEARCH               0x0307 /* VCD Default Play Item Search */
#define FMP_VCD_NUMERIC_SELECTIONS				0x0308 /* VCD Numeric Selection */
#define FMP_VCD_FAST_FORWARD					0x0309 /* VCD Numeric Selection */
#define FMP_VCD_REWIND							0x030A /* VCD Numeric Selection */

#define FMP_CDDA_NEXT_TRACK						0x0400
#define FMP_CDDA_PREV_TRACK						0x0401
#define FMP_CDDA_TRACK_DURATION					0x0402
#define FMP_CDDA_PLAY_TRACK						0x0403

#define FMP_WRITE_PROFILE_WORD					0x0500
#define FMP_GET_PROFILE_WORD					0x0501

#define FMP_ANALOG_OVERLAY						0x10000000 /* Analog Overlay */

#define FMP_EXTENDED_CMD_MASK					0xF0000000 /* Extended commands mask */
#define FMP_EXTENDED_SUBCMD_MASK				0x0FFFFFFF /* Extended subcommands mask */


/*******************************************************************************
// FMP Errors
*******************************************************************************/
#define FMPE_OK                           0x00000000    /* No error */
#define FMPE_ERROR                        0xFFFFFFFF	/* Error : use getLastError to get the Error code */
#define FMPE_DRIVER_OPEN                  0x00000001    /* Driver Already Open (FMPOpen) */
#define FMPE_DRIVER_NOT_OPEN              0x00000002    /* Driver Not Open (FMPClose) */
#define FMPE_CANNOT_OPEN_DRIVER           0x00000003    /* Cannot open the MPEG driver */
#define FMPE_NOT_ENOUGH_MEMORY            0x00000004    /* No more memory available */
#define FMPE_NOT_IMPLEMENTED              0x00000005    /* This function is not implemented */
#define FMPE_INVALID_INDEX                0x00000006    /* Invalid Index (settings)*/
#define FMPE_READ_ONLY                    0x00000007    /* Read Only settings */
#define FMPE_PUSH_WHILE_STOPPED           0x00000008    /* Pushing data in Stopped state */
#define FMPE_INVALID_PARAMETER            0x00000009    /* Invalid Parameter */
#define FMPE_NO_MPEG_SERVER               0x0000000A    /* No MPEG Server is running*/
#define FMPE_NO_ANALOG_OVERLAY_CAPABILITY 0x0000000B	/* No Analog Overlay */
#define FMPE_INVALID_STREAM_TYPE          0x0000000C	/* This function does not support the current stream type */

#define FMPE_ENTRY_MOD_INIT_FAILED        0x0000000D    /* MPEGDriverEntry Failed : Module loader failed to initialize */
#define FMPE_ENTRY_HWL_INIT_FAILED        0x0000000E    /* MPEGDriverEntry Failed : Failed to create MPEG Hardware driver*/
#define FMPE_ENTRY_DVDDEV_INIT_FAILED     0x0000000F    /* MPEGDriverEntry Failed : Failed to create DVD-ROM device driver*/

#define FMPE_CDDA_FAILURE                 0x00000010    /* CDDANextTrack : Failure or end of disk has been reached (or beginning)*/

#define FMPE_UNEXPECTED					  0x00000011    /* Unexpected unknown errors for VCD and SVCD */
#define FMPE_DISABLED                     0x00000012    /* The function is currently is disabled */
#define FMPE_INVALIDARG                   0x00000013    /* The list offset requested is out of range */
#define FMPE_TIMERACTIVATED               0x00000014    /* The timer has been activated */
#define FMPE_FAILURE                      0x00000015    /* For VCD and SVCD failure */
/*******************************************************************************
* FMP Settings
*******************************************************************************/
#define FMPI_TRICKMODE                  0x00000001
#define FMPI_STC                        0x00000002
#define FMPI_PTS                        0x00000003
#define FMPI_AUDIO_COUNT                0x00000004
#define FMPI_AUDIO_SELECT               0x00000005
#define FMPI_NPT_REFERENCE				0x00000006
#define FMPI_STC_REFERENCE				0x00000007
#define FMPI_BRIGHTNESS					0x00000008
#define FMPI_SATURATION					0x00000009
#define FMPI_CONTRAST					0x0000000A

#define FMPI_LEFT_VOLUME				0x0000000B
#define FMPI_RIGHT_VOLUME				0x0000000C
#define FMPI_MUTE						0x0000000D
#define FMPI_AUDIO_OUTPUT				0x0000000E
#define FMPI_AUDIO_MODE					0x0000000F
#define FMPI_AUDIO_VCXO					0x00000010

#define FMPI_VIDEOOUT					0x00000011
#define FMPI_SOURCE_WINDOW				0x00000012
#define FMPI_VISIBLE_SOURCE_WINDOW		0x00000013
#define FMPI_DESTINATION_WINDOW			0x00000014
#define FMPI_OVERLAY_FLAGS				0x00000015
#define FMPI_VIDEO_SPEED				0x00000016
#define FMPI_VIDEOPORT_WINDOW			0x00000017
#define FMPI_VIDEOASPECT				0x00000018
#define FMPI_AUDIO_SPEED				0x00000019

#define FMPI_DISC_TYPE					0x00001001

#define	FMPI_ANGLES_AVAILABLE           0x00001002 
#define	FMPI_AUDIO_STREAMS_AVAILABLE	0x00001003  
#define	FMPI_BUTTONS_AVAILABLE			0x00001004  
#define	FMPI_CAN_SCAN					0x00001005  
#define	FMPI_CAN_SEEK					0x00001006   
#define	FMPI_CURRENT_ANGLE				0x00001007   
#define	FMPI_CURRENT_AUDIO_STREAM		0x00001008   
#define	FMPI_CURRENT_BUTTON				0x00001009   
#define	FMPI_CURRENT_CHAPTER			0x0000100A   
#define	FMPI_CURRENT_DOMAIN				0x0000100B   
#define	FMPI_CURRENT_POSITION			0x0000100C   
#define	FMPI_CURRENT_SUBPICT_STREAM		0x0000100D   
#define	FMPI_CURRENT_TIME				0x0000100E   
#define	FMPI_CURRENT_TITLE				0x0000100F   
#define	FMPI_SUBPICTURE_ON				0x00001010   
#define	FMPI_SUBPICTURE_STREAM_AVAILABLE	0x00001011  
#define	FMPI_TITLES_AVAILABLE			0x00001012   
#define	FMPI_TOTAL_TITLE_TIME			0x00001013   
#define FMPI_CURRENT_SPEED				0x00001014

#define	FMPI_SVCD_TRICK_MODE			0x00002000
#define	FMPI_CURRENT_TRACK				0x00002001
#define	FMPI_SVCD_DISCONTINUITY			0x00002002
#define	FMPI_SVCD_CURRENT_VOLUME		0x00002003
#define	FMPI_SVCD_VOLUMES_AVAILABLE		0x00002004
#define	FMPI_TRACKS_AVAILABLE			0x00002005
#define	FMPI_TOTAL_TRACK_TIME			0x00002006
#define	FMPI_CDDA_REPEAT_MODE			0x00002007

/*******************************************************************************
* FMP Messages
*******************************************************************************/
#define FMPM_ERROR                         0x00000001
#define FMPM_STARVATION                    0x00000002
#define FMPM_EOS                           0x00000003
#define FMPM_DVDROM_NOT_READY			   0x00000004
#define FMPM_AUTOPAUSE					   0x00000005	
#define FMPM_REGION_MISMATCH			   0x00000006
#define FMPM_TRICK_MODE_CHANGE		   0x00000007

#define FMPM_DVD_ANGLE_CHANGE              0x00000100 
#define FMPM_DVD_ANGLES_BLOCK              0x00000101 
#define FMPM_DVD_AUDIO_STREAM_CHANGE       0x00000102 
#define FMPM_DVD_BUTTONS_CHANGE            0x00000103 
#define FMPM_DVD_BUTTON_CHANGE             0x00000104 
#define FMPM_DVD_PROGRAM_START             0x00000105 
#define FMPM_DVD_CURRENT_TIME              0x00000106 
#define FMPM_DVD_DOMAIN_CHANGE             0x00000107 
#define FMPM_DVD_NO_FP_PGC                 0x00000108 
#define FMPM_DVD_PARENTAL_LEVEL_CHANGE     0x00000109 
#define FMPM_DVD_PLAYBACK_STOPPED          0x0000010A 
#define FMPM_DVD_STILL_OFF                 0x0000010B 
#define FMPM_DVD_STILL_ON                  0x0000010C 
#define FMPM_DVD_SUBPICTURE_STREAM_CHANGE  0x0000010D 
#define FMPM_DVD_TITLE_CHANGE              0x0000010E 
#define FMPM_DVD_VALID_UOPS_CHANGE         0x0000010F 
#define FMPM_DVD_CHAPTER_CHANGE            0x00000110 
#define FMPM_DVD_PARENTAL_ERROR			   0x00000111
#define FMPM_DVD_PARENTAL_CHECK			   0x00000112
#define FMPM_DVD_TITLE_AUTHENTICATION_ERR  0x00000113
#define FMPM_DVD_DISC_AUTHENTICATION_ERR   0x00000114
#define FMPM_DVD_END_PLAYBACK			   0x00000115
#define	FMPM_DVDROM_ERR					   0x00000116
#define FMPM_DVD_MACROVISION_LEVEL         0x00000117
#define FMPM_DVD_FATAL_ERROR			   0x00000118

#define FMPM_SVCD_PSD_END				   0x00000200
#define FMPM_CDDA_PSD_END				   0x00000300

/*******************************************************************************
* FMP Values
*******************************************************************************/
#define FMPV_TRICKMODE_NOCHANGE         0x00000000
#define FMPV_TRICKMODE_START            0x00000001
#define FMPV_TRICKMODE_END              0x00000002

// FMPI_VIDEOOUT related values.
// They have to match values in mpegcmn.h used by hardware library:
//		Bit7,Bit6 = COMPOSITE, COMPONENT_YUV, COMPONENT_RGB
//		Bit4      = SET_TV_AS_SOURCE, SET_TV_AS_USER
//		Bit2      = SET_ONETOONE, SET_SCALE
//		Bit3,Bit1 = SET_NTSC, SET_PAL, SET_PAL60, SET_PALM
//		Bit5,Bit0 = SET_VGA, SET_TV, SET_HDTV
#define FMPV_VIDEOOUT_COMPONENT_MASK	0x00C0
#define FMPV_VIDEOOUT_COMPOSITE			0x0000
#define FMPV_VIDEOOUT_COMPONENT_YUV		0x0080
#define FMPV_VIDEOOUT_COMPONENT_RGB		0x00C0

#define FMPV_VIDEOOUT_MODE_MASK			0x0021
#define FMPV_VIDEOOUT_VGA				0x0000
#define FMPV_VIDEOOUT_TV				0x0001
#define FMPV_VIDEOOUT_HDTV				0x0020

#define FMPV_VIDEOOUT_STANDARDTV_MASK	0x000A
#define FMPV_VIDEOOUT_NTSC				0x0000
#define FMPV_VIDEOOUT_PAL 				0x0002
#define FMPV_VIDEOOUT_PAL60				0x0008
#define FMPV_VIDEOOUT_PALM				0x000A

#define FMPV_VIDEOOUT_ONETOONE			0x0000
#define FMPV_VIDEOOUT_SCALE				0x0004

#define FMPV_VIDEOOUT_TV_AS_SOURCE		0x0010
#define FMPV_VIDEOOUT_TV_AS_USER		0x0000

// FMPI_VIDEOASPECT related values - they should be in sync
//			with KSPROPERTY_MPEG2VID enum defined in cgraphm.h
// FMPV_VIDEOASPECT_4x3_16x9_ZOOM_ON is not recommended for DVD because
// the subpicture is not visible
// Syntax of the setting name:
//		FMPV_VIDEOASPECT_InputAspectRatio_OutputAspectRatio_UserDisplayOption
//		InputAspectRatio - video material aspect ratio: 4x3 or 16x9
//		OutputAspectRatio - output display device: 4x3=standardTV or 16x9=wide TV
//		UserDisplayOption - user choice if the input and output aspect ratio are not
//			the same and if video material permit
#define FMPV_VIDEOASPECT_4x3_4x3_NORMAL			0x0002	// normal
#define FMPV_VIDEOASPECT_4x3_16x9_ZOOM_ON		0x0003	// upscale video to fit width keeping 4:3 aspect ratio, cropping top and bottom
#define FMPV_VIDEOASPECT_4x3_16x9_ZOOM_OFF		0x0004	// video not scaled horizontally centered, black on sides
#define FMPV_VIDEOASPECT_16x9_16x9_NORMAL		0x0005	// normal
#define FMPV_VIDEOASPECT_16x9_4x3_PANSCAN		0x0006	// pan and scan
#define FMPV_VIDEOASPECT_16x9_4x3_LETTERBOX		0x0007	// letter box

// FMPI_AUDIO_MODE related values.
// They have to match values in mpegcmn.h used by hardware library:
#define FMPV_AUDIO_MODE_STEREO			0x0000
#define FMPV_AUDIO_MODE_RIGHT_ONLY		0x0001
#define FMPV_AUDIO_MODE_LEFT_ONLY		0x0002
#define FMPV_AUDIO_MODE_MONOMIX			0x0003
/*******************************************************************************
* DVD Error Messages
*******************************************************************************/

#define DVDE_HANDLE                     0x2000 
#define DVDE_ARG                        0x2100
#define DVDE_TITLENUMBER                0x2200
#define DVDE_PTTNUMBER                  0x2300
#define DVDE_UNAUTHORIZED				0x2400
#define DVDE_OUT_OF_MEM					0x2500
#define DVDE_INCORRECT_STRUCT			0x2600
#define DVDE_NORESUME					0x2700
#define DVDE_PARENTAL_LOCK				0x2800

/*******************************************************************************
* DVD Menu ID
*******************************************************************************/
#define DVD_TITLE_ID                            0x02
#define DVD_ROOT_ID                             0x03
#define DVD_AUDIO_ID                            0x05
#define DVD_SUBPICTURE_ID                       0x04
#define DVD_ANGLE_ID                            0x06
#define DVD_PTT_ID                              0x07

/*******************************************************************************
* DVD Index
*******************************************************************************/
#define FMPI_DVD_VTS_NS                          0x0A01			// for internal use only
#define DVDI_TT_SRPTI                            0x0001
#define DVDI_AST_ATR                             0x0002
#define DVDI_SPST_ATR                            0x0003
#define DVDI_SPRM                                0x0004
#define DVDI_VIDEO_MODE                          0x0005
#define DVDI_TIME                                0x0006			// for internal use only
#define DVDI_UOP_CTL                             0x0007			// for internal use only
#define DVDI_STATE                               0x0008			// for internal use only
#define DVDI_REPEATMODE                          0x0009			// for internal use only
#define DVDI_VATR								 0x000A			// video attribute
#define DVDI_CURRENT_UOPS						 0x000B


/*******************************************************************************
* System Parameters (SPRMs)
*******************************************************************************/
// (VI4-157)

#define M_LCD				0
#define ASTN				1
#define SPSTN				2	
#define AGLN				3	
#define TTN					4
#define VTS_TTN				5
#define TT_PGCN				6
#define PTTN				7
#define HL_BTTN				8
#define NV_TMR				9
#define NV_TMR_TT_PGCN		10
#define P_AMXMD				11
#define CTY_CD				12
#define PTL_LVL				13
#define P_CFGV				14
#define P_CFGA				15
#define INI_LCD_AST			16
#define INI_LCD_EXT_AST		17
#define INI_LCD_SPST		18
#define INI_LCD_EXT_SPST	19
#define PRC					20


/*******************************************************************************
* Structures
*******************************************************************************/
#define MAX_AST 8		// 0..7
#define MAX_SP 32		// 0..31
#define MAX_TT 100		// 1..99
#define MAX_AGL 10		// 1..9

// TT_SRPTI
typedef struct {
	BYTE bTT_SRP_Ns;					
	BYTE bALG_Ns [MAX_TT];				
	WORD wPTT_Ns [MAX_TT];				
} TT_SRPTI, * PTT_SRPTI;

// AST_ATR
typedef struct {
	BYTE bAST_Ns;
	BOOL bAvailable [MAX_AST];
	BYTE bAudioCodingMode [MAX_AST];
	BOOL bMultichannelExtenstion [MAX_AST];
	BYTE bAudioType [MAX_AST];
	BYTE bAudioApplicationMode [MAX_AST];
	BYTE bQuantization [MAX_AST];
	BYTE bfs [MAX_AST];
	BYTE bNumberOfAudioChannels [MAX_AST];
	WORD wLanguageCode [MAX_AST];
	WORD wLanguageCodeExtension [MAX_AST];
	BYTE bApplicationExtension [MAX_AST];
} AST_ATR, *PAST_ATR;

// SPST_ATR
typedef struct {
	BYTE bSPST_Ns;
	BOOL bAvailable [MAX_SP];
	BYTE bSubpictureType [MAX_SP];
	WORD wLanguageCode [MAX_SP];
	WORD wLanguageCodeExtension [MAX_SP];
} SPST_ATR, *PSPST_ATR;

// VIDEO_MODE
typedef struct {
	BYTE bAspectRatio;
	BYTE bDisplayMode;
} VIDEO_MODE, *PVIDEO_MODE;

#define CRMBookmark_SIZE	9400

/******************************************************************************
* CDDA Index
*******************************************************************************/
#define CDDAI_TOC 0x0010

/*******************************************************************************
* Structures
*******************************************************************************/
typedef DWORD CDDA_CONTENT[99];
typedef DWORD* PCDDA_CONTENT[99]; 
	
/*******************************************************************************
* FMPAnalogOverlay Functions
*******************************************************************************/

DWORD FMPAnalogOverlay(DWORD SubCommand,DWORD dwArg1,DWORD dwArg2,
		       DWORD dwArg3,DWORD dwArg4,DWORD dwArg5);

// FMPAnalogOverlay subcommand
#define FMP_ANALOG_OVERLAY_ACCESS 0x1000

// FMPAnalogOverlay subcommand
#define FMP_ANALOG_OVERLAY_GET 0x1001
#define FMP_ANALOG_OVERLAY_SET 0x1002

// General commands
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGAKEY 0x202 // vgakey selection (dwArg2=R<<16+G<<8+B. indexed mode is not supported) 
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_DISPLAYRESOLUTION 0x203 // send dwArg2=depth (8,16,24,32), dwArg3=physical display width, dwArg4=physical display height
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_MODE 0x204  // mode selection (dwArg2=0(none),1(rectangle),2(overlay))
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_OVERLAY_CABLE_PRESENT 0x205 // get only: 1=yes, 0=no

// fine-tuning
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_XOFFSET 0x301
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_YOFFSET 0x302
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGACORRECTION 0x303
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGARUPPER 0x304
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGARLOWER 0x305
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGAGUPPER 0x306
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGAGLOWER 0x307
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGABUPPER 0x308
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_VGABLOWER 0x309
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_FINEADJUSTMENT 0x30a
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_HFREQUENCY 0x30b
#define FMP_ANALOG_OVERLAY_ATTRIBUTE_JITTERADJUSTMENT 0x30c

// FMPAnalogOverlay subcommand 
#define FMP_ANALOG_OVERLAY_AUTOCALIBRATION 0x1003
/* then dwArg2=0x88 YOffset autocalibration (horizontal at top pattern)
        dwArg2=0x89 XOffset autocalibration (vertical at left pattern)
        dwArg2=0x8a Correction autocalibration (vertical offset right pattern)
   
	dwArg2=0x80 Color (WHITE) autocalibration (full white vertical bar pattern)
	dwArg2=0x82 Color (GREY) autocalibration (half white vertical bar pattern) */

/*******************************************************************************
* Misc Values
*******************************************************************************/

#define FMP_MPEG_SERVER					"REALmagic/MPEGsvr"
#define FMP_MPEG_CLIENT					"REALmagic/MPEGclient"

#ifdef __cplusplus
}
#endif

#endif
