#include <pthread.h>
#include <stdio.h>
#include <queue>
#include <semeaphore.h>
#define NUM_THREADS     5

queue<int> clients;
sem_t waiting_connections;
sem_t space_on_q;
sem_t lock_on_q;

using namespace std;

void *startThreads(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, thread #%ld!\n", tid);

   for (;;) {
      sem_wait(&waiting_connections);
      sem_wait(&lock_on_q);

      sem_post(&lock_on_q);
      sem_post(&space_on_q);

      //perform normal request serving stuff
   }
   
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   int rc;
   long t;

   int queue_size = 20;
   sem_init(&waiting_connections, 0, 0);
   sem_init(&space_on_q, 0, queue_size);
   sem_init(&lock_on_q, 0, 1);
   
   for(t=0; t<NUM_THREADS; t++){
      printf("In main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, startThreads, (void *)t);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
   }

   //create socket
   //bind
   //listen

   for (;;) {
      // accept the socket
      sem_wait(&space_on_q);
      sem_wait(&lock_on_q);

      //add to queue

      sem_post(&lock_on_q);
      sem_post(&waiting_connections);
   }

   /* Last thing that main() should do */
   pthread_exit(NULL);
}






















