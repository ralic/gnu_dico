/* $Id: help.c,v 1.1 2001/11/03 22:36:00 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Box.h>
#include <WcharText.h>

#include <client.h>
#include <help.h>
#include <cystack.h>

enum {
    TOPIC_LIST,
    TOPIC_TITLE,
    TOPIC_TEXT,
    TOPIC_XREF,
    XREF_LIST,
    CLOSE,
    BACK,
    FORWARD,
    HISTORY,
    
    HELP_WIDGETS
};

enum {
    HIST_LIST,
    HIST_CLOSE,
    HIST_SELECT,
    HIST_BOX,
    
    HIST_WIDGETS
};

Widget helpw[HELP_WIDGETS];
Widget *topic_widget;
static struct popup_info popup_info = {
    "help", NULL, -1, 0, 0, NULL,
};

Widget histw[HIST_WIDGETS];
Widget *history_widgets;
static int selected_topic;
static struct popup_info history_popup = {
    "help history", NULL, -1, 0, 0, NULL,
};

int topic_cnt = -1;
static TopicXref xref;
static Cystack *history;

static int find_topic;

static void help_popup __PROTO((Widget, XtPointer, XtPointer));
static void help_select __PROTO((Widget, XtPointer, XtPointer));
static void xref_select __PROTO((Widget, XtPointer, XtPointer));
static void help_close __PROTO((Widget, XtPointer, XtPointer));
static void help_back __PROTO((Widget, XtPointer, XtPointer));
static void help_forward __PROTO((Widget, XtPointer, XtPointer));
static void show_help_history __PROTO((Widget, XtPointer, XtPointer));
static void help_history_popup __PROTO((Widget, XtPointer, XtPointer));
static void help_history_select __PROTO((Widget, XtPointer, XtPointer));
static void select_history_topic __PROTO((Widget, XtPointer, XtPointer));
static void do_help_select __PROTO((int));
void MakeHelpPopup();

void
init_help()
{
    if (topic_cnt == -1) {
	setstatus(False, "Preparing help...");
	MakeHelpPopup();
    }
    if (topic_cnt == 0) {
	setstatus(False, "No help available");
	return;
    }
    setstatus(False, "Ready");
}

void
HelpCallback(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    init_help();
    if (topic_cnt) {
	find_topic = data ? *(int*)data : -1;
	Showpopup(w, &popup_info);
    }
}

void
show_help_history(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    ShowExclusivePopup(w, &history_popup);
}

void
MakeHistoryPopup()
{
    XtCallbackRec popupCallbackRec[] = {
	help_history_popup, NULL,
	NULL, NULL
    };

    history_popup.widget =
	XtVaCreatePopupShell("helpHistoryShell",
			     transientShellWidgetClass,
			     toplevel,
			     XtNpopupCallback, popupCallbackRec,
			     NULL);

    form =
	XtVaCreateManagedWidget("helpHistoryForm",
				formWidgetClass,
				history_popup.widget,
				NULL);

    histw[HIST_LIST] =
	XtVaCreateManagedWidget("historyList",
				viewportWidgetClass,
				form,
				XtNallowVert, True,
				NULL);

    histw[HIST_BOX] =
	CreateNameList(histw[HIST_LIST],
		       0,
		       NULL,
		       True,
		       select_history_topic,
		       0,
		       NULL,
		       NULL);
    histw[HIST_SELECT] = 
	XtVaCreateManagedWidget("select",
				commandWidgetClass,
				form,
				XtNfromVert, histw[HIST_LIST],
				NULL);
    histw[HIST_CLOSE] = 
	XtVaCreateManagedWidget("close",
				commandWidgetClass,
				form,
				XtNfromVert, histw[HIST_LIST],
				XtNfromHoriz, histw[HIST_SELECT],
				NULL);
    AddCloseCommand(&history_popup, histw[HIST_CLOSE]);
    XtAddCallback(histw[HIST_SELECT], XtNcallback,
		  help_history_select, (XtPointer)&history_popup);
}

#define HORIZ_DISTANCE 10

void
MakeHelpPopup()
{
    Widget form, label, label2, label3;
    String *str;
    int n, height;
    int size;
    XtCallbackRec callbackRec[] = {
	help_popup, (XtPointer)&find_topic,
	NULL, NULL
    };
    
    topic_cnt = LoadHelpFile(&str);
    if (!topic_cnt)
	return;
    
    popup_info.widget =
	XtVaCreatePopupShell("helpShell",
			     transientShellWidgetClass,
			     toplevel,
			     XtNpopupCallback, callbackRec,
			     NULL);

    form =
	XtVaCreateManagedWidget("helpForm",
				formWidgetClass,
				popup_info.widget,
				NULL);

    label = XtVaCreateManagedWidget("topic",
				    labelWidgetClass,
				    form,
				    XtNborderWidth, 0,
				    NULL);

    helpw[TOPIC_LIST] =
	XtVaCreateManagedWidget("topicList",
				viewportWidgetClass,
				form,
				XtNfromVert, label,
				XtNallowVert, True,
				NULL);
    CreateNameList(helpw[TOPIC_LIST],
		   topic_cnt,
		   str,
		   False,
		   help_select,
		   0,
		   NULL,
		   &topic_widget);

    helpw[TOPIC_TITLE] =
	  XtVaCreateManagedWidget("topicTitle",
				  labelWidgetClass,
				  form,
				  XtNborderWidth, 0,
				  XtNlabel, "",
				  XtNfromHoriz, helpw[TOPIC_LIST],
				  XtNresize, False,
#ifdef HORIZ_DISTANCE
				  XtNhorizDistance, HORIZ_DISTANCE,
#endif
				  NULL);

    helpw[TOPIC_TEXT] =
	XtVaCreateManagedWidget("topicText",
				wcharTextWidgetClass,
				form,
				XtNstring, "                            ",
				XtNfromHoriz, helpw[TOPIC_LIST],
#ifdef HORIZ_DISTANCE
				XtNhorizDistance, HORIZ_DISTANCE,
#endif
				XtNfromVert, helpw[TOPIC_TITLE],
				XtNdisplayCaret, False,
				XtNwrap, XawtextWrapWord,
				XtNscrollVertical, XawtextScrollWhenNeeded,
				NULL);

    helpw[CLOSE] =
	XtVaCreateManagedWidget("close",
				commandWidgetClass,
				form,
				XtNfromVert, helpw[TOPIC_LIST],
				NULL);
    XtAddCallback(helpw[CLOSE],
		  XtNcallback, help_close,
		  (XtPointer) 0);

    helpw[BACK] =
	XtVaCreateManagedWidget("back",
				commandWidgetClass,
				form,
				XtNfromVert, helpw[TOPIC_LIST],
				XtNfromHoriz, helpw[CLOSE],
				NULL);
    XtAddCallback(helpw[BACK],
		  XtNcallback, help_back,
		  (XtPointer) 0);

    helpw[FORWARD] =
	XtVaCreateManagedWidget("forward",
				commandWidgetClass,
				form,
				XtNfromVert, helpw[TOPIC_LIST],
				XtNfromHoriz, helpw[BACK],
				NULL);
    XtAddCallback(helpw[FORWARD],
		  XtNcallback, help_forward,
		  (XtPointer) 0);

    helpw[HISTORY] =
	XtVaCreateManagedWidget("history",
				commandWidgetClass,
				form,
				XtNfromVert, helpw[TOPIC_LIST],
				XtNfromHoriz, helpw[FORWARD],
				NULL);
    XtAddCallback(helpw[HISTORY],
		  XtNcallback, show_help_history,
		  (XtPointer) 0);

    label3 =
	XtVaCreateManagedWidget("crossRefTitle",
				labelWidgetClass,
				form,
				XtNborderWidth, 0,
				XtNfromHoriz, helpw[TOPIC_LIST],
				XtNfromVert, helpw[TOPIC_TEXT],
#ifdef HORIZ_DISTANCE
				XtNhorizDistance, HORIZ_DISTANCE,
#endif
				NULL);
    helpw[TOPIC_XREF] =
	XtVaCreateManagedWidget("crossRef",
				viewportWidgetClass,
				form,
				XtNfromHoriz, helpw[TOPIC_LIST],
#ifdef HORIZ_DISTANCE
				XtNhorizDistance, HORIZ_DISTANCE,
#endif
				XtNfromVert, label3,
				XtNallowVert, True,
				NULL);
    helpw[XREF_LIST] =
	CreateNameList(helpw[TOPIC_XREF],
		       0,
		       NULL,
		       False,
		       NULL,
		       0,
		       NULL,
		       NULL);

    MakeHistoryPopup();
    
    
    size = config.help_history_size;
    if (size <= 0)
	size = 16;
    history = cystack_create(size+1, sizeof(int));
    if (!history) 
	die("%s:%d: Not enough core", __FILE__, __LINE__);
    info("allocated %d entries for help history queue", size); 
}


void
help_popup(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    if (*(int*)data >= 0)
	do_help_select(*(int*)data);
}

void
do_help_select(n)
    int n;
{
    XtVaSetValues(helpw[TOPIC_TITLE],
		  XtNlabel, GetTopicTitle(n),
		  NULL);
    XtVaSetValues(helpw[TOPIC_TEXT],
		  XtNstring, GetTopicText(n),
		  NULL);
    XtVaSetValues(topic_widget[n],
		  XtNstate, True,
		  NULL);
    LoadTopicXref(n, &xref);    
    XtDestroyWidget(helpw[XREF_LIST]);
    helpw[XREF_LIST] = 
	CreateNameList(helpw[TOPIC_XREF],
		       xref.cnt,
		       xref.str,
		       False,
		       xref_select,
		       0,
		       NULL,
		       NULL);
}

void
help_select(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    if ((int) call_data) {
	int n = (int)data;

	do_help_select(n);
	cystack_push(history, &n);
    }
}


void
xref_select(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    if ((int)call_data) {
	int n = (int) data;
	XtVaSetValues(topic_widget[ xref.offs[n] ], XtNstate, True, NULL);
	help_select(w, (XtPointer)xref.offs[n], call_data);
    }
}


void
help_close(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    Showpopup(w, &popup_info);
}
    

void
help_back(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    int val;
    
    if (cystack_pop(history, &val, STACK_UP))
	return;
    cystack_get(history, &val, 0);
    XtVaSetValues(topic_widget[val], XtNstate, True, NULL);
    do_help_select(val);
}

void
help_forward(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    int val;
    
    if (cystack_pop(history, &val, STACK_DN))
	return;
    XtVaSetValues(topic_widget[val], XtNstate, True, NULL);
    do_help_select(val);
}

void
select_history_topic(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    selected_topic = (int) data;
}

void
help_history_select(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    if (selected_topic >= 0) {
	int n;
	cystack_get(history, &n, selected_topic);
	XtVaSetValues(topic_widget[n], XtNstate, True, NULL);
	do_help_select(n);
    }
}

void
help_history_popup(w, data, call_data)
    Widget w;
    XtPointer data;
    XtPointer call_data;
{
    int i, len, n;
    String *str;
	
    XtDestroyWidget(histw[HIST_BOX]);

    len = cystack_length(history);
    str = calloc(len, sizeof(str[0]));
    if (!str) {
	error("%s:%d: low core!", __FILE__, __LINE__);
	return;
    }
    
    for (i = 0; i < len; i++) {
	cystack_get(history, &n, i);
	str[i] = GetTopicTitle(n);
    }
    
    histw[HIST_BOX] =
	CreateNameList(histw[HIST_LIST],
		       len,
		       str,
		       False,
		       select_history_topic,
		       0,
		       NULL,
		       NULL);

    free(str);
}


void
help_me(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    int num;
    
    if (*num_params != 1)
	parm_cnt_error("help-me", 1, *num_params);
    init_help();
    num = FindTopic(params[0]);
    if (num < 0) {
	warning("help_me: topic `%s' not found", params[0]);
	return;
    }
    HelpCallback(w, (XtPointer)&num, NULL);
}
