/* $Id: dreadpriv.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <dbs.h>

int
dreadpriv(file, priv, size)
    Data *file;
    void *priv;
    int size;
{
    if (size + sizeof(struct dataheader) > file->pagesize) {
	logmsg(L_ERR, "can't read that many private data from file `%s'",
	      file->name);
	return -1;
    }
    if (lseek(file->fd, sizeof(struct dataheader), SEEK_SET) !=
	sizeof(struct dataheader)) {
	logmsg(L_ERR, "seek error on `%s': %m", file->name);
	return -1;
    }
    if (read(file->fd, priv, size) != size) {
	logmsg(L_ERR, "error reading private header from `%s': %m", file->name);
	return -1;
    }
    return 0;
}



