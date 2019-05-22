/*
 *  Andrew Yang
 *  V00878595
 *  CSC360 Assignment 2 - UThread_Spinlock
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

int items = 0;

spinlock_t pLock;
spinlock_t cLock;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    spinlock_lock( &pLock );
    while(items == MAX_ITEMS){
      producer_wait_count++;
    }

    assert(items < MAX_ITEMS);

    items++;
    histogram[items]++;
    printf("Produced item, item count: %d\n", items);
    spinlock_unlock( &pLock );
  }
    return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    spinlock_lock( &cLock );
    while(items == MIN_ITEMS){
      consumer_wait_count++;
    }

    assert(items > MIN_ITEMS);
    items--;
    histogram[items]++;
    printf("Consumed item, item count: %d\n", items);
    spinlock_unlock( &cLock );
  }
  return NULL;
}

int main (int argc, char** argv) {
  spinlock_create( &cLock );
  spinlock_create( &pLock );

  uthread_t t[4];

  uthread_init (4);

  int index = 0;
  for(int i = 0; i< NUM_PRODUCERS; i++){
    uthread_t tmp = uthread_create( &producer, NULL );
    t[index] = tmp;
    index++;
  }

  for(int j = 0; j< NUM_CONSUMERS; j++){
    uthread_t tmp = uthread_create( &consumer, NULL );
    t[index] = tmp;
    index++;
  }

  for(int k = 0; k< NUM_PRODUCERS + NUM_CONSUMERS; k++){
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
