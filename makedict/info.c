/* $Id: info.c,v 1.1 2001/11/03 22:34:56 gray Exp $ 
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <dbs.h>


char *base;

void print_string();
void print_int();
void print_long();
void print_subkeys();

static char int_flags[] = "oxd";
#define NAMESIZ 5

struct builtin_t {
    int used;
    char name[NAMESIZ];
    void (*inst)();
    int offset;
    char *flags;
    int flg;
    int len;
    int pos;
} builtin[] = {
    0, "NAME", print_string, offsetof(Index, name),      "rl",        0, 0, 0,
    0, "PSIZ", print_int,    offsetof(Index, pagesize),  int_flags, 'd', 8, 0,
    0, "PSFT", print_int,    offsetof(Index, pageshift), int_flags, 'd', 8, 0,
    0, "KLEN", print_int,    offsetof(Index, key_len),   int_flags, 'd', 8, 0,
    0, "KMAX", print_int,    offsetof(Index, keys_max),  int_flags, 'd', 8, 0,
    0, "KHLF", print_int,    offsetof(Index, keys_half), int_flags, 'd', 8, 0,
    0, "KNUM", print_int,    offsetof(Index, numsubkeys),int_flags, 'd', 8, 0,
    0, "KEYS", print_subkeys,offsetof(Index, subkey),    "",          0,21, 0,
    0, "FLAG", print_int,    offsetof(Index, idxflags),  int_flags, 'x', 8, 0,
    0, "ROOT", print_long,   offsetof(Index, root),      int_flags, 'd', 8, 0,
};
#define NUMFIELDS sizeof(builtin)/sizeof(builtin[0])

struct builtin_t * prog[NUMFIELDS+1];

static struct builtin_t *
find_builtin(name)
    char *name;
{
    int i;
    
    for (i = 0; i < sizeof(builtin)/sizeof(builtin[0]); i++) {
	if (strcmp(name, builtin[i].name) == 0)
	    return &builtin[i];
    }
    return NULL;
}


void
printheader()
{
    int i, n;
    struct builtin_t **ptr;

    for (ptr = prog; *ptr; ptr++) {
	int len = (*ptr)->len;
	n = (len - NAMESIZ - 1)/2;
	for (i = 0; i < n; i++)
	    printf(" ");
	printf("%s", (*ptr)->name);
	i += NAMESIZ - 1;
	for (; i <= len; i++)
            printf(" ");
    }
    printf("\n");
}

void
mkprog(str)
    char *str;
{
    char name[NAMESIZ];
    int flg, len;
    char *q;
    struct builtin_t *bp;
    struct builtin_t **ptr;
    int pos;
    
    ptr = prog;
    pos = 0;
    while (*str) {
	q = name;
	while (*str && *str != ':' && *str != ',') {
	    if (q >= name + NAMESIZ)
		die("Field name too long near `%s'", str);
	    *q++ = *str++;
	}
	*q = 0;
	bp = find_builtin(name);
	if (!bp)
	    die("Unknown field: %s", name);

	flg = bp->flg;
	len = bp->len;
	
	if (*str == ':') {
	    if (!bp->flags) {
		logmsg(L_WARN, "Field `%s' takes no flags. Flag %c ignored",
			name, *++str);
	    } else {
		flg = *++str;
		if (strchr(bp->flags, flg) == 0) 
		    die("Flag %c can't be used with field `%s'", flg, name);
	    }
	    
	    if (*++str == ':') {
		len = strtol(str+1, &str, 10);
		if (*str && *str != ',')
		    die("Can't convert field length. Stopped near %s", str);
	    }
	}

	if (bp->used) {
	    logmsg(L_WARN, "Field `%s' already specified", name);
	} else {
	    bp->flg = flg;
	    bp->len = len;
	    bp->pos = pos;
	    *ptr++ = bp;
	    pos += len+1;
	}   
	if (*str && *str++ != ',')
	    die("Expected ',' near `%s'", str-1);
    }
    *ptr++ = 0;
}
    
void
info(name)
    char *name;
{
    Index *iptr = NULL;
    struct builtin_t **ptr;

    iptr = iopen(name, 0);
    if (!iptr) {
	logmsg(L_WARN, "Can't open index file `%s'", name);
	return;
    }
    base = (char*)iptr;
    for (ptr = prog; *ptr; ptr++) {
	(*ptr)->inst((*ptr)->offset, (*ptr)->flg, (*ptr)->len, (*ptr)->pos);
	printf(" ");
    }
    printf("\n");
}

static char *
makeintfmt(islong, flg, len)
    int islong, flg, len;
{
    static char fmt[10];
    char *fmtp;

    fmtp = fmt;
    *fmtp++ = '%';
    if (len) 
	fmtp += sprintf(fmtp, "%d", len);
    if (islong)
	*fmtp++ = 'l';
    *fmtp++ = flg;
    *fmtp = 0;
    return fmt;
}

void
print_string(off, flg, len, pos)
    int off, flg, len, pos;
{
    char fmt[10], *fmtp;

    fmtp = fmt;
    *fmtp++ = '%';
    if (flg == 'l') 
	*fmtp++ = '-';
    if (len) 
	fmtp += sprintf(fmtp, "%-d.%-d", len, len);
    *fmtp++ = 's';
    *fmtp = 0;
    printf(fmt, *(char**)(base + off));
}

void
print_int(off, flg, len, pos)
    int off, flg, len, pos;
{
    printf(makeintfmt(0, flg, len), *(int*)(base + off));
}

void
print_long(off, flg, len, pos)
    int off, flg, len, pos;
{
    printf(makeintfmt(0, flg, len), *(int*)(base + off));
}

static char *
typestr(n)
    int n;
{
    static char *types[] = {
	"USER",
	"INT_8",
	"UINT_8",
	"INT_16",
	"UINT_16",
	"INT_32",
	"UINT_32",
	"CHR_8",
	"CASECHR_8",
	"CHR_16",
    };

    if (n >= 0 && n < sizeof(types)/sizeof(types[0]))
	return types[n];
    return "UNKNOWN";
}

void
print_subkeys(off, flg, len, pos)
    int off, flg, len, pos;
{
    int i;
    int num = ((Index*)base)->numsubkeys;
    struct subkey *sp = ((Index*)base)->subkey;
    char buf[64];
    
    while (num--) {
	sprintf(buf, "%s(%d,%d,%c)",
		typestr(sp->type),
		sp->offset,
		sp->len,
		sp->desc ? 'd' : 'a');
	printf("%*.*s", len, len, buf);
	if (num) {
	    printf("\n");
	    for (i = 0; i < pos; i++)
		printf(" ");
	}
	sp++;
    }
}
