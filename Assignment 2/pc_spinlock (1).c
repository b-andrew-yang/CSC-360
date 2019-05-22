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

spinlock_t lock;
spinlock_create( &lock );

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    if(items <= MAX_ITEMS ){
      spinlock_lock ( &lock );

      if(items > MAX_ITEMS ){
        spinlock_unlock ( &lock );
      }else if{
        items--;
        histogram[items]++;
        spinlock_unlock ( &lock );
      }
    }else{
      producer_wait_count++;
    }  
  }

  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    if(items > MIN_ITEMS){
      spinlock_lock ( &lock );
      
      if(items <= MIN_ITEMS){
        spinlock_unlock ( &lock );
      }else{
        items--;
        histogram[items]++;
        spinlock_unlock ( &lock );
      }
    }else{
      consumer_wait_count++;
    }
  }

  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  
  uthread_create ( producer1, &producer );
  uthread_create ( producer2, &producer );
  uthread_create ( consumer1, &consumer );
  uthread_create ( consumer2, &consumer );

  uthread_join ( producer1 );
  uthread_join ( producer2 );
  uthread_join ( consumer1 );
  uthread_join ( consumer2 );
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
