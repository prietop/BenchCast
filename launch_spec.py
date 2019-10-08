#!/usr/bin/python
#

from LAUNCH_CMDS import *
import argparse
import sys
import subprocess
import os

class bcolors:
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def getNameApp(app_name):
    if ("gcc" in app_name):
        return "cpu"+app_name.split(".",1)[1]
    elif ("xalan" in app_name):
        return "cpuxalan_r"
    elif ("cactuBSSN" in app_name):
        return "cactusBSSN_r"
    else:
        return app_name.split(".",1)[1]

def getNumberApp(app_name):
    return app_name.split(".",1)[0]

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
parser.add_argument('--cmd', type=int, default=1,
                    help='Benchmark number')
parser.add_argument('--spec', type=str, action="store",
                    help="Name of the spec app to launch")
parser.add_argument('--conf', type=str, action="store", 
                    help='Name of the spec app to launch')


args = parser.parse_args()
config_name="gem5"
if args.conf:
    config_name=args.conf

print "using config file %s" % config_name
original_dir=os.getcwd()
if args.spec:
    app = abreviate_spec_name[args.spec]
else:
    printColor(("Error, spec option needed"), "red")
    exit()
cmd = args.cmd
print "app: %s" % getNameApp(app)
print "command number: %d" % cmd
exec_dir="benchspec/CPU/%s/run/run_base_refrate_%s-m64.0000/" % (app, config_name)
""" exec_dir = os.path.join(os.getcwd(), exec_dir) """
print exec_dir
checkDir(exec_dir)
exec_file="%s_base.%s-m64 " % (getNameApp(app), config_name)
""" checkFile(exec_dir,exec_file) """
os.chdir(exec_dir)
print launch_cmd_rate[app][cmd]
exec_cmd="./"+exec_file+launch_cmd_rate[app][cmd]
runCommand(exec_cmd)
os.chdir(original_dir)

printColor("End", "green")
