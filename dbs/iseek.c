/* $Id: iseek.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <dbs.h>

int
iseek(iptr, search_string)
    Index *iptr;
    void *search_string;
{
    int rc;

    if (ilock(iptr) < 0) {
	return FAILURE;
    }
    /* Do initial search, moving up only as far as necessary */
    while (_blkup(iptr) == SUCCESS);

    /*  Repeat until the key is found */
    for (;;) {
	/* Locate in the current block */
	rc = _blksearch(iptr, search_string);
	if (rc == FAILURE) {
	    return FAILURE;
	}
	if (_blkleaf(iptr)) {
	    if (rc == KEY_EOF) {
		while (1) {
		    Iblock *ref;
		    
		    if (_blkup(iptr) != SUCCESS)
			return KEY_EOF;

		    ref = iptr->block_ref;

		    if (ref->key_on < ref->num_keys) {
			if (icmp(iptr, search_string,
				 _blkkey(iptr)->value) == 0) {
			    return SUCCESS;
			}
			return KEY_GREATER;
		    }
		}
	    }
	    return rc;		/* _blksearch has the same "rc" conventions */
	} else {
	    /* We should be able to move down as we are not on a leaf */
	    if (_blkdn(iptr, -1) != SUCCESS) {
		return FAILURE;
	    }
	}
    }
}
