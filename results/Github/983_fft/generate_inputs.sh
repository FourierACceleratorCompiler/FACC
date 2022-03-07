#!/bin/bash

set -eu

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <input size>"
	exit 1
fi

mkdir -p inputs
# I haven't yet seen a data-dependent FFT algorithm,
# so we'll just use a constnat input.
re_contents="2.510045379820"
im_contents="21.72443302820"
typeset -a inputs
re_inputs=("$re_contents")
im_inputs=("$im_contents")
for i in $(seq 1 $(( $1 - 1))); do
	re_inputs+=(",$re_contents")
	im_inputs+=(",$im_contents")
done

# This uses an input template that gets extended.
template="{
	\"n\": $1,
	\"re\": [
	"${re_inputs[@]}"
	],
	\"im\": [
	"${im_inputs[@]}"
	]
}"

echo $template > inputs/$1.json
