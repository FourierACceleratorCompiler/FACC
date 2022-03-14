#FACC Artifact Evaluation

This set of instructions is split into several sections.  First, we cover the generic build process.  We then cover steps required to reproduce each figure.
The main directory is FACC.  Other sub-directories should be used as refered to.

#Build Instructions (Also in README.md)

Requierments: Nix (see https://nixos.org/download.html)

Building (in FACC folder):
1. In top-level director, run `nix-shell`. This should fetch all dependencies
2. Build FACC: `cd synth; make`
3. Build the FACC libraries: `cd synth/libs/clib; make`
4. Build the evaluation executables: `cd benchmarks/Github; ./make.sh`
5. Run the value-profiling framework: `cd results; ./build.sh; ./generate.sh; ./run_value_profiling.sh`

See the README for more generic examples and debugging information.

#Reproducing Results
#Figure 8
This figure is subjective, but the data for it is noted in the files benchmarks/Github/code/{FINAL_LIST,BUGGY}
The graph can be redrawn using the script in results/:

 python3.9 plot_opensource_distribution.py ../benchmarks/Github/code/FINAL_LIST ../benchmarks/Github/code/BROKEN

#Figure 9
This figure is a collation of data from various further figures, but can be redrawn using the command (executed from results/):

`python plot_success_comps.py CompileStatistics.dat`

#Table 1
This table was created by inspecting the code in FACC/benchmarks/Github/code.
The mapping between repository and project number is in FACC/results/Numbering.  The algorithms are in a file called self_contained_code.c in each sub-directory (of FACC/benchmarks/Github/code).

The reproduction information for the search of Github to obtain these benchmarks is in FACC/benchmarks/Github/code/SEARCH_LIST

#Figure 10
This figure relies on the SC-589 EZKit development board. (https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/eval-adsp-sc589.html).
The DSPAcceleratorSupportFFTA directory in the top-level contains the IDE configuration to rerun these experiments.

To run these experiments:
1. Open CCES and connect your SC-589 EZKit.

2. Compute the baseline (i.e. the un-offloaded time):
2.1. In Hello_Core0/src/Hello_Core0.h, /uncomment/ `#define EVAL_CORE0`.
2.2. In Hello_Core1/src/Hello_Core1.h, /comment/ `#define EVAL_CORE1`.
2.3. For each project in the Hello_Core0/src/Hello_Core0.h file, uncomment the `#define PROJ_x 1` line, then run the project, then re-comment that line.
2.3.1. Copy the stdout into (the top level directory) FACC/results/Github/<benchmark name>/ffta_results/UnAcceleratedResults

3. Compute the version accelered by the neural classifier:
3.1. In Hello_Core1/src/Hello_Core1.h, /uncomment/ `#define EVAL_CORE1`.
3.2. In Hello_Core1/src/Hello_Core1.h, /comment/ `#define EVAL_CORE0`.
3.3. In Hello_Core1/src/Hello_Core1.h, /comment/ `#define run_accelerator`.
3.3. In Hello_Core1/src/Hello_Core1.h, /uncomment/ `#define run_original`.
3.4. For each project in the Hello_Core1/src/Hello_Core1.h file, uncomment the `#define PROJ_x 1` line, then run the project, then re-comment that line.
3.4.1. Copy the stdout into (the top level directory) FACC/results/Github/<benchmark name>/ffta_results/SC589Results

4. Compute the version accelerated by FACC:
4.1. In Hello_Core1/src/Hello_Core1.h, /uncomment/ `#define EVAL_CORE1`.
4.2. In Hello_Core1/src/Hello_Core1.h, /comment/ `#define EVAL_CORE0`.
4.3. In Hello_Core1/src/Hello_Core1.h, /uncomment/ `#define run_accelerator`.
4.3. In Hello_Core1/src/Hello_Core1.h, /comment/ `#define run_original`.
4.4. For each project in the Hello_Core1/src/Hello_Core1.h file, uncomment the `#define PROJ_x 1` line, then run the project, then re-comment that line.
4.4.1. Copy the stdout into (the top level directory) FACC/results/Github/<benchmark name>/ffta_results/AcceleratedResults

Plot the graph (results are already included if the board is not available):

`python3.9 plot_bars.py 1024 Github/JodiTheTigger_meow_fft Github/akw0088_fft Github/cpuimage_FFTResampler Github/cpuimage_StockhamFFT Github/cpuimage_cpuFFT Github/dlinyj_fft Github/gregfjohnson_fft Github/jtfell_c-fft Github/liscio_fft Github/marton78_pffft Github/mborgerding_kissfft Github/mozanunal_SimpleDSP Github/tasimon_FFT Github/xbarin02_uFFT Github/xbarin02_uFFT_2 Github/xiahouzouxin_fft Github/omnister_fftp Github/MiBench_MiBench`

The graph will be in sc589_system_speedup.eps

To re-run FACC to re-generate the adaptors:
To re-generate the adaptors for benchmark X, go to FACC/synth:
1. Run `./main.byte ../benchmarks/Accelerators/SHARCCompileSettings.json ../benchmarks/Github/code/<benchmark>/FILE ../benchmarks/Accelerators/FFTA/FFTA_apispec.json`, where FILE is iospec_in_context.json (if it exists), or iospec.json otherwise.
2. Look in `synthethizer_temps/output/1.cpp`.
3. This file has: a JSON wrapper to enable testing, and a function that computes the FFT.
4. Copy and past the this function into Hello_Core1/src/<benchmark>/accelerated_ffta.c

#Figure 11
This figure is about FFT classification.  The instructions should be followed within the FFTClassification folder.
Unlike all other examples in this document, you should /exit/ the nix-shell environment discussed at the top before reproducing these results.

This directory contains the reproducibility steps for the machine learning part of the article (i.e., Figure 11).

We assume that the data (data/) is already present. It contains the data from the OJClone algorithm classification dataset introduced in Mou et al. 2015 + our extra class consisting of FFTs.

## Setup

Assume a Python3.8 installation in a Unix machine.

Run: 


```
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Run


```

1. Run training and evaluation with cross-validation (this bash loop will execute train.py with different data splits, saving the results for each run in output/):

```

bash run_cv.sh
```


2. Gather results from the cross-validation (this will generate a results.csv file):


```
source venv/bin/activate
python gather_results.py
```

3. Plot results from the previous step (obtaining figure 11)

```
source venv/bin/activate
python plot.py
```

#Figure 12
This figure can be re-plotted using the following command (from FACC/results):
python3.9 plot_idl_graph.py ../benchmarks/Github/code/FFT_IDL_Check

To recompute the data, take the following steps:
1. Go to FACC/benchmarks/Github
2. Run ./GetIDLClang.sh --with-clang
3. Copy code/FFTIdiom.idl to  llvm/lib/IDLParser/Idioms.idl
4. Run ./BuildIDLClang.sh
5. cd code
6. Run ./idl_check.sh ../llvm-build/bin/clang (note, you may have to edit the idl_check.sh script if it cannot find GLIBC)
7. Note the number of matched files and total files tried.
8. delete the last 10 lines from FFTIdiom.idl and go to step 3.

#Figure 13

To draw the graph, go to FACC/results and run ./plot_bars.sh.  The graph will be in barplot_speedup.eps

To gather the data, there are three seperate processes, one for each target.  For the FFTA, follow instructions for figure 10.

##PowerQuad
For the PowerQuad, obtain an NXP LPC55S69 development board.
Open the DSPAcceleratorSupportPowerQuad folder in MCUXpresso:

1. Compute the baseline (i.e. un accelerated):
1.1. Open LPC55S69_Project/source/LPC55S69_Project.h
1.2. Comment the line that says `#define RUNFUN run_accelerator`
1.3. Uncomment the line that says `#define RUNFUN run_original`
1.4. For each project on the lines that say `#define PROJ_x`, uncomment one at a time, and run the project.
1.5. Copy the stdout into FACC/results/Github/x/powerquad_results/UnAcceleratedResults

2. Compute the accelerated version:
2.1. Open LPC55S69_Project/source/LPC55S69_Project.h
2.2. Unomment the line that says `#define RUNFUN run_accelerator`
2.3. Comment the line that says `#define RUNFUN run_original`
2.4. For each project on the lines that say `#define PROJ_x`, uncomment one at a time, and run the project.
2.5. Copy the stdout into FACC/results/Github/x/powerquad_results/AcceleratedResults

To rebuild the adaptors:
1. Go to FACC/synth
2. Run `./main.byte ../benchmarks/Accelerators/ArmCompileSettings.json ../benchmarks/Github/code/<benchmark>/FILE ../benchmarks/Accelerators/PowerQuad/PowerQuad_apispec.json` where FILE is iospec_in_context.json (if it exists), or iospec.json otherwise.
3. Look in `synthethizer_temps/output/1.cpp`.
4. This file has: a JSON wrapper to enable testing, and a function that computes the FFT.
5. Copy and past the this function into LPC55S69_Project/source/x/accelerated_powerquad.c

##FFTW
To compute the speedups in FFTA, go to FACC/results and run ./execute.sh. 

To rebuild the adaptors

#Figure 14
To compute the speedups for different problem sizes, follow the instructions for figure 13 to obtain the data.

To plot the graph, go to FACC/results and run:

python3.9 plot_joint_graph.py Github/mozanunal_SimpleDSP/ Github/tasimon_FFT/ Github/cpuimage_StockhamFFT/ Github/xiahouzouxin_fft/ Github/JoditheTigger_meow_fft/ Github/marton78_pffft/ Github/omnister_fftp/ Github/dlinyj_fft/

#Figure 15
To compute compile times (this is obviously going to be machine-dependent), go to FACC/results and run ./all_compile_times.sh.

To plot the graph, run `python3.9 plot_compile_times.py compile_times`

#Figure 16
To compute the number of candidates, go to FACC/results and run ./all_candidate_counts.sh

To plot the graph, run `python3.9 plot_candidates.py binding_candidates`
