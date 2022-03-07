all:
	gcc -lm -std=c99 self_contained_code.c -o self_contained_code.o -c
	g++ -lm -DTIMES=1 -I ../../../../synth/libs/ self_contained_code.o wrapper.cpp -fsanitize=address -o exec
