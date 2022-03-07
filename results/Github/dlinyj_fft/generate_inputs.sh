#!/bin/bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi
if (( $1 > 10000 )); then
	echo "Arg too big, skipping"
	exit 1
fi


inplen=$(( $1 * 2 )) # this uses individual floats, so needs to be doubled
pow2len=$(echo $inplen | awk '{print log($inplen)/log(2)}')
if [[ $pow2len =~ ^[0-9]+\.[0-9]*$ ]]; then # Match floating points
	echo "$_num is not an exponent of 2"; # Not exponent if floating point
	# https://stackoverflow.com/questions/48910300/how-to-find-values-2-exponential-in-shell
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
contents="2.510045379820, 21.72443302820"
typeset -a inputs
inputs=("$contents")
for i in $(seq 1 $(( $inplen - 1 ))); do
	inputs+=(",$contents")
done

# This uses an input template that gets extended.
template="{
	\"norm\": 1,
	\"p\":$pow2len,
	\"in\": [
	"${inputs[@]}"
	]
}"

echo $template > inputs/$inplen.json
