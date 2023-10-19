/*****************************************************************************/

/*
 *      mpegsrv.h -- MPEG server on top of the NetStream 2000 driver
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
#ifndef __MPEGSRV_H__
#define __MPEGSRV_H__

void FMPcreatemessagequeue();

#define DEATHMESSAGE 0xdeaddead

extern int get_out_of_dummy;

#endif
