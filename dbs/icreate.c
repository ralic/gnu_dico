/* $Id: icreate.c,v 1.1 2001/11/03 22:33:36 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dbs.h>

int idx_pageshift = 10; /* default pagesize == 1024 */
int idx_create_perm = 0644;

int check_key(int, struct subkey *);

Index *
icreate(name, flags, pageshift, keylen, numsubkeys, subkey, rf, rdata)
    char *name;
    int flags;
    int pageshift;
    int keylen;
    int numsubkeys;
    struct subkey *subkey;
    int (*rf)();
    void *rdata;
{
    Index  *iptr;
    int rc;
    int pagesize;
    
    /* some initial checks */
    rc = check_key(numsubkeys, subkey);
    if (rc <= 0 || rc > keylen) {
	logmsg(L_ERR, "can't create `%s': invalid key", name);
	return NULL;
    }
    
    if (pageshift == 0)
	pageshift = idx_pageshift;
    pagesize = 1 << pageshift;
    if (sizeof(struct indexheader) +
	sizeof(struct subkey) * numsubkeys > pagesize) {
	logmsg(L_ERR, "can't create `%s': too many subkeys (%d, page %d bytes)",
	      name, numsubkeys, pagesize);
	return NULL;
    }
	
    iptr = (Index*)alloc_block(sizeof(Index));
    if (!iptr)
	return NULL;

    if ((rc = open(name, O_CREAT|O_TRUNC|O_RDWR, idx_create_perm)) < 0) {
        logmsg(L_ERR, "can't create `%s': %m", name);
        return NULL;
    }
    iptr->fd = rc;
    iptr->idxflags = flags;
    iptr->block_last = iptr->block_first = iptr->block_ref = NULL;
    iptr->block_max = _icache;
    iptr->lock = 0;
    iptr->pageshift = pageshift;
    iptr->pagesize = pagesize;
    iptr->key_len = keylen;

    iptr->numsubkeys = numsubkeys;
    rc = sizeof(struct subkey) * numsubkeys;
    iptr->subkey = malloc(rc);
    if (!iptr->subkey) {
	free_block(iptr);
	logmsg(L_ERR, "can't alloc %d subkeys for `%s'", numsubkeys, name);
	return NULL;
    }
    memcpy(iptr->subkey, subkey, rc);

    memset(&iptr->seq, 0, sizeof(iptr->seq));
    if ((rc = reindex(iptr, rf, rdata)) < 0) {
	free(iptr->subkey);
	free_block(iptr);
	logmsg(L_ERR, "failed to build index `%s'", name);
        return NULL;
    }
    iptr->name = strdup(name);

    if (!_ifile)
	_ifile = iptr;
    else
	insert_block(_ifile, iptr, 1);
    return iptr;
}

int
check_key(num, subkey)
    int num;
    struct subkey *subkey;
{
    int i, size = 0;

    for (i = 0; i < num; i++, subkey++) {
	if (i && subkey->offset < size) {
	    logmsg(L_ERR, "check_key(): overlapping keys (%d)", i);
	    return -1;
	}
	switch (subkey->type) {
	case KEY_INT_8:
	case KEY_UINT_8:
	    subkey->len = sizeof(char);
	    break;
	case KEY_INT_16:
	case KEY_UINT_16:
	    subkey->len = sizeof(short);
	    break;
	case KEY_INT_32:
	case KEY_UINT_32:
	    subkey->len = sizeof(long);
	    break;
	case KEY_CHR_8:
	case KEY_CASECHR_8:
	case KEY_CHR_16:
	    subkey->cmplen = subkey->len;
	    break;
	default:
	    logmsg(L_ERR, "check_key(): %d has unknown keytype %d", i, subkey->type);
	    return -1;
	}
	size = subkey->offset + subkey->len;
    }
    return size;
}

int
key_length(type, len)
    int type;
    int len;
{
    switch (type) {
    case KEY_INT_8:
    case KEY_UINT_8:
	return sizeof(char);
    case KEY_INT_16:
    case KEY_UINT_16:
	return sizeof(short);
    case KEY_INT_32:
    case KEY_UINT_32:
	return sizeof(long);
    case KEY_USER:
    case KEY_CHR_8:
    case KEY_CASECHR_8:
    case KEY_CHR_16:
	return len;
    default:
	internal_error("key_length(): unknown keytype");
    }
    /*NOTREACHED*/
}
