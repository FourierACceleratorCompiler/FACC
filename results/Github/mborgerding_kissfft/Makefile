TIMES_IN_BENCHMARK := $(shell v='$(TIMES_IN_BENCHMARK)'; echo "$${v%.*}")

all:
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -Ofast -lfftw3f -I ../../../synth/libs accelerated_fftw.cpp -o accelerated_fftw
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -Ofast -I ../../../synth/libs ../../../benchmarks/Github/code/mborgerding_kissfft/wrapper.cpp -o original
