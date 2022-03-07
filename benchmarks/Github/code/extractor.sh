#!/bin/bash
# set -x

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <extractor file format>"
	echo "Extractor file format is:"
	echo "one entry per line, format is 'filename: <lineno start>,<lineno end> <#optional comment>'"
	echo "or: INSERT: contents"
	echo "or: SUBSTITUTE: search string : replace string"
	echo " applies to all subsequent lines."
	exit 1
fi

typeset -a search_replaces

IFS=$'\n'
for line in $(cat $1); do
	fname=$(echo $line | cut -f1 -d ':')
	range=$(echo $line | cut -f2 -d ' ')
	if [[ $fname == *INSERT* ]]; then
		contents="$(echo $line | cut -f2- -d:)"
	elif [[ $fname == *SUBSTITUTE* ]]; then
		from=$(echo $line | cut -f2 -d ':')
		to=$(echo $line | cut -f3 -d ':')
		search_replaces+=("s:$from:$to:g")
		contents=""
	else
		contents="$(sed -n "${range}p" $fname)"
	fi

	if [[ $fname != "*SUBSTITUTE*" ]]; then
		for srep in ${search_replaces[@]}; do
			contents="$(echo "$contents" | sed --expression="$srep")"
		done

		echo "$contents"
	fi
done
