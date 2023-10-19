/*****************************************************************************/

/*
 *     fmpovlctrl.h -- header file for Overlay config functions
 *
 *      Copyright (C) 2000 Sigma Designs
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
#ifndef __FMPOVLCTRL_H__
#define __FMPOVLCTRL_H__

#define UNDETERMINED_STREAM_TYPE 0xfeeddeaf
#define UNSUPPORTED_STREAM_TYPE  0xdeaeeeed

/* FMP Values limits ... */
/* FMPI_BRIGHTNESS */
#define BRT_MIN 0
#define BRT_MAX 1000
#define BRT_RESET 550
/* FMPI_CONTRAST */
#define CON_MIN 0
#define CON_MAX 1000
#define CON_RESET 500 
/* FMPI_SATURATION */
#define CSC_MIN 0
#define CSC_MAX 1000
#define CSC_RESET 500
/* FMPI_VOLUME */
#define VOL_MIN 0
#define VOL_MAX 100
#define VOL_RESET 70
#define BAL_RESET 0

typedef struct _FMPOVERLAYVDO_
{
  BOOL b_overlay_access;
  BOOL b_vga_cable_present;
  BOOL b_vga;               
  BOOL b_cfg_loaded;
  BOOL b_mute;
  BOOL b_fullscreen;

  DWORD key_color;
  DWORD overlay_mode;

  DWORD video_output_mode;
  DWORD tv_standard;
  DWORD tv_component;
  DWORD brt;
  DWORD sat;
  DWORD con;
  
  Display *dpy; /* X11 stuff */
  int scr;      /* X11 stuff */
  DWORD xres;
  DWORD yres;
  DWORD depth;
  DWORD freq;
  
  DWORD volume;
  long balance;
  DWORD audio_mode;
  DWORD audio_output;
} FMPOVERLAYVDO, *PFMPOVERLAYVDO;

extern const char * strStreamType[];

DWORD getStreamType(char *);
BOOL loadOverlayConfig(FMPOVERLAYVDO *);
DWORD autocalibrate(FMPOVERLAYVDO *);
void initialize_info_structure_file(INFO *);
void initialize_info_structure_disc(INFO *);
void reinitialize_info_structure_disc(INFO *);
void initialize_info_structure_rtsp(INFO *);
void initialize_gui_state(FMPOVERLAYVDO *);
void reinitialize_gui_state(FMPOVERLAYVDO *);

#endif /* __FMPOVLCTRL_H__ */
