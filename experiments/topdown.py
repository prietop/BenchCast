#!/usr/bin/python
#

from experiment_list import exp_list
import argparse
import sys
import subprocess
import os
import signal
from random import seed
from random import randint
import multiprocessing
import pandas
from scipy import stats
import numpy as np


class bcolors:
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    BLUE = "\033[94m"
    ENDC = '\033[0m'


def checkDir(dir_to_check):
    """Checks the existance of a directory"""
    if not os.path.exists(dir_to_check):
        printColor(("Directory not found: %s" % dir_to_check), "red")
        exit()


def printColor(string_to_print, color):
    """Prints messages to stdout of a defined color"""
    if color == "green":
        print(bcolors.OKGREEN + string_to_print + bcolors.ENDC)
    elif color == "red":
        print(bcolors.FAIL + string_to_print + bcolors.ENDC)
    elif color == "blue":
        print(bcolors.BLUE + string_to_print + bcolors.ENDC)
    else:
        print(bcolors.FAIL + "Bad color argument to print" + bcolors.ENDC)


def runCommand(command):
    """Run a shell command"""
    print(command)
    ret_code = subprocess.call(command, shell=True)
    if ret_code:
        printColor(("bad return code for command execution %s: %s" %
                   (command, ret_code)), "red")
        exit()
    return ret_code


def normalityTest(DataSet, alpha, distr='norm'):
    # Perform Kolmogorov-Smirnov test for normality
    stat = 0.0
    p = 0.0
    if distr == 'lognorm':
        stat, p = stats.kstest(DataSet, distr, stats.lognorm.fit(DataSet))
    else:
        stat, p = stats.kstest(DataSet, distr)
    print('Statistic=%.3f, p=%.18f' % (stat, p))
    if p > alpha:
        printColor('Sample looks %s' % distr, 'green')
    else:
        printColor('Sample does not look %s' % distr, 'red')
    return p


def launchBenchCast(num_cpus, app_list, outfile, config, bench_cast_seconds, event_file):
    """ Launch BENCHCAST """
    printColor("Launching bench_cast with apps: %s" % app_list, "blue")
    exec_cmd = "./bench_cast -c %s -v %s -s %d -l 2 -p %d -e %s %s" % (
        config, outfile, bench_cast_seconds, num_cpus, event_file, app_list)
    runCommand(exec_cmd)
    printColor("Bench cast Ended", "blue")


def getPerfHybridValues(file_path, num_cores, metric):
    ret_val = []
    df = pandas.read_csv(file_path,
                         index_col='Event',
                         sep=';',
                         names=['Core', 'N', 'Value', 'Units', 'Event', 'others'])
    # print(df)
    if df.empty:
        printColor("Nothing found at %s" % file_path, "red")
        exit()
    for x in range(num_cores):
        ret_val.append(float(df['Value'][metric][x]))
    # print(ret_val)
    return ret_val


parser = argparse.ArgumentParser(description='')
parser.add_argument('--numtests', type=int, default=1,
                    help='Number of workloads to execute for the system evaluation.')
parser.add_argument('--seconds', type=int, default=60,
                    help='Number of seconds each tests last')
parser.add_argument('--outdir', type=str, default='exp1-python', action="store",
                    help="Name of the output directory to store...")
parser.add_argument('--filename', type=str, default='data.csv', action="store",
                    help='Name of the output file csv')
parser.add_argument('--infile', type=str, action="store",
                    help='Name of the input file csv')
parser.add_argument('--max_apps', type=int, default=8,
                    help='Maximum number of apps in a mix')
parser.add_argument('--num_procs', type=int,
                    help='Number of processors')
parser.add_argument('--eventfile', type=str, action="store",
                    help='Name of the file with the event list')
parser.add_argument('--reffile', type=str, action="store",
                    help='Name of the input file csv to get the experiments')


args = parser.parse_args()

output_dir = args.outdir
bench_cast_seconds = args.seconds
max_apps = args.max_apps

num_cpus = multiprocessing.cpu_count()
if args.num_procs:
    num_cpus = args.num_procs

seed(os.getpid())

original_dir = os.getcwd()
output = os.path.join(original_dir, output_dir)
events_file_name = os.path.join(original_dir, "TOPDOWN_events.cfg")
if args.eventfile:
    events_file_name = os.path.join(original_dir, args.eventfile)
if not os.path.exists(output):
    os.makedirs(output)
checkDir(output)
events_file = open(events_file_name, 'r')
events = events_file.readlines()
event_list = []
for line in events:
    event_list.append(line.strip())

count = 0
average_events = []
total_events = []
for event in event_list:
    average_events.append('Avg_'+event)
    total_events.append('Total_'+event)

if args.infile:
    print("reading dataframe from %s" % args.infile)
    BENCHCASTS_DF = pandas.read_csv(args.infile, index_col=0)
    count = len(BENCHCASTS_DF.index)
else:
    column_list = ['Benchmark']
    column_list.extend(event_list)
    column_list.extend(average_events)
    column_list.extend(total_events)
    BENCHCASTS_DF = pandas.DataFrame(columns=column_list)

if args.reffile:
    refdata = pandas.read_csv(args.reffile, index_col=0)
    benchmarks = refdata['Benchmark']
    for b in benchmarks:
        bench_split = b.split('-')
        bench_split.pop(0)  # First element is null
        print(bench_split)
        app_list = ""
        for t in bench_split:
            app = t.split('.')
            app_list = "%s --%s %s.%s" % (app_list, app[0], app[1], app[2])
        filename = "papi-%dp-%dsec%s-%d.csv" % (
            num_cpus, bench_cast_seconds, b, count)
        outputfile = os.path.join(output, filename)
        open(outputfile, 'a').close()
        launchBenchCast(num_cpus, app_list, outputfile, config='spec-cast',
                        bench_cast_seconds=bench_cast_seconds, event_file=events_file_name)
        new_row = {'Benchmark': b}
        BENCHCASTS_DF = BENCHCASTS_DF.append(
            pandas.Series(new_row), ignore_index=True)
        skip = 0
        for event in event_list:
            value = getPerfHybridValues(outputfile, num_cpus, event)
            if 0.0 in value:
                printColor("Application do not run during execution", "red")
                skip = 1
                break
        if skip == 0:
            for event in event_list:
                value = getPerfHybridValues(outputfile, num_cpus, event)
                BENCHCASTS_DF.loc[BENCHCASTS_DF.index[-1],
                                  event] = '['+','.join(map(str, value))+']'
                BENCHCASTS_DF.loc[BENCHCASTS_DF.index[-1],
                                  'Avg_'+event] = np.mean(value)
                BENCHCASTS_DF.loc[BENCHCASTS_DF.index[-1],
                                  'Total_'+event] = np.sum(value)
            count = count+1
            BENCHCASTS_DF.to_csv(args.filename)
else:
    while count < args.numtests:
        bench = []
        app = []
        cmd = []
        procs = []
        app_list = ""
        batch_name = ""
        sort_name = ""
        j = 0
        for n in range(num_cpus):
            if n < max_apps:
                bench_num = randint(0, len(exp_list)-1)
                bench.append(list(exp_list)[bench_num])
                current_bench = list(exp_list.values())[bench_num]
                app_num = randint(0, len(current_bench)-1)
                app.append(list(current_bench)[app_num])
                current_app = list(current_bench.values())[app_num]
                cmd_num = randint(0, len(current_app)-1)
                cmd.append(list(current_app)[cmd_num])
                print("--%s %s.%s" % (bench[n], app[n], cmd[n]))
                j = n
                sort_name = "%s-%s.%s.%s" % (sort_name,
                                             bench[j], app[j], cmd[j])
            else:
                j = n % max_apps
            batch_name = "%s-%s.%s" % (batch_name, app[j], cmd[j])
            app_list = "%s --%s %s.%s" % (app_list, bench[j], app[j], cmd[j])
        filename = "papi-%dp-%dsec%s-%d.csv" % (
            num_cpus, bench_cast_seconds, sort_name, count)
        outputfile = os.path.join(output, filename)
        open(outputfile, 'a').close()
        launchBenchCast(num_cpus, app_list, outputfile, config='spec-cast',
                        bench_cast_seconds=bench_cast_seconds, event_file=events_file_name)
        #new_row={'Benchmark': batch_name}
        new_row = {'Benchmark': sort_name}
        BENCHCASTS_DF = BENCHCASTS_DF.append(
            pandas.Series(new_row), ignore_index=True)
        skip = 0
        for event in event_list:
            value = getPerfHybridValues(outputfile, num_cpus, event)
            if 0.0 in value:
                printColor("Application do not run during execution", "red")
                skip = 1
                break
        if skip == 0:
            for event in event_list:
                value = getPerfHybridValues(outputfile, num_cpus, event)
                BENCHCASTS_DF.loc[BENCHCASTS_DF.index[-1],
                                  event] = '['+','.join(map(str, value))+']'
                BENCHCASTS_DF.loc[BENCHCASTS_DF.index[-1],
                                  'Avg_'+event] = np.mean(value)
                BENCHCASTS_DF.loc[BENCHCASTS_DF.index[-1],
                                  'Total_'+event] = np.sum(value)
            count = count+1
            BENCHCASTS_DF.to_csv(args.filename)

printColor("THE END!", "green")
