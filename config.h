#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string.h>

#include<sys/mman.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/epoll.h>
#include<signal.h>
#include<fcntl.h>

#include<unistd.h>
#include<linux/sockios.h>
#include<sys/ioctl.h>
#include<pthread.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<sys/inotify.h>
#include<dirent.h>
#include"cmph/cmph.h"
#include"libdeflate.h"

#ifndef O_PATH
#define O_PATH 010000000
#endif

#define PORT 8080

#define MAX_BUFLEN 5840
#define MAX_CONN_PER_LISTENER 10000
#define MAX_EVENTS 10000

#define KEEPALIVE_TIMEOUT 10
#define MAX_INITIAL_FILENUM 50

#define HTML_HOME "./html/"

#define MAIN_PAGE "homepage.html"

#define WORKER_THREADS 1

#define MAX_CACHE_FILE 15

#define DEFLATE_COMPRESSION_LEVEL 6 //1.. 6 .. 12
