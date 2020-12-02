#include "parser.h"

int ParseHeader(char* header, int len, struct http_header* hh)
{
    if(len <= 0) return 401;
    int Val = 1;
    for(int c = 0, i = 0, f = 0, l = 0;;)
    {
	if((*header == '\r' && *(header + 1) == '\n'))
	{
	    c++;
	    Val = 1;
	    f = 0;
	    i = 0;
	    header++;
	    l++;
	    goto SKIP;
	}
	if(*header == ' ')
	{
	    f += Val;
	    i = !Val;
	    goto SKIP;
	}
	if(*header == ':' && *(header + 1) == ' ')
	{
	    f++;
	    i = 0;
	    header++;
	    Val = 0;
	    l++;
	    goto SKIP;
	}
	if(c == 0)
	{
	    switch(f)
	    {
		case 0:
		    hh->method += *header;
		    break;
		case 1:
		    if(i == 0 && *header == '/') goto SKIP;
		    hh->path[i*(i <= MAX_PATH)] = *header;
		    i++;
		    break;
		case 2:
		    //assuming that the version number starts at 6th character
		    //if(*header == '/') { b = 1; goto SKIP;}
		    //if(b == 0) goto SKIP;
		    //hh->protocol_ver[i*(i < MAX_PROTOCOL_LEN)] = *header;
		    //i++;
		    hh->protocol_ver = *(header + 5)*10 + *(header + 7) - 528;
		    c++;
		    Val = 1;
		    f = 0;
		    i = 0;
		    header++;
		    l++;
		    goto SKIP;
	    }
	}
	else
	{
	    switch(f)
	    {
		case 0:
		    hh->h_names[c - 1][i *(i<= MAX_ENTRIES_VAL)]  = *header;
		    i++;
		    break;
		case 1:
		    hh->h_val[c - 1][i * (i <= MAX_ENTRIES_VAL)] = *header;
		    i++;
		    break;
	    }
	}
SKIP:
	header++;
	l++;
	if(l >= len)
	{

	    return 0;
	}
    }
}
