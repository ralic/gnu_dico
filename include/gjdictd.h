#include <time.h>
#include <gjdict.h>
#include <dbs.h>
#include <dict.h>

struct status {
    time_t load_time;
    time_t start_time;
    int query;
    int match;
    int total_match;
};

extern char *progname;
extern char myhostname[];
extern int foreground;
extern int debug;
extern int verbose;
extern int daemon;
extern char *dictpath;
extern struct status status;

DictEntry * dict_entry(DictEntry *entry, Recno recno);
void * dict_string(Offset off);
void dict_setcmplen(int tree_no, int len);
void dict_setcmpcut(int tree_no, int num);
DictEntry * read_match_entry(DictEntry *entry, int tree_no, void *value, Matchdir dir);
DictEntry * read_closest_entry(DictEntry *entry, int tree_no, void *value);
DictEntry * read_exact_match_entry(DictEntry *entry, int tree_no, void *value);
Recno dict_index(int tree_no, void *value);
Recno next_dict_index(int tree_no, void *value);
void dict_enum(int tree_no, int (*fun) ());
   
char * ip_hostname(IPADDR ipaddr);
IPADDR get_ipaddr(char *host);
int good_ipaddr(char *addr);
char * ipaddr2str(char *buffer, IPADDR ipaddr);
IPADDR ipstr2long(char *ip_str);
