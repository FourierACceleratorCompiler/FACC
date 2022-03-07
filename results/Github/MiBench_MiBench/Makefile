TIMES_IN_BENCHMARK := $(shell v='$(TIMES_IN_BENCHMARK)'; echo "$${v%.*}")

all:
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -fpermissive -lfftw3f -Ofast accelerated_fftw.cpp -L /usr/lib/x86_64-linux-gnu -I ../../../synth/libs -o accelerated_fftw
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -fpermissive -Ofast -I ../../../synth/libs ../../../benchmarks/Suites/MiBench/MiBench_wrapper.cpp ../../../benchmarks/Suites/MiBench/fftmisc.c -o original
