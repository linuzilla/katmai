/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    mpegcmn.h

Abstract:

    These are the definitions which are common to the MPEG API and
    Port/Miniport interfaces.

Revision History:
 
--*/
#ifndef _MPEGCMN_H
#define _MPEGCMN_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include "mpegerr.h"

// maximum number of mpeg devices in the computer
#define MAX_MPEG_DEVICES    40	// equivalent for RM_MAXIMUM =40 in fmpdefs.h

#ifndef _NO_64BITS_SUPPORT_

typedef ULONGLONG MPEG_SYSTEM_TIME, *PMPEG_SYSTEM_TIME;
#define MPEG_SYSTEM_TIME_MASK   ((MPEG_SYSTEM_TIME)0x00000001FFFFFFFF)
// information flags or-ed with Scr, Pts, Stc ( 64 bits variables )
#define PTS_VALIDITY				0x8000000000000000
#define PTS_WAVIFACE_4000			0x4000000000000000
#define DATA_DISCONTINUITY			0x0008000000000000
#define FIRST_AUDIO_PTS				0x0004000000000000
#define PTS_WAVIFACE_0002			0x0002000000000000
#define PTS_DISCONTINUITY			0x0001000000000000
// Information flags to indicate new DATA Format (See RM_DATA_FORMAT_STRUCT)
#define RMFLAG_DATA_FORMAT_CHANGE	0x2000000000000000
#define CTS_VALIDITY				0x1000000000000000

#endif

#ifdef MMOSA
typedef unsigned char       BOOLEAN, *PBOOLEAN;
typedef char                CHAR, *PCHAR;

#ifndef MIDL_PASS
typedef unsigned char       UCHAR, *PUCHAR;
typedef short               SHORT, *PSHORT;
typedef unsigned short      USHORT, *PUSHORT;
typedef unsigned __int64    ULONGLONG, *PULONGLONG;

#define VOID                void
typedef void *              PVOID;
#endif

#define IN
#define OUT
#define OPTIONAL
#else
#ifdef MIDL_PASS
typedef char                CHAR, *PCHAR;
typedef unsigned char       *PUCHAR;
typedef long                *PLONG;
#endif

#ifndef _COLORREF_DEFINED
#define _COLORREF_DEFINED
typedef ULONG COLORREF;
#endif

#ifndef _DWORD_
typedef unsigned long DWORD;
#define _DWORD_
#endif

#ifndef _WORD_
typedef unsigned short WORD;
#define _WORD_
#endif

#endif

// MpegAttrForcedProgressive flags
#define BROADCASTED_VIDEO		0x0200
#define DVD_VIDEO				0x0000
#define FORCED_PROGRESSIVE_OFF	0x0100	// display progressive or interlaced like in the video stream
#define FORCED_PROGRESSIVE_ON	0x0000	// display only progressive if the stream switches very fast from interlaced to progressive
#define PROGRESSIVE_ALWAYS		0x0080
#define PROGRESSIVE_MOVIES		0x0000
#define FRAME_DROP				0x0040
#define FIELD_DROP				0x0000
#define VGA_INTERLACED			0x0002
#define VGA_PROGRESSIVE			0x0000

// MpegAttrHardwareInfo1 flags
#define DARK_COLOR_LETTERBOX	0x0001	// hwDecoder is Quasar1 - letterbox bug
#define VGA_CABLE_NOT_CONNECTED	0x0002	// VGA cable is not connected for analog overlay boards

//	MpegAttrOverlayFlags
#define OVERLAY_FLAGS_MASK			0x000F
#define VIDEO_ZOOM_ENABLE			0x0001
#define VIDEO_TV_DEST_ENABLE		0x0002
#define VIDEO_HDTV_DEST_ENABLE		0x0004
#define OSD_VIDEO_INDEPENDENT_DEST	0x0008

// MpegAttrPlayOptions
typedef enum
{
	VideoHwPlayNormal = 0,
	VideoHwPlayToEvent,
	VideoHwPlayIFrame,
	VideoHwPlaySyncFaster,
	VideoHwPlayYUV
}PLAY_OPTIONS;

//****************************************************************************
// Enumerated Constants
//****************************************************************************

typedef enum _MPEG_ATTRIBUTE {
    MpegAttrAudioBass =     0,
    MpegAttrAudioChannel,
    MpegAttrAudioMode,
    MpegAttrAudioTreble,
    MpegAttrAudioVolumeLeft,
    MpegAttrAudioVolumeRight,
	MpegAttrAudioPosition,
	MpegAttrAudioRate,
	MpegAttrAudioLayerIndex,
	MpegAttrAudioBitrateIndex,
	MpegAttrAudioFormat,
	MpegAttrAudioNumChannels,
	MpegAttrAudioBlockAlign,
	MpegAttrAudioEnableSpdif,
	MpegAttrAudioSetPcmOutputSpdif,
	MpegAttrAudioVcxo,
	MpegAttrAudioOutput,
    MpegAttrMaximumAudioAttribute,

    MpegAttrVideoBrightness  =400,
    MpegAttrVideoChannel,
    MpegAttrVideoContrast,
    MpegAttrVideoHue,
    MpegAttrVideoMode,
    MpegAttrVideoSaturation,
    MpegAttrVideoAGC,
    MpegAttrVideoClamp,
    MpegAttrVideoCoring,
    MpegAttrVideoGain,
    MpegAttrVideoGenLock,			//410
    MpegAttrVideoSharpness,
    MpegAttrVideoSignalType,
	MpegAttrVideoPosition,	
	MpegAttrVideoSpeed,
	MpegAttrVideoFramePosition,
	MpegAttrVideoRate,
	MpegAttrVideoAspectRatio,
	MpegAttrVideoSpeed2,
	MpegAttrVideoCD10Parameter,
	MpegAttrVideoGamma,				//420
	MpegAttrVideoAccessHardware,
	MpegAttrVideoTv,
	MpegAttrGetSubID,
	MpegAttrVideoEnableSync,
	MpegAttrVideoOutputMode,
	MpegAttrVideoStill,
	MpegAttrVideoMacrovisionFlags,
    MpegAttrChgKey,
	MpegAttrKey,
	MpegAttrDiscKey,				//430
	MpegAttrTitleKey,
	MpegAttrVideoForceEos,
	MpegAttrAddPerfStat,
	MpegAttrRemovePerfStat,
	MpegAttrVideoDeviceSize,
	MpegAttrVideoModeChange,
	MpegAttrVideoKeepAspectRatio,
	MpegAttrTvFullScreen,
	MpegAttrTvEncoderToleranceNtsc,
	MpegAttrTvEncoderTolerancePal,	//440
	MpegAttrForcedProgressive,
	MpegAttrHDTVMode,
	MpegAttrVideoFramePts45k,
	MpegAttrVideoFrameNumberInGOP,
	MpegAttrPlayOptions,
	MpegAttrCodeVersion,
	MpegAttrMaximumVideoAttribute,

    MpegAttrOverlayXOffset = 800,
    MpegAttrOverlayYOffset,
	MpegAttrOverlayVgaCorrection,
	MpegAttrOverlayVgaRUpper,
	MpegAttrOverlayVgaRLower,
	MpegAttrOverlayVgaGUpper,
	MpegAttrOverlayVgaGLower,
	MpegAttrOverlayVgaBUpper,
	MpegAttrOverlayVgaBLower,
	MpegAttrOverlaySourceWidth,
	MpegAttrOverlaySourceHeight,
	MpegAttrOverlayHFrequency,
	MpegAttrOverlayFineAdjustment,	
	MpegAttrOverlayInDelay,
	MpegAttrOverlayOutDelay,
	MpegAttrOverlayCropLeft,
	MpegAttrOverlayCropRight,
	MpegAttrOverlayCropTop,
	MpegAttrOverlayCropBottom,
	MpegAttrOverlayJitterAdjustment,
	MpegAttrOverlayTVXYPos,
	MpegAttrOverlayFlags,
	MpegAttrOsdDest,
	MpegAttrOsdHiLiDest,
	MpegAttrOsdON,
	MpegAttrOsdOFF,
	MpegAttrOsdFLUSH,
	MpegAttrOsdBufferSize,
	MpegAttrDisplayResolutionEx,
	MpegAttrMaximumOverlayAttribute,

	// Subpicture & other
	MpegAttrWindowHandle = 1000,
	MpegAttrSubpicturePalette,
	MpegAttrUserPalette,
	MpegAttrPaletteHWnd,
	MpegAttrSprm8,
	MpegAttrCmd,
	MpegAttrDisplayState,
	MpegAttrDisplayType,
	MpegAttrCapabilities,
	MpegAttrDisplayResolution,
	MpegAttrHighlight,
	MpegAttrButtonAvailable,
	MpegAttrHardwareInfo1,
	MpegAttrPowerState,
	MpegAttrDirectShow,
	MpegAttrGetFreeInstanceNmb,
	MpegAttrGetMaxInstanceNmb,
	MpegAttrVgaVendorInfo,
	MpegAttrMaximumSubpictureAttribute,

	// Values used for debug purposes
	MpegAttrOverlayAdjustmentA = 1200,
	MpegAttrOverlayAdjustmentB,
	MpegAttrOverlayAdjustmentC,
	MpegAttrOverlayAdjustmentD,
	MpegAttrOverlayAdjustmentE,
	MpegAttrOverlayAdjustmentF,

	MpegAttrMaximum
} MPEG_ATTRIBUTE, *PMPEG_ATTRIBUTE;

#define MPEG_OEM_ATTRIBUTE(a) ((MPEG_ATTRIBUTE)(((unsigned)(a))+0x00008000))

// MpegAttrAudioMode flags
#define MPEG_ATTRIBUTE_AUDIO_MONO				0
#define MPEG_ATTRIBUTE_AUDIO_STEREO				1
#define MPEG_ATTRIBUTE_AUDIO_SPATIAL_STEREO		2
#define MPEG_ATTRIBUTE_AUDIO_PSEUDO_STEREO		3

// MpegAttrVideoMode flags
#define MPEG_ATTRIBUTE_VIDEO_COMPONENT			0
#define MPEG_ATTRIBUTE_VIDEO_PAL				1
#define MPEG_ATTRIBUTE_VIDEO_NTSC				2
#define MPEG_ATTRIBUTE_VIDEO_SECAM				3
#define MPEG_ATTRIBUTE_VIDEO_MAC				4
#define MPEG_ATTRIBUTE_VIDEO_AUTO				5

// MpegAttrAudioFormat 
#define AUDIO_FORMAT_MPEG1						1
#define AUDIO_FORMAT_MPEG2						2
#define AUDIO_FORMAT_AC3						3
#define AUDIO_FORMAT_PCM						4
#define AUDIO_FORMAT_DTS						5
#define AUDIO_FORMAT_DVD_AUDIO					6
#define AUDIO_FORMAT_ADPCM						7	// Use AUDIO_FORMAT_REVERSE_PCM instead
#define AUDIO_FORMAT_REVERSE_PCM				7
#define AUDIO_FORMAT_AAC						8

// MpegAttrAudioOutput 
#define AUDIO_OUTPUT_STEREO						0
#define AUDIO_OUTPUT_AC3DTS						1

// flags used to enable/disable the hardware audio outputs
#define AUDIO_DAC_ENABLE						0x0001
#define AUDIO_DAC_DISABLE						0x0000
#define AUDIO_SPDIF_ENABLE						0x0002
#define AUDIO_SPDIF_DISABLE						0x0000

// MpegAttrAudioMode; Audio modes defined also in impeg32, hardware.h
#define AUDIO_MODE_STEREO						0
#define AUDIO_MODE_RIGHT_ONLY					1
#define AUDIO_MODE_LEFT_ONLY					2
#define AUDIO_MODE_MONOMIX						3

// MpegAttrVideoAspectRatio
#define VIDEO_ASPECT_RATIO_16_9					3
#define VIDEO_ASPECT_RATIO_4_3					2

// MpegAttrVideoOutputMode
#define VIDEO_OUTPUT_MODE_NORMAL_OR_WIDE		0
#define VIDEO_OUTPUT_MODE_PAN_SCAN				1
#define VIDEO_OUTPUT_MODE_LETTERBOX				2
#define VIDEO_OUTPUT_MODE_HORZCENTER			3
#define VIDEO_OUTPUT_MODE_VERTCENTER			4

// MpegAttrVideoTv
#define COMPONENT_MASK		0x02C0	// old was 0x00C0
#define COMPOSITE			0x0000
#define COMPONENT_YUV		0x0080
#define COMPONENT_RGB		0x00C0
#define OUTPUT_OFF			0x0040
#define COMPONENT_RGB_SCART	0x0200

#define TV_DACS_ENABLE		0x0000	// default behaviour - disabled after HwReset, enabled when playing
#define TV_DACS_DISABLE		0x0100	// force TvDacs disabled 

#define VIDEOOUT_MASK		0x0021
#define SET_VGA				0x0000
#define SET_TV				0x0001
#define SET_HDTV			0x0020

#define STANDARDTV_MASK		0x010A
#define SET_NTSC			0x0000
#define SET_PAL 			0x0002
#define SET_PAL60			0x0008
#define SET_PALM			0x000A
#define SET_480P			0x0100	// used only for Vga scan convertion

// The following define prevents the driver from forcing TV output back
// to the VGA when using NetStream 2000 TV (TvOut under "General"),
#define SET_NO_TV_CHANGE	0x8000

#define SET_ONETOONE		0x0000
#define SET_SCALE			0x0004

#define SET_TV_AS_SOURCE	0x0010
#define SET_TV_AS_USER		0x0000

#define SCART_MASK			0x0003
#define SCART_DISABLE		0x0000
#define SCART_COMPOSITE		0x0001
#define SCART_RGB			0x0003

// MpegAttrVideoGenLock type
#define MPEG_ATTRIBUTE_VIDEO_GEN_LOCK_TV		0
#define MPEG_ATTRIBUTE_VIDEO_GEN_LOCK_VTR		1

// MpegAttrVideoSignalType type
#define MPEG_ATTRIBUTE_VIDEO_SIGNAL_COMPOSITE	0
#define MPEG_ATTRIBUTE_VIDEO_SIGNAL_SVHS		1

// MpegAttrAudioChannel Mpeg channel
//  auxiliary channels are mini-port specific
#define MPEG_ATTRIBUTE_AUDIO_CHANNEL_MPEG  0

// MpegAttrVideoChannel Mpeg channel
//  auxiliary channels are mini-port specific
#define MPEG_ATTRIBUTE_VIDEO_CHANNEL_MPEG  0

// MpegAttrCapabilities
// Values must same as FMP_CAPABILITY_xxx

// An analog overlay is present and a border adjustment utility is needed
// in order to be able to adjust the overlay to the VGA resolution.
#define MPEG_CAPABILITY_BORDER_ADJUST            0x00000001

// Every key color can be independantly ajusted. This is only used when 
// an analog overlay is used.
#define MPEG_CAPABILITY_KEYCOLOR_ADJUST          0x00000002

// Brightness, contrast and saturation can be adjusted
#define MPEG_CAPABILITY_BSC_ADJUST               0x00000004

// The analog overlay supports auto-calibration
#define MPEG_CAPABILITY_AUTO_CALIBRATE           0x00000008

// Mpeg4 capability
#define MPEG_CAPABILITY_MPEG4_DECODE             0x00000010

// Gamma correction can be adjusted
#define MPEG_CAPABILITY_GAMMA_ADJUST             0x00000020

// ????????
#define MPEG_CAPABILITY_HIRES                    0x00000040

// ????????
#define MPEG_CAPABILITY_USES_VPM                 0x00000080

// The hardware can play DVD titles
#define MPEG_CAPABILITY_DVD_CAN_PLAY             0x00000100

// ????????
#define MPEG_CAPABILITY_ASPECT_RATIO             0x00000200

// The video can be displayed on the VGA monitor. Note that it might
// also be possible to display it on TV when MPEG_CAPABILITY_TV_OUTPUT
// is set but it may not be possible at the same time. Use the flag
// MPEG_CAPABILITY_VGA_AND_TV to find if this is possible.
#define MPEG_CAPABILITY_VGA_WINDOW               0x00000400

// Indicates that hardware decryption is supported by the hardware
#define MPEG_CAPABILITY_DVD_DECRYPTION           0x00000800

// Used by DVD station to know if the video source size can change
// depending on the VGA alignement. This is to determine the default DVD
// station window size and to prevent scaling (and therefore video
// artifacts) when scale is 1:1.
#define MPEG_CAPABILITY_SRC_CHANGE               0x00001000

// Indicates that the hardware supports Frame Fast Forward
#define MPEG_CAPABILITY_FRAME_FF                 0x00002000

// The video can be displayed on TV. Note that it might also be possible
// to display the video on VGA when the flag MPEG_CAPABILITY_VGA_WINDOW
// is set but it may not be possible at the same time. Use the flag
// MPEG_CAPABILITY_VGA_AND_TV to find if this is possible.
#define MPEG_CAPABILITY_TV_OUTPUT                0x00004000

// Indicates that the video can be displayed both on TV and VGA. When this
// flag is set, both flag MPEG_CAPABILITY_VGA_WINDOW and 
// MPEG_CAPABILITY_TV_OUTPUT needs to be set. This capability can change
// in real time depending on the TV output format set using MpegAttrVideoTv.
// Therefore, every time DVD station changes the TV output format to
// PAL or NTSC with or without scale, it needs to verify this flag.
#define MPEG_CAPABILITY_VGA_AND_TV               0x00008000

// Indicates that the TV supports PAL 60, Note that this flag is invalid when
// MPEG_CAPABILITY_TV_OUTPUT is not set. Note also that PAL60 can only be used
// when source is NTSC unless MPEG_CAPABILITY_PAL_NTSC_CONVERT is set. This
// capability does not concern VGA.
#define MPEG_CAPABILITY_PAL60                    0x00010000

// Indicates that the hardware is capable of converting NTSC source into PAL and
// vice-versa. This is mostly usefull when a TV output is present.
#define MPEG_CAPABILITY_PAL_NTSC_CONVERT         0x00020000

// Indicates that the video can be displayed on a HDTV (progressive scan). This
// output format is incompatible (for now) with VGA and TV (ie, it cannot be
// displayed both on HDTV and TV or HDTV and VGA).
#define MPEG_CAPABILITY_HDTV_OUTPUT              0x00040000

// Indicates that the video on VGA cannot be scaled (Dragon products)
// MPEG_CAPABILITY_VGA_WINDOW cannot be set in the same time
#define MPEG_CAPABILITY_FULLSCREEN_ONLY          0x00080000

// Indicates that the video output cannot play simultaneously on VGA 
// and TV. If set DVD station will paint a VGA_TV button.
#define MPEG_CAPABILITY_VGATV_SWITCH_BUTTON      0x00100000

// Indicates that the TV encoder can be slightly adjusted for color
// control one some PAL TVs that only display back and white.
#define MPEG_CAPABILITY_TV_TOLERANCE             0x00200000

// Indicates that the EEPROM to store the region code is present on the board
#define MPEG_CAPABILITY_EEPROM_PRESENT           0x00400000

// Indicates that hardware supports seeking accurately (this is used by DirectShow
// in order to allow playing first frame after a StreamPause or Video flush).
#define MPEG_CAPABILITY_SEEK_ACCURATE            0x00800000

// Indicates that hardware only supports NTSC (Only used for IGS to prevent
// double conversion)
#define MPEG_CAPABILITY_NTSC_ONLY                0x01000000

// Indicates that hardware only supports NTSC (Only used for IGS to prevent
// double conversion)
#define MPEG_CAPABILITY_PAL_ONLY                 0x02000000

// Indicates that hardware supports Tv Component YUV
#define MPEG_CAPABILITY_TV_YUV_COMPONENT		0x04000000

// Indicates that hardware can play in the same time PCM data
// through Dac and Spdif
#define MPEG_CAPABILITY_DAC_SPDIF				0x08000000

// Indicates that hardware has a 480P connector to display fullscreen on
// another Vga monitor - Ventura2000
#define MPEG_CAPABILITY_480P					0x10000000

// Indicates that hardware supports Tv Component RGB
#define MPEG_CAPABILITY_TV_RGB_COMPONENT		0x20000000

// Indicates that hardware has DVI output
#define MPEG_CAPABILITY_DVI						0x40000000

// Indicates that hardware has the audio clock controlled by VCXO
#define MPEG_CAPABILITY_AUDIO_VCXO				0x80000000
/*
#ifndef _RMDEV_CAPABILITIES_DEFINED_
#define _RMDEV_CAPABILITIES_DEFINED_

typedef enum {
	RMCAP_Flags_cap0,
	RMCAP_Flags_cap1_dw,
	RMCAP_Flags_cap1_ull
}RMCAP_TYPE;

typedef struct {
    ULONG    Size;
    ULONG    Count;
} RMMULTIPLE_ITEM, *PRMMULTIPLE_ITEM;

typedef struct{
	RMCAP_TYPE	type;
	union{
		ULONG	Capabilities;
		PVOID	pExtraData;	
	}cap0;
	union{
		DWORD			dwCaps[2];
		ULONGLONG		ullCaps;
		RMMULTIPLE_ITEM	multi_header;
	}cap1;
}RMDEV_CAPABILITIES,*PRMDEV_CAPABILITIES;

#endif // _RMDEV_CAPABILITIES_DEFINED_
*/
typedef enum _MPEG_DEVICE_TYPE
{
    MpegAudioDevice,
    MpegVideoDevice,
    MpegOverlayDevice,
    MpegCombinedDevice,
	MpegSubpictureDevice
} MPEG_DEVICE_TYPE, *PMPEG_DEVICE_TYPE;

typedef enum _MPEG_STREAM_TYPE
{
    MpegSystemStream = 1,
    MpegAudioStream,
    MpegVideoStream,
	MpegSubpictureStream,
	MpegPCIStream
} MPEG_STREAM_TYPE, *PMPEG_STREAM_TYPE;

typedef enum _MPEG_CAPABILITY
{
    MpegCapAudioDevice = 0,
    MpegCapVideoDevice,
    MpegCapSeparateStreams,
    MpegCapCombinedStreams,
    MpegCapBitmaskOverlay,
    MpegCapChromaKeyOverlay,
    MpegCapAudioRenderToMemory,
    MpegCapVideoRenderToMemory,
	MpegCapSubPictureDevice,
    MpegCapMaximumCapability
} MPEG_CAPABILITY, *PMPEG_CAPABILITY;

#define MPEG_OEM_CAPABILITY(a)  ((MPEG_CAPABILITY)(((unsigned)a) + 0x00008000))

typedef enum _MPEG_INFO_ITEM
{
    MpegInfoCurrentPendingRequest = 1,      // Video and Audio
    MpegInfoMaximumPendingRequests,         // Video and Audio
    MpegInfoDecoderBufferSize,              // Video and Audio
    MpegInfoDecoderBufferBytesInUse,        // Video and Audio
    MpegInfoCurrentPacketBytesOutstanding,  // Video and Audio
    MpegInfoCurrentFrameNumber,             // Video and Audio
    MpegInfoStarvationCount,                // Video and Audio
    MpegInfoDecompressHeight,               // Video
    MpegInfoDecompressWidth,                // Video
    MpegInfoMinDestinationHeight,           // Overlay
    MpegInfoMaxDestinationHeight,           // Overlay
    MpegInfoMinDestinationWidth,            // Overlay
    MpegInfoMaxDestinationWidth,            // Overlay
	MpegInfoEventStatus,					// Video and Audio
	MpegInfoHwDecode						// Audio
} MPEG_INFO_ITEM, *PMPEG_INFO_ITEM;

#define MPEG_OEM_INFO_ITEM(a)   ((MPEG_INFO_ITEM)(((unsigned)a) + 0x00008000))

typedef enum _MPEG_DEVICE_STATE
{
    MpegStateStartup = 0,
    MpegStatePaused,
    MpegStatePlaying,
    MpegStateStarved,
    MpegStateFailed,
	MpegStateFrozen
} MPEG_DEVICE_STATE, *PMPEG_DEVICE_STATE;

typedef enum _REQUESTED_STATE_TYPE {
    RequestedNone = 0,
    RequestedPlay,
    RequestedPause,
    RequestedStop
} REQUESTED_STATE_TYPE;

typedef enum _MPEG_OVERLAY_MODE
{
    MpegModeNone = 1,
    MpegModeRectangle,
    MpegModeOverlay,
	MpegModeCalibrate
} MPEG_OVERLAY_MODE, *PMPEG_OVERLAY_MODE;

typedef enum _MPEG_EVENT_TYPE
{
    MpegPulseEvent = 0,
	MpegTimerEvent,
	MpegNullEvent
} MPEG_EVENT_TYPE, *PMPEG_EVENT_TYPE;

typedef enum _MPEG_DISPLAY_CONTEXT
{
	MpegDisplayGdi = 0
} MPEG_DISPLAY_CONTEXT, *PMPEG_DISPLAY_CONTEXT;

#ifdef _WINGDI_

typedef struct _MPEG_DISPLAY_PALETTE
{
	PLOGPALETTE pLogPalette;
	HPALETTE hPalette;
} MPEG_DISPLAY_PALETTE, *PMPEG_DISPLAY_PALETTE;

#endif

typedef enum _MPEG_HLI_CMD
{
	MpegSelectAdjacentButton = 0,
	MpegActivate,
	MpegSelectAndActivate,
	MpegMouseMove,
	MpegMouseButton,
	MpegSelectButton
} MPEG_HLI_CMD, *PMPEG_HLI_CMD;

typedef enum _MPEG_HLI_DIRECTION
{
	MpegUpperButton = 0,
	MpegLowerButton,
	MpegLeftButton,
	MpegRightButton
} MPEG_HLI_DIRECTION, *PMPEG_HLI_DIRECTION;

typedef enum _MPEG_DISPLAY_STATE
{
	MpegDisplayOff = 0,
	MpegDisplayOn
} MPEG_DISPLAY_STATE, *PMPEG_DISPLAY_STATE;

typedef enum _MPEG_DISPLAY_TYPE
{
	MpegDisplayNormal = 0,  //  4:3
	MpegDisplayWide,        // 16:9
	MpegDisplayLetterBox,
	MpegDisplayPanScan
} MPEG_DISPLAY_TYPE, *PMPEG_DISPLAY_TYPE;

typedef struct _MPEG_DISPLAY_RESOLUTION
{
	int iBitsPerPixel;
	int iScreenWidth;
	int iScreenHeight;
} MPEG_DISPLAY_RESOLUTION, *PMPEG_DISPLAY_RESOLUTION;

typedef struct _MPEG_DISPLAY_RESOLUTION_EX
{
	int iBitsPerPixel;
	int iScreenWidth;
	int iScreenHeight;
	int iHFrequency;
	int iTotalPixelsPerLine;
	int iTotalLinesPerFrame;
	int iPixelXOffset;
	int iLineYOffset;
	DWORD dwReserved[4];
} MPEG_DISPLAY_RESOLUTION_EX, *PMPEG_DISPLAY_RESOLUTION_EX;

typedef struct  _MPEG_ASYNC_CONTEXT{
	HANDLE	hEvent;
	ULONG	Reserved[ 10 ];
} MPEG_ASYNC_CONTEXT, *PMPEG_ASYNC_CONTEXT;

// Color limits for one specified color
//
typedef struct tagCOLOR_LIMITS{
	COLORREF crKeyColor;
	COLORREF crUpperLimit;
	COLORREF crLowerLimit;
} COLOR_LIMITS, *PCOLOR_LIMITS;

typedef enum _MPEG_DISPLAY_FRAME
{
	MpegDisplayMotion,
	MpegDisplayStill
} MPEG_DISPLAY_FRAME, *PMPEG_DISPLAY_FRAME;

typedef enum
{
	NO_OWNER,			// WDM, SYS and VXD not currently using the hardware
	OWNER_IS_WDM,
	OWNER_IS_SYS,		// We may want later to differentiate SYS from VxD
	OWNER_IS_VXD
} OWNER, *POWNER;

typedef struct tagHDTV_MODE
{
	DWORD HFreq;
	DWORD VFreq;
	DWORD VideoWidth;
	DWORD VideoHeight;

	DWORD HSyncTotal;	
	DWORD PreHSync;
	DWORD HSyncActive;
	DWORD PostHSync;

	DWORD VSyncTotal;	
	DWORD PreVSync;
	DWORD VSyncActive;
	DWORD PostVSync;

	DWORD PixelFreq;
	DWORD Interlaced;
} HDTV_MODE, *PHDTV_MODE;

//****************************************************************************
// Data Structures
//****************************************************************************

typedef struct _MPEG_PACKET_LIST_ENTRY
{
#ifdef MIDL_PASS
    ADDRESS     pPacketData;
#else
    UCHAR      *pPacketData;
#endif
    ULONG       ulPacketSize;
    MPEG_SYSTEM_TIME Scr;

	// the following 2 fields are used by the windows 95 VxDs
	// please do not use!
	UCHAR		*pData;
	ULONG		ulDataSize;
} MPEG_PACKET_LIST_ENTRY, *PMPEG_PACKET_LIST_ENTRY;

typedef struct _MPEG_ATTRIBUTE_PARAMS
{
    MPEG_ATTRIBUTE Attribute;			// attribute to Get or Set
    LONG	Value;                      // attribute dependent parameter 1
} MPEG_ATTRIBUTE_PARAMS, *PMPEG_ATTRIBUTE_PARAMS;

typedef struct _MPEG_ATTRIBUTE_PARAMS_EX
{
    MPEG_ATTRIBUTE Attribute;	// attribute to Get or Set
    void *pData;				// pointer to attribute data
    ULONG  ulDataSize;			// size of attribute data
} MPEG_ATTRIBUTE_PARAMS_EX, *PMPEG_ATTRIBUTE_PARAMS_EX;

typedef struct _MPEG_OVERLAY_RECT
{
    ULONG X;                         // window x position,width
    ULONG Y;                         // window y position,height
    ULONG cX;                        // window x position,width
    ULONG cY;                        // window y position,height
} MPEG_OVERLAY_RECT, *PMPEG_OVERLAY_RECT;

typedef struct _MPEG_POINT{
	ULONG	x;
	ULONG	y;
}MPEG_POINT,*PMPEG_POINT;

// Overlay VGA key
typedef struct _MPEG_OVERLAY_KEY
{
    COLORREF rgbColor;              // palette index or RGB color
    COLORREF rgbMask;               // significant bits in color
} MPEG_OVERLAY_KEY, *PMPEG_OVERLAY_KEY;

typedef struct _OVERLAY_COLOUR_REGISTERS
{
	USHORT	RedUpper;
	USHORT	RedLower;
	USHORT	GreenUpper;
	USHORT	GreenLower;
	USHORT	BlueUpper;
	USHORT	BlueLower;
	USHORT	Mask;
	USHORT	Status;
} OVERLAY_COLOUR_REGISTERS, *POVERLAY_COLOUR_REGISTERS;

typedef struct _MPEG_OVERLAY_KEY2
{
    COLORREF rgbColor;					// RGB color
	POVERLAY_COLOUR_REGISTERS pOverride;   
} MPEG_OVERLAY_KEY2, *PMPEG_OVERLAY_KEY2;

typedef struct _CALIBRATION_PARAMETERS
{
	COLORREF	UpperFF;	
	COLORREF	LowerFF;
	COLORREF	Upper80;	
	COLORREF	Lower80;
	COLORREF	Upper00;	
	COLORREF	Lower00;
} CALIBRATION_PARAMETERS, *PCALIBRATION_PARAMETERS;

typedef struct _MPEG_OVERLAY_BIT_MASK
{
    ULONG   PixelHeight;            // the height of the bit-mask buffer
    ULONG   PixelWidth;             // the wight of the bit-mask buffer
    ULONG   BufferPitch;            // the number of bytes-per-line
    ULONG   LeftEdgeBitOffset;      // the number of bits to skip on the left edge
    char   *pBitMask;               // pointer to the data
} MPEG_OVERLAY_BIT_MASK, *PMPEG_OVERLAY_BIT_MASK;

//
// This should really be a union but mmmidl doesn't support them
//
typedef struct _MPEG_DEVICE_INFO
{
    MPEG_DEVICE_STATE DeviceState;		// Current MPEG device decode state
	REQUESTED_STATE_TYPE RequestedState;// Current requested state of MPEG device
    ULONG DecoderBufferSize;			// Size of the decoder buffer
    ULONG DecoderBufferFullness;		// Used bytes in docoder buffer
    ULONG StarvationCount;				// The numer of times the device has
										//     entered the starvation state

    // The following two are only valid for video
    ULONG DecompressHeight;				// Native MPEG decode height
    ULONG DecompressWidth;				// Native MPEG decode width

    // The following are only valid for overlay
    ULONG MinDestinationHeight;         // Minimum height of overlay
    ULONG MaxDestinationHeight;         // Maximum height of overlay
    ULONG MinDestinationWidth;          // Minimum width of overlay
    ULONG MaxDestinationWidth;          // Maximum width of overlay

	ULONG EventStatus;					// status of an event

} MPEG_DEVICE_INFO, *PMPEG_DEVICE_INFO;

#define MPEG_EVENT_PAUSE_ON_EVENT		0x0001
#define MPEG_EVENT_CONTINUE_ON_EVENT	0x0002
#define MPEG_EVENT_CANCEL_ON_EVENT		0x0004
#define MPEG_EVENT_PAUSE_DSP_ON_EVENT	0x0008
#define	MPEG_EVENT_VIDEO_RELATIVE_FRAME	0x0010
#define MPEG_EVENT_VIDEO_ABSOLUTE_FRAME	0x0020
#define MPEG_EVENT_VIDEO_BYTES			0x0040
#define MPEG_EVENT_AUDIO_BYTES			0x0080
#define MPEG_EVENT_STEP					0x0100

typedef struct _MPEG_EVENT
{
	MPEG_EVENT_TYPE MpegEvent;			// type of event requested
	ULONG	ulData;						// event specific info (eg #of frames)
	ULONG	ulFlags;					// event flags - pause or continue?
} MPEG_EVENT, *PMPEG_EVENT;

typedef struct _CIRCULAR_BUFFER_INFO
{
	UCHAR	*pBuffer;	// pointer to start of buffer
	ULONG	ulSize;		// size of buffer
	LONG	RdPtr;		// offset from start of buffer
	LONG	WrPtr;		// offset from start of buffer
} CIRCULAR_BUFFER_INFO, *PCIRCULAR_BUFFER_INFO;

typedef struct _VXD_ADDRESS
{
	ULONG	BaseAddress;
	ULONG	Length;
} VXD_ADDRESS, *PVXD_ADDRESS;

typedef struct _WDM_ADDRESS
{
	ULONG	HighPart;
	ULONG	LowPart;
	ULONG	RangeLength;
	BOOLEAN	RangeInMemory;
} WDM_ADDRESS, *PWDM_ADDRESS;

typedef struct _HIGHLIGHT_COMMAND
{
	MPEG_HLI_CMD	command;
	DWORD			dwParameter;
} HIGHLIGHT_COMMAND, *PHIGHLIGHT_COMMAND;

typedef struct _HIGHLIGHT_COMMANDEX
{
	MPEG_HLI_CMD	command;
	union{
		BYTE		bButtonNumber;
		DWORD		dwParam;
		MPEG_POINT	point;
	}u;
} HIGHLIGHT_COMMANDEX, *PHIGHLIGHT_COMMANDEX;

typedef struct _PERFORMANCE_STAT
{
	PULONG	pAddr;
	char	szDesc[64];
} PERFORMANCE_STAT, *PPERFORMANCE_STAT;

typedef struct _I2C_INFO
{
	ULONG i2c_address;
	ULONG reg;
	ULONG data;
} I2C_INFO, *PI2C_INFO;
/*
typedef enum _BOARD_CTRL_CMD{
	CmdNoCommand,
	CmdReadHostRegisters,
	CmdWriteHostRegister,
	CmdGetDriverAddrInfo,
	CmdReadEEPROM,
	CmdWriteEEPROM,
	CmdReadOvrlRegisters,
	CmdWriteOvrlRegister,
	CmdReadTVEncRegisters,
	CmdWriteTVEncRegister,
	CmdGetBoardLayout,
	CmdGetDriverInfo
}BOARD_CTRL_CMD;
*/
typedef enum _BOARD_CTL_PROPERTY_SUBID{
	NoType = 0,
	chip_reg,
	ucode_sym,
	ovr_reg,	
	tvenc_reg,	
	var_rm8400,
	rtMaxActiveTable,
	rtEEPROM,
	data_memory,
	rtMaxType
}BOARD_CTL_PROPERTY_SUBID;

#ifndef _RM_WRITE_DEFINED
#define _RM_WRITE_DEFINED
typedef struct _RM_WRITE{
	ULONG	Addr;
	ULONG	Data;
}RM_WRITE, *PRM_WRITE;
#endif // _RM_WRITE_DEFINED

typedef struct _BI_HEADER{
	ULONG						ExtraSize;		// in bytes
	BOARD_CTL_PROPERTY_SUBID	PropSubID;
}BI_HEADER;

typedef struct _BOARD_INFO{
	BI_HEADER	Header;
	union{
		RM_WRITE	Write;
		UCHAR		uchar;
		USHORT		ushort;
		ULONG		ulong;
		ULONGLONG	ulonglong;
	}u;
}BOARD_INFO,*PBOARD_INFO;

#define DMABFLAGS_KernelMode	0x00000001
#define DMABFLAGS_CheckStatus	0x00000002

typedef struct{
	ULONG			Size;
	PVOID			UserVirtualAddress;
	ULONGLONG		PhysicalAddress;
	ULONG			Flags;
	ULONG			Reserved;
}DMABUFFER_INFO, *PDMABUFFER_INFO;

typedef struct _VCXO_SET
{
	ULONG	Coefficients[2];	// N,M,K,ppmError;
	ULONG	RepeatCount[2];		// number of VSyncs to keep the corresponding Coefficients
}VCXO_SET,*PVCXO_SET;

typedef enum tagHW_VCXO
{
	eHwVcxoDefault = -1,
	eHwVcxoPio0,
	eHwVcxoPio1,
	eHwVcxoPio2,
	eHwVcxoPio3,
	eHwVcxoPio4,
	eHwVcxoPio5,
	eHwVcxoPio6,
	eHwVcxoPio7,
	eHwVcxoPio8,
	eHwVcxoPio9,
	eHwVcxoPio10,
	eHwVcxoPio11,
	eHwVcxoPio12,
	eHwVcxoPio13,
	eHwVcxoPio14,
	eHwVcxoPio15,
	eHwVcxoNovaLitePll,
	eHwVcxoMax
}HW_VCXO;

typedef struct tagPCR_INFO	// used for PropId=etimPcrInfo in PropSet=TIME_SET
{
	WORD wStructureVersion;		// 0
	WORD PcrEnable;				// 1 for Pcr master, 0 for Apts master
	DWORD RecoveryTime;			// = log2(TimeRecovery/200us) = 16 for 13.1072sec
	DWORD PcrIntrPeriodUs;		// 200us ... 13,107,000us (1*200us ... 0xFFFF*200us)
	DWORD TimeResolution;		// 90000 for unit90k
	DWORD DeltaPCR_SCR_Max;		// unit90k, ex: 9000 unit90k = 100ms
	DWORD DeltaSCR_APTS_Max;	// unit90k, ex: 180 unit90k = 2ms
	HW_VCXO HwVcxo;				// Pio5 for Siemens box
	DWORD PpmRange;				// 150, for +/-150
}PCR_INFO;

typedef struct tagTIME_INFO	// used for PropId=etimPcrTime in PropSet=TIME_SET
{
	DWORD dwTimeResolution;	// 90000 for 90k pts unit
	ULONGLONG dwlTime;		// Pts
}TIME_INFO;

// MpegAttrVgaVendorInfo
typedef struct _VGA_VENDOR_INFO
{
	WORD wVendorID;
	WORD wDeviceID;
	WORD wSubSystemVendorID;
	WORD wSubSystemID;
	UCHAR bRevisionID;
} VGA_VENDOR_INFO, *PVGA_VENDOR_INFO;

// List all the different data format type passed in RM_DATA_FORMAT_STRUCT
typedef enum tagRM_DATA_FORMAT_ENUM
{
	RmDataFormatUnknown = -1,

	RmDataFormatMPEG12Video = 0,
	RmDataFormatMPEG4Video = 1,

	RmDataFormatMPEG1Audio = 0x10,		// AUDIO_FORMAT_MPEG1
	RmDataFormatMPEG2Audio,				// AUDIO_FORMAT_MPEG2
	RmDataFormatAC3Audio,				// AUDIO_FORMAT_AC3
	RmDataFormatPCMAudio,				// AUDIO_FORMAT_PCM
	RmDataFormatDTSAudio,				// AUDIO_FORMAT_DTS
	RmDataFormatDVDAudio,				// AUDIO_FORMAT_DVD_AUDIO
	RmDataFormatReversePCMAudio,		// AUDIO_FORMAT_REVERSE_PCM
	RmDataFormatMPEG4PCMAudio,			// AUDIO_FORMAT_PCM
	RmDataFormatMPEG4ReversePCMAudio,	// AUDIO_FORMAT_REVERSE_PCM
	RmDataFormatMPEG4Ac3Audio,			// AUDIO_FORMAT_AC3
	RmDataFormatMaximumAudio

} RM_DATA_FORMAT_ENUM, *PRM_DATA_FORMAT_ENUM;

// Base structure used to pass new data format to driver inside a data buffer.
// The flags of this packet needs to be set to RMFLAG_DATA_FORMAT_CHANGE
typedef struct tagRM_DATA_FORMAT_STRUCT
{
	WORD wStructSize;
	BYTE bStandard;		// See RM_DATA_FORMAT_ENUM (Use (BYTE) RM_DATA_FORMAT_ENUM)
	BYTE bRevision;		// Revision number (== RM_DATA_FORMAT_REVISION)
} RM_DATA_FORMAT_STRUCT, *PRM_DATA_FORMAT_STRUCT;

#define RM_DATA_FORMAT_REVISION 1	// Changes every time RM_DATA_FORMAT_STRUCT changes

// Base structure used to pass new data format to driver inside a data buffer.
// The flags of this packet needs to be set to RMFLAG_DATA_FORMAT_CHANGE
typedef struct tagRM_MPEG4_DATA_FORMAT_STRUCT
{
	WORD wStructSize;
	BYTE bStandard;		// See RM_DATA_FORMAT_ENUM (Use (BYTE) RM_DATA_FORMAT_ENUM)
	BYTE bRevision;		// Revision number (== RM_DATA_FORMAT_REVISION)

	DWORD dwTimeScale;	// MPEG4 track Time Scale
} RM_MPEG4_DATA_FORMAT_STRUCT, *PRM_MPEG4_DATA_FORMAT_STRUCT;

// RM_DATA_FORMAT_STRUCT structure used to pass RmDataFormatMPEG4Video video format
typedef struct tagRM_MPEG4_VIDEO_FORMAT_STRUCT
{
	WORD wStructSize;
	BYTE bStandard;		// See RM_DATA_FORMAT_ENUM (Use (BYTE) RM_DATA_FORMAT_ENUM)
	BYTE bRevision;		// Revision number (== RM_DATA_FORMAT_REVISION)
	
	DWORD dwTimeScale;	// MPEG4 track Time Scale
	
	DWORD dwVopTimeIncrementResolution;	// Number of ticks per second found in DSI or forced
										// when bForceFixedVOPRate is 1
	BYTE bAspectRatio;	// See MPEG4 Video Aspect Ratios
	WORD wWidth;		// Video Width
	WORD wHeight;		// Video Height

	BOOL bForceFixedVOPRate;		// Force VOP Rate <> DSI (Only for DivX)
	DWORD dwFixedVopTimeIncrement;	// When bForceFixedVOPRate==1, number of ticks per VOP
									// (Use dwVopTimeIncrementResolution for time resolution)
} RM_MPEG4_VIDEO_FORMAT_STRUCT, *PRM_MPEG4_VIDEO_FORMAT_STRUCT;

// RM_DATA_FORMAT_STRUCT structure used to pass RmDataFormatMPEG4Ac3Audio audio format
typedef struct tagRM_MPEG4_AC3_FORMAT_STRUCT
{
	WORD wStructSize;
	BYTE bStandard;		// See RM_DATA_FORMAT_ENUM (Use (BYTE) RM_DATA_FORMAT_ENUM)
	BYTE bRevision;		// Revision number (== RM_DATA_FORMAT_REVISION)

	DWORD dwTimeScale;	// MPEG4 track Time Scale

	LONG lSampling_Frequency;
} RM_MPEG4_AC3_FORMAT_STRUCT, *PRM_MPEG4_AC3_FORMAT_STRUCT;

// RM_DATA_FORMAT_STRUCT structure used to pass RmDataFormatMPEG4PCMAudio and RmDataFormatMPEG4ReversePCMAudio
typedef struct tagRM_MPEG4_PCM_FORMAT_STRUCT
{
	WORD wStructSize;
	BYTE bStandard;		// See RM_DATA_FORMAT_ENUM (Use (BYTE) RM_DATA_FORMAT_ENUM)
	BYTE bRevision;		// Revision number (== RM_DATA_FORMAT_REVISION)

	DWORD dwTimeScale;	// MPEG4 track Time Scale

	LONG lBitsPerSample;
	LONG lNumberOfChannels;
	LONG lSampling_Frequency;
} RM_MPEG4_PCM_FORMAT_STRUCT, *PRM_MPEG4_PCM_FORMAT_STRUCT;

// Make sure the following size is bigger or equal to the maximum RM_xxx_FORMAT_STRUCT!
#define MAX_DATA_FORMAT_STRUCT_SIZE 100

#ifdef __cplusplus
}
#endif

#endif
