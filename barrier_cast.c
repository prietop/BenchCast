#include "barrier_cast.h"

int initialize_barrier()
{
    char    shm_name[] = "tmp_pthread_barrierattr_getpshared";
    int     shm_fd;
    shm_fd = shm_open(shm_name, O_RDWR, 0644);
    if(shm_fd == -1)
    {
      fprintf(stderr,"[E] Error at shm_open()");
      exit(-2);
    }
    my_barrier = (spec_barrier_t*)mmap(NULL, sizeof(spec_barrier_t), PROT_READ|PROT_WRITE,
      MAP_SHARED, shm_fd, (off_t)0);
    if(my_barrier == MAP_FAILED)
    {
      fprintf(stderr,"[E] child: Error at first mmap()");
      exit(-1);
    }
    return 0;
}

int initialize_barrier_()
{
    char    shm_name[] = "tmp_pthread_barrierattr_getpshared";
    int     shm_fd;
    shm_fd = shm_open(shm_name, O_RDWR, 0644);
    if(shm_fd == -1)
    {
      fprintf(stderr,"[E] Error at shm_open()");
      exit(-2);
    }
    my_barrier = (spec_barrier_t*)mmap(NULL, sizeof(spec_barrier_t), PROT_READ|PROT_WRITE,
      MAP_SHARED, shm_fd, (off_t)0);
    if(my_barrier == MAP_FAILED)
    {
      fprintf(stderr,"[E] child: Error at first mmap()");
      exit(-1);
    }
    return 0;
}

void call_barrier()
{
  if(my_barrier->doWait)
  {  pthread_barrier_wait(&my_barrier->barrier);}
}

void call_barrier_()
{
  if(my_barrier->doWait)
  {  pthread_barrier_wait(&my_barrier->barrier);}
}