/* $Id: check.c,v 1.1 2001/11/03 22:34:56 gray Exp $ */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dbs.h>

static void print_subkey(char *, struct subkey*);

void
dump(name)
    char * name;
{
    int i;
    Index *iptr = NULL;
    Key *key;

    iptr = iopen(name, 0);
    if (!iptr) {
	logmsg(L_WARN, "Can't open index file `%s'", name);
	return;
    }
    
    printf("Dump of `%s'\n", iptr->name);

    if (itop(iptr) != SUCCESS)
	die("itop!");

    do {
	key = _blkkey(iptr);
	if (!key) {
	    logmsg(L_WARN, "_blkkey() returned NULL");
	    break;
	}
	printf("%8ld ", key->rec_num);
	for (i = 0; i < iptr->numsubkeys; i++) {
	    print_subkey(key->value, iptr->subkey+i);
	    if (i < iptr->numsubkeys-1)
		printf("-");
	}
	printf("\n");
    } while (iskip(iptr, 1L) == 1L);
    
    iclose(iptr);
}

void
print_subkey(keyval, subkey)
    char *keyval;
    struct subkey *subkey;
{
    keyval += subkey->offset;
    switch (subkey->type) {
    case KEY_INT_8:
    case KEY_UINT_8:
	printf("%02x", *(unsigned char*)keyval);
	break;
    case KEY_INT_16:
    case KEY_UINT_16:
	printf("%04x", *(unsigned short*)keyval);
	break;
    case KEY_INT_32:
    case KEY_UINT_32:
	printf("%08x", *(unsigned int*)keyval);
	break;
    case KEY_CHR_8:
    case KEY_CASECHR_8:
	printf("%-*.*s", subkey->len, subkey->len, keyval);
	break;
    case KEY_CHR_16:
	printf("%s", keyval);
    }
}

