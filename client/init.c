/* $Id: init.c,v 1.1 2001/11/03 22:36:05 gray Exp $ */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>		/* for srand48() */
#include <unistd.h>		/* for "access()" */
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Composite.h>

#include <gjdict.h>
#include <util.h>
#include <client.h>
#include <gjdict.xbm>

IPADDR server_addr;
int port_no;
Config config;
static Pixmap iconpixmap;
Atom wm_message, delete_message;

void translate_options();
int init_connection();
void setup_server_options();
void handle_delete(Widget, XtPointer, XEvent *, Boolean *);
void setup_deletewindow();
RETSIGTYPE sig_pipe(int sig);

struct popup_info connect_popup_info = {
    "connect", NULL, -1, 0, 0, NULL,
};

enum {
    SERVER_INPUT, PORT_INPUT,
    OK_CMD, CANCEL_CMD,
    CONNECT_NW
};

Widget connect_w[CONNECT_NW];

static char *fallback[] = {
    NULL
};


static OptionsRec optrec;

XtResource resources[] = {
#define off(c) XtOffsetOf(OptionsRec, c)
    {"server", "Server", XtRString, sizeof (String),
	 off(server), XtRImmediate, NULL},
    {"port", "Port", XtRString, sizeof (String),
	 off(port), XtRImmediate, NULL},
    {"verbose", "Verbose", XtRBoolean, sizeof(Boolean),
	 off(verbose), XtRImmediate, False},
    {"debug", "Debug", XtRInt, sizeof(int),
         off(debug), XtRImmediate, NULL},
    {"romaji", "Romaji", XtRBoolean, sizeof(Boolean),
	 off(romaji), XtRImmediate, (XtPointer)False},
    {"largeKanjiFont", "LargeKanjiFont", XtRString, sizeof(String),
	 off(LargeKanjiFont), XtRImmediate, NULL},
    {"smallKanjiFont", "SmallKanjiFont", XtRString, sizeof(String),
	 off(SmallKanjiFont), XtRImmediate, NULL},
    {"bushuFont", "BushuFont", XtRString, sizeof(String),
	 off(BushuFont), XtRImmediate, NULL},
    {"englishFont", "EnglishFont", XtRString, sizeof(String),
	 off(EnglishFont), XtRImmediate, NULL},
    {"dictBufferSize", "BufferSize", XtRInt, sizeof(int),
	 off(dict_entry.buf_size), XtRImmediate, (XtPointer)2048 },
    {"dictBufferIncr", "BufferIncr", XtRInt, sizeof(int),
	 off(dict_entry.buf_incr), XtRImmediate, (XtPointer)128 },
    {"bell", "Bell", XtRBoolean, sizeof(Boolean),
	 off(bell), XtRImmediate, (XtPointer)True},
    {"helpHistorySize", "HistorySize", XtRInt, sizeof(int),
	 off(help_history_size), XtRImmediate, (XtPointer)16 },
    {"dictHistorySize", "HistorySize", XtRInt, sizeof(int),
	 off(history_size), XtRImmediate, (XtPointer)16 },
    {"helpfile", "HelpFile", XtRString, sizeof(String),
	 off(helpfilename), XtRImmediate, NULL},
    {"highlightBackground", "Background", XtRPixel, sizeof(Pixel),
	 off(highlight.background), XtRImmediate, NULL },
    {"highlightForeground", "Foreground", XtRPixel, sizeof(Pixel),
	 off(highlight.foreground), XtRImmediate, NULL }
#undef off
};

/* command line args */
static XrmOptionDescRec options[] = {
    {"-server", ".server", XrmoptionSepArg, "localhost"},
    {"-port", ".port", XrmoptionSepArg, "7320"},
    {"-verbose", ".verbose", XrmoptionNoArg, "1" },
    {"-pinyin", ".pinyin", XrmoptionNoArg, "1" },
};

int
initsys(argc, argv)
    int argc;
    char **argv;
{
    int need_connect = 0;
    
    toplevel = XtVaAppInitialize(&Context, "Gjdict",
				 options, XtNumber(options),
				 &argc, argv, fallback,
				 NULL, NULL);
    
    if (argc > 1) {
	usage();
    }
    display = XtDisplay(toplevel);

    if (display == NULL)
	die("NULL DISPLAY");

    white = WhitePixel(display, 0);
    black = BlackPixel(display, 0);

    bzero(&optrec, sizeof(optrec));
    XtGetApplicationResources (toplevel, &optrec, resources,
			       XtNumber (resources),
			       NULL, 0);
    translate_options();

    init_dict_entry();
    
    if (init_connection())
	need_connect = 1;

    MakeWidgets();
    MakeConnectWidget();
    
    XtRealizeWidget(toplevel);
    mainwindow = XtWindow(toplevel);
    if (mainwindow == 0)
	die("NULL WINDOW");
    setup_deletewindow();
    iconpixmap = XCreateBitmapFromData(display, mainwindow,
				       (char *) gjdict_bits,
				       gjdict_width, gjdict_height);
    XtVaSetValues(toplevel, XtNiconName, "gjdict",
		  XtNiconPixmap, iconpixmap, NULL);


    XtVaGetValues(toplevel, XtNbackground, &default_background, NULL);
    XtVaGetValues(toplevel, XtNforeground, &default_foreground, NULL);
    signal(SIGPIPE, sig_pipe);
    
    if (need_connect)
	reconnect(True);
}

RETSIGTYPE
sig_pipe(sig)
    int sig;
{
    broken_pipe = 1;
    signal(sig, sig_pipe);
}

void
connectOkCallback(w, data, calldata)
    Widget w;
    XtPointer data;
    XtPointer calldata;
{
    String server_str, port_str;

    XtVaGetValues(connect_w[SERVER_INPUT], XtNstring, &server_str, NULL);
    server_addr = get_ipaddr(server_str);
    if (server_addr == (IPADDR)-1) {
	Xgetyn("unknown host: %s", server_str);
	return;
    }
    XtVaGetValues(connect_w[PORT_INPUT], XtNstring, &port_str, NULL);
    port_no = str2port(port_str);
    
    drop_connection();
    if (init_connection() == 0) {
	set_server_name(server_str, port_str);
	popdown((struct popup_info*)data);
    }
}

void
closeCallback(w, data, calldata)
    Widget w;
    XtPointer data;
    XtPointer calldata;
{
    popdown((struct popup_info*)data);
}

static XtCallbackRec okCallbackRec[] = {
    connectOkCallback, &connect_popup_info,
    NULL, NULL,
};
static XtCallbackRec cancelCallbackRec[] = {
    closeCallback, &connect_popup_info,
    NULL, NULL,
};
static XtCallbackRec quitCallbackRec[] = {
    quit, NULL,
    NULL, NULL,
};

void
MakeConnectWidget()
{
    Widget form, server_label, port_label;

    connect_popup_info.widget =
	XtVaCreatePopupShell("connectShell",
			     transientShellWidgetClass,
			     toplevel,
			     NULL);

    form =
	XtVaCreateManagedWidget("connectForm",
				formWidgetClass,
				connect_popup_info.widget,
				NULL);

    server_label =
	XtVaCreateManagedWidget("serverLabel",
				labelWidgetClass,
				form,
				XtNborderWidth, 0,
				NULL);

    connect_w[SERVER_INPUT] =
	XtVaCreateManagedWidget("serverInput",
				asciiTextWidgetClass,
				form,
				XtNfromHoriz, server_label,
				XtNeditType, XawtextEdit,
				NULL);

    port_label =
	XtVaCreateManagedWidget("portLabel",
				labelWidgetClass,
				form,
				XtNborderWidth, 0,
				XtNfromVert, server_label,
				NULL);

    connect_w[PORT_INPUT] =
	XtVaCreateManagedWidget("portInput",
				asciiTextWidgetClass,
				form,
				XtNfromVert, connect_w[SERVER_INPUT],
				XtNfromHoriz, port_label,
				XtNeditType, XawtextEdit,
				NULL);

    connect_w[OK_CMD] =
	XtVaCreateManagedWidget("ok",
				commandWidgetClass,
				form,
				XtNfromVert, port_label,
				XtNvertDistance, 20,
				XtNcallback, okCallbackRec,
				NULL);
	
    connect_w[CANCEL_CMD] =
	XtVaCreateManagedWidget("cancel",
				commandWidgetClass,
				form,
				XtNfromVert, port_label,
				XtNfromHoriz, connect_w[OK_CMD],
				XtNvertDistance, 20,
				XtNcallback, cancelCallbackRec,
				NULL);
    
}

void
connectCallback(w, data, calldata)
    Widget w;
    XtPointer data;
    XtPointer calldata;
{
    reconnect(False);
}

void
reconnect(cancel_quits)
    Boolean cancel_quits;
{
    XtVaSetValues(connect_w[SERVER_INPUT],
		  XtNstring, config.server,
		  NULL);
    XtVaSetValues(connect_w[PORT_INPUT],
		  XtNstring, config.port,
		  NULL);
    XtVaSetValues(connect_w[CANCEL_CMD],
		  XtNcallback, cancel_quits ? quitCallbackRec : cancelCallbackRec,
		  NULL);
    ShowExclusivePopup(toplevel, &connect_popup_info);
}

void
setup_server_options()
{
}

void
set_server_name(servername, port)
    String servername;
    String port;
{
    if (config.server)
	XtFree(config.server);
    config.server = XtNewString(servername);
    if (config.port)
	XtFree(config.port);
    config.port = XtNewString(port);
}

void
translate_options()
{
    /* Server IP */
    server_addr = get_ipaddr(optrec.server);
    port_no = str2port(optrec.port);
    set_server_name(optrec.server, optrec.port); 
    config.LargeKanjiFont = optrec.LargeKanjiFont;
    config.SmallKanjiFont = optrec.SmallKanjiFont;
    config.BushuFont = optrec.BushuFont;
    config.EnglishFont = optrec.EnglishFont;
    config.verbose = optrec.verbose; 
    config.debug = optrec.debug;
    config.romaji = optrec.romaji;
    config.dict_entry.buf_size = optrec.dict_entry.buf_size;
    config.dict_entry.buf_incr = optrec.dict_entry.buf_incr;
    config.bell = optrec.bell;
    config.help_history_size = optrec.help_history_size;
    config.history_size = optrec.history_size;
    config.helpfilename = optrec.helpfilename;
    if (!optrec.highlight.background)
	config.highlight.background = white;
    else
	config.highlight.background = optrec.highlight.background;
    if (optrec.highlight.foreground)
	config.highlight.foreground = optrec.highlight.foreground;
}

/* handle_delete
 *	Its sole purpose is to handle WM_DELETE messages
 */
void
handle_delete(w, closure, event, cont)
    Widget w;
    XtPointer closure;
    XEvent *event;
    Boolean *cont;
{
    XClientMessageEvent *cevent = (XClientMessageEvent *) event;

    if (cevent->type != ClientMessage)
	return;
    if (cevent->message_type != wm_message)
	return;
    if (cevent->data.l[0] != delete_message)
	return;
#if 0
    if (w == options_popup_info.widget) {
	OptionsCallback(options_popup_info.widget,
			(XtPointer) NULL, (XtPointer) NULL);
	return;
    }
    if (w == search_popup_info.widget) {
	SearchCallback(search_popup_info.widget,
		       (XtPointer) NULL, (XtPointer) NULL);
	return;
    }
#endif    
    quit(NULL, NULL, NULL);
    /* NOTREACHED */
}

void
setup_deletewindow()
{
    wm_message = XInternAtom(display, "WM_PROTOCOLS", False);
    delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);


    XtAddEventHandler(toplevel, NoEventMask, True,
		      handle_delete, (XtPointer) NULL);
    XChangeProperty(display, mainwindow,
		    wm_message, XA_ATOM, 32, PropModeReplace,
		    (unsigned char *) &delete_message, 1);

    if (wm_message == (Atom) NULL) 
	die("unable to create wm_message property");

    if (delete_message == (Atom) NULL) 
	die("unable to create delete property");
}


