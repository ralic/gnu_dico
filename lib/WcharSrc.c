/* $Id: WcharSrc.c,v 1.1 2001/11/03 22:34:24 gray Exp $ */
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
#define NDEBUG 1
#undef DEBUG
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/ObjectP.h>
#include <stdio.h> /* for the definition of BUFSIZ */ 
#include <limits.h> /* contains the definition of SCHAR_MAX */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <WcharSrcP.h>

#define ESCAPE 27
#define SHIN_KANJI_MODE 'K'
#define SHIN_ROMAJI_MODE 'H'

#define ACCESSRIGHTS 0644
typedef unsigned char uchar;
static void WcharSrcClassInitialize();
static void WcharSrcInitialize(
#if NeedFunctionPrototypes
     Widget,
     Widget,
     ArgList,
     Cardinal*
#endif
);
static Boolean StringToWcharTypeConverter(
#if NeedFunctionPrototypes
    Display *          /* dpy */,
    XrmValuePtr        /* args */,
    Cardinal *         /* num_args */,
    XrmValuePtr        /* fromVal */,
    XrmValuePtr        /* toVal */,
    XtPointer *        /* data */
#endif
);
static Boolean StringToEncodingConverter(
#if NeedFunctionPrototypes
    Display *          /* dpy */,
    XrmValuePtr        /* args */,
    Cardinal *         /* num_args */,
    XrmValuePtr        /* fromVal */,
    XrmValuePtr        /* toVal */,
    XtPointer *        /* data */
#endif
);
static Boolean StringToReplaceModeConverter(
#if NeedFunctionPrototypes
    Display *          /* dpy */,
    XrmValuePtr        /* args */,
    Cardinal *         /* num_args */,
    XrmValuePtr        /* fromVal */,
    XrmValuePtr        /* toVal */,
    XtPointer *        /* data */
#endif
);
static XawTextPosition	WcharSrcRead(
#if NeedFunctionPrototypes
    Widget             /* w */,
    XawTextPosition    /* pos */,
    XawTextBlock *     /* text */,
    int	               /* length */
#endif
);
static int WcharSrcReplace(
#if NeedFunctionPrototypes
    Widget             /* w */,
    XawTextPosition    /* start */,
    XawTextPosition    /* end */,
    XawTextBlock *     /* text */
#endif
);
static XawTextPosition	WcharSrcScan(
#if NeedFunctionPrototypes
    Widget             /* w */,
    XawTextPosition    /* pos */,
    XawTextScanType    /* type */,
    XawTextScanDirection /* dir */,
    int                /* count */,
    Boolean            /* include */
#endif
);
static XawTextPosition WcharSrcSearch(
#if NeedFunctionPrototypes
    Widget             /* w */,
    XawTextPosition    /* pos */,
    XawTextScanDirection /* dir */,
    XawTextBlock *     /* text */
#endif
);
static void WcharSrcSetSelection(
#if NeedFunctionPrototypes
    Widget             /* w */,
    XawTextPosition    /* start */,
    XawTextPosition    /* end */,
    Atom               /* selection */
#endif
);
static Boolean WcharSrcConvertSelection(
#if NeedFunctionPrototypes
    Widget             /* w */,
    Atom *             /* selection */,
    Atom *             /* target */,
    Atom *             /* type */,
    XtPointer *        /* value */,
    unsigned long *    /* length */,
    int *              /* format */
#endif
);
static Boolean WcharSrcSetValuesHook(
#if NeedFunctionPrototypes
    Widget             /* w */,
    ArgList            /* args */,
    Cardinal *         /* num_args */
#endif
);
static void WcharSrcDestroy(
#if NeedFunctionPrototypes
    Widget             /* w */,
    XtPointer          /* closure */,
    XtPointer          /* call_data */
#endif
);
static int LoadFile(
#if NeedFunctionPrototypes
    WcharSrcObject     /* w */
#endif
);
static int LoadString(
#if NeedFunctionPrototypes
    WcharSrcObject     /* w */
#endif
);
static void FreeChunkList(
#if NeedFunctionPrototypes
    WcharChunk *       /* ptr */
#endif
);

static XtResource resources[] = {
#define off(field) XtOffset(WcharSrcObject, wchar_src.field)
    {XtNcallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	 off(callback), XtRImmediate, NULL},
    {XtNdataCompression, XtCDataCompression, XtRBoolean, sizeof(Boolean),
	 off(data_compression), XtRImmediate, (XtPointer) True},
    {XtNlength, XtCLength, XtRInt, sizeof(int),
	 off(length), XtRImmediate, 0 /*internal*/},
    {XtNchunkSize, XtCChunkSize, XtRInt, sizeof(int),
	 off(chunk_size), XtRImmediate, (XtPointer)BUFSIZ},
    {XtNmaxLines, XtCMaxLines, XtRInt, sizeof(int),
	 off(max_lines), XtRImmediate, (XtPointer)MAXLINES},
    {XtNmaxSegments, XtCMaxSegments, XtRInt, sizeof(int),
	 off(max_segm), XtRImmediate, (XtPointer)MAXSEGMENTS},
    {XtNstring, XtCString, XtRString, sizeof(String),
	 off(string), XtRImmediate, NULL},
    {XtNtype, XtCType, XtRWcharType, sizeof(XawWcharType),
	 off(type), XtRString, XtEstring},
    {XtNencoding, XtCEncoding, XtREncoding, sizeof(Encoding),
	 off(encoding), XtRString, XtEdetect},
    {XtNuseStringInPlace, XtCUseStringInPlace, XtRBoolean, sizeof(Boolean),
	 off(use_string_in_place), XtRImmediate, (XtPointer)False},
    {XtNreplaceMode, XtCReplaceMode, XtRReplaceMode, sizeof(XawWcharReplaceMode),
	 off(replace_mode), XtRString, XtEreplaceNone},
#undef off
};


WcharSrcClassRec wcharSrcClassRec = {
    { /* Object class */
      /* superclass             */      (WidgetClass) &textSrcClassRec,
      /* class_name             */      "WcharSrc",
      /* widget_size            */      sizeof(WcharSrcRec), 
      /* class_initialize       */      WcharSrcClassInitialize,
      /* class_part_initialize  */      NULL,
      /* class_inited           */      FALSE,
      /* initialize             */      WcharSrcInitialize,
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
      /* set_values_hook        */      WcharSrcSetValuesHook,
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
    { /* TextSrc Class */
      /* Read                   */     (XawTextPosition(*)()) WcharSrcRead,
      /* Replace                */     (int(*)()) WcharSrcReplace,
      /* Scan                   */     (XawTextPosition (*)()) WcharSrcScan,
      /* Search                 */     (XawTextPosition (*)()) WcharSrcSearch,
      /* SetSelection           */     (void (*)()) WcharSrcSetSelection,
      /* ConvertSelection       */     (Boolean (*)())WcharSrcConvertSelection,
    },
    { /* WcharSrcClass */
      /* foo                    */      0,
    }
};

WidgetClass wcharSrcObjectClass = (WidgetClass)&wcharSrcClassRec;
static XrmQuark typequark[2];
static XrmQuark encoding_quark[MaxEncoding];
static XrmQuark replace_mode_quark[NREPLACEMODES];

static char *encoding_str[] = {
    XtEdetect, XtEshinJis, XtEshiftJis, XtEeuc
};
static char *replace_mode_str[] = {
    XtEreplaceNone,
    XtEreplaceBlock,
    XtEreplaceAll
};

void
WcharSrcClassInitialize()
{
    int i;
    /* Install converters and prepare their data
     */
    XtSetTypeConverter(XtRString, XtRWcharType, StringToWcharTypeConverter,
		       NULL, 0, XtCacheByDisplay, NULL);
    XtSetTypeConverter(XtRString, XtREncoding, StringToEncodingConverter,
		       NULL, 0, XtCacheByDisplay, NULL);
    XtSetTypeConverter(XtRString, XtRReplaceMode, StringToReplaceModeConverter,
		       NULL, 0, XtCacheByDisplay, NULL);

    typequark[XawWcharFile] = XrmStringToQuark(XtEfile);
    typequark[XawWcharString] = XrmStringToQuark(XtEstring);
    
    for (i = Detect; i < MaxEncoding; i++) 
	encoding_quark[i] = XrmStringToQuark(encoding_str[i]);

    for (i = 0; i < XtNumber(replace_mode_str); i++) 
	replace_mode_quark[i] = XrmStringToQuark(replace_mode_str[i]);
}

void
_WcharSrcReset(obj)
    WcharSrcObject obj;
{
    if (!obj->wchar_src.use_string_in_place && obj->wchar_src.string) {
	obj->wchar_src.string = XtNewString(obj->wchar_src.string);
	obj->wchar_src.allocated_string = True;
    } else
	obj->wchar_src.allocated_string = False;

    obj->wchar_src.bufsize = 32;
    obj->wchar_src.buf = (XChar2b*) XtMalloc(obj->wchar_src.bufsize*
				             sizeof(*obj->wchar_src.buf));

    switch (obj->wchar_src.type) {
    case XawWcharString:
	LoadString(obj);
	break;
    case XawWcharFile:
	LoadFile(obj);
    }
    if (obj->wchar_src.first_chunk &&
	obj->wchar_src.replace_mode == XawWcharReplaceAll)
	XawTextSetSelection(XtParent((Widget)obj),
			    0, obj->wchar_src.filelength);
}


void
WcharSrcInitialize(request, new, args, num_args)
    Widget request;
    Widget new;
    ArgList args;
    Cardinal *num_args;
{
    WcharSrcObject obj = (WcharSrcObject)new;

    obj->wchar_src.first_chunk = NULL;
    XtAddCallback(new, XtNdestroyCallback, WcharSrcDestroy, NULL);

    _WcharSrcReset(obj);
}

/*ARGSUSED*/
void
WcharSrcDestroy(w, closure, call_data)
    Widget w;
    XtPointer closure, call_data;
{
    WcharSrcObject obj = (WcharSrcObject)w;
    if (obj->wchar_src.changed) {
	/* Should save the file here */
    }
    FreeChunkList(obj->wchar_src.first_chunk);
    if (obj->wchar_src.allocated_string)
	XtFree(obj->wchar_src.string);
    XtFree((char*)obj->wchar_src.buf);
    
    obj->wchar_src.first_chunk = NULL;
    obj->wchar_src.string = NULL;
    obj->wchar_src.buf = NULL;
}

Boolean
WcharSrcSetValuesHook(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal *num_args;
{
    return True;
}

int
LengthOf(w)
    WcharSrcObject w;
{
    if (w->wchar_src.type == XawWcharString && w->wchar_src.length)
	return (w->wchar_src.filelength < w->wchar_src.length) ?
	    w->wchar_src.filelength : w->wchar_src.length;
    else
	return w->wchar_src.filelength;
}

#define DONE(var, type) \
 if (toVal->addr) { \
     if (toVal->size < sizeof(type)) { \
         toVal->size = sizeof(type); \
         return False; \
     } else \
         *((type *) toVal->addr) = var; \
 } else \
     toVal->addr = (caddr_t) &var; \
 toVal->size = sizeof(type); \
 return True;

#define DONESTR(str) \
 if (toVal->addr && toVal->size < sizeof(String)) { \
     toVal->size = sizeof(String); \
     return False; \
 } else \
     toVal->addr = (caddr_t) str; \
 toVal->size = sizeof(String); \
 return True;

Boolean
StringToWcharTypeConverter(dpy, args, num_args, fromVal, toVal, data)
    Display *dpy;
    XrmValuePtr args;
    Cardinal *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer *data;
{
    XrmQuark quark = XrmStringToQuark(fromVal->addr);
    static int ret;
    
    if (quark == typequark[XawWcharFile])
	ret = XawWcharFile;
    else if (quark == typequark[XawWcharString])
	ret = XawWcharString;
    else
	fprintf(stderr, "cannot convert `%s' to XawWcharType\n", fromVal->addr);
    DONE(ret, int);
}

Boolean
StringToEncodingConverter(dpy, args, num_args, fromVal, toVal, data)
    Display *dpy;
    XrmValuePtr args;
    Cardinal *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer *data;
{
    XrmQuark quark = XrmStringToQuark(fromVal->addr);
    static Encoding ret;

    for (ret = Detect; ret < MaxEncoding; ret++) {
	if (quark == encoding_quark[ret])
	    break;
    }
    if (ret == MaxEncoding)
	fprintf(stderr, "cannot convert `%s' to Encoding\n", fromVal->addr);
    DONE(ret, int);
}

Boolean
StringToReplaceModeConverter(dpy, args, num_args, fromVal, toVal, data)
    Display *dpy;
    XrmValuePtr args;
    Cardinal *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
    XtPointer *data;
{
    XrmQuark quark = XrmStringToQuark(fromVal->addr);
    static int ret;
    
    for (ret = 0; ret < XtNumber(replace_mode_quark); ret++) {
	if (quark == replace_mode_quark[ret])
	    break;
    }
    if (ret == NREPLACEMODES)
	fprintf(stderr, "cannot convert `%s' to ReplaceMode\n", fromVal->addr);

    DONE(ret, int);
}

#define MAX_CHAR SCHAR_MAX

int 
ScanChunk_shift(w, cp)
    WcharSrcObject w;
    WcharChunk *cp;
{
    uchar *p;
    int line=0;
    int segm=0;
    int line_len=1;
    int segm_len=1;
    XawTextPosition i;
    int fmt;
#ifdef DEBUG
    int n;
#endif
    
#define NEXT_CHAR i++, p++, line_len++, segm_len++
	    
    fmt = ((uchar)cp->text[0] < MAX_CHAR) ? FormatNarrow : FormatWide;
    for (i = 0, p = cp->text; i < cp->used; NEXT_CHAR) {
	switch (fmt) {
	case FormatWide:
	    if (*p < MAX_CHAR) {
		assert(segm_len%2==0);
		cp->segm[segm].length = segm_len;
		cp->segm[segm++].fmt = fmt;
		if ((n=shift_to_shin(p - segm_len, segm_len)) < segm_len) 
		    fprintf(stderr, "shift to shin cvt failed near %2.2s\n",
			    p+n);
		    
		segm_len = 1;
		if (segm >= w->wchar_src.max_segm)
		    goto endloop;
		fmt = FormatNarrow;
	    } else {
		NEXT_CHAR;
		break;
	    }

	case FormatNarrow:
	    if (*p == '\n') {
		cp->line_length[line++] = line_len;
		line_len = 0; 
		if (line >= w->wchar_src.max_lines) {
		    goto endloop;
		}
		cp->segm[segm].length = segm_len;
		cp->segm[segm++].fmt = fmt;
		if (segm >= w->wchar_src.max_segm)
		    goto endloop;
		segm_len = 0;
	    } else if (*p >= MAX_CHAR) {
		cp->segm[segm].length = segm_len-1;
		cp->segm[segm++].fmt = fmt;
		segm_len = 0;
		if (segm >= w->wchar_src.max_segm)
		    goto endloop;
		fmt = FormatWide;
		NEXT_CHAR;
	    }
	}
    }
    if (p[-1] != '\n') {
	cp->line_length[line++] = line_len;
	cp->segm[segm].fmt = fmt;
	cp->segm[segm++].length = segm_len;
    }
 endloop:
    cp->nlines = line;
    cp->nsegm = segm;
#ifdef DEBUG
    for (n = 0, segm_len = 0; n < segm; segm_len += cp->segm[n++].length);
    assert(segm_len==cp->used);
#endif
    return i;
#undef NEXT_CHAR
}

#define TOK_END 0
#define TOK_ESCAPE_KANJI 1
#define TOK_ESCAPE_ROMAJI 2
#define TOK_ESCAPE_UNKNOWN 3
#define TOK_NL 4
#define TOK_OTHER 5

int action_tab[2][6] = {
/* States:\Tokens: 0 ESCAPE_KANJI ESCAPE_ROMAJI ESCAPE_UNKNOWN   '\n'  Other */
/* FormatNarrow*/  0,           1,           2,             3,    4,      5,
/* FormatWide  */  0,           2,           6,             3,    7,      8,
};

int 
ScanChunk_shin(w, cp)
    WcharSrcObject w;
    WcharChunk *cp;
{
    uchar *p, *q;
    int line=0;
    int segm=0;
    int line_len=0;
    int segm_len=0;
    XawTextPosition i;
    int fmt;
    int action, tok;
#ifdef DEBUG
    int n;
#endif

#define NEXT_CHAR i++, (*q++=*p++), line_len++, segm_len++
    
    p = q = cp->text;
    if (*p++ != ESCAPE)
	return -1;
    switch (*p++) {
    case SHIN_KANJI_MODE:
	fmt = FormatWide;
	break;
    case SHIN_ROMAJI_MODE:
	fmt = FormatNarrow;
	break;
    default:
	return -1;
    }
    cp->used -= 2;

    for (i=0; i < cp->used; ) {
	switch (*p) {
	case ESCAPE:
	    switch (*++p) {
	    case SHIN_KANJI_MODE:
		tok = TOK_ESCAPE_KANJI;
		break;
	    case SHIN_ROMAJI_MODE:
		tok = TOK_ESCAPE_ROMAJI;
		break;
	    default:
		tok = TOK_ESCAPE_UNKNOWN;
		break;
	    }
	    ++p;
	    cp->used -= 2;
	    break;
	case '\n':
	    tok = TOK_NL;
	    break;
	default:
	    tok = TOK_OTHER;
	}
	action = action_tab[fmt][tok];

	switch (action) {
	case 1: /* Switch to kanji segment */
	    if (segm_len > 0) {
                cp->segm[segm].length = segm_len;
	        cp->segm[segm++].fmt = fmt;
		if (segm >= w->wchar_src.max_segm)
		    goto endloop;
            }
            segm_len = 0;
	    fmt = FormatWide;
	    break;
	case 2: /* Superfluous escape */
	    break;
	case 3: /* Unknown escape */
	    break;
	case 4: /* New line (romaji) */
	case 7: /* New line (kanji) */
	    NEXT_CHAR;
	    cp->line_length[line++] = line_len;
	    line_len = 0; 
	    if (line >= w->wchar_src.max_lines) {
		goto endloop;
	    }
	    cp->segm[segm].length = segm_len;
	    cp->segm[segm++].fmt = fmt;
	    if (segm >= w->wchar_src.max_segm)
		goto endloop;
	    segm_len = 0;
	    break;
	case 5: /* Char on Romaji */
	    NEXT_CHAR;
	    break;
	case 6: /* Switch to Romaji segment */
	    assert(segm_len%2==0);
	    if (segm_len > 0) {
                cp->segm[segm].length = segm_len;
	        cp->segm[segm++].fmt = fmt;
       	        if (segm >= w->wchar_src.max_segm)
		    goto endloop;
            }
            segm_len = 0;
	    fmt = FormatNarrow;
	    break;
	case 8: /* Char in Kanji */
	    NEXT_CHAR;NEXT_CHAR;
            break;
	default:
	    WcharInternalError("Unknown action");
	}
    }

    if (p[-1] != '\n') {
	cp->line_length[line++] = line_len;
	cp->segm[segm].fmt = fmt;
	cp->segm[segm++].length = segm_len;
    }
 endloop:
    cp->nlines = line;
    cp->nsegm = segm;
#ifdef DEBUG
    for (n = 0, segm_len = 0; n < segm; segm_len += cp->segm[n++].length);
    assert(segm_len==cp->used);
#endif
    return i;
#undef NEXT_CHAR
}

#undef DEBUG
int 
ScanChunk_euc(w, cp)
    WcharSrcObject w;
    WcharChunk *cp;
{
    uchar *p, *start;
    int used;
    int line=0;
    int segm=0;
    int line_len=0;
    int segm_len=0;
    XawTextPosition i;
    int fmt;
    int n;
    
    p = cp->text;
    i = 0;
    line_len = 0;
    do {
	start = p;
	used = cp->used;
	if (*p >= MAX_CHAR) {
	    fmt = FormatWide;
	    used--;
	    if (used == 0) 
		return -1;
	    segm_len = 0;
	    for ( ; i < used; i += 2, segm_len += 2, line_len += 2)
		if (*p >= MAX_CHAR) 
		    p += 2;
		else 
		    break;
	    
	    if (segm_len % 2) {
		fprintf(stderr, "segm_len %d\n", segm_len);
		segm_len--;
	    }
	
	    euc_to_shin(start, segm_len);
	    cp->segm[segm].length = segm_len;
	    cp->segm[segm++].fmt = fmt;
	} else {
	    fmt = FormatNarrow;
	    segm_len = 0;
	    while (i < used) {
		i++; line_len++; segm_len++;
		if (*p == '\n') {
		    cp->line_length[line++] = line_len;
		    line_len = 0;
		    p++;
		    break;
		} else if (*p < MAX_CHAR) {
		    p++;
		} else {
		    i--; line_len--; segm_len--; 
		    break;
		}
	    }
	    cp->segm[segm].length = segm_len;
	    cp->segm[segm++].fmt = fmt;
	}
    } while (i < cp->used &&
	     segm < w->wchar_src.max_segm &&
	     line < w->wchar_src.max_lines);

    cp->nlines = line;
    cp->nsegm = segm;
#ifdef DEBUG
    for (n = 0, segm_len = 0; n < segm; segm_len += cp->segm[n++].length) {
	printf("%d(%d): %*.*s",
	       n,
	       cp->segm[n].length,
	       cp->segm[n].length,
	       cp->segm[n].length,
	       cp->text + segm_len);
    }
    assert(segm_len==cp->used);
#endif
    return i;
#undef NEXT_CHAR
}


int 
ScanChunk(w, cp)
    WcharSrcObject w;
    WcharChunk *cp;
{
    cp->dirty = False;
    switch (w->wchar_src.encoding) {
    case ShinJis:
	return ScanChunk_shin(w, cp);
    case ShiftJis:
	return ScanChunk_shift(w, cp);
    case EUC:
	return ScanChunk_euc(w, cp);
    default:
	WcharInternalError("Unhandled encoding");
    }
    /*NOTREACHED*/
}


WcharChunk *
AllocChunk(w, string, used)
    WcharSrcObject w;
    char *string;
    int used;
{
    WcharChunk *p;
    
    p = malloc(sizeof(*p)+
	       w->wchar_src.chunk_size+
	       w->wchar_src.max_lines * sizeof(p->line_length[0])+
	       w->wchar_src.max_segm * sizeof(p->segm[0]));
    if (p) {
	memset(p, 0, sizeof(*p));
	p->line_length = (Count*)(p+1);
	p->segm = (Segm*)(p->line_length + w->wchar_src.max_lines);
	p->text = (char*)(p->segm + w->wchar_src.max_segm);
	memcpy(p->text, string, used);
	p->used = used;
	p->dirty = True;
	/*p->nlines = p->nsegm = 0;*/
    }
    return p;
}

void
FreeChunkList(ptr)
    WcharChunk *ptr;
{
    WcharChunk *next;

    while (ptr) {
	next = ptr->next;
	free(ptr);
	ptr = next;
    }
}

int
LoadString(w)
    WcharSrcObject w;
{
    WcharChunk *ptr, *newptr;
    int len, length, rdlen;
    char *text;

    if (!w->wchar_src.string) 
	return 1;

    length = strlen(w->wchar_src.string);
    if (w->wchar_src.length > 0 && w->wchar_src.length < length)
	length = w->wchar_src.length;
    w->wchar_src.filelength = length;
    
    text = w->wchar_src.string;

    if (w->wchar_src.encoding == Detect)
	w->wchar_src.encoding = DetectEncoding(text, len);

    for (ptr = NULL; length > 0; length -= len) {
	rdlen = (length < w->wchar_src.chunk_size) ?
	    length : w->wchar_src.chunk_size;

	if (rdlen == 0) {
	    WcharInternalError("rdlen == 0");
	    break;
	}

	for (len = rdlen-1; len >= 0 && text[len] != '\n'; len--) ;
	if (len > 0)
	    rdlen = len+1;
	
	newptr = AllocChunk(w, text, rdlen);
	if (ptr)
	    ptr->next = newptr;
	else
	    w->wchar_src.first_chunk = newptr;
	if (!newptr) {
	    break;
	}
	newptr->prev = ptr;
	ptr = newptr;
	len = ScanChunk(w, ptr);
	if (len < 0)
	    break;
	text += len;
	ptr->used = len;
    }
    return 0;
}

int
LoadFile(w)
    WcharSrcObject w;
{
    WcharChunk *ptr, *newptr;
    int size, rdlen, off, len;
    int fd;
    char *text, *p;
    
    switch (w->text_src.edit_mode) {
    case XawtextRead:
	fd = open(w->wchar_src.string, O_RDONLY, ACCESSRIGHTS);
	break;
    case XawtextEdit:
	fd = open(w->wchar_src.string, O_RDWR, ACCESSRIGHTS);
	break;
    case XawtextAppend:
	fd = -1;
	break;
    }
    
    if (fd == -1)
	return 1;
    
    size = w->wchar_src.chunk_size;
    ptr = NULL;
    text = malloc(size);
    if (!text) {
	close(fd);
	return 1;
    }

    if (w->wchar_src.encoding == Detect) {
	rdlen = read(fd, text, size);
	w->wchar_src.encoding = DetectEncoding(text, rdlen);
	lseek(fd, 0, SEEK_SET);
    }
    
    off = 0;
    for (;;) {
	rdlen = read(fd, text+off, size-off);
	if (rdlen <= 0)
	    break;
	rdlen += off;
	for (p = text+rdlen-1, off = 0; p >= text && *p != '\n'; p--, off++) ;
	if (p >= text) 
	    off = rdlen - (++p - text);
	else
	    off = 0;

	newptr = AllocChunk(w, text, rdlen - off);
	if (ptr)
	    ptr->next = newptr;
	else
	    w->wchar_src.first_chunk = newptr;
	if (!newptr) {
	    break;
	}
	newptr->prev = ptr;
	ptr = newptr;
	len = ScanChunk(w, ptr);
	if (len < ptr->used) {
	    p = text + len;
	    off = ptr->used - len;
	    ptr->used = len;
	}
	if (off)
	    memcpy(text, p, off);
    }
    w->wchar_src.filelength = lseek(fd, 0, SEEK_CUR);
    close(fd);
    free(text);
    return 0;
}

WcharChunk *
LocateChunk(obj, pos, pos_return, offs_return)
    WcharSrcObject obj;
    XawTextPosition pos;
    XawTextPosition *pos_return;
    XawTextPosition *offs_return;
{
    WcharChunk * p;
    XawTextPosition len;
    
    for (p = obj->wchar_src.first_chunk, len = 0; p;
	 len += p->used, p = p->next) {
	if (len + p->used > pos) {
	    *offs_return = pos - len;
	    *pos_return = len;
	    return p;
	}
    }
    return 0;
}

XawTextPosition
FindForward(type, text, offs, length)
    XawTextScanType type;
    char *text;
    XawTextPosition offs;
    XawTextPosition length;
{
    char *p;
    
    switch (type) {
    case XawstPositions:
    case XawstParagraph:
    case XawstAll:
	break;
    case XawstWhiteSpace:
	for (p = text+offs; offs < length; ) {
	    if (*p < SCHAR_MAX) {
		if (isspace(*p))
		    return offs;
		p++;
		offs++;
	    } else {
		p += 2;
		offs+=2;
	    }
	}
	break;
    case XawstEOL:
	for (p = text+offs; offs < length; ) {
	    if (*p < SCHAR_MAX) {
		if (*p == '\n')
		    return offs;
		p++;
		offs++;
	    } else {
		p += 2;
		offs+=2;
	    }
	}
	break;
    }
    return XawTextScanError;
}

XawTextPosition
FindBackward(type, text, offs, length)
    XawTextScanType type;
    char *text;
    XawTextPosition offs;
    XawTextPosition length;
{
    char *p;

    if (offs == 0 || length == 0)
	return XawTextScanError;

    switch (type) {
    case XawstPositions:
    case XawstParagraph:
    case XawstAll:
	break;
    case XawstWhiteSpace:
	for (p = text+offs-1; length > 0; p--, length--) {
	    if (isspace(*p))
		return length;
	}
	break;
    case XawstEOL:
	if (offs<=1)
	    break;
	for (p = text+offs-1; length > 0; p--, length--) {
	    if (*p == '\n')
		return length;
	}
	break;
    }
    return 0;
}

/*	Function Name: WcharSrcRead
 *	Description: This function reads the source.
 *	Arguments: w - the WcharSrc object.
 *                 pos - position of the text to retreive.
 * RETURNED        text - text block that will contain returned text.
 *                 length - maximum number of characters to read.
 *	Returns: Position immediately after the last byte read.
 * NOTE: <X11/Xaw/Text.h> incorrectly states that XawTextRead returns 'the
 *      number of bytes read'. Instead it returns next text position just as
 *      WcharSrcRead() does.  
 */
XawTextPosition
WcharSrcRead(w, pos, text, length)
    Widget w;
    XawTextPosition pos;
    XawTextBlock *text;
    int	length;
{
    WcharSrcObject obj = (WcharSrcObject)w;
    int size=0;
    XawTextPosition offs, pos1;
    WcharChunk * p;
    int i, cnt;
    XawTextPosition n;
    char *q;
    XChar2b *q2;

    text->length = 0;
    p = LocateChunk(obj, pos, &pos1, &offs);
    if (!p)
	return pos;
    size = (p->used-offs > length) ? length : p->used-offs;
    if (size < 0)
	return pos;

    for (n=i=0; n<=offs && i<p->nsegm; n+=p->segm[i++].length) ;
    --i;
    text->format = SegmentFormat(p, i);
    if (n - offs < size)
	size = n - offs;
    
    pos += size;
    switch (text->format) {
    case FormatNarrow:
	text->ptr = p->text;
	text->firstPos = offs;
	if (size && text->ptr[text->firstPos+size-1] == '\n')
	    size--;
	break;
    case FormatWide:
	cnt = (size+1)/2;
        size = 2*cnt;
	if (cnt > obj->wchar_src.bufsize) {
	    obj->wchar_src.buf = (XChar2b*)XtRealloc((char*)obj->wchar_src.buf,
					      cnt*sizeof(*obj->wchar_src.buf));
	    obj->wchar_src.bufsize = cnt;
	}
	for (q = p->text+offs, q2 = obj->wchar_src.buf; cnt; cnt--, q2++) {
	    q2->byte1 = *q++;
	    q2->byte2 = *q++;
	}
	text->ptr = (char*)obj->wchar_src.buf;
	text->firstPos = 0;
	break;
    }
    text->length = size;
    return pos;
}


int
GuessEncoding(obj, out, in, size)
    WcharSrcObject obj;
    String *out;
    String in;
    int size;
{
    if (in[0] == '\033') {
	if (memcmp(in, "\033$)B", 4) == 0) {
	    size -= 4;
	    *out = XtMalloc(size+1);
	    memcpy(*out, in + 4, size);
	    (*out)[size]=0;
	    return 0;
	}
    }

    /* Default: copy as is */
    *out = XtMalloc(size+1);
    memcpy(*out, in, size);
    (*out)[size]=0;
    return 0;
}

/*	Function Name: WcharSrcReplace.
 *	Description: Replaces a block of text with new text.
 *	Arguments: src - the Text Source Object.
 *                 startPos, endPos - ends of text that will be removed.
 *                 text - new text to be inserted into buffer at startPos.
 *	Returns: XawEditError or XawEditDone.
 */
int
WcharSrcReplace(w, start, end, text)
    Widget w;
    XawTextPosition start;
    XawTextPosition end;
    XawTextBlock *text;
{
    String str;
    
/*work*/
    WcharSrcObject obj = (WcharSrcObject)w;
    switch (obj->wchar_src.replace_mode) {
    case XawWcharReplaceNone:
	return XawEditError;
    case XawWcharReplaceAll:
	if (start == end) {
	    /* Try to guess the string's encoding and convert it
             * if necessary
	     */
	    GuessEncoding(obj, &str, text->ptr, text->length);
	    WcharSrcDestroy(w, NULL, NULL);
	    obj->wchar_src.use_string_in_place = False;
	    obj->wchar_src.string = str;
	    _WcharSrcReset(obj);
	    XtFree(str);
	    XawTextDisplay(XtParent((Widget)obj));
	}
	break;
    case XawWcharReplaceBlock:
	/*FIXME: should do something */
	break;
    default:
	WcharInternalError("bad replace mode");
    }
    return XawEditDone;
}

/*	Function Name: WcharSrcScan
 *	Description: Scans the text source for the number and type
 *                   of item specified.
 *	Arguments: w - the WcharSrc object.
 *                 pos - the position to start scanning.
 *                 type - type of thing to scan for.
 *                 dir - direction to scan.
 *                 count - which occurence of this thing to search for.
 *                 include - whether or not to include the character found in
 *                           the position that is returned. 
 *	Returns: The position of the text.
 */
XawTextPosition
WcharSrcScan(w, pos, type, dir, count, include)
    Widget w;
    XawTextPosition pos;
    XawTextScanType type;
    XawTextScanDirection dir;
    int count;
    Boolean include;
{
    WcharSrcObject obj = (WcharSrcObject)w;
    XawTextPosition offs;
    int size;
    WcharChunk * p;

    if (!obj->wchar_src.first_chunk)
	return pos;
    if (type == XawstPositions) {
	switch (dir) {
	case XawsdLeft:
	    pos -= count;
	    break;
	case XawsdRight:
	    pos += count;
	}
	if (!include)
	    pos--;
	return pos;
    }
    
    switch (type) {
    case XawstParagraph:
	fprintf(stderr, "WcharSrcScan(): XawstParagraph not handled");
	return obj->wchar_src.filelength - (include ? 0 : 1);
    case XawstAll:
	return obj->wchar_src.filelength - (include ? 0 : 1);
    }
        
    p = LocateChunk(obj, pos, &pos, &offs);
    if (!p) {
	offs = XawTextScanError;
    }

    else switch (dir) {
    case XawsdRight:
	if (type == XawstEOL) {
	    int i;
	    XawTextPosition n;
	    for (n=i=0; n<=offs && i<p->nlines; n+=p->line_length[i++]) ;
	    while (--count) {
		if (i == p->nlines) {
		    if ((p = p->next)==NULL)
			break;
		    i = 0;
		}
		n += p->line_length[i++];
	    }
	    offs = n-1;
	}
	    
	else do {
	    if ((offs = FindForward(type, p->text, offs, p->used))==
		XawTextScanError) {
		for (size=p->used, p=p->next; p; size=p->used, p=p->next) {
		    pos += size;
		    if ((offs = FindForward(type, p->text, 0, p->used))!=
			XawTextScanError)
			break;
		}
	    }
	} while (offs != XawTextScanError && --count);
	break;
    case XawsdLeft:
	if (type == XawstEOL) {
	    int i;
	    XawTextPosition n;
	    
	    for (i=p->nlines, n=p->used;
		 n>=offs && i>0; n-=p->line_length[--i]) ;
	    if (n < 0) 
		++count;
			
	    while (--count) {
		if (i == 0) {
		    if ((p = p->prev)==NULL)
			break;
		    i = p->nlines;
		    n = p->used;
		    pos -= p->used;
		}
		n -= p->line_length[--i];
	    }
	    offs = n;
	}
	
	else do {
	    if ((offs = FindBackward(type, p->text, offs, offs))==
		XawTextScanError) {
		for (p = p->prev; p; p = p->prev) {
		    pos -= p->used;
		    if ((offs = FindBackward(type, p->text,
					     p->used, p->used))!=
			XawTextScanError)
			break;
		}
	    }
	    if (offs != XawTextScanError)
		offs--;
	} while (offs != XawTextScanError && --count);
	if (offs == XawTextScanError)
	    offs = 0;
/*	else
	    offs++;*/
    }
    if (offs != XawTextScanError)
	offs += pos;
    else
	return LengthOf(obj) - (include ? 0 : 1);

    if (include) switch (dir) {
    case XawsdRight:
	offs++;
	break;
    case XawsdLeft:
	offs--;
    }
    return offs;
}

/*	Function Name: WcharSrcSearch
 *	Description: Searches the text source for the text block passed
 *	Arguments: w - the TextSource Object.
 *                 pos - the position to start scanning.
 *                 dir - direction to scan.
 *                 text - the text block to search for.
 *	Returns: The position of the text we are searching for or
 *               XawTextSearchError.
 */
/*ARGSUSED*/
XawTextPosition
WcharSrcSearch(w, pos, dir, text)
    Widget w;
    XawTextPosition pos;
    XawTextScanDirection dir;
    XawTextBlock *text;
{
/*    WcharSrcObject obj = (WcharSrcObject)w;*/
    return XawTextSearchError;
}

/*	Function Name: WcharSrcSetSelection
 *	Description: allows special setting of the selection.
 *	Arguments: w - the WcharSrc object.
 *                 left, right - bounds of the selection.
 *                 selection - the selection atom.
 *	Returns: none
 */
/*ARGSUSED*/
void
WcharSrcSetSelection(w, start, end, selection)
    Widget w;
    XawTextPosition start;
    XawTextPosition end;
    Atom selection;
{
/*    WcharSrcObject obj = (WcharSrcObject)w;*/
}

/*	Function Name: WcharSrcConvertSelection
 *	Description: Selection converter.
 *	Arguments: w - the WcharSrc object.
 *                 selection - the current selection atom.
 *                 target    - the current target atom.
 *                 type      - the type to conver the selection to.
 * RETURNED        value, length - the return value that has been converted.
 * RETURNED        format    - the format of the returned value.
 *	Returns: TRUE if the selection has been converted.
 *
 */
/*ARGSUSED*/
Boolean
WcharSrcConvertSelection(w, selection, target, type, value, length, format)
    Widget w;
    Atom *selection;
    Atom *target;
    Atom *type;
    XtPointer *value;
    unsigned long *length;
    int *format;
{
/*    WcharSrcObject obj = (WcharSrcObject)w;*/
    return False;
}

/*ARGSUSED*/
Encoding
DetectEncoding(text, length)
    char *text;
    int  length;
{
    if (text[0] == ESCAPE)
	return ShinJis;
    else
	return ShiftJis;
}





