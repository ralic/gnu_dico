/* $Id: bushu.code.c,v 1.1 2001/11/03 22:35:32 gray Exp $ */

int
get_bushu(ind)
    int ind;
{
    return bushu_num[ind];
}

void
format_bushu(buf, radical, strokes)
    char *buf;
    int radical;
    int strokes;
{
    int delim;
    
    if (bushu_stroke[radical] > 0) {
	delim = '.';
	strokes -= bushu_stroke[radical];
    } else {
	delim = ':';
    }
    sprintf(buf, "%d%c%d", radical, delim, strokes);
}
    
int
parse_bushu(str, return_bushu)
    char *str;
    TempBushu *return_bushu;
{
    int num, radical;
    int rc;
    int off = 0;
    
    radical = strtol(str, (char**)&str, 10);
    switch (*str) {
    case '.':
	num = strtol(str+1, (char**)&str, 10);
	if (bushu_stroke[radical] < 0) 
	    rc = BushuFuzzy;
	else
	    rc = BushuStrict;
	break;
    case ':':
	num = strtol(str+1, (char**)&str, 10);
	rc = BushuAbsolute;
	break;
    default:
	msg(RC_BADINPUT, "invalid bushu pattern: '.' or ':' expected");
	return BushuError;
    }
    if (*str && *str == ',') {
	off = strtol(str+1, (char**)&str, 10);
	if (off > num) {
	    msg(RC_BADINPUT, "invalid bushu pattern: bad offset");
	    return BushuError;
	}
	if (rc != BushuFuzzy)
	    off = 0;
    } 
    
    while (*str && isspace(*str))
	str++;
    if (*str) {
	msg(RC_BADINPUT, "invalid bushu pattern: junk at the end");
	return BushuError;
    }
    return_bushu->type = rc;
    return_bushu->bushu = radical;
    return_bushu->numstrokes = num;
    return_bushu->off = off;
    return rc;
}

int
decode_bushu(dir, b, return_bushu)
    Matchdir dir;
    TempBushu *b;
    Bushu *return_bushu;
{
    int num;

    return_bushu->bushu = b->bushu;
    switch (b->type) {
    case BushuStrict:
	return_bushu->numstrokes = b->numstrokes + bushu_stroke[b->bushu];
	break;
    case BushuAbsolute:
	return_bushu->numstrokes = b->numstrokes;
	break;
    case BushuFuzzy:
	num = bushu_var_stroke[-bushu_stroke[b->bushu] + b->off];
	if (!num) {
	    if (dir == MatchNext)
		b->off = 0;
	    else {
		num = -bushu_stroke[b->bushu]+1;
		for (b->off = 0; bushu_var_stroke[num + b->off]; b->off++) ;
	    }
	    return 1;
	}
	return_bushu->numstrokes = b->numstrokes + num;
    }
    return 0;
}
