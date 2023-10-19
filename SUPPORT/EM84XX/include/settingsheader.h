/*****************************************************************************/

/*
 *      settingsheader.c -- Analog overlay settings header
 *
 *      Copyright (C) 2000 Sigma Designs
 *                    written by Nicolas Vignal <nicolas_vignal@sdesigns.com>
 *                    modified by Emmanuel Michon <emmanuel_michon@sdesigns.com>
 *                                Pascal Cannenterre <pascal_cannenterre@sdesigns.com>
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
#ifndef _SETTINGSHEADER_H
#define _SETTINGSHEADER_H

#define ANALOGREGISTRYFILE ".realmagicanalogoverlayrc"

// try to load and set the analog overlay parameters for this resolution
// return 0 if the resolution exist in the .realmagicanalogoverlayrc file
// and set up the parameters on the card
// You MUST connect the novaserver before using this function
// return :
// 1 : cannot open the file
// 2 : cannot find the resolution

DWORD LoadParamForThisResolution(DWORD xresolution, DWORD yresolution, DWORD depth, DWORD freq);

// save the parameters in the .realmagicanalogoverlayrc file
// you MUST connect the novaserver before using this function
// return
// 0 : Ok
// 1 : cannot open the file

DWORD SaveParamForThisResolution(DWORD xresolution, DWORD yresolution, DWORD depth, DWORD freq);

// returns screen resolution. We are talking about monitor resolution, which can
// be different from root window size (see Get/SetViewPort).

DWORD getres(DWORD *x, DWORD *y, DWORD *depth, DWORD *freq);

#endif

