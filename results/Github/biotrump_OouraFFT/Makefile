TIMES_IN_BENCHMARK := $(shell v='$(TIMES_IN_BENCHMARK)'; echo "$${v%.*}")

all:
	g++ -g -DTIMES=${TIMES_IN_BENCHMARK} -fsanitize=address -I ../../../synth/libs in_context_original.cpp -o original
	g++ -DTIMES=1 -g value_profiler.cpp -I ../../../synth/libs -o value_profiler
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -lfftw3f -Ofast accelerated_fftw.cpp -L /usr/lib/x86_64-linux-gnu -I ../../../synth/libs -o accelerated_fftw
