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
#include <vector>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1000
#define MESSAGE             "This is the message I'm sending back and forth"
#define MAX_MSG_SZ      1024
#define QUEUE_SIZE          5

using namespace std;



//------------------------------------------------------------------------------------



// Determine if the character is whitespace
bool isWhitespace(char c)
{
    switch (c)
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

        if (amtread > 0)
        {
            messagesize += amtread;
        }
        else if( amtread == 0 )
        {
            break;
        }
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
        {
            break;
        }
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
char *FormatHeader(char *str, char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 2;
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
                line = FormatHeader(tline, const_cast<char*>( "" ) );
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, const_cast<char*>( "HTTP_" ) );
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);                
            }
        }
        fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}



//-------------------------------------------------------------------------------------


// bool isWhitespace(char c)
// { switch (c)
//     {
//         case '\r':
//         case '\n':
//         case ' ':
//         case '\0':
//             return true;
//         default:
//             return false;
//     }
// }

// // Strip off whitespace characters from the end of the line
// void chomp(char *line)
// {
//     int len = strlen(line);
//     while (isWhitespace(line[len]))
//     {
//         line[len--] = '\0';
//     }
// }

// // Read the line one character at a time, looking for the CR
// // You dont want to read too far, or you will mess up the content
// char * GetLine(int fds)
// {
//     char tline[MAX_MSG_SZ];
//     char *line;
    
//     int messagesize = 0;
//     int amtread = 0;
//     while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
//     {
//         if (amtread >= 0)
//             messagesize += amtread;
//         else
//         {
//             perror("Socket Error is:");
//             fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
//             exit(2);
//         }
//         //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
//         if (tline[messagesize - 1] == '\n')
//             break;
//     }
//     tline[messagesize] = '\0';
//     chomp(tline);
//     line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
//     strcpy(line, tline);
//     //fprintf(stderr, "GetLine: [%s]\n", line);
//     return line;
// }

// // Change to upper case and replace with underlines for CGI scripts
// void UpcaseAndReplaceDashWithUnderline(char *str)
// {
//     int i;
//     char *s;
    
//     s = str;
//     for (i = 0; s[i] != ':'; i++)
//     {
//         if (s[i] >= 'a' && s[i] <= 'z')
//             s[i] = 'A' + (s[i] - 'a');
        
//         if (s[i] == '-')
//             s[i] = '_';
//     }
    
// }


// // When calling CGI scripts, you will have to convert header strings
// // before inserting them into the environment.  This routine does most
// // of the conversion
// char *FormatHeader(char *str, const char *prefix)
// {
//     char *result = (char *)malloc(strlen(str) + strlen(prefix));
//     char* value = strchr(str,':') + 1;
//     UpcaseAndReplaceDashWithUnderline(str);
//     *(strchr(str,':')) = '\0';
//     sprintf(result, "%s%s=%s", prefix, str, value);
//     return result;
// }

// // Get the header lines from a socket
// //   envformat = true when getting a request from a web client
// //   envformat = false when getting lines from a CGI program

// void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat)
// {
//     // Read the headers, look for specific ones that may change our responseCode
//     char *line;
//     char *tline;
    
//     tline = GetLine(skt);
//     while(strlen(tline) != 0)
//     {
//         if (strstr(tline, "Content-Length") ||
//             strstr(tline, "Content-Type"))
//         {
//             if (envformat)
//                 line = FormatHeader(tline, "");
//             else
//                 line = strdup(tline);
//         }
//         else
//         {
//             if (envformat)
//                 line = FormatHeader(tline, "HTTP_");
//             else
//             {
//                 line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
//                 sprintf(line, "HTTP_%s", tline);
//             }
//         }
//         //fprintf(stderr, "Header --> [%s]\n", line);
        
//         headerLines.push_back(line);
//         free(tline);
//         tline = GetLine(skt);
//     }
//     free(tline);
// }

string getFileName(char *message)
{
    string name = "";
    bool finding = false;
    for(int i = 0; message[i] != '\0'; i++) {
        if (finding && isspace(message[i])) {
            return name;
        }
        if (finding) {
            name +=message[i];
        }
        if (isspace(message[i])) {
            finding = !finding;
        }
    }
    return "";
}

string contentType(string extension)
{
    if (extension == "html") {return "Content-Type: text/html";}
    else if (extension == "txt") {return "Content-Type: text/plain";}
    else if (extension == "jpg") {return "Content-Type: image/jpg";}
    else if (extension == "gif") {return "Content-Type: image/gif";}
    else {return "Content-Type: text/plain";}
}

bool indexFile(string str)
{
    printf("testing directory for: %s\n", str.c_str());
    struct stat fileStat;
    if (stat(str.c_str(),&fileStat)) {
        return false;
    }
    if (S_ISREG(fileStat.st_mode)) {
        return true;
    }
    if (S_ISDIR(fileStat.st_mode)) {
        return false;
    }
}

//return the error, or html
void serve(int connectionSocket, char buffer[], string prefix, string ending)
{
    struct stat filestat;
    string resource = prefix+ending;
    printf("resource: %s\n", resource.c_str());

    
    if(stat(resource.c_str(), &filestat)) {
        printf("ERROR in stat\n\n");
        memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"HTTP/1.1 404 Not Found\r\n\r\n<html><h1>NOT FOUND</h1></html>\n");
            write(connectionSocket,buffer,strlen(buffer));
            return;
        //return 404 not found headers
    }
    if(S_ISREG(filestat.st_mode)) {
        cout << resource << " is a regular file \n";
        cout << "file size = "<<filestat.st_size <<"\n\n";
        string content = contentType(resource.substr(resource.find_last_of(".")+1));
        if (content == "Content-Type: image/jpg" || content == "Content-Type: image/gif") {
            printf("-------image/jpg/gif---------");

            memset(buffer,0,sizeof(buffer));
            sprintf(buffer, "HTTP/1.1 200 OK\r\n\
                %s\r\n\
                Content-Length: %d\
                \r\n\r\n",content.c_str(),(int)filestat.st_size);
            write(connectionSocket,buffer,strlen(buffer));

            FILE *fp = fopen(resource.c_str(),"r");
            char *buff = (char *)malloc(filestat.st_size+1);
            fread(buff,filestat.st_size,1,fp);
            write(connectionSocket,buff,filestat.st_size);
            free(buff);
            fclose(fp);
            return;
        } 
        FILE *fp = fopen(resource.c_str(),"r");
        char *buff = (char *)malloc(filestat.st_size);
        fread(buff,filestat.st_size,1,fp);
        printf("Content-Type is: %s\n", content.c_str());
        //printf("Got\n%s\n", buff);
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,
                "HTTP/1.1 200 OK\r\n\
                %s\r\n\
                Content-Length: %d \
                \r\n\r\n\
                %s\n",content.c_str(), (int)sizeof(buff),buff);
    
        write(connectionSocket,buffer,strlen(buffer));
        fclose(fp);
        //format headers, read file, send it to client
    }
    if(S_ISDIR(filestat.st_mode)) {
        cout << resource << " is a directory \n\n";
        DIR *dirp;
        struct dirent *dp;
        string result = "<html>\n<h1>Directory for " + resource + "</h1>\n<ul>\n";
        
        dirp = opendir(resource.c_str());
        while ((dp = readdir(dirp)) != NULL) {
            result += "<li><a href='"; result+= ending + "/";
            result.append(dp->d_name);
            result+="''>";
            result.append(dp->d_name);
            result += "</a></li>\n";
            //printf("name %s\n", dp->d_name);
        }
        result += "</ul>\n</html>";
        (void)closedir(dirp);
        if (indexFile(resource+"/index.html")) {
            serve(connectionSocket,buffer,resource,"/index.html");
            return;
        }
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,
                "HTTP/1.1 200 OK\r\n\
                Content-Type: text/html\r\n\
                Content-Length: %d\r\n\
                \r\n\r\n\
                %s\n",(int)sizeof(result.c_str()),result.c_str());
    
        write(connectionSocket,buffer,strlen(buffer));

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


int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;
    string prefix;

    if(argc < 3)
      {
        printf("\nUsage: server host-port\n");
        return 0;
      }
    else
      {
          nHostPort=atoi(argv[1]);
          prefix = argv[2];
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
        
        
        char *startline = GetLine(hSocket);
        printf("Status line %s\n",startline);
        string ending = getFileName(startline);
        printf("testing filename: %s\n\n", ending.c_str());
        if(ending != "/favicon.ico") {
            serve(hSocket, pBuffer, prefix, ending);
        
            vector<char *> headerLines;
            GetHeaderLines(headerLines,hSocket,false);
            // for (int i = 0; i < headerLines.size(); i++) {
            //     printf("[%d] %s\n",i,headerLines[i]);
            // }
            // printf("\n=======================\n");
            // printf("Headers are finished\n");
            // printf("=======================\n\n");
        
        
            //strcpy(pBuffer,MESSAGE);
            //printf("\nSending \"%s\" to client\n\n",pBuffer);
            // memset(pBuffer,0,sizeof(pBuffer));
            // read(hSocket,pBuffer,BUFFER_SIZE);
            // printf("Got from browser \n%s\n\n",pBuffer);
        
            // memset(pBuffer,0,sizeof(pBuffer));
            // sprintf(pBuffer,
            //         "HTTP/1.1 200 OK\r\n\
            //         Content-Type: text/html\
            //         \r\n\r\n\
            //         <html>hello world</html>\n");
        
            // write(hSocket,pBuffer,strlen(pBuffer));
        
            linger lin;
            unsigned int y = sizeof(lin);
            lin.l_onoff =1;
            lin.l_linger=10;
            setsockopt(hSocket,SOL_SOCKET,SO_LINGER,&lin,sizeof(lin));
            shutdown(hSocket,SHUT_RDWR);
        }
        
//        if(strcmp(pBuffer,MESSAGE) == 0)
//            printf("\nThe messages match");
//        else
//            printf("\nSomething was changed in the message");
//
        printf("\nClosing the socket\n");
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
            printf("\nCould not close socket\n");
            return 0;
        }
    }
    return 0;
}
