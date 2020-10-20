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

#define MAX_CWD 600
#define MAX_APP_LENGTH 40
#define MAX_EVENT_LENGTH 40
#define MAX_NUM_EVENTS 20
static const char shm_name[] = "tmp_pthread_barrierattr_getpshared";

typedef struct bench_barrier {
   pthread_barrier_t barrier;
   bool doWait;;
} bench_barrier_t;

static int max_num_processors;
static int print_help;

static void usage(char *argv0) {

  fprintf(stderr, "Usage: %s [-b] [-w] [-p number] [-n number] [-l number] "
        "[-c config_name] [-v csv_filename] [-s seconds] [-r number] [-m N]"
        "--<BENCH> [app1.cmd] [--<BENCH2> [app2.cmd] --<BENCH3> [app3.cmd] ...]\n", argv0);
  fprintf(stderr, "   BENCH.app is the program/s you want to cast. Applications usually"
        " require an aditional param to indicate the exact command line to execute\n"
        " # BENCH is the benchmark that application belongs to (SPEC, NPB, PARSEC...)\n"
        " # app is the current program (is, ft, bwaves, mcf, namd...)\n"
        " # cmd is the exact version of tha app to execute (is A...)"
        " Check LAUNCH_CMDS.py for the SPEC 2017 version of the cmd options\n");
  fprintf(stderr, "   -b makes %s to return immediately after launching the "
        "programs. The default is to wait for them to finish.\n", argv0);
  fprintf(stderr, "   -p is the number of processors you want to use from the "
        "system (default value: the number of available processors in the "
        "system, i.e: %d)\n", max_num_processors);
  fprintf(stderr, "   -w should be used with GEM5, and executes a "
        "m5_work_begin_op\n");
  fprintf(stderr, "   -s use papi library to analyze performance counters during "
         "N seconds\n");
  fprintf(stderr, "   -c SPEC config name\n");
  fprintf(stderr, "   -v print papi results in a csv file (PAPI IS ASSUMED)\n");
  fprintf(stderr, "   -r repeat papi measures N times (PAPI IS ASSUMED, and csv output)\n");
  fprintf(stderr, "   -n Number of different programs to run. It defaults to the"
        " number of progs arguments passed. Should be used as a control\n");
  fprintf(stderr, "   -l Number of times the main loop of the ROI should be"
        " executed before the creation of the checkpoint (end of bench_cast)\n");
  fprintf(stderr, "   -m Measure the behavior of the N first cores. BenchCast reserves those cores"
        " so no benchmark is launched there (PAPI is assumed, but not needed).");
  fprintf(stderr, "        %s -c bench-cast -d -p 8 -n 2 -l 2 --NPB is.A --SPEC bwaves.2\n", argv0);
  fprintf(stderr, "      will run 4 instances of \"is.A \" and 4 instances of"
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

static struct option long_options[] =
{
    /* These options set a flag. */
    {"SPEC",      required_argument, 0, 1},
    {"NPB",       required_argument, 0, 1},
    {"PARSEC",    required_argument, 0, 1},
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
      char (*sub_app)[MAX_APP_LENGTH], char* config, int* num_processors, int* num_apps, int* num_loops, int* use_csv, int* sleep_sec, 
      int* num_papi_loops, char* csv_filename, int *initcore)
{
    int option_index = 0;
    int app_index=0;
    int c = 0;
      
    while (c >= 0)
    {
        c = getopt_long(argc, argv, "wbp:n:l:c:v:s:r:m:", long_options, &option_index);

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
            strcpy(sub_app[app_index],"1");
            app_index++;
            break;

            case 1:
            //long option with required arguments
            printf ("App: %s\n", long_options[option_index].name);
            strcpy(app[app_index], long_options[option_index].name);
            strcpy(sub_app[app_index],optarg);
            if(sub_app[app_index]==NULL)
            {
                fprintf(stderr, "\n*** ERROR *** Application \"%s\"", app[app_index]); 
                fprintf(stderr, " requires benchmark number \n\n\n");
                usage(argv[0]);
                break;
            }
            printf("Using benchmark %s of %s\n", sub_app[app_index], app[app_index]);
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

            case 'm':
            *initcore=atoi(optarg);
            printf("Measuring the behavior of first %d cores\n", *initcore);
            if (*use_papi==0)
                  fprintf(stderr, "WARNING! Not using PAPI with -m option\n");
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

            case 'r':
            if(*use_csv==0)
            {
                  fprintf(stderr,"WARNING! -r option without CSV file (-v option)\n");  
                  usage(argv[0]);
                  exit(1);
            }
            if(*use_papi==0)
            {      
                  *use_papi=1;
                  fprintf(stderr,"WARNING! Using PAPI due to -r option\n");
            }
            *num_papi_loops=atoi(optarg);
            printf("Periodic PAPI measure: %d times", *num_papi_loops);
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

void create_event_set(int* EventSet, int cpu_num, char (*event_list)[MAX_EVENT_LENGTH], int* num_events)
{
      FILE * efp;
      char * line = NULL;
      size_t len = 0;
      ssize_t read;
      *num_events=0;
      //Create event Set
      int retval = PAPI_create_eventset(&(EventSet[cpu_num]));
      if (retval != PAPI_OK)
      {
            fprintf(stderr, "Error in PAPI_create_eventset\n");
            handle_error(retval);
      }
      //Add events to the event set and event list from BenchCast_PAPI_events.cfg
      efp = fopen("BenchCast_PAPI_events.cfg", "r");
      if (efp == NULL)
      {
            fprintf(stderr, "ERROR reading BenchCast_PAPI_events.cfg\n");
            exit(1);
      }
      while ((read = getline(&line, &len, efp)) != -1) {
            line[strcspn(line, "\r\n")] = 0;
            retval=PAPI_add_named_event(EventSet[cpu_num], line);
            if (retval != PAPI_OK)
            {
                  fprintf(stderr, "[PAPI] Not able to include %s event\n", line);
                  fprintf(stderr, "PAPI Error %d: %s\n", retval, PAPI_strerror(retval));
            }
            else
            {
                  strcpy(event_list[*num_events], line);
                  printf("[PAPI] Adding %s event to the list\n", event_list[*num_events]);
                  *num_events=(*num_events)+1;
            }
      }
      printf("\n[PAPI] Proc %d using %d counters of %d available\n", cpu_num, *num_events, PAPI_num_hwctrs());
}

void init_papi(pid_t* pids, int num_procs, int* EventSet, char (*event_list)[MAX_EVENT_LENGTH], int init_proc, int* num_events)
{
      int retval;
      int i=0, count=0;
      pid_t temp_pid=0;

      for(i=0; i<num_procs+init_proc; i++)
      {
            EventSet[i]=PAPI_NULL;
      }

      for(i=init_proc; i<num_procs+init_proc; i++)
      {
            if(pids[i]<=0)
            {
                  fprintf(stderr,"Error, pid %d tiene un valor %d\n", i, pids[i]);
                  handle_error(1);
            }
            do
            {
                  char str[60] = "ps -o pid --ppid ";
                  char ppid [12];
                  sprintf(ppid,"%d",pids[i]);
                  strcat(str,ppid);
                  strcat(str," | grep -v PID");
                  FILE *pfp = popen(str, "r");
                  retval=fscanf(pfp, "%d", &temp_pid);
                  if(retval==EOF)
                        break;
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
      if(retval != PAPI_VER_CURRENT)
      {
            fprintf(stderr,"Error in PAPI_library_init\n");
            handle_error(retval);
      }
      printf("[PAPI] Library Init OK\n");
      int events_found=0;
      for(i=0; i<num_procs+init_proc; i++)
      {
            create_event_set(EventSet, i, event_list, num_events);
            if(i==0)
            {
                  events_found=*num_events;
            }
            else
            {
                  if(events_found!=*num_events)
                  {
                        fprintf(stderr, "ERROR, number of events found in proc %d is %d, while till now they were %d", 
                              i, *num_events, events_found);
                        exit(1);
                  }
            }
            
      }
      for(i=0; i<init_proc; i++)
      {
            PAPI_granularity_option_t gran_opt;
            PAPI_cpu_option_t cpu_opt;
            gran_opt.def_cidx=0;
            gran_opt.eventset=EventSet[i];
            gran_opt.granularity=PAPI_GRN_SYS;
            retval = PAPI_set_opt(PAPI_GRANUL,(PAPI_option_t*)&gran_opt);
            if (retval != PAPI_OK) {
                  printf("Unable to set PAPI_GRN_SYS\n");
                  exit(1);
            }
            
            cpu_opt.eventset=EventSet[i];
            cpu_opt.cpu_num=i;

            retval = PAPI_set_opt(PAPI_CPU_ATTACH,(PAPI_option_t*)&cpu_opt);
            if (retval != PAPI_OK) {
                  if (retval==PAPI_EPERM) {
                        printf("Permission error trying to CPU_ATTACH; need to run as root\n");
                  }
                  fprintf(stderr, "PAPI_CPU_ATTACH FAIL %d",retval);
                  exit(1);
            }
      }
      for(i=init_proc; i<num_procs+init_proc; i++)
      {
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

void do_papi(int num_papi_loops, int num_secs, pid_t* pids, int num_procs, char (*apps)[MAX_APP_LENGTH], char (*sub_app)[MAX_APP_LENGTH], 
            int num_apps, int use_csv, int* EventSet, char* csv_filename, char (*event_list)[MAX_EVENT_LENGTH], int init_proc, int num_events)
{
      int retval, app_index;
      long long values[num_procs+init_proc][num_events];
      int i=0, j=0, count=0;
      FILE *cfp, *pcfp;
      char periodic_filename[MAX_CWD];

      for(i=0; i<num_procs+init_proc; i++)
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

      if(use_csv)
      {
            cfp = fopen(csv_filename, "w+");
            if(cfp == NULL)
            {
                  fprintf(stderr, "Error opening file %s\n", csv_filename);
                  handle_error(1);
            }
            if(num_papi_loops>1)
            {
                  strcpy(periodic_filename,csv_filename);
                  strcpy(periodic_filename,".periodic.csv");
                  pcfp = fopen(periodic_filename, "w+");
                  if(pcfp == NULL)
                  {
                        fprintf(stderr, "Error opening file %s\n", periodic_filename);
                        handle_error(1);
                  }
            }
      }
      char my_app[MAX_APP_LENGTH];
      char my_sub_app[MAX_APP_LENGTH];
      for(count=0; count<num_papi_loops; count++)
      {
            sleep(num_secs);
            app_index=0;
            for(i=0; i<num_procs+init_proc; i++)
            {
                  if (PAPI_read(EventSet[i], values[i]) != PAPI_OK)
                        handle_error(1);

                  if(i>=init_proc)
                  {
                        strcpy(my_app,apps[app_index]);
                        strcpy(my_sub_app,sub_app[app_index]);
                        app_index++;
                        app_index=app_index%num_apps;
                  }
                  else
                  {
                        strcpy(my_app,"Unknown");
                        strcpy(my_sub_app, "Unknown");
                  }
                  if(use_csv==1 && num_papi_loops>1)
                  {
                        for(j=0; j<num_events; j++)
                        {
                              fprintf(pcfp, "CPU%d;%d;%lld;;%s;%s-%s\n", i,count,values[i][j],event_list[j],my_app,my_sub_app);
                        }

                  } else
                  {
                        printf("Loop %d, PID:%d - %s-%s\n", count, pids[i], my_app,my_sub_app);
                        for(j=0; j<num_events; j++)
                        {
                              printf("%d-%s-%s %s %lld\n", i, my_app,my_sub_app,event_list[j], values[i][j]);
                        }
                        printf("********************\n\n");
                  }                  
            }
      }
      printf("Ending PAPI\n");
      app_index=0;
      for(i=0; i<num_procs+init_proc; i++)
      {
            if(i>=init_proc)
            {
                  strcpy(my_app,apps[app_index]);
                  strcpy(my_sub_app,sub_app[app_index]);
                  app_index++;
                  app_index=app_index%num_apps;
            }
            else
            {
                  strcpy(my_app,"Unknown");
                  strcpy(my_sub_app, "Unknown");
            }
            printf("Stopping PAPI %d\n", i);
            if (PAPI_stop(EventSet[i], values[i]) != PAPI_OK)
                  handle_error(4);

            if(use_csv==1)
            {
                  for(j=0; j<num_events; j++)
                  {
                        fprintf(pcfp, "CPU%d;%d;%lld;;%s;%s-%s\n", i,count,values[i][j],event_list[j],my_app,my_sub_app);
                  }
            }
            else
            {
                  printf("END Loop %d, PID:%d - %s-%s\n", count, pids[i], my_app,my_sub_app);
                  for(j=0; j<num_events; j++)
                  {
                        printf("%d-%s-%s %s %lld\n", i, my_app,my_sub_app, event_list[j], values[i][j]);
                  }
                  printf("********************\n\n");
            }
            
      }
      if(use_csv==1)
      {
            if(fflush(cfp))
                  perror("fflush csv error\n");
            if(fclose(cfp))
                  perror("fclose csv error\n");
            if(num_papi_loops>1)
            {
                  if(fflush(pcfp))
                        perror("fflush periodic csv error\n");
                  if(fclose(pcfp))
                        perror("fclose periodic csv error\n");
            }
      }
}
