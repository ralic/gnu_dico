#ifndef __gjdict_h
#define __gjdict_h

#define GJDICT_PORT 7320

typedef unsigned long UINT4;
typedef UINT4 IPADDR;

typedef enum {
    MatchNext,
    MatchPrev
} Matchdir;

#define QUERY_SEQ 0
#define QUERY_EXACT 1
#define QUERY_CLOSEST 2

/* Return types */

#define RC_OK          100
#define RC_NOMATCH     200
#define RC_NOTSUPPORT  400
#define RC_ERR         500
#define RC_BADINPUT    510
#define RC_CRIT        600

#endif
    




