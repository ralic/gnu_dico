/* $Id: dbs.h,v 1.1 2001/11/03 22:33:31 gray Exp $
 */
#ifndef __dbs_h
#define __dbs_h

#include <stddef.h>

typedef long Blockno;
typedef long Recno;
typedef int Int;

typedef double Align;

#define OPENMODE O_RDWR
#define OPENACCESS S_IREAD|S_IWRITE

#define setflag(w,f) (w)|=(f)
#define clearflag(w,f) (w)&=~(f)

/* Memory management */
#define MAXGROUPS 100
#define BUCKETSIZE 65536 

#define MIN_BLOCKS_PER_BUCKET 2
#define BLOCKSIZE(nbytes) ((nbytes + HEADERSIZE - 1)/HEADERSIZE+1)
#define HEADERS_PER_BUCKET BUCKETSIZE/HEADERSIZE
#define DATASTART BLOCKSIZE(sizeof(Bucket))
#define MAX_BLOCK_SIZE \
 ((HEADERS_PER_BUCKET-DATASTART)/MIN_BLOCKS_PER_BUCKET-1)*HEADERSIZE

#define DEFBLOCKSIZE 512
#define DEFBLOCKCOUNT 8

#define __DEBUG

#define ALIGN_COUNT \
 (sizeof(struct block_header_str)+sizeof(Align)-1)/sizeof(Align)

typedef union block_header_rec * Block;

typedef union block_header_rec {
    struct block_header_str {
	void *bucket;
	long blockno; /* Number of block in the file */
	int size;
	int dirty;
	Block next, prev; /* ptrs to next & previous headers in the chain */
    } s;
    Align foo[ALIGN_COUNT];
    /* Actual data goes after this header */
} BlockHeader;

#define HEADERSIZE sizeof(BlockHeader)

typedef struct bucket_t {
    struct bucket_t *next, *prev;
    int group;
    int cnt;     /* Number of blocks */
    int used;    /* Number of used blocks */
    Block avail;   /* Entry to the list of available ones */
/*  Item item[cnt];    */
} Bucket;

/* These are intermediate macros. Do not use them on their own.
 */
#define __GET_BLOCK_NUMBER(ptr) ((Block)(ptr))[-1].s.blockno
#define __GET_BLOCK_SIZE(ptr) ((Block)(ptr))[-1].s.size
#define __SET_BLOCK_NUMBER(ptr, n) ((Block)(ptr))[-1].s.blockno = n
#define __SET_BLOCK_SIZE(ptr, size)  ((Block)(ptr))[-1].s.size = size
#define __SET_BLOCK_DIRTY(ptr, val)  ((Block)(ptr))[-1].s.dirty = val
#define __IS_BLOCK_DIRTY(ptr) ((Block)(ptr))[-1].s.dirty
#define __NEXT_BLOCK(ptr) \
 (((Block)(ptr))[-1].s.next ? (char*)(((Block)(ptr))[-1].s.next+1) : (char*)0)
#define __PREV_BLOCK(ptr) \
 (((Block)(ptr))[-1].s.prev ? (char*)(((Block)(ptr))[-1].s.prev+1) : (char*)0)

#ifdef INLINE_BLOCK_FUNCTIONS
#ifdef __GNUC__
/* Define functions inline */
extern inline long
get_block_number(void *ptr)
{
    return __GET_BLOCK_NUMBER(ptr);
}

extern inline size_t
get_block_size(void *ptr)
{
    return __GET_BLOCK_SIZE(ptr);
}

extern inline void
set_block_number(void *ptr, long num)
{
    __SET_BLOCK_NUMBER(ptr, num);
}

extern inline void
set_block_size(void *ptr, int size)
{
    __SET_BLOCK_SIZE(ptr, size);
}

extern inline void 
set_block_dirty(void *ptr, int val)
{
    __SET_BLOCK_DIRTY(ptr, val);
}

extern inline int
is_block_dirty(void *ptr)
{
    return __IS_BLOCK_DIRTY(ptr);
}

extern inline void *
next_block(void *ptr)
{
    return __NEXT_BLOCK(ptr);
}

extern inline void *
prev_block(void *ptr)
{
    return __PREV_BLOCK(ptr);
}

#else /* !defined(__GNUC__) */
/* Define macros */
#define get_block_number __GET_BLOCK_NUMBER
#define get_block_size __GET_BLOCK_SIZE
#define set_block_number __SET_BLOCK_NUMBER
#define set_block_size __SET_BLOCK_SIZE
#define next_block __NEXT_BLOCK
#define prev_block __PREV_BLOCK
#define get_block_offset __GET_BLOCK_OFFSET
#endif /* __GNUC__ */
#else  /* !defined(INLINE_BLOCK_FUNCTIONS) */
extern long get_block_number (void *);
extern size_t get_block_size (void *);
extern void set_block_number (void *, long);
extern void set_block_size (void *, int);
extern void set_block_dirty (void *ptr, int val);
extern int is_block_dirty (void *ptr);
extern void * next_block (void *);
extern void * prev_block (void *);
#endif /* INLINE_BLOCK_FUNCTIONS */

void * free_block (void *);
void * alloc_block (size_t);
void * insert_block (void *, void *, int);
void * remove_block (void *);
void swap_blocks (void *, void *);

void * emalloc (size_t);
void * ecalloc (int, size_t);
#define efree free

/* Data files */

/* File mode values
 */
#define FL_CLOSED  000000   /* file closed */
/* These flags are used in `mode' arg to f_open(). */
#define FL_READ    000001   /* open file for reading */
#define FL_WRITE   000002   /* open file for writing */
#define FL_CREATE  000004   /* create and open file (implies FL_WRITE) */
/* These are for internal use only
 */
#define FL_APPEND  000010

/* Buffering modes
 */
#define _BUF_NONE      000040
#define _BUF_READ      000100  /* Buffer reads */
#define _BUF_WRITE     000200  /* Buffer writes */
#define _BUF_SMART     000400  /* Reserved */
#define _BUF_LRU       001000  /* LRU scheme */
#define _BUF_LOOKAHEAD 002000  /* Lookahead scheme */
/* Other flags
 */
#define _FL_EOF        004000  /* eof reached */
#define _FL_ERR        010000  /* error ocurred */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif


#define DATA_MAGIC   010125040504L /* DATA */
#define DATA_VERSION 0

struct dataheader {
    long magic;
    long version;
    long unused;
    int pageshift;      /* pagesize = 1 << pageshift */
    int reclen;
    long reccnt;
};

typedef struct {
    int fd;                 /* file handle */
    int mode;               /* file mode */
    int pageshift;
    int pagesize;           /* buffer block size */
    int reclen;
    long reccnt;
    char *name;
    
    int rec_per_page;       /* number of records per page */
    int blk_max;            /* max # of blocks for the file */
    int blk_cnt;            /* actual # of blocks in buffer chain */
    char * head, * tail;    /* first and last buffer blocks of the chain */
    long pos;               /* current file position */
    long delta;             /* reserved for future use */

    int (*read_block) (void *, long, long);
    int (*write_block) (void *, long, long);
} Data;

#define POS_TO_BLOCK_NUM(f, pos) pos / (f)->rec_per_page + 1
#define POS_TO_BLOCK_OFF(f, pos) ((pos) % (f)->rec_per_page) * (f)->reclen

#define __GET_BLOCK_OFFSET(f,ptr) (f)->pagesize * __GET_BLOCK_NUMBER(ptr)
#ifdef INLINE_BLOCK_FUNCTIONS
#ifdef __GNUC__
/* Define functions inline */
extern inline long
get_block_offset(Data *file, void *ptr)
{
    return __GET_BLOCK_OFFSET(file, ptr);
}
#else /* !defined(__GNUC__) */
/* Define macros */
#define get_block_offset __GET_BLOCK_OFFSET
#endif /* __GNUC__ */
#else  /* !defined(INLINE_BLOCK_FUNCTIONS) */
extern long get_block_offset (Data *, void *);
#endif

#define dwhere(f) (f)->pos
#define dclear_error(f) clearflag((f)->mode,_FL_ERR)
#define dclrerr dclear_error
#define deof(f) ((f)->mode & _FL_EOF)
#define dseterror(f) setflag((f)->mode, _FL_ERR)

void promote_block (Data *, void *);
int dclose (Data *file);
int dflush_buffer (Data *file, char *buf_ptr);
int dflush_file (Data *file);
void _f_free_blocks (Data *file);
int dchgmode (Data *file, int flag, int set);
Data * dopen_short (char *name, int mode);
int _f_attach_buffer (Data *file);
int dread (Data *file, void *buf);
long dseek (Data *file, long recno, int from);
int dwrite (Data *file, void *buf);
void flush_header (Data *file);
Data * dopen (char *name, int mode, int max_pages,
		int (*read_block)(), int (*write_block)());
Data * dcreate (char *name, int mode, int reclen, int pageshift,
		  int max_pages, int (*read_block)(), int (*write_block)());
int dreadpriv (Data* file, void *priv, int size);
int dwritepriv (Data* file, void *priv, int size);
void _dfree_blocks(Data *file);
    
/* Index files */

#define MAX_KEY_SIZE    338
#define MIN_PAGE_SHIFT 10
#define MAX_PAGE_SHIFT 16

#define IF_UNIQUE       0x00000001
#define IF_ALLOWINEXACT 0x00000002

#define KEY_USER      0
#define KEY_INT_8     1
#define KEY_UINT_8    2
#define KEY_INT_16    3
#define KEY_UINT_16   4
#define KEY_INT_32    5
#define KEY_UINT_32   6
#define KEY_CHR_8     7
#define KEY_CASECHR_8 8
#define KEY_CHR_16    9

#define INDEX_MAGIC 013021047111L /* INDX */
#define INDEX_VERSION 0

struct subkey {
    int type;             /* subkey type */
    int offset;           /* offset in the key */
    int len;              /* length of the key */
    int cmplen;           /* length to use in compare (string keys only) */
    int desc;             /* descending sort */
};

struct indexheader {
    long magic;
    long version;
    long unused;
    int pageshift;      /* pagesize = 1 << pageshift */
    Blockno root;       /* Root Block */
    Blockno eof;        /* Offset of First Free Block */
    int group_len;      /* Key Length + 2*sizeof(long) */
    int key_len;        /* Key Length */
    int keys_max;       /* Maximum # of keys per block;  <= 100 */
    int keys_half;      /* Maximum # of keys per half block */
    long idxflags;      /* Indexing flags */
    int numsubkeys;     /* Number of subkeys */
                        /* Here go the `struct subkey' records */
};

typedef struct index *Indexp;
typedef struct iblock *Iblockp;

struct seq_search { /* sequential search structure */
    void *key;      /* Search key value */
    Recno recno;    /* record number of the last found match */
    void *lastkey;  /* keyvalue of the last found match */
};

typedef struct index {
    char *name;        /* File name */
    int fd;            /* File handle */
    int lock;          /* Locking mode */
    int version;
    int old_version;
    
    Iblockp block_ref;   /* Reference to the current block */
    Iblockp block_first; /* The first used memory block */
    Iblockp block_last;  /* The last used memory block */
    int block_num;	/* The number of blocks in the buffer chain */
    int block_max;	/* The maximum number of blocks in the chain */

    /* Index File Header Information */

    int idxflags;
    int pageshift;
    int pagesize;
    Blockno virtual_eof;/* The next available file block */
    Blockno root;       /* Root Block */
    Blockno eof;        /* Offset of First Free Block */
    int group_len;      /* Key Length + 2*sizeof(long) */
    int key_len;        /* Key Length */
    int keys_max;       /* Maximum # of keys per block;  <= 100 */
    int keys_half;      /* Maximum # of keys per half block */
    int numsubkeys;     /* Number of subkeys in the key */
    struct subkey *subkey; /* Subkey descriptors */
    int numcompkeys;       /* Number of subkeys to use in comparing */
    struct seq_search seq; /* Sequential search structure */
}  Index;

typedef struct {
   Blockno file_block;
   Recno rec_num;
   char value[1] ;  /* The key size is variable */
} Key;

/* Index File Page Format:
 *
 * Number of Keys
 * Repeat for Each Key in Block -
 *    Block Pointer  (Starting from 0)
 *    Record Number
 *    Key
 * End
 */


typedef struct iblock {
   int key_on;          /* The current key.  Starts at 0.  -1 means none.
                         * num_keys+1 indicates EOF
			 */
   short num_keys;
   short pointers[1];
} Iblock;

#define BLOCK_MEMSIZE(p) (offsetof(Iblock, num_keys) + (p)->pagesize)

#define SUCCESS 0
#define FAILURE 1
#define EMPTYFILE 2
#define EMPTYLIST 3
#define LEAFREACHED 4
#define IADD_DUP     5
#define IADD_CLASH   6
#define IADD_FAILURE 7
#define ISYNC_KEYLOCATED 8
#define ISYNC_NOTFOUND 9
#define ISYNC_EOF 10
#define ISYNC_EOF_KEYLOCATED 11
#define KEY_EOF      12
#define KEY_GREATER  13
#define WOULDBLOCK 14

extern int _icache;
extern Index *_ifile;
extern Index *_ifree;
extern int idx_pageshift;
extern int idx_create_perm;
extern int _blkcount[];
extern int (*idx_compare[])();
    
Iblock * alloc_index_block (Index*);
Iblock * free_index_block (Index*, Iblock*);

Iblock * alloc_index_block (Index *iptr);
Iblock *free_index_block (Index *iptr, Iblock *bp);
int ifree (Index *iptr);
int _blkwrite (Index *iptr, Iblock *bp);
Iblock *_blkget (Index *iptr);
int _addkey (Index *iptr, Key *key_ptr);
int _blkleaf (Index *iptr);
int _blkdn (Index *iptr, int direction);
int _blkup (Index *iptr);
Key * _blkkey (Index *iptr);
int _removeblk (Index *iptr);
int _removekey (Index *iptr);
int _blksearch (Index *iptr, void *search_string);
Recno _blkskip (Index *iptr, Recno recno);
int icmp (Index *iptr, char *, char *);
int key_length (int type, int len);
int ilock (Index *iptr);
int iunlock (Index *iptr);
int iseek (Index *iptr, void *search_string);
Recno iskip (Index *iptr, Recno num_skip);
int ibottom (Index *iptr);
int itop (Index *iptr);
Index * iopen (char *name, int rw);
int iclose (Index *iptr);
int isync (Index *iptr, void *value_ptr, Recno rec_num);
int reindex (Index *iptr, int (*rf)(), void *rdata);
Index * icreate (char *name, int flags, int pageshift, int keylen, 
		   int numsubkeys, struct subkey *subkeys,
		   int (*rf)(), void *rdata);
int iadd (Index *iptr, void *value_ptr, Recno rec_num);
int iseq_begin (Index *iptr, void *value_ptr);
int iseq_next (Index *iptr);
int iseq_prev (Index *iptr);
int iseq_end (Index *iptr);
int iseq_matchkey (Index *iptr, void *value);
Recno irecno (Index *iptr);
void *ivalue (Index *iptr);
void imode (Index *iptr, int flags, int set);
int icmplen (Index *iptr, int n, int len);
int check_key(int, struct subkey *);
Iblock * _free_index_block(int shift, Iblock *bp);

extern int logmsg();
#define L_DEBUG     0 
#define L_INFO      1
#define L_NOTICE    2
#define L_WARN      3
#define L_ERR       4
#define L_CRIT      5
#define L_ALERT     6
#define L_EMERG     7

#define internal_error(text) \
 do { logmsg(L_CRIT, "INTERNAL ERROR:%s:%d:%s", __FILE__, __LINE__, text);\
      abort(); } while (0) 

    
#endif
