#!/usr/bin/env bash

# Script to build all the wrappers in a mode for use with the synthesizer (not for benchmarking,
# confusingly enough, see the results folder for that!)

makefiles=( $(find -name Makefile) )

for makefile in ${makefiles[@]}; do
	pushd $(dirname $makefile)
	make
	popd
done
