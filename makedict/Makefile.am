# $Id: Makefile.am,v 1.1 2001/11/03 22:34:34 gray Exp $
dictdir=@dictdir@
inputdictpath=@inputdictpath@
bin_PROGRAMS=makedict btree
makedict_SOURCES=main.c compile.c utils.c output.c index.c romaji.c makedict.h 
btree_SOURCES=btree.c check.c info.c
makedict_LDADD=-L../dbs -ldbs
btree_LDADD=-L../dbs -ldbs
AM_CFLAGS=-DDICTPATH=\"$(dictdir)\" -DINPUTDICTPATH=\"$(inputdictpath)\"
INCLUDES=-I$(top_srcdir)/include

