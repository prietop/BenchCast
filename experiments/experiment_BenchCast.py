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

def runCommand(command):
    """Run a shell command"""
    print(command)
    ret_code=subprocess.call(command, shell=True)
    if ret_code:
        printColor(("bad return code for command execution %s: %s" % (command,ret_code)), "red")
        exit()
    return ret_code

def normalityTest(DataSet, alpha, distr='norm'):
    #Perform Kolmogorov-Smirnov test for normality
    stat = 0.0
    p = 0.0
    if distr == 'lognorm':
        stat, p = stats.kstest(DataSet, distr,stats.lognorm.fit(DataSet))
    else:
        stat, p = stats.kstest(DataSet, distr)
    print('Statistic=%.3f, p=%.18f' % (stat, p))
    if p > alpha:
        printColor('Sample looks %s' % distr,'green')
    else:
        printColor('Sample does not look %s' % distr,'red')
    return p

def launchBenchCast(num_cpus,app_list,outfile,config,spec_cast_seconds, rdt=False):
    """ Launch BENCHCAST """
    printColor("Launching bench_cast with apps: %s" % app_list, "blue")
    rd_mask="sudo rdtset "
    if rdt:
        rd_mask=rd_mask+"-r 0-%d -t 'l3=0x01;cpu=0,8,16,24,32,40,48,56' -t 'l3=0x02;cpu=1,9,17,25,33,41,49,57' -t 'l3=0x04;cpu=2,10,18,26,34,42,50,58' -t 'l3=0x08;cpu=3,11,19,27,35,43,51,59' -t 'l3=0x10;cpu=4,12,20,28,36,44,52,60' -t 'l3=0x20;cpu=5,13,21,29,37,45,53,61' -t 'l3=0x40;cpu=6,14,22,30,38,46,54,62' -t 'l3=0x80;cpu=7,15,23,31,39,47,55,63'"%(num_cpus-1)
        rd_mask=rd_mask+" -c 0-%d "%(num_cpus-1)
    else:
        #rd_mask=rd_mask+"-t 'l3=0xff;cpu=0-%d' -c 0-%d "%(num_cpus-1, num_cpus-1)
        rd_mask=""
    exec_cmd="%s./bench_cast -c %s -v %s -s %d -l 2 -p %d %s" % (rd_mask, config, outfile, spec_cast_seconds,num_cpus, app_list)
    runCommand(exec_cmd)
    printColor("Spec cast Ended", "blue")

def getBenchCastSingleValue(file_path, metric='IPC'):
    ret_val=[]
    df = pandas.read_csv(file_path, 
            index_col='Event', 
            sep = ';',
            names=['Core','N','Value', 'Units','Event', 'others'])
    #print(df)
    if metric == 'IPC':
        instructions=df['Value']['PAPI_TOT_INS']
        cycles=df['Value']['PAPI_TOT_CYC']
        ret_val.append(float(instructions)/float(cycles))
    else:
        ret_val.append(float(df['Value'][metric]))
    #print(ret_val)
    return ret_val

def getPerfHybridValues(file_path, num_cores, metric='IPC'):
    ret_val=[]
    df = pandas.read_csv(file_path, 
            index_col='Event', 
            sep = ';',
            names=['Core','N','Value', 'Units','Event', 'others'])
    #print(df)
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
parser.add_argument('--batch', type=int, default=1,
                    help='Number of test to execute each batch')
parser.add_argument('--mintests', type=int, default=1,
                    help='minimum Number of test to execute')
parser.add_argument('--alpha', type=float, default=0.5,
                    help='Confidence for normality test')
parser.add_argument('--seconds', type=int, default=60,
                    help='Number of seconds each tests last')
parser.add_argument('--outdir', type=str, default='exp1-python',action="store",
                    help="Name of the output directory to store...")
parser.add_argument('--filename', type=str, default='data.csv',action="store", 
                    help='Name of the output file csv')
parser.add_argument('--infile', type=str, action="store", 
                    help='Name of the input file csv')
parser.add_argument('--nosingle', action='store_true', 
                    help='Launch single apps to get the reference')
parser.add_argument('--userdt', action='store_true', 
                    help='Use resource director')
parser.add_argument('--max_apps', type=int, default=8,
                    help='Maximum number of apps in a mix')
parser.add_argument('--num_procs', type=int, 
                    help='Number of processors')

args = parser.parse_args()

output_dir=args.outdir
batch_size=args.batch
alpha=args.alpha
spec_cast_seconds=args.seconds
max_apps = args.max_apps

num_cpus = multiprocessing.cpu_count()
if args.num_procs:
    num_cpus=args.num_procs

seed(os.getpid())

original_dir=os.getcwd()
output=os.path.join(original_dir,output_dir)
events_file_name=os.path.join(original_dir,"BenchCast_PAPI_events.cfg")
if not os.path.exists(output):
    os.makedirs(output)
checkDir(output)
events_file=open(events_file_name, 'r')
events=events_file.readlines() 
event_list=[]
for line in events:
    event_list.append(line.strip())
column_list=['Bench','App','Cmd','IPC']
column_list.extend(event_list) 
BENCHCASTsingleIPC = pandas.DataFrame(columns=column_list)
singlefile=os.path.join(output,"BENCHCAST-single-IPC.csv")

for x in exp_list:
    #print(x)
    for y in exp_list[x]:
        #print(y)
        for z in exp_list[x][y]:
            #print(z)
            filename="papi-"+x+"-"+y+"-"+z+".csv"
            outputfile=os.path.join(output,filename)
            if not args.nosingle:
                app_list=" --"+x+" "+y+"."+z
                launchBenchCast(1,app_list,outputfile,config='spec-cast',spec_cast_seconds=spec_cast_seconds, rdt=args.userdt)
            ipc = getBenchCastSingleValue(outputfile, 'IPC')
            BENCHCASTsingleIPC = BENCHCASTsingleIPC.append({'Bench': x, 'App': y, 'Cmd': z, 'IPC': ipc[0]}, ignore_index=True)
            for event in event_list:
                value=getBenchCastSingleValue(outputfile, event)
                BENCHCASTsingleIPC.loc[BENCHCASTsingleIPC.index[-1], event]=value[0]
BENCHCASTsingleIPC.to_csv(singlefile)

count=0
average_events=[]
total_events=[]
for event in event_list:
    average_events.append('Avg_'+event)
    total_events.append('Total_'+event)

if args.infile:
    print("reading dataframe from %s" % args.infile)
    BENCHCASTSpeedUpDF = pandas.read_csv(args.infile,index_col=0)
    count=len(BENCHCASTSpeedUpDF.index)
else:
    column_list=['Benchmark','IPCs','AvgIPC','AvgSpeedUp','GeoSpeedUp','HmeanSpeedUp', 'MaxSpeedUp', 'MinSpeedUp', 'MaxSUApp', 'MinSUApp']
    column_list.extend(event_list)
    column_list.extend(average_events)
    column_list.extend(total_events)
    BENCHCASTSpeedUpDF=pandas.DataFrame(columns=column_list)

count=0

while count < args.mintests:
    for i in range(batch_size):
        bench=[]
        app=[]
        cmd=[]
        procs=[]
        app_list=""
        batch_name=""
        sort_name=""
        j=0
        for n in range(num_cpus):
            if n < max_apps:
                bench_num = randint(0, len(exp_list)-1)
                bench.append(list(exp_list)[bench_num])
                current_bench=list(exp_list.values())[bench_num]
                app_num = randint(0, len(current_bench)-1)
                app.append(list(current_bench)[app_num])
                current_app = list(current_bench.values())[app_num]
                cmd_num = randint(0,len(current_app)-1)
                cmd.append(list(current_app)[cmd_num])
                print("--%s %s.%s" % (bench[n],app[n],cmd[n]))
                j = n
            else:
                j = n%max_apps
            batch_name="%s-%s.%s" %(batch_name, app[j], cmd[j])
            if n < max_apps:
                sort_name="%s-%s.%s" %(sort_name, app[j], cmd[j])
            app_list="%s --%s %s.%s" % (app_list,bench[j],app[j],cmd[j])
        filename="papi-%dp-%dsec%s-%d.csv" %(num_cpus,spec_cast_seconds,sort_name,count)
        outputfile=os.path.join(output,filename)
        #open(outputfile, 'a').close()
        launchBenchCast(num_cpus,app_list,outputfile,config='spec-cast',spec_cast_seconds=spec_cast_seconds, rdt=args.userdt)
        IPCs=getPerfHybridValues(outputfile, num_cpus, 'IPC')
        if 0.0 in IPCs:
            continue
        MaxSpeedUp=0.0
        MinSpeedUp=1000.0
        MaxSUApp=''
        MinSUApp=''
        SpeedUps=[]
        for j in range(num_cpus):
            app_index = j%max_apps
            single_ipc=BENCHCASTsingleIPC.loc[BENCHCASTsingleIPC['App']==app[app_index],'IPC'].values[0]
            speedup=IPCs[j]/single_ipc
            SpeedUps.append(speedup)
            if speedup > MaxSpeedUp:
                MaxSpeedUp=speedup
                MaxSUApp=app[app_index]
            if speedup < MinSpeedUp:
                MinSpeedUp=speedup
                MinSUApp=app[app_index]
        AvgIPC=np.mean(IPCs)
        AvgSpeedUp=np.mean(SpeedUps)
        GeoSpeedUp=stats.mstats.gmean(SpeedUps)
        HmeanSpeedUp=stats.mstats.hmean(SpeedUps)
        new_row={'Benchmark': batch_name, 'IPCs': IPCs,'AvgIPC': AvgIPC,'AvgSpeedUp': AvgSpeedUp,'GeoSpeedUp': GeoSpeedUp,
                'HmeanSpeedUp': HmeanSpeedUp, 'MaxSpeedUp': MaxSpeedUp, 'MinSpeedUp': MinSpeedUp, 'MaxSUApp': MaxSUApp, 'MinSUApp': MinSUApp}
        BENCHCASTSpeedUpDF = BENCHCASTSpeedUpDF.append(pandas.Series(new_row), ignore_index=True)
        for event in event_list:
            value=getPerfHybridValues(outputfile, num_cpus, event)
            BENCHCASTSpeedUpDF.loc[BENCHCASTSpeedUpDF.index[-1], event]='['+','.join(map(str, value))+']'
            BENCHCASTSpeedUpDF.loc[BENCHCASTSpeedUpDF.index[-1], 'Avg_'+event]=np.mean(value)
            BENCHCASTSpeedUpDF.loc[BENCHCASTSpeedUpDF.index[-1], 'Total_'+event]=np.sum(value)
        count=count+1
    """ end batch loop """
    BENCHCASTSpeedUpDF.to_csv(args.filename)
    
printColor("THE END!", "green")
