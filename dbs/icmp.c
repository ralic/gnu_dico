/* $Id: icmp.c,v 1.1 2001/11/03 22:33:36 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <ctype.h>
#include <string.h>
#include <dbs.h>

static int
compare_error()
{
    internal_error("zero subkey type!");
    /*NOTREACHED*/
}

static int
compare_int_8(one, two)
    char *one;
    char *two;
{
    return (int)*one - *two;
}

static int
compare_uint_8(one, two)
    unsigned char *one;
    unsigned char *two;
{
    return *one - *two;
}

static int
compare_int_16(one, two)
    short *one;
    short *two;
{
    if (*one > *two)
	return 1;
    else if (*one < *two)
	return -1;
    return 0;
}

static int
compare_uint_16(one, two)
    unsigned short *one;
    unsigned short *two;
{
    return *one - *two;
}

static int
compare_int_32(one, two)
    long *one;
    long *two;
{
    if (*one > *two)
	return 1;
    else if (*one < *two)
	return -1;
    return 0;
}

static int
compare_uint_32(one, two)
    unsigned long *one;
    unsigned long *two;
{
    return *one - *two;
}

static int
compare_chr_8(one, two, len)
    char *one;
    char *two;
    int len;
{
    return strncmp(one, two, len);
}

static int
compare_casechr_8(one, two, len)
    char *one;
    char *two;
    int len;
{
    return strncasecmp(one, two, len);
}

static int
compare_chr_16(one, two, len)
    unsigned short *one;
    unsigned short *two;
    int len;
{
    return memcmp(one, two, len);
}

static int (*compare[])() = {
    compare_error,
    compare_int_8,
    compare_uint_8,
    compare_int_16,
    compare_uint_16,
    compare_int_32,
    compare_uint_32,
    compare_chr_8,     
    compare_casechr_8, 
    compare_chr_16,     
};

int
icmp(iptr, search_value, key_value)
    Index *iptr;
    char *search_value, *key_value;
{
    int i, num = iptr->numsubkeys;
    struct subkey *subkey = iptr->subkey;
    int result;

    if (iptr->idxflags & IF_ALLOWINEXACT) {
	num = iptr->numcompkeys ? iptr->numcompkeys : iptr->numsubkeys;
	for (i = 0, result = 0; result == 0 && i < num; i++, subkey++) {
	    result = compare[subkey->type](search_value+subkey->offset,
					   key_value+subkey->offset,
					   subkey->cmplen);
	    if (subkey->desc)
		result = - result;
	}
    } else {
	num = iptr->numsubkeys;
	for (i = 0, result = 0; result == 0 && i < num; i++, subkey++) {
	    result = compare[subkey->type](search_value+subkey->offset,
					   key_value+subkey->offset,
					   subkey->len);
	    if (subkey->desc)
		result = - result;
	}
    }
    return result;
}
	
