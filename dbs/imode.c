/* $Id: imode.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

void
imode(iptr, mode, set)
    Index *iptr;
    int mode;
    int set;
{
    if (set)
	setflag(iptr->idxflags, mode);
    else
	clearflag(iptr->idxflags, mode);
}
