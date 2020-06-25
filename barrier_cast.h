#include <pthread.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct bench_barrier {
   pthread_barrier_t barrier;
   bool doWait;;
} bench_barrier_t;

static bench_barrier_t* my_barrier;

int initialize_barrier();

int initialize_barrier_();

void call_barrier();

void call_barrier_();