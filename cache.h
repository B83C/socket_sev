#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include "config.h"
#include "xxhash.h"

static void* cache[MAX_CACHE_FILE];
static XXH64_hash_t key[MAX_CACHE_FILE];
static int acc_count[MAX_CACHE_FILE];
static int arr_len = 0;

char* RetrieveFile(char* name, int len, int dirfd);

#endif
