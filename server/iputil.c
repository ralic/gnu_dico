/* This file is part of RADIUS.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef lint
static char rcsid[] = 
"$Id: iputil.c,v 1.1 2001/11/03 22:35:32 gray Exp $";
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>

#include <gjdictd.h>

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
    UINT4		n_ipaddr;
    
    n_ipaddr = htonl(ipaddr);
    hp = gethostbyaddr((char *)&n_ipaddr, sizeof (struct in_addr), AF_INET);
    if (hp == 0) {
	ipaddr2str(hstname, ipaddr);
	return(hstname);
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
    IPADDR	ipstr2long();
    
    if (good_ipaddr(host) == 0) {
	return ipstr2long(host);
    } else if ((hp = gethostbyname(host)) == (struct hostent *)NULL) {
	return((UINT4)0);
    }
    return ntohl(*(UINT4 *)hp->h_addr);
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
		return((IPADDR)0);
	    }
	    *ptr++ = *ip_str++;
	    count++;
	}
	if (count >= 4 || count == 0) {
	    return((IPADDR)0);
	}
	*ptr = '\0';
	cur_byte = atoi(buf);
	if (cur_byte < 0 || cur_byte > 255) {
	    return((IPADDR)0);
	}
	ip_str++;
	ipaddr = ipaddr << 8 | (IPADDR)cur_byte;
    }
    return ipaddr;
}

