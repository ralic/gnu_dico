#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>

#include <client.h>
#include <options.h>
#if has_stdarg
# include <stdarg.h>
# define vararg_start(a,f) va_start(a,f)
#else
# include <varargs.h>
# define vararg_start(a,f) va_start(a)
#endif

extern char *progname;

#if has_stdarg
void
die(char *fmt, ...)
#else    
/*VARARGS1*/
void
die(fmt, va_alist) 
    char *fmt;
    va_dcl
#endif    
{
    va_list ap;

    vararg_start(ap, fmt);
    fprintf(stderr, "%s:fatal error:", progname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

#if has_stdarg
void
warning(char *fmt, ...)
#else
/*VARARGS1*/
void
warning(fmt, va_alist) 
    char *fmt;
    va_dcl
#endif
{
    va_list ap;

    vararg_start(ap, fmt);
    fprintf(stderr, "%s:warning: ", progname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

#if has_stdarg
void
error(char *fmt, ...)
#else
/*VARARGS1*/
void
error(fmt, va_alist) 
    char *fmt;
    va_dcl
#endif
{
    va_list ap;

    vararg_start(ap, fmt);
    fprintf(stderr, "%s:error:", progname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

#if has_stdarg
void
serror(char *fmt, ...)
#else
/*VARARGS1*/
void
serror(fmt, va_alist) 
    char *fmt;
    va_dcl
#endif
{
    va_list ap;

    vararg_start(ap, fmt);
    fprintf(stderr, "%s:error:", progname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ":%s\n", strerror(errno));
    va_end(ap);
}

#if has_stdarg
void
info(char *fmt, ...)
#else
/*VARARGS1*/
void
info(fmt, va_alist) 
    char *fmt;
    va_dcl
#endif
{
    va_list ap;

    if (!config.verbose)
	return;
    vararg_start(ap, fmt);
/*    printf("%s:", progname);*/
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

#if has_stdarg
void
debug(char *fmt, ...)
#else
/*VARARGS1*/
void
debug(fmt, va_alist) 
    char *fmt;
    va_dcl
#endif
{
    va_list ap;

    vararg_start(ap, fmt);
    fprintf(stderr, "%s:DEBUG:", progname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}


