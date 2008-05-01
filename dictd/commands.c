/* This file is part of Gjdict.
   Copyright (C) 2008 Sergey Poznyakoff

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <dictd.h>

int got_quit;

void
dictd_quit(stream_t str, int argc, char **argv)
{
    got_quit = 1;
    stream_writez(str, "221 bye\r\n");
}

void dictd_show_std_help(stream_t str);

void
dictd_help(stream_t str, int argc, char **argv)
{
    const char *text = help_text;
    stream_writez(str, "113 help text follows\r\n");
    if (text) {
	if (text[0] == '+') {
	    dictd_show_std_help(str);
	    text++;
	}
	stream_write_multiline(str, text);
    } else
	dictd_show_std_help(str);
    stream_writez(str, "\r\n.\r\n");
    stream_writez(str, "250 ok\r\n");    
}

static int
_show_database(void *item, void *data)
{
    dictd_dictionary_t *dict = item;
    stream_t str = data;

    stream_printf(str, "%s \"%s\"\r\n",
		  dict->name, dict->descr); /* FIXME: Quote descr. */
    return 0;
}

void
dictd_show_info(stream_t str, int argc, char **argv)
{
    char *dbname = argv[2];
    dictd_dictionary_t *dict = find_dictionary(dbname);
    if (!dict) 
	stream_writez(str, "550 invalid database, use SHOW DB for list\r\n");
    else {
	stream_printf(str, "112 information for %s\r\n", dbname);
	if (dict->info)
	    stream_write_multiline(str, dict->info);
	else
	    stream_writez(str, "No information available.\r\n");
	stream_writez(str, "\r\n.\r\n");
	stream_writez(str, "250 ok\r\n");    
    }
}


void
dictd_show_databases(stream_t str, int argc, char **argv)
{
    size_t count = dict_list_count(dictionary_list);
    stream_printf(str, "110 %lu databases present\r\n",
		  (unsigned long) count);
    dict_list_iterate(dictionary_list, _show_database, str);
    stream_writez(str, ".\r\n");
    stream_writez(str, "250 ok\r\n");    
}

void
dictd_show_server(stream_t str, int argc, char **argv)
{
    stream_writez(str, "114 server information\r\n");
    /* FIXME: (For logged in users) show:
       dictd (gjdict 1.0.90) on Linux 2.6.18, Trurl.gnu.org.ua up 81+01:33:49, 12752570 forks (6554.7/hour)
    */
    stream_write_multiline(str, server_info);
    stream_writez(str, "\r\n.\r\n");
    stream_writez(str, "250 ok\r\n");    
}

void
dictd_client(stream_t str, int argc, char **argv)
{
    logmsg(L_INFO, 0, "Client info: %s", argv[1]);
    stream_writez(str, "250 ok\r\n");
}


struct dictd_command command_tab[] = {
    { "DEFINE", 3, "database word", "look up word in database" },
    { "MATCH", 4, "database strategy word",
      "match word in database using strategy" },
    { "SHOW DB", 2, NULL, "list all accessible databases",
      dictd_show_databases, },
    { "SHOW DATABASES", 2, NULL, "list all accessible databases",
      dictd_show_databases, },
    { "SHOW STRAT", 2, NULL, "list available matching strategies",
      NULL },
    { "SHOW STRATEGIES", 2, NULL, "list available matching strategies",
      NULL },
    { "SHOW INFO", 3, "database", "provide information about the database",
      dictd_show_info },
    { "SHOW SERVER", 2, NULL, "provide site-specific information",
      dictd_show_server },
    { "OPTION MIME", 2, NULL, "use MIME headers",
      NULL }, /* FIXME: capa */
    { "CLIENT", 2, "info", "identify client to server",
      dictd_client },
    { "STATUS", 1, NULL, "display timing information" },
    { "AUTH", 3, "user string", "provide authentication information",
      NULL }, /* FIXME: capa */
#if 0
    SASLAUTH mechanism initial-response
    SASLRESP response
#endif
    { "HELP", 1, NULL, "display this help information",
      dictd_help },
    { "QUIT", 1, NULL, "terminate connection", dictd_quit },
    { NULL }
};

void
dictd_show_std_help(stream_t str)
{
    struct dictd_command *p;
    
    for (p = command_tab; p->keyword; p++) {
	int len = strlen(p->keyword);

	stream_writez(str, p->keyword);
	if (p->param) {
	    stream_printf(str, " %s", p->param);
	    len += strlen(p->param) + 1;
	}
	
	if (len < 31)
	    len = 31 - len;
	else
	    len = 0;
	stream_printf(str, "%*.*s -- %s\r\n", len, len, "", p->help);
    }
}


struct dictd_command *
locate_command(int argc, char **argv)
{
    struct dictd_command *p;
    
    for (p = command_tab; p->keyword; p++) {
	int i, off = 0;
	for (i = 0; i < argc; i++) {
	    int len = strlen(argv[i]);
	    if (c_strncasecmp(p->keyword + off, argv[i], len) == 0) {
		off += len;
		if (p->keyword[off] == 0)
		    return p;
		if (p->keyword[off] == ' ') {
		    off++;
		    continue;
		} else
		    break;
	    }
	}
    }		
    return NULL;
}

void
dictd_handle_command(stream_t str, int argc, char **argv)
{
    struct dictd_command *cmd = locate_command(argc, argv);
    if (!cmd) 
	stream_writez(str, "500 unknown command\r\n");
    else if (argc != cmd->nparam) 
	stream_writez(str, "501 wrong number of arguments\r\n");
    else if (!cmd->handler)
	stream_writez(str, "502 command is not yet implemented, sorry\r\n");
    else
	cmd->handler(str, argc, argv);
}

