/*
Author: Pablo Prieto Torralbo <prietop@unican.es>
*/

#include "spec_cast.h"

#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef GEM5
#include "m5ops.h"
#endif
int main (int argc, char **argv)
{
    max_num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Máx number of processors: %d\n", max_num_processors);

    // GET OPTIONS
    int waiting = 1, gem5_work_op = 0, use_papi=0, i=0, use_csv=0;

    // options struct defined in spec_cast.h
    char app[max_num_processors][MAX_APP_LENGTH];
    int sub_app[max_num_processors];
    char my_app[MAX_APP_LENGTH];
    char config[20] = "gem5";
    int my_sub_app;
    int num_processors=0;
    int num_apps=0;
    int num_loops=1;
    int num_papi_loops=1;
    int num_secs=60;
    int event_mask=0;
    char *csv_filename;

    int *pid;
    static spec_barrier_t* my_barrier;

    struct stat st;
    csv_filename = (char *)malloc(MAX_CWD * sizeof(char));

    get_options(argc, argv, &waiting, &gem5_work_op, &use_papi, app, sub_app, config, 
                &num_processors, &num_apps, &num_loops, &use_csv, &num_secs, &num_papi_loops, csv_filename);

    pid_t child_pid;
    
    if (gem5_work_op > 0)
    {
        // M5 OP intitialization
        // Assume gem5 uses kvm to create checkpoints
#ifdef GEM5        
        map_m5_mem();
#else
        fprintf(stderr, "[W] Using m5 ops without compiling with DGEM5");
        exit(-1);
#endif
    }

    /* Make sure there is process-shared capability. */
    #ifndef PTHREAD_PROCESS_SHARED
    fprintf(stderr,"process-shared attribute is not available for testing\n");
    return -2;
    #endif

    // The barrier will be shared between threads
    pthread_barrierattr_t ba;
    int	pshared = PTHREAD_PROCESS_SHARED;

    // Shared memory portion (through file)
    int 	shm_fd;

    // child process pids
    pid = (int *)malloc(sizeof(int)*(max_num_processors+1));
    // generic return value
    int	    rc;
    // returning status of child processes
    int	status = 0;

    /* Initialize a barrier attributes object */
    if(pthread_barrierattr_init(&ba) != 0)
    {
        printf("[W] Error at pthread_barrierattr_init()\n");
        return -3;
    }

    /* Set the pshard value to private to shared */
    if(pthread_barrierattr_setpshared(&ba, pshared) != 0)
    {
        printf("[W] Error at pthread_barrierattr_setpshared()\n");
        return -3;
    }

    if(pshared != PTHREAD_PROCESS_SHARED)
    {
        printf("[W] Test FAILED: Incorrect pshared value %d\n", pshared);
        return -1;
    }

    /* Create shared object */
    shm_unlink(shm_name);
    shm_fd = shm_open(shm_name, O_RDWR|O_CREAT|O_EXCL, 0644);
    if(shm_fd == -1)
    {
        perror("[E] Error at shm_open()");
        return -3;
    }
    else
    {
        printf("[I] shm_fd_orig: %d\n", shm_fd);
    }

    // define size of shared object
    if(ftruncate(shm_fd, sizeof(spec_barrier_t)) != 0)
    {
        perror("[E] Error at ftruncate()");
        shm_unlink(shm_name);
        return -3;
    }

    /* Map the shared memory object to my memory */
    my_barrier = (spec_barrier_t*)mmap(NULL, sizeof(spec_barrier_t), PROT_READ|PROT_WRITE,
    MAP_SHARED, shm_fd, (off_t)0);

    if(my_barrier == MAP_FAILED)
    {
        perror("[E] Error at first mmap()");
        shm_unlink(shm_name);
        return -3;
    }

    /* Initialize a barrier */
    printf("Num processors = %d\n", num_processors);
    if((pthread_barrier_init(&my_barrier->barrier, &ba, num_processors+waiting)) != 0)
    {
        printf("[W] Error at pthread_barrier_init()\n");
        return -3;
    }
    my_barrier->doWait=true;

    // Initialize pids
    int proc;
    for(proc=0; proc<max_num_processors+1; proc++)
    {
        pid[proc] = -1;
    }

    int init_proc=0;
    if(num_processors < max_num_processors)
    {
        /* There will be at least one processor iddle and we will use it to
        execute the OS */
        printf("Devoting core 0 to OS\n");
        init_proc=1;
    }

    /* Fork a child process */
    proc=0;
    int app_index=0;
    while(proc<num_processors)
    {
        pid[proc] = fork();
        if(pid[proc] == -1)
        {
            perror("[E] Error at fork()");
            return -3;
        }
        else if(pid[proc] == 0)
        {
            //child (SPEC app)
            bind_pid(proc+init_proc, getpid());
            strcpy(my_app, app[app_index]);
            my_sub_app = sub_app[app_index];
            break;
        }
        else
        {
            //parent (spec_cast)
            printf("parent pid : %d, child pid : %d\n", (int)getpid(), pid[proc]);
        }
        proc++;
        app_index++;
        app_index = app_index%num_apps;
    }

    // To use with SIGALARM
    /*
    if(pid[proc]!=0)
    {
        printf("parent %d: send me SIGALRM 10 minutes later in case I am blocked\n", (int)getpid());
        alarm(600);
    }
    else
    {
        printf("Child process %d in processor %d\n", (int)getpid(), proc);
    }
    */
    int EventSet[num_processors];
    rc = -1;
    int numWaits=0;
    if(pid[proc]!=0 && waiting==1)
    {
        while(my_barrier->doWait)
        {
            fprintf(stderr,"Parent Waiting %d\n", numWaits);
            rc = pthread_barrier_wait(&my_barrier->barrier);
            if(numWaits == 0 && use_papi == 1)
            {
                init_papi(num_papi_loops, pid, num_processors, app, num_apps, EventSet, &event_mask);
            }
            numWaits++;
            if(numWaits>=num_loops)
            { my_barrier->doWait=false; }
        }
    }
    else
    {
        fprintf(stderr, "%d executing %s %d\n", getpid(), my_app, my_sub_app);
        char my_cmd[4];
        sprintf(my_cmd, "%d", my_sub_app);
        char *prog[] = { "./launch_spec.py", "--spec", my_app, "--cmd", my_cmd, "--conf", config, NULL };
        fprintf(stderr, "Executing: ");
        int i=0;
        while(prog[i]!=NULL)
        {
            fprintf(stderr, " %s", prog[i]);
            i++;
        }
        fprintf(stderr, "\n");
        rc = execv("./launch_spec.py", prog);
        if(rc<0)
        {
            fprintf(stderr,"Error execv\n");
            exit(rc);
        }
    }

    if(pid[proc] != 0 && gem5_work_op>0)
    {
        //execute m5_work_begin_op
#ifdef GEM5
        m5_work_begin(0);
#else
        fprintf(stderr, "[W] Using m5 ops without compiling with DGEM5");
        exit(-1);
#endif
    }

    if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        fprintf(stderr, "[W] Test FAILED: %d: pthread_barrier_wait() got unexpected "
        "return code : %d\n" , getpid(), rc);
        exit(rc);
    }
    else if(rc == PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("process %d: get PTHREAD_BARRIER_SERIAL_THREAD\n"
        , getpid());
    } else
    {
        printf("process %d: get along with the barrier\n"
        , getpid());
    }

    if(pid[proc] != 0 && use_papi == 1)
    {
        //call PAPI (defined in .h)
        printf("Starting PAPI measures %d each %ds\n", num_papi_loops, num_secs);
        do_papi(num_papi_loops, num_secs, pid, num_processors, app, num_apps,use_csv, EventSet, csv_filename, event_mask);
        printf("PAPI ENDED\n");
        fflush(stdout);
        fflush(stderr);
        for(proc=0; proc<num_processors; proc++)
        {
            kill(pid[proc], SIGKILL);
            child_pid=wait(&status);
            if( child_pid == -1)
            {
                printf("[W] parent: error at waitpid()\n");
                return -3;
            }
            if(!WIFEXITED(status))
            {
                printf("[W] Child %d exited abnormally\n", pid[proc]);
                return -3;
            }

            if(WEXITSTATUS(status))
            {
                printf("[I] %d status = %d\n", child_pid, status);
            }
        }
        return 0;
    } else if(pid[proc] != 0 && waiting==1)
    {
        /* parent */
        for(proc=0;proc<num_processors;proc++)
        {
            child_pid = wait(&status);
            if( child_pid == -1)
            {
                printf("[W] parent: error at waitpid()\n");
                return -3;
            }

            if(!WIFEXITED(status))
            {
                printf("[W] Child exited abnormally\n");
                return -3;
            }

            if(WEXITSTATUS(status))
            {
                printf("[I] %d status = %d\n", child_pid, status);
            }
        }

        printf("[I] All done.\n");

        /* Cleanup */
        if(pthread_barrier_destroy(&my_barrier->barrier) != 0)
        {
            printf("[W] Error at pthread_barrier_destroy()");
            return -3;
        }

        if((shm_unlink(shm_name)) != 0)
        {
            perror("[E] Error at shm_unlink()");
            return -3;
        }

        printf("PASSED\n");
        fflush(stdout);
        fflush(stderr);
        return 0;
    } else
    {
        fprintf(stderr,"Finished %d\n", getpid());
        fflush(stderr);
        fflush(stdout);
        exit(rc);
    }

}
