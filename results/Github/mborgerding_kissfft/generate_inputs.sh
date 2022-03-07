#!/bin/bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
contents="{\"r\": 2.510045379820, \"i\": 21.72443302820}"
typeset -a inputs
inputs=("$contents")
for i in $(seq 1 $(( $1 - 1))); do
	inputs+=(",$contents")
done

# This uses an input template that gets extended.
template="{
	\"nfft\": $1,
	\"fin\": [
	"${inputs[@]}"
	]
}"

echo $template > inputs/$1.json
