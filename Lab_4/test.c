#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <ctype.h>
#include <map>
#include <math.h>


#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define HOST_NAME_SIZE      255


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
double getAverage(std::map<int,double> myMap)
{
	double average = 0;
	int counter = 0;
	std::map<int,double>::iterator it = myMap.begin();
	for (it = myMap.begin(); it != myMap.end(); ++it) {
		counter++;
		average += it->second;
	}
	return average/counter;
}
double getStandardDeviation(std::map<int,double> myMap, double averageTime)
{
	std::vector<double> squaredResults;
	std::map<int,double>::iterator it = myMap.begin();
	for (it = myMap.begin(); it != myMap.end(); ++it) {
		double temp = it->second - averageTime;
		temp *= temp;
		squaredResults.push_back(temp);
	}
	double averageSquared;
	for (int i = 0; i < squaredResults.size(); i++) {
		averageSquared += squaredResults[i];
	}
	double average = averageSquared/squaredResults.size();
	return sqrt(average);
}

int  main(int argc, char* argv[])
{
	int NSOCKETS = 1;
	bool debug = false;
	int c, err = 0;

	while((c = getopt(argc,argv,"d")) != -1) {
		switch (c) {
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

	
	char* socketsNum = argv[optind+3];
	if (determinePort(socketsNum)) {
    	NSOCKETS = atoi(argv[optind+3]);
    } else {
    	printf("\nBad Sockets Number\n");
    }


	struct timeval oldtime[NSOCKETS+10];
    int hSocket[NSOCKETS];                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;

    strcpy(strHostName,argv[optind]);
    char* port = argv[optind+1];
    if (determinePort(port)) {
    	nHostPort = atoi(argv[optind+1]);
    } else {
    	printf("\nBad Port Number\n");
    }
    std::string page = argv[optind+2];

    printf("\n\n---Input---\nHostname: %s\nPort: %d\nPath: %s\nSockets: %d",strHostName,nHostPort,page.c_str(),NSOCKETS);


    printf("\nMaking a socket\n");
    /* make a socket */
	for(int i = 0; i < NSOCKETS; i++) {
	    hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(hSocket[i] == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			return 0;
		}
	}

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

	int epollFD = epoll_create(1);
	// Send the requests and set up the epoll data
	for(int i = 0; i < NSOCKETS; i++) {
		/* connect to host */
		if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address)) 
		   == SOCKET_ERROR)
		{
			printf("\nCould not connect to host\n");
			return 0;
		}
		std::string tempRequest = "GET " + page + " HTTP/1.0\r\n\r\n";
		char request[tempRequest.size()];
		strcpy(request,tempRequest.c_str());

	    write(hSocket[i],request,strlen(request));
		struct epoll_event event;
		event.data.fd = hSocket[i];
		event.events = EPOLLIN;
		int ret = epoll_ctl(epollFD,EPOLL_CTL_ADD,hSocket[i],&event);
		if(ret) {perror ("epoll_ctl");}
		gettimeofday(&oldtime[event.data.fd],NULL);
			
	}

	std::map<int,double> socketTime;

	for(int i = 0; i < NSOCKETS; i++) {
		struct epoll_event event;
		int rval = epoll_wait(epollFD,&event,1,-1);
		if(rval < 0)
			perror("epoll_wait");
		read(event.data.fd,pBuffer,BUFFER_SIZE);
		//printf("got from %d\n",event.data.fd);

		struct timeval newtime;
		gettimeofday(&newtime, NULL);
		double usec = (newtime.tv_sec - 
			oldtime[event.data.fd].tv_sec)*(double)1000000+(newtime.tv_usec-oldtime[event.data.fd].tv_usec);
		socketTime.insert(std::pair<int,double>((int)event.data.fd,usec/1000000));
		//std::cout << "Time: " << usec/1000000 << std::endl;

		//printf("\nClosing socket\n");
		/* close socket */                       
		if(close(hSocket[i]) == SOCKET_ERROR)
		{
			printf("\nCould not close socket\n");
			return 0;
		}
	}
	double averageTime = getAverage(socketTime);
	double standardDeviation = getStandardDeviation(socketTime,averageTime);
	if (debug) {
		std::map<int,double>::iterator it = socketTime.begin();
		for (it=socketTime.begin(); it!=socketTime.end(); ++it) {
			printf("\nSocket %d returned in %f seconds",it->first,it->second);
		}
	}
	printf("\n\nAverage time per response: %f\n", averageTime);
	printf("Standard deviation: %f\n\n",standardDeviation);
}
























