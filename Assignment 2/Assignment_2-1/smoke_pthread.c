/*
 *  Andrew Yang
 *  V00878595
 *  CSC360 Assignment 2 - Smoker Problem
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  pthread_mutex_init( &agent->mutex, NULL);
  pthread_cond_init( &agent->paper, NULL);
  pthread_cond_init( &agent->match, NULL);
  pthread_cond_init( &agent->tobacco, NULL);
  pthread_cond_init( &agent->smoke, NULL);
  return agent;
}

struct Resource {
  struct Agent* agent;
  int paper;
  int match;
  int tobacco;
};

struct Smoker {
  struct Resource* resource;
  int resourceType;
};

struct Resource* initResource(struct Agent* agent){
  struct Resource* resource = malloc(sizeof (struct Resource));
  resource->agent = agent;
  int paper = 0;
  int match = 0;
  int tobacco = 0;
  return resource;
}

struct Smoker* createSmoker(struct Resource* resource, int resourceType){
  struct Smoker* smoker = malloc (sizeof (struct Smoker));
  smoker->resource = resource;
  smoker->resourceType = resourceType;
  return smoker;
}
//
// TODO
// You will probably need to add some procedures and struct etc.
//

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resources            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random resources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  pthread_mutex_lock ( &a->mutex );
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        pthread_cond_signal ( &a->match );
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        pthread_cond_signal ( &a->paper );
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        pthread_cond_signal ( &a->tobacco );
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      pthread_cond_wait ( &a->smoke, &a->mutex );
    }
  pthread_mutex_unlock ( &a->mutex );
  return NULL;
}

void* smoker(void* av){
  struct Smoker* smoker = av;
  struct Resource* resource = smoker->resource;
  struct Agent* agent = resource->agent;

  pthread_mutex_lock ( &agent->mutex );
  for(;;){
    if(smoker->resourceType == TOBACCO){
      pthread_cond_wait( &agent->tobacco, &agent->mutex );
      resource->tobacco++;
    }else if(smoker->resourceType == PAPER){
      pthread_cond_wait( &agent->paper, &agent->mutex );
      resource->paper++;
    }else if(smoker->resourceType == MATCH){
      pthread_cond_wait( &agent->match, &agent->mutex );
      resource->match++;
    }

    if(smoker->resourceType == TOBACCO){
      if(resource->paper > 0 && resource->match > 0){
        resource->paper--;
        resource->match--;
        signal_count[PAPER + MATCH]++;
        smoke_count[TOBACCO]++;
        pthread_cond_signal( &agent->smoke );
        VERBOSE_PRINT("tobacco smoker smoked");
      }
    }else if(smoker->resourceType == PAPER){
      if(resource->match > 0 && resource->tobacco > 0){
        resource->match--;
        resource->tobacco--;
        signal_count[MATCH + TOBACCO]++;
        smoke_count[PAPER]++;
        pthread_cond_signal( &agent->smoke );
        VERBOSE_PRINT("paper smoker smoked");
      }
    }else if(smoker->resourceType == MATCH){
      if(resource->paper > 0 && resource->match > 0){
        resource->paper--;
        resource->match--;
        signal_count[PAPER + MATCH]++;
        smoke_count[MATCH]++;
        pthread_cond_signal( &agent->smoke );
        VERBOSE_PRINT("match smoker smoked");
      }
    }
  }
  pthread_mutex_unlock ( &agent->mutex );
}

int main (int argc, char** argv) {
  struct Agent*  a = createAgent();
  struct Resource* resource = initResource( a );

  pthread_t tobacco, paper, match, theAgent;

  pthread_create( &tobacco, NULL, &smoker, createSmoker(resource, TOBACCO));
  pthread_create( &paper, NULL, &smoker, createSmoker(resource, PAPER));
  pthread_create( &match, NULL, &smoker, createSmoker(resource, MATCH));
  pthread_create( &theAgent, NULL, &agent, a);

  void *res;
  pthread_join( theAgent, &res );

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}