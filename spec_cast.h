/*
Author: Pablo Prieto Torralbo <prietop@unican.es>
*/

#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <papi.h>
#include <sched.h> // linux
#include <string.h>
#include <unistd.h>

#define NUM_EVENTS 6
#define MAX_CWD 600
#define MAX_APP_LENGTH 20
#define MAX_THREADS 128
#define SLEEP_SECONDS 60
#define NUM_EVENT_LOOPS 1
static const char shm_name[] = "tmp_pthread_barrierattr_getpshared";
static const char papi_dir[] = "papi-results";

typedef struct spec_barrier {
   pthread_barrier_t barrier;
   bool doWait;;
} spec_barrier_t;

static int max_num_processors;
static int print_help;
char *filename;
char *csv_filename;

static void usage(char *argv0) {

  fprintf(stderr, "Usage: %s [-b] [-d] [-w] [-p number] [-n number] [-l number] "
        "[-c config_name] [-v csv_filename] [-o outputfile] [-s seconds] "
        "--<prog> [number] [--<prog2> [number] --<prog3> ...]\n", argv0);
  fprintf(stderr, "   prog is the program/s you want to cast. Some programs"
        " require an aditional param to indicate the benchmark number\n");
  fprintf(stderr, "   among all available for that prog (check LAUNCH_CMKS.py"
        " to see the benchmarks available for each prog)\n");
  fprintf(stderr, "   -b makes %s to return immediately after launching the "
        "programs. The default is to wait for them to finish.\n", argv0);
  fprintf(stderr, "   -p is the number of processors you want to use from the "
        "system (default value: the number of available processors in the "
        "system, i.e: %d)\n", max_num_processors);
  fprintf(stderr, "   -w should be used with GEM5, and executes a "
        "m5_work_begin_op\n");
  fprintf(stderr, "   -s use papi library to analyze performance counters during "
         "N seconds\n");
  fprintf(stderr, "   -c print papi results in a csv file (ASUME USING PAPI)\n");
  fprintf(stderr, "   -o output file name (ASUME USING PAPI)\n");
  fprintf(stderr, "   -n Number of different programs to run. It defaults to the"
        " number of progs arguments passed. Should be used as a control\n");
  fprintf(stderr, "   -l Number of times the main loop of the ROI should be"
        " executed before the creation of the checkpoint (end of spec_cast)\n");
  fprintf(stderr, "        %s -d -p 8 -n 2 -l 2 --mcf --bwaves 2\n", argv0);
  fprintf(stderr, "      will run 4 instances of \"mcf \" and 4 instances of"
        " \"bwaves\" (with input bwaves_2) and stop after 2 loops of the ROI\n");
  fprintf(stderr, "\n");

  exit(EXIT_FAILURE);
}

static void bind_pid (int cpu_num, pid_t PID_NUM)
{
    // PID_NUM should be 0 for the current process, or whatever if the parent does
    // everything
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    //CPU_SET(2, &mask);
    unsigned int len = sizeof(mask);
    if(cpu_num < max_num_processors)
    {
        CPU_SET(cpu_num, &mask);
        int rc=sched_setaffinity(PID_NUM, len, &mask);
        if (rc < 0) {
            fprintf(stderr, "[E] Trying %d in processor %d", (int)PID_NUM, cpu_num);
            //fprintf(stderr, "\n [E] max number of procs %d", CPU_SETSIZE);
            if(errno == ESRCH)
            { fprintf(stderr, "No process or thread with the given ID found."); }
            if(errno == EFAULT)
            { fprintf(stderr, "The pointer cpuset does not point to a valid object."); }
            if(errno == EINVAL)
            { fprintf(stderr, "The bitset is not valid. This might mean that "
                "the affinity set might not leave a processor for the process "
                "or thread to run on."); }
            perror("[E] sched_setaffinity error");
        } else
        {
            printf("[I] %i bound to processor %i\n", (int)PID_NUM, cpu_num);
        }
    }
    else
    {
        fprintf(stderr, "[W] Cannot bind more than %i pids\n", max_num_processors);
    }
}

void sig_handler()
{
    printf("[E] Interrupted by SIGALRM\n");
    printf("[E] Test Fail: block on pthread_barrier_wait()\n");
    exit(-1);
}

static struct option long_options[] =
{
    /* These options set a flag. */
    {"perlbench", required_argument, 0, 1},
    {"gcc",       required_argument, 0, 1},
    {"bwaves",    required_argument, 0, 1},
    {"mcf",       no_argument, 0, 0},
    {"cactuBSSN", no_argument, 0, 0},
    {"namd",      no_argument, 0, 0},
    {"parest",    no_argument, 0, 0},
    {"povray",    no_argument, 0, 0},
    {"lbm",       no_argument, 0, 0},
    {"omnetpp",   no_argument, 0, 0},
    {"wrf",       no_argument, 0, 0},
    {"xalancbmk", no_argument, 0, 0},
    {"x264",      required_argument, 0, 1},
    {"blender",   no_argument, 0, 0},
    {"cam4",      no_argument, 0, 0},
    {"deepsjeng", no_argument, 0, 0},
    {"imagick",   no_argument, 0, 0},
    {"leela",     no_argument, 0, 0},
    {"nab",       no_argument, 0, 0},
    {"exchange2", no_argument, 0, 0},
    {"fotonik3d", no_argument, 0, 0},
    {"roms",      no_argument, 0, 0},
    {"xz",        required_argument, 0, 1},
    {"help",      no_argument, &print_help, 1},
    {0, 0, 0, 0}
};

void handle_error (int retval)
{
     if(retval>0)
     {
          printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
     }
     kill(0,SIGKILL);
     exit(retval);
}

void get_options(int argc, char** argv, int* waiting, int* gem5_work_op, int* use_papi, char (*app)[MAX_APP_LENGTH], 
      int* sub_app, char* config, int* num_processors, int* num_apps, int* num_loops, int* use_csv, int* sleep_sec)
{
    int option_index = 0;
    int app_index=0;
    int c = 0;
      
    while (c >= 0)
    {
        c = getopt_long(argc, argv, "wbp:n:l:c:v:o:s:", long_options, &option_index);

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

            case 's':
            *use_papi=1;
            *sleep_sec=atoi(optarg);
            printf("Using PAPI for %d seconds\n", *sleep_sec);
            break;

            case 'w':
            printf("Executing GEM5 work_begin op\n");
            *gem5_work_op=1;
            break;

            case 'b':
            printf ("Background Execution\n");
            *waiting=0;
            break;

            case 'p':
            *num_processors=atoi(optarg);
            printf("Number of processors %d\n", *num_processors);
            break;

            case 'n':
            *num_apps=atoi(optarg);
            printf("Number of applications %d\n", *num_apps);
            break;

            case 'l':
            *num_loops=atoi(optarg);
            printf("Number of loops in ROI: %d\n", *num_loops);
            break;

            case 'c':
            strcpy(config,optarg);
            printf("Config name: %s\n", config);
            break;

            case 'v':
            *use_csv=1;
            if(*use_papi==0)
            {      
                  *use_papi=1;
                  fprintf(stderr,"WARNING! Using PAPI due to -v option\n");
            }
            strcpy(csv_filename,optarg);
            printf("CSV file name: %s\n", csv_filename);
            break;

            case 'o':
            if(*use_papi==0)
            {      
                  *use_papi=1;
                  fprintf(stderr,"WARNING! Using PAPI due to -o option\n");
            }
            strcpy(filename,optarg);
            printf("Output file name: %s\n", filename);
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

    if (app_index <= 0)
    {
        printf("\n** ERROR: No applications defined. Need at least one app **\n\n");
        usage(argv[0]);
    }

    if(*num_processors == 0)
    {
        *num_processors = max_num_processors;
        printf("Number of processors not defined, set to %d\n", *num_processors);
    } else if(*num_processors > max_num_processors)
    {
        fprintf(stderr, "Number of processors set to %d and only %d available\n"
        ,*num_processors, max_num_processors);
        exit(-1);
    }
    if(*num_apps == 0)
    {
        *num_apps = app_index;
        printf("Number of applications not defined, set to %d\n", *num_apps);
    }
    else
    {
        if(*num_apps != app_index)
        {
            fprintf(stderr,"Only %d of %d apps have been specified\n",
            app_index, *num_apps);
            exit(-1);
        }
    }

    if (*num_apps > *num_processors)
    {
        fprintf(stderr, "Number of applications (%d) greater than the number of"
        " processors (%d)\n", *num_apps, *num_processors);
        exit(-1);
    }
}

void init_papi(int num_papi_loops, pid_t* pids, int num_procs, char (*apps)[MAX_APP_LENGTH], int num_apps, int* EventSet)
{
      int retval;
      int i=0, count=0;
      pid_t temp_pid=0;

      for(i=0; i<num_procs; i++)
      {
            if(pids[i]<=0)
            {
                  fprintf(stderr,"Error, pid %d tiene un valor %d\n", i, pids[i]);
                  handle_error(1);
            }
            EventSet[i]=PAPI_NULL;
            do
            {
                  char str[60] = "ps -o pid --ppid ";
                  char ppid [12];
                  sprintf(ppid,"%d",pids[i]);
                  strcat(str,ppid);
                  strcat(str," | grep -v PID");
                  FILE *pfp = popen(str, "r");
                  fscanf(pfp, "%d", &temp_pid);
                  pclose(pfp);
                  printf("Read %d\n", temp_pid);
                  if((temp_pid > 0 && temp_pid != pids[i]))
                  {
                        printf("PID %d now %d\n", pids[i], temp_pid);
                        pids[i]=temp_pid;
                  }
                  else
                  {
                        temp_pid=0;
                  }
                  
            } while(temp_pid > 0);
      }
      retval = PAPI_library_init(PAPI_VER_CURRENT);
      if(retval != PAPI_VER_CURRENT && retval > 0)
      {
            fprintf(stderr,"Error in PAPI_library_init\n");
            handle_error(retval);
      }
      for(i=0; i<num_procs; i++)
      {
            retval = PAPI_create_eventset(&(EventSet[i]));
            if (retval != PAPI_OK)
            {
                  fprintf(stderr, "Error in PAPI_create_eventset\n");
                  handle_error(retval);
            }
            
            if (PAPI_add_event(EventSet[i], PAPI_TOT_CYC) != PAPI_OK)
                  handle_error(1);
            if (PAPI_add_event(EventSet[i], PAPI_TOT_INS) != PAPI_OK)
                  handle_error(1);
            if (PAPI_add_event(EventSet[i], PAPI_L3_TCM) != PAPI_OK)
                  handle_error(1);
            if (PAPI_add_event(EventSet[i], PAPI_L3_TCA) != PAPI_OK)
                  handle_error(1);

            /* Attach this EventSet to the forked process */
            retval=PAPI_attach(EventSet[i], pids[i]);
            if (retval != PAPI_OK)
            {
                  fprintf(stderr, "Error in PAPI_attach\n");
                  handle_error(retval);
            }
      }
      printf("PAPI initialization done\n");
}

void do_papi(int num_papi_loops, int num_secs, pid_t* pids, int num_procs, char (*apps)[MAX_APP_LENGTH], int num_apps, int use_csv, int* EventSet)
{
      int retval;
      unsigned long_long values[num_procs][4];
      int i=0, count=0;
      FILE *fp, *cfp;

      for(i=0; i<num_procs; i++)
      {
            retval=PAPI_start(EventSet[i]);
            if (retval != PAPI_OK)
            {
                  fprintf(stderr, "Error in PAPI_start\n");
                  handle_error(retval);
            }
            else
            {
                  printf("Starting PAPI %d(PID:%d): %d times %d\n", 
                  i, pids[i], num_papi_loops, num_secs);
            }
      }

      fp = fopen(filename, "w+");
      if(fp == NULL)
      {
            fprintf(stderr, "Error opening file %s\n", filename);
            handle_error(1);
      }
      if(use_csv)
      {
            cfp = fopen(csv_filename, "w+");
            if(cfp == NULL)
            {
                  fprintf(stderr, "Error opening file %s\n", csv_filename);
                  handle_error(1);
            }
      }
      float miss_rate = 0.0, ipc = 0.0, mpki = 0.0;

      for(count=0; count<num_papi_loops; count++)
      {
            sleep(num_secs);
            for(i=0; i<num_procs; i++)
            {
                  if (PAPI_read(EventSet[i], values[i]) != PAPI_OK)
                        handle_error(1);
                  if (values[i][3] > 0)
                        miss_rate = ((float)values[i][2]) / ((float)values[i][3]);
                  if (values[i][0] > 0)
                        ipc = ((float)values[i][1]) / ((float)values[i][0]);
                  if (values[i][1] > 0)
                        mpki = 1000.0*(((float)values[i][2]) / ((float)values[i][1]));
                  fprintf(fp, "Loop %d, PID:%d - %s\n", count, pids[i], apps[i%num_apps]);
                  fprintf(fp, "%d-%s Cycles %lld\n", count, apps[i%num_apps], values[i][0]);
                  fprintf(fp, "%d-%s Instructions %lld\n", count, apps[i%num_apps], values[i][1]);
                  fprintf(fp, "%d-%s L3 misses %lld\n", count, apps[i%num_apps], values[i][2]);
                  fprintf(fp, "%d-%s L3 accesses %lld\n", count, apps[i%num_apps], values[i][3]);
                  fprintf(fp, "%d-%s L3 Miss rate %.5f\n", count, apps[i%num_apps], miss_rate);
                  fprintf(fp, "%d-%s L3 MPKI %.5f\n", count, apps[i%num_apps], mpki);
                  fprintf(fp, "%d-%s IPC %.5f\n", count, apps[i%num_apps], ipc);
                  fprintf(fp, "********************\n\n");
            }
      }

      for(i=0; i<num_procs; i++)
      {
            if (PAPI_stop(EventSet[i], values[i]) != PAPI_OK)
                  handle_error(4);
                  if (values[i][3] > 0)
                        miss_rate = ((float)values[i][2]) / ((float)values[i][3]);
                  if (values[i][0] > 0)
                        ipc = ((float)values[i][1]) / ((float)values[i][0]);
                  if (values[i][1] > 0)
                        mpki = 1000.0*(((float)values[i][2]) / ((float)values[i][1]));
                  fprintf(fp, "END %d, PID:%d - %s\n", count, pids[i], apps[i%num_apps]);
                  fprintf(fp, "Final %d-%s Cycles %lld\n", count, apps[i%num_apps], values[i][0]);
                  fprintf(fp, "Final %d-%s Instructions %lld\n", count, apps[i%num_apps], values[i][1]);
                  fprintf(fp, "Final %d-%s L3 misses %lld\n", count, apps[i%num_apps], values[i][2]);
                  fprintf(fp, "Final %d-%s L3 accesses %lld\n", count, apps[i%num_apps], values[i][3]);
                  fprintf(fp, "Final %d-%s L3 Miss rate %.5f\n", count, apps[i%num_apps], miss_rate);
                  fprintf(fp, "Final %d-%s L3 MPKI %.5f\n", count, apps[i%num_apps], mpki);
                  fprintf(fp, "Final %d-%s IPC %.5f\n", count, apps[i%num_apps], ipc);
                  fprintf(fp, "********************\n\n");
                  if(use_csv==1)
                  {
                     fprintf(cfp, "CPU%d;%d;%lld;;instructions;%s\n", i,count,values[i][1],apps[i%num_apps]);
                     fprintf(cfp, "CPU%d;%d;%lld;;cycles;%s\n", i,count,values[i][0],apps[i%num_apps]);
                     fprintf(cfp, "CPU%d;%d;%.8f;;miss_rate;%s\n", i,count,miss_rate,apps[i%num_apps]);
                     fprintf(cfp, "CPU%d;%d;%.8f;;MPKI;%s\n", i,count,mpki,apps[i%num_apps]);
                  }
      }
      if(fflush(fp))
            perror("fflush error\n");
      if(fclose(fp))
            perror("fclose error\n");
      if(use_csv==1)
      {
            if(fflush(cfp))
                  perror("fflush csv error\n");
            if(fclose(cfp))
                  perror("fclose csv error\n"); 
      }
}
