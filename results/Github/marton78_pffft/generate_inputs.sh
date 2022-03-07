#!/bin/bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi

inputlen=$(( $1 * 2 ))
if (( inputlen < 32 )); then
	echo "input too small"
	exit 0
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
contents="2.510045379820, 21.72443302820"
typeset -a inputs
inputs=("$contents")
for i in $(seq 1 $(( $inputlen - 1 ))); do
	inputs+=(",$contents")
done

# This uses an input template that gets extended.
template="{
	\"input\": [
	"${inputs[@]}"
	]
}"

echo $template > inputs/$inputlen.json
