/* $Id: WcharSinkP.h,v 1.1 2001/11/03 22:34:29 gray Exp $ 
 */
/*  Wchar Widgets
 *  Copyright (C) 1998 Gray
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _XawWcharSinkP_h
#define _XawWcharSinkP_h

/***********************************************************************
 *
 * WcharSink Object Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/TextSinkP.h> 
#include "WcharSink.h" 
#include "WcharCommonP.h"

/************************************************************
 *
 * New fields for the WcharSink object class record.
 *
 ************************************************************/

typedef struct {
  int foo;
} WcharSinkClassPart;

/* Full class record declaration */

typedef struct _WcharSinkClassRec {
    ObjectClassPart     object_class;
    TextSinkClassPart	text_sink_class;
    WcharSinkClassPart	wchar_sink_class;
} WcharSinkClassRec;

extern WcharSinkClassRec wcharSinkClassRec;

/* New fields for the WcharSink object record */
typedef struct {
    /* public resources */
    Boolean echo;
    Boolean display_nonprinting;
    XFontStruct *font8;
    XFontStruct	*font;		/* Kanji font */
    Pixel background;
    Pixel foreground;
    /* private state */
    GC normgc, invgc, xorgc;
    int row_org[2];
    int row_height;
    int avg_width;
    Pixmap cursor;
    XRectangle cursor_bounds;   /* Cursor location and size. */
    XawTextInsertState laststate;
} WcharSinkPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _WcharSinkRec {
    ObjectPart          object;
    TextSinkPart	text_sink;
    WcharSinkPart	wchar_sink;
} WcharSinkRec;

#endif /* _XawWcharSinkP_h */

