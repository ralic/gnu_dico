/* $Id: iopen.c,v 1.1 2001/11/03 22:33:31 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <dbs.h>

Index  *_ifile;         /* Index file control structures */
Iblock *_iblocks;       /* Index file blocks */
int    _icache = 48;    /* Max # of allocated index blocks */
Index  *_ifree;         /* Reference to Index whose blocks are currently
                         * being freed. This is used to allocate more
                         * blocks for another index without expanding
                         * _iblocks array
                         */
int _blkcount[MAX_PAGE_SHIFT-MIN_PAGE_SHIFT+1] ;

Index *
iopen(name, rw)
    char *name;
    int rw;
{
    int rc;
    Index   *iptr;
    int mode;
    struct indexheader hdr;

    iptr = (Index*)alloc_block(sizeof(Index));
    if (!iptr)
	return NULL;

    mode = rw ? O_RDWR : O_RDONLY;
    
    if ((rc = open(name, mode)) < 0) {
	if (errno == EACCES && rw && (rc = open(name, O_RDONLY)) >= 0) {
	    logmsg(L_WARN, "write access denied on `%s'. File opened read-only",
		    name);
	} else {
	    logmsg(L_ERR, "cannot open `%s': %m", name);
	    free_block(iptr);
	    return NULL;
	}
    }
    iptr->fd = rc;
    lseek(iptr->fd, (long) 0, SEEK_SET);

    /*  Try to accsess the file */
    rc = read(iptr->fd, &hdr, sizeof(hdr));
    if (rc != sizeof(hdr)) {
	logmsg(L_ERR, "can't read header from `%s': %m", name);
	close(iptr->fd);
	free_block(iptr);
	return NULL;
    }
       /* Do some checks on the index file header */
    if (hdr.magic != INDEX_MAGIC || hdr.version != INDEX_VERSION ||
	hdr.key_len > MAX_KEY_SIZE || hdr.key_len <= 0 ||
	hdr.keys_max != 2 * hdr.keys_half ||
	hdr.keys_half <= 0 ||
	hdr.group_len != hdr.key_len+8) {
	     logmsg(L_ERR, "`%s' has invalid index header", name);
	     close(iptr->fd);
	     free_block(iptr);
	     return NULL;
    }
    /* Initialize index ctl structure */
    iptr->pageshift = hdr.pageshift;
    iptr->pagesize = 1 << hdr.pageshift;
    iptr->root = hdr.root;       
    iptr->eof = hdr.eof;        
    iptr->group_len = hdr.group_len;  
    iptr->key_len = hdr.key_len;      
    iptr->keys_max = hdr.keys_max;    
    iptr->keys_half = hdr.keys_half;  
    iptr->idxflags = hdr.idxflags;    
    iptr->block_last = iptr->block_first = iptr->block_ref = NULL;
    iptr->block_max = _icache;
    iptr->lock = 0;
    memset(&iptr->seq, 0, sizeof(iptr->seq));

    /* Allocate and read in subkey descriptors */
    iptr->numsubkeys = hdr.numsubkeys;
    rc = hdr.numsubkeys * sizeof(struct subkey);
    iptr->subkey = calloc(hdr.numsubkeys, sizeof(struct subkey));
    if (!iptr->subkey) {
	logmsg(L_ERR, "can't alloc %d subkeys for `%s'", hdr.numsubkeys, name);
	close(iptr->fd);
	free_block(iptr);
	return NULL;
    }
	
    if (rc != read(iptr->fd, iptr->subkey, rc)) {
	logmsg(L_ERR, "can't read all subkey descriptors from `%s'", name);
	close(iptr->fd);
	free_block(iptr);
	return NULL;
    }
    rc = check_key(iptr->numsubkeys, iptr->subkey);
    if (rc <= 0 || rc > iptr->key_len) {
	logmsg(L_ERR, "can't use `%s': invalid key (key_len = %d, rc = %d)",
	      name, iptr->key_len, rc);
	close(iptr->fd);
	free(iptr->subkey);
	free_block(iptr);
	return NULL;
    }
	
    iptr->name = strdup(name);
    
    if (!_ifile)
	_ifile = iptr;
    else
	insert_block(_ifile, iptr, 1);
    return iptr;
}


