/* $Id: iclose.c,v 1.1 2001/11/03 22:33:31 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <dbs.h>

int
iclose(iptr)
    Index *iptr;
{
    if (!iptr)
	return SUCCESS;
    if (ifree(iptr))
	return FAILURE;
    if (iunlock(iptr))	
	return FAILURE;
    iseq_end(iptr);
    
    if (close(iptr->fd) < 0) {
	logmsg(L_ERR, "close error on `%s': %m", iptr->name);
	return FAILURE;
    }
    free(iptr->name);

    free_block(iptr);
    return SUCCESS;
}
