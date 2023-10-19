/*****************************************************************************
*  osdlib.h : Header file of the OSD Library
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 08/18/2000
*****************************************************************************/


#ifndef __OSDLIB_H_
#define __OSDLIB_H_

#ifdef __cplusplus
extern "C"{
#endif 

QRESULT OSDModCreateInstance(DWORD dwInstance, REFQCLSID rclsid, REFQIID riid, 
	LPVOID *ppv);

// Memory Helper
#ifdef DEBUG
	#define MemoryAlloc(x,y) OSDebugmalloc(x,y,TEXT(__FILE__),__LINE__)
	#define MEMALLOC(x,y) OSDebugmalloc(x,y,TEXT(__FILE__),__LINE__)
    #define MemoryFree(x) OSDebugfree(x)
    #define MEMFREE(x) OSDebugfree(x)
#else
	#define MemoryAlloc(x,y) OSmalloc(x)
	#define MEMALLOC(x,y) OSmalloc(x)
    #define MemoryFree(x) OSfree(x)
    #define MEMFREE(x) OSfree(x)
#endif

#ifdef __cplusplus
}
#endif 

#endif /*__OSDLIB_H_*/
