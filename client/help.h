/* $Id: help.h,v 1.1 2001/11/03 22:36:32 gray Exp $
 */
typedef struct {
    int cnt;
    String *str;
    long *offs;
} TopicXref;

void HelpCallback(Widget w, XtPointer data, XtPointer call_data);
int LoadHelpFile __PROTO((String **));
int LoadTopicXref __PROTO((int, TopicXref *));
String GetTopicTitle __PROTO((int));
String GetTopicText __PROTO((int));
int FindTopic __PROTO((String));
void help_me __PROTO((Widget, XEvent *, String *, Cardinal *));


