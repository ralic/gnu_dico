/* $Id: WcharSrcP.h,v 1.1 2001/11/03 22:34:29 gray Exp $ */
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

#ifndef _XawWcharSrcP_h
#define _XawWcharSrcP_h

#include <stddef.h>
#include <X11/Xaw/TextSrcP.h>
#include "WcharSrc.h"
#include "WcharCommonP.h"

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#ifdef L_tmpnam
#define TMPSIZ L_tmpnam
#else
#define TMPSIZ 32		/* bytes to allocate for tmpnam */
#endif

typedef short Count;
typedef struct {
    char fmt;
    Count length;
} Segm;

typedef struct _WcharChunk {   	/* Chunk of the text file of BUFSIZ bytes */
    struct _WcharChunk *prev, *next;	/* linked list pointers. */
    Boolean dirty;                /* dirty flag */
    Count *line_length;           /* Array of line lengths in this block */
    Segm *segm;                   /* lengths of the segments in this block */
    char * text;      		  /* The text in this buffer. */
    XawTextPosition used;	  /* The number of characters of this buffer 
				     that have been used. */
    int nlines;                   /* Number of lines in the block */
    int nsegm;                    /* Number of segments in the block */
} WcharChunk;

#define SegmentFormat(cp, n) (cp)->segm[n].fmt


/************************************************************
 *
 * New fields for the WcharSrc object class record.
 *
 ************************************************************/

typedef struct _WcharSrcClassPart { char foo; } WcharSrcClassPart;

/* Full class record declaration */
typedef struct _WcharSrcClassRec {
    ObjectClassPart     object_class;
    TextSrcClassPart	text_src_class;
    WcharSrcClassPart	wchar_src_class;
} WcharSrcClassRec;

extern WcharSrcClassRec wcharSrcClassRec;

/* New fields for the WcharSrc object record */

typedef struct _WcharSrcPart {

  /* Resources. */

    char       *string;		    /* either the string, or the
				       file name, depending upon the type. */
    XawWcharType type;		    /* either string or disk. */
    Encoding encoding;
    XawWcharReplaceMode replace_mode;
    XawTextPosition chunk_size;	    /* Size of text buffer for each chunk. */
    int max_lines;                  /* Max number of text lines per block */
    int max_segm;                   /* Max number of text segments per block */
    Boolean data_compression;	    /* compress to minimum memory automatically
				       on save? */
    XtCallbackList callback;	    /* A callback list to call when the
				       source is changed. */
    Boolean use_string_in_place;    /* Use the string passed in place. */
    int     length;		    /* string length */
    
/* Private data. */

    Boolean	is_tempfile;	    /* Is this a temporary file? */
    Boolean       changed;	    /* Has this file been edited? */
    Boolean       allocated_string; /* string field is malloc'ed */
    XawTextPosition filelength;     /* length of file */
    WcharChunk * first_chunk;       /* first piece of the text. */
    XChar2b *buf;
    int bufsize;
} WcharSrcPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _WcharSrcRec {
    ObjectPart    object;
    TextSrcPart	text_src;
    WcharSrcPart  wchar_src;
} WcharSrcRec;

_XFUNCPROTOBEGIN

extern Encoding DetectEncoding(
#if NeedFunctionPrototypes
    char * /* text */,
    int    /* length */
#endif
);

_XFUNCPROTOEND

#endif /* _XawWcharSrcP_h  --- Don't add anything after this line. */

