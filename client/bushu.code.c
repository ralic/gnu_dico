/* $Id: bushu.code.c,v 1.1 2001/11/03 22:36:43 gray Exp $ */

int
GetBushu(ind)
    int ind;
{
    return bushu_num[ind];
}

void
print_bushu(buf, radical, strokes)
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









