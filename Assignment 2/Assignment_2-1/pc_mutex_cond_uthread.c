/*
 *  Andrew Yang
 *  V00878595
 *  CSC360 Assignment 2 - UThread_Mutex_Cond
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
#define MIN_ITEMS 0
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

struct Resource{
  uthread_mutex_t mutex;
  uthread_cond_t notEmpty;
  uthread_cond_t notFull;
  int items;
};

struct Resource* initResource(){
  struct Resource* resource = malloc(sizeof (struct Resource));
  resource->mutex = uthread_mutex_create();
  resource->notEmpty = uthread_cond_create(resource->mutex);
  resource->notFull = uthread_cond_create(resource->mutex);
  resource->items = 0;
  return resource;
}

void* producer (void* v) {
  struct Resource* resource = v;

  //uthread_mutex_lock(resource->mutex);

  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_mutex_lock(resource->mutex);
    // while the resource is full, wait for space to open up.
    while(resource->items == MAX_ITEMS){
      producer_wait_count++;
      uthread_cond_wait( resource->notFull );
    }
    // assert that we don't go over the MAX_ITEM limit
    assert(resource->items <= MAX_ITEMS);

    
    resource->items++;
    histogram[resource->items]++;

    uthread_cond_signal(resource->notEmpty);
    uthread_mutex_unlock(resource->mutex);
  }

  //uthread_mutex_unlock(resource->mutex);
  return NULL;
}

void* consumer (void* v) {
  struct Resource* resource = v;

  //uthread_mutex_lock(resource->mutex);

  for (int i=0; i<NUM_ITERATIONS; i++) {
    // while the resource is empty, wait for a resource to be produced.
    uthread_mutex_lock(resource->mutex);
    while(resource->items == MIN_ITEMS){
      consumer_wait_count++;
      uthread_cond_wait(resource->notEmpty);
    }

    //assert before because we need to make sure there is a resource to use.
    assert(resource->items > MIN_ITEMS);
    
    resource->items--;
    uthread_cond_signal( resource-> notFull );
    histogram[resource->items]++;
    uthread_mutex_unlock(resource->mutex);
  }

  //uthread_mutex_unlock(resource->mutex);
  return NULL;
}

int main (int argc, char** argv) {
  int numThreads = NUM_PRODUCERS + NUM_CONSUMERS;
  int index = 0;
  //list of threads
  uthread_t t[numThreads];

  uthread_init (numThreads);

  //initialize resource struct
  struct Resource *resource = initResource();
  
  for(int i = 0; i< NUM_PRODUCERS; i++){
    uthread_t tmp = uthread_create( &producer, resource );
    t[index] = tmp;
    index++;
  }

  for(int j = 0; j< NUM_CONSUMERS; j++){
    uthread_t tmp = uthread_create( &consumer, resource );
    t[index] = tmp;
    index++;
  }

  for(int k = 0; k< numThreads; k++){
    void *res;
    uthread_join( t[k], &res );
  }
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
