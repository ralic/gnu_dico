/* $Id: iblk.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dbs.h>

static void i_insert(Index *iptr, Key *key_ptr, int position);
#define checklist(a)
#ifdef __DEBUG
unsigned long block_misses;
unsigned long block_hits;
#endif

Iblock *
alloc_index_block(iptr)
    Index *iptr;
{
    Iblock *p;

    p = alloc_block(BLOCK_MEMSIZE(iptr));
    if (p)
	_blkcount[iptr->pageshift-MIN_PAGE_SHIFT]++;
    return p;
}

Iblock *
free_index_block(iptr, bp)
    Index *iptr;
    Iblock *bp;
{
    _blkcount[iptr->pageshift-MIN_PAGE_SHIFT]--;
    return free_block(bp);
}

Iblock *
_free_index_block(shift, bp)
    int shift;
    Iblock *bp;
{
    _blkcount[shift-MIN_PAGE_SHIFT]--;
    return free_block(bp);
}

int
ifree(iptr)
    Index *iptr;
{
    Iblock *bp, *tmp;
    
    for (bp = iptr->block_ref; bp; bp = tmp) {
	if (is_block_dirty(bp))
	    _blkwrite(iptr, bp);
	tmp = prev_block(bp);
	free_index_block(iptr, bp);
    }

    for (bp = iptr->block_first; bp; bp = tmp) {
	if (is_block_dirty(bp))
	    _blkwrite(iptr, bp);
	tmp = next_block(bp);
	free_index_block(iptr, bp);
    }
    iptr->block_num = 0;
    iptr->block_first = iptr->block_last = iptr->block_ref = NULL;
    return 0;
}

/* _blkwrite(): Writes the current block memory unit of the specified index
 *             file.
 */
int
_blkwrite(iptr, bp)
    Index *iptr;
    Iblock *bp;
{
    set_block_dirty(bp, 0);

    lseek(iptr->fd, (long) get_block_number(bp), SEEK_SET);
    if (write(iptr->fd, &bp->num_keys, iptr->pagesize) != iptr->pagesize) {
        logmsg(L_ERR, "_blkwrite(): write error on `%s': %m", iptr->name);
        return -1;
    }
    return 0;
}

/* _blkget(): Allocate one more block
 */
Iblock *
_blkget(iptr)
    Index *iptr;
{
    Iblock *bp;
    
    if (_blkcount[iptr->pageshift-MIN_PAGE_SHIFT] >= _icache) {
	/* Find an index that uses some blocks and free them */
	if (!_ifree)
	    _ifree = iptr;

	if (_ifree->pageshift != iptr->pageshift || _ifree->block_num <= 0) {
	    Index *iref;
	    
	    for (iref = next_block(_ifree);
		 iref &&
		 (iref->pageshift!=iptr->pageshift
                		 || (iref->block_num <= 0  && iref != _ifree));
		 iref = next_block(iref));
	    if (!iref)
		goto alloc_anyway;
	    _ifree = iref;
	}

	if (_ifree->block_num <= 0) {
	    logmsg(L_ERR, "%s:%d: can't find iblocks to free", 
	    	__FILE__,__LINE__);
	    return NULL;
	}

	bp = _ifree->block_first;
	if (is_block_dirty(bp))
	    _blkwrite(_ifree, bp);
	_ifree->block_first = free_index_block(iptr, bp);
	if (_ifree->block_first == NULL)
	    _ifree->block_last = NULL;
	_ifree->block_num--;
	if (_ifree->block_num < _ifree->block_max) 
	    _ifree = next_block(_ifree);
    }
 alloc_anyway:
    checklist(iptr);
    bp = alloc_index_block(iptr);
    if (iptr->block_ref == NULL)
	iptr->block_ref = bp;
    else
	iptr->block_ref = insert_block(iptr->block_ref, bp, 0);
    checklist(iptr);
    return iptr->block_ref;
}


/* i_insert(): Insert 'key_ptr' into current index block at 'position'
 */
static void
i_insert(iptr, key_ptr, position)
    Index *iptr;
    Key *key_ptr;
    int position;
{
    Iblock *block_ptr;
    int free_slot;

    block_ptr = iptr->block_ref;

    free_slot =  block_ptr->pointers[block_ptr->num_keys+1];
    if ( free_slot + iptr->group_len >= iptr->pagesize) {
        internal_error("bad index block");
    }

    memmove(block_ptr->pointers + position + 1,
	    block_ptr->pointers + position,
	    (int) sizeof(block_ptr->pointers[0]) *
	              (block_ptr->num_keys + 1 - position) );
    block_ptr->pointers[position] = free_slot;

    memmove((char *)&block_ptr->num_keys + free_slot, (char *) key_ptr,
	    iptr->group_len);
    block_ptr->num_keys++;
    set_block_dirty(block_ptr, 1);
}

/*  _addkey(): Add the 'key' to the current index block.  Position of
 *  'key_on' is assumed to point to the key in the block just
 *   greater than the key being added.
 */
int
_addkey(iptr, key_ptr)
    Index *iptr;
    Key *key_ptr;
{
    Iblock *block_ptr, *low_block_ptr;
    Key *save_key_ptr, *half_key_ptr;
    char save_key_data[MAX_KEY_SIZE+8];
    int i, swap_val, rc, i_half_key, insert_position, key_psn;
    short *from, *to;
    Blockno save_file_block;

    block_ptr = iptr->block_ref;
    if ( block_ptr->num_keys < iptr->keys_max ) {
	/* There is still space in the block: just insert */
        i_insert(iptr, key_ptr, block_ptr->key_on);
        return 0;
    }

    /* The block is full */
    
    save_key_ptr = (Key*) save_key_data;

    /* Create the Middle Key, Fill in the key to be added */
    if ( block_ptr->key_on == iptr->keys_half ) {
        memcpy( save_key_ptr, key_ptr, iptr->group_len );
    } else {
        insert_position =  block_ptr->key_on;
        i_half_key =  iptr->keys_half;

        if ( block_ptr->key_on < iptr->keys_half )
            i_half_key--;
        else
            insert_position--;
	
        memcpy(save_key_ptr,
	       ((char *)&block_ptr->num_keys) +
	                   block_ptr->pointers[i_half_key],
	       iptr->group_len);
	/* Remove the Half Way Key */
        block_ptr->key_on =  i_half_key;
        _removekey(iptr);

        i_insert(iptr, key_ptr, insert_position);
    }

    /* Split the block */
    if ( _blkget(iptr) == NULL )
        return -1;
    low_block_ptr = iptr->block_ref;
    set_block_dirty(low_block_ptr, 1);

    /* 'block_ptr' may not be accurate because of '_blkget' call. ???*/
    block_ptr = prev_block(low_block_ptr);
    set_block_dirty(block_ptr, 1);

    /* Copy old block data to new block */
    memcpy((char *)low_block_ptr, (char *)block_ptr, BLOCK_MEMSIZE(iptr));
    low_block_ptr->num_keys = iptr->keys_half;

    /* Add to the end of the file */
    if ( !iptr->eof ) {
	/* Add to the End */
        if (iptr->virtual_eof == 0L)
            iptr->virtual_eof = lseek(iptr->fd, 0L, SEEK_END);

        set_block_number(low_block_ptr, iptr->virtual_eof);
        iptr->virtual_eof += iptr->pagesize;
    } else {
	set_block_number(low_block_ptr, iptr->eof);

	/* Find new EOF */
        lseek(iptr->fd, iptr->eof, SEEK_SET);
        rc = read(iptr->fd, (char *)&iptr->eof, sizeof(iptr->eof) );
        if ( rc < 0 ) {
            logmsg(L_ERR, "%s:%d: read error: %m", __FILE__, __LINE__);
            return -1;
        }
    }

    save_file_block = save_key_ptr->file_block;
    save_key_ptr->file_block = get_block_number(low_block_ptr);

    low_block_ptr->key_on = iptr->keys_half;
    half_key_ptr = _blkkey(iptr);
    half_key_ptr->file_block = save_file_block;

    if (_blkup(iptr) != SUCCESS) {
        return -1;
    }

    /* Modify 'block_ptr' (with the 'high' keys) */
    if ( block_ptr->num_keys > iptr->keys_max ) {
        logmsg(L_ERR, "_addkey(): bad number of keys in the index file block (file %s)",
	      iptr->name);
        return -1;
    }

    to = block_ptr->pointers;
    from = to + iptr->keys_half;
    for (i = iptr->keys_half; i <= block_ptr->num_keys; i++) {
        swap_val=  *to;
        *to++   =  *from;
        *from++ =   swap_val;
    }

    block_ptr->num_keys = iptr->keys_half;

    switch (_blkup(iptr)) {
    default:
	/* _blkup() worked ok */
	/* Add the reference to the new block to the block just up */
	return _addkey(iptr, save_key_ptr);
    case LEAFREACHED:
	/*  We are on the root block and a recursive call will not work.
	 *  Do the following:
	 *     1.  Create a new root block
	 *     2.  Add block pointers to the new root block pointing to both
	 *         the new block and the old block.
	 */
	save_file_block =  get_block_number(block_ptr);

	/* Get a new root block, flip the positions, and free the previous
	 * root block
	 */
	block_ptr = _blkget(iptr);
	iptr->block_ref = remove_block(block_ptr);
	insert_block(iptr->block_ref, block_ptr, 1);
	if (_blkup(iptr) != SUCCESS) {
	    return -1;
	}

	key_psn = (iptr->keys_max+1)*sizeof(block_ptr->pointers[0]) +
	          sizeof(block_ptr->num_keys);
	for (i = 0; i <= iptr->keys_max; i++, key_psn += iptr->group_len)
	    block_ptr->pointers[i] =  key_psn;

	block_ptr->num_keys = 0;

	/* Specify New Root; Increment EOF value */
	if ( iptr->eof == 0L ) {
	    if (iptr->virtual_eof == 0L)
		iptr->virtual_eof = lseek(iptr->fd, 0L, SEEK_END);

	    /* Add to the End */
	    iptr->root = iptr->virtual_eof;
	    set_block_number(block_ptr, iptr->root);
	    iptr->virtual_eof += iptr->pagesize;
	} else {
	    iptr->root = iptr->eof;
	    set_block_number(block_ptr, iptr->root);
	    /* Find new EOF */
	    lseek(iptr->fd, iptr->eof, 0);
	    rc = read( iptr->fd, (char *) &iptr->eof, sizeof(iptr->eof));
	    if ( rc < 0 ) {
		logmsg(L_ERR, "%s:%d: read error: %m", __FILE__, __LINE__);
		return -1;
	    }
	}
	i_insert(iptr, save_key_ptr, 0);
	save_key_ptr->file_block = save_file_block;
	i_insert(iptr, save_key_ptr, 1);
	block_ptr->num_keys = 1;
	set_block_dirty(block_ptr, 1);
	break;
    case EMPTYFILE:
    case EMPTYLIST:
    case FAILURE:
	return -1;
    }
	    
    return 0;
}


/* _blkleaf(): Returns TRUE if the current block is a leaf in the B+ tree
 */
int
_blkleaf(iptr)
    Index *iptr;
{
    Iblock  *block_ptr;
    Key *key_ptr;
    
    block_ptr = iptr->block_ref;
    key_ptr = (Key*)(((char *)&block_ptr->num_keys) + block_ptr->pointers[0]);
    return key_ptr->file_block == 0L;
}

/* _blkdn(): Moves down the B+ tree, reading in the block if necessary.
 *  Returns: Pointer to obtained block. If it's NULL, the location
 *           pointed to by `reason' parameter gets filled with the
 *           reason of it.
 */
int
_blkdn(iptr, direction)
    Index *iptr;
    int direction;
{
    Iblock *block_ptr ;
    Blockno file_block;
    Iblock *block_new;
    int from_disk=1;
    Key *key_ptr;

    if (iptr->block_ref == NULL) {
        file_block = iptr->root;     /* read the root block */
    } else {
        key_ptr = _blkkey(iptr);
        if ( key_ptr == (Key *) 0 ) 
	    return EMPTYFILE;

        file_block = key_ptr->file_block;
        if ( file_block <= 0 ) {
	    /* We are on a leaf and we cannot go down further! */
	    return LEAFREACHED;
	}
    }

    /* Search for the block in memory */
    for (block_ptr=iptr->block_last; block_ptr;
	                                   block_ptr=prev_block(block_ptr)) {
        if ( get_block_number(block_ptr) == file_block ) {
	    /* Found, remove from one chain and add to another */
	    checklist(iptr);
            block_new = remove_block(block_ptr);
            iptr->block_num--;
            if ( block_ptr == iptr->block_last )
                iptr->block_last = block_new;
            if ( iptr->block_first == block_ptr )
                iptr->block_first =  block_new;
	    checklist(iptr);
            iptr->block_ref = insert_block(iptr->block_ref, block_ptr, 0 );
	    checklist(iptr);
            from_disk = 0;
            block_ptr = iptr->block_ref;
#ifdef __DEBUG
            block_hits++;
#endif            
            break;
        }
    }

    if ( from_disk ) {
	/* Was not located, allocate memory for the new block */
        if ( _blkget(iptr) == NULL ) 
	    return FAILURE;

        block_ptr = iptr->block_ref;

        lseek(iptr->fd, file_block, SEEK_SET);
        if (read(iptr->fd, (char *) &block_ptr->num_keys, iptr->pagesize)
            != iptr->pagesize) {
            logmsg(L_ERR, "%s:%d: read error on `%s': %m", __FILE__,__LINE__,iptr->name);
	    return FAILURE;
        }
        set_block_number(block_ptr, file_block);
#ifdef __DEBUG
        block_misses++;
#endif        
    }

    if (direction < 0) {
        block_ptr->key_on = 0;
    } else  {
        block_ptr->key_on = block_ptr->num_keys;
        if ( _blkleaf(iptr) )
            block_ptr->key_on--;  /* On a leaf block in the tree */
    }

    return SUCCESS;
}

/* _blkup():  Moves up the B+ tree.
 */
int
_blkup(iptr)
    Index *iptr;
{
    Iblock *block_ref;
    
    if (iptr->block_ref == NULL) 
	return EMPTYLIST;

    if (!prev_block(iptr->block_ref)) 
	return LEAFREACHED; /* means root is reached */

    block_ref = iptr->block_ref;
    checklist(iptr);
    iptr->block_ref = remove_block(block_ref);
    if (!iptr->block_last)
	iptr->block_last = block_ref;
    else
	iptr->block_last = insert_block(iptr->block_last, block_ref, 0);
    if (!iptr->block_first)
        iptr->block_first = iptr->block_last;
    iptr->block_num++;
    checklist(iptr);
    
    return SUCCESS;
}

/*  _blkkey(): Returns a (Key *) pointer
*/

Key *
_blkkey(iptr)
    Index *iptr;
{
    Iblock *block_ptr;
    char *ptr;

    if (iptr->block_ref) {
        block_ptr = iptr->block_ref;

        if (block_ptr->key_on>=0 && block_ptr->key_on<=block_ptr->num_keys) {
            ptr = ((char *) &block_ptr->num_keys) +
		              block_ptr->pointers[block_ptr->key_on];
            return (Key *) ptr;
        }
    }
    return (Key *) 0;
}

int
_removeblk(iptr)
    Index *iptr;
{
    Iblock *block_ptr;

    block_ptr = iptr->block_ref;

    *(Blockno*) &block_ptr->num_keys = iptr->eof;
    iptr->eof = get_block_number(block_ptr);

    if (_blkwrite(iptr, iptr->block_ref) < 0)
        return -1;
    iptr->block_ref = free_index_block(iptr, iptr->block_ref);
    return 0;
}

int
_removekey(iptr)
    Index *iptr;
{
    int  num_copy, old_ptr;
    short *pointers, *from, *to;
    Iblock *block_ptr;

    if (!iptr->block_ref)
        return -1;

    block_ptr = iptr->block_ref;
    block_ptr->num_keys--;
    set_block_dirty(block_ptr, 1);

    if ( block_ptr->key_on < 0 || block_ptr->key_on > iptr->keys_max ||
        block_ptr->num_keys < 0 ) {
        logmsg(L_ERR, "%s:%d: bad iblock", __FILE__,__LINE__);
        return  -1;
    }

    pointers = block_ptr->pointers;
    old_ptr  = pointers[block_ptr->key_on];
    num_copy = iptr->keys_max - block_ptr->key_on;
    to = pointers + block_ptr->key_on;
    from =  to + 1;
    memmove(to, from, num_copy*sizeof(pointers[0]));
    pointers[iptr->keys_max] = old_ptr;

    if ( ((Key *)((char *)&block_ptr->num_keys + old_ptr))->file_block == 0 )
        return block_ptr->num_keys;
    else
        return block_ptr->num_keys + 1;
}

/* _blksearch():  Locate a key in the current block using a binary
 *                search.
 *  Returns:      Sets  key_on to the # of key located.
 *                FAILURE      -  Something went wrong!
 *                SUCCESS      -  Key found (check IF_INEXACT flag if
 *                                       IF_ALLOWINEXACT is set)
 *                KEY_GREATER  -  After the specified key
 *                KEY_EOF      -  After the Block
 */
int
_blksearch(iptr, search_string)
    Index *iptr;
    void *search_string;
{
    Iblock *block_ptr;
    Key *key_ptr;
    int key_len, key_lower, key_upper, key_on, rc;

    /* Make sure a block exists !! */
    if (!iptr->block_ref) {
	/* Read the Top Block */
        if (_blkdn(iptr, -1) != SUCCESS)
	    return FAILURE;
    }
    block_ptr = iptr->block_ref;

   /* Prepare for the search */
    key_len = iptr->key_len;

    /* key_on must be between  key_lower and key_upper */
    key_lower = -1;
    key_upper = block_ptr->num_keys;

    if (key_upper == 0) {
        block_ptr->key_on = block_ptr->num_keys;
        return KEY_EOF;           /* Key after the block */
    }

    /*  Repeat until the key is found in the block */
    for ( ; ; ) {
        key_on  = (key_upper + key_lower) / 2;
        key_ptr = (Key*)(((char *)&block_ptr->num_keys)
			         + block_ptr->pointers[key_on] );
        rc = icmp(iptr, search_string, key_ptr->value); 
        if ( rc == 0 ) {
            if ( key_on <= key_lower+1 ) {
                block_ptr->key_on = key_on;
		return SUCCESS;  /* Match Found */
            }

	    /* Perhaps there is Another Match (we want the first match) */
            key_upper = key_on + 1;
            continue;
        } else if ( rc < 0 )
	    key_upper =  key_on;
	else
	    key_lower =  key_on;
        
        if ( key_lower >= key_upper-1 )  {
	    /*  There is no Exact Match */
            if ( key_lower >=  block_ptr->num_keys - 1 ) {
                block_ptr->key_on = block_ptr->num_keys;
                return KEY_EOF;     /* Key after the block */
            }

	    /* Located inside the block */
            block_ptr->key_on = key_upper;
            return KEY_GREATER;              /* Found nearest greater */
        }
    }
    /*NOTREACHED*/
}

/* _blkskip(): Skips keys in the  current index block
 */

Recno
_blkskip(iptr, recno)
    Index *iptr;
    Recno recno;
{
    Iblock *block_ptr;
    Recno num_left;

    if (!iptr->block_ref)
        return -recno;

    block_ptr = iptr->block_ref;
    if ( recno > 0 ) {
        num_left = (Recno) (block_ptr->num_keys - block_ptr->key_on);
        if ( _blkleaf(iptr) && num_left != 0 )
            num_left--;
    } else
        num_left = (Recno) -block_ptr->key_on;

    if ( (recno <= 0 ) ? (num_left <= recno) : (num_left >= recno) ) {
        block_ptr->key_on += (int) recno;
        return recno;
    } else {
        block_ptr->key_on += (int) num_left;
        return num_left;
    }
    /*NOTREACHED*/
}

#ifndef checklist
int
checklist(iptr)
    Index *iptr;
{
    Iblock *bp, *ptr;
    int cnt;

    if (iptr->block_first && prev_block(iptr->block_first))
	internal_error("checklist(): 1");
    if (iptr->block_last && next_block(iptr->block_last))
	internal_error("checklist(): 2");
    if (iptr->block_ref && next_block(iptr->block_ref))
	internal_error("checklist(): 3");
    
    for (bp = iptr->block_first, cnt=0; bp; bp = next_block(bp), cnt++) ;
    if (cnt != iptr->block_num)
	internal_error("checklist(): 4");

    for (bp = iptr->block_last, cnt=0; bp; bp = prev_block(bp), cnt++) ;
    if (cnt != iptr->block_num)
	internal_error("checklist(): 5");

    for (bp = iptr->block_ref, cnt=0; bp; bp = prev_block(bp), cnt++) {
	if (cnt > _icache)
	    internal_error("checklist(): 6");
	for (ptr = iptr->block_first; ptr; ptr = next_block(ptr))
	    if (bp == ptr)
		internal_error("checklist(): short circuit");
    }
    
}    
#endif
