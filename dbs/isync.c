/* $Id: isync.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <dbs.h>

/* isync(): Index file resynchronization
 * Parameters:
 *       iptr          Index file.
 *       value_ptr     A pointer to the key value.
 *       rec_num       A record number to search for.
 * Function Return
 *     FAILURE               Error
 *     SUCCESS               Successfully Located
 *     ISYNC_KEYLOCATED        Key Value Located; Record Number not Located.
 *                           (Positioned at key after 'value_ptr')
 *     ISYNC_NOTFOUND          Neither Key Value nor Record Number Located
 *                           (Positioned at key after 'value_ptr')
 *     ISYNC_EOF               EOF  (Just at End)
 */

int
isync(iptr, value_ptr, rec_num)
    Index *iptr;
    void *value_ptr;
    Recno rec_num;
{
    int rc;
    int save_flags = iptr->idxflags;

    clearflag(iptr->idxflags, IF_ALLOWINEXACT);
    switch (iseek(iptr, value_ptr)) {
    case SUCCESS: /* Found */
	/* Go until either record number located or on after */
	do {
	    /* Was the record located ? */
	    if (_blkkey(iptr)->rec_num == rec_num) {
	        iptr->idxflags = save_flags;
		return SUCCESS;
            }
	    if ((rc = (int) iskip(iptr, 1L)) <= 0)
		break;
	} while (0 == icmp(iptr, value_ptr, _blkkey(iptr)->value));

	iptr->idxflags = save_flags;
	if (rc == -1) {
	    return FAILURE;
	}
	if (rc == 0)
	    return ISYNC_EOF_KEYLOCATED;	
	return ISYNC_KEYLOCATED;		/* Key Located, Record Not */

    case KEY_GREATER:			/* On After */
	iptr->idxflags = save_flags;
	return ISYNC_NOTFOUND;

    case KEY_EOF:			/* EOF */
	iptr->idxflags = save_flags;
	return ISYNC_EOF;
    }
    iptr->idxflags = save_flags;
    return FAILURE;
}


