#include <pthread.h>
#include <sys/mman.h>

static pthread_barrier_t* barrier;

void initialize_barrier()
{
    char    shm_name[] = "tmp_pthread_barrierattr_getpshared";
    int     shm_fd;
    shm_fd = shm_open(shm_name, O_RDWR, 0644);
    if(shm_fd == -1)
    {
      perror("[E] [Bzip2] Error at shm_open()");
      return -2;
    }
    barrier = mmap(NULL, sizeof(pthread_barrier_t), PROT_READ|PROT_WRITE,
      MAP_SHARED, shm_fd, (off_t)0);
    if(barrier == MAP_FAILED)
    {
      perror("[E] child: Error at first mmap()");
      return -1;
    }
}

void call_barrier()
{
    pthread_barrier_wait(barrier);
    printf("Begin ROI\n");
}
