/*
 *  Andrew Yang
 *  V00878595
 *  CSC360 Assignment 2 - PThread_Semaphore
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_ITEMS 10
#define MIN_ITEMS 0
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
int consumer_wait_count = 0;
int producer_wait_count = 0;

struct Resource{
  sem_t mutex;
  sem_t notFull;
  sem_t notEmpty;
  int items;
};

struct Resource* initResource(){
  struct Resource* resource = malloc(sizeof (struct Resource));
  sem_init(&resource->mutex, 0, 1);
  sem_init(&resource->notFull, 0, 2);
  sem_init(&resource->notEmpty, 0, 3);
  return resource;
} 

void* producer (void* v) {
  struct Resource* resource = v;

  for (int i=0; i<NUM_ITERATIONS; i++) {
    while( resource->items == MAX_ITEMS ){
      sem_wait( &resource->notFull );
      producer_wait_count++;
    }

    assert( resource->items < MAX_ITEMS );
    sem_wait( &resource->mutex );
    resource->items++;
    histogram[resource->items]++;
    sem_post( &resource->mutex );
    printf("Produced item, item count: %d\n", resource->items);
    sem_post( &resource->notEmpty );
  }
  return NULL;
}

void* consumer (void* v) {
  struct Resource* resource = v;

  for (int i=0; i<NUM_ITERATIONS; i++) {
    while( resource->items == MIN_ITEMS ){
      sem_wait( &resource->notEmpty );
      consumer_wait_count++;
    }

    assert( resource->items > MIN_ITEMS );
    sem_wait( &resource->mutex );
    resource->items--;
    histogram[resource->items]++;
    sem_post( &resource->mutex );
    printf("Consumed item, item count: %d\n", resource->items);
    sem_post( &resource->notFull );
  }
  return NULL;
}

int main (int argc, char** argv) {
  pthread_t t[4];
  struct Resource* resource = initResource();

  for(int i = 0; i< NUM_PRODUCERS; i++){
    pthread_create( &t[i], NULL, &producer, resource );
  }

  for(int j = 0; j< NUM_CONSUMERS; j++){
    pthread_create( &t[j + NUM_PRODUCERS], NULL, &consumer, resource );
  }

  void *res;
  for(int k = 0; k< NUM_PRODUCERS + NUM_CONSUMERS; k++){
    pthread_join( t[k], &res);
  }

  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  //assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
