#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include "config.h"

static void* cache[MAX_CACHE_FILE];
static int key[MAX_CACHE_FILE];
static int acc_count[MAX_CACHE_FILE];
static int arr_len = 0;

char* RetrieveFile(char* name, int len, int dirfd);

#endif
