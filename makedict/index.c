#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbs.h>
#include <makedict.h>
#define obstack_chunk_alloc emalloc
#define obstack_chunk_free efree
#include <obstack.h>

int index_char_read (struct index_rdata *, long, long *, void *, int);
int index_short_read (struct index_rdata *, long, long *, void *, int);
int index_int_read (struct index_rdata *, long, long *, void *, int);
int index_text_read (struct index_rdata *, long, long *, void *, int);
int index_text16_read (struct index_rdata *, long, long *, void *, int);
int index_pinyin_read (struct index_rdata *, long, long *, void *, int);
int index_yomi_read (struct index_rdata *, long, long *, void *, int);
int index_romaji_read (struct index_rdata *, long, long *, void *, int);
int index_words_read (struct index_rdata *, long, long *, void *, int);
int index_bushu_read (struct index_rdata *, long, long *, void *, int);
int index_xref16_read (struct index_rdata *, long, long *, void *, int);

int text_init (int, struct index_rdata *, struct indexfile *);
int text16_init (int, struct index_rdata *, struct indexfile *);
int pinyin_init (int, struct index_rdata *, struct indexfile *);

struct subkey bushu_key[] = {
    KEY_UINT_16, offsetof(Bushu, bushu), 0, 0, 0,
    KEY_UINT_16, offsetof(Bushu, numstrokes), 0, 0, 0,
};

struct indexfile indexes[] = {
    /* JIS codes */
    JIS_TREE, NULL, index_short_read,
    offsetof(DictEntry,Jindex), 0, 12, sizeof(short), KEY_UINT_16, NULL,

    /* Unicode */
    UNICODE_TREE, NULL, index_short_read,
    offsetof(DictEntry,Uindex), 0, 12, sizeof(short), KEY_UINT_16, NULL,

    /* Four-corner index */
    CORNER_TREE, NULL, index_short_read,
    offsetof(DictEntry,Qindex), 0, 12, sizeof(short), KEY_UINT_16, NULL,

    /* Frequency */
    FREQ_TREE, NULL, index_short_read,
    offsetof(DictEntry,frequency), 0, 12, sizeof(short), KEY_UINT_16, NULL,

    /* Nelson dictionary index */
    NELSON_TREE, NULL, index_short_read,
    offsetof(DictEntry,Nindex), 0, 12, sizeof(short), KEY_UINT_16, NULL,

    /* Halpern dictionary index */
    HALPERN_TREE, NULL, index_short_read,
    offsetof(DictEntry,Hindex), 0, 12, sizeof(short), KEY_UINT_16, NULL,

    /* Jouyou grade level */
    GRADE_TREE, NULL, index_char_read,
    offsetof(DictEntry,grade_level), 0, 12, sizeof(char), KEY_UINT_8, NULL,

    /* Radical */
    BUSHU_TREE, NULL, index_bushu_read,
    offsetof(DictEntry,bushu), 0, 12, sizeof(Bushu), NUMITEMS(bushu_key),
    bushu_key,
    
    /* SKIP code */
    SKIP_TREE, NULL, index_int_read,
    offsetof(DictEntry,skip), 0, 12, sizeof(unsigned), KEY_UINT_32, NULL,

    /* Pinyin */
    PINYIN_TREE, pinyin_init, index_pinyin_read,
    offsetof(DictEntry,pinyin), 0, 13, MAXPINYIN, KEY_CASECHR_8, NULL,

    /* English translation index */
    D_ENGLISH_TREE, text_init, index_text_read,
    offsetof(DictEntry,english), 0, 13, 0, KEY_CASECHR_8, NULL,

    /* Kanji/kana text index */
    KANJI_TREE, text_init, index_text16_read,
    offsetof(DictEntry,kanji), 0, 13, 0, KEY_CHR_16, NULL,

    /* Cross-reference of kanji */
    XREF_TREE, NULL, index_xref16_read,
    offsetof(DictEntry,kanji), 0, 13, sizeof(short), KEY_UINT_16, NULL,

    /* Cross-reference of the words in translations */
    WORDS_TREE, text_init, index_words_read,
    offsetof(DictEntry,english), 0, 13, MAXWORDSIZE, KEY_CASECHR_8, NULL,

    /* Readings */
    D_YOMI_TREE, text16_init, index_yomi_read,
    offsetof(DictEntry,pronunciation), 0, 13, 0, KEY_CHR_16, NULL,

    /* Readings in romaji */
    D_ROMAJI_TREE, text16_init, index_romaji_read,
    offsetof(DictEntry,pronunciation), 0, 13, 0, KEY_CASECHR_8, NULL,

    NULL,
};

void
create_indexes(cnt, istr)
    int cnt;
    struct indexfile *istr;
{
    int i;
    Index *iptr;
    struct index_rdata foo;
    struct subkey subkey, *subkey_ptr;
    int numkeys;
    
    if (verbose)
	printf("Creating indexes: \n");
    for (i = 0; i < cnt; i++, istr++) {
	if (verbose) {
	    printf("%s:", istr->name);
	    fflush(stdout);
	}
	memset(&foo, 0, sizeof(foo));
	foo.offset = istr->offset;
	if (istr->fun)
	    istr->fun(0, &foo, istr);
	if (istr->keyspec == NULL) {
	    numkeys = 1;
	    subkey_ptr = &subkey;
	    subkey.type = istr->keytype;
	    subkey.offset = 0;
	    subkey.len = istr->keylen;
	    subkey.desc = 0;
	} else {
	    numkeys = istr->keytype;
	    subkey_ptr = istr->keyspec;
	}
	
	iptr = icreate(istr->name, istr->flags, istr->pageshift,
		       istr->keylen, numkeys, subkey_ptr, istr->read, &foo);

	if (istr->fun)
	    istr->fun(1, &foo, istr);

	if (verbose)
	    printf("\n");
	if (iptr) {
	    iclose(iptr);
	} else
	    logmsg(L_WARN, "failed to create %s", istr->name);
    }
}



