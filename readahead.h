#ifndef READAHEAD_DEC
#define READAHEAD_DEC
#include "config.h"

struct readahead_t 
{
    char base[256];
    char* basepointer; 
    int __dirfd;
};

struct file_data
{
    char* data;
    size_t data_len;
};

struct file_cache
{
    char** filenames;
    struct file_data* data;
    int entry_len;
};

struct file_data* retrievefile(cmph_t*, struct file_cache*,  char*, size_t);
void readahead_init(int, struct file_cache* pool, cmph_t**);
#endif
