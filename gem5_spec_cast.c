/*
Author: Pablo Prieto Torralbo <prietop@unican.es>
*/

#include "gem5_spec_cast.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
//#include <sys/processor.h> // solaris
//#include <sys/procset.h> // solaris
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

#include "m5ops.h"

int main (int argc, char **argv)
{
    max_num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Máx number of processors: %d\n", max_num_processors);

    // GET OPTIONS
    int c = 0;
    int waiting = 1, gem5_work_op = 0, chdir_before_exec= 0;

    // options struct defined in gem5_spec_cast.h

    int option_index = 0;

    char app[max_num_processors][15];
    char my_app[15];
    int app_index=0;
    int num_processors=0;
    int num_apps=0;

    while (c >= 0)
    {
        c = getopt_long (argc, argv, "dwbp:n:", long_options, &option_index);

        if(c == -1)
        break;
        switch (c)
        {
            case 0:
            printf ("App: %s\n", long_options[option_index].name);
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
            break;
            strcpy(app[app_index], long_options[option_index].name);
            app_index++;
            break;

            case 'd':
            chdir_before_exec=1;
            printf("Changing directory before execution\n");
            break;

            case 'w':
            printf("Executing GEM5 work_begin op\n");
            gem5_work_op=1;
            break;

            case 'b':
            printf ("Background Execution\n");
            waiting=0;
            break;

            case 'p':
            num_processors=atoi(optarg);
            printf("Number of processors %d\n", num_processors);
            break;

            case 'n':
            num_apps=atoi(optarg);
            printf("Number of applications %d\n", num_apps);
            break;

            case '?':
            /* getopt_long already printed an error message. */
            break;

            default:
            abort ();
        }
    }

    if (print_help)
      usage(argv[0]);

    if(num_processors == 0)
    {
        num_processors = max_num_processors;
        printf("Number of processors not defined, set to %d\n", num_processors);
    } else if(num_processors > max_num_processors)
    {
        fprintf(stderr, "Number of processors set to %d and only %d available\n"
        ,num_processors, max_num_processors);
        return -1;
    }
    if(num_apps == 0)
    {
        num_apps = app_index;
        printf("Number of applications not defined, set to %d\n", num_apps);
    }
    else
    {
        if(num_apps != app_index)
        {
            fprintf(stderr,"Only %d of %d apps have been specified\n",
            app_index, num_apps);
            return -1;
        }
    }

    if (num_apps > num_processors)
    {
        fprintf(stderr, "Number of applications (%d) greater than the number of"
        " processors (%d)\n", num_apps, num_processors);
        return -1;
    }

    pid_t child_pid;

    /* Make sure there is process-shared capability. */
    #ifndef PTHREAD_PROCESS_SHARED
    fprintf(stderr,"process-shared attribute is not available for testing\n");
    return -2;
    #endif

    // This barrier will be shared between threads
    static pthread_barrier_t* barrier;
    pthread_barrierattr_t ba;
    int	pshared = PTHREAD_PROCESS_SHARED;

    // Shared memory portion (through file)
    char 	shm_name[] = "tmp_pthread_barrierattr_getpshared";
    int 	shm_fd;

    // child process pids
    int 	pid[max_num_processors+1];
    // generic return value
    int	    rc;
    // returning status of child processes
    int	status = 0;
    //struct sigaction act; //uncomment to use SIGALARM
    char *cast_wd;
    char *new_wd;
    char *buf;
    size_t allocSize = sizeof(char) * MAX_CWD;
    buf = (char *)malloc(allocSize);

    cast_wd = getcwd(buf, allocSize);
    if (cast_wd == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    /* Set up parent to handle SIGALRM */
    /*act.sa_flags = 0;
    act.sa_handler = sig_handler;
    sigfillset(&act.sa_mask);
    sigaction(SIGALRM, &act, 0);*/

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
    if(ftruncate(shm_fd, sizeof(pthread_barrier_t)) != 0)
    {
        perror("[E] Error at ftruncate()");
        shm_unlink(shm_name);
        return -3;
    }

    /* Map the shared memory object to my memory */
    barrier = mmap(NULL, sizeof(pthread_barrier_t), PROT_READ|PROT_WRITE,
    MAP_SHARED, shm_fd, (off_t)0);

    if(barrier == MAP_FAILED)
    {
        perror("[E] Error at first mmap()");
        shm_unlink(shm_name);
        return -3;
    }

    /* Initialize a barrier */
    printf("Num processors = %d\n", num_processors);
    if((pthread_barrier_init(barrier, &ba, num_processors+waiting)) != 0)
    {
        printf("[W] Error at pthread_barrier_init()\n");
        return -3;
    }

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
    proc=init_proc;
    app_index=0;
    while(proc<(num_processors+init_proc))
    {
        pid[proc] = fork();
        if(pid[proc] == -1)
        {
            perror("[E] Error at fork()");
            return -3;
        }
        else if(pid[proc] == 0)
        {
            bind_pid(proc, getpid());
            strcpy(my_app, app[app_index]);
            break;
        }
        else
        {
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

    rc = -1;
    if(pid[proc]!=0 && waiting==1)
    {
        printf("Parent Waiting\n");
        rc = pthread_barrier_wait(barrier);
    }
    else
    {
        if (chdir(cast_wd) == -1) {
            perror("cding to cast directory");
            exit(EXIT_FAILURE);
        }
        printf("pwd : %s\n", cast_wd);
        if(chdir_before_exec)
        {
            if (chdir(my_app) == -1)
            {
                perror("chdir");
                exit(EXIT_FAILURE);
            }
            printf("cd to %s, done\n", my_app);
            new_wd = getcwd(buf, allocSize);
            printf("now pwd : %s\n", new_wd);
        }
        fprintf(stderr, "%d executing %s\n", getpid(), my_app);
        char *prog[] = { "./launch_script.sh", "", NULL };
        rc = execv("./launch_script.sh", prog);
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

    if(pid[proc] != 0 && gem5_work_op>0)
    {
        //execute m5_work_begin_op
        m5_work_begin(0);
    }

    if(pid[proc] != 0 && waiting==1)
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
        if(pthread_barrier_destroy(barrier) != 0)
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
    }else
    {
        fprintf(stderr,"Finished %d\n", getpid());
        fflush(stderr);
        fflush(stdout);
        exit(rc);
    }

}
