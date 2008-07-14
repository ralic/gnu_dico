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

/* A "transcript stream" transparently writes data to and reads data from
   an underlying transport stream, writing each lineful of data to a "log
   stream". Writes to log stream are prefixed with a string indicating
   direction of the data (read/write). Default prefixes are those used in
   most RFCs - "S: " for data written ("Server"), and "C: " for data read
   ("Client"). */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <string.h>
#include <xalloc.h>

#define TRANS_READ 0x1
#define TRANS_WRITE 0x2
#define FLAG_TO_PFX(c) ((c) - 1)

struct transcript_stream {
    int flags;
    dico_stream_t transport;
    dico_stream_t logstr;
    char *prefix[2];
};

static void
print_transcript(struct transcript_stream *str, int flag,
		 const char *buf, size_t size)
{
    while (size) {
	const char *p;
	size_t len;
	
	if (str->flags & flag) {
	    dico_stream_write(str->logstr,
			      str->prefix[FLAG_TO_PFX(flag)],
			      strlen(str->prefix[FLAG_TO_PFX(flag)]));
	    str->flags &= ~flag;
	}
	p = memchr(buf, '\n', size);
	if (p) {
	    len = p - buf;
	    if (p > buf && p[-1] == '\r')
		len--;
	    dico_stream_write(str->logstr, buf, len);
	    dico_stream_write(str->logstr, "\n", 1);
	    str->flags |= flag;

	    len = p - buf + 1;
	    buf = p + 1;
	    size -= len;
	} else {
	    dico_stream_write(str->logstr, buf, size);
	    break;
	}
    }
}

static int
transcript_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct transcript_stream *p = data;
    size_t nbytes;

    if (dico_stream_read(p->transport, buf, size, &nbytes) == 0) {
	print_transcript(p, TRANS_READ, buf, nbytes);
	if (pret)
	    *pret = nbytes;
    } else
	return dico_stream_last_error(p->transport);
    return 0;
}

static int
transcript_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct transcript_stream *p = data;
    if (dico_stream_write(p->transport, buf, size) == 0) {
	print_transcript(p, TRANS_WRITE, buf, size);
	if (pret)
	    *pret = size;
    } else
	return dico_stream_last_error(p->transport);
    return 0;
}

static int
transcript_flush(void *data)
{
    struct transcript_stream *p = data;
    return dico_stream_flush(p->transport);
}

static int
transcript_close(void *data)
{
    struct transcript_stream *p = data;
    return dico_stream_close(p->logstr);
    return dico_stream_close(p->transport);
}
    

static int
transcript_destroy(void *data)
{
    struct transcript_stream *p = data;
    free(p->prefix[0]);
    free(p->prefix[1]);
    dico_stream_destroy(&p->transport);
    dico_stream_destroy(&p->logstr);
    free(p);
    return 0;
}

static const char *
transcript_strerror(void *data, int rc)
{
    struct transcript_stream *p = data;
    return dico_stream_strerror(p->transport, rc);
}

const char *default_prefix[2] = {
    "C: ", "S: "
};

dico_stream_t
xdico_transcript_stream_create(dico_stream_t transport, dico_stream_t logstr,
			       const char *prefix[])
{
    struct transcript_stream *p = xmalloc(sizeof(*p));
    dico_stream_t stream;
    int rc = dico_stream_create(&stream, DICO_STREAM_READ|DICO_STREAM_WRITE, p);
    if (rc)
	xalloc_die();
    p->flags = TRANS_READ | TRANS_WRITE;
    if (prefix) {
	p->prefix[0] = xstrdup(prefix[0] ? prefix[0] : default_prefix[0]);
	p->prefix[1] = xstrdup(prefix[1] ? prefix[1] : default_prefix[1]);
    } else {
	p->prefix[0] = xstrdup(default_prefix[0]);
	p->prefix[1] = xstrdup(default_prefix[1]);
    }
    p->transport = transport;
    p->logstr = logstr;
    
    dico_stream_set_read(stream, transcript_read);
    dico_stream_set_write(stream, transcript_write);
    dico_stream_set_flush(stream, transcript_flush);
    dico_stream_set_close(stream, transcript_close);
    dico_stream_set_destroy(stream, transcript_destroy);
    dico_stream_set_error_string(stream, transcript_strerror);
    dico_stream_set_buffer(stream, dico_buffer_line, 1024);

    return stream;
}
