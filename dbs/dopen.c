/* $Id: dopen.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dbs.h>

Data *_files;
int data_create_perm = 0644;
int data_pageshift = 12;
int data_pages_per_file = 10;
int data_max_pages = 16;

Data *
dopen_short(name, mode)
    char *name;
    int mode;
{
    return dopen(name, mode, data_pages_per_file, NULL, NULL);
}

Data *
dopen(name, mode, max_pages, read_block, write_block)
    char *name;
    int mode;
    int max_pages;
    int (*read_block)();
    int (*write_block)();
{
    Data *fp;
    int rc;
    struct dataheader hdr;
    int omode = 0;

    fp = alloc_block(sizeof(*fp));
    if (!fp)
	return NULL;

    if (mode & FL_WRITE)
	omode |= O_RDWR;
    else
	omode |= O_RDONLY;
    rc = open(name, omode);
    if (rc == -1) {
	free_block(fp);
	return NULL;
    }
    fp->fd = rc;
    if (read(fp->fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
	logmsg(L_ERR, "Can't read header on datafile `%s': %m", name);
	close(fp->fd);
	free_block(fp);
	return NULL;
    }
    if (hdr.magic != DATA_MAGIC || hdr.version != DATA_VERSION) {
	logmsg(L_ERR, "bad datafile header. File `%s'", name);
	close(fp->fd);
	free_block(fp);
	return NULL;
    }
    fp->pageshift = hdr.pageshift;
    fp->pagesize = 1 << fp->pageshift;
    fp->reclen = hdr.reclen;
    fp->reccnt = hdr.reccnt;
    fp->rec_per_page = fp->pagesize / fp->reclen;
    fp->name = strdup(name);
    fp->blk_max = max_pages ? max_pages : data_max_pages;
    fp->blk_cnt = 0;
    fp->read_block = read_block;
    fp->write_block = write_block;
    fp->head = fp->tail = NULL;
    if (mode & (_BUF_LOOKAHEAD | _BUF_LRU))
	setflag(mode, _BUF_LRU);
    fp->mode = mode;
    if (_files)
	_files = insert_block(_files, fp, 1);
    else
	_files = fp;
    return fp;
}

#ifndef INLINE_BLOCK_FUNCTIONS
long
get_block_offset(file, ptr)
    Data *file;
    void *ptr;
{
    return __GET_BLOCK_OFFSET(file, ptr);
}
#endif
