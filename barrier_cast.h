#include <pthread.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>

typedef struct spec_barrier {
   pthread_barrier_t barrier;
   bool doWait;;
} spec_barrier_t;

static spec_barrier_t* my_barrier;

int initialize_barrier()
{
    char    shm_name[] = "tmp_pthread_barrierattr_getpshared";
    int     shm_fd;
    shm_fd = shm_open(shm_name, O_RDWR, 0644);
    if(shm_fd == -1)
    {
      perror("[E] Error at shm_open()");
      return -2;
    }
    my_barrier = (spec_barrier_t*)mmap(NULL, sizeof(spec_barrier_t), PROT_READ|PROT_WRITE,
      MAP_SHARED, shm_fd, (off_t)0);
    if(my_barrier == MAP_FAILED)
    {
      perror("[E] child: Error at first mmap()");
      return -1;
    }
    return 0;
}

void call_barrier()
{
  if(my_barrier->doWait)
    pthread_barrier_wait(&my_barrier->barrier);
}
