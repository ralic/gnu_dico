/* $Id: util.c,v 1.1 2001/11/03 22:36:26 gray Exp $ */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <varargs.h>
#include <client.h>
#include <util.h>
#include <log.h>

/* ***********************************************************************
 * IP address fiddling 
 *************************************************************************/

/*
 *	Return a printable host name (or IP address in dot notation)
 *	for the supplied IP address.
 */
char *
ip_hostname(ipaddr)
	IPADDR ipaddr;
{
	struct		hostent *hp;
	static char	hstname[128];
	IPADDR		n_ipaddr;

	n_ipaddr = htonl(ipaddr);
	hp = gethostbyaddr((char *)&n_ipaddr, sizeof(struct in_addr), AF_INET);
	if (hp == 0) {
		ipaddr2str(hstname, ipaddr);
		return hstname;
	}
	return (char *)hp->h_name;
}


/*
 *	Return an IP address in host long notation from a host
 *	name or address in dot notation.
 */
IPADDR
get_ipaddr(host)
	char *host;
{
	struct hostent	*hp;
	IPADDR		ipstr2long();

	if (good_ipaddr(host) == 0) 
		return ipstr2long(host);
	else if ((hp = gethostbyname(host)) == (struct hostent *)NULL) 
		return((IPADDR)-1);
	/* else */
	return ntohl(*(IPADDR *)hp->h_addr);
}


/*
 *	Check for valid IP address in standard dot notation.
 */
int
good_ipaddr(addr)
	char *addr;
{
	int	dot_count;
	int	digit_count;

	dot_count = 0;
	digit_count = 0;
	while (*addr != '\0' && *addr != ' ') {
		if (*addr == '.') {
			dot_count++;
			digit_count = 0;
		} else if (!isdigit(*addr)) {
			dot_count = 5;
		} else {
			digit_count++;
			if(digit_count > 3) {
				dot_count = 5;
			}
		}
		addr++;
	}
	if (dot_count != 3) 
		return -1;
	else 
		return 0;
}


/*
 *	Return an IP address in standard dot notation for the
 *	provided address in host long notation.
 */
char *
ipaddr2str(buffer, ipaddr)
	char *buffer;
	IPADDR ipaddr;
{
	int	addr_byte[4];
	int	i;
	IPADDR	xbyte;

	for (i = 0; i < 4; i++) {
		xbyte = ipaddr >> (i*8);
		xbyte = xbyte & (IPADDR)0x000000FF;
		addr_byte[i] = xbyte;
	}
	sprintf(buffer, "%u.%u.%u.%u", addr_byte[3], addr_byte[2],
		addr_byte[1], addr_byte[0]);
	return buffer;
}


/*
 *	Return an IP address in host long notation from
 *	one supplied in standard dot notation.
 */
IPADDR
ipstr2long(ip_str)
	char *ip_str;
{
	char	buf[6];
	char	*ptr;
	int	i;
	int	count;
	IPADDR	ipaddr;
	int	cur_byte;

	ipaddr = (IPADDR)0;
	for (i = 0; i < 4; i++) {
		ptr = buf;
		count = 0;
		*ptr = '\0';
		while (*ip_str != '.' && *ip_str != '\0' && count < 4) {
			if (!isdigit(*ip_str)) {
				return (IPADDR)0;
			}
			*ptr++ = *ip_str++;
			count++;
		}
		if (count >= 4 || count == 0) {
			return (IPADDR)0;
		}
		*ptr = '\0';
		cur_byte = atoi(buf);
		if (cur_byte < 0 || cur_byte > 255) {
			return (IPADDR)0;
		}
		ip_str++;
		ipaddr = ipaddr << 8 | (IPADDR)cur_byte;
	}
	return ipaddr;
}

IPADDR
getmyip()
{
	char myname[256];

	(void)gethostname(myname, sizeof(myname));
	return get_ipaddr(myname);
}

int
str2port(str)
    char *str;
{
    struct servent *serv;
    char *p;
    int port;
    
    /* First try to read it from /etc/services */
    serv = getservbyname(str, "tcp");

    if (serv != NULL)
	port = ntohs(serv->s_port);
    else {
	long l;
	/* Not in services, maybe a number? */
	l = strtol(str, &p, 0);

	if (*p || l < 0 || l > USHRT_MAX)
	    return -1;

	port = l;
    }

    return port;
}

/* ******************************************************************** */

XChar2b *
dup_16(str)
    XChar2b *str;
{
    int len;
    XChar2b *ret_str;
    
    len = strlen((char *) str);
    
    ret_str = (XChar2b *) malloc(sizeof(char) * len + sizeof(XChar2b));
    if (ret_str) 
	memcpy(ret_str, str, sizeof(char)*len + sizeof(XChar2b));
    return ret_str;
}

void
Beep()
{
    if (config.bell)
	XBell(display, 100);
}

int
GetWidgetNumber(w, fl)
    Widget w;
    int fl;
{
    String str;
    char fmt[] = "%d";
    int ret;
    
    XtVaGetValues(w, XtNstring, &str, NULL);
    fmt[1] = fl;
    if (sscanf(str, fmt, &ret) != 1)
	return -1;
    return ret;
}


int
GetWidgetSkipCode(w, sptr)
    Widget w;
    String *sptr;
{
    String str;
    int num = 0;

    XtVaGetValues(w, XtNstring, &str, NULL);
    if (sptr)
	*sptr = str;
    num = strtol(str, (char**)&str, 10);
    if (*str != '-') {
	setstatus(True, "Invalid SKIP pattern: '-' expected");
	return 0;
    }
    num <<= 8;
    num |= strtol(str+1, (char**)&str, 10);
    if (*str != '-') {
	setstatus(True, "Invalid SKIP pattern: '-' expected");
	return 0;
    }
    num <<= 8;
    num |= strtol(str+1, (char**)&str, 10);
    while (*str && isspace(*str))
	str++;
    if (*str) {
	setstatus(True, "Invalid SKIP pattern: junk at the end");
	return 0;
    }
    return num;
}

void
SetWidgetNumber(w, fmt, val)
    Widget w;
    char *fmt;
    int val;
{
    char tempstr[100];
    if (val == 0)
	tempstr[0] = '\0';
    else
	sprintf(tempstr, fmt, val);
    XtVaSetValues(w, XtNstring, tempstr, NULL);
    return;
}

void
SetWidgetSkipCode(w, code)
    Widget w;
    int code;
{
    char buf[24];

    sprintf(buf, "%d-%d-%d", code >> 16, (code >> 8) & 0xf, code & 0xf);
    XtVaSetValues(w, XtNstring, buf, NULL);
}

void
SetWidgetBushu(w, radical, strokes)
    Widget w;
    int radical;
    int strokes;
{
    char buf[24];

    print_bushu(buf, radical, strokes);
    XtVaSetValues(w, XtNstring, buf, NULL);
}


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

char *dictentry_buf;

void
init_dict_entry()
{
    logmsg(L_INFO, "Allocating %d+%d bytes for dictionary entry buffer",
	 config.dict_entry.buf_size,
	 config.dict_entry.buf_incr);
    dictentry_buf = malloc(config.dict_entry.buf_size +
			   config.dict_entry.buf_incr);
    if (!dictentry_buf)
	logmsg(L_WARN,
	       "Can't allocate dictionary entry buffer: using raw entry format");
}

char *
format_string(str)
    char *str;
{
    char *p;
    int num;
    
    if (!dictentry_buf)
	return str;

    num = 0;
    p = dictentry_buf;
    num++;
    p += sprintf(p, "%d. ", num);
    while (*str)
	if (*str == '|') {
	    *p++ = ';';
	    *p++ = '\n';
	    str++;
	    while (*str && *str == ' ')
		str++;
	    if (*str) {
		num++;
		p += sprintf(p, "%d. ", num);
	    }
	} else
	    *p++ = *str++;
    *p++ = ';';
    *p = 0;

    return dictentry_buf;
}

char *
format_string16(str)
    char *str;
{
    char *p;
    int len;
    int num;

    if (!dictentry_buf)
	return str;
    len = strlen(str);
    shin_to_euc(str, len);
    num = 1;
    p = dictentry_buf;
    p += sprintf(p, "%d. ", num);
    while (*str) {
	if (*str == '\xa1' && str[1] == '\xa1') {
	    *p++ = '\n';
	    str += 2;
	    while (*str && *str == '\xa1' && str[1] == '\xa1')
		    str += 2;
	    
	    if (*str) {
		num++;
		p += sprintf(p, "%d. ", num);
	    }
	} else {
	    *p++ = *str++;
	    *p++ = *str++;
	}
    }
    *p = 0;
    return dictentry_buf;
}



/* Error - handling functions
 */
void
parm_error(func_name, parm_no, val)
    char *func_name;
    int parm_no;
    char *val;
{
    error("Parameter %d to %s(). Value passed: `%s'\n",
	  parm_no, func_name, val);
}

void
parm_cnt_error(func_name, parm_cnt, passed_cnt)
    char *func_name;
    int parm_cnt;
    int passed_cnt;
{
    error("Too %s (%d) arguments to %s(). Expected %d\n",
	   (passed_cnt < parm_cnt) ? "few" : "many", passed_cnt,
	   func_name, parm_cnt);
}


int
decode_keyword(kw, string)
    struct keyword *kw;
    char *string;
{
    int len = strlen(string);

    for ( ; kw->name; kw++)
	if (strcmp(kw->name, string) == 0)
	    return kw->code;
    return -1;
}

#define MIN_ESC_BUF_SIZE 128

static char *escape_buf;
static int escape_len;

static void
alloc_escape_buf(len)
    int len;
{
    if (escape_len >= len)
	return;
    escape_len = (len > MIN_ESC_BUF_SIZE) ? len : MIN_ESC_BUF_SIZE;
    escape_buf = XtRealloc(escape_buf, escape_len);
}
    
    

char *
escape_string(str)
    char *str;
{
    char *p;
    
    alloc_escape_buf(2*strlen(str)+1);

    p = escape_buf;

    while (*str) {
	if (*str == '\\' || *str == '"') 
	    *p++ = '\\';
	*p++ = *str++;
    }
    *p = 0;
    return escape_buf;
}
	    
