/* $Id: mem.c,v 1.1 2001/11/03 22:33:53 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dbs.h>
#ifdef __DEBUG
 #include <stdio.h>
#endif

#ifndef MAXGROUPS
#define MAXGROUPS 10
#endif
#ifndef BUCKETSIZE
#define BUCKETSIZE 4096
#endif

typedef struct {                                                  
    size_t esize; /* entry size */
    int ecnt;     /* number of entries */
    int numbuckets;
    struct bucket_t *first;
} Group;

#define  BLOCKP(p,n) \
 ((Block)(p)+DATASTART+groups[(p)->group].esize*(n))

Group groups[MAXGROUPS];
int numgroups;

int
mem_group(esize)
    size_t esize;
{
    int group_no, cnt;

    esize = BLOCKSIZE(esize);
    for (group_no = 0; group_no < numgroups; group_no++)
	if (groups[group_no].esize == esize)
	    return group_no;

    assert(numgroups<MAXGROUPS-1);
    group_no = numgroups++;

    cnt = (HEADERS_PER_BUCKET-DATASTART) / esize;
    assert(cnt >= MIN_BLOCKS_PER_BUCKET);
    
    groups[group_no].esize = esize;
    groups[group_no].ecnt  = cnt;
    groups[group_no].numbuckets = 0;
    return group_no;
}

void
insert_new_bucket(bucket)
    Bucket *bucket;
{
    bucket->next = groups[bucket->group].first;
    bucket->prev = NULL;
    if (groups[bucket->group].first)
	groups[bucket->group].first->prev = bucket;
    groups[bucket->group].first = bucket;
    groups[bucket->group].numbuckets++;
}

void
unlink_bucket(bucket)
    Bucket *bucket;
{
    if (bucket->next)
	bucket->next->prev = bucket->prev;
    if (bucket->prev)
	bucket->prev->next = bucket->next;
}

void
promote_bucket(bucket)
    Bucket *bucket;
{
    unlink_bucket(bucket);
    bucket->next = groups[bucket->group].first;
    bucket->prev = NULL;
    if (groups[bucket->group].first)
	groups[bucket->group].first->prev = bucket;
    groups[bucket->group].first = bucket;
}

Bucket *
allocbucket(group)
    int group;
{
    int i;
    Bucket *newbucket;
    Block prevp;
    Block blk;

    newbucket = malloc(BUCKETSIZE);
    if (!newbucket)
	return NULL;
    memset(newbucket, 0, BUCKETSIZE);
    newbucket->group = group;
    newbucket->cnt = groups[group].ecnt;
    newbucket->used = 0;
    insert_new_bucket(newbucket);
    blk = newbucket->avail = BLOCKP(newbucket,0);
    blk->s.prev = NULL;
    blk->s.bucket = newbucket;
    for (i = 1, prevp = blk, blk += groups[group].esize;
	 i < groups[group].ecnt;
	 i++, prevp = blk, blk += groups[group].esize) {
	prevp->s.next = blk;
	blk->s.prev = prevp;
	blk->s.bucket = newbucket;
    }
    return newbucket;
}

Block
alloc_block_from_bucket(group)
    int group;
{
    Block blk;
    Bucket *bucket;

    for (bucket = groups[group].first; bucket; bucket = bucket->next) {
	if (!bucket->avail)
	    continue;
	blk = bucket->avail;
	bucket->avail = blk->s.next;
	bucket->used++;
	if (bucket != groups[group].first) 
	    promote_bucket(bucket);

	blk->s.blockno = -1;
	blk->s.size = 0;
	blk->s.dirty = 0;
	blk->s.next = blk->s.prev = NULL;
	return blk;
    }
    return NULL;
}

void *
free_block(ptr)
    void * ptr;
{
    Bucket *bucket;
    void *ret;
    Block blk = (Block)ptr-1;

    ret = remove_block(ptr);
    bucket = blk->s.bucket;
    if (blk >= BLOCKP(bucket,0)	&& blk <= BLOCKP(bucket,bucket->cnt-1)) {
	blk->s.prev = NULL;
	blk->s.next = bucket->avail;
	bucket->avail = blk;
	bucket->used--;
    } else
	internal_error("free_block(): attempt to free alien memory ptr");
    return ret;
}

void *
alloc_block(size)
    size_t size;
{
    Block blk;
    int group = mem_group(size);

    if (!groups[group].first &&
	(groups[group].first = allocbucket(group)) == NULL) {
	logmsg(L_ERR, "alloc_block(): can't alloc root bucket");
	return NULL;
    }
    
    blk = alloc_block_from_bucket(group);
    if (!blk) {
	if (!allocbucket(group)
	    || (blk = alloc_block_from_bucket(group)) == NULL) {
	    logmsg(L_ERR, "not enough core");
	    return NULL;
	} 
    }
    memset((void*)(blk+1), 0, size);
    return (void*)(blk+1);
}    

/* Upper-level interface functions
 */

void *
insert_block(base, mem, before)
    void * base;
    void * mem;
    int before;
{
    Block prev_blk = (Block) base - 1;
    Block mem_blk = (Block) mem - 1;

    if (before) {
        insert_block(base, mem, 0);
	remove_block(base);
        return insert_block(mem, base, 0);
    }
    /* add mem after base */
    if (prev_blk->s.next)
	prev_blk->s.next->s.prev = mem_blk;
    mem_blk->s.next = prev_blk->s.next;
    prev_blk->s.next = mem_blk;
    mem_blk->s.prev = prev_blk;
    return mem;
}

void *
remove_block(mem)
    void * mem;
{
    Block mem_blk = (Block) mem - 1;
    Block ret_blk, next;

    if (next = mem_blk->s.next) {
	ret_blk = mem_blk->s.next;
	mem_blk->s.next->s.prev = mem_blk->s.prev;
	mem_blk->s.next = NULL;
    } else
	ret_blk = NULL;
    if (mem_blk->s.prev) {
	ret_blk = mem_blk->s.prev;
	mem_blk->s.prev->s.next = next;
	mem_blk->s.prev = NULL;
    }
    return ret_blk ? (void*)(ret_blk+1) : NULL;
}

void
swap_blocks(mem_a, mem_b)
    void * mem_a;
    void * mem_b;
{
    Block blk_a = (Block) mem_a - 1;
    Block blk_b = (Block) mem_b - 1;
    Block temp;
    
    if (blk_a->s.prev)
	blk_a->s.prev->s.next = blk_b;
    if (blk_a->s.next)
	blk_a->s.next->s.prev = blk_b;
    if (blk_b->s.prev)
	blk_b->s.prev->s.next = blk_a;
    if (blk_b->s.next)
	blk_b->s.next->s.prev = blk_a;

    temp = blk_a->s.next;
    blk_a->s.next = blk_b->s.next;
    blk_b->s.next = temp;

    temp = blk_a->s.prev;
    blk_a->s.prev = blk_b->s.prev;
    blk_b->s.prev = temp;
}

#ifndef INLINE_BLOCK_FUNCTIONS
long
get_block_number(void *ptr)
{
    return __GET_BLOCK_NUMBER(ptr);
}

size_t
get_block_size(void *ptr)
{
    return __GET_BLOCK_SIZE(ptr);
}

void
set_block_number(void *ptr, long num)
{
    __SET_BLOCK_NUMBER(ptr, num);
}

void
set_block_size(void *ptr, int size)
{
    __SET_BLOCK_SIZE(ptr, size);
}

void 
set_block_dirty(void *ptr, int val)
{
    __SET_BLOCK_DIRTY(ptr, val);
}

int
is_block_dirty(void *ptr)
{
    return __IS_BLOCK_DIRTY(ptr);
}

void *
next_block(void *ptr)
{
    return __NEXT_BLOCK(ptr);
}

void *
prev_block(void *ptr)
{
    return __PREV_BLOCK(ptr);
}

#endif

/*
void
promote_block(File *file, void *ptr)
{
    if (ptr != file->head) {
	char *prev = remove_block(ptr);
	if (ptr == file->tail)
	    file->tail = prev;
	insert_block(file->head, ptr, 1);
	file->head = ptr;
    }
}
*/

/* Simple memory-allocation routines
 */
void *
emalloc(size)
    size_t size;
{
    void *p = malloc(size);
    if (!p)
	internal_error("emalloc(): Out of core");
    return p;
}

void *
ecalloc(count, size)
    int count;
    size_t size;
{
    void *p = calloc(count, size);
    if (!p)
	internal_error("emalloc(): Out of core");
    return p;
}

#ifdef __DEBUG
void
debug_print_buckets(fp)
    FILE *fp;
{
    int i;
    Bucket *bucket;

    fprintf(fp, "Memory status:\n");
    fprintf(fp, "Number of groups: %d\n", numgroups);

    for (i = 0; i < numgroups; i++) {
	fprintf(fp, "Group %4d:%8d%8d%8d\n", i,
		groups[i].esize,
		groups[i].ecnt,
		groups[i].numbuckets);
	fprintf(fp, "buckets:\n");
	for (bucket = groups[i].first; bucket; bucket = bucket->next) {
	    fprintf(fp, "\t%10p%10p%8d\n",
		    bucket->prev, bucket->next, bucket->used);
	}
    }
}

int
checkbucket(bucket)
    Bucket *bucket;
{
    Block ptr;
    int cnt=0;
    
    if (bucket->used < 0)
	internal_error("bucket->used < 0");
    for (ptr = (Block)bucket->avail; ptr; ptr = ptr->s.next) {
	cnt++;
	if (cnt > bucket->cnt)
	    internal_error("short-circuited bucket");
    }
    if (cnt != bucket->cnt - bucket->used)
	internal_error("cnt != bucket->cnt - bucket->used");
    return 0;
}
#endif
