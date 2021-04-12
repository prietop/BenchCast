# BenchCast
BenchCast is a performance evaluation tool that uses profiling ([PAPI](https://icl.utk.edu/papi/)), application sampling and synchronization mechanisms to overcome limitations of current evaluation alternatives. BenchCast is able to generate a huge amount of multiprogrammed workloads from different benchmark suites, executing their Region Of Interest (ROI) simultaneously. Therefore it can provide high number of measures in a short amount of time, and thanks to PAPI, it is not limited to execution time or IPC. BenchCast is intended to be used on real hardware and/or Full System simulations tools (like [gem5](https://www.gem5.org/)). At the moment, BenchCast comes prepared to run SPEC, PARSEC and NPB benchmarks.

## Announcements
* SPECcast paper at ICPP'20 [Paper](https://doi.org/10.1145/3404397.3404424)
* SPECcast video presentation [Video](https://www.youtube.com/watch?v=h5hMzUvMAWY)
  

## SPEC CPU 2017 Integration
BenchCast can be used alongside [SPEC CPU® 2017](https://www.spec.org/cpu2017/). If you want to use it with SPEC2017, you need a legal copy.
Download and [install](https://www.spec.org/cpu2017/Docs/quick-start.html) your SPEC CPU 2017 (do not need to compile them yet).
Create a git repository within your SPEC2017 folder:
```bash
cd <SPEC2017_folder>
git init
git add .
git commit -am "SPEC CPU 2017 Initial commit"
```
Copy the patch included in this repository inside the SPEC2017 folder and apply it:
```bash
cd <BenchCast_folder>
cp SPEC17.patch <SPEC2017_folder>
cd <SPEC2017_folder>
git apply SPEC17.patch
git commit -am "BenchCast patch applied"
```
Now, proceed to compile the SPEC CPU 2017.
First of all, you need to create a configuration file as explained in the [SPEC CPU 2017 documentation](https://www.spec.org/cpu2017/Docs/quick-start.html).
Then, to use BenchCast, you should enable the corresponding lines in the Makefile.defaults of SPEC 2017
> edit <SPEC2017_folder>/benchspec/Makefile.defaults

There you can find the lines to compile the SPEC 2017 using BenchCast and/or GEM5. Uncomment the corresponding lines of the Makefile (lines 85 and 283)
Compile the SPEC CPU 2017 using:
```bash
runcpu --config=<your_config_file> --action=run_setup specrate
```

## PARSEC 3.0 Integration
BenchCast can be used alongside [PARSEC 3.0](https://parsec.cs.princeton.edu/). If you want to use it with PARSEC, you need a legal copy.
Download and [install](https://parsec.cs.princeton.edu/parsec3-doc.htm) your PARSEC (do not need to compile them yet).
Create a git repository within your PARSEC folder:
```bash
cd <PARSEC_folder>
git init
git add .
git commit -am "PARSEC 3.0 Initial commit"
```
Copy the patch included in this repository inside the PARSEC folder and apply it:
```bash
cd <BenchCast_folder>
cp PARSEC3_0.patch <PARSEC_folder>
cd <PARSEC_folder>
git apply PARSEC3_0.patch
git commit -am "BenchCast patch applied"
```
Then proceed to compile your PARSEC Benchmark suite.

## NPB 3.3.1 SERIAL (NAS Parallel Benchmarks) Integration
BenchCast can be used alongside [NPB 3.3.1](https://www.nas.nasa.gov/publications/npb.html). If you want to use it with NPB, you need a legal copy.
Download and [install](https://www.nas.nasa.gov/assets/npb/NPB3.3.1.tar.gz) your NPB (do not need to compile them yet).
Create a git repository within your NPB folder:
```bash
cd <NPB_folder>
git init
git add .
git commit -am "NPB 3.3.1 Initial commit"
```
Copy the patch included in this repository inside the PARSEC folder and apply it:
```bash
cd <BenchCast_folder>
cp NPB_PAPI_BENCHCAST.patch <NPB_folder>
cd <NPB_folder>
git apply NPB_PAPI_BENCHCAST.patch
git commit -am "BenchCast patch applied"
```
Then proceed to compile your NPB Benchmark suite. BenchCast is intended to be used with the serial implementation (SER) of NPB. Use NPB3.3-SER/config/NAS.samples/make.def_gcc_x86_bench_cast, and uncomment the "CKPT = -DBENCHCAST" line in order to compile NPB-SER to be used alongside BenchCast.

## Compilation
To compile this repository (BenchCast), execute:
```bash
make
```
Inside the BenchCast folder. 
If you want to use it with gem5, you need to copy the following files from the gem5 repository inside the BenchCast folder:
> m5ops.h
> 
> m5_mmap.c
>
> m5_mmap.h
> 
> m5op_addr.S

You will need to modify this last file (m5op_addr.S), so the include of m5ops.h target the one inside BenchCast folder:
```C
#include "m5ops.h"
```
When this README was written, these files were under gem5/include/gem5/asm/generic/, gem5/util/m5/src and gem5/util/m5/src/x86 folders.

Then execute:
```bash
make GEM5=true
```

## Usage
Once all your Suites and BenchCast are compiled, you can begin using BenchCast:
There are some configuration files you may need to edit, in order to execute BenchCast: 
* *BENCH_CONF.py* has the paths to the directories of the benchmark suites.

### System evaluation
BenchCast includes a python script for System evaluation, that automatically generates random workloads form the available suites and stores the results in the apropiate files.
```bash
./experiments/topdown.py --max_apps=12 --outdir=results/prueba --filename=results/prueba.csv --seconds=20 --numtests=1000 --eventfile=TOPDOWN_events.cfg
```
This command will launch 1000 different workloads of 12 random applications from the available suites, running each workload for 20 seconds. For each workload, PAPI will measure the events described in *TOPDOWN_events.cfg* file, and write the PAPI results in a file inside folder *results/prueba*. Aditionally, the file *results/prueba.csv* includes the results obtained for each workload, including sum and average of each PAPI_event.
There are some configuration files you may want to edit:
* *experiment_list.py* has a list of all the available benchmarks to choose from, used for the random workload generation. You should add any additional benchmark you want to use to this file. Additionally, you may want to remove a benchmark suite (edit *exp_list* variable, commenting the corresponding line) or some of the applications of a given suite (there are some examples in the file).

#### Options

* **--numtests**: Number of workloads to execute for the system evaluation.
* **--seconds**: Number of seconds each tests executes during the measurement.
* **--outdir**: Name of the output directory where PAPI output files are stored (in a perf-like csv format).
* **--filename**: Name of the output file csv with the global dataframe information.
* **--infile**: Option to resume a previous experiment. Needs the name of the csv file from the previous experiment to resume.
* **--max_apps**: Option to limit the number of different applications used in each workload.
* **--num_procs**: Number of processors to be used for the experiment (should be less than or equal to the number available hardware contexts in the system under test).
* **--eventfile**: Name of the file with the event list for PAPI to measure.
* **--reffile**: Used to launch the same workloads as a previous experiment. The **numtests** option will be ignored.


### Single Workload execution
BenchCast can be used separately to launch a single workload.

```bash
./bench_cast -c spec-cast -p 8 -l 2 -s 10 -v papi_results.csv --SPEC mcf.1 --SPEC bwaves.2 --NPB ft.C --PARSEC swaptions.1
```
This command will launch 2 copies of mcf from SPEC2017, 2 copies of bwaves from SPEC2017 (using the second command line), 2 copies of FT from NPB (using the C class) and 2 copies of swaptions from PARSEC 3.0.
In total, the first 8 cores of the machine will be used, and applications will be attached to them. Applications will be synchronized in the second interation of their ROI loop. PAPI will be used to take measures for 10 seconds, and results will be written in "papi_results.csv" file. The configuration used for SPEC2017 applications is "spec_cast.cfg".

#### Options

  Usage: bench_cast [-b] [-w] [-p number] [-n number] [-l number] "
        "[-c config_name] [-v csv_filename] [-s seconds] [-r number]"
        "--\<prog\> [number] [--\<prog2\> [number] --\<prog3\> [number]...]
  * **prog** is the program/s you want to cast. Some programs require an aditional param to indicate the benchmark number among all available for that prog (check LAUNCH_CMKS.py to see the benchmarks available for each prog).
  * **-b** makes BenchCast to return immediately after launching the programs. The default is to wait for them to finish (not recommended). Should not be used along with PAPI measurements.
  * **-p** is the number of processors you want to use from the system (default value: the number of available processors in the system.
  * **-w** should be used with GEM5, and executes a m5_work_begin_op at the start of the ROI.
  * **-s \<N\>** use papi library to analyze performance counters during N seconds.
  * **-c \<file\>** SPEC config name without the .cfg extension (Use the same used when you compile SPEC CPU 2017). This parameter is needed to obtain the path to execute the SPECs.
  * **-v \<file\>** print papi results in a csv file (ASUME USING PAPI).
  * **-r \<N\>** repeat papi measures N times (ASUME USING PAPI, and csv output).
  * **-n \<N\>** Number of different programs to run. It defaults to the number of progs arguments passed. Should be used as a control.
  * **-l \<N\>** Number of times the main loop of the ROI should be executed before synchronized execution and the beginning of measurements.
  * **-m** Measure the behavior of the N first cores. BenchCast reserves those cores so no benchmark is launched there (PAPI is assumed, but not needed).

## Adding a new Benchmark
First, you should find the ROI of your application (**TO DO**).
Once the ROI loop is detected, you should add a call to *initialize_barrier* before the loop, and a call to *call_barrier* inside the loop. 
Both functions are defined in barrier_cast.c and barrier_cast.h, included in this repository. You should include them in the benchmark source code and add the files to compile the benchmark with them. You may need to add **-pthread** and **-lrt** flags if not already included in the linking process of your benchmark.
Once compiled with barrier_cast, you need to add the benchmark to the BenchCast pool.

There are some files you can check to see how to add a new benchmark on your own:
  * **BENCH_CONF.py** includes the paths to the benchmarks used with BenchCast. You should add your new benchmar to this file.
  * **bench_types.py** has all needed to launch a benchmark, including possible subdirectories and command lines. Check at how existing benchmark are defined (at least getExecPath and getExecCmd should be defined). You need to add your benchmark to the **create_bench** function (the "name" of the benchmark will be used later). Also, a CMDS specific module may be needed if there are multiple benchmarks in a single suite that have a similar structure (see below).
  * **SPEC_CMDS.py**, **PARSEC_CMDS.py** and **NPB_CMDS.py** have the commands needed to execute the different benchmarks on each suite.
  * **bench_cast.h** You need to add your benchmark to the BenchCast long options (bench_cast.h line:105). Use the same name used in **create_bench** of **bench_types.py**.
  * **experiment_list.py** Finally, you may want to add your benchmark to this file in order to use it with the evaluation script *evaluate.py*.
  
## References 
If you use this tool in your research, please cite the following paper:
*P. Prieto, P. Abad, J.A. Herrero, J.A. Gregorio, V. Puente, "SPECcast: A Methodology for Fast Performance Evaluation with SPEC CPU 2017 Multiprogrammed Workloads" in ICPP 2020 - 49th International Conference on Parallel Processing, 2020*

## Disclaimer
BenchCast is currently under development, so future improvements and modifications are spected. If you have any problems, please send an e-mail to prietop@unican.es.
At the current version, BenchCast has been tested on Linux and x86-64 systems.
