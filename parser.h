#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdlib.h>
#include "headerhash.h"

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

typedef struct http_header
{
    char* path;
    char* h_val[TOTAL_KEYWORDS]; 
    int32_t method; //4 bytes
    int32_t protocol_ver;//4 bytes
} header_t;

int ParseHeader(char* header, struct http_header* hh);
char* base64_encode(char*, size_t);
#endif // PARSER_H_INCLUDED
