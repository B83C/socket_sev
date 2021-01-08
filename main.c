//For now, I will use epoll for the event handling, and in the not-so-distant future, I might well switch to signal-per-fd approach. or AIO?
//To give you an idea of what I am doing, here's a diagram:
//			        LISTENERS
//			    -----------------
//             LISTENER 1      LISTENER 2       LISTENER 3 .........
//		    \              ||               /
//		     --------------||---------------
//				 epollfd
//				   ||
//		     -------------------------------
//		    /              ||               \
//	       worker1          worker2          worker3 ............
//		  |                |                |
//		--------------------------------------
//				WORKERS
//Well it turns out no, the epoll implementation itself has some prob...
#include "cache.h"
#include "config.h"
#include "parser.h"
#include "headerhash.h"
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define CONN_FD(x) (((struct conn_t*)x.data.ptr)->fd)
#define CONN_BUF(x) (((struct conn_t*)x.data.ptr)->buf)
#define CONN_HEAD(x) (&((struct conn_t*)x.data.ptr)->header)
#define SHUTDOWN_CONN(x)	    \
{	    \
    free(x.data.ptr); \
    shutdown(CONN_FD(x), SHUT_RDWR);	    \
    close(CONN_FD(x));	    \
}	    

#define STRa(x) #x
#define STR(x) STRa(x)

#define HTML_404 "<html><body>CAN'T FIND THE FILE FOR YOU</body></html>"
#define HTML_LEN_404 35

#define HTML_413 "<html><body>WHAT IS WRONG WITH YOU</body></html>"
#define HTML_LEN_413 48

#define ERROR(x) "HTTP/1.1 " #x " \r\nConnection: close\r\nContent-Length: " STR(HTML_LEN_ ## x) "\r\n\r\n" HTML_ ## x 
#define ERROR_LEN(x) __builtin_strlen(ERROR(x))

#define HTTP_OK "HTTP/1.1 200 OK\r\n" "Content-Type: text/html; charset=UTF-8\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=" STR(KEEPALIVE_TIMEOUT) "\r\nContent-Length: %d\r\n\r\n%s"

#define DIE_WITH_ERROR(x, y)  send( CONN_FD(x) , ERROR(y), ERROR_LEN(y), 0) 
int dirfd_;


struct conn_t 
{
    header_t header;
    char buf[MAX_BUFLEN];
    int fd;
}__attribute__((aligned(16)));

int64_t listeners[LISTENING_THREADS]; 

void* siginthandler(int dummy __attribute__((unused)))
{
    printf("Received SIGINT\n");
    //shutdown_conn(listener[1]);
    exit(-1);
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
		//printf("Connection closing for %d\n" , CONN_FD(events[i]));
		continue;
	    }
	    ioctl(CONN_FD(events[i]), SIOCINQ, &recvlength);
	    if(recvlength > MAX_BUFLEN)
	    {
		DIE_WITH_ERROR(events[i], 413);
	    }
	    else if(recvlength > 0)
	    {
		int iResult= recv(CONN_FD(events[i]), CONN_BUF(events[i]), &recvlength, 0);
		ParseHeader(CONN_BUF(events[i]), CONN_HEAD(events[i]));
		char* temp = RetrieveFile(CONN_HEAD(events[i])->path, strlen(CONN_HEAD(events[i])->path), dirfd_);
		if(temp == -1 )
		{
		    DIE_WITH_ERROR(events[i], 404);
		}
		else
		{
		    sprintf(tmp, HTTP_OK, strlen(temp), temp);
		    send(CONN_FD(events[i]), tmp, strlen(tmp), 0);
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
    }

}

int main()
{
    pthread_t workers[LISTENING_THREADS];
    struct sockaddr_in sock_opt = {
	.sin_family= AF_INET, 
	.sin_addr.s_addr = INADDR_ANY, 
	.sin_port = htons(PORT)
    };

    if((dirfd_ = open(HTML_HOME,  O_PATH)) == -1)
    {
	fprintf(stderr, "Unable to retrieve path : %s\n", strerror(errno));
	exit(-1);
    }


    for(int i = 0; i < LISTENING_THREADS; i++)
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
    //if((servfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    //{
    //    printf("Failed. Error Code : %d\n", strerror(errno));
    //    goto CLEANUP;
    //}

    printf("Listeners initialized!\nWaiting for foreign connections\n");

    //pthread_create(&pump, NULL, (void*)accept_pump, NULL);

    struct sigaction sigact;
    bzero(&sigact, sizeof(sigact));
    sigact.sa_handler = siginthandler; 
    sigaction(SIGINT, &sigact, NULL); 

    sleep(10000000);

CLEANUP:
    for(int i = 0; i < LISTENING_THREADS; i++)
    {
	close(listeners[i]);
	pthread_cancel(workers[i]);
    }
    printf("Server terminating...\n");
    pthread_exit(NULL);
    /*
       while((news = accept(s, (struct sockaddr*)&client, &clen)) != -1)
       {
       printf("Connection accepted\n");

       iResult = recv(news, RecvBuf, MAX_BUFLEN, 0);

       if(iResult <= 0)
       {
       printf("Connection closing...\n");
       goto CLEANUP;
       }

       printf("%s\n", RecvBuf);

       ParseHeader(RecvBuf, &header);

       for(int i = 0; i < 35; i++)
       {
       printf("%d Val: %s\n" ,i,  header.h_val[i]);
       }

       handle(&header, &news);

       printf("Response sent!\n");

       shutdown(news, SHUT_RDWR);
       close(news);
       bzero(&header, sizeof(struct http_header));
       bzero(&RecvBuf, sizeof(char) * 512);
       }
       */

    //    printf("Unable to accept connection : %d\n", strerror(errno));
    //
    //CLEANUP:
    //    close(s);
    //close(news);
    return 0;
}
