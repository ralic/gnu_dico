/* $Id: iseq.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <dbs.h>

static void
keycpy(iptr, value)
    Index *iptr;
    void *value;
{
    int len;
    
    if (iptr->numsubkeys == 1) {
	switch (iptr->subkey->type) {
	case KEY_CHR_8:
	case KEY_CASECHR_8:
	case KEY_CHR_16:
	    len = strlen((char*)value);
	    if (len > iptr->key_len)
		len = iptr->key_len-1;
	    len++;
	    break;
	default:
	    len = iptr->key_len;
	}
    } else {
	len = iptr->key_len;
    }
    memcpy(iptr->seq.key, value, len);
}

int
iseq_begin(iptr, value_ptr)
    Index *iptr;
    void *value_ptr;
{
    Key *key;
    int rc;
     
    if (!iptr->seq.key) {
	iptr->seq.key = malloc(2*iptr->key_len);
	if (!iptr->seq.key) 
	    return FAILURE;
	iptr->seq.lastkey = (char*)iptr->seq.key+iptr->key_len;
    }
    rc = iseek(iptr, value_ptr);
    if (rc != SUCCESS)
	return rc;
    keycpy(iptr, value_ptr);
    key = _blkkey(iptr);
    if (!key) {
	logmsg(L_WARN, "%s:%d: _blkkey() returned NULL", __FILE__, __LINE__);
	return FAILURE;
    }
    iptr->seq.recno = key->rec_num;
    memcpy(iptr->seq.lastkey, key->value, iptr->key_len);
    return SUCCESS;
}


int
iseq_next(iptr)
    Index *iptr;
{
    int rc;
    Key *key;

    if (!iptr->seq.key)
	return FAILURE;
    if (iptr->seq.recno == -1) {
	return iseq_begin(iptr, iptr->seq.key);
    }

    /* Position the last found key */
    switch (rc = isync(iptr, iptr->seq.lastkey, iptr->seq.recno)) {
    case SUCCESS:
	break;
    default:			
	return rc;
    }
    /* Find next key */
    if (iskip(iptr, 1) != 1) {
	iptr->seq.recno = -1;
	return KEY_EOF;
    }

    key = _blkkey(iptr);
    if (!key) {
	logmsg(L_WARN, "%s:%d: _blkkey() returned NULL", __FILE__, __LINE__);
	return FAILURE;
    }
    memcpy(iptr->seq.lastkey, key->value, iptr->key_len);
    
    if (icmp(iptr, iptr->seq.key, key->value)) {
	iptr->seq.recno = -1;
	return KEY_EOF;
    }
    iptr->seq.recno = key->rec_num;
    return SUCCESS;
}

int
iseq_prev(iptr)
    Index *iptr;
{
    int rc;
    Key *key;

    if (!iptr->seq.key)
	return FAILURE;
    if (iptr->seq.recno == -1) {
	rc = iseq_begin(iptr, iptr->seq.key);
	if (rc != SUCCESS)
	    return rc;
	while (icmp(iptr, iptr->seq.key, ivalue(iptr)) == 0) {
	    iptr->seq.recno = irecno(iptr);
	    memcpy(iptr->seq.lastkey, ivalue(iptr), iptr->key_len);
	    if (iskip(iptr, 1) != 1)
		break;
	}
	return SUCCESS;
    }

    /* Position the last found key */
    switch (rc = isync(iptr, iptr->seq.lastkey, iptr->seq.recno)) {
    case SUCCESS:
	break;
    default:			
	return rc;
    }
    /* Find prev key */
    if (iskip(iptr, -1) != -1) {
	iptr->seq.recno = -1;
	return KEY_EOF;
    }

    key = _blkkey(iptr);
    if (!key) {
	logmsg(L_ERR, "%s:%d: _blkkey() returned NULL", __FILE__, __LINE__);
	return FAILURE;
    }
    memcpy(iptr->seq.lastkey, key->value, iptr->key_len);
    
    if (icmp(iptr, iptr->seq.key, key->value)) {
	iptr->seq.recno = -1;
	return KEY_EOF;
    }
    iptr->seq.recno = key->rec_num;
    return SUCCESS;
}


int 
iseq_end(iptr)
    Index *iptr;
{
    if (iptr->seq.key)
	free(iptr->seq.key);
    return 0;
}

int
iseq_matchkey(iptr, value)
    Index *iptr;
    void *value;
{
    if (!iptr->seq.key)
	return -1;
    return icmp(iptr, value, iptr->seq.key);
}
