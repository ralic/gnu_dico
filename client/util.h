#ifndef __util_h
#define __util_h

#include <gjdict.h>
#define IP_ADDR_SIZE 16

char * ip_hostname(IPADDR ipaddr);
IPADDR get_ipaddr(char *host);
int good_ipaddr(char *addr);
char *ipaddr2str(char *buffer, IPADDR ipaddr);
IPADDR ipstr2long(char *ip_str);
IPADDR getmyip();
int str2port(char *str);

#endif
