/* $Id: WcharTextP.h,v 1.1 2001/11/03 22:34:29 gray Exp $ */
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

#ifndef _WcharTextP_h
#define _WcharTextP_h

#include "WcharText.h"
#include "WcharCommonP.h"
/* include superclass private header file */
#include <X11/CoreP.h>

/* define unique representation types not found in <X11/StringDefs.h> */

#define XtRWcharTextResource "WcharTextResource"

typedef struct {
    int empty;
} WcharTextClassPart;

typedef struct _WcharTextClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
    WcharTextClassPart	wcharText_class;
} WcharTextClassRec;

extern WcharTextClassRec wcharTextClassRec;

typedef struct {
    /* resources */
    XFontStruct *font;
    XFontStruct	*kanji_font;		/* Kanji font */
    Pixel background;
    Pixel foreground;
    /* private state */
} WcharTextPart;

typedef struct _WcharTextRec {
    CorePart		core;
    SimplePart		simple;
    TextPart		text;
    WcharTextPart	wcharText;
} WcharTextRec;

#endif /* _WcharTextP_h */







