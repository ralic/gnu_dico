/* $Id: WcharText.c,v 1.1 2001/11/03 22:34:24 gray Exp $ */
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
#include <X11/Xaw/TextP.h>
#include <X11/CoreP.h>
#include <stdio.h>
#include <string.h>
#include <WcharTextP.h>
#include <WcharSrc.h>
#include <WcharSink.h>

static void WcharTextAction(
#if NeedFunctionPrototypes
     Widget,
     XEvent*,
     String*, Cardinal*
#endif
);
static void WcharTextInitialize(
#if NeedFunctionPrototypes
    Widget         /* request */,
    Widget         /* new */,
    ArgList        /* args */,
    Cardinal *     /* num_args */
#endif
);
static void WcharTextClassInitialize(/* void */);
static void WcharTextClassPartInitialize(
#if NeedFunctionPrototypes
    WidgetClass class
#endif					 
);
static void WcharTextDestroy(
#if NeedFunctionPrototypes
    WcharTextWidget /* w */,
    XtPointer       /* closure */,
    XtPointer       /* call_data */
#endif
);
static Boolean WcharTextSetValues(
#if NeedFunctionPrototypes
    Widget          /* old */,
    Widget          /* request */,
    Widget          /* new */,
    ArgList         /* args */,
    Cardinal *      /* num_args */
#endif
);
static Boolean WcharTextSetValuesHook(
#if NeedFunctionPrototypes
    Widget          /* w */,
    ArgList         /* args */,
    Cardinal *      /* num_args */
#endif
);

static XtResource resources[] = {
#define off(field) XtOffset(WcharTextWidget, wcharText.field)
    {XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct*),
	 off(font), XtRString, (XtPointer) XtDefaultFont},
    {XtNkanjiFont, XtCKanjiFont, XtRFontStruct, sizeof(XFontStruct*),
	 off(kanji_font), XtRString, (XtPointer) "kanji16"},
    {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	 off(background), XtRString, XtDefaultBackground},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	 off(foreground), XtRString, XtDefaultForeground},
#undef off
};

static XtActionsRec actions[] = {
  /* {name, procedure}, */
    {"wcharText",	WcharTextAction},
};

#if 0
static char translations[] =
"<Key>:		wcharText()	\n\
";
#endif

WcharTextClassRec wcharTextClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &textClassRec,
    /* class_name		*/	"WcharText",
    /* widget_size		*/	sizeof(WcharTextRec),
    /* class_initialize		*/	WcharTextClassInitialize,
    /* class_part_initialize	*/	WcharTextClassPartInitialize,
    /* class_inited		*/	FALSE,
    /* initialize		*/	WcharTextInitialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class	        */	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/      XtExposeNoExpose|
	                                XtExposeGraphicsExpose|
					XtExposeNoCompress,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	XtInheritResize,
    /* expose			*/	XtInheritExpose,
    /* set_values		*/	NULL /*WcharTextSetValues*/,
    /* set_values_hook		*/	WcharTextSetValuesHook,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	XtInheritAcceptFocus,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* SimpleClass fields */
    /* change_sensitive */              XtInheritChangeSensitive,
  },
  { /* TextClass fields */
    /* empty */                         0,
  },
  { /* wcharText fields */
    /* empty			*/	0
  }
};

WidgetClass wcharTextWidgetClass = (WidgetClass)&wcharTextClassRec;

void
WcharTextAction(widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
}

void
WcharTextClassInitialize()
{
    wcharTextClassRec.core_class.compress_exposure =
	wcharTextClassRec.core_class.superclass->core_class.compress_exposure;
}

void
WcharTextClassPartInitialize(class)
    WidgetClass class;
{
}

/*ARGSUSED*/
void
WcharTextDestroy(w, closure, call_data)
    WcharTextWidget w;
    XtPointer closure, call_data;
{
    if (w->text.sink)
	XtDestroyWidget(w->text.sink);
    if (w->text.source)
	XtDestroyWidget(w->text.source);
}

void
WcharTextInitialize(request, new, args, num_args)
    Widget request;
    Widget new;
    ArgList args;
    Cardinal *num_args;
{
    WcharTextWidget w = (WcharTextWidget)new;
    Arg ar[10];
    int n = 0;

    w->text.source = XtCreateWidget("source", wcharSrcObjectClass,
				    new, NULL, 0);
    
    XtSetArg(ar[n], XtNfont, w->wcharText.font); n++;
    XtSetArg(ar[n], XtNkanjiFont, w->wcharText.kanji_font); n++;
    XtSetArg(ar[n], XtNbackground, w->wcharText.background); n++;
    XtSetArg(ar[n], XtNforeground, w->wcharText.foreground); n++;
    w->text.sink = XtCreateWidget("sink", wcharSinkObjectClass,
			          new, ar, n);
    XtAddCallback(new, XtNdestroyCallback,
		  (XtCallbackProc)WcharTextDestroy, NULL);
}


int
WcharTextSetSource(w, name, args, num_args, pos)
    Widget w;
    String name;
    ArgList args;
    Cardinal num_args;
    XawTextPosition pos;
{
    Widget s = XtCreateWidget(name, wcharSrcObjectClass, w, args, num_args);
    if (s) {
	Widget old = XawTextGetSource(w);
	if (old)
	    XtDestroyWidget(old);
	XawTextSetSource(w, s, pos);
	return 0;
    }
    return 1;
}

Encoding
WcharTextGetEncoding(w)
    Widget w;
{
    Encoding encoding;
    Arg args[1];
    Widget src = XawTextGetSource(w);
    if (!src)
	return MaxEncoding;
    XtSetArg(args[0], XtNencoding, &encoding);
    XtGetValues(src, args, 1);
    return encoding;
}

/*ARGSUSED*/
Boolean
WcharTextSetValues(old, request, new, args, num_args)
    Widget old;
    Widget request;
    Widget new;
    ArgList args;
    Cardinal *num_args;
{
    return False;
}

Boolean
WcharTextSetValuesHook(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal *num_args;
{
    WcharTextWidget tw = (WcharTextWidget)w;
    int i, num = *num_args;

    for (i = 0; i < num; i++) {
	if (strcmp(args[i].name, XtNstring) == 0) {
	    WcharTextSetSource(w, "source", args, *num_args, 0);
	    break;
	} else if (strcmp(args[i].name, XtNbackground) == 0) {
	    if (tw->text.sink)
		XtVaSetValues(tw->text.sink,
			      args[i].name, args[i].value,
			      NULL);
	}
    }
    return True;
}


void
_wchar_internal_error(file, line, text)
    char *file;
    int line;
    char *text;
{
    fprintf(stderr, "Wchar:INTERNAL ERROR:%s:%d:%s\n", file, line, text);
    abort();
}
