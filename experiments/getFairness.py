#!/usr/bin/python
#

from experiment_list import exp_list
import argparse
import os
import re
import subprocess
import glob
import multiprocessing
from random import seed
from random import randint
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


def launchBenchCast(num_cpus, app_list, outfile, config, bench_cast_seconds, event_file):
    """ Launch BENCHCAST """
    printColor("Launching bench_cast with apps: %s" % app_list, "blue")
    exec_cmd = "./bench_cast -c %s -v %s -s %d -l 2 -p %d -e %s %s" % (
        config, outfile, bench_cast_seconds, num_cpus, event_file, app_list)
    runCommand(exec_cmd)
    printColor("Bench cast Ended", "blue")


def getPerfHybridValues(file_path, num_cores, metric='IPC'):
    ret_val = []
    df = pandas.read_csv(file_path,
                         index_col='Event',
                         sep=';',
                         names=['Core', 'N', 'Value', 'Units', 'Event', 'others'])
    # print file_path
    if df.empty:
        printColor("Nothing found at %s" % file_path, "red")
        exit()
    for x in range(num_cores):
        if metric == 'IPC':
            instructions = df['Value']['PAPI_TOT_INS'][x]
            cycles = df['Value']['PAPI_TOT_CYC'][x]
            if cycles > 0:
                ret_val.append(float(instructions)/float(cycles))
            else:
                ret_val.append(0.0)
        else:
            ret_val.append(float(df['Value'][metric][x]))
    # print(ret_val)
    return ret_val


def getPerfSingleValues(file_path, metric='IPC'):
    ret_val = []
    df = pandas.read_csv(file_path,
                         index_col='Event',
                         sep=';',
                         names=['Core', 'N', 'Value', 'Units', 'Event', 'others'])
    # print file_path
    if df.empty:
        printColor("Nothing found at %s" % file_path, "red")
        exit()
    if metric == 'IPC':
        instructions = df['Value']['PAPI_TOT_INS']
        cycles = df['Value']['PAPI_TOT_CYC']
        if cycles > 0:
            ret_val.append(float(instructions)/float(cycles))
        else:
            ret_val.append(0.0)
    else:
        ret_val.append(float(df['Value'][metric]))
    # print(ret_val)
    return ret_val


Fparser = argparse.ArgumentParser(description='')
Fparser.add_argument('--outdir', type=str, default='fairness', action="store",
                     help="Name of the output directory to store...")
Fparser.add_argument('--filename', type=str, action="store",
                     help='Name of the output file csv')
Fparser.add_argument('--singlefile', type=str, action="store",
                     help='Name of the input: single VALUE csv file')
Fparser.add_argument('--nosingle', action='store_true',
                     help='Launch single apps to get the reference')
Fparser.add_argument('--num_procs', type=int, action="store",
                     help='Number of CPUs used in the benchcast experiment')
Fparser.add_argument('--readdir', type=str, action="store",
                     help='Input Directory where csv files are')
Fparser.add_argument('--event', type=str, action="store", default='PAPI_TOT_INS',
                     help='Name of the performance event to extract (optional, IPC)')
Fparser.add_argument('--seconds', type=int, default=20,
                     help='Number of seconds each tests last')
Fparser.add_argument('--eventfile', type=str, action="store",
                     help='Name of the file with the event list')


Fargs = Fparser.parse_args()

event_name = Fargs.event

if Fargs.num_procs:
    num_cpus = Fargs.num_procs
else:
    num_cpus = multiprocessing.cpu_count()
    printColor(
        "WARNING: number of processors not specified (--num_procs). Using %d" % num_cpus, "blue")

if not Fargs.filename:
    printColor("Error, you should provide an output file name (--filename)", "red")
    exit()

seed(os.getpid())

original_dir = os.getcwd()
output = os.path.join(original_dir, Fargs.outdir)
if not os.path.exists(output):
    os.makedirs(output)
checkDir(output)
input = os.path.join(original_dir, Fargs.readdir)
checkDir(input)

BenchCastSingleValue = pandas.DataFrame(columns=['App', event_name])
if Fargs.nosingle:
    for bench in exp_list:
        current_bench = exp_list[bench]
        print(bench)
        for app in current_bench:
            current_app = current_bench[app]
            print(app)
            for cmd in current_app:
                app_name = bench+"."+app+"."+cmd
                app_list = "--%s %s.%s" % (bench, app, cmd)
                filename = "papi-1p-%ssec-%s.csv" % (
                    str(Fargs.seconds), app_name)
                outputfile = os.path.join(output, filename)
                launchBenchCast(1, app_list, outputfile, config='spec-cast',
                                bench_cast_seconds=Fargs.seconds, event_file=Fargs.eventfile)
                value = getPerfSingleValues(outputfile, event_name)
                BenchCastSingleValue = BenchCastSingleValue.append(
                    {'App': app_name, event_name: value[0]}, ignore_index=True)
    BenchCastSingleValue.to_csv(Fargs.singlefile)
else:
    BenchCastSingleValue = pandas.read_csv(Fargs.singlefile, index_col=0)

print(BenchCastSingleValue)

BenchCastDF = pandas.DataFrame(columns=['Benchmark', event_name, 'Avg_'+event_name,
                               'HarmonicWSpeedUp'+event_name, 'DevSpeedUp'+event_name,
                                        'MaxSpeedUp'+event_name, 'MinSpeedUp'+event_name])
FilenamesList = glob.glob(input+'/papi-'+str(num_cpus)+'p-*.csv')

if not FilenamesList:
    printColor("No files found in %s mathing parameters: num cpus=%d" %
               (input, num_cpus), "red")
    exit()

for infile in FilenamesList:
    # Convert named tuple to dictionary
    file_tok = re.search('papi-'+str(num_cpus)+'p-' +
                         str(Fargs.seconds)+'sec-(.+)-[0-9]+.csv', infile)
    bench = 'unknown'
    if file_tok:
        bench = file_tok.group(1)
    values = getPerfHybridValues(infile, num_cpus, event_name)
    avgValue = np.mean(values)
    bench_split = bench.split('-')
    i = 0
    SpeedUp = np.zeros(num_cpus, dtype=float)
    MinSpeedUp = 1000000000
    MaxSpeedUp = 0
    HWSpeedUp = 0.0
    DevSpeedUp = 0.0
    for i in range(num_cpus):
        j = i % len(bench_split)
        singleValue = BenchCastSingleValue.loc[BenchCastSingleValue['App']
                                               == bench_split[j], event_name].values[0]
        SpeedUp[i] = values[i]/singleValue
        if SpeedUp[i] < MinSpeedUp:
            MinSpeedUp = SpeedUp[i]
        if SpeedUp[i] > MaxSpeedUp:
            MaxSpeedUp = SpeedUp[i]
    print(bench_split)
    print(values)
    print(SpeedUp)
    HWSpeedUp = stats.hmean(SpeedUp)
    DevSpeedUp = np.std(SpeedUp)
    BenchCastDF.loc[len(BenchCastDF)] = [bench, values,
                                         avgValue, HWSpeedUp, DevSpeedUp, MaxSpeedUp, MinSpeedUp]

outfile = os.path.join(output, Fargs.filename)
BenchCastDF.to_csv(outfile)
