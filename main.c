//For now, I will use epoll for the event handling, and in the not-so-distant future, I might well switch to signal-per-fd approach.
#include "cache.h"
#include "config.h"
#include "parser.h"
#include "headerhash.h"

char response_header[100];
char *response_headers = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\nConnction: close\r\n\r\n";
int dirfd_;

enum CON_MODE { READ, WRITE, IDLE };

struct conn_t
{
    header_t header;
    char* buf;
    enum CON_MODE mode;
    int fd;
};

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

    //        fl = fcntl(s, F_GETFL);
    //        fcntl(s, F_SETFL, f1 | 0_NONBLOCK);
    //    char* sample = "GET /tutorials/other/top-20-mysql-best-practices/ HTTP/1.1\r\n"
    //"Host: net.tutsplus.com\r\n"
    //"User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)\r\n"
    //"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
    //"Accept-Language: en-us,en;q=0.5\r\n"
    //"Accept-Encoding: gzip,deflate\r\n"
    //"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
    //"Keep-Alive: 300\r\n"
    //"Connection: keep-alive\r\n"
    //"Cookie: PHPSESSID=r2t5uvjq435r4q7ib3vtdjq120\r\n"
    //"Pragma: no-cache\r\n"
    //"Cache-Control: no-cache;\r\n\r\n";
    //    int tt = ParseHeader(sample, strlen(sample), &test);
    //    printf(".Parsed : %s. ", test.protocol_ver);

struct epoll_event ev, events[20]; 
pthread_t pump;
int epollfd;
int servfd;

void* siginthandler(int dummy __attribute__((unused)))
{
    printf("Received SIGINT\n");
    shutdown(servfd, SHUT_RDWR);
    close(servfd);
    pthread_cancel(pump);
    exit(-1);
}

void accept_pump(void)
{
    struct sockaddr_in client;
    int news;
    int clen = sizeof(struct sockaddr_in);
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT ;
    while((news = accept4(servfd,(struct sockaddr*)&client, &clen, SOCK_NONBLOCK)) != -1)
    {
	ev.data.fd = news; 
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, news, &ev) == -1)
	{
	    fprintf(stderr, "Unable to add socket to interest list : %s", strerror(errno));
	} 

    }
    pthread_exit(NULL);
}


int main()
{
    struct sockaddr_in server = {0};
    char RecvBuf[MAX_BUFLEN] = {0};
    int nfds;
    if((dirfd_ = open(HTML_HOME,  O_PATH)) == -1)
    {
	fprintf(stderr, "Unable to retrieve path : %s\n", strerror(errno));
	exit(-1);
    }
    if((epollfd = epoll_create1(0)) == -1)
    {
	fprintf(stderr, "Unable to create epoll file descriptor : %s", strerror(errno));
	exit(-1);
    }
    if((servfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	printf("Failed. Error Code : %d\n", strerror(errno));
	goto CLEANUP;
    }

    printf("Socket Created. Binding\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //setsockopt(s, SOL_SOCKET, SO_REUSEPORT, 1, 4); 
    //setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1, 4); 

    if(bind(servfd, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
	printf("Binding failed with : %s\n", strerror(errno));
	goto CLEANUP;
    }

    printf("Done binding\n");

    listen(servfd, MAX_SOCK);

    printf("Waiting for incoming connections\n");

    pthread_create(&pump, NULL, (void*)accept_pump, NULL);

    char BUFFPOOL[MAX_BUFLEN * 4];
    struct sigaction sigact;
    bzero(&sigact, sizeof(sigact));
    sigact.sa_handler = siginthandler; 
    sigaction(SIGINT, &sigact, NULL); 


    while(1)
    {
	nfds = epoll_wait(epollfd, events, 20, -1);
	//nfds = epoll_pwait(epollfd, events, 20, -1, &mask);
	if (nfds == -1)
	{
	    fprintf(stderr, "epoll_wait : %s\n", strerror(errno));
	    break;
	}	    
	for(int i = 0; i < nfds; i++)
	{
	    printf("Got : %d, event : %d\n", events[i].data.fd, events[i].events);
	    if(events[i].events & ( EPOLLRDHUP | EPOLLRDHUP | EPOLLERR))
	    {
		shutdown(events[i].data.fd, SHUT_RDWR);
		close(events[i].data.fd);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
		printf("Connection closing for %d\n" , events[i].data.fd);
		continue;
	    }
	    int iResult;
	    int length;
	    ioctl(events[i].data.fd, SIOCINQ, &length);
	    printf("length : %d \n", length);
READ:
	    iResult= recv(events[i].data.fd, RecvBuf, MAX_BUFLEN, 0);
	    if(!(iResult < MAX_BUFLEN || errno == EAGAIN)) goto READ;
	    send(events[i].data.fd, response_headers, strlen(response_headers), MSG_MORE);
	    send(events[i].data.fd, "<html>NOOB</html>\0", strlen("<html>NOOB</html>\0"), MSG_ZEROCOPY);
	    events[i].events |= EPOLLONESHOT;
	    if(epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) == -1)
	    {
		fprintf(stderr, "Unable to del socket from interest list : %s", strerror(errno));
	    } 
	    shutdown(events[i].data.fd, SHUT_RDWR);
	    close(events[i].data.fd);
	    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
	    printf("Connection closing for %d\n" , events[i].data.fd);
	}
    }

CLEANUP:
    close(servfd); 
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
