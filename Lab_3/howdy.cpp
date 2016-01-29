#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


void *howdy(void *arg) {
	long tid;
	tid = (long) arg;
	printf("hi %ld\n",tid);
}

int main()
{
	#define NTHREADS 20
	int threadid;
	pthread_t threads[NTHREADS];
	
	for (threadid = 0; threadid<NTHREADS; threadid++) {
		pthread_create(&threads[threadid],NULL,howdy,(void *)threadid);
	}
	//set up socket, bind, listen, accept
	// for(;;) {
	// 	fd = accept;
	// 	enqueue(fd);
	// }
	pthread_exit(NULL);
	return 0;
}