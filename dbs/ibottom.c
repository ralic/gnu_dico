/* $Id: ibottom.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

int
ibottom(iptr)
    Index *iptr;
{
    int rc;
    Iblock *blkptr;

    if (ilock(iptr) < 0) {
	return -1;
    }
    if (iptr->block_ref >= 0) {
	while (_blkup(iptr) == SUCCESS);
	_blkskip(iptr, 9999L);
    }
    while ((rc = _blkdn(iptr, 1)) == SUCCESS);
    if (rc == FAILURE)
	return FAILURE;

    blkptr = iptr->block_ref;

    if (!blkptr->num_keys)
	return KEY_EOF;
    else
	blkptr->key_on = blkptr->num_keys - 1;

    return SUCCESS;
}
