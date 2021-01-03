#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdlib.h>
#include "headerhash.h"

#define MAX_PROTOCOL_LEN 4 //0.9 1.0 1.1 2

//Given that the collision rate is fairly low, I think this is okay.
//Method values ( which is the sum of their ascii values )
#define GET 224
#define POST 326
#define HEAD 274
#define PUT 249
#define DELETE 435
#define CONNECT 522
#define OPTIONS 556
#define TRACE 367

struct http_header
{
    int32_t method; //4 bytes
    int32_t protocol_ver;//4 bytes
    char* path;
    char* h_val[TOTAL_KEYWORDS]; 
};

int ParseHeader(char* header, struct http_header* hh);
#endif // PARSER_H_INCLUDED
