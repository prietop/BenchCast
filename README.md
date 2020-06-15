# SPECcast
SPECcast is a performance evaluation tool that uses profiling ([PAPI](https://icl.utk.edu/papi/)) and synchronization mechanisms to overcome limitations of current alternatives. SPECcast is able to generate huge amount of multiprogrammed workloads from SPEC, executing their Region Of Interest (ROI) simultaneously. Therefore it can provide high number of measures in a short amount of time, and thanks to PAPI, it is not limited to execution time or IPC. SPECcast is intended to be used on real hardware and/or Full System simulations tools (like [gem5](https://www.gem5.org/)). 

## Announcements
* SPECcast is presented at [ICPP'20](https://doi.org/10.1145/340397.3404424)
  

## SPEC CPU 2017 Integration
SPECcast is intended to be used with [SPEC CPU® 2017](https://www.spec.org/cpu2017/) and you need a legal copy to use it.
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
cd <SPECcast_folder>
cp SPEC17.patch <SPEC2017_folder>
cd <SPEC2017_folder>
git apply SPEC17.patch
git commit -am "SPECcast patch applied"
```

## Compilation
To compile this repository, execute:
```bash
make
```
Inside the SPECcast folder. 
If you want to use it with gem5, you need to copy the following files from the gem5 repository inside the SPECcast folder:
> m5ops.h
> 
> m5_mmap.c
>
> m5_mmap.h
> 
> m5op_addr.S

You will need to modify this last file (m5op_addr.S), so the include of m5ops.h target the one inside SPECcast folder:
```C
#include "m5ops.h"
```
When this README was written, these files were under gem5/include/gem5/asm/generic/, gem5/util/m5/src and gem5/util/m5/src/x86 folders.

Then execute:
```bash
make GEM5=true
```
Once compiled, copy the spec_cast executable file to the SPEC2017 folder
```bash
cp spec_cast <SPEC2017_folder>
```

Then, proceed to compile the SPEC CPU 2017.
First of all, you need to create a configuration file as explained in the [SPEC CPU 2017 documentation](https://www.spec.org/cpu2017/Docs/quick-start.html).
Then, to use SPECcast, you should enable the corresponding lines in the Makefile.defaults of SPEC 2017
> edit <SPEC2017_folder>/benchspec/Makefile.defaults

There you can find the lines to compile the SPEC 2017 using SPECcast and/or GEM5. Uncomment the corresponding lines of the Makefile (lines 85 and 283)
Compile the SPEC CPU 2017 using:
```bash
runcpu --config=<your_config_file> --action=run_setup specrate
```

## Usage
Once compiled and with the spec_cast executable inside the SPEC2017 folder, you can begin using SPECcast:
### Example
```bash
cd <SPEC2017_folder>
./spec_cast -c spec-cast -d -p 8 -n 2 -l 2 --mcf --bwaves 2
```
### Options

  Usage: spec_cast [-b] [-w] [-p number] [-n number] [-l number] "
        "[-c config_name] [-v csv_filename] [-s seconds] [-r number]"
        "--\<prog\> [number] [--\<prog2\> [number] --\<prog3\> [number]...]
  * **prog** is the program/s you want to cast. Some programs require an aditional param to indicate the benchmark number among all available for that prog (check LAUNCH_CMKS.py to see the benchmarks available for each prog).
  * **-b** makes spec_cast to return immediately after launching the programs. The default is to wait for them to finish (Not recommended).
  * **-p** is the number of processors you want to use from the system (default value: the number of available processors in the system.
  * **-w** should be used with GEM5, and executes a m5_work_begin_op at the start of the ROI.
  * **-s \<N\>** use papi library to analyze performance counters during N seconds.
  * **-c \<file\>** SPEC config name without the .cfg extension (Use the same used when you compile SPEC CPU 2017). This parameter is needed to obtain the path to execute the SPECs.
  * **-v \<file\>** print papi results in a csv file (ASUME USING PAPI).
  * **-r \<N\>** repeat papi measures N times (ASUME USING PAPI, and csv output).
  * **-n \<N\>** Number of different programs to run. It defaults to the number of progs arguments passed. Should be used as a control.
  * **-l \<N\>** Number of times the main loop of the ROI should be executed before the creation of the checkpoint (end of spec_cast).
  
  Example:
  ./spec_cast -c spec-cast -p 8 -l 2 --mcf --bwaves 2
  This will run 4 instances of **mcf** and 4 instances of **bwaves** (with input bwaves_2) and synchronize them after 2 loops of the ROI. The configuration file employed is spec_cast.cfg.
  
## References 
If you use this tool in your research, please cite the following paper:
*P. Prieto, P. Abad, J.A. Herrero, J.A. Gregorio, V. Puente, "SPECcast: A Methodology for Fast Performance Evaluation with SPEC CPU 2017 Multiprogrammed Workloads" in ICPP 2020 - 49th International Conference on Parallel Processing, 2020*

## Disclaimer
SPECcast is currently under development, so future improvements and modifications are spected. If you have any problems, please send an e-mail to prietop@unican.es.
At the current version, SPECcast has been tested on Linux and x86-64 systems.