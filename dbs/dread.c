/* $Id: dread.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbs.h>

int
_f_attach_buffer(file)
    Data *file;
{
    char *ptr, *buf_ptr;
    long nblock;
    int rdsize;

    /* compute page offset in file */
    nblock = POS_TO_BLOCK_NUM(file, file->pos);

    /* Look up in the buffer chain
     */
    for (ptr = file->head; ptr; ptr = next_block(ptr)) {
	if (get_block_number(ptr) == nblock) {
	    promote_block(file, ptr);
	    return 0;
	}
    }

    /* If buffer chain is full, discard last recently used buffer */
    if (file->blk_cnt == file->blk_max) {
	buf_ptr = file->tail;
	if (file->write_block) {
	    long first_rec = (get_block_number(buf_ptr)-1) *
		             file->rec_per_page;
	    long rec_num;
	    if (first_rec + file->rec_per_page + 1 > file->reccnt)
		rec_num = file->reccnt - first_rec;
	    else
		rec_num = file->rec_per_page;
	    file->write_block(buf_ptr, first_rec, rec_num);
	}
	    
	dflush_buffer(file, buf_ptr);
	file->tail = remove_block(file->tail);
	file->blk_cnt--;
    } else {
	buf_ptr = alloc_block(file->pagesize);
    }

    if (file->mode & FL_APPEND) {
	rdsize = file->pagesize;
	memset(buf_ptr, 0, rdsize);
	/* something else ? */
    } else if (!(deof(file))) {
	if (lseek(file->fd, nblock * file->pagesize, SEEK_SET) == -1) {
	    free_block(buf_ptr);
	    setflag(file->mode, _FL_ERR);
	    return -1;
	}
	rdsize = read(file->fd, buf_ptr, file->pagesize);
	if (rdsize < 0) {
	    setflag(file->mode, _FL_ERR);
	    free_block(buf_ptr);
	    return -1;
	}
	if (rdsize < file->pagesize) {
	    setflag(file->mode, _FL_EOF);
	}
    } else {
	rdsize = 0;
    }
    set_block_size(buf_ptr, rdsize);
    set_block_number(buf_ptr, nblock);
    if (file->read_block) {
	long first_rec = (nblock-1) * file->rec_per_page;
	long rec_num;
	if (first_rec + file->rec_per_page + 1 > file->reccnt)
	    rec_num = file->reccnt - first_rec;
	else
	    rec_num = file->rec_per_page;
	file->read_block(buf_ptr, first_rec, rec_num);
    }

    if (file->head)
	insert_block(file->head, buf_ptr, 1);
    else
	file->tail = buf_ptr;
    file->head = buf_ptr;
    file->blk_cnt++;
    return 0;
}

int
dread(file, buf)
    Data *file;
    void *buf;
{
    int off;

    if (file->mode & _FL_ERR)
	return -1;
    /* Read first buffer */
    if (file->mode & FL_APPEND || _f_attach_buffer(file))
	return -1;
    if (buf) {
	off = POS_TO_BLOCK_OFF(file, file->pos);
	memcpy(buf, file->head + off, file->reclen);
    }
    return 0;
}


void
promote_block(file, ptr)
    Data *file;
    void *ptr;
{
    if (ptr != file->head) {
	char *prev = remove_block(ptr);
	if (ptr == file->tail)
	    file->tail = prev;
	insert_block(file->head, ptr, 1);
	file->head = ptr;
    }
}

