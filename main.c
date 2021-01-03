#include "cache.h"
#include "config.h"
#include "parser.h"
#include "headerhash.h"

/*
   void* Thread()
   {
   while(1)
   {
   switch(getch())
   {
   case 27:
   _exit(0);
//            ExitThread(0);
break;
default:
break;
}
}
}*/
//
//void poll_wait(int fd, int events)
//{
//        int
//}

char response_header[100];
char *response_headers = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";
int dirfd_;

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

int main()
{
    pthread_t t_ [MAX_THREADS];
    int thread_ind = 0;
    struct sockaddr_in server, client;
    int c;
    int s, news;
    int fl;
    dirfd_ = open(HTML_HOME,  O_PATH);
    if(dirfd_ == -1)
    {
	fprintf(stderr, "Unable to retrieve path : %s\n", strerror(errno));
	exit(-1);
    }
    bzero((char*)&server, sizeof(server));
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
    {
	printf("Failed. Error Code : %d\n", strerror(errno));
	goto CLEANUP;
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

    printf("Socket Created. Binding\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    struct http_header header = {0};
    char RecvBuf[MAX_BUFLEN] = {0};

    int iResult, iSendResult;
    int clen = sizeof(struct sockaddr_in);

    if(bind(s, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
	printf("Binding failed with : %s\n", strerror(errno));
	goto CLEANUP;
    }

    printf("Done binding\n");

    listen(s, 5);

    printf("Waiting for incoming connections\n");

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

    printf("Unable to accept connection : %d\n", strerror(errno));

CLEANUP:
    close(s);
    close(news);
    return 0;
}
