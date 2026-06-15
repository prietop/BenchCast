# BenchCast

BenchCast is a performance evaluation tool for multiprogrammed workloads. It combines profiling with PAPI ([PAPI](https://icl.utk.edu/papi/)), application sampling, and synchronized ROI execution so you can evaluate many benchmarks simultaneously. BenchCast is designed for real hardware and full-system simulation environments such as [gem5](https://www.gem5.org/).

BenchCast currently supports SPEC CPU 2017, PARSEC 3.0, and NPB 3.3.1 serial benchmarks.

## Announcements

* SPECcast paper at ICPP'20 [Paper](https://doi.org/10.1145/3404397.3404424)
* SPECcast video presentation [Video](https://www.youtube.com/watch?v=h5hMzUvMAWY)
* BenchCast paper at TPDS [Paper](https://doi.org/10.1109/TPDS.2021.3080702)
  
---

## Quick Start

### 1. Open the BenchCast folder

```bash
cd /path/to/BenchCast
```

### 2. Build BenchCast

```bash
make
```

### 3. Configure suite paths

Edit `BENCH_CONF.py` to point to the installed directories for your benchmark suites.

---

## Suite Integration Guide

### SPEC CPU 2017

1. Install SPEC CPU 2017 and initialize a git repository in the SPEC root:

```bash
cd <SPEC2017_folder>
git init
git add .
git commit -am "SPEC CPU 2017 Initial commit"
```

2. Apply the BenchCast patch:

```bash
cd /path/to/BenchCast
cp SPEC17.patch <SPEC2017_folder>
cd <SPEC2017_folder>
git apply SPEC17.patch
git commit -am "BenchCast patch applied"
```

3. Create the SPEC config file per SPEC documentation.
4. Enable BenchCast/GEM5 lines in `<SPEC2017_folder>/benchspec/Makefile.defaults`.
5. Run SPEC setup:

```bash
runcpu --config=<your_config_file> --action=run_setup specrate
```

### PARSEC 3.0

1. Install [PARSEC 3.0](https://parsec.cs.princeton.edu/) and initialize a git repository in the PARSEC root:

```bash
cd <PARSEC_folder>
git init
git add .
git commit -am "PARSEC 3.0 Initial commit"
```

2. Apply the BenchCast patch:

```bash
cd /path/to/BenchCast
cp PARSEC3_0.patch <PARSEC_folder>
cd <PARSEC_folder>
git apply PARSEC3_0.patch
git commit -am "BenchCast patch applied"
```

3. Compile the PARSEC suite [normally](https://parsec.cs.princeton.edu/parsec3-doc.htm).

### NPB 3.3.1 SERIAL

1. Install [NPB 3.3.1](https://www.nas.nasa.gov/publications/npb.html) and initialize a git repository in its root:

```bash
cd <NPB_folder>
git init
git add .
git commit -am "NPB 3.3.1 Initial commit"
```

2. Apply the BenchCast patch:

```bash
cd /path/to/BenchCast
cp NPB_PAPI_BENCHCAST.patch <NPB_folder>
cd <NPB_folder>
git apply NPB_PAPI_BENCHCAST.patch
git commit -am "BenchCast patch applied"
```

3. Build the serial NPB implementation and enable `CKPT = -DBENCHCAST` in `NPB3.3-SER/config/NAS.samples/make.def_gcc_x86_bench_cast`.

---

## Using BenchCast

### System evaluation

Use `experiments/topdown.py` to generate randomized multiprogram workloads and collect PAPI metrics.

Example:

```bash
./experiments/topdown.py --max_apps=12 --outdir=results/prueba --filename=results/prueba.csv --seconds=20 --numtests=1000 --eventfile=TOPDOWN_events.cfg
```

This runs 1,000 random workloads with up to 12 apps each, measures the PAPI events in `TOPDOWN_events.cfg`, and writes raw outputs to `results/prueba/`.

Helpful files:

* `BENCH_CONF.py` — benchmark suite locations.
* `experiment_list.py` — benchmark pool and workload selection.

#### System evaluation options

* `--numtests` : number of workloads to execute.
* `--seconds` : measurement duration in seconds.
* `--outdir` : output directory for raw PAPI results.
* `--filename` : global CSV summary filename.
* `--infile` : resume from an existing CSV file.
* `--max_apps` : maximum number of different applications per workload.
* `--num_procs` : number of processors to use.
* `--eventfile` : PAPI event file.
* `--reffile` : replay a previous workload set (ignores `--numtests`).

### Single workload execution

Run a single benchmark mix with:

```bash
./bench_cast -c spec-cast -p 8 -l 2 -s 10 -v papi_results.csv --SPEC mcf.1 --SPEC bwaves.2 --NPB ft.C --PARSEC swaptions.1
```

This example launches 8 benchmark instances from SPEC, PARSEC, and NPB, synchronizes them in the ROI, and samples PAPI counters for 10 seconds.

#### Common options

* `-b` : launch and return immediately (do not use with PAPI).
* `-p <N>` : number of processors to use.
* `-w` : enable GEM5 work begin in ROI.
* `-s <N>` : measure PAPI counters for N seconds.
* `-c <config>` : SPEC configuration name without `.cfg`.
* `-v <file>` : write PAPI results to CSV.
* `-r <N>` : repeat PAPI measurement N times.
* `-n <N>` : number of distinct programs to run.
* `-l <N>` : number of warm-up ROI iterations before synchronization.
* `-m` : reserve the first N cores for measurement only.

---

## GEM5 Support

To use BenchCast with GEM5, copy these files from gem5 into BenchCast:

* `m5ops.h`
* `m5_mmap.c`
* `m5_mmap.h`
* `m5op_addr.S`

Update `m5op_addr.S` to include the local `m5ops.h`:

```c
#include "m5ops.h"
```

Then build:

```bash
make GEM5=true
```

---

## Adding a new benchmark

1. Identify the application ROI.
2. Add `initialize_barrier` before the ROI loop.
3. Add `call_barrier` inside the ROI loop.
4. Compile the benchmark with `barrier_cast.c` and `barrier_cast.h`.
5. Register the new benchmark in `bench_types.py`.

Support files:

* `BENCH_CONF.py` — benchmark suite paths.
* `bench_types.py` — benchmark definitions and execution commands.
* `SPEC_CMDS.py`, `PARSEC_CMDS.py`, `NPB_CMDS.py` — suite-specific launch commands.
* `bench_cast.h` — long option names.
* `experiment_list.py` — add benchmarks for randomized experiments.

---

## References

If you use BenchCast in research, please cite:

* P. Prieto, P. Abad, J. A. Gregorio and V. Puente, "Fast, Accurate Processor Evaluation Through Heterogeneous, Sample-Based Benchmarking," IEEE Transactions on Parallel and Distributed Systems, vol. 32, no. 12, pp. 2983–2995, 2021.

---

## License

BenchCast is provided under the terms of the included `LICENSE` file.

---
## Disclaimer

BenchCast is currently under development, so future improvements and modifications are spected. If you have any problems, please send an e-mail to prietop@unican.es.
At the current version, BenchCast has been tested on Linux and x86-64 systems.
