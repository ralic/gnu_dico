/* $Id: dcreate.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dbs.h>

extern Data *_files;
extern int data_pageshift;
extern int data_pages_per_file;
extern int data_create_perm;

Data *
dcreate(name, mode, reclen, pageshift, max_pages, read_block, write_block)
    char *name;
    int mode;
    int reclen;
    int pageshift;
    int max_pages;
    int (*read_block)();
    int (*write_block)();
{
    Data *fp;
    int rc;

    fp = alloc_block(sizeof(*fp));
    if (!fp)
	return NULL;

    setflag(mode, FL_WRITE);
    rc = open(name, O_RDWR|O_CREAT|O_TRUNC, data_create_perm);
    if (rc == -1) {
	free_block(fp);
	return NULL;
    }
    fp->fd = rc;
    /* Create a header */
    fp->pageshift = pageshift ? pageshift : data_pageshift;
    fp->pagesize = 1 << fp->pageshift;
    fp->reclen = reclen;
    fp->reccnt = 0;
    fp->rec_per_page = fp->pagesize / fp->reclen;
    fp->name = strdup(name);
    fp->blk_max = max_pages ? max_pages : data_pages_per_file;
    fp->blk_cnt = 0;
    fp->head = fp->tail = NULL;
    if (mode & (_BUF_LOOKAHEAD | _BUF_LRU))
	setflag(mode, _BUF_LRU);
    fp->mode = mode | FL_APPEND;
    flush_header(fp);
    if (_files)
	_files = insert_block(_files, fp, 1);
    else
	_files = fp;
    return fp;
}

