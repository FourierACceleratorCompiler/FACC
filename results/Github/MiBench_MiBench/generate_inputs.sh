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


inplen=$(( $1 ))
pow2len=$(echo $inplen | awk '{print log($inplen)/log(2)}')
if [[ $pow2len =~ ^[0-9]+\.[0-9]*$ ]]; then # Match floating points
	echo "$_num is not an exponent of 2"; # Not exponent if floating point
	# https://stackoverflow.com/questions/48910300/how-to-find-values-2-exponential-in-shell
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
re_contents="2.510045379820"
im_contents="21.7244"
typeset -a re_inputs
typeset -a im_inputs
re_inputs=("$re_contents")
im_inputs=("$im_contents")

for i in $(seq 1 $(( $inplen - 1 ))); do
	re_inputs+=(",$re_contents")
	im_inputs+=(",$im_contents")
done

# This uses an input template that gets extended.
template="{
	\"NumSamples\":$inplen,
	\"InverseTransform\": 0,
	\"RealIn\": [
	"${re_inputs[@]}"
	],
	\"ImagIn\": [
	"${im_inputs[@]}"
	]
}"

echo $template > inputs/$inplen.json
