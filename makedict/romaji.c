#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <dbs.h>
#include <makedict.h>

static char *kanamap[128] = {
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",


    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",

    "", "a", "a", "i", "i", "u", "u", "e",
    "e", "o", "o", "ka", "ga", "ki", "gi", "ku",

    "gu", "ke", "ge", "ko", "go", "sa", "za", "shi",
    "zi", "su", "zu", "se", "ze", "so", "zo", "ta",

    "da", "chi", "ji", "tsu", "tsu", "zu", "te", "de",
    "to", "do", "na", "ni", "nu", "ne", "no", "ha",

    "ba", "pa", "hi", "bi", "pi", "fu", "bu", "pu",
    "he", "be", "pe", "ho", "bo", "po", "ma", "mi",

    "mu", "me", "mo", "ya", "ya", "yu", "yu", "yo",
    "yo", "ra", "ri", "ru", "re", "ro", "wa", "wa",

    "i", "u", "o", "n", "", "", "", "",
    "", "", "", "", "", "", "", "",
};

#ifndef HAS_STPCPY
char *
stpcpy(p, q)
    char *p, *q;
{
    while (*p++ = *q++) ;
    return p-1;
}
#endif

XChar2b *
kanatoromaji(kana, buf, size, endp)
    XChar2b *kana;
    char *buf;
    int size;
    char **endp;
{
    int lastchar = 0;		/* for state machine */

    while (kana->byte1 != HIRAGANA_BYTE && kana->byte1 != KATAKANA_BYTE
	    && kana->byte1 != 0x0) 
	kana++;
    do {
	if (kana->byte1 != HIRAGANA_BYTE && kana->byte1 != KATAKANA_BYTE
	    && kana->byte1 != 0x0) 
	    break;
	if (kana->byte2 > 128) {
	    kana++;
	    continue;
	}

	switch (kana->byte2) {
	case 0x25: /* u */         
	case 0x26: /* U */
	    buf = stpcpy(buf, kanamap[lastchar]);
	    if (kanamap[lastchar][strlen(kanamap[lastchar])-1] == 'o')
		*buf++ = 'o';
	    break;

	case 0x63:
	case 0x65:
	case 0x67:
	    /* small ya,yu,yo: */
	    /* Put the appropriate prefix */
	    /* Later, will append "a", "u", or "o" */

	    switch (lastchar) {
	    case 0x2d:		/*ki*/
		buf = stpcpy(buf, "ky");
		break;
	    case 0x2e:		/*gi*/
		buf = stpcpy(buf, "gy");
		break;
	    case 0x37:		/*shi*/
		buf = stpcpy(buf, "sh");
		break;
	    case 0x38:		/* shi''*/
		buf = stpcpy(buf, "j");
		break;
	    case 0x41:		/*chi*/
		buf = stpcpy(buf, "ch");
		break;
	    case 0x4b:		/*ni*/
		buf = stpcpy(buf, "ny");
		break;
	    case 0x52:		/*hi*/
		buf = stpcpy(buf, "hy");
		break;
	    case 0x53:		/*bi*/
		buf = stpcpy(buf, "by");
		break;
	    case 0x54:		/*pi*/
		buf = stpcpy(buf, "py");
		break;
	    case 0x5f:		/*mi*/
		buf = stpcpy(buf, "my");
		break;
	    case 0x6a:		/*ri*/
		buf = stpcpy(buf, "ry");
		break;
	    default:
		/* oh well.. just print it as-is */
		/* but mark it as small */
		buf = stpcpy(buf, "_y");
		break;
	    }
	    switch (kana->byte2) {
	    case 0x63:
		buf = stpcpy(buf, "a");
		break;
	    case 0x65:
		buf = stpcpy(buf, "u");
		break;
	    case 0x67:
		buf = stpcpy(buf, "o");
		break;
		/* must be ya,yu,yo */
	    }
	    break;

	default:
	    switch (lastchar) {
	    case 0:
		break;
	    case 0x43: /* small tsu */         
		switch (kanamap[kana->byte2][0]) {
		case 'k':
		case 's':
		case 't':
		case 'p':
		    *buf++ = kanamap[kana->byte2][0];
		    break;
		case 'c': /* chi */
		    *buf++ = 't';
		default:
		    buf = stpcpy(buf, kanamap[lastchar]);
		}
		break;
	    case 0x73: /* n */
		switch (kanamap[kana->byte2][0]) {
		case 'm':
		case 'b':
		case 'p':
		    *buf++ = kanamap[kana->byte2][0];
		default:
		    buf = stpcpy(buf, kanamap[lastchar]);
		}
		break;
	    default:
		/* print previous char!... */
		/* then save THIS char,
		 * and continue through loop..
		 */
		buf = stpcpy(buf, kanamap[lastchar]);
	    }
	}
	lastchar = kana->byte2;
	kana++;
    } while (kana->byte1 != 0);
    buf = stpcpy(buf, kanamap[lastchar]);
    buf[0] = ' ';
    if (endp)
	*endp = buf;
    return kana;
}
