#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <varargs.h>
#include <dbs.h>
#include <log.h>

static char *statstr[] = {
    "debug",
    "info",
    "warning",
    "error",
};

vlogmsg(code, fmt, ap)
    int code;
    char *fmt;
    va_list ap;
{
    fprintf(stderr, "%s:%s: ", progname, statstr[code]);
    if (fmt)
	vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

logmsg(code, va_alist)
    int code;
    va_dcl
{
    va_list ap;
    char *fmt;
	
    va_start(ap);
    fmt = va_arg(ap, char*);
    vlogmsg(code, fmt, ap);
    va_end(ap);
}

syserr(va_alist)
    va_dcl
{
    va_list ap;
    char *fmt;

    va_start(ap);
    fmt = va_arg(ap, char*);
    fprintf(stderr, "%s:%s: ", progname, statstr[L_ERR]);
    if (fmt)
	vfprintf(stderr, fmt, ap);
    fprintf(stderr, ": %s\n", strerror(errno));
    va_end(ap);
}
    
