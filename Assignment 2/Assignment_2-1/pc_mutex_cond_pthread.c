/*
 *  Andrew Yang
 *  V00878595
 *  CSC360 Assignment 2 - PThread_Mutex_Cond
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#define CONSUMER_HALT 0
#define MAX_ITEMS  10
#define MIN_ITEMS  0 

const int NUM_PRODUCERS = 2;
const int NUM_CONSUMERS = 2;
const int MAX_ITERATIONS = 200;

int producer_wait_count = 0;
int consumer_wait_count = 0;
int histogram[MAX_ITEMS + 1];

struct Resource{
	pthread_mutex_t mutex;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
	int items;
};

struct Resource* initResource(){
	struct Resource* resource = malloc(sizeof (struct Resource));
	pthread_mutex_init( &resource->mutex, NULL );
	pthread_cond_init( &resource->notEmpty, NULL );
	pthread_cond_init( &resource->notFull, NULL );
	resource->items = 0;
	return resource;
}

void *consumer();
void *producer();

int main()
{
	int rc1, rc2, rc3 , rc4;
	pthread_t thread1, thread2, thread3, thread4;
	int numThreads = NUM_PRODUCERS + NUM_CONSUMERS;

	struct Resource *resource = initResource();

	if( (rc1=pthread_create( &thread1, NULL, &producer, resource)) ){
		printf("Thread creation failed: %d\n", rc1);
	}

	if( (rc2=pthread_create( &thread2, NULL, &producer, resource)) ){
		printf("Thread creation failed: %d\n", rc2);
	}

	if( (rc3=pthread_create( &thread3, NULL, &consumer, resource)) ){
		printf("Thread creation failed: %d\n", rc3);
	}

	if( (rc4=pthread_create( &thread4, NULL, &consumer, resource)) ){
		printf("Thread creation failed: %d\n", rc4);
	}

	void *res;
	pthread_join( thread1, &res );
	pthread_join( thread2, &res );
	pthread_join( thread3, &res );
	pthread_join( thread4, &res );

	printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  	printf ("items value histogram:\n");
  	int sum=0;
  	for (int i = 0; i <= MAX_ITEMS; i++) {
    	printf ("  items=%d, %d times\n", i, histogram [i]);
    	sum += histogram [i];
  	}
  	//assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);

	exit(0);
}

/*
 * Consumer will consume resources but has to meet the condition. The condition is that
 * there are not less than or equal to 0 items. The function will lock using a condition
 * mutex if items is <= 0, wait for the items to be greater than 0 and then unlock.
 * When this condition is met, then there is another mutex tied to the items and will 
 * lock to allow consumer access to items and no other threads to access it. 
 */
void *consumer(void* v)
{
	struct Resource* resource = v;

	for(int i = 0; i< MAX_ITERATIONS; i++){
		pthread_mutex_lock ( &resource->mutex );
		// while there isn't space in items
		while( resource->items == MIN_ITEMS ){
			// wait until resource is not empty, loop until there is an item to consume
			consumer_wait_count++;
			pthread_cond_wait( &resource->notEmpty, &resource->mutex );
		}

		assert(resource->items > MIN_ITEMS);
		resource->items--;
		printf("Consumed item, item count: %d\n", resource->items);
		histogram[resource->items]++;
		pthread_cond_signal( &resource->notFull);
		pthread_mutex_unlock( &resource->mutex );
	}
	
}

/*
 * Producer will produce resources so long as another thread is not accessing
 * the items variable currently. The function locks the mutex if it is allowed
 * to access the items, increments by 1, and then unlocks the mutex.
 */
void *producer(void* v)
{
	struct Resource* resource = v;

	for(int i = 0; i<MAX_ITERATIONS; i++){
		pthread_mutex_lock( &resource->mutex );
		while( resource->items == MAX_ITEMS ){
			// wait until resource isn't full, loop until there is space to produce another
			producer_wait_count++;
			pthread_cond_wait( &resource->notFull, &resource->mutex );
		}

		assert(resource->items < MAX_ITEMS);
		resource->items++;
		printf("Produced item, item count: %d\n", resource->items);
		histogram[resource->items]++;
		pthread_cond_signal( &resource->notEmpty);
		pthread_mutex_unlock( &resource->mutex );
	}
	
}