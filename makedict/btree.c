/* $Id: btree.c,v 1.1 2001/11/03 22:34:56 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <varargs.h>
#include <dbs.h>

char *progname;
static char deffmt[] =
"NAME:l:25,PSIZ:d,PSFT:d,KLEN:d,KMAX:d,KHLF:d,KNUM:d,KEYS,FLAG:x,ROOT:x";

void dump(char*);
void info(char*);
void usage();
    
int
main(argc, argv)
    int argc;
    char **argv;
{
    int c;
    char *fmt = deffmt;
    void (*f)() = NULL;
    int info_mode = 0;
    
    progname = strrchr(argv[0], '/');
    if (!progname)
	progname = argv[0];
    else
	progname++;
    
    while ((c = getopt(argc, argv, "hdif:")) != EOF) {
	switch (c) {
	case 'h':
	    usage();
	case 'f':
	    fmt = strdup(optarg);
	    break;
	case 'i':
	    f = info;
	    info_mode = 1;
	    break;
	case 'd':
	    f = dump;
	    break;
	default:
	    return -1;
	}
    }

    if (!f) 
	usage();
    
    if (info_mode) {
	mkprog(fmt);
	printheader();
    }
    argv += optind;
    while (*argv) 
	f(*argv++);
    return 0;
}

void
usage()
{
    static char text[] = "[-d] [-i] [-f keys]";
    printf("usage: %s %s\n", progname, text);
    exit(0);
}

#if has_stdarg
void die(char *fmt, ...)
#else
void
die(fmt, va_alist)
    char *fmt;
    va_dcl
#endif
{
    va_list ap;

    va_start(ap);
    fprintf(stderr, "%s: ", progname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

int
logmsg(lvl, ufmt, va_alist)
    int lvl;
    char *ufmt;
    va_dcl
{
    va_list ap;
    char buf[512];
    char fmt[512];
    int errmark = 0;
    int len;
    static char *prefix[] = {
	"Debug", 
	"Info",  
	"Notice", 
	"Warning",
	"Error",  
	"CRIT",   
	"ALERT",       
	"EMERG",       
    };
    
    strcpy(fmt, ufmt);
    len = strlen(fmt);
    if (len >= 2 && strcmp(&fmt[len-2], "%m") == 0) {
	errmark = errno;
	fmt[len-2] = 0;
    }
    
    fprintf(stderr, "%s: %s: ", progname, prefix[lvl & 0xff]);
    va_start(ap);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (errmark)
	fprintf(stderr, "%s", strerror(errmark));
    fprintf(stderr, "\n");
}


