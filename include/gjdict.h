#ifndef __gjdict_h
#define __gjdict_h

#include <stdlib.h>
#include <stdarg.h>

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

#define L_DEBUG     0
#define L_INFO      1
#define L_NOTICE    2
#define L_WARN      3
#define L_ERR       4
#define L_CRIT      5
#define L_ALERT     6
#define L_EMERG     7

#define L_MASK      0xff

#define L_CONS      0x8000

extern const char *program_name;
void set_program_name(char *name);

typedef void (*gjdict_log_printer_t) (int /* lvl */,
				      int /* exitcode */,
				      int /* errcode */,
				      const char * /* fmt */,
				      va_list);
void _stderr_log_printer(int, int, int, const char *, va_list);

void set_log_printer(gjdict_log_printer_t prt);
void logmsg(int lvl, int errcode, const char *fmt, ...);
void die(int exitcode, int lvl, int errcode, char *fmt, ...);

char * ip_hostname(IPADDR ipaddr);
IPADDR get_ipaddr(char *host);
int str2port(char *str);

int str_to_diag_level(const char *str);


void *xmalloc(size_t size);
void *xzalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);

#endif
    




