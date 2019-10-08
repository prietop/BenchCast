/*
Author: Pablo Prieto Torralbo <prietop@unican.es>
*/

#include "gem5_spec_cast.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
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
    char app[max_num_processors][MAX_APP_LENGTH];
    int sub_app[max_num_processors];
    char my_app[MAX_APP_LENGTH];
    char config[20] = "gem5";
    int my_sub_app;
    int app_index=0;
    int num_processors=0;
    int num_apps=0;
    int num_loops=1;

    int *pid;
    static spec_barrier_t* my_barrier;
    char 	shm_name[] = "tmp_pthread_barrierattr_getpshared";

    while (c >= 0)
    {
        c = getopt_long(argc, argv, "dwbp:n:l:c:", long_options, &option_index);

        if(c == -1)
            break;
        switch (c)
        {
            case 0:
            //long option with no arguments
            printf ("App: %s\n", long_options[option_index].name);
            /* If this option set a flag, do nothing else now (help). */
            if (long_options[option_index].flag != 0)
            break;
            strcpy(app[app_index], long_options[option_index].name);
            sub_app[app_index]=1;
            app_index++;
            break;

            case 1:
            //long option with required arguments
            printf ("App: %s\n", long_options[option_index].name);
            strcpy(app[app_index], long_options[option_index].name);
            sub_app[app_index]=atoi(optarg);
            if(sub_app[app_index]<=0)
            {
                fprintf(stderr, "\n*** ERROR *** Application \"%s\"", app[app_index]); 
                fprintf(stderr, " requires benchmark number \n\n\n");
                usage(argv[0]);
                break;
            }
            printf("Using benchmark %d of %s\n", sub_app[app_index], app[app_index]);
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

            case 'l':
            num_loops=atoi(optarg);
            printf("Number of loops in ROI: %d\n", num_loops);
            break;

            case 'c':
            strcpy(config,optarg);
            printf("Config name: %s\n", config);
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
    
    if (gem5_work_op > 0)
    {
        // M5 OP intitialization
        // Assume gem5 uses kvm to create checkpoints
        map_m5_mem();
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
            //child (SPEC app)
            bind_pid(proc, getpid());
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

    rc = -1;
    int numWaits=0;
    if(pid[proc]!=0 && waiting==1)
    {
        while(my_barrier->doWait)
        {
            fprintf(stderr,"Parent Waiting %d\n", numWaits);
            rc = pthread_barrier_wait(&my_barrier->barrier);
            numWaits++;
            if(numWaits>=num_loops)
            { my_barrier->doWait=false; }
        }
    }
    else
    {
        if (chdir(cast_wd) == -1) {
            perror("cding to cast directory");
            exit(EXIT_FAILURE);
        }
        printf("pwd : %s\n", cast_wd);
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
    }

    if(pid[proc] != 0 && gem5_work_op>0)
    {
        //execute m5_work_begin_op
        m5_work_begin(0);
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
    }else
    {
        fprintf(stderr,"Finished %d\n", getpid());
        fflush(stderr);
        fflush(stdout);
        exit(rc);
    }

}
