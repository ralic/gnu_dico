/* This file is part of Gjdict.
   Copyright (C) 1998-2000, 2008 Sergey Poznyakoff

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
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gjdict.h>

void
xmalloc_die()
{
    die(1, L_CRIT, 0, "Not enough memory");
}

void *
xmalloc(size_t size)
{
    void *p = malloc(size);
    if (!p)
	xmalloc_die();
    return p;
}

void *
xzalloc(size_t size)
{
    void *p = xmalloc(size);
    memset(p, 0, size);
    return p;
}

void *
xcalloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if (!p)
	xmalloc_die();
    return p;
}

void *
xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p)
	xmalloc_die();
    return p;
}



