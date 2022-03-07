#!/bin/bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi

if (( $1 < 30 )); then
	echo "Stop from generating inputs too small"
	exit 0
fi
if (( $1 > 1024 )); then
	echo "Stop from generating inputs too large"
	exit 0
fi

# OK, so this code can actually handle more than this,
# just my benchmark doesnto (needs prime factorization)
inplen=$1
pow2len=$(echo $inplen | awk '{print log($inplen)/log(2)}')
if [[ $pow2len =~ ^[0-9]+\.[0-9]*$ ]]; then # Match floating points
	echo "$_num is not an exponent of 2"; # Not exponent if floating point
	# https://stackoverflow.com/questions/48910300/how-to-find-values-2-exponential-in-shell
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
contents="{\"re\": 2.510045379820, \"im\": 21.72443302820}"
typeset -a inputs
inputs=("$contents")
for i in $(seq 1 $(( $1 - 1))); do
	inputs+=(",$contents")
done

# This uses an input template that gets extended.
template="{
	\"N\": $1,
	\"x\": [
	"${inputs[@]}"
	]
}"

echo $template > inputs/$1.json
