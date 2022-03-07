#!/bin/bash

if [[ $# -ne 2 ]]; then
	echo "Usage: $0 <model> <fft eval flder>"
	exit 1
fi

for file in $(find $2 -name "*.c" | sort); do
	echo "For file: $file, running classification."
	python evaluate.py $1 $file >> tmpeval
done

echo "Results are:"
grep tmpeval -e "tensor"
rm tmpeval
