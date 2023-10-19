/*****************************************************************************/

/*
 *      sduuids.h -- SD unique identifiers definitions
 *
 *      Copyright (C) 1999-2000 Sigma Designs
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

#ifndef RM_LIB_QGUID_ENTRY
typedef unsigned long QGUID;
#ifndef INITGUID
	#define RM_LIB_QGUID_ENTRY(name, l) \
	    EXTERN_C QGUID name;
#else
	#define RM_LIB_QGUID_ENTRY(name, l) \
        EXTERN_C QGUID name \
                = { l };
#endif // INITGUID
#endif

RM_LIB_QGUID_ENTRY(IID_IRMA,					0x00000001)

RM_LIB_QGUID_ENTRY(CLSID_CRMA,					0x00010000)

