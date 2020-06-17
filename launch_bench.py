#!/usr/bin/python
#

from bench_types import *
import argparse
import sys
import subprocess
import os

class bcolors:
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def checkDir(dir_to_check):
    """Checks the existance of a directory"""
    if not os.path.exists(dir_to_check):
        printColor(("Directory not found: %s" % dir_to_check), "red")
        exit()

def checkFile(dir_to_search, file_to_check):
    """Checks the presence of a file in a directory"""
    check = os.path.join(dir_to_search, file_to_check)
    if not os.path.exists(check):
        printColor(("File not found: %s" % check), "red")
        exit()

def printColor(string_to_print,color):
    """Prints messages to stdout of a defined color"""
    if color== "green":
        print bcolors.OKGREEN + string_to_print + bcolors.ENDC
    elif color== "red":
        print bcolors.FAIL + string_to_print + bcolors.ENDC
    else:
        print bcolors.FAIL + "Bad color argument to print" + bcolors.ENDC

def runCommand(command, inputVal=''):
    """Run a shell command"""
    if (inputVal == ''):
       print command
    ret_code=subprocess.call(command, shell=True)
    if ret_code:
        printColor(("bad return code for command execution: %s" % ret_code), "red")
        exit()
    return ret_code


""" Script main function """
parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('--cmd', type=str, default="1",
                    help='Benchmark number')
parser.add_argument('--app', type=str, action="store",
                    help="Name of the benchmark app to launch")
parser.add_argument('--conf', type=str, action="store", 
                    help='Name of the configuration file (only on spec)')
parser.add_argument('--bench', type=str, action="store",
                    help="Name of the benchmark the app belongs to")


args = parser.parse_args()
config_name="gem5"
if args.conf:
    config_name=args.conf

print "using config file %s" % config_name
original_dir=os.getcwd()

if (args.bench == "SPEC"):
    app = SPEC(args.app, args.conf)
elif (args.bench == "NPB"):
    app = NPB(args.app)
else:
    printColor("Benchmark type %s not included" % args.bench, "red")
    exit()
exec_dir=app.getExecPath()
print exec_dir
checkDir(exec_dir)
exec_cmd=app.getExecCmd(args.cmd)
os.chdir(exec_dir)
runCommand(exec_cmd)
os.chdir(original_dir)

printColor("End", "green")
