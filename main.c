//For now, I will use epoll for the event handling, and in the not-so-distant future, I might well switch to signal-per-fd approach.
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
#include "cache.h"
#include "config.h"
#include "parser.h"
#include "headerhash.h"
#define ALWAYS_INLINE inline __attribute__((always_inline))

char *response_headers = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\nConnction: close\r\n\r\n fasdfkja;sdlkjaf;sdfkjasdfkadfha;ssdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;laknj;falnj;falsdfj;alkj;asdfkja;sdlkjsdfj;lak";
int dirfd_;

enum CON_MODE { READ, WRITE, IDLE };

struct conn_t
{
    header_t header;
    char* buf;
    enum CON_MODE mode;
    int fd;
};


void ALWAYS_INLINE shutdown_conn(int fd)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

int handle(struct http_header* header, int* sock )
{
    //if(header->protocol_ver < 11)
    //{
    //    //backwards compatibility, meaning no persistent socket is supported
    //    sprintf(response_header, "HTTP/%d %d\r\n\r\nHTTP Version Not Supported", header->protocol_ver, 505);
    //    send(*sock, response_header, strlen(response_header), MSG_ZEROCOPY);
    //    return -1;
    //}
    char* file;
    switch(header->method)
    {
	case GET:

	    file = RetrieveFile(header->path + 1, strlen(header->path) - 1, dirfd_);
	    if(file == -1)
	    {
		printf("Error occurred!\n");
		return -1;
	    }
	    send(*sock, response_headers, strlen(response_headers), MSG_MORE);
	    send(*sock, file, strlen(file), MSG_ZEROCOPY);
	    break;
	case POST:
	    break;
    }
}

pthread_t pump;
int listeners[LISTENING_THREADS]; 

void* siginthandler(int dummy __attribute__((unused)))
{
    printf("Received SIGINT\n");
    //shutdown_conn(listener[1]);
    pthread_cancel(pump);
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

int check(int fd)
{
    for(int i = 0; i< LISTENING_THREADS; i++)
    {
	if(listeners[i] == fd) return 0;
    }
    return 1;
}

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
    struct epoll_event ev = {.data.fd = (int)listener, .events = EPOLLIN | EPOLLET};
    epoll_ctl(epollfd, EPOLL_CTL_ADD, (int)listener, &ev); 
    ev.events |= EPOLLONESHOT;
    char tmp[5000];
    char RecvBuf[MAX_BUFLEN] = {0};
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
		struct sockaddr_in client;
		int clen = sizeof(struct sockaddr_in);
ACCEPTMORE:
		new_sock = accept4(events[i].data.fd,(struct sockaddr*)&client, &clen, SOCK_NONBLOCK);
		if(new_sock != -1)
		{
		    ev.data.fd = new_sock; 
		    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, new_sock, &ev) == -1)
		    {
			fprintf(stderr, "Unable to add socket to interest list : %s", strerror(errno));
		    }
		    if(errno != EAGAIN) goto ACCEPTMORE;
		}
		continue;
	    }
	    if(events[i].events & ( EPOLLRDHUP | EPOLLRDHUP | EPOLLERR))
	    {
		shutdown(events[i].data.fd, SHUT_RDWR);
		close(events[i].data.fd);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
		printf("Connection closing for %d\n" , events[i].data.fd);
		continue;
	    }
	    int iResult;
	    header_t test;
	    int length;
READ:
	    ioctl(events[i].events, SIOCINQ, &length);
	    iResult= recv(events[i].data.fd, RecvBuf, MAX_BUFLEN, 0);
	    //if(!(iResult < MAX_BUFLEN || errno == EAGAIN)) goto READ;
	    char* temp = RetrieveFile("test.html", 9, dirfd_);
	    sprintf(tmp, "%s%s", response_headers, temp);
	    send(events[i].data.fd, tmp, strlen(tmp), MSG_ZEROCOPY);
	    ParseHeader(RecvBuf, &test);

	    //	printf("%s\n", test.h_val[Host]);
	    //    send(events[i].data.fd, "<html>NOOB</html>\0", strlen("<html>NOOB</html>\0"), MSG_ZEROCOPY);
	    events[i].events |= EPOLLONESHOT;
	    if(epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) == -1)
	    {
		fprintf(stderr, "Unable to del socket from interest list : %s", strerror(errno));
	    } 
	    shutdown(events[i].data.fd, SHUT_RDWR);
	    close(events[i].data.fd);
	    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
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
	setsockopt(listeners[i], SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
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
