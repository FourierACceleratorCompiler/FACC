#!/usr/bin/env bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
typeset -a inputs
inputs=("1.0,1.0")
for i in $(seq 1 $(( $1 - 1))); do
	# rnum1=$(printf '0%s' $(echo "scale=8; $RANDOM/32768" | bc ))
	# rnum2=$(printf '0%s' $(echo "scale=8; $RANDOM/32768" | bc ))
	rnum1=1.00
	rnum2=21.313
	inputs+=(", $rnum1, $rnum2")
done

# This uses an input template that gets extended.
template="{
	\"n\": $(( $1 )),
	\"a\": [
	"${inputs[@]}"
	]
}"

echo $template > inputs/$1.json
