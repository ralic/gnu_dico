/* $Id: ilock.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <dbs.h>

int
ilock(iptr)
    Index *iptr;
{
    struct flock flock;
    
    if (iptr->lock)
	return SUCCESS;

    flock.l_type = F_WRLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 1;
    flock.l_pid = 0;
    if (fcntl(iptr->fd, F_GETLK, &flock) == -1)
	return FAILURE;
    if (flock.l_pid)
	return WOULDBLOCK;

    flock.l_type = F_WRLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 1;
    if (fcntl(iptr->fd, F_SETLK, &flock) == -1)
	return FAILURE;
    
    iptr->old_version = iptr->version;
    iptr->lock = 1;

    if (iptr->old_version != iptr->version) {
	iptr->old_version = iptr->version;
	iptr->virtual_eof = 0L;
	if (ifree(iptr) < 0)
	    return FAILURE;
    }
    return 0;
}


int
iunlock(iptr) 
    Index *iptr;
{
    ifree(iptr);
    if (iptr->lock) {
	if (iptr->version != iptr->old_version) {
	    int rc;
	    struct indexheader hdr;
	    struct flock flock;

	    iptr->old_version = iptr->version;
	    lseek(iptr->fd, 0L, SEEK_SET);

	    hdr.magic = INDEX_MAGIC;
	    hdr.version = INDEX_VERSION;
	    hdr.unused = 0;
	    hdr.pageshift = iptr->pageshift;
	    hdr.root = iptr->root;       
	    hdr.eof = iptr->eof;        
	    hdr.group_len = iptr->group_len;  
	    hdr.key_len = iptr->key_len;      
	    hdr.keys_max = iptr->keys_max;    
	    hdr.keys_half = iptr->keys_half;  
	    hdr.idxflags = iptr->idxflags;    
	    hdr.numsubkeys = iptr->numsubkeys;
	    /* NOTE: I don't write subkeys: they should be on disk already */
	    rc = write(iptr->fd, &hdr, sizeof(hdr));
	    if (rc != sizeof(hdr)) {
		logmsg(L_ERR, "can't write header to `%s': %m", iptr->name);
		return FAILURE;
	    }

	    flock.l_type = F_UNLCK;
	    flock.l_whence = SEEK_SET;
	    flock.l_start = 0;
	    flock.l_len = 1;
	    if (fcntl(iptr->fd, F_SETLK, &flock) == -1)
		return FAILURE;
	    iptr->lock = 0;
	}
    }
    return SUCCESS;
}
