/* An "echo" database for GNU Dico test suite.
   Copyright (C) 2012-2017 Sergey Poznyakoff

   GNU Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <string.h>

/* This module either returns query strings without any changes (echo mode),
   or denies any queries (null mode). */

enum echo_mode {
    ECHO_ECHO, /* Operate in echo mode */
    ECHO_NULL  /* Operate in null mode */
};

struct echo_handle {
    enum echo_mode mode;
};

static int
echo_init(int argc, char **argv)
{
    return 0;
}

static dico_handle_t
echo_init_db(const char *dbname, int argc, char **argv)
{
    int null_mode = 0;
    struct echo_handle *hp;
    
    struct dico_option init_db_option[] = {
	{ DICO_OPTSTR(null), dico_opt_bool, &null_mode },
	{ NULL }
    };

    if (dico_parseopt(init_db_option, argc, argv, 0, NULL))
	return NULL;

    hp = malloc(sizeof(*hp));
    if (hp)
	hp->mode = null_mode ? ECHO_NULL : ECHO_ECHO;
    else
	dico_log(L_ERR, 0, "not enough memory");
    return (dico_handle_t)hp;
}

static int
echo_free_db(dico_handle_t hp)
{
    free(hp);
    return 0;
}

static int
echo_open(dico_handle_t dp)
{
    return 0;
}

static int
echo_close(dico_handle_t hp)
{
    return 0;
}

static char *
echo_info(dico_handle_t hp)
{
    struct echo_handle *ep = (struct echo_handle*)hp;
    static char *echo_info_str[2] = {
	"\
ECHO database.\n\n\
This database echoes each query.\n",
	"\
NULL database.\n\n\
This database returns NULL (no result) to any match and define\n\
requests.\n"
    };
    return strdup(echo_info_str[ep->mode]);
}

static char *
echo_descr(dico_handle_t hp)
{
    struct echo_handle *ep = (struct echo_handle*)hp;
    static char *echo_descr_str[2] = {
	"GNU Dico ECHO database",
	"GNU Dico NULL database"
    };
    return strdup(echo_descr_str[ep->mode]);
}

static dico_result_t
echo_match(dico_handle_t hp, const dico_strategy_t strat, const char *word)
{
    struct echo_handle *ep = (struct echo_handle*)hp;
    if (ep->mode == ECHO_NULL)
	return NULL;
    return (dico_result_t) strdup(word);
}

static dico_result_t
echo_define(dico_handle_t hp, const char *word)
{
    struct echo_handle *ep = (struct echo_handle*)hp;
    if (ep->mode == ECHO_NULL)
	return NULL;
    return (dico_result_t) strdup(word);
}

static int
echo_output_result (dico_result_t rp, size_t n, dico_stream_t str)
{
    char *word = (char*)rp;
    dico_stream_write(str, word, strlen(word));
    return 0;
}

static size_t
echo_result_count(dico_result_t rp)
{
    return 1;
}

static size_t
echo_compare_count(dico_result_t rp)
{
    return 1;
}

static void
echo_free_result(dico_result_t rp)
{
    free(rp);
}

struct dico_database_module DICO_EXPORT(echo, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NONE,
    echo_init,
    echo_init_db,
    echo_free_db,
    echo_open,
    echo_close,
    echo_info,
    echo_descr,
    NULL, /* echo_lang */
    echo_match,
    echo_define,
    echo_output_result,
    echo_result_count,
    echo_compare_count,
    echo_free_result,
    NULL /* echo_result_headers */
};
