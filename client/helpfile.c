/* $Id: helpfile.c,v 1.1 2001/11/03 22:36:00 gray Exp $
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>

#include <client.h>
#include <help.h>

typedef struct {
    long numtopics;
    long text_offset;
    long xref_offset;
    long xref_length;
    long titles_start;
    long titles_length;
} HelpHeader;

typedef struct {                                                  
    long start;
    long length;
    long title;
    long xref;
    long encoding;
} TopicHeader;

HelpHeader hdr;
TopicHeader *topics;
char *titles;
FILE *fp;
struct {
    String ptr;
    int size;
} page;
long *cross_refs;
String *cross_ref_str;
long max_cross_refs;

int
FindTopic(ttl)
    String ttl;
{
    int i;
    
    for (i = 0; i < hdr.numtopics; i++)
	if (strcasecmp(ttl, GetTopicTitle(i)) == 0)
	    return i;
    return -1;
}

String
GetTopicTitle(n)
    int n;
{
    return titles + topics[n].title;
}

String
GetTopicText(n)
    int n;
{
    if (page.size < topics[n].length+1) {
	if (page.ptr)
	    free(page.ptr);
	page.ptr = malloc(topics[n].length+1);
	if (!page.ptr) {
	    Beep();
	    page.size = 0;
	    return "Not enough memory to display help entry";
	}
	page.size = topics[n].length+1;
    }
    fseek(fp, hdr.text_offset + topics[n].start, SEEK_SET);
    fread(page.ptr, 1, topics[n].length, fp);
    page.ptr[ topics[n].length ] = 0;
    return page.ptr;
}

int
LoadHelpFile(return_str)
    String **return_str;
{
    int i;
    int xref_size;
    String *str;

    fp = fopen(config.helpfilename, "r");
    if (!fp) {
	syserr("Can't open help file `%s'", config.helpfilename);
	return 0;
    }

    if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
	syserr("read error on `%s'", config.helpfilename);
	fclose(fp);
	return 0;
    }
    topics = calloc(hdr.numtopics, sizeof(topics[0]));
    if (!topics) {
	error("%s:%d: low core!", __FILE__, __LINE__);
	fclose(fp);
	return 0;
    }
    titles = malloc(hdr.titles_length);
    if (!titles) {
	error("%s:%d: low core!", __FILE__, __LINE__);
	fclose(fp);
	free(topics);
	return 0;
    }

    if (fread(topics, sizeof(topics[0]), hdr.numtopics, fp) != hdr.numtopics) {
	syserr("read error on `%s'", config.helpfilename);
	fclose(fp);
	free(topics);
	free(titles);
	return 0;
    }

    if (fread(titles, 1, hdr.titles_length, fp) != hdr.titles_length) {
	syserr("read error on `%s'", config.helpfilename);
	fclose(fp);
	free(topics);
	free(titles);
	return 0;
    }

    str = calloc(hdr.numtopics, sizeof(str[0]));
    if (!str) {
	error("%s:%d: low core!", __FILE__, __LINE__);
	fclose(fp);
	free(topics);
	free(titles);
	return 0;
    }

    xref_size = sizeof(long)*hdr.xref_length;
    cross_refs = malloc(xref_size);
    if (!cross_refs) {
	error("%s:%d: low core!", __FILE__, __LINE__);
	fclose(fp);
	free(topics);
	free(titles);
	return 0;
    }
    if (fread(cross_refs, 1, xref_size, fp) != xref_size) {
	syserr("read error on `%s'", config.helpfilename);
	fclose(fp);
	free(topics);
	free(titles);
	free(cross_refs);
	return 0;
    }
    
    for (i = 0; i < hdr.numtopics; i++) {
	str[i] = titles + topics[i].title;
	if (topics[i].xref >= 0 && cross_refs[topics[i].xref] > max_cross_refs)
	    max_cross_refs = cross_refs[topics[i].xref];
    }
    cross_ref_str = calloc(max_cross_refs, sizeof(cross_ref_str[0]));
    if (!cross_ref_str) {
	error("%s:%d: low core!", __FILE__, __LINE__);
	fclose(fp);
	free(topics);
	free(titles);
	free(cross_refs);
	return 0;
    }
    
    *return_str = str;
    return hdr.numtopics;
}

int
LoadTopicXref(n, xref)
    int n;
    TopicXref *xref;
{
    int i, start;

    if (topics[n].xref == -1) {
	xref->cnt = 0;
	return 0;
    }
    start = topics[n].xref+1;
    xref->cnt = cross_refs[ topics[n].xref ];
    for (i = 0; i < xref->cnt; i++) 
	cross_ref_str[i] = titles + topics[ cross_refs[start+i] ].title;
    xref->str = cross_ref_str;
    xref->offs = cross_refs + start;
    return xref->cnt;
}
