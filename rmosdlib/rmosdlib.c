/*
 *	rmosdlib.c
 *
 *	Copyright (c) 2002-2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>

/*
#define _DEFINE_FMP_TYPES_
#include "fmp.h"
#include "ifmp.h"
*/

#include "osthread.h"
#include "osdsupport.h"
#include "rmosdlib.h"


#define ROUND(x) ((DWORD)((x) + 0.5f))

struct rmosdlib_private_data_t {
	IRma			*pIRma;
	IOsd			*pIOsd;
}; 

const static DWORD	MAX_HEIGHT    = 480;
const static DWORD	MAX_WIDTH     = 680;
const static DWORD	RIGHT_INDENT  = 72;
const static DWORD	TOP_INDENT    = 60;
const static DWORD	BOTTOM_INDENT = 35;


/*
static void setHIGHLIGHT (IOsd *pIOsd, OSD_WND *pwnd, float sw, float sh) {
	if (pIOsd) {
		OSD_WND		Wnd;

		Wnd.x = ROUND(sw * pwnd->x);
		Wnd.y = ROUND(sh * pwnd->y);
		Wnd.w = ROUND(sw * pwnd->w);
		Wnd.h = ROUND(sh * pwnd->h);
		IOsd_SetHli(pIOsd, &Wnd);
	}
}
*/

static void setOSDwindow (OSD_WND *wnd, long nx, long ny,
					long nw, long nh, long al) {
	wnd->x = nx - (al >> 1)*nw;
	wnd->y = ny - (al & 1)*nh;
	wnd->w = nw;
	wnd->h = nh;
}

static void displayOSD (IOsd *pIOsd, BYTE *p, long x, long y, long al) {
	if (pIOsd) {
		OSD_WND		Wnd;
		DWORD		width, height;
		int		size;
		PBYTE		pict = p;

		if (pict) {
			size = (((DWORD)(pict[1])) << 16) |
				(((DWORD)(pict[2])) << 8) |
				(DWORD)(pict[3]);
			width = ((((DWORD)(pict[4])) << 8) | pict[5]);
			height = ((((DWORD)(pict[6])) << 8) | pict[7]) - 2;

			setOSDwindow (&Wnd, ROUND(x) & 0xfffffffe,
					ROUND(y) & 0xfffffffe,
					width, height, al);

			/*DPRINT(("COsd__Write %dn", COsd__Write_c++));*/

			IOsd_Write   (pIOsd, pict, size);
			IOsd_SetDest (pIOsd, &Wnd);

			/*OSSleep(10);*/

			IOsd_On (pIOsd);
		}
	}
}

static void displayBitmap (struct rmosdlib_t *self,
				BYTE *pict1, const int indent) {
	int		x, y, al;
	BYTE		*tmppict = pict1;

	/*
	y = MAX_HEIGHT - BOTTOM_INDENT;
	x = MAX_WIDTH  - RIGHT_INDENT;
	al = 3;
	*/
	/*
	x = 12;
	y = 45;
	al = 0;
	*/
	//x = 45;
	x = 40;
	y = MAX_HEIGHT - BOTTOM_INDENT - indent;
	al = 1;

	if (tmppict[0]) {
		OSD_WND		wnd;

		memset (&wnd, 0, sizeof(OSD_WND));
		displayOSD (self->pd->pIOsd, tmppict, x, y, al);
		// setHIGHLIGHT (self->pd->pIOsd, &wnd, scrW, scrH);
	}
}

static void rmosd_displayBitmap (struct rmosdlib_t *self, void *bmp,
					const int indent) {
	displayBitmap (self, bmp, indent);
}

static int rmosd_open (struct rmosdlib_t *self) {
	if (FMPQueryInterface (IID_IOSD, (void **)(&(self->pd->pIOsd)))) {
		fprintf (stderr, "Cannot set OSD");
		self->pd->pIOsd = NULL;
		return 0;
	}

	if (QFAILED (OSDModCreateInstance (0, &CLSID_CRMA,
				&IID_IRMA, (void**)(&(self->pd->pIRma))))) {
		fprintf (stderr, "Cannot get instance of RMA interface");
		self->pd->pIRma = NULL;
		return 0;
	}

	return 1;
}

static int rmosd_close (struct rmosdlib_t *self) {
	if (self->pd->pIRma) {
		IRma_Delete ((IObject *) (self->pd->pIRma), TRUE);
		self->pd->pIRma = NULL;
	}

	return 1;
}

static void rmosd_dispose (struct rmosdlib_t *self) {
	self->close (self);
	free (self);
}

static void rmosd_osd_off (struct rmosdlib_t *self) {
	IOsd_Off (self->pd->pIOsd);
}

static void rmosd_osd_on (struct rmosdlib_t *self) {
	IOsd_On (self->pd->pIOsd);
}

struct rmosdlib_t * new_rmosdlib (void) {
	struct rmosdlib_t	*self;

	while ((self = malloc (sizeof (struct rmosdlib_t))) != NULL) {
		if ((self->pd = malloc (
				sizeof (struct rmosdlib_private_data_t)))
				== NULL) {
			free (self);
			self = NULL;
			break;
		}

		self->pd->pIRma = NULL;
		self->pd->pIOsd = NULL;

		self->dispose		= rmosd_dispose;
		self->open		= rmosd_open;
		self->close		= rmosd_close;
		self->osd_on		= rmosd_osd_on;
		self->osd_off		= rmosd_osd_off;
		self->displayBitmap	= rmosd_displayBitmap;

		break;
	}

	return self;
}

