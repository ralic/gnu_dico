/* $Id: itop.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

int
itop(iptr)
    Index *iptr;
{
    int rc;

    if (ilock(iptr) < 0) {
	return FAILURE;
    }
    if (iptr->block_ref >= 0) {
	while (_blkup(iptr) == SUCCESS) ;
	_blkskip(iptr, -9999L); /*???*/
    }
    while ((rc = _blkdn(iptr, -1)) == SUCCESS) ;
    if (rc == FAILURE) 
	return FAILURE;
	
    if (!iptr->block_ref->num_keys)
	return KEY_EOF;

    return SUCCESS;
}
