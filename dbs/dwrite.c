/* $Id: dwrite.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <dbs.h>

int
_copy_to_block(blk_ptr, mem_ptr, pos, size)
    char *blk_ptr;
    char *mem_ptr;
    int pos;
    int size;
{
    Block blk = (Block) blk_ptr - 1;

    memcpy(blk_ptr + pos, mem_ptr, size);
    size += pos;
    if (size > blk->s.size)
	blk->s.size = size;
    blk->s.dirty = 1;
    return 0;
}

int
dwrite(file, buf)
    Data *file;
    void *buf;
{
    if (file->mode & _FL_ERR || !(file->mode & FL_WRITE))
	return -1;
    /* Read first buffer */
    if (_f_attach_buffer(file))
	return -1;
    _copy_to_block(file->head, buf, POS_TO_BLOCK_OFF(file, file->pos),
		   file->reclen);
    if (file->mode & FL_APPEND) {
	file->reccnt++;
	set_block_dirty(file, 1);
	clearflag(file->mode, FL_APPEND|_FL_EOF);
    }
    if (!(file->mode & _BUF_WRITE))
	dflush_file(file);
    
    return 0;
}
