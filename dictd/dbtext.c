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

#include <dictd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static int
dbtext_open(void **handle, dico_url_t url)
{
    char *dirname = dico_url_full_path(url);
    struct stat st;
    if (!dirname) {
	logmsg(L_ERR, 0, _("cannot get path from URL `%s'"), url->string);
	return 1;
    }

    if (stat(dirname, &st)) {
	logmsg(L_ERR, errno, _("cannot stat directory `%s'"), dirname);
	free(dirname);
	return 1;
    }

    if (!S_ISDIR(st.st_mode)) {
	logmsg(L_ERR, 0, _("%s: not a directory"), dirname);
	free(dirname);
	return 1;
    }
    
    free(dirname);
    *handle = url;
    return 0;
}

static FILE *
open_file(const char *dir, const char *file, char **pfname)
{
    char *full_name = make_full_file_name(dir, file);
    FILE *fp = fopen(full_name, "r");
    if (!fp) {
	logmsg(L_ERR, errno, _("cannot open file `%s'"), full_name);
	free(full_name);
    } else
	*pfname = full_name;
    return fp;
}

static char *
find_key(FILE *fp, const char *key, char **pbuf, size_t *psize)
{
    ssize_t size;
    size_t keylen = strlen(key);
    
    while ((size = getline(pbuf, psize, fp)) > 0) {
	char *p = *pbuf;

	trimnl(p, size);
	while (*p && isspace(*p))
	    p++;
	if (!*p || *p == '#' || strlen(p) < keylen)
	    continue;
	if (memcmp(p, key, keylen) == 0) {
	    if (p[keylen] == 0)
		return p + keylen;
	    else if (isspace(p[keylen])) {
		for (p += keylen; *p && isspace(*p); p++)
		    ;
		return p;
	    }
	}
    }
    return NULL;
}

static int
dbtext_get_password(void *handle, const char *qpw, const char *key,
		    char **ppass)
{
    dico_url_t url = handle;
    char *dir = dico_url_full_path(url);
    char *full_name;
    FILE *fp = open_file(dir, qpw, &full_name);
    int rc;
	
    if (fp) {
	char *buf = NULL;
	size_t size = 0;
	char *val = find_key(fp, key, &buf, &size);
	if (val)
	    *ppass = xstrdup(val);
	else
	    *ppass = NULL;
	fclose(fp);
	free(full_name);
	free(buf);
	rc = 0;
    } else
	rc = 1;
    free(dir);
    return rc;
}

static int
dbtext_get_groups(void *handle, const char *qgr, const char *key,
		  char ***pgroups)
{
    dico_url_t url = handle;
    char *dir = dico_url_full_path(url);
    char *full_name;
    FILE *fp = open_file(dir, qgr, &full_name);
    int rc;
	
    if (fp) {
	char *buf = NULL;
	size_t size = 0;
	int count = 0, i = 0;
	char **groups = NULL;
	char *val;

	while ((val = find_key(fp, key, &buf, &size))) {
	    if (i == count) {
		if (count == 0)
		    count = 10; /* Initial allocation */
		groups = x2nrealloc(groups, &count, sizeof(groups[0]));
		groups[i++] = xstrdup(val);
	    }
	}
	if (groups && i == count) {
	    groups = xnrealloc(groups, count+1, sizeof(groups[0]));
	    groups[i++] = NULL;
	}

	fclose(fp);
	free(full_name);
	free(buf);
	rc = 0;
    } else
	rc = 1;
    return rc;
}

struct udb_def text_udb_def = {
    "text",
    dbtext_open,
    NULL,
    dbtext_get_password,
    dbtext_get_groups
};
