#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include "xxhash.h"
#include <stdio.h>
#include <errno.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string.h>

#include<sys/mman.h>
#include<sys/stat.h>

#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>

#include<unistd.h>
#define MAX_CACHE_FILE 15

static void* cache[MAX_CACHE_FILE];
static XXH64_hash_t key[MAX_CACHE_FILE];
static int acc_count[MAX_CACHE_FILE];
static int arr_len = 0;

char* RetrieveFile(char* name, int len, int dirfd);

#endif
