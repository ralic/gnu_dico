
#define L_CONS             0x8000
#define L_MASK             0xff

#define DEBUG_GRAM         0
#define DEBUG_SEARCH       1
#define DEBUG_COMM         2
#define NUMDEBUG           3

extern int console_log;
extern int debug_level[];

#define debug(c,l)\
 if (debug_level[c] >= l) dlog

/* log.c */
int		log();
int		dlog();
void            log_init();
