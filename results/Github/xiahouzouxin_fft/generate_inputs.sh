#!/bin/bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
contents="{\"real\": 2.510045379820, \"imag\": 21.72443302820}"
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
