/*****************************************************************************/

/*
 *      hwiface.h -- OSD and I2C hardware interfaces definitions
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

#ifndef _HWIFACE_H
#define _HWIFACE_H

/* exported interfaces */
#define IID_IOSD     0x00000000
#define IID_IHDTV    0x00000001
#define IID_IUserI2C 0x00000002
#define IID_IGPIO	 0x00000003

#ifdef PRINCETON
#define IID_ILCD     0x00000004
#endif

///////////////////////////////////////////////////////////////////////
/* GPIO interface */
typedef struct tagIGPIO
{
	struct tagIGPIOVtbl *lpVtbl;
} IGPIO;

typedef struct tagIGPIOVtbl
{
	ULONG (*Write)(IGPIO *This, ULONG GPIO, ULONG Value);
	ULONG (*Read)(IGPIO *This, ULONG GPIO);
} IGPIOVtbl;

#define GPIO0	0
#define GPIO1	1
#define GPIO2	2
#define GPIO3	3
#define GPIO4	4
#define GPIO5	5
#define GPIO6	6
#define GPIO7	7
/* write a gpio hi or low */
#define IGPIO_Write(This, gpio, v)			(This)->lpVtbl->Write(This, gpio, v)
/* read a gpio value */
#define IGPIO_Read(This, gpio)				(This)->lpVtbl->Read(This, gpio)

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
/* osd interface */
typedef struct tagOSD_WND 
{
	LONG x;
	LONG y;
	LONG w;
	LONG h;
} OSD_WND;

typedef struct tagIOsd
{
	struct tagIOsdVtbl *lpVtbl;
} IOsd;

typedef struct tagIOsdVtbl
{
	ULONG (*On)(IOsd *This);
	ULONG (*Off)(IOsd *This);
	ULONG (*Flush)(IOsd *This);
	ULONG (*Write)(IOsd *This, BYTE *pBuf, ULONG BufLen);
	ULONG (*SetDest)(IOsd *This, OSD_WND *pWnd);
	ULONG (*SetHli)(IOsd *This, OSD_WND *pWnd);
	ULONG (*ShowSplash)(IOsd *This, PBYTE buf, DWORD width, DWORD height);
	ULONG (*SetFrameBuffer)(IOsd *This, DWORD PhysicalAddress, DWORD Length);
	ULONG (*ShowSplashEx)(IOsd *This, PBYTE buf, DWORD width, DWORD height, int x, int y);
} IOsdVtbl;

/* all return 0 for success, !0 for error */
/* turn osd on - a valid osd bitmap should have been sent already */
#define IOsd_On(This)						(This)->lpVtbl->On(This)
/* turn osd off - a valid osd bitmap should have been sent already */
#define IOsd_Off(This)						(This)->lpVtbl->Off(This)
/* flush osd - any bitmap in the osd buffer becomes invalid */
#define IOsd_Flush(This)					(This)->lpVtbl->Flush(This)
/* display an osd bitmap */
#define IOsd_Write(This,pBuf,BufLen)	(This)->lpVtbl->Write(This,pBuf,BufLen)
/* set the destination for the osd bitmap */
#define IOsd_SetDest(This,pWnd)			(This)->lpVtbl->SetDest(This,pWnd);
/* set the destination for the hi-light window */
#define IOsd_SetHli(This,pWnd)			(This)->lpVtbl->SetHli(This,pWnd);
/* show the splash screen - the driver should not be in a playing state (closed or stopped only) */
/* this will overwrite ANY mpeg data currently displayed */
#define IOsd_ShowSplash(This,buf,width,height) (This)->lpVtbl->ShowSplash(This,buf,width,height);
/* allows the driver to send the osd frame buffer to the em8400 */
#define IOsd_SetFrameBuffer(This,PhysicalAddress,Length) (This)->lpVtbl->SetFrameBuffer(This,PhysicalAddress,Length);
/* show the splash screen - the driver should not be in a playing state (closed or stopped only) */
/* this will overwrite ANY mpeg data currently displayed - this function allows you to specify an */
/* x and y offset */
#define IOsd_ShowSplashEx(This,buf,width,height,x,y) (This)->lpVtbl->ShowSplashEx(This,buf,width,height,x,y);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
/*
 31469 5994 640  480 525 6  9 30   800 59 16  85  
 31500 6000 640  480 525 6  9 30   800 59 16  85  
 31469 5994 720  480 525 6  9 30   880 59 16  85  
 31500 6000 720  480 525 6  9 30   880 59 16  85  
 37800 7200 720  480 			                  
 50400 9600 720  480 			                  
 44955 5994 720  720 			                  
 45000 6000 720  720 			                  
 54000 7200 720  720 			                  
 72000 9600 720  720 			                  
144955 5994 280  720 			                  
145000 6000 280  720 750 5  5 20  1650 70 40 260  
145000 6000 280  720 750 5  5 20  1800 70 40 350  
154000 7200 280  720 			                  
172000 9600 280  720 			                  
131469 5994 440  480 			                  
131500 6000 440  480 			                  
137800 7200 440  480 			                  
150400 9600 440  480 			                  
133750 6000 920 1080 1125 4 10 30 2200  44 44 192 
133750 6000 920 1080 1125 4 10 30 2400 144 44 292 
233750 6000 160 1080 1125 4 10 30 2440  44 44 192 
*/
/* hdtv interface */
typedef struct tagHDTV_PARAMS
{
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
	BOOL Interlaced;
} HDTV_PARAMS;

typedef struct tagIHdtv
{
	struct tagIHdtvVtbl *lpVtbl;		
} IHdtv;

typedef struct tagIHdtvVtbl
{
	ULONG (*Off)(IHdtv *This, DWORD outputMode);
	ULONG (*SetMode)(IHdtv *This, HDTV_PARAMS *pMode, DWORD outputMode);
	ULONG (*SetAspectRatio)(IHdtv *This, ULONG Width, ULONG Height, ULONG OutputAspect);
} IHdtvVtbl;

/* all return 0 for success, !0 for error */
/* set display back to ntsc/pal */
#define IHdtv_Off(This,opMode)					(This)->lpVtbl->Off(This,opMode);
/* set a hdtv mode */
#define IHdtv_SetMode(This,Mode,opMode)	(This)->lpVtbl->SetMode(This,Mode,opMode);
/* set the input/output aspect ratio */
#define IHdtv_SetAspectRatio(This,Width,Height,OutputAspect)	(This)->lpVtbl->SetAspectRatio(This,Width,Height,OutputAspect);
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

typedef struct tagIUserI2C
{
	struct tagIUserI2CVtbl *lpVtbl;		
} IUserI2C;

typedef struct tagIUserI2CVtbl
{
	void (*SetI2CAddress)(IUserI2C *This, BYTE WriteAddr, BYTE ReadAddr, BOOL bDelay);
	BOOL (*Write)(IUserI2C *This, BYTE adr, BYTE *pData, int n);
	BOOL (*Read)(IUserI2C *This, BYTE adr, BYTE *pData, int n);
#ifdef PRINCETON
	BOOL (*Inc)(IUserI2C *This, BYTE adr);
	BOOL (*Dec)(IUserI2C *This, BYTE adr);
	BOOL (*XFR)(IUserI2C *This, BYTE adr);
#endif
} IUserI2CVtbl;

/* set address of I2C device */
#define IUserI2C_SetI2CAddress(This,WrAddr,RdAddr,bDelay)	(This)->lpVtbl->SetI2CAddress(This,WrAddr,RdAddr,bDelay);
/* write; returns TRUE */
// "adr" = address of first register to write
// "pData" = pointer to "n" consecutive bytes to write
#define IUserI2C_Write(This,adr,pData,n)	(This)->lpVtbl->Write(This,adr,pData,n);
/* read; returns FALSE if n < 1 */
// "adr" = address of first register to read from
// "pData" = pointer to "n" consecutive bytes to write to
#define IUserI2C_Read(This,adr,pData,n)	(This)->lpVtbl->Read(This,adr,pData,n);

#ifdef PRINCETON
#define II2C_IncLCD(This,adr)	(This)->lpVtbl->Inc(This,adr);
#define II2C_DecLCD(This,adr)	(This)->lpVtbl->Dec(This,adr);
#define II2C_XFRLCD(This,adr)	(This)->lpVtbl->XFR(This,adr);
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" 
{
#endif
// call this function to get an interface
// returns 0 for success, !0 for no interface found
ULONG GetHwInterface (ULONG iid, void **ppv);
#ifdef __cplusplus
}
#endif

#endif
