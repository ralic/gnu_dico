/* $Id: WcharSink.c,v 1.1 2001/11/03 22:34:15 gray Exp $ */
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/ObjectP.h>
#include <WcharSinkP.h>
#include <cursor.xbm>
#define GetSource(w,pos,tblk,length) \
 XawTextSourceRead(XawTextGetSource(XtParent(w)), pos, tblk, length)

static void WcharSinkInitialize(
#if NeedFunctionPrototypes
    Widget,
    Widget,
    ArgList,
    Cardinal*
#endif
);
static void WcharSinkDestroy(
#if NeedFunctionPrototypes
    Widget,
    XtPointer,
    XtPointer
#endif
);
static Boolean WcharSinkSetValuesHook(
#if NeedFunctionPrototypes
    Widget,
    ArgList,
    Cardinal *
#endif
);
static void WcharSinkDisplayText(
#if NeedFunctionPrototypes
    Widget            /* w */,
    Position          /* x */,
    Position          /* y */,
    XawTextPosition   /* pos1 */,
    XawTextPosition   /* pos2 */,
    Boolean           /* highlight */
#endif
);
static void WcharSinkInsertCursor(
#if NeedFunctionPrototypes
    Widget            /* w */,
    Position          /* x */,
    Position          /* y */,
    XawTextInsertState /* state */
#endif
);
static void WcharSinkClearToBackground(
#if NeedFunctionPrototypes
    Widget            /* w */,
    Position          /* x */,
    Position          /* y */,
    Dimension         /* width */,
    Dimension         /* height */
#endif
);
static void WcharSinkFindPosition(
#if NeedFunctionPrototypes
    Widget            /* w */,
    XawTextPosition   /* fromPos */, 
    int               /* fromX */,
    int	              /* width */,
    Boolean           /* stopAtWordBreak */,
    /* return: */
    XawTextPosition * /* pos_return */,
    int *             /* width_return */,
    int *             /* height_return */
#endif
);
static void WcharSinkFindDistance(
#if NeedFunctionPrototypes
    Widget            /* w */,
    XawTextPosition   /* fromPos */,
    int               /* fromX */,
    XawTextPosition   /* toPos */,
    /* Return: */
    int *             /* width */,
    XawTextPosition * /* pos */,
    int *             /* height */
#endif
);
static void WcharSinkResolve(
#if NeedFunctionPrototypes
    Widget            /* w */,
    XawTextPosition   /* fromPos */,
    int	              /* fromX */,
    int	              /* width */,
    /* return: */
    XawTextPosition * /* pos */
#endif
);
static int WcharSinkMaxLines(
#if NeedFunctionPrototypes
    Widget            /* w */,
    Dimension         /* height */
#endif
);
static int WcharSinkMaxHeight(
#if NeedFunctionPrototypes
    Widget            /* w */,
    int               /* lines */
#endif
);
static void WcharSinkSetTabs(
#if NeedFunctionPrototypes
    Widget            /* w */,
    int               /* tab_count */,
    int *             /* tabs */
#endif
);
static void WcharSinkGetCursorBounds(
#if NeedFunctionPrototypes
    Widget            /* w */,
    XRectangle *      /* rect */
#endif
);
static int MeasureText(
#if NeedFunctionPrototypes
    WcharSinkObject   /* w */,
    XawTextPosition   /* fromPos */,
    XawTextPosition   /* toPos */,
    XawTextPosition * /* resPos */
#endif
);
static int CheckPrivateData(
#if NeedFunctionPrototypes
    WcharSinkObject   /* w */
#endif
);

static XtResource resources[] = {
#define off(field) XtOffset(WcharSinkObject, wchar_sink.field)
    {XtNecho, XtCOutput, XtRBoolean, sizeof(Boolean),
	 off(echo), XtRImmediate, (XtPointer)True},
    {XtNdisplayNonprinting, XtCOutput, XtRBoolean, sizeof(Boolean),
	 off(display_nonprinting), XtRImmediate, (XtPointer) True},
    {XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct*),
	 off(font8), XtRString, (XtPointer) XtDefaultFont},
    {XtNkanjiFont, XtCFont, XtRFontStruct, sizeof(XFontStruct*),
	 off(font), XtRString, (XtPointer) XtDefaultFont},
    {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	 off(background), XtRString, XtDefaultBackground},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	 off(foreground), XtRString, XtDefaultForeground},
#undef off
};

typedef int (*IFUN)();
typedef void (*VFUN)();
    
WcharSinkClassRec wcharSinkClassRec = {
    { /* Object class */
      /* superclass             */      (WidgetClass) &textSinkClassRec,
      /* class_name             */      "WcharSink",
      /* widget_size            */      sizeof(WcharSinkRec), 
      /* class_initialize       */      NULL,
      /* class_part_initialize  */      NULL,
      /* class_inited           */      FALSE,
      /* initialize             */      WcharSinkInitialize,
      /* initialize_hook        */      NULL,
      /* obj1                   */      NULL,
      /* obj2                   */      NULL, 
      /* obj3                   */      0,
      /* resources              */      resources,
      /* num_resources          */      XtNumber(resources),
      /* xrm_class              */      NULLQUARK,
      /* obj4                   */      0,
      /* obj5                   */      0,
      /* obj6                   */      0,
      /* obj7                   */      0,
      /* destroy                */      NULL,
      /* obj8;                  */      NULL,
      /* obj9;                  */      NULL,
      /* set_values             */      NULL,
      /* set_values_hook        */      WcharSinkSetValuesHook,
      /* obj10                  */      NULL,
      /* get_values_hook        */      NULL,
      /* obj11                  */      NULL,
      /* version                */      XtVersion,
      /* callback_private       */      NULL,
      /* obj12                  */      NULL,
      /* obj13                  */      NULL,
      /* obj14                  */      NULL,
      /* extension              */      NULL,
    },
    { /* TextSink Class */
      /* DisplayText            */      (VFUN) WcharSinkDisplayText,        
      /* InsertCursor           */	(VFUN) WcharSinkInsertCursor,       
      /* ClearToBackground      */	(VFUN) WcharSinkClearToBackground,
      /* FindPosition           */	(VFUN) WcharSinkFindPosition,       
      /* FindDistance           */	(VFUN) WcharSinkFindDistance,       
      /* Resolve                */	(VFUN) WcharSinkResolve,            
      /* MaxLines               */	(IFUN) WcharSinkMaxLines,           
      /* MaxHeight              */	(IFUN) WcharSinkMaxHeight,          
      /* SetTabs                */	(VFUN) WcharSinkSetTabs,
      /* GetCursorBounds        */	(VFUN) WcharSinkGetCursorBounds,    
    },
    { /* WcharSinkClass */
      /* foo                    */      0,
    }
};

WidgetClass wcharSinkObjectClass = (WidgetClass)&wcharSinkClassRec;

void
WcharSinkInitialize(request, new, args, num_args)
    Widget request;
    Widget new;
    ArgList args;
    Cardinal *num_args;
{
    WcharSinkObject obj = (WcharSinkObject)new;

    XtAddCallback(new, XtNdestroyCallback, WcharSinkDestroy, NULL);

    /* Set up private data
     */
        /*GC normgc, invgc, xorgc; Pixmap insertCursorOn;*/
    obj->wchar_sink.normgc = NULL;
}

/*ARGSUSED*/
void
WcharSinkDestroy(w, closure, call_data)
    Widget w;
    XtPointer closure, call_data;
{
    WcharSinkObject obj = (WcharSinkObject)w;
    if (obj->wchar_sink.normgc) {
	XFreeGC(XtDisplay(XtParent(w)), obj->wchar_sink.normgc);
	obj->wchar_sink.normgc = NULL;
    }
    if (obj->wchar_sink.invgc) {
	XFreeGC(XtDisplay(XtParent(w)), obj->wchar_sink.invgc);
	obj->wchar_sink.invgc = NULL;
    }
    if (obj->wchar_sink.xorgc) {
	XFreeGC(XtDisplay(XtParent(w)), obj->wchar_sink.xorgc);
	obj->wchar_sink.xorgc = NULL;
    }
    if (obj->wchar_sink.cursor) {
	XFreePixmap(XtDisplay(XtParent(w)), obj->wchar_sink.cursor);
	obj->wchar_sink.cursor = 0;
    }
}

Boolean
WcharSinkSetValuesHook(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal *num_args;
{
    WcharSinkObject sink = (WcharSinkObject)w;
    int i, num = *num_args;
    Boolean destroy = False;
    
    for (i = 0; i < num; i++) {
	if (strcmp(args[i].name, XtNbackground) == 0 &&
	    sink->wchar_sink.background != (Pixel)args[i].value) {
	    sink->wchar_sink.background = (Pixel)args[i].value;
	    destroy = True;
	} else if (strcmp(args[i].name, XtNforeground) == 0 &&
		   sink->wchar_sink.foreground != (Pixel)args[i].value) {
	    sink->wchar_sink.foreground = (Pixel)args[i].value;
	    destroy = True;
	}
    }
    if (destroy && sink->wchar_sink.normgc)
	WcharSinkDestroy(w, NULL, NULL);
    return True;
}


int
CheckPrivateData(w)
    WcharSinkObject w;
{
    if (w->wchar_sink.normgc==NULL) {
	int h0, h1;
        XGCValues gcval;
	Widget parent = XtParent((Widget)w);
        Display *dpy;
        unsigned long t;

	if (!XtWindow(parent))
	    return 1;
        dpy = XtDisplay(parent); 

        gcval.foreground = w->wchar_sink.foreground;
        gcval.background = w->wchar_sink.background;
	w->wchar_sink.normgc = XCreateGC(dpy, XtWindow(parent),
					 GCForeground|GCBackground, &gcval); 
        assert(w->wchar_sink.normgc!=NULL);
        t = gcval.foreground;
        gcval.foreground = gcval.background;
        gcval.background = t;
        w->wchar_sink.invgc = XCreateGC(dpy, XtWindow(parent), 
                                        GCForeground|GCBackground, &gcval); 
        assert(w->wchar_sink.invgc!=NULL);
	gcval.function = GXxor;
	gcval.graphics_exposures=False;
        w->wchar_sink.xorgc = XCreateGC(dpy, XtWindow(parent), 
                                        GCFunction|GCGraphicsExposures,
					&gcval); 
        assert(w->wchar_sink.xorgc!=NULL);

	h0 = w->wchar_sink.font->ascent+w->wchar_sink.font->descent;
	h1 = w->wchar_sink.font8->ascent+w->wchar_sink.font8->descent;
	if (h0 > h1) { 
	    w->wchar_sink.row_height = h0;
	    w->wchar_sink.row_org[0] = w->wchar_sink.font->ascent;
	    w->wchar_sink.row_org[1] = h0-h1+w->wchar_sink.font8->ascent;
	} else {
	    w->wchar_sink.row_height = h1;
	    w->wchar_sink.row_org[1] = w->wchar_sink.font8->ascent;
	    w->wchar_sink.row_org[0] = h1-h0+w->wchar_sink.font->ascent;
	}

	w->wchar_sink.avg_width = (w->wchar_sink.font8->min_bounds.width+
				   w->wchar_sink.font8->max_bounds.width+
				   w->wchar_sink.font->min_bounds.width+
				   w->wchar_sink.font->max_bounds.width)/4;
	w->wchar_sink.cursor = XCreateBitmapFromData(dpy, XtWindow(parent),
						     cursor_bits,
						     cursor_width,
						     cursor_height);
	w->wchar_sink.cursor_bounds.x =
	    w->wchar_sink.cursor_bounds.y = 0;
	w->wchar_sink.cursor_bounds.width = cursor_width;
	w->wchar_sink.cursor_bounds.height= cursor_height;
    }
    if (XawTextGetSource(XtParent((Widget)w)) == 0)
	return 1;
    return 0;
}

/*	Function Name: WcharSinkDisplayText
 *	Description: Display text. 
 *	Arguments: w - the WcharSink object.
 *                 x, y - location to start drawing text.
 *                 pos1, pos2 - location of starting and ending points
 *                              in the text buffer.
 *                 highlight - hightlight this text?
 *	Returns: none.
 */
void
WcharSinkDisplayText(w, x, y, pos1, pos2, highlight)
    Widget w;
    Position x;
    Position y;
    XawTextPosition pos1;
    XawTextPosition pos2;
    Boolean highlight;
{
    XawTextBlock text_blk;
    WcharSinkObject obj = (WcharSinkObject)w;
    GC gc;
    
    if (CheckPrivateData(obj))
	return;
    gc = highlight ?  obj->wchar_sink.invgc : obj->wchar_sink.normgc;
    do {
	GetSource(w, pos1, &text_blk, pos2-pos1);
	if (text_blk.length == 0)
	    return;
	switch (text_blk.format) {
	case FormatNarrow:
	    XSetFont(XtDisplay(XtParent(w)), gc, obj->wchar_sink.font8->fid);
	    XDrawImageString(XtDisplay(XtParent(w)), XtWindow(XtParent(w)), gc,
			     x, y+obj->wchar_sink.row_org[1],
			     text_blk.ptr + text_blk.firstPos,
			     text_blk.length);
	    x += XTextWidth(obj->wchar_sink.font8,
			      text_blk.ptr + text_blk.firstPos,
			      text_blk.length);
	    break;
	case FormatWide:
	    XSetFont(XtDisplay(XtParent(w)), gc, obj->wchar_sink.font->fid);
	    XDrawImageString16(XtDisplay(XtParent(w)), XtWindow(XtParent(w)),
			       gc,
			       x, y+obj->wchar_sink.row_org[0],
			       (XChar2b*)text_blk.ptr + text_blk.firstPos,
			       text_blk.length/2);
	    x += XTextWidth16(obj->wchar_sink.font,
			      (XChar2b*)text_blk.ptr + text_blk.firstPos,
			      text_blk.length/2);
	    break;
	}
	pos1 += text_blk.length;
    } while (pos1 < pos2);
}

/*	Function Name: WcharSinkInsertCursor
 *	Description: Places the InsertCursor.
 *	Arguments: w - the WcharSink object.
 *                 x, y - location for the cursor.
 *                 state - whether to turn the cursor on, or off.
 *	Returns: none.
 */
void
WcharSinkInsertCursor(w, x, y, state)
    Widget w;
    Position x;
    Position y;
    XawTextInsertState state;
{
    WcharSinkObject obj = (WcharSinkObject)w;
    
    if (CheckPrivateData(obj))
	return;

    XCopyPlane(XtDisplay(XtParent(w)),
	       obj->wchar_sink.cursor,
	       XtWindow(XtParent(w)),
	       obj->wchar_sink.xorgc,
	       0,
	       0,
	       obj->wchar_sink.cursor_bounds.width,
	       obj->wchar_sink.cursor_bounds.height,
	       x, y-cursor_height,
	       1);
    obj->wchar_sink.laststate = state;
    obj->wchar_sink.cursor_bounds.x = x;
    obj->wchar_sink.cursor_bounds.y = y-cursor_height;
}

/*	Function Name: WcharSinkClearToBackground
 *	Description: Clears a region of the sink to the background color.
 *	Arguments: w - the WcharSink object.
 *                 x, y  - location of area to clear.
 *                 width, height - size of area to clear
 *	Returns: void.
 */
void
WcharSinkClearToBackground(w, x, y, width, height)
    Widget w;
    Position x;
    Position y;
    Dimension width;
    Dimension height;
{
    XClearArea(XtDisplay(XtParent(w)), XtWindow(XtParent(w)),
	       x, y, width, height, False);
}

/*      Function name: MeasureText
 *	Description: Determines a width of the text between positions
 *                   fromPos(inclusive) and toPos(exclusive.
 *      Arguments:   w - the WcharSinkObject
 *                   fromPos - starting position
 *                   toPos - ending position
 *      RETURN       resPos - actual ending position
 *      Returns:     the width of the text.
 */
int
MeasureText(w, fromPos, toPos, resPos)
    WcharSinkObject w;
    XawTextPosition fromPos;
    XawTextPosition toPos;
    XawTextPosition *resPos;
{
    XawTextBlock text_blk;
    int width=0;
    
    *resPos = fromPos;
    if (fromPos==toPos)
        return 0;
    
    do {
	GetSource((Widget)w, fromPos, &text_blk, toPos-fromPos);
	if (text_blk.length == 0) {
	    fromPos++;
	    break;
	}
	
	switch (text_blk.format) {
	case FormatNarrow:
	    width += XTextWidth(w->wchar_sink.font8,
				text_blk.ptr + text_blk.firstPos,
				text_blk.length);
	    break;
	case FormatWide:
	    width += XTextWidth16(w->wchar_sink.font,
				  (XChar2b*)text_blk.ptr + text_blk.firstPos,
				  text_blk.length/2);
	    break;
	}
	fromPos += text_blk.length;
    } while (fromPos < toPos);
    *resPos = fromPos;
    return width;
}

/*	Function Name: WcharSinkFindPosition
 *	Description: Finds a position in the text.
 *	Arguments: w - the WcharSink Object.
 *                 fromPos - reference position.
 *                 fromX   - reference location.
 *                 width   - width of section to paint text.
 *                 stopAtWordBreak - returned position is a word break?
 *      RETURNED   resPos - Position to return.      
 *      RETURNED   resWidth - Width actually used.   
 *      RETURNED   resHeight - Height actually used. 
 *	Returns: none (see above).
 */
void
WcharSinkFindPosition(w, fromPos, fromX, width, stopAtWordBreak,
		      pos_return, width_return, height_return)
    Widget w;
    XawTextPosition fromPos;
    int fromX;
    int	width;
    Boolean stopAtWordBreak;
    /* return: */
    XawTextPosition *pos_return;
    int *width_return;
    int *height_return;
{
    WcharSinkObject obj = (WcharSinkObject)w;
    XawTextPosition toPos, newPos;
    int delta, nwidth;
    
    if (CheckPrivateData(obj))
	return;

    *height_return = obj->wchar_sink.row_height;

    if (width > XtParent(w)->core.width) {
	*pos_return = fromPos;
	*width_return = XtParent(w)->core.width - fromX;
	return;
    }

    if (width > 0) {
	toPos = fromPos;
	delta = width;
	do {
	    toPos += (delta+obj->wchar_sink.avg_width-1)/
		          obj->wchar_sink.avg_width;
	    nwidth = MeasureText(obj, fromPos, toPos, &newPos);
	    delta = width - nwidth;
	} while (delta > 0 && newPos >= toPos);
	if (delta<0) 
	    --toPos;

	*width_return = MeasureText(obj, fromPos, toPos, pos_return);
    } else {
	*pos_return = fromPos;
	*width_return = 0;
    }
}

/*	Function Name: WcharSinkFindDistance
 *	Description: Find the Pixel Distance between two text Positions.
 *	Arguments: w - the WcharSink object.
 *                 fromPos - starting Position.
 *                 fromX   - x location of starting Position.
 *                 toPos   - end Position.
 *      RETURNED   Width - Distance between fromPos and toPos.
 *      RETURNED   Pos   - Actual toPos used.
 *      RETURNED   Height - Height required by this text.
 *	Returns: none.
 */
void
WcharSinkFindDistance(w, fromPos, fromX, toPos, width, pos, height)
    Widget w;
    XawTextPosition fromPos;
    int fromX;
    XawTextPosition toPos;
    /* Return: */
    int *width;
    XawTextPosition *pos;
    int *height;
{
    WcharSinkObject obj = (WcharSinkObject)w;

    if (CheckPrivateData(obj))
	return;
    if (fromPos<0) {
	*width = *height = 0;
	*pos = toPos;
	return;
    }
    *width = MeasureText(obj, fromPos, toPos, pos);
    *height = obj->wchar_sink.row_height;
}

/*	Function Name: WcharSinkResolve
 *	Description: Resloves a location to a position.
 *	Arguments: w - the WcharSink object.
 *                 pos - a reference Position.
 *                 fromx - a reference Location.
 *                 width - width to move.
 *                 resPos - the resulting position.
 *	Returns: none
 */
void
WcharSinkResolve(w, fromPos, fromX, width, pos)
    Widget w;
    XawTextPosition fromPos;
    int	fromX;
    int	width;
    /* return: */
    XawTextPosition *pos;
{
    int height;
    WcharSinkFindPosition(w, fromPos, fromX, width, False,
			  pos, &width, &height);
}

/*	Function Name: WcharSinkMaxLines
 *	Description: Finds the Maximum number of lines that will fit in
 *                   a given height.
 *	Arguments: w - the WcharSink Object.
 *                 height - height to fit lines into.
 *	Returns: the number of lines that will fit.
 */
int
WcharSinkMaxLines(w, height)
    Widget w;
    Dimension height;
{
    WcharSinkObject obj = (WcharSinkObject)w;
    if (CheckPrivateData(obj))
	return 0;
    return height / obj->wchar_sink.row_height;
}

/*	Function Name: WcharSinkMaxHeight
 *	Description: Finds the Minimum height that will contain a given number
 *                   of lines.
 *	Arguments: w - the WcharSink object.
 *                 lines - the number of lines.
 *	Returns: the height.
 */
int
WcharSinkMaxHeight(w, lines)
    Widget w;
    int lines;
{
    WcharSinkObject obj = (WcharSinkObject)w;
    if (CheckPrivateData(obj))
	return 0;
    return lines * obj->wchar_sink.row_height;
}


/*	Function Name: WcharSinkSetTabs
 *	Description: Sets the Tab stops.
 *	Arguments: w - the WcharSink object.
 *                 tab_count - the number of tabs in the list.
 *      RETURN     tabs - the text positions of the tabs.
 *	Returns: none
 */
/*ARGSUSED*/
void
WcharSinkSetTabs(w, tab_count, tabs)
    Widget w;
    int tab_count;
    int *tabs;
{
}

/*	Function Name: WcharSinkGetCursorBounds
 *	Description: Finds the bounding box for the insert curor (caret).
 *	Arguments: w - the TextSinkObject.
 *      RETURNED   rect - an X rectance containing the cursor bounds.
 *	Returns: none.
 */
void
WcharSinkGetCursorBounds(w, rect)
    Widget w;
    XRectangle *rect;
{
    WcharSinkObject obj = (WcharSinkObject)w;
    *rect = obj->wchar_sink.cursor_bounds;
}




