#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <gjdictd.h>
#include <server.h>

char *
mkfullname(dir, name) 
    char *dir, *name;
{
    int dirlen = strlen(dir);
    int namelen = strlen(name);
    char *rp;

    rp = malloc(dirlen + namelen + (dir[dirlen - 1] != '/') + 1);
    if (!rp) {
	logmsg(L_EMERG, "not enough core");
	exit(1);
    }
    strcpy(rp, dir);
    if (dir[dirlen - 1] != '/')
	rp[dirlen++] = '/';
    strcpy(rp + dirlen, name);
    return rp;
}

void
format_skipcode(buf, code)
    char *buf;
    unsigned code;
{
    sprintf(buf, "%d-%d-%d", code >> 16, (code >> 8) & 0xf, code & 0xf);
}




