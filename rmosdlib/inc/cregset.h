/*****************************************************************************
*  cregset.h : Registry implementation : settings
*  REALmagic FMP RTOS Driver
*  Created by Michael Ignaszewski
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 05/02/2000
*  Description:
* 		
* $Archive: /ANTILLES/Barbados/analogoverlay/RMDVDPLAYER/include/cregset.h $
*  $Author: Pascal $
*  $Date: 12/11/01 4:04p $
*  $Header: /ANTILLES/Barbados/analogoverlay/RMDVDPLAYER/include/cregset.h 23    12/11/01 4:04p Pascal $
*****************************************************************************/


typedef struct tagCRMRegEntry
    {
		LPCTSTR		m_lpKeyName;			// Key Name
		WORD		m_dwDefaultValue;		// Default Value
    }CRMRegEntry;



// Key indexes are defined in FMP.H

// DVD Keys
#define REG_LANGUAGECODE              TEXT("LanguageCode")
#define REG_MENU_LANGUAGECODE         TEXT("MenuLanguageCode")
#define REG_AUDIO_LANGUAGECODE        TEXT("AudioLanguageCode")
#define REG_SUBPICTURE_LANGUAGECODE   TEXT("SubpictureLanguageCode")
#define REG_SUBPICTURE_DISPLAY_MODE   TEXT("SubpictureDisplayMode")
#define REG_USER_ASPECT_RATIO         TEXT("UserAspectRatio")
#define REG_USER_VIDEO_OUTPUT_MODE    TEXT("UserVideoOutputMode")
#define REG_COUNTRYCODE               TEXT("CountryCode")
#define REG_PARENTALCONTROL           TEXT("ParentalControl")
#define REG_OUTPUTMODE                TEXT("OutputMode")
#define REG_HDTV_RES                  TEXT("HDTVResolution")
#define REG_AUTOPLAY                  TEXT("Autoplay")
#define REG_PASSW_STATE               TEXT("PasswordState")
#define REG_PASSW_LOW_WORD            TEXT("PasswordLowWord")
#define REG_PASSW_HIGH_WORD           TEXT("PasswordHighWord")
#define REG_REGION_CODE               TEXT("RegionCode")
#define REG_MACROVISION               TEXT("Macrovision")
#define REG_AUDIO_TYPE                TEXT("AudioType")
#define REG_REG_PCB					  TEXT("PCBOff")


/*******************************************************************************
* Player Configuration for Video
*******************************************************************************/
#define ASPECT_RATIO_4_3						0x0000
#define ASPECT_RATIO_16_9						0x0003
#define DISPLAY_MODE_NORMAL_OR_WIDE				0x0000
#define DISPLAY_MODE_PAN_SCAN					0x0001
#define DISPLAY_MODE_LETTERBOX					0x0002
#define DISPLAY_MODE_ZOOM_ON					0x0003

/*******************************************************************************
* Player Configuration for HDTV and TV output
*******************************************************************************/

#define TV_NTSC	                                0x0000
#define TV_PAL 	                                0x0002
#define TV_PAL60 	                              0x0008
#define TV_PALN 	                              0x000a
#define TV_MULTISYSTEM                          0x0100
#define HDTV_480P_60FPS                         0x0200
#define HDTV_480P_72FPS                         0x0201
#define HDTV_480P_96FPS                         0x0202
#define HDTV_480P_120FPS                        0x0203
#define HDTV_720P_60FPS                         0x0204
#define HDTV_720P_72FPS                         0x0205
#define HDTV_720P_96FPS                         0x0206
#define HDTV_960P_60FPS                         0x0207
#define HDTV_960P_72FPS                         0x0208
#define VGA				                              0x0300

#define RM_LANGUAGECODE_ENGLISH					0x656e			// TEXT ("en")
#define RM_LANGUAGECODE_FRENCH					0x6672			// TEXT ("fr")
#define RM_LANGUAGECODE_SPANISH					0x6573			// TEXT ("es")
#define RM_LANGUAGECODE_ITALIAN					0x6974			// TEXT ("it")
#define RM_LANGUAGECODE_GERMAN					0x6465			// TEXT ("de")
#define RM_LANGUAGECODE_DUTCH					0x6e6c			// TEXT ("nl")
#define RM_LANGUAGECODE_PORTUGUESE				0x7074			// TEXT ("pt")
#define RM_LANGUAGECODE_CHINESE					0x7a68			// TEXT ("zh")
#define RM_LANGUAGECODE_JAPANESE				0x6a61			// TEXT ("ja")
#define RM_LANGUAGECODE_KOREAN				        0x6b6f			// TEXT ("ko")
#define RM_LANGUAGECODE_VIETNAMESE				0x7669			// TEXT ("vi")
#define RM_LANGUAGECODE_ORIGINAL				0x6f72			// TEXT ("or")
#define RM_LANGUAGECODE_OTHERS					0x6f74			// TEXT ("ot")
#define RM_LANGUAGECODE_AUDIOFOLLOW				0x6166			// TEXT ("af")
#define RM_COUNTRYCODE_US					0x5553		        // TEXT	("US")
#define RM_COUNTRYCODE_OTHERS					0x4f54			// TEXT	("OT")
#define RM_PARENTALCONTROL_OFF					0x000f
#define RM_PARENTALCONTROL_LEVEL1				0x0001
#define RM_PARENTALCONTROL_LEVEL2				0x0002
#define RM_PARENTALCONTROL_LEVEL3				0x0003
#define RM_PARENTALCONTROL_LEVEL4				0x0004
#define RM_PARENTALCONTROL_LEVEL5				0x0005
#define RM_PARENTALCONTROL_LEVEL6				0x0006
#define RM_PARENTALCONTROL_LEVEL7				0x0007
#define RM_PARENTALCONTROL_LEVEL8				0x0008
#define RM_PASSWORD_OFF							0x0000
#define RM_PASSWORD_ON							0x0001
#define RM_NO_PASSWORD							0xffff
#define RM_MACROVISION_ENABLED					0x0001
#define RM_MACROVISION_DISABLED					0x0000
#define RM_AUDIO_TYPE_ANALOG					0x0000
#define RM_AUDIO_TYPE_SPDIF						0x0001
#define RM_DAC_TYPE_1720						1720
#define RM_DAC_TYPE_1716						1716
#define RM_AUTOPLAY_OFF							0x0000
#define RM_AUTOPLAY_ON							0x0001
#define RM_PBC_ON								0x0000
#define RM_PBC_OFF								0x0001


// DVD Defaults for Sigma Player
#define RM_LANGUAGECODE_DEFAULT						RM_LANGUAGECODE_ENGLISH
#define RM_AUDIO_LANGUAGECODE_DEFAULT				RM_LANGUAGECODE_ORIGINAL
#define RM_SUBPICTURE_LANGUAGECODE_DEFAULT			RM_LANGUAGECODE_AUDIOFOLLOW
#define RM_MENU_LANGUAGECODE_DEFAULT				RM_LANGUAGECODE_ENGLISH
#define RM_SUBPICTURE_DISPLAY_MODE_DEFAULT			0x00			
#define RM_COUNTRYCODE_DEFAULT						RM_COUNTRYCODE_US
#define RM_PARENTALCONTROL_DEFAULT					RM_PARENTALCONTROL_OFF
#define RM_ASPECT_RATIO_DEFAULT						ASPECT_RATIO_4_3
#define RM_USER_VIDEO_OUTPUT_MODE_DEFAULT			DISPLAY_MODE_LETTERBOX
#ifdef PRINCETON
#define RM_OUTPUTMODE_DEFAULT						TV_NTSC | 0x20
#else
#define RM_OUTPUTMODE_DEFAULT						TV_NTSC | 0x01
#endif
#define RM_HDTV_RES_DEFAULT     				HDTV_480P_60FPS
#define RM_PASSW_LOW_WORD_DEFAULT       RM_NO_PASSWORD
#define RM_PASSW_HIGH_WORD_DEFAULT      RM_NO_PASSWORD
#define RM_PASSW_STATE_DEFAULT          RM_PASSWORD_OFF
#define RM_MACROVISION_DEFAULT          RM_MACROVISION_ENABLED
#define RM_REGION_CODE_DEFAULT          0x0001
#define RM_AUDIO_TYPE_DEFAULT						RM_AUDIO_TYPE_ANALOG
#define RM_BALANCE_DEFAULT							100
#define RM_VOLUME_DEFAULT							  75
#define RM_XOFFSET_DEFAULT              0
#define RM_YOFFSET_DEFAULT              0
#define RM_VGACORRECTION_DEFAULT        1000
#define RM_VGARUPPER_DEFAULT            14
#define RM_VGARLOWER_DEFAULT            0
#define RM_VGAGUPPER_DEFAULT            56
#define RM_VGAGLOWER_DEFAULT            29
#define RM_VGABUPPER_DEFAULT            0
#define RM_VGABLOWER_DEFAULT            14
#define RM_JITTERADJUSTMENT_DEFAULT     0
#define RM_AUTOPLAY_DEFAULT							RM_AUTOPLAY_ON

