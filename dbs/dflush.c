/* $Id: dflush.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <dbs.h>
#include <assert.h>

int
dflush_buffer(file, buf_ptr)
    Data *file;
    char *buf_ptr;
{
    Block blk = (Block) buf_ptr - 1;
    int mode, size;

    if (!blk->s.dirty)
	return 0;
    size = file->pagesize;
    mode = file->mode;

    if (mode & FL_WRITE) {
	/* place file pointer at the block to be updated */
	if (lseek(file->fd, get_block_offset(file, buf_ptr), SEEK_SET) == -1L){
	    dseterror(file);
	    return 1;
	}
	if (write(file->fd, buf_ptr, size) != size) {
	    dseterror(file);
	    return 1;
	}
    } else if (mode & FL_READ) {
	return 0;
    } else {
	dseterror(file);
	return 1;
    }

    blk->s.dirty = 0;
    return 0;
}

int
dflush_file(file)
    Data *file;
{
    char *ptr;

    for (ptr = file->head; ptr; ptr = next_block(ptr))
	dflush_buffer(file, ptr);
    if (!(file->mode & _BUF_WRITE) && is_block_dirty(file))
	flush_header(file);
    return 0;
}

void
flush_header(file)
    Data *file;
{
    struct dataheader hdr;

    hdr.magic = DATA_MAGIC;
    hdr.version = DATA_VERSION;
    hdr.unused = 0;
    hdr.pageshift = file->pageshift;
    hdr.reclen = file->reclen;
    hdr.reccnt = file->reccnt;
    lseek(file->fd, 0, SEEK_SET);
    write(file->fd, &hdr, sizeof(hdr));
    set_block_dirty(file, 0);
}
