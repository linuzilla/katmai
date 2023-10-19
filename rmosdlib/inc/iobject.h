/*****************************************************************************
*  IObject.h : Type Definitions for IObject interface
*  REALmagic Quasar Hardware Library
*  Created by Michael Ignaszewski
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 8/23/99
*  Description:
*****************************************************************************/

#ifndef __IObject_h__
#define __IObject_h__

#ifdef __cplusplus
extern "C"{
#endif 

/*+-------------------------------------------------------------------------
*
*   Sigma Designs
*   Copyright 1999 Sigma Designs, Inc.
*
*--------------------------------------------------------------------------*/

#ifndef __IObject_INTERFACE_DEFINED
#define __IObject_INTERFACE_DEFINED

    typedef interface tagIObject
    {
        CONST_VTBL struct tagIObjectVtbl __RPC_FAR *lpVtbl;
    } IObject;

    typedef struct tagIObjectVtbl
    {
        void (*Delete)(IObject * This, BOOL bDeleteObject);
    } IObjectVtbl;

#define IObject_Delete(This,bDelete)	\
    (This)->lpVtbl->Delete(This,bDelete)

#endif 	/* __IObject_INTERFACE_DEFINED__ */

#ifdef __cplusplus
}
#endif

#endif
