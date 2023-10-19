/*****************************************************************************/

/*
 *     osdcontrol.c -- REALmagic OSD management functions
 *
 *      Copyright (C) 2001 Sigma Designs
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
// #include "define.h"

#define _DEFINE_FMP_TYPES_
#include "fmp.h"
#include "ifmp.h"
#include "osthread.h"
#include "osdsupport.h"
#include "handlemedia.h"

#include "gui/osd/palette.h"
#include "gui/osd/back.h"
#include "gui/osd/next.h"
#include "gui/osd/pause.h"
#include "gui/osd/play.h"
#include "gui/osd/reverse.h"
#include "gui/osd/skip.h"
#include "gui/osd/stop.h"
#include "gui/osd/close.h"
#include "gui/osd/open.h"
#include "gui/osd/stepff.h"
#include "gui/osd/steprw.h"
#include "gui/osd/slowff.h"
#include "gui/osd/slowrw.h"
#include "gui/osd/loading.h"
#include "gui/osd/font_name.h"


#define UNUSED_ARG 0xdeadbeef

// extern INFO Info;

DWORD MAX_HEIGHT    = 480;
DWORD MAX_WIDTH     = 720;
DWORD RIGHT_INDENT  = 72;
DWORD TOP_INDENT    = 60;
DWORD BOTTOM_INDENT = 60;

unsigned char *pictArr[15] = 
{ 
  NULL,         /*0*/
  play_osd,     /*1*/
  pause_osd,    /*2*/
  stop_osd,     /*3*/ 
  open_osd,     
  close_osd,    /*5*/
  reverse_osd,
  next_osd,
  back_osd,
  skip_osd,
  stepff_osd,
  steprw_osd,
  slowrw_osd,
  slowff_osd,
  loading_osd   /*14*/
};

BOOL initOSD(INFO *pInfo)
{
  if(FMPQueryInterface(IID_IOSD, (void **)(&(pInfo->pIOsd)))) 
    {
      fprintf (stderr, "Cannot set OSD");
      pInfo->pIOsd = NULL;
      return FALSE;
    }

  if(QFAILED(OSDModCreateInstance(0, &CLSID_CRMA, &IID_IRMA, 
				  (void**)(&(pInfo->pIRma))))) 
    {
      fprintf (stderr, "Cannot get instance of RMA interface");
      pInfo->pIRma = NULL;
      return FALSE;
    }
  
  return TRUE;
}

BOOL stopOSD(INFO *pInfo)
{
  if(pInfo->pIRma) 
    IRma_Delete((IObject *)(pInfo->pIRma), TRUE);
  
  
  return TRUE;
}


DWORD textToBitmap(IRma *pIRma,
                   BYTE**bmp,
                   char *string,
                   RMAYUVENTRY *palette,
                   PRMAFONT font) 
{
        RMAYUVENTRY *pPalette;
	RMARECT rect;
	RMAALIGNMENT al;
	RMASIZE textsize;
	ULONG size1;
        PBYTE tmp;
	
	pPalette = (RMAYUVENTRY *)palette;

////////////////////////////////////////////////////////////////////

	RmaGetTextSize(string, &textsize, font, 0);
	IRma_CreateDC(pIRma, COLORDEPTH_EIGHT_BPP, (WORD) (textsize.width + 4), 
		      (WORD) (textsize.height + 4), 255);
	IRma_SetPalette(pIRma, (HRMABITMAP) NULL, pPalette, 0, 256);
	IRma_SetPen(pIRma, RMAPEN_SOLID, 0x2a, 1);
	IRma_SetTextStyle(pIRma, RMAOPAQUE);
	IRma_SetTextColor(pIRma, 0x2a);
	IRma_SetTextBackground(pIRma, 0x01);
 	IRma_SetFont(pIRma, font);

	rect.left = 0;
	rect.right = textsize.width + 4;
	rect.top = 0;
	rect.bottom = textsize.height + 4;

	al = ALIGNMIDDLE|ALIGNCENTER;

	IRma_RMADrawText(pIRma, string, rect, al, 0);
	IRma_LockRmaOSD(pIRma, (HRMABITMAP) NULL, (PBYTE *)(&tmp), &size1);

	if((*bmp = calloc (size1, sizeof (unsigned char))))
		memcpy((void *)(*bmp), (void *)tmp, size1);
	else
		size1 = 0;
	IRma_DestroyDC(pIRma);
	return size1;
}

DWORD addPict(IRma *pIRma,
              BYTE *pict,
              BYTE *p1,
              BYTE *p2) 
{

	ULONG size;
	DWORD width, height, Width;
	RMAYUVENTRY *pPalette, onePE[1];
	RMAPOINT pos1;
	PBYTE tmp;
	RMARECT rect;

	if(!pict)
		return 0;

	if(p1 && p2) {
	
		onePE[0].Alpha = 0;
		onePE[0].yuvY = 0x0;
		onePE[0].yuvCr = 0x0;
		onePE[0].yuvCb = 0x0;

		width = (((DWORD)(p1[4])) << 8) | p1[5];
		Width = ((DWORD)p2[4] << 8) | (DWORD)p2[5];
		if(Width < width)
			Width = width;

		IRma_CreateDC(pIRma, COLORDEPTH_EIGHT_BPP, 
			      (WORD) Width, (WORD) MAX_HEIGHT, 255);
		IRma_GetPaletteFromBitmap(pIRma,
		                          (HRMABITMAP)p1,
		                          (PBYTE *)(&pPalette), (LONG *)(&size));
		IRma_SetPalette(pIRma, (HRMABITMAP)NULL, pPalette, 0, 255);
		IRma_SetPalette(pIRma, (HRMABITMAP)NULL, onePE, 255, 1);

		height = (((DWORD)(p1[6])) << 8) | p1[7];
		pos1.x = Width - width;
		pos1.y = 0;
		tmp = p1;
		IRma_LockRmaOSD(pIRma, (HRMABITMAP)tmp, &tmp, &size);
		rect.left = 0;
		rect.right = width - 1;
		rect.top = 0;
		rect.bottom = height - 1;
		RmaBitBlt(NULL, pIRma, (HRMABITMAP)tmp, (HRMABITMAP)NULL, 
			  pos1, rect, RMASRCCOPY, 0);
		
		/* RmaBitBlt(NULL, pIRma, (HRMABITMAP)tmp, (HRMABITMAP)NULL, */
		/* 			  pos1, rect, COLORKEYING, 0x004); */
		
		IRma_UnLockRmaOSD(pIRma, (HRMABITMAP)tmp, &tmp, &size);

		width = (((DWORD)(p2[4])) << 8) | p2[5];
		height = (((DWORD)(p2[6])) << 8) | p2[7];
		pos1.x = Width - width;
		pos1.y = MAX_HEIGHT - height;
		tmp = p2;
		IRma_LockRmaOSD(pIRma, (HRMABITMAP)tmp, &tmp, &size);
		rect.left = 0;
		rect.right = width - 1;
		rect.top = 0;
		rect.bottom = height - 1;
		
		RmaBitBlt(NULL, pIRma, (HRMABITMAP)tmp, (HRMABITMAP)NULL, 
			  pos1, rect, RMASRCCOPY, 0);
		
		/* RmaBitBlt(NULL, pIRma, (HRMABITMAP)tmp, (HRMABITMAP)NULL, */
		/*  			  pos1, rect, COLORKEYING, 0x004); */

		IRma_UnLockRmaOSD(pIRma, (HRMABITMAP)tmp, &tmp, &size);

		IRma_LockRmaOSD(pIRma, (HRMABITMAP)NULL, &tmp, &size);
		memcpy((void *)pict, (void *)tmp, size);
		IRma_DestroyDC(pIRma);
	}
	else if(!p1 && !p2) {
		pict[0] = 0;
		size = 0;
	}
	else {
		if(p1) 
		  tmp = p1;
		else 
		  tmp = p2;

		width = (((DWORD)(tmp[4])) << 8) | tmp[5];
		height = (((DWORD)(tmp[6])) << 8) | tmp[7];
		size = width * height + 1032;
		memcpy((void *)pict, (void *)tmp, size);
	}
	return size;
}



void setOSDwindow(OSD_WND *wnd, long nx, long ny, 
		  long nw, long nh, long al) 
{ 
  wnd->x = nx - (al >> 1)*nw;       
  wnd->y = ny - (al & 1)*nh;        
  wnd->w = nw; 
  wnd->h = nh;        
}

BYTE *bScale(PBYTE bmp, float sw, float sh) 
{
  PBYTE bitmap, pd, ps;
  DWORD width, height, size, w, h, i, j;

  w = (((DWORD)(bmp[4])) << 8) | bmp[5];
  h = (((DWORD)(bmp[6])) << 8) | bmp[7];
  width = ROUND(w * sw);
  height = ROUND(h * sh);
  size = width * height + 1032;
  
  if(!(bitmap = calloc (size, sizeof (unsigned char))) )
    {
      fprintf (stderr, "You're short in memory !!!");
      return NULL;
    }
  
  memcpy(bitmap, bmp, 1032);
  if(w == width && h == height) 
    {
      memcpy(bitmap + 1032, bmp + 1032, w * h);
      return bitmap;
    }
  
  bitmap[4] = (BYTE)(width >> 8);
  bitmap[5] = (BYTE)width;
  bitmap[6] = (BYTE)(height >> 8);
  bitmap[7] = (BYTE)height;
  bitmap[1] = (BYTE)(size >> 16);
  bitmap[2] = (BYTE)(size >> 8);
  bitmap[3] = (BYTE)size;
  pd = bitmap + 1032;
  
  if(w == width) {
    for(i = 0; i < height; i++) {
      ps = bmp + ROUND(i / sh) * w + 1032;
      memcpy(pd, ps, width);
      pd += width;
    }
  }
  else if(sw == 3.0f) {
    for(i = 0; i < height; i++) {
      ps = bmp + ROUND(i / sh) * w + 1032;
      
      for(j = 0; j < w; j++) {
	memset(pd, ps[j], 3);
	pd += 3;
      }
      
    }
  }
  else if(sw == 2.0f) {
    for(i = 0; i < height; i++) {
      ps = bmp + ROUND(i / sh) * w + 1032;
      
      for(j = 0; j < w; j++) {
	memset(pd, ps[j], 2);
	pd += 2;
      }
      
    }
  }
  else if(sw == 1.5f) {
    for(i = 0; i < height; i++) {
      ps = bmp + ROUND(i / sh) * w + 1032;
      
      for(j = 0; j < width; j += 3) {
	*pd++ = *ps++;
	memset(pd, *ps, 2);
	pd += 2;
	ps++;
      }
      
      if(w & 1) {
	memcpy(pd, ps, 2);
	pd += 2;
      }
      //			pd += width;
    }
  }
  else {
    for(i = 0; i < height; i++) {
      ps = bmp + ROUND(i / sh) * w + 1032;
      for(j = 0; j < width; j++) {
	*pd++ = ps[ROUND(j / sw)];
      }
      //			pd += width;
    }
  }
  return bitmap;
}

void displayOSD(IOsd *pIOsd, BYTE *p, long x, long y, 
		long al, float sw, float sh)
{
  if (pIOsd)
    {
      OSD_WND Wnd;
      DWORD width, height;
      int size;
      PBYTE pict = bScale(p, sw, sh);
      
      if(pict) 
	{                                                       
	  size = (((DWORD)(pict[1])) << 16) |
	    (((DWORD)(pict[2])) << 8) |
	    (DWORD)(pict[3]);
	  width = ((((DWORD)(pict[4])) << 8) | pict[5]);
	  height = ((((DWORD)(pict[6])) << 8) | pict[7]) - 2;
	  setOSDwindow(&Wnd, ROUND((x) * (sw)) & 0xfffffffe,
		       ROUND((y) * (sh)) & 0xfffffffe,
		       width, height, al);
	  /*DPRINT(("COsd__Write %dn", COsd__Write_c++));*/
	  IOsd_Write(pIOsd, pict, size);
	  IOsd_SetDest(pIOsd, &Wnd);
	  /*OSSleep(10);*/
	  IOsd_On(pIOsd);
	  free (pict);
	}
    }
}
  
void setHIGHLIGHT(IOsd *pIOsd, OSD_WND *pwnd, 
		   float sw, float sh)
{
  if(pIOsd) 
    {
      OSD_WND Wnd;                          
      Wnd.x = ROUND(sw * pwnd->x);
      Wnd.y = ROUND(sh * pwnd->y);
      Wnd.w = ROUND(sw * pwnd->w);
      Wnd.h = ROUND(sh * pwnd->h);
      IOsd_SetHli(pIOsd, &Wnd);
    }
}

/*
gint osd_delay_timeout (gpointer data)
{
  IOsd_Off(Info.pIOsd);
  GUI.osd_delay_tag = 0;
  return FALSE;
}
*/

void displaySimpleBitmap (INFO *Info, BYTE *pict1, int delay_in_seconds) {
	float scrH = 1.0f, scrW = 1.0f;
	int y, al;
	BYTE *tmppict = NULL;
	DWORD tmppictsize;

#define RIGHT_BOTTOM 3

	y = MAX_HEIGHT - BOTTOM_INDENT;
	al = RIGHT_BOTTOM;

	tmppict = calloc (MAX_WIDTH * MAX_HEIGHT + 1032,
				sizeof (unsigned char));
 
	tmppictsize = addPict (Info->pIRma, tmppict, pict1, NULL);
  
	if (tmppict[0]) {
		OSD_WND wnd;

		memset(&wnd, 0, sizeof(OSD_WND));
		displayOSD (Info->pIOsd, tmppict,
					(MAX_WIDTH - RIGHT_INDENT), 
					y, al, scrW, scrH);
		setHIGHLIGHT(Info->pIOsd, &wnd, scrW, scrH);

		//fprintf (stderr,"You should be able to see OSD by now ...\n");
	}

	free (tmppict);
}

void guiStateOSDdisplay(INFO *pInfo)
{
  DWORD picture_number = 0;
  
  switch(pInfo->playing)
    {
    case CLOSED:
      picture_number = 5;
      break;
    case STOPPED:
      picture_number = 3;
      break;
    case STOP_FOR_RESUME:
      picture_number = 3;
      break;
    case PAUSED:
      picture_number = 2;
      break;
    case PLAYING:
      picture_number = 1; 
     break;
    case TRICK_MODE:
      break;
    }
}
