#!/bin/bash

set -x

# Assumes that the inputs have been generated and
# things have been built.
if [[ $# -eq 1 ]]; then
	# Only run the one provided as arg. else run all
	cd $1
fi

files=$(find -name value_profiler)

for profiler in ${files[@]}; do
	pushd $(dirname $profiler)
	# Create outputs
	rm -rf value_profiles
	mkdir -p value_profiles

	# Get teh geneated input files.
	ifiles=$(find . -name "*.json")

	for file in ${ifiles[@]}; do
		eval ./$(basename $profiler) $file value_profiles/$(basename $file)
	done
	popd
done
