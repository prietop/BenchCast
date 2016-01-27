/*
Author: Pablo Prieto Torralbo <prietop@unican.es>
*/

#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h> // linux

#define MAX_CWD 80
#define MAX_THREADS 128

static int max_num_processors;
static int print_help;

static void usage(char *argv0) {

  fprintf(stderr, "Usage: %s [-b] [-d] [-w] [-p number] [-n number] --<prog> "
        "[--<prog2> --<prog3> ...]\n", argv0);
  fprintf(stderr, "  prog is the program/s you want to cast.\n");
  fprintf(stderr, "  -b makes %s to return immediately after launching the "
        "programs. The default is to wait for them to finish.\n", argv0);
  fprintf(stderr, "  -d cd to the directory of the benchmark before running "
        "the program. The directory should be the name of the program, for all "
        "the programs.\n");
  fprintf(stderr, "  -p is the number of processors you want to use from the "
        "system (default value: the number of available processors in the "
        "system, i.e: %d)\n", max_num_processors);
  fprintf(stderr, "  -w should be used with GEM5, and executes a "
        "m5_work_begin_op\n");
  fprintf(stderr, "  -n Number of different programs to run. It defaults to the"
        " number of progs arguments passed. Should be used as a control\n");
  fprintf(stderr, "        %s -d -p 8 -n 2 --bzip2 --gcc\n", argv0);
  fprintf(stderr, "      will run 4 instances of \"bzip2 \" and 4 instances of"
        " \"gcc\" executing the \"launch_script.sh\" located in their "
        "respectives directories (this launc_script.sh sould have the "
        "appropiate command for the application execution)\n");
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
            { fprintf(stderr, "The pointer cpuset is does not point to a valid object."); }
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
    {"perlbench", no_argument, 0, 0},
    {"bzip2",     no_argument, 0, 0},
    {"gcc",       no_argument, 0, 0},
    {"bwaves",    no_argument, 0, 0},
    {"gamess",    no_argument, 0, 0},
    {"mcf",       no_argument, 0, 0},
    {"milc",      no_argument, 0, 0},
    {"zeusmp",    no_argument, 0, 0},
    {"gromacs",   no_argument, 0, 0},
    {"cactusADM", no_argument, 0, 0},
    {"leslie3d",  no_argument, 0, 0},
    {"namd",      no_argument, 0, 0},
    {"gobmk",     no_argument, 0, 0},
    {"dealII",    no_argument, 0, 0},
    {"soplex",    no_argument, 0, 0},
    {"povray",    no_argument, 0, 0},
    {"calculix",  no_argument, 0, 0},
    {"hmmer",     no_argument, 0, 0},
    {"sjeng",     no_argument, 0, 0},
    {"GemsFDTD",  no_argument, 0, 0},
    {"libquantum", no_argument, 0, 0},
    {"h264ref",   no_argument, 0, 0},
    {"tonto",     no_argument, 0, 0},
    {"lbm",       no_argument, 0, 0},
    {"omnetpp",   no_argument, 0, 0},
    {"astar",     no_argument, 0, 0},
    {"wrf",       no_argument, 0, 0},
    {"sphinx3",   no_argument, 0, 0},
    {"xalancbmk", no_argument, 0, 0},
    {"specrand",  no_argument, 0, 0},
    {"help",      no_argument, &print_help, 1},
    {0, 0, 0, 0}
};
