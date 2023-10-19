/*****************************************************************************/
/*
 *      rm84mpegcmn.h -- rm84cmn.h + mpegcmn.h         
 *
 *      Copyright (C) 2000 Sigma Designs
 *                    
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
#ifndef _LOCAL_TYPES_
#define _LOCAL_TYPES_
typedef unsigned long long DWORDLONG;
typedef unsigned long ULONG,*PULONG;
typedef long INT;
typedef unsigned long UINT;
typedef unsigned short USHORT;
typedef unsigned char BOOLEAN;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef void VOID, *PVOID, *HANDLE;
typedef DWORDLONG *PDWORDLONG;
#endif /* _LOCAL_TYPES_ */

#define _DWORD_
#define _WORD_

#include "mpegcmn.h"
// #include "rm84cmn.h"

typedef struct _FMPPROPERTY_INFO
{
  DWORD dwFlags;
  DWORD dwPropSet;
  DWORD dwPropId;
  DWORD dwPropFlags;
  RM_WRITE PropData;
  DWORD dwPropSizeIn;
  DWORD dwPropSizeOut;
}FMPPROPERTY_INFO;
