/* $Id: reorder.c,v 1.1 2001/11/03 22:33:41 gray Exp $
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

#ifdef SHELL
void shellsort(char *base, unsigned nelem, unsigned width, int (*cmp) ());
#define sort(base,nelem,width,fcmp) \
        shellsort(base,nelem,width,fcmp)
#else
#define sort(base,nelem,width,fcmp) \
        qsort(base,nelem,width,(int (*)(const void *,const void *))fcmp)
#endif
static int read_buffer();

typedef struct {
    char *ptr;			/* Pointer to the current buffer position */
    char *end;			/* Pointer to the end buffer position */
    long disk;			/* Current Disk Position, Offset from Start;
                                 * -1 Means nothing is on disk. */
} BUFFER;

#define  NUM_BUFFER    600

struct memlimit {
    int minsize;
    int maxsize;
    int dec;
} fsort_mem = {
    8196,
    262144,
    4096
};

static BUFFER *spool_buf;	/* Spool working buffers */
static int i_small;		/* Current buffer number */
static char temp_ptr[255];	/* Buffer for temp file name */

static char *read_buf;		/* Disk Read buffer */
static unsigned buf_size, spool_size;
static int num_spool;		/* # of times the buffer was spooled */
static int temp_file = -1;	/* Temp file handle */
static unsigned rec_mem;	/* Number of records Read Buffer */

static long qsort_times;	/* # of times qsort was called */
static long qsort_len;		/* Length of qsort buffer */
static long num_read_buffer;	/* # of read_buffer() calls */
static long bytes_read_buffer;	/* count of bytes read (debug only)*/
static long disk_read_buffer;	/* # of actual disk reads */

/* IO stream information:
*/
static unsigned reclen;
static int (*compare) (char *, char *);
static int (*read_record) (long recno, char *buf);
static int (*write_record) (long recno, char *buf);

int read_buffer(int);

/* build_sort(): Builds sorted sequence of input records.
*/
static int
build_sort()
{
    long on_rec;
    unsigned int on_key;
    int i;
    char *rec_ptr;

    num_read_buffer = 0;
    disk_read_buffer = 0;
    bytes_read_buffer = 0;

    on_key = 0;
    num_spool = 0;

    rec_ptr = read_buf;

    qsort_times = 0;
    qsort_len = 0;

    for (on_rec = 0; (*read_record)(on_rec, rec_ptr) == 0; on_rec++) {
	on_key++;
	rec_ptr += reclen;

	if (on_key >= rec_mem) {
	    /* Buffer Full, Sort and Spool to Disk */
	    sort(read_buf, on_key, reclen, compare);

	    qsort_times++;
	    qsort_len += on_key * reclen;

	    if (num_spool == 0) {
		/* Open the temporary File */
		strcpy(temp_ptr, "/tmp/sortXXXXXX");
		mktemp(temp_ptr);
		temp_file = open(temp_ptr,
				 O_RDWR | O_CREAT | O_TRUNC,
				 S_IREAD | S_IWRITE);
		if (temp_file < 0) {
		    return -1;
		}
		lseek(temp_file, 0L, SEEK_SET);
	    }
	    if (write(temp_file, read_buf, on_key*reclen) != on_key * reclen) {
		logmsg(L_ERR, "write error on file `%s': %m", temp_ptr);
		return -1;
	    }
	    num_spool++;

	    rec_ptr = read_buf;
	    on_key = 0;
	}
    }

    /* Sort the buffer and spool if necessary */
    sort(read_buf, on_key, reclen, compare);
    qsort_times++;
    qsort_len += on_key * reclen;

    if (num_spool > 0) {
	if (write(temp_file, read_buf, on_key * reclen) != on_key * reclen) {
	    return -1;
	}
	num_spool++;

	spool_size = rec_mem * reclen;
	buf_size = (rec_mem / num_spool) * reclen;
	if (num_spool > NUM_BUFFER || rec_mem < num_spool) {
	    return -1;
	}
	for (i = 0; i < num_spool; i++) {
	    spool_buf[i].disk = i * (long) spool_size;
	    spool_buf[i].end = read_buf + buf_size * (i + 1);
	    read_buffer(i);
	}
    } else {
	/* One buffer was large enough. */
	spool_buf->disk = -1;
	spool_buf->ptr = read_buf;
	spool_buf->end = read_buf + (unsigned) on_rec*reclen;
	if (on_rec > 0)
	    num_spool = 1;
    }

    i_small = 0;
    return 0;
}


/* read_buffer(): Reads the next portion of spool buffer # 'i_buf'
                  into memory.
*/
int
read_buffer(i_buf)
    int i_buf;
{
    unsigned int bytes_read, ask_read, buffer_number;

    num_read_buffer++;

    /* Is there anything still on disk ? */
    if (spool_buf[i_buf].disk < 0) {
	if (num_spool > 0) {
	    /* Nothing left for Buffer: */
	    /* Move last buffer to current one and decrease num_spool */
	    spool_buf[i_buf] = spool_buf[num_spool - 1];
	    num_spool--;
	}
	return 0;
    }
    disk_read_buffer++;

    /* Calculate the buffer number from the current disk offset */
    buffer_number = (unsigned) (spool_buf[i_buf].disk / spool_size);
    spool_buf[i_buf].ptr = read_buf + buffer_number * buf_size;

    ask_read = (unsigned) ((long) spool_size * (buffer_number + 1) -
			   spool_buf[i_buf].disk);
    if (ask_read > buf_size)
	ask_read = buf_size;

    /* Read in the rest of the buffer from disk */

    lseek(temp_file, spool_buf[i_buf].disk, 0);
    bytes_read = read(temp_file, spool_buf[i_buf].ptr, ask_read);
    if (bytes_read < reclen || bytes_read == 0xFFFF) {
	/* Nothing left for Buffer */
	spool_buf[i_buf] = spool_buf[num_spool - 1];
	num_spool--;
	return 0;
    }
    spool_buf[i_buf].disk += bytes_read;
    if (bytes_read < buf_size ||
	spool_buf[i_buf].disk / spool_size != buffer_number) {
	spool_buf[i_buf].disk = -1;
    }
    bytes_read_buffer += bytes_read;

    spool_buf[i_buf].end = spool_buf[i_buf].ptr + bytes_read;

    return 0;
}

/* get_record():        Returns the next record from sorted buffer(s)
*/
static char *
get_record()
{
    char *current;
    int i;

    if (spool_buf[i_small].ptr >= spool_buf[i_small].end)
	read_buffer(i_small);
    if (num_spool <= 0)
	return (char *) 0;

    current = spool_buf[0].ptr;
    i_small = 0;
    for (i = 1; i < num_spool; i++) {
	if ((*compare) (spool_buf[i].ptr, current) < 0) {
	    current = spool_buf[i].ptr;
	    i_small = i;
	}
    }
    spool_buf[i_small].ptr += reclen;

    return current;
}

/* fsort():     Sorts the file.
 * Parameters:  rec_count      -        # of records to sort
 *              rec_len        -        Length of each record
 *              rread          -        Input function (reads record)
 *              rwrite         -        Output function (writes record)
 *              rcmp           -        Function for comparing two records.
 * Returns:     Success - 0
 *              Error   - <0
 */
int
fsort(rec_len, rread, rwrite, rcmp)
    unsigned rec_len;
    int (*rread) ();
    int (*rwrite) ();
    int (*rcmp) ();
{
    unsigned size;
    char *ptr;
    long rec;
    int rc = 0;
    
    reclen = rec_len;
    compare = rcmp;
    read_record = rread;
    write_record = rwrite;

    temp_file = -1;

    spool_buf = (BUFFER *) malloc(NUM_BUFFER * sizeof(BUFFER));
    if (!spool_buf)
	return -1;

    read_buf = NULL;
    size = fsort_mem.maxsize + fsort_mem.dec;
    while (size > fsort_mem.minsize && read_buf == NULL) {
	size -= fsort_mem.dec;
	read_buf = malloc(size);
    }

    if (!read_buf) {
	free(spool_buf);
	return -2;
    }
    rec_mem = size / reclen;


    if (build_sort() == 0) {
	for (rec = 0; ptr = get_record(); rec++) {
	    if ((rc = (*write_record) (rec, ptr)) < 0)
		break;
	}
    }
    if (temp_file >= 0) {
	close(temp_file);
	unlink(temp_ptr);
    }
    free(read_buf);
    free(spool_buf);
    return rc;
}
