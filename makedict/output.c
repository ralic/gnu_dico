/* $Id: output.c,v 1.1 2001/11/03 22:34:51 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <dbs.h>
#include <makedict.h>

Data *dictfile;
Data *textfile;

char *page;
Offset page_num;
Offset page_offset;

Offset write_to_page(void *data, int size);
void dump_page();
    
void
open_output()
{
    dictfile =  dcreate(dictname, _BUF_LRU|_BUF_WRITE, sizeof(DictEntry),
			0, 0, NULL, NULL);
    if (!dictfile)
	die("Cannot open `%s': %s", dictname, strerror(errno));
    if (verbose)
	printf("Output dictionary: %s\n", dictname);
    
    textfile =  dcreate(textname, _BUF_LRU|_BUF_WRITE, textpagesize,
			textpageshift, 0, NULL, NULL);
    if (!textfile)
	die("Cannot open `%s': %s", textname, strerror(errno));
    if (verbose)
	printf("Output text storage: %s. Pagesize %d\n",
	       textname, textpagesize);

    /* Allocate a page */
    page = malloc(textpagesize);
    if (!page)
	die("%s:%d: Not enough core", __FILE__, __LINE__);
    page_num = 0;
    page_offset = 1; /* offset 0 is reserved */
}

void
reposition()
{
    dseek(dictfile, 0, SEEK_END);
    dseek(textfile, 0, SEEK_END);
}

void
close_output()
{
    dclose(dictfile);

    dump_page();
    dclose(textfile);
    free(page);
}

void
write_statistics()
{
    Dictheader header;

    header.numkanji = numberofkanji;
    header.numentries = nedict;
    header.lowestkanji = lowestkanji;
    header.highestkanji = highestkanji;
    header.maxlen8 = maxbuflen8;
    header.maxlen16 = maxbuflen16;
    dwritepriv(dictfile, &header, sizeof(header));                
}

char *
get_text_ptr(off)
    Offset off;
{
    long recno;
    int pos;
    char *ptr;

    if (off == 0)
	return NULL;
    recno = off / textfile->pagesize;
    pos = off % textfile->pagesize;
    if (recno < textfile->reccnt) {
	dseek(textfile, recno, SEEK_SET);
	dread(textfile, NULL);
	ptr = textfile->head+pos;
    } else {
	ptr = page + pos;
    }
    return ptr;
}

int
get_text8(off, buf, buflen)
    Offset off;
    char *buf;
    int buflen;
{
    int length;
    char *ptr;

    if (off == 0)
	return 0;
    ptr = get_text_ptr(off);
    length = strlen(ptr)+1;
    if (length > buflen)
	length = buflen;
    memcpy(buf, ptr, length);
    buf[buflen-1] = 0;
    return length;
}

void
WriteDictEntry(trans)
    DictEntry *trans;
{
    long recno;
    
    if (dwrite(dictfile, trans))
	die("Write error on `%s': %s", dictname, strerror(errno));
    recno = dseek(dictfile, 1, SEEK_CUR);
}

Offset
WriteText(text)
    char *text;
{
    int len = strlen(text);
    return len ? write_to_page(text, len+1) : 0;
}
		      
Offset
WriteText16(text)
    XChar2b *text;
{
    int cnt;

    for (cnt = 0; !(text[cnt].byte1 == 0 && text[cnt].byte2 == 0); cnt++) ;
    return cnt ? write_to_page(text, (cnt+1) * sizeof(XChar2b)) : 0;
}


Offset
write_to_page(data, size)
    void *data;
    int size;
{
    Offset ret;

    if (textpagesize - page_offset < size) 
	dump_page();
    ret = page_num * textpagesize + page_offset;
    memcpy(page + page_offset, data, size);
    page_offset += size;
    return ret;
}

void
dump_page()
{
    dseek(textfile, 0, SEEK_END);
    if (dwrite(textfile, page))
	die("Write error on `%s': %s", textname, strerror(errno));
    memset(page, 0, textpagesize);
    page_offset = 0;
    page_num++;
}

int
index_char_read(foo, phys_recno, log_recno, buffer, len)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int len;
{
    DictEntry trans;

    *log_recno = phys_recno;
    dseek(dictfile, phys_recno, SEEK_SET);
    if (deof(dictfile))
	return -1;
    if (dread(dictfile, &trans))
	return -1;
    *(char*)buffer = *((char*)&trans+foo->offset);
    return 0;
}

int
index_short_read(foo, phys_recno, log_recno, buffer, len)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int len;
{
    DictEntry trans;

    *log_recno = phys_recno;
    dseek(dictfile, phys_recno, SEEK_SET);
    if (deof(dictfile))
	return -1;
    if (dread(dictfile, &trans))
	return -1;
    *(short*)buffer = *(short*)((char*)&trans+foo->offset);
    return 0;
}

int
index_int_read(foo, phys_recno, log_recno, buffer, len)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int len;
{
    DictEntry trans;

    *log_recno = phys_recno;
    dseek(dictfile, phys_recno, SEEK_SET);
    if (deof(dictfile))
	return -1;
    if (dread(dictfile, &trans))
	return -1;
    *(int*)buffer = *(int*)((char*)&trans+foo->offset);
    return 0;
}

int
index_bushu_read(foo, phys_recno, log_recno, buffer, len)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int len;
{
    DictEntry trans;
    Bushu *bp;
    
    *log_recno = phys_recno;
    dseek(dictfile, phys_recno, SEEK_SET);
    if (deof(dictfile))
	return -1;
    if (dread(dictfile, &trans))
	return -1;
    bp = (Bushu*)buffer;
    bp->bushu = trans.bushu;
    bp->numstrokes = trans.numstrokes;
    return 0;
}


int
index_text_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    char *p, *start;
    int length;
    
    while (*foo->curp && isspace(*foo->curp))
	++foo->curp;
    if (!*foo->curp) {
    again:
	phys_recno = ++*log_recno;
	if (verbose && phys_recno % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}
	dseek(dictfile, phys_recno, SEEK_SET);
	if (deof(dictfile))
	    return -1;
	if (dread(dictfile, &foo->trans))
	    return -1;

	if (strip_parens(foo->buf,
			  get_text_ptr(*(Offset*)((char*)&foo->trans+
						  foo->offset)),
			  foo->buflen) == 0)
	    goto again;
	foo->curp = foo->buf;
	while (*foo->curp && isspace(*foo->curp))
	    ++foo->curp;
    }

    p = strchr(foo->curp, '|');
    if (p) {
	start = foo->curp;
	foo->curp = p+1;
	while (p > start && isspace(p[-1]))
	    p--;
	*p = 0;
    } else {
	start = foo->curp;
	foo->curp += strlen(foo->curp);
    }
    length = strlen(start);
    if (length > buflen)
	length = buflen;
    memset(buffer, ' ', buflen);
    memcpy(buffer, start, length);
    return 0;
}

int
index_words_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    char *p, *start;
    int length;

    while (*foo->curp && isspace(*foo->curp))
	++foo->curp;
    if (!*foo->curp) {
    again:
	phys_recno = ++*log_recno;
	if (verbose && phys_recno % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}
	dseek(dictfile, phys_recno, SEEK_SET);
	if (deof(dictfile))
	    return -1;
	if (dread(dictfile, &foo->trans))
	    return -1;

	if (simplify_text(foo->buf,
			  get_text_ptr(*(Offset*)((char*)&foo->trans+
						  foo->offset)),
			  foo->buflen) == 0)
	    goto again;
	foo->curp = foo->buf;
	while (*foo->curp && isspace(*foo->curp))
	    ++foo->curp;
    }

    p = strchr(foo->curp, ' ');
    if (p) {
	start = foo->curp;
	foo->curp = p+1;
	*p = 0; 
    } else {
	start = foo->curp;
	foo->curp += strlen(foo->curp);
    }

    length = strlen(start);
    if (length > buflen)
	length = buflen;
    memset(buffer, ' ', buflen);
    memcpy(buffer, start, length);
    return 0;
}

int
index_yomi_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    char *p, *start;
    int length;
    static XChar2b delim[] = {
	0x21, 0x21, /* blank space */
	0x21, 0x4a, /* open paren */
	0, 0,
    };

    memset(buffer, 0x21, buflen);
    if (foo->stop) {
	if (foo->stop[0] == 0x21 && foo->stop[1] == 0x4a) {
	    /* okurigana */
	    int length = foo->stop - foo->start;
	    memcpy(buffer, foo->start, length);
	    p = buffer + length;
	    while (*foo->curp && !(foo->curp[0] == 0x21 && foo->curp[1] == 0x4b)){
		*p++ = *foo->curp++;
		*p++ = *foo->curp++;
	    }
	    foo->stop = foo->curp;
	    return 0;
	}
    }
    
    while (*foo->curp && *foo->curp!=HIRAGANA_BYTE && *foo->curp!=KATAKANA_BYTE)
	foo->curp += 2;
    
    if (!*foo->curp) {
    again:
	phys_recno = ++*log_recno;
	if (verbose && phys_recno % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}
	dseek(dictfile, phys_recno, SEEK_SET);
	if (deof(dictfile))
	    return -1;
	if (dread(dictfile, &foo->trans))
	    return -1;

	if (get_text8(*(Offset*)((char*)&foo->trans+foo->offset),
		      foo->buf, foo->buflen) == 0)
	    goto again;
	foo->curp = foo->buf;
	while (*foo->curp && isspace(*foo->curp))
	    ++foo->curp;
    }

    p = (char*)strseg16((XChar2b*)foo->curp, delim);
    if (p) {
	start = foo->curp;
	length = p - foo->curp;
	foo->curp = p+2;
    } else {
	start = foo->curp;
	length = strlen(foo->curp);
	foo->curp += length;
    }
    foo->start = start;
    foo->stop = p;
    if (length > buflen)
	length = buflen;
    memset(buffer, 0x21, buflen);
    memcpy(buffer, start, length);
    return 0;
}

int
index_romaji_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    char *p, *start;

    memset(buffer, ' ', buflen);
    if (foo->stop && foo->stop[0]) {
	if (foo->stop[0] == 0x21 && foo->stop[1] == 0x4a) {
	    /* okurigana */
	    kanatoromaji((XChar2b*)foo->start, buffer, buflen, &p);
	    kanatoromaji((XChar2b*)foo->stop, p, buflen-(p-(char*)buffer), NULL);
 	    foo->stop = NULL;
	    return 0;
	}
	foo->stop = NULL;
    }
    
    while (*foo->curp && *foo->curp!=HIRAGANA_BYTE && *foo->curp!=KATAKANA_BYTE)
	foo->curp += 2;
    
    if (!*foo->curp) {
    again:
	phys_recno = ++*log_recno;
	if (verbose && phys_recno % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}
	dseek(dictfile, phys_recno, SEEK_SET);
	if (deof(dictfile))
	    return -1;
	if (dread(dictfile, &foo->trans))
	    return -1;

	if (get_text8(*(Offset*)((char*)&foo->trans+foo->offset),
		      foo->buf, foo->buflen) == 0)
	    goto again;
	foo->curp = foo->buf;
	while (*foo->curp && isspace(*foo->curp))
	    ++foo->curp;
    }

    p = (char*)strchr16((XChar2b*)foo->curp, 0x2121);
    if (p) {
	start = foo->curp;
	foo->curp = p+2;
	*p = 0;
    } else {
	start = foo->curp;
	foo->curp += strlen(foo->curp);
    }
    foo->start = start;
    foo->stop = (char*)kanatoromaji((XChar2b*)start, buffer, buflen, &p);
    if (foo->stop[0] == 0x21 && foo->stop[1] == 0x41) {
	kanatoromaji((XChar2b*)foo->stop, p, buflen-(p-(char*)buffer), &p);
	foo->stop = NULL;
    }
    return 0;
}


int
index_text16_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    XChar2b *text;
    int len;
    
    do {
	phys_recno = ++*log_recno;
	if (verbose && phys_recno % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}
	dseek(dictfile, phys_recno, SEEK_SET);
	if (deof(dictfile))
	    return -1;
	if (dread(dictfile, &foo->trans))
	    return -1;
	
	text=(XChar2b*)get_text_ptr(*(Offset*)((char*)&foo->trans+
					       foo->offset));
    } while (!text);
    
    for (len = 0; !(text[len].byte1 == 0 && text[len].byte2 == 0); len++) ;
    len = (len+1)*sizeof(XChar2b);
    if (len > buflen)
	len = buflen;
    memset(buffer, ' ', buflen);
    memcpy(buffer, text, len);
    return 0;
}

int
index_xref16_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    XChar2b *text;

    if (!foo->curp || !*foo->curp) {
    again:
	do {
	    phys_recno = ++*log_recno;
	    if (verbose && phys_recno % 1000 == 0) {
		putchar('.');
		fflush(stdout);
	    }
	    dseek(dictfile, phys_recno, SEEK_SET);
	    if (deof(dictfile))
		return -1;
	    if (dread(dictfile, &foo->trans))
		return -1;
	
	    text=(XChar2b*)get_text_ptr(*(Offset*)((char*)&foo->trans+
						   foo->offset));
	} while (!text);
	foo->curp = (char*)text;
    }

    text = (XChar2b*)foo->curp;
    while (text->byte1 && !(text->byte1 >= lowestkanji >> 8 &&
			    text->byte1 <= highestkanji << 8))
	text++;
    if (!text->byte1)
	goto again;
/*    memcpy(buffer, text, buflen);*/
    *(unsigned short*)buffer = (unsigned short)(text->byte1<<8) + text->byte2;
    foo->curp = (char*)(text+1);
    return 0;
}


int
index_pinyin_read(foo, phys_recno, log_recno, buffer, buflen)
    struct index_rdata *foo;
    long phys_recno;
    long *log_recno;
    void *buffer;
    int buflen;
{
    char *p, *start;
    int length;

    if (!foo->start) {
	/* Process full pinyin */
	while (*foo->curp && isspace(*foo->curp))
	    ++foo->curp;
	if (!*foo->curp) {
	again:
	    phys_recno = ++*log_recno;
	    if (verbose && phys_recno % 1000 == 0) {
		putchar('.');
		fflush(stdout);
	    }
	    dseek(dictfile, phys_recno, SEEK_SET);
	    if (deof(dictfile))
		return -1;
	    if (dread(dictfile, &foo->trans))
		return -1;

	    if (get_text8(*(Offset*)((char*)&foo->trans+foo->offset),
			  foo->buf, foo->buflen) == 0)
		goto again;
	    foo->curp = foo->buf;
	    while (*foo->curp && isspace(*foo->curp))
		++foo->curp;
	}

	p = strchr(foo->curp, '|');
	if (p) {
	    start = foo->curp;
	    foo->curp = p+1;
	    while (p > start && isspace(p[-1]))
		p--;
	    *p = 0;
	} else {
	    start = foo->curp;
	    foo->curp += strlen(foo->curp);
	}
	length = strlen(start);
	if (length > buflen)
	    length = buflen;
	foo->start = start;
    } else {
	/* strip away the tone */
	start = foo->start;
	length = strlen(start);
	if (isdigit(start[length-1]))
	    start[--length] = 0;
	foo->start = 0;
    }
    memset(buffer, ' ', buflen);
    memcpy(buffer, start, length);
    return 0;
}

int
text_init(cmd, foo, istr)
    int cmd;
    struct index_rdata *foo;
    struct indexfile *istr;
{
    switch (cmd) {
    case 0: /* init */
	if (istr->keylen == 0) 
	    istr->keylen = maxstr8;
	foo->buflen = maxbuflen8;
	foo->buf = emalloc(foo->buflen);
	foo->buf[0] = 0;
	foo->curp = foo->buf;
	foo->start = NULL;
	break;
    case '1': /* done */
	free(foo->buf);
	break;
    }
}

int
text16_init(cmd, foo, istr)
    int cmd;
    struct index_rdata *foo;
    struct indexfile *istr;
{
    switch (cmd) {
    case 0: /* init */
	if (istr->keylen == 0)
	    istr->keylen = maxstr16;
	foo->buflen = maxbuflen16;
	foo->buf = emalloc(foo->buflen);
	foo->buf[0] = foo->buf[1] = 0;
	foo->curp = foo->buf;
	foo->start = foo->stop = NULL;
	break;
    case '1': /* done */
	free(foo->buf);
	break;
    }
}

int
pinyin_init(cmd, foo, istr)
    int cmd;
    struct index_rdata *foo;
    struct indexfile *istr;
{
    switch (cmd) {
    case 0: /* init */
	foo->buflen = maxbuflen8;
	foo->buf = emalloc(foo->buflen);
	foo->buf[0] = 0;
	foo->curp = foo->buf;
	foo->start = NULL;
	break;
    case '1': /* done */
	free(foo->buf);
	break;
    }
}


void
set_xref()
{
    Index *iptr, *jis_tree;
    DictEntry trans;
    int cnt;
    Recno recno;
    unsigned short saved_key;
    Key *key;
    
    if (verbose) {
	printf("Counting cross-references:");
	fflush(stdout);
    }
    
    iptr = iopen(indexes[TreeXref].name, 0); 
    if (!iptr)
	return;

    jis_tree = iopen(indexes[TreeJis].name, 0);
    if (!jis_tree)
	return;

    if (itop(iptr) != SUCCESS) {
	logmsg(L_ERR, "itop!");
	return;
    }

    key = _blkkey(iptr);
    if (!key) {
	logmsg(L_WARN, "_blkkey() returned NULL");
	return;
    }
    saved_key = *(unsigned short*)ivalue(iptr);
    cnt = 0;
    recno = 0;
    while (iskip(iptr, 1L) == 1L) {
	if (verbose && ++recno % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}

	key = _blkkey(iptr);
	if (!key) {
	    logmsg(L_WARN, "_blkkey() returned NULL");
	    break;
	}
	if (icmp(iptr, ivalue(iptr), (char*)&saved_key) == 0) {
	    cnt++;
	} else {
	    if (iseek(jis_tree, &saved_key) == SUCCESS) {
		dseek(dictfile, irecno(jis_tree), SEEK_SET);
		dread(dictfile, &trans);
		trans.refcnt = cnt;
		dwrite(dictfile, &trans);
	    } else {
		logmsg(L_WARN, "Can't find kanji %#x", saved_key);
	    }
	    cnt = 1;
	    saved_key = *(unsigned short*) ivalue(iptr);
	}
    }
    
    iclose(iptr);
    iclose(jis_tree);

    if (verbose)
	printf("\n");
}
