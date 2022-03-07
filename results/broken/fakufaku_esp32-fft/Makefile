TIMES_IN_BENCHMARK := $(shell v='$(TIMES_IN_BENCHMARK)'; echo "$${v%.*}")

all:
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -g -I ../../../synth/libs ../../../benchmarks/Github/code/fakufaku_esp32-fft/wrapper.cpp -o original
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -Wall -O0 -g -I ../../../benchmarks/Github/code/fakufaku_esp32-fft/ -I ../../../synth/libs value_profiler.cpp -o value_profiler -fsanitize=undefined -fstack-protector -fsanitize=address
	g++ -DTIMES=${TIMES_IN_BENCHMARK} -Ofast accelerated_fftw.cpp -L /usr/lib/x86_64-linux-gnu -I ../../../synth/libs -o accelerated_fftw
