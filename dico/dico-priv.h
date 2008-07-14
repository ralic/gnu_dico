/* This file is part of Dico.
   Copyright (C) 1998-2000, 2008 Sergey Poznyakoff

   Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <xalloc.h>
#include <ctype.h>
#include <syslog.h>
#include <inttypes.h>
#include <limits.h>
#include <size_max.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <ltdl.h>

#include <xdico.h>
#include <inttostr.h>
#include <c-strcase.h>
#include <gettext.h>
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
#include <obstack.h>
#include <quotearg.h>

enum dico_client_mode {
    mode_define,
    mode_match,
    mode_dbs,
    mode_strats,
    mode_help,
    mode_info,
    mode_server
};

struct dict_connection {
    dico_stream_t str;
    int capac;
    char **capav;
    char *msgid;
    char *buf;
    size_t size;
    size_t level;
    struct obstack stk;
    int stk_init;
};

#define DICO_CLIENT_ID PACKAGE_STRING 

extern struct dico_url dico_url;
extern char *user;
extern char *key;
extern char *client;
extern enum dico_client_mode mode;
extern int transcript;
extern IPADDR source_addr;

void get_options (int argc, char *argv[], int *index);

int dict_connect(struct dict_connection **pconn, dico_url_t url);
void dict_conn_close(struct dict_connection *conn);
int dict_read_reply(struct dict_connection *conn);
int dict_status_p(struct dict_connection *conn, char *status);
int dict_capa(struct dict_connection *conn, char *capa);
int dict_multiline_reply(struct dict_connection *conn, size_t *pnlines);
int dict_lookup_url(dico_url_t url);
int dict_lookup(char *word);
int dict_single_command(char *cmd, char *arg, char *code);
dico_stream_t create_pager_stream(size_t nlines);
