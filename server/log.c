/*
 * log.c	Logging module.
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <varargs.h>
#include <dbs.h>
#include "log.h"

#ifndef LOGFACILITY
# define LOGFACILITY LOG_LOCAL4
#endif

int console_log;
int debug_level[NUMDEBUG];

extern char	*log_dir;
extern char	*progname;

int vlog(int lvl, char *fmt, va_list ap);

void
initlog()
{
    openlog(progname, LOG_PID, LOGFACILITY);
}

void
set_debug_level(str)
    char *str;
{
    int fac, lev;
    char *p;
    
    fac = strtol(str, &p, 10);
    if (*p != '.') {
	logmsg(L_ERR|L_CONS, "bad debug format (near `%s')", p);
	return;
    }
    if (fac < 0 || fac > NUMDEBUG) {
	logmsg(L_ERR|L_CONS, "bad debug facility: %d", fac);
	return;
    }
    lev = strtol(p+1, &p, 10);
    if (*p != 0) {
	logmsg(L_ERR|L_CONS, "bad debug level (near `%s')", p);
	return;
    }
    debug_level[fac] = lev;
    if (fac == DEBUG_GRAM && lev >= 10)
	enable_yydebug();
}

/*
 *	Log the message to the logfile. 
 */
int
vlog(lvl, ufmt, ap)
    int lvl;
    char *ufmt;
    va_list ap;
{
    char *s;
    int prio = LOG_INFO;
    int cons = console_log;
    char buf[512];
    char fmt[512];
    int errmark = 0;
    int len;
    static struct {
	char *prefix;
	int priority;
    } loglevels[] = {
	"Debug",  LOG_DEBUG,
	"Info",   LOG_INFO,
	"Notice", LOG_NOTICE,
	"Warning",LOG_WARNING,
	"Error",  LOG_ERR,
	"CRIT",   LOG_CRIT,
	"ALERT",  LOG_ALERT,     
	"EMERG",  LOG_EMERG,     
    };
    
    strcpy(fmt, ufmt);
    len = strlen(fmt);
    if (len >= 2 && strcmp(&fmt[len-2], "%m") == 0) {
	errmark = errno;
	fmt[len-2] = 0;
    }
    
    if (lvl & L_CONS)
	cons++;

    s    = loglevels[lvl & L_MASK].prefix;
    prio = loglevels[lvl & L_MASK].priority;

    vsprintf(buf, fmt, ap);
    if (errmark)
	sprintf(buf+strlen(buf), "%s", strerror(errmark));
    
    syslog(prio, "%s: %s", s, buf);
    if (cons) {
	fprintf(stderr, "%s: %s\n", progname, buf);
    }
}


int
dlog(msg, va_alist)
    char *msg;
    va_dcl
{
    va_list ap;
    int r = 0;

    va_start(ap);
    r = vlog(L_DEBUG, msg, ap);
    va_end(ap);
    return r;
}

int
logmsg(lvl, msg, va_alist)
    int lvl;
    char *msg;
    va_dcl
{
    va_list ap;
    int r;
    
    va_start(ap);
    r = vlog(lvl, msg, ap);
    va_end(ap);
    
    return r;
}

int
die(msg, va_alist)
    char *msg;
    va_dcl
{
    va_list ap;
    
    va_start(ap);
    vlog(L_ERR|L_CONS, msg, ap);
    va_end(ap);
    exit(1);
}



