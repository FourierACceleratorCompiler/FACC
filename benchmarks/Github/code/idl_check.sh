#!/bin/bash
set -x

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <IDL Clang Executable>"
	echo "Tool for checking if an IDL pattern matches any code extracted.  "
	exit 1
fi

for file in $(find -name replace-report-self_contained_code.c.json); do
	rm $file
done

for file in $(find -name self_contained_code.c); do
	echo "Looking at $file"
	cd $(dirname $file)
	eval "timeout 50 $1 -DTIMES=1 -I ../../../../synth/libs -I/nix/store/am5qwbpriqhp1i9qhp2idid7ympxqb9a-glibc-2.32-46-dev/include/ self_contained_code.c -O2 -o- -S"
	cd ..
done

matched_files=0
total_run=0
for item in $(find -name replace-report-self_contained_code.c.json); do
	# compare the size of the file.  58B is the size of a non-matching file,
	# and matching files will be siginificantly bigger, so compare against 80
	# to give some buffer space.
	if ((`stat -c%s "$item"` > 80 )); then
		matched_files=$((matched_files + 1))
	fi
	if ((`stat -c%s "$item"` > 0 )); then
		# IDL crashes, and sometimes deps are missing -- good to count how many files
		# we're actually using at this phase.
		total_run=$((total_run + 1))
	fi
done

echo "Number of matched files is $matched_files"
echo "Number of tried files is $total_run"
