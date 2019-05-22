/*
 *  Andrew Yang
 *  V00878595
 *  CSC360 Assignment 2 - UThread_Semaphore
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"
#include "uthread_mutex_cond.h"

#define MAX_ITEMS 10
#define MIN_ITEMS 0
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

struct Resource{
  uthread_sem_t mutex;
  uthread_sem_t notFull;
  uthread_sem_t notEmpty;
  int items;
};

struct Resource* initResource(){
  struct Resource* resource = malloc(sizeof (struct Resource));
  resource->mutex = uthread_sem_create( 100 );
  resource->notFull = uthread_sem_create( 100 );
  resource->notEmpty = uthread_sem_create( 100 );
  return resource;
};

void* producer (void* v) {
  struct Resource* resource = v;

  for (int i=0; i<NUM_ITERATIONS; i++) {
    while( resource->items == MAX_ITEMS){
      uthread_sem_wait( resource->notFull );
    }

    assert( resource->items < MAX_ITEMS );
    uthread_sem_wait( resource->mutex );
    resource->items++;
    histogram[resource->items]++;
    printf("Produced item, item count: %d\n", resource->items );
    uthread_sem_signal( resource->mutex );
    uthread_sem_signal( resource->notEmpty );
  }
  return NULL;
}

void* consumer (void* v) {
  struct Resource* resource = v;

  for (int i=0; i<NUM_ITERATIONS; i++) {
    while( resource->items == MIN_ITEMS ){
      uthread_sem_wait( resource->notEmpty );
    }

    assert( resource->items > MIN_ITEMS );
    uthread_sem_wait( resource->mutex );
    resource->items--;
    histogram[resource->items]++;
    printf("Consumed item, item count: %d\n", resource->items );
    uthread_sem_signal( resource->mutex );
    uthread_sem_signal( resource->notFull );
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];
  struct Resource* resource = initResource();
  uthread_init (4);

  for(int i = 0; i< NUM_PRODUCERS; i++){
    t[i] = uthread_create( &producer, resource );
  }

  for(int j = 0; j< NUM_CONSUMERS; j++){
    t[j + NUM_PRODUCERS] = uthread_create( &consumer, resource );
  }

  for(int k = 0; k< NUM_CONSUMERS + NUM_PRODUCERS; k++){
    void *res;
    uthread_join( t[k], &res );
  }

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  //assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
