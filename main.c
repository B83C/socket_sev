//For now, I will use epoll for the event handling, and in the not-so-distant future, I might well switch to signal-per-fd approach. or AIO? or even NETMAP??
//
//To give you an idea of what I am doing, here's a diagram:
//			        LISTENERS
//			    -----------------
//             LISTENER 1      LISTENER 2       LISTENER 3 .........
//		|   \              ||               / |
//		|    --------------||---------------  | 
//	(per thread) 	    	epollfd       (per thread)
//	        |		   ||                 |   
//		|    -------------------------------  |    
//		|   /              ||               \ |    
//	       worker1          worker2          worker3 ............
//		  |                |                |
//		--------------------------------------
//				WORKERS
//Well it turns out no, the epoll implementation itself has some prob...
//To my surprise, TCP_KEEPIDLE and its companion don't work as expected 
//I was expecting it to work like how http keepalive does, but it turns out no 
#include "config.h"
#include "parser.h"
#include "headerhash.h"
#include "readahead.h"

#include<time.h>
struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
	temp.tv_sec = end.tv_sec-start.tv_sec-1;
	temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
	temp.tv_sec = end.tv_sec-start.tv_sec;
	temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}
#define ALWAYS_INLINE inline __attribute__((always_inline))

#define CONN_FD(x) (((struct conn_t*)x.data.ptr)->fd)
#define CONN_BUF(x) (((struct conn_t*)x.data.ptr)->buf)
#define CONN_HEAD(x) (&((struct conn_t*)x.data.ptr)->header)
#define SHUTDOWN_CONN(x)	    \
{	    \
    shutdown(CONN_FD(x), SHUT_RDWR);	    \
    close(CONN_FD(x));	    \
    free(x.data.ptr); \
}	    

#define STRa(x) #x
#define STR(x) STRa(x)

#define HTML_404 "<html><body>CAN'T FIND THE FILE FOR YOU, MY DEAR</body></html>"
#define HTML_LEN_404 61

#define HTML_413 "<html><body>WHAT IS WRONG WITH YOU</body></html>"
#define HTML_LEN_413 48

#define ERROR(x) "HTTP/1.1 " #x " \r\nConnection: close\r\nContent-Length: " STR(HTML_LEN_ ## x) "\r\n\r\n" HTML_ ## x 
#define ERROR_LEN(x) __builtin_strlen(ERROR(x))

#define ERROR_FILE(x) "HTTP/1.1 " #x " \r\nContent-Length: 0\r\n\r\n"
#define ERROR_FILE_LEN(x) __builtin_strlen(ERROR_FILE(x))

#define HTTP_OK "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=" STR(KEEPALIVE_TIMEOUT) "\r\nContent-Encoding: deflate\r\nContent-Length: %d\r\nCache-Control: max-age=7200\r\n\r\n"
//#define HTTP_OK "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=" STR(KEEPALIVE_TIMEOUT) "\r\nContent-Length: %d\r\nCache-Control: max-age=7200\r\n\r\n"

#define DIE_WITH_ERROR(x, y)  send( CONN_FD(x) , ERROR(y), ERROR_LEN(y), 0) 
#define FILE_ERROR(x, y)  send( CONN_FD(x) , ERROR_FILE(y), ERROR_FILE_LEN(y), 0) 

int dirfd_;

struct conn_t 
{
    header_t header;
    char buf[MAX_BUFLEN];
    int fd;
}__attribute__((aligned(16)));

int64_t listeners[WORKER_THREADS]; 
pthread_t workers[WORKER_THREADS];
int inotifyFd;
struct file_cache fc;
cmph_t* mphash;

void* siginthandler(int dummy __attribute__((unused)))
{
    printf("Received SIGINT\n");
    for(int i = 0; i < WORKER_THREADS; i++)
    {
	close(listeners[i]);
	pthread_cancel(workers[i]);
    }
    exit(-1);
}

#define BUF_LEN (10 * (sizeof(struct inotify_event) + 1 + 1))
void* sigiohandler(int sig)
{
    char buf[BUF_LEN] __attribute__ ((aligned(8)));

    ssize_t numRead = read(inotifyFd, buf, BUF_LEN);
    if (numRead == 0) {
	fprintf(stderr, "read() from inotify fd returned 0!");
    }

    if (numRead == -1) {
	fprintf(stderr, "read");
    }

    printf("Read %ld bytes from inotify fd\n", (long) numRead);

    /* Process all of the events in buffer returned by read() */

    for (char *p = buf; p < buf + numRead; ) {
	struct inotify_event *event = (struct inotify_event *) p;
	printf("events!");
	p += sizeof(struct inotify_event) + event->len;
    }
}

ALWAYS_INLINE const char* url_map(char* buf, int* len)
{
    if(*len == 0)
    {
	buf = MAIN_PAGE;
	*len = sizeof(MAIN_PAGE) - 1;
    }
    struct file_data* tmpfd = retrievefile(mphash, &fc, buf, *len);
    if(tmpfd == -1) return -1;
    *len = tmpfd->data_len;
    return tmpfd->data;
}

//void accept_pump(void)
//{
//    struct sockaddr_in client;
//    int news;
//    int clen = sizeof(struct sockaddr_in);
//    ev.events = EPOLLIN | EPOLLET;
//    while((news = accept4(servfd,(struct sockaddr*)&client, &clen, SOCK_NONBLOCK )) != -1)
//    {
//	ev.data.fd = news; 
//	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, news, &ev) == -1)
//	{
//	    fprintf(stderr, "Unable to add socket to interest list : %s", strerror(errno));
//	} 
//
//    }
//    pthread_exit(NULL);
//}


void worker_proc(void* listener)
{
    int epollfd;
    if((epollfd = epoll_create1(0)) == -1)
    {
	fprintf(stderr, "Unable to create epoll file descriptor : %s", strerror(errno));
	exit(-1);
    }
    int new_sock;
    struct epoll_event events[MAX_EVENTS];
    struct conn_t* connection;
    struct epoll_event ev = {.data.fd = (int)listener, .events = EPOLLIN | EPOLLET};
    epoll_ctl(epollfd, EPOLL_CTL_ADD, (int)listener, &ev); 
    //ev.events |= EPOLLONESHOT;
    struct sockaddr_in client;
    size_t recvlength;
    int clen = sizeof(struct sockaddr_in);

    char tmp[5000];
    while(1)
    {
	int pending_num = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	if (pending_num == -1)
	{
	    perror("A worker has failed to wait for epoll events");
	    break;
	}	    
	    struct timespec start, end = {0};
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(int i = 0; i < pending_num; i++)
	{
	    if(events[i].data.fd == (int)listener)
	    {
ACCEPTMORE:
		new_sock = accept4(events[i].data.fd,(struct sockaddr*)&client, &clen, SOCK_NONBLOCK);
		if(new_sock != -1)
		{
		    connection = malloc(sizeof(struct conn_t));
		    connection->fd = new_sock;
		    ev.data.ptr = connection; 
		    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, new_sock, &ev) == -1)
		    {
			fprintf(stderr, "Unable to add socket to interest list : %s", strerror(errno));
		    }
		    if(errno != EAGAIN) goto ACCEPTMORE;
		}
		continue;
	    }
	    if(events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR))
	    {
		SHUTDOWN_CONN(events[i]);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, CONN_FD(events[i]), NULL);
		continue;
	    }
	    ioctl(CONN_FD(events[i]), SIOCINQ, &recvlength);
	    if(recvlength > MAX_BUFLEN)
	    {
		DIE_WITH_ERROR(events[i], 413);
	    }
	    else if(recvlength > 0)
	    {
		recv(CONN_FD(events[i]), CONN_BUF(events[i]), &recvlength, 0);
		ParseHeader(CONN_BUF(events[i]), CONN_HEAD(events[i]));
		int len = strlen(CONN_HEAD(events[i])->path);
		char* temp = url_map(CONN_HEAD(events[i])->path, &len);
		if(temp == -1)
		{
		    if(!CONN_HEAD(events[i])->h_val[Referer])
		    {
			DIE_WITH_ERROR(events[i], 404);
		    }
		    else
		    {
			FILE_ERROR(events[i], 404);
		    }
		}
		else
		{
		    sprintf(tmp, HTTP_OK, len);
		    send(CONN_FD(events[i]), tmp, strlen(tmp), MSG_MORE);
		    send(CONN_FD(events[i]), temp , len, 0);
		}
	    }
	    else
	    {
		SHUTDOWN_CONN(events[i]);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, CONN_FD(events[i]), NULL);
		//printf("Connection closing for %d\n" , CONN_FD(events[i]));
		continue;
	    }
	    //events[i].events |= EPOLLONESHOT;
	    //if(epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) == -1)
	    //{
	    //    fprintf(stderr, "Unable to del socket from interest list : %s", strerror(errno));
	    //} 
	    //	    shutdown(events[i].data.fd, SHUT_RDWR);
	    //	    close(events[i].data.fd);
	    //	    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	struct timespec di = diff(start, end);
	printf("\nTime taken: %d seconds %d nanoseconds\n",di.tv_sec, di.tv_nsec);
    }

    pthread_exit(NULL);
}

int main()
{
    struct sockaddr_in sock_opt = {
	.sin_family= AF_INET, 
	.sin_addr.s_addr = INADDR_ANY, 
	.sin_port = htons(PORT)
    };

    if((dirfd_ = open(HTML_HOME,  O_DIRECTORY)) == -1)
    {
	fprintf(stderr, "Unable to retrieve path : %s\n", strerror(errno));
	exit(-1);
    }
    fc.filenames = malloc(sizeof(char*) * MAX_INITIAL_FILENUM);
    fc.data = malloc(sizeof(struct file_data) * MAX_INITIAL_FILENUM);
    readahead_init(dirfd_, &fc, &mphash);

    for(int i = 0; i < WORKER_THREADS; i++)
    {
	if( (listeners[i] = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1 )
	{
	    perror("Unable to create socket for listener");
	    goto CLEANUP;
	}
	int val = 1;
	setsockopt(listeners[i], SOL_SOCKET, SO_ZEROCOPY, &val, sizeof(val));
	setsockopt(listeners[i], SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	setsockopt(listeners[i], SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
	val = 1;
	//	    setsockopt(listeners[i], IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val));
	//	    setsockopt(listeners[i], IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val));
	//	    setsockopt(listeners[i], IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val));
	//	    setsockopt(listeners[i], IPPROTO_TCP, TCP_USER_TIMEOUT, &val, sizeof(val));
	//setsockopt(listener[i], SOL_SOCKET, SO_REUSEADDR, 1, sizeof(int));
	if(bind(listeners[i], (struct sockaddr*)&sock_opt, sizeof(sock_opt)) == -1)
	{
	    perror("Unable to bind listener to specified port");
	    goto CLEANUP;
	}
	if(listen(listeners[i], MAX_CONN_PER_LISTENER) == -1)
	{
	    perror("Unable to listen on port");
	    goto CLEANUP;
	}
	printf("Spawned worker %d, FD : %d\n", i + 1, listeners[i]);
	pthread_create(&workers[i], NULL, (void*)worker_proc, (void*)listeners[i]);
    }

    printf("Listeners initialized!\nWaiting for foreign connections\n");

    struct sigaction sigact;
    bzero(&sigact, sizeof(sigact));
    sigact.sa_handler = siginthandler; 
    sigaction(SIGINT, &sigact, NULL); 

    //    inotifyFd = inotify_init();
    //    bzero(&sigact, sizeof(sigact));
    //    //sigact.sa_flags = SA_RESTART;
    //    sigact.sa_handler = sigiohandler; 
    //    sigaction(SIGIO, &sigact, NULL); 
    //    fcntl(inotifyFd, F_SETOWN, getpid());
    //fcntl(inotifyFd, F_SETFL, fcntl(inotifyFd, F_GETFL) | O_ASYNC | O_NONBLOCK);
    //inotify_add_watch(inotifyFd, HTML_HOME, IN_DELETE | IN_MODIFY | IN_CREATE );

    sleep(20000000);
    //worker_proc(listeners[0]);

CLEANUP:
    for(int i = 0; i < WORKER_THREADS; i++)
    {
	close(listeners[i]);
	pthread_cancel(workers[i]);
    }
    printf("Server terminating...\n");
    pthread_exit(NULL);
    return 0;
}
