/* $Id: dclose.c,v 1.1 2001/11/03 22:33:47 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dbs.h>

int
dclose(file)
    Data *file;
{
    /* make dflush_file() write the header: */
    clearflag(file->mode, _BUF_WRITE); 
    dflush_file(file);
    _dfree_blocks(file);
    close(file->fd);
    free(file->name);
    free_block(file);
    return 0;
}
