/* $Id: makedict.h,v 1.1 2001/11/03 22:34:51 gray Exp $
 */
#ifndef __makedict_h
#define __makedict_h

#include <dict.h>

#define NUM_FIRST_INDEX TreePinyin-TreeJis+1
#define NUM_SECOND_INDEX TreeLast-TreeEnglish
/* for reading in kanjidic lines */

#define MAX_TOKEN_SIZE 256
#define MAXWORDSIZE 32
#define MAXPINYIN 7 /* maximum pinyin syllable length */

#define NOKANJI 0x2266
#define HIRAGANA_BYTE 0x24
#define KATAKANA_BYTE 0x25

#define UNCOMPRESS "gzip -d -c" 
#define UNCOMPRESSEXT ".gz"

#define DEFTEXTPAGESHIFT 12
#define MINTEXTPAGESHIFT 10

#define NUMITEMS(a) sizeof(a)/sizeof((a)[0])

/* This comes from <X11/Xutil.h> */
typedef struct {                /* normal 16 bit characters are two bytes */
    unsigned char byte1;
    unsigned char byte2;
} XChar2b;

struct indexfile {
    char *name;
    int (*fun)();
    int (*read)();
    int offset;
    int flags;
    int pageshift;
    int keylen;
    int keytype; /* if subkey == 0, else - number of keys */
    struct subkey *keyspec;
};


struct index_rdata {
    int offset;
    
    char *buf;
    int buflen;
    char *curp;
    DictEntry trans;
    Recno recno;

    char *start;
    char *stop;
};


extern char *dictdir;
extern char *kanjidict;
extern char *edict;
extern char *outdir;
extern char *dictname;
extern char *textname;
extern int verbose;
extern Uint textpagesize;
extern Uint textpageshift;
extern int maxstr8;
extern int maxstr16;
extern int maxbuflen8;
extern int maxbuflen16;
extern int numberofkanji;
extern int nedict;
extern struct indexfile indexes[];
extern int numberofkanji, highestkanji, lowestkanji;

void die();
int compile (void);
int xtoi (char *s);
XChar2b * dup_16 (XChar2b * kanabuffer);
FILE * open_compressed (char *dictname, int *);
Offset WriteText (char *);
Offset WriteText16 (XChar2b *);
Uint log2 (Uint);
XChar2b * strchr16 (XChar2b *str, unsigned c);
XChar2b * strseg16 (XChar2b *str, XChar2b *delim);
XChar2b * kanatoromaji (XChar2b *kana, char *buf, int size, char **endp);
char * get_text_ptr (Offset);
int get_text8 (Offset off, char *buf, int buflen);
char * mkfullname (char *, char *);
    

#endif


