#ifndef WRAPPER_H_INCLUDED
#define WRAPPER_H_INCLUDED

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string.h>

#include<sys/mman.h>
#include<sys/stat.h>

#include<sys/types.h>
#include<fcntl.h>

#include<unistd.h>
#include<pthread.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SOCKET int

#ifndef O_PATH
#define O_PATH 010000000
#endif
extern int errno;

int GetError();

#define PORT 8080
#define MAX_BUFLEN 512
#define MAX_CACHE_FILE 15

#define HTML_HOME "html"

#define MAX_THREADS 10

#endif // WRAPPER_H_INCLUDED
