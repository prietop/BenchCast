#include <pthread.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct spec_barrier {
   pthread_barrier_t barrier;
   bool doWait;;
} spec_barrier_t;

static spec_barrier_t* my_barrier;

int initialize_barrier();

int initialize_barrier_();

void call_barrier();

void call_barrier_();