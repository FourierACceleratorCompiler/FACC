This contains the scripts required for generating various results.


1.  Syntactic similarity measurements: the vectors are
already pre-computed in DeckardVectors.  Run plot_syntactic_similarity.py to plot this.
1.a. To recompute the vectors, go to ../benchmarks/Github/code/ and run ../../scripts/compute_deckard_vectors.sh FINAL_LIST vectors_file

2.  Compile Time results:
2.a. Build FACC (see top-level readme)
2.b. Build github executables (go to ../benchmarks/Github/ and run make.sh)
2.c. Do value profiling (see below)
2.c. Make sure C synthesizer libs are built (run make in ../synth/libs/clib/)
2.d. Run ./all_compile_times.sh, and then python plot_compile_times.py

3. Value profiling:
This is a requirement for running the synthesizer on some problems.
3.a. Build executables (./build.sh)
3.b. Generate the benchmark inputs (./generate.sh)
3.c. Run value profiling (./run_value_profiling.sh)
Not all programs actually work, so some errors can be expected.
