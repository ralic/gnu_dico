/* $Id: WcharSink.h,v 1.1 2001/11/03 22:34:29 gray Exp $
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

#ifndef _XawWcharSink_h
#define _XawWcharSink_h

/***********************************************************************
 *
 * WcharSink Object
 *
 ***********************************************************************/

#include <X11/Xaw/TextSink.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 echo                Output             Boolean         True
 displayNonprinting  Output             Boolean         True
 kanjiFont           KanjiFont          XFontStruct *   XtDefaultFont
*/

#define XtCOutput "Output"

#define XtNdisplayNonprinting "displayNonprinting"
#define XtNecho "echo"

#define XtNkanjiFont "kanjiFont"
#define XtCKanjiFont "KanjiFont"

/* Class record constants */

extern WidgetClass wcharSinkObjectClass;

typedef struct _WcharSinkClassRec *WcharSinkObjectClass;
typedef struct _WcharSinkRec      *WcharSinkObject;

/************************************************************
 *
 * Public Functions.
 *
 ************************************************************/

#endif /* _XawWcharSrc_h */
/* DON'T ADD STUFF AFTER THIS #endif */
