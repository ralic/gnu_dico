/* $Id: utils.c,v 1.1 2001/11/03 22:34:45 gray Exp $
 */
/* utils.c:
 * helper routines that don't really belong anywhere else
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dbs.h>
#include <makedict.h>

int 
xtoi(s)
    char *s;
{
    register int out = 0;

    if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
	s += 2;
    for (; *s; s++) {
	if (*s >= 'a' && *s <= 'f')
	    out = (out << 4) + *s - 'a' + 10;
	else if (*s >= 'A' && *s <= 'F')
	    out = (out << 4) + *s - 'A' + 10;
	else if (*s >= '0' && *s <= '9')
	    out = (out << 4) + *s - '0';
	else 
	    break;
    }
    return out;
}

XChar2b *
dup_16(kanabuffer)
    XChar2b * kanabuffer;
{
    int pronun_len;
    XChar2b *ret_str;
    
    pronun_len = strlen((char *) kanabuffer);

    ret_str = (XChar2b *) malloc(sizeof(char) * pronun_len + sizeof(XChar2b));
    if (ret_str == NULL) {
	fprintf(stderr, "Not enough memory to read in dictionary\n");
	exit(0);
    }
#ifdef NOMEMSET
    strncpy(ret_str, kanabuffer, pronun_len + sizeof(XChar2b));
#else
    memcpy(ret_str, kanabuffer, sizeof(char) * pronun_len + sizeof(XChar2b));
#endif
    return ret_str;
}


FILE *
open_compressed(dictname, pflag)
    char *dictname;
    int *pflag;
{
    FILE *fp;
    char command_string[100];
    int namelen;		/* length of filename, and flag */
    int extlen;

    if (access(dictname, R_OK) != 0) {
	return NULL;
    }
#ifdef UNCOMPRESS
    namelen = strlen(dictname);
    extlen = strlen(UNCOMPRESSEXT);
    if (strncmp(&dictname[namelen - extlen], UNCOMPRESSEXT, extlen) != 0) {
	namelen = 0;		/* flag for later on */
	fp = fopen(dictname, "r");
    } else {
	sprintf(command_string, "%s %s", UNCOMPRESS, dictname);
	fp = (FILE *) popen(command_string, "r");
    }
    *pflag = namelen;
#else
    *pflag = 0;
    fp = fopen(dictname, "r");
#endif				/* UNCOMPRESS */

    return fp;

}

Uint
log2(val)
    Uint val;
{
    register Uint i = 1;
    register Uint t = 2;

    do {
	i++;
	t <<= 1;
    } while (t < val);
    return ((t-val) < (val-(t>>1))) ? i : i-1;
}

XChar2b *
strchr16(str, c)
    XChar2b *str;
    unsigned c;
{
    XChar2b v;
    
    v.byte1 = (c & 0xff00) >> 8;
    v.byte2 = (c & 0xff);

    for (; str->byte1; str++)
	if (str->byte1 == v.byte1 && str->byte2 == v.byte2)
	    return str;
    return NULL;
}

XChar2b *
strseg16(str, delim)
    XChar2b *str;
    XChar2b *delim;
{
    unsigned c;

    for (; str->byte1; str++) {
	c = ((unsigned) str->byte1 << 8) + str->byte2; 
	if (strchr16(delim, c))
	    return str;
    }
    return NULL;
}

#define WORDDELIM " \t(){}[]&.,;:?!'`\"+-*/^%$#@~|\\<>"

#define isdelim(c) strchr(WORDDELIM, c)

char *auxiliary_words[] = {
    "a", "an", "the",
    "this", "that", "these", "those",
    "in", "to", "into",
    "from",
    "on", "up", "upon",
    "through", "thru",
    "with", "within", "without",
    "out", "of", "by",
    "for",
    "one", "one's",
    "and", "or",
    "as", "at",
    "before", "after"
};

int
auxword(str, len)
    char *str;
    int len;
{
    int i;

    for (i = 0; i < NUMITEMS(auxiliary_words); i++)
	if (strncasecmp(auxiliary_words[i], str, len) == 0)
	    return 1;
    return 0;
}

int 
single_word(text)
    char *text;
{
    while (*text && !isdelim(*text)) 
        ++text;
    return *text == 0;
} 
	
int
simplify_text(buf, text, size)
    char *buf;
    char *text;
    int size;
{
    int cnt = 0;
    int len;
    char *word;

    if (!text)
        return 0;
    
    if (single_word(text)) {
        strncpy(buf, text, size);
        buf[size-1] = 0;
        return 1;
    }	
    while (size > 0) {
	/* Skip delimiters */
	while (*text && isdelim(*text)) {
	    if (*text == '(')
		break;
	    ++text;
	}
	if (!*text)
	    break;

	/* Eliminate parenthesized words */
	if (*text == '(') {
	    while (*text && *text++ != ')') ;
	    if (!*text)
		break;
	    continue;
	}
	    
	/* Determine the word boundary */
	word = text;
	for (len = 0; word[len] && !isdelim(word[len]); len++) ;
	text += len;

	/* If aux. word -- throw it away */
	if (auxword(word, len))
	    continue;

	/* Copy word to buffer */
	cnt++;
	if (len > size)
	    len = size;
	size -= len;
	
	while (len--)
	    *buf++ = *word++;
	if (size) {
	    *buf++ = ' ';
	    size--;
	}
    }
    *buf = 0;
    return cnt;
}

int
strip_parens(buf, text, size)
    char *buf;
    char *text;
    int size;
{
    int cnt = 0;
    
    if (!text)
	return 0;
    while (size > 0 && *text) {
	if (*text == '(') {
	    while (*text && *text++ != ')') ;
	    continue;
	}
	*buf++ = *text++;
	cnt++;
	size--;
    }
    *buf++ = 0;
    return cnt;
}




