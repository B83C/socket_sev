#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdlib.h>

#define MAX_PROTOCOL_LEN 4 //0.9 1.0 1.1 2
#define MAX_HEADER_ENTRIES 16
#define MAX_PATH 100
#define MAX_ENTRIES_NAME 40
#define MAX_ENTRIES_VAL 260

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

//Trying to align the bytes
struct http_header
{
    int32_t method; //4 bytes
    int32_t protocol_ver;//4 bytes
    char path[MAX_PATH]; //100 bytes
    char h_names[MAX_HEADER_ENTRIES][MAX_ENTRIES_NAME]; //640 bytes
    char h_val[MAX_HEADER_ENTRIES][MAX_ENTRIES_VAL]; //4160 bytes
};

int ParseHeader(char* header, int len, struct http_header* hh);
#endif // PARSER_H_INCLUDED
