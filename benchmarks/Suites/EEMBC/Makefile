all:
	# Buld the main thing
	gcc -fdump-tree-vrp -Ofast -g -lm fft00.c verify.c lite_includes/*.c x86_includes/*.c bmark_lite.c -I lite_includes -I x86_includes -I. -I diffmeasure -I datasets
	# Build the executable that ACC uses.
	gcc -O2 -g -lm lite_includes/*.c fft00.c verify.c x86_includes/*.c -I lite_includes/ -I x86_includes -I. -I diffmeasure -I datasets -c
	g++ -O2 -g eembc_wrapper.cpp ./*.o -I ../../../synth/libs/ -I lite_includes/ -I x86_includes  -I. -I diffmeasure -o EEMBC_exec
