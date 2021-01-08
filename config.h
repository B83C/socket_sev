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
#include<signal.h>
#include<fcntl.h>

#include<unistd.h>
#include<linux/sockios.h>
#include<sys/ioctl.h>
#include<pthread.h>
#include<netinet/in.h>
#include<netinet/tcp.h>

#ifndef O_PATH
#define O_PATH 010000000
#endif

#define PORT 12354
#define MAX_BUFLEN 5840
#define MAX_CONN_PER_LISTENER 10000
#define MAX_EVENTS 10000

#define KEEPALIVE_TIMEOUT 10

#define HTML_HOME "html"

#define MAIN_PAGE "index.html"

#define LISTENING_THREADS 1
