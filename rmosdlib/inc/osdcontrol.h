/*****************************************************************************/

/*
 *     osdcontrol.h -- REALmagic OSD management header
 *
 *      Copyright (C) 2001-2002 Sigma Designs
 *             written by Pascal Cannenterre <pascal_cannenterre@sdesigns.com>
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
#ifndef __OSDCONTROL_H__
#define __OSDCONTROL_H__

extern RMAFONT _RMA_12x24;
extern unsigned char *pictArr[15];
extern unsigned char Palette[];

#define OSD_PICTS_FONT _RMA_12x24
#define FF_SLOW_LEFT_INDENT  8
#define FF_SLOW_RIGHT_INDENT 42
#define FF_SLOW_TOP_INDENT   5
#define LEFT_TOP     0
#define LEFT_BOTTOM  1
#define RIGHT_TOP    2
#define RIGHT_BOTTOM 3

#define ROUND(x) ((DWORD)((x) + 0.5f))

BOOL initOSD(INFO *);
BOOL stopOSD(INFO *);
DWORD textToBitmap(IRma *,unsigned char **, char *,
                   RMAYUVENTRY *, PRMAFONT);
DWORD addPict(IRma *, unsigned char *,
              unsigned char *, unsigned char *);

void displaySimpleBitmap(INFO *, BYTE *, int);

#endif /* OSDCONTROL_H */
