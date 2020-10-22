#!/usr/bin/python
#

import argparse
import os
import re
import glob
import multiprocessing
from random import seed
from random import randint
import pandas
from scipy import stats
import numpy as np
from ast import literal_eval

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

def printColor(string_to_print,color):
    """Prints messages to stdout of a defined color"""
    if color== "green":
        print(bcolors.OKGREEN + string_to_print + bcolors.ENDC)
    elif color== "red":
        print(bcolors.FAIL + string_to_print + bcolors.ENDC)
    elif color== "blue":
        print(bcolors.BLUE + string_to_print + bcolors.ENDC)
    else:
        print(bcolors.FAIL + "Bad color argument to print" + bcolors.ENDC)

def getPerfHybridValues(file_path, num_cores, metric='IPC'):
    ret_val=[]
    df = pandas.read_csv(file_path, 
            index_col='Event', 
            sep = ';',
            names=['Core','N','Value', 'Units','Event', 'others'])
    #print file_path
    if df.empty:
        printColor("Nothing found at %s"% file_path, "red")
        exit()
    for x in range(num_cores):
        if metric == 'IPC':
            instructions=df['Value']['PAPI_TOT_INS'][x]
            cycles=df['Value']['PAPI_TOT_CYC'][x]
            if cycles > 0:
                ret_val.append(float(instructions)/float(cycles))
            else:
                ret_val.append(0.0)
        else:
            ret_val.append(float(df['Value'][metric][x]))
    #print(ret_val)
    return ret_val

parser = argparse.ArgumentParser(description='')
parser.add_argument('--outdir', type=str, default='exp-value',action="store",
                    help="Name of the output directory to store...")
parser.add_argument('--filename', type=str,action="store", 
                    help='Name of the output file csv')
parser.add_argument('--singlefile', type=str, action="store", 
                    help='Name of the input of single IPC file csv')
parser.add_argument('--numcpus', type=int, action="store",
                    help='Number of CPUs used in the experiment')
parser.add_argument('--readdir', type=str, action="store",
                    help='Input Directory where csv are')          
parser.add_argument('--event', type=str, action="store", default='PAPI_TOT_INS',
                    help='Name of the performance event to extract')          
         

args = parser.parse_args()

event_name=args.event

if args.numcpus:
    num_cpus=args.numcpus
else:
    num_cpus = multiprocessing.cpu_count()
    printColor("WARNING: number of processors not specified (--numcpus). Using %d" %num_cpus, "blue")

if not args.filename:
    printColor("Error, you should provide an output file name (--filename)", "red")
    exit()

seed(os.getpid())

original_dir=os.getcwd()
output=os.path.join(original_dir,args.outdir)
if not os.path.exists(output):
    os.makedirs(output)
checkDir(output)
input=os.path.join(original_dir,args.readdir)
checkDir(input)

BenchCastDF = pandas.DataFrame(columns=['Benchmark',event_name, 'SumTotal_'+event_name, 'Avg_'+event_name])
FilenamesList = glob.glob(input+'/papi-'+str(num_cpus)+'p-*.csv')

if not FilenamesList:
    printColor("No files found in %s mathing parameters: num cpus=%d"% (input, num_cpus), "red")
    exit()

for infile in FilenamesList:
    # Convert named tuple to dictionary
    file_tok=re.search('papi-'+str(num_cpus)+'p-*sec-(.+)-*.csv', infile)
    bench='unknown'
    if file_tok:
        bench = file_tok.group(1)
    values=getPerfHybridValues(infile, num_cpus, event_name)
    avgValue=np.mean(values)
    sumValue=np.sum(values)
    BenchCastDF.loc[len(BenchCastDF)]=[bench,values,sumValue,avgValue]
    
outfile=os.path.join(output,args.filename)
BenchCastDF.to_csv(outfile)
