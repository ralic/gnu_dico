/* This file is part of Dico.
   Copyright (C) 2008 Sergey Poznyakoff

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

#include <dicod.h>
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
#include <obstack.h>

char *msg_id;

struct input {
    struct obstack stk;
    char *rootptr;
    int argc;
    char **argv;
};

#define ISWS(c) ((c) == ' ' || (c) == '\t')
#define ISQUOTE(c) ((c) == '"' || (c) == '\'')

int
tokenize_input(struct input *in, char *str)
{
    struct utf8_iterator itr;
    int i, argc = 0;
    
    if (!in->rootptr) 
	obstack_init(&in->stk);
    else
	obstack_free(&in->stk, in->rootptr);

    utf8_iter_first(&itr, (unsigned char *)str);

    while (1) {
	int quote;
	
	for (; !utf8_iter_end_p(&itr)
		 && utf8_iter_isascii(itr) && ISWS(*itr.curptr);
	     utf8_iter_next(&itr))
	    ;

	if (utf8_iter_end_p(&itr))
	    break;

	if (utf8_iter_isascii(itr) && ISQUOTE(*itr.curptr)) {
	    quote = *itr.curptr;
	    utf8_iter_next(&itr);
	} else
	    quote = 0;

	for (; !utf8_iter_end_p(&itr)
		 && !(utf8_iter_isascii(itr) && (quote ? 0 : ISWS(*itr.curptr)));
	     utf8_iter_next(&itr)) {
	    if (utf8_iter_isascii(itr)) {
		if (*itr.curptr == quote) {
		    utf8_iter_next(&itr);
		    break;
		} else if (*itr.curptr == '\\') {
		    utf8_iter_next(&itr);
		    if (utf8_iter_isascii(itr)) {
			obstack_1grow(&in->stk, quote_char(*itr.curptr));
		    } else {
			obstack_1grow(&in->stk, '\\');
			obstack_grow(&in->stk, itr.curptr, itr.curwidth);
		    }
		    continue;
		}
	    }
	    obstack_grow(&in->stk, itr.curptr, itr.curwidth);
	}
	obstack_1grow(&in->stk, 0);
	argc++;
    }

    in->rootptr = obstack_finish(&in->stk);

    in->argc = argc;
    in->argv = obstack_alloc(&in->stk, (argc + 1) * sizeof(in->argv[0]));

    for (i = 0; i < argc; i++) {
	in->argv[i] = in->rootptr;
	in->rootptr += utf8_strbytelen(in->rootptr) + 1;
    }
    in->argv[i] = NULL;
    return argc;
}

struct capa_print {
    size_t num;
    dico_stream_t stream;
};

static int
print_capa(const char *name, int enabled, void *data)
{
    struct capa_print *cp = data;
    if (enabled) {
	if (cp->num++)
	    dico_stream_write(cp->stream, ".", 1);
	stream_writez(cp->stream, (char*)name);
    }
    return 0;
}

static void
output_capabilities(dico_stream_t str)
{
    struct capa_print cp;
    cp.num = 0;
    cp.stream = str;
    dico_stream_write(str, "<", 1);
    dicod_capa_iterate(print_capa, &cp);
    dico_stream_write(str, ">", 1);
}

/*
  3.1.  Initial Connection

   When a client initially connects to a DICT server, a code 220 is sent
   if the client's IP is allowed to connect:

             220 text capabilities msg-id
*/
static void
initial_banner(dico_stream_t str)
{
    dico_stream_write(str, "220 ", 4);
    stream_writez(str, hostname);
    dico_stream_write(str, " ", 1);
    if (initial_banner_text)
	stream_writez(str, initial_banner_text);
    else
	stream_writez(str, (char*) program_version);
    dico_stream_write(str, " ", 1);
    output_capabilities(str);
    dico_stream_write(str, " ", 1);
    asprintf(&msg_id, "<%lu.%lu@%s>",
	     (unsigned long) getpid(),
	     (unsigned long) time(NULL),
	     hostname);
    stream_writez(str, msg_id);
    dico_stream_write(str, "\r\n", 2);
}

int
get_input_line(dico_stream_t str, char **buf, size_t *size, size_t *rdbytes)
{
    int rc;
    alarm(inactivity_timeout);
    rc = dico_stream_getline(str, buf, size, rdbytes);
    alarm(0);
    return rc;
}

RETSIGTYPE
sig_alarm(int sig)
{
    exit(1);
}

static void
load_modules()
{
    dico_iterator_t itr = xdico_iterator_create(modinst_list);
    dicod_module_instance_t *inst;

    for (inst = dico_iterator_first(itr); inst;
	 inst = dico_iterator_next(itr)) {
	if (dicod_load_module(inst)) {
	    database_remove_dependent(inst);
	    dico_iterator_remove_current(itr);
	}
    }
    dico_iterator_destroy(&itr);
}

static void
init_databases()
{
    dico_iterator_t itr = xdico_iterator_create(database_list);
    dicod_database_t *dp;

    for (dp = dico_iterator_first(itr); dp; dp = dico_iterator_next(itr)) {
	if (dicod_init_database(dp)) {
	    dico_log(L_NOTICE, 0, _("removing database %s"), dp->name);
	    dico_iterator_remove_current(itr);
	    dicod_database_free(dp);
	}
    }
    dico_iterator_destroy(&itr);
}

static void
open_databases()
{
    dico_iterator_t itr = xdico_iterator_create(database_list);
    dicod_database_t *dp;

    for (dp = dico_iterator_first(itr); dp; dp = dico_iterator_next(itr)) {
	if (dicod_open_database(dp)) {
	    dico_log(L_NOTICE, 0, _("removing database %s"), dp->name);
	    dico_iterator_remove_current(itr);
	    dicod_database_free(dp);
	}
    }
    dico_iterator_destroy(&itr);
}

static void
close_databases()
{
    dico_iterator_t itr = xdico_iterator_create(database_list);
    dicod_database_t *dp;

    for (dp = dico_iterator_first(itr); dp; dp = dico_iterator_next(itr)) {
	if (dicod_close_database(dp))
	    dico_log(L_NOTICE, 0, _("error closing database %s"), dp->name);
    }
    dico_iterator_destroy(&itr);
}

int
soundex_sel(int cmd, const char *word, const char *dict_word, void *closure)
{
    char *code = closure;
    char dcode[DICO_SOUNDEX_SIZE];
    
    switch (cmd) {
    case DICO_SELECT_BEGIN:
	dico_soundex(word, code);
	break;

    case DICO_SELECT_RUN:
	dico_soundex(dict_word, dcode);
	return strcmp(dcode, code) == 0;

    case DICO_SELECT_END:
	break;
    }
    return 0;
}
	    

void
dicod_init_strategies()
{
    static char code[5];
    static struct dico_strategy defstrat[] = {
	{ "exact", "Match words exactly" },
	{ "prefix", "Match word prefixes" },
	{ "soundex", "Match using SOUNDEX algorithm", soundex_sel, code },
    };
    int i;
    for (i = 0; i < DICO_ARRAY_SIZE(defstrat); i++)
	dico_strategy_add(defstrat + i);
}

void
dicod_server_init()
{
    load_modules();
    init_databases();
    if (!dico_get_default_strategy()) 
	dico_set_default_strategy(DICTD_DEFAULT_STRATEGY);
}

void
dicod_server_cleanup()
{
    dico_iterator_t itr = xdico_iterator_create(database_list);
    dicod_database_t *dp;

    for (dp = dico_iterator_first(itr); dp; dp = dico_iterator_next(itr)) {
	if (dicod_free_database(dp)) 
	    dico_log(L_NOTICE, 0, _("error freeing database %s"), dp->name);
    }
    dico_iterator_destroy(&itr);
}    

int
dicod_loop(dico_stream_t str)
{
    char *buf = NULL;
    size_t size = 0;
    size_t rdbytes;
    struct input input;

    begin_timing("dicod");
    signal(SIGALRM, sig_alarm);
    memset(&input, 0, sizeof input);
    got_quit = 0;
    if (transcript) {
	dico_stream_t logstr = dico_log_stream_create(L_DEBUG);
	if (!logstr)
	    xalloc_die();
	str = xdico_transcript_stream_create(str, logstr, NULL);
    }

    open_databases();
    check_db_visibility();
    initial_banner(str);

    while (!got_quit && get_input_line(str, &buf, &size, &rdbytes) == 0) {
	trimnl(buf, rdbytes);
	tokenize_input(&input, buf);
	if (input.argc == 0)
	    continue;
	dicod_handle_command(str, input.argc, input.argv);
    }

    close_databases();    
    init_auth_data();
    access_log_free_cache();
    return 0;
}

int
dicod_inetd()
{
    dico_stream_t str = dico_fd_io_stream_create(0, 1);

    if (!str)
        return 1;
    dico_stream_set_buffer(str, dico_buffer_line, DICO_MAX_BUFFER);
    dico_stream_set_buffer(str, dico_buffer_line, DICO_MAX_BUFFER);

    client_addrlen = sizeof(client_addr);
    if (getsockname (0, &client_addr, &client_addrlen) == -1)
	client_addrlen = 0;

    server_addrlen = sizeof(server_addr);
    if (getsockname (1, &server_addr, &server_addrlen) == -1)
	server_addrlen = 0;
    
    return dicod_loop(str);
}