/* $Id: ibuild.c,v 1.1 2001/11/03 22:33:36 gray Exp $
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

extern int fsort(unsigned rec_len,
		 int (*rread) (), int (*rwrite) (), int (*rcmp) ());
static int add_key(Key *), index_info(void);
static Blockno build_index(void);
static Index *index_ptr;	/* Index being processed */
static int (*read_fun)();
static void *read_data;
static int index_handle;	/* Index file handle */

static Key *key_value;		/* Key buffer */
static Blockno file_block;	/* Current index file block offset */
static int group_len;		/* Length of index group (key_len+8) */
static int key_len;		/* Length of index key */
static int key_len_4;		/* key_len + 4 */
static int keys_max;		/* Maximum # of keys in block */
static Blockno num_recs;	/* Keeps number of records indexed */
static Key *key_ptr;
Recno logical_recno;

static struct {
    Iblock *first, *last;
    Iblock *ptr;
    int level;   /* Current block level */
    int max;     /* Max. reached block level */
} block;

#define BLOCK_SIZE index_ptr->pagesize

/* These two are for unique indexes only: */
static int first_time;
static char last_ptr[MAX_KEY_SIZE + 2];

static int
compare(one, two)
    char *one;
    char *two;
{
    int i = icmp(index_ptr, one, two);
    return i ? i : (*(Blockno *)(one+key_len) - *(Blockno *)(two+key_len));
}

static int
gen_key(recno, ptr)
    Blockno recno;
    char *ptr;
/* Reads next record and generates the key for it */
{
    if (read_fun(read_data, recno, &logical_recno, ptr, key_len)) {
	return -1;
    }
    memmove(ptr + key_len, (char *) &logical_recno, sizeof(long));
    return 0;
}


static int
write_key(recno, from_ptr)
    Blockno recno;
    char *from_ptr;
/* Stores next key in the index */
{
    if (!first_time) {
	if (compare(from_ptr, last_ptr) == 0)
	    return 0;
	if (index_ptr->idxflags & IF_UNIQUE) {
	    if (icmp(index_ptr, from_ptr, last_ptr) == 0)
	       return 0;
        }	       
    }
    memmove(last_ptr, from_ptr, key_len_4);
    first_time = 0;

    num_recs++;
    memcpy(key_value->value, from_ptr, key_len);
    key_value->rec_num = *(Blockno*)(from_ptr+key_len);
    key_value->file_block = 0;
    if (add_key(key_value) < 0)
	return -1;
    return 0;
}

Blockno
build_index()
{
    num_recs = 0;
    first_time = 1;

    block.level = block.max = -1;
    block.first = block.last = block.ptr = NULL;

    file_block = BLOCK_SIZE;
    logical_recno = -1;
    
    lseek(index_handle, (Blockno) BLOCK_SIZE, SEEK_SET);
    fsort(key_len_4, gen_key, write_key, compare);

    /* Observe
     * 1.        Full Blocks will have been written if the entry which
     *           points the block has been entered.
     * 2.        Either the bottom block may be written and give a starting
     *           block pointer or the last block written is a good starting
     *           block pointer.
     * 3.        There may be one stranded entry which may be added later.
     * 4.        file_block is the next address to be written
     */

    /* Check for no Entries */
    if (block.max < 0) {
	add_key(key_value);
	block.first->num_keys = 0;
	if (write(index_handle, &block.first->num_keys, BLOCK_SIZE) != BLOCK_SIZE) {
	    logmsg(L_ERR, "build_index(): write error on `%s': %m", index_ptr->name);
	    return -1;
	}
	file_block += BLOCK_SIZE;
    } else {
	if (block.first->num_keys == 0) {
	    /* This produces the stranded entry */
	    for (block.ptr = block.first, block.level++;
		 block.ptr->num_keys == 0;
		 block.ptr = next_block(block.ptr), block.level++) ;

	    memcpy(key_value,
		   &block.ptr->num_keys +
		              block.ptr->pointers[block.ptr->num_keys - 1],
		   group_len);
	    block.ptr->num_keys--;
	    if (block.ptr->num_keys != 0) {
		block.level--;
		block.ptr = prev_block(block.ptr);
	    }
	}
    }

    while (block.level < block.max) {
	block.level++;
	if (!block.ptr)
	    block.ptr = block.first;
	else
	    block.ptr = next_block(block.ptr);

	if (block.ptr->num_keys > 0) {
	    key_ptr = (Key *) ((char *) &block.ptr->num_keys +
			       block.ptr->pointers[block.ptr->num_keys]);
	    if (block.level > 0)
		key_ptr->file_block = file_block - BLOCK_SIZE;

	    if (write(index_handle, &block.ptr->num_keys, BLOCK_SIZE) != BLOCK_SIZE) {
		logmsg(L_ERR, "write error on `%s': %m", index_ptr->name);
		return -1;
	    }
	    file_block += BLOCK_SIZE;
	}
    }
    index_ptr->root = file_block - BLOCK_SIZE;
    index_ptr->virtual_eof = index_ptr->eof = 0L;
    index_ptr->version = index_ptr->old_version + 1;
    return num_recs;
}

static int
add_key(key_ptr)
    Key *key_ptr;
{
    int i, i_block;

    block.level++;
    if (block.level > block.max) {
	block.ptr = alloc_index_block(index_ptr);
	/* Initialization for this block level */
	memset(&block.ptr->num_keys, 0, BLOCK_SIZE);
	i_block = keys_max * sizeof(short) + sizeof(long);
	for (i = 0; i <= keys_max; i++, i_block += group_len)
	    block.ptr->pointers[i] = i_block;

	block.ptr->num_keys = 0;
	if (block.last) 
	    block.last = insert_block(block.last, block.ptr, 0);
	else 
	    block.first = block.last = block.ptr;
	block.max = block.level;
    } else if (!block.ptr) {
	block.ptr = block.first;
    } else {
	block.ptr = next_block(block.ptr);
    }
    
    if (block.ptr->num_keys >= keys_max) {
	memmove((char *) &block.ptr->num_keys +
		block.ptr->pointers[block.ptr->num_keys],
		(char *) key_ptr, sizeof(Blockno));
	/* That's:
	 * *(Blockno*)((char *)&block.ptr->num_keys +
         *              block.ptr->pointers[block.ptr->num_keys]) =
	 *		key_ptr->file_block;
         */
	if (write(index_handle, &block.ptr->num_keys, BLOCK_SIZE) != BLOCK_SIZE) {
	    logmsg(L_ERR, "write error on `%s': %m", index_ptr->name);
	    return -1;
	}
	key_ptr->file_block = file_block;
	file_block += BLOCK_SIZE;
	if (add_key(key_ptr) < 0)
	    return -1;

	block.ptr->num_keys = 0;
    } else {
	memmove((char *) &block.ptr->num_keys +
		block.ptr->pointers[block.ptr->num_keys],
		(char *) key_ptr, group_len);
	block.ptr->num_keys++;
    }

    block.level--;
    block.ptr = prev_block(block.ptr);
    return 0;
}

static int
index_info()
{
    index_ptr->group_len = group_len = key_len + 8;
    index_ptr->keys_half = ((index_ptr->pagesize - sizeof(Blockno)) /
	                   (group_len + 2) - 1) / 2;
    index_ptr->keys_max = keys_max = index_ptr->keys_half * 2;
    if (keys_max < 2) {
	logmsg(L_ERR, "%s:%d: key_max", __FILE__,__LINE__);
	return -1;
    }
    index_handle = index_ptr->fd;
    key_len_4 = key_len + 4;
    return 0;
}


int
reindex(iref, rf, rdata)
    Index *iref;
    int (*rf)();
    void *rdata;
{
    Blockno lrc;
    struct indexheader *hdr;
    Index *iptr;
    Iblock *ptr, *tmp;
    
    index_ptr = iref;
    key_len = iref->key_len;
    read_fun = rf;
    read_data = rdata;
    
    if (index_info() < 0)
	return -1;

    /* Free all index blocks that have the requested size */
    for (iptr = _ifile; iptr; iptr = next_block(iptr))
	if (iptr->pagesize == iref->pagesize)
	    ifree(iptr);
    
    hdr = alloc_block(iref->pagesize);
    if (!hdr) {
	logmsg(L_ERR, "reindex(): can't alloc index header block (%d bytes)",
	      iref->pagesize);
	return -1;
    }
    /* Reserve place for the header */
    if (write(index_handle, hdr, iref->pagesize) != iref->pagesize) {
	logmsg(L_ERR, "can't write header to `%s': %m", index_ptr->name);
        return -1;
    }

    /* Build the Index File */
    key_value = malloc(sizeof(Key) + key_len - 1);
    if (!key_value) {
	logmsg(L_ERR, "can't alloc key structure");
	return -1;
    }
    /* The key_value is not yet active */
    memset(key_value, 0, sizeof(Key) + key_len - 1);
    lrc = build_index();
    /* free used blocks */
    ptr = block.first;
    while (ptr) {
	tmp = next_block(ptr);
	_free_index_block(iref->pageshift, ptr);
	ptr = tmp;
    }
    
    if (lrc < 0) {
	free(key_value);
	free_block(hdr);
	return -1;
    }
    /* Write the Index File Header (Block 0) */
    hdr->magic = INDEX_MAGIC;
    hdr->version = INDEX_VERSION;
    hdr->unused = 0;
    hdr->root = index_ptr->root;
    hdr->eof = index_ptr->eof;
    hdr->group_len = index_ptr->group_len; 
    hdr->key_len = index_ptr->key_len;  
    hdr->keys_max = index_ptr->keys_max;
    hdr->keys_half = index_ptr->keys_half;
    hdr->idxflags = index_ptr->idxflags;
    hdr->pageshift = index_ptr->pageshift;
    hdr->numsubkeys = index_ptr->numsubkeys;
    memcpy((char*)hdr+sizeof(*hdr), index_ptr->subkey,
	   index_ptr->numsubkeys * sizeof(index_ptr->subkey[0]));
    
    lseek(index_handle, 0L, 0);
    if (write(index_handle, hdr, iref->pagesize) != iref->pagesize) {
	logmsg(L_ERR, "can't write header to `%s': %m", index_ptr->name);
	return -1;
    }
    if (key_value->rec_num != 0) {
	if (iadd(index_ptr, key_value->value, key_value->rec_num) < 0)
	    return -1;
    }
    free(key_value);
    free_block(hdr);
    return 0;
}


