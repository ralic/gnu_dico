/* $Id: xref.c,v 1.1 2001/11/03 22:36:32 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/AsciiText.h>
#include <WcharText.h>
#include <WcharSrc.h>

#include <dict.h>
#include <gjdict.h>
#include <client.h>
#include <search.h>
#include <log.h>


enum {
    XREF_COUNT_LABEL,
    XREF_COUNT,
    XREF_LIST,
    XREF_BOX,
    
    XREF_MAXWIDGET
};

void makexrefwidgets(Widget);
static void xrefbuttonpress(Widget, XtPointer, XtPointer);
static void initxref(Widget, XtPointer, XtPointer);

Widget xref[XREF_MAXWIDGET]; 
Recno *xref_recno;
int xref_cnt;
ushort xref_jis;

static struct popup_info xref_popup_info = {
    "cross reference", NULL, -1, 0, 0, NULL,
};

void
MakeXrefPopup()
{
    Widget xref_form;
    XtCallbackRec popupCallbackRec[] = {
	initxref, NULL,
	NULL, NULL
    };

    xref_popup_info.widget = XtVaCreatePopupShell("xrefShell",
				      transientShellWidgetClass,
				      toplevel,
				      XtNpopupCallback, popupCallbackRec,
				      NULL);

    xref_form = XtVaCreateManagedWidget("xrefForm",
					formWidgetClass,
					xref_popup_info.widget,
					NULL);

    makexrefwidgets(xref_form);
}


void
makexrefwidgets(parent)
    Widget parent;
{
    xref[XREF_COUNT_LABEL] =
	XtVaCreateManagedWidget("countLabel",
				labelWidgetClass,
				parent,
				XtNborderWidth, 0,
				NULL);
    xref[XREF_COUNT] =
	XtVaCreateManagedWidget("count",
				labelWidgetClass,
				parent,
				XtNfromHoriz, xref[XREF_COUNT_LABEL],
				XtNborderWidth, 0,
				NULL);
    
    xref[XREF_LIST] =
	XtVaCreateManagedWidget("xrefList",
				viewportWidgetClass,
				parent,
				XtNfromVert, xref[XREF_COUNT],
				XtNallowVert, True,
				NULL);

    xref[XREF_BOX] =
	CreateNameList(xref[XREF_LIST],
		       0,
		       NULL,
		       False,
		       xrefbuttonpress,
		       0,
		       NULL,
		       NULL);
}

void
Showxrefs(w, client_data, call_data)
    Widget w;
    XtPointer client_data;
    XtPointer call_data;
{
    if (xref_popup_info.up != 1 && last_entry.refcnt == 0) {
	setstatus(True, "No references for this character");
	return;
    }
    setstatus(False, "Retrieving xref list");
    Showpopup(w, &xref_popup_info);
    setstatus(False, "Ready");
}

void
popdownxref()
{
    popdown(&xref_popup_info);
}

void
updatexref()
{
    if (xref_popup_info.up == 0)
	return;
    initxref(NULL, NULL, NULL);
}

int
find_by_recno(recno, entry)
    Recno recno;
    DictEntry *entry;
{
    int rc;
    
    sendf("REC %lu", recno);
    if (get_reply() == RC_OK && get_entry(entry, 1) == 1) {
	rc = 0;
    } else
	rc = -1;

    return rc;
}


void
setxref()
{
    int i;
    DictEntry entry;
    XChar2b **str;
    int argc;
    Arg args[2];
    char buf[16];
    
    if (!xref_recno)
	return
	    ;
    str = calloc(xref_cnt, sizeof(str[0]));
    if (!str) {
	error("%s:%d: low memory", __FILE__, __LINE__);
	return;
    }
    for (i = 0; i < xref_cnt; i++) {
	find_by_recno(xref_recno[i], &entry);
	str[i] = dup_16((XChar2b *)get_string(entry.kanji));
    }
    
    argc = 0;
    XtSetArg(args[argc], XtNencoding, XawTextEncodingChar2b); argc++;
    XtSetArg(args[argc], XtNfont, smallkfont); argc++;
    XtDestroyWidget(xref[XREF_BOX]);
    xref[XREF_BOX] = CreateNameList(xref[XREF_LIST],
				    xref_cnt,
				    (String*)str,
				    False,
				    xrefbuttonpress,
				    argc,
				    args,
				    NULL);
    
    for (i = 0; i < xref_cnt; i++) {
	free(str[i]);
    }
    free(str);

    sprintf(buf, "%d", xref_cnt);
    XtVaSetValues(xref[XREF_COUNT], XtNlabel, buf, NULL);
}

void
xrefbuttonpress(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    int num = (int)data;
    if (num >= xref_cnt) {
	Beep();
	return;
    }
    lookup_record(xref_recno[num]);
}

void
initxref(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    if (xref_jis == last_entry.Jindex) 
	return;
    if (xref_recno) {
	XtFree((char*)xref_recno);
	xref_recno = NULL;
	xref_jis = 0;
    }
    xref_cnt = last_entry.refcnt;
    xref_recno = (Recno*)XtCalloc(xref_cnt, sizeof(xref_recno));
    if (open_data_connection()) {
	setstatus(False, "Data conn. failed");
	return ;
    }
    
    sendf("XREF %d", last_entry.Jindex);
    if (get_reply() == RC_OK && get_recno(&xref_cnt, xref_recno) == 0) {
	xref_jis = last_entry.Jindex;
	setxref();
    }
    close_data_connection();
}




