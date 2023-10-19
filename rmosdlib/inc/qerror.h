/*****************************************************************************
*  qerror.h
*  REALmagic Quasar Hardware Library
*  Created by Aurelia Popa-Radu
*  Copyright Sigma Designs Inc
*  Sigma Designs Proprietary and confidential
*  Created on 8/27/99
*  Description:
*****************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif 

#ifndef __QERROR_H__
#define __QERROR_H__

//  Error code definitions(32 bit) for Quasar Library functions
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+---------------------------+-------------------------------+
//  |S|I|            R              |             ErrorCode         |
//  +-+-+---------------------------+-------------------------------+
//      S - is the severity code
//          0 - Success
//          1 - Error
//      I - is the suplementary info code
//          1 - a global string will contain some suplementary info
//          0 - No suplementary info
//      R - is a reserved
//      ErrorCode - is the error code

typedef LONG QRESULT;

typedef struct tagErrorEntry
{
	QRESULT Code;
	TCHAR* String;
} ErrorEntry;

#define QSEVERITY_SUCCESS			0L
#define QSEVERITY_ERROR				1L

#define QINFO						1L
#define QNOINFO						0L
// error codes
#define NO_ERROR					0L
#define MICROCODE_LOAD_FAILED		1L
#define GET_SYMBOL_FAILED			2L

#define QSUCCEEDED(Status)			((QRESULT)(Status) >= 0)
#define QFAILED(Status)				((QRESULT)(Status) < 0)
#define QRESULT_CODE(hr)			((qr) & 0xFFFF)
#define QRESULT_SEVERITY(qr)		(((qr) >> 31) & 0x1)
#define QRESULT_INFO(qr)			(((qr) >> 30) & 0x1)
#define MAKE_QRESULT(sev,info,code)	((QRESULT) (((unsigned long)(sev)<<31) | \
									((unsigned long)(info)<<30) | \
									((unsigned long)(code))) )
TCHAR* GetLastQError(QRESULT qr);


extern TCHAR g_InfoError[128];

// define QRESULTS for error codes
#define Q_OK							MAKE_QRESULT(QSEVERITY_SUCCESS, QNOINFO,0x0000)
#define Q_FAIL							MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0000)

// add next error to match ERROR_NOT_SUPPORTED=50L returned by  VxD for IOCTLs
#define E_NOT_SUPPORTED					MAKE_QRESULT(SEVERITY_SUCCESS, QNOINFO,	50)

#define E_TOO_MANY_INSTANCES			MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0001)
#define E_INVALID_PARAMETER				MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0002)

#define E_LOAD_MICROCODE_FAILED			MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0101)
#define E_GET_SYMBOLS_FAILED			MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0102)
#define E_MICROCODE_FAILED				MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0103)
#define E_VIDEO_WRITE_COMMAND_FAILED	MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x0104)

#define E_TEST_REG_FAILED				MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	0x1000)
#define E_TEST_PM_FAILED				MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	0x1001)
#define E_TEST_DM_FAILED				MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	0x1002)
#define E_TEST_DRAM_FAILED				MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	0x1003)
#define E_TEST_NOVA_FAILED				MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	0x1004)
#define E_EEPROM_NOT_PRESENT			MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	0x1005)
#define E_PROGRAM_EEPROM_FAILED			MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	0x1006)

#define RMAERRSTART 0x2000
#define RMA_CANNOT_ALLOCATE_MEM			MAKE_QRESULT(QSEVERITY_ERROR, QINFO,	RMAERRSTART+0)

#define RMA_EXCEED_DRAWING_SURFACE		MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	RMAERRSTART+1)
//If Src or Dest Bitmap or both of them compressed ( for DisplayBitmap and BitBlt functions )
#define RMA_INCORRECT_BITMAP_MODE		MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	RMAERRSTART+2)
//CLUT Src bitmap more than CLUT Dest Bitmap
#define RMA_BITMAP_NOT_COMPATIBLE		MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	RMAERRSTART+3)
//Src and Dest bitmaps have different color entries 
#define RMA_INCORRECT_PALETTE			MAKE_QRESULT(QSEVERITY_ERROR, QNOINFO,	RMAERRSTART+4)

#endif

#ifdef __cplusplus
}
#endif 

