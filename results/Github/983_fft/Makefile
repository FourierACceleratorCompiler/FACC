TIMES_IN_BENCHMARK := $(shell v='$(TIMES_IN_BENCHMARK)'; echo "$${v%.*}")

all:
	g++ -DTIMES=1 -I ../../../synth/libs value_profiler.cpp -o value_profiler
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -lfftw3f -Ofast ../../../benchmarks/Github/code/983_fft/self_contained_code.o accelerated_fftw.cpp -L /usr/lib/x86_64-linux-gnu -I ../../../synth/libs -o accelerated_fftw
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -Ofast -I ../../../synth/libs ../../../benchmarks/Github/code/983_fft/self_contained_code.o ../../../benchmarks/Github/code/983_fft/wrapper.cpp -o original
