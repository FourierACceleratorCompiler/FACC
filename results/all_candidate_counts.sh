#!/bin/bash

# FFTW
./candidate_counts.sh ../benchmarks/Github/code/FINAL_LIST ../benchmarks/Accelerators/x86CompileSettings.json ../benchmarks/Accelerators/FFTW/FFTW_apispec.json
mv binding_candidates/candidates binding_candidates/FFTW

# FFTA
./candidate_counts.sh ../benchmarks/Github/code/FINAL_LIST ../benchmarks/Accelerators/SHARCCompileSettings.json ../benchmarks/Accelerators/FFTA/FFTA_apispec.json
mv binding_candidates/candidates binding_candidates/FFTA

# PowerQuad
./candidate_counts.sh ../benchmarks/Github/code/FINAL_LIST ../benchmarks/Accelerators/ArmCompileSettings.json ../benchmarks/Accelerators/PowerQuad/PowerQuad_apispec.json
mv binding_candidates/candidates binding_candidates/PowerQuad
