/* $Id: icmplen.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

int
icmplen(iptr, n, len)
    Index *iptr;
    int n;
    int len;
{
    if (n >= iptr->numsubkeys)
	return -1;
    switch (iptr->subkey[n].type) {
    case KEY_CHR_8:
    case KEY_CASECHR_8:
    case KEY_CHR_16:
	if (len <= iptr->subkey[n].len)
	    iptr->subkey[n].cmplen = len;
	/*FALLTHRU*/
    default:
	return -1;
    }
}
