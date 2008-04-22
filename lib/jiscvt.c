/* $Id$
 */
/*  Gjdict
 *  Copyright (C) 1998, 2008 Sergey Poznyakoff
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

struct cvt_tab {
    uchar in1;
    uchar in2_range[2];
    uchar out1;
    uchar out2;
};

static struct cvt_tab *tab_lookup(struct cvt_tab*, int);

#define NITEMS(t) sizeof(t)/sizeof((t)[0])
#define Escape 27

struct cvt_tab shin_to_shift_tab[] = {
    { 0x21, { 0x21, 0x60 }, 0x81, 0x40 }, /* symbols, operators, musical
					     notations */
    { 0x21, { 0x60, 0x7f }, 0x81, 0x80 }, /* .... */
    { 0x22, { 0x21, 0x2e }, 0x81, 0x9f },
    { 0x23, { 0x30, 0x39 }, 0x82, 0x4f },
    { 0x23, { 0x41, 0x5a }, 0x82, 0x60 }, /* Roman alphabet (caps) */
    { 0x23, { 0x61, 0x7a }, 0x82, 0x81 }, /* Roman alphabet */
    { 0x24, { 0x21, 0x73 }, 0x82, 0x9f }, /* Hiragana */
    { 0x25, { 0x21, 0x60 }, 0x83, 0x40 }, /* Katakana */
    { 0x25, { 0x60, 0x77 }, 0x83, 0x80 },
    { 0x26, { 0x21, 0x38 }, 0x83, 0x9f }, /* Greek alphabet (caps) */
    { 0x26, { 0x41, 0x58 }, 0x81, 0xbf }, /* Greek alphabet */
    { 0x27, { 0x21, 0x60 }, 0x84, 0x40 }, /* Russian alphabet */
    { 0x27, { 0x60, 0x72 }, 0x84, 0x80 },
    { 0x28, { 0x21, 0x40 }, 0x84, 0x9f }, /* line graphic characters */
       /* Kanji */	  	   
    { 0x30, { 0x21, 0x7e }, 0x88, 0x9F },
    { 0x31, { 0x21, 0x60 }, 0x89, 0x40 },
    { 0x31, { 0x60, 0x7e }, 0x89, 0x80 },
    { 0x32, { 0x21, 0x7e }, 0x89, 0x9f },
    { 0x33, { 0x21, 0x60 }, 0x8a, 0x40 },
    { 0x33, { 0x60, 0x7e }, 0x8a, 0x80 },
    { 0x34, { 0x21, 0x7e }, 0x8a, 0x9f },
    { 0x35, { 0x21, 0x60 }, 0x8b, 0x40 },
    { 0x35, { 0x60, 0x7e }, 0x8b, 0x80 },
    { 0x36, { 0x21, 0x7e }, 0x8b, 0x9f },
    { 0x37, { 0x21, 0x60 }, 0x8c, 0x40 },
    { 0x37, { 0x60, 0x7e }, 0x8c, 0x80 },
    { 0x38, { 0x21, 0x7e }, 0x8c, 0x9f },
    { 0x39, { 0x21, 0x60 }, 0x8d, 0x40 },
    { 0x39, { 0x60, 0x7e }, 0x8d, 0x80 },
    { 0x3a, { 0x21, 0x7e }, 0x8d, 0x9f },
    { 0x3b, { 0x21, 0x60 }, 0x8e, 0x40 },
    { 0x3b, { 0x60, 0x7e }, 0x8e, 0x80 },
    { 0x3c, { 0x21, 0x7e }, 0x8e, 0x9f },
    { 0x3d, { 0x21, 0x60 }, 0x8f, 0x40 },
    { 0x3d, { 0x60, 0x7e }, 0x8f, 0x80 },
    { 0x3e, { 0x21, 0x7e }, 0x8f, 0x9f },
    { 0x3f, { 0x21, 0x60 }, 0x90, 0x40 },
    { 0x3f, { 0x60, 0x7e }, 0x90, 0x80 },
    { 0x40, { 0x21, 0x7e }, 0x90, 0x9f },
    { 0x41, { 0x21, 0x60 }, 0x91, 0x40 },
    { 0x41, { 0x60, 0x7e }, 0x91, 0x80 },
    { 0x42, { 0x21, 0x7e }, 0x91, 0x9f },
    { 0x43, { 0x21, 0x60 }, 0x92, 0x40 },
    { 0x43, { 0x60, 0x7e }, 0x92, 0x80 },
    { 0x44, { 0x21, 0x7e }, 0x92, 0x9f },
    { 0x45, { 0x21, 0x60 }, 0x93, 0x40 },
    { 0x45, { 0x60, 0x7e }, 0x93, 0x80 },
    { 0x46, { 0x21, 0x7e }, 0x93, 0x9f },
    { 0x47, { 0x21, 0x60 }, 0x94, 0x40 },
    { 0x47, { 0x60, 0x7e }, 0x94, 0x80 },
    { 0x48, { 0x21, 0x7e }, 0x94, 0x9f },
    { 0x49, { 0x21, 0x60 }, 0x95, 0x40 },
    { 0x49, { 0x60, 0x7e }, 0x95, 0x80 },
    { 0x4a, { 0x21, 0x7e }, 0x95, 0x9f },
    { 0x4b, { 0x21, 0x60 }, 0x96, 0x40 },
    { 0x4b, { 0x60, 0x7e }, 0x96, 0x80 },
    { 0x4c, { 0x21, 0x7e }, 0x96, 0x9f },
    { 0x4d, { 0x21, 0x60 }, 0x97, 0x40 },
    { 0x4d, { 0x60, 0x7e }, 0x97, 0x80 },
    { 0x4e, { 0x21, 0x7e }, 0x97, 0x9f },
    { 0x4f, { 0x21, 0x60 }, 0x98, 0x40 },
    { 0x4f, { 0x60, 0x7e }, 0x98, 0x80 },
    { 0 }    
};

struct cvt_tab shift_to_shin_tab[] = {
    { 0x81, { 0x40, 0x7f }, 0x21, 0x21 }, /* symbols, operators, musical
	        	    			 notations */
    { 0x81, { 0x80, 0x9e }, 0x21, 0x60 },
    { 0x81, { 0x9f, 0xac }, 0x22, 0x21 },
    { 0x82, { 0x4f, 0x58 }, 0x23, 0x30 },
    { 0x82, { 0x60, 0x79 }, 0x23, 0x41 }, /* Roman alphabet (caps) */
    { 0x82, { 0x81, 0x9a }, 0x23, 0x61 }, /* Roman alphabet */
    { 0x82, { 0x9f, 0xf1 }, 0x24, 0x21 }, /* Hiragana */
    { 0x83, { 0x40, 0x7f }, 0x25, 0x21 }, /* Katakana */
    { 0x83, { 0x80, 0x96 }, 0x25, 0x60 },
    { 0x83, { 0x9f, 0xb6 }, 0x26, 0x21 }, /* Greek alphabet (caps) */
    { 0x81, { 0xbf, 0xd6 }, 0x26, 0x41 }, /* Greek alphabet */
    { 0x84, { 0x40, 0x7f }, 0x27, 0x21 }, /* Russian alphabet */
    { 0x84, { 0x80, 0x91 }, 0x27, 0x60 },
    { 0x84, { 0x9f, 0xbe }, 0x28, 0x21 }, /* line graphic characters */
         /*  Kanji: */    		   
    { 0x88, { 0x9F, 0xFC }, 0x30, 0x21 },
    { 0x89, { 0x40, 0x7f }, 0x31, 0x21 },
    { 0x89, { 0x80, 0x9e }, 0x31, 0x60 },
    { 0x89, { 0x9f, 0xfc }, 0x32, 0x21 },
    { 0x8a, { 0x40, 0x7f }, 0x33, 0x21 },
    { 0x8a, { 0x80, 0x9e }, 0x33, 0x60 },
    { 0x8a, { 0x9f, 0xfc }, 0x34, 0x21 },
    { 0x8b, { 0x40, 0x7f }, 0x35, 0x21 },
    { 0x8b, { 0x80, 0x9e }, 0x35, 0x60 },
    { 0x8b, { 0x9f, 0xfc }, 0x36, 0x21 },
    { 0x8c, { 0x40, 0x7f }, 0x37, 0x21 },
    { 0x8c, { 0x80, 0x9e }, 0x37, 0x60 },
    { 0x8c, { 0x9f, 0xfc }, 0x38, 0x21 },
    { 0x8d, { 0x40, 0x7f }, 0x39, 0x21 },
    { 0x8d, { 0x80, 0x9e }, 0x39, 0x60 },
    { 0x8d, { 0x9f, 0xfc }, 0x3a, 0x21 },
    { 0x8e, { 0x40, 0x7f }, 0x3b, 0x21 },
    { 0x8e, { 0x80, 0x9e }, 0x3b, 0x60 },
    { 0x8e, { 0x9f, 0xfc }, 0x3c, 0x21 },
    { 0x8f, { 0x40, 0x7f }, 0x3d, 0x21 },
    { 0x8f, { 0x80, 0x9e }, 0x3d, 0x60 },
    { 0x8f, { 0x9f, 0xfc }, 0x3e, 0x21 },
    { 0x90, { 0x40, 0x7f }, 0x3f, 0x21 },
    { 0x90, { 0x80, 0x9e }, 0x3f, 0x60 },
    { 0x90, { 0x9f, 0xfc }, 0x40, 0x21 },
    { 0x91, { 0x40, 0x7f }, 0x41, 0x21 },
    { 0x91, { 0x80, 0x9e }, 0x41, 0x60 },
    { 0x91, { 0x9f, 0xfc }, 0x42, 0x21 },
    { 0x92, { 0x40, 0x7f }, 0x43, 0x21 },
    { 0x92, { 0x80, 0x9e }, 0x43, 0x60 },
    { 0x92, { 0x9f, 0xfc }, 0x44, 0x21 },
    { 0x93, { 0x40, 0x7f }, 0x45, 0x21 },
    { 0x93, { 0x80, 0x9e }, 0x45, 0x60 },
    { 0x93, { 0x9f, 0xfc }, 0x46, 0x21 },
    { 0x94, { 0x40, 0x7f }, 0x47, 0x21 },
    { 0x94, { 0x80, 0x9e }, 0x47, 0x60 },
    { 0x94, { 0x9f, 0xfc }, 0x48, 0x21 },
    { 0x95, { 0x40, 0x7f }, 0x49, 0x21 },
    { 0x95, { 0x80, 0x9e }, 0x49, 0x60 },
    { 0x95, { 0x9f, 0xfc }, 0x4a, 0x21 },
    { 0x96, { 0x40, 0x7f }, 0x4b, 0x21 },
    { 0x96, { 0x80, 0x9e }, 0x4b, 0x60 },
    { 0x96, { 0x9f, 0xfc }, 0x4c, 0x21 },
    { 0x97, { 0x40, 0x7f }, 0x4d, 0x21 },
    { 0x97, { 0x80, 0x9e }, 0x4d, 0x60 },
    { 0x97, { 0x9f, 0xfc }, 0x4e, 0x21 },
    { 0x98, { 0x40, 0x7f }, 0x4f, 0x21 },
    { 0x98, { 0x80, 0x9e }, 0x4f, 0x60 },
    { 0	}
};   


struct cvt_tab *
tab_lookup(struct cvt_tab *tab, int c)
{
    for (; tab->in1 > 0; tab++)
        if (tab->in1 == c)
            return tab;
    return NULL;
}

int
convert(uchar *text, int len, struct cvt_tab *tab)
{
    int i, c, c2;
    struct cvt_tab *t;

    for (i = 0; i < len; i+=2, text+=2) {
	c = text[0];
        if (t = tab_lookup(tab, c)) {
	    c2 = text[1];
            for ( ; t->in1 == c; t++) {
                if (t->in2_range[0] <= c2 && c2 <= t->in2_range[1]) {
                    break;
                }
            }
            if (t->in1 == c) {
                c2 -= t->in2_range[0] - t->out2;
		text[0] = t->out1;
                text[1] = c2;
            } else {
		break;
            }
        } else {
	    break;
        }
    }
    return i;
}


int
shift_to_shin(char *text, int len)
{
    return convert(text, len, shift_to_shin_tab);
}

int
euc_to_shin(uchar *text, int len)
{
    int i;

    for (i = 0; i < len; i++) {
	*text++ -= 0x80;
    }
    return len;
}

int
shin_to_euc(uchar *text, int len)
{
    int i;

    for (i = 0; i < len; i++) {
	*text++ += 0x80;
    }
    return len;
}



