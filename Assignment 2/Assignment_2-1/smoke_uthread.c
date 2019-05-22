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
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
  return agent;
}

struct Resource {
  // Resource is used to increment the amounts when we get notifications that they're available.
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
  VERBOSE_PRINT("agent");
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        uthread_cond_signal (a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        uthread_cond_signal (a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        uthread_cond_signal (a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
    }
  uthread_mutex_unlock (a->mutex);
  return NULL;
}

void* smoker(void* av){
  VERBOSE_PRINT("smoker");
  struct Smoker* smoker = av;
  struct Resource* resource = smoker->resource;
  struct Agent* agent = resource->agent;

  uthread_mutex_lock (agent->mutex);
  for(;;){
    if(smoker->resourceType == TOBACCO){
      uthread_cond_wait( agent->tobacco );
      resource->tobacco++;
    }else if(smoker->resourceType == PAPER){
      uthread_cond_wait( agent->paper );
      resource->paper++;
    }else if(smoker->resourceType == MATCH){
      uthread_cond_wait( agent->match );
      resource->match++;
    }

    if(smoker->resourceType == TOBACCO){
      if(resource->paper > 0 && resource->match > 0){
        resource->paper--;
        resource->match--;
        signal_count[PAPER + MATCH]++;
        smoke_count[TOBACCO]++;
        uthread_cond_signal( agent->smoke );
        VERBOSE_PRINT("tobacco smoker smoked");
      }
    }else if(smoker->resourceType == PAPER){
      if(resource->match > 0 && resource->tobacco > 0){
        resource->match--;
        resource->tobacco--;
        signal_count[MATCH + TOBACCO]++;
        smoke_count[PAPER]++;
        uthread_cond_signal( agent->smoke );
        VERBOSE_PRINT("paper smoker smoked");
      }
    }else if(smoker->resourceType == MATCH){
      if(resource->paper > 0 && resource->match > 0){
        resource->paper--;
        resource->match--;
        signal_count[PAPER + MATCH]++;
        smoke_count[MATCH]++;
        uthread_cond_signal( agent->smoke );
        VERBOSE_PRINT("match smoker smoked");
      }
    }
  }
  uthread_mutex_unlock (a->mutex);
}

int main (int argc, char** argv) {
  uthread_init (7);
  struct Agent*  a = createAgent();
  struct Resource* resource = initResource( a );

  uthread_t tobacco = uthread_create( smoker, createSmoker(resource, TOBACCO));
  uthread_t paper = uthread_create( smoker, createSmoker(resource, PAPER));
  uthread_t match = uthread_create( smoker, createSmoker(resource, MATCH));

  uthread_join (uthread_create (agent, a), 0);
  uthread_join (tobacco, 0);
  uthread_join (paper, 0);
  uthread_join (match, 0);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}