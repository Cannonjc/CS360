#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAX_MSG_SZ      1024
#define MAXGET 1000
#define NCONNECTIONS    20

// Determine if the character is whitespace
bool isWhitespace(char c)
{ switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread >= 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;
    
    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');
        
        if (s[i] == '-')
            s[i] = '_';
    }
    
}


// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, const char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;
    
    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") ||
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, "");
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, "HTTP_");
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}

bool determinePort(char* port)
{
    for (int i = 0; i < strlen(port); i++)
    {
        if (!isdigit(port[i]))
        {
            return false;
        }
    }
    return true;
}

int  main(int argc, char* argv[])
{
    int hSocket[NCONNECTIONS];                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount = 0;
    char strHostName[HOST_NAME_SIZE];
    int portno;

    extern char *optarg;
    int c, times_to_download = 1, err = 0;
    bool debug = false;
    bool countFlag = false;
    while((c = getopt(argc,argv,"c:d")) !=-1)
    {
        switch (c) {
            case 'c':
                times_to_download = atoi(optarg);
                countFlag = true;
                break;
            case 'd':
                debug = true;
                break;
            case '?':
                err = 1;
                break;
            default:
                break;
        }
    }
    strcpy(strHostName,argv[optind]);
    std::string page = argv[optind +2];
    std::string host = argv[optind];
    char* port = argv[optind+1];
    bool temp = determinePort(port);
    if (temp) {
        portno = atoi(argv[optind+1]);
    } else {
        printf("\nbad port number\n");
        return 0;
    }
    if (countFlag) {
        printf("\nNeeds to download: %d times\n\n",times_to_download);
    }

    //printf("\nMaking a socket\n");
    printf("\nConnecting to %s (%lX) on port %d\n\n",strHostName,nHostAddress,portno);

    int epollfd = epoll_create(1);

    /* make a socket */
    for (int i = 0; i < NCONNECTIONS; i++)
    {
        hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(hSocket[i] == SOCKET_ERROR)
        {
            printf("\nCould not make a socket\n");
            return 0;
        }

        /* get IP address from name */
        pHostInfo=gethostbyname(strHostName);
        if (pHostInfo == NULL) {
            printf("\nHost info incorrect\n\n");
            return 0;
        }
    
        /* copy address into long */
        memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

        /* fill address struct */
        Address.sin_addr.s_addr=nHostAddress;
        Address.sin_port=htons(portno);
        Address.sin_family=AF_INET;

    
        /* connect to host */
        if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address))
           == SOCKET_ERROR)
        {
            printf("\nCould not connect to host\n");
            return 0;
        }

        /* read from socket into buffer
         ** number returned by read() and write() is the number of bytes
         ** read or written, with -1 being that an error occured */


         struct epoll_event event;
         event.data.fd = hSocket[i];
         event.events = EPOLLIN;
         int ret = epoll_ctl(epollfd, EPOLL_ETC_ADD, hSocket[i],&event);

    
        char *message = (char *) malloc(MAXGET);
        sprintf(message, "GET %s HTTP/1.1\r\nHost:%s:%d\r\n\r\n",(char *)page.c_str(),(char *)host.c_str(),portno);
        if (debug) {
            printf("\n\nRequest:\n %s\n", message);
        }
        write(hSocket[i],message,strlen(message));
        free(message);
    }


    for (int i = 0; i < NCONNECTIONS; i++) {
        //create an epoll interface
        //when a socket has data ready, read it
        struct epoll_event event;
        int nr_events = epoll_wait(epollfd, &event, 1,-1);
        char myBuffer[10000];


        std::vector<char *> headerLines;
        //char contentType[MAX_MSG_SZ];
        char contentLength[MAX_MSG_SZ];
        
        // First read the status line
        char *startline = GetLine(hSocket[i]);
        if (debug) {
            printf("Status line %s\n\n",startline);
        }
        
        // Read the header lines
        GetHeaderLines(headerLines, hSocket[i], false);
        
        for (int i = 0; i < headerLines.size(); i++) {
            if (strstr(headerLines[i], "Content-Length")) {
                sscanf(headerLines[i], "Content-Length: %s", contentLength);
            }
        }
        // Now print them out
        if (debug)
        {
            for (int i = 0; i < headerLines.size(); i++) {
                printf("[%d] %s\n",i,headerLines[i]);
                //            if(strstr(headerLines[i], "Content-Type")) {
                //                sscanf(headerLines[i], "Content-Type: %s", contentType);
                //            }
            }
            printf("\n=======================\n");
            printf("Headers are finished\n");
            printf("=======================\n\n");
        }
        



        //reading the data from the socket
        int rval = read(event.data.fd,myBuffer,10000);
        




        if(close(hSocket[i]) == SOCKET_ERROR)
        {
            printf("\nCould not close socket\n");
            return 0;
        }
    }
                      
    
    
    return 0;
}
