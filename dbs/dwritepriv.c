/* $Id: dwritepriv.c,v 1.1 2001/11/03 22:33:52 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <dbs.h>

int
dwritepriv(file, priv, size)
    Data *file;
    void *priv;
    int size;
{
    if (size + sizeof(struct dataheader) > file->pagesize) {
	logmsg(L_ERR, "too many private data for file `%s'", file->name);
	return -1;
    }
    if (lseek(file->fd, sizeof(struct dataheader), SEEK_SET) !=
	sizeof(struct dataheader)) {
	logmsg(L_ERR, "seek error on `%s': %m", file->name);
	return -1;
    }
    if (write(file->fd, priv, size) != size) {
	logmsg(L_ERR, "error writing private header to `%s': %m", file->name);
	return -1;
    }
    return 0;
}



