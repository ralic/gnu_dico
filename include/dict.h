#ifndef __dict_h
#define __dict_h

#include <dbs.h>

#ifndef INPUTDICTPATH
# define INPUTDICTPATH "/usr/local/lib"
#endif
#ifndef DICTPATH
# define DICTPATH "/usr/local/lib/jdict"
#endif

#define DICTFILENAME "dict"
#define TEXTFILENAME "text"
#define JIS_TREE "jis.Bt"
#define UNICODE_TREE "unicode.Bt"
#define CORNER_TREE "corner.Bt"
#define FREQ_TREE "freq.Bt"
#define NELSON_TREE "nelson.Bt"
#define HALPERN_TREE "halpern.Bt"
#define GRADE_TREE "grade.Bt"
#define PINYIN_TREE "pinyin.Bt"
#define D_ENGLISH_TREE "english.Bt"
#define KANJI_TREE "kanji.Bt"
#define WORDS_TREE "words.Bt"
#define BUSHU_TREE "bushu.Bt"
#define SKIP_TREE "skip.Bt"
#define XREF_TREE "xref.Bt"
#define D_YOMI_TREE "yomi.Bt"
#define D_ROMAJI_TREE "romaji.Bt"

enum {
    TreeJis,
    TreeUnicode,
    TreeCorner,
    TreeFreq,
    TreeNelson,
    TreeHalpern,
    TreeGrade,
    TreeBushu,
    TreeSkip,
    TreePinyin,
    TreeEnglish,
    TreeKanji,
    TreeXref,
    TreeWords,
    TreeYomi,
    TreeRomaji,
    TreeLast          
};

typedef struct {
    Ushort bushu;
    Ushort numstrokes;
} Bushu;
    
typedef long Offset;

/* Dictionary entry structure.
 * Offsets are binary offsets to the strings in text datafile.
 * Offset 0 means there is no string.
 * All strings are zero-terminated (16-bit terminate with two 
 * zeroes.
 */
 
typedef struct translation {
    Ushort bushu;               /* The radical number */
    Ushort numstrokes;          /* The number of strokes */
    Ushort Qindex;		/* for the "four corner" lookup method */
    unsigned skip;              /* SKIP code */
    Ushort Jindex;              /* jis index */
    Ushort Uindex;		/* because it seems to be the future */
                                /* "Unicode" index */
    Ushort Nindex;		/* Nelson dictionary */
    Ushort Hindex;		/* Halpern dictionary */

    Ushort frequency;		/* frequency that kanji is used */
    Uchar grade_level;		/* akin to  school class level */

    int refcnt;                 /* Number of references to this kanji */ 
    
    Offset english;		/* english translation string. */

    Offset pronunciation;	/* kana, actually */
    Offset kanji;		/* can be pointer to pronunciation */
    Offset pinyin;              /* Reserved for future use */
} DictEntry;

typedef struct {
    int numkanji;
    int numentries;
    Ushort lowestkanji;
    Ushort highestkanji;
    int maxlen8;
    int maxlen16;
} Dictheader;

#endif
