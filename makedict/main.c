/* $Id: main.c,v 1.1 2001/11/03 22:34:40 gray Exp $
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
#include <dict.h>
#include <makedict.h>

char *dictdir = INPUTDICTPATH;
char *kanjidict = "kanjidic.gz";
char *edict = "edict.gz";
char *outdir = DICTPATH;
char *dictname = DICTFILENAME;
char *textname = TEXTFILENAME;
char *progname;
int verbose;

Uint textpageshift = DEFTEXTPAGESHIFT;
Uint textpagesize;

void make_names(void);
void helptext(char*);

char usage[] = "\
usage: $0 [-cv][-d dictdir][-k kanjidic][-e edict][-o outdir][-t pagesize]\n\
       $0 -L\n\
       $0 -V\n\
This is $0: program that converts `kanjidic.gz' and `edict.gz'\n\
files into a binary dictionary for use with gjdict.\n\
To compile run `$0 -c'. This assumes both input dictionaries to be\n\
located in " INPUTDICTPATH " directory and all output files to be\n\
placed in " DICTPATH ". To override these defaults use following\n\
switches:\n\
 	-d dictdir	Set input dictionary path to `dictdir'\n\
 	-k kanjidic	Set alternate kanjidic file name\n\
 	-e edict	Set alternate edict file name\n\
 	-o outdir	Set directory name\n\
Another options are:\n\
	-t pagesize	Set text storage file page size. `pagesize' can be\n\
	                either a number which is rounded to the nearest power\n\
	                of 2 or a string in a form ^<number> in which case\n\
                        the pagesize is set to 2^<number> bytes.
These options cause program to terminate with exit code of 0:
	-L		Display the license info\n\
	-V		Display current program version\n\
";

char version[] =
"$0 $Revision: 1.1 $($Date: 2001/11/03 22:34:40 $)\n";

char licence[] = "\
$0 $Revision: 1.1 $($Date: 2001/11/03 22:34:40 $)\n\
Copyright (C) 1998 Gray\n\
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
                                                
int
main(argc, argv)
    int argc;
    char **argv;
{
    int c;
    Uint val;

    progname = strrchr(argv[0], '/');
    if (!progname)
	progname = argv[0];
    else
	progname++;

    if (argc == 1)
	helptext(usage);
    
    while ((c = getopt(argc, argv, "cd:k:Le:o:vVt:")) != EOF) {
	switch (c) {
	case 'c':
	    break;
	case 'd':
	    dictdir = strdup(optarg);
	    break;
	case 'k':
	    kanjidict = strdup(optarg);
	    break;
	case 'e':
	    edict = strdup(optarg);
	    break;
	case 'o':
	    outdir = strdup(optarg);
	    break;
	case 'v':
	    verbose = !verbose;
	    break;
	case 't':
	    if (optarg[0] == '^') {
		val = atoi(optarg+1);
		if (val <= 0) {
		    logmsg(L_WARN, "ignored invalid 2's power value for textpagesize");
		    break;
		}
	    } else {
		val = atoi(optarg);
		if (val <= 0) {
		    logmsg(L_WARN, "ignored invalid value for textpagesize");
		    break;
		}
		val = log2(val);
	    }

	    if (MINTEXTPAGESHIFT <= val) 
		textpageshift = val;
	    else
		logmsg(L_WARN, "ignored invalid value for textpagesize");
	    break;
	case 'V':
	    helptext(version);
	case 'L':
	    helptext(licence);

	default:
	    return -1;
	}
    }
    textpagesize = 1 << textpageshift;
    make_names();
    return compile();
}

void
make_names()
{
    int i;
    kanjidict = mkfullname(dictdir, kanjidict);
    edict = mkfullname(dictdir, edict);
    dictname = mkfullname(outdir, dictname);
    textname = mkfullname(outdir, textname);
    for (i = 0; i < TreeLast; i++)
	indexes[i].name = mkfullname(outdir, indexes[i].name);
}

char *
mkfullname(dir, name)
    char *dir, *name;
{
    int dirlen = strlen(dir);
    int namelen = strlen(name);
    char *rp;

    rp = malloc(dirlen + namelen + (dir[dirlen-1] != '/') + 1);
    if (!rp) 
	die("Not enough core");
    strcpy(rp, dir);
    if (dir[dirlen-1] != '/')
	rp[dirlen++] = '/';
    strcpy(rp + dirlen, name);
    return rp;
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


/* helptext(): Print text on console and exit. Before printing scan text for
 * RCS keywords ("\$[a-zA-Z]+:\(.*\)\$") and replace them with the keyword's 
 * walue ("\1"). Also replaces $0 with the program name.
 * NOTE: The function does not check the whole regexp above, it simply
 * assumes that every occurence of '$' in the text is a start of RCS keyword.
 * It suffices for all used messages, but if you add any messages containing
 * '$' on it's own, you have to modify helptext() accordingly.
 */
void
helptext(text)
    char *text;
{
    char *p;

    p = text;
    while (*p) {
	if (*p == '$') {
	    if (p[1] == '0') {
		*p = 0;
		printf("%s%s", text, progname);
		*p = '$';
		p += 2;
		text = p;
		continue;
	    }
	    *p = 0;
	    printf("%s", text);
	    *p = '$';
	    while (*p++ != ':')
		;
	    text = p;
	    while (*p != '$')
		p++;
	    *p = 0;
	    printf("%s", text);
	    *p++ = '$';
	    text = p;
	} else
	    p++;
    }
    if (p > text)
	printf("%s", text);
    exit(0);
}

