/*****************************************************************************/

/*
 *     fmpcontrol.h -- header file of Video, Sound, Overlay control functions
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
#ifndef __FMPCONTROL_H__
#define __FMPCONTROL_H__

#define AC3DTS     1
#define PCM        0
#define NO_KEYCOLOR 0xabcdefab

#ifndef UNUSED_ARG
#define UNUSED_ARG  0xdeadbeef
#endif

BOOL check_vga_cable_presence();
BOOL waitForAnalogOverlayAccess();
DWORD setOverlayDestination(DWORD xx, DWORD yy, DWORD width, DWORD height);
DWORD setOverlayResolution(DWORD xres, DWORD yres, DWORD depth);
DWORD setOverlayCalibration(DWORD color_code);
DWORD setOverlay(DWORD overlay_mode, DWORD key_color);
DWORD setMute(BOOL mute);
DWORD getMute(BOOL *mute);
DWORD setAudioBalance(DWORD volume, long balance);
DWORD getAudioBalance(DWORD *vol, long *bal);
DWORD setTvStandard(DWORD  newtvmode);
DWORD getTvStandard(DWORD *ovl);
DWORD getComponentMode(DWORD *ovl);
DWORD setComponentMode(DWORD  newoutputmode);
DWORD setOutputMode(DWORD newvgatv);
DWORD getOutputMode(DWORD *ovl);
DWORD setAudioChannelMode(DWORD mode);
DWORD getAudioChannelMode(DWORD *ovl);
DWORD setAudioAC3DTS(DWORD spdif);
DWORD getAudioAC3DTS(DWORD *ovl);
DWORD calibrationInit();
DWORD calibrationEnd();
DWORD getFMPProperty(DWORD, DWORD *);
DWORD setFMPProperty(DWORD, DWORD);
DWORD convertSecondstoHhMmSs(DWORD);

#endif /* _FMPCONTROL_H__ */
