/* $Id: main.c,v 1.1 2001/11/03 22:35:17 gray Exp $ */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <strings.h>
#include <wait.h>
#include <varargs.h>
#include <log.h>
#include <gjdictd.h>
#include <patchlevel.h>
#include <server.h>

#define OPTLIST "cd:fhLm:p:st:vVx:"
#ifdef HAVE_GETOPT_LONG
typedef struct option LONGOPT;
LONGOPT longopts[] = {
    "console-logging", no_argument, 0, 'c',
    "dict-path", required_argument, 0, 'd',
    "foreground", no_argument, 0, 'f',
    "help",    no_argument, 0, 'h',
    "license", no_argument, 0, 'L',
    "max-children", required_argument, 0, 'm',
    "port", required_argument, 0, 'p',
    "single-user", no_argument, 0, 's',
    "timeout", required_argument, 0, 't',
    "verbose", no_argument, 0, 'v',
    "version", no_argument, 0, 'V',
    "debug", required_argument, 0, 'x',
    0,
};
#else
# define getopt_long(argc, argv, optstr, long_opts, iptr) \
 getopt(argc, argv, optstr)
#endif

char *progname;
char myhostname[128];
int foreground = 0;
int verbose = 0;
char *portstr = "gjdict";
int port = GJDICT_PORT;
int num_children;
int max_children = 16;
int single_user;
long inactivity_timeout = 3600;

char *dictpath = DICTPATH;
char msg2many[] = "500 too many sessions open. Try again later\r\n";
struct status status;

int daemon;

void say();
RETSIGTYPE sig_cleanup();
RETSIGTYPE sig_hup();
RETSIGTYPE sig_usr1();
RETSIGTYPE sig_usr2();
RETSIGTYPE sig_child();
void initsocket();
void get_port();
void set_inactivity_timeout(char *str);

int
main(argc, argv)
    int argc;
    char **argv;
{
    int c, len, result;
    fd_set rfds;
    struct sockaddr_in address;
    int listen_socket;
    int sock;
    int reuse_addr = 1;
    pid_t pid;
    extern struct sockaddr_in his_addr;
    
    char *p;

    p = strrchr(argv[0], '/');
    if (p)
	progname = p + 1;
    else
	progname = argv[0];

    while ((c = getopt_long(argc, argv, OPTLIST, longopts, &len)) != EOF) {
	switch (c) {
	case 'c':
	    console_log++;
	    break;
	case 'd': 
	    dictpath = optarg;
	    break;
	case 'f':
	    foreground = 1;
	    break;
	case 'h':
	    say(usage);
	    /*NOTREACHED*/
	case 'L':
	    say(licence);
	    break;
	case 'm':
	    max_children = atoi(optarg);
	    if (max_children <= 0) {
		logmsg(L_ERR|L_CONS, "bad -m parameter: %s", optarg);
		exit(1);
	    }
	    break;
	case 'p':
	    portstr = optarg;
	    break;
	case 's':
	    single_user++;
	    break;
	case 't':
	    set_inactivity_timeout(optarg);
	    break;
	case 'v':
	    verbose++;
	    break;
	case 'V':
	    say(version);
	    break;
	case 'x':
	    set_debug_level(optarg);
	    break;
	}
    }

    gethostname(myhostname, sizeof(myhostname));

    initlog();
    initdict();
    initsearch();
	    
    get_port();
    
    signal(SIGTERM, sig_cleanup);
    signal(SIGINT, sig_cleanup);
    signal(SIGCHLD, sig_child);
    signal(SIGHUP, sig_hup);
    signal(SIGUSR1, sig_usr1);
    signal(SIGUSR2, sig_usr2);

    memset((char *) &address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) {
	logmsg(L_ERR|L_CONS, "socket: %s", strerror(errno));
	exit(1);
    }

    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR,
	       &reuse_addr, sizeof(reuse_addr));

    if (bind(listen_socket,
	     (struct sockaddr *) &address, sizeof(address)) < 0) {
	close(listen_socket);
	logmsg(L_ERR|L_CONS, "bind: %s", strerror(errno));
	exit(1);
    }
    
    if (!foreground) {
	int fd;
	
	pid = fork();
	if (pid < 0) {
	    logmsg(L_ERR|L_CONS, "can't fork: %s", strerror(errno));
	    exit(1);
	}
	
	if (pid > 0) {
	    FILE *fp = fopen(PIDFILE, "w");
	    if (fp) {
		fprintf(fp, "%ld", pid);
		fclose(fp);
	    } else {
		logmsg(L_ERR|L_CONS, "can't write pidfile %s: %s",
		    PIDFILE, strerror(errno));
	    }
	    logmsg(L_INFO|L_CONS, "installed gjdict daemon at pid %ld", pid);
	    exit(0);
	}

	close(0);
	close(1);
	close(2);
	chdir("/");
	setsid();

	fd = open("/dev/console", O_WRONLY | O_NOCTTY);
	if (fd != 2) {
	    dup2(fd, 2);
	    close(fd);
	}

	daemon = 1;
    } else
	console_log++;

    status.load_time = time(NULL);
    
    listen(listen_socket, 8);

    while (1) {
	FD_ZERO(&rfds);
	FD_SET(listen_socket, &rfds);
	
	result = select(32, &rfds, NULL, NULL, NULL);
	if (result == -1) {
	    if (errno == EINTR) 
		continue;
	    logmsg(L_ERR, "select: %s", strerror(errno));
	    exit(1);
	}

	len = sizeof(his_addr);
	if ((sock = accept(listen_socket, (struct sockaddr *)&his_addr, &len)) < 0) {
	    logmsg(L_ERR, "accept: %s", strerror(errno));
	    exit(1);
	}
	
	if (num_children > max_children) {
	    logmsg(L_ERR, "rejecting service to %s: too many sessions open",
		ip_hostname(ntohl(his_addr.sin_addr.s_addr)));
	    write(sock, msg2many, strlen(msg2many));
	    close(sock);
	    continue;
	}
	num_children++;

	if (!single_user) {
	    pid = fork();
	    if (pid < 0) {
		close(sock);
		logmsg(L_ERR, "can't fork: %s", strerror(errno));
		continue;
	    }		
	    
	    if (pid > 0) {
		/* parent branch */
		logmsg(L_INFO, "PID %d serving %s",
		    pid, ip_hostname(ntohl(his_addr.sin_addr.s_addr)));
		close(sock);
		continue;
	    }
	    /* Child branch */
	    close(listen_socket);
	    result = server(sock);
	    exit(result);
	} else  /* single-process mode */
	    result = server(sock);
    }
}

void
get_port()
{
    if (isdigit(portstr[0])) {
	char *errpos;
	port = strtol(portstr, &errpos, 0);
	if (*errpos) {
	    logmsg(L_ERR|L_CONS, "error converting port number near %s", errpos);
	    exit(1);
	}
    } else {
	struct servent *serv = getservbyname(portstr, "tcp");

	if (!serv) {
	    logmsg(L_ERR|L_CONS, "service %s/tcp not found in /etc/services",
		portstr);
	    exit(1);
	}
    }
}


void
set_inactivity_timeout(str)
    char *str;
{
    long t, hrs, min, sec;
    char *p;
    
    hrs = min = sec = 0;
    t = strtol(str, &p, 10);
    switch (*p++) {
    case 'h':
	hrs = t;
	break;
    case 'm':
	min = t;
	break;
    case 0:
	p = NULL;
	/*FALLTHRU*/
    case 's':
	sec = t;
	break;
    default:
	logmsg(L_ERR|L_CONS,
	    "wrong time format near `%s'", p-1);
	return;
    }

    if (p && *p) {
	t = strtol(p, &p, 10);
	switch (*p++) {
	case 0:
	    p = NULL;
	    /*FALLTHRU*/
	case 'm':
	    min = t;
	    break;
	case 's':
	    sec = t;
	    break;
	default:
	    logmsg(L_ERR|L_CONS,
		"wrong time format near `%s'", p-1);
	    return;
	}
    
	if (p && *p) {
	    t = strtol(p, &p, 10);
	    switch (*p) {
	    case 0:
		/*FALLTHRU*/
	    case 's':
		sec = t;
		break;
	    default:
		logmsg(L_ERR|L_CONS,
		    "wrong time format near `%s'", p);
		return;
	    }
	}
    }
    inactivity_timeout = (hrs*60 + min)*60 + sec;
}

/*ARGSUSED*/
RETSIGTYPE
sig_child(num)
    int num;
{
    pid_t childpid;
    int status;

    signal(SIGCHLD, sig_child);    
    childpid = waitpid((pid_t)-1, &status, 0);
    if (WIFEXITED(status))
	status = WEXITSTATUS(status);
    logmsg(L_INFO, "child %ld exited with status %d", childpid, status);
    num_children--;
}

/*ARGSUSED*/
RETSIGTYPE
sig_cleanup(num)
    int num;
{
    unlink(PIDFILE);
    logmsg(L_INFO, "exited on signal %d", num);
    exit(0);
}

/*ARGSUSED*/
RETSIGTYPE
sig_hup(num)
    int num;
{
    signal(SIGHUP, sig_hup);
}

/*ARGSUSED*/
RETSIGTYPE
sig_usr1(num)
    int num;
{
    signal(SIGUSR1, sig_usr1);
}

/*ARGSUSED*/
RETSIGTYPE
sig_usr2(num)
    int num;
{
    signal(SIGUSR2, sig_usr2);
}

char usage[] = "\
usage: gjdictd ...\
\n";

char version[] =
"gjdictd " VERSION "\n"
"    Copyright (C) 1999 Gray\n"
"Compilation options:\n"
#ifdef DEBUG
"    DEBUG\n"
#endif
#ifdef GNU_STYLE_OPTIONS
"    GNU_STYLE_OPTIONS\n"
#endif
"\n";

char licence[] = "\
gjdictd " VERSION "\n\
    Copyright (C) 1999 Gray\n\
\n\
    This program is free software; you can redistribute it and/or modify\n\
    it under the terms of the GNU General Public License as published by\n\
    the Free Software Foundation; either version 2 of the License, or\n\
    (at your option) any later version.\n\
\n\
    This program is distributed in the hope that it will be useful,\n\
    but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
    GNU General Public License for more details.\n\
\n\
    You should have received a copy of the GNU General Public License\n\
    along with this program; if not, write to the Free Software\n\
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\
"; 


void
say(s)
    char *s;
{
    printf("%s", s);
    exit(0);
}










