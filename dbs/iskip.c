/* $Id: iskip.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

Recno
iskip(iptr, num_skip)
    Index *iptr;
    Recno num_skip;
{
    Iblock *block_ptr;
    int sign, rc;
    Recno num_left;

    if (ilock(iptr) < 0) {
	return FAILURE;
    }
    num_left = num_skip;
    if (num_skip < 0)
	sign = -1;
    else
	sign = 1;

    rc = 0;

    if (iptr->block_ref < 0)
	rc = itop(iptr);
    else {
	block_ptr = iptr->block_ref;
	if (block_ptr->key_on >= block_ptr->num_keys)
	    rc = ibottom(iptr);	/* EOF */
    }
    if (rc < 0) {
	return -num_skip;	/* Error. What to return ??? */
    }
    if (rc == KEY_EOF)
	return 0;
 
    while (1) {
	/* First Skip Over Current Key */
	if (!num_left)
	    return num_skip;	/* Successfully skipped */

	if (_blkleaf(iptr)) {
	    num_left -= _blkskip(iptr, num_left);
	    if (!num_left)
		return num_skip;

	    while (1) {		/* Loop needed for when (sign<0) */
		rc = _blkup(iptr);
		if (rc == EMPTYLIST) {
		    return -num_skip;	/* Error */
		}
		block_ptr = iptr->block_ref;
		if (rc == LEAFREACHED) { /* on root block already */
		    block_ptr->key_on += sign;
		    if (block_ptr->key_on < 0)
			block_ptr->key_on = 0;
		    return num_skip - num_left;
		}
		/* rc == SUCCESS */
		if (sign > 0) {
		    if (block_ptr->key_on < block_ptr->num_keys) {
			num_left--;
			break;
		    }
		} else {
		    if (--block_ptr->key_on >= 0) {
			num_left++;
			break;
		    }
		}
	    }
	} else {    /* Not a leaf */
	    block_ptr = iptr->block_ref;
	    if (sign > 0)
		if (++block_ptr->key_on > block_ptr->num_keys)	/* EOF */
		    return num_skip - num_left;

	    num_left -= sign;
	    while ((rc = _blkdn(iptr, -sign)) == SUCCESS) ;
	    if (rc == FAILURE) {
		return -num_skip;	/* Error */
	    }
	}
    }
}
