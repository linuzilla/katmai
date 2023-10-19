/*****************************************************************************
*  IRmaDef.h : OSD library RMA definition.
*  REALmagic Quasar Hardware Library
*  Created by Vladislav Zamakhovsky
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 01/24/00
*  Description:
*****************************************************************************/

#ifndef __IRmaDef_h__
#define __IRmaDef_h__

#ifdef __cplusplus
extern "C"{
#endif 

/*/today******************************************************
typedef unsigned char       BYTE;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef long				LONG;
typedef char				CHAR;
typedef CHAR				*PCHAR;
typedef void				*PVOID;


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define FALSE               0
#define TRUE                1
//#define NULL				0

#define FAR                 far


*///end today***************************************************


typedef BYTE RMACOLORINDEX;		// Each index contains color & alpha blending level

typedef enum 
{
	HDCERR_SUCCESS = 0				// No error
} HDCERROR;

typedef enum
{
	COLORDEPTH_TWO_BPP = 0,		 // 4 colors (2 bit-per-pixel)
	COMPRESSMODE_TWO_BPP = 1,	 // Compress bitmap usimg Run-Length Coding in 2 bpp mode
	COLORDEPTH_FOUR_BPP = 2,	 // 16 colors (4 bit-per-pixel)
	COMPRESSMODE_FOUR_BPP = 3,   // Compress bitmap usimg Run-Length Coding in 4 bpp mode
//	COLORDEPTH_SEVEN_BPP = 4,	 // 128 colors (7 bit-per-pixel)
	COMPRESSMODE_SEVEN_BPP = 5,	 // Compress bitmap usimg Run-Length Coding in 7 bpp mode
	COLORDEPTH_EIGHT_BPP = 6	 // 256 colors (8 bit-per-pixel)
} RMACOMPRESSMODE;

typedef struct {
	BYTE Alpha;
	BYTE yuvY;
	BYTE yuvCb;
	BYTE yuvCr;
} RMAYUVENTRY, *PRMAYUVENTRY;
typedef DWORD HRMAYUVENTRY;

#define RMA_DEFAULT_PALETTE ((DWORD) &g_szDefaultPalette)

typedef struct {
	BYTE		 m_byMode;			// [..CCCBBR] where CCC = Palette size, BB = NB bit/pel, R = RLC?
	BYTE         m_bStructSize[3];  // This structure size (including m_Mode)
	WORD		 m_wWidth;		    // Bitmap width
	WORD	     m_wHeight;		    // Bitmap height
	unsigned char* raw;			    // The raw pointer points to an array of bytes,
                                    // which is the palette date + picture data 
} RMABITMAP, *PRMABITMAP;
typedef DWORD HRMABITMAP;		    // Handle to bitmap

typedef enum
{
	RMAPEN_NULL = 0,		// The pen doesn't draw anything
	RMAPEN_SOLID			// The pen draws a solid line
} RMAPENSTYLE;

typedef struct {
	DWORD          m_dwStructSize;	     // This structure size
	RMAPENSTYLE    m_PenStyle;			 // See RMAPENSTYLE enumeration 
	RMACOLORINDEX  m_Color;				 // Pen color
	WORD           m_wWidth;			 // Pen width
} RMAPEN, *PRMAPEN; 

typedef struct {
	LONG x;
	LONG y;

} RMAPOINT, *PRMAPOINT; 

typedef struct {
	LONG left;
	LONG top;
	LONG right; 
    LONG bottom;
} RMARECT, *PRMARECT; 

typedef struct {
	LONG width;
	LONG height;
} RMASIZE, *PRMASIZE; 

typedef enum 
{
	FONT_ASCIICODE = 0,
	FONT_UNICODE 				
} RMAFONTTYPE;

typedef struct
{
	int offsetX;			/*base-line of character */
	int offsetY;
	int width;				/*width of char in pixels (bits)*/
	int height;				/*height of char in pixels (bits)*/
	unsigned char* raw;		/*raw bitmap*/
} CharInfo;

typedef struct {
	DWORD	m_dwStructSize;		// This structure size
	PCHAR	m_szFontName;		// Font name used when using SetTextFont
	LONG	m_wWidth;			// Bitmap width
	LONG	m_wHeight;			// Bitmap height
	LONG	m_offsetX;			// X Displacement of the lower left corner from origin 0
								// (for the horizontal writing direction)
	LONG	m_offsetY;			// Y Displacement of the lower left corner from origin 0
	RMAFONTTYPE    m_Type;
	WORD	m_wFirstChar;		// First valid character
	WORD	m_wLastChar;		// Last valid character
	/*const*/ CharInfo* raw;	// The raw pointer points to an array of bytes,
                                // which is the data represents the picture itself
} RMAFONT, *PRMAFONT;

typedef DWORD HRMAFONT;			// Handle to font

typedef enum
{
	RMATRANSPARENT = 0,		
	RMAOPAQUE
} RMATEXTSTYLE;

typedef struct {
	DWORD			 m_dwStructSize;	     // This structure size
	RMATEXTSTYLE	 m_TextStyle;			 // See RMATEXTSTYLE enumeration
	RMACOLORINDEX	 m_TextColor;
	RMACOLORINDEX	 m_TextBackGround;
} RMATEXTATTRIBUTES, *PRMATEXTATTRIBUTES; 

typedef struct
{
	DWORD				m_dwStructSize;	     // This structure size
	HRMAFONT			m_hFont;
	RMATEXTATTRIBUTES	m_TextAttributes;
	RMAPEN				m_Pen;
//	HRMAPALETTE			m_hPalette;			// Show us which kind of palette do we have
	HRMABITMAP			m_hBitmap;			// Bitmap attached to display context
} RMADC, *PRMADC;
typedef DWORD HRMADC;			// Handle to display context

typedef enum {
	ARCTOPRIGHT,
	ARCBOTTOMRIGHT,
	ARCBOTTOMLEFT,
	ARCTOPLEFT
} ARCTYPE;

// Different text alignments
#define ALIGNLEFT		0x0001	//Horizontal alligment
#define ALIGNRIGHT		0x0002	
#define ALIGNMIDDLE		0x0004

#define ALIGNTOP		0x0008/*0x0000*/	//Vertical alligment
#define ALIGNBOTTOM		0x0010	
#define ALIGNCENTER		0x0020

typedef WORD RMAALIGNMENT;

//Options for BitBlt function

#define RMASRCCOPY			0x0
#define COLORKEYING		0x2

#ifdef __cplusplus
}
#endif

#endif

