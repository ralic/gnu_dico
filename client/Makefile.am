INCLUDES = @X_CFLAGS@ -I${top_srcdir}/include -I${top_srcdir}/lib

bin_PROGRAMS=gjdict
pkgdata_DATA=gjdict.chl mini-gjdict.xpm

gjdict_LDADD= @XLDADD@ @X_PRE_LIBS@ -lX11 @X_EXTRA_LIBS@ -L../dbs -ldbs -L../lib -lwchar 
gjdict_LDFLAGS=@X_LIBS@

gjdict_SOURCES=\
 bushu.c\
 bushusearch.c\
 client.c\
 convert.c\
 cornersearch.c\
 cystack.c\
 getfile.c\
 help.c\
 helpfile.c\
 init.c\
 kanasearch.c\
 license.c\
 log.c\
 main.c\
 msg.c\
 options.c\
 search.c\
 skipsearch.c\
 util.c\
 widgets.c\
 xref.c\
 client.h\
 cystack.h\
 help.h\
 log.h\
 options.h\
 search.h\
 util.h\
 version.h

bushu.c: $(top_srcdir)/lib/bushu.dat $(srcdir)/bushu.code.c
	$(AWK) -f $(top_srcdir)/utils/bushu.awk $(top_srcdir)/lib/bushu.dat |\
		 cat - $(srcdir)/bushu.code.c > bushu.c

gjdict.chl: gjdict.help tmac.gjdict
	$(top_srcdir)/utils/helpcmp.pl \
               -v -mgjdict -ogjdict.chl -in0 -po2n -ll55n gjdict.help

EXTRA_DIST=gjdict.help tmac.gjdict Gjdict.ad bushu.code.c $(pkgdata_DATA)
