/** @addtogroup  property 
    @{ */
/**
   @file rm84cmn.h 
   @brief comon definitions for EM84xx HwLib and user applications settings
   REALmagic Quasar Hardware Library
   @author Aurelia Popa-Radu
   Copyright Sigma Designs Inc
   Sigma Designs Proprietary and confidential
   @date 01/19/01
*/

#ifndef __RM84CMN_H__
#define __RM84CMN_H__

#ifdef __cplusplus
extern "C"{
#endif 

#ifndef KSPROPERTY_TYPE_GET
#define KSPROPERTY_TYPE_GET	0x00000001
#define KSPROPERTY_TYPE_SET	0x00000002
#endif

/**
	Hardware Library Property sets
*/
typedef enum {
	/** registry set common for all board versions*/
	REGISTRY_COMMON_SET = 1,
	/** registry set used for hdtv mode*/
	REGISTRY_HDTV_SET,	
	/** registry set used mostly for digital overlay */
	REGISTRY_VGAVENDOR_SET,	 
	
	/** property set for scan converter*/
	SCANCONVERTER_SET,	
	/** property set for eeprom*/
	EEPROM_SET,		
	/** property set for general properties of the board*/
	BOARDINFO_SET,		 
	/** property set for video properties of the board*/
	VIDEO_SET,		 
	/** property set for audio properties of the board*/
	AUDIO_SET,		
	/** property set for time properties of the board*/
	TIME_SET,
	/** property set for subpicture properties of the board*/
	SUBPICTURE_SET,
	/** property set for video decoder*/
	VIDEO_DECODER_SET,	 
	/** property set for mpeg encoder*/
	MPEG_ENCODER_SET,	 
	/** property set for DVI transmitter*/
	DVI_TRANSMITTER_SET,	 
	/** property set for Mpeg decoder*/
	DECODER_SET,		 
	/** property set for TV encoder*/
	TVENCODER_SET,
	/** property set for TvTuner encoder*/
	TVTUNER_SET,
	/** property set for OSD properties*/
	OSD_SET,
	/** property set for I2C properties*/
	I2C_SET,
	PROPERTY_MAX_SET
}PROPERTY_SETS;

/**
	Hardware Library Registry property set
	These values are set only at initialisation time.
*/
// REGISTRY_COMMON_SET uses RMuint32 = unsigned long = DWORD
typedef enum {
	eTvOut = 0,                 //? DEPRECATED
	
	/** @li 0 HwReset returns without resetting the hardware
	    @li 1 HwReset resets the hardware
	    @note The default value is 1. 
	*/
	eDoHwReset,                 //? RMuint32, if not 0 HwReset does hardware reset
	
	/** @li 0 HwReset leaves the Spdif output enabled
	    @li 1 HwReset disables the Spdif output
	    @note The default value is 1.	
	*/
	eDisableSpdifOutputInReset, //? RMuint32, if TRUE don't disable SPDIF output on reset
	
	/** The two values are used when playing on TV to set the
	    width of the active window.  They show the number of
	    active pixels displayed on TV (any value between 0...720)
	    for Ntsc / Pal. They will not affect the destination
	    window of the video or the scaling factors.  
	    @note The default value is 720.
	*/
 	eActiveVideoWidthNtsc,       //? RMuint32, number of pixels per line in NTSC mode
	eActiveVideoWidthPal,        //? RMuint32, number of pixels per line in PAL mode
	
	/** @li 1 for BROADCASTED_VIDEO -  the video is streamed and contains I blocks.
	    @li 0 for DVD_VIDEO - the video contains complete I frames. 
	    @note The default value is DVD_VIDEO.
	*/
	eBroadcastedVideo,           //? RMuint32, ???

	/** @li 1 for FORCED_PROGRESSIVE_OFF - displays progressive or interlaced like in the video stream
	    @li 0 for FORCED_PROGRESSIVE_ON - displays only progressive if the stream switches very fast from interlaced to progressive
	    @note The default value is 0. 
	*/
 	eForcedProgressiveSourceOff, //? RMuint32, ???

	/** @li 1 for PROGRESSIVE_ALWAYS - displays progressive for any video stream
	    @li 0 for PROGRESSIVE_MOVIES - displays progressive only for movies
	    @note The default value is 0. 
	*/
 	eForcedProgressiveAlways,    //? RMuint32, ???
	
	/** @li 1 for FRAME_DROP - it drops frames when it converts Ntsc clip to PAL TV.
	    @li 0 for FIELD_DROP - it drops fields when it converts Ntsc clip to PAL TV.
	    @note The default value is 0.
	*/
 	eNtscPalFrameDrop,           //? RMuint32, Drop frames when displaying NTSC (30 fps) content on PAL (25 fps)
	
	/** @li 1 for VGA_INTERLACED - displays only BOB on VGA
	    @li 0 VGA_INTERLACED - displays BOB or WEAVE on VGA, depending on stream
	    @note The default value is 0.
	*/
 	eVGAForcedInterlaced,        //? RMuint32, ???
	
	/** @li 0 for AUDIO_OUTPUT_STEREO
	    @li 1 for AUDIO_OUTPUT_AC3DTS
	    @note The default value is 0.
	*/
 	eAudioOutput,           //? RMuint32, ??? 
	
	/** audio volume right from 0 to 100
	    @note The default value is 100.
	*/
	eVolumeRight,           //? RMuint32: [0..100], left volume
	
	/** audio volume left from 0 to 100
	    @note The default value is 100. 
	*/
 	eVolumeLeft,            //? RMuint32: [0..100], right volume
	
	/** Used only for a specific hardware design (STPC - to select Pcm1716 or Pcm1720) 
	    @note The default value is 1720. 
	*/
 	eDacType,               //? RMuint32 ???

	/** @li 0 the audio I2S 16 or 24 bit will be selected by HwLib
	    @li 16 for audio I2S 16 bit
	    @li 24 for audio I2S 24 bit
	    @note The default value is 0. 
	*/
 	eAudioDacBitsPerSample, //? RMuint32: {8,16}, 

	/** This dword can be used for testing audio / video synchronization.
	    DoAudioLater is the number of PTS units that will be added to the audio PTS
	    from file send to the hardware. The HwLib will typecast DoAudioLater
	    to LONG - this means that negative values can be programmed.
	    The PTS unit is  11.(1) microseconds ( 90 kHz ).
	    @note The default value is 0.
	*/
 	eDoAudioLater,          //? RMuint32, ???
	
	/** values from 0 to 1000, used when SET_VGA is selected
	    @note The default value is 500.
	*/
 	eBrightness,   //? RMuint32: [0..1000], image brightness
	
	/** values from 0 to 1000, used when SET_VGA is selected
	    @note The default value is 500.
	*/
 	eContrast,     //? RMuint32: ]0..1000], image contrast
	
	/** values from 0 to 1000, used when SET_VGA is selected
	    @note The default value is 500.
	*/
 	eSaturation,   //? RMuint32: [0..1000], image saturation
	
	/** values from 0 to 1000, used when SET_TV is selected
	    @note The default value is 500.
	*/
 	eTvBrightness, //? RMuint32: [0..1000], tv out brightness
	
	/** values from 0 to 1000, used when SET_TV is selected
	    @note The default value is 500.
	*/
	eTvContrast,   //? RMuint32: [0..1000], tv out contrast
	
	/** values from 0 to 1000, used when SET_TV is selected
	    @note The default value is 500.
	*/
 	eTvSaturation, //? RMuint32: [0..1000], tv out saturarion

	/** It should be set according to VGA mode selected.
	    The default values are:
	    @notes	eDResBitsPerPixel= 8;
	    eDResScreenWidth = 1024;
	    eDResScreenHeight= 768;
	*/
 	eDResBitsPerPixel, //? RMuint32, ???
	eDResScreenWidth,  //? RMuint32, ???
	eDResScreenHeight, //? RMuint32, ???
	
	/** Used for analog overlay to program the pixel clock frequency for Nova chip.
	    At IDecoderBoard_HwReset time the analog overlay chip will detect the horizontal
	    frequency of the VGA mode and will program its PLL trying to match the VGA pixel
	    frequency. Because of not enough accuracy the result can be slightly different from
	    one detection to another and this causes a one pixel change in position of the video.
	    In order to avoid this, the user should save the horizontal frequency got from HwLib
	    in the registry and when the new detection will happen the HwLib will use the registry
	    value if the value is in a +/-200Hz range. If the value is too different the
	    auto-detected frequency will be used - the VGA mode was probably changed.
	    If the user don't use the value should be 0.
	    @notes The default value is 0. */
 	ePreviousHFreq,      //? RMuint32, ???
	
	/** Used for analog overlay to program the pixel clock frequency for Nova chip.
	    At IDecoderBoard_HwReset time the analog overlay chip will program the PLL frequency
	    based on horizontal frequency and an estimation of the pixels per line number.
	    For a better accuracy this number can be programmed by user in registry.
	    If the user don't use the value should be 0.
	    @notes The default value is 0. 
	*/
 	eTotalPixelsPerLine, //? RMuint32, ???

	/** @li 0 will not power off/on the EM8400 (no ACPI ON/OFF)
	    @li 1 IDecoder_Init will switch to ACPI_ON, IDecoder_Delete will switch to ACPI_OFF
	    @notes The default value is 0. 
	*/
 	eAcpiEnable,          //? RMuint32, ??? 
	
	/** Used for Ventura2k:
	    @li 0 EM8400 is master - EM8400 generates the HSync and VSync
	    @li 1 EM8400 is slave - EM8400 doesn't generate the HSync and VSync
	    @notest The default value is 0. 
	*/
 	eDecoderIsSlave,      //? RMuint32, Sets the TV encoder slave or master concerning synchro
	
	/** @li 0 no Zoom
	    @li 1 enable AcqWnd window to be the zoomed video window 
	*/
 	eZoomEnable,          //? RMuint32, ???
	
	/** @li 0 fullscreen on TV 
	    @li 1 enable destination window on TV 
	*/
 	eWindowTvEnable,      //? RMuint32, ???
	
	/** @li 0 fullscreen on HDTV
	    @li 1 enable destination window on HDTV 
	*/
 	eWindowHdtvEnable,    //? RMuint32, ???
	
	/** @li 0 OSD will be displayed relative to the video window 
	    @li 1 OSD will be displayed relative to the output device screen 
	*/
 	eOsdVideoIndependent, //? RMuint32, ???
	
	/** Used to program the digital video pixel clock Dvclk for EM9010.
	    @li 0 Dvclk will match the VGA pixel frequency
	    @li 1 Dvclk will be set to maximum limit 80000kHz = 80MHz
	    @li any required Dvclk in kHz, limited between hardware limits (30000 and 80000 for EM9010)
	    @notes The default value is 0. 
	*/
	eMaximumDvclk,        //? RMuint32 ???
	
	/** not a property */
	eCommonRegMax         //?
}REGISTRY_COMMON_SET_ID;

/**
	high definition television mode (HDTV) settings.
*/
// REGISTRY_HDTV_SET uses RMuint32 = unsigned long = DWORD
typedef enum {
	/** horizontal frequency in Hz*/
	eHdtvHFreq = 0,   //? RMuint32, ???

	/** vertical frequency in cHz (1Hz=100cHz)*/
	eHdtvVFreq,       //? RMuint32, ???

	/** number of visible video pixels per line*/
	eHdtvVideoWidth,  //? RMuint32, ???

	/** number of visible lines per frame*/
	eHdtvVideoHeight, //? RMuint32, ???

	/** number of pixels per line (visible + blanking)*/
	eHdtvHSyncTotal,  //? RMuint32, ???

	/** number of pixels between visible and HSync*/
	eHdtvPreHSync,    //? RMuint32, ???

	/** number of pixels in HSync active signal*/
	eHdtvHSyncActive, //? ???

	/** number of pixels between HSync and visible*/
	eHdtvPostHSync,   //? ???

	/** number of lines per frame (visible + blanking)*/
	eHdtvVSyncTotal,  //? RMuint32, ???

	/** number of lines between visible and VSync*/
	eHdtvPreVSync,    //? RMuint32, ???

	/** number of lines in VSync active signal*/
	eHdtvVSyncActive, //? ???

	/** number of lines between VSync and visible*/
	eHdtvPostVSync,   //? ???

	/** pixel frequency resulted from previous settings*/
	eHdtvPixelFreq,   //? RMuint32, ???

	/** interlaced mode, resulted from VideoHeight and the HFreq/VFreq*/
	eHdtvInterlaced,  //? ???

	/** not a property */
	eHdtvRegMax       //? ???
}REGISTRY_HDTV_SET_ID;

/** VGA specific settings (Digital Video Port) */
// REGISTRY_VGAVENDOR_SET uses RMuint32 = unsigned long = DWORD
typedef enum {
	/** @li 0 for top-bottom display
	    @li 1 for bottom-top display
	    @note The default value is 0. It depends on the VGA card. 
	*/
 	eInvertField = 0, //? ??? 
	
	/** @li 0 for 8bits
	    @li 1 for 16 bits
	    @note The default value is 0.
	*/
 	eVmi_16bits,      //? ???
	
	/** @li  one of CCIR_601 or CCIR_656
	    @note The default value is CCIR_601 for analog overlay and CCIR_656 for digital overlay.
	*/
 	eCcir_656,        //? ???
	
	/** @li 2: VSync enabled, HSync disabled, VVLD/HS=HS enabled,
	    @li 1: VSync enabled, HSync enabled, VVLD/HS=VVLD enabled, 
	    @li 0: VSync, HSync, VVLD/HS=VVLD disabled.
	    @note The default value is 1 for analog overlay and 0 for digital overlay.
	*/
 	eSyncEnable,      //? RMuint32, ???
	
	/** @li 1 enables VIP 2.0
	    @note The default value is 0.
	*/
 	eVip20,           //? ???

	/** not a property */
	eVgaVendorRegMax  //? ???
}REGISTRY_VGAVENDOR_SET_ID;

// SCANCONVERTER_SET uses RMuint32 = unsigned long = DWORD
typedef enum {
	escAccessRegister = 0,	     //? ADDR_DATA
	escResetDefault,             //?
	escBrightness,               //?
	escContrast,                 //?
	escSaturation,               //?
	escSharpness,                //?
	escFlicker,                  //?
	escChromaFilter,             //?
	escLumaFilter,               //?
	escNtscPedestal,             //?
	escMacrovision,              //?
	escTvStandard,               //? 
	escOutputFormat,             //?
	escHwOutputHorzOffsShadow,   //?
	escHwOutputVertOffsShadow,   //?
	escTVPixels,                 //?
	escTVLines,                  //?
	escHorizontalPositionOffset, //?
	escVerticalPositionOffset,   //?
	escHorizontalScaleStep,      //?
	escVerticalScaleStep,        //?
	escHorizontalPanPosition,    //?
	escVerticalPanPosition,      //?
	escZoom,                     //?
	escScanConverterMax          //?
}SCANCONVERTER_SET_ID;

// EEPROM_SET
typedef enum {
	eEepromAccess = 0, //? ADDR_DATA
	eEepromMax         //?
}EEPROM_SET_ID;

// VIDEO_DECODER_SET uses RMuint32 = unsigned long = DWORD
typedef enum {
	evdAccessRegister = 0, //? ADDR_DATA
	evdTvStandard,
	evdAudioClock,
	evdInputSource,
	evdMax                 //?
}VIDEO_DECODER_SET_ID;

// MPEG_ENCODER_SET
typedef enum {
	emeAccessRegister = 0, //? ADDR_DATA
	emeMax                 //?
}MPEG_ENCODER_SET_ID;

// DVI_TRANSMITTER_SET
typedef enum {
	edtAccessRegister = 0, //? ADDR_DATA
	edtMax                 //? 
}DVI_TRANSMITTER_SET_ID;

// BOARDINFO_SET uses RMuint32 = unsigned long = DWORD
typedef enum {
	ebiDeviceId = 0,	//? RMuint32, only get (equivalent with MpegAttrGetSubID) ???
	ebiSubId,			//? RMuint32, only get (equivalent with MpegAttrGetSubID) ???
	ebiVersion,			//? RMuint32, only get ???
	ebiAPMState,		//? ???
	ebiPIOAccess,		//? ADDR_DATA. get changes PIO in input, set changes PIO in output ???
	ebiHwLibVersion,	//? RMuint32, only get (equivalent with MpegAttrCodeVersion)
	ebiUcodeVersion,	//? RMuint32, only get
	ebiCommand,			//? RMuint32 ???
	eBoardInfoMax		//? ???
}BOARDINFO_SET_ID;

// VIDEO_SET uses u.ulData in DWORD
typedef enum {

	/** Scart Output ( TV output )
	    @li evScartOutput_COMPOSITE
	    @li evScartOutput_RGB
	    @li evScartOutput_DISABLE
	    @note The default value is evScartOutput_DISABLE 
	*/
	evScartOutput = 0, // evScartOutput_type

	/** Output Device for video display
	    @li evOutputDevice_VGA
	    @li evOutputDevice_TV
	    @li evOutputDevice_HDTV
	    @note The default value is evOutputDevice_VGA for analog boards and evOutputDevice_VGATV for digital boards. 
	*/
	evOutputDevice, //? evOutputDevice_type (equivalent with MpegAttrVideoTv & VIDEOOUT_MASK) ???

	/** TV output format (TV output)
	    @li evTvOutputFormat_COMPOSITE
	    @li evTvOutputFormat_COMPONENT_YUV
	    @li evTvOutputFormat_COMPONENT_RGB
	    @li evTvOutputFormat_OUTPUT_OFF
	    @li evTvOutputFormat_COMPONENT_RGB_SCART
	    @note The default value is evTvOutputFormat_COMPOSITE. 
	*/
	evTvOutputFormat, //? evTvOutputFormat_type (equivalent with MpegAttrVideoTv & COMPONENT_MASK) ???
	
	/** TV standard (TV output)
	    @li evTvStandard_NTSC
	    @li evTvStandard_PAL
	    @li evTvStandard_PAL60
	    @li evTvStandard_PALM
	    @note The default value is evTvStandard_NTSC. 
	*/
	evTvStandard,   //? evTvStandard_type  (equivalent with MpegAttrVideoTv & STANDARDTV_MASK) ???

	evBrightness,	//?	RMuint32: [0..1000], brightness (equivalent with MpegAttrVideoBrightness)
	evContrast,		//?	RMuint32: [0..1000], contrast	(equivalent with MpegAttrVideoContrast)
	evSaturation,	//?	RMuint32: [0..1000], saturation	(equivalent with MpegAttrVideoSaturation)

	/** Video input aspect ratio
	    @li evInAspectRatio_4x3
	    @li evInAspectRatio_16x9
	    @note The default value is evInAspectRatio_4x3. 
	*/
	evInAspectRatio,	//?	evInAspectRatio_type: 	(equivalent with MpegAttrVideoAspectRatio)

	/** Video input standard ( based on frame rate and vertical width )
	    @li evInStandard_NTSC
	    @li evInStandard_PAL
	    @note The default value is evInStandard_NTSC. 
	*/
	evInStandard,		//?	evInStandard_type: 	(equivalent with MpegAttrVideoMode, MpegAttrVideoRate)

	/** Video output display option 
	    @li evOutDisplayOption_Normal
	    @li evOutDisplayOption_16x9to4x3_PanScan
	    @li evOutDisplayOption_16x9to4x3_LetterBox
	    @li evOutDisplayOption_4x3to16x9_HorzCenter
	    @li evOutDisplayOption_4x3to16x9_VertCenter
	    @note The default value is evOutDisplayOption_Normal. 
	*/
	evOutDisplayOption,	//?	evOutDisplayOption_type: 	(equivalent with MpegAttrVideoOutputMode)

	/** Source video window - {0,0, MpegWidth, MpegHeight}
	    @note Default Source window is 720x480
	*/
	evSourceWindow,	// Wnd_type, set/get

	/** Zoomed video window - it should be inside the SourceWindow
	    @note Default the Zoomed video window is equal with SourceWindow 
	*/
	evZoomedWindow,	// Wnd_type, set/get

	/** Maximum  output display resolution
	    @note It depends on the evOutputDevice selected. 
	*/
	evMaxDisplayWindow,		// Wnd_type, only get

	/** Video destination
	    @note For full screen get evMaxDisplayWindow and set evDestinationWindow
	*/
	evDestinationWindow,	// Wnd_type, set/get

	evCustomHdtvParams,		// CustomHdtvParams_type, set/get, (equivalent with MpegAttrHDTVMode)

	/** Video speed 
	    @note The default value is 0 for normal speed playback
	*/
	evSpeed,			// evSpeed_type (equivalent with MpegAttrVideoSpeed)

	evCaptureParams,        // evCaptureParams_type, get only
	evCaptureFormat,        // evCaptureFormat_type
	evCaptureFrame,         // variable buffer size = dwBufferSize get from evCaptureParams_type

	evFrameNumberInGOP,     // MpegAttrVideoFrameNumberInGOP

	/** AnalogCable Presence
	    @li evAnalogCable_NotConnected
	    @li evAnalogCable_Connected
	    @note The default value is evAnalogCable_Connected 
	*/
	evAnalogCable,		// evAnalogCable_type, get only

	/** DigitalCable Presence
	    @li evDigitalCable_NotConnected
	    @li evDigitalCable_Connected
	    @note The default value is evDigitalCable_Connected 
	*/
	evDigitalCable,		// evDigitalCable_type, get only

	/** ScartAspectRatio ( TV output )
	    @li evScartAspectRatio_Auto
	    @li evScartAspectRatio_4x3
	    @li evScartAspectRatio_16x9
	    @note The default value is evScartAspectRatio_Auto 
	*/
	evScartAspectRatio, // evScartAspectRatio_type, set and get

	/** Video frame counter gives the number of frame displayed.
	    @note The default value after stop is 0.
	*/
	evFrameCounter,     // RMuint32, get only (similar with MpegAttrVideoFramePosition)

	evMax
/*
	evSpeed2,		//	MpegAttrVideoSpeed2
	evEnableSync,		//	MpegAttrVideoEnableSync
	evStill,		//	MpegAttrVideoStill
	evMacrovisionFlags,	//	MpegAttrVideoMacrovisionFlags
	evForcedProgressive,    //	MpegAttrForcedProgressive
*/
}VIDEO_SET_ID;

// AUDIO_SET
typedef enum {
	eAudioFineVcxo = 0,         //? RMuint32, ???int
	eAudioVcxo,                 //? ???,  VCXO_SET
	eaAc3Conf,                  //? ???,  AC3_CONF
	eAudioMode,                 // eAudioMode_type
	eaInOutConfig,              // AudioInOutConfig_type

	/** audio volume right from 0 to 100
	    @note The default value is 50.
	*/
	eaVolumeRight,           //? RMuint32: [0..100], right volume

	/** audio volume left from 0 to 100
	    @note The default value is 50.
	*/
	eaVolumeLeft,                // RMuint32: [0..100], left volume

	eAudioFormat,                // eAudioFormat_type
	eAudioSampleRate,            // RMuint32
	eAudioNumberOfChannels,      // RMuint32: 1,2,...,8
	eAudioNumberOfBitsPerSample, // RMuint32: 24, 20 or 16.

	/** eAudioDacOutput enable or disable the I2S output
	    @li eAudioDacOutput_Disable
	    @li eAudioDacOutput_Enable
	    @note The default value is eAudioDacOutput_Enable.
	*/
	eAudioDacOutput,             // eAudioDacOutput_type

	/** eAudioSpdifOutput enable or disable the SPDIF output
	    @li eAudioSpdifOutput_Disable
	    @li eAudioSpdifOutput_Enable
	    @note The default value is eAudioSpdifOutput_Enable.
	*/
	eAudioSpdifOutput,           // eAudioSpdifOutput_type

	/** eAudioDigitalOutput selects if the audio data are decoded in Pcm or just passtrough
	    @li eAudioDigitalOutput_Pcm
	    @li eAudioDigitalOutput_Compressed
	    @note The default value is eAudioDigitalOutput_Pcm.
	*/
	eAudioDigitalOutput,         // eAudioDigitalOutput_type, (MpegAttrAudioOutput)
	eaMax
}AUDIO_SET_ID;

// SUBPICTURE_SET
typedef enum {
	/**
	   RMuint32.
	   This represents what will be set in the control register of the
	   SubPicture block of the chip. This register is named SP_CTRL.
	   For reference, here is its content:
	   15:9 --> Reserved.
	   8    --> Button Enable (Enables Highlight).
	   7    --> Field Enable (1: top, 0: Bottom) - microcode use
	   6    --> Field Alternate (1: Alternate, 0:Same) - microcode use
	   5    --> Vertical Downsampling Enable - microcode use
	   4    --> Horizontal Downsampling Enable - microcode use
	   3    --> Vertical Upsampling Enable - microcode use
	   2    --> Horizontal Upsampling Enable - microcode use
	   1    --> SubPicture Enable.
	   0    --> Reset - microcode use
	   The user can change through HwLib only bits 8 and 1 to enable/disable
	   the subpicture and the hilites = buttons.
	 */
	eSubpictureCmd,
	/**
	   An array of 16*4 bytes layed out as:
	   RYUV RYUV RYUV ... RYUV
	   Where each letter represents one byte.
	   R means reserved.
	   Y is the Y color component.
	   U is the U color component.
	   V is the V color component.
	 */
	eSubpictureUpdatePalette, 
	/**
	   a eSubpictureUpdateButton_type
	 */
	eSubpictureUpdateButton,
	eSubpictureMax               //? ???
}SUBPICTURE_SET_ID;
	
// TIME_SET uses DWORD
typedef enum {
	etimPcrInfo = 0,	       //? ???, PCR_INFO
	etimPcrTime,		       //? ???, TIME_INFO
	etimSystemTimeClock,	       //? ???, TIME_INFO - replace the old Read/WriteSCR
	etimLastDecodedVideoFrameTime, //? ???, TIME_INFO - replace the old ReadHwPts45k
	etimLastDecodedAudioFrameTime, //? ???, TIME_INFO
	etimVOPTimeIncrRes,            //? ???
	etimVideoCTSTimeScale,         //? ???
	etimAudioCTSTimeScale,         //? ???
	etimMax                        //? ??? 
}TIME_SET_ID;

// DECODER_SET uses DWORD
typedef enum {
	edecAudioInOutConfig = 0,	//? ???
	edecAudioDacBitsPerSample,	//? ??? 
	edecVideoStd,				//? ???
	edecOsdFlicker,				//? ???
	edecPciBurst,				//? ???
	edecForceFixedVOPRate,		//? ???, FIXED_VOP_RATE
	edecAccessRegister,			//? ADDR_DATA
	edecCSSChlg,				//? ???, only get.
	edecCSSKey1,				//? ???, only set.
	edecCSSChlg2,				//? ???, only set.
	edecCSSKey2,				//? ???, only get.
	edecCSSDiscKey,				//? ???, only set.
	edecCSSTitleKey,			//? ???, only set. 
	edecMax						//? ???
}DECODER_SET_ID;

// TVENCODER_SET uses DWORD
typedef enum {
	etvNtscPedestal = 0,	// etvNtscPedestal_type
	etvLumaFilter,			// etvLumaFilter_type
	etvChromaFilter,		// etvChromaFilter_type
	etvColorBars,			// etvColorBars_type
	etvMax					// 
}TVENCODER_SET_ID;

// TVTUNER_SET
typedef enum {
       eTvTunerChannel = 0,      // uses struct eTvTunerChannel_type
       eTvTunerMax               //? ??? 
}TVTUNER_SET_ID;

typedef enum {
	/**
	   Set OSD to On, Off or Flush its buffer.
	 */
	eOsdCommand,            // edecOsdCommand_type
	/**
	   Set the OSD Hilight window. The window coordinates
	   are relative to the OSD bitmap.
	   In the highlight window, if the OSD is stored in 2,
	   4, or 7 BPP mode, the colors are looked up inthe 
	   upper half of the CLUT (Color LookUp Table).
	   ie: in 2bpp mode, the color 0 for highlight
	   will be looked up in index 4 of the CLUT.
	      
	 */
	eOsdHiLiWindow,             // Wnd_type
	/**
	   Set the position of the OSD bitmap relative to the current
	   coordinate system.
	 */
	eOsdDestinationWindow,      // Wnd_type
	eOsdMax
} OSD_SET_ID;

typedef enum {
	eI2cInit = 0,               // I2cInit_type, set only
	eI2cAccess,                 // I2cReadWrite_type + variable size buffer
								// set will write I2C, get will read
	eI2cMax                     // 
}I2C_SET_ID;

/************************************************************************************/
/************************************************************************************/
/*********************** Possible values ********************************************/
/*********************** Do not define SET_IDs above this limit *********************/
/************************************************************************************/

#ifndef SCART_MASK
/****************************** HACK *******************************************/
 // (hack) we have to link mpegcmn.h to here.

#define SCART_MASK      0x0003
#define SCART_DISABLE   0x0000
#define SCART_COMPOSITE 0x0001
#define SCART_RGB       0x0003

#define VIDEOOUT_MASK   0x0021
#define SET_VGA         0x0000
#define SET_TV          0x0001
#define SET_HDTV        0x0020

#define STANDARDTV_MASK 0x010A
#define SET_NTSC        0x0000
#define SET_PAL         0x0002
#define SET_PAL60       0x0008
#define SET_PALM        0x000A
#define SET_480P        0x0100	// used only for Vga scan conversion

#define COMPONENT_MASK      0x02C0
#define COMPOSITE           0x0000
#define COMPONENT_YUV       0x0080
#define COMPONENT_RGB       0x00C0
#define OUTPUT_OFF          0x0040
#define COMPONENT_RGB_SCART 0x0200

#define AUDIO_FORMAT_MPEG1       1
#define AUDIO_FORMAT_MPEG2       2
#define AUDIO_FORMAT_AC3         3
#define AUDIO_FORMAT_PCM         4
#define AUDIO_FORMAT_DTS         5
#define AUDIO_FORMAT_DVD_AUDIO   6
#define AUDIO_FORMAT_REVERSE_PCM 7
#define AUDIO_FORMAT_AAC         8

#define VIDEO_ASPECT_RATIO_4_3   2
#define VIDEO_ASPECT_RATIO_16_9  3

#define MPEG_ATTRIBUTE_VIDEO_PAL  1
#define MPEG_ATTRIBUTE_VIDEO_NTSC 2

#define VIDEO_OUTPUT_MODE_NORMAL_OR_WIDE 0
#define VIDEO_OUTPUT_MODE_PAN_SCAN       1
#define VIDEO_OUTPUT_MODE_LETTERBOX      2
#define VIDEO_OUTPUT_MODE_HORZCENTER     3
#define VIDEO_OUTPUT_MODE_VERTCENTER     4

// flags used to enable/disable the hardware audio outputs
#define AUDIO_DAC_ENABLE						0x0001
#define AUDIO_DAC_DISABLE						0x0000
#define AUDIO_SPDIF_ENABLE						0x0002
#define AUDIO_SPDIF_DISABLE						0x0000

#define AUDIO_OUTPUT_STEREO						0
#define AUDIO_OUTPUT_AC3DTS						1

#endif // SCART_MASK

#define BTN_ENABLE 0x0100
#define SP_ENABLE 0x0002
/****************************** HACK *******************************************/

// For Xcard the user should select one of:
//		- evScartOutput_COMPOSITE or 
//		- evScartOutput_RGB.
// The Xcard scart output is enabled only if both next conditions are met:
//   - evTvOutputFormat is set to evTvOutputFormat_COMPONENT_RGB_SCART
//	 - evOutputDevice is set to evOutputDevice_TV

// For NE2000TV card the user should select one of:
//		-  evScartOutput_DISABLE (scart output is disabled) or
//		- any of evScartOutput_COMPOSITE or evScartOutput_RGB (scart output is enabled).
 
typedef enum 
{
	evScartOutput_COMPOSITE	= SCART_COMPOSITE,
	evScartOutput_RGB       = SCART_RGB,
	evScartOutput_DISABLE   = SCART_DISABLE
} evScartOutput_type;

typedef enum 
{
	evScartAspectRatio_Auto = 0,
	evScartAspectRatio_4x3,
	evScartAspectRatio_16x9,
} evScartAspectRatio_type;

typedef enum 
{
	evOutputDevice_VGA  = SET_VGA,
	evOutputDevice_TV   = SET_TV,
	evOutputDevice_HDTV = SET_HDTV
} evOutputDevice_type;

typedef enum 
{
	evTvStandard_NTSC  = SET_NTSC,
	evTvStandard_PAL   = SET_PAL,
	evTvStandard_PAL60 = SET_PAL60,
	evTvStandard_PALM  = SET_PALM
} evTvStandard_type;

typedef enum 
{
	evTvOutputFormat_COMPOSITE           = COMPOSITE,
	evTvOutputFormat_COMPONENT_YUV       = COMPONENT_YUV,
	evTvOutputFormat_COMPONENT_RGB       = COMPONENT_RGB,
	evTvOutputFormat_COMPONENT_RGB_SCART = COMPONENT_RGB_SCART,
	evTvOutputFormat_OUTPUT_OFF          = OUTPUT_OFF
} evTvOutputFormat_type;

typedef enum
{
	evInAspectRatio_4x3  = VIDEO_ASPECT_RATIO_4_3,
	evInAspectRatio_16x9 = VIDEO_ASPECT_RATIO_16_9
} evInAspectRatio_type;

typedef enum
{
	evOutDisplayOption_Normal               = VIDEO_OUTPUT_MODE_NORMAL_OR_WIDE,
	evOutDisplayOption_16x9to4x3_PanScan    = VIDEO_OUTPUT_MODE_PAN_SCAN,
	evOutDisplayOption_16x9to4x3_LetterBox  = VIDEO_OUTPUT_MODE_LETTERBOX,
	evOutDisplayOption_4x3to16x9_HorzCenter = VIDEO_OUTPUT_MODE_HORZCENTER,
	evOutDisplayOption_4x3to16x9_VertCenter = VIDEO_OUTPUT_MODE_VERTCENTER
} evOutDisplayOption_type;

typedef enum
{
	evInStandard_NTSC = MPEG_ATTRIBUTE_VIDEO_NTSC,
	evInStandard_PAL  = MPEG_ATTRIBUTE_VIDEO_PAL
} evInStandard_type;

typedef DWORD evSpeed_type;  // see cqsrbrd.c for peculiar values to set here.		
                             // 1 to 255 = slow speed (from slowest to normal)
                             // 0 and 256 = normal speed
                             // 257 to 1024 = fast speed (from normal to fastest)

typedef enum
{
	YUV_420 = 0,
	BGR_24BITS_BMP,
	PALETTE_INDEX_BMP,
} evCaptureFormat_type;

typedef struct
{
	WORD wWidth;			// Picture width in pixels
	WORD wHeight;			// Picture height in lines
	WORD wPictureType;		// 0/1 for frame/field
	DWORD dwBufferSize;		// size of the buffer to receive a complete frame according to the CaptureFormat already set
} evCaptureParams_type;

typedef enum 
{
	evAnalogCable_NotConnected = 0,
	evAnalogCable_Connected
} evAnalogCable_type;

typedef enum 
{
	evDigitalCable_NotConnected  = 0,
	evDigitalCable_Connected
} evDigitalCable_type;

typedef enum {
	eAudioFormat_MPEG1       = AUDIO_FORMAT_MPEG1, // mpeg1 layer 1.
	eAudioFormat_MPEG2       = AUDIO_FORMAT_MPEG2, // mpeg1 layer 2.
	eAudioFormat_AC3         = AUDIO_FORMAT_AC3,
	eAudioFormat_PCM         = AUDIO_FORMAT_PCM,
	eAudioFormat_DTS         = AUDIO_FORMAT_DTS,
	eAudioFormat_DVD_AUDIO   = AUDIO_FORMAT_DVD_AUDIO,
	eAudioFormat_REVERSE_PCM = AUDIO_FORMAT_REVERSE_PCM,
	eAudioFormat_AAC         = AUDIO_FORMAT_AAC,
	eAudioFormat_MPEG1_LAYER3,  // mpeg1 layer 3.
	eAudioFormat_UNKNOWN, 
} eAudioFormat_type;

typedef enum {
	eAudioDigitalOutput_Pcm        = AUDIO_OUTPUT_STEREO,
	eAudioDigitalOutput_Compressed = AUDIO_OUTPUT_AC3DTS
} eAudioDigitalOutput_type;

typedef enum {
	eAudioSpdifOutput_Disable = AUDIO_SPDIF_DISABLE,
	eAudioSpdifOutput_Enable  = AUDIO_SPDIF_ENABLE
} eAudioSpdifOutput_type;

typedef enum {
	eAudioDacOutput_Disable   = AUDIO_DAC_DISABLE,
	eAudioDacOutput_Enable    = AUDIO_DAC_ENABLE
} eAudioDacOutput_type;

typedef enum {
	ebiAPMState_Suspend,       //? ???
	ebiAPMState_ResumeSuspend, //? ???
	ebiAPMState_Standby,       //? ???
	ebiAPMState_ResumeStandby, //? ???
	ebiAPMState_Max            //? ???
} ebiAPMState_type;

typedef enum 
{
	edecVideoStd_MPEG12Video = 0,
	edecVideoStd_MPEG4Video  = 1,
} edecVideoStd_type;

typedef struct {	// used for PropId=etimPcrTime in PropSet=TIME_SET
	DWORD dwTimeResolution_Hz;	// 90000 for 90k pts unit
	ULONGLONG dwlTime;		// Pts
} etimSystemTimeClock_type;  //TIME_INFO; // (hack) we have to link mpegcmn.h to here.

typedef struct {
	DWORD	dwForceFixedVOPRate;	// command for set, status for get
	DWORD	dwFixedVopTimeIncr;		// ticks/VOP, default 1000
	DWORD	dwFixedTimeIncrRes;		// ticks/sec, default 24000
} edecForceFixedVOPRate_type; //FIXED_VOP_RATE, *PFIXED_VOP_RATE; // (hack) we have to link qhwlib.h to here.

typedef BYTE edecCSSChlg_type[10];
typedef BYTE edecCSSKey1_type[5];
typedef BYTE edecCSSChlg2_type[10];
typedef BYTE edecCSSKey2_type[5]; 
typedef BYTE edecCSSDiscKey_type[2048];
typedef BYTE edecCSSTitleKey_type[5];

typedef enum {
	eSubpictureCmd_BTN_ENABLE = BTN_ENABLE,
	eSubpictureCmd_SP_ENABLE  = SP_ENABLE,
} eSubpictureCmd_type;
	
typedef struct {
	BYTE yuvReserved;
	BYTE yuvY;
	BYTE yuvCr;
	BYTE yuvCb;
} eSubpictureUpdatePalette_type[16];

typedef struct {
	LONG leftb;
	LONG topb;
	LONG rightb;
	LONG bottomb;
	LONG wColor;
	LONG wContrast;
} eSubpictureUpdateButton_type;

typedef enum {
	ebiCommand_HardwareReset = 1
} ebiCommand_type;

typedef struct {
	DWORD Addr;
	DWORD Data;
} genericReg_type; // hack link qhwinc.h to here.

typedef struct {
	LONG x;
	LONG y;
	LONG w;
	LONG h;
} Wnd_type; // hack link qhwinc.h to here.

typedef struct {
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
} CustomHdtvParams_type; // same as HDTV_MODE in mpegcmn.h.

typedef enum 
{
	etvNtscPedestal_OffJapan = 0,
	etvNtscPedestal_OnUSA
} etvNtscPedestal_type;

typedef enum 
{
	// For EM847x Extended means 6.75MHz, LowPass 4.5MHz
	etvLumaFilter_CompositeExtended_ComponentExtended = 0,	// default
	etvLumaFilter_CompositeLowPass_ComponentExtended,		// not recommended
	etvLumaFilter_CompositeExtended_ComponentLowPass,		// not recommended
	etvLumaFilter_CompositeLowPass_ComponentLowPass			// not recommended
} etvLumaFilter_type;

typedef enum 
{
	// For EM847x Extended means 6.75MHz/2
	etvChromaFilter_CompositeDefaultLowPass_ComponentExtended = 0,	// default
	etvChromaFilter_CompositeExtended_ComponentExtended				// not recommended
} etvChromaFilter_type;

typedef enum 
{
	etvColorBars_Off = 0,	// default
	etvColorBars_On
} etvColorBars_type;

typedef enum {
	evdInputSource_COMPOSITE=0,
	evdInputSource_SVIDEO,
	evdInputSource_ONBOARDTUNER,
} evdInputSource_type;

typedef evTvStandard_type evdTvStandard_type; // The tv _encoder_ setting uses the same enums as tv _decoder_

typedef DWORD evdAudioClock_type; // use only 32000, 44100, 48000 values.

typedef enum {
	evTvTunerRange_NTSC_MNAIR,               // ntsc m/n air (antenna) channels
	evTvTunerRange_NTSC_MNCABLE,             // ntsc m/n cable channels
} eTvTunerRange_type;                      // for struct eTvTunerChannel_type

typedef struct {
	eTvTunerRange_type evTvTunerRange_value; // uses enum eTvTunerRange_type
	BYTE eTvTunerChannelInRange;             // [2..69] ntsc m/n air, [1..125] ntsc m/n cable
} eTvTunerChannel_type;

typedef enum
{
	eAudioInOutDefault = 0,
	eAudioInOutVcxoJda1Ckin,
	eAudioInOutExternJda1Ckin,
	eAudioInOutSCkinCapture
} AudioInOutConfig_type;

typedef enum	// used for PropId = eAudioMode in PropSet=AUDIO_SET
{
	eAudioMode_Stereo = 0,
	eAudioMode_MonoRight,
	eAudioMode_MonoLeft,
	eAudioMode_MonoMix
}eAudioMode_type;

typedef enum	// used for PropId = eaAc3Conf in PropSet=AUDIO_SET
{
	eAc3SpeakerSurround = 0,
	eAc3Speaker1_0,
	eAc3Speaker2_0
}Ac3SpeakerConfig;

typedef enum	// used for PropId = eaAc3Conf in PropSet=AUDIO_SET
{
	eAc3ComprDiagNormOff = 0,
	eAc3DiagNormOn,
	eAc3LineOut,
	eAc3RFModulation
}Ac3ComprDlgNorm;

typedef struct tagAC3_CONF	// used for PropId = eaAc3Conf in PropSet=AUDIO_SET
{
	WORD wStructureVersion;			// 0
	BYTE bAc3ComprDlgNorm;			// HwLib default eAc3LineOut
	BYTE bAc3SpeakerConfig;			// HwLib default eAc3SpeakerSurround
	WORD wAc3HiLoDynamicRange;		// HwLib default 0xFFFF
	WORD wAc3RepeatCounter;			// HwLib default 0x0000
}AC3_CONF;

typedef enum {
	eOsdOn = 0,
	eOsdOff,
	eOsdFlush
} eOsdCommand_type;

typedef DWORD evFrameNumberInGOP_type;

typedef struct {
	WORD PIO_clock;
	WORD PIO_data;
} I2cInit_type;

// base structure structure for Read/Write I2C. 
typedef struct {
	WORD I2c_WriteAddress;	// write address of the I2C device. Reserve WORD instead of BYTE for later extensions
	WORD I2c_ReadAddress;	// read address of the I2C device. Reserve WORD instead of BYTE for later extensions
	WORD I2c_usDelay;		// Delay in microseconds
	WORD SubAddress;		// subadress.  Reserve WORD instead of BYTE for later extensions, like 0xFFFF for no subaddress
	WORD nBytes;			// size of the BYTE buffer that follow this structure
} I2cReadWrite_type;

#ifdef __cplusplus
}
#endif 
/** @} */

#endif // __RM84CMN_H__
