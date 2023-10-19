/*****************************************************************************
*  IRmaOsd.h : OSD library interface.
*  REALmagic Quasar Hardware Library
*  Created by Vladislav Zamakhovsky
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 01/24/00
*  Description:
*****************************************************************************/

#ifndef __IRmaOsd_h__
#define __IRmaOsd_h__

#ifdef __cplusplus
extern "C"{
#endif 

typedef interface tagIRma
{
	CONST_VTBL struct tagIRmaVtbl __RPC_FAR *lpVtbl;
} IRma;

typedef struct tagIRmaVtbl
{
	BEGIN_INTERFACE

	void    (*Delete)               (IRma *This, BOOL bDeleteObject);

	QRESULT (*SetPalette)           (IRma *This, HRMABITMAP hBitmap, PRMAYUVENTRY pPalette,
	                                 LONG lFirstIndex, size_t NumberOfEntries);
	QRESULT (*GetPaletteFromBitmap) (IRma *This, HRMABITMAP hBitmap, PBYTE *ppPalette, LONG *NbOfByte);
	QRESULT (*CreateBitmap)         (IRma *This, RMACOMPRESSMODE ColorMode, RMACOLORINDEX Color, WORD wWidth, WORD wHeight, 
	                                 HRMABITMAP *phBitmap );
	QRESULT (*DisplayBitmap)        (IRma *This, HRMABITMAP hBitmap, RMAPOINT ptTopLeftPosition);
	
	QRESULT (*BitBlt)				(IRma *This, IRma *pIRMADest, HRMABITMAP hBitmapSrc, HRMABITMAP hBitmapDest, 
									 RMAPOINT ptTopLeftPositionDest, RMARECT SrcRect, BYTE Operation, 
									 RMACOLORINDEX Color);

	QRESULT (*GetBitmapFromDC)      (IRma *This, HRMABITMAP *phBitmap);
	QRESULT (*SetBitmap)            (IRma *This, HRMABITMAP *phBitmap);
	QRESULT (*DestroyBitmap)        (IRma *This, HRMABITMAP hBitmap);
	QRESULT (*SetPen)               (IRma *This, RMAPENSTYLE PenStyle, RMACOLORINDEX Color, WORD wWidth);
	QRESULT (*CreateDC)             (IRma *This, RMACOMPRESSMODE ColorMode, WORD wWidth, WORD wHeight, RMACOLORINDEX Color);
	QRESULT (*CreateDCFromBitmap)	  (IRma *This, HRMABITMAP hBitmap);
	QRESULT (*DestroyDC)            (IRma *This);
	QRESULT (*CompressBitmap)       (IRma *This, HRMABITMAP hBitmap, RMACOMPRESSMODE CompressMode);
	QRESULT (*ExtendBitmap)         (IRma *This, HRMABITMAP hBitmap, 
	                                 RMACOMPRESSMODE CompressMode, HRMABITMAP *phBitmap);
#ifdef WIN32
//	QRESULT (*ConvertYUVtoRGB)      (IRma *This, HRMAPALETTE hPalette, RGBQUAD *rgbPalette);
	QRESULT (*ConvertRGBtoYUV)      (IRma *This, RGBQUAD *rgbPalette, PRMAYUVENTRY pPalette);
	QRESULT (*ConvertRMABitmapToDib)(IRma *This, HRMABITMAP hBitmap, 
	                                 BITMAPFILEHEADER *bmpFileHdr, 
	                                 PBITMAPINFO *pDisplayBmi);
	QRESULT (*ConvertDibBitmapToRMA)(IRma *This, PBITMAPINFO pDisplayBmi,
	                                 HRMABITMAP *hBitmap, RMACOMPRESSMODE ColorMode);
#endif
	QRESULT (*DrawLine)             (IRma *This, RMAPOINT ptStart, RMAPOINT ptEnd);
	QRESULT (*DrawRectangle)        (IRma *This, RMARECT Rect, BOOL bFill);
	QRESULT (*DrawEllipse)          (IRma *This, RMARECT rect, double rotation, BOOL bFill);
	QRESULT (*DrawCircle)           (IRma *This, RMARECT Rect, BOOL bFill);
	QRESULT (*DrawArc)              (IRma *This, RMAPOINT ptStart, 
	                                 RMAPOINT ptEnd, double rotation, ARCTYPE ArcType);
	QRESULT (*DrawPie)              (IRma *This, RMARECT rect, LONG start_angle, LONG end_angle,
	                                 ARCTYPE ArcType, BOOL bFill);
	QRESULT (*DrawRoundRectangle)   (IRma *This, RMARECT Rect, BOOL bFill);
	QRESULT (*DisplayFont)          (IRma *This, HRMAFONT hFont, RMAPOINT ptTopLeftPosition);
	QRESULT (*SetFont)              (IRma *This, PRMAFONT pRmaFont);
	QRESULT (*GetFontSize)          (IRma *This, RMASIZE *pSize);
	QRESULT (*RMADrawText)          (IRma *This, PCHAR string, RMARECT rect, RMAALIGNMENT Alignment, WORD off);
	QRESULT (*SetTextStyle)         (IRma *This, RMATEXTSTYLE TextStyle);
	QRESULT (*SetTextColor)         (IRma *This, RMACOLORINDEX TextColor);
	QRESULT (*SetTextBackground)    (IRma *This, RMACOLORINDEX BackgroundColor);
	QRESULT (*GetTextSize)		    (IRma *This, PCHAR szText, RMASIZE *TextSize, PRMAFONT pRmaFont, WORD off);
	QRESULT (*LockRmaOSD)           (IRma *This, HRMABITMAP hBitmap, PBYTE *ppBitmap, LONG *pBitmapSize);
	QRESULT (*UnLockRmaOSD)         (IRma *This, HRMABITMAP hBitmap, PBYTE *ppBitmap, LONG *pBitmapSize);
	END_INTERFACE
} IRmaVtbl;

#define IRma_Delete(This, bDeleteObject) \
				(This)->lpVtbl->Delete(This, bDeleteObject)

#define IRma_SetPalette(This, hBitmap, pPalette, lFirstIndex, NumberOfEntries) \
				(This)->lpVtbl->SetPalette(This, hBitmap, pPalette, lFirstIndex, NumberOfEntries)

#define IRma_GetPaletteFromBitmap(This, hBitmap, ppPalette, pNbOfByte) \
				(This)->lpVtbl->GetPaletteFromBitmap(This, hBitmap, ppPalette, pNbOfByte)

#define IRma_CreateBitmap(This, ColorMode, Color, wWidth, wHeight, phBitmap)	\
				(This)->lpVtbl->CreateBitmap(This, ColorMode, Color, wWidth, wHeight, phBitmap)

#define IRma_DisplayBitmap(This, hBitmap, ptTopLeftPosition) \
				(This)->lpVtbl->DisplayBitmap(This, hBitmap, ptTopLeftPosition)

#define IRma_BitBlt(This, pIRMADest,hBitmapSrc, hBitmapDest, ptTopLeftPositionDest,SrcRect, Operation, Color) \
				(This)->lpVtbl->BitBlt(This, pIRMADest, hBitmapSrc, hBitmapDest, ptTopLeftPositionDest, SrcRect, Operation, Color)

#define IRma_GetBitmapFromDC(This, phBitmap) \
				(This)->lpVtbl->GetBitmapFromDC(This, phBitmap)

#define IRma_SetBitmap(This, phBitmap) \
				(This)->lpVtbl->SetBitmap(This, phBitmap)

#define IRma_DestroyBitmap(This, hBitmap) \
				(This)->lpVtbl->DestroyBitmap(This, hBitmap)

#define IRma_SetPen(This, PenStyle, Color, wWidth) \
				(This)->lpVtbl->SetPen(This, PenStyle, Color, wWidth)

#define IRma_CreateDC(This, ColorMode, wWidth, wHeight, Color) \
				(This)->lpVtbl->CreateDC(This, ColorMode, wWidth, wHeight, Color)

#define IRma_CreateDCFromBitmap(This, hBitmap) \
				(This)->lpVtbl->CreateDCFromBitmap(This, hBitmap)

#define IRma_DestroyDC(This) (This)->lpVtbl->DestroyDC(This)

#define IRma_CompressBitmap(This, hBitmap, CompressMode) \
				(This)->lpVtbl->CompressBitmap(This, hBitmap, CompressMode)

#define IRma_ExtendBitmap(This, hBitmap, CompressMode, phBitmap) \
				(This)->lpVtbl->ExtendBitmap(This, hBitmap, CompressMode, phBitmap)

#ifdef WIN32
/*
#define IRma_ConvertYUVtoRGB(This, hPalette, prgbPalette) \
				(This)->lpVtbl->ConvertYUVtoRGB(This, hPalette, prgbPalette)
*/
#define IRma_ConvertRGBtoYUV(This, prgbPalette, pPalette) \
				(This)->lpVtbl->ConvertRGBtoYUV(This, prgbPalette, pPalette)
#define IRma_ConvertRMABitmapToDib(This, hBitmap, pbmpFileHdr, pDisplayBmi) \
				(This)->lpVtbl->ConvertRMABitmapToDib(This, hBitmap, pbmpFileHdr, pDisplayBmi)
#define IRma_ConvertDibBitmapToRMA(This, pDisplayBmi, phBitmap, ColorMode)		\
				(This)->lpVtbl->ConvertDibBitmapToRMA(This, pDisplayBmi, phBitmap, ColorMode)
#endif

#define IRma_DrawLine(This, ptStart, ptEnd) \
				(This)->lpVtbl->DrawLine(This, ptStart, ptEnd)

#define IRma_DrawRectangle(This, Rect, bFill) \
				(This)->lpVtbl->DrawRectangle(This, Rect, bFill)

#define IRma_DrawEllipse(This, rect, rotation, bFill) \
				(This)->lpVtbl->DrawEllipse(This, rect, rotation, bFill)

#define IRma_DrawCircle(This, Rect, bFill ) \
				(This)->lpVtbl->DrawCircle(This, Rect, bFill)

#define IRma_DrawArc(This, ptStart, ptEnd, rotation, ArcType) \
				(This)->lpVtbl->DrawArc(This, ptStart, ptEnd, rotation, ArcType)	

#define IRma_DrawPie(This, rect, start_angle, end_angle, ArcType, bFill) \
				(This)->lpVtbl->DrawPie(This, rect, start_angle, end_angle, ArcType, bFill)

#define IRma_DrawRoundRectangle(This, Rect, bFill) \
				(This)->lpVtbl->DrawRoundRectangle(This, Rect, bFill)

#define IRma_DisplayFont(This, hDC, hFont, ptTopLeftPosition) \
				(This)->lpVtbl->DisplayFont(This, hDC, hFont, ptTopLeftPosition)

#define IRma_SetFont(This, pRmaFont) \
				(This)->lpVtbl->SetFont(This, pRmaFont)	

#define IRma_GetFontSize(This, pSize) \
				(This)->lpVtbl->GetFontSize(This, pSize)

#define IRma_RMADrawText(This, string, rect, Alignment, off)	\
				(This)->lpVtbl->RMADrawText(This, string, rect, Alignment, off)

#define IRma_SetTextStyle(This, TextStyle) \
				(This)->lpVtbl->SetTextStyle(This, TextStyle)

#define IRma_SetTextColor(This, TextColor) \
				(This)->lpVtbl->SetTextColor(This, TextColor)

#define IRma_SetTextBackground(This, BackgroundColor) \
				(This)->lpVtbl->SetTextBackground(This, BackgroundColor)	

#define IRma_GetTextSize(This, szText, pTextSize, pRmaFont, wOffset) \
				(This)->lpVtbl->GetTextSize(This, szText, pTextSize, pRmaFont, wOffset)

#define IRma_LockRmaOSD(This, hBitmap, ppBitmap, pBitmapSize) \
				(This)->lpVtbl->LockRmaOSD(This, hBitmap, ppBitmap, pBitmapSize)

#define IRma_UnLockRmaOSD(This, hBitmap, ppBitmap, pBitmapSize) \
				(This)->lpVtbl->LockRmaOSD(This, hBitmap, ppBitmap, pBitmapSize)

// Functions used when library statically linked
QRESULT RmaBitBlt(IRma *pIRMASrc, IRma *pIRMADest, HRMABITMAP hBitmapSrc, 
	HRMABITMAP hBitmapDest, RMAPOINT ptTopLeftPositionDest, RMARECT SrcRect, 
	BYTE Operation, RMACOLORINDEX Color);
QRESULT RmaGetTextSize(PCHAR szText, RMASIZE *TextSize, PRMAFONT pRmaFont, WORD offset);


#ifdef __cplusplus
}
#endif 

#endif
