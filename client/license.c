/* $Id: license.c,v 1.1 2001/11/03 22:36:09 gray Exp $
 */
/*  Gjdict
 *  Copyright (C) 1999-2000 Gray
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
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <client.h>

#define __PROTO(c) c

char *helptext __PROTO((char *text));

static char help_str[] = "\
GNU Japanese Dictionary $Revision: 1.1 $($Date: 2001/11/03 22:36:09 $)\n\
Copyright (C) 1999-2000 Gray\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\
"; 


static void
popdownCallback(w, data, calldata)
    Widget w;
    XtPointer data;
    XtPointer calldata;
{
    popdown((struct popup_info*)data);
}

void
licenseDialog(w, data, calldata)
    Widget w;
    XtPointer data, calldata;
{
    Widget shell, form, btn[2]; 
    struct popup_info popup_info;
    XtAppContext    context;
    XtCallbackRec closeCallbackRec[] = {
	popdownCallback, &popup_info,
	NULL, NULL,
    };

    shell = XtVaCreatePopupShell("licenseShell",
				 transientShellWidgetClass,
				 toplevel,
				 NULL);
    
    form = XtCreateWidget("form", formWidgetClass, shell, NULL, 0);

    btn[0] = XtVaCreateWidget("info",
			      labelWidgetClass,
			      form,
			      XtNlabel, helptext(help_str),
			      NULL);

    btn[1] = XtVaCreateWidget("close",
			      commandWidgetClass,
			      form,
			      XtNcallback, closeCallbackRec,
			      NULL);
    
    XtManageChildren(btn, 2);
    XtManageChild(form);
    XawFormDoLayout(form, 0);

    popup_info.name = "License";
    popup_info.widget = shell;
    popup_info.up = -1;
    popup_info.rel_x = 0;
    popup_info.rel_y = 0;
    popup_info.rel_from = NULL;

    ShowExclusivePopup(w, &popup_info);

    context = XtWidgetToApplicationContext(shell);
    while (popup_info.up == 1 || XtAppPending(context)) {
	XtAppProcessEvent (context, XtIMAll);
    }
    
    XtDestroyWidget(shell);  /* blow away the dialog box and shell */
}

/* helptext(): Creates a copy of the text, having scanned it for
 * RCS keywords ("\$[a-zA-Z]+:\(.*\)\$") and replaced them with the keyword's 
 * walue ("\1").
 * NOTE: The function does not check the whole regexp above, it simply
 * assumes that every occurence of '$' in the text is a start of RCS keyword.
 * It suffices for all used messages, but if you add any messages containing
 * '$' on it's own, you have to modify helptext() accordingly.
 */
char *
helptext(text)
    char *text;
{
    char *rets, *p, *q;

    rets = p = q = XtNewString(text);
    while (*p) {
	if (*p == '$') {
	    while (*p++ != ':')
		;
	    while (*p && *p != '$')
		*q++ = *p++;
	    if (*p)
		++p;
	} else
	    *q++ = *p++;
    }
    *q = 0;
    return rets;
}
