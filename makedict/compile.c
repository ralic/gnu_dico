/* $Id: compile.c,v 1.1 2001/11/03 22:34:45 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <errno.h>
#include <stdio.h>	       
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define obstack_chunk_alloc emalloc
#define obstack_chunk_free efree
#include <obstack.h>
#include <dbs.h>
#include <makedict.h>

int numberofkanji = 0, highestkanji = 0, lowestkanji = 0;
int nedict = 0; /* number of edict entiries */

int maxstr8 = 0;
int maxstr16 = 0;

int maxbuflen8 = 0;
int maxbuflen16 = 0;

void compile_kanjidic(void);
void compile_edict(void);
void setmaxlen_8(char *);
void setmaxlen_16(XChar2b *);
void setmaxlen_yomi(XChar2b *);

char * alloc_text_buffer(int length);

int
compile()
{
    open_output();
    compile_kanjidic();
    create_indexes(NUM_FIRST_INDEX, indexes);
    reposition();
    compile_edict();
    create_indexes(NUM_SECOND_INDEX, indexes+TreeEnglish);
    set_xref();
    write_statistics();
    close_output();
    
    if (verbose) {
	printf("%d kanji (from %4X to %4X)\n%d dictionary entries\n",
	       numberofkanji, lowestkanji, highestkanji, nedict);
    }
		 
    return 0;
}

#define isws(c) (c == ' ' || c == '\t')

#define TOK_ENGLISH    256
#define TOK_KANA       257
#define TOK_KANJI      258
#define TOK_NUMBER     259
#define TOK_EOL        '\n'

struct dict_token {
	int type;
	int length;
	unsigned char string[MAX_TOKEN_SIZE];
};

static FILE *infile;
static char *filename;
static int line_num;
static struct dict_token token;
static char *text_buffer;
static int text_buffer_length = 1024;

char *
alloc_text_buffer(length)
    int length;
{
    if (text_buffer == NULL) {
	if (length > text_buffer_length)
	    text_buffer_length = length;
	text_buffer = malloc(text_buffer_length);
    } else if (length > text_buffer_length) {
	char *p;

	p = realloc(text_buffer, length);
	if (!p)
	    return NULL;
	text_buffer_length = length;
	text_buffer = p;
    }
    return text_buffer;
}


void
skip_to_eol()
{
    int c;
    
    while ((c = getc(infile)) != EOF) {
	if (c == '\n') {
	    ungetc(c, infile);
	    return;
	}
    }
}

void
skip_to_delim(str)
    char *str;
{
    int c;
    
    while ((c = getc(infile)) != EOF) {
	if (strchr(str, c) == 0) {
	    ungetc(c, infile);
	    return;
	}
    }
}

void
copy_to(firstc, term)
    int firstc;
    int term;
{
    unsigned char *p;
    int c;
    
    p = token.string;
    if (firstc)
	*p++ = firstc;
    while ((c = getc(infile)) != EOF && c != term) {
	if (p - token.string >= sizeof(token.string)-1) {
	    char delim[] = "?  \t\n";
	    fprintf(stderr, "%s:%d: token too long\n",
		    filename, line_num);
	    delim[0] = term;
	    skip_to_delim(delim);
	    break;
	}
	*p++ = c;
    }
    *p = 0;
    token.length = p - token.string;
}

void
copy_kana(firstc)
    int firstc;
{
    int c1, c2;
    unsigned char *p;
    int okurigana = 0;
    int done = 0;
    
    p = token.string;
    c1 = firstc;

    do {
	if (p >= token.string + sizeof(token.string) - 3) {
	    fprintf(stderr, "%s:%d: token too long\n",
		    filename, line_num);
	    skip_to_delim(" {\n");
	    break;
	}

	switch (c1) {
	case '.':
	    okurigana++;
	    *p++ = 0x21;
	    *p++ = 0x4a;
	    break;
	case '-':
	    *p++ = 0x21;
	    *p++ = 0x41;
	    continue;
	    
	case ' ':
	    done++;
	    break;

	default:
	    if (c1 < 127) {
		if (okurigana) {
		    /* FIXME: emit warning? */
		}
		done++;
	    } else { 
		c2 = getc(infile);
		*p++ = c1 & 0x7f;
		*p++ = c2 & 0x7f;
	    }
	    break;
	}

    } while (!done && (c1 = getc(infile)) != EOF);

    if (okurigana) {
	/* add closing parenthesis */
	*p++ = 0x21;
	*p++ = 0x4b;
    }
    
    token.length = p - token.string;

    *p++ = 0;
    *p = 0;
}

int
nextkn()
{
    int c;
    
 again:
    while ((c = getc(infile)) != EOF && isws(c))
	;

    if (c == EOF)
	return token.type = 0;
	
    if (c == '#') {
	skip_to_eol();
	goto again;
    } else if (c == '\n') {
	return token.type = '\n';
    } else if (c == '{') {
	copy_to(0, '}');
	return token.type = TOK_ENGLISH;
    } else if (c > 127) {
	copy_kana(c);
	return token.type = TOK_KANA;
    } else {
	copy_to(c, ' ');
	if (isdigit(c))
	    return token.type = TOK_NUMBER;
	else
	    return token.type = token.string[0];
    }
    /*NOTREACHED*/
}

int
edict_nextkn()
{
    int c;
    
 again:
    while ((c = getc(infile)) != EOF && isws(c))
	;

    if (c == EOF)
	return token.type = 0;
	
    if (c == '#') {
	skip_to_eol();
	goto again;
    } else if (c == '\n') {
	return token.type = '\n';
    } else if (c == '[') {
	char *p;
	
	copy_to(0, ']');
	/* convert to JIS */
	for (p = token.string; *p; p++)
	    *p &= 0x7f;
	return token.type = TOK_KANA;
    } else if (c > 127) {
	copy_kana(c);
	return token.type = TOK_KANJI;
    } else if (c == '/') {
	if ((c = getc(infile)) == EOF)
	    return token.type = 0;
	else if (c == '\n')
	    return token.type = TOK_EOL;
	
	copy_to(c, '/');
	ungetc('/', infile);
	return token.type = TOK_ENGLISH;
    } else {
	copy_to(c, '\n');
	return token.type = token.string[0];
    }
    /*NOTREACHED*/
}

enum text_type {
    TEXT_ENGLISH=1,
    TEXT_YOMI,
    TEXT_PINYIN
};

struct text_hdr {
    enum text_type type;
    int length;
};


void 
compile_kanjidic()
{
    DictEntry entry;
    struct obstack stk;
    int length, english_length, yomi_length, pinyin_length;
    struct text_hdr *text_chain, *textp, text;
    char *buffer, *p;
    int kanji;
    int num;
    XChar2b buff[2];
    int pflag;
    
    infile = open_compressed(kanjidict, &pflag);
    if (infile == NULL) {
	fprintf(stderr, "cannot open kanjidic file `%s'. Skipping.\n",
		kanjidict);
	return;
    }
    if (verbose)
	printf("processing dictionary %s\n", kanjidict);
    filename = kanjidict;
    line_num = 0;
    
    obstack_init(&stk);
    while (nextkn() != 0) {
	line_num++;

	if (token.type == TOK_EOL)
	    continue;
	
	if (token.type != TOK_KANA || token.length != 2) {
	    fprintf(stderr, "%s:%d: unrecognized line\n",
		    kanjidict, line_num);
	    skip_to_eol();
	    continue;
	}
	
	if (nextkn() != TOK_NUMBER) {
	    fprintf(stderr, "%s:%d: expected JIS code but found `%s'\n",
		    kanjidict, line_num, token.string);
	    skip_to_eol();
	    continue;
	}
		
	kanji = strtol(token.string, &p, 16);
	if (!isws(*p) && *p != 0) {
	    fprintf(stderr, "%s:%d: unrecognized line\n",
		    kanjidict, line_num);
	    skip_to_eol();
	    continue;
	}    

	memset(&entry, 0, sizeof(entry));
	english_length = pinyin_length = yomi_length = 0;

	while (nextkn() != TOK_EOL && token.type != 0) {
	    switch (token.type) {
	    case 'F':
		entry.frequency = atoi(token.string + 1);
		break;
	    case 'G':
		entry.grade_level = atoi(token.string + 1);
		break;
	    case 'H':
		entry.Hindex = atoi(token.string + 1);
		break;
	    case 'N':
		entry.Nindex = atoi(token.string + 1);
		break;
	    case 'Q':
		entry.Qindex = atoi(token.string + 1);
		break;
	    case 'U':
		entry.Uindex = xtoi(token.string + 1);
		break;
	    case 'B':
		entry.bushu = atoi(token.string + 1);
		break;
	    case 'S':
		entry.numstrokes = atoi(token.string + 1);
		break;
	    case 'Y':
		text.type = TEXT_PINYIN;
		text.length = token.length-1;
		obstack_grow(&stk, &text, sizeof(text));
		obstack_grow(&stk, token.string+1, text.length);
		pinyin_length += text.length + 1;
		break;
	    case 'P': /* skip code */
		p = token.string;
		num = strtol(p+1, &p, 10);
		if (*p != '-')
		    break;
		num <<= 8;
		num |= strtol(p+1, &p, 10);
		if (*p != '-')
		    break;
		num <<= 8;
		num |= strtol(p+1, &p, 10);
		entry.skip = num;
		break;
	    case TOK_ENGLISH:
		text.type = TEXT_ENGLISH;
		text.length = token.length;
		obstack_grow(&stk, &text, sizeof(text));
		obstack_grow(&stk, token.string, token.length);
		english_length += text.length + 1;
		break;
	    case TOK_KANA:
		text.type = TEXT_YOMI;
		text.length = token.length;
		obstack_grow(&stk, &text, sizeof(text));
		obstack_grow(&stk, token.string, token.length);
		yomi_length += text.length + 2;
		break;
	    }
	}		

	if (lowestkanji == highestkanji && highestkanji == 0) {
	    lowestkanji = highestkanji = kanji;
	} else {
	    if (kanji < lowestkanji)
		lowestkanji = kanji;
	    if (kanji > highestkanji)
		highestkanji = kanji;
	}


	/* Finish collecting text blocks */
	text.type = 0;
	obstack_grow(&stk, &text, sizeof(text));
	text_chain = obstack_finish(&stk);

	/* Compute the maximum buffer length */
	length = (english_length > pinyin_length) ?
	                english_length : pinyin_length;
	if (length < yomi_length)
	    length = yomi_length;

	buffer = alloc_text_buffer(length);
	if (!buffer) {
	    fprintf(stderr,
		    "%s:%d: not enough core (trying to allocate %d bytes)\n",
		    kanjidict, line_num, length);
	    obstack_free(&stk, text_chain);
	    continue;
	}
	
	/* Finally, collect and save text blocks */
	/* 1. English */
	textp = text_chain;
	p = buffer;
	while (textp->type != 0) {
	    if (textp->type == TEXT_ENGLISH) {
		*p++ = '|';
		strncpy(p, (char*)(textp+1), textp->length);
		p += textp->length;
	    }
	    textp = (struct text_hdr *)((char*)(textp + 1) + textp->length);
	}
	*p = 0;
	setmaxlen_8(buffer+1);
	entry.english = WriteText(buffer+1);
	
	/* 2. Pinyin */
	textp = text_chain;
	p = buffer;
	while (textp->type != 0) {
	    if (textp->type == TEXT_PINYIN) {
		*p++ = '|';
		strncpy(p, (char*)(textp+1), textp->length);
		p += textp->length;
	    }
	    textp = (struct text_hdr *)((char*)(textp + 1) + textp->length);
	}
	*p = 0;
	setmaxlen_8(buffer+1);
	entry.pinyin = WriteText(buffer+1);

	/* 3. Yomi */
	textp = text_chain;
	p = buffer;
	while (textp->type != 0) {
	    if (textp->type == TEXT_YOMI) {
		*p++ = 0x21;
		*p++ = 0x21;
		strncpy(p, (char*)(textp+1), textp->length);
		p += textp->length;
	    }
	    textp = (struct text_hdr *)((char*)(textp + 1) + textp->length);
	}
	*p++ = 0;
	*p = 0;
	setmaxlen_yomi((XChar2b*)buffer+1);
	entry.pronunciation = WriteText16((XChar2b*)buffer+1);

	buff[0].byte1 = (kanji & 0xff00) >> 8;
	buff[0].byte2 = (kanji & 0xff);
	buff[1].byte1 = 0;
	buff[1].byte2 = 0;
	entry.kanji = WriteText16(buff);

	entry.Jindex = kanji;
	
	WriteDictEntry(&entry);
	numberofkanji++;
	if (verbose && numberofkanji % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}

	obstack_free(&stk, text_chain);

    }			      
    if (verbose)
	puts("");
    
    if (pflag)
	pclose(infile);
    else
	fclose(infile);

    obstack_free(&stk, NULL);
}

void
compile_edict()
{
    struct obstack stk;
    char *english_text;
    DictEntry entry;
    int pflag;
    
    infile = open_compressed(edict, &pflag);
    if (infile == NULL) {
	fprintf(stderr,
		"cannot open edict file `%s'\n",
		edict);
	return;
    }
    if (verbose)
	printf("processing dictionary %s\n", edict);

    line_num = 0;
    filename = edict;
    
    obstack_init(&stk);
    while (edict_nextkn()) {
	line_num++;

	if (token.type == TOK_EOL)
	    continue;

	if (token.type != TOK_KANJI) {
	    fprintf(stderr, "%s:%d: unrecognized line\n",
		    filename, line_num);
	    skip_to_eol();
	    continue;
	}

	if (strcmp(token.string, "\x21\x29\x21\x29\x21\x29\x21\x29") == 0) {
	    /* copyright mark: "？？？？" */
	    skip_to_eol();
	    continue;
	}
	
	if (verbose && nedict % 1000 == 0) {
	    putchar('.');
	    fflush(stdout);
	}

	memset(&entry, 0, sizeof(entry));

	entry.kanji = WriteText16((XChar2b*)token.string);
	if (edict_nextkn() == TOK_KANA) {
	    entry.pronunciation = WriteText16((XChar2b*)token.string);
	    setmaxlen_16((XChar2b*)token.string);
	} else {
	    entry.pronunciation = entry.kanji;
	    
	    if (token.type == TOK_ENGLISH) {
		obstack_1grow(&stk, '|');
		obstack_grow(&stk, token.string, token.length);
	    } else {
		fprintf(stderr, "%s:%d: unrecognized string\n",
			filename, line_num);
	    }
	}
 	
	while (edict_nextkn() == TOK_ENGLISH) {
	    obstack_1grow(&stk, '|');
	    obstack_grow(&stk, token.string, token.length);
	}

	if (token.type != TOK_EOL && token.type != 0) {
	    fprintf(stderr,
		    "%s:%d: trash after end of line\n",
		    filename, line_num);
	    skip_to_eol();
	}
	obstack_1grow(&stk, 0);
	english_text = obstack_finish(&stk);
	entry.english = WriteText(english_text+1);
	setmaxlen_8(english_text+1);
	obstack_free(&stk, english_text);
	nedict++;
	WriteDictEntry(&entry);
    }

    obstack_free(&stk, NULL);
    if (pflag)
	pclose(infile);
    else
	fclose(infile);

    if (verbose)
	puts("");
    
}

void
setmaxlen_8(str)
    char *str;
{
    char *p;
    int len;

    len = strlen(str) + 1;
    if (len > maxbuflen8)
	maxbuflen8 = len;
    while (p = strchr(str, '|')) {
	len = p - str + 1;
	if (len > maxstr8)
	    maxstr8 = len;
	str = p+1;
    }
    len = strlen(str) + 1;
    if (len > maxstr8)
	maxstr8 = len;
}

void
setmaxlen_yomi(str)
    XChar2b *str;
{
    XChar2b *p;
    int len;
    
    len = strlen((char*)str) + sizeof(XChar2b);
    if (len > maxbuflen16)
	maxbuflen16 = len;
    
    while (p = strchr16(str, 0x2121)) {
	len = sizeof(XChar2b)*(p - str + 1);
	if (len > maxstr16)
	    maxstr16 = len;
	str = p+1;
    }
    len = strlen((char*)str) + sizeof(XChar2b);
    if (len > maxstr16)
	maxstr16 = len;
}

void
setmaxlen_16(str)
    XChar2b *str;
{
    int len;

    len = strlen((char*)str) + sizeof(XChar2b);
    if (len > maxbuflen16)
	maxbuflen16 = len;
    if (len > maxstr16)
	maxstr16 = len;
}

   
