/* $Id: iadd.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <unistd.h>
#include <dbs.h>

/*
 *         0 -  Successful Add
 *         1 -  Index file has flagged for unique keys only and
 *              the key's value is a duplicate.
 *         2 -  The record was located in the index file.
 */
int
iadd(iptr, value_ptr, rec_num)
    Index *iptr;
    void *value_ptr;
    Recno rec_num;
{
    char key_data[MAX_KEY_SIZE + 8];
    Key *key_ptr;
    int rc;

    /* Position the correct key */
    switch (rc = isync(iptr, value_ptr, rec_num)) {
    case SUCCESS:
	return IADD_CLASH;
    case ISYNC_KEYLOCATED:
	if (iptr->idxflags & IF_UNIQUE)
	    return IADD_DUP;
	break;
    case ISYNC_NOTFOUND: /* a normal case */
	break;
    case ISYNC_EOF_KEYLOCATED:
	if (iptr->idxflags & IF_UNIQUE)
	    return IADD_DUP;
    case ISYNC_EOF:
	if (!_blkleaf(iptr) && ibottom(iptr) != SUCCESS)
	    return IADD_FAILURE;
	iptr->block_ref->key_on = iptr->block_ref->num_keys;
	break;
    default:			/* rc < 0 */
	return IADD_FAILURE;
    }

    if (!_blkleaf(iptr)) {
	if (iskip(iptr, -1L) != -1L)
	    return IADD_FAILURE;
	iptr->block_ref->key_on = iptr->block_ref->num_keys;
    }
    iptr->version = iptr->old_version + 1;

    /* Prepare 'key' for the call to '_addkey'. */
    key_ptr = (Key *) key_data;
    key_ptr->file_block = 0;
    key_ptr->rec_num = rec_num;
    memcpy(key_ptr->value, value_ptr, iptr->key_len);

    if (_addkey(iptr, key_ptr))
	return IADD_FAILURE;
    return SUCCESS;
}
