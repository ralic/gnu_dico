/* $Id: WcharSrc.h,v 1.1 2001/11/03 22:34:29 gray Exp $ */
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

#ifndef _XawWcharSrc_h
#define _XawWcharSrc_h

#include <X11/Xaw/TextSrc.h>
#include <X11/Xfuncproto.h>
#include "WcharCommon.h"

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 callback	     Callback		Callback	(none)
 dataCompression     DataCompression	Boolean		True
 length		     Length		int		(internal)
 chunkSize	     ChunkSize		int		BUFSIZ
 maxLines            MaxLines           int             MAXLINES
 maxSegments         MaxSegments        int             MAXSEGMENTS
 string		     String		String		NULL
 type		     Type		XawWcharType	XawWcharString
 useStringInPlace    UseStringInPlace	Boolean		False
 encoding            Encoding           Encoding        Detect
 replaceMode         ReplaceMode        XawWcharReplaceMode XawWcharReplaceNone
*/

#ifndef MAXLINES
#define MAXLINES 128
#endif
#ifndef MAXSEGMENTS
#define MAXSEGMENTS 512
#endif

/* Class record constants */

extern WidgetClass wcharSrcObjectClass;

typedef struct _WcharSrcClassRec *WcharSrcObjectClass;
typedef struct _WcharSrcRec      *WcharSrcObject;

/*
 * Just to make people's lives a bit easier.
 */

#define WcharSourceObjectClass WcharSrcObjectClass
#define WcharSourceObject      WcharSrcObject

/*
 * Resource Definitions.
 */

#define XtCDataCompression "DataCompression"
#define XtCChunkSize "ChunkSize"
#define XtCType "Type"
#define XtCUseStringInPlace "UseStringInPlace"
#define XtCMaxLines "MaxLines"
#define XtCMaxSegments "MaxSegments"
#define XtCEncoding "Encoding"
#define XtCReplaceMode "ReplaceMode"

#define XtNdataCompression "dataCompression"
#define XtNchunkSize "chunkSize"
#define XtNtype "type"
#define XtNuseStringInPlace "useStringInPlace"
#define XtNmaxLines "maxLines"
#define XtNmaxSegments "maxSegments"
#define XtNencoding "encoding"
#define XtNreplaceMode "replaceMode"

#define XtRWcharType "WcharType"
#define XtREncoding "Encoding"
#define XtRReplaceMode "ReplaceMode"

#define XtEstring "string"
#define XtEfile "file"
#define XtEdetect "detect"
#define XtEshiftJis "shift-jis"
#define XtEshinJis "shin-jis"
#define XtEeuc "euc"
#define XtEreplaceNone "none"
#define XtEreplaceBlock "block"
#define XtEreplaceAll "all"

typedef enum {XawWcharFile, XawWcharString} XawWcharType;

typedef enum {
    XawWcharReplaceNone,
    XawWcharReplaceBlock,
    XawWcharReplaceAll,
    NREPLACEMODES,
} XawWcharReplaceMode;

/************************************************************
 *
 * Public routines 
 *
 ************************************************************/

_XFUNCPROTOBEGIN

/*	Function Name: XawWcharSourceFreeString
 *	Description: Frees the string returned by a get values call
 *                   on the string when the source is of type string.
 *	Arguments: w - the WcharSrc object.
 *	Returns: none.
 */

extern void XawWcharSourceFreeString(
#if NeedFunctionPrototypes
    Widget		/* w */
#endif
);

/*	Function Name: XawWcharSave
 *	Description: Saves all the pieces into a file or string as required.
 *	Arguments: w - the WcharSrc Object.
 *	Returns: TRUE if the save was successful.
 */

extern Boolean XawWcharSave(
#if NeedFunctionPrototypes
    Widget		/* w */
#endif
);

/*	Function Name: XawWcharSaveAsFile
 *	Description: Save the current buffer as a file.
 *	Arguments: w - the WcharSrc object.
 *                 name - name of the file to save this file into.
 *	Returns: True if the save was successful.
 */

extern Boolean XawWcharSaveAsFile(
#if NeedFunctionPrototypes
    Widget		/* w */,
    _Xconst char*	/* name */
#endif 
);

/*	Function Name: XawWcharSourceChanged
 *	Description: Returns true if the source has changed since last saved.
 *	Arguments: w - the WcharSource object.
 *	Returns: a Boolean (see description).
 */

extern Boolean XawWcharSourceChanged(
#if NeedFunctionPrototypes
    Widget		/* w */
#endif
);

_XFUNCPROTOEND

#endif /* _XawWcharSrc_h  - Don't add anything after this line. */

