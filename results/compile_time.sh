#!/bin/bash

# given (a) a list of benchmarks, (b) a compile mode, and (c) a target apisspec, determine the compile time and place it in an output file.

if [[ $# -ne 3 ]]; then
	echo "Usge: $0 <benchmark list (newline separated)> <compile settings file> <target apispec>"
	exit 1
fi

mkdir -p compile_times
pushd ../synth

bmarks=( $(cat $1 | cut -f1 -d:) )
compsettings=$2
apispec=$3

echo "" -n > ../results/compile_times/compile_times

# Use small number of tests as dictated by theory.
for bmark in "${bmarks[@]}"; do
	echo "Starting new test"
	# use the benchmark list folder as a base.
	bmark_folder="$(dirname $1)"
	iospec="iospec.json"
	if [[ -f $bmark_folder/$bmark/iospec_in_context.json ]]; then
		iospec="iospec_in_context.json"
	fi

	( time ./main.byte --number-of-tests 2 $compsettings $bmark_folder/$bmark/$iospec $apispec ) 2>> ../results/compile_times/compile_times

done
