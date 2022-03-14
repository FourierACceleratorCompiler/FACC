#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
	echo "Usage: $0 <files list> <output file>"
fi

output_file=$2

vembeddings=""

for file in $(cat $1); do
	file=$(cut -f1 -d: <<<$file)
	# there are just some bugs with the way
	# this is doing line processing that mean
	# that this is sometimes not a directory.
	if [[ -d $file ]]; then
		code=$file/self_contained_code.c
		cvecgen -i $code -o tempout # generate the vectors
		# Get teh appropriate vector line:
		info_file=$file/INFO
		line=$(grep -e FunctionLine: $info_file | cut -f2 -d: | xargs)
		echo "$line"
		# Get the vector embedding for that line: --- the first one seems to have
		# the most vector informatin.
		vembeddings="$vembeddings\n$file: $(grep -e "LINE:$line" tempout -A1 | head -n 2 | tail -n 1)"
	fi
done

rm tmpout

# Write to the output file:
printf "$vembeddings" > $output_file
