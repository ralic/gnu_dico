/* $Id: dmode.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

void
_dfree_blocks(file)
    Data *file;
{
    char *ptr, *next;

    for (ptr = file->head; ptr; ptr = next) {
	next = next_block(ptr);
	free_block(ptr);
    }
    file->head = file->tail = NULL;
    file->blk_cnt = 0;
}

int
dchgmode(file, flag, set)
    Data *file;
    int flag;
    int set;
{
    int newmode = file->mode;
    if (set)
	setflag(newmode, flag);
    else
	clearflag(newmode, flag);
    if (newmode == file->mode)
	return 0;
    dflush_file(file);
    file->mode = newmode;
    return 0;
}
