/* $Id: WcharText.h,v 1.1 2001/11/03 22:34:29 gray Exp $ */
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

#ifndef _WcharText_h
#define _WcharText_h

#include <X11/Xaw/Text.h>
#include "WcharCommon.h"
/****************************************************************
 *
 * WcharText widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 font                Font               XFontStruct *   XtDefaultFont
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 foreground          Foreground         Pixel           XtDefaultForeground
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#define XtNkanjiFont "kanjiFont"

#define XtCKanjiFont "KanjiFont"

/* declare specific WcharTextWidget class and instance datatypes */

typedef struct _WcharTextClassRec*	WcharTextWidgetClass;
typedef struct _WcharTextRec*		WcharTextWidget;

/* declare the class constant */

extern WidgetClass wcharTextWidgetClass;

_XFUNCPROTOBEGIN

extern int WcharTextSetSource(
#ifdef NeedFunctionPrototypes
    Widget              /* w */,
    String              /* name */,
    ArgList             /* args */,
    Cardinal            /* num_args*/,
    XawTextPosition	/* pos */	      
#endif
);

extern Encoding WcharTextGetEncoding(
#ifdef NeedFunctionPrototypes
    Widget              /* w */
#endif
);


_XFUNCPROTOEND

#endif /* _WcharText_h */




