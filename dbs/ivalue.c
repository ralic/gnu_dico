/* $Id: ivalue.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

void *
ivalue(iptr)
    Index *iptr;
{
    Key *key;

    key = _blkkey(iptr);
    if (key)
	return key->value;
    return (void*)0;
}
