/* $Id: dseek.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dbs.h>

long
dseek(file, recno, from)
    Data *file;
    long recno;
    int from;
{
    if (file->mode & _FL_ERR)
	return -1;

    switch (from) {
    case SEEK_SET:
	file->pos = recno;
	break;
    case SEEK_CUR:
	file->pos += recno;
	break;
    case SEEK_END:
	file->pos = file->reccnt - recno;
	break;
    }
    if (file->pos >= file->reccnt) {
	file->pos = file->reccnt;
	setflag(file->mode, FL_APPEND|_FL_EOF);
    } else
	clearflag(file->mode, FL_APPEND|_FL_EOF);
    
    return file->pos;
}
