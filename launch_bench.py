#!/usr/bin/python3
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
        print(bcolors.OKGREEN + string_to_print + bcolors.ENDC)
    elif color== "red":
        print(bcolors.FAIL + string_to_print + bcolors.ENDC)
    else:
        print(bcolors.FAIL + "Bad color argument to print" + bcolors.ENDC)

def runCommand(command, inputVal=''):
    """Run a shell command"""
    if (inputVal == ''):
       print(command)
    ret_code=subprocess.call(command, shell=True)
    if ret_code:
        printColor(("bad return code for command execution: %s" % ret_code), "red")
        exit()
    return ret_code


""" Script main function """
parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('--cmd', type=str, default="1",
                    help='Benchmark number/letter... (is A, bwaves 2)')
parser.add_argument('--app', type=str, action="store",
                    help="Name of the benchmark app to launch (is, ft, bwaves, mcf...)")
parser.add_argument('--conf', type=str, action="store", 
                    help='Name of the configuration file (only with SPEC2017)')
parser.add_argument('--bench', type=str, action="store",
                    help="Name of the benchmark the app belongs to (SPEC, PARSEC, NPB...)")


args = parser.parse_args()
config_name="gem5"
if args.conf:
    config_name=args.conf

print("using config file %s" % config_name)
original_dir=os.getcwd()

if args.app is None:
    printColor("ERROR --app needed and not included", "red")
    parser.print_help(sys.stderr)
    exit()
if args.cmd is None:
    printColor("ERRPR --cmd needed and not included", "red")
    parser.print_help(sys.stderr)
    exit()
app = create_bench(bench=args.bench, app=args.app, conf=args.conf)

if (args.bench == "PARSEC"):
    print("changing LD_LIBRARY_PATH... TODO")
    ld_path = os.path.join(os.getcwd(), app.PATH+'/pkgs/libs/hooks/inst/amd64-linux.gcc-serial/lib')
    if os.environ.get('LD_LIBRARY_PATH') is not None:
        os.environ['LD_LIBRARY_PATH'] += ':'+os.path.normpath(ld_path)
    else:
        os.environ['LD_LIBRARY_PATH'] = ':'+os.path.normpath(ld_path)

exec_dir=app.getExecPath()
print(exec_dir)
checkDir(exec_dir)
exec_cmd=app.getExecCmd(args.cmd)
print("Entering %s" % exec_dir)
os.chdir(exec_dir)
print("Executing %s" % exec_cmd)
runCommand(exec_cmd)
os.chdir(original_dir)

printColor("End", "green")
