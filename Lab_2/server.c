#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "utils.cpp"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1000
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5


void serve(int conn_sock)
{
    struct stat filestat;
    //std::string rs = path + requested resource
    //use rs inplace of argv[1]
    
    if(stat(argv[1], &filestat)) {
        printf("ERROR in stat\n");
        //return 404 not found headers
    }
    if(S_ISREG(filestat.st_mode)) {
        cout << argv[1] << " is a regular file \n";
        cout << "file size = "<<filestat.st_size <<"\n";
        FILE *fp = fopen(argv[1],"r");
        char *buff = (char *)malloc(filestat.st_size);
        fread(buff,filestat.st_size,1,fp);
        printf("Got\n%s\n", buff);
        //format headers, read file, send it to client
    }
    if(S_ISDIR(filestat.st_mode)) {
        cout << argv[1] << " is a directory \n";
        DIR *dirp;
        struct dirent *dp;
        
        dirp = opendir(argv[1]);
        while ((dp = readdir(dirp)) != NULL)
            printf("name %s\n", dp->d_name);
        (void)closedir(dirp);
        //look for index.html(run stat function again)
        //if(stat(rs+"/index.html", &filestat))
        //{
            //index doesnt exist
            //read dir listing
            //generate html
            //send appropriate headers and body to client
        //}
        //else
        //{
                //formate headers, read index.hmtl, send all to client
        //}
    }
}
int get_file_size(std::string path)
{
    struct stat filestat;
    if(stat(path.c_str(), &filestat))
    {
        return -1;
    }
    return filestat.st_size;
}
std::string get_file_contents(const char* filename)
{
    
}

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;

    if(argc < 2)
      {
        printf("\nUsage: server host-port\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
      }

    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d",nHostPort);

    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        //need to parse to get headers, and then get the url.. or file/dir asked for
        //chrome asks 2 request. one with favicon.ico, if this, return 200 ok
        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
        strcpy(pBuffer,MESSAGE);
        printf("\nSending \"%s\" to client",pBuffer);
        memset(pBuffer,0,sizeof(pBuffer));
        read(hSocket,pBuffer,BUFFER_SIZE);
        printf("Got from browser \n%s\n",pBuffer);
        
        memset(pBuffer,0,sizeof(pBuffer));
        sprintf(pBuffer,
                "HTTP/1.1 200 OK\r\n\
                Content-Type: text/html\
                \r\n\r\n\
                <html>hello world</html>\n");
        
        write(hSocket,pBuffer,strlen(pBuffer));
        
        linger lin;
        unsigned int y = sizeof(lin);
        lin.l_onoff =1;
        lin.l_linger=10;
        setsockopt(hSocket,SOL_SOCKET,SO_LINGER,&lin,sizeof(lin));
        shutdown(hSocket,SHUT_RDWR);
        
        
//        if(strcmp(pBuffer,MESSAGE) == 0)
//            printf("\nThe messages match");
//        else
//            printf("\nSomething was changed in the message");
//
        printf("\nClosing the socket");
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
            printf("\nCould not close socket\n");
            return 0;
        }
    }
    return 0;
}
