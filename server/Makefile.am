# $Id: Makefile.am,v 1.1 2001/11/03 22:35:02 gray Exp $

dictdir=@dictdir@

INCLUDES = -I${top_srcdir}/include -I${top_srcdir}/lib
AM_YFLAGS=-vt
AM_CFLAGS="-DDICTPATH=\"$(dictdir)\""

bin_PROGRAMS=gjdictd
gjdictd_SOURCES=\
 bushu.c\
 main.c\
 server.c\
 gram.y\
 log.c\
 dict.c\
 utils.c\
 iputil.c\
 search.c\
 log.h\
 server.h\
 patchlevel.h\
 syntax.h

gjdictd_LDADD=-L../dbs -ldbs

bushu.c: $(top_srcdir)/lib/bushu.dat $(srcdir)/bushu.code.c
	$(AWK) -f $(top_srcdir)/utils/bushu.awk $(top_srcdir)/lib/bushu.dat |\
                  cat - $(srcdir)/bushu.code.c > bushu.c


EXTRA_DIST = bushu.code.c

